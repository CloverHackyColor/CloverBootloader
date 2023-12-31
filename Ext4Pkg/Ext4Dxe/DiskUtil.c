/** @file
  Disk utilities

  Copyright (c) 2021 Pedro Falcato All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include "Ext4Dxe.h"

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
  )
{
  return EXT4_DISK_IO (Partition)->ReadDisk (
                                     EXT4_DISK_IO (Partition),
                                     EXT4_MEDIA_ID (Partition),
                                     Offset,
                                     Length,
                                     Buffer
                                     );
}

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
  )
{
  UINT64  Offset;
  UINTN   Length;

  ASSERT (NumberBlocks != 0);
  ASSERT (BlockNumber != EXT4_BLOCK_FILE_HOLE);

  Offset = MultU64x32 (BlockNumber, Partition->BlockSize);
  Length = NumberBlocks * Partition->BlockSize;

  // Check for overflow on the block -> byte conversions.
  // Partition->BlockSize is never 0, so we don't need to check for that.

  if (DivU64x64Remainder (Offset, BlockNumber, NULL) != Partition->BlockSize) {
    return EFI_INVALID_PARAMETER;
  }

  if (Length / NumberBlocks != Partition->BlockSize) {
    return EFI_INVALID_PARAMETER;
  }

  return Ext4ReadDiskIo (Partition, Buffer, Length, Offset);
}

/**
   Allocates a buffer and reads blocks from the partition's disk using the DISK_IO protocol.
   This function is deprecated and will be removed in the future.

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
  )
{
  VOID   *Buf;
  UINTN  Length;

  // Check that number of blocks isn't empty, because
  // this is incorrect condition for opened partition,
  // so we just early-exit
  if ((NumberBlocks == 0) || (BlockNumber == EXT4_BLOCK_FILE_HOLE)) {
    return NULL;
  }

  Length = NumberBlocks * Partition->BlockSize;

  // Check for integer overflow
  if (Length / NumberBlocks != Partition->BlockSize) {
    return NULL;
  }

  Buf = AllocatePool (Length);
  if (Buf == NULL) {
    return NULL;
  }

  if (Ext4ReadBlocks (Partition, Buf, NumberBlocks, BlockNumber) != EFI_SUCCESS) {
    FreePool (Buf);
    return NULL;
  }

  return Buf;
}
