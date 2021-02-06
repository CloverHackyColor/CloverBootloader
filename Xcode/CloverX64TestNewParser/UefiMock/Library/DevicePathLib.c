//
//  DevicePathLib.h
//  cpp_tests_compare_settings
//
//  Created by Jief on 05/02/2021.
//  Copyright © 2021 JF Knudsen. All rights reserved.
//

#include <Efi.h>

EFI_DEVICE_PATH_PROTOCOL *
EFIAPI
AppendDevicePath (
  IN CONST EFI_DEVICE_PATH_PROTOCOL  *FirstDevicePath,  OPTIONAL
  IN CONST EFI_DEVICE_PATH_PROTOCOL  *SecondDevicePath  OPTIONAL
  )
{
  panic("not yet");
}

UINTN
EFIAPI
GetDevicePathSize (
  IN CONST EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
{
  panic("not yet");
}

EFI_DEVICE_PATH_PROTOCOL *
EFIAPI
GetNextDevicePathInstance (
  IN OUT EFI_DEVICE_PATH_PROTOCOL    **DevicePath,
  OUT UINTN                          *Size
  )
{
  panic("not yet");
}
