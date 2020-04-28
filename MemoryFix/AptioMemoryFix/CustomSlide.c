/**

  Allows to choose a random KASLR slide offset,
  when some offsets cannot be used.

  by Download-Fritz & vit9696

**/

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DeviceTreeLib.h>
#include <Library/OcMiscLib.h>
#include <Library/PrintLib.h>
#include <Library/RngLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <IndustryStandard/AppleCsrConfig.h>
#include <Register/Microcode.h>

#include "Config.h"
#include "CustomSlide.h"
#include "BootArgs.h"
#include "BootFixes.h"
#include "MemoryMap.h"
#include "RtShims.h"
#include "ServiceOverrides.h"

//
// Modified boot-args buffer with an additional slide parameter, when custom slide is used.
//
STATIC BOOLEAN mStoredBootArgsVarSet;
STATIC UINTN   mStoredBootArgsVarSize;
STATIC CHAR8   mStoredBootArgsVar[BOOT_LINE_LENGTH];

//
// Memory map slide availability analysis status.
//
STATIC BOOLEAN mAnalyzeMemoryMapDone;

//
// Original csr-active-config value to be restored before kernel handoff.
//
STATIC BOOLEAN mCsrActiveConfigSet;
STATIC UINT32  mCsrActiveConfig;

//
// List of KASLR slides that do not conflict with the previously allocated memory.
//
STATIC UINT8   mValidSlides[TOTAL_SLIDE_NUM];
STATIC UINT32  mValidSlidesNum = TOTAL_SLIDE_NUM;

//
// Detect Sandy or Ivy Bridge CPUs, since they use a different slide formula.
//
STATIC BOOLEAN mSandyOrIvy;
STATIC BOOLEAN mSandyOrIvySet;

STATIC
BOOLEAN
IsSandyOrIvy (
  VOID
  )
{
  CPU_MICROCODE_PROCESSOR_SIGNATURE  Sig;
  UINT32                             CpuFamily;
  UINT32                             CpuModel;

  if (!mSandyOrIvySet) {
    Sig.Uint32 = 0;

    AsmCpuid (1, &Sig.Uint32, NULL, NULL, NULL);

    CpuFamily = Sig.Bits.Family;
    if (CpuFamily == 15) {
      CpuFamily += Sig.Bits.ExtendedFamily;
    }

    CpuModel = Sig.Bits.Model;
    if (CpuFamily == 15 || CpuFamily == 6) {
      CpuModel |= Sig.Bits.ExtendedModel << 4;
    }

    mSandyOrIvy = CpuFamily == 6 && (CpuModel == 0x2A || CpuModel == 0x3A);
    mSandyOrIvySet = TRUE;

    DEBUG ((DEBUG_VERBOSE, "Discovered CpuFamily %d CpuModel %d SandyOrIvy %d\n", CpuFamily, CpuModel, mSandyOrIvy));
  }

  return mSandyOrIvy;
}

STATIC
VOID
GetSlideRangeForValue (
  UINT8   Slide,
  UINTN   *StartAddr,
  UINTN   *EndAddr
  )
{
  *StartAddr = (UINTN)Slide * SLIDE_GRANULARITY + BASE_KERNEL_ADDR;

  //
  // Skip ranges used by Intel HD 2000/3000.
  //
  if (Slide >= 0x80 && IsSandyOrIvy ()) {
    *StartAddr += 0x10200000;
  }

  *EndAddr = *StartAddr + APTIOFIX_SPECULATED_KERNEL_SIZE;
}

STATIC
UINT8
GenerateRandomSlideValue (
  VOID
  )
{
  UINT32  Clock = 0;
  UINT32  Ecx = 0;
  UINT8   Slide = 0;
  UINT16  Value = 0;
  BOOLEAN RdRandSupport;

  AsmCpuid (0x1, NULL, NULL, &Ecx, NULL);
  RdRandSupport = (Ecx & 0x40000000) != 0;

  do {
    if (RdRandSupport && GetRandomNumber16 (&Value) == EFI_SUCCESS && (UINT8) Value != 0) {
      Slide = (UINT8) Value;
    } else {
      Clock = (UINT32) AsmReadTsc ();
      Slide = (Clock & 0xFF) ^ ((Clock >> 8) & 0xFF);
    }
  } while (Slide == 0);

  DEBUG ((DEBUG_VERBOSE, "Generated slide index %d value %d\n", Slide, mValidSlides[Slide % mValidSlidesNum]));

  //
  //FIXME: This is bad due to uneven distribution, but let's use it for now.
  //
  return mValidSlides[Slide % mValidSlidesNum];
}

#if APTIOFIX_CLEANUP_SLIDE_BOOT_ARGUMENT == 1
STATIC
VOID
HideSlideFromOS (
  AMF_BOOT_ARGUMENTS  *BootArgs
  )
{
  EFI_STATUS  Status;
  DTEntry     Chosen;
  CHAR8       *ArgsStr;
  UINT32      ArgsSize;

  //
  // First, there is a BootArgs entry for XNU
  //
  RemoveArgumentFromCommandLine (BootArgs->CommandLine, "slide=");

  //
  // Second, there is a DT entry
  //
  DTInit ((VOID *)(UINTN)(*BootArgs->deviceTreeP), BootArgs->deviceTreeLength);
  Status = DTLookupEntry (NULL, "/chosen", &Chosen);
  if (!EFI_ERROR(Status)) {
    DEBUG ((DEBUG_VERBOSE, "Found /chosen\n"));
    Status = DTGetProperty(Chosen, "boot-args", (VOID **)&ArgsStr, &ArgsSize);
    if (!EFI_ERROR(Status) && ArgsSize > 0) {
      DEBUG ((DEBUG_VERBOSE, "Found boot-args in /chosen\n"));
      RemoveArgumentFromCommandLine (ArgsStr, "slide=");
    }
  }

  //
  // Third, clean the boot args just in case
  //
  mValidSlidesNum = 0;
  mStoredBootArgsVarSize = 0;
  ZeroMem (mValidSlides, sizeof(mValidSlides));
  ZeroMem (mStoredBootArgsVar, sizeof(mStoredBootArgsVar));
}
#endif

STATIC
VOID
DecideOnCustomSlideImplementation (
  VOID
  )
{
  UINTN                  AllocatedMapPages;
  UINTN                  MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR  *MemoryMap;
  UINTN                  MapKey;
  EFI_STATUS             Status;
  UINTN                  DescriptorSize;
  UINT32                 DescriptorVersion;
  UINTN                  Index;
  UINTN                  Slide;
  UINTN                  NumEntries;
  UINTN                  MaxAvailableSize = 0;
  UINT8                  FallbackSlide = 0;

  Status = GetMemoryMapAlloc (
    &AllocatedMapPages,
    &MemoryMapSize,
    &MemoryMap,
    &MapKey,
    &DescriptorSize,
    &DescriptorVersion
    );

  if (Status != EFI_SUCCESS) {
    Print (L"AMF: Failed to obtain memory map for KASLR - %r\n", Status);
    return;
  }

  //
  // At this point we have a memory map that we could use to determine what slide values are allowed.
  //
  NumEntries = MemoryMapSize / DescriptorSize;

  //
  // Reset valid slides to zero and find actually working ones.
  //
  mValidSlidesNum = 0;

  for (Slide = 0; Slide < TOTAL_SLIDE_NUM; Slide++) {
    EFI_MEMORY_DESCRIPTOR  *Desc = MemoryMap;
    BOOLEAN                Supported = TRUE;
    UINTN                  StartAddr;
    UINTN                  EndAddr;
    UINTN                  DescEndAddr;
    UINTN                  AvailableSize;

    GetSlideRangeForValue ((UINT8)Slide, &StartAddr, &EndAddr);

    AvailableSize = 0;

    for (Index = 0; Index < NumEntries; Index++) {
      DescEndAddr = (Desc->PhysicalStart + EFI_PAGES_TO_SIZE (Desc->NumberOfPages));

      if ((Desc->PhysicalStart < EndAddr) && (DescEndAddr > StartAddr)) {
        //
        // The memory overlaps with the slide region.
        //
        if (Desc->Type != EfiConventionalMemory) {
          //
          // The memory is unusable atm.
          //
          Supported = FALSE;
          break;
        } else {
          //
          // The memory will be available for the kernel.
          //
          AvailableSize += EFI_PAGES_TO_SIZE (Desc->NumberOfPages);

          if (Desc->PhysicalStart < StartAddr) {
            //
            // The region starts before the slide region.
            // Subtract the memory that is located before the slide region.
            //
            AvailableSize -= (StartAddr - Desc->PhysicalStart);
          }

          if (DescEndAddr > EndAddr) {
            //
            // The region ends after the slide region.
            // Subtract the memory that is located after the slide region.
            //
            AvailableSize -= (DescEndAddr - EndAddr);
          }
        }
      }

      Desc = NEXT_MEMORY_DESCRIPTOR (Desc, DescriptorSize);
    }

    if (AvailableSize > MaxAvailableSize) {
      MaxAvailableSize = AvailableSize;
      FallbackSlide = (UINT8)Slide;
    }

    if ((StartAddr + AvailableSize) != EndAddr) {
      //
      // The slide region is not continuous.
      //
      Supported = FALSE;
    }

    if (Supported) {
      DEBUG ((DEBUG_VERBOSE, "Slide %03d at %08x:%08x should be ok.\n", (UINT32)Slide, (UINT32)StartAddr, (UINT32)EndAddr));
      mValidSlides[mValidSlidesNum++] = (UINT8)Slide;
    } else {
      DEBUG ((DEBUG_VERBOSE, "Slide %03d at %08x:%08x cannot be used!\n", (UINT32)Slide, (UINT32)StartAddr, (UINT32)EndAddr));
    }
  }

  gBS->FreePages ((EFI_PHYSICAL_ADDRESS)MemoryMap, AllocatedMapPages);

  if (mValidSlidesNum != TOTAL_SLIDE_NUM) {
    if (mValidSlidesNum == 0) {
      Print (L"AMF: No slide values are usable! Falling back to %d with 0x%08X bytes!\n", FallbackSlide, MaxAvailableSize);
      mValidSlides[mValidSlidesNum++] = (UINT8)FallbackSlide;
    } else {
      //
      // Pretty-print valid slides as ranges.
      // For example, 1, 2, 3, 4, 5 will become 1-5.
      //
      Print (L"AMF: Only %d/%d slide values are usable!\n", mValidSlidesNum, TOTAL_SLIDE_NUM);
      NumEntries = 0;
      for (Index = 0; Index <= mValidSlidesNum; Index++) {
        if (Index == 0) {
          Print (L"Valid slides: %d", mValidSlides[Index]);
        } else if (Index == mValidSlidesNum || mValidSlides[Index - 1] + 1 != mValidSlides[Index]) {
          if (NumEntries == 1) {
            Print (L", %d", mValidSlides[Index - 1]);
          } else if (NumEntries > 1) {
            Print (L"-%d", mValidSlides[Index - 1]);
          }
          if (Index == mValidSlidesNum) {
            Print (L"\n");
          } else {
            Print (L", %d", mValidSlides[Index]);
          }
          NumEntries = 0;
        } else {
          NumEntries++;
        }
      }
    }
  }
}

STATIC
EFI_STATUS
GetVariableCsrActiveConfig (
  IN     CHAR16    *VariableName,
  IN     EFI_GUID  *VendorGuid,
  OUT    UINT32    *Attributes OPTIONAL,
  IN OUT UINTN     *DataSize,
  OUT    VOID      *Data
  )
{
  EFI_STATUS  Status;
  UINT32      *Config;

  //
  // If we were asked for the size, just return it right away.
  //
  if (!Data || *DataSize < sizeof(UINT32)) {
    *DataSize = sizeof(UINT32);
    return EFI_BUFFER_TOO_SMALL;
  }

  Config = (UINT32 *)Data;

  //
  // Otherwise call the original function.
  //
  Status = OrgGetVariable (VariableName, VendorGuid, Attributes, DataSize, Data);
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_INFO, "GetVariable csr-active-config returned %r\n", Status));

    *Config = 0;
    Status = EFI_SUCCESS;
    if (Attributes) {
      *Attributes =
        EFI_VARIABLE_BOOTSERVICE_ACCESS |
        EFI_VARIABLE_RUNTIME_ACCESS |
        EFI_VARIABLE_NON_VOLATILE;
    }
  }

  //
  // We must unrestrict NVRAM from SIP or slide=X will not be supported.
  //
  mCsrActiveConfig     = *Config;
  mCsrActiveConfigSet  = TRUE;
  *Config |= CSR_ALLOW_UNRESTRICTED_NVRAM;

  return Status;
}

STATIC
EFI_STATUS
GetVariableBootArgs (
  IN     CHAR16    *VariableName,
  IN     EFI_GUID  *VendorGuid,
  OUT    UINT32    *Attributes OPTIONAL,
  IN OUT UINTN     *DataSize,
  OUT    VOID      *Data
  )
{
  EFI_STATUS  Status;
  UINTN       StoredBootArgsSize = BOOT_LINE_LENGTH;
  UINT8       Slide;
  CHAR8       SlideArgument[10];
  CONST UINTN SlideArgumentLength = ARRAY_SIZE (SlideArgument)-1;

  if (!mStoredBootArgsVarSet) {
    Slide  = GenerateRandomSlideValue ();
    Status = OrgGetVariable (VariableName, VendorGuid, Attributes, &StoredBootArgsSize, mStoredBootArgsVar);
    if (EFI_ERROR(Status)) {
      mStoredBootArgsVar[0] = '\0';
    }

    //
    // Note, the point is to always pass 3 characters to avoid side attacks on value length.
    //
    AsciiSPrint (SlideArgument, ARRAY_SIZE (SlideArgument), "slide=%-03d", Slide);

    if (!AppendArgumentToCommandLine (mStoredBootArgsVar, SlideArgument, SlideArgumentLength)) {
      //
      // Broken boot-args, try to overwrite.
      //
      AsciiStrnCpyS (mStoredBootArgsVar, SlideArgumentLength + 1, SlideArgument, SlideArgumentLength + 1);;
    }

    mStoredBootArgsVarSize = AsciiStrLen(mStoredBootArgsVar) + 1;
    mStoredBootArgsVarSet = TRUE;
  }

  if (Attributes) {
    *Attributes =
      EFI_VARIABLE_BOOTSERVICE_ACCESS |
      EFI_VARIABLE_RUNTIME_ACCESS |
      EFI_VARIABLE_NON_VOLATILE;
  }

  if (*DataSize >= mStoredBootArgsVarSize && Data) {
    AsciiStrnCpyS (Data, *DataSize, mStoredBootArgsVar, mStoredBootArgsVarSize);
    Status = EFI_SUCCESS;
  } else {
    Status = EFI_BUFFER_TOO_SMALL;
  }

  *DataSize = mStoredBootArgsVarSize;

  return Status;
}

VOID
UnlockSlideSupportForSafeMode (
  UINT8  *ImageBase,
  UINTN  ImageSize
  )
{
  //
  // boot.efi performs the following check:
  // if (State & (BOOT_MODE_SAFE | BOOT_MODE_ASLR)) == (BOOT_MODE_SAFE | BOOT_MODE_ASLR)) {
  //   * Disable KASLR *
  // }
  // We do not care about the asm it will use for it, but we could assume that the constants
  // will be used twice and their location will be very close to each other.
  //
  // BOOT_MODE_SAFE | BOOT_MODE_ASLR constant is 0x4001 in hex.
  // It has not changed since its appearance, so is most likely safe to look for.
  // Furthermore, since boot.efi state mask uses higher bits, it is safe to assume that
  // the comparison will be at least 32-bit.
  //
  //
  // The new way patch is a workaround for 10.13.5 and newer, where the code got finally changed.
  // if (State & BOOT_MODE_SAFE) {
  //   ReportFeature(FEATURE_BOOT_MODE_SAFE);
  //   if (State & BOOT_MODE_ASLR) {
  //     * Disable KASLR *
  //   }
  // }
  //
  CONST UINT8 SearchSeqNew[] = {0xF6, 0xC4, 0x40, 0x75};
  CONST UINT8 SearchSeq[] = {0x01, 0x40, 0x00, 0x00};

  //
  // This is a reasonable maximum distance to expect between the instructions.
  //
  CONST UINTN MaxDist = 0x10;

  UINT8      *StartOff = ImageBase;
  UINT8      *EndOff   = StartOff + ImageSize - sizeof (SearchSeq) - MaxDist;

  UINTN       FirstOff = 0;
  UINTN       SecondOff = 0;

  BOOLEAN     NewWay = FALSE;

  do {
    while (StartOff + FirstOff <= EndOff) {
      if (!CompareMem (StartOff + FirstOff, SearchSeqNew, sizeof (SearchSeqNew))) {
        NewWay = TRUE;
        break;
      } else if (!CompareMem (StartOff + FirstOff, SearchSeq, sizeof (SearchSeq))) {
        break;
      }
      FirstOff++;
    }

    DEBUG ((DEBUG_VERBOSE, "Found first %d at off %X\n", (UINT32)NewWay, (UINT32)FirstOff));

    if (StartOff + FirstOff > EndOff) {
      DEBUG ((DEBUG_INFO, "Failed to find first BOOT_MODE_SAFE | BOOT_MODE_ASLR sequence\n"));
      break;
    }

    if (NewWay) {
      //
      // Here we just patch the comparison code and the check by straight nopping.
      //
      DEBUG ((DEBUG_VERBOSE, "Patching new safe mode aslr check...\n"));
      SetMem (StartOff + FirstOff, sizeof (SearchSeqNew) + 1, 0x90);
      return;
    }

    SecondOff = FirstOff + sizeof (SearchSeq);

    while (
      StartOff + SecondOff <= EndOff && FirstOff + MaxDist >= SecondOff &&
      CompareMem (StartOff + SecondOff, SearchSeq, sizeof (SearchSeq))) {
      SecondOff++;
    }

    DEBUG ((DEBUG_VERBOSE, "Found second at off %X\n", (UINT32)SecondOff));

    if (FirstOff + MaxDist < SecondOff) {
      DEBUG ((DEBUG_VERBOSE, "Trying next match...\n"));
      SecondOff = 0;
      FirstOff += sizeof (SearchSeq);
    }
  } while (SecondOff == 0);

  if (SecondOff != 0) {
    //
    // Here we use 0xFFFFFFFF constant as a replacement value.
    // Since the state values are contradictive (e.g. safe & single at the same time)
    // We are allowed to use this instead of to simulate if (false).
    //
    DEBUG ((DEBUG_VERBOSE, "Patching safe mode aslr check...\n"));
    SetMem (StartOff + FirstOff, sizeof (SearchSeq), 0xFF);
    SetMem (StartOff + SecondOff, sizeof (SearchSeq), 0xFF);
  }
}

BOOLEAN
OverlapsWithSlide (
  EFI_PHYSICAL_ADDRESS  Address,
  UINTN                 Size
  )
{
  BOOLEAN               SandyOrIvy;
  EFI_PHYSICAL_ADDRESS  Start;
  EFI_PHYSICAL_ADDRESS  End;
  UINTN                 Slide = 0xFF;

  SandyOrIvy = IsSandyOrIvy ();

  if (SandyOrIvy) {
    Slide = 0x7F;
  }

  Start = BASE_KERNEL_ADDR;
  End   = Start + Slide * SLIDE_GRANULARITY + APTIOFIX_SPECULATED_KERNEL_SIZE;

  if (End >= Address && Start <= Address + Size) {
    return TRUE;
  } else if (SandyOrIvy) {
    Start = 0x80 * SLIDE_GRANULARITY + BASE_KERNEL_ADDR + 0x10200000;
    End   = Start + Slide * SLIDE_GRANULARITY + APTIOFIX_SPECULATED_KERNEL_SIZE;
    if (End >= Address && Start <= Address + Size) {
      return TRUE;
    }
  }

  return FALSE;
}

EFI_STATUS
EFIAPI
GetVariableCustomSlide (
  IN     CHAR16    *VariableName,
  IN     EFI_GUID  *VendorGuid,
  OUT    UINT32    *Attributes OPTIONAL,
  IN OUT UINTN     *DataSize,
  OUT    VOID      *Data
  )
{
  if (gMacOSBootNestedCount > 0 && VariableName && VendorGuid && DataSize &&
    CompareGuid (VendorGuid, &gEfiAppleBootGuid)) {
    //
    // We override csr-active-config with CSR_ALLOW_UNRESTRICTED_NVRAM bit set
    // to allow one to pass a custom slide value even when SIP is on.
    // This original value of csr-active-config is returned to OS at XNU boot.
    // This allows SIP to be fully enabled in the operating system.
    //
    if (!StrCmp (VariableName, L"csr-active-config")) {
      return GetVariableCsrActiveConfig (
        VariableName,
        VendorGuid,
        Attributes,
        DataSize,
        Data
        );
    }
#if APTIOFIX_ALLOW_CUSTOM_ASLR_IMPLEMENTATION == 1
    //
    // When we cannot allow some KASLR values due to used address we generate
    // a random slide value among the valid options, which we we pass via boot-args.
    // See DecideOnCustomSlideImplementation for more details.
    //
    else if (!StrCmp (VariableName, L"boot-args")) {
      //
      // We delay memory map analysis as much as we can, in case boot.efi or anything else allocates
      // stuff with gBS->AllocatePool and it overlaps with the kernel area.
      // Overriding AllocatePool with a custom allocator does not really improve the situation,
      // because on older boards allocated memory above BASE_4GB causes instant reboots, and
      // on the only (so far) problematic X99 and X299 we have no free region for our pool anyway.
      // In any case, the current APTIOFIX_SPECULATED_KERNEL_SIZE value appears to work reliably.
      //
      if (!gSlideArgPresent && !mAnalyzeMemoryMapDone) {
        DecideOnCustomSlideImplementation ();
        mAnalyzeMemoryMapDone = TRUE;
      }
      //
      // Only return custom boot-args if mValidSlidesNum were determined to be less than TOTAL_SLIDE_NUM
      // And thus we have to use a custom slide implementation to boot reliably.
      //
      if (mValidSlidesNum != TOTAL_SLIDE_NUM && mValidSlidesNum > 0) {
        return GetVariableBootArgs (
          VariableName,
          VendorGuid,
          Attributes,
          DataSize,
          Data
          );
      }
    }
#endif
  }

  return OrgGetVariable (VariableName, VendorGuid, Attributes, DataSize, Data);
}

VOID
RestoreCustomSlideOverrides (
  AMF_BOOT_ARGUMENTS *BA
  )
{
  //
  // Restore csr-active-config to a value it was before our slide=X alteration.
  //
  if (BA->csrActiveConfig && mCsrActiveConfigSet) {
    *BA->csrActiveConfig = mCsrActiveConfig;
  }

#if APTIOFIX_CLEANUP_SLIDE_BOOT_ARGUMENT == 1
  //
  // Having slide=X values visible in the operating system defeats the purpose of KASLR.
  // Since our custom implementation works by passing random KASLR slide via boot-args,
  // this is especially important.
  //
  HideSlideFromOS(BA);
#endif
}
