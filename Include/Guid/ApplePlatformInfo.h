/** @file
  Copyright (C) 2005 - 2016, Apple Inc.  All rights reserved.
  Portions Copyright (C) 2014 - 2016, CupertinoNet.  All rights reserved.<BR>

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
#ifndef APPLE_PLATFORM_INFO_H_
#define APPLE_PLATFORM_INFO_H_

// APPLE_PLATFORM_INFO_KEYBOARD_GUID
#define APPLE_PLATFORM_INFO_KEYBOARD_GUID                 \
  { 0x51871CB9, 0xE25D, 0x44B4,                           \
    { 0x96, 0x99, 0x0E, 0xE8, 0x64, 0x4C, 0xED, 0x69 } }

// gApplePlatformInfoKeyboardGuid
extern EFI_GUID gApplePlatformInfoKeyboardGuid;

#endif // APPLE_PLATFORM_INFO_H_