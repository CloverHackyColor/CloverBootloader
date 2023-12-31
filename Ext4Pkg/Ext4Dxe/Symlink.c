/** @file
  Symbolic links routines

  Copyright (c) 2022-2023 Savva Mitrofanov All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Ext4Dxe.h"

/**
   Detects if a symlink is a fast symlink.

   @param[in]      File          Pointer to the opened file.

   @return BOOLEAN         Whether symlink is a fast symlink
**/
STATIC
BOOLEAN
Ext4SymlinkIsFastSymlink (
  IN CONST EXT4_FILE  *File
  )
{
  //
  // Detection logic of the fast-symlink splits into two behaviors - old and new.
  // The old behavior is based on comparing the extended attribute blocks
  // with the inode's i_blocks, and if it's zero we know the inode isn't storing
  // the link in filesystem blocks, so we look to the inode->i_data.
  // The new behavior is apparently needed only with the large EA inode feature.
  // In this case we check that inode size less than maximum fast symlink size.
  // So, we revert to the old behavior if the large EA inode feature is not set.
  //
  UINT32  FileAcl;
  UINT32  ExtAttrBlocks;

  if ((File->Inode->i_flags & EXT4_EA_INODE_FL) == 0) {
    FileAcl = File->Inode->i_file_acl;
    if (EXT4_IS_64_BIT (File->Partition)) {
      //
      // We don't care about final value, we are just checking for any bit is set
      // so, thats why we neglect LShiftU64(.., 32)
      //
      FileAcl |= File->Inode->i_osd2.data_linux.l_i_file_acl_high;
    }

    ExtAttrBlocks = FileAcl != 0 ? (File->Partition->BlockSize >> 9) : 0;

    return File->Inode->i_blocks == ExtAttrBlocks;
  }

  return EXT4_INODE_SIZE (File->Inode) <= EXT4_FAST_SYMLINK_MAX_SIZE;
}

/**
  Reads a fast symlink file.

  @param[in]      Partition   Pointer to the ext4 partition.
  @param[in]      File        Pointer to the open symlink file.
  @param[out]     AsciiSymlink     Pointer to the output ascii symlink string.
  @param[out]     AsciiSymlinkSize Pointer to the output ascii symlink string length.

  @retval EFI_SUCCESS            Fast symlink was read.
  @retval EFI_OUT_OF_RESOURCES   Memory allocation error.
**/
STATIC
EFI_STATUS
Ext4ReadFastSymlink (
  IN     EXT4_PARTITION  *Partition,
  IN     EXT4_FILE       *File,
  OUT    CHAR8           **AsciiSymlink,
  OUT    UINT32          *AsciiSymlinkSize
  )
{
  UINT32  SymlinkSize;
  CHAR8   *AsciiSymlinkTmp;

  //
  // Fast-symlink's EXT4_INODE_SIZE is not necessarily validated when we checked it in
  // Ext4SymlinkIsFastSymlink(), so truncate if necessary.
  //
  SymlinkSize = (UINT32)MIN (EXT4_INODE_SIZE (File->Inode), EXT4_FAST_SYMLINK_MAX_SIZE);

  AsciiSymlinkTmp = AllocatePool (SymlinkSize + 1);
  if (AsciiSymlinkTmp == NULL) {
    DEBUG ((DEBUG_ERROR, "[ext4] Failed to allocate symlink ascii string buffer\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (AsciiSymlinkTmp, File->Inode->i_data, SymlinkSize);

  //
  // Add null-terminator
  //
  AsciiSymlinkTmp[SymlinkSize] = '\0';

  *AsciiSymlink     = AsciiSymlinkTmp;
  *AsciiSymlinkSize = SymlinkSize + 1;

  return EFI_SUCCESS;
}

/**
  Reads a slow symlink file.

  @param[in]      Partition        Pointer to the ext4 partition.
  @param[in]      File             Pointer to the open symlink file.
  @param[out]     AsciiSymlink     Pointer to the output ascii symlink string.
  @param[out]     AsciiSymlinkSize Pointer to the output ascii symlink string length.

  @retval EFI_SUCCESS           Slow symlink was read.
  @retval EFI_OUT_OF_RESOURCES  Memory allocation error.
  @retval EFI_INVALID_PARAMETER Slow symlink path has incorrect length
  @retval EFI_VOLUME_CORRUPTED  Symlink read block size differ from inode value
**/
STATIC
EFI_STATUS
Ext4ReadSlowSymlink (
  IN     EXT4_PARTITION  *Partition,
  IN     EXT4_FILE       *File,
  OUT    CHAR8           **AsciiSymlink,
  OUT    UINT32          *AsciiSymlinkSize
  )
{
  EFI_STATUS  Status;
  CHAR8       *SymlinkTmp;
  UINT64      SymlinkSizeTmp;
  UINT32      SymlinkAllocateSize;
  UINTN       ReadSize;

  SymlinkSizeTmp = EXT4_INODE_SIZE (File->Inode);

  //
  // Allocate EXT4_INODE_SIZE + 1
  //
  if (SymlinkSizeTmp >= EXT4_EFI_PATH_MAX) {
    DEBUG ((
      DEBUG_WARN,
      "[ext4] Warn: symlink path maximum length was hit!\n"
      ));
    return EFI_INVALID_PARAMETER;
  }

  SymlinkAllocateSize = (UINT32)SymlinkSizeTmp + 1;

  SymlinkTmp = AllocatePool (SymlinkAllocateSize);
  if (SymlinkTmp == NULL) {
    DEBUG ((DEBUG_FS, "[ext4] Failed to allocate symlink ascii string buffer\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  ReadSize = (UINTN)SymlinkSizeTmp;
  Status   = Ext4Read (Partition, File, SymlinkTmp, File->Position, &ReadSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_FS, "[ext4] Failed to read symlink from blocks with status %r\n", Status));
    FreePool (SymlinkTmp);
    return Status;
  }

  if (SymlinkSizeTmp != ReadSize) {
    DEBUG ((
      DEBUG_FS,
      "[ext4] Error! The size of the read block doesn't match the value from the inode!\n"
      ));
    FreePool (SymlinkTmp);
    return EFI_VOLUME_CORRUPTED;
  }

  //
  // Add null-terminator
  //
  SymlinkTmp[ReadSize] = '\0';

  *AsciiSymlinkSize = SymlinkAllocateSize;
  *AsciiSymlink     = SymlinkTmp;

  return EFI_SUCCESS;
}

/**
  Reads a symlink file.

  @param[in]      Partition   Pointer to the ext4 partition.
  @param[in]      File        Pointer to the open symlink file.
  @param[out]     Symlink     Pointer to the output unicode symlink string.

  @retval EFI_SUCCESS           Symlink was read.
  @retval EFI_ACCESS_DENIED     Symlink is encrypted.
  @retval EFI_OUT_OF_RESOURCES  Memory allocation error.
  @retval EFI_INVALID_PARAMETER Symlink path has incorrect length
  @retval EFI_VOLUME_CORRUPTED  Symlink read block size differ from inode value
**/
EFI_STATUS
Ext4ReadSymlink (
  IN     EXT4_PARTITION  *Partition,
  IN     EXT4_FILE       *File,
  OUT    CHAR16          **Symlink
  )
{
  EFI_STATUS  Status;
  CHAR8       *SymlinkTmp;
  UINT32      SymlinkSize;
  CHAR16      *Symlink16Tmp;
  CHAR16      *Needle;

  //
  // Assume that we already read Inode via Ext4ReadInode
  // Skip reading, just check encryption flag
  //
  if ((File->Inode->i_flags & EXT4_ENCRYPT_FL) != 0) {
    DEBUG ((DEBUG_WARN, "[ext4] Warn: symlink is encrypted\n"));
    return EFI_ACCESS_DENIED;
  }

  if (Ext4SymlinkIsFastSymlink (File)) {
    Status = Ext4ReadFastSymlink (Partition, File, &SymlinkTmp, &SymlinkSize);
  } else {
    Status = Ext4ReadSlowSymlink (Partition, File, &SymlinkTmp, &SymlinkSize);
  }

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_FS, "[ext4] Symlink read error with Status %r\n", Status));
    return Status;
  }

  Symlink16Tmp = AllocatePool (SymlinkSize * sizeof (CHAR16));
  if (Symlink16Tmp == NULL) {
    DEBUG ((DEBUG_FS, "[ext4] Failed to allocate symlink unicode string buffer\n"));
    FreePool (SymlinkTmp);
    return EFI_OUT_OF_RESOURCES;
  }

  Status = AsciiStrToUnicodeStrS (
             SymlinkTmp,
             Symlink16Tmp,
             SymlinkSize
             );

  FreePool (SymlinkTmp);

  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_FS,
      "[ext4] Failed to convert ascii symlink to unicode with Status %r\n",
      Status
      ));
    FreePool (Symlink16Tmp);
    return Status;
  }

  //
  // Convert to UEFI slashes
  //
  for (Needle = Symlink16Tmp; *Needle != L'\0'; Needle++) {
    if (*Needle == L'/') {
      *Needle = L'\\';
    }
  }

  *Symlink = Symlink16Tmp;

  return Status;
}
