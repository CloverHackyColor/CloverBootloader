/*++

Copyright (c) 2006, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  Support.c

Abstract:

Revision History:

--*/
#include "EfiLdr.h"
//#include "Debug.h"

EFI_STATUS
EfiAddMemoryDescriptor(
  UINTN                 *NoDesc,
  EFI_MEMORY_DESCRIPTOR *Desc,
  EFI_MEMORY_TYPE       Type,
  EFI_PHYSICAL_ADDRESS  BaseAddress,
  UINT64                NoPages,
  UINT64                Attribute
  )
{
  UINTN  NumberOfDesc;
  UINT64 Temp;
  UINTN  Index;

  if (NoPages == 0) {
    return EFI_SUCCESS;
  }

  //
  // See if the new memory descriptor needs to be carved out of an existing memory descriptor
  //

  NumberOfDesc = *NoDesc;
  for (Index = 0; Index < NumberOfDesc; Index++) {

    if (Desc[Index].Type == EfiConventionalMemory) {

      Temp = DivU64x32 ((BaseAddress - Desc[Index].PhysicalStart), EFI_PAGE_SIZE) + NoPages;

      if ((Desc[Index].PhysicalStart < BaseAddress) && (Desc[Index].NumberOfPages >= Temp)) {
        if (Desc[Index].NumberOfPages > Temp) {
          Desc[*NoDesc].Type          = EfiConventionalMemory;
          Desc[*NoDesc].PhysicalStart = BaseAddress + MultU64x32 (NoPages, EFI_PAGE_SIZE);
          Desc[*NoDesc].NumberOfPages = Desc[Index].NumberOfPages - Temp;
          Desc[*NoDesc].VirtualStart  = 0;
          Desc[*NoDesc].Attribute     = Desc[Index].Attribute;
          *NoDesc = *NoDesc + 1;
        }
        Desc[Index].NumberOfPages = Temp - NoPages;
      }

      if ((Desc[Index].PhysicalStart == BaseAddress) && (Desc[Index].NumberOfPages == NoPages)) {
        Desc[Index].Type      = Type;
        Desc[Index].Attribute = Attribute;
        return EFI_SUCCESS;
      }

      if ((Desc[Index].PhysicalStart == BaseAddress) && (Desc[Index].NumberOfPages > NoPages)) {
        Desc[Index].NumberOfPages -= NoPages;
        Desc[Index].PhysicalStart += MultU64x32 (NoPages, EFI_PAGE_SIZE);
      }
    }
  }

  //
  // Add the new memory descriptor
  //

  Desc[*NoDesc].Type          = Type;
  Desc[*NoDesc].PhysicalStart = BaseAddress;
  Desc[*NoDesc].NumberOfPages = NoPages;
  Desc[*NoDesc].VirtualStart  = 0;
  Desc[*NoDesc].Attribute     = Attribute;
  *NoDesc = *NoDesc + 1;

  return EFI_SUCCESS;
}

UINTN
FindSpace (
  UINTN                       NoPages,
  IN UINTN                    *NumberOfMemoryMapEntries,
  IN EFI_MEMORY_DESCRIPTOR    *EfiMemoryDescriptor,
  EFI_MEMORY_TYPE             Type,
  UINT64                      Attribute
  )
{
  EFI_PHYSICAL_ADDRESS        MaxPhysicalStart;
  UINT64                      MaxNoPages;
  UINTN                       Index;
  EFI_MEMORY_DESCRIPTOR       *CurrentMemoryDescriptor;

  MaxPhysicalStart = 0;
  MaxNoPages       = 0;
  CurrentMemoryDescriptor = NULL;
  for (Index = 0; Index < *NumberOfMemoryMapEntries; Index++) {
    if (EfiMemoryDescriptor[Index].PhysicalStart + LShiftU64(EfiMemoryDescriptor[Index].NumberOfPages, EFI_PAGE_SHIFT) <= 0x100000) {
      continue;
    }
    if ((EfiMemoryDescriptor[Index].Type == EfiConventionalMemory) && 
        (EfiMemoryDescriptor[Index].NumberOfPages >= NoPages)) {
      if (EfiMemoryDescriptor[Index].PhysicalStart > MaxPhysicalStart) {
        if (EfiMemoryDescriptor[Index].PhysicalStart + LShiftU64(EfiMemoryDescriptor[Index].NumberOfPages, EFI_PAGE_SHIFT) <= 0x100000000ULL) {
          MaxPhysicalStart = EfiMemoryDescriptor[Index].PhysicalStart;
          MaxNoPages       = EfiMemoryDescriptor[Index].NumberOfPages;
          CurrentMemoryDescriptor = &EfiMemoryDescriptor[Index];
        }
      }
    }
    if ((EfiMemoryDescriptor[Index].Type == EfiReservedMemoryType) ||
        (EfiMemoryDescriptor[Index].Type >= EfiACPIReclaimMemory) ) {
      continue;
    }
    if ((EfiMemoryDescriptor[Index].Type == EfiRuntimeServicesCode) ||
        (EfiMemoryDescriptor[Index].Type == EfiRuntimeServicesData)) {
      break;
    }
  }
 
  if (MaxPhysicalStart == 0) {
    return 0;
  }

  if (MaxNoPages != NoPages) {
    CurrentMemoryDescriptor->NumberOfPages = MaxNoPages - NoPages;
    EfiMemoryDescriptor[*NumberOfMemoryMapEntries].Type          = Type;
    EfiMemoryDescriptor[*NumberOfMemoryMapEntries].PhysicalStart = MaxPhysicalStart + LShiftU64(MaxNoPages - NoPages, EFI_PAGE_SHIFT);
    EfiMemoryDescriptor[*NumberOfMemoryMapEntries].NumberOfPages = NoPages;
    EfiMemoryDescriptor[*NumberOfMemoryMapEntries].VirtualStart  = 0;
    EfiMemoryDescriptor[*NumberOfMemoryMapEntries].Attribute     = Attribute;
    *NumberOfMemoryMapEntries = *NumberOfMemoryMapEntries + 1;
  } else {
    CurrentMemoryDescriptor->Type      = Type;
    CurrentMemoryDescriptor->Attribute = Attribute;
  }

  return (UINTN)(MaxPhysicalStart + LShiftU64(MaxNoPages - NoPages, EFI_PAGE_SHIFT));
}

VOID
GenMemoryMap (
  UINTN                 *NumberOfMemoryMapEntries,
  EFI_MEMORY_DESCRIPTOR *EfiMemoryDescriptor,
  BIOS_MEMORY_MAP       *BiosMemoryMap
  )
{
  UINT64                BaseAddress;
  UINT64                Length;
  EFI_MEMORY_TYPE       Type;
  UINTN                 Index, NumMap;
  UINTN                 Attr;
  UINT64                Ceiling;
  UINT64                EBDAaddr; // = 0x9E000;
  UINT64                EBDAmax = 0x100000;
  UINT64                EBDAsize = 2;

  // EBDA memory protection
  
  EBDAaddr = LShiftU64((UINT64)(*(UINT16 *)(UINTN)(0x40E)), 4);
  //fool proof
  if (EBDAaddr < 0x90000 || EBDAaddr > 0x9F800) {
    EBDAaddr = 0x9A000;
  }
  NumMap =  BiosMemoryMap->MemoryMapSize / sizeof(BIOS_MEMORY_MAP_ENTRY);
//  PrintString("Number of entries = %d\n", NumMap);
  Ceiling = 0xFFFFFFFF;
  for (Index = 0; Index < NumMap; Index++) {

    switch (BiosMemoryMap->MemoryMapEntry[Index].Type) { 
    case (INT15_E820_AddressRangeMemory):  //1 kMemoryRangeUsable
      Type = EfiConventionalMemory;
      Attr = EFI_MEMORY_WB;
      break;
    case (INT15_E820_AddressRangeReserved): //2 (Do not use)
      Type = EfiReservedMemoryType;
      Attr = EFI_MEMORY_UC;
      break;
    case (INT15_E820_AddressRangeACPI):  //3
      Type = EfiACPIReclaimMemory;
      Attr = EFI_MEMORY_WB;
      break;
    case (INT15_E820_AddressRangeNVS):  //4 (Do not use)
      Type = EfiACPIMemoryNVS;
      Attr = EFI_MEMORY_UC;
      break;
    default:
      // We should not get here, according to ACPI 2.0 Spec.
      // BIOS behaviour of the Int15h, E820h
      Type = EfiReservedMemoryType;
      Attr = EFI_MEMORY_UC;  //(Do not use)
      break;
    }
    if (Type == EfiConventionalMemory) {
      BaseAddress = BiosMemoryMap->MemoryMapEntry[Index].BaseAddress;
      Length      = BiosMemoryMap->MemoryMapEntry[Index].Length;
      if (BaseAddress & EFI_PAGE_MASK) {
        Length      = Length + (BaseAddress & EFI_PAGE_MASK) - EFI_PAGE_SIZE;
        BaseAddress = LShiftU64 (RShiftU64 (BaseAddress + EFI_PAGE_MASK, EFI_PAGE_SHIFT), EFI_PAGE_SHIFT);
      }
    } else {
      BaseAddress = BiosMemoryMap->MemoryMapEntry[Index].BaseAddress;
      Length      = BiosMemoryMap->MemoryMapEntry[Index].Length + (BaseAddress & EFI_PAGE_MASK);
      BaseAddress = LShiftU64 (RShiftU64 (BaseAddress, EFI_PAGE_SHIFT), EFI_PAGE_SHIFT);
      if (Length & EFI_PAGE_MASK) {
        Length = LShiftU64 (RShiftU64 (Length, EFI_PAGE_SHIFT) + 1, EFI_PAGE_SHIFT);
      }
      //
      // Update Memory Ceiling
      //
      //Slice - there was (BaseAddress >= 0x100000ULL) - the bred of sieve of cable 0x60000000ULL
      if ((BaseAddress >= 0x100000ULL) && (BaseAddress < 0x100000000ULL)) {
        if (Ceiling > BaseAddress) {
          Ceiling = BaseAddress;
        }
      }
      // Ignore the EBDA and bios rom area
      if (BaseAddress < EBDAaddr) {
        if ((BaseAddress + Length) >= EBDAaddr) {
          continue;
        }
      } else if (BaseAddress < EBDAmax) {
        continue;
      }
    }
    //ugly patch
/*    if (BaseAddress == 0x9b000) {
      //EBDA2 protection
      EfiAddMemoryDescriptor (
                              NumberOfMemoryMapEntries,
                              EfiMemoryDescriptor,
                              EfiACPIMemoryNVS,
                              (EFI_PHYSICAL_ADDRESS)BaseAddress,
                              RShiftU64 (Length + EFI_PAGE_MASK, EFI_PAGE_SHIFT),
                              EFI_MEMORY_UC
                              );
    } else {     */
      EfiAddMemoryDescriptor (
                              NumberOfMemoryMapEntries,
                              EfiMemoryDescriptor,
                              Type,
                              (EFI_PHYSICAL_ADDRESS)(UINTN)BaseAddress,
                              RShiftU64 (Length, EFI_PAGE_SHIFT),
                              Attr
                              );
//    }
  }
  
  //Slice - Add two more descriptors?
  /* dmazar: does not have effect, so removed */
  //Slice - or no! This is only thing that resolves memory KP in SnowLeopard
  //usr-sse2 http://www.projectosx.com/forum/index.php?showtopic=2008&view=findpost&p=13284
  //slice http://www.projectosx.com/forum/index.php?showtopic=2008&view=findpost&p=14702
  //dmazar http://www.projectosx.com/forum/index.php?showtopic=2008&view=findpost&p=16046
  //solution half a year later http://www.projectosx.com/forum/index.php?showtopic=2008&view=findpost&p=16405
  /* 
   before I am proposing 9E000 and 2 page  = 8kb. It is not common case.
  */
  //protect from the EBDA to the 1MB barrier
  EBDAsize = EBDAmax - EBDAaddr;
    
  EfiAddMemoryDescriptor (
                          NumberOfMemoryMapEntries,
                          EfiMemoryDescriptor,
                          EfiReservedMemoryType,
                          (EFI_PHYSICAL_ADDRESS)EBDAaddr,
                          RShiftU64 (EBDAsize + EFI_PAGE_MASK, EFI_PAGE_SHIFT),
                          EFI_MEMORY_UC
                          );
  //EBDA2 protection
/*  EfiAddMemoryDescriptor (
                          NumberOfMemoryMapEntries,
                          EfiMemoryDescriptor,
                          EfiACPIMemoryNVS,
                          (EFI_PHYSICAL_ADDRESS)0x9b000,
                          1,
                          EFI_MEMORY_UC
                          );
*/
  
 // this is just BIOS rom protection. Seems to be not needed.
  /*
  EfiAddMemoryDescriptor (
                          NumberOfMemoryMapEntries,
                          EfiMemoryDescriptor,
                          EfiReservedMemoryType,
                          (EFI_PHYSICAL_ADDRESS)0xE0000,
                          0x20,
                          EFI_MEMORY_UC
                          );
   // */
  
  //
  // Update MemoryMap according to Ceiling
  //
  /* dmazar: Or not?
   * We'll leave BIOS mem map untouched and add those EfiConventionalMemory
   * areas to UEFI mem map in BdsPlatformLib:UpdateMemoryMap().
   *
  for (Index = 0; Index < *NumberOfMemoryMapEntries; Index++) {
    if ((EfiMemoryDescriptor[Index].Type == EfiConventionalMemory) &&
        (EfiMemoryDescriptor[Index].PhysicalStart > 0x100000ULL) && 
        (EfiMemoryDescriptor[Index].PhysicalStart < 0x100000000ULL)) {
      if (EfiMemoryDescriptor[Index].PhysicalStart >= Ceiling){
        EfiMemoryDescriptor[Index].Type = EfiReservedMemoryType;
      }
    }
  }
   */
}
