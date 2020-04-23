/** @file
  Produces Keyboard MapDb Protocol.

  Copyright (C) 2016 CupertinoNet.  All rights reserved.<BR>

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
**/

//#include "EfiKey.h"
#include "AppleKey.h"
//#include "BiosKeyboard.h"


STATIC VOID *mAppleKeyMapDbRegistration = NULL;
APPLE_KEY_MAP_DATABASE_PROTOCOL *mAppleKeyMapDb = NULL;

STATIC
EFI_STATUS
BiosKbSetAppleKeyMapDb (
  IN BIOS_KEYBOARD_DEV                *BiosKeyboardDevice,
  IN APPLE_KEY_MAP_DATABASE_PROTOCOL  *AppleKeyMapDb
  )
{
  EFI_STATUS Status;

  if (!BiosKeyboardDevice || !AppleKeyMapDb) {
    return EFI_UNSUPPORTED;
  }

  Status = AppleKeyMapDb->CreateKeyStrokesBuffer (
                            AppleKeyMapDb,
                            6,
                            &BiosKeyboardDevice->KeyMapDbIndex
                            );

  if (!EFI_ERROR(Status)) {
    BiosKeyboardDevice->KeyMapDb = AppleKeyMapDb;
  }
  return Status;
}

/**
  Protocol installation notify for Apple KeyMap Database.

  @param[in] Event    Indicates the event that invoke this function.
  @param[in] Context  Indicates the calling context.
**/

VOID
EFIAPI
BiosKbAppleKeyMapDbInstallNotify (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS                      Status;

  ASSERT (Event != NULL);
  ASSERT (Context != NULL);


  Status = gBS->LocateProtocol (
                  &gAppleKeyMapDatabaseProtocolGuid,
                  mAppleKeyMapDbRegistration,
                  (VOID **)&mAppleKeyMapDb
                  );
  if (!EFI_ERROR(Status)) {
    BiosKbSetAppleKeyMapDb ((BIOS_KEYBOARD_DEV *)Context, mAppleKeyMapDb);
    gBS->CloseEvent (Event);
  }
}

EFI_STATUS
BiosKbLocateAppleKeyMapDb (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardDevice
  )
{
  EFI_STATUS                      Status;
  

  //ASSERT (BiosKeyboardDevice != NULL);
  if (!BiosKeyboardDevice) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->LocateProtocol (
                  &gAppleKeyMapDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&mAppleKeyMapDb
                  );
  if (!EFI_ERROR(Status)) {
    BiosKbSetAppleKeyMapDb (BiosKeyboardDevice, mAppleKeyMapDb);
  } 
    else  {  
    EfiCreateProtocolNotifyEvent (
      &gAppleKeyMapDatabaseProtocolGuid,
      TPL_NOTIFY,
      BiosKbAppleKeyMapDbInstallNotify,
      (VOID *)BiosKeyboardDevice,
      &mAppleKeyMapDbRegistration
      );
  }
    
  return Status; //signal that Db is not ready
}

VOID
BiosKbFreeAppleKeyMapDb (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardDevice
  )
{
  if (BiosKeyboardDevice->KeyMapDb != NULL) {
    BiosKeyboardDevice->KeyMapDb->RemoveKeyStrokesBuffer (
                                   BiosKeyboardDevice->KeyMapDb,
                                   BiosKeyboardDevice->KeyMapDbIndex
                                   );
  }
}

