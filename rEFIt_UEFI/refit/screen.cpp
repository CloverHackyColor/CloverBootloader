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

#include "screen.h"
#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "../libeg/libegint.h"
#include "../libeg/XTheme.h"
#include "../Platform/BasicIO.h"
#include "menu.h"
#include "../gui/REFIT_MENU_SCREEN.h"
#include "../Platform/CloverVersion.h"

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

static void SwitchToText(IN BOOLEAN CursorEnabled);
static void SwitchToGraphics(void);
static void DrawScreenHeader(IN CONST CHAR16 *Title);
static void UpdateConsoleVars(void);
static INTN ConvertEdgeAndPercentageToPixelPosition(INTN Edge, INTN DesiredPercentageFromEdge, INTN ImageDimension, INTN ScreenDimension);
INTN CalculateNudgePosition(INTN Position, INTN NudgeValue, INTN ImageDimension, INTN ScreenDimension);
//INTN RecalculateImageOffset(INTN AnimDimension, INTN ValueToScale, INTN ScreenDimensionToFit, INTN ThemeDesignDimension);
static BOOLEAN IsImageWithinScreenLimits(INTN Value, INTN ImageDimension, INTN ScreenDimension);
static INTN RepositionFixedByCenter(INTN Value, INTN ScreenDimension, INTN DesignScreenDimension);
static INTN RepositionRelativeByGapsOnEdges(INTN Value, INTN ImageDimension, INTN ScreenDimension, INTN DesignScreenDimension);
INTN HybridRepositioning(INTN Edge, INTN Value, INTN ImageDimension, INTN ScreenDimension, INTN DesignScreenDimension);
// UGA defines and variables

INTN   UGAWidth;
INTN   UGAHeight;
BOOLEAN AllowGraphicsMode;
const EFI_GRAPHICS_OUTPUT_BLT_PIXEL StdBackgroundPixel   = { 0xbf, 0xbf, 0xbf, 0xff};
const EFI_GRAPHICS_OUTPUT_BLT_PIXEL MenuBackgroundPixel  = { 0x00, 0x00, 0x00, 0x00};
const EFI_GRAPHICS_OUTPUT_BLT_PIXEL InputBackgroundPixel = { 0xcf, 0xcf, 0xcf, 0x80};
EFI_GRAPHICS_OUTPUT_BLT_PIXEL BlueBackgroundPixel  = { 0x7f, 0x0f, 0x0f, 0xff};
//const EFI_GRAPHICS_OUTPUT_BLT_PIXEL EmbeddedBackgroundPixel  = { 0xaa, 0xaa, 0xaa, 0xff};
const EFI_GRAPHICS_OUTPUT_BLT_PIXEL DarkSelectionPixel   = { 66, 66, 66, 0xff};
const EFI_GRAPHICS_OUTPUT_BLT_PIXEL DarkEmbeddedBackgroundPixel  = { 0x33, 0x33, 0x33, 0xff};
const EFI_GRAPHICS_OUTPUT_BLT_PIXEL WhitePixel  = { 0xff, 0xff, 0xff, 0xff};
const EFI_GRAPHICS_OUTPUT_BLT_PIXEL BlackPixel  = { 0x00, 0x00, 0x00, 0xff};
EFI_GRAPHICS_OUTPUT_BLT_PIXEL SelectionBackgroundPixel = { 0xef, 0xef, 0xef, 0xff };

static BOOLEAN GraphicsScreenDirty;

// general defines and variables

//
// Screen initialization and switching
//

void InitScreen(IN BOOLEAN SetMaxResolution)
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

void SetupScreen(void)
{
    if (gSettings.GUI.TextOnly) {
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

static void SwitchToText(IN BOOLEAN CursorEnabled)
{
  egSetGraphicsModeEnabled(FALSE);
	gST->ConOut->EnableCursor(gST->ConOut, CursorEnabled);
}

static void SwitchToGraphics(void)
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
void BeginTextScreen(IN CONST CHAR16 *Title)
{
    DrawScreenHeader(Title);
    SwitchToText(FALSE);
    
    // reset error flag
    haveError = FALSE;
}

void FinishTextScreen(IN BOOLEAN WaitAlways)
{
    if (haveError || WaitAlways) {
        SwitchToText(FALSE);
 //       PauseForKey(L"FinishTextScreen");
    }
    
    // reset error flag
    haveError = FALSE;
}

void BeginExternalScreen(IN BOOLEAN UseGraphicsMode/*, IN CONST CHAR16 *Title*/)
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

void FinishExternalScreen(void)
{
    // make sure we clean up later
    GraphicsScreenDirty = TRUE;
    
    if (haveError) {
        // leave error messages on screen in case of error,
        // wait for a key press, and then switch
        PauseForKey("was error."_XS8);
        SwitchToText(FALSE);
    }
    
    // reset error flag
    haveError = FALSE;
}

void TerminateScreen(void)
{
    // clear text screen
	gST->ConOut->SetAttribute(gST->ConOut, ATTR_BANNER);
	gST->ConOut->ClearScreen(gST->ConOut);
    
    // enable cursor
	gST->ConOut->EnableCursor(gST->ConOut, TRUE);
}

static void DrawScreenHeader(IN CONST CHAR16 *Title)
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
	printf("%ls", BannerLine);

	for (i = 1; i < ConWidth-1; i++)
    BannerLine[i] = ' ';
	BannerLine[0] = BOXDRAW_VERTICAL;
	BannerLine[ConWidth-1] = BOXDRAW_VERTICAL;
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 1);
	printf("%ls", BannerLine);

	for (i = 1; i < ConWidth-1; i++)
    BannerLine[i] = BOXDRAW_HORIZONTAL;
 	BannerLine[0] = BOXDRAW_UP_RIGHT;
	BannerLine[ConWidth-1] = BOXDRAW_UP_LEFT;
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 2);
	printf("%ls", BannerLine);

	FreePool(BannerLine);

  // print header text
  gST->ConOut->SetCursorPosition (gST->ConOut, 3, 1);
  printf("Clover rev %ls - %ls", gFirmwareRevision, Title);

  // reposition cursor
  gST->ConOut->SetAttribute (gST->ConOut, ATTR_BASIC);
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 4);
}


//
// Error handling
//
/*
void
StatusToString (
				OUT CHAR16      *Buffer,
				EFI_STATUS      Status
				)
{
	snwprintf(Buffer, 64, "EFI Error %s", efiStrError(Status));
}*/


//
// Graphics functions
//

void SwitchToGraphicsAndClear(void) //called from MENU_FUNCTION_INIT
{
  SwitchToGraphics();
  ThemeX.Background.DrawWithoutCompose(0,0,0,0);
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
/*
  --------------------------------------------------------------------
  Pos                           : Bottom    -> Mid        -> Top
  --------------------------------------------------------------------
   GlobalConfig.SelectionOnTop  : MainImage -> Badge      -> Selection
  !GlobalConfig.SelectionOnTop  : Selection -> MainImage  -> Badge

 GlobalConfig.SelectionOnTop
  BaseImage = MainImage, TopImage = Selection
*/


#define MAX_SIZE_ANIME 256

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

void REFIT_MENU_SCREEN::GetAnime()
{
  FilmC = ThemeX.Cinema.GetFilm(ID);
//  DBG("ScreenID=%lld Film found=%d\n", ID, (FilmC != nullptr)?1:0);
  if (FilmC != nullptr) {
    FilmC->AnimeRun = true;
  }
}

void REFIT_MENU_SCREEN::InitAnime()
{
  if (GlobalConfig.gThemeChanged) {
    FilmC = nullptr;
  }
  if (FilmC == nullptr) {
//    DBG("Screen %lld inited without anime\n", ID);
//    FilmC->AnimeRun = FALSE;
    return;
  }
//  DBG("=== Debug Film ===\n");
//  DBG("FilmX=%lld\n", FilmC->FilmX);
//  DBG("ID=%lld\n", FilmC->GetIndex());
//  DBG("RunOnce=%d\n", FilmC->RunOnce?1:0);
//  DBG("NumFrames=%lld\n", FilmC->NumFrames);
//  DBG("FrameTime=%lld\n", FilmC->FrameTime);
//  DBG("Path=%ls\n", FilmC->Path.wc_str());
//  DBG("LastFrame=%lld\n\n", FilmC->LastFrameID());

  XImage FirstFrame = FilmC->GetImage(FilmC->LastFrameID()); //can not be absent
  INTN CWidth = FirstFrame.GetWidth();
  INTN CHeight = FirstFrame.GetHeight();
  if ((FilmC->FilmX >=0) && (FilmC->FilmX <=100) &&
      (FilmC->FilmY >=0) && (FilmC->FilmY <=100)) { //default is 0xFFFF
    // Check if screen size being used is different from theme origination size.
    // If yes, then recalculate the animation placement % value.
    // This is necessary because screen can be a different size, but anim is not scaled.
    FilmC->FilmPlace.XPos = HybridRepositioning(FilmC->ScreenEdgeHorizontal, FilmC->FilmX, CWidth,  UGAWidth,  ThemeX.ThemeDesignWidth );
    FilmC->FilmPlace.YPos = HybridRepositioning(FilmC->ScreenEdgeVertical,   FilmC->FilmY, CHeight, UGAHeight, ThemeX.ThemeDesignHeight);

    // Does the user want to fine tune the placement?
    FilmC->FilmPlace.XPos = CalculateNudgePosition(FilmC->FilmPlace.XPos, FilmC->NudgeX, CWidth, UGAWidth);
    FilmC->FilmPlace.YPos = CalculateNudgePosition(FilmC->FilmPlace.YPos, FilmC->NudgeY, CHeight, UGAHeight);

    FilmC->FilmPlace.Width = CWidth;
    FilmC->FilmPlace.Height = CHeight;
//    DBG("recalculated Film position [%lld, %lld]\n", FilmC->FilmPlace.XPos, FilmC->FilmPlace.YPos);
  } else {
    // We are here if there is no anime, or if we use oldstyle placement values
    // For both these cases, FilmPlace will be set after banner/menutitle positions are known
    FilmC->FilmPlace = ThemeX.BannerPlace;
    if (CWidth > 0 && CHeight > 0) {
      // Retained for legacy themes without new anim placement options.
      FilmC->FilmPlace.XPos = ((INTN)FilmC->FilmPlace.XPos * 2 > CWidth  - (INTN)FilmC->FilmPlace.Width ) ? (UINTN)((INTN)FilmC->FilmPlace.XPos + ((INTN)FilmC->FilmPlace.Width  - CWidth ) / 2) : 0;
      FilmC->FilmPlace.YPos = ((INTN)FilmC->FilmPlace.YPos * 2 > CHeight - (INTN)FilmC->FilmPlace.Height) ? (UINTN)((INTN)FilmC->FilmPlace.YPos + ((INTN)FilmC->FilmPlace.Height - CHeight) / 2) : 0;
    }
  }
  if (FilmC->NumFrames != 0) {
    DBG(" Anime seems OK, init it\n");
    FilmC->AnimeRun = TRUE;
    FilmC->Reset();
    FilmC->LastDraw = 0;
  }
}

//
// Sets next/previous available screen resolution, according to specified offset
//

void SetNextScreenMode(INT32 Next)
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

static void UpdateConsoleVars()
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
