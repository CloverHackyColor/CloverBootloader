/** @file
  Install the default Type 1 SMBIOS table if QEMU doesn't provide one through
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
// Text strings (unformatted area) for the default Tpe 1 SMBIOS table.
//
// All possible strings must be provided because Smbios->UpdateString() can
// only update existing strings, it can't introduce new ones.
//
#define OVMF_TYPE1_STRINGS                            \
          "QEMU\0"                 /* Manufacturer */ \
          "QEMU Virtual Machine\0" /* ProductName */  \
          "n/a\0"                  /* Version */      \
          "n/a\0"                  /* SerialNumber */ \
          "n/a\0"                  /* SKUNumber */    \
          "n/a\0"                  /* Family */


//
// Type definition and contents of the default Type 1 SMBIOS table.
//
#pragma pack(1)
OVMF_SMBIOS (1);
#pragma pack()

STATIC CONST OVMF_TYPE1 mOvmfType1 = {
  {
    // SMBIOS_STRUCTURE Hdr
    {
      EFI_SMBIOS_TYPE_SYSTEM_INFORMATION, // UINT8 Type
      sizeof (SMBIOS_TABLE_TYPE1)         // UINT8 Length
    },
    1,                           // SMBIOS_TABLE_STRING Manufacturer
    2,                           // SMBIOS_TABLE_STRING ProductName
    3,                           // SMBIOS_TABLE_STRING Version
    4,                           // SMBIOS_TABLE_STRING SerialNumber
    { 0 },                       // GUID                Uuid
    SystemWakeupTypePowerSwitch, // UINT8               WakeUpType
    5,                           // SMBIOS_TABLE_STRING SKUNumber
    6,                           // SMBIOS_TABLE_STRING Family
  },
  OVMF_TYPE1_STRINGS
};


/**
  Install default (fallback) table for SMBIOS Type 1.

  In case QEMU has provided no Type 1 SMBIOS table in whole, prepare one here,
  patch it with any referring saved patches, and install it.

  @param[in]     Smbios          The EFI_SMBIOS_PROTOCOL instance used for
                                 installing SMBIOS tables.
  @param[in]     ProducerHandle  Passed on to Smbios->Add(), ProducerHandle
                                 tracks the origin of installed SMBIOS tables.
  @param[in,out] Context         The BUILD_CONTEXT object tracking installed
                                 tables and saved patches.

  @retval EFI_SUCCESS  A Type 1 table has already been installed from the
                       SMBIOS firmware configuration blob.
  @retval EFI_SUCCESS  No Type 1 table was installed previously, and installing
                       the default here has succeeded.
  @return              Error codes from the PATCH_FORMATTED() and
                       PATCH_UNFORMATTED() macros, except EFI_NOT_FOUND, which
                       is only an informative result of theirs.
**/
EFI_STATUS
EFIAPI
InstallSmbiosType1 (
  IN     EFI_SMBIOS_PROTOCOL *Smbios,
  IN     EFI_HANDLE          ProducerHandle,
  IN OUT BUILD_CONTEXT       *Context
  )
{
  TABLE_CONTEXT     *Table;
  OVMF_TYPE1        OvmfType1;
  EFI_STATUS        Status;
  EFI_SMBIOS_HANDLE SmbiosHandle;

  Table = &Context->Table[1];
  if (Table->Installed) {
    return EFI_SUCCESS;
  }

  CopyMem (&OvmfType1, &mOvmfType1, sizeof OvmfType1);

  QemuFwCfgSelectItem (QemuFwCfgItemSystemUuid);
  OvmfType1.Base.Uuid.Data1 = SwapBytes32 (QemuFwCfgRead32 ());
  OvmfType1.Base.Uuid.Data2 = SwapBytes16 (QemuFwCfgRead16 ());
  OvmfType1.Base.Uuid.Data3 = SwapBytes16 (QemuFwCfgRead16 ());
  QemuFwCfgReadBytes (sizeof OvmfType1.Base.Uuid.Data4,
    &OvmfType1.Base.Uuid.Data4);

  //
  // Default contents ready. Formatted fields must be patched before installing
  // the table, while strings in the unformatted area will be patched
  // afterwards.
  //
  Status = PATCH_FORMATTED (Context, 1, &OvmfType1, Uuid);
  switch (Status) {
  case EFI_NOT_FOUND:
    break;
  case EFI_SUCCESS:
    OvmfType1.Base.Uuid.Data1 = SwapBytes32 (OvmfType1.Base.Uuid.Data1);
    OvmfType1.Base.Uuid.Data2 = SwapBytes16 (OvmfType1.Base.Uuid.Data2);
    OvmfType1.Base.Uuid.Data3 = SwapBytes16 (OvmfType1.Base.Uuid.Data3);
    break;
  default:
    return Status;
  }

  Status = PATCH_FORMATTED (Context, 1, &OvmfType1, WakeUpType);
  if (Status != EFI_NOT_FOUND && Status != EFI_SUCCESS) {
    return Status;
  }

  //
  // Install SMBIOS table with patched formatted area and default strings.
  //
  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  Status = Smbios->Add (Smbios, ProducerHandle, &SmbiosHandle,
                     (EFI_SMBIOS_TABLE_HEADER *) &OvmfType1);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Smbios->Add(): %r\n", __FUNCTION__, Status));
    return Status;
  }
  Table->Installed = TRUE;

  //
  // Patch strings in the unformatted area of the installed table.
  //
  Status = PATCH_UNFORMATTED (Smbios, SmbiosHandle, Context, 1, &OvmfType1,
             Manufacturer);
  if (Status != EFI_NOT_FOUND && Status != EFI_SUCCESS) {
    return Status;
  }
  Status = PATCH_UNFORMATTED (Smbios, SmbiosHandle, Context, 1, &OvmfType1,
             ProductName);
  if (Status != EFI_NOT_FOUND && Status != EFI_SUCCESS) {
    return Status;
  }
  Status = PATCH_UNFORMATTED (Smbios, SmbiosHandle, Context, 1, &OvmfType1,
             Version);
  if (Status != EFI_NOT_FOUND && Status != EFI_SUCCESS) {
    return Status;
  }
  Status = PATCH_UNFORMATTED (Smbios, SmbiosHandle, Context, 1, &OvmfType1,
             SerialNumber);
  if (Status != EFI_NOT_FOUND && Status != EFI_SUCCESS) {
    return Status;
  }
  Status = PATCH_UNFORMATTED (Smbios, SmbiosHandle, Context, 1, &OvmfType1,
             SKUNumber);
  if (Status != EFI_NOT_FOUND && Status != EFI_SUCCESS) {
    return Status;
  }
  Status = PATCH_UNFORMATTED (Smbios, SmbiosHandle, Context, 1, &OvmfType1,
             Family);
  if (Status != EFI_NOT_FOUND && Status != EFI_SUCCESS) {
    return Status;
  }
  return EFI_SUCCESS;
}
