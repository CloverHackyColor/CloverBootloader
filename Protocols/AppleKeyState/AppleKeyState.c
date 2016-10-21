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

EFI_STATUS
EFIAPI
ReadKeyState (APPLE_KEY_STATE_PROTOCOL* This,
              OUT UINT16 *ModifyFlags,
              OUT UINTN  *PressedKeyStatesCount,
              OUT APPLE_KEY *PressedKeyStates)
{
  EFI_INPUT_KEY Key;
  UINTN         Ind = 0;

  if (!ModifyFlags || !PressedKeyStatesCount || !PressedKeyStates) {
    return EFI_INVALID_PARAMETER;
  }
  /*
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  if (Status == EFI_NOT_READY) {
    gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &Ind);
  }

  if (ReadAllKeyStrokes()) {  // remove buffered key strokes
    gBS->Stall(500000);      // 0.5 seconds delay
    ReadAllKeyStrokes();    // empty the buffer again
  }
  */
  gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &Ind);
  Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
  
  *PressedKeyStatesCount = 2;
  PressedKeyStates[0] = Key.Unicode; 
  PressedKeyStates[1] = Key.ScanCode;
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
