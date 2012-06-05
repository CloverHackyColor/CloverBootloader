/*
 * Copyright (c) 2011-2012 Frank Peng. All rights reserved.
 *
 */

#include "kernel_patcher.h"
#include "fakesmc.h"
#include "device_tree.h"

#define KEXT_DEBUG 1

#if KEXT_DEBUG
#define DBG(x...)	Print(x);
#else
#define DBG(x...)
#endif

#define kPrelinkTextSegment                "__PRELINK_TEXT"
#define kPrelinkTextSection                "__text"

#define kPrelinkLinkStateSegment           "__PRELINK_STATE"
#define kPrelinkKernelLinkStateSection     "__kernel"
#define kPrelinkKextsLinkStateSection      "__kexts"

#define kPrelinkInfoSegment                "__PRELINK_INFO"
#define kPrelinkInfoSection                "__info"

#define kPrelinkBundlePathKey              "_PrelinkBundlePath"
#define kPrelinkExecutableRelativePathKey  "_PrelinkExecutableRelativePath"
#define kPrelinkExecutableLoadKey          "_PrelinkExecutableLoadAddr"
#define kPrelinkExecutableSourceKey        "_PrelinkExecutableSourceAddr"
#define kPrelinkExecutableSizeKey          "_PrelinkExecutableSize"
#define kPrelinkInfoDictionaryKey          "_PrelinkInfoDictionary"
#define kPrelinkInterfaceUUIDKey           "_PrelinkInterfaceUUID"
#define kPrelinkKmodInfoKey                "_PrelinkKmodInfo"
#define kPrelinkLinkStateKey               "_PrelinkLinkState"
#define kPrelinkLinkStateSizeKey           "_PrelinkLinkStateSize"

#define kPropCFBundleIdentifier ("CFBundleIdentifier")
#define kPropCFBundleExecutable ("CFBundleExecutable")
#define kPropOSBundleRequired   ("OSBundleRequired")
#define kPropOSBundleLibraries  ("OSBundleLibraries")
#define kPropIOKitPersonalities ("IOKitPersonalities")
#define kPropIONameMatch        ("IONameMatch")

typedef struct _BooterKextFileInfo {
    UINT32  infoDictPhysAddr;
    UINT32  infoDictLength;
    UINT32  executablePhysAddr;
    UINT32  executableLength;
    UINT32  bundlePathPhysAddr;
    UINT32  bundlePathLength;
} _BooterKextFileInfo;

typedef struct _DeviceTreeBuffer {
    uint32_t paddr;
    uint32_t length;
} _DeviceTreeBuffer;

UINT32     KextAddr=0;
UINT32     KextAddr1=0;
UINT32     KextLength=0;
UINT32     KextInfoAddr=0;
UINT32     KextInfoAddr1=0;
UINT32     KextInfoLength=0;

VOID KextPatcher_Start()
{

    VOID*      KernelData=(VOID*)0x00200000;

    while(TRUE)
    {
      	// Parse through the load commands
        if(MACH_GET_MAGIC(KernelData) == MH_MAGIC || MACH_GET_MAGIC(KernelData) == MH_MAGIC_64)
	    {
	        //DBG(L"found Kernel address patch start!\n");
            break;
        }
        KernelData += 1;
    }
    
    Get_PreLink(KernelData);
    
    if (KextAddr!=0 && KextLength !=0 && (DisplayVendor[0] == 0x1002 || DisplayVendor[1] == 0x1002))
    {
        KextPatcher_ATI(KextAddr, KextLength);
    }
    
    if (KextAddr!=0 && KextLength ==0 && (DisplayVendor[0] == 0x1002 || DisplayVendor[1] == 0x1002))
    {
        KextPatcher_driver_ATI();
    }
    
    //if (KextAddr!=0 && KextLength!=0)
    //{
    //    InjectKernelCache(KernelData);
    //}
}
    
VOID Get_PreLink(VOID* binary)
{
  UINT32 ncmds, cmdsize;
  UINT32 binaryIndex;
  UINTN  cnt;
    
  switch (MACH_GET_MAGIC(binary))
	{
		case MH_MAGIC:
			binaryIndex = sizeof(struct mach_header);
			break;
		case MH_MAGIC_64:
			binaryIndex = sizeof(struct mach_header_64);
			break;
		default:
			return;
	}
/*			
  DBG(L"magic:      %x\n", (unsigned)mH->magic);
  DBG(L"cputype:    %x\n", (unsigned)mH->cputype);
  DBG(L"cpusubtype: %x\n", (unsigned)mH->cpusubtype);
  DBG(L"filetype:   %x\n", (unsigned)mH->filetype);
  DBG(L"ncmds:      %x\n", (unsigned)mH->ncmds);
  DBG(L"sizeofcmds: %x\n", (unsigned)mH->sizeofcmds);
  DBG(L"flags:      %x\n", (unsigned)mH->flags);
*/
  ncmds = MACH_GET_NCMDS(binary);
  
  struct load_command *loadCommand = NULL;
  
  //struct segment_command *segCmd;
  struct segment_command_64 *segCmd64;
  
  for (cnt = 0; cnt < ncmds; cnt++) {
      loadCommand = binary + binaryIndex;
      cmdsize = loadCommand->cmdsize;
    
      switch (loadCommand->cmd) 
      {
	        case LC_SEGMENT_64: 
	             segCmd64 = binary + binaryIndex;	
	             //DBG(L"segCmd64->segname = %a\n",segCmd64->segname);
	             //DBG(L"segCmd64->vmaddr = 0x%08x\n",segCmd64->vmaddr)
	             //DBG(L"segCmd64->vmsize = 0x%08x\n",segCmd64->vmsize); 
	             if (AsciiStrCmp(segCmd64->segname, "__PRELINK_TEXT") == 0)
	             {
	                 KextAddr = segCmd64->vmaddr;
	                 KextLength = segCmd64->vmsize;
	                 KextAddr1 = (UINT32)(UINTN)binary + binaryIndex;
	                 //DBG(L"cmd = 0x%08x\n",segCmd64->cmd);
	                 //DBG(L"cmdsize = 0x%08x\n",segCmd64->cmdsize);
	                 //DBG(L"vmaddr = 0x%08x\n",segCmd64->vmaddr);
	                 //DBG(L"vmsize = 0x%08x\n",segCmd64->vmsize);
	                 //DBG(L"fileoff = 0x%08x\n",segCmd64->fileoff);
	                 //DBG(L"filesize = 0x%08x\n",segCmd64->filesize);
	                 //DBG(L"maxprot = 0x%08x\n",segCmd64->maxprot);
	                 //DBG(L"initprot = 0x%08x\n",segCmd64->initprot);
	                 //DBG(L"nsects = 0x%08x\n",segCmd64->nsects);
	                 //DBG(L"flags = 0x%08x\n",segCmd64->flags);
	                 //DBG(L"Found PRELINK_TEXT\n");
	             }
	             if (AsciiStrCmp(segCmd64->segname, "__PRELINK_INFO") == 0)
	             {
	                 //KextInfoAddr = segCmd64->vmaddr;
	                 //KextInfoLength = segCmd64->vmsize;
	                 //DBG(L"cmd = 0x%08x\n",segCmd64->cmd);
	                 //DBG(L"cmdsize = 0x%08x\n",segCmd64->cmdsize);
	                 //DBG(L"vmaddr = 0x%08x\n",segCmd64->vmaddr);
	                 //DBG(L"vmsize = 0x%08x\n",segCmd64->vmsize);
	                 //DBG(L"fileoff = 0x%08x\n",segCmd64->fileoff);
	                 //DBG(L"filesize = 0x%08x\n",segCmd64->filesize);
	                 //DBG(L"maxprot = 0x%08x\n",segCmd64->maxprot);
	                 //DBG(L"initprot = 0x%08x\n",segCmd64->initprot);
	                 //DBG(L"nsects = 0x%08x\n",segCmd64->nsects);
	                 //DBG(L"flags = 0x%08x\n",segCmd64->flags);
	                 //DBG(L"Found PRELINK_INFO\n");
	                 UINT32 sectionIndex;
                     sectionIndex = sizeof(struct segment_command_64);
                     struct section_64 *sect;
                    
                     while(sectionIndex < segCmd64->cmdsize)
                     {
                         sect = binary + binaryIndex + sectionIndex;
                         sectionIndex += sizeof(struct section_64);
                                                
                         if(AsciiStrCmp(sect->sectname, "__info") == 0 && AsciiStrCmp(sect->segname, "__PRELINK_INFO") == 0)
                         {
                             // __TEXT,__text found, save the offset and address for when looking for the calls.
                             //DBG(L"__info found address = 0x%08x\n",sect->addr);
                             //DBG(L"__info found size = 0x%08x\n",sect->size);
                             KextInfoAddr1 = (UINT32)(UINTN)binary + binaryIndex + sectionIndex;
                             KextInfoAddr = sect->addr;
	                         KextInfoLength = sect->size;
                             //CHAR8* DATA = AllocateZeroPool (sect->size);
                             //CopyMem(DATA, (CHAR8*)&sect->addr, sect->size);
                             //TagPtr						dict=NULL;
                             //if(ParseXML(DATA, &dict) == EFI_SUCCESS)
		                     //{
			                 //   DBG(L"%a\n",dict);
		                     //}
                             //textSection = sect->offset;
                             //textAddress = sect->addr;
                         }					
                     }	
	             } /*
	             if (AsciiStrCmp(segCmd64->segname, "__TEXT") == 0)
	             {
	                 KextInfoAddr = segCmd64->vmaddr;
	                 KextInfoLength = segCmd64->vmsize;
	                 //DBG(L"cmd = 0x%08x\n",segCmd64->cmd);
	                 //DBG(L"cmdsize = 0x%08x\n",segCmd64->cmdsize);
	                 //DBG(L"vmaddr = 0x%08x\n",segCmd64->vmaddr);
	                 //DBG(L"vmsize = 0x%08x\n",segCmd64->vmsize);
	                 //DBG(L"fileoff = 0x%08x\n",segCmd64->fileoff);
	                 //DBG(L"filesize = 0x%08x\n",segCmd64->filesize);
	                 //DBG(L"maxprot = 0x%08x\n",segCmd64->maxprot);
	                 //DBG(L"initprot = 0x%08x\n",segCmd64->initprot);
	                 //DBG(L"nsects = 0x%08x\n",segCmd64->nsects);
	                 //DBG(L"flags = 0x%08x\n",segCmd64->flags);
	                 //DBG(L"Found PRELINK_TEXT\n");
	                 UINT32 sectionIndex;
                     sectionIndex = sizeof(struct segment_command_64);
                     struct section_64 *sect;
                    
                     while(sectionIndex < segCmd64->cmdsize)
                     {
                         sect = binary + binaryIndex + sectionIndex;
                        
                         sectionIndex += sizeof(struct section_64);
                        
                         //if(section_handler) section_handler(sect->sectname, segCommand->segname, sect->offset, sect->addr);
                        
                         if(AsciiStrCmp(sect->sectname, "__txet") == 0)
                         {
                             // __TEXT,__text found, save the offset and address for when looking for the calls.
                             //DBG(L"__text found address = 0x%08x\n",sect->addr);
                             //DBG(L"_cpuid_set_info found address = 0x%08x\n",sect->size);
                             //CHAR8* DATA = AllocateZeroPool (sect->size);
                             //CopyMem(DATA, (CHAR8*)&sect->addr, sect->size);
                             //TagPtr						dict=NULL;
                             //if(ParseXML(DATA, &dict) == EFI_SUCCESS)
		                     //{
			                 //   DBG(L"%a\n",dict);
		                     //}
                             
                             //textSection = sect->offset;
                             //textAddress = sect->addr;
                         }					
                     }	
	             }*/
	             break; /*
            case LC_SEGMENT:
            	 segCmd = binary + binaryIndex;	 
            	 //DBG(L"segCmd->segname = %a\n",segCmd->segname);
	             //DBG(L"segCmd->vmaddr = 0x%08x\n",segCmd->vmaddr)
	             //DBG(L"segCmd->vmsize = 0x%08x\n",segCmd->vmsize);
	             if (AsciiStrCmp(segCmd->segname, "__PRELINK_TEXT") == 0)
	             {
	                 KextAddr = segCmd->vmaddr;
	                 KextLength = segCmd->vmsize;
	                 //DBG(L"prelinkData = 0x%08x\n",KextAddr);
	                 //DBG(L"preLinksize = 0x%08x\n",KextLength);
	                 //DBG(L"Found PRELINK_TEXT\n");
	             }
	             if (AsciiStrCmp(segCmd->segname, "__PRELINK_INFO") == 0)
	             {
	                 KextInfoAddr = segCmd->vmaddr;
	                 KextInfoLength = segCmd->vmsize;
	                 //DBG(L"prelinkData = 0x%08x\n",KextInfoAddr);
	                 //DBG(L"preLinksize = 0x%08x\n",KextInfoLength);
	                 //DBG(L"Found PRELINK_INFO\n");
	             }
                 break; */
            default:
                 break;
      }  
      binaryIndex += cmdsize;
  }	
  return;
}

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

VOID KextPatcher_driver_ATI()
{     
    UINT64              plistAddr=0;
    UINT64              plistSize=0;
    UINT64              driverAddr=0;
    UINT64              driverSize=0;
	DTEntry				memoryMap;
    _BooterKextFileInfo * kextFileInfo   = NULL;
    //const _DeviceTreeBuffer   * deviceTreeBuffer        = NULL;
    CHAR8*				plistBuffer;
    TagPtr				dict;
	TagPtr				prop;
	CHAR8*              temp=0;
	UINT64              temp1;
	UINTN count=0;
	BOOLEAN check = FALSE;
      
    if (dtRoot)
    {
        DTInit(dtRoot);
	    if (DTLookupEntry(NULL,"/chosen/memory-map",&memoryMap))
	    {
		    DTPropertyIterator mmPropIter;
            CHAR8*	ptrmm;
		    if (DTCreatePropertyIterator(memoryMap,&mmPropIter))
		    {   
 			    while (DTIterateProperties(mmPropIter,&ptrmm))
			    {	
				    if (AsciiStrStr(ptrmm,"Driver-")!=0)
				    {
				        temp = AllocateZeroPool (AsciiStrLen(ptrmm)-6);
				        CopyMem(temp, ptrmm+7, AsciiStrLen(ptrmm)-6);
				        //DTGetProperty(memoryMap, ptrmm, &driverAddr, &driverSize);
				        //if (count<3)
				        //{
         				//   DBG(L"Found %a, temp = %a, change = 0x%08x\n", ptrmm, temp, AsciiStrHexToUint64(temp));
         				//} 
                        temp1 = AsciiStrHexToUint64(temp);
                        
                        //(const _DeviceTreeBuffer *)
                        //deviceTreeEntry->getBytesNoCopy(0, sizeof(deviceTreeBuffer));
				        kextFileInfo = (_BooterKextFileInfo *)(UINTN)temp1;
				        //if (count<3)
				        //{
				        //    DBG(L"Driver addr = 0x%08x, Length = 0x%08x\n", driverAddr, driverSize);
				        //    DBG(L"Driver addr = 0x%08x, Length = 0x%08x\n", kextFileInfo->executablePhysAddr, kextFileInfo->executableLength);
				        //    DBG(L"Plist addr = 0x%08x, Length = 0x%08x\n", kextFileInfo->infoDictPhysAddr, kextFileInfo->infoDictLength);
				        //}
				        plistAddr = kextFileInfo->infoDictPhysAddr;
				        plistSize = kextFileInfo->infoDictLength;
				        driverAddr = kextFileInfo->executablePhysAddr;
				        driverSize = kextFileInfo->executableLength;
				        plistBuffer = AllocateZeroPool ((kextFileInfo->infoDictLength)+1);
				        CopyMem(plistBuffer,(UINT8*)(UINTN)kextFileInfo->infoDictPhysAddr, kextFileInfo->infoDictLength);
				        if(ParseXML(plistBuffer, &dict) != EFI_SUCCESS)
		                {
		                    //if (count<4)
		                    //{
		                    //    DBG(L"Not Found Plist\n");
			                //}
		                }
		                else
		                {
		                    if (dict)
		                    {
		                        //DBG(L"ParseXML Done\n");
		                        prop = GetProperty(dict, "CFBundleIdentifier");
		                        if (prop)
		                        {
		                            //DBG(L"Found Prop\n");
		                            if (AsciiStrStr(prop->string, "ATI5000"))
		                            {
		                                DBG(L"Found ATI5000Controller\n");
				                        KextPatcher_ATI(kextFileInfo->executablePhysAddr, kextFileInfo->executableLength);
				                        FreePool(plistBuffer);
				                        FreePool(temp);
				                        FreePool(dict);
				                        check = TRUE;
				                        break;
				                    }
				                    if (AsciiStrStr(prop->string, "FakeSMC"))
		                            {
		                                DBG(L"Found FakeSMC\n");
				                    }
				                }
				            }
				        }    
				        count++;
				        FreePool(plistBuffer);
				        FreePool(temp);
				        FreePool(dict);

				    }
				    //if(AsciiStrStr(ptrmm,"DriversPackage-")!=0)
				    //{
				    //    DBG(L"Found %a\n", ptrmm);
				    //    break;
				    //}
				}
				
				//if (!check)
				//{
			    //    KextPatcher_ATI(0x000A0000, 0x9C100000);
				//}
		        //DBG(L"Driver count = %d\n", count);
		    } 
	    }
	}
}
 
VOID KextPatcher_ATI(UINT32 driverAddr, UINT32 driverSize)
{
 
    UINT8* bytes = (UINT8*)(UINTN)driverAddr;
    UINTN i, j;
    UINTN count=0;
    BOOLEAN CHECK=FALSE;
    
    //DBG(L"KextPatcher_ATI Start.\n");
    
    for (i=0; i<driverSize; i++)	 
    {   
        for (j=0; j<(sizeof(Eulemur)); j++)
        {
            if (bytes[i+j] == Hoolock[j])
            {
                CHECK = TRUE;
            }
            else
            {
                CHECK = FALSE;
                break;
            }
        }
              
        if (CHECK)
        {
            //DBG(L"Found ATI Code\n");
            for (j=0; j<(sizeof(ATI)); j++)
            {
                bytes[i+j] = ATI[j];
            }
            CHECK = FALSE;
            if (count) break;
            
            count++;
        }
        i++;
    }
}

VOID InjectKernelCache(VOID* binary)
{
    UINT8* bytes = (UINT8*)(UINTN)KextInfoAddr;
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
	//plistBuffer = AllocateZeroPool (KextInfoLength+1+sizeof(Info_plist));
	//CopyMem(plistBuffer,(UINT8*)(UINTN)KextInfoAddr, KextInfoLength);
	/*
	for (i=0; i<(sizeof(KextInfoLength)-29); i++)
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
	    //        CopyMem(Data, bytes, KextInfoLength-15);
	    //        CopyMem(Data+(KextInfoLength-15), Info_plist, sizeof(Info_plist));
	    //        CopyMem(Data+(KextInfoLength-15)+sizeof(Info_plist), bytes+(KextInfoLength-15), 15);
	            //backupBuffer = AllocateZeroPool (KextInfoLength-k+1);
	            //CopyMem(backupBuffer,(UINT8*)(UINTN)KextInfoAddr+k, KextInfoLength-k);
	            //CopyMem(plistBuffer+k, Info_plist, sizeof(Info_plist));
	            //CopyMem(plistBuffer+k+sizeof(Info_plist), backupBuffer, KextInfoLength-k);
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
    for (i=0;i<KextInfoLength;i++)
    {
        if (bytes[i] == 'F' && bytes[i+1] == 'a' && bytes[i+2] == 'k' && bytes[i+3] == 'e' &&
            bytes[i+4] == 'S' && bytes[i+5] == 'M' && bytes[i+6] == 'C')
        {
            DBG(L"Found FakeSMC plist\n");
            break;
        }
    }
    */
    if (KextInfoAddr && KextInfoLength)
    {       
        DBG(L"InjectKernelCache Start\n");
        UINT8* NewKextAddr = (UINT8*)(UINTN)0x1C100000;
        CopyMem(NewKextAddr, FakeSMC, sizeof(FakeSMC));
        CopyMem(NewKextAddr+sizeof(FakeSMC), (UINT8*)(UINTN)KextAddr, KextLength);
        UINT8* NewKextInfoAddr = NewKextAddr+KextLength+sizeof(FakeSMC);
        CopyMem(NewKextInfoAddr, (UINT8*)(UINTN)bytes, KextInfoLength-15);
        CopyMem(NewKextInfoAddr+(KextInfoLength-15), Info_plist, sizeof(Info_plist));
        CopyMem(NewKextInfoAddr+(KextInfoLength-15)+sizeof(Info_plist), (UINT8*)(UINTN)bytes+(KextInfoLength-15), 15);
        
        struct segment_command *segCmd;
        struct segment_command_64 *segCmd64;
        struct section_64 *sect64;
        struct section    *sect;
    
        switch (MACH_GET_MAGIC(binary))
	    {
		      case MH_MAGIC:
			       segCmd = (struct segment_command*)(UINTN)KextAddr1;
			       segCmd->vmaddr = (UINT32)(UINTN)NewKextAddr;
	               segCmd->vmsize += sizeof(FakeSMC);
	               segCmd->filesize += sizeof(FakeSMC);
	               sect = (struct section*)(UINTN)KextInfoAddr1;
	               sect->addr = (UINT32)(UINTN)NewKextInfoAddr;
	               sect->size += sizeof(Info_plist);	
			       break;
		      case MH_MAGIC_64:
			       segCmd64 = (struct segment_command_64*)(UINTN)KextAddr1;
			       segCmd64->vmaddr = (UINT64)(UINTN)NewKextAddr;
	               segCmd64->vmsize += sizeof(FakeSMC);
	               segCmd64->filesize += sizeof(FakeSMC);
	               sect64 = (struct section_64*)(UINTN)KextInfoAddr1;
	               sect64->addr = (UINT64)(UINTN)NewKextInfoAddr;
	               sect64->size += sizeof(Info_plist);	               
			       break;
		      default:
			       return;
	    }        
    }
    DBG(L"InjectKernelCache Done\n");  
}