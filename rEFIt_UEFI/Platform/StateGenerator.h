/*
 * Copyright 2008 mackerintel
 * 2010 mojodojo
 */

#ifndef __LIBSAIO_ACPI_PATCHER_H
#define __LIBSAIO_ACPI_PATCHER_H

#include <IndustryStandard/Acpi10.h>
#include <IndustryStandard/Acpi20.h>
#include "AmlGenerator.h"

#ifndef DEBUG_AML
#ifndef DEBUG_ALL
#define DEBUG_AML 1
#else
#define DEBUG_AML DEBUG_ALL
#endif
#endif

#if DEBUG_AML==0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_AML, __VA_ARGS__)	
#endif


typedef EFI_ACPI_DESCRIPTION_HEADER SSDT_TABLE;


SSDT_TABLE *generate_pss_ssdt(UINTN Number);
SSDT_TABLE *generate_cst_ssdt(EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE* fadt, UINTN Number);

#endif /* !__LIBSAIO_ACPI_PATCHER_H */
