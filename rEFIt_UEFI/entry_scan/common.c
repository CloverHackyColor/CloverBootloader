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
    DuplicateEntry->DevicePathString = Entry->DevicePathString;
    DuplicateEntry->LoadOptions     = Entry->LoadOptions;
    DuplicateEntry->LoaderPath      = Entry->LoaderPath;
    DuplicateEntry->VolName         = Entry->VolName;
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

/*
 static CHAR16 *RemoveLoadOption(IN CHAR16 *LoadOptions, IN CHAR16 *LoadOption)
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
 if ((Offset > 0 && LoadOptions[Offset - 1] != L' ')
 || ((Offset + OptionLength) < Length && LoadOptions[Offset + OptionLength] != L' ')
 )
 {
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
 
 // PoolPrint(L"%*s",... does not work as expected in previous code
 //if ((Offset + OptionLength) >= Length) return PoolPrint(L"%*s", Offset - 1, LoadOptions);
 // Otherwise remove the option
 //return PoolPrint(L"%*s%s", Offset, LoadOptions, Placement + OptionLength + 1);
 
 }
 
 // */