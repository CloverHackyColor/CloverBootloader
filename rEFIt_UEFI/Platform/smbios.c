/**
 smbios.c
 implementation for smbios table patching
  Slice 2011. 
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/



#define DEBUG_SMBIOS 0

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


extern CHAR8*					AppleSystemVersion[];
extern UINT8					gDefaultType;
extern EFI_SYSTEM_TABLE*		gSystemTable;
extern EFI_BOOT_SERVICES*		gBootServices; 
extern GUI_MENU_DATA			gSettingsFromMenu;
extern CPU_STRUCTURE			gCPUStructure;
extern EFI_GUID					gPlatformUuid;
CHAR8							gOEMProduct[40];

EFI_GUID						gUuid;

EFI_GUID						*gTableGuidArray[] = {&gEfiSmbiosTableGuid};

//EFI_PHYSICAL_ADDRESS			*smbiosTable;
VOID							*Smbios;  //pointer to SMBIOS data
SMBIOS_TABLE_ENTRY_POINT		*EntryPoint; //SmbiosEps original
SMBIOS_TABLE_ENTRY_POINT		*SmbiosEpsNew; //new SmbiosEps
//for patching
SMBIOS_STRUCTURE_POINTER		SmbiosTable;
SMBIOS_STRUCTURE_POINTER		newSmbiosTable;
UINT16							NumberOfRecords;
UINT16							MaxStructureSize;
UINT8*							Current; //pointer to the current end of tables
EFI_SMBIOS_TABLE_HEADER			*Record;
EFI_SMBIOS_HANDLE				Handle;
EFI_SMBIOS_TYPE					Type;
//EFI_HANDLE						ProducerHandle;
UINTN							stringNumber;
UINTN							TableSize;
UINTN							Index, Size, NewSize, MaxSize;
//UINT16							CpuBus;
UINT16							CoreCache = 0;
UINT16							TotalCount = 0;
UINT16							L1, L2, L3;
UINT32							MaxMemory = 0;
UINT32			mTotalSystemMemory;
UINT64						gTotalMemory;
UINT16			mHandle3;
UINT16			mHandle16 = 0xFFFE;;
UINT16			mHandle17[MAX_SLOT_COUNT];
UINT16			mHandle19;
UINT16			mMemory17[MAX_SLOT_COUNT];
UINT8			mInstalled[MAX_SLOT_COUNT];
UINT8			mEnabled[MAX_SLOT_COUNT];
BOOLEAN			gMobile;
UINT8			gBootStatus;
BOOLEAN			Once;

MEM_STRUCTURE*					gRAM;
DMI*							gDMI;


#define MAX_HANDLE	0xFEFF
#define SMBIOS_PTR        SIGNATURE_32('_','S','M','_')
#define MAX_TABLE_SIZE 512


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

	if (Once) {
		DBG("Raw table data:\n");
		for (ALength=0; ALength<Length; ALength++){
			if((ALength & 0xF) == 0) DBG("\n");
			DBG("%02x ", SmbiosTable.Raw[ALength]);		
		}
		DBG("\n");
		Once = FALSE;
	}

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
	DBG("Table type %d field %d\n", SmbiosTable.Hdr->Type, *Field);
	DBG("Old string length=%d new length=%d\n", ALength, BLength);
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

#if DEBUG_SMBIOS != 0
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
		AString ++;
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
	// Get Table Type0
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_BIOS_INFORMATION, 0);
	if (SmbiosTable.Raw == NULL) {
//		AsciiPrint("SmbiosTable: Type 0 (Bios Information) not found!\n");
		
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
	if(iStrLen(gSettingsFromMenu.VendorName, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type0->Vendor, gSettingsFromMenu.VendorName);
	}
	if(iStrLen(gSettingsFromMenu.RomVersion, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type0->BiosVersion, gSettingsFromMenu.RomVersion);
	}
	if(iStrLen(gSettingsFromMenu.ReleaseDate, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type0->BiosReleaseDate, gSettingsFromMenu.ReleaseDate);		
	}
	
	Handle = LogSmbiosTable(newSmbiosTable);
}

VOID GetTableType1()
{
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_SYSTEM_INFORMATION, 0);
	if (SmbiosTable.Raw == NULL) {
		AsciiPrint("SmbiosTable: Type 1 (System Information) not found!\n");
		return;
	}
		
	CopyMem(&gUuid, (VOID*)&SmbiosTable.Type1->Uuid, 16);
//	AsciiStrCpy(gOEMProduct, GetSmbiosString(SmbiosTable, SmbiosTable.Type1->ProductName));
	//Check the validity
	if (((gUuid.Data3 & 0xF000) == 0) || (AsciiStrStr(gSettingsFromMenu.Uuid_Fix,"y") || AsciiStrStr(gSettingsFromMenu.Uuid_Fix,"Y"))) {
		CopyMem(&gUuid, (VOID*)&gEfiSmbiosTableGuid, 16); //gPlatformUuid
		gUuid.Data1 ^= 0x1234; //random 
		gUuid.Data2 ^= 0x2341;
		gUuid.Data3 ^= 0x3412;
	}
	return;
}


VOID PatchTableType1()
{
//	CHAR8* AString;
	// Get Table Type1
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
	
	if((gUuid.Data3 & 0xF000) != 0) {
		CopyMem((VOID*)&newSmbiosTable.Type1->Uuid, &gUuid, 16);
	}
	
	if(iStrLen(gSettingsFromMenu.ManufactureName, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type1->Manufacturer, gSettingsFromMenu.ManufactureName);
	}
	if(iStrLen(gSettingsFromMenu.ProductName, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type1->ProductName, gSettingsFromMenu.ProductName);
	}
	if(iStrLen( gSettingsFromMenu.VersionNr, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type1->Version, gSettingsFromMenu.VersionNr);
	}
	if(iStrLen(gSettingsFromMenu.SerialNr, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type1->SerialNumber, gSettingsFromMenu.SerialNr);
	}
	if(iStrLen(gSettingsFromMenu.BoardNumber, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type1->SKUNumber, gSettingsFromMenu.BoardNumber);
	}	
	if(iStrLen(gSettingsFromMenu.FamilyName, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type1->Family, gSettingsFromMenu.FamilyName);
	}	
	
	Handle = LogSmbiosTable(newSmbiosTable);
	return;
}

VOID PatchTableType2()
{
	// Get Table Type2
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_BASEBOARD_INFORMATION, 0);
	if (SmbiosTable.Raw == NULL) {
//		AsciiPrint("SmbiosTable: Type 2 (BaseBoard Information) not found!\n");
		return;
	}
	Size = SmbiosTable.Type2->Hdr.Length; //old size
	TableSize = SmbiosTableLength(SmbiosTable); //including strings
	NewSize = 0x0F; //sizeof(SMBIOS_TABLE_TYPE2);
	ZeroMem((VOID*)newSmbiosTable.Type2, MAX_TABLE_SIZE);
	
	if (NewSize > Size) {
		CopyMem((VOID*)newSmbiosTable.Type2, (VOID*)SmbiosTable.Type2, Size); //copy main table
		CopyMem((CHAR8*)newSmbiosTable.Type2 + NewSize, (CHAR8*)SmbiosTable.Type2 + Size, TableSize - Size); //copy strings
		newSmbiosTable.Type2->Hdr.Length = NewSize;
	} else {
		CopyMem((VOID*)newSmbiosTable.Type2, (VOID*)SmbiosTable.Type2, TableSize); //copy full table
	}
	
	newSmbiosTable.Type2->ChassisHandle = mHandle3;	//from GetTableType3
	newSmbiosTable.Type2->BoardType = BaseBoardTypeMotherBoard;
	ZeroMem((VOID*)&newSmbiosTable.Type2->FeatureFlag, sizeof(BASE_BOARD_FEATURE_FLAGS));
	newSmbiosTable.Type2->FeatureFlag.Motherboard = 1;
	newSmbiosTable.Type2->FeatureFlag.Replaceable = 1;
	Once = TRUE;
	
	if(iStrLen(gSettingsFromMenu.BoardManufactureName, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type2->Manufacturer, gSettingsFromMenu.BoardManufactureName);
	}
	if(iStrLen(gSettingsFromMenu.BoardNumber, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type2->ProductName, gSettingsFromMenu.BoardNumber);
	}
	if(iStrLen( gSettingsFromMenu.VersionNr, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type2->Version, gSettingsFromMenu.VersionNr);
	}	
	if(iStrLen(gSettingsFromMenu.BoardSerialNumber, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type2->SerialNumber, gSettingsFromMenu.BoardSerialNumber);
	}
	if(iStrLen(gSettingsFromMenu.LocationInChassis, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type2->LocationInChassis, gSettingsFromMenu.LocationInChassis);
	}
	
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
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_SYSTEM_ENCLOSURE, 0);
	if (SmbiosTable.Raw == NULL) {
		AsciiPrint("SmbiosTable: Type 3 (System Chassis Information) not found!\n");
		gMobile = FALSE; //default value
		return;
	}
	mHandle3 = SmbiosTable.Type3->Hdr.Handle;
	gMobile = ((SmbiosTable.Type3->Type) >= 8);

	return;
}

VOID PatchTableType3()
{
	// Get Table Type3
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_SYSTEM_ENCLOSURE, 0);
	if (SmbiosTable.Raw == NULL) {
//		AsciiPrint("SmbiosTable: Type 3 (System Chassis Information) not found!\n");
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
		
	if(iStrLen(gSettingsFromMenu.ChassisManufacturer, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type3->Manufacturer, gSettingsFromMenu.ChassisManufacturer);
	}
//SIC! According to iMac there must be the BoardNumber	
	if(iStrLen(gSettingsFromMenu.BoardNumber, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type3->Version, gSettingsFromMenu.BoardNumber);
	}
	if(iStrLen(gSettingsFromMenu.SerialNr, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type3->SerialNumber, gSettingsFromMenu.SerialNr);
	}
	if(iStrLen(gSettingsFromMenu.ChassisAssetTag, 64)>0){
		UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type3->AssetTag, gSettingsFromMenu.ChassisAssetTag);
	}
	
	Handle = LogSmbiosTable(newSmbiosTable);
	return;
}

VOID GetTableType4()
{
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_PROCESSOR_INFORMATION, 0);
	if (SmbiosTable.Raw == NULL) {
		AsciiPrint("SmbiosTable: Type 4 (Processor Information) not found!\n");
		return;
	}
	gCPUStructure.ExternalClock = SmbiosTable.Type4->ExternalClock;
	AsciiSPrint(gSettingsFromMenu.BusSpeed, 10, "%d", gCPUStructure.ExternalClock);
	gCPUStructure.CurrentSpeed = SmbiosTable.Type4->CurrentSpeed;
	AsciiSPrint(gSettingsFromMenu.CpuFreqMHz, 10, "%d", gCPUStructure.CurrentSpeed);
	
	return;
}

VOID PatchTableType4()
{
	UINTN							AddBrand = 0;
	CHAR8							BrandStr[48];
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
		// TODO: Set SmbiosTable.Type4->ProcessorFamily for all implemented CPU models
		Once = TRUE;
		if (gCPUStructure.Model == CPU_MODEL_ATOM) {
			newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelAtom;
		}
		if (gCPUStructure.Model == CPU_MODEL_DOTHAN) {
			if (gCPUStructure.Mobile==TRUE)
			{
				newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCoreSoloMobile;
				if (gCPUStructure.Cores==2)
				{
					newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCoreDuoMobile;
				}
			} else {
				newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCoreSolo;
				if (gCPUStructure.Cores==2)
				{
					newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCoreDuo;
				}
			}
		}
		if (gCPUStructure.Model == CPU_MODEL_YONAH) {
			//this is change in Smbios v2.6 vs v2.3
			if (gCPUStructure.Mobile==TRUE)
			{
				newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCoreSoloMobile;
			} else {
				newSmbiosTable.Type4->ProcessorFamily = ProcessorFamilyIntelCoreSolo;
			}
		}
		if (gCPUStructure.Model == CPU_MODEL_MEROM) { //and Conroe
			if (gCPUStructure.Mobile==TRUE)
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
			if (gCPUStructure.Mobile==TRUE)
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
			// Set Default Family ?
			
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
		//should check before uncomment
//		newSmbiosTable.Type4->ProcessorId.Signature.FeatureFlags = gCPUStructure.Features;
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
		
		if(iStrLen(gSettingsFromMenu.CPUSerial, 10)>0){
			UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type4->SerialNumber, gSettingsFromMenu.CPUSerial);
		}
		
		Handle = LogSmbiosTable(newSmbiosTable);
	}	
	return;
}

VOID PatchTableType6()
{
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
		mInstalled[Index]	=  (1ULL << (SmbiosTable.Type6->InstalledSize.InstalledOrEnabledSize & 0x7F)) * (1024 * 1024);
		MsgLog("MEMORY_MODULE %d Installed %d ", Index, mInstalled[Index]);
		mEnabled[Index]		= (1ULL << ((UINT8)SmbiosTable.Type6->EnabledSize.InstalledOrEnabledSize & 0x7F)) * (1024 * 1024);
		MsgLog(" Enabled %d \n", mEnabled[Index]);
		LogSmbiosTable(SmbiosTable);		
	}

	return;
}

VOID PatchTableType7()
{
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
		CoreCache = newSmbiosTable.Type7->CacheConfiguration & CPU_CACHE_LEVEL; //0x3 means a two bit mask, so CPU_CACHE_LEVEL is always 3
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
	 
	 */

	for (Index = 0; Index < 64; Index++) { 
		SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_SYSTEM_SLOTS,Index);
		if (SmbiosTable.Raw == NULL) {
			continue;
		}
		LogSmbiosTable(SmbiosTable);		
	}
	
	return;
}

		  
VOID PatchTableTypeSome()
{
	//some unused but interesting tables. Just log as is
	UINT8 tableTypes[13] = {8, 10, 11, 18, 21, 22, 27, 28, 32, 33, 129, 217, 219};
	UINTN	IndexType;
	//
	// Different types 
	for (IndexType = 0; IndexType < 13; IndexType++) {
		for (Index = 0; Index < 16; Index++) {
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
	mTotalSystemMemory = 0; //later we will add to the value, here initialize it
	TotalCount = 0;
	// Get Table Type16 and set Device Count
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_PHYSICAL_MEMORY_ARRAY, 0);
	if (SmbiosTable.Raw == NULL) {
		AsciiPrint("SmbiosTable: Type 16 (Physical Memory Array) not found!\n");
		return;
	}
	TotalCount = newSmbiosTable.Type16->NumberOfMemoryDevices;
	if (!TotalCount) {
		TotalCount = MAX_SLOT_COUNT;
	}
	gDMI->MaxMemorySlots = TotalCount;
	return;
}


VOID PatchTableType16()
{
//	mTotalSystemMemory = 0; //later we will add to the value, here initialize it
//	TotalCount = 0;
	mHandle16 = 0xFFFE;
	
	// Get Table Type16 and set Device Count
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_PHYSICAL_MEMORY_ARRAY, 0);
	if (SmbiosTable.Raw == NULL) {
		AsciiPrint("SmbiosTable: Type 16 (Physical Memory Array) not found!\n");
		return;
	}
	TableSize = SmbiosTableLength(SmbiosTable);
	ZeroMem((VOID*)newSmbiosTable.Type16, MAX_TABLE_SIZE);
	CopyMem((VOID*)newSmbiosTable.Type16, (VOID*)SmbiosTable.Type16, TableSize);
//Slice - I am not sure I want these values	
//	newSmbiosTable.Type16->Location = MemoryArrayLocationProprietaryAddonCard;
//	newSmbiosTable.Type16->Use = MemoryArrayUseSystemMemory;
//	newSmbiosTable.Type16->MemoryErrorCorrection = MemoryErrorCorrectionMultiBitEcc;
	TotalCount = newSmbiosTable.Type16->NumberOfMemoryDevices;
	if (!TotalCount) {
		TotalCount = MAX_SLOT_COUNT;
	}
	mHandle16 = LogSmbiosTable(newSmbiosTable);
	return;
}

VOID GetTableType17()
{
	// Get Table Type17 and count Size
	gDMI->CntMemorySlots = 0;
	gDMI->MemoryModules = 0;
	for (Index = 0; Index < TotalCount; Index++) {
		SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_MEMORY_DEVICE, Index);
		if (SmbiosTable.Raw == NULL) {
//			AsciiPrint("SmbiosTable: Type 17 (Memory Device number %d) not found!\n", Index);
			continue;
		}
		gDMI->CntMemorySlots++;
		if (SmbiosTable.Type17->Size > 0) {
			gDMI->MemoryModules++;
		}
		if (SmbiosTable.Type17->Speed > 0) {
			gRAM->DIMM[Index].Frequency = SmbiosTable.Type17->Speed;
		}
	}
}
		

VOID PatchTableType17()
{
	// Get Table Type17 and count Size
	for (Index = 0; Index < TotalCount; Index++) {
		SmbiosTable = GetSmbiosTableFromType (EntryPoint, EFI_SMBIOS_TYPE_MEMORY_DEVICE, Index);
		if (SmbiosTable.Raw == NULL) {
//			AsciiPrint("SmbiosTable: Type 17 (Memory Device number %d) not found!\n", Index);
			continue;
		}
		TableSize = SmbiosTableLength(SmbiosTable);
		ZeroMem((VOID*)newSmbiosTable.Type17, MAX_TABLE_SIZE);
		CopyMem((VOID*)newSmbiosTable.Type17, (VOID*)SmbiosTable.Type17, TableSize);
		Once = TRUE;		
		newSmbiosTable.Type17->MemoryArrayHandle = mHandle16;
		mMemory17[Index] = mTotalSystemMemory + newSmbiosTable.Type17->Size;
		mTotalSystemMemory = mMemory17[Index];
		
		
#if NOTSPD		
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
					case CPU_MODEL_JAKETOWN:
						
						newSmbiosTable.Type17->MemoryType = MemoryTypeDdr3;
						break;
												
					default:
						newSmbiosTable.Type17->MemoryType = MemoryTypeDdr2;						
						break;						
				}				
			}				
		}
		//??? - correct for table 6?

		if(iStrLen(gSettingsFromMenu.MemoryManufacturer, 64)>0){
			UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type17->Manufacturer, gSettingsFromMenu.MemoryManufacturer);			
		}
		if(iStrLen(gSettingsFromMenu.MemorySerialNumber, 64)>0){
			UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type17->SerialNumber, gSettingsFromMenu.MemorySerialNumber);			
		}
		if(iStrLen(gSettingsFromMenu.MemoryPartNumber, 64)>0){
			UpdateSmbiosString(newSmbiosTable, &newSmbiosTable.Type17->PartNumber, gSettingsFromMenu.MemoryPartNumber);		
		}
/*		if(iStrLen(gSettingsFromMenu.MemorySpeed)>0){
			newSmbiosTable.Type17->Speed = (UINT16)AsciiStrDecimalToUintn(gSettingsFromMenu.MemorySpeed);			
		}
*/
#else
		INTN map = gDMI->DIMM[Index];
		if (gRAM->DIMM[map].InUse) {
			newSmbiosTable.Type17->MemoryType = gRAM->DIMM[map].Type;
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
			break;
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
		for (j=0; j<TotalCount; j++) {
			if (((UINT32)mMemory17[j]  << 20) < newSmbiosTable.Type20->EndingAddress)
			{
				newSmbiosTable.Type20->MemoryDeviceHandle = mHandle17[j];
			}
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
		newSmbiosTable.Type128->FirmwareFeatures = 0x80000015; //imac112 -> 0x1403
		newSmbiosTable.Type128->FirmwareFeaturesMask = 0x800003ff; // 0xffff
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
		newSmbiosTable.Type128->RegionCount = 2; 
		newSmbiosTable.Type128->RegionType[0] = FW_REGION_MAIN; 
		//UpAddress = mTotalSystemMemory << 20; //Mb -> b
		//			gHob->MemoryAbove1MB.PhysicalStart;
		newSmbiosTable.Type128->FlashMap[0].StartAddress = 0xFFE00000; //0xF0000;
		//			gHob->MemoryAbove1MB.PhysicalStart + gHob->MemoryAbove1MB.ResourceLength - 1;
		newSmbiosTable.Type128->FlashMap[0].EndAddress = 0xFFEFFFFF;
		newSmbiosTable.Type128->RegionType[1] = FW_REGION_NVRAM; //Efivar
		newSmbiosTable.Type128->FlashMap[1].StartAddress = 0x15000; //0xF0000;
		newSmbiosTable.Type128->FlashMap[1].EndAddress = 0x1FFFF;
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
	// TODO: LocateProtocol(Smbus) and read SPD. Addresses are known from Chameleon sources. 
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
	if (SmbiosTable.Raw == NULL) {
		ZeroMem((VOID*)newSmbiosTable.Type131, MAX_TABLE_SIZE);
		newSmbiosTable.Type131->Hdr.Type = 131;
		newSmbiosTable.Type131->Hdr.Length = sizeof(SMBIOS_STRUCTURE)+2; 
		newSmbiosTable.Type131->Hdr.Handle = 0x8300; //common rule
		// Patch ProcessorType
		if(iStrLen(gSettingsFromMenu.CpuType, 10)>0){
			newSmbiosTable.Type131->ProcessorType = (UINT16)AsciiStrHexToUintn(gSettingsFromMenu.CpuType);
		} else {
			newSmbiosTable.Type131->ProcessorType = gCPUtype;
		}	
		Handle = LogSmbiosTable(newSmbiosTable);
		return;
	}	
		
	LogSmbiosTable(SmbiosTable);
	return;
}

VOID PatchTableType132()
{
	// Get Table Type132
	SmbiosTable = GetSmbiosTableFromType (EntryPoint, 132, 0);
	if (SmbiosTable.Raw == NULL) {
		ZeroMem((VOID*)newSmbiosTable.Type132, MAX_TABLE_SIZE);
		newSmbiosTable.Type132->Hdr.Type = 132;
		newSmbiosTable.Type132->Hdr.Length = sizeof(SMBIOS_STRUCTURE)+2; 
		newSmbiosTable.Type132->Hdr.Handle = 0x8400; //ugly
	// Patch ProcessorBusSpeed
		if(gCPUStructure.ProcessorInterconnectSpeed){
			newSmbiosTable.Type132->ProcessorBusSpeed = gCPUStructure.ProcessorInterconnectSpeed;
		} else {
			UINT16 res = (gBusSpeed % 10) / 3;
			newSmbiosTable.Type132->ProcessorBusSpeed = (gBusSpeed << 2) + res;
		}
		Handle = LogSmbiosTable(newSmbiosTable);
		return;
	}
	
	LogSmbiosTable(SmbiosTable);
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
	DBG("OEM SMBIOS EPS=%p\n", Smbios);
	DBG("OEM Tables = %x\n", ((SMBIOS_TABLE_ENTRY_POINT*)Smbios)->TableAddress);
	if (!Smbios) {
		Status = EFI_NOT_FOUND;
		//before desktop created we can't use SHOW_ON_ERROR
		//SHOW_ON_ERROR(Status,"Original SMBIOS System Table not found! Getting from Hob...",
		//			  L"Исходный SMBIOS не найден. Ищем в ЕФИ...", WARNING_ICON);
		AsciiPrint("Original SMBIOS System Table not found! Getting from Hob...\n");
		Smbios = GetSmbiosTablesFromHob ();
		if (!Smbios) {
			//SHOW_ON_ERROR(Status,"And here SMBIOS System Table not found! Exiting...",
			//			  L"И здесь ничего нет... Ну и че делать?", STOP_ICON);
			AsciiPrint("And here SMBIOS System Table not found! Exiting...\n");
			return EFI_NOT_FOUND;
		}		
	}
	//original EPS and tables
	EntryPoint = (SMBIOS_TABLE_ENTRY_POINT*)Smbios; //yes, it is old SmbiosEPS
//	Smbios = (VOID*)(UINT32)EntryPoint->TableAddress; // here is flat Smbios database. Work with it
	//how many we need to add for tables 128, 130, 131, 132 and for strings?
	BufferLen = 0x1F + SYS_TABLE_PAD(0x1F) + EntryPoint->TableLength + 64 * 10; 
//	DBG("OEM Table length = %d = 0x%x\n", EntryPoint->TableLength, EntryPoint->TableLength);
//	DBG("Reclaim for buffer %x\n", BufferLen);
	//new place for EPS and tables. Allocated once for both
	BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS;
	Status = gBootServices->AllocatePages (AllocateMaxAddress, EfiACPIMemoryNVS, /*EfiACPIReclaimMemory, 	*/
	EFI_SIZE_TO_PAGES(BufferLen), &BufferPtr);
	if (EFI_ERROR (Status)) {
		//SHOW_ON_ERROR(Status, "There were errors allocating pages!",
		//			  L"Не могу выделить память для SMBIOS", STOP_ICON);
		AsciiPrint("There is error allocating pages in EfiACPIMemoryNVS!\n");
		Status = gBootServices->AllocatePages (AllocateMaxAddress,  /*EfiACPIMemoryNVS, */EfiACPIReclaimMemory,
											   RoundPage(BufferLen)/EFI_PAGE_SIZE, &BufferPtr);
		if (EFI_ERROR (Status)) {
			AsciiPrint("There is error allocating pages in EfiACPIReclaimMemory!\n");
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
	GetTableType3();
	GetTableType4();
	GetTableType16();
	GetTableType17();
	GetTableType32(); //get BootStatus here to decide what to do
	MsgLog("Boot status=%x\n", gBootStatus);
/*	if (DEBUG_SMBIOS == 2) {
		WaitForKeyPress("press any key...");
	}*/
	return 	Status;
} 

VOID PatchSmbios() //continue
{
	EFI_STATUS Status = EFI_UNSUPPORTED;
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
	   SHOW_ON_ERROR(Status, "Found too large SMBIOS table! Correct settings and try again.",
					 L"Длинновата у вас таблица! Укоротите в меню что-нибудь...", STOP_ICON);
	}
	FreePool((VOID*)newSmbiosTable.Raw);	
	
	// there is no need to keep all tables in numeric order. It is not needed
// neither by specs nor by AppleSmbios.kext	
}	

VOID FinalizeSmbios() //continue
{
	EFI_PEI_HOB_POINTERS	GuidHob;
	EFI_PEI_HOB_POINTERS	HobStart;
	EFI_PHYSICAL_ADDRESS    *Table;
	UINTN					TableLength;

	// Get Hob List
	HobStart.Raw = GetHobList ();

	// Iteratively add Smbios Table to EFI System Table
	for (Index = 0; Index < sizeof (gTableGuidArray) / sizeof (*gTableGuidArray); ++Index) {
		GuidHob.Raw = GetNextGuidHob (gTableGuidArray[Index], HobStart.Raw);
//		DBG("NextGuidHob = %p\n", GuidHob.Raw);
		if (GuidHob.Raw != NULL) {
			Table = GET_GUID_HOB_DATA (GuidHob.Guid);
			TableLength = GET_GUID_HOB_DATA_SIZE (GuidHob);
//			DBG("Table in HOB: *%p -> %p with size %x\n", Table, *Table, TableLength);

			if (Table != NULL) {
				//
				SmbiosEpsNew->TableLength = (UINT16)((UINT32)(UINTN)Current - (UINT32)(UINTN)Smbios);
				SmbiosEpsNew->NumberOfSmbiosStructures = NumberOfRecords;
				SmbiosEpsNew->MaxStructureSize = MaxStructureSize;
				SmbiosEpsNew->IntermediateChecksum = 0;
				SmbiosEpsNew->IntermediateChecksum = (UINT8)(256 - Checksum8((UINT8*)SmbiosEpsNew + 0x10, 																	SmbiosEpsNew->EntryPointLength - 0x10));
				SmbiosEpsNew->EntryPointStructureChecksum = 0;
				SmbiosEpsNew->EntryPointStructureChecksum = (UINT8)(256 - Checksum8((UINT8*)SmbiosEpsNew, SmbiosEpsNew->EntryPointLength));
				DBG("SmbiosEpsNew->EntryPointLength = %d\n", SmbiosEpsNew->EntryPointLength);
				DBG("DMI checksum = %d\n", Checksum8((UINT8*)SmbiosEpsNew, SmbiosEpsNew->EntryPointLength));
				//*Table = (UINT32)(UINTN)SmbiosEpsNew;
				gBootServices->InstallConfigurationTable (&gEfiSmbiosTableGuid, (VOID*)SmbiosEpsNew);
				*Table = (UINT32)(UINTN)SmbiosEpsNew;
				gSystemTable->Hdr.CRC32 = 0;
				gBootServices->CalculateCrc32 ((UINT8 *) &gSystemTable->Hdr, 
											   gSystemTable->Hdr.HeaderSize, &gSystemTable->Hdr.CRC32);	
				
			//	if(AsciiStrStr(gSettingsFromMenu.Debug,"y") || AsciiStrStr(gSettingsFromMenu.Debug,"Y")) {
				if (DEBUG_SMBIOS == 2) {
					DBG("Table:				0x%08x\n", Table);
					DBG("*Table:			0x%08x\n", Table);
					DBG("SmbiosEPS:      	0x%08x\n", SmbiosEpsNew);					
					DBG("SmbiosTables:		0x%08x\n", SmbiosEpsNew->TableAddress);
					WaitForKeyPress("press [ESC]/[RETURN] to continue...");
				} else {
					DBG("SmbiosEPS:      	0x%p\n", SmbiosEpsNew);
				}
						
			} else {
				CreateDesktop(TRUE);
				SHOW_ON_ERROR(EFI_NOT_FOUND,"There were errors Converting SMBIOS System Table!",
							  L"Ошибка конвертирования SMBIOS!", STOP_ICON);			
			}
		}
	}	
	return;
}
