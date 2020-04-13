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
#include "../Platform/Platform.h"
#include "../libeg/libegint.h" // included Platform.h
#include "../libeg/XTheme.h"
#include "../Platform/BasicIO.h"
#include "menu.h"

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

VOID BeginExternalScreen(IN BOOLEAN UseGraphicsMode/*, IN CONST CHAR16 *Title*/)
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
VOID
StatusToString (
				OUT CHAR16      *Buffer,
				EFI_STATUS      Status
				)
{
	snwprintf(Buffer, 64, "EFI Error %s", strerror(Status));
}*/


BOOLEAN CheckFatalError(IN EFI_STATUS Status, IN CONST CHAR16 *where)
{
//    CHAR16 ErrorName[64];
    
    if (!EFI_ERROR(Status))
        return FALSE;
    
//    StatusToString(ErrorName, Status);
    gST->ConOut->SetAttribute (gST->ConOut, ATTR_ERROR);
    printf("Fatal Error: %s %ls\n", strerror(Status), where);
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
    printf("Error: %s %ls\n", strerror(Status), where);
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
  CompImage = egCreateFilledImage(Width, Height, !ThemeX.Background.isEmpty(), BackgroundPixel); //no matter

  egComposeImage(CompImage, NewImage, 0, 0);
  if (NewImage) {
    egFreeImage(NewImage);
  }
  if (ThemeX.Background.isEmpty()) {
    egDrawImageArea(CompImage, 0, 0, 0, 0, XPos, YPos);
    egFreeImage(CompImage);
    return;
  }
  NewImage = egCreateImage(Width, Height, FALSE);
  if (!NewImage) return;
//  DBG("draw on background\n");
  egRawCopy(NewImage->PixelData,
            (EG_PIXEL*)ThemeX.Background.GetPixelPtr(0,0) + YPos * ThemeX.Background.GetWidth() + XPos,
            Width, Height,
            Width,
            ThemeX.Background.GetWidth());

  egComposeImage(NewImage, CompImage, 0, 0);
  egFreeImage(CompImage);

  // blit to screen and clean up
  egDrawImageArea(NewImage, 0, 0, 0, 0, XPos, YPos);
  egFreeImage(NewImage);
}

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
#if XCINEMA
BOOLEAN REFIT_MENU_SCREEN::GetAnime()
{
  FilmX = ThemeX.Cinema.GetFilm(ID);
  return FilmX != nullptr;
}

VOID REFIT_MENU_SCREEN::InitAnime()
{
  //something
}
#else

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
  if ((ID > 1) && (ThemeX.LayoutAnimMoveForMenuX != 0)) { // these screens have text menus which the anim may interfere with.
    MenuWidth = (INTN)(TEXT_XMARGIN * 2 + (50 * ThemeX.CharWidth * ThemeX.Scale)); // taken from menu.c
    if ((x + Film[0]->Width) > (UGAWidth - MenuWidth) >> 1) {
      if ((x + ThemeX.LayoutAnimMoveForMenuX >= 0) || (UGAWidth-(x + ThemeX.LayoutAnimMoveForMenuX + Film[0]->Width)) <= 100) {
        x += ThemeX.LayoutAnimMoveForMenuX;
      }
    }
  }

  Now = AsmReadTsc();
  if (LastDraw == 0) {
    //first start, we should save background into last frame
    egFillImageArea(AnimeImage, 0, 0, AnimeImage->Width, AnimeImage->Height, (EG_PIXEL*)&MenuBackgroundPixel);
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
//by initial we use EG_IMAGE anime
//TODO will be rewritten by XCinema class
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
    if ((ThemeX.TypeSVG || Path) && Film) {
      // Look through contents of the directory
      UINTN i;
      for (i = 0; i < Anime->Frames; i++) {
        //       DBG("Try to load file %ls\n", FileName);
        if (ThemeX.TypeSVG) {
          p = LoadSvgFrame(i);
          //       DBG("frame %d loaded\n", i);
        } else {
          snwprintf(FileName, 512, "%ls\\%ls_%03llu.png", Path, Path, i);
          p = egLoadImage(ThemeX.ThemeDir, FileName, TRUE);
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
        DBG(" found %llu frames of the anime\n", i);
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

BOOLEAN REFIT_MENU_SCREEN::GetAnime()
{
  GUI_ANIME   *Anime;
  
  if (!GuiAnime) return FALSE;
  
  for (Anime = GuiAnime; Anime != NULL && Anime->ID != ID; Anime = Anime->Next);
  if (Anime == NULL || Anime->Path == NULL) {
    return FALSE;
  }
  
	DBG("Use anime=%ls frames=%llu\n", Anime->Path, Anime->Frames);
  
  return TRUE;
}
#endif
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
