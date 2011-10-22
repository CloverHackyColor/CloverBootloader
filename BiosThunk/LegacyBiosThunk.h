/*++

Copyright (c) 2006 - 2008, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  LegacyBiosThunk.h

Abstract:

--*/
#ifndef _LEGACY_BIOS_THUNK_H_
#define _LEGACY_BIOS_THUNK_H_

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "EfiCommonLib.h"
#include "Thunk16Lib.h"

#include EFI_PROTOCOL_DEFINITION (LegacyBiosThunk)
#include EFI_PROTOCOL_CONSUMER (Legacy8259)

//
// Function declarations
//
EFI_STATUS
EFIAPI
InitializeLegacyBiosThunk (
  IN EFI_HANDLE                       ImageHandle,
  IN EFI_SYSTEM_TABLE                 *SystemTable
  );

VOID
InitializeBiosIntCaller (
  VOID
  );

VOID
InitializeInterruptRedirection (
  VOID
  );

BOOLEAN
EFIAPI
LegacyBiosInt86 (
  IN  EFI_LEGACY_BIOS_THUNK_PROTOCOL  *This,
  IN  UINT8                           BiosInt,
  IN  EFI_IA32_REGISTER_SET           *Regs
  );

BOOLEAN
EFIAPI
LegacyBiosFarCall86 (
  IN  EFI_LEGACY_BIOS_THUNK_PROTOCOL  *This,
  IN  UINT16                          Segment,
  IN  UINT16                          Offset,
  IN  EFI_IA32_REGISTER_SET           *Regs,
  IN  VOID                            *Stack,
  IN  UINTN                           StackSize
  );

#endif
