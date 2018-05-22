/*++

  Copyright (c)  2006 - 2010 Intel Corporation. All rights reserved
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  EDID.h

Abstract:

  EDID data structure definitions

Revision History

--*/

//
// Intel Controller Driver
//
#ifndef _EDID_H_
#define _EDID_H_

//#include "Tiano.h"
#include <Uefi.h>

//
// Driver Consumed Protocol Prototypes
//
#include <Protocol/DevicePath.h>
#include <Protocol/PciIo.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/ComponentName.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/UgaDraw.h>
//#include <Protocol/VgaMiniPort.h>
//#include <Protocol/Legacy8259.h>
#include <Protocol/EdidActive.h>
#include <Protocol/EdidDiscovered.h>
#include <Protocol/DevicePath.h>

#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>

#include <IndustryStandard/Pci.h>


#define EDID_HEADER_SIGNATURE   0x00FFFFFFFFFFFF00ull
#define SUPPORTED_EDID_VERSION  1
#define SUPPORTED_EDID_REVISION 3

#define PREFERRED_TIMING_MODE   0x02

//
// DETAILED_TIMING_DESC data structure
//
typedef struct {
  UINT8 LSB;
  UINT8 MSB;
} PIXEL_CLOCK;

typedef struct {
  UINT8 Tbp : 4;
  UINT8 Ta : 4;
} TA_TBP;

typedef struct {
  UINT8 Vspw : 2;
  UINT8 Tvfp : 2;
  UINT8 Hspw : 2;
  UINT8 Thfp : 2;
} THFP_HSPW_TVFP_VSPW;

typedef struct {
  PIXEL_CLOCK         PixelClock;
  UINT8               Tha;
  UINT8               Thbp;
  TA_TBP              Tha_Thbp;
  UINT8               Tva;
  UINT8               Tvbp;
  TA_TBP              Tva_Tvbp;
  UINT8               Thfp;
  UINT8               Thspw;
  TA_TBP              Tvfp_Tvspw;
  THFP_HSPW_TVFP_VSPW Thfp_Thspw_Tvfp_Tvspw;
  UINT8               HorizontalSize;
  UINT8               VerticalSize;
  TA_TBP              Hori_VerSize;
  UINT8               HorizontalBorder;
  UINT8               VerticalBorder;
  UINT8               Flags;
} DETAILED_TIMING_DESC;

//
// Monitor Descriptor Block
//
typedef union {
  UINT8 DescData[13];
  struct {
    UINT8 ThspwMin;
    UINT8 ThspwMax;
    UINT8 ThbpMin;
    UINT8 ThbpMax;
    UINT8 TvspwMin;
    UINT8 TvspwMax;
    UINT8 TvbpMin;
    UINT8 TvbpMax;
    UINT8 ThpMin;
    UINT8 ThpMax;
    UINT8 TvpMin;
    UINT8 TvpMax;
    UINT8 Revision;
  } AlternateTiming;
} DESC_DATA_BLOCK;

typedef struct {
  UINT16          Flag1;
  UINT8           Flag2;
  UINT8           DataTypeTag;
  UINT8           Flag3;
  DESC_DATA_BLOCK DescData;
} MONITOR_DESC_BLOCK;

//
// EDID block
//
typedef struct {
  UINT64                EDIDHeader;

  UINT16                VendorName;
  UINT16                ProductCode;
  UINT32                SerialNumber;
  UINT8                 ManufactureWeek;
  UINT8                 ManufactureYear;

  UINT8                 EDIDVersion;
  UINT8                 EDIDRevision;

  UINT8                 VideoInputDefinition;
  UINT8                 MaxHorizontalSize;
  UINT8                 MaxVerticalSize;
  UINT8                 DisplayTransferChar;
  UINT8                 FeatureSupport;

  UINT8                 Red_GreenLowBits;
  UINT8                 Blue_WhiteLowBits;
  UINT8                 Red_x;
  UINT8                 Red_y;
  UINT8                 Green_x;
  UINT8                 Green_y;
  UINT8                 Blue_x;
  UINT8                 Blue_y;
  UINT8                 White_x;
  UINT8                 White_y;

  UINT8                 ET1;
  UINT8                 ET2;
  UINT8                 ET3;

  UINT16                STD1;
  UINT16                STD2;
  UINT16                STD3;
  UINT16                STD4;
  UINT16                STD5;
  UINT16                STD6;
  UINT16                STD7;
  UINT16                STD8;

  DETAILED_TIMING_DESC  DTD1;
  MONITOR_DESC_BLOCK    DTD2;
  MONITOR_DESC_BLOCK    DTD3;
  MONITOR_DESC_BLOCK    DTD4;

  UINT8                 ExtensionFlag;

  UINT8                 Checksum;
} EDID_BLOCK;

#define VBE_EDID_ESTABLISHED_TIMING_MAX_NUMBER 17

typedef struct {
  UINT16  HorizontalResolution;
  UINT16  VerticalResolution;
  UINT16  RefreshRate;
} EDID_TIMING;

typedef struct {
  UINT32  ValidNumber;
  UINT32  Key[VBE_EDID_ESTABLISHED_TIMING_MAX_NUMBER];
} VALID_EDID_TIMING;


#define GMBUS0                  0x5100
#define GMBUS1                  0x5104
#define GMBUS2                  0x5108
#define GMBUS3                  0x510C
#define GMINUSE                 0x8000
#define VCLADDRESS              0xA0                  
#define RETRY_TIMES             0x10000


typedef union _EFI_GMBUS_CLOCK_PORT_REG {
  UINT32  uint32;
  struct {
    UINT32  PinPairSelect : 3;
    UINT32  RSVD1 : 4;
    UINT32  HoldTimeExtension : 1;
    UINT32  BusRateSelect : 3;
    UINT32  RSVD2 : 21;
  } Clock_Port;
} EFI_GMBUS_CLOCK_PORT_REG;

typedef union _EFI_GMBUS_COMMAND_REG {
  UINT32  uint32;
  struct {
    UINT32  SlaveDirect : 1;
    UINT32  SlaveAddr : 7;
    UINT32  SlaveIndex : 8;
    UINT32  TotalByte : 9;
    UINT32  BusCycle : 3;
    UINT32  RSVD : 1;
    UINT32  EnableTimeout : 1;
    UINT32  SWRdy : 1;
    UINT32  SWClrInt : 1;
  } Command;
} EFI_GMBUS_COMMAND_REG;

typedef union _EFI_GMBUS_STATUS_REG {
  UINT32  uint32;
  struct {
    UINT32  ByteCount : 9;
    UINT32  GActive : 1;
    UINT32  SlaveAckTimeoutErr : 1;
    UINT32  HardwareReady : 1;
    UINT32  Interrupt : 1;
    UINT32  SlaveStallTimeoutErr : 1;
    UINT32  HardwareWaitPhase : 1;
    UINT32  InUse : 1;
    UINT32  RSVD : 16;
  } Status;
} EFI_GMBUS_STATUS_REG;



EFI_STATUS
ReadEDID (
  IN  EFI_PCI_IO_PROTOCOL         *PciIo,
  OUT VOID                        **EDIDBlock,
  OUT UINT32                      *EDIDSize
  )
/*++

  Routine Description:
    Returns information about the geometry and configuration of
    the graphics controller's current frame buffer configuration

  Arguments:
    PciIo                 - PCI IO protocol
    EDIDBlock             - Callee allocated buffer to return EDID table
    EDIDSize              - Size of the returning buffer

  Returns:
    EFI_SUCCESS           - Valid EDID table was returned
    EFI_OUT_OF_RESOURCES  - Not enough resources
    EFI_UNSUPPORTED       - Panel did not support DDC2/EDID
    EFI_NOT_READY         - GMBus not ready

--*/
;

EFI_STATUS
ParseEDIDTable (
  IN  EDID_BLOCK               *EDIDTable,
  IN  UINTN                    TableSize,
  OUT VALID_EDID_TIMING        *ValidEdidTiming
  )
/*++

  Routine Description:
    Returns information about the geometry and configuration of
    the graphics controller's current frame buffer configuration

  Arguments:
    EDIDTable             - EDID block to be parsed
    TableSize             - Size of the EDID block
    TimingInfo            - The timing information read out from EDID

  Returns:
    EFI_SUCCESS           - Valid timing infor returned
    EFI_VOLUME_CORRUPTED  - EDID table corrupted
    EFI_UNSUPPORTED       - No valid information found

--*/
;

BOOLEAN
SearchEdidTiming (
  VALID_EDID_TIMING *ValidEdidTiming,
  EDID_TIMING       *EdidTiming
  )
/*++

  Routine Description:

  Search a specified Timing in all the valid EDID timings.

  Arguments:

  ValidEdidTiming  - All valid EDID timing information.
  EdidTiming       - The Timing to search for.

  Returns:

  TRUE  - Found.
  FALSE - Not found.

--*/
;

#endif
