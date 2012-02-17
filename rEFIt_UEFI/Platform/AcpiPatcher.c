/**
initial concept of DSDT patching by mackerintel
 
Re-Work by Slice 2011.

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

#include "Platform.h"

#define DEBUG_ACPI 1

#if DEBUG_ACPI == 2
#define DBG(x...) AsciiPrint(x)
#elif DEBUG_ACPI == 1
#define DBG(x...) MsgLog(x)
#else
#define DBG(x...)
#endif

#define offsetof(st, m) \
((UINTN) ( (UINT8 *)&((st *)(0))->m - (UINT8 *)0 ))

#define HPET_SIGN        SIGNATURE_32('H','P','E','T')
#define HPET_OEM_ID        { 'A', 'P', 'P', 'L', 'E' }
#define HPET_OEM_TABLE_ID  { 'A', 'p', 'p', 'l', 'e', '0', '0' }
#define HPET_CREATOR_ID    { 'i', 'B', 'O', 'O', 'T' }

#define NUM_TABLES 12
CHAR16* ACPInames[NUM_TABLES] = {
	L"DSDT.aml",
	L"SSDT.aml",
	L"SSDT-1.aml",
	L"SSDT-2.aml",
	L"SSDT-3.aml",
	L"SSDT-4.aml",
	L"SSDT-5.aml",
	L"SSDT-6.aml",
	L"SSDT-7.aml",
	L"APIC.aml",
	L"HPET.aml",
	L"MCFG.aml"
};

EFI_PHYSICAL_ADDRESS        *Table;

UINT8 pmBlock[] = {
	
	/*0070: 0xA5, 0x84, 0x00, 0x00,*/ 0x01, 0x08, 0x00, 0x01, 0xF9, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
	/*0080:*/ 0x06, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x67, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x6F, 0xBF,  
	/*0090:*/ 0x00, 0x00, 0x00, 0x00, 0x01, 0x20, 0x00, 0x03, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	/*00A0:*/ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x02, 
	/*00B0:*/ 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
	/*00C0:*/ 0x00, 0x00, 0x00, 0x00, 0x01, 0x08, 0x00, 0x00, 0x50, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
	/*00D0:*/ 0x01, 0x20, 0x00, 0x03, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x04,  
	/*00E0:*/ 0x20, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,  
	/*00F0:*/ 0x00, 0x00, 0x00, 0x00                                      

};

VOID *
FindAcpiRsdPtr (VOID)
{
	UINTN                           Address;
	UINTN                           Index;
	
	//
	// First Seach 0x0e0000 - 0x0fffff for RSD Ptr
	//
	for (Address = 0xe0000; Address < 0xfffff; Address += 0x10) {
		if (*(UINT64 *)(Address) == EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER_SIGNATURE) {
			return (VOID *)Address;
		}
	}
	
	//
	// Search EBDA
	//
	
	Address = (*(UINT16 *)(UINTN)(EBDA_BASE_ADDRESS)) << 4;
	for (Index = 0; Index < 0x400 ; Index += 16) {
		if (*(UINT64 *)(Address + Index) == EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER_SIGNATURE) {
			return (VOID *)Address;
		}
	}
	return NULL;
}

/*########################################################################################
Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Routine Description:
  Convert RSDP of ACPI Table if its location is lower than Address:0x100000
  Assumption here:
   As in legacy Bios, ACPI table is required to place in E/F Seg, 
   So here we just check if the range is E/F seg, 
   and if Not, assume the Memory type is EfiACPIReclaimMemory/EfiACPIMemoryNVS

Arguments:
  TableLen  - Acpi RSDP length
  Table     - pointer to the table  

Returns:
  EFI_SUCEESS - Convert Table successfully
  Other       - Failed

##########################################################################################*/
EFI_STATUS ConvertAcpiTable (IN UINTN TableLen,IN OUT VOID **Table)
{
	VOID                  *AcpiTableOri;
	VOID                  *AcpiTableNew;
	EFI_STATUS            Status;
	EFI_PHYSICAL_ADDRESS  BufferPtr;


	AcpiTableOri    =  (VOID *)(UINTN)(*(UINT64*)(*Table));
	if (((UINTN)AcpiTableOri < 0x100000) && ((UINTN)AcpiTableOri > 0xE0000)) {
		BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS;
		Status = gBS->AllocatePages (
						AllocateMaxAddress,
						EfiACPIMemoryNVS, // EfiACPIReclaimMemory, //
						EFI_SIZE_TO_PAGES(TableLen),
						&BufferPtr
						);
		ASSERT_EFI_ERROR (Status);
		AcpiTableNew = (VOID *)(UINTN)BufferPtr;
		CopyMem (AcpiTableNew, AcpiTableOri, TableLen);
	} else {
		AcpiTableNew = AcpiTableOri;
	}

	// Change configuration table Pointer
	*Table = AcpiTableNew;
  
	return EFI_SUCCESS;
}

UINT8 Checksum8(VOID * startPtr, UINT32 len)
{
	UINT8	Value = 0;
	UINT8	*ptr=(UINT8 *)startPtr;
	UINT32	i = 0;
	for(i = 0; i < len; i++)
		Value += *ptr++;

	return Value;
}

UINT32* ScanRSDT (RSDT_TABLE *Rsdt, UINT32 Signature) 
{
	EFI_ACPI_DESCRIPTION_HEADER     *Table;
	UINTN							Index;
	UINT32							EntryCount;
	UINT32							*EntryPtr;

	EntryCount = (Rsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT32);

	EntryPtr = &Rsdt->Entry;
	for (Index = 0; Index < EntryCount; Index++, EntryPtr++) {
		Table = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(*EntryPtr));
		if (Table->Signature == Signature) {
			return EntryPtr; //point to table entry
		}
	}
	return NULL;
}

UINT64* ScanXSDT (XSDT_TABLE *Xsdt, UINT32 Signature)
{
	EFI_ACPI_DESCRIPTION_HEADER		*Table;
	UINTN							Index;
	UINT32							EntryCount;
	UINT64							*BasePtr;
	UINT64							Entry64;

	EntryCount = (Xsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);
	BasePtr = (UINT64*)(&(Xsdt->Entry));
	for (Index = 0; Index < EntryCount; Index ++, BasePtr++) 
	{
		CopyMem (&Entry64, (VOID*)BasePtr, sizeof(UINT64)); //value from BasePtr->
		Table = (EFI_ACPI_DESCRIPTION_HEADER *)((UINTN)(Entry64));
		if (Table->Signature==Signature) 
		{
			return BasePtr; //pointer to the table entry
		}
	}
	return NULL;
}

EFI_STATUS PatchACPI(IN EFI_FILE *RootDir)
{
	EFI_STATUS										Status = EFI_SUCCESS;
	UINTN                         Index;
	RSDT_TABLE										*Rsdt = NULL;
	XSDT_TABLE										*Xsdt = NULL;	
	EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER	*RsdPointer;
	EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE		*FadtPointer = NULL;	
	EFI_ACPI_4_0_FIXED_ACPI_DESCRIPTION_TABLE		*newFadt	 = NULL;
	EFI_ACPI_HIGH_PRECISION_EVENT_TIMER_TABLE_HEADER	*Hpet    = NULL;
	EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE	*Facs = NULL;
  //	EFI_GUID										*gTableGuidArray[] = {&gEfiAcpi20TableGuid, &gEfiAcpi10TableGuid};
  //	EFI_PEI_HOB_POINTERS							GuidHob;
  //	EFI_PEI_HOB_POINTERS							HobStart;
//	EFI_DEVICE_PATH_PROTOCOL*	PathBooter = NULL;
//	EFI_DEVICE_PATH_PROTOCOL*	FilePath;
//	EFI_HANDLE					FileSystemHandle;
	EFI_PHYSICAL_ADDRESS		dsdt = EFI_SYSTEM_TABLE_MAX_ADDRESS; //0xFE000000;
	EFI_PHYSICAL_ADDRESS		BufferPtr;
	UINT8*						buffer = NULL;
	UINTN             bufferLen = 0;
	CHAR16*						PathPatched   = L"\\EFI\\acpi\\patched\\DSDT.aml";
	CHAR16*						PathDsdt      = L"\\DSDT.aml";
  CHAR16*						PathDsdtMini  = L"\\EFI\\acpi\\mini\\DSDT.aml";
//	CHAR16*						path = NULL;
	UINT32* rf = NULL;
	UINT64* xf = NULL;
  UINT64 XDsdt; //save values if present
  UINT64 XFirmwareCtrl;
  
	
	//Slice - I want to begin from BIOS ACPI tables like with SMBIOS
	RsdPointer = (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER*)FindAcpiRsdPtr();
	DBG("Found RsdPtr in BIOS: %p\n", RsdPointer);
  
  //Slice - some tricks to do with Bios Acpi Tables
  Rsdt = (RSDT_TABLE*)(UINTN)(RsdPointer->RsdtAddress);
  if (Rsdt == NULL || Rsdt->Header.Signature != EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
    Xsdt = (XSDT_TABLE *)(UINTN)(RsdPointer->XsdtAddress);
  }
  if (Rsdt) {
    FadtPointer = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE*)(UINTN)(Rsdt->Entry);
  } else if (Xsdt) {
    FadtPointer = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE*)(UINTN)(Xsdt->Entry);
  }
  DBG("Found FADT in BIOS: %p\n", FadtPointer);
  Facs = (EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE*)(UINTN)(FadtPointer->FirmwareCtrl);
  DBG("Found FACS in BIOS: %p\n", Facs);
  
#if 0	  //Slice - this codes reserved for a future
	if (0) { //RsdPointer) {
		if (((EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER *)RsdPointer)->Reserved == 0x00){
			
			// If Acpi 1.0 Table, then RSDP structure doesn't contain Length field, use structure size
			AcpiTableLen = sizeof (EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER);
		} else if (((EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER *)RsdPointer)->Reserved >= 0x02){
			
			// If Acpi 2.0 or later, use RSDP Length field.
			AcpiTableLen = RsdPointer->Length;
		} else {
			
			// Invalid Acpi Version, return
			return EFI_UNSUPPORTED;
		}
		Status = ConvertAcpiTable (AcpiTableLen, (VOID**)&RsdPointer);
		//Now install the new table
		gBS->InstallConfigurationTable (&gEfiAcpiTableGuid, (VOID*)RsdPointer);
		DBG("Converted RsdPtr 0x%p\n", RsdPointer);
		gST->Hdr.CRC32 = 0;
		gBS->CalculateCrc32 ((UINT8 *) &gST->Hdr, 
                         gST->Hdr.HeaderSize, &gST->Hdr.CRC32);	
		DBG("AcpiTableLen %x\n", AcpiTableLen);
	} else
#else	
	{
		//try to find in SystemTable
		for(Index = 0; Index < gST->NumberOfTableEntries; Index++)
		{
			if(CompareGuid (&gST->ConfigurationTable[Index].VendorGuid, &gEfiAcpi20TableGuid))
			{
				// Acpi 2.0
				RsdPointer = (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER*)gST->ConfigurationTable[Index].VendorTable;
				break;
			}
			else if(CompareGuid (&gST->ConfigurationTable[Index].VendorGuid, &gEfiAcpi10TableGuid))
			{
				// Acpi 1.0 - RSDT only
				RsdPointer = (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER*)gST->ConfigurationTable[Index].VendorTable;
				continue;
			}
		}
	}
#endif	
  
	if (!RsdPointer) {
		return EFI_UNSUPPORTED;
	}
	Rsdt = (RSDT_TABLE*)(UINTN)RsdPointer->RsdtAddress;
	DBG("RSDT 0x%p\n", Rsdt);
	rf = ScanRSDT(Rsdt, EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE);
	if(rf)
		FadtPointer = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE*)(UINTN)(*rf);
    
    Xsdt = NULL;			
    if (RsdPointer->Revision >=2 && (RsdPointer->XsdtAddress < (UINT64)(UINTN)-1))
    {
      Xsdt = (XSDT_TABLE*)(UINTN)RsdPointer->XsdtAddress;
      DBG("XSDT 0x%p\n", Xsdt);
      xf = ScanXSDT(Xsdt, EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE);
      if(xf)
        FadtPointer = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE*)(UINTN)(*xf);
    }
	
	if(!xf){
	 	Print(L"Error! Xsdt is not found!!!\n");
	 	//We should make here ACPI20 RSDP with all needed subtables based on ACPI10
	}
  
  //	DBG("FADT pointer = %x\n", (UINTN)FadtPointer);
	if(!FadtPointer)
	{
    return EFI_NOT_FOUND;
  }
  //Slice - then we do FADT patch no matter if we don't have DSDT.aml
  BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS;
  Status=gBS->AllocatePages(AllocateMaxAddress,EfiACPIReclaimMemory, 1, &BufferPtr);		
  if(!EFI_ERROR(Status))
  {
    newFadt = (EFI_ACPI_4_0_FIXED_ACPI_DESCRIPTION_TABLE*)(UINTN)BufferPtr;
    UINT32 oldLength = ((EFI_ACPI_DESCRIPTION_HEADER*)FadtPointer)->Length;
    //		MsgLog("old FADT length=%x\n", oldLength);
    CopyMem((UINT8*)newFadt, (UINT8*)FadtPointer, oldLength); //old data
    newFadt->Header.Length = 0xF4; 				
    CopyMem((UINT8*)newFadt->Header.OemId, (UINT8*)BiosVendor, 6);
    newFadt->Header.Revision = EFI_ACPI_4_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION;
    newFadt->Reserved0 = 0; //ACPIspec said it should be 0, while 1 is possible, but no more
    
    if (gSettings.smartUPS==TRUE) {
      newFadt->PreferredPmProfile = 3;
    } else {
      newFadt->PreferredPmProfile = gMobile?2:1; //as calculated before
    }
    newFadt->Flags |= 0x400; //Reset Register Supported
    XDsdt = newFadt->XDsdt; //save values if present
    XFirmwareCtrl = newFadt->XFirmwareCtrl;
    CopyMem((UINT8*)&newFadt->ResetReg, pmBlock, 0x80);
    //but these common values are not specific, so adjust
    //ACPIspec said that if Xdsdt !=0 then Dsdt must be =0. But real Mac no! Both values present
    if (newFadt->Dsdt) {
      newFadt->XDsdt = (UINT64)(newFadt->Dsdt);
    } else if (XDsdt) {
      newFadt->Dsdt = (UINT32)XDsdt;
    }
    if (Facs) newFadt->FirmwareCtrl = (UINT32)(UINTN)Facs;
    else MsgLog("No FACS table ?!\n");
    if (newFadt->FirmwareCtrl) {
      newFadt->XFirmwareCtrl = (UINT64)(newFadt->FirmwareCtrl);
    } else if (newFadt->XFirmwareCtrl) {
      newFadt->FirmwareCtrl = (UINT32)XFirmwareCtrl;
    }
    //patch for FACS included here
    Facs->Version = EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_VERSION;
    //
    newFadt->ResetReg.Address    = (UINT64)(gResetAddress); //UINT16->UINT64
    newFadt->ResetValue			 = gResetValue; //UINT8
    newFadt->XPm1aEvtBlk.Address = (UINT64)(newFadt->Pm1aEvtBlk);
    newFadt->XPm1bEvtBlk.Address = (UINT64)(newFadt->Pm1bEvtBlk);
    newFadt->XPm1aCntBlk.Address = (UINT64)(newFadt->Pm1aCntBlk);
    newFadt->XPm1bCntBlk.Address = (UINT64)(newFadt->Pm1bCntBlk);
    newFadt->XPm2CntBlk.Address  = (UINT64)(newFadt->Pm2CntBlk);
    newFadt->XPmTmrBlk.Address   = (UINT64)(newFadt->PmTmrBlk);
    newFadt->XGpe0Blk.Address    = (UINT64)(newFadt->Gpe0Blk);
    newFadt->XGpe1Blk.Address    = (UINT64)(newFadt->Gpe1Blk);
    FadtPointer = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE*)newFadt;
    //We are sure that Fadt is the first entry in RSDT/XSDT table
    if (Rsdt!=NULL) {
      Rsdt->Entry = (UINT32)(UINTN)newFadt;
      Rsdt->Header.Checksum = 0;
      Rsdt->Header.Checksum = (UINT8)(256-Checksum8((CHAR8*)Rsdt, Rsdt->Header.Length));
    }
    if (Xsdt!=NULL) {
      Xsdt->Entry = (UINT64)((UINT32)(UINTN)newFadt);
      Xsdt->Header.Checksum = 0;
      Xsdt->Header.Checksum = (UINT8)(256-Checksum8((CHAR8*)Xsdt, Xsdt->Header.Length));
    }
    
  }
  
  //HPET creation
  BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS;
  Status=gBS->AllocatePages(AllocateMaxAddress, EfiACPIReclaimMemory, 1, &BufferPtr);
  if(!EFI_ERROR(Status))
  {
    UINT8	oemID[6]        = HPET_OEM_ID;
    UINT64	oemTableID[]  = HPET_OEM_TABLE_ID;
    UINT32	creatorID[]   = HPET_CREATOR_ID;
    xf = ScanXSDT(Xsdt, HPET_SIGN);
    if(!xf) { //we want to make the new table if OEM is not found
      
      Hpet = (EFI_ACPI_HIGH_PRECISION_EVENT_TIMER_TABLE_HEADER*)(UINTN)BufferPtr;
      Hpet->Header.Signature = EFI_ACPI_3_0_HIGH_PRECISION_EVENT_TIMER_TABLE_SIGNATURE;
      Hpet->Header.Length = sizeof(EFI_ACPI_HIGH_PRECISION_EVENT_TIMER_TABLE_HEADER);
      Hpet->Header.Revision = EFI_ACPI_HIGH_PRECISION_EVENT_TIMER_TABLE_REVISION;
      CopyMem(&Hpet->Header.OemId, oemID, 6);
      CopyMem(&Hpet->Header.OemTableId, oemTableID, sizeof(oemTableID));
      Hpet->Header.OemRevision = 0x00000001;
      CopyMem(&Hpet->Header.CreatorId, creatorID, sizeof(creatorID));
      Hpet->EventTimerBlockId = 0x8086A201; // we should remember LPC VendorID to place here
      Hpet->BaseAddressLower32Bit.AddressSpaceId = EFI_ACPI_2_0_SYSTEM_IO;
      Hpet->BaseAddressLower32Bit.RegisterBitWidth = 0x40; //64bit
      Hpet->BaseAddressLower32Bit.RegisterBitOffset = 0x00; 
      Hpet->BaseAddressLower32Bit.Address = 0xFED00000; //Physical Addr.
      Hpet->HpetNumber = 0;
      Hpet->MainCounterMinimumClockTickInPeriodicMode = 0x0080; 
      Hpet->PageProtectionAndOemAttribute = EFI_ACPI_64KB_PAGE_PROTECTION; //Flags |= EFI_ACPI_4KB_PAGE_PROTECTION , EFI_ACPI_64KB_PAGE_PROTECTION
      // verify checksum
      Hpet->Header.Checksum = 0;
      Hpet->Header.Checksum = (UINT8)(256-Checksum8((CHAR8*)Hpet,Hpet->Header.Length));
      
      //then we have to install new table into Xsdt
      if (Xsdt!=NULL) {
        //we have no such table. Add a new entry into Xsdt
        UINT64	EntryCount;
        
        EntryCount = (Xsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);
        xf = (UINT64*)(&(Xsdt->Entry)) + EntryCount;
        Xsdt->Header.Length += sizeof(UINT64);				
        *xf = (UINT64)(UINTN)Hpet;
        DBG("HPET placed into XSDT: %lx\n", *xf);
        Xsdt->Header.Checksum = 0;
        Xsdt->Header.Checksum = (UINT8)(256-Checksum8((CHAR8*)Xsdt, Xsdt->Header.Length));
      }
    }
  }
  
  //DSDT finding
  Status = EFI_NOT_FOUND;
  if (gSettings.UseDSDTmini==TRUE) {
    if (FileExists(SelfRootDir, PathDsdtMini)) {
      Status = egLoadFile(SelfRootDir, PathDsdtMini, &buffer, &bufferLen);
    }
  }
  if (EFI_ERROR(Status) && FileExists(RootDir, PathDsdt)) {
    Status = egLoadFile(RootDir, PathDsdt, &buffer, &bufferLen);
  }
  if (EFI_ERROR(Status) && FileExists(SelfRootDir, PathPatched)) {
    Status = egLoadFile(SelfRootDir, PathPatched, &buffer, &bufferLen);
  }
  //apply DSDT
  if (!EFI_ERROR(Status)) {
    Status = gBS->AllocatePages (
                                 AllocateMaxAddress,
                                 EfiACPIReclaimMemory,
                                 EFI_SIZE_TO_PAGES(bufferLen),
                                 &dsdt
                                 );
    
    //if success insert dsdt pointer into ACPI tables
    if(!EFI_ERROR(Status))
    {
      CopyMem((VOID*)(UINTN)dsdt, buffer, bufferLen);
      
      FadtPointer->Dsdt  = (UINT32)dsdt;
      FadtPointer->XDsdt = dsdt;
      // verify checksum
      FadtPointer->Header.Checksum = 0;
      FadtPointer->Header.Checksum = (UINT8)(256-Checksum8((CHAR8*)FadtPointer,FadtPointer->Header.Length));
      if (Rsdt) {
        Rsdt->Header.Checksum = 0;
        Rsdt->Header.Checksum = (UINT8)(256-Checksum8((CHAR8*)Rsdt, Rsdt->Header.Length));
      }
      if (Xsdt) {
        Xsdt->Header.Checksum = 0;
        Xsdt->Header.Checksum = (UINT8)(256-Checksum8((CHAR8*)Xsdt, Xsdt->Header.Length));
      }
    }
  } 
  
  //find other ACPI tables
  
  return Status;
}
