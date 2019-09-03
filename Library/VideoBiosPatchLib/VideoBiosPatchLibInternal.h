/** @file
  Internal include file.
**/

#ifndef __VIDEO_BIOS_PATCH_LIB_INTERNAL_H__
#define __VIDEO_BIOS_PATCH_LIB_INTERNAL_H__


#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemLogLib.h>
#include <Library/VideoBiosPatchLib.h>

#include <Protocol/LegacyRegion.h>
#include <Protocol/LegacyRegion2.h>
#include <Protocol/EdidActive.h>

#include <IndustryStandard/AtomBios.h>

//#include "shortatombios.h"
#include "915resolution.h"
#include "edid.h"


#define DEBUG_VBP 1

#if DEBUG_VBP == 1
#define DBG(...)  MemLog(TRUE, 1, __VA_ARGS__)
#else
#define DBG(...)
#endif


//
// Video bios start addr and size
//
#define VBIOS_START         0xc0000
#define VBIOS_SIZE          0x10000


//
// Block port access in 915resolution.c - not needed
//
#define outl(...)
#define inl(...)    0
#define outb(...)
#define inb(...)    0


//
// Temp var for passing Edid to readEDID() in edid.c
//
extern UINT8     *mEdid;

#endif // __VIDEO_BIOS_PATCH_LIB_INTERNAL_H__
