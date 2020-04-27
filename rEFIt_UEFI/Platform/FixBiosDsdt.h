/*
 * FixBiosDsdt.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_FIXBIOSDSDT_H_
#define PLATFORM_FIXBIOSDSDT_H_



//DSDT fixes MASK
//0x00FF
#define FIX_DTGP      bit(0)
#define FIX_WARNING   bit(1)
#define FIX_SHUTDOWN  bit(2)
#define FIX_MCHC      bit(3)
#define FIX_HPET      bit(4)
#define FIX_LPC       bit(5)
#define FIX_IPIC      bit(6)
#define FIX_SBUS      bit(7)
//0xFF00
#define FIX_DISPLAY   bit(8)
#define FIX_IDE       bit(9)
#define FIX_SATA      bit(10)
#define FIX_FIREWIRE  bit(11)
#define FIX_USB       bit(12)
#define FIX_LAN       bit(13)
#define FIX_WIFI      bit(14)
#define FIX_HDA       bit(15)
//new bits 16-31 0xFFFF0000
//#define FIX_NEW_WAY   bit(31) will be reused
#define FIX_DARWIN    bit(16)
#define FIX_RTC       bit(17)
#define FIX_TMR       bit(18)
#define FIX_IMEI      bit(19)
#define FIX_INTELGFX  bit(20)
#define FIX_WAK       bit(21)
#define FIX_UNUSED    bit(22)
#define FIX_ADP1      bit(23)
#define FIX_PNLF      bit(24)
#define FIX_S3D       bit(25)
#define FIX_ACST      bit(26)
#define FIX_HDMI      bit(27)
#define FIX_REGIONS   bit(28)
#define FIX_HEADERS   bit(29)
#define FIX_MUTEX     bit(30)


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
  const UINT8  *Pattern,
  UINT32 PatternLen
  );


UINT32
FixAny (
  UINT8* dsdt,
  UINT32 len,
  const UINT8* ToFind,
  UINT32 LenTF,
  const UINT8* ToReplace,
  UINT32 LenTR
  );



#endif /* PLATFORM_FIXBIOSDSDT_H_ */
