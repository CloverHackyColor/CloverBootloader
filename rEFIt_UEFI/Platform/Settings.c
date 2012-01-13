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

CHAR8							gSelectedUUID[40];

EFI_STATUS GetNVRAMSettings(EFI_DEVICE_PATH_PROTOCOL* DevicePath, CHAR16* NVRAMPlistPath)
{
	EFI_STATUS	Status;
	UINT32		size;
	CHAR8*		gNvramPtr;
	CHAR8*		efiBootDevice;
	TagPtr		dict;
	TagPtr		prop;
	TagPtr		dictPointer;
	UINT32		pos=0;
	
	Status = LoadFile(DevicePath, NVRAMPlistPath, (CHAR8**)&gNvramPtr, &size, FALSE);
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
/*		prop=GetProperty(dict, "boot-args");
		if(prop)
		{
			AsciiStrCpy(gSettings.KernelFlags, prop->string);
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

EFI_STATUS GetUserSettings(EFI_HANDLE DeviceHandle, CHAR16* ConfigPlistPath)
{
	EFI_STATUS	Status;
	UINT32		size;
	CHAR8*		gConfigPtr;
	TagPtr		dict;
	TagPtr		prop;
//	TagPtr		dictPointer;
	
	
	// load config
	Status=LoadFileFromDevice(DeviceHandle, ConfigPlistPath, (CHAR8**)&gConfigPtr, &size, FALSE);
	if(EFI_ERROR(Status))
	{
		AsciiPrint("Error loading config.plist!\n");
		return Status;
	}
	if(gConfigPtr!=NULL)
	{		
		if(ParseXML((const CHAR8*)gConfigPtr, &dict) != EFI_SUCCESS)
		{
			AsciiPrint("config error\n");
			return EFI_UNSUPPORTED;
		}
		//*** SYSTEM ***//
		prop=GetProperty(dict,"Language");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.Language, prop->string);
		} else {
			prop=GetProperty(dict,"prev-lang:kbd");
			if(prop!=NULL)
			{
				AsciiStrCpy(gSettings.Language, prop->string);
			}
		}
		prop=GetProperty(dict,"boot-args");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.KernelFlags, prop->string);
		} else {
			prop=GetProperty(dict,"KernelFlags");
			if(prop!=NULL)
			{
				AsciiStrCpy(gSettings.KernelFlags, prop->string);
			}
		}
		//gSettings.TimeOut
		prop=GetProperty(dict,"TimeOut");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.TimeOut, prop->string);
			gTimeoutSec = (UINT16)AsciiStrDecimalToUintn(gSettings.TimeOut);	
		} else {
			gTimeoutSec = 5;
			AsciiStrCpy(gSettings.TimeOut, "5");
		}
		

		//*** ACPI ***//
		prop=GetProperty(dict,"ResetAddress");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.ResetAddr, prop->string);
			gResetAddress  = (UINT16)AsciiStrHexToUintn(gSettings.ResetAddr); 
		}  else {
			gResetAddress  = 0x64; //I wish it will be default
			AsciiStrCpy(gSettings.ResetAddr,"0x64");
		}
		prop=GetProperty(dict,"ResetValue");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.ResetVal, prop->string);
			gResetValue = (UINT16)AsciiStrHexToUintn(gSettings.ResetVal);	
		} else {
			gResetValue = 0xFE;
			AsciiStrCpy(gSettings.ResetVal, "0xFE");
		}
		//other known pair is 0x02F9/0x06
		
		//*** SMBIOS ***//
		prop=GetProperty(dict,"BiosVendor");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.VendorName, prop->string);
		}
		prop=GetProperty(dict,"BiosVersion");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.RomVersion, prop->string);
		}
		prop=GetProperty(dict,"BiosReleaseDate");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.ReleaseDate, prop->string);
		}
		prop=GetProperty(dict,"Manufacturer");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.ManufactureName, prop->string);
		}
		prop=GetProperty(dict,"ProductName");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.ProductName, prop->string);
		}
		prop=GetProperty(dict,"Version");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.VersionNr, prop->string);
		}
		prop=GetProperty(dict,"Family");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.FamilyName, prop->string);
		}
		prop=GetProperty(dict,"SerialNumber");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.SerialNr, prop->string);
		}
		prop=GetProperty(dict,"CustomUUID");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.CustomUuid, prop->string);
		}
		prop=GetProperty(dict,"BoardManufacturer");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.BoardManufactureName, prop->string);
		}
		prop=GetProperty(dict,"BoardSerialNumber");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.BoardSerialNumber, prop->string);
		}
		prop=GetProperty(dict,"Board-ID");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.BoardNumber, prop->string);
		}
		prop=GetProperty(dict,"LocationInChassis");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.LocationInChassis, prop->string);
		}
		
		prop=GetProperty(dict,"ChassisManufacturer");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.ChassisManufacturer, prop->string);
		}
		prop=GetProperty(dict,"ChassisAssetTag");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.ChassisAssetTag, prop->string);
		}
		prop=GetProperty(dict,"smartUPS");
		if(prop!=NULL)
		{
			if (AsciiStriCmp(prop->string,"y")==0 || AsciiStriCmp(prop->string,"Y")==0)
				gSettings.smartUPS=TRUE;
			else
				gSettings.smartUPS=FALSE;
		}
		prop=GetProperty(dict,"ShowLegacyBoot");
		if(prop!=NULL)
		{
			if (AsciiStriCmp(prop->string,"y")==0 || AsciiStriCmp(prop->string,"Y")==0)
				gSettings.ShowLegacyBoot=TRUE;
			else
				gSettings.ShowLegacyBoot=FALSE;
		}
    
/*		
		prop=GetProperty(dict,"MemorySerialNumber");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.MemorySerialNumber, prop->string);
		}
		prop=GetProperty(dict,"MemoryPartNumber");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.MemoryPartNumber, prop->string);
		}
 */
		prop=GetProperty(dict,"CpuFrequencyMHz");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.CpuFreqMHz, prop->string);
			gCpuSpeed = (UINT16)AsciiStrDecimalToUintn(gSettings.CpuFreqMHz);
		}
		prop=GetProperty(dict,"ProcessorType");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.CpuType, prop->string);
			gCPUtype = (UINT16)AsciiStrHexToUintn(gSettings.CpuType);
		} else {
			gCPUtype = GetAdvancedCpuType();
		}

		prop=GetProperty(dict,"BusSpeed");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettings.BusSpeed, prop->string);
			gBusSpeed = (UINT16)AsciiStrDecimalToUintn(gSettings.BusSpeed);
		}
		SaveSettings();
	}	
	return Status;
}	
