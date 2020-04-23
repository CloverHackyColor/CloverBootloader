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

#include "EfiKey.h"
#include "AppleKey.h"

#include <Library/DevicePathLib.h>
/*
STATIC UINT16 mIdVendor = 0x05ac; //Apple inc.

STATIC UINT16 mIdProduct = 0x021d; //iMac aluminium keyboard

STATIC UINT8 mCountryCode = 0;
*/
STATIC VOID *mAppleKeyMapDbRegistration = NULL;
APPLE_KEY_MAP_DATABASE_PROTOCOL *mAppleKeyMapDb = NULL;

STATIC
EFI_STATUS
UsbKbSetAppleKeyMapDb (
  IN USB_KB_DEV                       *UsbKeyboardDevice,
  IN APPLE_KEY_MAP_DATABASE_PROTOCOL  *AppleKeyMapDb
  )
{
  EFI_STATUS Status;

  if (!UsbKeyboardDevice || !AppleKeyMapDb) {
    return EFI_UNSUPPORTED;
  }

  Status = AppleKeyMapDb->CreateKeyStrokesBuffer (
                            AppleKeyMapDb,
                            6,
                            &UsbKeyboardDevice->KeyMapDbIndex
                            );

  if (!EFI_ERROR(Status)) {
    UsbKeyboardDevice->KeyMapDb = AppleKeyMapDb;
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
UsbKbAppleKeyMapDbInstallNotify (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS                      Status;
//  APPLE_KEY_MAP_DATABASE_PROTOCOL *AppleKeyMapDb;

  ASSERT (Event != NULL);
  ASSERT (Context != NULL);
//  ASSERT (((USB_KB_DEV *)Context)->Signature == USB_KB_DEV_SIGNATURE); //not only usb
// 
  Status = gBS->LocateProtocol (
                  &gAppleKeyMapDatabaseProtocolGuid,
                  mAppleKeyMapDbRegistration,
                  (VOID **)&mAppleKeyMapDb
                  );
  if (!EFI_ERROR(Status)) {
    UsbKbSetAppleKeyMapDb ((USB_KB_DEV *)Context, mAppleKeyMapDb);
    gBS->CloseEvent (Event);
  }
}

EFI_STATUS
UsbKbLocateAppleKeyMapDb (
  IN USB_KB_DEV  *UsbKeyboardDevice
  )
{
  EFI_STATUS                      Status;
  

  //ASSERT (UsbKeyboardDevice != NULL);
  if (!UsbKeyboardDevice) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->LocateProtocol (
                  &gAppleKeyMapDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&mAppleKeyMapDb
                  );
  if (!EFI_ERROR(Status)) {
    UsbKbSetAppleKeyMapDb (UsbKeyboardDevice, mAppleKeyMapDb);
  } 
    else /*if (PcdGetBool (PcdNotifyAppleKeyMapDbInUsbKbDriver))*/ {  //true
    EfiCreateProtocolNotifyEvent (
      &gAppleKeyMapDatabaseProtocolGuid,
      TPL_NOTIFY,
      UsbKbAppleKeyMapDbInstallNotify,
      (VOID *)UsbKeyboardDevice,
      &mAppleKeyMapDbRegistration
      );
  }
    
  return Status; //signal that Db is not ready
}

VOID
UsbKbFreeAppleKeyMapDb (
  IN USB_KB_DEV  *UsbKeyboardDevice
  )
{
  if (UsbKeyboardDevice->KeyMapDb != NULL) {
    UsbKeyboardDevice->KeyMapDb->RemoveKeyStrokesBuffer (
                                   UsbKeyboardDevice->KeyMapDb,
                                   UsbKeyboardDevice->KeyMapDbIndex
                                   );
  }
}

