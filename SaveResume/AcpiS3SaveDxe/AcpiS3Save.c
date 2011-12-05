/** @file
  This is an implementation of the ACPI S3 Save protocol.  This is defined in
  S3 boot path specification 0.9.

Copyright (c) 2006 - 2011, Intel Corporation. All rights reserved.<BR>

This program and the accompanying materials
are licensed and made available under the terms and conditions
of the BSD License which accompanies this distribution.  The
full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/HobLib.h>
#include <Library/LockBoxLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Guid/AcpiVariableCompatibility.h>
#include <Guid/AcpiS3Context.h>
#include <Guid/Acpi.h>
#include <Protocol/AcpiS3Save.h>
#include <IndustryStandard/Acpi.h>

#include "AcpiS3Save.h"
// protocol
///
/// Gets the size of legacy memory below 1 MB that is required for S3 resume.
///EFI_ACPI_GET_LEGACY_MEMORY_SIZE   GetLegacyMemorySize;
///
/// Prepare all information for an S3 resume.
//EFI_ACPI_S3_SAVE                  S3Save;
///

UINTN     mLegacyRegionSize;

EFI_ACPI_S3_SAVE_PROTOCOL mS3Save = {
  LegacyGetS3MemorySize,  //GetLegacyMemorySize
  S3Ready,                //S3Save
};

EFI_GUID              mAcpiS3IdtrProfileGuid = {
	0xdea652b0, 0xd587, 0x4c54, {0xb5, 0xb4, 0xc6, 0x82, 0xe7, 0xa0, 0xaa, 0x3d}
};

/**
  Allocate EfiACPIMemoryNVS below 4G memory address.

  This function allocates EfiACPIMemoryNVS below 4G memory address.

  @param  Size         Size of memory to allocate.
  
  @return Allocated address for output.

**/
VOID*
AllocateAcpiNvsMemoryBelow4G (
  IN   UINTN   Size
  )
{
  UINTN                 Pages;
  EFI_PHYSICAL_ADDRESS  Address;
  EFI_STATUS            Status;
  VOID*                 Buffer;

  Pages = EFI_SIZE_TO_PAGES (Size);
  Address = 0xffffffff;

  Status  = gBS->AllocatePages (
                   AllocateMaxAddress,
                   EfiACPIMemoryNVS,
                   Pages,
                   &Address
                   );
  ASSERT_EFI_ERROR (Status);

  Buffer = (VOID *) (UINTN) Address;
  ZeroMem (Buffer, Size);

  return Buffer;
}

/**
  To find Facs in Acpi tables.
 
  To find Firmware ACPI control strutcure in Acpi Tables since the S3 waking vector is stored 
  in the table.

  @param AcpiTableGuid   The guid used to find ACPI table in UEFI ConfigurationTable.
  
  @return  Facs table pointer.
**/
EFI_ACPI_2_0_FIRMWARE_ACPI_CONTROL_STRUCTURE  *
FindAcpiFacsTableByAcpiGuid (
  IN EFI_GUID  *AcpiTableGuid
  )
{
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER  *Rsdp;
  EFI_ACPI_DESCRIPTION_HEADER                   *Rsdt;
  EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE     *Fadt;
  EFI_ACPI_2_0_FIRMWARE_ACPI_CONTROL_STRUCTURE  *Facs;
  UINTN                                         Index;
  UINT32                                        Data32;
  Rsdp  = NULL;
  Rsdt  = NULL;
  Fadt  = NULL;
  //
  // found ACPI table RSD_PTR from system table
  //
  for (Index = 0; Index < gST->NumberOfTableEntries; Index++) {
    if (CompareGuid (&(gST->ConfigurationTable[Index].VendorGuid), AcpiTableGuid)) {
      //
      // A match was found.
      //
      Rsdp = gST->ConfigurationTable[Index].VendorTable;
      break;
    }
  }

  if (Rsdp == NULL) {
    return NULL;
  }

  Rsdt = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN) Rsdp->RsdtAddress;
  if (Rsdt == NULL || Rsdt->Signature != EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
    Rsdt = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN) Rsdp->XsdtAddress;
    if (Rsdt == NULL || Rsdt->Signature != EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
      return NULL;
    }
  }

  for (Index = sizeof (EFI_ACPI_DESCRIPTION_HEADER); Index < Rsdt->Length; Index = Index + sizeof (UINT32)) {

    Data32  = *(UINT32 *) ((UINT8 *) Rsdt + Index);
    Fadt    = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE *) (UINT32 *) (UINTN) Data32;
    if (Fadt->Header.Signature == EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE) {
      break;
    }
  }

  if (Fadt == NULL || Fadt->Header.Signature != EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE) {
    return NULL;
  }

  Facs = (EFI_ACPI_2_0_FIRMWARE_ACPI_CONTROL_STRUCTURE *)(UINTN)Fadt->FirmwareCtrl;

  return Facs;
}

/**
  To find Facs in Acpi tables.
 
  To find Firmware ACPI control strutcure in Acpi Tables since the S3 waking vector is stored 
  in the table.
  
  @return  Facs table pointer.
**/
EFI_ACPI_2_0_FIRMWARE_ACPI_CONTROL_STRUCTURE  *
FindAcpiFacsTable (
  VOID
  )
{
  EFI_ACPI_2_0_FIRMWARE_ACPI_CONTROL_STRUCTURE *Facs;

  Facs = FindAcpiFacsTableByAcpiGuid (&gEfiAcpi20TableGuid);
  if (Facs != NULL) {
    return Facs;
  }

  return FindAcpiFacsTableByAcpiGuid (&gEfiAcpi10TableGuid);
}

/**
  Allocates and fills in the Page Directory and Page Table Entries to
  establish a 1:1 Virtual to Physical mapping.
  If BootScriptExector driver will run in 64-bit mode, this function will establish the 1:1 
  virtual to physical mapping page table.
  If BootScriptExector driver will not run in 64-bit mode, this function will do nothing. 
  
  @return  the 1:1 Virtual to Physical identity mapping page table base address. 

**/
EFI_PHYSICAL_ADDRESS
S3CreateIdentityMappingPageTables (
  VOID
  )
{  
    //
    // If DXE is running 32-bit mode, no need to establish page table.
    //
    return  (EFI_PHYSICAL_ADDRESS) 0;  
}

/**
  Gets the buffer of legacy memory below 1 MB 
  This function is to get the buffer in legacy memory below 1MB that is required during S3 resume.

  @param This           A pointer to the EFI_ACPI_S3_SAVE_PROTOCOL instance.
  @param Size           The returned size of legacy memory below 1 MB.

  @retval EFI_SUCCESS           Size is successfully returned.
  @retval EFI_INVALID_PARAMETER The pointer Size is NULL.

**/
EFI_STATUS
EFIAPI
LegacyGetS3MemorySize (
  IN  EFI_ACPI_S3_SAVE_PROTOCOL   *This,
  OUT UINTN                       *Size
  )
{
  if (Size == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *Size = mLegacyRegionSize;
  return EFI_SUCCESS;
}

/**
  Prepares all information that is needed in the S3 resume boot path.
  
  Allocate the resources or prepare informations and save in ACPI variable set for S3 resume boot path  
  
  @param This                 A pointer to the EFI_ACPI_S3_SAVE_PROTOCOL instance.
  @param LegacyMemoryAddress  The base address of legacy memory.

  @retval EFI_NOT_FOUND         Some necessary information cannot be found.
  @retval EFI_SUCCESS           All information was saved successfully.
  @retval EFI_OUT_OF_RESOURCES  Resources were insufficient to save all the information.
  @retval EFI_INVALID_PARAMETER The memory range is not located below 1 MB.

**/
EFI_STATUS
EFIAPI
S3Ready (
  IN EFI_ACPI_S3_SAVE_PROTOCOL    *This,
  IN VOID                         *LegacyMemoryAddress
  )
{
  EFI_STATUS                                    Status;
  EFI_PHYSICAL_ADDRESS                          AcpiS3ContextBuffer;
  ACPI_S3_CONTEXT                               *AcpiS3Context;
  STATIC BOOLEAN                                AlreadyEntered;
  IA32_DESCRIPTOR                               *Idtr;
  IA32_IDT_GATE_DESCRIPTOR                      *IdtGate;

//  DEBUG ((EFI_D_INFO, "S3Ready!\n"));

  //
  // Platform may invoke AcpiS3Save->S3Save() before ExitPmAuth,
  //	because we need save S3 information there, while BDS ReadyToBoot may invoke it again.
  // So if 2nd S3Save() is triggered later, we need ignore it.
  //
  if (AlreadyEntered) {
    return EFI_SUCCESS;
  }
  AlreadyEntered = TRUE;

  AcpiS3Context = AllocateAcpiNvsMemoryBelow4G (sizeof(*AcpiS3Context));
  ASSERT (AcpiS3Context != NULL);
  AcpiS3ContextBuffer = (EFI_PHYSICAL_ADDRESS)(UINTN)AcpiS3Context;

  //
  // Get ACPI Table because we will save its position to variable
  //
  AcpiS3Context->AcpiFacsTable = (EFI_PHYSICAL_ADDRESS)(UINTN)FindAcpiFacsTable ();
  ASSERT (AcpiS3Context->AcpiFacsTable != 0);

  IdtGate = AllocateAcpiNvsMemoryBelow4G (sizeof(IA32_IDT_GATE_DESCRIPTOR) * 0x100 + sizeof(IA32_DESCRIPTOR));
  Idtr = (IA32_DESCRIPTOR *)(IdtGate + 0x100);
  Idtr->Base  = (UINTN)IdtGate;
  Idtr->Limit = (UINT16)(sizeof(IA32_IDT_GATE_DESCRIPTOR) * 0x100 - 1);
  AcpiS3Context->IdtrProfile = (EFI_PHYSICAL_ADDRESS)(UINTN)Idtr;

  Status = SaveLockBox (
             &mAcpiS3IdtrProfileGuid,
             (VOID *)(UINTN)Idtr,
             (UINTN)sizeof(IA32_DESCRIPTOR)
             );
  ASSERT_EFI_ERROR (Status);

  Status = SetLockBoxAttributes (&mAcpiS3IdtrProfileGuid, LOCK_BOX_ATTRIBUTE_RESTORE_IN_PLACE);
  ASSERT_EFI_ERROR (Status);

  //
  // Allocate page table
  //
  AcpiS3Context->S3NvsPageTableAddress = S3CreateIdentityMappingPageTables ();

  //
  // Allocate stack
  //
  AcpiS3Context->BootScriptStackSize = PcdGet32 (PcdS3BootScriptStackSize);
  AcpiS3Context->BootScriptStackBase = (EFI_PHYSICAL_ADDRESS)(UINTN)AllocateAcpiNvsMemoryBelow4G (PcdGet32 (PcdS3BootScriptStackSize));
  ASSERT (AcpiS3Context->BootScriptStackBase != 0);

  //
  // Allocate a code buffer < 4G for S3 debug to load external code
  //
  AcpiS3Context->S3DebugBufferAddress = (EFI_PHYSICAL_ADDRESS)(UINTN)AllocateAcpiNvsMemoryBelow4G (EFI_PAGE_SIZE);

//  DEBUG((EFI_D_INFO, "AcpiS3Context: AcpiFacsTable is 0x%8x\n", AcpiS3Context->AcpiFacsTable));
//  DEBUG((EFI_D_INFO, "AcpiS3Context: IdtrProfile is 0x%8x\n", AcpiS3Context->IdtrProfile));
//  DEBUG((EFI_D_INFO, "AcpiS3Context: S3NvsPageTableAddress is 0x%8x\n", AcpiS3Context->S3NvsPageTableAddress));
//  DEBUG((EFI_D_INFO, "AcpiS3Context: S3DebugBufferAddress is 0x%8x\n", AcpiS3Context->S3DebugBufferAddress));

  Status = SaveLockBox (
             &gEfiAcpiVariableGuid,
             &AcpiS3ContextBuffer,
             sizeof(AcpiS3ContextBuffer)
             );
  ASSERT_EFI_ERROR (Status);

  Status = SaveLockBox (
             &gEfiAcpiS3ContextGuid,
             (VOID *)(UINTN)AcpiS3Context,
             (UINTN)sizeof(*AcpiS3Context)
             );
  ASSERT_EFI_ERROR (Status);

  Status = SetLockBoxAttributes (&gEfiAcpiS3ContextGuid, LOCK_BOX_ATTRIBUTE_RESTORE_IN_PLACE);
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

/**
  The Driver Entry Point.
  
  The function is the driver Entry point which will produce AcpiS3SaveProtocol.
  
  @param ImageHandle   A handle for the image that is initializing this driver
  @param SystemTable   A pointer to the EFI system table

  @retval EFI_SUCCESS:              Driver initialized successfully
  @retval EFI_LOAD_ERROR:           Failed to Initialize or has been loaded
  @retval EFI_OUT_OF_RESOURCES      Could not allocate needed resources

**/
EFI_STATUS
EFIAPI
InstallAcpiS3Save (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS        Status;

    //
    // More memory for no CSM tip, because GDT need relocation
    //
  mLegacyRegionSize = 0x250;

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gEfiAcpiS3SaveProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mS3Save
                  );
  ASSERT_EFI_ERROR (Status);
  return Status;
}
