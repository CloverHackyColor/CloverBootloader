/*++

  Copyright (c)  2006 - 2010 Intel Corporation. All rights reserved
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  GraphicsOutput.c

Abstract:

  This file produces the graphics abstration of Graphics Output. It is called by 
  Gop.c file which deals with the EFI 1.1 driver model. 
  This file just does graphics.

--*/

#include "Gop.h"
#include "EDID.h"

//
// Local Function Prototypes
//
EFI_STATUS
InitializeGraphicsMode (
  INTEL_PRIVATE_DATA    *Private,
  INTEL_VIDEO_MODES     *ModeData
  );

EFI_STATUS
WaitForVBlank (
  INTEL_PRIVATE_DATA         *Private,
  UINT32                     Timeout
  );

EFI_STATUS
StartupGfxController (
  INTEL_PRIVATE_DATA    *Private
  );

EFI_STATUS
ShutdownGfxController (
  INTEL_PRIVATE_DATA    *Private
  );

EFI_STATUS
ConfigureGfxController (
  INTEL_PRIVATE_DATA    *Private,
  INTEL_VIDEO_MODES     *ModeData
  );

EFI_STATUS
ClearScreen (
  INTEL_PRIVATE_DATA         *Private,
  UINT32                     Color
  );

//
// Graphics Output Protocol Member Functions
//
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
{
  EFI_STATUS                           Status;
  INTEL_PRIVATE_DATA                   *Private;
  INTEL_GRAPHICS_OUTPUT_MODE_DATA      *ModeData;

  //
  // Parameter checking
  //
  if (This == NULL || Info == NULL || SizeOfInfo == NULL || ModeNumber >= This->Mode->MaxMode) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // return parameters stored in local tracking structure
  // If someone changes the mode outside the GOP driver,
  // then these values will return inaccurate values.  These
  // values should really be read directly from the registers.
  // [bugbug]
  //
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION),
                  (VOID**)&Info
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *SizeOfInfo = sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);

  Private = INTEL_PRIVATE_DATA_FROM_GRAPHICS_OUTPUT_THIS (This);
  ModeData = &Private->ModeData[ModeNumber];

  (*Info)->Version              = 0;
  (*Info)->HorizontalResolution = ModeData->HorizontalResolution;
  (*Info)->VerticalResolution   = ModeData->VerticalResolution;
  (*Info)->PixelFormat          = ModeData->PixelFormat;
  (*Info)->PixelInformation     = ModeData->PixelInformation;
  (*Info)->PixelsPerScanLine    = ModeData->PixelsPerScanLine;

  return EFI_SUCCESS;
}

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
{
  EFI_STATUS                           Status;
  INTEL_PRIVATE_DATA                   *Private;
  INTEL_GRAPHICS_OUTPUT_MODE_DATA      *ModeData;

  //
  // Check the validity of the This pointer
  //
  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (ModeNumber >= This->Mode->MaxMode) {
    return EFI_UNSUPPORTED;
  }

  if (ModeNumber == This->Mode->Mode) {
    return EFI_SUCCESS;
  }

  //
  // Get pointer to private data
  //
  Private = INTEL_PRIVATE_DATA_FROM_GRAPHICS_OUTPUT_THIS (This);

  ModeData = &Private->ModeData[ModeNumber];
  This->Mode->Mode                       = ModeNumber;
  This->Mode->Info->Version              = 0;
  This->Mode->Info->HorizontalResolution = ModeData->HorizontalResolution;
  This->Mode->Info->VerticalResolution   = ModeData->VerticalResolution;
  This->Mode->Info->PixelFormat          = ModeData->PixelFormat;
  This->Mode->Info->PixelInformation     = ModeData->PixelInformation;
  This->Mode->Info->PixelsPerScanLine    = ModeData->PixelsPerScanLine;
  This->Mode->SizeOfInfo                 = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);

  This->Mode->FrameBufferBase = ModeData->FrameBufferBase;
  This->Mode->FrameBufferSize = ModeData->FrameBufferSize;

  //
  // The request is for a supported mode, so program
  // up the graphics controller.
  //
  Status = InitializeGraphicsMode (Private, &mVideoModes[ModeData->ModeNumber]);
  ClearScreen (Private, COLOR_BLACK);

  return Status;
}

EFI_STATUS
EFIAPI
GraphicsOutputBitBlt (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL       *This,
  IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL      *BltBuffer, OPTIONAL
  IN  EFI_GRAPHICS_OUTPUT_BLT_OPERATION  BltOperation,
  IN  UINTN                              SourceX,
  IN  UINTN                              SourceY,
  IN  UINTN                              DestinationX,
  IN  UINTN                              DestinationY,
  IN  UINTN                              Width,
  IN  UINTN                              Height,
  IN  UINTN                              Delta
  )
/*++

Routine Description:

  Graphics Output protocol instance to block transfer

Arguments:

  This          - Pointer to Graphics Output protocol instance
  BltBuffer     - The data to transfer to screen
  BltOperation  - The operation to perform
  SourceX       - The X coordinate of the source for BltOperation
  SourceY       - The Y coordinate of the source for BltOperation
  DestinationX  - The X coordinate of the destination for BltOperation
  DestinationY  - The Y coordinate of the destination for BltOperation
  Width         - The width of a rectangle in the blt rectangle in pixels
  Height        - The height of a rectangle in the blt rectangle in pixels
  Delta         - Not used for EfiBltVideoFill and EfiBltVideoToVideo operation.
                  If a Delta of 0 is used, the entire BltBuffer will be operated on.
                  If a subrectangle of the BltBuffer is used, then Delta represents
                  the number of bytes in a row of the BltBuffer.

Returns:

  EFI_INVALID_PARAMETER - Invalid parameter passed in
  EFI_SUCCESS - Blt operation success

--*/
{
  INTEL_PRIVATE_DATA                   *Private;
  EFI_PCI_IO_PROTOCOL                  *PciIo;
  EFI_TPL                              OriginalTPL;
  UINT32                               ScreenWidth;
  UINT32                               ScreenHeight;
  UINT32                               Row;
  UINTN                                SrcBuf;
  UINTN                                DestBuf;
  EFI_STATUS                           Status;
  INTEL_GRAPHICS_OUTPUT_MODE_DATA      *ModeData;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL        *TmpBuf;
  UINTN                                Index;
  Status  = EFI_SUCCESS;
  Private = INTEL_PRIVATE_DATA_FROM_GRAPHICS_OUTPUT_THIS (This);
  PciIo   = Private->PciIo;

  //
  // Parameter checking
  //
  if (This == NULL || ((UINTN) BltOperation) >= EfiGraphicsOutputBltOperationMax) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width == 0 || Height == 0) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // If Delta is zero, then the entire BltBuffer is being used, so Delta
  // is the number of bytes in each row of BltBuffer.  Since BltBuffer is Width pixels size,
  // the number of bytes in each row can be computed.
  //
  if (Delta == 0) {
    Delta = Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  }

  ModeData = &Private->ModeData[This->Mode->Mode];
  //
  // Some general calculations we will need for our operations
  // Values are in pixels
  //
  ScreenWidth   = ModeData->HorizontalResolution;
  ScreenHeight  = ModeData->VerticalResolution;

  //
  // Convert Screenwidth from Pixels to buffer offset (4bytes/pixel)
  //
  ScreenWidth *= sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL);

  //
  // We need to fill the Virtual Screen buffer with the blt data.
  // The virtual screen is upside down, as the first row is the bottom row of
  // the image.
  //
  //
  // Make sure the SourceX, SourceY, DestinationX, DestinationY, Width, and Height parameters
  // are valid for the operation and the current screen geometry.
  //
  //
  if (BltOperation == EfiBltVideoToBltBuffer) {
    //
    // Video to BltBuffer: Source is Video, destination is BltBuffer
    //
    if (SourceY + Height > ModeData->VerticalResolution) {
      return EFI_INVALID_PARAMETER;
    }

    if (SourceX + Width > ModeData->HorizontalResolution) {
      return EFI_INVALID_PARAMETER;
    }
  } else {
    //
    // BltBuffer to Video: Source is BltBuffer, destination is Video
    //
    if (DestinationY + Height > ModeData->VerticalResolution) {
      return EFI_INVALID_PARAMETER;
    }

    if (DestinationX + Width > ModeData->HorizontalResolution) {
      return EFI_INVALID_PARAMETER;
    }
  }
  // We have to raise to TPL Notify, so we make an atomic write the frame buffer.
  // We would not want a timer based event (Cursor, ...) to come in while we are
  // doing this operation.
  //
  OriginalTPL = gBS->RaiseTPL (TPL_NOTIFY);

  switch (BltOperation) {
  case EfiBltVideoToBltBuffer:
    //
    // Set the memory locations of the source and destination buffer
    //
    DestBuf = DestinationY * Delta + (DestinationX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)) + (UINTN) BltBuffer;

    //
    // Copy the video screen a scann line at a time to the blt buffer.
    //
    for (Row = 0; Row < Height; Row++) {
      PciIo->Mem.Read (
                  PciIo,
                  EfiPciIoWidthUint32,
                  GMADR_BAR_INDEX,
                  (Row + SourceY) * ScreenWidth + (SourceX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)),
                  Width,
                  (VOID *) (UINTN) (DestBuf + Row * Delta)
                  );
    }
    break;

  case EfiBltVideoToVideo:
    //
    // use the 2D BLT engine to copy data within the IGD Gfx frame buffer
    //
    TmpBuf = AllocatePool(Width * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
	for (Index = 0; Index < Width; Index ++) {

		TmpBuf[Index] = *BltBuffer;
	}

    for (Row = 0; Row < Height; Row ++) {

		PciIo->Mem.Read (
					PciIo,
					EfiPciIoWidthUint32,
					GMADR_BAR_INDEX,
					(((SourceY > DestinationY) ? Row :(Height - 1 - Row)) + SourceY) * ScreenWidth + (SourceX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)),
					Width,
					(VOID *)TmpBuf
					);
		PciIo->Mem.Write (
					PciIo,
					EfiPciIoWidthUint32,
					GMADR_BAR_INDEX,
					(((SourceY > DestinationY) ? Row :(Height - 1 - Row)) + DestinationY) * ScreenWidth + (DestinationX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)),
					Width,
					(VOID *) TmpBuf
					);
    }
	FreePool(TmpBuf);
    break;

  case EfiBltVideoFill:
    //
    // Get the color to use for the fill; it is the color of the first
    // pixel in the bltbit buffer
    // Assuming the bltbuffer is the same width as the screen
    //
    TmpBuf = AllocatePool(Width * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
	for (Index = 0; Index < Width; Index ++) {

		TmpBuf[Index] = *BltBuffer;
	}
    for (Row = 0; Row < Height; Row ++) {

		PciIo->Mem.Write (
					PciIo,
					EfiPciIoWidthUint32,
					GMADR_BAR_INDEX,
					(Row + DestinationY) * ScreenWidth + (DestinationX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)),
					Width,
					(VOID *) TmpBuf
					);
    }
	FreePool(TmpBuf);

    break;

  case EfiBltBufferToVideo:
    //
    // Set the memory locations of the source and destination buffer.
    // These will be the physical addresses to where we will write
    // and read data from.
    //
    SrcBuf = SourceY * Delta + (SourceX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)) + (UINTN) BltBuffer;

    //
    // Copy the Blt buffer a row at a time.
    //
    for (Row = 0; Row < Height; Row++) {
      PciIo->Mem.Write (
                  PciIo,
                  EfiPciIoWidthUint32,
                  GMADR_BAR_INDEX,
                  (Row + DestinationY) * ScreenWidth + (DestinationX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)),
                  Width,
                  (VOID *) (UINTN) (SrcBuf + Row * Delta)
                  );
    }

    break;

  default:
    Status = EFI_INVALID_PARAMETER;
    break;
  }
    
  gBS->RestoreTPL (OriginalTPL);

  return Status;
}
//
// Construction and Destruction functions
//
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
{
  UINT32                                Index;
  EFI_PCI_IO_PROTOCOL                   *PciIo;
  EFI_STATUS                            Status;
  UINT32                                Value;
  PCI_TYPE00                            Pci;
  UINT32                                EdidSize;
  VALID_EDID_TIMING                     ValidEdidTiming;
  EDID_TIMING                           TempTiming;
  BOOLEAN                               TimingMatch;
  UINT32                                ValidModeCount;
  UINT32                                PreferMode;
  INTEL_VIDEO_MODES                     *VideoMode;
  INTEL_GRAPHICS_OUTPUT_MODE_DATA       *ModeData;


  PciIo = Private->PciIo;
  
  ZeroMem (&ValidEdidTiming, sizeof (VALID_EDID_TIMING));

  //
  // Enable the PCI Device
  //
  Status = Private->PciIo->Attributes (
                            Private->PciIo,
                            EfiPciIoAttributeOperationEnable,
                            EFI_PCI_DEVICE_ENABLE | EFI_PCI_IO_ATTRIBUTE_VGA_MEMORY | EFI_PCI_IO_ATTRIBUTE_VGA_IO,
                            NULL
                            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Read the PCI Configuration Header from the PCI Device
  //
  Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, sizeof (Pci) / sizeof (UINT32), &Pci);
  if (EFI_ERROR (Status)) {
    return Status;
  }

//  //
//  // at this point, we have a physically contiguous frame buffer pointer &
//  // a physically contiguous GTT buffer.
//  // Initialize the GTT pointer in the GFX device and enable it.
//  //
//  Value = (UINT32) (Private->GTTBaseAddress.BaseAddress | 1);
//  //
//  // OR with 1 to enable GTT
//  //
//  Status = PciIo->Mem.Write (
//                        PciIo,
//                        EfiPciIoWidthUint32,
//                        MMADR_BAR_INDEX,
//                        PGTBL_CTL,
//                        1,
//                        &Value
//                        );

  //
  // Now initialize the memory pages of the gfx GTT table with our allocated
  // memory.  Initialize all GTT entries to good RAM locations.  This is done
  // by initializing every GTT page to the first frame buffer page.  All GTT
  // entries must point to valid RAM locations.
  //
  Value   = (UINT32)((Private->AllocatedMemory.BaseAddress & 0xFFFFF000) | 1);
  for (Index = 0; Index < (GTT_ALLOC_SIZE >> 2); Index++) {
    PciIo->Mem.Write (
                PciIo,
                EfiPciIoWidthUint32,
                GTTADR_BAR_INDEX,
                Index * 4,
                1,
                &Value
                );

  }
  //
  // Initialize GTT entries with frame buffer memory.  This will get the
  // GTT pointing to the true frame buffer and enable access through the
  // GMADR BAR.
  //
  for (Index = 0; Index < Private->AllocatedMemory.Pages; Index++) {

    Value = (UINT32) (((Private->AllocatedMemory.BaseAddress & 0xFFFFF000) + (Index << 12)) | 1);
    PciIo->Mem.Write (
                PciIo,
                EfiPciIoWidthUint32,
                GTTADR_BAR_INDEX,
                Index * 4,
                1,
                &Value
                );
  }

  //
  // setup EDID information
  //
  Private->EdidDiscovered.Edid = NULL;
  Private->EdidDiscovered.SizeOfEdid = 0;
  Private->EdidActive.Edid = NULL;
  Private->EdidActive.SizeOfEdid = 0;

  Status = ReadEDID (PciIo, (VOID**)&Private->EdidDiscovered.Edid, &EdidSize);
  if (!EFI_ERROR (Status)) {
    Status = ParseEDIDTable ((EDID_BLOCK *) Private->EdidDiscovered.Edid, EdidSize, &ValidEdidTiming);
    if (!EFI_ERROR (Status)) {
      Private->EdidDiscovered.SizeOfEdid = EdidSize;
      Private->EdidActive.SizeOfEdid = EdidSize;
      Private->EdidActive.Edid = AllocateCopyPool (EdidSize, Private->EdidDiscovered.Edid);
      if (Private->EdidActive.Edid == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
    }
  }

  //
  // Initialize the private mode data with the supported modes.
  //
  ValidModeCount = 0;
  ModeData = &Private->ModeData[0];
  VideoMode = &mVideoModes[0];
  EdidSize = Private->EdidDiscovered.SizeOfEdid;
  for (Index = 0; Index < NUM_SUPPORTED_MODES; Index++) {

    TimingMatch = TRUE;

    if (EdidSize != 0 && (ValidEdidTiming.ValidNumber > 0)) {
      //
      // EDID found, check whether match with video mode
      //
      TempTiming.HorizontalResolution = (UINT16) VideoMode->Width;
      TempTiming.VerticalResolution   = (UINT16) VideoMode->Height;
      TempTiming.RefreshRate          = (UINT16) VideoMode->RefreshRate;
      if (SearchEdidTiming (&ValidEdidTiming, &TempTiming) != TRUE) {
        TimingMatch = FALSE;
      }
    }

    //
    // Not export Mode 0x0 as GOP mode, this is not defined in spec.
    //
    if ((VideoMode->Width == 0) || (VideoMode->Height == 0)) {
      TimingMatch = FALSE;
    }

    if (TimingMatch) {
      ModeData->ModeNumber = Index;
      ModeData->HorizontalResolution          = VideoMode->Width;
      ModeData->VerticalResolution            = VideoMode->Height;
      ModeData->RefreshRate                   = VideoMode->RefreshRate;
      ModeData->PixelFormat                   = Private->PixelFormat;
      ModeData->PixelInformation.RedMask      = 0x00ff0000;
      ModeData->PixelInformation.GreenMask    = 0x0000ff00;
      ModeData->PixelInformation.BlueMask     = 0x000000ff;
      ModeData->PixelInformation.ReservedMask = 0xff000000;
      ModeData->PixelsPerScanLine             = ModeData->HorizontalResolution;
      if (ModeData->PixelFormat != PixelBltOnly) {
        ModeData->FrameBufferBase             = (EFI_PHYSICAL_ADDRESS) (UINTN) (Pci.Device.Bar[GMADR_BAR_INDEX] & 0xF0000000);
        ModeData->FrameBufferSize             = FB_SIZE;
      } else {
        ModeData->FrameBufferBase             = 0;
        ModeData->FrameBufferSize             = 0;
      }

      ModeData ++;
      ValidModeCount ++;
    }

    VideoMode ++;
  }

  if (ValidModeCount == 0) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate buffer for Graphics Output Protocol mode information
  //
  Status = gBS->AllocatePool (
                EfiBootServicesData,
                sizeof (EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE),
                (VOID **) &Private->GraphicsOutput.Mode
                );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = gBS->AllocatePool (
                EfiBootServicesData,
                sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION),
                (VOID **) &Private->GraphicsOutput.Mode->Info
                );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Private->GraphicsOutput.Mode->MaxMode = ValidModeCount;

  //
  // Initialize the hardware with a prefer mode
  //
  PreferMode = 0;
  ModeData = &Private->ModeData[0];
  for (Index = 0; Index < ValidModeCount; Index++, ModeData++) {
    if ((ModeData->HorizontalResolution == 800 && ModeData->VerticalResolution == 600)
        || (ModeData->HorizontalResolution == 1024 && ModeData->VerticalResolution == 768)
        ) {
      PreferMode = Index;
      break;
    }
  }

  Private->GraphicsOutput.Mode->Mode = GRAPHICS_OUTPUT_INVALIDE_MODE_NUMBER;
  Private->GraphicsOutput.SetMode (&Private->GraphicsOutput, PreferMode);

  return EFI_SUCCESS;
}

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
{
  EFI_PCI_IO_PROTOCOL *PciIo;
  EFI_STATUS          Status;

  PciIo = Private->PciIo;

  //
  // First shut down the controller
  //
  Status = ShutdownGfxController (Private);

  //
  // Shutdown the hardware
  //
  Private->PciIo->Attributes (
                    Private->PciIo,
                    EfiPciIoAttributeOperationDisable,
                    EFI_PCI_DEVICE_ENABLE | EFI_PCI_IO_ATTRIBUTE_VGA_MEMORY | EFI_PCI_IO_ATTRIBUTE_VGA_IO,
                    NULL
                    );

  //
  // Free graphics output protocol occupied resource
  //
  if (Private->GraphicsOutput.Mode != NULL) {
    if (Private->GraphicsOutput.Mode->Info != NULL) {
        gBS->FreePool (Private->GraphicsOutput.Mode->Info);
    }
    gBS->FreePool (Private->GraphicsOutput.Mode);
  }

  //
  // Free  EDID protocol occupied resource
  //
  if (Private->EdidDiscovered.Edid != NULL) {
    gBS->FreePool (Private->EdidDiscovered.Edid);
  }
  if (Private->EdidActive.Edid != NULL) {
    gBS->FreePool (Private->EdidActive.Edid);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
ClearScreen (
  INTEL_PRIVATE_DATA        *Private,
  UINT32                    Color
  )
/*++

Routine Description:

  This function paints the display area by writing Color to the 
  entire frame buffer.  To clear the screen, BLACK is written 
  to the frame buffer.


Arguments:

  Private  - Pointer to the private data members
   
  Color    - Color used to fill the screen.  For screen 
             clears, color is typically BLACK.
   
Returns:

  EFI_SUCCESS if successful, else the status from the COLOR_BLT
  function is returned.
   
--*/
{
  EFI_STATUS  Status;
  INTEL_GRAPHICS_OUTPUT_MODE_DATA      *ModeData;

  ModeData = &Private->ModeData[Private->GraphicsOutput.Mode->Mode];
  Status = Private->GraphicsOutput.Blt(
  	                                 &Private->GraphicsOutput,
  	                                 (VOID*)&Color,
  	                                 EfiBltVideoFill,
  	                                 0,
  	                                 0,
  	                                 0,
  	                                 0,
  	                                 ModeData->HorizontalResolution,
  	                                 ModeData->VerticalResolution,
  	                                 ModeData->HorizontalResolution * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
  	                                 );

  return Status;
}

EFI_STATUS
ShutdownGfxController (
  INTEL_PRIVATE_DATA  *Private
  )
/*++

Routine Description:

  Shut down GFX controller

Arguments:

  Private - Pointer to the private data members

Returns:

  EFI_SUCCESS - Successfully return

--*/
{
  UINT32              Index;
  UINT32              Value;
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;

  PciIo = Private->PciIo;

  //
  // ------------------------------------------------
  // Issue shutdown sequence
  // Done in two lines for readability and debugging
  // purposes.
  //
  for (Index = 0; Index < mNUM_SHUTDOWN_ENTRIES; Index++) {
    Value = mDISPLAY_SHUTDOWN[Index].Value;
    Status = PciIo->Mem.Write (
                          PciIo,
                          EfiPciIoWidthUint32,
                          MMADR_BAR_INDEX,
                          mDISPLAY_SHUTDOWN[Index].RegisterAddress,
                          1,
                          &Value
                          );
    if (mDISPLAY_SHUTDOWN[Index].DoubleBuffer) {
      Status = WaitForVBlank (Private, VBLANK_TIMEOUT);
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
ConfigureGfxController (
  INTEL_PRIVATE_DATA  *Private,
  INTEL_VIDEO_MODES   *ModeData
  )
/*++

Routine Description:

  Config GFX controller

Arguments:

  Private   - Pointer to the private data members
  ModeData  - Video mode data buffer

Returns:

  EFI_SUCCESS - Successfully return

--*/
{
  UINT32              Index;
  UINT32              Value;
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;

  Status  = EFI_SUCCESS;

  PciIo   = Private->PciIo;

  //
  // -----------------------------------------------
  // Set pipe timings, planes and DPLLs to desired mode values
  // Loop thru all the entries in the configuration table.
  //
  for (Index = 0; Index < NUM_DS_ENTRIES; Index++) {
    //
    // Done in two lines for readability and debugging
    // purposes.
    //
    Value = ModeData->VideoModeData[Index].Value;
    Status = PciIo->Mem.Write (
                          PciIo,
                          EfiPciIoWidthUint32,
                          MMADR_BAR_INDEX,
                          ModeData->VideoModeData[Index].RegisterAddress,
                          1,
                          &Value
                          );

    //
    // check if register program must wait for a vblank interval
    // to occur for the register program to take effect..
    //
    if (ModeData->VideoModeData[Index].DoubleBuffer) {
      Status = WaitForVBlank (Private, VBLANK_TIMEOUT);
    }
  }

  return Status;
}

EFI_STATUS
StartupGfxController (
  INTEL_PRIVATE_DATA  *Private
  )
/*++

Routine Description:

  Startup GFX controller

Arguments:

  Private - Pointer to the private data members

Returns:

  EFI_SUCCESS - Successfully return

--*/
{
  UINT32              Index;
  UINT32              Value;
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;

  Status  = EFI_SUCCESS;

  PciIo   = Private->PciIo;

  //
  // ------------------------------------------------
  // Issue startup sequence
  // Loop thru all the entries in the startup table.
  //
  for (Index = 0; Index < mNUM_STARTUP_ENTRIES; Index++) {
    //
    // Done in two lines for readability and debugging
    // purposes.
    //
    Value = mDISPLAY_STARTUP[Index].Value;
    Status = PciIo->Mem.Write (
                          PciIo,
                          EfiPciIoWidthUint32,
                          MMADR_BAR_INDEX,
                          mDISPLAY_STARTUP[Index].RegisterAddress,
                          1,
                          &Value
                          );

    //
    // check if register program must wait for a vblank interval
    // to occur for the register program to take effect..
    //
    if (mDISPLAY_STARTUP[Index].DoubleBuffer) {
      Status = WaitForVBlank (Private, VBLANK_TIMEOUT);
    }
  }

  return Status;
}

EFI_STATUS
WaitForVBlank (
  INTEL_PRIVATE_DATA        *Private,
  UINT32                    Timeout
  )
/*++

Routine Description:

  Wait for VBlank

Arguments:

  Private - Pointer to the private data members
  Timeout - Time out value

Returns:

  EFI_SUCCESS - Successfully return

--*/
{
  EFI_STATUS          Status;
  UINT32              Value;
  UINT32              Index;
  EFI_PCI_IO_PROTOCOL *PciIo;

  Status  = EFI_SUCCESS;
  Index   = 0;

  PciIo   = Private->PciIo;

  //
  // Read the bit and clear the status bit by writing a 1 to the VBLANK bit
  //
  Status = PciIo->Mem.Read (
                        PciIo,
                        EfiPciIoWidthUint32,
                        MMADR_BAR_INDEX,
                        PIPEASTAT,
                        1,
                        &Value
                        );
  Value |= VBLANK_MASK;
  //
  // set the VBlank status but causing it to clear
  //
  Status = PciIo->Mem.Write (
                        PciIo,
                        EfiPciIoWidthUint32,
                        MMADR_BAR_INDEX,
                        PIPEASTAT,
                        1,
                        &Value
                        );
  Status = PciIo->Mem.Write (
                        PciIo,
                        EfiPciIoWidthUint32,
                        MMADR_BAR_INDEX,
                        PIPEASTAT,
                        1,
                        &Value
                        );

  //
  // Loop Timeout # of times, waiting for VBLANK_STATUS bit to become set
  //
  while (Index < Timeout) {
    Status = PciIo->Mem.Read (
                          PciIo,
                          EfiPciIoWidthUint32,
                          MMADR_BAR_INDEX,
                          PIPEASTAT,
                          1,
                          &Value
                          );
    Index++;
    //
    // increment our loop counter
    //
    // If the VBLANK_MASK is set, a VBLANK has occurred and
    // we are done here.
    //
    if (Value & VBLANK_MASK) {
      break;
    }
  }
  //
  // Check the loop counter against the requested Timeout value.
  // if the time out was reached, return an error.
  //
  if (Index >= Timeout) {
    Status = EFI_TIMEOUT;
  }

  return Status;
}

EFI_STATUS
ShutdownLVDS (
  INTEL_PRIVATE_DATA  *Private
  )
/*++

Routine Description:

  Shut down GFX controller

Arguments:

  Private - Pointer to the BearLake private data members

Returns:

  EFI_SUCCESS - Successfully return

--*/
{
  UINT32              Index;
  UINT32              Value;
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;

  PciIo = Private->PciIo;

  //
  // ------------------------------------------------
  // Issue shutdown sequence
  // Done in two lines for readability and debugging
  // purposes.
  //
  for (Index = 0; TRUE; Index++) {
    if (LVDS_SHUTDOWN[Index].RegisterAddress == 0) break;
    Value = LVDS_SHUTDOWN[Index].Value;
    Status = PciIo->Mem.Write (
                          PciIo,
                          EfiPciIoWidthUint32,
                          MMADR_BAR_INDEX,
                          LVDS_SHUTDOWN[Index].RegisterAddress,
                          1,
                          &Value
                          );
    if (LVDS_SHUTDOWN[Index].DoubleBuffer) {
      Status = WaitForVBlank (Private, VBLANK_TIMEOUT);
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS StartLVDS (
  INTEL_PRIVATE_DATA  *Private,
  INTEL_VIDEO_MODES   *ModeData
  )
{
  UINT32              Index;
  UINT32              Value;
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;

  Status  = EFI_SUCCESS;

  PciIo   = Private->PciIo;

  //
  // -----------------------------------------------
  // Set pipe timings, planes and DPLLs to desired mode values
  // Loop thru all the entries in the configuration table.
  //
  for (Index = 0; TRUE; Index++) {
    //
    // Done in two lines for readability and debugging
    // purposes.
    //
    if (ModeData->LVDSModeData[Index].RegisterAddress == 0) break;
    Value = ModeData->LVDSModeData[Index].Value;
    Status = PciIo->Mem.Write (
                          PciIo,
                          EfiPciIoWidthUint32,
                          MMADR_BAR_INDEX,
                          ModeData->LVDSModeData[Index].RegisterAddress,
                          1,
                          &Value
                          );

    //
    // check if register program must wait for a vblank interval
    // to occur for the register program to take effect..
    //
    if (ModeData->LVDSModeData[Index].DoubleBuffer) {
      Status = WaitForVBlank (Private, VBLANK_TIMEOUT);
    }
  }

  return Status;
	
}

EFI_STATUS
InitializeGraphicsMode (
  INTEL_PRIVATE_DATA  *Private,
  INTEL_VIDEO_MODES   *ModeData
  )
/*++

Routine Description:

  Initialize graphics mode

Arguments:

  Private   - Pointer to the private data members

  ModeData  - Graphics mode data buffer

Returns:

  EFI_SUCCESS - Successfully return

--*/
{
  EFI_STATUS  Status;

  //
  // Program the controller registers.
  //
  Status = ConfigureGfxController (Private, ModeData);

  //
  // If the driver is already started, then the GTT is setup and
  // we should execute the h/w start-up sequence; otherwise, the
  // constructor will call StartupGfxController itself.  The
  // controller will hang if the startup sequence is executed
  // befor the GTT is setup (ie, we need this following check!)
  //
  Status = StartupGfxController (Private);

  ShutdownLVDS(Private);
  StartLVDS(Private, ModeData);
  return Status;
}

VOID
EFIAPI
ClearGfxController (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  INTEL_PRIVATE_DATA         *Private;
  EFI_STATUS                 Status;

  Private = (INTEL_PRIVATE_DATA *) Context;

  Status = ShutdownGfxController (Private);

  //
  // Shutdown the hardware
  //
  Private->PciIo->Attributes (
                    Private->PciIo,
                    EfiPciIoAttributeOperationDisable,
                    EFI_PCI_DEVICE_ENABLE | EFI_PCI_IO_ATTRIBUTE_VGA_MEMORY | EFI_PCI_IO_ATTRIBUTE_VGA_IO,
                    NULL
                    );

  return ;
}
