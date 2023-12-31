/** @file
  Implementation of MemoryAllocationLibEx based on PhaseMemoryAllocationLib.

  Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
  Copyright (c) 2023, Marvin Häuser. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiDxe.h>

#include <Library/MemoryAllocationLibEx.h>
#include <Library/PhaseMemoryAllocationLib.h>
#include <Library/DebugLib.h>

/**
  Allocates one or more 4KB pages of a certain memory type.

  Allocates the number of 4KB pages of a certain memory type and returns a pointer
  to the allocated buffer.  The buffer returned is aligned on a 4KB boundary.

  @param  Type                  The type of allocation to perform.
  @param  MemoryType            The type of memory to allocate.
  @param  Pages                 The number of 4 KB pages to allocate.
  @param  Memory                The pointer to a physical address. On input, the
                                way in which the address is used depends on the
                                value of Type.

  @retval EFI_SUCCESS           The requested pages were allocated.
  @retval EFI_OUT_OF_RESOURCES  The pages could not be allocated.
  @retval EFI_NOT_FOUND         The requested pages could not be found.

**/
EFI_STATUS
EFIAPI
AllocatePagesEx (
  IN     EFI_ALLOCATE_TYPE     Type,
  IN     EFI_MEMORY_TYPE       MemoryType,
  IN     UINTN                 Pages,
  IN OUT EFI_PHYSICAL_ADDRESS  *Memory
  )
{
  return PhaseAllocatePages (Type, MemoryType, Pages, Memory);
}

/**
  Allocates one or more 4KB pages of a certain memory type at a specified alignment.

  Allocates the number of 4KB pages specified by Pages of a certain memory type with an alignment
  specified by Alignment.
  If Alignment is not a power of two and Alignment is not zero, then ASSERT().
  If Pages plus EFI_SIZE_TO_PAGES (Alignment) overflows, then ASSERT().

  @param  Type                  The type of allocation to perform.
  @param  MemoryType            The type of memory to allocate.
  @param  Pages                 The number of 4 KB pages to allocate.
  @param  Alignment             The requested alignment of the allocation.  Must be a power of two.
                                If Alignment is zero, then byte alignment is used.
  @param  Memory                The pointer to a physical address. On input, the
                                way in which the address is used depends on the
                                value of Type.

  @retval EFI_SUCCESS           The requested pages were allocated.
  @retval EFI_OUT_OF_RESOURCES  The pages could not be allocated.
  @retval EFI_NOT_FOUND         The requested pages could not be found.

**/
EFI_STATUS
EFIAPI
AllocateAlignedPagesEx (
  IN     EFI_ALLOCATE_TYPE     Type,
  IN     EFI_MEMORY_TYPE       MemoryType,
  IN     UINTN                 Pages,
  IN     UINTN                 Alignment,
  IN OUT EFI_PHYSICAL_ADDRESS  *Memory
  )
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  Address;
  EFI_PHYSICAL_ADDRESS  AlignedMemory;
  UINTN                 AlignmentMask;
  UINTN                 UnalignedPages;
  UINTN                 RealPages;

  //
  // Alignment must be a power of two or zero.
  //
  ASSERT ((Alignment & (Alignment - 1)) == 0);

  //
  // Reserving a specific address does not require guaranteeing alignment
  // constraints.
  //
  ASSERT (Type != AllocateAddress);

  if (Pages == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if (Alignment > EFI_PAGE_SIZE) {
    //
    // Calculate the total number of pages since alignment is larger than page size.
    //
    AlignmentMask = Alignment - 1;
    RealPages     = Pages + EFI_SIZE_TO_PAGES (Alignment);
    //
    // Make sure that Pages plus EFI_SIZE_TO_PAGES (Alignment) does not overflow.
    //
    ASSERT (RealPages > Pages);

    Status = AllocatePagesEx (Type, MemoryType, RealPages, Memory);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Address = *Memory;

    AlignedMemory  = (Address + AlignmentMask) & ~(EFI_PHYSICAL_ADDRESS)AlignmentMask;
    UnalignedPages = EFI_SIZE_TO_PAGES ((UINTN)(AlignedMemory - Address));
    if (UnalignedPages > 0) {
      //
      // Free first unaligned page(s).
      //
      Status = PhaseFreePages (Address, UnalignedPages);
      ASSERT_EFI_ERROR (Status);
    }

    Address        = AlignedMemory + EFI_PAGES_TO_SIZE (Pages);
    UnalignedPages = RealPages - Pages - UnalignedPages;
    if (UnalignedPages > 0) {
      //
      // Free last unaligned page(s).
      //
      Status = PhaseFreePages (Address, UnalignedPages);
      ASSERT_EFI_ERROR (Status);
    }

    *Memory = AlignedMemory;
  } else {
    //
    // Do not over-allocate pages in this case.
    //
    Status = AllocatePagesEx (Type, MemoryType, Pages, Memory);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return EFI_SUCCESS;
}
