/** @file
  Common header for the driver

  Copyright (c) 2021 - 2023 Pedro Falcato All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef EXT4_H_
#define EXT4_H_

#include <Uefi.h>

#include <Guid/FileInfo.h>
#include <Guid/FileSystemInfo.h>
#include <Guid/FileSystemVolumeLabelInfo.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DiskIo.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/SimpleFileSystem.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/OrderedCollectionLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include "Ext4Disk.h"

#define SYMLOOP_MAX  8
//
// We need to specify path length limit for security purposes, to prevent possible
// overflows and dead-loop conditions. Originally this limit is absent in FS design,
// but present in UNIX distros and shell environments, which may varies from 1024 to 4096.
//
#define EXT4_EFI_PATH_MAX    4096
#define EXT4_DRIVER_VERSION  0x0000

//
// The EXT4 Specification doesn't strictly limit block size and this value could be up to 2^31,
// but in practice it is limited by PAGE_SIZE due to performance significant impact.
// Many EXT4 implementations have size of block limited to PAGE_SIZE. In many cases it's limited
// to 4096, which is a commonly supported page size on most MMU-capable hardware, and up to 65536.
// So, to take a balance between compatibility and security measures, it is decided to use the
// value of 2MiB as the limit, which is equal to large page size on new hardware.
// As for supporting big block sizes, EXT4 has a RO_COMPAT_FEATURE called BIGALLOC, which changes
// EXT4 to use clustered allocation, so that each bit in the ext4 block allocation bitmap addresses
// a power of two number of blocks. So it would be wiser to implement and use this feature
// if there is such a need instead of big block size.
//
#define EXT4_LOG_BLOCK_SIZE_MAX  11

/**
   Opens an ext4 partition and installs the Simple File System protocol.

   @param[in]        DeviceHandle     Handle to the block device.
   @param[in]        DiskIo           Pointer to an EFI_DISK_IO_PROTOCOL.
   @param[in opt]    DiskIo2          Pointer to an EFI_DISK_IO2_PROTOCOL,
if supported.
   @param[in]        BlockIo          Pointer to an EFI_BLOCK_IO_PROTOCOL.

   @retval EFI_SUCCESS      The opening was successful.
           !EFI_SUCCESS     Opening failed.
**/
EFI_STATUS
Ext4OpenPartition (
  IN EFI_HANDLE                      DeviceHandle,
  IN EFI_DISK_IO_PROTOCOL            *DiskIo,
  IN OPTIONAL EFI_DISK_IO2_PROTOCOL  *DiskIo2,
  IN EFI_BLOCK_IO_PROTOCOL           *BlockIo
  );

typedef struct _Ext4File     EXT4_FILE;
typedef struct _Ext4_Dentry  EXT4_DENTRY;

typedef struct _Ext4_PARTITION {
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL    Interface;
  EFI_DISK_IO_PROTOCOL               *DiskIo;
  EFI_DISK_IO2_PROTOCOL              *DiskIo2;
  EFI_BLOCK_IO_PROTOCOL              *BlockIo;

  EXT4_SUPERBLOCK                    SuperBlock;
  BOOLEAN                            Unmounting;

  UINT32                             FeaturesIncompat;
  UINT32                             FeaturesCompat;
  UINT32                             FeaturesRoCompat;
  UINT32                             InodeSize;
  UINT32                             BlockSize;
  BOOLEAN                            ReadOnly;
  UINT64                             NumberBlockGroups;
  EXT4_BLOCK_NR                      NumberBlocks;

  EXT4_BLOCK_GROUP_DESC              *BlockGroups;
  UINT32                             DescSize;
  EXT4_FILE                          *Root;

  UINT32                             InitialSeed;

  LIST_ENTRY                         OpenFiles;

  EXT4_DENTRY                        *RootDentry;
} EXT4_PARTITION;

/**
   This structure represents a directory entry inside our directory entry tree.
   For now, it will be used as a way to track file names inside our opening
   code, but it may very well be used as a directory cache in the future.
   Because it's not being used as a directory cache right now,
   an EXT4_DENTRY structure is not necessarily unique name-wise in the list of
   children. Therefore, the dentry tree does not accurately reflect the
   filesystem structure.
 */
struct _Ext4_Dentry {
  UINTN                  RefCount;
  CHAR16                 Name[EXT4_NAME_MAX + 1];
  EXT4_INO_NR            Inode;
  struct _Ext4_Dentry    *Parent;
  LIST_ENTRY             Children;
  LIST_ENTRY             ListNode;
};

#define EXT4_DENTRY_FROM_DENTRY_LIST(Node)  BASE_CR(Node, EXT4_DENTRY, ListNode)

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
  IN OUT EXT4_DENTRY  *Parent OPTIONAL
  );

/**
   Increments the ref count of the dentry.

   @param[in out]            Dentry    Pointer to a valid EXT4_DENTRY.
**/
VOID
Ext4RefDentry (
  IN OUT EXT4_DENTRY  *Dentry
  );

/**
   Decrements the ref count of the dentry.
   If the ref count is 0, it's destroyed.

   @param[in out]            Dentry    Pointer to a valid EXT4_DENTRY.

   @retval True if it was destroyed, false if it's alive.
**/
BOOLEAN
Ext4UnrefDentry (
  IN OUT EXT4_DENTRY  *Dentry
  );

/**
   Opens and parses the superblock.

   @param[out]     Partition Partition structure to fill with filesystem
details.
   @retval EFI_SUCCESS       Parsing was successful and the partition is a
                             valid ext4 partition.
**/
EFI_STATUS
Ext4OpenSuperblock (
  OUT EXT4_PARTITION  *Partition
  );

/**
   Retrieves the EFI_BLOCK_IO_PROTOCOL of the partition.

   @param[in]     Partition  Pointer to the opened ext4 partition.
   @return The Block IO protocol associated with the partition.
**/
#define EXT4_BLOCK_IO(Partition)  Partition->BlockIo

/**
   Retrieves the EFI_DISK_IO_PROTOCOL of the partition.

   @param[in]     Partition  Pointer to the opened ext4 partition.
   @return The Disk IO protocol associated with the partition.
**/
#define EXT4_DISK_IO(Partition)  Partition->DiskIo

/**
   Retrieves the EFI_DISK_IO2_PROTOCOL of the partition.

   @param[in]     Partition  Pointer to the opened ext4 partition.
   @return The Disk IO 2 protocol associated with the partition, or NULL if
           not supported.
**/
#define EXT4_DISK_IO2(Partition)  Partition->DiskIo2

/**
   Retrieves the media ID of the partition.

   @param[in]     Partition  Pointer to the opened ext4 partition.
   @return The media ID associated with the partition.
**/
#define EXT4_MEDIA_ID(Partition)  Partition->BlockIo->Media->MediaId

/**
   Reads from the partition's disk using the DISK_IO protocol.

   @param[in]  Partition      Pointer to the opened ext4 partition.
   @param[out] Buffer         Pointer to a destination buffer.
   @param[in]  Length         Length of the destination buffer.
   @param[in]  Offset         Offset, in bytes, of the location to read.

   @return Success status of the disk read.
**/
EFI_STATUS
Ext4ReadDiskIo (
  IN EXT4_PARTITION  *Partition,
  OUT VOID           *Buffer,
  IN UINTN           Length,
  IN UINT64          Offset
  );

/**
   Reads blocks from the partition's disk using the DISK_IO protocol.

   @param[in]  Partition      Pointer to the opened ext4 partition.
   @param[out] Buffer         Pointer to a destination buffer.
   @param[in]  NumberBlocks   Length of the read, in filesystem blocks.
   @param[in]  BlockNumber    Starting block number.

   @return Success status of the read.
**/
EFI_STATUS
Ext4ReadBlocks (
  IN EXT4_PARTITION  *Partition,
  OUT VOID           *Buffer,
  IN UINTN           NumberBlocks,
  IN EXT4_BLOCK_NR   BlockNumber
  );

/**
   Allocates a buffer and reads blocks from the partition's disk using the
DISK_IO protocol. This function is deprecated and will be removed in the future.

   @param[in]  Partition      Pointer to the opened ext4 partition.
   @param[in]  NumberBlocks   Length of the read, in filesystem blocks.
   @param[in]  BlockNumber    Starting block number.

   @return Buffer allocated by AllocatePool, or NULL if some part of the process
           failed.
**/
VOID *
Ext4AllocAndReadBlocks (
  IN EXT4_PARTITION  *Partition,
  IN UINTN           NumberBlocks,
  IN EXT4_BLOCK_NR   BlockNumber
  );

/**
   Checks if the opened partition has the 64-bit feature (see
EXT4_FEATURE_INCOMPAT_64BIT).

   @param[in]  Partition      Pointer to the opened ext4 partition.

   @return TRUE if EXT4_FEATURE_INCOMPAT_64BIT is enabled, else FALSE.
**/
#define EXT4_IS_64_BIT(Partition)                                              \
  ((Partition->FeaturesIncompat & EXT4_FEATURE_INCOMPAT_64BIT) != 0)

/**
   Composes an EXT4_BLOCK_NR safely, from two halfs.

   @param[in]  Partition      Pointer to the opened ext4 partition.
   @param[in]  Low            Low half of the block number.
   @param[in]  High           High half of the block number.

   @return The block number formed by Low, and if 64 bit is enabled, High.
**/
#define EXT4_BLOCK_NR_FROM_HALFS(Partition, Low, High)                         \
  EXT4_IS_64_BIT(Partition) ? (Low | LShiftU64(High, 32)) : Low

/**
   Retrieves a block group descriptor of the ext4 filesystem.

   @param[in]  Partition      Pointer to the opened ext4 partition.
   @param[in]  BlockGroup    Block group number.

   @return A pointer to the block group descriptor.
**/
EXT4_BLOCK_GROUP_DESC *
Ext4GetBlockGroupDesc (
  IN EXT4_PARTITION  *Partition,
  IN UINT32          BlockGroup
  );

/**
   Checks inode number validity across superblock of the opened partition.

   @param[in]  Partition      Pointer to the opened ext4 partition.

   @return TRUE if inode number is valid.
**/
#define EXT4_IS_VALID_INODE_NR(Partition, InodeNum)                            \
  (((InodeNum) > 0) && (InodeNum) <= (Partition->SuperBlock.s_inodes_count))

/**
   Reads an inode from disk.

   @param[in]    Partition  Pointer to the opened partition.
   @param[in]    InodeNum   Number of the desired Inode
   @param[out]   OutIno     Pointer to where it will be stored a pointer to the
read inode.

   @return Status of the inode read.
**/
EFI_STATUS
Ext4ReadInode (
  IN EXT4_PARTITION  *Partition,
  IN EXT4_INO_NR     InodeNum,
  OUT EXT4_INODE     **OutIno
  );

/**
   Converts blocks to bytes.

   @param[in]    Partition  Pointer to the opened partition.
   @param[in]    Block      Block number/number of blocks.

   @return The number of bytes.
**/
#define EXT4_BLOCK_TO_BYTES(Partition, Block)                                  \
  MultU64x32(Block, Partition->BlockSize)

/**
   Reads from an EXT4 inode.
   @param[in]      Partition     Pointer to the opened EXT4 partition.
   @param[in]      File          Pointer to the opened file.
   @param[out]     Buffer        Pointer to the buffer.
   @param[in]      Offset        Offset of the read.
   @param[in out]  Length        Pointer to the length of the buffer, in bytes.
                                 After a successful read, it's updated to the
number of read bytes.

   @return Status of the read operation.
**/
EFI_STATUS
Ext4Read (
  IN     EXT4_PARTITION  *Partition,
  IN     EXT4_FILE       *File,
  OUT    VOID            *Buffer,
  IN     UINT64          Offset,
  IN OUT UINTN           *Length
  );

/**
   Retrieves the size of the inode.

   @param[in]    Inode      Pointer to the ext4 inode.

   @return The size of the inode, in bytes.
**/
#define EXT4_INODE_SIZE(Inode)                                                 \
  (LShiftU64(Inode->i_size_hi, 32) | Inode->i_size_lo)

/**
   Retrieves an extent from an EXT4 inode.
   @param[in]      Partition     Pointer to the opened EXT4 partition.
   @param[in]      File          Pointer to the opened file.
   @param[in]      LogicalBlock  Block number which the returned extent must
cover.
   @param[out]     Extent        Pointer to the output buffer, where the extent
will be copied to.

   @retval EFI_SUCCESS        Retrieval was successful.
   @retval EFI_NO_MAPPING     Block has no mapping.
**/
EFI_STATUS
Ext4GetExtent (
  IN EXT4_PARTITION  *Partition,
  IN EXT4_FILE       *File,
  IN EXT4_BLOCK_NR   LogicalBlock,
  OUT EXT4_EXTENT    *Extent
  );

struct _Ext4File {
  EFI_FILE_PROTOCOL     Protocol;
  EXT4_INODE            *Inode;
  EXT4_INO_NR           InodeNum;

  UINT64                OpenMode;
  UINT64                Position;
  UINT32                SymLoops;

  EXT4_PARTITION        *Partition;

  ORDERED_COLLECTION    *ExtentsMap;

  LIST_ENTRY            OpenFilesListNode;

  // Owning reference to this file's directory entry.
  EXT4_DENTRY           *Dentry;
};

#define EXT4_FILE_FROM_THIS(This)  BASE_CR ((This), EXT4_FILE, Protocol)

#define EXT4_FILE_FROM_OPEN_FILES_NODE(Node)                                   \
  BASE_CR(Node, EXT4_FILE, OpenFilesListNode)

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
  IN CONST CHAR16     *NameUnicode,
  IN EXT4_PARTITION   *Partition,
  OUT EXT4_DIR_ENTRY  *Result
  );

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
  IN EXT4_FILE       *Directory,
  IN CONST CHAR16    *Name,
  IN EXT4_PARTITION  *Partition,
  IN UINT64          OpenMode,
  OUT EXT4_FILE      **OutFile
  );

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
  IN EXT4_PARTITION  *Partition,
  IN UINT64          OpenMode,
  OUT EXT4_FILE      **OutFile,
  IN EXT4_DIR_ENTRY  *Entry,
  IN EXT4_FILE       *Directory
  );

/**
   Allocates a zeroed inode structure.
   @param[in]      Partition     Pointer to the opened EXT4 partition.

   @return Pointer to the allocated structure, from the pool,
           with size Partition->InodeSize.
**/
EXT4_INODE *
Ext4AllocateInode (
  IN EXT4_PARTITION  *Partition
  );

// Part of the EFI_SIMPLE_FILE_SYSTEM_PROTOCOL

/**
  Open the root directory on a volume.

  @param[in]   This A pointer to the volume to open the root directory.
  @param[out]  Root A pointer to the location to return the opened file handle
for the root directory.

  @retval EFI_SUCCESS          The device was opened.
  @retval EFI_UNSUPPORTED      This volume does not support the requested file
system type.
  @retval EFI_NO_MEDIA         The device has no medium.
  @retval EFI_DEVICE_ERROR     The device reported an error.
  @retval EFI_VOLUME_CORRUPTED The file system structures are corrupted.
  @retval EFI_ACCESS_DENIED    The service denied access to the file.
  @retval EFI_OUT_OF_RESOURCES The volume was not opened due to lack of
resources.
  @retval EFI_MEDIA_CHANGED    The device has a different medium in it or the
medium is no longer supported. Any existing file handles for this volume are no
longer valid. To access the files on the new medium, the volume must be reopened
with OpenVolume().

**/
EFI_STATUS
EFIAPI
Ext4OpenVolume (
  IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *This,
  IN EFI_FILE_PROTOCOL                **Root
  );

// End of EFI_SIMPLE_FILE_SYSTEM_PROTOCOL

/**
   Sets up the protocol and metadata of a file that is being opened.

   @param[in out]        File        Pointer to the file.
   @param[in]            Partition   Pointer to the opened partition.
**/
VOID
Ext4SetupFile (
  IN OUT EXT4_FILE   *File,
  IN EXT4_PARTITION  *Partition
  );

/**
  Opens a new file relative to the source file's location.

  @param[out] FoundFile  A pointer to the location to return the opened handle for the new
                         file.
  @param[in]  Source     A pointer to the EXT4_FILE instance that is the file
                         handle to the source location. This would typically be an open
                         handle to a directory.
  @param[in]  FileName   The Null-terminated string of the name of the file to be opened.
                         The file name may contain the following path modifiers: "\", ".",
                         and "..".
  @param[in]  OpenMode   The mode to open the file. The only valid combinations that the
                         file may be opened with are: Read, Read/Write, or Create/Read/Write.
  @param[in]  Attributes Only valid for EFI_FILE_MODE_CREATE, in which case these are the
                         attribute bits for the newly created file.

  @retval EFI_SUCCESS          The file was opened.
  @retval EFI_NOT_FOUND        The specified file could not be found on the device.
  @retval EFI_NO_MEDIA         The device has no medium.
  @retval EFI_MEDIA_CHANGED    The device has a different medium in it or the medium is no
                               longer supported.
  @retval EFI_DEVICE_ERROR     The device reported an error.
  @retval EFI_VOLUME_CORRUPTED The file system structures are corrupted.
  @retval EFI_WRITE_PROTECTED  An attempt was made to create a file, or open a file for write
                               when the media is write-protected.
  @retval EFI_ACCESS_DENIED    The service denied access to the file.
  @retval EFI_OUT_OF_RESOURCES Not enough resources were available to open the file.
  @retval EFI_VOLUME_FULL      The volume is full.

**/
EFI_STATUS
Ext4OpenInternal (
  OUT EXT4_FILE  **FoundFile,
  IN  EXT4_FILE  *Source,
  IN  CHAR16     *FileName,
  IN  UINT64     OpenMode,
  IN  UINT64     Attributes
  );

/**
   Closes a file.

   @param[in]        File        Pointer to the file.

   @return Status of the closing of the file.
**/
EFI_STATUS
Ext4CloseInternal (
  IN EXT4_FILE  *File
  );

// Part of the EFI_FILE_PROTOCOL

/**
  Opens a new file relative to the source file's location.

  @param[in]  This       A pointer to the EFI_FILE_PROTOCOL instance that is the
file handle to the source location. This would typically be an open handle to a
directory.
  @param[out] NewHandle  A pointer to the location to return the opened handle
for the new file.
  @param[in]  FileName   The Null-terminated string of the name of the file to
be opened. The file name may contain the following path modifiers: "\", ".", and
"..".
  @param[in]  OpenMode   The mode to open the file. The only valid combinations
that the file may be opened with are: Read, Read/Write, or Create/Read/Write.
  @param[in]  Attributes Only valid for EFI_FILE_MODE_CREATE, in which case
these are the attribute bits for the newly created file.

  @retval EFI_SUCCESS          The file was opened.
  @retval EFI_NOT_FOUND        The specified file could not be found on the
device.
  @retval EFI_NO_MEDIA         The device has no medium.
  @retval EFI_MEDIA_CHANGED    The device has a different medium in it or the
medium is no longer supported.
  @retval EFI_DEVICE_ERROR     The device reported an error.
  @retval EFI_VOLUME_CORRUPTED The file system structures are corrupted.
  @retval EFI_WRITE_PROTECTED  An attempt was made to create a file, or open a
file for write when the media is write-protected.
  @retval EFI_ACCESS_DENIED    The service denied access to the file.
  @retval EFI_OUT_OF_RESOURCES Not enough resources were available to open the
file.
  @retval EFI_VOLUME_FULL      The volume is full.

**/
EFI_STATUS
EFIAPI
Ext4Open (
  IN EFI_FILE_PROTOCOL   *This,
  OUT EFI_FILE_PROTOCOL  **NewHandle,
  IN CHAR16              *FileName,
  IN UINT64              OpenMode,
  IN UINT64              Attributes
  );

/**
  Closes a specified file handle.

  @param[in]  This          A pointer to the EFI_FILE_PROTOCOL instance that is
the file handle to close.

  @retval EFI_SUCCESS   The file was closed.

**/
EFI_STATUS
EFIAPI
Ext4Close (
  IN EFI_FILE_PROTOCOL  *This
  );

/**
  Close and delete the file handle.

  @param[in]  This                     A pointer to the EFI_FILE_PROTOCOL
instance that is the handle to the file to delete.

  @retval EFI_SUCCESS              The file was closed and deleted, and the
handle was closed.
  @retval EFI_WARN_DELETE_FAILURE  The handle was closed, but the file was not
deleted.

**/
EFI_STATUS
EFIAPI
Ext4Delete (
  IN EFI_FILE_PROTOCOL  *This
  );

/**
  Reads data from a file.

  @param[in]      This             A pointer to the EFI_FILE_PROTOCOL instance
that is the file handle to read data from.
  @param[in out]  BufferSize       On input, the size of the Buffer. On output,
the amount of data returned in Buffer. In both cases, the size is measured in
bytes.
  @param[out]     Buffer           The buffer into which the data is read.

  @retval EFI_SUCCESS          Data was read.
  @retval EFI_NO_MEDIA         The device has no medium.
  @retval EFI_DEVICE_ERROR     The device reported an error.
  @retval EFI_DEVICE_ERROR     An attempt was made to read from a deleted file.
  @retval EFI_DEVICE_ERROR     On entry, the current file position is beyond the
end of the file.
  @retval EFI_VOLUME_CORRUPTED The file system structures are corrupted.
  @retval EFI_BUFFER_TOO_SMALL The BufferSize is too small to read the current
directory entry. BufferSize has been updated with the size needed to complete
the request.

**/
EFI_STATUS
EFIAPI
Ext4ReadFile (
  IN EFI_FILE_PROTOCOL  *This,
  IN OUT UINTN          *BufferSize,
  OUT VOID              *Buffer
  );

/**
  Writes data to a file.

  @param[in]      This        A pointer to the EFI_FILE_PROTOCOL instance that
is the file handle to write data to.
  @param[in out]  BufferSize  On input, the size of the Buffer. On output, the
amount of data actually written. In both cases, the size is measured in bytes.
  @param[in]      Buffer      The buffer of data to write.

  @retval EFI_SUCCESS          Data was written.
  @retval EFI_UNSUPPORTED      Writes to open directory files are not supported.
  @retval EFI_NO_MEDIA         The device has no medium.
  @retval EFI_DEVICE_ERROR     The device reported an error.
  @retval EFI_DEVICE_ERROR     An attempt was made to write to a deleted file.
  @retval EFI_VOLUME_CORRUPTED The file system structures are corrupted.
  @retval EFI_WRITE_PROTECTED  The file or medium is write-protected.
  @retval EFI_ACCESS_DENIED    The file was opened read only.
  @retval EFI_VOLUME_FULL      The volume is full.

**/
EFI_STATUS
EFIAPI
Ext4WriteFile (
  IN EFI_FILE_PROTOCOL  *This,
  IN OUT UINTN          *BufferSize,
  IN VOID               *Buffer
  );

/**
  Returns a file's current position.

  @param[in]   This            A pointer to the EFI_FILE_PROTOCOL instance that
is the file handle to get the current position on.
  @param[out]  Position        The address to return the file's current position
value.

  @retval EFI_SUCCESS      The position was returned.
  @retval EFI_UNSUPPORTED  The request is not valid on open directories.
  @retval EFI_DEVICE_ERROR An attempt was made to get the position from a
deleted file.

**/
EFI_STATUS
EFIAPI
Ext4GetPosition (
  IN EFI_FILE_PROTOCOL  *This,
  OUT UINT64            *Position
  );

/**
  Sets a file's current position.

  @param[in]  This            A pointer to the EFI_FILE_PROTOCOL instance that
is the file handle to set the requested position on.
  @param[in] Position        The byte position from the start of the file to
set.

  @retval EFI_SUCCESS      The position was set.
  @retval EFI_UNSUPPORTED  The seek request for nonzero is not valid on open
                           directories.
  @retval EFI_DEVICE_ERROR An attempt was made to set the position of a deleted
file.

**/
EFI_STATUS
EFIAPI
Ext4SetPosition (
  IN EFI_FILE_PROTOCOL  *This,
  IN UINT64             Position
  );

/**
  Returns information about a file.

  @param[in]      This            A pointer to the EFI_FILE_PROTOCOL instance
that is the file handle the requested information is for.
  @param[in]      InformationType The type identifier for the information being
requested.
  @param[in out]  BufferSize      On input, the size of Buffer. On output, the
amount of data returned in Buffer. In both cases, the size is measured in bytes.
  @param[out]     Buffer          A pointer to the data buffer to return. The
buffer's type is indicated by InformationType.

  @retval EFI_SUCCESS          The information was returned.
  @retval EFI_UNSUPPORTED      The InformationType is not known.
  @retval EFI_NO_MEDIA         The device has no medium.
  @retval EFI_DEVICE_ERROR     The device reported an error.
  @retval EFI_VOLUME_CORRUPTED The file system structures are corrupted.
  @retval EFI_BUFFER_TOO_SMALL The BufferSize is too small to read the current
directory entry. BufferSize has been updated with the size needed to complete
                               the request.
**/
EFI_STATUS
EFIAPI
Ext4GetInfo (
  IN EFI_FILE_PROTOCOL  *This,
  IN EFI_GUID           *InformationType,
  IN OUT UINTN          *BufferSize,
  OUT VOID              *Buffer
  );

/**
  Sets information about a file.

  @param[in]  This            A pointer to the EFI_FILE_PROTOCOL instance that
is the file handle the information is for.
  @param[in]  InformationType The type identifier for the information being set.
  @param[in]  BufferSize      The size, in bytes, of Buffer.
  @param[in]  Buffer          A pointer to the data buffer to write. The
buffer's type is indicated by InformationType.

  @retval EFI_SUCCESS          The information was set.
  @retval EFI_UNSUPPORTED      The InformationType is not known.
  @retval EFI_NO_MEDIA         The device has no medium.
  @retval EFI_DEVICE_ERROR     The device reported an error.
  @retval EFI_VOLUME_CORRUPTED The file system structures are corrupted.
  @retval EFI_WRITE_PROTECTED  InformationType is EFI_FILE_INFO_ID and the media
is read-only.
  @retval EFI_WRITE_PROTECTED  InformationType is
EFI_FILE_PROTOCOL_SYSTEM_INFO_ID and the media is read only.
  @retval EFI_WRITE_PROTECTED  InformationType is
EFI_FILE_SYSTEM_VOLUME_LABEL_ID and the media is read-only.
  @retval EFI_ACCESS_DENIED    An attempt is made to change the name of a file
to a file that is already present.
  @retval EFI_ACCESS_DENIED    An attempt is being made to change the
EFI_FILE_DIRECTORY Attribute.
  @retval EFI_ACCESS_DENIED    An attempt is being made to change the size of a
directory.
  @retval EFI_ACCESS_DENIED    InformationType is EFI_FILE_INFO_ID and the file
was opened read-only and an attempt is being made to modify a field other than
Attribute.
  @retval EFI_VOLUME_FULL      The volume is full.
  @retval EFI_BAD_BUFFER_SIZE  BufferSize is smaller than the size of the type
indicated by InformationType.

**/
EFI_STATUS
EFIAPI
Ext4SetInfo (
  IN EFI_FILE_PROTOCOL  *This,
  IN EFI_GUID           *InformationType,
  IN UINTN              BufferSize,
  IN VOID               *Buffer
  );

// EFI_FILE_PROTOCOL implementation ends here.

/**
   Checks if a file is a directory.
   @param[in]      File          Pointer to the opened file.

   @return TRUE if file is a directory.
**/
BOOLEAN
Ext4FileIsDir (
  IN CONST EXT4_FILE  *File
  );

/**
   Checks if a file is a symlink.

   @param[in]      File          Pointer to the opened file.

   @return BOOLEAN         Whether file is a symlink
**/
BOOLEAN
Ext4FileIsSymlink (
  IN CONST EXT4_FILE  *File
  );

/**
   Checks if a file is a regular file.
   @param[in]      File          Pointer to the opened file.

   @return BOOLEAN         TRUE if file is a regular file.
**/
BOOLEAN
Ext4FileIsReg (
  IN CONST EXT4_FILE  *File
  );

// In EFI we can't open FIFO pipes, UNIX sockets, character/block devices since
// these concepts are at the kernel level and are OS dependent.

/**
   Checks if a file is openable.
   @param[in]      File    Pointer to the file trying to be opened.


   @return TRUE if file is openable. A file is considered openable if
           it's a regular file or a directory, since most other file types
           don't make sense under UEFI.
**/
#define Ext4FileIsOpenable(File)  (Ext4FileIsReg (File) || Ext4FileIsDir (File) || Ext4FileIsSymlink (File))

#define EXT4_INODE_HAS_FIELD(Inode, Field)                                     \
  (Inode->i_extra_isize + EXT4_GOOD_OLD_INODE_SIZE >=                          \
   OFFSET_OF(EXT4_INODE, Field) + sizeof(((EXT4_INODE *)NULL)->Field))

/**
   Calculates the physical space used by a file.
   @param[in]      File          Pointer to the opened file.

   @return Physical space used by a file, in bytes.
**/
UINT64
Ext4FilePhysicalSpace (
  IN EXT4_FILE  *File
  );

/**
   Gets the file's last access time.
   @param[in]      File   Pointer to the opened file.
   @param[out]     Time   Pointer to an EFI_TIME structure.
**/
VOID
Ext4FileATime (
  IN EXT4_FILE  *File,
  OUT EFI_TIME  *Time
  );

/**
   Gets the file's last (data) modification time.
   @param[in]      File   Pointer to the opened file.
   @param[out]     Time   Pointer to an EFI_TIME structure.
**/
VOID
Ext4FileMTime (
  IN EXT4_FILE  *File,
  OUT EFI_TIME  *Time
  );

/**
   Gets the file's creation time, if possible.
   @param[in]      File   Pointer to the opened file.
   @param[out]     Time   Pointer to an EFI_TIME structure.
                          In the case where the the creation time isn't
recorded, Time is zeroed.
**/
VOID
Ext4FileCreateTime (
  IN EXT4_FILE  *File,
  OUT EFI_TIME  *Time
  );

/**
   Initialises Unicode collation, which is needed for case-insensitive string
comparisons within the driver (a good example of an application of this is
filename comparison).

   @param[in]      DriverHandle    Handle to the driver image.

   @retval EFI_SUCCESS   Unicode collation was successfully initialised.
   @retval !EFI_SUCCESS  Failure.
**/
EFI_STATUS
Ext4InitialiseUnicodeCollation (
  EFI_HANDLE  DriverHandle
  );

/**
   Does a case-insensitive string comparison. Refer to
EFI_UNICODE_COLLATION_PROTOCOL's StriColl for more details.

   @param[in]      Str1   Pointer to a null terminated string.
   @param[in]      Str2   Pointer to a null terminated string.

   @retval 0   Str1 is equivalent to Str2.
   @retval >0  Str1 is lexically greater than Str2.
   @retval <0  Str1 is lexically less than Str2.
**/
INTN
Ext4StrCmpInsensitive (
  IN CHAR16  *Str1,
  IN CHAR16  *Str2
  );

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
  );

/**
   Retrieves information about the file and stores it in the EFI_FILE_INFO
format.

   @param[in]      File           Pointer to an opened file.
   @param[out]     Info           Pointer to a EFI_FILE_INFO.
   @param[in out]  BufferSize     Pointer to the buffer size

   @return Status of the file information request.
**/
EFI_STATUS
Ext4GetFileInfo (
  IN EXT4_FILE       *File,
  OUT EFI_FILE_INFO  *Info,
  IN OUT UINTN       *BufferSize
  );

/**
   Reads a directory entry.

   @param[in]      Partition   Pointer to the ext4 partition.
   @param[in]      File        Pointer to the open directory.
   @param[out]     Buffer      Pointer to the output buffer.
   @param[in]      Offset      Initial directory position.
   @param[in out] OutLength    Pointer to a UINTN that contains the length of
the buffer, and the length of the actual EFI_FILE_INFO after the call.

   @return Result of the operation.
**/
EFI_STATUS
Ext4ReadDir (
  IN EXT4_PARTITION  *Partition,
  IN EXT4_FILE       *File,
  OUT VOID           *Buffer,
  IN UINT64          Offset,
  IN OUT UINTN       *OutLength
  );

/**
   Initialises the (empty) extents map, that will work as a cache of extents.

   @param[in]      File        Pointer to the open file.

   @return Result of the operation.
**/
EFI_STATUS
Ext4InitExtentsMap (
  IN EXT4_FILE  *File
  );

/**
   Frees the extents map, deleting every extent stored.

   @param[in]      File        Pointer to the open file.
**/
VOID
Ext4FreeExtentsMap (
  IN EXT4_FILE  *File
  );

/**
   Calculates the checksum of the given buffer.
   @param[in]      Partition     Pointer to the opened EXT4 partition.
   @param[in]      Buffer        Pointer to the buffer.
   @param[in]      Length        Length of the buffer, in bytes.
   @param[in]      InitialValue  Initial value of the CRC.

   @return The checksum.
**/
UINT32
Ext4CalculateChecksum (
  IN CONST EXT4_PARTITION  *Partition,
  IN CONST VOID            *Buffer,
  IN UINTN                 Length,
  IN UINT32                InitialValue
  );

/**
   Calculates the checksum of the given inode.
   @param[in]      Partition     Pointer to the opened EXT4 partition.
   @param[in]      Inode         Pointer to the inode.
   @param[in]      InodeNum      Inode number.

   @return The checksum.
**/
UINT32
Ext4CalculateInodeChecksum (
  IN CONST EXT4_PARTITION  *Partition,
  IN CONST EXT4_INODE      *Inode,
  IN EXT4_INO_NR           InodeNum
  );

/**
   Checks if the checksum of the inode is correct.
   @param[in]      Partition     Pointer to the opened EXT4 partition.
   @param[in]      Inode         Pointer to the inode.
   @param[in]      InodeNum      Inode number.

   @return TRUE if checksum is correct, FALSE if there is corruption.
**/
BOOLEAN
Ext4CheckInodeChecksum (
  IN CONST EXT4_PARTITION  *Partition,
  IN CONST EXT4_INODE      *Inode,
  IN EXT4_INO_NR           InodeNum
  );

/**
   Unmounts and frees an ext4 partition.

   @param[in]        Partition        Pointer to the opened partition.

   @return Status of the unmount.
**/
EFI_STATUS
Ext4UnmountAndFreePartition (
  IN EXT4_PARTITION  *Partition
  );

/**
   Checks if the checksum of the block group descriptor is correct.
   @param[in]      Partition       Pointer to the opened EXT4 partition.
   @param[in]      BlockGroupDesc  Pointer to the block group descriptor.
   @param[in]      BlockGroupNum   Number of the block group.

   @return TRUE if checksum is correct, FALSE if there is corruption.
**/
BOOLEAN
Ext4VerifyBlockGroupDescChecksum (
  IN CONST EXT4_PARTITION         *Partition,
  IN CONST EXT4_BLOCK_GROUP_DESC  *BlockGroupDesc,
  IN UINT32                       BlockGroupNum
  );

/**
   Calculates the checksum of the block group descriptor.
   @param[in]      Partition       Pointer to the opened EXT4 partition.
   @param[in]      BlockGroupDesc  Pointer to the block group descriptor.
   @param[in]      BlockGroupNum   Number of the block group.

   @return The checksum.
**/
UINT16
Ext4CalculateBlockGroupDescChecksum (
  IN CONST EXT4_PARTITION         *Partition,
  IN CONST EXT4_BLOCK_GROUP_DESC  *BlockGroupDesc,
  IN UINT32                       BlockGroupNum
  );

/**
   Verifies the existence of a particular RO compat feature set.
   @param[in]      Partition           Pointer to the opened EXT4 partition.
   @param[in]      RoCompatFeatureSet  Feature set to test.

   @return TRUE if all features are supported, else FALSE.
**/
#define EXT4_HAS_RO_COMPAT(Partition, RoCompatFeatureSet)                      \
  ((Partition->FeaturesRoCompat & RoCompatFeatureSet) == RoCompatFeatureSet)

/**
   Verifies the existence of a particular compat feature set.
   @param[in]      Partition           Pointer to the opened EXT4 partition.
   @param[in]      CompatFeatureSet  Feature set to test.

   @return TRUE if all features are supported, else FALSE.
**/
#define EXT4_HAS_COMPAT(Partition, CompatFeatureSet)                           \
  ((Partition->FeaturesCompat & CompatFeatureSet) == CompatFeatureSet)

/**
   Verifies the existence of a particular compat feature set.
   @param[in]      Partition           Pointer to the opened EXT4 partition.
   @param[in]      IncompatFeatureSet  Feature set to test.

   @return TRUE if all features are supported, else FALSE.
**/
#define EXT4_HAS_INCOMPAT(Partition, IncompatFeatureSet)                       \
  ((Partition->FeaturesIncompat & IncompatFeatureSet) == IncompatFeatureSet)

// Note: Might be a good idea to provide generic Ext4Has$feature() through
// macros.

/**
   Checks if metadata_csum is enabled on the partition.
   @param[in]      Partition           Pointer to the opened EXT4 partition.

   @return TRUE if the metadata_csum is supported, else FALSE.
**/
#define EXT4_HAS_METADATA_CSUM(Partition)                                      \
  EXT4_HAS_RO_COMPAT(Partition, EXT4_FEATURE_RO_COMPAT_METADATA_CSUM)

/**
   Checks if gdt_csum is enabled on the partition.
   @param[in]      Partition           Pointer to the opened EXT4 partition.

   @return TRUE if the gdt_csum is supported, else FALSE.
**/
#define EXT4_HAS_GDT_CSUM(Partition)                                           \
  EXT4_HAS_RO_COMPAT(Partition, EXT4_FEATURE_RO_COMPAT_GDT_CSUM)

/**
   Retrieves the volume name.

   @param[in]      Part           Pointer to the opened partition.
   @param[out]     Info           Pointer to a CHAR16*.
   @param[out]     BufferSize     Pointer to a UINTN, where the string length
                                  of the name will be put.

   @return Status of the volume name request.
**/
EFI_STATUS
Ext4GetVolumeName (
  IN EXT4_PARTITION  *Partition,
  OUT CHAR16         **OutVolName,
  OUT UINTN          *VolNameLen
  );

/**
   Checks the superblock's magic value.

   @param[in] DiskIo      Pointer to the DiskIo.
   @param[in] BlockIo     Pointer to the BlockIo.

   @returns Whether the partition has a valid EXT4 superblock magic value.
**/
BOOLEAN
Ext4SuperblockCheckMagic (
  IN EFI_DISK_IO_PROTOCOL   *DiskIo,
  IN EFI_BLOCK_IO_PROTOCOL  *BlockIo
  );

/**
   Check if the extent is uninitialized

   @param[in] Extent    Pointer to the EXT4_EXTENT

   @returns True if uninitialized, else false.
**/
#define EXT4_EXTENT_IS_UNINITIALIZED(Extent)                                   \
  ((Extent)->ee_len > EXT4_EXTENT_MAX_INITIALIZED)

/**
   Retrieves the extent's length, dealing with uninitialized extents in the
process.

   @param[in] Extent      Pointer to the EXT4_EXTENT

   @returns Extent's length, in filesystem blocks.
**/
EXT4_BLOCK_NR
Ext4GetExtentLength (
  IN CONST EXT4_EXTENT  *Extent
  );

/**
   Retrieves an extent from an EXT2/3 inode (with a blockmap).
   @param[in]      Partition     Pointer to the opened EXT4 partition.
   @param[in]      File          Pointer to the opened file.
   @param[in]      LogicalBlock  Block number which the returned extent must cover.
   @param[out]     Extent        Pointer to the output buffer, where the extent will be copied to.

   @retval EFI_SUCCESS        Retrieval was successful.
   @retval EFI_NO_MAPPING     Block has no mapping.
**/
EFI_STATUS
Ext4GetBlocks (
  IN  EXT4_PARTITION  *Partition,
  IN  EXT4_FILE       *File,
  IN  EXT2_BLOCK_NR   LogicalBlock,
  OUT EXT4_EXTENT     *Extent
  );

#endif
