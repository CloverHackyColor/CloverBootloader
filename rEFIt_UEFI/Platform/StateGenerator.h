/*
 * Copyright 2008 mackerintel
 * 2010 mojodojo
 */

#ifndef __LIBSAIO_ACPI_PATCHER_H
#define __LIBSAIO_ACPI_PATCHER_H

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


SSDT_TABLE *generate_pss_ssdt(UINT8 FirstID, UINTN Number);
SSDT_TABLE *generate_cst_ssdt(EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE* fadt, UINT8 FirstID, UINTN Number);

//TODO - credits to kozlek and abxite
// it should be for CPU0 in case of IvyBridge model=3A
/*
14 28 5F 44 53 4D 04 70 12 13 02 0D 70 6C 75 67
69 6E 2D 74 79 70 65 00 11 03 01 01 60 44 54 47
50 68 69 6A 6B 71 60 A4 60

Method (_DSM, 4, NotSerialized)
{
  Store (Package (0x02)
         {
           "plugin-type",
           Buffer (One)
           {
             0x01
           }
         }, Local0)
  DTGP (Arg0, Arg1, Arg2, Arg3, RefOf (Local0))
  Return (Local0)
}
*/

#endif /* !__LIBSAIO_ACPI_PATCHER_H */
