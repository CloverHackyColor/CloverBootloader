/*
 * FixBiosDsdt.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_FIXBIOSDSDT_H_
#define PLATFORM_FIXBIOSDSDT_H_


struct _oper_region {
  CHAR8  Name[8];
  UINT32 Address;
  struct _oper_region *next;
};
typedef struct _oper_region OPER_REGION;


VOID
FixBiosDsdt (
  UINT8                                     *Dsdt,
  EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE *fadt,
  CHAR8                                     *OSVersion
  );


VOID
RenameDevices(UINT8* table);

VOID
GetBiosRegions (
  UINT8  *buffer
  );

INT32
FindBin (
  UINT8  *Array,
  UINT32 ArrayLen,
  UINT8  *Pattern,
  UINT32 PatternLen
  );


UINT32
FixAny (
  UINT8* dsdt,
  UINT32 len,
  UINT8* ToFind,
  UINT32 LenTF,
  UINT8* ToReplace,
  UINT32 LenTR
  );



#endif /* PLATFORM_FIXBIOSDSDT_H_ */
