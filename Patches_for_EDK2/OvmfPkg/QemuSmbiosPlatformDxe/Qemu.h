/** @file
  This header file provides QEMU-specific public prototypes for the main driver
  file, "SmbiosPlatformDxe.c".

  Copyright (C) 2013, Red Hat, Inc.

  This program and the accompanying materials are licensed and made available
  under the terms and conditions of the BSD License which accompanies this
  distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, WITHOUT
  WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef _QEMU_H_
#define _QEMU_H_

#include <Protocol/Smbios.h>


/**
  Fetch and install SMBIOS tables on the QEMU hypervisor.

  First, tables provided by QEMU in entirety are installed verbatim.

  Then the function prepares some of the remaining tables required by the
  SMBIOS-2.7.1 specification. For each such table,
  - if QEMU provides any fields for the table, they take effect verbatim,
  - remaining fields are set by this function.

  @param[in] Smbios       The EFI_SMBIOS_PROTOCOL instance used for installing
                          the SMBIOS tables.
  @param[in] ImageHandle  The image handle of the calling module, passed as
                          ProducerHandle to the Smbios->Add() call.

  @retval EFI_SUCCESS            All tables have been installed.
  @retval EFI_UNSUPPORTED        The pair (Smbios->MajorVersion,
                                 Smbios->MinorVersion) precedes (2, 3)
                                 lexicographically.
  @return                        Error codes returned by Smbios->Add() or
                                 internal functions. Some tables may not have
                                 been installed or fully patched.
**/
EFI_STATUS
EFIAPI
InstallQemuSmbiosTables (
  IN EFI_SMBIOS_PROTOCOL *Smbios,
  IN EFI_HANDLE          ImageHandle
  );

#endif
