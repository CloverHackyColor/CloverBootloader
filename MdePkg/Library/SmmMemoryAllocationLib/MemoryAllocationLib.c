/** @file
  Support routines for memory allocation routines based
  on SMM Services Table services for SMM phase drivers.

  The PI System Management Mode Core Interface Specification only allows the use
  of EfiRuntimeServicesCode and EfiRuntimeServicesData memory types for memory
  allocations through the SMM Services Table as the SMRAM space should be
  reserved after BDS phase.  The functions in the Memory Allocation Library use
  EfiBootServicesData as the default memory allocation type.  For this SMM
  specific instance of the Memory Allocation Library, EfiRuntimeServicesData
  is used as the default memory type for all allocations. In addition,
  allocation for the Reserved memory types are not supported and will always
  return NULL.

  Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiSmm.h>

#include <Protocol/SmmAccess2.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PhaseMemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

GLOBAL_REMOVE_IF_UNREFERENCED CONST EFI_MEMORY_TYPE gPhaseDefaultDataType = EfiRuntimeServicesData;
GLOBAL_REMOVE_IF_UNREFERENCED CONST EFI_MEMORY_TYPE gPhaseDefaultCodeType = EfiRuntimeServicesCode;

EFI_SMRAM_DESCRIPTOR  *mSmramRanges;
UINTN                 mSmramRangeCount;

/**
  The constructor function caches SMRAM ranges that are present in the system.

  It will ASSERT() if SMM Access2 Protocol doesn't exist.
  It will ASSERT() if SMRAM ranges can't be got.
  It will ASSERT() if Resource can't be allocated for cache SMRAM range.
  It will always return EFI_SUCCESS.

  @param  ImageHandle   The firmware allocated handle for the EFI image.
  @param  SystemTable   A pointer to the EFI System Table.

  @retval EFI_SUCCESS   The constructor always returns EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
SmmMemoryAllocationLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                Status;
  EFI_SMM_ACCESS2_PROTOCOL  *SmmAccess;
  UINTN                     Size;

  //
  // Locate SMM Access2 Protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiSmmAccess2ProtocolGuid,
                  NULL,
                  (VOID **)&SmmAccess
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Get SMRAM range information
  //
  Size   = 0;
  Status = SmmAccess->GetCapabilities (SmmAccess, &Size, NULL);
  ASSERT (Status == EFI_BUFFER_TOO_SMALL);

  mSmramRanges = (EFI_SMRAM_DESCRIPTOR *)AllocatePool (Size);
  ASSERT (mSmramRanges != NULL);

  Status = SmmAccess->GetCapabilities (SmmAccess, &Size, mSmramRanges);
  ASSERT_EFI_ERROR (Status);

  mSmramRangeCount = Size / sizeof (EFI_SMRAM_DESCRIPTOR);

  return EFI_SUCCESS;
}

/**
  If SMM driver exits with an error, it must call this routine
  to free the allocated resource before the exiting.

  @param[in]  ImageHandle   The firmware allocated handle for the EFI image.
  @param[in]  SystemTable   A pointer to the EFI System Table.

  @retval     EFI_SUCCESS   The deconstructor always returns EFI_SUCCESS.
**/
EFI_STATUS
EFIAPI
SmmMemoryAllocationLibDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  FreePool (mSmramRanges);

  return EFI_SUCCESS;
}

/**
  Check whether the start address of buffer is within any of the SMRAM ranges.

  @param[in]  Buffer   The pointer to the buffer to be checked.

  @retval     TRUE     The buffer is in SMRAM ranges.
  @retval     FALSE    The buffer is out of SMRAM ranges.
**/
BOOLEAN
EFIAPI
BufferInSmram (
  IN EFI_PHYSICAL_ADDRESS  Buffer
  )
{
  UINTN  Index;

  for (Index = 0; Index < mSmramRangeCount; Index++) {
    if ((Buffer >= mSmramRanges[Index].CpuStart) &&
        (Buffer < (mSmramRanges[Index].CpuStart + mSmramRanges[Index].PhysicalSize)))
    {
      return TRUE;
    }
  }

  return FALSE;
}

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
PhaseAllocatePages (
  IN     EFI_ALLOCATE_TYPE     Type,
  IN     EFI_MEMORY_TYPE       MemoryType,
  IN     UINTN                 Pages,
  IN OUT EFI_PHYSICAL_ADDRESS  *Memory
  )
{
  if (Pages == 0) {
    return EFI_INVALID_PARAMETER;
  }

  return gSmst->SmmAllocatePages (Type, MemoryType, Pages, Memory);
}

/**
  Frees one or more 4KB pages that were previously allocated with one of the page allocation
  functions in the Memory Allocation Library.

  Frees the number of 4KB pages specified by Pages from the buffer specified by Buffer.
  Buffer must have been allocated on a previous call to the page allocation services
  of the Memory Allocation Library.  If it is not possible to free allocated pages,
  then this function will perform no actions.

  If Buffer was not allocated with a page allocation function in the Memory Allocation
  Library, then ASSERT().
  If Pages is zero, then ASSERT().

  @param  Memory                The base physical address of the pages to be freed.
  @param  Pages                 The number of 4 KB pages to free.

  @retval EFI_SUCCESS           The requested pages were freed.
  @retval EFI_NOT_FOUND         The requested memory pages were not allocated with
                                PhaseAllocatePages().

**/
EFI_STATUS
EFIAPI
PhaseFreePages (
  IN EFI_PHYSICAL_ADDRESS  Memory,
  IN UINTN                 Pages
  )
{
  EFI_STATUS  Status;

  ASSERT (Pages != 0);
  if (BufferInSmram (Memory)) {
    //
    // When Buffer is in SMRAM range, it should be allocated by gSmst->SmmAllocatePages() service.
    // So, gSmst->SmmFreePages() service is used to free it.
    //
    Status = gSmst->SmmFreePages (Memory, Pages);
  } else {
    //
    // When Buffer is out of SMRAM range, it should be allocated by gBS->AllocatePages() service.
    // So, gBS->FreePages() service is used to free it.
    //
    Status = gBS->FreePages (Memory, Pages);
  }

  return Status;
}

/**
  Allocates a buffer of a certain pool type.

  Allocates the number bytes specified by AllocationSize of a certain pool type
  and returns a pointer to the allocated buffer.  If AllocationSize is 0, then a
  valid buffer of 0 size is returned.  If there is not enough memory remaining to
  satisfy the request, then NULL is returned.

  @param  MemoryType            The type of memory to allocate.
  @param  AllocationSize        The number of bytes to allocate.

  @return A pointer to the allocated buffer or NULL if allocation fails.

**/
VOID *
EFIAPI
PhaseAllocatePool (
  IN EFI_MEMORY_TYPE  MemoryType,
  IN UINTN            AllocationSize
  )
{
  EFI_STATUS  Status;
  VOID        *Memory;

  Status = gSmst->SmmAllocatePool (MemoryType, AllocationSize, &Memory);
  if (EFI_ERROR (Status)) {
    Memory = NULL;
  }

  return Memory;
}

/**
  Frees a buffer that was previously allocated with one of the pool allocation
  functions in the Memory Allocation Library.

  Frees the buffer specified by Buffer.  Buffer must have been allocated on a
  previous call to the pool allocation services of the Memory Allocation Library.
  If it is not possible to free pool resources, then this function will perform
  no actions.

  If Buffer was not allocated with a pool allocation function in the Memory
  Allocation Library, then ASSERT().

  @param  Buffer                The pointer to the buffer to free.

**/
VOID
EFIAPI
PhaseFreePool (
  IN VOID  *Buffer
  )
{
  EFI_STATUS  Status;

  if (BufferInSmram ((UINTN)Buffer)) {
    //
    // When Buffer is in SMRAM range, it should be allocated by gSmst->SmmAllocatePool() service.
    // So, gSmst->SmmFreePool() service is used to free it.
    //
    Status = gSmst->SmmFreePool (Buffer);
  } else {
    //
    // When Buffer is out of SMRAM range, it should be allocated by gBS->AllocatePool() service.
    // So, gBS->FreePool() service is used to free it.
    //
    Status = gBS->FreePool (Buffer);
  }

  ASSERT_EFI_ERROR (Status);
}
