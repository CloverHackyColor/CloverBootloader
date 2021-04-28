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
#define DEBUG_KEXTLIST 1
#else
#define DEBUG_KEXTLIST DEBUG_ALL
#endif

#if DEBUG_SET == 0
#define DBG(...)
#else
#define DBG(...) DebugLog (DEBUG_KEXTLIST, __VA_ARGS__)
#endif



XObjArray<SIDELOAD_KEXT>        InjectKextList;


/*
 * Relative path to SelfDir (the efi dir)
 */
XStringW GetBundleVersion(const XStringW& pathUnderSelf)
{
  EFI_STATUS      Status;
  XStringW        CFBundleVersion;
  XStringW        InfoPlistPath;
  CHAR8*          InfoPlistPtr = NULL;
  TagDict*      InfoPlistDict = NULL;
  const TagStruct*      Prop = NULL;
  UINTN           Size;
  InfoPlistPath = SWPrintf("%ls\\%ls", pathUnderSelf.wc_str(), L"Contents\\Info.plist");
  Status = egLoadFile(&self.getCloverDir(), InfoPlistPath.wc_str(), (UINT8**)&InfoPlistPtr, &Size);
  if (EFI_ERROR(Status)) {
    InfoPlistPath = SWPrintf("%ls\\%ls", pathUnderSelf.wc_str(), L"Info.plist");
    Status = egLoadFile(&self.getCloverDir(), InfoPlistPath.wc_str(), (UINT8**)&InfoPlistPtr, &Size);
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
  if ( InfoPlistDict ) InfoPlistDict->FreeTag();
  return CFBundleVersion;
}

void GetListOfInjectKext(CHAR16 *KextDirNameUnderOEMPath)
{

  REFIT_DIR_ITER  DirIter;
  EFI_FILE_INFO*  DirEntry;
  SIDELOAD_KEXT*  mKext;
  SIDELOAD_KEXT*  mPlugInKext;
  XStringW        FullName;
//  XStringW        FullPath = SWPrintf("%ls\\KEXTS\\%ls", OEMPath.wc_str(), KextDirNameUnderOEMPath);
  REFIT_DIR_ITER  PlugInsIter;
  EFI_FILE_INFO   *PlugInEntry;
  XStringW        PlugInsPath;
  XStringW         PlugInsName;
  BOOLEAN         Blocked = FALSE;

  if( !selfOem.isKextsDirFound() ) return;

  if (StrCmp(KextDirNameUnderOEMPath, L"Off") == 0) {
    Blocked = TRUE;
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
    XStringW pathRelToSelfDir = SWPrintf("%ls\\%ls\\%ls", selfOem.getKextsDirPathRelToSelfDir().wc_str(), KextDirNameUnderOEMPath, DirEntry->FileName);
    mKext = new SIDELOAD_KEXT;
    mKext->FileName.SWPrintf("%ls", DirEntry->FileName);
    mKext->MenuItem.BValue = Blocked;
    mKext->KextDirNameUnderOEMPath.SWPrintf("%ls", KextDirNameUnderOEMPath);
    mKext->Version = GetBundleVersion(pathRelToSelfDir);
    InjectKextList.AddReference(mKext, true);

    DBG("Added Kext=%ls\\%ls\n", mKext->KextDirNameUnderOEMPath.wc_str(), mKext->FileName.wc_str());

    // Obtain PlugInList
    // Iterate over PlugIns directory
    PlugInsPath = SWPrintf("%ls\\Contents\\PlugIns", pathRelToSelfDir.wc_str());

    DirIterOpen(&self.getCloverDir(), PlugInsPath.wc_str(), &PlugInsIter);
    while (DirIterNext(&PlugInsIter, 1, L"*.kext", &PlugInEntry)) {
      if (PlugInEntry->FileName[0] == L'.' || StrStr(PlugInEntry->FileName, L".kext") == NULL) {
        continue;
      }
      PlugInsName = SWPrintf("%ls\\%ls", PlugInsPath.wc_str(), PlugInEntry->FileName);
      mPlugInKext = new SIDELOAD_KEXT;
      mPlugInKext->FileName.SWPrintf("%ls", PlugInEntry->FileName);
      mPlugInKext->MenuItem.BValue = Blocked;
      mPlugInKext->KextDirNameUnderOEMPath = SWPrintf("%ls\\%ls\\Contents\\PlugIns", KextDirNameUnderOEMPath, mKext->FileName.wc_str());
      mPlugInKext->Version = GetBundleVersion(PlugInsName);
      mKext->PlugInList.AddReference(mPlugInKext, true);
      //      DBG("---| added plugin=%ls, MatchOS=%ls\n", mPlugInKext->FileName, mPlugInKext->MatchOS);
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
