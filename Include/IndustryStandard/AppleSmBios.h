/** @file
  Apple-Specific Definitions of SMBIOS Table Specification v3.0.0.

Copyright (c) 2006 - 2015, Intel Corporation. All rights reserved.<BR>
(C) Copyright 2015 Hewlett Packard Enterprise Development LP<BR>
This program and the accompanying materials are licensed and made available under 
the terms and conditions of the BSD License that accompanies this distribution.  
The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.                                          
    
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#ifndef __APPLE_SMBIOS_STANDARD_H__
#define __APPLE_SMBIOS_STANDARD_H__

#include <IndustryStandard/SmBios.h>

///
/// Apple Firmware Volume.
///
enum {
  FW_REGION_RESERVED   = 0,
  FW_REGION_RECOVERY   = 1,
  FW_REGION_MAIN       = 2,
  FW_REGION_NVRAM      = 3,
  FW_REGION_CONFIG     = 4,
  FW_REGION_DIAGVAULT  = 5,
  NUM_FLASHMAP_ENTRIES = 8
};

#pragma pack(1)

///
/// Apple Firmware Volume - Region Info Type.
///
typedef struct {
  UINT32                StartAddress;
  UINT32                EndAddress;
} FW_REGION_INFO;

///
/// Apple Firmware Volume (Type 128).
///
typedef struct {
  SMBIOS_STRUCTURE      Hdr;
  UINT8                 RegionCount;
  UINT8                 Reserved[3];
  UINT32                FirmwareFeatures;
  UINT32                FirmwareFeaturesMask;
  UINT8                 RegionType[NUM_FLASHMAP_ENTRIES];
  FW_REGION_INFO        FlashMap[NUM_FLASHMAP_ENTRIES];
} SMBIOS_TABLE_TYPE128;

///
/// Apple Memory SPD Data (Type 130).
///
typedef struct {
  SMBIOS_STRUCTURE      Hdr;
  UINT16                Type17Handle;
  UINT16                Offset;
  UINT16                Size;
  UINT16                Data[];
} SMBIOS_TABLE_TYPE130;

///
/// Apple Processor Type (Type 131).
///
typedef struct {
  SMBIOS_STRUCTURE      Hdr;
  UINT16                ProcessorType;
} SMBIOS_TABLE_TYPE131;

///
/// Apple Processor Bus Speed (Type 132).
///
typedef struct {
  SMBIOS_STRUCTURE      Hdr;
  UINT16                ProcessorBusSpeed; // MT/s unit
} SMBIOS_TABLE_TYPE132;

///
/// Apple Platform Feature (Type 133).
///
typedef struct {
  SMBIOS_STRUCTURE      Hdr;
  UINT64                PlatformFeature;
} SMBIOS_TABLE_TYPE133;

///
/// Apple SMC Version (Type 134).
///
typedef struct {
  SMBIOS_STRUCTURE      Hdr;
  UINT64                SMCVersion;
} SMBIOS_TABLE_TYPE134;

///
/// Union of all the possible SMBIOS record types.
///
typedef union {
  SMBIOS_STRUCTURE      *Hdr;
  SMBIOS_TABLE_TYPE0    *Type0;
  SMBIOS_TABLE_TYPE1    *Type1;
  SMBIOS_TABLE_TYPE2    *Type2;
  SMBIOS_TABLE_TYPE3    *Type3;
  SMBIOS_TABLE_TYPE4    *Type4;
  SMBIOS_TABLE_TYPE5    *Type5;
  SMBIOS_TABLE_TYPE6    *Type6;
  SMBIOS_TABLE_TYPE7    *Type7;
  SMBIOS_TABLE_TYPE8    *Type8;
  SMBIOS_TABLE_TYPE9    *Type9;
  SMBIOS_TABLE_TYPE10   *Type10;
  SMBIOS_TABLE_TYPE11   *Type11;
  SMBIOS_TABLE_TYPE12   *Type12;
  SMBIOS_TABLE_TYPE13   *Type13;
  SMBIOS_TABLE_TYPE14   *Type14;
  SMBIOS_TABLE_TYPE15   *Type15;
  SMBIOS_TABLE_TYPE16   *Type16;
  SMBIOS_TABLE_TYPE17   *Type17;
  SMBIOS_TABLE_TYPE18   *Type18;
  SMBIOS_TABLE_TYPE19   *Type19;
  SMBIOS_TABLE_TYPE20   *Type20;
  SMBIOS_TABLE_TYPE21   *Type21;
  SMBIOS_TABLE_TYPE22   *Type22;
  SMBIOS_TABLE_TYPE23   *Type23;
  SMBIOS_TABLE_TYPE24   *Type24;
  SMBIOS_TABLE_TYPE25   *Type25;
  SMBIOS_TABLE_TYPE26   *Type26;
  SMBIOS_TABLE_TYPE27   *Type27;
  SMBIOS_TABLE_TYPE28   *Type28;
  SMBIOS_TABLE_TYPE29   *Type29;
  SMBIOS_TABLE_TYPE30   *Type30;
  SMBIOS_TABLE_TYPE31   *Type31;
  SMBIOS_TABLE_TYPE32   *Type32;
  SMBIOS_TABLE_TYPE33   *Type33;
  SMBIOS_TABLE_TYPE34   *Type34;
  SMBIOS_TABLE_TYPE35   *Type35;
  SMBIOS_TABLE_TYPE36   *Type36;
  SMBIOS_TABLE_TYPE37   *Type37;
  SMBIOS_TABLE_TYPE38   *Type38;
  SMBIOS_TABLE_TYPE39   *Type39;
  SMBIOS_TABLE_TYPE40   *Type40;
  SMBIOS_TABLE_TYPE41   *Type41;
  SMBIOS_TABLE_TYPE42   *Type42;
  SMBIOS_TABLE_TYPE126  *Type126;
  SMBIOS_TABLE_TYPE127  *Type127;
  SMBIOS_TABLE_TYPE128  *Type128;
  SMBIOS_TABLE_TYPE130  *Type130;
  SMBIOS_TABLE_TYPE131  *Type131;
  SMBIOS_TABLE_TYPE132  *Type132;
  SMBIOS_TABLE_TYPE133  *Type133;
  SMBIOS_TABLE_TYPE134  *Type134;
  UINT8                 *Raw;
} APPLE_SMBIOS_STRUCTURE_POINTER;

#pragma pack()

#endif
