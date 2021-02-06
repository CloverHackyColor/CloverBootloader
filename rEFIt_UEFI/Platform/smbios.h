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

typedef struct {
  BOOLEAN  InUse;
  UINT8   Type;
  UINT16  pad0;
  UINT32  pad1;
  UINT32  ModuleSize;
  UINT32  Frequency;
  CONST CHAR8*  Vendor;
  CHAR8*  PartNo;
  CHAR8*  SerialNo;
} RAM_SLOT_INFO;

typedef struct
{
  UINT32        Frequency;
  UINT32        Divider;
  UINT8         TRC;
  UINT8         TRP;
  UINT8         RAS;
  UINT8         Channels;
  UINT8         Slots;
  UINT8         Type;
  UINT8         SPDInUse;
  UINT8         SMBIOSInUse;
  UINT8         UserInUse;
  UINT8         UserChannels;
  UINT8         pad[2];

  RAM_SLOT_INFO SPD[MAX_RAM_SLOTS * 4];
  RAM_SLOT_INFO SMBIOS[MAX_RAM_SLOTS * 4];
  RAM_SLOT_INFO User[MAX_RAM_SLOTS * 4];

} MEM_STRUCTURE;


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
