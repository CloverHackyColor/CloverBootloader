/*
 * refit/menu.c
 * Menu functions
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

#include "./REFIT_MAINMENU_SCREEN.h"
#include <Platform.h>
#include "../Platform/BasicIO.h"
#include "../libeg/libegint.h"   //this includes platform.h
//#include "../include/scroll_images.h"

//#include "colors.h"

#include "../libeg/nanosvg.h"
#include "../libeg/FloatLib.h"
#include "../Platform/HdaCodecDump.h"
#include "REFIT_MENU_SCREEN.h"
//#include "screen.h"
#include "../cpp_foundation/XString.h"
#include "../libeg/XTheme.h"
#include "../libeg/VectorGraphics.h" // for testSVG
#include "shared_with_menu.h"
#include "../refit/menu.h"  // for DrawTextXY. Must disappear soon.
#include "../Platform/AcpiPatcher.h"
#include "../Platform/Nvram.h"
#include "../refit/screen.h"
#include "../Platform/Events.h"
#include "../Settings/Self.h"
#include "../Platform/Volumes.h"
#include "../include/OSFlags.h"
#include "../Platform/CloverVersion.h"

#ifndef DEBUG_ALL
#define DEBUG_MENU 1
#else
#define DEBUG_MENU DEBUG_ALL
#endif

#if DEBUG_MENU == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_MENU, __VA_ARGS__)
#endif

INTN row0PosXRunning;
INTN row1PosXRunning;
INTN *itemPosX = NULL;
INTN *itemPosY = NULL;
INTN row1PosY, textPosY, FunctextPosY;
INTN OldTimeoutTextWidth = 0;
INTN EntriesWidth, EntriesHeight, EntriesGap;
INTN MaxItemOnScreen = -1;


REFIT_MAINMENU_SCREEN::REFIT_MAINMENU_SCREEN(UINTN ID, XStringW TTitle, XStringW TTimeoutText) : REFIT_MENU_SCREEN(ID, TTitle, TTimeoutText)
{
};

/**
 * Draw entries for GUI.
 */

void REFIT_MAINMENU_SCREEN::DrawMainMenuLabel(IN CONST XStringW& Text, IN INTN XPos, IN INTN YPos)
{
  INTN TextWidth = 0;
  INTN BadgeDim = (INTN)(BADGE_DIMENSION * ThemeX.Scale);

  ThemeX.MeasureText(Text, &TextWidth, NULL);

  //Clear old text
  ThemeX.FillRectAreaOfScreen(OldX, OldY, OldTextWidth, OldTextHeight);

  if (!(ThemeX.BootCampStyle)
      && (ThemeX.HideBadges & HDBADGES_INLINE) && (!OldRow)
      && (OldTextWidth) && (OldTextWidth != TextWidth)
      ) {
    //Clear badge
    ThemeX.FillRectAreaOfScreen((OldX - (OldTextWidth >> 1) - (BadgeDim + 16)),
                                (OldY - ((BadgeDim - ThemeX.TextHeight) >> 1)), 128, 128);
  }
  DrawTextXY(Text, XPos, YPos, X_IS_CENTER);

  //show inline badge
  if (!(ThemeX.BootCampStyle) &&
      (ThemeX.HideBadges & HDBADGES_INLINE) &&
      (Entries[ScrollState.CurrentSelection].Row == 0)) {
    // Display Inline Badge: small icon before the text
    XImage Back(BadgeDim, BadgeDim);
    INTN X = XPos - (TextWidth >> 1) - (BadgeDim + 16);
    INTN Y = YPos - ((BadgeDim - ThemeX.TextHeight) >> 1);
    Back.CopyRect(ThemeX.Background, X, Y);
    bool free = false;
    XImage *CurrSel = Entries[ScrollState.CurrentSelection].Image.GetBest(!Daylight, &free);
    Back.Compose(0, 0, *CurrSel, false, BadgeDim/128.f);
    Back.DrawOnBack(X, Y, Back);
    if (free) {
      delete CurrSel;
    }
  }

  OldX = XPos;
  OldY = YPos;
  OldTextWidth = TextWidth;
  OldRow = Entries[ScrollState.CurrentSelection].Row;
}

void REFIT_MENU_SCREEN::CountItems()
{
  row0PosX = 0;
  row1PosX = Entries.size();
  // layout
  row0Count = 0; //Nr items in row0
  row1Count = 0;
  for (INTN i = 0; i < (INTN)Entries.size(); i++) {
    if (Entries[i].Row == 0) {
      row0Count++;
      CONSTRAIN_MIN(row0PosX, i);
    } else {
      row1Count++;
      CONSTRAIN_MAX(row1PosX, i);
    }
  }
}

void REFIT_MENU_SCREEN::DrawTextCorner(UINTN TextC, UINT8 Align)
{
  INTN    Xpos;
//  CHAR16  *Text = NULL;
  XStringW Text;

  if (
      // HIDEUI_ALL - included
      ((TextC == TEXT_CORNER_REVISION) && ((ThemeX.HideUIFlags & HIDEUI_FLAG_REVISION) != 0)) ||
      ((TextC == TEXT_CORNER_HELP) && ((ThemeX.HideUIFlags & HIDEUI_FLAG_HELP) != 0)) ||
      ((TextC == TEXT_CORNER_OPTIMUS) && (gSettings.GUI.ShowOptimus == FALSE))
      ) {
    return;
  }

  switch (TextC) {
    case TEXT_CORNER_REVISION:
      // Display Clover boot volume
      if (SelfVolume->VolLabel.notEmpty()  &&  SelfVolume->VolLabel[0] != L'#') {
        Text = SWPrintf("%ls, booted from %ls %ls", gFirmwareRevision, SelfVolume->VolLabel.wc_str(), self.getCloverDirFullPath().wc_str());
      }
      if (Text.isEmpty()) {
        Text = SWPrintf("%ls %ls %ls", gFirmwareRevision, SelfVolume->VolName.wc_str(), self.getCloverDirFullPath().wc_str());
      }
      break;
    case TEXT_CORNER_HELP:
      Text = L"F1:Help"_XSW;
      break;
    case TEXT_CORNER_OPTIMUS:
      if (gConf.GfxPropertiesArray.size() > 0 && gConf.GfxPropertiesArray[0].Vendor != Intel) {
        Text = L"Discrete"_XSW;
      } else {
        Text = L"Intel"_XSW;
      }
      //      Text = (gConf.GfxPropertiesArray.size() == 2)?L"Intel":L"Discrete";
      break;
    default:
      return;
  }

  switch (Align) {
    case X_IS_LEFT:
      Xpos = (INTN)(ThemeX.TextHeight * 0.75f);
      break;
    case X_IS_RIGHT:
      Xpos = UGAWidth - (INTN)(ThemeX.TextHeight * 0.75f);//2
      break;
    case X_IS_CENTER:
      Xpos = UGAWidth >> 1;
      break;
    default:
      Text.setEmpty();
      return;
  }
  //  DBG("draw text %ls at (%d, %d)\n", Text, Xpos, UGAHeight - 5 - TextHeight),
  // clovy  DrawTextXY(Text, Xpos, UGAHeight - 5 - TextHeight, Align);
  DrawTextXY(Text, Xpos, UGAHeight - (INTN)(ThemeX.TextHeight * 1.5f), Align);
}

void REFIT_MAINMENU_SCREEN::DrawMainMenuEntry(REFIT_ABSTRACT_MENU_ENTRY *Entry, BOOLEAN selected, INTN XPos, INTN YPos)
{
  INTN MainSize = ThemeX.MainEntriesSize;
//  XImage MainImage(MainSize, MainSize);
//  XImage* BadgeImage;
  XIcon MainIcon;  //it can be changed here
  XIcon* BadgeIcon = NULL;

  if (Entry->Row == 0 && Entry->getDriveImage()  &&  !(ThemeX.HideBadges & HDBADGES_SWAP)) {
    MainIcon = *Entry->getDriveImage();
  } else {
    MainIcon = Entry->Image; // XIcon*
  }
  //this should be inited by the Theme
  if (MainIcon.isEmpty()) {
 //   DBG(" why MainImage is empty? Report to devs\n");
    if (!ThemeX.IsEmbeddedTheme()) {
      MainIcon = ThemeX.GetIcon("os_mac"_XS8);
    }
    if (MainIcon.Image.isEmpty()) {
      MainIcon.Image.DummyImage(MainSize);
      MainIcon.setFilled();
    }
  }

//  const XImage& MainImage = (!ThemeX.Daylight && !MainIcon.ImageNight.isEmpty())? MainIcon.ImageNight : MainIcon.Image;
  bool free = false;
  XImage *MainImage = MainIcon.GetBest(!Daylight, &free);

  INTN CompWidth = (Entry->Row == 0) ? ThemeX.row0TileSize : ThemeX.row1TileSize;
  INTN CompHeight = CompWidth;

//  float fScale;
//  if (ThemeX.TypeSVG) {
//    fScale = (selected ? 1.f : -1.f);
//  } else {
//    fScale = ((Entry->Row == 0) ? (ThemeX.MainEntriesSize/128.f * (selected ? 1.f : -1.f)): 1.f) ;
//  }

  if (Entry->Row == 0) {
    BadgeIcon = Entry->getBadgeImage();
  }

  const XImage& TopImage = ThemeX.SelectionImages[((Entry->Row == 0) ? 0 : 2) + (selected ? 0 : 1)];
//    DBG("   SelectionWidth=%lld\n", TopImage.GetWidth());
  if (TopImage.GetWidth() > CompWidth) {
    CompWidth = TopImage.GetWidth();
    CompHeight = CompWidth;
  }
  XImage Back(CompWidth, CompHeight);
  Back.CopyRect(ThemeX.Background, XPos, YPos);

  INTN OffsetX = (CompWidth - MainImage->GetWidth()) / 2;
  OffsetX = (OffsetX > 0) ? OffsetX: 0;
  INTN OffsetY = (CompHeight - MainImage->GetHeight()) / 2;
  OffsetY = (OffsetY > 0) ? OffsetY: 0;

  INTN OffsetTX = (CompWidth - TopImage.GetWidth()) / 2;
  OffsetTX = (OffsetTX > 0) ? OffsetTX: 0;
  INTN OffsetTY = (CompHeight - TopImage.GetHeight()) / 2;
  OffsetTY = (OffsetTY > 0) ? OffsetTY: 0;

//  DBG("  Comp=[%lld,%lld], offset=[%lld,%lld]\n", CompWidth, CompHeight, OffsetX, OffsetY);

  float composeScale = (ThemeX.NonSelectedGrey && !selected)? -1.f: 1.f;
  if(ThemeX.SelectionOnTop) {
    //place main image in centre. It may be OS or Drive
    Back.Compose(OffsetX, OffsetY, *MainImage, false, composeScale);
  } else {
    Back.Compose(OffsetTX, OffsetTY, TopImage, false); //selection first
    Back.Compose(OffsetX, OffsetY, *MainImage, false, composeScale);
  }
  
  Entry->Place.XPos = XPos;
  Entry->Place.YPos = YPos;
  Entry->Place.Width = MainImage->GetWidth();
  Entry->Place.Height = MainImage->GetHeight();

  if (free) {
    delete MainImage;
  }
  // place the badge image
  float fBadgeScale = ThemeX.BadgeScale/16.f;
  if ((Entry->Row == 0) && BadgeIcon && !BadgeIcon->isEmpty()) {
//    const XImage& BadgeImage = (!ThemeX.Daylight && !BadgeIcon->ImageNight.isEmpty()) ? &BadgeIcon->ImageNight : BadgeImage = &BadgeIcon->Image;
    free = false;
    XImage* BadgeImage = BadgeIcon->GetBest(!Daylight, &free);
    INTN BadgeWidth = (INTN)(BadgeImage->GetWidth() * fBadgeScale);
    INTN BadgeHeight = (INTN)(BadgeImage->GetHeight() * fBadgeScale);
    
    if ((BadgeWidth + 8) < CompWidth && (BadgeHeight + 8) < CompHeight) {
      
      // Check for user badge x offset from theme.plist
      if (ThemeX.BadgeOffsetX != 0xFFFF) {
        OffsetX += ThemeX.BadgeOffsetX;
      } else {
        // Set default position
        OffsetX += CompWidth  - 8 - BadgeWidth;
      }
      // Check for user badge y offset from theme.plist
      if (ThemeX.BadgeOffsetY != 0xFFFF) {
        OffsetY += ThemeX.BadgeOffsetY;
      } else {
        // Set default position
        OffsetY += CompHeight - 8 - BadgeHeight;
      }
 //     DBG("  badge offset=[%lld,%lld]\n", OffsetX, OffsetY);
      Back.Compose(OffsetX, OffsetY, *BadgeImage, false, fBadgeScale);
      if (free) delete BadgeImage;
    }
  }

  if(ThemeX.SelectionOnTop) {
    Back.Compose(OffsetTX, OffsetTY, TopImage, false); //selection at the top
  }
  Back.DrawWithoutCompose(XPos, YPos);


  // draw BCS indicator
  // Needy: if Labels (Titles) are hidden there is no point to draw the indicator
  if (ThemeX.BootCampStyle && !(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL)) {
    // indicator is for row 0, main entries, only
    if (Entry->Row == 0) {
      const XImage& SelImage = ThemeX.SelectionImages[4 + (selected ? 0 : 1)];
      XPos = XPos + (ThemeX.row0TileSize / 2) - (INTN)(INDICATOR_SIZE * 0.5f * ThemeX.Scale);
      YPos = row0PosY + ThemeX.row0TileSize + ThemeX.TextHeight + (INTN)((BCSMargin * 2) * ThemeX.Scale);
      CompWidth = (INTN)(INDICATOR_SIZE * ThemeX.Scale);
      CompHeight = (INTN)(INDICATOR_SIZE * ThemeX.Scale);
      Back = XImage(CompWidth, CompHeight);
      Back.CopyRect(ThemeX.Background, XPos, YPos);
      Back.Compose(0, 0, SelImage, false);
      Back.DrawWithoutCompose(XPos, YPos);
    }
  }

}


/**
 * Main screen text.
 */
void REFIT_MAINMENU_SCREEN::MainMenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText)
{
  EFI_STATUS Status = EFI_SUCCESS;
//  INTN i = 0;
  INTN MessageHeight = 0;
// clovy
  if (ThemeX.TypeSVG && textFace[1].valid) {
    MessageHeight = (INTN)(textFace[1].size * RowHeightFromTextHeight * ThemeX.Scale);
  } else {
    MessageHeight = (INTN)(ThemeX.TextHeight * RowHeightFromTextHeight * ThemeX.Scale);
  }

  switch (Function) {

    case MENU_FUNCTION_INIT:
      egGetScreenSize(&UGAWidth, &UGAHeight);
      InitAnime();
      SwitchToGraphicsAndClear();
      //BltClearScreen(FALSE);

      EntriesGap = (int)(ThemeX.TileXSpace * ThemeX.Scale);
      EntriesWidth = ThemeX.row0TileSize;
      EntriesHeight = ThemeX.MainEntriesSize + (int)(16.f * ThemeX.Scale);

      MaxItemOnScreen = (UGAWidth - (int)((ROW0_SCROLLSIZE * 2)* ThemeX.Scale)) / (EntriesWidth + EntriesGap); //8
      CountItems();
      InitScroll(row0Count, Entries.size(), MaxItemOnScreen, 0);

      row0PosX = EntriesWidth + EntriesGap;
      row0PosX = row0PosX * ((MaxItemOnScreen < row0Count)?MaxItemOnScreen:row0Count);
      row0PosX = row0PosX - EntriesGap;
      row0PosX = UGAWidth - row0PosX;
      row0PosX = row0PosX >> 1;

      row0PosY = (int)(((float)UGAHeight - ThemeX.LayoutHeight * ThemeX.Scale) * 0.5f +
                  ThemeX.LayoutBannerOffset * ThemeX.Scale);

      row1PosX = (UGAWidth + 8 - (ThemeX.row1TileSize + (INTN)(8.0f * ThemeX.Scale)) * row1Count) >> 1;

      if (ThemeX.BootCampStyle && !(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL)) {
        row1PosY = row0PosY + ThemeX.row0TileSize + (INTN)((BCSMargin * 2) * ThemeX.Scale) + ThemeX.TextHeight +
            (INTN)(INDICATOR_SIZE * ThemeX.Scale) +
            (INTN)((ThemeX.LayoutButtonOffset + ThemeX.TileYSpace) * ThemeX.Scale);
      } else {
        row1PosY = row0PosY + EntriesHeight +
            (INTN)((ThemeX.TileYSpace + ThemeX.LayoutButtonOffset) * ThemeX.Scale);
      }

      if (row1Count > 0) {
          textPosY = row1PosY + MAX(ThemeX.row1TileSize, MessageHeight) + (INTN)((ThemeX.TileYSpace + ThemeX.LayoutTextOffset) * ThemeX.Scale);
        } else {
          textPosY = row1PosY;
        }

      if (ThemeX.BootCampStyle) {
        textPosY = row0PosY + ThemeX.row0TileSize + (INTN)((TEXT_YMARGIN + BCSMargin) * ThemeX.Scale);
      }

      FunctextPosY = row1PosY + ThemeX.row1TileSize + (INTN)((ThemeX.TileYSpace + ThemeX.LayoutTextOffset) * ThemeX.Scale);

      if (!itemPosX) {
        itemPosX = (__typeof__(itemPosX))AllocatePool(sizeof(UINT64) * Entries.size());
      }

      row0PosXRunning = row0PosX;
      row1PosXRunning = row1PosX;
      //DBG("EntryCount =%d\n", Entries.size());
      for (INTN i = 0; i < (INTN)Entries.size(); i++) {
        if (Entries[i].Row == 0) {
          itemPosX[i] = row0PosXRunning;
          row0PosXRunning += EntriesWidth + EntriesGap;
        } else {
          itemPosX[i] = row1PosXRunning;
          row1PosXRunning += ThemeX.row1TileSize + (INTN)(TILE1_XSPACING * ThemeX.Scale);
          //DBG("next item in row1 at x=%d\n", row1PosXRunning);
        }
      }
      // initial painting
 //     ThemeX.InitSelection(); //not needed to do here

      // Update FilmPlace only if not set by InitAnime
      if (FilmC->FilmPlace.Width == 0 || FilmC->FilmPlace.Height == 0) {
//        CopyMem(&FilmPlace, &BannerPlace, sizeof(BannerPlace));
        FilmC->FilmPlace = ThemeX.BannerPlace;
      }

      //DBG("main menu inited\n");
      break;

    case MENU_FUNCTION_CLEANUP:
      FreePool(itemPosX);
      itemPosX = NULL;
      HidePointer();
      break;

    case MENU_FUNCTION_PAINT_ALL:

      for (INTN i = 0; i <= ScrollState.MaxIndex; i++) {
        if (Entries[i].Row == 0) {
          if ((i >= ScrollState.FirstVisible) && (i <= ScrollState.LastVisible)) {
            DrawMainMenuEntry(&Entries[i], (i == ScrollState.CurrentSelection)?1:0,
                              itemPosX[i - ScrollState.FirstVisible], row0PosY);
            // draw static text for the boot options, BootCampStyle

            if (ThemeX.BootCampStyle && !(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL)) {
              INTN textPosX = itemPosX[i - ScrollState.FirstVisible] + (ThemeX.row0TileSize / 2);
              // clear the screen

              ThemeX.FillRectAreaOfScreen(textPosX, textPosY, EntriesWidth + ThemeX.TileXSpace,
                                   MessageHeight);
              DrawBCSText(Entries[i].Title.wc_str(), textPosX, textPosY, X_IS_CENTER);
            }
          }
        } else {
          DrawMainMenuEntry(&Entries[i], (i == ScrollState.CurrentSelection)?1:0,
                            itemPosX[i], row1PosY);
        }
      }

      // clear the text from the second row, required by the BootCampStyle
      if ((ThemeX.BootCampStyle) && (Entries[ScrollState.LastSelection].Row == 1)
          && (Entries[ScrollState.CurrentSelection].Row == 0) && !(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL)) {
        ThemeX.FillRectAreaOfScreen((UGAWidth >> 1), FunctextPosY,
                             OldTextWidth, MessageHeight);
      }

      if ((Entries[ScrollState.LastSelection].Row == 0) && (Entries[ScrollState.CurrentSelection].Row == 1)
          && ThemeX.BootCampStyle && !(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL)) {
        DrawMainMenuLabel(Entries[ScrollState.CurrentSelection].Title,
                          (UGAWidth >> 1), FunctextPosY);
      }
      if (!(ThemeX.BootCampStyle) && !(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL)) {
        DrawMainMenuLabel(Entries[ScrollState.CurrentSelection].Title,
                          (UGAWidth >> 1), textPosY);
      }
//      DBG("draw TEXT_CORNER_HELP\n");
      DrawTextCorner(TEXT_CORNER_HELP, X_IS_LEFT);
//      DBG("draw TEXT_CORNER_OPTIMUS\n");
      DrawTextCorner(TEXT_CORNER_OPTIMUS, X_IS_CENTER);
//      DBG("draw TEXT_CORNER_REVISION\n");
      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_RIGHT);
//      DBG("MouseBirth\n");
      Status = MouseBirth();
      if(EFI_ERROR(Status)) {
        DBG("can't bear mouse at all! Status=%s\n", efiStrError(Status));
      }
      break;

    case MENU_FUNCTION_PAINT_SELECTION:
      HidePointer();
      if (Entries[ScrollState.LastSelection].Row == 0) {
        DrawMainMenuEntry(&Entries[ScrollState.LastSelection], FALSE,
                      itemPosX[ScrollState.LastSelection - ScrollState.FirstVisible], row0PosY);
      } else {
        DrawMainMenuEntry(&Entries[ScrollState.LastSelection], FALSE,
                          itemPosX[ScrollState.LastSelection], row1PosY);
      }

      if (Entries[ScrollState.CurrentSelection].Row == 0) {
        DrawMainMenuEntry(&Entries[ScrollState.CurrentSelection], TRUE,
                      itemPosX[ScrollState.CurrentSelection - ScrollState.FirstVisible], row0PosY);
      } else {
        DrawMainMenuEntry(&Entries[ScrollState.CurrentSelection], TRUE,
                          itemPosX[ScrollState.CurrentSelection], row1PosY);
      }

      if ((ThemeX.BootCampStyle) && (!(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL))
          && Entries[ScrollState.CurrentSelection].Row == 1) {
        DrawMainMenuLabel(Entries[ScrollState.CurrentSelection].Title,
                          (UGAWidth >> 1), FunctextPosY);
      }
      if ((!(ThemeX.BootCampStyle)) && (!(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL))) {
        DrawMainMenuLabel(Entries[ScrollState.CurrentSelection].Title,
                          (UGAWidth >> 1), textPosY);
      }

      DrawTextCorner(TEXT_CORNER_HELP, X_IS_LEFT);
      DrawTextCorner(TEXT_CORNER_OPTIMUS, X_IS_CENTER);
      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_RIGHT);
      Status = MouseBirth();
      if(EFI_ERROR(Status)) {
        DBG("can't bear mouse at sel! Status=%s\n", efiStrError(Status));
      }
      break;

    case MENU_FUNCTION_PAINT_TIMEOUT:
      INTN hi = MessageHeight * ((ThemeX.HideBadges & HDBADGES_INLINE)?3:1);
      HidePointer();
      if (!(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL)){
        ThemeX.FillRectAreaOfScreen((UGAWidth >> 1), FunctextPosY + hi,
                             OldTimeoutTextWidth, MessageHeight);
        XStringW TextX;
        TextX.takeValueFrom(ParamText);
        OldTimeoutTextWidth = DrawTextXY(TextX, (UGAWidth >> 1), FunctextPosY + hi, X_IS_CENTER);
      }

      DrawTextCorner(TEXT_CORNER_HELP, X_IS_LEFT);
      DrawTextCorner(TEXT_CORNER_OPTIMUS, X_IS_CENTER);
      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_RIGHT);
      Status = MouseBirth();
      if(EFI_ERROR(Status)) {
        DBG("can't bear mouse at timeout! Status=%s\n", efiStrError(Status));
      }
      break;

  }
}

void REFIT_MAINMENU_SCREEN::MainMenuVerticalStyle(IN UINTN Function, IN CONST CHAR16 *ParamText)
{
//  INTN i;
//  INTN row0PosYRunning;
//  INTN VisibleHeight = 0; //assume vertical layout

  switch (Function) {

    case MENU_FUNCTION_INIT:
    {
      egGetScreenSize(&UGAWidth, &UGAHeight); //do this when needed
      InitAnime();
      SwitchToGraphicsAndClear();
      //BltClearScreen(FALSE);
      //adjustable by theme.plist?
      EntriesPosY = (int)(LAYOUT_Y_EDGE * ThemeX.Scale);
      EntriesGap = (int)(ThemeX.TileYSpace * ThemeX.Scale);
      EntriesWidth = ThemeX.MainEntriesSize + (int)(16 * ThemeX.Scale);
      EntriesHeight = ThemeX.MainEntriesSize + (int)(16 * ThemeX.Scale);
      //
      INTN VisibleHeight = (UGAHeight - EntriesPosY - (int)(LAYOUT_Y_EDGE * ThemeX.Scale) + EntriesGap) / (EntriesHeight + EntriesGap);
      EntriesPosX = UGAWidth - EntriesWidth - (int)((BAR_WIDTH + LAYOUT_X_EDGE) * ThemeX.Scale);
      INTN MessageHeight = 20;
      if (ThemeX.TypeSVG && textFace[1].valid) {
        MessageHeight = (INTN)(textFace[1].size * RowHeightFromTextHeight * ThemeX.Scale);
      } else {
        MessageHeight = (INTN)(ThemeX.TextHeight * RowHeightFromTextHeight * ThemeX.Scale);
      }
      TimeoutPosY = UGAHeight - (int)(LAYOUT_Y_EDGE * ThemeX.Scale) - MessageHeight * 2; //optimus + timeout texts

      CountItems();
      InitScroll(row0Count, Entries.size(), VisibleHeight, 0);
      row0PosX = EntriesPosX;
      row0PosY = EntriesPosY;
      row1PosX = (UGAWidth + EntriesGap - (ThemeX.row1TileSize + (int)(TILE1_XSPACING * ThemeX.Scale)) * row1Count) >> 1;
      textPosY = TimeoutPosY - (int)(ThemeX.TileYSpace * ThemeX.Scale) - MessageHeight; //message text
      row1PosY = textPosY - ThemeX.row1TileSize - (int)(ThemeX.TileYSpace * ThemeX.Scale) - ThemeX.LayoutTextOffset;
      if (!itemPosX) {
        itemPosX = (__typeof__(itemPosX))AllocatePool(sizeof(UINT64) * Entries.size());
        itemPosY = (__typeof__(itemPosY))AllocatePool(sizeof(UINT64) * Entries.size());
      }
      INTN row0PosYRunning = row0PosY;
      row1PosXRunning = row1PosX;

      //     DBG("EntryCount =%d\n", Entries.size());
      for (INTN i = 0; i < (INTN)Entries.size(); i++) {
        if (Entries[i].Row == 0) {
          itemPosX[i] = row0PosX;
          itemPosY[i] = row0PosYRunning;
          row0PosYRunning += EntriesHeight + EntriesGap;
        } else {
          itemPosX[i] = row1PosXRunning;
          itemPosY[i] = row1PosY;
          row1PosXRunning += ThemeX.row1TileSize + (int)(ThemeX.TileXSpace * ThemeX.Scale);
          //         DBG("next item in row1 at x=%d\n", row1PosXRunning);
        }
      }

      // Update FilmPlace only if not set by InitAnime
      if (FilmC->FilmPlace.Width == 0 || FilmC->FilmPlace.Height == 0) {
        FilmC->FilmPlace = ThemeX.BannerPlace;
      }

      ThemeX.InitBar(); //not sure
      break;
    }
    case MENU_FUNCTION_CLEANUP:
      FreePool(itemPosX);
      itemPosX = NULL;
      FreePool(itemPosY);
      itemPosY = NULL;
      HidePointer();
      break;

    case MENU_FUNCTION_PAINT_ALL:
      SetBar(EntriesPosX + EntriesWidth + (int)(10 * ThemeX.Scale),
             EntriesPosY, UGAHeight - (int)(LAYOUT_Y_EDGE * ThemeX.Scale), &ScrollState);
      for (INTN i = 0; i <= ScrollState.MaxIndex; i++) {
        if (Entries[i].Row == 0) {
          if ((i >= ScrollState.FirstVisible) && (i <= ScrollState.LastVisible)) {
            DrawMainMenuEntry(&Entries[i], (i == ScrollState.CurrentSelection)?1:0,
                              itemPosX[i - ScrollState.FirstVisible], itemPosY[i - ScrollState.FirstVisible]);
          }
        } else { //row1
          DrawMainMenuEntry(&Entries[i], (i == ScrollState.CurrentSelection)?1:0,
                            itemPosX[i], itemPosY[i]);
        }
      }
      if (!(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL)){
        DrawMainMenuLabel(Entries[ScrollState.CurrentSelection].Title,
                          (UGAWidth >> 1), textPosY);
      }

      ScrollingBar(); //&ScrollState);
      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_LEFT);
      DrawTextCorner(TEXT_CORNER_OPTIMUS, X_IS_CENTER);
      MouseBirth();
      break;

    case MENU_FUNCTION_PAINT_SELECTION:
      HidePointer();
      if (Entries[ScrollState.LastSelection].Row == 0) {
        DrawMainMenuEntry(&Entries[ScrollState.LastSelection], FALSE,
                          itemPosX[ScrollState.LastSelection - ScrollState.FirstVisible],
                          itemPosY[ScrollState.LastSelection - ScrollState.FirstVisible]);
      } else {
        DrawMainMenuEntry(&Entries[ScrollState.LastSelection], FALSE,
                          itemPosX[ScrollState.LastSelection],
                          itemPosY[ScrollState.LastSelection]);
      }

      if (Entries[ScrollState.CurrentSelection].Row == 0) {
        DrawMainMenuEntry(&Entries[ScrollState.CurrentSelection], TRUE,
                          itemPosX[ScrollState.CurrentSelection - ScrollState.FirstVisible],
                          itemPosY[ScrollState.CurrentSelection - ScrollState.FirstVisible]);
      } else {
        DrawMainMenuEntry(&Entries[ScrollState.CurrentSelection], TRUE,
                          itemPosX[ScrollState.CurrentSelection],
                          itemPosY[ScrollState.CurrentSelection]);
      }
      if (!(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL)) {
        DrawMainMenuLabel(Entries[ScrollState.CurrentSelection].Title,
                          (UGAWidth >> 1), textPosY);
      }

      ScrollingBar(); //&ScrollState);
      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_LEFT);
      DrawTextCorner(TEXT_CORNER_OPTIMUS, X_IS_CENTER);
      MouseBirth();
      break;

    case MENU_FUNCTION_PAINT_TIMEOUT:
      INTN MessageHeight = 20;
      if (ThemeX.TypeSVG && textFace[1].valid) {
        MessageHeight = (INTN)(textFace[1].size * RowHeightFromTextHeight * ThemeX.Scale);
      } else {
        MessageHeight = (INTN)(ThemeX.TextHeight * RowHeightFromTextHeight * ThemeX.Scale);
      }
      INTN hi = MessageHeight * ((ThemeX.HideBadges & HDBADGES_INLINE)?3:1);
      HidePointer();
      if (!(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL)) {
        ThemeX.FillRectAreaOfScreen((UGAWidth >> 1), textPosY + hi,
                             OldTimeoutTextWidth, ThemeX.TextHeight);
        XStringW TextX;
        TextX.takeValueFrom(ParamText);
        OldTimeoutTextWidth = DrawTextXY(TextX, (UGAWidth >> 1), textPosY + hi, X_IS_CENTER);
      }

      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_LEFT);
      break;

  }
}

UINTN REFIT_MAINMENU_SCREEN::RunMainMenu(IN INTN DefaultSelection, OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry)
{

//  MENU_STYLE_FUNC     Style             = &REFIT_MENU_SCREEN::TextMenuStyle;
//  MENU_STYLE_FUNC     MainStyle         = &REFIT_MENU_SCREEN::TextMenuStyle;

  REFIT_ABSTRACT_MENU_ENTRY    *TempChosenEntry  = 0;
  REFIT_ABSTRACT_MENU_ENTRY    *MainChosenEntry  = 0;
  REFIT_ABSTRACT_MENU_ENTRY    *NextChosenEntry  = NULL;
  UINTN               MenuExit = 0, SubMenuExit = 0;
  INTN                DefaultEntryIndex = DefaultSelection;
  INTN                SubMenuIndex;

  // initialize static variables when menu runs so that values from previos sessions won't be used
  OldX = 0;
  OldY = 0;
  OldTextWidth = 0;
  OldTextHeight = 0;
  OldRow = 0;
  OldTimeoutTextWidth = 0;

//  if (AllowGraphicsMode) {
////    Style = &REFIT_MENU_SCREEN::GraphicsMenuStyle;
//    if (ThemeX.VerticalLayout) {
//      m_MainStyle = &REFIT_MAINMENU_SCREEN::MainMenuVerticalStyle;
//    } else {
//      m_MainStyle = &REFIT_MAINMENU_SCREEN::MainMenuStyle;
//    }
//  }else{
//    m_MainStyle = &REFIT_MAINMENU_SCREEN::TextMenuStyle;
//  }

  while (!MenuExit) {
    GetAnime();
    DBG("AnimeRun=%d\n", (FilmC && FilmC->AnimeRun)?1:0);
    MenuExit = RunGenericMenu(&DefaultEntryIndex, &MainChosenEntry);
    TimeoutSeconds = 0;

    if (MenuExit == MENU_EXIT_DETAILS && MainChosenEntry->SubScreen != NULL) {
      if ( MainChosenEntry->SubScreen->Entries.size() > 0 ) { // if MainChosenEntry->SubScreen->Entries.size() == 0, we got a crash in GraphicsMenuStyle
        XString8Array TmpArgs;
        if ( gSettings.Boot.BootArgs.length() > 0) {
          TmpArgs = Split<XString8Array>(gSettings.Boot.BootArgs, " ");
        }
        SubMenuIndex = -1;

        GlobalConfig.OptionsBits = EncodeOptions(TmpArgs);
  //      DBG("main OptionsBits = 0x%X\n", GlobalConfig.OptionsBits);

        if (MainChosenEntry->getLOADER_ENTRY()) {
          GlobalConfig.OptionsBits |= EncodeOptions(MainChosenEntry->getLOADER_ENTRY()->LoadOptions);
  //        DBG("add OptionsBits = 0x%X\n", GlobalConfig.OptionsBits);
        }

        if (MainChosenEntry->getREFIT_MENU_ITEM_BOOTNUM()) {
          DecodeOptions(MainChosenEntry->getREFIT_MENU_ITEM_BOOTNUM());
        }
  //      DBG(" enter menu with LoadOptions: %ls\n", ((LOADER_ENTRY*)MainChosenEntry)->LoadOptions);

        if (MainChosenEntry->getLOADER_ENTRY()) {
          // Only for non-legacy entries, as LEGACY_ENTRY doesn't have Flags
          GlobalConfig.FlagsBits = MainChosenEntry->getLOADER_ENTRY()->Flags;
        }
  //      DBG(" MainChosenEntry with FlagsBits = 0x%X\n", GlobalConfig.FlagsBits);

        SubMenuExit = 0;
        while (!SubMenuExit) {
          //
          //running details menu
          //
          SubMenuExit = MainChosenEntry->SubScreen->RunGenericMenu(&SubMenuIndex, &TempChosenEntry);

          if (SubMenuExit == MENU_EXIT_ESCAPE || TempChosenEntry->getREFIT_MENU_ITEM_RETURN() ) {
            SubMenuExit = MENU_EXIT_ENTER;
            MenuExit = 0;
            break;
          }

          if (MainChosenEntry->getREFIT_MENU_ENTRY_CLOVER()) {
            MainChosenEntry->getREFIT_MENU_ENTRY_CLOVER()->LoadOptions = (((REFIT_MENU_ENTRY_CLOVER*)TempChosenEntry)->LoadOptions);
          }

          if (SubMenuExit == MENU_EXIT_DETAILS) {
            SubMenuExit = 0;
            continue;
          }
   //       DBG(" exit menu with LoadOptions: %ls\n", ((LOADER_ENTRY*)MainChosenEntry)->LoadOptions);

          if (SubMenuExit == MENU_EXIT_ENTER && MainChosenEntry->getLOADER_ENTRY() && TempChosenEntry->getLOADER_ENTRY()) {
            // Only for non-legacy entries, as LEGACY_ENTRY doesn't have Flags/Options
            MainChosenEntry->getLOADER_ENTRY()->Flags = TempChosenEntry->getLOADER_ENTRY()->Flags;
            DBG(" get MainChosenEntry FlagsBits = 0x%X\n", ((LOADER_ENTRY*)MainChosenEntry)->Flags);
            if (OSFLAG_ISUNSET(TempChosenEntry->getLOADER_ENTRY()->Flags, OSFLAG_NODEFAULTARGS)) {
              DecodeOptions(TempChosenEntry->getLOADER_ENTRY());
  //            DBG("get OptionsBits = 0x%X\n", GlobalConfig.OptionsBits);
  //            DBG(" TempChosenEntry FlagsBits = 0x%X\n", ((LOADER_ENTRY*)TempChosenEntry)->Flags);
            }
            // copy also loadoptions from subentry to mainentry
            MainChosenEntry->getLOADER_ENTRY()->LoadOptions = TempChosenEntry->getLOADER_ENTRY()->LoadOptions;
          }

          if (/*MenuExit == MENU_EXIT_ENTER &&*/ TempChosenEntry->getLOADER_ENTRY()) {
            if (TempChosenEntry->getLOADER_ENTRY()->LoadOptions.notEmpty()) {
              gSettings.Boot.BootArgs = TempChosenEntry->getLOADER_ENTRY()->LoadOptions.ConcatAll(" "_XS8);
            } else {
              gSettings.Boot.BootArgs.setEmpty();
            }
            DBG(" boot with args: %s\n", gSettings.Boot.BootArgs.c_str());
          }

          //---- Details submenu (kexts disabling etc)
          if (SubMenuExit == MENU_EXIT_ENTER /*|| MenuExit == MENU_EXIT_DETAILS*/) {
            if (TempChosenEntry->SubScreen != NULL) {
              UINTN NextMenuExit = 0;
              INTN NextEntryIndex = -1;
              while (!NextMenuExit) {
                //
                // running submenu
                //
                NextMenuExit = TempChosenEntry->SubScreen->RunGenericMenu(&NextEntryIndex, &NextChosenEntry);
                if (NextMenuExit == MENU_EXIT_ESCAPE || NextChosenEntry->getREFIT_MENU_ITEM_RETURN() ) {
                  SubMenuExit = 0;
                  NextMenuExit = MENU_EXIT_ENTER;
                  break;
                }
                DBG(" get NextChosenEntry FlagsBits = 0x%X\n", ((LOADER_ENTRY*)NextChosenEntry)->Flags);
                //---- Details submenu (kexts disabling etc) second level
                if (NextMenuExit == MENU_EXIT_ENTER /*|| MenuExit == MENU_EXIT_DETAILS*/) {
                  if (NextChosenEntry->SubScreen != NULL) {
                    UINTN DeepMenuExit = 0;
                    INTN DeepEntryIndex = -1;
                    REFIT_ABSTRACT_MENU_ENTRY    *DeepChosenEntry  = NULL;
                    while (!DeepMenuExit) {
                      //
                      // run deep submenu
                      //
                      DeepMenuExit = NextChosenEntry->SubScreen->RunGenericMenu(&DeepEntryIndex, &DeepChosenEntry);
                      if (DeepMenuExit == MENU_EXIT_ESCAPE || DeepChosenEntry->getREFIT_MENU_ITEM_RETURN() ) {
                        DeepMenuExit = MENU_EXIT_ENTER;
                        NextMenuExit = 0;
                        break;
                      }
                      DBG(" get DeepChosenEntry FlagsBits = 0x%X\n", ((LOADER_ENTRY*)DeepChosenEntry)->Flags);
                    } //while(!DeepMenuExit)
                  }
                }

              } //while(!NextMenuExit)
            }
          }
          //---------
        }
      }else{
        // Here, it means MainChosenEntry->SubScreen != null, but MainChosenEntry->SubScreen->Entries.size() == 0.
        // This is a technical bug. GraphicsMenuStyle would crash.
        #ifdef DEBUG
          panic("A sub menu doesn't have any entries");
        #else
          MenuExit = 0; // loop on main menu
        #endif
      }
    }
  }

  if (ChosenEntry) {
    *ChosenEntry = MainChosenEntry;
  }
  return MenuExit;
}

