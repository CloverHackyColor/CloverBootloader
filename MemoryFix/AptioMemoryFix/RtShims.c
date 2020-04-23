/**

  Runtime Services Wrappers.

  by Download-Fritz & vit9696

**/

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/OcMiscLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Guid/OcVariables.h>

#include "Config.h"
#include "RtShims.h"
#include "BootFixes.h"
#include "MemoryMap.h"

extern UINTN gRtShimsDataStart;
extern UINTN gRtShimsDataEnd;

extern UINTN gGetVariable;
extern UINTN gGetNextVariableName;
extern UINTN gSetVariable;
extern UINTN gGetTime;
extern UINTN gSetTime;
extern UINTN gGetWakeupTime;
extern UINTN gSetWakeupTime;
extern UINTN gGetNextHighMonoCount;
extern UINTN gResetSystem;
extern UINTN gGetVariableOverride;

extern UINTN gRequiresWriteUnprotect;
extern UINTN gBootVariableRedirect;

extern EFI_GUID gReadOnlyVariableGuid;
extern EFI_GUID gWriteOnlyVariableGuid;
extern EFI_GUID gBootVariableGuid;
extern EFI_GUID gRedirectVariableGuid;

extern UINTN RtShimGetVariable;
extern UINTN RtShimGetNextVariableName;
extern UINTN RtShimSetVariable;
extern UINTN RtShimGetTime;
extern UINTN RtShimSetTime;
extern UINTN RtShimGetWakeupTime;
extern UINTN RtShimSetWakeupTime;
extern UINTN RtShimGetNextHighMonoCount;
extern UINTN RtShimResetSystem;

VOID *gRtShims = NULL;

STATIC BOOLEAN mRtShimsAddrUpdated = FALSE;

STATIC RT_SHIM_PTRS mShimPtrArray[] = {
  { &gGetVariable },
  { &gSetVariable },
  { &gGetNextVariableName },
  { &gGetTime },
  { &gSetTime },
  { &gGetWakeupTime },
  { &gSetWakeupTime },
  { &gGetNextHighMonoCount },
  { &gResetSystem }
};

STATIC RT_RELOC_PROTECT_DATA mRelocInfoData;

VOID InstallRtShims (
  EFI_GET_VARIABLE GetVariableOverride
  )
{
  EFI_STATUS            Status;
  UINTN                 PageCount;
  EFI_PHYSICAL_ADDRESS  RtShims = BASE_4GB;

  //
  // Support read-only and write-only variables from runtime-services.
  //
  CopyGuid (&gReadOnlyVariableGuid, &gOcReadOnlyVariableGuid);
  CopyGuid (&gWriteOnlyVariableGuid, &gOcWriteOnlyVariableGuid);
  CopyGuid (&gBootVariableGuid, &gEfiGlobalVariableGuid);
  CopyGuid (&gRedirectVariableGuid, &gOcVendorVariableGuid);

  if (gHasBrokenS4Allocator) {
    //
    // Some firmwares appear to allocate rt shims at randomly incomprehensible area.
    // This unfortunately results in crashes and using pool allocs is one of the workarounds.
    //
    Status = gBS->AllocatePool (
      EfiRuntimeServicesCode,
      ((UINTN)&gRtShimsDataEnd - (UINTN)&gRtShimsDataStart),
      &gRtShims
      );
  } else {
    //
    // It is important to allocate properly from top on saner firmwares.
    // If we do not and use hacks like above many other operating systems (like Linux)
    // may stop loading.
    //
    PageCount = EFI_SIZE_TO_PAGES ((UINTN)&gRtShimsDataEnd - (UINTN)&gRtShimsDataStart);
    Status = AllocatePagesFromTop (
      EfiRuntimeServicesCode,
      PageCount,
      &RtShims,
      FALSE
      );
    if (!EFI_ERROR(Status)) {
      gRtShims = (VOID *)(UINTN)RtShims;
    }
  }

  if (!EFI_ERROR(Status)) {
    gGetVariable          = (UINTN)gRT->GetVariable;
    gSetVariable          = (UINTN)gRT->SetVariable;
    gGetNextVariableName  = (UINTN)gRT->GetNextVariableName;
    gGetTime              = (UINTN)gRT->GetTime;
    gSetTime              = (UINTN)gRT->SetTime;
    gGetWakeupTime        = (UINTN)gRT->GetWakeupTime;
    gSetWakeupTime        = (UINTN)gRT->SetWakeupTime;
    gGetNextHighMonoCount = (UINTN)gRT->GetNextHighMonotonicCount;
    gResetSystem          = (UINTN)gRT->ResetSystem;

    gGetVariableOverride  = (UINTN)GetVariableOverride;

    CopyMem (
      gRtShims,
      (VOID *)&gRtShimsDataStart,
      ((UINTN)&gRtShimsDataEnd - (UINTN)&gRtShimsDataStart)
      );

    gRT->GetVariable               = (EFI_GET_VARIABLE)((UINTN)gRtShims              + ((UINTN)&RtShimGetVariable          - (UINTN)&gRtShimsDataStart));
    gRT->SetVariable               = (EFI_SET_VARIABLE)((UINTN)gRtShims              + ((UINTN)&RtShimSetVariable          - (UINTN)&gRtShimsDataStart));
    gRT->GetNextVariableName       = (EFI_GET_NEXT_VARIABLE_NAME)((UINTN)gRtShims    + ((UINTN)&RtShimGetNextVariableName  - (UINTN)&gRtShimsDataStart));
    gRT->GetTime                   = (EFI_GET_TIME)((UINTN)gRtShims                  + ((UINTN)&RtShimGetTime              - (UINTN)&gRtShimsDataStart));
    gRT->SetTime                   = (EFI_SET_TIME)((UINTN)gRtShims                  + ((UINTN)&RtShimSetTime              - (UINTN)&gRtShimsDataStart));
    gRT->GetWakeupTime             = (EFI_GET_WAKEUP_TIME)((UINTN)gRtShims           + ((UINTN)&RtShimGetWakeupTime        - (UINTN)&gRtShimsDataStart));
    gRT->SetWakeupTime             = (EFI_SET_WAKEUP_TIME)((UINTN)gRtShims           + ((UINTN)&RtShimSetWakeupTime        - (UINTN)&gRtShimsDataStart));
    gRT->GetNextHighMonotonicCount = (EFI_GET_NEXT_HIGH_MONO_COUNT)((UINTN)gRtShims  + ((UINTN)&RtShimGetNextHighMonoCount - (UINTN)&gRtShimsDataStart));
    gRT->ResetSystem               = (EFI_RESET_SYSTEM)((UINTN)gRtShims              + ((UINTN)&RtShimResetSystem          - (UINTN)&gRtShimsDataStart));

    gRT->Hdr.CRC32 = 0;
    gBS->CalculateCrc32(gRT, gRT->Hdr.HeaderSize, &gRT->Hdr.CRC32);
  } else {
    DEBUG ((DEBUG_VERBOSE, "Nulling RtShims\n"));
    gRtShims = NULL;
  }
}

VOID
VirtualizeRtShims (
  UINTN                  MemoryMapSize,
  UINTN                  DescriptorSize,
  EFI_MEMORY_DESCRIPTOR  *MemoryMap
  )
{
  EFI_MEMORY_DESCRIPTOR  *Desc;
  UINTN                  Index, Index2, FixedCount = 0;

  //
  // For some reason creating an event for catching SetVirtualAddress doesn't work on APTIO IV Z77,
  // So we cannot use a dedicated ConvertPointer function and have to implement everything manually.
  //

  //
  // Are we already done?
  //
  if (mRtShimsAddrUpdated)
    return;

  Desc = MemoryMap;

  //
  // Custom GetVariable wrapper is no longer allowed!
  //
  *(UINTN *)((UINTN)gRtShims + ((UINTN)&gGetVariableOverride - (UINTN)&gRtShimsDataStart)) = 0;

  for (Index = 0; Index < ARRAY_SIZE (mShimPtrArray); ++Index) {
    mShimPtrArray[Index].Func = (UINTN *)((UINTN)gRtShims + ((UINTN)(mShimPtrArray[Index].gFunc) - (UINTN)&gRtShimsDataStart));
  }

  for (Index = 0; Index < (MemoryMapSize / DescriptorSize); ++Index) {
    for (Index2 = 0; Index2 < ARRAY_SIZE (mShimPtrArray); ++Index2) {
      if (
        !mShimPtrArray[Index2].Fixed &&
        (*(mShimPtrArray[Index2].gFunc) >= Desc->PhysicalStart) &&
        (*(mShimPtrArray[Index2].gFunc) < (Desc->PhysicalStart + EFI_PAGES_TO_SIZE (Desc->NumberOfPages)))
      ) {
        mShimPtrArray[Index2].Fixed = TRUE;
        *(mShimPtrArray[Index2].Func) += (Desc->VirtualStart - Desc->PhysicalStart);
        FixedCount++;
      }
    }

    if (FixedCount == ARRAY_SIZE (mShimPtrArray)) {
      break;
    }

    Desc = NEXT_MEMORY_DESCRIPTOR (Desc, DescriptorSize);
  }

  mRtShimsAddrUpdated = TRUE;
}

EFI_STATUS
EFIAPI
OrgGetVariable (
  IN     CHAR16    *VariableName,
  IN     EFI_GUID  *VendorGuid,
  OUT    UINT32    *Attributes OPTIONAL,
  IN OUT UINTN     *DataSize,
  OUT    VOID      *Data
  )
{
  return (gGetVariable ? (EFI_GET_VARIABLE)gGetVariable : gRT->GetVariable) (
    VariableName,
    VendorGuid,
    Attributes,
    DataSize,
    Data
    );
}

/** Protect RT data from relocation by marking them MemMapIO. Except area with EFI system table.
 *  This one must be relocated into kernel boot image or kernel will crash (kernel accesses it
 *  before RT areas are mapped into vm).
 *  This fixes NVRAM issues on some boards where access to nvram after boot services is possible
 *  only in SMM mode. RT driver passes data to SM handler through previously negotiated buffer
 *  and this buffer must not be relocated.
 *  Explained and examined in detail by CodeRush and night199uk:
 *  http://www.projectosx.com/forum/index.php?showtopic=3298
 *
 *  It seems this does not do any harm to others where this is not needed,
 *  so it's added as standard fix for all.
 *
 *  Starting with APTIO V for nvram to work not only data but could too can no longer be moved
 *  due to the use of commbuffers. This, however, creates a memory protection issue, because
 *  XNU maps RT data as RW and code as RX, and AMI appears use global variables in some RT drivers.
 *  For this reason we shim (most?) affected RT services via wrapers that unset the WP bit during
 *  the UEFI call and set it back on return.
 *  Explained in detail by Download-Fritz and vit9696:
 *  http://www.insanelymac.com/forum/topic/331381-aptiomemoryfix (first 2 links in particular).
 */
VOID
ProtectRtMemoryFromRelocation (
  IN     UINTN                  MemoryMapSize,
  IN     UINTN                  DescriptorSize,
  IN     UINT32                 DescriptorVersion,
  IN OUT EFI_MEMORY_DESCRIPTOR  *MemoryMap,
  IN     EFI_PHYSICAL_ADDRESS   SysTableArea
  )
{
  UINTN                   NumEntries;
  UINTN                   Index;
  EFI_MEMORY_DESCRIPTOR   *Desc;
  RT_RELOC_PROTECT_INFO   *RelocInfo;

  Desc = MemoryMap;
  NumEntries = MemoryMapSize / DescriptorSize;

  mRelocInfoData.NumEntries = 0;

  RelocInfo = &mRelocInfoData.RelocInfo[0];

  for (Index = 0; Index < NumEntries; Index++) {
    if ((Desc->Attribute & EFI_MEMORY_RUNTIME) != 0 &&
        (Desc->Type == EfiRuntimeServicesCode ||
        (Desc->Type == EfiRuntimeServicesData && Desc->PhysicalStart != SysTableArea))) {

      if (mRelocInfoData.NumEntries < ARRAY_SIZE (mRelocInfoData.RelocInfo)) {
        RelocInfo->PhysicalStart = Desc->PhysicalStart;
        RelocInfo->Type          = Desc->Type;
        ++RelocInfo;
        ++mRelocInfoData.NumEntries;
      } else {
        DEBUG ((DEBUG_INFO, "WARNING: Cannot save mem type for entry: %lx (type 0x%x)\n", Desc->PhysicalStart, (UINTN)Desc->Type));
      }

      DEBUG ((DEBUG_VERBOSE, "RT mem %lx (0x%x) -> MemMapIO\n", Desc->PhysicalStart, Desc->NumberOfPages));
      Desc->Type = EfiMemoryMappedIO;
    }

    Desc = NEXT_MEMORY_DESCRIPTOR (Desc, DescriptorSize);
  }
}

VOID
RestoreProtectedRtMemoryTypes (
  IN     UINTN                  MemoryMapSize,
  IN     UINTN                  DescriptorSize,
  IN OUT EFI_MEMORY_DESCRIPTOR  *MemoryMap
  )
{
  UINTN Index;
  UINTN Index2;
  UINTN NumEntriesLeft;

  NumEntriesLeft = mRelocInfoData.NumEntries;

  if (NumEntriesLeft > 0) {
    for (Index = 0; Index < (MemoryMapSize / DescriptorSize); ++Index) {
      if (NumEntriesLeft > 0) {
        for (Index2 = 0; Index2 < mRelocInfoData.NumEntries; ++Index2) {
          if (MemoryMap->PhysicalStart == mRelocInfoData.RelocInfo[Index2].PhysicalStart) {
            MemoryMap->Type = mRelocInfoData.RelocInfo[Index2].Type;
            --NumEntriesLeft;
          }
        }
      }

      MemoryMap = NEXT_MEMORY_DESCRIPTOR (MemoryMap, DescriptorSize);
    }
  }
}

VOID
SetWriteUnprotectorMode (
  IN     BOOLEAN                Enable
  )
{
  *(UINTN *)((UINTN)gRtShims + ((UINTN)&gRequiresWriteUnprotect - (UINTN)&gRtShimsDataStart)) = Enable;
}

BOOLEAN
EFIAPI
SetBootVariableRedirect (
  IN     BOOLEAN                Enable
  )
{
  UINTN       DataSize;
  EFI_STATUS  Status;
  BOOLEAN     Previous;

  if (Enable) {
    DataSize = sizeof (Enable);
    Status = gRT->GetVariable (
      OC_BOOT_REDIRECT_VARIABLE_NAME,
      &gOcVendorVariableGuid,
      NULL,
      &DataSize,
      &Enable
      );

    if (EFI_ERROR(Status)) {
      Enable = FALSE;
    }
  }

  Previous = *(BOOLEAN *)((UINTN)gRtShims + ((UINTN)&gBootVariableRedirect - (UINTN)&gRtShimsDataStart));
  *(BOOLEAN *)((UINTN)gRtShims + ((UINTN)&gBootVariableRedirect - (UINTN)&gRtShimsDataStart)) = Enable;
  return Previous;
}
