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

#define MACOSX_LOADER_PATH L"\\System\\Library\\CoreServices\\boot.efi"

#define LINUX_ISSUE_PATH L"\\etc\\issue"
#define LINUX_BOOT_PATH L"\\boot"
#define LINUX_BOOT_ALT_PATH L"\\boot"
#define LINUX_LOADER_PATH L"vmlinuz"
#define LINUX_FULL_LOADER_PATH LINUX_BOOT_PATH L"\\" LINUX_LOADER_PATH
#define LINUX_LOADER_SEARCH_PATH L"vmlinuz*"
#define LINUX_DEFAULT_OPTIONS L"ro add_efi_memmap quiet splash vt.handoff=7"

#if defined(MDE_CPU_X64)
#define BOOT_LOADER_PATH L"\\EFI\\BOOT\\BOOTX64.efi"
#else
#define BOOT_LOADER_PATH L"\\EFI\\BOOT\\BOOTIA32.efi"
#endif

// Linux loader path data
typedef struct LINUX_PATH_DATA
{
   CHAR16 *Path;
   CHAR16 *Title;
   CHAR16 *Icon;
   CHAR8  *Issue;
} LINUX_PATH_DATA;

STATIC LINUX_PATH_DATA LinuxEntryData[] = {
#if defined(MDE_CPU_X64)
  { L"\\EFI\\grub\\grubx64.efi", L"Grub EFI boot menu", L"grub,linux" },
  { L"\\EFI\\Gentoo\\grubx64.efi", L"Gentoo EFI boot menu", L"gentoo,linux", "Gentoo" },
  { L"\\EFI\\Gentoo\\kernelx64.efi", L"Gentoo EFI kernel", L"gentoo,linux" },
  { L"\\EFI\\RedHat\\grubx64.efi", L"RedHat EFI boot menu", L"redhat,linux", "Redhat" },
  { L"\\EFI\\ubuntu\\grubx64.efi", L"Ubuntu EFI boot menu", L"ubuntu,linux", "Ubuntu" },
  { L"\\EFI\\kubuntu\\grubx64.efi", L"kubuntu EFI boot menu", L"kubuntu,linux", "kubuntu" },
  { L"\\EFI\\LinuxMint\\grubx64.efi", L"Linux Mint EFI boot menu", L"mint,linux", "Linux Mint" },
  { L"\\EFI\\Fedora\\grubx64.efi", L"Fedora EFI boot menu", L"fedora,linux", "Fedora" },
  { L"\\EFI\\opensuse\\grubx64.efi", L"OpenSuse EFI boot menu", L"suse,linux", "openSUSE" },
  { L"\\EFI\\arch\\grubx64.efi", L"ArchLinux EFI boot menu", L"arch,linux" },
  { L"\\EFI\\arch_grub\\grubx64.efi", L"ArchLinux EFI boot menu", L"arch,linux" },
#else
  { L"\\EFI\\grub\\grub.efi", L"Grub EFI boot menu", L"grub,linux" },
  { L"\\EFI\\Gentoo\\grub.efi", L"Gentoo EFI boot menu", L"gentoo,linux", "Gentoo" },
  { L"\\EFI\\Gentoo\\kernel.efi", L"Gentoo EFI kernel", L"gentoo,linux" },
  { L"\\EFI\\RedHat\\grub.efi", L"RedHat EFI boot menu", L"redhat,linux", "Redhat" },
  { L"\\EFI\\ubuntu\\grub.efi", L"Ubuntu EFI boot menu", L"ubuntu,linux", "Ubuntu" },
  { L"\\EFI\\kubuntu\\grub.efi", L"kubuntu EFI boot menu", L"kubuntu,linux", "kubuntu" },
  { L"\\EFI\\LinuxMint\\grub.efi", L"Linux Mint EFI boot menu", L"mint,linux", "Linux Mint" },
  { L"\\EFI\\Fedora\\grub.efi", L"Fedora EFI boot menu", L"fedora,linux", "Fedora" },
  { L"\\EFI\\opensuse\\grub.efi", L"OpenSuse EFI boot menu", L"suse,linux", "openSUSE" },
  { L"\\EFI\\arch\\grub.efi", L"ArchLinux EFI boot menu", L"arch,linux" },
  { L"\\EFI\\arch_grub\\grub.efi", L"ArchLinux EFI boot menu", L"arch,linux" },
#endif
  { L"\\EFI\\SuSe\\elilo.efi", L"OpenSuse EFI boot menu", L"suse,linux" },
};
STATIC CONST UINTN LinuxEntryDataCount = (sizeof(LinuxEntryData) / sizeof(LINUX_PATH_DATA));

// OS X installer paths
STATIC CHAR16 *OSXInstallerPaths[] = {
  L"\\Mac OS X Install Data\\boot.efi",
  L"\\OS X Install Data\\boot.efi",
  L"\\.IABootFiles\\boot.efi"
};
STATIC CONST UINTN OSXInstallerPathsCount = (sizeof(OSXInstallerPaths) / sizeof(CHAR16 *));

STATIC INTN TimeCmp(IN EFI_TIME *Time1,
                    IN EFI_TIME *Time2)
{
   INTN Comparison;
   if (Time1 == NULL) {
     if (Time2 == NULL) {
       return 0;
     } else {
       return -1;
     }
   } else if (Time2 == NULL) {
     return 1;
   }
   Comparison = Time1->Year - Time2->Year;
   if (Comparison == 0) {
     Comparison = Time1->Month - Time2->Month;
     if (Comparison == 0) {
       Comparison = Time1->Day - Time2->Day;
       if (Comparison == 0) {
         Comparison = Time1->Hour - Time2->Hour;
         if (Comparison == 0) {
           Comparison = Time1->Minute - Time2->Minute;
           if (Comparison == 0) {
             Comparison = Time1->Second - Time2->Second;
             if (Comparison == 0) {
               Comparison = Time1->Nanosecond - Time2->Nanosecond;
             }
           }
         }
       }
     }
   }
   return Comparison;
}

UINT8 GetOSTypeFromPath(IN CHAR16 *Path)
{
  if (Path == NULL) {
    return OSTYPE_OTHER;
  }
  if (StriCmp(Path, MACOSX_LOADER_PATH) == 0) {
    return OSTYPE_OSX;
  } else if ((StriCmp(Path, L"\\OS X Install Data\\boot.efi") == 0) ||
             (StriCmp(Path, L"\\Mac OS X Install Data\\boot.efi") == 0) ||
             (StriCmp(Path, L"\\.IABootFiles\\boot.efi") == 0)) {
    return OSTYPE_OSX_INSTALLER;
  } else if (StriCmp(Path, L"\\com.apple.recovery.boot\\boot.efi") == 0) {
    return OSTYPE_RECOVERY;
  } else if ((StriCmp(Path, L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi") == 0) ||
             (StriCmp(Path, L"\\EFI\\Microsoft\\Boot\\bootmgfw-orig.efi") == 0) ||
             (StriCmp(Path, L"\\bootmgr.efi") == 0) ||
             (StriCmp(Path, L"\\EFI\\MICROSOFT\\BOOT\\cdboot.efi") == 0)) {
    return OSTYPE_WINEFI;
  } else if (StrniCmp(Path, LINUX_FULL_LOADER_PATH, StrLen(LINUX_FULL_LOADER_PATH)) == 0) {
    return OSTYPE_LINEFI;
  } else {
    UINTN Index = 0;
    while (Index < LinuxEntryDataCount) {
      if (StriCmp(Path, LinuxEntryData[Index].Path) == 0) {
        return OSTYPE_LIN;
      }
      ++Index;
    }
  }
  return OSTYPE_OTHER;
}

STATIC CHAR16 *LinuxIconNameFromPath(IN CHAR16            *Path,
                                     IN EFI_FILE_PROTOCOL *RootDir)
{
  UINTN Index = 0;
  while (Index < LinuxEntryDataCount) {
    if (StriCmp(Path, LinuxEntryData[Index].Path) == 0) {
      return LinuxEntryData[Index].Icon;
    }
    ++Index;
  }
  // Try to open the linux issue
  if ((RootDir != NULL) && (StrniCmp(Path, LINUX_FULL_LOADER_PATH, StrLen(LINUX_FULL_LOADER_PATH)) == 0)) {
    CHAR8 *Issue = NULL;
    UINTN  IssueLen = 0;
    if (!EFI_ERROR(egLoadFile(RootDir, LINUX_ISSUE_PATH, (UINT8 **)&Issue, &IssueLen)) && (Issue != NULL)) {
      if (IssueLen > 0) {
        for (Index = 0; Index < LinuxEntryDataCount; ++Index) {
          if ((LinuxEntryData[Index].Issue != NULL) &&
              (AsciiStrStr(Issue, LinuxEntryData[Index].Issue) != NULL)) {
            FreePool(Issue);
            return LinuxEntryData[Index].Icon;
          }
        }
      }
      FreePool(Issue);
    }
  }
  return L"linux";
}

STATIC CHAR16 *LinuxInitImagePath[] = {
   L"initrd%s",
   L"initrd.img%s",
   L"initrd%s.img",
   L"initramfs%s",
   L"initramfs.img%s",
   L"initramfs%s.img",
};
STATIC CONST UINTN LinuxInitImagePathCount = (sizeof(LinuxInitImagePath) / sizeof(CHAR16 *));

STATIC CHAR16 *LinuxKernelOptions(IN EFI_FILE_PROTOCOL *Dir,
                                  IN CHAR16            *Version,
                                  IN CHAR16            *PartUUID,
                                  IN CHAR16            *Options OPTIONAL)
{
  UINTN Index = 0;
  if ((Dir == NULL) || (PartUUID == NULL)) {
    return (Options == NULL) ? NULL : EfiStrDuplicate(Options);
  }
  while (Index < LinuxInitImagePathCount) {
    CHAR16 *InitRd = PoolPrint(LinuxInitImagePath[Index++], (Version == NULL) ? L"" : Version);
    if (InitRd != NULL) {
      if (FileExists(Dir, InitRd)) {
        CHAR16 *CustomOptions = PoolPrint(L"root=/dev/disk/by-partuuid/%s initrd=%s\\%s %s %s", PartUUID, LINUX_BOOT_ALT_PATH, InitRd, LINUX_DEFAULT_OPTIONS, (Options == NULL) ? L"" : Options);
        FreePool(InitRd);
        return CustomOptions;
      }
      FreePool(InitRd);
    }
  }
  return PoolPrint(L"root=/dev/disk/by-partuuid/%s %s %s", PartUUID, LINUX_DEFAULT_OPTIONS, (Options == NULL) ? L"" : Options);
}

STATIC BOOLEAN isFirstRootUUID(REFIT_VOLUME *Volume)
{
  UINTN         VolumeIndex;
  REFIT_VOLUME *scanedVolume;
  
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
STATIC EFI_STATUS GetOSXVolumeName(LOADER_ENTRY *Entry)
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


STATIC LOADER_ENTRY *CreateLoaderEntry(IN CHAR16 *LoaderPath, IN CHAR16 *LoaderOptions, IN CHAR16 *FullTitle, IN CHAR16 *LoaderTitle, IN REFIT_VOLUME *Volume,
                                       IN EG_IMAGE *Image, IN EG_IMAGE *DriveImage, IN UINT8 OSType, IN UINT8 Flags, IN CHAR16 Hotkey, EG_PIXEL *BootBgColor, IN BOOLEAN CustomEntry)
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
  if ((LoaderPath == NULL) || (*LoaderPath == 0) || (Volume == NULL)) {
    return NULL;
  }
  
  // Get the loader device path
  LoaderDevicePath = FileDevicePath(Volume->DeviceHandle, (*LoaderPath == '\\') ? (LoaderPath + 1) : LoaderPath);
  if (LoaderDevicePath == NULL) {
    return NULL;
  }
  LoaderDevicePathString = FileDevicePathToStr(LoaderDevicePath);
  if (LoaderDevicePathString == NULL) {
    return NULL;
  }
  
  // Ignore this loader if it's self path
  FilePathAsString = FileDevicePathFileToStr(SelfFullDevicePath);
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
              if (((Volume->DiskKind == DISK_KIND_INTERNAL) && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
                  ((Volume->DiskKind == DISK_KIND_EXTERNAL) && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
                  ((Volume->DiskKind == DISK_KIND_OPTICAL) && (Custom->VolumeType & VOLTYPE_OPTICAL)) ||
                  ((Volume->DiskKind == DISK_KIND_FIREWIRE) && (Custom->VolumeType & VOLTYPE_FIREWIRE))) {
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
          if (((Volume->DiskKind == DISK_KIND_INTERNAL) && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
              ((Volume->DiskKind == DISK_KIND_EXTERNAL) && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
              ((Volume->DiskKind == DISK_KIND_OPTICAL) && (Custom->VolumeType & VOLTYPE_OPTICAL)) ||
              ((Volume->DiskKind == DISK_KIND_FIREWIRE) && (Custom->VolumeType & VOLTYPE_FIREWIRE))) {
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
  Entry->me.Tag = TAG_LOADER;
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
  Entry->OSVersion = GetOSVersion(Entry);
  
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
      if (gSettings.NoCaches) {
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NOCACHES);
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
    case OSTYPE_LINEFI:
      OSIconName = LinuxIconNameFromPath(LoaderPath, Volume->RootDir);
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
  } else if ((Entry->VolName == NULL) || (StrLen(Entry->VolName) == 0)) {
    Entry->me.Title = PoolPrint(L"Boot %s from %s", (LoaderTitle != NULL) ? LoaderTitle : Basename(LoaderPath), Basename(Volume->DevicePathString));
  } else {
    Entry->me.Title = PoolPrint(L"Boot %s from %s", (LoaderTitle != NULL) ? LoaderTitle : Basename(LoaderPath), Entry->VolName);
  }
  Entry->me.ShortcutLetter = (Hotkey == 0) ? ShortcutLetter : Hotkey;
  
  // get custom volume icon if present

    if (GlobalConfig.CustomIcons && FileExists(Volume->RootDir, L"\\.VolumeIcon.icns")){
      Entry->me.Image = LoadIcns(Volume->RootDir, L"\\.VolumeIcon.icns", 128);
      DBG("using VolumeIcon.icns image from Volume\n");
    } else if (Image) {
      Entry->me.Image = Image;
    } else {      
      Entry->me.Image = LoadOSIcon(OSIconName, L"unknown", 128, FALSE, TRUE);
    }

  // Load DriveImage
  Entry->me.DriveImage = (DriveImage != NULL) ? DriveImage : ScanVolumeDefaultIcon(Volume, Entry->LoaderType);
  
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
  
  if (BootBgColor != NULL) {
    Entry->BootBgColor = BootBgColor;
  }
  DBG("found %s\n", Entry->DevicePathString);
  return Entry;
}

STATIC VOID AddDefaultMenu(IN LOADER_ENTRY *Entry)
{
  CHAR16            *FileName, *TempOptions;
  CHAR16            DiagsFileName[256];
  LOADER_ENTRY      *SubEntry;
  REFIT_MENU_SCREEN *SubScreen;
  REFIT_VOLUME      *Volume;
  UINT64            VolumeSize;
  EFI_GUID          *Guid = NULL;
  BOOLEAN           KernelIs64BitOnly;

  if (Entry == NULL) {
    return;
  }
  Volume = Entry->Volume;
  if (Volume == NULL) {
    return;
  }

  // Only kernels up to 10.7 have 32-bit mode
  KernelIs64BitOnly = (Entry->OSVersion == NULL || AsciiStrnCmp(Entry->OSVersion,"10.",3) != 0 || Entry->OSVersion[3] > '7');
  
  FileName = Basename(Entry->LoaderPath);
  
  // create the submenu
  SubScreen = AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
  SubScreen->Title = PoolPrint(L"Boot Options for %s on %s", Entry->me.Title, Entry->VolName);
  SubScreen->TitleImage = Entry->me.Image;
  SubScreen->ID = Entry->LoaderType + 20;
  //  DBG("get anime for os=%d\n", SubScreen->ID);
  SubScreen->AnimeRun = GetAnime(SubScreen);
  VolumeSize = RShiftU64(MultU64x32(Volume->BlockIO->Media->LastBlock, Volume->BlockIO->Media->BlockSize), 20);
  AddMenuInfoLine(SubScreen, PoolPrint(L"Volume size: %dMb", VolumeSize));
  AddMenuInfoLine(SubScreen, FileDevicePathToStr(Entry->DevicePath));
  Guid = FindGPTPartitionGuidInDevicePath(Volume->DevicePath);
  if (Guid) {
    CHAR8 *GuidStr = AllocateZeroPool(50);
    AsciiSPrint(GuidStr, 50, "%g", Guid);
    AddMenuInfoLine(SubScreen, PoolPrint(L"UUID: %a", GuidStr));
    FreePool(GuidStr);
  }
  AddMenuInfoLine(SubScreen, PoolPrint(L"Options: %s", Entry->LoadOptions));
  
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
    if (!KernelIs64BitOnly) {
      SubEntry = DuplicateLoaderEntry(Entry);
      if (SubEntry) {
        SubEntry->LoadOptions     = AddLoadOption(SubEntry->LoadOptions, L"arch=x86_64");
        SubEntry->me.Title        = L"Boot Mac OS X (64-bit)";
        AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
      }
    }
#endif
    if (!KernelIs64BitOnly) {
      SubEntry = DuplicateLoaderEntry(Entry);
      if (SubEntry) {
        SubEntry->me.Title        = L"Boot Mac OS X (32-bit)";
        SubEntry->LoadOptions     = AddLoadOption(SubEntry->LoadOptions, L"arch=i386");
        AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
      }
    }
    
    if (!(GlobalConfig.DisableFlags & HIDEUI_FLAG_SINGLEUSER)) {
      
#if defined(MDE_CPU_X64)
      if (KernelIs64BitOnly) {
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
      
      if (!KernelIs64BitOnly) {
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
    if (FileExists(Volume->RootDir, DiagsFileName) && !(GlobalConfig.DisableFlags & HIDEUI_FLAG_HWTEST)) {
      DBG("  - Apple Hardware Test found\n");
      
      // NOTE: Sothor - I'm not sure if to duplicate parent entry here.
      SubEntry = AllocateZeroPool(sizeof(LOADER_ENTRY));
      SubEntry->me.Title        = L"Run Apple Hardware Test";
      SubEntry->me.Tag          = TAG_LOADER;
      SubEntry->LoaderPath      = EfiStrDuplicate(DiagsFileName);
      SubEntry->Volume          = Volume;
      SubEntry->VolName         = EfiStrDuplicate(Entry->VolName);
      SubEntry->DevicePath      = FileDevicePath(Volume->DeviceHandle, SubEntry->LoaderPath);
      SubEntry->DevicePathString = EfiStrDuplicate(Entry->DevicePathString);
      SubEntry->Flags           = OSFLAG_SET(Entry->Flags, OSFLAG_USEGRAPHICS);
      SubEntry->me.AtClick      = ActionEnter;
      AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
    }
    
  } else if (Entry->LoaderType == OSTYPE_LINEFI) {
    BOOLEAN Quiet = (StrStr(Entry->LoadOptions, L"quiet") != NULL);
    BOOLEAN WithSplash = (StrStr(Entry->LoadOptions, L"splash") != NULL);
    SubEntry = DuplicateLoaderEntry(Entry);
    if (SubEntry) {
      FreePool(SubEntry->LoadOptions);
      if (Quiet) {
        SubEntry->me.Title    = PoolPrint(L"%s verbose", Entry->me.Title);
        SubEntry->LoadOptions = RemoveLoadOption(Entry->LoadOptions, L"quiet");
      } else {
        SubEntry->me.Title    = PoolPrint(L"%s quiet", Entry->me.Title);
        SubEntry->LoadOptions = AddLoadOption(Entry->LoadOptions, L"quiet");
      }
    }
    AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
    SubEntry = DuplicateLoaderEntry(Entry);
    if (SubEntry) {
      FreePool(SubEntry->LoadOptions);
      if (WithSplash) {
        SubEntry->me.Title    = PoolPrint(L"%s without splash", Entry->me.Title);
        SubEntry->LoadOptions = RemoveLoadOption(Entry->LoadOptions, L"splash");
      } else {
        SubEntry->me.Title    = PoolPrint(L"%s with splash", Entry->me.Title);
        SubEntry->LoadOptions = AddLoadOption(Entry->LoadOptions, L"splash");
      }
    }
    AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
    SubEntry = DuplicateLoaderEntry(Entry);
    if (SubEntry) {
      FreePool(SubEntry->LoadOptions);
      if (WithSplash) {
        if (Quiet) {
          CHAR16 *TempOptions = RemoveLoadOption(Entry->LoadOptions, L"splash");
          SubEntry->me.Title    = PoolPrint(L"%s verbose without splash", Entry->me.Title);
          SubEntry->LoadOptions = RemoveLoadOption(TempOptions, L"quiet");
          FreePool(TempOptions);
        } else {
          CHAR16 *TempOptions = RemoveLoadOption(Entry->LoadOptions, L"splash");
          SubEntry->me.Title    = PoolPrint(L"%s quiet without splash", Entry->me.Title);
          SubEntry->LoadOptions = AddLoadOption(TempOptions, L"quiet");
          FreePool(TempOptions);
        }
      } else if (Quiet) {
        CHAR16 *TempOptions = RemoveLoadOption(Entry->LoadOptions, L"quiet");
        SubEntry->me.Title    = PoolPrint(L"%s verbose with splash", Entry->me.Title);
        SubEntry->LoadOptions = AddLoadOption(Entry->LoadOptions, L"splash");
        FreePool(TempOptions);
      } else {
        SubEntry->me.Title    = PoolPrint(L"%s quiet with splash", Entry->me.Title);
        SubEntry->LoadOptions = AddLoadOption(Entry->LoadOptions, L"quiet splash");
      }
    }
    AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
  } else if ((Entry->LoaderType == OSTYPE_WIN) || (Entry->LoaderType == OSTYPE_WINEFI)) {
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
  // DBG("    Added '%s': OSType='%d', OSVersion='%a'\n", Entry->me.Title, Entry->LoaderType, Entry->OSVersion);
}

STATIC BOOLEAN AddLoaderEntry(IN CHAR16 *LoaderPath, IN CHAR16 *LoaderOptions, IN CHAR16 *LoaderTitle,
                           IN REFIT_VOLUME *Volume, IN EG_IMAGE *Image, IN UINT8 OSType, IN UINT8 Flags)
{
  LOADER_ENTRY *Entry;
  if ((LoaderPath == NULL) || (Volume == NULL) || (Volume->RootDir == NULL) || !FileExists(Volume->RootDir, LoaderPath)) {
    return FALSE;
  }
  Entry = CreateLoaderEntry(LoaderPath, LoaderOptions, NULL, LoaderTitle, Volume, Image, NULL, OSType, Flags, 0, NULL, FALSE);
  if (Entry != NULL) {
    AddDefaultMenu(Entry);
    AddMenuEntry(&MainMenu, (REFIT_MENU_ENTRY *)Entry);
    return TRUE;
  }
  return FALSE;
}

VOID ScanLoader(VOID)
{
  UINTN         VolumeIndex, Index;
  REFIT_VOLUME *Volume;
  EFI_GUID     *PartGUID;
  
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
    if ((Volume->DiskKind == DISK_KIND_OPTICAL && (GlobalConfig.DisableFlags & VOLTYPE_OPTICAL)) ||
        (Volume->DiskKind == DISK_KIND_EXTERNAL && (GlobalConfig.DisableFlags & VOLTYPE_EXTERNAL)) ||
        (Volume->DiskKind == DISK_KIND_INTERNAL && (GlobalConfig.DisableFlags & VOLTYPE_INTERNAL)) ||
        (Volume->DiskKind == DISK_KIND_FIREWIRE && (GlobalConfig.DisableFlags & VOLTYPE_FIREWIRE)))
    {
      DBG(" hidden\n");
      continue;
    }
    
    if (Volume->Hidden) {
      DBG(" hidden\n");
      continue;
    }
    DBG("\n");
    
    // Use standard location for boot.efi, unless the file /.IAPhysicalMedia is present
    // That file indentifies a 2nd-stage Install Media, so when present, skip standard path to avoid entry duplication
    if (!FileExists(Volume->RootDir, L"\\.IAPhysicalMedia")) {
      if(EFI_ERROR(GetRootUUID(Volume)) || isFirstRootUUID(Volume)) {
        AddLoaderEntry(MACOSX_LOADER_PATH, NULL, L"Mac OS X", Volume, NULL, OSTYPE_OSX, 0);
      }
    }
    // check for Mac OS X Install Data
    AddLoaderEntry(L"\\OS X Install Data\\boot.efi", NULL, L"OS X Install", Volume, NULL, OSTYPE_OSX_INSTALLER, 0);
    AddLoaderEntry(L"\\Mac OS X Install Data\\boot.efi", NULL, L"Mac OS X Install", Volume, NULL, OSTYPE_OSX_INSTALLER, 0);
    AddLoaderEntry(L"\\.IABootFiles\\boot.efi", NULL, L"OS X Install", Volume, NULL, OSTYPE_OSX_INSTALLER, 0);
    // check for Mac OS X Recovery Boot
    AddLoaderEntry(L"\\com.apple.recovery.boot\\boot.efi", NULL, L"Recovery", Volume, NULL, OSTYPE_RECOVERY, 0);

    // Sometimes, on some systems (HP UEFI, if Win is installed first)
    // it is needed to get rid of bootmgfw.efi to allow starting of
    // Clover as /efi/boot/bootx64.efi from HD. We can do that by renaming
    // bootmgfw.efi to bootmgfw-orig.efi
    AddLoaderEntry(L"\\EFI\\Microsoft\\Boot\\bootmgfw-orig.efi", L"", L"Microsoft EFI boot menu", Volume, NULL, OSTYPE_WINEFI, 0);
    // check for Microsoft boot loader/menu
    // If there is bootmgfw-orig.efi, then do not check for bootmgfw.efi
    // since on some systems this will actually be CloverX64.efi
    // renamed to bootmgfw.efi
    AddLoaderEntry(L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi", L"", L"Microsoft EFI boot menu", Volume, NULL, OSTYPE_WINEFI, 0);
    // check for Microsoft boot loader/menu
    AddLoaderEntry(L"\\bootmgr.efi", L"", L"Microsoft EFI boot menu", Volume, NULL, OSTYPE_WINEFI, 0);
    // check for Microsoft boot loader/menu on CDROM
    AddLoaderEntry(L"\\EFI\\MICROSOFT\\BOOT\\cdboot.efi", L"", L"Microsoft EFI boot menu", Volume, NULL, OSTYPE_WINEFI, 0);
    
    // check for linux loaders
    for (Index = 0; Index < LinuxEntryDataCount; ++Index) {
      AddLoaderEntry(LinuxEntryData[Index].Path, L"", LinuxEntryData[Index].Title, Volume,
                     LoadOSIcon(LinuxEntryData[Index].Icon, L"unknown", 128, FALSE, TRUE), OSTYPE_LIN, OSFLAG_NODEFAULTARGS);
    }
    // check for linux kernels
    PartGUID = FindGPTPartitionGuidInDevicePath(Volume->DevicePath);
    if ((PartGUID != NULL) && (Volume->RootDir != NULL)) {
      REFIT_DIR_ITER  Iter;
      EFI_FILE_INFO  *FileInfo = NULL;
      EFI_TIME        PreviousTime;
      CHAR16         *Path = NULL;
      CHAR16         *Options;
      // Get the partition UUID and make sure it's lower case
      CHAR16          PartUUID[40];
      ZeroMem(&PreviousTime, sizeof(EFI_TIME));
      UnicodeSPrint(PartUUID, sizeof(PartUUID), L"%g", PartGUID);
      StrToLower(PartUUID);
      // open the /boot directory (or whatever directory path)
      DirIterOpen(Volume->RootDir, LINUX_BOOT_PATH, &Iter);
      // Check which kernel scan to use
      switch (gSettings.KernelScan) {
      case KERNEL_SCAN_FIRST:
        // First kernel found only
        while (DirIterNext(&Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo)) {
          if (FileInfo != NULL) {
            if (FileInfo->FileSize == 0) {
              FreePool(FileInfo);
              FileInfo = NULL;
              continue;
            }
            // get the kernel file path
            Path = PoolPrint(L"%s\\%s", LINUX_BOOT_PATH, FileInfo->FileName);
            if (Path != NULL) {
              Options = LinuxKernelOptions(Iter.DirHandle, Basename(Path) + StrLen(LINUX_LOADER_PATH), PartUUID, NULL);
              // Add the entry
              AddLoaderEntry(Path, (Options == NULL) ? LINUX_DEFAULT_OPTIONS : Options, NULL, Volume, NULL, OSTYPE_LINEFI, OSFLAG_NODEFAULTARGS);
              if (Options != NULL) {
                FreePool(Options);
              }
              FreePool(Path);
            }
            // free the file info
            FreePool(FileInfo);
            FileInfo = NULL;
            break;
          }
        }
        break;

      case KERNEL_SCAN_LAST:
        // Last kernel found only
        while (DirIterNext(&Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo)) {
          if (FileInfo != NULL) {
            if (FileInfo->FileSize > 0) {
              // get the kernel file path
              if (Path != NULL) {
                FreePool(Path);
              }
              Path = PoolPrint(L"%s\\%s", LINUX_BOOT_PATH, FileInfo->FileName);
            }
            // free the file info
            FreePool(FileInfo);
            FileInfo = NULL;
          }
        }
        if (Path != NULL) {
          Options = LinuxKernelOptions(Iter.DirHandle, Basename(Path) + StrLen(LINUX_LOADER_PATH), PartUUID, NULL);
          // Add the entry
          AddLoaderEntry(Path, (Options == NULL) ? LINUX_DEFAULT_OPTIONS : Options, NULL, Volume, NULL, OSTYPE_LINEFI, OSFLAG_NODEFAULTARGS);
          if (Options != NULL) {
            FreePool(Options);
          }
          FreePool(Path);
        }
        break;

      case KERNEL_SCAN_NEWEST:
        // Newest dated kernel only
        while (DirIterNext(&Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo)) {
          if (FileInfo != NULL) {
            if (FileInfo->FileSize > 0) {
              // get the kernel file path
              if ((PreviousTime.Year == 0) || (TimeCmp(&PreviousTime, &(FileInfo->ModificationTime)) < 0)) {
                if (Path != NULL) {
                  FreePool(Path);
                }
                Path = PoolPrint(L"%s\\%s", LINUX_BOOT_PATH, FileInfo->FileName);
                PreviousTime = FileInfo->ModificationTime;
              }
            }
            // free the file info
            FreePool(FileInfo);
            FileInfo = NULL;
          }
        }
        if (Path != NULL) {
          Options = LinuxKernelOptions(Iter.DirHandle, Basename(Path) + StrLen(LINUX_LOADER_PATH), PartUUID, NULL);
          // Add the entry
          AddLoaderEntry(Path, (Options == NULL) ? LINUX_DEFAULT_OPTIONS : Options, NULL, Volume, NULL, OSTYPE_LINEFI, OSFLAG_NODEFAULTARGS);
          if (Options != NULL) {
            FreePool(Options);
          }
          FreePool(Path);
        }
        break;

      case KERNEL_SCAN_OLDEST:
        // Oldest dated kernel only
        while (DirIterNext(&Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo)) {
          if (FileInfo != NULL) {
            if (FileInfo->FileSize > 0) {
              // get the kernel file path
              if ((PreviousTime.Year == 0) || (TimeCmp(&PreviousTime, &(FileInfo->ModificationTime)) > 0)) {
                if (Path != NULL) {
                  FreePool(Path);
                }
                Path = PoolPrint(L"%s\\%s", LINUX_BOOT_PATH, FileInfo->FileName);
                PreviousTime = FileInfo->ModificationTime;
              }
            }
            // free the file info
            FreePool(FileInfo);
            FileInfo = NULL;
          }
        }
        if (Path != NULL) {
          Options = LinuxKernelOptions(Iter.DirHandle, Basename(Path) + StrLen(LINUX_LOADER_PATH), PartUUID, NULL);
          // Add the entry
          AddLoaderEntry(Path, (Options == NULL) ? LINUX_DEFAULT_OPTIONS : Options, NULL, Volume, NULL, OSTYPE_LINEFI, OSFLAG_NODEFAULTARGS);
          if (Options != NULL) {
            FreePool(Options);
          }
          FreePool(Path);
        }
        break;

      case KERNEL_SCAN_MOSTRECENT:
        // most recent kernel version only
        while (DirIterNext(&Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo)) {
          if (FileInfo != NULL) {
            if (FileInfo->FileSize > 0) {
              // get the kernel file path
              CHAR16 *NewPath = PoolPrint(L"%s\\%s", LINUX_BOOT_PATH, FileInfo->FileName);
              if (NewPath != NULL) {
                if ((Path == NULL) || (StrCmp(Path, NewPath) < 0)) {
                  if (Path != NULL) {
                    FreePool(Path);
                  }
                  Path = NewPath;
                } else {
                  FreePool(NewPath);
                }
              }
            }
            // free the file info
            FreePool(FileInfo);
            FileInfo = NULL;
          }
        }
        if (Path != NULL) {
          Options = LinuxKernelOptions(Iter.DirHandle, Basename(Path) + StrLen(LINUX_LOADER_PATH), PartUUID, NULL);
          // Add the entry
          AddLoaderEntry(Path, (Options == NULL) ? LINUX_DEFAULT_OPTIONS : Options, NULL, Volume, NULL, OSTYPE_LINEFI, OSFLAG_NODEFAULTARGS);
          if (Options != NULL) {
            FreePool(Options);
          }
          FreePool(Path);
        }
        break;

      case KERNEL_SCAN_EARLIEST:
        // earliest kernel version only
        while (DirIterNext(&Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo)) {
          if (FileInfo != NULL) {
            if (FileInfo->FileSize > 0) {
              // get the kernel file path
              CHAR16 *NewPath = PoolPrint(L"%s\\%s", LINUX_BOOT_PATH, FileInfo->FileName);
              if (NewPath != NULL) {
                if ((Path == NULL) || (StrCmp(Path, NewPath) > 0)) {
                  if (Path != NULL) {
                    FreePool(Path);
                  }
                  Path = NewPath;
                } else {
                  FreePool(NewPath);
                }
              }
            }
            // free the file info
            FreePool(FileInfo);
            FileInfo = NULL;
          }
        }
        if (Path != NULL) {
          Options = LinuxKernelOptions(Iter.DirHandle, Basename(Path) + StrLen(LINUX_LOADER_PATH), PartUUID, NULL);
          // Add the entry
          AddLoaderEntry(Path, (Options == NULL) ? LINUX_DEFAULT_OPTIONS : Options, NULL, Volume, NULL, OSTYPE_LINEFI, OSFLAG_NODEFAULTARGS);
          if (Options != NULL) {
            FreePool(Options);
          }
          FreePool(Path);
        }
        break;

      case KERNEL_SCAN_ALL:
        // get all the filename matches
        while (DirIterNext(&Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo)) {
          if (FileInfo != NULL) {
            if (FileInfo->FileSize > 0) {
              // get the kernel file path
              Path = PoolPrint(L"%s\\%s", LINUX_BOOT_PATH, FileInfo->FileName);
              if (Path != NULL) {
                Options = LinuxKernelOptions(Iter.DirHandle, Basename(Path) + StrLen(LINUX_LOADER_PATH), PartUUID, NULL);
                // Add the entry
                AddLoaderEntry(Path, (Options == NULL) ? LINUX_DEFAULT_OPTIONS : Options, NULL, Volume, NULL, OSTYPE_LINEFI, OSFLAG_NODEFAULTARGS);
                if (Options != NULL) {
                  FreePool(Options);
                }
                FreePool(Path);
              }
            }
            // free the file info
            FreePool(FileInfo);
            FileInfo = NULL;
          }
        }
        break;

      default:
      case KERNEL_SCAN_NONE:
        // No kernel scan
        break;
      }
      //close the directory
      DirIterClose(&Iter);
    }

    //     DBG("search for  optical UEFI\n");
    if (Volume->DiskKind == DISK_KIND_OPTICAL) {
      AddLoaderEntry(BOOT_LOADER_PATH, L"", L"UEFI optical", Volume, NULL, OSTYPE_OTHER, 0);
    }
    //     DBG("search for internal UEFI\n");
    if (Volume->DiskKind == DISK_KIND_INTERNAL) {
      AddLoaderEntry(BOOT_LOADER_PATH, L"", L"UEFI internal", Volume, NULL, OSTYPE_OTHER, 0);
    }
    //    DBG("search for external UEFI\n");
    if (Volume->DiskKind == DISK_KIND_EXTERNAL) {
      AddLoaderEntry(BOOT_LOADER_PATH, L"", L"UEFI external", Volume, NULL, OSTYPE_OTHER, 0);
    }
  }
}

STATIC VOID AddCustomEntry(IN UINTN                CustomIndex,
                           IN CHAR16              *CustomPath,
                           IN CUSTOM_LOADER_ENTRY *Custom,
                           IN REFIT_MENU_SCREEN   *SubMenu)
{
  UINTN           VolumeIndex;
  REFIT_VOLUME   *Volume;
  REFIT_DIR_ITER  SIter;
  REFIT_DIR_ITER *Iter = &SIter;
  CHAR16          PartUUID[40];
  BOOLEAN         IsSubEntry = (SubMenu != NULL);
  BOOLEAN         FindCustomPath = (CustomPath == NULL);

  if (Custom == NULL) {
    return;
  }
  if (FindCustomPath && (Custom->Type != OSTYPE_LINEFI)) {
    DBG("Custom %sentry %d skipped because it didn't have a path.\n", IsSubEntry ? L"sub " : L"", CustomIndex);
    return;
  }
  if (OSFLAG_ISSET(Custom->Flags, OSFLAG_DISABLED)) {
    DBG("Custom %sentry %d skipped because it is disabled.\n", IsSubEntry ? L"sub " : L"", CustomIndex);
    return;
  }
  if (!gSettings.ShowHiddenEntries && OSFLAG_ISSET(Custom->Flags, OSFLAG_HIDDEN)) {
    DBG("Custom %sentry %d skipped because it is hidden.\n", IsSubEntry ? L"sub " : L"", CustomIndex);
    return;
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
    CUSTOM_LOADER_ENTRY *CustomSubEntry;
    LOADER_ENTRY        *Entry = NULL;
    EG_IMAGE            *Image, *DriveImage;
    EFI_GUID            *Guid = NULL;
    UINT64               VolumeSize;

    Volume = Volumes[VolumeIndex];
    if ((Volume == NULL) || (Volume->RootDir == NULL)) {
      continue;
    }
    if (Volume->VolName == NULL) {
      Volume->VolName = L"Unknown";
    }
    
    DBG("   Checking volume \"%s\" (%s) ... ", Volume->VolName, Volume->DevicePathString);
    
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
    if (StriCmp(CustomPath, MACOSX_LOADER_PATH) == 0 && FileExists(Volume->RootDir, L"\\.IAPhysicalMedia")) {
      DBG("skipped standard OSX path because volume is 2nd stage Install Media\n");
      continue;
    }
    Guid = FindGPTPartitionGuidInDevicePath(Volume->DevicePath);
    // Open the boot directory to search for kernels
    if (FindCustomPath) {
      EFI_FILE_INFO *FileInfo = NULL;
      EFI_TIME       PreviousTime;
      ZeroMem(&PreviousTime, sizeof(EFI_TIME));
      // Get the partition UUID and make sure it's lower case
      if (Guid == NULL) {
        DBG("skipped because volume does not have partition uuid\n");
        continue;
      }
      UnicodeSPrint(PartUUID, sizeof(PartUUID), L"%g", Guid);
      StrToLower(PartUUID);
      // open the /boot directory (or whatever directory path)
      DirIterOpen(Volume->RootDir, LINUX_BOOT_PATH, Iter);
      // Check if user wants to find newest kernel only
      switch (Custom->KernelScan) {
      case KERNEL_SCAN_FIRST:
        // First kernel found only
        while (DirIterNext(Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo)) {
          if (FileInfo != NULL) {
            if (FileInfo->FileSize == 0) {
              FreePool(FileInfo);
              FileInfo = NULL;
              continue;
            }
            // get the kernel file path
            CustomPath = PoolPrint(L"%s\\%s", LINUX_BOOT_PATH, FileInfo->FileName);
            // free the file info
            FreePool(FileInfo);
            FileInfo = NULL;
            break;
          }
        }
        break;

      case KERNEL_SCAN_LAST:
        // Last kernel found only
        while (DirIterNext(Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo)) {
          if (FileInfo != NULL) {
            if (FileInfo->FileSize > 0) {
              // get the kernel file path
              if (CustomPath != NULL) {
                FreePool(CustomPath);
              }
              CustomPath = PoolPrint(L"%s\\%s", LINUX_BOOT_PATH, FileInfo->FileName);
            }
            // free the file info
            FreePool(FileInfo);
            FileInfo = NULL;
          }
        }
        break;

      case KERNEL_SCAN_NEWEST:
        // Newest dated kernel only
        while (DirIterNext(Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo)) {
          if (FileInfo != NULL) {
            if (FileInfo->FileSize > 0) {
              // get the kernel file path
              if ((PreviousTime.Year == 0) || (TimeCmp(&PreviousTime, &(FileInfo->ModificationTime)) < 0)) {
                if (CustomPath != NULL) {
                  FreePool(CustomPath);
                }
                CustomPath = PoolPrint(L"%s\\%s", LINUX_BOOT_PATH, FileInfo->FileName);
                PreviousTime = FileInfo->ModificationTime;
              }
            }
            // free the file info
            FreePool(FileInfo);
            FileInfo = NULL;
          }
        }
        break;

      case KERNEL_SCAN_OLDEST:
        // Oldest dated kernel only
        while (DirIterNext(Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo)) {
          if (FileInfo != NULL) {
            if (FileInfo->FileSize > 0) {
              // get the kernel file path
              if ((PreviousTime.Year == 0) || (TimeCmp(&PreviousTime, &(FileInfo->ModificationTime)) > 0)) {
                if (CustomPath != NULL) {
                  FreePool(CustomPath);
                }
                CustomPath = PoolPrint(L"%s\\%s", LINUX_BOOT_PATH, FileInfo->FileName);
                PreviousTime = FileInfo->ModificationTime;
              }
            }
            // free the file info
            FreePool(FileInfo);
            FileInfo = NULL;
          }
        }
        break;

      case KERNEL_SCAN_MOSTRECENT:
        // most recent kernel version only
        while (DirIterNext(Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo)) {
          if (FileInfo != NULL) {
            if (FileInfo->FileSize > 0) {
              // get the kernel file path
              CHAR16 *NewPath = PoolPrint(L"%s\\%s", LINUX_BOOT_PATH, FileInfo->FileName);
              if ((CustomPath == NULL) || (StrCmp(CustomPath, NewPath) < 0)) {
                if (CustomPath != NULL) {
                  FreePool(CustomPath);
                }
                CustomPath = NewPath;
              } else {
                FreePool(NewPath);
              }
            }
            // free the file info
            FreePool(FileInfo);
            FileInfo = NULL;
          }
        }
        break;

      case KERNEL_SCAN_EARLIEST:
        // earliest kernel version only
        while (DirIterNext(Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo)) {
          if (FileInfo != NULL) {
            if (FileInfo->FileSize > 0) {
              // get the kernel file path
              CHAR16 *NewPath = PoolPrint(L"%s\\%s", LINUX_BOOT_PATH, FileInfo->FileName);
              if ((CustomPath == NULL) || (StrCmp(CustomPath, NewPath) > 0)) {
                if (CustomPath != NULL) {
                  FreePool(CustomPath);
                }
                CustomPath = NewPath;
              } else {
                FreePool(NewPath);
              }
            }
            // free the file info
            FreePool(FileInfo);
            FileInfo = NULL;
          }
        }
        break;

      default:
        // Set scan to all just in case
        Custom->KernelScan = KERNEL_SCAN_ALL;
        break;
      }
    } else if (!FileExists(Volume->RootDir, CustomPath)) {
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
    do
    {
      // Search for linux kernels
      CHAR16 *CustomOptions = Custom->Options;
      if (FindCustomPath && (Custom->KernelScan == KERNEL_SCAN_ALL)) {
        EFI_FILE_INFO *FileInfo = NULL;
        // Get the next kernel path or stop looking
        if (!DirIterNext(Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo) || (FileInfo == NULL)) {
          DBG("\n");
          break;
        }
        // who knows....
        if (FileInfo->FileSize == 0) {
          FreePool(FileInfo);
          continue;
        }
        // get the kernel file path
        CustomPath = PoolPrint(L"%s\\%s", LINUX_BOOT_PATH, FileInfo->FileName);
        // free the file info
        FreePool(FileInfo);
      }
      if (CustomPath == NULL) {
        DBG("skipped\n");
        break;
      }
      // Check to make sure we should update custom options or not
      if (FindCustomPath && OSFLAG_ISUNSET(Custom->Flags, OSFLAG_NODEFAULTARGS)) {
        // Find the init ram image and select root
        CustomOptions = LinuxKernelOptions(Iter->DirHandle, Basename(CustomPath) + StrLen(LINUX_LOADER_PATH), PartUUID, Custom->Options);
      }
      // Check to make sure that this entry is not hidden or disabled by another custom entry
      if (!IsSubEntry) {
        CUSTOM_LOADER_ENTRY *Ptr;
        UINTN                i = 0;
        BOOLEAN              BetterMatch = FALSE;
        for (Ptr = gSettings.CustomEntries; Ptr != NULL; ++i, Ptr = Ptr->Next) {
          // Don't match against this custom
          if (Ptr == Custom) {
            continue;
          }
          // Can only match the same types
          if (Custom->Type != Ptr->Type) {
            continue;
          }
          // Check if the volume string matches
          if (Custom->Volume != Ptr->Volume) {
            if (Ptr->Volume == NULL) {
              // Less precise volume match
              if (Custom->Path != Ptr->Path) {
                // Better path match
                BetterMatch = ((Ptr->Path != NULL) && (StrCmp(CustomPath, Ptr->Path) == 0) &&
                               ((Custom->VolumeType == Ptr->VolumeType) ||
                                (Volume->DiskKind == DISK_KIND_OPTICAL && (Custom->VolumeType & VOLTYPE_OPTICAL)) ||
                                (Volume->DiskKind == DISK_KIND_EXTERNAL && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
                                (Volume->DiskKind == DISK_KIND_INTERNAL && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
                                (Volume->DiskKind == DISK_KIND_FIREWIRE && (Custom->VolumeType & VOLTYPE_FIREWIRE))));
              }
            } else if ((StrStr(Volume->DevicePathString, Custom->Volume) == NULL) &&
                       ((Volume->VolName == NULL) || (StrStr(Volume->VolName, Custom->Volume) == NULL))) {
              if (Custom->Volume == NULL) {
                // More precise volume match
                if (Custom->Path != Ptr->Path) {
                  // Better path match
                  BetterMatch = ((Ptr->Path != NULL) && (StrCmp(CustomPath, Ptr->Path) == 0) &&
                                 ((Custom->VolumeType == Ptr->VolumeType) ||
                                  (Volume->DiskKind == DISK_KIND_OPTICAL && (Custom->VolumeType & VOLTYPE_OPTICAL)) ||
                                  (Volume->DiskKind == DISK_KIND_EXTERNAL && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
                                  (Volume->DiskKind == DISK_KIND_INTERNAL && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
                                  (Volume->DiskKind == DISK_KIND_FIREWIRE && (Custom->VolumeType & VOLTYPE_FIREWIRE))));
                } else if (Custom->VolumeType != Ptr->VolumeType) {
                  // More precise volume type match
                  BetterMatch = ((Custom->VolumeType == 0) &&
                                 ((Volume->DiskKind == DISK_KIND_OPTICAL && (Custom->VolumeType & VOLTYPE_OPTICAL)) ||
                                  (Volume->DiskKind == DISK_KIND_EXTERNAL && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
                                  (Volume->DiskKind == DISK_KIND_INTERNAL && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
                                  (Volume->DiskKind == DISK_KIND_FIREWIRE && (Custom->VolumeType & VOLTYPE_FIREWIRE))));
                } else {
                  // Better match
                  BetterMatch = TRUE;
                }
              // Duplicate volume match
              } else if (Custom->Path != Ptr->Path) {
                // Better path match
                BetterMatch = ((Ptr->Path != NULL) && (StrCmp(CustomPath, Ptr->Path) == 0) &&
                               ((Custom->VolumeType == Ptr->VolumeType) ||
                                (Volume->DiskKind == DISK_KIND_OPTICAL && (Custom->VolumeType & VOLTYPE_OPTICAL)) ||
                                (Volume->DiskKind == DISK_KIND_EXTERNAL && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
                                (Volume->DiskKind == DISK_KIND_INTERNAL && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
                                (Volume->DiskKind == DISK_KIND_FIREWIRE && (Custom->VolumeType & VOLTYPE_FIREWIRE))));
              // Duplicate path match
              } else if (Custom->VolumeType != Ptr->VolumeType) {
                // More precise volume type match
                BetterMatch = ((Custom->VolumeType == 0) &&
                               ((Volume->DiskKind == DISK_KIND_OPTICAL && (Custom->VolumeType & VOLTYPE_OPTICAL)) ||
                                (Volume->DiskKind == DISK_KIND_EXTERNAL && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
                                (Volume->DiskKind == DISK_KIND_INTERNAL && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
                                (Volume->DiskKind == DISK_KIND_FIREWIRE && (Custom->VolumeType & VOLTYPE_FIREWIRE))));
              } else {
                // Duplicate entry
                BetterMatch = (i <= CustomIndex);
              }
            }
          // Duplicate volume match
          } else if (Custom->Path != Ptr->Path) {
            if (Ptr->Path == NULL) {
              // Less precise path match
              BetterMatch = ((Custom->VolumeType != Ptr->VolumeType) &&
                              ((Volume->DiskKind == DISK_KIND_OPTICAL && (Custom->VolumeType & VOLTYPE_OPTICAL)) ||
                               (Volume->DiskKind == DISK_KIND_EXTERNAL && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
                               (Volume->DiskKind == DISK_KIND_INTERNAL && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
                               (Volume->DiskKind == DISK_KIND_FIREWIRE && (Custom->VolumeType & VOLTYPE_FIREWIRE))));
            } else if (StrCmp(CustomPath, Ptr->Path) == 0) {
              if (Custom->Path == NULL) {
                // More precise path and volume type match
                BetterMatch = ((Custom->VolumeType == Ptr->VolumeType) ||
                               (Volume->DiskKind == DISK_KIND_OPTICAL && (Custom->VolumeType & VOLTYPE_OPTICAL)) ||
                               (Volume->DiskKind == DISK_KIND_EXTERNAL && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
                               (Volume->DiskKind == DISK_KIND_INTERNAL && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
                               (Volume->DiskKind == DISK_KIND_FIREWIRE && (Custom->VolumeType & VOLTYPE_FIREWIRE)));
              } else if (Custom->VolumeType != Ptr->VolumeType) {
                // More precise volume type match
                BetterMatch = ((Custom->VolumeType == 0) &&
                               ((Volume->DiskKind == DISK_KIND_OPTICAL && (Custom->VolumeType & VOLTYPE_OPTICAL)) ||
                                (Volume->DiskKind == DISK_KIND_EXTERNAL && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
                                (Volume->DiskKind == DISK_KIND_INTERNAL && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
                                (Volume->DiskKind == DISK_KIND_FIREWIRE && (Custom->VolumeType & VOLTYPE_FIREWIRE))));
              } else {
                // Duplicate entry
                BetterMatch = (i <= CustomIndex);
              }
            }
          // Duplicate path match
          } else if (Custom->VolumeType != Ptr->VolumeType) {
            // More precise volume type match
            BetterMatch = ((Custom->VolumeType == 0) &&
                           ((Volume->DiskKind == DISK_KIND_OPTICAL && (Custom->VolumeType & VOLTYPE_OPTICAL)) ||
                            (Volume->DiskKind == DISK_KIND_EXTERNAL && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
                            (Volume->DiskKind == DISK_KIND_INTERNAL && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
                            (Volume->DiskKind == DISK_KIND_FIREWIRE && (Custom->VolumeType & VOLTYPE_FIREWIRE))));
          } else {
            // Duplicate entry
            BetterMatch = (i <= CustomIndex);
          }
          if (BetterMatch) {
            break;
          }
        }
        if (BetterMatch) {
          DBG("skipped because custom entry %d is a better match and will produce a duplicate entry\n", i);
          continue;
        }
      }
      DBG("match!\n");
      // Create a entry for this volume
      Entry = CreateLoaderEntry(CustomPath, CustomOptions, Custom->FullTitle, Custom->Title, Volume, Image, DriveImage, Custom->Type, Custom->Flags, Custom->Hotkey, Custom->BootBgColor, TRUE);
      if (Entry != NULL) {
        if (OSFLAG_ISUNSET(Custom->Flags, OSFLAG_NODEFAULTMENU)) {
          AddDefaultMenu(Entry);
        } else if (Custom->SubEntries != NULL) {
          UINTN CustomSubIndex = 0;
          // Add subscreen
          REFIT_MENU_SCREEN *SubScreen = AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
          if (SubScreen) {
            SubScreen->Title = PoolPrint(L"Boot Options for %s on %s", (Custom->Title != NULL) ? Custom->Title : CustomPath, Entry->VolName);
            SubScreen->TitleImage = Entry->me.Image;
            SubScreen->ID = Custom->Type + 20;
            SubScreen->AnimeRun = GetAnime(SubScreen);
            VolumeSize = RShiftU64(MultU64x32(Volume->BlockIO->Media->LastBlock, Volume->BlockIO->Media->BlockSize), 20);
            AddMenuInfoLine(SubScreen, PoolPrint(L"Volume size: %dMb", VolumeSize));
            AddMenuInfoLine(SubScreen, FileDevicePathToStr(Entry->DevicePath));
            if (Guid) {
              CHAR8 *GuidStr = AllocateZeroPool(50);
              AsciiSPrint(GuidStr, 50, "%g", Guid);
              AddMenuInfoLine(SubScreen, PoolPrint(L"UUID: %a", GuidStr));
              FreePool(GuidStr);
            }
            AddMenuInfoLine(SubScreen, PoolPrint(L"Options: %s", Entry->LoadOptions));
            // Create sub entries
            for (CustomSubEntry = Custom->SubEntries; CustomSubEntry; CustomSubEntry = CustomSubEntry->Next) {
              AddCustomEntry(CustomSubIndex++, (CustomSubEntry->Path != NULL) ? CustomSubEntry->Path : CustomPath, CustomSubEntry, SubScreen);
            }
            AddMenuEntry(SubScreen, &MenuEntryReturn);
            Entry->me.SubScreen = SubScreen;
          }
        }
        AddMenuEntry(IsSubEntry ? SubMenu : &MainMenu, (REFIT_MENU_ENTRY *)Entry);
      }
      // cleanup custom
      if (FindCustomPath) {
        FreePool(CustomPath);
        FreePool(CustomOptions);
      }
    } while (FindCustomPath && (Custom->KernelScan == KERNEL_SCAN_ALL));
    // Close the kernel boot directory
    if (FindCustomPath) {
      DirIterClose(Iter);
    }
  }
}

// Add custom entries
VOID AddCustomEntries(VOID)
{
  CUSTOM_LOADER_ENTRY *Custom;
  UINTN                i = 0;
  
  DBG("Custom entries start\n");
  // Traverse the custom entries
  for (Custom = gSettings.CustomEntries; Custom; ++i, Custom = Custom->Next) {
    if ((Custom->Path == NULL) && (Custom->Type != 0)) {
      if (OSTYPE_IS_OSX(Custom->Type)) {
        AddCustomEntry(i, MACOSX_LOADER_PATH, Custom, NULL);
      } else if (OSTYPE_IS_OSX_RECOVERY(Custom->Type)) {
        AddCustomEntry(i, L"\\com.apple.recovery.boot\\boot.efi", Custom, NULL);
      } else if (OSTYPE_IS_OSX_INSTALLER(Custom->Type)) {
        UINTN Index = 0;
        while (Index < OSXInstallerPathsCount) {
          AddCustomEntry(i, OSXInstallerPaths[Index++], Custom, NULL);
        }
      } else if (OSTYPE_IS_WINDOWS(Custom->Type)) {
        AddCustomEntry(i, L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi", Custom, NULL);
      } else if (OSTYPE_IS_LINUX(Custom->Type)) {
        UINTN Index = 0;
        while (Index < LinuxEntryDataCount) {
          AddCustomEntry(i, LinuxEntryData[Index++].Path, Custom, NULL);
        }
      } else if (Custom->Type == OSTYPE_LINEFI) {
        AddCustomEntry(i, NULL, Custom, NULL);
      } else {
        AddCustomEntry(i, BOOT_LOADER_PATH, Custom, NULL);
      }
    } else {
      AddCustomEntry(i, Custom->Path, Custom, NULL);
    }
  }
  DBG("Custom entries finish\n");
}
