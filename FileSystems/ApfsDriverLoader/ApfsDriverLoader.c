/** @file

APFS Driver Loader - loads apfs.efi from EfiBootRecord block

Copyright (c) 2017-2018, savvas
Copyright (c) 2018, vit9696

All rights reserved.

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
//#include <AppleSupportPkgVersion.h>

#define APPLE_SUPPORT_VERSION  L"2.0.9"
#include "ApfsDriverLoader.h"
#include "EfiComponentName.h"

STATIC BOOLEAN  LegacyScan       = FALSE;
STATIC UINT64   LegacyBaseOffset = 0;

UINT64
ApfsBlockChecksumCalculate (
  UINT32  *Data,
  UINTN   DataSize
  )
{
  UINTN         Index;
  UINT64        Sum1 = 0;
  UINT64        Check1 = 0;
  UINT64        Sum2 = 0;
  UINT64        Check2 = 0;
  CONST UINT64  ModValue = 0xFFFFFFFFull;

  for (Index = 0; Index < DataSize / sizeof (UINT32); Index++) {
//    Sum1 = ((Sum1 + (UINT64)Data[Index]) % ModValue);
//    Sum2 = (Sum2 + Sum1) % ModValue;
    Sum1 += (UINT64)*Data++;
    Sum2 += Sum1;
  }

  Check1 = ModValue - ((Sum1 + Sum2) % ModValue);
  Check2 = ModValue - ((Sum1 + Check1) % ModValue);

  return (Check2 << 32) | Check1;
}

//
// Function to check block checksum.
// Returns TRUE if the checksum is valid.
//
BOOLEAN
ApfsBlockChecksumVerify (
  UINT8   *Data,
  UINTN   DataSize
  )
{
  UINT64  NewChecksum;
  UINT64  *CurrChecksum = (UINT64 *) Data;

  NewChecksum = ApfsBlockChecksumCalculate (
    (UINT32 *) (Data + sizeof (UINT64)),
    DataSize - sizeof (UINT64)
    );

  return NewChecksum == *CurrChecksum;
}

EFI_STATUS
EFIAPI
StartApfsDriver (
  IN EFI_HANDLE  ControllerHandle,
  IN VOID        *EfiFileBuffer,
  IN UINTN       EfiFileSize
  )
{
  EFI_STATUS                 Status;
  EFI_HANDLE                 ImageHandle              = NULL;
  EFI_DEVICE_PATH_PROTOCOL   *ParentDevicePath        = NULL;
  EFI_LOADED_IMAGE_PROTOCOL  *LoadedApfsDrvImage      = NULL;
  EFI_SYSTEM_TABLE           *NewSystemTable          = NULL;

  if (EfiFileBuffer == NULL
    || EfiFileSize == 0
    || EfiFileSize > (UINT32) 0xFFFFFFFFULL) {
    DEBUG ((DEBUG_WARN, "Broken apfs.efi\n"));
    return EFI_UNSUPPORTED;
  }

  DEBUG ((DEBUG_VERBOSE, "Loading apfs.efi from memory!\n"));

  //
  // Try to retrieve DevicePath
  //
  Status = gBS->HandleProtocol (
    ControllerHandle,
    &gEfiDevicePathProtocolGuid,
    (VOID **) &ParentDevicePath
    );

  if (EFI_ERROR(Status)) {
      ParentDevicePath = NULL;
      DEBUG ((DEBUG_WARN, "ApfsDriver DevicePath not present\n"));
  }

/*
  DEBUG ((DEBUG_WARN, "Verifying binary signature"));
=======

  DEBUG ((DEBUG_WARN, "ImageSize before verification: %lu\n", EfiFileSize));

  DEBUG ((DEBUG_WARN, "Verifying binary signature\n"));
  Status = VerifyApplePeImageSignature (
    EfiFileBuffer,
    &EfiFileSize,
    NULL
    );

  DEBUG ((DEBUG_WARN, "New ImageSize after verification: %lu\n", EfiFileSize));


  if (!EFI_ERROR(Status)) {
*/
    Status = gBS->LoadImage (
      FALSE,
      gImageHandle,
      ParentDevicePath,
      EfiFileBuffer,
      EfiFileSize,
      &ImageHandle
      );
      if (EFI_ERROR(Status)) {
        DEBUG ((DEBUG_WARN, "Load image failed with Status: %r\n", Status));
        return Status;
      }
  /*
  }
   else {
      DEBUG ((DEBUG_WARN, "SECURITY VIOLATION!!! Binary modified!\n"));
      return Status;
    }
   */

  Status = gBS->HandleProtocol (
    ImageHandle,
    &gEfiLoadedImageProtocolGuid,
    (VOID *) &LoadedApfsDrvImage
    );

  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_WARN, "Failed to Handle LoadedImage Protool with Status: %r\n", Status));
    gBS->UnloadImage (ImageHandle);
    return Status;
  }

  //
  // Patch verbose
  //
  NewSystemTable = (EFI_SYSTEM_TABLE *) AllocateZeroPool(gST->Hdr.HeaderSize);

  if (NewSystemTable == NULL) {
    gBS->UnloadImage (ImageHandle);
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem((VOID *) NewSystemTable, gST, gST->Hdr.HeaderSize);
  NewSystemTable->ConOut = &mNullTextOutputProtocol;
  NewSystemTable->Hdr.CRC32 = 0;

  Status = gBS->CalculateCrc32 (
    NewSystemTable,
    NewSystemTable->Hdr.HeaderSize,
    &NewSystemTable->Hdr.CRC32
    );

  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_WARN, "Failed to calculated new system table CRC32 with Status: %r\n", Status));
    FreePool(NewSystemTable);
    gBS->UnloadImage (ImageHandle);
    return Status;
  }

  LoadedApfsDrvImage->SystemTable = NewSystemTable;

  Status = gBS->StartImage (
    ImageHandle,
    NULL,
    NULL
    );

  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_WARN, "Failed to start ApfsDriver with Status: %r\n", Status));

    //
    // Unload ApfsDriver image from memory
    //
    gBS->UnloadImage (ImageHandle);
    FreePool(NewSystemTable);
    return Status;
  }

  //
  // Connect loaded apfs.efi to controller from which we retrieve it
  //
  gBS->ConnectController (ControllerHandle, NULL, NULL, TRUE);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ReadDisk (
  IN EFI_DISK_IO_PROTOCOL   *DiskIo,
  IN EFI_DISK_IO2_PROTOCOL  *DiskIo2,
  IN UINT32                 MediaId,
  IN UINT64                 Offset,
  IN UINTN                  BufferSize,
  OUT UINT8                 *Buffer
  )
{
  EFI_STATUS  Status;

  if (DiskIo2 != NULL) {
    Status = DiskIo2->ReadDiskEx (
      DiskIo2,
      MediaId,
      Offset,
      NULL,
      BufferSize,
      Buffer
      );
  } else if (DiskIo != NULL) {
      Status = DiskIo->ReadDisk (
        DiskIo,
        MediaId,
        Offset,
        BufferSize,
        Buffer
        );
    } else {
      Status = EFI_UNSUPPORTED;
    }

  return Status;
}

//
// Function to parse GPT entries in legacy
//
EFI_STATUS
EFIAPI
LegacyApfsContainerScan (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle
  )
{
  EFI_STATUS                  Status;
  UINTN                       Index               = 0;
  UINT8                       *Block              = NULL;
  EFI_LBA                     Lba                 = 0;
  UINT32                      PartitionNumber     = 0;
  UINT32                      PartitionEntrySize  = 0;
  EFI_PARTITION_TABLE_HEADER  *GptHeader          = NULL;
  UINT32                      MediaId             = 0;
  UINT32                      BlockSize           = 0;
  EFI_BLOCK_IO_PROTOCOL       *BlockIo            = NULL;
  EFI_BLOCK_IO2_PROTOCOL      *BlockIo2           = NULL;
  EFI_DISK_IO_PROTOCOL        *DiskIo             = NULL;
  EFI_DISK_IO2_PROTOCOL       *DiskIo2            = NULL;
  EFI_PARTITION_ENTRY         *ApfsGptEntry       = NULL;

  //
  // Open I/O protocols
  //
  Status = gBS->OpenProtocol (
    ControllerHandle,
    &gEfiBlockIo2ProtocolGuid,
    (VOID **) &BlockIo2,
    This->DriverBindingHandle,
    ControllerHandle,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );

  if (EFI_ERROR(Status)) {
    BlockIo2 = NULL;

    Status = gBS->OpenProtocol (
      ControllerHandle,
      &gEfiBlockIoProtocolGuid,
      (VOID **) &BlockIo,
      This->DriverBindingHandle,
      ControllerHandle,
      EFI_OPEN_PROTOCOL_GET_PROTOCOL
      );

    if (EFI_ERROR(Status)) {
      return EFI_UNSUPPORTED;
    }
  }

  Status = gBS->OpenProtocol (
    ControllerHandle,
    &gEfiDiskIo2ProtocolGuid,
    (VOID **) &DiskIo2,
    This->DriverBindingHandle,
    ControllerHandle,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );

  if (EFI_ERROR(Status)) {
    DiskIo2 = NULL;
    Status = gBS->OpenProtocol (
      ControllerHandle,
      &gEfiDiskIoProtocolGuid,
      (VOID **) &DiskIo,
      This->DriverBindingHandle,
      ControllerHandle,
      EFI_OPEN_PROTOCOL_GET_PROTOCOL
      );

    if (EFI_ERROR(Status)){
      return EFI_UNSUPPORTED;
    }
  }

  if (BlockIo2 != NULL) {
    BlockSize     = BlockIo2->Media->BlockSize;
    MediaId       = BlockIo2->Media->MediaId;
  } else if (BlockIo != NULL) {
      BlockSize     = BlockIo->Media->BlockSize;
      MediaId       = BlockIo->Media->MediaId;
    } else {
      return EFI_UNSUPPORTED;
    }


  Block = AllocateZeroPool((UINTN)BlockSize);
  if (Block == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Read GPT header first.
  //
  Status = ReadDisk (
    DiskIo,
    DiskIo2,
    MediaId,
    BlockSize,
    BlockSize,
    Block
    );

  if (EFI_ERROR(Status)) {
    FreePool(Block);
    return EFI_DEVICE_ERROR;
  }

  GptHeader = (EFI_PARTITION_TABLE_HEADER *) Block;
  PartitionEntrySize = GptHeader->SizeOfPartitionEntry;

  //
  // Check GPT Header signature.
  //
  if (GptHeader->Header.Signature == EFI_PTAB_HEADER_ID) {
    //
    // Get partitions count.
    //
    PartitionNumber = GptHeader->NumberOfPartitionEntries;
    //
    // Get partitions array start_lba.
    //
    Lba = GptHeader->PartitionEntryLBA;
    //
    // Reallocate Block size to contain all of partition entries.
    //
    FreePool(Block);
    Block = AllocateZeroPool((UINTN)PartitionNumber * PartitionEntrySize);
    if (Block == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
  } else {
    FreePool(Block);
    return EFI_UNSUPPORTED;
  }

 Status = ReadDisk (
    DiskIo,
    DiskIo2,
    MediaId,
    MultU64x32 (Lba, BlockSize),
    (UINTN)PartitionNumber * PartitionEntrySize,
    Block
    );

  if (EFI_ERROR(Status)) {
    FreePool(Block);
    return EFI_DEVICE_ERROR;
  }

  //
  // Analyze partition entries.
  //
  for (Index = 0; Index < (UINTN)PartitionEntrySize * PartitionNumber; Index += PartitionEntrySize) {
    EFI_PARTITION_ENTRY *CurrentEntry = (EFI_PARTITION_ENTRY *) (Block + Index);
    if (CompareGuid (&CurrentEntry->PartitionTypeGUID, &gAppleApfsPartitionTypeGuid)) {
      ApfsGptEntry = CurrentEntry;
      break;
    }

    if (CurrentEntry->StartingLBA == 0ull && CurrentEntry->EndingLBA == 0ull) {
      break;
    }
  }

  if (ApfsGptEntry == NULL)  {
    FreePool(Block);
    return EFI_UNSUPPORTED;
  }
  LegacyBaseOffset = MultU64x32 (ApfsGptEntry->StartingLBA, BlockSize);
  FreePool(Block);

  return EFI_SUCCESS;

}

/**

  Routine Description:

    Test to see if this driver can load ApfsDriver from Block device.
    ControllerHandle must support both Disk IO and Block IO protocols.

  Arguments:

    This                  - Protocol instance pointer.
    ControllerHandle      - Handle of device to test.
    RemainingDevicePath   - Not used.

  Returns:

    EFI_SUCCESS           - This driver supports this device.
    EFI_ALREADY_STARTED   - This driver is already running on this device.
    other                 - This driver does not support this device.

**/
EFI_STATUS
EFIAPI
ApfsDriverLoaderSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
{
  EFI_STATUS                    Status;
  APPLE_PARTITION_INFO_PROTOCOL *ApplePartitionInfo          = NULL;
  EFI_PARTITION_INFO_PROTOCOL   *Edk2PartitionInfo           = NULL;

  Status = gBS->OpenProtocol (
    ControllerHandle,
    &gApfsEfiBootRecordInfoProtocolGuid,
    NULL,
    This->DriverBindingHandle,
    ControllerHandle,
    EFI_OPEN_PROTOCOL_TEST_PROTOCOL
    );

   if (!EFI_ERROR(Status)) {
    return EFI_UNSUPPORTED;
   }

  //
  // We check for both DiskIO and BlockIO protocols.
  // Both V1 and V2.
  //

  Status = gBS->OpenProtocol (
    ControllerHandle,
    &gEfiDiskIo2ProtocolGuid,
    NULL,
    This->DriverBindingHandle,
    ControllerHandle,
    EFI_OPEN_PROTOCOL_TEST_PROTOCOL
    );

  if (EFI_ERROR(Status)) {
    Status = gBS->OpenProtocol (
      ControllerHandle,
      &gEfiDiskIoProtocolGuid,
      NULL,
      This->DriverBindingHandle,
      ControllerHandle,
      EFI_OPEN_PROTOCOL_TEST_PROTOCOL
      );
  }

  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = gBS->OpenProtocol (
    ControllerHandle,
    &gEfiBlockIo2ProtocolGuid,
    NULL,
    This->DriverBindingHandle,
    ControllerHandle,
    EFI_OPEN_PROTOCOL_TEST_PROTOCOL
    );

  if (EFI_ERROR(Status)) {
    Status = gBS->OpenProtocol(
      ControllerHandle,
      &gEfiBlockIoProtocolGuid,
      NULL,
      This->DriverBindingHandle,
      ControllerHandle,
      EFI_OPEN_PROTOCOL_TEST_PROTOCOL
      );
  }

  if (EFI_ERROR(Status)) {
    return Status;
  }

  if (LegacyScan) {
    return LegacyApfsContainerScan (This, ControllerHandle);
  }

  //
  // We check EfiPartitionInfoProtocol and ApplePartitionInfoProtocol
  //

  Status = gBS->OpenProtocol (
    ControllerHandle,
    &gAppleApfsPartitionTypeGuid,
    (VOID **) &Edk2PartitionInfo,
    This->DriverBindingHandle,
    ControllerHandle,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );

  if (EFI_ERROR(Status)) {
    Status = gBS->OpenProtocol (
      ControllerHandle,
      &gEfiPartitionInfoProtocolGuid,
      (VOID **) &Edk2PartitionInfo,
      This->DriverBindingHandle,
      ControllerHandle,
      EFI_OPEN_PROTOCOL_GET_PROTOCOL
      );

    if (EFI_ERROR(Status)) {
      Status = gBS->OpenProtocol (
        ControllerHandle,
        &gApplePartitionInfoProtocolGuid,
        (VOID **) &ApplePartitionInfo,
        This->DriverBindingHandle,
        ControllerHandle,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL
        );

      if (EFI_ERROR(Status)) {
        ApplePartitionInfo = NULL;
        return Status;
      }

      if (ApplePartitionInfo != NULL) {
        //
        // Verify GPT entry GUID
        //
        if (!CompareGuid ((EFI_GUID *) ApplePartitionInfo->PartitionType,
                         &gAppleApfsPartitionTypeGuid)) {
          return EFI_UNSUPPORTED;
        }
      }
    } else {

      //
      // Verify PartitionType
      //
      if (Edk2PartitionInfo->Type != PARTITION_TYPE_GPT) {
        return EFI_UNSUPPORTED;
      }

      //
      // Verify GPT entry GUID
      //
      if (!CompareGuid (&Edk2PartitionInfo->Info.Gpt.PartitionTypeGUID,
                       &gAppleApfsPartitionTypeGuid)) {
        return EFI_UNSUPPORTED;
      }
    }
  }

  return Status;
}

/**
  Routine Description:

    Start this driver on ControllerHandle by opening a Block IO and Disk IO
    protocol, reading ApfsContainer if present.

  Arguments:

    This                  - Protocol instance pointer.
    ControllerHandle      - Handle of device to bind driver to.
    RemainingDevicePath   - Not used.

  Returns:

    EFI_SUCCESS           - This driver is added to DeviceHandle.
    EFI_ALREADY_STARTED   - This driver is already running on DeviceHandle.
    EFI_OUT_OF_RESOURCES  - Can not allocate the memory.
    other                 - This driver does not support this device.

**/
EFI_STATUS
EFIAPI
ApfsDriverLoaderStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
{
  EFI_STATUS                        Status;
  UINTN                             Index                        = 0;
  UINTN                             CurPos                       = 0;
  EFI_BLOCK_IO_PROTOCOL             *BlockIo                     = NULL;
  EFI_BLOCK_IO2_PROTOCOL            *BlockIo2                    = NULL;
  EFI_DISK_IO_PROTOCOL              *DiskIo                      = NULL;
  EFI_DISK_IO2_PROTOCOL             *DiskIo2                     = NULL;
  UINT32                            ApfsBlockSize                = 0;
  UINT32                            MediaId                      = 0;
  UINT8                             *ApfsBlock                   = NULL;
  EFI_GUID                          ContainerUuid;
  UINT64                            EfiBootRecordBlockOffset     = 0;
  INT64                             EfiBootRecordBlockPtr        = 0;
  APFS_EFI_BOOT_RECORD              *EfiBootRecordBlock          = NULL;
  APFS_CSB                          *ContainerSuperBlock         = NULL;
  UINT64                            EfiFileCurrentExtentOffset   = 0;
  VOID                              *EfiFileBuffer               = NULL;
  UINTN                             EfiFileCurrentExtentSize     = 0;
  APFS_DRIVER_INFO_PRIVATE_DATA     *Private                     = NULL;
  APFS_EFIBOOTRECORD_LOCATION_INFO  *EfiBootRecordLocationInfo   = NULL;

  Status = gBS->OpenProtocol (
    ControllerHandle,
    &gApfsEfiBootRecordInfoProtocolGuid,
    NULL,
    This->DriverBindingHandle,
    ControllerHandle,
    EFI_OPEN_PROTOCOL_TEST_PROTOCOL
    );

   if (!EFI_ERROR(Status)) {
    return EFI_UNSUPPORTED;
   }

  DEBUG ((DEBUG_VERBOSE, "Apfs Container found.\n"));

  //
  // Open I/O protocols
  //
  Status = gBS->OpenProtocol (
    ControllerHandle,
    &gEfiBlockIo2ProtocolGuid,
    (VOID **) &BlockIo2,
    This->DriverBindingHandle,
    ControllerHandle,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );

  if (EFI_ERROR(Status)) {
    BlockIo2 = NULL;

    Status = gBS->OpenProtocol (
      ControllerHandle,
      &gEfiBlockIoProtocolGuid,
      (VOID **) &BlockIo,
      This->DriverBindingHandle,
      ControllerHandle,
      EFI_OPEN_PROTOCOL_GET_PROTOCOL
      );

    if (EFI_ERROR(Status)) {
      return EFI_UNSUPPORTED;
    }
  }

  Status = gBS->OpenProtocol (
    ControllerHandle,
    &gEfiDiskIo2ProtocolGuid,
    (VOID **) &DiskIo2,
    This->DriverBindingHandle,
    ControllerHandle,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );

  if (EFI_ERROR(Status)) {
    DiskIo2 = NULL;
    Status = gBS->OpenProtocol (
      ControllerHandle,
      &gEfiDiskIoProtocolGuid,
      (VOID **) &DiskIo,
      This->DriverBindingHandle,
      ControllerHandle,
      EFI_OPEN_PROTOCOL_GET_PROTOCOL
      );

    if (EFI_ERROR(Status)) {
      return EFI_UNSUPPORTED;
    }
  }

  if (BlockIo2 != NULL) {
    MediaId       = BlockIo2->Media->MediaId;
  } else {
    MediaId       = BlockIo->Media->MediaId;
  }

  ApfsBlock = AllocateZeroPool(2048);
  if (ApfsBlock == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Read ContainerSuperblock and get ApfsBlockSize.
  //
  Status = ReadDisk (
    DiskIo,
    DiskIo2,
    MediaId,
    LegacyBaseOffset,
    2048,
    ApfsBlock
    );

  if (EFI_ERROR(Status)) {
    FreePool(ApfsBlock);
    return EFI_DEVICE_ERROR;
  }

  ContainerSuperBlock = (APFS_CSB *) ApfsBlock;

  //
  // Verify ObjectOid and ObjectType
  //
  DEBUG ((DEBUG_VERBOSE, "ObjectId: %016llx\n", ContainerSuperBlock->BlockHeader.ObjectOid ));
  DEBUG ((DEBUG_VERBOSE, "ObjectType: %08x\n", ContainerSuperBlock->BlockHeader.ObjectType ));
  if (ContainerSuperBlock->BlockHeader.ObjectOid != 1
      || ContainerSuperBlock->BlockHeader.ObjectType != 0x80000001) {
    FreePool(ApfsBlock);
    return EFI_UNSUPPORTED;
  }

  //
  // Verify ContainerSuperblock magic.
  //
  DEBUG ((DEBUG_VERBOSE, "CsbMagic: %08x\n", ContainerSuperBlock->Magic));
  DEBUG ((DEBUG_VERBOSE, "Should be: %08x\n", APFS_CSB_SIGNATURE));

  if (ContainerSuperBlock->Magic != APFS_CSB_SIGNATURE) {
    FreePool(ApfsBlock);
    return EFI_UNSUPPORTED;
  }

  //
  // Get ApfsBlockSize.
  //
  ApfsBlockSize = ContainerSuperBlock->BlockSize;

  DEBUG ((
    DEBUG_VERBOSE,
    "Container Blocksize: %u bytes\n",
    ApfsBlockSize
    ));
  DEBUG ((
    DEBUG_VERBOSE,
    "ContainerSuperblock checksum: %016llx \n",
    ContainerSuperBlock->BlockHeader.Checksum
    ));

  //
  // Take pointer to EfiBootRecordBlock.
  //
  EfiBootRecordBlockPtr = ContainerSuperBlock->EfiBootRecordBlock;

  DEBUG ((
    DEBUG_VERBOSE,
    "EfiBootRecord located at: %llu block\n",
    EfiBootRecordBlockPtr
    ));

  //
  // Free ApfsBlock and allocate one of a correct size.
  // ContainerSuperBlock (& EfiBootRecordBlockPtr ?) will not valid now
  //
  FreePool(ApfsBlock);
  ApfsBlock = AllocateZeroPool(ApfsBlockSize);
  if (ApfsBlock == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Read full ContainerSuperblock with known BlockSize.
  //
  Status = ReadDisk (
    DiskIo,
    DiskIo2,
    MediaId,
    LegacyBaseOffset,
    ApfsBlockSize,
    ApfsBlock
    );

  if (EFI_ERROR(Status)) {
    FreePool(ApfsBlock);
    return EFI_DEVICE_ERROR;
  }

  //
  // Verify ContainerSuperblock checksum.
  //
  if (!ApfsBlockChecksumVerify ((UINT8 *) ApfsBlock, ApfsBlockSize)) {
    FreePool(ApfsBlock);
    return EFI_UNSUPPORTED;
  }

  //
  // Extract Container UUID
  //
  ContainerSuperBlock = (APFS_CSB *)ApfsBlock;
  CopyMem(&ContainerUuid, &ContainerSuperBlock->Uuid, sizeof (EFI_GUID));
  EfiBootRecordBlockPtr = ContainerSuperBlock->EfiBootRecordBlock;

  //
  // Calculate Offset of EfiBootRecordBlock
  //
  EfiBootRecordBlockOffset = MultU64x32 ((UINT64)EfiBootRecordBlockPtr, ApfsBlockSize)
                              + LegacyBaseOffset;

  DEBUG ((
    DEBUG_VERBOSE,
    "EfiBootRecordBlock offset: %016llx \n",
     EfiBootRecordBlockOffset
     ));

  //
  // Read EfiBootRecordBlock.
  //
  Status = ReadDisk (
    DiskIo,
    DiskIo2,
    MediaId,
    EfiBootRecordBlockOffset,
    ApfsBlockSize,
    ApfsBlock
    );

  if (EFI_ERROR(Status)) {
    FreePool(ApfsBlock);
    return EFI_DEVICE_ERROR;
  }

  //
  // Verify EfiBootRecordBlock checksum.
  //
  if (!ApfsBlockChecksumVerify (ApfsBlock, ApfsBlockSize)) {
    FreePool(ApfsBlock);
    return EFI_UNSUPPORTED;
  }

  EfiBootRecordBlock = (APFS_EFI_BOOT_RECORD *) ApfsBlock;
  if (EfiBootRecordBlock->Magic != APFS_EFIBOOTRECORD_SIGNATURE) {
    FreePool(ApfsBlock);
    return EFI_UNSUPPORTED;
  }

  DEBUG ((
    DEBUG_VERBOSE,
    "EfiBootRecordBlock checksum: %016llx\n",
    EfiBootRecordBlock->BlockHeader.Checksum
    ));

  //
  // Loop over extents inside EfiBootRecord
  //        EFI embedded driver could be defragmented across whole container
  //
  DEBUG ((
    DEBUG_VERBOSE,
    "EFI embedded driver extents number %u\n",
    EfiBootRecordBlock->NumOfExtents
    ));

  //
  // Read EFI embedded file from extents
  //
  for (Index = 0; Index < EfiBootRecordBlock->NumOfExtents; Index++) {
    DEBUG ((
        DEBUG_VERBOSE,
        "EFI embedded driver extent located at: %lld block\n with size %llu\n",
        EfiBootRecordBlock->RecordExtents[Index].StartPhysicalAddr,
        EfiBootRecordBlock->RecordExtents[Index].BlockCount
        ));

    EfiFileCurrentExtentOffset = MultU64x32 (
                                (UINT64)EfiBootRecordBlock->RecordExtents[Index].StartPhysicalAddr,
                                ApfsBlockSize
                                )  + LegacyBaseOffset;

    EfiFileCurrentExtentSize = (UINTN)MultU64x32 (
                                EfiBootRecordBlock->RecordExtents[Index].BlockCount,
                                ApfsBlockSize
                                );

    if (EfiFileCurrentExtentSize == 0) {
      continue;
    }
    //
    // Adjust buffer size
    //
    EfiFileBuffer = ReallocatePool (
                      CurPos,
                      CurPos + EfiFileCurrentExtentSize,
                      EfiFileBuffer
                      );

    if (EfiFileBuffer == NULL) {
      FreePool(ApfsBlock);
      return EFI_OUT_OF_RESOURCES;
    }

    //
    // Read current extent
    //
    Status = ReadDisk (
      DiskIo,
      DiskIo2,
      MediaId,
      EfiFileCurrentExtentOffset,
      EfiFileCurrentExtentSize,
      (UINT8*)EfiFileBuffer + CurPos
      );

    if (EFI_ERROR(Status)) {
      FreePool(EfiFileBuffer);
      FreePool(ApfsBlock);
      return EFI_DEVICE_ERROR;
    }
    //
    // Sum size for buffer offset
    //
    CurPos += EfiFileCurrentExtentSize;

  }

  //
  // Drop tail
  // We do it because we read blocksize aligned data
  // Apfs driver size given in bytes
  //
  if (CurPos > EfiBootRecordBlock->EfiFileLen) {
    //
    // Zero tail
    //
    ZeroMem (
      (UINT8*)EfiFileBuffer + EfiBootRecordBlock->EfiFileLen,
      CurPos - EfiBootRecordBlock->EfiFileLen
      );
    //
    // Reallocate buffer
    //
    EfiFileBuffer = ReallocatePool (
                      CurPos,
                      EfiBootRecordBlock->EfiFileLen,
                      EfiFileBuffer
                      );
  }

  //
  // Fill public AppleFileSystemEfiBootRecordInfo protocol interface
  //
  Private = AllocatePool (sizeof (APFS_DRIVER_INFO_PRIVATE_DATA));
  if (Private == NULL) {
    FreePool(ApfsBlock);
    if (EfiFileBuffer) {
      FreePool(EfiFileBuffer);
    }
    return EFI_OUT_OF_RESOURCES;
  }

  Private->ControllerHandle  = ControllerHandle;
  EfiBootRecordLocationInfo = &Private->EfiBootRecordLocationInfo;
  EfiBootRecordLocationInfo->ControllerHandle = ControllerHandle;
  CopyMem(&EfiBootRecordLocationInfo->ContainerUuid, &ContainerUuid, 16);

  Status = gBS->InstallMultipleProtocolInterfaces (
    &Private->ControllerHandle,
    &gApfsEfiBootRecordInfoProtocolGuid,
    &Private->EfiBootRecordLocationInfo,
    NULL
    );

  if (EFI_ERROR(Status)) {
    DEBUG ((
      DEBUG_WARN,
      "ApfsEfiBootRecordInfoProtocol install failed with Status %r\n",
      Status
      ));
    if (EfiFileBuffer != NULL) {
      FreePool(EfiFileBuffer);
    }
    if (Private != NULL) {
      FreePool(Private);
    }
    FreePool(ApfsBlock);

    return Status;
  }

  Status = StartApfsDriver (
    ControllerHandle,
    EfiFileBuffer,
    EfiBootRecordBlock->EfiFileLen
    );

  FreePool(ApfsBlock);

  if (EFI_ERROR(Status)) {
    gBS->UninstallProtocolInterface (
      ControllerHandle,
      &gApfsEfiBootRecordInfoProtocolGuid,
      NULL
      );

    if (EfiFileBuffer != NULL) {
      FreePool(EfiFileBuffer);
    }
    if (Private != NULL) {
      FreePool(Private);
    }

    return EFI_UNSUPPORTED;
  }

  //
  // Free memory and close DiskIo protocol
  //
  if (EfiFileBuffer != NULL) {
    FreePool(EfiFileBuffer);
  }
  if (Private != NULL) {
    FreePool(Private);
  }
  if (DiskIo2 != NULL) {
    gBS->CloseProtocol (
      ControllerHandle,
      &gEfiDiskIo2ProtocolGuid,
      This->DriverBindingHandle,
      ControllerHandle
      );
  } else {
    gBS->CloseProtocol (
      ControllerHandle,
      &gEfiDiskIoProtocolGuid,
      This->DriverBindingHandle,
      ControllerHandle
      );
  }

  return EFI_SUCCESS;
}

/**

  Routine Description:
    Stop this driver on ControllerHandle.

  Arguments:
    This                  - Protocol instance pointer.
    ControllerHandle      - Handle of device to stop driver on.
    NumberOfChildren      - Not used.
    ChildHandleBuffer     - Not used.

  Returns:
    EFI_SUCCESS           - This driver is removed DeviceHandle.
    other                 - This driver was not removed from this device.

**/
EFI_STATUS
EFIAPI
ApfsDriverLoaderStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   ControllerHandle,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  )
{
  EFI_STATUS                        Status;
  APFS_EFIBOOTRECORD_LOCATION_INFO  *EfiBootRecordLocationInfo = NULL;

  Status = gBS->OpenProtocol (
    ControllerHandle,
    &gApfsEfiBootRecordInfoProtocolGuid,
    (VOID **) &EfiBootRecordLocationInfo,
    This->DriverBindingHandle,
    ControllerHandle,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );

  if (EFI_ERROR(Status)) {
    Status = gBS->CloseProtocol (
      ControllerHandle,
      &gEfiDiskIoProtocolGuid,
      This->DriverBindingHandle,
      ControllerHandle
      );
    Status = gBS->CloseProtocol (
      ControllerHandle,
      &gEfiDiskIo2ProtocolGuid,
      This->DriverBindingHandle,
      ControllerHandle
      );
  } else {
    Status = gBS->UninstallMultipleProtocolInterfaces (
      EfiBootRecordLocationInfo->ControllerHandle,
      &gApfsEfiBootRecordInfoProtocolGuid,
      EfiBootRecordLocationInfo
      );
  }

  return Status;
}

//
// Interface structure for the EFI Driver Binding protocol.
// According to UEFI Spec 2.6 , we should define Supported, Start, Stop function for
// DriverBinding
//
EFI_DRIVER_BINDING_PROTOCOL gApfsDriverLoaderDriverBinding = {
  ApfsDriverLoaderSupported,
  ApfsDriverLoaderStart,
  ApfsDriverLoaderStop,
  0x10,
  NULL,
  NULL,
};

/**

  Routine Description:

    Register Driver Binding protocol for this driver.

  Arguments:

    ImageHandle           - Handle for the image of this driver.
    SystemTable           - Pointer to the EFI System Table.

  Returns:

    EFI_SUCCESS           - Driver loaded.
    other                 - Driver not loaded.

**/
EFI_STATUS
EFIAPI
ApfsDriverLoaderInit (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
)
{
  EFI_STATUS                          Status;
  VOID                                *PartitionInfoInterface = NULL;

  DEBUG ((
    DEBUG_VERBOSE,
    "Starting ApfsDriverLoader ver. %s\n",
    APPLE_SUPPORT_VERSION
    ));

  //
  // Check that PartitionInfo protocol present
  // If not present use Legacy scan
  //
  Status = gBS->LocateProtocol (
    &gAppleApfsPartitionTypeGuid,
    NULL,
    (VOID **) &PartitionInfoInterface
    );
  if (Status == EFI_NOT_FOUND) {
    Status = gBS->LocateProtocol (
      &gEfiPartitionInfoProtocolGuid,
      NULL,
      (VOID **) &PartitionInfoInterface
      );
    if (Status == EFI_NOT_FOUND) {
      Status = gBS->LocateProtocol (
        &gApplePartitionInfoProtocolGuid,
        NULL,
        (VOID **) &PartitionInfoInterface
      );
    }
  }

  if (EFI_ERROR(Status)) {
    DEBUG ((
      DEBUG_VERBOSE,
      "No partition info protocol, using Legacy scan\n"
      ));
    LegacyScan = TRUE;
  }

  //
  // Install Driver Binding Instance
  //
  Status = EfiLibInstallDriverBindingComponentName2 (
    ImageHandle,
    SystemTable,
    &gApfsDriverLoaderDriverBinding,
    ImageHandle,
    &gApfsDriverLoaderComponentName,
    &gApfsDriverLoaderComponentName2
    );

  return Status;
}
