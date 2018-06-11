/** @file
  An application reproducing local variable corruption in recursive calls with
  gcc-4.8 on the X64 target.

  Copyright (C) 2014, Red Hat, Inc.
  Copyright (c) 2010 - 2011, Intel Corporation. All rights reserved.<BR>

  This program and the accompanying materials are licensed and made available
  under the terms and conditions of the BSD License which accompanies this
  distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, WITHOUT
  WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/
#include  <Uefi.h>
#include  <Library/UefiLib.h>
#include  <Library/ShellCEntryLib.h>

VOID
EFIAPI
PrintSeveralQuadruplets (
  IN UINT64  Dummy1,
  IN UINT64  Dummy2,
  IN UINT64  Dummy3,
  IN BOOLEAN Recursive,
  ...
  );

VOID
EFIAPI
Print4 (
  IN UINT64  Dummy1,
  IN UINT64  Dummy2,
  IN UINT64  Dummy3,
  IN BOOLEAN Recursive,
  IN VA_LIST Marker
  )
{
  UINT64 Value1, Value2, Value3, Value4;

  do {
    Value1 = VA_ARG (Marker, UINT64);
    Value2 = VA_ARG (Marker, UINT64);
    Value3 = VA_ARG (Marker, UINT64);
    Value4 = VA_ARG (Marker, UINT64);

    if (!Recursive) {
      AsciiPrint ("0x%02Lx 0x%02Lx 0x%02Lx 0x%02Lx\n",
        Value1, Value2, Value3, Value4);
      return;
    }
    PrintSeveralQuadruplets (Dummy1, Dummy2, Dummy3, FALSE,
      Value1, Value2, Value3, Value4);
  } while (Value4 != 0);
}

VOID
EFIAPI
PrintSeveralQuadruplets (
  IN UINT64  Dummy1,
  IN UINT64  Dummy2,
  IN UINT64  Dummy3,
  IN BOOLEAN Recursive,
  ...
  )
{
  VA_LIST Marker;

  VA_START (Marker, Recursive);
  Print4 (Dummy1 + 1, Dummy2 + 2, Dummy3 + 3, Recursive, Marker);
  VA_END (Marker);
}

INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  PrintSeveralQuadruplets (0, 0, 0, TRUE,
     0LLU,  1LLU,  2LLU,  3LLU,
     4LLU,  5LLU,  6LLU,  7LLU,
     8LLU,  9LLU, 10LLU, 11LLU,
    12LLU, 13LLU, 14LLU, 15LLU,
    16LLU, 17LLU, 18LLU, 19LLU,
    20LLU, 21LLU, 22LLU, 23LLU,
    24LLU, 25LLU, 26LLU, 27LLU,
    28LLU, 29LLU, 30LLU,  0LLU
    );
  return 0;
}
