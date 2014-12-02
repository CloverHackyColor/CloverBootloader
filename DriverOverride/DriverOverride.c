/*
 * DriverOverride.c
 *
 * Funcs for installing EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL 
 * in CloverEFI
 *
 */

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/PlatformDriverOverride.h>


//////////////////////////////////////////////////////////////////////
//
// Our EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL implementation
//

/** Our EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL.GetDriver implementation. */
EFI_STATUS
EFIAPI
PlatformGetDriver (
  IN EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL              *This,
  IN     EFI_HANDLE                                     ControllerHandle,
  IN OUT EFI_HANDLE                                     *DriverImageHandle
  )
{
    return EFI_NOT_FOUND;
}

/** Our EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL.GetDriverPath implementation. */
EFI_STATUS
EFIAPI
PlatformGetDriverPath (
  IN EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL              *This,
  IN     EFI_HANDLE                                     ControllerHandle,
  IN OUT EFI_DEVICE_PATH_PROTOCOL                       **DriverImagePath
  )
{
  return EFI_UNSUPPORTED;
}

/** Our EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL.DriverLoaded implementation. */
EFI_STATUS
EFIAPI
PlatformDriverLoaded (
  IN EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL          *This,
  IN EFI_HANDLE                                     ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL                       *DriverImagePath,
  IN EFI_HANDLE                                     DriverImageHandle
  )
{
  return EFI_UNSUPPORTED;
}

/** Our EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL structure. */
EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL mPlatformDriverOverrideProtocol = {
  PlatformGetDriver,
  PlatformGetDriverPath,
  PlatformDriverLoaded
};



//////////////////////////////////////////////////////////////////////
//
// Entry
//
//////

EFI_STATUS
EFIAPI
DriverOverrideEntrypoint(
                         IN EFI_HANDLE           ImageHandle,
                         IN EFI_SYSTEM_TABLE		*SystemTable
                         )
{
  EFI_STATUS                            Status;
  EFI_HANDLE              NewHandle;
  
  NewHandle = NULL;  // install to a new handle
  
  Status = gBS->InstallMultipleProtocolInterfaces (
                      &NewHandle,
                      &gEfiPlatformDriverOverrideProtocolGuid,
                      &mPlatformDriverOverrideProtocol,
                      NULL
                      );
  return Status;
}

