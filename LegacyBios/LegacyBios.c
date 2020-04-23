/** @file

Copyright (c) 2006 - 2011, Intel Corporation. All rights reserved.<BR>

This program and the accompanying materials
are licensed and made available under the terms and conditions
of the BSD License which accompanies this distribution.  The
full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "LegacyBiosInterface.h"

#define PHYSICAL_ADDRESS_TO_POINTER(Address)  ((VOID *) ((UINTN) Address))

//
// define maximum number of HDD system supports
//
#define MAX_HDD_ENTRIES 0x30

//
// Module Global:
//  Since this driver will only ever produce one instance of the Private Data
//  protocol you are not required to dynamically allocate the PrivateData.
//
LEGACY_BIOS_INSTANCE  mPrivateData;

/**
  Do an AllocatePages () of type AllocateMaxAddress for EfiBootServicesCode
  memory.

  @param  AllocateType               Allocated Legacy Memory Type
  @param  StartPageAddress           Start address of range
  @param  Pages                      Number of pages to allocate
  @param  Result                     Result of allocation

  @retval EFI_SUCCESS                Legacy16 code loaded
  @retval Other                      No protocol installed, unload driver.

**/
EFI_STATUS
AllocateLegacyMemory (
  IN  EFI_ALLOCATE_TYPE         AllocateType,
  IN  EFI_PHYSICAL_ADDRESS      StartPageAddress,
  IN  UINTN                     Pages,
  OUT EFI_PHYSICAL_ADDRESS      *Result
  )
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  MemPage;

  //
  // Allocate Pages of memory less <= StartPageAddress
  //
  MemPage = (EFI_PHYSICAL_ADDRESS) (UINTN) StartPageAddress;
  Status = gBS->AllocatePages (
                  AllocateType,
                  EfiBootServicesCode,
                  Pages,
                  &MemPage
                  );
  //
  // Do not ASSERT on Status error but let caller decide since some cases
  // memory is already taken but that is ok.
  //
  if (!EFI_ERROR(Status)) {
    *Result = (EFI_PHYSICAL_ADDRESS) (UINTN) MemPage;
  }
  //
  // If reach here the status = EFI_SUCCESS
  //
  return Status;
}


/**
  This function is called when EFI needs to reserve an area in the 0xE0000 or 0xF0000
  64 KB blocks.

  Note: inconsistency with the Framework CSM spec. Per the spec, this function may be
  invoked only once. This limitation is relaxed to allow multiple calls in this implemenation.

  @param  This                       Protocol instance pointer.
  @param  LegacyMemorySize           Size of required region
  @param  Region                     Region to use. 00 = Either 0xE0000 or 0xF0000 block
                                     Bit0 = 1 0xF0000 block
                                     Bit1 = 1 0xE0000 block
  @param  Alignment                  Address alignment. Bit mapped. First non-zero
                                     bit from right is alignment.
  @param  LegacyMemoryAddress        Region Assigned

  @retval EFI_SUCCESS                Region assigned
  @retval EFI_ACCESS_DENIED          Procedure previously invoked
  @retval Other                      Region not assigned

**/
EFI_STATUS
EFIAPI
LegacyBiosGetLegacyRegion (
  IN    EFI_LEGACY_BIOS_PROTOCOL *This,
  IN    UINTN                    LegacyMemorySize,
  IN    UINTN                    Region,
  IN    UINTN                    Alignment,
  OUT   VOID                     **LegacyMemoryAddress
  )
{

//  LEGACY_BIOS_INSTANCE  *Private;
  EFI_STATUS            Status;
  UINTN                 PagesBelow1MB;

//  Private = LEGACY_BIOS_INSTANCE_FROM_THIS (This);
  PagesBelow1MB = 0x000A0000 - 1;
  Status = gBS->AllocatePages (
                               AllocateMaxAddress,
                               EfiBootServicesData,
                               LegacyMemorySize,
                               &PagesBelow1MB
                               );

  *LegacyMemoryAddress = (VOID*)PagesBelow1MB;
  
  return Status;
}


/**
  This function is called when copying data to the region assigned by
  EFI_LEGACY_BIOS_PROTOCOL.GetLegacyRegion().

  @param  This                       Protocol instance pointer.
  @param  LegacyMemorySize           Size of data to copy
  @param  LegacyMemoryAddress        Legacy Region destination address Note: must
                                     be in region assigned by
                                     LegacyBiosGetLegacyRegion
  @param  LegacyMemorySourceAddress  Source of data

  @retval EFI_SUCCESS                The data was copied successfully.
  @retval EFI_ACCESS_DENIED          Either the starting or ending address is out of bounds.
**/
EFI_STATUS
EFIAPI
LegacyBiosCopyLegacyRegion (
  IN EFI_LEGACY_BIOS_PROTOCOL *This,
  IN    UINTN                 LegacyMemorySize,
  IN    VOID                  *LegacyMemoryAddress,
  IN    VOID                  *LegacyMemorySourceAddress
  )
{

  return EFI_ACCESS_DENIED;
}


/**
  Find Legacy16 BIOS image in the FLASH device and shadow it into memory. Find
  the $EFI table in the shadow area. Thunk into the Legacy16 code after it had
  been shadowed.

  @param  Private                    Legacy BIOS context data

  @retval EFI_SUCCESS                Legacy16 code loaded
  @retval Other                      No protocol installed, unload driver.

**/
EFI_STATUS
ShadowAndStartLegacy16 (
  IN  LEGACY_BIOS_INSTANCE  *Private
  )
{
  EFI_STATUS                        Status;
  UINT8                             *Ptr;
  UINT8                             *PtrEnd;
  BOOLEAN                           Done;
  EFI_COMPATIBILITY16_TABLE         *Table;
  UINT8                             CheckSum;
  EFI_IA32_REGISTER_SET             Regs;
  EFI_TO_COMPATIBILITY16_INIT_TABLE *EfiToLegacy16InitTable;
  EFI_TO_COMPATIBILITY16_BOOT_TABLE *EfiToLegacy16BootTable;
  VOID                              *LegacyBiosImage;
  UINTN                             LegacyBiosImageSize;
  UINTN                             E820Size;
  UINT32                            *ClearPtr;
  BBS_TABLE                         *BbsTable;
  LEGACY_EFI_HDD_TABLE              *LegacyEfiHddTable;
  UINTN                             Index;
  UINT32                            TpmPointer;
  VOID                              *TpmBinaryImage;
  UINTN                             TpmBinaryImageSize;
  UINTN                             Location;
  UINTN                             Alignment;
  UINTN                             TempData;
  EFI_PHYSICAL_ADDRESS              Address;
  UINT16                            OldMask;
  UINT16                            NewMask;
  UINT32                            Granularity;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR   Descriptor;

  Location  = 0;
  Alignment = 0;

  //
  // we allocate the C/D/E/F segment as RT code so no one will use it any more.
  //
  Address = 0xC0000;
  gDS->GetMemorySpaceDescriptor (Address, &Descriptor);
  if (Descriptor.GcdMemoryType == EfiGcdMemoryTypeSystemMemory) {
    //
    // If it is already reserved, we should be safe, or else we allocate it.
    //
    Status = gBS->AllocatePages (
                    AllocateAddress,
                    EfiRuntimeServicesCode,
                    0x40000/EFI_PAGE_SIZE,
                    &Address
                    );
    if (EFI_ERROR(Status)) {
      //
      // Bugbug: need to figure out whether C/D/E/F segment should be marked as reserved memory.
      // 
      DEBUG ((DEBUG_ERROR, "Failed to allocate the C/D/E/F segment Status = %r", Status));
    }
  }

  //
  // start testtest
  //    GetTimerValue (&Ticker);
  //
  //  gRT->SetVariable (L"StartLegacy",
  //                    &gEfiGlobalVariableGuid,
  //                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
  //                    sizeof (UINT64),
  //                    (VOID *)&Ticker
  //                    );
  // end testtest
  //
  EfiToLegacy16BootTable = &Private->IntThunk->EfiToLegacy16BootTable;
  LegacyBiosImageSize = 0x20000;
  
  Private->BiosStart            = (UINT32) (0x100000 - LegacyBiosImageSize);
  Private->OptionRom            = 0xc0000;
  Private->LegacyBiosImageSize  = (UINT32) LegacyBiosImageSize;

  //
  // Search for Legacy16 table in Shadowed ROM
  //
  Done  = FALSE;
  Table = NULL;
  //
  // Remember location of the Legacy16 table
  //
  Private->Legacy16Table            = Table;
  Private->Legacy16CallSegment      = 0;
  Private->Legacy16CallOffset       = 0;
  EfiToLegacy16InitTable            = NULL;
  Private->Legacy16InitPtr          = NULL;
  Private->Legacy16BootPtr          = NULL;
  Private->InternalIrqRoutingTable  = NULL;
  Private->NumberIrqRoutingEntries  = 0;
  Private->BbsTablePtr              = NULL;
  Private->LegacyEfiHddTable        = NULL;
  Private->DiskEnd                  = 0;
  Private->Disk4075                 = 0;
  Private->HddTablePtr              = NULL;
  Private->NumberHddControllers     = MAX_IDE_CONTROLLER;
  Private->Dump[0]                  = 'D';
  Private->Dump[1]                  = 'U';
  Private->Dump[2]                  = 'M';
  Private->Dump[3]                  = 'P';

  //
  // Store away a copy of the EFI System Table
  //
//  Table->EfiSystemTable = (UINT32) (UINTN) gST;

  //
  // IPF CSM integration -Bug
  //
  // Construct the Legacy16 boot memory map. This sets up number of
  // E820 entries.
  //
  LegacyBiosBuildE820 (Private, &E820Size);
  //
  // Initialize BDA and EBDA standard values needed to load Legacy16 code
  //
//  LegacyBiosInitBda (Private);
//  LegacyBiosInitCmos (Private);

  //
  // All legacy interrupt should be masked when do initialization work from legacy 16 code.
  //
  Private->Legacy8259->GetMask(Private->Legacy8259, &OldMask, NULL, NULL, NULL);
//  NewMask = 0xFFFF;
//  Private->Legacy8259->SetMask(Private->Legacy8259, &NewMask, NULL, NULL, NULL);
  
  //
  // Check if PCI Express is supported. If yes, Save base address.
  //
  Status = Private->LegacyBiosPlatform->GetPlatformInfo (
                                          Private->LegacyBiosPlatform,
                                          EfiGetPlatformPciExpressBase,
                                          NULL,
                                          NULL,
                                          &Location,
                                          &Alignment,
                                          0,
                                          0
                                          );
  if (!EFI_ERROR(Status)) {
    Private->Legacy16Table->PciExpressBase  = (UINT32)Location;
    Location = 0;
  }
  //
  // Check if TPM is supported. If yes get a region in E0000,F0000 to copy it
  // into, copy it and update pointer to binary image. This needs to be
  // done prior to any OPROM for security purposes.
  //
  Status = Private->LegacyBiosPlatform->GetPlatformInfo (
                                          Private->LegacyBiosPlatform,
                                          EfiGetPlatformBinaryTpmBinary,
                                          &TpmBinaryImage,
                                          &TpmBinaryImageSize,
                                          &Location,
                                          &Alignment,
                                          0,
                                          0
                                          );
  if (!EFI_ERROR(Status)) {
//    Table->TpmSegment = (UINT16)(Location >> 4) & 0xFFFF;
//    Table->TpmOffset  = (UINT16)(Location & 0xFFFF);
  }
  //
  // Lock the Legacy BIOS region
  //
  Private->Cpu->FlushDataCache (Private->Cpu, Private->BiosStart, (UINT32) LegacyBiosImageSize, EfiCpuFlushTypeWriteBackInvalidate);
  Private->LegacyRegion->Lock (Private->LegacyRegion, Private->BiosStart, (UINT32) LegacyBiosImageSize, &Granularity);

  return EFI_SUCCESS;
}

/**
  Shadow all legacy16 OPROMs that haven't been shadowed.
  Warning: Use this with caution. This routine disconnects all EFI
  drivers. If used externally then caller must re-connect EFI
  drivers.

  @param  This                    Protocol instance pointer.

  @retval EFI_SUCCESS             OPROMs shadowed

**/
EFI_STATUS
EFIAPI
LegacyBiosShadowAllLegacyOproms (
  IN EFI_LEGACY_BIOS_PROTOCOL *This
  )
{
//  LEGACY_BIOS_INSTANCE  *Private;

  //
  //  EFI_LEGACY_BIOS_PLATFORM_PROTOCOL    *LegacyBiosPlatform;
  //  EFI_LEGACY16_TABLE                   *Legacy16Table;
  //
//  Private = LEGACY_BIOS_INSTANCE_FROM_THIS (This);

  //
  //  LegacyBiosPlatform       = Private->LegacyBiosPlatform;
  //  Legacy16Table            = Private->Legacy16Table;
  //
  // Shadow PCI ROMs. We must do this near the end since this will kick
  // of Native EFI drivers that may be needed to collect info for Legacy16
  //
  //  WARNING: PciIo is gone after this call.
  //
//  PciProgramAllInterruptLineRegisters (Private);

//  PciShadowRoms (Private);

  //
  // Shadow PXE base code, BIS etc.
  //
  //  LegacyBiosPlatform->ShadowServiceRoms (LegacyBiosPlatform,
  //                       &Private->OptionRom,
  //                       Legacy16Table);
  //
  return EFI_SUCCESS;
}

/**
  Get the PCI BIOS interface version.

  @param  Private  Driver private data.

  @return The PCI interface version number in Binary Coded Decimal (BCD) format.
          E.g.: 0x0210 indicates 2.10, 0x0300 indicates 3.00

**/
UINT16
GetPciInterfaceVersion (
  IN LEGACY_BIOS_INSTANCE *Private
  )
{
  EFI_IA32_REGISTER_SET Reg;
  BOOLEAN               ThunkFailed;
  UINT16                PciInterfaceVersion;

  PciInterfaceVersion = 0;
  
  Reg.X.AX = 0xB101;
  Reg.E.EDI = 0;

  ThunkFailed = Private->LegacyBios.Int86 (&Private->LegacyBios, 0x1A, &Reg);
  if (!ThunkFailed) {
    //
    // From PCI Firmware 3.0 Specification:
    //   If the CARRY FLAG [CF] is cleared and AH is set to 00h, it is still necessary to examine the
    //   contents of [EDX] for the presence of the string "PCI" + (trailing space) to fully validate the
    //   presence of the PCI function set. [BX] will further indicate the version level, with enough
    //   granularity to allow for incremental changes in the code that don't affect the function interface.
    //   Version numbers are stored as Binary Coded Decimal (BCD) values. For example, Version 2.10
    //   would be returned as a 02h in the [BH] registers and 10h in the [BL] registers.
    //
    if ((Reg.X.Flags.CF == 0) && (Reg.H.AH == 0) && (Reg.E.EDX == SIGNATURE_32 ('P', 'C', 'I', ' '))) {
      PciInterfaceVersion = Reg.X.BX;
    }
  }
  return PciInterfaceVersion;
}

/**
  Install Driver to produce Legacy BIOS protocol.

  @param  ImageHandle  Handle of driver image.
  @param  SystemTable  Pointer to system table.

  @retval EFI_SUCCESS  Legacy BIOS protocol installed
  @retval No protocol installed, unload driver.

**/
EFI_STATUS
EFIAPI
LegacyBiosInstall (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                         Status;
  LEGACY_BIOS_INSTANCE               *Private;
  EFI_TO_COMPATIBILITY16_INIT_TABLE  *EfiToLegacy16InitTable;
  EFI_PHYSICAL_ADDRESS               MemoryAddress;
  VOID                               *MemoryPtr;
  EFI_PHYSICAL_ADDRESS               MemoryAddressUnder1MB;
  UINTN                              Index;
  UINT32                             *BaseVectorMaster;
  EFI_PHYSICAL_ADDRESS               StartAddress;
  UINT32                             *ClearPtr;
  EFI_PHYSICAL_ADDRESS               MemStart;
  UINT32                             IntRedirCode;
  UINT32                             Granularity;
  BOOLEAN                            DecodeOn;
  UINT32                             MemorySize;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR    Descriptor;
  UINT64                             Length;

  //
  // Load this driver's image to memory
  //
  Status = RelocateImageUnder4GIfNeeded (ImageHandle, SystemTable);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Private = &mPrivateData;
  ZeroMem (Private, sizeof (LEGACY_BIOS_INSTANCE));

  //
  // Grab a copy of all the protocols we depend on. Any error would
  // be a dispatcher bug!.
  //
  Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **) &Private->Cpu);
  ASSERT_EFI_ERROR(Status);

  Status = gBS->LocateProtocol (&gEfiTimerArchProtocolGuid, NULL, (VOID **) &Private->Timer);
  ASSERT_EFI_ERROR(Status);

  Status = gBS->LocateProtocol (&gEfiLegacyRegion2ProtocolGuid, NULL, (VOID **) &Private->LegacyRegion);
  ASSERT_EFI_ERROR(Status);

  Status = gBS->LocateProtocol (&gEfiLegacyBiosPlatformProtocolGuid, NULL, (VOID **) &Private->LegacyBiosPlatform);
  ASSERT_EFI_ERROR(Status);

  Status = gBS->LocateProtocol (&gEfiLegacy8259ProtocolGuid, NULL, (VOID **) &Private->Legacy8259);
  ASSERT_EFI_ERROR(Status);

  Status = gBS->LocateProtocol (&gEfiLegacyInterruptProtocolGuid, NULL, (VOID **) &Private->LegacyInterrupt);
  ASSERT_EFI_ERROR(Status);

  //
  // Locate Memory Test Protocol if exists
  //
  Status = gBS->LocateProtocol (
                  &gEfiGenericMemTestProtocolGuid,
                  NULL,
                  (VOID **) &Private->GenericMemoryTest
                  );
  ASSERT_EFI_ERROR(Status);

  //
  // Make sure all memory from 0-640K is tested
  //
  for (StartAddress = 0; StartAddress < 0xa0000; ) {
    gDS->GetMemorySpaceDescriptor (StartAddress, &Descriptor);
    if (Descriptor.GcdMemoryType != EfiGcdMemoryTypeReserved) {
      StartAddress = Descriptor.BaseAddress + Descriptor.Length;
      continue;
    }
    Length = MIN (Descriptor.Length, 0xa0000 - StartAddress);
    Private->GenericMemoryTest->CompatibleRangeTest (
                                  Private->GenericMemoryTest,
                                  StartAddress,
                                  Length
                                  );
    StartAddress = StartAddress + Length;
  }
  //
  // Make sure all memory from 1MB to 16MB is tested and added to memory map
  //
  for (StartAddress = BASE_1MB; StartAddress < BASE_16MB; ) {
    gDS->GetMemorySpaceDescriptor (StartAddress, &Descriptor);
    if (Descriptor.GcdMemoryType != EfiGcdMemoryTypeReserved) {
      StartAddress = Descriptor.BaseAddress + Descriptor.Length;
      continue;
    }
    Length = MIN (Descriptor.Length, BASE_16MB - StartAddress);
    Private->GenericMemoryTest->CompatibleRangeTest (
                                  Private->GenericMemoryTest,
                                  StartAddress,
                                  Length
                                  );
    StartAddress = StartAddress + Length;
  }

  Private->Signature = LEGACY_BIOS_INSTANCE_SIGNATURE;

  Private->LegacyBios.Int86 = LegacyBiosInt86;
  Private->LegacyBios.FarCall86 = LegacyBiosFarCall86;
  Private->LegacyBios.CheckPciRom = LegacyBiosCheckPciRom;
  Private->LegacyBios.InstallPciRom = LegacyBiosInstallPciRom;
  Private->LegacyBios.LegacyBoot = LegacyBiosLegacyBoot;
  Private->LegacyBios.UpdateKeyboardLedStatus = LegacyBiosUpdateKeyboardLedStatus;
  Private->LegacyBios.GetBbsInfo = LegacyBiosGetBbsInfo;
  Private->LegacyBios.ShadowAllLegacyOproms = LegacyBiosShadowAllLegacyOproms;
  Private->LegacyBios.PrepareToBootEfi = LegacyBiosPrepareToBootEfi;
  Private->LegacyBios.GetLegacyRegion = LegacyBiosGetLegacyRegion;
  Private->LegacyBios.CopyLegacyRegion = LegacyBiosCopyLegacyRegion;
  Private->LegacyBios.BootUnconventionalDevice = LegacyBiosBootUnconventionalDevice;

  Private->ImageHandle = ImageHandle;

  //
  // Enable read attribute of legacy region.
  //
  DecodeOn = TRUE;
  Private->LegacyRegion->Decode (
                           Private->LegacyRegion,
                           0xc0000,
                           0x40000,
                           &Granularity,
                           &DecodeOn
                           );
  //
  // Set Cachebility for legacy region
  // BUGBUG: Comments about this legacy region cacheability setting
  //         This setting will make D865GCHProduction CSM Unhappy
  //
  if (PcdGetBool (PcdLegacyBiosCacheLegacyRegion)) {
    gDS->SetMemorySpaceAttributes (
           0x0,
           0xA0000,
           EFI_MEMORY_WB
           );
    gDS->SetMemorySpaceAttributes (
           0xc0000,
           0x40000,
           EFI_MEMORY_UC  //EFI_MEMORY_WB
           );
  }

  gDS->SetMemorySpaceAttributes (
         0xA0000,
         0x20000,
         EFI_MEMORY_UC
         );

  //
  // Allocate 0 - 4K for real mode interupt vectors and BDA.
  //
  AllocateLegacyMemory (
    AllocateAddress,
    0,
    1,
    &MemoryAddress
    );
  ASSERT (MemoryAddress == 0x000000000);

//  ClearPtr = (VOID *) ((UINTN) 0x0000);

  //
  // Initialize region from 0x0000 to 4k. This initializes interrupt vector
  // range.
  //
//  SetMem((VOID *) ClearPtr, 0x400, INITIAL_VALUE_BELOW_1K);
//  ZeroMem ((VOID *) ((UINTN)ClearPtr + 0x400), 0xC00);

  //
  // Allocate space for thunker and Init Thunker
  //
  Status = AllocateLegacyMemory (
             AllocateMaxAddress,
             CONVENTIONAL_MEMORY_TOP,
             (sizeof (LOW_MEMORY_THUNK) / EFI_PAGE_SIZE) + 2,
             &MemoryAddress
             );
  ASSERT_EFI_ERROR(Status);
  Private->IntThunk                   = (LOW_MEMORY_THUNK *) (UINTN) MemoryAddress;
  EfiToLegacy16InitTable                   = &Private->IntThunk->EfiToLegacy16InitTable;
  EfiToLegacy16InitTable->ThunkStart       = (UINT32) (EFI_PHYSICAL_ADDRESS) (UINTN) MemoryAddress;
  EfiToLegacy16InitTable->ThunkSizeInBytes = (UINT32) (sizeof (LOW_MEMORY_THUNK));

  Status = LegacyBiosInitializeThunk (Private);
  ASSERT_EFI_ERROR(Status);

  //
  // Init the legacy memory map in memory < 1 MB.
  //
  EfiToLegacy16InitTable->BiosLessThan1MB         = (UINT32) MemoryAddressUnder1MB;
  EfiToLegacy16InitTable->LowPmmMemory            = (UINT32) MemoryAddressUnder1MB;
  EfiToLegacy16InitTable->LowPmmMemorySizeInBytes = MemorySize;

  //
  // Allocate high PMM Memory under 16 MB
  //
  MemorySize = PcdGet32 (PcdHighPmmMemorySize);
  ASSERT ((MemorySize & 0xFFF) == 0);    
  Status = AllocateLegacyMemory (
             AllocateMaxAddress,
             0x1000000,
             EFI_SIZE_TO_PAGES (MemorySize),
             &MemoryAddress
             );
  if (!EFI_ERROR(Status)) {
    EfiToLegacy16InitTable->HiPmmMemory            = (UINT32) (EFI_PHYSICAL_ADDRESS) (UINTN) MemoryAddress;
    EfiToLegacy16InitTable->HiPmmMemorySizeInBytes = MemorySize;
  }

  //
  //  ShutdownAPs();
  //
  //
  // Initialize interrupt redirection code and entries;
  // IDT Vectors 0x68-0x6f must be redirected to IDT Vectors 0x08-0x0f.
  //
  CopyMem (
         Private->IntThunk->InterruptRedirectionCode,
         (VOID *) (UINTN) InterruptRedirectionTemplate,
         sizeof (Private->IntThunk->InterruptRedirectionCode)
         );

  //
  // Save Unexpected interrupt vector so can restore it just prior to boot
  //
  BaseVectorMaster = (UINT32 *) (sizeof (UINT32) * PROTECTED_MODE_BASE_VECTOR_MASTER);
  Private->BiosUnexpectedInt = BaseVectorMaster[0];
  IntRedirCode = (UINT32) (UINTN) Private->IntThunk->InterruptRedirectionCode;
  for (Index = 0; Index < 8; Index++) {
    BaseVectorMaster[Index] = (EFI_SEGMENT (IntRedirCode + Index * 4) << 16) | EFI_OFFSET (IntRedirCode + Index * 4);
  }
  //
  // Save EFI value
  //
  Private->ThunkSeg = (UINT16) (EFI_SEGMENT (IntRedirCode));

  //
  // Make a new handle and install the protocol
  //
  Private->Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &Private->Handle,
                  &gEfiLegacyBiosProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &Private->LegacyBios
                  );
  Private->Csm16PciInterfaceVersion = GetPciInterfaceVersion (Private);
  
  ASSERT (Private->Csm16PciInterfaceVersion != 0);
  return Status;
}
