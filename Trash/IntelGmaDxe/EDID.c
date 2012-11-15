/*++

  Copyright (c)  2006 - 2010 Intel Corporation. All rights reserved
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  EDID.c

Abstract:

  This file produces the panel EDID information to graphics controller.
  It is called by GraphicsOutput.c. It reads the EDID table and parse
  the timting information to graphics controller

--*/

#include "Gop.h"
#include "EDID.h"

//
// Standard timing defined by VESA EDID
//
EDID_TIMING mVbeEstablishedEdidTiming[] = {
  //
  // Established Timing I
  //
  {800, 600, 60},
  {800, 600, 56},
  {640, 480, 75},
  {640, 480, 72},
  {640, 480, 67},
  {640, 480, 60},
  {720, 400, 88},
  {720, 400, 70},
  //
  // Established Timing II
  //
  {1280, 1024, 75},
  {1024,  768, 75},
  {1024,  768, 70},
  {1024,  768, 60},
  {1024,  768, 87},
  {832,   624, 75},
  {800,   600, 75},
  {800,   600, 72},
  //
  // Established Timing III
  //
  {1152, 870, 75}
};

//
// Commonly used utilities to read/write MMIO register
//
UINT8
GopMMIOReadByte (
  IN  EFI_PCI_IO_PROTOCOL         *PciIo,
  IN  UINTN                       Offset
  )
{
  UINT8 Data;

  PciIo->Mem.Read (PciIo, EfiPciIoWidthUint8, MMADR_BAR_INDEX, Offset, 1, &Data);

  return Data;
}

VOID
GopMMIOWriteByte (
  IN  EFI_PCI_IO_PROTOCOL         *PciIo,
  IN  UINTN                       Offset,
  IN  UINT8                       Data
  )
{
  PciIo->Mem.Write (PciIo, EfiPciIoWidthUint8, MMADR_BAR_INDEX, Offset, 1, &Data);

  return ;
}

UINT16
GopMMIOReadWord (
  IN  EFI_PCI_IO_PROTOCOL         *PciIo,
  IN  UINTN                       Offset
  )
{
  UINT16  Data;

  PciIo->Mem.Read (PciIo, EfiPciIoWidthUint16, MMADR_BAR_INDEX, Offset, 1, &Data);

  return Data;
}

VOID
GopMMIOWriteWord (
  IN  EFI_PCI_IO_PROTOCOL         *PciIo,
  IN  UINTN                       Offset,
  IN  UINT16                      Data
  )
{
  PciIo->Mem.Write (PciIo, EfiPciIoWidthUint16, MMADR_BAR_INDEX, Offset, 1, &Data);

  return ;
}

UINT32
GopMMIOReadDWord (
  IN  EFI_PCI_IO_PROTOCOL         *PciIo,
  IN  UINTN                       Offset
  )
{
  UINT32  Data;

  PciIo->Mem.Read (PciIo, EfiPciIoWidthUint32, MMADR_BAR_INDEX, Offset, 1, &Data);

  return Data;
}

VOID
GopMMIOWriteDWord (
  IN  EFI_PCI_IO_PROTOCOL         *PciIo,
  IN  UINTN                       Offset,
  IN  UINT32                      Data
  )
{
  PciIo->Mem.Write (PciIo, EfiPciIoWidthUint32, MMADR_BAR_INDEX, Offset, 1, &Data);

  return ;
}


//
// Local Function Prototypes
//
EFI_STATUS
AcquireGMBus (
  IN  EFI_PCI_IO_PROTOCOL         *PciIo
  )
{
  UINTN                 Index;
  EFI_GMBUS_STATUS_REG  BusStatus;

  for (Index = 0; Index < RETRY_TIMES; Index++) {
    BusStatus.uint32 = GopMMIOReadDWord (PciIo, GMBUS2);
    if (BusStatus.Status.InUse == 0) {
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_READY;
}

EFI_STATUS
ReleaseGMBus (
  IN  EFI_PCI_IO_PROTOCOL         *PciIo
  )
{
  UINT16  Value;

  Value = GopMMIOReadWord (PciIo, GMBUS2);
  Value = (UINT16) (Value | GMINUSE);
  GopMMIOWriteWord (PciIo, GMBUS2, Value);

  return EFI_SUCCESS;
}

EFI_STATUS
GMBusReadData (
  IN  EFI_PCI_IO_PROTOCOL         *PciIo,
  IN  UINT8                       *Buffer,
  IN OUT UINT32                   *NumOfBytes
  )
{
  EFI_GMBUS_COMMAND_REG BusCommand;
  EFI_GMBUS_STATUS_REG  BusStatus;
  UINTN                 Index;
  UINT32                *Data;

  BusCommand.Command.SlaveDirect    = 1;
  BusCommand.Command.SlaveAddr      = VCLADDRESS >> 1;
  BusCommand.Command.SlaveIndex     = 0;
  BusCommand.Command.TotalByte      = *NumOfBytes;
  BusCommand.Command.BusCycle       = 0x5;
  BusCommand.Command.RSVD           = 0;
  BusCommand.Command.EnableTimeout  = 1;
  BusCommand.Command.SWRdy          = 1;
  BusCommand.Command.SWClrInt       = 0;
  GopMMIOWriteDWord (PciIo, GMBUS1, BusCommand.uint32);

  for (Index = 0; Index < *NumOfBytes; Index += 4) {
    //
    // HW_RDY bit asserted?
    //
    do {
      BusStatus.uint32 = GopMMIOReadDWord (PciIo, GMBUS2);
    } while (BusStatus.Status.HardwareReady != 1);

    if ((BusStatus.Status.SlaveStallTimeoutErr == 1) || (BusStatus.Status.SlaveAckTimeoutErr == 1)) {
      //
      // bus error, set/reset software clear interrupt
      //
      BusCommand.uint32           = 0;
      BusCommand.Command.SWClrInt = 1;
      GopMMIOWriteDWord (PciIo, GMBUS1, BusCommand.uint32);
      BusCommand.Command.SWClrInt = 0;
      GopMMIOWriteDWord (PciIo, GMBUS1, BusCommand.uint32);

      //
      // Now poll GMBUS2 again for HW_RDY and GA
      //
      do {
        BusStatus.uint32 = GopMMIOReadDWord (PciIo, GMBUS2);
      } while ((BusStatus.Status.HardwareReady != 1) && (BusStatus.Status.GActive != 0));

      return EFI_UNSUPPORTED;
    }
    //
    // Data is now in GMBUS data register now
    //
    Data  = (UINT32 *) Buffer;
    *Data = GopMMIOReadDWord (PciIo, GMBUS3);
    Buffer += 4;
  }
  //
  // read completed? bus idle?
  //
  do {
    BusStatus.uint32 = GopMMIOReadDWord (PciIo, GMBUS2);
  } while (BusStatus.Status.GActive == 1);

  return EFI_SUCCESS;
}

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
{
  EFI_STATUS                Status;
  UINT8                     EdidData[512];
  UINT8                     *ValidEdid;
  UINT32                    TotalBytes;
  EFI_GMBUS_CLOCK_PORT_REG  BusClockPort;
  UINTN                     Index;
  UINT64                    Signature;

  //
  // First acquire the bus
  //
  Status = AcquireGMBus (PciIo);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Select I2C port/clock: Analog Monitor
  //
  BusClockPort.uint32 = 0;
  BusClockPort.Clock_Port.BusRateSelect = 0;
  BusClockPort.Clock_Port.PinPairSelect = 2;
  GopMMIOWriteDWord (PciIo, GMBUS0, BusClockPort.uint32);

  //
  // Read 256 byte data
  //
  TotalBytes  = 2 * sizeof (EDID_BLOCK);
  Status      = GMBusReadData (PciIo, &EdidData[0], &TotalBytes);

  //
  // Release the bus
  //
  ReleaseGMBus (PciIo);

  if (!EFI_ERROR (Status)) {
    //
    // Make a copy of the 256 byte EDID data to ensure integrity of at least one EDID block
    //
    CopyMem (&EdidData[256], &EdidData[0], 256);

    //
    // Search for the EDID signature
    //
    ValidEdid = &EdidData[0];
    Signature = EDID_HEADER_SIGNATURE;
    for (Index = 0; Index < 256; Index++, ValidEdid++) {
      if (CompareMem (ValidEdid, &Signature, 8) == 0) {
        break;
      }
    }
    if (Index == 256) {
      //
      // No EDID signature found
      //
      return EFI_UNSUPPORTED;
    }

    *EDIDBlock = AllocateCopyPool (
                  sizeof (EDID_BLOCK),
                  ValidEdid
                  );
    if (!(*EDIDBlock)) {
      return EFI_OUT_OF_RESOURCES;
    }

    //
    // Currently only support EDID 1.x
    //
    *EDIDSize = 128;
  }

  return Status;
}

BOOLEAN
CheckTableIntegrity (
  UINT8                           *Data,
  UINTN                           NumOfBytes
  )
/*++

  Routine Description:
    Returns information about the geometry and configuration of
    the graphics controller's current frame buffer configuration

  Arguments:
    Data                  - Data buffer
    NumOfBytes            - Size in bytes of the data buffer

  Returns:
    TRUE                  - Valid EDID table
    FALSE                 - Data corrupted

--*/
{
  UINT8 Checksum;
  UINTN Index;

  Checksum = 0;
  for (Index = 0; Index < NumOfBytes; Index++) {
    Checksum = (UINT8) (Checksum + Data[Index]);
  }

  if (Checksum) {
    return FALSE;
  } else {
    return TRUE;
  }
}

UINT32
CalculateEdidKey (
  EDID_TIMING       *EdidTiming
  )
/*++

  Routine Description:

  Generate a search key for a specified timing data.

  Arguments:

  EdidTiming       - Pointer to EDID timing

  Returns:
  The 32 bit unique key for search.

--*/
{
  UINT32 Key;

  //
  // Be sure no conflicts for all standard timing defined by VESA.
  //
  Key = (EdidTiming->HorizontalResolution * 2) + EdidTiming->VerticalResolution + EdidTiming->RefreshRate;
  return Key;
}

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
{
  UINT32       Index;
  UINT32       ValidNumber;
  UINT32       TimingBits;
  UINT8        *BufferIndex;
  UINT16       HorizontalResolution;
  UINT16       VerticalResolution;
  UINT8        AspectRatio;
  UINT8        RefreshRate;
  EDID_TIMING  TempTiming;

  //
  // First check the integrity of the first block
  //
  if (!CheckTableIntegrity ((UINT8 *) EDIDTable, sizeof (EDID_BLOCK))) {
    return EFI_VOLUME_CORRUPTED;
  }

  if (EDIDTable->EDIDHeader != EDID_HEADER_SIGNATURE) {
    return EFI_UNSUPPORTED;
  }
  //
  // We only support EDID 1.3 and above
  //
  if ((EDIDTable->EDIDVersion < SUPPORTED_EDID_VERSION) || (EDIDTable->EDIDRevision < SUPPORTED_EDID_REVISION)) {
    return EFI_UNSUPPORTED;
  }
  //
  // In EDID1.3, preferred timing mode is required. The preferred
  // timing mode is indicated in the first detailed timing block
  //
  if (!(EDIDTable->FeatureSupport & PREFERRED_TIMING_MODE)) {
    return EFI_UNSUPPORTED;
  }

  ValidNumber = 0;
  ZeroMem (ValidEdidTiming, sizeof (VALID_EDID_TIMING));

  if ((EDIDTable->ET1 != 0) || (EDIDTable->ET2 != 0) || (EDIDTable->ET3 != 0)) {
    //
    // Established timing data
    //
    TimingBits = EDIDTable->ET1 | (EDIDTable->ET2 << 8) | ((EDIDTable->ET3 & 0x80) << 9) ;
    for (Index = 0; Index < VBE_EDID_ESTABLISHED_TIMING_MAX_NUMBER; Index ++) {
      if (TimingBits & 0x1) {
        ValidEdidTiming->Key[ValidNumber] = CalculateEdidKey (&mVbeEstablishedEdidTiming[Index]);
        ValidNumber ++;
      }
      TimingBits = TimingBits >> 1;
    }
  } else {
    //
    // If no Established timing data, read the standard timing data
    //
    BufferIndex = (UINT8 *) &EDIDTable->STD1;
    for (Index = 0; Index < 8; Index ++) {
      if ((BufferIndex[0] != 0x1) && (BufferIndex[1] != 0x1)){
        //
        // A valid Standard Timing
        //
        HorizontalResolution = BufferIndex[0] * 8 + 248;
        AspectRatio = BufferIndex[1] >> 6;
        switch (AspectRatio) {
          case 0:
            VerticalResolution = HorizontalResolution / 16 * 10;
            break;
          case 1:
            VerticalResolution = HorizontalResolution / 4 * 3;
            break;
          case 2:
            VerticalResolution = HorizontalResolution / 5 * 4;
            break;
          case 3:
            VerticalResolution = HorizontalResolution / 16 * 9;
            break;
          default:
            VerticalResolution = HorizontalResolution / 4 * 3;
            break;
        }
        RefreshRate = (BufferIndex[1] & 0x1f) + 60;
        TempTiming.HorizontalResolution = HorizontalResolution;
        TempTiming.VerticalResolution = VerticalResolution;
        TempTiming.RefreshRate = RefreshRate;
        ValidEdidTiming->Key[ValidNumber] = CalculateEdidKey (&TempTiming);
        ValidNumber ++;
      }
      BufferIndex += 2;
    }
  }

  ValidEdidTiming->ValidNumber = ValidNumber;
  return TRUE;
}

BOOLEAN
SearchEdidTiming (
  VALID_EDID_TIMING  *ValidEdidTiming,
  EDID_TIMING        *EdidTiming
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
{
  UINT32 Index;
  UINT32 Key;

  Key = CalculateEdidKey (EdidTiming);

  for (Index = 0; Index < ValidEdidTiming->ValidNumber; Index ++) {
    if (Key == ValidEdidTiming->Key[Index]) {
      return TRUE;
    }
  }

  return FALSE;
}
