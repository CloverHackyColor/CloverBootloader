/*
 * FixBiosDsdt.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_FIXBIOSDSDT_H_
#define PLATFORM_FIXBIOSDSDT_H_

extern "C" {
#include <IndustryStandard/Acpi20.h>
}

#include "../cpp_foundation/XBuffer.h"
#include "../Platform/MacOsVersion.h"
#include "../include/DsdtFixList.h"

struct _oper_region {
  CHAR8  Name[8];
  UINT32 Address;
  struct _oper_region *next;
};
typedef struct _oper_region OPER_REGION;


void
FixBiosDsdt (
  UINT8                                     *Dsdt,
  EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE *fadt,
  const MacOsVersion&                        OSVersion
  );


void
RenameDevices(UINT8* table);

void
GetBiosRegions (
  UINT8  *buffer
  );

INT32
FindBin (
  UINT8  *Array,
  UINT32 ArrayLen,
  const UINT8  *Pattern,
  UINT32 PatternLen
  );
INT32 FindBin (UINT8 *dsdt, size_t len, const XBuffer<UINT8>& bin);


UINT32 FixAny (UINT8* dsdt, UINT32 len, const XBuffer<UINT8> ToFind, const XBuffer<UINT8> ToReplace);
UINT32 FixRenameByBridge2 (UINT8* dsdt, UINT32 len, const XBuffer<UINT8>& TgtBrgName, const XBuffer<UINT8>& ToFind, const XBuffer<UINT8>& ToReplace);


#endif /* PLATFORM_FIXBIOSDSDT_H_ */
