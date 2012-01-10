/*++

Copyright (c) 1999 Intel Corporation

Module Name:

    legacyboot

Abstract:

    EFI support for legacy boot



Revision History

--*/

#ifndef _LEGACY_BOOT_INCLUDE_
#define _LEGACY_BOOT_INCLUDE_

#define LEGACY_BOOT_PROTOCOL \
    { 0x376e5eb2, 0x30e4, 0x11d3, { 0xba, 0xe5, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 } }

#pragma pack(1)

//
// BBS 1.01 (See Appendix A) IPL and BCV Table Entry Data structure.
//  Seg:Off pointers have been converted to EFI pointers in this data structure
//  This is the structure that also maps to the EFI device path for the boot selection
//
typedef struct {
    UINT16  DeviceType;
    UINT16  StatusFlag;
    UINT32  Reserved;
    VOID    *BootHandler;   // Not an EFI entry point
    CHAR8   *DescString;
} BBS_TABLE_ENTRY;
#pragma pack()

typedef
EFI_STATUS
(EFIAPI *LEGACY_BOOT_CALL) (
    IN EFI_DEVICE_PATH      *DevicePath
    );


//
// BBS support functions
//  PnP Call numbers and BiosSelector hidden in implementation
//

typedef enum {
    IplRelative,
    BcvRelative
} BBS_TYPE;

INTERFACE_DECL(_LEGACY_BOOT_INTERFACE);

//
// == PnP Function 0x60 then BbsVersion == 0x0101 if this call fails then BbsVersion == 0x0000
//

//
// == PnP Function 0x61
//
typedef
EFI_STATUS
(EFIAPI *GET_DEVICE_COUNT) (
    IN  struct _LEGACY_BOOT_INTERFACE   *This,
    IN  BBS_TYPE        *TableType,
    OUT UINTN           *DeviceCount,
    OUT UINTN           *MaxCount
    );

//
// == PnP Function 0x62
//
typedef
EFI_STATUS
(EFIAPI *GET_PRIORITY_AND_TABLE) (
    IN  struct _LEGACY_BOOT_INTERFACE   *This,
    IN  BBS_TYPE        *TableType,
    IN OUT  UINTN       *PrioritySize, // MaxCount * sizeof(UINT8)
    OUT     UINTN       *Priority,
    IN OUT  UINTN       *TableSize,    // MaxCount * sizeof(BBS_TABLE_ENTRY)
    OUT BBS_TABLE_ENTRY *TableEntrySize
    );

//
// == PnP Function 0x63
//
typedef
EFI_STATUS
(EFIAPI *SET_PRIORITY) (
    IN  struct _LEGACY_BOOT_INTERFACE   *This,
    IN  BBS_TYPE        *TableType,
    IN OUT  UINTN       *PrioritySize,
    OUT     UINTN       *Priority
    );

typedef struct _LEGACY_BOOT_INTERFACE {
    LEGACY_BOOT_CALL    BootIt;

    //
    // New functions to allow BBS booting to be configured from EFI
    //
    UINTN                   BbsVersion;     // Currently 0x0101
    GET_DEVICE_COUNT        GetDeviceCount;
    GET_PRIORITY_AND_TABLE  GetPriorityAndTable;
    SET_PRIORITY            SetPriority;   
} LEGACY_BOOT_INTERFACE;

EFI_STATUS
PlInitializeLegacyBoot (
    VOID
    );

#endif
