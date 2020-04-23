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

#include "../Platform/Platform.h"
#include "../cpp_util/globals_ctor.h"
#include "../cpp_util/globals_dtor.h"
#include "../cpp_unit_test/all_tests.h"

#include "../entry_scan/entry_scan.h"
#include "../libeg/nanosvg.h"
#include "../gui/menu_items/menu_globals.h"
#include "menu.h"
#include "../Platform/Settings.h"
#include "../Platform/DataHubCpu.h"
#include "../Platform/Events.h"
#include "screen.h"
#include "../entry_scan/bootscreen.h"
#include "../Platform/Nvram.h"
#include "../entry_scan/common.h"
#include "../gui/shared_with_menu.h"
#include "../Platform/platformdata.h"
#include "../Platform/guid.h"
#include "../Platform/APFS.h"
#include "../Platform/cpu.h"
#include "../Platform/smbios.h"
#include "../Platform/AcpiPatcher.h"
#include "../Platform/Hibernate.h"
#include "../Platform/LegacyBoot.h"
#include "../Platform/PlatformDriverOverride.h"
#include "../Platform/Edid.h"
#include "../Platform/Console.h"
#include "../Platform/Net.h"
#include "../Platform/spd.h"
#include "../Platform/Injectors.h"
#include "../Platform/StartupSound.h"
#include "../Platform/BootOptions.h"
#include "../Platform/boot.h"
#include "../Platform/kext_inject.h"

#ifndef DEBUG_ALL
#define DEBUG_MAIN 1
#else
#define DEBUG_MAIN DEBUG_ALL
#endif

#if DEBUG_MAIN == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_MAIN, __VA_ARGS__)
#endif

#ifndef HIBERNATE
#define HIBERNATE 0
#endif


#ifndef CHECK_SMC
#define CHECK_SMC 0
#endif


#define PCAT_RTC_ADDRESS_REGISTER 0x70
#define PCAT_RTC_DATA_REGISTER    0x71


// variables

BOOLEAN                 gGuiIsReady     = FALSE;
BOOLEAN                 gThemeNeedInit  = TRUE;
BOOLEAN                 DoHibernateWake = FALSE;


EFI_HANDLE ConsoleInHandle;
EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* SimpleTextEx;
EFI_KEY_DATA KeyData;

CONST CHAR8* AudioOutputNames[] = {
  "LineOut",
  "Speaker",
  "Headphones",
  "SPDIF",
  "Garniture",
  "HDMI",
  "Other"
};

extern VOID HelpRefit(VOID);
extern VOID AboutRefit(VOID);
extern BOOLEAN BooterPatch(IN UINT8 *BooterData, IN UINT64 BooterSize, LOADER_ENTRY *Entry);

extern UINTN                ThemesNum;
extern CONST CHAR16               *ThemesList[];
extern UINTN                 ConfigsNum;
extern CHAR16                *ConfigsList[];
extern UINTN                 DsdtsNum;
extern CHAR16                *DsdtsList[];
extern UINTN                 AudioNum;
extern HDA_OUTPUTS           AudioList[20];
extern EFI_AUDIO_IO_PROTOCOL *AudioIo;

//#ifdef _cplusplus
//void FreePool(const wchar_t * A)
//{
//  FreePool((VOID*)A);
//}
//#endif

static EFI_STATUS LoadEFIImageList(IN EFI_DEVICE_PATH **DevicePaths,
                                    IN CONST CHAR16 *ImageTitle,
                                    OUT UINTN *ErrorInStep,
                                    OUT EFI_HANDLE *NewImageHandle)
{
  EFI_STATUS              Status, ReturnStatus;
  EFI_HANDLE              ChildImageHandle = 0;
  UINTN                   DevicePathIndex;
  CHAR16                  ErrorInfo[256];

  DBG("Loading %ls", ImageTitle);
  if (ErrorInStep != NULL) {
    *ErrorInStep = 0;
  }
  if (NewImageHandle != NULL) {
    *NewImageHandle = NULL;
  }

  // load the image into memory
  ReturnStatus = Status = EFI_NOT_FOUND;  // in case the list is empty
  for (DevicePathIndex = 0; DevicePaths[DevicePathIndex] != NULL; DevicePathIndex++) {
    ReturnStatus = Status = gBS->LoadImage(FALSE, SelfImageHandle, DevicePaths[DevicePathIndex], NULL, 0, &ChildImageHandle);
    DBG("  status=%s", strerror(Status));
    if (ReturnStatus != EFI_NOT_FOUND)
      break;
  }
	snwprintf(ErrorInfo, 512, "while loading %ls", ImageTitle);
  if (CheckError(Status, ErrorInfo)) {
    if (ErrorInStep != NULL)
      *ErrorInStep = 1;
    PauseForKey(L"press any key");
    goto bailout;
  }

  if (!EFI_ERROR(ReturnStatus)) { //why unload driver?!
    if (NewImageHandle != NULL) {
      *NewImageHandle = ChildImageHandle;
    }
#ifdef JIEF_DEBUG
    EFI_LOADED_IMAGE_PROTOCOL* loadedBootImage = NULL;
    if (!EFI_ERROR(Status = gBS->HandleProtocol(ChildImageHandle, &gEfiLoadedImageProtocolGuid, (void**)(&loadedBootImage)))) {
		DBG("%S : Image base = 0x%llx", ImageTitle, (uintptr_t)loadedBootImage->ImageBase); // Jief : Do not change this, it's used by grep to feed the debugger
    }else{
      DBG("Can't get loaded image protocol");
    }
#endif
    goto bailout;
  }

  // unload the image, we don't care if it works or not...
  Status = gBS->UnloadImage(ChildImageHandle);
bailout:
  DBG("\n");
  return ReturnStatus;
}


static EFI_STATUS StartEFILoadedImage(IN EFI_HANDLE ChildImageHandle,
                                    IN CONST XString& LoadOptions, IN CONST CHAR16 *LoadOptionsPrefix,
                                    IN CONST CHAR16 *ImageTitle,
                                    OUT UINTN *ErrorInStep)
{
  EFI_STATUS                  Status, ReturnStatus;
  EFI_LOADED_IMAGE_PROTOCOL   *ChildLoadedImage;
  CHAR16                      ErrorInfo[256];
//  CHAR16                  *FullLoadOptions = NULL;

//  DBG("Starting %ls\n", ImageTitle);
  if (ErrorInStep != NULL) {
    *ErrorInStep = 0;
  }
  ReturnStatus = Status = EFI_NOT_FOUND;  // in case no image handle was specified
  if (ChildImageHandle == NULL) {
    if (ErrorInStep != NULL) *ErrorInStep = 1;
    goto bailout;
  }

  // set load options
  if (!LoadOptions.isEmpty()) {
    ReturnStatus = Status = gBS->HandleProtocol(ChildImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &ChildLoadedImage);
    if (CheckError(Status, L"while getting a LoadedImageProtocol handle")) {
      if (ErrorInStep != NULL)
        *ErrorInStep = 2;
      goto bailout_unload;
    }

    XStringW loadOptionsW;
    if (LoadOptionsPrefix != NULL) {
      // NOTE: That last space is also added by the EFI shell and seems to be significant
      //  when passing options to Apple's boot.efi...
      loadOptionsW = SWPrintf("%ls %s ", LoadOptionsPrefix, LoadOptions.c_str());
    }else{
      loadOptionsW = SWPrintf("%s ", LoadOptions.c_str()); // Jief : should we add a space ? Wasn't the case before big refactoring. Yes, a space required.
    }
    // NOTE: We also include the terminating null in the length for safety.
    ChildLoadedImage->LoadOptions = (void*)EfiStrDuplicate(loadOptionsW.wc_str()); //will it be deleted after the procedure exit?
    ChildLoadedImage->LoadOptionsSize = (UINT32)loadOptionsW.sizeInBytes() + sizeof(wchar_t);
    //((UINT32)StrLen(LoadOptions) + 1) * sizeof(CHAR16);
    DBG("start image '%ls'\n", ImageTitle);
    DBG("Using load options '%ls'\n", (CHAR16*)ChildLoadedImage->LoadOptions);

  }
  //DBG("Image loaded at: %p\n", ChildLoadedImage->ImageBase);
  //PauseForKey(L"continue");
  
  // close open file handles
  UninitRefitLib();

  // turn control over to the image
  //
  // Before calling the image, enable the Watchdog Timer for
  // the 5 Minute period - Slice - NO! For slow driver and slow disk we need more
  //
  gBS->SetWatchdogTimer (600, 0x0000, 0x00, NULL);

  ReturnStatus = Status = gBS->StartImage(ChildImageHandle, NULL, NULL);
  //
  // Clear the Watchdog Timer after the image returns
  //
  gBS->SetWatchdogTimer (0x0000, 0x0000, 0x0000, NULL);

  //PauseForKey(L"Returned from StartImage\n");

  // control returns here when the child image calls Exit()
  if (ImageTitle) {
	  snwprintf(ErrorInfo, 512, "returned from %ls", ImageTitle);
  }

  if (CheckError(Status, ErrorInfo)) {
    if (ErrorInStep != NULL)
      *ErrorInStep = 3;
  }
  if (!EFI_ERROR(ReturnStatus)) { //why unload driver?!
    goto bailout;
  }

bailout_unload:
  // unload the image, we don't care if it works or not...
  Status = gBS->UnloadImage(ChildImageHandle);
bailout:
  return ReturnStatus;
}


static EFI_STATUS LoadEFIImage(IN EFI_DEVICE_PATH *DevicePath,
                                IN CONST CHAR16 *ImageTitle,
                                OUT UINTN *ErrorInStep,
                                OUT EFI_HANDLE *NewImageHandle)
{
  EFI_DEVICE_PATH *DevicePaths[2];

#ifdef ENABLE_SECURE_BOOT
  // Verify secure boot policy
  if (gSettings.SecureBoot && gSettings.SecureBootSetupMode) {
    // Only verify if in forced secure boot mode
    EFI_STATUS Status = VerifySecureBootImage(DevicePath);
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }
#endif // ENABLE_SECURE_BOOT

  // Load the image now
  DevicePaths[0] = DevicePath;
  DevicePaths[1] = NULL;
  return LoadEFIImageList(DevicePaths, ImageTitle, ErrorInStep, NewImageHandle);
}


static EFI_STATUS StartEFIImage(IN EFI_DEVICE_PATH *DevicePath,
                                IN CONST XString& LoadOptions, IN CONST CHAR16 *LoadOptionsPrefix,
                                IN CONST CHAR16 *ImageTitle,
                                OUT UINTN *ErrorInStep,
                                OUT EFI_HANDLE *NewImageHandle)
{
  EFI_STATUS Status;
  EFI_HANDLE ChildImageHandle = NULL;

  Status = LoadEFIImage(DevicePath, ImageTitle, ErrorInStep, &ChildImageHandle);
  if (!EFI_ERROR(Status)) {
    Status = StartEFILoadedImage(ChildImageHandle, LoadOptions, LoadOptionsPrefix, ImageTitle, ErrorInStep);
  }

  if (NewImageHandle != NULL) {
      *NewImageHandle = ChildImageHandle;
  }
  return Status;
}

/*
static EFI_STATUS StartEFIImageList(IN EFI_DEVICE_PATH **DevicePaths,
                                IN CHAR16 *LoadOptions, IN CHAR16 *LoadOptionsPrefix,
                                IN CHAR16 *ImageTitle,
                                OUT UINTN *ErrorInStep,
                                OUT EFI_HANDLE *NewImageHandle)
{
  EFI_STATUS Status;
  EFI_HANDLE ChildImageHandle = NULL;

  Status = LoadEFIImageList(DevicePaths, ImageTitle, ErrorInStep, &ChildImageHandle);
  if (!EFI_ERROR(Status)) {
    Status = StartEFILoadedImage(ChildImageHandle, LoadOptions, LoadOptionsPrefix, ImageTitle, ErrorInStep);
  }

  if (NewImageHandle != NULL) {
      *NewImageHandle = ChildImageHandle;
  }
  return Status;
}
*/
/*
static CONST CHAR8 *SearchString(
  IN  CONST CHAR8       *Source,
  IN  UINT64      SourceSize,
  IN  CONST CHAR8       *Search,
  IN  UINTN       SearchSize
  )
{
  CONST CHAR8 *End = Source + SourceSize;

  while (Source < End) {
    if (CompareMem(Source, Search, SearchSize) == 0) {
      return Source;
    } else {
      Source++;
    }
  }
  return NULL;
}
 */
#ifdef DUMP_KERNEL_KEXT_PATCHES
VOID DumpKernelAndKextPatches(KERNEL_AND_KEXT_PATCHES *Patches)
{
  if (!Patches) {
    DBG("Kernel and Kext Patches null pointer\n");
    return;
  }
  DBG("Kernel and Kext Patches at %p:\n", Patches);
  DBG("\tAllowed: %c\n", gSettings.KextPatchesAllowed ? 'y' : 'n');
  DBG("\tDebug: %c\n", Patches->KPDebug ? 'y' : 'n');
  DBG("\tKernelCpu: %c\n", Patches->KPKernelCpu ? 'y' : 'n');
  DBG("\tKernelLapic: %c\n", Patches->KPKernelLapic ? 'y' : 'n');
  DBG("\tKernelXCPM: %c\n", Patches->KPKernelXCPM ? 'y' : 'n');
  DBG("\tKernelPm: %c\n", Patches->KPKernelPm ? 'y' : 'n');
  DBG("\tAppleIntelCPUPM: %c\n", Patches->KPAppleIntelCPUPM ? 'y' : 'n');
  DBG("\tAppleRTC: %c\n", Patches->KPAppleRTC ? 'y' : 'n');
  // Dell smbios truncate fix
  DBG("\tDellSMBIOSPatch: %c\n", Patches->KPDELLSMBIOS ? 'y' : 'n');
  DBG("\tFakeCPUID: 0x%X\n", Patches->FakeCPUID);
  DBG("\tATIController: %s\n", (Patches->KPATIConnectorsController == NULL) ? "(null)": Patches->KPATIConnectorsController);
  DBG("\tATIDataLength: %d\n", Patches->KPATIConnectorsDataLen);
  DBG("\t%d Kexts to load\n", Patches->NrForceKexts);
  if (Patches->ForceKexts) {
    INTN i = 0;
    for (; i < Patches->NrForceKexts; ++i) {
       DBG("\t  KextToLoad[%d]: %ls\n", i, Patches->ForceKexts[i]);
    }
  }
  DBG("\t%d Kexts to patch\n", Patches->NrKexts);
  if (Patches->KextPatches) {
    INTN i = 0;
    for (; i < Patches->NrKexts; ++i) {
       if (Patches->KextPatches[i].IsPlistPatch) {
          DBG("\t  KextPatchPlist[%d]: %d bytes, %s\n", i, Patches->KextPatches[i].DataLen, Patches->KextPatches[i].Name);
       } else {
          DBG("\t  KextPatch[%d]: %d bytes, %s\n", i, Patches->KextPatches[i].DataLen, Patches->KextPatches[i].Name);
       }
    }
  }
}
#endif
VOID FilterKextPatches(IN LOADER_ENTRY *Entry) //zzzz
{
  if (gSettings.KextPatchesAllowed && (Entry->KernelAndKextPatches->KextPatches != NULL) && Entry->KernelAndKextPatches->NrKexts) {
    INTN i;
    DBG("Filtering KextPatches:\n");
    for (i = 0; i < Entry->KernelAndKextPatches->NrKexts; i++) {
		DBG(" - [%02lld]: %s :: %s :: [OS: %s | MatchOS: %s | MatchBuild: %s]",
        i,
        Entry->KernelAndKextPatches->KextPatches[i].Label,
        Entry->KernelAndKextPatches->KextPatches[i].IsPlistPatch ? "PlistPatch" : "BinPatch",
        Entry->OSVersion,
        Entry->KernelAndKextPatches->KextPatches[i].MatchOS ? Entry->KernelAndKextPatches->KextPatches[i].MatchOS : "All",
        Entry->KernelAndKextPatches->KextPatches[i].MatchBuild != NULL ? Entry->KernelAndKextPatches->KextPatches[i].MatchBuild : "All"
      );
      if (!Entry->KernelAndKextPatches->KextPatches[i].MenuItem.BValue) {
        DBG(" ==> disabled by user\n");
        continue;
      }
      
      if ((Entry->BuildVersion != NULL) && (Entry->KernelAndKextPatches->KextPatches[i].MatchBuild != NULL)) {
        Entry->KernelAndKextPatches->KextPatches[i].MenuItem.BValue = IsPatchEnabled(Entry->KernelAndKextPatches->KextPatches[i].MatchBuild, Entry->BuildVersion);
        DBG(" ==> %s\n", Entry->KernelAndKextPatches->KextPatches[i].MenuItem.BValue ? "allowed" : "not allowed");
        continue; 
      }

      Entry->KernelAndKextPatches->KextPatches[i].MenuItem.BValue = IsPatchEnabled(Entry->KernelAndKextPatches->KextPatches[i].MatchOS, Entry->OSVersion);
      DBG(" ==> %s\n", Entry->KernelAndKextPatches->KextPatches[i].MenuItem.BValue ? "allowed" : "not allowed");
    }
  }
}

VOID FilterKernelPatches(IN LOADER_ENTRY *Entry)
{
  if (gSettings.KernelPatchesAllowed && (Entry->KernelAndKextPatches->KernelPatches != NULL) && Entry->KernelAndKextPatches->NrKernels) {
    INTN i = 0;
    DBG("Filtering KernelPatches:\n");
    for (; i < Entry->KernelAndKextPatches->NrKernels; ++i) {
		DBG(" - [%02lld]: %s :: [OS: %s | MatchOS: %s | MatchBuild: %s]",
        i,
        Entry->KernelAndKextPatches->KernelPatches[i].Label,
        Entry->OSVersion,
        Entry->KernelAndKextPatches->KernelPatches[i].MatchOS ? Entry->KernelAndKextPatches->KernelPatches[i].MatchOS : "All",
        Entry->KernelAndKextPatches->KernelPatches[i].MatchBuild != NULL ? Entry->KernelAndKextPatches->KernelPatches[i].MatchBuild : "no"
      );
      if (!Entry->KernelAndKextPatches->KernelPatches[i].MenuItem.BValue) {
        DBG(" ==> disabled by user\n");
        continue;
      }

      if ((Entry->BuildVersion != NULL) && (Entry->KernelAndKextPatches->KernelPatches[i].MatchBuild != NULL)) {
        Entry->KernelAndKextPatches->KernelPatches[i].MenuItem.BValue = IsPatchEnabled(Entry->KernelAndKextPatches->KernelPatches[i].MatchBuild, Entry->BuildVersion);
        DBG(" ==> %s by build\n", Entry->KernelAndKextPatches->KernelPatches[i].MenuItem.BValue ? "allowed" : "not allowed");
        continue; 
      }

      Entry->KernelAndKextPatches->KernelPatches[i].MenuItem.BValue = IsPatchEnabled(Entry->KernelAndKextPatches->KernelPatches[i].MatchOS, Entry->OSVersion);
      DBG(" ==> %s by OS\n", Entry->KernelAndKextPatches->KernelPatches[i].MenuItem.BValue ? "allowed" : "not allowed");
    }
  }
}

VOID FilterBootPatches(IN LOADER_ENTRY *Entry)
{
  if ((Entry->KernelAndKextPatches->BootPatches != NULL) && Entry->KernelAndKextPatches->NrBoots) {
    INTN i = 0;
    DBG("Filtering BootPatches:\n");
    for (; i < Entry->KernelAndKextPatches->NrBoots; ++i) {
		DBG(" - [%02lld]: %s :: [OS: %s | MatchOS: %s | MatchBuild: %s]",
          i,
          Entry->KernelAndKextPatches->BootPatches[i].Label,
          Entry->OSVersion,
          Entry->KernelAndKextPatches->BootPatches[i].MatchOS ? Entry->KernelAndKextPatches->BootPatches[i].MatchOS : "All",
          Entry->KernelAndKextPatches->BootPatches[i].MatchBuild != NULL ? Entry->KernelAndKextPatches->BootPatches[i].MatchBuild : "no"
          );
      if (!Entry->KernelAndKextPatches->BootPatches[i].MenuItem.BValue) {
        DBG(" ==> disabled by user\n");
        continue;
      }

      if ((Entry->BuildVersion != NULL) && (Entry->KernelAndKextPatches->BootPatches[i].MatchBuild != NULL)) {
        Entry->KernelAndKextPatches->BootPatches[i].MenuItem.BValue = IsPatchEnabled(Entry->KernelAndKextPatches->BootPatches[i].MatchBuild, Entry->BuildVersion);
        DBG(" ==> %s by build\n", Entry->KernelAndKextPatches->BootPatches[i].MenuItem.BValue ? "allowed" : "not allowed");
        continue;
      }
 
      Entry->KernelAndKextPatches->BootPatches[i].MenuItem.BValue = IsPatchEnabled(Entry->KernelAndKextPatches->BootPatches[i].MatchOS, Entry->OSVersion);
      DBG(" ==> %s by OS\n", Entry->KernelAndKextPatches->BootPatches[i].MenuItem.BValue ? "allowed" : "not allowed");
  
    }
  }
}

VOID ReadSIPCfg()
{
  UINT32 csrCfg = gSettings.CsrActiveConfig & CSR_VALID_FLAGS;
  CHAR16 *csrLog = (__typeof__(csrLog))AllocateZeroPool(SVALUE_MAX_SIZE);

  if (csrCfg & CSR_ALLOW_UNTRUSTED_KEXTS)
    StrCatS(csrLog, SVALUE_MAX_SIZE/2, L"CSR_ALLOW_UNTRUSTED_KEXTS");
  if (csrCfg & CSR_ALLOW_UNRESTRICTED_FS)
    StrCatS(csrLog, SVALUE_MAX_SIZE/2, PoolPrint(L"%a%a", StrLen(csrLog) ? " | " : "", "CSR_ALLOW_UNRESTRICTED_FS"));
  if (csrCfg & CSR_ALLOW_TASK_FOR_PID)
    StrCatS(csrLog, SVALUE_MAX_SIZE/2, PoolPrint(L"%a%a", StrLen(csrLog) ? " | " : "", "CSR_ALLOW_TASK_FOR_PID"));
  if (csrCfg & CSR_ALLOW_KERNEL_DEBUGGER)
    StrCatS(csrLog, SVALUE_MAX_SIZE/2, PoolPrint(L"%a%a", StrLen(csrLog) ? " | " : "", "CSR_ALLOW_KERNEL_DEBUGGER"));
  if (csrCfg & CSR_ALLOW_APPLE_INTERNAL)
    StrCatS(csrLog, SVALUE_MAX_SIZE/2, PoolPrint(L"%a%a", StrLen(csrLog) ? " | " : "", "CSR_ALLOW_APPLE_INTERNAL"));
  if (csrCfg & CSR_ALLOW_UNRESTRICTED_DTRACE)
    StrCatS(csrLog, SVALUE_MAX_SIZE/2, PoolPrint(L"%a%a", StrLen(csrLog) ? " | " : "", "CSR_ALLOW_UNRESTRICTED_DTRACE"));
  if (csrCfg & CSR_ALLOW_UNRESTRICTED_NVRAM)
    StrCatS(csrLog, SVALUE_MAX_SIZE/2, PoolPrint(L"%a%a", StrLen(csrLog) ? " | " : "", "CSR_ALLOW_UNRESTRICTED_NVRAM"));
  if (csrCfg & CSR_ALLOW_DEVICE_CONFIGURATION)
    StrCatS(csrLog, SVALUE_MAX_SIZE/2, PoolPrint(L"%a%a", StrLen(csrLog) ? " | " : "", "CSR_ALLOW_DEVICE_CONFIGURATION"));
  if (csrCfg & CSR_ALLOW_ANY_RECOVERY_OS)
    StrCatS(csrLog, SVALUE_MAX_SIZE/2, PoolPrint(L"%a%a", StrLen(csrLog) ? " | " : "", "CSR_ALLOW_ANY_RECOVERY_OS"));
  if (csrCfg & CSR_ALLOW_UNAPPROVED_KEXTS)
    StrCatS(csrLog, SVALUE_MAX_SIZE/2, PoolPrint(L"%a%a", StrLen(csrLog) ? " | " : "", "CSR_ALLOW_UNAPPROVED_KEXTS"));
    
  if (StrLen(csrLog)) {
    DBG("CSR_CFG: %ls\n", csrLog);
  }

  FreePool(csrLog);
}

//
// Null ConOut OutputString() implementation - for blocking
// text output from boot.efi when booting in graphics mode
//
EFI_STATUS EFIAPI
NullConOutOutputString(IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, IN CONST CHAR16 *String) {
  return EFI_SUCCESS;
}

//
// EFI OS loader functions
//
//EG_PIXEL DarkBackgroundPixel  = { 0x0, 0x0, 0x0, 0xFF };

VOID CheckEmptyFB()
{
  BOOLEAN EmptyFB = (gSettings.IgPlatform == 0x00050000) ||
  (gSettings.IgPlatform == 0x01620007) ||
  (gSettings.IgPlatform == 0x04120004) ||
  (gSettings.IgPlatform == 0x19120001) ||
  (gSettings.IgPlatform == 0x59120003) ||
  (gSettings.IgPlatform == 0x3E910003);
  if (EmptyFB) {
    gPlatformFeature |= PT_FEATURE_HAS_HEADLESS_GPU;
  } else {
    gPlatformFeature &= ~PT_FEATURE_HAS_HEADLESS_GPU;
  }
}

static VOID StartLoader(IN LOADER_ENTRY *Entry)
{
  EFI_STATUS              Status;
  EFI_TEXT_STRING         ConOutOutputString = 0;
  EFI_HANDLE              ImageHandle = NULL;
  EFI_LOADED_IMAGE        *LoadedImage = NULL;
  CONST CHAR8                   *InstallerVersion;
  TagPtr                  dict = NULL;
  UINTN                   i;
  NSVGfont                *font; // , *nextFont;

//  DBG("StartLoader() start\n");
  DbgHeader("StartLoader");
  if (Entry->Settings) {
    DBG("Entry->Settings: %ls\n", Entry->Settings);
    Status = LoadUserSettings(SelfRootDir, Entry->Settings, &dict);
    if (!EFI_ERROR(Status)) {
      DBG(" - found custom settings for this entry: %ls\n", Entry->Settings);
      gBootChanged = TRUE;
      Status = GetUserSettings(SelfRootDir, dict);
      if (EFI_ERROR(Status)) {
        DBG(" - ... but: %s\n", strerror(Status));
      } else {
        if ((gSettings.CpuFreqMHz > 100) && (gSettings.CpuFreqMHz < 20000)) {
          gCPUStructure.MaxSpeed      = gSettings.CpuFreqMHz;
        }
        //CopyMem (Entry->KernelAndKextPatches,
        //         &gSettings.KernelAndKextPatches,
        //         sizeof(KERNEL_AND_KEXT_PATCHES));
        //DBG("Custom KernelAndKextPatches copyed to started entry\n");
      }
    } else {
      DBG(" - [!] LoadUserSettings failed: %s\n", strerror(Status));
    }
  }
  
	DBG("Finally: ExternalClock=%lluMHz BusSpeed=%llukHz CPUFreq=%uMHz",
  				DivU64x32(gCPUStructure.ExternalClock + kilo - 1, kilo),
  				DivU64x32(gCPUStructure.FSBFrequency + kilo - 1, kilo),
          gCPUStructure.MaxSpeed);
  if (gSettings.QPI) {
	  DBG(" QPI: hw.busfrequency=%lluHz\n", MultU64x32(gSettings.QPI, Mega));
  } else {
    // to match the value of hw.busfrequency in the terminal
	  DBG(" PIS: hw.busfrequency=%lluHz\n", MultU64x32(LShiftU64(DivU64x32(gCPUStructure.ExternalClock + kilo - 1, kilo), 2), Mega));
  }
  
  //Free memory
  for (i = 0; i < ThemesNum; i++) {
    if (ThemesList[i]) {
      FreePool(ThemesList[i]);
      ThemesList[i] = NULL;
    }
  }
  for (i = 0; i < ConfigsNum; i++) {
    if (ConfigsList[i]) {
      FreePool(ConfigsList[i]);
      ConfigsList[i] = NULL;
    }
  }
  for (i = 0; i < DsdtsNum; i++) {
    if (DsdtsList[i]) {
      FreePool(DsdtsList[i]);
      DsdtsList[i] = NULL;
    }
  }
  OptionMenu.FreeMenu();
  //there is a place to free memory
  // GuiAnime
  // mainParser
  // BuiltinIcons
  // OSIcons
  NSVGfontChain *fontChain = fontsDB;
  while (fontChain) {
    font = fontChain->font;
    NSVGfontChain *nextChain = fontChain->next;
    if (font) {
      nsvg__deleteFont(font);
      fontChain->font = NULL;
    }
    FreePool(fontChain);
    fontChain = nextChain;
  }
  fontsDB = NULL;
//  nsvg__deleteParser(mainParser); //temporary disabled
  //destruct_globals_objects(NULL); //we can't destruct our globals here. We need, for example, Volumes.
  
  //DumpKernelAndKextPatches(Entry->KernelAndKextPatches);
  DBG("start loader\n");
  // Load image into memory (will be started later)
  Status = LoadEFIImage(Entry->DevicePath, Basename(Entry->LoaderPath), NULL, &ImageHandle);
  if (EFI_ERROR(Status)) {
    DBG("Image is not loaded, status=%s\n", strerror(Status));
    return; // no reason to continue if loading image failed
  }
  egClearScreen(&Entry->BootBgColor); //if not set then it is already MenuBackgroundPixel

//  KillMouse();

//  if (Entry->LoaderType == OSTYPE_OSX) {
  if (OSTYPE_IS_OSX(Entry->LoaderType) ||
      OSTYPE_IS_OSX_RECOVERY(Entry->LoaderType) ||
      OSTYPE_IS_OSX_INSTALLER(Entry->LoaderType)) {

    // To display progress bar properly (especially in FV2 mode) boot.efi needs to be in graphics mode.
    // Unfortunately many UEFI implementations change the resolution when SetMode happens.
    // This is not what boot.efi expects, and it freely calls SetMode at its will.
    // As a result we see progress bar at improper resolution and the background is also missing (10.12.x+).
    //
    // libeg already has a workaround for SetMode behaviour, so we extend it for boot.efi support.
    // The approach tries to be  follows:
    // 1. Ensure we have graphics mode set (since it is a must in the future).
    // 2. Request text mode for boot.efi, which it expects by default (here a SetMode libeg hack will trigger
    //    on problematic UEFI implementations like AMI).
    egSetGraphicsModeEnabled(TRUE);
    egSetGraphicsModeEnabled(FALSE);

    DBG("GetOSVersion:");

    //needed for boot.efi patcher
    Status = gBS->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &LoadedImage);
    // Correct OSVersion if it was not found
    // This should happen only for 10.7-10.9 OSTYPE_OSX_INSTALLER
    // For these cases, take OSVersion from loaded boot.efi image in memory
    if (/*Entry->LoaderType == OSTYPE_OSX_INSTALLER ||*/ !Entry->OSVersion) {

      if (!EFI_ERROR(Status)) {
        // version in boot.efi appears as "Mac OS X 10.?"
        /*
          Start OSName Mac OS X 10.12 End OSName Start OSVendor Apple Inc. End
        */
  //      InstallerVersion = SearchString((CHAR8*)LoadedImage->ImageBase, LoadedImage->ImageSize, "Mac OS X ", 9);
        InstallerVersion = AsciiStrStr((CHAR8*)LoadedImage->ImageBase, "Mac OS X ");
        if (InstallerVersion != NULL) { // string was found
          InstallerVersion += 9; // advance to version location

          if (AsciiStrnCmp(InstallerVersion, "10.7", 4) &&
              AsciiStrnCmp(InstallerVersion, "10.8", 4) &&
              AsciiStrnCmp(InstallerVersion, "10.9", 4) &&
              AsciiStrnCmp(InstallerVersion, "10.10", 5) &&
              AsciiStrnCmp(InstallerVersion, "10.11", 5) &&
              AsciiStrnCmp(InstallerVersion, "10.12", 5) &&
              AsciiStrnCmp(InstallerVersion, "10.13", 5) &&
              AsciiStrnCmp(InstallerVersion, "10.14", 5) &&
              AsciiStrnCmp(InstallerVersion, "10.15", 5)) {   
            InstallerVersion = NULL; // flag known version was not found
          }
          if (InstallerVersion != NULL) { // known version was found in image
            if (Entry->OSVersion != NULL) {
              FreePool(Entry->OSVersion);
            }
            Entry->OSVersion = (__typeof__(Entry->OSVersion))AllocateCopyPool(AsciiStrLen(InstallerVersion)+1, InstallerVersion);
            Entry->OSVersion[AsciiStrLen(InstallerVersion)] = '\0';
//            DBG("Corrected OSVersion: %s\n", Entry->OSVersion);
          }
        }
      }

      if (Entry->BuildVersion != NULL) {
        FreePool(Entry->BuildVersion);
        Entry->BuildVersion = NULL;
      }
    }

    if (Entry->BuildVersion != NULL) {
      DBG(" %s (%s)\n", Entry->OSVersion, Entry->BuildVersion);
    } else {
      DBG(" %s\n", Entry->OSVersion);
    }

    if (Entry->OSVersion && (AsciiOSVersionToUint64(Entry->OSVersion) >= AsciiOSVersionToUint64("10.11"))) {
      if (OSFLAG_ISSET(Entry->Flags, OSFLAG_NOSIP)) {
        gSettings.CsrActiveConfig = (UINT32)0x37F;
        gSettings.BooterConfig = 0x28;
      }
      ReadSIPCfg();
    }

    FilterKextPatches(Entry);
    FilterKernelPatches(Entry);
    FilterBootPatches(Entry);
    if (LoadedImage && !BooterPatch((UINT8*)LoadedImage->ImageBase, LoadedImage->ImageSize, Entry)) {
      DBG("Will not patch boot.efi\n");
    }

    // Set boot argument for kernel if no caches, this should force kernel loading
    if (  OSFLAG_ISSET(Entry->Flags, OSFLAG_NOCACHES)  &&  !Entry->LoadOptions.containsIC("Kernel="_XS)  ) {
      XString KernelLocation;

      if (Entry->OSVersion && AsciiOSVersionToUint64(Entry->OSVersion) <= AsciiOSVersionToUint64("10.9")) {
        KernelLocation.SPrintf("\"Kernel=/mach_kernel\"");
      } else {
        // used for 10.10, 10.11, and new version.
        KernelLocation.SPrintf("\"Kernel=/System/Library/Kernels/kernel\"");
      }
      Entry->LoadOptions = AddLoadOption(Entry->LoadOptions, KernelLocation);
    }

    //we are booting OSX - restore emulation if it's not installed before g boot.efi
    if (gEmuVariableControl != NULL) {
        gEmuVariableControl->InstallEmulation(gEmuVariableControl);
    }

    // first patchACPI and find PCIROOT and RTC
    // but before ACPI patch we need smbios patch
	  CheckEmptyFB();
    PatchSmbios();
//    DBG("PatchACPI\n");
    PatchACPI(Entry->Volume, Entry->OSVersion);

    // If KPDebug is true boot in verbose mode to see the debug messages
    if ((Entry->KernelAndKextPatches != NULL) && Entry->KernelAndKextPatches->KPDebug) {
      Entry->LoadOptions = AddLoadOption(Entry->LoadOptions, "-v"_XS);
    }

    DbgHeader("RestSetup macOS");

//    DBG("SetDevices\n");
    SetDevices(Entry);
//    DBG("SetFSInjection\n");
    SetFSInjection(Entry);
    //PauseForKey(L"SetFSInjection");
//    DBG("SetVariablesForOSX\n");
    SetVariablesForOSX(Entry);
//    DBG("SetVariablesForOSX\n");
    EventsInitialize(Entry);
//    DBG("FinalizeSmbios\n");
    FinalizeSmbios();

    SetCPUProperties();

    if (OSFLAG_ISSET(Entry->Flags, OSFLAG_HIBERNATED)) {
      DoHibernateWake = PrepareHibernation(Entry->Volume);
    }
    SetupDataForOSX(DoHibernateWake);
    

    if (  gDriversFlags.AptioFixLoaded &&
          !DoHibernateWake &&
          !Entry->LoadOptions.contains("slide=")  ) {
      // Add slide=0 argument for ML+ if not present
      Entry->LoadOptions = AddLoadOption(Entry->LoadOptions, "slide=0"_XS);
    }
     
      
    /**
     * syscl - append "-xcpm" argument conditionally if set KernelXCPM on Intel Haswell+ low-end CPUs
     */
    if ((Entry->KernelAndKextPatches != NULL) && Entry->KernelAndKextPatches->KPKernelXCPM &&
        gCPUStructure.Vendor == CPU_VENDOR_INTEL && gCPUStructure.Model >= CPU_MODEL_HASWELL &&
       (AsciiStrStr(gCPUStructure.BrandString, "Celeron") || AsciiStrStr(gCPUStructure.BrandString, "Pentium")) &&
       (AsciiOSVersionToUint64(Entry->OSVersion) >= AsciiOSVersionToUint64("10.8.5")) &&
       (AsciiOSVersionToUint64(Entry->OSVersion) < AsciiOSVersionToUint64("10.12")) &&
       (!Entry->LoadOptions.contains("-xcpm"_XS))) {
        // add "-xcpm" argv if not present on Haswell+ Celeron/Pentium
        Entry->LoadOptions = AddLoadOption(Entry->LoadOptions, "-xcpm"_XS);
    }
    
    // add -xcpm on Ivy Bridge if set KernelXCPM and system version is 10.8.5 - 10.11.x
    if ((Entry->KernelAndKextPatches != NULL) && Entry->KernelAndKextPatches->KPKernelXCPM &&
        gCPUStructure.Model == CPU_MODEL_IVY_BRIDGE &&
        (AsciiOSVersionToUint64(Entry->OSVersion) >= AsciiOSVersionToUint64("10.8.5")) &&
        (AsciiOSVersionToUint64(Entry->OSVersion) < AsciiOSVersionToUint64("10.12")) &&
        (!Entry->LoadOptions.contains("-xcpm"))) {
      // add "-xcpm" argv if not present on Ivy Bridge
      Entry->LoadOptions = AddLoadOption(Entry->LoadOptions, "-xcpm"_XS);
    }

    if (AudioIo) {
      AudioIo->StopPlayback(AudioIo);
    }

//    DBG("Set FakeCPUID: 0x%X\n", gSettings.FakeCPUID);
//    DBG("LoadKexts\n");
    // LoadKexts writes to DataHub, where large writes can prevent hibernate wake (happens when several kexts present in Clover's kexts dir)
    if (!DoHibernateWake) {
      LoadKexts(Entry);
    }

    // blocking boot.efi output if -v is not specified
    // note: this blocks output even if -v is specified in
    // /Library/Preferences/SystemConfiguration/com.apple.Boot.plist
    // which is wrong
    // apianti - only block console output if using graphics
    //           but don't block custom boot logo
    if (  Entry->LoadOptions.containsIC("-v"_XS)  ) {
          Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_USEGRAPHICS);
	} else if ( Entry->LoadOptions.isEmpty() ) {
	  Entry->LoadOptions = AddLoadOption(Entry->LoadOptions, " "_XS);
	}
  }
  else if (OSTYPE_IS_WINDOWS(Entry->LoaderType)) {

    if (AudioIo) {
        AudioIo->StopPlayback(AudioIo);
    }

    DBG("Closing events for Windows\n");
    gBS->CloseEvent (OnReadyToBootEvent);
    gBS->CloseEvent (ExitBootServiceEvent);
    gBS->CloseEvent (mSimpleFileSystemChangeEvent);


    if (gEmuVariableControl != NULL) {
      gEmuVariableControl->UninstallEmulation(gEmuVariableControl);
    }

    PatchACPI_OtherOS(L"Windows", FALSE);
    //PauseForKey(L"continue");

  }
  else if (OSTYPE_IS_LINUX(Entry->LoaderType) || (Entry->LoaderType == OSTYPE_LINEFI)) {

    DBG("Closing events for Linux\n");
    gBS->CloseEvent (OnReadyToBootEvent);
    gBS->CloseEvent (ExitBootServiceEvent);
    gBS->CloseEvent (mSimpleFileSystemChangeEvent);

    if (gEmuVariableControl != NULL) {
      gEmuVariableControl->UninstallEmulation(gEmuVariableControl);
    }
    //FinalizeSmbios();
    PatchACPI_OtherOS(L"Linux", FALSE);
    //PauseForKey(L"continue");
  }

  if (gSettings.LastBootedVolume) {
    SetStartupDiskVolume(Entry->Volume, Entry->LoaderType == OSTYPE_OSX ? NULL : Entry->LoaderPath);
  } else if (gSettings.DefaultVolume != NULL) {
    // DefaultVolume specified in Config.plist or in Boot Option
    // we'll remove macOS Startup Disk vars which may be present if it is used
    // to reboot into another volume
    RemoveStartupDiskVolume();
  }
/*
  {
    //    UINT32                    machineSignature    = 0;
    EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE     *FadtPointer = NULL;
    EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE  *Facs = NULL;

    //    DBG("---dump hibernations data---\n");
    FadtPointer = GetFadt();
    if (FadtPointer != NULL) {
      Facs = (EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE*)(UINTN)(FadtPointer->FirmwareCtrl);

      DBG("  Firmware wake address=%08lx\n", Facs->FirmwareWakingVector);
      DBG("  Firmware wake 64 addr=%16llx\n",  Facs->XFirmwareWakingVector);
      DBG("  Hardware signature   =%08lx\n", Facs->HardwareSignature);
      DBG("  GlobalLock           =%08lx\n", Facs->GlobalLock);
      DBG("  Flags                =%08lx\n", Facs->Flags);
      DBG(" HS at offset 0x%08X\n", OFFSET_OF(EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE, HardwareSignature));
      //   machineSignature = Facs->HardwareSignature;
    }
  }
*/

//    DBG("BeginExternalScreen\n");
  BeginExternalScreen(OSFLAG_ISSET(Entry->Flags, OSFLAG_USEGRAPHICS)/*, L"Booting OS"*/);

  if (!OSTYPE_IS_WINDOWS(Entry->LoaderType)) {
    if (OSFLAG_ISSET(Entry->Flags, OSFLAG_USEGRAPHICS)) {
      // save orig OutputString and replace it with
      // null implementation
      ConOutOutputString = gST->ConOut->OutputString;
      gST->ConOut->OutputString = NullConOutOutputString;
    }
    
    // Initialize the boot screen
    if (EFI_ERROR(Status = InitBootScreen(Entry))) {
      if (Status != EFI_ABORTED) DBG("Failed to initialize custom boot screen: %s!\n", strerror(Status));
    }
    else if (EFI_ERROR(Status = LockBootScreen())) {
      DBG("Failed to lock custom boot screen: %s!\n", strerror(Status));
    }
  } // !OSTYPE_IS_WINDOWS

  if (OSTYPE_IS_OSX(Entry->LoaderType) ||
      OSTYPE_IS_OSX_RECOVERY(Entry->LoaderType) ||
      OSTYPE_IS_OSX_INSTALLER(Entry->LoaderType)) {

    if (DoHibernateWake) {
      DBG("Closing events for wake\n");
      gBS->CloseEvent (OnReadyToBootEvent);
      gBS->CloseEvent (ExitBootServiceEvent);
      gBS->CloseEvent (mSimpleFileSystemChangeEvent);
//      gBS->CloseEvent (mVirtualAddressChangeEvent);
      // When doing hibernate wake, save to DataHub only up to initial size of log
      SavePreBootLog = FALSE;
    } else {
      // delete boot-switch-vars if exists
      Status = gRT->SetVariable(L"boot-switch-vars", &gEfiAppleBootGuid,
                                EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                0, NULL);
      DeleteNvramVariable(L"IOHibernateRTCVariables", &gEfiAppleBootGuid);
      DeleteNvramVariable(L"boot-image",              &gEfiAppleBootGuid);

    }
    SetupBooterLog(!DoHibernateWake);
  }


  
  DBG("Closing log\n");
  if (SavePreBootLog) {
    Status = SaveBooterLog(SelfRootDir, PREBOOT_LOG);
    if (EFI_ERROR(Status)) {
      /*Status = */SaveBooterLog(NULL, PREBOOT_LOG);
    }
  }

//  DBG("StartEFIImage\n");
//  StartEFIImage(Entry->DevicePath, Entry->LoadOptions,
//                Basename(Entry->LoaderPath), Basename(Entry->LoaderPath), NULL, NULL);

//  DBG("StartEFILoadedImage\n");
  StartEFILoadedImage(ImageHandle, Entry->LoadOptions,
                Basename(Entry->LoaderPath), Basename(Entry->LoaderPath), NULL);
  // Unlock boot screen
  if (EFI_ERROR(Status = UnlockBootScreen())) {
    DBG("Failed to unlock custom boot screen: %s!\n", strerror(Status));
  }
  if (OSFLAG_ISSET(Entry->Flags, OSFLAG_USEGRAPHICS)) {
    // return back orig OutputString
    gST->ConOut->OutputString = ConOutOutputString;
  }

//  PauseForKey(L"FinishExternalScreen");
  FinishExternalScreen();
//  PauseForKey(L"System started?!");
}

#define MAX_DISCOVERED_PATHS (16)
//#define PREBOOT_LOG L"EFI\\CLOVER\\misc\\preboot.log"

static VOID StartLegacy(IN LEGACY_ENTRY *Entry)
{
    EFI_STATUS          Status = EFI_UNSUPPORTED;

    // Unload EmuVariable before booting legacy.
    // This is not needed in most cases, but it seems to interfere with legacy OS
    // booted on some UEFI bioses, such as Phoenix UEFI 2.0
    if (gEmuVariableControl != NULL) {
      gEmuVariableControl->UninstallEmulation(gEmuVariableControl);
    }

    if (gSettings.LastBootedVolume) {
      SetStartupDiskVolume(Entry->Volume, NULL);
    } else if (gSettings.DefaultVolume != NULL) {
      // DefaultVolume specified in Config.plist:
      // we'll remove macOS Startup Disk vars which may be present if it is used
      // to reboot into another volume
      RemoveStartupDiskVolume();
    }


  egClearScreen(&MenuBackgroundPixel);
  BeginExternalScreen(TRUE/*, L"Booting Legacy OS"*/);
  XImage BootLogoX;
  BootLogoX.LoadXImage(ThemeX.ThemeDir, Entry->Volume->LegacyOS->IconName);
  BootLogoX.Draw((UGAWidth  - BootLogoX.GetWidth()) >> 1,
                 (UGAHeight - BootLogoX.GetHeight()) >> 1);

      //try my LegacyBoot
      switch (Entry->Volume->BootType) {
        case BOOTING_BY_CD:
          Status = bootElTorito(Entry->Volume);
          break;
        case BOOTING_BY_MBR:
          Status = bootMBR(Entry->Volume);
          break;
        case BOOTING_BY_PBR:
          if (StrCmp(gSettings.LegacyBoot, L"LegacyBiosDefault") == 0) {
            Status = bootLegacyBiosDefault(gSettings.LegacyBiosDefaultEntry);
          } else if (StrCmp(gSettings.LegacyBoot, L"PBRtest") == 0) {
            Status = bootPBRtest(Entry->Volume);
          } else if (StrCmp(gSettings.LegacyBoot, L"PBRsata") == 0) {
            Status = bootPBR(Entry->Volume, TRUE);
          } else {
            // default
            Status = bootPBR(Entry->Volume, FALSE);
          }
          break;
        default:
          break;
      }
      CheckError(Status, L"while LegacyBoot");

  FinishExternalScreen();
}

//
// pre-boot tool functions
//

static VOID StartTool(IN REFIT_MENU_ENTRY_LOADER_TOOL *Entry)
{
  DBG("Start Tool: %ls\n", Entry->LoaderPath);
  egClearScreen(&MenuBackgroundPixel);
	// assumes "Start <title>" as assigned below
	BeginExternalScreen(OSFLAG_ISSET(Entry->Flags, OSFLAG_USEGRAPHICS)/*, &Entry->Title[6]*/); // Shouldn't we check that length of Title is at least 6 ?
    StartEFIImage(Entry->DevicePath, Entry->LoadOptions, Basename(Entry->LoaderPath), Basename(Entry->LoaderPath), NULL, NULL);
    FinishExternalScreen();
	//ReinitSelfLib();
}

//
// pre-boot driver functions
//

static VOID ScanDriverDir(IN CONST CHAR16 *Path, OUT EFI_HANDLE **DriversToConnect, OUT UINTN *DriversToConnectNum)
{
  EFI_STATUS              Status;
  REFIT_DIR_ITER          DirIter;
  EFI_FILE_INFO           *DirEntry;
  CHAR16                  FileName[256];
  EFI_HANDLE              DriverHandle;
  EFI_DRIVER_BINDING_PROTOCOL  *DriverBinding;
  UINTN                   DriversArrSize;
  UINTN                   DriversArrNum;
  EFI_HANDLE              *DriversArr;
  INTN                    i;
  BOOLEAN                 Skip;
  UINT8                   AptioBlessed;
  STATIC CHAR16 CONST * CONST AptioNames[] = {
    L"AptioMemoryFix",
    L"AptioFix3Drv",
    L"AptioFix2Drv",
    L"AptioFixDrv",
    L"LowMemFix"
  };
  STATIC UINT8 CONST AptioIndices[] = {
    OFFSET_OF(DRIVERS_FLAGS, AptioMemFixLoaded),
    OFFSET_OF(DRIVERS_FLAGS, AptioFix3Loaded),
    OFFSET_OF(DRIVERS_FLAGS, AptioFix2Loaded),
    OFFSET_OF(DRIVERS_FLAGS, AptioFixLoaded),
    OFFSET_OF(DRIVERS_FLAGS, MemFixLoaded)
  };

  DriversArrSize = 0;
  DriversArrNum = 0;
  DriversArr = NULL;

//only one driver with highest priority will obtain status "Loaded"
  DirIterOpen(SelfRootDir, Path, &DirIter);
#define BOOLEAN_AT_INDEX(k) (*(BOOLEAN*)((UINTN)&gDriversFlags + AptioIndices[(k)]))
  for (i = 0; i != ARRAY_SIZE(AptioIndices); ++i)
    BOOLEAN_AT_INDEX(i) = FALSE;
  AptioBlessed = (UINT8) ARRAY_SIZE(AptioNames);
  while (DirIterNext(&DirIter, 2, L"*.efi", &DirEntry)) {
    for (i = 0; i != ARRAY_SIZE(AptioNames); ++i)
      if (StrStr(DirEntry->FileName, AptioNames[i]) != NULL)
        break;
    if (((UINT8) i) >= AptioBlessed)
      continue;
    AptioBlessed = (UINT8) i;
    if (!i)
      break;
  }
  DirIterClose(&DirIter);

  // look through contents of the directory
  DirIterOpen(SelfRootDir, Path, &DirIter);
  while (DirIterNext(&DirIter, 2, L"*.efi", &DirEntry)) {
    Skip = (DirEntry->FileName[0] == L'.');
    for (i=0; i<gSettings.BlackListCount; i++) {
      if (StrStr(DirEntry->FileName, gSettings.BlackList[i]) != NULL) {
        Skip = TRUE;   // skip this
        break;
      }
    }
    if (Skip) {
      continue;
    }

    // either AptioMem, AptioFix* or LowMemFix exclusively
    for (i = 0; i != ARRAY_SIZE(AptioNames); ++i)
      if (StrStr(DirEntry->FileName, AptioNames[i]) != NULL)
        break;
    if (i != ARRAY_SIZE(AptioNames)) {
      if (((UINT8) i) != AptioBlessed)
        continue;
      if (AptioBlessed < (UINT8) ARRAY_SIZE(AptioIndices))
        BOOLEAN_AT_INDEX(AptioBlessed) = TRUE;
      AptioBlessed = (UINT8) ARRAY_SIZE(AptioNames);
    }
#undef BOOLEAN_AT_INDEX

	  snwprintf(FileName, 512, "%ls\\%ls", Path, DirEntry->FileName);
    Status = StartEFIImage(FileDevicePath(SelfLoadedImage->DeviceHandle, FileName),
                           ""_XS, DirEntry->FileName, DirEntry->FileName, NULL, &DriverHandle);
    if (EFI_ERROR(Status)) {
      continue;
    }
    if (StrStr(FileName, L"EmuVariable") != NULL) {
      gDriversFlags.EmuVariableLoaded = TRUE;
    } else if (StrStr(FileName, L"Video") != NULL) {
      gDriversFlags.VideoLoaded = TRUE;
    } else if (StrStr(FileName, L"Partition") != NULL) {
      gDriversFlags.PartitionLoaded = TRUE;
    } else if (StrStr(FileName, L"HFS") != NULL) {
      gDriversFlags.HFSLoaded = TRUE;
    } else if (StriStr(FileName, L"apfs") != NULL) {
      gDriversFlags.APFSLoaded = TRUE;
    }
    if (DriverHandle != NULL && DriversToConnectNum != NULL && DriversToConnect != NULL) {
      // driver loaded - check for EFI_DRIVER_BINDING_PROTOCOL
      Status = gBS->HandleProtocol(DriverHandle, &gEfiDriverBindingProtocolGuid, (VOID **) &DriverBinding);
      if (!EFI_ERROR(Status) && DriverBinding != NULL) {
        DBG(" - driver needs connecting\n");
        // standard UEFI driver - we would reconnect after loading - add to array
        if (DriversArrSize == 0) {
          // new array
          DriversArrSize = 16;
          DriversArr = (__typeof__(DriversArr))AllocateZeroPool(sizeof(EFI_HANDLE) * DriversArrSize);
        } else if (DriversArrNum + 1 == DriversArrSize) {
          // extend array
          DriversArr = (__typeof__(DriversArr))ReallocatePool(DriversArrSize, DriversArrSize + 16, DriversArr);
          DriversArrSize += 16;
        }
        DriversArr[DriversArrNum] = DriverHandle;
 //       DBG(" driver %ls included with Binding=%X\n", FileName, DriverBinding);
        DriversArrNum++;
        // we'll make array terminated
        DriversArr[DriversArrNum] = NULL;
      }
    }
  }
  Status = DirIterClose(&DirIter);
  if (Status != EFI_NOT_FOUND) {
	  snwprintf(FileName, 512, "while scanning the %ls directory", Path);
    CheckError(Status, FileName);
  }

  if (DriversToConnectNum != NULL && DriversToConnect != NULL) {
    *DriversToConnectNum = DriversArrNum;
    *DriversToConnect = DriversArr;
  }
//release memory for BlackList
  for (i=0; i<gSettings.BlackListCount; i++) {
    if (gSettings.BlackList[i]) {
      FreePool(gSettings.BlackList[i]);
      gSettings.BlackList[i] = NULL;
    }
  }
}


/**
 * Some UEFI's (like HPQ EFI from HP notebooks) have DiskIo protocols
 * opened BY_DRIVER (by Partition driver in HP case) even when no file system
 * is produced from this DiskIo. This then blocks our FS drivers from connecting
 * and producing file systems.
 * To fix it: we will disconnect drivers that connected to DiskIo BY_DRIVER
 * if this is partition volume and if those drivers did not produce file system.
 */
VOID DisconnectInvalidDiskIoChildDrivers(VOID)
{
  EFI_STATUS                            Status;
  UINTN                                 HandleCount = 0;
  UINTN                                 Index;
  UINTN                                 OpenInfoIndex;
  EFI_HANDLE                            *Handles = NULL;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL       *Fs;
  EFI_BLOCK_IO_PROTOCOL                 *BlockIo;
  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY   *OpenInfo;
  UINTN                                 OpenInfoCount;
  BOOLEAN                               Found;

  DBG("Searching for invalid DiskIo BY_DRIVER connects:");

  //
  // Get all DiskIo handles
  //
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiDiskIoProtocolGuid, NULL, &HandleCount, &Handles);
  if (EFI_ERROR(Status) || HandleCount == 0) {
    DBG(" no DiskIo handles\n");
    return;
  }

  //
  // Check every DiskIo handle
  //
  Found = FALSE;
  for (Index = 0; Index < HandleCount; Index++) {
    //DBG("\n");
    //DBG(" - Handle %p:", Handles[Index]);
    //
    // If this is not partition - skip it.
    // This is then whole disk and DiskIo
    // should be opened here BY_DRIVER by Partition driver
    // to produce partition volumes.
    //
    Status = gBS->HandleProtocol (
                                  Handles[Index],
                                  &gEfiBlockIoProtocolGuid,
                                  (VOID **) &BlockIo
                                  );
    if (EFI_ERROR(Status)) {
      //DBG(" BlockIo: %s - skipping\n", strerror(Status));
      continue;
    }
    if (BlockIo->Media == NULL) {
      //DBG(" BlockIo: no media - skipping\n");
      continue;

    }
    if (!BlockIo->Media->LogicalPartition) {
      //DBG(" BlockIo: whole disk - skipping\n");
      continue;

    }
    //DBG(" BlockIo: partition");

    //
    // If SimpleFileSystem is already produced - skip it, this is ok
    //
    Status = gBS->HandleProtocol (
                                  Handles[Index],
                                  &gEfiSimpleFileSystemProtocolGuid,
                                  (VOID **) &Fs
                                  );
    if (Status == EFI_SUCCESS) {
      //DBG(" FS: ok - skipping\n");
      continue;
    }
    //DBG(" FS: no");

    //
    // If no SimpleFileSystem on this handle but DiskIo is opened BY_DRIVER
    // then disconnect this connection
    //
    Status = gBS->OpenProtocolInformation (
                                           Handles[Index],
                                           &gEfiDiskIoProtocolGuid,
                                           &OpenInfo,
                                           &OpenInfoCount
                                           );
    if (EFI_ERROR(Status)) {
      //DBG(" OpenInfo: no - skipping\n");
      continue;
    }
    //DBG(" OpenInfo: %d", OpenInfoCount);
    for (OpenInfoIndex = 0; OpenInfoIndex < OpenInfoCount; OpenInfoIndex++) {
      if ((OpenInfo[OpenInfoIndex].Attributes & EFI_OPEN_PROTOCOL_BY_DRIVER) == EFI_OPEN_PROTOCOL_BY_DRIVER) {
        if (!Found) {
          DBG("\n");
        }
        Found = TRUE;
        Status = gBS->DisconnectController (Handles[Index], OpenInfo[OpenInfoIndex].AgentHandle, NULL);
        //DBG(" BY_DRIVER Agent: %p, Disconnect: %s", OpenInfo[OpenInfoIndex].AgentHandle, strerror(Status));
        DBG(" - Handle %p with DiskIo, is Partition, no Fs, BY_DRIVER Agent: %p, Disconnect: %s\n", Handles[Index], OpenInfo[OpenInfoIndex].AgentHandle, strerror(Status));
      }
    }
    FreePool(OpenInfo);
  }
  FreePool(Handles);

  if (!Found) {
    DBG(" not found, all ok\n");
  }
}

VOID DisconnectSomeDevices(VOID)
{
  EFI_STATUS              Status;
  UINTN                   HandleCount;
  UINTN                   Index, Index2;
  EFI_HANDLE              *Handles ;
  EFI_HANDLE              *ControllerHandles;
  UINTN                   ControllerHandleCount;
  EFI_BLOCK_IO_PROTOCOL   *BlockIo  = NULL;
//  EFI_DISK_IO_PROTOCOL    *DiskIo = NULL;
  EFI_PCI_IO_PROTOCOL     *PciIo  = NULL;
//  EFI_FILE_PROTOCOL       *RootFP = NULL;
//  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *VolumeFS = NULL;
  PCI_TYPE00              Pci;
  CHAR16                           *DriverName;
  EFI_COMPONENT_NAME_PROTOCOL      *CompName;

  if (gDriversFlags.PartitionLoaded) {
    DBG("Partition driver loaded: ");
    // get all BlockIo handles
    HandleCount = 0;
    Handles = NULL;

    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &HandleCount, &Handles);
    if (Status == EFI_SUCCESS) {
      for (Index = 0; Index < HandleCount; Index++) {
        Status = gBS->HandleProtocol(Handles[Index], &gEfiBlockIoProtocolGuid, (VOID **) &BlockIo);
        if (EFI_ERROR(Status)) {
          continue;
        }
        if (BlockIo->Media->BlockSize == 2048) {
          // disconnect CD controller
          Status = gBS->DisconnectController(Handles[Index], NULL, NULL);
          DBG("CD disconnect %s", strerror(Status));
        }
      }
/*      for (Index = 0; Index < HandleCount; Index++) {
        Status = gBS->DisconnectController(Handles[Index], NULL, NULL);
      } */
      FreePool(Handles);
    }
    DBG("\n");
  }

  if ((gDriversFlags.HFSLoaded) || (gDriversFlags.APFSLoaded)) {
    if (gDriversFlags.HFSLoaded) {
      DBG("HFS+ driver loaded\n");
    }
    if (gDriversFlags.APFSLoaded) {
      DBG("APFS driver loaded\n");
    }

    // get all FileSystem handles
    ControllerHandleCount = 0;
    ControllerHandles = NULL;

    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &ControllerHandleCount, &ControllerHandles);
 /*   if (!EFI_ERROR(Status)) {
      for (Index2 = 0; Index2 < ControllerHandleCount; Index2++) {
        Status = gBS->DisconnectController(ControllerHandles[Index2],
                                           NULL, NULL);
        DBG("Driver [%d] disconnect %s\n", Index2, strerror(Status));
      }
    } */

    HandleCount = 0;
    Handles = NULL;

    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiComponentNameProtocolGuid, NULL, &HandleCount, &Handles);
    if (!EFI_ERROR(Status)) {
      for (Index = 0; Index < HandleCount; Index++) {
        Status = gBS->OpenProtocol(
                                   Handles[Index],
                                   &gEfiComponentNameProtocolGuid,
                                   (VOID**)&CompName,
                                   gImageHandle,
                                   NULL,
                                   EFI_OPEN_PROTOCOL_GET_PROTOCOL);

        if (EFI_ERROR(Status)) {
//          DBG("CompName %s\n", strerror(Status));
          continue;
        }
        Status = CompName->GetDriverName(CompName, "eng", &DriverName);
        if (EFI_ERROR(Status)) {
          continue;
        }
        if ((StriStr(DriverName, L"HFS")) || (StriStr(DriverName, L"apfs"))) {
          for (Index2 = 0; Index2 < ControllerHandleCount; Index2++) {
            Status = gBS->DisconnectController(ControllerHandles[Index2],
                                               Handles[Index], NULL);
//            DBG("Disconnect [%ls] from %X: %s\n", DriverName, ControllerHandles[Index2], strerror(Status));
          }
        }
      }
      FreePool(Handles);
    }
//    DBG("\n");
    FreePool(ControllerHandles);
  }


  if (gDriversFlags.VideoLoaded) {
    DBG("Video driver loaded: ");
    // get all PciIo handles
    HandleCount = 0;
    Handles = NULL;
    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiPciIoProtocolGuid, NULL, &HandleCount, &Handles);
    if (Status == EFI_SUCCESS) {
      for (Index = 0; Index < HandleCount; Index++) {
        Status = gBS->HandleProtocol(Handles[Index], &gEfiPciIoProtocolGuid, (VOID **) &PciIo);
        if (EFI_ERROR(Status)) {
          continue;
        }
        Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, sizeof (Pci) / sizeof (UINT32), &Pci);
        if (!EFI_ERROR(Status)) {
          if(IS_PCI_VGA(&Pci) == TRUE) {
            // disconnect VGA
            Status = gBS->DisconnectController(Handles[Index], NULL, NULL);
            DBG("disconnect %s", strerror(Status));
          }
        }
      }
      FreePool(Handles);
    }
    DBG("\n");
  }

  if (!gFirmwareClover) {
    DisconnectInvalidDiskIoChildDrivers();
  }
}


VOID PatchVideoBios(UINT8 *Edid)
{

  if (gSettings.PatchVBiosBytesCount > 0 && gSettings.PatchVBiosBytes != NULL) {
    VideoBiosPatchBytes(gSettings.PatchVBiosBytes, gSettings.PatchVBiosBytesCount);
  }

  if (gSettings.PatchVBios) {
    VideoBiosPatchNativeFromEdid(Edid);
  }
}


static VOID LoadDrivers(VOID)
{
  EFI_STATUS  Status;
  EFI_HANDLE  *DriversToConnect = NULL;
  UINTN       DriversToConnectNum = 0;
  UINT8       *Edid;
  UINTN       VarSize = 0;
  BOOLEAN     VBiosPatchNeeded;

  DbgHeader("LoadDrivers");

    // load drivers from /efi/drivers
#if defined(MDE_CPU_X64)
  if (gFirmwareClover) {
    if (FileExists(SelfRootDir, L"\\EFI\\CLOVER\\drivers\\BIOS")) {
      ScanDriverDir(L"\\EFI\\CLOVER\\drivers\\BIOS", &DriversToConnect, &DriversToConnectNum);
    } else {
      ScanDriverDir(L"\\EFI\\CLOVER\\drivers64", &DriversToConnect, &DriversToConnectNum);
    }
  } else
  if (FileExists(SelfRootDir, L"\\EFI\\CLOVER\\drivers\\UEFI")) {
    ScanDriverDir(L"\\EFI\\CLOVER\\drivers\\UEFI", &DriversToConnect, &DriversToConnectNum);
  } else {
    ScanDriverDir(L"\\EFI\\CLOVER\\drivers64UEFI", &DriversToConnect, &DriversToConnectNum);
  }
#else
  ScanDriverDir(L"\\EFI\\CLOVER\\drivers32", &DriversToConnect, &DriversToConnectNum);
#endif

  VBiosPatchNeeded = gSettings.PatchVBios || (gSettings.PatchVBiosBytesCount > 0 && gSettings.PatchVBiosBytes != NULL);
  if (VBiosPatchNeeded) {
    // check if it is already done in CloverEFI BiosVideo
    Status = gRT->GetVariable (
                               L"CloverVBiosPatchDone",
                               &gEfiGlobalVariableGuid,
                               NULL,
                               &VarSize,
                               NULL
                               );
    if (Status == EFI_BUFFER_TOO_SMALL) {
      // var exists - it's done - let's not do it again
      VBiosPatchNeeded = FALSE;
    }
  }

  if (((gSettings.CustomEDID != NULL) && gFirmwareClover) || (VBiosPatchNeeded && !gDriversFlags.VideoLoaded)) {
    // we have video bios patch - force video driver reconnect
    DBG("Video bios patch requested or CustomEDID - forcing video reconnect\n");
    gDriversFlags.VideoLoaded = TRUE;
    DriversToConnectNum++;
  }

  if (DriversToConnectNum > 0) {
	  DBG("%llu drivers needs connecting ...\n", DriversToConnectNum);
    // note: our platform driver protocol
    // will use DriversToConnect - do not release it
    RegisterDriversToHighestPriority(DriversToConnect);
    if (VBiosPatchNeeded) {
      if (gSettings.CustomEDID != NULL) {
        Edid = gSettings.CustomEDID;
      } else {
        Edid = getCurrentEdid();
      }
      DisconnectSomeDevices();
      PatchVideoBios(Edid);
      if (gSettings.CustomEDID == NULL) {
        FreePool(Edid);
      }
    } else {
      DisconnectSomeDevices();
    }
    BdsLibConnectAllDriversToAllControllers();

    // Boot speedup: remove temporary "BiosVideoBlockSwitchMode" RT var
    // to unlock mode switching in CsmVideo
    gRT->SetVariable(L"BiosVideoBlockSwitchMode", &gEfiGlobalVariableGuid, EFI_VARIABLE_BOOTSERVICE_ACCESS, 0, NULL);
  }
}


INTN FindDefaultEntry(VOID)
{
  INTN                Index = -1;
  REFIT_VOLUME        *Volume;
  BOOLEAN             SearchForLoader;

//  DBG("FindDefaultEntry ...\n");
  //DbgHeader("FindDefaultEntry");

  //
  // try to detect volume set by Startup Disk or previous Clover selection
  // with broken nvram this requires emulation to be installed.
  // enable emulation to determin efi-boot-device-data
  if (gEmuVariableControl != NULL) {
    gEmuVariableControl->InstallEmulation(gEmuVariableControl);
  }

  Index = FindStartupDiskVolume(&MainMenu);

  if (Index >= 0) {
	  DBG("Boot redirected to Entry %lld. '%ls'\n", Index, MainMenu.Entries[Index].Title.s());
    // we got boot-device-data, no need to keep emulating anymore
    if (gEmuVariableControl != NULL) {
        gEmuVariableControl->UninstallEmulation(gEmuVariableControl);
    }
    return Index;
  }

  //
  // if not found, then try DefaultVolume from config.plist
  // if not null or empty, search volume that matches gSettings.DefaultVolume
  //
  if (gSettings.DefaultVolume != NULL) {

    // if not null or empty, also search for loader that matches gSettings.DefaultLoader
    SearchForLoader = (gSettings.DefaultLoader != NULL && gSettings.DefaultLoader[0] != L'\0');
/*
    if (SearchForLoader) {
      DBG("Searching for DefaultVolume '%ls', DefaultLoader '%ls' ...\n", gSettings.DefaultVolume, gSettings.DefaultLoader);
    } else {
      DBG("Searching for DefaultVolume '%ls' ...\n", gSettings.DefaultVolume);
    }
*/
    for (Index = 0; Index < (INTN)MainMenu.Entries.size()  &&  MainMenu.Entries[Index].getLOADER_ENTRY()  &&  MainMenu.Entries[Index].getLOADER_ENTRY()->Row == 0 ; Index++) {

      LOADER_ENTRY& Entry = *MainMenu.Entries[Index].getLOADER_ENTRY();
      if (!Entry.Volume) {
        continue;
      }

      Volume = Entry.Volume;
      if ((Volume->VolName == NULL || StrCmp(Volume->VolName, gSettings.DefaultVolume) != 0) &&
          !StrStr(Volume->DevicePathString, gSettings.DefaultVolume)) {
        continue;
      }

      //                       we alreday know that Entry.isLoader
      if (SearchForLoader && (/*Entry.Tag != TAG_LOADER ||*/ !StriStr(Entry.LoaderPath, gSettings.DefaultLoader))) {
        continue;
      }

		DBG(" - found entry %lld. '%ls', Volume '%ls', DevicePath '%ls'\n", Index, Entry.Title.s(), Volume->VolName, Entry.DevicePathString);
      // if first method failed and second succeeded - uninstall emulation
      if (gEmuVariableControl != NULL) {
        gEmuVariableControl->UninstallEmulation(gEmuVariableControl);
      }
      return Index;
    }

  }

  DBG("Default boot entry not found\n");
 // if both methods to determine default boot entry have failed - uninstall emulation before GUI
 if (gEmuVariableControl != NULL) {
    gEmuVariableControl->UninstallEmulation(gEmuVariableControl);
 }
  return -1;
}

VOID SetVariablesFromNvram()
{
  CHAR8  *tmpString;
  UINTN   Size = 0;
  UINTN   index = 0, index2, len, i;
  CHAR8  *arg = NULL;

//  DbgHeader("SetVariablesFromNvram");

  tmpString = (__typeof__(tmpString))GetNvramVariable(L"boot-args", &gEfiAppleBootGuid, NULL, &Size);
  if (tmpString && (Size <= 0x1000) && (Size > 0)) {
	  DBG("found boot-args in NVRAM:%s, size=%llu\n", tmpString, Size);
    // use and forget old one
//    DeleteNvramVariable(L"boot-args", &gEfiAppleBootGuid);
    Size = AsciiStrLen(tmpString); // some EFI implementations include '\0' in Size, and others don't, so update Size to string length
    arg = (__typeof__(arg))AllocatePool(Size+1);
    
/*    if (AsciiStrStr(tmpString, "nvda_drv=1")) { //found substring
      gSettings.NvidiaWeb = TRUE;
    } */
    //first we will find new args that is not present in main args
    index = 0;
    while ((index < Size) && (tmpString[index] != 0x0)) {
      ZeroMem(arg, Size+1);
      index2 = 0;
      if (tmpString[index] != '\"') {
 //       DBG("search space index=%d\n", index);
        while ((index < Size) && (tmpString[index] != 0x20) && (tmpString[index] != 0x0)) {
          arg[index2++] = tmpString[index++];
        }
        DBG("...found arg:%s\n", arg);
      } else {
        index++;
//        DBG("search quote index=%d\n", index);
        while ((index < Size) && (tmpString[index] != '\"') && (tmpString[index] != 0x0)) {
          arg[index2++] = tmpString[index++];
        }
        if (tmpString[index] == '\"') {
          index++;
        }
        DBG("...found quoted arg:\n"/*, arg*/);
      }
      while (tmpString[index] == 0x20) {
        index++;
      }
      // For the moment only arg -s must be ignored
      if (AsciiStrCmp(arg, "-s") == 0) {
        DBG("...ignoring arg:%s\n", arg);
        continue;
      }
      if (!AsciiStrStr(gSettings.BootArgs, arg)) {
        //this arg is not present will add
        DBG("...adding arg:%s\n", arg);
        len = iStrLen(gSettings.BootArgs, 256);
        if (len + index2 > 256) {
          DBG("boot-args overflow... bytes=%llu+%llu\n", len, index2);
          break;
        }
        gSettings.BootArgs[len++] = 0x20;
        for (i = 0; i < index2; i++) {
          gSettings.BootArgs[len++] = arg[i];
        }
        gSettings.BootArgs[len++] = 0x20;
      }
    }
    FreePool(arg);
  }
  if (tmpString) {
    FreePool(tmpString);
  }
  
  tmpString = (__typeof__(tmpString))GetNvramVariable(L"nvda_drv", &gEfiAppleBootGuid, NULL, NULL);
  if (tmpString && AsciiStrCmp(tmpString, "1") == 0) {
    gSettings.NvidiaWeb = TRUE;
  }
  if (tmpString) {
    FreePool(tmpString);
  }

}

extern UINTN                           nLanCards;        // number of LAN cards
extern UINT16                          gLanVendor[4];    // their vendors
extern UINT8                           *gLanMmio[4];     // their MMIO regions
extern UINT8                           gLanMac[4][6];    // their MAC addresses
extern UINTN                           nLanPaths;        // number of LAN pathes

BOOLEAN SetOEMPathIfExists(IN EFI_FILE *Root, IN CHAR16 *path, CONST CHAR16 *ConfName)
{
	BOOLEAN res = FileExists(Root, path);
	if ( res ) {
	  CHAR16 ConfigPath[1024];
		snwprintf(ConfigPath, sizeof(ConfigPath), "%ls\\%ls.plist", path, ConfName);
	  BOOLEAN res2 = FileExists(Root, ConfigPath);
	  if ( res2 ) {
	  	OEMPath = path;
	  	DBG("CheckOEMPathExists: set OEMPath: %ls\n", OEMPath);
	  	return 1;
	  }else{
	  	DBG("CheckOEMPathExists tried %ls. '%ls.plist' not exists in dir\n", path, ConfName);
	  	FreePool(path);
	  }
	}else{
		DBG("CheckOEMPathExists tried %ls. Dir not exists\n", path);
		FreePool(path);
	}
	return 0;
}

VOID SetOEMPath(CONST CHAR16 *ConfName)
  {
    OEMPath = PoolPrint(L"%s", L"EFI\\CLOVER");
    if (ConfName == NULL) {
      DBG("set OEMPath (ConfName == NULL): %ls\n", OEMPath);
    } else if ( nLanCards > 0   &&  SetOEMPathIfExists(SelfRootDir, PoolPrint(L"EFI\\CLOVER\\OEM\\%a--%02x-%02x-%02x-%02x-%02x-%02x", gSettings.OEMProduct, gLanMac[0][0], gLanMac[0][1], gLanMac[0][2], gLanMac[0][3], gLanMac[0][4], gLanMac[0][5]), ConfName)) {
    } else if ( nLanCards > 1   &&  SetOEMPathIfExists(SelfRootDir, PoolPrint(L"EFI\\CLOVER\\OEM\\%a--%02x-%02x-%02x-%02x-%02x-%02x", gSettings.OEMProduct, gLanMac[1][0], gLanMac[1][1], gLanMac[1][2], gLanMac[1][3], gLanMac[1][4], gLanMac[1][5]), ConfName)) {
    } else if ( nLanCards > 2   &&  SetOEMPathIfExists(SelfRootDir, PoolPrint(L"EFI\\CLOVER\\OEM\\%a--%02x-%02x-%02x-%02x-%02x-%02x", gSettings.OEMProduct, gLanMac[2][0], gLanMac[2][1], gLanMac[2][2], gLanMac[2][3], gLanMac[2][4], gLanMac[2][5]), ConfName)) {
    } else if ( nLanCards > 3   &&  SetOEMPathIfExists(SelfRootDir, PoolPrint(L"EFI\\CLOVER\\OEM\\%a--%02x-%02x-%02x-%02x-%02x-%02x", gSettings.OEMProduct, gLanMac[3][0], gLanMac[3][1], gLanMac[3][2], gLanMac[3][3], gLanMac[3][4], gLanMac[3][5]), ConfName)) {
    } else if (!gFirmwareClover && SetOEMPathIfExists(SelfRootDir, PoolPrint(L"EFI\\CLOVER\\OEM\\%a\\UEFI", gSettings.OEMBoard), ConfName)) {
    } else if (SetOEMPathIfExists(SelfRootDir, PoolPrint(L"EFI\\CLOVER\\OEM\\%a", gSettings.OEMProduct), ConfName)) {
    } else if (SetOEMPathIfExists(SelfRootDir, PoolPrint(L"EFI\\CLOVER\\OEM\\%a-%d", gSettings.OEMProduct, (INT32)(DivU64x32(gCPUStructure.CPUFrequency, Mega))), ConfName)) {
    } else if (SetOEMPathIfExists(SelfRootDir, PoolPrint(L"EFI\\CLOVER\\OEM\\%a", gSettings.OEMBoard), ConfName)) {
    } else if (SetOEMPathIfExists(SelfRootDir, PoolPrint(L"EFI\\CLOVER\\OEM\\%a-%d", gSettings.OEMBoard, (INT32)(DivU64x32(gCPUStructure.CPUFrequency, Mega))), ConfName)  ) {
    } else {
      DBG("set OEMPath by default: %ls\n", OEMPath);
    }
  }

//System / Install / Recovery version filler
CONST CHAR16 *SystemVersionPlist       = L"\\System\\Library\\CoreServices\\SystemVersion.plist";
CONST CHAR16 *ServerVersionPlist       = L"\\System\\Library\\CoreServices\\ServerVersion.plist";
CONST CHAR16 *InstallVersionPlist      = L"\\macOS Install Data\\Locked Files\\Boot Files\\SystemVersion.plist";
CONST CHAR16 *RecoveryVersionPlist     = L"\\com.apple.recovery.boot\\SystemVersion.plist";
CONST CHAR16  APFSSysPlistPath[86]     = L"\\00000000-0000-0000-0000-000000000000\\System\\Library\\CoreServices\\SystemVersion.plist";
CONST CHAR16  APFSServerPlistPath[86]  = L"\\00000000-0000-0000-0000-000000000000\\System\\Library\\CoreServices\\ServerVersion.plist";
CONST CHAR16  APFSInstallPlistPath[79] = L"\\00000000-0000-0000-0000-000000000000\\com.apple.installer\\SystemVersion.plist";
CONST CHAR16  APFSRecPlistPath[58]     = L"\\00000000-0000-0000-0000-000000000000\\SystemVersion.plist";
  

VOID SystemVersionInit(VOID)
{
  //Plists iterators
  UINTN      SysIter            = 2;
  UINTN      InsIter            = 1;
  UINTN      RecIter            = 1;
  UINTN      k                  = 0;
  UINTN      i;

  // If scanloader starts multiple times, then we need to free systemplists, installplists, recoveryplists variables, also
  // refresh APFSUUIDBank
  if ((SystemPlists != NULL) || (InstallPlists != NULL) || (RecoveryPlists != NULL)) {
    if ((APFSUUIDBank != NULL) && (APFSSupport == TRUE)) {
      FreePool(APFSUUIDBank);
      //Reset APFSUUIDBank counter, we will re-enumerate it
      APFSUUIDBankCounter = 0;
      APFSUUIDBank = APFSContainer_Support();
      if (APFSUUIDBankCounter == 0) {
        APFSSupport = FALSE;
      }
    }
    if (SystemPlists != NULL) {
      k = 0;
      while (SystemPlists[k] != NULL) {
        SystemPlists[k] = NULL;
        k++;
      }
      FreePool(SystemPlists);
      SystemPlists = NULL;
    }
    if (InstallPlists != NULL) {
      k = 0;
      while (InstallPlists[k] != NULL) {
        InstallPlists[k] = NULL;
        k++;
      }
      FreePool(InstallPlists);
      InstallPlists = NULL;
    }
    if (RecoveryPlists != NULL) {
      k = 0;
      while (RecoveryPlists[k] != NULL) {
        RecoveryPlists[k] = NULL;
        k++;
      }
      FreePool(RecoveryPlists);
      RecoveryPlists = NULL;
    }
  }
  /************************************************************************/
  /*Allocate Memory for systemplists, installplists and recoveryplists********************/
  //Check apfs support
  if (APFSSupport == TRUE) {
    SystemPlists = (__typeof__(SystemPlists))AllocateZeroPool((2*APFSUUIDBankCounter+3)*sizeof(CHAR16 *));//array of pointers
    InstallPlists = (__typeof__(InstallPlists))AllocateZeroPool((APFSUUIDBankCounter+2)*sizeof(CHAR16 *));//array of pointers
    RecoveryPlists = (__typeof__(RecoveryPlists))AllocateZeroPool((APFSUUIDBankCounter+2)*sizeof(CHAR16 *));//array of pointers
  } else {
    SystemPlists = (__typeof__(SystemPlists))AllocateZeroPool(sizeof(CHAR16 *)*3);
    InstallPlists = (__typeof__(InstallPlists))AllocateZeroPool(sizeof(CHAR16 *)*2);
    RecoveryPlists = (__typeof__(RecoveryPlists))AllocateZeroPool(sizeof(CHAR16 *)*2);
  }
  /* Fill it with standard paths*******************************************/
  SystemPlists[0] = SystemVersionPlist;
  SystemPlists[1] = ServerVersionPlist;
  SystemPlists[2] = NULL;
  InstallPlists[0] = InstallVersionPlist;
  InstallPlists[1] = NULL;
  RecoveryPlists[0] = RecoveryVersionPlist;
  RecoveryPlists[1] = NULL;
  /************************************************************************/
  //Fill Plists 
  for (i = 0; i < APFSUUIDBankCounter; i++) {
    //Store UUID from bank
    CHAR16 *CurrentUUID = GuidLEToStr((EFI_GUID *)((UINT8 *)APFSUUIDBank+i*0x10));
    //Init temp string with system/install/recovery APFS path
    CHAR16 *TmpSysPlistPath = (__typeof__(TmpSysPlistPath))AllocateZeroPool(86*sizeof(CHAR16));
    CHAR16 *TmpServerPlistPath = (__typeof__(TmpServerPlistPath))AllocateZeroPool(86*sizeof(CHAR16));
    CHAR16 *TmpInsPlistPath = (__typeof__(TmpInsPlistPath))AllocateZeroPool(79*sizeof(CHAR16));
    CHAR16 *TmpRecPlistPath = (__typeof__(TmpRecPlistPath))AllocateZeroPool(58*sizeof(CHAR16));
    StrnCpy(TmpSysPlistPath, APFSSysPlistPath, 85);
    StrnCpy(TmpServerPlistPath, APFSServerPlistPath, 85);
    StrnCpy(TmpInsPlistPath, APFSInstallPlistPath, 78);
    StrnCpy(TmpRecPlistPath, APFSRecPlistPath, 57);
    StrnCpy(TmpSysPlistPath+1, CurrentUUID, 36);
    StrnCpy(TmpServerPlistPath+1, CurrentUUID, 36);
    StrnCpy(TmpInsPlistPath+1, CurrentUUID, 36);
    StrnCpy(TmpRecPlistPath+1, CurrentUUID, 36);
    //Fill SystemPlists/InstallPlists/RecoveryPlists arrays
    SystemPlists[SysIter] = TmpSysPlistPath;
    SystemPlists[SysIter+1] = TmpServerPlistPath;
    SystemPlists[SysIter+2] = NULL;
    InstallPlists[InsIter] = TmpInsPlistPath;
    InstallPlists[InsIter+1] = NULL;
    RecoveryPlists[RecIter] = TmpRecPlistPath;
    RecoveryPlists[RecIter+1] = NULL;
    SysIter+=2;
    InsIter++;
    RecIter++;
  }
}

#ifdef DEBUG_CLOVER
XStringW g_str(L"g_str:foobar");
XStringW g_str2(L"g_str:foobar2");
//XStringW g_str3(L"g_str:foobar2");
//XStringW g_str4(L"g_str:foobar2");
//XStringW g_str5(L"g_str:foobar2");
//XStringW g_str6(L"g_str:foobar2");
//XStringW g_str7(L"g_str:foobar2");
//XStringW g_str8(L"g_str:foobar2");
//XStringW g_str9(L"g_str:foobar2");
//XStringW g_str10(L"g_str:foobar2");
//XStringW g_str11(L"g_str:foobar2");
//XStringW g_str12(L"g_str:foobar2");
#endif


//
// main entry point
//
EFI_STATUS
EFIAPI
RefitMain (IN EFI_HANDLE           ImageHandle,
           IN EFI_SYSTEM_TABLE     *SystemTable)
{
  EFI_STATUS Status;
  BOOLEAN           MainLoopRunning = TRUE;
  BOOLEAN           ReinitDesktop = TRUE;
  BOOLEAN           AfterTool = FALSE;
  REFIT_ABSTRACT_MENU_ENTRY  *ChosenEntry = NULL;
  REFIT_ABSTRACT_MENU_ENTRY  *DefaultEntry = NULL;
  REFIT_ABSTRACT_MENU_ENTRY  *OptionEntry = NULL;
  INTN              DefaultIndex;
  UINTN             MenuExit;
  UINTN             Size, i;
	//UINT64            TscDiv;
	//UINT64            TscRemainder = 0;
//  LOADER_ENTRY      *LoaderEntry;
  CHAR16            *ConfName = NULL;
  TagPtr            smbiosTags = NULL;
  TagPtr            UniteTag = NULL;
  BOOLEAN           UniteConfigs = FALSE;
  EFI_TIME          Now;
  BOOLEAN           HaveDefaultVolume;
  REFIT_MENU_SCREEN BootScreen;
  BootScreen.isBootScreen = true; //other screens will be constructed as false
  // CHAR16            *InputBuffer; //, *Y;
  //  EFI_INPUT_KEY Key;

  // Init assets dir: misc
  /*Status = */ //egMkDir(SelfRootDir,  L"EFI\\CLOVER\\misc");
  //Should apply to: "ACPI/origin/" too

  // get TSC freq and init MemLog if needed
  gCPUStructure.TSCCalibr = GetMemLogTscTicksPerSecond(); //ticks for 1second
  //GlobalConfig.TextOnly = TRUE;

  // bootstrap
  gST       = SystemTable;
  gImageHandle  = ImageHandle;
  gBS       = SystemTable->BootServices;
  gRT       = SystemTable->RuntimeServices;
  /*Status = */EfiGetSystemConfigurationTable (&gEfiDxeServicesTableGuid, (VOID **) &gDS);
  
  ConsoleInHandle = SystemTable->ConsoleInHandle;
  

  gRT->GetTime(&Now, NULL);

  // firmware detection
  gFirmwareClover = StrCmp(gST->FirmwareVendor, L"CLOVER") == 0;
  if (!gFirmwareRevision) {
//    gFirmwareRevision = PoolPrint(L"%d", gST->FirmwareRevision);
  }
  InitializeConsoleSim();
  InitBooterLog();
  ZeroMem((VOID*)&gGraphics[0], sizeof(GFX_PROPERTIES) * 4);
  ZeroMem((VOID*)&gAudios[0], sizeof(HDA_PROPERTIES) * 4);

  DBG("\n");
  if (Now.TimeZone < -1440 || Now.TimeZone > 1440) {
    MsgLog("Now is %02d.%02d.%d,  %02d:%02d:%02d (GMT)\n",
           Now.Day, Now.Month, Now.Year, Now.Hour, Now.Minute, Now.Second);
  } else {
    MsgLog("Now is %02d.%02d.%d,  %02d:%02d:%02d (GMT+%d)\n",
      Now.Day, Now.Month, Now.Year, Now.Hour, Now.Minute, Now.Second, GlobalConfig.Timezone);
  }
  //MsgLog("Starting Clover rev %ls on %ls EFI\n", gFirmwareRevision, gST->FirmwareVendor);
	MsgLog("Starting %s on %ls EFI\n", gRevisionStr, gST->FirmwareVendor);

	if ( gBuildInfo ) DBG("Build with: [%s]\n", gBuildInfo);


  Status = InitRefitLib(gImageHandle);
  if (EFI_ERROR(Status))
    return Status;

	DBG("Clover : Image base = 0x%llX\n", (uintptr_t)SelfLoadedImage->ImageBase); // do not change, it's used by grep to feed the debugger
#ifdef JIEF_DEBUG
  gBS->Stall(1500000); // to give time to gdb to connect
//  PauseForKey(L"press\n");
#endif

  construct_globals_objects(); // do this after SelfLoadedImage is initialized
#ifdef JIEF_DEBUG
  all_tests();
#endif

  //dumping SETTING structure
  // if you change something in Platform.h, please uncomment and test that all offsets
  // are natural aligned i.e. pointers are 8 bytes aligned
  /*
  DBG("Settings offsets:\n");
  DBG(" OEMProduct:     %X\n",    OFFSET_OF(SETTINGS_DATA, OEMProduct));
  DBG(" DefaultVolume:  %X\n",    OFFSET_OF(SETTINGS_DATA, DefaultVolume));
  DBG(" DefaultLoader:  %X\n",    OFFSET_OF(SETTINGS_DATA, DefaultLoader));
  DBG(" ResetAddr:      %X\n",    OFFSET_OF(SETTINGS_DATA, ResetAddr));
  DBG(" FixDsdt:        %X\n",    OFFSET_OF(SETTINGS_DATA, FixDsdt));
  DBG(" FakeATI:        %X\n",    OFFSET_OF(SETTINGS_DATA, FakeATI));
  DBG(" PatchVBiosBytes:%X\n",    OFFSET_OF(SETTINGS_DATA, PatchVBiosBytes));
  DBG(" VRAM:           %X\n",    OFFSET_OF(SETTINGS_DATA, VRAM));
  DBG(" SecureBootWhiteListCount: %X\n",    OFFSET_OF(SETTINGS_DATA, SecureBootWhiteListCount));
  DBG(" LegacyBoot:     %X\n",    OFFSET_OF(SETTINGS_DATA, LegacyBoot));
  DBG(" HVHideStrings:  %X\n",    OFFSET_OF(SETTINGS_DATA, HVHideStrings));
  DBG(" PointerSpeed:   %X\n",    OFFSET_OF(SETTINGS_DATA, PointerSpeed));
  DBG(" RtMLB:          %X\n",    OFFSET_OF(SETTINGS_DATA, RtMLB));
  DBG(" ConfigName:     %X\n",    OFFSET_OF(SETTINGS_DATA, ConfigName));
  DBG(" PointerSpeed:   %X\n",    OFFSET_OF(SETTINGS_DATA, PointerSpeed));
  DBG(" PatchDsdtNum:   %X\n",    OFFSET_OF(SETTINGS_DATA, PatchDsdtNum));
  DBG(" LenToReplace:   %X\n",    OFFSET_OF(SETTINGS_DATA, LenToReplace));
  DBG(" ACPIDropTables: %X\n",    OFFSET_OF(SETTINGS_DATA, ACPIDropTables));
  DBG(" CustomEntries:  %X\n",    OFFSET_OF(SETTINGS_DATA, CustomEntries));
  DBG(" CustomTool:     %X\n",    OFFSET_OF(SETTINGS_DATA, CustomTool));
  DBG(" AddProperties:  %X\n",    OFFSET_OF(SETTINGS_DATA, AddProperties));
  DBG(" BlockKexts:     %X\n",    OFFSET_OF(SETTINGS_DATA, BlockKexts));
   */

  // disable EFI watchdog timer
  gBS->SetWatchdogTimer(0x0000, 0x0000, 0x0000, NULL);
  ZeroMem((VOID*)&gSettings, sizeof(SETTINGS_DATA));

  Status = InitializeUnicodeCollationProtocol();
  if (EFI_ERROR(Status)) {
    DBG("UnicodeCollation Status=%s\n", strerror(Status));
  }
  
  Status = gBS->HandleProtocol(ConsoleInHandle, &gEfiSimpleTextInputExProtocolGuid, (VOID **)&SimpleTextEx);
  if ( EFI_ERROR(Status) ) {
    SimpleTextEx = NULL;
  }
  DBG("SimpleTextEx Status=%s\n", strerror(Status));

  PrepatchSmbios();

  //replace / with _
  Size = iStrLen(gSettings.OEMProduct, 64);
  for (i=0; i<Size; i++) {
    if (gSettings.OEMProduct[i] == 0x2F) {
      gSettings.OEMProduct[i] = 0x5F;
    }
  }
  Size = iStrLen(gSettings.OEMBoard, 64);
  for (i=0; i<Size; i++) {
    if (gSettings.OEMBoard[i] == 0x2F) {
      gSettings.OEMBoard[i] = 0x5F;
    }
  }
  DBG("Running on: '%s' with board '%s'\n", gSettings.OEMProduct, gSettings.OEMBoard);

  GetCPUProperties();
  GetDevices();
  GetDefaultSettings();

  // LoadOptions Parsing
  DBG("Clover load options size = %d bytes\n", SelfLoadedImage->LoadOptionsSize);
  if ((SelfLoadedImage->LoadOptions != NULL) &&
      (SelfLoadedImage->LoadOptionsSize != 0)){
    if (*(UINT32*)SelfLoadedImage->LoadOptions == CLOVER_SIGN) {
      GetBootFromOption();
    } else {
      ParseLoadOptions(&ConfName, &gConfigDict[1]);
      if (ConfName) {
        if (StrLen(ConfName) == 0) {
          gConfigDict[1] = NULL;
          FreePool(ConfName);
          ConfName = NULL;
        } else {
          SetOEMPath(ConfName);
          Status = LoadUserSettings(SelfRootDir, ConfName, &gConfigDict[1]);
          DBG("%ls\\%ls.plist%ls loaded with name from LoadOptions: %s\n",
              OEMPath, ConfName, EFI_ERROR(Status) ? L" not" : L"", strerror(Status));
          if (EFI_ERROR(Status)) {
            gConfigDict[1] = NULL;
            FreePool(ConfName);
            ConfName = NULL;
          }
        }
      }
    }
  }
  if (gConfigDict[1]) {
    UniteTag = GetProperty(gConfigDict[1], "Unite");
    if(UniteTag) {
      UniteConfigs =  (UniteTag->type == kTagTypeTrue) ||
                      ((UniteTag->type == kTagTypeString) &&
                      ((UniteTag->string[0] == 'y') || (UniteTag->string[0] == 'Y')));
      DBG("UniteConfigs = %ls", UniteConfigs ? L"TRUE\n": L"FALSE\n" );
    }
  }
  if (!gConfigDict[1] || UniteConfigs) {
    SetOEMPath(L"config");
    Status = LoadUserSettings(SelfRootDir, L"config", &gConfigDict[0]);
      DBG("%ls\\config.plist%ls loaded: %s\n", OEMPath, EFI_ERROR(Status) ? L" not" : L"", strerror(Status));
  }
	snwprintf(gSettings.ConfigName, 64, "%ls%ls%ls",
/*  gSettings.ConfigName = PoolPrint(L"%s%s%s", */
                                   gConfigDict[0] ? L"config": L"",
                                   (gConfigDict[0] && gConfigDict[1]) ? L" + ": L"",
                                   !gConfigDict[1] ? L"": (ConfName ? ConfName : L"Load Options"));
  gSettings.MainConfigName = EfiStrDuplicate(gSettings.ConfigName);

  gSettings.PointerEnabled = TRUE;
  gSettings.PointerSpeed = 2;
  gSettings.DoubleClickTime = 500; //TODO - make it constant as nobody change it

#ifdef ENABLE_SECURE_BOOT
  InitializeSecureBoot();
#endif // ENABLE_SECURE_BOOT


  {
//    UINT32                    machineSignature    = 0;
    EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE     *FadtPointer = NULL;
    EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE  *Facs = NULL;

//    DBG("---dump hibernations data---\n");
    FadtPointer = GetFadt();
    if (FadtPointer != NULL) {
      Facs = (EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE*)(UINTN)(FadtPointer->FirmwareCtrl);
      /*
      DBG("  Firmware wake address=%08lx\n", Facs->FirmwareWakingVector);
      DBG("  Firmware wake 64 addr=%16llx\n",  Facs->XFirmwareWakingVector);
      DBG("  Hardware signature   =%08lx\n", Facs->HardwareSignature);
      DBG("  GlobalLock           =%08lx\n", Facs->GlobalLock);
      DBG("  Flags                =%08lx\n", Facs->Flags);
       */
      machineSignature = Facs->HardwareSignature;
    }
#if HIBERNATE_DUMP_DATA
//------------------------------------------------------
    DumpVariable(L"Boot0082", &gEfiGlobalVariableGuid, 8);
    DumpVariable(L"boot-switch-vars", &gEfiAppleBootGuid, -1);
    DumpVariable(L"boot-signature",   &gEfiAppleBootGuid, -1);
    DumpVariable(L"boot-image-key",   &gEfiAppleBootGuid, -1);
    DumpVariable(L"boot-image",       &gEfiAppleBootGuid, 0);
//-----------------------------------------------------------
 
#endif //
  }

#if 0
  //testing place
  {
    const CHAR16 *aaa = L"12345  ";
    const CHAR8 *bbb = "12345  ";
    DBG(" string %ls, size=%lld, len=%lld sizeof=%ld iStrLen=%lld\n", aaa, StrSize(aaa), StrLen(aaa), sizeof(aaa), iStrLen(bbb, 10));

    CHAR8           compatible[64];
    UINT32 FakeLAN = 0x0030168c;
    UINT32 FakeID = FakeLAN >> 16;
    UINT32 FakeVendor = FakeLAN & 0xFFFF;
    snprintf(compatible, 64, "pci%x,%x", FakeVendor, FakeID);
    DBG(" FakeLAN = 0x%x\n", FakeLAN);
    DBG(" Compatible=%s strlen=%ld sizeof=%ld iStrLen=%lld\n", compatible,
        strlen(compatible), sizeof(compatible), iStrLen(compatible, 64));
//    LowCase(compatible);
//    DBG(" Low Compatible=%s strlen=%ld sizeof=%ld iStrLen=%lld\n", compatible,
//        strlen(compatible), sizeof(compatible), iStrLen(compatible, 64));

    DBG("void*=%ld int=%ld long=%ld longlong=%ld enum=%ld\n",
        sizeof(void*), sizeof(int), sizeof(long int), sizeof(long long), sizeof(EFI_ALLOCATE_TYPE));
  }
#endif
  if (!GlobalConfig.FastBoot) {
    GetListOfThemes();
    GetListOfConfigs();
  }

//  ThemeX.FillByEmbedded(); //init XTheme before EarlyUserSettings

  for (i=0; i<2; i++) {
    if (gConfigDict[i]) {
      GetEarlyUserSettings(SelfRootDir, gConfigDict[i]);
    }
  }

#ifdef ENABLE_SECURE_BOOT
  // Install secure boot shim
  if (EFI_ERROR(Status = InstallSecureBoot())) {
    PauseForKey(L"Secure boot failure!\n");
    return Status;
  }
#endif // ENABLE_SECURE_BOOT

  MainMenu.TimeoutSeconds = GlobalConfig.Timeout >= 0 ? GlobalConfig.Timeout : 0;
  //DBG("LoadDrivers() start\n");
  LoadDrivers();
  //DBG("LoadDrivers() end\n");

/*  if (!gFirmwareClover &&
      !gDriversFlags.EmuVariableLoaded) {
    GetSmcKeys(FALSE);  // later we can get here SMC information
  } */
  
  Status = gBS->LocateProtocol (&gEmuVariableControlProtocolGuid, NULL, (VOID**)&gEmuVariableControl);
  if (EFI_ERROR(Status)) {
    gEmuVariableControl = NULL;
  }
  if (gEmuVariableControl != NULL) {
    gEmuVariableControl->InstallEmulation(gEmuVariableControl);
  }

  DbgHeader("InitScreen");
	
  if (!GlobalConfig.FastBoot) {
    // init screen and dump video modes to log
    if (gDriversFlags.VideoLoaded) {
      InitScreen(FALSE);
    } else {
      InitScreen(!gFirmwareClover); // ? FALSE : TRUE);
    }
    //DBG("DBG: setup screen\n");
    SetupScreen();
  } else {
    InitScreen(FALSE);
  }
	
  //  DBG("DBG: ReinitSelfLib\n");
  //Now we have to reinit handles
  Status = ReinitSelfLib();
  if (EFI_ERROR(Status)){
    DebugLog(2, " %s", strerror(Status));
    PauseForKey(L"Error reinit refit\n");
#ifdef ENABLE_SECURE_BOOT
    UninstallSecureBoot();
#endif // ENABLE_SECURE_BOOT
    return Status;
  }
	
  //  DBG("DBG: messages\n");
  if (!GlobalConfig.NoEarlyProgress && !GlobalConfig.FastBoot  && GlobalConfig.Timeout>0) {
    XStringW Message = SWPrintf("   Welcome to Clover %ls   ", gFirmwareRevision);
    BootScreen.DrawTextXY(Message, (UGAWidth >> 1), UGAHeight >> 1, X_IS_CENTER);
    BootScreen.DrawTextXY(L"... testing hardware ..."_XSW, (UGAWidth >> 1), (UGAHeight >> 1) + 20, X_IS_CENTER);
  }

//  DumpBiosMemoryMap();

  GuiEventsInitialize();

  if (!gSettings.EnabledCores) {
    gSettings.EnabledCores = gCPUStructure.Cores;
  }

  GetMacAddress();
  //DBG("ScanSPD() start\n");
  ScanSPD();
  //DBG("ScanSPD() end\n");

  SetPrivateVarProto();
//  GetDefaultSettings();
  GetAcpiTablesList();

	DBG("Calibrated TSC Frequency = %llu = %lluMHz\n", gCPUStructure.TSCCalibr, DivU64x32(gCPUStructure.TSCCalibr, Mega));
  if (gCPUStructure.TSCCalibr > 200000000ULL) {  //200MHz
    gCPUStructure.TSCFrequency = gCPUStructure.TSCCalibr;
  }

  gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
  gCPUStructure.FSBFrequency = DivU64x32(MultU64x32(gCPUStructure.CPUFrequency, 10),
                                         (gCPUStructure.MaxRatio == 0) ? 1 : gCPUStructure.MaxRatio);
  gCPUStructure.MaxSpeed = (UINT32)DivU64x32(gCPUStructure.TSCFrequency + (Mega >> 1), Mega);

  switch (gCPUStructure.Model) {
    case CPU_MODEL_PENTIUM_M:
    case CPU_MODEL_ATOM://  Atom
    case CPU_MODEL_DOTHAN:// Pentium M, Dothan, 90nm
    case CPU_MODEL_YONAH:// Core Duo/Solo, Pentium M DC
    case CPU_MODEL_MEROM:// Core Xeon, Core 2 Duo, 65nm, Mobile
    //case CPU_MODEL_CONROE:// Core Xeon, Core 2 Duo, 65nm, Desktop like Merom but not mobile
    case CPU_MODEL_CELERON:
    case CPU_MODEL_PENRYN:// Core 2 Duo/Extreme, Xeon, 45nm , Mobile
    case CPU_MODEL_NEHALEM:// Core i7 LGA1366, Xeon 5500, "Bloomfield", "Gainstown", 45nm
    case CPU_MODEL_FIELDS:// Core i7, i5 LGA1156, "Clarksfield", "Lynnfield", "Jasper", 45nm
    case CPU_MODEL_DALES:// Core i7, i5, Nehalem
    case CPU_MODEL_CLARKDALE:// Core i7, i5, i3 LGA1156, "Westmere", "Clarkdale", , 32nm
    case CPU_MODEL_WESTMERE:// Core i7 LGA1366, Six-core, "Westmere", "Gulftown", 32nm
    case CPU_MODEL_NEHALEM_EX:// Core i7, Nehalem-Ex Xeon, "Beckton"
    case CPU_MODEL_WESTMERE_EX:// Core i7, Nehalem-Ex Xeon, "Eagleton"
      gCPUStructure.ExternalClock = (UINT32)DivU64x32(gCPUStructure.FSBFrequency + kilo - 1, kilo);
      //DBG(" Read TSC ExternalClock: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.ExternalClock, kilo)));
      break;
    default:
      //DBG(" Read TSC ExternalClock: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.FSBFrequency, Mega)));
	  
      // for sandy bridge or newer
      // to match ExternalClock 25 MHz like real mac, divide FSBFrequency by 4
      gCPUStructure.ExternalClock = ((UINT32)DivU64x32(gCPUStructure.FSBFrequency + kilo - 1, kilo) + 3) / 4;
      //DBG(" Corrected TSC ExternalClock: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.ExternalClock, kilo)));
      break;
  }

  if (!GlobalConfig.NoEarlyProgress && !GlobalConfig.FastBoot && GlobalConfig.Timeout>0) {
    XStringW Message = SWPrintf("... user settings ...");
    BootScreen.EraseTextXY();
    BootScreen.DrawTextXY(Message, (UGAWidth >> 1), (UGAHeight >> 1) + 20, X_IS_CENTER);
  }

  //Second step. Load config.plist into gSettings
  for (i=0; i<2; i++) {
    if (gConfigDict[i]) {
      Status = GetUserSettings(SelfRootDir, gConfigDict[i]);
      if (EFI_ERROR(Status)) {
 //       DBG("Error in Second part of settings%d: %s\n", i, strerror(Status));
      }
    }
  }
  

  if (gSettings.QEMU) {
//    UINT64 Msrflex = 0ULL;

    if (!gSettings.UserChange) {
      gSettings.BusSpeed = 200000;
    }
    gCPUStructure.MaxRatio = (UINT32)DivU64x32(gCPUStructure.TSCCalibr, gSettings.BusSpeed * kilo);
    DBG("Set MaxRatio for QEMU: %d\n", gCPUStructure.MaxRatio);
    gCPUStructure.MaxRatio *= 10;
    gCPUStructure.MinRatio = 60;
/*    AsmWriteMsr64(MSR_FLEX_RATIO, ((6ULL << 40) + //(1ULL << 16) +
                                   (gCPUStructure.MaxRatio << 8)));
    DBG("check if flex is RW\n");
    Msrflex = AsmReadMsr64(MSR_FLEX_RATIO); //0 == not Rw :(
    DBG("MSR_FLEX_RATIO = %lx\n", Msrflex);
 */
    gCPUStructure.FSBFrequency = DivU64x32(MultU64x32(gCPUStructure.CPUFrequency, 10),
                                           (gCPUStructure.MaxRatio == 0) ? 1 : gCPUStructure.MaxRatio);
    gCPUStructure.ExternalClock = (UINT32)DivU64x32(gCPUStructure.FSBFrequency + kilo - 1, kilo);
  }

  dropDSM = 0xFFFF; //by default we drop all OEM _DSM. They have no sense for us.
  if (defDSM) {
    dropDSM = gSettings.DropOEM_DSM;   //if set by user
  }
  // Load any extra SMBIOS information
  if (!EFI_ERROR(LoadUserSettings(SelfRootDir, L"smbios", &smbiosTags)) && (smbiosTags != NULL)) {
    TagPtr dictPointer = GetProperty(smbiosTags,"SMBIOS");
    if (dictPointer) {
      ParseSMBIOSSettings(dictPointer);
    } else {
      DBG("Invalid smbios.plist, not overriding config.plist!\n");
    }
  }
/*
  if (gFirmwareClover || gDriversFlags.EmuVariableLoaded) {
    if (GlobalConfig.StrictHibernate) {
      DBG(" Don't use StrictHibernate with emulated NVRAM!\n");
    }
    GlobalConfig.StrictHibernate = FALSE;    
  }
*/
  HaveDefaultVolume = gSettings.DefaultVolume != NULL;
  if (!gFirmwareClover &&
      !gDriversFlags.EmuVariableLoaded &&
      !HaveDefaultVolume &&
      GlobalConfig.Timeout == 0 && !ReadAllKeyStrokes()) {
// UEFI boot: get gEfiBootDeviceGuid from NVRAM.
// if present, ScanVolumes() will skip scanning other volumes
// in the first run.
// this speeds up loading of default macOS  volume.
     GetEfiBootDeviceFromNvram();
  }

  if (!GlobalConfig.NoEarlyProgress && !GlobalConfig.FastBoot && GlobalConfig.Timeout>0) {
    XStringW Message = SWPrintf("...  scan entries  ...");
    BootScreen.EraseTextXY();
    BootScreen.DrawTextXY(Message, (UGAWidth >> 1), (UGAHeight >> 1) + 20, X_IS_CENTER);
  }


  GetListOfDsdts(); //only after GetUserSettings
  GetListOfACPI(); //ssdt and other tables

  AfterTool = FALSE;
  gGuiIsReady = TRUE;
  do {
    MainMenu.Entries.Empty();
    OptionMenu.Entries.Empty();
    InitKextList();
    ScanVolumes();

    //Check apfs driver loaded state
    //Free APFSUUIDBank
    if (APFSUUIDBank != NULL) {
     //Free mem
     FreePool(APFSUUIDBank);
     //Reset counter
     APFSUUIDBankCounter=0;
    }
    /* APFS container support */
    //Fill APFSUUIDBank
    APFSUUIDBank = APFSContainer_Support();
    if (APFSUUIDBankCounter != 0) {
      APFSSupport = TRUE;
    } else {
      APFSSupport = FALSE;
    }
    //Fill systemversion plists path
    SystemVersionInit();

    // as soon as we have Volumes, find latest nvram.plist and copy it to RT vars
    if (!AfterTool) {
      if (gFirmwareClover || gDriversFlags.EmuVariableLoaded) {
        PutNvramPlistToRtVars();
      }
    }
    
    // log Audio devices in boot-log. Thisis for clients like Clover.app
    GetOutputs();
    for (i = 0; i < AudioNum; i++) {
      if (AudioList[i].Name) {
        // Never change this log, otherwise clients will stop interprete the output.
		  MsgLog("Found Audio Device %ls (%s) at index %llu\n", AudioList[i].Name, AudioOutputNames[AudioList[i].Device], i);
      }
    }
    
    if (!GlobalConfig.FastBoot) {
//      CHAR16 *TmpArgs;
      if (gThemeNeedInit) {
        InitTheme(TRUE, &Now);
        gThemeNeedInit = FALSE;
        gThemeChanged = TRUE;
      } else if (gThemeChanged) {
        DBG("change theme\n");
        InitTheme(FALSE, NULL);
        OptionMenu.FreeMenu();
      }
      DBG("theme inited\n");
      if (ThemeX.embedded) {
        DBG("Chosen embedded theme\n");
      } else {
        DBG("Chosen theme %ls\n", ThemeX.Theme.data());
      }

//      DBG("initial boot-args=%s\n", gSettings.BootArgs);
      //now it is a time to set RtVariables
      SetVariablesFromNvram();
      
      XString TmpArgs = SPrintf("%s ", gSettings.BootArgs);
      DBG("after NVRAM boot-args=%s\n", gSettings.BootArgs);
      gSettings.OptionsBits = EncodeOptions(TmpArgs);
//      DBG("initial OptionsBits %X\n", gSettings.OptionsBits);
      FillInputs(TRUE);

      // scan for loaders and tools, add then to the menu
      if (GlobalConfig.LegacyFirst){
        AddCustomLegacy();
        if (!GlobalConfig.NoLegacy) {
          ScanLegacy();
        }
      }
    }
    GetSmcKeys(TRUE);
    
    // Add custom entries
    AddCustomEntries();
    if (gSettings.DisableEntryScan) {
      DBG("Entry scan disabled\n");
    } else {
      ScanLoader();
    }

    if (!GlobalConfig.FastBoot) {

      if (!GlobalConfig.LegacyFirst) {
        AddCustomLegacy();
        if (!GlobalConfig.NoLegacy) {
          ScanLegacy();
        }
      }

      // fixed other menu entries
      if (!(ThemeX.HideUIFlags & HIDEUI_FLAG_TOOLS)) {
        AddCustomTool();
        if (!gSettings.DisableToolScan) {
          ScanTool();
#ifdef ENABLE_SECURE_BOOT
          // Check for secure boot setup mode
          AddSecureBootTool();
#endif // ENABLE_SECURE_BOOT
        }
      }

      MenuEntryOptions.Image = ThemeX.GetIcon(BUILTIN_ICON_FUNC_OPTIONS);

      if (gSettings.DisableCloverHotkeys)
        MenuEntryOptions.ShortcutLetter = 0x00;
      MainMenu.AddMenuEntry(&MenuEntryOptions, false);
      MenuEntryAbout.Image = ThemeX.GetIcon((INTN)BUILTIN_ICON_FUNC_ABOUT);

      if (gSettings.DisableCloverHotkeys)
        MenuEntryAbout.ShortcutLetter = 0x00;
      MainMenu.AddMenuEntry(&MenuEntryAbout, false);

      if (!(ThemeX.HideUIFlags & HIDEUI_FLAG_FUNCS) || MainMenu.Entries.size() == 0) {
        if (gSettings.DisableCloverHotkeys)
          MenuEntryReset.ShortcutLetter = 0x00;
        MenuEntryReset.Image = ThemeX.GetIcon(BUILTIN_ICON_FUNC_RESET);
        MainMenu.AddMenuEntry(&MenuEntryReset, false);
        if (gSettings.DisableCloverHotkeys)
          MenuEntryShutdown.ShortcutLetter = 0x00;
        MenuEntryShutdown.Image = ThemeX.GetIcon(BUILTIN_ICON_FUNC_EXIT);
        MainMenu.AddMenuEntry(&MenuEntryShutdown, false);
      }

// font already changed and this message very quirky, clear line here
//     if (!GlobalConfig.NoEarlyProgress && !GlobalConfig.FastBoot && GlobalConfig.Timeout>0) {
//        XStringW Message = L"                          "_XSW;
//        BootScreen.EraseTextXY();
//        DrawTextXY(Message, (UGAWidth >> 1), (UGAHeight >> 1) + 20, X_IS_CENTER);
//      }
    }
    // wait for user ACK when there were errors
    FinishTextScreen(FALSE);
#if CHECK_SMC
    DumpSmcKeys();
#endif

    DefaultIndex = FindDefaultEntry();
//	  DBG("DefaultIndex=%lld and MainMenu.Entries.size()=%llu\n", DefaultIndex, MainMenu.Entries.size());
    if ((DefaultIndex >= 0) && (DefaultIndex < (INTN)MainMenu.Entries.size())) {
      DefaultEntry = &MainMenu.Entries[DefaultIndex];
    } else {
      DefaultEntry = NULL;
    }

    MainLoopRunning = TRUE;
    //    MainMenu.TimeoutSeconds = GlobalConfig.Timeout >= 0 ? GlobalConfig.Timeout : 0;
    if (DefaultEntry && (GlobalConfig.FastBoot ||
                         (gSettings.SkipHibernateTimeout &&
                           DefaultEntry->getLOADER_ENTRY()
                           && OSFLAG_ISSET(DefaultEntry->getLOADER_ENTRY()->Flags, OSFLAG_HIBERNATED)
                         )
                        )
        )
    {
      if (DefaultEntry->getLOADER_ENTRY()) {
        StartLoader(DefaultEntry->getLOADER_ENTRY());
      } else if (DefaultEntry->getLEGACY_ENTRY()){
        StartLegacy(DefaultEntry->getLEGACY_ENTRY());
      }
      GlobalConfig.FastBoot = FALSE; //Hmm... will never be here
    }
//    BOOLEAN MainAnime = MainMenu.GetAnime();
//    DBG("MainAnime=%d\n", MainAnime);
    AfterTool = FALSE;
    gEvent = 0; //clear to cancel loop
    while (MainLoopRunning) {
 //     CHAR8 *LastChosenOS = NULL;
      if (GlobalConfig.Timeout == 0 && DefaultEntry != NULL && !ReadAllKeyStrokes()) {
        // go strait to DefaultVolume loading
        MenuExit = MENU_EXIT_TIMEOUT;
      } else {
        MainMenu.GetAnime();
        if (gThemeChanged) {
          gThemeChanged = FALSE;
          ThemeX.ClearScreen();
        }
        MenuExit = MainMenu.RunMainMenu(DefaultIndex, &ChosenEntry);
      }
//		DBG("exit from MainMenu %llu\n", MenuExit); //MENU_EXIT_ENTER=(1) MENU_EXIT_DETAILS=3
      // disable default boot - have sense only in the first run
      GlobalConfig.Timeout = -1;
      if ((DefaultEntry != NULL) && (MenuExit == MENU_EXIT_TIMEOUT)) {
        if (DefaultEntry->getLOADER_ENTRY()) {
          StartLoader(DefaultEntry->getLOADER_ENTRY());
        } else if (DefaultEntry->getLEGACY_ENTRY()){
          StartLegacy(DefaultEntry->getLEGACY_ENTRY());
        }
        // if something goes wrong - break main loop to reinit volumes
        break;
      }

      if (MenuExit == MENU_EXIT_OPTIONS){
        gBootChanged = FALSE;
        OptionsMenu(&OptionEntry);
        if (gBootChanged) {
          AfterTool = TRUE;
          MainLoopRunning = FALSE;
          break;
        }
        continue;
      }

      if (MenuExit == MENU_EXIT_HELP){
        HelpRefit();
        continue;
      }

      // EjectVolume
      if (MenuExit == MENU_EXIT_EJECT){
        Status = EFI_SUCCESS;
        if (ChosenEntry->getLOADER_ENTRY() ) {
          Status = EjectVolume(ChosenEntry->getLOADER_ENTRY()->Volume);
        }
        if ( ChosenEntry->getLEGACY_ENTRY() ) {
          Status = EjectVolume(ChosenEntry->getLEGACY_ENTRY()->Volume);
        }
        if (!EFI_ERROR(Status)) {
          break; //main loop is broken so Reinit all
        }
        continue;
      }

      // Hide toggle
      if (MenuExit == MENU_EXIT_HIDE_TOGGLE) {
        gSettings.ShowHiddenEntries = !gSettings.ShowHiddenEntries;
        AfterTool = TRUE;
        break;
      }

      // We don't allow exiting the main menu with the Escape key.
      if (MenuExit == MENU_EXIT_ESCAPE){
        break;   //refresh main menu
        //           continue;
      }

      if ( ChosenEntry->getREFIT_MENU_ITEM_RESET() ) {    // Restart
        if (MenuExit == MENU_EXIT_DETAILS) {
//            EFI_KEY_DATA KeyData;
//            ZeroMem(&KeyData, sizeof KeyData);
//            SimpleTextEx->ReadKeyStrokeEx (SimpleTextEx, &KeyData);
//            if ((KeyData.KeyState.KeyShiftState & (EFI_LEFT_CONTROL_PRESSED | EFI_RIGHT_CONTROL_PRESSED)) != 0) {
          //do clear cmos as for AMI BIOS
          // not sure for more robust method
          IoWrite8 (PCAT_RTC_ADDRESS_REGISTER, 0x10);
          IoWrite8 (PCAT_RTC_DATA_REGISTER, 0x0);
          IoWrite8 (PCAT_RTC_ADDRESS_REGISTER, 0x11);
          IoWrite8 (PCAT_RTC_DATA_REGISTER, 0x0);
// or may be
//           IoWrite8 (PCAT_RTC_ADDRESS_REGISTER, 0x17);
//           IoWrite8 (PCAT_RTC_DATA_REGISTER, 0x17);

//          }
        }
        // Attempt warm reboot
        gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
        // Warm reboot may not be supported attempt cold reboot
        gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
        // Terminate the screen and just exit
        TerminateScreen();
        MainLoopRunning = FALSE;
        ReinitDesktop = FALSE;
        AfterTool = TRUE;
      }

      if ( ChosenEntry->getREFIT_MENU_ITEM_SHUTDOWN() ) { // It is not Shut Down, it is Exit from Clover
        TerminateScreen();
        //         gRT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        MainLoopRunning = FALSE;   // just in case we get this far
        ReinitDesktop = FALSE;
        AfterTool = TRUE;
      }
      if ( ChosenEntry->getREFIT_MENU_ITEM_OPTIONS() ) {    // Options like KernelFlags, DSDTname etc.
        gBootChanged = FALSE;
        OptionsMenu(&OptionEntry);
        if (gBootChanged)
          AfterTool = TRUE;
        if (gBootChanged || gThemeChanged) // If theme has changed reinit the desktop
          MainLoopRunning = FALSE;
      }
      if ( ChosenEntry->getREFIT_MENU_ITEM_ABOUT() ) {    // About rEFIt
        AboutRefit();
      }

  /* -- not passed here
//  case TAG_HELP:
      HelpRefit();
      break;
  */
      if ( ChosenEntry->getLOADER_ENTRY() ) {   // Boot OS via .EFI loader
        SetBootCurrent(ChosenEntry->getLOADER_ENTRY());
        StartLoader(ChosenEntry->getLOADER_ENTRY());
        //if boot.efi failed we should somehow exit from the loop
        TerminateScreen();
        MainLoopRunning = FALSE;
        ReinitDesktop = FALSE;
        AfterTool = TRUE;
      }
      if ( ChosenEntry->getLEGACY_ENTRY() ) {   // Boot legacy OS
        if (StrCmp(gST->FirmwareVendor, L"Phoenix Technologies Ltd.") == 0 &&
            gST->Hdr.Revision >> 16 == 2 && (gST->Hdr.Revision & ((1 << 16) - 1)) == 0){
          // Phoenix SecureCore Tiano 2.0 can't properly initiate LegacyBios protocol when called externally
          // which results in "Operating System not found" message coming from BIOS
          // in this case just quit Clover to enter BIOS again
          TerminateScreen();
          MainLoopRunning = FALSE;
          ReinitDesktop = FALSE;
          AfterTool = TRUE;
        } else {
          SetBootCurrent(ChosenEntry->getLEGACY_ENTRY());
          StartLegacy(ChosenEntry->getLEGACY_ENTRY());
        }
      }

      if ( ChosenEntry->getREFIT_MENU_ENTRY_LOADER_TOOL() ) {     // Start a EFI tool
        StartTool(ChosenEntry->getREFIT_MENU_ENTRY_LOADER_TOOL());
        TerminateScreen(); //does not happen
        //   return EFI_SUCCESS;
        //  BdsLibConnectAllDriversToAllControllers();
        //    PauseForKey(L"Returned from StartTool\n");
        MainLoopRunning = FALSE;
        AfterTool = TRUE;
      }

  #ifdef ENABLE_SECURE_BOOT
      if ( ChosenEntry->getREFIT_MENU_ENTRY_SECURE_BOOT() ) { // Try to enable secure boot
            EnableSecureBoot();
            MainLoopRunning = FALSE;
            AfterTool = TRUE;
      }

      if ( ChosenEntry->getREFIT_MENU_ENTRY_SECURE_BOOT_CONFIG() ) { // Configure secure boot
            MainLoopRunning = !ConfigureSecureBoot();
            AfterTool = TRUE;
      }
  #endif // ENABLE_SECURE_BOOT

      if ( ChosenEntry->getREFIT_MENU_ENTRY_CLOVER() ) {     // Clover options
        REFIT_MENU_ENTRY_CLOVER* LoaderEntry = ChosenEntry->getREFIT_MENU_ENTRY_CLOVER();
        if (LoaderEntry->LoadOptions.notEmpty()) {
          // we are uninstalling in case user selected Clover Options and EmuVar is installed
          // because adding bios boot option requires access to real nvram
          //Slice: sure?
     /*     if (gEmuVariableControl != NULL) {
            gEmuVariableControl->UninstallEmulation(gEmuVariableControl);
          }
      */
          if (  LoaderEntry->LoadOptions.contains("BO-ADD")  ) {
            CHAR16 *Description;
            CONST CHAR16 *VolName;
            CONST CHAR16 *LoaderName;
            INTN EntryIndex, NameSize, Name2Size;
            LOADER_ENTRY *Entry;
            UINT8 *OptionalData;
            UINTN OptionalDataSize;
            UINTN BootNum;

            PrintBootOptions(FALSE);

            for (EntryIndex = 0; EntryIndex < (INTN)MainMenu.Entries.size(); EntryIndex++) {
              if (MainMenu.Entries[EntryIndex].Row != 0) {
                continue;
              }
              if (!MainMenu.Entries[EntryIndex].getLOADER_ENTRY()) {
                continue;
              }

              Entry = (LOADER_ENTRY *)MainMenu.Entries[EntryIndex].getLOADER_ENTRY();
              VolName = Entry->Volume->VolName;
              if (VolName == NULL) {
                VolName = L"";
              }
              NameSize = StrSize(VolName); //can't use StrSize with NULL! Stupid UEFI!!!
              Name2Size = 0;
              if (Entry->LoaderPath != NULL) {
                LoaderName = Basename(Entry->LoaderPath);
              } else {
                LoaderName = NULL;  //legacy boot
              }
              if (LoaderName != NULL) {
                Name2Size = StrSize(LoaderName);
              }

              Description = PoolPrint(L"Clover start %s at %s", (LoaderName != NULL)?LoaderName:L"legacy", VolName);
              OptionalDataSize = NameSize + Name2Size + 4 + 2; //signature + VolNameSize
              OptionalData = (__typeof__(OptionalData))AllocateZeroPool(OptionalDataSize);
              if (OptionalData == NULL) {
                break;
              }
              CopyMem(OptionalData, "Clvr", 4); //signature = 0x72766c43
              CopyMem(OptionalData + 4, &NameSize, 2);
              CopyMem(OptionalData + 6, VolName, NameSize);
              if (Name2Size != 0) {
                CopyMem(OptionalData + 6 + NameSize, LoaderName, Name2Size);
              }

              Status = AddBootOptionForFile (
                                    LoaderEntry->Volume->DeviceHandle,
                                    LoaderEntry->LoaderPath,
                                    TRUE,
                                    Description,
                                    OptionalData,
                                    OptionalDataSize,
                                    EntryIndex,
                                    (UINT16*)&BootNum
                                    );
              if (!EFI_ERROR(Status)) {
				  DBG("Entry %lld assigned option %04llX\n", EntryIndex, BootNum);
                Entry->BootNum = BootNum;
              }
              FreePool(OptionalData);
              FreePool(Description);
            } //for (EntryIndex


            PrintBootOptions(FALSE);
          } else if ( LoaderEntry->LoadOptions.contains("BO-REMOVE") ) {
            PrintBootOptions(FALSE);
            Status = DeleteBootOptionForFile (LoaderEntry->Volume->DeviceHandle,
                                              LoaderEntry->LoaderPath
                                              );
            PrintBootOptions(FALSE);
          } else if ( LoaderEntry->LoadOptions.contains("BO-PRINT") ) {
            PrintBootOptions(TRUE);
          }

        }
        MainLoopRunning = FALSE;
        AfterTool = TRUE;
      }
    } //MainLoopRunning
    UninitRefitLib();
    if (!AfterTool) {
      //   PauseForKey(L"After uninit");
      //reconnectAll
      if (!gFirmwareClover) {
        BdsLibConnectAllEfi();
      }
      else {
        DBG("ConnectAll after refresh menu\n");
        BdsLibConnectAllDriversToAllControllers();
      }
      //  ReinitRefitLib();
      //    PauseForKey(L"After ReinitRefitLib");
    }
    if (ReinitDesktop) {
      DBG("ReinitSelfLib after theme change\n");
      ReinitSelfLib();
    }
    //    PauseForKey(L"After ReinitSelfLib");
  } while (ReinitDesktop);

  // If we end up here, things have gone wrong. Try to reboot, and if that
  // fails, go into an endless loop.
  //Slice - NO!!! Return to EFI GUI
  //   gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
  //   EndlessIdleLoop();

#ifdef ENABLE_SECURE_BOOT
  UninstallSecureBoot();
#endif // ENABLE_SECURE_BOOT

  // Unload EmuVariable before returning to EFI GUI, as it should not be present when booting other Operating Systems.
  // This seems critical in some UEFI implementations, such as Phoenix UEFI 2.0
  if (gEmuVariableControl != NULL) {
    gEmuVariableControl->UninstallEmulation(gEmuVariableControl);
  }
  return EFI_SUCCESS;
}


