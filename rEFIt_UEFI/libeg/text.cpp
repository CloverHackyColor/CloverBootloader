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
//Slice 2011 - 2016 numerous improvements, 2020 full rewritten for c++

extern "C" {
#include <Protocol/GraphicsOutput.h>
}

#include "libegint.h"
#include "nanosvg.h"
#include "VectorGraphics.h"

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

#if USE_XTHEME
const EFI_GRAPHICS_OUTPUT_BLT_PIXEL SemiWhitePixel = {0xFF, 0xFF, 0xFF, 0xD2}; //semitransparent white
#else
EG_IMAGE *FontImage = NULL;  //theme member
CONST EG_PIXEL SemiWhitePixel = {255, 255, 255, 210}; //semitransparent
#endif

#if USE_XTHEME
#else

INTN FontWidth = 9;  //local variable
INTN FontHeight = 18;
INTN TextHeight = 19;
#endif
NSVGfontChain *fontsDB = NULL;

//
// Text rendering
//
#if USE_XTHEME
//it is not good for vector theme
//it will be better to sum each letter width for the chosen font
// so one more parameter is TextStyle
VOID XTheme::MeasureText(IN const XStringW& Text, OUT INTN *Width, OUT INTN *Height)
{
  INTN ScaledWidth = CharWidth;
  INTN ScaledHeight = FontHeight;
  if (Scale != 0.f) {
    ScaledWidth = (INTN)(CharWidth * Scale);
    ScaledHeight = (INTN)(FontHeight * Scale);
  }
  if (Width != NULL)
    *Width = StrLen(Text.wc_str()) * ((FontWidth > ScaledWidth) ? FontWidth : ScaledWidth);
  if (Height != NULL)
    *Height = ScaledHeight;
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

#if USE_XTHEME
void XTheme::LoadFontImage(IN BOOLEAN UseEmbedded, IN INTN Rows, IN INTN Cols)
{
  EFI_STATUS Status = EFI_NOT_FOUND;
  XImage      NewImage; //tempopary image from file

  INTN        ImageWidth, ImageHeight;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *PixelPtr;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *FontPtr;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL FirstPixel;
  BOOLEAN     isKorean = (gLanguage == korean);
  XStringW    fontFilePath;
  const XStringW& commonFontDir = L"EFI\\CLOVER\\font"_XSW;

  if (IsEmbeddedTheme() && !isKorean) { //or initial screen before theme init
    Status = NewImage.FromPNG(ACCESS_EMB_DATA(emb_font_data), ACCESS_EMB_SIZE(emb_font_data)); //always success
    MsgLog("Using embedded font\n");
  } else if (isKorean){
    Status = NewImage.LoadXImage(ThemeDir, L"FontKorean.png"_XSW);
    MsgLog("Loading korean font from ThemeDir: %s\n", strerror(Status));
    if (!EFI_ERROR(Status)) {
      CharWidth = 22; //standard for korean
    } else {
      MsgLog("...using english\n");
      gLanguage = english;
    }
  }

  if (EFI_ERROR(Status)) {
    //not loaded, use common
    Rows = 16; //standard for english
    Cols = 16;
    Status = NewImage.LoadXImage(ThemeDir, FontFileName);
  }

  if (EFI_ERROR(Status)) {
    //then take from common font folder
//    fontFilePath = SWPrintf(L"%s\\%s", commonFontDir, isKorean ? L"FontKorean.png" : ThemeX.FontFileName.data());
    fontFilePath = commonFontDir + FontFileName;
    Status = NewImage.LoadXImage(SelfRootDir, fontFilePath);
    //else use embedded even if it is not embedded
    if (EFI_ERROR(Status)) {
      Status = NewImage.FromPNG(ACCESS_EMB_DATA(emb_font_data), ACCESS_EMB_SIZE(emb_font_data));
    }
    if (EFI_ERROR(Status)) {
      MsgLog("No font found!\n");
      return;
    }
  }

  ImageWidth = NewImage.GetWidth();
  //  DBG("ImageWidth=%lld\n", ImageWidth);
  ImageHeight = NewImage.GetHeight();
  //  DBG("ImageHeight=%lld\n", ImageHeight);
  PixelPtr = NewImage.GetPixelPtr(0,0);

  FontImage.setSizeInPixels(ImageWidth * Rows, ImageHeight / Rows);
  FontPtr = FontImage.GetPixelPtr(0,0);

  FontWidth = ImageWidth / Cols;
  FontHeight = ImageHeight / Rows;
  TextHeight = FontHeight + (int)(TEXT_YMARGIN * 2 * Scale);
//  if (!isKorean) {
//    CharWidth = FontWidth; //there is default value anyway
//  }

  FirstPixel = *PixelPtr;
  for (INTN y = 0; y < Rows; y++) {
    for (INTN j = 0; j < FontHeight; j++) {
      INTN Ypos = ((j * Rows) + y) * ImageWidth;
      for (INTN x = 0; x < ImageWidth; x++) {
        if (
            //First pixel is accounting as "blue key"
            (PixelPtr->Blue == FirstPixel.Blue) &&
            (PixelPtr->Green == FirstPixel.Green) &&
            (PixelPtr->Red == FirstPixel.Red)
            ) {
          PixelPtr->Reserved = 0; //if a pixel has same color as first pixel then it will be transparent
        } else if (ThemeX.DarkEmbedded) {
          *PixelPtr = SemiWhitePixel;  //special case to change a text to semi white, not blue pixels
        }
        FontPtr[Ypos + x] = *PixelPtr++; //not (x, YPos) !!!
      }
    }
  }
}

#else
//should be rewritten to use XImage and XStringW
EG_IMAGE * egLoadFontImage(IN BOOLEAN UseEmbedded, IN INTN Rows, IN INTN Cols)
{
  EG_IMAGE    *NewImage = NULL, *NewFontImage;
  INTN        ImageWidth, ImageHeight, x, y, Ypos, j;
  EG_PIXEL    *PixelPtr, FirstPixel;
  BOOLEAN     isKorean = (gLanguage == korean);
  CHAR16      *fontFilePath = NULL;
  CONST CHAR16  *commonFontDir = L"EFI\\CLOVER\\font";

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
  //  DBG("ImageWidth=%lld\n", ImageWidth);
  ImageHeight = NewImage->Height;
  //  DBG("ImageHeight=%lld\n", ImageHeight);
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
          PixelPtr->a = 0; //if a pixel has same color as first pixel then it will be transparent
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

#endif

#if USE_XTHEME
VOID XTheme::PrepareFont()
{

  TextHeight = FontHeight + (int)(TEXT_YMARGIN * 2 * Scale);
  if (TypeSVG) {
    return;
  }

  // load the font
  if (FontImage.isEmpty()) {
    DBG("load font image type %d\n", Font);
    LoadFontImage(TRUE, 16, 16); //anyway success
  }

  if (!FontImage.isEmpty()) {
    if (Font == FONT_GRAY) {
      //invert the font. embedded is dark
      EFI_GRAPHICS_OUTPUT_BLT_PIXEL *p = FontImage.GetPixelPtr(0,0);
      for (INTN Height = 0; Height < FontImage.GetHeight(); Height++){
        for (INTN Width = 0; Width < FontImage.GetWidth(); Width++, p++){
          p->Blue  ^= 0xFF;
          p->Green ^= 0xFF;
          p->Red   ^= 0xFF;
          //p->a = 0xFF;    //huh! dont invert opacity
        }
      }
 //     FontImage.Draw(0, 300, 0.6f); //for debug purpose
    }
    DBG("Font %d prepared WxH=%lldx%lld CharWidth=%lld\n", Font, FontWidth, FontHeight, CharWidth);

  } else {
    DBG("Failed to load font\n");
  }
}

#else
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
      GlobalConfig.CharWidth = 22;
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
    DBG("Font %d prepared WxH=%lldx%lld CharWidth=%lld\n", GlobalConfig.Font, FontWidth, FontHeight, GlobalConfig.CharWidth);

    //    TextHeight = FontHeight + TEXT_YMARGIN * 2;
  } else {
    DBG("Failed to load font\n");
  }
}

#endif

//search pixel similar to first
#if USE_XTHEME
inline bool SamePix(const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& Ptr, const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& FirstPixel)
{
  //compare with first pixel of the array top-left point [0][0]
  return ((Ptr.Red >= FirstPixel.Red - (FirstPixel.Red >> 2)) &&
          (Ptr.Red <= FirstPixel.Red + (FirstPixel.Red >> 2)) &&
          (Ptr.Green >= FirstPixel.Green - (FirstPixel.Green >> 2)) &&
          (Ptr.Green <= FirstPixel.Green + (FirstPixel.Green >> 2)) &&
          (Ptr.Blue >= FirstPixel.Blue - (FirstPixel.Blue >> 2)) &&
          (Ptr.Blue <= FirstPixel.Blue + (FirstPixel.Blue >> 2)) &&
          (Ptr.Reserved == FirstPixel.Reserved)); //hack for transparent fonts
}

//used for proportional fonts in raster themes
//search empty column from begin Step=1 or from end Step=-1 in the input buffer
// empty means similar to FirstPixel
INTN XTheme::GetEmpty(const XImage& Buffer, const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& FirstPixel, INTN Start, INTN Step)
{
  INTN m, i;
//  INTN Shift = (Step > 0)?0:1;
  m = FontWidth;
  if (Step == 1) {
    for (INTN j = 0; j < FontHeight; j++) {
      for (i = 0; i < FontWidth; i++) {
        if (!SamePix(Buffer.GetPixel(Start + i,j), FirstPixel)) { //found not empty pixel
          break;
        }
      }
      m = MIN(m, i); //for each line to find minimum
      if (m == 0) break;
    }
  } else { // go back
    m = 0;
    for (INTN j = 0; j < FontHeight; j++) {
      for (i = FontWidth - 1; i >= 0; --i) {
        if (!SamePix(Buffer.GetPixel(Start + i,j), FirstPixel)) { //found not empty pixel
          break;
        }
      }
      m = MAX(m, i); //for each line to find minimum
    }
  }
  return m;
}
#else

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
#endif

#if USE_XTHEME
INTN XTheme::RenderText(IN const XString& Text, OUT XImage* CompImage_ptr,
                        IN INTN PosX, IN INTN PosY, IN INTN Cursor, INTN textType, float textScale)
{
  const XStringW& UTF16Text = XStringW().takeValueFrom(Text.c_str());
  return RenderText(UTF16Text, CompImage_ptr, PosX, PosY, Cursor, textType, textScale);
}

INTN XTheme::RenderText(IN const XStringW& Text, OUT XImage* CompImage_ptr,
                  IN INTN PosX, IN INTN PosY, IN INTN Cursor, INTN textType, float textScale)
{
  XImage& CompImage = *CompImage_ptr;

  EFI_GRAPHICS_OUTPUT_BLT_PIXEL    FontPixel;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL    FirstPixel;
  INTN           TextLength;
  UINTN          Cho = 0, Jong = 0, Joong = 0;
  INTN           LeftSpace, RightSpace;

  if (TypeSVG) {
    return renderSVGtext(&CompImage, PosX, PosY, textType, Text, Cursor);
  }

  if (textScale == 0.f) {
    textScale = 1.f;
  }
  INTN CharScaledWidth = (INTN)(CharWidth * textScale);
  INTN FontScaledWidth = (INTN)(FontWidth * textScale); //FontWidth should be scaled as well?
  // clip the text
//  TextLength = StrLenInWChar(Text.wc_str()); //it must be UTF16 length
  TextLength = StrLen(Text.wc_str());
  DBG("text to render %ls length %lld\n", Text.wc_str(), TextLength);
  if (FontImage.isEmpty()) {
    PrepareFont(); //at the boot screen there is embedded font
  }

    DBG("TextLength =%lld PosX=%lld PosY=%lld\n", TextLength, PosX, PosY);
  FirstPixel = CompImage.GetPixel(0,0);
  FontPixel  = FontImage.GetPixel(0,0);
  UINT16 c0 = 0x20;
  INTN RealWidth = CharScaledWidth;
  INTN Shift = (FontScaledWidth - CharScaledWidth) / 2;
  if (Shift < 0) {
    Shift = 0;
  }
    DBG("FontWidth=%lld, CharWidth=%lld\n", FontWidth, RealWidth);

  EG_RECT Area; //area is scaled
  Area.YPos = PosY; // not sure
  Area.Height = TextHeight;
  EG_RECT Bukva; //bukva is not scaled place
  Bukva.YPos = 0;
  Bukva.Width = FontWidth;
  Bukva.Height = FontHeight;
  DBG("codepage=%llx, asciiPage=%x\n", GlobalConfig.Codepage, AsciiPageSize);
  for (INTN i = 0; i < TextLength && c0 != 0; i++) {
    UINT16 c = Text.wc_str()[i]; //including UTF8 -> UTF16 conversion
    DBG("initial char to render 0x%x\n", c); //good
    if (gLanguage != korean) { //russian Codepage = 0x410
      if (c >= 0x410 && c < 0x450) {
        //we have russian raster fonts with chars at 0xC0
        c -= 0x350;
      } else {
        INTN c2 = (c >= GlobalConfig.Codepage) ? (c - GlobalConfig.Codepage + AsciiPageSize) : c; //International letters
        c = c2 & 0xFF; //this maximum raster font size
      }
//      DBG("char to render 0x%x\n", c);
      if (Proportional) {
        //find spaces {---comp--__left__|__right__--char---}
        if (c0 <= 0x20) {  // space before or at buffer edge
          LeftSpace = 2;
        } else {
          LeftSpace = GetEmpty(CompImage, FirstPixel, PosX, -1);
        }
        if (c <= 0x20) { //new space will be half font width
          RightSpace = 1;
          RealWidth = (CharScaledWidth >> 1) + 1;
        } else {
          RightSpace = GetEmpty(FontImage, FontPixel, c * FontWidth, 1); //not scaled yet
          if (RightSpace >= FontWidth) {
            RightSpace = 0; //empty place for invisible characters
          }
          RealWidth = CharScaledWidth - (int)(RightSpace * textScale); //a part of char
        }
      } else {
        LeftSpace = 2;
        RightSpace = Shift;
      }
      LeftSpace = (int)(LeftSpace * textScale); //was not scaled yet
      //RightSpace will not be scaled
      // RealWidth are scaled now
     DBG(" RealWidth =  %lld LeftSpace = %lld RightSpace = %lld\n", RealWidth, LeftSpace, RightSpace);
      c0 = c; //remember old value
      if (PosX + RealWidth  > CompImage.GetWidth()) {
        DBG("no more place for character\n");
        break;
      }

      Area.XPos = PosX + 2 - LeftSpace;
      Area.Width = RealWidth;
      Bukva.XPos = c * FontWidth + RightSpace;
      DBG("place [%lld,%lld,%lld,%lld], bukva [%lld,%lld,%lld,%lld]\n",
          Area.XPos, Area.YPos, Area.Width, Area.Height,
          Bukva.XPos, Bukva.YPos, Bukva.Width, Bukva.Height);

      CompImage.Compose(Area, Bukva, FontImage, false, textScale);
//      CompImage.CopyRect(FontImage, Area, Bukva);
      if (i == Cursor) {
        c = 0x5F;
        Bukva.XPos = c * FontWidth + RightSpace;
        CompImage.Compose(Area, Bukva, FontImage, false, textScale);
      }
      PosX += RealWidth - LeftSpace + 2; //next char position
    } else {
      //
      //Slice - I am not sure in any of this digits
      //someone knowning korean should revise this
      //
      UINT16 c1 = c;
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

      Area.XPos = PosX;
      Area.Width = CharWidth;

      //        DBG("Cho=%d Joong=%d Jong=%d\n", Cho, Joong, Jong);
      if (Shift == 18) {
        Bukva.XPos = Cho * FontWidth + 4;
        Bukva.YPos = 1;
        CompImage.Compose(Area, Bukva, FontImage, false, textScale);
      } else {
        Area.YPos = PosY + 3;
        Bukva.XPos = Cho * FontWidth + 2;
        Bukva.YPos = 0;
        CompImage.Compose(Area, Bukva, FontImage, false, textScale);
      }
      if (i == Cursor) {
        c = 99;
        Bukva.XPos = c * FontWidth + 2;
        CompImage.Compose(Area, Bukva, FontImage, false, textScale);
      }
      if (Shift == 18) {
        Area.XPos = PosX + 9;
        Area.YPos = PosY;
        Bukva.XPos = Joong * FontWidth + 6;
        Bukva.YPos = 0;
        CompImage.Compose(Area, Bukva, FontImage, false, textScale);

        Area.XPos = PosX;
        Area.YPos = PosY + 9;
        Bukva.XPos = Jong * FontWidth + 1;
        CompImage.Compose(Area, Bukva, FontImage, false, textScale);
      }

      PosX += CharWidth; //Shift;
    }
  }
  return PosX;
}
#else
INTN egRenderText(IN CONST CHAR16 *Text, IN OUT EG_IMAGE *CompImage,
                  IN INTN PosX, IN INTN PosY, IN INTN Cursor, INTN textType)
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

  INTN ScaledWidth = (INTN)(GlobalConfig.CharWidth * GlobalConfig.Scale);
  if (GlobalConfig.TypeSVG) {
    return renderSVGtext(CompImage, PosX, PosY, textType, Text, Cursor);
  }

  // clip the text
  TextLength = StrLen(Text);

  if (!FontImage) {
    //    GlobalConfig.Font = FONT_ALFA;
    PrepareFont(); //at the boot screen there is embedded font
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

  if (ScaledWidth < FontWidth) {
    Shift = (FontWidth - ScaledWidth) >> 1;
  }
  c0 = 0;
  RealWidth = ScaledWidth;
  //  DBG("FontWidth=%d, CharWidth=%d\n", FontWidth, RealWidth);
  for (i = 0; i < TextLength; i++) {
    c = Text[i];
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
#endif


/* EOF */
