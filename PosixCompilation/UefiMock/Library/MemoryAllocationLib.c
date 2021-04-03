//
//  MemoryAllocationLib.c
//  cpp_tests UTF16 signed char
//
//  Created by Jief on 30/01/2021.
//  Copyright © 2021 Jief_Machak. All rights reserved.
//

#include <Library/MemoryAllocationLib.h>


void* AllocatePool(UINTN  AllocationSize)
{
  return (void*)malloc((size_t)AllocationSize);
}

void* AllocateZeroPool(UINTN  AllocationSize)
{
  void* p = (void*)malloc((size_t)AllocationSize);
  memset(p, 0, (size_t)AllocationSize);
  return p;
}

void* AllocateCopyPool (UINTN AllocationSize, CONST VOID  *Buffer)
{
  void* p = malloc(AllocationSize);
  memcpy(p, Buffer, AllocationSize);
  return p;
}

void* ReallocatePool(UINTN  OldSize, UINTN  NewSize, void* OldBuffer)
{
  (void)OldSize;
  if ( !OldBuffer ) return AllocatePool(NewSize);
  return (void*)realloc(OldBuffer, (size_t)NewSize);
}

void FreePool(IN JCONST VOID   *Buffer)
{
  free((void*)Buffer);
}

//#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

VOID *
EFIAPI
AllocateAlignedPages (
  IN UINTN  Pages,
  IN UINTN  Alignment
  )
{
  panic("not yet");
}


VOID
EFIAPI
FreeAlignedPages (
  IN VOID   *Buffer,
  IN UINTN  Pages
  )
{
  panic("not yet");
}

VOID *
EFIAPI
AllocatePages (
  IN UINTN  Pages
  )
{
  panic("not yet");
}

VOID
EFIAPI
FreePages (
  IN VOID   *Buffer,
  IN UINTN  Pages
  )
{
  panic("not yet");
}
