/*++

Copyright (c) 2005, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the Software
License Agreement which accompanies this distribution.


Module Name:

  debug.c

Abstract:

  Debug functions for fat driver

Revision History

--*/

#include "Fat.h"

VOID
FatDumpFatTable (
  IN FAT_VOLUME   *Volume
  )
/*++

Routine Description:

  Dump all the FAT Entry of the FAT table in the volume

Arguments:

  Volume - The volume whose FAT info will be dumped

Returns:

  None

--*/
{
  UINTN   EntryValue;
  UINTN   MaxIndex;
  UINTN   Index;
  CHAR16  *Pointer;

  MaxIndex = Volume->MaxCluster + 2;

  Print (L"Dump of Fat Table, MaxCluster %x\n", MaxIndex);
  for (Index = 0; Index < MaxIndex; Index++) {
    EntryValue = FatGetFatEntry (Volume, Index);
    if (EntryValue != FAT_CLUSTER_FREE) {
      Pointer = NULL;
      switch (EntryValue) {
      case FAT_CLUSTER_RESERVED:
        Pointer = L"RESERVED";
        break;

      case FAT_CLUSTER_BAD:
        Pointer = L"BAD";
        break;
      }

      if (FAT_END_OF_FAT_CHAIN (EntryValue)) {
        Pointer = L"LAST";
      }

      if (Pointer != NULL) {
        Print (L"Entry %x = %s\n", Index, Pointer);
      } else {
        Print (L"Entry %x = %x\n", Index, EntryValue);
      }
    }
  }
}
