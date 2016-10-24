#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/AppleKeyState.h>
#include <Protocol/AppleEvent.h>

//EFI_RUNTIME_SERVICES*   gRT;
EFI_HANDLE              mHandle = NULL;

extern EFI_GUID gAppleEventProtocolGuid;

EFI_STATUS
EFIAPI
RegisterHandler (
                 IN  APPLE_EVENT_TYPE             Type,
                 IN  APPLE_EVENT_NOTIFY_FUNCTION  NotifyFunction,
                 OUT APPLE_EVENT_HANDLE           *Handle,
                 IN  VOID                         *NotifyContext
                 )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
UnregisterHandler (
                   IN APPLE_EVENT_HANDLE  EventHandle
                   )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
SetCursorPosition (
                   IN DIMENSION  *Position
                   )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SetEventName (
              IN OUT APPLE_EVENT_HANDLE  Handle,
              IN     CHAR8               *Name
              )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
IsCapsLockOn (
              IN OUT BOOLEAN  *ClockOn
              )
{
  if (!ClockOn) {
    return EFI_INVALID_PARAMETER;
  }
  *ClockOn = FALSE;
  return EFI_SUCCESS;
}

STATIC APPLE_EVENT_PROTOCOL mAppleEventProtocol = {
  1,
  RegisterHandler,
  UnregisterHandler,
  SetCursorPosition,
  SetEventName,
  IsCapsLockOn
};

EFI_STATUS
EFIAPI
AppleEventEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                    Status;

//  gRT = SystemTable->RuntimeServices;

  Status = gBS->InstallProtocolInterface(&mHandle, &gAppleEventProtocolGuid, 0, &mAppleEventProtocol);

  return Status;
}
