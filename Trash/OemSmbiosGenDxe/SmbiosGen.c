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
2012 Slice - copy OEM SMBIOS
**/

#include "SmbiosGen.h"

/*
 kSMBType32BitMemoryErrorInfo        = 18,
 kSMBType64BitMemoryErrorInfo        = 33,

 */

EFI_SMBIOS_PROTOCOL         *gSmbios;
/*
EFI_STATUS
GetSystemConfigurationTable (
  IN EFI_GUID *TableGuid,
  OUT VOID **Table
  )
 */
/*++

Routine Description:

  Get table from configuration table by name

Arguments:

  TableGuid       - Table name to search
  
  Table           - Pointer to the table caller wants

Returns: 

  EFI_NOT_FOUND   - Not found the table
  
  EFI_SUCCESS     - Found the table

--*/
/*
{
  UINTN Index;

  *Table = NULL;
  for (Index = 0; Index < gST->NumberOfTableEntries; Index++) {
    if (CompareGuid (TableGuid, &(gST->ConfigurationTable[Index].VendorGuid))) {
      *Table = gST->ConfigurationTable[Index].VendorTable;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}
*/
VOID *
GetSmbiosTablesFromConfigTables (
  VOID
  )
{
  EFI_STATUS              Status;
  EFI_PHYSICAL_ADDRESS       *Table;
  
  Status = EfiGetSystemConfigurationTable (&gEfiSmbiosTableGuid, (VOID **)  &Table);
  if (EFI_ERROR (Status) || Table == NULL) {
	  Table = NULL;
	//  Print(L"GetSmbiosTablesFromConfigTables: not found\n");
  }

  return Table;
}

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
InstallMiscSmbios (  
					 IN VOID          *Smbios
					 )
{
	SMBIOS_STRUCTURE_POINTER          SmbiosTable;
	UINTN	i, j;
	//
  for (j=0; j<256; j++) {
    for (i=0; i<128; i++) {
      SmbiosTable = GetSmbiosTableFromType ((SMBIOS_TABLE_ENTRY_POINT *)Smbios, (UINT8)j, i);
      if (SmbiosTable.Raw == NULL) {			
        break;
      }		
      //
      // Record Smbios Type j index i
      //
      LogSmbiosData(gSmbios, (UINT8*)SmbiosTable.Type0);		
    }    
  }
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
    // Print(L"SmbiosGenEntrypoint: GetSmbiosTablesFromHob not found\n");
    
    Smbios = GetSmbiosTablesFromConfigTables();
    if (Smbios == NULL) {
        // Print(L"SmbiosGenEntrypoint: GetSmbiosTablesFromConfigTables not found\n");
      return EFI_NOT_FOUND;
    }
  }

  Status = gBS->LocateProtocol (
                  &gEfiSmbiosProtocolGuid,
                  NULL,
                  (VOID**)&gSmbios
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  InstallMiscSmbios(Smbios);

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
