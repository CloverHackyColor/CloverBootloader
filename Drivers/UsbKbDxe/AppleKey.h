/** @file
  Header file for Apple-specific USB Keyboard Driver's Data Structures.

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
#ifndef _APPLE_USB_KB_H_
#define _APPLE_USB_KB_H_

#include "EfiKey.h"

//#include <Guid/ApplePlatformInfo.h>

#include <Protocol/AppleKeyMapDatabase.h>
//#include <Protocol/ApplePlatformInfoDatabase.h>
//#include <Protocol/KeyboardInfo.h>
#include <Protocol/UsbIo.h>
/*
//
// Functions of Keyboard Info Protocol
//
EFI_STATUS
EFIAPI
UsbKbGetKeyboardDeviceInfo (
  OUT UINT16  *IdVendor,
  OUT UINT16  *IdProduct,
  OUT UINT8   *CountryCode
  );

VOID
UsbKbInstallKeyboardDeviceInfoProtocol (
  IN USB_KB_DEV           *UsbKeyboardDevice,
  IN EFI_USB_IO_PROTOCOL  *UsbIo
  );
*/
EFI_STATUS
UsbKbLocateAppleKeyMapDb (
  IN USB_KB_DEV  *UsbKeyboardDevice
  );

VOID
UsbKbFreeAppleKeyMapDb (
  IN USB_KB_DEV  *UsbKeyboardDevice
  );

#endif

