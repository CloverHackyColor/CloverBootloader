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

static CHAR16 *BuiltinIconNames[] = {
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
  L"NTFS",
  L"EXT",
  L"Recovery",
};
static const UINTN BuiltinIconNamesCount = (sizeof(BuiltinIconNames) / sizeof(CHAR16 *));

EG_IMAGE *LoadBuiltinIcon(IN CHAR16 *IconName)
{
  UINTN Index = 0;
  if (IconName == NULL) {
    return NULL;
  }
  while (Index < BuiltinIconNamesCount) {
    if (StriCmp(IconName, BuiltinIconNames[Index]) == 0) {
      return BuiltinIcon(BUILTIN_ICON_VOL_INTERNAL + Index);
    }
    ++Index;
  }
  return NULL;
}

EG_IMAGE* ScanVolumeDefaultIcon(REFIT_VOLUME *Volume, IN UINT8 OSType) //IN UINT8 DiskKind)
{
  UINTN IconNum;
  // default volume icon based on disk kind
  switch (Volume->DiskKind) {
    case DISK_KIND_INTERNAL:
      switch (OSType) {
        case OSTYPE_OSX:
        case OSTYPE_OSX_INSTALLER:
          IconNum = BUILTIN_ICON_VOL_INTERNAL_HFS;
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
}

LOADER_ENTRY * DuplicateLoaderEntry(IN LOADER_ENTRY *Entry)
{
  LOADER_ENTRY *DuplicateEntry;
  if(Entry == NULL) {
    return NULL;
  }
  DuplicateEntry = AllocateZeroPool(sizeof(LOADER_ENTRY));
  if (DuplicateEntry) {
    DuplicateEntry->me.Tag          = Entry->me.Tag;
    DuplicateEntry->me.AtClick      = ActionEnter;
    DuplicateEntry->Volume          = Entry->Volume;
    DuplicateEntry->DevicePathString = EfiStrDuplicate(Entry->DevicePathString);
    DuplicateEntry->LoadOptions     = EfiStrDuplicate(Entry->LoadOptions);
    DuplicateEntry->LoaderPath      = EfiStrDuplicate(Entry->LoaderPath);
    DuplicateEntry->VolName         = EfiStrDuplicate(Entry->VolName);
    DuplicateEntry->DevicePath      = Entry->DevicePath;
    DuplicateEntry->Flags           = Entry->Flags;
    DuplicateEntry->LoaderType      = Entry->LoaderType;
    DuplicateEntry->OSVersion       = Entry->OSVersion;
  }
  return DuplicateEntry;
}

CHAR16 *AddLoadOption(IN CHAR16 *LoadOptions, IN CHAR16 *LoadOption)
{
  // If either option strings are null nothing to do
  if (LoadOptions == NULL)
  {
    if (LoadOption == NULL) return NULL;
    // Duplicate original options as nothing to add
    return EfiStrDuplicate(LoadOption);
  }
  // If there is no option or it is already present duplicate original
  else if ((LoadOption == NULL) || StrStr(LoadOptions, LoadOption)) return EfiStrDuplicate(LoadOptions);
  // Otherwise add option
  return PoolPrint(L"%s %s", LoadOptions, LoadOption);
}

CHAR16 *RemoveLoadOption(IN CHAR16 *LoadOptions, IN CHAR16 *LoadOption)
{
  CHAR16 *Placement;
  CHAR16 *NewLoadOptions;
  UINTN   Length, Offset, OptionLength;

  //DBG("LoadOptions: '%s', remove LoadOption: '%s'\n", LoadOptions, LoadOption);
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
    NewLoadOptions = AllocateZeroPool((Length - OptionLength + 2) * sizeof(CHAR16));
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
INTN StrniCmp(IN CHAR16 *Str1,
              IN CHAR16 *Str2,
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

CHAR16 *StriStr(IN CHAR16 *Str,
                IN CHAR16 *SearchFor)
{
  CHAR16 *End;
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

STATIC CHAR16 **CreateInfoLines(IN CHAR16 *Message, OUT UINTN *Count)
{
  CHAR16 *Ptr, **Information;
  UINTN   Index = 0, Total = 0;
  UINTN   Length = ((Message == NULL) ? 0 : StrLen(Message));
  // Check parameters
  if (Length == 0) {
    return NULL;
  }
  // Count how many new lines
  Ptr = Message;
  while (Ptr != NULL) {
    ++Total;
    Ptr = StrStr(Ptr, L"\n");
  }
  // Create information
  Information = (CHAR16 **)AllocatePool((Total * sizeof(CHAR16 *)) + (Length + Total * sizeof(CHAR16)));
  Information[Index++] = Ptr = (CHAR16 *)(Information + Total);
  StrCpy(Ptr, Message);
  while ((Index < Total) &&
         (Ptr = StrStr(Ptr, L"\n")) != NULL) {
    *Ptr++ = 0;
    Information[Index++] = Ptr;
  }
  // Return the info lines
  if (Count != NULL) {
    *Count = Total;
  }
  return Information;
}

extern REFIT_MENU_ENTRY MenuEntryReturn;

STATIC REFIT_MENU_ENTRY  *AlertMessageEntries[] = { &MenuEntryReturn };
STATIC REFIT_MENU_SCREEN  AlertMessageMenu = { 0, NULL, NULL, 0, NULL, 1, AlertMessageEntries,
                                               0, NULL, FALSE, FALSE, 0, 0, 0, 0, { 0, 0, 0, 0 } , NULL };

// Display an alert message
VOID AlertMessage(IN CHAR16 *Title, IN CHAR16 *Message)
{
  UINTN              Count = 0;
  // Break message into info lines
  CHAR16           **Information = CreateInfoLines(Message, &Count);
  // Check parameters
  if (Information != NULL) {
    if (Count > 0) {
      // Display the alert message
      AlertMessageMenu.InfoLineCount = Count;
      AlertMessageMenu.InfoLines = Information;
      AlertMessageMenu.Title = Title;
      RunMenu(&AlertMessageMenu, NULL);
    }
    FreePool(Information);
  }
}

#define TAG_YES 1
#define TAG_NO  2

STATIC REFIT_MENU_ENTRY   YesMessageEntry = { L"Yes", TAG_YES, 0, 0, 0, NULL, NULL, NULL, { 0, 0, 0, 0 }, ActionEnter, ActionNone, ActionNone, NULL };
STATIC REFIT_MENU_ENTRY   NoMessageEntry = { L"No", TAG_NO, 0, 0, 0, NULL, NULL, NULL, { 0, 0, 0, 0 }, ActionEnter, ActionNone, ActionNone, NULL };
STATIC REFIT_MENU_ENTRY  *YesNoMessageEntries[] = { &YesMessageEntry, &NoMessageEntry };
STATIC REFIT_MENU_SCREEN  YesNoMessageMenu = { 0, NULL, NULL, 0, NULL, 2, YesNoMessageEntries,
                                               0, NULL, FALSE, FALSE, 0, 0, 0, 0, { 0, 0, 0, 0 } , NULL };

// Display a yes/no prompt
BOOLEAN YesNoMessage(IN CHAR16 *Title, IN CHAR16 *Message)
{
  BOOLEAN            Result = FALSE;
  UINTN              Count = 0;
  REFIT_MENU_ENTRY  *ChosenEntry = NULL;
  // Break message into info lines
  CHAR16           **Information = CreateInfoLines(Message, &Count);
  // Display the yes/no message
  YesNoMessageMenu.InfoLineCount = Count;
  YesNoMessageMenu.InfoLines = Information;
  YesNoMessageMenu.Title = Title;
  Result = ((RunMenu(&YesNoMessageMenu, &ChosenEntry) == MENU_EXIT_ENTER) &&
            (ChosenEntry != NULL) && (ChosenEntry->Tag == TAG_YES));
  if (Information != NULL) {
    FreePool(Information);
  }
  return Result;
}
