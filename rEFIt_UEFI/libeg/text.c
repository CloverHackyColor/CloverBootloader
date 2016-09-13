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
INTN FontWidth = 9;
INTN FontHeight = 18;
INTN TextHeight = 19;

//
// Text rendering
//

VOID egMeasureText(IN CHAR16 *Text, OUT INTN *Width, OUT INTN *Height)
{
    if (Width != NULL)
        *Width = StrLen(Text) * ((FontWidth > GlobalConfig.CharWidth)?FontWidth:GlobalConfig.CharWidth);
    if (Height != NULL)
        *Height = FontHeight;
}
#if 0
EG_IMAGE * egLoadFontImage(IN BOOLEAN FromTheme, IN INTN Rows, IN INTN Cols)
{
  EG_IMAGE            *NewImage = NULL;
  EG_IMAGE            *NewFontImage;
  INTN        ImageWidth, ImageHeight;
  INTN        x, y, Ypos, j;
  EG_PIXEL    *PixelPtr;
  EG_PIXEL    FirstPixel;
  BOOLEAN     WantAlpha = TRUE;
  BOOLEAN     Embedded = FALSE;

  if (FromTheme) {
    NewImage = egLoadImage(ThemeDir, GlobalConfig.FontFileName, WantAlpha);
  }

  if (NewImage) {
    if (FromTheme) {
      DBG("font %s loaded from themedir\n", GlobalConfig.FontFileName);
    } else {
      DBG("Korean font loaded from themedir\n");
    }
  } else {
    CHAR16 *commonFontDir = L"EFI\\CLOVER\\font";
    CHAR16 *fontFilePath;
    if (FromTheme) {
      fontFilePath = PoolPrint(L"%s\\%s", commonFontDir, GlobalConfig.FontFileName);
    } else {
      fontFilePath = PoolPrint(L"%s\\%s", commonFontDir, L"FontKorean.png");
    }
    NewImage = egLoadImage(SelfRootDir, fontFilePath, WantAlpha);
    if (!NewImage) {
      DBG("Font %s is not loaded, using embedded\n", fontFilePath);
      NewImage = egDecodePNG(&emb_font_data[0], sizeof(emb_font_data), WantAlpha);
      Embedded = TRUE;
    } else {
      DBG("font %s loaded from common font dir %s\n", GlobalConfig.FontFileName, commonFontDir);
    }
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

#else
EG_IMAGE * egLoadFontImage(IN BOOLEAN FromTheme, IN INTN Rows, IN INTN Cols)
{
  EG_IMAGE    *NewImage = NULL, *NewFontImage;
  INTN        ImageWidth, ImageHeight, x, y, Ypos, j;
  EG_PIXEL    *PixelPtr, FirstPixel;
  BOOLEAN     isKorean = (gLanguage == korean);
  CHAR16      *fontFilePath, *commonFontDir = L"EFI\\CLOVER\\font";
  
  if (IsEmbeddedTheme() && !isKorean) {
    DBG("Using embedded font\n");
    goto F_EMBEDDED;
  } else {
    NewImage = egLoadImage(ThemeDir, isKorean ? L"FontKorean.png" : GlobalConfig.FontFileName, TRUE);
    DBG("Loading font from ThemeDir: %a\n", NewImage ? "Success" : "Error");
  }
  
  if (NewImage) {
    goto F_THEME;
  } else {
    fontFilePath = PoolPrint(L"%s\\%s", commonFontDir, isKorean ? L"FontKorean.png" : GlobalConfig.FontFileName);
    NewImage = egLoadImage(SelfRootDir, fontFilePath, TRUE);
    
    if (!NewImage) {
      if (!isKorean) {
        DBG("Font %s is not loaded, using embedded\n", fontFilePath);
        FreePool(fontFilePath);
        goto F_EMBEDDED;
      }
      FreePool(fontFilePath);
      return NULL;
    } else {
      DBG("font %s loaded from common font dir %s\n", GlobalConfig.FontFileName, commonFontDir);
      FreePool(fontFilePath);
      goto F_THEME;
    }
  }
  
F_EMBEDDED:
  //NewImage = DEC_PNG_BUILTIN(emb_font_data);
  NewImage = egDecodePNG(&emb_font_data[0], sizeof(emb_font_data), TRUE);
  
F_THEME:
  ImageWidth = NewImage->Width;
  //DBG("ImageWidth=%d\n", ImageWidth);
  ImageHeight = NewImage->Height;
  //DBG("ImageHeight=%d\n", ImageHeight);
  PixelPtr = NewImage->PixelData;
  DBG("Font loaded: ImageWidth=%d ImageHeight=%d\n", ImageWidth, ImageHeight);
  NewFontImage = egCreateImage(ImageWidth * Rows, ImageHeight / Rows, TRUE);
  
  if (NewFontImage == NULL) {
    DBG("Can't create new font image!\n");
    return NULL;
  }
  
  FontWidth = ImageWidth / Cols;
  FontHeight = ImageHeight / Rows;
  FirstPixel = *PixelPtr;
  for (y = 0; y < Rows; y++) {
    for (j = 0; j < FontHeight; j++) {
      Ypos = ((j * Rows) + y) * ImageWidth;
      for (x = 0; x < ImageWidth; x++) {
        if (//WantAlpha &&
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
#endif
VOID PrepareFont()
{
  EG_PIXEL    *p;
  INTN         Width, Height;
  
  if (gLanguage == korean) {
    FontImage = egLoadFontImage(FALSE, 10, 28);
    if (FontImage) {
//      FontHeight = 16;  //delete?
      GlobalConfig.CharWidth = 22;
//      FontWidth = GlobalConfig.CharWidth; //delete?
      TextHeight = FontHeight + TEXT_YMARGIN * 2;
      DBG("Using Korean font matrix\n");
      return;
    } else {
      DBG("font image not loaded, use english\n");
      gLanguage = english;
    }
  }
  
  // load the font
  if (FontImage == NULL){
    DBG("load font image type %d\n", GlobalConfig.Font);
    FontImage = egLoadFontImage(TRUE, 16, 16);
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
    
    TextHeight = FontHeight + TEXT_YMARGIN * 2;
    DBG("Font %d prepared WxH=%dx%d CharWidth=%d\n", GlobalConfig.Font, FontWidth, FontHeight, GlobalConfig.CharWidth);
  } else {
    DBG("Failed to load font\n");
  }
}


static inline BOOLEAN EmptyPix(EG_PIXEL *Ptr, EG_PIXEL *FirstPixel)
{
  //compare with first pixel of the array top-left point [0][0]
   return ((Ptr->r >= FirstPixel->r - (FirstPixel->r >> 2)) && (Ptr->r <= FirstPixel->r + (FirstPixel->r >> 2)) &&
           (Ptr->g >= FirstPixel->g - (FirstPixel->g >> 2)) && (Ptr->g <= FirstPixel->g + (FirstPixel->g >> 2)) &&
           (Ptr->b >= FirstPixel->b - (FirstPixel->b >> 2)) && (Ptr->b <= FirstPixel->b + (FirstPixel->b >> 2)) &&
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

INTN egRenderText(IN CHAR16 *Text, IN OUT EG_IMAGE *CompImage,
                  IN INTN PosX, IN INTN PosY, IN INTN Cursor)
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
  
  // clip the text
  TextLength = StrLen(Text);
  if (!FontImage) {
//    GlobalConfig.Font = FONT_ALFA;
    PrepareFont();
  }
  
//  DBG("TextLength =%d PosX=%d PosY=%d\n", TextLength, PosX, PosY);
  // render it
  BufferPtr = CompImage->PixelData;
  BufferLineOffset = CompImage->Width;
  BufferLineWidth = BufferLineOffset - PosX; // remove indent from drawing width
  BufferPtr += PosX + PosY * BufferLineOffset;
  FirstPixelBuf = BufferPtr;
  FontPixelData = FontImage->PixelData;
  FontLineOffset = FontImage->Width;
//  DBG("BufferLineOffset=%d  FontLineOffset=%d\n", BufferLineOffset, FontLineOffset);

  if (GlobalConfig.CharWidth < FontWidth) {
    Shift = (FontWidth - GlobalConfig.CharWidth) >> 1;
  }
  c0 = 0;
  RealWidth = GlobalConfig.CharWidth;
//  DBG("FontWidth=%d, CharWidth=%d\n", FontWidth, RealWidth);
  for (i = 0; i < TextLength; i++) {
    c = Text[i];
    if (gLanguage != korean) {
      /*      if (GlobalConfig.Font != FONT_LOAD) {
       if (c < 0x20 || c >= 0x7F)
       c = 0x5F;
       else
       c -= 0x20;
       } else { */
      c1 = (((c >=0x410) ? (c -= 0x350) : c) & 0xff); //Russian letters
      c = c1;
      //      }

      if (GlobalConfig.Proportional) {
        if (c0 <= 0x20) {  // space before or at buffer edge
          LeftSpace = 2;
        } else {
          LeftSpace = GetEmpty(BufferPtr, FirstPixelBuf, GlobalConfig.CharWidth, -1, BufferLineOffset);
        }
        if (c <= 0x20) { //new space will be half width
          RightSpace = 1;
          RealWidth = (GlobalConfig.CharWidth >> 1) + 1;
        } else {
          RightSpace = GetEmpty(FontPixelData + c * FontWidth, FontPixelData, FontWidth, 1, FontLineOffset);
          if (RightSpace >= GlobalConfig.CharWidth + Shift) {
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
                     GlobalConfig.CharWidth, FontHeight,
                     BufferLineOffset, FontLineOffset);
      } else {
        egRawCompose(BufferPtr + BufferLineOffset * 3, FontPixelData + Cho * FontWidth + 2,
                     GlobalConfig.CharWidth, FontHeight,
                     BufferLineOffset, FontLineOffset);
      }
      if (i == Cursor) {
        c = 99;
        egRawCompose(BufferPtr, FontPixelData + c * FontWidth + 2,
                     GlobalConfig.CharWidth, FontHeight,
                     BufferLineOffset, FontLineOffset);
      }
      if (Shift == 18) {
        egRawCompose(BufferPtr + 9, FontPixelData + Joong * FontWidth + 6, //9 , 4 are tunable
                     GlobalConfig.CharWidth - 8, FontHeight,
                     BufferLineOffset, FontLineOffset);
        egRawCompose(BufferPtr + BufferLineOffset * 9, FontPixelData + Jong * FontWidth + 5,
                     GlobalConfig.CharWidth, FontHeight - 10,
                     BufferLineOffset, FontLineOffset);

      }

      BufferPtr += Shift;
    }
  }
  return ((INTN)BufferPtr - (INTN)FirstPixelBuf) / sizeof(EG_PIXEL);
}

/* EOF */
