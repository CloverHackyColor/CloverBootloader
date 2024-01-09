/*
 * KextList.cpp
 *
 *  Created on: Feb 4, 2021
 *      Author: jief
 */

#include "KextList.h"
#include <Platform.h>
#include "../Settings/SelfOem.h"
#include "../libeg/libeg.h"


#ifndef DEBUG_ALL
#define DEBUG_SET 1
#else
#define DEBUG_KEXTLIST DEBUG_ALL
#endif

#if DEBUG_SET == 0
#define DBG(...)
#else
#define DBG(...) DebugLog (DEBUG_SET, __VA_ARGS__)
#endif



XObjArray<SIDELOAD_KEXT>        InjectKextList;


/*
 * Relative path to SelfDir (the efi dir)
 */
XStringW GetBundleVersion(const XStringW& pathUnderKextdDir)
{
  EFI_STATUS      Status;
  XStringW        CFBundleVersion;
  XStringW        InfoPlistPath;
  UINT8*          InfoPlistPtr = NULL;
  TagDict*      InfoPlistDict = NULL;
  const TagStruct*      Prop = NULL;
  UINTN           Size;
  InfoPlistPath = SWPrintf("%ls\\%ls\\%ls", selfOem.getKextsDirPathRelToSelfDir().wc_str(), pathUnderKextdDir.wc_str(), L"Contents\\Info.plist");
  Status = egLoadFile(&self.getCloverDir(), InfoPlistPath.wc_str(), &InfoPlistPtr, &Size);
  if (EFI_ERROR(Status)) {
    InfoPlistPath = SWPrintf("%ls\\%ls\\%ls", selfOem.getKextsDirPathRelToSelfDir().wc_str(), pathUnderKextdDir.wc_str(), L"Info.plist");
    Status = egLoadFile(&self.getCloverDir(), InfoPlistPath.wc_str(), &InfoPlistPtr, &Size);
  }
  if(!EFI_ERROR(Status)) {
    //DBG("about to parse xml file %ls\n", InfoPlistPath.wc_str());
    Status = ParseXML(InfoPlistPtr, &InfoPlistDict, Size);
    if(!EFI_ERROR(Status) && (InfoPlistDict != nullptr)) {
      Prop = InfoPlistDict->propertyForKey("CFBundleVersion");
      if (Prop != NULL && Prop->isString() && Prop->getString()->stringValue().notEmpty()) {
        CFBundleVersion = SWPrintf("%s", Prop->getString()->stringValue().c_str());
      }
    }
  }
  if (InfoPlistPtr) {
    FreePool(InfoPlistPtr);
  }
  if ( InfoPlistDict ) InfoPlistDict->ReleaseTag();
  return CFBundleVersion;
}

void GetListOfInjectKext(CHAR16 *KextDirNameUnderOEMPath)
{

  REFIT_DIR_ITER   DirIter;
  EFI_FILE_INFO   *DirEntry;
  SIDELOAD_KEXT   *mKext;
  SIDELOAD_KEXT   *mPlugInKext;
  XStringW         FullName;
//  XStringW        FullPath = SWPrintf("%ls\\KEXTS\\%ls", OEMPath.wc_str(), KextDirNameUnderOEMPath);
  REFIT_DIR_ITER   PlugInsIter;
  EFI_FILE_INFO   *PlugInEntry;
  XStringW         PlugInsPathUnderKextsDir;
  XStringW         PlugInPathRelToSelfDir;
  XStringW         PlugInsName;
  XBool            Blocked = false;

  if( !selfOem.isKextsDirFound() ) return;

  if (StrCmp(KextDirNameUnderOEMPath, L"Off") == 0) {
    Blocked = true;
  }

  DirIterOpen(&selfOem.getKextsDir(), KextDirNameUnderOEMPath, &DirIter);
  while (DirIterNext(&DirIter, 1, L"*.kext", &DirEntry)) {
    if (DirEntry->FileName[0] == L'.' || StrStr(DirEntry->FileName, L".kext") == NULL) {
      continue;
    }
    /*
     <key>CFBundleVersion</key>
     <string>8.8.8</string>
     */
//    FullName = SWPrintf("%ls\\%ls", FullPath.wc_str(), DirEntry->FileName);
    XStringW pathUnderKextsDir = SWPrintf("%ls\\%ls", KextDirNameUnderOEMPath, DirEntry->FileName);
    mKext = new SIDELOAD_KEXT;
    mKext->FileName.SWPrintf("%ls", DirEntry->FileName);
    mKext->MenuItem.BValue = Blocked;
    mKext->KextDirNameUnderOEMPath.SWPrintf("%ls", KextDirNameUnderOEMPath);
    mKext->Version = GetBundleVersion(pathUnderKextsDir);
    InjectKextList.AddReference(mKext, true);

    DBG("Added Kext=%ls\\%ls (%ls)\n", mKext->KextDirNameUnderOEMPath.wc_str(), mKext->FileName.wc_str(), mKext->Version.wc_str());

    // Obtain PlugInList
    // Iterate over PlugIns directory
    PlugInsPathUnderKextsDir = SWPrintf("%ls\\Contents\\PlugIns", pathUnderKextsDir.wc_str());
    PlugInPathRelToSelfDir = SWPrintf("%ls\\%ls", selfOem.getKextsDirPathRelToSelfDir().wc_str(), PlugInsPathUnderKextsDir.wc_str());

    DirIterOpen(&self.getCloverDir(), PlugInPathRelToSelfDir.wc_str(), &PlugInsIter);
    while (DirIterNext(&PlugInsIter, 1, L"*.kext", &PlugInEntry)) {
      if (PlugInEntry->FileName[0] == L'.' || StrStr(PlugInEntry->FileName, L".kext") == NULL) {
        continue;
      }
      PlugInsName = SWPrintf("%ls\\%ls", PlugInsPathUnderKextsDir.wc_str(), PlugInEntry->FileName);
      mPlugInKext = new SIDELOAD_KEXT;
      mPlugInKext->FileName.SWPrintf("%ls", PlugInEntry->FileName);
      mPlugInKext->MenuItem.BValue = Blocked;
      mPlugInKext->KextDirNameUnderOEMPath = SWPrintf("%ls\\%ls\\Contents\\PlugIns", KextDirNameUnderOEMPath, mKext->FileName.wc_str());
      mPlugInKext->Version = GetBundleVersion(PlugInsName);
      mKext->PlugInList.AddReference(mPlugInKext, true);
      DBG("---| added plugin=%ls (%ls)\n", mPlugInKext->FileName.wc_str(), mPlugInKext->Version.wc_str());
    }
    DirIterClose(&PlugInsIter);
  }
  DirIterClose(&DirIter);
}

void InitKextList()
{
  REFIT_DIR_ITER  KextsIter;
  EFI_FILE_INFO   *FolderEntry = NULL;
//  XStringW        KextsPath;

  if (InjectKextList.notEmpty()) {
    return;  //don't scan again
  }
//  KextsPath = SWPrintf("%ls\\kexts", OEMPath.wc_str());
  DbgHeader("InitKextList");

  if ( selfOem.isKextsDirFound() ) {
    // Iterate over kexts directory
    DirIterOpen(&selfOem.getKextsDir(), NULL, &KextsIter);
    while (DirIterNext(&KextsIter, 1, L"*", &FolderEntry)) {
      if (FolderEntry->FileName[0] == L'.') {
        continue;
      }
      GetListOfInjectKext(FolderEntry->FileName);
    }
    DirIterClose(&KextsIter);
  }
}
