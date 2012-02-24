/*
 Slice 2012
*/

#include "device_inject.h"
#include "Platform.h"

#define DEBUG_SET 0

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
UINT16                          gResetAddress;
UINT16                          gResetValue;

//should be excluded. Now refit.conf
/*EFI_STATUS GetTheme (CHAR16* ThemePlistPath)
{
  EFI_STATUS	Status = EFI_NOT_FOUND;
	UINT32		size;
	CHAR8*		gThemePtr;
	TagPtr		dict;
	TagPtr		prop;

  if (FileExists(SelfRootDir, ThemePlistPath)) {
    Status = egLoadFile(SelfRootDir, ThemePlistPath, (UINT8**)&gThemePtr, &size);
  } 
  if (EFI_ERROR(Status)) {
    Print(L"No theme found!\n");
    gBS->Stall(3000000);
    return Status;
  }

  if(gThemePtr)
	{		
		if(ParseXML((const CHAR8*)gThemePtr, &dict) != EFI_SUCCESS)
		{
			Print(L"Theme.plist parsing error!\n");
      gBS->Stall(3000000);
			return EFI_UNSUPPORTED;
		}
  }
  return Status;
}*/

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
		DBG("Error loading nvram.plist!\n");
		return Status;
	}
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
						//	AsciiStrToUnicodeStr(prop->string, gSelectedUUID);
              AsciiStrCpy(gSelectedUUID, prop->string);
						}									
					}
				}				
			}
		}		
		SaveSettings();
	}	
	return Status;
}	


EFI_STATUS GetUserSettings(IN EFI_FILE *RootDir)
{
	EFI_STATUS	Status = EFI_NOT_FOUND;
	UINTN		size;
	CHAR8*		gConfigPtr;
	TagPtr		dict;
	TagPtr		prop;
  CHAR16    UStr[64];
//	TagPtr		dictPointer;
  CHAR16* ConfigPlistPath = L"EFI\\config.plist";
	
	
	// load config
  if ((RootDir != NULL) && FileExists(RootDir, ConfigPlistPath)) {
    Status = egLoadFile(RootDir, ConfigPlistPath, (UINT8**)&gConfigPtr, &size);
  } 
  if (EFI_ERROR(Status)) {
    Status = egLoadFile(SelfRootDir, ConfigPlistPath, (UINT8**)&gConfigPtr, &size);
  }
    
	if(EFI_ERROR(Status)) {
		Print(L"Error loading config.plist!\n");
		return Status;
	}
	if(gConfigPtr)
	{		
		if(ParseXML((const CHAR8*)gConfigPtr, &dict) != EFI_SUCCESS)
		{
			Print(L"config error\n");
			return EFI_UNSUPPORTED;
		}
		//*** SYSTEM ***//
    prop = GetProperty(dict,"prev-lang:kbd");
    if(prop)
    {
      AsciiStrToUnicodeStr(prop->string, gSettings.Language);
    }
    
		prop = GetProperty(dict,"boot-args");
		if(prop)
		{
			AsciiStrCpy(gSettings.BootArgs, prop->string);
		} 
    
 		prop = GetProperty(dict,"DefaultBootVolume");
		if(prop)
		{
      AsciiStrToUnicodeStr(prop->string, gSettings.DefaultBoot);
		}
//Graphics
 		prop = GetProperty(dict,"LoadVBios");
		if(prop)
		{
//			AsciiStrCpy(gSettings.LoadVBios, prop->string);
      if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
				gSettings.smartUPS=TRUE;
			else
				gSettings.smartUPS=FALSE;
      
		}
 		prop = GetProperty(dict,"VideoPorts");
		if(prop)
		{
      AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
			gSettings.VideoPorts = (UINT16)StrDecimalToUintn((CHAR16*)&UStr[0]);

		}
 		prop = GetProperty(dict,"FBName");
		if(prop)
		{
      AsciiStrToUnicodeStr(prop->string, gSettings.FBName);
		}
    
    
		//gSettings.TimeOut - will be in refit.config
/*		prop = GetProperty(dict,"TimeOut");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.TimeOut);
			gTimeoutSec = (UINT16)StrDecimalToUintn(gSettings.TimeOut);	
		} else {
			gTimeoutSec = 5;
			StrCpy(gSettings.TimeOut, L"5");
		}
*/		

		//*** ACPI ***//
		prop = GetProperty(dict,"ResetAddress");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
			gResetAddress  = (UINT16)StrHexToUint64(UStr); 
		}  else {
			gResetAddress  = 0x64; //I wish it will be default
		}
		prop = GetProperty(dict,"ResetValue");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
			gSettings.ResetVal = (UINT16)StrHexToUint64((CHAR16*)&UStr[0]);	
		} else {
			gSettings.ResetVal = 0xFE;
		}
		//other known pair is 0x02F9/0x06
		
		//*** SMBIOS ***//
		prop = GetProperty(dict,"BiosVendor");
		if(prop)
		{
			AsciiStrCpy(gSettings.VendorName, prop->string);
		}
		prop = GetProperty(dict,"BiosVersion");
		if(prop)
		{
			AsciiStrCpy(gSettings.RomVersion, prop->string);
		}
		prop = GetProperty(dict,"BiosReleaseDate");
		if(prop)
		{
			AsciiStrCpy(gSettings.ReleaseDate, prop->string);
		}
		prop = GetProperty(dict,"Manufacturer");
		if(prop)
		{
			AsciiStrCpy(gSettings.ManufactureName, prop->string);
		}
		prop = GetProperty(dict,"ProductName");
		if(prop)
		{
			AsciiStrCpy(gSettings.ProductName, prop->string);
		}
		prop = GetProperty(dict,"Version");
		if(prop)
		{
			AsciiStrCpy(gSettings.VersionNr, prop->string);
		}
		prop = GetProperty(dict,"Family");
		if(prop)
		{
			AsciiStrCpy(gSettings.FamilyName, prop->string);
		}
		prop = GetProperty(dict,"SerialNumber");
		if(prop)
		{
			AsciiStrCpy(gSettings.SerialNr, prop->string);
		}
		prop = GetProperty(dict,"CustomUUID");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.CustomUuid);
      Status = StrToGuid(gSettings.CustomUuid, &gUuid);
      if (EFI_ERROR(Status)) {
        CopyMem((VOID*)&gUuid, (VOID*)&gRandomUUID, 16);
      }
		}
		prop = GetProperty(dict,"BoardManufacturer");
		if(prop)
		{
			AsciiStrCpy(gSettings.BoardManufactureName, prop->string);
		}
		prop = GetProperty(dict,"BoardSerialNumber");
		if(prop)
		{
			AsciiStrCpy(gSettings.BoardSerialNumber, prop->string);
		}
		prop = GetProperty(dict,"Board-ID");
		if(prop)
		{
			AsciiStrCpy(gSettings.BoardNumber, prop->string);
		}
		prop = GetProperty(dict,"LocationInChassis");
		if(prop)
		{
			AsciiStrCpy(gSettings.LocationInChassis, prop->string);
		}
		
		prop = GetProperty(dict,"ChassisManufacturer");
		if(prop)
		{
			AsciiStrCpy(gSettings.ChassisManufacturer, prop->string);
		}
		prop = GetProperty(dict,"ChassisAssetTag");
		if(prop)
		{
			AsciiStrCpy(gSettings.ChassisAssetTag, prop->string);
		}
		prop = GetProperty(dict,"smartUPS");
		if(prop)
		{
			if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
				gSettings.smartUPS=TRUE;
			else
				gSettings.smartUPS=FALSE;
		}
/*		prop = GetProperty(dict,"ShowLegacyBoot");
		if(prop)
		{
			if ((prop->string[0] == "y") || (prop->string[0] == "Y"))
				gSettings.ShowLegacyBoot=TRUE;
			else
				gSettings.ShowLegacyBoot=FALSE;
		}
    
		
		prop = GetProperty(dict,"MemorySerialNumber");
		if(prop)
		{
			AsciiStrCpy(gSettings.MemorySerialNumber);
		}
		prop = GetProperty(dict,"MemoryPartNumber");
		if(prop)
		{
			AsciiStrCpy(gSettings.MemoryPartNumber);
		}
 */
		prop = GetProperty(dict,"CpuFrequencyMHz");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
			gSettings.CpuFreqMHz = (UINT16)StrDecimalToUintn((CHAR16*)&UStr[0]);
		}
		prop = GetProperty(dict,"ProcessorType");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
			gSettings.CpuType = (UINT16)StrHexToUint64((CHAR16*)&UStr[0]);
		} else {
			gSettings.CpuType = GetAdvancedCpuType();
		}

		prop = GetProperty(dict,"BusSpeedkHz");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
			gSettings.BusSpeed = (UINT16)StrDecimalToUintn((CHAR16*)&UStr[0]);
		}
		SaveSettings();
	}	
  Print(L"config.plist read and return %r\n", Status);
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
	if(!EFI_ERROR(Status))
	{
		if(ParseXML(plistBuffer, &dict) != EFI_SUCCESS)
		{
//			Print(L"Error Parsing System Version!\n");
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
      
    } 
	}
//	MsgLog("Booting %a\n", sysVersion[Volume->OSType]);
	return Status;
}


EFI_STATUS GetEdid(VOID)
{
	EFI_STATUS						Status;
	UINTN i, j, N;
/*	EFI_EDID_ACTIVE_PROTOCOL*                EdidActive;

  //	EFI_GRAPHICS_OUTPUT_PROTOCOL	*GraphicsOutput=NULL;
	Status = gBS->LocateProtocol (&gMsgLogProtocolGuid, NULL, (VOID **)&EdidActive);
	
	if (!EFI_ERROR (Status)) 
	{
		N = EdidActive->SizeOfEdid;
		MsgLog("Log from Clover:\n");
		MsgLog("%a\n", (CHAR8*)EdidActive->Edid);
	}
*/  
	Status = gBS->LocateProtocol (&gEfiEdidDiscoveredProtocolGuid, NULL, (VOID **)&EdidDiscovered);
	
	if (!EFI_ERROR (Status)) 
	{
		N = EdidDiscovered->SizeOfEdid;
		if (N == 0) {
			MsgLog("EdidDiscovered size=0\n");
			return EFI_NOT_FOUND;
		}
		for (i=0; i<N; i+=16) {
			MsgLog("%02x: ", i);
			for (j=0; j<16; j++) {
				MsgLog("%02x ", EdidDiscovered->Edid[i+j]);
			}
			MsgLog("\n");		   
		}
	}
  return Status;
}


VOID SetGraphics(VOID)
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
  pci_dt_t*             GFXdevice;
  
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
						if (!EFI_ERROR(Status)) {
							Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, sizeof (Pci) / sizeof (UINT32), &Pci);
							if (EFI_ERROR (Status)) {
								continue;
							}
							// Ati GFX
							if (Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY) {
								if (Pci.Hdr.VendorId == 0x1002) {
									gGraphics.Vendor = Ati;
                  MsgLog("ATI GFX found\n");
									GFXdevice = AllocatePool(sizeof(pci_dt_t));
									GFXdevice->vendor_id = Pci.Hdr.VendorId;
									GFXdevice->device_id = Pci.Hdr.DeviceId;
                  gGraphics.DeviceID = Pci.Hdr.DeviceId;
									//GFXdevice->subsys_id = (UINT16)(0x10de0000 | Pci.Hdr.DeviceId);
									GFXdevice->revision = Pci.Hdr.RevisionID;
									GFXdevice->subclass = Pci.Hdr.ClassCode[0];
									GFXdevice->class_id = *((UINT16*)(Pci.Hdr.ClassCode+1));
									//setup_ati_devprop(GFXdevice);
									FreePool(GFXdevice);
								}
							}
							// Intel GFX
							if (Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY) {
								if (Pci.Hdr.VendorId == 0x8086) {
									gGraphics.Vendor = Intel;
                  MsgLog("Intel GFX found\n");
									GFXdevice = AllocatePool(sizeof(pci_dt_t));
									GFXdevice->vendor_id = Pci.Hdr.VendorId;
									GFXdevice->device_id = Pci.Hdr.DeviceId;
                  gGraphics.DeviceID = Pci.Hdr.DeviceId;
									//GFXdevice->subsys_id = (UINT16)(0x10de0000 | Pci.Hdr.DeviceId);
									GFXdevice->revision = Pci.Hdr.RevisionID;
									GFXdevice->subclass = Pci.Hdr.ClassCode[0];
									GFXdevice->class_id = *((UINT16*)(Pci.Hdr.ClassCode+1));
									//setup_gma_devprop(GFXdevice);
                  MsgLog("Intel GFX device_id =0x%x\n", GFXdevice->device_id);
                  MsgLog("Intel GFX revision  =0x%x\n", GFXdevice->revision);
                 // MsgLog("Intel GFX found\n");
									FreePool(GFXdevice);
								}
							}
							// Nvidia GFX
							if (Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY) {
								if (Pci.Hdr.VendorId == 0x10de) {
									gGraphics.Vendor = Nvidia;
                   MsgLog("nVidia GFX found\n");
									GFXdevice = AllocatePool(sizeof(pci_dt_t));
									GFXdevice->vendor_id = Pci.Hdr.VendorId;
									GFXdevice->device_id = Pci.Hdr.DeviceId;
                  gGraphics.DeviceID = Pci.Hdr.DeviceId;
									//GFXdevice->subsys_id = (UINT16)(0x10de0000 | Pci.Hdr.DeviceId);
									GFXdevice->revision = Pci.Hdr.RevisionID;
									GFXdevice->subclass = Pci.Hdr.ClassCode[0];
									GFXdevice->class_id = *((UINT16*)(Pci.Hdr.ClassCode+1));
									//setup_nvidia_devprop(GFXdevice);
									FreePool(GFXdevice);
								}
							}
						}
					}
				}
			}
		}
	}
  
	DBG("CurrentMode: Width=%d Height=%d\n", gGraphics.Width, gGraphics.Height);  
}

EFI_STATUS SaveSettings()
{
  //TODO - SetVariable()..
  return EFI_SUCCESS;
}