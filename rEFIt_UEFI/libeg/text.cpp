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
//Slice 2011 - 2016 numerous improvements

#include "libegint.h"
#include "nanosvg.h"

//#include "egemb_font.h"
//#define FONT_CELL_WIDTH (7)
//#define FONT_CELL_HEIGHT (12)

#ifndef DEBUG_ALL
#define DEBUG_TEXT 0
#else
#define DEBUG_TEXT DEBUG_ALL
#endif

#if DEBUG_TEXT == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_TEXT, __VA_ARGS__)
#endif


EG_IMAGE *FontImage = NULL;
INTN FontWidth = 9;
INTN FontHeight = 18;
INTN TextHeight = 19;
NSVGfontChain *fontsDB = NULL;


CONST EG_PIXEL SemiWhitePixel = {255, 255, 255, 210}; //semitransparent

//
// Text rendering
//
#if USE_XTHEME
VOID egMeasureText(IN CONST CHAR16 *Text, OUT INTN *Width, OUT INTN *Height)
{
  INTN ScaledWidth = (INTN)(ThemeX.CharWidth * ThemeX.Scale);
  if (Width != NULL)
    *Width = StrLen(Text) * ((FontWidth > ScaledWidth)?FontWidth:ScaledWidth);
  if (Height != NULL)
    *Height = FontHeight;
}
#else


VOID egMeasureText(IN CONST CHAR16 *Text, OUT INTN *Width, OUT INTN *Height)
{
  INTN ScaledWidth = (INTN)(GlobalConfig.CharWidth * GlobalConfig.Scale);
    if (Width != NULL)
        *Width = StrLen(Text) * ((FontWidth > ScaledWidth)?FontWidth:ScaledWidth);
    if (Height != NULL)
        *Height = FontHeight;
}
#endif

EG_IMAGE * egLoadFontImage(IN BOOLEAN UseEmbedded, IN INTN Rows, IN INTN Cols)
{
  EG_IMAGE    *NewImage = NULL, *NewFontImage;
  INTN        ImageWidth, ImageHeight, x, y, Ypos, j;
  EG_PIXEL    *PixelPtr, FirstPixel;
  BOOLEAN     isKorean = (gLanguage == korean);
  CHAR16      *fontFilePath = NULL;
  CONST CHAR16      *commonFontDir = L"EFI\\CLOVER\\font";
  
  if (IsEmbeddedTheme() && !isKorean) { //or initial screen before theme init
    NewImage = egDecodePNG(ACCESS_EMB_DATA(emb_font_data), ACCESS_EMB_SIZE(emb_font_data), TRUE);
    MsgLog("Using embedded font: %s\n", NewImage ? "Success" : "Error");
  } else {
    if (!GlobalConfig.TypeSVG) {
      NewImage = egLoadImage(ThemeDir, isKorean ? L"FontKorean.png" : GlobalConfig.FontFileName, TRUE);
      MsgLog("Loading font from ThemeDir: %s\n", NewImage ? "Success" : "Error");
    } else {
      MsgLog("Using SVG font\n");
      return FontImage;
    }
  }
  
  if (!NewImage) {
    //then take from common font folder
    fontFilePath = PoolPrint(L"%s\\%s", commonFontDir, isKorean ? L"FontKorean.png" : GlobalConfig.FontFileName);
    NewImage = egLoadImage(SelfRootDir, fontFilePath, TRUE);
    //else use embedded
    if (!NewImage) {
      if (UseEmbedded) {
        NewImage = egDecodePNG(ACCESS_EMB_DATA(emb_font_data), ACCESS_EMB_SIZE(emb_font_data), TRUE);
      } else {
        MsgLog("Font %ls is not loaded\n", fontFilePath);
        FreePool(fontFilePath);
        return NULL;
      }
    }
    FreePool(fontFilePath);
  }
  
  ImageWidth = NewImage->Width;
  DBG("ImageWidth=%d\n", ImageWidth);
  ImageHeight = NewImage->Height;
  DBG("ImageHeight=%d\n", ImageHeight);
  PixelPtr = NewImage->PixelData;
  NewFontImage = egCreateImage(ImageWidth * Rows, ImageHeight / Rows, TRUE);
  
  if (NewFontImage == NULL) {
    DBG("Can't create new font image!\n");
    if (NewImage) {
      egFreeImage(NewImage);
    }
    return NULL;
  }
  
  FontWidth = ImageWidth / Cols;
  FontHeight = ImageHeight / Rows;
  TextHeight = FontHeight + (int)(TEXT_YMARGIN * 2 * GlobalConfig.Scale);
  FirstPixel = *PixelPtr;
  for (y = 0; y < Rows; y++) {
    for (j = 0; j < FontHeight; j++) {
      Ypos = ((j * Rows) + y) * ImageWidth;
      for (x = 0; x < ImageWidth; x++) {
        if (
            //First pixel is accounting as "blue key"
            (PixelPtr->b == FirstPixel.b) &&
            (PixelPtr->g == FirstPixel.g) &&
            (PixelPtr->r == FirstPixel.r)
            ) {
          PixelPtr->a = 0;
        } else if (GlobalConfig.DarkEmbedded) {
          *PixelPtr = SemiWhitePixel;
        }
        NewFontImage->PixelData[Ypos + x] = *PixelPtr++;
      }
    }
  }
  
  egFreeImage(NewImage);
  
  return NewFontImage;
}

VOID PrepareFont()
{
  EG_PIXEL    *p;
  INTN         Width, Height;

  TextHeight = FontHeight + (int)(TEXT_YMARGIN * 2 * GlobalConfig.Scale);
  if (GlobalConfig.TypeSVG) {
//    FontImage = RenderSVGfont();
    return;
  }

  if (FontImage) {
    egFreeImage(FontImage);
    FontImage = NULL;
  }

  if (gLanguage == korean) {
    FontImage = egLoadFontImage(FALSE, 10, 28);
    if (FontImage) {
//      FontHeight = 16;  //delete?
      GlobalConfig.CharWidth = 22;
//      FontWidth = GlobalConfig.CharWidth; //delete?
//      TextHeight = FontHeight + TEXT_YMARGIN * 2;
      MsgLog("Using Korean font matrix\n");
      return;
    } else {
      MsgLog("Korean font image not loaded, use english\n");
      gLanguage = english;
    }
  }
  
  // load the font
  if (FontImage == NULL){
    DBG("load font image type %d\n", GlobalConfig.Font);
    FontImage = egLoadFontImage(TRUE, 16, 16); //anyway success
  }
  
  if (FontImage) {
    if (GlobalConfig.Font == FONT_GRAY) {
      //invert the font. embedded is dark
      p = FontImage->PixelData;
      for (Height = 0; Height < FontImage->Height; Height++){
        for (Width = 0; Width < FontImage->Width; Width++, p++){
          p->b ^= 0xFF;
          p->g ^= 0xFF;
          p->r ^= 0xFF;
          //p->a = 0xFF;    //huh!
        }
      }
    }
    
//    TextHeight = FontHeight + TEXT_YMARGIN * 2;
    DBG("Font %d prepared WxH=%dx%d CharWidth=%d\n", GlobalConfig.Font, FontWidth, FontHeight, GlobalConfig.CharWidth);
  } else {
    DBG("Failed to load font\n");
  }
}


static inline BOOLEAN EmptyPix(EG_PIXEL *Ptr, EG_PIXEL *FirstPixel)
{
  //compare with first pixel of the array top-left point [0][0]
   return ((Ptr->r >= FirstPixel->r - (FirstPixel->r >> 2)) &&
           (Ptr->r <= FirstPixel->r + (FirstPixel->r >> 2)) &&
           (Ptr->g >= FirstPixel->g - (FirstPixel->g >> 2)) &&
           (Ptr->g <= FirstPixel->g + (FirstPixel->g >> 2)) &&
           (Ptr->b >= FirstPixel->b - (FirstPixel->b >> 2)) &&
           (Ptr->b <= FirstPixel->b + (FirstPixel->b >> 2)) &&
           (Ptr->a == FirstPixel->a)); //hack for transparent fonts
}

INTN GetEmpty(EG_PIXEL *Ptr, EG_PIXEL *FirstPixel, INTN MaxWidth, INTN Step, INTN Row)
{
  INTN i, j, m;
  EG_PIXEL *Ptr0, *Ptr1;

  Ptr1 = (Step > 0)?Ptr:Ptr - 1;
  m = MaxWidth;
  for (j = 0; j < FontHeight; j++) {
    Ptr0 = Ptr1 + j * Row;
    for (i = 0; i < MaxWidth; i++) {
      if (!EmptyPix(Ptr0, FirstPixel)) {
        break;
      }
      Ptr0 += Step;
    }
    m = (i > m)?m:i;
  }
  return m;
}

#if USE_XTHEME
INTN egRenderText(IN XStringW& Text, IN XImage& CompImage,
                  IN INTN PosX, IN INTN PosY, IN INTN Cursor, INTN textType)
#else
INTN egRenderText(IN CONST CHAR16 *Text, IN OUT EG_IMAGE *CompImage,
                  IN INTN PosX, IN INTN PosY, IN INTN Cursor, INTN textType)
#endif
{
  EG_PIXEL        *BufferPtr;
  EG_PIXEL        *FontPixelData;
  EG_PIXEL        *FirstPixelBuf;
  INTN            BufferLineWidth, BufferLineOffset, FontLineOffset;
  INTN            TextLength /*, NewTextLength = 0 */;
  INTN            i;
  UINT16          c, c1, c0;
  UINTN           Shift = 0;
  UINTN           Cho = 0, Jong = 0, Joong = 0;
  UINTN           LeftSpace, RightSpace;
  INTN            RealWidth = 0;


#if USE_XTHEME
    INTN ScaledWidth = (INTN)(ThemeX.CharWidth * ThemeX.Scale);
  if (ThemeX.TypeSVG) {
    return renderSVGtext(CompImage, PosX, PosY, textType, Text, Cursor);
  }
#else
    INTN ScaledWidth = (INTN)(GlobalConfig.CharWidth * GlobalConfig.Scale);
  if (GlobalConfig.TypeSVG) {
    return renderSVGtext(CompImage, PosX, PosY, textType, Text, Cursor);
  }
#endif

  
  // clip the text
  TextLength = StrLen(Text);
  if (!FontImage) {
//    GlobalConfig.Font = FONT_ALFA;
    PrepareFont();
  }
  
//  DBG("TextLength =%d PosX=%d PosY=%d\n", TextLength, PosX, PosY);
  // render it
#if USE_XTHEME
  BufferPtr = (EG_PIXEL*)CompImage.GetPixelPtr(0,0);
  BufferLineOffset = CompImage.GetWidth();
#else
  BufferPtr = CompImage->PixelData;
  BufferLineOffset = CompImage->Width;
#endif
  BufferLineWidth = BufferLineOffset - PosX; // remove indent from drawing width
  BufferPtr += PosX + PosY * BufferLineOffset;
  FirstPixelBuf = BufferPtr;
  FontPixelData = FontImage->PixelData;
  FontLineOffset = FontImage->Width;
//  DBG("BufferLineOffset=%d  FontLineOffset=%d\n", BufferLineOffset, FontLineOffset);

  if (ScaledWidth < FontWidth) {
    Shift = (FontWidth - ScaledWidth) >> 1;
  }
  c0 = 0;
  RealWidth = ScaledWidth;
//  DBG("FontWidth=%d, CharWidth=%d\n", FontWidth, RealWidth);
  for (i = 0; i < TextLength; i++) {
#if USE_XTHEME
    c = Text.data()[i];
#else
    c = Text[i];
#endif
    if (gLanguage != korean) {
      c1 = (((c >= GlobalConfig.Codepage) ? (c - (GlobalConfig.Codepage - AsciiPageSize)) : c) & 0xff); //International letters
      c = c1;

      if (GlobalConfig.Proportional) {
        if (c0 <= 0x20) {  // space before or at buffer edge
          LeftSpace = 2;
        } else {
          LeftSpace = GetEmpty(BufferPtr, FirstPixelBuf, ScaledWidth, -1, BufferLineOffset);
        }
        if (c <= 0x20) { //new space will be half width
          RightSpace = 1;
          RealWidth = (ScaledWidth >> 1) + 1;
        } else {
          RightSpace = GetEmpty(FontPixelData + c * FontWidth, FontPixelData, FontWidth, 1, FontLineOffset);
          if (RightSpace >= ScaledWidth + Shift) {
            RightSpace = 0; //empty place for invisible characters
          }
          RealWidth = FontWidth - RightSpace;
        }

      } else {
        LeftSpace = 2;
        RightSpace = Shift;
      }
      c0 = c; //old value
      if ((UINTN)BufferPtr + RealWidth * 4 > (UINTN)FirstPixelBuf + BufferLineWidth * 4) {
        break;
      }
      egRawCompose(BufferPtr - LeftSpace + 2, FontPixelData + c * FontWidth + RightSpace,
                   RealWidth, FontHeight,
                   BufferLineOffset, FontLineOffset);
      if (i == Cursor) {
        c = 0x5F;
        egRawCompose(BufferPtr - LeftSpace + 2, FontPixelData + c * FontWidth + RightSpace,
                     RealWidth, FontHeight,
                     BufferLineOffset, FontLineOffset);
      }
      BufferPtr += RealWidth - LeftSpace + 2;
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
        egRawCompose(BufferPtr, FontPixelData + Cho * FontWidth + 4 + FontLineOffset,
                     ScaledWidth, FontHeight,
                     BufferLineOffset, FontLineOffset);
      } else {
        egRawCompose(BufferPtr + BufferLineOffset * 3, FontPixelData + Cho * FontWidth + 2,
                     ScaledWidth, FontHeight,
                     BufferLineOffset, FontLineOffset);
      }
      if (i == Cursor) {
        c = 99;
        egRawCompose(BufferPtr, FontPixelData + c * FontWidth + 2,
                     ScaledWidth, FontHeight,
                     BufferLineOffset, FontLineOffset);
      }
      if (Shift == 18) {
        egRawCompose(BufferPtr + 9, FontPixelData + Joong * FontWidth + 6, //9 , 4 are tunable
                     ScaledWidth - 8, FontHeight,
                     BufferLineOffset, FontLineOffset);
        egRawCompose(BufferPtr + BufferLineOffset * 9, FontPixelData + Jong * FontWidth + 1,
                     ScaledWidth, FontHeight - 3,
                     BufferLineOffset, FontLineOffset);

      }

      BufferPtr += Shift;
    }
  }
  return ((INTN)BufferPtr - (INTN)FirstPixelBuf) / sizeof(EG_PIXEL);
}

/* EOF */
