/*
 * refit/scan/tool.c
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
#include "../refit/screen.h"

//
// Clover File location to boot from on removable media devices
//
#define CLOVER_MEDIA_FILE_NAME_IA32    L"\\EFI\\CLOVER\\CLOVERIA32.EFI"
#define CLOVER_MEDIA_FILE_NAME_IA64    L"\\EFI\\CLOVER\\CLOVERIA64.EFI"
#define CLOVER_MEDIA_FILE_NAME_X64     L"\\EFI\\CLOVER\\CLOVERX64.EFI"
#define CLOVER_MEDIA_FILE_NAME_ARM     L"\\EFI\\CLOVER\\CLOVERARM.EFI"

#if   defined (MDE_CPU_IA32)
#define CLOVER_MEDIA_FILE_NAME   CLOVER_MEDIA_FILE_NAME_IA32
#elif defined (MDE_CPU_IPF)
#define CLOVER_MEDIA_FILE_NAME   CLOVER_MEDIA_FILE_NAME_IA64
#elif defined (MDE_CPU_X64)
#define CLOVER_MEDIA_FILE_NAME   CLOVER_MEDIA_FILE_NAME_X64
#elif defined (MDE_CPU_EBC)
#elif defined (MDE_CPU_ARM)
#define CLOVER_MEDIA_FILE_NAME   CLOVER_MEDIA_FILE_NAME_ARM
//#else
//#error Unknown Processor Type
#endif

#ifndef DEBUG_ALL
#define DEBUG_SCAN_TOOL 1
#else
#define DEBUG_SCAN_TOOL DEBUG_ALL
#endif

#if DEBUG_SCAN_TOOL == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_SCAN_TOOL, __VA_ARGS__)
#endif

extern EMU_VARIABLE_CONTROL_PROTOCOL *gEmuVariableControl;

STATIC BOOLEAN AddToolEntry(IN CONST CHAR16 *LoaderPath, IN CONST CHAR16 *FullTitle, IN CONST CHAR16 *LoaderTitle,
                            IN REFIT_VOLUME *Volume, IN EG_IMAGE *Image,
                            IN CHAR16 ShortcutLetter, IN CONST CHAR16 *Options)
{
  REFIT_MENU_ENTRY_LOADER_TOOL *Entry;
  // Check the loader exists
  if ((LoaderPath == NULL) || (Volume == NULL) || (Volume->RootDir == NULL) ||
      !FileExists(Volume->RootDir, LoaderPath)) {
    return FALSE;
  }
  // Allocate the entry
  Entry = (__typeof__(Entry))AllocateZeroPool(sizeof(LOADER_ENTRY));
//  Entry = new REFIT_MENU_ENTRY_LOADER_TOOL();
  if (Entry == NULL) {
    return FALSE;
  }

  if (FullTitle) {
    Entry->Title = EfiStrDuplicate(FullTitle);
  } else {
    Entry->Title = PoolPrint(L"Start %s", LoaderTitle);
  }
//  Entry->Tag = TAG_TOOL;
  Entry->Row = 1;
  Entry->ShortcutLetter = ShortcutLetter;
  Entry->Image = Image;
//  Entry->ImageHover = ImageHover;
  Entry->LoaderPath = EfiStrDuplicate(LoaderPath);
  Entry->DevicePath = FileDevicePath(Volume->DeviceHandle, Entry->LoaderPath);
  Entry->DevicePathString = FileDevicePathToStr(Entry->DevicePath);
  Entry->LoadOptions = Options ? Options : NULL;
  //actions
  Entry->AtClick = ActionSelect;
  Entry->AtDoubleClick = ActionEnter;
  Entry->AtRightClick = ActionHelp;

  DBG("found tool %s\n", LoaderPath);
  AddMenuEntry(&MainMenu, Entry, true);
  return TRUE;
}

STATIC VOID AddCloverEntry(IN CONST CHAR16 *LoaderPath, IN CONST CHAR16 *LoaderTitle, IN REFIT_VOLUME *Volume)
{
  REFIT_MENU_ENTRY_CLOVER      *Entry;
  LOADER_ENTRY      *SubEntry;
  REFIT_MENU_SCREEN *SubScreen;
//  EFI_STATUS        Status;

  // prepare the menu entry
  Entry = (__typeof__(Entry))AllocateZeroPool(sizeof(LOADER_ENTRY));
//  Entry = new REFIT_MENU_ENTRY_CLOVER();
  Entry->Title          = LoaderTitle;
//  Entry->Tag            = TAG_CLOVER;
  Entry->Row            = 1;
  Entry->ShortcutLetter = 'C';
  Entry->Image          = BuiltinIcon(BUILTIN_ICON_FUNC_CLOVER);
  Entry->Volume = Volume;
  Entry->LoaderPath      = EfiStrDuplicate(LoaderPath);
  Entry->VolName         = Volume->VolName;
  Entry->DevicePath      = FileDevicePath(Volume->DeviceHandle, Entry->LoaderPath);
  Entry->DevicePathString = FileDevicePathToStr(Entry->DevicePath);
  Entry->Flags           = 0;
  Entry->LoadOptions     = NULL;
  Entry->LoaderType      = OSTYPE_OTHER;

  //actions
  Entry->AtClick = ActionEnter;
  Entry->AtDoubleClick = ActionDetails;
  Entry->AtRightClick = ActionDetails;

  // create the submenu
  SubScreen = (__typeof__(SubScreen))AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
  SubScreen->Title = EfiStrDuplicate(LoaderTitle);
  SubScreen->TitleImage = Entry->Image;
  SubScreen->ID = SCREEN_BOOT;
  SubScreen->AnimeRun = GetAnime(SubScreen);
  AddMenuInfoLine(SubScreen, FileDevicePathToStr(Volume->DevicePath));

  if (gEmuVariableControl != NULL) {
    gEmuVariableControl->UninstallEmulation(gEmuVariableControl);
  }

//always add and always remove menu entries
  SubEntry = DuplicateLoaderEntry(Entry);
  if (SubEntry) {
    SubEntry->Title        = L"Add Clover boot options for all entries";
    SubEntry->LoadOptions     = L"BO-ADD";
    AddMenuEntry(SubScreen, SubEntry, true);
  }

  SubEntry = DuplicateLoaderEntry(Entry);
  if (SubEntry) {
    SubEntry->Title        = L"Remove all Clover boot options";
    SubEntry->LoadOptions     = L"BO-REMOVE";
    AddMenuEntry(SubScreen, SubEntry, true);
  }

  SubEntry = DuplicateLoaderEntry(Entry);
  if (SubEntry) {
    SubEntry->Title        = L"Print all UEFI boot options to log";
    SubEntry->LoadOptions     = L"BO-PRINT";
    AddMenuEntry(SubScreen, SubEntry, true);
  }

  AddMenuEntry(SubScreen, &MenuEntryReturn, false);
  Entry->SubScreen = SubScreen;
  AddMenuEntry(&MainMenu, Entry, true);
}

VOID ScanTool(VOID)
{
  EFI_STATUS              Status;
  UINTN                   VolumeIndex;
  REFIT_VOLUME            *Volume;
  VOID                    *Interface;

  if (GlobalConfig.DisableFlags & HIDEUI_FLAG_TOOLS)
    return;

  //    Print(L"Scanning for tools...\n");

  // look for the EFI shell
  if (!(GlobalConfig.DisableFlags & HIDEUI_FLAG_SHELL)) {
#if defined(MDE_CPU_X64)
//    if (gFirmwareClover) {
//      AddToolEntry(L"\\EFI\\CLOVER\\tools\\Shell64U.efi", NULL, L"EFI Shell 64", SelfVolume, BuiltinIcon(BUILTIN_ICON_TOOL_SHELL), 'S');
//    } else
    //there seems to be the best version
      if (!AddToolEntry(L"\\EFI\\CLOVER\\tools\\Shell64U.efi", NULL, L"UEFI Shell 64", SelfVolume, BuiltinIcon(BUILTIN_ICON_TOOL_SHELL), 'S', NULL)) {
        AddToolEntry(L"\\EFI\\CLOVER\\tools\\Shell64.efi", NULL, L"EFI Shell 64", SelfVolume, BuiltinIcon(BUILTIN_ICON_TOOL_SHELL), 'S', NULL);
      }
//  }
#else
    AddToolEntry(L"\\EFI\\CLOVER\\tools\\Shell32.efi", NULL, L"EFI Shell 32", SelfVolume, BuiltinIcon(BUILTIN_ICON_TOOL_SHELL), 'S', NULL);
#endif
  }

//  if (!gFirmwareClover) { //Slice: I wish to extend functionality on emulated nvram
    for (VolumeIndex = 0; VolumeIndex < Volumes.size(); VolumeIndex++) {
      Volume = &Volumes[VolumeIndex];
      if (!Volume->RootDir || !Volume->DeviceHandle) {
        continue;
      }

      Status = gBS->HandleProtocol (Volume->DeviceHandle, &gEfiPartTypeSystemPartGuid, &Interface);
      if (Status == EFI_SUCCESS) {
        DBG("Checking EFI partition Volume %d for Clover\n", VolumeIndex);

        // OSX adds label "EFI" to EFI volumes and some UEFIs see that
        // as a file. This file then blocks access to the /EFI directory.
        // We will delete /EFI file here and leave only /EFI directory.
        if (DeleteFile(Volume->RootDir, L"EFI")) {
          DBG(" Deleted /EFI label\n");
        }

        if (FileExists(Volume->RootDir, CLOVER_MEDIA_FILE_NAME)) {
          DBG(" Found Clover\n");
          // Volume->BootType = BOOTING_BY_EFI;
          AddCloverEntry(CLOVER_MEDIA_FILE_NAME, L"Clover Boot Options", Volume);
          break;
        }
      }
    }
//  }
}

// Add custom tool entries
VOID AddCustomTool(VOID)
{
  UINTN             VolumeIndex;
  REFIT_VOLUME      *Volume;
  CUSTOM_TOOL_ENTRY *Custom;
  EG_IMAGE          *Image;
  UINTN              i = 0;

//  DBG("Custom tool start\n");
  DbgHeader("AddCustomTool");
  // Traverse the custom entries
  for (Custom = gSettings.CustomTool; Custom; ++i, Custom = Custom->Next) {
    if (OSFLAG_ISSET(Custom->Flags, OSFLAG_DISABLED)) {
      DBG("Custom tool %d skipped because it is disabled.\n", i);
      continue;
    }
    if (!gSettings.ShowHiddenEntries && OSFLAG_ISSET(Custom->Flags, OSFLAG_HIDDEN)) {
      DBG("Custom tool %d skipped because it is hidden.\n", i);
      continue;
    }

    if (Custom->Volume) {
      DBG("Custom tool %d matching \"%s\" ...\n", i, Custom->Volume);
    }
    for (VolumeIndex = 0; VolumeIndex < Volumes.size(); ++VolumeIndex) {
      Volume = &Volumes[VolumeIndex];

      DBG("   Checking volume \"%s\" (%s) ... ", Volume->VolName, Volume->DevicePathString);

      // Skip Whole Disc Boot
      if (Volume->RootDir == NULL) {
        DBG("skipped because volume is not readable\n");
        continue;
      }

      // skip volume if its kind is configured as disabled
      if ((Volume->DiskKind == DISK_KIND_OPTICAL && (GlobalConfig.DisableFlags & VOLTYPE_OPTICAL)) ||
          (Volume->DiskKind == DISK_KIND_EXTERNAL && (GlobalConfig.DisableFlags & VOLTYPE_EXTERNAL)) ||
          (Volume->DiskKind == DISK_KIND_INTERNAL && (GlobalConfig.DisableFlags & VOLTYPE_INTERNAL)) ||
          (Volume->DiskKind == DISK_KIND_FIREWIRE && (GlobalConfig.DisableFlags & VOLTYPE_FIREWIRE)))
      {
        DBG("skipped because media is disabled\n");
        continue;
      }

      if (Custom->VolumeType != 0) {
        if ((Volume->DiskKind == DISK_KIND_OPTICAL && ((Custom->VolumeType & VOLTYPE_OPTICAL) == 0)) ||
            (Volume->DiskKind == DISK_KIND_EXTERNAL && ((Custom->VolumeType & VOLTYPE_EXTERNAL) == 0)) ||
            (Volume->DiskKind == DISK_KIND_INTERNAL && ((Custom->VolumeType & VOLTYPE_INTERNAL) == 0)) ||
            (Volume->DiskKind == DISK_KIND_FIREWIRE && ((Custom->VolumeType & VOLTYPE_FIREWIRE) == 0))) {
          DBG("skipped because media is ignored\n");
          continue;
        }
      }

      if (Volume->Hidden) {
        DBG("skipped because volume is hidden\n");
        continue;
      }

      // Check for exact volume matches
      if (Custom->Volume) {
        if ((StrStr(Volume->DevicePathString, Custom->Volume) == NULL) &&
            ((Volume->VolName == NULL) || (StrStr(Volume->VolName, Custom->Volume) == NULL))) {
          DBG("skipped\n");
          continue;
        }
      }
      // Check the tool exists on the volume
      if (!FileExists(Volume->RootDir, Custom->Path)) {
        DBG("skipped because path does not exist\n");
        continue;
      }
      // Change to custom image if needed
      Image = Custom->Image;
      if ((Image == NULL) && Custom->ImagePath) {
        Image = egLoadImage(Volume->RootDir, Custom->ImagePath, TRUE);
        if (Image == NULL) {
          Image = egLoadImage(ThemeDir, Custom->ImagePath, TRUE);
          if (Image == NULL) {
            Image = egLoadImage(SelfDir, Custom->ImagePath, TRUE);
            if (Image == NULL) {
              Image = egLoadImage(SelfRootDir, Custom->ImagePath, TRUE);
            }
          }
        }
      }
      if (Image == NULL) {
        Image = BuiltinIcon(BUILTIN_ICON_TOOL_SHELL);
      }

      // Create a legacy entry for this volume

      AddToolEntry(Custom->Path, Custom->FullTitle, Custom->Title, Volume, Image, Custom->Hotkey, Custom->Options);

      DBG("match!\n");
//      break; // break scan volumes, continue scan entries -- why?
    }
  }
//  DBG("Custom tool end\n");
}
