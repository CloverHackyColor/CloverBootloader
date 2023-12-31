/** @file
  Directory related routines

  Copyright (c) 2021 - 2023 Pedro Falcato All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Ext4Dxe.h"

#include <Library/BaseUcs2Utf8Lib.h>

/**
   Retrieves the filename of the directory entry and converts it to UTF-16/UCS-2

   @param[in]      Entry   Pointer to a EXT4_DIR_ENTRY.
   @param[out]      Ucs2FileName   Pointer to an array of CHAR16's, of size EXT4_NAME_MAX + 1.

   @retval EFI_SUCCESS              The filename was successfully retrieved and converted to UCS2.
   @retval EFI_INVALID_PARAMETER    The filename is not valid UTF-8.
   @retval !EFI_SUCCESS             Failure.
**/
EFI_STATUS
Ext4GetUcs2DirentName (
  IN EXT4_DIR_ENTRY  *Entry,
  OUT CHAR16         Ucs2FileName[EXT4_NAME_MAX + 1]
  )
{
  CHAR8       Utf8NameBuf[EXT4_NAME_MAX + 1];
  UINT16      *Str;
  UINT8       Index;
  EFI_STATUS  Status;

  for (Index = 0; Index < Entry->name_len; ++Index) {
    if (Entry->name[Index] == '\0') {
      return EFI_INVALID_PARAMETER;
    }

    Utf8NameBuf[Index] = Entry->name[Index];
  }

  Utf8NameBuf[Entry->name_len] = '\0';

  // Unfortunately, BaseUcs2Utf8Lib doesn't have a convert-buffer-to-buffer-like
  // function. Therefore, we need to allocate from the pool (inside UTF8StrToUCS2),
  // copy it to our out buffer (Ucs2FileName) and free.

  Status = UTF8StrToUCS2 (Utf8NameBuf, &Str);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = StrCpyS (Ucs2FileName, EXT4_NAME_MAX + 1, Str);

  FreePool (Str);

  return Status;
}

/**
   Validates a directory entry.

   @param[in]      Dirent      Pointer to the directory entry.

   @retval TRUE          Valid directory entry.
           FALSE         Invalid directory entry.
**/
STATIC
BOOLEAN
Ext4ValidDirent (
  IN CONST EXT4_DIR_ENTRY  *Dirent
  )
{
  UINTN  RequiredSize;

  RequiredSize = Dirent->name_len + EXT4_MIN_DIR_ENTRY_LEN;

  if (Dirent->rec_len < RequiredSize) {
    DEBUG ((DEBUG_ERROR, "[ext4] dirent size %lu too small (compared to %lu)\n", Dirent->rec_len, RequiredSize));
    return FALSE;
  }

  // Dirent sizes need to be 4 byte aligned
  if ((Dirent->rec_len % 4) != 0) {
    return FALSE;
  }

  return TRUE;
}

/**
   Retrieves a directory entry.

   @param[in]      Directory   Pointer to the opened directory.
   @param[in]      NameUnicode Pointer to the UCS-2 formatted filename.
   @param[in]      Partition   Pointer to the ext4 partition.
   @param[out]     Result      Pointer to the destination directory entry.

   @return The result of the operation.
**/
EFI_STATUS
Ext4RetrieveDirent (
  IN EXT4_FILE        *Directory,
  IN CONST CHAR16     *Name,
  IN EXT4_PARTITION   *Partition,
  OUT EXT4_DIR_ENTRY  *Result
  )
{
  EFI_STATUS      Status;
  CHAR8           *Buf;
  UINT64          Off;
  EXT4_INODE      *Inode;
  UINT64          DirInoSize;
  UINT32          BlockRemainder;
  UINTN           Length;
  EXT4_DIR_ENTRY  *Entry;
  UINTN           RemainingBlock;
  CHAR16          DirentUcs2Name[EXT4_NAME_MAX + 1];
  UINTN           ToCopy;
  UINTN           BlockOffset;

  Buf = AllocatePool (Partition->BlockSize);

  if (Buf == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Off = 0;

  Inode      = Directory->Inode;
  DirInoSize = EXT4_INODE_SIZE (Inode);

  DivU64x32Remainder (DirInoSize, Partition->BlockSize, &BlockRemainder);
  if (BlockRemainder != 0) {
    // Directory inodes need to have block aligned sizes
    Status = EFI_VOLUME_CORRUPTED;
    goto Out;
  }

  while (Off < DirInoSize) {
    Length = Partition->BlockSize;

    Status = Ext4Read (Partition, Directory, Buf, Off, &Length);

    if (Status != EFI_SUCCESS) {
      goto Out;
    }

    for (BlockOffset = 0; BlockOffset < Partition->BlockSize; ) {
      Entry          = (EXT4_DIR_ENTRY *)(Buf + BlockOffset);
      RemainingBlock = Partition->BlockSize - BlockOffset;
      // Check if the minimum directory entry fits inside [BlockOffset, EndOfBlock]
      if (RemainingBlock < EXT4_MIN_DIR_ENTRY_LEN) {
        Status = EFI_VOLUME_CORRUPTED;
        goto Out;
      }

      if (!Ext4ValidDirent (Entry)) {
        Status = EFI_VOLUME_CORRUPTED;
        goto Out;
      }

      if ((Entry->name_len > RemainingBlock) || (Entry->rec_len > RemainingBlock)) {
        // Corrupted filesystem
        Status = EFI_VOLUME_CORRUPTED;
        goto Out;
      }

      // Unused entry
      if (Entry->inode == 0) {
        BlockOffset += Entry->rec_len;
        continue;
      }

      Status = Ext4GetUcs2DirentName (Entry, DirentUcs2Name);

      /* In theory, this should never fail.
       * In reality, it's quite possible that it can fail, considering filenames in
       * Linux (and probably other nixes) are just null-terminated bags of bytes, and don't
       * need to form valid ASCII/UTF-8 sequences.
       */
      if (EFI_ERROR (Status)) {
        if (Status == EFI_INVALID_PARAMETER) {
          // If we error out due to a bad UTF-8 sequence (see Ext4GetUcs2DirentName), skip this entry.
          // I'm not sure if this is correct behaviour, but I don't think there's a precedent here.
          BlockOffset += Entry->rec_len;
          continue;
        }

        // Other sorts of errors should just error out.
        FreePool (Buf);
        return Status;
      }

      if ((Entry->name_len == StrLen (Name)) &&
          !Ext4StrCmpInsensitive (DirentUcs2Name, (CHAR16 *)Name))
      {
        ToCopy = MIN (Entry->rec_len, sizeof (EXT4_DIR_ENTRY));

        CopyMem (Result, Entry, ToCopy);
        Status = EFI_SUCCESS;
        goto Out;
      }

      BlockOffset += Entry->rec_len;
    }

    Off += Partition->BlockSize;
  }

  Status = EFI_NOT_FOUND;

Out:
  FreePool (Buf);
  return Status;
}

/**
   Opens a file using a directory entry.

   @param[in]      Partition   Pointer to the ext4 partition.
   @param[in]      OpenMode    Mode in which the file is supposed to be open.
   @param[out]     OutFile     Pointer to the newly opened file.
   @param[in]      Entry       Directory entry to be used.
   @param[in]      Directory   Pointer to the opened directory.

   @retval EFI_STATUS          Result of the operation
**/
EFI_STATUS
Ext4OpenDirent (
  IN  EXT4_PARTITION  *Partition,
  IN  UINT64          OpenMode,
  OUT EXT4_FILE       **OutFile,
  IN  EXT4_DIR_ENTRY  *Entry,
  IN  EXT4_FILE       *Directory
  )
{
  EFI_STATUS  Status;
  CHAR16      FileName[EXT4_NAME_MAX + 1];
  EXT4_FILE   *File;

  File = AllocateZeroPool (sizeof (EXT4_FILE));

  if (File == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }

  Status = Ext4GetUcs2DirentName (Entry, FileName);

  if (EFI_ERROR (Status)) {
    goto Error;
  }

  if (StrCmp (FileName, L".") == 0) {
    // We're using the parent directory's dentry
    File->Dentry = Directory->Dentry;

    ASSERT (File->Dentry != NULL);

    Ext4RefDentry (File->Dentry);
  } else if (StrCmp (FileName, L"..") == 0) {
    // Using the parent's parent's dentry
    File->Dentry = Directory->Dentry->Parent;

    if (!File->Dentry) {
      // Someone tried .. on root, so direct them to /
      // This is an illegal EFI Open() but is possible to hit from a variety of internal code
      File->Dentry = Directory->Dentry;
    }

    Ext4RefDentry (File->Dentry);
  } else {
    File->Dentry = Ext4CreateDentry (FileName, Directory->Dentry);

    if (File->Dentry == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Error;
    }
  }

  Status = Ext4InitExtentsMap (File);

  if (EFI_ERROR (Status)) {
    goto Error;
  }

  File->InodeNum = Entry->inode;

  Ext4SetupFile (File, Partition);

  Status = Ext4ReadInode (Partition, Entry->inode, &File->Inode);

  if (EFI_ERROR (Status)) {
    goto Error;
  }

  *OutFile = File;

  InsertTailList (&Partition->OpenFiles, &File->OpenFilesListNode);

  return EFI_SUCCESS;

Error:
  if (File != NULL) {
    if (File->Dentry != NULL) {
      Ext4UnrefDentry (File->Dentry);
    }

    if (File->ExtentsMap != NULL) {
      OrderedCollectionUninit (File->ExtentsMap);
    }

    FreePool (File);
  }

  return Status;
}

/**
   Opens a file.

   @param[in]      Directory   Pointer to the opened directory.
   @param[in]      Name        Pointer to the UCS-2 formatted filename.
   @param[in]      Partition   Pointer to the ext4 partition.
   @param[in]      OpenMode    Mode in which the file is supposed to be open.
   @param[out]     OutFile     Pointer to the newly opened file.

   @return Result of the operation.
**/
EFI_STATUS
Ext4OpenFile (
  IN  EXT4_FILE       *Directory,
  IN  CONST CHAR16    *Name,
  IN  EXT4_PARTITION  *Partition,
  IN  UINT64          OpenMode,
  OUT EXT4_FILE       **OutFile
  )
{
  EXT4_DIR_ENTRY  Entry;
  EFI_STATUS      Status;

  Status = Ext4RetrieveDirent (Directory, Name, Partition, &Entry);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  // EFI requires us to error out on ".." opens for the root directory
  if (Entry.inode == Directory->InodeNum) {
    return EFI_NOT_FOUND;
  }

  return Ext4OpenDirent (Partition, OpenMode, OutFile, &Entry, Directory);
}

/**
  Open the root directory on a volume.

  @param[in]   This A pointer to the volume to open the root directory.
  @param[out]  Root A pointer to the location to return the opened file handle for the
                    root directory.

  @retval EFI_SUCCESS          The device was opened.
  @retval EFI_UNSUPPORTED      This volume does not support the requested file system type.
  @retval EFI_NO_MEDIA         The device has no medium.
  @retval EFI_DEVICE_ERROR     The device reported an error.
  @retval EFI_VOLUME_CORRUPTED The file system structures are corrupted.
  @retval EFI_ACCESS_DENIED    The service denied access to the file.
  @retval EFI_OUT_OF_RESOURCES The volume was not opened due to lack of resources.
  @retval EFI_MEDIA_CHANGED    The device has a different medium in it or the medium is no
                               longer supported. Any existing file handles for this volume are
                               no longer valid. To access the files on the new medium, the
                               volume must be reopened with OpenVolume().

**/
EFI_STATUS
EFIAPI
Ext4OpenVolume (
  IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *This,
  OUT EFI_FILE_PROTOCOL               **Root
  )
{
  EXT4_INODE      *RootInode;
  EFI_STATUS      Status;
  EXT4_FILE       *RootDir;
  EXT4_PARTITION  *Partition;

  Partition = (EXT4_PARTITION *)This;

  Status = Ext4ReadInode (Partition, EXT4_ROOT_INODE_NR, &RootInode);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[ext4] Could not open root inode - error %r\n", Status));
    return Status;
  }

  RootDir = AllocateZeroPool (sizeof (EXT4_FILE));

  if (RootDir == NULL) {
    FreePool (RootInode);
    return EFI_OUT_OF_RESOURCES;
  }

  RootDir->Inode    = RootInode;
  RootDir->InodeNum = EXT4_ROOT_INODE_NR;

  Status = Ext4InitExtentsMap (RootDir);

  if (EFI_ERROR (Status)) {
    FreePool (RootInode);
    FreePool (RootDir);
    return EFI_OUT_OF_RESOURCES;
  }

  Ext4SetupFile (RootDir, Partition);
  *Root = &RootDir->Protocol;

  InsertTailList (&Partition->OpenFiles, &RootDir->OpenFilesListNode);

  ASSERT (Partition->RootDentry != NULL);
  RootDir->Dentry = Partition->RootDentry;

  Ext4RefDentry (RootDir->Dentry);

  return EFI_SUCCESS;
}

/**
   Reads a directory entry.

   @param[in]      Partition   Pointer to the ext4 partition.
   @param[in]      File        Pointer to the open directory.
   @param[out]     Buffer      Pointer to the output buffer.
   @param[in]      Offset      Initial directory position.
   @param[in out] OutLength    Pointer to a UINTN that contains the length of the buffer,
                               and the length of the actual EFI_FILE_INFO after the call.

   @return Result of the operation.
**/
EFI_STATUS
Ext4ReadDir (
  IN EXT4_PARTITION  *Partition,
  IN EXT4_FILE       *File,
  OUT VOID           *Buffer,
  IN UINT64          Offset,
  IN OUT UINTN       *OutLength
  )
{
  EXT4_INODE      *DirIno;
  EFI_STATUS      Status;
  UINT64          DirInoSize;
  UINTN           Len;
  UINT32          BlockRemainder;
  EXT4_DIR_ENTRY  Entry;
  EXT4_FILE       *TempFile;
  BOOLEAN         ShouldSkip;
  BOOLEAN         IsDotOrDotDot;
  CHAR16          DirentUcs2Name[EXT4_NAME_MAX + 1];

  DirIno     = File->Inode;
  Status     = EFI_SUCCESS;
  DirInoSize = EXT4_INODE_SIZE (DirIno);

  DivU64x32Remainder (DirInoSize, Partition->BlockSize, &BlockRemainder);
  if (BlockRemainder != 0) {
    // Directory inodes need to have block aligned sizes
    return EFI_VOLUME_CORRUPTED;
  }

  while (TRUE) {
    TempFile = NULL;

    // We (try to) read the maximum size of a directory entry at a time
    // Note that we don't need to read any padding that may exist after it.
    Len    = sizeof (Entry);
    Status = Ext4Read (Partition, File, &Entry, Offset, &Len);

    if (EFI_ERROR (Status)) {
      goto Out;
    }

    if (Len == 0) {
      *OutLength = 0;
      Status     = EFI_SUCCESS;
      goto Out;
    }

    if (Len < EXT4_MIN_DIR_ENTRY_LEN) {
      Status = EFI_VOLUME_CORRUPTED;
      goto Out;
    }

    // Invalid directory entry length
    if (!Ext4ValidDirent (&Entry)) {
      DEBUG ((DEBUG_ERROR, "[ext4] Invalid dirent at offset %lu\n", Offset));
      Status = EFI_VOLUME_CORRUPTED;
      goto Out;
    }

    // Check if the entire dir entry length fits in Len
    if (Len < (UINTN)(EXT4_MIN_DIR_ENTRY_LEN + Entry.name_len)) {
      Status = EFI_VOLUME_CORRUPTED;
      goto Out;
    }

    // We don't care about passing . or .. entries to the caller of ReadDir(),
    // since they're generally useless entries *and* may break things if too
    // many callers assume FAT32.

    // Entry.name_len may be 0 if it's a nameless entry, like an unused entry
    // or a checksum at the end of the directory block.
    // memcmp (and CompareMem) return 0 when the passed length is 0.

    // We must bound name_len as > 0 and <= 2 to avoid any out-of-bounds accesses or bad detection of
    // "." and "..".
    IsDotOrDotDot = Entry.name_len > 0 && Entry.name_len <= 2 &&
                    CompareMem (Entry.name, "..", Entry.name_len) == 0;

    // When inode = 0, it's unused. When name_len == 0, it's a nameless entry
    // (which we should not expose to ReadDir).
    ShouldSkip = Entry.inode == 0 || Entry.name_len == 0 || IsDotOrDotDot;

    if (ShouldSkip) {
      Offset += Entry.rec_len;
      continue;
    }

    // Test if the dirent is valid utf-8. This is already done inside Ext4OpenDirent but EFI_INVALID_PARAMETER
    // has the danger of its meaning being overloaded in many places, so we can't skip according to that.
    // So test outside of it, explicitly.
    Status = Ext4GetUcs2DirentName (&Entry, DirentUcs2Name);

    if (EFI_ERROR (Status)) {
      if (Status == EFI_INVALID_PARAMETER) {
        // Bad UTF-8, skip.
        Offset += Entry.rec_len;
        continue;
      }

      goto Out;
    }

    Status = Ext4OpenDirent (Partition, EFI_FILE_MODE_READ, &TempFile, &Entry, File);

    if (EFI_ERROR (Status)) {
      goto Out;
    }

    Status = Ext4GetFileInfo (TempFile, Buffer, OutLength);
    if (!EFI_ERROR (Status)) {
      File->Position = Offset + Entry.rec_len;
    }

    Ext4CloseInternal (TempFile);

    goto Out;
  }

  Status = EFI_SUCCESS;
Out:
  return Status;
}

/**
   Removes a dentry from the other's list.

   @param[in out]            Parent       Pointer to the parent EXT4_DENTRY.
   @param[in out]            ToBeRemoved  Pointer to the child EXT4_DENTRY.
**/
STATIC
VOID
Ext4RemoveDentry (
  IN OUT EXT4_DENTRY  *Parent,
  IN OUT EXT4_DENTRY  *ToBeRemoved
  )
{
  ASSERT (IsNodeInList (&ToBeRemoved->ListNode, &Parent->Children));
  RemoveEntryList (&ToBeRemoved->ListNode);
}

/**
   Adds a dentry to the other's list.

   The dentry that is added to the other one's list gets ->Parent set to Parent,
   and the parent gets its reference count incremented.

   @param[in out]            Parent       Pointer to the parent EXT4_DENTRY.
   @param[in out]            ToBeAdded    Pointer to the child EXT4_DENTRY.
**/
STATIC
VOID
Ext4AddDentry (
  IN OUT EXT4_DENTRY  *Parent,
  IN OUT EXT4_DENTRY  *ToBeAdded
  )
{
  ToBeAdded->Parent = Parent;
  InsertTailList (&Parent->Children, &ToBeAdded->ListNode);
  Ext4RefDentry (Parent);
}

/**
   Creates a new dentry object.

   @param[in]              Name        Name of the dentry.
   @param[in out opt]      Parent      Parent dentry, if it's not NULL.

   @return The new allocated and initialised dentry.
           The ref count will be set to 1.
**/
EXT4_DENTRY *
Ext4CreateDentry (
  IN CONST CHAR16     *Name,
  IN OUT EXT4_DENTRY  *Parent  OPTIONAL
  )
{
  EXT4_DENTRY  *Dentry;
  EFI_STATUS   Status;

  Dentry = AllocateZeroPool (sizeof (EXT4_DENTRY));

  if (Dentry == NULL) {
    return NULL;
  }

  Dentry->RefCount = 1;

  // This StrCpyS should not fail.
  Status = StrCpyS (Dentry->Name, ARRAY_SIZE (Dentry->Name), Name);

  ASSERT_EFI_ERROR (Status);

  InitializeListHead (&Dentry->Children);

  if (Parent != NULL) {
    Ext4AddDentry (Parent, Dentry);
  }

  DEBUG ((DEBUG_FS, "[ext4] Created dentry %s\n", Name));

  return Dentry;
}

/**
   Increments the ref count of the dentry.

   @param[in out]            Dentry    Pointer to a valid EXT4_DENTRY.
**/
VOID
Ext4RefDentry (
  IN OUT EXT4_DENTRY  *Dentry
  )
{
  UINTN  OldRef;

  OldRef = Dentry->RefCount;

  Dentry->RefCount++;

  // I'm not sure if this (Refcount overflow) is a valid concern,
  // but it's better to be safe than sorry.
  ASSERT (OldRef < Dentry->RefCount);
}

/**
   Deletes the dentry.

   @param[in out]            Dentry    Pointer to a valid EXT4_DENTRY.
**/
STATIC
VOID
Ext4DeleteDentry (
  IN OUT EXT4_DENTRY  *Dentry
  )
{
  if (Dentry->Parent) {
    Ext4RemoveDentry (Dentry->Parent, Dentry);
    Ext4UnrefDentry (Dentry->Parent);
  }

  DEBUG ((DEBUG_FS, "[ext4] Deleted dentry %s\n", Dentry->Name));
  FreePool (Dentry);
}

/**
   Decrements the ref count of the dentry.
   If the ref count is 0, it's destroyed.

   @param[in out]            Dentry    Pointer to a valid EXT4_DENTRY.

   @retval True if it was destroyed, false if it's alive.
**/
BOOLEAN
Ext4UnrefDentry (
  IN OUT EXT4_DENTRY  *Dentry
  )
{
  Dentry->RefCount--;

  if (Dentry->RefCount == 0) {
    Ext4DeleteDentry (Dentry);
    return TRUE;
  }

  return FALSE;
}
