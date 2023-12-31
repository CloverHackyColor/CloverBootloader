/** @file
  Implementation of routines that deal with ext2/3 block maps.

  Copyright (c) 2022 Pedro Falcato All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Ext4Dxe.h>

// Note: The largest path we can take uses up 4 indices
#define EXT4_MAX_BLOCK_PATH  4

typedef enum ext4_logical_block_type {
  EXT4_TYPE_DIRECT_BLOCK = 0,
  EXT4_TYPE_SINGLY_BLOCK,
  EXT4_TYPE_DOUBLY_BLOCK,
  EXT4_TYPE_TREBLY_BLOCK,
  EXT4_TYPE_BAD_BLOCK
} EXT4_LOGICAL_BLOCK_TYPE;

/**
   @brief Detect the type of path the logical block will follow

   @param[in] LogicalBlock The logical block
   @param[in] Partition    Pointer to an EXT4_PARTITION
   @return The type of path the logical block will need to follow
 */
STATIC
EXT4_LOGICAL_BLOCK_TYPE
Ext4DetectBlockType (
  IN UINT32                LogicalBlock,
  IN CONST EXT4_PARTITION  *Partition
  )
{
  UINT32  Entries;
  UINT32  MinSinglyBlock;
  UINT32  MinDoublyBlock;
  UINT32  MinTreblyBlock;
  UINT32  MinQuadBlock;

  Entries        = (Partition->BlockSize / sizeof (UINT32));
  MinSinglyBlock = EXT4_DBLOCKS;
  MinDoublyBlock = Entries + MinSinglyBlock;
  MinTreblyBlock = Entries * Entries + MinDoublyBlock;
  MinQuadBlock   = Entries * Entries * Entries + MinTreblyBlock; // Doesn't actually exist

  if (LogicalBlock < MinSinglyBlock) {
    return EXT4_TYPE_DIRECT_BLOCK;
  } else if ((LogicalBlock >= MinSinglyBlock) && (LogicalBlock < MinDoublyBlock)) {
    return EXT4_TYPE_SINGLY_BLOCK;
  } else if ((LogicalBlock >= MinDoublyBlock) && (LogicalBlock < MinTreblyBlock)) {
    return EXT4_TYPE_DOUBLY_BLOCK;
  } else if (((LogicalBlock >= MinTreblyBlock) && (LogicalBlock < MinQuadBlock))) {
    return EXT4_TYPE_TREBLY_BLOCK;
  } else {
    return EXT4_TYPE_BAD_BLOCK;
  }
}

/**
   @brief Get a block's path in indices

   @param[in]  Partition       Pointer to an EXT4_PARTITION
   @param[in]  LogicalBlock    Logical block
   @param[out] BlockPath       Pointer to an array of EXT4_MAX_BLOCK_PATH elements, where the
                               indices we'll need to read are inserted.
   @return The number of path elements that are required (and were inserted in BlockPath)
 */
UINTN
Ext4GetBlockPath (
  IN  CONST EXT4_PARTITION  *Partition,
  IN  UINT32                LogicalBlock,
  OUT EXT2_BLOCK_NR         BlockPath[EXT4_MAX_BLOCK_PATH]
  )
{
  // The logic behind the block map is very much like a page table
  // Let's think of blocks with 512 entries (exactly like a page table on x64).
  // On doubly indirect block paths, we subtract the min doubly blocks from the logical block.
  // The top 9 bits of the result are the index inside the dind block, the bottom 9 bits are the
  // index inside the ind block. Since Entries is always a power of 2, entries - 1 will give us
  // a mask of the BlockMapBits.
  // Note that all this math could be done with ands and shifts (similar implementations exist
  // in a bunch of other places), but I'm doing it a simplified way with divs and modulus,
  // since it's not going to be a bottleneck anyway.

  UINT32  Entries;
  UINT32  EntriesEntries;
  UINT32  MinSinglyBlock;
  UINT32  MinDoublyBlock;
  UINT32  MinTreblyBlock;

  EXT4_LOGICAL_BLOCK_TYPE  Type;

  Entries        = (Partition->BlockSize / sizeof (UINT32));
  EntriesEntries = Entries * Entries;

  MinSinglyBlock = EXT4_DBLOCKS;
  MinDoublyBlock = Entries + MinSinglyBlock;
  MinTreblyBlock = EntriesEntries + MinDoublyBlock;

  Type = Ext4DetectBlockType (LogicalBlock, Partition);

  switch (Type) {
    case EXT4_TYPE_DIRECT_BLOCK:
      BlockPath[0] = LogicalBlock;
      break;
    case EXT4_TYPE_SINGLY_BLOCK:
      BlockPath[0] = EXT4_IND_BLOCK;
      BlockPath[1] = LogicalBlock - EXT4_DBLOCKS;
      break;
    case EXT4_TYPE_DOUBLY_BLOCK:
      BlockPath[0]  = EXT4_DIND_BLOCK;
      LogicalBlock -= MinDoublyBlock;
      BlockPath[1]  = LogicalBlock / Entries;
      BlockPath[2]  = LogicalBlock % Entries;
      break;
    case EXT4_TYPE_TREBLY_BLOCK:
      BlockPath[0]  = EXT4_DIND_BLOCK;
      LogicalBlock -= MinTreblyBlock;
      BlockPath[1]  = LogicalBlock / EntriesEntries;
      BlockPath[2]  = (LogicalBlock % EntriesEntries) / Entries;
      BlockPath[3]  = (LogicalBlock % EntriesEntries) % Entries;
      break;
    default:
      // EXT4_TYPE_BAD_BLOCK
      break;
  }

  return Type + 1;
}

/**
   @brief Get an extent from a block map
   Note: Also parses file holes and creates uninitialized extents from them.

   @param[in]  Buffer          Buffer of block pointers
   @param[in]  IndEntries      Number of entries in this block pointer table
   @param[in]  StartIndex      The start index from which we want to find a contiguous extent
   @param[out] Extent          Pointer to the resulting EXT4_EXTENT
 */
VOID
Ext4GetExtentInBlockMap (
  IN CONST UINT32  *Buffer,
  IN CONST UINT32  IndEntries,
  IN UINT32        StartIndex,
  OUT EXT4_EXTENT  *Extent
  )
{
  UINT32  Index;
  UINT32  FirstBlock;
  UINT32  LastBlock;
  UINT16  Count;

  Count      = 1;
  LastBlock  = Buffer[StartIndex];
  FirstBlock = LastBlock;

  if (FirstBlock == EXT4_BLOCK_FILE_HOLE) {
    // File hole, let's see how many blocks this hole spans
    Extent->ee_start_hi = 0;
    Extent->ee_start_lo = 0;

    for (Index = StartIndex + 1; Index < IndEntries; Index++) {
      if (Count == EXT4_EXTENT_MAX_INITIALIZED - 1) {
        // We've reached the max size of an uninit extent, break
        break;
      }

      if (Buffer[Index] == EXT4_BLOCK_FILE_HOLE) {
        Count++;
      } else {
        break;
      }
    }

    // We mark the extent as uninitialized, although there's a difference between uninit
    // extents and file holes.
    Extent->ee_len = EXT4_EXTENT_MAX_INITIALIZED + Count;
    return;
  }

  for (Index = StartIndex + 1; Index < IndEntries; Index++) {
    if (Count == EXT4_EXTENT_MAX_INITIALIZED) {
      // We've reached the max size of an extent, break
      break;
    }

    if ((Buffer[Index] == LastBlock + 1) && (Buffer[Index] != EXT4_BLOCK_FILE_HOLE)) {
      Count++;
    } else {
      break;
    }

    LastBlock = Buffer[Index];
  }

  Extent->ee_start_lo = FirstBlock;
  Extent->ee_start_hi = 0;
  Extent->ee_len      = Count;
}

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
  )
{
  EXT4_INODE     *Inode;
  EXT2_BLOCK_NR  BlockPath[EXT4_MAX_BLOCK_PATH];
  UINTN          BlockPathLength;
  UINTN          Index;
  UINT32         *Buffer;
  EFI_STATUS     Status;
  UINT32         Block;
  UINT32         BlockIndex;

  Inode = File->Inode;

  BlockPathLength = Ext4GetBlockPath (Partition, LogicalBlock, BlockPath);

  if (BlockPathLength - 1 == EXT4_TYPE_BAD_BLOCK) {
    // Bad logical block (out of range)
    return EFI_NO_MAPPING;
  }

  Extent->ee_block = LogicalBlock;

  if (BlockPathLength == 1) {
    // Fast path for blocks 0 - 12 that skips allocations
    Ext4GetExtentInBlockMap (Inode->i_data, EXT4_DBLOCKS, BlockPath[0], Extent);

    return EFI_SUCCESS;
  }

  Buffer = AllocatePool (Partition->BlockSize);
  if (Buffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  // Note the BlockPathLength - 1 so we don't end up reading the final block
  for (Index = 0; Index < BlockPathLength - 1; Index++) {
    BlockIndex = BlockPath[Index];

    if (Index == 0) {
      Block = Inode->i_data[BlockIndex];
    } else {
      Block = Buffer[BlockIndex];
    }

    if (Block == EXT4_BLOCK_FILE_HOLE) {
      FreePool (Buffer);
      return EFI_NO_MAPPING;
    }

    Status = Ext4ReadBlocks (Partition, Buffer, 1, Block);

    if (EFI_ERROR (Status)) {
      FreePool (Buffer);
      return Status;
    }
  }

  Ext4GetExtentInBlockMap (
    Buffer,
    Partition->BlockSize / sizeof (UINT32),
    BlockPath[BlockPathLength - 1],
    Extent
    );

  FreePool (Buffer);

  return EFI_SUCCESS;
}
