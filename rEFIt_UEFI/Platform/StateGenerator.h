/*
 * Copyright 2008 mackerintel
 * 2010 mojodojo
 */

#ifndef __LIBSAIO_ACPI_PATCHER_H
#define __LIBSAIO_ACPI_PATCHER_H

#include "AmlGenerator.h"

#ifndef DEBUG_ACPI
#define DEBUG_ACPI 1
#endif

#if DEBUG_ACPI==2
#define DBG(x...)  AsciiPrint(x)
#elif DEBUG_ACPI==1
#define DBG(x...)  MsgLog(x)
#else
#define DBG(x...)
#endif

/*

#pragma pack(1)
struct acpi_2_ssdt {
	CHAR8         Signature[4]; //0
	UINT32        Length;       //4
	UINT8         Revision;     //8
	UINT8         Checksum;     //9
	CHAR8         OEMID[6];     //0xa
	CHAR8         OEMTableId[8];  //0x10
	UINT32        OEMRevision;    //0x14
	UINT32        CreatorId;      //0x18  
	UINT32        CreatorRevision; //0x20
} __attribute__((packed));    //0x24
#pragma pack()
*/
typedef EFI_ACPI_DESCRIPTION_HEADER SSDT_TABLE;


SSDT_TABLE *generate_pss_ssdt(VOID);
SSDT_TABLE *generate_cst_ssdt(EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE* fadt);


#endif /* !__LIBSAIO_ACPI_PATCHER_H */
