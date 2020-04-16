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

//CONST INTN SecureMenuDef = 0; // jief : not used ??

#ifdef ENABLE_SECURE_BOOT

#include "entry_scan.h"

#include <Guid/ImageAuthentication.h>

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
//  Entry = (__typeof__(Entry))AllocateZeroPool(sizeof(LOADER_ENTRY));
  if (gSettings.SecureBoot) {
    Entry = new REFIT_MENU_ENTRY_SECURE_BOOT();
    Entry->Title.SPrintf("Clover Secure Boot Configuration");
//    Entry->Tag = TAG_SECURE_BOOT_CONFIG;
    Entry->Image = ThemeX.GetIcon(BUILTIN_ICON_FUNC_SECURE_BOOT_CONFIG);

  } else {
    Entry = new REFIT_MENU_ENTRY_SECURE_BOOT_CONFIG();
    Entry->Title.SPrintf("Enable Clover Secure Boot");
//    Entry->Tag = TAG_SECURE_BOOT;
    Entry->Image = ThemeX.GetIcon(BUILTIN_ICON_FUNC_SECURE_BOOT);
  }
  Entry->Row = 1;
  //actions
  Entry->AtClick = ActionSelect;
  Entry->AtDoubleClick = ActionEnter;
  Entry->AtRightClick = ActionHelp;
  MainMenu.AddMenuEntry(Entry);
}


STATIC REFIT_MENU_ENTRY   QueryEntry[] = {
  { L"Deny authentication"_XSW, SECURE_BOOT_POLICY_DENY, 0, 0, 0, NULL, NULL, NULL, {0, 0, 0, 0}, ActionEnter, ActionNone, ActionNone, ActionNone, NULL },
  { L"Allow authentication"_XSW, SECURE_BOOT_POLICY_ALLOW, 0, 0, 0, NULL, NULL, NULL, {0, 0, 0, 0}, ActionEnter, ActionNone, ActionNone, ActionNone, NULL },
  { L"Insert authentication into database"_XSW, SECURE_BOOT_POLICY_INSERT, 0, 0, 0, NULL, NULL, NULL, {0, 0, 0, 0}, ActionEnter, ActionNone, ActionNone, ActionNone, NULL },
};
STATIC REFIT_MENU_ENTRY  *QueryEntries[] = { QueryEntry, QueryEntry + 1, QueryEntry + 2 };
STATIC REFIT_MENU_SCREEN  QueryUserMenu = { 0, L"Secure Boot Authentication"_XSW, L""_XSW, 3, NULL, 2, QueryEntries,
  0, NULL, NULL, FALSE, FALSE, 0, 0, 0, 0,
  /*  FILM_CENTRE, FILM_CENTRE,*/ { 0, 0, 0, 0 }, NULL };

// Query the secure boot user what to do with image
UINTN QuerySecureBootUser(IN CONST EFI_DEVICE_PATH_PROTOCOL *DevicePath)
{
  UINTN Response = SECURE_BOOT_POLICY_DENY;
  // Check parameters
  if (DevicePath != NULL) {
    // Get the device path string
    CHAR16 *Information[] = { L"Please select the authentication action for", NULL, NULL };
    Information[1] = FileDevicePathToStr((EFI_DEVICE_PATH_PROTOCOL *)DevicePath);
    if (Information[1] != NULL) {
      // Get the device path file path
      Information[2] = FileDevicePathFileToStr((EFI_DEVICE_PATH_PROTOCOL *)DevicePath);
      if (Information[2] != NULL) {
        // Create the entries
        REFIT_MENU_ENTRY  *ChosenEntry = NULL;
        UINTN              MenuExit;
        // Update the menu
        QueryUserMenu.InfoLines = Information;
        QueryUserMenu.Entries.size() = gSettings.SecureBootSetupMode ? 2 : 3;
        // Debug message
        DBG("VerifySecureBootImage: Query user for authentication action for %ls\n", Information[1]);
        // Because we may
        if (!gGuiIsReady) {
          InitScreen(FALSE);
          if (gThemeNeedInit) {
            InitTheme(TRUE, NULL);
            gThemeNeedInit = FALSE;
          }
          gGuiIsReady = TRUE;
        }
        // Run the query menu
        do
        {
           MenuExit = QueryUserMenu.RunMenu(&ChosenEntry);
           if ((ChosenEntry != NULL) &&
               ((MenuExit == MENU_EXIT_ENTER) || (MenuExit == MENU_EXIT_DETAILS))) {
             Response = (UINTN)ChosenEntry->Tag;
             MenuExit = MENU_EXIT_ESCAPE;
           }
        } while (MenuExit != MENU_EXIT_ESCAPE);
        FreePool(Information[2]);
      }
      FreePool(Information[1]);
    }
  }
  return Response;
}

// Find a device path's signature list
STATIC VOID *FindImageSignatureDatabase(IN  CONST EFI_DEVICE_PATH_PROTOCOL *DevicePath,
                                        OUT UINTN                          *DatabaseSize)
{
  EFI_IMAGE_EXECUTION_INFO_TABLE  *ImageExeInfoTable = NULL;
  EFI_IMAGE_EXECUTION_INFO        *ImageExeInfo;
  CHAR16                          *FDP;
  UINT8                           *Ptr;
  UINTN                            Index;
  // Check parameters
  if (DatabaseSize == NULL) {
    return NULL;
  }
  *DatabaseSize = 0;
  if (DevicePath == NULL) {
    return NULL;
  }
  // Get the execution information table
  if (EFI_ERROR(EfiGetSystemConfigurationTable(&gEfiImageSecurityDatabaseGuid, (VOID **)&ImageExeInfoTable)) ||
      (ImageExeInfoTable == NULL)) {
    return NULL;
  }
  // Get device path string
  FDP = FileDevicePathToStr((EFI_DEVICE_PATH_PROTOCOL *)DevicePath);
  if (FDP == NULL) {
    return NULL;
  }
  // Get the execution information
  Ptr = (UINT8 *)ImageExeInfoTable;
  Ptr += sizeof(EFI_IMAGE_EXECUTION_INFO_TABLE);
  // Traverse the execution information table
  ImageExeInfo = (EFI_IMAGE_EXECUTION_INFO *)Ptr;
  for (Index = 0; Index < ImageExeInfoTable->NumberOfImages; ++Index, Ptr += ImageExeInfo->InfoSize) {
    UINT8  *Offset = Ptr + OFFSET_OF(EFI_IMAGE_EXECUTION_INFO, InfoSize) + sizeof(ImageExeInfo->InfoSize);
    CHAR16 *Name = (CHAR16 *)Offset;
    // Check to make sure this is valid
    if ((ImageExeInfo->Action == EFI_IMAGE_EXECUTION_AUTH_SIG_FAILED) ||
        (ImageExeInfo->Action == EFI_IMAGE_EXECUTION_AUTH_SIG_FOUND)) {
      continue;
    }
    // Skip the name
    do
    {
      Offset += sizeof(CHAR16);
    } while (*Name++);
    // Compare the device paths
    Name = FileDevicePathToStr((EFI_DEVICE_PATH_PROTOCOL *)Offset);
    if (Name) {
      if (StrCmp(FDP, Name) == 0) {
        // Get the signature list and size
        Offset += GetDevicePathSize((EFI_DEVICE_PATH_PROTOCOL *)Offset);
        *DatabaseSize = (ImageExeInfo->InfoSize - (Offset - Ptr));
        FreePool(Name);
        FreePool(FDP);
        return Offset;
      }
      FreePool(Name);
    }
  }
  FreePool(FDP);
  // Not found
  return NULL;
}

// Insert secure boot image signature
EFI_STATUS AppendImageToAuthorizedDatabase(IN CONST EFI_DEVICE_PATH_PROTOCOL *DevicePath,
                                           IN VOID                           *FileBuffer,
                                           IN UINTN                           FileSize)
{
  EFI_STATUS  Status = EFI_INVALID_PARAMETER;
  CHAR16     *ErrorString = NULL;
  VOID       *Database = NULL;
  UINTN       DatabaseSize = 0;
  // Check that either the device path or the file buffer is valid
  if ((DevicePath == NULL) && ((FileBuffer == NULL) || (FileSize == 0))) {
    return EFI_INVALID_PARAMETER;
  }
  // Get the image signature
  Database = FindImageSignatureDatabase(DevicePath, &DatabaseSize);
  if (Database) {
    // Add the image signature to database
    Status = AppendImageDatabaseToAuthorizedDatabase(Database, DatabaseSize);
  } else if ((FileBuffer == NULL) || (FileSize == 0)) {
    // Load file by device path
    UINT32 AuthenticationStatus = 0;
    FileBuffer = GetFileBufferByFilePath(FALSE, DevicePath, &FileSize, &AuthenticationStatus);
    if (FileBuffer) {
      if (FileSize > 0) {
        // Create image signature
        Database = GetImageSignatureDatabase(FileBuffer, FileSize, &DatabaseSize, TRUE);
        if (Database) {
          // Add the image signature to database
          if (EFI_ERROR(Status = AppendImageDatabaseToAuthorizedDatabase(Database, DatabaseSize))) {
            ErrorString = L"Failed to insert image authentication";
          }
          FreePool(Database);
        } else {
          ErrorString = L"Image has no certificates or is not valid";
        }
      } else {
        ErrorString = L"Image has no certificates or is not valid";
      }
      FreePool(FileBuffer);
    } else {
      ErrorString = L"Failed to load the image";
    }
  } else {
    // Create image signature
    Database = GetImageSignatureDatabase(FileBuffer, FileSize, &DatabaseSize, TRUE);
    if (Database) {
      // Add the image signature to database
      if (EFI_ERROR(Status = AppendImageDatabaseToAuthorizedDatabase(Database, DatabaseSize))) {
        ErrorString = L"Failed to insert image authentication";
      }
      FreePool(Database);
    } else {
      ErrorString = L"Image has no certificates or is not valid";
    }
  }
  if (ErrorString != NULL) {
    CHAR16 *DevicePathStr = FileDevicePathToStr((EFI_DEVICE_PATH_PROTOCOL *)DevicePath);
    if (DevicePathStr != NULL) {
      CHAR16 *FileDevicePathStr = FileDevicePathFileToStr((EFI_DEVICE_PATH_PROTOCOL *)DevicePath);
      if (FileDevicePathStr != NULL) {
        CHAR16 *Str = PoolPrint(L"%s\n%s\n%s", ErrorString, DevicePathStr, FileDevicePathStr);
        if (Str != NULL) {
          AlertMessage(L"Insert Image Authentication", Str);
          FreePool(Str);
        }
        FreePool(FileDevicePathStr);
      } else {
        CHAR16 *Str = PoolPrint(L"%s\n%s", ErrorString, DevicePathStr);
        if (Str != NULL) {
          AlertMessage(L"Insert Image Authentication", Str);
          FreePool(Str);
        }
      }
      FreePool(DevicePathStr);
    } else {
      AlertMessage(L"Insert Image Authentication", ErrorString);
    }
  }
  return Status;
}

// Insert secure boot image signature
EFI_STATUS RemoveImageFromAuthorizedDatabase(IN CONST EFI_DEVICE_PATH_PROTOCOL *DevicePath,
                                             IN VOID                           *FileBuffer,
                                             IN UINTN                           FileSize)
{
  EFI_STATUS  Status = EFI_INVALID_PARAMETER;
  CHAR16     *ErrorString = NULL;
  VOID       *Database;
  UINTN       DatabaseSize = 0;
  // Check that either the device path or the file buffer is valid
  if (DevicePath == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  // Get the image signature
  Database = FindImageSignatureDatabase(DevicePath, &DatabaseSize);
  if (Database) {
    // Remove the image signature from database
    Status = RemoveImageDatabaseFromAuthorizedDatabase(Database, DatabaseSize);
  } else if ((FileBuffer == NULL) || (FileSize == 0)) {
    // Load file by device path
    UINT32 AuthenticationStatus = 0;
    FileBuffer = GetFileBufferByFilePath(FALSE, DevicePath, &FileSize, &AuthenticationStatus);
    if (FileBuffer) {
      if (FileSize > 0) {
        // Create image signature
        Database = GetImageSignatureDatabase(FileBuffer, FileSize, &DatabaseSize, TRUE);
        if (Database) {
          // Remove the image signature from database
          if (EFI_ERROR(Status = RemoveImageDatabaseFromAuthorizedDatabase(Database, DatabaseSize))) {
            ErrorString = L"Failed to remove image authentication";
          }
          FreePool(Database);
        } else {
          ErrorString = L"Image has no certificates or is not valid";
        }
      } else {
        ErrorString = L"Image has no certificates or is not valid";
      }
      FreePool(FileBuffer);
    } else {
      ErrorString = L"Failed to load the image";
    }
  } else {
    // Create image signature
    Database = GetImageSignatureDatabase(FileBuffer, FileSize, &DatabaseSize, TRUE);
    if (Database) {
      // Remove the image signature from database
      if (EFI_ERROR(Status = RemoveImageDatabaseFromAuthorizedDatabase(Database, DatabaseSize))) {
        ErrorString = L"Failed to remove image authentication";
      }
      FreePool(Database);
    }  else {
      ErrorString = L"Image has no certificates or is not valid";
    }
  }
  if (ErrorString != NULL) {
    CHAR16 *DevicePathStr = FileDevicePathToStr((EFI_DEVICE_PATH_PROTOCOL *)DevicePath);
    if (DevicePathStr != NULL) {
      CHAR16 *FileDevicePathStr = FileDevicePathFileToStr((EFI_DEVICE_PATH_PROTOCOL *)DevicePath);
      if (FileDevicePathStr != NULL) {
        CHAR16 *Str = PoolPrint(L"%s\n%s\n%s", ErrorString, DevicePathStr, FileDevicePathStr);
        if (Str != NULL) {
          AlertMessage(L"Remove Image Authentication", Str);
          FreePool(Str);
        }
        FreePool(FileDevicePathStr);
      } else {
        CHAR16 *Str = PoolPrint(L"%s\n%s", ErrorString, DevicePathStr);
        if (Str != NULL) {
          AlertMessage(L"Remove Image Authentication", Str);
          FreePool(Str);
        }
      }
      FreePool(DevicePathStr);
    } else {
      AlertMessage(L"Remove Image Authentication", ErrorString);
    }
  }
  return Status;
}

extern REFIT_MENU_ITEM_RETURN MenuEntryReturn;

#define TAG_POLICY  1
#define TAG_INSERT  2
#define TAG_REMOVE  3
#define TAG_CLEAR   4
#define TAG_DISABLE 5

STATIC REFIT_MENU_ENTRY   SecureBootPolicyEntry = { NULL, TAG_POLICY, 0, 0, 0, NULL,  NULL, NULL, { 0, 0, 0, 0 }, ActionEnter, ActionNone, ActionNone, ActionNone, NULL };
STATIC REFIT_MENU_ENTRY   InsertImageSignatureEntry = { L"Add image authentication to database", TAG_INSERT, 0, 0, 0, NULL, NULL, NULL, {0, 0, 0, 0}, ActionEnter, ActionNone, ActionNone, ActionNone, NULL };
STATIC REFIT_MENU_ENTRY   RemoveImageSignatureEntry = { L"Remove image authentication from database", TAG_REMOVE, 0, 0, 0, NULL, NULL, NULL, {0, 0, 0, 0}, ActionEnter, ActionNone, ActionNone, ActionNone, NULL };
STATIC REFIT_MENU_ENTRY   ClearImageSignatureEntry = { L"Clear image authentication database", TAG_CLEAR, 0, 0, 0, NULL, NULL, NULL, {0, 0, 0, 0}, ActionEnter, ActionNone, ActionNone, ActionNone, NULL };
STATIC REFIT_MENU_ENTRY   DisableSecureBootEntry = { L"Disable secure boot", TAG_DISABLE, 0, 0, 0, NULL, NULL, NULL, {0, 0, 0, 0}, ActionEnter, ActionNone, ActionNone, ActionNone, NULL };
STATIC REFIT_MENU_ENTRY  *SecureBootEntries[] = { NULL, NULL, NULL, NULL, NULL, NULL };
STATIC REFIT_MENU_SCREEN  SecureBootMenu = { 0, L"Secure Boot Configuration", NULL, 0, NULL, 0, SecureBootEntries,
                                             0, NULL, NULL, FALSE, FALSE, 0, 0, 0, 0,
                                        /*   FILM_CENTRE, FILM_CENTRE,*/ { 0, 0, 0, 0 }, NULL };

STATIC REFIT_MENU_ENTRY   SecureBootPolicyNameEntry[] = {
  { L"Deny", SECURE_BOOT_POLICY_DENY, 0, 0, 0, NULL, NULL, NULL, { 0, 0, 0, 0 }, ActionEnter, ActionNone, ActionNone, ActionNone, NULL },
  { L"Allow", SECURE_BOOT_POLICY_ALLOW, 0, 0, 0, NULL, NULL, NULL, { 0, 0, 0, 0 }, ActionEnter, ActionNone, ActionNone, ActionNone, NULL },
  { L"Query", SECURE_BOOT_POLICY_QUERY, 0, 0, 0, NULL, NULL, NULL, { 0, 0, 0, 0 }, ActionEnter, ActionNone, ActionNone, ActionNone, NULL },
  { L"Insert", SECURE_BOOT_POLICY_INSERT, 0, 0, 0, NULL, NULL, NULL, { 0, 0, 0, 0 }, ActionEnter, ActionNone, ActionNone, ActionNone, NULL },
  { L"WhiteList", SECURE_BOOT_POLICY_WHITELIST, 0, 0, 0, NULL, NULL, NULL, { 0, 0, 0, 0 }, ActionEnter, ActionNone, ActionNone, ActionNone, NULL },
  { L"BlackList", SECURE_BOOT_POLICY_BLACKLIST, 0, 0, 0, NULL, NULL, NULL, { 0, 0, 0, 0 }, ActionEnter, ActionNone, ActionNone, ActionNone, NULL },
  { L"User", SECURE_BOOT_POLICY_USER, 0, 0, 0, NULL, NULL, NULL, { 0, 0, 0, 0 }, ActionEnter, ActionNone, ActionNone, ActionNone, NULL },
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
                                                   0, NULL, NULL, FALSE, FALSE, 0, 0, 0, 0,
                                              /*    FILM_CENTRE, FILM_CENTRE,*/ { 0, 0, 0, 0 } , NULL };

// Configure secure boot
BOOLEAN ConfigureSecureBoot(VOID)
{
  BOOLEAN StillConfiguring = TRUE;
  do
  {
    UINTN             Index = 0, MenuExit;
    REFIT_MENU_ENTRY *ChosenEntry = NULL;
    EFI_DEVICE_PATH  *DevicePath = NULL;
    // Add the entry for secure boot policy
    SecureBootPolicyEntry.Title.SPrintf("Secure boot policy: %ls", SecureBootPolicyToStr(gSettings.SecureBootPolicy));
    if (SecureBootPolicyEntry.Title.isEmpty()) {
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
    SecureBootMenu.Entries.size() = Index;
    // Run the configuration menu
    MenuExit = SecureBootMenu.RunMenu(&ChosenEntry);
    if ((ChosenEntry != NULL) &&
        ((MenuExit == MENU_EXIT_ENTER) || (MenuExit == MENU_EXIT_DETAILS))) {
      switch (ChosenEntry->Tag) {
      case TAG_POLICY:
        // Change the secure boot policy
        do
        {
          ChosenEntry = NULL;
          MenuExit = SecureBootPolicyMenu.RunMenu(&ChosenEntry);
          if ((ChosenEntry != NULL) &&
              ((MenuExit == MENU_EXIT_ENTER) || (MenuExit == MENU_EXIT_DETAILS))) {
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
              DBG("User changed secure boot policy: %ls\n", SecureBootPolicyToStr(gSettings.SecureBootPolicy));

            default:
              MenuExit = MENU_EXIT_ESCAPE;
              break;
            }
          }
        } while (MenuExit != MENU_EXIT_ESCAPE);
        break;

      case TAG_INSERT:
        // Insert authentication
         if (AskUserForFilePathFromVolumes(L"Select Image to Insert Authentication...", &DevicePath) &&
             (DevicePath != NULL)) {
          AppendImageToAuthorizedDatabase(DevicePath, NULL, 0);
        }
        break;

      case TAG_REMOVE:
        // Remove authentication
        if (AskUserForFilePathFromVolumes(L"Select Image to Remove Authentication...", &DevicePath) &&
            (DevicePath != NULL)) {
          RemoveImageFromAuthorizedDatabase(DevicePath, NULL, 0);
        }
        break;

      case TAG_CLEAR:
        // Clear authentication database
        if (YesNoMessage(L"Clear Authentication Database", L"Are you sure you want to clear\nthe image authentication database?")) {
          DBG("User cleared authentication database\n");
          AlertMessage(L"Clear Authentication Database",
                       EFI_ERROR(ClearAuthorizedDatabase()) ?
                         L"Clearing the image authentication database failed!" :
                         L"Cleared image authentication database successfully");
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
    } else if (MenuExit == MENU_EXIT_ESCAPE) {
      StillConfiguring = FALSE;
    }
    FreePool(SecureBootPolicyEntry.Title);
  } while (StillConfiguring);
  return FALSE;
}

#endif // ENABLE_SECURE_BOOT
