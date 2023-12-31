/** @file
  Extent related routines

  Copyright (c) 2021 - 2023 Pedro Falcato All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Ext4Dxe.h"

/**
   Checks if the checksum of the extent data block is correct.
   @param[in]      ExtHeader     Pointer to the EXT4_EXTENT_HEADER.
   @param[in]      File          Pointer to the file.

   @return TRUE if the checksum is correct, FALSE if there is corruption.
**/
BOOLEAN
Ext4CheckExtentChecksum (
  IN CONST EXT4_EXTENT_HEADER  *ExtHeader,
  IN CONST EXT4_FILE           *File
  );

/**
   Calculates the checksum of the extent data block.
   @param[in]      ExtHeader     Pointer to the EXT4_EXTENT_HEADER.
   @param[in]      File          Pointer to the file.

   @return The checksum.
**/
UINT32
Ext4CalculateExtentChecksum (
  IN CONST EXT4_EXTENT_HEADER  *ExtHeader,
  IN CONST EXT4_FILE           *File
  );

/**
   Caches a range of extents, by allocating pool memory for each extent and adding it to the tree.

   @param[in]      File        Pointer to the open file.
   @param[in]      Extents     Pointer to an array of extents.
   @param[in]      NumberExtents Length of the array.

   @return         Result of the caching
**/
STATIC
EFI_STATUS
Ext4CacheExtents (
  IN EXT4_FILE          *File,
  IN CONST EXT4_EXTENT  *Extents,
  IN UINT16             NumberExtents
  );

/**
   Gets an extent from the extents cache of the file.

   @param[in]      File          Pointer to the open file.
   @param[in]      Block         Block we want to grab.

   @return Pointer to the extent, or NULL if it was not found.
**/
EXT4_EXTENT *
Ext4GetExtentFromMap (
  IN EXT4_FILE  *File,
  IN UINT32     Block
  );

/**
   Retrieves the pointer to the top of the extent tree.
   @param[in]      Inode         Pointer to the inode structure.

   @return Pointer to an EXT4_EXTENT_HEADER. This pointer is inside
           the inode and must not be freed.
**/
STATIC
EXT4_EXTENT_HEADER *
Ext4GetInoExtentHeader (
  IN EXT4_INODE  *Inode
  )
{
  return (EXT4_EXTENT_HEADER *)Inode->i_data;
}

/**
   Checks if an extent header is valid.
   @param[in]      Header         Pointer to the EXT4_EXTENT_HEADER structure.
   @param[in]      MaxEntries     Maximum number of entries possible for this tree node.

   @return TRUE if valid, FALSE if not.
**/
STATIC
BOOLEAN
Ext4ExtentHeaderValid (
  IN CONST EXT4_EXTENT_HEADER  *Header,
  IN UINT16                    MaxEntries
  )
{
  if (Header->eh_depth > EXT4_EXTENT_TREE_MAX_DEPTH) {
    DEBUG ((DEBUG_ERROR, "[ext4] Invalid extent header depth %u\n", Header->eh_depth));
    return FALSE;
  }

  if (Header->eh_magic != EXT4_EXTENT_HEADER_MAGIC) {
    DEBUG ((DEBUG_ERROR, "[ext4] Invalid extent header magic %x\n", Header->eh_magic));
    return FALSE;
  }

  // Note: We do not need to check eh_entries here, as the next branch makes sure max >= entries
  if (Header->eh_max > MaxEntries) {
    DEBUG ((
      DEBUG_ERROR,
      "[ext4] Invalid extent header max entries (%u eh_max, "
      "theoretical max is %u) (larger than permitted)\n",
      Header->eh_max,
      MaxEntries
      ));
    return FALSE;
  }

  if (Header->eh_max < Header->eh_entries) {
    DEBUG ((
      DEBUG_ERROR,
      "[ext4] Invalid extent header num entries %u max entries %u\n",
      Header->eh_entries,
      Header->eh_max
      ));
    return FALSE;
  }

  return TRUE;
}

/**
   Performs a binary search for a EXT4_EXTENT_INDEX that corresponds to a
   logical block in a given extent tree node.

   @param[in]      Header         Pointer to the EXT4_EXTENT_HEADER structure.
   @param[in]      LogicalBlock   Block that will be searched

   @return Pointer to the found EXT4_EXTENT_INDEX.
**/
STATIC
EXT4_EXTENT_INDEX *
Ext4BinsearchExtentIndex (
  IN EXT4_EXTENT_HEADER  *Header,
  IN EXT4_BLOCK_NR       LogicalBlock
  )
{
  EXT4_EXTENT_INDEX  *l;
  EXT4_EXTENT_INDEX  *r;
  EXT4_EXTENT_INDEX  *m;

  l = ((EXT4_EXTENT_INDEX *)(Header + 1)) + 1;
  r = ((EXT4_EXTENT_INDEX *)(Header + 1)) + Header->eh_entries - 1;

  // Perform a mostly-standard binary search on the array
  // This works very nicely because the extents arrays are always sorted.

  while (l <= r) {
    m = l + (r - l) / 2;

    if (LogicalBlock < m->ei_block) {
      r = m - 1;
    } else {
      l = m + 1;
    }
  }

  return l - 1;
}

/**
   Performs a binary search for a EXT4_EXTENT that corresponds to a
   logical block in a given extent tree node.

   @param[in]      Header         Pointer to the EXT4_EXTENT_HEADER structure.
   @param[in]      LogicalBlock   Block that will be searched

   @return Pointer to the found EXT4_EXTENT_INDEX, else NULL if the array is empty.
           Note: The caller must check if the logical block
           is actually mapped under the given extent.
**/
STATIC
EXT4_EXTENT *
Ext4BinsearchExtentExt (
  IN EXT4_EXTENT_HEADER  *Header,
  IN EXT4_BLOCK_NR       LogicalBlock
  )
{
  EXT4_EXTENT  *l;
  EXT4_EXTENT  *r;
  EXT4_EXTENT  *m;

  l = ((EXT4_EXTENT *)(Header + 1)) + 1;
  r = ((EXT4_EXTENT *)(Header + 1)) + Header->eh_entries - 1;
  // Perform a mostly-standard binary search on the array
  // This works very nicely because the extents arrays are always sorted.

  // Empty array
  if (Header->eh_entries == 0) {
    return NULL;
  }

  while (l <= r) {
    m = l + (r - l) / 2;

    if (LogicalBlock < m->ee_block) {
      r = m - 1;
    } else {
      l = m + 1;
    }
  }

  return l - 1;
}

/**
   Retrieves the leaf block from an EXT4_EXTENT_INDEX.

   @param[in]      Index          Pointer to the EXT4_EXTENT_INDEX structure.

   @return Block number of the leaf node.
**/
STATIC
EXT4_BLOCK_NR
Ext4ExtentIdxLeafBlock (
  IN EXT4_EXTENT_INDEX  *Index
  )
{
  return LShiftU64 (Index->ei_leaf_hi, 32) | Index->ei_leaf_lo;
}

// Results of sizeof(i_data) / sizeof(extent) - 1 = 4
#define EXT4_NR_INLINE_EXTENTS  4

/**
   Retrieves an extent from an EXT4 inode.
   @param[in]      Partition     Pointer to the opened EXT4 partition.
   @param[in]      File          Pointer to the opened file.
   @param[in]      LogicalBlock  Block number which the returned extent must cover.
   @param[out]     Extent        Pointer to the output buffer, where the extent will be copied to.

   @retval EFI_SUCCESS        Retrieval was successful.
   @retval EFI_NO_MAPPING     Block has no mapping.
**/
EFI_STATUS
Ext4GetExtent (
  IN  EXT4_PARTITION  *Partition,
  IN  EXT4_FILE       *File,
  IN  EXT4_BLOCK_NR   LogicalBlock,
  OUT EXT4_EXTENT     *Extent
  )
{
  EXT4_INODE          *Inode;
  VOID                *Buffer;
  EXT4_EXTENT         *Ext;
  UINT32              CurrentDepth;
  EXT4_EXTENT_HEADER  *ExtHeader;
  EXT4_EXTENT_INDEX   *Index;
  EFI_STATUS          Status;
  UINT16              MaxExtentsPerNode;
  EXT4_BLOCK_NR       BlockNumber;

  Inode  = File->Inode;
  Ext    = NULL;
  Buffer = NULL;

  DEBUG ((DEBUG_FS, "[ext4] Looking up extent for block %lu\n", LogicalBlock));

  // ext4 does not have support for logical block numbers bigger than UINT32_MAX
  if (LogicalBlock > (UINT32)-1) {
    return EFI_NO_MAPPING;
  }

  // Note: Right now, holes are the single biggest reason for cache misses
  // We should find a way to get (or cache) holes
  if ((Ext = Ext4GetExtentFromMap (File, (UINT32)LogicalBlock)) != NULL) {
    *Extent = *Ext;

    return EFI_SUCCESS;
  }

  if ((Inode->i_flags & EXT4_EXTENTS_FL) == 0) {
    // If this is an older ext2/ext3 filesystem, emulate Ext4GetExtent using the block map
    // By specification files using block maps are limited to 2^32 blocks,
    // so we can safely cast LogicalBlock to uint32
    Status = Ext4GetBlocks (Partition, File, (UINT32)LogicalBlock, Extent);

    if (!EFI_ERROR (Status)) {
      Status = Ext4CacheExtents (File, Extent, 1);

      if (EFI_ERROR (Status) && (Status != EFI_OUT_OF_RESOURCES)) {
        return Status;
      }

      Status = EFI_SUCCESS;
    }

    return Status;
  }

  // Slow path, we'll need to read from disk and (try to) cache those extents.

  ExtHeader = Ext4GetInoExtentHeader (Inode);

  if (!Ext4ExtentHeaderValid (ExtHeader, EXT4_NR_INLINE_EXTENTS)) {
    return EFI_VOLUME_CORRUPTED;
  }

  CurrentDepth = ExtHeader->eh_depth;

  // A single node fits into a single block, so we can only have (BlockSize / sizeof(EXT4_EXTENT)) - 1
  // extents in a single node. Note the -1, because both leaf and internal node headers are 12 bytes,
  // and so are individual entries.
  MaxExtentsPerNode = (UINT16)((Partition->BlockSize / sizeof (EXT4_EXTENT)) - 1);

  while (ExtHeader->eh_depth != 0) {
    CurrentDepth--;
    // While depth != 0, we're traversing the tree itself and not any leaves
    // As such, every entry is an EXT4_EXTENT_INDEX entry
    // Note: Entries after the extent header, either index or actual extent, are always sorted.
    // Therefore, we can use binary search, and it's actually the standard for doing so
    // (see FreeBSD).

    Index       = Ext4BinsearchExtentIndex (ExtHeader, LogicalBlock);
    BlockNumber = Ext4ExtentIdxLeafBlock (Index);

    // Check that block isn't file hole
    if (BlockNumber == EXT4_BLOCK_FILE_HOLE) {
      if (Buffer != NULL) {
        FreePool (Buffer);
      }

      return EFI_VOLUME_CORRUPTED;
    }

    if (Buffer == NULL) {
      Buffer = AllocatePool (Partition->BlockSize);
      if (Buffer == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
    }

    // Read the leaf block onto the previously-allocated buffer.

    Status = Ext4ReadBlocks (Partition, Buffer, 1, BlockNumber);
    if (EFI_ERROR (Status)) {
      FreePool (Buffer);
      return Status;
    }

    ExtHeader = Buffer;

    if (!Ext4ExtentHeaderValid (ExtHeader, MaxExtentsPerNode)) {
      FreePool (Buffer);
      return EFI_VOLUME_CORRUPTED;
    }

    if (!Ext4CheckExtentChecksum (ExtHeader, File)) {
      DEBUG ((DEBUG_ERROR, "[ext4] Invalid extent checksum\n"));
      FreePool (Buffer);
      return EFI_VOLUME_CORRUPTED;
    }

    if (ExtHeader->eh_depth != CurrentDepth) {
      FreePool (Buffer);
      return EFI_VOLUME_CORRUPTED;
    }
  }

  /* We try to cache every extent under a single leaf, since it's quite likely that we
   * may need to access things sequentially. Furthermore, ext4 block allocation as done
   * by linux (and possibly other systems) is quite fancy and usually it results in a small number of extents.
   * Therefore, we shouldn't have any memory issues.
  **/
  Status = Ext4CacheExtents (File, (EXT4_EXTENT *)(ExtHeader + 1), ExtHeader->eh_entries);

  if (EFI_ERROR (Status) && (Status != EFI_OUT_OF_RESOURCES)) {
    if (Buffer != NULL) {
      FreePool (Buffer);
    }

    return Status;
  }

  Ext = Ext4BinsearchExtentExt (ExtHeader, LogicalBlock);

  if (!Ext) {
    if (Buffer != NULL) {
      FreePool (Buffer);
    }

    return EFI_NO_MAPPING;
  }

  if (!((LogicalBlock >= Ext->ee_block) && (Ext->ee_block + Ext4GetExtentLength (Ext) > LogicalBlock))) {
    // This extent does not cover the block
    if (Buffer != NULL) {
      FreePool (Buffer);
    }

    return EFI_NO_MAPPING;
  }

  *Extent = *Ext;

  if (Buffer != NULL) {
    FreePool (Buffer);
  }

  return EFI_SUCCESS;
}

/**
  Compare two EXT4_EXTENT structs.
  Used in the extent map's ORDERED_COLLECTION.

  @param[in] UserStruct1  Pointer to the first user structure.

  @param[in] UserStruct2  Pointer to the second user structure.

  @retval <0  If UserStruct1 compares less than UserStruct2.

  @retval  0  If UserStruct1 compares equal to UserStruct2.

  @retval >0  If UserStruct1 compares greater than UserStruct2.
**/
STATIC
INTN
EFIAPI
Ext4ExtentsMapStructCompare (
  IN CONST VOID  *UserStruct1,
  IN CONST VOID  *UserStruct2
  )
{
  CONST EXT4_EXTENT  *Extent1;
  CONST EXT4_EXTENT  *Extent2;

  Extent1 = UserStruct1;
  Extent2 = UserStruct2;

  return Extent1->ee_block < Extent2->ee_block ? -1 :
         Extent1->ee_block > Extent2->ee_block ? 1 : 0;
}

/**
  Compare a standalone key against a EXT4_EXTENT containing an embedded key.
  Used in the extent map's ORDERED_COLLECTION.

  @param[in] StandaloneKey  Pointer to the bare key.

  @param[in] UserStruct     Pointer to the user structure with the embedded
                            key.

  @retval <0  If StandaloneKey compares less than UserStruct's key.

  @retval  0  If StandaloneKey compares equal to UserStruct's key.

  @retval >0  If StandaloneKey compares greater than UserStruct's key.
**/
STATIC
INTN
EFIAPI
Ext4ExtentsMapKeyCompare (
  IN CONST VOID  *StandaloneKey,
  IN CONST VOID  *UserStruct
  )
{
  CONST EXT4_EXTENT  *Extent;
  UINT32             Block;

  // Note that logical blocks are 32-bits in size so no truncation can happen here
  // with regards to 32-bit architectures.
  Extent = UserStruct;
  Block  = (UINT32)(UINTN)StandaloneKey;

  if ((Block >= Extent->ee_block) && (Block - Extent->ee_block < Ext4GetExtentLength (Extent))) {
    return 0;
  }

  return Block < Extent->ee_block ? -1 :
         Block > Extent->ee_block ? 1 : 0;
}

/**
   Initialises the (empty) extents map, that will work as a cache of extents.

   @param[in]      File        Pointer to the open file.

   @return Result of the operation.
**/
EFI_STATUS
Ext4InitExtentsMap (
  IN EXT4_FILE  *File
  )
{
  File->ExtentsMap = OrderedCollectionInit (Ext4ExtentsMapStructCompare, Ext4ExtentsMapKeyCompare);
  if (!File->ExtentsMap) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}

/**
   Frees the extents map, deleting every extent stored.

   @param[in]      File        Pointer to the open file.
**/
VOID
Ext4FreeExtentsMap (
  IN EXT4_FILE  *File
  )
{
  // Keep calling Min(), so we get an arbitrary node we can delete.
  // If Min() returns NULL, it's empty.

  ORDERED_COLLECTION_ENTRY  *MinEntry;
  EXT4_EXTENT               *Ext;

  MinEntry = NULL;

  while ((MinEntry = OrderedCollectionMin (File->ExtentsMap)) != NULL) {
    OrderedCollectionDelete (File->ExtentsMap, MinEntry, (VOID **)&Ext);
    FreePool (Ext);
  }

  ASSERT (OrderedCollectionIsEmpty (File->ExtentsMap));

  OrderedCollectionUninit (File->ExtentsMap);
  File->ExtentsMap = NULL;
}

/**
   Caches a range of extents, by allocating pool memory for each extent and adding it to the tree.

   @param[in]      File        Pointer to the open file.
   @param[in]      Extents     Pointer to an array of extents.
   @param[in]      NumberExtents Length of the array.

   @return         Result of the caching
**/
STATIC
EFI_STATUS
Ext4CacheExtents (
  IN EXT4_FILE          *File,
  IN CONST EXT4_EXTENT  *Extents,
  IN UINT16             NumberExtents
  )
{
  UINT16       Idx;
  EXT4_EXTENT  *Extent;
  EFI_STATUS   Status;

  /* Note that any out of memory condition might mean we don't get to cache a whole leaf of extents
   * in which case, future insertions might fail.
   */

  for (Idx = 0; Idx < NumberExtents; Idx++, Extents++) {
    if (Extents->ee_len == 0) {
      // 0-sized extent, must be corruption
      return EFI_VOLUME_CORRUPTED;
    }

    Extent = AllocatePool (sizeof (EXT4_EXTENT));

    if (Extent == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    CopyMem (Extent, Extents, sizeof (EXT4_EXTENT));
    Status = OrderedCollectionInsert (File->ExtentsMap, NULL, Extent);

    // EFI_ALREADY_STARTED = already exists in the tree.
    if (EFI_ERROR (Status)) {
      FreePool (Extent);

      if (Status == EFI_ALREADY_STARTED) {
        continue;
      }

      return EFI_SUCCESS;
    }
  }

  return EFI_SUCCESS;
}

/**
   Gets an extent from the extents cache of the file.

   @param[in]      File          Pointer to the open file.
   @param[in]      Block         Block we want to grab.

   @return Pointer to the extent, or NULL if it was not found.
**/
EXT4_EXTENT *
Ext4GetExtentFromMap (
  IN EXT4_FILE  *File,
  IN UINT32     Block
  )
{
  ORDERED_COLLECTION_ENTRY  *Entry;

  Entry = OrderedCollectionFind (File->ExtentsMap, (CONST VOID *)(UINTN)Block);

  if (Entry == NULL) {
    return NULL;
  }

  return OrderedCollectionUserStruct (Entry);
}

/**
   Calculates the checksum of the extent data block.
   @param[in]      ExtHeader     Pointer to the EXT4_EXTENT_HEADER.
   @param[in]      File          Pointer to the file.

   @return The checksum.
**/
UINT32
Ext4CalculateExtentChecksum (
  IN CONST EXT4_EXTENT_HEADER  *ExtHeader,
  IN CONST EXT4_FILE           *File
  )
{
  UINT32          Csum;
  EXT4_PARTITION  *Partition;
  EXT4_INODE      *Inode;

  Partition = File->Partition;
  Inode     = File->Inode;

  Csum = Ext4CalculateChecksum (Partition, &File->InodeNum, sizeof (EXT4_INO_NR), Partition->InitialSeed);
  Csum = Ext4CalculateChecksum (Partition, &Inode->i_generation, sizeof (Inode->i_generation), Csum);
  Csum = Ext4CalculateChecksum (Partition, ExtHeader, Partition->BlockSize - sizeof (EXT4_EXTENT_TAIL), Csum);

  return Csum;
}

/**
   Checks if the checksum of the extent data block is correct.
   @param[in]      ExtHeader     Pointer to the EXT4_EXTENT_HEADER.
   @param[in]      File          Pointer to the file.

   @return TRUE if the checksum is correct, FALSE if there is corruption.
**/
BOOLEAN
Ext4CheckExtentChecksum (
  IN CONST EXT4_EXTENT_HEADER  *ExtHeader,
  IN CONST EXT4_FILE           *File
  )
{
  EXT4_PARTITION    *Partition;
  EXT4_EXTENT_TAIL  *Tail;

  Partition = File->Partition;

  if (!EXT4_HAS_METADATA_CSUM (Partition)) {
    return TRUE;
  }

  Tail = (EXT4_EXTENT_TAIL *)((CONST CHAR8 *)ExtHeader + (Partition->BlockSize - 4));

  return Tail->eb_checksum == Ext4CalculateExtentChecksum (ExtHeader, File);
}

/**
   Retrieves the extent's length, dealing with uninitialized extents in the process.

   @param[in] Extent      Pointer to the EXT4_EXTENT

   @returns Extent's length, in filesystem blocks.
**/
EXT4_BLOCK_NR
Ext4GetExtentLength (
  IN CONST EXT4_EXTENT  *Extent
  )
{
  // If it's an uninitialized extent, the true length is ee_len - 2^15
  if (EXT4_EXTENT_IS_UNINITIALIZED (Extent)) {
    return Extent->ee_len - EXT4_EXTENT_MAX_INITIALIZED;
  }

  return Extent->ee_len;
}
