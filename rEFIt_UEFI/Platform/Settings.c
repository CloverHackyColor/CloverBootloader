/*
 Slice 2012
*/

#include "Platform.h"

#define DEBUG_BOOT 0

#if DEBUG_BOOT == 2
#define DBG(x...) AsciiPrint(x)
#elif DEBUG_BOOT == 1
#define DBG(x...) MsgLog(x)
#else
#define DBG(x...)
#endif

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
			AsciiStrCpy(gSettingsFromMenu.KernelFlags, prop->string);
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
			AsciiStrCpy(gSettingsFromMenu.Language, prop->string);
		} else {
			prop=GetProperty(dict,"prev-lang:kbd");
			if(prop!=NULL)
			{
				AsciiStrCpy(gSettingsFromMenu.Language, prop->string);
			}
		}
		prop=GetProperty(dict,"boot-args");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.KernelFlags, prop->string);
		} else {
			prop=GetProperty(dict,"KernelFlags");
			if(prop!=NULL)
			{
				AsciiStrCpy(gSettingsFromMenu.KernelFlags, prop->string);
			}
		}
		//gSettingsFromMenu.TimeOut
		prop=GetProperty(dict,"TimeOut");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.TimeOut, prop->string);
			gTimeoutSec = (UINT16)AsciiStrDecimalToUintn(gSettingsFromMenu.TimeOut);	
		} else {
			gTimeoutSec = 5;
			AsciiStrCpy(gSettingsFromMenu.TimeOut, "5");
		}
		

		//*** ACPI ***//
		prop=GetProperty(dict,"ResetAddress");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.ResetAddr, prop->string);
			gResetAddress  = (UINT16)AsciiStrHexToUintn(gSettingsFromMenu.ResetAddr); 
		}  else {
			gResetAddress  = 0x64; //I wish it will be default
			AsciiStrCpy(gSettingsFromMenu.ResetAddr,"0x64");
		}
		prop=GetProperty(dict,"ResetValue");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.ResetVal, prop->string);
			gResetValue = (UINT16)AsciiStrHexToUintn(gSettingsFromMenu.ResetVal);	
		} else {
			gResetValue = 0xFE;
			AsciiStrCpy(gSettingsFromMenu.ResetVal, "0xFE");
		}
		//other known pair is 0x02F9/0x06
		
		//*** SMBIOS ***//
		prop=GetProperty(dict,"BiosVendor");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.VendorName, prop->string);
		}
		prop=GetProperty(dict,"BiosVersion");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.RomVersion, prop->string);
		}
		prop=GetProperty(dict,"BiosReleaseDate");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.ReleaseDate, prop->string);
		}
		prop=GetProperty(dict,"Manufacturer");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.ManufactureName, prop->string);
		}
		prop=GetProperty(dict,"ProductName");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.ProductName, prop->string);
		}
		prop=GetProperty(dict,"Version");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.VersionNr, prop->string);
		}
		prop=GetProperty(dict,"Family");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.FamilyName, prop->string);
		}
		prop=GetProperty(dict,"SerialNumber");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.SerialNr, prop->string);
		}
		prop=GetProperty(dict,"CustomUUID");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.CustomUuid, prop->string);
		}
		prop=GetProperty(dict,"BoardManufacturer");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.BoardManufactureName, prop->string);
		}
		prop=GetProperty(dict,"BoardSerialNumber");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.BoardSerialNumber, prop->string);
		}
		prop=GetProperty(dict,"Board-ID");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.BoardNumber, prop->string);
		}
		prop=GetProperty(dict,"LocationInChassis");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.LocationInChassis, prop->string);
		}
		
		prop=GetProperty(dict,"ChassisManufacturer");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.ChassisManufacturer, prop->string);
		}
		prop=GetProperty(dict,"ChassisAssetTag");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.ChassisAssetTag, prop->string);
		}
		prop=GetProperty(dict,"smartUPS");
		if(prop!=NULL)
		{
			if (AsciiStriCmp(prop->string,"y")==0 || AsciiStriCmp(prop->string,"Y")==0)
				gSettingsFromMenu.smartUPS=TRUE;
			else
				gSettingsFromMenu.smartUPS=FALSE;
		}
		prop=GetProperty(dict,"ShowLegacyBoot");
		if(prop!=NULL)
		{
			if (AsciiStriCmp(prop->string,"y")==0 || AsciiStriCmp(prop->string,"Y")==0)
				gSettingsFromMenu.ShowLegacyBoot=TRUE;
			else
				gSettingsFromMenu.ShowLegacyBoot=FALSE;
		}
    
/*		
		prop=GetProperty(dict,"MemorySerialNumber");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.MemorySerialNumber, prop->string);
		}
		prop=GetProperty(dict,"MemoryPartNumber");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.MemoryPartNumber, prop->string);
		}
 */
		prop=GetProperty(dict,"CpuFrequencyMHz");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.CpuFreqMHz, prop->string);
			gCpuSpeed = (UINT16)AsciiStrDecimalToUintn(gSettingsFromMenu.CpuFreqMHz);
		}
		prop=GetProperty(dict,"ProcessorType");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.CpuType, prop->string);
			gCPUtype = (UINT16)AsciiStrHexToUintn(gSettingsFromMenu.CpuType);
		} else {
			gCPUtype = GetAdvancedCpuType();
		}

		prop=GetProperty(dict,"ProcessorBusSpeed");
		if(prop!=NULL)
		{
			AsciiStrCpy(gSettingsFromMenu.BusSpeed, prop->string);
			gBusSpeed = (UINT16)AsciiStrDecimalToUintn(gSettingsFromMenu.BusSpeed);
		}
		SaveSettings();
	}	
	return Status;
}	
