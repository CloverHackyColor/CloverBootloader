/*
 *
 */
#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
extern "C" {
#include <Library/OcDeviceTreeLib.h>
}
#include "kext_inject.h"
#include "DataHubCpu.h"
#include "../Platform/plist/plist.h"
#include "../Platform/Settings.h"
#include "../Platform/guid.h"
#include "MemoryOperation.h"

#ifndef DEBUG_ALL
#define KEXT_INJECT_DEBUG 1
#else
#define KEXT_INJECT_DEBUG DEBUG_ALL
#endif

#if KEXT_INJECT_DEBUG == 2
#define DBG(...)    printf(__VA_ARGS__);
#elif KEXT_INJECT_DEBUG == 1
#define DBG(...)    DebugLog(KEXT_INJECT_DEBUG, __VA_ARGS__)
#else
#define DBG(...)
#endif

// runtime debug
//
#define OLD_EXTRA_KEXT_PATCH 0

////////////////////
// globals
////////////////////
LIST_ENTRY gKextList = INITIALIZE_LIST_HEAD_VARIABLE (gKextList);


////////////////////
// before booting
////////////////////
EFI_STATUS EFIAPI ThinFatFile(IN OUT UINT8 **binary, IN OUT UINTN *length, IN cpu_type_t archCpuType)
{
  UINT32 nfat, swapped, size = 0;
  FAT_HEADER *fhp = (FAT_HEADER *)*binary;
  FAT_ARCH   *fap = (FAT_ARCH *)(*binary + sizeof(FAT_HEADER));
  cpu_type_t fapcputype;
  UINT32 fapoffset;
  UINT32 fapsize;

  swapped = 0;
  if (fhp->magic == FAT_MAGIC) {
    nfat = fhp->nfat_arch;
  } else if (fhp->magic == FAT_CIGAM) {
    nfat = SwapBytes32(fhp->nfat_arch);
    swapped = 1;
    //already thin
  } else if (fhp->magic == THIN_X64){
    if (archCpuType == CPU_TYPE_X86_64) {
      return EFI_SUCCESS;
    }
    return EFI_NOT_FOUND;
  } else if (fhp->magic == THIN_IA32){
    if (archCpuType == CPU_TYPE_I386) {
      return EFI_SUCCESS;
    }
    return EFI_NOT_FOUND;
  } else {
    MsgLog("Thinning fails\n");
    return EFI_NOT_FOUND;
  }

  for (; nfat > 0; nfat--, fap++) {
    if (swapped) {
      fapcputype = SwapBytes32(fap->cputype);
      fapoffset = SwapBytes32(fap->offset);
      fapsize = SwapBytes32(fap->size);
    } else {
      fapcputype = fap->cputype;
      fapoffset = fap->offset;
      fapsize = fap->size;
    }
    if (fapcputype == archCpuType) {
      *binary = (*binary + fapoffset);
      size = fapsize;
      break;
    }
  }
  if (length != 0) *length = size;

  return EFI_SUCCESS;
}

void toLowerStr(CHAR8 *tstr, IN CONST CHAR8 *str) {
    UINT16 cnt = 0;
    
    for (cnt = 0; *str != '\0' && cnt <= 0xFF; cnt++, str++, tstr++) {
        if (*str >= 'A' && *str <= 'Z')
            *tstr = 'a' + (*str - 'A');
        else
            *tstr = *str;
    }
    *tstr = '\0';
}

BOOLEAN checkOSBundleRequired(UINT8 loaderType, const TagDict* dict)
{
    BOOLEAN inject = TRUE;
    const TagStruct*  osBundleRequiredTag;
    XString8 osbundlerequired;
    
    osBundleRequiredTag = dict->propertyForKey("OSBundleRequired");
    if (osBundleRequiredTag) {
      osbundlerequired = osBundleRequiredTag->getString()->stringValue();
      osbundlerequired.lowerAscii();
    }

    if (OSTYPE_IS_OSX_RECOVERY(loaderType) ||
        OSTYPE_IS_OSX_INSTALLER(loaderType)) {
        if ( osbundlerequired != "root"_XS8  &&
             osbundlerequired != "local"_XS8  &&
             osbundlerequired != "console"_XS8  &&
             osbundlerequired != "network-root"_XS8 ) {
            inject = FALSE;
        }
    }
    
    return inject;
}

//extern void KernelAndKextPatcherInit(IN LOADER_ENTRY *Entry);
//extern void AnyKextPatch(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize, INT32 N, LOADER_ENTRY *Entry);

EFI_STATUS LOADER_ENTRY::LoadKext(const EFI_FILE *RootDir, const XString8& FileName, IN cpu_type_t archCpuType, IN OUT void *kext_v)
{
  EFI_STATUS  Status;
  UINT8*      infoDictBuffer = NULL;
  UINTN       infoDictBufferLength = 0;
  UINT8*      executableFatBuffer = NULL;
  UINT8*      executableBuffer = NULL;
  UINTN       executableBufferLength = 0;
//  CHAR8*      bundlePathBuffer = NULL;
//  UINTN       bundlePathBufferLength = 0;
  XStringW    TempName;
  TagDict*    dict = NULL;
  const TagStruct*      prop = NULL;
  BOOLEAN     NoContents = FALSE;
  BOOLEAN     inject = FALSE;
  _BooterKextFileInfo *infoAddr = NULL;
  _DeviceTreeBuffer *kext = (_DeviceTreeBuffer *)kext_v;

  TempName = SWPrintf("%s\\%ls", FileName.c_str(), L"Contents\\Info.plist");
  Status = egLoadFile(RootDir, TempName.wc_str(), &infoDictBuffer, &infoDictBufferLength);
  if (EFI_ERROR(Status)) {
    //try to find a planar kext, without Contents
    TempName = SWPrintf("%s\\%ls", FileName.c_str(), L"Info.plist");
    infoDictBufferLength = 0;
    Status = egLoadFile(RootDir, TempName.wc_str(), &infoDictBuffer, &infoDictBufferLength);
    if (EFI_ERROR(Status)) {
      MsgLog("Failed to load extra kext : %ls status=%s\n", TempName.wc_str(), efiStrError(Status));
      return EFI_NOT_FOUND;
    }
    NoContents = TRUE;
  }
  if( ParseXML((CHAR8*)infoDictBuffer, &dict,infoDictBufferLength)!=0 ) {
    FreePool(infoDictBuffer);
    MsgLog("Failed to load extra kext (failed to parse Info.plist): %s\n", FileName.c_str());
    return EFI_NOT_FOUND;
  }
    
  inject = checkOSBundleRequired(LoaderType, dict);
  if(!inject) {
      MsgLog("Skipping kext injection by OSBundleRequired : %s\n", FileName.c_str());
      return EFI_UNSUPPORTED;
  }
    
  prop = dict->propertyForKey("CFBundleExecutable");
  if( prop != NULL && prop->isString() && prop->getString()->stringValue().notEmpty() ) {
    XString8 Executable = prop->getString()->stringValue();
    if (NoContents) {
      TempName = SWPrintf("%s\\%s", FileName.c_str(), Executable.c_str());
      //     snwprintf(TempName, 512, "%s\\%s", FileName, Executable);
    } else {
      TempName = SWPrintf("%s\\Contents\\MacOS\\%s", FileName.c_str(), Executable.c_str());
      //    snwprintf(TempName, 512, L"%s\\%s\\%s", FileName, "Contents\\MacOS",Executable);
    }
    Status = egLoadFile(RootDir, TempName.wc_str(), &executableFatBuffer, &executableBufferLength);
    if (EFI_ERROR(Status)) {
      FreePool(infoDictBuffer);
      MsgLog("Failed to load extra kext (executable not found): %s\n", FileName.c_str());
      return EFI_NOT_FOUND;
    }
    executableBuffer = executableFatBuffer;
    if (ThinFatFile(&executableBuffer, &executableBufferLength, archCpuType)) {
      FreePool(infoDictBuffer);
      FreePool(executableBuffer);
      MsgLog("Thinning failed: %s\n", FileName.c_str());
      return EFI_NOT_FOUND;
    }
  }
//  bundlePathBufferLength = StrLen(FileName) + 1;
//  bundlePathBuffer = (__typeof__(bundlePathBuffer))AllocateZeroPool(bundlePathBufferLength);
//  UnicodeStrToAsciiStrS(FileName, bundlePathBuffer, bundlePathBufferLength);

  kext->length = (UINT32)(sizeof(_BooterKextFileInfo) + infoDictBufferLength + executableBufferLength + FileName.sizeInBytesIncludingTerminator());
  infoAddr = (_BooterKextFileInfo *)AllocatePool(kext->length);
  infoAddr->infoDictPhysAddr = sizeof(_BooterKextFileInfo);
  infoAddr->infoDictLength = (UINT32)infoDictBufferLength;
  infoAddr->executablePhysAddr = (UINT32)(sizeof(_BooterKextFileInfo) + infoDictBufferLength);
  infoAddr->executableLength = (UINT32)executableBufferLength;
  infoAddr->bundlePathPhysAddr = (UINT32)(sizeof(_BooterKextFileInfo) + infoDictBufferLength + executableBufferLength);
  infoAddr->bundlePathLength = (UINT32)FileName.sizeInBytesIncludingTerminator();
  kext->paddr = (UINT32)(UINTN)infoAddr; // Note that we cannot free infoAddr because of this
  CopyMem((CHAR8 *)infoAddr + sizeof(_BooterKextFileInfo), infoDictBuffer, infoDictBufferLength);
  CopyMem((CHAR8 *)infoAddr + sizeof(_BooterKextFileInfo) + infoDictBufferLength, executableBuffer, executableBufferLength);
  CopyMem((CHAR8 *)infoAddr + sizeof(_BooterKextFileInfo) + infoDictBufferLength + executableBufferLength, FileName.c_str(), FileName.sizeInBytesIncludingTerminator());
  FreePool(infoDictBuffer);
  FreePool(executableFatBuffer);
  dict->FreeTag();

  return EFI_SUCCESS;
}

EFI_STATUS LOADER_ENTRY::AddKext(const EFI_FILE *RootDir, const XString8& FileName, IN cpu_type_t archCpuType)
{
  EFI_STATUS  Status;
  KEXT_ENTRY  *KextEntry;

  KextEntry = (__typeof__(KextEntry))AllocatePool (sizeof(KEXT_ENTRY));
  KextEntry->Signature = KEXT_SIGNATURE;
  Status = LoadKext(RootDir, FileName, archCpuType, &KextEntry->kext);
  if(EFI_ERROR(Status)) {
    FreePool(KextEntry);
  } else {
    InsertTailList (&gKextList, &KextEntry->Link);
  }

  return Status;
}

UINT32 GetListCount(LIST_ENTRY const* List)
{
  LIST_ENTRY    *Link;
  UINT32      Count=0;

  if(!IsListEmpty(List)) {
    for (Link = List->ForwardLink; Link != List; Link = Link->ForwardLink)
      Count++;
  }

  return Count;
}

UINT32 GetKextCount()
{
  return (UINT32)GetListCount(&gKextList);
}

UINT32 GetKextsSize()
{
  LIST_ENTRY    *Link;
  KEXT_ENTRY    *KextEntry;
  UINT32        kextsSize=0;

  if(!IsListEmpty(&gKextList)) {
    for (Link = gKextList.ForwardLink; Link != &gKextList; Link = Link->ForwardLink) {
      KextEntry = CR(Link, KEXT_ENTRY, Link, KEXT_SIGNATURE);
      kextsSize += RoundPage(KextEntry->kext.length);
    }
  }
  return kextsSize;
}

void LOADER_ENTRY::LoadPlugInKexts(const EFI_FILE *RootDir, const XString8& DirName, IN cpu_type_t archCpuType, IN BOOLEAN Force)
{
   REFIT_DIR_ITER          PlugInIter;
   EFI_FILE_INFO           *PlugInFile;
   XString8                FileName;
   if ( RootDir == NULL || DirName.isEmpty() ) {
      return;
   }
   DirIterOpen(RootDir, XStringW(DirName).wc_str(), &PlugInIter);
   while (DirIterNext(&PlugInIter, 1, L"*.kext", &PlugInFile)) {
     if (PlugInFile->FileName[0] == '.' || StrStr(PlugInFile->FileName, L".kext") == NULL)
       continue;   // skip this
     FileName = SWPrintf("%s\\%ls", DirName.c_str(), PlugInFile->FileName);
     //     snwprintf(FileName, 512, "%s\\%s", DirName, PlugInFile->FileName);
     MsgLog("    %ls PlugIn kext: %s\n", Force ? L"Force" : L"Extra", FileName.c_str());
     AddKext( RootDir, FileName, archCpuType);
   }
   DirIterClose(&PlugInIter);
}

//void LOADER_ENTRY::AddKexts(const XStringW& SrcDir, const XStringW& Path, cpu_type_t archCpuType)
//{
//  XStringW                 FileName;
//  XStringW                 PlugInName;
//  SIDELOAD_KEXT           *CurrentKext;
//  SIDELOAD_KEXT           *CurrentPlugInKext;
//  EFI_STATUS              Status;
//
//  MsgLog("Preparing kexts injection from %ls\n", SrcDir.wc_str());
//  CurrentKext = InjectKextList;
//  while (CurrentKext) {
////    DBG("  current kext name=%ls path=%ls, match against=%ls\n", CurrentKext->FileName, CurrentKext->KextDirNameUnderOEMPath, Path);
//    if ( CurrentKext->KextDirNameUnderOEMPath == Path ) {
//      FileName = SWPrintf("%ls\\%ls", SrcDir.wc_str(), CurrentKext->FileName.wc_str());
//      //   snwprintf(FileName, 512, "%s\\%s", SrcDir, CurrentKext->FileName);
//      if (!(CurrentKext->MenuItem.BValue)) {
//        // inject require
//        MsgLog("->Extra kext: %ls (v.%ls)\n", FileName.wc_str(), CurrentKext->Version.wc_str());
//        Status = AddKext(SelfVolume->RootDir, FileName.wc_str(), archCpuType);
//        if(!EFI_ERROR(Status)) {
//          // decide which plugins to inject
//          CurrentPlugInKext = CurrentKext->PlugInList;
//          while (CurrentPlugInKext) {
//            PlugInName = SWPrintf("%ls\\%ls\\%ls", FileName.wc_str(), L"Contents\\PlugIns", CurrentPlugInKext->FileName.wc_str());
//            //     snwprintf(PlugInName, 512, L"%s\\%s\\%s", FileName, "Contents\\PlugIns", CurrentPlugInKext->FileName);
//            if (!(CurrentPlugInKext->MenuItem.BValue)) {
//              // inject PlugIn require
//              MsgLog("    |-- PlugIn kext: %ls (v.%ls)\n", PlugInName.wc_str(), CurrentPlugInKext->Version.wc_str());
//              AddKext(SelfVolume->RootDir, PlugInName.wc_str(), archCpuType);
//            } else {
//              MsgLog("    |-- Disabled plug-in kext: %ls (v.%ls)\n", PlugInName.wc_str(), CurrentPlugInKext->Version.wc_str());
//            }
//            CurrentPlugInKext = CurrentPlugInKext->Next;
//          } // end of plug-in kext injection
//        }
//      } else {
//        // disable current kext injection
//        if ( SrcDir.containsIC(L"Off") ) {
//          MsgLog("Disabled kext: %ls (v.%ls)\n", FileName.wc_str(), CurrentKext->Version.wc_str());
//        }
//      }
//    }
//    CurrentKext = CurrentKext->Next;
//  } // end of kext injection
//
//}

// Jief : this should replace LOADER_ENTRY::AddKexts
void LOADER_ENTRY::AddKextsFromDirInArray(const XString8& SrcDir, const XString8& Path, cpu_type_t archCpuType, XObjArray<SIDELOAD_KEXT>* kextArray)
{
  XStringW                 FileName;
  XStringW                 PlugInName;

  MsgLog("AddKextsInArray from %s\n", SrcDir.c_str());
  for ( size_t idx = 0 ; idx < InjectKextList.size() ; idx ++ ) {
    SIDELOAD_KEXT& CurrentKext = InjectKextList[idx];
//    DBG("  current kext name=%ls path=%ls, match against=%s\n", CurrentKext.FileName.wc_str(), CurrentKext.KextDirNameUnderOEMPath.wc_str(), Path.c_str());
    if ( CurrentKext.KextDirNameUnderOEMPath == Path ) {
      FileName = SWPrintf("%s\\%ls", SrcDir.c_str(), CurrentKext.FileName.wc_str());
      if (!(CurrentKext.MenuItem.BValue)) {
        // inject require
        MsgLog("->Extra kext: %ls (v.%ls)\n", FileName.wc_str(), CurrentKext.Version.wc_str());
//        Status = AddKext(SelfVolume->RootDir, FileName.wc_str(), archCpuType);
        kextArray->AddReference(&CurrentKext, false); // do not free, CurrentKext belongs to an other object
        // decide which plugins to inject
        for ( size_t idxPlugin = 0 ; idxPlugin < CurrentKext.PlugInList.size() ; idxPlugin ++ ) {
          SIDELOAD_KEXT& CurrentPlugInKext = CurrentKext.PlugInList[idxPlugin];
          PlugInName = SWPrintf("%ls\\Contents\\PlugIns\\%ls", FileName.wc_str(), CurrentPlugInKext.FileName.wc_str());
          //     snwprintf(PlugInName, 512, L"%s\\%s\\%s", FileName, "Contents\\PlugIns", CurrentPlugInKext.FileName);
          if (!(CurrentPlugInKext.MenuItem.BValue)) {
            // inject PlugIn require
            MsgLog("    |-- PlugIn kext: %ls (v.%ls)\n", PlugInName.wc_str(), CurrentPlugInKext.Version.wc_str());
//              AddKext(SelfVolume->RootDir, PlugInName.wc_str(), archCpuType);
            kextArray->AddReference(&CurrentPlugInKext, false); // do not free, CurrentKext belongs to an other object
          } else {
            MsgLog("    |-- Disabled plug-in kext: %ls (v.%ls)\n", PlugInName.wc_str(), CurrentPlugInKext.Version.wc_str());
          }
        } // end of plug-in kext injection
      } else {
        // disable current kext injection
        if ( SrcDir.containsIC(L"Off") ) {
          MsgLog("Disabled kext: %ls (v.%ls)\n", FileName.wc_str(), CurrentKext.Version.wc_str());
        }
      }
    }
  } // end of kext injection

}

void LOADER_ENTRY::AddKextsInArray(XObjArray<SIDELOAD_KEXT>* kextArray)
{
  XStringW                SrcDir;
  REFIT_DIR_ITER          PlugInIter;
  EFI_FILE_INFO          *PlugInFile;
  XString8                FileName;
  XString8                PlugIns;
//  CONST CHAR16                  *Arch = NULL;
//  CONST CHAR16                  *Ptr = NULL;
#if defined(MDE_CPU_X64)
  cpu_type_t              archCpuType=CPU_TYPE_X86_64;
#else
  cpu_type_t              archCpuType=CPU_TYPE_I386;
#endif


  // Make Arch point to the last appearance of "arch=" in LoadOptions (which is what boot.efi will use).
//  if (LoadOptions.notEmpty()) {
//    for (Ptr = StrStr(LoadOptions, L"arch="); Ptr != NULL; Arch = Ptr + StrLen(L"arch="), Ptr = StrStr(Arch, L"arch="));
//  }

//  if (Arch != NULL && StrnCmp(Arch,L"x86_64",StrLen(L"x86_64")) == 0) {
  if (LoadOptions.contains("arch=x86_64")) {
    archCpuType = CPU_TYPE_X86_64;
//  } else if (Arch != NULL && StrnCmp(Arch,L"i386",StrLen(L"i386")) == 0) {
  } else if (LoadOptions.contains("arch=i386")) {
    archCpuType = CPU_TYPE_I386;
  } else if (OSVersion.notEmpty()) {
    UINT64 os_version = AsciiOSVersionToUint64(OSVersion);
    if (os_version >= AsciiOSVersionToUint64("10.8"_XS8)) {
      archCpuType = CPU_TYPE_X86_64; // For OSVersion >= 10.8, only x86_64 exists
    } else if (os_version < AsciiOSVersionToUint64("10.7"_XS8)) {
      archCpuType = CPU_TYPE_I386; // For OSVersion < 10.7, use default of i386
    }
  }

  // Force kexts to load
  if ( KernelAndKextPatches.ForceKexts.notEmpty() ) {
    for (size_t i = 0; i < KernelAndKextPatches.ForceKexts.size(); ++i) {
      MsgLog("  Force kext: %ls\n", KernelAndKextPatches.ForceKexts[i].wc_str());
      if (Volume && Volume->RootDir) {
        // Check if the entry is a directory
        if (StrStr(KernelAndKextPatches.ForceKexts[i].wc_str(), L".kext") == NULL) {
          DirIterOpen(Volume->RootDir, KernelAndKextPatches.ForceKexts[i].wc_str(), &PlugInIter);
          while (DirIterNext(&PlugInIter, 1, L"*.kext", &PlugInFile)) {
            if (PlugInFile->FileName[0] == '.' || StrStr(PlugInFile->FileName, L".kext") == NULL)
              continue;   // skip this
            FileName = SWPrintf("%ls\\%ls", KernelAndKextPatches.ForceKexts[i].wc_str(), PlugInFile->FileName);
            //    snwprintf(FileName, 512, "%s\\%s", KernelAndKextPatches.ForceKexts[i], PlugInFile->FileName);
            MsgLog("  Force kext: %s\n", FileName.c_str());
            AddKext( Volume->RootDir, FileName, archCpuType);
            PlugIns = SWPrintf("%s\\Contents\\PlugIns", FileName.c_str());
            //  snwprintf(PlugIns, 512, "%s\\Contents\\PlugIns", FileName);
            LoadPlugInKexts(Volume->RootDir, PlugIns, archCpuType, TRUE);
          }
          DirIterClose(&PlugInIter);
        } else {
          AddKext( Volume->RootDir, KernelAndKextPatches.ForceKexts[i], archCpuType);
          PlugIns = SWPrintf("%ls\\Contents\\PlugIns", KernelAndKextPatches.ForceKexts[i].wc_str());
          //  snwprintf(PlugIns, 512, "%s\\Contents\\PlugIns", KernelAndKextPatches.ForceKexts[i]);
          LoadPlugInKexts(Volume->RootDir, PlugIns, archCpuType, TRUE);
        }
      }
    }
  }

  XStringW UniOSVersion;
  UniOSVersion = OSVersion;
  DBG("UniOSVersion == %ls\n", UniOSVersion.wc_str());

  XStringW UniShortOSVersion;
//  XString8 ShortOSVersion;
  if (AsciiOSVersionToUint64(OSVersion) < AsciiOSVersionToUint64("10.10"_XS8)) {
    // OSVersion that are earlier than 10.10(form: 10.x.y)
//    ShortOSVersion.strncpy(OSVersion.c_str(), 4);
    UniShortOSVersion.strncpy(OSVersion.c_str(), 4);
  } else {
//    ShortOSVersion.strncpy(OSVersion.c_str(), 5);
    UniShortOSVersion.strncpy(OSVersion.c_str(), 5);
  }
//  DBG("ShortOSVersion == %s\n", ShortOSVersion.c_str());
  DBG("UniShortOSVersion == %ls\n", UniShortOSVersion.wc_str());

  // syscl - allow specific load inject kext
  // Clover/Kexts/Other is for general injection thus we need to scan both Other and OSVersion folder
  SrcDir = GetOtherKextsDir(TRUE);
  if ( SrcDir.notEmpty() ) {
    AddKextsFromDirInArray(SrcDir, "Other"_XS8, archCpuType, kextArray);
  } else {
    DBG("GetOtherKextsDir(TRUE) return NULL\n");
  }
    // slice: CLOVER/kexts/Off keep disabled kext which can be allowed
  SrcDir = GetOtherKextsDir(FALSE);
  if ( SrcDir.notEmpty() ) {
    AddKextsFromDirInArray(SrcDir, "Off"_XS8, archCpuType, kextArray);
  } else {
    DBG("GetOtherKextsDir(FALSE) return NULL\n");
  }

  if ( OSVersion.contains(".") )
  {
  // Add kext from 10 or 11
    XStringW osMajorVersion = OSVersion.subString(0, OSVersion.indexOf('.'));

    XStringW OSAllVersionKextsDir;
    XStringW OSShortVersionKextsDir;
    XStringW OSVersionKextsDirName;
    XStringW DirName;
    XStringW DirPath;

    OSAllVersionKextsDir = SWPrintf("%ls", osMajorVersion.wc_str());
    AddKextsFromDirInArray(OSAllVersionKextsDir, osMajorVersion, archCpuType, kextArray);

    if (OSTYPE_IS_OSX_INSTALLER(LoaderType)) {
      DirName = SWPrintf("%ls_install", osMajorVersion.wc_str());
    } else if (OSTYPE_IS_OSX_RECOVERY(LoaderType)) {
      DirName = SWPrintf("%ls_recovery", osMajorVersion.wc_str());
    } else {
      DirName = SWPrintf("%ls_normal", osMajorVersion.wc_str());
    }
    DirPath = SWPrintf("%ls", DirName.wc_str());
    AddKextsFromDirInArray(DirPath, DirName, archCpuType, kextArray);


    // Add kext from ${osMajorVersion}.{version}

    OSShortVersionKextsDir = SWPrintf("%ls", UniShortOSVersion.wc_str());
    AddKextsFromDirInArray( OSShortVersionKextsDir, UniShortOSVersion, archCpuType, kextArray);

    if (OSTYPE_IS_OSX_INSTALLER(LoaderType)) {
      DirName = SWPrintf("%ls_install", UniShortOSVersion.wc_str());
    } else if (OSTYPE_IS_OSX_RECOVERY(LoaderType)) {
      DirName = SWPrintf("%ls_recovery", UniShortOSVersion.wc_str());
    } else {
      DirName = SWPrintf("%ls_normal", UniShortOSVersion.wc_str());
    }
    DirPath = SWPrintf("%ls", DirName.wc_str());
    AddKextsFromDirInArray(DirPath, DirName, archCpuType, kextArray);


    // Add kext from :
    // ${osMajorVersion}.{version}.0 if NO minor version
    // ${osMajorVersion}.{version}.{minor version} if minor version is > 0

    if ( UniShortOSVersion == OSVersion ) {
      OSVersionKextsDirName = SWPrintf("%s.0", OSVersion.c_str());
    } else {
      OSVersionKextsDirName = SWPrintf("%s", OSVersion.c_str());
    }

    DirPath = SWPrintf("%ls", OSVersionKextsDirName.wc_str());
    AddKextsFromDirInArray(DirPath, OSVersionKextsDirName, archCpuType, kextArray);

    if ( OSTYPE_IS_OSX_INSTALLER(LoaderType)) {
      DirName = SWPrintf("%ls_install", OSVersionKextsDirName.wc_str());
    } else if (OSTYPE_IS_OSX_RECOVERY(LoaderType)) {
      DirName = SWPrintf("%ls_recovery", OSVersionKextsDirName.wc_str());
    } else {
      DirName = SWPrintf("%ls_normal", OSVersionKextsDirName.wc_str());
    }
    DirPath = SWPrintf("%ls", DirName.wc_str());
    AddKextsFromDirInArray(DirPath, DirName, archCpuType, kextArray);
  }else{
    //MsgLog("No os version is detected\n");
  }

}

EFI_STATUS LOADER_ENTRY::LoadKexts()
{
  XObjArray<SIDELOAD_KEXT> kextArray;
  AddKextsInArray(&kextArray);


  #if defined(MDE_CPU_X64)
    cpu_type_t              archCpuType=CPU_TYPE_X86_64;
  #else
    cpu_type_t              archCpuType=CPU_TYPE_I386;
  #endif
    // Make Arch point to the last appearance of "arch=" in LoadOptions (which is what boot.efi will use).
  //  if (LoadOptions.notEmpty()) {
  //    for (Ptr = StrStr(LoadOptions, L"arch="); Ptr != NULL; Arch = Ptr + StrLen(L"arch="), Ptr = StrStr(Arch, L"arch="));
  //  }

  //  if (Arch != NULL && StrnCmp(Arch,L"x86_64",StrLen(L"x86_64")) == 0) {
    if (LoadOptions.contains("arch=x86_64")) {
      archCpuType = CPU_TYPE_X86_64;
  //  } else if (Arch != NULL && StrnCmp(Arch,L"i386",StrLen(L"i386")) == 0) {
    } else if (LoadOptions.contains("arch=i386")) {
      archCpuType = CPU_TYPE_I386;
    } else if (OSVersion.notEmpty()) {
      UINT64 os_version = AsciiOSVersionToUint64(OSVersion);
      if (os_version >= AsciiOSVersionToUint64("10.8"_XS8)) {
        archCpuType = CPU_TYPE_X86_64; // For OSVersion >= 10.8, only x86_64 exists
      } else if (os_version < AsciiOSVersionToUint64("10.7"_XS8)) {
        archCpuType = CPU_TYPE_I386; // For OSVersion < 10.7, use default of i386
      }
    }

  for (size_t idx = 0 ; idx < kextArray.size()  ; idx++ ) {
    AddKext(SelfVolume->RootDir, S8Printf("%ls\\%ls", kextArray[idx].KextDirNameUnderOEMPath.wc_str(), kextArray[idx].FileName.wc_str()), archCpuType);
  }

  UINTN                    mm_extra_size;
  void                    *mm_extra;
  UINTN                    extra_size;
  void                    *extra;

  // reserve space in the device tree
  if (GetKextCount() > 0) {
    mm_extra_size = GetKextCount() * (sizeof(DTProperty) + sizeof(_DeviceTreeBuffer));
    mm_extra = (__typeof__(mm_extra))AllocateZeroPool(mm_extra_size - sizeof(DTProperty));
    /*Status =  */LogDataHub(&gEfiMiscSubClassGuid, L"mm_extra", mm_extra, (UINT32)(mm_extra_size - sizeof(DTProperty)));
    extra_size = GetKextsSize();
    extra = (__typeof__(extra))AllocateZeroPool(extra_size - sizeof(DTProperty) + EFI_PAGE_SIZE);
    /*Status =  */LogDataHub(&gEfiMiscSubClassGuid, L"extra", extra, (UINT32)(extra_size - sizeof(DTProperty) + EFI_PAGE_SIZE));
    // MsgLog("count: %d    \n", GetKextCount());
    // MsgLog("mm_extra_size: %d    \n", mm_extra_size);
    // MsgLog("extra_size: %d     \n", extra_size);
    // MsgLog("offset: %d       \n", extra_size - sizeof(DTProperty) + EFI_PAGE_SIZE);
    //no more needed
    FreePool(mm_extra);
    FreePool(extra);
  }

//  InjectKextList.setEmpty();
  return EFI_SUCCESS;
}

/*
 * Adler32 from Chameleon, not used
 */
#define BASE 65521L /* largest prime smaller than 65536 */
#define NMAX 5000
// NMAX (was 5521) the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1

#define DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);
#if 0
static UINT32 Adler32(unsigned char *buf, long len)
{
  unsigned long s1 = 1; // adler & 0xffff;
  unsigned long s2 = 0; // (adler >> 16) & 0xffff;
  unsigned long result;
  long k;

  while (len > 0) {
    k = len < NMAX ? len : NMAX;
    len -= k;
    while (k >= 16) {
      DO16(buf);
      buf += 16;
      k -= 16;
    }
    if (k != 0) do {
      s1 += *buf++;
      s2 += s1;
    } while (--k);
    s1 %= BASE;
    s2 %= BASE;
  }
  result = (s2 << 16) | s1;
  // result is in big endian
  return (UINT32)result;
}

typedef struct {
  UINT32  Magic;
  UINT32  Signature;
  UINT32  Length;
  UINT32  Adler32;
  UINT32  Version;
  UINT32  NumKexts;
  UINT32  CpuType;
  UINT32  CpuSubtype;
} MKextHeader;

typedef struct {
  UINT32  PlistOffset;
  UINT32  PlistCompressedSize;
  UINT32  PlistFullSize;
  UINT32  PlistModifiedSeconds;
  UINT32  BinaryOffset;
  UINT32  BinaryCompressedSize;
  UINT32  BinaryFullSize;
  UINT32  BinaryModifiedSeconds;
} MKextFile;

#define MKEXT_MAGIC     0x54584b4d
#define MKEXT_SIGNATURE 0x58534f4d
#define MKEXT_VERSION_1 0x00800001

int LOADER_ENTRY::is_mkext_v1(UINT8* drvPtr)
{
  _DeviceTreeBuffer *dtb = (_DeviceTreeBuffer*) (((UINT8*)drvPtr) + sizeof(DTProperty));
  MKextHeader* mkext_ptr = (MKextHeader*)(UINTN)(dtb->paddr);

  if (mkext_ptr->Magic == MKEXT_MAGIC
   && mkext_ptr->Signature == MKEXT_SIGNATURE
   && mkext_ptr->Version == MKEXT_VERSION_1) {
    DBG_RT("MKext_v1 found at paddr=0x%08x, length=0x%08x\n", dtb->paddr, dtb->length);
    return 1;
  }
  return 0;
}

void LOADER_ENTRY::patch_mkext_v1(UINT8 *drvPtr)
{
  _DeviceTreeBuffer *dtb = (_DeviceTreeBuffer*) (((UINT8*)drvPtr) + sizeof(DTProperty));
  MKextHeader* mkext_ptr = (MKextHeader*)(UINTN)dtb->paddr;

  UINT32 mkext_len      = SwapBytes32(mkext_ptr->Length);
  UINT32 mkext_numKexts = SwapBytes32(mkext_ptr->NumKexts);

  LIST_ENTRY    *Link;
  KEXT_ENTRY    *KextEntry;
  if(!IsListEmpty(&gKextList)) {
    for (Link = gKextList.ForwardLink; Link != &gKextList; Link = Link->ForwardLink) {
      KextEntry = CR(Link, KEXT_ENTRY, Link, KEXT_SIGNATURE);
      MKextFile *mkext_insert = (MKextFile*)((UINT8*)mkext_ptr + sizeof(MKextHeader) + mkext_numKexts * sizeof(MKextFile));

      // free some space
      CopyMem((UINT8*)mkext_insert + sizeof(MKextFile),
              (UINT8*)mkext_insert,
              mkext_len - (sizeof(MKextHeader) + mkext_numKexts * sizeof(MKextFile)));
      mkext_len += sizeof(MKextFile);

      // update the offsets to reflect 0x20 bytes moved above
      for (UINT32 i = 0; i < mkext_numKexts; i++) {
        MKextFile *kext_base = (MKextFile*)((UINT8*)mkext_ptr + sizeof(MKextHeader) + i * sizeof(MKextFile));
        UINT32 plist_offset  = SwapBytes32(kext_base->PlistOffset)  + sizeof(MKextFile);
        UINT32 binary_offset = SwapBytes32(kext_base->BinaryOffset) + sizeof(MKextFile);
        kext_base->PlistOffset  = SwapBytes32(plist_offset);
        kext_base->BinaryOffset = SwapBytes32(binary_offset);
      }

      // copy kext data (plist+binary)
      CopyMem((UINT8*)mkext_ptr + mkext_len,
              (UINT8*)(KextEntry->kext.paddr + sizeof(_BooterKextFileInfo)),
              (UINT32)((_BooterKextFileInfo*)(UINTN)(KextEntry->kext.paddr))->infoDictLength
               + (UINT32)((_BooterKextFileInfo*)(UINTN)(KextEntry->kext.paddr))->executableLength);

      // insert kext offsets
      mkext_insert->PlistOffset           = SwapBytes32(mkext_len);
      mkext_len += ((_BooterKextFileInfo*)(UINTN)(KextEntry->kext.paddr))->infoDictLength;
      mkext_insert->PlistCompressedSize   = 0;
      mkext_insert->PlistFullSize         = SwapBytes32((UINT32)((_BooterKextFileInfo*)(UINTN)(KextEntry->kext.paddr))->infoDictLength);
      mkext_insert->PlistModifiedSeconds  = 0;
      mkext_insert->BinaryOffset          = SwapBytes32(mkext_len);
      mkext_len += ((_BooterKextFileInfo*)(UINTN)(KextEntry->kext.paddr))->executableLength;
      mkext_insert->BinaryCompressedSize  = 0;
      mkext_insert->BinaryFullSize        = SwapBytes32((UINT32)((_BooterKextFileInfo*)(UINTN)(KextEntry->kext.paddr))->executableLength);
      mkext_insert->BinaryModifiedSeconds = 0;
      mkext_numKexts ++;

      // update the header
      mkext_ptr->Length   = SwapBytes32(mkext_len);
      mkext_ptr->NumKexts = SwapBytes32(mkext_numKexts);

      // update the checksum
      mkext_ptr->Adler32 = SwapBytes32(Adler32((UINT8*)mkext_ptr + 0x10, mkext_len - 0x10));

      // update the memory-map reference
      dtb->length = mkext_len;
    }
  }
}
#endif

////////////////////
// OnExitBootServices
////////////////////
EFI_STATUS LOADER_ENTRY::InjectKexts(IN UINT32 deviceTreeP, IN UINT32* deviceTreeLength)
{
  UINT8                             *dtEntry = (UINT8*)(UINTN) deviceTreeP;
  UINTN                             dtLen = (UINTN) *deviceTreeLength;

  DTEntry                           platformEntry;
  DTEntry                           memmapEntry;
  CHAR8                             *ptr;
  OpaqueDTPropertyIterator          OPropIter;
  DTPropertyIterator                iter = &OPropIter;
  DTProperty            *prop = NULL;

  UINT8                             *infoPtr = 0;
  UINT8                             *extraPtr = 0;
  UINT8                             *drvPtr = 0;
  UINTN                             offset = 0;

  LIST_ENTRY                        *Link;
  KEXT_ENTRY                        *KextEntry;
  UINTN                             KextBase = 0;
  _DeviceTreeBuffer                 *mm;
  _BooterKextFileInfo               *drvinfo;

  UINT32                            KextCount;
  UINTN                             Index;


  DBG_RT("\nInjectKexts: ");
  DBG("\nInjectKexts: ");
  KextCount = GetKextCount();
  if (KextCount == 0) {
    DBG_RT("no kexts to inject.\nPausing 5 secs ...\n");
    if (KernelAndKextPatches.KPDebug) {
      gBS->Stall(5000000);
    }
    return EFI_NOT_FOUND;
  }
  DBG_RT("%d kexts ...\n", KextCount);

  // kextsBase = Desc->PhysicalStart + (((UINTN) Desc->NumberOfPages) * EFI_PAGE_SIZE);
  // kextsPages = EFI_SIZE_TO_PAGES(kext.length);
  // Status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, kextsPages, &kextsBase);
  // if (EFI_ERROR(Status)) { MsgLog("Kext inject: could not allocate memory\n"); return Status; }
  // Desc->NumberOfPages += kextsPages;
  // CopyMem((void*)kextsBase, (void*)(UINTN)kext.paddr, kext.length);
  // drvinfo = (_BooterKextFileInfo*) kextsBase;
  // drvinfo->infoDictPhysAddr += (UINT32)kextsBase;
  // drvinfo->executablePhysAddr += (UINT32)kextsBase;
  // drvinfo->bundlePathPhysAddr += (UINT32)kextsBase;

  DTInit(dtEntry, deviceTreeLength);
  if(!EFI_ERROR(DTLookupEntry(NULL,"/chosen/memory-map", &memmapEntry))) {
    if(!EFI_ERROR(DTCreatePropertyIterator(memmapEntry, iter))) {
      while(!EFI_ERROR(DTIterateProperties(iter, &ptr))) {
        prop = iter->CurrentProperty;
        drvPtr = (UINT8*) prop;
        if(strncmp(prop->Name, "Driver-", 7)==0 || strncmp(prop->Name, "DriversPackage-", 15)==0) {
          break;
        }
      }
    }
  }

  if(!EFI_ERROR(DTLookupEntry(NULL, "/efi/platform", &platformEntry))) {
    if(!EFI_ERROR(DTCreatePropertyIterator(platformEntry, iter))) {
      while(!EFI_ERROR(DTIterateProperties(iter, &ptr))) {
        prop = iter->CurrentProperty;
        if(strncmp(prop->Name, "mm_extra", 8)==0) {
          infoPtr = (UINT8*)prop;
        }
        if(strncmp(prop->Name, "extra", 5)==0) {
          extraPtr = (UINT8*)prop;
        }
      }
    }
  }

  if (drvPtr == 0 || infoPtr == 0 || extraPtr == 0 || drvPtr > infoPtr || drvPtr > extraPtr || infoPtr > extraPtr) {
    printf("\nInvalid device tree for kext injection\n");
    gBS->Stall(5000000);
    return EFI_INVALID_PARAMETER;
  }

  // make space for memory map entries
  platformEntry->NumProperties -= 2;
  offset = sizeof(DTProperty) + ((DTProperty*) infoPtr)->Length;
  CopyMem(drvPtr+offset, drvPtr, infoPtr-drvPtr);

  // make space behind device tree
  // platformEntry->nProperties--;
  offset = sizeof(DTProperty)+((DTProperty*) extraPtr)->Length;
  CopyMem(extraPtr, extraPtr+offset, dtLen-(UINTN)(extraPtr-dtEntry)-offset);
  *deviceTreeLength -= (UINT32)offset;

  KextBase = RoundPage(dtEntry + *deviceTreeLength);
  if(!IsListEmpty(&gKextList)) {
    Index = 1;
    for (Link = gKextList.ForwardLink; Link != &gKextList; Link = Link->ForwardLink) {
      KextEntry = CR(Link, KEXT_ENTRY, Link, KEXT_SIGNATURE);

      CopyMem((void*) KextBase, (void*)(UINTN) KextEntry->kext.paddr, KextEntry->kext.length);
      drvinfo = (_BooterKextFileInfo*) KextBase;
      drvinfo->infoDictPhysAddr += (UINT32) KextBase;
      drvinfo->executablePhysAddr += (UINT32) KextBase;
      drvinfo->bundlePathPhysAddr += (UINT32) KextBase;

      memmapEntry->NumProperties++;
      prop = ((DTProperty*) drvPtr);
      prop->Length = sizeof(_DeviceTreeBuffer);
      mm = (_DeviceTreeBuffer*) (((UINT8*)prop) + sizeof(DTProperty));
      mm->paddr = (UINT32)KextBase;
      mm->length = KextEntry->kext.length;
      snprintf(prop->Name, 31, "Driver-%X", (UINT32)KextBase);

      drvPtr += sizeof(DTProperty) + sizeof(_DeviceTreeBuffer);
      KextBase = RoundPage(KextBase + KextEntry->kext.length);
      DBG_RT(" %llu - %s\n", Index, (CHAR8 *)(UINTN)drvinfo->bundlePathPhysAddr);
      DBG(" %llu - %s\n", Index, (CHAR8 *)(UINTN)drvinfo->bundlePathPhysAddr);
      if (gSettings.KextPatchesAllowed) {
        CHAR8  SavedValue;
        CHAR8 *InfoPlist = (CHAR8*)(UINTN)drvinfo->infoDictPhysAddr;
        SavedValue = InfoPlist[drvinfo->infoDictLength];
        InfoPlist[drvinfo->infoDictLength] = '\0';
 //       KernelAndKextPatcherInit();
        for (size_t i = 0; i < KernelAndKextPatches.KextPatches.size(); i++) {
          if ((KernelAndKextPatches.KextPatches[i].Data.size() > 0) &&
              (AsciiStrStr(InfoPlist, KernelAndKextPatches.KextPatches[i].Name.c_str()) != NULL)) {
            AnyKextPatch(
                         (UINT8*)(UINTN)drvinfo->executablePhysAddr,
                         drvinfo->executableLength,
                         InfoPlist,
                         drvinfo->infoDictLength,
                         i
                         );
          }
        }
        InfoPlist[drvinfo->infoDictLength] = SavedValue;
      }
      Index++;
    }
  }

  DBG_RT("Done.\n");
  Stall(5000000);
  return EFI_SUCCESS;
}


////////////////////////////////////
//
// KernelBooterExtensionsPatch to load extra kexts besides kernelcache
//
//

// Snow Leopard i386
const UINT8   KBESnowSearchEXT_i386[]  = { 0xE8, 0xED, 0xF9, 0xFF, 0xFF, 0xEB, 0x08, 0x89, 0x1C, 0x24 };
const UINT8   KBESnowReplaceEXT_i386[] = { 0xE8, 0xED, 0xF9, 0xFF, 0xFF, 0x90, 0x90, 0x89, 0x1C, 0x24 };

// Snow Leopard X64
const UINT8   KBESnowSearchEXT_X64[]   = { 0xE8, 0x5A, 0xFB, 0xFF, 0xFF, 0xEB, 0x08, 0x48, 0x89, 0xDF };
const UINT8   KBESnowReplaceEXT_X64[]  = { 0xE8, 0x5A, 0xFB, 0xFF, 0xFF, 0x90, 0x90, 0x48, 0x89, 0xDF };

// Lion i386
const UINT8   KBELionSearchEXT_i386[]  = { 0xE8, 0xAA, 0xFB, 0xFF, 0xFF, 0xEB, 0x08, 0x89, 0x34, 0x24 };
const UINT8   KBELionReplaceEXT_i386[] = { 0xE8, 0xAA, 0xFB, 0xFF, 0xFF, 0x90, 0x90, 0x89, 0x34, 0x24 };

// Lion X64
const UINT8   KBELionSearchEXT_X64[]   = { 0xE8, 0x0C, 0xFD, 0xFF, 0xFF, 0xEB, 0x08, 0x48, 0x89, 0xDF };
const UINT8   KBELionReplaceEXT_X64[]  = { 0xE8, 0x0C, 0xFD, 0xFF, 0xFF, 0x90, 0x90, 0x48, 0x89, 0xDF };


//
// We can not rely on OSVersion global variable for OS version detection,
// since in some cases it is not correct (install of ML from Lion, for example).
// So, we'll use "brute-force" method - just try to patch.
// Actually, we'll at least check that if we can find only one instance of code that
// we are planning to patch.
//
// Fully reworked by Sherlocks. 2019.06.23
//


void EFIAPI LOADER_ENTRY::KernelBooterExtensionsPatch()
{
//  UINTN   Num = 0;
  UINTN   NumSnow_i386_EXT   = 0;
  UINTN   NumSnow_X64_EXT    = 0;
  UINTN   NumLion_i386_EXT   = 0;
  UINTN   NumLion_X64_EXT    = 0;
  UINTN   patchLocation2 = 0, patchLocation3 = 0;


  DBG_RT("\nPatching kernel for injected kexts...\n");

  if (is64BitKernel) {
    NumSnow_X64_EXT  = SearchAndCount(KernelData, KERNEL_MAX_SIZE, KBESnowSearchEXT_X64, sizeof(KBESnowSearchEXT_X64));
    NumLion_X64_EXT  = SearchAndCount(KernelData, KERNEL_MAX_SIZE, KBELionSearchEXT_X64, sizeof(KBELionSearchEXT_X64));
  } else {
    NumSnow_i386_EXT = SearchAndCount(KernelData, KERNEL_MAX_SIZE, KBESnowSearchEXT_i386, sizeof(KBESnowSearchEXT_i386));
    NumLion_i386_EXT = SearchAndCount(KernelData, KERNEL_MAX_SIZE, KBELionSearchEXT_i386, sizeof(KBELionSearchEXT_i386));
  }

  if (NumSnow_i386_EXT + NumSnow_X64_EXT + NumLion_i386_EXT + NumLion_X64_EXT > 1) {
    // more then one pattern found - we do not know what to do with it
    // and we'll skipp it
//	  DBG_RT("\nERROR patching kernel for injected kexts:\nmultiple patterns found (Snowi386: %llu, SnowX64: %llu, Lioni386: %llu, LionX64: %llu) - skipping patching!\n", NumSnow_i386_EXT, NumSnow_X64_EXT, NumLion_i386_EXT, NumLion_X64_EXT);
//    Stall(10000000);
    return;
  }

  // X64
  if (is64BitKernel) {
    if (NumSnow_X64_EXT == 1) {
      /*Num=*/ SearchAndReplace(KernelData, KERNEL_MAX_SIZE, KBESnowSearchEXT_X64, sizeof(KBESnowSearchEXT_X64), KBESnowReplaceEXT_X64, 1);
//		DBG_RT("==> kernel Snow Leopard X64: %llu replaces done.\n", Num);
    } else if (NumLion_X64_EXT == 1) {
      /*Num=*/ SearchAndReplace(KernelData, KERNEL_MAX_SIZE, KBELionSearchEXT_X64, sizeof(KBELionSearchEXT_X64), KBELionReplaceEXT_X64, 1);
//		DBG_RT("==> kernel Lion X64: %llu replaces done.\n", Num);
    } else {
      // EXT - load extra kexts besides kernelcache.
#if OLD_EXTRA_KEXT_PATCH
      UINT32  patchLocation1 = 0;
      for (UINT32 i = 0; i < 0x1000000; i++) {
        // 01 00 31 FF BE 14 00 05
        if (Kernel[i+0] == 0x01 && Kernel[i+1] == 0x00 && Kernel[i+2] == 0x31 &&
            Kernel[i+3] == 0xFF && Kernel[i+4] == 0xBE && Kernel[i+5] == 0x14 &&
            Kernel[i+6] == 0x00 && Kernel[i+7] == 0x05) {
          DBG_RT("==> found EXT Base (10.8 - recent macOS)\n");
          for (UINT32 y = i; y < 0x1000000; y++) {
            // E8 XX 00 00 00 EB XX XX
            if (Kernel[y+0] == 0xE8 && Kernel[y+2] == 0x00 && Kernel[y+3] == 0x00 &&
                Kernel[y+4] == 0x00 && Kernel[y+5] == 0xEB) {
                //(Kernel[y+7] == 0x48 || Kernel[y+7] == 0xE8)) { // 48:10.8-10.9/E8:10.10+
              patchLocation1 = y;
              DBG_RT("==> found EXT (10.8 - recent macOS) at 0x%08x\n", patchLocation1);
              break;
            }
          }
          break;
        }
      }
            
      if (!patchLocation1) {
        DBG_RT("==> can't find EXT (10.8 - recent macOS), kernel patch aborted.\n");
        gBS->Stall(3000000);
      }
            
      if (patchLocation1) {
        DBG_RT("==> patched EXT (10.8 - recent macOS) location=%x\n", patchLocation1);
        for (i = 5; i < 7; i++) {
          // E8 XX 00 00 00 EB XX XX
          // E8 XX 00 00 00 90 90 XX
          Kernel[patchLocation1 + i] = 0x90;
        }
      }
#else
      //Capitan
//      procedure at 950950, len = fffffffffffff3f0
//      proclen=256, end=256 startLen=0
//      found start at 0x950950
//      found pattern: 1
//    address: 0095098b
//    bytes:eb05
      
// BS
//      E8 ?? 00 00 00 EB 05 E8 -->
//      E8 ?? 00 00 00 90 90 E8.

      UINTN procLocation = searchProc("readStartupExtensions"_XS8);
      const UINT8 findJmp[] = {0xEB, 0x05};
      const UINT8 patchJmp[] = {0x90, 0x90};
      DBG("==> readStartupExtensions at %llx\n", procLocation);
      if (!SearchAndReplace(&KernelData[procLocation], 0x100, findJmp, 2, patchJmp, 1)) {
        DBG("load kexts not patched\n");
        for (UINTN j=procLocation+0x2b; j<procLocation+0x4b; ++j) {
          DBG("%02x ", KernelData[j]);
        }
        DBG("\n");
//        Stall(10000000);
        //second attempt brute force for 10.16
//        const UINT8 findJmp2[] = {0xEB, 0x05, 0xE8, 0x7D, 0x03};
//        const UINT8 patchJmp2[] = {0x90, 0x90, 0xE8, 0x7D, 0x03};
//        if (!SearchAndReplace(&KernelData[0], KERNEL_MAX_SIZE, findJmp2, 5, patchJmp2, 1)) {
//          DBG("load kexts 2 not patched\n");
//        } else {
//          DBG("load kexts 2 patched !!!\n");
//        }
      } else {
        DBG("load kexts patched \n");
//        for (UINTN j=procLocation+0x3b; j<procLocation+0x5b; ++j) {
//          DBG_RT("%02x", Kernel[j]);
//        }
//        DBG_RT("\n");
      }
      Stall(12000000);
#endif
      // SIP - bypass kext check by System Integrity Protection.
      //the pattern found in  __ZN6OSKext14loadExecutableEv:        // OSKext::loadExecutable()
//    iMac2017:Catalina sergey$ ./FindMask kernel -p loadExecutable -e 1000 -f 488500740048000048,FFFF00FF00FF0000FF
//      descending
//      procedure at 7a1ed0, len = ffffffffffff4b50
//      proclen=4096, end=4096 startLen=0
//      found start at 0x7a1ed0
//      found pattern: 1
//    address: 007a29b7
//    bytes:4885c074224889c348
      

//Capitan
//      ffffff800084897b 4885DB                          test       rbx, rbx
//      ffffff800084897e 7470                            je         0xffffff80008489f0 -> patch to not jump
//                       7412                                    jmp ffffff8000848992
//      ; Basic Block Input Regs: rbx -  Killed Regs: rax rdi
//      ffffff8000848980 488B03                          mov        rax, qword [ds:rbx]
//      ffffff8000848983 4889DF                          mov        rdi, rbx
//      ffffff8000848986 FF5028                          call       qword [ds:rax+0x28]
//      ffffff8000848989 483B1DE04F2B00                  cmp        rbx, qword [ds:0xffffff8000afd970]
//      ffffff8000848990 745E                            je         0xffffff80008489f0 -> patch to not jump
//      ; Basic Block Input Regs: r13 -  Killed Regs: rax rbx rdi
//      ffffff8000848992 498B4500                        mov        rax, qword [ds:r13+0x0]
//      procedure at 6487f0, len = 7250
//      proclen=512, end=512 startLen=0
//      found start at 0x6487f0
//      found pattern: 1
//    address: 0064897b
//    bytes:4885db7470

#if OLD_EXTRA_KEXT_PATCH
      for (UINT32 i = 0; i < 0x1000000; i++) {
        // 45 31 FF 41 XX 01 00 00 DC 48
        if (Kernel[i+0] == 0x45 && Kernel[i+1] == 0x31 && Kernel[i+3] == 0x41 &&
            //(Kernel[i+4] == 0xBF || Kernel[i+4] == 0xBE) && // BF:10.11/BE:10.12+
            Kernel[i+5] == 0x01 && Kernel[i+6] == 0x00 && Kernel[i+7] == 0x00 &&
            Kernel[i+8] == 0xDC && Kernel[i+9] == 0x48) {
          
          DBG_RT("==> found loadExecutable (10.11 - recent macOS) at %x\n", i);
          for (UINT32 y = i; y < 0x100000; y++) {
            // 48 85 XX 74 XX 48 XX XX 48
            if (Kernel[y+0] == 0x48 && Kernel[y+1] == 0x85 && Kernel[y+3] == 0x74 &&
                Kernel[y+5] == 0x48 && Kernel[y+8] == 0x48) {
              patchLocation2 = y;
              DBG_RT("==> found SIP (10.11 - 10.14) at 0x%08x\n", patchLocation2);
              break;
            // 00 85 C0 0F 84 XX 00 00 00 49  //???? - not found in Catalina
            } else if (Kernel[y+0] == 0x00 && Kernel[y+1] == 0x85 && Kernel[y+2] == 0xC0 &&
                       Kernel[y+3] == 0x0F && Kernel[y+4] == 0x84 && Kernel[y+9] == 0x49) {
              patchLocation2 = y;
              DBG_RT("==> found SIP (10.15 - recent macOS) at 0x%08x\n", patchLocation2);
              break;
            }
          }
          break;
        }
      }
#else
      // BS
      //      E8 ?? ?? ?? 00 85 C0 0F 84 ?? 00 00 00 49 8B 45 -->
      //      E8 ?? ?? ?? 00 85 C0 90 90 90 90 90 90 49 8B 45.

      const UINT8 find3[] = {0x48, 0x85, 00, 0x74, 00, 0x48, 00, 00, 0x48 };
      const UINT8 mask3[] = {0xFF, 0xFF, 00, 0xFF, 00, 0xFF, 00, 00, 0xFF };
#endif
      
//ffffff80009a2267 488D35970D2400                  lea        rsi, qword [ds:0xffffff8000be3005] ; "com.apple.private.security.kext-management"
//ffffff80009a226e E89D780D00                      call       _IOTaskHasEntitlement
//ffffff80009a2273 85C0                            test       eax, eax =>change to eb06 -> jmp .+6
//ffffff80009a2275 0F843C010000                    je         0xffffff80009a23b7
//ffffff80009a227b
      UINTN taskLocation = searchProc("IOTaskHasEntitlement"_XS8);
      procLocation = searchProc("loadExecutable"_XS8);
      patchLocation2 = FindMemMask(&KernelData[procLocation], 0x500, find3, sizeof(find3), mask3, sizeof(mask3));
      DBG("IOTaskHasEntitlement at 0x%llx, loadExecutable at 0x%llx\n", taskLocation, procLocation);
      DBG("find3 at 0x%llx\n", patchLocation2);
      if (patchLocation2 != MAX_UINTN) {
        DBG_RT("=> patch SIP applied\n");
        patchLocation2 += procLocation;
        KernelData[patchLocation2 + 3] = 0xEB;
        if (KernelData[patchLocation2 + 4] == 0x6C) {
          KernelData[patchLocation2 + 4] = 0x15;
        } else {
          KernelData[patchLocation2 + 4] = 0x12;
        }
      } else {
        patchLocation2 = FindRelative32(KernelData, procLocation, 0x700, taskLocation);
        DBG("else search relative at 0x%llx\n", patchLocation2);
        if (patchLocation2 != 0) {
          DBG_RT("=> patch2 SIP applied\n");
          KernelData[patchLocation2] = 0xEB;
          KernelData[patchLocation2 + 1] = 0x06;
        } else {
          DBG_RT("=> patch2 SIP not applied\n");
          const UINT8 find7[] = {0xE8, 0x00, 0x00, 0x00, 0x00, 0x85, 0xC0, 0x0F, 0x84, 0xFF, 0x00, 0x00, 0x00, 0x49, 0x8B, 0x45 };
          const UINT8 mask7[] = {0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
          patchLocation2 = FindMemMask(&KernelData[0], KERNEL_MAX_SIZE, find7, sizeof(find7), mask7, sizeof(mask7));
          DBG("found call to TE at 0x%llx\n", patchLocation2);
          KernelData[0 + patchLocation2 + 7] = 0xEB;
          KernelData[0 + patchLocation2 + 8] = 0x04;

        }
      }
      Stall(10000000);
      
/*
      //Capitan: 48 85 db 74 70 48 8b 03 48
      if (patchLocation2) {
        if (taskFound) {
          UINT8 jmp = Kernel[patchLocation2 + 4];
          const UINT8 repl4[] = {0xB8, 0x01, 0x00, 0x00, 0x00, 0xEB};
          CopyMem(&Kernel[patchLocation2], repl4, sizeof(repl4));
          Kernel[patchLocation2 + 6] = jmp;
          DBG_RT("=> mojave SIP applyed\n");
        } else
        if (Kernel[patchLocation2 + 0] == 0x48 && Kernel[patchLocation2 + 1] == 0x85) {
          
          Kernel[patchLocation2 + 3] = 0xEB;
          DBG_RT("==> patched SIP (10.11 - 10.14)\n");
          if (Kernel[patchLocation2 + 4] == 0x6C) {
            // 48 85 XX 74 6C 48 XX XX 48
            // 48 85 XX EB 15 48 XX XX 48
            Kernel[patchLocation2 + 4] = 0x15; // 10.14.4-10.14.6
          } else {
            // 48 85 XX 74 XX 48 XX XX 48
            // 48 85 XX EB 12 48 XX XX 48
            Kernel[patchLocation2 + 4] = 0x12; // 10.11-10.14.3
          }
        // PMheart
        } else if (Kernel[patchLocation2 + 0] == 0x00 && Kernel[patchLocation2 + 1] == 0x85) {
          DBG_RT("==> patched SIP (10.15 - recent macOS)\n");
          for (i = 3; i < 9; i++) {
            // 00 85 C0 0F 84 XX 00 00 00 49
            // 00 85 C0 90 90 90 90 90 90 49
            Kernel[patchLocation2 + i] = 0x90;
          }
        }
      }
      Stall(9000000);
 */
 //Slice - hope this patch useful for some system that I have no.
      // KxldUnmap by vit9696
      // Avoid race condition in OSKext::removeKextBootstrap when using booter kexts without keepsyms=1.
      procLocation = searchProc("removeKextBootstrap"_XS8);
      const UINT8 find5[] = {0x00, 0x0F, 0x85, 00, 00, 0x00, 0x00, 0x48 };
      const UINT8 mask5[] = {0xFF, 0xFF, 0xFF, 00, 00, 0xFF, 0xFF, 0xFF };
      patchLocation3 = FindMemMask(&KernelData[procLocation], 0x300, find5, sizeof(find5), mask5, sizeof(mask5));
      DBG("removeKextBootstrap at 0x%llx\n", patchLocation3);

 /*
      for (UINT32 i = 0; i < 0x1000000; i++) {
        // 55 48 89 E5 41 57 41 56 41 54 53 //10
        // 48 83 EC 30 48 C7 45 B8 XX XX XX //21
        // XX XX XX XX XX XX XX XX XX XX XX //32
        // XX XX XX XX XX XX XX XX XX XX XX //43
        // XX XX XX XX XX XX XX XX XX FF XX //54
        // XX XX XX XX XX XX XX XX XX FF FF //65
        if (Kernel[i+0] == 0x55 && Kernel[i+1] == 0x48 && Kernel[i+2] == 0x89 &&
            Kernel[i+3] == 0xE5 && Kernel[i+4] == 0x41 && Kernel[i+5] == 0x57 &&
            Kernel[i+6] == 0x41 && Kernel[i+7] == 0x56 && Kernel[i+8] == 0x41 &&
            Kernel[i+9] == 0x54 && Kernel[i+10] == 0x53 && Kernel[i+11] == 0x48 &&
            Kernel[i+12] == 0x83 && Kernel[i+13] == 0xEC && Kernel[i+14] == 0x30 &&
            Kernel[i+15] == 0x48 && Kernel[i+16] == 0xC7 && Kernel[i+17] == 0x45 &&
            Kernel[i+18] == 0xB8 && Kernel[i+53] == 0xFF && Kernel[i+64] == 0xFF && Kernel[i+65] == 0xFF) {
          DBG_RT("==> found KxldUnmap Base (10.14 - recent macOS)\n");
          for (UINT32 y = i; y < 0x1000000; y++) {
            // 00 0F 85 XX XX 00 00 48
            if (Kernel[y+0] == 0x00 && Kernel[y+1] == 0x0F && Kernel[y+2] == 0x85 &&
                Kernel[y+5] == 0x00 && Kernel[y+6] == 0x00 && Kernel[y+7] == 0x48) {
              patchLocation3 = y;
              DBG_RT("==> found KxldUnmap (10.14 - recent macOS) at 0x%08x\n", patchLocation3);
              break;
            }
          }
          break;
        }
      }
 */
      //BS
      //FF 80 3D ?? ?? ?? 00 00 0F 85 ?? 01 00 00 41 -->
      //FF 80 3D ?? ?? ?? 00 00 90 E9 ?? 01 00 00 41.
/*
      if (patchLocation3  == KERNEL_MAX_SIZE) {
        DBG_RT("==> can't find KxldUnmap (10.14 - 10.15)\n");
        Stall(3000000);
        const UINT8 find6[] = {0xFF, 0x80, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x85, 0x00, 0x01, 0x00, 0x00, 0x41 };
        const UINT8 mask6[] = {0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF };
        patchLocation3 = FindMemMask(&KernelData[0], KERNEL_MAX_SIZE, find6, sizeof(find6), mask6, sizeof(mask6));
        DBG("find mask 6 at 0x%llx\n", patchLocation3);
        if (patchLocation3  != KERNEL_MAX_SIZE) {
          KernelData[0 + patchLocation3 + 8] = 0x90;
          KernelData[0 + patchLocation3 + 9] = 0xE9;
        }
      } else {
 */
      //The patch is not needed for bigsur
      if (patchLocation3 != MAX_UINTN) {
        DBG("==> patched KxldUnmap (10.14 - 10.15)\n");
        // 00 0F 85 XX XX 00 00 48
        // 00 90 E9 XX XX 00 00 48
        KernelData[procLocation + patchLocation3 + 1] = 0x90;
        KernelData[procLocation + patchLocation3 + 2] = 0xE9;
      }
    }
  } else {
    // i386
    if (NumSnow_i386_EXT == 1) {
/*Num=*/ SearchAndReplace(KernelData, KERNEL_MAX_SIZE, KBESnowSearchEXT_i386, sizeof(KBESnowSearchEXT_i386), KBESnowReplaceEXT_i386, 1);
//		DBG_RT("==> kernel Snow Leopard i386: %llu replaces done.\n", Num);
    } else if (NumLion_i386_EXT == 1) {
/*Num=*/ SearchAndReplace(KernelData, KERNEL_MAX_SIZE, KBELionSearchEXT_i386, sizeof(KBELionSearchEXT_i386), KBELionReplaceEXT_i386, 1);
//		DBG_RT("==> kernel Lion i386: %llu replaces done.\n", Num);
    } else {
      DBG_RT("==> ERROR: NOT patched - unknown kernel.\n");
    }
  }

  DBG_RT("Pausing 5 secs ...\n");
  Stall(5000000);
}
