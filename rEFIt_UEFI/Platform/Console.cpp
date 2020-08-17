/* $Id: Console.c $ */
/** @file
 * Console.c - VirtualBox Console control emulation
 */

/*
 * Copyright (C) 2010 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */

/*
  Removed commented out code in rev 2965
*/

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile

STATIC EFI_CONSOLE_CONTROL_SCREEN_MODE CurrentMode = EfiConsoleControlScreenText;

EFI_STATUS
EFIAPI
GetModeImpl (
  IN      EFI_CONSOLE_CONTROL_PROTOCOL    *This,
     OUT  EFI_CONSOLE_CONTROL_SCREEN_MODE *Mode,
     OUT  BOOLEAN                         *GopUgaExists,  OPTIONAL
     OUT  BOOLEAN                         *StdInLocked    OPTIONAL
  )
{
  *Mode = CurrentMode;

  if (GopUgaExists) {
    *GopUgaExists = TRUE;
  }

  if (StdInLocked) {
    *StdInLocked  = FALSE;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SetModeImpl (
  IN  EFI_CONSOLE_CONTROL_PROTOCOL    *This,
  IN  EFI_CONSOLE_CONTROL_SCREEN_MODE Mode
  )
{
  CurrentMode = Mode;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
LockStdInImpl (
  IN  EFI_CONSOLE_CONTROL_PROTOCOL *This,
  IN  CHAR16                       *Password
  )
{
  return EFI_SUCCESS;
}


EFI_CONSOLE_CONTROL_PROTOCOL gConsoleController =
{
  GetModeImpl,
  SetModeImpl,
  LockStdInImpl
};

EFI_STATUS
InitializeConsoleSim ()
{
  EFI_STATUS Status;
  
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &gImageHandle,
                  &gEfiConsoleControlProtocolGuid,
                  &gConsoleController,
                  NULL
                  );

  return Status;
}
