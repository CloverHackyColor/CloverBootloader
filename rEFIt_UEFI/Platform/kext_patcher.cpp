/*
 * Copyright (c) 2011-2012 Frank Peng. All rights reserved.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#ifdef __cplusplus
}
#endif

#include <UefiLoader.h>
#include "Platform.h"
#include "kernel_patcher.h"

#define OLD_METHOD 0


#ifndef DEBUG_ALL
#define KEXT_DEBUG 1
#else
#define KEXT_DEBUG DEBUG_ALL
#endif

#if KEXT_DEBUG == 2
#define DBG(...)    printf(__VA_ARGS__);
#elif KEXT_DEBUG == 1
#define DBG(...)    DebugLog(KEXT_DEBUG, __VA_ARGS__)
#else
#define DBG(...)
#endif


// runtime debug
#define DBG_RT(...)    if ((KernelAndKextPatches != NULL) && KernelAndKextPatches->KPDebug) { printf(__VA_ARGS__); }

//
// Searches Source for Search pattern of size SearchSize
// and returns the number of occurences.
//
UINTN SearchAndCount(const UINT8 *Source, UINT64 SourceSize, const UINT8 *Search, UINTN SearchSize)
{
  UINTN        NumFounds = 0;
  const UINT8  *End = Source + SourceSize;
  
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
UINTN SearchAndReplace(UINT8 *Source, UINT64 SourceSize, const UINT8 *Search, UINTN SearchSize, const UINT8 *Replace, INTN MaxReplaces)
{
  UINTN     NumReplaces = 0;
  BOOLEAN   NoReplacesRestriction = MaxReplaces <= 0;
//  UINT8     *Begin = Source;
  UINT8     *End = Source + SourceSize;
  if (!Source || !Search || !Replace || !SearchSize) {
    return 0;
  }
  
  while ((Source < End) && (NoReplacesRestriction || (MaxReplaces > 0))) {
    if (CompareMem(Source, Search, SearchSize) == 0) {
 //     printf("  found pattern at %llx\n", (UINTN)(Source - Begin));
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

BOOLEAN CompareMemMask(const UINT8 *Source, const UINT8 *Search, UINTN SearchSize, const UINT8 *Mask, UINTN MaskSize)
{
  UINT8 M;
 
  if (!Mask || MaskSize == 0) {
    return !CompareMem(Source, Search, SearchSize);
  }
  for (UINTN Ind = 0; Ind < SearchSize; Ind++) {
    if (Ind < MaskSize)
      M = *Mask++;
    else M = 0xFF;
    if ((*Source++ & M) != (*Search++ & M)) {
      return FALSE;
    }
  }
  return TRUE;
}

VOID CopyMemMask(UINT8 *Dest, const UINT8 *Replace, const UINT8 *Mask, UINTN SearchSize)
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

// search a pattern like
// call task or jmp address
//return the address next to the command
// 0 if not found
UINTN FindRelative32(const UINT8 *Source, UINTN Start, UINTN SourceSize, UINTN taskLocation)
{
  INT32 Offset; //can be negative, so 0xFFFFFFFF == -1
  for (UINTN i = Start; i < Start + SourceSize - 4; ++i) {
    Offset = (INT32)((UINT32)Source[i] + ((UINT32)Source[i+1]<<8) + ((UINT32)Source[i+2]<<16) + ((UINT32)Source[i+3]<<24)); //should not use *(UINT32*) because of alignment
    if (taskLocation == i + Offset + 4) {
      return (i+4);
    }
  }
  return 0;
}
/*
UINTN FindSection(const UINT8 *Source, UINTN len, const UINT8* seg, const UINT8* sec)
{
  BOOLEAN eq;
  
  for (UINTN i = 0x20; i < len; i++) {
    eq = TRUE;
    for (UINTN j = 0; j < 16 && (sec[j] != 0); j++) {
      if (Source[i + j] != sec[j]) {
        eq = FALSE;
        break;
      }
    }
    if (eq) {
      for (UINTN j = 0; j < 16 && (seg[j] != 0); j++) {
        if (Source[i + 0x10 + j] != seg[j]) {
          eq = FALSE;
          break;
        }
      }
      if (eq)
        return i + 16;
    }
  }
  return 0;
}
*/
UINTN FindMemMask(const UINT8 *Source, UINTN SourceSize, const UINT8 *Search, UINTN SearchSize, const UINT8 *MaskSearch, UINTN MaskSize)
{
  if (!Source || !Search || !SearchSize) {
    return KERNEL_MAX_SIZE;
  }

  for (UINTN i = 0; i < SourceSize - SearchSize; ++i) {
    if (CompareMemMask(&Source[i], Search, SearchSize, MaskSearch, MaskSize)) {
      return i;
    }
  }
  return KERNEL_MAX_SIZE;
}

UINTN SearchAndReplaceMask(UINT8 *Source, UINT64 SourceSize, const UINT8 *Search, const UINT8 *MaskSearch, UINTN SearchSize,
                       const UINT8 *Replace, const UINT8 *MaskReplace, INTN MaxReplaces)
{
  UINTN     NumReplaces = 0;
  BOOLEAN   NoReplacesRestriction = MaxReplaces <= 0;
  UINT8     *End = Source + SourceSize;
  if (!Source || !Search || !Replace || !SearchSize) {
    return 0;
  }
  while ((Source < End) && (NoReplacesRestriction || (MaxReplaces > 0))) {
    if (CompareMemMask((const UINT8 *)Source, Search, SearchSize, MaskSearch, SearchSize)) {
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
    CopyMem(Pos, Replace, SearchSize);
    SetMem(Pos + SearchSize, Skip, 0x20); //fill skip places with spaces
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
    
    if (strncmp(Tag, "<dict>", 6) == 0) {
      // opening dict
      DictLevel++;
      Tag += 6;
      
    } else if (strncmp(Tag, "</dict>", 7) == 0) {
      // closing dict
      DictLevel--;
      Tag += 7;
      
    } else if (DictLevel == 1 && strncmp(Tag, "<key>CFBundleIdentifier</key>", 29) == 0) {
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
VOID LOADER_ENTRY::ATIConnectorsPatchInit()
{
  //
  // prepar boundle ids
  //
  
  // Lion, SnowLeo 10.6.7 2011 MBP
  snprintf(ATIKextBundleId[0],
              sizeof(ATIKextBundleId[0]),
		   "com.apple.kext.ATI%sController", // when it was AsciiSPrint, %a was used with KPATIConnectorsController which is CHAR16 ??? Result is printing stop at first char <= 255
           //now it is CHAR8*
              KernelAndKextPatches->KPATIConnectorsController
              );
  // ML
  snprintf(ATIKextBundleId[1],
              sizeof(ATIKextBundleId[1]),
		   "com.apple.kext.AMD%sController", // when it was AsciiSPrint, %a was used with KPATIConnectorsController which is CHAR16 ??? Result is printing stop at first char <= 255
              KernelAndKextPatches->KPATIConnectorsController
              );
  
  ATIConnectorsPatchInited = TRUE;
  
  //DBG(L"Bundle1: %s\n", ATIKextBundleId[0]);
  //DBG(L"Bundle2: %s\n", ATIKextBundleId[1]);
  //gBS->Stall(10000000);
}

//
// Registers kexts that need force-load during WithKexts boot.
//
VOID LOADER_ENTRY::ATIConnectorsPatchRegisterKexts(void *FSInject_v, void *ForceLoadKexts_v)
{
  FSINJECTION_PROTOCOL *FSInject = (FSINJECTION_PROTOCOL *)FSInject_v;
  FSI_STRING_LIST *ForceLoadKexts = (FSI_STRING_LIST *)ForceLoadKexts_v;
  // for future?
  FSInject->AddStringToList(ForceLoadKexts,
                            SWPrintf("\\AMD%sController.kext\\Contents\\Info.plist", KernelAndKextPatches->KPATIConnectorsController).wc_str()
                            );
  // Lion, ML, SnowLeo 10.6.7 2011 MBP
  FSInject->AddStringToList(ForceLoadKexts,
                            SWPrintf("\\ATI%sController.kext\\Contents\\Info.plist", KernelAndKextPatches->KPATIConnectorsController).wc_str()
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
VOID LOADER_ENTRY::ATIConnectorsPatch(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize)
{
  
  UINTN   Num = 0;
  
	DBG_RT("\nATIConnectorsPatch: driverAddr = %llx, driverSize = %x\nController = %s\n",
         (UINTN)Driver, DriverSize, KernelAndKextPatches->KPATIConnectorsController);
  ExtractKextBundleIdentifier(InfoPlist);
	DBG_RT("Kext: %s\n", gKextBundleIdentifier);
  
  // number of occurences od Data should be 1
  Num = SearchAndCount(Driver, DriverSize, KernelAndKextPatches->KPATIConnectorsData, KernelAndKextPatches->KPATIConnectorsDataLen);
  if (Num > 1) {
    // error message - shoud always be printed
	  printf("==> KPATIConnectorsData found %llu times in %s - skipping patching!\n", Num, gKextBundleIdentifier);
    Stall(5*1000000);
    return;
  }
  
  // patch
  Num = SearchAndReplace(Driver,
                         DriverSize,
                         KernelAndKextPatches->KPATIConnectorsData,
                         KernelAndKextPatches->KPATIConnectorsDataLen,
                         KernelAndKextPatches->KPATIConnectorsPatch,
                         1);
    if (Num > 0) {
		DBG_RT("==> patched %llu times!\n", Num);
    } else {
      DBG_RT("==> NOT patched!\n");
    }
    Stall(5000000);
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

const UINT8   MovlE2ToEcx[] = { 0xB9, 0xE2, 0x00, 0x00, 0x00 };
const UINT8   MovE2ToCx[]   = { 0x66, 0xB9, 0xE2, 0x00 };
const UINT8   Wrmsr[]       = { 0x0F, 0x30 };

VOID LOADER_ENTRY::AppleIntelCPUPMPatch(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize)
{
  UINTN   Index1;
  UINTN   Index2;
  UINTN   Count = 0;
  UINTN   Start = 0;
  UINTN   Size = DriverSize;

	DBG_RT("\nAppleIntelCPUPMPatch: driverAddr = %llx, driverSize = %x\n", (UINTN)Driver, DriverSize);
//  if (KernelAndKextPatches->KPDebug) {
//    ExtractKextBundleIdentifier(InfoPlist);
//  }
//	DBG_RT("Kext: %s\n", gKextBundleIdentifier);

  // we should scan only __text __TEXT | Slice -> do this
  INTN textName = FindMem(Driver, DriverSize, kPrelinkTextSection, sizeof(kPrelinkTextSection));
  if (textName > 0) {
    SEGMENT *textSeg = (SEGMENT *)&Driver[textName];
    Start = textSeg->fileoff;
    Size = textSeg->filesize;
    DBG("found __text [%llX,%llX]\n",Start, Size);
    if (Start > DriverSize) Start = 0;
    if (Size > DriverSize) {
      Size = DriverSize;
    }
  }
  
  for (Index1 = Start; Index1 < Start + Size; Index1++) {
    // search for MovlE2ToEcx
    if (CompareMem(Driver + Index1, MovlE2ToEcx, sizeof(MovlE2ToEcx)) == 0) {
      // search for wrmsr in next few bytes
      for (Index2 = Index1 + sizeof(MovlE2ToEcx); Index2 < Index1 + sizeof(MovlE2ToEcx) + 32; Index2++) {
        if (Driver[Index2] == Wrmsr[0] && Driver[Index2 + 1] == Wrmsr[1]) {
          // found it - patch it with nops
          Count++;
          Driver[Index2] = 0x90;
          Driver[Index2 + 1] = 0x90;
          DBG_RT(" %llu. patched at 0x%llx\n", Count, Index2);
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
          DBG_RT(" %llu. patched CX at 0x%llx\n", Count, Index2);
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
	DBG_RT("= %llu patches\n", Count);
  Stall(5000000);
}



////////////////////////////////////
//
// AppleRTC patch to prevent CMOS reset
//
// http://www.insanelymac.com/forum/index.php?showtopic=253992
// http://www.insanelymac.com/forum/index.php?showtopic=276066
//
#if OLD_METHOD
const UINT8   LionSearch_X64[]  = { 0x75, 0x30, 0x44, 0x89, 0xf8 };
const UINT8   LionReplace_X64[] = { 0xeb, 0x30, 0x44, 0x89, 0xf8 };

const UINT8   LionSearch_i386[]  = { 0x75, 0x3d, 0x8b, 0x75, 0x08 };
const UINT8   LionReplace_i386[] = { 0xeb, 0x3d, 0x8b, 0x75, 0x08 };

const UINT8   MLSearch[]  = { 0x75, 0x30, 0x89, 0xd8 };
const UINT8   MLReplace[] = { 0xeb, 0x30, 0x89, 0xd8 };

// SunKi: 10.9 - 10.14.3
const UINT8   MavMoj3Search[]  = { 0x75, 0x2e, 0x0f, 0xb6 };
const UINT8   MavMoj3Replace[] = { 0xeb, 0x2e, 0x0f, 0xb6 };

// RodionS: 10.14.4+ / 10.15 DB1
const UINT8   Moj4CataSearch[]  = { 0x75, 0x33, 0x0f, 0xb7 };
const UINT8   Moj4CataReplace[] = { 0xeb, 0x33, 0x0f, 0xb7 };
#endif
//
// We can not rely on OSVersion global variable for OS version detection,
// since in some cases it is not correct (install of ML from Lion, for example). -- AppleRTC patch is not needed for installation
// So, we'll use "brute-force" method - just try to patch.
// Actually, we'll at least check that if we can find only one instance of code that
// we are planning to patch.
//

VOID LOADER_ENTRY::AppleRTCPatch(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize)
{
#if OLD_METHOD
  UINTN   Num = 0;
  UINTN   NumLion_X64 = 0;
  UINTN   NumLion_i386 = 0;
  UINTN   NumML = 0;
  UINTN   NumMavMoj3 = 0;
  UINTN   NumMoj4 = 0;
  
	DBG_RT("\nAppleRTCPatch: driverAddr = %llx, driverSize = %x\n", (UINTN)Driver, DriverSize);
  if (KernelAndKextPatches->KPDebug) {
    ExtractKextBundleIdentifier(InfoPlist);
  }
	DBG_RT("Kext: %s\n", gKextBundleIdentifier);
  
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
	  DBG_RT("AppleRTCPatch: ERROR: multiple patterns found (LionX64: %llu, Lioni386: %llu, ML: %llu, MavMoj3: %llu, Moj4: %llu) - skipping patching!\n",
          NumLion_X64, NumLion_i386, NumML, NumMavMoj3, NumMoj4);
    Stall(5000000);
    return;
  }
  
  if (NumLion_X64 == 1) {
    Num = SearchAndReplace(Driver, DriverSize, LionSearch_X64, sizeof(LionSearch_X64), LionReplace_X64, 1);
	  DBG_RT("==> Lion X64: %llu replaces done.\n", Num);
  } else if (NumLion_i386 == 1) {
    Num = SearchAndReplace(Driver, DriverSize, LionSearch_i386, sizeof(LionSearch_i386), LionReplace_i386, 1);
	  DBG_RT("==> Lion i386: %llu replaces done.\n", Num);
  } else if (NumML == 1) {
    Num = SearchAndReplace(Driver, DriverSize, MLSearch, sizeof(MLSearch), MLReplace, 1);
	  DBG_RT("==> MountainLion X64: %llu replaces done.\n", Num);
  } else if (NumMavMoj3 == 1) {
    Num = SearchAndReplace(Driver, DriverSize, MavMoj3Search, sizeof(MavMoj3Search), MavMoj3Replace, 1);
	  DBG_RT("==> Mav/Yos/El/Sie/HS/Moj3 X64: %llu replaces done.\n", Num);
  } else if (NumMoj4 == 1) {
    Num = SearchAndReplace(Driver, DriverSize, Moj4CataSearch, sizeof(Moj4CataSearch), Moj4CataReplace, 1);
	  DBG_RT("==> Mojave4 X64: %llu replaces done.\n", Num);
  } else {
    DBG_RT("==> Patterns not found - patching NOT done.\n");
  }
#else
  //RodionS

  UINTN procLocation = searchProcInDriver(Driver, DriverSize, "updateChecksum");
  DBG("updateChecksum at 0x%llx\n", procLocation);
  if (procLocation != 0) {
    if ((((struct mach_header_64*)KernelData)->filetype) == MH_KERNEL_COLLECTION) {
      DBG("procedure in kernel space\n");
      for (int j = 0; j < 20; ++j) {
        DBG("%02X", KernelData[procLocation + j]);
      }
      DBG("\n");
      KernelData[procLocation] = 0xC3;
    } else {
      DBG("procedure in Driver space\n");
      for (int j = 0; j < 20; ++j) {
        DBG("%02X", Driver[procLocation + j]);
      }
      DBG("\n");
      Driver[procLocation] = 0xC3;
    }
    DBG_RT("AppleRTC: patched\n");
  } else {
    DBG_RT("AppleRTC: not patched\n");
  }
  

#endif
  Stall(5000000);
}



///////////////////////////////////
//
// InjectKexts if no FakeSMC: Detect FakeSMC and if present then
// disable kext injection InjectKexts()
//
// not used since 4242
#if 0
VOID LOADER_ENTRY::CheckForFakeSMC(CHAR8 *InfoPlist)
{
  if (OSFLAG_ISSET(Flags, OSFLAG_CHECKFAKESMC) &&
      OSFLAG_ISSET(Flags, OSFLAG_WITHKEXTS)) {
    if (AsciiStrStr(InfoPlist, "<string>org.netkas.driver.FakeSMC</string>") != NULL
        || AsciiStrStr(InfoPlist, "<string>org.netkas.FakeSMC</string>") != NULL
        || AsciiStrStr(InfoPlist, "<string>as.vit9696.VirtualSMC</string>") != NULL)
    {
      Flags = OSFLAG_UNSET(Flags, OSFLAG_WITHKEXTS);
      DBG_RT("\nFakeSMC or VirtualSMC found, UNSET WITHKEXTS\n");
      Stall(5000000);
    }
  }
}
#endif


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
VOID LOADER_ENTRY::DellSMBIOSPatch(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize)
{
  //
  // syscl
  // Note, smbios truncate issue only affects Broadwell platform and platform
  // later than Broadwell thus we don't need to consider OS versinos earlier
  // than Yosemite, they are all pure 64bit platforms
  //
  UINTN gPatchCount = 0;
  
  DBG_RT("\nDellSMBIOSPatch: driverAddr = %llx, driverSize = %x\n", (UINTN)Driver, DriverSize);
  if (KernelAndKextPatches->KPDebug)
  {
    ExtractKextBundleIdentifier(InfoPlist);
  }
  DBG_RT("Kext: %s\n", gKextBundleIdentifier);
  
  //
  // now, let's patch it!
  //
  gPatchCount = SearchAndReplace(Driver, DriverSize, DELL_SMBIOS_GUID_Search, sizeof(DELL_SMBIOS_GUID_Search), DELL_SMBIOS_GUID_Replace, 1);
  
  if (gPatchCount >= 1)
  {
    DBG_RT("==> AppleSMBIOS: %llu replaces done.\n", gPatchCount);
  }
  else
  {
    DBG_RT("==> Patterns not found - patching NOT done.\n");
  }
  
  Stall(5000000);
}



////////////////////////////////////
//
// SNBE_AICPUPatch implemented by syscl
// Fix AppleIntelCPUPowerManagement on SandyBridge-E (c) omni, stinga11
//
VOID LOADER_ENTRY::SNBE_AICPUPatch(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize)
{
    UINT32 i;
    UINT64 os_ver = AsciiOSVersionToUint64(OSVersion);
    
	DBG_RT("\nSNBE_AICPUPatch: driverAddr = %llx, driverSize = %x\n", (UINTN)Driver, DriverSize);
    if (KernelAndKextPatches->KPDebug) {
        ExtractKextBundleIdentifier(InfoPlist);
    }
    
	DBG_RT("Kext: %s\n", gKextBundleIdentifier);
    
    // now let's patch it
    if (os_ver < AsciiOSVersionToUint64("10.9") || os_ver >= AsciiOSVersionToUint64("10.14")) {
        DBG("Unsupported macOS.\nSandyBridge-E requires macOS 10.9 - 10.13.x, aborted\n");
        DBG("SNBE_AICPUPatch() <===FALSE\n");
        return;
    }
    
    if (os_ver < AsciiOSVersionToUint64("10.10")) {
        // 10.9.x
        const UINT8 find[][3] = {
            { 0x84, 0x2F, 0x01 },
            { 0x3E, 0x75, 0x3A },
            { 0x84, 0x5F, 0x01 },
            { 0x74, 0x10, 0xB9 },
            { 0x75, 0x07, 0xB9 },
            { 0xFC, 0x02, 0x74 },
            { 0x01, 0x74, 0x58 }
        };
        const UINT8 repl[][3] = {
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
        const UINT8 find[][3] = {
            { 0x3E, 0x75, 0x39 },
            { 0x74, 0x11, 0xB9 },
            { 0x01, 0x74, 0x56 }
        };
        const UINT8 repl[][3] = {
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
        
        const UINT8 find_1[] = { 0xFF, 0x0F, 0x84, 0x2D };
        const UINT8 repl_1[] = { 0xFF, 0x0F, 0x85, 0x2D };
        if (SearchAndReplace(Driver, DriverSize, find_1, sizeof(find_1), repl_1, 0)) {
            DBG("SNBE_AICPUPatch (4/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (4/7) not apply\n");
        }
        
        
        const UINT8 find_2[] = { 0x01, 0x00, 0x01, 0x0F, 0x84 };
        const UINT8 repl_2[] = { 0x01, 0x00, 0x01, 0x0F, 0x85 };
        if (SearchAndReplace(Driver, DriverSize, find_2, sizeof(find_2), repl_2, 0)) {
            DBG("SNBE_AICPUPatch (5/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (5/7) not apply\n");
        }
        
        const UINT8 find_3[] = { 0x02, 0x74, 0x0B, 0x41, 0x83, 0xFC, 0x03, 0x75, 0x22, 0xB9, 0x02, 0x06 };
        const UINT8 repl_3[] = { 0x02, 0xEB, 0x0B, 0x41, 0x83, 0xFC, 0x03, 0x75, 0x22, 0xB9, 0x02, 0x06 };
        if (SearchAndReplace(Driver, DriverSize, find_3, sizeof(find_3), repl_3, 0)) {
            DBG("SNBE_AICPUPatch (6/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (6/7) not apply\n");
        }
        
        const UINT8 find_4[] = { 0x74, 0x0B, 0x41, 0x83, 0xFC, 0x03, 0x75, 0x11, 0xB9, 0x42, 0x06, 0x00 };
        const UINT8 repl_4[] = { 0xEB, 0x0B, 0x41, 0x83, 0xFC, 0x03, 0x75, 0x11, 0xB9, 0x42, 0x06, 0x00 };
        if (SearchAndReplace(Driver, DriverSize, find_4, sizeof(find_4), repl_4, 0)) {
            DBG("SNBE_AICPUPatch (7/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (7/7) not apply\n");
        }
    } else if (os_ver < AsciiOSVersionToUint64("10.12")) {
        // 10.11
        const UINT8 find[][3] = {
            { 0x3E, 0x75, 0x39 },
            { 0x75, 0x11, 0xB9 },
            { 0x01, 0x74, 0x5F }
        };
        const UINT8 repl[][3] = {
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
        
        const UINT8 find_1[] = { 0xFF, 0x0F, 0x84, 0x2D };
        const UINT8 repl_1[] = { 0xFF, 0x0F, 0x85, 0x2D };
        if (SearchAndReplace(Driver, DriverSize, find_1, sizeof(find_1), repl_1, 0)) {
            DBG("SNBE_AICPUPatch (4/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (4/7) not apply\n");
        }
        
        const UINT8 find_2[] = { 0x01, 0x00, 0x01, 0x0F, 0x84 };
        const UINT8 repl_2[] = { 0x01, 0x00, 0x01, 0x0F, 0x85 };
        if (SearchAndReplace(Driver, DriverSize, find_2, sizeof(find_2), repl_2, 0)) {
            DBG("SNBE_AICPUPatch (5/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (5/7) not apply\n");
        }
        
        const UINT8 find_3[] = { 0xC9, 0x74, 0x16, 0x0F, 0x32, 0x48, 0x25, 0xFF, 0x0F, 0x00, 0x00, 0x48 };
        const UINT8 repl_3[] = { 0xC9, 0xEB, 0x16, 0x0F, 0x32, 0x48, 0x25, 0xFF, 0x0F, 0x00, 0x00, 0x48 };
        if (SearchAndReplace(Driver, DriverSize, find_3, sizeof(find_3), repl_3, 0)) {
            DBG("SNBE_AICPUPatch (6/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (6/7) not apply\n");
        }
        
        const UINT8 find_4[] = { 0xC9, 0x74, 0x0C, 0x0F, 0x32, 0x83, 0xE0, 0x1F, 0x42, 0x89, 0x44, 0x3B };
        const UINT8 repl_4[] = { 0xC9, 0xEB, 0x0C, 0x0F, 0x32, 0x83, 0xE0, 0x1F, 0x42, 0x89, 0x44, 0x3B };
        if (SearchAndReplace(Driver, DriverSize, find_4, sizeof(find_4), repl_4, 0)) {
            DBG("SNBE_AICPUPatch (7/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (7/7) not apply\n");
        }
    } else if (os_ver < AsciiOSVersionToUint64("10.13")) {
        // 10.12
        const UINT8 find[][3] = {
            { 0x01, 0x74, 0x61 },
            { 0x3E, 0x75, 0x38 },
            { 0x75, 0x11, 0xB9 }
        };
        const UINT8 repl[][3] = {
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
        
        const UINT8 find_1[] = { 0xFF, 0x0F, 0x84, 0x2D };
        const UINT8 repl_1[] = { 0xFF, 0x0F, 0x85, 0x2D };
        if (SearchAndReplace(Driver, DriverSize, find_1, sizeof(find_1), repl_1, 0)) {
            DBG("SNBE_AICPUPatch (4/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (4/7) not apply\n");
        }

        const UINT8 find_2[] = { 0x01, 0x00, 0x01, 0x0F, 0x84 };
        const UINT8 repl_2[] = { 0x01, 0x00, 0x01, 0x0F, 0x85 };
        if (SearchAndReplace(Driver, DriverSize, find_2, sizeof(find_2), repl_2, 0)) {
            DBG("SNBE_AICPUPatch (5/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (5/7) not apply\n");
        }
        
        const UINT8 find_3[] = { 0xC9, 0x74, 0x15, 0x0F, 0x32, 0x25, 0xFF, 0x0F, 0x00, 0x00, 0x48 };
        const UINT8 repl_3[] = { 0xC9, 0xEB, 0x15, 0x0F, 0x32, 0x25, 0xFF, 0x0F, 0x00, 0x00, 0x48 };
        if (SearchAndReplace(Driver, DriverSize, find_3, sizeof(find_3), repl_3, 0)) {
            DBG("SNBE_AICPUPatch (6/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (6/7) not apply\n");
        }

        const UINT8 find_4[] = { 0xC9, 0x74, 0x0C, 0x0F, 0x32, 0x83, 0xE0, 0x1F, 0x42, 0x89, 0x44, 0x3B };
        const UINT8 repl_4[] = { 0xC9, 0xEB, 0x0C, 0x0F, 0x32, 0x83, 0xE0, 0x1F, 0x42, 0x89, 0x44, 0x3B };
        if (SearchAndReplace(Driver, DriverSize, find_4, sizeof(find_4), repl_4, 0)) {
            DBG("SNBE_AICPUPatch (7/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (7/7) not apply\n");
        }
    } else if (os_ver < AsciiOSVersionToUint64("10.15")) {
        // 10.13/10.14
        const UINT8 find[][3] = {
            { 0x01, 0x74, 0x61 },
            { 0x3E, 0x75, 0x38 },
            { 0x75, 0x11, 0xB9 }
        };
        const UINT8 repl[][3] = {
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
        
        const UINT8 find_1[] = { 0xFF, 0x0F, 0x84, 0xD3 };
        const UINT8 repl_1[] = { 0xFF, 0x0F, 0x85, 0xD3 };
        if (SearchAndReplace(Driver, DriverSize, find_1, sizeof(find_1), repl_1, 0)) {
            DBG("SNBE_AICPUPatch (4/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (4/7) not apply\n");
        }
        
        const UINT8 find_2[] = { 0x01, 0x00, 0x01, 0x0F, 0x84 };
        const UINT8 repl_2[] = { 0x01, 0x00, 0x01, 0x0F, 0x85 };
        if (SearchAndReplace(Driver, DriverSize, find_2, sizeof(find_2), repl_2, 0)) {
            DBG("SNBE_AICPUPatch (5/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (5/7) not apply\n");
        }
        
        const UINT8 find_3[] = { 0xC9, 0x74, 0x14, 0x0F, 0x32, 0x25, 0xFF, 0x0F, 0x00, 0x00, 0x6B };
        const UINT8 repl_3[] = { 0xC9, 0xEB, 0x14, 0x0F, 0x32, 0x25, 0xFF, 0x0F, 0x00, 0x00, 0x6B};
        if (SearchAndReplace(Driver, DriverSize, find_3, sizeof(find_3), repl_3, 0)) {
            DBG("SNBE_AICPUPatch (6/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (6/7) not apply\n");
        }
        
        const UINT8 find_4[] = { 0xC9, 0x74, 0x0C, 0x0F, 0x32, 0x83, 0xE0, 0x1F, 0x42, 0x89, 0x44, 0x3B };
        const UINT8 repl_4[] = { 0xC9, 0xEB, 0x0C, 0x0F, 0x32, 0x83, 0xE0, 0x1F, 0x42, 0x89, 0x44, 0x3B };
        if (SearchAndReplace(Driver, DriverSize, find_4, sizeof(find_4), repl_4, 0)) {
            DBG("SNBE_AICPUPatch (7/7) applied\n");
        } else {
            DBG("SNBE_AICPUPatch (7/7) not apply\n");
        }
    }
    
  Stall(5000000);
}


////////////////////////////////////
//
// BDWE_IOPCIPatch implemented by syscl
// Fix Broadwell-E IOPCIFamily issue
//

// El Capitan
const UINT8   BroadwellE_IOPCI_Find_El[] = { 0x48, 0x81, 0xF9, 0x01, 0x00, 0x00, 0x40 };
const UINT8   BroadwellE_IOPCI_Repl_El[] = { 0x48, 0x81, 0xF9, 0x01, 0x00, 0x00, 0x80 };

// Sierra/High Sierra
const UINT8   BroadwellE_IOPCI_Find_SieHS[] = { 0x48, 0x81, 0xFB, 0x00, 0x00, 0x00, 0x40 };
const UINT8   BroadwellE_IOPCI_Repl_SieHS[] = { 0x48, 0x81, 0xFB, 0x00, 0x00, 0x00, 0x80 };

// Mojave
const UINT8   BroadwellE_IOPCI_Find_MojCata[] = { 0x48, 0x3D, 0x00, 0x00, 0x00, 0x40 };
const UINT8   BroadwellE_IOPCI_Repl_MojCata[] = { 0x48, 0x3D, 0x00, 0x00, 0x00, 0x80 };

VOID LOADER_ENTRY::BDWE_IOPCIPatch(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize)
{
  UINTN count = 0;
  UINT64 os_ver = AsciiOSVersionToUint64(OSVersion);
    
	DBG_RT("\nBDWE_IOPCIPatch: driverAddr = %llx, driverSize = %x\n", (UINTN)Driver, DriverSize);
  if (KernelAndKextPatches->KPDebug) {
    ExtractKextBundleIdentifier(InfoPlist);
  }
    
	DBG_RT("Kext: %s\n", gKextBundleIdentifier);
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
	  DBG_RT("==> IOPCIFamily: %llu replaces done.\n", count);
  } else {
    DBG_RT("==> Patterns not found - patching NOT done.\n");
  }
    
  Stall(5000000);
}

VOID LOADER_ENTRY::EightApplePatch(UINT8 *Driver, UINT32 DriverSize)
{
//  UINTN procLen = 0;
  DBG("8 apple patch\n");
  UINTN procAddr = searchProcInDriver(Driver, DriverSize, "initFB");
  UINTN verbose  = searchProcInDriver(Driver, DriverSize, "gIOFBVerboseBoot");
  if (procAddr != 0) {
    UINTN patchLoc;
  
    if ((((struct mach_header_64*)KernelData)->filetype) == MH_KERNEL_COLLECTION) {
      DBG("procedure in kernel space\n");
      patchLoc = FindRelative32(KernelData, procAddr, 0x300, verbose-1);
      for (int j = 0; j < 20; ++j) {
        DBG("%02X", KernelData[procAddr + j]);
      }
      DBG("\n");
      if (patchLoc != 0 && KernelData[patchLoc + 1] == 0x75) {
        KernelData[patchLoc + 1] = 0xEB;
        DBG("8 apples patch success\n");
      } else {
        DBG("8 apples patch not found, loc=0x%llx\n", patchLoc);
      }
    } else {
      DBG("procedure in Driver space\n");
      patchLoc = FindRelative32(Driver, procAddr, 0x300, verbose-1);
      for (int j = 0; j < 20; ++j) {
        DBG("%02X", Driver[patchLoc + j]);
      }
      DBG("\n");
      if (patchLoc != 0 && Driver[patchLoc + 1] == 0x75) {
        Driver[patchLoc + 1] = 0xEB;
        DBG("8 apples patch success\n");
      } else {
        DBG("8 apples patch not found, loc=0x%llx\n", patchLoc);
      }
    }
    DBG_RT("AppleRTC: patched\n");
  } else {
    DBG_RT("AppleRTC: not patched\n");
  }

  Stall(5000000);
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
VOID LOADER_ENTRY::AnyKextPatch(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize, INT32 N)
{
  UINTN   Num = 0;
  INTN    Ind;

  // if we modify value directly at KernelAndKextPatches->KextPatches[N].SearchLen, it will be wrong for next driver
  UINTN   SearchLen = KernelAndKextPatches->KextPatches[N].SearchLen;
  
  DBG_RT("\nAnyKextPatch %d: driverAddr = %llx, driverSize = %x\nAnyKext = %s\n",
         N, (UINTN)Driver, DriverSize, KernelAndKextPatches->KextPatches[N].Label);
  DBG("\nAnyKextPatch %d: driverAddr = %llx, driverSize = %x\nLabel = %s\n",
         N, (UINTN)Driver, DriverSize, KernelAndKextPatches->KextPatches[N].Label);

  if (!KernelAndKextPatches->KextPatches[N].MenuItem.BValue) {
    return;
  }

  
  if (!SearchLen ||
      (SearchLen > DriverSize)) {
    SearchLen = DriverSize;
  }

  if (KernelAndKextPatches->KPDebug) {
    ExtractKextBundleIdentifier(InfoPlist);
  }

	DBG_RT("Kext: %s\n", gKextBundleIdentifier);

  if (!KernelAndKextPatches->KextPatches[N].IsPlistPatch) {
    // kext binary patch
    DBG_RT("Binary patch\n");
    bool once = false;
    UINTN procLen = 0;
    UINTN procAddr = searchProcInDriver(Driver, DriverSize, KernelAndKextPatches->KextPatches[N].ProcedureName);
    
    if (SearchLen == DriverSize) {
      procLen = DriverSize - procAddr;
      once = true;
    } else {
      procLen = SearchLen;
    }
    UINT8 * curs = &Driver[procAddr];
    UINTN j = 0;
    while (j < DriverSize) {
      if (!KernelAndKextPatches->KextPatches[N].StartPattern || //old behavior
          CompareMemMask((const UINT8*)curs,
                         (const UINT8 *)KernelAndKextPatches->KextPatches[N].StartPattern,
                         KernelAndKextPatches->KextPatches[N].StartPatternLen,
                         (const UINT8 *)KernelAndKextPatches->KextPatches[N].StartMask,
                         KernelAndKextPatches->KextPatches[N].StartPatternLen)) {
        DBG_RT(" StartPattern found\n");

        Num = SearchAndReplaceMask(curs,
                                   procLen,
                                   (const UINT8*)KernelAndKextPatches->KextPatches[N].Data,
                                   (const UINT8*)KernelAndKextPatches->KextPatches[N].MaskFind,
                                   KernelAndKextPatches->KextPatches[N].DataLen,
                                   (const UINT8*)KernelAndKextPatches->KextPatches[N].Patch,
                                   (const UINT8*)KernelAndKextPatches->KextPatches[N].MaskReplace,
                                   -1);
        if (Num) {
          curs += SearchLen - 1;
          j    += SearchLen - 1;
        }
      }
      if (once ||
          !KernelAndKextPatches->KextPatches[N].StartPattern ||
          !KernelAndKextPatches->KextPatches[N].StartPatternLen) {
        break;
      }
      j++; curs++;
    }
  } else {
    // Info plist patch
    DBG_RT("Info.plist data : '");
    for (Ind = 0; Ind < KernelAndKextPatches->KextPatches[N].DataLen; Ind++) {
      DBG_RT("%c", KernelAndKextPatches->KextPatches[N].Data[Ind]);
    }
    DBG_RT("' ->\n");
    DBG_RT("Info.plist patch: '");
    for (Ind = 0; Ind < KernelAndKextPatches->KextPatches[N].DataLen; Ind++) {
      DBG_RT("%c", KernelAndKextPatches->KextPatches[N].Patch[Ind]);
    }
    DBG_RT("' \n");
    
    Num = SearchAndReplaceTxt((UINT8*)InfoPlist,
                           InfoPlistSize,
                           KernelAndKextPatches->KextPatches[N].Data,
                           KernelAndKextPatches->KextPatches[N].DataLen,
                           KernelAndKextPatches->KextPatches[N].Patch,
                           -1);
  }
  
  if (KernelAndKextPatches->KPDebug) {
    if (Num > 0) {
		DBG_RT("==> patched %llu times!\n", Num);
    } else {
      DBG_RT("==> NOT patched!\n");
    }
    gBS->Stall(2000000);
  }
}

//
// Called from SetFSInjection(), before boot.efi is started,
// to allow patchers to prepare FSInject to force load needed kexts.
//
VOID LOADER_ENTRY::KextPatcherRegisterKexts(void *FSInject_v, void *ForceLoadKexts)
{
  FSINJECTION_PROTOCOL *FSInject = (FSINJECTION_PROTOCOL *)FSInject_v;
  if (KernelAndKextPatches->KPATIConnectorsController != NULL) {
    ATIConnectorsPatchRegisterKexts(FSInject_v, ForceLoadKexts);
  }
  
  for (INTN i = 0; i < KernelAndKextPatches->NrKexts; i++) {
    FSInject->AddStringToList((FSI_STRING_LIST*)ForceLoadKexts,
                              SWPrintf("\\%s.kext\\Contents\\Info.plist", KernelAndKextPatches->KextPatches[i].Name).wc_str() );
  }
}

//
// PatchKext is called for every kext from prelinked kernel (kernelcache) or from DevTree (booting with drivers).
// Add kext detection code here and call kext specific patch function.
//
VOID LOADER_ENTRY::PatchKext(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize)
{
  if (KernelAndKextPatches->KPATIConnectorsController != NULL) {
    //
    // ATIConnectors
    //
    if (!ATIConnectorsPatchInited) {
      ATIConnectorsPatchInit();
    }
    if (   AsciiStrStr(InfoPlist, ATIKextBundleId[0]) != NULL  // ATI boundle id
        || AsciiStrStr(InfoPlist, ATIKextBundleId[1]) != NULL  // AMD boundle id
        || AsciiStrStr(InfoPlist, "com.apple.kext.ATIFramebuffer") != NULL // SnowLeo
        || AsciiStrStr(InfoPlist, "com.apple.kext.AMDFramebuffer") != NULL //Maverics
        ) {
      ATIConnectorsPatch(Driver, DriverSize, InfoPlist, InfoPlistSize);
      return;
    }
  }
  
  ExtractKextBundleIdentifier(InfoPlist);
  
  if (KernelAndKextPatches->KPAppleIntelCPUPM &&
      (AsciiStrStr(InfoPlist,
                   "<string>com.apple.driver.AppleIntelCPUPowerManagement</string>") != NULL)) {
    //
    // AppleIntelCPUPM
    //
    AppleIntelCPUPMPatch(Driver, DriverSize, InfoPlist, InfoPlistSize);
  } else if (KernelAndKextPatches->KPAppleRTC &&
             (AsciiStrStr(InfoPlist, "com.apple.driver.AppleRTC") != NULL)) {
    //
    // AppleRTC
    //
    AppleRTCPatch(Driver, DriverSize, InfoPlist, InfoPlistSize);
  } else if (KernelAndKextPatches->KPDELLSMBIOS &&
           (AsciiStrStr(InfoPlist, "com.apple.driver.AppleSMBIOS") != NULL)) {
    //
    // DellSMBIOSPatch
    //
    DBG_RT("Remap SMBIOS Table require, AppleSMBIOS...\n");
    DellSMBIOSPatch(Driver, DriverSize, InfoPlist, InfoPlistSize);
  } else if (KernelAndKextPatches->KPDELLSMBIOS &&
             (AsciiStrStr(InfoPlist, "com.apple.driver.AppleACPIPlatform") != NULL)) {
    //
    // DellSMBIOS
    //
    // AppleACPIPlatform
    //
    DellSMBIOSPatch(Driver, DriverSize, InfoPlist, InfoPlistSize);
  } else if (gBDWEIOPCIFixRequire && (AsciiStrStr(InfoPlist, "com.apple.iokit.IOPCIFamily") != NULL)) {
    //
    // Broadwell-E IOPCIFamily Patch
    //
    BDWE_IOPCIPatch(Driver, DriverSize, InfoPlist, InfoPlistSize);
  } else if (gSNBEAICPUFixRequire && (AsciiStrStr(InfoPlist, "com.apple.driver.AppleIntelCPUPowerManagement") != NULL)) {
    //
    // SandyBridge-E AppleIntelCPUPowerManagement Patch implemented by syscl
    //
    SNBE_AICPUPatch(Driver, DriverSize, InfoPlist, InfoPlistSize);
  } else if (KernelAndKextPatches->EightApple &&
          /*   (AsciiStrStr(InfoPlist, "com.apple.iokit.IOGraphicsFamily") != NULL) && */
             (AsciiStrStr(InfoPlist, "I/O Kit Graphics Family") != NULL)) {
    //
    // Patch against 8 apple glitch
    //
    DBG_RT("Patch 8 apple required, IOGraphicsFamily...\n");
    EightApplePatch(Driver, DriverSize);
    Stall(10000000);
  }
  //com.apple.iokit.IOGraphicsFamily
  
    for (INT32 i = 0; i < KernelAndKextPatches->NrKexts; i++) {
      CHAR8 *Name = KernelAndKextPatches->KextPatches[i].Name;
      BOOLEAN   isBundle = (AsciiStrStr(Name, ".") != NULL);
      if ((KernelAndKextPatches->KextPatches[i].DataLen > 0) &&
          isBundle?(AsciiStrCmp(gKextBundleIdentifier, Name) == 0):(AsciiStrStr(gKextBundleIdentifier, Name) != NULL)) {
      //    (AsciiStrStr(InfoPlist, KernelAndKextPatches->KextPatches[i].Name) != NULL)) {
        DBG_RT("\n\nPatch kext: %s\n", KernelAndKextPatches->KextPatches[i].Name);
        AnyKextPatch(Driver, DriverSize, InfoPlist, InfoPlistSize, i);
      }
    }
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
  
  // search for <integer>
  IntTag = AsciiStrStr(Value, "<integer>"); //this is decimal value
  if (IntTag != NULL) {
    IntTag += 9; //next after ">"
    if ((IntTag[1] == 'x') || (IntTag[1] == 'X')) {
      return AsciiStrHexToUintn(IntTag);
    }
    if (IntTag[0] == '-') {
      return (UINTN)(-(INTN)AsciiStrDecimalToUintn(IntTag + 1));
    }
    return AsciiStrDecimalToUintn((IntTag[0] == '+') ? (IntTag + 1) : IntTag);
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
//    DBG("\nNo <integer end\n");
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
//       <key>CFBundleName</key> //No! we have CFBundleIdentifier
//       <string>MAC Framework Pseudoextension</string>
//       <key>_PrelinkExecutableLoadAddr</key>
//       <integer size="64">0xffffff7f8072f000</integer> || <integer>-549736448000</integer>
//       <!-- Kext size -->
//       <key>_PrelinkExecutableSize</key>
//       <integer size="64">0x3d0</integer>  || <integer>49152</integer>
//       <!-- Kext address -->
//       <key>_PrelinkExecutableSourceAddr</key>
//       <integer size="64">0xffffff80009a3000</integer>
//       ...
//      <key>OSBundleUUID</key>
//      <data>
//        rVVMo9l6M9K/lZm/X2BzEA==
//      </data>

//     </dict>
//     <!-- start of next kext Info.plist -->
//     <dict>
//       ...
//     </dict>
//       ...
VOID LOADER_ENTRY::PatchPrelinkedKexts()
{
  CHAR8     *WholePlist;
  CHAR8     *DictPtr;
  CHAR8     *InfoPlistStart = NULL;
  CHAR8     *InfoPlistEnd = NULL;
  INTN      DictLevel = 0;
  CHAR8     SavedValue;
  //INTN      DbgCount = 0;
  UINT64    KextAddr = 0;
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
 // CheckForFakeSMC(WholePlist);
  
  DBG("dump begin of WholePlist: ");
  for (int j=0; j<20; ++j) {
    DBG("%c", WholePlist[j]);
  }
  DBG("\n");

  DictPtr = WholePlist;
  //new dict is the new kext
  while ((DictPtr = strstr(DictPtr, "dict>")) != NULL) {
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
        // truncate to 32 bit to get physical addr? Yes!
        KextAddr = (UINT32)GetPlistHexValue(InfoPlistStart, kPrelinkExecutableSourceKey, WholePlist);
        // KextAddr is always relative to 0x200000
        // and if KernelSlide is != 0 then KextAddr must be adjusted
        KextAddr += KernelSlide;
        // and adjust for AptioFixDrv's KernelRelocBase
        KextAddr += KernelRelocBase;
        
        KextSize = (UINT32)GetPlistHexValue(InfoPlistStart, kPrelinkExecutableSizeKey, WholePlist);
//        DBG("found kext addr=0x%llX size=0x%X\n", KextAddr, KextSize);
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
VOID LOADER_ENTRY::PatchLoadedKexts()
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
  
  
  DBG("\nPatchLoadedKexts ... dtRoot = %llx\n", (UINTN)dtRoot);
  
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
                    KextFileInfo->infoDictLength
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
VOID LOADER_ENTRY::KextPatcherStart()
{
//  if (isKernelcache) {
    DBG_RT("Patching kernelcache ...\n");
      Stall(2000000);
    PatchPrelinkedKexts();
//  } else {
    DBG_RT("Patching loaded kexts ...\n");
      Stall(2000000);
    PatchLoadedKexts();
//  }
}

