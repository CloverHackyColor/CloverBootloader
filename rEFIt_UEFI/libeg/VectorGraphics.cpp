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

#include "../Platform/Platform.h"

#include "nanosvg.h"
#include "FloatLib.h"
#include "lodepng.h"
#include "../refit/screen.h"
#include "../cpp_foundation/XString.h"

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
extern XTheme ThemeX;
extern CONST CHAR8* IconsNames[];


#define NSVG_RGB(r, g, b) (((unsigned int)b) | ((unsigned int)g << 8) | ((unsigned int)r << 16))
//#define NSVG_RGBA(r, g, b, a) (((unsigned int)b) | ((unsigned int)g << 8) | ((unsigned int)r << 16) | ((unsigned int)a << 24))

extern VOID
WaitForKeyPress(CHAR16 *Message);

extern void DumpFloat2 (CONST char* s, float* t, int N);

extern UINTN NumFrames;
extern UINTN FrameTime;
extern BOOLEAN DayLight;


textFaces       textFace[4]; //0-help 1-message 2-menu 3-test, far future it will be infinite list with id

EFI_STATUS XTheme::ParseSVGXIcon(INTN Id, const XString& IconNameX, XImage* Image)
{
  EFI_STATUS      Status = EFI_NOT_FOUND;
  NSVGimage       *SVGimage;
  NSVGparser *p = (NSVGparser *)SVGParser;
  NSVGrasterizer* rast = nsvgCreateRasterizer();
  SVGimage = p->image;
  NSVGshape   *shape;
  NSVGgroup   *group;
  NSVGimage   *IconImage;
  NSVGshape   *shapeNext, *shapesTail = NULL, *shapePrev;
//  CONST CHAR8 *IconName = IconNameX.c_str();

  NSVGparser* p2 = nsvg__createParser();
  IconImage = p2->image;
  shape = SVGimage->shapes;
  shapePrev = NULL;
  while (shape) {
    group = shape->group;
    shapeNext = shape->next;
    while (group) {
      if (strcmp(group->id, IconNameX.c_str()) == 0) {
        break;
      }
      group = group->next;
    }
    if (group) { //the shape is in the group
      // keep this sample for debug purpose
/*    DBG("found shape %s", shape->id);
      DBG(" from group %s\n", group->id);
      if ((Id == BUILTIN_SELECTION_BIG) ||
          (Id == BUILTIN_ICON_BACKGROUND) ||
          (Id == BUILTIN_ICON_BANNER)) {
        shape->debug = TRUE;
      } */
//      if (BootCampStyle && (strstr(IconName, "selection_big") != NULL)) {
//        shape->opacity = 0.f;
//      }
      if (BootCampStyle && IconNameX.ExistIn("selection_big")) {
        shape->opacity = 0.f;
      }
//      if (strstr(shape->id, "BoundingRect") != NULL) {
      if (XString().takeValueFrom(shape->id).ExistIn("BoundingRect")) {
        //there is bounds after nsvgParse()
        IconImage->width = shape->bounds[2] - shape->bounds[0];
        IconImage->height = shape->bounds[3] - shape->bounds[1];
  //      DBG("parsed bounds: %f, %f\n", IconImage->width, IconImage->height);
        if ( IconImage->height < 1.f ) {
          IconImage->height = 200.f;
        }
        if (IconNameX.ExistIn("selection_big") && (!SelectionOnTop)) {
          MainEntriesSize = (int)(IconImage->width * Scale); //xxx
          row0TileSize = MainEntriesSize + (int)(16.f * Scale);
          DBG("main entry size = %lld\n", MainEntriesSize);
        }
        if (IconNameX.ExistIn("selection_small") && (!SelectionOnTop)) {
          row1TileSize = (int)(IconImage->width * Scale);
        }

        // not exclude BoundingRect from IconImage?
        shape->flags = 0;  //invisible
        if (shapePrev) {
          shapePrev->next = shapeNext;
        }
        else {
          SVGimage->shapes = shapeNext;
        }
        shape = shapeNext;
        continue; //while(shape) it is BoundingRect shape

//        shape->opacity = 0.3f;
      }
      shape->flags = NSVG_VIS_VISIBLE;
      // Add to tail
//      ClipCount += shape->clip.count;
      if (IconImage->shapes == NULL)
        IconImage->shapes = shape;
      else
        shapesTail->next = shape;
      shapesTail = shape;
      if (shapePrev) {
        shapePrev->next = shapeNext;
      }
      else {
        SVGimage->shapes = shapeNext;
      }
      shapePrev->next = shapeNext;
    } //the shape in the group
    else {
      shapePrev = shape;
    }
    shape = shapeNext;
  } //while shape
  shapesTail->next = NULL;
  //add clipPaths  //xxx
  NSVGclipPath* clipPaths = SVGimage->clipPaths;
  NSVGclipPath* clipNext = NULL;
  while (clipPaths) {
    //   ClipCount += clipPaths->shapes->clip.count;
    if (!clipPaths->shapes) {
      break;
    }
    group = clipPaths->shapes->group;
    clipNext = clipPaths->next;
    while (group) {
 //     if (strcmp(group->id, IconNameX.c_str()) == 0) {
 //       break;
 //     }
      if (IconNameX == XString().takeValueFrom(group->id)) {
        break;
      }
      group = group->parent;
    }
    if (group) {
      DBG("found clipPaths for %s\n", IconNameX.c_str());
      IconImage->clipPaths = SVGimage->clipPaths;
      break;
    }
    clipPaths = clipNext;
  }
  //  DBG("found %d clips for %s\n", ClipCount, IconName);
  //  if (ClipCount) { //Id == BUILTIN_ICON_BANNER) {
  //    IconImage->clipPaths = SVGimage->clipPaths;
  //  }

  float bounds[4];
  bounds[0] = FLT_MAX;
  bounds[1] = FLT_MAX;
  bounds[2] = -FLT_MAX;
  bounds[3] = -FLT_MAX;
  nsvg__imageBounds(p2, bounds);
  CopyMem(IconImage->realBounds, bounds, 4 * sizeof(float));
  if ((Id == BUILTIN_ICON_BANNER) && IconNameX.ExistIn("Banner")) {
    BannerPosX = (int)(bounds[0] * Scale - CentreShift);
    if (BannerPosX < 0) {
      BannerPosX = 1; //one pixel
    }
    BannerPosY = (int)(bounds[1] * Scale);
    DBG("Banner position at parse [%lld,%lld]\n", BannerPosX, BannerPosY);
  }

  float Height = IconImage->height * Scale;
  float Width = IconImage->width * Scale;
//    DBG("icon %s width=%f height=%f\n", IconNameX.c_str(), Width, Height);
  int iWidth = (int)(Width + 0.5f);
  int iHeight = (int)(Height + 0.5f);
//  EG_IMAGE  *NewImage = egCreateFilledImage(iWidth, iHeight, TRUE, &MenuBackgroundPixel);
  XImage NewImage(iWidth, iHeight); //empty
  if (IconImage->shapes == NULL) {
    *Image = NewImage;
 //   DBG("return empty with status=%s\n", strerror(Status));
    return Status;
  }

//    DBG("begin rasterize %s\n", IconNameX.c_str());
  float tx = 0.f, ty = 0.f;
  if ((Id != BUILTIN_ICON_BACKGROUND) &&
      (Id != BUILTIN_ICON_ANIME) &&
      IconNameX.ExistIn("Banner")) {
    float realWidth = (bounds[2] - bounds[0]) * Scale;
    float realHeight = (bounds[3] - bounds[1]) * Scale;
    tx = (Width - realWidth) * 0.5f;
    ty = (Height - realHeight) * 0.5f;
  }

  nsvgRasterize(rast, IconImage, tx, ty, Scale, Scale, (UINT8*)NewImage.GetPixelPtr(0,0), iWidth, iHeight, iWidth*4);
  //  DBG("%s rastered, blt\n", IconImage);

  nsvgDeleteRasterizer(rast);
  //  nsvg__deleteParser(p2);
  //  nsvgDelete(p2->image);
  *Image = NewImage;

  return EFI_SUCCESS;
}

EFI_STATUS XTheme::ParseSVGXTheme(CONST CHAR8* buffer)
{
  EFI_STATUS      Status;
  NSVGimage       *SVGimage;
  NSVGparser     *mainParser;

  Icons.Empty();

  // --- Parse theme.svg --- low case
  SVGParser = (void *)nsvgParse((CHAR8*)buffer, 72, 1.f); //the buffer will be modified, it is how nanosvg works
  mainParser = (NSVGparser*)SVGParser;
  SVGimage = mainParser->image;
  if (!SVGimage) {
    DBG("Theme not parsed!\n");
    return EFI_NOT_STARTED;
  }

  // --- Get scale as theme design height vs screen height

  // must be svg view-box
  float vbx = mainParser->viewWidth;
  float vby = mainParser->viewHeight;
  DBG("Theme view-bounds: w=%f h=%f units=px\n", vbx, vby); //Theme view-bounds: w=1600.000000 h=900.000000 units=px
  if (vby > 1.0f) {
    SVGimage->height = vby;
  } else {
    SVGimage->height = 768.f;  //default height
  }
  float ScaleF = UGAHeight / SVGimage->height;
  DBG("using scale %f\n", ScaleF); // using scale 0.666667
  Scale = ScaleF;
  CentreShift = (vbx * Scale - (float)UGAWidth) * 0.5f;

//  if (mainParser->font) { //this is strange like last found font
//    DBG("theme contains font-family=%s\n", mainParser->font->fontFamily);
//  }

  Background = XImage(UGAWidth, UGAHeight);
  if (!BigBack.isEmpty()) {
    BigBack.setEmpty();
  }
  Status = EFI_NOT_FOUND;
  if (!DayLight) {
    Status = ParseSVGXIcon(BUILTIN_ICON_BACKGROUND, "Background_night"_XS, &BigBack);
  }
  if (EFI_ERROR(Status)) {
    Status = ParseSVGXIcon(BUILTIN_ICON_BACKGROUND, "Background"_XS, &BigBack);
  }
  DBG(" Background parsed [%lld, %lld]\n", BigBack.GetWidth(), BigBack.GetHeight()); //Background parsed [1067, 133]
  // --- Make Banner
  Banner.setEmpty(); //for the case of theme switch
  Status = EFI_NOT_FOUND;
  if (!DayLight) {
    Status = ParseSVGXIcon(BUILTIN_ICON_BANNER, "Banner_night"_XS, &Banner);
  }
  if (EFI_ERROR(Status)) {
    Status = ParseSVGXIcon(BUILTIN_ICON_BANNER, "Banner"_XS, &Banner);
  }
  DBG("Banner parsed\n");
  BanHeight = (int)(Banner.GetHeight() * Scale + 1.f);
  DBG(" parsed banner->width=%lld height=%lld\n", Banner.GetWidth(), BanHeight); //parsed banner->width=467 height=89
  
  // --- Make other icons

  for (INTN i = BUILTIN_ICON_FUNC_ABOUT; i <= BUILTIN_CHECKBOX_CHECKED; ++i) {
    if (i == BUILTIN_ICON_BANNER) { //exclude "logo" as it done other way
      continue;
    }
    Icon* NewIcon = new Icon(i, false); //initialize without embedded
    Status = ParseSVGXIcon(i, NewIcon->Name, &NewIcon->Image);
    if (EFI_ERROR(Status) &&
        (i >= BUILTIN_ICON_VOL_INTERNAL_HFS) &&
        (i <= BUILTIN_ICON_VOL_INTERNAL_REC)) {
      NewIcon->Image = GetIcon(BUILTIN_ICON_VOL_INTERNAL); //copy existing
    }
//    DBG("parse %s status %s\n", NewIcon->Name.c_str(), strerror(Status));
    Status = ParseSVGXIcon(i, NewIcon->Name + "_night"_XS, &NewIcon->ImageNight);
//    DBG("...night status %s\n", strerror(Status));
    if (EFI_ERROR(Status) &&
        (i >= BUILTIN_ICON_VOL_INTERNAL_HFS) &&
        (i <= BUILTIN_ICON_VOL_INTERNAL_REC)) {
      NewIcon->ImageNight = GetIcon(BUILTIN_ICON_VOL_INTERNAL); //copy existing
    }
    Icons.AddReference(NewIcon, true);
  }

  //selections
  SelectionBackgroundPixel.Red      = (SelectionColor >> 24) & 0xFF;
  SelectionBackgroundPixel.Green    = (SelectionColor >> 16) & 0xFF;
  SelectionBackgroundPixel.Blue     = (SelectionColor >> 8) & 0xFF;
  SelectionBackgroundPixel.Reserved = (SelectionColor >> 0) & 0xFF;

  SelectionImages[0] = GetIcon(BUILTIN_SELECTION_BIG);
  SelectionImages[2] = GetIcon(BUILTIN_SELECTION_SMALL);

  //selection for bootcamp style
  Status = EFI_NOT_FOUND;
  if (!DayLight) {
    Status = ParseSVGXIcon(BUILTIN_ICON_SELECTION, "selection_indicator_night"_XS, &SelectionImages[4]);
  }
  if (EFI_ERROR(Status)) {
    Status = ParseSVGXIcon(BUILTIN_ICON_SELECTION, "selection_indicator"_XS, &SelectionImages[4]);
  }

  //buttons
  for (INTN i = BUILTIN_RADIO_BUTTON; i <= BUILTIN_CHECKBOX_CHECKED; ++i) {
    Buttons[i - BUILTIN_RADIO_BUTTON] = GetIcon(i);
  }
  //for (int i=0 ; i<6 ; i+=2 ) {
  //SelectionImages[i].Draw(i*100, 0);
  //}

  //TODO parse anime like for PNG themes
  /*
  Dict = GetProperty (DictPointer, "Anime");
  if (Dict != NULL) {
    INTN  Count = GetTagCount (Dict);
    for (INTN i = 0; i < Count; i++) {
      FILM *NewFilm = new FILM();
      if (EFI_ERROR (GetElement (Dict, i, &Dict3))) {
        continue;
      }
      if (Dict3 == NULL) {
        break;
      }
      Dict2 = GetProperty (Dict3, "ID");
      NewFilm->SetIndex((UINTN)GetPropertyInteger (Dict2, 1)); //default=main screen

      Dict2 = GetProperty (Dict3, "Path");
      if (Dict2 != NULL && (Dict2->type == kTagTypeString) && Dict2->string) {
        NewFilm->Path.takeValueFrom(Dict2->string);
      }

      Dict2 = GetProperty (Dict3, "Frames");
      NewFilm->NumFrames = (UINTN)GetPropertyInteger (Dict2, 0);

      Dict2 = GetProperty (Dict3, "FrameTime");
      NewFilm->FrameTime = (UINTN)GetPropertyInteger (Dict2, 50); //default will be 50ms

      Dict2 = GetProperty (Dict3, "ScreenEdgeX");
      if (Dict2 != NULL && (Dict2->type == kTagTypeString) && Dict2->string) {
        if (AsciiStrCmp (Dict2->string, "left") == 0) {
          NewFilm->ScreenEdgeHorizontal = SCREEN_EDGE_LEFT;
        } else if (AsciiStrCmp (Dict2->string, "right") == 0) {
          NewFilm->ScreenEdgeHorizontal = SCREEN_EDGE_RIGHT;
        }
      }

      Dict2 = GetProperty (Dict3, "ScreenEdgeY");
      if (Dict2 != NULL && (Dict2->type == kTagTypeString) && Dict2->string) {
        if (AsciiStrCmp (Dict2->string, "top") == 0) {
          NewFilm->ScreenEdgeVertical = SCREEN_EDGE_TOP;
        } else if (AsciiStrCmp (Dict2->string, "bottom") == 0) {
          NewFilm->ScreenEdgeVertical = SCREEN_EDGE_BOTTOM;
        }
      }

      //default values are centre

      Dict2 = GetProperty (Dict3, "DistanceFromScreenEdgeX%");
      NewFilm->FilmX = GetPropertyInteger (Dict2, INITVALUE);

      Dict2 = GetProperty (Dict3, "DistanceFromScreenEdgeY%");
      NewFilm->FilmY = GetPropertyInteger (Dict2, INITVALUE);

      Dict2 = GetProperty (Dict3, "NudgeX");
      NewFilm->NudgeX = GetPropertyInteger (Dict2, INITVALUE);

      Dict2 = GetProperty (Dict3, "NudgeY");
      NewFilm->NudgeY = GetPropertyInteger (Dict2, INITVALUE);

      Dict2 = GetProperty (Dict3, "Once");
      NewFilm->RunOnce = IsPropertyTrue (Dict2);

      NewFilm->GetFrames(ThemeX); //used properties: ID, Path, NumFrames
      ThemeX.Cinema.AddFilm(NewFilm);
      //     delete NewFilm; //looks like already deleted
    }
  }

*/

//  nsvgDeleteRasterizer(rast);
  
  TypeSVG = TRUE;
  ThemeDesignHeight = (int)SVGimage->height;
  ThemeDesignWidth = (int)SVGimage->width;
  if (SelectionOnTop) {
    row0TileSize = (INTN)(144.f * Scale);
    row1TileSize = (INTN)(64.f * Scale);
    MainEntriesSize = (INTN)(128.f * Scale);
  }
 // DBG("parsing svg theme finished\n");

  return Status;
}

EFI_STATUS XTheme::LoadSvgFrame(INTN i, OUT XImage* XFrame)
{
  EFI_STATUS Status = EFI_NOT_FOUND;
  XString XFrameName = SPrintf("frame_%04lld", i+1);
  Status = ParseSVGXIcon(BUILTIN_ICON_ANIME, XFrameName, XFrame);
  if (EFI_ERROR(Status)) {
    DBG("frame '%s' not loaded, status=%s\n", XFrameName.c_str(), strerror(Status));
  }

  return Status;
}

// it is not draw, it is render and mainly used in egRenderText
// which is used in icns.cpp as an icon replacement if no image found, looks like not used
// in menu.cpp 3 places
//textType = 0-help 1-message 2-menu 3-test
//return text width in pixels
//it is not theme member!
INTN renderSVGtext(XImage* TextBufferXY_ptr, INTN posX, INTN posY, INTN textType, const XStringW& string, UINTN Cursor)
{
  XImage& TextBufferXY = *TextBufferXY_ptr;
  INTN Width;
//  UINTN i;
  UINTN len;
  NSVGparser* p;
  NSVGrasterizer* rast;
  if (!textFace[textType].valid) {
    for (decltype(textType) i=0; i<4; i++) {
      if (textFace[i].valid) {
        textType = i;
        break;
      }
    }
  }
  if (!textFace[textType].valid) {
    DBG("valid fontface not found!\n");
    return 0;
  }
  NSVGfont* fontSVG = textFace[textType].font;
  UINT32 color = textFace[textType].color;
  INTN Height = (INTN)(textFace[textType].size * ThemeX.Scale);
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
  NSVGtext* text = (NSVGtext*)AllocateZeroPool(sizeof(NSVGtext));
  if (!text) {
    return 0;
  }
  text->font = fontSVG;
  text->fontColor = color;
  text->fontSize = (float)Height;
  nsvg__xformIdentity(text->xform);
  p->text = text;

  len = string.size();
  Width = TextBufferXY.GetWidth();
  if ( fontSVG->unitsPerEm < 1.f ) {
    fontSVG->unitsPerEm = 1000.f;
  }
  float fH = fontSVG->bbox[3] - fontSVG->bbox[1]; //1250
  if (fH == 0.f) {
    DBG("wrong font: %f\n", fontSVG->unitsPerEm);
    DumpFloat2("Font bbox", fontSVG->bbox, 4);
    fH = (fontSVG->unitsPerEm > 1.f) ? fontSVG->unitsPerEm : 1000.0f;  //1000
  }
  sy = (float)Height / fH; //(float)fontSVG->unitsPerEm; // 260./1250.
  Scale = sy;
  x = (float)posX; //0.f;
  y = (float)posY + fontSVG->bbox[1] * Scale;
  p->isText = TRUE;
  for (UINTN i=0; i < len; i++) {
    CHAR16 letter = string.wc_str()[i];
    if (!letter) {
      break;
    }
 //       DBG("add letter 0x%X\n", letter);
    if (i == Cursor) {
      addLetter(p, 0x5F, x, y, sy, color);
    }
    x = addLetter(p, letter, x, y, sy, color);
  } //end of string

  p->image->realBounds[0] = fontSVG->bbox[0] * Scale;
  p->image->realBounds[1] = fontSVG->bbox[1] * Scale;
  p->image->realBounds[2] = fontSVG->bbox[2] * Scale + x; //last bound
  p->image->realBounds[3] = fontSVG->bbox[3] * Scale;
  rast = nsvgCreateRasterizer();
  nsvgRasterize(rast, p->image, 0, 0, 1.f, 1.f, (UINT8*)TextBufferXY.GetPixelPtr(0,0),
                (int)TextBufferXY.GetWidth(), (int)TextBufferXY.GetHeight(), (int)(Width*4));
  float RealWidth = p->image->realBounds[2] - p->image->realBounds[0];

  nsvgDeleteRasterizer(rast);
  //  nsvg__deleteParser(p);
  nsvgDelete(p->image);
  return (INTN)RealWidth; //x;
}
VOID testSVG()
{
  do {

    EFI_STATUS      Status;
    UINT8           *FileData = NULL;
    UINTN           FileDataLength = 0;

    INTN Width = 400, Height = 400;

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
      EG_IMAGE        *RndImage = egCreateImage(256, 256, FALSE);
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
    NSVGrasterizer* rast = nsvgCreateRasterizer();
//    EG_IMAGE        *NewImage;
    NSVGimage       *SVGimage;
    float Scale, ScaleX, ScaleY;

    // load file
    Status = egLoadFile(SelfRootDir, L"Sample.svg", &FileData, &FileDataLength);
    if (!EFI_ERROR(Status)) {
      //Parse XML to vector data

      p = nsvgParse((CHAR8*)FileData, 72, 1.f);
      SVGimage = p->image;
      DBG("Test image width=%d heigth=%d\n", (int)(SVGimage->width), (int)(SVGimage->height));
//      FreePool(FileData);
/*
      if (p->patterns && p->patterns->image) {
        BltImageAlpha((EG_IMAGE*)(p->patterns->image),
                      40,
                      40,
                      &MenuBackgroundPixel,
                      16);
      }
*/
      // Rasterize
      XImage NewImage(Width, Height);
      if (SVGimage->width <= 0) SVGimage->width = (float)Width;
      if (SVGimage->height <= 0) SVGimage->height = (float)Height;

      ScaleX = Width / SVGimage->width;
      ScaleY = Height / SVGimage->height;
      Scale = (ScaleX > ScaleY)?ScaleY:ScaleX;
      float tx = 0; //-SVGimage->realBounds[0] * Scale;
      float ty = 0; //-SVGimage->realBounds[1] * Scale;
		DBG("timing rasterize start tx=%f ty=%f\n", tx, ty);
      nsvgRasterize(rast, SVGimage, tx,ty,Scale,Scale, (UINT8*)NewImage.GetPixelPtr(0,0), (int)Width, (int)Height, (int)Width*4);
      DBG("timing rasterize end\n");
      NewImage.Draw((UGAWidth - Width) / 2,
                (UGAHeight - Height) / 2);
      FreePool(FileData);
      FileData = NULL;
//
//      nsvg__deleteParser(p);
      nsvgDeleteRasterizer(rast);

    }

#endif
    //Test text
    Height = 80;
    Width = UGAWidth-200;
//    DBG("create test textbuffer\n");
    XImage TextBufferXY(Width, Height);
    Status = egLoadFile(SelfRootDir, L"Font.svg", &FileData, &FileDataLength);
    DBG("test Font.svg loaded status=%s\n", strerror(Status));
    if (!EFI_ERROR(Status)) {
      p = nsvgParse((CHAR8*)FileData, 72, 1.f);
      if (!p) {
        DBG("font not parsed\n");
        break;
      }
//     NSVGfont* fontSVG = p->font;
      textFace[3].font = p->font;
      textFace[3].color = NSVG_RGBA(0x80, 0xFF, 0, 255);
      textFace[3].size = Height;
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
