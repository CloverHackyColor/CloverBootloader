/*
 * Copyright (c) 2011-2012 Frank Peng. All rights reserved.
 *
 */

#include "Platform.h"
#include "LoaderUefi.h"
#include "device_tree.h"

#include "kernel_patcher.h"

#define KEXT_DEBUG 0

#if KEXT_DEBUG
#define DBG(...)	Print(__VA_ARGS__);
#else
#define DBG(...)
#endif

// runtime debug
#define DBG_RT(...)    if (gSettings.KPDebug) { AsciiPrint(__VA_ARGS__); }

//
// Searches Source for Search pattern of size SearchSize
// and returns the number of occurences.
//
UINTN SearchAndCount(UINT8 *Source, UINT32 SourceSize, UINT8 *Search, UINTN SearchSize)
{
  UINTN     NumFounds = 0;
  UINT8     *End = Source + SourceSize;
  
  while (Source < End) {
    if (CompareMem(Source, Search, SearchSize) == 0) {
      NumFounds++;
      Source += SearchSize;
    } else {
      Source++;
    }
  }
  return NumFounds;
}

//
// Searches Source for Search pattern of size SearchSize
// and replaces it with Replace up to MaxReplaces times.
// If MaxReplaces <= 0, then there is no restriction on number of replaces.
// Replace should have the same size as Search.
// Returns number of replaces done.
//
UINTN SearchAndReplace(UINT8 *Source, UINT32 SourceSize, UINT8 *Search, UINTN SearchSize, UINT8 *Replace, INTN MaxReplaces)
{
  UINTN     NumReplaces = 0;
  BOOLEAN   NoReplacesRestriction = MaxReplaces <= 0;
  UINT8     *End = Source + SourceSize;
  if (!Source || !Search || !Replace || !SearchSize) {
    return 0;
  }
  
  while ((Source < End) && (NoReplacesRestriction || (MaxReplaces > 0))) {
    if (CompareMem(Source, Search, SearchSize) == 0) {
      CopyMem(Source, Replace, SearchSize);
      NumReplaces++;
      MaxReplaces--;
      Source += SearchSize;
    } else {
      Source++;
    }
  }
  return NumReplaces;
}

/** Global for storing KextBoundleIdentifier */
CHAR8 gKextBoundleIdentifier[256];

/** Extracts kext BoundleIdentifier from given Plist into gKextBoundleIdentifier */
VOID ExtractKextBoundleIdentifier(CHAR8 *Plist)
{
  CHAR8     *Tag;
  CHAR8     *BIStart;
  CHAR8     *BIEnd;
  INTN      DictLevel = 0;
  
  
  gKextBoundleIdentifier[0] = '\0';
  
  // start with first <dict>
  Tag = AsciiStrStr(Plist, "<dict>");
  if (Tag == NULL) {
    return;
  }
  Tag += 6;
  DictLevel++;
  
  while (Tag != '\0') {
    
    if (AsciiStrnCmp(Tag, "<dict>", 6) == 0) {
      // opening dict
      DictLevel++;
      Tag += 6;
      
    } else if (AsciiStrnCmp(Tag, "</dict>", 7) == 0) {
      // closing dict
      DictLevel--;
      Tag += 7;
      
    } else if (DictLevel == 1 && AsciiStrnCmp(Tag, "<key>CFBundleIdentifier</key>", 29) == 0) {
      // BundleIdentifier is next <string>...</string>
      BIStart = AsciiStrStr(Tag + 29, "<string>");
      if (BIStart != NULL) {
        BIStart += 8; // skip "<string>"
        BIEnd = AsciiStrStr(BIStart, "</string>");
        if (BIEnd != NULL && (BIEnd - BIStart + 1) < sizeof(gKextBoundleIdentifier)) {
          CopyMem(gKextBoundleIdentifier, BIStart, BIEnd - BIStart);
          gKextBoundleIdentifier[BIEnd - BIStart] = '\0';
          return;
        }
      }
      Tag++;
    } else {
      Tag++;
    }
    
    // advance to next tag
    while (*Tag != '<' && *Tag != '\0') {
      Tag++;
    }
  }
}



////////////////////////////////////
//
// ATIConnectors patch
//
// bcc9's patch: http://www.insanelymac.com/forum/index.php?showtopic=249642
//

// inited or not?
BOOLEAN ATIConnectorsPatchInited = FALSE;

// ATIConnectorsController's boundle IDs for
// 0: ATI version - Lion, SnowLeo 10.6.7 2011 MBP
// 1: AMD version - ML
CHAR8 ATIKextBoundleId[2][64];

//
// Inits patcher: prepares ATIKextBoundleIds.
//
VOID ATIConnectorsPatchInit(VOID)
{
  //
  // prepar boundle ids
  //
  
  // Lion, SnowLeo 10.6.7 2011 MBP
  AsciiSPrint(ATIKextBoundleId[0],
              sizeof(ATIKextBoundleId[0]),
              "com.apple.kext.ATI%sController",
              gSettings.KPATIConnectorsController
              );
  // ML
  AsciiSPrint(ATIKextBoundleId[1],
              sizeof(ATIKextBoundleId[1]),
              "com.apple.kext.AMD%sController",
              gSettings.KPATIConnectorsController
              );
  
  ATIConnectorsPatchInited = TRUE;
  
  //DBG(L"Boundle1: %a\n", ATIKextBoundleId[0]);
  //DBG(L"Boundle2: %a\n", ATIKextBoundleId[1]);
  //gBS->Stall(10000000);
}

//
// Registers kexts that need force-load during WithKexts boot.
//
VOID ATIConnectorsPatchRegisterKexts(FSINJECTION_PROTOCOL *FSInject, FSI_STRING_LIST *ForceLoadKexts)
{
  
  // for future?
  FSInject->AddStringToList(ForceLoadKexts,
                            PoolPrint(L"\\AMD%sController.kext\\Contents\\Info.plist", gSettings.KPATIConnectorsController)
                            );
  // Lion, ML, SnowLeo 10.6.7 2011 MBP
  FSInject->AddStringToList(ForceLoadKexts,
                            PoolPrint(L"\\ATI%sController.kext\\Contents\\Info.plist", gSettings.KPATIConnectorsController)
                            );
  // SnowLeo
  FSInject->AddStringToList(ForceLoadKexts, L"\\ATIFramebuffer.kext\\Contents\\Info.plist");
  FSInject->AddStringToList(ForceLoadKexts, L"\\AMDFramebuffer.kext\\Contents\\Info.plist");
  
  // dependencies
  FSInject->AddStringToList(ForceLoadKexts, L"\\IOGraphicsFamily.kext\\Info.plist");
  FSInject->AddStringToList(ForceLoadKexts, L"\\ATISupport.kext\\Contents\\Info.plist");
  FSInject->AddStringToList(ForceLoadKexts, L"\\AMDSupport.kext\\Contents\\Info.plist");
}

//
// Patch function.
//
VOID ATIConnectorsPatch(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize)
{
  
  UINTN   Num = 0;
  
  DBG_RT("\nATIConnectorsPatch: driverAddr = %x, driverSize = %x\nController = %s\n",
         Driver, DriverSize, gSettings.KPATIConnectorsController);
  ExtractKextBoundleIdentifier(InfoPlist);
  DBG_RT("Kext: %a\n", gKextBoundleIdentifier);
  
  // number of occurences od Data should be 1
  Num = SearchAndCount(Driver, DriverSize, gSettings.KPATIConnectorsData, gSettings.KPATIConnectorsDataLen);
  if (Num > 1) {
    // error message - shoud always be printed
    Print(L"==> KPATIConnectorsData found %d times in %a - skipping patching!\n", Num, gKextBoundleIdentifier);
    gBS->Stall(5*1000000);
    return;
  }
  
  // patch
  Num = SearchAndReplace(Driver,
                         DriverSize,
                         gSettings.KPATIConnectorsData,
                         gSettings.KPATIConnectorsDataLen,
                         gSettings.KPATIConnectorsPatch,
                         1);
  if (gSettings.KPDebug) {
    if (Num > 0) {
      DBG_RT("==> patched %d times!\n", Num);
    } else {
      DBG_RT("==> NOT patched!\n");
    }
    gBS->Stall(5000000);
  }
}



////////////////////////////////////
//
// AsusAICPUPM patch
//
// fLaked's SpeedStepper patch for Asus (and some other) boards:
// http://www.insanelymac.com/forum/index.php?showtopic=258611
//
// Credits: Samantha/RevoGirl/DHP
// http://www.insanelymac.com/forum/topic/253642-dsdt-for-asus-p8p67-m-pro/page__st__200#entry1681099
//
//

UINT8   MovlE2ToEcx[] = { 0xB9, 0xE2, 0x00, 0x00, 0x00 };
UINT8   Wrmsr[]       = { 0x0F, 0x30 };

VOID AsusAICPUPMPatch(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize)
{
  UINTN   Index1;
  UINTN   Index2;
  UINTN   Count = 0;
  
  DBG_RT("\nAsusAICPUPMPatch: driverAddr = %x, driverSize = %x\n", Driver, DriverSize);
  if (gSettings.KPDebug) {
    ExtractKextBoundleIdentifier(InfoPlist);
  }
  DBG_RT("Kext: %a\n", gKextBoundleIdentifier);
  
  //TODO: we should scan only __text __TEXT
  for (Index1 = 0; Index1 < DriverSize; Index1++) {
    // search for MovlE2ToEcx
    if (CompareMem(Driver + Index1, MovlE2ToEcx, sizeof(MovlE2ToEcx)) == 0) {
      // search for wrmsr in next few bytes
      for (Index2 = Index1 + sizeof(MovlE2ToEcx); Index2 < Index1 + sizeof(MovlE2ToEcx) + 16; Index2++) {
        if (Driver[Index2] == Wrmsr[0] && Driver[Index2 + 1] == Wrmsr[1]) {
          // found it - patch it with nops
          Count++;
          Driver[Index2] = 0x90;
          Driver[Index2 + 1] = 0x90;
          DBG_RT(" %d. patched at 0x%x\n", Count, Index2);
        }
      }
    }
  }
  DBG_RT("= %d patches\n", Count);
  if (gSettings.KPDebug) {
    gBS->Stall(5000000);
  }
}



////////////////////////////////////
//
// AppleRTC patch to prevent CMOS reset
//
// http://www.insanelymac.com/forum/index.php?showtopic=253992
// http://www.insanelymac.com/forum/index.php?showtopic=276066
//

UINT8   LionSearch_X64[]  = { 0x75, 0x30, 0x44, 0x89, 0xf8 };
UINT8   LionReplace_X64[] = { 0xeb, 0x30, 0x44, 0x89, 0xf8 };

UINT8   LionSearch_i386[]  = { 0x75, 0x3d, 0x8b, 0x75, 0x08 };
UINT8   LionReplace_i386[] = { 0xeb, 0x3d, 0x8b, 0x75, 0x08 };

UINT8   MLSearch[]  = { 0x75, 0x30, 0x89, 0xd8 };
UINT8   MLReplace[] = { 0xeb, 0x30, 0x89, 0xd8 };
//SunKi
//752e0fb6 -> eb2e0fb6
UINT8   MavSearch[]  = { 0x75, 0x2e, 0x0f, 0xb6 };
UINT8   MavReplace[] = { 0xeb, 0x2e, 0x0f, 0xb6};
//
// We can not rely on OSVersion global variable for OS version detection,
// since in some cases it is not correct (install of ML from Lion, for example).
// So, we'll use "brute-force" method - just try to patch.
// Actually, we'll at least check that if we can find only one instance of code that
// we are planning to patch.
//

VOID AppleRTCPatch(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize)
{
  
  UINTN   Num = 0;
  UINTN   NumLion_X64 = 0;
  UINTN   NumLion_i386 = 0;
  UINTN   NumML = 0;
  UINTN   NumMav = 0;
  
  DBG_RT("\nAppleRTCPatch: driverAddr = %x, driverSize = %x\n", Driver, DriverSize);
  if (gSettings.KPDebug) {
    ExtractKextBoundleIdentifier(InfoPlist);
  }
  DBG_RT("Kext: %a\n", gKextBoundleIdentifier);
  
  if (is64BitKernel) {
    NumLion_X64 = SearchAndCount(Driver, DriverSize, LionSearch_X64, sizeof(LionSearch_X64));
    NumML  = SearchAndCount(Driver, DriverSize, MLSearch,  sizeof(MLSearch));
    NumMav = SearchAndCount(Driver, DriverSize, MavSearch, sizeof(MavSearch));
  } else {
    NumLion_i386 = SearchAndCount(Driver, DriverSize, LionSearch_i386, sizeof(LionSearch_i386));
  }
  
  if (NumLion_X64 + NumLion_i386 + NumML + NumMav > 1) {
    // more then one pattern found - we do not know what to do with it
    // and we'll skipp it
    Print(L"AppleRTCPatch: ERROR: multiple patterns found (LionX64: %d, Lioni386: %d, ML: %d, Mav: %d) - skipping patching!\n",
          NumLion_X64, NumLion_i386, NumML, NumMav);
    gBS->Stall(5000000);
    return;
  }
  
  if (NumLion_X64 == 1) {
    Num = SearchAndReplace(Driver, DriverSize, LionSearch_X64, sizeof(LionSearch_X64), LionReplace_X64, 1);
    DBG_RT("==> Lion X64: %d replaces done.\n", Num);
  }
  else if (NumLion_i386 == 1) {
    Num = SearchAndReplace(Driver, DriverSize, LionSearch_i386, sizeof(LionSearch_i386), LionReplace_i386, 1);
    DBG_RT("==> Lion i386: %d replaces done.\n", Num);
  }
  else if (NumML == 1) {
    Num = SearchAndReplace(Driver, DriverSize, MLSearch, sizeof(MLSearch), MLReplace, 1);
    DBG_RT("==> MountainLion X64: %d replaces done.\n", Num);
  }
  else if (NumMav == 1) {
    Num = SearchAndReplace(Driver, DriverSize, MavSearch, sizeof(MavSearch), MavReplace, 1);
    DBG_RT("==> Mavericks X64: %d replaces done.\n", Num);
  }
  else {
    DBG_RT("==> Patterns not found - patching NOT done.\n");
  }
  if (gSettings.KPDebug) {
    gBS->Stall(5000000);
  }
}



///////////////////////////////////
//
// InjectKexts if no FakeSMC: Detect FakeSMC and if present then
// disable kext injection InjectKexts()
//

VOID CheckForFakeSMC(CHAR8 *InfoPlist)
{
  if (gSettings.WithKextsIfNoFakeSMC && !gSettings.FakeSMCFound) {
    if (AsciiStrStr(InfoPlist, "<string>org.netkas.driver.FakeSMC</string>") != NULL
        || AsciiStrStr(InfoPlist, "<string>org.netkas.FakeSMC</string>") != NULL)
    {
      gSettings.FakeSMCFound = TRUE;
      if (gSettings.KPDebug) {
        DBG_RT("\nFakeSMC found\n");
        gBS->Stall(5000000);
      }
    }
  }
}



////////////////////////////////////
//
// Place other kext patches here
//

// ...



////////////////////////////////////
//
// Generic kext patch functions
//
//
VOID AnyKextPatch(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize, INT32 N)
{
  
  UINTN   Num = 0;
  
  DBG_RT("\nAnyKextPatch %d: driverAddr = %x, driverSize = %x\nAnyKext = %a\n",
         N, Driver, DriverSize, gSettings.KextPatches[N].Name);
  if (gSettings.KPDebug) {
    ExtractKextBoundleIdentifier(InfoPlist);
  }
  DBG_RT("Kext: %a\n", gKextBoundleIdentifier);
  
  if (!gSettings.KextPatches[N].IsPlistPatch) {
    // kext binary patch
    DBG_RT("Binary patch\n");
    Num = SearchAndReplace(Driver,
                           DriverSize,
                           gSettings.KextPatches[N].Data,
                           gSettings.KextPatches[N].DataLen,
                           gSettings.KextPatches[N].Patch,
                           -1);
  } else {
    // Info plist patch
    DBG_RT("Info.plist patch: '%a' -> '%a'\n", gSettings.KextPatches[N].Data, gSettings.KextPatches[N].Patch);
    Num = SearchAndReplace((UINT8*)InfoPlist,
                           InfoPlistSize,
                           gSettings.KextPatches[N].Data,
                           gSettings.KextPatches[N].DataLen,
                           gSettings.KextPatches[N].Patch,
                           -1);
  }
  
  if (gSettings.KPDebug) {
    if (Num > 0) {
      DBG_RT("==> patched %d times!\n", Num);
    } else {
      DBG_RT("==> NOT patched!\n");
    }
    gBS->Stall(5000000);
  }
}

//
// Called from SetFSInjection(), before boot.efi is started,
// to allow patchers to prepare FSInject to force load needed kexts.
//
VOID KextPatcherRegisterKexts(FSINJECTION_PROTOCOL *FSInject, FSI_STRING_LIST *ForceLoadKexts)
{
  INTN i;
  
  if (gSettings.KPATIConnectorsController != NULL) {
    ATIConnectorsPatchRegisterKexts(FSInject, ForceLoadKexts);
  }
  
  for (i = 0; i < gSettings.NrKexts; i++) {
    FSInject->AddStringToList(ForceLoadKexts,
                              PoolPrint(L"\\%a.kext\\Contents\\Info.plist",
                                        gSettings.KextPatches[i].Name) );
  }
  
}

//
// PatchKext is called for every kext from prelinked kernel (kernelcache) or from DevTree (booting with drivers).
// Add kext detection code here and call kext speciffic patch function.
//
VOID PatchKext(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize)
{
  INT32 i;
  
  if (gSettings.KPATIConnectorsController != NULL) {
    //
    // ATIConnectors
    //
    if (!ATIConnectorsPatchInited) {
      ATIConnectorsPatchInit();
    }
    if (   AsciiStrStr(InfoPlist, ATIKextBoundleId[0]) != NULL  // ATI boundle id
        || AsciiStrStr(InfoPlist, ATIKextBoundleId[1]) != NULL  // AMD boundle id
        || AsciiStrStr(InfoPlist, "com.apple.kext.ATIFramebuffer") != NULL // SnowLeo
        || AsciiStrStr(InfoPlist, "com.apple.kext.AMDFramebuffer") != NULL //Maverics
        ) {
      ATIConnectorsPatch(Driver, DriverSize, InfoPlist, InfoPlistSize);
      return;
    }
  }
  
  if (gSettings.KPAsusAICPUPM &&
      (AsciiStrStr(InfoPlist,
                   "<string>com.apple.driver.AppleIntelCPUPowerManagement</string>") != NULL)) {
    //
    // AsusAICPUPM
    //
    AsusAICPUPMPatch(Driver, DriverSize, InfoPlist, InfoPlistSize);
  } else if (gSettings.KPAppleRTC &&
             (AsciiStrStr(InfoPlist, "com.apple.driver.AppleRTC") != NULL)) {
    //
    // AppleRTC
    //
    AppleRTCPatch(Driver, DriverSize, InfoPlist, InfoPlistSize);
  } else {
    //
    //others
    //
    for (i = 0; i < gSettings.NrKexts; i++) {
      if ((gSettings.KextPatches[i].DataLen > 0) &&
          (AsciiStrStr(InfoPlist, gSettings.KextPatches[i].Name) != NULL)) {
        DBG_RT("patch kext %a\n", gSettings.KextPatches[i].Name);
        AnyKextPatch(Driver, DriverSize, InfoPlist, InfoPlistSize, i);
      }
    }
  }
  
  //
  // Check for FakeSMC (InjectKexts if no FakeSMC)
  //
  CheckForFakeSMC(InfoPlist);
}

//
// Returns parsed hex integer key.
// Plist - kext pist
// Key - key to find
// WholePlist - _PrelinkInfoDictionary, used to find referenced values
//
// Searches for Key in Plist and it's value:
// a) <integer ID="26" size="64">0x2b000</integer>
//    returns 0x2b000
// b) <integer IDREF="26"/>
//    searches for <integer ID="26"... from WholePlist
//    and returns value from that referenced field
//
// Whole function is here since we should avoid ParseXML() and it's
// memory allocations during ExitBootServices(). And it seems that
// ParseXML() does not support IDREF.
// This func is hard to read and debug and probably not reliable,
// but it seems it works.
//
UINT64 GetPlistHexValue(CHAR8 *Plist, CHAR8 *Key, CHAR8 *WholePlist)
{
  CHAR8     *Value;
  CHAR8     *IntTag;
  UINT64    NumValue = 0;
  CHAR8     *IDStart;
  CHAR8     *IDEnd;
  UINTN     IDLen;
  CHAR8     Buffer[48];
  //static INTN   DbgCount = 0;
  
  // search for Key
  Value = AsciiStrStr(Plist, Key);
  if (Value == NULL) {
    //DBG(L"\nNo key: %a\n", Key);
    return 0;
  }
  
  // search for <integer
  IntTag = AsciiStrStr(Value, "<integer");
  if (IntTag == NULL) {
    DBG(L"\nNo integer\n");
    return 0;
  }
  
  // find <integer end
  Value = AsciiStrStr(IntTag, ">");
  if (Value == NULL) {
    DBG(L"\nNo <integer end\n");
    return 0;
  }
  
  if (Value[-1] != '/') {
    
    // normal case: value is here
    NumValue = AsciiStrHexToUint64(Value + 1);
    return NumValue;
    
  }
  
  // it might be a reference: IDREF="173"/>
  Value = AsciiStrStr(IntTag, "<integer IDREF=\"");
  if (Value != IntTag) {
    DBG(L"\nNo <integer IDREF=\"\n");
    return 0;
  }
  
  // compose <integer ID="xxx" in the Buffer
  IDStart = AsciiStrStr(IntTag, "\"") + 1;
  IDEnd = AsciiStrStr(IDStart, "\"");
  IDLen = IDEnd - IDStart;
  /*
   if (DbgCount < 3) {
   AsciiStrnCpy(Buffer, Value, sizeof(Buffer) - 1);
   DBG(L"\nRef: '%a'\n", Buffer);
   }
   */
  if (IDLen > 8) {
    DBG(L"\nIDLen too big\n");
    return 0;
  }
  AsciiStrCpy(Buffer, "<integer ID=\"");
  AsciiStrnCat(Buffer, IDStart, IDLen);
  AsciiStrCat(Buffer, "\"");
  /*
   if (DbgCount < 3) {
   DBG(L"Searching: '%a'\n", Buffer);
   }
   */
  
  // and search whole plist for ID
  IntTag = AsciiStrStr(WholePlist, Buffer);
  if (IntTag == NULL) {
    DBG(L"\nNo %a\n", Buffer);
    return 0;
  }
  
  // got it. find closing >
  /*
   if (DbgCount < 3) {
   AsciiStrnCpy(Buffer, IntTag, sizeof(Buffer) - 1);
   DBG(L"Found: '%a'\n", Buffer);
   }
   */
  Value = AsciiStrStr(IntTag, ">");
  if (Value == NULL) {
    DBG(L"\nNo <integer end\n");
    return 0;
  }
  if (Value[-1] == '/') {
    DBG(L"\nInvalid <integer IDREF end\n");
    return 0;
  }
  
  // we should have value now
  NumValue = AsciiStrHexToUint64(Value + 1);
  
  /*
   if (DbgCount < 3) {
   AsciiStrnCpy(Buffer, IntTag, sizeof(Buffer) - 1);
   DBG(L"Found num: %x\n", NumValue);
   gBS->Stall(10000000);
   }
   DbgCount++;
   */
  
  return NumValue;
}

//
// Iterates over kexts in kernelcache
// and calls PatchKext() for each.
//
// PrelinkInfo section contains following plist, without spaces:
// <dict>
//   <key>_PrelinkInfoDictionary</key>
//   <array>
//     <!-- start of kext Info.plist -->
//     <dict>
//       <key>CFBundleName</key>
//       <string>MAC Framework Pseudoextension</string>
//       <key>_PrelinkExecutableLoadAddr</key>
//       <integer size="64">0xffffff7f8072f000</integer>
//       <!-- Kext size -->
//       <key>_PrelinkExecutableSize</key>
//       <integer size="64">0x3d0</integer>
//       <!-- Kext address -->
//       <key>_PrelinkExecutableSourceAddr</key>
//       <integer size="64">0xffffff80009a3000</integer>
//       ...
//     </dict>
//     <!-- start of next kext Info.plist -->
//     <dict>
//       ...
//     </dict>
//       ...
VOID PatchPrelinkedKexts(VOID)
{
  CHAR8     *WholePlist;
  CHAR8     *DictPtr;
  CHAR8     *InfoPlistStart = NULL;
  CHAR8     *InfoPlistEnd = NULL;
  INTN      DictLevel = 0;
  CHAR8     SavedValue;
  //INTN      DbgCount = 0;
  UINT32    KextAddr;
  UINT32    KextSize;
  
  
  WholePlist = (CHAR8*)(UINTN)PrelinkInfoAddr;
  
  //
  // Detect FakeSMC and if present then
  // disable kext injection InjectKexts().
  // There is some bug in the folowing code that
  // searches for individual kexts in prelink info
  // and FakeSMC is not found on my SnowLeo although
  // it is present in kernelcache.
  // But searching through the whole prelink info
  // works and that's the reason why it is here.
  //
  CheckForFakeSMC(WholePlist);
  
  DictPtr = WholePlist;
  while ((DictPtr = AsciiStrStr(DictPtr, "dict>")) != NULL) {
    
    if (DictPtr[-1] == '<') {
      // opening dict
      DictLevel++;
      if (DictLevel == 2) {
        // kext start
        InfoPlistStart = DictPtr - 1;
      }
      
    } else if (DictPtr[-2] == '<' && DictPtr[-1] == '/') {
      
      // closing dict
      if (DictLevel == 2 && InfoPlistStart != NULL) {
        // kext end
        InfoPlistEnd = DictPtr + 5 /* "dict>" */;
        
        // terminate Info.plist with 0
        SavedValue = *InfoPlistEnd;
        *InfoPlistEnd = '\0';
        
        // get kext address from _PrelinkExecutableSourceAddr
        // truncate to 32 bit to get physical addr
        KextAddr = (UINT32)GetPlistHexValue(InfoPlistStart, kPrelinkExecutableSourceKey, WholePlist);
        // KextAddr is always relative to 0x200000
        // and if KernelSlide is != 0 then KextAddr must be adjusted
        KextAddr += KernelSlide;
        // and adjust for AptioFixDrv's KernelRelocBase
        KextAddr += (UINT32)KernelRelocBase;
        
        KextSize = (UINT32)GetPlistHexValue(InfoPlistStart, kPrelinkExecutableSizeKey, WholePlist);
        
        /*if (DbgCount < 3
         || DbgCount == 100 || DbgCount == 101 || DbgCount == 102
         ) {
         DBG(L"\n\nKext: St = %x, Size = %x\n", KextAddr, KextSize);
         DBG(L"Info: St = %p, End = %p\n%a\n", InfoPlistStart, InfoPlistEnd, InfoPlistStart);
         gBS->Stall(20000000);
         }
         */
        
        // patch it
        PatchKext(
                  (UINT8*)(UINTN)KextAddr,
                  KextSize,
                  InfoPlistStart,
                  (UINT32)(InfoPlistEnd - InfoPlistStart)
                  );
        
        // return saved char
        *InfoPlistEnd = SavedValue;
        //DbgCount++;
      }
      
      DictLevel--;
      
    }
    DictPtr += 5;
  }
}

//
// Iterates over kexts loaded by booter
// and calls PatchKext() for each.
//
VOID PatchLoadedKexts(VOID)
{
	DTEntry             MMEntry;
  _BooterKextFileInfo *KextFileInfo;
  CHAR8               *PropName;
  _DeviceTreeBuffer   *PropEntry;
  CHAR8               SavedValue;
  CHAR8               *InfoPlist;
	struct OpaqueDTPropertyIterator OPropIter;
	DTPropertyIterator	PropIter = &OPropIter;
	//UINTN               DbgCount = 0;
  
  
  DBG(L"\nPatchLoadedKexts ... dtRoot = %p\n", dtRoot);
  
  if (!dtRoot) {
    return;
  }
  
  DTInit(dtRoot);
  
  if (DTLookupEntry(NULL,"/chosen/memory-map", &MMEntry) == kSuccess)
  {
    if (DTCreatePropertyIteratorNoAlloc(MMEntry, PropIter) == kSuccess)
    {
      while (DTIterateProperties(PropIter, &PropName) == kSuccess)
      {
        //DBG(L"Prop: %a\n", PropName);
        if (AsciiStrStr(PropName,"Driver-"))
        {
          // PropEntry _DeviceTreeBuffer is the value of Driver-XXXXXX property
          PropEntry = (_DeviceTreeBuffer*)(((UINT8*)PropIter->currentProperty) + sizeof(DeviceTreeNodeProperty));
          //if (DbgCount < 3) DBG(L"%a: paddr = %x, length = %x\n", PropName, PropEntry->paddr, PropEntry->length);
          
          // PropEntry->paddr points to _BooterKextFileInfo
          KextFileInfo = (_BooterKextFileInfo *)(UINTN)PropEntry->paddr;
          
          // Info.plist should be terminated with 0, but will also do it just in case
          InfoPlist = (CHAR8*)(UINTN)KextFileInfo->infoDictPhysAddr;
          SavedValue = InfoPlist[KextFileInfo->infoDictLength];
          InfoPlist[KextFileInfo->infoDictLength] = '\0';
          
          PatchKext(
                    (UINT8*)(UINTN)KextFileInfo->executablePhysAddr,
                    KextFileInfo->executableLength,
                    InfoPlist,
                    KextFileInfo->infoDictLength
                    );
          
          InfoPlist[KextFileInfo->infoDictLength] = SavedValue;
          //DbgCount++;
        }
        //if(AsciiStrStr(PropName,"DriversPackage-")!=0)
        //{
        //    DBG(L"Found %a\n", PropName);
        //    break;
        //}
      }
    }
  }
}

//
// Entry for all kext patches.
// Will iterate through kext in prelinked kernel (kernelcache)
// or DevTree (drivers boot) and do patches.
//
VOID KextPatcherStart(VOID)
{
  if (isKernelcache) {
    DBG_RT("Patching kernelcache ...\n");
    if (gSettings.KPDebug) {
      gBS->Stall(5000000);
    }
    PatchPrelinkedKexts();
    
  } else {
    
    DBG_RT("Patching loaded kexts ...\n");
    if (gSettings.KPDebug) {
      gBS->Stall(5000000);
    }
    PatchLoadedKexts();
    
  }
}

