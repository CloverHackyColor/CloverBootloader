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

//#include "Platform.h"
#include "libegint.h"

#include "egemb_refit_banner.h"

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
CHAR16 *BlankLine;

static VOID SwitchToText(IN BOOLEAN CursorEnabled);
static VOID SwitchToGraphics(VOID);
static VOID DrawScreenHeader(IN CHAR16 *Title);


// UGA defines and variables

INTN   UGAWidth;
INTN   UGAHeight;
BOOLEAN AllowGraphicsMode;

EG_RECT  BannerPlace = {0, 0, 0, 0};

EG_PIXEL StdBackgroundPixel   = { 0xbf, 0xbf, 0xbf, 0x00};
EG_PIXEL MenuBackgroundPixel  = { 0xbf, 0xbf, 0xbf, 0x80};
EG_PIXEL InputBackgroundPixel = { 0xcf, 0xcf, 0xcf, 0x80};

EG_IMAGE *BackgroundImage = NULL;


static BOOLEAN GraphicsScreenDirty;

// general defines and variables

static BOOLEAN haveError = FALSE;

//
// Screen initialization and switching
//

VOID InitScreen(VOID)
{
    UINTN i;
    
    // initialize libeg
    egInitScreen();
    
    if (egHasGraphicsMode()) {
        egGetScreenSize(&UGAWidth, &UGAHeight);
        AllowGraphicsMode = TRUE;
    } else {
        AllowGraphicsMode = FALSE;
        egSetGraphicsModeEnabled(FALSE);   // just to be sure we are in text mode
    }
    GraphicsScreenDirty = TRUE;
    
    // disable cursor
    gST->ConOut->EnableCursor (gST->ConOut, FALSE);
    
    // get size of text console
    if  (gST->ConOut->QueryMode (gST->ConOut, gST->ConOut->Mode->Mode, &ConWidth, &ConHeight) != EFI_SUCCESS) {
        // use default values on error
        ConWidth = 80;
        ConHeight = 25;
    }
    
    // make a buffer for a whole text line
    BlankLine = AllocatePool((ConWidth + 1) * sizeof(CHAR16));
    for (i = 0; i < ConWidth; i++)
        BlankLine[i] = ' ';
    BlankLine[i] = 0;
    
    // show the banner (even when in graphics mode)
//    DrawScreenHeader(L"Initializing...");
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
//        BltClearScreen(TRUE);
    }
}

static VOID SwitchToText(IN BOOLEAN CursorEnabled)
{
    egSetGraphicsModeEnabled(FALSE);
    gST->ConOut->EnableCursor (gST->ConOut, CursorEnabled);
}

static VOID SwitchToGraphics(VOID)
{
    if (AllowGraphicsMode && !egIsGraphicsModeEnabled()) {
        egSetGraphicsModeEnabled(TRUE);
        GraphicsScreenDirty = TRUE;
    }
}

//
// Screen control for running tools
//

VOID BeginTextScreen(IN CHAR16 *Title)
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
        PauseForKey(L"FinishTextScreen");
    }
    
    // reset error flag
    haveError = FALSE;
}

VOID BeginExternalScreen(IN BOOLEAN UseGraphicsMode, IN CHAR16 *Title)
{
    if (!AllowGraphicsMode)
        UseGraphicsMode = FALSE;
    
    if (UseGraphicsMode) {
        SwitchToGraphics();
//        BltClearScreen(FALSE);
    }
    
    // show the header
    DrawScreenHeader(Title);
    
    if (!UseGraphicsMode)
        SwitchToText(TRUE);
    
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
    gST->ConOut->SetAttribute (gST->ConOut, ATTR_BASIC);
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // enable cursor
    gST->ConOut->EnableCursor (gST->ConOut, TRUE);
}

static VOID DrawScreenHeader(IN CHAR16 *Title)
{
    UINTN y;
    
    // clear to black background
    gST->ConOut->SetAttribute (gST->ConOut, ATTR_BASIC);
//    gST->ConOut->ClearScreen (gST->ConOut);
    
    // paint header background
    gST->ConOut->SetAttribute (gST->ConOut, ATTR_BANNER);
    for (y = 0; y < 3; y++) {
        gST->ConOut->SetCursorPosition (gST->ConOut, 0, y);
        Print(BlankLine);
    }
    
    // print header text
    gST->ConOut->SetCursorPosition (gST->ConOut, 3, 1);
    Print(L"rEFIt - %s", Title);
    
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

VOID PauseForKey(CHAR16* msg)
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


BOOLEAN CheckFatalError(IN EFI_STATUS Status, IN CHAR16 *where)
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

BOOLEAN CheckError(IN EFI_STATUS Status, IN CHAR16 *where)
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

VOID SwitchToGraphicsAndClear(VOID)
{
    SwitchToGraphics();
    if (GraphicsScreenDirty)
        BltClearScreen(TRUE);
}
/*
typedef struct {
  INTN     XPos;
  INTN     YPos;
  INTN     Width;
  INTN     Height;
} EG_RECT;
*/

static EG_IMAGE *Banner = NULL;


VOID BltClearScreen(IN BOOLEAN ShowBanner)
{
  EG_IMAGE *BigBack = NULL;
  INTN BanHeight = ((UGAHeight - LAYOUT_TOTAL_HEIGHT) >> 1) + LAYOUT_BANNER_HEIGHT;
  INTN i, j, x, x1, x2, y, y1, y2;
  EG_PIXEL    *p1;
  
  // load banner on first call
  if (!Banner) {
    if (!GlobalConfig.BannerFileName)
      Banner = egPrepareEmbeddedImage(&egemb_refit_banner, FALSE);
    else
      Banner = egLoadImage(ThemeDir, GlobalConfig.BannerFileName, FALSE);
    if (Banner != NULL)
      MenuBackgroundPixel = Banner->PixelData[0];
  }
  if (!Banner) {
    DBG("banner file not read\n");
  }
  
  //load Background and scale
  if (!BackgroundImage && (GlobalConfig.BackgroundName != NULL)) {
    BigBack = egLoadImage(ThemeDir, GlobalConfig.BackgroundName, FALSE);
    if (BigBack != NULL) {
      BackgroundImage = egCreateFilledImage(UGAWidth, UGAHeight, FALSE, &MenuBackgroundPixel);
      switch (GlobalConfig.BackgroundScale) {
          //TODO - make scale
        case Scale: //not for now
        case None:
        case Crop:
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
        case Tile:
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
        default:
          break;
      }
    }
  }
    
  if (ShowBanner && !(GlobalConfig.HideUIFlags & HIDEUI_FLAG_BANNER)) {
    // clear and draw banner
    egClearScreen(&MenuBackgroundPixel);
    BltImage(BackgroundImage, 0, 0); //if NULL then do nothing
    if (Banner != NULL){
      BannerPlace.XPos = (UGAWidth - Banner->Width) >> 1;
      BannerPlace.YPos = (BanHeight >= Banner->Height) ? (BanHeight - Banner->Height) : 0;
      BannerPlace.Width = Banner->Width;
      BannerPlace.Height = (BanHeight >= Banner->Height) ? Banner->Height : BanHeight;
      BltImageAlpha(Banner, BannerPlace.XPos, BannerPlace.YPos, &MenuBackgroundPixel, 16);
    }
  } else {
    // clear to standard background color
    egClearScreen(&StdBackgroundPixel);
    BannerPlace.XPos = 0;
    BannerPlace.YPos = 0;
    BannerPlace.Width = UGAWidth;
    BannerPlace.Height = BanHeight;
  }
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
  
  egRawCopy(NewImage->PixelData,
            BackgroundImage->PixelData + YPos * BackgroundImage->Width + XPos,
            Width, Height,
            Width,
            BackgroundImage->Width);
  egComposeImage(NewImage, CompImage, 0, 0);
  egFreeImage(CompImage);

  // blit to screen and clean up
  egDrawImageArea(NewImage, 0, 0, 0, 0, XPos, YPos);
  if (NewImage) {
    egFreeImage(NewImage);
  }
}

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

VOID BltImageCompositeBadge(IN EG_IMAGE *BaseImage, IN EG_IMAGE *TopImage, IN EG_IMAGE *BadgeImage, IN INTN XPos, IN INTN YPos)
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
//  DBG("BaseImage: Width=%d Height=%d Alfa=%d\n", TotalWidth, TotalHeight, CompImage->HasAlpha);
    // place the top image
    CompWidth = TopImage->Width;
    if (CompWidth > TotalWidth)
        CompWidth = TotalWidth;
    OffsetX = (TotalWidth - CompWidth) >> 1;
//  DBG("TopImage: Width=%d Height=%d\n", TopImage->Width, TopImage->Height);
    CompHeight = TopImage->Height;
    if (CompHeight > TotalHeight)
        CompHeight = TotalHeight;
    OffsetY = (TotalHeight - CompHeight) >> 1;
    egComposeImage(CompImage, TopImage, OffsetX, OffsetY);
    
    // place the badge image
    if (BadgeImage != NULL && (BadgeImage->Width + 8) < CompWidth && (BadgeImage->Height + 8) < CompHeight) {
        OffsetX += CompWidth  - 8 - BadgeImage->Width;
        OffsetY += CompHeight - 8 - BadgeImage->Height;
        egComposeImage(CompImage, BadgeImage, OffsetX, OffsetY);
    }
    
    // blit to screen and clean up
//    egDrawImageArea(CompImage, 0, 0, TotalWidth, TotalHeight, XPos, YPos);
  BltImageAlpha(CompImage, XPos, YPos, &MenuBackgroundPixel, 16);
    egFreeImage(CompImage);
    GraphicsScreenDirty = TRUE;
}

VOID InitAnime()
{
  /*
  // dmazar: original init code is here:
  INTN i;
  for (i=0; i<MAX_ANIME; i++){
    AnimeName[i] = NULL;
    AnimeFrames[i] = 0;
    AnimeFrameTime[i] = 100;
  }
  // but MS compiler optimizes this with memset() RTL function and causes linker error.
  // note: setting AnimeFrameTime elements to 100 depends on them
  // being UINTN or INTN with following code
  */
  ZeroMem (AnimeName, MAX_ANIME * sizeof(AnimeName[0]));  
  ZeroMem (AnimeFrames, MAX_ANIME * sizeof(AnimeFrames[0]));  
  SetMemN(AnimeFrameTime, MAX_ANIME, 100);  
}

static EG_IMAGE    *CompImage = NULL;
#define MAX_SIZE_ANIME 256

VOID UpdateAnime(REFIT_MENU_SCREEN *Screen, EG_RECT *Place)
{
  UINT64      Now;
  INTN   x, y;
  if (!Screen || !Screen->AnimeRun) return;
  if (!CompImage ||
      (CompImage->Width != Screen->Film[0]->Width) ||
      (CompImage->Height != Screen->Film[0]->Height)){
    if (CompImage) {
      egFreeImage(CompImage);
    }
    CompImage = egCreateImage(Screen->Film[0]->Width, Screen->Film[0]->Height, TRUE);
  }   
  x = Place->XPos + (Place->Width - CompImage->Width) / 2;
  y = Place->YPos + (Place->Height - CompImage->Height) / 2;
  Now = AsmReadTsc();
  if (Screen->LastDraw == 0) {
    //first start, we should save background into last frame
    egFillImageArea(CompImage, 0, 0, CompImage->Width, CompImage->Height, &MenuBackgroundPixel);
    egTakeImage(Screen->Film[Screen->Frames],
                x, y,
                Screen->Film[Screen->Frames]->Width,
                Screen->Film[Screen->Frames]->Height);
  }
  if (TimeDiff(Screen->LastDraw, Now) < Screen->FrameTime) return;
  if (Screen->Film[Screen->CurrentFrame]) {
    egRawCopy(CompImage->PixelData, Screen->Film[Screen->Frames]->PixelData,
              Screen->Film[Screen->Frames]->Width, 
              Screen->Film[Screen->Frames]->Height,
              CompImage->Width,
              Screen->Film[Screen->Frames]->Width);
    egComposeImage(CompImage, Screen->Film[Screen->CurrentFrame], 0, 0);
    BltImage(CompImage, x, y);
  }
  Screen->CurrentFrame++;
  if (Screen->CurrentFrame >= Screen->Frames) {
    Screen->AnimeRun = !Screen->Once;
    Screen->CurrentFrame = 0;
  }
  Screen->LastDraw = Now;
}

BOOLEAN GetAnime(REFIT_MENU_SCREEN *Screen)
{
  CHAR16      FileName[256];
  CHAR16      *Path;
  INTN        i, N;
  EG_IMAGE    *p = NULL;
  EG_IMAGE    *Last = NULL;
  
  if (!Screen) return FALSE;
  
  // look through contents of the directory
  Path = AnimeName[Screen->ID];
  if (!Path) return FALSE;
  N = AnimeFrames[Screen->ID];
  DBG("Use anime=%s frames=%d\n", Path, N);
  Screen->Film = (EG_IMAGE**)AllocateZeroPool(N * sizeof(VOID*));
  for (i=0; i<N; i++){
  
    UnicodeSPrint(FileName, 512, L"%s\\%s_%03d.png", Path, Path, i);
//    DBG("Try to load file %s\n", FileName);
    p = egLoadImage(ThemeDir, FileName, TRUE);
    if (!p) {
      p = Last;
      if (!p) break;
    } else {
      Last = p;
    }
    Screen->Film[i] = p;    
  }
  if (Screen->Film[0] == NULL)  return FALSE;

  Screen->Frames = i;
    //Create background frame
  Screen->Film[i] = egCreateImage(Screen->Film[0]->Width, Screen->Film[0]->Height, FALSE);

  Screen->FrameTime = AnimeFrameTime[Screen->ID];
  DBG(" found %d frames of the anime\n", i);
  Screen->CurrentFrame = 0;
  Screen->LastDraw = 0;
  Screen->Once = AnimeOnce[Screen->ID];
  return TRUE;
}

