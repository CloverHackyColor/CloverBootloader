/**

  MemoryMap helper functions.

  by dmazar

**/

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/OcMiscLib.h>
#include <Library/PrintLib.h>
#include <Library/DevicePathLib.h>

#include "Config.h"
#include "MemoryMap.h"
#include "CustomSlide.h"
#include "ServiceOverrides.h"

STATIC CHAR16 *mEfiMemoryTypeDesc[EfiMaxMemoryType] = {
  L"Reserved",
  L"LDR_code",
  L"LDR_data",
  L"BS_code",
  L"BS_data",
  L"RT_code",
  L"RT_data",
  L"Available",
  L"Unusable",
  L"ACPI_recl",
  L"ACPI_NVS",
  L"MemMapIO",
  L"MemPortIO",
  L"PAL_code"
};

VOID
ShrinkMemMap (
  IN OUT UINTN                  *MemoryMapSize,
  IN OUT EFI_MEMORY_DESCRIPTOR  *MemoryMap,
  IN     UINTN                  DescriptorSize
  )
{
  UINTN                   SizeFromDescToEnd;
  UINT64                  Bytes;
  EFI_MEMORY_DESCRIPTOR   *PrevDesc;
  EFI_MEMORY_DESCRIPTOR   *Desc;
  BOOLEAN                 CanBeJoined;
  BOOLEAN                 HasEntriesToRemove;

  PrevDesc           = MemoryMap;
  Desc               = NEXT_MEMORY_DESCRIPTOR (PrevDesc, DescriptorSize);
  SizeFromDescToEnd  = *MemoryMapSize - DescriptorSize;
  *MemoryMapSize     = DescriptorSize;
  HasEntriesToRemove = FALSE;

  while (SizeFromDescToEnd > 0) {
    Bytes = EFI_PAGES_TO_SIZE (PrevDesc->NumberOfPages);
    CanBeJoined = FALSE;
    if (Desc->Attribute == PrevDesc->Attribute && PrevDesc->PhysicalStart + Bytes == Desc->PhysicalStart) {
      //
      // It *should* be safe to join this with conventional memory, because the firmware should not use
      // GetMemoryMap for allocation, and for the kernel it does not matter, since it joins them.
      //
      CanBeJoined = (Desc->Type == EfiBootServicesCode ||
        Desc->Type == EfiBootServicesData ||
        Desc->Type == EfiConventionalMemory ||
        Desc->Type == EfiLoaderCode ||
        Desc->Type == EfiLoaderData) && (
        PrevDesc->Type == EfiBootServicesCode ||
        PrevDesc->Type == EfiBootServicesData ||
        PrevDesc->Type == EfiConventionalMemory ||
        PrevDesc->Type == EfiLoaderCode ||
        PrevDesc->Type == EfiLoaderData);
    }

    if (CanBeJoined) {
      //
      // Two entries are the same/similar - join them
      //
      PrevDesc->Type = EfiConventionalMemory;
      PrevDesc->NumberOfPages += Desc->NumberOfPages;
      HasEntriesToRemove = TRUE;
    } else {
      //
      // Cannot be joined - we need to move to next
      //
      *MemoryMapSize += DescriptorSize;
      PrevDesc = NEXT_MEMORY_DESCRIPTOR (PrevDesc, DescriptorSize);
      if (HasEntriesToRemove) {
        //
        // Have entries between PrevDesc and Desc which are joined to PrevDesc,
        // we need to copy [Desc, end of list] to PrevDesc + 1
        //
        CopyMem(PrevDesc, Desc, SizeFromDescToEnd);
        Desc = PrevDesc;
        HasEntriesToRemove = FALSE;
      }
    }

    Desc = NEXT_MEMORY_DESCRIPTOR (Desc, DescriptorSize);
    SizeFromDescToEnd -= DescriptorSize;
  }
}

/** AMI CSM module allocates up to two regions for legacy video output.
 *  1. For PMM and EBDA areas.
 *     On Ivy Bridge and below it ends at 0xA0000-0x1000-0x1 and has EfiBootServicesCode type.
 *     On Haswell and above it is allocated below 0xA0000 address with the same type.
 *  2. For Intel RC S3 reserved area, fixed from 0x9F000 to 0x9FFFF.
 *     On Sandy Bridge and below it is not present in memory map.
 *     On Ivy Bridge and newer it is present as EfiRuntimeServicesData.
 *     Starting from at least SkyLake it is present as EfiReservedMemoryType.
 *
 *  Prior to AptioMemoryFix EfiRuntimeServicesData could have been relocated by boot.efi,
 *  and the 2nd region could have been overwritten by the kernel. Now it is no longer the
 *  case, and only the 1st region may need special handling.
 *
 *  For the 1st region there appear to be (unconfirmed) reports that it may still be accessed
 *  after waking from sleep. This does not seem to be valid according to AMI code, but we still
 *  protect it in case such systems really exist.
 *
 *  Researched and fixed on gigabyte boards by Slice
 */
VOID
ProtectCsmRegion (
  UINTN                  MemoryMapSize,
  EFI_MEMORY_DESCRIPTOR  *MemoryMap,
  UINTN                  DescriptorSize
  )
{
  UINTN                   NumEntries;
  UINTN                   Index;
  EFI_MEMORY_DESCRIPTOR   *Desc;
  UINTN                   BlockSize;
  UINTN                   PhysicalEnd;

  Desc = MemoryMap;
  NumEntries = MemoryMapSize / DescriptorSize;

  for (Index = 0; Index < NumEntries; Index++) {
    BlockSize = EFI_PAGES_TO_SIZE ((UINTN)Desc->NumberOfPages);
    PhysicalEnd = Desc->PhysicalStart + BlockSize;

    if (PhysicalEnd >= 0x9E000 && PhysicalEnd < 0xA0000 && Desc->Type == EfiBootServicesData) {
      Desc->Type = EfiACPIMemoryNVS;
      break;
    }

    Desc = NEXT_MEMORY_DESCRIPTOR(Desc, DescriptorSize);
  }
}

VOID
PrintMemMap (
  IN CONST CHAR16           *Name,
  IN UINTN                  MemoryMapSize,
  IN UINTN                  DescriptorSize,
  IN EFI_MEMORY_DESCRIPTOR  *MemoryMap,
  IN VOID                   *Shims,
  IN EFI_PHYSICAL_ADDRESS   SysTable
  )
{
  UINTN                   NumEntries;
  UINTN                   Index;
  UINT64                  Bytes;
  EFI_MEMORY_DESCRIPTOR   *Desc;

  //
  // Printing onscreen may allocate the memory internally.
  // This is very dangerous to do in GetMemoryMap or SetVirtualAddresses wrappers,
  // because on many ASUS boards the internal memory map will get modified, and
  // for some reason this will cause crashes right after os boots.
  //
  DisableDynamicPoolAllocations ();

  Desc = MemoryMap;
  NumEntries = MemoryMapSize / DescriptorSize;
  Print (L"--- Dump Memory Map (%s) start ---\n", Name);
  Print (L"MEMMAP: Size=%d, Addr=%p, DescSize=%d, Shims=%08lX, ST=%08lX\n",
    MemoryMapSize, MemoryMap, DescriptorSize, (UINTN)Shims, (UINTN)SysTable);
  Print (L"Type      Start      End        Virtual          # Pages    Attributes\n");
  for (Index = 0; Index < NumEntries; Index++) {

    Bytes = EFI_PAGES_TO_SIZE (Desc->NumberOfPages);

    Print (L"%-9s %010lX %010lX %016lX %010lX %016lX\n",
      mEfiMemoryTypeDesc[Desc->Type],
      Desc->PhysicalStart,
      Desc->PhysicalStart + Bytes - 1,
      Desc->VirtualStart,
      Desc->NumberOfPages,
      Desc->Attribute
    );
    Desc = NEXT_MEMORY_DESCRIPTOR (Desc, DescriptorSize);
    //
    // It is often the case that memory map does not fit onscreen.
    // There is no way to reliably determine the largest console window, so we just stall
    // for a moment to let one read the output.
    //
    if ((Index + 1) % 16 == 0)
      gBS->Stall (SECONDS_TO_MICROSECONDS (5));
  }

  Print (L"--- Dump Memory Map (%s) end ---\n", Name);
  gBS->Stall (SECONDS_TO_MICROSECONDS (5));

  EnableDynamicPoolAllocations ();
}

EFI_STATUS
GetMemoryMapAlloc (
  IN OUT UINTN                  *AllocatedTopPages,
     OUT UINTN                  *MemoryMapSize,
     OUT EFI_MEMORY_DESCRIPTOR  **MemoryMap,
     OUT UINTN                  *MapKey,
     OUT UINTN                  *DescriptorSize,
     OUT UINT32                 *DescriptorVersion
  )
{
  EFI_STATUS               Status;

  *MemoryMapSize       = 0;
  *MemoryMap           = NULL;
  Status = OrgGetMemoryMap (
    MemoryMapSize,
    *MemoryMap,
    MapKey,
    DescriptorSize,
    DescriptorVersion
    );

  if (Status != EFI_BUFFER_TOO_SMALL) {
    DEBUG ((DEBUG_INFO, "Insane GetMemoryMap %r\n", Status));
    return Status;
  }

  do {
    //
    // This is done because extra allocations may increase memory map size.
    //
    *MemoryMapSize   += 512;

    //
    // Requested to allocate from top via pages.
    // This may be needed, because the pool memory may collide with the kernel.
    //
    if (AllocatedTopPages) {
      *MemoryMap         = (EFI_MEMORY_DESCRIPTOR *)BASE_4GB;
      *AllocatedTopPages = EFI_SIZE_TO_PAGES (*MemoryMapSize);
      Status = AllocatePagesFromTop (
        EfiBootServicesData,
        *AllocatedTopPages,
        (EFI_PHYSICAL_ADDRESS *)MemoryMap,
        FALSE
        );
      if (EFI_ERROR(Status)) {
        DEBUG ((DEBUG_INFO, "Temp memory map allocation from top failure %r\n", Status));
        *MemoryMap = NULL;
        return Status;
      }
    } else {
      *MemoryMap = AllocatePool (*MemoryMapSize);
      if (!*MemoryMap) {
        DEBUG ((DEBUG_INFO, "Temp memory map direct allocation failure\n"));
        return EFI_OUT_OF_RESOURCES;
      }
    }

    Status = OrgGetMemoryMap (
      MemoryMapSize,
      *MemoryMap,
      MapKey,
      DescriptorSize,
      DescriptorVersion
      );

    if (EFI_ERROR(Status)) {
      if (AllocatedTopPages) {
        gBS->FreePages ((EFI_PHYSICAL_ADDRESS)*MemoryMap, *AllocatedTopPages);
      } else {
        FreePool(*MemoryMap);
      }
      *MemoryMap = NULL;
    }
  } while (Status == EFI_BUFFER_TOO_SMALL);

  if (Status != EFI_SUCCESS) {
    DEBUG ((DEBUG_INFO, "Failed to obtain memory map %r\n", Status));
  }

  return Status;
}

EFI_STATUS
AllocatePagesFromTop (
  IN     EFI_MEMORY_TYPE       MemoryType,
  IN     UINTN                 Pages,
  IN OUT EFI_PHYSICAL_ADDRESS  *Memory,
  IN     BOOLEAN               CheckRange
  )
{
  EFI_STATUS              Status;
  UINTN                   MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR   *MemoryMap;
  UINTN                   MapKey;
  UINTN                   DescriptorSize;
  UINT32                  DescriptorVersion;
  EFI_MEMORY_DESCRIPTOR   *MemoryMapEnd;
  EFI_MEMORY_DESCRIPTOR   *Desc;

  Status = GetMemoryMapAlloc (NULL, &MemoryMapSize, &MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = EFI_NOT_FOUND;

  MemoryMapEnd = NEXT_MEMORY_DESCRIPTOR (MemoryMap, MemoryMapSize);
  Desc = PREV_MEMORY_DESCRIPTOR (MemoryMapEnd, DescriptorSize);

  for ( ; Desc >= MemoryMap; Desc = PREV_MEMORY_DESCRIPTOR (Desc, DescriptorSize)) {
    //
    // We are looking for some free memory descriptor that contains enough space below the specified memory
    // 
    if (Desc->Type == EfiConventionalMemory && Pages <= Desc->NumberOfPages &&
      Desc->PhysicalStart + EFI_PAGES_TO_SIZE (Pages) <= *Memory) {
      //
      // Free block found
      //
      if (Desc->PhysicalStart + EFI_PAGES_TO_SIZE ((UINTN)Desc->NumberOfPages) <= *Memory) {
        //
        // The whole block is under Memory - allocate from the top of the block
        //
        *Memory = Desc->PhysicalStart + EFI_PAGES_TO_SIZE ((UINTN)Desc->NumberOfPages - Pages);
      } else {
        //
        // The block contains enough pages under Memory, but spans above it - allocate below Memory
        //
        *Memory = *Memory - EFI_PAGES_TO_SIZE (Pages);
      }
      //
      // Ensure that the found block does not overlap with the kernel area
      //
      if (CheckRange && OverlapsWithSlide (*Memory, EFI_PAGES_TO_SIZE (Pages))) {
        continue;
      }

      Status = gBS->AllocatePages (
        AllocateAddress,
        MemoryType,
        Pages,
        Memory
        );
      break;
    }
  }

  FreePool(MemoryMap);

  return Status;
}
