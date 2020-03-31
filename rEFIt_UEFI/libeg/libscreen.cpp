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
#include "lodepng.h"


//#include <efiUgaDraw.h>
#include <Protocol/GraphicsOutput.h>
//#include <Protocol/efiConsoleControl.h>

// Console defines and variables

static EFI_GUID ConsoleControlProtocolGuid = EFI_CONSOLE_CONTROL_PROTOCOL_GUID;
static EFI_CONSOLE_CONTROL_PROTOCOL *ConsoleControl = NULL;

static EFI_GUID UgaDrawProtocolGuid = EFI_UGA_DRAW_PROTOCOL_GUID;
static EFI_UGA_DRAW_PROTOCOL *UgaDraw = NULL;

static EFI_GUID GraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
static EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput = NULL;

static BOOLEAN egHasGraphics = FALSE;
static UINTN egScreenWidth  = 0; //1024;
static UINTN egScreenHeight = 0; //768;

static BOOLEAN IgnoreConsoleSetMode = FALSE;
static EFI_CONSOLE_CONTROL_SCREEN_MODE CurrentForcedConsoleMode = EfiConsoleControlScreenText;
static EFI_CONSOLE_CONTROL_PROTOCOL_GET_MODE ConsoleControlGetMode = NULL;
static EFI_CONSOLE_CONTROL_PROTOCOL_SET_MODE ConsoleControlSetMode = NULL;

static EFI_STATUS GopSetModeAndReconnectTextOut(IN UINT32 ModeNumber);

//
// Wrapped ConsoleControl GetMode() implementation - for blocking resolution switch when changing modes
//
EFI_STATUS EFIAPI
egConsoleControlGetMode(IN EFI_CONSOLE_CONTROL_PROTOCOL *This, OUT EFI_CONSOLE_CONTROL_SCREEN_MODE *Mode, OUT BOOLEAN *GopUgaExists, OPTIONAL OUT BOOLEAN *StdInLocked OPTIONAL) {
    if (IgnoreConsoleSetMode) {
        *Mode = CurrentForcedConsoleMode;
        if (GopUgaExists)
            *GopUgaExists = TRUE;
        if (StdInLocked)
            *StdInLocked = FALSE;
        return EFI_SUCCESS;
    }

    return ConsoleControlGetMode(This, Mode, GopUgaExists, StdInLocked);
}

EFI_STATUS EFIAPI
egConsoleControlSetMode(IN EFI_CONSOLE_CONTROL_PROTOCOL *This, IN EFI_CONSOLE_CONTROL_SCREEN_MODE Mode) {
    // Pretend that we updated our console mode but do not call SetMode itself to avoid breaking the resolution.
    // Please note, that it is also relevant for proper boot.efi progress bar rendering with FileVault 2.
    // See more details in refit/main.c
    if (IgnoreConsoleSetMode) {
        CurrentForcedConsoleMode = Mode;
        return EFI_SUCCESS;
    }

    return ConsoleControlSetMode(This, Mode);
}

//
// Screen handling
//
/*
VOID egDumpGOPVideoModes(VOID)
{
    EFI_STATUS  Status;
    UINT32      MaxMode;
    UINT32      Mode;
    UINTN       SizeOfInfo;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
    CHAR16      *PixelFormatDesc;
    
    if (GraphicsOutput == NULL) {
        return;
    }
    
    // get dump
    MaxMode = GraphicsOutput->Mode->MaxMode;
    Mode = GraphicsOutput->Mode->Mode;
//    MsgLog("Available graphics modes for refit.conf screen_resolution:\n");
//    MsgLog("Curr. Mode = %d, Modes = %d, FB = %lx, FB size=0x%X\n",
//           Mode, MaxMode, GraphicsOutput->Mode->FrameBufferBase, GraphicsOutput->Mode->FrameBufferSize);
    
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
            
            MsgLog("- Mode %d: %dx%d PixFmt = %ls, PixPerScanLine = %d\n",
                  Mode, Info->HorizontalResolution, Info->VerticalResolution, PixelFormatDesc, Info->PixelsPerScanLine);
        } else {
            MsgLog("- Mode %d: %s\n", Mode, strerror(Status));
        }
    }
    
}
*/
VOID egDumpSetConsoleVideoModes(VOID)
{
  UINTN i;
  UINTN Width, Height;
  UINTN BestMode = 0, BestWidth = 0, BestHeight = 0;
  EFI_STATUS Status;
  STATIC int Once=0;
  
  if (gST->ConOut != NULL && gST->ConOut->Mode != NULL) {
    if (!Once) {
      MsgLog("Console modes reported: %d, available modes:\n",gST->ConOut->Mode->MaxMode);
    }
    
    for (i=1; i <= (UINTN)gST->ConOut->Mode->MaxMode; i++) {
      Status = gST->ConOut->QueryMode(gST->ConOut, i-1, &Width, &Height);
      if (Status == EFI_SUCCESS) {
        //MsgLog("  Mode %d: %dx%d%ls\n", i, Width, Height, (i-1==(UINTN)gST->ConOut->Mode->Mode)?L" (current mode)":L"");
        if (!Once) {
			MsgLog(" - [%02llu]: %llux%llu%ls\n", i, Width, Height, (i-1==(UINTN)gST->ConOut->Mode->Mode)?L" (current mode)":L"");
        }
        // Select highest mode (-1) or lowest mode (-2) as/if requested
        if ((GlobalConfig.ConsoleMode == -1 && (BestMode == 0 || Width > BestWidth || (Width == BestWidth && Height > BestHeight))) ||
            (GlobalConfig.ConsoleMode == -2 && (BestMode == 0 || Width < BestWidth || (Width == BestWidth && Height < BestHeight)))) {
          BestMode = i;
          BestWidth = Width;
          BestHeight = Height;
        }
      }
    }
    Once++;
  } else {
    MsgLog("Console modes are not available.\n");
    return;
  }
  
  if (GlobalConfig.ConsoleMode > 0) {
    // Specific mode chosen, try to set it
    BestMode = GlobalConfig.ConsoleMode;
  }
  
  if (BestMode >= 1 && BestMode <= (UINTN)gST->ConOut->Mode->MaxMode) {
    // Mode is valid
    if (BestMode-1 != (UINTN)gST->ConOut->Mode->Mode) {
      Status = gST->ConOut->SetMode(gST->ConOut, BestMode-1);
		MsgLog("  Setting mode (%llu): %s\n", BestMode, strerror(Status));
    } else {
		MsgLog("  Selected mode (%llu) is already set\n", BestMode);
    }
  } else if (BestMode != 0) {
	  MsgLog("  Selected mode (%llu) is not valid\n", BestMode);
  }
}

EFI_STATUS egSetMaxResolution()
{
  EFI_STATUS  Status = EFI_UNSUPPORTED;
  UINT32      Width = 0;
  UINT32      Height = 0;
  UINT32      BestMode = 0;
  UINT32      MaxMode;
  UINT32      Mode;
  UINTN       SizeOfInfo = 0;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info = NULL;
  
  if (GraphicsOutput == NULL) {
    return EFI_UNSUPPORTED;
  }

  MsgLog("SetMaxResolution: ");
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
  MsgLog("found best mode %d: %dx%d\n", BestMode, Width, Height);
  // check if requested mode is equal to current mode
  if (BestMode == GraphicsOutput->Mode->Mode) {
    MsgLog(" - already set\n");
    egScreenWidth = GraphicsOutput->Mode->Info->HorizontalResolution;
    egScreenHeight = GraphicsOutput->Mode->Info->VerticalResolution;
    Status = EFI_SUCCESS;
  } else {
    //Status = GraphicsOutput->SetMode(GraphicsOutput, BestMode);
    Status = GopSetModeAndReconnectTextOut(BestMode);
    if (Status == EFI_SUCCESS) {
      egScreenWidth = Width;
      egScreenHeight = Height;
      MsgLog(" - set\n");
    } else {
      // we can not set BestMode - search for first one that we can
      MsgLog(" - %s\n", strerror(Status));
      Status = egSetMode(1);
    }
  }

  return Status;
}

EFI_STATUS egSetMode(INT32 Next)
{
  EFI_STATUS  Status = EFI_UNSUPPORTED;
  UINT32      MaxMode;
  UINTN       SizeOfInfo = 0;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info = NULL;
  INT32      Mode;
  UINT32     Index = 0;
  
  if (GraphicsOutput == NULL) {
    return EFI_UNSUPPORTED;
  }

  MaxMode = GraphicsOutput->Mode->MaxMode;
  Mode = GraphicsOutput->Mode->Mode;
  while (EFI_ERROR(Status) && Index <= MaxMode) {
    Mode = Mode + Next;
    Mode = (Mode >= (INT32)MaxMode)?0:Mode;
    Mode = (Mode < 0)?((INT32)MaxMode - 1):Mode;
    Status = GraphicsOutput->QueryMode(GraphicsOutput, (UINT32)Mode, &SizeOfInfo, &Info);
    MsgLog("QueryMode %d Status=%s\n", Mode, strerror(Status));
    if (Status == EFI_SUCCESS) {
      //Status = GraphicsOutput->SetMode(GraphicsOutput, (UINT32)Mode);
      Status = GopSetModeAndReconnectTextOut((UINT32)Mode);
      //MsgLog("SetMode %d Status=%s\n", Mode, strerror(Status));
      egScreenWidth = GraphicsOutput->Mode->Info->HorizontalResolution;
      egScreenHeight = GraphicsOutput->Mode->Info->VerticalResolution;
    }
    Index++;
  }

  return Status;
}

EFI_STATUS egSetScreenResolution(IN const CHAR16 *WidthHeight)
{
    EFI_STATUS  Status = EFI_UNSUPPORTED;
    UINT32      Width;
    UINT32      Height;
    const CHAR16      *HeightP;
    UINT32      MaxMode;
    UINT32      Mode;
    UINTN       SizeOfInfo;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;

    if (GraphicsOutput == NULL) {
        return EFI_UNSUPPORTED;
    }

    if (WidthHeight == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    MsgLog("SetScreenResolution: %ls", WidthHeight);
    // we are expecting WidthHeight=L"1024x768"
    // parse Width and Height
    HeightP = WidthHeight;
    while (*HeightP != L'\0' && *HeightP != L'x' && *HeightP != L'X') {
        HeightP++;
    }
    if (*HeightP == L'\0') {
        return EFI_INVALID_PARAMETER;
    }
    HeightP++;
    Width = (UINT32)StrDecimalToUintn(WidthHeight);
    Height = (UINT32)StrDecimalToUintn(HeightP);
    
    // check if requested mode is equal to current mode
    if ((GraphicsOutput->Mode->Info->HorizontalResolution == Width) && (GraphicsOutput->Mode->Info->VerticalResolution == Height)) {
        MsgLog(" - already set\n");
        egScreenWidth = GraphicsOutput->Mode->Info->HorizontalResolution;
        egScreenHeight = GraphicsOutput->Mode->Info->VerticalResolution;
        return EFI_SUCCESS;
    }
    
    // iterate through modes and set it if found
    MaxMode = GraphicsOutput->Mode->MaxMode;
    for (Mode = 0; Mode < MaxMode; Mode++) {
        Status = GraphicsOutput->QueryMode(GraphicsOutput, Mode, &SizeOfInfo, &Info);
        if (Status == EFI_SUCCESS) {
            if (Width == Info->HorizontalResolution && Height == Info->VerticalResolution) {
                MsgLog(" - setting Mode %d\n", Mode);
                //Status = GraphicsOutput->SetMode(GraphicsOutput, Mode);
                Status = GopSetModeAndReconnectTextOut(Mode);
                if (Status == EFI_SUCCESS) {
                    egScreenWidth = Width;
                    egScreenHeight = Height;
                    return EFI_SUCCESS;
                }
            }
        }
    }
    MsgLog(" - not found!\n");
    return EFI_UNSUPPORTED;
}

VOID egInitScreen(IN BOOLEAN SetMaxResolution)
{
    EFI_STATUS Status;
    UINT32 Width, Height, Depth, RefreshRate;
//    CHAR16 *Resolution;

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

    // Wrap GetMode and SetMode
    if (ConsoleControl != NULL && (ConsoleControlGetMode == NULL || ConsoleControlSetMode == NULL)) {
        ConsoleControlGetMode = ConsoleControl->GetMode;
        ConsoleControlSetMode = ConsoleControl->SetMode;
        ConsoleControl->GetMode = egConsoleControlGetMode;
        ConsoleControl->SetMode = egConsoleControlSetMode;
    }
 
    // if it not the first run, just restore resolution   
    if (egScreenWidth  != 0 && egScreenHeight != 0) {
 //       Resolution = PoolPrint(L"%dx%d",egScreenWidth,egScreenHeight);
      XStringW Resolution = SWPrintf("%llux%llu", egScreenWidth, egScreenHeight);
//        if (Resolution) { //no sense
            Status = egSetScreenResolution(Resolution.wc_str());
 //           FreePool(Resolution);
            if (!EFI_ERROR(Status)) {
                return;
            }
//        }
    }

    egDumpSetConsoleVideoModes();

    // get screen size
    egHasGraphics = FALSE;
    if (GraphicsOutput != NULL) {
 //       egDumpGOPVideoModes();
        if (GlobalConfig.ScreenResolution != NULL) {
            if (EFI_ERROR(egSetScreenResolution(GlobalConfig.ScreenResolution))) {
                if (SetMaxResolution) {
                    egSetMaxResolution();
                }
            }
        } else {
            if (SetMaxResolution) {
                egSetMaxResolution();
            }
        }
        egScreenWidth = GraphicsOutput->Mode->Info->HorizontalResolution;
        egScreenHeight = GraphicsOutput->Mode->Info->VerticalResolution;
        egHasGraphics = TRUE;
    } 
    //is there anybody ever see UGA protocol???
    else if (UgaDraw != NULL) {
      //MsgLog("you are lucky guy having UGA, inform please projectosx!\n");
        Status = UgaDraw->GetMode(UgaDraw, &Width, &Height, &Depth, &RefreshRate);
        if (EFI_ERROR(Status)) {
            UgaDraw = NULL;   // graphics not available
        } else {
            egScreenWidth  = Width;
            egScreenHeight = Height;
            egHasGraphics = TRUE;
        }
    }
}

VOID egGetScreenSize(OUT INTN *ScreenWidth, OUT INTN *ScreenHeight)
{
    if (ScreenWidth != NULL)
        *ScreenWidth = egScreenWidth;
    if (ScreenHeight != NULL)
        *ScreenHeight = egScreenHeight;
}

CONST CHAR16 * egScreenDescription(VOID)
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
        // Some UEFI bioses may cause resolution switch when switching to Text Mode via the ConsoleControl->SetMode command
        // EFI applications wishing to use text, call the ConsoleControl->GetMode() command, and depending on its result may call ConsoleControl->SetMode().
        // To avoid the resolution switch, when we set text mode, we can make ConsoleControl->GetMode report that text mode is enabled.

        // ConsoleControl->SetMode should not be needed on UEFI 2.x to switch to text, but some firmwares seem to block text out if it is not given.
        // We know it blocks text out on HPQ UEFI (HP ProBook for example - reported by dmazar), Apple firmwares with UGA, and some VMs.
        // So, it may be better considering to do this only with firmware vendors where the bug was observed (currently it is known to exist on some AMI firmwares).
        //if (GraphicsOutput != NULL && StrCmp(gST->FirmwareVendor, L"American Megatrends") == 0) {
        if (GraphicsOutput != NULL && StrCmp(gST->FirmwareVendor, L"HPQ") != 0 &&
          StrCmp(gST->FirmwareVendor, L"VMware, Inc.") != 0 &&
          StrCmp(gST->FirmwareVendor, L"INSYDE Corp.") != 0) {
            if (!Enable) {
                // Don't allow switching to text mode, but report that we are in text mode when queried
                CurrentForcedConsoleMode = EfiConsoleControlScreenText;
                IgnoreConsoleSetMode = TRUE;
                return;
            }
            // Allow mode switching to work normally again
            IgnoreConsoleSetMode = FALSE;
        }

        ConsoleControl->GetMode(ConsoleControl, &CurrentMode, NULL, NULL);

        
        NewMode = Enable ? EfiConsoleControlScreenGraphics : EfiConsoleControlScreenText;

        if (CurrentMode != NewMode) {
            ConsoleControl->SetMode(ConsoleControl, NewMode);
        }
    }
}

//
// Drawing to the screen
//

VOID egClearScreen(IN EG_PIXEL *Color)
{
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL FillColor;
    
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
        GraphicsOutput->Blt(GraphicsOutput, &FillColor, EfiBltVideoFill,
                            0, 0, 0, 0, egScreenWidth, egScreenHeight, 0);
    } else if (UgaDraw != NULL) {
        UgaDraw->Blt(UgaDraw, (EFI_UGA_PIXEL*)&FillColor, EfiUgaVideoFill,
                     0, 0, 0, 0, egScreenWidth, egScreenHeight, 0);
    }
}
    
VOID egDrawImageArea(IN EG_IMAGE *Image,
                     IN INTN AreaPosX, IN INTN AreaPosY,
                     IN INTN AreaWidth, IN INTN AreaHeight,
                     IN INTN ScreenPosX, IN INTN ScreenPosY)
{
  if (!egHasGraphics || !Image) return;
  
  if (ScreenPosX < 0 || ScreenPosX >= UGAWidth || ScreenPosY < 0 || ScreenPosY >= UGAHeight) {
    // This is outside of screen area
    return;
  }
  
  if (AreaWidth == 0) {
    AreaWidth = Image->Width;
  }
  
  if (AreaHeight == 0) {
    AreaHeight = Image->Height;
  }
  
  if ((AreaPosX != 0) || (AreaPosY != 0)) {
    egRestrictImageArea(Image, AreaPosX, AreaPosY, &AreaWidth, &AreaHeight);
    if (AreaWidth == 0)
      return;
  }
  
//   if (Image->HasAlpha) { // It shouldn't harm Blt
//     //Image->HasAlpha = FALSE;
//     egSetPlane(PLPTR(Image, a), 255, Image->Width * Image->Height);
//   }
  
  if (ScreenPosX + AreaWidth > UGAWidth)
  {
    AreaWidth = UGAWidth - ScreenPosX;
  }
  if (ScreenPosY + AreaHeight > UGAHeight)
  {
    AreaHeight = UGAHeight - ScreenPosY;
  }
  
  if (GraphicsOutput != NULL) {
    GraphicsOutput->Blt(GraphicsOutput, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)Image->PixelData,
                        EfiBltBufferToVideo,
                        (UINTN)AreaPosX, (UINTN)AreaPosY, (UINTN)ScreenPosX, (UINTN)ScreenPosY,
                        (UINTN)AreaWidth, (UINTN)AreaHeight, (UINTN)Image->Width * 4);
  } else if (UgaDraw != NULL) {
    UgaDraw->Blt(UgaDraw, (EFI_UGA_PIXEL *)Image->PixelData, EfiUgaBltBufferToVideo,
                 (UINTN)AreaPosX, (UINTN)AreaPosY, (UINTN)ScreenPosX, (UINTN)ScreenPosY,
                 (UINTN)AreaWidth, (UINTN)AreaHeight, (UINTN)Image->Width * 4);
  }
}
// Blt(this, Buffer, mode, srcX, srcY, destX, destY, w, h, deltaSrc);
VOID egTakeImage(IN EG_IMAGE *Image, INTN ScreenPosX, INTN ScreenPosY,
                 IN INTN AreaWidth, IN INTN AreaHeight)
{
//  if (GraphicsOutput != NULL) {
    if (ScreenPosX + AreaWidth > UGAWidth)
    {
      AreaWidth = UGAWidth - ScreenPosX;
    }
    if (ScreenPosY + AreaHeight > UGAHeight)
    {
      AreaHeight = UGAHeight - ScreenPosY;
    }
    
    if (GraphicsOutput != NULL) {
      GraphicsOutput->Blt(GraphicsOutput,
                          (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)Image->PixelData,
                          EfiBltVideoToBltBuffer,
                          ScreenPosX,
                          ScreenPosY,
                          0, 0, AreaWidth, AreaHeight, (UINTN)Image->Width * 4);
    } else if (UgaDraw != NULL) {
      UgaDraw->Blt(UgaDraw,
                   (EFI_UGA_PIXEL *)Image->PixelData,
                   EfiUgaVideoToBltBuffer,
                   ScreenPosX,
                   ScreenPosY,
                   0, 0, AreaWidth, AreaHeight, (UINTN)Image->Width * 4);
    }

//  }
}

//
// Make a screenshot
//
//CONST CHAR8 ScreenShotName[] = "EFI\\CLOVER\\misc\\screenshot";
EFI_STATUS egScreenShot(VOID)
{
  EFI_STATUS      Status = EFI_NOT_READY;
  //take screen
  XImage Screen(egScreenWidth, egScreenHeight);
	MsgLog("Make screenshot W=%llu H=%llu\n", egScreenWidth, egScreenHeight);
  Screen.GetArea(0, 0, egScreenWidth, egScreenHeight);
  //convert to PNG
  UINT8           *FileData = NULL;
  UINTN           FileDataLength = 0U;
  Status = Screen.ToPNG(&FileData, FileDataLength);
  if (EFI_ERROR(Status)) {
    if (FileData != NULL) {
      FreePool(FileData);
    }
    return Status;
  }
  if (!FileData) {
    return EFI_NOT_READY;
  }
  //save file with a first unoccupied name
//  XStringW CommonName(L"EFI\\CLOVER\\misc\\screenshot"_XSW);
  for (UINTN Index = 0; Index < 60; Index++) {
//    ScreenshotName = PoolPrint(L"%a%d.png", ScreenShotName, Index);
    XStringW Name = SWPrintf("EFI\\CLOVER\\misc\\screenshot%lld.png", Index);
    if (!FileExists(SelfRootDir, Name.wc_str())) {
      Status = egSaveFile(SelfRootDir, Name.wc_str(), FileData, FileDataLength);
      if (!EFI_ERROR(Status)) {
        break;
      }
    }
  }
  FreePool(FileData);
  return Status;
}

//
// Sets mode via GOP protocol, and reconnects simple text out drivers
//

static EFI_STATUS GopSetModeAndReconnectTextOut(IN UINT32 ModeNumber)
{
    UINTN       HandleCount = 0;
//    UINTN       Index;
    EFI_HANDLE  *HandleBuffer = NULL;
    EFI_STATUS  Status;

    if (GraphicsOutput == NULL) {
        return EFI_UNSUPPORTED;
    }

    Status = GraphicsOutput->SetMode(GraphicsOutput, ModeNumber);
    MsgLog("Video mode change to mode #%d: %s\n", ModeNumber, strerror(Status));

    if (gFirmwareClover && !EFI_ERROR (Status)) { 
        // When we change mode on GOP, we need to reconnect the drivers which produce simple text out
        // Otherwise, they won't produce text based on the new resolution
        Status = gBS->LocateHandleBuffer (
            ByProtocol,
            &gEfiSimpleTextOutProtocolGuid,
            NULL,
            &HandleCount,
            &HandleBuffer
            );
        if (!EFI_ERROR (Status)) {
            for (UINTN Index = 0; Index < HandleCount; Index++) {
                gBS->DisconnectController (HandleBuffer[Index], NULL, NULL);
            }
            for (UINTN Index = 0; Index < HandleCount; Index++) {
                gBS->ConnectController (HandleBuffer[Index], NULL, NULL, TRUE);
            }
            if (HandleBuffer != NULL) {
                FreePool (HandleBuffer);
            }
            egDumpSetConsoleVideoModes();
        }
        // return value is according to whether SetMode succeeded
        Status = EFI_SUCCESS;
    } 

    return Status;
}

/* EOF */
