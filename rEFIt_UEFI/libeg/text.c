/*
 * libeg/text.c
 * Text drawing functions
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

#include "egemb_font.h"
//#define FONT_CELL_WIDTH (7)
//#define FONT_CELL_HEIGHT (12)

#ifndef DEBUG_ALL
#define DEBUG_TEXT 1
#else
#define DEBUG_TEXT DEBUG_ALL
#endif

#if DEBUG_TEXT == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_TEXT, __VA_ARGS__)
#endif


EG_IMAGE *FontImage = NULL;
INTN FontWidth = 7;
INTN FontHeight = 12;
INTN TextHeight;

//
// Text rendering
//

VOID egMeasureText(IN CHAR16 *Text, OUT INTN *Width, OUT INTN *Height)
{
    if (Width != NULL)
        *Width = StrLen(Text) * GlobalConfig.CharWidth;
    if (Height != NULL)
        *Height = FontHeight;
}

EG_IMAGE * egLoadFontImage(IN BOOLEAN FromTheme, IN INTN Rows, IN INTN Cols)
{
  EG_IMAGE            *NewImage;
  EG_IMAGE            *NewFontImage;
//  UINTN     FontWidth;  //using global variables
//  UINTN     FontHeight;
  INTN        ImageWidth, ImageHeight;
  INTN        x, y, Ypos, j;
  EG_PIXEL    *PixelPtr;
  EG_PIXEL    FirstPixel;
  BOOLEAN     WantAlpha = TRUE;
  
  if (!ThemeDir) {
    GlobalConfig.Font = FONT_GRAY;
    return NULL;
  }

  if (FromTheme) {
    NewImage = egLoadImage(ThemeDir, GlobalConfig.FontFileName, WantAlpha);
  } else {
    NewImage = egLoadImage(ThemeDir, L"FontKorean.png", WantAlpha);
  }

  if (NewImage) {
    if (FromTheme) {
      DBG("font %s loaded from themedir\n", GlobalConfig.FontFileName);
    } else {
      DBG("Korean font loaded from themedir\n");
    }
  } else {
    CHAR16 *commonFontDir = L"EFI\\CLOVER\\font";
    CHAR16 *fontFilePath = PoolPrint(L"%s\\%s", commonFontDir, GlobalConfig.FontFileName);
    NewImage = egLoadImage(SelfRootDir, fontFilePath, WantAlpha);
    if (!NewImage) {
      DBG("Font %s is not loaded, using default\n", fontFilePath);
      FreePool(fontFilePath);
      return NULL;
    }
    DBG("font %s loaded from common font dir %s\n", GlobalConfig.FontFileName, commonFontDir);
    FreePool(fontFilePath);
  }

  ImageWidth = NewImage->Width;
//  DBG("ImageWidth=%d\n", ImageWidth);
  ImageHeight = NewImage->Height;
//  DBG("ImageHeight=%d\n", ImageHeight);
  PixelPtr = NewImage->PixelData;
  DBG("Font loaded: ImageWidth=%d ImageHeight=%d\n", ImageWidth, ImageHeight);
  NewFontImage = egCreateImage(ImageWidth * Rows, ImageHeight / Rows, WantAlpha);
  if (NewFontImage == NULL) {
    DBG("Can't create new font image!\n");
    return NULL;
  }
  
  FontWidth = ImageWidth / Cols;
  FontHeight = ImageHeight / Rows;
  FirstPixel = *PixelPtr;
  for (y = 0; y < Rows; y++) {
    for (j = 0; j < FontHeight; j++) {
//      Ypos = MultU64x64(LShiftU64(j, 4) + y, ImageWidth);
      Ypos = ((j * Rows) + y) * ImageWidth;
      for (x = 0; x < ImageWidth; x++) {
       if (WantAlpha && 
           (PixelPtr->b == FirstPixel.b) &&
           (PixelPtr->g == FirstPixel.g) &&
           (PixelPtr->r == FirstPixel.r)
           ) {
          PixelPtr->a = 0;
        }
        NewFontImage->PixelData[Ypos + x] = *PixelPtr++;
      }
    }    
  }
  egFreeImage(NewImage);
  
  return NewFontImage;  
} 

VOID PrepareFont(VOID)
{
  BOOLEAN ChangeFont = FALSE;
//  EG_PIXEL *FontPixelData;
  EG_PIXEL *p;
  INTN      Width;
  INTN      Height;
  if (gLanguage == korean) {
//    FontImage = egLoadImage(ThemeDir, L"FontKorean.png", TRUE);
    FontImage = egLoadFontImage(FALSE, 10, 28);
    if (FontImage) {
      FontHeight = 16;
 //     if (GlobalConfig.CharWidth == 0) {
        GlobalConfig.CharWidth = 16;
 //     }
      FontWidth = GlobalConfig.CharWidth;
      TextHeight = FontHeight + TEXT_YMARGIN * 2;
      DBG("Using Korean font matrix\n");
      return;
    } else {
      gLanguage = english;
    }
  }

  // load the font
  if (FontImage == NULL){
    switch (GlobalConfig.Font) {
      case FONT_ALFA:
        ChangeFont = TRUE;
        FontImage = egPrepareEmbeddedImage(&egemb_font, TRUE);        
        break;
      case FONT_GRAY:
        ChangeFont = TRUE;
        FontImage = egPrepareEmbeddedImage(&egemb_font_gray, TRUE);
        break;
      case FONT_LOAD:
  //      DBG("load font image\n");
        FontImage = egLoadFontImage(TRUE, 16, 16);
        if (!FontImage) {
          ChangeFont = TRUE;
          GlobalConfig.Font = FONT_ALFA;
          FontImage = egPrepareEmbeddedImage(&egemb_font, TRUE);
          //invert the font
          p = FontImage->PixelData;
          for (Height = 0; Height < FontImage->Height; Height++){
            for (Width = 0; Width < FontImage->Width; Width++, p++){
              p->b ^= 0xFF;
              p->g ^= 0xFF;
              p->r ^= 0xFF;
      //        p->a = 0xFF;    //huh!          
            }
          }
        }
        break;
      default:
        FontImage = egPrepareEmbeddedImage(&egemb_font, TRUE);
        break;
    }    
  }
  if (ChangeFont) {
    // set default values
    GlobalConfig.CharWidth = 7;
    FontWidth = GlobalConfig.CharWidth;
    FontHeight = 12;
  }
  TextHeight = FontHeight + TEXT_YMARGIN * 2;
  DBG("Font %d prepared WxH=%dx%d CharWidth=%d\n", GlobalConfig.Font, FontWidth, FontHeight, GlobalConfig.CharWidth);
}

VOID egRenderText(IN CHAR16 *Text, IN OUT EG_IMAGE *CompImage,
                  IN INTN PosX, IN INTN PosY, IN INTN Cursor)
{
  EG_PIXEL        *BufferPtr;
  EG_PIXEL        *FontPixelData;
  INTN            BufferLineOffset, FontLineOffset;
  INTN            TextLength;
  INTN            i;
  UINT16          c, c1;
  UINTN           Shift = 0;
  UINTN           Cho = 0, Jong = 0, Joong = 0;
  
  // clip the text
  TextLength = StrLen(Text);
/*  if ((MultU64x64(TextLength, GlobalConfig.CharWidth) + PosX) > CompImage->Width){
    if (GlobalConfig.CharWidth) {
      TextLength = DivU64x32(CompImage->Width - PosX, GlobalConfig.CharWidth);
    } else
      TextLength = DivU64x32(CompImage->Width - PosX, FontWidth);
  }*/
  if ((TextLength * GlobalConfig.CharWidth + PosX) > CompImage->Width){
    if (GlobalConfig.CharWidth) {
      TextLength = (CompImage->Width - PosX) / GlobalConfig.CharWidth;
    } else
      TextLength = (CompImage->Width - PosX) / FontWidth;
  }
  if (!FontImage) {
    GlobalConfig.Font = FONT_LOAD;
    PrepareFont();
  }
  
//  DBG("TextLength =%d PosX=%d PosY=%d\n", TextLength, PosX, PosY);
  // render it
  BufferPtr = CompImage->PixelData;
  BufferLineOffset = CompImage->Width;
  BufferPtr += PosX + PosY * BufferLineOffset;
  FontPixelData = FontImage->PixelData;
  FontLineOffset = FontImage->Width;
  DBG("BufferLineOffset=%d  FontLineOffset=%d\n", BufferLineOffset, FontLineOffset);
  if (GlobalConfig.CharWidth < FontWidth) {
    Shift = (FontWidth - GlobalConfig.CharWidth) >> 1;
  }
  for (i = 0; i < TextLength; i++) {
    c = Text[i];
    if (gLanguage != korean) {
      if (GlobalConfig.Font != FONT_LOAD) {
        if (c < 0x20 || c >= 0x7F)
          c = 0x5F;
        else
          c -= 0x20;
      } else {
        c1 = (((c >=0x410) ? (c -= 0x350) : c) & 0xff); //Russian letters
        c = c1;
      }

      egRawCompose(BufferPtr, FontPixelData + c * FontWidth + Shift,
                   GlobalConfig.CharWidth, FontHeight,
                   BufferLineOffset, FontLineOffset);
      if (i == Cursor) {
        c = (GlobalConfig.Font == FONT_LOAD)?0x5F:0x3F;
        egRawCompose(BufferPtr, FontPixelData + c * FontWidth + Shift,
                     GlobalConfig.CharWidth, FontHeight,
                     BufferLineOffset, FontLineOffset);
      }
      BufferPtr += GlobalConfig.CharWidth;
    } else {
      //
      if ((c >= 0x20) && (c <= 0x7F)) {
        c1 = ((c - 0x20) >> 4) * 28 + (c & 0x0F);
        Cho = c1;
        Shift = 12;
      } else if ((c < 0x20) || ((c > 0x7F) && (c < 0xAC00))) {
        Cho = 0x0E; //just a dot
        Shift = 8;
      } else if ((c >= 0xAC00) && (c <= 0xD638)) {
        //korean
        Shift = 18;
        c -= 0xAC00;
        c1 = c / 28;
        Jong = c % 28;
        Cho = c1 / 21;
        Joong = c1 % 21;
        Cho += 28 * 7;
        Joong += 28 * 8;
        Jong += 28 * 9;
      }
//        DBG("Cho=%d Joong=%d Jong=%d\n", Cho, Joong, Jong);
      if (Shift == 18) {
        egRawCompose(BufferPtr, FontPixelData + Cho * 28 + 4 + FontLineOffset,
                     GlobalConfig.CharWidth, FontHeight,
                     BufferLineOffset, FontLineOffset);
      } else {
        egRawCompose(BufferPtr + BufferLineOffset * 3, FontPixelData + Cho * 28 + 2,
                     GlobalConfig.CharWidth, FontHeight,
                     BufferLineOffset, FontLineOffset);
      }
      if (i == Cursor) {
        c = 99;
        egRawCompose(BufferPtr, FontPixelData + c * 28 + 2,
                     GlobalConfig.CharWidth, FontHeight,
                     BufferLineOffset, FontLineOffset);
      }
      if (Shift == 18) {
        egRawCompose(BufferPtr + 8, FontPixelData + Joong * 28 + 6, //9 , 4 are tunable
                     GlobalConfig.CharWidth - 8, FontHeight,
                     BufferLineOffset, FontLineOffset);
        egRawCompose(BufferPtr + BufferLineOffset * 10, FontPixelData + Jong * 28 + 5,
                     GlobalConfig.CharWidth, FontHeight - 10,
                     BufferLineOffset, FontLineOffset);

      }

      BufferPtr += Shift;
    }
  }
}

/* EOF */
