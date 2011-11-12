/*++

Copyright (c) 2006 - 2008, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  LegacyBiosThunk.c

Abstract:

--*/

#include "LegacyBiosThunk.h"
#include "CpuIa32.h"

#define EFI_CPU_EFLAGS_IF 0x200

EFI_LEGACY_8259_PROTOCOL        *gLegacy8259 = NULL;
THUNK_CONTEXT                   mThunkContext;
EFI_LEGACY_BIOS_THUNK_PROTOCOL  mLegacyBiosThunk = {
  LegacyBiosInt86,
  LegacyBiosFarCall86
};

EFI_DRIVER_ENTRY_POINT (InitializeLegacyBiosThunk)

EFI_STATUS
EFIAPI
InitializeLegacyBiosThunk (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:
  Initialize the state information for the Simple CSM driver

Arguments:
  ImageHandle of the loaded driver
  Pointer to the System Table

Returns:
  EFI_SUCCESS           - thread can be successfully created
  EFI_OUT_OF_RESOURCES  - cannot allocate protocol data structure
  EFI_DEVICE_ERROR      - cannot create the thread

--*/
{
  EFI_STATUS                  Status;
  EFI_HANDLE                  Handle;

  EfiInitializeDriverLib (ImageHandle, SystemTable);

  //
  // Find the Legacy8259 protocol.
  //
  Status = gBS->LocateProtocol (&gEfiLegacy8259ProtocolGuid, NULL, (VOID **) &gLegacy8259);
  ASSERT_EFI_ERROR (Status);

  InitializeBiosIntCaller ();

  InitializeInterruptRedirection ();

  //
  // Install thunk protocol
  //
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiLegacyBiosThunkProtocolGuid,
                  &mLegacyBiosThunk,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

EFI_STATUS
GetFreeLegacyRegion (
  IN OUT UINTN    *LegacyRegionBase,
  IN OUT UINTN    *LegacyRegionLength
  )
/*++

Routine Description:

  Get a free Legacy Region.

Arguments:

  LegacyRegionBase   - return free legacy region base
  LegacyRegionLength - on input, it means the minimal length request,
                       on output, it means the length required.

Returns:

  EFI_SUCCESS   - Find a free legacy region and return.
  EFI_NOT_FOUND - Can not find a free legacy region.
  
--*/
{
  EFI_PHYSICAL_ADDRESS PhysicalBase;
  EFI_STATUS           Status;

  PhysicalBase = 0x100000;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiACPIMemoryNVS,
                  EFI_SIZE_TO_PAGES(*LegacyRegionLength),
                  &PhysicalBase
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  *LegacyRegionBase   = (UINTN)PhysicalBase;

  return EFI_SUCCESS;
}

VOID
InitializeBiosIntCaller (
  VOID
  )
/*++

  Routine Description:
    Initialize call infrastructure to 16-bit BIOS code

  Arguments:
    NONE

  Returns:
    NONE
--*/
{
  UINTN                 LegacyRegionBase;
  UINTN                 LegacyRegionLength;
  UINTN                 LegacyStackSize;
  EFI_STATUS            Status;

  //
  // Get LegacyRegion
  //
  LegacyRegionLength = AsmThunk16GetProperties (&LegacyStackSize);
  LegacyRegionLength += LegacyStackSize;
  Status = GetFreeLegacyRegion (&LegacyRegionBase, &LegacyRegionLength);
  ASSERT_EFI_ERROR (Status);

  //
  // Prepare environment
  //
  AsmThunk16SetProperties (
    &mThunkContext,
    (VOID*)LegacyRegionBase,
    LegacyRegionLength
    );

  return;
}


VOID
InitializeInterruptRedirection (
  VOID
  )
/*++

  Routine Description:
    Initialize interrupt redirection code and entries, because
    IDT Vectors 0x68-0x6f must be redirected to IDT Vectors 0x08-0x0f.
    Or the interrupt will lost when we do thunk.
    NOTE: We do not reset 8259 vector base, because it will cause pending
    interrupt lost.

  Arguments:
    NONE

  Returns:
    NONE
--*/
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  LegacyRegionBase;
  UINTN                 LegacyRegionLength;
  UINT32                *IdtArray;
  UINTN                 Index;
  UINT8                 ProtectedModeBaseVector;
  UINT32                InterruptRedirectionCode[] = {
    0x90CF08CD, // INT8; IRET; NOP
    0x90CF09CD, // INT9; IRET; NOP
    0x90CF0ACD, // INTA; IRET; NOP
    0x90CF0BCD, // INTB; IRET; NOP
    0x90CF0CCD, // INTC; IRET; NOP
    0x90CF0DCD, // INTD; IRET; NOP
    0x90CF0ECD, // INTE; IRET; NOP
    0x90CF0FCD  // INTF; IRET; NOP
  };

  //
  // Get LegacyRegion
  //
  LegacyRegionLength = sizeof(InterruptRedirectionCode);
  Status = GetFreeLegacyRegion (&LegacyRegionBase, &LegacyRegionLength);
  ASSERT_EFI_ERROR (Status);

  //
  // Copy code to legacy region
  //
  CopyMem ((VOID *)(UINTN)LegacyRegionBase, InterruptRedirectionCode, sizeof (InterruptRedirectionCode));

  //
  // Get VectorBase, it should be 0x68
  //
  Status = gLegacy8259->GetVector (gLegacy8259, Efi8259Irq0, &ProtectedModeBaseVector);
  ASSERT_EFI_ERROR (Status);

  //
  // Patch IVT 0x68 ~ 0x6f
  //
  IdtArray = (UINT32 *) 0;
  for (Index = 0; Index < 8; Index++) {
    IdtArray[ProtectedModeBaseVector + Index] = ((EFI_SEGMENT (LegacyRegionBase + Index * 4)) << 16) | (EFI_OFFSET (LegacyRegionBase + Index * 4));
  }

  return ;
}

BOOLEAN
EFIAPI
LegacyBiosInt86 (
  IN  EFI_LEGACY_BIOS_THUNK_PROTOCOL  *This,
  IN  UINT8                           BiosInt,
  IN  EFI_IA32_REGISTER_SET           *Regs
  )
/*++

  Routine Description:
    Thunk to 16-bit real mode and execute a software interrupt with a vector 
    of BiosInt. Regs will contain the 16-bit register context on entry and 
    exit.

  Arguments:
    This    - Protocol instance pointer.
    BiosInt - Processor interrupt vector to invoke
    Reg     - Register contexted passed into (and returned) from thunk to 
              16-bit mode

  Returns:
    FALSE   - Thunk completed, and there were no BIOS errors in the target code.
              See Regs for status.
    TRUE    - There was a BIOS erro in the target code.

--*/
{
  UINTN                 Status;
  UINT32                Eflags;
  IA32_REGISTER_SET     ThunkRegSet;
  BOOLEAN               Ret;
  UINT16                *Stack16;
 
  Regs->X.Flags.Reserved1 = 1;
  Regs->X.Flags.Reserved2 = 0;
  Regs->X.Flags.Reserved3 = 0;
  Regs->X.Flags.Reserved4 = 0;
  Regs->X.Flags.IOPL      = 3;
  Regs->X.Flags.NT        = 0;
  Regs->X.Flags.IF        = 1;
  Regs->X.Flags.TF        = 0;
  Regs->X.Flags.CF        = 0;

  ZeroMem (&ThunkRegSet, sizeof (ThunkRegSet));
  ThunkRegSet.E.EDI  = Regs->E.EDI;
  ThunkRegSet.E.ESI  = Regs->E.ESI;
  ThunkRegSet.E.EBP  = Regs->E.EBP;
  ThunkRegSet.E.EBX  = Regs->E.EBX;
  ThunkRegSet.E.EDX  = Regs->E.EDX;
  ThunkRegSet.E.ECX  = Regs->E.ECX;
  ThunkRegSet.E.EAX  = Regs->E.EAX;
  ThunkRegSet.E.DS   = Regs->E.DS;
  ThunkRegSet.E.ES   = Regs->E.ES;

  CopyMem (&(ThunkRegSet.E.EFLAGS), &(Regs->E.EFlags), sizeof (UINT32));
 
  //
  // The call to Legacy16 is a critical section to EFI
  //
  Eflags = AsmReadEflags ();
  if ((Eflags | EFI_CPU_EFLAGS_IF) != 0) {
    DisableInterrupts ();
  }

  //
  // Set Legacy16 state. 0x08, 0x70 is legacy 8259 vector bases.
  //
  Status = gLegacy8259->SetMode (gLegacy8259, Efi8259LegacyMode, NULL, NULL);
  ASSERT_EFI_ERROR (Status);
  
//  AsmThunk16Int86 (&mThunkContext, BiosInt, &ThunkRegSet, 0);
  Stack16 = (UINT16 *)((UINT8 *) mThunkContext->RealModeBuffer + mThunkContext->RealModeBufferSize - sizeof (UINT16));

  ThunkRegSet.E.SS   = (UINT16) (((UINTN) Stack16 >> 16) << 12);
  ThunkRegSet.E.ESP  = (UINT16) (UINTN) Stack16;

  ThunkRegSet.E.Eip  = (UINT16)((UINT32 *)NULL)[BiosInt];
  ThunkRegSet.E.CS   = (UINT16)(((UINT32 *)NULL)[BiosInt] >> 16);
  mThunkContext->RealModeState = &ThunkRegSet;
  AsmThunk16 (mThunkContext);


  //
  // Restore protected mode interrupt state
  //
  Status = gLegacy8259->SetMode (gLegacy8259, Efi8259ProtectedMode, NULL, NULL);
  ASSERT_EFI_ERROR (Status);

  //
  // End critical section
  //
  if ((Eflags | EFI_CPU_EFLAGS_IF) != 0) {
    EnableInterrupts ();
  }

  Regs->E.EDI      = ThunkRegSet.E.EDI;      
  Regs->E.ESI      = ThunkRegSet.E.ESI;  
  Regs->E.EBP      = ThunkRegSet.E.EBP;  
  Regs->E.EBX      = ThunkRegSet.E.EBX;  
  Regs->E.EDX      = ThunkRegSet.E.EDX;  
  Regs->E.ECX      = ThunkRegSet.E.ECX;  
  Regs->E.EAX      = ThunkRegSet.E.EAX;
  Regs->E.SS       = ThunkRegSet.E.SS;
  Regs->E.CS       = ThunkRegSet.E.CS;  
  Regs->E.DS       = ThunkRegSet.E.DS;  
  Regs->E.ES       = ThunkRegSet.E.ES;

  CopyMem (&(Regs->E.EFlags), &(ThunkRegSet.E.EFLAGS), sizeof (UINT32));

  Ret = (BOOLEAN) (Regs->E.EFlags.CF == 1);

  return Ret;
}

BOOLEAN
EFIAPI
LegacyBiosFarCall86 (
  IN  EFI_LEGACY_BIOS_THUNK_PROTOCOL  *This,
  IN  UINT16                          Segment,
  IN  UINT16                          Offset,
  IN  EFI_IA32_REGISTER_SET           *Regs,
  IN  VOID                            *Stack,
  IN  UINTN                           StackSize
  )
{
  UINTN                 Status;
  UINT32                Eflags;
  IA32_REGISTER_SET     ThunkRegSet;
  BOOLEAN               Ret;

  Regs->X.Flags.Reserved1 = 1;
  Regs->X.Flags.Reserved2 = 0;
  Regs->X.Flags.Reserved3 = 0;
  Regs->X.Flags.Reserved4 = 0;
  Regs->X.Flags.IOPL      = 3;
  Regs->X.Flags.NT        = 0;
  Regs->X.Flags.IF        = 1;
  Regs->X.Flags.TF        = 0;
  Regs->X.Flags.CF        = 0;

  EfiCommonLibZeroMem (&ThunkRegSet, sizeof (ThunkRegSet));
  ThunkRegSet.E.EDI  = Regs->E.EDI;
  ThunkRegSet.E.ESI  = Regs->E.ESI;
  ThunkRegSet.E.EBP  = Regs->E.EBP;
  ThunkRegSet.E.EBX  = Regs->E.EBX;
  ThunkRegSet.E.EDX  = Regs->E.EDX;
  ThunkRegSet.E.ECX  = Regs->E.ECX;
  ThunkRegSet.E.EAX  = Regs->E.EAX;
  ThunkRegSet.E.DS   = Regs->E.DS;
  ThunkRegSet.E.ES   = Regs->E.ES;

  EfiCommonLibCopyMem (&(ThunkRegSet.E.EFLAGS), &(Regs->E.EFlags), sizeof (UINT32));

  if ((Stack != NULL) && (StackSize != 0)) {
    AsmThunk16SetUserStack (&mThunkContext, Stack, StackSize);
  } 

  //
  // The call to Legacy16 is a critical section to EFI
  //
  Eflags = EfiGetEflags ();
  if ((Eflags | EFI_CPU_EFLAGS_IF) != 0) {
    EfiDisableInterrupts ();
  }

  ThunkRegSet.E.CS   = Segment;
  ThunkRegSet.E.EIP  = Offset;

  //
  // Set Legacy16 state. 0x08, 0x70 is legacy 8259 vector bases.
  //
  Status = gLegacy8259->SetMode (gLegacy8259, Efi8259LegacyMode, NULL, NULL);
  ASSERT_EFI_ERROR (Status);
  
  AsmThunk16FarCall86 (&mThunkContext, &ThunkRegSet, 0);

  //
  // Restore protected mode interrupt state
  //
  Status = gLegacy8259->SetMode (gLegacy8259, Efi8259ProtectedMode, NULL, NULL);
  ASSERT_EFI_ERROR (Status);

  //
  // End critical section
  //
  if ((Eflags | EFI_CPU_EFLAGS_IF) != 0) {
    EfiEnableInterrupts ();
  }

  Regs->E.EDI      = ThunkRegSet.E.EDI;      
  Regs->E.ESI      = ThunkRegSet.E.ESI;  
  Regs->E.EBP      = ThunkRegSet.E.EBP;  
  Regs->E.EBX      = ThunkRegSet.E.EBX;  
  Regs->E.EDX      = ThunkRegSet.E.EDX;  
  Regs->E.ECX      = ThunkRegSet.E.ECX;  
  Regs->E.EAX      = ThunkRegSet.E.EAX;
  Regs->E.SS       = ThunkRegSet.E.SS;
  Regs->E.CS       = ThunkRegSet.E.CS;  
  Regs->E.DS       = ThunkRegSet.E.DS;  
  Regs->E.ES       = ThunkRegSet.E.ES;

  EfiCommonLibCopyMem (&(Regs->E.EFlags), &(ThunkRegSet.E.EFLAGS), sizeof (UINT32));

  Ret = (BOOLEAN) (Regs->E.EFlags.CF == 1);

  return Ret;
}

