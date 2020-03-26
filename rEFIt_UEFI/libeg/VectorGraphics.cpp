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

#if USE_XTHEME
#include "XTheme.h"
extern XTheme ThemeX;
#endif


#define NSVG_RGB(r, g, b) (((unsigned int)b) | ((unsigned int)g << 8) | ((unsigned int)r << 16))
//#define NSVG_RGBA(r, g, b, a) (((unsigned int)b) | ((unsigned int)g << 8) | ((unsigned int)r << 16) | ((unsigned int)a << 24))

extern VOID
WaitForKeyPress(CHAR16 *Message);

extern void DumpFloat2 (CONST char* s, float* t, int N);
extern EG_IMAGE *BackgroundImage;
extern EG_IMAGE *Banner;
extern EG_IMAGE *BigBack;
extern INTN BanHeight;
extern INTN row0TileSize;
extern INTN row1TileSize;
extern INTN FontWidth;
extern UINTN NumFrames;
extern UINTN FrameTime;
extern BOOLEAN DayLight;

textFaces       textFace[4]; //0-help 1-message 2-menu 3-test
NSVGparser      *mainParser = NULL;  //it must be global variable

#if USE_XTHEME
EFI_STATUS ParseSVGXIcon(NSVGparser  *p, INTN Id, CONST CHAR8 *IconName, float Scale, OUT XImage&  Image)
{
  EFI_STATUS      Status = EFI_NOT_FOUND;
  NSVGimage       *SVGimage;
  NSVGrasterizer* rast = nsvgCreateRasterizer();
  SVGimage = p->image;
  NSVGshape   *shape;
  NSVGgroup   *group;
  NSVGimage   *IconImage;
  NSVGshape   *shapeNext, *shapesTail = NULL, *shapePrev;

  NSVGparser* p2 = nsvg__createParser();
  IconImage = p2->image;

  shape = SVGimage->shapes;
  shapePrev = NULL;
  while (shape) {
    group = shape->group;
    shapeNext = shape->next;
    while (group) {
      if (strcmp(group->id, IconName) == 0) {
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
      if (ThemeX.BootCampStyle && (strstr(IconName, "selection_big") != NULL)) {
        shape->opacity = 0.f;
      }
      if (strstr(shape->id, "BoundingRect") != NULL) {
        //there is bounds after nsvgParse()
        IconImage->width = shape->bounds[2] - shape->bounds[0];
        IconImage->height = shape->bounds[3] - shape->bounds[1];
        if (!IconImage->height) {
          IconImage->height = 200;
        }

        if ((strstr(IconName, "selection_big") != NULL) && (!ThemeX.SelectionOnTop)) {
          ThemeX.MainEntriesSize = (int)(IconImage->width * Scale); //xxx
          ThemeX.row0TileSize = ThemeX.MainEntriesSize + (int)(16.f * Scale);
          DBG("main entry size = %lld\n", ThemeX.MainEntriesSize);
        }
        if ((strstr(IconName, "selection_small") != NULL) && (!ThemeX.SelectionOnTop)) {
          ThemeX.row1TileSize = (int)(IconImage->width * Scale);
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
    group = clipPaths->shapes->group;
    clipNext = clipPaths->next;
    while (group) {
      if (strcmp(group->id, IconName) == 0) {
        break;
      }
      group = group->parent;
    }
    if (group) {
      DBG("found clipPaths for %s\n", IconName);
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
  if ((Id == BUILTIN_ICON_BANNER) && (strstr(IconName, "Banner") != NULL)) {
    ThemeX.BannerPosX = (int)(bounds[0] * Scale - ThemeX.CentreShift);
    ThemeX.BannerPosY = (int)(bounds[1] * Scale);
    DBG("Banner position at parse [%lld,%lld]\n", ThemeX.BannerPosX, ThemeX.BannerPosY);
  }

  float Height = IconImage->height * Scale;
  float Width = IconImage->width * Scale;
  //  DBG("icon %s width=%f height=%f\n", IconName, Width, Height);
  int iWidth = (int)(Width + 0.5f);
  int iHeight = (int)(Height + 0.5f);
//  EG_IMAGE  *NewImage = egCreateFilledImage(iWidth, iHeight, TRUE, &MenuBackgroundPixel);
  XImage NewImage(iWidth, iHeight); //empty
  if (IconImage->shapes == NULL) {
    Image = NewImage;
    return Status;
  }

  //  DBG("begin rasterize %s\n", IconName);
  float tx = 0.f, ty = 0.f;
  if ((Id != BUILTIN_ICON_BACKGROUND) &&
      (Id != BUILTIN_ICON_ANIME) &&
      (strstr(IconName, "Banner") == NULL)) {
    float realWidth = (bounds[2] - bounds[0]) * Scale;
    float realHeight = (bounds[3] - bounds[1]) * Scale;
    tx = (Width - realWidth) * 0.5f;
    ty = (Height - realHeight) * 0.5f;
  }

  nsvgRasterize(rast, IconImage, tx,ty,Scale,Scale, (UINT8*)NewImage.GetPixelPtr(0,0), iWidth, iHeight, iWidth*4);
  //  DBG("%s rastered, blt\n", IconImage);

  nsvgDeleteRasterizer(rast);
  //  nsvg__deleteParser(p2);
  //  nsvgDelete(p2->image);
  Image = NewImage;

  return EFI_SUCCESS;
}
#endif


EFI_STATUS ParseSVGIcon(NSVGparser  *p, INTN Id, CONST CHAR8 *IconName, float Scale, EG_IMAGE  **Image)
{
  EFI_STATUS      Status = EFI_NOT_FOUND;
  NSVGimage       *SVGimage;
  NSVGrasterizer* rast = nsvgCreateRasterizer();
  SVGimage = p->image;
  NSVGshape   *shape;
  NSVGgroup   *group;
  NSVGimage *IconImage; // = (NSVGimage*)AllocateZeroPool(sizeof(NSVGimage));
  NSVGshape *shapeNext, *shapesTail=NULL, *shapePrev;
//  INTN ClipCount = 0;

  NSVGparser* p2 = nsvg__createParser();
  IconImage = p2->image;

  shape = SVGimage->shapes;
  shapePrev = NULL;
  while (shape) {
    group = shape->group;
    shapeNext = shape->next;
    while (group) {
      if (strcmp(group->id, IconName) == 0) {
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
      if (GlobalConfig.BootCampStyle && (strstr(IconName, "selection_big") != NULL)) {
        shape->opacity = 0.f;
      }
      if (strstr(shape->id, "BoundingRect") != NULL) {
        //there is bounds after nsvgParse()
        IconImage->width = shape->bounds[2] - shape->bounds[0];
        IconImage->height = shape->bounds[3] - shape->bounds[1];
        if (!IconImage->height) {
          IconImage->height = 200;
        }
 //       if (Id == BUILTIN_ICON_BACKGROUND || Id == BUILTIN_ICON_BANNER) {
 //         DBG("IconImage size [%d,%d]\n", (int)IconImage->width, (int)IconImage->height);
 //         DBG("IconImage left corner x=%f y=%f\n", IconImage->realBounds[0], IconImage->realBounds[1]);
 //         DumpFloat2("IconImage real bounds", IconImage->realBounds, 4);
 //       }
        if ((strstr(IconName, "selection_big") != NULL) && (!GlobalConfig.SelectionOnTop)) {
          GlobalConfig.MainEntriesSize = (int)(IconImage->width * Scale); //xxx
          row0TileSize = GlobalConfig.MainEntriesSize + (int)(16.f * Scale);
			DBG("main entry size = %lld\n", GlobalConfig.MainEntriesSize);
        }
         if ((strstr(IconName, "selection_small") != NULL) && (!GlobalConfig.SelectionOnTop)) {
          row1TileSize = (int)(IconImage->width * Scale);
        }

// not exclude BoundingRect from IconImage?
        shape->flags = 0;  //invisible
        if (shapePrev) {
          shapePrev->next = shapeNext;
        } else {
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
      } else {
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
    group = clipPaths->shapes->group;
    clipNext = clipPaths->next;
    while (group) {
      if (strcmp(group->id, IconName) == 0) {
        break;
      }
      group = group->parent;
    }
    if (group) {
      DBG("found clipPaths for %s\n", IconName);
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
  CopyMem(IconImage->realBounds, bounds, 4*sizeof(float));

  if ((Id == BUILTIN_ICON_BANNER) && (strstr(IconName, "Banner") != NULL)) {
    GlobalConfig.BannerPosX = (int)(bounds[0] * Scale - GlobalConfig.CentreShift);
    GlobalConfig.BannerPosY = (int)(bounds[1] * Scale);
	  DBG("Banner position at parse [%lld,%lld]\n", GlobalConfig.BannerPosX, GlobalConfig.BannerPosY);
  }

  float Height = IconImage->height * Scale;
  float Width = IconImage->width * Scale;
//  DBG("icon %s width=%f height=%f\n", IconName, Width, Height);
  int iWidth = (int)(Width + 0.5f);
  int iHeight = (int)(Height + 0.5f);
  EG_IMAGE  *NewImage = egCreateFilledImage(iWidth, iHeight, TRUE, &MenuBackgroundPixel);

  if (IconImage->shapes == NULL) {
    *Image = NewImage;
    return Status;
  }

//  DBG("begin rasterize %s\n", IconName);
  float tx = 0.f, ty = 0.f;
  if ((Id != BUILTIN_ICON_BACKGROUND) &&
      (Id != BUILTIN_ICON_ANIME) &&
      (strstr(IconName, "Banner") == NULL)) {
    float realWidth = (bounds[2] - bounds[0]) * Scale;
    float realHeight = (bounds[3] - bounds[1]) * Scale;
    tx = (Width - realWidth) * 0.5f;
    ty = (Height - realHeight) * 0.5f;
  }

  nsvgRasterize(rast, IconImage, tx,ty,Scale,Scale, (UINT8*)NewImage->PixelData, iWidth, iHeight, iWidth*4);
//  DBG("%s rastered, blt\n", IconImage);
#if 0
  BltImageAlpha(NewImage,
                (int)(UGAWidth - NewImage->Width) / 2,
                (int)(UGAHeight * 0.05f),
                &MenuBackgroundPixel,
                16);
//  WaitForKeyPress(L"waiting for key press...\n");
#endif
  nsvgDeleteRasterizer(rast);
//  nsvg__deleteParser(p2);
//  nsvgDelete(p2->image);
  *Image = NewImage;
  return EFI_SUCCESS;
}


EFI_STATUS ParseSVGTheme(CONST CHAR8* buffer, TagPtr * dict)
{
  EFI_STATUS Status;
  NSVGimage       *SVGimage;
  NSVGrasterizer  *rast = nsvgCreateRasterizer();

// --- Parse theme.svg --- low case
  mainParser = nsvgParse((CHAR8*)buffer, 72, 1.f);
  SVGimage = mainParser->image;
  if (!SVGimage) {
    DBG("Theme not parsed!\n");
    return EFI_NOT_STARTED;
  }

// --- Get scale as theme design height vs screen height
  float Scale;
  // must be svg view-box
  float vbx = mainParser->viewWidth;
  float vby = mainParser->viewHeight;
  DBG("Theme view-bounds: w=%d h=%d units=%s\n", (int)vbx, (int)vby, "px");
    if (vby > 1.0f) {
      SVGimage->height = vby;
    } else {
      SVGimage->height = 768.f;  //default height
    }
  Scale = UGAHeight / SVGimage->height;
	DBG("using scale %f\n", Scale);
  GlobalConfig.Scale = Scale;
  GlobalConfig.CentreShift = (vbx * Scale - (float)UGAWidth) * 0.5f;

  if (mainParser->font) {
    DBG("theme contains font-family=%s\n", mainParser->font->fontFamily);
  }

#if 0
// --- Create rastered font
  if (fontSVG) {
    if (p->font) {
      FontHeight = (int)(textFace[2].size * Scale); //as in MenuRows
      DBG("Menu font scaled height=%d color=%X\n", FontHeight, textFace[2].color);
    }
    if (!FontHeight) FontHeight = 16;  //xxx
    if (fontSVG->fontFamily[0] < 0x30) {
      AsciiStrCpyS(fontSVG->fontFamily, 64, fontSVG->id);
    }
    RenderSVGfont(fontSVG, p->fontColor);
    DBG("font %s parsed\n", fontSVG->fontFamily);
  }
#endif

// --- Make background
  BackgroundImage = egCreateFilledImage(UGAWidth, UGAHeight, TRUE, &BlackPixel);
  if (BigBack) {
    egFreeImage(BigBack);
    BigBack = NULL;
  }
  Status = EFI_NOT_FOUND;
  if (!DayLight) {
    Status = ParseSVGIcon(mainParser, BUILTIN_ICON_BACKGROUND, "Background_night", Scale, &BigBack);
  }
  if (EFI_ERROR(Status)) {
    Status = ParseSVGIcon(mainParser, BUILTIN_ICON_BACKGROUND, "Background", Scale, &BigBack);
  }
  DBG("background parsed\n");

// --- Make Banner
  if (Banner) {
    egFreeImage(Banner);
    Banner = NULL;
  }
  Status = EFI_NOT_FOUND;
  if (!DayLight) {
    Status = ParseSVGIcon(mainParser, BUILTIN_ICON_BANNER, "Banner_night", Scale, &Banner);
  }
  if (EFI_ERROR(Status)) {
    Status = ParseSVGIcon(mainParser, BUILTIN_ICON_BANNER, "Banner", Scale, &Banner);
  }
  DBG("Banner parsed\n");

  BuiltinIconTable[BUILTIN_ICON_BANNER].Image = Banner;
  BanHeight = (int)(Banner->Height * Scale + 1.f);
	DBG("parsed banner->width=%lld\n", Banner->Width);

// --- Make other icons
  INTN i = BUILTIN_ICON_FUNC_ABOUT;
  CHAR8           *IconName;
  while (BuiltinIconTable[i].Path) {
    if (i == BUILTIN_ICON_BANNER) {
      i++;
      continue;
    }
    CONST CHAR16 *IconPath = BuiltinIconTable[i].Path;
//    DBG("next table icon=%ls\n", IconPath);
    CONST CHAR16 *ptr = StrStr(IconPath, L"\\");
    if (!ptr) {
      ptr = IconPath;
    } else {
      ptr++;
    }
 //   DBG("next icon=%ls Len=%d\n", ptr, StrLen(ptr));
    UINTN Size = StrLen(ptr)+1;
    IconName = (__typeof__(IconName))AllocateZeroPool(Size);
    UnicodeStrToAsciiStrS(ptr, IconName, Size);
//    DBG("search for icon name %s\n", IconName);
    CHAR8 IconNight[64];
    AsciiStrCpyS(IconNight, 64, IconName);
    AsciiStrCatS(IconNight, 64, "_night");
    Status = EFI_NOT_FOUND;
    if (!DayLight) {
      Status = ParseSVGIcon(mainParser, i, IconNight, Scale, &BuiltinIconTable[i].Image);
    }
    if (EFI_ERROR(Status)) {
      Status = ParseSVGIcon(mainParser, i, IconName, Scale, &BuiltinIconTable[i].Image);
    }
    if (EFI_ERROR(Status)) {
		DBG(" icon %lld not parsed take common %ls\n", i, BuiltinIconTable[i].Path);
      if ((i >= BUILTIN_ICON_VOL_EXTERNAL) && (i <= BUILTIN_ICON_VOL_INTERNAL_REC)) {
        if (BuiltinIconTable[BUILTIN_ICON_VOL_INTERNAL].Image) {
          BuiltinIconTable[i].Image = egCopyImage(BuiltinIconTable[BUILTIN_ICON_VOL_INTERNAL].Image);
        }
      }
    }
    if (i == BUILTIN_SELECTION_BIG) {
		DBG("icon main size=[%lld,%lld]\n", BuiltinIconTable[i].Image->Width,
          BuiltinIconTable[i].Image->Height);
    }
    i++;
    FreePool(IconName);
  }

  // OS icons and buttons
  i = 0;
  while (OSIconsTable[i].name) {
    CHAR8 IconNight[64];
    AsciiStrCpyS(IconNight, 64, OSIconsTable[i].name);
    AsciiStrCatS(IconNight, 64, "_night");
    OSIconsTable[i].image = NULL;
 //   DBG("search for %s\n", OSIconsTable[i].name);
    Status = EFI_NOT_FOUND;
    if (!DayLight) {
      Status = ParseSVGIcon(mainParser, i, IconNight, Scale, &OSIconsTable[i].image);
    }
    if (EFI_ERROR(Status)) {
      Status = ParseSVGIcon(mainParser, i, OSIconsTable[i].name, Scale, &OSIconsTable[i].image);
    }
    if (EFI_ERROR(Status)) {
      DBG("OSicon %s not parsed\n", OSIconsTable[i].name);
      if ((i > 0) && (i < 13)) {
        if (OSIconsTable[0].image) {
          OSIconsTable[i].image = egCopyImage(OSIconsTable[0].image);
        }
      } else if (i < 18) {
        if (OSIconsTable[13].image) {
          OSIconsTable[i].image = egCopyImage(OSIconsTable[13].image);
        }
      }
    }
    i++;
  }

  //selection for bootcamp style
  Status = EFI_NOT_FOUND;
  if (!DayLight) {
    Status = ParseSVGIcon(mainParser, BUILTIN_ICON_SELECTION, "selection_indicator_night", Scale, &SelectionImages[4]);
  }
  if (EFI_ERROR(Status)) {
    Status = ParseSVGIcon(mainParser, BUILTIN_ICON_SELECTION, "selection_indicator", Scale, &SelectionImages[4]);
  }

  //banner animation
  GUI_ANIME *Anime = (__typeof__(Anime))AllocateZeroPool (sizeof(GUI_ANIME));
  Anime->ID = 1; //main screen
  //there is no Anime->Path in vectors
  Anime->Frames = NumFrames;
  Anime->FrameTime = FrameTime;
  Anime->Next = GuiAnime;
  Anime->FilmX = INITVALUE;
  Anime->FilmY = INITVALUE;
  Anime->NudgeX = INITVALUE;
  Anime->NudgeY = INITVALUE;
  GuiAnime = Anime;

  nsvgDeleteRasterizer(rast);

  *dict = (__typeof_am__(*dict))AllocateZeroPool(sizeof(TagStruct));
  (*dict)->type = kTagTypeNone;
  GlobalConfig.TypeSVG = TRUE;
  GlobalConfig.ThemeDesignHeight = (int)SVGimage->height;
  GlobalConfig.ThemeDesignWidth = (int)SVGimage->width;
  if (GlobalConfig.SelectionOnTop) {
    row0TileSize = (INTN)(144.f * Scale);
    row1TileSize = (INTN)(64.f * Scale);
    GlobalConfig.MainEntriesSize = (INTN)(128.f * Scale);
  }
  DBG("parsing theme finish\n");
#if 0 //dump fonts
  {
    NSVGfont        *fontSVG = NULL;
    NSVGfontChain   *fontChain = fontsDB;

    while (fontChain) {
      fontSVG = fontChain->font;
      if (fontSVG) {
        DBG("probe fontFamily=%s fontStyle=%c\n", fontSVG->fontFamily, fontSVG->fontStyle);
      }
      else {
        DBG("nextChain is empty\n");
      }
      fontChain = fontChain->next;
    }
  }
#endif
  return EFI_SUCCESS;
}

#if USE_XTHEME
EFI_STATUS ParseSVGXTheme(CONST CHAR8* buffer, TagPtr * dict)
{
  EFI_STATUS Status;
  NSVGimage       *SVGimage;
//  NSVGrasterizer  *rast = nsvgCreateRasterizer();

  // --- Parse theme.svg --- low case
  mainParser = nsvgParse((CHAR8*)buffer, 72, 1.f);
  SVGimage = mainParser->image;
  if (!SVGimage) {
    DBG("Theme not parsed!\n");
    return EFI_NOT_STARTED;
  }

  // --- Get scale as theme design height vs screen height

  // must be svg view-box
  float vbx = mainParser->viewWidth;
  float vby = mainParser->viewHeight;
  DBG("Theme view-bounds: w=%f h=%f units=px\n", vbx, vby);
  if (vby > 1.0f) {
    SVGimage->height = vby;
  }
  else {
    SVGimage->height = 768.f;  //default height
  }
  float Scale = UGAHeight / SVGimage->height;
  DBG("using scale %f\n", Scale);
  ThemeX.Scale = Scale;
  ThemeX.CentreShift = (vbx * Scale - (float)UGAWidth) * 0.5f;

  if (mainParser->font) {
    DBG("theme contains font-family=%s\n", mainParser->font->fontFamily);
  }

  ThemeX.Background = XImage(UGAWidth, UGAHeight);
  if (!ThemeX.BigBack.isEmpty()) {
    ThemeX.BigBack.setEmpty();
  }
  Status = EFI_NOT_FOUND;
  if (!DayLight) {
    Status = ParseSVGXIcon(mainParser, BUILTIN_ICON_BACKGROUND, "Background_night", Scale, ThemeX.BigBack);
  }
  if (EFI_ERROR(Status)) {
    Status = ParseSVGXIcon(mainParser, BUILTIN_ICON_BACKGROUND, "Background", Scale, ThemeX.BigBack);
  }
  DBG(" Background parsed\n");
  return Status;
}
#endif

EG_IMAGE * LoadSvgFrame(INTN i)
{
  EG_IMAGE  *Frame = NULL;
  EFI_STATUS Status;
  CHAR8 FrameName[64];
  //TODO if extend SVG syntax then we can use dynamic SVG with parameter Frame
  // for example use variable instead of constant like javascript
  AsciiSPrint(FrameName, 63, "frame_%d", i+1);
  Status = ParseSVGIcon(mainParser, BUILTIN_ICON_ANIME, FrameName, GlobalConfig.Scale, &Frame);
  if (EFI_ERROR(Status)) {
    DBG("icon '%s' not loaded, status=%s\n", FrameName, strerror(Status));
  }
  return Frame;
}

#if 0
VOID RenderSVGfont(NSVGfont  *fontSVG, UINT32 color)
{
//  EFI_STATUS      Status;
  float           FontScale;
  NSVGparser      *p;
  NSVGrasterizer  *rast;
  INTN i;
  if (!fontSVG) {
    return;
  }
  //free old font
  if (FontImage != NULL) {
    egFreeImage (FontImage);
    FontImage = NULL;
  }
  INTN Height = FontHeight + 4;
//  DBG("load font %s\n", fontSVG->fontFamily);
  if (fontSVG->unitsPerEm < 1.f) {
    fontSVG->unitsPerEm = 1000.f;
  }
  float fH = fontSVG->bbox[3] - fontSVG->bbox[1];
  if (fH == 0.f) {
    fH = fontSVG->unitsPerEm;
  }
  FontScale = (float)FontHeight / fH;
  DBG("font scale %ls\n", FontScale);
  FontWidth = (int)(fontSVG->horizAdvX * FontScale);
  INTN Width = FontWidth * (AsciiPageSize + GlobalConfig.CodepageSize);
  FontImage = egCreateImage(Width, Height, TRUE);

  p = nsvg__createParser();
  if (!p) {
    return;
  }
//  p->font = fontSVG;
  p->image->height = (float)Height;
  p->image->width = (float)Width;

  NSVGtext* text = (NSVGtext*)AllocateZeroPool(sizeof(NSVGtext));
  if (!text) {
    return;
  }
  text->fontSize = (float)FontHeight;
  text->font = fontSVG;
  text->fontColor = color;

//  DBG("RenderSVGfont: fontID=%s\n", text->font->id);
//  DBG("RenderSVGfont:  family=%s\n", text->font->fontFamily);
  //add to head
  text->next = p->text;
  p->text = text;
  //for each letter rasterize glyph into FontImage
  //0..0xC0 == AsciiPageSize
  // cyrillic 0x410..0x450 at 0xC0
  float x = 0.f;
  float y = fontSVG->bbox[1] * FontScale;; //(float)Height;
  p->isText = TRUE;
  for (i = 0; i < AsciiPageSize; i++) {
    addLetter(p, i, x, y, FontScale, color);
    x += (float)FontWidth;
  }
  x = AsciiPageSize * FontWidth;
  for (i = GlobalConfig.Codepage; i < GlobalConfig.Codepage+GlobalConfig.CodepageSize; i++) {
    addLetter(p, i, x, y, FontScale, color);
    x += (float)FontWidth;
  }
  p->image->realBounds[0] = fontSVG->bbox[0] * FontScale;
  p->image->realBounds[1] = fontSVG->bbox[1] * FontScale;
  p->image->realBounds[2] = fontSVG->bbox[2] * FontScale + x; //last bound
  p->image->realBounds[3] = fontSVG->bbox[3] * FontScale;

  //We made an image, then rasterize it
  rast = nsvgCreateRasterizer();
  nsvgRasterize(rast, p->image, 0, 0, 1.0f, 1.0f, (UINT8*)FontImage->PixelData, (int)Width, (int)Height, (int)(Width*4));

#if 0 //DEBUG_FONT
  //save font as png yyyyy
  UINT8           *FileData = NULL;
  UINTN           FileDataLength = 0U;

  EFI_UGA_PIXEL *ImagePNG = (EFI_UGA_PIXEL *)FontImage->PixelData;

  unsigned lode_return =
    eglodepng_encode(&FileData, &FileDataLength, (CONST UINT8*)ImagePNG, (UINTN)FontImage->Width, (UINTN)FontImage->Height);

  if (!lode_return) {
    egSaveFile(SelfRootDir, L"\\FontSVG.png", FileData, FileDataLength);
  }
#endif
  nsvgDeleteRasterizer(rast);
//  nsvg__deleteParser(p);
  return;
}
#endif

// it is not draw, it is render and mainly used in egRenderText
// which is used in icns.cpp as an icon rplacement if no image found, looks like not used
// in menu.cpp 3 places
//textType = 0-help 1-message 2-menu 3-test
//return text width in pixels
#if USE_XTHEME
INTN renderSVGtext(XImage& TextBufferXY, INTN posX, INTN posY, INTN textType, XStringW string, UINTN Cursor)
{
  INTN Width;
  UINTN i;
  UINTN len;
  NSVGparser* p;
  NSVGrasterizer* rast;
  if (!textFace[textType].valid) {
    for (i=0; i<4; i++) {
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

  len = string.length();
  Width = TextBufferXY.GetWidth();
  if (!fontSVG->unitsPerEm) {
    fontSVG->unitsPerEm = 1000.f;
  }
  float fH = fontSVG->bbox[3] - fontSVG->bbox[1]; //1250
  if (fH == 0.f) {
    DBG("wrong font: %f\n", fontSVG->unitsPerEm);
    DumpFloat2("Font bbox", fontSVG->bbox, 4);
    fH = fontSVG->unitsPerEm?fontSVG->unitsPerEm:1000.0f;  //1000
  }
  sy = (float)Height / fH; //(float)fontSVG->unitsPerEm; // 260./1250.
  Scale = sy;
  x = (float)posX; //0.f;
  y = (float)posY + fontSVG->bbox[1] * Scale;
  p->isText = TRUE;
  for (i=0; i < len; i++) {
    CHAR16 letter = string[i];
    if (!letter) {
      break;
    }
    //    DBG("add letter 0x%X\n", letter);
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
#else
INTN renderSVGtext(EG_IMAGE* TextBufferXY, INTN posX, INTN posY, INTN textType, CONST CHAR16* string, UINTN Cursor)
{
  INTN Width;
  UINTN i;
  UINTN len;
  NSVGparser* p;
  NSVGrasterizer* rast;
  if (!textFace[textType].valid) {
    for (i=0; i<4; i++) {
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
  INTN Height = (INTN)(textFace[textType].size * GlobalConfig.Scale);
  float Scale, sy;
  float x, y;
  if (!fontSVG) {
    DBG("no font for renderSVGtext\n");
    return 0;
  }
  if (!TextBufferXY) {
    DBG("no buffer\n");
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

  len = StrLen(string);
  Width = TextBufferXY->Width;
//  Height = TextBufferXY->Height;
//  DBG("Text Height=%d  Buffer Height=%d\n", Height, TextBufferXY->Height);

//  Height = 180; //for test
//  DBG("textBuffer: [%d,%d], fontUnits=%d\n", Width, TextBufferXY->Height, (int)fontSVG->unitsPerEm);
  if (!fontSVG->unitsPerEm) {
    fontSVG->unitsPerEm = 1000.f;
  }
  float fH = fontSVG->bbox[3] - fontSVG->bbox[1]; //1250
  if (fH == 0.f) {
	  DBG("wrong font: %f\n", fontSVG->unitsPerEm);
    DumpFloat2("Font bbox", fontSVG->bbox, 4);
    fH = fontSVG->unitsPerEm?fontSVG->unitsPerEm:1000.0f;  //1000
  }
  sy = (float)Height / fH; //(float)fontSVG->unitsPerEm; // 260./1250.
  //in font units
//  float fW = fontSVG->bbox[2] - fontSVG->bbox[0];
//  sx = (float)Width / (fW * len);
//  Scale = (sx > sy)?sy:sx;
  Scale = sy;
  x = (float)posX; //0.f;
  y = (float)posY + fontSVG->bbox[1] * Scale;
  p->isText = TRUE;

//DBG("renderSVGtext -> Enter. Text=%s\n", XString(string).c);
  for (i=0; i < len; i++) {
    CHAR16 letter = string[i];

    if (!letter) {
      break;
    }
//    DBG("add letter 0x%X\n", letter);
    if (i == Cursor) {
      addLetter(p, 0x5F, x, y, sy, color);
    }
    x = addLetter(p, letter, x, y, sy, color);
//    DBG("next x=%ls\n", x);
  } //end of string

  p->image->realBounds[0] = fontSVG->bbox[0] * Scale;
  p->image->realBounds[1] = fontSVG->bbox[1] * Scale;
  p->image->realBounds[2] = fontSVG->bbox[2] * Scale + x; //last bound
  p->image->realBounds[3] = fontSVG->bbox[3] * Scale;
//  DBG("internal Scale=%lf\n", Scale);
//  DumpFloat2("text bounds", p->image->realBounds, 4);
  //We made an image, then rasterize it
  rast = nsvgCreateRasterizer();
//  DBG("begin raster text, scale=%ls\n", Scale);
  nsvgRasterize(rast, p->image, 0, 0, 1.f, 1.f, (UINT8*)TextBufferXY->PixelData,
                (int)TextBufferXY->Width, (int)TextBufferXY->Height, (int)(Width*4));
  float RealWidth = p->image->realBounds[2] - p->image->realBounds[0];
//  DBG("end raster text\n");
  nsvgDeleteRasterizer(rast);
//  nsvg__deleteParser(p);
  nsvgDelete(p->image);
  return (INTN)RealWidth; //x;
}
#endif
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
    EG_IMAGE        *NewImage;
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
      NewImage = egCreateFilledImage(Width, Height, TRUE, &MenuBackgroundPixel);
      if (SVGimage->width <= 0) SVGimage->width = (float)Width;
      if (SVGimage->height <= 0) SVGimage->height = (float)Height;

      ScaleX = Width / SVGimage->width;
      ScaleY = Height / SVGimage->height;
      Scale = (ScaleX > ScaleY)?ScaleY:ScaleX;
      float tx = 0; //-SVGimage->realBounds[0] * Scale;
      float ty = 0; //-SVGimage->realBounds[1] * Scale;
		DBG("timing rasterize start tx=%f ty=%f\n", tx, ty);
      nsvgRasterize(rast, SVGimage, tx,ty,Scale,Scale, (UINT8*)NewImage->PixelData, (int)Width, (int)Height, (int)Width*4);
      DBG("timing rasterize end\n");
      //now show it!
#if 1 //test XImage
      XImage NewX(NewImage);
      NewX.Draw((UGAWidth - Width) / 2,
        (UGAHeight - Height) / 2);
#else
      BltImageAlpha(NewImage,
                    (UGAWidth - Width) / 2,
                    (UGAHeight - Height) / 2,
                    &MenuBackgroundPixel,
                    16);
#endif //test XImage
      FreePool(FileData);
      FileData = NULL;
      egFreeImage(NewImage);
//      nsvg__deleteParser(p);
      nsvgDeleteRasterizer(rast);

    }

#endif
    //Test text
    Height = 80;
    Width = UGAWidth-200;
    DBG("create test textbuffer\n");
#if USE_XTHEME
    XImage TextBufferXY(Width, Height);
#else
    EG_IMAGE* TextBufferXY = egCreateFilledImage(Width, Height, TRUE, &MenuBackgroundPixel);
#endif
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
#if USE_XTHEME
      renderSVGtext(TextBufferXY, 0, 0, 3, XStringW().takeValueFrom("Clover Кловер"), 1);
#else
      renderSVGtext(TextBufferXY, 0, 0, 3, L"Clover Кловер", 1);
#endif
//      DBG("text ready to blit\n");
#if USE_XTHEME
      TextBufferXY.Draw((UGAWidth - Width) / 2,
                (UGAHeight - Height) / 2);
#else
      BltImageAlpha(TextBufferXY,
                    (UGAWidth - Width) / 2,
                    (UGAHeight - Height) / 2,
                    &MenuBackgroundPixel,
                    16);
      egFreeImage(TextBufferXY);
#endif
//      nsvg__deleteParser(p);
//      DBG("draw finished\n");
    }
  } while (0);

}
