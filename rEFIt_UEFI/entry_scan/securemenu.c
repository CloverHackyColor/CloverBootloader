/*
 * refit/scan/securemenu.c
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

#ifndef DEBUG_ALL
#define DEBUG_SECURE_MENU 1
#else
#define DEBUG_SECURE_MENU DEBUG_ALL
#endif

#if DEBUG_SECURE_MENU == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_SECURE_MENU, __VA_ARGS__)
#endif

extern BOOLEAN gGuiIsReady;
extern BOOLEAN gThemeNeedInit;

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

// Query the secure boot user what to do with image
UINTN QuerySecureBootUser(IN CONST EFI_DEVICE_PATH_PROTOCOL *DevicePath)
{
  UINTN Response = SECURE_BOOT_POLICY_DENY;
  // Check parameters
  if (DevicePath != NULL) {
    // Get the device path string
    CHAR16 *DevicePathStr = FileDevicePathToStr(DevicePath);
    if (DevicePathStr != NULL) {
      CHAR16 *DevicePathFileStr = FileDevicePathFileToStr(DevicePath);
      if (DevicePathFileStr != NULL) {
        REFIT_MENU_SCREEN  QueryUserMenu = { 10101, L"Secure Boot Authentication", NULL, 0, NULL, 0, NULL, 0, NULL, FALSE, FALSE, 0, 0, 0, 0, {0, 0, 0, 0}, NULL };
        REFIT_MENU_ENTRY   DenyImage = { L"Deny authentication", SECURE_BOOT_POLICY_DENY, 0, 0, 0, NULL, NULL, NULL, {0, 0, 0, 0}, ActionEnter, ActionNone, ActionNone, NULL };
        REFIT_MENU_ENTRY   AllowImage = { L"Allow authentication", SECURE_BOOT_POLICY_ALLOW, 0, 0, 0, NULL, NULL, NULL, {0, 0, 0, 0}, ActionEnter, ActionNone, ActionNone, NULL };
        REFIT_MENU_ENTRY   InsertImage = { L"Insert image authentication into database", SECURE_BOOT_POLICY_INSERT, 0, 0, 0, NULL, NULL, NULL, {0, 0, 0, 0}, ActionEnter, ActionNone, ActionNone, NULL };
        REFIT_MENU_ENTRY  *ChosenEntry = NULL;
        // Debug message
        DBG("VerifySecureBootImage: Query user for authentication action for %s\n", DevicePathStr);
        // Set the menu up
        AddMenuInfoLine(&QueryUserMenu, L"Please select the authentication action for");
        AddMenuInfoLine(&QueryUserMenu, DevicePathStr);
        AddMenuInfoLine(&QueryUserMenu, DevicePathFileStr);
        AddMenuEntry(&QueryUserMenu, &DenyImage);
        AddMenuEntry(&QueryUserMenu, &AllowImage);
        if (!gSettings.SecureBootSetupMode) {
          // Only insert image if secure boot but not in forced mode
          AddMenuEntry(&QueryUserMenu, &InsertImage);
        }
        // Because we may
        if (!gGuiIsReady) {
          InitScreen(FALSE);
          InitTheme(TRUE);
          gThemeNeedInit = FALSE;
          gGuiIsReady = TRUE;
        }
        // Run the query menu
        if ((RunMenu(&QueryUserMenu, &ChosenEntry) == MENU_EXIT_ENTER) &&
            (ChosenEntry != NULL)) {
          Response = (UINTN)ChosenEntry->Tag;
        }
        FreePool(DevicePathFileStr);
      }
      FreePool(DevicePathStr);
    }
  }
  return Response;
}

// TODO: Configure secure boot
BOOLEAN ConfigureSecureBoot(VOID)
{
   return FALSE;
}
