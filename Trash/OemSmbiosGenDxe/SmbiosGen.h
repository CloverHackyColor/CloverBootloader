/** @file

Copyright (c) 2009 - 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name: 

  DataHubGen.h

Abstract:

**/

#ifndef _SMBIOS_GEN_H_
#define _SMBIOS_GEN_H_

#include <FrameworkDxe.h>
#include <IndustryStandard/Smbios.h>
//#include "SmBios.h"

#include <Guid/HobList.h>
#include <Guid/SmBios.h>
#include <Guid/DataHubRecords.h>

#include <Protocol/Smbios.h>
#include <Protocol/FrameworkHii.h>
//#include <Protocol/HiiDatabase.h>

#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/HobLib.h>
#include <Library/UefiBootServicesTableLib.h>
//#include <Library/HiiLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>


SMBIOS_STRUCTURE_POINTER
GetSmbiosTableFromType (
  IN SMBIOS_TABLE_ENTRY_POINT  *Smbios,
  IN UINT8                 Type,
  IN UINTN                 Index
  );

CHAR8 *
GetSmbiosString (
  IN SMBIOS_STRUCTURE_POINTER  SmbiosTable,
  IN SMBIOS_TABLE_STRING       String
  );

/**
  Logs SMBIOS record.

  @param  Smbios   Pointer to SMBIOS protocol instance.
  @param  Buffer   Pointer to the data buffer.
  @return handle of the new table

**/
EFI_SMBIOS_HANDLE
LogSmbiosData (
  IN   EFI_SMBIOS_PROTOCOL        *Smbios,
  IN   UINT8                      *Buffer
  );

#endif
