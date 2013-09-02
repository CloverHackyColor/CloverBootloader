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

#include "StateGenerator.h"

//#define TEST 1

#define offsetof(st, m) \
((UINTN) ( (UINT8 *)&((st *)(0))->m - (UINT8 *)0 ))

#define XXXX_SIGN        SIGNATURE_32('X','X','X','X')
#define HPET_SIGN        SIGNATURE_32('H','P','E','T')
#define APIC_SIGN        SIGNATURE_32('A','P','I','C')
#define MCFG_SIGN        SIGNATURE_32('M','C','F','G')
#define ECDT_SIGN        SIGNATURE_32('E','C','D','T')
#define DMAR_SIGN        SIGNATURE_32('D','M','A','R')
#define BGRT_SIGN        SIGNATURE_32('B','G','R','T')
#define APPLE_OEM_ID        { 'A', 'P', 'P', 'L', 'E', ' ' }
#define APPLE_OEM_TABLE_ID  { 'A', 'p', 'p', 'l', 'e', '0', '0', ' ' }
#define APPLE_CREATOR_ID    { 'L', 'o', 'k', 'i' }
//#define EFI_ACPI_4_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE  SIGNATURE_32('S', 'S', 'D', 'T')

CONST CHAR8	oemID[6]       = APPLE_OEM_ID;
CONST CHAR8	oemTableID[8]  = APPLE_OEM_TABLE_ID;
CONST CHAR8	creatorID[4]   = APPLE_CREATOR_ID;

//Global pointers
RSDT_TABLE    *Rsdt = NULL;
XSDT_TABLE    *Xsdt = NULL;

//CHAR8*   orgBiosDsdt;
UINT64    BiosDsdt;
UINT32    BiosDsdtLen;
UINT8     acpi_cpu_count;
CHAR8*    acpi_cpu_name[32];
CHAR8*    OSVersion;

//-----------------------------------


#define NUM_TABLES 31
CHAR16* ACPInames[NUM_TABLES] = {
    L"SSDT.aml",
    L"SSDT-0.aml",
    L"SSDT-1.aml",
    L"SSDT-2.aml",
    L"SSDT-3.aml",
    L"SSDT-4.aml",
    L"SSDT-5.aml",
    L"SSDT-6.aml",
    L"SSDT-7.aml",
    L"SSDT-8.aml",
    L"SSDT-9.aml",
    L"SSDT-10.aml",
    L"SSDT-11.aml",
    L"SSDT-12.aml",
    L"SSDT-13.aml",
    L"SSDT-14.aml",
    L"SSDT-15.aml",
    L"SSDT-16.aml",
    L"SSDT-17.aml",
    L"SSDT-18.aml",
    L"SSDT-19.aml",
    L"APIC.aml",
    L"BOOT.aml",
    L"DMAR.aml",
    L"ECDT.aml",
    L"HPET.aml",
    L"MCFG.aml",
    L"SLIC.aml",
    L"SLIT.aml",
    L"SRAT.aml",
    L"UEFI.aml"
};

//EFI_PHYSICAL_ADDRESS        *Table;

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

// Slice: Signature compare function
BOOLEAN tableSign(CHAR8 *table, CONST CHAR8 *sgn)
{
  INTN i;
  for (i=0; i<4; i++) {
    if ((table[i] &~0x20) != (sgn[i] &~0x20)) {
      return FALSE;
    }
  }
  return TRUE;
}

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
EFI_STATUS ConvertAcpiTable (IN UINTN TableLen,IN OUT VOID **TheTable)
{
	VOID                  *AcpiTableOri;
	VOID                  *AcpiTableNew;
	EFI_STATUS            Status;
	EFI_PHYSICAL_ADDRESS  BufferPtr;


	AcpiTableOri    =  (VOID *)(UINTN)(*(UINT64*)(*TheTable));
	if (((UINTN)AcpiTableOri < 0x100000) && ((UINTN)AcpiTableOri > 0xE0000)) {
		BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS;
		Status = gBS->AllocatePages (
						AllocateMaxAddress,
						EfiACPIMemoryNVS, // EfiACPIReclaimMemory, //
						EFI_SIZE_TO_PAGES(TableLen),
						&BufferPtr
						);
//		ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR(Status)) {
      return Status;
    }
		AcpiTableNew = (VOID *)(UINTN)BufferPtr;
		CopyMem (AcpiTableNew, AcpiTableOri, TableLen);
	} else {
		AcpiTableNew = AcpiTableOri;
	}

	// Change configuration table Pointer
	*TheTable = AcpiTableNew;
  
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

UINT32* ScanRSDT (UINT32 Signature) 
{
	EFI_ACPI_DESCRIPTION_HEADER     *TableEntry;
	UINTN                           Index;
	UINT32                          EntryCount;
	UINT32                          *EntryPtr;
  if (!Rsdt) {
    return NULL;
  }

	EntryCount = (Rsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT32);

	EntryPtr = &Rsdt->Entry;
	for (Index = 0; Index < EntryCount; Index++, EntryPtr++) {
		TableEntry = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(*EntryPtr));
		if (TableEntry->Signature == Signature) {
			return EntryPtr; //point to TableEntry entry
		}
	}
	return NULL;
}

UINT64* ScanXSDT (UINT32 Signature)
{
	EFI_ACPI_DESCRIPTION_HEADER		*TableEntry;
	UINTN							Index;
	UINT32							EntryCount;
	CHAR8							*BasePtr;
	UINT64							Entry64;

	EntryCount = (Xsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);
	BasePtr = (CHAR8*)(&(Xsdt->Entry));
	for (Index = 0; Index < EntryCount; Index ++, BasePtr+=sizeof(UINT64)) 
	{
		CopyMem (&Entry64, (VOID*)BasePtr, sizeof(UINT64)); //value from BasePtr->
		TableEntry = (EFI_ACPI_DESCRIPTION_HEADER *)((UINTN)(Entry64));
		if (TableEntry->Signature==Signature) 
		{
			return (UINT64 *)BasePtr; //pointer to the TableEntry entry
		}
	}
	return NULL;
}

UINT32* ScanRSDTId (UINT64 id)
{
  EFI_ACPI_DESCRIPTION_HEADER     *TableEntry;
  UINTN                           Index;
  UINT32                          EntryCount;
  UINT32                          *EntryPtr;
  if (!Rsdt) {
    return NULL;
  }

  EntryCount = (Rsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT32);

  EntryPtr = &Rsdt->Entry;
  for (Index = 0; Index < EntryCount; Index++, EntryPtr++) {
    TableEntry = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(*EntryPtr));
    if (TableEntry->OemTableId==id) 
    {
      return EntryPtr; //point to TableEntry entry
    }
  }
  return NULL;
}

UINT64* ScanXSDTId (UINT64 id)
{
	EFI_ACPI_DESCRIPTION_HEADER		*TableEntry;
	UINTN							Index;
	UINT32							EntryCount;
	CHAR8							*BasePtr;
	UINT64							Entry64;

	EntryCount = (Xsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);
	BasePtr = (CHAR8*)(&(Xsdt->Entry));
	for (Index = 0; Index < EntryCount; Index ++, BasePtr+=sizeof(UINT64)) 
	{
		CopyMem (&Entry64, (VOID*)BasePtr, sizeof(UINT64)); //value from BasePtr->
		TableEntry = (EFI_ACPI_DESCRIPTION_HEADER *)((UINTN)(Entry64));
		if (TableEntry->OemTableId==id) 
		{
			return (UINT64 *)BasePtr; //pointer to the TableEntry entry
		}
	}
	return NULL;
}

VOID DropTableFromRSDT (UINT32 Signature) 
{
	EFI_ACPI_DESCRIPTION_HEADER     *TableEntry;
	UINTN               Index, Index2;
	UINT32							EntryCount;
	UINT32							*EntryPtr, *Ptr, *Ptr2;
  CHAR8 sign[5];
  CHAR8 OTID[9];
  BOOLEAN 			DoubleZero = FALSE;
  
  if (!Rsdt) {
    return;
  }
  // Если адрес RSDT < адреса XSDT и хвост RSDT наползает на XSDT, то подрезаем хвост RSDT до начала XSDT
  if (((UINTN)Rsdt < (UINTN)Xsdt) && (((UINTN)Rsdt + Rsdt->Header.Length) > (UINTN)Xsdt)) {
    Rsdt->Header.Length = ((UINTN)Xsdt - (UINTN)Rsdt) & ~3;
    DBG("Cropped Rsdt->Header.Length=%d\n", Rsdt->Header.Length);
  }
  
	EntryCount = (Rsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT32);
	if (EntryCount > 100) EntryCount = 100; //it's enough
  DBG("Drop tables from Rsdt, count=%d\n", EntryCount); 
	EntryPtr = &Rsdt->Entry;
	for (Index = 0; Index < EntryCount; Index++, EntryPtr++) {
    if (*EntryPtr == 0) {
      if (DoubleZero) {
        Rsdt->Header.Length = (UINT32)(sizeof(UINT32) * Index + sizeof(EFI_ACPI_DESCRIPTION_HEADER));
        DBG("DoubleZero in RSDT table\n");
        break;
      }
      DBG("First zero in RSDT table\n");
      DoubleZero = TRUE;
      Rsdt->Header.Length -= sizeof(UINT32);
      continue; //avoid zero field
    }
    DoubleZero = FALSE;
	  TableEntry = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(*EntryPtr));
    CopyMem((CHAR8*)&sign, (CHAR8*)&TableEntry->Signature, 4);
    sign[4] = 0;
    CopyMem((CHAR8*)&OTID, (CHAR8*)&TableEntry->OemTableId, 8);
    OTID[8] = 0;
    if (!GlobalConfig.DebugLog) {
      DBG(" Found table: %a  %a\n", sign, OTID);
    }
	  if (TableEntry->Signature != Signature) {
			continue;
    }
    if (!GlobalConfig.DebugLog) {
      DBG(" ... dropped\n");
    }
    Ptr = EntryPtr;
    Ptr2 = Ptr + 1;
    for (Index2 = Index; Index2 < EntryCount-1; Index2++) {
      *Ptr++ = *Ptr2++;
    }
  //  *Ptr = 0; //end of table
    EntryPtr--; //SunKi
    Rsdt->Header.Length -= sizeof(UINT32);
	}
  DBG("corrected RSDT length=%d\n", Rsdt->Header.Length);
}

VOID DropTableFromXSDT (UINT32 Signature) 
{
  EFI_STATUS                      Status = EFI_SUCCESS;
	EFI_ACPI_DESCRIPTION_HEADER     *TableEntry;
  EFI_PHYSICAL_ADDRESS            ssdt = EFI_SYSTEM_TABLE_MAX_ADDRESS;
	UINTN                           Index, Index2;
	UINT32                          EntryCount;
	CHAR8                           *BasePtr, *Ptr, *Ptr2;
	UINT64                          Entry64;
  CHAR8                           sign[5];
  CHAR8                           OTID[9];
  BOOLEAN                         DoubleZero = FALSE;
  BOOLEAN                         WillDrop;
  UINT32                          i, SsdtLen;
  
	EntryCount = (Xsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);
  DBG("Drop tables from Xsdt, count=%d\n", EntryCount); 
  if (EntryCount > 50) {
    DBG("BUG! Too many XSDT entries \n");
    EntryCount = 50;
  }
	BasePtr = (CHAR8*)(UINTN)(&(Xsdt->Entry));
	for (Index = 0; Index < EntryCount; Index++, BasePtr += sizeof(UINT64)) {
    if (ReadUnaligned64((CONST UINT64*)BasePtr) == 0) {
      if (DoubleZero) {
        Xsdt->Header.Length = (UINT32)(sizeof(UINT64) * Index + sizeof(EFI_ACPI_DESCRIPTION_HEADER));
        DBG("DoubleZero in XSDT table\n");
        break;
      }
      DBG("First zero in XSDT table\n");
      DoubleZero = TRUE;
      Xsdt->Header.Length -= sizeof(UINT64);
      continue; //avoid zero field
    }
    DoubleZero = FALSE;
    CopyMem (&Entry64, (VOID*)BasePtr, sizeof(UINT64)); //value from BasePtr->
	  TableEntry = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(Entry64));
    CopyMem((CHAR8*)&sign, (CHAR8*)&TableEntry->Signature, 4);
    sign[4] = 0;
    CopyMem((CHAR8*)&OTID, (CHAR8*)&TableEntry->OemTableId, 8);
    OTID[8] = 0;
    DBG(" Found table: %a  %a\n", sign, OTID);
	  if (TableEntry->Signature != Signature) {
      continue;
	  }
    WillDrop = TRUE;
    for (i = 0; i < gSettings.KeepSsdtNum; i++) {
      if (AsciiStrCmp(OTID, gSettings.KeepTableId[i]) == 0) {
        WillDrop = FALSE;
        break;
      }
    }
    if (!WillDrop &&
        (TableEntry->Signature == EFI_ACPI_4_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE)) {
      //will patch here
      SsdtLen = TableEntry->Length;
			DBG("SSDT len = 0x%x", SsdtLen);
//			SsdtLen += 4096;
//			DBG(" new len = 0x%x\n", SsdtLen);
			
			ssdt = EFI_SYSTEM_TABLE_MAX_ADDRESS;
			Status = gBS->AllocatePages(AllocateMaxAddress,
                                  EfiACPIReclaimMemory,
                                  EFI_SIZE_TO_PAGES(SsdtLen + 4096),
                                  &ssdt);
			if(EFI_ERROR(Status)) {
        DBG(" ... not patched\n");
        continue;
      }
      Ptr = (CHAR8*)(UINTN)ssdt;
			CopyMem(Ptr, (VOID*)TableEntry, SsdtLen);
        
      for (i = 0; i < gSettings.PatchDsdtNum; i++) {
        SsdtLen = FixAny((UINT8*)(UINTN)ssdt, SsdtLen,
                         gSettings.PatchDsdtFind[i], gSettings.LenToFind[i],
                         gSettings.PatchDsdtReplace[i], gSettings.LenToReplace[i]);
      }
      CopyMem ((VOID*)BasePtr, &ssdt, sizeof(UINT64)); //*BasePtr = ssdt;
      // Finish SSDT patch and resize SSDT Length
      CopyMem (&Ptr[4], &SsdtLen, 4);
      ((EFI_ACPI_DESCRIPTION_HEADER*)Ptr)->Checksum = 0;
      ((EFI_ACPI_DESCRIPTION_HEADER*)Ptr)->Checksum = (UINT8)(256-Checksum8(Ptr, SsdtLen));

      DBG(" ... patched\n");
      continue;
    }
    DBG(" ... dropped\n");
    Ptr = BasePtr;
    Ptr2 = Ptr + sizeof(UINT64);
    for (Index2 = Index; Index2 < EntryCount-1; Index2++) {
      //*Ptr++ = *Ptr2++;
      CopyMem(Ptr, Ptr2, sizeof(UINT64));
      Ptr  += sizeof(UINT64);
      Ptr2 += sizeof(UINT64);
    }
 //   ZeroMem(Ptr, sizeof(UINT64));
    BasePtr -= sizeof(UINT64); //SunKi
    Xsdt->Header.Length -= sizeof(UINT64);
	}	
  DBG("corrected XSDT length=%d\n", Xsdt->Header.Length);
}

VOID DropTableFromRSDTId (UINT64 id) 
{
	EFI_ACPI_DESCRIPTION_HEADER     *TableEntry;
	UINTN               Index, Index2;
	UINT32							EntryCount;
	UINT32							*EntryPtr, *Ptr, *Ptr2;
  CHAR8 sign[5];
  CHAR8 OTID[9];
  BOOLEAN 			DoubleZero = FALSE;
  
  if (!Rsdt) {
    return;
  }
  // Если адрес RSDT < адреса XSDT и хвост RSDT наползает на XSDT, то подрезаем хвост RSDT до начала XSDT
  if (((UINTN)Rsdt < (UINTN)Xsdt) && (((UINTN)Rsdt + Rsdt->Header.Length) > (UINTN)Xsdt)) {
    Rsdt->Header.Length = ((UINTN)Xsdt - (UINTN)Rsdt) & ~3;
    DBG("Cropped Rsdt->Header.Length=%d\n", Rsdt->Header.Length);
  }
  
	EntryCount = (Rsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT32);
	if (EntryCount > 100) EntryCount = 100; //it's enough
  DBG("Drop tables from Rsdt, count=%d\n", EntryCount); 
	EntryPtr = &Rsdt->Entry;
	for (Index = 0; Index < EntryCount; Index++, EntryPtr++) {
    if (*EntryPtr == 0) {
      if (DoubleZero) {
        Rsdt->Header.Length = (UINT32)(sizeof(UINT32) * Index + sizeof(EFI_ACPI_DESCRIPTION_HEADER));
        DBG("DoubleZero in RSDT table\n");
        break;
      }
      DBG("First zero in RSDT table\n");
      DoubleZero = TRUE;
      Rsdt->Header.Length -= sizeof(UINT32);
      continue; //avoid zero field
    }
    DoubleZero = FALSE;
	  TableEntry = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(*EntryPtr));
    CopyMem((CHAR8*)&sign, (CHAR8*)&TableEntry->Signature, 4);
    sign[4] = 0;
    CopyMem((CHAR8*)&OTID, (CHAR8*)&TableEntry->OemTableId, 8);
    OTID[8] = 0;
    if (!GlobalConfig.DebugLog) {
      DBG(" Found table: %a  %a\n", sign, OTID);
    }
	  if (TableEntry->OemTableId != id) {
			continue;
    }
    if (!GlobalConfig.DebugLog) {
      DBG(" ... dropped\n");
    }
    Ptr = EntryPtr;
    Ptr2 = Ptr + 1;
    for (Index2 = Index; Index2 < EntryCount-1; Index2++) {
      *Ptr++ = *Ptr2++;
    }
  //  *Ptr = 0; //end of table
    EntryPtr--; //SunKi
    Rsdt->Header.Length -= sizeof(UINT32);
	}
  DBG("corrected RSDT length=%d\n", Rsdt->Header.Length);
}

VOID DropTableFromXSDTId (UINT64 id) 
{
  EFI_STATUS                      Status = EFI_SUCCESS;
	EFI_ACPI_DESCRIPTION_HEADER     *TableEntry;
  EFI_PHYSICAL_ADDRESS            ssdt = EFI_SYSTEM_TABLE_MAX_ADDRESS;
	UINTN                           Index, Index2;
	UINT32                          EntryCount;
	CHAR8                           *BasePtr, *Ptr, *Ptr2;
	UINT64                          Entry64;
  CHAR8                           sign[5];
  CHAR8                           OTID[9];
  BOOLEAN                         DoubleZero = FALSE;
  BOOLEAN                         WillDrop;
  UINT32                          i, SsdtLen;
  
	EntryCount = (Xsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);
  DBG("Drop tables from Xsdt, count=%d\n", EntryCount); 
  if (EntryCount > 50) {
    DBG("BUG! Too many XSDT entries \n");
    EntryCount = 50;
  }
	BasePtr = (CHAR8*)(UINTN)(&(Xsdt->Entry));
	for (Index = 0; Index < EntryCount; Index++, BasePtr += sizeof(UINT64)) {
    if (ReadUnaligned64((CONST UINT64*)BasePtr) == 0) {
      if (DoubleZero) {
        Xsdt->Header.Length = (UINT32)(sizeof(UINT64) * Index + sizeof(EFI_ACPI_DESCRIPTION_HEADER));
        DBG("DoubleZero in XSDT table\n");
        break;
      }
      DBG("First zero in XSDT table\n");
      DoubleZero = TRUE;
      Xsdt->Header.Length -= sizeof(UINT64);
      continue; //avoid zero field
    }
    DoubleZero = FALSE;
    CopyMem (&Entry64, (VOID*)BasePtr, sizeof(UINT64)); //value from BasePtr->
	  TableEntry = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(Entry64));
    CopyMem((CHAR8*)&sign, (CHAR8*)&TableEntry->Signature, 4);
    sign[4] = 0;
    CopyMem((CHAR8*)&OTID, (CHAR8*)&TableEntry->OemTableId, 8);
    OTID[8] = 0;
    DBG(" Found table: %a  %a\n", sign, OTID);
	  if (TableEntry->OemTableId != id) {
      continue;
	  }
    WillDrop = TRUE;
    for (i = 0; i < gSettings.KeepSsdtNum; i++) {
      if (AsciiStrCmp(OTID, gSettings.KeepTableId[i]) == 0) {
        WillDrop = FALSE;
        break;
      }
    }
    if (!WillDrop &&
        (TableEntry->Signature == EFI_ACPI_4_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE)) {
      //will patch here
      SsdtLen = TableEntry->Length;
			DBG("SSDT len = 0x%x", SsdtLen);
//			SsdtLen += 4096;
//			DBG(" new len = 0x%x\n", SsdtLen);
			
			ssdt = EFI_SYSTEM_TABLE_MAX_ADDRESS;
			Status = gBS->AllocatePages(AllocateMaxAddress,
                                  EfiACPIReclaimMemory,
                                  EFI_SIZE_TO_PAGES(SsdtLen + 4096),
                                  &ssdt);
			if(EFI_ERROR(Status)) {
        DBG(" ... not patched\n");
        continue;
      }
      Ptr = (CHAR8*)(UINTN)ssdt;
			CopyMem(Ptr, (VOID*)TableEntry, SsdtLen);
        
      for (i = 0; i < gSettings.PatchDsdtNum; i++) {
        SsdtLen = FixAny((UINT8*)(UINTN)ssdt, SsdtLen,
                         gSettings.PatchDsdtFind[i], gSettings.LenToFind[i],
                         gSettings.PatchDsdtReplace[i], gSettings.LenToReplace[i]);
      }
      CopyMem ((VOID*)BasePtr, &ssdt, sizeof(UINT64)); //*BasePtr = ssdt;
      // Finish SSDT patch and resize SSDT Length
      CopyMem (&Ptr[4], &SsdtLen, 4);
      ((EFI_ACPI_DESCRIPTION_HEADER*)Ptr)->Checksum = 0;
      ((EFI_ACPI_DESCRIPTION_HEADER*)Ptr)->Checksum = (UINT8)(256-Checksum8(Ptr, SsdtLen));

      DBG(" ... patched\n");
      continue;
    }
    DBG(" ... dropped\n");
    Ptr = BasePtr;
    Ptr2 = Ptr + sizeof(UINT64);
    for (Index2 = Index; Index2 < EntryCount-1; Index2++) {
      //*Ptr++ = *Ptr2++;
      CopyMem(Ptr, Ptr2, sizeof(UINT64));
      Ptr  += sizeof(UINT64);
      Ptr2 += sizeof(UINT64);
    }
 //   ZeroMem(Ptr, sizeof(UINT64));
    BasePtr -= sizeof(UINT64); //SunKi
    Xsdt->Header.Length -= sizeof(UINT64);
	}	
  DBG("corrected XSDT length=%d\n", Xsdt->Header.Length);
}

EFI_STATUS InsertTable(VOID* TableEntry, UINTN Length)
{
  EFI_STATUS		Status = EFI_SUCCESS;
  EFI_PHYSICAL_ADDRESS		BufferPtr  = EFI_SYSTEM_TABLE_MAX_ADDRESS;

  UINT32*       Ptr;
  UINT64*       XPtr;
  
  if (!TableEntry) {
    return EFI_NOT_FOUND;
  }

  Status = gBS->AllocatePages (
                               AllocateMaxAddress,
                               EfiACPIReclaimMemory,
                               EFI_SIZE_TO_PAGES(Length),
                               &BufferPtr
                               );
  //if success insert table pointer into ACPI tables
  if(!EFI_ERROR(Status))
  {
    //      DBG("page is allocated, write SSDT into\n");      
    CopyMem((VOID*)(UINTN)BufferPtr, (VOID*)TableEntry, Length);
    
    //insert into RSDT
    if (Rsdt) {
      Ptr = (UINT32*)((UINTN)Rsdt + Rsdt->Header.Length);
      *Ptr = (UINT32)(UINTN)BufferPtr;
      Rsdt->Header.Length += sizeof(UINT32);
      //       DBG("Rsdt->Length = %d\n", Rsdt->Header.Length);
    }
    //insert into XSDT
    if (Xsdt) {
      XPtr = (UINT64*)((UINTN)Xsdt + Xsdt->Header.Length);
     // *XPtr = (UINT64)(UINTN)BufferPtr;
      WriteUnaligned64(XPtr, (UINT64)BufferPtr);
      Xsdt->Header.Length += sizeof(UINT64);
      //        DBG("Xsdt->Length = %d\n", Xsdt->Header.Length);
    }        
  } 
  
  return Status;
}

/** Saves Buffer of Length to disk as DirName\\FileName. */
EFI_STATUS SaveBufferToDisk(VOID *Buffer, UINTN Length, CHAR16 *DirName, CHAR16 *FileName)
{
	EFI_STATUS		Status;
	
	if (DirName == NULL || FileName == NULL) {
		return EFI_INVALID_PARAMETER;
	}
	
	FileName = PoolPrint(L"%s\\%s", DirName, FileName);
	if (FileName == NULL) {
		return EFI_OUT_OF_RESOURCES;
	}
	
	Status = egSaveFile(SelfRootDir, FileName, Buffer, Length);
	if (EFI_ERROR(Status)) {
		Status = egSaveFile(NULL, FileName, Buffer, Length);
	}
	
	FreePool(FileName);
	
	return Status;
}



//
// Remembering saved tables
//
#define SAVED_TABLES_ALLOC_ENTRIES  64
VOID   **mSavedTables = NULL;
UINTN   mSavedTablesEntries = 0;
UINTN   mSavedTablesNum = 0;

/** Returns TRUE is TableEntry is already saved. */
BOOLEAN IsTableSaved(VOID *TableEntry)
{
  UINTN   Index;
  
  if (mSavedTables != NULL) {
    for (Index = 0; Index < mSavedTablesNum; Index++) {
      if (mSavedTables[Index] == TableEntry) {
        return TRUE;
      }
    }
  }
  return FALSE;
}

/** Adds TableEntry to mSavedTables if not already there. */
VOID MarkTableAsSaved(VOID *TableEntry)
{
  //
  // If mSavedTables does not exists yet - allocate it
  //
  if (mSavedTables == NULL) {
    //DBG(" Allocaing mSavedTables");
    mSavedTablesEntries = SAVED_TABLES_ALLOC_ENTRIES;
    mSavedTablesNum = 0;
    mSavedTables = AllocateZeroPool(sizeof(*mSavedTables) * mSavedTablesEntries);
    if (mSavedTables == NULL) {
      return;
    }
  }
  
  //
  // If TableEntry is not in mSavedTables - add it
  //
  //DBG(" MarkTableAsSaved %p", TableEntry);
  if (IsTableSaved(TableEntry)) {
    // already saved
    //DBG(" - already saved\n");
    return;
  }
  
  //
  // If mSavedTables is full - extend it
  //
  if (mSavedTablesNum + 1 >= mSavedTablesEntries) {
    // not enough space
    //DBG(" - extending mSavedTables from %d", mSavedTablesEntries);
    mSavedTables = ReallocatePool(
                                  sizeof(*mSavedTables) * mSavedTablesEntries,
                                  sizeof(*mSavedTables) * (mSavedTablesEntries + SAVED_TABLES_ALLOC_ENTRIES),
                                  mSavedTables
                                  );
    if (mSavedTables == NULL) {
      return;
    }
    mSavedTablesEntries = mSavedTablesEntries + SAVED_TABLES_ALLOC_ENTRIES;
    //DBG(" to %d", mSavedTablesEntries);
  }
  
  //
  // Add TableEntry to mSavedTables
  //
  mSavedTables[mSavedTablesNum] = (VOID*)TableEntry;
  //DBG(" - added to index %d\n", mSavedTablesNum);
  mSavedTablesNum++;
}

STATIC CHAR8 NameSSDT[] = {0x08, 0x53, 0x53, 0x44, 0x54};
// OperationRegion (SSDT, SystemMemory, 0xDF5DAC18, 0x038C)
STATIC UINT8 NameSSDT2[] = {0x80, 0x53, 0x53, 0x44, 0x54};
// OperationRegion (CSDT, SystemMemory, 0xDF5DBE18, 0x84)
STATIC UINT8 NameCSDT[] = {0x80, 0x43, 0x53, 0x44, 0x54};

VOID DumpChildSsdt(EFI_ACPI_DESCRIPTION_HEADER *TableEntry, CHAR16 *DirName, CHAR16 *FileNamePrefix, UINTN *SsdtCount)
{
  EFI_STATUS		Status = EFI_SUCCESS;
  INTN    j, k, pacLen;
  CHAR16  *FileName;
	CHAR8		Signature[5];
	CHAR8		OemTableId[9];
  UINTN   adr, len;
  UINT8   *Entry;
  UINT8   *End;
	 
  Entry = (UINT8*)TableEntry;
  End = Entry + TableEntry->Length;
  while (Entry < End) {
    
    if (CompareMem(Entry, NameSSDT, 5) == 0) {
      pacLen = Entry[8];
      if (pacLen % 3 == 0) {
        DBG("\n Found hidden SSDT %d pcs\n", pacLen/3);
        for (j = 0; j < pacLen/3; j++) {
          
          adr = ReadUnaligned32((UINT32*)(Entry + 20 + j*20));
          len = ReadUnaligned32((UINT32*)(Entry + 25 + j*20));
          
          // Take Signature and OemId for printing
          CopyMem((CHAR8*)&Signature, (CHAR8*)&((EFI_ACPI_DESCRIPTION_HEADER *)adr)->Signature, 4);
          Signature[4] = 0;
          CopyMem((CHAR8*)&OemTableId, (CHAR8*)&((EFI_ACPI_DESCRIPTION_HEADER *)adr)->OemTableId, 8);
          OemTableId[8] = 0;
          DBG("      * %p: '%a', '%a', Rev: %d, Len: %d  ", adr, Signature, OemTableId,
              ((EFI_ACPI_DESCRIPTION_HEADER *)adr)->Revision, ((EFI_ACPI_DESCRIPTION_HEADER *)adr)->Length);
          for(k=0; k<16; k++){
            DBG("%02x ", ((UINT8*)adr)[k]);
          }
          if ((AsciiStrCmp(Signature, "SSDT") == 0) && (len < 0x20000) && DirName != NULL && !IsTableSaved((VOID*)adr)) {
            FileName = PoolPrint(L"%sSSDT-%dx.aml", FileNamePrefix, *SsdtCount);
            len = ((UINT16*)adr)[2];
            DBG("Internal length = %d", len);
            Status = SaveBufferToDisk((VOID*)adr, len, DirName, FileName);
            if (!EFI_ERROR(Status)) {
              DBG(" -> %s\n", FileName);
              MarkTableAsSaved((VOID*)adr);
              *SsdtCount += 1;
            } else {
              DBG(" -> %r\n", Status);
            }
            FreePool(FileName);
          }
        }
      }
      Entry += 5;
    } else if (CompareMem(Entry, NameSSDT2, 5) == 0
               || CompareMem(Entry, NameCSDT, 5) == 0)
    {
      
      adr = ReadUnaligned32((UINT32*)(Entry + 7));
      len = 0;
      j = *(Entry + 11);
      if (j == 0x0b) {
        len = ReadUnaligned16((UINT16*)(Entry + 12));
      } else if (j == 0x0a) {
        len = *(Entry + 12);
      } else {
        //not a number so skip for security
        Entry += 5;
        continue;
      }
      
      if (len > 0) {
        // Take Signature and OemId for printing
        CopyMem((CHAR8*)&Signature, (CHAR8*)&((EFI_ACPI_DESCRIPTION_HEADER *)adr)->Signature, 4);
        Signature[4] = 0;
        CopyMem((CHAR8*)&OemTableId, (CHAR8*)&((EFI_ACPI_DESCRIPTION_HEADER *)adr)->OemTableId, 8);
        OemTableId[8] = 0;
        DBG("      * %p: '%a', '%a', Rev: %d, Len: %d  ", adr, Signature, OemTableId,
            ((EFI_ACPI_DESCRIPTION_HEADER *)adr)->Revision, ((EFI_ACPI_DESCRIPTION_HEADER *)adr)->Length);
        for(k=0; k<16; k++){
          DBG("%02x ", ((UINT8*)adr)[k]);
        }
        if ((AsciiStrCmp(Signature, "SSDT") == 0) && (len < 0x20000) && DirName != NULL && !IsTableSaved((VOID*)adr)) {
          FileName = PoolPrint(L"%sSSDT-%dx.aml", FileNamePrefix, *SsdtCount);
          Status = SaveBufferToDisk((VOID*)adr, len, DirName, FileName);
          if (!EFI_ERROR(Status)) {
            DBG(" -> %s", FileName);
            MarkTableAsSaved((VOID*)adr);
            *SsdtCount += 1;
          } else {
            DBG(" -> %r", Status);
          }
          FreePool(FileName);
        }
        DBG("\n");
      }
      Entry += 5;
    } else {
      Entry++;
    }
  }
}

/** Saves Table to disk as DirName\\FileName (DirName != NULL)
 *  or just prints basic table data to log (DirName == NULL).
 */
EFI_STATUS DumpTable(EFI_ACPI_DESCRIPTION_HEADER *TableEntry, CHAR8 *CheckSignature, CHAR16 *DirName, CHAR16 *FileName, CHAR16 *FileNamePrefix, UINTN *SsdtCount)
{
	EFI_STATUS    Status;
	CHAR8         Signature[5];
	CHAR8         OemTableId[9];
	BOOLEAN       ReleaseFileName = FALSE;
	
	// Take Signature and OemId for printing
	CopyMem((CHAR8*)&Signature, (CHAR8*)&TableEntry->Signature, 4);
	Signature[4] = 0;
	CopyMem((CHAR8*)&OemTableId, (CHAR8*)&TableEntry->OemTableId, 8);
	OemTableId[8] = 0;
	
	DBG(" %p: '%a', '%a', Rev: %d, Len: %d", TableEntry, Signature, OemTableId, TableEntry->Revision, TableEntry->Length);
	
	//
	// Additional checks
	//
	if (CheckSignature != NULL && AsciiStrCmp(Signature, CheckSignature) != 0) {
		DBG(" -> invalid signature, expecting %a\n", CheckSignature);
		return EFI_INVALID_PARAMETER;
	}
	// XSDT checks
	if (TableEntry->Signature == EFI_ACPI_2_0_EXTENDED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
		if (TableEntry->Length < sizeof(XSDT_TABLE)) {
			DBG(" -> invalid length\n");
			return EFI_INVALID_PARAMETER;
		}
	}
	// RSDT checks
	if (TableEntry->Signature == EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
		if (TableEntry->Length < sizeof(RSDT_TABLE)) {
			DBG(" -> invalid length\n");
			return EFI_INVALID_PARAMETER;
		}
	}
	// FADT/FACP checks
	if (TableEntry->Signature == EFI_ACPI_1_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE) {
		if (TableEntry->Length < sizeof(EFI_ACPI_1_0_FIXED_ACPI_DESCRIPTION_TABLE)) {
			DBG(" -> invalid length\n");
			return EFI_INVALID_PARAMETER;
		}
	}
	// DSDT checks
	if (TableEntry->Signature == EFI_ACPI_1_0_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
		if (TableEntry->Length < sizeof(EFI_ACPI_DESCRIPTION_HEADER)) {
			DBG(" -> invalid length\n");
			return EFI_INVALID_PARAMETER;
		}
	}
	// SSDT checks
	if (TableEntry->Signature == EFI_ACPI_1_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
		if (TableEntry->Length < sizeof(EFI_ACPI_DESCRIPTION_HEADER)) {
			DBG(" -> invalid length\n");
			return EFI_INVALID_PARAMETER;
		}
	}

	if (DirName == NULL || IsTableSaved(TableEntry)) {
		// just debug log dump
		return EFI_SUCCESS;
	}
	
	if (FileNamePrefix == NULL) {
		FileNamePrefix = L"";
	}
	
	if (FileName == NULL) {
		// take the name from the signature
		if (TableEntry->Signature == EFI_ACPI_1_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE && SsdtCount != NULL) {
			// Ssdt counter
			FileName = PoolPrint(L"%sSSDT-%d.aml", FileNamePrefix, *SsdtCount);
			*SsdtCount = *SsdtCount + 1;
			DumpChildSsdt(TableEntry, DirName, FileNamePrefix, SsdtCount);
		} else {
			FileName = PoolPrint(L"%s%a.aml", FileNamePrefix, Signature);
		}
		
		if (FileName == NULL) {
			return EFI_OUT_OF_RESOURCES;
		}
		ReleaseFileName = TRUE;
	}
	DBG(" -> %s", FileName);
	
	// Save it
	Status = SaveBufferToDisk((VOID*)TableEntry, TableEntry->Length, DirName, FileName);
	MarkTableAsSaved(TableEntry);
	
	if (ReleaseFileName) {
		FreePool(FileName);
	}
	
	return Status;
}

/** Saves to disk (DirName != NULL) or prints to log (DirName == NULL) Fadt tables: Dsdt and Facs. */
EFI_STATUS DumpFadtTables(EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE *Fadt, CHAR16 *DirName, CHAR16 *FileNamePrefix, UINTN *SsdtCount)
{
	EFI_ACPI_DESCRIPTION_HEADER                   *TableEntry;
	EFI_ACPI_2_0_FIRMWARE_ACPI_CONTROL_STRUCTURE  *Facs;
	EFI_STATUS    Status  = EFI_SUCCESS;
	UINT64        DsdtAdr;
	UINT64        FacsAdr;
	CHAR8         Signature[5];
	CHAR16        *FileName;
	
	//
	// if Fadt->Revision < 3 (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION), then it is Acpi 1.0
	// and fields after Flags are not available
	//
	
	DBG("      (Dsdt: %x, Facs: %x", Fadt->Dsdt, Fadt->FirmwareCtrl);
	
	// for Acpi 1.0
	DsdtAdr = Fadt->Dsdt;
	FacsAdr = Fadt->FirmwareCtrl;
	
	if (Fadt->Header.Revision >= EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION) {
		// Acpi 2.0 or up
		// may have it in XDsdt or XFirmwareCtrl
		DBG(", XDsdt: %lx, XFacs: %lx", Fadt->XDsdt, Fadt->XFirmwareCtrl);
		if (Fadt->XDsdt != 0) {
			DsdtAdr = Fadt->XDsdt;
		}
		if (Fadt->XFirmwareCtrl != 0) {
			FacsAdr = Fadt->XFirmwareCtrl;
		}
	}
	DBG(")\n");
	
	//
	// Save Dsdt
	//
	if (DsdtAdr != 0) {
		DBG("     ");
		TableEntry = (EFI_ACPI_DESCRIPTION_HEADER*)(UINTN)DsdtAdr;
		Status = DumpTable(TableEntry, "DSDT", DirName,  NULL, FileNamePrefix, NULL);
		if (EFI_ERROR(Status)) {
			DBG(" - %r\n", Status);
			return Status;
		}
		DBG("\n");
		DumpChildSsdt(TableEntry, DirName, FileNamePrefix, SsdtCount);
	}
	//
	// Save Facs
	//
	if (FacsAdr != 0) {
		// Taking it as structure from Acpi 2.0 just to get Version (it's reserved field in Acpi 1.0 and == 0)
		Facs = (EFI_ACPI_2_0_FIRMWARE_ACPI_CONTROL_STRUCTURE*)(UINTN)FacsAdr;
		// Take Signature for printing
		CopyMem((CHAR8*)&Signature, (CHAR8*)&Facs->Signature, 4);
		Signature[4] = 0;
		DBG("      %p: '%a', Ver: %d, Len: %d", Facs, Signature, Facs->Version, Facs->Length);
		
		// FACS checks
		if (Facs->Signature != EFI_ACPI_1_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_SIGNATURE) {
			DBG(" -> invalid signature, expecting FACS\n");
			return EFI_INVALID_PARAMETER;
		}
		if (Facs->Length < sizeof(EFI_ACPI_1_0_FIRMWARE_ACPI_CONTROL_STRUCTURE)) {
			DBG(" -> invalid length\n");
			return EFI_INVALID_PARAMETER;
		}
		
		if (DirName != NULL && !IsTableSaved((VOID*)Facs)) {
			FileName = PoolPrint(L"%sFACS.aml", FileNamePrefix);
			DBG(" -> %s", FileName);
			Status = SaveBufferToDisk((VOID*)Facs, Facs->Length, DirName, FileName);
			MarkTableAsSaved(Facs);
			FreePool(FileName);
			if (EFI_ERROR(Status)) {
				DBG(" - %r\n", Status);
				return Status;
			}
		}
		
		DBG("\n");
	}

	return Status;
}

EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE* GetFadt()
{
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER  *RsdPtr;
  EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE     *FadtPointer = NULL;	

  EFI_STATUS      Status;
  
  RsdPtr = FindAcpiRsdPtr();
	if (RsdPtr == NULL) {
    Status = EfiGetSystemConfigurationTable (&gEfiAcpi20TableGuid, (VOID **)&RsdPtr);
    if (RsdPtr == NULL) {
      Status = EfiGetSystemConfigurationTable (&gEfiAcpi10TableGuid, (VOID **)&RsdPtr);
      if (RsdPtr == NULL) {
        return NULL;
      }
    }
  }
  Rsdt = (RSDT_TABLE*)(UINTN)(RsdPtr->RsdtAddress);
  if (RsdPtr->Revision > 0) {
    if (Rsdt == NULL || Rsdt->Header.Signature != EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
      Xsdt = (XSDT_TABLE *)(UINTN)(RsdPtr->XsdtAddress);
    }
  }
  if (Rsdt == NULL && Xsdt == NULL) {
    return NULL;
  }
  
  if (Rsdt) {
    FadtPointer = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE*)(UINTN)(Rsdt->Entry);
  }
  if (Xsdt) {
    //overwrite previous find as xsdt priority
    FadtPointer = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE*)(UINTN)(Xsdt->Entry);
  }
  return FadtPointer;
}

/** Saves to disk (DirName != NULL)
 *  or prints to debug log (DirName == NULL)
 *  ACPI tables given by RsdPtr.
 *  Takes tables from Xsdt if present or from Rsdt if Xsdt is not present.
 */
VOID DumpTables(VOID *RsdPtrVoid, CHAR16 *DirName)
{
	EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER    *RsdPtr;
	EFI_ACPI_DESCRIPTION_HEADER                     *TableEntry;
	EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE       *Fadt;

	EFI_STATUS      Status;
	UINTN           Length;
	CHAR8           Signature[9];
	UINT32          *EntryPtr32;
	CHAR8           *EntryPtr;
	UINTN           EntryCount;
	UINTN           Index;
	UINTN           SsdtCount;
	CHAR16          *FileNamePrefix;

	//
	// RSDP
	// Take it as Acpi 2.0, but take care that if RsdPtr->Revision == 0
	// then it is actually Acpi 1.0 and fields after RsdtAddress (like XsdtAddress)
	// are not available
	//
	
	RsdPtr = (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *) RsdPtrVoid;
	if (DirName != NULL) {
		DBG("Saving ACPI tables from RSDP %p to %s ...\n", RsdPtr, DirName);
	} else {
		DBG("Printing ACPI tables from RSDP %p ...\n", RsdPtr);
	}
	
	if (RsdPtr->Signature != EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER_SIGNATURE) {
		DBG(" RsdPrt at %p has invaid signature 0x%lx - exiting.\n", RsdPtr, RsdPtr->Signature);
		return;
	}
	
	// Take Signature and OemId for printing
	CopyMem((CHAR8*)&Signature, (CHAR8*)&RsdPtr->Signature, 8);
	Signature[8] = 0;
	
	// Take Rsdt and Xsdt
	Rsdt = NULL;
	Xsdt = NULL;
	
	DBG(" %p: '%a', Rev: %d", RsdPtr, Signature, RsdPtr->Revision);
	if (RsdPtr->Revision == 0) {
		// Acpi 1.0
		Length = sizeof(EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER);
		DBG(" (Acpi 1.0)");
	} else {
		// Acpi 2.0 or newer
		Length = RsdPtr->Length;
		DBG(" (Acpi 2.0 or newer)");
	}
	DBG(", Len: %d", Length);
	
	//
	// Save RsdPtr
	//
	if (DirName != NULL && !IsTableSaved((VOID*)RsdPtr)) {
		DBG(" -> RSDP.aml");
		Status = SaveBufferToDisk((VOID*)RsdPtr, Length, DirName, L"RSDP.aml");
		MarkTableAsSaved(RsdPtr);
		if (EFI_ERROR(Status)) {
			DBG(" - %r\n", Status);
			return;
		}
	}
	DBG("\n");
	
	if (RsdPtr->Revision == 0) {
		// Acpi 1.0 - no Xsdt
		Rsdt = (RSDT_TABLE*)(UINTN)(RsdPtr->RsdtAddress);
		DBG("  (Rsdt: %p)\n", Rsdt);
	} else {
		// Acpi 2.0 or newer - may have Xsdt and/or Rsdt
		Rsdt = (RSDT_TABLE*)(UINTN)(RsdPtr->RsdtAddress);
		Xsdt = (XSDT_TABLE *)(UINTN)(RsdPtr->XsdtAddress);
		DBG("  (Xsdt: %p, Rsdt: %p)\n", Xsdt, Rsdt);
	}
	
	if (Rsdt == NULL && Xsdt == NULL) {
		DBG(" No Rsdt and Xsdt - exiting.\n");
		return;
	}
	
	FileNamePrefix = L"";
  
	//
	// Save Xsdt
	//
	if (Xsdt != NULL) {
		DBG(" ");
		Status = DumpTable((EFI_ACPI_DESCRIPTION_HEADER *)Xsdt, "XSDT", DirName,  L"XSDT.aml", FileNamePrefix, NULL);
		if (EFI_ERROR(Status)) {
			DBG(" - %r", Status);
			Xsdt = NULL;
		}
		DBG("\n");
	}
	
	//
	// Save Rsdt
	//
	if (Rsdt != NULL) {
		DBG(" ");
		Status = DumpTable((EFI_ACPI_DESCRIPTION_HEADER *)Rsdt, "RSDT", DirName,  L"RSDT.aml", FileNamePrefix, NULL);
		if (EFI_ERROR(Status)) {
			DBG(" - %r", Status);
			Rsdt = NULL;
		}
		DBG("\n");
	}
	
	//
	// Check once more since they might be invalid
	//
	if (Rsdt == NULL && Xsdt == NULL) {
		DBG(" No Rsdt and Xsdt - exiting.\n");
		return;
	}
	
	if (Xsdt != NULL) {
		
		//
		// Take tables from Xsdt
		//
		
		EntryCount = (Xsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);
		if (EntryCount > 100) EntryCount = 100; //it's enough
		DBG("  Tables in Xsdt: %d\n", EntryCount); 
		
		// iterate over table entries
		EntryPtr = (CHAR8*)&Xsdt->Entry;
		SsdtCount = 0;
		for (Index = 0; Index < EntryCount; Index++, EntryPtr += sizeof(UINT64)) {
//         UINT64	*EntryPtr64 = (UINT64 *)EntryPtr;
			DBG("  %d.", Index);
			
			// skip NULL entries
			//if (*EntryPtr64 == 0) {
			if (ReadUnaligned64((CONST UINT64*)EntryPtr) == 0) {
				DBG(" = 0\n", Index);
				continue;
			}
			
			// Save table with the name from signature
			TableEntry = (EFI_ACPI_DESCRIPTION_HEADER*)(UINTN)(ReadUnaligned64((CONST UINT64*)EntryPtr));
			
			if (TableEntry->Signature == EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE) {
				//
				// Fadt - save Dsdt and Facs
				//
				Status = DumpTable(TableEntry, NULL, DirName,  NULL, FileNamePrefix, &SsdtCount);
				if (EFI_ERROR(Status)) {
					DBG(" - %r\n", Status);
					return;
				}
				DBG("\n");
				
				Fadt = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE*)TableEntry;        
				Status = DumpFadtTables(Fadt, DirName, FileNamePrefix, &SsdtCount);
				if (EFI_ERROR(Status)) {
					return;
				}
			} else {
				Status = DumpTable(TableEntry, NULL, DirName,  NULL /* take the name from the signature*/, FileNamePrefix, &SsdtCount);
				if (EFI_ERROR(Status)) {
					DBG(" - %r\n", Status);
					return;
				}
				DBG("\n");
			}
		}
		
		// additional Rsdt tables whih are not present in Xsdt will have "RSDT-" prefix, like RSDT-FACS.aml
		FileNamePrefix = L"RSDT-";
		
	} // if Xsdt
	
	
	if (Rsdt != NULL) {
		
		//
		// Take tables from Rsdt
		// if saved from Xsdt already, then just print debug
		//
		
		EntryCount = (Rsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT32);
		if (EntryCount > 100) EntryCount = 100; //it's enough
		DBG("  Tables in Rsdt: %d\n", EntryCount); 
		
		// iterate over table entries
		EntryPtr32 = (UINT32*)&Rsdt->Entry;
		SsdtCount = 0;
		for (Index = 0; Index < EntryCount; Index++, EntryPtr32++) {
			DBG("  %d.", Index);
			
			// skip NULL entries
			if (*EntryPtr32 == 0) {
				DBG(" = 0\n", Index);
				continue;
			}
			
			// Save table with the name from signature
			TableEntry = (EFI_ACPI_DESCRIPTION_HEADER*)(UINTN)(*EntryPtr32);
			if (TableEntry->Signature == EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE) {
				//
				// Fadt - save Dsdt and Facs
				//
				Status = DumpTable(TableEntry, NULL, DirName,  NULL, FileNamePrefix, &SsdtCount);
				if (EFI_ERROR(Status)) {
					DBG(" - %r\n", Status);
					return;
				}
				DBG("\n");
				
				Fadt = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE*)TableEntry;
				Status = DumpFadtTables(Fadt, DirName, FileNamePrefix, &SsdtCount);
				if (EFI_ERROR(Status)) {
					return;
				}
			} else {
				Status = DumpTable(TableEntry, NULL, DirName,  NULL /* take the name from the signature*/, FileNamePrefix, &SsdtCount);
				if (EFI_ERROR(Status)) {
						DBG(" - %r\n", Status);
					return;
				}
				DBG("\n");
			}
		}
	} // if Rsdt
}

/** Saves OEM ACPI tables to disk.
 *  Searches BIOS, then UEFI Sys.Tables for Acpi 2.0 or newer tables, then for Acpi 1.0 tables
 *  CloverEFI:
 *   - saves first one found, dump others to log
 *  UEFI:
 *   - saves first one found in UEFI Sys.Tables, dump others to log
 *
 * Dumping of other tables to log can be removed if it turns out that there is no value in doing it.
 */
VOID SaveOemTables()
{
	EFI_STATUS              Status;
	VOID                    *RsdPtr;
	CHAR16                  *AcpiOriginPath = PoolPrint(L"%s\\ACPI\\origin", OEMPath);
	BOOLEAN                 Saved = FALSE;
	CHAR8                   *MemLogStart;
	UINTN                   MemLogStartLen;
	
	MemLogStartLen = GetMemLogLen();
	MemLogStart = GetMemLogBuffer() + MemLogStartLen;
	
	
	//
	// Search in BIOS
	// CloverEFI - Save
	// UEFI - just print to log
	//
//	RsdPtr = NULL;
	RsdPtr = FindAcpiRsdPtr();
	if (RsdPtr != NULL) {
		DBG("Found BIOS RSDP at %p\n", RsdPtr);
		if (gFirmwareClover) {
			// Save it 
			DumpTables(RsdPtr, AcpiOriginPath);
			Saved = TRUE;
		} else {
			// just print to log
			DumpTables(RsdPtr, NULL);
		}
	}
	
	//
	// Search Acpi 2.0 or newer in UEFI Sys.Tables
	//
	RsdPtr = NULL;
	Status = EfiGetSystemConfigurationTable (&gEfiAcpi20TableGuid, &RsdPtr);
	if (RsdPtr != NULL) {
		DBG("Found UEFI Acpi 2.0 RSDP at %p\n", RsdPtr);
		// if tables already saved, then just print to log
		DumpTables(RsdPtr, Saved ? NULL : AcpiOriginPath);
		Saved = TRUE;
	}
	
	//
	// Then search Acpi 1.0 UEFI Sys.Tables
	//
	RsdPtr = NULL;
	Status = EfiGetSystemConfigurationTable (&gEfiAcpi10TableGuid, &RsdPtr);
	if (RsdPtr != NULL) {
		DBG("Found UEFI Acpi 1.0 RSDP at %p\n", RsdPtr);
		// if tables already saved, then just print to log
		DumpTables(RsdPtr, Saved ? NULL : AcpiOriginPath);
		Saved = TRUE;
	}
	
	SaveBufferToDisk(MemLogStart, GetMemLogLen() - MemLogStartLen, AcpiOriginPath, L"DumpLog.txt");
	
	FreePool(mSavedTables);
}

VOID        SaveOemDsdt(BOOLEAN FullPatch)
{
  EFI_STATUS                                    Status = EFI_NOT_FOUND;
//  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER	*RsdPointer = NULL;
  EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE     *FadtPointer = NULL;	
//  EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE     *XFadtPointer = NULL;	
  EFI_PHYSICAL_ADDRESS                          dsdt = EFI_SYSTEM_TABLE_MAX_ADDRESS; 

	UINTN				Pages;
	UINT8       *buffer = NULL;
	UINTN       DsdtLen = 0;
  CHAR16*     OriginDsdt = PoolPrint(L"%s\\ACPI\\origin\\DSDT.aml", OEMPath);
  CHAR16*     OriginDsdtFixed = PoolPrint(L"%s\\ACPI\\origin\\DSDT-%x.aml", OEMPath, gSettings.FixDsdt);
  CHAR16*     PathPatched   = L"\\EFI\\CLOVER\\ACPI\\patched";
	CHAR16*     PathDsdt;    //  = L"\\DSDT.aml";
//  CHAR16*     PathDsdtMini  = L"\\EFI\\CLOVER\\ACPI\\mini\\DSDT.aml";
  CHAR16*     AcpiOemPath = PoolPrint(L"%s\\ACPI\\patched", OEMPath);
  
  PathDsdt = PoolPrint(L"\\%s", gSettings.DsdtName);
/*  
  if (gSettings.UseDSDTmini) {
    DBG("search DSDTmini\n"); 
    if (FileExists(SelfRootDir, PathDsdtMini)) {
      DBG(" DSDTmini found\n");
      Status = egLoadFile(SelfRootDir, PathDsdtMini, &buffer, &bufferLen);
    }
  } */
  
  if (FileExists(SelfRootDir, PoolPrint(L"%s%s", AcpiOemPath, PathDsdt))) {
    DBG("DSDT found in Clover volume OEM folder: %s%s\n", AcpiOemPath, PathDsdt);
    Status = egLoadFile(SelfRootDir, PoolPrint(L"%s%s", AcpiOemPath, PathDsdt), &buffer, &DsdtLen);
  }
  
  if (EFI_ERROR(Status) && FileExists(SelfRootDir, PoolPrint(L"%s%s", PathPatched, PathDsdt))) {
    DBG("DSDT found in Clover volume common folder: %s%s\n", PathPatched, PathDsdt);
    Status = egLoadFile(SelfRootDir, PoolPrint(L"%s%s", PathPatched, PathDsdt), &buffer, &DsdtLen);
  }
  
  if (EFI_ERROR(Status)) {
/*    if (gFirmwareClover) {
      RsdPointer = (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER*)FindAcpiRsdPtr();
    } else {
      Status = EfiGetSystemConfigurationTable (&gEfiAcpi20TableGuid, (VOID**)&RsdPointer);
      if (EFI_ERROR(Status)) {
        Status = EfiGetSystemConfigurationTable (&gEfiAcpi10TableGuid, (VOID**)&RsdPointer);
      }
    }
    
    if (RsdPointer == NULL) {
      return;
    }
    
    Rsdt = (RSDT_TABLE*)(UINTN)(RsdPointer->RsdtAddress);
    if (RsdPointer->Revision > 0) {
      if (Rsdt == NULL || Rsdt->Header.Signature != EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
        Xsdt = (XSDT_TABLE *)(UINTN)(RsdPointer->XsdtAddress);
      }
    }
    if (Rsdt == NULL && Xsdt == NULL) {
      return;
    }
    
    if (Rsdt) {
      FadtPointer = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE*)(UINTN)(Rsdt->Entry);
    }
    if (Xsdt) {
      XFadtPointer = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE*)(UINTN)(Xsdt->Entry);
    }
    if (!FadtPointer) {
      FadtPointer = XFadtPointer;
    }
*/    
    FadtPointer = GetFadt();
    if (FadtPointer == NULL) {
      return;
    }
        
    BiosDsdt = FadtPointer->Dsdt;
    if (FadtPointer->Header.Revision >= EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION && FadtPointer->XDsdt != 0) {
      BiosDsdt = FadtPointer->XDsdt;
    }
    buffer = (UINT8*)(UINTN)BiosDsdt;
	} 
  
  if (!buffer) {
    DBG("Cannot found DSDT in BIOS or in files!\n");
    return;
  }
  
  DsdtLen = ((EFI_ACPI_DESCRIPTION_HEADER*)buffer)->Length;
	Pages = EFI_SIZE_TO_PAGES(DsdtLen + DsdtLen / 8); // take some extra space for patches
  Status = gBS->AllocatePages (
                               AllocateMaxAddress,
                               EfiBootServicesData,
                               Pages,
                               &dsdt
                               );
  
  //if success insert dsdt pointer into ACPI tables
  if(!EFI_ERROR(Status))
  {
    CopyMem((UINT8*)(UINTN)dsdt, buffer, DsdtLen);
    buffer = (UINT8*)(UINTN)dsdt;
    if (FullPatch) {
      FixBiosDsdt(buffer);
      DsdtLen = ((EFI_ACPI_DESCRIPTION_HEADER*)buffer)->Length;
			OriginDsdt = OriginDsdtFixed;
    }
    Status = egSaveFile(SelfRootDir, OriginDsdt, buffer, DsdtLen);
    if (EFI_ERROR(Status)) {
      Status = egSaveFile(NULL, OriginDsdt, buffer, DsdtLen);
    }
		if (!EFI_ERROR(Status)) {
      MsgLog("DSDT saved to %s\n", OriginDsdt);
		} else {
			MsgLog("Saving DSDT to % s failed - %r\n", OriginDsdt, Status);
		}
    gBS->FreePages(dsdt, Pages);
  }
}

EFI_STATUS PatchACPI(IN REFIT_VOLUME *Volume)
{
	EFI_STATUS										Status = EFI_SUCCESS;
	UINTN                         Index;
	EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER	*RsdPointer = NULL;
	EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE		*FadtPointer = NULL;	
	EFI_ACPI_4_0_FIXED_ACPI_DESCRIPTION_TABLE		*newFadt	 = NULL;
  //	EFI_ACPI_HIGH_PRECISION_EVENT_TIMER_TABLE_HEADER	*Hpet    = NULL;
	EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE	*Facs = NULL;
  //	EFI_GUID										*gTableGuidArray[] = {&gEfiAcpi20TableGuid, &gEfiAcpi10TableGuid};
  //	EFI_PEI_HOB_POINTERS							GuidHob;
  //	EFI_PEI_HOB_POINTERS							HobStart;
  //	EFI_DEVICE_PATH_PROTOCOL*	PathBooter = NULL;
  //	EFI_DEVICE_PATH_PROTOCOL*	FilePath;
  //	EFI_HANDLE					FileSystemHandle;
	EFI_PHYSICAL_ADDRESS		dsdt = EFI_SYSTEM_TABLE_MAX_ADDRESS; //0xFE000000;
	EFI_PHYSICAL_ADDRESS		BufferPtr;
  SSDT_TABLE              *Ssdt = NULL;
	UINT8                   *buffer = NULL;
	UINTN                   bufferLen = 0;
	CHAR16*                 PathPatched   = L"\\EFI\\CLOVER\\ACPI\\patched";
	CHAR16*                 PathDsdt;    //  = L"\\DSDT.aml";
 // CHAR16*                 PathDsdtMini  = L"\\EFI\\CLOVER\\ACPI\\mini\\DSDT.aml";
  CHAR16*                 PatchedAPIC = L"\\EFI\\CLOVER\\ACPI\\origin\\APIC-p.aml";
	UINT32*                 rf = NULL;
	UINT64*                 xf = NULL;
  UINT64                  XDsdt; //save values if present
  UINT64                  XFirmwareCtrl;
  EFI_FILE                *RootDir;
  UINT32                  eCntR; //, eCntX;
  UINT32                  *pEntryR;
  CHAR8                   *pEntry;
  EFI_ACPI_DESCRIPTION_HEADER *TableHeader;
  // -===== APIC =====-
  EFI_ACPI_DESCRIPTION_HEADER                           *ApicTable;
//  EFI_ACPI_2_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER   *ApicHeader;
  EFI_ACPI_2_0_PROCESSOR_LOCAL_APIC_STRUCTURE           *ProcLocalApic;
  EFI_ACPI_2_0_LOCAL_APIC_NMI_STRUCTURE                 *LocalApicNMI;
  UINTN             ApicLen;
  UINT8             CPUBase;
  UINTN             ApicCPUNum;
  UINT8             *SubTable;
  BOOLEAN           DsdtLoaded = FALSE;
  INTN              ApicCPUBase = 0;
  UINTN             i;
  CHAR16*     AcpiOemPath = PoolPrint(L"%s\\ACPI\\patched", OEMPath);
  PathDsdt = PoolPrint(L"\\%s", gSettings.DsdtName);
/*	
  if (gFirmwareClover || gSettings.RememberBIOS) {
    // although it work on Aptio, no need for the following on other UEFis
    
    //Slice - I want to begin from BIOS ACPI tables like with SMBIOS
    RsdPointer = (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER*)FindAcpiRsdPtr();
    //	DBG("Found RsdPtr in BIOS: %p\n", RsdPointer);
    
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
    //  DBG("Found FADT in BIOS: %p\n", FadtPointer);
    Facs = (EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE*)(UINTN)(FadtPointer->FirmwareCtrl);
//    DBG("Found FACS in BIOS: %p\n", Facs);
    BiosDsdt = FadtPointer->XDsdt;
    if (BiosDsdt == 0) {
      BiosDsdt = FadtPointer->Dsdt;
      if (BiosDsdt == 0) {
        DBG("Cannot found DSDT in Bios tables!\n");
      }
    }
  }
*/  
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
  if (1) //!RsdPointer) //if we have RsdPointer from BIOS them nothing to do here
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
 /*   if (!Facs && RsdPointer) {
      Rsdt = (RSDT_TABLE*)(UINTN)(RsdPointer->RsdtAddress);
      if (Rsdt == NULL || Rsdt->Header.Signature != EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
        Xsdt = (XSDT_TABLE *)(UINTN)(RsdPointer->XsdtAddress);
      }
      if (Rsdt) {
        FadtPointer = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE*)(UINTN)(Rsdt->Entry);
        DBG("Take FADT from RSDT: %p\n", FadtPointer);
      } else if (Xsdt) {
        FadtPointer = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE*)(UINTN)(Xsdt->Entry);
        DBG("Take FADT from XSDT: %p\n", FadtPointer);
      } else {
        DBG("No RSDT or XSDT, exiting\n");
        return EFI_NOT_FOUND;
      }

      Facs = (EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE*)(UINTN)(FadtPointer->FirmwareCtrl);
      DBG("Found FACS in System table: %p\n", Facs);
    } */
	}
#endif	
  
	if (!RsdPointer) {
		return EFI_UNSUPPORTED;
	}
	Rsdt = (RSDT_TABLE*)(UINTN)RsdPointer->RsdtAddress;
  DBG("RSDT 0x%p\n", Rsdt);
	rf = ScanRSDT(EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE);
	if(rf) {
		FadtPointer = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE*)(UINTN)(*rf);
    DBG("FADT from RSDT: 0x%p\n", FadtPointer);
  }
  
  Xsdt = NULL;			
  if (RsdPointer->Revision >=2 && (RsdPointer->XsdtAddress < (UINT64)(UINTN)-1))
  {
    Xsdt = (XSDT_TABLE*)(UINTN)RsdPointer->XsdtAddress;
    DBG("XSDT 0x%p\n", Xsdt);
    xf = ScanXSDT(EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE);
    if(xf) {
      //Slice - change priority. First Xsdt, second Rsdt
      if (*xf) {
        FadtPointer = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE*)(UINTN)(*xf);
        DBG("FADT from XSDT: 0x%p\n", FadtPointer);
      } else {
        *xf =  (UINT64)(UINTN)FadtPointer;
        DBG("reuse FADT\n");
      }
    }
  }
	
	if(!xf && Rsdt){
	 	DBG("Error! Xsdt is not found!!! Creating new one\n");
	 	//We should make here ACPI20 RSDP with all needed subtables based on ACPI10
    BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS;
    Status = gBS->AllocatePages(AllocateMaxAddress, EfiACPIReclaimMemory, 1, &BufferPtr);		
    if(!EFI_ERROR(Status))
    {
      if (RsdPointer->Revision == 0) {
        // Acpi 1.0 RsdPtr, but we need Acpi 2.0
        EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER  *NewRsdPointer;
        DBG("RsdPointer is Acpi 1.0 - creating new one Acpi 2.0\n");
        
        // add new pointer to the beginning of a new buffer
        NewRsdPointer = (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER*)(UINTN)BufferPtr;
        
        // and Xsdt will come after it
        BufferPtr += 0x30;
        
        // Signature, Checksum, OemId, Reserved/Revision, RsdtAddress
        CopyMem((VOID*)NewRsdPointer, (VOID*)RsdPointer, sizeof(EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER));
        NewRsdPointer->Revision = 2;
        NewRsdPointer->Length = sizeof(EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER);
        RsdPointer = NewRsdPointer;
        gBS->InstallConfigurationTable (&gEfiAcpiTableGuid, (VOID*)RsdPointer);
        gBS->InstallConfigurationTable (&gEfiAcpi10TableGuid, (VOID*)RsdPointer);
        DBG("RsdPointer Acpi 2.0 installed\n");
      }
      Xsdt = (XSDT_TABLE*)(UINTN)BufferPtr;
      //      Print(L"XSDT = 0x%x\n\r", Xsdt);
      Xsdt->Header.Signature = 0x54445358; //EFI_ACPI_2_0_EXTENDED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE
      eCntR = (Rsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT32);        
      Xsdt->Header.Length = eCntR * sizeof(UINT64) + sizeof (EFI_ACPI_DESCRIPTION_HEADER);
      Xsdt->Header.Revision = 1;
      CopyMem((CHAR8 *)&Xsdt->Header.OemId, (CHAR8 *)&FadtPointer->Header.OemId, 6);
      Xsdt->Header.OemTableId = Rsdt->Header.OemTableId;
      Xsdt->Header.OemRevision = Rsdt->Header.OemRevision;
      Xsdt->Header.CreatorId = Rsdt->Header.CreatorId;
      Xsdt->Header.CreatorRevision = Rsdt->Header.CreatorRevision;
      pEntryR = (UINT32*)(&(Rsdt->Entry));
      pEntry = (CHAR8*)(&(Xsdt->Entry));
      for (Index = 0; Index < eCntR; Index ++) 
      {
        UINT64  *pEntryX = (UINT64 *)pEntry;
        DBG("RSDT entry = 0x%x\n", *pEntryR);
        if (*pEntryR != 0) {
          *pEntryX = 0;
          CopyMem ((VOID*)pEntryX, (VOID*)pEntryR, sizeof(UINT32));
          pEntryR++;
          pEntry += sizeof(UINT64);
        } else {
          DBG("... skip it\n");
          Xsdt->Header.Length -= sizeof(UINT64);
          pEntryR++;
        }
      }
    }
  }
  if (Xsdt) {
    //Now we need no more Rsdt
    Rsdt =  NULL;
    RsdPointer->RsdtAddress = 0;
    //and we want to reallocate Xsdt
    BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS;
    Status = gBS->AllocatePages(AllocateMaxAddress, EfiACPIReclaimMemory, 1, &BufferPtr);		
    if(!EFI_ERROR(Status))
    {
      CopyMem((UINT8*)(UINTN)BufferPtr, (UINT8*)(UINTN)Xsdt, Xsdt->Header.Length);
      Xsdt = (XSDT_TABLE*)(UINTN)BufferPtr;      
    }      
    
 //   DBG("Finishing RsdPointer\n");
    RsdPointer->XsdtAddress = (UINT64)(UINTN)Xsdt;
    RsdPointer->Checksum = 0;
    RsdPointer->Checksum = (UINT8)(256-Checksum8((CHAR8*)RsdPointer, 20));
    RsdPointer->ExtendedChecksum = 0;
    RsdPointer->ExtendedChecksum = (UINT8)(256-Checksum8((CHAR8*)RsdPointer, RsdPointer->Length));
    DBG("Xsdt reallocation done\n");    
  }
  
  //  DBG("FADT pointer = %x\n", (UINTN)FadtPointer);
  if(!FadtPointer)
  {
    return EFI_NOT_FOUND;
  }
  //Slice - then we do FADT patch no matter if we don't have DSDT.aml
  BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS;
  Status = gBS->AllocatePages(AllocateMaxAddress, EfiACPIReclaimMemory, 1, &BufferPtr);		
  if(!EFI_ERROR(Status))
  {
    UINT32 oldLength = ((EFI_ACPI_DESCRIPTION_HEADER*)FadtPointer)->Length;
    newFadt = (EFI_ACPI_4_0_FIXED_ACPI_DESCRIPTION_TABLE*)(UINTN)BufferPtr;
    DBG("old FADT length=%x\n", oldLength);
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
    if (gSettings.EnableC6 || gSettings.EnableISS) {
      newFadt->CstCnt = 0x85; //as in Mac
    }
    if (gSettings.EnableC2) newFadt->PLvl2Lat = 0x65;
    if (gSettings.C3Latency > 0) {
      newFadt->PLvl3Lat = gSettings.C3Latency;
    } else if (gSettings.EnableC4) {
      newFadt->PLvl3Lat = 0x3E9;
    }
    if (gSettings.C3Latency == 0) {
      gSettings.C3Latency = newFadt->PLvl3Lat;
    }
    newFadt->IaPcBootArch = 0x3;
    newFadt->Flags |= 0x400; //Reset Register Supported
    XDsdt = newFadt->XDsdt; //save values if present
    XFirmwareCtrl = newFadt->XFirmwareCtrl;
    CopyMem((UINT8*)&newFadt->ResetReg, pmBlock, 0x80);
    //but these common values are not specific, so adjust
    //ACPIspec said that if Xdsdt !=0 then Dsdt must be =0. But real Mac no! Both values present
    if (BiosDsdt) {
      newFadt->XDsdt = BiosDsdt;
      newFadt->Dsdt = (UINT32)BiosDsdt;
    } else 
      if (newFadt->Dsdt) {
        newFadt->XDsdt = (UINT64)(newFadt->Dsdt);
      } else if (XDsdt) {
        newFadt->Dsdt = (UINT32)XDsdt;
      }
//    if (Facs) newFadt->FirmwareCtrl = (UINT32)(UINTN)Facs;
//    else DBG("No FACS table ?!\n");
    if (newFadt->FirmwareCtrl) {
      Facs = (EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE*)(UINTN)newFadt->FirmwareCtrl;
      newFadt->XFirmwareCtrl = (UINT64)(UINTN)(Facs);
    } else if (newFadt->XFirmwareCtrl) {
      newFadt->FirmwareCtrl = (UINT32)XFirmwareCtrl;
      Facs = (EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE*)(UINTN)XFirmwareCtrl;
    }
    //patch for FACS included here
    Facs->Version = EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_VERSION;
    //
    if ((gSettings.ResetAddr == 0) && ((oldLength < 0x80) || (newFadt->ResetReg.Address == 0))) {
      newFadt->ResetReg.Address   = 0x64; 
      newFadt->ResetValue         = 0xFE; 
      gSettings.ResetAddr         = 0x64;
      gSettings.ResetVal          = 0xFE;
    } else if (gSettings.ResetAddr != 0) {
      newFadt->ResetReg.Address    = gSettings.ResetAddr; 
      newFadt->ResetValue          = gSettings.ResetVal; 
    }
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
    FadtPointer->Header.Checksum = 0;
    FadtPointer->Header.Checksum = (UINT8)(256-Checksum8((CHAR8*)FadtPointer, FadtPointer->Header.Length));
    
  }
  
  /*  
   BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS;
   Status=gBS->AllocatePages(AllocateMaxAddress, EfiACPIReclaimMemory, 1, &BufferPtr);
   if(!EFI_ERROR(Status))
   {
   xf = ScanXSDT(HPET_SIGN);
   if(!xf) { //we want to make the new table if OEM is not found
   DBG("HPET creation\n");
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
   */  
  //  DBG("DSDT finding\n");
  if (!Volume) {
    DBG("Volume not found!\n");
    return EFI_NOT_FOUND;
  }
  
  RootDir = Volume->RootDir;
  Status = EFI_NOT_FOUND;
/*  if (gSettings.UseDSDTmini) {
    DBG("search DSDTmini\n"); 
    if (FileExists(SelfRootDir, PathDsdtMini)) {
      DBG(" DSDTmini found\n");
      Status = egLoadFile(SelfRootDir, PathDsdtMini, &buffer, &bufferLen);
    }
  }*/
  
  if (EFI_ERROR(Status) && FileExists(SelfRootDir, PoolPrint(L"%s%s", AcpiOemPath, PathDsdt))) {
    DBG("DSDT found in Clover volume OEM folder: %s%s\n", AcpiOemPath, PathDsdt);
    Status = egLoadFile(SelfRootDir, PoolPrint(L"%s%s", AcpiOemPath, PathDsdt), &buffer, &bufferLen);
  }
  
  if (EFI_ERROR(Status) && FileExists(RootDir, PathDsdt)) {
    DBG("DSDT found in booted volume\n");
    Status = egLoadFile(RootDir, PathDsdt, &buffer, &bufferLen);
  }
  
  if (EFI_ERROR(Status) && FileExists(SelfRootDir, PoolPrint(L"%s%s", PathPatched, PathDsdt))) {
    DBG("DSDT found in Clover volume: %s%s\n", PathPatched, PathDsdt);
    Status = egLoadFile(SelfRootDir, PoolPrint(L"%s%s", PathPatched, PathDsdt), &buffer, &bufferLen);
  }
  //
  //apply DSDT loaded from a file into buffer
  //else FADT will contain old BIOS DSDT
  //
  DsdtLoaded = FALSE;
  if (!EFI_ERROR(Status)) {
    // if we will apply fixes, allocate additional space
//		if (gSettings.FixDsdt) { //unconditional
			bufferLen = bufferLen + bufferLen / 8;
//		}
    dsdt = EFI_SYSTEM_TABLE_MAX_ADDRESS;
    Status = gBS->AllocatePages (
                                 AllocateMaxAddress,
                                 EfiACPIReclaimMemory,
                                 EFI_SIZE_TO_PAGES(bufferLen),
                                 &dsdt
                                 );
    
    //if success insert dsdt pointer into ACPI tables
    if(!EFI_ERROR(Status))
    {
      DBG("page is allocated, write DSDT into\n");
      CopyMem((VOID*)(UINTN)dsdt, buffer, bufferLen);
      
      FadtPointer->Dsdt  = (UINT32)dsdt;
      FadtPointer->XDsdt = dsdt;
      // verify checksum
      FadtPointer->Header.Checksum = 0;
      FadtPointer->Header.Checksum = (UINT8)(256-Checksum8((CHAR8*)FadtPointer,FadtPointer->Header.Length));
      DsdtLoaded = TRUE;
    }
  } 
  
		if (!DsdtLoaded) {
			// allocate space for fixes
			TableHeader = (EFI_ACPI_DESCRIPTION_HEADER*)(UINTN)FadtPointer->Dsdt;
			bufferLen = TableHeader->Length;
			DBG("DSDT len = 0x%x", bufferLen);
			bufferLen = bufferLen + bufferLen / 8;
			DBG(" new len = 0x%x\n", bufferLen);
			
			dsdt = EFI_SYSTEM_TABLE_MAX_ADDRESS;
			Status = gBS->AllocatePages(AllocateMaxAddress,
                                  EfiACPIReclaimMemory,
                                  EFI_SIZE_TO_PAGES(bufferLen),
                                  &dsdt);
			
			//if success insert dsdt pointer into ACPI tables
			if(!EFI_ERROR(Status)) {
				CopyMem((VOID*)(UINTN)dsdt, (VOID*)TableHeader, bufferLen);
				
				FadtPointer->Dsdt  = (UINT32)dsdt;
				FadtPointer->XDsdt = dsdt;
				// verify checksum
				FadtPointer->Header.Checksum = 0;
				FadtPointer->Header.Checksum = (UINT8)(256 - Checksum8((CHAR8*)FadtPointer, FadtPointer->Header.Length));
			}
		}
  if (gSettings.DebugDSDT) {
    DBG("Output DSDT before patch to /EFI/CLOVER/ACPI/origin/DSDT-or.aml\n");
    Status = egSaveFile(SelfRootDir, L"\\EFI\\CLOVER\\ACPI\\origin\\DSDT-or.aml",
                        (UINT8*)(UINTN)FadtPointer->XDsdt, bufferLen);    
  }
  //native DSDT or loaded we want to apply autoFix to this
  //  if (gSettings.FixDsdt) { //fix even with zero mask because we want to know PCIRootUID and CPUBase and count(?)
  DBG("Apply DsdtFixMask=0x%04x\n", gSettings.FixDsdt);
  FixBiosDsdt((UINT8*)(UINTN)FadtPointer->XDsdt);
  if (gSettings.DebugDSDT) { 
    for (Index=0; Index < 60; Index++) {
      CHAR16					DsdtPatchedName[128];
      UnicodeSPrint(DsdtPatchedName, 256, L"\\EFI\\CLOVER\\ACPI\\origin\\DSDT-pa%d.aml", Index);
      if(!FileExists(SelfRootDir, DsdtPatchedName)){
        Status = egSaveFile(SelfRootDir, DsdtPatchedName,
                            (UINT8*)(UINTN)FadtPointer->XDsdt, bufferLen);
        if (!EFI_ERROR(Status)) {
          break;
        }		
      }
    }
    if (EFI_ERROR(Status)) {
      DBG("...saving DSDT failed with status=%r\n", Status);
    }
  } 
  
  /*
  if (gSettings.bDropAPIC) {
    xf = ScanXSDT(APIC_SIGN);
    if(xf) { DropTableFromXSDT(APIC_SIGN); }
    rf = ScanRSDT(APIC_SIGN);
    if(rf) { DropTableFromRSDT(APIC_SIGN); }
    }
  if (gSettings.bDropMCFG) {
		xf = ScanXSDT(MCFG_SIGN);
		if(xf) { DropTableFromXSDT(MCFG_SIGN); }
		rf = ScanRSDT(MCFG_SIGN);
		if(rf) { DropTableFromRSDT(MCFG_SIGN); }
		}
  if (gSettings.bDropHPET) {
		xf = ScanXSDT(HPET_SIGN);
		if(xf) { DropTableFromXSDT(HPET_SIGN); }
		rf = ScanRSDT(HPET_SIGN);
		if(rf) { DropTableFromRSDT(HPET_SIGN); }
		}
  if (gSettings.bDropECDT) {
		xf = ScanXSDT(ECDT_SIGN);
		if(xf) { DropTableFromXSDT(ECDT_SIGN); }
		rf = ScanRSDT(ECDT_SIGN);
		if(rf) { DropTableFromRSDT(ECDT_SIGN); }
		}
  if (gSettings.bDropDMAR) {
		xf = ScanXSDT(DMAR_SIGN);
		if(xf) { DropTableFromXSDT(DMAR_SIGN); }
		rf = ScanRSDT(DMAR_SIGN);
		if(rf) { DropTableFromRSDT(DMAR_SIGN); }
  }
  if (gSettings.bDropBGRT) {
		xf = ScanXSDT(BGRT_SIGN);
		if(xf) { DropTableFromXSDT(BGRT_SIGN); }
		rf = ScanRSDT(BGRT_SIGN);
		if(rf) { DropTableFromRSDT(BGRT_SIGN); }
  }
  */
  // Drop tables by signature
  for (i = 0; i < gSettings.DropTableSignatureCount; ++i) {
    UINT32  Signature;
    CHAR8  *SignatureString = gSettings.DropTableSignatures[i];
    if (SignatureString == NULL) {
      continue;
    }
    Signature = SIGNATURE_32(SignatureString[0], SignatureString[1],
                             SignatureString[2], SignatureString[3]);
    xf = ScanXSDT(Signature);
    if(xf) {
      DropTableFromXSDT(Signature);
    }
    rf = ScanRSDT(Signature);
    if(rf) {
      DropTableFromRSDT(Signature);
    }
  }
  
  // Drop tables by name
  for (i = 0; i < gSettings.DropTableSignatureCount; ++i) {
    UINT64  Id;
    CHAR8  *IdString = gSettings.DropTableNames[i];
    if (IdString == NULL) {
      continue;
    }
    Id = *(UINT64 *)IdString;
    xf = ScanXSDTId(Id);
    if(xf) {
      DropTableFromXSDTId(Id);
    }
    rf = ScanRSDTId(Id);
    if(rf) {
      DropTableFromRSDTId(Id);
    }
  }

  if (gSettings.DropSSDT) {
    DropTableFromXSDT(EFI_ACPI_4_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE);
    DropTableFromRSDT(EFI_ACPI_4_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE);
  } else { //do the empty drop to clean xsdt
    DropTableFromXSDT(XXXX_SIGN);
    DropTableFromRSDT(XXXX_SIGN);
  }
  
  
  //find other ACPI tables
  for (Index = 0; Index < NUM_TABLES; Index++) {
    CHAR16* FullName = PoolPrint(L"%s\\%s", AcpiOemPath, ACPInames[Index]);
    Status = EFI_NOT_FOUND;
    // DBG("Looking for ACPI table %s from %s ... ", ACPInames[Index], AcpiOemPath);
    if (FileExists(SelfRootDir, FullName)) {
      DBG("Inserting %s from %s ... ", ACPInames[Index], AcpiOemPath);
      Status = egLoadFile(SelfRootDir, FullName, &buffer, &bufferLen);
      if (!EFI_ERROR(Status)) {
        //before insert we should checksum it
        if (buffer) {
          TableHeader = (EFI_ACPI_DESCRIPTION_HEADER*)buffer;
          TableHeader->Checksum = 0;
          TableHeader->Checksum = (UINT8)(256-Checksum8((CHAR8*)buffer, TableHeader->Length));
        }
        Status = InsertTable((VOID*)buffer, bufferLen);
      }
      DBG("%r\n", Status);
    }
  }
  
  //Slice - this is a time to patch MADT table. 
//  DBG("Fool proof: size of APIC NMI  = %d\n", sizeof(EFI_ACPI_2_0_LOCAL_APIC_NMI_STRUCTURE));
//  DBG("----------- size of APIC DESC = %d\n", sizeof(EFI_ACPI_2_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER));
//  DBG("----------- size of APIC PROC = %d\n", sizeof(EFI_ACPI_2_0_PROCESSOR_LOCAL_APIC_STRUCTURE));
  //
  // 1. For CPU base number 0 or 1.  codes from SunKi
  CPUBase = acpi_cpu_name[0][3] - '0'; //"CPU0"
  if ((UINT8)CPUBase > 11) {
    DBG("Abnormal CPUBase=%x will set to 0\n", CPUBase);
    CPUBase = 0;
  }
  ApicCPUNum = 0;  
  // 2. For absent NMI subtable
    xf = ScanXSDT(APIC_SIGN);
    if (xf) {
      ApicTable = (EFI_ACPI_DESCRIPTION_HEADER*)(UINTN)(*xf);
      ApicLen = ApicTable->Length;
      ProcLocalApic = (EFI_ACPI_2_0_PROCESSOR_LOCAL_APIC_STRUCTURE *)(UINTN)(*xf + sizeof(EFI_ACPI_2_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER));
      //determine first ID of CPU. This must be 0 for Mac and for good Hack
      // but = 1 for stupid ASUS
      //
      if (ProcLocalApic->Type == 0) {
        ApicCPUBase = ProcLocalApic->AcpiProcessorId; //we want first instance
      } 

      while ((ProcLocalApic->Type == 0) && (ProcLocalApic->Length == 8)) {
        ProcLocalApic++;
        ApicCPUNum++;
        if (ApicCPUNum > 16) {
          DBG("Out of control with CPU numbers\n");          
          break;
        }
      }
      //fool proof
      if ((ApicCPUNum == 0) || (ApicCPUNum > 16)) {
        ApicCPUNum = gCPUStructure.Threads;
      }
      
      DBG(" CPUBase=%d and ApicCPUBase=%d ApicCPUNum=%d\n", CPUBase, ApicCPUBase, ApicCPUNum);
 //reallocate table  
      if (gSettings.PatchNMI) {
        
        BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS;
        Status=gBS->AllocatePages(AllocateMaxAddress, EfiACPIReclaimMemory, 1, &BufferPtr);
        if(!EFI_ERROR(Status))
        {
          //save old table and drop it from XSDT
          CopyMem((VOID*)(UINTN)BufferPtr, ApicTable, ApicTable->Length);
          DropTableFromXSDT(APIC_SIGN);
          ApicTable = (EFI_ACPI_DESCRIPTION_HEADER*)(UINTN)BufferPtr;
          ApicTable->Revision = EFI_ACPI_4_0_MULTIPLE_APIC_DESCRIPTION_TABLE_REVISION;
          CopyMem(&ApicTable->OemId, oemID, 6);
          CopyMem(&ApicTable->OemTableId, oemTableID, 8);
          ApicTable->OemRevision = 0x00000001;
          CopyMem(&ApicTable->CreatorId, creatorID, 4);
          
          SubTable = (UINT8*)((UINTN)BufferPtr + sizeof(EFI_ACPI_2_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER));
          Index = CPUBase;
          while (*SubTable != EFI_ACPI_4_0_LOCAL_APIC_NMI) {
            DBG("Found subtable in MADT: type=%d\n", *SubTable);
            //xxx - OSX paniced
  /*          
            if (*SubTable == EFI_ACPI_4_0_PROCESSOR_LOCAL_APIC) {
              ProcLocalApic = (EFI_ACPI_2_0_PROCESSOR_LOCAL_APIC_STRUCTURE *)SubTable;
              ProcLocalApic->AcpiProcessorId = Index;
              Index++;
            }
  */          
            bufferLen = (UINTN)SubTable[1];
            SubTable += bufferLen;
            if (((UINTN)SubTable - (UINTN)BufferPtr) >= ApicTable->Length) {
              break;
            }
          }
          
          if (*SubTable == EFI_ACPI_4_0_LOCAL_APIC_NMI) {
            DBG("LocalApicNMI is already present, no patch needed\n");
          } else {
            LocalApicNMI = (EFI_ACPI_2_0_LOCAL_APIC_NMI_STRUCTURE*)((UINTN)ApicTable + ApicTable->Length);
            for (Index = 0; Index < ApicCPUNum; Index++) {
              LocalApicNMI->Type = EFI_ACPI_4_0_LOCAL_APIC_NMI;
              LocalApicNMI->Length = sizeof(EFI_ACPI_4_0_LOCAL_APIC_NMI_STRUCTURE);
              LocalApicNMI->AcpiProcessorId = (UINT8)(ApicCPUBase + Index);
              LocalApicNMI->Flags = 5;
              LocalApicNMI->LocalApicLint = 1;
              LocalApicNMI++;
              ApicTable->Length += LocalApicNMI->Length;
            }
            DBG("ApicTable new Length=%d\n", ApicTable->Length);
            // insert corrected MADT
          }
          ApicTable->Checksum = 0;
          ApicTable->Checksum = (UINT8)(256-Checksum8((CHAR8*)ApicTable, ApicTable->Length));
          Status = InsertTable((VOID*)ApicTable, ApicTable->Length);
          if (!EFI_ERROR(Status)) {
            DBG("New APIC table successfully inserted\n");
          }
          Status = egSaveFile(SelfRootDir, PatchedAPIC, (UINT8 *)ApicTable, ApicTable->Length);
          if (EFI_ERROR(Status)) {
            Status = egSaveFile(NULL, PatchedAPIC,  (UINT8 *)ApicTable, ApicTable->Length);
          }          
          if (!EFI_ERROR(Status)) {
            DBG("Patched APIC table saved into efi/acpi/patched \n");
          }
       }
      } 
    } 
      else DBG("No APIC table Found !!!\n");
  //fool proof
  //It's appeared that APIC CPU number is wrong
  //so return to CPU structure
//  if ((ApicCPUNum == 0) || (ApicCPUNum > 16)) {
    if (gCPUStructure.Threads >= gCPUStructure.Cores) {
      ApicCPUNum = gCPUStructure.Threads;
    } else {
      ApicCPUNum = gCPUStructure.Cores;
    }
//  }
    /*
     At this moment we have CPU numbers from DSDT - acpi_cpu_num
     and from CPU characteristics gCPUStructure
     Also we had the number from APIC table ApicCPUNum
     What to choose? 
     Since rev745 I will return to acpi_cpu_count global variable
    */
  if (acpi_cpu_count) {
    ApicCPUNum = acpi_cpu_count;
  }
    
  if (gSettings.GeneratePStates) {
    Status = EFI_NOT_FOUND;
    Ssdt = generate_pss_ssdt(CPUBase, ApicCPUNum);
    if (Ssdt) {
      Status = InsertTable((VOID*)Ssdt, Ssdt->Length);      
    }
    if(EFI_ERROR(Status)){
      DBG("GeneratePStates failed Status=%r\n", Status);
    }
  }
  
  if (gSettings.GenerateCStates) {
    Status = EFI_NOT_FOUND;
    Ssdt = generate_cst_ssdt(FadtPointer, CPUBase, ApicCPUNum);
    if (Ssdt) {
      Status = InsertTable((VOID*)Ssdt, Ssdt->Length);      
    }
    if(EFI_ERROR(Status)){
      DBG("GenerateCStates failed Status=%r\n", Status);
    }
  }
  
  
  if (Rsdt) {
    Rsdt->Header.Checksum = 0;
    Rsdt->Header.Checksum = (UINT8)(256-Checksum8((CHAR8*)Rsdt, Rsdt->Header.Length));
  }
  if (Xsdt) {
    Xsdt->Header.Checksum = 0;
    Xsdt->Header.Checksum = (UINT8)(256-Checksum8((CHAR8*)Xsdt, Xsdt->Header.Length));
  }
  
  return EFI_SUCCESS;
}



/**
 * Searches for TableName in AcpiOemPath or PathPatched dirs and loads it
 * to Buffer if found. Buffer is allocated here and should be released
 * by caller.
 */
EFI_STATUS LoadAcpiTable
(
  CHAR16                *AcpiOemPath,
  CHAR16                *PathPatched,
  CHAR16                *TableName,
  UINT8                 **Buffer,
  UINTN                 *BufferLen
)
{
  EFI_STATUS            Status;
  CHAR16*               TmpStr;
  
  Status = EFI_NOT_FOUND;
  
  // checking \EFI\OEM\xxx\ACPI\patched dir
  TmpStr = PoolPrint(L"%s\\%s", AcpiOemPath, TableName);
  if (FileExists(SelfRootDir, TmpStr))
  {
    DBG("found %s\n", TmpStr);
    Status = egLoadFile(SelfRootDir, TmpStr, Buffer, BufferLen);
  }
  FreePool(TmpStr);
  
  if (EFI_ERROR(Status))
  {
    // checking \EFI\ACPI\patched dir
    TmpStr = PoolPrint(L"%s\\%s", PathPatched, TableName);
    if (FileExists(SelfRootDir, TmpStr)) {
      DBG("found %s\n", TmpStr);
      Status = egLoadFile(SelfRootDir, TmpStr, Buffer, BufferLen);
    }
    FreePool(TmpStr);
  }
  
  return Status;
}

/**
 * Searches for DSDT in AcpiOemPath or PathPatched dirs and inserts it
 * to FadtPointer if found.
 */
EFI_STATUS LoadAndInjectDSDT(CHAR16 *AcpiOemPath, CHAR16 *PathPatched, EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE *FadtPointer)
{
	EFI_STATUS            Status;
	UINT8                 *Buffer = NULL;
	UINTN        				  BufferLen = 0;
	EFI_PHYSICAL_ADDRESS  Dsdt;
  
  // load if exists
  Status = LoadAcpiTable(AcpiOemPath, PathPatched, gSettings.DsdtName, &Buffer, &BufferLen);
  
  if (!EFI_ERROR(Status))
  {
    // loaded - allocate EfiACPIReclaim
    Dsdt = EFI_SYSTEM_TABLE_MAX_ADDRESS; //0xFE000000;
    Status = gBS->AllocatePages (
                                 AllocateMaxAddress,
                                 EfiACPIReclaimMemory,
                                 EFI_SIZE_TO_PAGES(BufferLen),
                                 &Dsdt
                                 );
    
    if(!EFI_ERROR(Status))
    {
      // copy DSDT into EfiACPIReclaim block
      CopyMem((VOID*)(UINTN)Dsdt, Buffer, BufferLen);
      
      // update FADT
      FadtPointer->Dsdt  = (UINT32)Dsdt;
      FadtPointer->XDsdt = Dsdt;
      
      // and checksum
      FadtPointer->Header.Checksum = 0;
      FadtPointer->Header.Checksum = (UINT8)(256-Checksum8((CHAR8*)FadtPointer, FadtPointer->Header.Length));
      DBG("DSDT at 0x%x injected to FADT 0x%p\n", Dsdt, FadtPointer);
    }
    
    // Buffer allocated with AllocatePages() and we do not know how many pages is allocated
    gBS->FreePages((EFI_PHYSICAL_ADDRESS)(UINTN)Buffer, EFI_SIZE_TO_PAGES(BufferLen));
  }
  
  return Status;
}

/**
 * Searches for TableName in AcpiOemPath or PathPatched dirs and inserts it
 * to Rsdt and/or Xsdt (globals) if found.
 */
EFI_STATUS LoadAndInjectAcpiTable(CHAR16 *AcpiOemPath, CHAR16 *PathPatched, CHAR16 *TableName)
{
	EFI_STATUS                    Status;
	UINT8                         *Buffer = NULL;
	UINTN                         BufferLen = 0;
  EFI_ACPI_DESCRIPTION_HEADER   *TableHeader = NULL;
  
  
  // load if exists
  Status = LoadAcpiTable(AcpiOemPath, PathPatched, TableName, &Buffer, &BufferLen);
  
  if(!EFI_ERROR(Status))
  {
    //before insert we should checksum it
    if (Buffer) {
      TableHeader = (EFI_ACPI_DESCRIPTION_HEADER*)Buffer;
      TableHeader->Checksum = 0;
      TableHeader->Checksum = (UINT8)(256-Checksum8((CHAR8*)Buffer, TableHeader->Length));
    }

    // loaded - insert it into XSDT/RSDT
    Status = InsertTable((VOID*)Buffer, BufferLen);
    
    if(!EFI_ERROR(Status))
    {
      DBG("Table %s inserted.\n", TableName);
      
      // if this was SLIC, then update IDs in XSDT/RSDT
      
      if (TableHeader->Signature == SIGNATURE_32('S', 'L', 'I', 'C'))
      {
        if (Rsdt)
        {
          DBG("SLIC: Rsdt OEMid '%6.6a', TabId '%8.8a'", (CHAR8*)&Rsdt->Header.OemId, (CHAR8*)&Rsdt->Header.OemTableId);
          CopyMem((CHAR8 *)&Rsdt->Header.OemId, (CHAR8 *)&TableHeader->OemId, 6);
          Rsdt->Header.OemTableId = TableHeader->OemTableId;
          DBG(" to OEMid '%6.6a', TabId '%8.8a'\n", (CHAR8*)&Rsdt->Header.OemId, (CHAR8*)&Rsdt->Header.OemTableId);
        }
        if (Xsdt)
        {
          DBG("SLIC: Xsdt OEMid '%6.6a', TabId '%8.8a'", (CHAR8*)&Xsdt->Header.OemId, (CHAR8*)&Xsdt->Header.OemTableId);
          CopyMem((CHAR8 *)&Xsdt->Header.OemId, (CHAR8 *)&TableHeader->OemId, 6);
          Xsdt->Header.OemTableId = TableHeader->OemTableId;
          DBG(" to OEMid '%6.6a', TabId '%8.8a'\n", (CHAR8*)&Xsdt->Header.OemId, (CHAR8*)&Xsdt->Header.OemTableId);
        }
      }
    }
    else
    {
      DBG("Insert return status %r\n", Status);
    }
    
    // buffer allocated with AllocatePages() and we do not know how many pages is allocated
    gBS->FreePages((EFI_PHYSICAL_ADDRESS)(UINTN)Buffer, EFI_SIZE_TO_PAGES(BufferLen));
    
  } // if table loaded
  
  return Status;
}

/**
 * Patches UEFI ACPI tables with tables found in OsSubdir.
 */
EFI_STATUS PatchACPI_OtherOS(CHAR16* OsSubdir, BOOLEAN DropSSDT)
{
	EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER    *RsdPointer;
	EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE       *FadtPointer;
	
	EFI_STATUS            Status = EFI_SUCCESS;
	UINTN                 Index;
	CHAR16*               PathPatched;
  CHAR16*               AcpiOemPath;
  
  
  //
  // Search for RSDP in UEFI SystemTable/ConfigTable (first Acpi 2.0, then 1.0)
  //
  RsdPointer = NULL;
  
  Status = EfiGetSystemConfigurationTable (&gEfiAcpi20TableGuid, (VOID **) &RsdPointer);
  if (RsdPointer != NULL)
  {
    DBG("Found Acpi 2.0 RSDP 0x%x\n", RsdPointer);
  }
  else
  {
    Status = EfiGetSystemConfigurationTable (&gEfiAcpi10TableGuid, (VOID **) &RsdPointer);
    if (RsdPointer != NULL)
    {
      DBG("Found Acpi 1.0 RSDP 0x%x\n", RsdPointer);
    }
  }
  // if RSDP not found - quit
	if (!RsdPointer)
  {
		return EFI_UNSUPPORTED;
	}
  
  //
  // Find RSDT and/or XSDT
  //
  Rsdt = (RSDT_TABLE*)(UINTN)(RsdPointer->RsdtAddress);
  if (Rsdt != NULL && Rsdt->Header.Signature != EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
    Rsdt = NULL;
  }
  DBG("RSDT at %p\n", Rsdt);
  
  // check for XSDT
  Xsdt = NULL;
  if (RsdPointer->Revision >=2 && (RsdPointer->XsdtAddress < (UINT64)((UINTN)(-1))))
  {
    Xsdt = (XSDT_TABLE*)(UINTN)RsdPointer->XsdtAddress;
    if (Xsdt != NULL && Xsdt->Header.Signature != EFI_ACPI_2_0_EXTENDED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
      Xsdt = NULL;
    }
  }
  DBG("XSDT at %p\n", Xsdt);
  
  // if RSDT and XSDT not found - quit
  if (Rsdt == NULL && Xsdt == NULL)
  {
    return EFI_UNSUPPORTED;
  }

  //
  // Take FADT (FACP) from XSDT or RSDT (always first entry)
  //
  FadtPointer = NULL;
  if (Xsdt)
  {
    FadtPointer = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE*)(UINTN)(Xsdt->Entry);
  }
  else if (Rsdt)
  {
    FadtPointer = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE*)(UINTN)(Rsdt->Entry);
  }
  DBG("FADT pointer = %p\n", FadtPointer);

  // if not found - quit
  if(FadtPointer == NULL)
  {
    return EFI_NOT_FOUND;
  }
  
  
  //
  // Inject/drop tables
  //
  
  // prepare dirs that will be searched for custom ACPI tables
  AcpiOemPath = PoolPrint(L"%s\\ACPI\\%s", OEMPath, OsSubdir);
  PathPatched = PoolPrint(L"\\EFI\\CLOVER\\ACPI\\%s", OsSubdir);
  if (!FileExists(SelfRootDir, AcpiOemPath) && !FileExists(SelfRootDir, PathPatched))
  {
    DBG("Dir %s not found. No patching will be done.\n", OsSubdir);
    return EFI_NOT_FOUND;
  }
	
  //
  // Inject DSDT
  //
  Status = LoadAndInjectDSDT(AcpiOemPath, PathPatched, FadtPointer);
  
  //
  // Drop SSDT if requested
  //
  if (DropSSDT)
  {
    DropTableFromXSDT(EFI_ACPI_4_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE);
    DropTableFromRSDT(EFI_ACPI_4_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE);
  }
  
  //
  // find and inject other ACPI tables
  //
  for (Index = 0; Index < NUM_TABLES; Index++)
  {
    Status = LoadAndInjectAcpiTable(AcpiOemPath, PathPatched, ACPInames[Index]);
  }
  
  //
  // fix checksums
  //
  if (Rsdt) {
    Rsdt->Header.Checksum = 0;
    Rsdt->Header.Checksum = (UINT8)(256-Checksum8((CHAR8*)Rsdt, Rsdt->Header.Length));
  }
  if (Xsdt) {
    Xsdt->Header.Checksum = 0;
    Xsdt->Header.Checksum = (UINT8)(256-Checksum8((CHAR8*)Xsdt, Xsdt->Header.Length));
  }
  
  // release mem
  if (AcpiOemPath) FreePool(AcpiOemPath);
  if (PathPatched) FreePool(PathPatched);
  
  return EFI_SUCCESS;
}

