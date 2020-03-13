/*
 * Copyright (c) 2013-14 Mikko Mononen memon@inside.org
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * The SVG parser is based on Anti-Grain Geometry 2.4 SVG example
 * Copyright (C) 2002-2004 Maxim Shemanarev (McSeem) (http://www.antigrain.com/)
 *
 * Arc calculation code based on canvg (https://code.google.com/p/canvg/)
 *
 * Bounding box calculation based on http://blog.hackers-cafe.net/2009/06/how-to-calculate-bezier-curves-bounding.html
 *
 * aksdfauytv - support for recursive images
 * porglezomb - handle visibility
 * tpecholt, tesch1 - feature groups
 * boris-ulyanov, rzaumseil - style processing
 * jamislike - basic text parsing
 * darealshinji - multiple improvements
 * technosaurus - independent x-y scaling
 * olivierchatry - added suppport for stylesheets and other small improvements
 * JaimeIvanCervantes - idea for <use href>
 * MalcolmMcLean - forwards differencing to flatten Bezier, binary search color name - improve speed
 * poke1024 - fix handling of <defs> after <g>, add basic support for clip paths
 *
*/
// Adoptation to Clover project by Slice, 2018

#ifndef NANOSVG_H
#define NANOSVG_H

extern "C" {
#include <Library/BaseMemoryLib.h>
}
#include "Platform.h"

#define NANOSVG_ALL_COLOR_KEYWORDS 1
#define NSVG_RGBA(r, g, b, a) (((unsigned int)b) | ((unsigned int)g << 8) | ((unsigned int)r << 16) | ((unsigned int)a << 24))


//There are defines for compilation as first step. Must be revised
#define memcpy(dest,source,count) CopyMem(dest,(void*)(source),(UINTN)(count))
#define memset(dest,ch,count)     SetMem(dest,(UINTN)(count),(UINT8)(ch))
#define strcmp(a,b) AsciiStrCmp(a,b)
#define strncmp(a,b,n) AsciiStrnCmp(a,b,n)
#define strstr(a,b) AsciiStrStr(a,b)

#define strlen(s) AsciiStrLen(s)
#define strncpy(a,b,n) AsciiSPrint(a,n,"%a",b)

enum NSVGpaintType {
  NSVG_PAINT_NONE = 0,
  NSVG_PAINT_COLOR = 1,
  NSVG_PAINT_LINEAR_GRADIENT = 2,
  NSVG_PAINT_RADIAL_GRADIENT = 3,
  NSVG_PAINT_CONIC_GRADIENT = 4,
  NSVG_PAINT_GRADIENT_LINK = 5,
  NSVG_PAINT_PATTERN = 6,
};

enum NSVGspreadType {
  NSVG_SPREAD_PAD = 0,
  NSVG_SPREAD_REFLECT = 1,
  NSVG_SPREAD_REPEAT = 2
};

enum NSVGlineJoin {
  NSVG_JOIN_MITER = 0,
  NSVG_JOIN_ROUND = 1,
  NSVG_JOIN_BEVEL = 2
};

enum NSVGlineCap {
  NSVG_CAP_BUTT = 0,
  NSVG_CAP_ROUND = 1,
  NSVG_CAP_SQUARE = 2
};

enum NSVGfillRule {
  NSVG_FILLRULE_NONZERO = 0,
  NSVG_FILLRULE_EVENODD = 1
};
/*
enum NSVGflags {
  NSVG_FLAGS_VISIBLE = 0x01
};
*/
typedef struct NSVGgradientLink {
  char id[64];
  float xform[6];
} NSVGgradientLink;

typedef struct NSVGgradientStop {
  unsigned int color;
  float offset;
} NSVGgradientStop;

typedef struct NSVGgradient {  //undefined sizeof
  float xform[6];
//  float position[6];
  float fx, fy;
  int ditherCoarse;
  char spread;
  int nstops;
  NSVGgradientStop stops[1];
} NSVGgradient;

typedef struct NSVGpaint {
  union {
    unsigned int color;
    NSVGgradient* gradient;
    NSVGgradientLink* gradientLink;
  } paint;
  char type;
  char pad[7]; //alignment
} NSVGpaint;

typedef struct NSVGpath
{
  float* pts;          // Cubic bezier points: x0,y0, [cpx1,cpx1,cpx2,cpy2,x1,y1], ...
  int npts;          // Total number of bezier points.
  char closed;        // Flag indicating if shapes should be treated as closed.
  char pad[3];
  float bounds[4];      // Tight bounding box of the shape [minx,miny,maxx,maxy].
  struct NSVGpath* next;    // Pointer to next path, or NULL if last element.
} NSVGpath;

typedef unsigned char NSVGclipPathIndex;

typedef struct NSVGclip
{
  NSVGclipPathIndex* index;  // Array of clip path indices (of related NSVGimage).
  NSVGclipPathIndex count;  // Number of clip paths in this set.
  char pad[7];
} NSVGclip;

#define kMaxIDLength 64
#define kMaxTextLength 256

typedef struct NSVGshape NSVGshape;

typedef struct NSVGpattern {
  char id[kMaxIDLength];
  int nx, ny;  //repeat
  float width;
  float height;
  void* image;
  struct NSVGpattern* next;
} NSVGpattern;

typedef struct NSVGgroup
{
  char id[kMaxIDLength];
  struct NSVGgroup* parent;      // Pointer to parent group or NULL
  struct NSVGgroup* next;      // Pointer to next group or NULL
  struct NSVGshape* shapeList; // list of shapes inside the group
  int visibility;
} NSVGgroup;

typedef struct NSVGshape
{
  char id[kMaxIDLength];        // Optional 'id' attr of the shape
  char title[kMaxIDLength];        // Optional 'title' of the shape or its ancestor(s)
  NSVGpaint fill;        // Fill paint
  NSVGpaint stroke;      // Stroke paint
  float opacity;        // Opacity of the shape.
  float strokeWidth;      // Stroke width (scaled).
  float strokeDashOffset;    // Stroke dash offset (scaled).
  float strokeDashArray[8];      // Stroke dash array (scaled).
  char strokeDashCount;        // Number of dash values in dash array.
  char strokeLineJoin;    // Stroke join type.
  char strokeLineCap;      // Stroke cap type.
  char fillRule;        // Fill rule, see NSVGfillRule.
  unsigned char flags;    // Logical or of NSVG_FLAGS_* flags
  BOOLEAN isText;
  BOOLEAN debug;
  BOOLEAN isSymbol;
  float miterLimit;      // Miter limit
  float bounds[4];      // Tight bounding box of the shape [minx,miny,maxx,maxy].
  float xform[6];
  NSVGpath* paths;      // Linked list of paths in the image. One shape - one path.
  NSVGgroup* group;      // Pointer to parent group or NULL
  NSVGclip clip;
  struct NSVGshape* next;    // Pointer to next shape, or NULL if last element.
  struct NSVGshape* link;    // pointer for reference shape
  struct NSVGfont* fontFace; //one letter - one shape
  const char *image_href;
} NSVGshape;

typedef struct NSVGclipPath
{
  char id[64];        // Unique id of this clip path (from SVG).
  NSVGclipPathIndex index;  // Unique internal index of this clip path.
  NSVGshape* shapes;      // Linked list of shapes in this clip path.
  NSVGgroup* group;      // Pointer to parent group or NULL
  struct NSVGclipPath* next;  // Pointer to next clip path or NULL.
} NSVGclipPath;

typedef struct NSVGimage
{
  float width;        // Width of the image.
  float height;        // Height of the image.
  float realBounds[4];
  NSVGshape* shapes;      // Linked list of shapes in the image.
  NSVGgroup* groups;      // Linked list of all groups in the image
  NSVGpath* paths;        // Linked list of paths in the image.
  BOOLEAN isFont;
  NSVGclipPath* clipPaths;
} NSVGimage;

#define NSVG_MAX_ATTR 1024
#define NSVG_MAX_CLIP_PATHS 255 // also note NSVGclipPathIndex

enum NSVGgradientUnits {
  NSVG_USER_SPACE = 0,
  NSVG_OBJECT_SPACE = 1
};

#define NSVG_MAX_DASHES 8

enum NSVGunits {
  NSVG_UNITS_USER,
  NSVG_UNITS_PX,
  NSVG_UNITS_PT,
  NSVG_UNITS_PC,
  NSVG_UNITS_MM,
  NSVG_UNITS_CM,
  NSVG_UNITS_IN,
  NSVG_UNITS_PERCENT,
  NSVG_UNITS_EM,
  NSVG_UNITS_EX
};

enum NSVGvisibility {
  NSVG_VIS_DISPLAY = 1,
  NSVG_VIS_VISIBLE = 2,
};

typedef struct NSVGcoordinate {
  float value;
  int units;
} NSVGcoordinate;

typedef struct NSVGlinearData {
  NSVGcoordinate x1, y1, x2, y2;
} NSVGlinearData;

typedef struct NSVGradialData {
  NSVGcoordinate cx, cy, r, fx, fy;
} NSVGradialData;

typedef struct NSVGgradientData
{
  char id[kMaxIDLength];
  char ref[kMaxIDLength];
  char type;
  union {
    NSVGlinearData linear;
    NSVGradialData radial;
  } direction;
  char spread;
  char units;
  int ditherCoarse;
  float xform[6];
  int nstops;
  NSVGgradientStop* stops;
  struct NSVGgradientData* next;
} NSVGgradientData;

typedef struct NSVGattrib
{
  char id[kMaxIDLength];
  char title[kMaxIDLength];
  float xform[6];
  unsigned int fillColor;
  unsigned int strokeColor;
  float opacity;
  float fillOpacity;
  float strokeOpacity;
  char fillGradient[64];
  char strokeGradient[64];
  float strokeWidth;
  float strokeDashOffset;
  float strokeDashArray[NSVG_MAX_DASHES];
  int strokeDashCount;
  char strokeLineJoin;
  char strokeLineCap;
  float miterLimit;
  char fillRule;
  struct NSVGfont* fontFace;
//  float fontSize;
//  char fontFamily[64];
//  char fontWeight[64];
  unsigned int stopColor;
  float stopOpacity;
  float stopOffset;
  char hasFill;
  char hasStroke;
  char visible;
  NSVGclipPathIndex clipPathCount;
  NSVGgroup* group;
//  NSVGpattern* pattern;
} NSVGattrib;

 typedef struct NSVGstyles
 {
   char*  name;
   char* description;
   struct NSVGstyles* next;
 } NSVGstyles;

//------------- Fonts ---------------------
typedef struct NSVGglyph {
  char name[16];
  CHAR16 unicode;
  int horizAdvX;
  NSVGpath* path;
  struct NSVGglyph *next;
} NSVGglyph;

typedef struct NSVGfont {
  char id[kMaxIDLength];
  int horizAdvX;
  // --- font-face
  char fontFamily[kMaxIDLength];
  float fontWeight; //usually 400 like stroke-width
  float fontSize; // 8,9,12,14...
  float unitsPerEm; //usually 1000
//  char panose[64]; //int[10] obsolete
  int ascent;
  int descent;
  int xHeight;
  int capHeight;
  float bbox[4]; //maximum Bounding box for chars
  int underlineThickness;
  int underlinePosition;
  int slope;
  int unicodeRange[2];
  char fontStretch; //normal, ...
  char fontStyle; //light, italic, bold
//  int fillColor; -- font is colorless while text has color
  // -- glyphs
  NSVGglyph* missingGlyph;
  NSVGglyph* glyphs; // a chain
//  struct NSVGfont* next;
} NSVGfont;

typedef struct NSVGfontChain {
  NSVGfont *font;
  struct NSVGfontChain* next;
} NSVGfontChain;

typedef struct textFaces {
  NSVGfont *font;
  UINT32 color;
  INTN   size;
  BOOLEAN valid;
} textFaces;

extern textFaces textFace[]; //0-help 1-message 2-menu 3-test

typedef struct NSVGtext {
  char id[kMaxIDLength];
//  char class[64];
  float x,y;
  float xform[6];
  //  char fontFamily[64];
  NSVGfont* font;
  NSVGfont* fontFace;
  float fontSize;
  char fontStyle;
  unsigned int fontColor;
  unsigned int strokeColor;
  float fillOpacity;
  float strokeOpacity;
  float strokeWidth;
  NSVGstyles* style;
  NSVGshape* shapes;
  NSVGpath* paths;  //the paths for shapes. Shapes will have only handles
  struct NSVGtext *next;
  NSVGgroup* group;
} NSVGtext;

typedef struct NSVGsymbol {
  char id[kMaxIDLength];
//  float xform[6];
  float bounds[4];
  float viewBox[4];
  NSVGshape* shapes;
  NSVGshape* shapesTail;
  NSVGpath* paths;
  struct NSVGsymbol *next;
} NSVGsymbol;

typedef struct NSVGparser
{
  NSVGattrib attr[NSVG_MAX_ATTR];
  int attrHead;
  float* pts;
  int npts;
  int cpts;
  NSVGpath* plist;
  NSVGimage* image;
  NSVGstyles* styles;
  NSVGgradientData* gradients;
  NSVGshape* shapesTail;
  struct NSVGfont* font;
  float opacity;
  // this is temporary set for Menu text, later each text will have own face
  float fontSize;
  char fontStyle;
  unsigned int fontColor;
  //------------
  float viewMinx, viewMiny, viewWidth, viewHeight;
  int alignX, alignY, alignType;
  float dpi;
  char pathFlag;
  char defsFlag;
  char titleFlag;
  char shapeFlag;
  char styleFlag;
  char patternFlag;
  char symbolFlag;
  BOOLEAN isText;
  char unknown[64];
  NSVGtext* text;
  NSVGsymbol* symbols;
  NSVGpattern *patterns;
  NSVGclipPath* clipPath;
  NSVGclipPathIndex clipPathStack[NSVG_MAX_CLIP_PATHS];
} NSVGparser;


//---

// Duplicates a path.
//NSVGpath* nsvgDuplicatePath(NSVGpath* p);

// Parses SVG file from a file, returns SVG image as paths.
//NSVGimage* nsvgParseFromFile(const char* filename, const char* units, float dpi);

// Parses SVG file from a null terminated string, returns SVG image as paths.
// Important note: changes the string.
NSVGparser* nsvgParse(char* input, /* const char* units,*/ float dpi, float opacity);

// Deletes list of paths.
void nsvgDelete(NSVGimage* image);
void nsvg__xformIdentity(float* t);
void nsvg__deleteParser(NSVGparser* p);
void nsvg__xformInverse(float* inv, float* t);
void nsvg__xformSetScale(float* t, float sx, float sy);
void nsvg__xformPremultiply(float* t, float* s);
void nsvg__xformMultiply(float* t, float* s);
void nsvg__deleteFont(NSVGfont* font);
void nsvg__imageBounds(NSVGparser* p, float* bounds);
float addLetter(NSVGparser* p, CHAR16 letter, float x, float y, float scale, UINT32 color);
VOID RenderSVGfont(NSVGfont  *fontSVG, UINT32 color);

//--------------- Rasterizer --------------
typedef struct NSVGrasterizer NSVGrasterizer;
typedef void (*recursive_image)(const void *obj, NSVGrasterizer *r, const char *href, const float area[4]);


// Allocated rasterizer context.
NSVGrasterizer* nsvgCreateRasterizer(VOID);

// Rasterizes SVG image, returns RGBA image (non-premultiplied alpha)
//   r - pointer to rasterizer context
//   image - pointer to image to rasterize
//   tx,ty - image offset (applied after scaling)
//   scale - image scale
//   dst - pointer to destination image data, 4 bytes per pixel (RGBA)
//   w - width of the image to render
//   h - height of the image to render
//   stride - number of bytes per scaleline in the destination buffer
void nsvgRasterize(NSVGrasterizer* r,
                   NSVGimage* image, float tx, float ty, float scalex, float scaley,
                   unsigned char* dst, int w, int h, int stride);

// Deletes rasterizer context.
void nsvgDeleteRasterizer(NSVGrasterizer*);
NSVGparser* nsvg__createParser();

#define NSVG__SUBSAMPLES  5
#define NSVG__FIXSHIFT    14
#define NSVG__FIX      (1 << NSVG__FIXSHIFT)
#define NSVG__FIXMASK    (NSVG__FIX-1)
#define NSVG__MEMPAGE_SIZE  1024

typedef struct NSVGedge {
  float x0,y0, x1,y1;
  int dir;
  struct NSVGedge* next;
} NSVGedge;

typedef struct NSVGpoint {
  float x, y;
  float dx, dy;
  float len;
  float dmx, dmy;
  unsigned char flags;
  char pad[3];
} NSVGpoint;

typedef struct NSVGactiveEdge {
  int x,dx;
  float ey;
  int dir;
  struct NSVGactiveEdge *next;
} NSVGactiveEdge;

typedef struct NSVGmemPage {
  unsigned char mem[NSVG__MEMPAGE_SIZE];
  int size;
  struct NSVGmemPage* next;
} NSVGmemPage;

typedef struct NSVGcachedPaint {
  char type;
  char spread;
  char pad[2];
  int coarse;
  float xform[6];
//  float opacity;
  void *image;
  unsigned int colors[256];
//  unsigned int colors2[256];
} NSVGcachedPaint;

typedef void (*NSVGscanlineFunction)(
        unsigned char* dst, int count, unsigned char* cover, int x, int y,
    /*    float tx, float ty, float scalex, float scaley, */ NSVGcachedPaint* cache);

struct NSVGrasterizer
{
  float px, py;

  float tessTol;
  float distTol;

  NSVGedge* edges;
  int nedges;
  int cedges;

  NSVGpoint* points;
  int npoints;
  int cpoints;

  NSVGpoint* points2;
  int npoints2;
  int cpoints2;

  NSVGactiveEdge* freelist;
  NSVGmemPage* pages;
  NSVGmemPage* curpage;

  unsigned char* scanline;
  int cscanline;
  NSVGscanlineFunction fscanline;

  unsigned char* stencil;
  int stencilSize;
  int stencilStride;

  unsigned char* bitmap;
  int width, height, stride;
};

extern NSVGfontChain *fontsDB;
extern struct NSVGparser *mainParser;

#endif
