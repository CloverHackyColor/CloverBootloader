/*
 * PlatformDriverOverride.c
 *
 * Funcs for installing or overriding EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL
 * to enable putting our drivers to highest priority when connecting devices.
 *
 * dmazar, 9/11/2012
 *
 */

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include <Protocol/PlatformDriverOverride.h>

#ifndef DEBUG_ALL
#define DEBUG_PLO 1
#else
#define DEBUG_PLO DEBUG_ALL
#endif

#if DEBUG_PLO == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_PLO, __VA_ARGS__)
#endif

/** NULL terminated list of driver's handles that will be served by EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL. */
EFI_HANDLE  *mPriorityDrivers = NULL;

/** Saved original EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL.GetDriver when doing override. */
EFI_PLATFORM_DRIVER_OVERRIDE_GET_DRIVER mOrigPlatformGetDriver = NULL;
EFI_PLATFORM_DRIVER_OVERRIDE_DRIVER_LOADED mOrigPlatformDriverLoaded = NULL;


//////////////////////////////////////////////////////////////////////
//
// Our EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL implementation
//

/** Our EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL.GetDriver implementation. */
EFI_STATUS
EFIAPI
OurPlatformGetDriver (
  IN EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL              *This,
  IN     EFI_HANDLE                                     ControllerHandle,
  IN OUT EFI_HANDLE                                     *DriverImageHandle
  )
{
  EFI_HANDLE     *HandlePtr;
  
  if (ControllerHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  if (mPriorityDrivers == NULL) {
    return EFI_NOT_FOUND;
  }
  
  // if first call - return first
  if (*DriverImageHandle == NULL) {
    *DriverImageHandle = mPriorityDrivers[0];
    return EFI_SUCCESS;
  }
  
  // search through our list
  for (HandlePtr = mPriorityDrivers; *HandlePtr != NULL; HandlePtr++) {
    if (*HandlePtr == *DriverImageHandle) {
      // we should return next after that
      HandlePtr++;
      if (*HandlePtr == NULL) {
        return EFI_NOT_FOUND;
      }
      *DriverImageHandle = *HandlePtr;
      return EFI_SUCCESS;
    }
  }
  
  return EFI_INVALID_PARAMETER;
}
///
// Our EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL.GetDriverPath implementation.
///
EFI_STATUS
EFIAPI
OurPlatformGetDriverPath (
  IN EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL              *This,
  IN     EFI_HANDLE                                     ControllerHandle,
  IN OUT EFI_DEVICE_PATH_PROTOCOL                       **DriverImagePath
  )
{
  return EFI_UNSUPPORTED;
}

///
// Our EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL.DriverLoaded implementation.
///
EFI_STATUS
EFIAPI
OurPlatformDriverLoaded (
  IN EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL          *This,
  IN EFI_HANDLE                                     ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL                       *DriverImagePath,
  IN EFI_HANDLE                                     DriverImageHandle
  )
{
#if DEBUG_DRIVER_OVERRIDE
  EFI_STATUS              Status;
  CHAR16                           *DriverName;
  EFI_COMPONENT_NAME_PROTOCOL      *CompName;
  EFI_BLOCK_IO_PROTOCOL         *BlkIo = NULL;
  
  Status = gBS->HandleProtocol (
                                ControllerHandle,
                                &gEfiBlockIoProtocolGuid,
                                (VOID **) &BlkIo
                                );
  if (EFI_ERROR(Status)) {
    return EFI_UNSUPPORTED;
  }
  
  Status = gBS->OpenProtocol(
                             DriverImageHandle,
                             &gEfiComponentNameProtocolGuid,
                             (VOID**)&CompName,
                             gImageHandle,
                             NULL,
                             EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  
  if (EFI_ERROR(Status)) {
    DBG(" CompName %s\n", efiStrError(Status));
    return EFI_UNSUPPORTED;
  }
  Status = CompName->GetDriverName(CompName, "eng", &DriverName);
  if (!EFI_ERROR(Status)) {
    DBG(" DriverName=%ls at Controller=%hhX\n", DriverName, ControllerHandle);
  }

#endif
  if (mOrigPlatformDriverLoaded) {
    return mOrigPlatformDriverLoaded(This, ControllerHandle, DriverImagePath, DriverImageHandle);
  }
  return EFI_UNSUPPORTED;
}

/** Our EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL structure. */
EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL mOurPlatformDriverOverrideProtocol = {
  OurPlatformGetDriver,
  OurPlatformGetDriverPath,
  OurPlatformDriverLoaded
};


//////////////////////////////////////////////////////////////////////
//
// Overriding EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL if already installed
//
// EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL.GetDriver override.
//
//////////

EFI_STATUS
EFIAPI
OvrPlatformGetDriver(
  IN EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL              *This,
  IN     EFI_HANDLE                                     ControllerHandle,
  IN OUT EFI_HANDLE                                     *DriverImageHandle
)
{
  EFI_HANDLE     *HandlePtr;

  if (mPriorityDrivers == NULL) {
    return mOrigPlatformGetDriver(This, ControllerHandle, DriverImageHandle);
  }

  // if first call - return first
  if (*DriverImageHandle == NULL) {
    *DriverImageHandle = mPriorityDrivers[0];
    return EFI_SUCCESS;
  }
  
  // search through our list
  for (HandlePtr = mPriorityDrivers; *HandlePtr != NULL; HandlePtr++) {
    if (*HandlePtr == *DriverImageHandle) {
      // we should return next after that
      HandlePtr++;
      if (*HandlePtr == NULL) {
        // our list is exhausted - we'll pass call to original
        *DriverImageHandle = NULL;
        break;
      }
      *DriverImageHandle = *HandlePtr;
      return EFI_SUCCESS;
    }
  }

  // not found in our list, or was last in our list - call original
  return mOrigPlatformGetDriver(This, ControllerHandle, DriverImageHandle);
}


//////////////////////////////////////////////////////////////////////
//
// Public funcs
//
// Registers given PriorityDrivers (NULL terminated) to highest priority during connecting controllers.
// Does this by installing our EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL
// or by overriding existing EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL.GetDriver.
//
////

VOID RegisterDriversToHighestPriority(IN EFI_HANDLE *PriorityDrivers)
{
  EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL *PlatformDriverOverride;
  EFI_STATUS                            Status;
  
  mPriorityDrivers = PriorityDrivers;
  Status = gBS->LocateProtocol(&gEfiPlatformDriverOverrideProtocolGuid, NULL, (VOID **) &PlatformDriverOverride);
  if (EFI_ERROR(Status)) {
    DBG("PlatformDriverOverrideProtocol not found. Installing ... ");
    Status = gBS->InstallMultipleProtocolInterfaces (
                                                     &gImageHandle,
                                                     &gEfiPlatformDriverOverrideProtocolGuid,
                                                     &mOurPlatformDriverOverrideProtocol,
                                                     NULL
                                                     );
    DBG("%s\n", efiStrError(Status));
    return;
  }
  
  mOrigPlatformGetDriver = PlatformDriverOverride->GetDriver;
  PlatformDriverOverride->GetDriver = OvrPlatformGetDriver;
  DBG("PlatformDriverOverrideProtocol->GetDriver overriden\n");
  mOrigPlatformDriverLoaded = PlatformDriverOverride->DriverLoaded;
  PlatformDriverOverride->DriverLoaded = OurPlatformDriverLoaded;
  
}

