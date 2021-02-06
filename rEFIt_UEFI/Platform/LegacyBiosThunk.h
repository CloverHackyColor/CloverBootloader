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


#define EFI_SEGMENT(_Adr)     (UINT16) ((UINT16) (((UINTN) (_Adr)) >> 4) & 0xf000)
#define EFI_OFFSET(_Adr)      (UINT16) (((UINT16) ((UINTN) (_Adr))) & 0xffff)

// 8259 Interrupt Controller Hardware definitions

#define LEGACY_MODE_BASE_VECTOR_MASTER       0x08
#define LEGACY_MODE_BASE_VECTOR_SLAVE        0x70

#define PROTECTED_MODE_BASE_VECTOR_MASTER    0x68
#define PROTECTED_MODE_BASE_VECTOR_SLAVE     0x70

// 8254 Timer Hardware definitions

#define TIMER_CONTROL_PORT                   0x43
#define TIMER0_COUNT_PORT                    0x40
#define TIMER0_CONTROL_WORD                  0x36

//
// Function declarations
//
/*EFI_STATUS
EFIAPI
InitializeLegacyBiosThunk (
  IN EFI_HANDLE                       ImageHandle,
  IN EFI_SYSTEM_TABLE                 *SystemTable
  );*/

EFI_STATUS
InitializeBiosIntCaller (
//  THUNK_CONTEXT     *ThunkContext
  );

EFI_STATUS
InitializeInterruptRedirection (
//  IN  EFI_LEGACY_8259_PROTOCOL  *Legacy8259
  );

BOOLEAN
EFIAPI
LegacyBiosInt86 (
//  IN  EFI_LEGACY_BIOS_THUNK_PROTOCOL  *This,
  IN  UINT8                         BiosInt,
  IN  IA32_REGISTER_SET				*Regs
  );

BOOLEAN
EFIAPI
LegacyBiosFarCall86 (
//  IN  EFI_LEGACY_BIOS_THUNK_PROTOCOL  *This,
  IN  UINT16                        Segment,
  IN  UINT16                        Offset,
  IN  IA32_REGISTER_SET				*Regs //,
//  IN  void                          *Stack,
//  IN  UINTN                         StackSize
  );

#endif
