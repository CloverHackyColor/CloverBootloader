/** @file
  Decode an Apple formatted partition table

  Copyright (c) 2009-2010, Oracle Corporation
**/


#include "Partition.h"

#define DPISTRLEN       32

#pragma pack(1)
typedef struct APPLE_PT_HEADER {
    UINT16  	sbSig;		/* must be BE 0x4552 */
    UINT16  	sbBlkSize;	/* block size of device */
    UINT32  	sbBlkCount;	/* number of blocks on device */
    UINT16  	sbDevType;	/* device type */
    UINT16  	sbDevId;	/* device id */
    UINT32  	sbData;		/* not used */
    UINT16  	sbDrvrCount;	/* driver descriptor count */
    UINT16  	sbMap[247];	/* descriptor map */
} APPLE_PT_HEADER;

typedef struct APPLE_PT_ENTRY  {
    UINT16       signature          ; /* must be BE 0x504D for new style PT */
    UINT16       reserved_1         ;
    UINT32       map_entries        ; /* how many PT entries are there */
    UINT32       pblock_start       ; /* first physical block */
    UINT32       pblocks            ; /* number of physical blocks */
    char         name[DPISTRLEN]    ; /* name of partition */
    char         type[DPISTRLEN]    ; /* type of partition */
    /* Some more data we don't really need */
} APPLE_PT_ENTRY;
#pragma pack()

/**
  Install child handles if the Handle supports Apple partition table format.

  @param[in]  This        Calling context.
  @param[in]  Handle      Parent Handle
  @param[in]  DiskIo      Parent DiskIo interface
  @param[in]  BlockIo     Parent BlockIo interface
  @param[in]  DevicePath  Parent Device Path


  @retval EFI_SUCCESS         Child handle(s) was added
  @retval EFI_MEDIA_CHANGED   Media changed Detected
  @retval other               no child handle was added

**/
EFI_STATUS
PartitionInstallAppleChildHandles (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  EFI_DISK_IO_PROTOCOL         *DiskIo,
  IN  EFI_DISK_IO2_PROTOCOL        *DiskIo2,
  IN  EFI_BLOCK_IO_PROTOCOL        *BlockIo,
  IN  EFI_BLOCK_IO2_PROTOCOL       *BlockIo2,
  IN  EFI_DEVICE_PATH_PROTOCOL     *DevicePath
  )
{
  EFI_STATUS                Status;
  UINT32                    Lba;
  EFI_BLOCK_IO_MEDIA       *Media;
  VOID                     *Block;
  //UINTN                   MaxIndex;
  /** @todo: wrong, as this PT can be on both HDD or CD */
  CDROM_DEVICE_PATH         CdDev;
  //EFI_DEVICE_PATH_PROTOCOL  Dev;
  EFI_STATUS                Found;
  UINT32                    Partition;
  UINT32                    PartitionEntries;
  UINT32                    VolSpaceSize;
  UINT32                    SubBlockSize;
  UINT32                    BlkPerSec;
  UINT32                    MediaId;
  UINT32                    BlockSize;
  EFI_LBA                   LastBlock;
  EFI_DISK_IO2_TOKEN        DiskIo2Token;

  Found         = EFI_NOT_FOUND;
  VolSpaceSize  = 0;

  if (BlockIo2 != NULL)
  {
    Media         = BlockIo2->Media;
    BlockSize     = BlockIo2->Media->BlockSize;
    LastBlock     = BlockIo2->Media->LastBlock;
    MediaId       = BlockIo2->Media->MediaId;
  } else {
    Media         = BlockIo->Media;
    BlockSize     = BlockIo->Media->BlockSize;
    LastBlock     = BlockIo->Media->LastBlock;
    MediaId       = BlockIo->Media->MediaId;
  }

  Block = AllocatePool ((UINTN)BlockSize);

  if (Block == NULL) {
    return EFI_NOT_FOUND;
  }

  do {
      APPLE_PT_HEADER * Header;

      /* read PT header first */
      Lba = 0;

      if (DiskIo2 != NULL)
      {
        Status = DiskIo2->ReadDiskEx (
                       DiskIo2,
                       MediaId,
                       MultU64x32 (Lba, BlockSize),
                       &DiskIo2Token,
                       BlockSize,
                       Block
                       );
      } else {
        Status = DiskIo->ReadDisk (
                       DiskIo,
                       MediaId,
                       MultU64x32 (Lba, BlockSize),
                       BlockSize,
                       Block
                       );
      }
      if (EFI_ERROR(Status)) {
          Found = Status;
          break;
      }

      Header = (APPLE_PT_HEADER *)Block;
      if (SwapBytes16(Header->sbSig) != 0x4552) {
          break;
      }
      SubBlockSize = SwapBytes16(Header->sbBlkSize);
      BlkPerSec    = BlockSize / SubBlockSize;

      /* Fail if media block size isn't an exact multiple */
      if (BlockSize != SubBlockSize * BlkPerSec) {
          break;
      }

      /* Now iterate over PT entries and install child handles */
      PartitionEntries = 1;
      for (Partition = 1; Partition <= PartitionEntries; Partition++) {
          APPLE_PT_ENTRY * Entry;
          UINT32 StartLba;
          UINT32 SizeLbs;

          if (DiskIo2 != NULL)
          {
            Status = DiskIo2->ReadDiskEx (
                       DiskIo2,
                       MediaId,
                       MultU64x32 (Partition, SubBlockSize),
                       &DiskIo2Token,
                       SubBlockSize,
                       Block
                       );
          } else {
            Status = DiskIo->ReadDisk (
                       DiskIo,
                       MediaId,
                       MultU64x32 (Partition, SubBlockSize),
                       SubBlockSize,
                       Block
                       );
          }

          if (EFI_ERROR(Status)) {
              Status = EFI_NOT_FOUND;
              goto done; /* would break, but ... */
          }

          Entry = (APPLE_PT_ENTRY *)Block;

          if (SwapBytes16(Entry->signature) != 0x504D) {
              Print(L"Not a new PT entry: %x", Entry->signature);
              continue;
          }

          /* First partition contains partitions count */
          if (Partition == 1) {
             PartitionEntries  = SwapBytes32(Entry->map_entries);
          }

          StartLba = SwapBytes32(Entry->pblock_start);
          SizeLbs  = SwapBytes32(Entry->pblocks);

   /*       if (0 && CompareMem("Apple_HFS", Entry->type, 10) == 0)
              Print(L"HFS partition (%d of %d) at LBA 0x%x size=%dM\n",
                    Partition, PartitionEntries, StartLba,
                    (UINT32)(MultU64x32(SizeLbs, SubBlockSize) / (1024 * 1024)));
    */
      //
      // Create child device handle
      //

          ZeroMem (&CdDev, sizeof (CdDev));
          CdDev.Header.Type     = MEDIA_DEVICE_PATH;
          CdDev.Header.SubType  = MEDIA_CDROM_DP;
          SetDevicePathNodeLength (&CdDev.Header, sizeof (CdDev));

          CdDev.BootEntry = 0;
          /* Convert from partition to media blocks */
          CdDev.PartitionStart = StartLba / BlkPerSec;  /* start, LBA */
          CdDev.PartitionSize  = SizeLbs / BlkPerSec;   /* size,  LBs */

          Status = PartitionInstallChildHandle (
              This,
              Handle,
              DiskIo,
              DiskIo2,
              BlockIo,
              BlockIo2,
              DevicePath,
              (EFI_DEVICE_PATH_PROTOCOL *) &CdDev,
              CdDev.PartitionStart,
              CdDev.PartitionStart + CdDev.PartitionSize - 1,
              SubBlockSize,
              FALSE
                                                );

          if (!EFI_ERROR(Status)) {
              Found = EFI_SUCCESS;
          }
      }

  } while (0);

 done:
  FreePool(Block);

  return Found;
}
