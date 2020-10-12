//
//  BaseMemoryLib.c
//  cpp_tests UTF16 signed char
//
//  Created by Jief on 12/10/2020.
//  Copyright © 2020 JF Knudsen. All rights reserved.
//

#include "BaseMemoryLib.h"

#include <memory.h>


void* SetMem(void *Destination, UINTN Length, UINT8 c)
{
  return memset(Destination, c, (size_t)Length);
}

INTN CompareMem(const void* DestinationBuffer, const void* SourceBuffer, UINTN Length)
{
  return memcmp(SourceBuffer, DestinationBuffer, Length);
}

void* CopyMem(void *Destination, const void *Source, UINTN Length)
{
  return memmove(Destination, Source, (size_t)Length);
}

void* ZeroMem(void *Destination, UINTN Length)
{
  return memset(Destination, 0, (size_t)Length);
}
