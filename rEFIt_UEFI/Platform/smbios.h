/*
 * smbios.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_SMBIOS_H_
#define PLATFORM_SMBIOS_H_

extern "C" {
#include <IndustryStandard/AppleSmBios.h>
}

// The maximum number of RAM slots to detect
// even for 3-channels chipset X58 there are no more then 8 slots
#define MAX_RAM_SLOTS 24
// The maximum sane frequency for a RAM module
#define MAX_RAM_FREQUENCY 5000

class RAM_SLOT_INFO {
public:
  UINT32  ModuleSize = UINT32();
  UINT32  Frequency = UINT32();
  XString8 Vendor = XString8();
  XString8 PartNo = XString8();
  XString8 SerialNo = XString8();
  UINT8   Type = UINT8();
  bool  InUse = bool();
  
  #if __cplusplus > 201703L
    bool operator == (const RAM_SLOT_INFO&) const = default;
  #endif
  bool isEqual(const RAM_SLOT_INFO& other) const
  {
    if ( !(ModuleSize == other.ModuleSize ) ) return false;
    if ( !(Frequency == other.Frequency ) ) return false;
    if ( !(Vendor == other.Vendor ) ) return false;
    if ( !(PartNo == other.PartNo ) ) return false;
    if ( !(SerialNo == other.SerialNo ) ) return false;
    if ( !(Type == other.Type ) ) return false;
    if ( !(InUse == other.InUse ) ) return false;
    return true;
  }
};

class MEM_STRUCTURE
{
public:
  UINT32        Frequency = UINT32();
  UINT32        Divider = UINT32();
  UINT8         TRC = UINT8();
  UINT8         TRP = UINT8();
  UINT8         RAS = UINT8();
  UINT8         Channels = UINT8();
  UINT8         Slots = UINT8();
  UINT8         Type = UINT8();
  UINT8         SPDInUse = UINT8();
  UINT8         SMBIOSInUse = UINT8();

  RAM_SLOT_INFO SPD[MAX_RAM_SLOTS * 4];
  RAM_SLOT_INFO SMBIOS[MAX_RAM_SLOTS * 4];

};


extern APPLE_SMBIOS_STRUCTURE_POINTER SmbiosTable;

extern MEM_STRUCTURE            gRAM;
extern BOOLEAN                        gMobile;



UINTN
iStrLen(
  CONST CHAR8* String,
  UINTN  MaxLen
  );

EFI_STATUS
PrepatchSmbios (void);

void
PatchSmbios (void);

void
FinalizeSmbios (void);



#endif /* PLATFORM_SMBIOS_H_ */
