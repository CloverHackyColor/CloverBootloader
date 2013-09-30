/** @file
  Install the default Type 0 SMBIOS table if QEMU doesn't provide one through
  the firmware configuration interface.

  Copyright (C) 2013, Red Hat, Inc.

  This program and the accompanying materials are licensed and made available
  under the terms and conditions of the BSD License which accompanies this
  distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, WITHOUT
  WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include "QemuInternal.h"


//
// Text strings (unformatted area) for the default Tpe 0 SMBIOS table.
//
// All possible strings must be provided because Smbios->UpdateString() can
// only update existing strings, it can't introduce new ones.
//
#define OVMF_TYPE0_STRINGS                                        \
          "EFI Development Kit II / OVMF\0" /* Vendor */          \
          "0.1\0"                           /* BiosVersion */     \
          "06/03/2013\0"                    /* BiosReleaseDate */


//
// Type definition and contents of the default Type 0 SMBIOS table.
//
#pragma pack(1)
OVMF_SMBIOS (0);
#pragma pack()

STATIC CONST OVMF_TYPE0 mOvmfType0 = {
  {
    // SMBIOS_STRUCTURE Hdr
    {
      EFI_SMBIOS_TYPE_BIOS_INFORMATION, // UINT8 Type
      sizeof (SMBIOS_TABLE_TYPE0)       // UINT8 Length
    },
    1,     // SMBIOS_TABLE_STRING       Vendor
    2,     // SMBIOS_TABLE_STRING       BiosVersion
    0xE800,// UINT16                    BiosSegment
    3,     // SMBIOS_TABLE_STRING       BiosReleaseDate
    0,     // UINT8                     BiosSize
    { 0 }, // MISC_BIOS_CHARACTERISTICS BiosCharacteristics
    { 0 }, // UINT8                     BIOSCharacteristicsExtensionBytes[2]
    0,     // UINT8                     SystemBiosMajorRelease
    1,     // UINT8                     SystemBiosMinorRelease
    0xFF,  // UINT8                     EmbeddedControllerFirmwareMajorRelease
    0xFF   // UINT8                     EmbeddedControllerFirmwareMinorRelease
  },
  OVMF_TYPE0_STRINGS
};


/**
  Install default (fallback) table for SMBIOS Type 0.

  In case QEMU has provided no Type 0 SMBIOS table in whole, prepare one here,
  patch it with any referring saved patches, and install it.

  @param[in]     Smbios          The EFI_SMBIOS_PROTOCOL instance used for
                                 installing SMBIOS tables.
  @param[in]     ProducerHandle  Passed on to Smbios->Add(), ProducerHandle
                                 tracks the origin of installed SMBIOS tables.
  @param[in,out] Context         The BUILD_CONTEXT object tracking installed
                                 tables and saved patches.

  @retval EFI_SUCCESS  A Type 0 table has already been installed from the
                       SMBIOS firmware configuration blob.
  @retval EFI_SUCCESS  No Type 0 table was installed previously, and installing
                       the default here has succeeded.
  @return              Error codes from the PATCH_FORMATTED() and
                       PATCH_UNFORMATTED() macros, except EFI_NOT_FOUND, which
                       is only an informative result of theirs.
**/
EFI_STATUS
EFIAPI
InstallSmbiosType0 (
  IN     EFI_SMBIOS_PROTOCOL *Smbios,
  IN     EFI_HANDLE          ProducerHandle,
  IN OUT BUILD_CONTEXT       *Context
  )
{
  TABLE_CONTEXT                       *Table;
  OVMF_TYPE0                          OvmfType0;
  MISC_BIOS_CHARACTERISTICS_EXTENSION *Ext;
  EFI_STATUS                          Status;
  EFI_SMBIOS_HANDLE                   SmbiosHandle;

  Table = &Context->Table[0];
  if (Table->Installed) {
    return EFI_SUCCESS;
  }

  CopyMem (&OvmfType0, &mOvmfType0, sizeof OvmfType0);
  Ext = (VOID *) &OvmfType0.Base.BIOSCharacteristicsExtensionBytes[0];

  OvmfType0.Base.BiosCharacteristics.BiosCharacteristicsNotSupported = 1;
  Ext->SystemReserved.UefiSpecificationSupported = 1;
  Ext->SystemReserved.VirtualMachineSupported    = 1;

  //
  // Default contents ready. Formatted fields must be patched before installing
  // the table, while strings in the unformatted area will be patched
  // afterwards.
  //
  Status = PATCH_FORMATTED (Context, 0, &OvmfType0, BiosSegment);
  if (Status != EFI_NOT_FOUND && Status != EFI_SUCCESS) {
    return Status;
  }
  Status = PATCH_FORMATTED (Context, 0, &OvmfType0, BiosSize);
  if (Status != EFI_NOT_FOUND && Status != EFI_SUCCESS) {
    return Status;
  }
  Status = PATCH_FORMATTED (Context, 0, &OvmfType0, BiosCharacteristics);
  if (Status != EFI_NOT_FOUND && Status != EFI_SUCCESS) {
    return Status;
  }
  Status = PATCH_FORMATTED (Context, 0, &OvmfType0,
             BIOSCharacteristicsExtensionBytes);
  if (Status != EFI_NOT_FOUND && Status != EFI_SUCCESS) {
    return Status;
  }
  Status = PATCH_FORMATTED (Context, 0, &OvmfType0, SystemBiosMajorRelease);
  if (Status != EFI_NOT_FOUND && Status != EFI_SUCCESS) {
    return Status;
  }
  Status = PATCH_FORMATTED (Context, 0, &OvmfType0, SystemBiosMinorRelease);
  if (Status != EFI_NOT_FOUND && Status != EFI_SUCCESS) {
    return Status;
  }
  Status = PATCH_FORMATTED (Context, 0, &OvmfType0,
             EmbeddedControllerFirmwareMajorRelease);
  if (Status != EFI_NOT_FOUND && Status != EFI_SUCCESS) {
    return Status;
  }
  Status = PATCH_FORMATTED (Context, 0, &OvmfType0,
             EmbeddedControllerFirmwareMinorRelease);
  if (Status != EFI_NOT_FOUND && Status != EFI_SUCCESS) {
    return Status;
  }

  //
  // Install SMBIOS table with patched formatted area and default strings.
  //
  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  Status = Smbios->Add (Smbios, ProducerHandle, &SmbiosHandle,
                     (EFI_SMBIOS_TABLE_HEADER *) &OvmfType0);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Smbios->Add(): %r\n", __FUNCTION__, Status));
    return Status;
  }
  Table->Installed = TRUE;

  //
  // Patch strings in the unformatted area of the installed table.
  //
  Status = PATCH_UNFORMATTED (Smbios, SmbiosHandle, Context, 0, &OvmfType0,
             Vendor);
  if (Status != EFI_NOT_FOUND && Status != EFI_SUCCESS) {
    return Status;
  }
  Status = PATCH_UNFORMATTED (Smbios, SmbiosHandle, Context, 0, &OvmfType0,
             BiosVersion);
  if (Status != EFI_NOT_FOUND && Status != EFI_SUCCESS) {
    return Status;
  }
  Status = PATCH_UNFORMATTED (Smbios, SmbiosHandle, Context, 0, &OvmfType0,
             BiosReleaseDate);
  if (Status != EFI_NOT_FOUND && Status != EFI_SUCCESS) {
    return Status;
  }
  return EFI_SUCCESS;
}
