/** @file
  Copyright (C) 2019, vit9696. All rights reserved.

  All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef OC_AFTER_BOOT_COMPAT_LIB_4CLOVER_H
#define OC_AFTER_BOOT_COMPAT_LIB_4CLOVER_H

#include <Library/OcAfterBootCompatLib.h>
/**
  Apple Boot Compatibility layer configuration.
**/
typedef struct OC_ABC_SETTINGS_4CLOVER
{
  OC_ABC_SETTINGS OcAbcSettings;

  CHAR8                **MmioWhitelistLabels; //null
  BOOLEAN              *MmioWhitelistEnabled; //null
} OC_ABC_SETTINGS_4CLOVER;

/**
  Initialize Apple Boot Compatibility layer. This layer is needed on partially
  incompatible firmwares to prevent boot failure and UEFI services breakage.

  @param[in]  Settings  Compatibility layer configuration.

  @retval EFI_SUCCESS on success.
**/
//EFI_STATUS
//OcAbcInitialize (
//  IN OC_ABC_SETTINGS_4CLOVER  *Settings
//  );

#endif // OC_AFTER_BOOT_COMPAT_LIB_H
