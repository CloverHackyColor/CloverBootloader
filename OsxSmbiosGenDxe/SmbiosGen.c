/** @file

Copyright (c) 2009 - 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  SmbiosGen.c

Abstract:
2011 Slice - patch for MacOSX
**/

#include "SmbiosGen.h"
#include "cpuid.h"
EFI_HII_DATABASE_PROTOCOL   *gHiiDatabase;
extern UINT8                SmbiosGenDxeStrings[];
EFI_SMBIOS_PROTOCOL         *gSmbios;
EFI_HII_HANDLE              gStringHandle;

UINT8			gMacType;
BOOLEAN			gMobile;
UINT16			gCpuType;
UINT16			gBusSpeed;

enum MachineTypes {
	MacBook11 = 0,
	MacBook21,
	MacBook41,
	MacBookPro51,
	MacBookAir11,
	MacMini21,
	iMac112,
	MacPro31
};

CHAR8* SMbiosversion[8] = {  //t0 BiosVersion
	"MB11.0061.B03.0809221748",
	"MB21.88Z.00A5.B07.0706270922",
	"MB41.88Z.0073.B00.0809221748",
	"MBP51.88Z.00AC.B03.0906151647",
	"MBA11.88Z.00BB.B00.0712201139",
	"MM21.88Z.009A.B00.0706281359",
	"IM112.0034.B00.0802091538",
	"MP31.88Z.006C.B05.0802291410"
};

CHAR8* SMboardproduct[8] = { //t2 ProductName
	"Mac-F4208CA9",
	"Mac-F4208CA9",
	"Mac-F42D89C8",
	"Mac-F22587C8",
	"Mac-F42C8CC8",
	"Mac-F4208EAA",
	"Mac-F2268DAE",
	"Mac-F4208DC8"
};

CHAR8* SMserial = "W87234JHYA4";  //t1,t2 SerialNumber, 
CHAR8* SMbiosvendor = "Apple Inc.";  //t0 Vendor
CHAR8* SMboardmanufacter = "Apple Computer, Inc."; //t1, t2 Manufacturer

CHAR8* SMproductname[8] = { //t1 ProductName
	"MacBook1,1",
	"MacBook2,1",
	"MacBook4,1",
	"MacBookPro5,1",
	"MacBookAir1,1",
	"MacMini2,1",
	"iMac11,2",
	"MacPro3,1"
};

CHAR8* Family[8] = {  //t1 Family
	"MacBook",
	"MacBook",
	"MacBook",
	"MacBookPro",
	"MacBookAir",
	"MacMini",
	"iMac"
	"MacPro"
};


VOID *
GetSmbiosTablesFromHob (
  VOID
  )
{
  EFI_PHYSICAL_ADDRESS       *Table;
  EFI_PEI_HOB_POINTERS        GuidHob;

  GuidHob.Raw = GetFirstGuidHob (&gEfiSmbiosTableGuid);
  if (GuidHob.Raw != NULL) {
    Table = GET_GUID_HOB_DATA (GuidHob.Guid);
    if (Table != NULL) {
      return (VOID *)(UINTN)*Table;
    }
  }

  return NULL;
}


VOID
InstallProcessorSmbios (
  IN VOID                  *Smbios
  )
{
  SMBIOS_STRUCTURE_POINTER          SmbiosTable;
  CHAR8                             *AString;
  CHAR16                            *UString;
  STRING_REF                        Token;

  //
  // Processor info (TYPE 4)
  // 
  SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 4, 0);
  if (SmbiosTable.Raw == NULL) {
//    DEBUG ((EFI_D_ERROR, "SmbiosTable: Type 4 (Processor Info) not found!\n"));
    return ;
  }
	gBusSpeed = SmbiosTable.Type4->ExternalClock;
	if (SmbiosTable.Type4->CoreCount > 20) {  // :)
		SmbiosTable.Type4->CoreCount = cpuid_info()->core_count;
		SmbiosTable.Type4->ThreadCount = cpuid_info()->thread_count;
	}
	if (SmbiosTable.Type4->CoreCount < SmbiosTable.Type4->EnabledCoreCount) {
		SmbiosTable.Type4->EnabledCoreCount = cpuid_info()->core_count;
	}
  //
  // Log Smbios Record Type4
  //
  LogSmbiosData(gSmbios,(UINT8*)SmbiosTable.Type4);

  //
  // Set ProcessorVersion string
  //
  AString = GetSmbiosString (SmbiosTable, SmbiosTable.Type4->ProcessorVersion);
  UString = AllocateZeroPool ((AsciiStrLen(AString) + 1) * sizeof(CHAR16));
  ASSERT (UString != NULL);
  AsciiStrToUnicodeStr (AString, UString);

  Token = HiiSetString (gStringHandle, 0, UString, NULL);
  if (Token == 0) {
    gBS->FreePool (UString);
    return ;
  }
  gBS->FreePool (UString);
  return ;
}

VOID
InstallCacheSmbios ( //7
  IN VOID                  *Smbios
  )
{
	SMBIOS_STRUCTURE_POINTER          SmbiosTable;
	UINTN i;
	//
	// Cache info (TYPE 7)
	// 
	//Slice - How many caches do we have ?
	for (i=0; i<8; i++) {
		SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 7, i);
		if (SmbiosTable.Raw == NULL) {
			return ;
		}		
		// Log Smbios Record Type7
		//
		LogSmbiosData(gSmbios,(UINT8*)SmbiosTable.Type7);		
	}
	return ;
}

//Slice
VOID
InstallBaseBoardSmbios			( //2
IN VOID                  *Smbios
								 )
{
	SMBIOS_STRUCTURE_POINTER          SmbiosTable;
	EFI_SMBIOS_HANDLE				Handle;
	UINTN StringNumber = 0;
	//
	// BaseBoard (TYPE 2)
	// 
	SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 2, 0);
	if (SmbiosTable.Raw == NULL) {
		return ;
	}
	Handle = LogSmbiosData(gSmbios,(UINT8*)SmbiosTable.Type2);
	StringNumber = SmbiosTable.Type2->Manufacturer;
	gSmbios->UpdateString(gSmbios,
						  &Handle,
						  &StringNumber,
						  SMboardmanufacter); 
	//CHAR8* SMboardproduct[8] = { //t2 ProductName
	StringNumber = SmbiosTable.Type2->ProductName;
	gSmbios->UpdateString(gSmbios,
						  &Handle,
						  &StringNumber,
						  SMboardproduct[gMacType]); 
	//SMserial = "W87234JHYA4";  //t1,t2 SerialNumber,
	StringNumber = SmbiosTable.Type2->SerialNumber;
	gSmbios->UpdateString(gSmbios,
						  &Handle,
						  &StringNumber,
						  SMserial); 
	StringNumber = SmbiosTable.Type1->Version;
	gSmbios->UpdateString(gSmbios,
						  &Handle,
						  &StringNumber,
						  SMbiosversion[gMacType]); 
	
	return ;
}

VOID									
InstallSystemEnclosureSmbios    (//3
  IN VOID                  *Smbios
  )
{
	SMBIOS_STRUCTURE_POINTER          SmbiosTable;
	gMobile = FALSE;
	//
	// SystemEnclosure (TYPE 3)
	// 
	SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 3, 0);
	if (SmbiosTable.Raw == NULL) {
		//    DEBUG ((EFI_D_ERROR, "SmbiosTable: Type 3 (SystemEnclosure) not found!\n"));
		return ;
	}
	gMobile = ((SmbiosTable.Type3->Type) >= 8);
	//
	// Log Smbios Record Type3
	//
	LogSmbiosData(gSmbios,(UINT8*)SmbiosTable.Type3);
	return ;
}

VOID									
InstallMemoryModuleSmbios		(//6
  IN VOID                  *Smbios
  )
{
	SMBIOS_STRUCTURE_POINTER          SmbiosTable;
	UINTN i;
	//
	// MemoryModule (TYPE 6)
	// 
	//we can have more then 1 module
	for (i=0; i<8; i++) {
		SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 6, i);
		if (SmbiosTable.Raw == NULL) {
			return ;
		}
		//
		// Log Smbios Record Type6
		//
		LogSmbiosData(gSmbios,(UINT8*)SmbiosTable.Type6);
		
	}
	return ;
}

VOID
InstallSystemSlotSmbios			(//9
  IN VOID                  *Smbios
  )
{
	SMBIOS_STRUCTURE_POINTER          SmbiosTable;
	UINTN	i;
	//
	// SystemSlot (TYPE 9)
	// 
	for (i=0; i<16; i++) {
		SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 9, i);
		if (SmbiosTable.Raw == NULL) {
			return ;
		}	
		//
		// Log Smbios Record Type6
		//
		LogSmbiosData(gSmbios,(UINT8*)SmbiosTable.Type9);
		
	}
	
	return ;
}

VOID									
InstallPhysicalMemoryArraySmbios(//16
  IN VOID                  *Smbios
  )
{
	SMBIOS_STRUCTURE_POINTER          SmbiosTable;
	//
	// PhysicalMemoryArray (TYPE 16)
	// 
	SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 16, 0);
	if (SmbiosTable.Raw == NULL) {
		//    DEBUG ((EFI_D_ERROR, "SmbiosTable: Type 16 (PhysicalMemoryArray) not found!\n"));
		return ;
	}	
	//
	// Log Smbios Record Type16
	//
	LogSmbiosData(gSmbios,(UINT8*)SmbiosTable.Type16);
	
	return ;
}

VOID									
InstallMemoryDeviceSmbios		(//17
  IN VOID                  *Smbios
  )
{
	SMBIOS_STRUCTURE_POINTER          SmbiosTable;
	UINTN	i;
  //
  // MemoryDevice (TYPE 17)
  // 
	for (i=0; i<8; i++) {
		SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 17, i);
		if (SmbiosTable.Raw == NULL) {
			//    DEBUG ((EFI_D_ERROR, "SmbiosTable: Type 17 (MemoryDevice) not found!\n"));
			return ;
		}
		
		//
		// Log Smbios Record Type17
		//
		LogSmbiosData(gSmbios,(UINT8*)SmbiosTable.Type17);
	}
	return ;
}

		// Apple Specific Structures
VOID
InstallFirmwareVolumeSmbios		(//128
  IN VOID                  *Smbios
  )
{
	SMBIOS_STRUCTURE_POINTER          SmbiosTable;
	//
	// FirmwareVolume (TYPE 128)
	// 
	SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 128, 0);
	if (SmbiosTable.Raw == NULL) {
		//    DEBUG ((EFI_D_ERROR, "SmbiosTable: Type 128 (FirmwareVolume) not found!\n"));
		return ;
	}	
	//
	// Log Smbios Record Type128
	//
	LogSmbiosData(gSmbios,(UINT8*)SmbiosTable.Type128);
	
	return ;
}

VOID									
InstallMemorySPDSmbios			(//130
  IN VOID                  *Smbios
  )
{
	SMBIOS_STRUCTURE_POINTER          SmbiosTable;
	//
	// MemorySPD (TYPE 130)
	// 
	SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 130, 0);
	if (SmbiosTable.Raw == NULL) {
		return ;
	}	
	//
	// Log Smbios Record Type130
	//
	LogSmbiosData(gSmbios,(UINT8*)SmbiosTable.Type130);	
	return ;
}

VOID									
InstallOemProcessorTypeSmbios	(//131
  IN VOID                  *Smbios
  )
{
	SMBIOS_STRUCTURE_POINTER          SmbiosTable;
	//
	// OemProcessorType (TYPE 131)
	// 
	SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 131, 0);
	if (SmbiosTable.Raw == NULL) {
		SmbiosTable = (SMBIOS_STRUCTURE_POINTER)(SMBIOS_TABLE_TYPE131*)AllocateZeroPool(sizeof(SMBIOS_TABLE_TYPE131));
		SmbiosTable.Type131->Hdr.Type = 131;
		SmbiosTable.Type131->Hdr.Length = sizeof(SMBIOS_STRUCTURE)+2; 
		SmbiosTable.Type131->ProcessorType = gCpuType;
	}	
	//
	// Log Smbios Record Type131
	//
	LogSmbiosData(gSmbios,(UINT8*)SmbiosTable.Type131);
	
	return ;
}

VOID									
InstallOemProcessorBusSpeed		(//132					 
  IN VOID                  *Smbios
  )
{
	SMBIOS_STRUCTURE_POINTER          SmbiosTable;
	//
	// OemProcessorBus (TYPE 132)
	// 
	SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 132, 0);
	if (SmbiosTable.Raw == NULL) {
		SmbiosTable = (SMBIOS_STRUCTURE_POINTER)(SMBIOS_TABLE_TYPE132*)AllocateZeroPool(sizeof(SMBIOS_TABLE_TYPE132));
		SmbiosTable.Type132->Hdr.Type = 132;
		SmbiosTable.Type132->Hdr.Length = sizeof(SMBIOS_STRUCTURE)+2; 
		//    return ;
	}
	SmbiosTable.Type132->ProcessorBusSpeed = gBusSpeed;
	//
	// Log Smbios Record Type132
	//
	LogSmbiosData(gSmbios,(UINT8*)SmbiosTable.Type132);	
	return ;
}
										

VOID
InstallMemorySmbios (
  IN VOID                  *Smbios
  )
{
	SMBIOS_STRUCTURE_POINTER          SmbiosTable;
	UINTN	i;
	//
	// Generate Memory Array Mapped Address info (TYPE 19)
	//
	for (i=0; i<16; i++) {
		SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 19, i);
		if (SmbiosTable.Raw == NULL) {			
			return ;
		}		
		//
		// Record Smbios Type 19
		//
		LogSmbiosData(gSmbios, (UINT8*)SmbiosTable.Type19);		
	}
	return ;
}

VOID
InstallMemoryMapSmbios (
					 IN VOID          *Smbios
					 )
{
	SMBIOS_STRUCTURE_POINTER          SmbiosTable;
	UINTN	i;
	//
	// Generate Memory Array Mapped Address info (TYPE 20)
	//
	for (i=0; i<16; i++) {
		SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 20, i);
		if (SmbiosTable.Raw == NULL) {			
			return ;
		}		
		//
		// Record Smbios Type 20
		//
		LogSmbiosData(gSmbios, (UINT8*)SmbiosTable.Type20);		
	}
	return ;
}

VOID
InstallMiscSmbios (
				   IN VOID          *Smbios
				   )
{
	SMBIOS_STRUCTURE_POINTER          SmbiosTable;
	CHAR8                             *AString;
	CHAR16                            *UString;
	STRING_REF                        Token;
	EFI_SMBIOS_HANDLE				Handle;
	UINTN StringNumber = 0;
	//
	// BIOS information (TYPE 0)
	// 
	SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 0, 0);
	if (SmbiosTable.Raw == NULL) {
		//   DEBUG ((EFI_D_ERROR, "SmbiosTable: Type 0 (BIOS Information) not found!\n"));
		return ;
	}	
	//
	// Record Num 2
	//
	AString = GetSmbiosString (SmbiosTable, SmbiosTable.Type0->BiosVersion);
	UString = AllocateZeroPool ((AsciiStrLen(AString) + 1) * sizeof(CHAR16) + sizeof(FIRMWARE_BIOS_VERSIONE));
	ASSERT (UString != NULL);
	CopyMem (UString, FIRMWARE_BIOS_VERSIONE, sizeof(FIRMWARE_BIOS_VERSIONE));
	AsciiStrToUnicodeStr (AString, UString + sizeof(FIRMWARE_BIOS_VERSIONE) / sizeof(CHAR16) - 1);
	
	Token = HiiSetString (gStringHandle, 0, UString, NULL);
	if (Token == 0) {
		gBS->FreePool (UString);
		return ;
	}
	gBS->FreePool (UString);	
	//
	// Log Smios Type 0
	//
	Handle = LogSmbiosData(gSmbios, (UINT8*)SmbiosTable.Type0);
	StringNumber = SmbiosTable.Type0->Vendor;
	gSmbios->UpdateString(gSmbios,
						  &Handle,
						  &StringNumber,
						  SMbiosvendor); 
	StringNumber = SmbiosTable.Type0->BiosVersion;
	gSmbios->UpdateString(gSmbios,
						  &Handle,
						  &StringNumber,
						  SMbiosversion[gMacType]); 
	//
	// System information (TYPE 1)
	// 
	SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 1, 0);
	if (SmbiosTable.Raw == NULL) {
		//    DEBUG ((EFI_D_ERROR, "SmbiosTable: Type 1 (System Information) not found!\n"));
		return ;
	}
	
	//
	// Record Type 1
	//
	AString = GetSmbiosString (SmbiosTable, SmbiosTable.Type1->ProductName);
	UString = AllocateZeroPool ((AsciiStrLen(AString) + 1) * sizeof(CHAR16) + sizeof(FIRMWARE_PRODUCT_NAME));
	ASSERT (UString != NULL);
	CopyMem (UString, FIRMWARE_PRODUCT_NAME, sizeof(FIRMWARE_PRODUCT_NAME));
	AsciiStrToUnicodeStr (AString, UString + sizeof(FIRMWARE_PRODUCT_NAME) / sizeof(CHAR16) - 1);
	
	Token = HiiSetString (gStringHandle, 0, UString, NULL);
	if (Token == 0) {
		gBS->FreePool (UString);
		return ;
	}
	gBS->FreePool (UString);
	
	//
	// Log Smbios Type 1
	//
	Handle = LogSmbiosData(gSmbios, (UINT8*)SmbiosTable.Type1);
	StringNumber = SmbiosTable.Type1->Manufacturer;
	gSmbios->UpdateString(gSmbios,
						  &Handle,
						  &StringNumber,
						  SMboardmanufacter); 
	StringNumber = SmbiosTable.Type1->ProductName;
	gSmbios->UpdateString(gSmbios,
						  &Handle,
						  &StringNumber,
						  SMproductname[gMacType]); 
	StringNumber = SmbiosTable.Type1->SerialNumber;
	gSmbios->UpdateString(gSmbios,
						  &Handle,
						  &StringNumber,
						  SMserial); 
	StringNumber = SmbiosTable.Type1->Family;
	gSmbios->UpdateString(gSmbios,
						  &Handle,
						  &StringNumber,
						  Family[gMacType]); 
	return ;
}

EFI_STATUS
EFIAPI
SmbiosGenEntrypoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS              Status;
  VOID                    *Smbios;

  Smbios = GetSmbiosTablesFromHob ();
  if (Smbios == NULL) {
    return EFI_NOT_FOUND;
  }

  Status = gBS->LocateProtocol (
                  &gEfiSmbiosProtocolGuid,
                  NULL,
                  (VOID**)&gSmbios
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID**)&gHiiDatabase
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  gStringHandle = HiiAddPackages (
                    &gEfiCallerIdGuid,
                    NULL,
                    SmbiosGenDxeStrings,
                    NULL
                    );
  ASSERT (gStringHandle != NULL);
	cpuid_update_generic_info();
	InstallSystemEnclosureSmbios    (Smbios); //3
	/*
	 N = cpuid_cpu_info.core_count;
	 */
	/* Mobile CPU ? */
	//Slice 
/*	msr = rdmsr64(MSR_IA32_PLATFORM_ID);
	DBG("msr(0x%04x): MSR_IA32_PLATFORM_ID 0x%08x\n", MSR_IA32_PLATFORM_ID, msr & 0xffffffff); //__LINE__ - source line number :)
	if (!scanDMI() && msr) {
		p->CPU.Mobile = FALSE;
		switch (p->CPU.Model) {
			case 0x0D:
				p->CPU.Mobile = TRUE; // CPU_FEATURE_MOBILE;
				break;
			case 0x0F:
				p->CPU.Mobile = FALSE; // CPU_FEATURE_MOBILE;
				break;
			case 0x02:
			case 0x03:
			case 0x04:
			case 0x06:	
				p->CPU.Mobile = (rdmsr64(MSR_P4_EBC_FREQUENCY_ID) && (1 << 21));
				break;
			default:
				p->CPU.Mobile = (rdmsr64(MSR_IA32_PLATFORM_ID) && (1<<28));
				break;
		}
		if (p->CPU.Mobile) {
			p->CPU.Features |= CPU_FEATURE_MOBILE;
		}
	}
	DBG("CPU is %s\n", p->CPU.Mobile?"Mobile":"Desktop");
*/
	
	gCpuType = 0; //Later decided
	if (gMobile) {
		switch (cpuid_info()->cpuid_model) {
			case CPU_MODEL_PENTIUM_M: 	
			case CPU_MODEL_YONAH: 
				gMacType = MacBook11;
				gCpuType = 0x101;
				break;
			case CPU_MODEL_MEROM: 
				gMacType = MacBook21;
				gCpuType = 0x201;
				break;
			case CPU_MODEL_PENRYN:
				//TODO. If Video=Intel then MB41. If Nvidia then MBP51
				gMacType = MacBook41;
				gCpuType = 0x301;
				break;
			case CPU_MODEL_ATOM:
				gMacType = MacMini21;
				gCpuType = 0x401;
				break;
	
			default:
				gMacType = MacBookPro51;
				gCpuType = 0x501;
				break;
		}
	} else {
		switch (cpuid_cpu_info.cpuid_extmodel) {
			case CPU_MODEL_MEROM: //or Conroe!
			case CPU_MODEL_PENRYN:	
				gMacType = iMac112;
				gCpuType = 0x201;
				break;
			case CPU_MODEL_DALES_32NM:
				gMacType = iMac112;
				gCpuType = 0x901;
				break;
			case CPU_MODEL_NEHALEM: 
			case CPU_MODEL_FIELDS: 
			case CPU_MODEL_DALES: 
				gMacType = iMac112;
				gCpuType = 0x701;
				break;
			default:
				gMacType = MacPro31;
				//gCpuType = 0x801;
				break;
		}
	}
	if (gCpuType == 0) {
		gCpuType = (cpuid_cpu_info.core_count > 4)?0x701:0x601;
	}

	
	InstallProcessorSmbios (Smbios); //kSMBTypeProcessorInformation=4
	InstallCacheSmbios     (Smbios); //kSMBTypeCacheInformation=7 
	InstallMemorySmbios    (Smbios); //Memory Array Mapped Address info (TYPE 19) unknown to Apple
	InstallMemoryMapSmbios    (Smbios); //Memory Device Mapped info (TYPE 20) unknown to Apple
	InstallMiscSmbios      (Smbios); //kSMBTypeSystemInformation=1 kSMBTypeBIOSInformation=0
	InstallBaseBoardSmbios			(Smbios); //2

	InstallMemoryModuleSmbios		(Smbios); //6
	InstallSystemSlotSmbios			(Smbios); //9
	InstallPhysicalMemoryArraySmbios(Smbios); //16
	InstallMemoryDeviceSmbios		(Smbios); //17
// Apple Specific Structures
	InstallFirmwareVolumeSmbios		(Smbios); //128
	InstallMemorySPDSmbios			(Smbios); //130
	InstallOemProcessorTypeSmbios	(Smbios); //131
	InstallOemProcessorBusSpeed		(Smbios); //132

  return EFI_SUCCESS;
}

//
// Internal function
//

UINTN
SmbiosTableLength (
  IN SMBIOS_STRUCTURE_POINTER SmbiosTable
  )
{
  CHAR8  *AChar;
  UINTN  Length;

  AChar = (CHAR8 *)(SmbiosTable.Raw + SmbiosTable.Hdr->Length);
  while ((*AChar != 0) || (*(AChar + 1) != 0)) {
    AChar ++;
  }
  Length = ((UINTN)AChar - (UINTN)SmbiosTable.Raw + 2);
  
  return Length;
}

SMBIOS_STRUCTURE_POINTER
GetSmbiosTableFromType (
  IN SMBIOS_TABLE_ENTRY_POINT  *Smbios,
  IN UINT8                     Type,
  IN UINTN                     Index
  )
{
  SMBIOS_STRUCTURE_POINTER SmbiosTable;
  UINTN                    SmbiosTypeIndex;
  
  SmbiosTypeIndex = 0;
  SmbiosTable.Raw = (UINT8 *)(UINTN)Smbios->TableAddress;
  if (SmbiosTable.Raw == NULL) {
    return SmbiosTable;
  }
  while ((SmbiosTypeIndex != Index) || (SmbiosTable.Hdr->Type != Type)) {
    if (SmbiosTable.Hdr->Type == 127) {
      SmbiosTable.Raw = NULL;
      return SmbiosTable;
    }
    if (SmbiosTable.Hdr->Type == Type) {
      SmbiosTypeIndex ++;
    }
    SmbiosTable.Raw = (UINT8 *)(SmbiosTable.Raw + SmbiosTableLength (SmbiosTable));
  }

  return SmbiosTable;
}

CHAR8 *
GetSmbiosString (
  IN SMBIOS_STRUCTURE_POINTER  SmbiosTable,
  IN SMBIOS_TABLE_STRING       String
  )
{
  CHAR8      *AString;
  UINT8      Index;

  Index = 1;
  AString = (CHAR8 *)(SmbiosTable.Raw + SmbiosTable.Hdr->Length);
  while (Index != String) {
    while (*AString != 0) {
      AString ++;
    }
    AString ++;
    if (*AString == 0) {
      return AString;
    }
    Index ++;
  }

  return AString;
}


/**
  Logs SMBIOS record.

  @param  Smbios   Pointer to SMBIOS protocol instance.
  @param  Buffer   Pointer to the data buffer.

**/
EFI_SMBIOS_HANDLE
LogSmbiosData (
  IN   EFI_SMBIOS_PROTOCOL        *Smbios,
  IN   UINT8                      *Buffer
  )
{
  EFI_STATUS         Status;
  EFI_SMBIOS_HANDLE  SmbiosHandle;
  
  SmbiosHandle = 0;
  Status = Smbios->Add (
                     Smbios,
                     NULL,
                     &SmbiosHandle,
                     (EFI_SMBIOS_TABLE_HEADER*)Buffer
                     );
  ASSERT_EFI_ERROR (Status);
	return SmbiosHandle;
}
