/*
 * Copyright (c) 2011-2012 Frank Peng. All rights reserved.
 *
 */

#include "kernel_patcher.h"
#include "fakesmc.h"

#define KEXT_DEBUG 0

#if KEXT_DEBUG
#define DBG(x...)	Print(x);
#else
#define DBG(x...)
#endif

////////////////////////////////////
//
// ATIConnectorInfo patch
// bcc9's patch: http://www.insanelymac.com/forum/index.php?showtopic=249642
// 
// Currently still speciffic to pcj. Should be amended with some config to be generic.
//

// ATI card
UINT8 Hoolock[] =
{
    0x00, 0x04, 0x00, 0x00, 0x04, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x21, 0x03, 0x05, 0x01,
    0x00, 0x04, 0x00, 0x00, 0x04, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x11, 0x02, 0x04, 0x02,
    0x04, 0x00, 0x00, 0x00, 0x14, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x04, 0x01, 0x03
};

UINT8 Eulemur[] =
{
    0x04, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x02, 0x01, 0x04,
    0x00, 0x08, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x71, 0x00, 0x00, 0x12, 0x04, 0x04, 0x02,
    0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x01
};

// if you want apple icon show, connector port data should be on last port
UINT8 ATI[] =
{
    0x04, 0x00, 0x00, 0x00, 0x14, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x10, 0x11, 0x01, 0x02,   //DVI
    0x00, 0x08, 0x00, 0x00, 0x04, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x11, 0x22, 0x05, 0x04,   //HDMI
    0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x01    //VGA
};

VOID ATIConnectorInfoPatch(UINT8 *Driver, UINT32 DriverSize)
{
  
  UINTN   i;
  UINTN   count = 0;
  
  DBG(L"ATIConnectorInfoPatch: driverAddr = %x, driverSize = %x\n", Driver, DriverSize);
  
  for (i=0; i<DriverSize; i++)	 
  {   
    if (CompareMem(Driver + i, Hoolock, sizeof(Hoolock)) == 0)
    {
      DBG(L"Found ATI Code - patching\n");
      CopyMem(Driver + i, ATI, sizeof(ATI));
      
      if (count) break;
      
      count++;
    }
  }
  //gBS->Stall(5000000);
}


////////////////////////////////////
//
// AsusAICPUPM patch
//
// fLaked's SpeedStepper patch for Asus (and some other) boards:
// http://www.insanelymac.com/forum/index.php?showtopic=258611
// 

UINT8   MovlE2ToEcx[] = { 0xB9, 0xE2, 0x00, 0x00, 0x00 };
UINT8   Wrmsr[]       = { 0x0F, 0x30 };

VOID AsusAICPUPMPatch(UINT8 *Driver, UINT32 DriverSize)
{
  UINTN   Index1;
  UINTN   Index2;
  UINTN   Count = 0;
  
  DBG(L"AsusAICPUPMPatch: driverAddr = %x, driverSize = %x\n", Driver, DriverSize);
  
  // todo: we should scan only __text __TEXT
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
          DBG(L" %d. patched at 0x%x\n", Count, Index2);
        }
      }
    }
  }
  DBG(L"= %d patches\n", Count);
  //gBS->Stall(1000000);
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
// Called from SetFSInjection(), before boot.efi is started,
// to allow patchers to prepare FSInject to force load needed kexts.
//
VOID KextPatcherRegisterKexts(FSINJECTION_PROTOCOL *FSInject, FSI_STRING_LIST *ForceLoadKexts)
{
  if (gSettings.KPATIConnectorInfo) {
    FSInject->AddStringToList(ForceLoadKexts, L"\\ATI5000Controller.kext\\Contents\\Info.plist");
    FSInject->AddStringToList(ForceLoadKexts, L"\\IOGraphicsFamily.kext\\Info.plist");
    FSInject->AddStringToList(ForceLoadKexts, L"\\ATISupport.kext\\Contents\\Info.plist");
  }
}






//
// PatchKext is called for every kext from prelinked kernel (kernelcache) or from DevTree (booting with drivers).
// Add kext detection code here and call kext speciffic patch function.
//
VOID PatchKext(UINT8 *Driver, UINT32 DriverSize, CHAR8 *InfoPlist, UINT32 InfoPlistSize)
{
  
  if (gSettings.KPATIConnectorInfo
      && (AsciiStrStr(InfoPlist, "com.apple.kext.ATI5000Controller")
          || AsciiStrStr(InfoPlist, "com.apple.kext.AMD5000Controller" /* ML */)
          )
      ) {
    ATIConnectorInfoPatch(Driver, DriverSize);
  }
  else if (gSettings.KPAsusAICPUPM && AsciiStrStr(InfoPlist, "<string>com.apple.driver.AppleIntelCPUPowerManagement</string>")) {
    AsusAICPUPMPatch(Driver, DriverSize);
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
        KextAddr += KernelRelocBase;
        
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
                  InfoPlistEnd - InfoPlistStart
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
    
    PatchPrelinkedKexts();
    
  } else {
    
    PatchLoadedKexts();
    
  }
}


 
VOID InjectKernelCache(VOID* binary)
{
    UINT8* bytes = (UINT8*)(UINTN)PrelinkInfoAddr;
    //UINT32 i, j, k;
    //EFI_STATUS		Status = EFI_SUCCESS;
    //CHAR8*			plistBuffer;
    //CHAR8*          backupBuffer;
    //DTEntry							efiPlatform;
    //TagPtr			dict;
    //TagPtr          dictPointer;
    //TagPtr          dictPointer1;
    //TagPtr          dictPointer2;
    //TagPtr          dictPointer3;
	//TagPtr			prop;
	//INTN count, count1, count2, count3;
	//VOID* Data=0;
	//UINT32 DataSize=0;
	//CHAR8* data = "/Sandbox.kext</string></dict>"; //29
	//BOOLEAN check;
	/*
	if (dtRoot)
	{
		DTInit(dtRoot);

	    if (DTLookupEntry(NULL,"/efi/platform",&efiPlatform)==kSuccess)
	    {
            DTGetProperty(efiPlatform, "InfoPlist", &Data, &DataSize);
            //if (Data && DataSize)
            //{
                //DBG(L"Found /efi/platform/InfoPlist\n");
                //CopyMem(Data, plistBuffer, sizeof(plistBuffer));
                //FreePool(plistBuffer);
            //}
		}
	}
	*/
	//plistBuffer = AllocateZeroPool (PrelinkInfoSize+1+sizeof(Info_plist));
	//CopyMem(plistBuffer,(UINT8*)(UINTN)PrelinkInfoAddr, PrelinkInfoSize);
	/*
	for (i=0; i<(sizeof(PrelinkInfoSize)-29); i++)
	{
	    for (j=0; j<29; j++)
	    {
	        if (bytes[i+j] == data[j])
	        {
	            check = TRUE;
	            k = i + j;
	        }
	        else
	        {
	            check = FALSE;
	            break;
	        }
	    }
	    
	    if (check)
	    {
	    //    DBG(L"Found InfoPlist\n");
	    //    if (Data && DataSize)
	    //    {
	    //        CopyMem(Data, bytes, PrelinkInfoSize-15);
	    //        CopyMem(Data+(PrelinkInfoSize-15), Info_plist, sizeof(Info_plist));
	    //        CopyMem(Data+(PrelinkInfoSize-15)+sizeof(Info_plist), bytes+(PrelinkInfoSize-15), 15);
	            //backupBuffer = AllocateZeroPool (PrelinkInfoSize-k+1);
	            //CopyMem(backupBuffer,(UINT8*)(UINTN)PrelinkInfoAddr+k, PrelinkInfoSize-k);
	            //CopyMem(plistBuffer+k, Info_plist, sizeof(Info_plist));
	            //CopyMem(plistBuffer+k+sizeof(Info_plist), backupBuffer, PrelinkInfoSize-k);
	            //FreePool(backupBuffer);
	    //        break;
	    //    }
	    }
	}
	*/
					            
	/* 
	if (ParseXML(plistBuffer, &dict) != EFI_SUCCESS)
	{
        DBG(L"Not Found Plist\n");
    }
    else
    {
        if (dict)
        {
            DBG(L"ParseXML Done\n");
            dictPointer = GetProperty(dict, kPrelinkInfoDictionaryKey);
            if (dictPointer)
            {
                DBG(L"_PrelinkInfoDictionary Done\n");
                count = GetTagCount(dictPointer);
                DBG(L"dictPointer TagCount = %d\n", count);
                for (i=0; i<count; i++)
                {
                    if (GetElement(dictPointer, i, &dictPointer1) == EFI_SUCCESS)
                    {
                        count1 = GetTagCount(dictPointer1);
                        if (count1>1)
                        {
                            for (j=0; j<count1; j++)
                            {
                                if (GetElement(dictPointer1, j, &dictPointer2) == EFI_SUCCESS)
                                {
                                    count2 = GetTagCount(dictPointer2);
                                    if (count2> 1)
                                    {
                                        for (k=0; k<count2;k++)
                                        {
                                            if (GetElement(dictPointer2, j, &dictPointer3) == EFI_SUCCESS)
                                            {
                                                count3 = GetTagCount(dictPointer3);
                                                if (count3>1)
                                                {
                                                    DBG(L"too many item quit\n");
                                                    FreePool(plistBuffer);
                                                    goto jump;
                                                }
                                                else
                                                {
                                                    prop = GetProperty(dictPointer3, kPropCFBundleIdentifier);
                                                    if (prop)
                                                    {
                                                        DBG(L"Found Prop name = %a\n", prop->string);
		                                            }
		                                        }
		                                    }
		                                }
		                            }
		                            else
		                            {
		                                prop = GetProperty(dictPointer2, kPropCFBundleIdentifier);
                                        if (prop)
                                        {
                                            DBG(L"Found Prop name = %a\n", prop->string);
		                                }
		                            }
		                        }
		                    }
		                }
		                else
		                {
                            prop = GetProperty(dictPointer2, kPropCFBundleIdentifier);
                            if (prop)
                            {
                                DBG(L"Found Prop name = %a\n", prop->string);
		                    }
		                }    
		            }
		        }
		    }
		}
		FreePool(plistBuffer);
    }
jump: 
    for (i=0;i<PrelinkInfoSize;i++)
    {
        if (bytes[i] == 'F' && bytes[i+1] == 'a' && bytes[i+2] == 'k' && bytes[i+3] == 'e' &&
            bytes[i+4] == 'S' && bytes[i+5] == 'M' && bytes[i+6] == 'C')
        {
            DBG(L"Found FakeSMC plist\n");
            break;
        }
    }
    */
    if (PrelinkInfoAddr && PrelinkInfoSize)
    {       
        DBG(L"InjectKernelCache Start\n");
        UINT8* NewKextAddr = (UINT8*)(UINTN)0x1C100000;
        CopyMem(NewKextAddr, FakeSMC, sizeof(FakeSMC));
        CopyMem(NewKextAddr+sizeof(FakeSMC), (UINT8*)(UINTN)PrelinkTextAddr, PrelinkTextSize);
        UINT8* NewKextInfoAddr = NewKextAddr+PrelinkTextSize+sizeof(FakeSMC);
        CopyMem(NewKextInfoAddr, (UINT8*)(UINTN)bytes, PrelinkInfoSize-15);
        CopyMem(NewKextInfoAddr+(PrelinkInfoSize-15), Info_plist, sizeof(Info_plist));
        CopyMem(NewKextInfoAddr+(PrelinkInfoSize-15)+sizeof(Info_plist), (UINT8*)(UINTN)bytes+(PrelinkInfoSize-15), 15);
        
        struct segment_command *segCmd;
        struct segment_command_64 *segCmd64;
        struct section_64 *sect64;
        struct section    *sect;
    
        switch (MACH_GET_MAGIC(binary))
	    {
		      case MH_MAGIC:
			       segCmd = (struct segment_command*)(UINTN)PrelinkTextLoadCmdAddr;
			       segCmd->vmaddr = (UINT32)(UINTN)NewKextAddr;
	               segCmd->vmsize += sizeof(FakeSMC);
	               segCmd->filesize += sizeof(FakeSMC);
	               sect = (struct section*)(UINTN)PrelinkInfoLoadCmdAddr;
	               sect->addr = (UINT32)(UINTN)NewKextInfoAddr;
	               sect->size += sizeof(Info_plist);	
			       break;
		      case MH_MAGIC_64:
			       segCmd64 = (struct segment_command_64*)(UINTN)PrelinkTextLoadCmdAddr;
			       segCmd64->vmaddr = (UINT64)(UINTN)NewKextAddr;
	               segCmd64->vmsize += sizeof(FakeSMC);
	               segCmd64->filesize += sizeof(FakeSMC);
	               sect64 = (struct section_64*)(UINTN)PrelinkInfoLoadCmdAddr;
	               sect64->addr = (UINT64)(UINTN)NewKextInfoAddr;
	               sect64->size += sizeof(Info_plist);	               
			       break;
		      default:
			       return;
	    }        
    }
    DBG(L"InjectKernelCache Done\n");  
}

