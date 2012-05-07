/*
 * libeg/screen.c
 * Screen handling functions
 *
 * Copyright (c) 2006 Christoph Pfisterer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Christoph Pfisterer nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "libegint.h"

#include <efiUgaDraw.h>
#include <efiGraphicsOutput.h>
#include <efiConsoleControl.h>

// Console defines and variables

static EFI_GUID ConsoleControlProtocolGuid = EFI_CONSOLE_CONTROL_PROTOCOL_GUID;
static EFI_CONSOLE_CONTROL_PROTOCOL *ConsoleControl = NULL;

static EFI_GUID UgaDrawProtocolGuid = EFI_UGA_DRAW_PROTOCOL_GUID;
static EFI_UGA_DRAW_PROTOCOL *UgaDraw = NULL;

static EFI_GUID GraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
static EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput = NULL;

static BOOLEAN egHasGraphics = FALSE;
static UINTN egScreenWidth  = 1024;
static UINTN egScreenHeight = 768;

//
// Screen handling
//

CHAR8* egDumpGOPVideoModes(VOID)
{
    EFI_STATUS  Status;
    UINT32      MaxMode;
    UINT32      Mode;
    UINTN       SizeOfInfo;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
    CHAR16      *PixelFormatDesc;
    CHAR8       *OutputBase;
    CHAR8       *Output;
    UINTN       OutputSize;
    UINTN       Len;
    
    if (GraphicsOutput == NULL) {
        return NULL;
    }
    
    // alloc mem, caller should release
    OutputSize = 1024;
    OutputBase = AllocateZeroPool(OutputSize);
    Output = OutputBase;
    
    // get dump
    MaxMode = GraphicsOutput->Mode->MaxMode;
    Mode = GraphicsOutput->Mode->Mode;
    AsciiSPrint(Output, OutputSize, "Available graphics modes for refit.conf screen_resolution:\nCurr. Mode = %d, MaxMode = %d, FB = %lx, FB size=0x%x\n",
          Mode, MaxMode, GraphicsOutput->Mode->FrameBufferBase, GraphicsOutput->Mode->FrameBufferSize);
    Len = AsciiStrLen(OutputBase);
    OutputSize = 1024 - Len;
    Output = OutputBase + Len;
    
    for (Mode = 0; Mode < MaxMode; Mode++) {
        Status = GraphicsOutput->QueryMode(GraphicsOutput, Mode, &SizeOfInfo, &Info);
        if (Status == EFI_SUCCESS) {
            
            switch (Info->PixelFormat) {
                case PixelRedGreenBlueReserved8BitPerColor:
                    PixelFormatDesc = L"8bit RGB";
                    break;
                    
                case PixelBlueGreenRedReserved8BitPerColor:
                    PixelFormatDesc = L"8bit BGR";
                    break;
                    
                case PixelBitMask:
                    PixelFormatDesc = L"BITMASK";
                    break;
                    
                case PixelBltOnly:
                    PixelFormatDesc = L"NO FB";
                    break;
                    
                default:
                    PixelFormatDesc = L"invalid";
                    break;
            }
            
            AsciiSPrint(Output, OutputSize, "- Mode %d: %dx%d PixFmt = %s, PixPerScanLine = %d\n",
                  Mode, Info->HorizontalResolution, Info->VerticalResolution, PixelFormatDesc, Info->PixelsPerScanLine);
        } else {
            AsciiSPrint(Output, OutputSize, "- Mode %d: %r\n", Mode, Status);
        }
        Len = AsciiStrLen(OutputBase);
        OutputSize = 1024 - Len;
        Output = OutputBase + Len;
    }
    return OutputBase;
}

EFI_STATUS egSetMaxResolution()
{
  EFI_STATUS  Status = EFI_UNSUPPORTED;
  UINT32      Width = 0;
  UINT32      Height = 0;
  UINT32      BestMode = 0;
  UINT32      MaxMode;
  UINT32      Mode;
  UINTN       SizeOfInfo;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  
  MaxMode = GraphicsOutput->Mode->MaxMode;
  for (Mode = 0; Mode < MaxMode; Mode++) {
    Status = GraphicsOutput->QueryMode(GraphicsOutput, Mode, &SizeOfInfo, &Info);
    if (Status == EFI_SUCCESS) {
      if (Width > Info->HorizontalResolution) {
        continue;
      }
      if (Height > Info->VerticalResolution) {
        continue;
      }
      Width = Info->HorizontalResolution;
      Height = Info->VerticalResolution;
      BestMode = Mode;
    }
  }
  MsgLog("Found best mode %d: %dx%d\n", BestMode, Width, Height);
  GraphicsOutput->SetMode(GraphicsOutput, Mode);
  egScreenWidth = Width;
  egScreenHeight = Height;
  return Status;
}

EFI_STATUS egSetScreenResolution(IN CHAR16 *WidthHeight)
{
    EFI_STATUS  Status = EFI_UNSUPPORTED;
    UINT32      Width;
    UINT32      Height;
    CHAR16      *HeightP;
    UINT32      MaxMode;
    UINT32      Mode;
    UINTN       SizeOfInfo;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
    
    if (WidthHeight == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    // we are expecting WidthHeight=L"1024x768"
    // parse Width and Height
    HeightP = WidthHeight;
    while (*HeightP != L'\0' && *HeightP != L'x' && *HeightP != L'X') {
        HeightP++;
    }
    if (HeightP == L'\0') {
        return EFI_INVALID_PARAMETER;
    }
    *HeightP = L'\0';
    HeightP++;
    Width = (UINT32)StrDecimalToUintn(WidthHeight);
    Height = (UINT32)StrDecimalToUintn(HeightP);
    
    // iterate through modes and set it if found
    MaxMode = GraphicsOutput->Mode->MaxMode;
    for (Mode = 0; Mode < MaxMode; Mode++) {
        Status = GraphicsOutput->QueryMode(GraphicsOutput, Mode, &SizeOfInfo, &Info);
        if (Status == EFI_SUCCESS) {
            if (Width == Info->HorizontalResolution && Height == Info->VerticalResolution) {
                GraphicsOutput->SetMode(GraphicsOutput, Mode);
                egScreenWidth = Width;
                egScreenHeight = Height;
                break;
            }
        } else {
            Status = EFI_UNSUPPORTED;
        }
    }
    return Status;
}

VOID egInitScreen(VOID)
{
    EFI_STATUS Status;
    UINT32 UGAWidth, UGAHeight, UGADepth, UGARefreshRate;
    
    // get protocols
    Status = EfiLibLocateProtocol(&ConsoleControlProtocolGuid, (VOID **) &ConsoleControl);
    if (EFI_ERROR(Status))
        ConsoleControl = NULL;
    
    Status = EfiLibLocateProtocol(&UgaDrawProtocolGuid, (VOID **) &UgaDraw);
    if (EFI_ERROR(Status))
        UgaDraw = NULL;
    
    Status = EfiLibLocateProtocol(&GraphicsOutputProtocolGuid, (VOID **) &GraphicsOutput);
    if (EFI_ERROR(Status))
        GraphicsOutput = NULL;
    
    // get screen size
    egHasGraphics = FALSE;
    if (GraphicsOutput != NULL) {
        if (GlobalConfig.ScreenResolution != NULL) {
            egSetScreenResolution(GlobalConfig.ScreenResolution);
        } else {
            egSetMaxResolution();
        }

        egScreenWidth = GraphicsOutput->Mode->Info->HorizontalResolution;
        egScreenHeight = GraphicsOutput->Mode->Info->VerticalResolution;
        egHasGraphics = TRUE;
    } else if (UgaDraw != NULL) {
        Status = UgaDraw->GetMode(UgaDraw, &UGAWidth, &UGAHeight, &UGADepth, &UGARefreshRate);
        if (EFI_ERROR(Status)) {
            UgaDraw = NULL;   // graphics not available
        } else {
            egScreenWidth  = UGAWidth;
            egScreenHeight = UGAHeight;
            egHasGraphics = TRUE;
        }
    }
}

VOID egGetScreenSize(OUT UINTN *ScreenWidth, OUT UINTN *ScreenHeight)
{
    if (ScreenWidth != NULL)
        *ScreenWidth = egScreenWidth;
    if (ScreenHeight != NULL)
        *ScreenHeight = egScreenHeight;
}

CHAR16 * egScreenDescription(VOID)
{
    if (egHasGraphics) {
        if (GraphicsOutput != NULL) {
            return PoolPrint(L"Graphics Output (UEFI), %dx%d",
                             egScreenWidth, egScreenHeight);
        } else if (UgaDraw != NULL) {
            return PoolPrint(L"UGA Draw (EFI 1.10), %dx%d",
                             egScreenWidth, egScreenHeight);
        } else {
            return L"Internal Error";
        }
    } else {
        return L"Text Console";
    }
}

BOOLEAN egHasGraphicsMode(VOID)
{
    return egHasGraphics;
}

BOOLEAN egIsGraphicsModeEnabled(VOID)
{
    EFI_CONSOLE_CONTROL_SCREEN_MODE CurrentMode;
    
    if (ConsoleControl != NULL) {
        ConsoleControl->GetMode(ConsoleControl, &CurrentMode, NULL, NULL);
        return (CurrentMode == EfiConsoleControlScreenGraphics) ? TRUE : FALSE;
    }
    
    return FALSE;
}

VOID egSetGraphicsModeEnabled(IN BOOLEAN Enable)
{
    EFI_CONSOLE_CONTROL_SCREEN_MODE CurrentMode;
    EFI_CONSOLE_CONTROL_SCREEN_MODE NewMode;
    
    if (ConsoleControl != NULL) {
        ConsoleControl->GetMode(ConsoleControl, &CurrentMode, NULL, NULL);
        
        NewMode = Enable ? EfiConsoleControlScreenGraphics
                         : EfiConsoleControlScreenText;
        if (CurrentMode != NewMode)
            ConsoleControl->SetMode(ConsoleControl, NewMode);
    }
}

//
// Drawing to the screen
//

VOID egClearScreen(IN EG_PIXEL *Color)
{
    EFI_UGA_PIXEL FillColor;
    
    if (!egHasGraphics)
        return;
    
    FillColor.Red   = Color->r;
    FillColor.Green = Color->g;
    FillColor.Blue  = Color->b;
    FillColor.Reserved = 0;
    
    if (GraphicsOutput != NULL) {
        // EFI_GRAPHICS_OUTPUT_BLT_PIXEL and EFI_UGA_PIXEL have the same
        // layout, and the header from TianoCore actually defines them
        // to be the same type.
        GraphicsOutput->Blt(GraphicsOutput, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)&FillColor, EfiBltVideoFill,
                            0, 0, 0, 0, egScreenWidth, egScreenHeight, 0);
    } else if (UgaDraw != NULL) {
        UgaDraw->Blt(UgaDraw, &FillColor, EfiUgaVideoFill,
                     0, 0, 0, 0, egScreenWidth, egScreenHeight, 0);
    }
}

VOID egDrawImage(IN EG_IMAGE *Image, IN UINTN ScreenPosX, IN UINTN ScreenPosY)
{
    if (!egHasGraphics)
        return;
    
    if (Image->HasAlpha) {
        Image->HasAlpha = FALSE;
        egSetPlane(PLPTR(Image, a), 0, Image->Width * Image->Height);
    }
    
    if (GraphicsOutput != NULL) {
        GraphicsOutput->Blt(GraphicsOutput, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)Image->PixelData, EfiBltBufferToVideo,
                            0, 0, ScreenPosX, ScreenPosY, Image->Width, Image->Height, 0);
    } else if (UgaDraw != NULL) {
        UgaDraw->Blt(UgaDraw, (EFI_UGA_PIXEL *)Image->PixelData, EfiUgaBltBufferToVideo,
                     0, 0, ScreenPosX, ScreenPosY, Image->Width, Image->Height, 0);
    }
}

VOID egDrawImageArea(IN EG_IMAGE *Image,
                     IN UINTN AreaPosX, IN UINTN AreaPosY,
                     IN UINTN AreaWidth, IN UINTN AreaHeight,
                     IN UINTN ScreenPosX, IN UINTN ScreenPosY)
{
    if (!egHasGraphics)
        return;
    
    egRestrictImageArea(Image, AreaPosX, AreaPosY, &AreaWidth, &AreaHeight);
    if (AreaWidth == 0)
        return;
    
    if (Image->HasAlpha) {
        Image->HasAlpha = FALSE;
        egSetPlane(PLPTR(Image, a), 0, Image->Width * Image->Height);
    }
    
    if (GraphicsOutput != NULL) {
        GraphicsOutput->Blt(GraphicsOutput, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)Image->PixelData, EfiBltBufferToVideo,
                            AreaPosX, AreaPosY, ScreenPosX, ScreenPosY, AreaWidth, AreaHeight, Image->Width * 4);
    } else if (UgaDraw != NULL) {
        UgaDraw->Blt(UgaDraw, (EFI_UGA_PIXEL *)Image->PixelData, EfiUgaBltBufferToVideo,
                     AreaPosX, AreaPosY, ScreenPosX, ScreenPosY, AreaWidth, AreaHeight, Image->Width * 4);
    }
}

//
// Make a screenshot
//

EFI_STATUS egScreenShot(VOID)
{
    EFI_STATUS      Status = EFI_NOT_READY;
    EG_IMAGE        *Image;
    UINT8           *FileData;
    UINTN           FileDataLength;
    UINTN           Index;
    CHAR16					ScreenshotName[128];
      
    if (!egHasGraphics)
        return EFI_NOT_READY;
    
    // allocate a buffer for the whole screen
    Image = egCreateImage(egScreenWidth, egScreenHeight, FALSE);
    if (Image == NULL) {
        Print(L"Error egCreateImage returned NULL\n");
        return EFI_NO_MEDIA;
    }
    
    // get full screen image
    if (GraphicsOutput != NULL) {
        GraphicsOutput->Blt(GraphicsOutput, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)Image->PixelData,
                            EfiBltVideoToBltBuffer,
                            0, 0, 0, 0, Image->Width, Image->Height, 0);
    } else if (UgaDraw != NULL) {
        UgaDraw->Blt(UgaDraw, (EFI_UGA_PIXEL *)Image->PixelData, EfiUgaVideoToBltBuffer,
                     0, 0, 0, 0, Image->Width, Image->Height, 0);
    }
    
    // encode as BMP
    egEncodeBMP(Image, &FileData, &FileDataLength);
    egFreeImage(Image);
    if (FileData == NULL) {
        Print(L"Error egEncodeBMP returned NULL\n");
        return EFI_NO_MEDIA;
    }
  
  for (Index=0; Index < 20; Index++) {
    UnicodeSPrint(ScreenshotName, 128, L"EFI\\misc\\screenshot%d.bmp", Index);
    if(!FileExists(SelfRootDir, ScreenshotName)){
      Status = egSaveFile(SelfRootDir, ScreenshotName, FileData, FileDataLength);
      if (!EFI_ERROR(Status)) {
        break;
      }		
    }
  }
  // else save to file on the ESP
  if (EFI_ERROR(Status)) {
    for (Index=0; Index < 20; Index++) {
      UnicodeSPrint(ScreenshotName, 128, L"EFI\\misc\\screenshot%d.bmp", Index);
//     if(!FileExists(NULL, ScreenshotName)){
        Status = egSaveFile(NULL, ScreenshotName, FileData, FileDataLength);
        if (!EFI_ERROR(Status)) {
          break;
        }		
//      }
    }
    CheckError(Status, L"Error egSaveFile\n");
  }
  FreePool(FileData);    
  return Status;
}

/* EOF */
