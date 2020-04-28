/** @file
 Sample ACPI Platform Driver

 Copyright (c) 2008 - 2010, Intel Corporation. All rights reserved.<BR>
 This program and the accompanying materials
 are licensed and made available under the terms and conditions of the BSD License
 which accompanies this distribution.  The full text of the license may be found at
 http://opensource.org/licenses/bsd-license.php

 THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
 WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

 **/
/*
 Slice 2011 - corrections for MacOS
 Load and install patched DSDT.aml, SSDT-N.aml

 For now this module is obsolete but it needed just to install native ACPI tables into gST
 */

#ifndef DEBUG_ACPI
#ifndef DEBUG_ALL
#define DEBUG_ACPI 0
#else
#define DEBUG_ACPI 0
//DEBUG_ALL
#endif
#endif
#define LIP 0
#define READTABLES 0

#if DEBUG_ACPI==2
#define DBG(...)  Print(__VA_ARGS__)
#elif DEBUG_ACPI==1
#define DBG(...)  BootLog(__VA_ARGS__)
#else
#define DBG(...)
#endif

#include <Uefi.h>
#include <PiDxe.h>

#include <Protocol/AcpiTable.h>
//#include <Protocol/FirmwareVolume2.h>
#include <Protocol/DevicePath.h>
//#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>

#include <Guid/HobList.h>
#include <Guid/FileInfo.h>
#include <Guid/FileSystemInfo.h>
#include <Guid/Acpi.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
//#include <Library/DevicePathLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
//#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/HobLib.h>
#include <Library/PrintLib.h>

#include <IndustryStandard/Acpi.h>
//#include "HobGeneration.h"
#include <Protocol/MsgLog.h>

#include "AcpiTable.h"

#define RoundPage(x)  ((((unsigned)(x)) + EFI_PAGE_SIZE - 1) & ~(EFI_PAGE_SIZE - 1))

CHAR8 *msgCursor;
MESSAGE_LOG_PROTOCOL *Msg;


#pragma pack(1)

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER  Header;
  UINT32                       Entry;
} RSDT_TABLE;

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER  Header;
  UINT64                       Entry;
} XSDT_TABLE;

typedef union {
  UINT32 Sign;
  CHAR8  ASign[4];
} SIGNAT;

#pragma pack()
EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE   *Fadt;
//extern EFI_ACPI_TABLE_INSTANCE   *mPrivateData;


VOID
InstallLegacyTables (
                     EFI_ACPI_TABLE_PROTOCOL         *AcpiTable,
                     EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER *Rsdp   //legacy table
)
{
  EFI_STATUS                      Status;
  UINTN              Index;
  UINTN                           TableHandle;
  UINTN                           TableSize;
  UINT32              EntryCount;
  UINT32              *EntryPtr;
  UINT64              Entry64;
  EFI_ACPI_DESCRIPTION_HEADER    *Table;
  RSDT_TABLE            *Rsdt;
  XSDT_TABLE            *Xsdt;
  //  EFI_ACPI_COMMON_HEADER          *Dsdt;
  //  EFI_ACPI_2_0_FIRMWARE_ACPI_CONTROL_STRUCTURE *Facs;
  UINTN              BasePtr;
  //  UINT32                       Signature;
  SIGNAT              Signature;
  EFI_ACPI_TABLE_INSTANCE      *AcpiInstance;
  //  BOOLEAN              Found = FALSE;

  TableHandle = 0;
  AcpiInstance = EFI_ACPI_TABLE_INSTANCE_FROM_THIS(AcpiTable);
  Rsdt = (RSDT_TABLE *)(UINTN)(Rsdp->RsdtAddress); //(UINTN)
  Xsdt = NULL;
  if ((Rsdp->Revision >= 2) && (Rsdp->XsdtAddress < (UINT64)(UINTN)-1)) {
    Xsdt = (XSDT_TABLE *)(UINTN)Rsdp->XsdtAddress;
  }
  //Begin patching from Xsdt
  //Install Xsdt if any
  if (Xsdt) {
    /*
     TableSize = sizeof(EFI_ACPI_DESCRIPTION_HEADER) + sizeof(UINT64);
     //Now copy legacy table into new protocol
     CopyMem(AcpiInstance->Xsdt, Xsdt, TableSize);
     AcpiInstance->Xsdt->Length = (UINT32)TableSize;
     AcpiInstance->Xsdt->Checksum = 0;
     AcpiInstance->Xsdt->Checksum = CalculateCheckSum8((UINT8 *)AcpiInstance->Xsdt, TableSize);

     Signature.Sign = Xsdt->Header.Signature;
     DBG(L"Install table: %c%c%c%c\n",
     Signature.ASign[0], Signature.ASign[1], Signature.ASign[2], Signature.ASign[3]);
     */

    /*    Status = AcpiTable->InstallAcpiTable (
     AcpiTable,
     Xsdt,
     TableSize,
     &TableHandle
     );
     if (EFI_ERROR(Status)) {
     return;
     }
     */
    //First scan for Xsdt
    EntryCount = (Xsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);

    BasePtr = (UINTN)(&(Xsdt->Entry));
    for (Index = 0; Index < EntryCount; Index ++) {
      CopyMem (&Entry64, (VOID *)(BasePtr + Index * sizeof(UINT64)), sizeof(UINT64));
      Table = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(Entry64));
      TableSize = Table->Length;
      if (Index == 0) {
        Fadt = (EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE*)Table;
        if (Fadt->FirmwareCtrl) {
          AcpiInstance->Facs1 = (EFI_ACPI_1_0_FIRMWARE_ACPI_CONTROL_STRUCTURE*)(UINTN)Fadt->FirmwareCtrl;
          AcpiInstance->Facs3 = (EFI_ACPI_3_0_FIRMWARE_ACPI_CONTROL_STRUCTURE*)(UINTN)Fadt->FirmwareCtrl;
        }
        else if (Fadt->Header.Revision >= EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION && Fadt->XFirmwareCtrl) {
          AcpiInstance->Facs1 = (EFI_ACPI_1_0_FIRMWARE_ACPI_CONTROL_STRUCTURE*)(UINTN)Fadt->XFirmwareCtrl;
          AcpiInstance->Facs3 = (EFI_ACPI_3_0_FIRMWARE_ACPI_CONTROL_STRUCTURE*)(UINTN)Fadt->XFirmwareCtrl;
        }
      }
      Signature.Sign = Table->Signature;
      DBG(L"Install table from %x: %c%c%c%c\n", (UINTN)Table,
          Signature.ASign[0], Signature.ASign[1], Signature.ASign[2], Signature.ASign[3]);
      Status = AcpiTable->InstallAcpiTable (
                                            AcpiTable,
                                            Table,
                                            TableSize,
                                            &TableHandle
                                            );
      if (EFI_ERROR(Status)) {
        continue;
      }
    }
    //Now find Fadt and install dsdt and facs
    /*    Table = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(Fadt->FirmwareCtrl));
     TableSize = Table->Length;
     Signature.Sign = Table->Signature;
     DBG(L"Install table: %c%c%c%c\n",
     Signature.ASign[0], Signature.ASign[1], Signature.ASign[2], Signature.ASign[3]);
     Status = AcpiTable->InstallAcpiTable (
     AcpiTable,
     Table,
     TableSize,
     &TableHandle
     );
     */
    // do not install legacy DSDT yet
    /*    Table = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(Fadt->Dsdt));
     TableSize = Table->Length;
     Signature.Sign = Table->Signature;
     DBG(L"Install table: %c%c%c%c\n",
     Signature.ASign[0], Signature.ASign[1], Signature.ASign[2], Signature.ASign[3]);

     Status = AcpiTable->InstallAcpiTable (
     AcpiTable,
     Table,
     TableSize,
     &TableHandle
     );
     */
  }
  if (Xsdt && Rsdt) {
    Rsdt->Entry = (UINT32)(UINTN)Fadt; //Copy Fadt from XSDT
  }
  if (!Xsdt && Rsdt) {
    //Install Rsdt
    /*
     DBG(L"Xsdt not found, patch Rsdt\n");

     TableSize = Rsdt->Header.Length;
     Signature.Sign = Rsdt->Header.Signature;
     DBG(L"Install table: %c%c%c%c\n",
     Signature.ASign[0], Signature.ASign[1], Signature.ASign[2], Signature.ASign[3]);
     CopyMem(AcpiInstance->Rsdt1, Rsdt, TableSize);
     CopyMem(AcpiInstance->Rsdt3, Rsdt, TableSize);
     */
    /*    Status = AcpiTable->InstallAcpiTable (
     AcpiTable,
     Rsdt,
     TableSize,
     &TableHandle
     );
     if (EFI_ERROR(Status)) {
     return;
     }
     */
    //First scan for RSDT
    EntryCount = (UINT32)(Rsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT32);
    DBG(L"RSDT table length %d\n", EntryCount);
    EntryPtr = &Rsdt->Entry;
    Fadt = (EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE*)((UINTN)(*EntryPtr));
    DBG(L"Fadt from Rsdt @ %x\n", (UINTN)Fadt);
    if (Fadt->FirmwareCtrl) {
      AcpiInstance->Facs1 = (EFI_ACPI_1_0_FIRMWARE_ACPI_CONTROL_STRUCTURE*)(UINTN)Fadt->FirmwareCtrl;
      AcpiInstance->Facs3 = (EFI_ACPI_3_0_FIRMWARE_ACPI_CONTROL_STRUCTURE*)(UINTN)Fadt->FirmwareCtrl;
    } else if (Fadt->Header.Revision >= EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION &&
               Fadt->XFirmwareCtrl) {
      AcpiInstance->Facs1 = (EFI_ACPI_1_0_FIRMWARE_ACPI_CONTROL_STRUCTURE*)(UINTN)Fadt->XFirmwareCtrl;
      AcpiInstance->Facs3 = (EFI_ACPI_3_0_FIRMWARE_ACPI_CONTROL_STRUCTURE*)(UINTN)Fadt->XFirmwareCtrl;
    }
    for (Index = 0; Index < EntryCount; Index ++, EntryPtr ++) {
      Table = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(*EntryPtr));
      TableSize = Table->Length;
      Signature.Sign = Table->Signature;
      DBG(L"Install table: %c%c%c%c\n",
          Signature.ASign[0], Signature.ASign[1], Signature.ASign[2], Signature.ASign[3]);
      Status = AcpiTable->InstallAcpiTable (
                                            AcpiTable,
                                            Table,
                                            TableSize,
                                            &TableHandle
                                            );
      if (EFI_ERROR(Status)) {
        continue;
      }

    }
    //Now find Fadt and install dsdt and facs
    Table = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(Fadt->FirmwareCtrl));
    TableSize = Table->Length;
    Signature.Sign = Table->Signature;
    DBG(L"Install table: %c%c%c%c\n",
        Signature.ASign[0], Signature.ASign[1], Signature.ASign[2], Signature.ASign[3]);
    /*Status = */AcpiTable->InstallAcpiTable (
                                              AcpiTable,
                                              Table,
                                              TableSize,
                                              &TableHandle
                                              );

    // do not install legacy dsdt until we test a file DSDT.aml
    /*
     Table = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(Fadt->Dsdt));
     TableSize = Table->Length;
     Signature.Sign = Table->Signature;
     DBG(L"Install table: %c%c%c%c\n",
     Signature.ASign[0], Signature.ASign[1], Signature.ASign[2], Signature.ASign[3]);

     Status = AcpiTable->InstallAcpiTable (
     AcpiTable,
     Table,
     TableSize,
     &TableHandle
     );
     */
  }
#if DEBUG_ACPI==2
  gBS->Stall(5000000);
#endif

}
#define NUM_TABLES 12
CHAR16* ACPInames[NUM_TABLES] = {
  L"DSDT.aml",
  L"SSDT.aml",
  L"SSDT-1.aml",
  L"SSDT-2.aml",
  L"SSDT-3.aml",
  L"SSDT-4.aml",
  L"SSDT-5.aml",
  L"SSDT-6.aml",
  L"SSDT-7.aml",
  L"APIC.aml",
  L"HPET.aml",
  L"MCFG.aml"
};

/**
 This function calculates and updates an UINT8 checksum.

 @param  Buffer          Pointer to buffer to checksum
 @param  Size            Number of bytes to checksum

 **/
VOID
AcpiPlatformChecksum (
                      IN UINT8      *Buffer,
                      IN UINTN      Size
                      )
{
  UINTN ChecksumOffset;

  ChecksumOffset = OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER, Checksum);
  //
  // Set checksum to 0 first
  //
  Buffer[ChecksumOffset] = 0;
  //
  // Update checksum value
  //
  Buffer[ChecksumOffset] = CalculateCheckSum8(Buffer, Size);
}


/**
 Entrypoint of Acpi Platform driver.

 @param  ImageHandle
 @param  SystemTable

 @return EFI_SUCCESS
 @return EFI_LOAD_ERROR
 @return EFI_OUT_OF_RESOURCES

 **/
EFI_STATUS
EFIAPI
AcpiPlatformEntryPoint (
                        IN EFI_HANDLE         ImageHandle,
                        IN EFI_SYSTEM_TABLE   *SystemTable
                        )
{
  EFI_STATUS                      Status;
  EFI_ACPI_TABLE_PROTOCOL         *AcpiTable;
  //  INTN                            Instance;
  //  EFI_ACPI_COMMON_HEADER          *CurrentTable;
  EFI_ACPI_COMMON_HEADER      *oldDSDT;
  UINTN                           TableHandle;
  UINTN                           TableSize;
  //  UINTN                           Size;
#if READTABLES
  UINTN              Index;
  CHAR16*              FileName;
#if LIP
  EFI_LOADED_IMAGE_PROTOCOL    *LoadedImage;
#endif
  VOID              *FileBuffer;
  //  VOID**              TmpHandler;
  UINT64              FileSize;
  UINTN              BufferSize;
  //  UINTN              Key;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
  EFI_FILE_INFO                   *Info;
  EFI_FILE_HANDLE                 Root = NULL;
  EFI_FILE_HANDLE                 ThisFile = NULL;
#endif
  EFI_PHYSICAL_ADDRESS      *Acpi20;
  EFI_PEI_HOB_POINTERS      GuidHob;
  EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER *Rsdp;
  //  EFI_ACPI_DESCRIPTION_HEADER *Rsdt, *Xsdt;
  //  EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE *Fadt;
  EFI_ACPI_DESCRIPTION_HEADER    *Table;
  SIGNAT              Signature;
  //  EFI_ACPI_TABLE_INSTANCE      *AcpiInstance;

  Msg = NULL;
  Status = gBS->LocateProtocol(&gMsgLogProtocolGuid, NULL, (VOID **) &Msg);
  if (!EFI_ERROR(Status) && (Msg != NULL)) {
    msgCursor = Msg->Cursor;
    BootLog("MsgLog Protocol installed in AcpiPlatform\n");
  }

  //
  // Find the AcpiTable protocol
  //
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID**)&AcpiTable);
  if (EFI_ERROR(Status)) {
    return EFI_ABORTED;
  }
#if DEBUG_ACPI
  AcpiInstance = EFI_ACPI_TABLE_INSTANCE_FROM_THIS(AcpiTable);
  DBG(L"Rsdp1 %x\n", AcpiInstance->Rsdp1);
  DBG(L"Rsdp3 %x\n", AcpiInstance->Rsdp3);
  DBG(L"Rsdt1 %x\n", AcpiInstance->Rsdt1);
  DBG(L"Rsdt3 %x\n", AcpiInstance->Rsdt3);
  DBG(L"Xsdt  %x\n", AcpiInstance->Xsdt);
  DBG(L"Fadt1 %x\n", AcpiInstance->Fadt1);
  DBG(L"Fadt3 %x\n", AcpiInstance->Fadt3);
#endif
  //  Instance     = 0;
  //  CurrentTable = NULL;
  TableHandle  = 0;

  GuidHob.Raw = GetFirstGuidHob (&gEfiAcpiTableGuid);
  if (GuidHob.Raw == NULL) {
    GuidHob.Raw = GetFirstGuidHob (&gEfiAcpi10TableGuid);
    if (GuidHob.Raw == NULL) {
      return EFI_ABORTED;
    }
    //Slice: TODO if we found only Acpi1.0 we need to convert it to Acpi2.0
    // like I did in Chameleon
  }
  Acpi20 = GET_GUID_HOB_DATA (GuidHob.Guid);
  if (Acpi20 == NULL) {
    return EFI_ABORTED;
  }
  Rsdp = (EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER *)(UINTN)*Acpi20;
  DBG(L"Rsdp @ %x\n", (UINTN)Rsdp);
  DBG(L"Rsdt @ %x\n", (UINTN)(Rsdp->RsdtAddress));
  DBG(L"Xsdt @ %x\n", (UINTN)(Rsdp->XsdtAddress));

  InstallLegacyTables(AcpiTable, Rsdp);
  //  DBG(L"LegacyTables installed\n");
  oldDSDT = (EFI_ACPI_COMMON_HEADER*)(UINTN)Fadt->Dsdt;
  DBG(L"Fadt @ %x\n", (UINTN)Fadt);
  DBG(L"oldDSDT @ %x\n", (UINTN)oldDSDT);

#if READTABLES
#if LIP
  //  Looking for a volume from what we boot

  /*  TODO - look for a volume we want to boot System
   it is possible if we fix in BdsBoot.c
   gRT->SetVariable (
   L"BootCurrent",
   &gEfiGlobalVariableGuid,
   EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
   sizeof (UINT16),
   &Option->BootCurrent
   );
   gRT->GetVariable (
   L"BootNext",
   &gEfiGlobalVariableGuid,
   EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
   0,
   &BootNext
   );
   and extract DevicePath from BootNext - first available :(
   In Gui.efi we can repeat this patch with DSDT.aml loaded from another place
   */

  Status = gBS->HandleProtocol (
                                ImageHandle,
                                &gEfiLoadedImageProtocolGuid,
                                (VOID*)&LoadedImage
                                );
  if (EFI_ERROR(Status)) {
    return EFI_ABORTED;
  }
  Status = gBS->HandleProtocol (
                                LoadedImage->DeviceHandle,
                                &gEfiSimpleFileSystemProtocolGuid,
                                (VOID *) &Volume
                                );
  if (EFI_ERROR(Status)) {
    return EFI_ABORTED;
  }

  //    DBG(L"Volume found\n");
  //
  // Open the root directory of the volume
  //
  if (!EFI_ERROR(Status)) {
    Status = Volume->OpenVolume (Volume, &Root);
  }
#else //Multiple FS protocols
  EFI_HANDLE            *mFs = NULL;
  UINTN                 mFsCount = 0;
  // mFsInfo[] array entries must match mFs[] handles
  EFI_FILE_SYSTEM_INFO  **mFsInfo = NULL;

  gBS->LocateHandleBuffer (ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &mFsCount, &mFs);
  mFsInfo = AllocateZeroPool(mFsCount * sizeof (EFI_FILE_SYSTEM_INFO *));
  if (mFsInfo == NULL) {
    // If we can't do this then we can't support file system entries
    mFsCount = 0;
  } else {
    // Loop through all the file system structures and cache the file system info data
    for (Index =0; Index < mFsCount; Index++) {
      Status = gBS->HandleProtocol (mFs[Index], &gEfiSimpleFileSystemProtocolGuid, (VOID **)&Volume);
      if (!EFI_ERROR(Status)) {
        Status = Volume->OpenVolume (Volume, &Root);
        if (!EFI_ERROR(Status)) {
          // Get information about the volume
          /*          Size = 0;
           Status = Root->GetInfo (Root, &gEfiFileSystemInfoGuid, &Size, mFsInfo[Index]);
           if (Status == EFI_BUFFER_TOO_SMALL) {
           mFsInfo[Index] = AllocatePool (Size);
           Status = Root->GetInfo (Root, &gEfiFileSystemInfoGuid, &Size, mFsInfo[Index]);
           }
           */
          //          Root->Close (Root);
          break; //I will stop at first volume
          //TODO try to find DSDT in all volumes
        }
      }
    }
  }
#endif
  FileName = AllocateZeroPool(32); //Should be enough
  //
  // Read tables from the first volume.
  //
  for (Index=0; Index<NUM_TABLES; Index++) {
    StrCpyS(FileName, 32, ACPInames[Index]);
    //    DBG(L"File probe %s\n", FileName);
    Status = Root->Open (Root, &ThisFile, FileName, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
      continue;
    }
    /* Get right size we need to allocate */
    Status = ThisFile->GetInfo (
                                ThisFile,
                                &gEfiFileInfoGuid,
                                &BufferSize,
                                Info
                                );
    if (EFI_ERROR(Status) && Status != EFI_BUFFER_TOO_SMALL) {
      continue;
    }
    DBG(L"Buffer size %d\n", BufferSize);
    //    DBG(L"GetInfo success!\n");
    Status = gBS->AllocatePool (EfiBootServicesData, BufferSize, (VOID **) &Info);
    if (EFI_ERROR(Status)) {
      //      DBG(L"No pool!\n");
      continue;
    }
    Status = ThisFile->GetInfo (
                                ThisFile,
                                &gEfiFileInfoGuid,
                                &BufferSize,
                                Info
                                );
    FileSize = Info->FileSize;
    //        DBG(L"FileSize = %d!\n", FileSize);
    gBS->FreePool(Info);
    //Slice - this is the problem.
    //    FileBuffer = AllocatePool(FileSize);
    Status = gBS->AllocatePool (EfiBootServicesData, FileSize, (VOID **) &FileBuffer);
    if (EFI_ERROR(Status)) {
      //      DBG(L"No pool for FileBuffer size %d!\n", FileSize);
      continue;
    }
    /*    Status = gBS->AllocatePages (
     AllocateMaxAddress,
     EfiACPIMemoryNVS,
     EFI_SIZE_TO_PAGES(FileSize),
     FileBuffer
     );
     if (EFI_ERROR(Status)) {
     //      DBG(L"No pool for FileBuffer size %d!\n", FileSize);
     continue;
     }
     */

    //should use ACPI memory
    //    Status=gBS->AllocatePages(AllocateMaxAddress,
    //          EfiACPIReclaimMemory,RoundPage(FileSize)/EFI_PAGE_SIZE, FileBuffer);
    DBG(L"FileBuffer @ %x\n", (UINTN)FileBuffer);

    Status = ThisFile->Read (ThisFile, &FileSize, FileBuffer); //(VOID**)&
    //    DBG(L"FileRead status=%x\n",   Status);
    if (!EFI_ERROR(Status)) {
      //
      // Add the table
      //
      //      TableHandle = 0;
      if (ThisFile != NULL) {
        ThisFile->Close (ThisFile); //close file before use buffer?! Flush?!
      }

      //      DBG(L"FileRead success: %c%c%c%c\n",
      //          ((CHAR8*)FileBuffer)[0], ((CHAR8*)FileBuffer)[1], ((CHAR8*)FileBuffer)[2], ((CHAR8*)FileBuffer)[3]);
      TableSize = ((EFI_ACPI_DESCRIPTION_HEADER *) FileBuffer)->Length;
      //ASSERT (BufferSize >= TableSize);
      DBG(L"Table size=%d\n", TableSize);
      if (FileSize < TableSize) {
        //Data incorrect. What TODO? Quick fix
        //        ((EFI_ACPI_DESCRIPTION_HEADER *) FileBuffer)->Length = FileSize;
        //        TableSize = FileSize;
        DBG(L"Table size > file size :(\n");
        continue; //do nothing with broken table
      }
      //
      // Checksum ACPI table
      //
      AcpiPlatformChecksum ((UINT8*)FileBuffer, TableSize);
      if ((Index==0) && oldDSDT) {  //DSDT always at index 0
        if (((EFI_ACPI_DESCRIPTION_HEADER *) oldDSDT)->Length > TableSize) {
          CopyMem(oldDSDT, FileBuffer, TableSize);
          DBG(L"New DSDT copied to old place\n");
        }
      }
      //
      // Install ACPI table
      //
      //TmpHandler = &FileBuffer;
      Status = AcpiTable->InstallAcpiTable (
                                            AcpiTable,
                                            FileBuffer,
                                            TableSize,
                                            &TableHandle
                                            );

      DBG(L"Table install status=%x\n",   Status);
      if (EFI_ERROR(Status)) {
        continue;
      }
      //      DBG(L"Table installed #%d\n", Index);
      //
      // Increment the instance
      //
      //      Instance++;   //for a what?
      FileBuffer = NULL;

    } else if (oldDSDT && (Index==0)) {
      //if new DSDT not found then install legacy one
      Table = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(Fadt->Dsdt));
      TableSize = Table->Length;
      Signature.Sign = Table->Signature;
      DBG(L"Install legacy table: %c%c%c%c\n",
          Signature.ASign[0], Signature.ASign[1], Signature.ASign[2], Signature.ASign[3]);

      Status = AcpiTable->InstallAcpiTable (
                                            AcpiTable,
                                            Table,
                                            TableSize,
                                            &TableHandle
                                            );
    }
  }
  if (Root != NULL) {
    Root->Close (Root);
  }
#else
  //just install legacy tables
  if (oldDSDT) {
    //if new DSDT not found then install legacy one
    Table = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(Fadt->Dsdt));
    TableSize = Table->Length;
    Signature.Sign = Table->Signature;
    DBG(L"Install legacy table: %c%c%c%c\n",
        Signature.ASign[0], Signature.ASign[1], Signature.ASign[2], Signature.ASign[3]);

    /*Status = */AcpiTable->InstallAcpiTable (
                                              AcpiTable,
                                              Table,
                                              TableSize,
                                              &TableHandle
                                              );
  }

#endif
#if DEBUG_ACPI==2
  gBS->Stall(5000000);
#endif

  return EFI_SUCCESS;
}


