/*
 * Additional procedures for vector theme support
 *
 * Slice, 2018
 *
 */


#include "nanosvg.h"
#include "FloatLib.h"

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

#define TEST_MATH 0
#define TEST_SVG_IMAGE 1
#define TEST_SIZEOF 0


#define NSVG_RGB(r, g, b) (((unsigned int)b) | ((unsigned int)g << 8) | ((unsigned int)r << 16))
//#define NSVG_RGBA(r, g, b, a) (((unsigned int)b) | ((unsigned int)g << 8) | ((unsigned int)r << 16) | ((unsigned int)a << 24))

EFI_STATUS ParseSVGTheme(CONST CHAR8* buffer, TagPtr * dict, UINT32 bufSize)
{
  EFI_STATUS      Status;
  NSVGparser  *p, *p1 = NULL;
  NSVGfont    *fontSVG;
  NSVGimage   *SVGimage;
  NSVGtext    *text;
  NSVGshape   *shape;
  NSVGgroup   *group;
  float Scale = 1.0f;

  p = nsvgParse((CHAR8*)buffer, "px", 72);
  SVGimage = p->image;
  DBG("Image width=%d heigth=%d\n", (int)(SVGimage->width), (int)(SVGimage->height));
  DBG("Image viewBox: w=%d h=%d units=%a\n", 1, 1, "px");
  if (SVGimage->height == 0) {
    SVGimage->height = 768.f;  //default height
  }
  Scale = UGAHeight / SVGimage->height;
  DBG("using scale %s\n", PoolPrintFloat(Scale));
  fontSVG = p->font;
  if (fontSVG) {
    DBG("found embedded font-name=%a\n", fontSVG->fontFamily);
  }
  text = p->text;
  if (text) {
    UINT8           *FileData = NULL;
    UINTN           FileDataLength = 0;
    
    DBG("text uses font-name=%a\n", text->font->fontFamily);
    DBG("text uses font size=%d\n", (int)text->font->fontSize);
    if (!fontSVG || strcmp(fontSVG->fontFamily, text->font->fontFamily) != 0) {
      Status = egLoadFile(ThemeDir, PoolPrint(L"%a.svg", text->font->fontFamily), &FileData, &FileDataLength);
      DBG("font loaded status=%r\n", Status);
      if (!EFI_ERROR(Status)) {
        p1 = nsvgParse((CHAR8*)FileData, "px", 72);
        if (!p1) {
          DBG("font not parsed\n");
        } else {
          fontSVG = p1->font;
          DBG("font parsed\n");
          
        }
        FreePool(FileData);
      }
    }
  }
  /*
  typedef struct NSVGimage
  {
    float width;        // Width of the image.
    float height;        // Height of the image.
    float realBounds[4];
    NSVGshape* shapes;      // Linked list of shapes in the image.
    NSVGgroup* groups;			// Linked list of all groups in the image
    BOOLEAN isFont;
    NSVGclipPath* clipPaths;
  } NSVGimage;
*/
  NSVGimage *Banner = (NSVGimage*)AllocateZeroPool(sizeof(NSVGimage));
  NSVGshape *shapeNext, *shapePrev = NULL;
  
  shape = SVGimage->shapes;
  while (shape) {    
    group = shape->group;

    shapeNext = shape->next;
    while (group)
    {
      if (strcmp(group->id, "Banner") == 0) {
        break;
      }
      group = group->parent;
    }
    
    if (group) {
      DBG("found shape %a", shape->id);
      DBG(" from group %a\n", group->id);
      if (strstr(shape->id, "BoundingRect") != NULL) {
        Banner->width = shape->bounds[2] - shape->bounds[0];
        Banner->height = shape->bounds[3] - shape->bounds[1];
        memcpy(Banner->realBounds, shape->bounds, 4*sizeof(float));
        if (!Banner->height) {
          Banner->height = 200;
        }
        DBG("Banner size [%d,%d]\n", (int)Banner->width, (int)Banner->height);
        shape->flags = 0;  //invisible
      }
      shape->next = Banner->shapes; //add to head
      Banner->shapes = shape;
      if (shapePrev) {
        shapePrev->next = shapeNext;
      }
    } else {
      shapePrev = shape;
    }
    shape = shapeNext;
  }
  
  //suppose Banner will be 2-30% of Height
  float Height = UGAHeight * 0.28f;
  float scale = Height / Banner->height;
  float Width = Banner->height * scale;
  EG_IMAGE        *NewImage;
  NewImage = egCreateImage((int)Width, (int)Height, TRUE);
  DBG("new banner size=[%d,%d]\n", (int)Width, (int)Height);
  NSVGrasterizer* rast = nsvgCreateRasterizer();
  float tx = -Banner->realBounds[0] * Scale;
  float ty = -Banner->realBounds[1] * Scale;
  
  nsvgRasterize(rast, Banner, tx,ty,Scale,Scale, (UINT8*)NewImage->PixelData, (int)Width, (int)Height, (int)Width*4, NULL, NULL);
  //now show it!
  
  BltImageAlpha(NewImage,
                (int)(UGAWidth - Width) / 4,
                (int)(UGAHeight * 0.05f),
                &MenuBackgroundPixel,
                16);
  
  egFreeImage(NewImage);
  nsvg__deleteParser(p);
  if (p1) {
    nsvg__deleteParser(p1);
  }
  if (fontSVG) {
    nsvg__deleteFont(fontSVG);
  }
  nsvgDelete(Banner);
  nsvgDeleteRasterizer(rast);
  
  return EFI_NOT_AVAILABLE_YET;
}

VOID drawSVGtext(EG_IMAGE* TextBufferXY, VOID* font, CONST CHAR16* text)
{
  INTN Width, Height;
  int i;
  UINTN len;
  NSVGparser* p;
  NSVGshape *shape; //, *cur, *prev;
  NSVGrasterizer* rast;
  NSVGfont* fontSVG = (NSVGfont*)font;
  float Scale;
  INTN x, y;
  if (!fontSVG) {
    DBG("no font\n");
    return;
  }
  if (!TextBufferXY) {
    DBG("no buffer\n");
    return;
  }
  p = nsvg__createParser();
  if (!p) {
    DBG("no parser\n");
    return;
  }
  len = StrLen(text);
  Width = TextBufferXY->Width;
  Height = TextBufferXY->Height;
//  Height = 180; //for test
  DBG("textBuffer: [%d,%d], fontHight=%d\n", Width, TextBufferXY->Height, fontSVG->unitsPerEm);
  if (!fontSVG->unitsPerEm) {
    fontSVG->unitsPerEm = 1000;
  }
  float fH = fontSVG->bbox[3] - fontSVG->bbox[1];
  if (fH == 0.f) {
    fH = (float)fontSVG->unitsPerEm;
  }
  Scale = (float)Height / fH; //(float)fontSVG->unitsPerEm; //
  //in font units
  x = 0;
  y = 0;
  for (i=0; i < len; i++) {
    CHAR16 letter = text[i];
    NSVGglyph* g;
    if (!letter) {
      break;
    }

    shape = (NSVGshape*)AllocateZeroPool(sizeof(NSVGshape));
    if (shape == NULL) return;
    shape->strokeWidth = 1.2f;

    g = fontSVG->glyphs;
    while (g) {
      if (g->unicode == letter) {
        shape->paths = g->path;
        DBG("Found letter %x, point[0]=(%d,%d)\n", letter,
            (int)shape->paths->pts[0], (int)shape->paths->pts[1]);
        break;
      }
      g = g->next;
    }
    if (!g) {
      //missing glyph
      NSVGglyph* g = fontSVG->missingGlyph;
      shape->paths = g->path;
      DBG("Missing letter %x, path[0]=%d\n", letter, (int)shape->paths->pts[0]);
    }
    if (!shape->paths) {
      if (g) {
        x += g->horizAdvX;
      }
      if (shape) {
        FreePool(shape);
      }
      continue;
    }
    //fill shape
    shape->id[0] = (char)(letter & 0xff);
    shape->id[1] = (char)((letter >> 8) & 0xff);
    shape->fill.type = NSVG_PAINT_COLOR;
    shape->fill.color = NSVG_RGBA(5, 100, 0, 255); //dark green
    shape->stroke.type = NSVG_PAINT_COLOR;
    shape->stroke.color = NSVG_RGBA(0,0,0, 255); //black?
    shape->strokeWidth = 2.0f;
    shape->flags = NSVG_FLAGS_VISIBLE;
    nsvg__xformIdentity(shape->xform);
    shape->xform[0] = 1.f;
    shape->xform[3] = -1.f; //glyphs are mirrored by Y
    shape->xform[4] = (float)x - fontSVG->bbox[0];
    shape->xform[5] = (float)y - fontSVG->bbox[3]; // Y2 is a floor for a letter
//    DumpFloat(shape->xform, 6);
    //in glyph units
    shape->bounds[0] = fontSVG->bbox[0];
    shape->bounds[1] = fontSVG->bbox[1];
    shape->bounds[2] = fontSVG->bbox[2];
    shape->bounds[3] = fontSVG->bbox[3];

    x += g->horizAdvX; //position for next letter
    shape->strokeLineJoin = NSVG_JOIN_MITER;
    shape->strokeLineCap = NSVG_CAP_BUTT;
    shape->stroke.color = (255<<24); //black non transparent
    shape->miterLimit = 4;
    shape->fillRule = NSVG_FILLRULE_NONZERO;
    shape->opacity = 1;

    // Add to tail
    if (p->image->shapes == NULL)
      p->image->shapes = shape;
    else
      p->shapesTail->next = shape;
    p->shapesTail = shape;
  }
  p->image->realBounds[0] = fontSVG->bbox[0];
  p->image->realBounds[1] = fontSVG->bbox[1];
  p->image->realBounds[2] = fontSVG->bbox[2] + x; //last bound
  p->image->realBounds[3] = fontSVG->bbox[3];

  //We made an image, then rasterize it
  rast = nsvgCreateRasterizer();
  DBG("begin raster text\n");
  nsvgRasterize(rast, p->image, 0, 0, Scale, Scale, (UINT8*)TextBufferXY->PixelData, (int)Width, (int)Height, (int)(Width*4), NULL, NULL);
  DBG("end raster text\n");
  nsvgDeleteRasterizer(rast);
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
#define fabsf(x) ((x >= 0.0f)?x:(-x))
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
#if TEST_SVG_IMAGE
    NSVGrasterizer* rast = nsvgCreateRasterizer();
    EG_IMAGE        *NewImage;
    NSVGimage       *SVGimage;
    float Scale, ScaleX, ScaleY;
    // load file
    Status = egLoadFile(SelfRootDir, L"Sample.svg", &FileData, &FileDataLength);
    if (!EFI_ERROR(Status)) {
      //Parse XML to vector data
      
      p = nsvgParse((CHAR8*)FileData, "px", 72);
      SVGimage = p->image;
      DBG("Image width=%d heigth=%d\n", (int)(SVGimage->width), (int)(SVGimage->height));
      FreePool(FileData);
      
      // Rasterize
      NewImage = egCreateImage(Width, Height, TRUE);
      if (SVGimage->width <= 0) SVGimage->width = Width;
      if (SVGimage->height <= 0) SVGimage->height = Height;
      
      ScaleX = Width / SVGimage->width;
      ScaleY = Height / SVGimage->height;
      Scale = (ScaleX > ScaleY)?ScaleY:ScaleX;
      float tx = -SVGimage->realBounds[0] * Scale;
      float ty = -SVGimage->realBounds[1] * Scale;
      DBG("timing rasterize start tx=%s ty=%s\n", PoolPrintFloat(tx), PoolPrintFloat(ty));
      nsvgRasterize(rast, SVGimage, tx,ty,Scale,Scale, (UINT8*)NewImage->PixelData, (int)Width, (int)Height, (int)Width*4, NULL, NULL);
      DBG("timing rasterize end\n");
      //now show it!
      BltImageAlpha(NewImage,
                    (UGAWidth - Width) / 2,
                    (UGAHeight - Height) / 2,
                    &MenuBackgroundPixel,
                    16);
      egFreeImage(NewImage);
      nsvg__deleteParser(p);
      nsvgDeleteRasterizer(rast);
    }
#endif
    //Test text
    Height = 260;
    Width = UGAWidth-200;
    DBG("create textbuffer\n");
    EG_IMAGE* TextBufferXY = egCreateFilledImage(Width, Height, TRUE, &MenuBackgroundPixel);
    Status = egLoadFile(SelfRootDir, L"Font.svg", &FileData, &FileDataLength);
    DBG("font loaded status=%r\n", Status);
    if (!EFI_ERROR(Status)) {
      p = nsvgParse((CHAR8*)FileData, "px", 72);
      if (!p) {
        DBG("font not parsed\n");
        break;
      }
      NSVGfont* fontSVG = p->font;
      DBG("font parsed\n");
      FreePool(FileData);
      //   Scale = Height / fontSVG->unitsPerEm;
      drawSVGtext(TextBufferXY, fontSVG, L"Clover");
      DBG("text ready to blit\n");
      BltImageAlpha(TextBufferXY,
                    (UGAWidth - Width) / 2,
                    (UGAHeight - Height) / 2,
                    &MenuBackgroundPixel,
                    16);
      egFreeImage(TextBufferXY);
      nsvg__deleteParser(p);
      DBG("draw finished\n");
    }
  } while (0);

}
