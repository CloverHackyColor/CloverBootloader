/*
 * refit/scan/common.c
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
#include "../refit/menu.h"

#ifndef DEBUG_ALL
#define DEBUG_COMMON_MENU 1
#else
#define DEBUG_COMMON_MENU DEBUG_ALL
#endif

#if DEBUG_COMMON_MENU == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_COMMON_MENU, __VA_ARGS__)
#endif

static CONST CHAR16 *BuiltinIconNames[] = {
  /*
   L"About",
   L"Options",
   L"Clover",
   L"Reset",
   L"Shutdown",
   L"Help",
   L"Shell",
   L"Part",
   L"Rescue",
   L"Pointer",
   */
  L"Internal",
  L"External",
  L"Optical",
  L"FireWire",
  L"Boot",
  L"HFS",
  L"APFS",
  L"NTFS",
  L"EXT",
  L"Recovery",
};
static const UINTN BuiltinIconNamesCount = (sizeof(BuiltinIconNames) / sizeof(CHAR16 *));

EG_IMAGE *LoadBuiltinIcon(IN CONST CHAR16 *IconName)
{
  UINTN Index = 0;
  if (IconName == NULL) {
    return NULL;
  }
  while (Index < BuiltinIconNamesCount) {
    if (StriCmp(IconName, BuiltinIconNames[Index]) == 0) {
#if USE_XTHEME
      XImage IconX = ThemeX.GetIcon(BUILTIN_ICON_VOL_INTERNAL + Index);
      return IconX.ToEGImage();
#else
      return BuiltinIcon(BUILTIN_ICON_VOL_INTERNAL + Index);
#endif
    }
    ++Index;
  }
  return NULL;
}

EG_IMAGE* ScanVolumeDefaultIcon(REFIT_VOLUME *Volume, IN UINT8 OSType, IN EFI_DEVICE_PATH_PROTOCOL *DevicePath) //IN UINT8 DiskKind)
{
  UINTN IconNum = 0;
#if USE_XTHEME
  XImage IconX;
#endif
  // default volume icon based on disk kind
  switch (Volume->DiskKind) {
    case DISK_KIND_INTERNAL:
      switch (OSType) {
        case OSTYPE_OSX:
        case OSTYPE_OSX_INSTALLER:
          while (!IsDevicePathEndType(DevicePath) &&
            !(DevicePathType(DevicePath) == MEDIA_DEVICE_PATH && DevicePathSubType (DevicePath) == MEDIA_VENDOR_DP)) {
            DevicePath = NextDevicePathNode(DevicePath);
          }
          if (DevicePathType(DevicePath) == MEDIA_DEVICE_PATH && DevicePathSubType (DevicePath) == MEDIA_VENDOR_DP) {
            if (StriCmp(GuidLEToStr((EFI_GUID *)((UINT8 *)DevicePath+0x04)),GuidLEToStr(&APFSSignature)) == 0 ) {
              IconNum = BUILTIN_ICON_VOL_INTERNAL_APFS;
            }
          } else {
            IconNum = BUILTIN_ICON_VOL_INTERNAL_HFS;
          }
          break;
        case OSTYPE_RECOVERY:
          IconNum = BUILTIN_ICON_VOL_INTERNAL_REC;
          break;
        case OSTYPE_LIN:
        case OSTYPE_LINEFI:
          IconNum = BUILTIN_ICON_VOL_INTERNAL_EXT3;
          break;
        case OSTYPE_WIN:
        case OSTYPE_WINEFI:
          IconNum = BUILTIN_ICON_VOL_INTERNAL_NTFS;
          break;
        default:
          IconNum = BUILTIN_ICON_VOL_INTERNAL;
          break;
      }
#if USE_XTHEME
      IconX = ThemeX.GetIcon(IconNum);
      break;
    case DISK_KIND_EXTERNAL:
      IconX = ThemeX.GetIcon(BUILTIN_ICON_VOL_EXTERNAL);
      break;
    case DISK_KIND_OPTICAL:
      IconX = ThemeX.GetIcon(BUILTIN_ICON_VOL_OPTICAL);
      break;
    case DISK_KIND_FIREWIRE:
      IconX = ThemeX.GetIcon(BUILTIN_ICON_VOL_FIREWIRE);
      break;
    case DISK_KIND_BOOTER:
      IconX = ThemeX.GetIcon(BUILTIN_ICON_VOL_BOOTER);
      break;
    default:
      return NULL;
  }
  return IconX.ToEGImage();
#else
      return BuiltinIcon(IconNum);
    case DISK_KIND_EXTERNAL:
      return BuiltinIcon(BUILTIN_ICON_VOL_EXTERNAL);
    case DISK_KIND_OPTICAL:
      return BuiltinIcon(BUILTIN_ICON_VOL_OPTICAL);
    case DISK_KIND_FIREWIRE:
      return BuiltinIcon(BUILTIN_ICON_VOL_FIREWIRE);
    case DISK_KIND_BOOTER:
      return BuiltinIcon(BUILTIN_ICON_VOL_BOOTER);
    default:
      break;
  }
  return NULL;
#endif
}


CHAR16 *AddLoadOption(IN CONST CHAR16 *LoadOptions, IN CONST CHAR16 *LoadOption)
{
  // If either option strings are null nothing to do
  if (LoadOptions == NULL)
  {
    if (LoadOption == NULL) return NULL;
    // Duplicate original options as nothing to add
    return EfiStrDuplicate(LoadOption);
  }
  // If there is no option or it is already present duplicate original
  // with XStringW it will be replaced by if (LoadOptions.ExistIn(LoadOption))
  else if ((LoadOption == NULL) || StrStr(LoadOptions, LoadOption))
    return EfiStrDuplicate(LoadOptions);
  // Otherwise add option
  return PoolPrint(L"%s %s", LoadOptions, LoadOption); //LoadOptions + LoadOption
}

CHAR16 *RemoveLoadOption(IN CONST CHAR16 *LoadOptions, IN CONST CHAR16 *LoadOption)
{
  CONST CHAR16 *Placement;
  CHAR16 *NewLoadOptions;
  UINTN   Length, Offset, OptionLength;

  //DBG("LoadOptions: '%ls', remove LoadOption: '%ls'\n", LoadOptions, LoadOption);
  // If there are no options then nothing to do
  if (LoadOptions == NULL) return NULL;
  // If there is no option to remove then duplicate original
  if (LoadOption == NULL) return EfiStrDuplicate(LoadOptions);
  // If not present duplicate original
  Placement = StrStr(LoadOptions, LoadOption);
  if (Placement == NULL) return EfiStrDuplicate(LoadOptions);

  // Get placement of option in original options
  Offset = (Placement - LoadOptions);
  Length = StrLen(LoadOptions);
  OptionLength = StrLen(LoadOption);

  // If this is just part of some larger option (contains non-space at the beginning or end)
  if ((Offset > 0 && LoadOptions[Offset - 1] != L' ') ||
      ((Offset + OptionLength) < Length && LoadOptions[Offset + OptionLength] != L' ')) {
    return EfiStrDuplicate(LoadOptions);
  }

  // Consume preceeding spaces
  while (Offset > 0 && LoadOptions[Offset - 1] == L' ') {
    OptionLength++;
    Offset--;
  }

  // Consume following spaces
  while (LoadOptions[Offset + OptionLength] == L' ') {
   OptionLength++;
  }

  // If it's the whole string return NULL
  if (OptionLength == Length) return NULL;

  if (Offset == 0) {
    // Simple case - we just need substring after OptionLength position
    NewLoadOptions = EfiStrDuplicate(LoadOptions + OptionLength);
  } else {
    // The rest of LoadOptions is Length - OptionLength, but we may need additional space and ending 0
    NewLoadOptions = (__typeof__(NewLoadOptions))AllocateZeroPool((Length - OptionLength + 2) * sizeof(CHAR16));
    // Copy preceeding substring
    CopyMem(NewLoadOptions, LoadOptions, Offset * sizeof(CHAR16));
    if ((Offset + OptionLength) < Length) {
      // Copy following substring, but include one space also
      OptionLength--;
      CopyMem(NewLoadOptions + Offset, LoadOptions + Offset + OptionLength, (Length - OptionLength - Offset) * sizeof(CHAR16));
    }
  }
  return NewLoadOptions;
}

#define TO_LOWER(ch) (((ch >= L'A') && (ch <= L'Z')) ? ((ch - L'A') + L'a') : ch)
INTN StrniCmp(IN CONST CHAR16 *Str1,
              IN CONST CHAR16 *Str2,
              IN UINTN   Count)
{
  CHAR16 Ch1, Ch2;
  if (Count == 0) {
    return 0;
  }
  if (Str1 == NULL) {
    if (Str2 == NULL) {
      return 0;
    } else {
      return -1;
    }
  } else  if (Str2 == NULL) {
    return 1;
  }
  do {
    Ch1 = TO_LOWER(*Str1);
    Ch2 = TO_LOWER(*Str2);
    Str1++;
    Str2++;
    if (Ch1 != Ch2) {
      return (Ch1 - Ch2);
    }
    if (Ch1 == 0) {
      return 0;
    }
  } while (--Count > 0);
  return 0;
}

CONST CHAR16 *StriStr(IN CONST CHAR16 *Str,
                IN CONST CHAR16 *SearchFor)
{
  CONST CHAR16 *End;
  UINTN   Length;
  UINTN   SearchLength;
  if ((Str == NULL) || (SearchFor == NULL)) {
    return NULL;
  }
  Length = StrLen(Str);
  if (Length == 0) {
    return NULL;
  }
  SearchLength = StrLen(SearchFor);
  if (SearchLength > Length) {
    return NULL;
  }
  End = Str + (Length - SearchLength) + 1;
  while (Str < End) {
    if (StrniCmp(Str, SearchFor, SearchLength) == 0) {
      return Str;
    }
    ++Str;
  }
  return NULL;
}

VOID StrToLower(IN CHAR16 *Str)
{
   while (*Str) {
     *Str = TO_LOWER(*Str);
     ++Str;
   }
}

// TODO remove that and AlertMessage with a printf-like format
#if USE_XTHEME
STATIC void CreateInfoLines(IN CONST XStringW& Message, OUT XStringWArray* Information)
{
  if (Message.isEmpty()) {
    return;
  }
  Information->Empty();
  //TODO will fill later
}
#else
STATIC void CreateInfoLines(IN CONST CHAR16 *Message, OUT XStringWArray* Information)
{
  CONST CHAR16 *Ptr;
//  CHAR16 **Information;
  UINTN   Index = 0, Total = 0;
  UINTN   Length = ((Message == NULL) ? 0 : StrLen(Message));
  // Check parameters
  if (Length == 0) {
    return;
  }
  // Count how many new lines
  Ptr = Message - 1;
  while (Ptr != NULL) {
    ++Total;
    Ptr = StrStr(++Ptr, L"\n");
  }
//  // Create information
//  Information = (CHAR16 **)AllocatePool((Total * sizeof(CHAR16 *)) + ((Length + 1) * sizeof(CHAR16)));
//  if (Information == NULL) {
//    return NULL;
//  }
  Information->Empty();
  // Copy strings
  CHAR16* Ptr2 = NULL; // VS2017 complains about uninitialized var.
//  CHAR16* Ptr2 = Information[Index++] = (CHAR16 *)(Information + Total);
//  StrCpyS(Ptr2, Length + 1, Message);
  while ((Index < Total) &&
         ((Ptr2 = (CHAR16*)StrStr(Ptr2, L"\n")) != NULL)) { // cast is ok because FilePath is not const, and we know that StrStr returns a pointer in FilePath. Will disappear when using a string object instead of CHAR16*
    *Ptr2++ = 0;
    XStringW* s = new XStringW;
    s->takeValueFrom(Ptr2);
    Information->AddReference(s, true);
//    Information[Index++] = Ptr2;
  }
//  // Return the info lines
//  if (Count != NULL) {
//    *Count = Total;
//  }
//  return Information;
}
#endif
extern REFIT_MENU_ITEM_RETURN MenuEntryReturn;


// it is not good to use Options menu style for messages and one line dialogs
// it can be a semitransparent rectangular at the screen centre as it was in Clover v1.0
#if USE_XTHEME
STATIC REFIT_MENU_SCREEN  AlertMessageMenu(0, XStringW(), XStringW(), &MenuEntryReturn, NULL);
VOID AlertMessage(IN XStringW& Title, IN CONST XStringW& Message)
{
  CreateInfoLines(Message, &AlertMessageMenu.InfoLines);
  AlertMessageMenu.Title = Title;
  AlertMessageMenu.RunMenu(NULL);
  AlertMessageMenu.InfoLines.Empty();
}
#else
STATIC REFIT_MENU_SCREEN  AlertMessageMenu(0, NULL, NULL, &MenuEntryReturn, NULL);

// Display an alert message
VOID AlertMessage(IN CONST CHAR16 *Title, IN CONST CHAR16 *Message)
{
//  UINTN              Count = 0;
  // Break message into info lines
//  CHAR16           **Information = CreateInfoLines(Message, &Count);
  CreateInfoLines(Message, &AlertMessageMenu.InfoLines);
    AlertMessageMenu.Title = EfiStrDuplicate(Title);
    AlertMessageMenu.RunMenu(NULL);
//  // Check parameters
//  if (Information != NULL) {
//    if (Count > 0) {
//      // Display the alert message
//      AlertMessageMenu.InfoLineCount = Count;
//      AlertMessageMenu.InfoLines = (CONST CHAR16**)Information;
//      AlertMessageMenu.Title = Title;
//      RunMenu(&AlertMessageMenu, NULL);
//    }
//    FreePool(Information);
//  }
  AlertMessageMenu.InfoLines.Empty();
}
#endif

#define TAG_YES 1
#define TAG_NO  2

//REFIT_SIMPLE_MENU_ENTRY_TAG(CONST CHAR16 *Title_, UINTN Tag_, ACTION AtClick_)
STATIC REFIT_SIMPLE_MENU_ENTRY_TAG   YesMessageEntry(XStringW().takeValueFrom(L"Yes"), TAG_YES, ActionEnter);
STATIC REFIT_SIMPLE_MENU_ENTRY_TAG   NoMessageEntry(XStringW().takeValueFrom(L"No"), TAG_NO, ActionEnter);

//REFIT_MENU_SCREEN(UINTN ID, CONST CHAR16* Title, CONST CHAR16* TimeoutText, REFIT_ABSTRACT_MENU_ENTRY* entry1, REFIT_ABSTRACT_MENU_ENTRY* entry2)
#if USE_XTHEME
STATIC REFIT_MENU_SCREEN  YesNoMessageMenu(0, XStringW(), XStringW(), &YesMessageEntry, &NoMessageEntry);
#else
STATIC REFIT_MENU_SCREEN  YesNoMessageMenu(0, NULL, NULL, &YesMessageEntry, &NoMessageEntry);
#endif
// Display a yes/no prompt
#if USE_XTHEME
BOOLEAN YesNoMessage(IN XStringW& Title, IN CONST XStringW& Message)
{
  BOOLEAN            Result = FALSE;
  UINTN              MenuExit;
  CreateInfoLines(Message, &YesNoMessageMenu.InfoLines);
  YesNoMessageMenu.Title = Title;
  do
  {
    REFIT_ABSTRACT_MENU_ENTRY  *ChosenEntry = NULL;
    MenuExit = YesNoMessageMenu.RunMenu(&ChosenEntry);
    if ( ChosenEntry != NULL  &&  ChosenEntry->getREFIT_SIMPLE_MENU_ENTRY_TAG()  &&  ChosenEntry->getREFIT_SIMPLE_MENU_ENTRY_TAG()->Tag == TAG_YES  &&
        ((MenuExit == MENU_EXIT_ENTER) || (MenuExit == MENU_EXIT_DETAILS))) {
      Result = TRUE;
      MenuExit = MENU_EXIT_ENTER;
    }
  } while (MenuExit != MENU_EXIT_ENTER);
  YesNoMessageMenu.InfoLines.Empty();
  return Result;
}
#else
BOOLEAN YesNoMessage(IN CHAR16 *Title, IN CONST CHAR16 *Message)
{
  BOOLEAN            Result = FALSE;
  UINTN              /*Count = 0,*/ MenuExit;
  // Break message into info lines
//  CHAR16           **Information = CreateInfoLines(Message, &Count);
  CreateInfoLines(Message, &YesNoMessageMenu.InfoLines);
  // Display the yes/no message
//  YesNoMessageMenu.InfoLineCount = Count;
//  YesNoMessageMenu.InfoLines = (CONST CHAR16**)Information;
  YesNoMessageMenu.Title = Title;
  do
  {
     REFIT_ABSTRACT_MENU_ENTRY  *ChosenEntry = NULL;
     MenuExit = YesNoMessageMenu.RunMenu(&ChosenEntry);
     if ( ChosenEntry != NULL  &&  ChosenEntry->getREFIT_SIMPLE_MENU_ENTRY_TAG()  &&  ChosenEntry->getREFIT_SIMPLE_MENU_ENTRY_TAG()->Tag == TAG_YES  &&
         ((MenuExit == MENU_EXIT_ENTER) || (MenuExit == MENU_EXIT_DETAILS))) {
       Result = TRUE;
       MenuExit = MENU_EXIT_ENTER;
     }
  } while (MenuExit != MENU_EXIT_ENTER);
  YesNoMessageMenu.InfoLines.Empty();
//  if (Information != NULL) {
//    FreePool(Information);
//  }
  return Result;
}
#endif
// Ask user for file path from directory menu
BOOLEAN AskUserForFilePathFromDir(IN CHAR16 *Title OPTIONAL, IN REFIT_VOLUME *Volume,
                                  IN CHAR16 *ParentPath OPTIONAL, IN EFI_FILE *Dir,
                                  OUT EFI_DEVICE_PATH_PROTOCOL **Result)
{
  //REFIT_MENU_SCREEN   Menu = { 0, L"Please Select File...", NULL, 0, NULL, 0, NULL,
  //                             0, NULL, FALSE, FALSE, 0, 0, 0, 0, { 0, 0, 0, 0 }, NULL};
  // Check parameters
  if ((Volume == NULL) || (Dir == NULL) || (Result == NULL)) {
    return FALSE;
  }
  // TODO: Generate directory menu
  return FALSE;
}

#define TAG_OFFSET 1000

//STATIC REFIT_MENU_SCREEN InitialMenu = {0, L"Please Select File...", NULL, 0, NULL,
//  0, NULL, NULL, FALSE, FALSE, 0, 0, 0, 0,
//  { 0, 0, 0, 0 }, NULL};
#if USE_XTHEME
STATIC REFIT_MENU_SCREEN  InitialMenu(0, L"Please Select File..."_XSW, XStringW());
#else
STATIC REFIT_MENU_SCREEN InitialMenu(0, L"Please Select File...", NULL);
#endif
// Ask user for file path from volumes menu
BOOLEAN AskUserForFilePathFromVolumes(IN CHAR16 *Title OPTIONAL, OUT EFI_DEVICE_PATH_PROTOCOL **Result)
{
  REFIT_MENU_SCREEN   Menu = InitialMenu;
//  REFIT_MENU_ENTRY  **Entries;
//  REFIT_MENU_ENTRY   *EntryPtr;
  UINTN               Index = 0, /*Count = 0,*/ MenuExit;
  BOOLEAN             Responded = FALSE;
  if (Result == NULL) {
    return FALSE;
  }
  // Allocate entries
//  Entries = (REFIT_MENU_ENTRY **)AllocateZeroPool(sizeof(REFIT_MENU_ENTRY *) + ((sizeof(REFIT_MENU_ENTRY *) + sizeof(REFIT_MENU_ENTRY)) * Volumes.size()));
//  if (Entries == NULL) {
//    return FALSE;
//  }
//  EntryPtr = (REFIT_MENU_ENTRY *)(Entries + (Volumes.size() + 1));
  // Create volume entries
  for (Index = 0; Index < Volumes.size(); ++Index) {
    REFIT_VOLUME     *Volume = &Volumes[Index];
    if ((Volume == NULL) || (Volume->RootDir == NULL) ||
        ((Volume->DevicePathString == NULL) && (Volume->VolName == NULL))) {
      continue;
    }
	  REFIT_SIMPLE_MENU_ENTRY_TAG *Entry = new REFIT_SIMPLE_MENU_ENTRY_TAG(SWPrintf("%ls", (Volume->VolName == NULL) ? Volume->DevicePathString : Volume->VolName), TAG_OFFSET + Index, MENU_EXIT_ENTER);
//    Entry = Entries[Count++] = EntryPtr++;
//    Entry->Title = (Volume->VolName == NULL) ? Volume->DevicePathString : Volume->VolName;
//    Entry->Tag = TAG_OFFSET + Index;
//    Entry->AtClick = MENU_EXIT_ENTER;
    Menu.Entries.AddReference(Entry, true);
  }
  // Setup menu
//  CopyMem(&Menu, &InitialMenu, sizeof(REFIT_MENU_SCREEN));
//  Entries[Count++] = &MenuEntryReturn;
  Menu.Entries.AddReference(&MenuEntryReturn, false);
//  Menu.Entries.size() = Count;
//  Menu.Entries = Entries;
#if USE_XTHEME
  Menu.Title.takeValueFrom(Title);
#else
  Menu.Title = Title;
#endif
  do
  {
    REFIT_ABSTRACT_MENU_ENTRY *ChosenEntry = NULL;
    // Run the volume chooser menu
    MenuExit = Menu.RunMenu(&ChosenEntry);
    if ((ChosenEntry != NULL) && ChosenEntry->getREFIT_SIMPLE_MENU_ENTRY_TAG() &&
        ((MenuExit == MENU_EXIT_ENTER) || (MenuExit == MENU_EXIT_DETAILS))) {
      if (ChosenEntry->getREFIT_SIMPLE_MENU_ENTRY_TAG()->Tag >= TAG_OFFSET) {
        Index = (ChosenEntry->getREFIT_SIMPLE_MENU_ENTRY_TAG()->Tag - TAG_OFFSET);
        if (Index < Volumes.size()) {
          // Run directory chooser menu
          if (!AskUserForFilePathFromDir(Title, &Volumes[Index], NULL, Volumes[Index].RootDir, Result)) {
            continue;
          }
          Responded = TRUE;
        }
      }
      break;
    }
  } while (MenuExit != MENU_EXIT_ESCAPE);
//  FreePool(Entries);
  return Responded;
}

// Ask user for file path
BOOLEAN AskUserForFilePath(IN CHAR16 *Title OPTIONAL, IN EFI_DEVICE_PATH_PROTOCOL *Root OPTIONAL, OUT EFI_DEVICE_PATH_PROTOCOL **Result)
{
  EFI_FILE *Dir = NULL;
  if (Result == NULL) {
    return FALSE;
  }
  if (Root != NULL) {
    // Get the file path
    CHAR16 *DevicePathStr = FileDevicePathToStr(Root);
    if (DevicePathStr != NULL) {
      UINTN Index = 0;
      // Check the volumes for a match
      for (Index = 0; Index < Volumes.size(); ++Index) {
        REFIT_VOLUME *Volume = &Volumes[Index];
        UINTN         Length;
        if ((Volume == NULL) || (Volume->RootDir == NULL) ||
            (Volume->DevicePathString == NULL)) {
          continue;
        }
        Length = StrLen(Volume->DevicePathString);
        if (Length == 0) {
          continue;
        }
        // If the path begins with this volumes path it matches
        if (StrniCmp(DevicePathStr, Volume->DevicePathString, Length)) {
          // Need to
          CHAR16 *FilePath = DevicePathStr + Length;
          UINTN   FileLength = StrLen(FilePath);
          if (FileLength == 0) {
            // If there is no path left then open the root
            return AskUserForFilePathFromDir(Title, Volume, NULL, Volume->RootDir, Result);
          } else {
            // Check to make sure this is directory
            if (!EFI_ERROR(Volume->RootDir->Open(Volume->RootDir, &Dir, FilePath, EFI_FILE_MODE_READ, 0)) &&
                (Dir != NULL)) {
              // Get file information
              EFI_FILE_INFO *Info = EfiLibFileInfo(Dir);
              if (Info != NULL) {
                // Check if the file is a directory
                if ((Info->Attribute & EFI_FILE_DIRECTORY) == 0) {
                  // Return the passed device path if it is a file
                  FreePool(Info);
                  Dir->Close(Dir);
                  *Result = Root;
                  return TRUE;
                } else {
                  // Ask user other wise
                  BOOLEAN Success = AskUserForFilePathFromDir(Title, Volume, FilePath, Dir, Result);
                  FreePool(Info);
                  Dir->Close(Dir);
                  return Success;
                }
                //FreePool(Info);
              }
              Dir->Close(Dir);
            }
          }
        }
      }
      FreePool(DevicePathStr);
    }
  }
  return AskUserForFilePathFromVolumes(Title, Result);
}
