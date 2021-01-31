//
//  MemoryAllocationLib.c
//  cpp_tests UTF16 signed char
//
//  Created by Jief on 30/01/2021.
//  Copyright © 2021 JF Knudsen. All rights reserved.
//

#include "MemoryAllocationLib.h"


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

void* ReallocatePool(UINTN  OldSize, UINTN  NewSize, void* OldBuffer)
{
  (void)OldSize;
  if ( !OldBuffer ) return AllocatePool(NewSize);
  return (void*)realloc(OldBuffer, (size_t)NewSize);
}

void FreePool(IN VOID   *Buffer)
{
  free((void*)Buffer);
}
