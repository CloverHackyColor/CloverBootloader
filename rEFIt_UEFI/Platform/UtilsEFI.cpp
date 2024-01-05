///*++
//
//All rights reserved. This program and the accompanying materials
//are licensed and made available under the terms and conditions of the BSD License
//which accompanies this distribution. The full text of the license may be found at
//http://opensource.org/licenses/bsd-license.php
//
//THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
//WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
//
//
#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include <Efi.h>

extern "C" {
#include <Library/OcMemoryLib.h>
}

void displayFreeMemory(const XString8& prefix)
{
  UINTN                LowMemory;
  UINTN                TotalMemory;

  TotalMemory = OcCountFreePages (&LowMemory);
  DebugLog(1, "--> %s: Firmware has %llu free pages (%llu in lower 4 GB)\n", prefix.c_str(), TotalMemory, LowMemory);
}
