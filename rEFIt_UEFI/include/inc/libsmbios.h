#ifndef _LIB_SMBIOS_H
#define _LIB_SMBIOS_H
/*++

Copyright (c) 2000  Intel Corporation

Module Name:

    LibSmbios.h
    
Abstract:

    Lib include  for SMBIOS services. Used to get system serial number and GUID

Revision History

--*/

//
// Define SMBIOS tables.
//
#pragma pack(1)
typedef struct {
    UINT8   AnchorString[4];
    UINT8   EntryPointStructureChecksum;
    UINT8   EntryPointLength;
    UINT8   MajorVersion;
    UINT8   MinorVersion;
    UINT16  MaxStructureSize;
    UINT8   EntryPointRevision;
    UINT8   FormattedArea[5];
    UINT8   IntermediateAnchorString[5];
    UINT8   IntermediateChecksum;
    UINT16  TableLength;
    UINT32  TableAddress;
    UINT16  NumberOfSmbiosStructures;
    UINT8   SmbiosBcdRevision;
} SMBIOS_STRUCTURE_TABLE;

//
// Please note that SMBIOS structures can be odd byte aligned since the
//  unformated section of each record is a set of arbitrary size strings.
//

typedef struct {
    UINT8   Type;
    UINT8   Length;
    UINT8   Handle[2];
} SMBIOS_HEADER;

typedef UINT8   SMBIOS_STRING;

typedef struct {
    SMBIOS_HEADER   Hdr;
    SMBIOS_STRING   Vendor;
    SMBIOS_STRING   BiosVersion;
    UINT8           BiosSegment[2];
    SMBIOS_STRING   BiosReleaseDate;
    UINT8           BiosSize;
    UINT8           BiosCharacteristics[8];
} SMBIOS_TYPE0;

typedef struct {
    SMBIOS_HEADER   Hdr;
    SMBIOS_STRING   Manufacturer;
    SMBIOS_STRING   ProductName;
    SMBIOS_STRING   Version;
    SMBIOS_STRING   SerialNumber;

    //
    // always byte copy this data to prevent alignment faults!
    //
    EFI_GUID        Uuid;
    
    UINT8           WakeUpType;
} SMBIOS_TYPE1;

typedef struct {
    SMBIOS_HEADER   Hdr;
    SMBIOS_STRING   Manufacturer;
    SMBIOS_STRING   ProductName;
    SMBIOS_STRING   Version;
    SMBIOS_STRING   SerialNumber;
} SMBIOS_TYPE2;

typedef struct {
    SMBIOS_HEADER   Hdr;
    SMBIOS_STRING   Manufacturer;
    UINT8           Type;
    SMBIOS_STRING   Version;
    SMBIOS_STRING   SerialNumber;
    SMBIOS_STRING   AssetTag;
    UINT8           BootupState;
    UINT8           PowerSupplyState;
    UINT8           ThermalState;
    UINT8           SecurityStatus;
    UINT8           OemDefined[4];
} SMBIOS_TYPE3;

typedef struct {
    SMBIOS_HEADER   Hdr;
    UINT8           Socket;
    UINT8           ProcessorType;
    UINT8           ProcessorFamily;
    SMBIOS_STRING   ProcessorManufacture;
    UINT8           ProcessorId[8];
    SMBIOS_STRING   ProcessorVersion;
    UINT8           Voltage;
    UINT8           ExternalClock[2];
    UINT8           MaxSpeed[2];
    UINT8           CurrentSpeed[2];
    UINT8           Status;
    UINT8           ProcessorUpgrade;
    UINT8           L1CacheHandle[2];
    UINT8           L2CacheHandle[2];
    UINT8           L3CacheHandle[2];
} SMBIOS_TYPE4;

typedef union {
    SMBIOS_HEADER   *Hdr;
    SMBIOS_TYPE0    *Type0;
    SMBIOS_TYPE1    *Type1;
    SMBIOS_TYPE2    *Type2;
    SMBIOS_TYPE3    *Type3;
    SMBIOS_TYPE4    *Type4;
    UINT8           *Raw;
} SMBIOS_STRUCTURE_POINTER;
#pragma pack()


#endif

