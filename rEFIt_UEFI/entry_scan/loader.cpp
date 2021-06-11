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

#include <Platform.h>
#include "../refit/lib.h"
#include "../libeg/libeg.h"
#include "loader.h"
#include "../cpp_foundation/XString.h"
#include "entry_scan.h"
#include "../Platform/Settings.h"
#include "../Platform/Hibernate.h"
#include "../refit/screen.h"
#include "../refit/menu.h"
#include "../Platform/Nvram.h"
#include "../Platform/APFS.h"
#include "../Platform/guid.h"
#include "../refit/lib.h"
#include "../gui/REFIT_MENU_SCREEN.h"
#include "../gui/REFIT_MAINMENU_SCREEN.h"
#include "../Settings/Self.h"
#include "../include/OSTypes.h"
#include "../Platform/BootOptions.h"
#include "../Platform/Volumes.h"
#include "../include/OSFlags.h"
#include "../libeg/XTheme.h"

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

const XStringW MACOSX_LOADER_PATH = L"\\System\\Library\\CoreServices\\boot.efi"_XSW;

const XStringW LINUX_ISSUE_PATH = L"\\etc\\issue"_XSW;
#define LINUX_BOOT_PATH L"\\boot"
#define LINUX_BOOT_ALT_PATH L"\\boot"
const XString8 LINUX_LOADER_PATH = "vmlinuz"_XS8;
const XStringW LINUX_FULL_LOADER_PATH = SWPrintf("%ls\\%s", LINUX_BOOT_PATH, LINUX_LOADER_PATH.c_str());
#define LINUX_LOADER_SEARCH_PATH L"vmlinuz*"
const XString8Array LINUX_DEFAULT_OPTIONS = Split<XString8Array>("ro add_efi_memmap quiet splash vt.handoff=7", " ");

#if defined(MDE_CPU_X64)
#define BOOT_LOADER_PATH L"\\EFI\\BOOT\\BOOTX64.efi"_XSW
#else
#define BOOT_LOADER_PATH L"\\EFI\\BOOT\\BOOTIA32.efi"_XSW
#endif


//extern LOADER_ENTRY *SubMenuKextInjectMgmt(LOADER_ENTRY *Entry);

// Linux loader path data
typedef struct LINUX_PATH_DATA
{
   CONST XStringW Path;
   CONST XStringW Title;
   CONST XStringW Icon;
   CONST XString8 Issue;
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
STATIC CONST UINTN LinuxIconMappingCount = (sizeof(LinuxIconMapping) / sizeof(LinuxIconMapping[0]));

STATIC LINUX_PATH_DATA LinuxEntryData[] = {
#if defined(MDE_CPU_X64)
  //comment out all common names
//  { L"\\EFI\\grub\\grubx64.efi", L"Grub EFI boot menu", L"grub,linux" },
//  { L"\\EFI\\Gentoo\\grubx64.efi", L"Gentoo EFI boot menu", L"gentoo,linux", "Gentoo" },
  { L"\\EFI\\Gentoo\\kernelx64.efi"_XSW, L"Gentoo EFI kernel"_XSW, L"gentoo,linux"_XSW, ""_XS8 },
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
  { L"\\EFI\\goofiboot\\goofibootx64.efi"_XSW, L"Solus EFI boot menu"_XSW, L"solus,linux"_XSW, "Solus"_XS8 },
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
  { L"\\EFI\\SuSe\\elilo.efi"_XSW, L"OpenSuse EFI boot menu"_XSW, L"suse,linux"_XSW, ""_XS8 },
};
STATIC CONST UINTN LinuxEntryDataCount = (sizeof(LinuxEntryData) / sizeof(LinuxEntryData[0]));

#if defined(ANDX86)
#if !defined(MDE_CPU_X64)
#undef ANDX86
#else
// ANDX86 loader path data
#define ANDX86_FINDLEN 3
typedef struct ANDX86_PATH_DATA
{
   CONST XStringW Path;
   CONST XStringW Title;
   CONST XStringW Icon;
   CONST XStringW Find[ANDX86_FINDLEN];
} ANDX86_PATH_DATA;

STATIC ANDX86_PATH_DATA AndroidEntryData[] = {
  //{ L"\\EFI\\boot\\grubx64.efi", L"Grub", L"grub,linux" },
  //{ L"\\EFI\\boot\\bootx64.efi", L"Grub", L"grub,linux" },
  { L"\\EFI\\remixos\\grubx64.efi"_XSW,         L"Remix"_XSW,     L"remix,grub,linux"_XSW,   { L"\\isolinux\\isolinux.bin"_XSW, L"\\initrd.img"_XSW, L"\\kernel"_XSW } },
  { L"\\EFI\\PhoenixOS\\boot\\grubx64.efi"_XSW, L"PhoenixOS"_XSW, L"phoenix,grub,linux"_XSW, { L"\\EFI\\PhoenixOS\\boot\\efi.img"_XSW, L"\\EFI\\PhoenixOS\\initrd.img"_XSW, L"\\EFI\\PhoenixOS\\kernel"_XSW } },
  { L"\\EFI\\boot\\grubx64.efi"_XSW,            L"Phoenix"_XSW,   L"phoenix,grub,linux"_XSW, { L"\\phoenix\\kernel"_XSW, L"\\phoenix\\initrd.img"_XSW, L"\\phoenix\\ramdisk.img"_XSW } },
  { L"\\EFI\\boot\\bootx64.efi"_XSW,            L"Chrome"_XSW,    L"chrome,grub,linux"_XSW,  { L"\\syslinux\\vmlinuz.A"_XSW, L"\\syslinux\\vmlinuz.B"_XSW, L"\\syslinux\\ldlinux.sys"_XSW} },

};
STATIC CONST UINTN AndroidEntryDataCount = (sizeof(AndroidEntryData) / sizeof(ANDX86_PATH_DATA));
#endif
#endif

CONST XStringW PaperBoot   = L"\\com.apple.boot.P\\boot.efi"_XSW;
CONST XStringW RockBoot    = L"\\com.apple.boot.R\\boot.efi"_XSW;
CONST XStringW ScissorBoot = L"\\com.apple.boot.S\\boot.efi"_XSW;

// OS X installer paths
CONST XStringW OSXInstallerPaths[] = {
  L"\\.IABootFiles\\boot.efi"_XSW, // 10.9 - 10.13.3
  L"\\Mac OS X Install Data\\boot.efi"_XSW, // 10.7
  L"\\OS X Install Data\\boot.efi"_XSW, // 10.8 - 10.11
  L"\\macOS Install Data\\boot.efi"_XSW, // 10.12 - 10.12.3
  L"\\macOS Install Data\\Locked Files\\Boot Files\\boot.efi"_XSW, // 10.12.4-10.15
  L"\\macOS Install Data\\Locked Files\\boot.efi"_XSW // 10.16+
};

STATIC CONST UINTN OSXInstallerPathsCount = (sizeof(OSXInstallerPaths) / sizeof(OSXInstallerPaths[0]));

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

UINT8 GetOSTypeFromPath(IN CONST XStringW& Path)
{
  if (Path.isEmpty()) {
    return OSTYPE_OTHER;
  }
  if ( Path.isEqualIC(MACOSX_LOADER_PATH)) {
    return OSTYPE_OSX;
  } else if ( Path.isEqualIC(OSXInstallerPaths[0]) ||
             ( Path.isEqualIC(OSXInstallerPaths[1])) ||
             ( Path.isEqualIC(OSXInstallerPaths[2])) ||
             ( Path.isEqualIC(OSXInstallerPaths[3])) ||
             ( Path.isEqualIC(OSXInstallerPaths[4])) ||
             ( Path.isEqualIC(RockBoot)) || ( Path.isEqualIC(PaperBoot)) || ( Path.isEqualIC(ScissorBoot)) ||
             (! Path.isEqualIC(L"\\.IABootFiles\\boot.efi") &&  Path.isEqualIC(L"\\.IAPhysicalMedia") &&  Path.isEqualIC(MACOSX_LOADER_PATH))
             ) {
    return OSTYPE_OSX_INSTALLER;
  } else if ( Path.isEqualIC(L"\\com.apple.recovery.boot\\boot.efi")) {
    return OSTYPE_RECOVERY;
  } else if (( Path.isEqualIC(L"\\EFI\\Microsoft\\Boot\\bootmgfw-orig.efi")) || //test first as orig
             ( Path.isEqualIC(L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi")) ||      //it can be Clover
    //         ( Path.isEqualIC(L"\\bootmgr.efi")) || //never worked, just extra icon in menu
             ( Path.isEqualIC(L"\\EFI\\MICROSOFT\\BOOT\\cdboot.efi"))) {
    return OSTYPE_WINEFI;
  } else if (LINUX_FULL_LOADER_PATH.isEqualIC(Path)) {
    return OSTYPE_LINEFI;
  } else if ( Path.containsIC("grubx64.efi") ) {
    return OSTYPE_LINEFI;
  } else {
    UINTN Index;
#if defined(ANDX86)
    Index = 0;
    while (Index < AndroidEntryDataCount) {
      if ( Path.isEqualIC(AndroidEntryData[Index].Path) ) {
        return OSTYPE_LIN;
      }
      ++Index;
    }
#endif
    Index = 0;
    while (Index < LinuxEntryDataCount) {
      if ( Path.isEqualIC(LinuxEntryData[Index].Path) ) {
        return OSTYPE_LIN;
      }
      ++Index;
    }
  }
  return OSTYPE_OTHER;
}

static const XStringW linux = L"linux"_XSW;

STATIC CONST XStringW& LinuxIconNameFromPath(IN CONST XStringW& Path,
                       const EFI_FILE_PROTOCOL *RootDir)
{
  UINTN Index;
#if defined(ANDX86)
  Index = 0;
  while (Index < AndroidEntryDataCount) {
    if ( Path.isEqualIC(AndroidEntryData[Index].Path) ) {
      return AndroidEntryData[Index].Icon;
    }
    ++Index;
  }
#endif
  
  //check not common names
  Index = 0;
  while (Index < LinuxEntryDataCount) {
    if ( Path.isEqualIC(LinuxEntryData[Index].Path) ) {
      return LinuxEntryData[Index].Icon;
    }
    ++Index;
  }
  
  // Try to open the linux issue
  if ((RootDir != NULL) && LINUX_FULL_LOADER_PATH.isEqualIC(Path)) {
    CHAR8 *Issue = NULL;
    UINTN  IssueLen = 0;
    if (!EFI_ERROR(egLoadFile(RootDir, LINUX_ISSUE_PATH.wc_str(), (UINT8 **)&Issue, &IssueLen)) && (Issue != NULL)) {
      if (IssueLen > 0) {
        for (Index = 0; Index < LinuxEntryDataCount; ++Index) {
          if ( LinuxEntryData[Index].Issue.notEmpty()  &&  AsciiStrStr(Issue, LinuxEntryData[Index].Issue.c_str()) != NULL ) {
            FreePool(Issue);
            return LinuxEntryData[Index].Icon;
          }
        }
      }
      FreePool(Issue);
    }
  }
  return linux;
}

STATIC CONST XString8 LinuxInitImagePath[] = {
   "initrd%s"_XS8,
   "initrd.img%s"_XS8,
   "initrd%s.img"_XS8,
   "initramfs%s"_XS8,
   "initramfs.img%s"_XS8,
   "initramfs%s.img"_XS8,
};
STATIC CONST UINTN LinuxInitImagePathCount = (sizeof(LinuxInitImagePath) / sizeof(LinuxInitImagePath[0]));

STATIC XString8Array LinuxKernelOptions(const EFI_FILE_PROTOCOL *Dir,
                                  IN CONST CHAR16            *Version,
                                  IN CONST CHAR16            *PartUUID,
                                  IN CONST XString8Array&           Options OPTIONAL)
{
  UINTN Index = 0;
  if ((Dir == NULL) || (PartUUID == NULL)) {
    return Options;
  }
  while (Index < LinuxInitImagePathCount) {
    XStringW InitRd = SWPrintf(LinuxInitImagePath[Index++].c_str(), (Version == NULL) ? L"" : Version);
    if (InitRd.notEmpty()) {
      if (FileExists(Dir, InitRd)) {
        XString8Array CustomOptions;
        CustomOptions.Add(S8Printf("root=/dev/disk/by-partuuid/%ls", PartUUID));
        CustomOptions.Add(S8Printf("initrd=%ls\\%ls", LINUX_BOOT_ALT_PATH, InitRd.wc_str()));
        CustomOptions.import(LINUX_DEFAULT_OPTIONS);
        CustomOptions.import(Options);
        return CustomOptions;
      }
    }
  }
  XString8Array CustomOptions;
  CustomOptions.Add(S8Printf("root=/dev/disk/by-partuuid/%ls", PartUUID));
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
  UINTN   fileLen = 0;
  if(FileExists(Entry->Volume->RootDir, targetNameFile)) {
    Status = egLoadFile(Entry->Volume->RootDir, targetNameFile, (UINT8 **)&fileBuffer, &fileLen);
    if(!EFI_ERROR(Status)) {
//      CHAR16  *tmpName;
      //Create null terminated string
//      targetString = (CHAR8*) A_llocateZeroPool(fileLen+1);
//      CopyMem( (void*)targetString, (void*)fileBuffer, fileLen);
//      DBG("found disk_label with contents:%s\n", targetString);

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

//      //Convert to Unicode
//      tmpName = (CHAR16*)A_llocateZeroPool((fileLen+1)*2);
//      AsciiStrToUnicodeStrS(targetString, tmpName, fileLen);

      Entry->DisplayedVolName.strncpy(fileBuffer, fileLen);
      DBG("Created name:%ls\n", Entry->DisplayedVolName.wc_str());

      FreePool(fileBuffer);
    }
  }
  return Status;
}


MacOsVersion GetOSVersion(int LoaderType, const XStringW& APFSTargetUUID, const REFIT_VOLUME* Volume, XString8* BuildVersionPtr)
{
  XString8   OSVersion;
  XString8   BuildVersion;
  EFI_STATUS Status      = EFI_NOT_FOUND;
  CHAR8*     PlistBuffer = NULL;
  UINTN      PlistLen;
  TagDict*     Dict        = NULL;
  const TagDict*     DictPointer = NULL;
  const TagStruct*     Prop        = NULL;

  if ( !Volume ) {
    return NullXString8;
  }

  if (OSTYPE_IS_OSX(LoaderType))
  {
    XString8 uuidPrefix;
    if ( APFSTargetUUID.notEmpty() ) uuidPrefix = S8Printf("\\%ls", APFSTargetUUID.wc_str());

    XStringW plist = SWPrintf("%s\\System\\Library\\CoreServices\\SystemVersion.plist", uuidPrefix.c_str());
    if ( !FileExists(Volume->RootDir, plist) ) {
      plist = SWPrintf("%s\\System\\Library\\CoreServices\\ServerVersion.plist", uuidPrefix.c_str());
      if ( !FileExists(Volume->RootDir, plist) ) {
        plist.setEmpty();
      }
    }

    if ( plist.notEmpty() ) { // found macOS System
      Status = egLoadFile(Volume->RootDir, plist.wc_str(), (UINT8 **)&PlistBuffer, &PlistLen);
      if (!EFI_ERROR(Status) && PlistBuffer != NULL && ParseXML(PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
        Prop = Dict->propertyForKey("ProductVersion");
        if ( Prop != NULL ) {
          if ( !Prop->isString() ) {
            MsgLog("ATTENTION : property not string in ProductVersion\n");
          }else{
            if( Prop->getString()->stringValue().notEmpty() ) {
              OSVersion = Prop->getString()->stringValue();
            }
          }
        }
        Prop = Dict->propertyForKey("ProductBuildVersion");
        if ( Prop != NULL ) {
          if ( !Prop->isString() ) {
            MsgLog("ATTENTION : property not string in ProductBuildVersion\n");
          }else{
            if( Prop->getString()->stringValue().notEmpty() ) {
              BuildVersion = Prop->getString()->stringValue();
            }
          }
        }
        Dict->FreeTag();
      }
    }
  }

  if (OSTYPE_IS_OSX_INSTALLER (LoaderType)) {
    // Detect exact version for 2nd stage Installer (thanks to dmazar for this idea)
    // This should work for most installer cases. Rest cases will be read from boot.efi before booting.
    // Reworked by Sherlocks. 2018.04.12

    // 1st stage - 1
    // Check for plist - createinstallmedia/BaseSystem/InstallDVD/InstallESD

    XStringW InstallerPlist;

    if ( APFSTargetUUID.notEmpty() ) {
      InstallerPlist = SWPrintf("%ls\\System\\Library\\CoreServices\\SystemVersion.plist", APFSTargetUUID.wc_str());
      if ( !FileExists(Volume->RootDir, InstallerPlist) ) InstallerPlist.setEmpty();
    }

    if ( InstallerPlist.isEmpty() ) {
      InstallerPlist = SWPrintf("\\.IABootFilesSystemVersion.plist"); // 10.9 - 10.13.3
      if (!FileExists(Volume->RootDir, InstallerPlist) && FileExists (Volume->RootDir, L"\\System\\Library\\CoreServices\\boot.efi") &&
          ((FileExists(Volume->RootDir, L"\\BaseSystem.dmg") && FileExists (Volume->RootDir, L"\\mach_kernel")) || // 10.7/10.8
           FileExists(Volume->RootDir, L"\\System\\Installation\\CDIS\\Mac OS X Installer.app") || // 10.6/10.7
           FileExists(Volume->RootDir, L"\\System\\Installation\\CDIS\\OS X Installer.app") || // 10.8 - 10.11
           FileExists(Volume->RootDir, L"\\System\\Installation\\CDIS\\macOS Installer.app") || // 10.12+
           FileExists(Volume->RootDir, L"\\.IAPhysicalMedia"))) { // 10.13.4+
        InstallerPlist = SWPrintf("\\System\\Library\\CoreServices\\SystemVersion.plist");
      }
    }
    if (FileExists (Volume->RootDir, InstallerPlist)) {
      Status = egLoadFile(Volume->RootDir, InstallerPlist.wc_str(), (UINT8 **)&PlistBuffer, &PlistLen);
      if (!EFI_ERROR(Status) && PlistBuffer != NULL && ParseXML(PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
        Prop = Dict->propertyForKey("ProductVersion");
        if ( Prop != NULL ) {
          if ( !Prop->isString() ) {
            MsgLog("ATTENTION : property not string in ProductVersion\n");
          }else{
            if( Prop->getString()->stringValue().notEmpty() ) {
              OSVersion = Prop->getString()->stringValue();
            }
          }
        }
        Prop = Dict->propertyForKey("ProductBuildVersion");
        if ( Prop != NULL ) {
          if ( !Prop->isString() ) {
            MsgLog("ATTENTION : property not string in ProductBuildVersion\n");
          }else{
            if( Prop->getString()->stringValue().notEmpty() ) {
              BuildVersion = Prop->getString()->stringValue();
            }
          }
        }
        Dict->FreeTag();
      }
    }

//    if ( OSVersion.isEmpty() )
//    {
//      if ( FileExists(Volume->RootDir, SWPrintf("\\%ls\\com.apple.installer\\BridgeVersion.plist", APFSTargetUUID.wc_str()).wc_str()) ) {
//        OSVersion = "11.0"_XS8;
//        // TODO so far, is there is a BridgeVersion.plist, it's version 11.0. Has to be improved with next releases.
//      }
//    }

    // 1st stage - 2
    // Check for plist - createinstallmedia/NetInstall
    if (OSVersion.isEmpty()) {
      InstallerPlist = SWPrintf("\\.IABootFiles\\com.apple.Boot.plist"); // 10.9 - ...
      if (FileExists (Volume->RootDir, InstallerPlist)) {
        Status = egLoadFile(Volume->RootDir, InstallerPlist.wc_str(), (UINT8 **)&PlistBuffer, &PlistLen);
        if (!EFI_ERROR(Status) && PlistBuffer != NULL && ParseXML(PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
          Prop = Dict->propertyForKey("Kernel Flags");
          if ( Prop != NULL ) {
            if ( !Prop->isString() ) {
              MsgLog("ATTENTION : property not string in Kernel Flags\n");
            }else{
              if ( Prop->getString()->stringValue().contains("Install%20macOS%20BigSur") || Prop->getString()->stringValue().contains("Install%20macOS%2011")) {
                OSVersion = "11"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20macOS%20Monterey") || Prop->getString()->stringValue().contains("Install%20macOS%2012")) {
                OSVersion = "12"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20macOS%2010.16")) {
                OSVersion = "10.16"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20macOS%20Catalina") || Prop->getString()->stringValue().contains("Install%20macOS%2010.15")) {
                OSVersion = "10.15"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20macOS%20Mojave") || Prop->getString()->stringValue().contains("Install%20macOS%2010.14")) {
                OSVersion = "10.14"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20macOS%20High%20Sierra") || Prop->getString()->stringValue().contains("Install%20macOS%2010.13")) {
                OSVersion = "10.13"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20macOS%20Sierra") || Prop->getString()->stringValue().contains("Install%20OS%20hhX%2010.12")) {
                OSVersion = "10.12"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20OS%20hhX%20El%20Capitan") || Prop->getString()->stringValue().contains("Install%20OS%20hhX%2010.11")) {
                OSVersion = "10.11"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20OS%20hhX%20Yosemite") || Prop->getString()->stringValue().contains("Install%20OS%20hhX%2010.10")) {
                OSVersion = "10.10"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20OS%20hhX%20Mavericks.app")) {
                OSVersion = "10.9"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20OS%20hhX%20Mountain%20Lion")) {
                OSVersion = "10.8"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20Mac%20OS%20hhX%20Lion")) {
                OSVersion = "10.7"_XS8;
              }
            }
          }
        }
      }
    }

    // 2nd stage - 1
    // Check for plist - AppStore/createinstallmedia/startosinstall/Fusion Drive
    if (OSVersion.isEmpty()) {
      InstallerPlist = SWPrintf("\\macOS Install Data\\Locked Files\\Boot Files\\SystemVersion.plist"); // 10.12.4+
      if (!FileExists (Volume->RootDir, InstallerPlist)) {
        InstallerPlist = SWPrintf("\\macOS Install Data\\InstallInfo.plist"); // 10.12+
        if (!FileExists (Volume->RootDir, InstallerPlist)) {
          InstallerPlist = SWPrintf("\\com.apple.boot.R\\SystemVersion.plist)"); // 10.12+
          if (!FileExists (Volume->RootDir, InstallerPlist)) {
            InstallerPlist = SWPrintf("\\com.apple.boot.P\\SystemVersion.plist"); // 10.12+
            if (!FileExists (Volume->RootDir, InstallerPlist)) {
              InstallerPlist = SWPrintf("\\com.apple.boot.S\\SystemVersion.plist"); // 10.12+
              if (!FileExists (Volume->RootDir, InstallerPlist) &&
                  (FileExists (Volume->RootDir, L"\\com.apple.boot.R\\System\\Library\\PrelinkedKernels\\prelinkedkernel") ||
                   FileExists (Volume->RootDir, L"\\com.apple.boot.P\\System\\Library\\PrelinkedKernels\\prelinkedkernel") ||
                   FileExists (Volume->RootDir, L"\\com.apple.boot.S\\System\\Library\\PrelinkedKernels\\prelinkedkernel"))) {
                InstallerPlist = SWPrintf("\\System\\Library\\CoreServices\\SystemVersion.plist"); // 10.11
              }
            }
          }
        }
      }
      if (FileExists (Volume->RootDir, InstallerPlist)) {
        Status = egLoadFile(Volume->RootDir, InstallerPlist.wc_str(), (UINT8 **)&PlistBuffer, &PlistLen);
        if (!EFI_ERROR(Status) && PlistBuffer != NULL && ParseXML(PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
          Prop = Dict->propertyForKey("ProductVersion");
          if ( Prop != NULL ) {
            if ( !Prop->isString() ) {
              MsgLog("ATTENTION : property not string in ProductVersion\n");
            }else{
              if( Prop->getString()->stringValue().notEmpty() ) {
                OSVersion = Prop->getString()->stringValue();
              }
            }
          }
          Prop = Dict->propertyForKey("ProductBuildVersion");
          if ( Prop != NULL ) {
            if ( !Prop->isString() ) {
              MsgLog("ATTENTION : property not string in ProductBuildVersion\n");
            }else{
              if( Prop->getString()->stringValue().notEmpty() ) {
                BuildVersion = Prop->getString()->stringValue();
              }
            }
          }
          // In InstallInfo.plist, there is no a version key only when updating from AppStore in 10.13+
          // If use the startosinstall in 10.13+, this version key exists in InstallInfo.plist
          DictPointer = Dict->dictPropertyForKey("System Image Info"); // 10.12+
          if (DictPointer != NULL) {
            Prop = DictPointer->propertyForKey("version");
            if ( Prop != NULL ) {
              if ( !Prop->isString() ) {
                MsgLog("ATTENTION : property not string in version\n");
              }else{
                OSVersion = Prop->getString()->stringValue();
              }
            }
          }
          Dict->FreeTag();
        }
      }
    }

    // 2nd stage - 2
    // Check for ia.log - InstallESD/createinstallmedia/startosinstall
    // Implemented by Sherlocks
    if (OSVersion.isEmpty()) {
      CONST CHAR8  *s, *fileBuffer;
//      CHAR8  *Res5 = (__typeof__(Res5))AllocateZeroPool(5);
//      CHAR8  *Res6 = (__typeof__(Res6))AllocateZeroPool(6);
//      CHAR8  *Res7 = (__typeof__(Res7))AllocateZeroPool(7);
//      CHAR8  *Res8 = (__typeof__(Res8))AllocateZeroPool(8);
      UINTN  fileLen = 0;
      XStringW InstallerLog;
      InstallerLog = L"\\Mac OS X Install Data\\ia.log"_XSW; // 10.7
      if (!FileExists (Volume->RootDir, InstallerLog)) {
        InstallerLog = L"\\OS X Install Data\\ia.log"_XSW; // 10.8 - 10.11
        if (!FileExists (Volume->RootDir, InstallerLog)) {
          InstallerLog = L"\\macOS Install Data\\ia.log"_XSW; // 10.12+
        }
      }
      if (FileExists (Volume->RootDir, InstallerLog)) {
        Status = egLoadFile(Volume->RootDir, InstallerLog.wc_str(), (UINT8 **)&fileBuffer, &fileLen);
        if (!EFI_ERROR(Status)) {
          XString8 targetString;
          targetString.strncpy(fileBuffer, fileLen);
      //    s = SearchString(targetString, fileLen, "Running OS Build: Mac OS X ", 27);
          s = AsciiStrStr(targetString.c_str(), "Running OS Build: Mac OS X ");
          if (s[31] == ' ') {
            OSVersion.S8Printf("%c%c.%c\n", s[27], s[28], s[30]);
            if (s[38] == ')') {
              BuildVersion.S8Printf("%c%c%c%c%c\n", s[33], s[34], s[35], s[36], s[37]);
            } else if (s[39] == ')') {
              BuildVersion.S8Printf("%c%c%c%c%c%c\n", s[33], s[34], s[35], s[36], s[37], s[38]);
            }
          } else if (s[31] == '.') {
            OSVersion.S8Printf("%c%c.%c.%c\n", s[27], s[28], s[30], s[32]);
            if (s[40] == ')') {
              BuildVersion.S8Printf("%c%c%c%c%c\n", s[35], s[36], s[37], s[38], s[39]);
            } else if (s[41] == ')') {
              BuildVersion.S8Printf("%c%c%c%c%c%c\n", s[35], s[36], s[37], s[38], s[39], s[40]);
            }
          } else if (s[32] == ' ') {
            OSVersion.S8Printf("%c%c.%c%c\n", s[27], s[28], s[30], s[31]);
            if (s[39] == ')') {
              BuildVersion.S8Printf("%c%c%c%c%c\n", s[34], s[35], s[36], s[37], s[38]);
            } else if (s[40] == ')') {
              BuildVersion.S8Printf("%c%c%c%c%c%c\n", s[34], s[35], s[36], s[37], s[38], s[39]);
            } else if (s[41] == ')') {
              BuildVersion.S8Printf("%c%c%c%c%c%c%c\n", s[34], s[35], s[36], s[37], s[38], s[39], s[40]);
            }
          } else if (s[32] == '.') {
            OSVersion.S8Printf("%c%c.%c%c.%c\n", s[27], s[28], s[30], s[31], s[33]);
            if (s[41] == ')') {
              BuildVersion.S8Printf("%c%c%c%c%c\n", s[36], s[37], s[38], s[39], s[40]);
            } else if (s[42] == ')') {
              BuildVersion.S8Printf("%c%c%c%c%c%c\n", s[36], s[37], s[38], s[39], s[40], s[41]);
            } else if (s[43] == ')') {
              BuildVersion.S8Printf("%c%c%c%c%c%c%c\n", s[36], s[37], s[38], s[39], s[40], s[41], s[42]);
            }
          }
          FreePool(fileBuffer);
        }
      }
    }

    // 2nd stage - 3
    // Check for plist - Preboot of APFS
    if ( OSVersion.isEmpty() )
    {
      XStringW plist = L"\\macOS Install Data\\Locked Files\\Boot Files\\SystemVersion.plist"_XSW;
      if ( !FileExists(Volume->RootDir, plist) ) {
        plist.setEmpty();
      }

      if ( plist.notEmpty() ) { // found macOS System

        Status = egLoadFile(Volume->RootDir, plist.wc_str(), (UINT8 **)&PlistBuffer, &PlistLen);
        if (!EFI_ERROR(Status) && PlistBuffer != NULL && ParseXML(PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
          Prop = Dict->propertyForKey("ProductVersion");
          if ( Prop != NULL ) {
            if ( !Prop->isString() ) {
              MsgLog("ATTENTION : property not string in ProductVersion\n");
            }else{
              OSVersion = Prop->getString()->stringValue();
            }
          }
          Prop = Dict->propertyForKey("ProductBuildVersion");
          if ( Prop != NULL ) {
            if ( !Prop->isString() ) {
              MsgLog("ATTENTION : property not string in ProductBuildVersion\n");
            }else{
              BuildVersion = Prop->getString()->stringValue();
            }
          }
        }
        Dict->FreeTag();
      }
    }
  }

  if (OSTYPE_IS_OSX_RECOVERY (LoaderType)) {

    XString8 uuidPrefix;
    if ( APFSTargetUUID.notEmpty() ) uuidPrefix = S8Printf("\\%ls", APFSTargetUUID.wc_str());

    XStringW plist = SWPrintf("%s\\SystemVersion.plist", uuidPrefix.c_str());
    if ( !FileExists(Volume->RootDir, plist) ) {
      plist = SWPrintf("%s\\ServerVersion.plist", uuidPrefix.c_str());
      if ( !FileExists(Volume->RootDir, plist) ) {
        plist = L"\\com.apple.recovery.boot\\SystemVersion.plist"_XSW;
        if ( !FileExists(Volume->RootDir, plist) ) {
          plist = L"\\com.apple.recovery.boot\\ServerVersion.plist"_XSW;
          if ( !FileExists(Volume->RootDir, plist) ) {
            plist.setEmpty();
          }
        }
      }
    }

    // Detect exact version for OS X Recovery
    if ( plist.notEmpty() ) { // found macOS System
      Status = egLoadFile(Volume->RootDir, plist.wc_str(), (UINT8 **)&PlistBuffer, &PlistLen);
      if (!EFI_ERROR(Status) && PlistBuffer != NULL && ParseXML(PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
        Prop = Dict->propertyForKey("ProductVersion");
        if ( Prop != NULL ) {
          if ( !Prop->isString() ) {
            MsgLog("ATTENTION : property not string in ProductVersion\n");
          }else{
            OSVersion = Prop->getString()->stringValue();
          }
        }
        Prop = Dict->propertyForKey("ProductBuildVersion");
        if ( Prop != NULL ) {
          if ( !Prop->isString() ) {
            MsgLog("ATTENTION : property not string in ProductBuildVersion\n");
          }else{
            BuildVersion = Prop->getString()->stringValue();
          }
        }
      }
      Dict->FreeTag();
    } else if (FileExists (Volume->RootDir, L"\\com.apple.recovery.boot\\boot.efi")) {
      // Special case - com.apple.recovery.boot/boot.efi exists but SystemVersion.plist doesn't --> 10.9 recovery
      OSVersion = "10.9"_XS8;
    }
  }

  if (PlistBuffer != NULL) {
    FreePool(PlistBuffer);
  }
  (*BuildVersionPtr).stealValueFrom(&BuildVersion);
  return OSVersion;
}

inline MacOsVersion GetOSVersion (IN LOADER_ENTRY *Entry) { return GetOSVersion(Entry->LoaderType, Entry->APFSTargetUUID, Entry->Volume, &Entry->BuildVersion); };

//constexpr XStringW iconMac = L"mac"_XSW;
CONST XStringW
GetOSIconName (const MacOsVersion& OSVersion)
{
  XStringW OSIconName;
  if (OSVersion.isEmpty()) {
    OSIconName = L"mac"_XSW;
  } else if (OSVersion.elementAt(0) == 12 ){
    // Monterey
    OSIconName = L"monterey,mac"_XSW;
  } else if ( (OSVersion.elementAt(0) == 10 && OSVersion.elementAt(1) == 16 ) ||
              (OSVersion.elementAt(0) == 11 /*&& OSVersion.elementAt(1) == 0*/ )
            ) {
    // Big Sur
    OSIconName = L"bigsur,mac"_XSW;
  } else if ( OSVersion.elementAt(0) == 10 ) {
    if ( OSVersion.elementAt(1) == 15 ) {
      // Catalina
      OSIconName = L"cata,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 14 ) {
      // Mojave
      OSIconName = L"moja,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 13 ) {
      // High Sierra
      OSIconName = L"hsierra,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 12 ) {
      // Sierra
      OSIconName = L"sierra,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 11 ) {
      // El Capitan
      OSIconName = L"cap,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 10 ) {
      // Yosemite
      OSIconName = L"yos,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 9 ) {
      // Mavericks
      OSIconName = L"mav,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 8 ) {
      // Mountain Lion
      OSIconName = L"cougar,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 7 ) {
      // Lion
      OSIconName = L"lion,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 6 ) {
      // Snow Leopard
      OSIconName = L"snow,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 5 ) {
      // Leopard
      OSIconName = L"leo,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 4 ) {
      // Tiger
      OSIconName = L"tiger,mac"_XSW;
    } else {
      OSIconName = L"mac"_XSW;
    }
  } else {
    OSIconName = L"mac"_XSW;
  }

  return OSIconName;
}

STATIC LOADER_ENTRY *CreateLoaderEntry(IN CONST XStringW& LoaderPath,
                                       IN CONST XString8Array& LoaderOptions,
                                       IN CONST XString8& FullTitle,
                                       IN CONST XString8& LoaderTitle,
                                       IN REFIT_VOLUME *Volume,
                                       IN XIcon *Image,
                                       IN XIcon *DriveImage,
                                       IN UINT8 OSType,
                                       IN UINT8 Flags,
                                       IN char32_t Hotkey,
                                       EFI_GRAPHICS_OUTPUT_BLT_PIXEL BootBgColor,
                                       IN UINT8 CustomBoot,
                                       IN const XImage& CustomLogo,
                                       IN const KERNEL_AND_KEXT_PATCHES* Patches,
                                       IN BOOLEAN CustomEntry)
{
  EFI_DEVICE_PATH       *LoaderDevicePath;
  XStringW               LoaderDevicePathString;
  XStringW               FilePathAsString;
//  CONST CHAR16          *OSIconName = NULL;
  CHAR16                ShortcutLetter;
  LOADER_ENTRY          *Entry;
  CONST CHAR8           *indent = "      ";

  // Check parameters are valid
  if ((LoaderPath.isEmpty()) || (Volume == NULL)) {
    return NULL;
  }

  // Get the loader device path
  LoaderDevicePath = FileDevicePath(Volume->DeviceHandle, LoaderPath);
  if (LoaderDevicePath == NULL) {
    return NULL;
  }
  LoaderDevicePathString = FileDevicePathToXStringW(LoaderDevicePath);
  if (LoaderDevicePathString.isEmpty()) {
    return NULL;
  }

  // Ignore this loader if it's self path
  XStringW selfDevicePathAsXStringW = FileDevicePathToXStringW(&self.getSelfDevicePath());
  if ( selfDevicePathAsXStringW == LoaderDevicePathString ) {
    DBG("%sskipped because path `%ls` is self path!\n", indent, LoaderDevicePathString.wc_str());
    return NULL;
  }
// DBG("OSType =%d\n", OSType);
  // DBG("prepare the menu entry\n");
  // prepare the menu entry
  Entry = new LOADER_ENTRY;

  if (!CustomEntry) {
    // Ignore this loader if it's device path is already present in another loader
      for (UINTN i = 0; i < MainMenu.Entries.size(); ++i) {
        REFIT_ABSTRACT_MENU_ENTRY& MainEntry = MainMenu.Entries[i];
        // Only want loaders
        if (MainEntry.getLOADER_ENTRY()) {
          if (StriCmp(MainEntry.getLOADER_ENTRY()->DevicePathString.wc_str(), LoaderDevicePathString.wc_str()) == 0) {
            DBG("%sskipped because path `%ls` already exists for another entry!\n", indent, LoaderDevicePathString.wc_str());
            return NULL;
          }
        }
      }
    // If this isn't a custom entry make sure it's not hidden by a custom entry
    for (size_t CustomIndex = 0 ; CustomIndex < GlobalConfig.CustomEntries.size() ; ++CustomIndex ) {
      CUSTOM_LOADER_ENTRY& Custom = GlobalConfig.CustomEntries[CustomIndex];
      if ( Custom.settings.Disabled ) continue; // before, disabled entries settings weren't loaded.
      // Check if the custom entry is hidden or disabled
      if ( OSFLAG_ISSET(Custom.getFlags(gSettings.SystemParameters.NoCaches), OSFLAG_DISABLED)  || Custom.settings.Hidden ) {

        INTN volume_match=0;
        INTN volume_type_match=0;
        INTN path_match=0;
        INTN type_match=0;

        // Check if volume match
        if (Custom.settings.Volume.notEmpty()) {
          // Check if the string matches the volume
          volume_match =
            ((StrStr(Volume->DevicePathString.wc_str(), Custom.settings.Volume.wc_str()) != NULL) ||
             ((Volume->VolName.notEmpty()) && (StrStr(Volume->VolName.wc_str(), Custom.settings.Volume.wc_str()) != NULL))) ? 1 : -1;
        }

        // Check if the volume_type match
        if (Custom.settings.VolumeType != 0) {
          volume_type_match = (((1ull<<Volume->DiskKind) & Custom.settings.VolumeType) != 0) ? 1 : -1;
        }

        // Check if the path match
        if (Custom.settings.Path.notEmpty()) {
          // Check if the loader path match
          path_match = (Custom.settings.Path.isEqualIC(LoaderPath)) ? 1 : -1;
        }

        // Check if the type match
        if (Custom.settings.Type != 0) {
          type_match = OSTYPE_COMPARE(Custom.settings.Type, OSType) ? 1 : -1;
        }

        if (volume_match == -1 || volume_type_match == -1 || path_match == -1 || type_match == -1 ) {
          UINTN add_comma = 0;

          DBG("%sNot match custom entry %zu: ", indent, CustomIndex);
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
          DBG("%sHidden because matching custom entry %zu!\n", indent, CustomIndex);
          Entry->Hidden = true;
        }
      }
    }
  }
  Entry->Row = 0;
  Entry->Volume = Volume;

  if ( LoaderPath.length() >= 38 ) {
    if ( isPathSeparator(LoaderPath[0])  &&  isPathSeparator(LoaderPath[37]) ) {
      if ( IsValidGuidString(LoaderPath.data(1), 36) ) {
        Entry->APFSTargetUUID = LoaderPath.subString(1, 36);
      }
    }
  }

//  Entry->APFSTargetUUID = APFSTargetUUID;

  Entry->LoaderPath       = LoaderPath;
  Entry->DisplayedVolName = Volume->VolName;
  Entry->DevicePath       = LoaderDevicePath;
  Entry->DevicePathString = LoaderDevicePathString;
  Entry->Flags            = OSFLAG_SET(Flags, OSFLAG_USEGRAPHICS);

  if (OSFLAG_ISSET(Flags, OSFLAG_NODEFAULTARGS)) {
    Entry->LoadOptions  = LoaderOptions;
  }else{
    Entry->LoadOptions = Split<XString8Array>(gSettings.Boot.BootArgs, " ");
    Entry->LoadOptions.import(LoaderOptions);
  }
  //actions
  Entry->AtClick = ActionSelect;
  Entry->AtDoubleClick = ActionEnter;
  Entry->AtRightClick = ActionDetails;
  Entry->CustomBoot = CustomBoot;
  Entry->CustomLogo = CustomLogo; //could be an empty image

  Entry->LoaderType = OSType;
  Entry->BuildVersion.setEmpty();
#ifdef JIEF_DEBUG
if ( Entry->LoaderPath.contains("com.apple.installer") ) {
  DBG("%s", "");
}
if ( Entry->APFSTargetUUID.startWith("99999999") ) {
  DBG("%s", "");
}
#endif
  Entry->macOSVersion = GetOSVersion(Entry);
  DBG("%sOSVersion=%s \n", indent, Entry->macOSVersion.asString().c_str());
  // detect specific loaders
  XStringW OSIconName;
  ShortcutLetter = 0;

  switch (OSType) {
    case OSTYPE_OSX:
    case OSTYPE_RECOVERY:
    case OSTYPE_OSX_INSTALLER:
      OSIconName = GetOSIconName(Entry->macOSVersion);// Sothor - Get OSIcon name using OSVersion
      // apianti - force custom logo even when verbose
/* this is not needed, as this flag is also being unset when booting if -v is present (LoadOptions may change until then)
      if ( Entry->LoadOptions.containsIC("-v")  ) {
        // OSX is not booting verbose, so we can set console to graphics mode
        Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_USEGRAPHICS);
      }
*/
      if (OSType == OSTYPE_OSX && IsOsxHibernated(Entry)) {
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_HIBERNATED);
        DBG("%s  =>set entry as hibernated\n", indent);
      }
      //always unset checkFakeSmc for installer
      if (OSType == OSTYPE_OSX_INSTALLER){
//        Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_CHECKFAKESMC);
//        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
      }
      ShortcutLetter = 'M';
      if ( Entry->DisplayedVolName.isEmpty() ) {
        // else no sense to override it with dubious name
        GetOSXVolumeName(Entry);
      }
      break;
    case OSTYPE_WIN:
      OSIconName = L"win"_XSW;
      ShortcutLetter = 'W';
      break;
    case OSTYPE_WINEFI:
      OSIconName = L"vista,win"_XSW;
      //ShortcutLetter = 'V';
      ShortcutLetter = 'W';
      break;
    case OSTYPE_LIN:
    case OSTYPE_LINEFI:
    // we already detected linux and have Path and Image
      Entry->LoaderType = OSType;
      OSIconName = L"linux"_XSW;
      if (Image == nullptr) {
        DBG("%slinux image not found\n", indent);
        OSIconName = LinuxIconNameFromPath(LoaderPath, Volume->RootDir); //something named "issue"
      }
      ShortcutLetter = 'L';
      break;
    //case OSTYPE_OTHER: 
    case OSTYPE_EFI:
      OSIconName = L"clover"_XSW;
      ShortcutLetter = 'E';
      Entry->LoaderType = OSTYPE_OTHER;
      break;
    default:
      OSIconName = L"unknown"_XSW;
      Entry->LoaderType = OSTYPE_OTHER;
      break;
  }
//DBG("OSIconName=%ls \n", OSIconName);
  Entry->Title = FullTitle;
  if (Entry->Title.isEmpty()  &&  Volume->VolLabel.notEmpty()) {
    if (Volume->VolLabel[0] == L'#') {
      Entry->Title.SWPrintf("Boot %ls from %ls", (!LoaderTitle.isEmpty()) ? XStringW(LoaderTitle).wc_str() : LoaderPath.basename().wc_str(), Volume->VolLabel.data(1));
    }else{
      Entry->Title.SWPrintf("Boot %ls from %ls", (!LoaderTitle.isEmpty()) ? XStringW(LoaderTitle).wc_str() : LoaderPath.basename().wc_str(), Volume->VolLabel.wc_str());
    }
  }

  BOOLEAN BootCampStyle = ThemeX.BootCampStyle;

  if ( Entry->Title.isEmpty()  &&  Entry->DisplayedVolName.isEmpty() ) {
    XStringW BasenameXW = XStringW(Basename(Volume->DevicePathString.wc_str()));
 //   DBG("encounter Entry->VolName ==%ls and StrLen(Entry->VolName) ==%llu\n",Entry->VolName, StrLen(Entry->VolName));
    if (BootCampStyle) {
      if (!LoaderTitle.isEmpty()) {
        Entry->Title = LoaderTitle;
      } else {
        Entry->Title = (BasenameXW.contains(L"-")) ? BasenameXW.subString(0,BasenameXW.indexOf(L"-") + 1) + L"..)" : BasenameXW;
      }
    } else {
      Entry->Title.SWPrintf("Boot %ls from %ls", (!LoaderTitle.isEmpty()) ? XStringW(LoaderTitle).wc_str() : LoaderPath.basename().wc_str(),
                            (BasenameXW.contains(L"-")) ? (BasenameXW.subString(0,BasenameXW.indexOf(L"-") + 1) + L"..)").wc_str() : BasenameXW.wc_str());
    }
  }
//  DBG("check Entry->Title \n");
  if ( Entry->Title.isEmpty() ) {
 //   DBG("encounter LoaderTitle ==%ls and Entry->VolName ==%ls\n", LoaderTitle.wc_str(), Entry->VolName);
    if (BootCampStyle) {
      if ((StriCmp(XStringW(LoaderTitle).wc_str(), L"macOS") == 0) || (StriCmp(XStringW(LoaderTitle).wc_str(), L"Recovery") == 0)) {
        Entry->Title.takeValueFrom(Entry->DisplayedVolName);
      } else {
        if (!LoaderTitle.isEmpty()) {
          Entry->Title = LoaderTitle;
        } else {
          Entry->Title = LoaderPath.basename();
        }
      }
    } else {
      Entry->Title.SWPrintf("Boot %ls from %ls", (!LoaderTitle.isEmpty()) ? XStringW(LoaderTitle).wc_str() : LoaderPath.basename().wc_str(),
                            Entry->DisplayedVolName.wc_str());
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
  if (gSettings.GUI.CustomIcons && FileExists(Volume->RootDir, L"\\.VolumeIcon.icns")){
    Entry->Image.Image.LoadIcns(Volume->RootDir, L"\\.VolumeIcon.icns", 128);
    if (!Entry->Image.Image.isEmpty()) {
      Entry->Image.setFilled();
      DBG("%susing VolumeIcon.icns image from Volume\n", indent);
    }    
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
      Entry->BadgeImage.Image = XImage(Entry->DriveImage.Image, 0);
       DBG("%sShow badge as Drive.\n", indent);
    } else {
      Entry->BadgeImage.Image = XImage(Entry->Image.Image, 0);
       DBG("%sShow badge as OSImage.\n", indent);
    }
    if (!Entry->BadgeImage.Image.isEmpty()) {
      Entry->BadgeImage.setFilled();
    }
  }
  Entry->BootBgColor = BootBgColor;

//  Entry->KernelAndKextPatches = ((Patches == NULL) ? (KERNEL_AND_KEXT_PATCHES *)(((UINTN)&gSettings) + OFFSET_OF(SETTINGS_DATA, KernelAndKextPatches)) : Patches);
//  CopyKernelAndKextPatches(&Entry->KernelAndKextPatches, Patches == NULL ? &gSettings.KernelAndKextPatches : Patches);
  Entry->KernelAndKextPatches = Patches == NULL ? gSettings.KernelAndKextPatches : *Patches;
  
#ifdef DUMP_KERNEL_KEXT_PATCHES
  DumpKernelAndKextPatches(Entry->KernelAndKextPatches);
#endif
  DBG("%sLoader entry created for '%ls'\n", indent, Entry->DevicePathString.wc_str());
  return Entry;
}

void LOADER_ENTRY::AddDefaultMenu()
{
  XStringW     FileName;
//  CHAR16* TempOptions;
//  CHAR16            DiagsFileName[256];
  LOADER_ENTRY      *SubEntry;
//  REFIT_MENU_SCREEN *SubScreen;
//  REFIT_VOLUME      *Volume;
  UINT64            VolumeSize;
  EFI_GUID          *Guid = NULL;
  BOOLEAN           KernelIs64BitOnly;
//  UINT64            os_version = AsciiOSVersionToUint64(OSVersion);

  constexpr LString8 quietLitteral = "quiet"_XS8;
  constexpr LString8 splashLitteral = "splash"_XS8;

  // Only kernels up to 10.7 have 32-bit mode
  KernelIs64BitOnly = (macOSVersion.isEmpty() ||
                       macOSVersion >= MacOsVersion("10.8"_XS8));
  
  const char* macOS = (macOSVersion.notEmpty() && macOSVersion < MacOsVersion("10.8"_XS8))? "Mac OS X" :
                      (macOSVersion.notEmpty() && macOSVersion < MacOsVersion("10.12"_XS8))? "OS X" : "macOS";

  FileName = LoaderPath.basename();

  // create the submenu
  SubScreen = new REFIT_MENU_SCREEN;
  SubScreen->Title.SWPrintf("Options for %ls on %ls", Title.wc_str(), DisplayedVolName.wc_str());

  SubScreen->TitleImage = Image;
  SubScreen->ID = LoaderType + 20; //wow
//    DBG("get anime for os=%lld\n", SubScreen->ID);
  SubScreen->GetAnime();
  VolumeSize = RShiftU64(MultU64x32(Volume->BlockIO->Media->LastBlock, Volume->BlockIO->Media->BlockSize), 20);
  SubScreen->AddMenuInfoLine_f("Volume size: %lluMb", VolumeSize);
  SubScreen->AddMenuInfoLine_f("%ls", FileDevicePathToXStringW(DevicePath).wc_str());
  Guid = FindGPTPartitionGuidInDevicePath(Volume->DevicePath);
  if (Guid) {
    SubScreen->AddMenuInfoLine_f("UUID: %s", strguid(Guid));
  }
  if ( Volume->ApfsFileSystemUUID.notEmpty() ||  APFSTargetUUID.notEmpty() ) {
    SubScreen->AddMenuInfoLine_f("APFS volume name: %ls", DisplayedVolName.wc_str());
  }
  if ( Volume->ApfsFileSystemUUID.notEmpty() ) {
    SubScreen->AddMenuInfoLine_f("APFS file system UUID: %s", Volume->ApfsFileSystemUUID.c_str());
  }
  if ( Volume->ApfsContainerUUID.notEmpty() ) {
    SubScreen->AddMenuInfoLine_f("APFS container UUID: %s", Volume->ApfsContainerUUID.c_str());
  }
  if ( APFSTargetUUID.notEmpty() ) {
    SubScreen->AddMenuInfoLine_f("APFS target UUID: %ls", APFSTargetUUID.wc_str());
  }
  SubScreen->AddMenuInfoLine_f("Options: %s", LoadOptions.ConcatAll(" "_XS8).c_str());
  // loader-specific submenu entries
  if (LoaderType == OSTYPE_OSX ||
      LoaderType == OSTYPE_OSX_INSTALLER ||
      LoaderType == OSTYPE_RECOVERY) { // entries for Mac OS X
    SubScreen->AddMenuInfoLine_f("%s: %s", macOS, macOSVersion.asString().c_str());

    if (OSFLAG_ISSET(Flags, OSFLAG_HIBERNATED)) {
      SubEntry = getPartiallyDuplicatedEntry();
       SubEntry->Title.takeValueFrom("Cancel hibernate wake");
       SubEntry->Flags     = OSFLAG_UNSET(SubEntry->Flags, OSFLAG_HIBERNATED);
       SubScreen->AddMenuEntry(SubEntry, true);
    }

    SubEntry = getPartiallyDuplicatedEntry();
    SubEntry->Title.SWPrintf("Boot %s with selected options", macOS);
    SubScreen->AddMenuEntry(SubEntry, true);
#if 0
    SubEntry = getPartiallyDuplicatedEntry();
    SubEntry->Title.SWPrintf("Boot %s with injected kexts", macOS);
    SubEntry->Flags       = OSFLAG_UNSET(SubEntry->Flags, OSFLAG_CHECKFAKESMC);
    SubEntry->Flags       = OSFLAG_SET(SubEntry->Flags, OSFLAG_WITHKEXTS);
    SubScreen->AddMenuEntry(SubEntry, true);

    SubEntry = getPartiallyDuplicatedEntry();
    SubEntry->Title.SWPrintf("Boot %s without injected kexts", macOS);
    SubEntry->Flags       = OSFLAG_UNSET(SubEntry->Flags, OSFLAG_CHECKFAKESMC);
    SubEntry->Flags       = OSFLAG_UNSET(SubEntry->Flags, OSFLAG_WITHKEXTS);
    SubScreen->AddMenuEntry(SubEntry, true);
#endif
    SubScreen->AddMenuEntry(SubMenuKextInjectMgmt(), true);
    SubScreen->AddMenuInfo_f("=== boot-args ===");
    if (!KernelIs64BitOnly) {
      if ( macOSVersion.notEmpty() && macOSVersion < MacOsVersion("10.8"_XS8) ) {
        SubScreen->AddMenuCheck("Mac OS X 32bit",   OPT_I386, 68);
      }
//      SubScreen->AddMenuCheck(XString8().SPrintf("%s 64bit", macOS).c_str(), OPT_X64,  68);
      SubScreen->AddMenuCheck((macOS + " 64bit"_XS8).c_str(), OPT_X64,  68);
    }
    SubScreen->AddMenuCheck("Verbose (-v)",                               OPT_VERBOSE, 68);
    // No Caches option works on 10.6 - 10.9
    if ( macOSVersion.notEmpty() && macOSVersion < MacOsVersion("10.10"_XS8) ) {
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

    if (gSettings.RtVariables.CsrActiveConfig == 0) {
      SubScreen->AddMenuCheck("No SIP", OSFLAG_NOSIP, 69);
    }
    
  } else if (LoaderType == OSTYPE_LINEFI) {
    BOOLEAN Quiet = LoadOptions.contains(quietLitteral);
    BOOLEAN WithSplash = LoadOptions.contains(splashLitteral);
    
    // default entry
    SubEntry = getPartiallyDuplicatedEntry();
    SubEntry->Title.SWPrintf("Run %ls", FileName.wc_str());
    SubScreen->AddMenuEntry(SubEntry, true);

    SubEntry = getPartiallyDuplicatedEntry();
    if (Quiet) {
      SubEntry->Title.SWPrintf("%ls verbose", Title.s());
      SubEntry->LoadOptions.removeIC(quietLitteral);
    } else {
      SubEntry->Title.SWPrintf("%ls quiet", Title.s());
      SubEntry->LoadOptions.AddID(quietLitteral);
    }
    SubScreen->AddMenuEntry(SubEntry, true);

    SubEntry = getPartiallyDuplicatedEntry();
    if (WithSplash) {
      SubEntry->Title.SWPrintf("%ls without splash", Title.s());
      SubEntry->LoadOptions.removeIC(splashLitteral);
    } else {
      SubEntry->Title.SWPrintf("%ls with splash", Title.s());
      SubEntry->LoadOptions.AddID(splashLitteral);
    }
    SubScreen->AddMenuEntry(SubEntry, true);

    SubEntry = getPartiallyDuplicatedEntry();
    if (WithSplash) {
      if (Quiet) {
        SubEntry->Title.SWPrintf("%ls verbose without splash", Title.s());
        SubEntry->LoadOptions.removeIC(splashLitteral);
        SubEntry->LoadOptions.removeIC(quietLitteral);
      } else {
        SubEntry->Title.SWPrintf("%ls quiet without splash",Title.s());
        SubEntry->LoadOptions.removeIC(splashLitteral);
        SubEntry->LoadOptions.Add(quietLitteral);
      }
    } else if (Quiet) {
      SubEntry->Title.SWPrintf("%ls verbose with splash",Title.s());
      SubEntry->LoadOptions.removeIC(quietLitteral); //
      SubEntry->LoadOptions.AddID(splashLitteral);
    } else {
      SubEntry->Title.SWPrintf("%ls quiet with splash",Title.s());
      SubEntry->LoadOptions.AddID(quietLitteral);
      SubEntry->LoadOptions.AddID(splashLitteral);
    }
    SubScreen->AddMenuEntry(SubEntry, true);

  } else if ((LoaderType == OSTYPE_WIN) || (LoaderType == OSTYPE_WINEFI)) {
    // by default, skip the built-in selection and boot from hard disk only
    LoadOptions.setEmpty();
    LoadOptions.Add("-s"_XS8);
    LoadOptions.Add("-h"_XS8);
    
    // default entry
    SubEntry = getPartiallyDuplicatedEntry();
    SubEntry->Title.SWPrintf("Run %ls", FileName.wc_str());
    SubScreen->AddMenuEntry(SubEntry, true);

    SubEntry = getPartiallyDuplicatedEntry();
    SubEntry->Title.takeValueFrom("Boot Windows from Hard Disk");
    SubScreen->AddMenuEntry(SubEntry, true);

    SubEntry = getPartiallyDuplicatedEntry();
    SubEntry->Title.takeValueFrom("Boot Windows from CD-ROM");
    LoadOptions.setEmpty();
    LoadOptions.Add("-s"_XS8);
    LoadOptions.Add("-c"_XS8);
    SubScreen->AddMenuEntry(SubEntry, true);

    SubEntry = getPartiallyDuplicatedEntry();
    SubEntry->Title.SWPrintf("Run %ls in text mode", FileName.wc_str());
    SubEntry->Flags           = OSFLAG_UNSET(SubEntry->Flags, OSFLAG_USEGRAPHICS);
    LoadOptions.setEmpty();
    LoadOptions.Add("-v"_XS8);
    SubEntry->LoaderType      = OSTYPE_OTHER; // Sothor - Why are we using OSTYPE_OTHER here?
    SubScreen->AddMenuEntry(SubEntry, true);

  }else{
    // default entry
    SubEntry = getPartiallyDuplicatedEntry();
    SubEntry->Title.SWPrintf("Run %ls", FileName.wc_str());
    SubScreen->AddMenuEntry(SubEntry, true);
  }

//  SubScreen->AddMenuEntry(&MenuEntryReturn, false); //one-way ticket to avoid confusion
  // DBG("    Added '%ls': OSType='%d', OSVersion='%s'\n",Title,LoaderType,OSVersion);
}

LOADER_ENTRY* AddLoaderEntry(IN CONST XStringW& LoaderPath, IN CONST XString8Array& LoaderOptions,
                       IN CONST XStringW& FullTitle, IN CONST XStringW& LoaderTitle,
                       IN REFIT_VOLUME *Volume, IN XIcon *Image,
                       IN UINT8 OSType, IN UINT8 Flags)
{
  LOADER_ENTRY *Entry;

  if ((LoaderPath.isEmpty()) || (Volume == NULL) || (Volume->RootDir == NULL) || !FileExists(Volume->RootDir, LoaderPath)) {
    return NULL;
  }

  DBG("      AddLoaderEntry for Volume Name=%ls, idx=%zu\n", Volume->VolName.wc_str(), MainMenu.Entries.sizeIncludingHidden());
  if (OSFLAG_ISSET(Flags, OSFLAG_DISABLED)) {
    DBG("     skipped because entry is disabled\n");
    return NULL;
  }
//  if (!gSettings.ShowHiddenEntries && OSFLAG_ISSET(Flags, OSFLAG_HIDDEN)) {
//    DBG("        skipped because entry is hidden\n");
//    return NULL;
//  }
  //don't add hided entries
//  if (!gSettings.ShowHiddenEntries) {
//    for (HVi = 0; HVi < gSettings.HVCount; HVi++) {
//      if ( LoaderPath.containsIC(gSettings.GUI.HVHideStrings[HVi]) ) {
//        DBG("        hiding entry: %ls\n", LoaderPath.s());
//        return NULL;
//      }
//    }
//  }

  Entry = CreateLoaderEntry(LoaderPath, LoaderOptions, FullTitle, LoaderTitle, Volume, Image, NULL, OSType, Flags, 0, MenuBackgroundPixel, CUSTOM_BOOT_DISABLED, NullXImage, NULL, FALSE);
  if (Entry != NULL) {
    if ((Entry->LoaderType == OSTYPE_OSX) ||
        (Entry->LoaderType == OSTYPE_OSX_INSTALLER ) ||
        (Entry->LoaderType == OSTYPE_RECOVERY)) {
//      if (gSettings.WithKexts) {
//        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
//      }
//      if (gSettings.WithKextsIfNoFakeSMC) {
//        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_CHECKFAKESMC);
//        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
//      }
      if (gSettings.SystemParameters.NoCaches) {
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NOCACHES);
      }
    }
    if ( Volume->Hidden ) {
      DBG("     hiding entry because volume is hidden: %ls\n", LoaderPath.s());
      Entry->Hidden = true;
    }else{
      for (size_t HVi = 0; HVi < gSettings.GUI.HVHideStrings.size(); HVi++) {
        if ( LoaderPath.containsIC(gSettings.GUI.HVHideStrings[HVi]) ) {
          DBG("     hiding entry: %ls\n", LoaderPath.s());
          Entry->Hidden = true;
        }
      }
    }
    //TODO there is a problem that Entry->Flags is unique while InputItems are global ;(
//    InputItems[69].IValue = Entry->Flags;
    Entry->AddDefaultMenu();
    DBG("      Menu entry added at index %zd\n", MainMenu.Entries.sizeIncludingHidden());
    MainMenu.AddMenuEntry(Entry, true);
    return Entry;
  }
  return NULL;
}

STATIC void LinuxScan(REFIT_VOLUME *Volume, UINT8 KernelScan, UINT8 Type, XStringW *CustomPath, XIcon *CustomImage)
{
  // When used for Regular Entries, all found entries will be added by AddLoaderEntry()
  // When used for Custom Entries (detected by CustomPath!=NULL), CustomPath+CustomImage will be set to the first entry found and execution will stop
  // Scanning is adjusted according to Type: OSTYPE_LIN will scan for linux loaders, OSTYPE_LINEFI will scan for linux kernels, unspecified will scan for both
  UINTN        Index;
  EFI_GUID     *PartGUID;

  // check for linux loaders
  if (Type != OSTYPE_LINEFI) { // OSTYPE_LIN or unspecified
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
      XStringW OSName = XStringW().takeValueFrom(DirEntry->FileName); // this is folder name, for example "ubuntu"
      OSName.lowerAscii(); // lowercase for icon name and title (first letter in title will be capitalized later)
      if (FileExists(Volume->RootDir, File)) {
        // check if nonstandard icon mapping is needed
        for (Index = 0; Index < LinuxIconMappingCount; ++Index) {
          if (StrCmp(OSName.wc_str(),LinuxIconMapping[Index].DirectoryName) == 0) {
            OSName = XStringW().takeValueFrom(LinuxIconMapping[Index].IconName);
            break;
          }
        }
        if (!CustomPath) {
          DBG("  found entry %ls,linux\n", OSName.wc_str());
        }
        XStringW LoaderTitle = OSName.subString(0,1); // capitalize first letter for title
        LoaderTitle.upperAscii();
        LoaderTitle += OSName.subString(1, OSName.length()) + L" Linux"_XSW;
        // Very few linux icons exist in IconNames, but these few may be preloaded, so check that first
        XIcon ImageX = ThemeX.GetIcon(L"os_"_XSW + OSName); //will the image be destroyed or rewritten by next image after the cycle end?
        if (ImageX.isEmpty()) {
          // no preloaded icon, try to load from dir
          ImageX.LoadXImage(&ThemeX.getThemeDir(), L"os_"_XSW + OSName);
        }
        if (CustomPath) { 
          *CustomPath = File;
          if (CustomImage) {
            *CustomImage = ImageX;
          }
          DirIterClose(&DirIter);
          return;
        } 
        AddLoaderEntry(File, NullXString8Array, L""_XSW, LoaderTitle, Volume,
                      (ImageX.isEmpty() ? NULL : &ImageX), OSTYPE_LIN, OSFLAG_NODEFAULTARGS);
      } //anyway continue search other entries
    }
    DirIterClose(&DirIter);

    // check for non-standard grub path
    for (Index = 0; Index < LinuxEntryDataCount; ++Index) {
      if (FileExists(Volume->RootDir, LinuxEntryData[Index].Path)) {
        XStringW OSIconName = XStringW().takeValueFrom(LinuxEntryData[Index].Icon);
        OSIconName = OSIconName.subString(0, OSIconName.indexOf(','));
        XIcon ImageX = ThemeX.GetIcon(L"os_"_XSW + OSIconName);
        if (ImageX.isEmpty()) {
          ImageX.LoadXImage(&ThemeX.getThemeDir(), L"os_"_XSW + OSIconName);
        }
        if (CustomPath) {
          *CustomPath = LinuxEntryData[Index].Path;
          if (CustomImage) {
            *CustomImage = ImageX;
          }
          return;
        }
        AddLoaderEntry(LinuxEntryData[Index].Path, NullXString8Array, L""_XSW, XStringW().takeValueFrom(LinuxEntryData[Index].Title), Volume,
                       (ImageX.isEmpty() ? NULL : &ImageX), OSTYPE_LIN, OSFLAG_NODEFAULTARGS);
      }
    }

  }

  if (Type != OSTYPE_LIN) { //OSTYPE_LINEFI or unspecified
    // check for linux kernels
    PartGUID = FindGPTPartitionGuidInDevicePath(Volume->DevicePath);
    if ((PartGUID != NULL) && (Volume->RootDir != NULL)) {
      REFIT_DIR_ITER  Iter;
      EFI_FILE_INFO  *FileInfo = NULL;
      EFI_TIME        PreviousTime;
      XStringW        Path;
      // CHAR16         *Options;
      // Get the partition UUID and make sure it's lower case
      CHAR16          PartUUID[40];
      ZeroMem(&PreviousTime, sizeof(EFI_TIME));
      snwprintf(PartUUID, sizeof(PartUUID), "%s", strguid(PartGUID));
      StrToLower(PartUUID);
      // open the /boot directory (or whatever directory path)
      DirIterOpen(Volume->RootDir, LINUX_BOOT_PATH, &Iter);
  
      // Check which kernel scan to use
  
      // the following options can produce only a single option
      switch (KernelScan) {
        case KERNEL_SCAN_FIRST:
          // First kernel found only
          while (DirIterNext(&Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo)) {
            if (FileInfo != NULL) {
              if (FileInfo->FileSize == 0) {
                continue;
              }
              // get the kernel file path
              Path.SWPrintf("%ls\\%ls", LINUX_BOOT_PATH, FileInfo->FileName);
              // free the file info
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
                Path.SWPrintf("%ls\\%ls", LINUX_BOOT_PATH, FileInfo->FileName);
              }
            }
          }
          break;
        case KERNEL_SCAN_NEWEST:
          // Newest dated kernel only
          while (DirIterNext(&Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo)) {
            if (FileInfo != NULL) {
              if (FileInfo->FileSize > 0) {
                // get the kernel file path
                if ((PreviousTime.Year == 0) || (TimeCmp(&PreviousTime, &(FileInfo->ModificationTime)) < 0)) {
                  Path.SWPrintf("%ls\\%ls", LINUX_BOOT_PATH, FileInfo->FileName);
                  PreviousTime = FileInfo->ModificationTime;
                }
              }
            }
          }
          break;
        case KERNEL_SCAN_OLDEST:
          // Oldest dated kernel only
          while (DirIterNext(&Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo)) {
            if (FileInfo != NULL) {
              if (FileInfo->FileSize > 0) {
                // get the kernel file path
                if ((PreviousTime.Year == 0) || (TimeCmp(&PreviousTime, &(FileInfo->ModificationTime)) > 0)) {
                  Path.SWPrintf("%ls\\%ls", LINUX_BOOT_PATH, FileInfo->FileName);
                  PreviousTime = FileInfo->ModificationTime;
                }
              }
            }
          }
          break;
        case KERNEL_SCAN_MOSTRECENT:
          // most recent kernel version only
          while (DirIterNext(&Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo)) {
            if (FileInfo != NULL) {
              if (FileInfo->FileSize > 0) {
                // get the kernel file path
                XStringW NewPath = SWPrintf("%ls\\%ls", LINUX_BOOT_PATH, FileInfo->FileName);
                if ( Path < NewPath ) {
                   Path = NewPath;
                } else {
                    Path.setEmpty();
                }
              }
            }
          }
          break;
        case KERNEL_SCAN_EARLIEST:
          // earliest kernel version only
          while (DirIterNext(&Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo)) {
            if (FileInfo != NULL) {
              if (FileInfo->FileSize > 0) {
                // get the kernel file path
                XStringW NewPath = SWPrintf("%ls\\%ls", LINUX_BOOT_PATH, FileInfo->FileName);
                if ( Path > NewPath ) {
                  Path = NewPath;
                } else {
                  Path.setEmpty();
                }
              }
            }
          }
          break;
        case KERNEL_SCAN_NONE:
        default:
          // No kernel scan
          break;
      }

      // add the produced entry
      if (Path.notEmpty()) {
        if (CustomPath) {
          *CustomPath = Path;
          DirIterClose(&Iter);
          return;
        }
        XString8Array Options = LinuxKernelOptions(Iter.DirHandle, Basename(Path.wc_str()) + LINUX_LOADER_PATH.length(), PartUUID, NullXString8Array);
        // Add the entry
        AddLoaderEntry(Path, (Options.isEmpty()) ? LINUX_DEFAULT_OPTIONS : Options, L""_XSW, L""_XSW, Volume, NULL, OSTYPE_LINEFI, OSFLAG_NODEFAULTARGS);
        Path.setEmpty();
      }

      // the following produces multiple entries
      // custom entries has a different implementation, and does not use this code
      if (!CustomPath && KernelScan == KERNEL_SCAN_ALL) {
        // get all the filename matches
        while (DirIterNext(&Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo)) {
          if (FileInfo != NULL) {
            if (FileInfo->FileSize > 0) {
              // get the kernel file path
              Path.SWPrintf("%ls\\%ls", LINUX_BOOT_PATH, FileInfo->FileName);
              XString8Array Options = LinuxKernelOptions(Iter.DirHandle, Basename(Path.wc_str()) + LINUX_LOADER_PATH.length(), PartUUID, NullXString8Array);
              // Add the entry
              AddLoaderEntry(Path, (Options.isEmpty()) ? LINUX_DEFAULT_OPTIONS : Options, L""_XSW, L""_XSW, Volume, NULL, OSTYPE_LINEFI, OSFLAG_NODEFAULTARGS);
              Path.setEmpty();
            }
          }
        }
      }
      //close the directory
      DirIterClose(&Iter);
    }  
  }

}

#define Paper 1
#define Rock  2
#define Scissor 4

void AddPRSEntry(REFIT_VOLUME *Volume)
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
      AddLoaderEntry(PaperBoot, NullXString8Array, L""_XSW, L"macOS InstallP"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0);
      break;
    case Scissor:
    case (Paper | Scissor):
      AddLoaderEntry(ScissorBoot, NullXString8Array, L""_XSW, L"macOS InstallS"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0);
      break;
    case Rock:
    case (Rock | Scissor):
    case (Rock | Scissor | Paper):
      AddLoaderEntry(RockBoot, NullXString8Array, L""_XSW, L"macOS InstallR"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0);
      break;

    default:
      break;
  }
}
#undef Paper
#undef Rock
#undef Scissor

XString8 GetAuthRootDmg(const EFI_FILE& dir, const XStringW& path)
{
  XString8 returnValue;

  XStringW plist = SWPrintf("%ls\\com.apple.Boot.plist", path.wc_str());
  if ( !FileExists(dir, plist) ) return NullXString8;

  CHAR8*     PlistBuffer = NULL;
  UINTN      PlistLen;
  TagDict*     Dict        = NULL;
  const TagStruct*     Prop        = NULL;

  EFI_STATUS Status = egLoadFile(&dir, plist.wc_str(), (UINT8 **)&PlistBuffer, &PlistLen);
  if (!EFI_ERROR(Status) && PlistBuffer != NULL && ParseXML(PlistBuffer, &Dict, 0) == EFI_SUCCESS)
  {
    Prop = Dict->propertyForKey("Kernel Flags");
    if ( Prop != NULL ) {
      if ( !Prop->isString() ) {
        MsgLog("ATTENTION : Kernel Flags not string in ProductVersion\n");
      }else{
        if( Prop->getString()->stringValue().notEmpty() ) {
          const XString8& kernelFlags = Prop->getString()->stringValue();
          size_t idx = kernelFlags.indexOf("auth-root-dmg");
          if ( idx == MAX_XSIZE ) return NullXString8;
          idx += strlen("auth-root-dmg");
          while ( idx < kernelFlags.length()  &&  IS_BLANK(kernelFlags[idx]) ) ++idx;
          if ( kernelFlags[idx] == '=' ) ++idx;
          else return NullXString8;
          while ( idx < kernelFlags.length()  &&  IS_BLANK(kernelFlags[idx]) ) ++idx;
          if ( kernelFlags.isEqualAtIC(idx, "file://"_XS8) ) idx += strlen("file://");
          size_t idxEnd = idx;
          while ( idxEnd < kernelFlags.length()  &&  !IS_BLANK(kernelFlags[idxEnd]) ) ++idxEnd;
          returnValue = kernelFlags.subString(idx, idxEnd - idx);
        }
      }
    }
  }
  if ( PlistBuffer ) FreePool(PlistBuffer);
  return returnValue;
}

MacOsVersion GetMacOSVersionFromFolder(const EFI_FILE& dir, const XStringW& path)
{
  MacOsVersion macOSVersion;

  XStringW plist = SWPrintf("%ls\\SystemVersion.plist", path.wc_str());
  if ( !FileExists(dir, plist) ) {
    plist = SWPrintf("%ls\\ServerVersion.plist", path.wc_str());
    if ( !FileExists(dir, plist) ) {
      plist.setEmpty();
    }
  }

  if ( plist.notEmpty() ) { // found macOS System
    CHAR8*     PlistBuffer = NULL;
    UINTN      PlistLen;
    TagDict*     Dict        = NULL;
    const TagStruct*     Prop        = NULL;

    EFI_STATUS Status = egLoadFile(&dir, plist.wc_str(), (UINT8 **)&PlistBuffer, &PlistLen);
    if (!EFI_ERROR(Status) && PlistBuffer != NULL && ParseXML(PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
      Prop = Dict->propertyForKey("ProductVersion");
      if ( Prop != NULL ) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in ProductVersion\n");
        }else{
          if( Prop->getString()->stringValue().notEmpty() ) {
            macOSVersion = Prop->getString()->stringValue();
          }
        }
      }
    }
    if ( PlistBuffer ) FreePool(PlistBuffer);
  }
  return macOSVersion;
}

void ScanLoader(void)
{
  //DBG("Scanning loaders...\n");
  DbgHeader("ScanLoader");
   
  for (UINTN VolumeIndex = 0; VolumeIndex < Volumes.size(); VolumeIndex++)
  {
    REFIT_VOLUME* Volume = &Volumes[VolumeIndex];
    if (Volume->RootDir == NULL) { // || Volume->VolName == NULL)
      //DBG(", no file system\n", VolumeIndex);
      continue;
    }
    DBG("- [%02llu]: '%ls'", VolumeIndex, Volume->VolName.wc_str());
    if (Volume->VolName.isEmpty()) {
      Volume->VolName = L"Unknown"_XSW;
    }

    // skip volume if its kind is configured as disabled
    if (((1ull<<Volume->DiskKind) & GlobalConfig.DisableFlags) != 0)
    {
      DBG(", flagged disable\n");
      continue;
    }

    DBG("\n");

    if ( Volume->ApfsContainerUUID.notEmpty() ) DBG("    ApfsContainerUUID=%s\n", Volume->ApfsContainerUUID.c_str());
    if ( Volume->ApfsFileSystemUUID.notEmpty() ) DBG("    ApfsFileSystemUUID=%s\n", Volume->ApfsFileSystemUUID.c_str());


    // check for Mac OS X Install Data
    // 1st stage - createinstallmedia
    if (FileExists(Volume->RootDir, L"\\.IABootFiles\\boot.efi")) {
      if (FileExists(Volume->RootDir, L"\\Install OS X Mavericks.app") ||
          FileExists(Volume->RootDir, L"\\Install OS X Yosemite.app") ||
          FileExists(Volume->RootDir, L"\\Install OS X El Capitan.app")) {
        AddLoaderEntry(L"\\.IABootFiles\\boot.efi"_XSW, NullXString8Array, L""_XSW, L"OS X Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.9 - 10.11
      } else {
        AddLoaderEntry(L"\\.IABootFiles\\boot.efi"_XSW, NullXString8Array, L""_XSW, L"macOS Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.12 - 10.13.3
      }
    } else if (FileExists(Volume->RootDir, L"\\.IAPhysicalMedia") && FileExists(Volume->RootDir, MACOSX_LOADER_PATH)) {
      AddLoaderEntry(MACOSX_LOADER_PATH, NullXString8Array, L""_XSW, L"macOS Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.13.4+
    }
    // 2nd stage - InstallESD/AppStore/startosinstall/Fusion Drive
     // 10.7
    AddLoaderEntry(L"\\Mac OS X Install Data\\boot.efi"_XSW, NullXString8Array, L""_XSW, L"Mac OS X Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.7
    // 10.8 - 10.11
    AddLoaderEntry(L"\\OS X Install Data\\boot.efi"_XSW, NullXString8Array, L""_XSW, L"OS X Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.8 - 10.11
    // 10.12 - 10.12.3
    AddLoaderEntry(L"\\macOS Install Data\\boot.efi"_XSW, NullXString8Array, L""_XSW, L"macOS Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.12 - 10.12.3
    // 10.12.4-10.15
    AddLoaderEntry(L"\\macOS Install Data\\Locked Files\\Boot Files\\boot.efi"_XSW, NullXString8Array, L""_XSW, L"macOS Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0);
    // Big Sur install must be via Preboot. Next line must stay commented.
    // AddLoaderEntry(L"\\macOS Install Data\\Locked Files\\boot.efi"_XSW, NullXString8Array, L""_XSW, L"macOS Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 11+
    AddPRSEntry(Volume); // 10.12+

    // Netinstall
    AddLoaderEntry(L"\\NetInstall macOS High Sierra.nbi\\i386\\booter"_XSW, NullXString8Array, L""_XSW, L"macOS Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0);
    // Use standard location for boot.efi, according to the install files is present
    // That file indentifies a DVD/ESD/BaseSystem/Fusion Drive Install Media, so when present, check standard path to avoid entry duplication
    if (FileExists(Volume->RootDir, MACOSX_LOADER_PATH)) {
      if (FileExists(Volume->RootDir, L"\\System\\Installation\\CDIS\\Mac OS X Installer.app")) {
        // InstallDVD/BaseSystem
        AddLoaderEntry(MACOSX_LOADER_PATH, NullXString8Array, L""_XSW, L"Mac OS X Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.6/10.7
      } else if (FileExists(Volume->RootDir, L"\\System\\Installation\\CDIS\\OS X Installer.app")) {
        // BaseSystem
        AddLoaderEntry(MACOSX_LOADER_PATH, NullXString8Array, L""_XSW, L"OS X Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.8 - 10.11
      } else if (FileExists(Volume->RootDir, L"\\System\\Installation\\CDIS\\macOS Installer.app")) {
        // BaseSystem
        AddLoaderEntry(MACOSX_LOADER_PATH, NullXString8Array, L""_XSW, L"macOS Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.12+
      } else if (FileExists(Volume->RootDir, L"\\BaseSystem.dmg") && FileExists(Volume->RootDir, L"\\mach_kernel")) {
        // InstallESD
        if (FileExists(Volume->RootDir, L"\\MacOSX_Media_Background.png")) {
          AddLoaderEntry(MACOSX_LOADER_PATH, NullXString8Array, L""_XSW, L"Mac OS X Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.7
        } else {
          AddLoaderEntry(MACOSX_LOADER_PATH, NullXString8Array, L""_XSW, L"OS X Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.8
        }
      } else if (FileExists(Volume->RootDir, L"\\com.apple.boot.R\\System\\Library\\PrelinkedKernels\\prelinkedkernel") ||
                 FileExists(Volume->RootDir, L"\\com.apple.boot.P\\System\\Library\\PrelinkedKernels\\prelinkedkernel") ||
                 FileExists(Volume->RootDir, L"\\com.apple.boot.S\\System\\Library\\PrelinkedKernels\\prelinkedkernel")) {
        if (StriStr(Volume->VolName.wc_str(), L"Recovery") != NULL) {
          // FileVault of HFS+
          // TODO: need info for 10.11 and lower
          AddLoaderEntry(MACOSX_LOADER_PATH, NullXString8Array, L""_XSW, L"macOS FileVault"_XSW, Volume, NULL, OSTYPE_OSX, 0); // 10.12+
        } else {
          // Fusion Drive
          AddLoaderEntry(MACOSX_LOADER_PATH, NullXString8Array, L""_XSW, L"OS X Install"_XSW, Volume, NULL, OSTYPE_OSX_INSTALLER, 0); // 10.11
        }
      } else if (!FileExists(Volume->RootDir, L"\\.IAPhysicalMedia")) {
        // Installed
        if (EFI_ERROR(GetRootUUID(Volume)) || isFirstRootUUID(Volume)) {
          if (!FileExists(Volume->RootDir, L"\\System\\Library\\CoreServices\\NotificationCenter.app") && !FileExists(Volume->RootDir, L"\\System\\Library\\CoreServices\\Siri.app")) {
            AddLoaderEntry(MACOSX_LOADER_PATH, NullXString8Array, L""_XSW, L"Mac OS X"_XSW, Volume, NULL, OSTYPE_OSX, 0); // 10.6 - 10.7
          } else if (FileExists(Volume->RootDir, L"\\System\\Library\\CoreServices\\NotificationCenter.app") && !FileExists(Volume->RootDir, L"\\System\\Library\\CoreServices\\Siri.app")) {
            AddLoaderEntry(MACOSX_LOADER_PATH, NullXString8Array, L""_XSW, L"OS X"_XSW, Volume, NULL, OSTYPE_OSX, 0); // 10.8 - 10.11
          } else {
            MacOsVersion macOSVersion;
            if ( Volume->ApfsFileSystemUUID.notEmpty() && (Volume->ApfsRole & APPLE_APFS_VOLUME_ROLE_SYSTEM) != 0 )
            {
              macOSVersion = GetMacOSVersionFromFolder(*Volume->RootDir, L"\\System\\Library\\CoreServices"_XSW);
            }
            if ( macOSVersion < MacOsVersion("11"_XS8) ) {
              AddLoaderEntry(MACOSX_LOADER_PATH, NullXString8Array, L""_XSW, L"macOS"_XSW, Volume, NULL, OSTYPE_OSX, 0); // 10.12+
            }
          }
        }
      }
    }

    // check for Mac OS X Recovery Boot
    AddLoaderEntry(L"\\com.apple.recovery.boot\\boot.efi"_XSW, NullXString8Array, L""_XSW, L"Recovery"_XSW, Volume, NULL, OSTYPE_RECOVERY, 0);

    // Sometimes, on some systems (HP UEFI, if Win is installed first)
    // it is needed to get rid of bootmgfw.efi to allow starting of
    // Clover as /efi/boot/bootx64.efi from HD. We can do that by renaming
    // bootmgfw.efi to bootmgfw-orig.efi
    AddLoaderEntry(L"\\EFI\\microsoft\\Boot\\bootmgfw-orig.efi"_XSW, NullXString8Array, L""_XSW, L"Microsoft EFI"_XSW, Volume, NULL, OSTYPE_WINEFI, 0);
    // check for Microsoft boot loader/menu
    // If there is bootmgfw-orig.efi, then do not check for bootmgfw.efi
    // since on some systems this will actually be CloverX64.efi
    // renamed to bootmgfw.efi
    AddLoaderEntry(L"\\EFI\\microsoft\\Boot\\bootmgfw.efi"_XSW, NullXString8Array, L""_XSW, L"Microsoft EFI Boot"_XSW, Volume, NULL, OSTYPE_WINEFI, 0);
    // check for Microsoft boot loader/menu. This entry is redundant so excluded
    // AddLoaderEntry(L"\\bootmgr.efi", L"", L"Microsoft EFI mgrboot", Volume, NULL, OSTYPE_WINEFI, 0);
    // check for Microsoft boot loader/menu on CDROM
    if (!AddLoaderEntry(L"\\EFI\\MICROSOFT\\BOOT\\cdboot.efi"_XSW, NullXString8Array, L""_XSW, L"Microsoft EFI cdboot"_XSW, Volume, NULL, OSTYPE_WINEFI, 0)) {
      AddLoaderEntry(L"\\EFI\\MICROSOFT\\BOOT\\CDBOOT.EFI"_XSW, NullXString8Array, L""_XSW, L"Microsoft EFI CDBOOT"_XSW, Volume, NULL, OSTYPE_WINEFI, 0);
    }


#if defined(ANDX86)
    if (TRUE) { //gSettings.AndroidScan
      // check for Android loaders
      for (UINTN Index = 0; Index < AndroidEntryDataCount; ++Index) {
        UINTN aIndex, aFound;
      if (FileExists(Volume->RootDir, AndroidEntryData[Index].Path)) {
          aFound = 0;
          for (aIndex = 0; aIndex < ANDX86_FINDLEN; ++aIndex) {
            if ((AndroidEntryData[Index].Find[aIndex].isEmpty()) || FileExists(Volume->RootDir, AndroidEntryData[Index].Find[aIndex])) ++aFound;
          }
          if (aFound && (aFound == aIndex)) {
            XIcon ImageX;
            XStringW IconXSW = XStringW().takeValueFrom(AndroidEntryData[Index].Icon);
            ImageX.LoadXImage(&ThemeX.getThemeDir(), (L"os_"_XSW + IconXSW.subString(0, IconXSW.indexOf(','))).wc_str());
            AddLoaderEntry(AndroidEntryData[Index].Path, NullXString8Array, L""_XSW, XStringW().takeValueFrom(AndroidEntryData[Index].Title), Volume,
                           (ImageX.isEmpty() ? NULL : &ImageX), OSTYPE_LIN, OSFLAG_NODEFAULTARGS);
          }
        }
      }
    }
#endif

    if (gSettings.GUI.Scan.LinuxScan) {
      LinuxScan(Volume, gSettings.GUI.Scan.KernelScan, 0, NULL, NULL);
    }

    //     DBG("search for  optical UEFI\n");
    if (Volume->DiskKind == DISK_KIND_OPTICAL) {
      AddLoaderEntry(BOOT_LOADER_PATH, NullXString8Array, L""_XSW, L"UEFI optical"_XSW, Volume, NULL, OSTYPE_OTHER, 0);
    }
    //     DBG("search for internal UEFI\n");
    if (Volume->DiskKind == DISK_KIND_INTERNAL) {
      LOADER_ENTRY* le = AddLoaderEntry(BOOT_LOADER_PATH, NullXString8Array, L""_XSW, L"UEFI internal"_XSW, Volume, NULL, OSTYPE_OTHER, 0);
      if ( le ) {
        DBG("     hiding entry because DiskKind is DISK_KIND_INTERNAL: %ls\n", le->LoaderPath.s());
        le->Hidden = true;
      }
    }
    //    DBG("search for external UEFI\n");
    if (Volume->DiskKind == DISK_KIND_EXTERNAL) {
      LOADER_ENTRY* le = AddLoaderEntry(BOOT_LOADER_PATH, NullXString8Array, L""_XSW, L"UEFI external"_XSW, Volume, NULL, OSTYPE_OTHER, 0);
      if ( le ) {
        DBG("     hiding entry because DiskKind is DISK_KIND_EXTERNAL: %ls\n", le->LoaderPath.s());
        le->Hidden = true;
      }
    }

//DBG("Volume->ApfsTargetUUIDArray.size()=%zd\n", Volume->ApfsTargetUUIDArray.size());
    if ( Volume->ApfsTargetUUIDArray.size() > 0 ) {

      for (UINTN i = 0; i < Volume->ApfsTargetUUIDArray.size(); i++)
      {
        const XString8& ApfsTargetUUID = Volume->ApfsTargetUUIDArray[i];
        DBG("    APFSTargetUUID=%s\n", ApfsTargetUUID.c_str());
        XStringW FullTitle;
        XStringW FullTitleRecovery;
        XStringW FullTitleInstaller;
        XStringW LoaderTitle;
        XStringW LoaderTitleInstaller;

        // Find the "target" volume.
        REFIT_VOLUME* targetVolume = Volumes.getVolumeWithApfsContainerUUIDAndFileSystemUUID(Volume->ApfsContainerUUID, ApfsTargetUUID);
        // If targetVolume is found, and it's a data partition, try to find the system partition that goes with it.
//DBG("targetVolume=%d\n", targetVolume ? 1 : 0);
        if ( targetVolume ) {
          if ( (targetVolume->ApfsRole & APPLE_APFS_VOLUME_ROLE_DATA) != 0 ) {
            for (size_t VolumeIndex2 = 0; VolumeIndex2 < Volumes.size(); VolumeIndex2++) {
              REFIT_VOLUME* Volume2 = &Volumes[VolumeIndex2];
//DBG("idx=%zu  name %ls  uuid=%s \n", VolumeIndex2, Volume2->VolName.wc_str(), Volume2->ApfsFileSystemUUID.c_str());
              if ( Volume2->ApfsContainerUUID == targetVolume->ApfsContainerUUID ) {
                if ( (Volume2->ApfsRole & APPLE_APFS_VOLUME_ROLE_SYSTEM) != 0 ) {
                  if ( !targetVolume ) {
                    targetVolume = Volume2;
                  }else{
                    // More than one system partition in container. I don't know how to select which one is supposed to pair with this
                    targetVolume = NULL; // we'll try .disk_label.contentDetails
                    break; // we need to escape the loop after bootVolume = NULL;
                  }
                }
              }
            }
          }
        }
        // If targetVolume is not found, and it's not a recovery, find the preboot volume from the same container
//DBG("2) targetVolume=%d\n", targetVolume ? 1 : 0);
        if ( !targetVolume ) {
          REFIT_VOLUME* bootVolume = Volume;
          if ( (Volume->ApfsRole & APPLE_APFS_VOLUME_ROLE_RECOVERY) != 0 ) {
            for (size_t VolumeIndex2 = 0; VolumeIndex2 < Volumes.size(); VolumeIndex2++) {
              REFIT_VOLUME* Volume2 = &Volumes[VolumeIndex2];
//DBG("idx=%zu  name %ls  uuid=%s \n", VolumeIndex2, Volume2->VolName.wc_str(), Volume2->ApfsFileSystemUUID.c_str());
              if ( (Volume2->ApfsRole & APPLE_APFS_VOLUME_ROLE_PREBOOT) != 0 ) {
                if ( Volume2->ApfsContainerUUID == Volume->ApfsContainerUUID ) {
                  bootVolume = Volume2;
                  break;
                }
              }
            }
          }
          if ( bootVolume ) {
            XStringW targetNameFile;
            CHAR8*  fileBuffer;
            UINTN   fileLen = 0;
            targetNameFile.SWPrintf("%s\\System\\Library\\CoreServices\\.disk_label.contentDetails", ApfsTargetUUID.c_str());
            if ( FileExists(bootVolume->RootDir, targetNameFile) ) {
              EFI_STATUS Status = egLoadFile(bootVolume->RootDir, targetNameFile.wc_str(), (UINT8 **)&fileBuffer, &fileLen);
              if(!EFI_ERROR(Status)) {
                FullTitle.SWPrintf("Boot Mac OS from %.*s", (int)fileLen, fileBuffer);
                FullTitleRecovery.SWPrintf("Boot Mac OS Recovery for %.*s", (int)fileLen, fileBuffer);
                FullTitleInstaller.SWPrintf("Boot Mac OS Install for %.*s", (int)fileLen, fileBuffer);
                if ( fileLen < MAX_INT32 ) {
                  DBG("      contentDetails name:%.*s\n", (int)fileLen, fileBuffer);
                }
                FreePool(fileBuffer);
              }
            }
          }
        }
        if ( FullTitle.isEmpty() ) {
          if ( targetVolume ) {
            FullTitle.SWPrintf("Boot Mac OS from %ls", targetVolume->getVolLabelOrOSXVolumeNameOrVolName().wc_str());
            FullTitleRecovery.SWPrintf("Boot Mac OS Recovery for %ls", targetVolume->getVolLabelOrOSXVolumeNameOrVolName().wc_str());
            FullTitleInstaller.SWPrintf("Boot Mac OS Install for %ls", targetVolume->getVolLabelOrOSXVolumeNameOrVolName().wc_str());
          }else{
            FullTitle.SWPrintf("Boot Mac OS");
            FullTitleRecovery.SWPrintf("Boot Mac OS Recovery");
            FullTitleInstaller.SWPrintf("Mac OS Install");
          }
        }
        /*MacOsVersion macOSVersion = GetMacOSVersionFromFolder(*Volume->RootDir, SWPrintf("\\%s\\System\\Library\\CoreServices", ApfsTargetUUID.c_str()));
        if ( macOSVersion.notEmpty() && macOSVersion < MacOsVersion("11"_XS8) )*/ FullTitle.SWCatf(" via %ls", Volume->getVolLabelOrOSXVolumeNameOrVolName().wc_str());
        AddLoaderEntry(SWPrintf("\\%s\\System\\Library\\CoreServices\\boot.efi", ApfsTargetUUID.c_str()), NullXString8Array, FullTitle, LoaderTitle, Volume, NULL, OSTYPE_OSX, 0);

        //Try to add Recovery APFS entry
        /*macOSVersion = GetMacOSVersionFromFolder(*Volume->RootDir, SWPrintf("\\%s", ApfsTargetUUID.c_str()));
        if ( macOSVersion.notEmpty() && macOSVersion < MacOsVersion("11"_XS8) )*/ FullTitleRecovery.SWCatf(" via %ls", Volume->getVolLabelOrOSXVolumeNameOrVolName().wc_str());
        if (!AddLoaderEntry(SWPrintf("\\%s\\boot.efi", Volume->ApfsTargetUUIDArray[i].c_str()), NullXString8Array, FullTitleRecovery, L""_XSW, Volume, NULL, OSTYPE_RECOVERY, 0)) {
          //Try to add Recovery APFS entry as dmg
          AddLoaderEntry(SWPrintf("\\%s\\BaseSystem.dmg", Volume->ApfsTargetUUIDArray[i].c_str()), NullXString8Array, FullTitleRecovery, L""_XSW, Volume, NULL, OSTYPE_RECOVERY, 0);
        }
       //Try to add macOS install entry
        /*macOSVersion = GetMacOSVersionFromFolder(*Volume->RootDir, SWPrintf("\\%s\\com.apple.installer", ApfsTargetUUID.c_str()));
        if ( macOSVersion.notEmpty() && macOSVersion < MacOsVersion("11"_XS8) )*/ FullTitleInstaller.SWCatf(" via %ls", Volume->getVolLabelOrOSXVolumeNameOrVolName().wc_str());

        XString8 installerPath = SWPrintf("\\%s\\com.apple.installer", Volume->ApfsTargetUUIDArray[i].c_str());
        if ( FileExists(Volume->RootDir, installerPath) ) {
          XString8 rootDmg = GetAuthRootDmg(*Volume->RootDir, installerPath);
          rootDmg.replaceAll("%20"_XS8, " "_XS8);
//          while ( rootDmg.notEmpty()  &&  rootDmg.startWith('/') ) rootDmg.deleteCharsAtPos(0, 1);
          rootDmg.replaceAll('/', '\\');
          REFIT_VOLUME* targetInstallVolume = Volumes.getVolumeWithApfsContainerUUIDAndFileSystemUUID(Volume->ApfsContainerUUID, Volume->ApfsTargetUUIDArray[i]);
          if ( targetInstallVolume ) {
            EFI_FILE_PROTOCOL* TestFile;
            EFI_STATUS Status = targetInstallVolume->RootDir->Open(targetInstallVolume->RootDir, &TestFile, L"\\", EFI_FILE_MODE_READ, 0);
            if ( EFI_ERROR(Status) ) TestFile = NULL; // if the root of the volume can't be opened (most likely encrypted), add the installer anyway.
            if ( rootDmg.isEmpty()  ||  EFI_ERROR(Status)  ||  FileExists(*targetInstallVolume->RootDir, rootDmg) ) { // rootDmg empty is accepted, to be compatible with previous code
              AddLoaderEntry(SWPrintf("\\%s\\com.apple.installer\\boot.efi", Volume->ApfsTargetUUIDArray[i].c_str()), NullXString8Array, FullTitleInstaller, LoaderTitleInstaller, Volume, NULL, OSTYPE_OSX_INSTALLER, 0);
            }else{
              DBG("    Dead installer entry found (installer dmg boot file not found : '%s')\n", rootDmg.c_str());
            }
            if ( TestFile != NULL ) TestFile->Close(TestFile);
          }else{
            DBG("    Dead installer entry found (target volume not found : '%s')\n", Volume->ApfsTargetUUIDArray[i].c_str());
          }
        }
      }
    }
  }

  DBG("Entries list before ordering\n");
  for (size_t idx = 0; idx < MainMenu.Entries.sizeIncludingHidden(); idx++) {
    if ( MainMenu.Entries.ElementAt(idx).getLOADER_ENTRY() ) {
      DBG("    Entry %zd : %ls%s\n", idx, MainMenu.Entries.ElementAt(idx).Title.wc_str(), MainMenu.Entries.ElementAt(idx).Hidden ? " (hidden)" : "");
    }else{
      DBG("    Entry %zd : %ls%s\n", idx, MainMenu.Entries.ElementAt(idx).Title.wc_str(), MainMenu.Entries.ElementAt(idx).Hidden ? " (hidden)" : "");
    }
  }

  // Hide redundant preboot partition
  for (size_t entryIdx1 = 0; entryIdx1 < MainMenu.Entries.sizeIncludingHidden(); entryIdx1++)
  {
    LOADER_ENTRY* loaderEntry1Ptr = MainMenu.Entries.ElementAt(entryIdx1).getLOADER_ENTRY();
    if ( !loaderEntry1Ptr ) continue;
    LOADER_ENTRY& loaderEntry1 = *loaderEntry1Ptr;

    if ( ( loaderEntry1.LoaderType == OSTYPE_OSX || loaderEntry1.LoaderType == OSTYPE_OSX_INSTALLER )  &&  loaderEntry1.APFSTargetUUID.notEmpty() )
    {
      size_t entryIdx2 = MainMenu.Entries.getApfsLoaderIdx(loaderEntry1.Volume->ApfsContainerUUID, loaderEntry1.APFSTargetUUID, loaderEntry1.LoaderType);
      if ( entryIdx2 != SIZE_T_MAX ) {
        DBG("Hiding entry %zd because of entry %zd\n", entryIdx1, entryIdx2);
        loaderEntry1.Hidden = true;
      }
    }
  }



  typedef struct EntryIdx {
    size_t idx;
    REFIT_ABSTRACT_MENU_ENTRY* entry;
    EntryIdx(size_t _idx, REFIT_ABSTRACT_MENU_ENTRY* _entry) : idx(_idx), entry(_entry) {};
  } EntryIdx;

  XObjArray<EntryIdx> EntriesArrayTmp;

  for (size_t idx = 0; idx < MainMenu.Entries.sizeIncludingHidden(); idx++) {
    if ( MainMenu.Entries.ElementAt(idx).getLOADER_ENTRY() ) {
      if ( MainMenu.Entries.ElementAt(idx).getLOADER_ENTRY()->APFSTargetUUID.notEmpty() ) {
//        DBG("Add in EntriesArrayTmp at index %zd  Entry %zd : %ls\n", EntriesArrayTmp.size(), idx, MainMenu.Entries.ElementAt(idx).Title.wc_str());
        EntriesArrayTmp.AddReference(new EntryIdx(idx, &MainMenu.Entries.ElementAt(idx)), true);
      }
    }
  }

  bool hasMovedSomething;

  // Re-order preboot partition
  do {
    hasMovedSomething = false;
    for (size_t idx = 0; !hasMovedSomething && idx < EntriesArrayTmp.size(); )
    {
      LOADER_ENTRY* loaderEntry1Ptr = EntriesArrayTmp.ElementAt(idx).entry->getLOADER_ENTRY();
      if ( !loaderEntry1Ptr ) {
        EntriesArrayTmp.RemoveAtIndex(idx);
        // do not increment idx
        continue;
      }
      LOADER_ENTRY& loaderEntry1 = *loaderEntry1Ptr;

      if ( loaderEntry1.LoaderType == OSTYPE_OSX  &&  (loaderEntry1.Volume->ApfsRole & APPLE_APFS_VOLUME_ROLE_PREBOOT) != 0 )
      {
        size_t prebootIdx = MainMenu.Entries.getIdx(loaderEntry1Ptr);
        if ( prebootIdx == SIZE_T_MAX ) {
        	log_technical_bug("%s : prebootIdx == SIZE_T_MAX", __PRETTY_FUNCTION__);
        }else{
          size_t idxMain = MainMenu.Entries.getApfsLoaderIdx(loaderEntry1.Volume->ApfsContainerUUID, loaderEntry1.APFSTargetUUID, OSTYPE_OSX);
          if ( idxMain != SIZE_T_MAX && idxMain != prebootIdx+1 ) {
            DBG("Move preboot entry %zu before system %zu\n", prebootIdx, idxMain);
            MainMenu.Entries.moveBefore(prebootIdx, idxMain); // this will move preboot entry just before main
            EntriesArrayTmp.RemoveAtIndex(idx);
            hasMovedSomething = true;
          }
        }
      }
      ++idx;
    }
  } while ( hasMovedSomething );

  // Re-order installer partition
  do {
    hasMovedSomething = false;
    for (size_t idx = 0; !hasMovedSomething && idx < EntriesArrayTmp.size(); )
    {
      LOADER_ENTRY* loaderEntry1Ptr = EntriesArrayTmp.ElementAt(idx).entry->getLOADER_ENTRY();
      if ( !loaderEntry1Ptr ) {
        EntriesArrayTmp.RemoveAtIndex(idx);
        // do not increment idx
        continue;
      }
      LOADER_ENTRY& loaderEntry1 = *loaderEntry1Ptr;

      if ( loaderEntry1.LoaderType == OSTYPE_OSX_INSTALLER )
      {
        size_t installerIdx = MainMenu.Entries.getIdx(loaderEntry1Ptr);
        if ( installerIdx == SIZE_T_MAX ) {
          log_technical_bug("%s : installerIdx == SIZE_T_MAX", __PRETTY_FUNCTION__);
        }else{
          size_t idxPreboot = MainMenu.Entries.getApfsPrebootLoaderIdx(loaderEntry1.Volume->ApfsContainerUUID, loaderEntry1.APFSTargetUUID, OSTYPE_OSX);
          if ( idxPreboot != SIZE_T_MAX ) {
            if ( idxPreboot != installerIdx + 1 ) {
              DBG("Move installer entry %zu before preboot %zu\n", EntriesArrayTmp.ElementAt(idx).idx, idxPreboot);
              MainMenu.Entries.moveBefore(installerIdx, idxPreboot); // this will move preboot entry just before main
              EntriesArrayTmp.RemoveAtIndex(idx);
              hasMovedSomething = true;
            }
          }else{
            size_t idxMain = MainMenu.Entries.getApfsLoaderIdx(loaderEntry1.Volume->ApfsContainerUUID, loaderEntry1.APFSTargetUUID, OSTYPE_OSX);
            if ( idxMain != SIZE_T_MAX ) {
              if ( idxMain != installerIdx+1 ) {
                DBG("Move installer entry %zu before system %zu\n", EntriesArrayTmp.ElementAt(idx).idx, idxMain);
                MainMenu.Entries.moveBefore(installerIdx, idxMain); // this will move preboot entry just before main
                EntriesArrayTmp.RemoveAtIndex(idx);
                hasMovedSomething = true;
              }
            }
          }
        }
      }
      ++idx;
    }
  } while ( hasMovedSomething );

  // Re-order recovery partition
  do {
    hasMovedSomething = false;
    for (size_t idx = 0; !hasMovedSomething && idx < EntriesArrayTmp.size(); )
    {
      LOADER_ENTRY* loaderEntry1Ptr = EntriesArrayTmp.ElementAt(idx).entry->getLOADER_ENTRY();
      if ( !loaderEntry1Ptr ) {
        EntriesArrayTmp.RemoveAtIndex(idx);
        // do not increment idx
        continue;
      }
      LOADER_ENTRY& loaderEntry1 = *loaderEntry1Ptr;

      if ( loaderEntry1.LoaderType == OSTYPE_RECOVERY  &&  (loaderEntry1.Volume->ApfsRole & APPLE_APFS_VOLUME_ROLE_RECOVERY) != 0 )
      {
        size_t recoveryIdx = MainMenu.Entries.getIdx(loaderEntry1Ptr);
        if ( recoveryIdx == SIZE_T_MAX ) {
          log_technical_bug("%s : recoveryIdx == SIZE_T_MAX", __PRETTY_FUNCTION__);
        }else{
          size_t idxMain = MainMenu.Entries.getApfsLoaderIdx(loaderEntry1.Volume->ApfsContainerUUID, loaderEntry1.APFSTargetUUID, OSTYPE_OSX);
          if ( idxMain != SIZE_T_MAX ) {
            if ( idxMain + 1 != recoveryIdx ) {
              DBG("Move recovery entry %zu after system %zu\n", EntriesArrayTmp.ElementAt(idx).idx, idxMain);
              MainMenu.Entries.moveAfter(recoveryIdx, idxMain); // this will move preboot entry just before main
              EntriesArrayTmp.RemoveAtIndex(idx);
              hasMovedSomething = true;
            }
          }else{
            size_t idxPreboot = MainMenu.Entries.getApfsPrebootLoaderIdx(loaderEntry1.Volume->ApfsContainerUUID, loaderEntry1.APFSTargetUUID, OSTYPE_OSX);
            if ( idxPreboot != SIZE_T_MAX ) {
              if ( idxPreboot + 1 != recoveryIdx ) {
                DBG("Move recovery entry %zu after preboot %zu\n", EntriesArrayTmp.ElementAt(idx).idx, idxPreboot);
                MainMenu.Entries.moveAfter(recoveryIdx, idxPreboot); // this will move preboot entry just before main
                EntriesArrayTmp.RemoveAtIndex(idx);
                hasMovedSomething = true;
              }
            }
          }
        }
      }
      ++idx;
    }
  } while ( hasMovedSomething );


  DBG("Entries after before ordering\n");
  for (size_t idx = 0; idx < MainMenu.Entries.sizeIncludingHidden(); idx++) {
    if ( MainMenu.Entries.ElementAt(idx).getLOADER_ENTRY() ) {
      DBG("  Entry %zd : %ls%s\n", idx, MainMenu.Entries.ElementAt(idx).Title.wc_str(), MainMenu.Entries.ElementAt(idx).Hidden ? " (hidden)" : "");
    }else{
      DBG("  Entry %zd : %ls%s\n", idx, MainMenu.Entries.ElementAt(idx).Title.wc_str(), MainMenu.Entries.ElementAt(idx).Hidden ? " (hidden)" : "");
    }
  }

}

STATIC void AddCustomSubEntry(REFIT_VOLUME   *Volume,
                           IN UINTN                       CustomIndex,
                           IN const XStringW&             CustomPath,
                           UINT8 parentType,
                           IN const CUSTOM_LOADER_SUBENTRY&  Custom,
                           IN const XStringW&             DefaultEntrySettings,
                           IN REFIT_MENU_SCREEN          *SubMenu)
{
//  UINTN           VolumeIndex;
//  REFIT_VOLUME   *Volume;
//  REFIT_DIR_ITER  SIter;
//  REFIT_DIR_ITER *Iter = &SIter;
//  CHAR16          PartUUID[40];
//  XStringW        CustomPath = _CustomPath;

  if ( CustomPath.isEmpty() ) panic("BUG : CustomPath is empty");
  if ( SubMenu == NULL ) panic("BUG : this must be a sub entry");

//  if (FindCustomPath && (Custom.settings.Type != OSTYPE_LINEFI) && (Custom.settings.Type != OSTYPE_LIN)) {
////    DBG("Custom %lsentry %llu skipped because it didn't have a ", IsSubEntry ? L"sub " : L"", CustomIndex);
////    if (Custom.Type == 0) {
////      DBG("Type.\n");
////    } else {
////      DBG("Path.\n");
////    }
//    return;
//  }

  if ( Custom.settings.Disabled ) {
//    DBG("Custom %lsentry %llu skipped because it is disabled.\n", IsSubEntry ? L"sub " : L"", CustomIndex);
    return;
  }

//  if (!gSettings.ShowHiddenEntries && OSFLAG_ISSET(Custom.settings.Flags, OSFLAG_HIDDEN)) {
//    DBG("Custom %lsentry %llu skipped because it is hidden.\n", IsSubEntry ? L"sub " : L"", CustomIndex);
//    return;
//  }

#if 0  //if someone want to debug this
  DBG("Custom %lsentry %llu ", IsSubEntry ? L"sub " : L"", CustomIndex);
  //  if (Custom.settings.Title) {
  DBG("Title:\"%ls\" ", Custom.settings.Title.wc_str());
  //  }
  //  if (Custom.settings.FullTitle) {
  DBG("FullTitle:\"%ls\" ", Custom.settings.FullTitle.wc_str());
  //  }
  if (CustomPath) {
    DBG("Path:\"%ls\" ", CustomPath);
  }
  if (Custom.settings.Options != NULL) {
    DBG("Options:\"%ls\" ", Custom.settings.Options);
  }
  DBG("Type:%d Flags:0x%hhX matching ", Custom.settings.Type, Custom.settings.Flags);
  if (Custom.settings.Volume) {
    DBG("Volume:\"%ls\"\n", Custom.settings.Volume);
  } else {
    DBG("all volumes\n");
  }
#endif

//  for (VolumeIndex = 0; VolumeIndex < Volumes.size(); ++VolumeIndex) {
    LOADER_ENTRY        *Entry = NULL;

//    EFI_GUID            *Guid = NULL;
//    UINT64               VolumeSize;

//    Volume = &Volumes[VolumeIndex];
    if ((Volume == NULL) || (Volume->RootDir == NULL)) {
      return;
    }
    if (Volume->VolName.isEmpty()) {
      Volume->VolName = L"Unknown"_XSW;
    }

//    do { // when not scanning for kernels, this loop will execute only once
      XString8Array CustomOptions = Custom.getLoadOptions();

      UINT8 newCustomFlags = Custom.getFlags(gSettings.SystemParameters.NoCaches);

      // Create an entry for this volume
      Entry = CreateLoaderEntry(CustomPath, CustomOptions, Custom.getFullTitle(), Custom.getTitle(), Volume,
                                NULL, NULL,
                                parentType, newCustomFlags, 0, {0,0,0,0}, 0, NullXImage,
                                /*(KERNEL_AND_KEXT_PATCHES *)(((UINTN)Custom) + OFFSET_OF(CUSTOM_LOADER_ENTRY, KernelAndKextPatches))*/ NULL, TRUE);
      if (Entry != NULL) {
//        if ( Custom.settings.Settings.notEmpty() ) DBG("Custom settings: %ls.plist will %s be applied\n", Custom.settings.Settings.wc_str(), Custom.settings.CommonSettings?"not":"");
//        if (!Custom.settings.CommonSettings) {
//          Entry->Settings = DefaultEntrySettings;
//        }
        if (OSFLAG_ISUNSET(newCustomFlags, OSFLAG_NODEFAULTMENU)) {
          Entry->AddDefaultMenu();
//        } else if (Custom.SubEntries.notEmpty()) {
//          UINTN CustomSubIndex = 0;
//          // Add subscreen
//          REFIT_MENU_SCREEN *SubScreen = new REFIT_MENU_SCREEN;
//          SubScreen->Title.SWPrintf("Boot Options for %ls on %ls", (Custom.settings.Title.notEmpty()) ? Custom.settings.Title.wc_str() : CustomPath.wc_str(), Entry->DisplayedVolName.wc_str());
//          SubScreen->TitleImage = Entry->Image;
//          SubScreen->ID = Custom.settings.Type + 20;
//          SubScreen->GetAnime();
//          VolumeSize = RShiftU64(MultU64x32(Volume->BlockIO->Media->LastBlock, Volume->BlockIO->Media->BlockSize), 20);
//          SubScreen->AddMenuInfoLine_f("Volume size: %lldMb", VolumeSize);
//          SubScreen->AddMenuInfoLine_f("%ls", FileDevicePathToXStringW(Entry->DevicePath).wc_str());
//          if (Guid) {
//            SubScreen->AddMenuInfoLine_f("UUID: %s", strguid(Guid));
//          }
//          SubScreen->AddMenuInfoLine_f("Options: %s", Entry->LoadOptions.ConcatAll(" "_XS8).c_str());
//          DBG("Create sub entries\n");
//          for (size_t CustomSubEntryIndex = 0 ; CustomSubEntryIndex < Custom.SubEntries.size() ; ++CustomSubEntryIndex ) {
//            const CUSTOM_LOADER_SUBENTRY& CustomSubEntry = Custom.SubEntries[CustomSubEntryIndex];
//            if ( CustomSubEntry.settings.Settings.isEmpty() ) {
//              AddCustomSubEntry(Volume, CustomSubIndex++, CustomSubEntry.settings.Path.notEmpty() ? CustomSubEntry.settings.Path : CustomPath, CustomSubEntry, Custom.settings.Settings, SubScreen);
//            }else{
//              AddCustomSubEntry(Volume, CustomSubIndex++, CustomSubEntry.settings.Path.notEmpty() ? CustomSubEntry.settings.Path : CustomPath, CustomSubEntry, CustomSubEntry.settings.Settings, SubScreen);
//            }
//          }
//          SubScreen->AddMenuEntry(&MenuEntryReturn, true);
//          Entry->SubScreen = SubScreen;
        }
        SubMenu->AddMenuEntry(Entry, true);
//        Entry->Hidden = Custom.settings.Hidden;
//        if ( Custom.settings.Hidden ) DBG("     hiding entry because Custom.settings.Hidden\n");
      }
//    } while (FindCustomPath && Custom.settings.Type == OSTYPE_LINEFI && Custom.settings.KernelScan == KERNEL_SCAN_ALL); // repeat loop only for kernel scanning

//    // Close the kernel boot directory
//    if (FindCustomPath && Custom.settings.Type == OSTYPE_LINEFI) {
//      DirIterClose(Iter);
//    }
//  }

}

STATIC void AddCustomEntry(IN UINTN                       CustomIndex,
                           IN const XStringW&             _CustomPath,
                           IN const CUSTOM_LOADER_ENTRY&  Custom,
                           IN const XStringW&             DefaultEntrySettings,
                           IN REFIT_MENU_SCREEN          *SubMenu)
{
  UINTN           VolumeIndex;
  REFIT_VOLUME   *Volume;
  REFIT_DIR_ITER  SIter;
  REFIT_DIR_ITER *Iter = &SIter;
  CHAR16          PartUUID[40];
  XStringW        CustomPath = _CustomPath;
  BOOLEAN         FindCustomPath = (CustomPath.isEmpty());

  if ( SubMenu != NULL ) panic("Call AddCustomSubEntry instead");
  
  if (FindCustomPath && (Custom.settings.Type != OSTYPE_LINEFI) && (Custom.settings.Type != OSTYPE_LIN)) {
//    DBG("Custom %lsentry %llu skipped because it didn't have a ", IsSubEntry ? L"sub " : L"", CustomIndex);
//    if (Custom.Type == 0) {
//      DBG("Type.\n");
//    } else {
//      DBG("Path.\n");
//    }
    return;
  }

  if (OSFLAG_ISSET(Custom.getFlags(gSettings.SystemParameters.NoCaches), OSFLAG_DISABLED)) {
//    DBG("Custom %lsentry %llu skipped because it is disabled.\n", IsSubEntry ? L"sub " : L"", CustomIndex);
    return;
  }

//  if (!gSettings.ShowHiddenEntries && OSFLAG_ISSET(Custom.settings.Flags, OSFLAG_HIDDEN)) {
//    DBG("Custom %lsentry %llu skipped because it is hidden.\n", IsSubEntry ? L"sub " : L"", CustomIndex);
//    return;
//  }

#if 0  //if someone want to debug this
  DBG("Custom %lsentry %llu ", IsSubEntry ? L"sub " : L"", CustomIndex);
  //  if (Custom.settings.Title) {
  DBG("Title:\"%ls\" ", Custom.settings.Title.wc_str());
  //  }
  //  if (Custom.settings.FullTitle) {
  DBG("FullTitle:\"%ls\" ", Custom.settings.FullTitle.wc_str());
  //  }
  if (CustomPath) {
    DBG("Path:\"%ls\" ", CustomPath);
  }
  if (Custom.settings.Options != NULL) {
    DBG("Options:\"%ls\" ", Custom.settings.Options);
  }
  DBG("Type:%d Flags:0x%hhX matching ", Custom.settings.Type, Custom.settings.Flags);
  if (Custom.settings.Volume) {
    DBG("Volume:\"%ls\"\n", Custom.settings.Volume);
  } else {
    DBG("all volumes\n");
  }
#endif

  for (VolumeIndex = 0; VolumeIndex < Volumes.size(); ++VolumeIndex) {
    LOADER_ENTRY        *Entry = NULL;
    XIcon Image = Custom.Image;
    XIcon DriveImage = Custom.DriveImage;

    EFI_GUID            *Guid = NULL;
    UINT64               VolumeSize;

    Volume = &Volumes[VolumeIndex];
    if ((Volume == NULL) || (Volume->RootDir == NULL)) {
      continue;
    }
    if (Volume->VolName.isEmpty()) {
      Volume->VolName = L"Unknown"_XSW;
    }

    DBG("    Checking volume \"%ls\" (%ls) ... ", Volume->VolName.wc_str(), Volume->DevicePathString.wc_str());

    // skip volume if its kind is configured as disabled
    if (((1ull<<Volume->DiskKind) & GlobalConfig.DisableFlags) != 0) {
      DBG("skipped because media is disabled\n");
      continue;
    }

    if (Custom.settings.VolumeType != 0 && ((1<<Volume->DiskKind) & Custom.settings.VolumeType) == 0) {
      DBG("skipped because media is ignored\n");
      continue;
    }
    
    // Check the volume is readable and the entry exists on the volume
    if (Volume->RootDir == NULL) {
      DBG("skipped because filesystem is not readable\n");
      continue;
    }

    if (Volume->Hidden) {
      DBG("skipped because volume is hidden\n");
      continue;
    }

    
    // Check for exact volume matches (devicepath / volumelabel)
    if (Custom.settings.Volume.notEmpty()) {
      if ((StrStr(Volume->DevicePathString.wc_str(), Custom.settings.Volume.wc_str()) == NULL) &&
          ((Volume->VolName.isEmpty()) || (StrStr(Volume->VolName.wc_str(), Custom.settings.Volume.wc_str()) == NULL))) {
        bool CustomEntryFound = false;
        //..\VenMedia(BE74FCF7-0B7C-49F3-9147-01F4042E6842,E97E25EA28F4DF46AAD44CC3F12E28D3)
        EFI_DEVICE_PATH *MediaPath = Clover_FindDevicePathNodeWithType(Volume->DevicePath, MEDIA_DEVICE_PATH, MEDIA_VENDOR_DP);
        if (MediaPath) {
          EFI_GUID *MediaPathGuid = (EFI_GUID *)&((VENDOR_DEVICE_PATH_WITH_DATA*)MediaPath)->VendorDefinedData;
          XStringW MediaPathGuidStr = GuidLEToXStringW(*MediaPathGuid);
          //       DBG("  checking '%ls'\n", MediaPathGuidStr.wc_str());
          if (StrStr(Custom.settings.Volume.wc_str(), MediaPathGuidStr.wc_str())) {
            DBG("   - found entry for volume '%ls', '%ls'\n", Custom.settings.Volume.wc_str(), MediaPathGuidStr.wc_str());
            CustomEntryFound = true;
          } else {
            DBG("  - search volume '%ls', but MediaPath '%ls' \n", Custom.settings.Volume.wc_str(), MediaPathGuidStr.wc_str());
          }
        }
        if (!CustomEntryFound) {
          DBG("skipped because volume does not match\n");
          continue;
        }
      }
    }


    Guid = FindGPTPartitionGuidInDevicePath(Volume->DevicePath);
    if (FindCustomPath) {
      // Get the partition UUID and make sure it's lower case
      if (Guid == NULL) {
        DBG("skipped because volume does not have partition uuid\n");
        continue;
      }
      snwprintf(PartUUID, sizeof(PartUUID), "%s", strguid(Guid));
      StrToLower(PartUUID);

      // search for standard/nonstandard linux uefi paths, and all kernel scan options that != KERNEL_SCAN_ALL
      if (Custom.settings.Type == OSTYPE_LIN || Custom.settings.KernelScan != KERNEL_SCAN_ALL) {
        LinuxScan(Volume, Custom.settings.KernelScan, Custom.settings.Type, &CustomPath, &Image);
      }
      if (Custom.settings.Type == OSTYPE_LINEFI) {
        // Open the boot directory to determine linux loadoptions when found item, or kernels when KERNEL_SCAN_ALL
        DirIterOpen(Volume->RootDir, LINUX_BOOT_PATH, Iter);
      }
    } else if (!FileExists(Volume->RootDir, CustomPath)) {
      DBG("skipped because path '%ls' does not exist\n", CustomPath.wc_str());
      continue;
    }

    // Change to custom image if needed
    if (Image.isEmpty() && Custom.settings.dgetImagePath().notEmpty()) {
      Image.LoadXImage(&ThemeX.getThemeDir(), Custom.settings.dgetImagePath());
      if (Image.isEmpty()) {
        Image.LoadXImage(&ThemeX.getThemeDir(), L"os_"_XSW + Custom.settings.dgetImagePath());
        if (Image.isEmpty()) {
          Image.LoadXImage(&self.getCloverDir(), Custom.settings.dgetImagePath());
          if (Image.isEmpty()) {
            Image.LoadXImage(&self.getSelfVolumeRootDir(), Custom.settings.dgetImagePath());
            if (Image.isEmpty()) {
              Image.LoadXImage(Volume->RootDir, Custom.settings.dgetImagePath());
            }
          }
        }
      }
    }

    // Change to custom drive image if needed
    if (DriveImage.isEmpty() && Custom.settings.dgetDriveImagePath().notEmpty()) {
      DriveImage.LoadXImage(&ThemeX.getThemeDir(), Custom.settings.dgetDriveImagePath());
      if (DriveImage.isEmpty()) {
        DriveImage.LoadXImage(&self.getCloverDir(), Custom.settings.dgetImagePath());
        if (DriveImage.isEmpty()) {
          DriveImage.LoadXImage(&self.getSelfVolumeRootDir(), Custom.settings.dgetImagePath());
          if (DriveImage.isEmpty()) {
            DriveImage.LoadXImage(Volume->RootDir, Custom.settings.dgetImagePath());
          }
        }
      }
    }

    do { // when not scanning for kernels, this loop will execute only once
      XString8Array CustomOptions = Custom.getLoadOptions();

      // for LINEFI with option KERNEL_SCAN_ALL, use this loop to search for kernels
      if (FindCustomPath && Custom.settings.Type == OSTYPE_LINEFI && Custom.settings.KernelScan == KERNEL_SCAN_ALL) {
        EFI_FILE_INFO *FileInfo = NULL;
        // Get the next kernel path or stop looking
        if (!DirIterNext(Iter, 2, LINUX_LOADER_SEARCH_PATH, &FileInfo) || (FileInfo == NULL)) {
          DBG("\n");
          break;
        }
        // who knows....
        if (FileInfo->FileSize == 0) {
          continue;
        }
        // get the kernel file path
        CustomPath.SWPrintf("%ls\\%ls", LINUX_BOOT_PATH, FileInfo->FileName);
      }
      if (CustomPath.isEmpty()) {
        DBG("skipped\n");
        break;
      }

      UINT8 newCustomFlags = Custom.getFlags(gSettings.SystemParameters.NoCaches);

      // Check to make sure if we should update linux custom options or not
      if (FindCustomPath && Custom.settings.Type == OSTYPE_LINEFI && OSFLAG_ISUNSET(Custom.getFlags(gSettings.SystemParameters.NoCaches), OSFLAG_NODEFAULTARGS)) {
        // Find the init ram image and select root
        CustomOptions = LinuxKernelOptions(Iter->DirHandle, Basename(CustomPath.wc_str()) + LINUX_LOADER_PATH.length(), PartUUID, Custom.getLoadOptions());
        newCustomFlags = OSFLAG_SET(Custom.getFlags(gSettings.SystemParameters.NoCaches), OSFLAG_NODEFAULTARGS);
      }

      // Check to make sure that this entry is not hidden or disabled by another custom entry
      if (true) {
        BOOLEAN              BetterMatch = FALSE;
        for (size_t i = 0 ; i < GlobalConfig.CustomEntries.size() ; ++i ) {
          CUSTOM_LOADER_ENTRY& CustomEntry = GlobalConfig.CustomEntries[i];
          if ( CustomEntry.settings.Disabled ) continue; // before, disabled entries settings weren't loaded.
          // Don't match against this custom
          if (&CustomEntry == &Custom) {
            continue;
          }
          // Can only match the same types
          if (Custom.settings.Type != CustomEntry.settings.Type) {
            continue;
          }
          // Check if the volume string matches
          if (Custom.settings.Volume != CustomEntry.settings.Volume) {
            if (CustomEntry.settings.Volume.isEmpty()) {
              // Less precise volume match
              if (Custom.settings.Path != CustomEntry.settings.Path) {
                // Better path match
                BetterMatch = ((CustomEntry.settings.Path.notEmpty()) && CustomPath.isEqual(CustomEntry.settings.Path) &&
                               ((Custom.settings.VolumeType == CustomEntry.settings.VolumeType) ||
                                ((1ull<<Volume->DiskKind) & Custom.settings.VolumeType) != 0));
              }
            } else if ((StrStr(Volume->DevicePathString.wc_str(), Custom.settings.Volume.wc_str()) == NULL) &&
                       ((Volume->VolName.isEmpty()) || (StrStr(Volume->VolName.wc_str(), Custom.settings.Volume.wc_str()) == NULL))) {
              if (Custom.settings.Volume.isEmpty()) {
                // More precise volume match
                if (Custom.settings.Path != CustomEntry.settings.Path) {
                  // Better path match
                  BetterMatch = ((CustomEntry.settings.Path.notEmpty()) && CustomPath.isEqual(CustomEntry.settings.Path) &&
                                 ((Custom.settings.VolumeType == CustomEntry.settings.VolumeType) ||
                                  ((1ull<<Volume->DiskKind) & Custom.settings.VolumeType) != 0));
                } else if (Custom.settings.VolumeType != CustomEntry.settings.VolumeType) {
                  // More precise volume type match
                  BetterMatch = ((Custom.settings.VolumeType == 0) &&
                                 ((1ull<<Volume->DiskKind) & Custom.settings.VolumeType) != 0);
                } else {
                  // Better match
                  BetterMatch = TRUE;
                }
              // Duplicate volume match
              } else if (Custom.settings.Path != CustomEntry.settings.Path) {
                // Better path match
                BetterMatch = ((CustomEntry.settings.Path.notEmpty()) && CustomPath.isEqual(CustomEntry.settings.Path) &&
                               ((Custom.settings.VolumeType == CustomEntry.settings.VolumeType) ||
                                ((1ull<<Volume->DiskKind) & Custom.settings.VolumeType) != 0));
              // Duplicate path match
              } else if (Custom.settings.VolumeType != CustomEntry.settings.VolumeType) {
                // More precise volume type match
                BetterMatch = ((Custom.settings.VolumeType == 0) &&
                               ((1ull<<Volume->DiskKind) & Custom.settings.VolumeType) != 0);
              } else {
                // Duplicate entry
                BetterMatch = (i <= CustomIndex);
              }
            }
          // Duplicate volume match
          } else if (Custom.settings.Path != CustomEntry.settings.Path) {
            if (CustomEntry.settings.Path.isEmpty()) {
              // Less precise path match
              BetterMatch = ((Custom.settings.VolumeType != CustomEntry.settings.VolumeType) &&
                             ((1ull<<Volume->DiskKind) & Custom.settings.VolumeType) != 0);
            } else if (CustomPath.isEqual(CustomEntry.settings.Path)) {
              if (Custom.settings.Path.isEmpty()) {
                // More precise path and volume type match
                BetterMatch = ((Custom.settings.VolumeType == CustomEntry.settings.VolumeType) ||
                               ((1ull<<Volume->DiskKind) & Custom.settings.VolumeType) != 0);
              } else if (Custom.settings.VolumeType != CustomEntry.settings.VolumeType) {
                // More precise volume type match
                BetterMatch = ((Custom.settings.VolumeType == 0) &&
                               ((1ull<<Volume->DiskKind) & Custom.settings.VolumeType) != 0);
              } else {
                // Duplicate entry
                BetterMatch = (i <= CustomIndex);
              }
            }
          // Duplicate path match
          } else if (Custom.settings.VolumeType != CustomEntry.settings.VolumeType) {
            // More precise volume type match
            BetterMatch = ((Custom.settings.VolumeType == 0) &&
                           ((1ull<<Volume->DiskKind) & Custom.settings.VolumeType) != 0);
          } else {
            // Duplicate entry
            BetterMatch = (i <= CustomIndex);
          }
          if (BetterMatch) {
            DBG("skipped because custom entry %zu is a better match and will produce a duplicate entry\n", i);
            break;
          }
        }
        if (BetterMatch) {
          continue;
        }
      }

      DBG("match!\n");
      // Create an entry for this volume
      Entry = CreateLoaderEntry(CustomPath, CustomOptions, Custom.settings.FullTitle, Custom.settings.dgetTitle(), Volume,
                                (Image.isEmpty() ? NULL : &Image), (DriveImage.isEmpty() ? NULL : &DriveImage),            
                                Custom.settings.Type, newCustomFlags, Custom.settings.Hotkey, Custom.settings.BootBgColor, Custom.CustomLogoType, Custom.CustomLogoImage,
                                /*(KERNEL_AND_KEXT_PATCHES *)(((UINTN)Custom) + OFFSET_OF(CUSTOM_LOADER_ENTRY, KernelAndKextPatches))*/ NULL, TRUE);
      if (Entry != NULL) {
        if ( Custom.settings.Settings.notEmpty() ) {
          DBG("Custom settings: %ls.plist will %s be applied\n", Custom.settings.Settings.wc_str(), Custom.settings.CommonSettings?"not":"");
        }
        if (!Custom.settings.CommonSettings) {
          Entry->Settings = DefaultEntrySettings;
        }
        if (Custom.settings.ForceTextMode) {
          Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_USEGRAPHICS);
        }
        if (OSFLAG_ISUNSET(newCustomFlags, OSFLAG_NODEFAULTMENU)) {
          Entry->AddDefaultMenu();
        } else if (Custom.SubEntries.notEmpty()) {
          UINTN CustomSubIndex = 0;
          // Add subscreen
          REFIT_MENU_SCREEN *SubScreen = new REFIT_MENU_SCREEN;
          SubScreen->Title.SWPrintf("Boot Options for %ls on %ls", (Custom.settings.dgetTitle().notEmpty()) ? XStringW(Custom.settings.dgetTitle()).wc_str() : CustomPath.wc_str(), Entry->DisplayedVolName.wc_str());
          SubScreen->TitleImage = Entry->Image;
          SubScreen->ID = Custom.settings.Type + 20;
          SubScreen->GetAnime();
          VolumeSize = RShiftU64(MultU64x32(Volume->BlockIO->Media->LastBlock, Volume->BlockIO->Media->BlockSize), 20);
          SubScreen->AddMenuInfoLine_f("Volume size: %lldMb", VolumeSize);
          SubScreen->AddMenuInfoLine_f("%ls", FileDevicePathToXStringW(Entry->DevicePath).wc_str());
          if (Guid) {
            SubScreen->AddMenuInfoLine_f("UUID: %s", strguid(Guid));
          }
          SubScreen->AddMenuInfoLine_f("Options: %s", Entry->LoadOptions.ConcatAll(" "_XS8).c_str());
          DBG("Create sub entries\n");
          for (size_t CustomSubEntryIndex = 0 ; CustomSubEntryIndex < Custom.SubEntries.size() ; ++CustomSubEntryIndex ) {
            const CUSTOM_LOADER_SUBENTRY& CustomSubEntry = Custom.SubEntries[CustomSubEntryIndex];
//            if ( CustomSubEntry.settings.Settings.isEmpty() ) {
              AddCustomSubEntry(Volume, CustomSubIndex++, Custom.settings.Path.notEmpty() ? Custom.settings.Path : CustomPath, Custom.settings.Type, CustomSubEntry, Custom.settings.Settings, SubScreen);
//            }else{
//              AddCustomSubEntry(Volume, CustomSubIndex++, CustomSubEntry.settings.Path.notEmpty() ? CustomSubEntry.settings.Path : CustomPath, CustomSubEntry, CustomSubEntry.settings.Settings, SubScreen);
//            }
          }
          SubScreen->AddMenuEntry(&MenuEntryReturn, true);
          Entry->SubScreen = SubScreen;
        }
//        if (IsSubEntry)
//          SubMenu->AddMenuEntry(Entry, true);
//        else
          MainMenu.AddMenuEntry(Entry, true);

        Entry->Hidden = Custom.settings.Hidden;
        if ( Custom.settings.Hidden ) DBG("     hiding entry because Custom.settings.Hidden\n");
      }
    } while (FindCustomPath && Custom.settings.Type == OSTYPE_LINEFI && Custom.settings.KernelScan == KERNEL_SCAN_ALL); // repeat loop only for kernel scanning

    // Close the kernel boot directory
    if (FindCustomPath && Custom.settings.Type == OSTYPE_LINEFI) {
      DirIterClose(Iter);
    }
  }

}

// Add custom entries
void AddCustomEntries(void)
{
  if (GlobalConfig.CustomEntries.isEmpty()) return;

  //DBG("Custom entries start\n");
  DbgHeader("AddCustomEntries");
  // Traverse the custom entries
  for (size_t i = 0 ; i < GlobalConfig.CustomEntries.size(); ++i) {
    CUSTOM_LOADER_ENTRY& Custom = GlobalConfig.CustomEntries[i];
    DBG("- [00]: '%s'\n", Custom.settings.FullTitle.isEmpty() ? Custom.settings.dgetTitle().c_str() : Custom.settings.FullTitle.c_str() );
    if ( Custom.settings.Disabled ) {
      DBG("  Disabled\n");
      continue; // before, disabled entries settings weren't loaded.
    }
    if ((Custom.settings.Path.isEmpty()) && (Custom.settings.Type != 0)) {
      if (OSTYPE_IS_OSX(Custom.settings.Type)) {
        AddCustomEntry(i, MACOSX_LOADER_PATH, Custom, Custom.settings.Settings, NULL);
      } else if (OSTYPE_IS_OSX_RECOVERY(Custom.settings.Type)) {
        AddCustomEntry(i, L"\\com.apple.recovery.boot\\boot.efi"_XSW, Custom, Custom.settings.Settings, NULL);
      } else if (OSTYPE_IS_OSX_INSTALLER(Custom.settings.Type)) {
        UINTN Index = 0;
        while (Index < OSXInstallerPathsCount) {
          AddCustomEntry(i, OSXInstallerPaths[Index++], Custom, Custom.settings.Settings, NULL);
        }
      } else if (OSTYPE_IS_WINDOWS(Custom.settings.Type)) {
        AddCustomEntry(i, L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi"_XSW, Custom, Custom.settings.Settings, NULL);
      } else if (OSTYPE_IS_LINUX(Custom.settings.Type)) {
#if defined(ANDX86)
        for (UINTN Index = 0; Index < AndroidEntryDataCount; ++Index) {
          AddCustomEntry(i, AndroidEntryData[Index].Path, Custom, Custom.settings.Settings, NULL);
        }
#endif
        AddCustomEntry(i, NullXStringW, Custom, Custom.settings.Settings, NULL);
      } else if (Custom.settings.Type == OSTYPE_LINEFI) {
        AddCustomEntry(i, NullXStringW, Custom, Custom.settings.Settings, NULL);
      } else {
        AddCustomEntry(i, BOOT_LOADER_PATH, Custom, Custom.settings.Settings, NULL);
      }
    } else {
      AddCustomEntry(i, Custom.settings.Path, Custom, Custom.settings.Settings, NULL);
    }
  }
  //DBG("Custom entries finish\n");
}
