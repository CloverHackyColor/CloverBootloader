/**

  Methods for setting callback jump from kernel entry point, callback, fixes to kernel boot image.

  by dmazar

**/

#include <IndustryStandard/AppleHibernate.h>

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MachoLib.h>
#include <Library/OcMiscLib.h>
#include <Library/OcStringLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DevicePathLib.h>

#include "Config.h"
#include "BootFixes.h"
#include "AsmFuncs.h"
#include "BootArgs.h"
#include "CustomSlide.h"
#include "MemoryMap.h"
#include "RtShims.h"
#include "VMem.h"

// DBG_TO: 0=no debug, 1=serial, 2=console
// serial requires
// [PcdsFixedAtBuild]
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x07
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0xFFFFFFFF
// in package DSC file
#define DBG_TO 0

#if DBG_TO == 2
#define DBG(...) AsciiPrint(__VA_ARGS__);
#elif DBG_TO == 1
#define DBG(...) DebugPrint(1, __VA_ARGS__);
#else
#define DBG(...)
#endif


EFI_PHYSICAL_ADDRESS         gSysTableRtArea;
EFI_PHYSICAL_ADDRESS         gRelocatedSysTableRtArea;

BOOLEAN                      gHibernateWake;
BOOLEAN                      gDumpMemArgPresent;
BOOLEAN                      gSlideArgPresent;
BOOLEAN                      gHasBrokenS4MemoryMap;
BOOLEAN                      gHasBrokenS4Allocator;

//
// Buffer and size for original kernel entry code
//
STATIC UINT8                 mOrigKernelCode[32];
STATIC UINTN                 mOrigKernelCodeSize;

//
// Buffer for virtual address map - only for RT areas
// Note: DescriptorSize is usually > sizeof(EFI_MEMORY_DESCRIPTOR),
// so this buffer can hold less than 64 descriptors
//
STATIC EFI_MEMORY_DESCRIPTOR mVirtualMemoryMap[64];
STATIC UINTN                 mVirtualMapSize;
STATIC UINTN                 mVirtualMapDescriptorSize;

/** Fixes stuff when booting without relocation block. Called when boot.efi jumps to kernel. */
STATIC
VOID
UpdateEnvironmentForBooting (
  UINTN  BootArgs
  )
{
  AMF_BOOT_ARGUMENTS      *BA;
  UINTN                   MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR   *MemoryMap;
  UINTN                   DescriptorSize;

  BA = GetBootArgs ((VOID *)BootArgs);

  //
  // Restore the variables we tempered with to support custom slides.
  //
  RestoreCustomSlideOverrides (BA);

  MemoryMapSize  = *BA->MemoryMapSize;
  MemoryMap      = (EFI_MEMORY_DESCRIPTOR *)(UINTN)(*BA->MemoryMap);
  DescriptorSize = *BA->MemoryMapDescriptorSize;

  //
  // We must restore EfiRuntimeServicesCode memory areas, because otherwise
  // RuntimeServices won't be executable.
  //
  RestoreProtectedRtMemoryTypes (MemoryMapSize, DescriptorSize, MemoryMap);
}

/** Fixes stuff when waking from hibernate without relocation block. Called when boot.efi jumps to kernel. */
STATIC
VOID
UpdateEnvironmentForHibernateWake (
  UINTN  ImageHeaderPage
  )
{
  IOHibernateImageHeader  *ImageHeader;
  IOHibernateHandoff      *Handoff;

  ImageHeader = (IOHibernateImageHeader *)(ImageHeaderPage << EFI_PAGE_SHIFT);

  //
  // Pass our relocated copy of system table
  //
  ImageHeader->systemTableOffset = (UINT32)(UINTN)(gRelocatedSysTableRtArea - ImageHeader->runtimePages);

  //
  // When reusing the original memory mapping we do not have to restore memory protection types & attributes,
  // since the new memory map is discarded anyway.
  // Otherwise we must restore memory map types just like at a normal boot, because MMIO regions are not
  // mapped as executable by XNU.
  //
  // Due to a non-contiguous RT_Code/RT_Data areas (thanks to NVRAM hack) the original areas
  // will not be unmapped and this will result in a memory leak if some new runtime pages are added.
  // But even that should not cause crashes.
  //
  Handoff = (IOHibernateHandoff *)((UINTN)ImageHeader->handoffPages << EFI_PAGE_SHIFT);
  while (Handoff->type != kIOHibernateHandoffTypeEnd) {
    if (Handoff->type == kIOHibernateHandoffTypeMemoryMap) {
      if (gHasBrokenS4MemoryMap) {
        //
        // Some firmwares provide us a (supposedly) invalid memory map, which results in
        // broken NVRAM and crashes after waking from hibernation. These firmwares for
        // whatever reason have code to ensure the same memory map over the reboots, and
        // it is a way some Windows versions hibernate. While terrible, we just discard
        // the new memory map here, and let XNU use what it has.
        //
        Handoff->type = kIOHibernateHandoffType;
      } else {
        //
        // boot.efi removes any memory from the memory map but the one with runtime attribute.
        //
        RestoreProtectedRtMemoryTypes (Handoff->bytecount, mVirtualMapDescriptorSize, (EFI_MEMORY_DESCRIPTOR *)Handoff->data);
      }
      break;
    }
    Handoff = (IOHibernateHandoff *)(UINTN)((UINTN)Handoff + sizeof(Handoff) + Handoff->bytecount);
  }
}

VOID
ApplyFirmwareQuirks (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  //
  // Detect broken firmwares.
  //
  if (SystemTable->FirmwareVendor) {
    if (!StrCmp (SystemTable->FirmwareVendor, L"American Megatrends")) {
      //
      // All APTIO firmwares provide invalid memory map after waking from
      // hibernation. This results in not working NVRAM or crashes.
      // FIXME: Find the root cause of the problem.
      //
      gHasBrokenS4MemoryMap = TRUE;
    } else if (!StrCmp (SystemTable->FirmwareVendor, L"INSYDE Corp.")) {
      //
      // At least some INSYDE firmwares have NVRAM issues just like APTIO.
      // Some are heard not to, but we are not aware of it.
      // FIXME: In addition to that we have difficulties allocating RtShims
      // on INSYDE, some address lead to reboots after hibernate wake.
      //
      gHasBrokenS4MemoryMap = TRUE;
      gHasBrokenS4Allocator = TRUE;
    }
  }
}

VOID
ReadBooterArguments (
  CHAR16  *Options,
  UINTN   OptionsSize
  )
{
  CHAR8       BootArgsVar[BOOT_LINE_LENGTH];
  UINTN       BootArgsVarLen = BOOT_LINE_LENGTH;
  EFI_STATUS  Status;
  UINTN       LastIndex;
  CHAR16      Last;

  if (Options && OptionsSize > 0) {
    //
    // Just in case we do not have 0-termination.
    // This may cut some data with unexpected options, but it is not like we care.
    //
    LastIndex = OptionsSize - 1;
    Last = Options[LastIndex];
    Options[LastIndex] = '\0';

    UnicodeStrToAsciiStrS (Options, BootArgsVar, BOOT_LINE_LENGTH);

    if (GetArgumentFromCommandLine (BootArgsVar, "slide=", L_STR_LEN ("slide="))) {
      gSlideArgPresent = TRUE;
    }

#if APTIOFIX_ALLOW_MEMORY_DUMP_ARG == 1
    if (GetArgumentFromCommandLine (BootArgsVar, "-aptiodump", L_STR_LEN ("-aptiodump"))) {
      gDumpMemArgPresent = TRUE;
    }
#endif

    //
    // Options do not belong to us, restore the changed value
    //
    Options[LastIndex] = Last;
  }

  //
  // Important to avoid triggering boot-args wrapper too early
  //
  Status = OrgGetVariable (
    L"boot-args",
    &gEfiAppleBootGuid,
    NULL, &BootArgsVarLen,
    &BootArgsVar[0]
    );

  if (!EFI_ERROR(Status) && BootArgsVarLen > 0) {
    //
    // Just in case we do not have 0-termination
    //
    BootArgsVar[BootArgsVarLen-1] = '\0';

    if (GetArgumentFromCommandLine (BootArgsVar, "slide=", L_STR_LEN ("slide="))) {
      gSlideArgPresent = TRUE;
    }

#if APTIOFIX_ALLOW_MEMORY_DUMP_ARG == 1
    if (GetArgumentFromCommandLine (BootArgsVar, "-aptiodump", L_STR_LEN ("-aptiodump"))) {
      gDumpMemArgPresent = TRUE;
    }
#endif
  }
}

/** Saves current 64 bit state and copies JumpToKernel32 function to higher mem
  * (for copying kernel back to proper place and jumping back to it).
  */
EFI_STATUS
PrepareJumpFromKernel (
  VOID
  )
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  HigherMem;
  UINTN                 Size;

  //
  // Check if already prepared.
  //
  if (JumpToKernel32Addr != 0) {
    DEBUG ((DEBUG_VERBOSE, "PrepareJumpFromKernel() - already prepared\n"));
    return EFI_SUCCESS;
  }

  //
  // Save current 64bit state - will be restored later in callback from kernel jump.
  //
  AsmPrepareJumpFromKernel ();

  //
  // Allocate higher memory for JumpToKernel code.
  // Must be 32-bit to access via a relative jump.
  //
  HigherMem = BASE_4GB;
  Status = AllocatePagesFromTop (EfiBootServicesCode, 1, &HigherMem, FALSE);
  if (Status != EFI_SUCCESS) {
    Print (L"AMF: Failed to allocate JumpToKernel memory - %r\n", Status);
    return Status;
  }

  //
  // And relocate it to higher mem.
  //
  JumpToKernel32Addr = HigherMem + ( (UINT8 *)&JumpToKernel32 - (UINT8 *)&JumpToKernel );
  JumpToKernel64Addr = HigherMem + ( (UINT8 *)&JumpToKernel64 - (UINT8 *)&JumpToKernel );

  Size = (UINT8 *)&JumpToKernelEnd - (UINT8 *)&JumpToKernel;
  if (Size > EFI_PAGES_TO_SIZE (1)) {
    Print (L"AMF: JumpToKernel32 size is too big - %ld\n", Size);
    return EFI_BUFFER_TOO_SMALL;
  }

  CopyMem((VOID *)(UINTN)HigherMem, (VOID *)&JumpToKernel, Size);

  DEBUG ((DEBUG_VERBOSE, "PrepareJumpFromKernel(): JumpToKernel relocated from %p, to %x, size = %x\n",
    &JumpToKernel, HigherMem, Size));
  DEBUG ((DEBUG_VERBOSE, "JumpToKernel32 relocated from %p, to %x\n", &JumpToKernel32, JumpToKernel32Addr));
  DEBUG ((DEBUG_VERBOSE, "JumpToKernel64 relocated from %p, to %x\n", &JumpToKernel64, JumpToKernel64Addr));
  DEBUG ((DEBUG_VERBOSE, "SavedCR3 = %x, SavedGDTR = %x, SavedIDTR = %x\n", SavedCR3, SavedGDTR, SavedIDTR));
  DEBUG ((DEBUG_VERBOSE, "PrepareJumpFromKernel(): JumpToKernel relocated from %p, to %x, size = %x\n",
    &JumpToKernel, HigherMem, Size));

  //
  // Allocate 1 RT data page for copy of EFI system table for kernel.
  // This one also has to be 32-bit due to XNU BootArgs structure.
  //
  gSysTableRtArea = BASE_4GB;
  Status = AllocatePagesFromTop (EfiRuntimeServicesData, 1, &gSysTableRtArea, FALSE);
  if (Status != EFI_SUCCESS) {
    Print (L"AMF: Failed to allocate system table memory - %r\n", Status);
    return Status;
  }

  DEBUG ((DEBUG_VERBOSE, "gSysTableRtArea = %lx\n", gSysTableRtArea));

  //
  // Copy sys table to the new location.
  //
  CopyMem((VOID *)(UINTN)gSysTableRtArea, gST, gST->Hdr.HeaderSize);

  return Status;
}

/** Patches kernel entry point with jump to AsmJumpFromKernel (AsmFuncsX64). This will then call KernelEntryPatchJumpBack. */
EFI_STATUS
KernelEntryPatchJump (
  UINT32  KernelEntry
  )
{
  //
  // Size of EntryPatchCode code
  //
  mOrigKernelCodeSize = (UINT8*)&EntryPatchCodeEnd - (UINT8*)&EntryPatchCode;
  if (mOrigKernelCodeSize > sizeof (mOrigKernelCode)) {
    return EFI_NOT_FOUND;
  }

  //
  // Save original kernel entry code
  //
  CopyMem((VOID *)mOrigKernelCode, (VOID *)(UINTN)KernelEntry, mOrigKernelCodeSize);

  //
  // Copy EntryPatchCode code to kernel entry address
  //
  CopyMem((VOID *)(UINTN)KernelEntry, (VOID *)&EntryPatchCode, mOrigKernelCodeSize);

  //
  // Pass KernelEntry to assembler funcs.
  // This is not needed really, since asm code will determine kernel entry address from the stack.
  //
  AsmKernelEntry = KernelEntry;

  return EFI_SUCCESS;
}

/** Reads kernel entry from Mach-O load command and patches it with jump to AsmJumpFromKernel. */
EFI_STATUS
KernelEntryFromMachOPatchJump (
  VOID   *MachOImage,
  UINTN  SlideAddr
  )
{
  UINTN  KernelEntry;

  KernelEntry = MachoRuntimeGetEntryAddress (MachOImage);
  if (KernelEntry == 0) {
    return EFI_NOT_FOUND;
  }

  if (SlideAddr > 0) {
    KernelEntry += SlideAddr;
  }

  return KernelEntryPatchJump ((UINT32)KernelEntry);
}

/** Callback called when boot.efi jumps to kernel. */
UINTN
EFIAPI
KernelEntryPatchJumpBack (
  UINTN    Args,
  BOOLEAN  ModeX64
  )
{
  if (gHibernateWake) {
    UpdateEnvironmentForHibernateWake (Args);
  } else {
    UpdateEnvironmentForBooting (Args);
  }

  //
  // Restore original kernel entry code.
  //
  CopyMem((VOID *)(UINTN)AsmKernelEntry, (VOID *)mOrigKernelCode, mOrigKernelCodeSize);

  return Args;
}

/** Copies RT flagged areas to separate memmap, defines virtual to phisycal address mapping
 * and calls SetVirtualAddressMap() only with that partial memmap.
 *
 * About partial memmap:
 * Some UEFIs are converting pointers to virtual addresses even if they do not
 * point to regions with RT flag. This means that those UEFIs are using
 * Desc->VirtualStart even for non-RT regions. Linux had issues with this:
 * http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commit;h=7cb00b72876ea2451eb79d468da0e8fb9134aa8a
 * They are doing it Windows way now - copying RT descriptors to separate
 * mem map and passing that stripped map to SetVirtualAddressMap().
 * We'll do the same, although it seems that just assigning
 * VirtualStart = PhysicalStart for non-RT areas also does the job.
 *
 * About virtual to phisycal mappings:
 * Also adds virtual to phisycal address mappings for RT areas. This is needed since
 * SetVirtualAddressMap() does not work on my Aptio without that. Probably because some driver
 * has a bug and is trying to access new virtual addresses during the change.
 * Linux and Windows are doing the same thing and problem is
 * not visible there.
 */
EFI_STATUS
ExecSetVirtualAddressesToMemMap (
  IN UINTN                  MemoryMapSize,
  IN UINTN                  DescriptorSize,
  IN UINT32                 DescriptorVersion,
  IN EFI_MEMORY_DESCRIPTOR  *MemoryMap
  )
{
  UINTN                           NumEntries;
  UINTN                           Index;
  EFI_MEMORY_DESCRIPTOR           *Desc;
  EFI_MEMORY_DESCRIPTOR           *VirtualDesc;
  EFI_STATUS                      Status;
  PAGE_MAP_AND_DIRECTORY_POINTER  *PageTable;
  UINTN                           Flags;
  UINTN                           BlockSize;

  Desc                      = MemoryMap;
  NumEntries                = MemoryMapSize / DescriptorSize;
  VirtualDesc               = mVirtualMemoryMap;
  mVirtualMapSize           = 0;
  mVirtualMapDescriptorSize = DescriptorSize;
  DEBUG ((DEBUG_VERBOSE, "ExecSetVirtualAddressesToMemMap: Size=%d, Addr=%p, DescSize=%d\n", MemoryMapSize, MemoryMap, DescriptorSize));

  //
  // Get current VM page table
  //
  GetCurrentPageTable (&PageTable, &Flags);

  for (Index = 0; Index < NumEntries; Index++) {
    //
    // Some UEFIs end up with "reserved" area with EFI_MEMORY_RUNTIME flag set when Intel HD3000 or HD4000 is used.
    // For example, on GA-H81N-D2H there is a single 1 GB descriptor:
    // 000000009F800000-00000000DF9FFFFF 0000000000040200 8000000000000000
    //
    // All known boot.efi starting from at least 10.5.8 properly handle this flag and do not assign virtual addresses
    // to reserved descriptors.
    // However, the issue was with AptioFix itself, which did not check for EfiReservedMemoryType and replaced
    // it by EfiMemoryMappedIO to prevent boot.efi relocations.
    //
    // The relevant discussion and the original fix can be found here:
    // http://web.archive.org/web/20141111124211/http://www.projectosx.com:80/forum/lofiversion/index.php/t2428-450.html
    // https://sourceforge.net/p/cloverefiboot/code/605/
    //
    // Since it is not the bug in boot.efi, AptioMemoryFix only needs to properly handle EfiReservedMemoryType with
    // EFI_MEMORY_RUNTIME attribute set, and there is no reason to mess with the memory map passed to boot.efi.
    //
    if (Desc->Type != EfiReservedMemoryType && (Desc->Attribute & EFI_MEMORY_RUNTIME) != 0) {
      //
      // Check if there is enough space in mVirtualMemoryMap.
      //
      if (mVirtualMapSize + DescriptorSize > sizeof(mVirtualMemoryMap)) {
        DEBUG ((DEBUG_INFO, "ERROR: too much mem map RT areas\n"));
        return EFI_OUT_OF_RESOURCES;
      }

      //
      // Copy region with EFI_MEMORY_RUNTIME flag to mVirtualMemoryMap.
      //
      CopyMem((VOID*)VirtualDesc, (VOID*)Desc, DescriptorSize);

      //
      // Define virtual to phisical mapping.
      //
      DEBUG ((DEBUG_VERBOSE, "Map pages: %lx (%x) -> %lx\n", Desc->VirtualStart, Desc->NumberOfPages, Desc->PhysicalStart));
      VmMapVirtualPages (PageTable, Desc->VirtualStart, Desc->NumberOfPages, Desc->PhysicalStart);

      //
      // Next mVirtualMemoryMap slot.
      //
      VirtualDesc = NEXT_MEMORY_DESCRIPTOR (VirtualDesc, DescriptorSize);
      mVirtualMapSize += DescriptorSize;

      //
      // Remember future physical address for the relocated system table.
      //
      BlockSize = EFI_PAGES_TO_SIZE ((UINTN)Desc->NumberOfPages);
      if (Desc->PhysicalStart <= gSysTableRtArea &&  gSysTableRtArea < (Desc->PhysicalStart + BlockSize)) {
        //
        // Future physical = VirtualStart & 0x7FFFFFFFFF
        //
        gRelocatedSysTableRtArea = (Desc->VirtualStart & 0x7FFFFFFFFF) + (gSysTableRtArea - Desc->PhysicalStart);
      }
    }

    Desc = NEXT_MEMORY_DESCRIPTOR (Desc, DescriptorSize);
  }

  VmFlushCaches ();

  DEBUG ((DEBUG_VERBOSE, "ExecSetVirtualAddressesToMemMap: Size=%d, Addr=%p, DescSize=%d\nSetVirtualAddressMap ... ",
    mVirtualMapSize, MemoryMap, DescriptorSize));
  Status = gRT->SetVirtualAddressMap (mVirtualMapSize, DescriptorSize, DescriptorVersion, mVirtualMemoryMap);
  DEBUG ((DEBUG_VERBOSE, "%r\n", Status));

  return Status;
}

VOID
CopyEfiSysTableToRtArea (
  IN OUT UINT32  *EfiSystemTable
  )
{
  EFI_SYSTEM_TABLE  *Src;
  EFI_SYSTEM_TABLE  *Dest;

  Src  = (EFI_SYSTEM_TABLE*)(UINTN)*EfiSystemTable;
  Dest = (EFI_SYSTEM_TABLE*)(UINTN)gSysTableRtArea;

  CopyMem(Dest, Src, Src->Hdr.HeaderSize);

  *EfiSystemTable = (UINT32)(UINTN)Dest;
}

/**
 Returns the length of PathName.

 @param[in] FilePath  The file Device Path node to inspect.

 **/
UINTN
FileDevicePathNameLen (IN CONST FILEPATH_DEVICE_PATH  *FilePath)
{
  UINTN Size;
  UINTN Len;

  if (!FilePath) {
    return 0;
  }

  if (!IsDevicePathValid (&FilePath->Header, 0)) {
    return 0;
  }

  Size = DevicePathNodeLength (FilePath) - SIZE_OF_FILEPATH_DEVICE_PATH;
  //
  // Account for more than one termination character.
  //
  Len = (Size / sizeof (*FilePath->PathName)) - 1;
  while (Len > 0 && FilePath->PathName[Len - 1] == L'\0') {
    --Len;
  }

  return Len;
}


EFI_LOADED_IMAGE_PROTOCOL *
GetAppleBootLoadedImage (
  EFI_HANDLE  ImageHandle
  )
{
  EFI_STATUS                  Status;
  EFI_LOADED_IMAGE_PROTOCOL   *LoadedImage  = NULL;
  EFI_DEVICE_PATH_PROTOCOL    *CurrNode     = NULL;
  FILEPATH_DEVICE_PATH        *LastNode     = NULL;
  BOOLEAN                     IsMacOS       = FALSE;
  UINTN                       PathLen       = 0;
  UINTN                       BootPathLen   = L_STR_LEN ("boot.efi");
  UINTN                       Index;

  Status = gBS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&LoadedImage);

  if (!EFI_ERROR(Status) && LoadedImage->FilePath) {
    for (CurrNode = LoadedImage->FilePath; !IsDevicePathEnd (CurrNode); CurrNode = NextDevicePathNode (CurrNode)) {
      if (CurrNode->Type == MEDIA_DEVICE_PATH && CurrNode->SubType == MEDIA_FILEPATH_DP) {
        LastNode = (FILEPATH_DEVICE_PATH *)CurrNode;
      }
    }

    if (LastNode) {
      //
      // Detect macOS by boot.efi in the bootloader name.
      //
      PathLen = FileDevicePathNameLen (LastNode);
      if (PathLen >= BootPathLen) {
        Index = PathLen - BootPathLen;
        IsMacOS = (Index == 0 || LastNode->PathName[Index - 1] == L'\\')
          && !CompareMem (&LastNode->PathName[Index], L"boot.efi", L_STR_SIZE (L"boot.efi"));
      }
    }
  }

  return IsMacOS ? LoadedImage : NULL;
}
