/*
 * refit/main.c
 * Main code for the boot menu
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

#include "lib.h"
#include "Handle.h"

#include "syslinux_mbr.h"

// types

typedef struct {
    REFIT_MENU_ENTRY me;
    CHAR16           *LoaderPath;
    CHAR16           *VolName;
    EFI_DEVICE_PATH  *DevicePath;
    BOOLEAN          UseGraphicsMode;
    CHAR16           *LoadOptions;
} LOADER_ENTRY;

typedef struct {
    REFIT_MENU_ENTRY me;
    REFIT_VOLUME     *Volume;
    CHAR16           *LoadOptions;
} LEGACY_ENTRY;

// variables

#define MACOSX_LOADER_PATH      L"\\System\\Library\\CoreServices\\boot.efi"

#define TAG_ABOUT    (1)
#define TAG_RESET    (2)
#define TAG_SHUTDOWN (3)
#define TAG_TOOL     (4)
#define TAG_LOADER   (5)
#define TAG_LEGACY   (6)

EFI_HANDLE					gImageHandle;
EFI_SYSTEM_TABLE*			gST;
EFI_BOOT_SERVICES*			gBS; 
EFI_RUNTIME_SERVICES*		gRT;
EFI_DXE_SERVICES*			gDS;


static REFIT_MENU_ENTRY MenuEntryAbout    = { L"About rEFIt", TAG_ABOUT, 1, 0, 'A', NULL, NULL, NULL };
static REFIT_MENU_ENTRY MenuEntryReset    = { L"Restart Computer", TAG_RESET, 1, 0, 'R', NULL, NULL, NULL };
static REFIT_MENU_ENTRY MenuEntryShutdown = { L"Shut Down Computer", TAG_SHUTDOWN, 1, 0, 'U', NULL, NULL, NULL };
static REFIT_MENU_ENTRY MenuEntryReturn   = { L"Return to Main Menu", TAG_RETURN, 0, 0, 0, NULL, NULL, NULL };

static REFIT_MENU_SCREEN MainMenu       = { L"Main Menu", NULL, 0, NULL, 0, NULL, 0, L"Automatic boot" };
static REFIT_MENU_SCREEN AboutMenu      = { L"About", NULL, 0, NULL, 0, NULL, 0, NULL };

/**
 Concatenates a formatted unicode string to allocated pool. The caller must
 free the resulting buffer.
 
 @param Str             Tracks the allocated pool, size in use, and
 amount of pool allocated.
 @param Fmt             The format string
 @param ...             Variable arguments based on the format string.
 
 @return Allocated buffer with the formatted string printed in it.
 The caller must free the allocated buffer. The buffer
 allocation is not packed.
 
 **/
/*
CHAR16 *
EFIAPI
CatPrint (
		  IN OUT POOL_PRINT   *Str,
		  IN CHAR16           *Fmt,
		  ...
		  )
{
	UINT16  *AppendStr;
	VA_LIST Args;
	UINTN   Size;
	
	AppendStr = AllocateZeroPool (0x1000);
	if (AppendStr == NULL) {
		return Str->Str;
	}
	
	VA_START (Args, Fmt);
	UnicodeVSPrint (AppendStr, 0x1000, Fmt, Args);
	VA_END (Args);
	if (NULL == Str->Str) {
		Size   = StrLen (AppendStr);
		Str->Str  = AllocateZeroPool (Size);
		ASSERT (Str->Str != NULL);
	} else {
		Size = StrLen (AppendStr) - sizeof (UINT16);
		Size = Size + StrLen (Str->Str);
		Str->Str =  EfiReallocatePool(
								   StrLen (Str->Str),
								   Size,
								   Str->Str
								   );
		ASSERT (Str->Str != NULL);
	}
	
	Str->MaxLen = MAX_CHAR * sizeof (UINT16);
	if (Size < Str->MaxLen) {
		StrCat (Str->Str, AppendStr);
		Str->Len = Size - sizeof (UINT16);
	}
	
	FreePool (AppendStr);
	return Str->Str;
}
*/


static VOID AboutRefit(VOID)
{
    if (AboutMenu.EntryCount == 0) {
        AboutMenu.TitleImage = BuiltinIcon(BUILTIN_ICON_FUNC_ABOUT);
        AddMenuInfoLine(&AboutMenu, L"rEFIt Version 0.16 UEFI");
        AddMenuInfoLine(&AboutMenu, L"");
        AddMenuInfoLine(&AboutMenu, L"Copyright (c) 2006-2010 Christoph Pfisterer");
        AddMenuInfoLine(&AboutMenu, L"Portions Copyright (c) Intel Corporation and others");
        AddMenuInfoLine(&AboutMenu, L"");
        AddMenuInfoLine(&AboutMenu, L"Running on:");
        AddMenuInfoLine(&AboutMenu, PoolPrint(L" EFI Revision %d.%02d",
            gST->Hdr.Revision >> 16, gST->Hdr.Revision & ((1 << 16) - 1)));
#if defined(EFI32)
        AddMenuInfoLine(&AboutMenu, L" Platform: i386 (32 bit)");
#elif defined(EFIX64)
        AddMenuInfoLine(&AboutMenu, L" Platform: x86_64 (64 bit)");
#else
        AddMenuInfoLine(&AboutMenu, L" Platform: unknown");
#endif
        AddMenuInfoLine(&AboutMenu, PoolPrint(L" Firmware: %s %d.%02d",
            gST->FirmwareVendor, gST->FirmwareRevision >> 16, gST->FirmwareRevision & ((1 << 16) - 1)));
        AddMenuInfoLine(&AboutMenu, PoolPrint(L" Screen Output: %s", egScreenDescription()));
        AddMenuEntry(&AboutMenu, &MenuEntryReturn);
    }
    
    RunMenu(&AboutMenu, NULL);
}

static EFI_STATUS StartEFIImageList(IN EFI_DEVICE_PATH **DevicePaths,
                                    IN CHAR16 *LoadOptions, IN CHAR16 *LoadOptionsPrefix,
                                    IN CHAR16 *ImageTitle,
                                    OUT UINTN *ErrorInStep)
{
    EFI_STATUS              Status, ReturnStatus;
    EFI_HANDLE              ChildImageHandle;
    EFI_LOADED_IMAGE        *ChildLoadedImage;
    UINTN                   DevicePathIndex;
    CHAR16                  ErrorInfo[256];
    CHAR16                  *FullLoadOptions = NULL;
    
    Print(L"Starting %s\n", ImageTitle);
    if (ErrorInStep != NULL)
        *ErrorInStep = 0;
    
    // load the image into memory
    ReturnStatus = Status = EFI_NOT_FOUND;  // in case the list is empty
    for (DevicePathIndex = 0; DevicePaths[DevicePathIndex] != NULL; DevicePathIndex++) {
        ReturnStatus = Status = gBS->LoadImage(FALSE, SelfImageHandle, DevicePaths[DevicePathIndex], NULL, 0, &ChildImageHandle);
        if (ReturnStatus != EFI_NOT_FOUND)
            break;
    }
    SPrint(ErrorInfo, 255, L"while loading %s", ImageTitle);
    if (CheckError(Status, ErrorInfo)) {
        if (ErrorInStep != NULL)
            *ErrorInStep = 1;
        goto bailout;
    }
    
    // set load options
    if (LoadOptions != NULL) {
        ReturnStatus = Status = gBS->HandleProtocol(ChildImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &ChildLoadedImage);
        if (CheckError(Status, L"while getting a LoadedImageProtocol handle")) {
            if (ErrorInStep != NULL)
                *ErrorInStep = 2;
            goto bailout_unload;
        }
        
        if (LoadOptionsPrefix != NULL) {
            FullLoadOptions = PoolPrint(L"%s %s ", LoadOptionsPrefix, LoadOptions);
            // NOTE: That last space is also added by the EFI shell and seems to be significant
            //  when passing options to Apple's boot.efi...
            LoadOptions = FullLoadOptions;
        }
        // NOTE: We also include the terminating null in the length for safety.
        ChildLoadedImage->LoadOptions = (VOID *)LoadOptions;
        ChildLoadedImage->LoadOptionsSize = ((UINT32)StrLen(LoadOptions) + 1) * sizeof(CHAR16);
        Print(L"Using load options '%s'\n", LoadOptions);
    }
    
    // close open file handles
    UninitRefitLib();
    
    // turn control over to the image
    // TODO: (optionally) re-enable the EFI watchdog timer!
    ReturnStatus = Status = gBS->StartImage(ChildImageHandle, NULL, NULL);
    // control returns here when the child image calls Exit()
    SPrint(ErrorInfo, 255, L"returned from %s", ImageTitle);
    if (CheckError(Status, ErrorInfo)) {
        if (ErrorInStep != NULL)
            *ErrorInStep = 3;
    }
    
    // re-open file handles
    ReinitRefitLib();
    
bailout_unload:
    // unload the image, we don't care if it works or not...
    Status = gBS->UnloadImage(ChildImageHandle);
bailout:
    if (FullLoadOptions != NULL)
        FreePool(FullLoadOptions);
    return ReturnStatus;
}

static EFI_STATUS StartEFIImage(IN EFI_DEVICE_PATH *DevicePath,
                                IN CHAR16 *LoadOptions, IN CHAR16 *LoadOptionsPrefix,
                                IN CHAR16 *ImageTitle,
                                OUT UINTN *ErrorInStep)
{
    EFI_DEVICE_PATH *DevicePaths[2];
    
    DevicePaths[0] = DevicePath;
    DevicePaths[1] = NULL;
    return StartEFIImageList(DevicePaths, LoadOptions, LoadOptionsPrefix, ImageTitle, ErrorInStep);
}

//
// EFI OS loader functions
//

static VOID StartLoader(IN LOADER_ENTRY *Entry)
{
    BeginExternalScreen(Entry->UseGraphicsMode, L"Booting OS");
    StartEFIImage(Entry->DevicePath, Entry->LoadOptions,
                  Basename(Entry->LoaderPath), Basename(Entry->LoaderPath), NULL);
    FinishExternalScreen();
}

static LOADER_ENTRY * AddLoaderEntry(IN CHAR16 *LoaderPath, IN CHAR16 *LoaderTitle, IN REFIT_VOLUME *Volume)
{
    CHAR16          *FileName, *OSIconName;
    CHAR16          IconFileName[256];
    CHAR16          DiagsFileName[256];
    CHAR16          ShortcutLetter;
    UINTN           LoaderKind;
    LOADER_ENTRY    *Entry, *SubEntry;
    REFIT_MENU_SCREEN *SubScreen;
    
    FileName = Basename(LoaderPath);
    
    // prepare the menu entry
    Entry = AllocateZeroPool(sizeof(LOADER_ENTRY));
    Entry->me.Title        = PoolPrint(L"Boot %s from %s", (LoaderTitle != NULL) ? LoaderTitle : LoaderPath + 1, Volume->VolName);
    Entry->me.Tag          = TAG_LOADER;
    Entry->me.Row          = 0;
    if (GlobalConfig.HideBadges == 0 ||
        (GlobalConfig.HideBadges == 1 && Volume->DiskKind != DISK_KIND_INTERNAL))
        Entry->me.BadgeImage   = Volume->VolBadgeImage;
    Entry->LoaderPath      = EfiStrDuplicate(LoaderPath);
    Entry->VolName         = Volume->VolName;
    Entry->DevicePath      = FileDevicePath(Volume->DeviceHandle, Entry->LoaderPath);
    Entry->UseGraphicsMode = FALSE;
    
    // locate a custom icon for the loader
    StrCpy(IconFileName, LoaderPath);
    ReplaceExtension(IconFileName, L".icns");
  if (FileExists(Volume->RootDir, IconFileName)){
        Entry->me.Image = LoadIcns(Volume->RootDir, IconFileName, 128);
  } else if (FileExists(SelfRootDir, IconFileName)) {
    Entry->me.Image = LoadIcns(SelfRootDir, IconFileName, 128);
  }
    
    // detect specific loaders
    OSIconName = NULL;
    LoaderKind = 0;
    ShortcutLetter = 0;
    if (StriCmp(LoaderPath, MACOSX_LOADER_PATH) == 0) {
        OSIconName = L"mac";
        Entry->UseGraphicsMode = TRUE;
        LoaderKind = 1;
        ShortcutLetter = 'M';
    } else if (StriCmp(FileName, L"diags.efi") == 0) {
        OSIconName = L"hwtest";
    } else if (StriCmp(FileName, L"e.efi") == 0 ||
               StriCmp(FileName, L"elilo.efi") == 0) {
        OSIconName = L"elilo,linux";
        LoaderKind = 2;
        ShortcutLetter = 'L';
    } else if (StriCmp(FileName, L"cdboot.efi") == 0 ||
               StriCmp(FileName, L"bootmgr.efi") == 0 ||
               StriCmp(FileName, L"Bootmgfw.efi") == 0) {
        OSIconName = L"win";
        ShortcutLetter = 'W';
    } else if (StriCmp(FileName, L"xom.efi") == 0) {
        OSIconName = L"xom,win";
        Entry->UseGraphicsMode = TRUE;
        LoaderKind = 3;
        ShortcutLetter = 'W';
    }
    Entry->me.ShortcutLetter = ShortcutLetter;
    if (Entry->me.Image == NULL)
        Entry->me.Image = LoadOSIcon(OSIconName, L"unknown", FALSE);
    
    // create the submenu
    SubScreen = AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
    SubScreen->Title = PoolPrint(L"Boot Options for %s on %s", (LoaderTitle != NULL) ? LoaderTitle : FileName, Volume->VolName);
    SubScreen->TitleImage = Entry->me.Image;
    
    // default entry
    SubEntry = AllocateZeroPool(sizeof(LOADER_ENTRY));
    SubEntry->me.Title        = (LoaderKind == 1) ? L"Boot Mac OS X" : PoolPrint(L"Run %s", FileName);
    SubEntry->me.Tag          = TAG_LOADER;
    SubEntry->LoaderPath      = Entry->LoaderPath;
    SubEntry->VolName         = Entry->VolName;
    SubEntry->DevicePath      = Entry->DevicePath;
    SubEntry->UseGraphicsMode = Entry->UseGraphicsMode;
    AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
    
    // loader-specific submenu entries
    if (LoaderKind == 1) {          // entries for Mac OS X
#if defined(EFIX64)
        SubEntry = AllocateZeroPool(sizeof(LOADER_ENTRY));
        SubEntry->me.Title        = L"Boot Mac OS X with a 64-bit kernel";
        SubEntry->me.Tag          = TAG_LOADER;
        SubEntry->LoaderPath      = Entry->LoaderPath;
        SubEntry->VolName         = Entry->VolName;
        SubEntry->DevicePath      = Entry->DevicePath;
        SubEntry->UseGraphicsMode = Entry->UseGraphicsMode;
        SubEntry->LoadOptions     = L"arch=x86_64";
        AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);

        SubEntry = AllocateZeroPool(sizeof(LOADER_ENTRY));
        SubEntry->me.Title        = L"Boot Mac OS X with a 32-bit kernel";
        SubEntry->me.Tag          = TAG_LOADER;
        SubEntry->LoaderPath      = Entry->LoaderPath;
        SubEntry->VolName         = Entry->VolName;
        SubEntry->DevicePath      = Entry->DevicePath;
        SubEntry->UseGraphicsMode = Entry->UseGraphicsMode;
        SubEntry->LoadOptions     = L"arch=i386";
        AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
#endif
        
        if (!(GlobalConfig.DisableFlags & DISABLE_FLAG_SINGLEUSER)) {
            SubEntry = AllocateZeroPool(sizeof(LOADER_ENTRY));
            SubEntry->me.Title        = L"Boot Mac OS X in verbose mode";
            SubEntry->me.Tag          = TAG_LOADER;
            SubEntry->LoaderPath      = Entry->LoaderPath;
            SubEntry->VolName         = Entry->VolName;
            SubEntry->DevicePath      = Entry->DevicePath;
            SubEntry->UseGraphicsMode = FALSE;
            SubEntry->LoadOptions     = L"-v";
            AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
            
#if defined(EFIX64)
            SubEntry = AllocateZeroPool(sizeof(LOADER_ENTRY));
            SubEntry->me.Title        = L"Boot Mac OS X in verbose mode (64-bit)";
            SubEntry->me.Tag          = TAG_LOADER;
            SubEntry->LoaderPath      = Entry->LoaderPath;
            SubEntry->VolName         = Entry->VolName;
            SubEntry->DevicePath      = Entry->DevicePath;
            SubEntry->UseGraphicsMode = FALSE;
            SubEntry->LoadOptions     = L"-v arch=x86_64";
            AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
            
            SubEntry = AllocateZeroPool(sizeof(LOADER_ENTRY));
            SubEntry->me.Title        = L"Boot Mac OS X in verbose mode (32-bit)";
            SubEntry->me.Tag          = TAG_LOADER;
            SubEntry->LoaderPath      = Entry->LoaderPath;
            SubEntry->VolName         = Entry->VolName;
            SubEntry->DevicePath      = Entry->DevicePath;
            SubEntry->UseGraphicsMode = FALSE;
            SubEntry->LoadOptions     = L"-v arch=i386";
            AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
#endif
            
            SubEntry = AllocateZeroPool(sizeof(LOADER_ENTRY));
            SubEntry->me.Title        = L"Boot Mac OS X in single user mode";
            SubEntry->me.Tag          = TAG_LOADER;
            SubEntry->LoaderPath      = Entry->LoaderPath;
            SubEntry->VolName         = Entry->VolName;
            SubEntry->DevicePath      = Entry->DevicePath;
            SubEntry->UseGraphicsMode = FALSE;
            SubEntry->LoadOptions     = L"-v -s";
            AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
        }
        
        // check for Apple hardware diagnostics
        StrCpy(DiagsFileName, L"\\System\\Library\\CoreServices\\.diagnostics\\diags.efi");
        if (FileExists(Volume->RootDir, DiagsFileName) && !(GlobalConfig.DisableFlags & DISABLE_FLAG_HWTEST)) {
            Print(L"  - Apple Hardware Test found\n");
            
            SubEntry = AllocateZeroPool(sizeof(LOADER_ENTRY));
            SubEntry->me.Title        = L"Run Apple Hardware Test";
            SubEntry->me.Tag          = TAG_LOADER;
            SubEntry->LoaderPath      = EfiStrDuplicate(DiagsFileName);
            SubEntry->VolName         = Entry->VolName;
            SubEntry->DevicePath      = FileDevicePath(Volume->DeviceHandle, SubEntry->LoaderPath);
            SubEntry->UseGraphicsMode = TRUE;
            AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
        }
        
    } else if (LoaderKind == 2) {   // entries for elilo
        SubEntry = AllocateZeroPool(sizeof(LOADER_ENTRY));
        SubEntry->me.Title        = PoolPrint(L"Run %s in interactive mode", FileName);
        SubEntry->me.Tag          = TAG_LOADER;
        SubEntry->LoaderPath      = Entry->LoaderPath;
        SubEntry->VolName         = Entry->VolName;
        SubEntry->DevicePath      = Entry->DevicePath;
        SubEntry->UseGraphicsMode = Entry->UseGraphicsMode;
        SubEntry->LoadOptions     = L"-p";
        AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
        
        SubEntry = AllocateZeroPool(sizeof(LOADER_ENTRY));
        SubEntry->me.Title        = L"Boot Linux for a 17\" iMac or a 15\" MacBook Pro (*)";
        SubEntry->me.Tag          = TAG_LOADER;
        SubEntry->LoaderPath      = Entry->LoaderPath;
        SubEntry->VolName         = Entry->VolName;
        SubEntry->DevicePath      = Entry->DevicePath;
        SubEntry->UseGraphicsMode = TRUE;
        SubEntry->LoadOptions     = L"-d 0 i17";
        AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
        
        SubEntry = AllocateZeroPool(sizeof(LOADER_ENTRY));
        SubEntry->me.Title        = L"Boot Linux for a 20\" iMac (*)";
        SubEntry->me.Tag          = TAG_LOADER;
        SubEntry->LoaderPath      = Entry->LoaderPath;
        SubEntry->VolName         = Entry->VolName;
        SubEntry->DevicePath      = Entry->DevicePath;
        SubEntry->UseGraphicsMode = TRUE;
        SubEntry->LoadOptions     = L"-d 0 i20";
        AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
        
        SubEntry = AllocateZeroPool(sizeof(LOADER_ENTRY));
        SubEntry->me.Title        = L"Boot Linux for a Mac Mini (*)";
        SubEntry->me.Tag          = TAG_LOADER;
        SubEntry->LoaderPath      = Entry->LoaderPath;
        SubEntry->VolName         = Entry->VolName;
        SubEntry->DevicePath      = Entry->DevicePath;
        SubEntry->UseGraphicsMode = TRUE;
        SubEntry->LoadOptions     = L"-d 0 mini";
        AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
        
        AddMenuInfoLine(SubScreen, L"NOTE: This is an example. Entries");
        AddMenuInfoLine(SubScreen, L"marked with (*) may not work.");
        
    } else if (LoaderKind == 3) {   // entries for xom.efi
        // by default, skip the built-in selection and boot from hard disk only
        Entry->LoadOptions = L"-s -h";
        
        SubEntry = AllocateZeroPool(sizeof(LOADER_ENTRY));
        SubEntry->me.Title        = L"Boot Windows from Hard Disk";
        SubEntry->me.Tag          = TAG_LOADER;
        SubEntry->LoaderPath      = Entry->LoaderPath;
        SubEntry->VolName         = Entry->VolName;
        SubEntry->DevicePath      = Entry->DevicePath;
        SubEntry->UseGraphicsMode = Entry->UseGraphicsMode;
        SubEntry->LoadOptions     = L"-s -h";
        AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
        
        SubEntry = AllocateZeroPool(sizeof(LOADER_ENTRY));
        SubEntry->me.Title        = L"Boot Windows from CD-ROM";
        SubEntry->me.Tag          = TAG_LOADER;
        SubEntry->LoaderPath      = Entry->LoaderPath;
        SubEntry->VolName         = Entry->VolName;
        SubEntry->DevicePath      = Entry->DevicePath;
        SubEntry->UseGraphicsMode = Entry->UseGraphicsMode;
        SubEntry->LoadOptions     = L"-s -c";
        AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
        
        SubEntry = AllocateZeroPool(sizeof(LOADER_ENTRY));
        SubEntry->me.Title        = PoolPrint(L"Run %s in text mode", FileName);
        SubEntry->me.Tag          = TAG_LOADER;
        SubEntry->LoaderPath      = Entry->LoaderPath;
        SubEntry->VolName         = Entry->VolName;
        SubEntry->DevicePath      = Entry->DevicePath;
        SubEntry->UseGraphicsMode = FALSE;
        SubEntry->LoadOptions     = L"-v";
        AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
        
    }
    
    AddMenuEntry(SubScreen, &MenuEntryReturn);
    Entry->me.SubScreen = SubScreen;
    AddMenuEntry(&MainMenu, (REFIT_MENU_ENTRY *)Entry);
    return Entry;
}

static VOID ScanLoaderDir(IN REFIT_VOLUME *Volume, IN CHAR16 *Path)
{
    EFI_STATUS              Status;
    REFIT_DIR_ITER          DirIter;
    EFI_FILE_INFO           *DirEntry;
    CHAR16                  FileName[256];
    
    // look through contents of the directory
    DirIterOpen(Volume->RootDir, Path, &DirIter);
    while (DirIterNext(&DirIter, 2, L"*.EFI", &DirEntry)) {
        if (DirEntry->FileName[0] == '.' ||
            StriCmp(DirEntry->FileName, L"TextMode.efi") == 0 ||
            StriCmp(DirEntry->FileName, L"ebounce.efi") == 0 ||
            StriCmp(DirEntry->FileName, L"GraphicsConsole.efi") == 0)
            continue;   // skip this
        
        if (Path)
            SPrint(FileName, 255, L"\\%s\\%s", Path, DirEntry->FileName);
        else
            SPrint(FileName, 255, L"\\%s", DirEntry->FileName);
        AddLoaderEntry(FileName, NULL, Volume);
    }
    Status = DirIterClose(&DirIter);
    if (Status != EFI_NOT_FOUND) {
        if (Path)
            SPrint(FileName, 255, L"while scanning the %s directory", Path);
        else
            StrCpy(FileName, L"while scanning the root directory");
        CheckError(Status, FileName);
    }
}

static VOID ScanLoader(VOID)
{
    EFI_STATUS              Status;
    UINTN                   VolumeIndex;
    REFIT_VOLUME            *Volume;
    REFIT_DIR_ITER          EfiDirIter;
    EFI_FILE_INFO           *EfiDirEntry;
    CHAR16                  FileName[256];
    LOADER_ENTRY            *Entry;
    
    Print(L"Scanning for boot loaders...\n");
    
    for (VolumeIndex = 0; VolumeIndex < VolumesCount; VolumeIndex++) {
        Volume = Volumes[VolumeIndex];
        if (Volume->RootDir == NULL || Volume->VolName == NULL)
            continue;
        
        // skip volume if its kind is configured as disabled
        if ((Volume->DiskKind == DISK_KIND_OPTICAL && (GlobalConfig.DisableFlags & DISABLE_FLAG_OPTICAL)) ||
            (Volume->DiskKind == DISK_KIND_EXTERNAL && (GlobalConfig.DisableFlags & DISABLE_FLAG_EXTERNAL)) ||
            (Volume->DiskKind == DISK_KIND_INTERNAL && (GlobalConfig.DisableFlags & DISABLE_FLAG_INTERNAL)))
            continue;
        
        // check for Mac OS X boot loader
        StrCpy(FileName, MACOSX_LOADER_PATH);
        if (FileExists(Volume->RootDir, FileName)) {
            Print(L"  - Mac OS X boot file found\n");
            Entry = AddLoaderEntry(FileName, L"Mac OS X", Volume);
        }
        
        // check for XOM
        StrCpy(FileName, L"\\System\\Library\\CoreServices\\xom.efi");
        if (FileExists(Volume->RootDir, FileName)) {
            AddLoaderEntry(FileName, L"Windows XP (XoM)", Volume);
        }
        
        // check for Microsoft boot loader/menu
        StrCpy(FileName, L"\\EFI\\Microsoft\\Boot\\Bootmgfw.efi");
        if (FileExists(Volume->RootDir, FileName)) {
            Print(L"  - Microsoft boot menu found\n");
            AddLoaderEntry(FileName, L"Microsoft boot menu", Volume);
        }
        
        // scan the root directory for EFI executables
        ScanLoaderDir(Volume, NULL);
        // scan the elilo directory (as used on gimli's first Live CD)
        ScanLoaderDir(Volume, L"elilo");
        // scan the boot directory
        ScanLoaderDir(Volume, L"boot");
        
        // scan subdirectories of the EFI directory (as per the standard)
        DirIterOpen(Volume->RootDir, L"EFI", &EfiDirIter);
        while (DirIterNext(&EfiDirIter, 1, NULL, &EfiDirEntry)) {
            if (StriCmp(EfiDirEntry->FileName, L"TOOLS") == 0 ||
                EfiDirEntry->FileName[0] == '.')
                continue;   // skip this, doesn't contain boot loaders
            if (StriCmp(EfiDirEntry->FileName, L"REFIT") == 0 ||
                StriCmp(EfiDirEntry->FileName, L"REFITL") == 0 ||
                StriCmp(EfiDirEntry->FileName, L"RESCUE") == 0)
                continue;   // skip ourselves
            Print(L"  - Directory EFI\\%s found\n", EfiDirEntry->FileName);
            
            SPrint(FileName, 255, L"EFI\\%s", EfiDirEntry->FileName);
            ScanLoaderDir(Volume, FileName);
        }
        Status = DirIterClose(&EfiDirIter);
        if (Status != EFI_NOT_FOUND)
            CheckError(Status, L"while scanning the EFI directory");
    }
}

//
// legacy boot functions
//

static EFI_STATUS ActivateMbrPartition(IN EFI_BLOCK_IO *BlockIO, IN UINTN PartitionIndex)
{
    EFI_STATUS          Status;
    UINT8               SectorBuffer[512];
    MBR_PARTITION_INFO  *MbrTable, *EMbrTable;
    UINT32              ExtBase, ExtCurrent, NextExtCurrent;
    UINTN               LogicalPartitionIndex = 4;
    UINTN               i;
    BOOLEAN             HaveBootCode;
    
    // read MBR
    Status = BlockIO->ReadBlocks(BlockIO, BlockIO->Media->MediaId, 0, 512, SectorBuffer);
    if (EFI_ERROR(Status))
        return Status;
    if (*((UINT16 *)(SectorBuffer + 510)) != 0xaa55)
        return EFI_NOT_FOUND;  // safety measure #1
    
    // add boot code if necessary
    HaveBootCode = FALSE;
    for (i = 0; i < MBR_BOOTCODE_SIZE; i++) {
        if (SectorBuffer[i] != 0) {
            HaveBootCode = TRUE;
            break;
        }
    }
    if (!HaveBootCode) {
        // no boot code found in the MBR, add the syslinux MBR code
        SetMem(SectorBuffer, MBR_BOOTCODE_SIZE, 0);
        CopyMem(SectorBuffer, syslinux_mbr, SYSLINUX_MBR_SIZE);
    }
    
    // set the partition active
    MbrTable = (MBR_PARTITION_INFO *)(SectorBuffer + 446);
    ExtBase = 0;
    for (i = 0; i < 4; i++) {
        if (MbrTable[i].Flags != 0x00 && MbrTable[i].Flags != 0x80)
            return EFI_NOT_FOUND;   // safety measure #2
        if (i == PartitionIndex)
            MbrTable[i].Flags = 0x80;
        else if (PartitionIndex >= 4 && IS_EXTENDED_PART_TYPE(MbrTable[i].Type)) {
            MbrTable[i].Flags = 0x80;
            ExtBase = MbrTable[i].StartLBA;
        } else
            MbrTable[i].Flags = 0x00;
    }
    
    // write MBR
    Status = BlockIO->WriteBlocks(BlockIO, BlockIO->Media->MediaId, 0, 512, SectorBuffer);
    if (EFI_ERROR(Status))
        return Status;
    
    if (PartitionIndex >= 4) {
        // we have to activate a logical partition, so walk the EMBR chain
        
        // NOTE: ExtBase was set above while looking at the MBR table
        for (ExtCurrent = ExtBase; ExtCurrent; ExtCurrent = NextExtCurrent) {
            // read current EMBR
            Status = BlockIO->ReadBlocks(BlockIO, BlockIO->Media->MediaId, ExtCurrent, 512, SectorBuffer);
            if (EFI_ERROR(Status))
                return Status;
            if (*((UINT16 *)(SectorBuffer + 510)) != 0xaa55)
                return EFI_NOT_FOUND;  // safety measure #3
            
            // scan EMBR, set appropriate partition active
            EMbrTable = (MBR_PARTITION_INFO *)(SectorBuffer + 446);
            NextExtCurrent = 0;
            for (i = 0; i < 4; i++) {
                if (EMbrTable[i].Flags != 0x00 && EMbrTable[i].Flags != 0x80)
                    return EFI_NOT_FOUND;   // safety measure #4
                if (EMbrTable[i].StartLBA == 0 || EMbrTable[i].Size == 0)
                    break;
                if (IS_EXTENDED_PART_TYPE(EMbrTable[i].Type)) {
                    // link to next EMBR
                    NextExtCurrent = ExtBase + EMbrTable[i].StartLBA;
                    EMbrTable[i].Flags = (PartitionIndex >= LogicalPartitionIndex) ? 0x80 : 0x00;
                    break;
                } else {
                    // logical partition
                    EMbrTable[i].Flags = (PartitionIndex == LogicalPartitionIndex) ? 0x80 : 0x00;
                    LogicalPartitionIndex++;
                }
            }
            
            // write current EMBR
            Status = BlockIO->WriteBlocks(BlockIO, BlockIO->Media->MediaId, ExtCurrent, 512, SectorBuffer);
            if (EFI_ERROR(Status))
                return Status;
            
            if (PartitionIndex < LogicalPartitionIndex)
                break;  // stop the loop, no need to touch further EMBRs
        }
        
    }
    
    return EFI_SUCCESS;
}

// early 2006 Core Duo / Core Solo models
static UINT8 LegacyLoaderDevicePath1Data[] = {
    0x01, 0x03, 0x18, 0x00, 0x0B, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xE0, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xF9, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x06, 0x14, 0x00, 0xEB, 0x85, 0x05, 0x2B,
    0xB8, 0xD8, 0xA9, 0x49, 0x8B, 0x8C, 0xE2, 0x1B,
    0x01, 0xAE, 0xF2, 0xB7, 0x7F, 0xFF, 0x04, 0x00,
};
// mid-2006 Mac Pro (and probably other Core 2 models)
static UINT8 LegacyLoaderDevicePath2Data[] = {
    0x01, 0x03, 0x18, 0x00, 0x0B, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xE0, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xF7, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x06, 0x14, 0x00, 0xEB, 0x85, 0x05, 0x2B,
    0xB8, 0xD8, 0xA9, 0x49, 0x8B, 0x8C, 0xE2, 0x1B,
    0x01, 0xAE, 0xF2, 0xB7, 0x7F, 0xFF, 0x04, 0x00,
};
// mid-2007 MBP ("Santa Rosa" based models)
static UINT8 LegacyLoaderDevicePath3Data[] = {
    0x01, 0x03, 0x18, 0x00, 0x0B, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xE0, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xF8, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x06, 0x14, 0x00, 0xEB, 0x85, 0x05, 0x2B,
    0xB8, 0xD8, 0xA9, 0x49, 0x8B, 0x8C, 0xE2, 0x1B,
    0x01, 0xAE, 0xF2, 0xB7, 0x7F, 0xFF, 0x04, 0x00,
};
// early-2008 MBA
static UINT8 LegacyLoaderDevicePath4Data[] = {
    0x01, 0x03, 0x18, 0x00, 0x0B, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xC0, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xF8, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x06, 0x14, 0x00, 0xEB, 0x85, 0x05, 0x2B,
    0xB8, 0xD8, 0xA9, 0x49, 0x8B, 0x8C, 0xE2, 0x1B,
    0x01, 0xAE, 0xF2, 0xB7, 0x7F, 0xFF, 0x04, 0x00,
};
// late-2008 MB/MBP (NVidia chipset)
static UINT8 LegacyLoaderDevicePath5Data[] = {
    0x01, 0x03, 0x18, 0x00, 0x0B, 0x00, 0x00, 0x00,
    0x00, 0x40, 0xCB, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xBF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x06, 0x14, 0x00, 0xEB, 0x85, 0x05, 0x2B,
    0xB8, 0xD8, 0xA9, 0x49, 0x8B, 0x8C, 0xE2, 0x1B,
    0x01, 0xAE, 0xF2, 0xB7, 0x7F, 0xFF, 0x04, 0x00,
};

static EFI_DEVICE_PATH *LegacyLoaderList[] = {
    (EFI_DEVICE_PATH *)LegacyLoaderDevicePath1Data,
    (EFI_DEVICE_PATH *)LegacyLoaderDevicePath2Data,
    (EFI_DEVICE_PATH *)LegacyLoaderDevicePath3Data,
    (EFI_DEVICE_PATH *)LegacyLoaderDevicePath4Data,
    (EFI_DEVICE_PATH *)LegacyLoaderDevicePath5Data,
    NULL
};

#define MAX_DISCOVERED_PATHS (16)

static VOID StartLegacy(IN LEGACY_ENTRY *Entry)
{
    EFI_STATUS          Status;
    EG_IMAGE            *BootLogoImage;
    UINTN               ErrorInStep = 0;
    EFI_DEVICE_PATH     *DiscoveredPathList[MAX_DISCOVERED_PATHS];
    
    BeginExternalScreen(TRUE, L"Booting Legacy OS");
    
    BootLogoImage = LoadOSIcon(Entry->Volume->OSIconName, L"legacy", TRUE);
    if (BootLogoImage != NULL)
        BltImageAlpha(BootLogoImage,
                      (UGAWidth  - BootLogoImage->Width ) >> 1,
                      (UGAHeight - BootLogoImage->Height) >> 1,
                      &StdBackgroundPixel);
    
    if (Entry->Volume->IsMbrPartition)
        ActivateMbrPartition(Entry->Volume->WholeDiskBlockIO, Entry->Volume->MbrPartitionIndex);
    
    ExtractLegacyLoaderPaths(DiscoveredPathList, MAX_DISCOVERED_PATHS, LegacyLoaderList);
    
    Status = StartEFIImageList(DiscoveredPathList, Entry->LoadOptions, NULL, L"legacy loader", &ErrorInStep);
    if (Status == EFI_NOT_FOUND) {
        if (ErrorInStep == 1)
            Print(L"\nPlease make sure that you have the latest firmware update installed.\n");
        else if (ErrorInStep == 3)
            Print(L"\nThe firmware refused to boot from the selected volume. Note that external\n"
                  L"hard drives are not well-supported by Apple's firmware for legacy OS booting.\n");
    }
    FinishExternalScreen();
}

static LEGACY_ENTRY * AddLegacyEntry(IN CHAR16 *LoaderTitle, IN REFIT_VOLUME *Volume)
{
    LEGACY_ENTRY            *Entry, *SubEntry;
    REFIT_MENU_SCREEN       *SubScreen;
    CHAR16                  *VolDesc;
    CHAR16                  ShortcutLetter = 0;
    
    if (LoaderTitle == NULL) {
        if (Volume->OSName != NULL) {
            LoaderTitle = Volume->OSName;
            if (LoaderTitle[0] == 'W' || LoaderTitle[0] == 'L')
                ShortcutLetter = LoaderTitle[0];
        } else
            LoaderTitle = L"Legacy OS";
    }
    if (Volume->VolName != NULL)
        VolDesc = Volume->VolName;
    else
        VolDesc = (Volume->DiskKind == DISK_KIND_OPTICAL) ? L"CD" : L"HD";
    
    // prepare the menu entry
    Entry = AllocateZeroPool(sizeof(LEGACY_ENTRY));
    Entry->me.Title        = PoolPrint(L"Boot %s from %s", LoaderTitle, VolDesc);
    Entry->me.Tag          = TAG_LEGACY;
    Entry->me.Row          = 0;
    Entry->me.ShortcutLetter = ShortcutLetter;
    Entry->me.Image        = LoadOSIcon(Volume->OSIconName, L"legacy", FALSE);
    if (GlobalConfig.HideBadges == 0 ||
        (GlobalConfig.HideBadges == 1 && Volume->DiskKind != DISK_KIND_INTERNAL))
        Entry->me.BadgeImage   = Volume->VolBadgeImage;
    Entry->Volume          = Volume;
    Entry->LoadOptions     = (Volume->DiskKind == DISK_KIND_OPTICAL) ? L"CD" :
        ((Volume->DiskKind == DISK_KIND_EXTERNAL) ? L"USB" : L"HD");
    
    // create the submenu
    SubScreen = AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
    SubScreen->Title = PoolPrint(L"Boot Options for %s on %s", LoaderTitle, VolDesc);
    SubScreen->TitleImage = Entry->me.Image;
    
    // default entry
    SubEntry = AllocateZeroPool(sizeof(LEGACY_ENTRY));
    SubEntry->me.Title        = PoolPrint(L"Boot %s", LoaderTitle);
    SubEntry->me.Tag          = TAG_LEGACY;
    SubEntry->Volume          = Entry->Volume;
    SubEntry->LoadOptions     = Entry->LoadOptions;
    AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
    
    AddMenuEntry(SubScreen, &MenuEntryReturn);
    Entry->me.SubScreen = SubScreen;
    AddMenuEntry(&MainMenu, (REFIT_MENU_ENTRY *)Entry);
    return Entry;
}

static VOID ScanLegacy(VOID)
{
    UINTN                   VolumeIndex, VolumeIndex2;
    BOOLEAN                 ShowVolume, HideIfOthersFound;
    REFIT_VOLUME            *Volume;
    
    Print(L"Scanning for legacy boot volumes...\n");
    
    for (VolumeIndex = 0; VolumeIndex < VolumesCount; VolumeIndex++) {
        Volume = Volumes[VolumeIndex];
#if REFIT_DEBUG > 0
        Print(L" %d %s\n  %d %d %s %d %s\n",
              VolumeIndex, DevicePathToStr(Volume->DevicePath),
              Volume->DiskKind, Volume->MbrPartitionIndex,
              Volume->IsAppleLegacy ? L"AL" : L"--", Volume->HasBootCode,
               Volume->VolName ? Volume->VolName : L"(no name)");
#endif
        
        // skip volume if its kind is configured as disabled
        if ((Volume->DiskKind == DISK_KIND_OPTICAL && (GlobalConfig.DisableFlags & DISABLE_FLAG_OPTICAL)) ||
            (Volume->DiskKind == DISK_KIND_EXTERNAL && (GlobalConfig.DisableFlags & DISABLE_FLAG_EXTERNAL)) ||
            (Volume->DiskKind == DISK_KIND_INTERNAL && (GlobalConfig.DisableFlags & DISABLE_FLAG_INTERNAL)))
            continue;
        
        ShowVolume = FALSE;
        HideIfOthersFound = FALSE;
        if (Volume->IsAppleLegacy) {
            ShowVolume = TRUE;
            HideIfOthersFound = TRUE;
        } else if (Volume->HasBootCode) {
            ShowVolume = TRUE;
            if (Volume->BlockIO == Volume->WholeDiskBlockIO &&
                Volume->BlockIOOffset == 0 &&
                Volume->OSName == NULL)
                // this is a whole disk (MBR) entry; hide if we have entries for partitions
                HideIfOthersFound = TRUE;
        }
        if (HideIfOthersFound) {
            // check for other bootable entries on the same disk
            for (VolumeIndex2 = 0; VolumeIndex2 < VolumesCount; VolumeIndex2++) {
                if (VolumeIndex2 != VolumeIndex && Volumes[VolumeIndex2]->HasBootCode &&
                    Volumes[VolumeIndex2]->WholeDiskBlockIO == Volume->WholeDiskBlockIO)
                    ShowVolume = FALSE;
            }
        }
        
        if (ShowVolume)
            AddLegacyEntry(NULL, Volume);
    }
}

//
// pre-boot tool functions
//

static VOID StartTool(IN LOADER_ENTRY *Entry)
{
    BeginExternalScreen(Entry->UseGraphicsMode, Entry->me.Title + 6);  // assumes "Start <title>" as assigned below
    StartEFIImage(Entry->DevicePath, Entry->LoadOptions, Basename(Entry->LoaderPath),
                  Basename(Entry->LoaderPath), NULL);
    FinishExternalScreen();
}

static LOADER_ENTRY * AddToolEntry(IN CHAR16 *LoaderPath, IN CHAR16 *LoaderTitle, IN EG_IMAGE *Image,
                                   IN CHAR16 ShortcutLetter, IN BOOLEAN UseGraphicsMode)
{
    LOADER_ENTRY *Entry;
    
    Entry = AllocateZeroPool(sizeof(LOADER_ENTRY));
    
    Entry->me.Title = PoolPrint(L"Start %s", LoaderTitle);
    Entry->me.Tag = TAG_TOOL;
    Entry->me.Row = 1;
    Entry->me.ShortcutLetter = ShortcutLetter;
    Entry->me.Image = Image;
    Entry->LoaderPath = EfiStrDuplicate(LoaderPath);
    Entry->DevicePath = FileDevicePath(SelfLoadedImage->DeviceHandle, Entry->LoaderPath);
    Entry->UseGraphicsMode = UseGraphicsMode;
    
    AddMenuEntry(&MainMenu, (REFIT_MENU_ENTRY *)Entry);
    return Entry;
}

static VOID ScanTool(VOID)
{
    //EFI_STATUS              Status;
    CHAR16                  FileName[256];
    LOADER_ENTRY            *Entry;
    
    if (GlobalConfig.DisableFlags & DISABLE_FLAG_TOOLS)
        return;
    
    Print(L"Scanning for tools...\n");
    
    // look for the EFI shell
    if (!(GlobalConfig.DisableFlags & DISABLE_FLAG_SHELL)) {
        SPrint(FileName, 255, L"%s\\apps\\shell.efi", SelfDirPath);
        if (FileExists(SelfRootDir, FileName)) {
            Entry = AddToolEntry(FileName, L"EFI Shell", BuiltinIcon(BUILTIN_ICON_TOOL_SHELL), 'S', FALSE);
        } else {
            StrCpy(FileName, L"\\efi\\tools\\Shell.efi");
            if (FileExists(SelfRootDir, FileName)) {
                Entry = AddToolEntry(FileName, L"EFI Shell", BuiltinIcon(BUILTIN_ICON_TOOL_SHELL), 'S', FALSE);
            }
        }
    }
    
    // look for the GPT/MBR sync tool
/*    StrCpy(FileName, L"\\efi\\tools\\gptsync.efi");
    if (FileExists(SelfRootDir, FileName)) {
        Entry = AddToolEntry(FileName, L"Partitioning Tool", BuiltinIcon(BUILTIN_ICON_TOOL_PART), 'P', FALSE);
    }*/
/*    
    // look for rescue Linux
    StrCpy(FileName, L"\\efi\\rescue\\elilo.efi");
    if (SelfVolume != NULL && FileExists(SelfRootDir, FileName)) {
        Entry = AddToolEntry(FileName, L"Rescue Linux", BuiltinIcon(BUILTIN_ICON_TOOL_RESCUE), 0, FALSE);
        
        if (UGAWidth == 1440 && UGAHeight == 900)
            Entry->LoadOptions = L"-d 0 i17";
        else if (UGAWidth == 1680 && UGAHeight == 1050)
            Entry->LoadOptions = L"-d 0 i20";
        else
            Entry->LoadOptions = L"-d 0 mini";
    }
 */
}

//
// pre-boot driver functions
//

static VOID ScanDriverDir(IN CHAR16 *Path)
{
    EFI_STATUS              Status;
    REFIT_DIR_ITER          DirIter;
    EFI_FILE_INFO           *DirEntry;
    CHAR16                  FileName[256];
    
    // look through contents of the directory
    DirIterOpen(SelfRootDir, Path, &DirIter);
    while (DirIterNext(&DirIter, 2, L"*.EFI", &DirEntry)) {
        if (DirEntry->FileName[0] == '.')
            continue;   // skip this
        
        SPrint(FileName, 255, L"%s\\%s", Path, DirEntry->FileName);
        Status = StartEFIImage(FileDevicePath(SelfLoadedImage->DeviceHandle, FileName),
                               L"", DirEntry->FileName, DirEntry->FileName, NULL);
    }
    Status = DirIterClose(&DirIter);
    if (Status != EFI_NOT_FOUND) {
        SPrint(FileName, 255, L"while scanning the %s directory", Path);
        CheckError(Status, FileName);
    }
}
/*
static EFI_STATUS ConnectAllDriversToAllControllers(VOID)
{
    EFI_STATUS  Status;
    UINTN       AllHandleCount;
    EFI_HANDLE  *AllHandleBuffer;
    UINTN       Index;
    UINTN       HandleCount;
    EFI_HANDLE  *HandleBuffer;
    UINT32      *HandleType;
    UINTN       HandleIndex;
    BOOLEAN     Parent;
    BOOLEAN     Device;
    
    Status = gBS->LocateHandleBuffer(AllHandles,
                             NULL,
                             NULL,
                             &AllHandleCount,
                             &AllHandleBuffer);
    if (EFI_ERROR(Status))
        return Status;
    
    for (Index = 0; Index < AllHandleCount; Index++) {
        //
        // Scan the handle database
        //
        Status = LibScanHandleDatabase(NULL,
                                       NULL,
                                       AllHandleBuffer[Index],
                                       NULL,
                                       &HandleCount,
                                       &HandleBuffer,
                                       &HandleType);
        if (EFI_ERROR (Status))
            goto Done;
        
        Device = TRUE;
        if (HandleType[Index] & EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE)
            Device = FALSE;
        if (HandleType[Index] & EFI_HANDLE_TYPE_IMAGE_HANDLE)
            Device = FALSE;
        
        if (Device) {
            Parent = FALSE;
            for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
                if (HandleType[HandleIndex] & EFI_HANDLE_TYPE_PARENT_HANDLE)
                    Parent = TRUE;
            }
            
            if (!Parent) {
                if (HandleType[Index] & EFI_HANDLE_TYPE_DEVICE_HANDLE) {
                    Status = gBS->ConnectController(AllHandleBuffer[Index],
                                                   NULL,
                                                   NULL,
                                                   TRUE);
                }
            }
        }
        
        FreePool (HandleBuffer);
        FreePool (HandleType);
    }
    
Done:
    FreePool (AllHandleBuffer);
    return Status;
}
*/
static VOID LoadDrivers(VOID)
{
//    CHAR16                  DirName[256];
    
    Print(L"Scanning for drivers...\n");
    
    // load drivers from /efi/refit/drivers
//    SPrint(DirName, 255, L"%s\\drivers", SelfDirPath);
//    ScanDriverDir(DirName);
//    Print(L"Scanning for drivers in /efi/refit/drivers complete\n");
    // load drivers from /efi/drivers
    ScanDriverDir(L"\\efi\\drivers");
    Print(L"Scanning for drivers in /efi/tools/drivers complete\n");
    // connect all devices
    BdsLibConnectAllDriversToAllControllers();
	Print(L"Drivers connected\n");
}

//
// main entry point
//

//#ifdef __GNUC__
//#define RefitMain efi_main
//#endif

EFI_STATUS
EFIAPI
RefitMain (IN EFI_HANDLE           ImageHandle,
           IN EFI_SYSTEM_TABLE     *SystemTable)
{
  EFI_STATUS Status;
  BOOLEAN           MainLoopRunning = TRUE;
  REFIT_MENU_ENTRY  *ChosenEntry;
  UINTN             MenuExit;
  UINTN i;
  UINT8             *Buffer = NULL;
  
  // bootstrap
  //    InitializeLib(ImageHandle, SystemTable);
	gST				= SystemTable;
	gImageHandle	= ImageHandle;
	gBS				= SystemTable->BootServices;
	gRT				= SystemTable->RuntimeServices;
	Status = EfiGetSystemConfigurationTable (&gEfiDxeServicesTableGuid, (VOID **) &gDS);
	
	InitBooterLog();
  InitScreen();
  Status = InitRefitLib(ImageHandle);
  if (EFI_ERROR(Status))
    return Status;
  
  // read GUI configuration
  ReadConfig();
  MainMenu.TimeoutSeconds = GlobalConfig.Timeout;
  
  // disable EFI watchdog timer
  gBS->SetWatchdogTimer(0x0000, 0x0000, 0x0000, NULL);
  
  // further bootstrap (now with config available)
  SetupScreen();
  LoadDrivers();
  ScanVolumes();
  //   DebugPause();
  
  //setup properties
  SetGraphics();
  PrepatchSmbios();
  GetCPUProperties();
  ScanSPD();
  GetDefaultSettings();
  Status = gRS->GetVariable(L"boot-args",
                            &gEfiAppleBootGuid,  NULL,
                            &Size, 										   
                            Buffer);
	if (Status == EFI_BUFFER_TOO_SMALL) {
		Buffer = (UINT8 *) AllocatePool (Size);		
		if (!Buffer){
			Print(L"Errors allocating kernel flags!\n");
 		} else {
			Status = gRS->GetVariable (L"boot-args",
                                 &gEfiAppleBootGuid, NULL,
                                 &Size, 
                                 Buffer);
		}		
	}
	if (Status == EFI_SUCCESS)
		CopyMem(gSettingsFromMenu.BootArgs, Buffer, Size);			

  //Second step. Load config.plist into gSettings	
	GetUserSettings(SelfVolume, L"EFI\\config.plist");
  
  // scan for loaders and tools, add them to the menu
  if (GlobalConfig.LegacyFirst)
    ScanLegacy();
    ScanLoader();
    if (!GlobalConfig.LegacyFirst)
      ScanLegacy();
      if (!(GlobalConfig.DisableFlags & DISABLE_FLAG_TOOLS)) {
        ScanTool();
      }
  
  //    DebugPause();
  
  // fixed other menu entries
  if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_FUNCS)) {
    MenuEntryAbout.Image = BuiltinIcon(BUILTIN_ICON_FUNC_ABOUT);
    AddMenuEntry(&MainMenu, &MenuEntryAbout);
  }
  if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_FUNCS) || MainMenu.EntryCount == 0) {
    MenuEntryShutdown.Image = BuiltinIcon(BUILTIN_ICON_FUNC_SHUTDOWN);
    AddMenuEntry(&MainMenu, &MenuEntryShutdown);
    MenuEntryReset.Image = BuiltinIcon(BUILTIN_ICON_FUNC_RESET);
    AddMenuEntry(&MainMenu, &MenuEntryReset);
  }
  
  // assign shortcut keys
  for (i = 0; i < MainMenu.EntryCount && MainMenu.Entries[i]->Row == 0 && i < 9; i++)
    MainMenu.Entries[i]->ShortcutDigit = (CHAR16)('1' + i);
    
    // wait for user ACK when there were errors
    FinishTextScreen(FALSE);
    
    while (MainLoopRunning) {
      MenuExit = RunMainMenu(&MainMenu, GlobalConfig.DefaultSelection, &ChosenEntry);
      
      // We don't allow exiting the main menu with the Escape key.
      if (MenuExit == MENU_EXIT_ESCAPE)
        continue;
      
      switch (ChosenEntry->Tag) {
          
        case TAG_RESET:    // Restart
          TerminateScreen();
          gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
          MainLoopRunning = FALSE;   // just in case we get this far
          break;
          
        case TAG_SHUTDOWN: // Shut Down
          TerminateScreen();
          gRT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          MainLoopRunning = FALSE;   // just in case we get this far
          break;
          
        case TAG_ABOUT:    // About rEFIt
          AboutRefit();
          break;
          
        case TAG_LOADER:   // Boot OS via .EFI loader
          StartLoader((LOADER_ENTRY *)ChosenEntry);
          break;
          
        case TAG_LEGACY:   // Boot legacy OS
          StartLegacy((LEGACY_ENTRY *)ChosenEntry);
          break;
          
        case TAG_TOOL:     // Start a EFI tool
          StartTool((LOADER_ENTRY *)ChosenEntry);
          break;
          
      }
    }
  
  // If we end up here, things have gone wrong. Try to reboot, and if that
  // fails, go into an endless loop.
  gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
  EndlessIdleLoop();
  
  return EFI_SUCCESS;
}
