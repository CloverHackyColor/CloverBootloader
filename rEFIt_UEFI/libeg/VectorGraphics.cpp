/*
 * Additional procedures for vector theme support
 *
 * Slice, 2018
 *
 */


#define TEST_MATH 0
#define TEST_SVG_IMAGE 1
#define TEST_SIZEOF 0
#define TEST_FONT 0
#define TEST_DITHER 0

#include "VectorGraphics.h"

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile

#include "nanosvg.h"
#include "FloatLib.h"
#include "lodepng.h"
#include "../refit/screen.h"
#include "../cpp_foundation/XString.h"
#include "../refit/lib.h"
#include "../Settings/Self.h"

#ifndef DEBUG_ALL
#define DEBUG_VEC 1
#else
#define DEBUG_VEC DEBUG_ALL
#endif

#if DEBUG_VEC == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_VEC, __VA_ARGS__)
#endif

#include "XTheme.h"

extern const CHAR8* IconsNames[];
extern const INTN IconsNamesSize;


#define NSVG_RGB(r, g, b) (((unsigned int)b) | ((unsigned int)g << 8) | ((unsigned int)r << 16))
//#define NSVG_RGBA(r, g, b, a) (((unsigned int)b) | ((unsigned int)g << 8) | ((unsigned int)r << 16) | ((unsigned int)a << 24))

extern void
WaitForKeyPress(CHAR16 *Message);


EFI_STATUS XTheme::ParseSVGXIcon(NSVGparser* SVGParser, INTN Id, const XString8& IconNameX, OUT XImage* Image)
{
  EFI_STATUS      Status = EFI_NOT_FOUND;
  NSVGimage*   SVGimage = SVGParser->image;  // full theme SVG image
  NSVGshape   *shape;
  NSVGshape   *shapeNext/*, *shapesTail = NULL, *shapePrev*/;


  float IconImageWidth = 0;        // Width of the image.
  float IconImageHeight = 0;        // Height of the image.

  shape = SVGimage->shapes;
  while (shape) {
    shapeNext = shape->next;
    if ( nsvg__isShapeInGroup(shape, IconNameX.c_str()) )
    {
      if (BootCampStyle && IconNameX.contains("selection_big")) {
        shape->opacity = 0.f;
      }
      if (XString8().takeValueFrom(shape->id).contains("BoundingRect")) {
        //there is bounds after nsvgParse()
        IconImageWidth = shape->bounds[2] - shape->bounds[0];
        IconImageHeight = shape->bounds[3] - shape->bounds[1];
  //      DBG("parsed bounds: %f, %f\n", IconImage.width, IconImage.height);
        if ( IconImageHeight < 1.f ) {
          IconImageHeight = 200.f;
        }
        if (IconNameX.contains("selection_big") && (!SelectionOnTop)) {
          MainEntriesSize = (int)(IconImageWidth * Scale); //xxx
          row0TileSize = MainEntriesSize + (int)(16.f * Scale);
  //        DBG("main entry size = %lld\n", MainEntriesSize);
        }
        if (IconNameX.contains("selection_small") && (!SelectionOnTop)) {
          row1TileSize = (int)(IconImageWidth * Scale);
        }

        // not exclude BoundingRect from IconImage?
        shape->flags = 0;  //invisible
        shape = shapeNext;
        continue; //while(shape) it is BoundingRect shape
      }
      shape->flags = NSVG_VIS_VISIBLE;
    } //the shape in the group
    shape = shapeNext;
  } //while shape


  if ( IconImageWidth == 0 || IconImageHeight == 0 ) {
    return Status;
  }

  float bounds[4];
  nsvg__imageBounds(SVGimage, bounds, IconNameX.c_str());

  if ((Id == BUILTIN_ICON_BANNER) && IconNameX.contains("Banner")) {
    BannerPosX = (int)(bounds[0] * Scale - CentreShift);
    if (BannerPosX < 0) {
      BannerPosX = 1; //one pixel
    }
    BannerPosY = (int)(bounds[1] * Scale);
//    DBG("Banner position at parse [%lld,%lld]\n", BannerPosX, BannerPosY);
  }

  float Height = IconImageHeight * Scale;
  float Width = IconImageWidth * Scale;
  if (Height < 0 || Width < 0) {
    return EFI_NOT_FOUND;
  }
 //   DBG("icon %s width=%f height=%f\n", IconNameX.c_str(), Width, Height);
  int iWidth = ((int)(Width+0.5f) + 7) & ~0x07u;
  int iHeight = ((int)(Height+0.5f) + 7) & ~0x07u;
  XImage NewImage(iWidth, iHeight); //empty

  float tx = 0.f, ty = 0.f;
  if ((Id != BUILTIN_ICON_BACKGROUND) &&
      (Id != BUILTIN_ICON_ANIME) &&
      !IconNameX.contains("Banner")) {
    float realWidth = (bounds[2] - bounds[0]) * Scale;
    float realHeight = (bounds[3] - bounds[1]) * Scale;
//        DBG("icon=%s width=%f realwidth=%f\n", IconNameX.c_str(), Width, realWidth);
    tx = (Width - realWidth) * 0.5f;
    ty = (Height - realHeight) * 0.5f;
  }

  NSVGrasterizer* rast = nsvg__createRasterizer();
  nsvgRasterize(rast, SVGimage, bounds, IconNameX.c_str(), tx, ty, Scale, Scale, (UINT8*)NewImage.GetPixelPtr(0,0), iWidth, iHeight, iWidth*4);
  nsvg__deleteRasterizer(rast);
  *Image = NewImage; //copy array
  
  return EFI_SUCCESS;
}

EFI_STATUS XTheme::ParseSVGXTheme(UINT8* buffer, UINTN Size)
{
  EFI_STATUS      Status;

  Icons.setEmpty();

displayFreeMemory("XTheme::ParseSVGXTheme begin"_XS8);

#if defined(JIEF_DEBUG) && defined(NANOSVG_MEMORY_ALLOCATION_TRACE)
if ( nsvg__nbDanglingPtr() > 0 ) {
  DBG("There is already dangling ptr. nano svg memory leak test not done\n");
}else{
  apd<char*> buffer2 = (char*)malloc(Size);
  memcpy(buffer2, buffer, Size);
  nvsg__memoryallocation_verbose = false;
  NSVGparser* p = nsvg__parse(buffer2, 72, 1.f); //the buffer will be modified, it is how nanosvg works
  nsvg__deleteParser(p);
  if ( nsvg__nbDanglingPtr() > 0 ) {
    nsvg__outputDanglingPtr();
    nvsg__memoryallocation_verbose = true;
    #if 1
      // Do it a second time, to display all allocations and to be able to step in with debugger
      memcpy(buffer2, buffer, Size);
      p = nsvg__parse(buffer2, 72, 1.f); //the buffer will be modified, it is how nanosvg works
      nsvg__deleteParser(p);
      nsvg__outputDanglingPtr();
    #endif
  }else{
    nvsg__memoryallocation_verbose = false; // be sure that nvsg__memoryallocation_verbose is false, as it seems there is no memory leaks
  }
}
#else
  (void)Size;
#endif

  // --- Parse theme.svg --- low case
  NSVGparser* SVGParser = nsvg__parse((CHAR8*)buffer, 72, 1.f); //the buffer will be modified, it is how nanosvg works// Jief : NEVER cast const to not const. Just change the parameter to not const !!! Nothing better to deceive.

  NSVGimage    *SVGimage = SVGParser->image;
  if (!SVGimage) {
 //   DBG("Theme not parsed!\n");
    return EFI_NOT_STARTED;
  }

  // --- Get scale as theme design height vs screen height

  // must be svg view-box. This is Design Width and Heigth
  float vbx = SVGParser->viewWidth;
  float vby = SVGParser->viewHeight;
//  DBG("Theme view-bounds: w=%f h=%f units=px\n", vbx, vby); //Theme view-bounds: w=1600.000000 h=900.000000 units=px
  if (vby > 1.0f) {
    SVGimage->height = vby;
  } else {
    SVGimage->height = 768.f;  //default height
  }
  float ScaleF = UGAHeight / SVGimage->height;
//  DBG("using scale %f\n", ScaleF); // using scale 0.666667
  Scale = ScaleF;
  CentreShift = (vbx * Scale - (float)UGAWidth) * 0.5f;

  Background = XImage(UGAWidth, UGAHeight);
  if (!BigBack.isEmpty()) {
    BigBack.setEmpty();
  }
  Status = EFI_NOT_FOUND;
  if (!Daylight) {
    Status = ParseSVGXIcon(SVGParser, BUILTIN_ICON_BACKGROUND, "Background_night"_XS8, &BigBack);
  }
  if (EFI_ERROR(Status)) {
    Status = ParseSVGXIcon(SVGParser, BUILTIN_ICON_BACKGROUND, "Background"_XS8, &BigBack);
  }

//  DBG(" Background parsed [%lld, %lld]\n", BigBack.GetWidth(), BigBack.GetHeight()); //Background parsed [1067, 133]
  // --- Make Banner
  Banner.setEmpty(); //for the case of theme switch
  Status = EFI_NOT_FOUND;
  if (!Daylight) {
    Status = ParseSVGXIcon(SVGParser, BUILTIN_ICON_BANNER, "Banner_night"_XS8, &Banner);
  }
  if (EFI_ERROR(Status)) {
    Status = ParseSVGXIcon(SVGParser, BUILTIN_ICON_BANNER, "Banner"_XS8, &Banner);
  }
//  DBG("Banner parsed\n");
  BanHeight = (int)(Banner.GetHeight() * Scale + 1.f);
//  DBG(" parsed banner->width=%lld height=%lld\n", Banner.GetWidth(), BanHeight); //parsed banner->width=467 height=89

  
  // --- Make other icons
  for (INTN i = BUILTIN_ICON_FUNC_ABOUT; i <= BUILTIN_CHECKBOX_CHECKED; ++i) {
    if (i == BUILTIN_ICON_BANNER) { //exclude "logo" as it done as Banner
      continue;
    }
    XIcon* NewIcon = new XIcon(i, false); //initialize without embedded
    Status = ParseSVGXIcon(SVGParser, i, NewIcon->Name, &NewIcon->Image);
//    DBG("parse %s status %s\n", NewIcon->Name.c_str(), efiStrError(Status));
    NewIcon->Native = !EFI_ERROR(Status);
    if (!EFI_ERROR(Status)) {
      ParseSVGXIcon(SVGParser, i, NewIcon->Name + "_night"_XS8, &NewIcon->ImageNight);
    }
 //   DBG("parse night %s status %s\n", NewIcon->Name.c_str(), efiStrError(Status));
    Icons.AddReference(NewIcon, true);
    if (EFI_ERROR(Status)) {
      if (i >= BUILTIN_ICON_VOL_INTERNAL_HFS && i <= BUILTIN_ICON_VOL_INTERNAL_REC) {
        // call to GetIconAlt will get alternate/embedded into Icon if missing
        GetIconAlt(i, BUILTIN_ICON_VOL_INTERNAL);
      } else if (i == BUILTIN_SELECTION_BIG) {
        GetIconAlt(i, BUILTIN_SELECTION_SMALL);
      }
    }
  }
  
  // --- Make other OSes
  for (INTN i = ICON_OTHER_OS; i < IconsNamesSize; ++i) {
    if (AsciiStrLen(IconsNames[i]) == 0) break;
    XIcon* NewIcon = new XIcon(i, false); //initialize without embedded
    Status = ParseSVGXIcon(SVGParser, i, NewIcon->Name, &NewIcon->Image);
//    DBG("parse %s i=%lld status %s\n", NewIcon->Name.c_str(), i, efiStrError(Status));
    NewIcon->Native = !EFI_ERROR(Status);
    if (!EFI_ERROR(Status)) {
      ParseSVGXIcon(SVGParser, i, NewIcon->Name + "_night"_XS8, &NewIcon->ImageNight);
    }
    Icons.AddReference(NewIcon, true);
  }
  //selection for bootcampstyle
  XIcon *NewIcon = new XIcon(BUILTIN_ICON_SELECTION);
  Status = ParseSVGXIcon(SVGParser, BUILTIN_ICON_SELECTION, "selection_indicator"_XS8, &NewIcon->Image);
  if (!EFI_ERROR(Status)) {
    Status = ParseSVGXIcon(SVGParser, BUILTIN_ICON_SELECTION, "selection_indicator_night"_XS8, &NewIcon->ImageNight);
  }
  Icons.AddReference(NewIcon, true);

  //selections
  SelectionBackgroundPixel.Red      = (SelectionColor >> 24) & 0xFF;
  SelectionBackgroundPixel.Green    = (SelectionColor >> 16) & 0xFF;
  SelectionBackgroundPixel.Blue     = (SelectionColor >> 8) & 0xFF;
  SelectionBackgroundPixel.Reserved = (SelectionColor >> 0) & 0xFF;
//TODO make SelectionImages to be XIcon
  SelectionImages[0] = GetIcon(BUILTIN_SELECTION_BIG).GetBest(!Daylight);
  SelectionImages[2] = GetIcon(BUILTIN_SELECTION_SMALL).GetBest(!Daylight);
  SelectionImages[4] = GetIcon(BUILTIN_ICON_SELECTION).GetBest(!Daylight);

  //buttons
  for (INTN i = BUILTIN_RADIO_BUTTON; i <= BUILTIN_CHECKBOX_CHECKED; ++i) {
    Buttons[i - BUILTIN_RADIO_BUTTON] = GetIcon(i).GetBest(!Daylight);
  }

  TypeSVG = true;
  ThemeDesignHeight = (int)SVGimage->height;
  ThemeDesignWidth = (int)SVGimage->width;
  if (SelectionOnTop) {
    row0TileSize = (INTN)(144.f * Scale);
    row1TileSize = (INTN)(64.f * Scale);
    MainEntriesSize = (INTN)(128.f * Scale);
  }
  // DBG("parsing svg theme finished\n");

  // It looks like the fonts are self-contained. So we can just keep fontsDB pointer and copy textfaces and delete the parser.
  // I'm not sure if font are self contained with all theme. To avoid deleting, just comment out the next line.
  // SVGParser will still be deleted at XTheme dtor. So it's not a memory leak.
  fontsDB = SVGParser->fontsDB;
  for (size_t i = 0; i < sizeof(textFace)/sizeof(textFace[0]); i++) {
    textFace[i] = SVGParser->textFace[i];
  }
  SVGParser->fontsDB = NULL; // To avoid nsvg__deleteParser to delete it;
  nsvg__deleteParser(SVGParser); // comment out this line and the next to keep the parser memory, in case of doubt that font are dependent.
  SVGParser = NULL;

  displayFreeMemory("XTheme::ParseSVGXTheme end"_XS8);

  return EFI_SUCCESS;
}

// 2023-11 This is currently never called.
EFI_STATUS XTheme::LoadSvgFrame(NSVGparser* SVGParser, INTN i, OUT XImage* XFrame)
{
  EFI_STATUS Status = EFI_NOT_FOUND;
  XString8 XFrameName = S8Printf("frame_%04lld", i+1);
  Status = ParseSVGXIcon(SVGParser, BUILTIN_ICON_ANIME, XFrameName, XFrame); //svg anime will be full redesigned
  if (EFI_ERROR(Status)) {
    DBG("frame '%s' not loaded, status=%s\n", XFrameName.c_str(), efiStrError(Status));
  }

  return Status;
}

// it is not draw, it is render and mainly used in egRenderText
// which is used in icns.cpp as an icon replacement if no image found, looks like not used
// in menu.cpp 3 places
//textType = 0-help 1-message 2-menu 3-test
//return text width in pixels
//it is not theme member!
INTN renderSVGtext(XImage* TextBufferXY_ptr, INTN posX, INTN posY, const textFaces& textFace, const XStringW& string, UINTN Cursor)
{
  XImage& TextBufferXY = *TextBufferXY_ptr;
  INTN Width;

  NSVGparser* p;
  NSVGrasterizer* rast;

  if (!textFace.valid) {
    DBG("invalid fontface!\n");
    return 0;
  }
  NSVGfont* fontSVG = textFace.font;
  UINT32 color = textFace.color;
  INTN Height = (INTN)(textFace.size * ThemeX->Scale);
  float Scale, sy;
  float x, y;
  if (!fontSVG) {
    DBG("no font for renderSVGtext\n");
    return 0;
  }
  p = nsvg__createParser();
  if (!p) {
    return 0;
  }
  NSVGtext* text = (NSVGtext*)nsvg__alloczero(sizeof(NSVGtext), "renderSVGtext"_XS8); // use nsvg__alloczero method so it won't panic when it's freed.
  if (!text) {
    return 0;
  }
  text->font = fontSVG;
  text->fontColor = color;
  text->fontSize = (float)Height;
  nsvg__xformIdentity(text->xform);
  p->text = text;

  Width = TextBufferXY.GetWidth();
  if ( fontSVG->unitsPerEm < 1.f ) {
    fontSVG->unitsPerEm = 1000.f;
  }
  float fH = fontSVG->bbox[3] - fontSVG->bbox[1]; //1250
  if (fH == 0.f) {
    DBG("wrong font: %f\n", fontSVG->unitsPerEm);
    nsvg__dumpFloat("Font bbox", fontSVG->bbox, 4);
    fH = (fontSVG->unitsPerEm > 1.f) ? fontSVG->unitsPerEm : 1000.0f;  //1000
  }
  sy = (float)Height / fH; //(float)fontSVG->unitsPerEm; // 260./1250.
  Scale = sy;
  x = (float)posX; //0.f;
  y = (float)posY + fontSVG->bbox[1] * Scale;
  p->isText = true;
  size_t len = string.length();
  for (size_t i=0; i < len; i++) {
    CHAR16 letter = string.char16At(i);
    if (!letter) {
      break;
    }
 //       DBG("add letter 0x%X\n", letter);
    if (i == Cursor) {
      nsvg__addLetter(p, 0x5F, x, y, sy, color);
    }
    x = nsvg__addLetter(p, letter, x, y, sy, color);
  } //end of string

  p->image->realBounds[0] = fontSVG->bbox[0] * Scale;
  p->image->realBounds[1] = fontSVG->bbox[1] * Scale;
  p->image->realBounds[2] = fontSVG->bbox[2] * Scale + x; //last bound
  p->image->realBounds[3] = fontSVG->bbox[3] * Scale;
  rast = nsvg__createRasterizer();
  nsvgRasterize(rast, p->image, 0, 0, 1.f, 1.f, (UINT8*)TextBufferXY.GetPixelPtr(0,0),
                (int)TextBufferXY.GetWidth(), (int)TextBufferXY.GetHeight(), (int)(Width*4));
  float RealWidth = p->image->realBounds[2] - p->image->realBounds[0];

  nsvg__deleteRasterizer(rast);
  nsvg__deleteParser(p); // this deletes p->text;
//  nsvgDelete(p->image);
  // TODO delete parser p and p->text?
  return (INTN)RealWidth; //x;
}


INTN renderSVGtext(XImage* TextBufferXY_ptr, INTN posX, INTN posY, INTN textType, const XStringW& string, UINTN Cursor)
{
  if (!ThemeX->getTextFace(textType).valid) {
    for (decltype(textType) i=0; i<4; i++) {
      if (ThemeX->getTextFace(i).valid) {
        textType = i;
        break;
      }
    }
  }
  if (!ThemeX->getTextFace(textType).valid) {
    DBG("valid fontface not found!\n");
    return 0;
  }
  return renderSVGtext(TextBufferXY_ptr, posX, posY, ThemeX->getTextFace(textType), string, Cursor);
}

void testSVG()
{
  do {

    EFI_STATUS      Status;
    UINT8           *FileData = NULL;
    UINTN           FileDataLength = 0;

    INTN Width = 192, Height = 192;

#if TEST_MATH
    //Test mathematique
//#define fabsf(x) ((x >= 0.0f)?x:(-x))
#define pr(x) (int)fabsf(x), (int)fabsf((x - (int)x) * 1000000.0f)
    int i;
    float x, y1, y2;
    //          CHAR8 Str[128];
    DBG("Test float: -%d.%06d\n", pr(-0.7612f));
    for (i=0; i<15; i++) {
      x=(PI)/30.0f*i;
      y1=SinF(x);
      y2=CosF(x);

      DBG("x=%d: %d.%06d ", i*6, pr(x));
      DBG("  sinx=%c%d.%06d", (y1<0)?'-':' ', pr(y1));
      DBG("  cosx=%c%d.%06d\n", (y2<0)?'-':' ',pr(y2));
      y1 = Atan2F(y1, y2);
      DBG("  atan2x=%c%d.%06d", (y1<0)?'-':' ',pr(y1));
      y1 = AcosF(y2);
      DBG("  acos=%c%d.%06d", (y1<0)?'-':' ',pr(y1));
      y1 = SqrtF(x);
      DBG("  sqrt=%d.%06d", pr(y1));
      y1 = CeilF(x);
      DBG("  ceil=%c%d.%06d\n", (y1<0)?'-':' ',pr(y1));
    }
#undef pr
#endif
    NSVGparser* p;
#if TEST_DITHER
    {
      EG_IMAGE        *RndImage = egCreateImage(256, 256, false);
      INTN i,j;
      EG_PIXEL pixel = WhitePixel;
      for (i=0; i<256; i++) {
        for (j=0; j<256; j++) {
          pixel.b = 0x40 + (dither((float)j / 32.0f, 1) * 8);
          pixel.r = 0x0;
          pixel.g = 0x0;
//          if (i==1) {
//            DBG("r=%X g=%X\n", pixel.r, pixel.g);
//          }
          RndImage->PixelData[i * 256 + j] = pixel;
        }
      }

      BltImageAlpha(RndImage,
                    20,
                    20,
                    &MenuBackgroundPixel,
                    16);
    }
#endif
#if TEST_SVG_IMAGE

    NSVGrasterizer* rast = nsvg__createRasterizer();
//    EG_IMAGE        *NewImage;
    NSVGimage       *SVGimage;
    float Scale, ScaleX, ScaleY;

    // load file
    Status = egLoadFile(&self.getSelfVolumeRootDir(), L"Sample.svg", &FileData, &FileDataLength);
    if (!EFI_ERROR(Status)) {
      //Parse XML to vector data

      p = nsvg__parse((CHAR8*)FileData, 72, 1.f);
      SVGimage = p->image;
      DBG("Test image width=%d heigth=%d\n", (int)(SVGimage->width), (int)(SVGimage->height));

      // Rasterize
      XImage NewImage(Width, Height);
      if (SVGimage->width <= 0) SVGimage->width = (float)Width;
      if (SVGimage->height <= 0) SVGimage->height = (float)Height;

      ScaleX = Width / SVGimage->width;
      ScaleY = Height / SVGimage->height;
      Scale = (ScaleX > ScaleY)?ScaleY:ScaleX;
      float tx = 0; //-SVGimage->realBounds[0] * Scale;
      float ty = 0; //-SVGimage->realBounds[1] * Scale;
		DBG("timing rasterize start tx=%f ty=%f\n", tx, ty); //the aim is measure duration
      nsvgRasterize(rast, SVGimage, tx,ty,Scale,Scale, (UINT8*)NewImage.GetPixelPtr(0,0), (int)Width, (int)Height, (int)Width*4);
      DBG("timing rasterize end\n");
      NewImage.Draw((UGAWidth - Width) / 2,
                (UGAHeight - Height) / 2);
      FreePool(FileData);
      FileData = NULL;
//
//      nsvg__deleteParser(p);
      nsvg__deleteRasterizer(rast);

    }

#endif
    //Test text
    Height = 80;
    Width = UGAWidth-200;
//    DBG("create test textbuffer\n");
    XImage TextBufferXY(Width, Height);
    Status = egLoadFile(&self.getSelfVolumeRootDir(), L"Font.svg", &FileData, &FileDataLength);
    DBG("test Font.svg loaded status=%s\n", efiStrError(Status));
    if (!EFI_ERROR(Status)) {
      p = nsvg__parse((CHAR8*)FileData, 72, 1.f);
      if (!p) {
        DBG("font not parsed\n");
        break;
      }

      textFaces textFace;
      textFace.font = p->currentFont;
      textFace.color = NSVG_RGBA(0x80, 0xFF, 0, 255);
      textFace.size = Height;
      textFace.valid = true;
//      DBG("font parsed family=%s\n", p->font->fontFamily);
      FreePool(FileData);
      //   Scale = Height / fontSVG->unitsPerEm;

      renderSVGtext(&TextBufferXY, 0, 0, 3, XStringW().takeValueFrom("Clover Кловер"), 1);
//      DBG("text ready to blit\n");
      TextBufferXY.Draw((UGAWidth - Width) / 2,
                (UGAHeight - Height) / 2);
//      nsvg__deleteParser(p);
//      DBG("draw finished\n");
    }
  } while (0);

}
