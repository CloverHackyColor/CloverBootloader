/*
 * Copyright (c) 2011-2012 Frank Peng. All rights reserved.
 *
 */

#include "Platform.h"
#include "LoaderUefi.h"
//#include "device_tree.h"

#include "kernel_patcher.h"


#ifndef DEBUG_ALL
#define KEXT_DEBUG 0
#else
#define KEXT_DEBUG DEBUG_ALL
#endif

#if KEXT_DEBUG
#define DBG(...)	printf(__VA_ARGS__);
#else
#define DBG(...)
#endif

// runtime debug
#define DBG_RT(entry, ...)    if ((entry != NULL) && (entry->KernelAndKextPatches != NULL) && entry->KernelAndKextPatches->KPDebug) { printf(__VA_ARGS__); }

//
// Searches Source for Search pattern of size SearchSize
// and returns the number of occurences.
//
UINTN SearchAndCount(UINT8 *Source, UINT64 SourceSize, UINT8 *Search, UINTN SearchSize)
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
UINTN SearchAndReplace(UINT8 *Source, UINT64 SourceSize, UINT8 *Search, UINTN SearchSize, UINT8 *Replace, INTN MaxReplaces)
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

BOOLEAN CompareMemMask(UINT8 *Source, UINT8 *Search, UINT8 *Mask, UINTN SearchSize)
{
  UINT8 M;
 
  if (!Mask) {
    return !CompareMem(Source, Search, SearchSize);
  }
  for (UINTN Ind = 0; Ind < SearchSize; Ind++) {
    M = *Mask++;
    if ((*Source++ & M) != (*Search++ & M)) {
      return FALSE;
    }
  }
  return TRUE;
}

VOID CopyMemMask(UINT8 *Dest, UINT8 *Replace, UINT8 *Mask, UINTN SearchSize)
{
  UINT8 M, D;
  // the procedure is called from SearchAndReplaceMask with own check but for future it is better to check twice
  if (!Dest || !Replace) { 
    return;
  }

  if (!Mask) {
    CopyMem(Dest, Replace, SearchSize); //old behavior
    return;
  }
  for (UINTN Ind = 0; Ind < SearchSize; Ind++) {
    M = *Mask++;
    D = *Dest;
    *Dest++ = ((D ^ *Replace++) & M) ^ D;
  }
}

UINTN SearchAndReplaceMask(UINT8 *Source, UINT64 SourceSize, UINT8 *Search, UINT8 *MaskSearch, UINTN SearchSize,
                       UINT8 *Replace, UINT8 *MaskReplace, INTN MaxReplaces)
{
  UINTN     NumReplaces = 0;
  BOOLEAN   NoReplacesRestriction = MaxReplaces <= 0;
  UINT8     *End = Source + SourceSize;
  if (!Source || !Search || !Replace || !SearchSize) {
    return 0;
  }
  while ((Source < End) && (NoReplacesRestriction || (MaxReplaces > 0))) {
    if (CompareMemMask(Source, Search, MaskSearch, SearchSize)) {
      CopyMemMask(Source, Replace, MaskReplace, SearchSize);
      NumReplaces++;
      MaxReplaces--;
      Source += SearchSize;
    } else {
      Source++;
    }

  }

  return NumReplaces;
}


UINTN SearchAndReplaceTxt(UINT8 *Source, UINT64 SourceSize, UINT8 *Search, UINTN SearchSize, UINT8 *Replace, INTN MaxReplaces)
{
  UINTN     NumReplaces = 0;
  UINTN     Skip = 0;
  BOOLEAN   NoReplacesRestriction = MaxReplaces <= 0;
  UINT8     *End = Source + SourceSize;
  UINT8     *SearchEnd = Search + SearchSize;
  UINT8     *Pos = NULL;
  UINT8     *FirstMatch = Source;
  if (!Source || !Search || !Replace || !SearchSize) {
    return 0;
  }
  
  while (((Source + SearchSize) <= End) &&
         (NoReplacesRestriction || (MaxReplaces > 0))) { // num replaces
    while (*Source != '\0') {  //comparison
      Pos = Search;
      FirstMatch = Source;
      Skip = 0;
      while (*Source != '\0' && Pos != SearchEnd) {
        if (*Source <= 0x20) { //skip invisibles in sources
          Source++;
          Skip++;
          continue;
        }
        if (*Source != *Pos) {
          break;
        }
 //       printf("%c", *Source);
        Source++;
        Pos++;
      }
      
      if (Pos == SearchEnd) { // pattern found
        Pos = FirstMatch;
        break;
      }
      else
        Pos = NULL;
      
      Source = FirstMatch + 1;
/*      if (Pos != Search) {
        printf("\n");
      } */
      
    }

    if (!Pos) {
      break;
    }
    CopyMem (Pos, Replace, SearchSize);
    SetMem (Pos + SearchSize, Skip, 0x20); //fill skip places with spaces
    NumReplaces++;
    MaxReplaces--;
    Source = FirstMatch + SearchSize + Skip;
  }
  return NumReplaces;
}

/** Global for storing KextBundleIdentifier */
CHAR8 gKextBundleIdentifier[256];

/** Extracts kext BundleIdentifier from given Plist into gKextBundleIdentifier */
VOID ExtractKextBundleIdentifier(CHAR8 *Plist)
{
  CHAR8     *Tag;
  CHAR8     *BIStart;
  CHAR8     *BIEnd;
  INTN      DictLevel = 0;
  
  
  gKextBundleIdentifier[0] = '\0';
  
  // start with first <dict>
  Tag = AsciiStrStr(Plist, "<dict>");
  if (Tag == NULL) {
    return;
  }
  Tag += 6;
  DictLevel++;
  
  while (*Tag != '\0') {
    
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
        if (BIEnd != NULL && (UINTN)(BIEnd - BIStart + 1) < sizeof(gKextBundleIdentifier)) { // (UINTN)(BIEnd - BIStart + 1) = valid cast because BIEnd is > BIStart
          CopyMem(gKextBundleIdentifier, BIStart, BIEnd - BIStart);
          gKextBundleIdentifier[BIEnd - BIStart] = '\0';
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

BOOLEAN
isPatchNameMatch (CHAR8   *BundleIdentifier, CHAR8   *Name)
{
  BOOLEAN   isBundle = (AsciiStrStr(Name, ".") != NULL);
  return
    isBundle
      ? (AsciiStrCmp(BundleIdentifier, Name) == 0)
      : (AsciiStrStr(BundleIdentifier, Name) != NULL);
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
CHAR8 ATIKextBundleId[2][64];

//
// Inits patcher: prepares ATIKextBundleIds.
//
VOID ATIConnectorsPatchInit(LOADER_ENTRY *Entry)
{
  //
  // prepar boundle ids
  //
  
  // Lion, SnowLeo 10.6.7 2011 MBP
  snprintf(ATIKextBundleId[0],
              sizeof(ATIKextBundleId[0]),
		   "com.apple.kext.ATI%sController", // when it was AsciiSPrint, %a was used with KPATIConnectorsController which is CHAR16 ??? Result is printing stop at first char <= 255
           //now it is CHAR8*
              Entry->KernelAndKextPatches->KPATIConnectorsController
              );
  // ML
  snprintf(ATIKextBundleId[1],
              sizeof(ATIKextBundleId[1]),
		   "com.apple.kext.AMD%sController", // when it was AsciiSPrint, %a was used with KPATIConnectorsController which is CHAR16 ??? Result is printing stop at first char <= 255
              Entry->KernelAndKextPatches->KPATIConnectorsController
              );
  
  ATIConnectorsPatchInited = TRUE;
  
  //DBG(L"Bundle1: %s\n", ATIKextBundleId[0]);
  //DBG(L"Bundle2: %s\n", ATIKextBundleId[1]);
  //gBS->Stall(10000000);
}

//
// Registers kexts that need force-load during WithKexts boot.
//
VOID ATIConnectorsPatchRegisterKexts(FSINJECTION_PROTOCOL *FSInject, FSI_STRING_LIST *ForceLoadKexts, LOADER_ENTRY *Entry)
{
  
  // for future?
  FSInject->AddStringToList(ForceLoadKexts,
                            PoolPrint(L"\\AMD%aController.kext\\Contents\\Info.plist", Entry->KernelAndKextPatches->KPATIConnectorsController)
                            );
  // Lion, ML, SnowLeo 10.6.7 2011 MBP
  FSInject->AddStringToList(ForceLoadKexts,
                            PoolPrint(L"\\ATI%aController.kext\\Contents\\Info.plist", Entry->KernelAndKextPatches->KPATIConnectorsController)
                            );
  // SnowLeo
  FSInject->AddStringToList(ForceLoadKexts, L"\\ATIFramebuffer.kext\\Contents\\Info.plist");
  FSInject->AddStringToList(ForceLoadKexts, L"\\AMDFramebuffer.kext\\Contents\\Info.plist");
  
  // dependencies
  FSInject->AddStringToList(ForceLoadKexts, L"\\IOGraphicsFamily.kext\\Info.plist");
  FSInject->AddStringToList(ForceLoadKexts, L"\\ATISupport.kext\\Contents\\Info.plist");
  FSInject->AddStringToList(ForceLoadKexts, L"\\AMDSupport.kext\\Contents\\Info.plist");
  FSInject->AddStringToList(ForceLoadKexts, L"\\AppleGraphicsControl.kext\\Info.plist");
  FSInject->AddStringToList(ForceLoadKexts, L"\\AppleGraphicsControl.kext\\Contents\\PlugIns\\AppleGraphicsDeviceControl.kext\\Info.plist");
  //as well IOAcceleratorFamily2
}

//
// Patch function.
//
VOID ATIConnectorsPatch(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize, LOADER_ENTRY *Entry)
{
  
  UINTN   Num = 0;
  
	DBG_RT(Entry, "\nATIConnectorsPatch: driverAddr = %s, driverSize = %x\nController = %s\n",
         Driver, DriverSize, Entry->KernelAndKextPatches->KPATIConnectorsController);
  ExtractKextBundleIdentifier(InfoPlist);
	DBG_RT(Entry, "Kext: %s\n", gKextBundleIdentifier);
  
  // number of occurences od Data should be 1
  Num = SearchAndCount(Driver, DriverSize, Entry->KernelAndKextPatches->KPATIConnectorsData, Entry->KernelAndKextPatches->KPATIConnectorsDataLen);
  if (Num > 1) {
    // error message - shoud always be printed
	  printf("==> KPATIConnectorsData found %llu times in %s - skipping patching!\n", Num, gKextBundleIdentifier);
    gBS->Stall(5*1000000);
    return;
  }
  
  // patch
  Num = SearchAndReplace(Driver,
                         DriverSize,
                         Entry->KernelAndKextPatches->KPATIConnectorsData,
                         Entry->KernelAndKextPatches->KPATIConnectorsDataLen,
                         Entry->KernelAndKextPatches->KPATIConnectorsPatch,
                         1);
  if (Entry->KernelAndKextPatches->KPDebug) {
    if (Num > 0) {
		DBG_RT(Entry, "==> patched %llu times!\n", Num);
    } else {
      DBG_RT(Entry, "==> NOT patched!\n");
    }
    gBS->Stall(5000000);
  }
}



////////////////////////////////////
//
// AppleIntelCPUPM patch
//
// fLaked's SpeedStepper patch for Asus (and some other) boards:
// http://www.insanelymac.com/forum/index.php?showtopic=258611
//
// Credits: Samantha/RevoGirl/DHP
// http://www.insanelymac.com/forum/topic/253642-dsdt-for-asus-p8p67-m-pro/page__st__200#entry1681099
// Rehabman corrections 2014
//

STATIC UINT8   MovlE2ToEcx[] = { 0xB9, 0xE2, 0x00, 0x00, 0x00 };
STATIC UINT8   MovE2ToCx[]   = { 0x66, 0xB9, 0xE2, 0x00 };
STATIC UINT8   Wrmsr[]       = { 0x0F, 0x30 };

VOID AppleIntelCPUPMPatch(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize, LOADER_ENTRY *Entry)
{
  UINTN   Index1;
  UINTN   Index2;
  UINTN   Count = 0;

	DBG_RT(Entry, "\nAppleIntelCPUPMPatch: driverAddr = %s, driverSize = %x\n", Driver, DriverSize);
  if (Entry->KernelAndKextPatches->KPDebug) {
    ExtractKextBundleIdentifier(InfoPlist);
  }
	DBG_RT(Entry, "Kext: %s\n", gKextBundleIdentifier);

  //TODO: we should scan only __text __TEXT
  for (Index1 = 0; Index1 < DriverSize; Index1++) {
    // search for MovlE2ToEcx
    if (CompareMem(Driver + Index1, MovlE2ToEcx, sizeof(MovlE2ToEcx)) == 0) {
      // search for wrmsr in next few bytes
      for (Index2 = Index1 + sizeof(MovlE2ToEcx); Index2 < Index1 + sizeof(MovlE2ToEcx) + 32; Index2++) {
        if (Driver[Index2] == Wrmsr[0] && Driver[Index2 + 1] == Wrmsr[1]) {
          // found it - patch it with nops
          Count++;
          Driver[Index2] = 0x90;
          Driver[Index2 + 1] = 0x90;
			DBG_RT(Entry, " %llu. patched at 0x%llx\n", Count, Index2);
          break;
        } else if ((Driver[Index2] == 0xC9 && Driver[Index2 + 1] == 0xC3) ||
                   (Driver[Index2] == 0x5D && Driver[Index2 + 1] == 0xC3) ||
                   (Driver[Index2] == 0xB9 && Driver[Index2 + 3] == 0 && Driver[Index2 + 4] == 0) ||
                   (Driver[Index2] == 0x66 && Driver[Index2 + 1] == 0xB9 && Driver[Index2 + 3] == 0)) {
          // a leave/ret will cancel the search
          // so will an intervening "mov[l] $xx, [e]cx"
          break;
        }
      }
    } else if (CompareMem(Driver + Index1, MovE2ToCx, sizeof(MovE2ToCx)) == 0) {
      // search for wrmsr in next few bytes
      for (Index2 = Index1 + sizeof(MovE2ToCx); Index2 < Index1 + sizeof(MovE2ToCx) + 32; Index2++) {
        if (Driver[Index2] == Wrmsr[0] && Driver[Index2 + 1] == Wrmsr[1]) {
          // found it - patch it with nops
          Count++;
          Driver[Index2] = 0x90;
          Driver[Index2 + 1] = 0x90;
			DBG_RT(Entry, " %llu. patched at 0x%llx\n", Count, Index2);
          break;
        } else if ((Driver[Index2] == 0xC9 && Driver[Index2 + 1] == 0xC3) ||
                   (Driver[Index2] == 0x5D && Driver[Index2 + 1] == 0xC3) ||
                   (Driver[Index2] == 0xB9 && Driver[Index2 + 3] == 0 && Driver[Index2 + 4] == 0) ||
                   (Driver[Index2] == 0x66 && Driver[Index2 + 1] == 0xB9 && Driver[Index2 + 3] == 0)) {
          // a leave/ret will cancel the search
          // so will an intervening "mov[l] $xx, [e]cx"
          break;
        }
      }
    }
  }
	DBG_RT(Entry, "= %llu patches\n", Count);
  if (Entry->KernelAndKextPatches->KPDebug) {
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

STATIC UINT8   LionSearch_X64[]  = { 0x75, 0x30, 0x44, 0x89, 0xf8 };
STATIC UINT8   LionReplace_X64[] = { 0xeb, 0x30, 0x44, 0x89, 0xf8 };

STATIC UINT8   LionSearch_i386[]  = { 0x75, 0x3d, 0x8b, 0x75, 0x08 };
STATIC UINT8   LionReplace_i386[] = { 0xeb, 0x3d, 0x8b, 0x75, 0x08 };

STATIC UINT8   MLSearch[]  = { 0x75, 0x30, 0x89, 0xd8 };
STATIC UINT8   MLReplace[] = { 0xeb, 0x30, 0x89, 0xd8 };

// SunKi: 10.9 - 10.14.3
STATIC UINT8   MavMoj3Search[]  = { 0x75, 0x2e, 0x0f, 0xb6 };
STATIC UINT8   MavMoj3Replace[] = { 0xeb, 0x2e, 0x0f, 0xb6 };

// RodionS: 10.14.4+ / 10.15 DB1
STATIC UINT8   Moj4CataSearch[]  = { 0x75, 0x33, 0x0f, 0xb7 };
STATIC UINT8   Moj4CataReplace[] = { 0xeb, 0x33, 0x0f, 0xb7 };

//
// We can not rely on OSVersion global variable for OS version detection,
// since in some cases it is not correct (install of ML from Lion, for example).
// So, we'll use "brute-force" method - just try to patch.
// Actually, we'll at least check that if we can find only one instance of code that
// we are planning to patch.
//

VOID AppleRTCPatch(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize, LOADER_ENTRY *Entry)
{
  
  UINTN   Num = 0;
  UINTN   NumLion_X64 = 0;
  UINTN   NumLion_i386 = 0;
  UINTN   NumML = 0;
  UINTN   NumMavMoj3 = 0;
  UINTN   NumMoj4 = 0;
  
	DBG_RT(Entry, "\nAppleRTCPatch: driverAddr = %s, driverSize = %x\n", Driver, DriverSize);
  if (Entry->KernelAndKextPatches->KPDebug) {
    ExtractKextBundleIdentifier(InfoPlist);
  }
	DBG_RT(Entry, "Kext: %s\n", gKextBundleIdentifier);
  
  if (is64BitKernel) {
    NumLion_X64 = SearchAndCount(Driver, DriverSize, LionSearch_X64, sizeof(LionSearch_X64));
    NumML  = SearchAndCount(Driver, DriverSize, MLSearch,  sizeof(MLSearch));
    NumMavMoj3 = SearchAndCount(Driver, DriverSize, MavMoj3Search, sizeof(MavMoj3Search));
    NumMoj4 = SearchAndCount(Driver, DriverSize, Moj4CataSearch, sizeof(Moj4CataSearch));
  } else {
    NumLion_i386 = SearchAndCount(Driver, DriverSize, LionSearch_i386, sizeof(LionSearch_i386));
  }
  
  if (NumLion_X64 + NumLion_i386 + NumML + NumMavMoj3 + NumMoj4 > 1) {
    // more then one pattern found - we do not know what to do with it
    // and we'll skip it
	  printf("AppleRTCPatch: ERROR: multiple patterns found (LionX64: %llu, Lioni386: %llu, ML: %llu, MavMoj3: %llu, Moj4: %llu) - skipping patching!\n",
          NumLion_X64, NumLion_i386, NumML, NumMavMoj3, NumMoj4);
    gBS->Stall(5000000);
    return;
  }
  
  if (NumLion_X64 == 1) {
    Num = SearchAndReplace(Driver, DriverSize, LionSearch_X64, sizeof(LionSearch_X64), LionReplace_X64, 1);
	  DBG_RT(Entry, "==> Lion X64: %llu replaces done.\n", Num);
  } else if (NumLion_i386 == 1) {
    Num = SearchAndReplace(Driver, DriverSize, LionSearch_i386, sizeof(LionSearch_i386), LionReplace_i386, 1);
	  DBG_RT(Entry, "==> Lion i386: %llu replaces done.\n", Num);
  } else if (NumML == 1) {
    Num = SearchAndReplace(Driver, DriverSize, MLSearch, sizeof(MLSearch), MLReplace, 1);
	  DBG_RT(Entry, "==> MountainLion X64: %llu replaces done.\n", Num);
  } else if (NumMavMoj3 == 1) {
    Num = SearchAndReplace(Driver, DriverSize, MavMoj3Search, sizeof(MavMoj3Search), MavMoj3Replace, 1);
	  DBG_RT(Entry, "==> Mav/Yos/El/Sie/HS/Moj3 X64: %llu replaces done.\n", Num);
  } else if (NumMoj4 == 1) {
    Num = SearchAndReplace(Driver, DriverSize, Moj4CataSearch, sizeof(Moj4CataSearch), Moj4CataReplace, 1);
	  DBG_RT(Entry, "==> Mojave4 X64: %llu replaces done.\n", Num);
  } else {
    DBG_RT(Entry, "==> Patterns not found - patching NOT done.\n");
  }
  if (Entry->KernelAndKextPatches->KPDebug) {
    gBS->Stall(5000000);
  }
}



///////////////////////////////////
//
// InjectKexts if no FakeSMC: Detect FakeSMC and if present then
// disable kext injection InjectKexts()
//
// not used since 4242
VOID CheckForFakeSMC(CHAR8 *InfoPlist, LOADER_ENTRY *Entry)
{
  if (OSFLAG_ISSET(Entry->Flags, OSFLAG_CHECKFAKESMC) &&
      OSFLAG_ISSET(Entry->Flags, OSFLAG_WITHKEXTS)) {
    if (AsciiStrStr(InfoPlist, "<string>org.netkas.driver.FakeSMC</string>") != NULL
        || AsciiStrStr(InfoPlist, "<string>org.netkas.FakeSMC</string>") != NULL
        || AsciiStrStr(InfoPlist, "<string>as.vit9696.VirtualSMC</string>") != NULL)
    {
      Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_WITHKEXTS);
      if (Entry->KernelAndKextPatches->KPDebug) {
        DBG_RT(Entry, "\nFakeSMC or VirtualSMC found, UNSET WITHKEXTS\n");
        gBS->Stall(5000000);
      }
    }
  }
}



////////////////////////////////////
//
// Dell SMBIOS Patch by syscl
//
// Remap SMBIOS Table 1 for  both AppleSMBIOS and AppleACPIPlatform
//
// EB9D2D31-2D88-11D3-9A16-0090273F -> EB9D2D35-2D88-11D3-9A16-0090273F
//
STATIC UINT8   DELL_SMBIOS_GUID_Search[]  = { 0x45, 0x42, 0x39, 0x44, 0x32, 0x44, 0x33, 0x31 };
STATIC UINT8   DELL_SMBIOS_GUID_Replace[] = { 0x45, 0x42, 0x39, 0x44, 0x32, 0x44, 0x33, 0x35 };

//
// EB9D2D31-2D88-11D3-9A16-0090273F is the standard SMBIOS Table Type 1 for
// all computers even though Apple.Inc should obey the rule
// that's why we can be so confident to write patch pattern this way - syscl
//
VOID DellSMBIOSPatch(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize, LOADER_ENTRY *Entry)
{
    //
    // syscl
    // Note, smbios truncate issue only affects Broadwell platform and platform
    // later than Broadwell thus we don't need to consider OS versinos earlier
    // than Yosemite, they are all pure 64bit platforms
    //
    UINTN gPatchCount = 0;
    
	DBG_RT(Entry, "\nDellSMBIOSPatch: driverAddr = %s, driverSize = %x\n", Driver, DriverSize);
    if (Entry->KernelAndKextPatches->KPDebug)
    {
        ExtractKextBundleIdentifier(InfoPlist);
    }
	DBG_RT(Entry, "Kext: %s\n", gKextBundleIdentifier);
    
    //
    // now, let's patch it!
    //
    gPatchCount = SearchAndReplace(Driver, DriverSize, DELL_SMBIOS_GUID_Search, sizeof(DELL_SMBIOS_GUID_Search), DELL_SMBIOS_GUID_Replace, 1);
    
    if (gPatchCount == 1)
    {
		DBG_RT(Entry, "==> AppleSMBIOS: %llu replaces done.\n", gPatchCount);
    }
    else
    {
        DBG_RT(Entry, "==> Patterns not found - patching NOT done.\n");
    }

    if (Entry->KernelAndKextPatches->KPDebug)
    {
        gBS->Stall(5000000);
    }
}



////////////////////////////////////
//
// SNBE_AICPUPatch implemented by syscl
// Fix AppleIntelCPUPowerManagement on SandyBridge-E (c) omni, stinga11
//
VOID SNBE_AICPUPatch(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize, LOADER_ENTRY *Entry)
{
    UINT32 i;
    UINT64 os_ver = AsciiOSVersionToUint64(Entry->OSVersion);
    
	DBG_RT(Entry, "\nSNBE_AICPUPatch: driverAddr = %s, driverSize = %x\n", Driver, DriverSize);
    if (Entry->KernelAndKextPatches->KPDebug) {
        ExtractKextBundleIdentifier(InfoPlist);
    }
    
	DBG_RT(Entry, "Kext: %s\n", gKextBundleIdentifier);
    
    // now let's patch it
    if (os_ver < AsciiOSVersionToUint64("10.9") || os_ver >= AsciiOSVersionToUint64("10.14")) {
        DBG("Unsupported macOS.\nSandyBridge-E requires macOS 10.9 - 10.13.x, aborted\n");
        DBG("SNBE_AICPUPatch() <===FALSE\n");
        return;
    }
    
    if (os_ver < AsciiOSVersionToUint64("10.10")) {
        // 10.9.x
        STATIC UINT8 find[][3] = {
            { 0x84, 0x2F, 0x01 },
            { 0x3E, 0x75, 0x3A },
            { 0x84, 0x5F, 0x01 },
            { 0x74, 0x10, 0xB9 },
            { 0x75, 0x07, 0xB9 },
            { 0xFC, 0x02, 0x74 },
            { 0x01, 0x74, 0x58 }
        };
        STATIC UINT8 repl[][3] = {
            { 0x85, 0x2F, 0x01 },
            { 0x3E, 0x90, 0x90 },
            { 0x85, 0x5F, 0x01 },
            { 0xEB, 0x10, 0xB9 },
            { 0xEB, 0x07, 0xB9 },
            { 0xFC, 0x02, 0xEB },
            { 0x01, 0xEB, 0x58 }
        };
        
        for (i = 0; i < 7; i++) {
            if (SearchAndReplace(Driver, DriverSize, find[i], sizeof(find[i]), repl[i], 0)) {
                DBG("SNBE_AICPUPatch (%d/7) applied\n", i);
            } else {
                DBG("SNBE_AICPUPatch (%d/7) not apply\n", i);
            }
        }
    } else if (os_ver < AsciiOSVersionToUint64("10.11")) {
        // 10.10.x
        STATIC UINT8 find[][3] = {
            { 0x3E, 0x75, 0x39 },
            { 0x74, 0x11, 0xB9 },
            { 0x01, 0x74, 0x56 }
        };
        STATIC UINT8 repl[][3] = {
            { 0x3E, 0x90, 0x90 },
            { 0xEB, 0x11, 0xB9 },
            { 0x01, 0xEB, 0x56 }
        };
        for (i = 0; i < 3; i++) {
            if (SearchAndReplace(Driver, DriverSize, find[i], sizeof(find[i]), repl[i], 0)) {
                DBG("SNBE_AICPUPatch (%d/7) applied\n", i);
            } else {
                DBG("SNBE_AICPUPatch (%d/7) not apply\n", i);
            }
        }
        
        STATIC UINT8 find_1[] = { 0xFF, 0x0F, 0x84, 0x2D };
        STATIC UINT8 repl_1[] = { 0xFF, 0x0F, 0x85, 0x2D };
        if (SearchAndReplace(Driver, DriverSize, find_1, sizeof(find_1), repl_1, 0)) {
            DBG("SNBE_AICPUPatch (4/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (4/7) not apply\n");
        }
        
        
        STATIC UINT8 find_2[] = { 0x01, 0x00, 0x01, 0x0F, 0x84 };
        STATIC UINT8 repl_2[] = { 0x01, 0x00, 0x01, 0x0F, 0x85 };
        if (SearchAndReplace(Driver, DriverSize, find_2, sizeof(find_2), repl_2, 0)) {
            DBG("SNBE_AICPUPatch (5/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (5/7) not apply\n");
        }
        
        STATIC UINT8 find_3[] = { 0x02, 0x74, 0x0B, 0x41, 0x83, 0xFC, 0x03, 0x75, 0x22, 0xB9, 0x02, 0x06 };
        STATIC UINT8 repl_3[] = { 0x02, 0xEB, 0x0B, 0x41, 0x83, 0xFC, 0x03, 0x75, 0x22, 0xB9, 0x02, 0x06 };
        if (SearchAndReplace(Driver, DriverSize, find_3, sizeof(find_3), repl_3, 0)) {
            DBG("SNBE_AICPUPatch (6/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (6/7) not apply\n");
        }
        
        STATIC UINT8 find_4[] = { 0x74, 0x0B, 0x41, 0x83, 0xFC, 0x03, 0x75, 0x11, 0xB9, 0x42, 0x06, 0x00 };
        STATIC UINT8 repl_4[] = { 0xEB, 0x0B, 0x41, 0x83, 0xFC, 0x03, 0x75, 0x11, 0xB9, 0x42, 0x06, 0x00 };
        if (SearchAndReplace(Driver, DriverSize, find_4, sizeof(find_4), repl_4, 0)) {
            DBG("SNBE_AICPUPatch (7/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (7/7) not apply\n");
        }
    } else if (os_ver < AsciiOSVersionToUint64("10.12")) {
        // 10.11
        STATIC UINT8 find[][3] = {
            { 0x3E, 0x75, 0x39 },
            { 0x75, 0x11, 0xB9 },
            { 0x01, 0x74, 0x5F }
        };
        STATIC UINT8 repl[][3] = {
            { 0x3E, 0x90, 0x90 },
            { 0xEB, 0x11, 0xB9 },
            { 0x01, 0xEB, 0x5F }
        };
        for (i = 0; i < 3; i++) {
            if (SearchAndReplace(Driver, DriverSize, find[i], sizeof(find[i]), repl[i], 0)) {
                DBG("SNBE_AICPUPatch (%d/7) applied\n", i);
            } else {
                DBG("SNBE_AICPUPatch (%d/7) not apply\n", i);
            }
        }
        
        STATIC UINT8 find_1[] = { 0xFF, 0x0F, 0x84, 0x2D };
        STATIC UINT8 repl_1[] = { 0xFF, 0x0F, 0x85, 0x2D };
        if (SearchAndReplace(Driver, DriverSize, find_1, sizeof(find_1), repl_1, 0)) {
            DBG("SNBE_AICPUPatch (4/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (4/7) not apply\n");
        }
        
        STATIC UINT8 find_2[] = { 0x01, 0x00, 0x01, 0x0F, 0x84 };
        STATIC UINT8 repl_2[] = { 0x01, 0x00, 0x01, 0x0F, 0x85 };
        if (SearchAndReplace(Driver, DriverSize, find_2, sizeof(find_2), repl_2, 0)) {
            DBG("SNBE_AICPUPatch (5/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (5/7) not apply\n");
        }
        
        STATIC UINT8 find_3[] = { 0xC9, 0x74, 0x16, 0x0F, 0x32, 0x48, 0x25, 0xFF, 0x0F, 0x00, 0x00, 0x48 };
        STATIC UINT8 repl_3[] = { 0xC9, 0xEB, 0x16, 0x0F, 0x32, 0x48, 0x25, 0xFF, 0x0F, 0x00, 0x00, 0x48 };
        if (SearchAndReplace(Driver, DriverSize, find_3, sizeof(find_3), repl_3, 0)) {
            DBG("SNBE_AICPUPatch (6/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (6/7) not apply\n");
        }
        
        STATIC UINT8 find_4[] = { 0xC9, 0x74, 0x0C, 0x0F, 0x32, 0x83, 0xE0, 0x1F, 0x42, 0x89, 0x44, 0x3B };
        STATIC UINT8 repl_4[] = { 0xC9, 0xEB, 0x0C, 0x0F, 0x32, 0x83, 0xE0, 0x1F, 0x42, 0x89, 0x44, 0x3B };
        if (SearchAndReplace(Driver, DriverSize, find_4, sizeof(find_4), repl_4, 0)) {
            DBG("SNBE_AICPUPatch (7/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (7/7) not apply\n");
        }
    } else if (os_ver < AsciiOSVersionToUint64("10.13")) {
        // 10.12
        STATIC UINT8 find[][3] = {
            { 0x01, 0x74, 0x61 },
            { 0x3E, 0x75, 0x38 },
            { 0x75, 0x11, 0xB9 }
        };
        STATIC UINT8 repl[][3] = {
            { 0x01, 0xEB, 0x61 },
            { 0x3E, 0x90, 0x90 },
            { 0xEB, 0x11, 0xB9 }
        };
        for (i = 0; i < 3; i++) {
            if (SearchAndReplace(Driver, DriverSize, find[i], sizeof(find[i]), repl[i], 0)) {
                DBG("SNBE_AICPUPatch (%d/7) applied\n", i);
            } else {
                DBG("SNBE_AICPUPatch (%d/7) not apply\n", i);
            }
        }
        
        STATIC UINT8 find_1[] = { 0xFF, 0x0F, 0x84, 0x2D };
        STATIC UINT8 repl_1[] = { 0xFF, 0x0F, 0x85, 0x2D };
        if (SearchAndReplace(Driver, DriverSize, find_1, sizeof(find_1), repl_1, 0)) {
            DBG("SNBE_AICPUPatch (4/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (4/7) not apply\n");
        }

        STATIC UINT8 find_2[] = { 0x01, 0x00, 0x01, 0x0F, 0x84 };
        STATIC UINT8 repl_2[] = { 0x01, 0x00, 0x01, 0x0F, 0x85 };
        if (SearchAndReplace(Driver, DriverSize, find_2, sizeof(find_2), repl_2, 0)) {
            DBG("SNBE_AICPUPatch (5/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (5/7) not apply\n");
        }
        
        STATIC UINT8 find_3[] = { 0xC9, 0x74, 0x15, 0x0F, 0x32, 0x25, 0xFF, 0x0F, 0x00, 0x00, 0x48 };
        STATIC UINT8 repl_3[] = { 0xC9, 0xEB, 0x15, 0x0F, 0x32, 0x25, 0xFF, 0x0F, 0x00, 0x00, 0x48 };
        if (SearchAndReplace(Driver, DriverSize, find_3, sizeof(find_3), repl_3, 0)) {
            DBG("SNBE_AICPUPatch (6/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (6/7) not apply\n");
        }

        STATIC UINT8 find_4[] = { 0xC9, 0x74, 0x0C, 0x0F, 0x32, 0x83, 0xE0, 0x1F, 0x42, 0x89, 0x44, 0x3B };
        STATIC UINT8 repl_4[] = { 0xC9, 0xEB, 0x0C, 0x0F, 0x32, 0x83, 0xE0, 0x1F, 0x42, 0x89, 0x44, 0x3B };
        if (SearchAndReplace(Driver, DriverSize, find_4, sizeof(find_4), repl_4, 0)) {
            DBG("SNBE_AICPUPatch (7/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (7/7) not apply\n");
        }
    } else if (os_ver < AsciiOSVersionToUint64("10.15")) {
        // 10.13/10.14
        STATIC UINT8 find[][3] = {
            { 0x01, 0x74, 0x61 },
            { 0x3E, 0x75, 0x38 },
            { 0x75, 0x11, 0xB9 }
        };
        STATIC UINT8 repl[][3] = {
            { 0x01, 0xEB, 0x61 },
            { 0x3E, 0x90, 0x90 },
            { 0xEB, 0x11, 0xB9 }
        };
        for (i = 0; i < 3; i++) {
            if (SearchAndReplace(Driver, DriverSize, find[i], sizeof(find[i]), repl[i], 0)) {
                DBG("SNBE_AICPUPatch (%d/7) applied\n", i);
            } else {
                DBG("SNBE_AICPUPatch (%d/7) not apply\n", i);
            }
        }
        
        STATIC UINT8 find_1[] = { 0xFF, 0x0F, 0x84, 0xD3 };
        STATIC UINT8 repl_1[] = { 0xFF, 0x0F, 0x85, 0xD3 };
        if (SearchAndReplace(Driver, DriverSize, find_1, sizeof(find_1), repl_1, 0)) {
            DBG("SNBE_AICPUPatch (4/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (4/7) not apply\n");
        }
        
        STATIC UINT8 find_2[] = { 0x01, 0x00, 0x01, 0x0F, 0x84 };
        STATIC UINT8 repl_2[] = { 0x01, 0x00, 0x01, 0x0F, 0x85 };
        if (SearchAndReplace(Driver, DriverSize, find_2, sizeof(find_2), repl_2, 0)) {
            DBG("SNBE_AICPUPatch (5/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (5/7) not apply\n");
        }
        
        STATIC UINT8 find_3[] = { 0xC9, 0x74, 0x14, 0x0F, 0x32, 0x25, 0xFF, 0x0F, 0x00, 0x00, 0x6B };
        STATIC UINT8 repl_3[] = { 0xC9, 0xEB, 0x14, 0x0F, 0x32, 0x25, 0xFF, 0x0F, 0x00, 0x00, 0x6B};
        if (SearchAndReplace(Driver, DriverSize, find_3, sizeof(find_3), repl_3, 0)) {
            DBG("SNBE_AICPUPatch (6/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (6/7) not apply\n");
        }
        
        STATIC UINT8 find_4[] = { 0xC9, 0x74, 0x0C, 0x0F, 0x32, 0x83, 0xE0, 0x1F, 0x42, 0x89, 0x44, 0x3B };
        STATIC UINT8 repl_4[] = { 0xC9, 0xEB, 0x0C, 0x0F, 0x32, 0x83, 0xE0, 0x1F, 0x42, 0x89, 0x44, 0x3B };
        if (SearchAndReplace(Driver, DriverSize, find_4, sizeof(find_4), repl_4, 0)) {
            DBG("SNBE_AICPUPatch (7/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (7/7) not apply\n");
        }
    }
    
    if (Entry->KernelAndKextPatches->KPDebug) {
        gBS->Stall(5000000);
    }
}


////////////////////////////////////
//
// BDWE_IOPCIPatch implemented by syscl
// Fix Broadwell-E IOPCIFamily issue
//

// El Capitan
STATIC UINT8   BroadwellE_IOPCI_Find_El[] = { 0x48, 0x81, 0xF9, 0x01, 0x00, 0x00, 0x40 };
STATIC UINT8   BroadwellE_IOPCI_Repl_El[] = { 0x48, 0x81, 0xF9, 0x01, 0x00, 0x00, 0x80 };

// Sierra/High Sierra
STATIC UINT8   BroadwellE_IOPCI_Find_SieHS[] = { 0x48, 0x81, 0xFB, 0x00, 0x00, 0x00, 0x40 };
STATIC UINT8   BroadwellE_IOPCI_Repl_SieHS[] = { 0x48, 0x81, 0xFB, 0x00, 0x00, 0x00, 0x80 };

// Mojave
STATIC UINT8   BroadwellE_IOPCI_Find_MojCata[] = { 0x48, 0x3D, 0x00, 0x00, 0x00, 0x40 };
STATIC UINT8   BroadwellE_IOPCI_Repl_MojCata[] = { 0x48, 0x3D, 0x00, 0x00, 0x00, 0x80 };

VOID BDWE_IOPCIPatch(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize, LOADER_ENTRY *Entry)
{
  UINTN count = 0;
  UINT64 os_ver = AsciiOSVersionToUint64(Entry->OSVersion);
    
	DBG_RT(Entry, "\nBDWE_IOPCIPatch: driverAddr = %s, driverSize = %x\n", Driver, DriverSize);
  if (Entry->KernelAndKextPatches->KPDebug) {
    ExtractKextBundleIdentifier(InfoPlist);
  }
    
	DBG_RT(Entry, "Kext: %s\n", gKextBundleIdentifier);
  //
  // now, let's patch it!
  //

  if (os_ver < AsciiOSVersionToUint64("10.12")) {
    count = SearchAndReplace(Driver, DriverSize, BroadwellE_IOPCI_Find_El, sizeof(BroadwellE_IOPCI_Find_El), BroadwellE_IOPCI_Repl_El, 0);
  } else if (os_ver < AsciiOSVersionToUint64("10.14")) {
    count = SearchAndReplace(Driver, DriverSize, BroadwellE_IOPCI_Find_SieHS, sizeof(BroadwellE_IOPCI_Find_SieHS), BroadwellE_IOPCI_Repl_SieHS, 0);
  } else {
    count = SearchAndReplace(Driver, DriverSize, BroadwellE_IOPCI_Find_MojCata, sizeof(BroadwellE_IOPCI_Find_MojCata), BroadwellE_IOPCI_Repl_MojCata, 0);
  }
  
  if (count) {
	  DBG_RT(Entry, "==> IOPCIFamily: %llu replaces done.\n", count);
  } else {
    DBG_RT(Entry, "==> Patterns not found - patching NOT done.\n");
  }
    
  if (Entry->KernelAndKextPatches->KPDebug) {
    gBS->Stall(5000000);
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
VOID AnyKextPatch(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize, INT32 N, LOADER_ENTRY *Entry)
{
  UINTN   Num = 0;
  INTN    Ind;
  
	DBG_RT(Entry, "\nAnyKextPatch %d: driverAddr = %s, driverSize = %x\nAnyKext = %s\n",
         N, Driver, DriverSize, Entry->KernelAndKextPatches->KextPatches[N].Label);

  if (!Entry->KernelAndKextPatches->KextPatches[N].MenuItem.BValue) {
    DBG_RT(Entry, "==> DISABLED!\n");
    return;
  }
  
  if (!Entry->KernelAndKextPatches->KextPatches[N].SearchLen ||
      (Entry->KernelAndKextPatches->KextPatches[N].SearchLen > DriverSize)) {
    Entry->KernelAndKextPatches->KextPatches[N].SearchLen = DriverSize;
  }

  if (Entry->KernelAndKextPatches->KPDebug) {
    ExtractKextBundleIdentifier(InfoPlist);
  }

	DBG_RT(Entry, "Kext: %s\n", gKextBundleIdentifier);

  if (!Entry->KernelAndKextPatches->KextPatches[N].IsPlistPatch) {
    // kext binary patch
    DBG_RT(Entry, "Binary patch\n");
    bool once = false;
    UINTN procLen = 0;
    UINTN procAddr = searchProc(Driver, DriverSize,
                                 Entry->KernelAndKextPatches->KextPatches[N].ProcedureName, &procLen);
    
    if (Entry->KernelAndKextPatches->KextPatches[N].SearchLen == 0) {
      Entry->KernelAndKextPatches->KextPatches[N].SearchLen = DriverSize;
      if (procLen > DriverSize) {
        procLen = DriverSize - procAddr;
        once = true;
      }
    } else {
      procLen = Entry->KernelAndKextPatches->KextPatches[N].SearchLen;
    }
    UINT8 * curs = &Driver[procAddr];
    UINTN j = 0;
    while (j < DriverSize) {
      if (!Entry->KernelAndKextPatches->KextPatches[N].StartPattern || //old behavior
          CompareMemMask(curs,
                         Entry->KernelAndKextPatches->KextPatches[N].StartPattern,
                         Entry->KernelAndKextPatches->KextPatches[N].StartMask,
                         Entry->KernelAndKextPatches->KextPatches[N].StartPatternLen)) {
        DBG_RT(Entry, " StartPattern found\n");

        Num = SearchAndReplaceMask(Driver,
                                   procLen,
                                   Entry->KernelAndKextPatches->KextPatches[N].Data,
                                   Entry->KernelAndKextPatches->KextPatches[N].MaskFind,
                                   Entry->KernelAndKextPatches->KextPatches[N].DataLen,
                                   Entry->KernelAndKextPatches->KextPatches[N].Patch,
                                   Entry->KernelAndKextPatches->KextPatches[N].MaskReplace,
                                   -1);
        if (Num) {
          curs += Entry->KernelAndKextPatches->KextPatches[N].SearchLen - 1;
          j    += Entry->KernelAndKextPatches->KextPatches[N].SearchLen - 1;
        }
      }
      if (once ||
          !Entry->KernelAndKextPatches->KextPatches[N].StartPattern ||
          !Entry->KernelAndKextPatches->KextPatches[N].StartPatternLen) {
        break;
      }
      j++; curs++;
    }
  } else {
    // Info plist patch
    DBG_RT(Entry, "Info.plist data : '");
    for (Ind = 0; Ind < Entry->KernelAndKextPatches->KextPatches[N].DataLen; Ind++) {
      DBG_RT(Entry, "%c", Entry->KernelAndKextPatches->KextPatches[N].Data[Ind]);
    }
    DBG_RT(Entry, "' ->\n");
    DBG_RT(Entry, "Info.plist patch: '");
    for (Ind = 0; Ind < Entry->KernelAndKextPatches->KextPatches[N].DataLen; Ind++) {
      DBG_RT(Entry, "%c", Entry->KernelAndKextPatches->KextPatches[N].Patch[Ind]);
    }
    DBG_RT(Entry, "' \n");
    
    Num = SearchAndReplaceTxt((UINT8*)InfoPlist,
                           InfoPlistSize,
                           Entry->KernelAndKextPatches->KextPatches[N].Data,
                           Entry->KernelAndKextPatches->KextPatches[N].DataLen,
                           Entry->KernelAndKextPatches->KextPatches[N].Patch,
                           -1);
  }
  
  if (Entry->KernelAndKextPatches->KPDebug) {
    if (Num > 0) {
		DBG_RT(Entry, "==> patched %llu times!\n", Num);
    } else {
      DBG_RT(Entry, "==> NOT patched!\n");
    }
    gBS->Stall(2000000);
  }
}

//
// Called from SetFSInjection(), before boot.efi is started,
// to allow patchers to prepare FSInject to force load needed kexts.
//
VOID KextPatcherRegisterKexts(FSINJECTION_PROTOCOL *FSInject, FSI_STRING_LIST *ForceLoadKexts, LOADER_ENTRY *Entry)
{
  INTN i;
  
  if (Entry->KernelAndKextPatches->KPATIConnectorsController != NULL) {
    ATIConnectorsPatchRegisterKexts(FSInject, ForceLoadKexts, Entry);
  }
  
  for (i = 0; i < Entry->KernelAndKextPatches->NrKexts; i++) {
    FSInject->AddStringToList(ForceLoadKexts,
                              PoolPrint(L"\\%a.kext\\Contents\\Info.plist",
                                        Entry->KernelAndKextPatches->KextPatches[i].Name) );
  }
  
}

//
// PatchKext is called for every kext from prelinked kernel (kernelcache) or from DevTree (booting with drivers).
// Add kext detection code here and call kext specific patch function.
//
VOID PatchKext(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize, LOADER_ENTRY *Entry)
{
  INT32 i;
  
  if (Entry->KernelAndKextPatches->KPATIConnectorsController != NULL) {
    //
    // ATIConnectors
    //
    if (!ATIConnectorsPatchInited) {
      ATIConnectorsPatchInit(Entry);
    }
    if (   AsciiStrStr(InfoPlist, ATIKextBundleId[0]) != NULL  // ATI boundle id
        || AsciiStrStr(InfoPlist, ATIKextBundleId[1]) != NULL  // AMD boundle id
        || AsciiStrStr(InfoPlist, "com.apple.kext.ATIFramebuffer") != NULL // SnowLeo
        || AsciiStrStr(InfoPlist, "com.apple.kext.AMDFramebuffer") != NULL //Maverics
        ) {
      ATIConnectorsPatch(Driver, DriverSize, InfoPlist, InfoPlistSize, Entry);
      return;
    }
  }
  
  ExtractKextBundleIdentifier(InfoPlist);
  
  if (Entry->KernelAndKextPatches->KPAppleIntelCPUPM &&
      (AsciiStrStr(InfoPlist,
                   "<string>com.apple.driver.AppleIntelCPUPowerManagement</string>") != NULL)) {
    //
    // AppleIntelCPUPM
    //
    AppleIntelCPUPMPatch(Driver, DriverSize, InfoPlist, InfoPlistSize, Entry);
  } else if (Entry->KernelAndKextPatches->KPAppleRTC &&
             (AsciiStrStr(InfoPlist, "com.apple.driver.AppleRTC") != NULL)) {
    //
    // AppleRTC
    //
    AppleRTCPatch(Driver, DriverSize, InfoPlist, InfoPlistSize, Entry);
  } else if (Entry->KernelAndKextPatches->KPDELLSMBIOS &&
           (AsciiStrStr(InfoPlist, "com.apple.driver.AppleSMBIOS") != NULL)) {
    //
    // DellSMBIOSPatch
    //
    DBG_RT(Entry, "Remap SMBIOS Table require, AppleSMBIOS...\n");
    DellSMBIOSPatch(Driver, DriverSize, InfoPlist, InfoPlistSize, Entry);
  } else if (Entry->KernelAndKextPatches->KPDELLSMBIOS &&
             (AsciiStrStr(InfoPlist, "com.apple.driver.AppleACPIPlatform") != NULL)) {
    //
    // DellSMBIOS
    //
    // AppleACPIPlatform
    //
    DellSMBIOSPatch(Driver, DriverSize, InfoPlist, InfoPlistSize, Entry);
  } else if (gBDWEIOPCIFixRequire && (AsciiStrStr(InfoPlist, "com.apple.iokit.IOPCIFamily") != NULL)) {
    //
    // Braodwell-E IOPCIFamily Patch
    //
    BDWE_IOPCIPatch(Driver, DriverSize, InfoPlist, InfoPlistSize, Entry);
  } else if (gSNBEAICPUFixRequire && (AsciiStrStr(InfoPlist, "com.apple.driver.AppleIntelCPUPowerManagement") != NULL)) {
    //
    // SandyBridge-E AppleIntelCPUPowerManagement Patch implemented by syscl
    //
    SNBE_AICPUPatch(Driver, DriverSize, InfoPlist, InfoPlistSize, Entry);
  }
  //else {
    //
    //others
    //
    for (i = 0; i < Entry->KernelAndKextPatches->NrKexts; i++) {
      CHAR8 *Name = Entry->KernelAndKextPatches->KextPatches[i].Name;
      BOOLEAN   isBundle = (AsciiStrStr(Name, ".") != NULL);
      if ((Entry->KernelAndKextPatches->KextPatches[i].DataLen > 0) &&
          isBundle?(AsciiStrCmp(gKextBundleIdentifier, Name) == 0):(AsciiStrStr(gKextBundleIdentifier, Name) != NULL)) {
      //    (AsciiStrStr(InfoPlist, Entry->KernelAndKextPatches->KextPatches[i].Name) != NULL)) {
		  DBG_RT(Entry, "\n\nPatch kext: %s\n", Entry->KernelAndKextPatches->KextPatches[i].Name);
        AnyKextPatch(Driver, DriverSize, InfoPlist, InfoPlistSize, i, Entry);
      }
    }
//  }
  
  //
  // Check for FakeSMC (InjectKexts if no FakeSMC)
  //
  // apianti - Why recheck individual info plist if we checked the whole prelinked
  // CheckForFakeSMC(InfoPlist, Entry);
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
UINT64 GetPlistHexValue(CONST CHAR8 *Plist, CONST CHAR8 *Key, CONST CHAR8 *WholePlist)
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
    //DBG(L"\nNo key: %s\n", Key);
    return 0;
  }
  
  // search for <integer
  IntTag = AsciiStrStr(Value, "<integer");
  if (IntTag == NULL) {
    DBG("\nNo integer\n");
    return 0;
  }
  
  // find <integer end
  Value = AsciiStrStr(IntTag, ">");
  if (Value == NULL) {
    DBG("\nNo <integer end\n");
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
    DBG("\nNo <integer IDREF=\"\n");
    return 0;
  }
  
  // compose <integer ID="xxx" in the Buffer
  IDStart = AsciiStrStr(IntTag, "\"") + 1;
  IDEnd = AsciiStrStr(IDStart, "\"");
  IDLen = IDEnd - IDStart;
  /*
   if (DbgCount < 3) {
   AsciiStrnCpy(Buffer, Value, sizeof(Buffer) - 1);
   DBG(L"\nRef: '%s'\n", Buffer);
   }
   */
  if (IDLen > 8) {
    DBG("\nIDLen too big\n");
    return 0;
  }
  AsciiStrCpyS(Buffer, 48, "<integer ID=\"");
  AsciiStrnCatS(Buffer, 48, IDStart, IDLen);
  AsciiStrCatS(Buffer, 48, "\"");
  /*
   if (DbgCount < 3) {
   DBG(L"Searching: '%s'\n", Buffer);
   }
   */
  
  // and search whole plist for ID
  IntTag = AsciiStrStr(WholePlist, Buffer);
  if (IntTag == NULL) {
    DBG("\nNo %s\n", Buffer);
    return 0;
  }
  
  // got it. find closing >
  /*
   if (DbgCount < 3) {
   AsciiStrnCpy(Buffer, IntTag, sizeof(Buffer) - 1);
   DBG(L"Found: '%s'\n", Buffer);
   }
   */
  Value = AsciiStrStr(IntTag, ">");
  if (Value == NULL) {
    DBG("\nNo <integer end\n");
    return 0;
  }
  if (Value[-1] == '/') {
    DBG("\nInvalid <integer IDREF end\n");
    return 0;
  }
  
  // we should have value now
  NumValue = AsciiStrHexToUint64(Value + 1);
  
  /*
   if (DbgCount < 3) {
   AsciiStrnCpy(Buffer, IntTag, sizeof(Buffer) - 1);
   DBG(L"Found num: %hhX\n", NumValue);
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
VOID PatchPrelinkedKexts(LOADER_ENTRY *Entry)
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
  
  //Slice
  // I see no reason to disable kext injection if FakeSMC found in cache
  //since rev4240 we have manual kext inject disable
  CheckForFakeSMC(WholePlist, Entry);
  
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
         DBG(L"\n\nKext: St = %hhX, Size = %hhX\n", KextAddr, KextSize);
         DBG(L"Info: St = %p, End = %p\n%s\n", InfoPlistStart, InfoPlistEnd, InfoPlistStart);
         gBS->Stall(20000000);
         }
         */
        
        // patch it
        PatchKext(
                  (UINT8*)(UINTN)KextAddr,
                  KextSize,
                  InfoPlistStart,
                  (UINT32)(InfoPlistEnd - InfoPlistStart),
                  Entry
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
VOID PatchLoadedKexts(LOADER_ENTRY *Entry)
{
  DTEntry             MMEntry;
  _BooterKextFileInfo *KextFileInfo;
  CHAR8               *PropName;
  _DeviceTreeBuffer   *PropEntry;
  CHAR8               SavedValue;
  CHAR8               *InfoPlist;
  OpaqueDTPropertyIterator OPropIter;
  DTPropertyIterator	PropIter = &OPropIter;
  //UINTN               DbgCount = 0;
  
  
  DBG("\nPatchLoadedKexts ... dtRoot = %p\n", dtRoot);
  
  if (!dtRoot || !dtLength) {
    return;
  }
  
  DTInit(dtRoot, dtLength);
  
  if (!EFI_ERROR(DTLookupEntry(NULL,"/chosen/memory-map", &MMEntry))) {
    if (!EFI_ERROR(DTCreatePropertyIterator(MMEntry, PropIter))) {
      while (!EFI_ERROR(DTIterateProperties(PropIter, &PropName))) {
        //DBG(L"Prop: %s\n", PropName);
        if (AsciiStrStr(PropName,"Driver-")) {
          // PropEntry _DeviceTreeBuffer is the value of Driver-XXXXXX property
          PropEntry = (_DeviceTreeBuffer*)(((UINT8*)PropIter->CurrentProperty) + sizeof(DeviceTreeNodeProperty));
          //if (DbgCount < 3) DBG(L"%s: paddr = %hhX, length = %hhX\n", PropName, PropEntry->paddr, PropEntry->length);
          
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
                    KextFileInfo->infoDictLength,
                    Entry
                    );

          InfoPlist[KextFileInfo->infoDictLength] = SavedValue;
        }
        //if(AsciiStrStr(PropName,"DriversPackage-")!=0)
        //{
        //    DBG(L"Found %s\n", PropName);
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
VOID KextPatcherStart(LOADER_ENTRY *Entry)
{
  if (isKernelcache) {
    DBG_RT(Entry, "Patching kernelcache ...\n");
    if (Entry->KernelAndKextPatches->KPDebug) {
      gBS->Stall(2000000);
    }
    PatchPrelinkedKexts(Entry);
  } else {
    DBG_RT(Entry, "Patching loaded kexts ...\n");
    if (Entry->KernelAndKextPatches->KPDebug) {
      gBS->Stall(2000000);
    }
    PatchLoadedKexts(Entry);
  }
}

