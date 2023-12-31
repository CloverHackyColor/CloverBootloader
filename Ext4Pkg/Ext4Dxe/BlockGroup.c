/** @file
  Block group related routines

  Copyright (c) 2021 Pedro Falcato All rights reserved.
  Copyright (c) 2007 - 2018, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Ext4Dxe.h"

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
  )
{
  // Maybe assert that the block group nr isn't a nonsense number?
  return (EXT4_BLOCK_GROUP_DESC *)((CHAR8 *)Partition->BlockGroups + BlockGroup * Partition->DescSize);
}

/**
   Reads an inode from disk.

   @param[in]    Partition  Pointer to the opened partition.
   @param[in]    InodeNum   Number of the desired Inode
   @param[out]   OutIno     Pointer to where it will be stored a pointer to the read inode.

   @return Status of the inode read.
**/
EFI_STATUS
Ext4ReadInode (
  IN EXT4_PARTITION  *Partition,
  IN EXT4_INO_NR     InodeNum,
  OUT EXT4_INODE     **OutIno
  )
{
  UINT64                 InodeOffset;
  UINT32                 BlockGroupNumber;
  EXT4_INODE             *Inode;
  EXT4_BLOCK_GROUP_DESC  *BlockGroup;
  EXT4_BLOCK_NR          InodeTableStart;
  EFI_STATUS             Status;

  if (!EXT4_IS_VALID_INODE_NR (Partition, InodeNum)) {
    DEBUG ((DEBUG_ERROR, "[ext4] Error reading inode: inode number %lu isn't valid\n", InodeNum));
    return EFI_VOLUME_CORRUPTED;
  }

  BlockGroupNumber = (UINT32)DivU64x64Remainder (
                               InodeNum - 1,
                               Partition->SuperBlock.s_inodes_per_group,
                               &InodeOffset
                               );

  // Check for the block group number's correctness
  if (BlockGroupNumber >= Partition->NumberBlockGroups) {
    return EFI_VOLUME_CORRUPTED;
  }

  Inode = Ext4AllocateInode (Partition);

  if (Inode == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  BlockGroup = Ext4GetBlockGroupDesc (Partition, BlockGroupNumber);

  // Note: We'll need to check INODE_UNINIT and friends when/if we add write support

  InodeTableStart = EXT4_BLOCK_NR_FROM_HALFS (
                      Partition,
                      BlockGroup->bg_inode_table_lo,
                      BlockGroup->bg_inode_table_hi
                      );

  Status = Ext4ReadDiskIo (
             Partition,
             Inode,
             Partition->InodeSize,
             EXT4_BLOCK_TO_BYTES (Partition, InodeTableStart) + MultU64x32 (InodeOffset, Partition->InodeSize)
             );

  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "[ext4] Error reading inode: status %x; inode offset %lx"
      " inode table start %lu block group %lu\n",
      Status,
      InodeOffset,
      InodeTableStart,
      BlockGroupNumber
      ));
    FreePool (Inode);
    return Status;
  }

  if (!Ext4CheckInodeChecksum (Partition, Inode, InodeNum)) {
    DEBUG ((
      DEBUG_ERROR,
      "[ext4] Inode %llu has invalid checksum (calculated %x)\n",
      InodeNum,
      Ext4CalculateInodeChecksum (Partition, Inode, InodeNum)
      ));
    FreePool (Inode);
    return EFI_VOLUME_CORRUPTED;
  }

  *OutIno = Inode;
  return EFI_SUCCESS;
}

/**
   Calculates the checksum of the block group descriptor for METADATA_CSUM enabled filesystems.
   @param[in]      Partition       Pointer to the opened EXT4 partition.
   @param[in]      BlockGroupDesc  Pointer to the block group descriptor.
   @param[in]      BlockGroupNum   Number of the block group.

   @return The checksum.
**/
STATIC
UINT16
Ext4CalculateBlockGroupDescChecksumMetadataCsum (
  IN CONST EXT4_PARTITION         *Partition,
  IN CONST EXT4_BLOCK_GROUP_DESC  *BlockGroupDesc,
  IN UINT32                       BlockGroupNum
  )
{
  UINT32  Csum;
  UINT16  Dummy;

  Dummy = 0;

  Csum = Ext4CalculateChecksum (Partition, &BlockGroupNum, sizeof (BlockGroupNum), Partition->InitialSeed);
  Csum = Ext4CalculateChecksum (Partition, BlockGroupDesc, OFFSET_OF (EXT4_BLOCK_GROUP_DESC, bg_checksum), Csum);
  Csum = Ext4CalculateChecksum (Partition, &Dummy, sizeof (Dummy), Csum);
  Csum =
    Ext4CalculateChecksum (
      Partition,
      &BlockGroupDesc->bg_block_bitmap_hi,
      Partition->DescSize - OFFSET_OF (EXT4_BLOCK_GROUP_DESC, bg_block_bitmap_hi),
      Csum
      );
  return (UINT16)Csum;
}

/**
   Calculates the checksum of the block group descriptor for GDT_CSUM enabled filesystems.
   @param[in]      Partition       Pointer to the opened EXT4 partition.
   @param[in]      BlockGroupDesc  Pointer to the block group descriptor.
   @param[in]      BlockGroupNum   Number of the block group.

   @return The checksum.
**/
STATIC
UINT16
Ext4CalculateBlockGroupDescChecksumGdtCsum (
  IN CONST EXT4_PARTITION         *Partition,
  IN CONST EXT4_BLOCK_GROUP_DESC  *BlockGroupDesc,
  IN UINT32                       BlockGroupNum
  )
{
  UINT16  Csum;

  Csum = CalculateCrc16Ansi (Partition->SuperBlock.s_uuid, sizeof (Partition->SuperBlock.s_uuid), 0xFFFF);
  Csum = CalculateCrc16Ansi (&BlockGroupNum, sizeof (BlockGroupNum), Csum);
  Csum = CalculateCrc16Ansi (BlockGroupDesc, OFFSET_OF (EXT4_BLOCK_GROUP_DESC, bg_checksum), Csum);
  Csum =
    CalculateCrc16Ansi (
      &BlockGroupDesc->bg_block_bitmap_hi,
      Partition->DescSize - OFFSET_OF (EXT4_BLOCK_GROUP_DESC, bg_block_bitmap_hi),
      Csum
      );
  return Csum;
}

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
  )
{
  if (!EXT4_HAS_METADATA_CSUM (Partition) && !EXT4_HAS_GDT_CSUM (Partition)) {
    return TRUE;
  }

  return Ext4CalculateBlockGroupDescChecksum (Partition, BlockGroupDesc, BlockGroupNum) == BlockGroupDesc->bg_checksum;
}

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
  )
{
  if (EXT4_HAS_METADATA_CSUM (Partition)) {
    return Ext4CalculateBlockGroupDescChecksumMetadataCsum (Partition, BlockGroupDesc, BlockGroupNum);
  } else if (EXT4_HAS_GDT_CSUM (Partition)) {
    return Ext4CalculateBlockGroupDescChecksumGdtCsum (Partition, BlockGroupDesc, BlockGroupNum);
  }

  return 0;
}
