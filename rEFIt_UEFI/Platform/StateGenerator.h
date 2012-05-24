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


typedef EFI_ACPI_DESCRIPTION_HEADER SSDT_TABLE;


SSDT_TABLE *generate_pss_ssdt(UINT8 FirstID, UINTN Number);
SSDT_TABLE *generate_cst_ssdt(EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE* fadt, UINT8 FirstID, UINTN Number);


#endif /* !__LIBSAIO_ACPI_PATCHER_H */
