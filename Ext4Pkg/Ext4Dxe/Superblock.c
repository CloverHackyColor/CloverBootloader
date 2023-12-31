/** @file
  Superblock managing routines

  Copyright (c) 2021 - 2023 Pedro Falcato All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Ext4Dxe.h"

STATIC CONST UINT32  gSupportedCompatFeat = EXT4_FEATURE_COMPAT_EXT_ATTR;

STATIC CONST UINT32  gSupportedRoCompatFeat =
  EXT4_FEATURE_RO_COMPAT_DIR_NLINK | EXT4_FEATURE_RO_COMPAT_EXTRA_ISIZE |
  EXT4_FEATURE_RO_COMPAT_HUGE_FILE | EXT4_FEATURE_RO_COMPAT_LARGE_FILE |
  EXT4_FEATURE_RO_COMPAT_GDT_CSUM | EXT4_FEATURE_RO_COMPAT_METADATA_CSUM | EXT4_FEATURE_RO_COMPAT_SPARSE_SUPER;

STATIC CONST UINT32  gSupportedIncompatFeat =
  EXT4_FEATURE_INCOMPAT_64BIT | EXT4_FEATURE_INCOMPAT_DIRDATA |
  EXT4_FEATURE_INCOMPAT_FLEX_BG | EXT4_FEATURE_INCOMPAT_FILETYPE |
  EXT4_FEATURE_INCOMPAT_EXTENTS | EXT4_FEATURE_INCOMPAT_LARGEDIR |
  EXT4_FEATURE_INCOMPAT_MMP | EXT4_FEATURE_INCOMPAT_RECOVER | EXT4_FEATURE_INCOMPAT_CSUM_SEED;

// Future features that may be nice additions in the future:
// 1) Btree support: Required for write support and would speed up lookups in large directories.
// 2) meta_bg: Required to mount meta_bg-enabled partitions.

// Note: We ignore MMP because it's impossible that it's mapped elsewhere,
// I think (unless there's some sort of network setup where we're accessing a remote partition).

// Note on corruption signaling:
// We (Ext4Dxe) could signal corruption by setting s_state to |= EXT4_FS_STATE_ERRORS_DETECTED.
// I've decided against that, because right now the driver is read-only, and
// that would mean we would need to writeback the superblock. If something like
// this is desired, it's fairly trivial to look for EFI_VOLUME_CORRUPTED
// references and add some Ext4SignalCorruption function + function call.

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
  )
{
  UINT16      Magic;
  EFI_STATUS  Status;

  Status = DiskIo->ReadDisk (
                     DiskIo,
                     BlockIo->Media->MediaId,
                     EXT4_SUPERBLOCK_OFFSET + OFFSET_OF (EXT4_SUPERBLOCK, s_magic),
                     sizeof (Magic),
                     &Magic
                     );
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  if (Magic != EXT4_SIGNATURE) {
    return FALSE;
  }

  return TRUE;
}

/**
   Does brief validation of the ext4 superblock.

   @param[in] Sb     Pointer to the read superblock.

   @return TRUE if a valid ext4 superblock, else FALSE.
**/
BOOLEAN
Ext4SuperblockValidate (
  CONST EXT4_SUPERBLOCK  *Sb
  )
{
  if (Sb->s_magic != EXT4_SIGNATURE) {
    return FALSE;
  }

  if ((Sb->s_rev_level != EXT4_DYNAMIC_REV) && (Sb->s_rev_level != EXT4_GOOD_OLD_REV)) {
    return FALSE;
  }

  if ((Sb->s_state & EXT4_FS_STATE_UNMOUNTED) == 0) {
    DEBUG ((DEBUG_WARN, "[ext4] Filesystem was not unmounted cleanly\n"));
  }

  return TRUE;
}

/**
   Calculates the superblock's checksum.

   @param[in] Partition    Pointer to the opened partition.
   @param[in] Sb           Pointer to the superblock.

   @return The superblock's checksum.
**/
STATIC
UINT32
Ext4CalculateSuperblockChecksum (
  EXT4_PARTITION         *Partition,
  CONST EXT4_SUPERBLOCK  *Sb
  )
{
  // Most checksums require us to go through a dummy 0 as part of the requirement
  // that the checksum is done over a structure with its checksum field = 0.
  UINT32  Checksum;

  Checksum = Ext4CalculateChecksum (
               Partition,
               Sb,
               OFFSET_OF (EXT4_SUPERBLOCK, s_checksum),
               ~0U
               );

  return Checksum;
}

/**
   Verifies that the superblock's checksum is valid.

   @param[in] Partition    Pointer to the opened partition.
   @param[in] Sb           Pointer to the superblock.

   @return The superblock's checksum.
**/
STATIC
BOOLEAN
Ext4VerifySuperblockChecksum (
  EXT4_PARTITION         *Partition,
  CONST EXT4_SUPERBLOCK  *Sb
  )
{
  if (!EXT4_HAS_METADATA_CSUM (Partition)) {
    return TRUE;
  }

  return Sb->s_checksum == Ext4CalculateSuperblockChecksum (Partition, Sb);
}

/**
   Opens and parses the superblock.

   @param[out]     Partition Partition structure to fill with filesystem details.
   @retval EFI_SUCCESS       Parsing was successful and the partition is a
                             valid ext4 partition.
**/
EFI_STATUS
Ext4OpenSuperblock (
  OUT EXT4_PARTITION  *Partition
  )
{
  UINT32                 Index;
  EFI_STATUS             Status;
  EXT4_SUPERBLOCK        *Sb;
  UINT32                 NrBlocksRem;
  UINTN                  NrBlocks;
  UINT32                 UnsupportedRoCompat;
  EXT4_BLOCK_GROUP_DESC  *Desc;

  Status = Ext4ReadDiskIo (
             Partition,
             &Partition->SuperBlock,
             sizeof (EXT4_SUPERBLOCK),
             EXT4_SUPERBLOCK_OFFSET
             );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Sb = &Partition->SuperBlock;

  if (!Ext4SuperblockValidate (Sb)) {
    return EFI_VOLUME_CORRUPTED;
  }

  if (Sb->s_rev_level == EXT4_DYNAMIC_REV) {
    Partition->FeaturesCompat   = Sb->s_feature_compat;
    Partition->FeaturesIncompat = Sb->s_feature_incompat;
    Partition->FeaturesRoCompat = Sb->s_feature_ro_compat;
    Partition->InodeSize        = Sb->s_inode_size;

    // Check for proper alignment of InodeSize and that InodeSize is indeed larger than
    // the minimum size, 128 bytes.
    if (((Partition->InodeSize % 4) != 0) || (Partition->InodeSize < EXT4_GOOD_OLD_INODE_SIZE)) {
      return EFI_VOLUME_CORRUPTED;
    }
  } else {
    // GOOD_OLD_REV
    Partition->FeaturesCompat = Partition->FeaturesIncompat = Partition->FeaturesRoCompat = 0;
    Partition->InodeSize      = EXT4_GOOD_OLD_INODE_SIZE;
  }

  // Now, check for the feature set of the filesystem
  // It's essential to check for this to avoid filesystem corruption and to avoid
  // accidentally opening an ext2/3/4 filesystem we don't understand, which would be disastrous.

  if (Partition->FeaturesIncompat & ~gSupportedIncompatFeat) {
    DEBUG ((
      DEBUG_ERROR,
      "[ext4] Unsupported features %lx\n",
      Partition->FeaturesIncompat & ~gSupportedIncompatFeat
      ));
    return EFI_UNSUPPORTED;
  }

  if (EXT4_HAS_INCOMPAT (Partition, EXT4_FEATURE_INCOMPAT_RECOVER)) {
    DEBUG ((DEBUG_WARN, "[ext4] Needs journal recovery, mounting read-only\n"));
    Partition->ReadOnly = TRUE;
  }

  // At the time of writing, it's the only supported checksum.
  if (EXT4_HAS_METADATA_CSUM (Partition) && (Sb->s_checksum_type != EXT4_CHECKSUM_CRC32C)) {
    return EFI_UNSUPPORTED;
  }

  if (EXT4_HAS_INCOMPAT (Partition, EXT4_FEATURE_INCOMPAT_CSUM_SEED)) {
    Partition->InitialSeed = Sb->s_checksum_seed;
  } else {
    Partition->InitialSeed = Ext4CalculateChecksum (Partition, Sb->s_uuid, 16, ~0U);
  }

  UnsupportedRoCompat = Partition->FeaturesRoCompat & ~gSupportedRoCompatFeat;

  if (UnsupportedRoCompat != 0) {
    DEBUG ((DEBUG_WARN, "[ext4] Unsupported ro compat %x\n", UnsupportedRoCompat));
    Partition->ReadOnly = TRUE;
  }

  // gSupportedCompatFeat is documentation-only since we never need to access it.
  // The line below avoids unused variable warnings.
  (VOID)gSupportedCompatFeat;

  DEBUG ((DEBUG_FS, "Read only = %u\n", Partition->ReadOnly));

  if (Sb->s_inodes_per_group == 0) {
    DEBUG ((DEBUG_ERROR, "[ext4] Inodes per group can not be zero\n"));
    return EFI_VOLUME_CORRUPTED;
  }

  if (Sb->s_log_block_size > EXT4_LOG_BLOCK_SIZE_MAX) {
    DEBUG ((DEBUG_ERROR, "[ext4] SuperBlock s_log_block_size %lu is too big\n", Sb->s_log_block_size));
    return EFI_UNSUPPORTED;
  }

  Partition->BlockSize = (UINT32)LShiftU64 (1024, Sb->s_log_block_size);

  // The size of a block group can also be calculated as 8 * Partition->BlockSize
  if (Sb->s_blocks_per_group != 8 * Partition->BlockSize) {
    return EFI_UNSUPPORTED;
  }

  Partition->NumberBlocks      = EXT4_BLOCK_NR_FROM_HALFS (Partition, Sb->s_blocks_count, Sb->s_blocks_count_hi);
  Partition->NumberBlockGroups = DivU64x32 (Partition->NumberBlocks, Sb->s_blocks_per_group);

  DEBUG ((
    DEBUG_FS,
    "[ext4] Number of blocks = %lu\n[ext4] Number of block groups: %lu\n",
    Partition->NumberBlocks,
    Partition->NumberBlockGroups
    ));

  if (EXT4_IS_64_BIT (Partition)) {
    // s_desc_size should be 4 byte aligned and
    // 64 bit filesystems need DescSize to be 64 bytes
    if (((Sb->s_desc_size % 4) != 0) || (Sb->s_desc_size < EXT4_64BIT_BLOCK_DESC_SIZE)) {
      return EFI_VOLUME_CORRUPTED;
    }

    Partition->DescSize = Sb->s_desc_size;
  } else {
    Partition->DescSize = EXT4_OLD_BLOCK_DESC_SIZE;
  }

  if (!Ext4VerifySuperblockChecksum (Partition, Sb)) {
    DEBUG ((DEBUG_ERROR, "[ext4] Bad superblock checksum %lx\n", Ext4CalculateSuperblockChecksum (Partition, Sb)));
    return EFI_VOLUME_CORRUPTED;
  }

  NrBlocks = (UINTN)DivU64x32Remainder (
                      MultU64x32 (Partition->NumberBlockGroups, Partition->DescSize),
                      Partition->BlockSize,
                      &NrBlocksRem
                      );

  if (NrBlocksRem != 0) {
    NrBlocks++;
  }

  Partition->BlockGroups = Ext4AllocAndReadBlocks (Partition, NrBlocks, Partition->BlockSize == 1024 ? 2 : 1);

  if (Partition->BlockGroups == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  for (Index = 0; Index < Partition->NumberBlockGroups; Index++) {
    Desc = Ext4GetBlockGroupDesc (Partition, Index);
    if (!Ext4VerifyBlockGroupDescChecksum (Partition, Desc, Index)) {
      DEBUG ((DEBUG_ERROR, "[ext4] Block group descriptor %u has an invalid checksum\n", Index));
      FreePool (Partition->BlockGroups);
      return EFI_VOLUME_CORRUPTED;
    }
  }

  // RootDentry will serve as the basis of our directory entry tree.
  Partition->RootDentry = Ext4CreateDentry (L"\\", NULL);

  if (Partition->RootDentry == NULL) {
    FreePool (Partition->BlockGroups);
    return EFI_OUT_OF_RESOURCES;
  }

  // Note that the cast below is completely safe, because EXT4_FILE is a specialization of EFI_FILE_PROTOCOL
  Status = Ext4OpenVolume (&Partition->Interface, (EFI_FILE_PROTOCOL **)&Partition->Root);

  if (EFI_ERROR (Status)) {
    Ext4UnrefDentry (Partition->RootDentry);
    FreePool (Partition->BlockGroups);
  }

  return Status;
}

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
  )
{
  if (!EXT4_HAS_METADATA_CSUM (Partition)) {
    return 0;
  }

  switch (Partition->SuperBlock.s_checksum_type) {
    case EXT4_CHECKSUM_CRC32C:
      // For some reason, EXT4 really likes non-inverted CRC32C checksums, so we stick to that here.
      return ~CalculateCrc32c(Buffer, Length, ~InitialValue);
    default:
      ASSERT (FALSE);
      return 0;
  }
}
