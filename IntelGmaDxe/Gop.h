/*++

  Copyright (c)  2006 - 2010 Intel Corporation. All rights reserved
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  Gop.h
    
Abstract:

  GOP Controller Driver header file

Revision History

--*/

//
// GOP Controller Driver
//
#ifndef _EFI_GOP_H_
#define _EFI_GOP_H_

//#include "Tiano.h"
//#include "Pci22.h"
//#include "EfiDriverLib.h"
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

# define EFI_SIGNATURE_32(a, b, c, d) SIGNATURE_32(a, b, c, d)
/*
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (PciIo)
#include EFI_PROTOCOL_DEFINITION (GraphicsOutput)
#include EFI_PROTOCOL_DEFINITION (EdidDiscovered)
#include EFI_PROTOCOL_DEFINITION (EdidActive)
 */

//
// Helper Macros
//
// Compiler check to ensure video tables have the correct
// number of entries
//
#define C_ASSERT(e) typedef char    __C_ASSERT__[(e) ? 1 : -1]

//
// aligns Addr to a 4kB page
//
#define ALLIGN_TO_4K(Addr)  (Addr & (~0xFFF))

#define SIZE_TO_PAGE(Size)  ((Size) >> 12)
#define PAGE_TO_SIZE(Page)  ((Page) << 12)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gDriverBinding;
//#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
extern EFI_COMPONENT_NAME2_PROTOCOL gComponentName2;
//#else
extern EFI_COMPONENT_NAME_PROTOCOL  gComponentName;
//#endif

//
// Intel Graphical Mode Data
//
#define VBLANK_TIMEOUT      0x100000  // max # of loops to allow before ejecting from vblank check
#define VBLANK_MASK         0x2       // bit mask to turn on vblank

//
// List of predefined colors
//
#define COLOR_BLACK 0x0
#define COLOR_RED   0x00FF0000
#define COLOR_BLUE  0x0000FF00
#define COLOR_GREEN 0x000000FF
#define COLOR_WHITE 0x00FFFFFF

//
// Intel PCI Configuration Header values
//
#define INTEL_VENDOR_ID			0x8086
#define INTEL_GOP_DEVICE_ID		0xA011
#define INTEL_GMA_DEVICE_ID		0x27A2
#define INTEL_X3100_DEVICE_ID	0x2A02

//
// PCI Cfg Registers
//
#define GTTADR_BAR_INDEX  3 // GTTADR BAR index value
#define GMADR_BAR_INDEX   2 // GMADR BAR index value
#define MMADR_BAR_INDEX   0 // MMADR BAR index value

#define IGD_GGC_OFFSET    0x52    // Graphics Control 
#define   B_GMS           0x70
#define     V_GMS_DIS     0x00
#define     V_GMS_1MB     0x10
#define     V_GMS_8MB     0x30
#define IGD_BSM_OFFSET    0x5C    //Base of Stolen Memory

//
// Gfx Registers
//
#define DSPACNTR                  0x70180 // bit 31 - enable/disable
#define ADPA                      0x61100 // bit 31 - enable/disable
#define PIPEACONF                 0x70008 // bit 31 - enable/disable
#define PIPEASTAT                 0x70024 // bit 1 (1 = blank has occurred)
#define VGACNTRL                  0x71400 // legacy VGA control register
#define DPLLAControl              0x6014  // bit 31 - enable/disable
#define DPLLADivisor              0x6040  // PLL divisor register
#define HTOTAL_A                  0x60000 // Horizontal display clocks
#define HBLANK_A                  0x60004 // Horizontal blanking register
#define HSYNC_A                   0x60008 // Horizontal sync register
#define VTOTAL_A                  0x6000C // Vertical display lines
#define VBLANK_A                  0x60010 // Vertical blanking register
#define VSYNC_A                   0x60014 // Vertical sync register
#define PIPESRC_A                 0x6001C // Source image size
#define BDRCOLRPTRN_A             0x60020 // Border Color Pattern
#define ColorChannel_Red_A        0x60050 // CRC Channel color register (Red)
#define ColorChannel_Grn_A        0x60054 // CRC Channel color register (Green)
#define ColorChannel_Blue_A       0x60058 // CRC Channel color register (Blue)
#define ColorChannelResult_Red_A  0x60060 // CRC Channel color register (Red)
#define ColorChannelResult_Grn_A  0x60064 // CRC Channel color register (Green)
#define ColorChannelResult_Blue_A 0x60068 // CRC Channel color register (Blue)
#define DSPABASE                  0x70184 // Display A display buffer base address offset
#define DSPASTRIDE                0x70188 // Display A stride value (length of scan line in bytes)
#define PGTBL_CTL                 0x2020  // The page table control register (GTT enable)

#define PP_CONTROL                0x61204
#define PP_ON_DELAYS              0x61208
#define PP_OFF_DELAYS             0x6120c
#define PP_DIVISOR                0x61210
#define HTOTAL_B                  0x61000
#define HBLANK_B                  0x61004
#define HSYNC_B                   0x61008
#define VTOTAL_B                  0x6100c
#define VBLANK_B                  0x61010
#define VSYNC_B                   0x61014
#define FPB0                      0x6048 
#define DPLLB_CTRL                0x6018 
#define PIPEBSRC                  0x6101c
#define BCLRPAT_B                 0x61020
#define CRCCtrlColorBR            0x61050
#define CRCCtrlColorBG            0x61054
#define CRCCtrlColorBB            0x61058
#define PIPEBCONF                 0x71008
#define DSPBSIZE                  0x71190
#define DSPBSTRIDE                0x71188
#define DSPBLINOFFSET             0x71184
#define DSPBCNTR                  0x71180
#define LVDSPC                    0x61180
#define PFIT_CONTROL              0x61230
#define BLC_PWM_CTL               0x61254
#define PIPEBSTAT                 0x71024

//
// Frame buffer size allocation
// Change the FRAME_BUFFER_SIZE value to allocate different sizes
// of frame buffer. Frame buffer size determined by following:
// FBsize = (horz pixzel) * (vert pixel) * (color depth)
// where color depth is in bytes per pixel.  For 1280x1024x32 bit color,
// the frame buffer size needed is:
// FBsize = (1280) * (1024) * (4) = 5,242,880 bytes.
//
#define FRAME_BUFFER_SIZE 8 // size of the frame buffer in MB
//
// size of space to allocate for GTT.  
//
#define GTT_ALLOC_SIZE    (256 * 1024)                      // allocate 256K buffer for GTT (max size)

#define FB_SIZE           (FRAME_BUFFER_SIZE * 1024 * 992)  // (FRAME_BUFFER_SIZE * 1024 * (1024 - 32))
#define MEM_ALLOC_SIZE    (FB_SIZE)                         //
#define CACHE_LINE_SIZE   64                                // 64 bytes per cache line

#define PREALLOCATED_SIZE (8 * 1024 * 1024)

//
// ======================================================================
// Display Structure (DS) type defninition that describes the mode
// to be programmed into the chip.
//
typedef struct {
  UINT32  RegisterAddress;      // Offset of register to access
  UINT32  Value;                // value to write to the register
  BOOLEAN DoubleBuffer;         // TRUE = wait for VBLANK to occur; FALSE = don't wait
} MODE_FORMAT;

#define NUM_DS_ENTRIES      16  // # of enteries in a display structure
//
// Modes above 800x600 are not well supported, 
// temporarily set with limited supported modes
//
#define NUM_SUPPORTED_MODES 4   // the number of supported video modes
#define GRAPHICS_OUTPUT_INVALIDE_MODE_NUMBER 0xffff
//
// MODE 0 - turns off display controller.
//
extern MODE_FORMAT  DS_0_0_0_0[];
extern MODE_FORMAT  DS_640_480_32_60[];
extern MODE_FORMAT  DS_800_600_32_60[];
extern MODE_FORMAT  DS_1024_768_32_60[];

//
// Generic shutdown controller;  this is used to turn off the controller
// before changing the mode.  These must be done in this order.  The order
// in which these entries appear in the table is the order in which the
// values are written to the h/w.
//
extern MODE_FORMAT  mDISPLAY_SHUTDOWN[];
extern UINT16       mNUM_SHUTDOWN_ENTRIES;
extern MODE_FORMAT  LVDS_SHUTDOWN[];

//
// Generic start-up controller;  This is used to turn on the controller
// before changing the mode.  These must be done in this order.  The order
// in which these entries appear in the table is the order in which the
// values are written to the h/w.
//
extern MODE_FORMAT  mDISPLAY_STARTUP[];
extern UINT16       mNUM_STARTUP_ENTRIES;

typedef struct {
  UINT32      Width;          // screen width in pixels
  UINT32      Height;         // screen height in pixels
  UINT32      ColorDepth;     // color depth in bits per pixel
  UINT32      RefreshRate;    // screen refresh rate in Hz
  MODE_FORMAT *VideoModeData; // the corresponding structure for the desired mode
  MODE_FORMAT *LVDSModeData; // the corresponding structure for the desired mode
} INTEL_VIDEO_MODES;

extern INTEL_VIDEO_MODES  mVideoModes[];

typedef struct {
  UINT32  ModeNumber;
  UINT32  HorizontalResolution;
  UINT32  VerticalResolution;
  UINT32  RefreshRate;
  EFI_GRAPHICS_PIXEL_FORMAT  PixelFormat;
  EFI_PIXEL_BITMASK PixelInformation;
  UINT32                     PixelsPerScanLine;
  EFI_PHYSICAL_ADDRESS        FrameBufferBase;
  UINTN                       FrameBufferSize;
} INTEL_GRAPHICS_OUTPUT_MODE_DATA;

//
// Intel GOP Private Data Structure
//
#define INTEL_PRIVATE_DATA_SIGNATURE  EFI_SIGNATURE_32 ('i', 'G', 'O', 'P')

typedef struct {
  UINTN   Pages;
  UINTN   BaseAddress;
} ALLOC_STRUCT;

typedef struct {
  UINT64                               Signature;
  EFI_HANDLE                           Handle;
  EFI_PCI_IO_PROTOCOL                  *PciIo;
  EFI_DEVICE_PATH_PROTOCOL             *DevicePath;
  EFI_GRAPHICS_OUTPUT_PROTOCOL         GraphicsOutput;
  EFI_EDID_DISCOVERED_PROTOCOL         EdidDiscovered;
  EFI_EDID_ACTIVE_PROTOCOL             EdidActive;
  ALLOC_STRUCT                         GTTBaseAddress;
  ALLOC_STRUCT                         AllocatedMemory;
  UINT32                               FrameBufferOffset;

  UINT32                               PixelFormat;
  EFI_EVENT                            ExitBootServiceEvent;

  //
  // Graphics Output Private Data
  //
  INTEL_GRAPHICS_OUTPUT_MODE_DATA  ModeData[NUM_SUPPORTED_MODES];
} INTEL_PRIVATE_DATA;

typedef struct {
  UINT8 Register;
  UINT8 Function;
  UINT8 Device;
  UINT8 Bus;
  UINT8 Segment;
  UINT8 Reserved[3];
} PCI_ADDRESS;

#define INTEL_PRIVATE_DATA_FROM_GRAPHICS_OUTPUT_THIS(a) \
  CR ( \
  a, \
  INTEL_PRIVATE_DATA, \
  GraphicsOutput, \
  INTEL_PRIVATE_DATA_SIGNATURE \
  )

//
// Graphics Output Hardware abstraction internal worker functions
//
//
// EFI 1.1 driver model prototypes for Intel Graphics Output
//
EFI_STATUS
EFIAPI
DriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:

  Entrypoint of the GOP driver

Arguments:

  ImageHandle - Driver image handle
  SystemTable - Pointer to system table 

Returns:

  EFI_SUCCESS - GOP driver finished initialize

--*/
;

//
// EFI_DRIVER_BINDING_PROTOCOL Protocol Interface
//
EFI_STATUS
EFIAPI
ControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
  
Routine Description:
  
  The function test whether or not GOP driver support the Controller.
    
Arguments:

  This                - Driver Binding Protocol instance 
  Controller          - Pointer to controller Handle
  RemainingDevicePath - Remaining Device Path
    
Returns:
  EFI_SUCCESS         - Successfully support this controller 
  
--*/
;

EFI_STATUS
EFIAPI
ControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
  
Routine Description:

  GOP driver start entrypoint.
    
Arguments:

  This                - Driver Binding Protocol instance 
  Controller          - Pointer to controller Handle
  RemainingDevicePath - Remaining Device Path
    
Returns:

  EFI_SUCCESS         - Driver success to start
  
--*/
;

EFI_STATUS
EFIAPI
ControllerDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                   *ChildHandleBuffer
  )
/*++

Routine Description:

  GOP driver stop entrypoint

Arguments:

  This              - Driver Binding Protocol instance 
  Controller        - Controller Handle
  NumberOfChildren  - Child number
  ChildHandleBuffer - A buffer that contain childern handles

Returns:

  EFI_SUCCESS       - GOP driver successfully Stop 

--*/
;

EFI_STATUS
GraphicsOutputConstructor (
  INTEL_PRIVATE_DATA  *Private
  )
/*++

Routine Description:

  This function acts like a C++ constructor that is called when the 
  the started routine is called.  This function is allocates buffers, 
  GTT memory and initializes the gfx controller for the default mode.

Arguments:

  Private - Pointer to the private data members

Returns:

  EFI_SUCCESS - Successfully return

--*/
;

EFI_STATUS
GraphicsOutputDestructor (
  INTEL_PRIVATE_DATA  *Private
  )
/*++

Routine Description:

  Undo what was done in the Constructor.

Arguments:

  Private - Pointer to the private data members

Returns:

  EFI_SUCCESS - Successfully return

--*/
;

EFI_STATUS
EFIAPI
GraphicsOutputQueryMode (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL          *This,
  IN  UINT32                                ModeNumber,
  OUT UINTN                                 *SizeOfInfo,
  OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  **Info
  )
/*++

Routine Description:

  Graphics Output protocol interface to get video mode

  Arguments:
    This                  - Protocol instance pointer.
    ModeNumber            - The mode number to return information on.
    Info                  - Caller allocated buffer that returns information about ModeNumber.
    SizeOfInfo            - A pointer to the size, in bytes, of the Info buffer.

  Returns:
    EFI_SUCCESS           - Mode information returned.
    EFI_DEVICE_ERROR      - A hardware error occurred trying to retrieve the video mode.
    EFI_NOT_STARTED       - Video display is not initialized. Call SetMode ()
    EFI_INVALID_PARAMETER - One of the input args was NULL.

--*/
;

EFI_STATUS
EFIAPI
GraphicsOutputSetMode (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL * This,
  IN  UINT32                       ModeNumber
  )
/*++

Routine Description:

  Graphics Output protocol interface to set video mode

  Arguments:
    This             - Protocol instance pointer.
    ModeNumber       - The mode number to be set.

  Returns:
    EFI_SUCCESS      - Graphics mode was changed.
    EFI_DEVICE_ERROR - The device had an error and could not complete the request.
    EFI_UNSUPPORTED  - ModeNumber is not supported by this device.

--*/
;

EFI_STATUS
EFIAPI
GraphicsOutputBitBlt (
  IN EFI_GRAPHICS_OUTPUT_PROTOCOL          *This,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *BltBuffer,
  IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION     BltOperation,
  IN UINTN                   SourceX,
  IN UINTN                   SourceY,
  IN UINTN                   DestinationX,
  IN UINTN                   DestinationY,
  IN UINTN                   Width,
  IN UINTN                   Height,
  IN UINTN                   Delta
  )
/*++

Routine Description:

  This function performs the Bliting operations requested by the 
  caller.

Arguments:

  This          - Graphics Output Protocol instance
  BltBuffer     - Image Data buffer to blt
  BltOperation  - Blt operation methed 
  SourceX       - Source X coordinate
  SourceY       - Source Y coordinate
  DestinationX  - Destination X coordinate
  DestinationY  - Destination Y coordinate
  Width         - Width of Image
  Height        - Height of Image
  Delta         - The number of bytes in each row of image

Returns:

  EFI_INVALID_PARAMETER - Paramater invalid

--*/
;

VOID
EFIAPI
ClearGfxController (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  );

#endif
