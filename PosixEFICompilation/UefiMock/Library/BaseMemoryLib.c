//
//  BaseMemoryLib.c
//  cpp_tests UTF16 signed char
//
//  Created by Jief on 12/10/2020.
//  Copyright © 2020 Jief_Machak. All rights reserved.
//

#include <Efi.h>
#include <Library/BaseMemoryLib.h>
//#include "../../../../MdePkg/Library/BaseMemoryLib/MemLibInternals.h"
#include <memory.h>


void* SetMem(void *Destination, UINTN Length, UINT8 c)
{
  return memset(Destination, c, (size_t)Length);
}

VOID *
EFIAPI
SetMem32 (
  OUT VOID   *Buffer,
  IN UINTN   Length,
  IN UINT32  Value
  )
{
  for( uint32_t i = 0 ; i < Length/sizeof(Value) ; i++ )
  {
    ((__typeof__(&Value))Buffer)[i] = Value;
  }
  return Buffer;
}

VOID *
EFIAPI
SetMem64 (
  OUT VOID   *Buffer,
  IN UINTN   Length,
  IN UINT64  Value
  )
{
  for( uint32_t i = 0 ; i < Length/sizeof(Value) ; i++ )
  {
    ((__typeof__(&Value))Buffer)[i] = Value;
  }
  return Buffer;
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

BOOLEAN
EFIAPI
CompareGuid (
  IN CONST GUID  *Guid1,
  IN CONST GUID  *Guid2
  )
{
  UINT64  LowPartOfGuid1;
  UINT64  LowPartOfGuid2;
  UINT64  HighPartOfGuid1;
  UINT64  HighPartOfGuid2;

  LowPartOfGuid1  = * ((CONST UINT64*) Guid1);
  LowPartOfGuid2  = * ((CONST UINT64*) Guid2);
  HighPartOfGuid1 = * ((CONST UINT64*) Guid1 + 1);
  HighPartOfGuid2 = * ((CONST UINT64*) Guid2 + 1);

  return (BOOLEAN) (LowPartOfGuid1 == LowPartOfGuid2 && HighPartOfGuid1 == HighPartOfGuid2);
}

GUID *
EFIAPI
CopyGuid (
  OUT GUID       *DestinationGuid,
  IN CONST GUID  *SourceGuid
  )
{
  *DestinationGuid = *SourceGuid;
  return DestinationGuid;
}

BOOLEAN
EFIAPI
IsZeroGuid (
  IN CONST GUID  *Guid
  )
{
  UINT64  LowPartOfGuid;
  UINT64  HighPartOfGuid;

  LowPartOfGuid  = * ((CONST UINT64*) Guid);
  HighPartOfGuid = * ((CONST UINT64*) Guid + 1);

  return (BOOLEAN) (LowPartOfGuid == 0 && HighPartOfGuid == 0);
}

VOID *
EFIAPI
ScanGuid (
  IN CONST VOID  *Buffer,
  IN UINTN       Length,
  IN CONST GUID  *Guid
  )
{
  CONST GUID                        *GuidPtr;

  ASSERT (((UINTN)Buffer & (sizeof (Guid->Data1) - 1)) == 0);
  ASSERT (Length <= (MAX_ADDRESS - (UINTN)Buffer + 1));
  ASSERT ((Length & (sizeof (*GuidPtr) - 1)) == 0);

  GuidPtr = (GUID*)Buffer;
  Buffer  = GuidPtr + Length / sizeof (*GuidPtr);
  while (GuidPtr < (CONST GUID*)Buffer) {
    if (CompareGuid (GuidPtr, Guid)) {
      return (VOID*)GuidPtr;
    }
    GuidPtr++;
  }
  return NULL;
}

CONST VOID *
EFIAPI
InternalMemScanMem16 (
  IN      CONST VOID                *Buffer,
  IN      UINTN                     Length,
  IN      UINT16                    Value
  )
{
  CONST UINT16                      *Pointer;

  Pointer = (CONST UINT16*)Buffer;
  do {
    if (*Pointer == Value) {
      return Pointer;
    }
    ++Pointer;
  } while (--Length != 0);
  return NULL;
}

VOID *
EFIAPI
ScanMem16 (
  IN CONST VOID  *Buffer,
  IN UINTN       Length,
  IN UINT16      Value
  )
{
  if (Length == 0) {
    return NULL;
  }

  ASSERT (Buffer != NULL);
  ASSERT (((UINTN)Buffer & (sizeof (Value) - 1)) == 0);
  ASSERT ((Length - 1) <= (MAX_ADDRESS - (UINTN)Buffer));
  ASSERT ((Length & (sizeof (Value) - 1)) == 0);

  return (VOID*)InternalMemScanMem16 (Buffer, Length / sizeof (Value), Value);
}
