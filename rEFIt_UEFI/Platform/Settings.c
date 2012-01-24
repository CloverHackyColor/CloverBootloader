/*
 Slice 2012
*/

#include "Platform.h"

#define DEBUG_SET 0

#if DEBUG_SET == 2
#define DBG(x...) AsciiPrint(x)
#elif DEBUG_SET == 1
#define DBG(x...) MsgLog(x)
#else
#define DBG(x...)
#endif

CHAR8             gSelectedUUID[40];
SETTINGS_DATA     gSettings;
GFX_MANUFACTERER  gGraphicsCard;
EFI_EDID_DISCOVERED_PROTOCOL*            EdidDiscovered;

EFI_STATUS GetNVRAMSettings(IN REFIT_VOLUME *Volume, CHAR16* NVRAMPlistPath)
{
	EFI_STATUS	Status;
	UINT32		size;
	CHAR8*		gNvramPtr;
	CHAR8*		efiBootDevice;
	TagPtr		dict;
	TagPtr		prop;
	TagPtr		dictPointer;
	UINT32		pos = 0;
	
	Status = egLoadFile(Volume->RootDir, NVRAMPlistPath, (CHAR8**)&gNvramPtr, &size);
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
			AsciiStrToUnicodeStr(prop->string, gSettings.KernelFlags);
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
				Status = XMLParseNextTag(efiBootDevice + pos, &dict, &size);
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
							AsciiStrToUnicodeStr(prop->string, gSelectedUUID);
						}									
					}
				}				
			}
		}		
		SaveSettings();
	}	
	return Status;
}	

EFI_STATUS GetUserSettings(IN REFIT_VOLUME *Volume, CHAR16* ConfigPlistPath)
{
	EFI_STATUS	Status = EFI_NOT_FOUND;
	UINT32		size;
	CHAR8*		gConfigPtr;
	TagPtr		dict;
	TagPtr		prop;
//	TagPtr		dictPointer;
	
	
	// load config
  if (FileExists(Volume->RootDir, ConfigPlistPath)) {
    Status = egLoadFile(Volume->RootDir, ConfigPlistPath, (CHAR8**)&gConfigPtr, &size);
  } 
  if (EFI_ERROR(Status)) {
    Status = egLoadFile(SelfRootDir, ConfigPlistPath, (CHAR8**)&gConfigPtr, &size);
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
		prop = GetProperty(dict,"Language");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.Language);
		} else {
			prop = GetProperty(dict,"prev-lang:kbd");
			if(prop)
			{
				AsciiStrToUnicodeStr(prop->string, gSettings.Language);
			}
		}
		prop = GetProperty(dict,"boot-args");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.KernelFlags);
		} else {
			prop = GetProperty(dict,"KernelFlags");
			if(prop)
			{
				AsciiStrToUnicodeStr(prop->string, gSettings.KernelFlags);
			}
		}
		//gSettings.TimeOut
		prop = GetProperty(dict,"TimeOut");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.TimeOut);
			gTimeoutSec = (UINT16)StrDecimalToUintn(gSettings.TimeOut);	
		} else {
			gTimeoutSec = 5;
			StrCpy(gSettings.TimeOut, L"5");
		}
		

		//*** ACPI ***//
		prop = GetProperty(dict,"ResetAddress");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.ResetAddr);
			gResetAddress  = (UINT16)StrHexToUint64(gSettings.ResetAddr); 
		}  else {
			gResetAddress  = 0x64; //I wish it will be default
			StrCpy(gSettings.ResetAddr, L"0x64");
		}
		prop = GetProperty(dict,"ResetValue");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.ResetVal);
			gResetValue = (UINT16)StrHexToUint64(gSettings.ResetVal);	
		} else {
			gResetValue = 0xFE;
			StrCpy(gSettings.ResetVal, L"0xFE");
		}
		//other known pair is 0x02F9/0x06
		
		//*** SMBIOS ***//
		prop = GetProperty(dict,"BiosVendor");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.VendorName);
		}
		prop = GetProperty(dict,"BiosVersion");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.RomVersion);
		}
		prop = GetProperty(dict,"BiosReleaseDate");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.ReleaseDate);
		}
		prop = GetProperty(dict,"Manufacturer");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.ManufactureName);
		}
		prop = GetProperty(dict,"ProductName");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.ProductName);
		}
		prop = GetProperty(dict,"Version");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.VersionNr);
		}
		prop = GetProperty(dict,"Family");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.FamilyName);
		}
		prop = GetProperty(dict,"SerialNumber");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.SerialNr);
		}
		prop = GetProperty(dict,"CustomUUID");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.CustomUuid);
      Status = StrToGuid(gSettings.CustomUuid, gUuid);
      if (EFI_ERROR(Status)) {
        CopyMem(gUuid, gRandomUUID, 16);
      }
		}
		prop = GetProperty(dict,"BoardManufacturer");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.BoardManufactureName);
		}
		prop = GetProperty(dict,"BoardSerialNumber");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.BoardSerialNumber);
		}
		prop = GetProperty(dict,"Board-ID");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.BoardNumber);
		}
		prop = GetProperty(dict,"LocationInChassis");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.LocationInChassis);
		}
		
		prop = GetProperty(dict,"ChassisManufacturer");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.ChassisManufacturer);
		}
		prop = GetProperty(dict,"ChassisAssetTag");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.ChassisAssetTag);
		}
		prop = GetProperty(dict,"smartUPS");
		if(prop)
		{
			if (prop->string[0] == "y") || (prop->string[0] == "Y")
				gSettings.smartUPS=TRUE;
			else
				gSettings.smartUPS=FALSE;
		}
		prop = GetProperty(dict,"ShowLegacyBoot");
		if(prop)
		{
			if (prop->string[0] == "y") || (prop->string[0] == "Y")
				gSettings.ShowLegacyBoot=TRUE;
			else
				gSettings.ShowLegacyBoot=FALSE;
		}
    
/*		
		prop = GetProperty(dict,"MemorySerialNumber");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.MemorySerialNumber);
		}
		prop = GetProperty(dict,"MemoryPartNumber");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.MemoryPartNumber);
		}
 */
		prop = GetProperty(dict,"CpuFrequencyMHz");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.CpuFreqMHz);
			gCpuSpeed = (UINT16)StrDecimalToUintn(gSettings.CpuFreqMHz);
		}
		prop = GetProperty(dict,"ProcessorType");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.CpuType);
			gCPUtype = (UINT16)StrHexToUint64(gSettings.CpuType);
		} else {
			gCPUtype = GetAdvancedCpuType();
		}

		prop = GetProperty(dict,"BusSpeedkHz");
		if(prop)
		{
			AsciiStrToUnicodeStr(prop->string, gSettings.BusSpeed);
			gBusSpeed = (UINT16)StrDecimalToUintn(gSettings.BusSpeed);
		}
		SaveSettings();
	}	
	return Status;
}	

EFI_STATUS FindVersionPlist(IN REFIT_VOLUME *Volume)
{
	EFI_STATUS				Status = EFI_NOT_FOUND;
	CHAR8*						plistBuffer = 0;
	UINT32						plistLen;
	TagPtr						dict=NULL;
	TagPtr						prop = NULL;
  CONST CHAR16*     SystemPlist = L"System\\Library\\CoreServices\\SystemVersion.plist";
  CONST CHAR16*     ServerPlist = L"System\\Library\\CoreServices\\ServerVersion.plist";
  
	/* Mac OS X */ 
	if(FileExists(Volume->RootDir, SystemPlist)) 
	{
		Status = egLoadFile(Volume->RootDir, SystemPlist, &plistBuffer, &plistLen);
	}
	/* Mac OS X Server */
	else if(FileExists(Volume->RootDir, ServerPlist))
	{
		Status = egLoadFile(Volume->RootDir, ServerPlist, &plistBuffer, &plistLen);
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
        Status = EFI_SUCCESS;
      } else
			// Leopard
      if(AsciiStrStr(prop->string, "10.5") != 0){
				Volume->OSType = OSTYPE_LEO;
        Status = EFI_SUCCESS;
      } else
			// Snow Leopard
			if(AsciiStrStr(prop->string, "10.6") != 0){
				Volume->OSType = OSTYPE_SNOW;
        Status = EFI_SUCCESS;
      } else
			// Lion
			if(AsciiStrStr(prop->string, "10.7") != 0){
				Volume->OSType = OSTYPE_LION;
        Status = EFI_SUCCESS;
      }
    } 
	}
//	MsgLog("Booting OS %x\n", sysVersion);
	return Status;
}

VOID GetEdid(VOID)
{
	UINTN i, j, N;
	EFI_EDID_ACTIVE_PROTOCOL*                EdidActive;
	EFI_STATUS						Status;
  //	EFI_GRAPHICS_OUTPUT_PROTOCOL	*GraphicsOutput=NULL;
	Status = gBootServices->LocateProtocol (&gEfiEdidActiveProtocolGuid, NULL, (VOID **)&EdidActive);
	
	if (!EFI_ERROR (Status)) 
	{
		N = EdidActive->SizeOfEdid;
		MsgLog("Log from Clover:\n");
		MsgLog("%a\n", (CHAR8*)EdidActive->Edid);
	}
  
	Status = gBootServices->LocateProtocol (&gEfiEdidDiscoveredProtocolGuid, NULL, (VOID **)&EdidDiscovered);
	
	if (!EFI_ERROR (Status)) 
	{
		N = EdidDiscovered->SizeOfEdid;
		if (N == 0) {
			MsgLog("EdidDiscovered size=0\n");
			return;
		}
		for (i=0; i<N; i+=16) {
			MsgLog("%02x: ", i);
			for (j=0; j<16; j++) {
				MsgLog("%02x ", EdidDiscovered->Edid[i+j]);
			}
			MsgLog("\n");		   
		}
	}
}


VOID SetGraphics(VOID)
{
  UINTN MaxMode;
	UINTN modeNumber, currentRes, currentMode;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  *modeInfo;
  EFI_PCI_IO_PROTOCOL		*PciIo;
	PCI_TYPE00            Pci;
	UINTN                 HandleCount;
	UINTN                 ArrayCount;
	UINTN                 HandleIndex;
	UINTN                 ProtocolIndex;
	EFI_HANDLE            *HandleBuffer;
	EFI_GUID              **ProtocolGuidArray;
  pci_dt_t*             GFXdevice;
  
  GetEdid();
  /* Read Pci Bus for GFX */
	Status = gBootServices->LocateHandleBuffer (AllHandles, NULL, NULL, &HandleCount, &HandleBuffer);
	if (!EFI_ERROR(Status)) {
		for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
			Status = gBootServices->ProtocolsPerHandle (HandleBuffer[HandleIndex], &ProtocolGuidArray, &ArrayCount);
			if (!EFI_ERROR(Status)) {
				for (ProtocolIndex = 0; ProtocolIndex < ArrayCount; ProtocolIndex++) {
					if (CompareGuid( &gEfiPciIoProtocolGuid, ProtocolGuidArray[ProtocolIndex])) {
						Status = gBootServices->OpenProtocol (HandleBuffer[HandleIndex], &gEfiPciIoProtocolGuid, (VOID **)&PciIo, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
						if (!EFI_ERROR(Status)) {
							Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, sizeof (Pci) / sizeof (UINT32), &Pci);
							if (EFI_ERROR (Status)) {
								continue;
							}
							// Ati GFX
							if (Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY) {
								if (Pci.Hdr.VendorId == 0x1002) {
									gGraphicsCard.Ati = TRUE;
                  MsgLog("ATI GFX found\n");
									GFXdevice = AllocatePool(sizeof(pci_dt_t));
									GFXdevice->vendor_id = Pci.Hdr.VendorId;
									GFXdevice->device_id = Pci.Hdr.DeviceId;
									//GFXdevice->subsys_id = (UINT16)(0x10de0000 | Pci.Hdr.DeviceId);
									GFXdevice->revision = Pci.Hdr.RevisionID;
									GFXdevice->subclass = Pci.Hdr.ClassCode[0];
									GFXdevice->class_id = *((UINT16*)(Pci.Hdr.ClassCode+1));
									setup_ati_devprop(GFXdevice);
									FreePool(GFXdevice);
								}
							}
							// Intel GFX
							if (Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY) {
								if (Pci.Hdr.VendorId == 0x8086) {
									gGraphicsCard.Intel = TRUE;
                  MsgLog("Intel GFX found\n");
									GFXdevice = AllocatePool(sizeof(pci_dt_t));
									GFXdevice->vendor_id = Pci.Hdr.VendorId;
									GFXdevice->device_id = Pci.Hdr.DeviceId;
									//GFXdevice->subsys_id = (UINT16)(0x10de0000 | Pci.Hdr.DeviceId);
									GFXdevice->revision = Pci.Hdr.RevisionID;
									GFXdevice->subclass = Pci.Hdr.ClassCode[0];
									GFXdevice->class_id = *((UINT16*)(Pci.Hdr.ClassCode+1));
									setup_gma_devprop(GFXdevice);
									FreePool(GFXdevice);
								}
							}
							// Nvidia GFX
							if (Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY) {
								if (Pci.Hdr.VendorId == 0x10de) {
									gGraphicsCard.Nvidia = TRUE;
                   MsgLog("nVidia GFX found\n");
									GFXdevice = AllocatePool(sizeof(pci_dt_t));
									GFXdevice->vendor_id = Pci.Hdr.VendorId;
									GFXdevice->device_id = Pci.Hdr.DeviceId;
									//GFXdevice->subsys_id = (UINT16)(0x10de0000 | Pci.Hdr.DeviceId);
									GFXdevice->revision = Pci.Hdr.RevisionID;
									GFXdevice->subclass = Pci.Hdr.ClassCode[0];
									GFXdevice->class_id = *((UINT16*)(Pci.Hdr.ClassCode+1));
									setup_nvidia_devprop(GFXdevice);
									FreePool(GFXdevice);
								}
							}
						}
					}
				}
			}
		}
	}
  
	MaxMode = GetMaxMode();
	currentRes = 0;
  currentMode = GetMode();
	
	MsgLog("CurrentMode=%d MaxMode=%d\n", currentMode, MaxMode);
	for (modeNumber = 0; modeNumber < MaxMode; modeNumber++) {
		GetModeInfo(modeNumber, &modeInfo);
		MsgLog("mode %d: %dx%d pixels %d\n", modeNumber, modeInfo->HorizontalResolution, modeInfo->VerticalResolution,
           modeInfo->PixelsPerScanLine); 
		if (currentRes < modeInfo->PixelsPerScanLine) {
			currentRes = modeInfo->PixelsPerScanLine;
			gMaxResMode = modeNumber;
		}
	}
	MsgLog("ChosenMode=%d\n", gMaxResMode);
	SetResolution(gMaxResMode);
  
}