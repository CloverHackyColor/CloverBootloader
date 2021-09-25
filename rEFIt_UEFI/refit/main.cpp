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

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include <cpp_util/globals_ctor.h>
#include <cpp_util/globals_dtor.h>

#include "../cpp_foundation/XString.h"
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
#include "../Platform/spd.h"
#include "../Platform/Injectors.h"
#include "../Platform/StartupSound.h"
#include "../Platform/BootOptions.h"
#include "../Platform/boot.h"
#include "../Platform/kext_inject.h"
#include "../Platform/KextList.h"
#include "../gui/REFIT_MENU_SCREEN.h"
#include "../gui/REFIT_MAINMENU_SCREEN.h"
#include "../Settings/Self.h"
#include "../Settings/SelfOem.h"
#include "../Platform/BasicIO.h"
#include "../include/OSTypes.h"
#include "../include/OSFlags.h"
#include "../libeg/XTheme.h"
#include "../Settings/ConfigManager.h"
#include "../Platform/CloverVersion.h"
#include "../Platform/SmbiosFillPatchingValues.h"

#include "../include/OC.h"


#ifndef DEBUG_ALL
# ifdef DEBUG_ERALY_CRASH
#   define DEBUG_MAIN 2
# else
#   define DEBUG_MAIN 1
# endif
#else
# define DEBUG_MAIN DEBUG_ALL
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

EFI_HANDLE AudioDriverHandle;
XStringW OpenRuntimeEfiName;

extern void HelpRefit(void);
extern void AboutRefit(void);
//extern BOOLEAN BooterPatch(IN UINT8 *BooterData, IN UINT64 BooterSize, LOADER_ENTRY *Entry);

extern UINTN                 ConfigsNum;
extern CHAR16                *ConfigsList[];
extern UINTN                 DsdtsNum;
extern CHAR16                *DsdtsList[];
extern EFI_AUDIO_IO_PROTOCOL *AudioIo;

extern EFI_DXE_SERVICES  *gDS;

//#ifdef _cplusplus
//void FreePool(const wchar_t * A)
//{
//  FreePool((void*)A);
//}
//#endif

static EFI_STATUS LoadEFIImageList(IN EFI_DEVICE_PATH **DevicePaths,
                                    IN CONST XStringW& ImageTitle,
                                    OUT UINTN *ErrorInStep,
                                    OUT EFI_HANDLE *NewImageHandle)
{
  EFI_STATUS              Status, ReturnStatus;
  EFI_HANDLE              ChildImageHandle = 0;
  UINTN                   DevicePathIndex;
//  CHAR16                  ErrorInfo[256];

  DBG("Loading %ls", ImageTitle.wc_str());
  if (ErrorInStep != NULL) {
    *ErrorInStep = 0;
  }
  if (NewImageHandle != NULL) {
    *NewImageHandle = NULL;
  }

  // load the image into memory
  ReturnStatus = Status = EFI_NOT_FOUND;  // in case the list is empty
  for (DevicePathIndex = 0; DevicePaths[DevicePathIndex] != NULL; DevicePathIndex++) {
    ReturnStatus = Status = gBS->LoadImage(FALSE, self.getSelfImageHandle(), DevicePaths[DevicePathIndex], NULL, 0, &ChildImageHandle);
    DBG(" status=%s", efiStrError(Status));
    if (ReturnStatus != EFI_NOT_FOUND)
      break;
  }
  XStringW ErrorInfo =  SWPrintf(" while loading %ls", ImageTitle.wc_str());
  if (CheckError(Status, ErrorInfo.wc_str())) {
    if (ErrorInStep != NULL)
      *ErrorInStep = 1;
    PauseForKey(NullXString8);
    goto bailout;
  }else{
    DBG("\n");
#ifdef JIEF_DEBUG
    DBG("ChildImaheHandle=%llx\n", uintptr_t(ChildImageHandle));
#endif
  }

  if (!EFI_ERROR(ReturnStatus)) { //why unload driver?!
    if (NewImageHandle != NULL) {
      *NewImageHandle = ChildImageHandle;
    }
#ifdef JIEF_DEBUG
    EFI_LOADED_IMAGE_PROTOCOL* loadedBootImage = NULL;
    if (!EFI_ERROR(Status = gBS->HandleProtocol(ChildImageHandle, &gEfiLoadedImageProtocolGuid, (void**)(&loadedBootImage)))) {
      DBG("%ls : Image base = 0x%llx\n", ImageTitle.wc_str(), (uintptr_t)loadedBootImage->ImageBase); // Jief : Do not change this, it's used by grep to feed the debugger
    }else{
      DBG("Can't get loaded image protocol\n");
    }
#endif
    goto bailout;
  }

  // unload the image, we don't care if it works or not...
  Status = gBS->UnloadImage(ChildImageHandle);
bailout:
  return ReturnStatus;
}


static EFI_STATUS StartEFILoadedImage(IN EFI_HANDLE ChildImageHandle,
                                    IN CONST XString8Array& LoadOptions, IN CONST XStringW& LoadOptionsPrefix,
                                    IN CONST XStringW& ImageTitle,
                                    OUT UINTN *ErrorInStep)
{
  EFI_STATUS                  Status, ReturnStatus;
  EFI_LOADED_IMAGE_PROTOCOL   *ChildLoadedImage;
  CHAR16                      ErrorInfo[256];
//  CHAR16                  *FullLoadOptions = NULL;
  XStringW loadOptionsW; // This has to be declared here, so it's not be freed before calling StartImage

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
    ReturnStatus = Status = gBS->HandleProtocol(ChildImageHandle, &gEfiLoadedImageProtocolGuid, (void **) &ChildLoadedImage);
    if (CheckError(Status, L"while getting a LoadedImageProtocol handle")) {
      if (ErrorInStep != NULL)
        *ErrorInStep = 2;
      goto bailout_unload;
    }

    if ( LoadOptionsPrefix.notEmpty() ) {
      // NOTE: That last space is also added by the EFI shell and seems to be significant
      //  when passing options to Apple's boot.efi...
      loadOptionsW = SWPrintf("%ls %s ", LoadOptionsPrefix.wc_str(), LoadOptions.ConcatAll(" "_XS8).c_str());
    }else{
      loadOptionsW = SWPrintf("%s ", LoadOptions.ConcatAll(" "_XS8).c_str()); // Jief : should we add a space ? Wasn't the case before big refactoring. Yes, a space required.
    }
    // NOTE: We also include the terminating null in the length for safety.
    ChildLoadedImage->LoadOptionsSize = (UINT32)loadOptionsW.sizeInBytes() + sizeof(wchar_t);
    ChildLoadedImage->LoadOptions = loadOptionsW.wc_str(); //will it be deleted after the procedure exit? Yes, if we don't copy loadOptionsW, so it'll be freed at the end of method
    //((UINT32)StrLen(LoadOptions) + 1) * sizeof(CHAR16);
    DBG("start image '%ls'\n", ImageTitle.s());
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

  ReinitRefitLib();
  // control returns here when the child image calls Exit()
  if (ImageTitle.notEmpty()) {
    snwprintf(ErrorInfo, 512, "returned from %ls", ImageTitle.s());
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
                                IN CONST XStringW& ImageTitle,
                                OUT UINTN *ErrorInStep,
                                OUT EFI_HANDLE *NewImageHandle)
{
  EFI_DEVICE_PATH *DevicePaths[2];

#ifdef ENABLE_SECURE_BOOT
  // Verify secure boot policy
  if (GlobalConfig.SecureBoot && GlobalConfig.SecureBootSetupMode) {
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
                                IN CONST XString8Array& LoadOptions, IN CONST XStringW& LoadOptionsPrefix,
                                IN CONST XStringW& ImageTitle,
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
void DumpKernelAndKextPatches(KERNEL_AND_KEXT_PATCHES *Patches)
{
  if (!Patches) {
    DBG("Kernel and Kext Patches null pointer\n");
    return;
  }
  DBG("Kernel and Kext Patches at %llx:\n", (uintptr_t)Patches);
  DBG("\tAllowed: %c\n", GlobalConfig.KextPatchesAllowed ? 'y' : 'n');
  DBG("\tDebug: %c\n", Patches->KPDebug ? 'y' : 'n');
//  DBG("\tKernelCpu: %c\n", Patches->KPKernelCpu ? 'y' : 'n');
  DBG("\tKernelLapic: %c\n", Patches->KPKernelLapic ? 'y' : 'n');
  DBG("\tKernelXCPM: %c\n", Patches->KPKernelXCPM ? 'y' : 'n');
  DBG("\tKernelPm: %c\n", Patches->KPKernelPm ? 'y' : 'n');
  DBG("\tAppleIntelCPUPM: %c\n", Patches->KPAppleIntelCPUPM ? 'y' : 'n');
  DBG("\tAppleRTC: %c\n", Patches->KPAppleRTC ? 'y' : 'n');
  // Dell smbios truncate fix
  DBG("\tDellSMBIOSPatch: %c\n", Patches->KPDELLSMBIOS ? 'y' : 'n');
  DBG("\tFakeCPUID: 0x%X\n", Patches->FakeCPUID);
  DBG("\tATIController: %s\n", Patches->KPATIConnectorsController.isEmpty() ? "(null)": Patches->KPATIConnectorsController.c_str());
  DBG("\tATIDataLength: %zu\n", Patches->KPATIConnectorsData.size());
  DBG("\t%zu Kexts to load\n", Patches->ForceKextsToLoad.size());
  if (Patches->ForceKextsToLoad.size()) {
    size_t i = 0;
    for (; i < Patches->ForceKextsToLoad.size(); ++i) {
       DBG("\t  KextToLoad[%zu]: %ls\n", i, Patches->ForceKextsToLoad[i].wc_str());
    }
  }
  DBG("\t%zu Kexts to patch\n", Patches->KextPatches.size());
  if (Patches->KextPatches.size()) {
    size_t i = 0;
    for (; i < Patches->KextPatches.size(); ++i) {
       if (Patches->KextPatches[i].IsPlistPatch) {
          DBG("\t  KextPatchPlist[%zu]: %zu bytes, %s\n", i, Patches->KextPatches[i].Data.size(), Patches->KextPatches[i].Name.c_str());
       } else {
          DBG("\t  KextPatch[%zu]: %zu bytes, %s\n", i, Patches->KextPatches[i].Data.size(), Patches->KextPatches[i].Name.c_str());
       }
    }
  }
}
#endif
void LOADER_ENTRY::FilterKextPatches()
{
  if ( GlobalConfig.KextPatchesAllowed && KernelAndKextPatches.KextPatches.size() > 0 ) {
    DBG("Filtering KextPatches:\n");
    for (size_t i = 0; i < KernelAndKextPatches.KextPatches.size(); i++) {
      DBG(" - [%02zu]: %s :: %s :: [OS: %s | MatchOS: %s | MatchBuild: %s]",
        i,
        KernelAndKextPatches.KextPatches[i].Label.c_str(),
        KernelAndKextPatches.KextPatches[i].IsPlistPatch ? "PlistPatch" : "BinPatch",
        macOSVersion.asString().c_str(),
        KernelAndKextPatches.KextPatches[i].MatchOS.notEmpty() ? KernelAndKextPatches.KextPatches[i].MatchOS.c_str() : "All",
        KernelAndKextPatches.KextPatches[i].MatchBuild.notEmpty() ? KernelAndKextPatches.KextPatches[i].MatchBuild.c_str() : "All"
      );
      if (!gSettings.KernelAndKextPatches.KextPatches[i].MenuItem.BValue) {
        KernelAndKextPatches.KextPatches[i].MenuItem.BValue = false;
        DBG(" ==> disabled by user\n");
        continue;
      }
      KernelAndKextPatches.KextPatches[i].MenuItem.BValue = true;
      if ((BuildVersion.notEmpty()) && (KernelAndKextPatches.KextPatches[i].MatchBuild.notEmpty())) {
        KernelAndKextPatches.KextPatches[i].MenuItem.BValue = KernelAndKextPatches.KextPatches[i].IsPatchEnabledByBuildNumber(BuildVersion);
        DBG(" ==> %s\n", KernelAndKextPatches.KextPatches[i].MenuItem.BValue ? "allowed" : "not allowed");
        continue; 
      }

      KernelAndKextPatches.KextPatches[i].MenuItem.BValue = KernelAndKextPatches.KextPatches[i].IsPatchEnabled(macOSVersion);
      DBG(" ==> %s\n", KernelAndKextPatches.KextPatches[i].MenuItem.BValue ? "allowed" : "not allowed");
    }
  }
}

void LOADER_ENTRY::FilterKernelPatches()
{
  if ( GlobalConfig.KernelPatchesAllowed && KernelAndKextPatches.KernelPatches.notEmpty() ) {
    DBG("Filtering KernelPatches:\n");
    for (size_t i = 0; i < KernelAndKextPatches.KernelPatches.size(); ++i) {
      DBG(" - [%02zu]: %s :: [OS: %s | MatchOS: %s | MatchBuild: %s]",
        i,
        KernelAndKextPatches.KernelPatches[i].Label.c_str(),
        macOSVersion.asString().c_str(),
        KernelAndKextPatches.KernelPatches[i].MatchOS.notEmpty() ? KernelAndKextPatches.KernelPatches[i].MatchOS.c_str() : "All",
        KernelAndKextPatches.KernelPatches[i].MatchBuild.notEmpty() ? KernelAndKextPatches.KernelPatches[i].MatchBuild.c_str() : "no"
      );
      if (!gSettings.KernelAndKextPatches.KernelPatches[i].MenuItem.BValue) {
        KernelAndKextPatches.KernelPatches[i].MenuItem.BValue = false;
        DBG(" ==> disabled by user\n");
        continue;
      }
      KernelAndKextPatches.KernelPatches[i].MenuItem.BValue = true;
      if ((BuildVersion.notEmpty()) && (KernelAndKextPatches.KernelPatches[i].MatchBuild.notEmpty())) {
        KernelAndKextPatches.KernelPatches[i].MenuItem.BValue = KernelAndKextPatches.KernelPatches[i].IsPatchEnabledByBuildNumber(BuildVersion);
        DBG(" ==> %s by build\n", KernelAndKextPatches.KernelPatches[i].MenuItem.BValue ? "allowed" : "not allowed");
        continue; 
      }

      KernelAndKextPatches.KernelPatches[i].MenuItem.BValue = KernelAndKextPatches.KernelPatches[i].IsPatchEnabled(macOSVersion);
      DBG(" ==> %s by OS\n", KernelAndKextPatches.KernelPatches[i].MenuItem.BValue ? "allowed" : "not allowed");
    }
  }
}

void LOADER_ENTRY::FilterBootPatches()
{
  if ( KernelAndKextPatches.BootPatches.notEmpty() ) {
    DBG("Filtering BootPatches:\n");
    for (size_t i = 0; i < KernelAndKextPatches.BootPatches.size(); ++i) {
      DBG(" - [%02zu]: %s :: [OS: %s | MatchOS: %s | MatchBuild: %s]",
          i,
          KernelAndKextPatches.BootPatches[i].Label.c_str(),
          macOSVersion.asString().c_str(),
          KernelAndKextPatches.BootPatches[i].MatchOS.notEmpty() ? KernelAndKextPatches.BootPatches[i].MatchOS.c_str() : "All",
          KernelAndKextPatches.BootPatches[i].MatchBuild.notEmpty() ? KernelAndKextPatches.BootPatches[i].MatchBuild.c_str() : "no"
          );
      if (!gSettings.KernelAndKextPatches.BootPatches[i].MenuItem.BValue) {
        DBG(" ==> disabled by user\n");
        continue;
      }
      KernelAndKextPatches.BootPatches[i].MenuItem.BValue = true;
      if ((BuildVersion.notEmpty()) && (KernelAndKextPatches.BootPatches[i].MatchBuild.notEmpty())) {
        KernelAndKextPatches.BootPatches[i].MenuItem.BValue = KernelAndKextPatches.BootPatches[i].IsPatchEnabledByBuildNumber(BuildVersion);
        DBG(" ==> %s by build\n", KernelAndKextPatches.BootPatches[i].MenuItem.BValue ? "allowed" : "not allowed");
        continue;
      }
 
      KernelAndKextPatches.BootPatches[i].MenuItem.BValue = KernelAndKextPatches.BootPatches[i].IsPatchEnabled(macOSVersion);
      DBG(" ==> %s by OS\n", KernelAndKextPatches.BootPatches[i].MenuItem.BValue ? "allowed" : "not allowed");
  
    }
  }
}
/*
void ReadSIPCfg()
{
  UINT32 csrCfg = gSettings.RtVariables.CsrActiveConfig & CSR_VALID_FLAGS;
  CHAR16 *csrLog = (__typeof__(csrLog))AllocateZeroPool(SVALUE_MAX_SIZE);

  if (csrCfg & CSR_ALLOW_UNTRUSTED_KEXTS)
    StrCatS(csrLog, SVALUE_MAX_SIZE/2, L"CSR_ALLOW_UNTRUSTED_KEXTS");
  if (csrCfg & CSR_ALLOW_UNRESTRICTED_FS)
    StrCatS(csrLog, SVALUE_MAX_SIZE/2, P__oolPrint(L"%a%a", StrLen(csrLog) ? " | " : "", "CSR_ALLOW_UNRESTRICTED_FS"));
  if (csrCfg & CSR_ALLOW_TASK_FOR_PID)
    StrCatS(csrLog, SVALUE_MAX_SIZE/2, P__oolPrint(L"%a%a", StrLen(csrLog) ? " | " : "", "CSR_ALLOW_TASK_FOR_PID"));
  if (csrCfg & CSR_ALLOW_KERNEL_DEBUGGER)
    StrCatS(csrLog, SVALUE_MAX_SIZE/2, P__oolPrint(L"%a%a", StrLen(csrLog) ? " | " : "", "CSR_ALLOW_KERNEL_DEBUGGER"));
  if (csrCfg & CSR_ALLOW_APPLE_INTERNAL)
    StrCatS(csrLog, SVALUE_MAX_SIZE/2, P__oolPrint(L"%a%a", StrLen(csrLog) ? " | " : "", "CSR_ALLOW_APPLE_INTERNAL"));
  if (csrCfg & CSR_ALLOW_UNRESTRICTED_DTRACE)
    StrCatS(csrLog, SVALUE_MAX_SIZE/2, P__oolPrint(L"%a%a", StrLen(csrLog) ? " | " : "", "CSR_ALLOW_UNRESTRICTED_DTRACE"));
  if (csrCfg & CSR_ALLOW_UNRESTRICTED_NVRAM)
    StrCatS(csrLog, SVALUE_MAX_SIZE/2, P__oolPrint(L"%a%a", StrLen(csrLog) ? " | " : "", "CSR_ALLOW_UNRESTRICTED_NVRAM"));
  if (csrCfg & CSR_ALLOW_DEVICE_CONFIGURATION)
    StrCatS(csrLog, SVALUE_MAX_SIZE/2, P__oolPrint(L"%a%a", StrLen(csrLog) ? " | " : "", "CSR_ALLOW_DEVICE_CONFIGURATION"));
  if (csrCfg & CSR_ALLOW_ANY_RECOVERY_OS)
    StrCatS(csrLog, SVALUE_MAX_SIZE/2, P__oolPrint(L"%a%a", StrLen(csrLog) ? " | " : "", "CSR_ALLOW_ANY_RECOVERY_OS"));
  if (csrCfg & CSR_ALLOW_UNAPPROVED_KEXTS)
    StrCatS(csrLog, SVALUE_MAX_SIZE/2, P__oolPrint(L"%a%a", StrLen(csrLog) ? " | " : "", "CSR_ALLOW_UNAPPROVED_KEXTS"));
    
  if (StrLen(csrLog)) {
    DBG("CSR_CFG: %ls\n", csrLog);
  }

  FreePool(csrLog);
}
*/
//
// Null ConOut OutputString() implementation - for blocking
// text output from boot.efi when booting in graphics mode
//
EFI_STATUS EFIAPI
NullConOutOutputString(IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *, IN CONST CHAR16 *) {
  return EFI_SUCCESS;
}

//
// EFI OS loader functions
//
//EG_PIXEL DarkBackgroundPixel  = { 0x0, 0x0, 0x0, 0xFF };

void CheckEmptyFB()
{
  BOOLEAN EmptyFB = (GlobalConfig.IgPlatform == 0x00050000) ||
  (GlobalConfig.IgPlatform == 0x01620007) ||
  (GlobalConfig.IgPlatform == 0x04120004) ||
  (GlobalConfig.IgPlatform == 0x19120001) ||
  (GlobalConfig.IgPlatform == 0x59120003) ||
  (GlobalConfig.IgPlatform == 0x3E910003);
  if (EmptyFB) {
    gSettings.Smbios.gPlatformFeature |= PT_FEATURE_HAS_HEADLESS_GPU;
  } else {
    gSettings.Smbios.gPlatformFeature &= ~PT_FEATURE_HAS_HEADLESS_GPU;
  }
}

size_t setKextAtPos(XObjArray<SIDELOAD_KEXT>* kextArrayPtr, const XString8& kextName, size_t pos)
{
  XObjArray<SIDELOAD_KEXT>& kextArray = *kextArrayPtr;

  for (size_t kextIdx = 0 ; kextIdx < kextArray.size() ; kextIdx++ ) {
    if ( kextArray[kextIdx].FileName.contains(kextName) ) {
#ifdef DEBUG
      if ( pos >= kextArray.size() ) panic("pos >= kextArray.size()");
#else
      //it is impossible
#endif
      if ( pos == kextIdx ) return pos+1;
      if ( pos > kextIdx ) pos -= 1;
      SIDELOAD_KEXT* kextToMove = &kextArray[kextIdx];
      kextArray.RemoveWithoutFreeingAtIndex(kextIdx);
      kextArray.InsertRef(kextToMove, pos, false);
      return pos+1;
    }
  }
  return pos;
}

static XStringW getDriversPath()
{
#if defined(MDE_CPU_X64)
  if (gFirmwareClover) {
    if (FileExists(&self.getCloverDir(), L"drivers\\BIOS")) {
      return L"drivers\\BIOS"_XSW;
    } else {
      return L"drivers64"_XSW; //backward compatibility
    }
  } else
  if (FileExists(&self.getCloverDir(), L"drivers\\UEFI")) {
    return L"drivers\\UEFI"_XSW;
  } else {
    return L"drivers64UEFI"_XSW;
  }
#else
  return L"drivers32"_XSW;
#endif
}

#ifdef JIEF_DEBUG
void debugStartImageWithOC()
{
  MsgLog("debugStartImageWithOC\n");
  UINT64 CPUFrequencyFromART;
  InternalCalculateARTFrequencyIntel(&CPUFrequencyFromART, NULL, 1);

  EFI_LOADED_IMAGE* OcLoadedImage;
  EFI_STATUS Status = gBS->HandleProtocol(gImageHandle, &gEfiLoadedImageProtocolGuid, (void **) &OcLoadedImage);
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem = OcLocateFileSystem(OcLoadedImage->DeviceHandle, OcLoadedImage->FilePath);
  Status = OcStorageInitFromFs(&mOpenCoreStorage, FileSystem, NULL, NULL, self.getCloverDirFullPath().wc_str(), NULL);

  Status = ClOcReadConfigurationFile(&mOpenCoreStorage, L"config-oc.plist", &mOpenCoreConfiguration);
  if ( EFI_ERROR(Status) ) panic("ClOcReadConfigurationFile");

  mOpenCoreConfiguration.Misc.Debug.Target = 0;
  OC_STRING_ASSIGN(mOpenCoreConfiguration.Misc.Boot.PickerMode, "Builtin");
  OC_STRING_ASSIGN(mOpenCoreConfiguration.Misc.Security.DmgLoading, "Any");
  mOpenCoreConfiguration.Uefi.Quirks.IgnoreInvalidFlexRatio = 0;
  mOpenCoreConfiguration.Uefi.Quirks.TscSyncTimeout = 0;

  OcMain(&mOpenCoreStorage, NULL);

  XStringW devicePathToLookFor;
//  devicePathToLookFor.takeValueFrom("PciRoot(0x0)/Pci(0x1F,0x2)/Sata(0x0,0x0,0x0)/HD(4,GPT,CA224585-830E-4274-5826-1ACB6DA08A4E,0x299F000,0x4AE6310)/VenMedia(BE74FCF7-0B7C-49F3-9147-01F4042E6842,1ABE434C8D0357398516CFDF0A9DD7EF)"); // Jief High Sierra DevicePath
  devicePathToLookFor.takeValueFrom("PciRoot(0x0)/Pci(0x1F,0x2)/Sata(0x0,0x0,0x0)/HD(2,GPT,D8C7DA82-1E4C-4579-BA7C-6737A5D43464,0x64028,0x1BF08E8)"); // Jief Big Sur Install device path
  UINTN                   HandleCount = 0;
  EFI_HANDLE              *Handles = NULL;
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &HandleCount, &Handles);
  UINTN HandleIndex = 0;
  for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
    EFI_DEVICE_PATH_PROTOCOL* DevicePath = DevicePathFromHandle(Handles[HandleIndex]);
    CHAR16* UnicodeDevicePath = ConvertDevicePathToText(DevicePath, FALSE, FALSE);
    MsgLog("debugStartImageWithOC : path %ls\n", UnicodeDevicePath);
    if ( StrCmp(devicePathToLookFor.wc_str(), UnicodeDevicePath) == 0 ) break;
  }
  if ( HandleIndex < HandleCount )
  {
    EFI_DEVICE_PATH_PROTOCOL* jfkImagePath = FileDevicePath(Handles[HandleIndex], L"\\System\\Library\\CoreServices\\boot.efi");
    CHAR16* UnicodeDevicePath = ConvertDevicePathToText (jfkImagePath, FALSE, FALSE); (void)UnicodeDevicePath;

    EFI_HANDLE EntryHandle = NULL;

    // point to InternalEfiLoadImage from OC
    Status = gBS->LoadImage (
      FALSE,
      gImageHandle,
      jfkImagePath,
      NULL,
      0,
      &EntryHandle
      );
    if ( EFI_ERROR(Status) ) return; // TODO message ?

    EFI_LOADED_IMAGE *LoadedImage = NULL;
    EFI_STATUS OptionalStatus = gBS->HandleProtocol (
        EntryHandle,
        &gEfiLoadedImageProtocolGuid,
        (void **) &LoadedImage
        );
    if ( EFI_ERROR(OptionalStatus) ) return; // TODO message ?

  //  XStringW LoadOptionsAsXStringW = SWPrintf("%s ", LoadOptions.ConcatAll(" "_XS8).c_str());
    XStringW LoadOptionsAsXStringW = SWPrintf("boot.efi -v -no_compat_check slide=0 kext-dev-mode=1 keepsyms=1 -wegdbg igfxgl=1 bpr_probedelay=200 bpr_initialdelay=400 bpr_postresetdelay=400 ");
    LoadedImage->LoadOptions = (void*)LoadOptionsAsXStringW.wc_str();
    LoadedImage->LoadOptionsSize = (UINT32)LoadOptionsAsXStringW.sizeInBytesIncludingTerminator();

    // point to OcStartImage from OC
    Status = gBS->StartImage (EntryHandle, 0, NULL);
    if ( EFI_ERROR(Status) ) return; // TODO message ?
  }else{
    MsgLog("debugStartImageWithOC : not found\n");
  }
}
#endif

//const UINT32 standardMask[4] = {0xFF, 0xFF, 0xFF, 0xFF};

void LOADER_ENTRY::DelegateKernelPatches()
{
  XObjArray<ABSTRACT_KEXT_OR_KERNEL_PATCH> selectedPathArray;
  for (size_t kextPatchIdx = 0 ; kextPatchIdx < KernelAndKextPatches.KextPatches.size() ; kextPatchIdx++ )
  {
    if ( KernelAndKextPatches.KextPatches[kextPatchIdx].MenuItem.BValue )
      selectedPathArray.AddReference(&KernelAndKextPatches.KextPatches[kextPatchIdx], false);
  }
  for (size_t kernelPatchIdx = 0 ; kernelPatchIdx < KernelAndKextPatches.KernelPatches.size() ; kernelPatchIdx++ )
  {
    if ( KernelAndKextPatches.KernelPatches[kernelPatchIdx].MenuItem.BValue )
      selectedPathArray.AddReference(&KernelAndKextPatches.KernelPatches[kernelPatchIdx], false);
  }
  mOpenCoreConfiguration.Kernel.Patch.Count = (UINT32)selectedPathArray.size();
  mOpenCoreConfiguration.Kernel.Patch.AllocCount = mOpenCoreConfiguration.Kernel.Patch.Count;
  mOpenCoreConfiguration.Kernel.Patch.ValueSize = sizeof(__typeof_am__(**mOpenCoreConfiguration.Kernel.Patch.Values));
  mOpenCoreConfiguration.Kernel.Patch.Values = (__typeof_am__(*mOpenCoreConfiguration.Kernel.Patch.Values)*)malloc(mOpenCoreConfiguration.Kernel.Patch.AllocCount*sizeof(__typeof_am__(*mOpenCoreConfiguration.Kernel.Patch.Values)));
  memset(mOpenCoreConfiguration.Kernel.Patch.Values, 0, mOpenCoreConfiguration.Kernel.Patch.AllocCount*sizeof(*mOpenCoreConfiguration.Kernel.Patch.Values));
  
  UINT32 FakeCPU = gSettings.KernelAndKextPatches.FakeCPUID;
//  for (size_t Idx = 0; Idx < 4; Idx++) {
//    mOpenCoreConfiguration.Kernel.Emulate.Cpuid1Data[Idx] = FakeCPU & 0xFF;
//    mOpenCoreConfiguration.Kernel.Emulate.Cpuid1Mask[Idx] = 0xFF;
//    FakeCPU >>= 8;
//  }
  memset(mOpenCoreConfiguration.Kernel.Emulate.Cpuid1Data, 0, sizeof(mOpenCoreConfiguration.Kernel.Emulate.Cpuid1Data));
  memset(mOpenCoreConfiguration.Kernel.Emulate.Cpuid1Mask, 0, sizeof(mOpenCoreConfiguration.Kernel.Emulate.Cpuid1Mask));
  mOpenCoreConfiguration.Kernel.Emulate.Cpuid1Data[0] = FakeCPU;
  mOpenCoreConfiguration.Kernel.Emulate.Cpuid1Mask[0] = 0xFFFFFFFF;

  for (size_t kextPatchIdx = 0 ; kextPatchIdx < selectedPathArray.size() ; kextPatchIdx++ )
  {
    const ABSTRACT_KEXT_OR_KERNEL_PATCH& kextPatch = selectedPathArray[kextPatchIdx];  //as well as kernel patches
    DBG("Bridge %s patch to OC : %s\n", kextPatch.getName().c_str(), kextPatch.Label.c_str());
    mOpenCoreConfiguration.Kernel.Patch.Values[kextPatchIdx] = (__typeof_am__(*mOpenCoreConfiguration.Kernel.Patch.Values))AllocateZeroPool(mOpenCoreConfiguration.Kernel.Patch.ValueSize); // sizeof(OC_KERNEL_ADD_ENTRY) == 680
    OC_STRING_ASSIGN(mOpenCoreConfiguration.Kernel.Patch.Values[kextPatchIdx]->Arch, OC_BLOB_GET(&mOpenCoreConfiguration.Kernel.Scheme.KernelArch));
    OC_STRING_ASSIGN(mOpenCoreConfiguration.Kernel.Patch.Values[kextPatchIdx]->Base, kextPatch.ProcedureName.c_str());
    OC_STRING_ASSIGN(mOpenCoreConfiguration.Kernel.Patch.Values[kextPatchIdx]->Comment, kextPatch.Label.c_str());
    mOpenCoreConfiguration.Kernel.Patch.Values[kextPatchIdx]->Count = (UINT32)kextPatch.Count;
    mOpenCoreConfiguration.Kernel.Patch.Values[kextPatchIdx]->Enabled = 1;
    
    OC_DATA_ASSIGN_N(mOpenCoreConfiguration.Kernel.Patch.Values[kextPatchIdx]->Find, kextPatch.Find.data(), kextPatch.Find.size());
    OC_STRING_ASSIGN(mOpenCoreConfiguration.Kernel.Patch.Values[kextPatchIdx]->Identifier, kextPatch.getName().c_str());
    mOpenCoreConfiguration.Kernel.Patch.Values[kextPatchIdx]->Limit = (UINT32)kextPatch.SearchLen;
    OC_DATA_ASSIGN_N(mOpenCoreConfiguration.Kernel.Patch.Values[kextPatchIdx]->Mask, kextPatch.MaskFind.data(), kextPatch.MaskFind.size());
    OC_STRING_ASSIGN(mOpenCoreConfiguration.Kernel.Patch.Values[kextPatchIdx]->MaxKernel, ""); // it has been filtered, so we don't need to set Min and MaxKernel
    OC_STRING_ASSIGN(mOpenCoreConfiguration.Kernel.Patch.Values[kextPatchIdx]->MinKernel, "");
    OC_DATA_ASSIGN_N(mOpenCoreConfiguration.Kernel.Patch.Values[kextPatchIdx]->Replace, kextPatch.Replace.data(), kextPatch.Replace.size());
    OC_DATA_ASSIGN_N(mOpenCoreConfiguration.Kernel.Patch.Values[kextPatchIdx]->ReplaceMask, kextPatch.MaskReplace.data(), kextPatch.MaskReplace.size());
    mOpenCoreConfiguration.Kernel.Patch.Values[kextPatchIdx]->Skip = (UINT32)kextPatch.Skip;
#ifdef JIEF_DEBUG
if ( kextPatch.Label ==  "algrey - cpuid_set_info - ryzen cores and logicals count - part 3 - 10.14"_XS8 ) {
  DEBUG (( DEBUG_INFO, "" ));
}
#endif
  }
}

void LOADER_ENTRY::StartLoader()
{
  EFI_STATUS              Status;
  EFI_TEXT_STRING         ConOutOutputString = 0;
  EFI_HANDLE              ImageHandle = NULL;
  EFI_LOADED_IMAGE        *LoadedImage = NULL;
  CONST CHAR8                   *InstallerVersion;
  NSVGfont                *font; // , *nextFont;

  DbgHeader("StartLoader");
  
  DBG("Starting %ls\n", FileDevicePathToXStringW(DevicePath).wc_str());

  if (Settings.notEmpty()) {
    DBG("  Settings: %ls\n", Settings.wc_str());
    Status = gConf.ReLoadConfig(Settings);
    if (!EFI_ERROR(Status)) {
      DBG(" - found custom settings for this entry: %ls\n", Settings.wc_str());
    } else {
      DBG(" - [!] LoadUserSettings failed: %s\n", efiStrError(Status));
      /* we are not sure of the state of gSettings here... try to boot anyway */
    }
  }
  
  DBG("Finally: ExternalClock=%lluMHz BusSpeed=%llukHz CPUFreq=%uMHz",
          DivU64x32(gCPUStructure.ExternalClock + Kilo - 1, Kilo),
          DivU64x32(gCPUStructure.FSBFrequency + Kilo - 1, Kilo),
          gCPUStructure.MaxSpeed);
  if (gSettings.CPU.QPI) {
    DBG(" QPI: hw.busfrequency=%lluHz\n", MultU64x32(gSettings.CPU.QPI, Mega));
  } else {
    // to match the value of hw.busfrequency in the terminal
    DBG(" PIS: hw.busfrequency=%lluHz\n", MultU64x32(LShiftU64(DivU64x32(gCPUStructure.ExternalClock + Kilo - 1, Kilo), 2), Mega));
  }
  
  //Free memory
  for (size_t i = 0; i < ConfigsNum; i++) {
    if (ConfigsList[i]) {
      FreePool(ConfigsList[i]);
      ConfigsList[i] = NULL;
    }
  }
  for (size_t i = 0; i < DsdtsNum; i++) {
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
  
  //DumpKernelAndKextPatches(KernelAndKextPatches);

  if ( OSTYPE_IS_OSX(LoaderType) || OSTYPE_IS_OSX_RECOVERY(LoaderType) || OSTYPE_IS_OSX_INSTALLER(LoaderType) ) {

      // These 2 drivers are now filtered and won't load. This check is currentmy useless.
//    {
//      EFI_HANDLE Interface = NULL;
//      Status = gBS->LocateProtocol(&gAptioMemoryFixProtocolGuid, NULL, &Interface );
//      if ( !EFI_ERROR(Status) ) {
//#ifdef DEBUG
//        panic("Remove AptioMemoryFix.efi and OcQuirks.efi from your driver folder\n");
//#else
//        DBG("Remove AptioMemoryFix.efi and OcQuirks.efi from your driver folder\n");
//#endif
//      }
//    }


  // if OC is NOT initialized with OcMain, we need the following
  //  OcConfigureLogProtocol (
  //    9,
  //    0,
  //    2151678018,
  //    2147483648,
  //    OPEN_CORE_LOG_PREFIX_PATH,
  //    mOpenCoreStorage.FileSystem
  //    );
  //  DEBUG ((DEBUG_INFO, "OC: Log initialized...\n"));
  //  OcAppleDebugLogInstallProtocol(0);


  //debugStartImageWithOC();

  DBG("Beginning OC\n");

//    UINT64 CPUFrequencyFromART;
//    InternalCalculateARTFrequencyIntel(&CPUFrequencyFromART, NULL, 1);

    EFI_LOADED_IMAGE* OcLoadedImage;
    Status = gBS->HandleProtocol(gImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &OcLoadedImage);
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem = OcLocateFileSystem(OcLoadedImage->DeviceHandle, OcLoadedImage->FilePath);
    Status = OcStorageInitFromFs(&mOpenCoreStorage, FileSystem, NULL, NULL, self.getCloverDirFullPath().wc_str(), NULL);

  /*
   * Define READ_FROM_OC to have mOpenCoreConfiguration initialized from config-oc.plist
   * The boot should work.
   * Next, comment out the next lines one by one. Once the boot failed, we got the section that
   * holds the setting that makes a difference.
   */
  //#define USE_OC_SECTION_Acpi
  //#define USE_OC_SECTION_Booter
  //#define USE_OC_SECTION_DeviceProperties
  //#define USE_OC_SECTION_Kernel
  //#define USE_OC_SECTION_Misc
  //#define USE_OC_SECTION_Nvram
  //#define USE_OC_SECTION_PlatformInfo
  //#define USE_OC_SECTION_Uefi

  #if !defined(USE_OC_SECTION_Acpi) && !defined(USE_OC_SECTION_Booter) && !defined(USE_OC_SECTION_DeviceProperties) && !defined(USE_OC_SECTION_Kernel) && !defined(USE_OC_SECTION_Misc) && \
      !defined(USE_OC_SECTION_Nvram) && !defined(USE_OC_SECTION_PlatformInfo) && !defined(USE_OC_SECTION_Uefi)

    memset(&mOpenCoreConfiguration, 0, sizeof(mOpenCoreConfiguration));
    DBG("config-oc.plist isn't used at all\n");

  #else
    Status = ClOcReadConfigurationFile(&mOpenCoreStorage, L"config-oc.plist", &mOpenCoreConfiguration);
    if ( EFI_ERROR(Status) ) panic("ClOcReadConfigurationFile");

    #ifndef USE_OC_SECTION_Acpi
      memset(&mOpenCoreConfiguration.Acpi, 0, sizeof(mOpenCoreConfiguration.Acpi));
      DBG("Erase mOpenCoreConfiguration.Acpi\n");
    #else
      DBG("Keep mOpenCoreConfiguration.Acpi\n");
    #endif
    #ifndef USE_OC_SECTION_Booter
      memset(&mOpenCoreConfiguration.Booter, 0, sizeof(mOpenCoreConfiguration.Booter));
      DBG("Erase mOpenCoreConfiguration.Booter\n");
    #else
      DBG("Keep mOpenCoreConfiguration.Booter\n");
    #endif
    #ifndef USE_OC_SECTION_DeviceProperties
      memset(&mOpenCoreConfiguration.DeviceProperties, 0, sizeof(mOpenCoreConfiguration.DeviceProperties));
      DBG("Erase mOpenCoreConfiguration.DeviceProperties\n");
    #else
      DBG("Keep mOpenCoreConfiguration.DeviceProperties\n");
    #endif
    #ifndef USE_OC_SECTION_Kernel
      memset(&mOpenCoreConfiguration.Kernel, 0, sizeof(mOpenCoreConfiguration.Kernel));
      DBG("Erase mOpenCoreConfiguration.Kernel\n");
    #else
      DBG("Keep mOpenCoreConfiguration.Kernel\n");
      for ( size_t i = 0 ; i < mOpenCoreConfiguration.Kernel.Add.Count ; i ++ ) {
        OC_KERNEL_ADD_ENTRY* entry = mOpenCoreConfiguration.Kernel.Add.Values[i];
        OC_STRING_ASSIGN(entry->BundlePath, S8Printf("Kexts\\%s", OC_BLOB_GET(&entry->BundlePath)).c_str());
      }

//      DBG("mOpenCoreConfiguration.Kernel.Add.Count=%d\n", mOpenCoreConfiguration.Kernel.Add.Count);
//      for ( size_t i = 0 ; i < mOpenCoreConfiguration.Kernel.Add.Count ; i++ )
//      {
//        DBG("mOpenCoreConfiguration.Kernel.Add.Values[%zd]->Identifier=%s\n", i, OC_BLOB_GET(&mOpenCoreConfiguration.Kernel.Add.Values[i]->Identifier));
//        DBG("mOpenCoreConfiguration.Kernel.Add.Values[%zd]->BundlePath=%s\n", i, OC_BLOB_GET(&mOpenCoreConfiguration.Kernel.Add.Values[i]->BundlePath));
//        DBG("mOpenCoreConfiguration.Kernel.Add.Values[%zd]->PlistPath=%s\n", i, OC_BLOB_GET(&mOpenCoreConfiguration.Kernel.Add.Values[i]->PlistPath));
//      }
    #endif
    #ifndef USE_OC_SECTION_Misc
      memset(&mOpenCoreConfiguration.Misc, 0, sizeof(mOpenCoreConfiguration.Misc));
      DBG("Erase mOpenCoreConfiguration.Misc\n");
    #else
      DBG("Keep mOpenCoreConfiguration.Misc\n");
    #endif
    #ifndef USE_OC_SECTION_Nvram
      memset(&mOpenCoreConfiguration.Nvram, 0, sizeof(mOpenCoreConfiguration.Nvram));
      DBG("Erase mOpenCoreConfiguration.Nvram\n");
    #else
      DBG("Keep mOpenCoreConfiguration.Nvram\n");
    #endif
    #ifndef USE_OC_SECTION_PlatformInfo
      memset(&mOpenCoreConfiguration.PlatformInfo, 0, sizeof(mOpenCoreConfiguration.PlatformInfo));
      DBG("Erase mOpenCoreConfiguration.PlatformInfo\n");
    #else
      DBG("Keep mOpenCoreConfiguration.PlatformInfo\n");
    #endif
    #ifndef USE_OC_SECTION_Uefi
      memset(&mOpenCoreConfiguration.Uefi, 0, sizeof(mOpenCoreConfiguration.Uefi));
      DBG("Erase mOpenCoreConfiguration.Uefi\n");
    #else
      DBG("Keep mOpenCoreConfiguration.Uefi\n");
  //    memset(&mOpenCoreConfiguration.Uefi.Apfs, 0, sizeof(mOpenCoreConfiguration.Uefi.Apfs));
  //    memset(&mOpenCoreConfiguration.Uefi.Audio, 0, sizeof(mOpenCoreConfiguration.Uefi.Audio));
  //    memset(&mOpenCoreConfiguration.Uefi.ConnectDrivers, 0, sizeof(mOpenCoreConfiguration.Uefi.ConnectDrivers));
  //    memset(&mOpenCoreConfiguration.Uefi.Drivers, 0, sizeof(mOpenCoreConfiguration.Uefi.Drivers));
  //    memset(&mOpenCoreConfiguration.Uefi.Input, 0, sizeof(mOpenCoreConfiguration.Uefi.Input));
  //    memset(&mOpenCoreConfiguration.Uefi.Output, 0, sizeof(mOpenCoreConfiguration.Uefi.Output));
  //    memset(&mOpenCoreConfiguration.Uefi.ProtocolOverrides, 0, sizeof(mOpenCoreConfiguration.Uefi.ProtocolOverrides));
  //    memset(&mOpenCoreConfiguration.Uefi.Quirks, 0, sizeof(mOpenCoreConfiguration.Uefi.Quirks));
  //    memset(&mOpenCoreConfiguration.Uefi.ReservedMemory, 0, sizeof(mOpenCoreConfiguration.Uefi.ReservedMemory)); // doesn't matter
    #endif

  #endif



    if ( gSettings.Boot.DebugLog ) {
      mOpenCoreConfiguration.Misc.Debug.AppleDebug = true;
      mOpenCoreConfiguration.Misc.Debug.ApplePanic = true;
  //    mOpenCoreConfiguration.Misc.Debug.DisableWatchDog = true; // already done by Clover ?
  #ifndef LESS_DEBUG
      mOpenCoreConfiguration.Misc.Debug.DisplayLevel = 0x80400042;
  #else
      mOpenCoreConfiguration.Misc.Debug.DisplayLevel = 0x80000042;
  #endif
      mOpenCoreConfiguration.Misc.Debug.Target = 0x41;
    }else{
  #ifdef JIEF_DEBUG
      egSetGraphicsModeEnabled(false);
      mOpenCoreConfiguration.Misc.Debug.ApplePanic = true;
      mOpenCoreConfiguration.Misc.Debug.DisplayLevel = 0x80000042;
      mOpenCoreConfiguration.Misc.Debug.Target = 0x3;
  #endif
    }

  #ifndef USE_OC_SECTION_Misc
    OC_STRING_ASSIGN(mOpenCoreConfiguration.Misc.Security.SecureBootModel, "Disabled");
    OC_STRING_ASSIGN(mOpenCoreConfiguration.Misc.Security.Vault, "Optional");
  #endif
  #ifdef USE_OC_SECTION_Nvram
    mOpenCoreConfiguration.Nvram.WriteFlash = true;
  #endif

  #ifndef USE_OC_SECTION_Booter

    mOpenCoreConfiguration.Booter.MmioWhitelist.Count = (UINT32)gSettings.Quirks.mmioWhiteListArray.size();
    mOpenCoreConfiguration.Booter.MmioWhitelist.AllocCount = mOpenCoreConfiguration.Booter.MmioWhitelist.Count;
    mOpenCoreConfiguration.Booter.MmioWhitelist.ValueSize = sizeof(__typeof_am__(**mOpenCoreConfiguration.Booter.MmioWhitelist.Values)); // sizeof(OC_KERNEL_ADD_ENTRY) == 680
    if ( mOpenCoreConfiguration.Booter.MmioWhitelist.Count > 0 ) {
      mOpenCoreConfiguration.Booter.MmioWhitelist.Values = (OC_BOOTER_WL_ENTRY**)AllocatePool(mOpenCoreConfiguration.Booter.MmioWhitelist.AllocCount*sizeof(*mOpenCoreConfiguration.Booter.MmioWhitelist.Values)); // sizeof(OC_KERNEL_ADD_ENTRY) == 680
    }else{
      mOpenCoreConfiguration.Booter.MmioWhitelist.Values = NULL;
    }
    for ( size_t idx = 0 ; idx < gSettings.Quirks.mmioWhiteListArray.size() ; idx++ ) {
      const SETTINGS_DATA::QuirksClass::MMIOWhiteList& entry = gSettings.Quirks.mmioWhiteListArray[idx];
      DBG("Bridge mmioWhiteList[%zu] to OC : comment=%s\n", idx, entry.comment.c_str());
      mOpenCoreConfiguration.Booter.MmioWhitelist.Values[idx] = (__typeof_am__(*mOpenCoreConfiguration.Booter.MmioWhitelist.Values))AllocatePool(mOpenCoreConfiguration.Booter.MmioWhitelist.ValueSize);
      mOpenCoreConfiguration.Booter.MmioWhitelist.Values[idx]->Address = entry.address;
      OC_STRING_ASSIGN(mOpenCoreConfiguration.Booter.MmioWhitelist.Values[idx]->Comment, entry.comment.c_str());
      mOpenCoreConfiguration.Booter.MmioWhitelist.Values[idx]->Enabled = entry.enabled;
    }

    // It's possible to memcpy the whole struct instead of assigning individual member. But that would be relying on internel C++ binary structure,
    // and worse, if a field is added by OC, everything could be shifted.
    memset(&mOpenCoreConfiguration.Booter.Quirks, 0, sizeof(mOpenCoreConfiguration.Booter.Quirks));
    mOpenCoreConfiguration.Booter.Quirks.AvoidRuntimeDefrag = gSettings.Quirks.OcBooterQuirks.AvoidRuntimeDefrag;
    mOpenCoreConfiguration.Booter.Quirks.DevirtualiseMmio = gSettings.Quirks.OcBooterQuirks.DevirtualiseMmio;
    mOpenCoreConfiguration.Booter.Quirks.DisableSingleUser = gSettings.Quirks.OcBooterQuirks.DisableSingleUser;
    mOpenCoreConfiguration.Booter.Quirks.DisableVariableWrite = gSettings.Quirks.OcBooterQuirks.DisableVariableWrite;
    mOpenCoreConfiguration.Booter.Quirks.DiscardHibernateMap = gSettings.Quirks.OcBooterQuirks.DiscardHibernateMap;
    mOpenCoreConfiguration.Booter.Quirks.EnableSafeModeSlide = gSettings.Quirks.OcBooterQuirks.EnableSafeModeSlide;
    mOpenCoreConfiguration.Booter.Quirks.EnableWriteUnprotector = gSettings.Quirks.OcBooterQuirks.EnableWriteUnprotector;
    mOpenCoreConfiguration.Booter.Quirks.ForceExitBootServices = gSettings.Quirks.OcBooterQuirks.ForceExitBootServices;
    mOpenCoreConfiguration.Booter.Quirks.ProtectMemoryRegions = gSettings.Quirks.OcBooterQuirks.ProtectMemoryRegions;
    mOpenCoreConfiguration.Booter.Quirks.ProtectSecureBoot = gSettings.Quirks.OcBooterQuirks.ProtectSecureBoot;
    mOpenCoreConfiguration.Booter.Quirks.ProtectUefiServices = gSettings.Quirks.OcBooterQuirks.ProtectUefiServices;
    mOpenCoreConfiguration.Booter.Quirks.ProvideCustomSlide = gSettings.Quirks.OcBooterQuirks.ProvideCustomSlide;
    mOpenCoreConfiguration.Booter.Quirks.ProvideMaxSlide = gSettings.Quirks.OcBooterQuirks.ProvideMaxSlide;
    mOpenCoreConfiguration.Booter.Quirks.RebuildAppleMemoryMap = gSettings.Quirks.OcBooterQuirks.RebuildAppleMemoryMap;
    mOpenCoreConfiguration.Booter.Quirks.SetupVirtualMap = gSettings.Quirks.OcBooterQuirks.SetupVirtualMap;
    mOpenCoreConfiguration.Booter.Quirks.SignalAppleOS = gSettings.Quirks.OcBooterQuirks.SignalAppleOS;
    mOpenCoreConfiguration.Booter.Quirks.SyncRuntimePermissions = gSettings.Quirks.OcBooterQuirks.SyncRuntimePermissions;

  #endif

    FillOCCpuInfo(&mOpenCoreCpuInfo);

  // if OC is NOT initialized with OcMain, we need the following
  //  OcLoadBooterUefiSupport(&mOpenCoreConfiguration);
  //  OcLoadKernelSupport(&mOpenCoreStorage, &mOpenCoreConfiguration, &mOpenCoreCpuInfo);
  //  OcImageLoaderInit ();

  #ifndef USE_OC_SECTION_Kernel

    XObjArray<SIDELOAD_KEXT> kextArray;
    if (!DoHibernateWake) {
      AddKextsInArray(&kextArray);
    }



    OC_STRING_ASSIGN(mOpenCoreConfiguration.Kernel.Scheme.KernelArch, "x86_64");
    OC_STRING_ASSIGN(mOpenCoreConfiguration.Kernel.Scheme.KernelCache, gSettings.Quirks.OcKernelCache.c_str());
    mOpenCoreConfiguration.Kernel.Scheme.FuzzyMatch = gSettings.Quirks.FuzzyMatch;

    memset(&mOpenCoreConfiguration.Kernel.Quirks, 0, sizeof(mOpenCoreConfiguration.Kernel.Quirks));
    mOpenCoreConfiguration.Kernel.Quirks.AppleCpuPmCfgLock = GlobalConfig.KPAppleIntelCPUPM;
    mOpenCoreConfiguration.Kernel.Quirks.AppleXcpmCfgLock = GlobalConfig.KPKernelPm;
    mOpenCoreConfiguration.Kernel.Quirks.AppleXcpmExtraMsrs = gSettings.Quirks.OcKernelQuirks.AppleXcpmExtraMsrs;
    mOpenCoreConfiguration.Kernel.Quirks.AppleXcpmForceBoost = gSettings.Quirks.OcKernelQuirks.AppleXcpmForceBoost;
    #ifndef USE_OC_SECTION_PlatformInfo
      mOpenCoreConfiguration.Kernel.Quirks.CustomSmbiosGuid = gSettings.KernelAndKextPatches.KPDELLSMBIOS;
    #endif
    mOpenCoreConfiguration.Kernel.Quirks.DisableIoMapper = gSettings.Quirks.OcKernelQuirks.DisableIoMapper;
    mOpenCoreConfiguration.Kernel.Quirks.DisableLinkeditJettison = gSettings.Quirks.OcKernelQuirks.DisableLinkeditJettison;
    mOpenCoreConfiguration.Kernel.Quirks.DisableRtcChecksum = gSettings.KernelAndKextPatches.KPAppleRTC;
    mOpenCoreConfiguration.Kernel.Emulate.DummyPowerManagement = gSettings.Quirks.OcKernelQuirks.DummyPowerManagement;
    mOpenCoreConfiguration.Kernel.Quirks.ExtendBTFeatureFlags = gSettings.Quirks.OcKernelQuirks.ExtendBTFeatureFlags;
    mOpenCoreConfiguration.Kernel.Quirks.ExternalDiskIcons = gSettings.Quirks.OcKernelQuirks.ExternalDiskIcons;
    mOpenCoreConfiguration.Kernel.Quirks.IncreasePciBarSize = gSettings.Quirks.OcKernelQuirks.IncreasePciBarSize;
    mOpenCoreConfiguration.Kernel.Quirks.LapicKernelPanic = gSettings.KernelAndKextPatches.KPKernelLapic;
    mOpenCoreConfiguration.Kernel.Quirks.PanicNoKextDump = gSettings.KernelAndKextPatches.KPPanicNoKextDump;
    mOpenCoreConfiguration.Kernel.Quirks.PowerTimeoutKernelPanic = gSettings.Quirks.OcKernelQuirks.PowerTimeoutKernelPanic;
    mOpenCoreConfiguration.Kernel.Quirks.ThirdPartyDrives = gSettings.Quirks.OcKernelQuirks.ThirdPartyDrives;
    mOpenCoreConfiguration.Kernel.Quirks.XhciPortLimit = gSettings.Quirks.OcKernelQuirks.XhciPortLimit;


    mOpenCoreConfiguration.Kernel.Add.Count = (UINT32)kextArray.size();
    mOpenCoreConfiguration.Kernel.Add.AllocCount = mOpenCoreConfiguration.Kernel.Add.Count;
    mOpenCoreConfiguration.Kernel.Add.ValueSize = sizeof(__typeof_am__(**mOpenCoreConfiguration.Kernel.Add.Values)); // sizeof(OC_KERNEL_ADD_ENTRY) == 680
    mOpenCoreConfiguration.Kernel.Add.Values = (OC_KERNEL_ADD_ENTRY**)malloc(mOpenCoreConfiguration.Kernel.Add.AllocCount*sizeof(*mOpenCoreConfiguration.Kernel.Add.Values)); // sizeof(OC_KERNEL_ADD_ENTRY*) == sizeof(ptr)
    memset(mOpenCoreConfiguration.Kernel.Add.Values, 0, mOpenCoreConfiguration.Kernel.Add.AllocCount*sizeof(*mOpenCoreConfiguration.Kernel.Add.Values));

    // Seems that Lilu must be first.
    size_t pos = setKextAtPos(&kextArray, "Lilu.kext"_XS8, 0);
    pos = setKextAtPos(&kextArray, "VirtualSMC.kext"_XS8, pos);
//    pos = setKextAtPos(&kextArray, "FakeSMC.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "vecLib.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "IOAudioFamily.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "FakePCIID.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "FakePCIID_XHCIMux.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "AMDRyzenCPUPowerManagement﻿﻿.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "SMCAMDProcessor.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "WhateverGreen.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "AppleALC.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "IntelMausi.kext"_XS8, pos); // not needed special order?
    pos = setKextAtPos(&kextArray, "SMCProcessor.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "SMCSuperIO.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "USBPorts.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "VoodooGPIO.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "VoodooI2CServices.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "VoodooI2C.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "VoodooI2CHID.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "VoodooSMBus.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "VoodooRMI.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "BrcmFirmwareData.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "BrcmPatchRAM2.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "BrcmPatchRAM3.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "HS80211Family.kext"_XS8, pos);
    pos = setKextAtPos(&kextArray, "AirPortAtheros40.kext"_XS8, pos);

    for (size_t kextIdx = 0 ; kextIdx < kextArray.size() ; kextIdx++ )
    {
      const SIDELOAD_KEXT& KextEntry = kextArray[kextIdx];
      DBG("Bridge kext to OC : Path=%ls\n", KextEntry.FileName.wc_str());
      mOpenCoreConfiguration.Kernel.Add.Values[kextIdx] = (__typeof_am__(*mOpenCoreConfiguration.Kernel.Add.Values))malloc(mOpenCoreConfiguration.Kernel.Add.ValueSize);
      memset(mOpenCoreConfiguration.Kernel.Add.Values[kextIdx], 0, mOpenCoreConfiguration.Kernel.Add.ValueSize);
      mOpenCoreConfiguration.Kernel.Add.Values[kextIdx]->Enabled = 1;
      OC_STRING_ASSIGN(mOpenCoreConfiguration.Kernel.Add.Values[kextIdx]->Arch, OC_BLOB_GET(&mOpenCoreConfiguration.Kernel.Scheme.KernelArch));
      OC_STRING_ASSIGN(mOpenCoreConfiguration.Kernel.Add.Values[kextIdx]->Comment, "");
      OC_STRING_ASSIGN(mOpenCoreConfiguration.Kernel.Add.Values[kextIdx]->MaxKernel, "");
      OC_STRING_ASSIGN(mOpenCoreConfiguration.Kernel.Add.Values[kextIdx]->MinKernel, "");
      OC_STRING_ASSIGN(mOpenCoreConfiguration.Kernel.Add.Values[kextIdx]->Identifier, "");

      assert( selfOem.isKextsDirFound() ); // be sure before calling getKextsPathRelToSelfDir()
      XStringW dirPath = SWPrintf("%ls\\%ls", selfOem.getKextsDirPathRelToSelfDir().wc_str(), KextEntry.KextDirNameUnderOEMPath.wc_str());
  //    XString8 bundlePath = S8Printf("%ls\\%ls\\%ls", selfOem.getKextsPathRelToSelfDir().wc_str(), KextEntry.KextDirNameUnderOEMPath.wc_str(), KextEntry.FileName.wc_str());
      XString8 bundlePath = S8Printf("%ls\\%ls", dirPath.wc_str(), KextEntry.FileName.wc_str());
      if ( FileExists(&self.getCloverDir(), bundlePath) ) {
        OC_STRING_ASSIGN(mOpenCoreConfiguration.Kernel.Add.Values[kextIdx]->BundlePath, bundlePath.c_str());
      }else{
        DBG("Cannot find kext bundlePath at '%s'\n", bundlePath.c_str());
      }
  #if 1
      //CFBundleExecutable
      BOOLEAN   NoContents = FALSE;
      XStringW  infoPlistPath = getKextPlist(dirPath, KextEntry, &NoContents); //it will be fullPath, including dir
      TagDict*  dict = getInfoPlist(infoPlistPath);
  //    BOOLEAN inject = checkOSBundleRequired(dict);
      BOOLEAN inject = true;
      if (inject) {
        if ( infoPlistPath.notEmpty()) {
          if (NoContents) {
            OC_STRING_ASSIGN(mOpenCoreConfiguration.Kernel.Add.Values[kextIdx]->PlistPath, "Info.plist");
          } else {
            OC_STRING_ASSIGN(mOpenCoreConfiguration.Kernel.Add.Values[kextIdx]->PlistPath, "Contents/Info.plist");
          }
        }else{
          DBG("Cannot find kext info.plist at '%ls'\n", KextEntry.FileName.wc_str());
        }
        XString8 execpath = getKextExecPath(dirPath, KextEntry, dict, NoContents);
        if (execpath.notEmpty()) {
          OC_STRING_ASSIGN(mOpenCoreConfiguration.Kernel.Add.Values[kextIdx]->ExecutablePath, execpath.c_str());
          DBG("assign executable as '%s'\n", execpath.c_str());
        }
      }

  #else
      XStringW execpath = S8Printf("Contents\\MacOS\\%ls", KextEntry.FileName.subString(0, KextEntry.FileName.rindexOf(".")).wc_str());
      XStringW fullPath = SWPrintf("%s\\%ls", OC_BLOB_GET(&mOpenCoreConfiguration.Kernel.Add.Values[kextIdx]->BundlePath), execpath.wc_str());
      if ( FileExists(&self.getCloverDir(), fullPath) ) {
        OC_STRING_ASSIGN(mOpenCoreConfiguration.Kernel.Add.Values[kextIdx]->ExecutablePath, S8Printf("Contents\\MacOS\\%ls", KextEntry.FileName.subString(0, KextEntry.FileName.rindexOf(".")).wc_str()).c_str());
      }
      XStringW infoPlistPath = SWPrintf("%s\\Contents\\Info.plist", OC_BLOB_GET(&mOpenCoreConfiguration.Kernel.Add.Values[kextIdx]->BundlePath));
      if ( FileExists(&self.getCloverDir(), infoPlistPath) ) {
        OC_STRING_ASSIGN(mOpenCoreConfiguration.Kernel.Add.Values[kextIdx]->PlistPath, "Contents/Info.plist"); // TODO : is always Contents/Info.plist ?
      }else{
        DBG("Cannot find kext info.plist at '%ls'\n", infoPlistPath.wc_str());
      }
  #endif
      mOpenCoreConfiguration.Kernel.Add.Values[kextIdx]->ImageData = NULL;
      mOpenCoreConfiguration.Kernel.Add.Values[kextIdx]->ImageDataSize = 0;
      mOpenCoreConfiguration.Kernel.Add.Values[kextIdx]->PlistData = NULL;
      mOpenCoreConfiguration.Kernel.Add.Values[kextIdx]->PlistDataSize = 0;

    }

  //DelegateKernelPatches();

    for (size_t forceKextIdx = 0 ; forceKextIdx < KernelAndKextPatches.ForceKextsToLoad.size() ; forceKextIdx++ )
    {
      const XStringW& forceKext = KernelAndKextPatches.ForceKextsToLoad[forceKextIdx];
      DBG("TODO !!!!!!!! Bridge force kext to OC : %ls\n", forceKext.wc_str());
    }
  #endif

    mOpenCoreConfiguration.Uefi.Output.ProvideConsoleGop = gSettings.GUI.ProvideConsoleGop;
    OC_STRING_ASSIGN(mOpenCoreConfiguration.Uefi.Output.Resolution, XString8(gSettings.GUI.ScreenResolution).c_str());


    if ( OpenRuntimeEfiName.notEmpty() ) {
      XStringW FileName = SWPrintf("%ls\\%ls\\%ls", self.getCloverDirFullPath().wc_str(), getDriversPath().wc_str(), OpenRuntimeEfiName.wc_str());
      EFI_HANDLE DriverHandle;
      Status = gBS->LoadImage(false, gImageHandle, FileDevicePath(self.getSelfLoadedImage().DeviceHandle, FileName), NULL, 0, &DriverHandle);
      if ( !EFI_ERROR(Status) ) {
        Status = gBS->StartImage(DriverHandle, 0, 0);
        DBG("Start '%ls' : Status %s\n", OpenRuntimeEfiName.wc_str(), efiStrError(Status));

        if ( !EFI_ERROR(Status) )
        {
          OC_FIRMWARE_RUNTIME_PROTOCOL  *FwRuntime;
          Status = gBS->LocateProtocol (
            &gOcFirmwareRuntimeProtocolGuid,
            NULL,
            (VOID **) &FwRuntime
            );

          if (!EFI_ERROR (Status)) {
            if (FwRuntime->Revision == OC_FIRMWARE_RUNTIME_REVISION) {
            } else {
              DEBUG ((
                DEBUG_ERROR,
                "OCABC: Incompatible OpenRuntime r%u, require r%u\n",
                (UINT32) FwRuntime->Revision,
                (UINT32) OC_FIRMWARE_RUNTIME_REVISION
                ));
              panic("Incompatible OpenRuntime r%llu, require r%u\n", FwRuntime->Revision, OC_FIRMWARE_RUNTIME_REVISION);
            }
          }
        }
      }else{
        panic("Error when loading '%ls' : Status %s.\n", OpenRuntimeEfiName.wc_str(), efiStrError(Status));
      }
    }else{
      DBG("No OpenRuntime driver. This is ok, OpenRuntime is not mandatory.\n");
    }

    OcMain(&mOpenCoreStorage, NULL);

    XStringW DevicePathAsString = DevicePathToXStringW(DevicePath);
    if ( DevicePathAsString.rindexOf(".dmg") == MAX_XSIZE )
    {
      // point to InternalEfiLoadImage from OC
      Status = gBS->LoadImage (
        FALSE,
        gImageHandle,
        DevicePath,
        NULL,
        0,
        &ImageHandle
        );
      if ( EFI_ERROR(Status) ) {
        DBG("LoadImage at '%ls' failed. Status = %s\n", DevicePathAsString.wc_str(), efiStrError(Status));
        return;
      }
      DBG("ImageHandle = %llx\n", uintptr_t(ImageHandle));
    }else
    {
      // NOTE : OpenCore ignore the name of the dmg.
      //        InternalLoadDmg calls InternalFindFirstDmgFileName to find the dmg file name.
      //        So be careful that, if an other dmg exists in the dir, that might boot on the wrong one.

      EFI_DEVICE_PATH_PROTOCOL* DevicePathCopy = DuplicateDevicePath(DevicePath);

      EFI_DEVICE_PATH_PROTOCOL* PreviousNode = NULL;
      EFI_DEVICE_PATH_PROTOCOL* Node = DevicePathCopy;
      while (!IsDevicePathEnd(Node)) {
        if (  Node->Type == MEDIA_DEVICE_PATH  &&  Node->SubType == MEDIA_FILEPATH_DP  ) {
            PreviousNode = Node;
            break;
        }
  //      CHAR16* s1 = ConvertDeviceNodeToText(Node, FALSE, FALSE);
  //      MsgLog("Split DevicePath = %ls\n", s1);
        PreviousNode = Node;
        Node = NextDevicePathNode(Node);
      }
      SetDevicePathEndNode(PreviousNode);

      EFI_DEVICE_PATH_PROTOCOL* LoaderPathBasenameNode = ConvertTextToDeviceNode(LoaderPath.dirname().wc_str());
      EFI_DEVICE_PATH_PROTOCOL* DevicePathToDmgDir = AppendDevicePathNode(DevicePathCopy, LoaderPathBasenameNode);
      DBG("DevicePathToDmgDir = %ls\n", DevicePathToXStringW(DevicePathToDmgDir).wc_str());

      INTERNAL_DMG_LOAD_CONTEXT DmgLoadContext = {0,0,0};
      DmgLoadContext.DevicePath = DevicePathToDmgDir;
      EFI_DEVICE_PATH_PROTOCOL* BootEfiFromDmgDevicePath = InternalLoadDmg(&DmgLoadContext, OcDmgLoadingAnyImage);
      DBG("DevicePath of dmg = %ls\n", DevicePathToXStringW(BootEfiFromDmgDevicePath).wc_str());

      // point to InternalEfiLoadImage from OC
      Status = gBS->LoadImage (
        FALSE,
        gImageHandle,
        BootEfiFromDmgDevicePath,
        NULL,
        0,
        &ImageHandle
        );
      if ( EFI_ERROR(Status) ) {
        DBG("LoadImage at '%ls' failed. Status = %s\n", DevicePathToXStringW(BootEfiFromDmgDevicePath).wc_str(), efiStrError(Status));
        return;
      }
    }

    EFI_STATUS OptionalStatus = gBS->HandleProtocol (
        ImageHandle,
        &gEfiLoadedImageProtocolGuid,
        (void **) &LoadedImage
        );
    if ( EFI_ERROR(OptionalStatus) ) return; // TODO message ?
  }else{
    // Load image into memory (will be started later)
    Status = LoadEFIImage(DevicePath, LoaderPath.basename(), NULL, &ImageHandle);
    if (EFI_ERROR(Status)) {
      DBG("Image is not loaded, status=%s\n", efiStrError(Status));
      return; // no reason to continue if loading image failed
    }
  }

  egClearScreen(&BootBgColor); //if not set then it is already MenuBackgroundPixel

//  KillMouse();

//  if (LoaderType == OSTYPE_OSX) {
  if (OSTYPE_IS_OSX(LoaderType) ||
      OSTYPE_IS_OSX_RECOVERY(LoaderType) ||
      OSTYPE_IS_OSX_INSTALLER(LoaderType)) {

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
    Status = gBS->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (void **) &LoadedImage);
    // Correct OSVersion if it was not found
    // This should happen only for 10.7-10.9 OSTYPE_OSX_INSTALLER
    // For these cases, take OSVersion from loaded boot.efi image in memory
    if (/*LoaderType == OSTYPE_OSX_INSTALLER ||*/ macOSVersion.isEmpty()) {

      if (!EFI_ERROR(Status)) {
        // version in boot.efi appears as "Mac OS X 10.?"
        /*
          Start OSName Mac OS X 10.12 End OSName Start OSVendor Apple Inc. End
        */
  //      InstallerVersion = SearchString((CHAR8*)LoadedImage->ImageBase, LoadedImage->ImageSize, "Mac OS X ", 9);
        InstallerVersion = AsciiStrStr((CHAR8*)LoadedImage->ImageBase, "Mac OS X ");
        int location = 9;
        if (InstallerVersion == NULL) {
          InstallerVersion = AsciiStrStr((CHAR8*)LoadedImage->ImageBase, "macOS ");
          location = 7;
        }
        if (InstallerVersion != NULL) { // string was found
          InstallerVersion += location; // advance to version location

          if (strncmp(InstallerVersion, "10.7", 4) &&
              strncmp(InstallerVersion, "10.8", 4) &&
              strncmp(InstallerVersion, "10.9", 4) &&
              strncmp(InstallerVersion, "10.10", 5) &&
              strncmp(InstallerVersion, "10.11", 5) &&
              strncmp(InstallerVersion, "10.12", 5) &&
              strncmp(InstallerVersion, "10.13", 5) &&
              strncmp(InstallerVersion, "10.14", 5) &&
              strncmp(InstallerVersion, "10.15", 5) &&
              strncmp(InstallerVersion, "10.16", 5) &&
              strncmp(InstallerVersion, "11.", 3) &&
              strncmp(InstallerVersion, "12.", 3)) {
            InstallerVersion = NULL; // flag known version was not found
          }
          if (InstallerVersion != NULL) { // known version was found in image
            macOSVersion = InstallerVersion;
            DBG("Corrected OSVersion: %s\n", macOSVersion.asString().c_str());
          }
        }
      }
      BuildVersion.setEmpty();
    }

    if (BuildVersion.notEmpty()) {
      DBG(" %s (%s)\n", macOSVersion.asString().c_str(), BuildVersion.c_str());
    } else {
      DBG(" %s\n", macOSVersion.asString().c_str());
    }

    if ( macOSVersion >= MacOsVersion("10.11"_XS8) ) {
      if (OSFLAG_ISSET(Flags, OSFLAG_NOSIP)) {
        gSettings.RtVariables.CsrActiveConfig = (UINT32)0xB7F;
        gSettings.RtVariables.BooterConfig = 0x28;
      }
//      ReadSIPCfg();
    }

    FilterKextPatches();
    FilterKernelPatches();
    FilterBootPatches();
    if (LoadedImage && !BooterPatch((UINT8*)LoadedImage->ImageBase, LoadedImage->ImageSize)) {
      DBG("Will not patch boot.efi\n");
    }
    
    DelegateKernelPatches();

    // Set boot argument for kernel if no caches, this should force kernel loading
    if (  OSFLAG_ISSET(Flags, OSFLAG_NOCACHES)  &&  !LoadOptions.containsStartWithIC("Kernel=")  ) {
      XString8 KernelLocation;

      if ( macOSVersion.notEmpty() && macOSVersion <= MacOsVersion("10.9"_XS8) ) {
        KernelLocation.S8Printf("\"Kernel=/mach_kernel\"");
      } else {
        // used for 10.10, 10.11, and new version. Jief : also for unknown version.
        KernelLocation.S8Printf("\"Kernel=/System/Library/Kernels/kernel\"");
      }
      LoadOptions.AddID(KernelLocation);
    }

//  //we are booting OSX - restore emulation if it's not installed before g boot.efi
//  if (gEmuVariableControl != NULL) {
//      gEmuVariableControl->InstallEmulation(gEmuVariableControl);
//  }

    // first patchACPI and find PCIROOT and RTC
    // but before ACPI patch we need smbios patch
    CheckEmptyFB();
    SmbiosFillPatchingValues(GlobalConfig.SetTable132, GlobalConfig.EnabledCores, g_SmbiosDiscoveredSettings.RamSlotCount, gConf.SlotDeviceArray, gSettings, gCPUStructure, &g_SmbiosInjectedSettings);
    PatchSmbios(g_SmbiosInjectedSettings);
//    DBG("PatchACPI\n");
#ifdef USE_OC_SECTION_Acpi
    // If we use the ACPI section form config-oc.plist, let's also delegate the acpi patching to OC
#else
    PatchACPI(Volume, macOSVersion);
#endif

#ifdef JIEF_DEBUG
    //SaveOemTables();
#endif
//
//  // If KPDebug is true boot in verbose mode to see the debug messages
//  if (KernelAndKextPatches.KPDebug) {
//    LoadOptions.AddID("-v"_XS8);
//  }
//
    DbgHeader("RestSetup macOS");
//
//    DBG("SetDevices\n");
    SetDevices(this);
//    DBG("SetFSInjection\n");
  // Jief : do we need that ?
  //SetFSInjection();
    //PauseForKey(L"SetFSInjection");
//    DBG("SetVariablesForOSX\n");
    SetVariablesForOSX(this);
//    DBG("SetVariablesForOSX\n");
// Jief : if we want to use our FixUSBOwnership, we need our OnExitBootServices
    EventsInitialize(this);
//    DBG("FinalizeSmbios\n");
    FinalizeSmbios(g_SmbiosInjectedSettings);

    SetCPUProperties(); //very special procedure

    if (OSFLAG_ISSET(Flags, OSFLAG_HIBERNATED)) {
      DoHibernateWake = PrepareHibernation(Volume);
    }
    SetupDataForOSX(DoHibernateWake);
    

    if (  gDriversFlags.AptioFixLoaded &&
          !DoHibernateWake &&
          !LoadOptions.containsStartWithIC("slide=")  ) {
      // Add slide=0 argument for ML+ if not present
      LoadOptions.AddID("slide=0"_XS8);
    }
     
      
    /**
     * syscl - append "-xcpm" argument conditionally if set KernelXCPM on Intel Haswell+ low-end CPUs
     */
    if (KernelAndKextPatches.KPKernelXCPM &&
        gCPUStructure.Vendor == CPU_VENDOR_INTEL && gCPUStructure.Model >= CPU_MODEL_HASWELL &&
       (AsciiStrStr(gCPUStructure.BrandString, "Celeron") || AsciiStrStr(gCPUStructure.BrandString, "Pentium")) &&
       macOSVersion >= MacOsVersion("10.8.5"_XS8)  &&  macOSVersion < MacOsVersion("10.12"_XS8)  &&
       (!LoadOptions.containsIC("-xcpm"))) {
        // add "-xcpm" argv if not present on Haswell+ Celeron/Pentium
        LoadOptions.AddID("-xcpm"_XS8);
    }
    
    // add -xcpm on Ivy Bridge if set KernelXCPM and system version is 10.8.5 - 10.11.x
    if (KernelAndKextPatches.KPKernelXCPM &&
        gCPUStructure.Model == CPU_MODEL_IVY_BRIDGE &&
        macOSVersion >= MacOsVersion("10.8.5"_XS8)  &&  macOSVersion < MacOsVersion("10.12"_XS8)  &&
        (!LoadOptions.containsIC("-xcpm"))) {
      // add "-xcpm" argv if not present on Ivy Bridge
      LoadOptions.AddID("-xcpm"_XS8);
    }

    if (AudioIo) {
      AudioIo->StopPlayback(AudioIo);
//      CheckSyncSound(true);
      EFI_DRIVER_BINDING_PROTOCOL  *DriverBinding =  NULL;
      Status = gBS->HandleProtocol(AudioDriverHandle, &gEfiDriverBindingProtocolGuid, (void **)&DriverBinding);
      if (DriverBinding) {
        DriverBinding->Stop(DriverBinding, AudioDriverHandle, 0, NULL);
      }
    }

//    DBG("Set FakeCPUID: 0x%X\n", gSettings.Devices.FakeID.FakeCPUID);
//    DBG("LoadKexts\n");
    // LoadKexts writes to DataHub, where large writes can prevent hibernate wake (happens when several kexts present in Clover's kexts dir)

// Do not call LoadKexts() with OC
//    if (!DoHibernateWake) {
//      LoadKexts();
//    }

    // blocking boot.efi output if -v is not specified
    // note: this blocks output even if -v is specified in
    // /Library/Preferences/SystemConfiguration/com.apple.Boot.plist
    // which is wrong
    // apianti - only block console output if using graphics
    //           but don't block custom boot logo
    if (LoadOptions.containsIC("-v")) {
      Flags = OSFLAG_UNSET(Flags, OSFLAG_USEGRAPHICS);
    }
  }
  else if (OSTYPE_IS_WINDOWS(LoaderType)) {

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
  else if (OSTYPE_IS_LINUX(LoaderType) || (LoaderType == OSTYPE_LINEFI)) {

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

  if (gSettings.Boot.LastBootedVolume) {
    if ( APFSTargetUUID.notEmpty() ) {
      // Jief : we need to LoaderPath. If not, GUI can't know which target was selected.
      SetStartupDiskVolume(Volume, LoaderPath);
    }else{
      // Jief : I'm not sure why NullXStringW was given if LoaderType == OSTYPE_OSX.
      //        Let's do it like it was before when not in case of APFSTargetUUID
      SetStartupDiskVolume(Volume, LoaderType == OSTYPE_OSX ? NullXStringW : LoaderPath);
    }
  } else if (gSettings.Boot.DefaultVolume.notEmpty()) {
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
  BeginExternalScreen(OSFLAG_ISSET(Flags, OSFLAG_USEGRAPHICS)/*, L"Booting OS"*/);

  if (!OSTYPE_IS_WINDOWS(LoaderType) && !OSTYPE_IS_LINUX(LoaderType)) {
    if (OSFLAG_ISSET(Flags, OSFLAG_USEGRAPHICS)) {
      // save orig OutputString and replace it with
      // null implementation
      ConOutOutputString = gST->ConOut->OutputString;
      gST->ConOut->OutputString = NullConOutOutputString;
    }
    
    // Initialize the boot screen
    if (EFI_ERROR(Status = InitBootScreen(this))) {
      if (Status != EFI_ABORTED) DBG("Failed to initialize custom boot screen: %s!\n", efiStrError(Status));
    }
    else if (EFI_ERROR(Status = LockBootScreen())) {
      DBG("Failed to lock custom boot screen: %s!\n", efiStrError(Status));
    }
  } // !OSTYPE_IS_WINDOWS

  if (OSTYPE_IS_OSX(LoaderType) ||
      OSTYPE_IS_OSX_RECOVERY(LoaderType) ||
      OSTYPE_IS_OSX_INSTALLER(LoaderType)) {

    if (DoHibernateWake) {
      DBG("Closing events for wake\n");
      gBS->CloseEvent (OnReadyToBootEvent);
//      gBS->CloseEvent (ExitBootServiceEvent); // Jief : we don't need that anymore, if we continue to use OC onExtBootService event
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


//  XStringW LoadOptionsAsXStringW = SWPrintf("%s ", LoadOptions.ConcatAll(" "_XS8).c_str());
  XStringW LoadOptionsAsXStringW = SWPrintf("%ls %s ", Basename(LoaderPath.wc_str()), LoadOptions.ConcatAll(" "_XS8).c_str());
  LoadedImage->LoadOptions = (void*)LoadOptionsAsXStringW.wc_str();
  LoadedImage->LoadOptionsSize = (UINT32)LoadOptionsAsXStringW.sizeInBytesIncludingTerminator();

  DBG("Kernel quirks\n");
  DBG("ACPCL %d AXCL %d AXEM %d AXFB %d CSG %d DIM %d DLJ %d DRC %d DPM %d EBTFF %d EDI %d IPBS %d LKP %d PNKD %d PTKP %d TPD %d XPL %d\n",
      mOpenCoreConfiguration.Kernel.Quirks.AppleCpuPmCfgLock,
      mOpenCoreConfiguration.Kernel.Quirks.AppleXcpmCfgLock,
      mOpenCoreConfiguration.Kernel.Quirks.AppleXcpmExtraMsrs,
      mOpenCoreConfiguration.Kernel.Quirks.AppleXcpmForceBoost,
      mOpenCoreConfiguration.Kernel.Quirks.CustomSmbiosGuid,
      mOpenCoreConfiguration.Kernel.Quirks.DisableIoMapper,
      mOpenCoreConfiguration.Kernel.Quirks.DisableLinkeditJettison,
      mOpenCoreConfiguration.Kernel.Quirks.DisableRtcChecksum,
      mOpenCoreConfiguration.Kernel.Emulate.DummyPowerManagement,
      mOpenCoreConfiguration.Kernel.Quirks.ExtendBTFeatureFlags,
      mOpenCoreConfiguration.Kernel.Quirks.ExternalDiskIcons,
      mOpenCoreConfiguration.Kernel.Quirks.IncreasePciBarSize,
      mOpenCoreConfiguration.Kernel.Quirks.LapicKernelPanic,
      mOpenCoreConfiguration.Kernel.Quirks.PanicNoKextDump,
      mOpenCoreConfiguration.Kernel.Quirks.PowerTimeoutKernelPanic,
      mOpenCoreConfiguration.Kernel.Quirks.ThirdPartyDrives,
      mOpenCoreConfiguration.Kernel.Quirks.XhciPortLimit);
  
  DBG("Closing log\n");
  if (SavePreBootLog) {
    Status = SaveBooterLog(&self.getCloverDir(), PREBOOT_LOG);
    // Jief : do not write outside of SelfDir
//    if (EFI_ERROR(Status)) {
//      /*Status = */SaveBooterLog(NULL, PREBOOT_LOG);
//    }
  }

#ifdef JIEF_DEBUG
    //Status = EFI_NOT_FOUND;
    Status = gBS->StartImage (ImageHandle, 0, NULL); // point to OcStartImage from OC
#else
    Status = gBS->StartImage (ImageHandle, 0, NULL); // point to OcStartImage from OC
#endif

    if ( EFI_ERROR(Status) ) {
      // Ideally, we would return to the menu, displaying an error message
      // Truth is that we get a black screen before seeing the menu again.
      // If I remember well, we get a freeze in BdsLibConnectAllEfi()
      // I'm guessing there is a lot of patching done for booting.
      // To be able to go back to the menu and boot another thing,
      // we must undo all the patching...
      // Here is a quick, not as bad as a black screen solution : a text message and a reboot !
      DBG("StartImage failed : %s\n", efiStrError(Status));
      SaveBooterLog(&self.getCloverDir(), PREBOOT_LOG);
      egSetGraphicsModeEnabled(false);
      printf("StartImage failed : %s\n", efiStrError(Status));
      PauseForKey("Reboot needed."_XS8);
      // Attempt warm reboot
      gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
      // Warm reboot may not be supported attempt cold reboot
      gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
      // Terminate the screen and just exit

      return;
    }

}else{
//  DBG("StartEFIImage\n");
//  StartEFIImage(DevicePath, LoadOptions,
//                Basename(LoaderPath), Basename(LoaderPath), NULL, NULL);

//  DBG("StartEFILoadedImage\n");
  StartEFILoadedImage(ImageHandle, LoadOptions, NullXStringW, LoaderPath.basename(), NULL);
}
  // Unlock boot screen
  if (EFI_ERROR(Status = UnlockBootScreen())) {
    DBG("Failed to unlock custom boot screen: %s!\n", efiStrError(Status));
  }
  if (OSFLAG_ISSET(Flags, OSFLAG_USEGRAPHICS)) {
    // return back orig OutputString
    gST->ConOut->OutputString = ConOutOutputString;
  }

//  PauseForKey(L"FinishExternalScreen");
  FinishExternalScreen();
//  PauseForKey(L"System started?!");
}


void LEGACY_ENTRY::StartLegacy()
{
    EFI_STATUS          Status = EFI_UNSUPPORTED;

    // Unload EmuVariable before booting legacy.
    // This is not needed in most cases, but it seems to interfere with legacy OS
    // booted on some UEFI bioses, such as Phoenix UEFI 2.0
    if (gEmuVariableControl != NULL) {
      gEmuVariableControl->UninstallEmulation(gEmuVariableControl);
    }

    if (gSettings.Boot.LastBootedVolume) {
      SetStartupDiskVolume(Volume, NullXStringW);
    } else if (gSettings.Boot.DefaultVolume.notEmpty()) {
      // DefaultVolume specified in Config.plist:
      // we'll remove macOS Startup Disk vars which may be present if it is used
      // to reboot into another volume
      RemoveStartupDiskVolume();
    }


  egClearScreen(&MenuBackgroundPixel);
  BeginExternalScreen(TRUE/*, L"Booting Legacy OS"*/);
  XImage BootLogoX;
  BootLogoX.LoadXImage(&ThemeX.getThemeDir(), Volume->LegacyOS->IconName);
  BootLogoX.Draw((UGAWidth  - BootLogoX.GetWidth()) >> 1,
                 (UGAHeight - BootLogoX.GetHeight()) >> 1);

      //try my LegacyBoot
      switch (Volume->BootType) {
        case BOOTING_BY_CD:
          Status = bootElTorito(Volume);
          break;
        case BOOTING_BY_MBR:
          Status = bootMBR(Volume);
          break;
        case BOOTING_BY_PBR:
          if (gSettings.Boot.LegacyBoot == "LegacyBiosDefault"_XS8) {
            Status = bootLegacyBiosDefault(gSettings.Boot.LegacyBiosDefaultEntry);
          } else if (gSettings.Boot.LegacyBoot == "PBRtest"_XS8) {
            Status = bootPBRtest(Volume);
          } else if (gSettings.Boot.LegacyBoot == "PBRsata"_XS8) {
            Status = bootPBR(Volume, TRUE);
          } else {
            // default
            Status = bootPBR(Volume, FALSE);
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

void REFIT_MENU_ENTRY_LOADER_TOOL::StartTool()
{
  DBG("Start Tool: %ls\n", LoaderPath.wc_str());
  egClearScreen(&MenuBackgroundPixel);
  // assumes "Start <title>" as assigned below
  BeginExternalScreen(OSFLAG_ISSET(Flags, OSFLAG_USEGRAPHICS)/*, &Entry->Title[6]*/); // Shouldn't we check that length of Title is at least 6 ?
  StartEFIImage(DevicePath, LoadOptions, NullXStringW, LoaderPath.basename(), NULL, NULL);
  FinishExternalScreen();
}

//
// pre-boot driver functions
//

static void ScanDriverDir(IN CONST CHAR16 *Path, OUT EFI_HANDLE **DriversToConnect, OUT UINTN *DriversToConnectNum)
{
  EFI_STATUS              Status;
  REFIT_DIR_ITER          DirIter;
  EFI_FILE_INFO           *DirEntry;
  EFI_HANDLE              DriverHandle;
  EFI_DRIVER_BINDING_PROTOCOL  *DriverBinding;
  UINTN                   DriversArrSize;
  UINTN                   DriversArrNum;
  EFI_HANDLE              *DriversArr;
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
  OpenRuntimeEfiName.setEmpty();

//only one driver with highest priority will obtain status "Loaded"
  DirIterOpen(&self.getCloverDir(), Path, &DirIter);
#define BOOLEAN_AT_INDEX(k) (*(BOOLEAN*)((UINTN)&gDriversFlags + AptioIndices[(k)]))
  for (size_t i = 0; i != ARRAY_SIZE(AptioIndices); ++i)
    BOOLEAN_AT_INDEX(i) = FALSE;
  AptioBlessed = (UINT8) ARRAY_SIZE(AptioNames);
  while (DirIterNext(&DirIter, 2, L"*.efi", &DirEntry)) {
    size_t i;
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
  DirIterOpen(&self.getCloverDir(), Path, &DirIter);
  while (DirIterNext(&DirIter, 2, L"*.efi", &DirEntry)) {
    Skip = (DirEntry->FileName[0] == L'.');
    for (size_t i=0; i<gSettings.DisabledDriverArray.size(); i++) {
      if (StrStr(DirEntry->FileName, gSettings.DisabledDriverArray[i].wc_str()) != NULL) {
        Skip = TRUE;   // skip this
        break;
      }
    }
    if (Skip) {
      continue;
    }
    if ( LStringW(DirEntry->FileName).containsIC("OcQuirks") ) {
      continue;
    }
    if ( LStringW(DirEntry->FileName).containsIC("AptioMemoryFix") ) {
      continue;
    }
    if ( LStringW(DirEntry->FileName).containsIC("OpenRuntime") ) {
      if ( LStringW(DirEntry->FileName).isEqualIC("OpenRuntime-v12.efi") && LString8(OPEN_CORE_VERSION).isEqual("0.7.3") ) {
        OpenRuntimeEfiName.takeValueFrom(DirEntry->FileName);
      }else
      if ( LStringW(DirEntry->FileName).isEqualIC("OpenRuntime-v11.efi") && LString8(OPEN_CORE_VERSION).isEqual("0.6.5") ) {
        OpenRuntimeEfiName.takeValueFrom(DirEntry->FileName);
      }else
      if ( LStringW(DirEntry->FileName).isEqualIC("OpenRuntime-v11.efi") && LString8(OPEN_CORE_VERSION).isEqual("0.6.1") ) {
        OpenRuntimeEfiName.takeValueFrom(DirEntry->FileName);
      }else
      if ( OpenRuntimeEfiName.isEmpty() ) {
        OpenRuntimeEfiName.takeValueFrom(DirEntry->FileName);
      }
      continue;
    }
    {
      size_t i;
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
    }
#undef BOOLEAN_AT_INDEX

    XStringW FileName = SWPrintf("%ls\\%ls\\%ls", self.getCloverDirFullPath().wc_str(), Path, DirEntry->FileName);
    Status = StartEFIImage(FileDevicePath(self.getSelfLoadedImage().DeviceHandle, FileName), NullXString8Array, LStringW(DirEntry->FileName), XStringW().takeValueFrom(DirEntry->FileName), NULL, &DriverHandle);
    if (EFI_ERROR(Status)) {
      continue;
    }
    if ( FileName.containsIC("AudioDxe") ) {
      AudioDriverHandle = DriverHandle;
    }
    if ( FileName.containsIC("EmuVariable") ) {
      gDriversFlags.EmuVariableLoaded = TRUE;
    } else if ( FileName.containsIC("Video") ) {
      gDriversFlags.VideoLoaded = TRUE;
    } else if ( FileName.containsIC("Partition") ) {
      gDriversFlags.PartitionLoaded = TRUE;
    } else if ( FileName.containsIC("HFS") ) {
      gDriversFlags.HFSLoaded = TRUE;
    } else if ( FileName.containsIC("apfs") ) {
      gDriversFlags.APFSLoaded = TRUE;
    }
    if (DriverHandle != NULL && DriversToConnectNum != NULL && DriversToConnect != NULL) {
      // driver loaded - check for EFI_DRIVER_BINDING_PROTOCOL
      Status = gBS->HandleProtocol(DriverHandle, &gEfiDriverBindingProtocolGuid, (void **) &DriverBinding);
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
    CheckError(Status, SWPrintf( "while scanning the %ls directory", Path).wc_str());
  }

  if (DriversToConnectNum != NULL && DriversToConnect != NULL) {
    *DriversToConnectNum = DriversArrNum;
    *DriversToConnect = DriversArr;
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
void DisconnectInvalidDiskIoChildDrivers(void)
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
                                  (void **) &BlockIo
                                  );
    if (EFI_ERROR(Status)) {
      //DBG(" BlockIo: %s - skipping\n", efiStrError(Status));
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
                                  (void **) &Fs
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
        //DBG(" BY_DRIVER Agent: %p, Disconnect: %s", OpenInfo[OpenInfoIndex].AgentHandle, efiStrError(Status));
        DBG(" - Handle %llx with DiskIo, is Partition, no Fs, BY_DRIVER Agent: %llx, Disconnect: %s\n", (uintptr_t)Handles[Index], (uintptr_t)(OpenInfo[OpenInfoIndex].AgentHandle), efiStrError(Status));
      }
    }
    FreePool(OpenInfo);
  }
  FreePool(Handles);

  if (!Found) {
    DBG(" not found, all ok\n");
  }
}

void DisconnectSomeDevices(void)
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
  CHAR16                           *DriverName = NULL;
  EFI_COMPONENT_NAME_PROTOCOL      *CompName = NULL;

  if (gDriversFlags.PartitionLoaded) {
    DBG("Partition driver loaded: ");
    // get all BlockIo handles
    HandleCount = 0;
    Handles = NULL;

    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &HandleCount, &Handles);
    if (Status == EFI_SUCCESS) {
      for (Index = 0; Index < HandleCount; Index++) {
        Status = gBS->HandleProtocol(Handles[Index], &gEfiBlockIoProtocolGuid, (void **) &BlockIo);
        if (EFI_ERROR(Status)) {
          continue;
        }
        if (BlockIo->Media->BlockSize == 2048) {
          // disconnect CD controller
          Status = gBS->DisconnectController(Handles[Index], NULL, NULL);
          DBG("CD disconnect %s", efiStrError(Status));
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
        DBG("Driver [%d] disconnect %s\n", Index2, efiStrError(Status));
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
                                   (void**)&CompName,
                                   gImageHandle,
                                   NULL,
                                   EFI_OPEN_PROTOCOL_GET_PROTOCOL);

        if (EFI_ERROR(Status)) {
//          DBG("CompName %s\n", efiStrError(Status));
          continue;
        }
        // 2021-05, Jief : PG7 had a crash. In some cases, CompName->GetDriverName == NULL.
        if ( CompName->GetDriverName == NULL ) {
          DBG("DisconnectSomeDevices: GetDriverName CompName=%lld, CompName->GetDriverName=NULL\n", uintptr_t(CompName));
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
            //DBG("Disconnect [%ls] from %llX: %s\n", DriverName, uintptr_t(ControllerHandles[Index2]), efiStrError(Status));
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
        Status = gBS->HandleProtocol(Handles[Index], &gEfiPciIoProtocolGuid, (void **) &PciIo);
        if (EFI_ERROR(Status)) {
          continue;
        }
        Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, sizeof (Pci) / sizeof (UINT32), &Pci);
        if (!EFI_ERROR(Status)) {
          if(IS_PCI_VGA(&Pci) == TRUE) {
            // disconnect VGA
            Status = gBS->DisconnectController(Handles[Index], NULL, NULL);
            DBG("disconnect %s", efiStrError(Status));
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


void PatchVideoBios(UINT8 *Edid)
{

  if ( gSettings.Graphics.PatchVBiosBytes.notEmpty() ) {
    VideoBiosPatchBytes(gSettings.Graphics.PatchVBiosBytes.getVBIOS_PATCH_BYTES(), gSettings.Graphics.PatchVBiosBytes.getVBIOS_PATCH_BYTES_count());
  }

  if (gSettings.Graphics.PatchVBios) {
    VideoBiosPatchNativeFromEdid(Edid);
  }
}


static void LoadDrivers(void)
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
    if (FileExists(&self.getCloverDir(), L"drivers\\BIOS")) {
      ScanDriverDir(L"drivers\\BIOS", &DriversToConnect, &DriversToConnectNum);
    } else {
      ScanDriverDir(L"drivers64", &DriversToConnect, &DriversToConnectNum);
    }
  } else
  if (FileExists(&self.getCloverDir(), L"drivers\\UEFI")) {
    ScanDriverDir(L"drivers\\UEFI", &DriversToConnect, &DriversToConnectNum);
  } else {
    ScanDriverDir(L"drivers64UEFI", &DriversToConnect, &DriversToConnectNum);
  }
#else
  ScanDriverDir(L"drivers32", &DriversToConnect, &DriversToConnectNum);
#endif

  VBiosPatchNeeded = gSettings.Graphics.PatchVBios || gSettings.Graphics.PatchVBiosBytes.getVBIOS_PATCH_BYTES_count() > 0;
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

  if ( (gSettings.Graphics.EDID.CustomEDID.notEmpty() && gFirmwareClover) || (VBiosPatchNeeded && !gDriversFlags.VideoLoaded)) {
    // we have video bios patch - force video driver reconnect
    DBG("Video bios patch requested or CustomEDID - forcing video reconnect\n");
    gDriversFlags.VideoLoaded = TRUE;
    DriversToConnectNum++;
  }

  UninitRefitLib();
  if (DriversToConnectNum > 0) {
    DBG("%llu drivers needs connecting ...\n", DriversToConnectNum);
    // note: our platform driver protocol
    // will use DriversToConnect - do not release it
    RegisterDriversToHighestPriority(DriversToConnect);
    if (VBiosPatchNeeded) {
      if (gSettings.Graphics.EDID.CustomEDID.notEmpty()) {
        Edid = gSettings.Graphics.EDID.CustomEDID.data();
      } else {
        Edid = getCurrentEdid();
      }
      DisconnectSomeDevices();
      PatchVideoBios(Edid);
      if (gSettings.Graphics.EDID.CustomEDID.isEmpty()) {
        FreePool(Edid);
      }
    } else {
      DisconnectSomeDevices();
    }
    BdsLibConnectAllDriversToAllControllers();

    // Boot speedup: remove temporary "BiosVideoBlockSwitchMode" RT var
    // to unlock mode switching in CsmVideo
    gRT->SetVariable(L"BiosVideoBlockSwitchMode", &gEfiGlobalVariableGuid, EFI_VARIABLE_BOOTSERVICE_ACCESS, 0, NULL);
  }else{
    BdsLibConnectAllEfi(); // jief : without any driver loaded, i couldn't see my CD, unless I call BdsLibConnectAllEfi
  }
  ReinitRefitLib();
}


INTN FindDefaultEntry(void)
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
  // if not null or empty, search volume that matches gSettings.Boot.DefaultVolume
  //
  if (gSettings.Boot.DefaultVolume.notEmpty()) {

    // if not null or empty, also search for loader that matches gSettings.Boot.DefaultLoader
    SearchForLoader = gSettings.Boot.DefaultLoader.notEmpty();
/*
    if (SearchForLoader) {
      DBG("Searching for DefaultVolume '%ls', DefaultLoader '%ls' ...\n", gSettings.Boot.DefaultVolume, gSettings.Boot.DefaultLoader);
    } else {
      DBG("Searching for DefaultVolume '%ls' ...\n", gSettings.Boot.DefaultVolume);
    }
*/
    for (Index = 0; Index < (INTN)MainMenu.Entries.size()  &&  MainMenu.Entries[Index].getLOADER_ENTRY()  &&  MainMenu.Entries[Index].getLOADER_ENTRY()->Row == 0 ; Index++) {

      LOADER_ENTRY& Entry = *MainMenu.Entries[Index].getLOADER_ENTRY();
      if (!Entry.Volume) {
        continue;
      }

      Volume = Entry.Volume;
      if ( (Volume->VolName.isEmpty() || Volume->VolName != gSettings.Boot.DefaultVolume)  &&
           !Volume->DevicePathString.contains(gSettings.Boot.DefaultVolume) ) {
        continue;
      }

      //                       we alreday know that Entry.isLoader
      if (SearchForLoader && (/*Entry.Tag != TAG_LOADER ||*/ !Entry.LoaderPath.containsIC(gSettings.Boot.DefaultLoader))) {
        continue;
      }

    DBG(" - found entry %lld. '%ls', Volume '%ls', DevicePath '%ls'\n", Index, Entry.Title.s(), Volume->VolName.wc_str(), Entry.DevicePathString.wc_str());
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

void SetVariablesFromNvram()
{
  CHAR8  *tmpString;
  UINTN   Size = 0;
  UINTN   index = 0, index2, i;
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
      if (!gSettings.Boot.BootArgs.contains(arg)) {
        //this arg is not present will add
        DBG("...adding arg:%s\n", arg);
        gSettings.Boot.BootArgs.trim();
        gSettings.Boot.BootArgs += ' ';
        for (i = 0; i < index2; i++) {
          gSettings.Boot.BootArgs += arg[i];
        }
        gSettings.Boot.BootArgs += ' ';
      }
    }
    FreePool(arg);
  }
  if (tmpString) {
    FreePool(tmpString);
  }
  
  tmpString = (__typeof__(tmpString))GetNvramVariable(L"nvda_drv", &gEfiAppleBootGuid, NULL, NULL);
  if (tmpString && AsciiStrCmp(tmpString, "1") == 0) {
    gSettings.SystemParameters.NvidiaWeb = TRUE;
  }
  if (tmpString) {
    FreePool(tmpString);
  }

}

void
GetListOfConfigs()
{
  REFIT_DIR_ITER    DirIter;
  EFI_FILE_INFO     *DirEntry;
  INTN              NameLen;

  ConfigsNum = 0;
  OldChosenConfig = 0;

  DirIterOpen(&selfOem.getConfigDir(), NULL, &DirIter);
  DbgHeader("Found config plists");
  while (DirIterNext(&DirIter, 2, L"config*.plist", &DirEntry)) {
    if (DirEntry->FileName[0] == L'.') {
      continue;
    }
      if (StriCmp(DirEntry->FileName, L"config.plist") == 0) {
        OldChosenConfig = ConfigsNum;
      }
      NameLen = StrLen(DirEntry->FileName) - 6; //without ".plist"
      ConfigsList[ConfigsNum] = (CHAR16*)AllocateCopyPool(NameLen * sizeof(CHAR16) + 2, DirEntry->FileName);
      ConfigsList[ConfigsNum++][NameLen] = L'\0';
      DBG("- %ls\n", DirEntry->FileName);
    }
  DirIterClose(&DirIter);
}

void
GetListOfDsdts()
{
  REFIT_DIR_ITER    DirIter;
  EFI_FILE_INFO     *DirEntry;
  INTN              NameLen;

  if (DsdtsNum > 0) {
    for (UINTN i = 0; i < DsdtsNum; i++) {
      if (DsdtsList[DsdtsNum] != NULL) {
        FreePool(DsdtsList[DsdtsNum]);
      }
    }
  }
  DsdtsNum = 0;
  OldChosenDsdt = 0xFFFF;

  DirIterOpen(&selfOem.getConfigDir(), L"ACPI\\patched", &DirIter);
  DbgHeader("Found DSDT tables");
  while (DirIterNext(&DirIter, 2, L"DSDT*.aml", &DirEntry)) {
    if (DirEntry->FileName[0] == L'.') {
      continue;
    }
      if ( gSettings.ACPI.DSDT.DsdtName.isEqualIC(DirEntry->FileName) ) {
        OldChosenDsdt = DsdtsNum;
      }
      NameLen = StrLen(DirEntry->FileName); //with ".aml"
      DsdtsList[DsdtsNum] = (CHAR16*)AllocateCopyPool(NameLen * sizeof(CHAR16) + 2, DirEntry->FileName); // if changing, notice freepool above
      DsdtsList[DsdtsNum++][NameLen] = L'\0';
      DBG("- %ls\n", DirEntry->FileName);
    }
  DirIterClose(&DirIter);
}

void
GetListOfACPI()
{
  REFIT_DIR_ITER    DirIter;
  EFI_FILE_INFO     *DirEntry = NULL;

//  XStringW           AcpiPath = SWPrintf("%ls\\ACPI\\patched", OEMPath.wc_str());
//  DBG("Get list of ACPI at path %ls\n", AcpiPath.wc_str());
  ACPIPatchedAML.setEmpty();
  DirIterOpen(&selfOem.getConfigDir(), L"ACPI\\patched", &DirIter);

  while (DirIterNext(&DirIter, 2, L"*.aml", &DirEntry)) {
//    DBG("next entry is %ls\n", DirEntry->FileName);
    if (DirEntry->FileName[0] == L'.') {
      continue;
    }
    if (StriStr(DirEntry->FileName, L"DSDT")) {
      continue;
    }
//    DBG("Found name %ls\n", DirEntry->FileName);
      BOOLEAN ACPIDisabled = FALSE;
      ACPI_PATCHED_AML* ACPIPatchedAMLTmp = new ACPI_PATCHED_AML;
      ACPIPatchedAMLTmp->FileName.takeValueFrom(DirEntry->FileName);

      INTN Count = gSettings.ACPI.DisabledAML.size();
      for (INTN i = 0; i < Count; i++) {
        if ( gSettings.ACPI.DisabledAML[i].isEqualIC(ACPIPatchedAMLTmp->FileName) ) {
//        if ((gSettings.ACPI.DisabledAML[i] != NULL) &&
//            (StriCmp(ACPIPatchedAMLTmp->FileName, gSettings.ACPI.DisabledAML[i]) == 0)
//            ) {
          ACPIDisabled = TRUE;
          break;
        }
      }
      ACPIPatchedAMLTmp->MenuItem.BValue = ACPIDisabled;
      ACPIPatchedAML.AddReference(ACPIPatchedAMLTmp, true);
    }

  DirIterClose(&DirIter);
}

void
GetListOfThemes ()
{
  EFI_STATUS     Status          = EFI_NOT_FOUND;
  REFIT_DIR_ITER DirIter;
  EFI_FILE_INFO  *DirEntry;
  XStringW        ThemeTestPath;
  EFI_FILE       *ThemeTestDir   = NULL;
  CHAR8          *ThemePtr       = NULL;
  UINTN          Size = 0;

  DbgHeader("GetListOfThemes");

  ThemeNameArray.setEmpty();
  if ( !self.themesDirExists() ) {
    DBG("No theme dir was discovered\n");
    return;
  }
  DirIterOpen(&self.getThemesDir(), NULL, &DirIter);
  while (DirIterNext(&DirIter, 1, L"*", &DirEntry)) {
    if (DirEntry->FileName[0] == '.') {
      //DBG("Skip theme: %ls\n", DirEntry->FileName);
      continue;
    }
    //DBG("Found theme directory: %ls", DirEntry->FileName);
    DBG("- [%02zu]: %ls", ThemeNameArray.size(), DirEntry->FileName);
    Status = self.getThemesDir().Open(&self.getThemesDir(), &ThemeTestDir, DirEntry->FileName, EFI_FILE_MODE_READ, 0);
    if (!EFI_ERROR(Status)) {
      Status = egLoadFile(ThemeTestDir, CONFIG_THEME_FILENAME, (UINT8**)&ThemePtr, &Size);
      if (EFI_ERROR(Status) || (ThemePtr == NULL) || (Size == 0)) {
        Status = egLoadFile(ThemeTestDir, CONFIG_THEME_SVG, (UINT8**)&ThemePtr, &Size);
        if (EFI_ERROR(Status)) {
          Status = EFI_NOT_FOUND;
          DBG(" - bad theme because %ls nor %ls can't be load", CONFIG_THEME_FILENAME, CONFIG_THEME_SVG);
        }
      }
      if (!EFI_ERROR(Status)) {
        //we found a theme
        if ((StriCmp(DirEntry->FileName, L"embedded") == 0) ||
            (StriCmp(DirEntry->FileName, L"random") == 0)) {
          ThemePtr = NULL;
        } else {
          ThemeNameArray.Add(DirEntry->FileName);
        }
      }
    }
    DBG("\n");
    if (ThemePtr) {
      FreePool(ThemePtr);
    }
  }
  DirIterClose(&DirIter);
}

////
//// analyze self.getSelfLoadedImage().LoadOptions to extract Default Volume and Default Loader
//// input and output data are global
////
//void
//GetBootFromOption(void)
//{
//  UINT8  *Data = (UINT8*)self.getSelfLoadedImage().LoadOptions;
//  UINTN  Len = self.getSelfLoadedImage().LoadOptionsSize;
//  UINTN  NameSize, Name2Size;
//
//  Data += 4; //skip signature as we already here
//  NameSize = *(UINT16*)Data;
//
//  Data += 2; // pointer to Volume name
//  settingsData.Boot.DefaultVolume.strncpy((__typeof__(settingsData.Boot.DefaultVolume.wc_str()))Data, NameSize);
//
//  Data += NameSize;
//  Name2Size = Len - NameSize;
//  if (Name2Size != 0) {
//    settingsData.Boot.DefaultLoader.strncpy((__typeof__(settingsData.Boot.DefaultVolume.wc_str()))Data, NameSize);
//  }
//
//  DBG("Clover started with option to boot %ls from %ls\n",
//      settingsData.Boot.DefaultLoader.notEmpty() ? settingsData.Boot.DefaultLoader.wc_str() : L"legacy",
//      settingsData.Boot.DefaultVolume.wc_str());
//}

//void
//ParseLoadOptions (
//                  OUT  XStringW* ConfNamePtr,
//                  OUT  TagDict** Dict
//                  )
//{
//  CHAR8 *End;
//  CHAR8 *Start;
//  UINTN TailSize;
//  UINTN i;
//  CONST CHAR8 *PlistStrings[]  =
//  {
//    "<?xml",
//    "<!DOCTYPE plist",
//    "<plist",
//    "<dict>",
//    "\0"
//  };
//
//  UINTN PlistStringsLen;
//  *Dict                  = NULL;
//
//  XStringW& ConfName = *ConfNamePtr;
//
//  Start = (CHAR8*)self.getSelfLoadedImage().LoadOptions;
//  End   = (CHAR8*)((CHAR8*)self.getSelfLoadedImage().LoadOptions + self.getSelfLoadedImage().LoadOptionsSize);
//  while ((Start < End) && ((*Start == ' ') || (*Start == '\\') || (*Start == '/')))
//  {
//    ++Start;
//  }
//
//  TailSize = End - Start;
//  //DBG("TailSize = %d\n", TailSize);
//
//  if ((TailSize) <= 0) {
//    return;
//  }
//
//  for (i = 0; PlistStrings[i][0] != '\0'; i++) {
//    PlistStringsLen = AsciiStrLen(PlistStrings[i]);
//    //DBG("PlistStrings[%d] = %s\n", i, PlistStrings[i]);
//    if (PlistStringsLen < TailSize) {
//      if (AsciiStriNCmp(PlistStrings[i], Start, PlistStringsLen)) {
//        DBG(" - found plist string = %s, parse XML in LoadOptions\n", PlistStrings[i]);
//        if (ParseXML(Start, Dict, TailSize) != EFI_SUCCESS) {
//          *Dict = NULL;
//          DBG("  - [!] xml in load options is bad\n");
//          return;
//        }
//        return;
//      }
//    }
//  }
//
//  while ((End > Start) && ((*End == ' ') || (*End == '\\') || (*End == '/'))) {
//    --End;
//  }
//
//  TailSize = End - Start;
//  //  DBG("TailSize2 = %d\n", TailSize);
//
//  if (TailSize > 6) {
//    if (AsciiStriNCmp(".plist", End - 6, 6)) {
//      End      -= 6;
//      TailSize -= 6;
//      //      DBG("TailSize3 = %d\n", TailSize);
//    }
//  } else if (TailSize <= 0) {
//    return;
//  }
//
//  ConfName.strncpy(Start, TailSize + 1);
//}


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
  UINTN             i;
  //UINT64            TscDiv;
  //UINT64            TscRemainder = 0;
//  LOADER_ENTRY      *LoaderEntry;
//  XStringW          ConfName;
//  TagDict*          smbiosTags = NULL;
//  BOOLEAN           UniteConfigs = FALSE;
  EFI_TIME          Now;
  BOOLEAN           HaveDefaultVolume;
  REFIT_MENU_SCREEN BootScreen;
  BootScreen.isBootScreen = true; //other screens will be constructed as false
  // CHAR16            *InputBuffer; //, *Y;
  //  EFI_INPUT_KEY Key;

  // Init assets dir: misc
  /*Status = */ //egMkDir(&self.getCloverDir(), L"misc");
  //Should apply to: "ACPI/origin/" too

  // get TSC freq and init MemLog if needed
  gCPUStructure.TSCCalibr = GetMemLogTscTicksPerSecond(); //ticks for 1second
  //gSettings.GUI.TextOnly = TRUE;

  // bootstrap
  gST       = SystemTable;
  gImageHandle  = ImageHandle;
  gBS       = SystemTable->BootServices;
  gRT       = SystemTable->RuntimeServices;
  /*Status = */EfiGetSystemConfigurationTable (&gEfiDxeServicesTableGuid, (void **) &gDS);
  
  InitBooterLog();

  ConsoleInHandle = SystemTable->ConsoleInHandle;

//#define DEBUG_ERALY_CRASH
#ifdef DEBUG_ERALY_CRASH
  SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Step1");
  PauseForKey("press any key\n"_XS8);
#endif

#ifdef DEBUG_ON_SERIAL_PORT
  SerialPortInitialize();
#endif

  {
    EFI_LOADED_IMAGE* LoadedImage;
    Status = gBS->HandleProtocol(gImageHandle, &gEfiLoadedImageProtocolGuid, (void **) &LoadedImage);

#ifdef DEBUG_ERALY_CRASH
  SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Step2");
  PauseForKey("press any key\n"_XS8);
#endif

//    if ( !EFI_ERROR(Status) ) {
//      XString8 msg = S8Printf("CloverX64 : Image base = 0x%llX\n", (uintptr_t)LoadedImage->ImageBase); // do not change, it's used by grep to feed the debugger
//      SerialPortWrite((UINT8*)msg.c_str(), msg.length());
//    }
    if ( !EFI_ERROR(Status) ) {
      DBG("CloverX64 : Image base = 0x%llX\n", (uintptr_t)LoadedImage->ImageBase); // do not change, it's used by grep to feed the debugger
      DBG("Clover ImageHandle = %llx\n", (uintptr_t)ImageHandle);
#ifdef DEBUG_ERALY_CRASH
  SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Step3");
  PauseForKey("press any key\n"_XS8);
#endif
    }
#ifdef JIEF_DEBUG
    gBS->Stall(2500000); // to give time to gdb to connect
//  PauseForKey(L"press\n");
#endif
  }

#ifdef DEBUG_ERALY_CRASH
  SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Step4");
  PauseForKey("press any key\n"_XS8);
#endif

#ifdef CLOVER_BUILD
  construct_globals_objects(gImageHandle); // do this after self.getSelfLoadedImage() is initialized
#endif

#ifdef DEBUG_ERALY_CRASH
  SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Step5");
  PauseForKey("press any key\n"_XS8);
#endif

#ifdef JIEF_DEBUG
//  all_tests();
//  PauseForKey(L"press\n");
#endif

  gRT->GetTime(&Now, NULL);

  Status = InitRefitLib(gImageHandle); // From here, debug.log starts to be saved because InitRefitLib call self.initialize()
  if (EFI_ERROR(Status))
    return Status;

#ifdef DEBUG_ERALY_CRASH
  SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Step6");
  PauseForKey("press any key\n"_XS8);
#endif

// firmware detection
  gFirmwareClover = StrCmp(gST->FirmwareVendor, L"CLOVER") == 0;
  if (!gFirmwareRevision) {
//    gFirmwareRevision = P__oolPrint(L"%d", gST->FirmwareRevision);
  }
  DataHubInstall (ImageHandle, SystemTable);
  InitializeConsoleSim();

  DbgHeader("Starting Clover");
  if (Now.TimeZone < -1440 || Now.TimeZone > 1440) {
    MsgLog("Now is %02d.%02d.%d,  %02d:%02d:%02d (GMT)\n",
           Now.Day, Now.Month, Now.Year, Now.Hour, Now.Minute, Now.Second);
  } else {
    MsgLog("Now is %02d.%02d.%d,  %02d:%02d:%02d (GMT+%d)\n",
      Now.Day, Now.Month, Now.Year, Now.Hour, Now.Minute, Now.Second, gSettings.GUI.Timezone);
  }
  //MsgLog("Starting Clover rev %ls on %ls EFI\n", gFirmwareRevision, gST->FirmwareVendor);
  MsgLog("Starting %s on %ls EFI\n", gRevisionStr, gST->FirmwareVendor);
  MsgLog("Build id: %s\n", gBuildId.c_str());
  if ( gBuildInfo ) DBG("Build with: [%s]\n", gBuildInfo);



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
//  ZeroMem((void*)&gSettings, sizeof(SETTINGS_DATA));

  Status = InitializeUnicodeCollationProtocol();
  if (EFI_ERROR(Status)) {
    DBG("UnicodeCollation Status=%s\n", efiStrError(Status));
  }
  
  Status = gBS->HandleProtocol(ConsoleInHandle, &gEfiSimpleTextInputExProtocolGuid, (void **)&SimpleTextEx);
  if ( EFI_ERROR(Status) ) {
    SimpleTextEx = NULL;
  }
  DBG("SimpleTextEx Status=%s\n", efiStrError(Status));

  gConf.InitialisePlatform();

#ifdef JIEF_DEBUG
  DumpNvram();
#endif

  /*
   * saving debug.log works from here
   */


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
    const CHAR16 aaa[] = L"12345  ";
    const CHAR8 *bbb = "12345  ";
    DBG(" string %ls, size=%lld, len=%lld sizeof=%ld iStrLen=%lld\n", aaa, StrSize(aaa), StrLen(aaa), sizeof(aaa), iStrLen(bbb, 10));
    const CHAR8* ccc = "Выход  ";
    DBG(" string %s, size=%lld, len=%lld sizeof=%ld iStrLen=%lld\n", ccc, AsciiStrSize(ccc), AsciiStrLen(ccc), sizeof(ccc), iStrLen(ccc, 10));
    XString8 ddd = "Выход "_XS8;
 //   size_t sizex = ddd.allocatedSize();
    DBG(" xstring %s, asize=%ld, sizeinbyte=%ld sizeof=%ld lastcharat=%ld\n", ddd.c_str(), ddd.allocatedSize(), ddd.sizeInBytes(), sizeof(ddd),
      ddd.indexOf(ddd.lastChar()));
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
    /*
     Results
     41:381  0:000   string 12345  , size=16, len=7 sizeof=16 iStrLen=5
     41:381  0:000   string Выход  , size=13, len=12 sizeof=8 iStrLen=10
     41:381  0:000   xstring Выход , asize=0, sizeinbyte=11 sizeof=16 lastcharat=5
     41:381  0:000   FakeLAN = 0x30168c
     41:381  0:000   Compatible=pci168c,30 strlen=10 sizeof=64 iStrLen=10

     */
  }
#endif
  if (!GlobalConfig.isFastBoot()) {
    GetListOfThemes();
    GetListOfConfigs();
  }
//  ThemeX.FillByEmbedded(); //init XTheme before EarlyUserSettings
  {
    void       *Value = NULL;
    UINTN       Size = 0;
    //read aptiofixflag from nvram for special boot
    Status = GetVariable2(L"aptiofixflag", &gEfiAppleBootGuid, &Value, &Size);
    if (!EFI_ERROR(Status)) {
      GlobalConfig.SpecialBootMode = TRUE;
      FreePool(Value);
      DBG("Fast option enabled\n");
    }
  }

//  for (i=0; i<2; i++) {
//    if (gConfigDict[i]) {
//      GetEarlyUserSettings(gConfigDict[i], gSettings);
//    }
//  }

#ifdef ENABLE_SECURE_BOOT
  // Install secure boot shim
  if (EFI_ERROR(Status = InstallSecureBoot())) {
    PauseForKey("Secure boot failure!\n"_XS8);
    return Status;
  }
#endif // ENABLE_SECURE_BOOT

  MainMenu.TimeoutSeconds = gSettings.Boot.Timeout >= 0 ? gSettings.Boot.Timeout : 0;
  //DBG("LoadDrivers() start\n");
  LoadDrivers();
  //DBG("LoadDrivers() end\n");

//debugStartImageWithOC(); // ok

/*  if (!gFirmwareClover &&
      !gDriversFlags.EmuVariableLoaded) {
    GetSmcKeys(FALSE);  // later we can get here SMC information
  } */
  
  Status = gBS->LocateProtocol (&gEmuVariableControlProtocolGuid, NULL, (void**)&gEmuVariableControl);
  if (EFI_ERROR(Status)) {
    gEmuVariableControl = NULL;
  }
  if (gEmuVariableControl != NULL) {
    gEmuVariableControl->InstallEmulation(gEmuVariableControl);
  }

  DbgHeader("InitScreen");

  if (!GlobalConfig.isFastBoot()) {
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

  //DBG("ReinitRefitLib\n");
  //Now we have to reinit handles
  Status = ReinitRefitLib();
  if (EFI_ERROR(Status)){
    DebugLog(2, " %s", efiStrError(Status));
    PauseForKey("Error reinit refit."_XS8);
#ifdef ENABLE_SECURE_BOOT
    UninstallSecureBoot();
#endif // ENABLE_SECURE_BOOT
    return Status;
  }

  //  DBG("DBG: messages\n");
  if (!gSettings.Boot.NoEarlyProgress && !GlobalConfig.isFastBoot()  && gSettings.Boot.Timeout>0) {
    XStringW Message = SWPrintf("   Welcome to Clover %ls   ", gFirmwareRevision);
    BootScreen.DrawTextXY(Message, (UGAWidth >> 1), UGAHeight >> 1, X_IS_CENTER);
    BootScreen.DrawTextXY(L"... testing hardware ..."_XSW, (UGAWidth >> 1), (UGAHeight >> 1) + 20, X_IS_CENTER);
  }

//  DumpBiosMemoryMap();

  GuiEventsInitialize();

  //DBG("ScanSPD() start\n");
  ScanSPD();
  //DBG("ScanSPD() end\n");

  SetPrivateVarProto();
//  GetDefaultSettings();
  GetAcpiTablesList();


  if (!gSettings.Boot.NoEarlyProgress && !GlobalConfig.isFastBoot() && gSettings.Boot.Timeout>0) {
    XStringW Message = SWPrintf("... user settings ...");
    BootScreen.EraseTextXY();
    BootScreen.DrawTextXY(Message, (UGAWidth >> 1), (UGAHeight >> 1) + 20, X_IS_CENTER);
  }

//  //Second step. Load config.plist into gSettings
//  for (i=0; i<2; i++) {
//    if (gConfigDict[i]) {
//      Status = GetUserSettings(gConfigDict[i], gSettings);
//      afterGetUserSettings(gSettings);
//      if (EFI_ERROR(Status)) {
//        DBG("Error in Second part of settings %llu: %s\n", i, efiStrError(Status));
//      }
//    }
//  }
  
    afterGetUserSettings(gSettings);

//  dropDSM = 0xFFFF; //by default we drop all OEM _DSM. They have no sense for us.
//  if (defDSM) {
//    dropDSM = gSettings.DropOEM_DSM;   //if set by user
//  }
  // Load any extra SMBIOS information
//  if (!EFI_ERROR(LoadUserSettings(L"smbios"_XSW, &smbiosTags)) && (smbiosTags != NULL)) {
//    const TagDict* dictPointer = smbiosTags->dictPropertyForKey("SMBIOS");
//    if (dictPointer) {
//      ParseSMBIOSSettings(gSettings, dictPointer);
//    } else {
//      DBG("Invalid smbios.plist, not overriding config.plist!\n");
//    }
//  }
/*
  if (gFirmwareClover || gDriversFlags.EmuVariableLoaded) {
    if (gSettings.Boot.StrictHibernate) {
      DBG(" Don't use StrictHibernate with emulated NVRAM!\n");
    }
    gSettings.Boot.StrictHibernate = FALSE;    
  }
*/
  HaveDefaultVolume = gSettings.Boot.DefaultVolume.notEmpty();
  if (!gFirmwareClover &&
      !gDriversFlags.EmuVariableLoaded &&
      !HaveDefaultVolume &&
      gSettings.Boot.Timeout == 0 && !ReadAllKeyStrokes()) {
// UEFI boot: get gEfiBootDeviceGuid from NVRAM.
// if present, ScanVolumes() will skip scanning other volumes
// in the first run.
// this speeds up loading of default macOS  volume.
     GetEfiBootDeviceFromNvram();
  }

  if (!gSettings.Boot.NoEarlyProgress && !GlobalConfig.isFastBoot() && gSettings.Boot.Timeout>0) {
    XStringW Message = SWPrintf("...  scan entries  ...");
    BootScreen.EraseTextXY();
    BootScreen.DrawTextXY(Message, (UGAWidth >> 1), (UGAHeight >> 1) + 20, X_IS_CENTER);
  }

  AfterTool = FALSE;
  gGuiIsReady = TRUE;
  GlobalConfig.gBootChanged = TRUE;
  GlobalConfig.gThemeChanged = TRUE;

  do {
    if (GlobalConfig.gBootChanged && GlobalConfig.gThemeChanged) { // config changed
      GetListOfDsdts(); //only after GetUserSettings
      GetListOfACPI(); //ssdt and other tables
    }
    GlobalConfig.gBootChanged = FALSE;
    MainMenu.Entries.setEmpty();
    OptionMenu.Entries.setEmpty();
    InitKextList();
    ScanVolumes();

    // as soon as we have Volumes, find latest nvram.plist and copy it to RT vars
    if (!AfterTool) {
      if (gFirmwareClover || gDriversFlags.EmuVariableLoaded) {
        PutNvramPlistToRtVars();
      }
    }
    
    // log Audio devices in boot-log. This is for clients like Clover.app
    GetOutputs();
    for (i = 0; i < AudioList.size(); i++) {
      if (AudioList[i].Name.notEmpty()) {
        // Never change this log, otherwise clients will stop interprete the output.
        MsgLog("Found Audio Device %ls (%s) at index %llu\n", AudioList[i].Name.wc_str(), AudioOutputNames[AudioList[i].Device], i);
      }
    }
    
    if (!GlobalConfig.isFastBoot()) {
//      CHAR16 *TmpArgs;
      if (gThemeNeedInit) {
        UINTN      Size         = 0;
        InitTheme((CHAR8*)GetNvramVariable(L"Clover.Theme", &gEfiAppleBootGuid, NULL, &Size));
        gThemeNeedInit = FALSE;
      } else if (GlobalConfig.gThemeChanged) {
        DBG("change theme\n");
        InitTheme(NULL);
        //OptionMenu.FreeMenu(); // it is already freed at loop beginning
        AboutMenu.Entries.setEmpty();
        HelpMenu.Entries.setEmpty();
      }
      DBG("theme inited\n");
      if (ThemeX.embedded) {
        DBG("Chosen embedded theme\n");
      } else {
        DBG("Chosen theme %ls\n", ThemeX.Theme.wc_str());
      }

//      DBG("initial boot-args=%s\n", gSettings.Boot.BootArgs);
      //now it is a time to set RtVariables
      SetVariablesFromNvram();
      
    XString8Array TmpArgs = Split<XString8Array>(gSettings.Boot.BootArgs, " ");
      DBG("after NVRAM boot-args=%s\n", gSettings.Boot.BootArgs.c_str());
      GlobalConfig.OptionsBits = EncodeOptions(TmpArgs);
//      DBG("initial OptionsBits %X\n", GlobalConfig.OptionsBits);
      FillInputs(TRUE);

      // scan for loaders and tools, add then to the menu
      if (gSettings.GUI.Scan.LegacyFirst){
        AddCustomLegacy();
        if (!gSettings.GUI.Scan.NoLegacy) {
          ScanLegacy();
        }
      }
    }
    GetSmcKeys(TRUE);
    
    // Add custom entries
    AddCustomEntries();
    if (gSettings.GUI.Scan.DisableEntryScan) {
      DBG("Entry scan disabled\n");
    } else {
      ScanLoader();
    }

    if (!GlobalConfig.isFastBoot()) {
      if (!gSettings.GUI.Scan.LegacyFirst) {
        AddCustomLegacy();
        if (!gSettings.GUI.Scan.NoLegacy) {
          ScanLegacy();
        }
      }

      // fixed other menu entries
      if (!(ThemeX.HideUIFlags & HIDEUI_FLAG_TOOLS)) {
        AddCustomTool();
        if (!gSettings.GUI.Scan.DisableToolScan) {
          ScanTool();
#ifdef ENABLE_SECURE_BOOT
          // Check for secure boot setup mode
          AddSecureBootTool();
#endif // ENABLE_SECURE_BOOT
        }
      }

      MenuEntryOptions.Image = ThemeX.GetIcon(BUILTIN_ICON_FUNC_OPTIONS);
//      DBG("Options: IconID=%lld name=%s empty=%s\n", MenuEntryOptions.Image.Id, MenuEntryOptions.Image.Name.c_str(),
//          MenuEntryOptions.Image.isEmpty()?"пусто":"нет");
      if (gSettings.Boot.DisableCloverHotkeys)
        MenuEntryOptions.ShortcutLetter = 0x00;
      MainMenu.AddMenuEntry(&MenuEntryOptions, false);
      
      MenuEntryAbout.Image = ThemeX.GetIcon((INTN)BUILTIN_ICON_FUNC_ABOUT);
//      DBG("About: IconID=%lld name=%s empty=%s\n", MenuEntryAbout.Image.Id, MenuEntryAbout.Image.Name.c_str(),
//          MenuEntryAbout.Image.isEmpty()?"пусто":"нет");
      if (gSettings.Boot.DisableCloverHotkeys)
        MenuEntryAbout.ShortcutLetter = 0x00;
      MainMenu.AddMenuEntry(&MenuEntryAbout, false);

      if (!(ThemeX.HideUIFlags & HIDEUI_FLAG_FUNCS) || MainMenu.Entries.size() == 0) {
        if (gSettings.Boot.DisableCloverHotkeys)
          MenuEntryReset.ShortcutLetter = 0x00;
        MenuEntryReset.Image = ThemeX.GetIcon(BUILTIN_ICON_FUNC_RESET);
        MainMenu.AddMenuEntry(&MenuEntryReset, false);
        if (gSettings.Boot.DisableCloverHotkeys)
          MenuEntryShutdown.ShortcutLetter = 0x00;
        MenuEntryShutdown.Image = ThemeX.GetIcon(BUILTIN_ICON_FUNC_EXIT);
        MainMenu.AddMenuEntry(&MenuEntryShutdown, false);
      }

// font already changed and this message very quirky, clear line here
//     if (!gSettings.Boot.NoEarlyProgress && !GlobalConfig.isFastBoot() && gSettings.Boot.Timeout>0) {
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
//    DBG("DefaultIndex=%lld and MainMenu.Entries.size()=%llu\n", DefaultIndex, MainMenu.Entries.size());
    if ((DefaultIndex >= 0) && (DefaultIndex < (INTN)MainMenu.Entries.size())) {
      DefaultEntry = &MainMenu.Entries[DefaultIndex];
    } else {
      DefaultEntry = NULL;
    }

    MainLoopRunning = TRUE;
    //    MainMenu.TimeoutSeconds = gSettings.Boot.Timeout >= 0 ? gSettings.Boot.Timeout : 0;
    if (DefaultEntry && (GlobalConfig.isFastBoot() ||
                         (gSettings.Boot.SkipHibernateTimeout &&
                           DefaultEntry->getLOADER_ENTRY()
                           && OSFLAG_ISSET(DefaultEntry->getLOADER_ENTRY()->Flags, OSFLAG_HIBERNATED)
                         )
                        )
        )
    {
      if (DefaultEntry->getLOADER_ENTRY()) {
        DefaultEntry->StartLoader();
      } else if (DefaultEntry->getLEGACY_ENTRY()){
        DefaultEntry->StartLegacy();
      }
      gSettings.Boot.FastBoot = FALSE; //Hmm... will never be here
    }
//    BOOLEAN MainAnime = MainMenu.GetAnime();
//    DBG("MainAnime=%d\n", MainAnime);
    AfterTool = FALSE;
    gEvent = 0; //clear to cancel loop
    while (MainLoopRunning) {
 //     CHAR8 *LastChosenOS = NULL;
      if (gSettings.Boot.Timeout == 0 && DefaultEntry != NULL && !ReadAllKeyStrokes()) {
        // go strait to DefaultVolume loading
        MenuExit = MENU_EXIT_TIMEOUT;
      } else {
        MainMenu.GetAnime();
        if (GlobalConfig.gThemeChanged) {
          GlobalConfig.gThemeChanged = FALSE;
          ThemeX.ClearScreen();
        }
        MenuExit = MainMenu.RunMainMenu(DefaultIndex, &ChosenEntry);
      }
//    DBG("exit from MainMenu %llu\n", MenuExit); //MENU_EXIT_ENTER=(1) MENU_EXIT_DETAILS=3
      // disable default boot - have sense only in the first run
      gSettings.Boot.Timeout = -1;
      if ((DefaultEntry != NULL) && (MenuExit == MENU_EXIT_TIMEOUT)) {
        if (DefaultEntry->getLOADER_ENTRY()) {
          DefaultEntry->StartLoader();
        } else if (DefaultEntry->getLEGACY_ENTRY()){
          DefaultEntry->StartLegacy();
        }
        // if something goes wrong - break main loop to reinit volumes
        break;
      }

      if (MenuExit == MENU_EXIT_OPTIONS){
        GlobalConfig.gBootChanged = FALSE;
        OptionsMenu(&OptionEntry);
        if (GlobalConfig.gBootChanged) {
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
          MainMenu.Entries.includeHidden = !MainMenu.Entries.includeHidden;
          continue;
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
        GlobalConfig.gBootChanged = FALSE;
        OptionsMenu(&OptionEntry);
        if (GlobalConfig.gBootChanged)
          AfterTool = TRUE;
        if (GlobalConfig.gBootChanged || GlobalConfig.gThemeChanged) // If theme has changed reinit the desktop
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
        ChosenEntry->StartLoader();
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
          ChosenEntry->StartLegacy();
        }
      }

      if ( ChosenEntry->getREFIT_MENU_ENTRY_LOADER_TOOL() ) {     // Start a EFI tool
        ChosenEntry->StartTool();
        TerminateScreen(); //does not happen
        //   return EFI_SUCCESS;
        //  BdsLibConnectAllDriversToAllControllers();
        //    PauseForKey(L"Returned from StartTool\n");
        MainLoopRunning = FALSE;
        AfterTool = TRUE;
      }

  #ifdef ENABLE_SECURE_BOOT
panic("not done yet");
//      if ( ChosenEntry->getREFIT_MENU_ENTRY_SECURE_BOOT() ) { // Try to enable secure boot
//            EnableSecureBoot();
//            MainLoopRunning = FALSE;
//            AfterTool = TRUE;
//      }
//
//      if ( ChosenEntry->getREFIT_MENU_ENTRY_SECURE_BOOT_CONFIG() ) { // Configure secure boot
//            MainLoopRunning = !ConfigureSecureBoot();
//            AfterTool = TRUE;
//      }
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
            XStringW Description;
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
              XStringW& VolName = Entry->Volume->VolName;
              if (VolName.isEmpty()) {
                VolName = NullXStringW;
              }
              NameSize = VolName.sizeInBytes();
              Name2Size = 0;
              if (Entry->LoaderPath.notEmpty()) {
                LoaderName = Basename(Entry->LoaderPath.wc_str());
              } else {
                LoaderName = NULL;  //legacy boot
              }
              if (LoaderName != NULL) {
                Name2Size = StrSize(LoaderName);
              }

              Description = SWPrintf("Clover start %ls at %ls", (LoaderName != NULL)?LoaderName:L"legacy", VolName.wc_str());
              OptionalDataSize = NameSize + Name2Size + 4 + 2; //signature + VolNameSize
              OptionalData = (__typeof__(OptionalData))AllocateZeroPool(OptionalDataSize);
              if (OptionalData == NULL) {
                break;
              }
              CopyMem(OptionalData, "Clvr", 4); //signature = 0x72766c43
              CopyMem(OptionalData + 4, &NameSize, 2);
              CopyMem(OptionalData + 6, VolName.wc_str(), VolName.sizeInBytes());
              if (Name2Size != 0) {
                CopyMem(OptionalData + 6 + NameSize, LoaderName, Name2Size);
              }

              Status = AddBootOptionForFile (
                                    LoaderEntry->Volume->DeviceHandle,
                                    LoaderEntry->LoaderPath,
                                    TRUE,
                                    Description.wc_str(),
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
      DBG("ReinitRefitLib after theme change\n");
      ReinitRefitLib();
    }
    //    PauseForKey(L"After ReinitRefitLib");
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

  UninitializeConsoleSim ();

#ifdef CLOVER_BUILD
  destruct_globals_objects(NULL);
#endif

  return EFI_SUCCESS;
}


