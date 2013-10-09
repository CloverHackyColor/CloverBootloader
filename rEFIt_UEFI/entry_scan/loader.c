/*
 * refit/scan/loader.c
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
#define DEBUG_SCAN_LOADER 1
#else
#define DEBUG_SCAN_LOADER DEBUG_ALL
#endif

#if DEBUG_SCAN_LOADER == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_SCAN_LOADER, __VA_ARGS__)
#endif

#define MACOSX_LOADER_PATH      L"\\System\\Library\\CoreServices\\boot.efi"

static CHAR16 *LinuxEntryPaths[] = {
  L"\\EFI\\SuSe\\elilo.efi",
#if defined(MDE_CPU_X64)
  L"\\EFI\\grub\\grubx64.efi",
  L"\\EFI\\Gentoo\\grubx64.efi",
  L"\\EFI\\Gentoo\\kernelx64.efi",
  L"\\EFI\\RedHat\\grubx64.efi",
  L"\\EFI\\ubuntu\\grubx64.efi",
  L"\\EFI\\kubuntu\\grubx64.efi",
  L"\\EFI\\LinuxMint\\grubx64.efi",
  L"\\EFI\\Fedora\\grubx64.efi",
  L"\\EFI\\opensuse\\grubx64.efi",
  L"\\EFI\\arch\\grubx64.efi",
  L"\\EFI\\arch_grub\\grubx64.efi",
#else
  L"\\EFI\\grub\\grub.efi",
  L"\\EFI\\Gentoo\\grub.efi",
  L"\\EFI\\Gentoo\\kernel.efi",
  L"\\EFI\\RedHat\\grub.efi",
  L"\\EFI\\ubuntu\\grub.efi",
  L"\\EFI\\kubuntu\\grub.efi",
  L"\\EFI\\LinuxMint\\grub.efi",
  L"\\EFI\\Fedora\\grub.efi",
  L"\\EFI\\opensuse\\grub.efi",
  L"\\EFI\\arch\\grub.efi",
  L"\\EFI\\arch_grub\\grub.efi",
#endif
};
static const UINTN LinuxEntryPathsCount = (sizeof(LinuxEntryPaths) / sizeof(CHAR16 *));

static CHAR16 *OSXInstallerPaths[] = {
  L"\\Mac OS X Install Data\\boot.efi",
  L"\\OS X Install Data\\boot.efi",
  L"\\.IABootFiles\\boot.efi"
};
static const UINTN OSXInstallerPathsCount = (sizeof(OSXInstallerPaths) / sizeof(CHAR16 *));


static BOOLEAN isFirstRootUUID(REFIT_VOLUME *Volume)
{
  UINTN                   VolumeIndex;
  REFIT_VOLUME            *scanedVolume;
  
  for (VolumeIndex = 0; VolumeIndex < VolumesCount; VolumeIndex++) {
    scanedVolume = Volumes[VolumeIndex];
    
    if ( scanedVolume == Volume)
      return TRUE;
    
    if (CompareGuid(&scanedVolume->RootUUID, &Volume->RootUUID))
      return FALSE;
    
  }
  return TRUE;
}

//Set Entry->VolName to .disk_label.contentDetails if it exists
static EFI_STATUS GetOSXVolumeName(LOADER_ENTRY *Entry)
{
  EFI_STATUS	Status = EFI_NOT_FOUND;
  CHAR16* targetNameFile = L"System\\Library\\CoreServices\\.disk_label.contentDetails";
  CHAR8* 	fileBuffer;
  CHAR8*  targetString;
  UINTN   fileLen = 0;
  if(FileExists(Entry->Volume->RootDir, targetNameFile)) {
    Status = egLoadFile(Entry->Volume->RootDir, targetNameFile, (UINT8 **)&fileBuffer, &fileLen);
    if(!EFI_ERROR(Status)) {
      CHAR16  *tmpName;
      //Create null terminated string
      targetString = (CHAR8*) AllocateZeroPool(fileLen+1);
      CopyMem( (VOID*)targetString, (VOID*)fileBuffer, fileLen);
      
      //      NOTE: Sothor - This was never run. If we need this correct it and uncomment it.
      //      if (Entry->LoaderType == OSTYPE_OSX) {
      //        INTN i;
      //        //remove occurence number. eg: "vol_name 2" --> "vol_name"
      //        i=fileLen-1;
      //        while ((i>0) && (targetString[i]>='0') && (targetString[i]<='9')) {
      //          i--;
      //        }
      //        if (targetString[i] == ' ') {
      //          targetString[i] = 0;
      //        }
      //      }
      
      //Convert to Unicode
      tmpName = (CHAR16*)AllocateZeroPool((fileLen+1)*2);
      tmpName = AsciiStrToUnicodeStr(targetString, tmpName);
      
      Entry->VolName = EfiStrDuplicate(tmpName);
      
      FreePool(tmpName);
      FreePool(fileBuffer);
      FreePool(targetString);
    }
  }
  return Status;
}


static LOADER_ENTRY *CreateLoaderEntry(IN CHAR16 *LoaderPath, IN CHAR16 *LoaderOptions, IN CHAR16 *FullTitle, IN CHAR16 *LoaderTitle, IN REFIT_VOLUME *Volume,
                                       IN EG_IMAGE *Image, IN EG_IMAGE *DriveImage, IN UINT8 OSType, IN UINT8 Flags, IN CHAR16 Hotkey, IN BOOLEAN CustomEntry)
{
  EFI_DEVICE_PATH *LoaderDevicePath;
  CHAR16          *LoaderDevicePathString;
  CHAR16          *FilePathAsString;
  CHAR16          *OSIconName;
  //CHAR16           IconFileName[256]; Sothor - Unused?
  CHAR16           ShortcutLetter;
  LOADER_ENTRY    *Entry;
  INTN             i;
  
  // Check parameters are valid
  if ((LoaderPath == NULL) || (Volume == NULL)) {
    return NULL;
  }
  
  // Get the loader device path
  LoaderDevicePath = FileDevicePath(Volume->DeviceHandle, LoaderPath);
  if (LoaderDevicePath == NULL) {
    return NULL;
  }
  LoaderDevicePathString = FileDevicePathToStr(LoaderDevicePath);
  if (LoaderDevicePathString == NULL) {
    return NULL;
  }
  
  // Ignore this loader if it's self path
  FilePathAsString = FileDevicePathToStr(SelfLoadedImage->FilePath);
  if (FilePathAsString) {
    INTN Comparison = StriCmp(FilePathAsString, LoaderDevicePathString);
    FreePool(FilePathAsString);
    if (Comparison == 0) {
      DBG("skipped because path `%s` is self path!\n", LoaderDevicePathString);
      FreePool(LoaderDevicePathString);
      return NULL;
    }
  }
  
  if (!CustomEntry) {
    CUSTOM_LOADER_ENTRY *Custom;
    UINTN                CustomIndex = 0;
    
    // Ignore this loader if it's device path is already present in another loader
    if (MainMenu.Entries) {
      for (i = 0; i < MainMenu.EntryCount; ++i) {
        REFIT_MENU_ENTRY *MainEntry = MainMenu.Entries[i];
        // Only want loaders
        if (MainEntry && (MainEntry->Tag == TAG_LOADER)) {
          LOADER_ENTRY *Loader = (LOADER_ENTRY *)MainEntry;
          if (StriCmp(Loader->DevicePathString, LoaderDevicePathString) == 0) {
            DBG("skipped because path `%s` already exists for another entry!\n", LoaderDevicePathString);
            FreePool(LoaderDevicePathString);
            return NULL;
          }
        }
      }
    }
    // If this isn't a custom entry make sure it's not hidden by a custom entry
    Custom = gSettings.CustomEntries;
    while (Custom) {
      // Check if the custom entry is hidden or disabled
      if (OSFLAG_ISSET(Custom->Flags, OSFLAG_DISABLED) ||
          (OSFLAG_ISSET(Custom->Flags, OSFLAG_HIDDEN) && !gSettings.ShowHiddenEntries)) {
        // Check if there needs to be a volume match
        if (Custom->Volume != NULL) {
          // Check if the string matches the volume
          if ((StrStr(Volume->DevicePathString, Custom->Volume) != NULL) ||
              ((Volume->VolName != NULL) && (StrStr(Volume->VolName, Custom->Volume) != NULL))) {
            if (Custom->VolumeType != 0) {
              if (((Volume->DiskKind == DISK_KIND_INTERNAL) && (Custom->VolumeType & DISABLE_FLAG_INTERNAL)) ||
                  ((Volume->DiskKind == DISK_KIND_EXTERNAL) && (Custom->VolumeType & DISABLE_FLAG_EXTERNAL)) ||
                  ((Volume->DiskKind == DISK_KIND_OPTICAL) && (Custom->VolumeType & DISABLE_FLAG_OPTICAL)) ||
                  ((Volume->DiskKind == DISK_KIND_FIREWIRE) && (Custom->VolumeType & DISABLE_FLAG_FIREWIRE))) {
                if (Custom->Path != NULL) {
                  // Try to match the loader paths and types
                  if (StriCmp(Custom->Path, LoaderPath) == 0) {
                    if (Custom->Type != 0) {
                      if (OSTYPE_COMPARE(Custom->Type, OSType)) {
                        DBG("skipped path `%s` because it is a volume, volumetype, path and type match for custom entry %d!\n", LoaderDevicePathString, CustomIndex);
                        FreePool(LoaderDevicePathString);
                        return NULL;
                      } else {
                        DBG("partial volume, volumetype, and path match for path `%s` and custom entry %d, type did not match\n", LoaderDevicePathString, CustomIndex);
                      }
                    } else {
                      DBG("skipped path `%s` because it is a volume, volumetype and path match for custom entry %d!\n", LoaderDevicePathString, CustomIndex);
                      FreePool(LoaderDevicePathString);
                      return NULL;
                    }
                  } else {
                    DBG("partial volume, and volumetype match for path `%s` and custom entry %d, path did not match\n", LoaderDevicePathString, CustomIndex);
                  }
                } else if (Custom->Type != 0) {
                  if (OSTYPE_COMPARE(Custom->Type, OSType)) {
                    // Only match the loader type
                    DBG("skipped path `%s` because it is a volume, volumetype and type match for custom entry %d!\n", LoaderDevicePathString, CustomIndex);
                    FreePool(LoaderDevicePathString);
                    return NULL;
                  } else {
                    DBG("partial volume, and volumetype match for path `%s` and custom entry %d, type did not match\n", LoaderDevicePathString, CustomIndex);
                  }
                } else {
                  DBG("skipped path `%s` because it is a volume and volumetype match for custom entry %d!\n", LoaderDevicePathString, CustomIndex);
                  FreePool(LoaderDevicePathString);
                  return NULL;
                }
              } else {
                DBG("partial volume match for path `%s` and custom entry %d, volumetype did not match\n", LoaderDevicePathString, CustomIndex);
              }
            } if (Custom->Path != NULL) {
              // Check if there needs to be a path match also
              if (StriCmp(Custom->Path, LoaderPath) == 0) {
                if (Custom->Type != 0) {
                  if (OSTYPE_COMPARE(Custom->Type, OSType)) {
                    DBG("skipped path `%s` because it is a volume, path and type match for custom entry %d!\n", LoaderDevicePathString, CustomIndex);
                    FreePool(LoaderDevicePathString);
                    return NULL;
                  } else {
                    DBG("partial volume, and path match for path `%s` and custom entry %d, type did not match\n", LoaderDevicePathString, CustomIndex);
                  }
                } else {
                  DBG("skipped path `%s` because it is a volume and path match for custom entry %d!\n", LoaderDevicePathString, CustomIndex);
                  FreePool(LoaderDevicePathString);
                  return NULL;
                }
              } else {
                DBG("partial volume match for path `%s` and custom entry %d, path did not match\n", LoaderDevicePathString, CustomIndex);
              }
            } else if (Custom->Type != 0) {
              if (OSTYPE_COMPARE(Custom->Type, OSType)) {
                DBG("skipped path `%s` because it is a volume and type match for custom entry %d!\n", LoaderDevicePathString, CustomIndex);
                FreePool(LoaderDevicePathString);
                return NULL;
              } else {
                DBG("partial volume match for path `%s` and custom entry %d, type did not match\n", LoaderDevicePathString, CustomIndex);
              }
            } else {
              DBG("skipped path `%s` because it is a volume match for custom entry %d!\n", LoaderDevicePathString, CustomIndex);
              FreePool(LoaderDevicePathString);
              return NULL;
            }
          } else {
            DBG("volume did not match for path `%s` and custom entry %d\n", LoaderDevicePathString, CustomIndex);
          }
        } else if (Custom->VolumeType != 0) {
          if (((Volume->DiskKind == DISK_KIND_INTERNAL) && (Custom->VolumeType & DISABLE_FLAG_INTERNAL)) ||
              ((Volume->DiskKind == DISK_KIND_EXTERNAL) && (Custom->VolumeType & DISABLE_FLAG_EXTERNAL)) ||
              ((Volume->DiskKind == DISK_KIND_OPTICAL) && (Custom->VolumeType & DISABLE_FLAG_OPTICAL)) ||
              ((Volume->DiskKind == DISK_KIND_FIREWIRE) && (Custom->VolumeType & DISABLE_FLAG_FIREWIRE))) {
            if (Custom->Path != NULL) {
              // Try to match the loader paths and types
              if (StriCmp(Custom->Path, LoaderPath) == 0) {
                if (Custom->Type != 0) {
                  if (OSTYPE_COMPARE(Custom->Type, OSType)) {
                    DBG("skipped path `%s` because it is a volumetype, path and type match for custom entry %d!\n", LoaderDevicePathString, CustomIndex);
                    FreePool(LoaderDevicePathString);
                    return NULL;
                  } else {
                    DBG("partial volumetype, and path match for path `%s` and custom entry %d, type did not match\n", LoaderDevicePathString, CustomIndex);
                  }
                } else {
                  DBG("skipped path `%s` because it is a volumetype and path match for custom entry %d!\n", LoaderDevicePathString, CustomIndex);
                  FreePool(LoaderDevicePathString);
                  return NULL;
                }
              } else {
                DBG("partial volumetype match for path `%s` and custom entry %d, path did not match\n", LoaderDevicePathString, CustomIndex);
              }
            } else if (Custom->Type != 0) {
              if (OSTYPE_COMPARE(Custom->Type, OSType)) {
                // Only match the loader type
                DBG("skipped path `%s` because it is a volumetype and type match for custom entry %d!\n", LoaderDevicePathString, CustomIndex);
                FreePool(LoaderDevicePathString);
                return NULL;
              } else {
                DBG("partial volumetype match for path `%s` and custom entry %d, type did not match\n", LoaderDevicePathString, CustomIndex);
              }
            } else {
              DBG("skipped path `%s` because it is a volumetype match for custom entry %d!\n", LoaderDevicePathString, CustomIndex);
              FreePool(LoaderDevicePathString);
              return NULL;
            }
          } else {
            DBG("did not match volumetype for path `%s` and custom entry %d\n", LoaderDevicePathString, CustomIndex);
          }
        } else if (Custom->Path != NULL) {
          // Try to match the loader paths and types
          if (StriCmp(Custom->Path, LoaderPath) == 0) {
            if (Custom->Type != 0) {
              if (OSTYPE_COMPARE(Custom->Type, OSType)) {
                DBG("skipped path `%s` because it is a path and type match for custom entry %d!\n", LoaderDevicePathString, CustomIndex);
                FreePool(LoaderDevicePathString);
                return NULL;
              } else {
                DBG("partial path match for path `%s` and custom entry %d, type did not match\n", LoaderDevicePathString, CustomIndex);
              }
            } else {
              DBG("skipped path `%s` because it is a path match for custom entry %d!\n", LoaderDevicePathString, CustomIndex);
              FreePool(LoaderDevicePathString);
              return NULL;
            }
          } else {
            DBG("did not match path for path `%s` and custom entry %d\n", LoaderDevicePathString, CustomIndex);
          }
        } else if (Custom->Type != 0) {
          if (OSTYPE_COMPARE(Custom->Type, OSType)) {
            // Only match the loader type
            DBG("skipped path `%s` because it is a type match for custom entry %d!\n", LoaderDevicePathString, CustomIndex);
            FreePool(LoaderDevicePathString);
            return NULL;
          } else {
            DBG("did not match type for path `%s` and custom entry %d!\n", LoaderDevicePathString, CustomIndex);
          }
        }
      }
      Custom = Custom->Next;
      ++CustomIndex;
    }
  }
  
  // prepare the menu entry
  Entry = AllocateZeroPool(sizeof(LOADER_ENTRY));
  if (Volume->BootType == BOOTING_BY_EFI) {
    Entry->me.Tag          = TAG_LOADER;
  } else {
    Entry->me.Tag          = TAG_LEGACY;
  }
  
  Entry->me.Row = 0;
  Entry->Volume = Volume;
  
  Entry->LoaderPath       = EfiStrDuplicate(LoaderPath);
  Entry->VolName          = Volume->VolName;
  Entry->DevicePath       = LoaderDevicePath;
  Entry->DevicePathString = LoaderDevicePathString;
  Entry->Flags            = Flags;
  if (LoaderOptions) {
    if (OSFLAG_ISSET(Flags, OSFLAG_NODEFAULTARGS)) {
      Entry->LoadOptions  = EfiStrDuplicate(LoaderOptions);
    } else {
      Entry->LoadOptions  = PoolPrint(L"%a %s", gSettings.BootArgs, LoaderOptions);
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NODEFAULTARGS);// Sothor - Once we add arguments once we cannot do it again. So prevent being updated with boot arguments from options menu (unless reinit?).
    }
  } else if ((AsciiStrLen(gSettings.BootArgs) > 0) && OSFLAG_ISUNSET(Flags, OSFLAG_NODEFAULTARGS)) {
    Entry->LoadOptions    = PoolPrint(L"%a", gSettings.BootArgs);
  }
  
  // locate a custom icon for the loader
  //StrCpy(IconFileName, Volume->OSIconName); Sothor - Unused?
  //actions
  Entry->me.AtClick = ActionSelect;
  Entry->me.AtDoubleClick = ActionEnter;
  Entry->me.AtRightClick = ActionDetails;
  
  Entry->LoaderType = OSType;
  Entry->OSVersion = GetOSVersion(Entry->Volume);
  
  // detect specific loaders
  OSIconName = NULL;
  ShortcutLetter = 0;
  
  switch (OSType) {
    case OSTYPE_OSX:
    case OSTYPE_RECOVERY:
    case OSTYPE_OSX_INSTALLER:
      OSIconName = GetOSIconName(Entry->OSVersion);// Sothor - Get OSIcon name using OSVersion
      if (Entry->LoadOptions == NULL || (StrStr(Entry->LoadOptions, L"-v") == NULL && StrStr(Entry->LoadOptions, L"-V") == NULL)) {
        // OSX is not booting verbose, so we can set console to graphics mode
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_USEGRAPHICS);
      }
      if (gSettings.WithKexts) {
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
      }
      ShortcutLetter = 'M';
      GetOSXVolumeName(Entry);
      break;
    case OSTYPE_WIN:
      OSIconName = L"win";
      ShortcutLetter = 'W';
      break;
    case OSTYPE_WINEFI:
      OSIconName = L"vista";
      ShortcutLetter = 'V';
      break;
    case OSTYPE_LIN:
      OSIconName = L"linux";// Sothor - This is now here only for Custom Entries, the default scan loads them and passes as a parameter
      ShortcutLetter = 'L';
      break;
    case OSTYPE_OTHER:
    case OSTYPE_EFI:
      OSIconName = L"clover";
      ShortcutLetter = 'U';
      Entry->LoaderType = OSTYPE_OTHER;
      break;
    default:
      OSIconName = L"unknown";
      Entry->LoaderType = OSTYPE_OTHER;
      break;
  }
  
  if (FullTitle) {
    Entry->me.Title = EfiStrDuplicate(FullTitle);
  } else {
    Entry->me.Title = PoolPrint(L"Boot %s from %s", (LoaderTitle != NULL) ? LoaderTitle : LoaderPath + 1, Entry->VolName);
  }
  Entry->me.ShortcutLetter = (Hotkey == 0) ? ShortcutLetter : Hotkey;
  
  // get custom volume icon if present
  if (Image) {
    Entry->me.Image = Image;
  } else {
    if (GlobalConfig.CustomIcons && FileExists(Volume->RootDir, L"VolumeIcon.icns")){
      Entry->me.Image = LoadIcns(Volume->RootDir, L"VolumeIcon.icns", 128);
      DBG("using VolumeIcon.icns image from Volume\n");
    } else {
      Entry->me.Image = LoadOSIcon(OSIconName, L"unknown", 128, FALSE, TRUE);
    }
  }
  // Load DriveImage
  Entry->me.DriveImage = (DriveImage != NULL) ? DriveImage : ScanVolumeDefaultIcon(Volume);
  
  // DBG("HideBadges=%d Volume=%s ", GlobalConfig.HideBadges, Volume->VolName);
  if (GlobalConfig.HideBadges & HDBADGES_SHOW) {
    if (GlobalConfig.HideBadges & HDBADGES_SWAP) {
      Entry->me.BadgeImage = egCopyScaledImage(Entry->me.DriveImage, 4);
      // DBG(" Show badge as Drive.");
    } else {
      Entry->me.BadgeImage = egCopyScaledImage(Entry->me.Image, 8);
      // DBG(" Show badge as OSImage.");
    }
  }
  return Entry;
}

static LOADER_ENTRY * AddLoaderEntry2(IN CHAR16 *LoaderPath, IN CHAR16 *LoaderOptions, IN CHAR16 *FullTitle, IN CHAR16 *LoaderTitle, IN REFIT_VOLUME *Volume,
                                      IN EG_IMAGE *Image, IN EG_IMAGE *DriveImage, IN UINT8 OSType, IN UINT8 Flags, IN CHAR16 Hotkey, IN BOOLEAN CustomEntry)
{
  CHAR16            *FileName, *TempOptions;
  CHAR16            DiagsFileName[256];
  LOADER_ENTRY      *Entry, *SubEntry;
  REFIT_MENU_SCREEN *SubScreen;
  UINT64            VolumeSize;
  EFI_GUID          *Guid = NULL;
  
  Entry = CreateLoaderEntry(LoaderPath, LoaderOptions, FullTitle, LoaderTitle, Volume, Image, DriveImage, OSType, Flags, Hotkey, CustomEntry);
  if (Entry == NULL) {
    return NULL;
  }
  
  FileName = Basename(LoaderPath);
  
  // create the submenu
  SubScreen = AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
  SubScreen->Title = PoolPrint(L"Boot Options for %s on %s", (LoaderTitle != NULL) ? LoaderTitle : FileName, Entry->VolName);
  SubScreen->TitleImage = Entry->me.Image;
  SubScreen->ID = OSType + 20;
  //  DBG("get anime for os=%d\n", SubScreen->ID);
  SubScreen->AnimeRun = GetAnime(SubScreen);
  VolumeSize = RShiftU64(MultU64x32(Volume->BlockIO->Media->LastBlock, Volume->BlockIO->Media->BlockSize), 20);
  AddMenuInfoLine(SubScreen, PoolPrint(L"Volume size: %dMb", VolumeSize));
  AddMenuInfoLine(SubScreen, DevicePathToStr(Entry->DevicePath));
  Guid = FindGPTPartitionGuidInDevicePath(Volume->DevicePath);
  if (Guid) {
    CHAR8 *GuidStr = AllocateZeroPool(50);
    AsciiSPrint(GuidStr, 50, "%g", Guid);
    AddMenuInfoLine(SubScreen, PoolPrint(L"UUID: %a", GuidStr));
    FreePool(GuidStr);
  }
  
  
  // default entry
  SubEntry = DuplicateLoaderEntry(Entry);
  if (SubEntry) {
    SubEntry->me.Title = (Entry->LoaderType == OSTYPE_OSX ||
                          Entry->LoaderType == OSTYPE_OSX_INSTALLER ||
                          Entry->LoaderType == OSTYPE_RECOVERY) ?
    L"Boot Mac OS X" : PoolPrint(L"Run %s", FileName);
    AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
  }
  // loader-specific submenu entries
  if (Entry->LoaderType == OSTYPE_OSX || Entry->LoaderType == OSTYPE_OSX_INSTALLER || Entry->LoaderType == OSTYPE_RECOVERY) {          // entries for Mac OS X
#if defined(MDE_CPU_X64)
    if (AsciiStrnCmp(Entry->OSVersion,"10.7",4) &&
        AsciiStrnCmp(Entry->OSVersion,"10.8",4)) {
      SubEntry = DuplicateLoaderEntry(Entry);
      if (SubEntry) {
        SubEntry->LoadOptions     = AddLoadOption(SubEntry->LoadOptions, L"arch=x86_64");
        SubEntry->me.Title        = L"Boot Mac OS X (64-bit)";
        AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
      }
    }
#endif
    if (AsciiStrnCmp(Entry->OSVersion,"10.7",4) &&
        AsciiStrnCmp(Entry->OSVersion,"10.8",4)) {
      SubEntry = DuplicateLoaderEntry(Entry);
      if (SubEntry) {
        SubEntry->me.Title        = L"Boot Mac OS X (32-bit)";
        SubEntry->LoadOptions     = AddLoadOption(SubEntry->LoadOptions, L"arch=i386");
        AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
      }
    }
    
    if (!(GlobalConfig.DisableFlags & DISABLE_FLAG_SINGLEUSER)) {
      
#if defined(MDE_CPU_X64)
      if (AsciiStrnCmp(Entry->OSVersion,"10.7",4) == 0 ||
          AsciiStrnCmp(Entry->OSVersion,"10.8",4) == 0 ) {
        SubEntry = DuplicateLoaderEntry(Entry);
        if (SubEntry) {
          SubEntry->me.Title        = L"Boot Mac OS X in verbose mode";
          SubEntry->Flags           = OSFLAG_UNSET(SubEntry->Flags, OSFLAG_USEGRAPHICS);
          SubEntry->LoadOptions     = AddLoadOption(SubEntry->LoadOptions, L"-v");
          AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
        }
      } else {
        SubEntry = DuplicateLoaderEntry(Entry);
        if (SubEntry) {
          SubEntry->me.Title        = L"Boot Mac OS X in verbose mode (64bit)";
          SubEntry->Flags           = OSFLAG_UNSET(SubEntry->Flags, OSFLAG_USEGRAPHICS);
          TempOptions = AddLoadOption(SubEntry->LoadOptions, L"-v");
          SubEntry->LoadOptions     = AddLoadOption(TempOptions, L"arch=x86_64");
          FreePool(TempOptions);
          AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
        }
      }
      
#endif
      
      if (AsciiStrnCmp(Entry->OSVersion,"10.7",4) &&
          AsciiStrnCmp(Entry->OSVersion,"10.8",4)) {
        SubEntry = DuplicateLoaderEntry(Entry);
        if (SubEntry) {
          SubEntry->me.Title        = L"Boot Mac OS X in verbose mode (32-bit)";
          SubEntry->Flags           = OSFLAG_SET(SubEntry->Flags, OSFLAG_USEGRAPHICS);
          TempOptions = AddLoadOption(SubEntry->LoadOptions, L"-v");
          SubEntry->LoadOptions     = AddLoadOption(TempOptions, L"arch=i386");
          FreePool(TempOptions);
          AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
        }
      }
      
      SubEntry = DuplicateLoaderEntry(Entry);
      if (SubEntry) {
        SubEntry->me.Title        = L"Boot Mac OS X in safe mode";
        SubEntry->Flags           = OSFLAG_UNSET(SubEntry->Flags, OSFLAG_USEGRAPHICS);
        TempOptions = AddLoadOption(SubEntry->LoadOptions, L"-v");
        SubEntry->LoadOptions     = AddLoadOption(TempOptions, L"-x");
        FreePool(TempOptions);
        AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
      }
      
      SubEntry = DuplicateLoaderEntry(Entry);
      if (SubEntry) {
        SubEntry->me.Title        = L"Boot Mac OS X in single user verbose mode";
        SubEntry->Flags           = OSFLAG_UNSET(SubEntry->Flags, OSFLAG_USEGRAPHICS);
        TempOptions = AddLoadOption(SubEntry->LoadOptions, L"-v");
        SubEntry->LoadOptions     = AddLoadOption(TempOptions, L"-s");
        FreePool(TempOptions);
        AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
      }
      
      SubEntry = DuplicateLoaderEntry(Entry);
      if (SubEntry) {
        SubEntry->me.Title        = OSFLAG_ISSET(SubEntry->Flags, OSFLAG_NOCACHES) ?
        L"Boot Mac OS X with caches" :
        L"Boot Mac OS X without caches";
        SubEntry->Flags           = OSFLAG_TOGGLE(SubEntry->Flags, OSFLAG_NOCACHES);
        SubEntry->LoadOptions     = AddLoadOption(SubEntry->LoadOptions, L"-v");
        AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
      }
      
      SubEntry = DuplicateLoaderEntry(Entry);
      if (SubEntry) {
        SubEntry->me.Title        = OSFLAG_ISSET(SubEntry->Flags, OSFLAG_WITHKEXTS) ?
        L"Boot Mac OS X without injected kexts" :
        L"Boot Mac OS X with injected kexts";
        SubEntry->Flags           = OSFLAG_TOGGLE(SubEntry->Flags, OSFLAG_WITHKEXTS);
        SubEntry->LoadOptions     = AddLoadOption(SubEntry->LoadOptions, L"-v");
        AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
      }
      
      if (OSFLAG_ISSET(Entry->Flags, OSFLAG_WITHKEXTS))
      {
        if (OSFLAG_ISSET(Entry->Flags, OSFLAG_NOCACHES))
        {
          SubEntry = DuplicateLoaderEntry(Entry);
          if (SubEntry) {
            SubEntry->me.Title        = L"Boot Mac OS X with caches and without injected kexts";
            SubEntry->Flags           = OSFLAG_UNSET(OSFLAG_UNSET(SubEntry->Flags, OSFLAG_NOCACHES), OSFLAG_WITHKEXTS);
            SubEntry->LoadOptions     = AddLoadOption(SubEntry->LoadOptions, L"-v");
            AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
          }
        }
        else
        {
          SubEntry = DuplicateLoaderEntry(Entry);
          if (SubEntry) {
            SubEntry->me.Title        = L"Boot Mac OS X without caches and without injected kexts";
            SubEntry->Flags           = OSFLAG_UNSET(OSFLAG_SET(SubEntry->Flags, OSFLAG_NOCACHES), OSFLAG_WITHKEXTS);
            SubEntry->LoadOptions     = AddLoadOption(SubEntry->LoadOptions, L"-v");
            AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
          }
        }
      }
      else if (OSFLAG_ISSET(Entry->Flags, OSFLAG_NOCACHES))
      {
        SubEntry = DuplicateLoaderEntry(Entry);
        if (SubEntry) {
          SubEntry->me.Title        = L"Boot Mac OS X with caches and with injected kexts";
          SubEntry->Flags           = OSFLAG_SET(OSFLAG_UNSET(SubEntry->Flags, OSFLAG_NOCACHES), OSFLAG_WITHKEXTS);
          SubEntry->LoadOptions     = AddLoadOption(SubEntry->LoadOptions, L"-v");
          AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
        }
      }
      else
      {
        SubEntry = DuplicateLoaderEntry(Entry);
        if (SubEntry) {
          SubEntry->me.Title        = L"Boot Mac OS X without caches and with injected kexts";
          SubEntry->Flags           = OSFLAG_SET(OSFLAG_SET(SubEntry->Flags, OSFLAG_NOCACHES), OSFLAG_WITHKEXTS);
          SubEntry->LoadOptions     = AddLoadOption(SubEntry->LoadOptions, L"-v");
          AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
        }
      }
    }
    
    // check for Apple hardware diagnostics
    StrCpy(DiagsFileName, L"\\System\\Library\\CoreServices\\.diagnostics\\diags.efi");
    if (FileExists(Volume->RootDir, DiagsFileName) && !(GlobalConfig.DisableFlags & DISABLE_FLAG_HWTEST)) {
      DBG("  - Apple Hardware Test found\n");
      
      // NOTE: Sothor - I'm not sure if to duplicate parent entry here.
      SubEntry = AllocateZeroPool(sizeof(LOADER_ENTRY));
      SubEntry->me.Title        = L"Run Apple Hardware Test";
      SubEntry->me.Tag          = TAG_LOADER;
      SubEntry->LoaderPath      = EfiStrDuplicate(DiagsFileName);
      SubEntry->Volume          = Entry->Volume;
      SubEntry->VolName         = Entry->VolName;
      SubEntry->DevicePath      = FileDevicePath(Volume->DeviceHandle, SubEntry->LoaderPath);
      SubEntry->DevicePathString = Entry->DevicePathString;
      SubEntry->Flags           = OSFLAG_SET(Entry->Flags, OSFLAG_USEGRAPHICS);
      SubEntry->me.AtClick      = ActionEnter;
      AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
    }
    
  } else if (Entry->LoaderType == OSTYPE_LIN) {   // entries for elilo
    SubEntry = DuplicateLoaderEntry(Entry);
    if (SubEntry) {
      SubEntry->me.Title        = PoolPrint(L"Run %s in interactive mode", FileName);
      SubEntry->LoadOptions     = L"-p";
      AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
    }
    
    SubEntry = DuplicateLoaderEntry(Entry);
    if (SubEntry) {
      SubEntry->me.Title        = L"Boot Linux for a 17\" iMac or a 15\" MacBook Pro (*)";
      SubEntry->Flags           = OSFLAG_SET(SubEntry->Flags, OSFLAG_USEGRAPHICS);
      SubEntry->LoadOptions     = L"-d 0 i17";
      AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
    }
    
    SubEntry = DuplicateLoaderEntry(Entry);
    if (SubEntry) {
      SubEntry->me.Title        = L"Boot Linux for a 20\" iMac (*)";
      SubEntry->Flags           = OSFLAG_SET(SubEntry->Flags, OSFLAG_USEGRAPHICS);
      SubEntry->LoadOptions     = L"-d 0 i20";
      AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
    }
    
    SubEntry = DuplicateLoaderEntry(Entry);
    if (SubEntry) {
      SubEntry->me.Title        = L"Boot Linux for a Mac Mini (*)";
      SubEntry->Flags           = OSFLAG_SET(SubEntry->Flags, OSFLAG_USEGRAPHICS);
      SubEntry->LoadOptions     = L"-d 0 mini";
      AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
    }
    
    AddMenuInfoLine(SubScreen, L"NOTE: This is an example. Entries");
    AddMenuInfoLine(SubScreen, L"marked with (*) may not work.");
    
  } else if ((Entry->LoaderType == OSTYPE_WIN) || (Entry->LoaderType == OSTYPE_WINEFI)) {   // entries for xom.efi
    // by default, skip the built-in selection and boot from hard disk only
    Entry->LoadOptions = L"-s -h";
    
    SubEntry = DuplicateLoaderEntry(Entry);
    if (SubEntry) {
      SubEntry->me.Title        = L"Boot Windows from Hard Disk";
      AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
    }
    
    SubEntry = DuplicateLoaderEntry(Entry);
    if (SubEntry) {
      SubEntry->me.Title        = L"Boot Windows from CD-ROM";
      SubEntry->LoadOptions     = L"-s -c";
      AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
    }
    
    SubEntry = DuplicateLoaderEntry(Entry);
    if (SubEntry) {
      SubEntry->me.Title        = PoolPrint(L"Run %s in text mode", FileName);
      SubEntry->Flags           = OSFLAG_UNSET(SubEntry->Flags, OSFLAG_USEGRAPHICS);
      SubEntry->LoadOptions     = L"-v";
      SubEntry->LoaderType      = OSTYPE_OTHER; // Sothor - Why are we using OSTYPE_OTHER here?
      AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
    }
    
  }
  
  AddMenuEntry(SubScreen, &MenuEntryReturn);
  Entry->me.SubScreen = SubScreen;
  DBG("    Added '%s': OSType='%d', OSVersion='%a'\n", Entry->me.Title, Entry->LoaderType, Entry->OSVersion);
  return Entry;
}

static LOADER_ENTRY * AddLoaderEntry(IN CHAR16 *LoaderPath, IN CHAR16 *LoaderOptions, IN CHAR16 *FullTitle, IN CHAR16 *LoaderTitle, IN REFIT_VOLUME *Volume,
                                     IN EG_IMAGE *Image, IN EG_IMAGE *DriveImage, IN UINT8 OSType, IN UINT8 Flags, IN CHAR16 Hotkey)
{
  LOADER_ENTRY *Entry = AddLoaderEntry2(LoaderPath, LoaderOptions, FullTitle, LoaderTitle, Volume, Image, DriveImage, OSType, Flags, Hotkey, FALSE);
  if (Entry) {
    AddMenuEntry(&MainMenu, (REFIT_MENU_ENTRY *)Entry);
  }
  return Entry;
}

VOID ScanLoader(VOID)
{
  UINTN                   VolumeIndex;
  REFIT_VOLUME            *Volume;
  CHAR16                  FileName[256];
  LOADER_ENTRY            *Entry;
  EFI_STATUS              Status;
  EG_IMAGE                *Image;
  
  DBG("Scanning loaders...\n");
  
  for (VolumeIndex = 0; VolumeIndex < VolumesCount; VolumeIndex++) {
    Volume = Volumes[VolumeIndex];
    DBG("%2d: '%s'", VolumeIndex, Volume->VolName);
    if (Volume->RootDir == NULL) { // || Volume->VolName == NULL)
      DBG(" no file system\n", VolumeIndex);
      continue;
    }
    if (Volume->VolName == NULL) {
      Volume->VolName = L"Unknown";
    }
    
    // skip volume if its kind is configured as disabled
    if ((Volume->DiskKind == DISK_KIND_OPTICAL && (GlobalConfig.DisableFlags & DISABLE_FLAG_OPTICAL)) ||
        (Volume->DiskKind == DISK_KIND_EXTERNAL && (GlobalConfig.DisableFlags & DISABLE_FLAG_EXTERNAL)) ||
        (Volume->DiskKind == DISK_KIND_INTERNAL && (GlobalConfig.DisableFlags & DISABLE_FLAG_INTERNAL)) ||
        (Volume->DiskKind == DISK_KIND_FIREWIRE && (GlobalConfig.DisableFlags & DISABLE_FLAG_FIREWIRE)))
    {
      DBG(" hidden\n");
      continue;
    }
    
    if (Volume->Hidden) {
      DBG(" hidden\n");
      continue;
    }
    DBG("\n");
    
    // check for Mac OS X boot loader
    StrCpy(FileName, MACOSX_LOADER_PATH);
    if (FileExists(Volume->RootDir, FileName)) {
      //     Print(L"  - Mac OS X boot file found\n");
      Volume->BootType = BOOTING_BY_EFI;
      // check for Mac OS X Boot target
      Status = GetRootUUID(Volume);
      if(!EFI_ERROR(Status)) {
        //Volume->OSType = OSTYPE_BOOT_OSX;
        if (isFirstRootUUID(Volume))
          Entry = AddLoaderEntry(FileName, NULL, NULL, L"Mac OS X", Volume, NULL, NULL, OSTYPE_OSX, 0, 0);
      }
      else {
        Entry = AddLoaderEntry(FileName, NULL, NULL, L"Mac OS X", Volume, NULL, NULL, OSTYPE_OSX, 0, 0);
      }
      //     continue; //boot MacOSX only
    }
    //crazybirdy
    //============ add in begin ============
    // check for Mac OS X Install Data
    StrCpy(FileName, L"\\OS X Install Data\\boot.efi");
    if (FileExists(Volume->RootDir, FileName)) {
      Volume->BootType = BOOTING_BY_EFI;
      Entry = AddLoaderEntry(FileName, NULL, NULL, L"OS X Install", Volume, NULL, NULL, OSTYPE_OSX_INSTALLER, 0, 0);
      continue; //boot MacOSX only
    }
    // check for Mac OS X Install Data
    StrCpy(FileName, L"\\Mac OS X Install Data\\boot.efi");
    if (FileExists(Volume->RootDir, FileName)) {
      Volume->BootType = BOOTING_BY_EFI;
      Entry = AddLoaderEntry(FileName, NULL, NULL, L"Mac OS X Install", Volume, NULL, NULL, OSTYPE_OSX_INSTALLER, 0, 0);
      continue; //boot MacOSX only
    }
    // dmazar: ML install from Lion to empty partition
    // starting (Lion) partition: /.IABootFiles with boot.efi and kernelcache,
    // and with DMGs used from Install app.
    // destination partition: just logs and config
    StrCpy(FileName, L"\\.IABootFiles\\boot.efi");
    if (FileExists(Volume->RootDir, FileName)) {
      Volume->BootType = BOOTING_BY_EFI;
      Entry = AddLoaderEntry(FileName, NULL, NULL, L"OS X Install", Volume, NULL, NULL, OSTYPE_OSX_INSTALLER, 0, 0);
      //continue; //boot MacOSX only
    }
    //============ add in end ============
    
    // check for Mac OS X Recovery Boot
    StrCpy(FileName,  L"\\com.apple.recovery.boot\\boot.efi");
    if (FileExists(Volume->RootDir, FileName)) {
      Volume->BootType = BOOTING_BY_EFI;
      Entry = AddLoaderEntry(FileName, NULL, NULL, L"Recovery", Volume, NULL, NULL, OSTYPE_RECOVERY, 0, 0);
      continue; //boot recovery only
    }
    
    // Sometimes, on some systems (HP UEFI, if Win is installed first)
    // it is needed to get rid of bootmgfw.efi to allow starting of
    // Clover as /efi/boot/bootx64.efi from HD. We can do that by renaming
    // bootmgfw.efi to bootmgfw-orig.efi
    StrCpy(FileName, L"\\EFI\\Microsoft\\Boot\\bootmgfw-orig.efi");
    if (FileExists(Volume->RootDir, FileName)) {
      //     Print(L"  - Microsoft boot menu found\n");
      Volume->BootType = BOOTING_BY_EFI;
      Entry = AddLoaderEntry(FileName, L"", NULL, L"Microsoft EFI boot menu", Volume, NULL, NULL, OSTYPE_WINEFI, 0, 0);
      //     continue;
      
    } else {
      // check for Microsoft boot loader/menu
      // If there is bootmgfw-orig.efi, then do not check for bootmgfw.efi
      // since on some systems this will actually be CloverX64.efi
      // renamed to bootmgfw.efi
      StrCpy(FileName, L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi");
      if (FileExists(Volume->RootDir, FileName)) {
        //     Print(L"  - Microsoft boot menu found\n");
        Volume->BootType = BOOTING_BY_EFI;
        Entry = AddLoaderEntry(FileName, L"", NULL, L"Microsoft EFI boot menu", Volume, NULL, NULL, OSTYPE_WINEFI, 0, 0);
        //     continue;
      }
      
    }
    
    // check for Microsoft boot loader/menu
    StrCpy(FileName, L"\\bootmgr.efi");
    if (FileExists(Volume->RootDir, FileName)) {
      //     Print(L"  - Microsoft boot menu found\n");
      Volume->BootType = BOOTING_BY_EFI;
      Entry = AddLoaderEntry(FileName, L"", NULL, L"Microsoft EFI boot menu", Volume, NULL, NULL, OSTYPE_WINEFI, 0, 0);
      continue;
    }
    
    // check for Microsoft boot loader/menu on CDROM
    StrCpy(FileName, L"\\EFI\\MICROSOFT\\BOOT\\cdboot.efi");
    if (FileExists(Volume->RootDir, FileName)) {
      //     Print(L"  - Microsoft boot menu found\n");
      Volume->BootType = BOOTING_BY_EFI;
      Entry = AddLoaderEntry(FileName, L"", NULL, L"Microsoft EFI boot menu", Volume, NULL, NULL, OSTYPE_WINEFI, 0, 0);
      continue;
    }
    
    
    // check for grub boot loader/menu
#if defined(MDE_CPU_X64)
    StrCpy(FileName, L"\\EFI\\grub\\grubx64.efi");
#else
    StrCpy(FileName, L"\\EFI\\grub\\grub.efi");
#endif
    if (FileExists(Volume->RootDir, FileName)) {
      // Sothor - we know what icon we are looking for lets just load it now
      Image = LoadOSIcon(L"grub,linux", L"unknown", 128, FALSE, TRUE);
      Volume->BootType = BOOTING_BY_EFI;
      Entry = AddLoaderEntry(FileName, L"", NULL, L"Grub EFI boot menu", Volume, Image, NULL, OSTYPE_LIN, OSFLAG_NODEFAULTARGS, 0);
    }
    
    // check for Gentoo boot loader/menu
#if defined(MDE_CPU_X64)
    StrCpy(FileName, L"\\EFI\\Gentoo\\grubx64.efi");
#else
    StrCpy(FileName, L"\\EFI\\Gentoo\\grub.efi");
#endif
    if (FileExists(Volume->RootDir, FileName)) {
      // Sothor - we know what icon we are looking for lets just load it now
      Image = LoadOSIcon(L"gentoo,linux", L"unknown", 128, FALSE, TRUE);
      Volume->BootType = BOOTING_BY_EFI;
      Entry = AddLoaderEntry(FileName, L"", NULL, L"Gentoo EFI boot menu", Volume, Image, NULL, OSTYPE_LIN, OSFLAG_NODEFAULTARGS, 0);
    }
    
    // check for Gentoo kernel
#if defined(MDE_CPU_X64)
    StrCpy(FileName, L"\\EFI\\Gentoo\\kernelx64.efi");
#else
    StrCpy(FileName, L"\\EFI\\Gentoo\\kernel.efi");
#endif
    if (FileExists(Volume->RootDir, FileName)) {
      // Sothor - we know what icon we are looking for lets just load it now
      Image = LoadOSIcon(L"gentoo,linux", L"unknown", 128, FALSE, TRUE);
      Volume->BootType = BOOTING_BY_EFI;
      Entry = AddLoaderEntry(FileName, L"", NULL, L"Gentoo EFI kernel", Volume, Image, NULL, OSTYPE_LIN, OSFLAG_NODEFAULTARGS, 0);
    }
    
    // check for Redhat boot loader/menu
#if defined(MDE_CPU_X64)
    StrCpy(FileName, L"\\EFI\\RedHat\\grubx64.efi");
#else
    StrCpy(FileName, L"\\EFI\\RedHat\\grub.efi");
#endif
    if (FileExists(Volume->RootDir, FileName)) {
      // Sothor - we know what icon we are looking for lets just load it now
      Image = LoadOSIcon(L"redhat,linux", L"unknown", 128, FALSE, TRUE);
      Volume->BootType = BOOTING_BY_EFI;
      Entry = AddLoaderEntry(FileName, L"", NULL, L"RedHat EFI boot menu", Volume, Image, NULL, OSTYPE_LIN, OSFLAG_NODEFAULTARGS, 0);
    }
    
    // check for Ubuntu boot loader/menu
#if defined(MDE_CPU_X64)
    StrCpy(FileName, L"\\EFI\\Ubuntu\\grubx64.efi");
#else
    StrCpy(FileName, L"\\EFI\\Ubuntu\\grub.efi");
#endif
    if (FileExists(Volume->RootDir, FileName)) {
      // Sothor - we know what icon we are looking for lets just load it now
      Image = LoadOSIcon(L"ubuntu,linux", L"unknown", 128, FALSE, TRUE);
      Volume->BootType = BOOTING_BY_EFI;
      Entry = AddLoaderEntry(FileName, L"", NULL, L"Ubuntu EFI boot menu", Volume, Image, NULL, OSTYPE_LIN, OSFLAG_NODEFAULTARGS, 0);
    }
    
    // check for kubuntu boot loader/menu
#if defined(MDE_CPU_X64)
    StrCpy(FileName, L"\\EFI\\kubuntu\\grubx64.efi");
#else
    StrCpy(FileName, L"\\EFI\\kubuntu\\grub.efi");
#endif
    if (FileExists(Volume->RootDir, FileName)) {
      // Sothor - we know what icon we are looking for lets just load it now
      Image = LoadOSIcon(L"kubuntu,linux", L"unknown", 128, FALSE, TRUE);
      Volume->BootType = BOOTING_BY_EFI;
      Entry = AddLoaderEntry(FileName, L"", NULL, L"kubuntu EFI boot menu", Volume, Image, NULL, OSTYPE_LIN, OSFLAG_NODEFAULTARGS, 0);
    }
    
    // check for Linux Mint boot loader/menu
#if defined(MDE_CPU_X64)
    StrCpy(FileName, L"\\EFI\\Linuxmint\\grubx64.efi");
#else
    StrCpy(FileName, L"\\EFI\\Linuxmint\\grub.efi");
#endif
    if (FileExists(Volume->RootDir, FileName)) {
      // Sothor - we know what icon we are looking for lets just load it now
      Image = LoadOSIcon(L"mint,linux", L"unknown", 128, FALSE, TRUE);
      Volume->BootType = BOOTING_BY_EFI;
      Entry = AddLoaderEntry(FileName, L"", NULL, L"Linux Mint EFI boot menu", Volume, Image, NULL, OSTYPE_LIN, OSFLAG_NODEFAULTARGS, 0);
    }
    
    // check for Fedora boot loader/menu
#if defined(MDE_CPU_X64)
    StrCpy(FileName, L"\\EFI\\Fedora\\grubx64.efi");
#else
    StrCpy(FileName, L"\\EFI\\Fedora\\grub.efi");
#endif
    if (FileExists(Volume->RootDir, FileName)) {
      // Sothor - we know what icon we are looking for lets just load it now
      Image = LoadOSIcon(L"fedora,linux", L"unknown", 128, FALSE, TRUE);
      Volume->BootType = BOOTING_BY_EFI;
      Entry = AddLoaderEntry(FileName, L"", NULL, L"Fedora EFI boot menu", Volume, Image, NULL, OSTYPE_LIN, OSFLAG_NODEFAULTARGS, 0);
    }
    
    // check for OpenSuse boot loader/menu
    StrCpy(FileName, L"\\EFI\\SuSe\\elilo.efi");
    if (FileExists(Volume->RootDir, FileName)) {
      // Sothor - we know what icon we are looking for lets just load it now
      Image = LoadOSIcon(L"suse,linux", L"unknown", 128, FALSE, TRUE);
      Volume->BootType = BOOTING_BY_EFI;
      Entry = AddLoaderEntry(FileName, L"", NULL, L"OpenSuse EFI boot menu", Volume, Image, NULL, OSTYPE_LIN, OSFLAG_NODEFAULTARGS, 0);
    }
    
#if defined(MDE_CPU_X64)
    StrCpy(FileName, L"\\EFI\\opensuse\\grubx64.efi");
#else
    StrCpy(FileName, L"\\EFI\\opensuse\\grub.efi");
#endif
    if (FileExists(Volume->RootDir, FileName)) {
      // Sothor - we know what icon we are looking for lets just load it now
      Image = LoadOSIcon(L"suse,linux", L"unknown", 128, FALSE, TRUE);
      Volume->BootType = BOOTING_BY_EFI;
      Entry = AddLoaderEntry(FileName, L"", NULL, L"OpenSuse EFI boot menu", Volume, Image, NULL, OSTYPE_LIN, OSFLAG_NODEFAULTARGS, 0);
    }
    
    // check for archlinux boot loader/menu
#if defined(MDE_CPU_X64)
    StrCpy(FileName, L"\\EFI\\arch\\grubx64.efi");
#else
    StrCpy(FileName, L"\\EFI\\arch\\grub.efi");
#endif
    if (FileExists(Volume->RootDir, FileName)) {
      // Sothor - we know what icon we are looking for lets just load it now
      Image = LoadOSIcon(L"arch,linux", L"unknown", 128, FALSE, TRUE);
      Volume->BootType = BOOTING_BY_EFI;
      Entry = AddLoaderEntry(FileName, L"", NULL, L"ArchLinux EFI boot menu", Volume, Image, NULL, OSTYPE_LIN, OSFLAG_NODEFAULTARGS, 0);
    }
    
#if defined(MDE_CPU_X64)
    StrCpy(FileName, L"\\EFI\\arch_grub\\grubx64.efi");
#else
    StrCpy(FileName, L"\\EFI\\arch_grub\\grub.efi");
#endif
    if (FileExists(Volume->RootDir, FileName)) {
      // Sothor - we know what icon we are looking for lets just load it now
      Image = LoadOSIcon(L"arch,linux", L"unknown", 128, FALSE, TRUE);
      Volume->BootType = BOOTING_BY_EFI;
      Entry = AddLoaderEntry(FileName, L"", NULL, L"ArchLinux EFI boot menu", Volume, Image, NULL, OSTYPE_LIN, OSFLAG_NODEFAULTARGS, 0);
    }
    
#if defined(MDE_CPU_X64)
    StrCpy(FileName, L"\\EFI\\BOOT\\BOOTX64.efi");
#else
    StrCpy(FileName, L"\\EFI\\BOOT\\BOOTIA32.efi");
#endif
    //     DBG("search for  optical UEFI\n");
    if (FileExists(Volume->RootDir, FileName) && Volume->DiskKind == DISK_KIND_OPTICAL) {
      Volume->BootType = BOOTING_BY_EFI;
      AddLoaderEntry(FileName, L"", NULL, L"UEFI optical", Volume, NULL, NULL, OSTYPE_OTHER, 0, 0);
      //      continue;
    }
    
    //     DBG("search for internal UEFI\n");
    if (FileExists(Volume->RootDir, FileName) && Volume->DiskKind == DISK_KIND_INTERNAL) {
      Volume->BootType = BOOTING_BY_EFI;
      AddLoaderEntry(FileName, L"", NULL, L"UEFI internal", Volume, NULL, NULL, OSTYPE_OTHER, 0, 0);
      //      continue;
    }
    
    //    DBG("search for external UEFI\n");
    if (FileExists(Volume->RootDir, FileName) && Volume->DiskKind == DISK_KIND_EXTERNAL) {
      Volume->BootType = BOOTING_BY_EFI;
      AddLoaderEntry(FileName, L"", NULL, L"UEFI external", Volume, NULL, NULL, OSTYPE_OTHER, 0, 0);
      //      continue;
    }
  }
}

static UINT8 GetOSTypeFromPath(IN CHAR16 *Path, IN UINT8 OSType)
{
  if (Path == NULL) {
    return (OSType == 0) ? OSTYPE_OTHER : OSType;
  }
  if ((StriCmp(Path, MACOSX_LOADER_PATH) == 0) ||
      (StriCmp(Path, L"\\OS X Install Data\\boot.efi") == 0) ||
      (StriCmp(Path, L"\\Mac OS X Install Data\\boot.efi") == 0) ||
      (StriCmp(Path, L"\\.IABootFiles\\boot.efi") == 0)) {
    return OSType;
  } else if (StriCmp(Path, L"\\com.apple.recovery.boot\\boot.efi") == 0) {
    return OSTYPE_RECOVERY;
  } else if ((StriCmp(Path, L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi") == 0) ||
             (StriCmp(Path, L"\\bootmgr.efi") == 0) ||
             (StriCmp(Path, L"\\EFI\\MICROSOFT\\BOOT\\cdboot.efi") == 0)) {
    return OSTYPE_WINEFI;
  } else {
    UINTN Index = 0;
    while (Index < LinuxEntryPathsCount) {
      if (StriCmp(Path, LinuxEntryPaths[Index]) == 0) {
        return OSTYPE_LIN;
      }
      ++Index;
    }
  }
  return (OSType == 0) ? OSTYPE_OTHER : OSType;
}

static LOADER_ENTRY *AddCustomEntry(IN UINTN                CustomIndex,
                                    IN CHAR16              *CustomPath,
                                    IN CUSTOM_LOADER_ENTRY *Custom,
                                    IN BOOLEAN              IsSubEntry)
{
  UINTN                VolumeIndex;
  REFIT_VOLUME        *Volume;
  CUSTOM_LOADER_ENTRY *CustomSubEntry;
  LOADER_ENTRY        *Entry = NULL, *SubEntry;
  EG_IMAGE            *Image, *DriveImage;
  EFI_GUID            *Guid = NULL;
  UINT64               VolumeSize;
  UINT8                OSType;
  
  if (CustomPath == NULL) {
    DBG("Custom %sentry %d skipped because it didn't have a path.\n", IsSubEntry ? L"sub " : L"", CustomIndex);
    return NULL;
  }
  if (OSFLAG_ISSET(Custom->Flags, OSFLAG_DISABLED)) {
    DBG("Custom %sentry %d skipped because it is disabled.\n", IsSubEntry ? L"sub " : L"", CustomIndex);
    return NULL;
  }
  if (!gSettings.ShowHiddenEntries && OSFLAG_ISSET(Custom->Flags, OSFLAG_HIDDEN)) {
    DBG("Custom %sentry %d skipped because it is hidden.\n", IsSubEntry ? L"sub " : L"", CustomIndex);
    return NULL;
  }
  if (Custom->Volume) {
    if (Custom->Title) {
      if (CustomPath) {
        DBG("Custom %sentry %d \"%s\" \"%s\" \"%s\" (%d) 0x%X matching \"%s\" ...\n", IsSubEntry ? L"sub " : L"", CustomIndex, Custom->Title, CustomPath, ((Custom->Options != NULL) ? Custom->Options : L""), Custom->Type, Custom->Flags, Custom->Volume);
      } else {
        DBG("Custom %sentry %d \"%s\" \"%s\" (%d) 0x%X matching \"%s\" ...\n", IsSubEntry ? L"sub " : L"", CustomIndex, Custom->Title, ((Custom->Options != NULL) ? Custom->Options : L""), Custom->Type, Custom->Flags, Custom->Volume);
      }
    } else if (CustomPath) {
      DBG("Custom %sentry %d \"%s\" \"%s\" (%d) 0x%X matching \"%s\" ...\n", IsSubEntry ? L"sub " : L"", CustomIndex, CustomPath, ((Custom->Options != NULL) ? Custom->Options : L""), Custom->Type, Custom->Flags, Custom->Volume);
    } else {
      DBG("Custom %sentry %d \"%s\" (%d) 0x%X matching \"%s\" ...\n", IsSubEntry ? L"sub " : L"", CustomIndex, ((Custom->Options != NULL) ? Custom->Options : L""), Custom->Type, Custom->Flags, Custom->Volume);
    }
  } else if (CustomPath) {
    DBG("Custom %sentry %d \"%s\" \"%s\" (%d) 0x%X matching all volumes ...\n", IsSubEntry ? L"sub " : L"", CustomIndex, CustomPath, ((Custom->Options != NULL) ? Custom->Options : L""), Custom->Type, Custom->Flags);
  } else {
    DBG("Custom %sentry %d \"%s\" (%d) 0x%X matching all volumes ...\n", IsSubEntry ? L"sub " : L"", CustomIndex, ((Custom->Options != NULL) ? Custom->Options : L""), Custom->Type, Custom->Flags);
  }
  for (VolumeIndex = 0; VolumeIndex < VolumesCount; ++VolumeIndex) {
    Volume = Volumes[VolumeIndex];
    
    if (Volume->RootDir == NULL) {
      continue;
    }
    if (Volume->VolName == NULL) {
      Volume->VolName = L"Unknown";
    }
    
    DBG("   Checking volume \"%s\" (%s) ... ", Volume->VolName, Volume->DevicePathString);
    
    // skip volume if its kind is configured as disabled
    if ((Volume->DiskKind == DISK_KIND_OPTICAL && (GlobalConfig.DisableFlags & DISABLE_FLAG_OPTICAL)) ||
        (Volume->DiskKind == DISK_KIND_EXTERNAL && (GlobalConfig.DisableFlags & DISABLE_FLAG_EXTERNAL)) ||
        (Volume->DiskKind == DISK_KIND_INTERNAL && (GlobalConfig.DisableFlags & DISABLE_FLAG_INTERNAL)) ||
        (Volume->DiskKind == DISK_KIND_FIREWIRE && (GlobalConfig.DisableFlags & DISABLE_FLAG_FIREWIRE)))
    {
      DBG("skipped because media is disabled\n");
      continue;
    }
    
    if (Custom->VolumeType != 0) {
      if ((Volume->DiskKind == DISK_KIND_OPTICAL && ((Custom->VolumeType & DISABLE_FLAG_OPTICAL) == 0)) ||
          (Volume->DiskKind == DISK_KIND_EXTERNAL && ((Custom->VolumeType & DISABLE_FLAG_EXTERNAL) == 0)) ||
          (Volume->DiskKind == DISK_KIND_INTERNAL && ((Custom->VolumeType & DISABLE_FLAG_INTERNAL) == 0)) ||
          (Volume->DiskKind == DISK_KIND_FIREWIRE && ((Custom->VolumeType & DISABLE_FLAG_FIREWIRE) == 0))) {
        DBG("skipped because media is ignored\n");
        continue;
      }
    }
    
    if (Volume->Hidden) {
      DBG("skipped because volume is hidden\n");
      continue;
    }
    // Check for exact volume matches
    OSType = (Custom->Type == 0) ? GetOSTypeFromPath(CustomPath, 0/*Volume->OSType FIXME: Sothor - what should be here? We don't care what legacy os type is found */) : Custom->Type;
    if (Custom->Volume) {
      
      if ((StrStr(Volume->DevicePathString, Custom->Volume) == NULL) &&
          ((Volume->VolName == NULL) || (StrStr(Volume->VolName, Custom->Volume) == NULL))) {
        DBG("skipped\n");
        continue;
      }
      // NOTE: Sothor - We dont care about legacy OS type // Check if the volume should be of certain os type
      //if ((Custom->Type != 0) && (Volume->OSType != 0) && !OSTYPE_COMPARE(OSType, Volume->OSType)) {
      //  DBG("skipped because wrong type (%d != %d)\n", OSType, Volume->OSType);
      //  continue;
      //}
      //} else if ((Custom->Type != 0) && (Volume->OSType != 0) && !OSTYPE_COMPARE(OSType, Volume->OSType)) {
      //DBG("skipped because wrong type (%d != %d)\n", OSType, Volume->OSType);
      //continue;
    }
    // Check the volume is readable and the entry exists on the volume
    if (Volume->RootDir == NULL) {
      DBG("skipped because filesystem is not readable\n");
      continue;
    }
    if (!FileExists(Volume->RootDir, CustomPath)) {
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
            if (Image == NULL) {
              Image = LoadOSIcon(Custom->ImagePath, L"unknown", 128, FALSE, FALSE);
            }
          }
        }
      }
    }
    // Change to custom drive image if needed
    DriveImage = Custom->DriveImage;
    if ((DriveImage == NULL) && Custom->DriveImagePath) {
      DriveImage = egLoadImage(Volume->RootDir, Custom->DriveImagePath, TRUE);
      if (DriveImage == NULL) {
        DriveImage = egLoadImage(ThemeDir, Custom->DriveImagePath, TRUE);
        if (DriveImage == NULL) {
          DriveImage = egLoadImage(SelfDir, Custom->DriveImagePath, TRUE);
          if (DriveImage == NULL) {
            DriveImage = egLoadImage(SelfRootDir, Custom->DriveImagePath, TRUE);
            if (DriveImage == NULL) {
              DriveImage = LoadBuiltinIcon(Custom->DriveImagePath);
            }
          }
        }
      }
    }
    // Change custom drive image if needed
    // Update volume boot type
    Volume->BootType = BOOTING_BY_EFI;
    DBG("match!\n");
    // Create a legacy entry for this volume
    if (OSFLAG_ISUNSET(Custom->Flags, OSFLAG_NODEFAULTMENU)) {
      Entry = AddLoaderEntry2(CustomPath, Custom->Options, Custom->FullTitle, Custom->Title, Volume, Image, DriveImage, OSType, Custom->Flags, Custom->Hotkey, TRUE);
    } else {
      Entry = CreateLoaderEntry(CustomPath, Custom->Options, Custom->FullTitle, Custom->Title, Volume, Image, DriveImage, OSType, Custom->Flags, Custom->Hotkey, TRUE);
      if (Entry) {
        if (Custom->SubEntries) {
          UINTN CustomSubIndex = 0;
          // Add subscreen
          REFIT_MENU_SCREEN *SubScreen = AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
          if (SubScreen) {
            SubScreen->Title = PoolPrint(L"Boot Options for %s on %s", (Custom->Title != NULL) ? Custom->Title : CustomPath, Entry->VolName);
            SubScreen->TitleImage = Entry->me.Image;
            SubScreen->ID = OSType + 20;
            SubScreen->AnimeRun = GetAnime(SubScreen);
            VolumeSize = RShiftU64(MultU64x32(Volume->BlockIO->Media->LastBlock, Volume->BlockIO->Media->BlockSize), 20);
            AddMenuInfoLine(SubScreen, PoolPrint(L"Volume size: %dMb", VolumeSize));
            AddMenuInfoLine(SubScreen, DevicePathToStr(Entry->DevicePath));
            Guid = FindGPTPartitionGuidInDevicePath(Volume->DevicePath);
            if (Guid) {
              CHAR8 *GuidStr = AllocateZeroPool(50);
              AsciiSPrint(GuidStr, 50, "%g", Guid);
              AddMenuInfoLine(SubScreen, PoolPrint(L"UUID: %a", GuidStr));
              FreePool(GuidStr);
            }
            // Create sub entries
            for (CustomSubEntry = Custom->SubEntries; CustomSubEntry; CustomSubEntry = CustomSubEntry->Next) {
              SubEntry = AddCustomEntry(CustomSubIndex++, (CustomSubEntry->Path != NULL) ? CustomSubEntry->Path : CustomPath, CustomSubEntry, TRUE);
              if (SubEntry) {
                AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
              }
            }
            AddMenuEntry(SubScreen, &MenuEntryReturn);
            Entry->me.SubScreen = SubScreen;
          }
        }
      }
    }
  }
  return Entry;
}

// Add custom entries
VOID AddCustomEntries(VOID)
{
  CUSTOM_LOADER_ENTRY *Custom;
  UINTN                i = 0;
  
  DBG("Custom entries start\n");
  // Traverse the custom entries
  for (Custom = gSettings.CustomEntries; Custom; ++i, Custom = Custom->Next) {
    LOADER_ENTRY *Entry = NULL;
    
    if ((Custom->Path == NULL) && (Custom->Type != 0)) {
      if (OSTYPE_IS_OSX(Custom->Type)) {
        Entry = AddCustomEntry(i, MACOSX_LOADER_PATH, Custom, FALSE);
      } else if (OSTYPE_IS_OSX_RECOVERY(Custom->Type)) {
        Entry = AddCustomEntry(i, L"\\com.apple.recovery.boot\\boot.efi", Custom, FALSE);
      } else if (OSTYPE_IS_OSX_INSTALLER(Custom->Type)) {
        UINTN Index = 0;
        while (Index < OSXInstallerPathsCount) {
          Entry = AddCustomEntry(i, OSXInstallerPaths[Index++], Custom, FALSE);
        }
      } else if (OSTYPE_IS_WINDOWS(Custom->Type)) {
        Entry = AddCustomEntry(i, L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi", Custom, FALSE);
      } else if (OSTYPE_IS_LINUX(Custom->Type)) {
        UINTN Index = 0;
        while (Index < LinuxEntryPathsCount) {
          Entry = AddCustomEntry(i, LinuxEntryPaths[Index++], Custom, FALSE);
        }
      } else {
#if defined(MDE_CPU_X64)
        Entry = AddCustomEntry(i, L"\\EFI\\BOOT\\BOOTX64.efi", Custom, FALSE);
#else
        Entry = AddCustomEntry(i, L"\\EFI\\BOOT\\BOOTIA32.efi", Custom, FALSE);
#endif
      }
    } else {
      Entry = AddCustomEntry(i, Custom->Path, Custom, FALSE);
    }
    if (Entry != NULL) {
      AddMenuEntry(&MainMenu, (REFIT_MENU_ENTRY *)Entry);
    }
  }
  DBG("Custom entries finish\n");
}