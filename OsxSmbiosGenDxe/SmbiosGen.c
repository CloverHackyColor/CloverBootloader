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
/*
 kSMBType32BitMemoryErrorInfo        = 18,
 kSMBType64BitMemoryErrorInfo        = 33,

 */

EFI_HII_DATABASE_PROTOCOL   *gHiiDatabase;
extern UINT8                SmbiosGenDxeStrings[];
EFI_SMBIOS_PROTOCOL         *gSmbios;
EFI_HII_HANDLE              gStringHandle;
UINT32			mTotalSystemMemory;
UINT16			mHandle3;
UINT16			mHandle16;
UINT16			mHandleL1;
UINT16			mHandleL2;
UINT16			mHandleL3;
UINT16			mHandle17[8];
UINT16			mMemCount;

UINT8			gMacType;
BOOLEAN			gMobile;
UINT16			gCpuType;
UINT16			gBusSpeed;

enum MachineTypes {
	MacBook11 = 0,
	MacBook21,
	MacBook41,
	MacBook52,
	MacBookPro51,
	MacBookAir11,
	MacMini21,
	iMac112,
	MacPro31
};

CHAR8* SMbiosversion[9] = {  //t0 BiosVersion
	"MB11.0061.B03.0809221748",  //SMC 1.4f12
	"MB21.88Z.00A5.B07.0706270922",
	"MB41.88Z.0073.B00.0809221748",  //SMC 1.31f1
	"MB52.88Z.007D.003.0809221748",  //SMC 1.32f8
	"MBP51.88Z.007E.B05.0906151647",  //SMC 1.33f8
	"MBA11.88Z.00BB.B03.0712201139",
	"MM21.88Z.009A.B00.0706281359",
	"IM112.0034.B00.0802091538",
	"MP31.88Z.006C.B05.0802291410"
};
/*
 MB71.0039.B0B (EFI2.0)
 MM31.0081.B06 (EFI1.2)
 MBP55.00AC.B03 (EFI1.7)
 MBP71.0039.B0B (EFI2.0)
 MB51.007F.B03 (EFI1.5)
 MBA31.0061.B01 (EFI2.0)
 IM111.0034.B02 (EFI1.0)? SMC 1.54f36
 */

CHAR8* SMboardproduct[9] = { //t2 ProductName
	"Mac-F4208CC8",
	"Mac-F4208CA9",
	"Mac-F22788A9", //F42D89C8",
	"Mac-F22788AA", //F227BEC8", //"Mac-F22788C8",
	"Mac-F42D86C8", //F22587C8",
	"Mac-F42C8CC8",
	"Mac-F4208EAA",
	"Mac-F2238AC8",
	"Mac-F4208DC8"
};

CHAR8* SMserials[9] = {
	"4H629LYAU9C",  //+
	"4H7044LUWGP",  //+  
	"RM83064H0P1",  //+
	"W88AAAAA9GU", //"W874725NZ66", //MacBook3,1 
	"W88439FE1G0",  //+
	"W8811456Y51",  //+  
	"YM8054BYYL2",  //+
	"W8034342DB7",  //+
	"CK80728AXYL"	//+
};
CHAR8* SMserial  = "W87234JHYA4";//MB21  //t1,t2 SerialNumber, "W874725NZ66" //MB31
CHAR8* SMserial2 = "W874725NZ66";
CHAR8* SMbiosvendor = "Apple Inc.";  //t0 Vendor
CHAR8* SMboardmanufacter = "Apple Computer, Inc."; //t1, t2 Manufacturer

CHAR8* SMproductname[9] = { //t1 ProductName
	"MacBook1,1",
	"MacBook2,1",
	"MacBook4,1",
	"MacBook5,2",
	"MacBookPro5,1",
	"MacBookAir1,1",
	"Macmini2,1",
	"iMac11,2",
	"MacPro3,1"
};

CHAR8* Family[9] = {  //t1 Family
	"MacBook",
	"MacBook",
	"MacBook",
	"MacBook",
	"MacBookPro",
	"MacBookAir",
	"Macmini",
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


VOID
InstallProcessorSmbios (  //4
  IN VOID                  *Smbios
  )
{
	SMBIOS_STRUCTURE_POINTER        SmbiosTable;
	SMBIOS_STRUCTURE_POINTER        newSmbiosTable;
	CHAR8                           *AString;
	CHAR16                          *UString;
	STRING_REF                      Token;
	UINTN							Size, BigSize, newSize;
	CHAR8							*Socket, *Manuf, *Ver, *SN, *Asset, *Part;
	UINTN							StringNumber = 0;
	EFI_SMBIOS_HANDLE				Handle;
//	CHAR8							*StrStart;
	UINTN							AddBrand = 0;
	CHAR8							BrandStr[48];
	CopyMem(BrandStr, cpuid_info()->cpuid_brand_string, 48);
	//
	// Processor info (TYPE 4)
	// 
	SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 4, 0);
	if (SmbiosTable.Raw == NULL) {
		//    DEBUG ((EFI_D_ERROR, "SmbiosTable: Type 4 (Processor Info) not found!\n"));
		return ;
	}
	Size = SmbiosTable.Type4->Hdr.Length;
	BigSize = SmbiosTableLength(SmbiosTable);
	if (SmbiosTable.Type4->ProcessorVersion == 0) {
		AddBrand = 48;
	}
	
	Socket = GetSmbiosString (SmbiosTable, SmbiosTable.Type4->Socket);
	Manuf  = GetSmbiosString (SmbiosTable, SmbiosTable.Type4->ProcessorManufacture);
	Ver    = GetSmbiosString (SmbiosTable, SmbiosTable.Type4->ProcessorVersion);
	SN = NULL;
	Asset = NULL;
	Part = NULL;
	if (Size > 0x20) {
		SN    = GetSmbiosString (SmbiosTable, SmbiosTable.Type4->SerialNumber);
		Asset = GetSmbiosString (SmbiosTable, SmbiosTable.Type4->AssetTag);
		Part  = GetSmbiosString (SmbiosTable, SmbiosTable.Type4->PartNumber);
	}
//	if (Size > 0x28) {Size = 0x28;}
	//Smbios 2.6 has size = 0x2a
	newSize = sizeof(SMBIOS_TABLE_TYPE4);
	newSmbiosTable = (SMBIOS_STRUCTURE_POINTER)(SMBIOS_TABLE_TYPE4*)AllocateZeroPool(BigSize + newSize - Size + AddBrand);
	CopyMem((VOID*)newSmbiosTable.Type4, (VOID*)SmbiosTable.Type4, Size); //copy old data
	CopyMem((CHAR8*)newSmbiosTable.Type4+newSize, (CHAR8*)SmbiosTable.Type4+Size, BigSize - Size);
	
	// we make SMBios v2.6 while Apple needs 2.5
	newSmbiosTable.Type4->Hdr.Length = newSize;
	gBusSpeed = newSmbiosTable.Type4->ExternalClock;
	//this hack is related to different using of these fields by PC BIOS and Apple
	newSmbiosTable.Type4->MaxSpeed = newSmbiosTable.Type4->CurrentSpeed;
	
	if (Size <= 0x23) {  //Smbios <=2.3
		newSmbiosTable.Type4->CoreCount = cpuid_info()->core_count;
		newSmbiosTable.Type4->ThreadCount = cpuid_info()->thread_count;
	} //else we propose DMI data is better then cpuid().
	if (newSmbiosTable.Type4->CoreCount < newSmbiosTable.Type4->EnabledCoreCount) {
		newSmbiosTable.Type4->EnabledCoreCount = cpuid_info()->core_count;
	}
	UINT8 model = cpuid_info()->cpuid_extmodel;
	if (model == CPU_MODEL_YONAH) {
		//this is change in Smbios v2.6 vs v2.3
		newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCoreSolo;
	}
	newSmbiosTable.Type4->L1CacheHandle = mHandleL1;
	newSmbiosTable.Type4->L2CacheHandle = mHandleL2;
	newSmbiosTable.Type4->L3CacheHandle = mHandleL3;
	if (AddBrand) {
		newSmbiosTable.Type4->ProcessorVersion = 3; //ugly
		//UpdateString is not working, do this manually
		UINT8* p = (UINT8*)newSmbiosTable.Type4 + newSize;
		while ((*p++ != 0) || (*p != 0)) {}
		for (Size = 48; Size>0; Size--) {
			if ((BrandStr[Size] !=0) && (BrandStr[Size] != 32)) {
				break;
			}
		}
		CopyMem(p, BrandStr, Size);
	}
	/*
	 ProcessorCharacteristics ???
	 bit2 = cpuid_info()->cpuid_extfeatures & CPUID_EXTFEATURE_EM64T
	 bit3 = core_count > 1
	 bit4 = thread_count > 1 or CPUID_FEATURE_HTT
	 bit5 = CPUID_EXTFEATURE_XD 
	 bit6 = CPUID_FEATURE_VMX
	 bit7 = CPUID_FEATURE_EST
	 */
	//
	// Log Smbios Record Type4
	//
	Handle = LogSmbiosData(gSmbios,(UINT8*)newSmbiosTable.Type4);
	//then change strings
	StringNumber = newSmbiosTable.Type4->Socket;
	gSmbios->UpdateString(gSmbios, &Handle, &StringNumber, Socket); 
	StringNumber = newSmbiosTable.Type4->ProcessorManufacture;
	gSmbios->UpdateString(gSmbios, &Handle, &StringNumber, Manuf); 
	StringNumber = newSmbiosTable.Type4->ProcessorVersion;	
	if(BrandStr[0]){
//		gSmbios->UpdateString(gSmbios, &Handle, &StringNumber, Ver); 
//	} else {
		gSmbios->UpdateString(gSmbios, &Handle, &StringNumber, BrandStr);
	}

		StringNumber = newSmbiosTable.Type4->SerialNumber;
		gSmbios->UpdateString(gSmbios, &Handle, &StringNumber, SN); 
		StringNumber = newSmbiosTable.Type4->AssetTag;
		gSmbios->UpdateString(gSmbios, &Handle, &StringNumber, Asset); 
		StringNumber = newSmbiosTable.Type4->PartNumber;
		gSmbios->UpdateString(gSmbios, &Handle, &StringNumber, Part); 	
	//
	// Set ProcessorVersion string
	//
	AString = GetSmbiosString (newSmbiosTable, newSmbiosTable.Type4->ProcessorVersion);
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
	EFI_SMBIOS_HANDLE				Handle;
	UINT16							Level = 0;
	//
	// Cache info (TYPE 7)
	// 
	mHandleL1 = 0xFFFE;
	mHandleL2 = 0xFFFE;
	mHandleL3 = 0xFFFE;
	//Slice - How many caches do we have ?
	for (i=0; i<4; i++) {
		SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 7, i);
		if (SmbiosTable.Raw == NULL) {
			return ;
		}	
		Level = SmbiosTable.Type7->CacheConfiguration & 3;
		// Log Smbios Record Type7
		//
		Handle = LogSmbiosData(gSmbios,(UINT8*)SmbiosTable.Type7);	
		switch (Level) {
			case 0:
				mHandleL1 = Handle;
				break;
			case 1:
				mHandleL2 = Handle;
				break;
			case 2:
				mHandleL3 = Handle;
				break;
			default:
				break;
		}
		//Here it will be good to correct socket designation
		//but it doesn't work because of quirky protocol
		CHAR8* SSocketD = "L1-Cache";
		UINTN StringNumber = 1;
		 if(SmbiosTable.Type7->SocketDesignation == 0) {
			 
			 SmbiosTable.Type7->SocketDesignation = 1;
			 SSocketD[1] += Level;
			 gSmbios->UpdateString(gSmbios,
								   &Handle,
								   &StringNumber,
								   SSocketD); 
			 
		 }
		 
	}
	return ;
}

VOID
InstallBaseBoardSmbios			( //2
IN VOID                  *Smbios
								 )
{
	SMBIOS_STRUCTURE_POINTER          SmbiosTable;
	SMBIOS_STRUCTURE_POINTER          newSmbiosTable;
	UINTN							BigSize;
	EFI_SMBIOS_HANDLE				Handle;
	UINTN StringNumber = 0;
	//
	// BaseBoard (TYPE 2)
	// 
	SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 2, 0);
	if (SmbiosTable.Raw == NULL) {
		return ;
	}
	BigSize = SmbiosTableLength(SmbiosTable);
	newSmbiosTable = (SMBIOS_STRUCTURE_POINTER)(SMBIOS_TABLE_TYPE2*)AllocateZeroPool(BigSize);
	CopyMem((VOID*)newSmbiosTable.Type2, (VOID*)SmbiosTable.Type2, BigSize);
	newSmbiosTable.Type2->ChassisHandle = mHandle3;
	Handle = LogSmbiosData(gSmbios,(UINT8*)newSmbiosTable.Type2);
	
	StringNumber = newSmbiosTable.Type2->Manufacturer;
	gSmbios->UpdateString(gSmbios,
						  &Handle,
						  &StringNumber,
						  SMboardmanufacter); 
	//CHAR8* SMboardproduct[8] = { //t2 ProductName
	StringNumber = newSmbiosTable.Type2->ProductName;
	gSmbios->UpdateString(gSmbios,
						  &Handle,
						  &StringNumber,
						  SMboardproduct[gMacType]); 
	//SMserial = "W87234JHYA4";  //t1,t2 SerialNumber,
	StringNumber = newSmbiosTable.Type2->SerialNumber;
	gSmbios->UpdateString(gSmbios,
						  &Handle,
						  &StringNumber,
						  SMserials[gMacType]); 
	StringNumber = newSmbiosTable.Type2->Version;
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
	SMBIOS_STRUCTURE_POINTER          newSmbiosTable;
	UINTN							BigSize;
//	EFI_SMBIOS_HANDLE				Handle;
	UINTN							StringNumber = 0;
	gMobile = FALSE;
	//
	// SystemEnclosure (TYPE 3)
	// 
	SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 3, 0);
	if (SmbiosTable.Raw == NULL) {
		//    DEBUG ((EFI_D_ERROR, "SmbiosTable: Type 3 (SystemEnclosure) not found!\n"));
		return ;
	}
	BigSize = SmbiosTableLength(SmbiosTable);
	newSmbiosTable = (SMBIOS_STRUCTURE_POINTER)(SMBIOS_TABLE_TYPE3*)AllocateZeroPool(BigSize);
	CopyMem((VOID*)newSmbiosTable.Type3, (VOID*)SmbiosTable.Type3, BigSize);
	
	
	gMobile = ((newSmbiosTable.Type3->Type) >= 8);
	//
	// Log Smbios Record Type3
	//
	mHandle3 = LogSmbiosData(gSmbios,(UINT8*)SmbiosTable.Type3);
	StringNumber = newSmbiosTable.Type3->Manufacturer;
	gSmbios->UpdateString(gSmbios,
						  &mHandle3,
						  &StringNumber,
						  SMboardmanufacter); 
	StringNumber = newSmbiosTable.Type3->Version;
	gSmbios->UpdateString(gSmbios,
						  &mHandle3,
						  &StringNumber,
						  SMboardproduct[4]); 
	StringNumber = newSmbiosTable.Type3->SerialNumber;
	gSmbios->UpdateString(gSmbios,
						  &mHandle3,
						  &StringNumber,
						  SMserial); 
	
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
		// Log Smbios Record Type9
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
	mTotalSystemMemory = 0;
	mMemCount = 0;
	mHandle16 = 0xFFFE;
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
	mMemCount = SmbiosTable.Type16->NumberOfMemoryDevices;
	mHandle16 = LogSmbiosData(gSmbios,(UINT8*)SmbiosTable.Type16);
	
	return ;
}

VOID									
InstallMemoryDeviceSmbios		(//17
  IN VOID                  *Smbios
  )
{
	SMBIOS_STRUCTURE_POINTER          SmbiosTable;
	SMBIOS_STRUCTURE_POINTER          newSmbiosTable;
	UINTN	i, Size, BigSize;
//	CHAR8	*StrStart;
  //
  // MemoryDevice (TYPE 17)
  // 
	if (!mMemCount) {
		mMemCount = 8;
	}
	for (i=0; i<mMemCount; i++) {
		SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 17, i);
		if (SmbiosTable.Raw == NULL) {
			//    DEBUG ((EFI_D_ERROR, "SmbiosTable: Type 17 (MemoryDevice) not found!\n"));
			continue;
		}
		Size = SmbiosTable.Type17->Hdr.Length;
/*		StrStart = (CHAR8*)SmbiosTable.Type17+Size;
		for (BigSize = Size; !(*StrStart == 0 && *(StrStart + 1) == 0); BigSize++) {
			StrStart++;
		}	
		BigSize++;*/
		BigSize = SmbiosTableLength(SmbiosTable);
		newSmbiosTable = (SMBIOS_STRUCTURE_POINTER)(SMBIOS_TABLE_TYPE17*)AllocateZeroPool(BigSize);
		CopyMem((VOID*)newSmbiosTable.Type17, (VOID*)SmbiosTable.Type17, BigSize);
		newSmbiosTable.Type17->MemoryArrayHandle = mHandle16;
		mTotalSystemMemory += SmbiosTable.Type17->Size;
		//
		// Log Smbios Record Type17
		//
		mHandle17[i] = LogSmbiosData(gSmbios,(UINT8*)newSmbiosTable.Type17);
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
	UINT32						UpAddress;
	//
	// FirmwareVolume (TYPE 128)
	// 
	SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 128, 0);
	if (SmbiosTable.Raw == NULL) {
		//    DEBUG ((EFI_D_ERROR, "SmbiosTable: Type 128 (FirmwareVolume) not found!\n"));

		SmbiosTable = (SMBIOS_STRUCTURE_POINTER)(SMBIOS_TABLE_TYPE128*)AllocateZeroPool(sizeof(SMBIOS_TABLE_TYPE128));
		SmbiosTable.Type128->Hdr.Type = 128;
		SmbiosTable.Type128->Hdr.Length = sizeof(SMBIOS_TABLE_TYPE128)+2; 
		SmbiosTable.Type128->FirmwareFeatures = 0x80000015;
		SmbiosTable.Type128->FirmwareFeaturesMask = 0x800003ff;
		/*
		 FW_REGION_RESERVED   = 0,
		 FW_REGION_RECOVERY   = 1,
		 FW_REGION_MAIN       = 2, gHob->MemoryAbove1MB.PhysicalStart + ResourceLength
		    or fix as 0x200000 - 0x600000
		 FW_REGION_NVRAM      = 3,
		 FW_REGION_CONFIG     = 4,
		 FW_REGION_DIAGVAULT  = 5,		 
		 */
		SmbiosTable.Type128->RegionCount = 1; 
		SmbiosTable.Type128->RegionType[0] = FW_REGION_MAIN; //the only needed
		UpAddress = mTotalSystemMemory << 20; //Mb -> b
		SmbiosTable.Type128->FlashMap[0].StartAddress = 0; //0xF0000;
//			gHob->MemoryAbove1MB.PhysicalStart;
		SmbiosTable.Type128->FlashMap[0].EndAddress = 0x17FFFF;
//			gHob->MemoryAbove1MB.PhysicalStart + gHob->MemoryAbove1MB.ResourceLength - 1;
//		return ;
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
	// TODO: LocateProtocol(Smbus) and read SPD. Addresses are known from Chameleon sources. 
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
	UINT16 res = (gBusSpeed % 10) / 3;
	SmbiosTable.Type132->ProcessorBusSpeed = (gBusSpeed << 2) + res;
	//
	// Log Smbios Record Type132
	//
	LogSmbiosData(gSmbios,(UINT8*)SmbiosTable.Type132);	
	return ;
}
										

VOID
InstallMemorySmbios (  //19
  IN VOID                  *Smbios
  )
{
	SMBIOS_STRUCTURE_POINTER          SmbiosTable;
	SMBIOS_STRUCTURE_POINTER          newSmbiosTable;
	UINTN	i;
	//
	// Generate Memory Array Mapped Address info (TYPE 19)
	//
	/*
	 /// This structure provides the address mapping for a Physical Memory Array.  
	 /// One structure is present for each contiguous address range described.
	 ///
	 typedef struct {
		SMBIOS_STRUCTURE      Hdr;
		UINT32                StartingAddress;
		UINT32                EndingAddress;
		UINT16                MemoryArrayHandle;
		UINT8                 PartitionWidth;
	 } SMBIOS_TABLE_TYPE19;
	 
	 */
	if (!mMemCount) {
		mMemCount = 8;
	}
/*	
	for (i=0; i<mMemCount+1; i++) {
		SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 19, i);
		if (SmbiosTable.Raw == NULL) {			
			return ;
		}	
		newSmbiosTable = (SMBIOS_STRUCTURE_POINTER)(SMBIOS_TABLE_TYPE19*)AllocateZeroPool(sizeof(SMBIOS_TABLE_TYPE19));
		CopyMem((VOID*)newSmbiosTable.Type19, (VOID*)SmbiosTable.Type19, sizeof(SMBIOS_TABLE_TYPE19));

		newSmbiosTable.Type19->MemoryArrayHandle = mHandle16;
		if (!SmbiosTable.Type19->EndingAddress)
			newSmbiosTable.Type19->EndingAddress = (mTotalSystemMemory << 10) - 1; // Mb -> kb
		//
		// Record Smbios Type 19
		//
		LogSmbiosData(gSmbios, (UINT8*)newSmbiosTable.Type19);		
	}
 */
	UINT32	TotalEnd = 0; 
	UINT8	PartWidth = 1;
	for (i=0; i<mMemCount+1; i++) {
		SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, 19, i);
		if (SmbiosTable.Raw == NULL) {			
			break;
		}
		if (SmbiosTable.Type19->EndingAddress > TotalEnd) {
			TotalEnd = SmbiosTable.Type19->EndingAddress;
		}
		PartWidth = SmbiosTable.Type19->PartitionWidth;
	}
	if (TotalEnd == 0) {
		TotalEnd = (mTotalSystemMemory << 10) - 1;
	}
	newSmbiosTable = (SMBIOS_STRUCTURE_POINTER)(SMBIOS_TABLE_TYPE19*)AllocateZeroPool(sizeof(SMBIOS_TABLE_TYPE19));
	newSmbiosTable.Type19->Hdr.Type = 19;
	newSmbiosTable.Type19->Hdr.Length = sizeof(SMBIOS_TABLE_TYPE19); 
	newSmbiosTable.Type19->MemoryArrayHandle = mHandle16;
	newSmbiosTable.Type19->StartingAddress = 0;
	newSmbiosTable.Type19->EndingAddress = TotalEnd;
	newSmbiosTable.Type19->PartitionWidth = PartWidth;
	LogSmbiosData(gSmbios, (UINT8*)newSmbiosTable.Type19);
	return ;
}

VOID
InstallMemoryMapSmbios (  //20
					 IN VOID          *Smbios
					 )
{
	SMBIOS_STRUCTURE_POINTER          SmbiosTable;
	UINTN	i;
	//
	// Generate Memory Array Mapped Address info (TYPE 20)
	// not needed neither for Apple nor for EFI
	// so why I didn't correct wrong handles
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
				gMacType = MacBook11;
				gCpuType = 0x101;
				break;				
			case CPU_MODEL_YONAH: 
				gMacType = MacBook11; //MacBook11; //Probe to install Lion
				gCpuType = 0x201;  // or 0x101???
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
		switch (cpuid_info()->cpuid_model) {
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
		gCpuType = (cpuid_cpu_info.core_count >= 4)?0x701:0x601;
	}

	InstallCacheSmbios     (Smbios); //kSMBTypeCacheInformation=7 	
	InstallProcessorSmbios (Smbios); //kSMBTypeProcessorInformation=4

	InstallMiscSmbios      (Smbios); //kSMBTypeSystemInformation=1 kSMBTypeBIOSInformation=0
	InstallBaseBoardSmbios			(Smbios); //2

	InstallMemoryModuleSmbios		(Smbios); //6
	InstallSystemSlotSmbios			(Smbios); //9
	InstallPhysicalMemoryArraySmbios(Smbios); //16
	InstallMemoryDeviceSmbios		(Smbios); //17
	InstallMemorySmbios    (Smbios); //Memory Array Mapped Address info (TYPE 19) unknown to Apple
	InstallMemoryMapSmbios (Smbios); //Memory Device Mapped info (TYPE 20) unknown to Apple
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
  @return Handle to new table

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
