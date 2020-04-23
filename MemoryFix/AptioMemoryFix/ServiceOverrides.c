/**

  Temporary BS and RT overrides for boot.efi support.
  Unlike RtShims they do not affect the kernel.

  by dmazar

**/

#include <IndustryStandard/AppleHibernate.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/OcMiscLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/LoadedImage.h>

#include "Config.h"
#include "ServiceOverrides.h"
#include "BootArgs.h"
#include "BootFixes.h"
#include "CustomSlide.h"
#include "MemoryMap.h"
#include "RtShims.h"
#include "UmmMalloc/UmmMalloc.h"

//
// Placeholders for storing original Boot and RT Services functions
//
STATIC EFI_ALLOCATE_PAGES          mStoredAllocatePages;
STATIC EFI_ALLOCATE_POOL           mStoredAllocatePool;
STATIC EFI_FREE_POOL               mStoredFreePool;
STATIC EFI_GET_MEMORY_MAP          mStoredGetMemoryMap;
STATIC EFI_EXIT_BOOT_SERVICES      mStoredExitBootServices;
STATIC EFI_SET_VIRTUAL_ADDRESS_MAP mStoredSetVirtualAddressMap;
STATIC EFI_IMAGE_START             mStoredStartImage;

//
// Original runtime services hash we restore at address virtualisation
//
STATIC UINT32               mRtPreOverridesCRC32;

//
// Location of memory allocated by boot.efi for hibernate image
//
STATIC EFI_PHYSICAL_ADDRESS mHibernateImageAddress;

//
// Saved exit boot services arguments
//
STATIC EFI_HANDLE           mExitBSImageHandle;
STATIC UINTN                mExitBSMapKey;

//
// Dynamic memory allocation filtering status
//
STATIC BOOLEAN              mFilterDynamicPoolAllocations;

//
// Minimum and maximum addresses allocated by AlocatePages
//
STATIC EFI_PHYSICAL_ADDRESS mMinAllocatedAddr;
STATIC EFI_PHYSICAL_ADDRESS mMaxAllocatedAddr;

//
// Amount of nested boot.efi detected
//
UINTN                       gMacOSBootNestedCount;

//
// Last descriptor size obtained from GetMemoryMap
//
UINTN                       gMemoryMapDescriptorSize = sizeof(EFI_MEMORY_DESCRIPTOR);

VOID
InstallBsOverrides (
  VOID
  )
{
#if APTIOFIX_CUSTOM_POOL_ALLOCATOR == 1
  EFI_STATUS               Status;
  EFI_PHYSICAL_ADDRESS     UmmHeap = BASE_4GB;
  UINTN                    PageNum = EFI_SIZE_TO_PAGES (APTIOFIX_CUSTOM_POOL_ALLOCATOR_SIZE);

  //
  // We do not uninstall our custom allocator to avoid memory corruption issues
  // when shimming AMI code. This check ensures that we do not install it twice.
  // See UninstallBsOverrides for more details.
  //
  if (!UmmInitialized ()) {
    //
    // Enforce memory pool creation when -aptiodump argument is used, but let it slip otherwise.
    //
    Status = AllocatePagesFromTop (EfiBootServicesData, PageNum, &UmmHeap, !gDumpMemArgPresent);
    if (!EFI_ERROR(Status)) {
      SetMem((VOID *)UmmHeap, APTIOFIX_CUSTOM_POOL_ALLOCATOR_SIZE, 0);
      UmmSetHeap ((VOID *)UmmHeap);

      mStoredAllocatePool   = gBS->AllocatePool;
      mStoredFreePool       = gBS->FreePool;

      gBS->AllocatePool     = MOAllocatePool;
      gBS->FreePool         = MOFreePool;
    } else {
      //
      // This is undesired, but technically less fatal than attempting to reduce the number
      // of slides available when no memory map dumping is necessary, for example.
      //
      Print (L"AMF: Not using custom memory pool - %r\n", Status);
    }
  }
#endif

  mStoredAllocatePages    = gBS->AllocatePages;
  mStoredGetMemoryMap     = gBS->GetMemoryMap;
  mStoredExitBootServices = gBS->ExitBootServices;
  mStoredStartImage       = gBS->StartImage;

  gBS->AllocatePages      = MOAllocatePages;
  gBS->GetMemoryMap       = MOGetMemoryMap;
  gBS->ExitBootServices   = MOExitBootServices;
  gBS->StartImage         = MOStartImage;

  gBS->Hdr.CRC32 = 0;
  gBS->CalculateCrc32 (gBS, gBS->Hdr.HeaderSize, &gBS->Hdr.CRC32);
}

VOID
InstallRtOverrides (
  VOID
  )
{
  mRtPreOverridesCRC32 = gRT->Hdr.CRC32;

  mStoredSetVirtualAddressMap = gRT->SetVirtualAddressMap;

  gRT->SetVirtualAddressMap = MOSetVirtualAddressMap;

  gRT->Hdr.CRC32 = 0;
  gBS->CalculateCrc32 (gRT, gRT->Hdr.HeaderSize, &gRT->Hdr.CRC32);
}

VOID
UninstallRtOverrides (
  VOID
  )
{
  gRT->SetVirtualAddressMap = mStoredSetVirtualAddressMap;

  gRT->Hdr.CRC32 = mRtPreOverridesCRC32;
}

VOID
DisableDynamicPoolAllocations (
  VOID
  )
{
  mFilterDynamicPoolAllocations = TRUE;
}

VOID
EnableDynamicPoolAllocations (
  VOID
  )
{
  mFilterDynamicPoolAllocations = FALSE;
}

/** gBS->StartImage override:
 * Called to start an efi image.
 *
 * If this is boot.efi, then run it with our overrides.
 */
EFI_STATUS
EFIAPI
MOStartImage (
  IN     EFI_HANDLE  ImageHandle,
     OUT UINTN       *ExitDataSize,
     OUT CHAR16      **ExitData  OPTIONAL
  )
{
  EFI_STATUS                  Status;
  EFI_LOADED_IMAGE_PROTOCOL   *AppleLoadedImage = NULL;
  UINTN                       ValueSize = 0;
  VOID                        *Gop;

  DEBUG ((DEBUG_VERBOSE, "StartImage (%lx)\n", ImageHandle));

  AppleLoadedImage = GetAppleBootLoadedImage (ImageHandle);

  //
  // Clear monitoring vars
  //
  mMinAllocatedAddr = 0;
  mMaxAllocatedAddr = 0;

  //
  // Request boot variable redirection if enabled.
  //
  SetBootVariableRedirect (TRUE);

  if (AppleLoadedImage) {
    //
    // Report about macOS being loaded.
    //
    gMacOSBootNestedCount++;

    //
    // Latest Windows brings Virtualization-based security and monitors
    // CR0 by launching itself under a hypevisor. Since we need WP disable
    // on macOS to let NVRAM work, and for the time being no other OS
    // requires it, here we decide to use it for macOS exclusively.
    //
    SetWriteUnprotectorMode (TRUE);

    //
    // Boot.efi requires EfiGraphicsOutputProtocol on ConOutHandle, but it is not present
    // there on Aptio 2.0. EfiGraphicsOutputProtocol exists on some other handle.
    // If this is the case, we'll intercept that call and return EfiGraphicsOutputProtocol
    // from that other handle.
    //
    Gop = NULL;
    Status = gBS->HandleProtocol (gST->ConsoleOutHandle, &gEfiGraphicsOutputProtocolGuid, &Gop);
    if (EFI_ERROR(Status)) {
      Status = gBS->LocateProtocol (&gEfiGraphicsOutputProtocolGuid, NULL, &Gop);
      if (!EFI_ERROR(Status)) {
        gBS->InstallMultipleProtocolInterfaces (
          &gST->ConsoleOutHandle,
          &gEfiGraphicsOutputProtocolGuid,
          Gop,
          NULL
          );
      }
    }

    //
    // This is reverse engineered from boot.efi.
    // To cancel hibernate wake it is enough to delete the variables.
    // Starting with 10.13.6 boot-switch-vars is no longer supported.
    //
    ValueSize = 0;
    if (gRT->GetVariable (L"boot-signature", &gEfiAppleBootGuid, NULL, &ValueSize, NULL) == EFI_BUFFER_TOO_SMALL) {
      ValueSize = 0;
      if (gRT->GetVariable (L"boot-image-key", &gEfiAppleBootGuid, NULL, &ValueSize, NULL) == EFI_BUFFER_TOO_SMALL) {
        gHibernateWake = TRUE;
      }
    } else {
      ValueSize = 0;
      if (gRT->GetVariable (L"boot-switch-vars", &gEfiAppleBootGuid, NULL, &ValueSize, NULL) == EFI_BUFFER_TOO_SMALL) {
        gHibernateWake = TRUE;
      }
    }

    //
    // Save current 64bit state - will be restored later in callback from kernel jump
    // and relocate JumpToKernel32 code to higher mem (for copying kernel back to
    // proper place and jumping back to it)
    //
    Status = PrepareJumpFromKernel ();
    if (!EFI_ERROR(Status)) {
      //
      // Force boot.efi to use our copy of system table
      //
      AppleLoadedImage->SystemTable = (EFI_SYSTEM_TABLE *)(UINTN)gSysTableRtArea;

  #if APTIOFIX_ALLOW_ASLR_IN_SAFE_MODE == 1
      UnlockSlideSupportForSafeMode ((UINT8 *)AppleLoadedImage->ImageBase, AppleLoadedImage->ImageSize);
  #endif

      //
      // Read options
      //
      ReadBooterArguments ((CHAR16*)AppleLoadedImage->LoadOptions, AppleLoadedImage->LoadOptionsSize/sizeof (CHAR16));
    }
  }


  Status = mStoredStartImage (ImageHandle, ExitDataSize, ExitData);

  if (AppleLoadedImage) {
    //
    // We failed but other operating systems should be loadable.
    //
    gMacOSBootNestedCount--;
    SetWriteUnprotectorMode (FALSE);
  }

  //
  // Disable redirect on failure, this is cleaner design-wise.
  //
  SetBootVariableRedirect (FALSE);

  return Status;
}

/** gBS->AllocatePages override:
 * Returns pages from free memory block to boot.efi for kernel boot image.
 */
EFI_STATUS
EFIAPI
MOAllocatePages (
  IN     EFI_ALLOCATE_TYPE     Type,
  IN     EFI_MEMORY_TYPE       MemoryType,
  IN     UINTN                 NumberOfPages,
  IN OUT EFI_PHYSICAL_ADDRESS  *Memory
  )
{
  EFI_STATUS              Status;
  EFI_PHYSICAL_ADDRESS    UpperAddr;

  if (gMacOSBootNestedCount > 0 && Type == AllocateAddress && MemoryType == EfiLoaderData) {
    //
    // Called from boot.efi
    //
    UpperAddr = *Memory + EFI_PAGES_TO_SIZE (NumberOfPages);

    //
    // Store min and max mem - can be used later to determine start and end of kernel boot image
    //
    if (mMinAllocatedAddr == 0 || *Memory < mMinAllocatedAddr)
      mMinAllocatedAddr = *Memory;
    if (UpperAddr > mMaxAllocatedAddr)
      mMaxAllocatedAddr = UpperAddr;

    Status = mStoredAllocatePages (Type, MemoryType, NumberOfPages, Memory);
  } else if (gMacOSBootNestedCount > 0 && gHibernateWake && Type == AllocateAnyPages && MemoryType == EfiLoaderData) {
    //
    // Called from boot.efi during hibernate wake,
    // first such allocation is for hibernate image
    //
    Status = mStoredAllocatePages (Type, MemoryType, NumberOfPages, Memory);
    if (mHibernateImageAddress == 0 && Status == EFI_SUCCESS) {
      mHibernateImageAddress = *Memory;
    }
  } else {
    //
    // Generic page allocation
    //
    Status = mStoredAllocatePages (Type, MemoryType, NumberOfPages, Memory);
  }

  return Status;
}

/** gBS->AllocatePool override:
 * Allows us to use a custom allocator that uses a preallocated memory pool
 * for certain types of memory. See details in Print function.
 */
EFI_STATUS
EFIAPI
MOAllocatePool (
  IN     EFI_MEMORY_TYPE  Type,
  IN     UINTN            Size,
     OUT VOID             **Buffer
  )
{
  //
  // The code below allows us to more safely invoke Boot Services to perform onscreen
  // printing when no memory map modifications (pool memory allocation) is allowed.
  // While it is imperfect design-wise, it works very well on many ASUS Skylake boards
  // when performing memory map dumps via -aptiodump.
  //
  if (gMacOSBootNestedCount > 0 && Type == EfiBootServicesData && mFilterDynamicPoolAllocations) {
    *Buffer = UmmMalloc ((UINT32)Size);
    if (*Buffer)
      return EFI_SUCCESS;

    //
    // Dynamic pool allocations filtering should technically only be used when booting is more
    // important than not allocating the requested memory and failing to do something.
    // However, since we skip other types of allocations anyway, not falling back here to using
    // the default allocator may have its own consequences on other boards.
    //
  }

  return mStoredAllocatePool (Type, Size, Buffer);
}

/** gBS->FreePool override:
 * Allows us to use a custom allocator for certain types of memory.
 */
EFI_STATUS
EFIAPI
MOFreePool(
  IN VOID  *Buffer
  )
{
  //
  // By default it will return FALSE if Buffer was not allocated by us.
  //
  if (UmmFree (Buffer))
    return EFI_SUCCESS;

  return mStoredFreePool(Buffer);
}

/** gBS->GetMemoryMap override:
 * Returns shrinked memory map. XNU can handle up to PMAP_MEMORY_REGIONS_SIZE (128) entries.
 */
EFI_STATUS
EFIAPI
MOGetMemoryMap (
  IN OUT UINTN                  *MemoryMapSize,
  IN OUT EFI_MEMORY_DESCRIPTOR  *MemoryMap,
     OUT UINTN                  *MapKey,
     OUT UINTN                  *DescriptorSize,
     OUT UINT32                 *DescriptorVersion
  )
{
  EFI_STATUS            Status;

  Status = mStoredGetMemoryMap (MemoryMapSize, MemoryMap, MapKey, DescriptorSize, DescriptorVersion);

  if (gMacOSBootNestedCount > 0 && Status == EFI_SUCCESS) {
    if (gDumpMemArgPresent) {
      PrintMemMap (L"GetMemoryMap", *MemoryMapSize, *DescriptorSize, MemoryMap, gRtShims, gSysTableRtArea);
    }

#if APTIOFIX_PROTECT_CSM_REGION == 1
    ProtectCsmRegion (*MemoryMapSize, MemoryMap, *DescriptorSize);
#endif

    ShrinkMemMap (MemoryMapSize, MemoryMap, *DescriptorSize);

    //
    // Remember some descriptor size, since we will not have it later
    // during hibernate wake to be able to iterate memory map.
    //
    gMemoryMapDescriptorSize = *DescriptorSize;
  }

  return Status;
}

EFI_STATUS
EFIAPI
OrgGetMemoryMap (
  IN OUT UINTN                  *MemoryMapSize,
  IN OUT EFI_MEMORY_DESCRIPTOR  *MemoryMap,
     OUT UINTN                  *MapKey,
     OUT UINTN                  *DescriptorSize,
     OUT UINT32                 *DescriptorVersion
  )
{
  return (mStoredGetMemoryMap ? mStoredGetMemoryMap : gBS->GetMemoryMap) (
    MemoryMapSize,
    MemoryMap,
    MapKey,
    DescriptorSize,
    DescriptorVersion
    );
}

/** gBS->ExitBootServices override:
 * Patches kernel entry point with jump to our KernelEntryPatchJumpBack().
 */
EFI_STATUS
EFIAPI
MOExitBootServices (
  IN EFI_HANDLE  ImageHandle,
  IN UINTN       MapKey
  )
{
  EFI_STATUS               Status;
  UINTN                    SlideAddr = 0;
  VOID                     *MachOImage = NULL;
  IOHibernateImageHeader   *ImageHeader = NULL;

  //
  // For non-macOS operating systems return directly.
  //
  if (gMacOSBootNestedCount == 0) {
    return mStoredExitBootServices (ImageHandle, MapKey);
  }

  //
  // We need hibernate image address for wake
  //
  if (gHibernateWake && mHibernateImageAddress == 0) {
    Print (L"AMF: Failed to find hibernate image address\n");
    gBS->Stall (SECONDS_TO_MICROSECONDS (5));
    return EFI_INVALID_PARAMETER;
  }

  //
  // We can just return EFI_SUCCESS and continue using Print for debug
  //
  if (gDumpMemArgPresent) {
    mExitBSImageHandle = ImageHandle;
    mExitBSMapKey      = MapKey; 
    Status             = EFI_SUCCESS;
  } else {
    Status = ForceExitBootServices (mStoredExitBootServices, ImageHandle, MapKey);
  }

  if (EFI_ERROR(Status))
    return Status;

  if (!gHibernateWake) {
    SlideAddr  = mMinAllocatedAddr - 0x100000;
    MachOImage = (VOID*)(UINTN)(SlideAddr + SLIDE_GRANULARITY);
    KernelEntryFromMachOPatchJump (MachOImage, SlideAddr);
  } else {
    //
    // At this stage HIB section is not yet copied from sleep image to it's
    // proper memory destination. so we'll patch entry point in sleep image.
    //
    ImageHeader = (IOHibernateImageHeader *)(UINTN)mHibernateImageAddress;
    KernelEntryPatchJump (
      ((UINT32)(UINTN)&(ImageHeader->fileExtentMap[0])) + ImageHeader->fileExtentMapSize + ImageHeader->restore1CodeOffset
      );
  }

  return Status;
}

/** Helper function to call ExitBootServices that can handle outdated MapKey issues. */
EFI_STATUS
ForceExitBootServices (
  IN EFI_EXIT_BOOT_SERVICES  ExitBs,
  IN EFI_HANDLE              ImageHandle,
  IN UINTN                   MapKey
  )
{
  EFI_STATUS               Status;
  EFI_MEMORY_DESCRIPTOR    *MemoryMap;
  UINTN                    MemoryMapSize;
  UINTN                    DescriptorSize;
  UINT32                   DescriptorVersion;

  //
  // Firstly try the easy way
  //
  Status = ExitBs (ImageHandle, MapKey);

  if (EFI_ERROR(Status)) {
    //
    // Just report error as var in nvram to be visible from macOS with "nvram -p"
    //
    gRT->SetVariable (L"aptiomemfix-exitbs",
      &gEfiAppleBootGuid,
      EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
      4,
      "fail"
      );

    //
    // It is too late to free memory map here, but it does not matter, because boot.efi has an old one
    // and will freely use the memory.
    // It is technically forbidden to allocate pool memory here, but we should not hit this code
    // in the first place, and for older firmwares, where it was necessary (?), it worked just fine.
    //
    Status = GetMemoryMapAlloc (NULL, &MemoryMapSize, &MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (Status == EFI_SUCCESS) {
      //
      // We have the latest memory map and its key, try again!
      //
      Status = ExitBs (ImageHandle, MapKey);
      if (EFI_ERROR(Status))
        Print (L"AMF: ExitBootServices failed twice - %r\n", Status);
    } else {
      Print (L"AMF: Failed to get MapKey for ExitBootServices - %r\n", Status);
      Status = EFI_INVALID_PARAMETER;
    }

    if (EFI_ERROR(Status)) {
      Print (L"Waiting 10 secs...\n");
      gBS->Stall (SECONDS_TO_MICROSECONDS (10));
    }
  }

  return Status;
}

/** gRT->SetVirtualAddressMap override:
 * Fixes virtualizing of RT services.
 */
EFI_STATUS
EFIAPI
MOSetVirtualAddressMap (
  IN UINTN                  MemoryMapSize,
  IN UINTN                  DescriptorSize,
  IN UINT32                 DescriptorVersion,
  IN EFI_MEMORY_DESCRIPTOR  *VirtualMap
  )
{
  EFI_STATUS   Status;
  UINT32       EfiSystemTable;

  //
  // We do not need to recover BS, since they will be invalid.
  //
  UninstallRtOverrides ();

  //
  // Apply the necessary changes for macOS support
  //
  if (gMacOSBootNestedCount > 0) {
    if (gDumpMemArgPresent) {
      PrintMemMap (L"SetVirtualAddressMap", MemoryMapSize, DescriptorSize, VirtualMap, gRtShims, gSysTableRtArea);
      //
      // To print as much information as possible we delay ExitBootServices.
      // Most likely this will fail, but let's still try!
      //
      ForceExitBootServices (mStoredExitBootServices, mExitBSImageHandle, mExitBSMapKey);
    }

    //
    // Protect RT areas from relocation by marking then MemMapIO
    //
    ProtectRtMemoryFromRelocation (MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap, gSysTableRtArea);

    //
    // Remember physical sys table addr
    //
    EfiSystemTable = (UINT32)(UINTN)gST;

    //
    // Virtualize RT services with all needed fixes
    //
    Status = ExecSetVirtualAddressesToMemMap (MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap);

    CopyEfiSysTableToRtArea (&EfiSystemTable);
  } else {
    Status = gRT->SetVirtualAddressMap (MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap);
  }

  //
  // Correct shim pointers right away
  //
  VirtualizeRtShims (MemoryMapSize, DescriptorSize, VirtualMap);

  return Status;
}
