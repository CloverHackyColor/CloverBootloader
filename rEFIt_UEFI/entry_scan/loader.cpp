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

#include "loader.h"
#include "../cpp_foundation/XString.h"
#include "entry_scan.h"
#include "../Platform/Settings.h"
#include "../Platform/Hibernate.h"
#include "../refit/screen.h"
#include "../refit/menu.h"
#include "common.h"
#include "../Platform/Nvram.h"
#include "../Platform/APFS.h"
#include "../Platform/guid.h"
#include "../refit/lib.h"
#include "../gui/REFIT_MENU_SCREEN.h"

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

const XStringW LINUX_ISSUE_PATH = L"\\etc\\issue"_XSW;
#define LINUX_BOOT_PATH L"\\boot"
#define LINUX_BOOT_ALT_PATH L"\\boot"
const XString LINUX_LOADER_PATH = "vmlinuz"_XS;
const XStringW LINUX_FULL_LOADER_PATH = SWPrintf("%ls\\%s", LINUX_BOOT_PATH, LINUX_LOADER_PATH.c_str());
#define LINUX_LOADER_SEARCH_PATH L"vmlinuz*"
const XStringArray LINUX_DEFAULT_OPTIONS = Split<XStringArray>("ro add_efi_memmap quiet splash vt.handoff=7", " ");

#if defined(MDE_CPU_X64)
#define BOOT_LOADER_PATH L"\\EFI\\BOOT\\BOOTX64.efi"
#else
#define BOOT_LOADER_PATH L"\\EFI\\BOOT\\BOOTIA32.efi"
#endif


extern LOADER_ENTRY *SubMenuKextInjectMgmt(LOADER_ENTRY *Entry);

// Linux loader path data
typedef struct LINUX_PATH_DATA
{
   CONST CHAR16 *Path;
   CONST CHAR16 *Title;
   CONST CHAR16 *Icon;
   CONST CHAR8  *Issue;
} LINUX_PATH_DATA;

typedef struct LINUX_ICON_DATA
{
   CONST CHAR16 *DirectoryName;
   CONST CHAR16 *IconName;
} LINUX_ICON_MAPPING;

STATIC LINUX_ICON_DATA LinuxIconMapping[] = {
    { L"LinuxMint", L"mint" },
    { L"arch_grub", L"arch" },
    { L"opensuse", L"suse" },
    { L"ORACLE", L"solaris" },
    { L"elementary", L"eos" },
    { L"pclinuxos", L"pclinux" },
    { L"mx18", L"mx" },
    { L"mx19", L"mx" }
};
STATIC CONST UINTN LinuxIconMappingCount = (sizeof(LinuxIconMapping) / sizeof(LINUX_ICON_DATA));

STATIC LINUX_PATH_DATA LinuxEntryData[] = {
#if defined(MDE_CPU_X64)
  //comment out all common names
//  { L"\\EFI\\grub\\grubx64.efi", L"Grub EFI boot menu", L"grub,linux" },
//  { L"\\EFI\\Gentoo\\grubx64.efi", L"Gentoo EFI boot menu", L"gentoo,linux", "Gentoo" },
  { L"\\EFI\\Gentoo\\kernelx64.efi", L"Gentoo EFI kernel", L"gentoo,linux" },
//  { L"\\EFI\\RedHat\\grubx64.efi", L"RedHat EFI boot menu", L"redhat,linux", "Redhat" },
//  { L"\\EFI\\debian\\grubx64.efi", L"Debian EFI boot menu", L"debian,linux", "Debian" },
//  { L"\\EFI\\kali\\grubx64.efi", L"Kali EFI boot menu", L"kali,linux", "Kali" },
//  { L"\\EFI\\ubuntu\\grubx64.efi", L"Ubuntu EFI boot menu", L"ubuntu,linux", "Ubuntu" },
//  { L"\\EFI\\kubuntu\\grubx64.efi", L"kubuntu EFI boot menu", L"kubuntu,linux", "kubuntu" },
//  { L"\\EFI\\LinuxMint\\grubx64.efi", L"Linux Mint EFI boot menu", L"mint,linux", "Linux Mint" },
//  { L"\\EFI\\Fedora\\grubx64.efi", L"Fedora EFI boot menu", L"fedora,linux", "Fedora" },
//  { L"\\EFI\\opensuse\\grubx64.efi", L"OpenSuse EFI boot menu", L"suse,linux", "openSUSE" },
//  { L"\\EFI\\arch\\grubx64.efi", L"ArchLinux EFI boot menu", L"arch,linux" },
//  { L"\\EFI\\arch_grub\\grubx64.efi", L"ArchLinux EFI boot menu", L"arch,linux" },
//  { L"\\EFI\\ORACLE\\grubx64.efi", L"Oracle Solaris EFI boot menu", L"solaris,linux", "Solaris" },
//  { L"\\EFI\\Endless\\grubx64.efi", L"EndlessOS EFI boot menu", L"endless,linux", "EndlessOS" },
//  { L"\\EFI\\antergos_grub\\grubx64.efi", L"Antergos EFI boot menu", L"antergos,linux", "Antergos" },
//  { L"\\EFI\\Deepin\\grubx64.efi", L"Deepin EFI boot menu", L"deepin,linux", "Deepin" },
//  { L"\\EFI\\elementary\\grubx64.efi", L"Elementary EFI boot menu", L"eos,linux", "Elementary" },
//  { L"\\EFI\\Manjaro\\grubx64.efi", L"Manjaro EFI boot menu", L"manjaro,linux", "Manjaro" },
//  { L"\\EFI\\xubuntu\\grubx64.efi", L"Xubuntu EFI boot menu", L"xubuntu,linux", "Xubuntu" },
//  { L"\\EFI\\zorin\\grubx64.efi", L"Zorin EFI boot menu", L"zorin,linux", "Zorin" },
  { L"\\EFI\\goofiboot\\goofibootx64.efi", L"Solus EFI boot menu", L"solus,linux", "Solus" },
//  { L"\\EFI\\centos\\grubx64.efi", L"CentOS EFI boot menu", L"centos,linux", "CentOS" },
//  { L"\\EFI\\pclinuxos\\grubx64.efi", L"PCLinuxOS EFI boot menu", L"pclinux,linux", "PCLinux" },
//  { L"\\EFI\\neon\\grubx64.efi", L"KDE Neon EFI boot menu", L"neon,linux", "KDE Neon" },
//  { L"\\EFI\\MX19\\grubx64.efi", L"MX Linux EFI boot menu", L"mx,linux", "MX Linux" },
//  { L"\\EFI\\parrot\\grubx64.efi", L"Parrot OS EFI boot menu", L"parrot,linux", "Parrot OS" },
#else
  //dont bother about 32bit compilation
  { L"\\EFI\\grub\\grub.efi", L"Grub EFI boot menu", L"grub,linux" },
  { L"\\EFI\\Gentoo\\grub.efi", L"Gentoo EFI boot menu", L"gentoo,linux", "Gentoo" },
  { L"\\EFI\\Gentoo\\kernel.efi", L"Gentoo EFI kernel", L"gentoo,linux" },
  { L"\\EFI\\RedHat\\grub.efi", L"RedHat EFI boot menu", L"redhat,linux", "Redhat" },
  { L"\\EFI\\debian\\grub.efi", L"Debian EFI boot menu", L"debian,linux", "Debian" },
  { L"\\EFI\\kali\\grub.efi", L"Kali EFI boot menu", L"kali,linux", "Kali" },
  { L"\\EFI\\ubuntu\\grub.efi", L"Ubuntu EFI boot menu", L"ubuntu,linux", "Ubuntu" },
  { L"\\EFI\\kubuntu\\grub.efi", L"kubuntu EFI boot menu", L"kubuntu,linux", "kubuntu" },
  { L"\\EFI\\LinuxMint\\grub.efi", L"Linux Mint EFI boot menu", L"mint,linux", "Linux Mint" },
  { L"\\EFI\\Fedora\\grub.efi", L"Fedora EFI boot menu", L"fedora,linux", "Fedora" },
  { L"\\EFI\\opensuse\\grub.efi", L"OpenSuse EFI boot menu", L"suse,linux", "openSUSE" },
  { L"\\EFI\\arch\\grub.efi", L"ArchLinux EFI boot menu", L"arch,linux" },
  { L"\\EFI\\arch_grub\\grub.efi", L"ArchLinux EFI boot menu", L"arch,linux" },
  { L"\\EFI\\ORACLE\\grub.efi", L"Oracle Solaris EFI boot menu", L"solaris,linux", "Solaris" },
  { L"\\EFI\\Endless\\grub.efi", L"EndlessOS EFI boot menu", L"endless,linux", "EndlessOS" },
  { L"\\EFI\\antergos_grub\\grub.efi", L"Antergos EFI boot menu", L"antergos,linux", "Antergos" },
  { L"\\EFI\\Deepin\\grub.efi", L"Deepin EFI boot menu", L"deepin,linux", "Deepin" },
  { L"\\EFI\\elementary\\grub.efi", L"Elementary EFI boot menu", L"eos,linux", "Elementary" },
  { L"\\EFI\\Manjaro\\grub.efi", L"Manjaro EFI boot menu", L"manjaro,linux", "Manjaro" },
  { L"\\EFI\\xubuntu\\grub.efi", L"Xubuntu EFI boot menu", L"xubuntu,linux", "Xubuntu" },
  { L"\\EFI\\zorin\\grub.efi", L"Zorin EFI boot menu", L"zorin,linux", "Zorin" },
  { L"\\EFI\\goofiboot\\goofiboot.efi", L"Solus EFI boot menu", L"solus,linux", "Solus" },
  { L"\\EFI\\centos\\grub.efi", L"CentOS EFI boot menu", L"centos,linux", "CentOS" },
  { L"\\EFI\\pclinuxos\\grub.efi", L"PCLinuxOS EFI boot menu", L"pclinux,linux", "PCLinux" },
  { L"\\EFI\\neon\\grub.efi", L"KDE Neon EFI boot menu", L"neon,linux", "KDE Neon" },
  { L"\\EFI\\MX19\\grub.efi", L"MX Linux EFI boot menu", L"mx,linux", "MX Linux" },
  { L"\\EFI\\parrot\\grub.efi", L"Parrot OS EFI boot menu", L"parrot,linux", "Parrot OS" },
#endif
  { L"\\EFI\\SuSe\\elilo.efi", L"OpenSuse EFI boot menu", L"suse,linux" },
};
STATIC CONST UINTN LinuxEntryDataCount = (sizeof(LinuxEntryData) / sizeof(LINUX_PATH_DATA));

#if defined(ANDX86)
#if !defined(MDE_CPU_X64)
#undef ANDX86
#else
// ANDX86 loader path data
#define ANDX86_FINDLEN 3
typedef struct ANDX86_PATH_DATA
{
   CONST CHAR16   *Path;
   CONST CHAR16   *Title;
   CONST CHAR16   *Icon;
   CONST CHAR16   *Find[ANDX86_FINDLEN];
} ANDX86_PATH_DATA;

STATIC ANDX86_PATH_DATA AndroidEntryData[] = {
  //{ L"\\EFI\\boot\\grubx64.efi", L"Grub", L"grub,linux" },
  //{ L"\\EFI\\boot\\bootx64.efi", L"Grub", L"grub,linux" },
  { L"\\EFI\\remixos\\grubx64.efi",         L"Remix",     L"remix,grub,linux",   { L"\\isolinux\\isolinux.bin", L"\\initrd.img", L"\\kernel" } },
  { L"\\EFI\\PhoenixOS\\boot\\grubx64.efi", L"PhoenixOS", L"phoenix,grub,linux", { L"\\EFI\\PhoenixOS\\boot\\efi.img", L"\\EFI\\PhoenixOS\\initrd.img", L"\\EFI\\PhoenixOS\\kernel" } },
  { L"\\EFI\\boot\\grubx64.efi",            L"Phoenix",   L"phoenix,grub,linux", { L"\\phoenix\\kernel", L"\\phoenix\\initrd.img", L"\\phoenix\\ramdisk.img" } },
  { L"\\EFI\\boot\\bootx64.efi",            L"Chrome",    L"chrome,grub,linux",  { L"\\syslinux\\vmlinuz.A", L"\\syslinux\\vmlinuz.B", L"\\syslinux\\ldlinux.sys"} },

};
STATIC CONST UINTN AndroidEntryDataCount = (sizeof(AndroidEntryData) / sizeof(ANDX86_PATH_DATA));
#endif
#endif

CONST CHAR16  *PaperBoot   = L"\\com.apple.boot.P\\boot.efi";
CONST CHAR16  *RockBoot    = L"\\com.apple.boot.R\\boot.efi";
CONST CHAR16  *ScissorBoot = L"\\com.apple.boot.S\\boot.efi";

// OS X installer paths
CONST CHAR16 *OSXInstallerPaths[] = {
  L"\\.IABootFiles\\boot.efi", // 10.9 - 10.13.3
  L"\\Mac OS X Install Data\\boot.efi", // 10.7
  L"\\OS X Install Data\\boot.efi", // 10.8 - 10.11
  L"\\macOS Install Data\\boot.efi", // 10.12 - 10.12.3
  L"\\macOS Install Data\\Locked Files\\Boot Files\\boot.efi" // 10.12.4+
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

UINT8 GetOSTypeFromPath(IN CONST CHAR16 *Path)
{
  if (Path == NULL) {
    return OSTYPE_OTHER;
  }
  if (StriCmp(Path, MACOSX_LOADER_PATH) == 0) {
    return OSTYPE_OSX;
  } else if ((StriCmp(Path, OSXInstallerPaths[0]) == 0) ||
             (StriCmp(Path, OSXInstallerPaths[1]) == 0) ||
             (StriCmp(Path, OSXInstallerPaths[2]) == 0) ||
             (StriCmp(Path, OSXInstallerPaths[3]) == 0) ||
             (StriCmp(Path, OSXInstallerPaths[4]) == 0) ||
             (StriCmp(Path, RockBoot) == 0) || (StriCmp(Path, PaperBoot) == 0) || (StriCmp(Path, ScissorBoot) == 0) ||
             (!StriCmp(Path, L"\\.IABootFiles\\boot.efi") && StriCmp(Path, L"\\.IAPhysicalMedia") && StriCmp(Path, MACOSX_LOADER_PATH))
             ) {
    return OSTYPE_OSX_INSTALLER;
  } else if (StriCmp(Path, L"\\com.apple.recovery.boot\\boot.efi") == 0) {
    return OSTYPE_RECOVERY;
  } else if ((StriCmp(Path, L"\\EFI\\Microsoft\\Boot\\bootmgfw-orig.efi") == 0) || //test first as orig
             (StriCmp(Path, L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi") == 0) ||      //it can be Clover
    //         (StriCmp(Path, L"\\bootmgr.efi") == 0) || //never worked, just extra icon in menu
             (StriCmp(Path, L"\\EFI\\MICROSOFT\\BOOT\\cdboot.efi") == 0)) {
    return OSTYPE_WINEFI;
  } else if (LINUX_FULL_LOADER_PATH.equalIC(Path)) {
    return OSTYPE_LINEFI;
  } else if (StriStr(Path, L"grubx64.efi") != NULL) {
    return OSTYPE_LINEFI;
  } else {
    UINTN Index;
#if defined(ANDX86)
    Index = 0;
    while (Index < AndroidEntryDataCount) {
      if (StriCmp(Path, AndroidEntryData[Index].Path) == 0) {
        return OSTYPE_LIN;
      }
      ++Index;
    }
#endif
    Index = 0;
    while (Index < LinuxEntryDataCount) {
      if (StriCmp(Path, LinuxEntryData[Index].Path) == 0) {
        return OSTYPE_LIN;
      }
      ++Index;
    }
  }
  return OSTYPE_OTHER;
}

STATIC CONST CHAR16 *LinuxIconNameFromPath(IN CONST CHAR16            *Path,
                                     IN EFI_FILE_PROTOCOL *RootDir)
{
  UINTN Index;
#if defined(ANDX86)
  Index = 0;
  while (Index < AndroidEntryDataCount) {
    if (StriCmp(Path, AndroidEntryData[Index].Path) == 0) {
      return AndroidEntryData[Index].Icon;
    }
    ++Index;
  }
#endif
  
  //check not common names
  Index = 0;
  while (Index < LinuxEntryDataCount) {
    if (StriCmp(Path, LinuxEntryData[Index].Path) == 0) {
      return LinuxEntryData[Index].Icon;
    }
    ++Index;
  }
  
  // Try to open the linux issue
  if ((RootDir != NULL) && LINUX_FULL_LOADER_PATH.equalIC(Path)) {
    CHAR8 *Issue = NULL;
    UINTN  IssueLen = 0;
    if (!EFI_ERROR(egLoadFile(RootDir, LINUX_ISSUE_PATH.wc_str(), (UINT8 **)&Issue, &IssueLen)) && (Issue != NULL)) {
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

STATIC CONST XString LinuxInitImagePath[] = {
   "initrd%s"_XS,
   "initrd.img%s"_XS,
   "initrd%s.img"_XS,
   "initramfs%s"_XS,
   "initramfs.img%s"_XS,
   "initramfs%s.img"_XS,
};
STATIC CONST UINTN LinuxInitImagePathCount = (sizeof(LinuxInitImagePath) / sizeof(CHAR16 *));

STATIC XStringArray LinuxKernelOptions(IN EFI_FILE_PROTOCOL *Dir,
                                  IN CONST CHAR16            *Version,
                                  IN CONST CHAR16            *PartUUID,
                                  IN CONST XStringArray&           Options OPTIONAL)
{
  UINTN Index = 0;
  if ((Dir == NULL) || (PartUUID == NULL)) {
    return Options;
  }
  while (Index < LinuxInitImagePathCount) {
    XStringW InitRd = SWPrintf(LinuxInitImagePath[Index++].c_str(), (Version == NULL) ? L"" : Version);
    if (InitRd.notEmpty()) {
      if (FileExists(Dir, InitRd.wc_str())) {
		  XStringArray CustomOptions;
		  CustomOptions.Add(SPrintf("root=/dev/disk/by-partuuid/%ls", PartUUID));
		  CustomOptions.Add(SPrintf("initrd=%ls\\%ls", LINUX_BOOT_ALT_PATH, InitRd.wc_str()));
		  CustomOptions.import(LINUX_DEFAULT_OPTIONS);
		  CustomOptions.import(Options);
        return CustomOptions;
      }
    }
  }
  XStringArray CustomOptions;
  CustomOptions.Add(SPrintf("root=/dev/disk/by-partuuid/%ls", PartUUID));
  CustomOptions.import(LINUX_DEFAULT_OPTIONS);
  CustomOptions.import(Options);
  return CustomOptions;
}

STATIC BOOLEAN isFirstRootUUID(REFIT_VOLUME *Volume)
{
  UINTN         VolumeIndex;
  REFIT_VOLUME *scanedVolume;

  for (VolumeIndex = 0; VolumeIndex < Volumes.size(); VolumeIndex++) {
    scanedVolume = &Volumes[VolumeIndex];

    if (scanedVolume == Volume)
      return TRUE;

    if (CompareGuid(&scanedVolume->RootUUID, &Volume->RootUUID))
      return FALSE;

  }
  return TRUE;
}

//Set Entry->VolName to .disk_label.contentDetails if it exists
STATIC EFI_STATUS GetOSXVolumeName(LOADER_ENTRY *Entry)
{
  EFI_STATUS  Status = EFI_NOT_FOUND;
  CONST CHAR16* targetNameFile = L"\\System\\Library\\CoreServices\\.disk_label.contentDetails";
  CHAR8*  fileBuffer;
  CHAR8*  targetString;
  UINTN   fileLen = 0;
  if(FileExists(Entry->Volume->RootDir, targetNameFile)) {
    Status = egLoadFile(Entry->Volume->RootDir, targetNameFile, (UINT8 **)&fileBuffer, &fileLen);
    if(!EFI_ERROR(Status)) {
      CHAR16  *tmpName;
      //Create null terminated string
      targetString = (CHAR8*) AllocateZeroPool(fileLen+1);
      CopyMem( (VOID*)targetString, (VOID*)fileBuffer, fileLen);
      DBG("found disk_label with contents:%s\n", targetString);

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
      AsciiStrToUnicodeStrS(targetString, tmpName, fileLen);

      Entry->VolName = EfiStrDuplicate(tmpName);
      DBG("Created name:%ls\n", Entry->VolName);

      FreePool(tmpName);
      FreePool(fileBuffer);
      FreePool(targetString);
    }
  }
  return Status;
}
STATIC LOADER_ENTRY *CreateLoaderEntry(IN CONST CHAR16 *LoaderPath,
                                       IN CONST XStringArray& LoaderOptions,
                                       IN CONST XStringW& FullTitle,
                                       IN CONST XStringW& LoaderTitle,
                                       IN REFIT_VOLUME *Volume,
                                       IN XImage *Image,
                                       IN XImage *DriveImage,
                                       IN UINT8 OSType,
                                       IN UINT8 Flags,
                                       IN CHAR16 Hotkey,
                                       EFI_GRAPHICS_OUTPUT_BLT_PIXEL BootBgColor,
                                       IN UINT8 CustomBoot,
                                       IN XImage *CustomLogo,
                                       IN KERNEL_AND_KEXT_PATCHES *Patches,
                                       IN BOOLEAN CustomEntry)
{
  EFI_DEVICE_PATH *LoaderDevicePath;
  CONST CHAR16          *LoaderDevicePathString;
  CONST CHAR16          *FilePathAsString;
  CONST CHAR16          *OSIconName = NULL;
  CHAR16                ShortcutLetter;
  LOADER_ENTRY          *Entry;
  CONST CHAR8           *indent = "    ";

  // Check parameters are valid
  if ((LoaderPath == NULL) || (*LoaderPath == 0) || (Volume == NULL)) {
    return NULL;
  }

  // Get the loader device path
//  LoaderDevicePath = FileDevicePath(Volume->DeviceHandle, (*LoaderPath == L'\\') ? (LoaderPath + 1) : LoaderPath);
  LoaderDevicePath = FileDevicePath(Volume->DeviceHandle, LoaderPath);
  if (LoaderDevicePath == NULL) {
    return NULL;
  }
  LoaderDevicePathString = FileDevicePathToStr(LoaderDevicePath);
  if (LoaderDevicePathString == NULL) {
    return NULL;
  }

  // Ignore this loader if it's self path
  FilePathAsString = FileDevicePathToStr(SelfFullDevicePath);
  if (FilePathAsString) {
    INTN Comparison = StriCmp(FilePathAsString, LoaderDevicePathString);
    FreePool(FilePathAsString);
    if (Comparison == 0) {
      DBG("%s skipped because path `%ls` is self path!\n", indent, LoaderDevicePathString);
      FreePool(LoaderDevicePathString);
      return NULL;
    }
  }
// DBG("OSType =%d\n", OSType);
  if (!CustomEntry) {
    CUSTOM_LOADER_ENTRY *Custom;
    UINTN                CustomIndex = 0;

    // Ignore this loader if it's device path is already present in another loader
//    if (MainMenu.Entries) {
      for (UINTN i = 0; i < MainMenu.Entries.size(); ++i) {
        REFIT_ABSTRACT_MENU_ENTRY& MainEntry = MainMenu.Entries[i];
        // Only want loaders
        if (MainEntry.getLOADER_ENTRY()) {
          if (StriCmp(MainEntry.getLOADER_ENTRY()->DevicePathString, LoaderDevicePathString) == 0) {
            DBG("%s skipped because path `%ls` already exists for another entry!\n", indent, LoaderDevicePathString);
            FreePool(LoaderDevicePathString);
            return NULL;
          }
        }
      }
//    }
    // If this isn't a custom entry make sure it's not hidden by a custom entry
    Custom = gSettings.CustomEntries;
    while (Custom) {
      // Check if the custom entry is hidden or disabled
      if ((OSFLAG_ISSET(Custom->Flags, OSFLAG_DISABLED) ||
           (OSFLAG_ISSET(Custom->Flags, OSFLAG_HIDDEN) && !gSettings.ShowHiddenEntries))) {

        INTN volume_match=0;
        INTN volume_type_match=0;
        INTN path_match=0;
        INTN type_match=0;

        // Check if volume match
        if (Custom->Volume != NULL) {
          // Check if the string matches the volume
          volume_match =
            ((StrStr(Volume->DevicePathString, Custom->Volume) != NULL) ||
             ((Volume->VolName != NULL) && (StrStr(Volume->VolName, Custom->Volume) != NULL))) ? 1 : -1;
        }

        // Check if the volume_type match
        if (Custom->VolumeType != 0) {
          volume_type_match = (((1ull<<Volume->DiskKind) & Custom->VolumeType) != 0) ? 1 : -1;
      /*      (((Volume->DiskKind == DISK_KIND_INTERNAL) && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
             ((Volume->DiskKind == DISK_KIND_EXTERNAL) && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
             ((Volume->DiskKind == DISK_KIND_OPTICAL)  && (Custom->VolumeType & VOLTYPE_OPTICAL))  ||
             ((Volume->DiskKind == DISK_KIND_FIREWIRE) && (Custom->VolumeType & VOLTYPE_FIREWIRE))) ? 1 : -1; */
        }

        // Check if the path match
        if (Custom->Path != NULL) {
          // Check if the loader path match
          path_match = (StriCmp(Custom->Path, LoaderPath) == 0) ? 1 : -1;
        }

        // Check if the type match
        if (Custom->Type != 0) {
          type_match = OSTYPE_COMPARE(Custom->Type, OSType) ? 1 : -1;
        }

        if (volume_match == -1 || volume_type_match == -1 || path_match == -1 || type_match == -1 ) {
          UINTN add_comma = 0;

          DBG("%sNot match custom entry %llu: ", indent, CustomIndex);
          if (volume_match != 0) {
            DBG("Volume: %ls", volume_match == 1 ? L"match" : L"not match");
            add_comma++;
          }
          if (path_match != 0) {
            DBG("%lsPath: %ls",
                (add_comma ? L", " : L""),
                path_match == 1 ? L"match" : L"not match");
            add_comma++;
          }
          if (volume_type_match != 0) {
            DBG("%lsVolumeType: %ls",
                (add_comma ? L", " : L""),
                volume_type_match == 1 ? L"match" : L"not match");
            add_comma++;
          }
          if (type_match != 0) {
            DBG("%lsType: %ls",
                (add_comma ? L", " : L""),
                type_match == 1 ? L"match" : L"not match");
          }
          DBG("\n");
        } else {
          // Custom entry match
          DBG("%sSkipped because matching custom entry %llu!\n", indent, CustomIndex);
          FreePool(LoaderDevicePathString);
          return NULL;
        }
      }
      Custom = Custom->Next;
      ++CustomIndex;
    }
  }
// DBG("prepare the menu entry\n");
  // prepare the menu entry
//  Entry = (__typeof__(Entry))AllocateZeroPool(sizeof(LOADER_ENTRY));
  Entry = new LOADER_ENTRY();
//  Entry->Tag = TAG_LOADER;
  Entry->Row = 0;
  Entry->Volume = Volume;

  Entry->LoaderPath       = EfiStrDuplicate(LoaderPath);
  Entry->VolName          = Volume->VolName;
  Entry->DevicePath       = LoaderDevicePath;
  Entry->DevicePathString = LoaderDevicePathString;
  Entry->Flags            = OSFLAG_SET(Flags, OSFLAG_USEGRAPHICS);
//  if (LoaderOptions.notEmpty()) {
//    if (OSFLAG_ISSET(Flags, OSFLAG_NODEFAULTARGS)) {
//      Entry->LoadOptions  = LoaderOptions;
//    } else {
//      Entry->LoadOptions  = SPrintf("%s %s", gSettings.BootArgs, LoaderOptions.c_str());
//    }
//  } else if ((AsciiStrLen(gSettings.BootArgs) > 0) && OSFLAG_ISUNSET(Flags, OSFLAG_NODEFAULTARGS)) {
//    Entry->LoadOptions    = SPrintf("%s", gSettings.BootArgs);
//  }
  if (OSFLAG_ISSET(Flags, OSFLAG_NODEFAULTARGS)) {
	  Entry->LoadOptions  = LoaderOptions;
  }else{
	  Entry->LoadOptions = Split<XStringArray>(gSettings.BootArgs, " ");
	  Entry->LoadOptions.import(LoaderOptions);
  }
  //actions
  Entry->AtClick = ActionSelect;
  Entry->AtDoubleClick = ActionEnter;
  Entry->AtRightClick = ActionDetails;
  Entry->CustomBoot = CustomBoot;
  if (CustomLogo != nullptr) {
    Entry->CustomLogo = *CustomLogo; //else empty ximage already constructed
  }

  Entry->LoaderType = OSType;
  Entry->BuildVersion = NULL;
  Entry->OSVersion = GetOSVersion(Entry);
//DBG("OSVersion=%s \n", Entry->OSVersion);
  // detect specific loaders
  OSIconName = NULL;
  ShortcutLetter = 0;

  switch (OSType) {
    case OSTYPE_OSX:
    case OSTYPE_RECOVERY:
    case OSTYPE_OSX_INSTALLER:
      OSIconName = GetOSIconName(Entry->OSVersion);// Sothor - Get OSIcon name using OSVersion
      // apianti - force custom logo even when verbose
      if ( Entry->LoadOptions.containsIC("-v")  ) {
        // OSX is not booting verbose, so we can set console to graphics mode
        Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_USEGRAPHICS);
      }
      if (OSType == OSTYPE_OSX && IsOsxHibernated(Entry)) {
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_HIBERNATED);
        DBG("  =>set entry as hibernated\n");
      }
      //always unset checkFakeSmc for installer
      if (OSType == OSTYPE_OSX_INSTALLER){
        Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_CHECKFAKESMC);
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
      }
      ShortcutLetter = 'M';
      if ((Entry->VolName == NULL) || (StrLen(Entry->VolName) == 0)) {
        // else no sense to override it with dubious name
        GetOSXVolumeName(Entry);
      }
      break;
    case OSTYPE_WIN:
      OSIconName = L"win";
      ShortcutLetter = 'W';
      break;
    case OSTYPE_WINEFI:
      OSIconName = L"vista,win";
      //ShortcutLetter = 'V';
      ShortcutLetter = 'W';
      break;
    case OSTYPE_LIN:
    case OSTYPE_LINEFI:
    // we already detected linux and have Path and Image
      Entry->LoaderType = OSType;
      OSIconName = L"linux";
      if (Image == nullptr) {
        DBG(" image not found\n");
        OSIconName = LinuxIconNameFromPath(LoaderPath, Volume->RootDir); //something named "issue"
      }
      ShortcutLetter = 'L';
      break;
    //case OSTYPE_OTHER: 
    case OSTYPE_EFI:
      OSIconName = L"clover";
      ShortcutLetter = 'E';
      Entry->LoaderType = OSTYPE_OTHER;
      break;
    default:
      OSIconName = L"unknown";
      Entry->LoaderType = OSTYPE_OTHER;
      break;
  }
//DBG("OSIconName=%ls \n", OSIconName);
  Entry->Title = FullTitle;
  if (Entry->Title.isEmpty()  &&  Volume->VolLabel != NULL) {
    if (Volume->VolLabel[0] == L'#') {
    	Entry->Title.SWPrintf("Boot %ls from %ls", (!LoaderTitle.isEmpty()) ? LoaderTitle.wc_str() : Basename(LoaderPath), Volume->VolLabel+1);
    }else{
    	Entry->Title.SWPrintf("Boot %ls from %ls", (!LoaderTitle.isEmpty()) ? LoaderTitle.wc_str() : Basename(LoaderPath), Volume->VolLabel);
    }
  }

  BOOLEAN BootCampStyle = ThemeX.BootCampStyle;

  if ( Entry->Title.isEmpty()  &&  ((Entry->VolName == NULL) || (StrLen(Entry->VolName) == 0)) ) {
 //   DBG("encounter Entry->VolName ==%ls and StrLen(Entry->VolName) ==%llu\n",Entry->VolName, StrLen(Entry->VolName));
    if (BootCampStyle) {
      if (!LoaderTitle.isEmpty()) {
        Entry->Title = LoaderTitle;
      } else {
        Entry->Title.takeValueFrom(Basename(Volume->DevicePathString));
      }
    } else {
      Entry->Title.SWPrintf("Boot %ls from %ls", (!LoaderTitle.isEmpty()) ? LoaderTitle.wc_str() : Basename(LoaderPath),
                            Basename(Volume->DevicePathString));
    }
  }
//  DBG("check Entry->Title \n");
  if ( Entry->Title.isEmpty() ) {
 //   DBG("encounter LoaderTitle ==%ls and Entry->VolName ==%ls\n", LoaderTitle.wc_str(), Entry->VolName);
    if (BootCampStyle) {
      if ((StriCmp(LoaderTitle.wc_str(), L"macOS") == 0) || (StriCmp(LoaderTitle.wc_str(), L"Recovery") == 0)) {
        Entry->Title.takeValueFrom(Entry->VolName);
      } else {
        if (!LoaderTitle.isEmpty()) {
          Entry->Title = LoaderTitle;
        } else {
          Entry->Title.takeValueFrom(Basename(LoaderPath));
        }
      }
    } else {
      Entry->Title.SWPrintf("Boot %ls from %ls", (!LoaderTitle.isEmpty()) ? LoaderTitle.wc_str() : Basename(LoaderPath),
                            Entry->VolName);
    }
  }
//  DBG("Entry->Title =%ls\n", Entry->Title.wc_str());
  // just an example that UI can show hibernated volume to the user
  // should be better to show it on entry image
  if (OSFLAG_ISSET(Entry->Flags, OSFLAG_HIBERNATED)) {
    Entry->Title.SWPrintf("%ls (hibernated)", Entry->Title.s());
  }

  Entry->ShortcutLetter = (Hotkey == 0) ? ShortcutLetter : Hotkey;

  // get custom volume icon if present
  if (GlobalConfig.CustomIcons && FileExists(Volume->RootDir, L"\\.VolumeIcon.icns")){
    Entry->Image.LoadIcns(Volume->RootDir, L"\\.VolumeIcon.icns", 128);
    DBG("using VolumeIcon.icns image from Volume\n");
  } else if (Image) {
    Entry->Image = *Image; //copy image from temporary storage
  } else {
    Entry->Image = ThemeX.LoadOSIcon(OSIconName);
  }
//  DBG("Load DriveImage\n");
  // Load DriveImage
  if (DriveImage) {
//    DBG("DriveImage presents\n");
    Entry->DriveImage = *DriveImage;
  } else {
    Entry->DriveImage = ScanVolumeDefaultIcon(Volume, Entry->LoaderType, Volume->DevicePath);
  }
//   DBG("HideBadges=%llu Volume=%ls ", ThemeX.HideBadges, Volume->VolName);
  if (ThemeX.HideBadges & HDBADGES_SHOW) {
    if (ThemeX.HideBadges & HDBADGES_SWAP) {
//      Entry->BadgeImage = egCopyScaledImage(Entry->DriveImage, ThemeX.BadgeScale);
      Entry->BadgeImage = XImage(Entry->DriveImage, 0); //ThemeX.BadgeScale/16.f);
       DBG(" Show badge as Drive.");
    } else {
      Entry->BadgeImage = XImage(Entry->Image, 0); //ThemeX.BadgeScale/16.f);
 //     Entry->BadgeImage = egCopyScaledImage((Entry->Image).ToEGImage(), ThemeX.BadgeScale);
       DBG(" Show badge as OSImage.");
    }
  }
  Entry->BootBgColor = BootBgColor;

  Entry->KernelAndKextPatches = ((Patches == NULL) ? (KERNEL_AND_KEXT_PATCHES *)(((UINTN)&gSettings) + OFFSET_OF(SETTINGS_DATA, KernelAndKextPatches)) : Patches);
#ifdef DUMP_KERNEL_KEXT_PATCHES
  DumpKernelAndKextPatches(Entry->KernelAndKextPatches);
#endif
  DBG("%sLoader entry created for '%ls'\n", indent, Entry->DevicePathString);
  return Entry;
}

STATIC VOID AddDefaultMenu(IN LOADER_ENTRY *Entry)
{
  CONST CHAR16            *FileName;
//  CHAR16* TempOptions;
//  CHAR16            DiagsFileName[256];
  LOADER_ENTRY      *SubEntry;
  REFIT_MENU_SCREEN *SubScreen;
  REFIT_VOLUME      *Volume;
  UINT64            VolumeSize;
  EFI_GUID          *Guid = NULL;
  BOOLEAN           KernelIs64BitOnly;
  UINT64            os_version = AsciiOSVersionToUint64(Entry->OSVersion);

  if (Entry == NULL) {
    return;
  }
  Volume = Entry->Volume;
  if (Volume == NULL) {
    return;
  }

  // Only kernels up to 10.7 have 32-bit mode
  KernelIs64BitOnly = (Entry->OSVersion == NULL ||
                       AsciiOSVersionToUint64(Entry->OSVersion) >= AsciiOSVersionToUint64("10.8"));

  FileName = Basename(Entry->LoaderPath);

  // create the submenu
//  SubScreen = (__typeof__(SubScreen))AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
  SubScreen = new REFIT_MENU_SCREEN;
  SubScreen->Title.SWPrintf("Options for %ls on %ls", Entry->Title.wc_str(), Entry->VolName);

  SubScreen->TitleImage = Entry->Image;
  SubScreen->ID = Entry->LoaderType + 20; //wow
    DBG("get anime for os=%lld\n", SubScreen->ID);
  SubScreen->GetAnime();
  VolumeSize = RShiftU64(MultU64x32(Volume->BlockIO->Media->LastBlock, Volume->BlockIO->Media->BlockSize), 20);
	SubScreen->AddMenuInfoLine_f("Volume size: %lluMb", VolumeSize);
  SubScreen->AddMenuInfoLine_f("%ls", FileDevicePathToStr(Entry->DevicePath));
  Guid = FindGPTPartitionGuidInDevicePath(Volume->DevicePath);
  if (Guid) {
	SubScreen->AddMenuInfoLine_f("UUID: %s", strguid(Guid));
  }
	SubScreen->AddMenuInfoLine_f("Options: %s", Entry->LoadOptions.ConcatAll(" "_XS).c_str());
  // loader-specific submenu entries
  if (Entry->LoaderType == OSTYPE_OSX ||
      Entry->LoaderType == OSTYPE_OSX_INSTALLER ||
      Entry->LoaderType == OSTYPE_RECOVERY) { // entries for Mac OS X
    if (os_version < AsciiOSVersionToUint64("10.8")) {
		SubScreen->AddMenuInfoLine_f("Mac OS X: %s", Entry->OSVersion);
    } else if (os_version < AsciiOSVersionToUint64("10.12")) {
		SubScreen->AddMenuInfoLine_f("OS X: %s", Entry->OSVersion);
    } else {
		SubScreen->AddMenuInfoLine_f("macOS: %s", Entry->OSVersion);
    }

    if (OSFLAG_ISSET(Entry->Flags, OSFLAG_HIBERNATED)) {
      SubEntry = Entry->getPartiallyDuplicatedEntry();
      if (SubEntry) {
        SubEntry->Title.takeValueFrom("Cancel hibernate wake");
        SubEntry->Flags     = OSFLAG_UNSET(SubEntry->Flags, OSFLAG_HIBERNATED);
        SubScreen->AddMenuEntry(SubEntry, true);
      }
    }

    SubEntry = Entry->getPartiallyDuplicatedEntry();
    if (SubEntry) {
      if (os_version < AsciiOSVersionToUint64("10.8")) {
        SubEntry->Title.takeValueFrom("Boot Mac OS X with selected options");
      } else if (os_version < AsciiOSVersionToUint64("10.12")) {
        SubEntry->Title.takeValueFrom("Boot OS X with selected options");
      } else {
        SubEntry->Title.takeValueFrom("Boot macOS with selected options");
      }
      SubScreen->AddMenuEntry(SubEntry, true);
    }
    
    SubEntry = Entry->getPartiallyDuplicatedEntry();
    if (SubEntry) {
      if (os_version < AsciiOSVersionToUint64("10.8")) {
        SubEntry->Title.takeValueFrom("Boot Mac OS X with injected kexts");
      } else if (os_version < AsciiOSVersionToUint64("10.12")) {
        SubEntry->Title.takeValueFrom("Boot OS X with injected kexts");
      } else {
        SubEntry->Title.takeValueFrom("Boot macOS with injected kexts");
      }
      SubEntry->Flags       = OSFLAG_UNSET(SubEntry->Flags, OSFLAG_CHECKFAKESMC);
      SubEntry->Flags       = OSFLAG_SET(SubEntry->Flags, OSFLAG_WITHKEXTS);
      SubScreen->AddMenuEntry(SubEntry, true);
    }
    SubEntry = Entry->getPartiallyDuplicatedEntry();
    if (SubEntry) {
      if (os_version < AsciiOSVersionToUint64("10.8")) {
        SubEntry->Title.takeValueFrom("Boot Mac OS X without injected kexts");
      } else if (os_version < AsciiOSVersionToUint64("10.12")) {
        SubEntry->Title.takeValueFrom("Boot OS X without injected kexts");
      } else {
        SubEntry->Title.takeValueFrom("Boot macOS without injected kexts");
      }
      SubEntry->Flags       = OSFLAG_UNSET(SubEntry->Flags, OSFLAG_CHECKFAKESMC);
      SubEntry->Flags       = OSFLAG_UNSET(SubEntry->Flags, OSFLAG_WITHKEXTS);
      SubScreen->AddMenuEntry(SubEntry, true);
    }

    SubScreen->AddMenuEntry(SubMenuKextInjectMgmt(Entry), true);
    SubScreen->AddMenuInfo_f("=== boot-args ===");
    if (!KernelIs64BitOnly) {
      if (os_version < AsciiOSVersionToUint64("10.8")) {
        SubScreen->AddMenuCheck("Mac OS X 32bit",   OPT_I386, 68);
        SubScreen->AddMenuCheck("Mac OS X 64bit",   OPT_X64,  68);
      } else if (os_version < AsciiOSVersionToUint64("10.12")) {
        SubScreen->AddMenuCheck("OS X 32bit",       OPT_I386, 68);
        SubScreen->AddMenuCheck("OS X 64bit",       OPT_X64,  68);
      } else {
        SubScreen->AddMenuCheck("macOS 32bit",      OPT_I386, 68);
        SubScreen->AddMenuCheck("macOS 64bit",      OPT_X64,  68);
      }
    }
    SubScreen->AddMenuCheck("Verbose (-v)",                               OPT_VERBOSE, 68);
    // No Caches option works on 10.6 - 10.9
    if (os_version < AsciiOSVersionToUint64("10.10")) {
      SubScreen->AddMenuCheck("Without caches (-f)",                        OPT_NOCACHES, 68);
    }
    SubScreen->AddMenuCheck("Single User (-s)",                           OPT_SINGLE_USER, 68);
    SubScreen->AddMenuCheck("Safe Mode (-x)",                             OPT_SAFE, 68);
    SubScreen->AddMenuCheck("Disable KASLR (slide=0)",                    OPT_SLIDE, 68);
    SubScreen->AddMenuCheck("Set Nvidia to VESA (nv_disable=1)",          OPT_NVDISABLE, 68);
    SubScreen->AddMenuCheck("Use Nvidia WEB drivers (nvda_drv=1)",        OPT_NVWEBON, 68);
    SubScreen->AddMenuCheck("Disable PowerNap (darkwake=0)",              OPT_POWERNAPOFF, 68);
    SubScreen->AddMenuCheck("Use XNU CPUPM (-xcpm)",                      OPT_XCPM, 68);
//    SubScreen->AddMenuCheck("Disable Intel Idle Mode (-gux_no_idle)",     OPT_GNOIDLE, 68);
//    SubScreen->AddMenuCheck("Sleep Uses Shutdown (-gux_nosleep)",         OPT_GNOSLEEP, 68);
//    SubScreen->AddMenuCheck("Force No Msi Int (-gux_nomsi)",              OPT_GNOMSI, 68);
//    SubScreen->AddMenuCheck("EHC manage USB2 ports (-gux_defer_usb2)",    OPT_EHCUSB, 68);
    SubScreen->AddMenuCheck("Keep symbols on panic (keepsyms=1)",         OPT_KEEPSYMS, 68);
    SubScreen->AddMenuCheck("Don't reboot on panic (debug=0x100)",        OPT_DEBUG, 68);
    SubScreen->AddMenuCheck("Debug kexts (kextlog=0xffff)",               OPT_KEXTLOG, 68);
//    SubScreen->AddMenuCheck("Disable AppleALC (-alcoff)",                 OPT_APPLEALC, 68);
//    SubScreen->AddMenuCheck("Disable Shiki (-shikioff)",                  OPT_SHIKI, 68);

    if (gSettings.CsrActiveConfig == 0) {
      SubScreen->AddMenuCheck("No SIP", OSFLAG_NOSIP, 69);
    }
    
  } else if (Entry->LoaderType == OSTYPE_LINEFI) {
    BOOLEAN Quiet = Entry->LoadOptions.contains("quiet");
    BOOLEAN WithSplash = Entry->LoadOptions.contains("splash");
    
    // default entry
    SubEntry = Entry->getPartiallyDuplicatedEntry();
    if (SubEntry) {
      SubEntry->Title.SWPrintf("Run %ls", FileName);
      SubScreen->AddMenuEntry(SubEntry, true);
    }

    SubEntry = Entry->getPartiallyDuplicatedEntry();
    if (SubEntry) {
      if (Quiet) {
        SubEntry->Title.SWPrintf("%ls verbose", Entry->Title.s());
        SubEntry->LoadOptions.removeIC("quiet"_XS);
      } else {
        SubEntry->Title.SWPrintf("%ls quiet", Entry->Title.s());
        SubEntry->LoadOptions.AddID("quiet"_XS);
      }
    }
    SubScreen->AddMenuEntry(SubEntry, true);
    SubEntry = Entry->getPartiallyDuplicatedEntry();
    if (SubEntry) {
      if (WithSplash) {
        SubEntry->Title.SWPrintf("%ls without splash", Entry->Title.s());
        SubEntry->LoadOptions.removeIC("splash"_XS);
      } else {
        SubEntry->Title.SWPrintf("%ls with splash", Entry->Title.s());
        SubEntry->LoadOptions.AddID("splash"_XS);
      }
    }
    SubScreen->AddMenuEntry(SubEntry, true);
    SubEntry = Entry->getPartiallyDuplicatedEntry();
    if (SubEntry) {
      if (WithSplash) {
        if (Quiet) {
          SubEntry->Title.SWPrintf("%ls verbose without splash", Entry->Title.s());
          SubEntry->LoadOptions.removeIC("splash"_XS);
          SubEntry->LoadOptions.removeIC("quiet"_XS);
        } else {
          SubEntry->Title.SWPrintf("%ls quiet without splash", Entry->Title.s());
          SubEntry->LoadOptions.removeIC("splash"_XS);
          SubEntry->LoadOptions.Add("quiet"_XS);
        }
      } else if (Quiet) {
//        TempOptions.RemoveIC("quiet"_XS);
        SubEntry->Title.SWPrintf("%ls verbose with splash", Entry->Title.s());
        SubEntry->LoadOptions.AddID("splash"_XS);
//        FreePool(TempOptions);
      } else {
        SubEntry->Title.SWPrintf("%ls quiet with splash", Entry->Title.s());
        SubEntry->LoadOptions.AddID("quiet"_XS);
        SubEntry->LoadOptions.AddID("splash"_XS);
      }
    }
    SubScreen->AddMenuEntry(SubEntry, true);
  } else if ((Entry->LoaderType == OSTYPE_WIN) || (Entry->LoaderType == OSTYPE_WINEFI)) {
    // by default, skip the built-in selection and boot from hard disk only
    Entry->LoadOptions.setEmpty();
    Entry->LoadOptions.Add("-s"_XS);
    Entry->LoadOptions.Add("-h"_XS);
    
    // default entry
    SubEntry = Entry->getPartiallyDuplicatedEntry();
    if (SubEntry) {
      SubEntry->Title.SWPrintf("Run %ls", FileName);
      SubScreen->AddMenuEntry(SubEntry, true);
    }

    SubEntry = Entry->getPartiallyDuplicatedEntry();
    if (SubEntry) {
      SubEntry->Title.takeValueFrom("Boot Windows from Hard Disk");
      SubScreen->AddMenuEntry(SubEntry, true);
    }

    SubEntry = Entry->getPartiallyDuplicatedEntry();
    if (SubEntry) {
      SubEntry->Title.takeValueFrom("Boot Windows from CD-ROM");
      Entry->LoadOptions.setEmpty();
      Entry->LoadOptions.Add("-s"_XS);
      Entry->LoadOptions.Add("-c"_XS);
      SubScreen->AddMenuEntry(SubEntry, true);
    }

    SubEntry = Entry->getPartiallyDuplicatedEntry();
    if (SubEntry) {
      SubEntry->Title.SWPrintf("Run %ls in text mode", FileName);
      SubEntry->Flags           = OSFLAG_UNSET(SubEntry->Flags, OSFLAG_USEGRAPHICS);
      Entry->LoadOptions.setEmpty();
      Entry->LoadOptions.Add("-v"_XS);
      SubEntry->LoaderType      = OSTYPE_OTHER; // Sothor - Why are we using OSTYPE_OTHER here?
      SubScreen->AddMenuEntry(SubEntry, true);
    }

  }

  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  Entry->SubScreen = SubScreen;
  // DBG("    Added '%ls': OSType='%d', OSVersion='%s'\n", Entry->Title, Entry->LoaderType, Entry->OSVersion);
}
BOOLEAN AddLoaderEntry(IN CONST CHAR16 *LoaderPath, IN CONST XStringArray& LoaderOptions,
                       IN CONST XStringW& LoaderTitle,
                       IN REFIT_VOLUME *Volume, IN XImage *Image,
                       IN UINT8 OSType, IN UINT8 Flags)
{
  LOADER_ENTRY *Entry;
  INTN          HVi;

  if ((LoaderPath == NULL) || (Volume == NULL) || (Volume->RootDir == NULL) || !FileExists(Volume->RootDir, LoaderPath)) {
    return FALSE;
  }

  DBG("        AddLoaderEntry for Volume Name=%ls\n", Volume->VolName);
  if (OSFLAG_ISSET(Flags, OSFLAG_DISABLED)) {
    DBG("        skipped because entry is disabled\n");
    return FALSE;
  }
  if (!gSettings.ShowHiddenEntries && OSFLAG_ISSET(Flags, OSFLAG_HIDDEN)) {
    DBG("        skipped because entry is hidden\n");
    return FALSE;
  }
  //don't add hided entries
  if (!gSettings.ShowHiddenEntries) {
    for (HVi = 0; HVi < gSettings.HVCount; HVi++) {
      if (StriStr(LoaderPath, gSettings.HVHideStrings[HVi])) {
        DBG("        hiding entry: %ls\n", LoaderPath);
        return FALSE;
      }
    }
  }
  Entry = CreateLoaderEntry(LoaderPath, LoaderOptions, L""_XSW, LoaderTitle, Volume, Image, NULL, OSType, Flags, 0, MenuBackgroundPixel, CUSTOM_BOOT_DISABLED, NULL, NULL, FALSE);
  if (Entry != NULL) {
    if ((Entry->LoaderType == OSTYPE_OSX) ||
        (Entry->LoaderType == OSTYPE_OSX_INSTALLER ) ||
        (Entry->LoaderType == OSTYPE_RECOVERY)) {
      if (gSettings.WithKexts) {
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
      }
      if (gSettings.WithKextsIfNoFakeSMC) {
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_CHECKFAKESMC);
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
      }
      if (gSettings.NoCaches) {
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NOCACHES);
      }
    }
    //TODO there is a problem that Entry->Flags is unique while InputItems are global ;(
//    InputItems[69].IValue = Entry->Flags;
    AddDefaultMenu(Entry);
    MainMenu.AddMenuEntry(Entry, true);
    return TRUE;
  }
  return FALSE;
}
//constants
CHAR16  APFSFVBootPath[75]      = L"\\00000000-0000-0000-0000-000000000000\\System\\Library\\CoreServices\\boot.efi"; 
CHAR16  APFSRecBootPath[47]     = L"\\00000000-0000-0000-0000-000000000000\\boot.efi";
CHAR16  APFSInstallBootPath[67] = L"\\00000000-0000-0000-0000-000000000000\\com.apple.installer\\boot.efi";

#define Paper 1
#define Rock  2
#define Scissor 4

VOID AddPRSEntry(REFIT_VOLUME *Volume)
{
  INTN WhatBoot = 0;
  //CONST INTN Paper = 1;
  //CONST INTN Rock = 2;
  //CONST INTN Scissor = 4;

  WhatBoot |= FileExists(Volume->RootDir, RockBoot)?Rock:0;
  WhatBoot |= FileExists(Volume->RootDir, PaperBoot)?Paper:0;
  WhatBoot |= FileExists(Volume->RootDir, ScissorBoot)?Scissor:0;
  switch (WhatBoot) {
    case Paper:
    case (Paper | Rock):
      AddLoaderEntry(PaperBoot, NullXStringArray, L"macOS InstallP"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0);
      break;
    case Scissor:
    case (Paper | Scissor):
      AddLoaderEntry(ScissorBoot, NullXStringArray, L"macOS InstallS"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0);
      break;
    case Rock:
    case (Rock | Scissor):
    case (Rock | Scissor | Paper):
      AddLoaderEntry(RockBoot, NullXStringArray, L"macOS InstallR"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0);
      break;

    default:
      break;
  }
}
#undef Paper
#undef Rock
#undef Scissor

VOID ScanLoader(VOID)
{
  UINTN         VolumeIndex, Index;
  REFIT_VOLUME *Volume;
  EFI_GUID     *PartGUID;

  //DBG("Scanning loaders...\n");
  DbgHeader("ScanLoader");
   
  for (VolumeIndex = 0; VolumeIndex < Volumes.size(); VolumeIndex++) {
    Volume = &Volumes[VolumeIndex];
    if (Volume->RootDir == NULL) { // || Volume->VolName == NULL)
      //DBG(", no file system\n", VolumeIndex);
      continue;
    }
	  DBG("- [%02llu]: '%ls'", VolumeIndex, Volume->VolName);
    if (Volume->VolName == NULL) {
      Volume->VolName = L"Unknown";
    }

    // skip volume if its kind is configured as disabled
 /*   if ((Volume->DiskKind == DISK_KIND_OPTICAL && (GlobalConfig.DisableFlags & VOLTYPE_OPTICAL)) ||
        (Volume->DiskKind == DISK_KIND_EXTERNAL && (GlobalConfig.DisableFlags & VOLTYPE_EXTERNAL)) ||
        (Volume->DiskKind == DISK_KIND_INTERNAL && (GlobalConfig.DisableFlags & VOLTYPE_INTERNAL)) ||
        (Volume->DiskKind == DISK_KIND_FIREWIRE && (GlobalConfig.DisableFlags & VOLTYPE_FIREWIRE))) */
    if (((1ull<<Volume->DiskKind) & GlobalConfig.DisableFlags) != 0)
    {
      DBG(", flagged disable\n");
      continue;
    }

    if (Volume->Hidden) {
      DBG(", hidden\n");
      continue;
    }
    DBG("\n");

    // check for Mac OS X Install Data
    // 1st stage - createinstallmedia
    if (FileExists(Volume->RootDir, L"\\.IABootFiles\\boot.efi")) {
      if (FileExists(Volume->RootDir, L"\\Install OS X Mavericks.app") ||
          FileExists(Volume->RootDir, L"\\Install OS X Yosemite.app") ||
          FileExists(Volume->RootDir, L"\\Install OS X El Capitan.app")) {
        AddLoaderEntry(L"\\.IABootFiles\\boot.efi", NullXStringArray, L"OS X Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.9 - 10.11
      } else {
        AddLoaderEntry(L"\\.IABootFiles\\boot.efi", NullXStringArray, L"macOS Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.12 - 10.13.3
      }
    } else if (FileExists(Volume->RootDir, L"\\.IAPhysicalMedia") && FileExists(Volume->RootDir, MACOSX_LOADER_PATH)) {
      AddLoaderEntry(MACOSX_LOADER_PATH, NullXStringArray, L"macOS Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.13.4+
    }
    // 2nd stage - InstallESD/AppStore/startosinstall/Fusion Drive
    AddLoaderEntry(L"\\Mac OS X Install Data\\boot.efi", NullXStringArray, L"Mac OS X Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.7
    AddLoaderEntry(L"\\OS X Install Data\\boot.efi", NullXStringArray, L"OS X Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.8 - 10.11
    AddLoaderEntry(L"\\macOS Install Data\\boot.efi", NullXStringArray, L"macOS Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.12 - 10.12.3
    AddLoaderEntry(L"\\macOS Install Data\\Locked Files\\Boot Files\\boot.efi", NullXStringArray, L"macOS Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.12.4+
    AddPRSEntry(Volume); // 10.12+

    // Netinstall
    AddLoaderEntry(L"\\NetInstall macOS High Sierra.nbi\\i386\\booter", NullXStringArray, L"macOS Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0);
    // Use standard location for boot.efi, according to the install files is present
    // That file indentifies a DVD/ESD/BaseSystem/Fusion Drive Install Media, so when present, check standard path to avoid entry duplication
    if (FileExists(Volume->RootDir, MACOSX_LOADER_PATH)) {
      if (FileExists(Volume->RootDir, L"\\System\\Installation\\CDIS\\Mac OS X Installer.app")) {
        // InstallDVD/BaseSystem
        AddLoaderEntry(MACOSX_LOADER_PATH, NullXStringArray, L"Mac OS X Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.6/10.7
      } else if (FileExists(Volume->RootDir, L"\\System\\Installation\\CDIS\\OS X Installer.app")) {
        // BaseSystem
        AddLoaderEntry(MACOSX_LOADER_PATH, NullXStringArray, L"OS X Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.8 - 10.11
      } else if (FileExists(Volume->RootDir, L"\\System\\Installation\\CDIS\\macOS Installer.app")) {
        // BaseSystem
        AddLoaderEntry(MACOSX_LOADER_PATH, NullXStringArray, L"macOS Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.12+
      } else if (FileExists(Volume->RootDir, L"\\BaseSystem.dmg") && FileExists(Volume->RootDir, L"\\mach_kernel")) {
        // InstallESD
        if (FileExists(Volume->RootDir, L"\\MacOSX_Media_Background.png")) {
          AddLoaderEntry(MACOSX_LOADER_PATH, NullXStringArray, L"Mac OS X Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.7
        } else {
          AddLoaderEntry(MACOSX_LOADER_PATH, NullXStringArray, L"OS X Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.8
        }
      } else if (FileExists(Volume->RootDir, L"\\com.apple.boot.R\\System\\Library\\PrelinkedKernels\\prelinkedkernel") ||
                 FileExists(Volume->RootDir, L"\\com.apple.boot.P\\System\\Library\\PrelinkedKernels\\prelinkedkernel") ||
                 FileExists(Volume->RootDir, L"\\com.apple.boot.S\\System\\Library\\PrelinkedKernels\\prelinkedkernel")) {
        if (StriStr(Volume->VolName, L"Recovery") != NULL) {
          // FileVault of HFS+
          // TODO: need info for 10.11 and lower
          AddLoaderEntry(MACOSX_LOADER_PATH, NullXStringArray, L"macOS FileVault"_XSW, Volume, NULL, OSTYPE_OSX, 0); // 10.12+
        } else {
          // Fusion Drive
          AddLoaderEntry(MACOSX_LOADER_PATH, NullXStringArray, L"OS X Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.11
        }
      } else if (!FileExists(Volume->RootDir, L"\\.IAPhysicalMedia")) {
        // Installed
        if (EFI_ERROR(GetRootUUID(Volume)) || isFirstRootUUID(Volume)) {
          if (!FileExists(Volume->RootDir, L"\\System\\Library\\CoreServices\\NotificationCenter.app") && !FileExists(Volume->RootDir, L"\\System\\Library\\CoreServices\\Siri.app")) {
            AddLoaderEntry(MACOSX_LOADER_PATH, NullXStringArray, L"Mac OS X"_XSW, Volume, NULL, OSTYPE_OSX, 0); // 10.6 - 10.7
          } else if (FileExists(Volume->RootDir, L"\\System\\Library\\CoreServices\\NotificationCenter.app") && !FileExists(Volume->RootDir, L"\\System\\Library\\CoreServices\\Siri.app")) {
            AddLoaderEntry(MACOSX_LOADER_PATH, NullXStringArray, L"OS X"_XSW, Volume, NULL, OSTYPE_OSX, 0); // 10.8 - 10.11
          } else {
            AddLoaderEntry(MACOSX_LOADER_PATH, NullXStringArray, L"macOS"_XSW, Volume, NULL, OSTYPE_OSX, 0); // 10.12+
          }
        }
      }
    }
    /* APFS Container support.
     * s.mtr 2017
     */
    if ((StriCmp(Volume->VolName, L"Recovery") == 0 || StriCmp(Volume->VolName, L"Preboot") == 0) && APFSSupport == TRUE) {
      for (UINTN i = 0; i < APFSUUIDBankCounter + 1; i++) {
        //Store current UUID
        CHAR16 *CurrentUUID = GuidLEToStr((EFI_GUID *)((UINT8 *)APFSUUIDBank + i * 0x10));
        //Fill with current UUID
        StrnCpy(APFSFVBootPath + 1, CurrentUUID, 36);
        StrnCpy(APFSRecBootPath + 1, CurrentUUID, 36);
        StrnCpy(APFSInstallBootPath + 1, CurrentUUID, 36);
        //Try to add FileVault entry
        AddLoaderEntry(APFSFVBootPath, NullXStringArray, L"FileVault Prebooter"_XSW, Volume, NULL, OSTYPE_OSX, 0);
        //Try to add Recovery APFS entry
        AddLoaderEntry(APFSRecBootPath, NullXStringArray, L"Recovery"_XSW, Volume, NULL, OSTYPE_RECOVERY, 0);
        //Try to add macOS install entry
        AddLoaderEntry(APFSInstallBootPath, NullXStringArray, L"macOS Install Prebooter"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0);
        FreePool(CurrentUUID);
      }
    }
    // check for Mac OS X Recovery Boot
    AddLoaderEntry(L"\\com.apple.recovery.boot\\boot.efi", NullXStringArray, L"Recovery"_XSW, Volume, NULL, OSTYPE_RECOVERY, 0);

    // Sometimes, on some systems (HP UEFI, if Win is installed first)
    // it is needed to get rid of bootmgfw.efi to allow starting of
    // Clover as /efi/boot/bootx64.efi from HD. We can do that by renaming
    // bootmgfw.efi to bootmgfw-orig.efi
    AddLoaderEntry(L"\\EFI\\microsoft\\Boot\\bootmgfw-orig.efi", NullXStringArray, L"Microsoft EFI"_XSW, Volume, NULL, OSTYPE_WINEFI, 0);
    // check for Microsoft boot loader/menu
    // If there is bootmgfw-orig.efi, then do not check for bootmgfw.efi
    // since on some systems this will actually be CloverX64.efi
    // renamed to bootmgfw.efi
    AddLoaderEntry(L"\\EFI\\microsoft\\Boot\\bootmgfw.efi", NullXStringArray, L"Microsoft EFI Boot"_XSW, Volume, NULL, OSTYPE_WINEFI, 0);
    // check for Microsoft boot loader/menu. This entry is redundant so excluded
    // AddLoaderEntry(L"\\bootmgr.efi", L"", L"Microsoft EFI mgrboot", Volume, NULL, OSTYPE_WINEFI, 0);
    // check for Microsoft boot loader/menu on CDROM
    if (!AddLoaderEntry(L"\\EFI\\MICROSOFT\\BOOT\\cdboot.efi", NullXStringArray, L"Microsoft EFI cdboot"_XSW, Volume, NULL, OSTYPE_WINEFI, 0)) {
      AddLoaderEntry(L"\\EFI\\MICROSOF\\BOOT\\CDBOOT.EFI", NullXStringArray, L"Microsoft EFI CDBOOT"_XSW, Volume, NULL, OSTYPE_WINEFI, 0);
    }
#if defined(ANDX86)
    if (TRUE) { //gSettings.AndroidScan
      // check for Android loaders
      for (Index = 0; Index < AndroidEntryDataCount; ++Index) {
        UINTN aIndex, aFound;
        if (FileExists(Volume->RootDir, AndroidEntryData[Index].Path)) {
          aFound = 0;
          for (aIndex = 0; aIndex < ANDX86_FINDLEN; ++aIndex) {
            if ((AndroidEntryData[Index].Find[aIndex] == NULL) || FileExists(Volume->RootDir, AndroidEntryData[Index].Find[aIndex])) ++aFound;
          }
          if (aFound && (aFound == aIndex)) {
            XImage ImageX;
            XStringW IconXSW = XStringW().takeValueFrom(AndroidEntryData[Index].Icon);
            ImageX.LoadXImage(ThemeX.ThemeDir, (L"os_"_XSW + IconXSW.subString(0, IconXSW.indexOf(','))).wc_str());
            AddLoaderEntry(AndroidEntryData[Index].Path, NullXStringArray, XStringW().takeValueFrom(AndroidEntryData[Index].Title), Volume,
                           (ImageX.isEmpty() ? NULL : &ImageX), OSTYPE_LIN, OSFLAG_NODEFAULTARGS);
          }
        }
      }
    }
#endif

    if (gSettings.LinuxScan) {
      // check for linux loaders
      //
      //----- Test common linux name and path like /EFI/ubuntu/grubx64.efi
      REFIT_DIR_ITER  DirIter;
      EFI_FILE_INFO  *DirEntry = NULL;
      DirIterOpen(Volume->RootDir, L"\\EFI", &DirIter);
      while (DirIterNext(&DirIter, 1, L"*", &DirEntry)) {
        if (DirEntry->FileName[0] == '.') {
          //DBG("Skip dot entries: %ls\n", DirEntry->FileName);
          continue;
        }
        XStringW File = SWPrintf("EFI\\%ls\\grubx64.efi", DirEntry->FileName);
        XStringW OSName = SPrintf("%ls", DirEntry->FileName); // this is folder name, for example "ubuntu"
        OSName.lowerAscii(); // lowercase for icon name
        if (FileExists(Volume->RootDir, File.wc_str())) {
          // check if nonstandard icon mapping is needed
          for (Index = 0; Index < LinuxIconMappingCount; ++Index) {
            if (StrCmp(OSName.wc_str(),LinuxIconMapping[Index].DirectoryName) == 0) {
              OSName = XStringW().takeValueFrom(LinuxIconMapping[Index].IconName);
              break;
            }
          }
          DBG("  found entry %ls,linux\n", OSName.wc_str());
          XStringW LoaderTitle = OSName.subString(0,1); // capitalize first letter for title
          LoaderTitle.upperAscii();
          LoaderTitle += OSName.subString(1, OSName.length()) + L" Linux"_XSW;
          XImage ImageX; //will the image be destroyed or rewritten by next image after the cycle end?
          // load from directory, as we don't have linux icons preloaded
          ImageX.LoadXImage(ThemeX.ThemeDir, (L"os_"_XSW + OSName).wc_str());
          AddLoaderEntry(File.wc_str(), NullXStringArray, LoaderTitle, Volume,
                         (ImageX.isEmpty() ? NULL : &ImageX), OSTYPE_LINEFI, OSFLAG_NODEFAULTARGS);
        } //anyway continue search other entries
      }
      DirIterClose(&DirIter);

      for (Index = 0; Index < LinuxEntryDataCount; ++Index) {
        if (FileExists(Volume->RootDir, LinuxEntryData[Index].Path)) {
          XImage ImageX;
          XStringW IconXSW = XStringW().takeValueFrom(LinuxEntryData[Index].Icon);
          ImageX.LoadXImage(ThemeX.ThemeDir, (L"os_"_XSW + IconXSW.subString(0, IconXSW.indexOf(','))).wc_str());
          AddLoaderEntry(LinuxEntryData[Index].Path, NullXStringArray, XStringW().takeValueFrom(LinuxEntryData[Index].Title), Volume,
                         (ImageX.isEmpty() ? NULL : &ImageX), OSTYPE_LINEFI, OSFLAG_NODEFAULTARGS);
        }
      }
      // check for linux kernels
      PartGUID = FindGPTPartitionGuidInDevicePath(Volume->DevicePath);
      if ((PartGUID != NULL) && (Volume->RootDir != NULL)) {
        REFIT_DIR_ITER  Iter;
        EFI_FILE_INFO  *FileInfo = NULL;
        EFI_TIME        PreviousTime;
        CHAR16         *Path = NULL;
//        CHAR16         *Options;
        // Get the partition UUID and make sure it's lower case
        CHAR16          PartUUID[40];
        ZeroMem(&PreviousTime, sizeof(EFI_TIME));
        snwprintf(PartUUID, sizeof(PartUUID), "%s", strguid(PartGUID));
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
                  XStringArray Options = LinuxKernelOptions(Iter.DirHandle, Basename(Path) + LINUX_LOADER_PATH.length(), PartUUID, NullXStringArray);
                  // Add the entry
                  AddLoaderEntry(Path, (Options.isEmpty()) ? LINUX_DEFAULT_OPTIONS : Options, L""_XSW, Volume, NULL, OSTYPE_LINEFI, OSFLAG_NODEFAULTARGS);

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
              XStringArray Options = LinuxKernelOptions(Iter.DirHandle, Basename(Path) + LINUX_LOADER_PATH.length(), PartUUID, NullXStringArray);
              // Add the entry
              AddLoaderEntry(Path, (Options.isEmpty()) ? LINUX_DEFAULT_OPTIONS : Options, L""_XSW, Volume, NULL, OSTYPE_LINEFI, OSFLAG_NODEFAULTARGS);
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
              XStringArray Options = LinuxKernelOptions(Iter.DirHandle, Basename(Path) + LINUX_LOADER_PATH.length(), PartUUID, NullXStringArray);
              // Add the entry
              AddLoaderEntry(Path, (Options.isEmpty()) ? LINUX_DEFAULT_OPTIONS : Options, L""_XSW, Volume, NULL, OSTYPE_LINEFI, OSFLAG_NODEFAULTARGS);
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
              XStringArray Options = LinuxKernelOptions(Iter.DirHandle, XStringW().takeValueFrom(Basename(Path) + LINUX_LOADER_PATH.length()).wc_str(), PartUUID, NullXStringArray);
              // Add the entry
              AddLoaderEntry(Path, (Options.isEmpty()) ? LINUX_DEFAULT_OPTIONS : Options, L""_XSW, Volume, NULL, OSTYPE_LINEFI, OSFLAG_NODEFAULTARGS);
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
              XStringArray Options = LinuxKernelOptions(Iter.DirHandle, XStringW().takeValueFrom(Basename(Path) + LINUX_LOADER_PATH.length()).wc_str(), PartUUID, NullXStringArray);
              // Add the entry
              AddLoaderEntry(Path, (Options.isEmpty()) ? LINUX_DEFAULT_OPTIONS : Options, L""_XSW, Volume, NULL, OSTYPE_LINEFI, OSFLAG_NODEFAULTARGS);

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
              XStringArray Options = LinuxKernelOptions(Iter.DirHandle, Basename(Path) + LINUX_LOADER_PATH.length(), PartUUID, NullXStringArray);
              AddLoaderEntry(Path, (Options.isEmpty()) ? LINUX_DEFAULT_OPTIONS : Options, L""_XSW, Volume, NULL, OSTYPE_LINEFI, OSFLAG_NODEFAULTARGS);
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
                    XStringArray Options = LinuxKernelOptions(Iter.DirHandle, Basename(Path) + LINUX_LOADER_PATH.length(), PartUUID, NullXStringArray);
                    // Add the entry
                    AddLoaderEntry(Path, (Options.isEmpty()) ? LINUX_DEFAULT_OPTIONS : Options, L""_XSW, Volume, NULL, OSTYPE_LINEFI, OSFLAG_NODEFAULTARGS);
                    FreePool(Path);
                  }
                }
                // free the file info
                FreePool(FileInfo);
                FileInfo = NULL;
              }
            }
            break;

          case KERNEL_SCAN_NONE:
          default:
            // No kernel scan
            break;
        }
        //close the directory
        DirIterClose(&Iter);
      }
    } //if linux scan
    //     DBG("search for  optical UEFI\n");
    if (Volume->DiskKind == DISK_KIND_OPTICAL) {
      AddLoaderEntry(BOOT_LOADER_PATH, NullXStringArray, L"UEFI optical"_XSW, Volume, NULL, OSTYPE_OTHER, 0);
    }
    //     DBG("search for internal UEFI\n");
    if (Volume->DiskKind == DISK_KIND_INTERNAL) {
      AddLoaderEntry(BOOT_LOADER_PATH, NullXStringArray, L"UEFI internal"_XSW, Volume, NULL, OSTYPE_OTHER, OSFLAG_HIDDEN);
    }
    //    DBG("search for external UEFI\n");
    if (Volume->DiskKind == DISK_KIND_EXTERNAL) {
      AddLoaderEntry(BOOT_LOADER_PATH, NullXStringArray, L"UEFI external"_XSW, Volume, NULL, OSTYPE_OTHER, OSFLAG_HIDDEN);
    }
  }

}

STATIC VOID AddCustomEntry(IN UINTN                CustomIndex,
                           IN CONST CHAR16         *CustomPath,
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
//	  DBG("Custom %lsentry %llu skipped because it didn't have a ", IsSubEntry ? L"sub " : L"", CustomIndex);
//    if (Custom->Type == 0) {
//      DBG("Type.\n");
//    } else {
//      DBG("Path.\n");
//    }
    return;
  }

  if (OSFLAG_ISSET(Custom->Flags, OSFLAG_DISABLED)) {
//	  DBG("Custom %lsentry %llu skipped because it is disabled.\n", IsSubEntry ? L"sub " : L"", CustomIndex);
    return;
  }
  if (!gSettings.ShowHiddenEntries && OSFLAG_ISSET(Custom->Flags, OSFLAG_HIDDEN)) {
//	  DBG("Custom %lsentry %llu skipped because it is hidden.\n", IsSubEntry ? L"sub " : L"", CustomIndex);
    return;
  }
#if 0  //if someone want to debug this
  DBG("Custom %lsentry %llu ", IsSubEntry ? L"sub " : L"", CustomIndex);
  //  if (Custom->Title) {
  DBG("Title:\"%ls\" ", Custom->Title.wc_str());
  //  }
  //  if (Custom->FullTitle) {
  DBG("FullTitle:\"%ls\" ", Custom->FullTitle.wc_str());
  //  }
  if (CustomPath) {
    DBG("Path:\"%ls\" ", CustomPath);
  }
  if (Custom->Options != NULL) {
    DBG("Options:\"%ls\" ", Custom->Options);
  }
  DBG("Type:%d Flags:0x%hhX matching ", Custom->Type, Custom->Flags);
  if (Custom->Volume) {
    DBG("Volume:\"%ls\"\n", Custom->Volume);
  } else {
    DBG("all volumes\n");
  }
#endif

  for (VolumeIndex = 0; VolumeIndex < Volumes.size(); ++VolumeIndex) {
    CUSTOM_LOADER_ENTRY *CustomSubEntry;
    LOADER_ENTRY        *Entry = NULL;
    XImage Image; // = new XImage;
    XImage DriveImage;

    EFI_GUID            *Guid = NULL;
    UINT64               VolumeSize;

    Volume = &Volumes[VolumeIndex];
    if ((Volume == NULL) || (Volume->RootDir == NULL)) {
      continue;
    }
    if (Volume->VolName == NULL) {
      Volume->VolName = L"Unknown";
    }

    DBG("    Checking volume \"%ls\" (%ls) ... ", Volume->VolName, Volume->DevicePathString);

    // skip volume if its kind is configured as disabled
/*    if ((Volume->DiskKind == DISK_KIND_OPTICAL  && (GlobalConfig.DisableFlags & VOLTYPE_OPTICAL))  ||
        (Volume->DiskKind == DISK_KIND_EXTERNAL && (GlobalConfig.DisableFlags & VOLTYPE_EXTERNAL)) ||
        (Volume->DiskKind == DISK_KIND_INTERNAL && (GlobalConfig.DisableFlags & VOLTYPE_INTERNAL)) ||
        (Volume->DiskKind == DISK_KIND_FIREWIRE && (GlobalConfig.DisableFlags & VOLTYPE_FIREWIRE)))
    {
      DBG("skipped because media is disabled\n");
      continue;
    }
 */
    if (((1ull<<Volume->DiskKind) & GlobalConfig.DisableFlags) != 0) {
      DBG("skipped because media is disabled\n");
      continue;
    }

/*    if (Custom->VolumeType != 0) {
      if ((Volume->DiskKind == DISK_KIND_OPTICAL  && ((Custom->VolumeType & VOLTYPE_OPTICAL) == 0))  ||
          (Volume->DiskKind == DISK_KIND_EXTERNAL && ((Custom->VolumeType & VOLTYPE_EXTERNAL) == 0)) ||
          (Volume->DiskKind == DISK_KIND_INTERNAL && ((Custom->VolumeType & VOLTYPE_INTERNAL) == 0)) ||
          (Volume->DiskKind == DISK_KIND_FIREWIRE && ((Custom->VolumeType & VOLTYPE_FIREWIRE) == 0))) {
        DBG("skipped because media is ignored\n");
        continue;
      }
    }
 */
    if (Custom->VolumeType != 0 && ((1<<Volume->DiskKind) & Custom->VolumeType) == 0) {
      DBG("skipped because media is ignored\n");
      continue;
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
    /*
    if (StriCmp(CustomPath, MACOSX_LOADER_PATH) == 0 && FileExists(Volume->RootDir, L"\\.IAPhysicalMedia")) {
      DBG("skipped standard macOS path because volume is 2nd stage Install Media\n");
      continue;
    } */
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
      snwprintf(PartUUID, sizeof(PartUUID), "%s", strguid(Guid));
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
          //      CHAR16 *NewPath = PoolPrint(L"%s\\%s", LINUX_BOOT_PATH, FileInfo->FileName);
                XStringW NewPath = LINUX_BOOT_PATH"\\"_XSW + FileInfo->FileName;
                if ((CustomPath == NULL) || (StrCmp(CustomPath, NewPath.wc_str()) > 0)) {
                  if (CustomPath != NULL) {
                    FreePool(CustomPath);
                  }
                  CustomPath = EfiStrDuplicate(NewPath.wc_str());
                } /*else {
                  FreePool(NewPath);
                } */
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
    if (Image.isEmpty() && Custom->ImagePath) {
      Image.LoadXImage(ThemeX.ThemeDir, Custom->ImagePath);
      if (Image.isEmpty()) {
        Image.LoadXImage(ThemeX.ThemeDir, L"os_"_XSW + Custom->ImagePath);
        if (Image.isEmpty()) {
          Image.LoadXImage(SelfDir, Custom->ImagePath);
          if (Image.isEmpty()) {
            Image.LoadXImage(SelfRootDir, Custom->ImagePath);
            if (Image.isEmpty()) {
              Image.LoadXImage(Volume->RootDir, Custom->ImagePath);
            }
          }
        }
      }
    }

    // Change to custom drive image if needed
    DriveImage = Custom->DriveImage;
    if (DriveImage.isEmpty() && Custom->DriveImagePath) {
      DriveImage.LoadXImage(ThemeX.ThemeDir, Custom->DriveImagePath);
      if (DriveImage.isEmpty()) {
        DriveImage.LoadXImage(SelfDir, Custom->ImagePath);
        if (DriveImage.isEmpty()) {
          DriveImage.LoadXImage(SelfRootDir, Custom->ImagePath);
          if (DriveImage.isEmpty()) {
            DriveImage.LoadXImage(Volume->RootDir, Custom->ImagePath);
          }
        }
      }
    }

    do
    {
      // Search for linux kernels
      XStringArray CustomOptions = Custom->LoadOptions;
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
        CustomOptions = LinuxKernelOptions(Iter->DirHandle, Basename(CustomPath) + LINUX_LOADER_PATH.length(), PartUUID, Custom->LoadOptions);
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
                                ((1ull<<Volume->DiskKind) & Custom->VolumeType) != 0));
                         /*       (Volume->DiskKind == DISK_KIND_OPTICAL && (Custom->VolumeType & VOLTYPE_OPTICAL)) ||
                                (Volume->DiskKind == DISK_KIND_EXTERNAL && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
                                (Volume->DiskKind == DISK_KIND_INTERNAL && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
                                (Volume->DiskKind == DISK_KIND_FIREWIRE && (Custom->VolumeType & VOLTYPE_FIREWIRE)))); */
              }
            } else if ((StrStr(Volume->DevicePathString, Custom->Volume) == NULL) &&
                       ((Volume->VolName == NULL) || (StrStr(Volume->VolName, Custom->Volume) == NULL))) {
              if (Custom->Volume == NULL) {
                // More precise volume match
                if (Custom->Path != Ptr->Path) {
                  // Better path match
                  BetterMatch = ((Ptr->Path != NULL) && (StrCmp(CustomPath, Ptr->Path) == 0) &&
                                 ((Custom->VolumeType == Ptr->VolumeType) ||
                                  ((1ull<<Volume->DiskKind) & Custom->VolumeType) != 0));
                  /*       (Volume->DiskKind == DISK_KIND_OPTICAL && (Custom->VolumeType & VOLTYPE_OPTICAL)) ||
                   (Volume->DiskKind == DISK_KIND_EXTERNAL && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
                   (Volume->DiskKind == DISK_KIND_INTERNAL && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
                   (Volume->DiskKind == DISK_KIND_FIREWIRE && (Custom->VolumeType & VOLTYPE_FIREWIRE)))); */
                } else if (Custom->VolumeType != Ptr->VolumeType) {
                  // More precise volume type match
                  BetterMatch = ((Custom->VolumeType == 0) &&
                                 ((1ull<<Volume->DiskKind) & Custom->VolumeType) != 0);
                  /*       (Volume->DiskKind == DISK_KIND_OPTICAL && (Custom->VolumeType & VOLTYPE_OPTICAL)) ||
                   (Volume->DiskKind == DISK_KIND_EXTERNAL && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
                   (Volume->DiskKind == DISK_KIND_INTERNAL && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
                   (Volume->DiskKind == DISK_KIND_FIREWIRE && (Custom->VolumeType & VOLTYPE_FIREWIRE)))); */
                } else {
                  // Better match
                  BetterMatch = TRUE;
                }
              // Duplicate volume match
              } else if (Custom->Path != Ptr->Path) {
                // Better path match
                BetterMatch = ((Ptr->Path != NULL) && (StrCmp(CustomPath, Ptr->Path) == 0) &&
                               ((Custom->VolumeType == Ptr->VolumeType) ||
                                ((1ull<<Volume->DiskKind) & Custom->VolumeType) != 0));
                /*       (Volume->DiskKind == DISK_KIND_OPTICAL && (Custom->VolumeType & VOLTYPE_OPTICAL)) ||
                 (Volume->DiskKind == DISK_KIND_EXTERNAL && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
                 (Volume->DiskKind == DISK_KIND_INTERNAL && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
                 (Volume->DiskKind == DISK_KIND_FIREWIRE && (Custom->VolumeType & VOLTYPE_FIREWIRE)))); */
              // Duplicate path match
              } else if (Custom->VolumeType != Ptr->VolumeType) {
                // More precise volume type match
                BetterMatch = ((Custom->VolumeType == 0) &&
                               ((1ull<<Volume->DiskKind) & Custom->VolumeType) != 0);
                /*       (Volume->DiskKind == DISK_KIND_OPTICAL && (Custom->VolumeType & VOLTYPE_OPTICAL)) ||
                 (Volume->DiskKind == DISK_KIND_EXTERNAL && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
                 (Volume->DiskKind == DISK_KIND_INTERNAL && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
                 (Volume->DiskKind == DISK_KIND_FIREWIRE && (Custom->VolumeType & VOLTYPE_FIREWIRE)))); */
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
                             ((1ull<<Volume->DiskKind) & Custom->VolumeType) != 0);
              /*       (Volume->DiskKind == DISK_KIND_OPTICAL && (Custom->VolumeType & VOLTYPE_OPTICAL)) ||
               (Volume->DiskKind == DISK_KIND_EXTERNAL && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
               (Volume->DiskKind == DISK_KIND_INTERNAL && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
               (Volume->DiskKind == DISK_KIND_FIREWIRE && (Custom->VolumeType & VOLTYPE_FIREWIRE)))); */
            } else if (StrCmp(CustomPath, Ptr->Path) == 0) {
              if (Custom->Path == NULL) {
                // More precise path and volume type match
                BetterMatch = ((Custom->VolumeType == Ptr->VolumeType) ||
                               ((1ull<<Volume->DiskKind) & Custom->VolumeType) != 0);
                /*       (Volume->DiskKind == DISK_KIND_OPTICAL && (Custom->VolumeType & VOLTYPE_OPTICAL)) ||
                 (Volume->DiskKind == DISK_KIND_EXTERNAL && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
                 (Volume->DiskKind == DISK_KIND_INTERNAL && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
                 (Volume->DiskKind == DISK_KIND_FIREWIRE && (Custom->VolumeType & VOLTYPE_FIREWIRE)))); */
              } else if (Custom->VolumeType != Ptr->VolumeType) {
                // More precise volume type match
                BetterMatch = ((Custom->VolumeType == 0) &&
                               ((1ull<<Volume->DiskKind) & Custom->VolumeType) != 0);
                /*       (Volume->DiskKind == DISK_KIND_OPTICAL && (Custom->VolumeType & VOLTYPE_OPTICAL)) ||
                 (Volume->DiskKind == DISK_KIND_EXTERNAL && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
                 (Volume->DiskKind == DISK_KIND_INTERNAL && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
                 (Volume->DiskKind == DISK_KIND_FIREWIRE && (Custom->VolumeType & VOLTYPE_FIREWIRE)))); */
              } else {
                // Duplicate entry
                BetterMatch = (i <= CustomIndex);
              }
            }
          // Duplicate path match
          } else if (Custom->VolumeType != Ptr->VolumeType) {
            // More precise volume type match
            BetterMatch = ((Custom->VolumeType == 0) &&
                           ((1ull<<Volume->DiskKind) & Custom->VolumeType) != 0);
            /*       (Volume->DiskKind == DISK_KIND_OPTICAL && (Custom->VolumeType & VOLTYPE_OPTICAL)) ||
             (Volume->DiskKind == DISK_KIND_EXTERNAL && (Custom->VolumeType & VOLTYPE_EXTERNAL)) ||
             (Volume->DiskKind == DISK_KIND_INTERNAL && (Custom->VolumeType & VOLTYPE_INTERNAL)) ||
             (Volume->DiskKind == DISK_KIND_FIREWIRE && (Custom->VolumeType & VOLTYPE_FIREWIRE)))); */
          } else {
            // Duplicate entry
            BetterMatch = (i <= CustomIndex);
          }
          if (BetterMatch) {
            break;
          }
        }
        if (BetterMatch) {
			DBG("skipped because custom entry %llu is a better match and will produce a duplicate entry\n", i);
          continue;
        }
      }
      DBG("match!\n");
      // Create an entry for this volume
      Entry = CreateLoaderEntry(CustomPath, CustomOptions, Custom->FullTitle, Custom->Title, Volume,
                                (Image.isEmpty() ? NULL : &Image), (DriveImage.isEmpty() ? NULL : &DriveImage),            
                                Custom->Type, Custom->Flags, Custom->Hotkey, Custom->BootBgColor, Custom->CustomBoot, &Custom->CustomLogo, 
                                /*(KERNEL_AND_KEXT_PATCHES *)(((UINTN)Custom) + OFFSET_OF(CUSTOM_LOADER_ENTRY, KernelAndKextPatches))*/ NULL, TRUE);

      if (Entry != NULL) {
        DBG("Custom settings: %ls.plist will %s be applied\n",
            Custom->Settings, Custom->CommonSettings?"not":"");
        if (!Custom->CommonSettings) {
          Entry->Settings = Custom->Settings;
        }
        if (OSFLAG_ISUNSET(Custom->Flags, OSFLAG_NODEFAULTMENU)) {
          AddDefaultMenu(Entry);
        } else if (Custom->SubEntries != NULL) {
          UINTN CustomSubIndex = 0;
          // Add subscreen
//          REFIT_MENU_SCREEN *SubScreen = (__typeof__(SubScreen))AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
          REFIT_MENU_SCREEN *SubScreen = new REFIT_MENU_SCREEN;
          if (SubScreen) {
            SubScreen->Title.SWPrintf("Boot Options for %ls on %ls", (Custom->Title.notEmpty()) ? Custom->Title.wc_str() : CustomPath, Entry->VolName);
            SubScreen->TitleImage = Entry->Image;
            SubScreen->ID = Custom->Type + 20;
            SubScreen->GetAnime();
            VolumeSize = RShiftU64(MultU64x32(Volume->BlockIO->Media->LastBlock, Volume->BlockIO->Media->BlockSize), 20);
            SubScreen->AddMenuInfoLine_f("Volume size: %lldMb", VolumeSize);
            SubScreen->AddMenuInfoLine_f("%ls", FileDevicePathToStr(Entry->DevicePath));
            if (Guid) {
              SubScreen->AddMenuInfoLine_f("UUID: %s", strguid(Guid));
            }
            SubScreen->AddMenuInfoLine_f("Options: %s", Entry->LoadOptions.ConcatAll(" "_XS).c_str());
            DBG("Create sub entries\n");
            for (CustomSubEntry = Custom->SubEntries; CustomSubEntry; CustomSubEntry = CustomSubEntry->Next) {
              if (!CustomSubEntry->Settings) {
                CustomSubEntry->Settings = Custom->Settings;
              }
              AddCustomEntry(CustomSubIndex++, (CustomSubEntry->Path != NULL) ? CustomSubEntry->Path : CustomPath, CustomSubEntry, SubScreen);
            }
            SubScreen->AddMenuEntry(&MenuEntryReturn, true);
            Entry->SubScreen = SubScreen;
          }
        }
        if (IsSubEntry)
          SubMenu->AddMenuEntry(Entry, true);
        else
          MainMenu.AddMenuEntry(Entry, true);
      }
      // cleanup custom
      if (FindCustomPath) {
        FreePool(CustomPath);
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
  
  if (!gSettings.CustomEntries) {
    return;
  }

  //DBG("Custom entries start\n");
  DbgHeader("AddCustomEntries");
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
        UINTN Index;
#if defined(ANDX86)
        Index = 0;
        while (Index < AndroidEntryDataCount) {
          AddCustomEntry(i, AndroidEntryData[Index++].Path, Custom, NULL);
        }
#endif
        Index = 0;
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
  //DBG("Custom entries finish\n");
}
