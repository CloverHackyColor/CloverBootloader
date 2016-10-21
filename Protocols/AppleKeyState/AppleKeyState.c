//
//  AppleKeyState.c
//  Clover
//
//  Created by Slice on 21.10.16.
//

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/AppleKeyState.h>

#define NON_APPLE_SIGNATURE SIGNATURE_64('N','O','N','A','P','P','L','E')


EFI_HANDLE              mHandle = NULL;

//
// Keyboard input
//

BOOLEAN ReadAllKeyStrokes(VOID)
{
  BOOLEAN       GotKeyStrokes;
  EFI_STATUS    Status;
  EFI_INPUT_KEY key;
  
  GotKeyStrokes = FALSE;
  for (;;) {
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &key);
    if (Status == EFI_SUCCESS) {
      GotKeyStrokes = TRUE;
      continue;
    }
    break;
  }
  return GotKeyStrokes;
}


EFI_STATUS
EFIAPI
ReadKeyState (APPLE_KEY_STATE_PROTOCOL* This,
              OUT UINT16 *ModifyFlags,
              OUT UINTN  *PressedKeyStatesCount,
              OUT APPLE_KEY *PressedKeyStates)
{
  EFI_STATUS				Status;
  EFI_INPUT_KEY Key;
  UINTN         Ind = 0;

  if (!ModifyFlags || !PressedKeyStatesCount || !PressedKeyStates) {
    return EFI_INVALID_PARAMETER;
  }

  while (ReadAllKeyStrokes()) gBS->Stall(500 * 1000);

  gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &Ind);
  Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
  
  *PressedKeyStatesCount = 2;
  PressedKeyStates[0] = Key.UnicodeChar; 
  PressedKeyStates[1] = Key.ScanCode;
  *ModifyFlags = 0;
  return Status;
}

EFI_STATUS
EFIAPI
SearchKeyStroke (APPLE_KEY_STATE_PROTOCOL* This,
                 IN UINT16 ModifyFlags,
                 IN UINTN PressedKeyStatesCount,
                 IN OUT APPLE_KEY *PressedKeyStates,
                 IN BOOLEAN ExactMatch)
{
  return EFI_NOT_FOUND;
}


APPLE_KEY_STATE_PROTOCOL mAppleKeyState = {
  NON_APPLE_SIGNATURE,
  ReadKeyState,
  SearchKeyStroke,
};


/****************************************************************
 * Entry point
 ***************************************************************/

/*
 * AppleKeyState entry point. Installs AppleKeyStateProtocol also
 * known as AppleBootKeyPressProtocolGuid.
 *
 */
EFI_STATUS
EFIAPI
AppleKeyStateEntrypoint (
                  IN EFI_HANDLE           ImageHandle,
                  IN EFI_SYSTEM_TABLE		*SystemTable
                  )
{
  EFI_STATUS					Status; 
  EFI_BOOT_SERVICES*			gBS;  
  gBS				= SystemTable->BootServices;

  mHandle = 0;
  Status = gBS->InstallMultipleProtocolInterfaces (
                &mHandle,
                &gAppleKeyStateProtocolGuid,
                &mAppleKeyState,
                NULL
                );
 
/*  Status      = gBS->InstallProtocolInterface (
                                               &ImageHandle,
                                               &gAppleKeyStateProtocolGuid,
                                               EFI_NATIVE_INTERFACE,
                                               (VOID *)&mAppleKeyState
                                               );

*/  
  return Status;
}
