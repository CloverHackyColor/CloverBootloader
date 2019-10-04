/** @file
  Ami input translators.

Copyright (c) 2016, vit9696. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Keycode/AIK.h"
#include "Pointer/AIM.h"
#include "Timer/AIT.h"

#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>

STATIC EFI_EVENT                mExitBootServicesEvent;
STATIC BOOLEAN                  mPerformedExit;

VOID
EFIAPI
AmiShimTranslatorExitBootServicesHandler (
  IN  EFI_EVENT                 Event,
  IN  VOID                      *Context
  )
{
  if (mPerformedExit) {
    return;
  }

  mPerformedExit = TRUE;

  //
  // Not sure if necessary
  //
  gBS->CloseEvent (mExitBootServicesEvent);

  AITExit ();
  AIMExit ();
  AIKExit ();
}

EFI_STATUS
EFIAPI
AptioInputEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS    Status;

  AITInit ();
  AIMInit ();
  AIKInit ();

  Status = gBS->CreateEvent (EVT_SIGNAL_EXIT_BOOT_SERVICES, TPL_NOTIFY, AmiShimTranslatorExitBootServicesHandler, NULL, &mExitBootServicesEvent);
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_INFO, "Failed to create exit bs event %d", Status));
  }

  return EFI_SUCCESS;
}
