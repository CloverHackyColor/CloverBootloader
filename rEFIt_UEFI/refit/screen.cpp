/*
 * refit/screen.c
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

#include "../Platform/Platform.h"
#include "screen.h"
#include "../libeg/libegint.h" // included Platform.h
#include "../libeg/XTheme.h"

#ifndef DEBUG_ALL
#define DEBUG_SCR 1
#else
#define DEBUG_SCR DEBUG_ALL
#endif

#if DEBUG_SCR == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_SCR, __VA_ARGS__)	
#endif

// Console defines and variables

UINTN ConWidth;
UINTN ConHeight;
CHAR16 *BlankLine = NULL;
INTN BanHeight = 0;

static VOID SwitchToText(IN BOOLEAN CursorEnabled);
static VOID SwitchToGraphics(VOID);
static VOID DrawScreenHeader(IN CONST CHAR16 *Title);
static VOID UpdateConsoleVars(VOID);
static INTN ConvertEdgeAndPercentageToPixelPosition(INTN Edge, INTN DesiredPercentageFromEdge, INTN ImageDimension, INTN ScreenDimension);
INTN CalculateNudgePosition(INTN Position, INTN NudgeValue, INTN ImageDimension, INTN ScreenDimension);
//INTN RecalculateImageOffset(INTN AnimDimension, INTN ValueToScale, INTN ScreenDimensionToFit, INTN ThemeDesignDimension);
static BOOLEAN IsImageWithinScreenLimits(INTN Value, INTN ImageDimension, INTN ScreenDimension);
static INTN RepositionFixedByCenter(INTN Value, INTN ScreenDimension, INTN DesignScreenDimension);
static INTN RepositionRelativeByGapsOnEdges(INTN Value, INTN ImageDimension, INTN ScreenDimension, INTN DesignScreenDimension);
INTN HybridRepositioning(INTN Edge, INTN Value, INTN ImageDimension, INTN ScreenDimension, INTN DesignScreenDimension);

EG_IMAGE * LoadSvgFrame(INTN i);

// UGA defines and variables

INTN   UGAWidth;
INTN   UGAHeight;
BOOLEAN AllowGraphicsMode;

EG_RECT  BannerPlace; // default ctor called, so it's zero

EG_PIXEL StdBackgroundPixel   = { 0xbf, 0xbf, 0xbf, 0xff};
EG_PIXEL MenuBackgroundPixel  = { 0x00, 0x00, 0x00, 0x00};
EG_PIXEL InputBackgroundPixel = { 0xcf, 0xcf, 0xcf, 0x80};
EG_PIXEL BlueBackgroundPixel  = { 0x7f, 0x0f, 0x0f, 0xff};
EG_PIXEL EmbeddedBackgroundPixel  = { 0xaa, 0xaa, 0xaa, 0xff};
EG_PIXEL DarkSelectionPixel   = { 66, 66, 66, 0xff};
EG_PIXEL DarkEmbeddedBackgroundPixel  = { 0x33, 0x33, 0x33, 0xff};
EG_PIXEL WhitePixel  = { 0xff, 0xff, 0xff, 0xff};
EG_PIXEL BlackPixel  = { 0x00, 0x00, 0x00, 0xff};

EG_IMAGE *BackgroundImage = NULL;
EG_IMAGE *Banner = NULL;
EG_IMAGE *BigBack = NULL;

static BOOLEAN GraphicsScreenDirty;

// general defines and variables

static BOOLEAN haveError = FALSE;

//
// Screen initialization and switching
//

VOID InitScreen(IN BOOLEAN SetMaxResolution)
{
	//DbgHeader("InitScreen");
    // initialize libeg
    egInitScreen(SetMaxResolution);
    
    if (egHasGraphicsMode()) {
        egGetScreenSize(&UGAWidth, &UGAHeight);
        AllowGraphicsMode = TRUE;
    } else {
        AllowGraphicsMode = FALSE;
		//egSetGraphicsModeEnabled(FALSE);   // just to be sure we are in text mode
    }
	
    GraphicsScreenDirty = TRUE;
    
    // disable cursor
	gST->ConOut->EnableCursor(gST->ConOut, FALSE);
    
    UpdateConsoleVars();

    // show the banner (even when in graphics mode)
	//DrawScreenHeader(L"Initializing...");
}

VOID SetupScreen(VOID)
{
    if (GlobalConfig.TextOnly) {
        // switch to text mode if requested
        AllowGraphicsMode = FALSE;
        SwitchToText(FALSE);
    } else if (AllowGraphicsMode) {
        // clear screen and show banner
        // (now we know we'll stay in graphics mode)
        SwitchToGraphics();
		//BltClearScreen(TRUE);
    }
}

static VOID SwitchToText(IN BOOLEAN CursorEnabled)
{
    egSetGraphicsModeEnabled(FALSE);
	gST->ConOut->EnableCursor(gST->ConOut, CursorEnabled);
}

static VOID SwitchToGraphics(VOID)
{
    if (AllowGraphicsMode && !egIsGraphicsModeEnabled()) {
      InitScreen(FALSE);
        egSetGraphicsModeEnabled(TRUE);
        GraphicsScreenDirty = TRUE;
    }
}

//
// Screen control for running tools
//
VOID BeginTextScreen(IN CONST CHAR16 *Title)
{
    DrawScreenHeader(Title);
    SwitchToText(FALSE);
    
    // reset error flag
    haveError = FALSE;
}

VOID FinishTextScreen(IN BOOLEAN WaitAlways)
{
    if (haveError || WaitAlways) {
        SwitchToText(FALSE);
 //       PauseForKey(L"FinishTextScreen");
    }
    
    // reset error flag
    haveError = FALSE;
}

VOID BeginExternalScreen(IN BOOLEAN UseGraphicsMode, IN CONST CHAR16 *Title)
{
	if (!AllowGraphicsMode) {
        UseGraphicsMode = FALSE;
	}
    
    if (UseGraphicsMode) {
        SwitchToGraphics();
		//BltClearScreen(FALSE);
    }
    
    // show the header
	//DrawScreenHeader(Title);
    
	if (!UseGraphicsMode) {
        SwitchToText(TRUE);
	}
    
    // reset error flag
    haveError = FALSE;
}

VOID FinishExternalScreen(VOID)
{
    // make sure we clean up later
    GraphicsScreenDirty = TRUE;
    
    if (haveError) {
        // leave error messages on screen in case of error,
        // wait for a key press, and then switch
        PauseForKey(L"was error, press any key\n");
        SwitchToText(FALSE);
    }
    
    // reset error flag
    haveError = FALSE;
}

VOID TerminateScreen(VOID)
{
    // clear text screen
	gST->ConOut->SetAttribute(gST->ConOut, ATTR_BANNER);
	gST->ConOut->ClearScreen(gST->ConOut);
    
    // enable cursor
	gST->ConOut->EnableCursor(gST->ConOut, TRUE);
}

static VOID DrawScreenHeader(IN CONST CHAR16 *Title)
{
  UINTN i;
	CHAR16* BannerLine = (__typeof__(BannerLine))AllocatePool((ConWidth + 1) * sizeof(CHAR16));
  BannerLine[ConWidth] = 0;

  // clear to black background
	//gST->ConOut->SetAttribute(gST->ConOut, ATTR_BASIC);
  //gST->ConOut->ClearScreen (gST->ConOut);

  // paint header background
  gST->ConOut->SetAttribute(gST->ConOut, ATTR_BANNER);
	
	for (i = 1; i < ConWidth-1; i++) {
    BannerLine[i] = BOXDRAW_HORIZONTAL;
	}
	
	BannerLine[0] = BOXDRAW_DOWN_RIGHT;
	BannerLine[ConWidth-1] = BOXDRAW_DOWN_LEFT;
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 0);
	Print(BannerLine);

	for (i = 1; i < ConWidth-1; i++)
    BannerLine[i] = ' ';
	BannerLine[0] = BOXDRAW_VERTICAL;
	BannerLine[ConWidth-1] = BOXDRAW_VERTICAL;
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 1);
	Print(BannerLine);

	for (i = 1; i < ConWidth-1; i++)
    BannerLine[i] = BOXDRAW_HORIZONTAL;
 	BannerLine[0] = BOXDRAW_UP_RIGHT;
	BannerLine[ConWidth-1] = BOXDRAW_UP_LEFT;
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 2);
	Print(BannerLine);

	FreePool(BannerLine);

  // print header text
  gST->ConOut->SetCursorPosition (gST->ConOut, 3, 1);
  Print(L"Clover rev %s - %s", gFirmwareRevision, Title);

  // reposition cursor
  gST->ConOut->SetAttribute (gST->ConOut, ATTR_BASIC);
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 4);
}

//
// Keyboard input
//

BOOLEAN ReadAllKeyStrokes(VOID)
{
    BOOLEAN       GotKeyStrokes;
    EFI_STATUS    Status;
    EFI_INPUT_KEY key;
    
    GotKeyStrokes = FALSE;
    for (;;) {
        Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &key);
        if (Status == EFI_SUCCESS) {
            GotKeyStrokes = TRUE;
            continue;
        }
        break;
    }
    return GotKeyStrokes;
}

VOID PauseForKey(CONST CHAR16* msg)
{
#if REFIT_DEBUG > 0  
    UINTN index;
    if (msg) {
      Print(L"\n %s", msg);
    }
    Print(L"\n* Hit any key to continue *");
    
    if (ReadAllKeyStrokes()) {  // remove buffered key strokes
        gBS->Stall(5000000);     // 5 seconds delay
        ReadAllKeyStrokes();    // empty the buffer again
    }
    
    gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &index);
    ReadAllKeyStrokes();        // empty the buffer to protect the menu
    
    Print(L"\n");
#endif
}

#if REFIT_DEBUG > 0
VOID DebugPause(VOID)
{
    // show console and wait for key
    SwitchToText(FALSE);
    PauseForKey(L"");
    
    // reset error flag
    haveError = FALSE;
}
#endif

VOID EndlessIdleLoop(VOID)
{
    UINTN index;
    
    for (;;) {
        ReadAllKeyStrokes();
        gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &index);
    }
}

//
// Error handling
//
/*
VOID
StatusToString (
				OUT CHAR16      *Buffer,
				EFI_STATUS      Status
				)
{
	UnicodeSPrint(Buffer, 64, L"EFI Error %r", Status);
}*/


BOOLEAN CheckFatalError(IN EFI_STATUS Status, IN CONST CHAR16 *where)
{
//    CHAR16 ErrorName[64];
    
    if (!EFI_ERROR(Status))
        return FALSE;
    
//    StatusToString(ErrorName, Status);
    gST->ConOut->SetAttribute (gST->ConOut, ATTR_ERROR);
    Print(L"Fatal Error: %r %s\n", Status, where);
    gST->ConOut->SetAttribute (gST->ConOut, ATTR_BASIC);
    haveError = TRUE;
    
    //gBS->Exit(ImageHandle, ExitStatus, ExitDataSize, ExitData);
    
    return TRUE;
}

BOOLEAN CheckError(IN EFI_STATUS Status, IN CONST CHAR16 *where)
{
//    CHAR16 ErrorName[64];
    
    if (!EFI_ERROR(Status))
        return FALSE;
    
//    StatusToString(ErrorName, Status);
    gST->ConOut->SetAttribute (gST->ConOut, ATTR_ERROR);
    Print(L"Error: %r %s\n", Status, where);
    gST->ConOut->SetAttribute (gST->ConOut, ATTR_BASIC);
    haveError = TRUE;
    
    return TRUE;
}

//
// Graphics functions
//

VOID SwitchToGraphicsAndClear(VOID) //called from MENU_FUNCTION_INIT
{
  SwitchToGraphics();
#if USE_XTHEME
  ThemeX.ClearScreen();
#else
	if (GraphicsScreenDirty) { //Invented in rEFIt 15 years ago
    BltClearScreen();
	}
#endif
}

/*
typedef struct {
  INTN     XPos;
  INTN     YPos;
  INTN     Width;
  INTN     Height;
} EG_RECT;
 // moreover it is class EG_RECT;
 //same as EgRect but INTN <-> UINTN
*/


VOID BltClearScreen() //ShowBanner always TRUE. Called from line 400
{
  EG_PIXEL *p1;
  INTN i, j, x, x1, x2, y, y1, y2;
  if (BanHeight < 2) {
    BanHeight = ((UGAHeight - (int)(LAYOUT_TOTAL_HEIGHT * GlobalConfig.Scale)) >> 1);
    //+ (int)(LAYOUT_TOTAL_HEIGHT * GlobalConfig.Scale); //LAYOUT_TOTAL_HEIGHT=376
  }

  if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_BANNER)) {
    // Banner is used in this theme
    if (!Banner) {
      // Banner is not loaded yet
      if (IsEmbeddedTheme()) {
        // embedded theme - use text as banner
   //     Banner = egCreateImage(7 * StrLen(L"CLOVER"), 32, TRUE);
   //     egFillImage(Banner, &MenuBackgroundPixel);
   //     egRenderText(L"CLOVER", Banner, 0, 0, 0xFFFF);
   //     CopyMem(&BlueBackgroundPixel, &StdBackgroundPixel, sizeof(EG_PIXEL));
   //     DebugLog(1, "Text <%s> rendered\n", L"Clover");
        Banner = BuiltinIcon(BUILTIN_ICON_BANNER);
        if (GlobalConfig.DarkEmbedded) {
          CopyMem(&BlueBackgroundPixel, &DarkEmbeddedBackgroundPixel, sizeof(EG_PIXEL));
        } else {
          CopyMem(&BlueBackgroundPixel, &StdBackgroundPixel, sizeof(EG_PIXEL));
        }
      } else  {
        Banner = egLoadImage(ThemeDir, GlobalConfig.BannerFileName, FALSE);
        if (Banner) {
          // Banner was changed, so copy into BlueBackgroundBixel first pixel of banner
          CopyMem(&BlueBackgroundPixel, &Banner->PixelData[0], sizeof(EG_PIXEL));
        } else {
          DBG("banner file not read use embedded\n");
          Banner = BuiltinIcon(BUILTIN_ICON_BANNER);
        }
      }
    }
    if (Banner) {
      // Banner was loaded, so calculate its size and position
      BannerPlace.Width = Banner->Width;
      BannerPlace.Height = (BanHeight >= Banner->Height) ? (INTN)Banner->Height : BanHeight;
 //     DBG("banner width-height [%d,%d]\n", BannerPlace.Width, BannerPlace.Height);
 //     DBG("global banner pos [%d,%d]\n", GlobalConfig.BannerPosX, GlobalConfig.BannerPosY);
      if (GlobalConfig.TypeSVG) {
        BannerPlace.XPos = GlobalConfig.BannerPosX;
        BannerPlace.YPos = GlobalConfig.BannerPosY;
      } else {
        // Check if new style placement value was used for banner in theme.plist

        if ((GlobalConfig.BannerPosX >=0 && GlobalConfig.BannerPosX <=1000) && (GlobalConfig.BannerPosY >=0 && GlobalConfig.BannerPosY <=1000)) {
          // Check if screen size being used is different from theme origination size.
          // If yes, then recalculate the placement % value.
          // This is necessary because screen can be a different size, but banner is not scaled.
          BannerPlace.XPos = HybridRepositioning(GlobalConfig.BannerEdgeHorizontal, GlobalConfig.BannerPosX, BannerPlace.Width,  UGAWidth,  GlobalConfig.ThemeDesignWidth );
          BannerPlace.YPos = HybridRepositioning(GlobalConfig.BannerEdgeVertical,   GlobalConfig.BannerPosY, BannerPlace.Height, UGAHeight, GlobalConfig.ThemeDesignHeight);
          // Check if banner is required to be nudged.
          BannerPlace.XPos = CalculateNudgePosition(BannerPlace.XPos, GlobalConfig.BannerNudgeX, Banner->Width,  UGAWidth);
          BannerPlace.YPos = CalculateNudgePosition(BannerPlace.YPos, GlobalConfig.BannerNudgeY, Banner->Height, UGAHeight);
 //         DBG("banner position new style\n");
        } else {
          // Use rEFIt default (no placement values speicifed)
          BannerPlace.XPos = (UGAWidth - Banner->Width) >> 1;
          BannerPlace.YPos = (BanHeight >= Banner->Height) ? (BanHeight - Banner->Height) : 0;
  //        DBG("banner position old style\n");
        }
      }
    }
  }

//  DBG("Banner position [%d,%d]\n",  BannerPlace.XPos, BannerPlace.YPos);

  if (!Banner || (GlobalConfig.HideUIFlags & HIDEUI_FLAG_BANNER) || 
      !IsImageWithinScreenLimits(BannerPlace.XPos, BannerPlace.Width, UGAWidth) || 
      !IsImageWithinScreenLimits(BannerPlace.YPos, BannerPlace.Height, UGAHeight)) {
    // Banner is disabled or it cannot be used, apply defaults for placement
    if (Banner) {
      FreePool(Banner);
      Banner = NULL;
    }
    BannerPlace.XPos = 0;
    BannerPlace.YPos = 0;
    BannerPlace.Width = UGAWidth;
    BannerPlace.Height = BanHeight;
  }
  
  // Load Background and scale
  if (!BigBack && (GlobalConfig.BackgroundName != NULL)) {
    BigBack = egLoadImage(ThemeDir, GlobalConfig.BackgroundName, FALSE);
  }
  
  if (BackgroundImage != NULL && (BackgroundImage->Width != UGAWidth || BackgroundImage->Height != UGAHeight)) {
    // Resolution changed
    egFreeImage(BackgroundImage);
    BackgroundImage = NULL;
  }
  
  if (BackgroundImage == NULL) {
/*    DBG("BltClearScreen(%c): calling egCreateFilledImage UGAWidth %ld, UGAHeight %ld, BlueBackgroundPixel %02x%02x%02x%02x\n",
        ShowBanner?'Y':'N', UGAWidth, UGAHeight,
        BlueBackgroundPixel.r, BlueBackgroundPixel.g, BlueBackgroundPixel.b, BlueBackgroundPixel.a); */
    BackgroundImage = egCreateFilledImage(UGAWidth, UGAHeight, FALSE, &BlueBackgroundPixel);
  }
  
  if (BigBack != NULL) {
    switch (GlobalConfig.BackgroundScale) {
      case imScale:
        ScaleImage(BackgroundImage, BigBack);
        break;
      case imCrop:
        x = UGAWidth - BigBack->Width;
        if (x >= 0) {
          x1 = x >> 1;
          x2 = 0;
          x = BigBack->Width;
        } else {
          x1 = 0;
          x2 = (-x) >> 1;
          x = UGAWidth;
        }
        y = UGAHeight - BigBack->Height;
        if (y >= 0) {
          y1 = y >> 1;
          y2 = 0;
          y = BigBack->Height;
        } else {
          y1 = 0;
          y2 = (-y) >> 1;
          y = UGAHeight;
        }
        egRawCopy(BackgroundImage->PixelData + y1 * UGAWidth + x1,
                  BigBack->PixelData + y2 * BigBack->Width + x2,
                  x, y, UGAWidth, BigBack->Width);
        break;
      case imTile:
        x = (BigBack->Width * ((UGAWidth - 1) / BigBack->Width + 1) - UGAWidth) >> 1;
        y = (BigBack->Height * ((UGAHeight - 1) / BigBack->Height + 1) - UGAHeight) >> 1;
        p1 = BackgroundImage->PixelData;
        for (j = 0; j < UGAHeight; j++) {
          y2 = ((j + y) % BigBack->Height) * BigBack->Width;
          for (i = 0; i < UGAWidth; i++) {
            *p1++ = BigBack->PixelData[y2 + ((i + x) % BigBack->Width)];
          }
        }
        break;
      case imNone:
      default:
        // already scaled
        break;
    }
  }
  
  // Draw background
  if (BackgroundImage) {
/*    DBG("BltClearScreen(%c): calling BltImage BackgroundImage %p\n",
        ShowBanner?'Y':'N', BackgroundImage); */
    BltImage(BackgroundImage, 0, 0); //if NULL then do nothing
  } else {
/*    DBG("BltClearScreen(%c): calling egClearScreen StdBackgroundPixel %02x%02x%02x%02x\n",
        ShowBanner?'Y':'N', StdBackgroundPixel.r, StdBackgroundPixel.g, StdBackgroundPixel.b, StdBackgroundPixel.a); */
    egClearScreen(&StdBackgroundPixel);
  }
  
  // Draw banner
  if (Banner) {
    BltImageAlpha(Banner, BannerPlace.XPos, BannerPlace.YPos, &MenuBackgroundPixel, 16);
  }
//what is the idea for the conversion?
  InputBackgroundPixel.r = (MenuBackgroundPixel.r + 0) & 0xFF;
  InputBackgroundPixel.g = (MenuBackgroundPixel.g + 0) & 0xFF;
  InputBackgroundPixel.b = (MenuBackgroundPixel.b + 0) & 0xFF;
  InputBackgroundPixel.a = (MenuBackgroundPixel.a + 0) & 0xFF;
  GraphicsScreenDirty = FALSE;
}

VOID BltImage(IN EG_IMAGE *Image, IN INTN XPos, IN INTN YPos)
{
  if (!Image) {
    return;
  }
  egDrawImageArea(Image, 0, 0, 0, 0, XPos, YPos);
  GraphicsScreenDirty = TRUE;
}

VOID BltImageAlpha(IN EG_IMAGE *Image, IN INTN XPos, IN INTN YPos, IN EG_PIXEL *BackgroundPixel, INTN Scale)
{
  EG_IMAGE *CompImage;
  EG_IMAGE *NewImage = NULL;
  INTN Width = Scale << 3;
  INTN Height = Width;

  GraphicsScreenDirty = TRUE;
  if (Image) {
    NewImage = egCopyScaledImage(Image, Scale); //will be Scale/16
    Width = NewImage->Width;
    Height = NewImage->Height;
  }
//  DBG("w=%d, h=%d\n", Width, Height);
  // compose on background
  CompImage = egCreateFilledImage(Width, Height, (BackgroundImage != NULL), BackgroundPixel);
  egComposeImage(CompImage, NewImage, 0, 0);
  if (NewImage) {
    egFreeImage(NewImage);
  }
  if (!BackgroundImage) {
    egDrawImageArea(CompImage, 0, 0, 0, 0, XPos, YPos);
    egFreeImage(CompImage);
    return;
  }
  NewImage = egCreateImage(Width, Height, FALSE);
  if (!NewImage) return;
//  DBG("draw on background\n");
  egRawCopy(NewImage->PixelData,
            BackgroundImage->PixelData + YPos * BackgroundImage->Width + XPos,
            Width, Height,
            Width,
            BackgroundImage->Width);
  egComposeImage(NewImage, CompImage, 0, 0);
  egFreeImage(CompImage);

  // blit to screen and clean up
  egDrawImageArea(NewImage, 0, 0, 0, 0, XPos, YPos);
  egFreeImage(NewImage);
}
//not used
/*
VOID BltImageComposite(IN EG_IMAGE *BaseImage, IN EG_IMAGE *TopImage, IN INTN XPos, IN INTN YPos)
{
  INTN TotalWidth, TotalHeight, CompWidth, CompHeight, OffsetX, OffsetY;
  EG_IMAGE *CompImage;

  if (!BaseImage || !TopImage) {
    return;
  }

  // initialize buffer with base image
  CompImage = egCopyImage(BaseImage);
  TotalWidth  = BaseImage->Width;
  TotalHeight = BaseImage->Height;

  // place the top image
  CompWidth = TopImage->Width;
  if (CompWidth > TotalWidth)
    CompWidth = TotalWidth;
  OffsetX = (TotalWidth - CompWidth) >> 1;
  CompHeight = TopImage->Height;
  if (CompHeight > TotalHeight)
    CompHeight = TotalHeight;
  OffsetY = (TotalHeight - CompHeight) >> 1;
  egComposeImage(CompImage, TopImage, OffsetX, OffsetY);

  // blit to screen and clean up
  //    egDrawImageArea(CompImage, 0, 0, TotalWidth, TotalHeight, XPos, YPos);
  BltImageAlpha(CompImage, XPos, YPos, &MenuBackgroundPixel, 16);
  egFreeImage(CompImage);
  GraphicsScreenDirty = TRUE;
}
*/
/*
  --------------------------------------------------------------------
  Pos                           : Bottom    -> Mid        -> Top
  --------------------------------------------------------------------
   GlobalConfig.SelectionOnTop  : MainImage -> Badge      -> Selection
  !GlobalConfig.SelectionOnTop  : Selection -> MainImage  -> Badge

 GlobalConfig.SelectionOnTop
  BaseImage = MainImage, TopImage = Selection
*/

#if USE_XTHEME
/*
// TopImage = SelectionImages[index]
// The procedure will be replaced by
if(SelectionOnTop) {
  BaseImage.Draw(XPos, YPos, Scale/16.f);
  BadgeImage.Draw(XPos, YPos, Scale/16.f);
  TopImage.Draw(XPos, YPos, Scale/16.f);
} else {
  TopImage.Draw(XPos, YPos, Scale/16.f);
  BaseImage.Draw(XPos, YPos, Scale/16.f);
  BadgeImage.Draw(XPos, YPos, Scale/16.f);
}
 */
#else
VOID BltImageCompositeBadge(IN EG_IMAGE *BaseImage, IN EG_IMAGE *TopImage, IN EG_IMAGE *BadgeImage, IN INTN XPos, IN INTN YPos, INTN Scale)
{
  INTN TotalWidth, TotalHeight, CompWidth, CompHeight, OffsetX, OffsetY, OffsetXTmp, OffsetYTmp;
  BOOLEAN Selected = TRUE;
  EG_IMAGE *CompImage;
  EG_IMAGE *NewBaseImage;
  EG_IMAGE *NewTopImage;
  EG_PIXEL *BackgroundPixel = &EmbeddedBackgroundPixel;
  
  if (!IsEmbeddedTheme()) {
    BackgroundPixel = &MenuBackgroundPixel;
  } else if (GlobalConfig.DarkEmbedded) {
    BackgroundPixel = &DarkEmbeddedBackgroundPixel;
  }

  if (!BaseImage || !TopImage) {
    return;
  }
  if (Scale < 0) {
    Scale = -Scale;
    Selected = FALSE;
  }

  NewBaseImage = egCopyScaledImage(BaseImage, Scale); //will be Scale/16
  TotalWidth = NewBaseImage->Width;  //mainImage sizes if GlobalConfig.SelectionOnTop
  TotalHeight = NewBaseImage->Height;

  NewTopImage = egCopyScaledImage(TopImage, Scale); //will be Scale/16
  CompWidth = NewTopImage->Width;  //selection sizes if GlobalConfig.SelectionOnTop
  CompHeight = NewTopImage->Height;
  CompImage = egCreateFilledImage((CompWidth > TotalWidth)?CompWidth:TotalWidth,
                                    (CompHeight > TotalHeight)?CompHeight:TotalHeight,
                                    TRUE,
                                    BackgroundPixel);
  
  if (!CompImage) {
    DBG("Can't create CompImage\n");
    return;
  }
//  DBG("compose image total=[%d,%d], comp=[%d,%d] at [%d,%d] scale=%d\n", TotalWidth, TotalHeight,
//      CompWidth, CompHeight, XPos, YPos, Scale);
  //to simplify suppose square images
  if (CompWidth < TotalWidth) {
    OffsetX = (TotalWidth - CompWidth) >> 1;
    OffsetY = (TotalHeight - CompHeight) >> 1;
    egComposeImage(CompImage, NewBaseImage, 0, 0);
    if (!GlobalConfig.SelectionOnTop) {
      egComposeImage(CompImage, NewTopImage, OffsetX, OffsetY);
    }
    CompWidth = TotalWidth;
    CompHeight = TotalHeight;
  } else {
    OffsetX = (CompWidth - TotalWidth) >> 1;
    OffsetY = (CompHeight - TotalHeight) >> 1;
    egComposeImage(CompImage, NewBaseImage, OffsetX, OffsetY);
    if (!GlobalConfig.SelectionOnTop) {
      egComposeImage(CompImage, NewTopImage, 0, 0);
    }
  }

  OffsetXTmp = OffsetX;
  OffsetYTmp = OffsetY;

  // place the badge image
  if (BadgeImage != NULL &&
      (BadgeImage->Width + 8) < CompWidth &&
      (BadgeImage->Height + 8) < CompHeight) {

    //blackosx
    // Check for user badge x offset from theme.plist
    if (GlobalConfig.BadgeOffsetX != 0xFFFF) {
      // Check if value is between 0 and ( width of the main icon - width of badge )
      if (GlobalConfig.BadgeOffsetX < 0 || GlobalConfig.BadgeOffsetX > (CompWidth - BadgeImage->Width)) {
        DBG("User offset X %d is out of range\n", GlobalConfig.BadgeOffsetX);
        GlobalConfig.BadgeOffsetX = CompWidth  - 8 - BadgeImage->Width;
        DBG("   corrected to default %d\n", GlobalConfig.BadgeOffsetX);
      }
      OffsetX += GlobalConfig.BadgeOffsetX;
    } else {
      // Set default position
      OffsetX += CompWidth  - 8 - BadgeImage->Width;
    }
    // Check for user badge y offset from theme.plist
    if (GlobalConfig.BadgeOffsetY != 0xFFFF) {
      // Check if value is between 0 and ( height of the main icon - height of badge )
      if (GlobalConfig.BadgeOffsetY < 0 || GlobalConfig.BadgeOffsetY > (CompHeight - BadgeImage->Height)) {
        DBG("User offset Y %d is out of range\n",GlobalConfig.BadgeOffsetY);
        GlobalConfig.BadgeOffsetY = CompHeight - 8 - BadgeImage->Height;
        DBG("   corrected to default %d\n", GlobalConfig.BadgeOffsetY);
      }
      OffsetY += GlobalConfig.BadgeOffsetY;
    } else {
      // Set default position
      OffsetY += CompHeight - 8 - BadgeImage->Height;
    }
    egComposeImage(CompImage, BadgeImage, OffsetX, OffsetY);
  }

  if (GlobalConfig.SelectionOnTop) {
    if (CompWidth < TotalWidth) {
      egComposeImage(CompImage, NewTopImage, OffsetXTmp, OffsetYTmp);
    } else {
      egComposeImage(CompImage, NewTopImage, 0, 0);
    }
  }

  // blit to screen and clean up
//  if (!IsEmbeddedTheme()) { // regular theme
    if (GlobalConfig.NonSelectedGrey && !Selected) {
      BltImageAlpha(CompImage, XPos, YPos, &MenuBackgroundPixel, -16);
    } else {
      BltImageAlpha(CompImage, XPos, YPos, &MenuBackgroundPixel, 16);
    }
/*  } else { // embedded theme - don't use BltImageAlpha as it can't handle refit's built in image
    egDrawImageArea(CompImage, 0, 0, TotalWidth, TotalHeight, XPos, YPos);
  } */
  egFreeImage(CompImage);
  egFreeImage(NewBaseImage);
  egFreeImage(NewTopImage);
  GraphicsScreenDirty = TRUE;
}
#endif

#define MAX_SIZE_ANIME 256

VOID FreeAnime(GUI_ANIME *Anime)
{
   if (Anime) {
     if (Anime->Path) {
       FreePool(Anime->Path);
       Anime->Path = NULL;
     }
     FreePool(Anime);
//     Anime = NULL;
   }
}

/* Replaced for now with Reposition* below
INTN RecalculateImageOffset(INTN AnimDimension, INTN ValueToScale, INTN ScreenDimensionToFit, INTN ThemeDesignDimension)
{
    INTN SuppliedGapDimensionPxDesigned=0;
    INTN OppositeGapDimensionPxDesigned=0;
    INTN OppositeGapPcDesigned=0;
    INTN ScreenDimensionLessAnim=0;
    INTN GapNumTimesLarger=0;
    INTN GapNumFinal=0;
    INTN NewSuppliedGapPx=0;
    INTN NewOppositeGapPx=0;
    INTN ReturnValue=0;
    
    SuppliedGapDimensionPxDesigned = (ThemeDesignDimension * ValueToScale) / 100;
    OppositeGapDimensionPxDesigned = ThemeDesignDimension - (SuppliedGapDimensionPxDesigned + AnimDimension);
    OppositeGapPcDesigned = (OppositeGapDimensionPxDesigned * 100)/ThemeDesignDimension;
    ScreenDimensionLessAnim = (ScreenDimensionToFit - AnimDimension);
    if (ValueToScale > OppositeGapPcDesigned) {
      GapNumTimesLarger = (ValueToScale * 100)/OppositeGapPcDesigned;
      GapNumFinal = GapNumTimesLarger + 100;
      NewOppositeGapPx = (ScreenDimensionLessAnim * 100)/GapNumFinal;
      NewSuppliedGapPx = (NewOppositeGapPx * GapNumTimesLarger)/100;
    } else if (ValueToScale < OppositeGapPcDesigned) {
      GapNumTimesLarger = (OppositeGapPcDesigned * 100)/ValueToScale;
      GapNumFinal = (GapNumTimesLarger + 100);
      NewSuppliedGapPx = (ScreenDimensionLessAnim * 100)/GapNumFinal;
      NewOppositeGapPx = (NewSuppliedGapPx * GapNumTimesLarger)/100;
    } else if (ValueToScale == OppositeGapPcDesigned) {
      NewSuppliedGapPx = (ScreenDimensionLessAnim * 100)/200;
      NewOppositeGapPx = (NewSuppliedGapPx * 100)/100;
    }
    ReturnValue = (NewSuppliedGapPx * 100)/ScreenDimensionToFit;
    
    if (ReturnValue>0 && ReturnValue<100) {
      //DBG("Different screen size being used. Adjusted original anim gap to %d\n",ReturnValue);
      return ReturnValue;
    } else {
      DBG("Different screen size being used. Adjusted value %d invalid. Returning original value %d\n",ReturnValue, ValueToScale);
      return ValueToScale;
    }
}
*/

static INTN ConvertEdgeAndPercentageToPixelPosition(INTN Edge, INTN DesiredPercentageFromEdge, INTN ImageDimension, INTN ScreenDimension)
{
  if (Edge == SCREEN_EDGE_LEFT || Edge == SCREEN_EDGE_TOP) {
      return ((ScreenDimension * DesiredPercentageFromEdge) / 100);
  } else if (Edge == SCREEN_EDGE_RIGHT || Edge == SCREEN_EDGE_BOTTOM) {
      return (ScreenDimension - ((ScreenDimension * DesiredPercentageFromEdge) / 100) - ImageDimension);
  }
  return 0xFFFF; // to indicate that wrong edge was specified.
}

INTN CalculateNudgePosition(INTN Position, INTN NudgeValue, INTN ImageDimension, INTN ScreenDimension)
{
  INTN value=Position;
  
  if ((NudgeValue != INITVALUE) && (NudgeValue != 0) && (NudgeValue >= -32) && (NudgeValue <= 32)) {
    if ((value + NudgeValue >=0) && (value + NudgeValue <= ScreenDimension - ImageDimension)) {
     value += NudgeValue;
    }
  }
  return value;
}

static BOOLEAN IsImageWithinScreenLimits(INTN Value, INTN ImageDimension, INTN ScreenDimension)
{
  return (Value >= 0 && Value + ImageDimension <= ScreenDimension);
}

static INTN RepositionFixedByCenter(INTN Value, INTN ScreenDimension, INTN DesignScreenDimension)
{
  return (Value + ((ScreenDimension - DesignScreenDimension) / 2));
}

static INTN RepositionRelativeByGapsOnEdges(INTN Value, INTN ImageDimension, INTN ScreenDimension, INTN DesignScreenDimension)
{
  return (Value * (ScreenDimension - ImageDimension) / (DesignScreenDimension - ImageDimension));
}

INTN HybridRepositioning(INTN Edge, INTN Value, INTN ImageDimension, INTN ScreenDimension, INTN DesignScreenDimension)
{
  INTN pos, posThemeDesign;
  
  if (DesignScreenDimension == 0xFFFF || ScreenDimension == DesignScreenDimension) {
    // Calculate the horizontal pixel to place the top left corner of the animation - by screen resolution
    pos = ConvertEdgeAndPercentageToPixelPosition(Edge, Value, ImageDimension, ScreenDimension);
  } else {
    // Calculate the horizontal pixel to place the top left corner of the animation - by theme design resolution
    posThemeDesign = ConvertEdgeAndPercentageToPixelPosition(Edge, Value, ImageDimension, DesignScreenDimension);
    // Try repositioning by center first
    pos = RepositionFixedByCenter(posThemeDesign, ScreenDimension, DesignScreenDimension);
    // If out of edges, try repositioning by gaps on edges
    if (!IsImageWithinScreenLimits(pos, ImageDimension, ScreenDimension)) {
      pos = RepositionRelativeByGapsOnEdges(posThemeDesign, ImageDimension, ScreenDimension, DesignScreenDimension);
    }
  }
  return pos;
}

static EG_IMAGE *AnimeImage = NULL;

VOID REFIT_MENU_SCREEN::UpdateAnime()
{
  UINT64      Now;
  INTN        x, y;
  
  //INTN LayoutAnimMoveForMenuX = 0;
  INTN MenuWidth = 50;
  
  if (!AnimeRun || !Film || GlobalConfig.TextOnly) return;
  if (!AnimeImage ||
      (AnimeImage->Width != Film[0]->Width) ||
      (AnimeImage->Height != Film[0]->Height)){
    if (AnimeImage) {
      egFreeImage(AnimeImage);
    }
//    DBG("create new AnimeImage [%d,%d]\n", Film[0]->Width, Film[0]->Height);
    AnimeImage = egCreateImage(Film[0]->Width, Film[0]->Height, TRUE);
  }
//  DBG("anime rect pos=[%d,%d] size=[%d,%d]\n", Place->XPos, Place->YPos,
//      Place->Width, Place->Height);
//  DBG("anime size=[%d,%d]\n", AnimeImage->Width, AnimeImage->Height);
  
  // Retained for legacy themes without new anim placement options.
  x = FilmPlace.XPos + (FilmPlace.Width - AnimeImage->Width) / 2;
  y = FilmPlace.YPos + (FilmPlace.Height - AnimeImage->Height) / 2;
  
  if (!IsImageWithinScreenLimits(x, Film[0]->Width, UGAWidth) || !IsImageWithinScreenLimits(y, Film[0]->Height, UGAHeight)) {
 //   DBG(") This anime can't be displayed\n");
    return;
  }
  
  // Check if the theme.plist setting for allowing an anim to be moved horizontally in the quest 
  // to avoid overlapping the menu text on menu pages at lower resolutions is set.
  if ((ID > 1) && (LayoutAnimMoveForMenuX != 0)) { // these screens have text menus which the anim may interfere with.
    MenuWidth = (INTN)(TEXT_XMARGIN * 2 + (50 * GlobalConfig.CharWidth * GlobalConfig.Scale)); // taken from menu.c
    if ((x + Film[0]->Width) > (UGAWidth - MenuWidth) >> 1) {
      if ((x + LayoutAnimMoveForMenuX >= 0) || (UGAWidth-(x + LayoutAnimMoveForMenuX + Film[0]->Width)) <= 100) {
        x += LayoutAnimMoveForMenuX;
      }
    }
  }
  
  Now = AsmReadTsc();
  if (LastDraw == 0) {
    //first start, we should save background into last frame
    egFillImageArea(AnimeImage, 0, 0, AnimeImage->Width, AnimeImage->Height, &MenuBackgroundPixel);
    egTakeImage(Film[Frames],
                x, y,
                Film[Frames]->Width,
                Film[Frames]->Height);
  }
  if (TimeDiff(LastDraw, Now) < FrameTime) return;
  if (Film[CurrentFrame]) {
    egRawCopy(AnimeImage->PixelData, Film[Frames]->PixelData,
              Film[Frames]->Width, 
              Film[Frames]->Height,
              AnimeImage->Width,
              Film[Frames]->Width);
    AnimeImage->HasAlpha = FALSE;
    egComposeImage(AnimeImage, Film[CurrentFrame], 0, 0);  //aaaa
    BltImage(AnimeImage, x, y);
  }
  CurrentFrame++;
  if (CurrentFrame >= Frames) {
    AnimeRun = !Once;
    CurrentFrame = 0;
  }
  LastDraw = Now;
}
#if USE_XTHEME
//by initial we use EG_IMAGE anime
VOID REFIT_MENU_SCREEN::InitAnime()
{
  CHAR16      FileName[256];
  CHAR16      *Path;
  EG_IMAGE    *p = NULL;
  EG_IMAGE    *Last = NULL;
  GUI_ANIME   *Anime;

  if (GlobalConfig.TextOnly) return;
  //
  for (Anime = GuiAnime; Anime != NULL && Anime->ID != ID; Anime = Anime->Next);

  // Check if we should clear old film vars (no anime or anime path changed)
  //
  if (gThemeOptionsChanged || !Anime || !Film || IsEmbeddedTheme() ||
      gThemeChanged) {
    //    DBG(" free screen\n");
    if (Film) {
      //free images in the film
      for (INTN i = 0; i <= Frames; i++) { //really there are N+1 frames
        // free only last occurrence of repeated frames
        if (Film[i] != NULL && (i == Frames || Film[i] != Film[i+1])) {
          FreePool(Film[i]);
        }
      }
      FreePool(Film);
      Film = NULL;
      Frames = 0;
    }
  }
  // Check if we should load anime files (first run or after theme change)
  if (Anime && Film == NULL) {
    Path = Anime->Path;
    Film = (EG_IMAGE**)AllocateZeroPool((Anime->Frames + 1) * sizeof(VOID*));
    if ((GlobalConfig.TypeSVG || Path) && Film) {
      // Look through contents of the directory
      UINTN i;
      for (i = 0; i < Anime->Frames; i++) {
        //       DBG("Try to load file %s\n", FileName);
        if (ThemeX.TypeSVG) {
          p = LoadSvgFrame(i);
          //       DBG("frame %d loaded\n", i);
        } else {
          UnicodeSPrint(FileName, 512, L"%s\\%s_%03d.png", Path, Path, i);
          p = egLoadImage(ThemeDir, FileName, TRUE);
        }
        if (!p) {
          p = Last;
          if (!p) break;
        } else {
          Last = p;
        }
        Film[i] = p;
      }
      if (Film[0] != NULL) {
        Frames = i;
        DBG(" found %d frames of the anime\n", i);
        // Create background frame
        Film[i] = egCreateImage(Film[0]->Width, Film[0]->Height, FALSE);
        // Copy some settings from Anime into Screen
        FrameTime = Anime->FrameTime;
        Once = Anime->Once;
//        Theme = (__typeof__(Theme))AllocateCopyPool(StrSize(GlobalConfig.Theme), GlobalConfig.Theme);
      } /*else {
         DBG("Film[0] == NULL\n");
         } */
    }
  }
  // Check if a new style placement value has been specified
  if (Anime && (Anime->FilmX >=0) && (Anime->FilmX <=100) &&
      (Anime->FilmY >=0) && (Anime->FilmY <=100) &&
      (Film != NULL) && (Film[0] != NULL)) {
    // Check if screen size being used is different from theme origination size.
    // If yes, then recalculate the animation placement % value.
    // This is necessary because screen can be a different size, but anim is not scaled.
    FilmPlace.XPos = HybridRepositioning(Anime->ScreenEdgeHorizontal, Anime->FilmX, Film[0]->Width,  UGAWidth,  ThemeX.ThemeDesignWidth );
    FilmPlace.YPos = HybridRepositioning(Anime->ScreenEdgeVertical,   Anime->FilmY, Film[0]->Height, UGAHeight, ThemeX.ThemeDesignHeight);

    // Does the user want to fine tune the placement?
    FilmPlace.XPos = CalculateNudgePosition(FilmPlace.XPos, Anime->NudgeX, Film[0]->Width, UGAWidth);
    FilmPlace.YPos = CalculateNudgePosition(FilmPlace.YPos, Anime->NudgeY, Film[0]->Height, UGAHeight);

    FilmPlace.Width = Film[0]->Width;
    FilmPlace.Height = Film[0]->Height;
    DBG("recalculated Film position\n");
  } else {
    // We are here if there is no anime, or if we use oldstyle placement values
    // For both these cases, FilmPlace will be set after banner/menutitle positions are known
    FilmPlace.XPos = 0;
    FilmPlace.YPos = 0;
    FilmPlace.Width = 0;
    FilmPlace.Height = 0;
  }
  if (Film != NULL && Film[0] != NULL) {
    DBG(" Anime seems OK, init it\n");
    AnimeRun = TRUE;
    CurrentFrame = 0;
    LastDraw = 0;
  } else {
    //    DBG("not run anime\n");
    AnimeRun = FALSE;
  }
  //  DBG("anime inited\n");
}

#else

VOID REFIT_MENU_SCREEN::InitAnime()
{
  CHAR16      FileName[256];
  CHAR16      *Path;
  EG_IMAGE    *p = NULL;
  EG_IMAGE    *Last = NULL;
  GUI_ANIME   *Anime;

  if (GlobalConfig.TextOnly) return;
  // 
  for (Anime = GuiAnime; Anime != NULL && Anime->ID != ID; Anime = Anime->Next);

  // Check if we should clear old film vars (no anime or anime path changed)
  //
  if (gThemeOptionsChanged || !Anime || !Film || IsEmbeddedTheme() || !Theme ||
      (/*gThemeChanged && */StriCmp(GlobalConfig.Theme, Theme) != 0)) {
//    DBG(" free screen\n");
    if (Film) {
      //free images in the film
      INTN i;
      for (i = 0; i <= Frames; i++) { //really there are N+1 frames
        // free only last occurrence of repeated frames
        if (Film[i] != NULL && (i == Frames || Film[i] != Film[i+1])) {
          FreePool(Film[i]);
        }
      }
      FreePool(Film);
      Film = NULL;
      Frames = 0;
    }
    if (Theme) {
      FreePool(Theme);
      Theme = NULL;
    }
  }
  // Check if we should load anime files (first run or after theme change)
  if (Anime && Film == NULL) {
    Path = Anime->Path;
    Film = (EG_IMAGE**)AllocateZeroPool((Anime->Frames + 1) * sizeof(VOID*));
    if ((GlobalConfig.TypeSVG || Path) && Film) {
      // Look through contents of the directory
      UINTN i;
      for (i = 0; i < Anime->Frames; i++) {

 //       DBG("Try to load file %s\n", FileName);
        if (GlobalConfig.TypeSVG) {
          p = LoadSvgFrame(i);
   //       DBG("frame %d loaded\n", i);
        } else {
          UnicodeSPrint(FileName, 512, L"%s\\%s_%03d.png", Path, Path, i);
          p = egLoadImage(ThemeDir, FileName, TRUE);
        }
        if (!p) {
          p = Last;
          if (!p) break;
        } else {
          Last = p;
        }
        Film[i] = p;
      }
      if (Film[0] != NULL) {
        Frames = i;
        DBG(" found %d frames of the anime\n", i);
        // Create background frame
        Film[i] = egCreateImage(Film[0]->Width, Film[0]->Height, FALSE);
        // Copy some settings from Anime into Screen
        FrameTime = Anime->FrameTime;
        Once = Anime->Once;
        Theme = (__typeof__(Theme))AllocateCopyPool(StrSize(GlobalConfig.Theme), GlobalConfig.Theme);
      } /*else {
        DBG("Film[0] == NULL\n");
      } */
    }
  }
  // Check if a new style placement value has been specified
  if (Anime && (Anime->FilmX >=0) && (Anime->FilmX <=100) &&
      (Anime->FilmY >=0) && (Anime->FilmY <=100) &&
      (Film != NULL) && (Film[0] != NULL)) {
    // Check if screen size being used is different from theme origination size.
    // If yes, then recalculate the animation placement % value.
    // This is necessary because screen can be a different size, but anim is not scaled.
    FilmPlace.XPos = HybridRepositioning(Anime->ScreenEdgeHorizontal, Anime->FilmX, Film[0]->Width,  UGAWidth,  GlobalConfig.ThemeDesignWidth );
    FilmPlace.YPos = HybridRepositioning(Anime->ScreenEdgeVertical,   Anime->FilmY, Film[0]->Height, UGAHeight, GlobalConfig.ThemeDesignHeight);
    
    // Does the user want to fine tune the placement?
    FilmPlace.XPos = CalculateNudgePosition(FilmPlace.XPos, Anime->NudgeX, Film[0]->Width, UGAWidth);
    FilmPlace.YPos = CalculateNudgePosition(FilmPlace.YPos, Anime->NudgeY, Film[0]->Height, UGAHeight);
    
    FilmPlace.Width = Film[0]->Width;
    FilmPlace.Height = Film[0]->Height;
    DBG("recalculated Film position\n");
  } else {
    // We are here if there is no anime, or if we use oldstyle placement values
    // For both these cases, FilmPlace will be set after banner/menutitle positions are known
    FilmPlace.XPos = 0;
    FilmPlace.YPos = 0;
    FilmPlace.Width = 0;
    FilmPlace.Height = 0;
  }
  if (Film != NULL && Film[0] != NULL) {
    DBG(" Anime seems OK, init it\n");
    AnimeRun = TRUE;
    CurrentFrame = 0;
    LastDraw = 0;
  } else {
//    DBG("not run anime\n");
    AnimeRun = FALSE;
  }
//  DBG("anime inited\n");
}
#endif
BOOLEAN REFIT_MENU_SCREEN::GetAnime()
{
  GUI_ANIME   *Anime;
  
  if (!GuiAnime) return FALSE;
  
  for (Anime = GuiAnime; Anime != NULL && Anime->ID != ID; Anime = Anime->Next);
  if (Anime == NULL || Anime->Path == NULL) {
    return FALSE;
  }
  
  DBG("Use anime=%s frames=%d\n", Anime->Path, Anime->Frames);
  
  return TRUE;
}

//
// Sets next/previous available screen resolution, according to specified offset
//

VOID SetNextScreenMode(INT32 Next)
{
    EFI_STATUS Status;

    Status = egSetMode(Next);
    if (!EFI_ERROR(Status)) {
        UpdateConsoleVars();
    }
}

//
// Updates console variables, according to ConOut resolution 
// This should be called when initializing screen, or when resolution changes
//

static VOID UpdateConsoleVars()
{
    UINTN i;

    // get size of text console
	if  (gST->ConOut->QueryMode(gST->ConOut, gST->ConOut->Mode->Mode, &ConWidth, &ConHeight) != EFI_SUCCESS) {
        // use default values on error
        ConWidth = 80;
        ConHeight = 25;
    }

    // free old BlankLine when it exists
    if (BlankLine != NULL) {
        FreePool(BlankLine);
    }

    // make a buffer for a whole text line
    BlankLine = (__typeof__(BlankLine))AllocatePool((ConWidth + 1) * sizeof(CHAR16));
	
	for (i = 0; i < ConWidth; i++) {
        BlankLine[i] = ' ';
	}
	
    BlankLine[i] = 0;
}
