/** @file
  Provide legacy thunk interface for accessing Bios Video Rom.
  
Copyright (c) 2006 - 2007, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include <Efi.h>

#include "LegacyBiosThunk.h"
#include <posix/posix.h>

#ifndef DEBUG_ALL
#define DEBUG_LBTHUNK 0
#else
#define DEBUG_LBTHUNK DEBUG_ALL
#endif

#if DEBUG_LBTHUNK == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_LBTHUNK, __VA_ARGS__)
#endif


#define EFI_CPU_EFLAGS_IF 0x200
extern EFI_LEGACY_8259_PROTOCOL        *gLegacy8259;
extern THUNK_CONTEXT                   *mThunkContext;

/**
  Initialize legacy environment for BIOS INI caller.
  
  @param ThunkContext   the instance pointer of THUNK_CONTEXT
**/
EFI_STATUS
InitializeBiosIntCaller (
//  THUNK_CONTEXT     *ThunkContext
  )
{
  EFI_STATUS            Status;
  UINT32                RealModeBufferSize;
  UINT32                ExtraStackSize;
  EFI_PHYSICAL_ADDRESS  LegacyRegionBase;
  UINT32                LegacyRegionSize;
  //
  // Get LegacyRegion
  //
  AsmGetThunk16Properties (&RealModeBufferSize, &ExtraStackSize);
  LegacyRegionSize = (((RealModeBufferSize + ExtraStackSize) / EFI_PAGE_SIZE) + 1) * EFI_PAGE_SIZE;
  LegacyRegionBase = 0x0C0000;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiACPIMemoryNVS,
                  EFI_SIZE_TO_PAGES(LegacyRegionSize),
                  &LegacyRegionBase
                  );
//  ASSERT_EFI_ERROR(Status);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  mThunkContext->RealModeBuffer     = (void*)(UINTN)LegacyRegionBase;
  mThunkContext->RealModeBufferSize = LegacyRegionSize;
  mThunkContext->ThunkAttributes    = THUNK_ATTRIBUTE_DISABLE_A20_MASK_INT_15;
  DBG("mThunkContext->RealModeBuffer: %llx, mThunkContext->RealModeBufferSize: %d\n", (uintptr_t)(mThunkContext->RealModeBuffer), mThunkContext->RealModeBufferSize);
  AsmPrepareThunk16(mThunkContext);
  return Status;
}

/**
   Initialize interrupt redirection code and entries, because
   IDT Vectors 0x68-0x6f must be redirected to IDT Vectors 0x08-0x0f.
   Or the interrupt will lost when we do thunk.
   NOTE: We do not reset 8259 vector base, because it will cause pending
   interrupt lost.
   
   @param Legacy8259  Instance pointer for EFI_LEGACY_8259_PROTOCOL.
   
**/
CONST   UINT32   InterruptRedirectionCode[8] = {
  0x90CF08CD, // INT8; IRET; NOP
  0x90CF09CD, // INT9; IRET; NOP
  0x90CF0ACD, // INTA; IRET; NOP
  0x90CF0BCD, // INTB; IRET; NOP
  0x90CF0CCD, // INTC; IRET; NOP
  0x90CF0DCD, // INTD; IRET; NOP
  0x90CF0ECD, // INTE; IRET; NOP
  0x90CF0FCD  // INTF; IRET; NOP
};


EFI_STATUS
InitializeInterruptRedirection (
//  IN  EFI_LEGACY_8259_PROTOCOL  *Legacy8259
  )
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  LegacyRegionBase;
  UINTN                 LegacyRegionLength;
  volatile UINT32                *IdtArray;
  UINTN                 Index;
  UINT8                 ProtectedModeBaseVector;

  //
  // Get LegacyRegion
  //
  LegacyRegionLength = sizeof(InterruptRedirectionCode);
  LegacyRegionBase = 0x0C0000;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiACPIMemoryNVS,
                  EFI_SIZE_TO_PAGES(LegacyRegionLength),
                  &LegacyRegionBase
                  );
//  ASSERT_EFI_ERROR(Status);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Copy code to legacy region
  //
  CopyMem((void *)(UINTN)LegacyRegionBase, (void *)&InterruptRedirectionCode[0], sizeof (InterruptRedirectionCode));

  //
  // Get VectorBase, it should be 0x68
  //
  Status = gLegacy8259->GetVector (gLegacy8259, Efi8259Irq0, &ProtectedModeBaseVector);
//  ASSERT_EFI_ERROR(Status);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Patch IVT 0x68 ~ 0x6f
  //
  IdtArray = (UINT32 *) 0;
  for (Index = 0; Index < 8; Index++) {
    IdtArray[ProtectedModeBaseVector + Index] = ((EFI_SEGMENT (LegacyRegionBase + Index * 4)) << 16) | (EFI_OFFSET (LegacyRegionBase + Index * 4));
  }

  return Status;
}

/**
  Disconnect EFI VGA driver
  When BiosVideo disconnects, it takes care of setting Text VGA Mode (80x25) which works properly with Legacy mode
**/
EFI_STATUS
DisconnectVga ( void )
{
  EFI_STATUS              Status;
  UINTN                   HandleCount = 0;
  UINTN                   Index;
  EFI_HANDLE              *Handles = NULL;
  EFI_PCI_IO_PROTOCOL     *PciIo  = NULL;
  PCI_TYPE00              Pci;

  // get all PciIo handles
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiPciIoProtocolGuid, NULL, &HandleCount, &Handles);
  if (Status == EFI_SUCCESS) {
    for (Index = 0; Index < HandleCount; Index++) {
      Status = gBS->HandleProtocol(Handles[Index], &gEfiPciIoProtocolGuid, (void **) &PciIo);
      if (EFI_ERROR(Status)) {
        continue;
      }
      Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, sizeof (Pci) / sizeof (UINT32), &Pci);
      if (!EFI_ERROR(Status))
      {
        if(IS_PCI_VGA(&Pci) == TRUE)
        {
          // disconnect VGA
          DBG("Disonnecting VGA\n");
          Status = gBS->DisconnectController(Handles[Index], NULL, NULL);
          DBG("disconnect %s", efiStrError(Status));
        }
      }
    }
    FreePool(Handles);
  }
  return Status;
}


/**
  Thunk to 16-bit real mode and execute a software interrupt with a vector 
  of BiosInt. Regs will contain the 16-bit register context on entry and 
  exit.
  
  @param  This    Protocol instance pointer.
  @param  BiosInt Processor interrupt vector to invoke
  @param  Reg     Register contexted passed into (and returned) from thunk to 16-bit mode
  
  @retval TRUE   Thunk completed, and there were no BIOS errors in the target code.
                 See Regs for status.
  @retval FALSE  There was a BIOS erro in the target code.  
**/
BOOLEAN
EFIAPI
LegacyBiosInt86 (
//  IN  BIOS_VIDEO_DEV                 *BiosDev,
  IN  UINT8                          BiosInt,
  IN  IA32_REGISTER_SET              *Regs
  )
{
  UINTN                 Status;
  UINTN                 Eflags;
  IA32_REGISTER_SET     ThunkRegSet;
  BOOLEAN               Ret;
  UINT16                *Stack16;
  
  ZeroMem (&ThunkRegSet, sizeof (ThunkRegSet));
  ThunkRegSet.E.EFLAGS.Bits.Reserved_0 = 1;
  ThunkRegSet.E.EFLAGS.Bits.Reserved_1 = 0;
  ThunkRegSet.E.EFLAGS.Bits.Reserved_2 = 0;
  ThunkRegSet.E.EFLAGS.Bits.Reserved_3 = 0;
  ThunkRegSet.E.EFLAGS.Bits.IOPL       = 3;
  ThunkRegSet.E.EFLAGS.Bits.NT         = 0;
  ThunkRegSet.E.EFLAGS.Bits.IF         = 1;
  ThunkRegSet.E.EFLAGS.Bits.TF         = 0;
  ThunkRegSet.E.EFLAGS.Bits.CF         = 0;
  
  ThunkRegSet.E.EDI  = Regs->E.EDI;
  ThunkRegSet.E.ESI  = Regs->E.ESI;
  ThunkRegSet.E.EBP  = Regs->E.EBP;
  ThunkRegSet.E.EBX  = Regs->E.EBX;
  ThunkRegSet.E.EDX  = Regs->E.EDX;
  ThunkRegSet.E.ECX  = Regs->E.ECX;
  ThunkRegSet.E.EAX  = Regs->E.EAX;
  ThunkRegSet.E.DS   = Regs->E.DS;
  ThunkRegSet.E.ES   = Regs->E.ES;

  //
  // The call to Legacy16 is a critical section to EFI
  //
  Eflags = AsmReadEflags ();
  if ((Eflags & EFI_CPU_EFLAGS_IF) != 0) {
    DisableInterrupts ();
  }

  //
  // Set Legacy16 state. 0x08, 0x70 is legacy 8259 vector bases.
  //
  Status = gLegacy8259->SetMode (gLegacy8259, Efi8259LegacyMode, NULL, NULL);
//  ASSERT_EFI_ERROR(Status);
  if (EFI_ERROR(Status)) {
    return FALSE;
  }
  
  Stack16 = (UINT16 *)((UINT8 *) mThunkContext->RealModeBuffer + mThunkContext->RealModeBufferSize - sizeof (UINT16));

  ThunkRegSet.E.SS   = (UINT16) (((UINTN) Stack16 >> 16) << 12);
  ThunkRegSet.E.ESP  = (UINT16) (UINTN) Stack16;

  ThunkRegSet.E.Eip  = *(UINT16*)(((UINTN)BiosInt) * sizeof(UINT32));
  ThunkRegSet.E.CS   = *(UINT16*)((((UINTN)BiosInt) * sizeof(UINT32)) + sizeof(UINT16));
  mThunkContext->RealModeState = &ThunkRegSet;
  AsmThunk16 (mThunkContext);
  
  //
  // Restore protected mode interrupt state
  //
  Status = gLegacy8259->SetMode (gLegacy8259, Efi8259ProtectedMode, NULL, NULL);
//  ASSERT_EFI_ERROR(Status);
  if (EFI_ERROR(Status)) {
    return FALSE;
  }

  //
  // End critical section
  //
  if ((Eflags & EFI_CPU_EFLAGS_IF) != 0) {
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

  CopyMem(&(Regs->E.EFLAGS), &(ThunkRegSet.E.EFLAGS), sizeof (UINT32));

  Ret = (BOOLEAN) (Regs->E.EFLAGS.Bits.CF == 1);

  return Ret;
}

BOOLEAN
EFIAPI
LegacyBiosFarCall86 (
//  IN  EFI_LEGACY_BIOS_THUNK_PROTOCOL  *This,
  IN  UINT16							Segment,
  IN  UINT16							Offset,
  IN  IA32_REGISTER_SET					*Regs
//  IN  void                            *Stack
//  IN  UINTN                           StackSize
  )
{
  UINTN                 Status;
  UINT32                Eflags;
  IA32_REGISTER_SET     ThunkRegSet;
  BOOLEAN               Ret;
  UINT16                *Stack16;
//	UINT16				BiosInt = 0x100;
  EFI_TPL                 OriginalTpl;
  EFI_TIMER_ARCH_PROTOCOL *Timer;
  UINT64                  TimerPeriod = 0;
#if ENABLE_PS2MOUSE_LEGACYBOOT == 1
  UINT16                  LegacyMaskOld, LegacyMaskNew;
#endif

  // Disconnect EFI VGA driver (and switch to Text VGA Mode)
  DisconnectVga();

  ZeroMem (&ThunkRegSet, sizeof (ThunkRegSet));
  ThunkRegSet.E.EFLAGS.Bits.Reserved_0 = 1;
  ThunkRegSet.E.EFLAGS.Bits.Reserved_1 = 0;
  ThunkRegSet.E.EFLAGS.Bits.Reserved_2 = 0;
  ThunkRegSet.E.EFLAGS.Bits.Reserved_3 = 0;
  ThunkRegSet.E.EFLAGS.Bits.IOPL       = 3;
  ThunkRegSet.E.EFLAGS.Bits.NT         = 0;
  ThunkRegSet.E.EFLAGS.Bits.IF         = 0; //why???
  ThunkRegSet.E.EFLAGS.Bits.TF         = 0;
  ThunkRegSet.E.EFLAGS.Bits.CF         = 0;

  ThunkRegSet.E.EDI  = Regs->E.EDI;
  ThunkRegSet.E.ESI  = Regs->E.ESI;
  ThunkRegSet.E.EBP  = Regs->E.EBP;
  ThunkRegSet.E.EBX  = Regs->E.EBX;
  ThunkRegSet.E.EDX  = Regs->E.EDX;
  ThunkRegSet.E.ECX  = Regs->E.ECX;
  ThunkRegSet.E.EAX  = Regs->E.EAX;
  ThunkRegSet.E.DS   = Regs->E.DS;
  ThunkRegSet.E.ES   = Regs->E.ES;

/*  CopyMem(&(ThunkRegSet.E.EFLAGS), &(Regs->E.EFlags), sizeof (UINT32));

  if ((Stack != NULL) && (StackSize != 0)) {
    AsmThunk16SetUserStack (&mThunkContext, Stack, StackSize);
  } */

  //
  // The call to Legacy16 is a critical section to EFI
  //
	Eflags = (UINT32)AsmReadEflags ();
	if ((Eflags & EFI_CPU_EFLAGS_IF) != 0) {
		DisableInterrupts ();
	}
//    DisableInterrupts ();

  // Critical section - execute without interruption until Legacy boots
  OriginalTpl = gBS->RaiseTPL (TPL_HIGH_LEVEL);
	
//xxx - Slice
  //AsmWriteIdtr(NULL);
	Stack16 = (UINT16 *)((UINT8 *) mThunkContext->RealModeBuffer + mThunkContext->RealModeBufferSize - sizeof (UINT16));
	
	ThunkRegSet.E.SS   = (UINT16) (((UINTN) Stack16 >> 16) << 12);
	ThunkRegSet.E.ESP  = (UINT16) (UINTN) Stack16;
	
	ThunkRegSet.E.CS   = Segment;
	ThunkRegSet.E.Eip  = Offset;
	mThunkContext->RealModeState = &ThunkRegSet;
	DBG("SS=%hX, ESP=%X, CS=%hX, Eip=%X\n", ThunkRegSet.E.SS, ThunkRegSet.E.ESP, ThunkRegSet.E.CS, ThunkRegSet.E.Eip);
    #if DEBUG_LBTHUNK == 2
    PauseForKey("LegacyBiosFarCall86"_XS8);
    #endif

  // Save current rate of DXE Timer and disable DXE timer
  Status = gBS->LocateProtocol (&gEfiTimerArchProtocolGuid, NULL, (void **) &Timer);
  if (!EFI_ERROR(Status)) {
    Timer->GetTimerPeriod (Timer, &TimerPeriod);
    Timer->SetTimerPeriod (Timer, 0);
  }

  // Before switching to Legacy vector base, we set timer to default Legacy rate (54.9254 ms) - this is done by 3 steps:
  IoWrite8 (TIMER_CONTROL_PORT, TIMER0_CONTROL_WORD);
  IoWrite8 (TIMER0_COUNT_PORT, 0x00);
  IoWrite8 (TIMER0_COUNT_PORT, 0x00);

  // Set Irq Vector base to default Legacy base (master at 0x08, slave at 0x70)
  gLegacy8259->SetVectorBase (gLegacy8259, LEGACY_MODE_BASE_VECTOR_MASTER, LEGACY_MODE_BASE_VECTOR_SLAVE);

  // Set Irq Mask according to Legacy mode
#if ENABLE_PS2MOUSE_LEGACYBOOT == 1
  // Update the Mask to allow also PS2 Mouse usage (which requires also Secondary PIC cascade)
  gLegacy8259->GetMask (gLegacy8259, &LegacyMaskOld, NULL, NULL, NULL);
  LegacyMaskNew = LegacyMaskOld & 0xEFFB;
  gLegacy8259->SetMode (gLegacy8259, Efi8259LegacyMode, &LegacyMaskNew, NULL);
#else
  gLegacy8259->SetMode (gLegacy8259, Efi8259LegacyMode, NULL, NULL);
#endif

  // Thunk to 16-bit real mode to execute the farcall
  AsmThunk16 (mThunkContext);

  // EFI is probably overwritten here, but just in case we continue, try to restore EFI to previous state:

#if ENABLE_PS2MOUSE_LEGACYBOOT == 1
  // Restore original Legacy Mask, as it was changed by our call to SetMode with the new mask
  gLegacy8259->SetMask (gLegacy8259, &LegacyMaskOld, NULL, NULL, NULL);
#endif

  // Set Irq Mask according to Protected mode
  gLegacy8259->SetMode (gLegacy8259, Efi8259ProtectedMode, NULL, NULL);

  // Set Irq Vector base to Protected vector base (master at 0x68, slave at 0x70)
  gLegacy8259->SetVectorBase (gLegacy8259, PROTECTED_MODE_BASE_VECTOR_MASTER, PROTECTED_MODE_BASE_VECTOR_SLAVE);

  // Enable and restore rate of DXE Timer
  Timer->SetTimerPeriod (Timer, TimerPeriod);

  // End critical section
  gBS->RestoreTPL (OriginalTpl);

  //
  // End critical section
  //
  if ((Eflags & EFI_CPU_EFLAGS_IF) != 0) {
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

	CopyMem(&(Regs->E.EFLAGS), &(ThunkRegSet.E.EFLAGS), sizeof (UINT32));
	
	Ret = (BOOLEAN) (Regs->E.EFLAGS.Bits.CF == 1);

  // Connect VGA EFI Driver
  BdsLibConnectAllDriversToAllControllers();
	
  return Ret;
}


