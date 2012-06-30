/**
 smbios.c
 original idea of SMBIOS patching by mackerintel
 implementation for UEFI smbios table patching
  Slice 2011. 
 
 portion copyright Intel
 Copyright (c) 2009 - 2010, Intel Corporation. All rights reserved.<BR>
 This program and the accompanying materials                          
 are licensed and made available under the terms and conditions of the BSD License         
 which accompanies this distribution.  The full text of the license may be found at        
 http://opensource.org/licenses/bsd-license.php                                            
 
 THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
 WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
 
 Module Name:
 
 SmbiosGen.c
 **/

#include "Platform.h"

#define DEBUG_SMBIOS 1

#if DEBUG_SMBIOS == 2
#define DBG(x...) AsciiPrint(x)
#elif DEBUG_SMBIOS == 1
#define DBG(x...) MsgLog(x)
#else
#define DBG(x...)
#endif

#define _Bit(n)			(1ULL << n)
#define _HBit(n)		(1ULL << ((n)+32))

#define CPUID_EXTFEATURE_EM64T		_Bit(29)
#define CPUID_EXTFEATURE_XD			_Bit(20)
#define CPUID_FEATURE_VMX			_HBit(5)
#define CPUID_FEATURE_EST			_HBit(7)		

EFI_GUID						gUuid;
EFI_GUID						*gTableGuidArray[] = {&gEfiSmbiosTableGuid};

//EFI_PHYSICAL_ADDRESS			*smbiosTable;
VOID                        *Smbios;  //pointer to SMBIOS data
SMBIOS_TABLE_ENTRY_POINT		*EntryPoint; //SmbiosEps original
SMBIOS_TABLE_ENTRY_POINT		*SmbiosEpsNew; //new SmbiosEps
//for patching
SMBIOS_STRUCTURE_POINTER		SmbiosTable;
SMBIOS_STRUCTURE_POINTER		newSmbiosTable;
UINT16                      NumberOfRecords;
UINT16                      MaxStructureSize;
UINT8*                      Current; //pointer to the current end of tables
EFI_SMBIOS_TABLE_HEADER			*Record;
EFI_SMBIOS_HANDLE           Handle;
EFI_SMBIOS_TYPE             Type;

UINTN				stringNumber;
UINTN				TableSize;
UINTN				Index, Size, NewSize, MaxSize;
UINT16			CoreCache = 0;
UINT16			TotalCount = 0;
UINT16			L1, L2, L3;
UINT32			MaxMemory = 0;
UINT32			mTotalSystemMemory;
UINT64      gTotalMemory;
UINT16			mHandle3;
UINT16			mHandle16 = 0xFFFE;;
UINT16			mHandle17[MAX_SLOT_COUNT];
UINT16			mHandle19;
UINT16			mMemory17[MAX_SLOT_COUNT];
UINT64			mInstalled[MAX_SLOT_COUNT];
UINT64			mEnabled[MAX_SLOT_COUNT];
BOOLEAN			gMobile;
UINT8       gBootStatus;
BOOLEAN			Once;

MEM_STRUCTURE*		gRAM;
DMI*							gDMI;


#define MAX_HANDLE        0xFEFF
#define SMBIOS_PTR        SIGNATURE_32('_','S','M','_')
#define MAX_TABLE_SIZE    512


/* Functions */
VOID* FindOemSMBIOSPtr (VOID)
{
	UINTN      Address;	

	// Search 0x0f0000 - 0x0fffff for SMBIOS Ptr
	for (Address = 0xf0000; Address < 0xfffff; Address += 0x10) {
		if (*(UINT32 *)(Address) == SMBIOS_PTR) {
			return (VOID *)Address;
		}
	}
	return NULL;
}

UINTN iStrLen(CHAR8* String, UINTN MaxLen)
{
	UINTN	Len = 0;
	CHAR8*	BA;
	if(MaxLen > 0){
		for (Len=0; Len<MaxLen; Len++) {
			if (String[Len] == 0) {
				break;
			}
		}
		BA = &String[Len - 1];
		while ((Len != 0) && ((*BA == ' ') || (*BA == 0))) {
			BA--; Len--;
		}
	} else {
		BA = String;
		while(*BA){BA++; Len++;}
	}
	return Len;
}

VOID* GetSmbiosTablesFromHob (VOID)
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

VOID *
GetSmbiosTablesFromConfigTables (VOID)
{
	EFI_STATUS              Status;
	EFI_PHYSICAL_ADDRESS    *Table;
	
	Status = EfiGetSystemConfigurationTable (&gEfiSmbiosTableGuid, (VOID **)  &Table);
	if (EFI_ERROR (Status) || Table == NULL) {
		Table = NULL;
	}
	
	return Table;
}


// Internal functions for flat SMBIOS

UINT16 SmbiosTableLength (SMBIOS_STRUCTURE_POINTER SmbiosTable)
{
	CHAR8  *AChar;
	UINT16  Length;

	AChar = (CHAR8 *)(SmbiosTable.Raw + SmbiosTable.Hdr->Length);
	while ((*AChar != 0) || (*(AChar + 1) != 0)) {
		AChar ++; //stop at 00 - first 0
	}
	Length = (UINT16)((UINTN)AChar - (UINTN)SmbiosTable.Raw + 2); //length includes 00
	return Length;
}


EFI_SMBIOS_HANDLE LogSmbiosTable (SMBIOS_STRUCTURE_POINTER SmbiosTable)
{
	UINT16  Length = SmbiosTableLength(SmbiosTable);
	if (Length > MaxStructureSize) {
		MaxStructureSize = Length;
	}
	CopyMem(Current, SmbiosTable.Raw, Length);
	Current += Length;
	NumberOfRecords++;
	return SmbiosTable.Hdr->Handle;
}

EFI_STATUS UpdateSmbiosString (SMBIOS_STRUCTURE_POINTER SmbiosTable, SMBIOS_TABLE_STRING* Field, CHAR8* Buffer)
{
	CHAR8*	AString;
	CHAR8*	C1; //pointers for copy
	CHAR8*	C2;
	UINTN	Length = SmbiosTableLength(SmbiosTable);
	UINTN	ALength, BLength;
	UINT8	Index = 1;

	if ((SmbiosTable.Raw == NULL) || !Buffer || !Field) {
		return EFI_NOT_FOUND;
	}
/*
	if (Once) {
		DBG("Raw table data:\n");
		for (ALength=0; ALength<Length; ALength++){
			if((ALength & 0xF) == 0) DBG("\n");
			DBG("%02x ", SmbiosTable.Raw[ALength]);		
		}
		DBG("\n");
		Once = FALSE;
	}
*/
	AString = (CHAR8*)(SmbiosTable.Raw + SmbiosTable.Hdr->Length); //first string
	while (Index != *Field) {
		if (*AString) {
			Index++;
		}
		while (*AString != 0) AString++; //skip string at index
		AString++; //next string
		if (*AString == 0) {
			//this is end of the table
			if (*Field == 0) {
				AString[1] = 0; //one more zero
			}
			*Field = Index; //index of the next string that  is empty
			if (Index == 1) {
				AString--; //first string has no leading zero
			}
			break;
		}		
	}
	// AString is at place to copy
	ALength = iStrLen(AString, 0);
	BLength = iStrLen(Buffer, SMBIOS_STRING_MAX_LENGTH);
//	DBG("Table type %d field %d\n", SmbiosTable.Hdr->Type, *Field);
//	DBG("Old string length=%d new length=%d\n", ALength, BLength);
	if (BLength > ALength) {
		//Shift right
		C1 = (CHAR8*)SmbiosTable.Raw + Length; //old end
		C2 = C1  + BLength - ALength; //new end
		*C2 = 0;
		while (C1 != AString) *(--C2) = *(--C1);

	} else if (BLength < ALength) {
		//Shift left
		C1 = AString + ALength; //old start
		C2 = AString + BLength; //new start
		while (C1 != ((CHAR8*)SmbiosTable.Raw + Length)) {
			*C2++ = *C1++;
		}
		*C2 = 0;
		*(--C2) = 0; //end of table
	}
	CopyMem(AString, Buffer, BLength);
	*(AString + BLength) = 0; // not sure there is 0	

#if 0 // DEBUG_SMBIOS != 0
	DBG("Old table length=%d, calculated new=%d\n", Length, Length+BLength-ALength);
	Length = SmbiosTableLength(SmbiosTable);
	DBG("New table length=%d\n", Length);
	C1 = (CHAR8*)(SmbiosTable.Raw + SmbiosTable.Hdr->Length);
	while (*C1 != 0) {
		while (*C1 != 0) {
			DBG("%c", *C1++); 
		}
		DBG("\n");
		C1++; //next string
		if (*C1 == 0) { //end of table
			break;
		}
	}
	DBG("-----------------\n");
#endif

	return EFI_SUCCESS;
}

SMBIOS_STRUCTURE_POINTER GetSmbiosTableFromType (
	SMBIOS_TABLE_ENTRY_POINT *Smbios, UINT8 Type, UINTN Index)
{
	SMBIOS_STRUCTURE_POINTER SmbiosTable;
	UINTN                    SmbiosTypeIndex;

	SmbiosTypeIndex = 0;
	SmbiosTable.Raw = (UINT8 *)(UINTN)Smbios->TableAddress;
	if (SmbiosTable.Raw == NULL) {
		return SmbiosTable;
	}
	while ((SmbiosTypeIndex != Index) || (SmbiosTable.Hdr->Type != Type)) {
		if (SmbiosTable.Hdr->Type == SMBIOS_TYPE_END_OF_TABLE) {
			SmbiosTable.Raw = NULL;
			return SmbiosTable;
		}
		if (SmbiosTable.Hdr->Type == Type) {
			SmbiosTypeIndex++;
		}
		SmbiosTable.Raw = (UINT8 *)(SmbiosTable.Raw + SmbiosTableLength (SmbiosTable));
	}
	return SmbiosTable;
}

CHAR8* GetSmbiosString (
	SMBIOS_STRUCTURE_POINTER SmbiosTable, SMBIOS_TABLE_STRING String)
{
	CHAR8      *AString;
	UINT8      Index;

	Index = 1;
	AString = (CHAR8 *)(SmbiosTable.Raw + SmbiosTable.Hdr->Length); //first string
	while (Index != String) {
		while (*AString != 0) {
			AString ++;
		}
		AString ++; //skip zero ending
		if (*AString == 0) {
			return AString; //this is end of the table
		}
		Index ++;
	}

	return AString; //return pointer to Ascii string
}

VOID AddSmbiosEndOfTable()
{	
	SMBIOS_STRUCTURE* StructurePtr = (SMBIOS_STRUCTURE*)Current;
	StructurePtr->Type		= SMBIOS_TYPE_END_OF_TABLE; 
	StructurePtr->Length	= sizeof(SMBIOS_STRUCTURE);
	StructurePtr->Handle	= SMBIOS_TYPE_INACTIVE; //spec 2.7 p.120
	Current += sizeof(SMBIOS_STRUCTURE);
	*Current++ = 0;
	*Current++ = 0; //double 0 at the end
	NumberOfRecords++;
}

/* Patching Functions */
VOID PatchTableType0()
{
  // BIOS information
	// 
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_BIOS_INFORMATION, 0);
	if (SmbiosTable.Raw == NULL) {
//		Print(L"SmbiosTable: Type 0 (Bios Information) not found!\n");
		
		return;
	}
	TableSize = SmbiosTableLength(SmbiosTable);
	ZeroMem((VOID*)newSmbiosTable.Type0, MAX_TABLE_SIZE);
	CopyMem((VOID*)newSmbiosTable.Type0, (VOID*)SmbiosTable.Type0, TableSize); //can't point to union
	
	newSmbiosTable.Type0->BiosSegment = 0; //like in Mac
	newSmbiosTable.Type0->SystemBiosMajorRelease = 0;
	newSmbiosTable.Type0->SystemBiosMinorRelease = 1;
	newSmbiosTable.Type0->BiosCharacteristics.BiosCharacteristicsNotSupported = 0;
//	newSmbiosTable.Type0->BIOSCharacteristicsExtensionBytes[1] |= 8; //UefiSpecificationSupported;
	Once = TRUE;
	if(iStrLen(gSettings.VendorName, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type0->Vendor, gSettings.VendorName);
	}
	if(iStrLen(gSettings.RomVersion, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type0->BiosVersion, gSettings.RomVersion);
	}
	if(iStrLen(gSettings.ReleaseDate, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type0->BiosReleaseDate, gSettings.ReleaseDate);		
	}
	
	Handle = LogSmbiosTable(newSmbiosTable);
}

VOID GetTableType1()
{
  CHAR8* s;
  // System Information
  //
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_SYSTEM_INFORMATION, 0);
	if (SmbiosTable.Raw == NULL) {
		Print(L"SmbiosTable: Type 1 (System Information) not found!\n");
		return;
	}
		
	CopyMem((VOID*)&gSettings.SmUUID, (VOID*)&SmbiosTable.Type1->Uuid, 16);
	//AsciiStrToUnicodeStr(GetSmbiosString(SmbiosTable, SmbiosTable.Type1->ProductName), gSettings.OEMProduct);
  s = GetSmbiosString(SmbiosTable, SmbiosTable.Type1->ProductName);
  CopyMem(gSettings.OEMProduct, s, iStrLen(s, 64));
	//Check the validity
/*	if ((gUuid.Data3 & 0xF000) == 0) {
		CopyMem(&gUuid, (VOID*)&gEfiSmbiosTableGuid, 16); //gPlatformUuid
		gUuid.Data1 ^= 0x1234; //random 
		gUuid.Data2 ^= 0x2341;
		gUuid.Data3 ^= 0x3412;
	} */
	return;
}


VOID PatchTableType1()
{
  // System Information
  // 
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_SYSTEM_INFORMATION, 0);
	if (SmbiosTable.Raw == NULL) {
		return;
	}
	
	//Increase table size
	Size = SmbiosTable.Type1->Hdr.Length; //old size
	TableSize = SmbiosTableLength(SmbiosTable); //including strings
	NewSize = sizeof(SMBIOS_TABLE_TYPE1);
	ZeroMem((VOID*)newSmbiosTable.Type1, MAX_TABLE_SIZE);
	CopyMem((VOID*)newSmbiosTable.Type1, (VOID*)SmbiosTable.Type1, Size); //copy main table
	CopyMem((CHAR8*)newSmbiosTable.Type1+NewSize, (CHAR8*)SmbiosTable.Type1+Size, TableSize - Size); //copy strings
	newSmbiosTable.Type1->Hdr.Length = NewSize;
	
	newSmbiosTable.Type1->WakeUpType = SystemWakeupTypePowerSwitch;
	Once = TRUE;
	
	if((gSettings.SmUUID.Data3 & 0xF000) != 0) {
		CopyMem((VOID*)&newSmbiosTable.Type1->Uuid, (VOID*)&gSettings.SmUUID, 16);
	}
	
	if(iStrLen(gSettings.ManufactureName, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type1->Manufacturer, gSettings.ManufactureName);
	}
	if(iStrLen(gSettings.ProductName, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type1->ProductName, gSettings.ProductName);
	}
	if(iStrLen( gSettings.VersionNr, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type1->Version, gSettings.VersionNr);
	}
	if(iStrLen(gSettings.SerialNr, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type1->SerialNumber, gSettings.SerialNr);
	}
	if(iStrLen(gSettings.BoardNumber, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type1->SKUNumber, gSettings.BoardNumber);
	}	
	if(iStrLen(gSettings.FamilyName, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type1->Family, gSettings.FamilyName);
	}	
	
	Handle = LogSmbiosTable(newSmbiosTable);
	return;
}

VOID GetTableType2()
{
  CHAR8* s;
  // System Information
  //
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_BASEBOARD_INFORMATION, 0);
	if (SmbiosTable.Raw == NULL) {
		return;
	}
  
  s = GetSmbiosString(SmbiosTable, SmbiosTable.Type2->ProductName);
  CopyMem(gSettings.OEMBoard, s, iStrLen(s, 64));
}


VOID PatchTableType2()
{
  // BaseBoard Information
	// 
	NewSize = 0x0F; //sizeof(SMBIOS_TABLE_TYPE2);
	ZeroMem((VOID*)newSmbiosTable.Type2, MAX_TABLE_SIZE);	
	
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_BASEBOARD_INFORMATION, 0);
	if (SmbiosTable.Raw == NULL) {
		MsgLog("SmbiosTable: Type 2 (BaseBoard Information) not found, create new\n");
		//Create new one
		newSmbiosTable.Type2->Hdr.Type = 2;
		newSmbiosTable.Type2->Hdr.Handle = 0x0200; //common rule
		
	} else {
		Size = SmbiosTable.Type2->Hdr.Length; //old size
		TableSize = SmbiosTableLength(SmbiosTable); //including strings
		
		if (NewSize > Size) {
			CopyMem((VOID*)newSmbiosTable.Type2, (VOID*)SmbiosTable.Type2, Size); //copy main table
			CopyMem((CHAR8*)newSmbiosTable.Type2 + NewSize, (CHAR8*)SmbiosTable.Type2 + Size, TableSize - Size); //copy strings			
		} else {
			CopyMem((VOID*)newSmbiosTable.Type2, (VOID*)SmbiosTable.Type2, TableSize); //copy full table
		}	
	}

	newSmbiosTable.Type2->Hdr.Length = NewSize;
	newSmbiosTable.Type2->ChassisHandle = mHandle3;	//from GetTableType3
	newSmbiosTable.Type2->BoardType = BaseBoardTypeMotherBoard;
	ZeroMem((VOID*)&newSmbiosTable.Type2->FeatureFlag, sizeof(BASE_BOARD_FEATURE_FLAGS));
	newSmbiosTable.Type2->FeatureFlag.Motherboard = 1;
	newSmbiosTable.Type2->FeatureFlag.Replaceable = 1;
	Once = TRUE;
	
	if(iStrLen(gSettings.BoardManufactureName, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type2->Manufacturer, gSettings.BoardManufactureName);
	}
	if(iStrLen(gSettings.BoardNumber, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type2->ProductName, gSettings.BoardNumber);
	}
	if(iStrLen( gSettings.BoardVersion, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type2->Version, gSettings.BoardVersion);
	}	
	if(iStrLen(gSettings.BoardSerialNumber, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type2->SerialNumber, gSettings.BoardSerialNumber);
	}
	if(iStrLen(gSettings.LocationInChassis, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type2->LocationInChassis, gSettings.LocationInChassis);
	}
	//what about Asset Tag??? Not used in real mac. till now.
	
	//Slice - for the table2 one patch more needed
	/* spec
	Field 0x0E - Identifies the number (0 to 255) of Contained Object Handles that follow
	Field 0x0F - A list of handles of other structures (for example, Baseboard, Processor, Port, System Slots, Memory Device) that are contained by this baseboard
	It may be good before our patching but changed after. We should at least check if all tables mentioned here are present in final structure
	 I just set 0 as in iMac11
	*/
	newSmbiosTable.Type2->NumberOfContainedObjectHandles = 0;

	Handle = LogSmbiosTable(newSmbiosTable);
	return;
}

VOID GetTableType3()
{
  // System Chassis Information
  //
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_SYSTEM_ENCLOSURE, 0);
	if (SmbiosTable.Raw == NULL) {
		Print(L"SmbiosTable: Type 3 (System Chassis Information) not found!\n");
		gMobile = FALSE; //default value
		return;
	}
	mHandle3 = SmbiosTable.Type3->Hdr.Handle;
	gMobile = ((SmbiosTable.Type3->Type) >= 8);

	return;
}

VOID PatchTableType3()
{
  // System Chassis Information
  //
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_SYSTEM_ENCLOSURE, 0);
	if (SmbiosTable.Raw == NULL) {
//		Print(L"SmbiosTable: Type 3 (System Chassis Information) not found!\n");
		return;
	}
	Size = SmbiosTable.Type3->Hdr.Length; //old size
	TableSize = SmbiosTableLength(SmbiosTable); //including strings
	NewSize = 0x15; //sizeof(SMBIOS_TABLE_TYPE3);
	ZeroMem((VOID*)newSmbiosTable.Type3, MAX_TABLE_SIZE);
	
	if (NewSize > Size) {
		CopyMem((VOID*)newSmbiosTable.Type3, (VOID*)SmbiosTable.Type3, Size); //copy main table
		CopyMem((CHAR8*)newSmbiosTable.Type3 + NewSize, (CHAR8*)SmbiosTable.Type3 + Size, TableSize - Size); //copy strings
		newSmbiosTable.Type3->Hdr.Length = NewSize;
	} else {
		CopyMem((VOID*)newSmbiosTable.Type3, (VOID*)SmbiosTable.Type3, TableSize); //copy full table
	}
	
	newSmbiosTable.Type3->BootupState = ChassisStateSafe;
	newSmbiosTable.Type3->PowerSupplyState = ChassisStateSafe;
	newSmbiosTable.Type3->ThermalState = ChassisStateOther;
	newSmbiosTable.Type3->SecurityStatus = ChassisSecurityStatusNone;
	newSmbiosTable.Type3->NumberofPowerCords = 1;
	newSmbiosTable.Type3->ContainedElementCount = 0;
	newSmbiosTable.Type3->ContainedElementRecordLength = 0;
	Once = TRUE;
		
	if(iStrLen(gSettings.ChassisManufacturer, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type3->Manufacturer, gSettings.ChassisManufacturer);
	}
//SIC! According to iMac there must be the BoardNumber	
	if(iStrLen(gSettings.BoardNumber, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type3->Version, gSettings.BoardNumber);
	}
	if(iStrLen(gSettings.SerialNr, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type3->SerialNumber, gSettings.SerialNr);
	}
	if(iStrLen(gSettings.ChassisAssetTag, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type3->AssetTag, gSettings.ChassisAssetTag);
	}
	
	Handle = LogSmbiosTable(newSmbiosTable);
	return;
}

VOID GetTableType4()
{
  // Processor Information
  //
  INTN res = 0;
  
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_PROCESSOR_INFORMATION, 0);
	if (SmbiosTable.Raw == NULL) {
		Print(L"SmbiosTable: Type 4 (Processor Information) not found!\n");
		return;
	}
  res = (SmbiosTable.Type4->ExternalClock * 3 + 2) % 100;
  if (res > 2) {
    res = 0;
  } else {
    res = SmbiosTable.Type4->ExternalClock % 10;
  }

	gCPUStructure.ExternalClock = SmbiosTable.Type4->ExternalClock * 1000 + res * 110;//MHz->kHz  
//	UnicodeSPrint(gSettings.BusSpeed, 10, L"%d", gCPUStructure.ExternalClock);
//  gSettings.BusSpeed = gCPUStructure.ExternalClock; //why duplicate??
	gCPUStructure.CurrentSpeed = SmbiosTable.Type4->CurrentSpeed;
  gCPUStructure.MaxSpeed = SmbiosTable.Type4->MaxSpeed;
//	UnicodeSPrint(gSettings.CpuFreqMHz, 10, L"%d", gCPUStructure.CurrentSpeed);
//	gSettings.CpuFreqMHz = gCPUStructure.CurrentSpeed; 
	return;
}

VOID PatchTableType4()
{
  // Processor Information
  //
	UINTN               AddBrand = 0;
	CHAR8               BrandStr[48];
	UINT16							ProcChar = 0;

	//Note. iMac11,2 has four tables for CPU i3
	UINTN		CpuNumber;
	
	CopyMem(BrandStr, gCPUStructure.BrandString, 48);
	BrandStr[47] = '\0';
//	DBG("BrandString=%a\n", BrandStr);
	for (CpuNumber = 0; CpuNumber < gCPUStructure.Cores; CpuNumber++) {
		// Get Table Type4
		SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_PROCESSOR_INFORMATION, CpuNumber);
		if (SmbiosTable.Raw == NULL) {
			break;
		}
		// we make SMBios v2.6 while it may be older so we have to increase size
		Size = SmbiosTable.Type4->Hdr.Length; //old size
		TableSize = SmbiosTableLength(SmbiosTable); //including strings
		AddBrand = 0;
		if (SmbiosTable.Type4->ProcessorVersion == 0) { //if no BrandString we can add
			AddBrand = 48;
		}
		NewSize = sizeof(SMBIOS_TABLE_TYPE4);
		ZeroMem((VOID*)newSmbiosTable.Type4, MAX_TABLE_SIZE);
		CopyMem((VOID*)newSmbiosTable.Type4, (VOID*)SmbiosTable.Type4, Size); //copy main table
		CopyMem((CHAR8*)newSmbiosTable.Type4+NewSize, (CHAR8*)SmbiosTable.Type4+Size, TableSize - Size); //copy strings
		newSmbiosTable.Type4->Hdr.Length = NewSize;

		newSmbiosTable.Type4->MaxSpeed = gCPUStructure.CurrentSpeed;	
		//old version has no such fields. Fill now
		if (Size <= 0x20){
			//sanity check and clear
			newSmbiosTable.Type4->SerialNumber = 0;
			newSmbiosTable.Type4->AssetTag = 0;
			newSmbiosTable.Type4->PartNumber = 0;
		}		
		if (Size <= 0x23) {  //Smbios <=2.3
			newSmbiosTable.Type4->CoreCount = gCPUStructure.Cores;
			newSmbiosTable.Type4->ThreadCount = gCPUStructure.Threads;
			newSmbiosTable.Type4->ProcessorCharacteristics = gCPUStructure.Features;
		} //else we propose DMI data is better then cpuid().
		if (newSmbiosTable.Type4->CoreCount < newSmbiosTable.Type4->EnabledCoreCount) {
			newSmbiosTable.Type4->EnabledCoreCount = gCPUStructure.Cores;
		}
    //some verifications
    if ((newSmbiosTable.Type4->ThreadCount < newSmbiosTable.Type4->CoreCount) ||
        newSmbiosTable.Type4->ThreadCount > newSmbiosTable.Type4->CoreCount * 2) {
      newSmbiosTable.Type4->ThreadCount = gCPUStructure.Threads;
    }
    
		// TODO: Set SmbiosTable.Type4->ProcessorFamily for all implemented CPU models
		Once = TRUE;
		if (gCPUStructure.Model == CPU_MODEL_ATOM) {
			newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelAtom;
		}
		if ((gCPUStructure.Model == CPU_MODEL_DOTHAN) ||
        (gCPUStructure.Model == CPU_MODEL_YONAH)) {
			if (gCPUStructure.Mobile)
			{
				newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCoreSoloMobile;
				if (gCPUStructure.Cores == 2)
				{
					newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCoreDuoMobile;
				}
			} else {
				newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCoreSolo;
				if (gCPUStructure.Cores == 2)
				{
					newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCoreDuo;
				}
			}
		}
		if (gCPUStructure.Model == CPU_MODEL_MEROM) { //and Conroe
			if (gCPUStructure.Mobile)
			{
				if (gCPUStructure.Cores==2)
				{
					newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCore2DuoMobile;
				}
				if (gCPUStructure.Cores==1)
				{
					newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCore2SoloMobile;
				}
			} else {
				if (gCPUStructure.Cores>2)
				{
					newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCore2Quad;
				} else
          if (gCPUStructure.Cores==2)
				{
					newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCore2Extreme;
				}
				if (gCPUStructure.Cores==1)
				{
					newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCore2Solo;
				}
			}
		}
		if (gCPUStructure.Model == CPU_MODEL_PENRYN) {
			if (gCPUStructure.Mobile)
			{
				if (gCPUStructure.Cores>2)
				{
					newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCore2ExtremeMobile;
				}
				if (gCPUStructure.Cores==2)
				{
					newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCore2DuoMobile;
				}

			} else {
				if (gCPUStructure.Cores>2)
				{
					newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCore2Quad;
				} else
				if (gCPUStructure.Cores==2)
				{
					newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCore2Extreme;
				} else
					newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCore2Solo;
			}
  		 }
     if (gCPUStructure.Model >= CPU_MODEL_NEHALEM) {
       if (AsciiStrStr(gCPUStructure.BrandString, "i3"))
     	  newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCoreI3;
       if (AsciiStrStr(gCPUStructure.BrandString, "i5"))
     	  newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCoreI5;
       if (AsciiStrStr(gCPUStructure.BrandString, "i7"))
         newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCoreI7;
	  }
		//spec 2.7 page 48 note 3
		if ((newSmbiosTable.Type4->ProcessorFamily == ProcessorFamilyIntelCore2)
			&& gCPUStructure.Mobile) {
				newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCore2DuoMobile;
		}

		// Set CPU Attributes
		newSmbiosTable.Type4->L1CacheHandle = L1;
		newSmbiosTable.Type4->L2CacheHandle = L2;
		newSmbiosTable.Type4->L3CacheHandle = L3;
		newSmbiosTable.Type4->ProcessorType = CentralProcessor;
		newSmbiosTable.Type4->ProcessorId.Signature.ProcessorSteppingId = gCPUStructure.Stepping;
		newSmbiosTable.Type4->ProcessorId.Signature.ProcessorModel		= (gCPUStructure.Model & 0xF);
		newSmbiosTable.Type4->ProcessorId.Signature.ProcessorFamily		= gCPUStructure.Family;
		newSmbiosTable.Type4->ProcessorId.Signature.ProcessorType		= gCPUStructure.Type;
		newSmbiosTable.Type4->ProcessorId.Signature.ProcessorXModel		= gCPUStructure.Extmodel;
		newSmbiosTable.Type4->ProcessorId.Signature.ProcessorXFamily	= gCPUStructure.Extfamily;
//    CopyMem((VOID*)&newSmbiosTable.Type4->ProcessorId.FeatureFlags, (VOID*)&gCPUStructure.Features, 4);
//		newSmbiosTable.Type4->ProcessorId.FeatureFlags = (PROCESSOR_FEATURE_FLAGS)(UINT32)gCPUStructure.Features;
		if (Size <= 0x26) {
			newSmbiosTable.Type4->ProcessorFamily2 = newSmbiosTable.Type4->ProcessorFamily;
			ProcChar |= (gCPUStructure.ExtFeatures & CPUID_EXTFEATURE_EM64T)?0x04:0;
			ProcChar |= (gCPUStructure.Cores > 1)?0x08:0;
			ProcChar |= (gCPUStructure.Cores < gCPUStructure.Threads)?0x10:0;
			ProcChar |= (gCPUStructure.ExtFeatures & CPUID_EXTFEATURE_XD)?0x20:0;
			ProcChar |= (gCPUStructure.Features & CPUID_FEATURE_VMX)?0x40:0;
			ProcChar |= (gCPUStructure.Features & CPUID_FEATURE_EST)?0x80:0;
			newSmbiosTable.Type4->ProcessorCharacteristics = ProcChar;
		}
		
		if (AddBrand) {
			UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type4->ProcessorVersion, BrandStr);
		}

		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type4->AssetTag, BrandStr); //like mac
		// looks to be MicroCode revision
		if(gCPUStructure.MicroCode > 0){
      AsciiSPrint(BrandStr, 20, "%X", gCPUStructure.MicroCode);
			UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type4->SerialNumber, BrandStr);
		}
		
		Handle = LogSmbiosTable(newSmbiosTable);
	}	
	return;
}

VOID PatchTableType6()
{
  UINT8 SizeField = 0;
	//
	// MemoryModule (TYPE 6)

	// This table is obsolete accoding to Spec but Apple still using it so
	// copy existing table if found, no patches will be here
	// we can have more then 1 module.
	for (Index = 0; Index < MAX_SLOT_COUNT; Index++) {
		SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_MEMORY_MODULE_INFORMATON,Index);
		if (SmbiosTable.Raw == NULL) {
//			MsgLog("SMBIOS Table 6 index %d not found\n", Index);
			continue;
		}
    SizeField = SmbiosTable.Type6->InstalledSize.InstalledOrEnabledSize & 0x7F;
    if (SizeField < 0x7D) {
      mInstalled[Index]	=  (1ULL << SizeField) * (1024 * 1024);
    } else if (SizeField == 0x7F) {
      mInstalled[Index]	= 0;
    } else
    		mInstalled[Index]	=  4096ULL * (1024ULL * 1024ULL);
		MsgLog("Table 6 MEMORY_MODULE %d Installed %x ", Index, mInstalled[Index]);
    if (SizeField >= 0x7D) {
      mEnabled[Index]		= 0;
    } else
      mEnabled[Index]		= (1ULL << ((UINT8)SmbiosTable.Type6->EnabledSize.InstalledOrEnabledSize & 0x7F)) * (1024 * 1024);
		MsgLog("... enabled %x \n", mEnabled[Index]);
		LogSmbiosTable(SmbiosTable);		
	}

	return;
}

VOID PatchTableType7()
{
  // Cache Information 
  //
    //TODO - should be separate table for each CPU core
    //new handle for each core and attach Type4 tables for individual Type7
    // Handle = 0x0700 + CoreN<<2 + CacheN (4-level cache is supported
    // L1[CoreN] = Handle
  
	CHAR8* SSocketD;
	BOOLEAN correctSD = FALSE;
	
	//according to spec for Smbios v2.0 max handle is 0xFFFE, for v>2.0 (we made 2.6) max handle=0xFEFF.
	// Value 0xFFFF means no cache
	L1 = 0xFFFF; // L1 Cache
	L2 = 0xFFFF; // L2 Cache
	L3 = 0xFFFF; // L3 Cache

	// Get Table Type7 and set CPU Caches
	for (Index = 0; Index < MAX_CACHE_COUNT; Index++) {
		SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_CACHE_INFORMATION, Index);
		if (SmbiosTable.Raw == NULL) {
			break;
		}
		TableSize = SmbiosTableLength(SmbiosTable);
		ZeroMem((VOID*)newSmbiosTable.Type7, MAX_TABLE_SIZE);
		CopyMem((VOID*)newSmbiosTable.Type7, (VOID*)SmbiosTable.Type7, TableSize);
		correctSD = (newSmbiosTable.Type7->SocketDesignation == 0);
		CoreCache = newSmbiosTable.Type7->CacheConfiguration & 3; 
		Once = TRUE;

		SSocketD = "L1-Cache";
		if(correctSD) {
			SSocketD[1] = (CHAR8)(0x31 + CoreCache);
			UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type7->SocketDesignation, SSocketD);		
		}
		Handle = LogSmbiosTable(newSmbiosTable);		
		switch (CoreCache) {
			case 0:
				L1 = Handle;
				break;
			case 1:
				L2 = Handle;
				break;
			case 2:
				L3 = Handle;
				break;
			default:
				break;
		}		
	}

	return;
}

VOID PatchTableType9()
{
	//
	// System Slots (Type 9)
	// Warning!!! Tables type9 are used by AppleSMBIOS!
	// if we understand some patch needed we should apply it here 	
	/*
	 SlotDesignation: PCI1
	 System Slot Type: PCI
	 System Slot Data Bus Width:  32 bit
	 System Slot Current Usage:  Available
	 System Slot Length:  Short length
	 System Slot Type: PCI
	 Slot Id: the value present in the Slot Number field of the PCI Interrupt Routing table entry that is associated with this slot is: 1
	 Slot characteristics 1:  Provides 3.3 Volts |  Slot's opening is shared with another slot, e.g. PCI/EISA shared slot. | 
	 Slot characteristics 2:  PCI slot supports Power Management Enable (PME#) signal | 
	 SegmentGroupNum: 0x4350
	 BusNum: 0x49
	 DevFuncNum: 0x31

	 Real Mac always contain Airport table 9 as
	 09 0D xx xx 01 A5 08 03 03 00 00 04 06 "AirPort"
	 */
	CHAR8* AirPort = "AirPort";
	
	for (Index = 0; Index < 64; Index++) { 
		SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_SYSTEM_SLOTS,Index);
		if (SmbiosTable.Raw == NULL) {
			if (Arpt.Valid) {
				ZeroMem((VOID*)newSmbiosTable.Type9, MAX_TABLE_SIZE);
				newSmbiosTable.Type9->Hdr.Type = EFI_SMBIOS_TYPE_SYSTEM_SLOTS;
				newSmbiosTable.Type9->Hdr.Length = sizeof(SMBIOS_TABLE_TYPE9);
				newSmbiosTable.Type9->Hdr.Handle = 0x0900 + Index;
				newSmbiosTable.Type9->SlotDesignation = 1;
				newSmbiosTable.Type9->SlotType = SlotTypePciExpress;
				newSmbiosTable.Type9->SlotDataBusWidth = SlotDataBusWidth1X;
				newSmbiosTable.Type9->CurrentUsage = SlotUsageAvailable;
				newSmbiosTable.Type9->SlotLength = SlotLengthShort;
				newSmbiosTable.Type9->SlotID = 0;
				newSmbiosTable.Type9->MISC_SLOT_CHARACTERISTICS1 = 0x04;
				newSmbiosTable.Type9->MISC_SLOT_CHARACTERISTICS2 = 0x06;
				// take this from PCI bus for WiFi card
				newSmbiosTable.Type9->SegmentGroupNum = Arpt.SegmentGroupNum;
				newSmbiosTable.Type9->BusNum = Arpt.BusNum;
				newSmbiosTable.Type9->DevFuncNum = Arpt.DevFuncNum;
				//
				UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type9->SlotDesignation, AirPort);
				LogSmbiosTable(newSmbiosTable);
			}
			break;
		}
		LogSmbiosTable(SmbiosTable);		
	}
	
	return;
}

VOID PatchTableType11()
{
	CHAR8		*OEMString = "Apple inc. uses Clover"; //something else here?
  // System Information
  // 
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_OEM_STRINGS, 0);
	if (SmbiosTable.Raw != NULL) {
		MsgLog("Table 11 present, but rewritten for us\n");
	}
//	TableSize = SmbiosTableLength(SmbiosTable);
	ZeroMem((VOID*)newSmbiosTable.Type11, MAX_TABLE_SIZE);
//	CopyMem((VOID*)newSmbiosTable.Type11, (VOID*)SmbiosTable.Type11, 5); //minimum, other bytes = 0
	newSmbiosTable.Type11->Hdr.Type = EFI_SMBIOS_TYPE_OEM_STRINGS;
	newSmbiosTable.Type11->Hdr.Length = sizeof(SMBIOS_STRUCTURE)+2; 
	newSmbiosTable.Type11->Hdr.Handle = 0x0B00; //common rule
	
	newSmbiosTable.Type11->StringCount = 1;
	UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type11->StringCount, OEMString);
	
	LogSmbiosTable(newSmbiosTable);
	return;
}	
		  
VOID PatchTableTypeSome()
{
	//some unused but interesting tables. Just log as is
#define NUM_OTHER_TYPES 11	
	UINT8 tableTypes[NUM_OTHER_TYPES] = {8, 10, 18, 21, 22, 27, 28, 32, 33, /*129,*/ 217, 219};
	UINTN	IndexType;
	//
	// Different types 
	for (IndexType = 0; IndexType < NUM_OTHER_TYPES; IndexType++) {
		for (Index = 0; Index < 32; Index++) {
			SmbiosTable = GetSmbiosTableFromType(EntryPoint, tableTypes[IndexType], Index);
			if (SmbiosTable.Raw == NULL) {
				continue;
			}
			LogSmbiosTable(SmbiosTable);		
		}
	}

	return;
}

VOID GetTableType16()
{
  // Physical Memory Array
  //
	mTotalSystemMemory = 0; //later we will add to the value, here initialize it
	TotalCount = 0;
	// Get Table Type16 and set Device Count
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_PHYSICAL_MEMORY_ARRAY, 0);
	if (SmbiosTable.Raw == NULL) {
		Print(L"SmbiosTable: Type 16 (Physical Memory Array) not found!\n");
		return;
	}
	TotalCount = SmbiosTable.Type16->NumberOfMemoryDevices;
	if (!TotalCount) {
		TotalCount = MAX_SLOT_COUNT;
	}
	gDMI->MaxMemorySlots = TotalCount;
	DBG("Total Memory Slots Count = %d\n", TotalCount);
	return;
}


VOID PatchTableType16()
{
  // Physical Memory Array
  //

//	mTotalSystemMemory = 0; //later we will add to the value, here initialize it
//	TotalCount = 0;
	mHandle16 = 0xFFFE;
	
	// Get Table Type16 and set Device Count
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_PHYSICAL_MEMORY_ARRAY, 0);
	if (SmbiosTable.Raw == NULL) {
		Print(L"SmbiosTable: Type 16 (Physical Memory Array) not found!\n");
		return;
	}
	TableSize = SmbiosTableLength(SmbiosTable);
	ZeroMem((VOID*)newSmbiosTable.Type16, MAX_TABLE_SIZE);
	CopyMem((VOID*)newSmbiosTable.Type16, (VOID*)SmbiosTable.Type16, TableSize);
//Slice - I am not sure I want these values	
//	newSmbiosTable.Type16->Location = MemoryArrayLocationProprietaryAddonCard;
//	newSmbiosTable.Type16->Use = MemoryArrayUseSystemMemory;
//	newSmbiosTable.Type16->MemoryErrorCorrection = MemoryErrorCorrectionMultiBitEcc;
/*    if (gDMI->DIMM[2] && Index == 1 &&  newSmbiosTable.Type16->NumberOfMemoryDevices == 2)
    {
        gDMI->MaxMemorySlots = gDMI->CntMemorySlots;
        newSmbiosTable.Type16->NumberOfMemoryDevices = gDMI->CntMemorySlots;
    }
	TotalCount = newSmbiosTable.Type16->NumberOfMemoryDevices;
	if (!TotalCount) {
		TotalCount = MAX_SLOT_COUNT;
	} */
	mHandle16 = LogSmbiosTable(newSmbiosTable);
	return;
}

VOID GetTableType17()
{
  // Memory Device
  //
	// Get Table Type17 and count Size
	gDMI->CntMemorySlots = 0;
	gDMI->MemoryModules = 0;
	for (Index = 0; Index < TotalCount; Index++) {
		SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_MEMORY_DEVICE, Index);
		DBG("Type 17 Index = %d\n", Index);
		if (SmbiosTable.Raw == NULL) {
//			Print(L"SmbiosTable: Type 17 (Memory Device number %d) not found!\n", Index);
			continue;
		}
		gDMI->CntMemorySlots++;
		if (SmbiosTable.Type17->Size > 0) {
			gDMI->MemoryModules++;
		}
		if (SmbiosTable.Type17->Speed > 0) {
			gRAM->DIMM[Index].Frequency = SmbiosTable.Type17->Speed;
		}
//		DBG("CntMemorySlots = %d\n", gDMI->CntMemorySlots)
//		DBG("gDMI->MemoryModules = %d\n", gDMI->MemoryModules)
		DBG("SmbiosTable.Type17->Speed = %d\n", SmbiosTable.Type17->Speed);
		DBG("SmbiosTable.Type17->Size = %d\n", SmbiosTable.Type17->Size);
		if ((SmbiosTable.Type17->Size & 0x8000) == 0) {
			mTotalSystemMemory += SmbiosTable.Type17->Size; //Mb
			mMemory17[Index] = SmbiosTable.Type17->Size > 0 ? mTotalSystemMemory : 0;
		}
		DBG("mTotalSystemMemory = %d\n", mTotalSystemMemory);
	}
}
		

VOID PatchTableType17()
{
	CHAR8	deviceLocator[10];
	CHAR8	bankLocator[10];
	
  // Memory Device
  //
	for (Index = 0; Index < TotalCount; Index++) {
		SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_MEMORY_DEVICE, Index);
		//}
		//DBG("SMBIOS Type 17 Index = %d:\n", Index);
		if (SmbiosTable.Raw == NULL) {
//		//	Print(L"SmbiosTable: Type 17 (Memory Device number %d) not found!\n", Index);
			continue;
		}
		TableSize = SmbiosTableLength(SmbiosTable);
		ZeroMem((VOID*)newSmbiosTable.Type17, MAX_TABLE_SIZE);
		CopyMem((VOID*)newSmbiosTable.Type17, (VOID*)SmbiosTable.Type17, TableSize);
		Once = TRUE;		
		newSmbiosTable.Type17->MemoryArrayHandle = mHandle16;
/*		if (gDMI->DIMM[2] && Index == 1 &&  TotalCount == 2)
		{
		    newSmbiosTable.Type17->Size =  gRAM->DIMM[2].ModuleSize;
		    newSmbiosTable.Type17->Speed = gRAM->DIMM[2].Frequency;
		} 
		mTotalSystemMemory += newSmbiosTable.Type17->Size;
		mMemory17[Index] = newSmbiosTable.Type17->Size > 0 ? mTotalSystemMemory : 0;
		DBG("mMemory17[%d] = %d, mTotalSystemMemory = %d\n", Index, mMemory17[Index], mTotalSystemMemory);
*/		
		
	
	//	SmbiosTable.Type17->FormFactor = MemoryFormFactorProprietaryCard; //why?
		switch (gCPUStructure.Family) 		
		{				
			case 0x06:				
			{				
				switch (gCPUStructure.Model)				
				{						
					case CPU_MODEL_CLARKDALE:						
					case CPU_MODEL_FIELDS:						
					case CPU_MODEL_DALES:						
					case CPU_MODEL_NEHALEM:
					case CPU_MODEL_NEHALEM_EX:						
					case CPU_MODEL_WESTMERE:						
					case CPU_MODEL_WESTMERE_EX:						
					case CPU_MODEL_XEON_MP:						
					case CPU_MODEL_LINCROFT:						
					case CPU_MODEL_SANDY_BRIDGE:	
					case CPU_MODEL_IVY_BRIDGE:
					case CPU_MODEL_JAKETOWN:
						
						newSmbiosTable.Type17->MemoryType = MemoryTypeDdr3;
						break;
												
					default:
						newSmbiosTable.Type17->MemoryType = MemoryTypeDdr2;						
						break;						
				}				
			}	
				//else keep as is
		}
		//??? - correct for table 6?
#if NOTSPD	
		if(iStrLen(gSettings.MemoryManufacturer, 64)>0){
			UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type17->Manufacturer, gSettings.MemoryManufacturer);			
		}
		if(iStrLen(gSettings.MemorySerialNumber, 64)>0){
			UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type17->SerialNumber, gSettings.MemorySerialNumber);			
		}
		if(iStrLen(gSettings.MemoryPartNumber, 64)>0){
			UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type17->PartNumber, gSettings.MemoryPartNumber);		
		}
/*		if(iStrLen(gSettings.MemorySpeed)>0){
			newSmbiosTable.Type17->Speed = (UINT16)StrDecimalToUintn(gSettings.MemorySpeed);			
		}
*/
#else
		INTN map = gDMI->DIMM[Index];
    INTN map0 = map;
		MEMORY_DEVICE_TYPE spdType;
		if (gDMI->DIMM[2] && Index == 1 &&  TotalCount == 2)
		{
      DBG(" Index=1 but DIMM[2] present. Redirect map: old=%d\n", map);
		    map = gDMI->DIMM[2];
      DBG("...new map=%d\n", map);
		}		
		if (gRAM->DIMM[map].InUse) {
      DBG("new map InUse\n");
		} else if (gRAM->DIMM[map0].InUse){
      map = map0;
			DBG("old map InUse\n");
    } else {
			DBG("both map not In Use. TODO?\n");
		}

		spdType = gRAM->DIMM[map].Type;
		if ((spdType == MemoryTypeDdr2) ||
				(spdType == MemoryTypeDdr3)) {
			newSmbiosTable.Type17->MemoryType = spdType;
		}
				
		if(iStrLen(gRAM->DIMM[map].Vendor, 64)>0 && gRAM->DIMM[map].InUse){
			UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type17->Manufacturer, gRAM->DIMM[map].Vendor);			
		}
		if(iStrLen(gRAM->DIMM[map].SerialNo, 64)>0 && gRAM->DIMM[map].InUse){
			UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type17->SerialNumber, gRAM->DIMM[map].SerialNo);			
		}
		if(iStrLen(gRAM->DIMM[map].PartNo, 64)>0 && gRAM->DIMM[map].InUse){
			UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type17->PartNumber, gRAM->DIMM[map].PartNo);		
		}
		if(gRAM->DIMM[map].Frequency>0 && gRAM->DIMM[map].InUse){
			newSmbiosTable.Type17->Speed = gRAM->DIMM[map].Frequency;			
		}
#endif
		
		//now I want to update deviceLocator and bankLocator		
	
		AsciiSPrint(deviceLocator, 10, "DIMM%d",  Index >> 1);
		AsciiSPrint(bankLocator, 10, "BANK%d", Index & 1);
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type17->DeviceLocator, (CHAR8*)&deviceLocator[0]);
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type17->BankLocator, (CHAR8*)&bankLocator[0]);
		
		mHandle17[Index] = LogSmbiosTable(newSmbiosTable);
	}
	return;
}

VOID
PatchTableType19 ()
{
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
//Slice - I created one table as a sum of all other. It is needed for SetupBrowser
	UINT32	TotalEnd = 0; 
	UINT8	PartWidth = 1;
	UINT16  SomeHandle = 0x1300; //as a common rule handle=(type<<8 + index)
	for (Index=0; Index<TotalCount+1; Index++) {
		SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_MEMORY_ARRAY_MAPPED_ADDRESS, Index);
		if (SmbiosTable.Raw == NULL) {			
			continue;
		}
		if (SmbiosTable.Type19->EndingAddress > TotalEnd) {
			TotalEnd = SmbiosTable.Type19->EndingAddress;
		}
		PartWidth = SmbiosTable.Type19->PartitionWidth;
		//SomeHandle = SmbiosTable.Type19->Hdr.Handle;
	}
	if (TotalEnd == 0) {
		TotalEnd = (mTotalSystemMemory << 10) - 1;
	}
	gTotalMemory = mTotalSystemMemory << 20;
	ZeroMem((VOID*)newSmbiosTable.Type19, MAX_TABLE_SIZE);
	newSmbiosTable.Type19->Hdr.Type = EFI_SMBIOS_TYPE_MEMORY_ARRAY_MAPPED_ADDRESS;
	newSmbiosTable.Type19->Hdr.Length = sizeof(SMBIOS_TABLE_TYPE19); 
	newSmbiosTable.Type19->Hdr.Handle = SomeHandle;
	newSmbiosTable.Type19->MemoryArrayHandle = mHandle16;
	newSmbiosTable.Type19->StartingAddress = 0;
	newSmbiosTable.Type19->EndingAddress = TotalEnd;
	newSmbiosTable.Type19->PartitionWidth = PartWidth;
	mHandle19 = LogSmbiosTable(newSmbiosTable);
	return ;
}

VOID PatchTableType20 ()
{
	UINTN	j = 0;
	//
	// Generate Memory Array Mapped Address info (TYPE 20)
	// not needed neither for Apple nor for EFI
	
	for (Index = 0; Index < MAX_RAM_SLOTS; Index++) {
		SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_MEMORY_DEVICE_MAPPED_ADDRESS, Index);
		if (SmbiosTable.Raw == NULL) {			
			return ;
		}
		TableSize = SmbiosTableLength(SmbiosTable);
		ZeroMem((VOID*)newSmbiosTable.Type20, MAX_TABLE_SIZE);
		CopyMem((VOID*)newSmbiosTable.Type20, (VOID*)SmbiosTable.Type20, TableSize);
		for (j=0; j < TotalCount; j++) {
			//EndingAddress in kb while mMemory in Mb
			if (((UINT64)mMemory17[j]  << 10) > newSmbiosTable.Type20->EndingAddress) {	
				newSmbiosTable.Type20->MemoryDeviceHandle = mHandle17[j];
				DBG("Type20[%d]->End = 0x%lx, mMemory17[%d] = 0x%lx\n",
						Index, newSmbiosTable.Type20->EndingAddress, j, (UINT64)mMemory17[j]  << 10);
//				DBG(" MemoryDeviceHandle = 0x%x\n", newSmbiosTable.Type20->MemoryDeviceHandle);
				mMemory17[j] = 0; // used
				break;
			}
			DBG("\n");
		}
		
		newSmbiosTable.Type20->MemoryArrayMappedAddressHandle = mHandle19;
		//
		// Record Smbios Type 20
		//
		LogSmbiosTable(newSmbiosTable);
	}
	return ;
}

VOID GetTableType32()
{
		SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_SYSTEM_BOOT_INFORMATION, 0);
		if (SmbiosTable.Raw == NULL) {
			return;
		}
		gBootStatus = SmbiosTable.Type32->BootStatus;
}
						   
// Apple Specific Structures
VOID PatchTableType128()
{
	//UINT32						UpAddress;
	//
	// FirmwareVolume (TYPE 128)
	// 
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, 128, 0);
	if (SmbiosTable.Raw == NULL) {
		//    DEBUG ((EFI_D_ERROR, "SmbiosTable: Type 128 (FirmwareVolume) not found!\n"));
		
		ZeroMem((VOID*)newSmbiosTable.Type128, MAX_TABLE_SIZE);	
		newSmbiosTable.Type128->Hdr.Type = 128;
		newSmbiosTable.Type128->Hdr.Length = sizeof(SMBIOS_TABLE_TYPE128); 
		newSmbiosTable.Type128->Hdr.Handle = 0x8000; //common rule
		newSmbiosTable.Type128->FirmwareFeatures = gFwFeatures; //0x80001417; //imac112 -> 0x1403
		newSmbiosTable.Type128->FirmwareFeaturesMask = 0xC0007fff; // 0xffff
		/*
		 FW_REGION_RESERVED   = 0,
		 FW_REGION_RECOVERY   = 1,
		 FW_REGION_MAIN       = 2, gHob->MemoryAbove1MB.PhysicalStart + ResourceLength
		 or fix as 0x200000 - 0x600000
		 FW_REGION_NVRAM      = 3,
		 FW_REGION_CONFIG     = 4,
		 FW_REGION_DIAGVAULT  = 5,		 
		 */
		//TODO - Slice - I have an idea that region should be the same as Efivar.bin
		newSmbiosTable.Type128->RegionCount = 1; 
		newSmbiosTable.Type128->RegionType[0] = FW_REGION_MAIN; 
		//UpAddress = mTotalSystemMemory << 20; //Mb -> b
		//			gHob->MemoryAbove1MB.PhysicalStart;
		newSmbiosTable.Type128->FlashMap[0].StartAddress = 0xFFE00000; //0xF0000;
		//			gHob->MemoryAbove1MB.PhysicalStart + gHob->MemoryAbove1MB.ResourceLength - 1;
		newSmbiosTable.Type128->FlashMap[0].EndAddress = 0xFFEFFFFF;
//		newSmbiosTable.Type128->RegionType[1] = FW_REGION_NVRAM; //Efivar
//		newSmbiosTable.Type128->FlashMap[1].StartAddress = 0x15000; //0xF0000;
//		newSmbiosTable.Type128->FlashMap[1].EndAddress = 0x1FFFF;
		//region type=1 also present in mac
		LogSmbiosTable(newSmbiosTable);
		return ;		
	}	
	//
	// Log Smbios Record Type128
	//
	LogSmbiosTable(SmbiosTable);
	return ;
}

VOID PatchTableType130()
{
	//
	// MemorySPD (TYPE 130)
	// TODO:  read SPD and place here. But for a what? 
	//
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, 130, 0);
	if (SmbiosTable.Raw == NULL) {
		return ;
	}	
	//
	// Log Smbios Record Type130
	//
	LogSmbiosTable(SmbiosTable);		
	return ;
}



VOID PatchTableType131()
{
	// Get Table Type131
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, 131, 0);
	if (SmbiosTable.Raw != NULL) {
    MsgLog("Table 131 is present, CPUType=%x\n", SmbiosTable.Type131->ProcessorType);
    MsgLog("Change to: %x\n", gSettings.CpuType);
  }
  
		ZeroMem((VOID*)newSmbiosTable.Type131, MAX_TABLE_SIZE);
		newSmbiosTable.Type131->Hdr.Type = 131;
		newSmbiosTable.Type131->Hdr.Length = sizeof(SMBIOS_STRUCTURE)+2; 
		newSmbiosTable.Type131->Hdr.Handle = 0x8300; //common rule
		// Patch ProcessorType
			newSmbiosTable.Type131->ProcessorType = gSettings.CpuType;
		Handle = LogSmbiosTable(newSmbiosTable);
		return;
}

VOID PatchTableType132()
{
	// Get Table Type132
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, 132, 0);
	if (SmbiosTable.Raw != NULL) {
		MsgLog("Table 132 is present, QPI=%x\n", SmbiosTable.Type132->ProcessorBusSpeed);
    MsgLog("Change to: %x\n", gSettings.QPI);
  }
	
		ZeroMem((VOID*)newSmbiosTable.Type132, MAX_TABLE_SIZE);
		newSmbiosTable.Type132->Hdr.Type = 132;
		newSmbiosTable.Type132->Hdr.Length = sizeof(SMBIOS_STRUCTURE)+2; 
		newSmbiosTable.Type132->Hdr.Handle = 0x8400; //ugly
	// Patch ProcessorBusSpeed
		if(gSettings.QPI){
			newSmbiosTable.Type132->ProcessorBusSpeed = gSettings.QPI;
		} else {
			newSmbiosTable.Type132->ProcessorBusSpeed = DivU64x32(gSettings.BusSpeed, kilo)  << 2;
		}
		Handle = LogSmbiosTable(newSmbiosTable);
		return;
}


EFI_STATUS PrepatchSmbios()
{
	EFI_STATUS				Status = EFI_SUCCESS;
	UINTN					BufferLen;
	EFI_PHYSICAL_ADDRESS     BufferPtr;
//	UINTN					Index;

	// Get SMBIOS Tables
	Smbios = FindOemSMBIOSPtr();
//	DBG("OEM SMBIOS EPS=%p\n", Smbios);
//	DBG("OEM Tables = %x\n", ((SMBIOS_TABLE_ENTRY_POINT*)Smbios)->TableAddress);
	if (!Smbios) {
		Status = EFI_NOT_FOUND;
//		DBG("Original SMBIOS System Table not found! Getting from Hob...\n");
		Smbios = GetSmbiosTablesFromHob();
//		DBG("HOB SMBIOS EPS=%p\n", Smbios);
		if (!Smbios) {
//			DBG("And here SMBIOS System Table not found! Trying System table ...\n");
			// this should work on any UEFI
			Smbios = GetSmbiosTablesFromConfigTables();
//			DBG("ConfigTables SMBIOS EPS=%p\n", Smbios);
		if (!Smbios) {
//				DBG("And here SMBIOS System Table not found! Exiting...\n");
			return EFI_NOT_FOUND;
		}		
	}
	}
	
	//original EPS and tables
	EntryPoint = (SMBIOS_TABLE_ENTRY_POINT*)Smbios; //yes, it is old SmbiosEPS
//	Smbios = (VOID*)(UINT32)EntryPoint->TableAddress; // here is flat Smbios database. Work with it
	//how many we need to add for tables 128, 130, 131, 132 and for strings?
	BufferLen = 0x20 + EntryPoint->TableLength + 64 * 10; 
	//new place for EPS and tables. Allocated once for both
	BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS;
	Status = gBS->AllocatePages (AllocateMaxAddress, EfiACPIMemoryNVS, /*EfiACPIReclaimMemory, 	*/
	EFI_SIZE_TO_PAGES(BufferLen), &BufferPtr);
	if (EFI_ERROR (Status)) {
//		Print(L"There is error allocating pages in EfiACPIMemoryNVS!\n");
		Status = gBS->AllocatePages (AllocateMaxAddress,  /*EfiACPIMemoryNVS, */EfiACPIReclaimMemory,
											   ROUND_PAGE(BufferLen)/EFI_PAGE_SIZE, &BufferPtr);
		if (EFI_ERROR (Status)) {
//			Print(L"There is error allocating pages in EfiACPIReclaimMemory!\n");
		}
	}
//	DBG("Buffer @ %p\n", BufferPtr);
	if (BufferPtr) {
		SmbiosEpsNew = (SMBIOS_TABLE_ENTRY_POINT *)(UINTN)BufferPtr; //this is new EPS
	} else {
		SmbiosEpsNew = EntryPoint; //is it possible?!
	}
	ZeroMem(SmbiosEpsNew, BufferLen);
//	DBG("New EntryPoint = %p\n", SmbiosEpsNew);
	NumberOfRecords = 0;
	MaxStructureSize = 0;
	//preliminary fill EntryPoint with some data
	CopyMem ((VOID *)SmbiosEpsNew, (VOID *)EntryPoint, sizeof(SMBIOS_TABLE_ENTRY_POINT));
	
	
	Smbios = (VOID*)(SmbiosEpsNew + 1); //this is a C-language trick. I hate it but use. +1 means +sizeof(SMBIOS_TABLE_ENTRY_POINT)
	Current = (UINT8*)Smbios; //begin fill tables from here
	SmbiosEpsNew->TableAddress = (UINT32)(UINTN)Current;
	SmbiosEpsNew->EntryPointLength = sizeof(SMBIOS_TABLE_ENTRY_POINT); // no matter on other versions
	SmbiosEpsNew->MajorVersion = 2;
	SmbiosEpsNew->MinorVersion = 6;	
	SmbiosEpsNew->SmbiosBcdRevision = 0x26; //Slice - we want to have v2.6
	
	//Create space for SPD
	gRAM = AllocateZeroPool(sizeof(MEM_STRUCTURE));
	gDMI = AllocateZeroPool(sizeof(DMI));
	
	//Collect information for use in menu
	GetTableType1();
	GetTableType2();
	GetTableType3();
	GetTableType4();
	GetTableType16();
	GetTableType17();
	GetTableType32(); //get BootStatus here to decide what to do
	MsgLog("Boot status=%x\n", gBootStatus);
  //for example the bootloader may go to Recovery is BootStatus is Fail
	return 	Status;
} 

VOID PatchSmbios(VOID) //continue
{
//	EFI_STATUS Status = EFI_UNSUPPORTED;
//	newSmbiosTable = (SMBIOS_STRUCTURE_POINTER)(UINT8*)AllocateZeroPool(MAX_TABLE_SIZE);
  newSmbiosTable.Raw = (UINT8*)AllocateZeroPool(MAX_TABLE_SIZE);
	//Slice - order of patching is significant
	PatchTableType0();
	PatchTableType1();
	PatchTableType2();
	PatchTableType3();
	PatchTableType7(); //we should know handles before patch Table4
	PatchTableType4();		
	PatchTableType6();
	PatchTableType9();
  PatchTableType11();
	PatchTableTypeSome();
	PatchTableType16();
	PatchTableType17();
	PatchTableType19();
	PatchTableType20();
	PatchTableType128();
	PatchTableType130();
	PatchTableType131();
	PatchTableType132();
	AddSmbiosEndOfTable();
	if(MaxStructureSize > MAX_TABLE_SIZE){
//    Print(L"Too long SMBIOS!\n");
	}
	FreePool((VOID*)newSmbiosTable.Raw);	
	
	// there is no need to keep all tables in numeric order. It is not needed
// neither by specs nor by AppleSmbios.kext	
}	

VOID FinalizeSmbios() //continue
{
	EFI_PEI_HOB_POINTERS	GuidHob;
	EFI_PEI_HOB_POINTERS	HobStart;
	EFI_PHYSICAL_ADDRESS    *Table = NULL;
	//UINTN					TableLength = 0;
	
	// Get Hob List
	HobStart.Raw = GetHobList ();
	
	if (HobStart.Raw != NULL) {
		// find SMBIOS in hob
		for (Index = 0; Index < sizeof (gTableGuidArray) / sizeof (*gTableGuidArray); ++Index) {
			GuidHob.Raw = GetNextGuidHob (gTableGuidArray[Index], HobStart.Raw);
			if (GuidHob.Raw != NULL) {
				Table = GET_GUID_HOB_DATA (GuidHob.Guid);
				//TableLength = GET_GUID_HOB_DATA_SIZE (GuidHob);
				if (Table != NULL) {
					break;
				} 
			}
		}
	}
	
	//
	// Install SMBIOS in Config table
	SmbiosEpsNew->TableLength = (UINT16)((UINT32)(UINTN)Current - (UINT32)(UINTN)Smbios);
	SmbiosEpsNew->NumberOfSmbiosStructures = NumberOfRecords;
	SmbiosEpsNew->MaxStructureSize = MaxStructureSize;
	SmbiosEpsNew->IntermediateChecksum = 0;
	SmbiosEpsNew->IntermediateChecksum = (UINT8)(256 - Checksum8((UINT8*)SmbiosEpsNew + 0x10, 																	SmbiosEpsNew->EntryPointLength - 0x10));
	SmbiosEpsNew->EntryPointStructureChecksum = 0;
	SmbiosEpsNew->EntryPointStructureChecksum = (UINT8)(256 - Checksum8((UINT8*)SmbiosEpsNew, SmbiosEpsNew->EntryPointLength));
//	DBG("SmbiosEpsNew->EntryPointLength = %d\n", SmbiosEpsNew->EntryPointLength);
//	DBG("DMI checksum = %d\n", Checksum8((UINT8*)SmbiosEpsNew, SmbiosEpsNew->EntryPointLength));
	gBS->InstallConfigurationTable (&gEfiSmbiosTableGuid, (VOID*)SmbiosEpsNew);
	gST->Hdr.CRC32 = 0;
	gBS->CalculateCrc32 ((UINT8 *) &gST->Hdr, gST->Hdr.HeaderSize, &gST->Hdr.CRC32);
	
	//
	// Fix it in Hob list
	//
	// No smbios in Hob list on Aptio, so no need to update it there.
	// But even if it would be there, loading of OSX would overwrite it
	// since this list on my board is inside space needed for kernel
	// (ha! like many other UEFI stuff).
	// It's enough to add it to Conf.table.
	//
	if (Table != NULL) {
		//PauseForKey(L"installing SMBIOS in Hob\n");
		*Table = (UINT32)(UINTN)SmbiosEpsNew;
	} 
	
	return;
}
