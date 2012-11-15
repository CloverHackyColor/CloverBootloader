/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

#include "915resolution.h"
//#include "gui.h"

EFI_STATUS
EFIAPI
Resolution_start (
  IN EFI_HANDLE     ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  //UINT32 bp = 0;
  //UINT32 x, y;
  AsciiPrint("Video BIOS patcher\n");
  patchVideoBios();
  //getResolution(&x, &y, &bp);
  return EFI_SUCCESS;
}

