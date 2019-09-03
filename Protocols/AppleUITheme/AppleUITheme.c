

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>


STATIC UINT32           mCurrentColor;
STATIC EFI_HANDLE       Handle = NULL;

extern EFI_GUID gEfiAppleNvramGuid;

#define BLACK_COLOR  0x000000

#define APPLE_USER_INTERFACE_THEME_PROTOCOL_GUID \
{0xD5B0AC65, 0x9A2D, 0x4D2A, {0xBB, 0xD6, 0xE8, 0x71, 0xA9, 0x5E, 0x04, 0x35}}

//EFI_GUID gAppleUserInterfaceThemeProtocolGuid = APPLE_USER_INTERFACE_THEME_PROTOCOL_GUID;
extern EFI_GUID gAppleUserInterfaceThemeProtocolGuid;

typedef EFI_STATUS (EFIAPI *APPLE_USER_INTERFACE_THEME_GETCOLOR) (
  IN OUT UINT32 * Color
);

typedef struct {
  UINT64                                Version;
  APPLE_USER_INTERFACE_THEME_GETCOLOR   GetColor;
} APPLE_USER_INTERFACE_THEME_PROTOCOL;

EFI_STATUS
EFIAPI
UserInterfaceThemeGetColor (
  UINT32    *Color
  )
{
//  UINTN          DataSize;
//  EFI_STATUS     Status;

  if (Color == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *Color = mCurrentColor;
  return EFI_SUCCESS;
}

STATIC APPLE_USER_INTERFACE_THEME_PROTOCOL mAppleUserInterfaceThemeProtocol = {
  1,
  UserInterfaceThemeGetColor
};

EFI_STATUS
EFIAPI
UserInterfaceThemeEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                    Status;
  UINTN                         DataSize;
  UINT32                        Color;
  // Default color is black
  mCurrentColor = BLACK_COLOR;

  DataSize = 0;
  Status = gRT->GetVariable(L"DefaultBackgroundColor", &gEfiAppleNvramGuid, 0, &DataSize, &Color);
  if (!EFI_ERROR(Status)) {
    mCurrentColor = Color;
  }

  Status = gBS->InstallProtocolInterface(&Handle, &gAppleUserInterfaceThemeProtocolGuid, 0, &mAppleUserInterfaceThemeProtocol);

  return EFI_SUCCESS;
}
