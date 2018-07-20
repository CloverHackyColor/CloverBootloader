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


#define NSVG_RGB(r, g, b) (((unsigned int)b) | ((unsigned int)g << 8) | ((unsigned int)r << 16))

VOID drawSVGtext(EG_IMAGE* TextBufferXY, NSVGfont* fontSVG, const CHAR16* text)
{
  INTN Width, Height;
  int i;
  UINTN len;
  NSVGparser* p;
  NSVGshape *shape; //, *cur, *prev;
  NSVGrasterizer* rast;
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
    shape->fill.color = NSVG_RGB(5, 100, 0) | (255<<24); //dark green
    shape->stroke.type = NSVG_PAINT_COLOR;
    shape->stroke.color = NSVG_RGB(0,0,0) | (255<<24); //black?
    shape->strokeWidth = 2.0f;
    shape->flags = NSVG_VIS_DISPLAY | NSVG_FLAGS_VISIBLE;
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

