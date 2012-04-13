/*
 Slice 2012
*/

#include "Platform.h"

#define DEBUG_SET 1

#if DEBUG_SET == 2
#define DBG(x...) AsciiPrint(x)
#elif DEBUG_SET == 1
#define DBG(x...) MsgLog(x)
#else
#define DBG(x...)
#endif

#define kXMLTagArray   		"array"

EFI_GUID gRandomUUID = { 0x0A0B0C0D, 0x0000, 0x1010, {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 }};
CHAR8                           gSelectedUUID[40];
SETTINGS_DATA                   gSettings;
GFX_PROPERTIES                  gGraphics;
EFI_EDID_DISCOVERED_PROTOCOL    *EdidDiscovered;
//EFI_GRAPHICS_OUTPUT_PROTOCOL    *GraphicsOutput;
UINT16                          gCPUtype;

VOID WaitForSts(VOID) {
	UINT32 inline_timeout = 100000;
	while (AsmReadMsr64(MSR_IA32_PERF_STATUS) & (1 << 21)) { if (!inline_timeout--) break; }
}

EFI_STATUS GetNVRAMSettings(IN EFI_FILE *RootDir, CHAR16* NVRAMPlistPath)
{
	EFI_STATUS	Status;
	UINTN     size;
	CHAR8*		gNvramPtr;
	CHAR8*		efiBootDevice;
	TagPtr		dict;
	TagPtr		prop;
	TagPtr		dictPointer;
	UINT32		pos = 0;
	
	Status = egLoadFile(RootDir, NVRAMPlistPath, (UINT8**)&gNvramPtr, &size);
	if(EFI_ERROR(Status))
	{
		DBG(" nvram.plist not present\n");
		return Status;
	}
  /* check again:
	 fileInfo->EFI_TIME  ModificationTime;
	 /// EFI Time Abstraction:
	 ///  Year:       1900 - 9999
	 ///  Month:      1 - 12
	 ///  Day:        1 - 31
	 ///  Hour:       0 - 23
	 ///  Minute:     0 - 59
	 ///  Second:     0 - 59
	 ///  Nanosecond: 0 - 999,999,999
	 ///  TimeZone:   -1440 to 1440 or 2047
	 If we found nvram.plist at startVolume then we compare ModificationTimes, and use it if
	 the file is more recent then loaded.
   */	 
  
  
	if(gNvramPtr)
	{		
		if(ParseXML((const CHAR8*)gNvramPtr, &dict) != EFI_SUCCESS)
		{
			DBG("nvram file error\n");
			return EFI_UNSUPPORTED;
		}
	//for a example	
/*		prop = GetProperty(dict, "boot-args");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.BootArgs);
		}
 */
		prop = GetProperty(dict, "efi-boot-device");
		if(prop)
		{
			efiBootDevice = XMLDecode(prop->string);
			MsgLog("efi-boot-device = %a\n", efiBootDevice); 
			
	//		Status = XMLParseNextTag(efiBootDevice, &dictPointer, &size);
			while (TRUE)
			{
				Status = XMLParseNextTag(efiBootDevice + pos, &dict, (UINT32 *)&size);
				if (EFI_ERROR(Status))
					break;
				
				pos += size;
				MsgLog("parsing pos=%d size=%d\n", pos, size);
				if (dict == NULL) 
					continue;
				if (dict->type == kTagTypeArray) 
					break;
				
				FreeTag(dict);
			}
			
			if(dict)
			{
				MsgLog("Parse efi-boot-device success, size=%d!\n", size);
				DBG("dict type = %d\n", dict->type);
				if (dict->type == kTagTypeArray) {
					dict = dict->tag; //go inside
					DBG("  next dict type = %d\n", dict->type);
				}
				
				prop = GetProperty(dict, "IOMatch");
				if (prop) {
					DBG("IOMatch success!\n");
					dictPointer = prop;
					prop = GetProperty(dictPointer, "IOPropertyMatch");
					if (prop) {
						DBG("IOPropertyMatch success!\n");
						dictPointer = prop;
						prop = GetProperty(dictPointer, "UUID");
						if(prop)
						{
							MsgLog("UUID property type=%d string=%a\n", prop->type, prop->string);
              //AsciiStrToUnicodeStr(prop->string, gSelectedUUID);
              AsciiStrCpy(gSelectedUUID, prop->string);
						}									
					}
				}				
			}
		}	else {
      Status = EFI_NOT_FOUND;
    }
    
		SaveSettings();
	}
  else {
    return EFI_NOT_FOUND;
  }

	return Status;
}	



EFI_STATUS GetUserSettings(IN EFI_FILE *RootDir)
{
	EFI_STATUS	Status = EFI_NOT_FOUND;
	UINTN       size;
	TagPtr      dict;
	TagPtr      prop;
	TagPtr      dictPointer;
	CHAR8*      gConfigPtr = NULL;
  
  CHAR16      UStr[64];  
  CHAR16*     ConfigPlistPath = L"EFI\\config.plist";
  CHAR16*     ConfigOemPath = PoolPrint(L"%s\\config.plist", OEMPath);
	
	// load config
  if (FileExists(SelfRootDir, ConfigOemPath)) {
    Status = egLoadFile(SelfRootDir, ConfigOemPath, (UINT8**)&gConfigPtr, &size);
  } else {
    DBG("Oem config.plist not found at path: %s\n", ConfigOemPath);
  }
  
  if (EFI_ERROR(Status)) {
    if ((RootDir != NULL) && FileExists(RootDir, ConfigPlistPath)) {
      Status = egLoadFile(RootDir, ConfigPlistPath, (UINT8**)&gConfigPtr, &size);
    } 
    if (EFI_ERROR(Status)) {
      Status = egLoadFile(SelfRootDir, ConfigPlistPath, (UINT8**)&gConfigPtr, &size);
    }
  }	else {
    DBG("Using OEM config.plist at path: %s\n", ConfigOemPath);
  }
  
  
	if(EFI_ERROR(Status)) {
		DBG("Error loading config.plist! Status=%r\n", Status);
		return Status;
	}
	if(gConfigPtr)
	{		
		if(ParseXML((const CHAR8*)gConfigPtr, &dict) != EFI_SUCCESS)
		{
			DBG(" config error\n");
			return EFI_UNSUPPORTED;
		}
		//*** SYSTEM ***//
    dictPointer = GetProperty(dict,"SystemParameters");
    if (dictPointer) {
      prop = GetProperty(dictPointer,"prev-lang:kbd");
      if(prop)
      {
        AsciiStrToUnicodeStr(prop->string, gSettings.Language);
      }
      
      prop = GetProperty(dictPointer,"boot-args");
      if(prop)
      {
        AsciiStrCpy(gSettings.BootArgs, prop->string);
      } 
      
      prop = GetProperty(dictPointer,"DefaultBootVolume");
      if(prop)
      {
        AsciiStrToUnicodeStr(prop->string, gSettings.DefaultBoot);
      }
      prop = GetProperty(dictPointer,"CustomUUID");
      if(prop)
      {
        AsciiStrToUnicodeStr(prop->string, gSettings.CustomUuid);
        Status = StrToGuidLE(gSettings.CustomUuid, &gUuid);
        //else value from SMBIOS
      }      
    }
//Graphics
    
    dictPointer = GetProperty(dict,"Graphics");
    if (dictPointer) {
      prop = GetProperty(dictPointer,"GraphicsInjector");
      if(prop)
      {
        if ((prop->string[0] == 'n') || (prop->string[0] == 'N'))
          gSettings.GraphicsInjector=FALSE;
      }      
      prop = GetProperty(dictPointer,"VRAM");
      if(prop)
      {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.VRAM = (UINT64)StrDecimalToUintn((CHAR16*)&UStr[0]) << 20;  //bytes
      }
      
      prop = GetProperty(dictPointer,"LoadVBios");
      gSettings.LoadVBios = FALSE;
      if(prop)
      {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.LoadVBios = TRUE;
      }
      prop = GetProperty(dictPointer,"VideoPorts");
      if(prop)
      {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.VideoPorts = (UINT16)StrDecimalToUintn((CHAR16*)&UStr[0]);
        
      }
      prop = GetProperty(dictPointer,"FBName");
      if(prop)
      {
        AsciiStrToUnicodeStr(prop->string, gSettings.FBName);
      }
      
      prop = GetProperty(dictPointer,"NVCAP");
      if(prop)
      {      
        hex2bin(prop->string, (UINT8*)&gSettings.NVCAP[0], 20);
      } 
      prop = GetProperty(dictPointer,"display-cfg");
      if(prop)
      {      
        hex2bin(prop->string, (UINT8*)&gSettings.Dcfg[0], 8);
      } 
      
    }    

    dictPointer = GetProperty(dict,"PCI");
    if (dictPointer) {
      
      prop = GetProperty(dictPointer,"PCIRootUID");
      gSettings.PCIRootUID = 0;
      if(prop)
      {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.PCIRootUID = (UINT16)StrDecimalToUintn((CHAR16*)&UStr[0]);
      }
      prop = GetProperty(dictPointer,"StringInjector");
      gSettings.StringInjector = FALSE;
      if(prop)
      {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.StringInjector = TRUE;
      }
      prop = GetProperty(dictPointer,"DeviceProperties");
      if(prop)
      {
        cDeviceProperties = AllocateZeroPool(AsciiStrLen(prop->string)+1);
        AsciiStrCpy(cDeviceProperties, prop->string);
      }
      prop = GetProperty(dictPointer,"LpcTune");
      gSettings.LpcTune = FALSE;
      if(prop)
      {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.LpcTune = TRUE;
      }
      // HDA
      prop = GetProperty(dictPointer,"HDAInjection");
      if(prop)
      {
        // enabled by default
        // syntax:
        // - HDAInjection=No - disables injection
        // - HDAInjection=887 - injects layout-id 887 decimal (0x00000377)
        // - HDAInjection=0x377 - injects layout-id 887 decimal (0x00000377)
        // - HDAInjection=Detect - reads codec device id (eg. 0x0887)
        //   converts it to decimal 887 and injects this as layout-id.
        //   if hex device is cannot be converted to decimal, injects legacy value 12 decimal
        // - all other values are equal to HDAInjection=Detect
        if ((prop->string[0] == 'n') || (prop->string[0] == 'N')) {
          // if starts with n or N, then no HDA injection
          gSettings.HDAInjection = FALSE;
        } else if ((prop->string[0] == '0')  && 
                   (prop->string[1] == 'x' || prop->string[1] == 'X')) {
          // assume it's a hex layout id
          gSettings.HDALayoutId = AsciiStrHexToUintn(prop->string);
        } else {
          // assume it's a decimal layout id
          gSettings.HDALayoutId = AsciiStrDecimalToUintn(prop->string);
        }
      }      
    }
        
		//*** ACPI ***//
    
    dictPointer = GetProperty(dict,"ACPI");
    if (dictPointer) {
      prop = GetProperty(dictPointer,"DropOemSSDT");
      gSettings.DropSSDT = FALSE;
      if(prop)
      {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.DropSSDT = TRUE;
      }
      prop = GetProperty(dictPointer,"GeneratePStates");
      gSettings.GeneratePStates = FALSE;
      if(prop)
      {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.GeneratePStates = TRUE;
      }
      prop = GetProperty(dictPointer,"GenerateCStates");
      gSettings.GenerateCStates = FALSE;
      if(prop)
      {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.GenerateCStates = TRUE;
      }
      
      prop = GetProperty(dictPointer,"ResetAddress");
      if(prop)
      {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.ResetAddr  = StrHexToUint64(UStr); 
      }
      prop = GetProperty(dictPointer,"ResetValue");
      if(prop)
      {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.ResetVal = (UINT8)StrHexToUint64((CHAR16*)&UStr[0]);	
      }
      //other known pair is 0x02F9/0x06
      
      prop = GetProperty(dictPointer,"EnableC6");
      gSettings.EnableC6 = FALSE;
      if(prop)
      {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.EnableC6 = TRUE;
      }
      
      prop = GetProperty(dictPointer,"EnableC4");
      gSettings.EnableC4 = FALSE;
      if(prop)
      {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.EnableC4 = TRUE;
      }
      
      prop = GetProperty(dictPointer,"EnableC2");
      gSettings.EnableC2 = FALSE;
      if(prop)
      {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.EnableC2 = TRUE;
          DBG(" C2 enabled\n");
        }
      }
      
      prop = GetProperty(dictPointer,"EnableISS");
      gSettings.EnableISS = FALSE;
      if(prop)
      {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.EnableISS = TRUE;
      }      
      prop = GetProperty(dictPointer,"smartUPS");
      gSettings.smartUPS = FALSE;
      if(prop)
      {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.smartUPS = TRUE;
          DBG("Config set smartUPS present\n");
        }
      }
    }

    		
		//*** SMBIOS ***//
    dictPointer = GetProperty(dict,"SMBIOS");
    if (dictPointer) {
      prop = GetProperty(dictPointer,"BiosVendor");
      if(prop)
      {
        AsciiStrCpy(gSettings.VendorName, prop->string);
      }
      prop = GetProperty(dictPointer,"BiosVersion");
      if(prop)
      {
        AsciiStrCpy(gSettings.RomVersion, prop->string);
      }
      prop = GetProperty(dictPointer,"BiosReleaseDate");
      if(prop)
      {
        AsciiStrCpy(gSettings.ReleaseDate, prop->string);
      }
      prop = GetProperty(dictPointer,"Manufacturer");
      if(prop)
      {
        AsciiStrCpy(gSettings.ManufactureName, prop->string);
      }
      prop = GetProperty(dictPointer,"ProductName");
      if(prop)
      {
        AsciiStrCpy(gSettings.ProductName, prop->string);
      }
      prop = GetProperty(dictPointer,"Version");
      if(prop)
      {
        AsciiStrCpy(gSettings.VersionNr, prop->string);
      }
      prop = GetProperty(dictPointer,"Family");
      if(prop)
      {
        AsciiStrCpy(gSettings.FamilyName, prop->string);
      }
      prop = GetProperty(dictPointer,"SerialNumber");
      if(prop)
      {
        AsciiStrCpy(gSettings.SerialNr, prop->string);
      }
      
      prop = GetProperty(dictPointer,"BoardManufacturer");
      if(prop)
      {
        AsciiStrCpy(gSettings.BoardManufactureName, prop->string);
      }
      prop = GetProperty(dictPointer,"BoardSerialNumber");
      if(prop)
      {
        AsciiStrCpy(gSettings.BoardSerialNumber, prop->string);
      }
      prop = GetProperty(dictPointer,"Board-ID");
      if(prop)
      {
        AsciiStrCpy(gSettings.BoardNumber, prop->string);
      }
      prop = GetProperty(dictPointer,"BoardVersion");
      if(prop)
      {
        AsciiStrCpy(gSettings.BoardVersion, prop->string);
      }
      
      prop = GetProperty(dictPointer,"Mobile");
      if(prop)
      {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.Mobile = TRUE;
      }
      
      prop = GetProperty(dictPointer,"LocationInChassis");
      if(prop)
      {
        AsciiStrCpy(gSettings.LocationInChassis, prop->string);
      }
      
      prop = GetProperty(dictPointer,"ChassisManufacturer");
      if(prop)
      {
        AsciiStrCpy(gSettings.ChassisManufacturer, prop->string);
      }
      prop = GetProperty(dictPointer,"ChassisAssetTag");
      if(prop)
      {
        AsciiStrCpy(gSettings.ChassisAssetTag, prop->string);
      }
    }

    //CPU
    dictPointer = GetProperty(dict,"CPU");
    if (dictPointer) {
      prop = GetProperty(dictPointer,"Turbo");
      gSettings.Turbo = FALSE;
      if(prop)
      {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.Turbo = TRUE;
          DBG("Config set Turbo\n");
        }
      }
      prop = GetProperty(dictPointer,"CpuFrequencyMHz");
      if(prop)
      {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.CpuFreqMHz = (UINT16)StrDecimalToUintn((CHAR16*)&UStr[0]);
        DBG("Config set CpuFreq=%dMHz\n", gSettings.CpuFreqMHz);
      }
      prop = GetProperty(dictPointer,"ProcessorType");
      gSettings.CpuType = GetAdvancedCpuType();
      if(prop)
      {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.CpuType = (UINT16)StrHexToUint64((CHAR16*)&UStr[0]);
        DBG("Config set CpuType=%x\n", gSettings.CpuType);
      }
      
      prop = GetProperty(dictPointer,"BusSpeedkHz");
      if(prop)
      {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.BusSpeed = (UINT32)StrDecimalToUintn((CHAR16*)&UStr[0]);
        DBG("Config set BusSpeed=%dkHz\n", gSettings.BusSpeed);
      }      
    }
      	    
		SaveSettings();
	}	
//  DBG("config.plist read and return %r\n", Status);
	return Status;
}	

EFI_STATUS GetOSVersion(IN REFIT_VOLUME *Volume)
{
	EFI_STATUS				Status = EFI_NOT_FOUND;
	CHAR8*						plistBuffer = 0;
	UINTN             plistLen;
	TagPtr						dict=NULL;
	TagPtr						prop = NULL;
  CHAR16*     SystemPlist = L"System\\Library\\CoreServices\\SystemVersion.plist";
  CHAR16*     ServerPlist = L"System\\Library\\CoreServices\\ServerVersion.plist";
  CHAR16*     RecoveryPlist = L"\\com.apple.recovery.boot\\SystemVersion.plist";
  
  if (!Volume) {
    return EFI_NOT_FOUND;
  }
  
	/* Mac OS X */ 
	if(FileExists(Volume->RootDir, SystemPlist)) 
	{
		Status = egLoadFile(Volume->RootDir, SystemPlist, (UINT8 **)&plistBuffer, &plistLen);
	}
	/* Mac OS X Server */
	else if(FileExists(Volume->RootDir, ServerPlist))
	{
		Status = egLoadFile(Volume->RootDir, ServerPlist, (UINT8 **)&plistBuffer, &plistLen);
	}
	else if(FileExists(Volume->RootDir, RecoveryPlist))
	{
		Status = egLoadFile(Volume->RootDir, RecoveryPlist, (UINT8 **)&plistBuffer, &plistLen);
	}
	if(!EFI_ERROR(Status))
	{
		if(ParseXML(plistBuffer, &dict) != EFI_SUCCESS)
		{
			FreePool(plistBuffer);
			return EFI_NOT_FOUND;
		}
    
		prop = GetProperty(dict, "ProductVersion");
		if(prop != NULL)
		{
			// Tiger
			if(AsciiStrStr(prop->string, "10.4") != 0){
        Volume->OSType = OSTYPE_TIGER;
        Volume->OSIconName = L"tiger";
        Volume->BootType = BOOTING_BY_EFI;
        Status = EFI_SUCCESS;
      } else
			// Leopard
      if(AsciiStrStr(prop->string, "10.5") != 0){
				Volume->OSType = OSTYPE_LEO;
        Volume->OSIconName = L"leo";
        Volume->BootType = BOOTING_BY_EFI;
        Status = EFI_SUCCESS;
      } else
			// Snow Leopard
			if(AsciiStrStr(prop->string, "10.6") != 0){
				Volume->OSType = OSTYPE_SNOW;
        Volume->OSIconName = L"snow";
        Volume->BootType = BOOTING_BY_EFI;
        Status = EFI_SUCCESS;
      } else
			// Lion
			if(AsciiStrStr(prop->string, "10.7") != 0){
				Volume->OSType = OSTYPE_LION;
        Volume->OSIconName = L"lion";
        Volume->BootType = BOOTING_BY_EFI;
        Status = EFI_SUCCESS;
      } else
      // Mountain Lion
      if(AsciiStrStr(prop->string, "10.8") != 0){
				Volume->OSType = OSTYPE_COUGAR;
        Volume->OSIconName = L"cougar";
        Volume->BootType = BOOTING_BY_EFI;
        Status = EFI_SUCCESS;
      }
      MsgLog("Booting OS %a\n", prop->string);
    } 
	}
	
	return Status;
}


EFI_STATUS GetEdid(VOID)
{
	EFI_STATUS						Status;
//	UINTN i, j;
  UINTN N;
	Status = gBS->LocateProtocol (&gEfiEdidDiscoveredProtocolGuid, NULL, (VOID **)&EdidDiscovered);
	
	if (!EFI_ERROR (Status)) 
	{
		N = EdidDiscovered->SizeOfEdid;
    MsgLog("EdidDiscovered size=%d\n", N);
		if (N == 0) {
			MsgLog("EdidDiscovered size=0\n");
			return EFI_NOT_FOUND;
		}
/*		for (i=0; i<N; i+=16) {
			MsgLog("%02x: ", i);
			for (j=0; j<16; j++) {
				MsgLog("%02x ", EdidDiscovered->Edid[i+j]);
			}
			MsgLog("\n");		   
		} */
	}
  return Status;
}


VOID SetDevices(VOID)
{
//	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  *modeInfo;
  EFI_STATUS						Status;
  EFI_PCI_IO_PROTOCOL		*PciIo;
	PCI_TYPE00            Pci;
	UINTN                 HandleCount;
	UINTN                 ArrayCount;
	UINTN                 HandleIndex;
	UINTN                 ProtocolIndex;
	EFI_HANDLE            *HandleBuffer;
	EFI_GUID              **ProtocolGuidArray;
	pci_dt_t              PCIdevice;
  UINTN         Segment;
	UINTN         Bus;
	UINTN         Device;
	UINTN         Function;
  BOOLEAN       StringDirty = FALSE;
  BOOLEAN       TmpDirty = FALSE;
  UINT16        PmCon;
  
  
  gGraphics.Width  = UGAWidth;
  gGraphics.Height = UGAHeight;
  
  GetEdid();
  /* Read Pci Bus for GFX */
	Status = gBS->LocateHandleBuffer (AllHandles, NULL, NULL, &HandleCount, &HandleBuffer);
	if (!EFI_ERROR(Status)) {
		for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
			Status = gBS->ProtocolsPerHandle (HandleBuffer[HandleIndex], &ProtocolGuidArray, &ArrayCount);
			if (!EFI_ERROR(Status)) {
				for (ProtocolIndex = 0; ProtocolIndex < ArrayCount; ProtocolIndex++) {
					if (CompareGuid( &gEfiPciIoProtocolGuid, ProtocolGuidArray[ProtocolIndex])) {
						Status = gBS->OpenProtocol (HandleBuffer[HandleIndex], &gEfiPciIoProtocolGuid, (VOID **)&PciIo, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
						if (EFI_ERROR(Status)) {
							continue;
						}
            ZeroMem((UINT8*)&Pci, sizeof(PCI_TYPE00)); //paranoia
            Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, sizeof (Pci) / sizeof (UINT32), &Pci);
            if (EFI_ERROR (Status)) {
              continue;
            }
            Status = PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
						PCIdevice.DeviceHandle = HandleBuffer[HandleIndex];
						PCIdevice.dev.addr = PCIADDR(Bus, Device, Function);
						PCIdevice.vendor_id = Pci.Hdr.VendorId;
						PCIdevice.device_id = Pci.Hdr.DeviceId;
						PCIdevice.revision = Pci.Hdr.RevisionID;
						PCIdevice.subclass = Pci.Hdr.ClassCode[0];
						PCIdevice.class_id = *((UINT16*)(Pci.Hdr.ClassCode+1));
						PCIdevice.subsys_id.subsys.vendor_id = Pci.Device.SubsystemVendorID;
						PCIdevice.subsys_id.subsys.device_id = Pci.Device.SubsystemID;
            // GFX
            if (gSettings.GraphicsInjector &&
                (Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY) &&
                (Pci.Hdr.ClassCode[1] == PCI_CLASS_DISPLAY_VGA)) {
							
              gGraphics.DeviceID = Pci.Hdr.DeviceId;
              
              switch (Pci.Hdr.VendorId) {
                case 0x1002:
                  gGraphics.Vendor = Ati;
    //              MsgLog("ATI GFX found\n");
                  //can't do in one step because of C-conventions
                  TmpDirty = setup_ati_devprop(&PCIdevice);
                  StringDirty |=  TmpDirty;
									
                  break;
                case 0x8086:
                  
                  TmpDirty = setup_gma_devprop(&PCIdevice);
                  StringDirty |=  TmpDirty;
    //              MsgLog("Intel GFX device_id =0x%x\n", PCIdevice.device_id);
                  MsgLog("Intel GFX revision  =0x%x\n", PCIdevice.revision);
                  break;
                case 0x10de:
                  gGraphics.Vendor = Nvidia;
    //              MsgLog("nVidia GFX found\n");
                  TmpDirty = setup_nvidia_devprop(&PCIdevice);
                  StringDirty |=  TmpDirty;
                  break;
                default:
                  break;
              }
            }
            
            //LAN
            else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_NETWORK) &&
                     (Pci.Hdr.ClassCode[1] == PCI_CLASS_NETWORK_ETHERNET)) {
   //           MsgLog("Ethernet device found\n");
							TmpDirty = set_eth_props(&PCIdevice);
							StringDirty |=  TmpDirty;
            }
            
            //USB
            else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_SERIAL) &&
                     (Pci.Hdr.ClassCode[1] == PCI_CLASS_SERIAL_USB)) {
 //             MsgLog("USB device found\n");
							TmpDirty = set_usb_props(&PCIdevice);
							StringDirty |=  TmpDirty;
						}
						
						// HDA
						else if (gSettings.HDAInjection &&
                     (Pci.Hdr.ClassCode[2] == PCI_CLASS_MEDIA) &&
                     (Pci.Hdr.ClassCode[1] == PCI_CLASS_MEDIA_HDA)) {
//							MsgLog("HDA device found\n");
							TmpDirty = set_hda_props(PciIo, &PCIdevice);
							StringDirty |=  TmpDirty;
            }
						
            //LPC
            else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_BRIDGE) &&
                     (Pci.Hdr.ClassCode[1] == PCI_CLASS_BRIDGE_ISA))
            {
              if (gSettings.LpcTune) {
                Status = PciIo->Pci.Read (
                                          PciIo, 
                                          EfiPciIoWidthUint16, 
                                          GEN_PMCON_1, 
                                          1, 
                                          &PmCon
                                          );
                MsgLog("Initial PmCon value=%x\n", PmCon);  
                if (gSettings.EnableC6) {
                  PmCon |= 1 << 11;	
                  DBG("C6 enabled\n");
                } else {
                  PmCon &= ~(1 << 11);
                  DBG("C6 disabled\n");
                }
     /*           if (gSettings.EnableC2) {
                  PmCon |= 1 << 10;	
                  DBG("BIOS_PCIE enabled\n");
                } else {
                  PmCon &= ~(1 << 10);
                  DBG("BIOS_PCIE disabled\n");
                }
      */
                if (gSettings.EnableC4) {
                  PmCon |= 1 << 7;	
                  DBG("C4 enabled\n");
                } else {
                  PmCon &= ~(1 << 7);
                  DBG("C4 disabled\n");
                }
                if (gSettings.EnableISS) {
                  PmCon |= 1 << 3;	
                  DBG("SpeedStep enabled\n");
                } else {
                  PmCon &= ~(1 << 3);
                  DBG("SpeedStep disabled\n");
                }
                
                Status = PciIo->Pci.Write (
                                           PciIo, 
                                           EfiPciIoWidthUint16, 
                                           GEN_PMCON_1, 
                                           1, 
                                           &PmCon
                                           );
                
                Status = PciIo->Pci.Read (
                                          PciIo, 
                                          EfiPciIoWidthUint16, 
                                          GEN_PMCON_1, 
                                          1, 
                                          &PmCon
                                          );
                MsgLog("Set PmCon value=%x\n", PmCon);                   
                
              } 
            }                
					}
				}
      }        
    }
  }
	
  if (StringDirty) {
    stringlength = string->length * 2;
    
    gDeviceProperties = AllocateAlignedPages(EFI_SIZE_TO_PAGES(stringlength + 1), 64);
    CopyMem(gDeviceProperties, (VOID*)devprop_generate_string(string), stringlength);
    gDeviceProperties[stringlength] = 0;
//    DBG(gDeviceProperties);
//    DBG("\n");   
    StringDirty = FALSE;
    
	}
  
	MsgLog("CurrentMode: Width=%d Height=%d\n", gGraphics.Width, gGraphics.Height);  
}


#define	MSR_IA32_PERF_STATUS        0x0198
#define MSR_IA32_PERF_CONTROL       0x0199

EFI_STATUS SaveSettings()
{
  UINT64  msr;
  //TODO - SetVariable()..
  //here we can apply user settings instead of defult one
  gMobile = gSettings.Mobile;
  
  if ((gSettings.BusSpeed != 0) &&
      (gSettings.BusSpeed > 10 * kilo) &&
      (gSettings.BusSpeed < 500 * kilo)){
    gCPUStructure.ExternalClock = gSettings.BusSpeed;
    gCPUStructure.FSBFrequency = gSettings.BusSpeed * kilo; //kHz -> Hz
    gCPUStructure.MaxSpeed = DivU64x32(gSettings.BusSpeed, 100) * gCPUStructure.MaxRatio; //kHz->MHz
  }

  if ((gSettings.CpuFreqMHz != 0) &&
      (gSettings.CpuFreqMHz > 100) &&
      (gSettings.CpuFreqMHz < 20000)){
    gCPUStructure.MaxSpeed = gSettings.CpuFreqMHz;
  }
  
  if (gSettings.Turbo){
    if (gCPUStructure.Turbo4) {
      gCPUStructure.CPUFrequency = DivU64x32(gCPUStructure.Turbo4 * gCPUStructure.FSBFrequency, 10);
    }
    
    //attempt to make turbo
    if (gSettings.Turbo){
      msr = AsmReadMsr64(MSR_IA32_MISC_ENABLE);
      DBG("MSR_IA32_MISC_ENABLE = %lx\n", msr);
      msr &= ~(1ULL<<38);
      AsmWriteMsr64(MSR_IA32_MISC_ENABLE, msr);
      gBS->Stall(100);
      msr = AsmReadMsr64(MSR_IA32_MISC_ENABLE);
      DBG("Set turbo: MSR_IA32_MISC_ENABLE = %lx\n", msr);
    }
    if (TurboMsr != 0) {
      AsmWriteMsr64(MSR_IA32_PERF_CONTROL, TurboMsr);
      gBS->Stall(100);
      WaitForSts();
    }
    
    msr = AsmReadMsr64(MSR_IA32_PERF_STATUS);
    DBG("set turbo state msr=%x CPU=%dMHz\n", msr, DivU64x32(gCPUStructure.CPUFrequency, Mega));
  } 
  
  
  MsgLog("Finally: Bus=%dMHz CPU=%dMHz\n",
         DivU64x32(gCPUStructure.FSBFrequency, Mega),
         DivU64x32(gCPUStructure.CPUFrequency, Mega));
  return EFI_SUCCESS;
}

//dmazar
CHAR16* GetExtraKextsDir(REFIT_VOLUME *Volume)
{
  CHAR16                      *OSTypeStr = NULL;
  CHAR16                      *SrcDir = NULL;
  
  // get os version as string
  switch (Volume->OSType) {
    case OSTYPE_TIGER:
      OSTypeStr = L"10.4";
      break;
      
    case OSTYPE_LEO:
      OSTypeStr = L"10.5";
      break;
      
    case OSTYPE_SNOW:
      OSTypeStr = L"10.6";
      break;
      
    case OSTYPE_LION:
      OSTypeStr = L"10.7";
      break;
      
    case OSTYPE_COUGAR:
      OSTypeStr = L"10.8";
      break;
      
    default:
      OSTypeStr = L"Other";
      break;
  }
  MsgLog("OS=%s ", OSTypeStr);
  
  // find source injection folder with kexts
  // note: we are just checking for existance of particular folder, not checking if it is empty or not
  // check OEM subfolders: version speciffic or default to Other
  SrcDir = PoolPrint(L"%s\\kexts\\%s", OEMPath, OSTypeStr);
  if (!FileExists(SelfVolume->RootDir, SrcDir)) {
    FreePool(SrcDir);
    SrcDir = PoolPrint(L"%s\\kexts\\Other", OEMPath);
    if (!FileExists(SelfVolume->RootDir, SrcDir)) {
      FreePool(SrcDir);
      SrcDir = NULL;
    }
  }
  if (SrcDir == NULL) {
    // if not found, check EFI\kexts\...
    SrcDir = PoolPrint(L"\\EFI\\kexts\\%s", OSTypeStr);
    if (!FileExists(SelfVolume->RootDir, SrcDir)) {
      FreePool(SrcDir);
      SrcDir = PoolPrint(L"\\EFI\\kexts\\Other", gSettings.OEMProduct);
      if (!FileExists(SelfVolume->RootDir, SrcDir)) {
        FreePool(SrcDir);
        SrcDir = NULL;
      }
    }
  }
  
  return SrcDir;
}

EFI_STATUS SetFSInjection(IN LOADER_ENTRY *Entry)
{
    EFI_STATUS                  Status;
    REFIT_VOLUME                *Volume;
    FSINJECTION_PROTOCOL        *FSInject;
    CHAR16                      *SrcDir = NULL;
    CHAR16                      *Blacklist[16]; // reserve place for up to 16 file names
    UINTN                       BlacklistCnt = 0;
    BOOLEAN                     InjectionNeeded = FALSE;
    BOOLEAN                     BlockCaches = FALSE;
    
    MsgLog("FSInjection: ");
    
    Volume = Entry->Volume;
    
    // some checks?
    if (Volume->BootType != BOOTING_BY_EFI) {
        MsgLog("not started - not an EFI boot\n");
        return EFI_UNSUPPORTED;
    }
    
    if (Entry->LoadOptions == NULL || StrStr(Entry->LoadOptions, L"WithKexts") == NULL) {
        // FS injection not requested
        MsgLog("not requested\n");
        return EFI_NOT_STARTED;
    }
    
    // get FSINJECTION_PROTOCOL
    Status = gBS->LocateProtocol(&gFSInjectProtocolGuid, NULL, (void **)&FSInject);
    if (EFI_ERROR(Status)) {
        //Print(L"- No FSINJECTION_PROTOCOL, Status = %r\n", Status);
        MsgLog("not started - gFSInjectProtocolGuid not found\n");
        return EFI_NOT_STARTED;
    }
    
    if (StrStr(Entry->LoadOptions, L"WithKexts") != NULL) {
        
        SrcDir = GetExtraKextsDir(Volume);
        if (SrcDir != NULL) {
            // we have found it - injection will be done
    MsgLog("Injecting kexts from: '%s' ", SrcDir);
            BlockCaches = TRUE;
            InjectionNeeded = TRUE;
            
        } else {
            MsgLog("Skipping kext injection (kexts folder not found) ");
        }
    }
    
    if (!InjectionNeeded) {
        MsgLog("- not done!\n");
        return EFI_NOT_STARTED;
    }
    
    if (BlockCaches) {
        // add caches to blacklist
        Blacklist[BlacklistCnt++] = L"\\System\\Library\\Caches\\com.apple.kext.caches\\Startup\\kernelcache";
        Blacklist[BlacklistCnt++] = L"\\System\\Library\\Caches\\com.apple.kext.caches\\Startup\\Extensions.mkext";
        Blacklist[BlacklistCnt++] = L"\\System\\Library\\Extensions.mkext";
        Blacklist[BlacklistCnt++] = L"\\com.apple.recovery.boot\\kernelcache";
        Blacklist[BlacklistCnt++] = L"\\com.apple.recovery.boot\\Extensions.mkext";
        
    }
    
    //Print(L"FSInject->Install(%X, %s, %X, %s, %d, %p) ...\n", Volume->DeviceHandle, L"\\system\\library\\extensions", SelfVolume->DeviceHandle, SrcDir, BlacklistCnt, Blacklist);
    Status = FSInject->Install(Volume->DeviceHandle, L"\\System\\Library\\Extensions",
                               SelfVolume->DeviceHandle, SrcDir,
                               BlacklistCnt, Blacklist);
    
    if (SrcDir != NULL) FreePool(SrcDir);
    
    if (EFI_ERROR(Status)) {
        MsgLog("not done - could not install injection!\n");
        return EFI_NOT_STARTED;
    }
    
    // reinit Volume->RootDir? it seems it's not needed.
    
    MsgLog("done!\n");
	return Status;
}
