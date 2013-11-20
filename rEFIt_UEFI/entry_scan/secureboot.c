/*
 * refit/scan/secureboot.c
 *
 * Copyright (c) 2006-2010 Christoph Pfisterer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Christoph Pfisterer nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "entry_scan.h"

#include <Protocol/Security.h>
#include <Protocol/Security2.h>

#ifndef DEBUG_ALL
#define DEBUG_SECURE_BOOT 1
#else
#define DEBUG_SECURE_BOOT DEBUG_ALL
#endif

#if DEBUG_SECURE_BOOT == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_SECURE_BOOT, __VA_ARGS__)
#endif

// Add secure boot tool entry
VOID AddSecureBootTool(VOID)
{
  LOADER_ENTRY *Entry;
  // If in forced mode or no secure boot then don't add tool
  if ((gSettings.SecureBoot && gSettings.SecureBootSetupMode) ||
      (!gSettings.SecureBoot && !gSettings.SecureBootSetupMode)) {
    return;
  }
  Entry = AllocateZeroPool(sizeof(LOADER_ENTRY));
  if (gSettings.SecureBoot) {
    Entry->me.Title = PoolPrint(L"Clover Secure Boot Configuration");
    Entry->me.Tag = TAG_SECURE_BOOT_CONFIG;
    Entry->me.Image = BuiltinIcon(BUILTIN_ICON_FUNC_SECURE_BOOT_CONFIG);
  } else {
    Entry->me.Title = PoolPrint(L"Enable Clover Secure Boot");
    Entry->me.Tag = TAG_SECURE_BOOT;
    Entry->me.Image = BuiltinIcon(BUILTIN_ICON_FUNC_SECURE_BOOT);
  }
  Entry->me.Row = 1;
  //actions
  Entry->me.AtClick = ActionSelect;
  Entry->me.AtDoubleClick = ActionEnter;
  Entry->me.AtRightClick = ActionHelp;
  AddMenuEntry(&MainMenu, (REFIT_MENU_ENTRY *)Entry);
}

// Enable secure boot
VOID EnableSecureBoot(VOID)
{
   EFI_STATUS Status;
  // Check in setup mode
  if (gSettings.SecureBoot || !gSettings.SecureBootSetupMode) {
    return;
  }
  // TODO: Generate PK
  // TODO: Enroll PK
  // TODO: Generate KEK?
  // TODO: Enroll KEK?
  // TODO: Get this image's certificate
  // TODO: Enroll this image's certificate
  // Reinit secure boot now
  InitializeSecureBoot();
  // Install the security policy hooks or redisable
  if (EFI_ERROR(Status = InstallSecureBoot())) {
    DBG("Secure boot install failed: %r!\n", Status);
    DisableSecureBoot();
  }
}

STATIC CONST CHAR16 *SecureBootPolicyToStr(IN UINTN Policy)
{
   STATIC CONST CHAR16 *SecureBootPolicyStrings[] = {
     L"Deny",
     L"Allow",
     L"Query",
     L"Insert",
     L"WhiteList",
     L"BlackList",
   };
   STATIC CONST UINTN  SecureBootPolicyStringsCount = (sizeof(SecureBootPolicyStrings) / (sizeof(CONST CHAR16 *)));
   if (Policy < SecureBootPolicyStringsCount) {
     return SecureBootPolicyStrings[Policy];
   }
   return L"Unknown";
}

STATIC VOID PrintSecureBootInfo(VOID)
{
  // Nothing to do if secure boot is disabled or in setup mode
  if (!gSettings.SecureBoot) {
    DBG("Secure boot: %a\n", (gSettings.SecureBootSetupMode ? "Setup" : "Disabled"));
  } else {
    // Secure boot is enabled
    DBG("Secure boot: %a\n", (gSettings.SecureBootSetupMode ? "Forced" : "Enabled"));
    DBG("Boot Policy: %s\n", SecureBootPolicyToStr(gSettings.SecureBootPolicy));
  }
}

// Disable secure boot
VOID DisableSecureBoot(VOID)
{
  // Check in user mode
  if (gSettings.SecureBootSetupMode || !gSettings.SecureBoot) {
    return;
  }
  // TODO: Get this image's certificate
  // TODO: Remove this image's certificate
  // TODO: Delete KEK?
  // TODO: Delete PK
  // Reinit secure boot now
  InitializeSecureBoot();
  PrintSecureBootInfo();
}

// Insert secure boot image signature
STATIC EFI_STATUS InsertSecureBootImage(IN CONST EFI_DEVICE_PATH_PROTOCOL *DevicePath,
                                        IN VOID                           *FileBuffer,
                                        IN UINTN                           FileSize)
{
  EFI_STATUS  Status = EFI_INVALID_PARAMETER;
  VOID       *TempFileBuffer = NULL;
  // Check that either the device path or the file buffer is valid
  if ((DevicePath == NULL) && ((FileBuffer == NULL) || (FileSize == 0))) {
    return EFI_INVALID_PARAMETER;
  }
  // If the file buffer is not valid load file by device path
  if (FileBuffer == NULL) {
    UINT32 AuthenticationStatus = 0;
    FileBuffer = TempFileBuffer = GetFileBufferByFilePath(FALSE, DevicePath, &FileSize, &AuthenticationStatus);
  }
  // Check the file buffer is valid
  if ((FileBuffer != NULL) && (FileSize > 0)) {
    // TODO: Get the image signature
    Status = EFI_SUCCESS; // GetImageSignature(FileBuffer, FileSize, Signature);
    if (!EFI_ERROR(Status)) {
      // TODO: Add the image signature to database
    }
  }
  // Cleanup buffer if needed
  if (TempFileBuffer != NULL) {
    FreePool(TempFileBuffer);
  }
  return Status;
}

// The previous protocol functions
STATIC EFI_SECURITY_FILE_AUTHENTICATION_STATE gSecurityFileAuthentication;
STATIC EFI_SECURITY2_FILE_AUTHENTICATION      gSecurity2FileAuthentication;

// Pre check the secure boot policy
STATIC BOOLEAN EFIAPI
PrecheckSecureBootPolicy(IN OUT EFI_STATUS                     *AuthenticationStatus,
                         IN     CONST EFI_DEVICE_PATH_PROTOCOL *DevicePath)
{
  CHAR16 *DevicePathStr;
  UINTN   Index;
  if ((AuthenticationStatus == NULL) || (DevicePath == NULL)) {
    return FALSE;
  }
  switch (gSettings.SecureBootPolicy) {
  case SECURE_BOOT_POLICY_ALLOW:
    // Allow all images
    *AuthenticationStatus = EFI_SUCCESS;
    return TRUE;

  case SECURE_BOOT_POLICY_WHITELIST:
    // Check the white list for this image
    DevicePathStr = FileDevicePathToStr(DevicePath);
    if (DevicePathStr == NULL) {
      return FALSE;
    }
    for (Index = 0; Index < gSettings.SecureBootWhiteListCount; ++Index) {
      if ((gSettings.SecureBootWhiteList[Index] != NULL) &&
          (StriStr(DevicePathStr, gSettings.SecureBootWhiteList[Index]) != NULL)) {
        // White listed
        *AuthenticationStatus = EFI_SUCCESS;
        return TRUE;
      }
    }
    return TRUE;

  case SECURE_BOOT_POLICY_BLACKLIST:
    // Check the black list for this image
    DevicePathStr = FileDevicePathToStr(DevicePath);
    if (DevicePathStr == NULL) {
      return FALSE;
    }
    for (Index = 0; Index < gSettings.SecureBootBlackListCount; ++Index) {
      if ((gSettings.SecureBootBlackList[Index] != NULL) &&
          (StriStr(DevicePathStr, gSettings.SecureBootBlackList[Index]) != NULL)) {
        // Black listed
        return TRUE;
      }
    }
    *AuthenticationStatus = EFI_SUCCESS;
    return TRUE;
  }
  return FALSE;
}

// Check the secure boot policy
STATIC BOOLEAN EFIAPI
CheckSecureBootPolicy(IN OUT EFI_STATUS                     *AuthenticationStatus,
                      IN     CONST EFI_DEVICE_PATH_PROTOCOL *DevicePath,
                      IN     VOID                           *FileBuffer,
                      IN     UINTN                           FileSize)
{
  switch (gSettings.SecureBootPolicy) {
  case SECURE_BOOT_POLICY_QUERY:
    // TODO: Query user to allow image or deny image or insert image signature
    /*
    // Perform user action
    switch (UserResponse) {
    case SECURE_BOOT_POLICY_ALLOW:
      *AuthenticationStatus = EFI_SUCCESS;

    case SECURE_BOOT_POLICY_DENY:
      return TRUE;

    default:
      break;
    }
    // */
    // If this is forced mode then no insert
    if (gSettings.SecureBootSetupMode) {
      return TRUE;
    }
    // Purposeful fallback to insert

  case SECURE_BOOT_POLICY_INSERT:
    // Insert image signature
    if (!EFI_ERROR(InsertSecureBootImage(DevicePath, FileBuffer, FileSize))) {
      *AuthenticationStatus = EFI_SUCCESS;
      return TRUE;
    }
    break;

  case SECURE_BOOT_POLICY_ALLOW:
    // Allow all images
    *AuthenticationStatus = EFI_SUCCESS;
    return TRUE;

  case SECURE_BOOT_POLICY_DENY:
     // Deny all images
     return TRUE;
  }
  return FALSE;
}


// Override EFI_SECURITY_ARCH_PROTOCOL
EFI_STATUS EFIAPI
InternalFileAuthentication(IN CONST EFI_SECURITY_ARCH_PROTOCOL *This,
                           IN UINT32                            AuthenticationStatus,
                           IN CONST EFI_DEVICE_PATH_PROTOCOL   *DevicePath)
{
  EFI_STATUS Status = EFI_SECURITY_VIOLATION;
  // Check secure boot policy
  if (!PrecheckSecureBootPolicy(&Status, DevicePath)) {
    // Return original security policy
    Status = gSecurityFileAuthentication(This, AuthenticationStatus, DevicePath);
    if (EFI_ERROR(Status)) {
      // Check security policy on image
      CheckSecureBootPolicy(&Status, DevicePath, NULL, 0);
    }
  }
  if (EFI_ERROR(Status)) {
    CHAR16 *DevicePathStr = FileDevicePathToStr(DevicePath);
    if (DevicePathStr) {
      DBG("VerifySecureBootImage(1): %r %s\n", Status, DevicePathStr);
      FreePool(DevicePathStr);
    }
  }
  return Status;
}

// Override EFI_SECURITY2_ARCH_PROTOCOL
EFI_STATUS EFIAPI
Internal2FileAuthentication(IN CONST EFI_SECURITY2_ARCH_PROTOCOL *This,
                            IN CONST EFI_DEVICE_PATH_PROTOCOL    *DevicePath,
                            IN VOID                              *FileBuffer,
                            IN UINTN                              FileSize,
                            IN BOOLEAN                            BootPolicy)
{
  EFI_STATUS Status = EFI_SECURITY_VIOLATION;
  // Check secure boot policy
  if (!PrecheckSecureBootPolicy(&Status, DevicePath)) {
    // Return original security policy
    Status = gSecurity2FileAuthentication(This, DevicePath, FileBuffer, FileSize, BootPolicy);
    if (EFI_ERROR(Status)) {
      CheckSecureBootPolicy(&Status, DevicePath, FileBuffer, FileSize);
    }
  }
  if (EFI_ERROR(Status)) {
    CHAR16 *DevicePathStr = FileDevicePathToStr(DevicePath);
    if (DevicePathStr) {
      DBG("VerifySecureBootImage(2): %r %s\n", Status, DevicePathStr);
      FreePool(DevicePathStr);
    }
  }
  return Status;
}

// Verify boot policy for image
EFI_STATUS VerifySecureBootImage(IN CONST EFI_DEVICE_PATH_PROTOCOL *DevicePath)
{
  EFI_STATUS Status = EFI_SECURITY_VIOLATION;
  if (!PrecheckSecureBootPolicy(&Status, DevicePath)) {
    if (!CheckSecureBootPolicy(&Status, DevicePath, NULL, 0)) {
      Status = EFI_SUCCESS;
    }
  }
  if (EFI_ERROR(Status)) {
    CHAR16 *DevicePathStr = FileDevicePathToStr(DevicePath);
    if (DevicePathStr) {
      DBG("VerifySecureBootImage: %r %s\n", Status, DevicePathStr);
      FreePool(DevicePathStr);
    }
  }
  return Status;
}

// Install secure boot
EFI_STATUS InstallSecureBoot(VOID)
{
  EFI_STATUS                   Status;
  EFI_SECURITY_ARCH_PROTOCOL  *Security = NULL;
  EFI_SECURITY2_ARCH_PROTOCOL *Security2 = NULL;
  // Check if already installed
  if (gSecurityFileAuthentication) {
    return EFI_SUCCESS;
  }
  PrintSecureBootInfo();
  // Nothing to do if secure boot is disabled or in setup mode
  if (!gSettings.SecureBoot || gSettings.SecureBootSetupMode) {
    return EFI_SUCCESS;
  }
  // Locate security protocols
  gBS->LocateProtocol(&gEfiSecurity2ArchProtocolGuid, NULL, (VOID **)&Security2);
  Status = gBS->LocateProtocol(&gEfiSecurityArchProtocolGuid, NULL, (VOID **)&Security);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  if (Security == NULL) {
    return EFI_NOT_FOUND;
  }
  // Install policy hooks
  gSecurityFileAuthentication = Security->FileAuthenticationState;
  Security->FileAuthenticationState = InternalFileAuthentication;
  if (Security2) {
    gSecurity2FileAuthentication = Security2->FileAuthentication;
    Security2->FileAuthentication = Internal2FileAuthentication;
  }
  return EFI_SUCCESS;
}

// Uninstall secure boot
VOID UninstallSecureBoot(VOID)
{
  // Uninstall policy hooks
  if (gSecurityFileAuthentication) {
    EFI_SECURITY_ARCH_PROTOCOL  *Security = NULL;
    // Restore the security protocol function
    gBS->LocateProtocol(&gEfiSecurityArchProtocolGuid, NULL, (VOID **)&Security);
    if (Security) {
      Security->FileAuthenticationState = gSecurityFileAuthentication;
    }
    gSecurityFileAuthentication = 0;
  }
  if (gSecurity2FileAuthentication) {
    EFI_SECURITY2_ARCH_PROTOCOL *Security2 = NULL;
    // Restory the security 2 protocol function
    gBS->LocateProtocol(&gEfiSecurity2ArchProtocolGuid, NULL, (VOID **)&Security2);
    if (Security2) {
      Security2->FileAuthentication = gSecurity2FileAuthentication;
    }
    gSecurity2FileAuthentication = 0;
  }
}

// Initialize secure boot
VOID InitializeSecureBoot(VOID)
{
  // Set secure boot variables to firmware values
  UINTN Size = sizeof(gSettings.SecureBootSetupMode);
  gRT->GetVariable(L"SetupMode", &gEfiGlobalVariableGuid, NULL, &Size, &gSettings.SecureBootSetupMode);
  Size = sizeof(gSettings.SecureBoot);
  gRT->GetVariable(L"SecureBoot", &gEfiGlobalVariableGuid, NULL, &Size, &gSettings.SecureBoot);
  // Make sure that secure boot is disabled if in setup mode, this will
  //  allow us to specify later in settings that we want to override
  //  setup mode and pretend like we are in secure boot mode to enforce
  //  secure boot policy even when secure boot is not present/disabled.
  if (gSettings.SecureBootSetupMode) {
    gSettings.SecureBoot = 0;
  }
}
