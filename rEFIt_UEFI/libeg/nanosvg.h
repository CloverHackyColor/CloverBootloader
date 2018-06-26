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
 * tesch1 - feature gruops
 * boris-ulyanov - style processing
 * jamislike - basic text parsing
 * darealshinji - multiple improvements
 *
*/
// Adoptation to Clover project by Slice, 2018

#ifndef NANOSVG_H
#define NANOSVG_H
/*
#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
*/
#include "Platform.h"

#define NANOSVG_ALL_COLOR_KEYWORDS 1


//There are defines for compilation as first step. Must be revised
#define memcpy(dest,source,count) CopyMem(dest,(void*)source,(UINTN)(count))
#define memset(dest,ch,count)     SetMem(dest,(UINTN)(count),(UINT8)(ch))
#define strcmp(a,b) AsciiStrCmp(a,b)
#define strncmp(a,b,n) AsciiStrnCmp(a,b,n)
#define strstr(a,b) AsciiStrStr(a,b)
//#define strncpy(a,b,n) AsciiStrnCpy(a,b,n)
#define strlen(s) AsciiStrLen(s)
#define strncpy(a,b,n) AsciiSPrint(a,n,"%a",b)


enum NSVGpaintType {
  NSVG_PAINT_NONE = 0,
  NSVG_PAINT_COLOR = 1,
  NSVG_PAINT_LINEAR_GRADIENT = 2,
  NSVG_PAINT_RADIAL_GRADIENT = 3
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

enum NSVGflags {
  NSVG_FLAGS_VISIBLE = 0x01
};

typedef struct NSVGgradientStop {
  unsigned int color;
  float offset;
} NSVGgradientStop;

typedef struct NSVGgradient {
  float xform[6];
  char spread;
  float fx, fy;
  int nstops;
  NSVGgradientStop stops[1];
} NSVGgradient;

typedef struct NSVGpaint {
  char type;
  union {
    unsigned int color;
    NSVGgradient* gradient;
  };
} NSVGpaint;

typedef struct NSVGpath
{
  float* pts;          // Cubic bezier points: x0,y0, [cpx1,cpx1,cpx2,cpy2,x1,y1], ...
  int npts;          // Total number of bezier points.
  char closed;        // Flag indicating if shapes should be treated as closed.
  float bounds[4];      // Tight bounding box of the shape [minx,miny,maxx,maxy].
  struct NSVGpath* next;    // Pointer to next path, or NULL if last element.
} NSVGpath;

#define kMaxIDLength 64
#define kMaxTextLength 256

typedef struct NSVGgroup
{
	char id[kMaxIDLength];
	struct NSVGgroup* parent;			// Pointer to parent group or NULL
	struct NSVGgroup* next;			// Pointer to next group or NULL
} NSVGgroup;

typedef struct NSVGshape
{
	char id[kMaxIDLength];				// Optional 'id' attr of the shape
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
  float miterLimit;      // Miter limit
  char fillRule;        // Fill rule, see NSVGfillRule.
  unsigned char flags;    // Logical or of NSVG_FLAGS_* flags
  float bounds[4];      // Tight bounding box of the shape [minx,miny,maxx,maxy].
  NSVGpath* paths;      // Linked list of paths in the image.
	NSVGgroup* group;			// Pointer to parent group or NULL
  struct NSVGshape* next;    // Pointer to next shape, or NULL if last element.
  char fontFamily[64];
  char fontWeight[64];
  float fontSize;
  BOOLEAN isText;
  char textData[kMaxTextLength];
  const char *image_href;
} NSVGshape;

typedef struct NSVGimage
{
  float width;        // Width of the image.
  float height;        // Height of the image.
  float realBounds[4];
  NSVGshape* shapes;      // Linked list of shapes in the image.
	NSVGgroup* groups;			// Linked list of all groups in the image
} NSVGimage;

#define NSVG_MAX_ATTR 128

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
  char id[64];
  char ref[64];
  char type;
  union {
    NSVGlinearData linear;
    NSVGradialData radial;
  };
  char spread;
  char units;
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
  float fontSize;
  char fontFamily[64];
  char fontWeight[64];
  unsigned int stopColor;
  float stopOpacity;
  float stopOffset;
  char hasFill;
  char hasStroke;
  char visible;
  NSVGgroup* group;
} NSVGattrib;

 typedef struct NSVGstyles
 {
 	char*	name;
 	char* description;
 	struct NSVGstyles* next;
 } NSVGstyles;
 

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
  float viewMinx, viewMiny, viewWidth, viewHeight;
  int alignX, alignY, alignType;
  float dpi;
  char pathFlag;
  char defsFlag;
  char titleFlag;
  char shapeFlag;
  char styleFlag;
  char groupFlag;
  BOOLEAN isText;
} NSVGparser;

// Duplicates a path.
NSVGpath* nsvgDuplicatePath(NSVGpath* p);

// Parses SVG file from a file, returns SVG image as paths.
//NSVGimage* nsvgParseFromFile(const char* filename, const char* units, float dpi);

// Parses SVG file from a null terminated string, returns SVG image as paths.
// Important note: changes the string.
extern NSVGimage* nsvgParse(char* input, const char* units, float dpi);

// Deletes list of paths.
extern void nsvgDelete(NSVGimage* image);

//--------------- Rasterizer --------------
typedef struct NSVGrasterizer NSVGrasterizer;
typedef void (*recursive_image)(const void *obj, NSVGrasterizer *r, const char *href, const float area[4]);


// Allocated rasterizer context.
extern NSVGrasterizer* nsvgCreateRasterizer(VOID);

// Rasterizes SVG image, returns RGBA image (non-premultiplied alpha)
//   r - pointer to rasterizer context
//   image - pointer to image to rasterize
//   tx,ty - image offset (applied after scaling)
//   scale - image scale
//   dst - pointer to destination image data, 4 bytes per pixel (RGBA)
//   w - width of the image to render
//   h - height of the image to render
//   stride - number of bytes per scaleline in the destination buffer
extern void nsvgRasterize(NSVGrasterizer* r,
                   NSVGimage* image, float tx, float ty, float scalex, float scaley,
                   unsigned char* dst, int w, int h, int stride, recursive_image external_image, const void *obj);

// Deletes rasterizer context.
extern void nsvgDeleteRasterizer(NSVGrasterizer*);

#define NSVG__SUBSAMPLES  5
#define NSVG__FIXSHIFT    10
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
  float xform[6];
  unsigned int colors[256];
} NSVGcachedPaint;

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

  unsigned char* bitmap;
  int width, height, stride;
};





#endif
