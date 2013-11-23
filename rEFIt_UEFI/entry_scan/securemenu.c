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
  if (!gSettings.SecureBoot && !gSettings.SecureBootSetupMode) {
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

STATIC REFIT_MENU_ENTRY   QueryEntry[] = {
  { L"Deny authentication", SECURE_BOOT_POLICY_DENY, 0, 0, 0, NULL, NULL, NULL, {0, 0, 0, 0}, ActionEnter, ActionNone, ActionNone, NULL },
  { L"Allow authentication", SECURE_BOOT_POLICY_ALLOW, 0, 0, 0, NULL, NULL, NULL, {0, 0, 0, 0}, ActionEnter, ActionNone, ActionNone, NULL },
  { L"Insert authentication into database", SECURE_BOOT_POLICY_INSERT, 0, 0, 0, NULL, NULL, NULL, {0, 0, 0, 0}, ActionEnter, ActionNone, ActionNone, NULL },
};
STATIC REFIT_MENU_ENTRY  *QueryEntries[] = { QueryEntry, QueryEntry + 1, QueryEntry + 2 };
STATIC REFIT_MENU_SCREEN  QueryUserMenu = { 0, L"Secure Boot Authentication", NULL, 3, NULL, 2, QueryEntries,
                                            0, NULL, FALSE, FALSE, 0, 0, 0, 0, { 0, 0, 0, 0 }, NULL };

// Query the secure boot user what to do with image
UINTN QuerySecureBootUser(IN CONST EFI_DEVICE_PATH_PROTOCOL *DevicePath)
{
  UINTN Response = SECURE_BOOT_POLICY_DENY;
  // Check parameters
  if (DevicePath != NULL) {
    // Get the device path string
    CHAR16 *Information[] = { L"Please select the authentication action for", NULL, NULL };
    Information[1] = FileDevicePathToStr(DevicePath);
    if (Information[1] != NULL) {
      // Get the device path file path
      Information[2] = FileDevicePathFileToStr(DevicePath);
      if (Information[2] != NULL) {
        // Create the entries
        REFIT_MENU_ENTRY  *ChosenEntry = NULL;
        // Update the menu
        QueryUserMenu.InfoLines = Information;
        QueryUserMenu.EntryCount = gSettings.SecureBootSetupMode ? 2 : 3;
        // Debug message
        DBG("VerifySecureBootImage: Query user for authentication action for %s\n", Information[0]);
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
        FreePool(Information[2]);
      }
      FreePool(Information[1]);
    }
  }
  return Response;
}

extern REFIT_MENU_ENTRY MenuEntryReturn;

#define TAG_POLICY  1
#define TAG_INSERT  2
#define TAG_REMOVE  3
#define TAG_CLEAR   4
#define TAG_DISABLE 5

STATIC REFIT_MENU_ENTRY   SecureBootPolicyEntry = { NULL, TAG_POLICY, 0, 0, 0, NULL, NULL, NULL, { 0, 0, 0, 0 }, ActionEnter, ActionNone, ActionNone, NULL };
STATIC REFIT_MENU_ENTRY   InsertImageSignatureEntry = { L"Add image authentication to database", TAG_INSERT, 0, 0, 0, NULL, NULL, NULL, {0, 0, 0, 0}, ActionEnter, ActionNone, ActionNone, NULL };
STATIC REFIT_MENU_ENTRY   RemoveImageSignatureEntry = { L"Remove image authentication from database", TAG_REMOVE, 0, 0, 0, NULL, NULL, NULL, {0, 0, 0, 0}, ActionEnter, ActionNone, ActionNone, NULL };
STATIC REFIT_MENU_ENTRY   ClearImageSignatureEntry = { L"Clear image authentication database", TAG_CLEAR, 0, 0, 0, NULL, NULL, NULL, {0, 0, 0, 0}, ActionEnter, ActionNone, ActionNone, NULL };
STATIC REFIT_MENU_ENTRY   DisableSecureBootEntry = { L"Disable secure boot", TAG_DISABLE, 0, 0, 0, NULL, NULL, NULL, {0, 0, 0, 0}, ActionEnter, ActionNone, ActionNone, NULL };
STATIC REFIT_MENU_ENTRY  *SecureBootEntries[] = { NULL, NULL, NULL, NULL, NULL, NULL };
STATIC REFIT_MENU_SCREEN  SecureBootMenu = { 0, L"Secure Boot Configuration", NULL, 0, NULL, 0, NULL,
                                             0, NULL, FALSE, FALSE, 0, 0, 0, 0, { 0, 0, 0, 0 }, NULL };

STATIC REFIT_MENU_ENTRY   SecureBootPolicyNameEntry[] = {
  { L"Deny", SECURE_BOOT_POLICY_DENY, 0, 0, 0, NULL, NULL, NULL, { 0, 0, 0, 0 }, ActionEnter, ActionNone, ActionNone, NULL },
  { L"Allow", SECURE_BOOT_POLICY_ALLOW, 0, 0, 0, NULL, NULL, NULL, { 0, 0, 0, 0 }, ActionEnter, ActionNone, ActionNone, NULL },
  { L"Query", SECURE_BOOT_POLICY_QUERY, 0, 0, 0, NULL, NULL, NULL, { 0, 0, 0, 0 }, ActionEnter, ActionNone, ActionNone, NULL },
  { L"Insert", SECURE_BOOT_POLICY_INSERT, 0, 0, 0, NULL, NULL, NULL, { 0, 0, 0, 0 }, ActionEnter, ActionNone, ActionNone, NULL },
  { L"WhiteList", SECURE_BOOT_POLICY_WHITELIST, 0, 0, 0, NULL, NULL, NULL, { 0, 0, 0, 0 }, ActionEnter, ActionNone, ActionNone, NULL },
  { L"BlackList", SECURE_BOOT_POLICY_BLACKLIST, 0, 0, 0, NULL, NULL, NULL, { 0, 0, 0, 0 }, ActionEnter, ActionNone, ActionNone, NULL },
  { L"User", SECURE_BOOT_POLICY_USER, 0, 0, 0, NULL, NULL, NULL, { 0, 0, 0, 0 }, ActionEnter, ActionNone, ActionNone, NULL },
};
STATIC REFIT_MENU_ENTRY  *SecureBootPolicyEntries[] = { 
  SecureBootPolicyNameEntry,
  SecureBootPolicyNameEntry + 1,
  SecureBootPolicyNameEntry + 2,
  SecureBootPolicyNameEntry + 3,
  SecureBootPolicyNameEntry + 4,
  SecureBootPolicyNameEntry + 5,
  SecureBootPolicyNameEntry + 6,
  &MenuEntryReturn
};
STATIC REFIT_MENU_SCREEN  SecureBootPolicyMenu = { 0, L"Secure Boot Policy", NULL, 0, NULL,
                                                   sizeof(SecureBootPolicyEntries) / sizeof(REFIT_MENU_ENTRY *), SecureBootPolicyEntries,
                                                   0, NULL, FALSE, FALSE, 0, 0, 0, 0, { 0, 0, 0, 0 } , NULL };

// Configure secure boot
BOOLEAN ConfigureSecureBoot(VOID)
{
  BOOLEAN StillConfiguring = TRUE;
  do
  {
    REFIT_MENU_ENTRY *ChosenEntry = NULL;
    UINTN             Index = 0;
    // Add the entry for secure boot policy
    SecureBootPolicyEntry.Title = PoolPrint(L"Secure boot policy: %s", SecureBootPolicyToStr(gSettings.SecureBootPolicy));
    if (SecureBootPolicyEntry.Title == NULL) {
      break;
    }
    SecureBootPolicyMenu.Title = SecureBootPolicyEntry.Title;
    SecureBootMenu.Entries[Index++] = &SecureBootPolicyEntry;
    // Get the proper entries for the secure boot mode
    if (!gSettings.SecureBootSetupMode) {
      SecureBootMenu.Entries[Index++] = &InsertImageSignatureEntry;
      SecureBootMenu.Entries[Index++] = &RemoveImageSignatureEntry;
      SecureBootMenu.Entries[Index++] = &ClearImageSignatureEntry;
      SecureBootMenu.Entries[Index++] = &DisableSecureBootEntry;
    }
    SecureBootMenu.Entries[Index++] = &MenuEntryReturn;
    SecureBootMenu.EntryCount = Index;
    // Run the configuration menu
    if ((RunMenu(&SecureBootMenu, &ChosenEntry) == MENU_EXIT_ENTER) &&
        (ChosenEntry != NULL)) {
      switch (ChosenEntry->Tag) {
      case TAG_POLICY:
        // Change the secure boot policy
        ChosenEntry = NULL;
        if ((RunMenu(&SecureBootPolicyMenu, &ChosenEntry) == MENU_EXIT_ENTER) &&
            (ChosenEntry != NULL)) {
          switch (ChosenEntry->Tag) {
          case SECURE_BOOT_POLICY_DENY:
          case SECURE_BOOT_POLICY_ALLOW:
          case SECURE_BOOT_POLICY_QUERY:
          case SECURE_BOOT_POLICY_INSERT:
          case SECURE_BOOT_POLICY_WHITELIST:
          case SECURE_BOOT_POLICY_BLACKLIST:
          case SECURE_BOOT_POLICY_USER:
             // Set a new policy
             gSettings.SecureBootPolicy = (UINT8)ChosenEntry->Tag;
             DBG("User changed secure boot policy: %s\n", SecureBootPolicyToStr(gSettings.SecureBootPolicy));
             break;

          default:
             break;
          }
        }
        break;

      case TAG_INSERT:
        // TODO: Insert authentication
        break;

      case TAG_REMOVE:
        // TODO: Remove authentication
        break;

      case TAG_CLEAR:
        // Clear authentication database
        if (YesNoMessage(L"Clear Authentication Database", L"Are you sure you want to clear the authentication database?")) {
          DBG("User clear authentication database\n");
          AlertMessage(L"Clear Authentication Database",
                       EFI_ERROR(ClearImageSignatureDatabase()) ?
                         L"Clearing authentication database failed!" :
                         L"Authentication database was cleared successfully");
        }
        break;

      case TAG_DISABLE:
        // Disable secure boot
        if (YesNoMessage(L"Disable Secure Boot", L"Are you sure you want to disable secure boot?")) {
          DBG("User disabled secure boot\n");
          DisableSecureBoot();
          if (!gSettings.SecureBoot) {
            return TRUE;
          }
          AlertMessage(L"Disable Secure Boot", L"Disabling secure boot failed!\nClover does not appear to own the PK");
        }
        break;

      default:
        StillConfiguring = FALSE;
        break;
      }
    } else {
      StillConfiguring = FALSE;
    }
    FreePool(SecureBootPolicyEntry.Title);
  } while (StillConfiguring);
  return FALSE;
}
