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
 */


// NanoSVG is a simple stupid single-header-file SVG parse. The output of the parser is a list of cubic bezier shapes.
//
// The library suits well for anything from rendering scalable icons in your editor application to prototyping a game.
//
// NanoSVG supports a wide range of SVG features, but something may be missing, feel free to create a pull request!
//


#include "nanosvg.h"
#include "FloatLib.h"
#include "../Platform/b64cdecode.h"
#include "XImage.h"
#include "../refit/lib.h"
#include "../libeg/XTheme.h"
//#include "../include/OneLinerMacros.h"
#include "../Platform/Utils.h"
#include "BmLib.h"
#include "../include/OneLinerMacros.h"

#ifndef DEBUG_ALL
#define DEBUG_SVG 1
#else
#define DEBUG_SVG DEBUG_ALL
#endif

#if DEBUG_SVG == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_SVG, __VA_ARGS__)
#endif

//typedef UINTN size_t;

#define NSVG_PI (3.14159265358979323846264338327f)
#define NSVG_PI_DEG (0.01745329251994f)
#define NSVG_KAPPA90 (0.5522847493f)  // Length proportional to radius of a cubic bezier handle for 90deg arcs.
#define pow(x,n) PowF(x,n)
#define sqrt(x) SqrtF(x)
#define sqrtf(x) SqrtF(x)
#define sinf(x) SinF(x)
#define cosf(x) CosF(x)
#define tanf(x) TanF(x)
#define ceilf(x) CeilF(x)
#define floorf(x) FloorF(x)
#define acosf(x) AcosF(x)
#define atan2f(y,x) Atan2F(y,x)
#define fabsf(x) FabsF(x)


//#define sscanf(s,f,x) AsciiStrToFloat(s, NULL, x)

#define fabs(x) ((x > 0.0)?x:(-x))
//#define fabsf(x) ((x > 0.0f)?x:(-x))

#define NSVG_ALIGN_MIN 0
#define NSVG_ALIGN_MID 1
#define NSVG_ALIGN_MAX 2
#define NSVG_ALIGN_NONE 0
#define NSVG_ALIGN_MEET 1
#define NSVG_ALIGN_SLICE 2

#define NSVG_NOTUSED(v) do { (void)(1 ? (void)0 : ( (void)(v) ) ); } while(0)
#define NSVG_RGB(r, g, b) (((unsigned int)b) | ((unsigned int)g << 8) | ((unsigned int)r << 16))

#ifdef _MSC_VER
#pragma warning (disable: 4996) // Switch off security warnings
#pragma warning (disable: 4100) // Switch off unreferenced formal parameter warnings
#ifdef __cplusplus
#define NSVG_INLINE inline
#else
#define NSVG_INLINE
#endif
#else
#define NSVG_INLINE inline
#endif

//TODO there are anime properties should be properties of FilmC
//which is not accessible here

//#define NANOSVG_MEMORY_ALLOCATION_TRACE

#ifdef NANOSVG_MEMORY_ALLOCATION_TRACE
#include "../cpp_foundation/XString.h"

int nvsg__memoryallocation_verbose = false;
uint64_t nsvg__alloc_count = 0;

XArray<uintptr_t> nsvg__allocatedPtr;
XArray<uint64_t> nsvg__allocatedPtrIdx;
#ifdef NANOSVG_MEMORY_ALLOCATION_TRACE_VERBOSE
XObjArray<XString8> nsvg__allocatedPtrMsg;
#endif

void nsvg__alloc_insert(void* p, const XString8& msg)
{
//if ( nsvg__alloc_count == 36602 ) {
//NOP;
//}
  nsvg__allocatedPtr.Add(uintptr_t(p));
  nsvg__allocatedPtrIdx.Add(nsvg__alloc_count++);
#ifdef NANOSVG_MEMORY_ALLOCATION_TRACE_VERBOSE
  nsvg__allocatedPtrMsg.AddCopy(msg, true);
#endif
}

void* nsvg__alloc(UINTN size, const XString8& msg)
{
  void* buffer = AllocatePool(size);
  if ( nvsg__memoryallocation_verbose ) DBG("nsvg__alloc(%lld) - %s = %llx\n", size, msg.c_str(), uintptr_t(buffer));
  nsvg__alloc_insert(buffer, msg);
  return buffer;
}

void* nsvg__alloczero(UINTN size, const XString8& msg)
{
  void* buffer = AllocateZeroPool(size);
  if ( nvsg__memoryallocation_verbose ) DBG("nsvg__alloczero(%lld) - %s = %llx\n", size, msg.c_str(), uintptr_t(buffer));
  nsvg__alloc_insert(buffer, msg);
  return buffer;
}

void* nsvg__alloccopy(UINTN size, const void* ref, const XString8& msg)
{
  void* buffer = AllocateCopyPool(size, ref);
  if ( nvsg__memoryallocation_verbose ) DBG("nsvg__alloccopy(%lld, %llx) - %s = %llx\n", size, uintptr_t(ref), msg.c_str(), uintptr_t(buffer));
  nsvg__alloc_insert(buffer, msg);
  return buffer;
}

void* nsvg__realloc(UINTN oldsize, UINTN newsize, void* ref, const XString8& msg)
{
  uintptr_t ref2 = uintptr_t(ref);
  auto idx = nsvg__allocatedPtr.indexOf(ref2);
  if ( idx == MAX_XSIZE ) log_technical_bug("nsvg__realloc");
  void* buffer = ReallocatePool(oldsize, newsize, ref);
  if ( nvsg__memoryallocation_verbose ) DBG("nsvg__realloc(%lld, %lld, %llx) - %s = %llx\n", oldsize, newsize, uintptr_t(ref), msg.c_str(), uintptr_t(buffer));
  nsvg__allocatedPtr.RemoveAtIndex(idx);
  nsvg__allocatedPtrIdx.RemoveAtIndex(idx);
#ifdef NANOSVG_MEMORY_ALLOCATION_TRACE_VERBOSE
  nsvg__allocatedPtrMsg.RemoveAtIndex(idx);
#endif
  nsvg__alloc_insert(buffer, msg);
  return buffer;
}

void nsvg__delete(void* buffer, const XString8& msg)
{
  uintptr_t ref2 = uintptr_t(buffer);
  auto idx = nsvg__allocatedPtr.indexOf(ref2);
  if ( idx == MAX_XSIZE ) {
    log_technical_bug("nsvg__delete %llx", uintptr_t(buffer));
  }
//  EFI_STATUS    Status = gBS->FreePool(buffer);
//  if ( EFI_ERROR(Status) ) {
//    log_technical_bug("nsvg__delete %llx", uintptr_t(buffer));
//  }
  EFI_STATUS    Status = 0;
  (void)Status;
  FreePool(buffer);
#ifdef NANOSVG_MEMORY_ALLOCATION_TRACE_VERBOSE
  if ( nvsg__memoryallocation_verbose ) DBG("nsvg__delete(%llx) - count=%lld - allocation msg %s - %s - Status = %s\n", uintptr_t(buffer), nsvg__allocatedPtrIdx[idx], nsvg__allocatedPtrMsg[idx].c_str(), msg.c_str(), efiStrError(Status));
#else
  if ( nvsg__memoryallocation_verbose ) DBG("nsvg__delete[%zu](%llx) - %s - Status = %s\n", idx, uintptr_t(buffer), msg.c_str(), efiStrError(Status));
#endif
  nsvg__allocatedPtr.RemoveAtIndex(idx);
  nsvg__allocatedPtrIdx.RemoveAtIndex(idx);
#ifdef NANOSVG_MEMORY_ALLOCATION_TRACE_VERBOSE
  nsvg__allocatedPtrMsg.RemoveAtIndex(idx);
#endif
}

size_t nsvg__nbDanglingPtr()
{
  return nsvg__allocatedPtr.length();
}


void nsvg__outputDanglingPtr()
{
  for(size_t idx=0;idx<nsvg__allocatedPtr.length();++idx){
#ifdef NANOSVG_MEMORY_ALLOCATION_TRACE_VERBOSE
    DBG("Dangling ptr %llx - count=%lld - allocation msg=%s\n", nsvg__allocatedPtr[idx], nsvg__allocatedPtrIdx[idx], nsvg__allocatedPtrMsg[idx].c_str());
#else
    DBG("Dangling ptr[%zu] %llx\n", idx, nsvg__allocatedPtr[idx]);
#endif
  }
}
#else
  // Macros defined in header.
#endif




//int nsvg__shapesBound(NSVGshape *shapes, float* bounds);
void nsvg__takeXformBounds(NSVGshape *shape, float *xform, float *bounds);
void nsvg__deleteShapes(NSVGshape* shape);

void nsvg__dumpFloat(CONST char* s, float* t, int N)
{
#if DEBUG_SVG
  int i;
  DBG("%s: ", s);
  for(i=0; i<N;i++)
  {
    float a = t[i];
    int b = (int)a;
    int sign = (a < 0.f);
    DBG("%c%d.%06d ", ((b == 0) && sign)?'-':' ', b, (int)(fabsf((a-(float)b)*1.0e6f)));
  }
  DBG("\n");
#else
  (void)s;
  (void)t;
  (void)N;
#endif
}

static int nsvg__getIntegerDict(const char* s)
{
  if ((s[1] == 'x') || (s[1] == 'X')) {
    return (int)AsciiStrHexToUintn (s);
  } else if (IS_DIGIT(s[0])) {
    return (int)AsciiStrDecimalToUintn(s);
  }
  return 0xFFFF;
}

float nsvg__sqr(float x) { return x*x; }
float nsvg__vmag(float x, float y) { return sqrtf(x*x + y*y); }

static float nsvg__vecrat(float ux, float uy, float vx, float vy)
{
  return (ux*vx + uy*vy) / (nsvg__vmag(ux,uy) * nsvg__vmag(vx,vy));
}

static float nsvg__vecang(float ux, float uy, float vx, float vy)
{
  float r = nsvg__vecrat(ux,uy, vx,vy);
  if (r < -1.0f) r = -1.0f;
  if (r > 1.0f) r = 1.0f;
  return ((ux*vy < uy*vx) ? -1.0f : 1.0f) * acosf(r);
}


static char *nsvg__strndup(const char *s, size_t n);

// Calculate number of characters.
static int nsvg__strchr(const char *s, char c)
{
  int n = 0;
  while (*s != 0) {
    if (*s++ == c) {
      n++;
    }
  }
  return n;
}

static int nsvg__isspace(char c)
{
  return nsvg__strchr(" \t\n\v\f\r", c) != 0;
}

static int nsvg__isdigit(char c)
{
  return c >= '0' && c <= '9';
}
/*
static int nsvg__isnum(char c)
{
  return nsvg__strchr("0123456789+-.eE", c) != 0;  //SIC!
}
*/
static NSVG_INLINE float nsvg__minf(float a, float b) { return a < b ? a : b; }
static NSVG_INLINE float nsvg__maxf(float a, float b) { return a > b ? a : b; }


// Simple XML parser

#define NSVG_XML_TAG 1
#define NSVG_XML_CONTENT 2
#define NSVG_XML_MAX_ATTRIBS 256

static void nsvg__parseContent(char* s,
                               void (*contentCb)(void* ud, char* s),
                               void* ud)
{
  // Trim start white spaces
  while (*s && nsvg__isspace(*s)) s++;
  if (!*s) return;

  if (contentCb)
    (*contentCb)(ud, s);
}

static void nsvg__parseElement(char* s,
                               void (*startelCb)(void* ud, const char* el, char** attr),
                               void (*endelCb)(void* ud, const char* el),
                               void* ud)
{
  char* attr[NSVG_XML_MAX_ATTRIBS];
  int nattr = 0;
  char* tagname;
  int start = 0;
  int end = 0;
  char quote;

  // Skip white space after the '<'
  while (*s && nsvg__isspace(*s)) s++;

  // Check if the tag is end tag
  if (*s == '/') {
    s++;
    end = 1;
  } else {
    start = 1;
  }

  // Skip comments, data and preprocessor stuff.
  if (!*s || *s == '?' || *s == '!')
    return;

  // Get tag name
  tagname = s;
  while (*s && !nsvg__isspace(*s)) s++;
  if (*s) { *s++ = '\0'; }

  // Get attribs
  while (!end && *s && nattr < NSVG_XML_MAX_ATTRIBS-3) {
    char* name = NULL;
    char* value = NULL;

    // Skip white space before the attrib name
    while (*s && nsvg__isspace(*s)) s++;
    if (!*s) break;
    if (*s == '/') {
      end = 1;
      break;
    }
    name = s;
    // Find end of the attrib name.
    while (*s && !nsvg__isspace(*s) && *s != '=') s++;
    if (*s) {
      *s++ = '\0';
      //      DBG("attrib name %s\n", name);
    }
    // Skip until the beginning of the value.
    while (*s && *s != '\"' && *s != '\'') s++;
    if (!*s) break;
    quote = *s;
    s++;
    // Store value and find the end of it.
    value = s;
    while (*s && *s != quote) s++;
    if (*s) {
      *s++ = '\0';
      //      DBG("value:%s\n", value);
    }

    // Store only well formed attributes
    if (name && value) {
      attr[nattr++] = name;  //class
      attr[nattr++] = value;  //Master_Slide
      //         DBG("attrib %d: name %s value %s\n", nattr, name, value);
    }
  }

  // List terminator
  attr[nattr++] = 0;
  attr[nattr++] = 0;

  // Call callbacks.
  if (start && startelCb) {
    (*startelCb)(ud, tagname, attr);  //nsvg__startElement
  }
  if (end && endelCb) {
    (*endelCb)(ud, tagname);  //nsvg__endElement
  }
  //  DBG("parseElement %s ended\n", tagname);
}

void nsvg__parseXML(char* input,
                    void (*startelCb)(void* ud, const char* el, char** attr),
                    void (*endelCb)(void* ud, const char* el),
                    void (*contentCb)(void* ud, char* s),
                    void* ud)
{
  char* s = input;
  char* mark = s;
  int state = NSVG_XML_CONTENT;
  while (*s) {
    if (*s == '<' && state == NSVG_XML_CONTENT) {
      // skip cdata
      if (strncmp(s, "<![CDATA[", 9) == 0) {
        s += 9;
        char* rv = strstr(s, "]]>");
        if (rv) s = rv + 3;
        continue;
      }
      // Start of a tag
      *s++ = '\0';
      nsvg__parseContent(mark, contentCb, ud);
//           DBG("tag content %s parsed\n", mark);
      mark = s;
      state = NSVG_XML_TAG;
    } else if (*s == '>' && state == NSVG_XML_TAG) {
      // Start of a content or new tag.
      *s++ = '\0';
      //    nsvg__parseContent(mark, contentCb, ud);
//          DBG("nsvg__parseElement %s\n", mark);
      nsvg__parseElement(mark, startelCb, endelCb, ud);
//          DBG(" element %s parsed\n", mark);
      mark = s;
      state = NSVG_XML_CONTENT;
    } else {
      s++;
    }
  }

  return;
}


/* Simple SVG parser. */

void nsvg__xformIdentity(float* t)
{
  t[0] = 1.0f; t[1] = 0.0f;
  t[2] = 0.0f; t[3] = 1.0f;
  t[4] = 0.0f; t[5] = 0.0f;
}

static void nsvg__xformSetTranslation(float* t, float tx, float ty)
{
  t[0] = 1.0f; t[1] = 0.0f;
  t[2] = 0.0f; t[3] = 1.0f;
  t[4] = tx; t[5] = ty;
}

void nsvg__xformSetScale(float* t, float sx, float sy)
{
  t[0] = sx; t[1] = 0.0f;
  t[2] = 0.0f; t[3] = sy;
  t[4] = 0.0f; t[5] = 0.0f;
  //DBG("nsvg__xformSetScale %f %f %f %f %f %f\n", t[0], t[1], t[2], t[3], t[4], t[5]);
}

static void nsvg__xformSetSkewX(float* t, float a)
{
  t[0] = 1.0f; t[1] = 0.0f;
  t[2] = tanf(a); t[3] = 1.0f;
  t[4] = 0.0f; t[5] = 0.0f;
}

static void nsvg__xformSetSkewY(float* t, float a)
{
  t[0] = 1.0f; t[1] = tanf(a);
  t[2] = 0.0f; t[3] = 1.0f;
  t[4] = 0.0f; t[5] = 0.0f;
}

static void nsvg__xformSetRotation(float* t, float a)
{
  float cs = cosf(a), sn = sinf(a);
  t[0] = cs; t[1] = sn;
  t[2] = -sn; t[3] = cs;
  t[4] = 0.0f; t[5] = 0.0f;
}

void nsvg__xformMultiply(float* t, float* s)
{
  float t0 = t[0] * s[0] + t[1] * s[2];
  float t2 = t[2] * s[0] + t[3] * s[2];
  float t4 = t[4] * s[0] + t[5] * s[2] + s[4];
  t[1] = t[0] * s[1] + t[1] * s[3];
  t[3] = t[2] * s[1] + t[3] * s[3];
  t[5] = t[4] * s[1] + t[5] * s[3] + s[5];
  t[0] = t0;
  t[2] = t2;
  t[4] = t4;
}

void nsvg__xformInverse(float* inv, float* t)
{
  double invdet, det = (double)t[0] * t[3] - (double)t[2] * t[1];
  if (det > -1e-6 && det < 1e-6) {
    nsvg__xformIdentity(inv);
    return;
  }
  invdet = 1.0 / det;
  inv[0] = (float)(t[3] * invdet);
  inv[2] = (float)(-t[2] * invdet);
  inv[4] = (float)(((double)t[2] * t[5] - (double)t[3] * t[4]) * invdet);
  inv[1] = (float)(-t[1] * invdet);
  inv[3] = (float)(t[0] * invdet);
  inv[5] = (float)(((double)t[1] * t[4] - (double)t[0] * t[5]) * invdet);
}

void nsvg__xformPremultiply(float* t, float* s)
{
  float s2[6];
  memcpy(s2, s, sizeof(float)*6);
  nsvg__xformMultiply(s2, t);
  memcpy(t, s2, sizeof(float)*6);
}

static void nsvg__xformPoint(float* dx, float* dy, float x, float y, float* t)
{
  *dx = x*t[0] + y*t[2] + t[4];
  *dy = x*t[1] + y*t[3] + t[5];
}

static void nsvg__xformVec(float* dx, float* dy, float x, float y, float* t)
{
  *dx = x*t[0] + y*t[2];
  *dy = x*t[1] + y*t[3];
}

#define NSVG_EPSILON (1e-12)

static int nsvg__ptInBounds(float* pt, float* bounds)
{
  return pt[0] >= bounds[0] && pt[0] <= bounds[2] && pt[1] >= bounds[1] && pt[1] <= bounds[3];
}


static double nsvg__evalBezier(double t, double p0, double p1, double p2, double p3)
{
  double it = 1.0-t;
  return it*it*it*p0 + 3.0*it*it*t*p1 + 3.0*it*t*t*p2 + t*t*t*p3;
}

static void nsvg__curveBounds(float* bounds, float* curve)
{
  double roots[2], a, b, c, b2ac, t, v;
  float* v0 = &curve[0];
  float* v1 = &curve[2];
  float* v2 = &curve[4];
  float* v3 = &curve[6];

  // Start the bounding box by end points
  bounds[0] = nsvg__minf(v0[0], v3[0]);
  bounds[1] = nsvg__minf(v0[1], v3[1]);
  bounds[2] = nsvg__maxf(v0[0], v3[0]);
  bounds[3] = nsvg__maxf(v0[1], v3[1]);

  // Bezier curve fits inside the convex hull of it's control points.
  // If control points are inside the bounds, we're done.
  if (nsvg__ptInBounds(v1, bounds) && nsvg__ptInBounds(v2, bounds))
    return;

  // Add bezier curve inflection points in X and Y.
  for (int i = 0; i < 2; i++) {
    a = -3.0f * v0[i] + 9.0f * v1[i] - 9.0f * v2[i] + 3.0f * v3[i];
    b = 6.0f * v0[i] - 12.0f * v1[i] + 6.0f * v2[i];
    c = 3.0f * v1[i] - 3.0f * v0[i];
    int count = 0;
    if (fabs(a) < NSVG_EPSILON) {
      if (fabs(b) > NSVG_EPSILON) {
        t = -c / b;
        if (t > NSVG_EPSILON && t < 1.0f-NSVG_EPSILON)
          roots[count++] = t;
      }
    } else {
      b2ac = b*b - 4.0f*c*a;
      if (b2ac > NSVG_EPSILON) {
        t = (-b + (double)sqrtf((float)b2ac)) / (2.0 * a);
        if (t > NSVG_EPSILON && t < 1.0-NSVG_EPSILON)          roots[count++] = t;
        t = (-b - (double)sqrtf((float)b2ac)) / (2.0 * a);
        if (t > NSVG_EPSILON && t < 1.0-NSVG_EPSILON)          roots[count++] = t;
      }
    }
    for (int  j = 0; j < count; j++) {
      v = nsvg__evalBezier(roots[j], v0[i], v1[i], v2[i], v3[i]);
      bounds[0+i] = nsvg__minf(bounds[0+i], (float)v);
      bounds[2+i] = nsvg__maxf(bounds[2+i], (float)v);
    }
  }
}

NSVGparser* nsvg__createParser()
{
  NSVGparser* p;
  p = (NSVGparser*)nsvg__alloczero(sizeof(NSVGparser), "nsvg__createParser"_XS8);
  if (p == NULL) return NULL;

  p->image = (NSVGimage*)nsvg__alloczero(sizeof(NSVGimage), "nsvg__createParser image"_XS8);
  if (p->image == NULL) {
    nsvg__delete(p, "nsvg__createParser"_XS8);
    return NULL;
  }
  // Init style
  nsvg__xformIdentity(p->attr[0].xform);
  memset(p->attr[0].id, 0, kMaxIDLength);
  p->attr[0].fillColor = NSVG_RGB(0,0,0);
  p->attr[0].strokeColor = NSVG_RGB(0,0,0);
  p->attr[0].opacity = 1.f;
  p->attr[0].fillOpacity = 1.f;
  p->attr[0].strokeOpacity = 1.f;
  p->attr[0].stopOpacity = 1.f;
  p->attr[0].strokeWidth = 1.f;
  p->attr[0].strokeLineJoin = NSVG_JOIN_MITER;
  p->attr[0].strokeLineCap = NSVG_CAP_BUTT;
  p->attr[0].miterLimit = 4;
  p->attr[0].fillRule = NSVG_FILLRULE_NONZERO;
  p->attr[0].hasFill = 1;
  p->attr[0].visible = NSVG_VIS_DISPLAY | NSVG_VIS_VISIBLE;
  p->isText = false;

  return p;
}

static void nsvg__deleteStyles(NSVGstyles* style)
{
  while (style) {
    NSVGstyles *next = style->next;
    if (style->name != NULL)
      nsvg__delete(style->name, "nsvg__deleteStyles"_XS8);
    if (style->description != NULL)
      nsvg__delete(style->description, "nsvg__deleteStyles"_XS8);
    nsvg__delete(style, "nsvg__deleteStyles"_XS8);
    style = next;
  }
}

static void nsvg__deletePaths(NSVGpath* path)
{
  while (path) {
    NSVGpath *next = path->next;
    if (path->pts != NULL) {
      nsvg__delete(path->pts, "nsvg__deletePaths"_XS8);
      path->pts = NULL;
    }
    nsvg__delete(path, "nsvg__deletePaths"_XS8);
    path = next;
  }
}

static void nsvg__deleteGlyphs(NSVGglyph* glyphs)
{
  while (glyphs) {
    NSVGglyph *next = glyphs->next;
    nsvg__deletePaths(glyphs->path);
    nsvg__delete(glyphs, "nsvg__deleteGlyphs"_XS8);
    glyphs = next;
  }
}

void nsvg__deleteFont(NSVGfont* font)
{
  if (!font) {
    return;
  }
  //DBG("nsvg__deleteFont %s %llx\n", font->id, uintptr_t(font));
  nsvg__deleteGlyphs(font->glyphs);
  nsvg__deleteGlyphs(font->missingGlyph);
  nsvg__delete(font, "nsvg__deleteFont"_XS8);
}

void nsvg__deleteFontChain(NSVGfontChain *fontChain)
{
  while (fontChain) {
    NSVGfont* font = fontChain->font;
    NSVGfontChain *nextChain = fontChain->next;
    nsvg__deleteFont(font);
    nsvg__delete(fontChain, "nsvg__deleteParser1"_XS8);
    fontChain = nextChain;
  }
}

static void nsvg__deletePaint(NSVGpaint* paint)
{
  if (!paint || !paint->paint.gradient) {
    return;
  }
  if (paint->type == NSVG_PAINT_LINEAR_GRADIENT ||
      paint->type == NSVG_PAINT_RADIAL_GRADIENT ||
      paint->type == NSVG_PAINT_CONIC_GRADIENT) {
    nsvg__delete(paint->paint.gradient, "nsvg__deletePaint"_XS8);
    paint->paint.gradient = NULL;
  }
}

static void nsvg__deleteGradientData(NSVGgradientData* grad)
{
  NSVGgradientData* next;
  while (grad != NULL) {
    next = grad->next;
    if (grad->nstops > 0) {
      nsvg__delete(grad->stops, "nsvg__deleteGradientData"_XS8);
    }
    nsvg__delete(grad, "nsvg__deleteGradientData"_XS8);
    grad = next;
  }
}


static void nsvg__deleteSymbols(NSVGsymbol* symbol)
{
  NSVGsymbol* next;
  while (symbol) {
    next = symbol->next;
    NSVGshape* shape = symbol->shapes;
    nsvg__deleteShapes(shape);
    nsvg__delete(symbol, "nsvg__deleteSymbols"_XS8);
    symbol = next;
  }
}

static void nsvg__popAttr(NSVGparser* p);
static NSVGattrib* nsvg__getAttr(NSVGparser* p);

void nsvg__deleteParser(NSVGparser* p)
{
  if (p != NULL) {
    nsvg__deleteStyles(p->styles);
    nsvg__deleteSymbols(p->symbols);
    nsvg__deletePaths(p->pathList);
    nsvg__deleteGradientData(p->gradients);
    nsvg__deleteFontChain(p->fontsDB);
    nsvg__deleteImage(p->image);
    if (p->cpts > 0 && p->pts) {
      nsvg__delete(p->pts, "nsvg__deleteParser2"_XS8);
    }

    auto text = p->text;
    while ( text ) {
      nsvg__delete(text, "nsvg__deleteParser3"_XS8);
      text = text->next;
    }

    while ( p->attrHead > 0 ) {
       NSVGattrib* attr = nsvg__getAttr(p);
      if (attr && attr->fontFace) {
        nsvg__delete(attr->fontFace, "nsvg__deleteParser3"_XS8);
        attr->fontFace = NULL;
      }
      if ( attr->group ) {
        nsvg__delete(attr->group, "nsvg__deleteParser4"_XS8);
      }
      nsvg__popAttr(p);
    }

    nsvg__delete(p, "nsvg__deleteParser5"_XS8);
  }
}

static void nsvg__resetPath(NSVGparser* p)
{
  p->npts = 0;
  if (p->cpts > 0 && p->pts) {
    nsvg__delete(p->pts, "nsvg__resetPath"_XS8);
    p->pts = NULL;
    p->cpts = 0;
  }
}

static void nsvg__addPoint(NSVGparser* p, float x, float y)
{
  //  DBG("enter addPoint\n");
  if (p->npts*2+7 > p->cpts) {
    //    DBG("npts=%d, cpts=%d\n", p->npts, p->cpts);
    if ((p->cpts == 0) || !p->pts) {
      p->cpts = 8;
      p->pts = (float*)nsvg__alloc(16 * sizeof(float), "nsvg__addPoint"_XS8);
    } else {
      //      DBG("reallocate\n");
      p->cpts *= 2;
      p->pts = (float*)nsvg__realloc(p->cpts*sizeof(float), p->cpts*2*sizeof(float), p->pts, "nsvg__addPoint"_XS8);

    }
    if (!p->pts) return;
  }
  //  DBG("new point\n");
  p->pts[p->npts*2+0] = x;
  p->pts[p->npts*2+1] = y;
  p->npts++;
}

static void nsvg__moveTo(NSVGparser* p, float x, float y)
{
  if (p->npts > 0) {
    p->pts[(p->npts-1)*2+0] = x;
    p->pts[(p->npts-1)*2+1] = y;
  } else {
    nsvg__addPoint(p, x, y);
  }
}

static void nsvg__lineTo(NSVGparser* p, float x, float y)
{
  float px,py, dx,dy;
  if (p->npts > 0) {
    //    DBG("npts=%d, and cpts=%d\n", p->npts, p->cpts);
    px = p->pts[(p->npts-1)*2+0];
    py = p->pts[(p->npts-1)*2+1];
    dx = x - px;
    dy = y - py;

    nsvg__addPoint(p, px + dx/3.0f, py + dy/3.0f);
    nsvg__addPoint(p, x - dx/3.0f, y - dy/3.0f);
    nsvg__addPoint(p, x, y);
  }
}

static void nsvg__cubicBezTo(NSVGparser* p, float cpx1, float cpy1, float cpx2, float cpy2, float x, float y)
{
  if (p->npts > 0) {
    nsvg__addPoint(p, cpx1, cpy1);
    nsvg__addPoint(p, cpx2, cpy2);
    nsvg__addPoint(p, x, y);
  }
}

static NSVGattrib* nsvg__getAttr(NSVGparser* p)
{
  return &p->attr[p->attrHead];
}

static void nsvg__pushAttr(NSVGparser* p)
{
  if (p->attrHead < NSVG_MAX_ATTR-1) {
    p->attrHead++;
    memcpy(&p->attr[p->attrHead], &p->attr[p->attrHead-1], sizeof(NSVGattrib));
    memset(&p->attr[p->attrHead].id, 0, sizeof(p->attr[p->attrHead].id));
    //    p->attr[p->attrHead].opacity = 1.0f; //let it be copy
  }
}

static void nsvg__popAttr(NSVGparser* p)
{
  if (p->attrHead > 0) {
    auto attr = nsvg__getAttr(p);
    if ( attr->fontFace ) {
      nsvg__delete(attr->fontFace, "nsvg__popAttr"_XS8);
    }
    p->attrHead--;
  }
}

static float nsvg__actualOrigX(NSVGparser* p)
{
  return p->viewMinx;
}

static float nsvg__actualOrigY(NSVGparser* p)
{
  return p->viewMiny;
}

static float nsvg__actualWidth(NSVGparser* p)
{
  return p->viewWidth;
}

static float nsvg__actualHeight(NSVGparser* p)
{
  return p->viewHeight;
}

static float nsvg__actualLength(NSVGparser* p)
{
  float w = nsvg__actualWidth(p), h = nsvg__actualHeight(p);
  return nsvg__vmag(w, h) * 0.70710678118655f; // 1.0/sqrtf(2.0f);
}

static float nsvg__convertToPixels(NSVGparser* p, NSVGcoordinate* c, float orig, float length)
{
  NSVGattrib* attr = nsvg__getAttr(p);
  float fontSize = 10;
  if ((p->dpi <= 0) || (p->dpi > 2400)) {
//    DBG("wrong dpi=%d\n", (int)p->dpi);
    p->dpi = 72;
  }
  if (attr->fontFace) {
    fontSize = attr->fontFace->fontSize;
  }
  switch (c->units) {
    case NSVG_UNITS_USER:    return c->value;
    case NSVG_UNITS_PX:      return c->value;
    case NSVG_UNITS_PT:      return c->value / 72.0f * p->dpi;
    case NSVG_UNITS_PC:      return c->value / 6.0f * p->dpi;
    case NSVG_UNITS_MM:      return c->value / 25.4f * p->dpi;
    case NSVG_UNITS_CM:      return c->value / 2.54f * p->dpi;
    case NSVG_UNITS_IN:      return c->value * p->dpi;
    case NSVG_UNITS_EM:      return c->value * fontSize;
    case NSVG_UNITS_EX:      return c->value * fontSize * 0.52f; // x-height of Helvetica.
    case NSVG_UNITS_PERCENT:  return orig + c->value * 0.01f * length;
    default:          return c->value;
  }
  //  return c.value;
}

static float nsvg__convertToPixelsForGradient(NSVGparser* p, char units, NSVGcoordinate* c, float orig, float length)
{
  //  float temp = 0.0;
  //units can be NSVG_USER_SPACE or NSVG_OBJECT_SPACE
  if (units == NSVG_USER_SPACE || c->units == NSVG_UNITS_PERCENT)
    return nsvg__convertToPixels(p, c, orig, length);

  return orig + c->value * length; //orig=x1 length=x2-x1 c->value= 86%
}

static NSVGgradientData* nsvg__findGradientData(NSVGparser* p, const char* id)
{
  NSVGgradientData* grad = p->gradients;
  if (id == NULL || *id == '\0')
    return NULL;

  while (grad) {
    if (strcmp(grad->id, id) == 0)
      return grad;
    grad = grad->next;
  }
  return NULL;
}

static NSVGgradientLink* nsvg__createGradientLink(const char* id)
{
  NSVGgradientLink* grad = (NSVGgradientLink*)nsvg__alloczero(sizeof(NSVGgradientLink), "nsvg__createGradientLink"_XS8);
  if (grad == NULL) return NULL;
  strncpy(grad->id, id, 63);
  grad->id[63] = '\0';
  return grad;
}

static void nsvg__getLocalBounds(float* bounds, NSVGshape *shape); //, float* xform);

static NSVGgradient* nsvg__createGradient(NSVGparser* p, NSVGshape* shape, NSVGgradientLink* link, char* paintType)
{
  //  NSVGattrib* attr = nsvg__getAttr(p);
  NSVGgradientData* data = NULL;
  NSVGgradientData* ref = NULL;
  NSVGgradientStop* stops = NULL;
  NSVGgradient* grad;
  float ox, oy, sw, sh, sl;
  int nstops = 0;
  int refIter = 0;
  if (!link) {
    *paintType = NSVG_PAINT_NONE;
    return NULL;
  }
  data = nsvg__findGradientData(p, link->id);
  if (data == NULL) return NULL;
  //  nsvg__dumpFloat("gradient data xform:", data->xform, 6);
  stops = data->stops;
  nstops = data->nstops;
  ref = nsvg__findGradientData(p, data->ref);
  while (ref != NULL) {
    NSVGgradientData* nextRef = NULL;
    if (stops == NULL && ref->stops != NULL) { //take stops only once at first occuerence
      stops = ref->stops;
      nstops = ref->nstops;
    }
    //left referenced, right is current,
    // matrix are reversed
    nsvg__xformPremultiply(data->xform, ref->xform);
    nextRef = nsvg__findGradientData(p, ref->ref); //recursive refs?
    if (nextRef == ref) break; // prevent infinite loops on malformed data
    ref = nextRef;
    refIter++;
    if (refIter > 32) break; // prevent infinite loops on malformed data
  }
  if (stops == NULL) return NULL;
  //  nsvg__dumpFloat("gradient final xform:", data->xform, 6);
  grad = (NSVGgradient*)nsvg__alloczero(sizeof(NSVGgradient) + sizeof(NSVGgradientStop)*(nstops-1), "nsvg__createGradient"_XS8);
  if (grad == NULL) return NULL;
  // The shape width and height.
  if (data->units == NSVG_OBJECT_SPACE) {
    float localBounds[4];
    nsvg__getLocalBounds(localBounds, shape); //, inv); //before any transform

    ox = localBounds[0];
    oy = localBounds[1];
    sw = localBounds[2] - localBounds[0];
    sh = localBounds[3] - localBounds[1];
  } else {
    ox = nsvg__actualOrigX(p);
    oy = nsvg__actualOrigY(p);
    sw = nsvg__actualWidth(p);
    sh = nsvg__actualHeight(p);
  }

  float gradForm[6]; //coordinates
  nsvg__xformIdentity(gradForm);
  //  sl = sqrtf(sw*sw + sh*sh) * 0.70710678118655f; // == 1. / sqrtf(2.0f);
  sl = nsvg__vmag(sw, sh) * 0.70710678118655f;
  if (data->type == NSVG_PAINT_LINEAR_GRADIENT) {
    float x1, y1, x2, y2, dx, dy;

    x1 = nsvg__convertToPixelsForGradient(p, data->units, &data->direction.linear.x1, ox, sw);
    y1 = nsvg__convertToPixelsForGradient(p, data->units, &data->direction.linear.y1, oy, sh);
    x2 = nsvg__convertToPixelsForGradient(p, data->units, &data->direction.linear.x2, ox, sw);
    y2 = nsvg__convertToPixelsForGradient(p, data->units, &data->direction.linear.y2, oy, sh);
    // Calculate transform aligned to the line
    dx = x2 - x1;
    dy = y2 - y1;

    gradForm[0] = dy; gradForm[1] = -dx;
    gradForm[2] = dx; gradForm[3] = dy;
    gradForm[4] = x1; gradForm[5] = y1;
  } else if ((data->type == NSVG_PAINT_RADIAL_GRADIENT) ||
             (data->type == NSVG_PAINT_CONIC_GRADIENT)) {
    float cx, cy, fx, fy, r;
    cx = nsvg__convertToPixelsForGradient(p, data->units, &data->direction.radial.cx, ox, sw);
    cy = nsvg__convertToPixelsForGradient(p, data->units, &data->direction.radial.cy, oy, sh);
    fx = nsvg__convertToPixelsForGradient(p, data->units, &data->direction.radial.fx, ox, sw);
    fy = nsvg__convertToPixelsForGradient(p, data->units, &data->direction.radial.fy, oy, sh);
    r  = nsvg__convertToPixelsForGradient(p, data->units, &data->direction.radial.r, 0, sl);
    // Calculate transform aligned to the circle
    gradForm[0] = r; gradForm[1] = 0;
    gradForm[2] = 0;  gradForm[3] = r;
    gradForm[4] = cx; gradForm[5] = cy;
    grad->fx = (fx - cx) / r;
    grad->fy = (fy - cy) / r;
  }

  //  nsvg__xformInverse(grad->xform, gradForm);
  //  nsvg__xformMultiply(grad->xform, data->xform); //from GradientData "gradientTransform"
  nsvg__xformMultiply(gradForm, data->xform);
  nsvg__xformInverse(grad->xform, gradForm);

  grad->spread = data->spread;
  grad->ditherCoarse = data->ditherCoarse;
  memcpy(grad->stops, stops, nstops*sizeof(NSVGgradientStop));
  grad->nstops = nstops;

  *paintType = data->type;

  return grad;
}

static float nsvg__getAverageScale(float* t)
{
  return (nsvg__vmag(t[0], t[2]) + nsvg__vmag(t[1], t[3])) * 0.5f;
}

static void nsvg__getLocalBounds(float* bounds, NSVGshape *shape) //, float* atXform)
{
  NSVGpath* path;
  float curve[8];
  float curveBounds[4];

  int i, first = 1;

  for (path = shape->paths; path != NULL; path = path->next) {
    curve[0] = path->pts[0];
    curve[1] = path->pts[1];
    for (i = 1; i < path->npts; i += 3) {
   //   curve = &path->pts[i*2];
      memcpy(&curve[2], &path->pts[i*2], 6*sizeof(float));
      nsvg__curveBounds(curveBounds, curve);
      if (first) {
        bounds[0] = curveBounds[0];
        bounds[1] = curveBounds[1];
        bounds[2] = curveBounds[2];
        bounds[3] = curveBounds[3];
        first = 0;
      } else {
        bounds[0] = nsvg__minf(bounds[0], curveBounds[0]);
        bounds[1] = nsvg__minf(bounds[1], curveBounds[1]);
        bounds[2] = nsvg__maxf(bounds[2], curveBounds[2]);
        bounds[3] = nsvg__maxf(bounds[3], curveBounds[3]);
      }
      curve[0] = curve[6];
      curve[1] = curve[7];
    }
  }
}
/*
static void  nsvg__getSymbolBounds(NSVGparser* p)
{
  NSVGsymbol* symbol = p->symbols;
  NSVGshape* shape = symbol->shapes;
  if (!shape) return;
  symbol->bounds[0] = FLT_MAX;
  symbol->bounds[1] = FLT_MAX;
  symbol->bounds[2] = -FLT_MAX;
  symbol->bounds[3] = -FLT_MAX;
  nsvg__shapesBound(shape, symbol->bounds);
//  nsvg__dumpFloat("Symbol has bounds", symbol->bounds, 4); //nothing
  nsvg__dumpFloat("Symbol has viewbox", symbol->viewBox, 4);
}
*/
static void nsvg__addShape(NSVGparser* p)
{
  NSVGattrib* attr = nsvg__getAttr(p);
  float scale;
  NSVGshape* shape;
//  int i;

  if (p->pathList == NULL /*&& !p->isText*/ )
    return;

  shape = (NSVGshape*)nsvg__alloczero(sizeof(NSVGshape), S8Printf("nsvg__addShape %s", attr->id));
  if (shape == NULL) return;

  memcpy(shape->id, attr->id, sizeof shape->id);
  memcpy(shape->title, attr->title, sizeof shape->title);
//    DBG("parse shapeID=%s\n", shape->id);
  shape->group = attr->group;
  scale = nsvg__getAverageScale(attr->xform);  //ssss
  shape->strokeWidth = attr->strokeWidth * scale;
  shape->strokeDashOffset = attr->strokeDashOffset * scale;
  shape->strokeDashCount = (char)attr->strokeDashCount;
  for (int i = 0; i < attr->strokeDashCount; i++)
    shape->strokeDashArray[i] = attr->strokeDashArray[i] * scale;
  shape->strokeLineJoin = attr->strokeLineJoin;
  shape->strokeLineCap = attr->strokeLineCap;
  shape->miterLimit = attr->miterLimit;
  shape->fillRule = attr->fillRule;
  shape->opacity = attr->opacity;
  memcpy(shape->xform, attr->xform, sizeof(float)*6);

  shape->paths = p->pathList;
  p->pathList = NULL;

  shape->clip.count = attr->clipPathCount;
//  if (shape->clip.count > 0) {
//    shape->clip.index = (NSVGclipPathIndex*)nsvg__alloccopy(attr->clipPathCount * sizeof(NSVGclipPathIndex),
//                                                             p->clipPathStack);
//    if (shape->clip.index == NULL) {
//      nsvg__delete(shape);
//      return;
//    }
//  }
  for (int i=0; i<shape->clip.count; i++) {
    shape->clip.index[i] = p->clipPathStack[i];
  }

  nsvg__getLocalBounds(shape->bounds, shape);  //(dest, src)

  // Set fill
  shape->fill.type = NSVG_PAINT_NONE;
  if (attr->hasFill == 1) {
    shape->fill.type = NSVG_PAINT_COLOR;
    shape->fill.paint.color = ((unsigned int)(attr->fillOpacity*255) << 24) | attr->fillColor;
  } else if (attr->hasFill == 2) {
    shape->fill.type = NSVG_PAINT_GRADIENT_LINK;
    shape->fill.paint.gradientLink = nsvg__createGradientLink(attr->fillGradient);
    if (shape->fill.paint.gradientLink == NULL) {
      shape->fill.type = NSVG_PAINT_NONE;
//      if (shape->clip.index) {
//        nsvg__delete(shape->clip.index);
//      }
      nsvg__delete(shape, "nsvg__addShape"_XS8);
      return;
    }
  } else if (attr->hasFill == 3) {
    shape->fill.type = NSVG_PAINT_PATTERN;
    const char *id = attr->fillGradient;
    NSVGpattern* pt = p->patterns;
    while (pt) {
      if (strcmp(pt->id, id) == 0) {
        break;
      }
      pt = pt->next;
    }
    shape->fill.paint.gradient = (NSVGgradient*)pt;
  }

  // Set stroke
  shape->stroke.type = NSVG_PAINT_NONE;
  if (attr->hasStroke == 1) {
    shape->stroke.type = NSVG_PAINT_COLOR;
    shape->stroke.paint.color = ((unsigned int)(attr->strokeOpacity*255) << 24) | attr->strokeColor;
  } else if (attr->hasStroke == 2) {
    shape->stroke.type = NSVG_PAINT_GRADIENT_LINK;
    shape->stroke.paint.gradientLink = nsvg__createGradientLink(attr->strokeGradient);
    if (shape->stroke.paint.gradientLink == NULL) {
      shape->fill.type = NSVG_PAINT_NONE;
//      if (shape->clip.index) {
//        nsvg__delete(shape->clip.index);
//      }
      nsvg__delete(shape, "nsvg__addShape"_XS8);
      return;
    }
  } else if (attr->hasStroke == 3) {
    shape->stroke.type = NSVG_PAINT_PATTERN;
    const char *id = attr->strokeGradient;
    NSVGpattern* pt = p->patterns;
    while (pt) {
      if (strcmp(pt->id, id) == 0) {
        break;
      }
      pt = pt->next;
    }
    shape->stroke.paint.gradient = (NSVGgradient*)pt;
  }

  // Set flags
  //  shape->flags = ((attr->visible & NSVG_VIS_DISPLAY) && (attr->visible & NSVG_VIS_VISIBLE) ? NSVG_VIS_VISIBLE : 0x00);
  shape->flags = attr->visible;
  if (p->defsFlag) {
    shape->flags = 0;
  }

  // Add shape
  if (p->clipPath != NULL) {
    shape->next = p->clipPath->shapes;
    p->clipPath->shapes = shape;
  } else
  if (p->symbolFlag) {
    if (p->symbols->shapes == NULL)
      p->symbols->shapes = shape;
    else
      p->symbols->shapesTail->next = shape;
    p->symbols->shapesTail = shape;
  } else {
    // Add to tail
    if (p->image->shapes == NULL)
      p->image->shapes = shape;
    else
      p->shapesTail->next = shape;
    p->shapesTail = shape;
  }
  return;
}

static void nsvg__addPath(NSVGparser* p, char closed, const char* fromWhere)
{
  //  NSVGattrib* attr = nsvg__getAttr(p);
  NSVGpath* path = NULL;
  float bounds[4];
  float* curve;
//  int i;

  if (p->npts < 4)
    return;

  if (closed)
    nsvg__lineTo(p, p->pts[0], p->pts[1]);
  // Expect 1 + N*3 points (N = number of cubic bezier segments).
  if ((p->npts % 3) != 1)
    return;

  path = (NSVGpath*)nsvg__alloczero(sizeof(NSVGpath), S8Printf("nsvg__addPath from %s", fromWhere));
  if (path == NULL) {
    return;
  }
  path->pts = (float*)nsvg__alloczero(p->npts*2*sizeof(float), S8Printf("nsvg__addPath2 from %s", fromWhere));
  if (path->pts == NULL) {
    nsvg__delete(path, "nsvg__addPath3"_XS8);
    return;
  }
  path->closed = closed;
  path->npts = p->npts;

  memcpy(path->pts, p->pts, p->npts * 2 * sizeof(float));

  // Find bounds
  for (int i = 0; i < path->npts-1; i += 3) {
    curve = &path->pts[i*2];
    nsvg__curveBounds(bounds, curve);
    if (i == 0) {
      path->bounds[0] = bounds[0];
      path->bounds[1] = bounds[1];
      path->bounds[2] = bounds[2];
      path->bounds[3] = bounds[3];
    } else {
      path->bounds[0] = nsvg__minf(path->bounds[0], bounds[0]);
      path->bounds[1] = nsvg__minf(path->bounds[1], bounds[1]);
      path->bounds[2] = nsvg__maxf(path->bounds[2], bounds[2]);
      path->bounds[3] = nsvg__maxf(path->bounds[3], bounds[3]);
    }
  }
  path->next = p->pathList;
  p->pathList = path;
  return;
}

//Slice - replace by own implementation
#ifdef USE_ATOF
// We roll our own string to float because the std library one uses locale and messes things up.
static double nsvg__atof(const char* s)
{
  char* cur = (char*)s;
  char* end = NULL;
  double res = 0.0, sign = 1.0;
  long long intPart = 0, fracPart = 0;
  char hasIntPart = 0, hasFracPart = 0;

  // Parse optional sign
  if (*cur == '+') {
    cur++;
  } else if (*cur == '-') {
    sign = -1;
    cur++;
  }

  // Parse integer part
  if (nsvg__isdigit(*cur)) {
    // Parse digit sequence
    //  intPart = (double)strtoll(cur, &end, 10);
    AsciiStrDecimalToUintnS(cur, &end, &intPart);
    if (cur != end) {
      res = (double)intPart;
      hasIntPart = 1;
      cur = end;
    }
  }

  // Parse fractional part.
  if (*cur == '.') {
    cur++; // Skip '.'
    if (nsvg__isdigit(*cur)) {
      // Parse digit sequence
      //    fracPart = strtoll(cur, &end, 10);
      AsciiStrDecimalToUintnS(cur, &end, &fracPart);
      if (cur != end) {
        res += (double)fracPart / pow(10.0, (double)(end - cur));
        hasFracPart = 1;
        cur = end;
      }
    }
  }

  // A valid number should have integer or fractional part.
  if (!hasIntPart && !hasFracPart)
    return 0.0;

  // Parse optional exponent
  if (*cur == 'e' || *cur == 'E') {
    long expPart = 0;
    cur++; // skip 'E'
    expPart = strtol(cur, &end, 10); // Parse digit sequence with sign
    if (cur != end) {
      res *= pow(10.0, (double)expPart);
    }
  }

  return res * sign;
}
#else
static float nsvg__atof(const char* s)
{
  float Data = 0.0f;
  AsciiStrToFloat(s, NULL, &Data);
  return Data;
}
#endif

static const char* nsvg__parseNumber(const char* s, char* it, const int size)
{
  const int last = size-1;
  int i = 0;

  // sign
  if (*s == '-' || *s == '+') {
    if (i < last) it[i++] = *s;
    s++;
  }
  // integer part
  while (*s && nsvg__isdigit(*s)) {
    if (i < last) it[i++] = *s;
    s++;
  }
  if (*s == '.') {
    // decimal point
    if (i < last) it[i++] = *s;
    s++;
    // fraction part
    while (*s && nsvg__isdigit(*s)) {
      if (i < last) it[i++] = *s;
      s++;
    }
  }
  // exponent but not units ex|em
  if ((*s == 'e' || *s == 'E') && (s[1] != 'm' && s[1] != 'x')) {
    if (i < last) it[i++] = *s;
    s++;
    if (*s == '-' || *s == '+') {
      if (i < last) it[i++] = *s;
      s++;
    }
    while (*s && nsvg__isdigit(*s)) {
      if (i < last) it[i++] = *s;
      s++;
    }
  }
  it[i] = '\0';

  return s;
}

static const char* nsvg__getNextPathItemWhenArcFlag(const char* s, char* it)
{
  it[0] = '\0';
  while (*s && (nsvg__isspace(*s) || *s == ',')) s++;
  if (!*s) return s;
  if (*s == '0' || *s == '1') {
    it[0] = *s++;
    it[1] = '\0';
    return s;
  }
  return s;
}


static const char* nsvg__getNextPathItem(const char* s, char* it)
{
  it[0] = '\0';
  // Skip white spaces and commas
  while (*s && (nsvg__isspace(*s) || *s == ',')) s++;
  if (!*s) return s;
  if (*s == '-' || *s == '+' || *s == '.' || nsvg__isdigit(*s)) {
    s = nsvg__parseNumber(s, it, 64);
  } else {
    // Parse command
    it[0] = *s++;
    it[1] = '\0';
    return s;
  }

  return s;
}

//w3.org
/*
 <circle cx="200" cy="135" r="20" fill="#3b3"/>  //Three digit hex — #rgb
 <circle cx="240" cy="135" r="20" fill="#33bb33"/> //Six digit hex — #rrggbb
 <circle cx="200" cy="175" r="20" fill="rgb(51,187,51)"/> //Integer functional — rgb(rrr, ggg, bbb)
 <circle cx="240" cy="175" r="20" fill="rgb(20%,73.333%,20%)"/> //Float functional — rgb(R%, G%, B%)
 */
static unsigned int nsvg__parseColorHex(const char* str)
{
  unsigned int r = 0, g = 0, b = 0;
  UINTN  c = 0;
  int n = 0;
  str++; // skip #
  // Calculate number of characters.
  while(str[n] && IsHexDigit(str[n]))
    n++;
  if (n == 6) {
    hex2bin((CHAR8*)str, 6, (UINT8*)&c, 3); //big endian
    b = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    r = c & 0xff;
  } else if (n == 3) {
    c = AsciiStrHexToUintn(str);
    r = ((c&0xf00) >> 8) * 17;
    g = ((c&0xf0) >> 4) * 17;
    b = (c&0xf) * 17;
  }
  return NSVG_RGB(r,g,b);
}

static unsigned int nsvg__parseColorRGB(const char* str)
{
  int r = -1, g = -1, b = -1, a = 0;
  float fr, fg, fb, fa;
  char *s1 = NULL;
  AsciiStrToFloat(str+4, &s1, &fr);
  if (*s1 == '%') {
    r = (int)(fr * 2.55f);
    str = s1 + 2;
  } else if (*s1 == ',') {
    r = (int)fr;
    str = s1 + 1;
  } else {
    //error
    DBG("StrFloat error:%s\n", str);
    return NSVG_RGB(0,0,0);
  }
  AsciiStrToFloat(str, &s1, &fg);
  if (*s1 == '%') {
    g = (int)(fg * 2.55f);
    str = s1 + 2;
  } else if (*s1 == ',') {
    g = (int)fg;
    str = s1 + 1;
  } else {
    //error
    DBG("StrFloat error:%s\n", str);
    return NSVG_RGB(0,0,0);
  }
  AsciiStrToFloat(str, &s1, &fb);
  if (*s1++ == '%') {
    b = (int)(fb * 2.55f);
    str = s1 + 1;
  }  else {
    b = (int)fb;
    str = s1;
  }
  if (*s1 == ',') { //there can be no spaces?!
    AsciiStrToFloat(str, &s1, &fa);
    if (*s1 == '%') {
      a = (int)(fa * 2.55f);
    } else {
      a = (int)fa;
    }
    return NSVG_RGBA(r,g,b,a);
  }
  return NSVG_RGB(r,g,b);
}

typedef struct NSVGNamedColor {
  const char* name;
  unsigned int color;
} NSVGNamedColor;

NSVGNamedColor nsvg__colors[] = {

  { "red", NSVG_RGB(255, 0, 0) },
  { "green", NSVG_RGB( 0, 128, 0) },
  { "blue", NSVG_RGB( 0, 0, 255) },
  { "yellow", NSVG_RGB(255, 255, 0) },
  { "cyan", NSVG_RGB( 0, 255, 255) },
  { "magenta", NSVG_RGB(255, 0, 255) },
  { "black", NSVG_RGB( 0, 0, 0) },
  { "grey", NSVG_RGB(128, 128, 128) },
  { "gray", NSVG_RGB(128, 128, 128) },
  { "white", NSVG_RGB(255, 255, 255) },

#ifdef NANOSVG_ALL_COLOR_KEYWORDS
  { "aliceblue", NSVG_RGB(240, 248, 255) },
  { "antiquewhite", NSVG_RGB(250, 235, 215) },
  { "aqua", NSVG_RGB( 0, 255, 255) },
  { "aquamarine", NSVG_RGB(127, 255, 212) },
  { "azure", NSVG_RGB(240, 255, 255) },
  { "beige", NSVG_RGB(245, 245, 220) },
  { "bisque", NSVG_RGB(255, 228, 196) },
  { "blanchedalmond", NSVG_RGB(255, 235, 205) },
  { "blueviolet", NSVG_RGB(138, 43, 226) },
  { "brown", NSVG_RGB(165, 42, 42) },
  { "burlywood", NSVG_RGB(222, 184, 135) },
  { "cadetblue", NSVG_RGB( 95, 158, 160) },
  { "chartreuse", NSVG_RGB(127, 255, 0) },
  { "chocolate", NSVG_RGB(210, 105, 30) },
  { "coral", NSVG_RGB(255, 127, 80) },
  { "cornflowerblue", NSVG_RGB(100, 149, 237) },
  { "cornsilk", NSVG_RGB(255, 248, 220) },
  { "crimson", NSVG_RGB(220, 20, 60) },
  { "darkblue", NSVG_RGB( 0, 0, 139) },
  { "darkcyan", NSVG_RGB( 0, 139, 139) },
  { "darkgoldenrod", NSVG_RGB(184, 134, 11) },
  { "darkgray", NSVG_RGB(169, 169, 169) },
  { "darkgreen", NSVG_RGB( 0, 100, 0) },
  { "darkgrey", NSVG_RGB(169, 169, 169) },
  { "darkkhaki", NSVG_RGB(189, 183, 107) },
  { "darkmagenta", NSVG_RGB(139, 0, 139) },
  { "darkolivegreen", NSVG_RGB( 85, 107, 47) },
  { "darkorange", NSVG_RGB(255, 140, 0) },
  { "darkorchid", NSVG_RGB(153, 50, 204) },
  { "darkred", NSVG_RGB(139, 0, 0) },
  { "darksalmon", NSVG_RGB(233, 150, 122) },
  { "darkseagreen", NSVG_RGB(143, 188, 143) },
  { "darkslateblue", NSVG_RGB( 72, 61, 139) },
  { "darkslategray", NSVG_RGB( 47, 79, 79) },
  { "darkslategrey", NSVG_RGB( 47, 79, 79) },
  { "darkturquoise", NSVG_RGB( 0, 206, 209) },
  { "darkviolet", NSVG_RGB(148, 0, 211) },
  { "deeppink", NSVG_RGB(255, 20, 147) },
  { "deepskyblue", NSVG_RGB( 0, 191, 255) },
  { "dimgray", NSVG_RGB(105, 105, 105) },
  { "dimgrey", NSVG_RGB(105, 105, 105) },
  { "dodgerblue", NSVG_RGB( 30, 144, 255) },
  { "firebrick", NSVG_RGB(178, 34, 34) },
  { "floralwhite", NSVG_RGB(255, 250, 240) },
  { "forestgreen", NSVG_RGB( 34, 139, 34) },
  { "fuchsia", NSVG_RGB(255, 0, 255) },
  { "gainsboro", NSVG_RGB(220, 220, 220) },
  { "ghostwhite", NSVG_RGB(248, 248, 255) },
  { "gold", NSVG_RGB(255, 215, 0) },
  { "goldenrod", NSVG_RGB(218, 165, 32) },
  { "greenyellow", NSVG_RGB(173, 255, 47) },
  { "honeydew", NSVG_RGB(240, 255, 240) },
  { "hotpink", NSVG_RGB(255, 105, 180) },
  { "indianred", NSVG_RGB(205, 92, 92) },
  { "indigo", NSVG_RGB( 75, 0, 130) },
  { "ivory", NSVG_RGB(255, 255, 240) },
  { "khaki", NSVG_RGB(240, 230, 140) },
  { "lavender", NSVG_RGB(230, 230, 250) },
  { "lavenderblush", NSVG_RGB(255, 240, 245) },
  { "lawngreen", NSVG_RGB(124, 252, 0) },
  { "lemonchiffon", NSVG_RGB(255, 250, 205) },
  { "lightblue", NSVG_RGB(173, 216, 230) },
  { "lightcoral", NSVG_RGB(240, 128, 128) },
  { "lightcyan", NSVG_RGB(224, 255, 255) },
  { "lightgoldenrodyellow", NSVG_RGB(250, 250, 210) },
  { "lightgray", NSVG_RGB(211, 211, 211) },
  { "lightgreen", NSVG_RGB(144, 238, 144) },
  { "lightgrey", NSVG_RGB(211, 211, 211) },
  { "lightpink", NSVG_RGB(255, 182, 193) },
  { "lightsalmon", NSVG_RGB(255, 160, 122) },
  { "lightseagreen", NSVG_RGB( 32, 178, 170) },
  { "lightskyblue", NSVG_RGB(135, 206, 250) },
  { "lightslategray", NSVG_RGB(119, 136, 153) },
  { "lightslategrey", NSVG_RGB(119, 136, 153) },
  { "lightsteelblue", NSVG_RGB(176, 196, 222) },
  { "lightyellow", NSVG_RGB(255, 255, 224) },
  { "lime", NSVG_RGB( 0, 255, 0) },
  { "limegreen", NSVG_RGB( 50, 205, 50) },
  { "linen", NSVG_RGB(250, 240, 230) },
  { "maroon", NSVG_RGB(128, 0, 0) },
  { "mediumaquamarine", NSVG_RGB(102, 205, 170) },
  { "mediumblue", NSVG_RGB( 0, 0, 205) },
  { "mediumorchid", NSVG_RGB(186, 85, 211) },
  { "mediumpurple", NSVG_RGB(147, 112, 219) },
  { "mediumseagreen", NSVG_RGB( 60, 179, 113) },
  { "mediumslateblue", NSVG_RGB(123, 104, 238) },
  { "mediumspringgreen", NSVG_RGB( 0, 250, 154) },
  { "mediumturquoise", NSVG_RGB( 72, 209, 204) },
  { "mediumvioletred", NSVG_RGB(199, 21, 133) },
  { "midnightblue", NSVG_RGB( 25, 25, 112) },
  { "mintcream", NSVG_RGB(245, 255, 250) },
  { "mistyrose", NSVG_RGB(255, 228, 225) },
  { "moccasin", NSVG_RGB(255, 228, 181) },
  { "navajowhite", NSVG_RGB(255, 222, 173) },
  { "navy", NSVG_RGB( 0, 0, 128) },
  { "oldlace", NSVG_RGB(253, 245, 230) },
  { "olive", NSVG_RGB(128, 128, 0) },
  { "olivedrab", NSVG_RGB(107, 142, 35) },
  { "orange", NSVG_RGB(255, 165, 0) },
  { "orangered", NSVG_RGB(255, 69, 0) },
  { "orchid", NSVG_RGB(218, 112, 214) },
  { "palegoldenrod", NSVG_RGB(238, 232, 170) },
  { "palegreen", NSVG_RGB(152, 251, 152) },
  { "paleturquoise", NSVG_RGB(175, 238, 238) },
  { "palevioletred", NSVG_RGB(219, 112, 147) },
  { "papayawhip", NSVG_RGB(255, 239, 213) },
  { "peachpuff", NSVG_RGB(255, 218, 185) },
  { "peru", NSVG_RGB(205, 133, 63) },
  { "pink", NSVG_RGB(255, 192, 203) },
  { "plum", NSVG_RGB(221, 160, 221) },
  { "powderblue", NSVG_RGB(176, 224, 230) },
  { "purple", NSVG_RGB(128, 0, 128) },
  { "rosybrown", NSVG_RGB(188, 143, 143) },
  { "royalblue", NSVG_RGB( 65, 105, 225) },
  { "saddlebrown", NSVG_RGB(139, 69, 19) },
  { "salmon", NSVG_RGB(250, 128, 114) },
  { "sandybrown", NSVG_RGB(244, 164, 96) },
  { "seagreen", NSVG_RGB( 46, 139, 87) },
  { "seashell", NSVG_RGB(255, 245, 238) },
  { "sienna", NSVG_RGB(160, 82, 45) },
  { "silver", NSVG_RGB(192, 192, 192) },
  { "skyblue", NSVG_RGB(135, 206, 235) },
  { "slateblue", NSVG_RGB(106, 90, 205) },
  { "slategray", NSVG_RGB(112, 128, 144) },
  { "slategrey", NSVG_RGB(112, 128, 144) },
  { "snow", NSVG_RGB(255, 250, 250) },
  { "springgreen", NSVG_RGB( 0, 255, 127) },
  { "steelblue", NSVG_RGB( 70, 130, 180) },
  { "tan", NSVG_RGB(210, 180, 140) },
  { "teal", NSVG_RGB( 0, 128, 128) },
  { "thistle", NSVG_RGB(216, 191, 216) },
  { "tomato", NSVG_RGB(255, 99, 71) },
  { "turquoise", NSVG_RGB( 64, 224, 208) },
  { "violet", NSVG_RGB(238, 130, 238) },
  { "wheat", NSVG_RGB(245, 222, 179) },
  { "whitesmoke", NSVG_RGB(245, 245, 245) },
  { "yellowgreen", NSVG_RGB(154, 205, 50) },
#endif
};

static unsigned int nsvg__parseColorName(const char* str)
{
  int i, ncolors = sizeof(nsvg__colors) / sizeof(NSVGNamedColor);
  //  DBG("namedcolor=%d\n", sizeof(NSVGNamedColor));
#if 0
  for (i = 0; i < ncolors; i++) {
    if (strcmp(nsvg__colors[i].name, str) == 0) {
      return nsvg__colors[i].color;
    }
  }
#else
  int low, high, med;
  INTN res;
  low = 10;
  high = ncolors - 1;
  for (i = 0; i < 10; i++) {
    if (strcmp(nsvg__colors[i].name, str) == 0) {
      return nsvg__colors[i].color;
    }
  }

  while (low <= high)
  {
    med = (low + high) / 2;
    res = strcmp(nsvg__colors[med].name, str);
    if(res < 0)
      low = med + 1;
    else if (res > 0)
      high = med - 1;
    else
      return nsvg__colors[med].color;
  }
#endif

  return NSVG_RGB(128, 128, 128); //if not found then Grey50%
}

/*
static unsigned int nsvg__parseColorRGBA(const char* str)
{
  int r = -1, g = -1, b = -1;
  float a = -1;
  char s1[32]="", s2[32]="", s3[32]="";
  sscanf(str + 5, "%d%[%%, \t]%d%[%%, \t]%d%[%%, \t]%f", &r, s1, &g, s2, &b, s3, &a);
  if (strchr(s1, '%')) {
    return NSVG_RGBA((r*255)/100,(g*255)/100,(b*255)/100,(a*255)/100);
  } else {
    return NSVG_RGBA(r,g,b,(a*255));
  }
}
 */

static unsigned int nsvg__parseColor(const char* str)
{
  size_t len = 0;
  while(*str == ' ') ++str;
  len = strlen(str);
  if (len >= 1 && *str == '#')
    return nsvg__parseColorHex(str);
  else if (len >= 4 && str[0] == 'r' && str[1] == 'g' && str[2] == 'b' && str[3] == '(')
    return nsvg__parseColorRGB(str);
  else if (len >= 5 && str[0] == 'r' && str[1] == 'g' && str[2] == 'b' && str[3] == 'a' && str[4] == '(')
    return nsvg__parseColorRGB(str);

  return nsvg__parseColorName(str);
}

static float nsvg__parseOpacity(const char* str)
{
  float val = 0;
  //  sscanf(str, "%f", &val);
  AsciiStrToFloat(str, NULL, &val);
  if (val < 0.0f) val = 0.0f;
  if (val > 1.0f) val = 1.0f;
  return val;
}

static float nsvg__parseMiterLimit(const char* str)
{
  float val = 0;
  //  sscanf(str, "%f", &val);
  AsciiStrToFloat(str, NULL, &val);
  if (val < 0.0f) val = 0.0f;
  return val;
}

static int nsvg__parseUnits(const char* units)
{
  if (!units || units[0] == '\0') {
    return NSVG_UNITS_USER;
  }
  if (units[0] == 'p' && units[1] == 'x')
    return NSVG_UNITS_PX;
  else if (units[0] == 'p' && units[1] == 't')
    return NSVG_UNITS_PT;
  else if (units[0] == 'p' && units[1] == 'c')
    return NSVG_UNITS_PC;
  else if (units[0] == 'm' && units[1] == 'm')
    return NSVG_UNITS_MM;
  else if (units[0] == 'c' && units[1] == 'm')
    return NSVG_UNITS_CM;
  else if (units[0] == 'i' && units[1] == 'n')
    return NSVG_UNITS_IN;
  else if (units[0] == '%')
    return NSVG_UNITS_PERCENT;
  else if (units[0] == 'e' && units[1] == 'm')
    return NSVG_UNITS_EM;
  else if (units[0] == 'e' && units[1] == 'x')
    return NSVG_UNITS_EX;
  return NSVG_UNITS_USER;
}

static int nsvg__isCoordinate(const char* s)
{
  // optional sign
  if (*s == '-' || *s == '+')
    s++;
  // must have at least one digit, or start by a dot
  return (nsvg__isdigit(*s) || *s == '.');
}

static NSVGcoordinate nsvg__parseCoordinateRaw(const char* str)
{
  NSVGcoordinate coord = {0, NSVG_UNITS_USER};
  //  char units[32]="";
  char* UnitsStr = NULL;
  //  sscanf(str, "%f%31s", &coord.value, units);
  AsciiStrToFloat(str, &UnitsStr, &coord.value);

  //  coord.units = nsvg__parseUnits(units);
  coord.units = nsvg__parseUnits((const char*)UnitsStr);
  return coord;
}

static NSVGcoordinate nsvg__coord(float v, int units)
{
  NSVGcoordinate coord;
  coord.value = v;
  coord.units = units;
  return coord;
}

static float nsvg__parseCoordinate(NSVGparser* p, const char* str, float orig, float length)
{
  NSVGcoordinate coord = nsvg__parseCoordinateRaw(str);
  float v = nsvg__convertToPixels(p, &coord, orig, length);
  return v;
}

static int nsvg__parseTransformArgs(const char* str, float* args, int maxNa, int* na)
{
  const char* end;
  const char* ptr;
  char it[64];

  *na = 0;
  ptr = str;
  while (*ptr && *ptr != '(') ++ptr;
  if (*ptr == 0)
    return 1;
  end = ptr;
  while (*end && *end != ')') ++end;
  if (*end == 0)
    return 1;

  while (ptr < end) {
    if (*ptr == '-' || *ptr == '+' || *ptr == '.' || nsvg__isdigit(*ptr)) {
      if (*na >= maxNa) return 0;
      ptr = nsvg__parseNumber(ptr, it, 64);
      args[(*na)++] = (float)nsvg__atof(it);
    } else {
      ++ptr;
    }
  }
  return (int)(end - str);
}


static int nsvg__parseMatrix(float* xform, const char* str)
{
  float t[6];
  int na = 0;
  int len = nsvg__parseTransformArgs(str, t, 6, &na);
  if (na != 6) return len;
  memcpy(xform, t, sizeof(float)*6);
  return len;
}

static int nsvg__parseTranslate(float* xform, const char* str)
{
  float args[2];
  float t[6];
  int na = 0;
  int len = nsvg__parseTransformArgs(str, args, 2, &na);
  if (na == 1) args[1] = 0.0;

  nsvg__xformSetTranslation(t, args[0], args[1]);
  memcpy(xform, t, sizeof(float)*6);
  return len;
}

static int nsvg__parseScale(float* xform, const char* str)
{
  float args[2];
  int na = 0;
  float t[6];
  int len = nsvg__parseTransformArgs(str, args, 2, &na);
  if (na == 1) args[1] = args[0];
  nsvg__xformSetScale(t, args[0], args[1]);
  memcpy(xform, t, sizeof(float)*6);
  return len;
}

static int nsvg__parseSkewX(float* xform, const char* str)
{
  float args[1];
  int na = 0;
  float t[6];
  int len = nsvg__parseTransformArgs(str, args, 1, &na);
  nsvg__xformSetSkewX(t, args[0] * NSVG_PI_DEG);
  memcpy(xform, t, sizeof(float)*6);
  return len;
}

static int nsvg__parseSkewY(float* xform, const char* str)
{
  float args[1];
  int na = 0;
  float t[6];
  int len = nsvg__parseTransformArgs(str, args, 1, &na);
  nsvg__xformSetSkewY(t, args[0] * NSVG_PI_DEG);
  memcpy(xform, t, sizeof(float)*6);
  return len;
}

static int nsvg__parseRotate(float* xform, const char* str)
{
  float args[3];
  int na = 0;
  float m[6];
  float t[6];
  int len = nsvg__parseTransformArgs(str, args, 3, &na);
  if (na == 1)
    args[1] = args[2] = 0.0f;
  nsvg__xformIdentity(m);

  if (na > 1) {
    nsvg__xformSetTranslation(t, -args[1], -args[2]);
    nsvg__xformMultiply(m, t);
  }

  nsvg__xformSetRotation(t, args[0] * NSVG_PI_DEG);
  nsvg__xformMultiply(m, t);

  if (na > 1) {
    nsvg__xformSetTranslation(t, args[1], args[2]);
    nsvg__xformMultiply(m, t);
  }

  memcpy(xform, m, sizeof(float)*6);

  return len;
}

static void nsvg__parseTransform(float* xform, const char* str)
{
  float t[6];
  int len;
  nsvg__xformIdentity(xform);

  while (*str)
  {
    if (strncmp(str, "matrix", 6) == 0)
      len = nsvg__parseMatrix(t, str);
    else if (strncmp(str, "translate", 9) == 0)
      len = nsvg__parseTranslate(t, str);
    else if (strncmp(str, "scale", 5) == 0)
      len =nsvg__parseScale(t, str);
    else if (strncmp(str, "rotate", 6) == 0)
      len = nsvg__parseRotate(t, str);
    else if (strncmp(str, "skewX", 5) == 0)
      len = nsvg__parseSkewX(t, str);
    else if (strncmp(str, "skewY", 5) == 0)
      len = nsvg__parseSkewY(t, str);
    else{
      ++str;
      continue;
    }
    if (len != 0) {
      str += len;
    } else {
      ++str;
      continue;
    }
    nsvg__xformPremultiply(xform, t);
  }
}

static void nsvg__parseUrl(char* id, const char* str)
{
  int i = 0;
  str += 4; // "url(";
  if (*str == '#')
    str++;
  while (i < 63 && *str != ')') {
    id[i] = *str++;
    i++;
  }
  id[i] = '\0';
}

static char nsvg__parseLineCap(const char* str)
{
  if (strcmp(str, "butt") == 0)
    return NSVG_CAP_BUTT;
  else if (strcmp(str, "round") == 0)
    return NSVG_CAP_ROUND;
  else if (strcmp(str, "square") == 0)
    return NSVG_CAP_SQUARE;
  // TODO: handle inherit.
  return NSVG_CAP_BUTT;
}

static char nsvg__parseLineJoin(const char* str)
{
  if (strcmp(str, "miter") == 0)
    return NSVG_JOIN_MITER;
  else if (strcmp(str, "round") == 0)
    return NSVG_JOIN_ROUND;
  else if (strcmp(str, "bevel") == 0)
    return NSVG_JOIN_BEVEL;
  // TODO: handle inherit.
  return NSVG_JOIN_MITER;
}

static char nsvg__parseFillRule(const char* str)
{
  if (strcmp(str, "nonzero") == 0)
    return NSVG_FILLRULE_NONZERO;
  else if (strcmp(str, "evenodd") == 0)
    return NSVG_FILLRULE_EVENODD;
  // TODO: handle inherit.
  return NSVG_FILLRULE_NONZERO;
}

static const char* nsvg__getNextDashItem(const char* s, char* it)
{
  int n = 0;
  it[0] = '\0';
  // Skip white spaces and commas
  while (*s && (nsvg__isspace(*s) || *s == ',')) s++;
  // Advance until whitespace, comma or end.
  while (*s && (!nsvg__isspace(*s) && *s != ',')) {
    if (n < 63)
      it[n++] = *s;
    s++;
  }
  it[n++] = '\0';
  return s;
}

static int nsvg__parseStrokeDashArray(NSVGparser* p, const char* str, float* strokeDashArray)
{
  char item[64];
  int count = 0, i;
  float sum = 0.0f;

  // Handle "none"
  if (str[0] == 'n')
    return 0;

  // Parse dashes
  while (*str) {
    str = nsvg__getNextDashItem(str, item);
    if (!*item) break;
    if (count < NSVG_MAX_DASHES)
      strokeDashArray[count++] = fabsf(nsvg__parseCoordinate(p, item, 0.0f, nsvg__actualLength(p)));
  }

  for (i = 0; i < count; i++)
    sum += strokeDashArray[i];
  if (sum <= 1e-6f)
    count = 0;

  return count;
}

static NSVGclipPath* nsvg__createClipPath(const char* name, int index)
{
  NSVGclipPath* clipPath = (NSVGclipPath*)nsvg__alloczero(sizeof(NSVGclipPath), "nsvg__createClipPath"_XS8);
  if (clipPath == NULL) return NULL;

  strncpy(clipPath->id, name, 63);
  clipPath->id[63] = '\0';
  clipPath->index = (NSVGclipPathIndex)index;
  return clipPath;
}

static NSVGclipPath* nsvg__findClipPath(NSVGparser* p, const char* name)
{
  int i = 0;
  NSVGclipPath** link;
  NSVGattrib* attr = nsvg__getAttr(p);

  link = &p->image->clipPaths;
  while (*link != NULL) {
    if (strcmp((*link)->id, name) == 0) {
      break;
    }
    link = &(*link)->next;
    i++;
  }
  if (*link == NULL) {
    *link = nsvg__createClipPath(name, i);
    (*link)->group = attr->group;
  }
  return *link;
}

static int nsvg__substr(const char* aClass, char* style)
{
  const char *p;

  while (*aClass) {
    char *s = style;
    p = aClass++;
    while (*p++ == *s++) {
      if (*s == '\0') {
        if ((*p == '\0') || (*p == ' ')) {
          return 1;
        } else break;
      }
    }
  }
  return 0;
}

static void nsvg__parseStyle(NSVGparser* p, const char* str);

static int nsvg__parseAttr(NSVGparser* p, const char* name, char* value)
{
  float xform[6];
  //  DBG("parse Name:%s Value:%s\n", name, value);
  NSVGattrib* attr = nsvg__getAttr(p);
  if (!attr) return 0;
  if (strcmp(name, "style") == 0) {
    nsvg__parseStyle(p, value);
  } else if (strcmp(name, "display") == 0) {
    if (strcmp(value, "none") == 0) {
      attr->visible &= ~NSVG_VIS_DISPLAY;
    }
    // Don't reset ->visible on display:inline, one display:none hides the whole subtree
  } else if (strcmp(name, "visibility") == 0) {
    if (strcmp(value, "hidden") == 0) {
      attr->visible &= ~NSVG_VIS_VISIBLE;
    } else if (strcmp(value, "visible") == 0) {
      attr->visible |= NSVG_VIS_VISIBLE;
    }
  } else if (strcmp(name, "fill") == 0) {
    if (strcmp(value, "none") == 0) {
      attr->hasFill = 0;
    } else if (strncmp(value, "url(", 4) == 0) {
      if (strstr(value, "pattern")) {
        attr->hasFill = 3;
      } else {
        attr->hasFill = 2;
      }
      nsvg__parseUrl(attr->fillGradient, value);

    } else {
      attr->hasFill = 1;
      attr->fillColor = nsvg__parseColor(value);
      // if the fillColor has an alpha value then use it to
      // set the fillOpacity
      if (attr->fillColor & 0xFF000000) {
        attr->fillOpacity = (float)(((attr->fillColor >> 24) & 0xFF)) / 255.0f; // safe cast
        // remove the alpha value from the color
        attr->fillColor &= 0x00FFFFFF;
      }
    }
  } else if (strcmp(name, "opacity") == 0) {
    float opacity = nsvg__parseOpacity(value);
    attr->opacity *= opacity;
    if (attr->opacity == 0.0f) {
      attr->opacity = p->opacity;  //some trick for seal an image on preview
    }
  } else if (strcmp(name, "fill-opacity") == 0) {
    attr->fillOpacity = nsvg__parseOpacity(value);
  } else if (strcmp(name, "stroke") == 0) {
    if (strcmp(value, "none") == 0) {
      attr->hasStroke = 0;
    } else if (strncmp(value, "url(", 4) == 0) {
      if (strstr(value, "pattern")) {
        attr->hasStroke = 3;
      } else {
        attr->hasStroke = 2;
      }
      nsvg__parseUrl(attr->strokeGradient, value);
    } else {
      attr->hasStroke = 1;
      attr->strokeColor = nsvg__parseColor(value);
      // if the strokeColor has an alpha value then use it to
      // set the strokeOpacity
      if (attr->strokeColor & 0xFF000000) {
        attr->strokeOpacity = (float)(((attr->strokeColor >> 24) & 0xFF)) / 255.0f; // safe cast
        // remove the alpha value from the color
        attr->strokeColor &= 0x00FFFFFF;
      }
    }
  } else if (strcmp(name, "stroke-width") == 0) {
    attr->strokeWidth = nsvg__parseCoordinate(p, value, 0.0f, nsvg__actualLength(p));
  } else if (strcmp(name, "stroke-dasharray") == 0) {
    attr->strokeDashCount = nsvg__parseStrokeDashArray(p, value, attr->strokeDashArray);
  } else if (strcmp(name, "stroke-dashoffset") == 0) {
    attr->strokeDashOffset = nsvg__parseCoordinate(p, value, 0.0f, nsvg__actualLength(p));
  } else if (strcmp(name, "stroke-opacity") == 0) {
    attr->strokeOpacity = nsvg__parseOpacity(value);
  } else if (strcmp(name, "stroke-linecap") == 0) {
    attr->strokeLineCap = nsvg__parseLineCap(value);
  } else if (strcmp(name, "stroke-linejoin") == 0) {
    attr->strokeLineJoin = nsvg__parseLineJoin(value);
  } else if (strcmp(name, "stroke-miterlimit") == 0) {
    attr->miterLimit = nsvg__parseMiterLimit(value);
  } else if (strcmp(name, "fill-rule") == 0) {
    attr->fillRule = nsvg__parseFillRule(value);
  } else if (strcmp(name, "font-size") == 0) {
    if (!attr->fontFace) {
      //      DBG("font face=%d\n", sizeof(NSVGfont));
      attr->fontFace = (NSVGfont*)nsvg__alloczero(sizeof(NSVGfont), "nsvg__parseAttr fontFace"_XS8);
    }
    attr->fontFace->fontSize = nsvg__parseCoordinate(p, value, 0.0f, nsvg__actualLength(p));
  } else if (strcmp(name, "clip-path") == 0) {
    if (strncmp(value, "url(", 4) == 0 && attr->clipPathCount < NSVG_MAX_CLIP_PATHS) {
      char clipName[64];
      nsvg__parseUrl(clipName, value);
      NSVGclipPath *clipPath = nsvg__findClipPath(p, clipName);
      p->clipPathStack[attr->clipPathCount++] = clipPath->index;
    }
  } else if (strcmp(name, "stop-color") == 0) {
    attr->stopColor = nsvg__parseColor(value);
  } else if (strcmp(name, "stop-opacity") == 0) {
    attr->stopOpacity = nsvg__parseOpacity(value);
  } else if (strcmp(name, "offset") == 0) {
    attr->stopOffset = nsvg__parseCoordinate(p, value, 0.0f, 1.0f);
  } else if (strcmp(name, "font-family") == 0) {
    if (!attr->fontFace) {
      //     DBG("font face=%d\n", sizeof(NSVGfont));
      attr->fontFace = (NSVGfont*)nsvg__alloczero(sizeof(NSVGfont), "nsvg__parseAttr fontFace2"_XS8);
    }
    if (attr->fontFace) {
      if (value[0] == 0x27) {  //'
        CHAR8* apo = strstr(++value, "'");
        if (apo) apo[0] = '\0';
      }
 //     DBG("reduced font-family:%s\n", value);
      strncpy(attr->fontFace->fontFamily, value, 63);
      attr->fontFace->fontFamily[63] = '\0';
    }
  } else if (strcmp(name, "font-weight") == 0) {
    if (!attr->fontFace) {
      attr->fontFace = (NSVGfont*)nsvg__alloczero(sizeof(NSVGfont), "nsvg__parseAttr fontFace3"_XS8);
    }
    if (attr->fontFace) {
      //      char* Next = 0;
      float fontWeight = 0.0f;
      AsciiStrToFloat(value, NULL /*&Next*/, &fontWeight);
      attr->fontFace->fontWeight = fontWeight;
    }
  } else if (strcmp(name, "font-style") == 0)  {
    DBG("attr=%s value=%s\n", name, value);
    if (!attr->fontFace) {
      attr->fontFace = (NSVGfont*)nsvg__alloczero(sizeof(NSVGfont), "nsvg__parseAttr fontFace4"_XS8);
    }
    if (strstr(value, "italic") != NULL)  {
      DBG("it is italic\n");
      attr->fontFace->fontStyle = 'i';
    } else if (strstr(value, "bold") != NULL) {
      DBG("it is bold\n");
      attr->fontFace->fontStyle = 'b';
    } else if (strstr(value, "light") != NULL) {
      DBG("it is light\n");
      attr->fontFace->fontStyle = 'l';
    } else {
      DBG("it is other\n");
      attr->fontFace->fontStyle = 'n';
    }
  } else if (strcmp(name, "id") == 0) {
    strncpy(attr->id, value, sizeof(attr->id)-1); // -1 or not -1, doesn't change a thing because of the next line.
    attr->id[63] = '\0';
  } else if (strcmp(name, "x") == 0) {
    nsvg__xformSetTranslation(xform, (float)nsvg__atof(value), 0);
    nsvg__xformPremultiply(attr->xform, xform);
  } else if (strcmp(name, "y") == 0) {
    nsvg__xformSetTranslation(xform, 0, (float)nsvg__atof(value));
    nsvg__xformPremultiply(attr->xform, xform);
  } else if (strcmp(name, "transform") == 0) {
    nsvg__parseTransform(xform, value);
    nsvg__xformPremultiply(attr->xform, xform);
  }
  else if (strcmp(name, "class") == 0) {
    NSVGstyles* style = p->styles;
    while (style) {
      if (nsvg__substr(value, style->name) != 0) {
        nsvg__parseStyle(p, style->description);
      }
      style = style->next;
    }
  }
  else {
    return 0;
  }
  return 1;
}

static int nsvg__parseNameValue(NSVGparser* p, const char* start, const char* end)
{
  const char* str;
  const char* val;
  char name[512];
  char value[512];
  int n;

  str = start;
  while (str < end && *str != ':') ++str;

  val = str;

  // Right Trim
  while (str > start &&  (*str == ':' || nsvg__isspace(*str))) --str;
  ++str;

  n = (int)(str - start);
  if (n > 511) n = 511;
  if (n) memcpy(name, start, n);
  name[n] = 0;

  while (val < end && (*val == ':' || nsvg__isspace(*val))) ++val;

  n = (int)(end - val);
  if (n > 511) n = 511;
  if (n) memcpy(value, val, n);
  value[n] = 0;

  n = nsvg__parseAttr(p, name, value);
  return n;
}

static void nsvg__parseStyle(NSVGparser* p, const char* str)
{
  const char* start;
  const char* end;

  if (str == NULL) return;

  while (*str) {
    // Left Trim
    while(*str && nsvg__isspace(*str)) ++str;
    start = str;
    while(*str && *str != ';') ++str;
    end = str;

    // Right Trim
    while (end > start &&  (*end == ';' || nsvg__isspace(*end))) --end;
    ++end;
    nsvg__parseNameValue(p, start, end);
    if (*str) ++str;
  }
}
/*
static void nsvg__parseAttribs(NSVGparser* p, const char** dict)
{
  int i;
  for (i = 0; dict[i]; i += 2)
  {
    if (strcmp(dict[i], "style") == 0)
      nsvg__parseStyle(p, dict[i + 1]);
    else {
      nsvg__parseAttr(p, dict[i], dict[i + 1]);
    }
  }
}
 */
static int nsvg__getArgsPerElement(char cmd)
{
  switch (cmd) {
    case 'v':
    case 'V':
    case 'h':
    case 'H':
      return 1;
    case 'm':
    case 'M':
    case 'l':
    case 'L':
    case 't':
    case 'T':
      return 2;
    case 'q':
    case 'Q':
    case 's':
    case 'S':
      return 4;
    case 'c':
    case 'C':
      return 6;
    case 'a':
    case 'A':
      return 7;
    case 'z':
    case 'Z':
      return 0;
  }
  return -1;
}

static void nsvg__pathMoveTo(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
{
  if (rel) {
    *cpx += args[0];
    *cpy += args[1];
  } else {
    *cpx = args[0];
    *cpy = args[1];
  }
  nsvg__moveTo(p, *cpx, *cpy);
}

static void nsvg__pathLineTo(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
{
  if (rel) {
    *cpx += args[0];
    *cpy += args[1];
  } else {
    *cpx = args[0];
    *cpy = args[1];
  }
  nsvg__lineTo(p, *cpx, *cpy);
}

static void nsvg__pathHLineTo(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
{
  if (rel)
    *cpx += args[0];
  else
    *cpx = args[0];
  nsvg__lineTo(p, *cpx, *cpy);
}

static void nsvg__pathVLineTo(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
{
  if (rel)
    *cpy += args[0];
  else
    *cpy = args[0];
  nsvg__lineTo(p, *cpx, *cpy);
}

static void nsvg__pathCubicBezTo(NSVGparser* p, float* cpx, float* cpy,
                                 float* cpx2, float* cpy2, float* args, int rel)
{
  float x2, y2, cx1, cy1, cx2, cy2;

  if (rel) {
    cx1 = *cpx + args[0];
    cy1 = *cpy + args[1];
    cx2 = *cpx + args[2];
    cy2 = *cpy + args[3];
    x2 = *cpx + args[4];
    y2 = *cpy + args[5];
  } else {
    cx1 = args[0];
    cy1 = args[1];
    cx2 = args[2];
    cy2 = args[3];
    x2 = args[4];
    y2 = args[5];
  }

  nsvg__cubicBezTo(p, cx1,cy1, cx2,cy2, x2,y2);

  *cpx2 = cx2;
  *cpy2 = cy2;
  *cpx = x2;
  *cpy = y2;
}

static void nsvg__pathCubicBezShortTo(NSVGparser* p, float* cpx, float* cpy,
                                      float* cpx2, float* cpy2, float* args, int rel)
{
  float x1, y1, x2, y2, cx1, cy1, cx2, cy2;

  x1 = *cpx;
  y1 = *cpy;
  if (rel) {
    cx2 = *cpx + args[0];
    cy2 = *cpy + args[1];
    x2 = *cpx + args[2];
    y2 = *cpy + args[3];
  } else {
    cx2 = args[0];
    cy2 = args[1];
    x2 = args[2];
    y2 = args[3];
  }

  cx1 = 2*x1 - *cpx2;
  cy1 = 2*y1 - *cpy2;

  nsvg__cubicBezTo(p, cx1,cy1, cx2,cy2, x2,y2);

  *cpx2 = cx2;
  *cpy2 = cy2;
  *cpx = x2;
  *cpy = y2;
}

static void nsvg__pathQuadBezTo(NSVGparser* p, float* cpx, float* cpy,
                                float* cpx2, float* cpy2, float* args, int rel)
{
  float x1, y1, x2, y2, cx, cy;
  float cx1, cy1, cx2, cy2;

  x1 = *cpx;
  y1 = *cpy;
  if (rel) {
    cx = *cpx + args[0];
    cy = *cpy + args[1];
    x2 = *cpx + args[2];
    y2 = *cpy + args[3];
  } else {
    cx = args[0];
    cy = args[1];
    x2 = args[2];
    y2 = args[3];
  }

  // Convert to cubic bezier
  cx1 = x1 + 2.0f/3.0f*(cx - x1);
  cy1 = y1 + 2.0f/3.0f*(cy - y1);
  cx2 = x2 + 2.0f/3.0f*(cx - x2);
  cy2 = y2 + 2.0f/3.0f*(cy - y2);

  nsvg__cubicBezTo(p, cx1,cy1, cx2,cy2, x2,y2);

  *cpx2 = cx;
  *cpy2 = cy;
  *cpx = x2;
  *cpy = y2;
}

static void nsvg__pathQuadBezShortTo(NSVGparser* p, float* cpx, float* cpy,
                                     float* cpx2, float* cpy2, float* args, int rel)
{
  float x1, y1, x2, y2, cx, cy;
  float cx1, cy1, cx2, cy2;

  x1 = *cpx;
  y1 = *cpy;
  if (rel) {
    x2 = *cpx + args[0];
    y2 = *cpy + args[1];
  } else {
    x2 = args[0];
    y2 = args[1];
  }

  cx = 2*x1 - *cpx2;
  cy = 2*y1 - *cpy2;

  // Convert to cubix bezier
  cx1 = x1 + 2.0f/3.0f*(cx - x1);
  cy1 = y1 + 2.0f/3.0f*(cy - y1);
  cx2 = x2 + 2.0f/3.0f*(cx - x2);
  cy2 = y2 + 2.0f/3.0f*(cy - y2);

  nsvg__cubicBezTo(p, cx1,cy1, cx2,cy2, x2,y2);

  *cpx2 = cx;
  *cpy2 = cy;
  *cpx = x2;
  *cpy = y2;
}

static void nsvg__pathArcTo(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
{
  // Ported from canvg (https://code.google.com/p/canvg/)
  float rx, ry, rotx;
  float x1, y1, x2, y2, cx, cy, dx, dy, d;
  float x1p, y1p, cxp, cyp, s, sa, sb;
  float ux, uy, vx, vy, a1, da;
  float x, y, tanx, tany, a, px = 0, py = 0, ptanx = 0, ptany = 0, t[6];
  float sinrx, cosrx;
  int fa, fs;
  int i, ndivs;
  float hda, kappa;

  rx = fabsf(args[0]);        // y radius
  ry = fabsf(args[1]);        // x radius
  rotx = args[2] * NSVG_PI_DEG;    // x rotation angle
  fa = fabsf(args[3]) > 1e-6 ? 1 : 0;  // Large arc
  fs = fabsf(args[4]) > 1e-6 ? 1 : 0;  // Sweep direction
  x1 = *cpx;              // start point
  y1 = *cpy;
  if (rel) {              // end point
    x2 = *cpx + args[5];
    y2 = *cpy + args[6];
  } else {
    x2 = args[5];
    y2 = args[6];
  }
  dx = x1 - x2;
  dy = y1 - y2;
//  d = sqrtf(dx*dx + dy*dy);
  d = nsvg__vmag(dx, dy);
  if (d < 1e-6f || rx < 1e-6f || ry < 1e-6f) {
    // The arc degenerates to a line
    nsvg__lineTo(p, x2, y2);
    *cpx = x2;
    *cpy = y2;
    return;
  }

  sinrx = sinf(rotx);
  cosrx = cosf(rotx);

  // Convert to center point parameterization.
  // http://www.w3.org/TR/SVG11/implnote.html#ArcImplementationNotes
  // 1) Compute x1', y1'
  x1p = cosrx * dx / 2.0f + sinrx * dy / 2.0f;
  y1p = -sinrx * dx / 2.0f + cosrx * dy / 2.0f;
  d = nsvg__sqr(x1p)/nsvg__sqr(rx) + nsvg__sqr(y1p)/nsvg__sqr(ry);
  if (d > 1) {
    d = sqrtf(d);
    rx *= d;
    ry *= d;
  }
  // 2) Compute cx', cy'
  s = 0.0f;
  sa = nsvg__sqr(rx)*nsvg__sqr(ry) - nsvg__sqr(rx)*nsvg__sqr(y1p) - nsvg__sqr(ry)*nsvg__sqr(x1p);
  sb = nsvg__sqr(rx)*nsvg__sqr(y1p) + nsvg__sqr(ry)*nsvg__sqr(x1p);
  if (sa < 0.0f) sa = 0.0f;
  if (sb > 0.0f)
    s = sqrtf(sa / sb);
  if (fa == fs)
    s = -s;
  cxp = s * rx * y1p / ry;
  cyp = s * -ry * x1p / rx;

  // 3) Compute cx,cy from cx',cy'
  cx = (x1 + x2)/2.0f + cosrx*cxp - sinrx*cyp;
  cy = (y1 + y2)/2.0f + sinrx*cxp + cosrx*cyp;

  // 4) Calculate theta1, and delta theta.
  ux = (x1p - cxp) / rx;
  uy = (y1p - cyp) / ry;
  vx = (-x1p - cxp) / rx;
  vy = (-y1p - cyp) / ry;
  a1 = nsvg__vecang(1.0f,0.0f, ux,uy);  // Initial angle
  da = nsvg__vecang(ux,uy, vx,vy);    // Delta angle

  //  if (vecrat(ux,uy,vx,vy) <= -1.0f) da = NSVG_PI;
  //  if (vecrat(ux,uy,vx,vy) >= 1.0f) da = 0;

  if (fs == 0 && da > 0)
    da -= 2 * NSVG_PI;
  else if (fs == 1 && da < 0)
    da += 2 * NSVG_PI;

  // Approximate the arc using cubic spline segments.
  t[0] = cosrx; t[1] = sinrx;
  t[2] = -sinrx; t[3] = cosrx;
  t[4] = cx; t[5] = cy;

  // Split arc into max 90 degree segments.
  // The loop assumes an iteration per end point (including start and end), this +1.
  ndivs = (int)(fabsf(da) / (NSVG_PI * 0.5f) + 1.0f);
  if (ndivs > 6) ndivs = 6;
  hda = (da / (float)ndivs) * 0.5f;
  if ((hda < 1e-3f) && (hda > -1e-3f)) hda *= 0.5f;
  else hda = (1.0f - cosf(hda)) / sinf(hda);
  kappa = fabsf(4.0f / 3.0f * hda);
  if (da < 0.0f)
    kappa = -kappa;

  for (i = 0; i <= ndivs; i++) {
    a = a1 + da * ((float)i/(float)ndivs);
    dx = cosf(a);
    dy = sinf(a);
    nsvg__xformPoint(&x, &y, dx*rx, dy*ry, t); // position
    nsvg__xformVec(&tanx, &tany, -dy*rx * kappa, dx*ry * kappa, t); // tangent
    if (i > 0)
      nsvg__cubicBezTo(p, px+ptanx,py+ptany, x-tanx, y-tany, x, y);
    px = x;
    py = y;
    ptanx = tanx;
    ptany = tany;
  }

  *cpx = x2;
  *cpy = y2;
}

static void nsvg__parsePath(NSVGparser* p, char** attr)
{
  const char* s = NULL;
  char cmd = '\0';
  float args[30];
  int nargs;
  int rargs = 0;
  char initPoint;
  float cpx, cpy, cpx2, cpy2;
  //  const char* tmp[4];
  char closedFlag;
  int i;
  char item[kMaxIDLength];

  for (i = 0; attr[i]; i += 2) {
    if (strcmp(attr[i], "d") == 0) {
      s = attr[i + 1];
    } else {
      nsvg__parseAttr(p, attr[i], attr[i + 1]);
    }
  }

  if (s) {
    nsvg__resetPath(p);
    cpx = 0; cpy = 0;
    cpx2 = 0; cpy2 = 0;
    initPoint = 0;
    closedFlag = 0;
    nargs = 0;
    while (*s) {
      item[0] = '\0';
      if ((cmd == 'A' || cmd == 'a') && (nargs == 3 || nargs == 4))
        s = nsvg__getNextPathItemWhenArcFlag(s, item);
      if (!*item)
        s = nsvg__getNextPathItem(s, item);

      if (!*item) break;
      if (cmd != '\0' && nsvg__isCoordinate(item)) {
        if (nargs < 30) {
          float x = (float)nsvg__atof(item);
          args[nargs++] = x;
        }
        if (nargs >= rargs) {
          switch (cmd) {
            case 'm':
            case 'M':
              nsvg__pathMoveTo(p, &cpx, &cpy, args, cmd == 'm' ? 1 : 0);
              // Moveto can be followed by multiple coordinate pairs,
              // which should be treated as linetos.
              cmd = (cmd == 'm') ? 'l' : 'L';
              rargs = nsvg__getArgsPerElement(cmd);
              cpx2 = cpx; cpy2 = cpy;
              initPoint = 1;
              break;
            case 'l':
            case 'L':
              nsvg__pathLineTo(p, &cpx, &cpy, args, cmd == 'l' ? 1 : 0);
              cpx2 = cpx; cpy2 = cpy;
              break;
            case 'H':
            case 'h':
              nsvg__pathHLineTo(p, &cpx, &cpy, args, cmd == 'h' ? 1 : 0);
              cpx2 = cpx; cpy2 = cpy;
              break;
            case 'V':
            case 'v':
              nsvg__pathVLineTo(p, &cpx, &cpy, args, cmd == 'v' ? 1 : 0);
              cpx2 = cpx; cpy2 = cpy;
              break;
            case 'C':
            case 'c':
              nsvg__pathCubicBezTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 'c' ? 1 : 0);
              break;
            case 'S':
            case 's':
              nsvg__pathCubicBezShortTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 's' ? 1 : 0);
              break;
            case 'Q':
            case 'q':
              nsvg__pathQuadBezTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 'q' ? 1 : 0);
              break;
            case 'T':
            case 't':
              nsvg__pathQuadBezShortTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 't' ? 1 : 0);
              break;
            case 'A':
            case 'a':
              nsvg__pathArcTo(p, &cpx, &cpy, args, cmd == 'a' ? 1 : 0);
              cpx2 = cpx; cpy2 = cpy;
              break;
            default:
              if (nargs >= 2) {
                cpx = args[nargs-2];
                cpy = args[nargs-1];
                cpx2 = cpx; cpy2 = cpy;
              }
              break;
          }
          nargs = 0;
        }
      } else {

        cmd = item[0];
//        rargs = nsvg__getArgsPerElement(cmd);
        if (cmd == 'M' || cmd == 'm') {
          if (p->npts > 0)
            nsvg__addPath(p, closedFlag, "nsvg__parsePath");
          // Start new subpath.
          nsvg__resetPath(p);
          closedFlag = 0;
          nargs = 0;
        } else if (initPoint == 0) {
          // Do not allow other commands until initial point has been set (moveTo called once).
          cmd = '\0';
        }
        if (cmd == 'Z' || cmd == 'z') {
          closedFlag = 1;
          // Commit path.
          //    DBG("commit path, npts=%d\n", p->npts);
          if (p->npts > 0) {
            // Move current point to first point
            cpx = p->pts[0];
            cpy = p->pts[1];
            cpx2 = cpx; cpy2 = cpy;
            nsvg__addPath(p, closedFlag, "nsvg__parsePath");
          }
          // Start new subpath.
          nsvg__resetPath(p);
          nsvg__moveTo(p, cpx, cpy);
          closedFlag = 0;
          nargs = 0;
        }
        rargs = nsvg__getArgsPerElement(cmd);
        if (rargs == -1) {
          // Command not recognized
          cmd = '\0';
          rargs = 0;
        }
      }
    }
    // Commit path.
    if (p->npts)
      nsvg__addPath(p, closedFlag, "nsvg__parsePath");
  }
}

static void nsvg__parseRect(NSVGparser* p, char** attr)
{
  float x = 0.0f;
  float y = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
  float rx = -1.0f; // marks not set
  float ry = -1.0f;
  int i;

  for (i = 0; attr[i]; i += 2) {
    if (strcmp(attr[i], "x") == 0) x = nsvg__parseCoordinate(p, attr[i+1], nsvg__actualOrigX(p), nsvg__actualWidth(p));
    else if (strcmp(attr[i], "y") == 0) y = nsvg__parseCoordinate(p, attr[i+1], nsvg__actualOrigY(p), nsvg__actualHeight(p));
    else if (strcmp(attr[i], "width") == 0) w = nsvg__parseCoordinate(p, attr[i+1], 0.0f, nsvg__actualWidth(p));
    else if (strcmp(attr[i], "height") == 0) h = nsvg__parseCoordinate(p, attr[i+1], 0.0f, nsvg__actualHeight(p));
    else if (strcmp(attr[i], "rx") == 0) rx = fabsf(nsvg__parseCoordinate(p, attr[i+1], 0.0f, nsvg__actualWidth(p)));
    else if (strcmp(attr[i], "ry") == 0) ry = fabsf(nsvg__parseCoordinate(p, attr[i+1], 0.0f, nsvg__actualHeight(p)));
    else nsvg__parseAttr(p, attr[i], attr[i + 1]);
  }

  if (rx < 0.0f && ry > 0.0f) rx = ry;
  if (ry < 0.0f && rx > 0.0f) ry = rx;
  if (rx < 0.0f) rx = 0.0f;
  if (ry < 0.0f) ry = 0.0f;
  if (rx > w/2.0f) rx = w/2.0f;
  if (ry > h/2.0f) ry = h/2.0f;

  if (w != 0.0f && h != 0.0f) {
    nsvg__resetPath(p);

    if (rx < 0.00001f || ry < 0.0001f) {
      nsvg__moveTo(p, x, y);
      nsvg__lineTo(p, x+w, y);
      nsvg__lineTo(p, x+w, y+h);
      nsvg__lineTo(p, x, y+h);
    } else {
      // Rounded rectangle
      nsvg__moveTo(p, x+rx, y);
      nsvg__lineTo(p, x+w-rx, y);
      nsvg__cubicBezTo(p, x+w-rx*(1-NSVG_KAPPA90), y, x+w, y+ry*(1-NSVG_KAPPA90), x+w, y+ry);
      nsvg__lineTo(p, x+w, y+h-ry);
      nsvg__cubicBezTo(p, x+w, y+h-ry*(1-NSVG_KAPPA90), x+w-rx*(1-NSVG_KAPPA90), y+h, x+w-rx, y+h);
      nsvg__lineTo(p, x+rx, y+h);
      nsvg__cubicBezTo(p, x+rx*(1-NSVG_KAPPA90), y+h, x, y+h-ry*(1-NSVG_KAPPA90), x, y+h-ry);
      nsvg__lineTo(p, x, y+ry);
      nsvg__cubicBezTo(p, x, y+ry*(1-NSVG_KAPPA90), x+rx*(1-NSVG_KAPPA90), y, x+rx, y);
    }
    nsvg__addPath(p, 1, "parseRect");
    nsvg__addShape(p);
  }
}

static void nsvg__parseUse(NSVGparser* p, char** dict)
{
  NSVGattrib* attr = nsvg__getAttr(p);
  NSVGshape* shape = NULL;
  NSVGshape* ref = NULL;
  NSVGsymbol* refSym = NULL;
  int i;

  float x = 0.0f;
  float y = 0.0f;
  float xform[6];

  for (i = 0; dict[i]; i += 2) {
    if (strcmp(dict[i], "x") == 0) {
      x = nsvg__parseCoordinate(p, dict[i+1], nsvg__actualOrigX(p), nsvg__actualWidth(p));
    } else if (strcmp(dict[i], "y") == 0) {
      y = nsvg__parseCoordinate(p, dict[i+1], nsvg__actualOrigY(p), nsvg__actualHeight(p));
    } else if (strcmp(dict[i], "xlink:href") == 0) {
      const char *href = dict[i+1];
      refSym = p->symbols;
      while (refSym) {
        if (strcmp(refSym->id, href+1) == 0)
          break;
        refSym = refSym->next;
      }
      if (!refSym) {
        ref = p->image->shapes;
        while (ref) {
          if (strcmp(ref->id, href+1) == 0)
            break;
          ref = ref->next;
        }
      }
      if (!ref && !refSym) {
        return; // use without ref
      }
    } else {
      nsvg__parseAttr(p, dict[i], dict[i + 1]);
    }
  }

  if (refSym) {
    x -= refSym->viewBox[0];
    y -= refSym->viewBox[1];
  } else if (ref) {
    x -= ref->bounds[0];
    y -= ref->bounds[1];
  }

  nsvg__xformSetTranslation(&xform[0], x, y);
  nsvg__xformMultiply(&xform[0], attr->xform); //translate before rotate
//  nsvg__dumpFloat("use xform", xform, 6);

  if (ref) {
    shape = (NSVGshape*)nsvg__alloccopy(sizeof(NSVGshape), ref, "nsvg__parseUse shape"_XS8);
    if (!shape) return;
    memcpy(shape->xform, &xform[0], sizeof(float)*6);
    shape->isSymbol = false;
    shape->link = ref;
    shape->group = attr->group;
    AsciiStrCatS(shape->id, 64, "_lnk");
    shape->bounds[0] = FLT_MAX;
    shape->bounds[1] = FLT_MAX;
    shape->bounds[2] = -FLT_MAX;
    shape->bounds[3] = -FLT_MAX;
    nsvg__takeXformBounds(ref, &xform[0], shape->bounds);
//    nsvg__dumpFloat("used shape has bounds", shape->bounds, 4);
  } else if (refSym) {
    shape = (NSVGshape*)nsvg__alloczero(sizeof(NSVGshape), "nsvg__parseUse shape2"_XS8);
    if (!shape) return;
    memcpy(shape->xform, xform, sizeof(float)*6);
//    nsvg__xformMultiply(shape->xform, &xform[0]);
    shape->isSymbol = true;
    shape->link = refSym->shapes;
    shape->group = attr->group;
    AsciiStrCpyS(shape->id, 64, attr->id);
    shape->bounds[0] = FLT_MAX;
    shape->bounds[1] = FLT_MAX;
    shape->bounds[2] = -FLT_MAX;
    shape->bounds[3] = -FLT_MAX;
    NSVGshape* shapeInt = refSym->shapes;
    float xform2[6];
    while (shapeInt) {
      memcpy(&xform2[0], shape->xform, sizeof(float)*6);
      nsvg__xformPremultiply(&xform2[0], shapeInt->xform);
      nsvg__takeXformBounds(shapeInt, &xform2[0], shape->bounds);
      shapeInt = shapeInt->next;
    }
//    nsvg__dumpFloat("used symbol has bounds", shape->bounds, 4);
  }

  /* //there can't be own gradient
   //  DBG("paint type=%d\n", shape->fill.type);
   if (shape->fill.type == NSVG_PAINT_GRADIENT_LINK) {
   shape->fill.gradientLink = nsvg__createGradientLink(ref->fill.gradientLink->id);
   }

   if (shape->stroke.type == NSVG_PAINT_GRADIENT_LINK) {
   shape->stroke.gradientLink = nsvg__createGradientLink(ref->stroke.gradientLink->id);
   }
   */

  shape->next = NULL;
  shape->flags = NSVG_VIS_DISPLAY | NSVG_VIS_VISIBLE; //use always visible

  // Add to tail
  if (p->image->shapes == NULL)
    p->image->shapes = shape;
  else
    p->shapesTail->next = shape;
  p->shapesTail = shape;

}


static void nsvg__parseTextSpan(NSVGparser* p, char** dict)
{
  NSVGattrib* attr = nsvg__getAttr(p);
  NSVGtext* text = p->text;
  float x = 0.f, y = 0.f, r = 0.f;
  int i;
//    DBG("parse textSpan\n");
  //there should be text->next with own attribs
  for (i = 0; dict[i]; i += 2) {
    if (strcmp(dict[i], "x") == 0) {
      x = nsvg__parseCoordinate(p, dict[i+1], nsvg__actualOrigX(p), nsvg__actualWidth(p));
      text->x = x;
//            DBG("span posX=%f\n", x);
    } else if (strcmp(dict[i], "y") == 0) {
      y = nsvg__parseCoordinate(p, dict[i+1], nsvg__actualOrigY(p), nsvg__actualHeight(p));
      text->y = y;
//            DBG("span posY=%f\n", y);
    } else if (strcmp(dict[i], "font-size") == 0)  {
      r = nsvg__parseCoordinate(p, dict[i+1], 0.0f, nsvg__actualHeight(p));
      text->fontSize = r;
//            DBG("span fontSize=%f from=%s\n", r, dict[i+1]);
    } else if (strcmp(dict[i], "font-style") == 0)  {
//      DBG("span: attr=%s value=%s\n", dict[i], dict[i+1]);
      if (strstr(dict[i+1], "italic") != NULL)  {
        text->fontStyle = 'i';
      } else if (strstr(dict[i+1], "bold") != NULL)  {
        text->fontStyle = 'b';
      } else if (strstr(dict[i + 1], "light") != NULL) {
        text->fontStyle = 'l';
      } else {
        text->fontStyle = 'n';
      }
    } else {
      nsvg__parseAttr(p, dict[i], dict[i + 1]);
    }
  }
  if (attr->fontFace) {
    text->fontFace = attr->fontFace;
  }
  if (attr->hasFill == 1) {
    text->fontColor = attr->fillColor | ((int)(attr->fillOpacity * 255.f) << 24);
  }
  if (attr->hasStroke == 1) {
    text->strokeColor = attr->strokeColor | ((int)(attr->strokeOpacity * 255.f) << 24);
    text->strokeWidth = attr->strokeWidth;
  }
  if (text->fontStyle < 0x30) {
    text->fontStyle = 'n';
  }
}

//static int once = 0;
//static int once2 = 0;

static void nsvg__parseText(NSVGparser* p, char** dict)
{
  float x = 0.0f;
  float y = 0.0f;
  //float r = 0.0f;
  //  float xform[6];

  NSVGattrib* attr = nsvg__getAttr(p);

  int i;
//    DBG("text found\n");
  NSVGtext* text = (NSVGtext*)nsvg__alloczero(sizeof(NSVGtext), "nsvg__parseText"_XS8);
  if (!text) {
    return;
  }
  text->group = attr->group;

  for (i = 0; dict[i]; i += 2) {
    if (strcmp(dict[i], "x") == 0) {
      x = nsvg__parseCoordinate(p, dict[i+1], nsvg__actualOrigX(p), nsvg__actualWidth(p));
    } else if (strcmp(dict[i], "y") == 0) {
      y = nsvg__parseCoordinate(p, dict[i+1], nsvg__actualOrigY(p), nsvg__actualHeight(p));
    } else {
//      DBG("%d: attr=%s value=%s\n", i, dict[i], dict[i+1]);
      nsvg__parseAttr(p, dict[i], dict[i + 1]);
    }
  }
//  DBG("text: x=%f y=%f attr:Style=%hhX, size=%f, id=%s\n", x, y, attr->fontFace->fontStyle, attr->fontFace->fontSize, attr->id);
  text->x = x;
  text->y = y;
  text->fontSize = attr->fontFace->fontSize;
  text->fontStyle = attr->fontFace->fontStyle;
  memcpy(text->id, attr->id, kMaxIDLength);
  text->fontFace = attr->fontFace;
  if (attr->hasFill == 1) {
    text->fontColor = attr->fillColor | ((int)(attr->fillOpacity * 255.f) << 24);
  }
  if (attr->hasStroke == 1) {
    text->strokeColor = attr->strokeColor | ((int)(attr->strokeOpacity * 255.f) << 24);
    text->strokeWidth = attr->strokeWidth;
  }

  memcpy(text->xform, attr->xform, 6*sizeof(float));
  if (text->fontStyle < 0x30) {
    text->fontStyle = 'n';
  }
//  DBG("required font %s  required style=%c\n", text->fontFace->fontFamily, text->fontStyle);
  //if the font is not registered then we have to load new one
  NSVGfont        *fontSVG = NULL;
  NSVGfontChain   *fontChain = p->fontsDB;
  NSVGfontChain   *fontChainSimilar = NULL;
  while (fontChain) {
    fontSVG = fontChain->font;
    if (fontSVG) {
 //     DBG("probe fontFamily=%s fontStyle=%c\n", fontSVG->fontFamily, fontSVG->fontStyle);
      if (strcmp(fontSVG->fontFamily, text->fontFace->fontFamily) == 0) {
        fontChainSimilar = fontChain;
 //       DBG("font %s found\n", fontSVG->fontFamily);
        if (fontSVG->fontStyle == text->fontStyle) {
          break;
        }
      }
    }
    fontChain = fontChain->next;
  }
  if (!fontChain && fontChainSimilar) { //font with this style is not found but we have same font with other style
//    DBG("found similar font with style=%c\n", fontChainSimilar->font->fontStyle);
    fontChain = fontChainSimilar;
    fontSVG = fontChain->font;
  }
  if (!fontChain) {  // font not found in the chain
    //then load it
    UINT8           *FileData = NULL;
    UINTN           FileDataLength = 0;
    NSVGparser      *p1 = NULL;
    EFI_STATUS      Status;
//    DBG("required font %s not found, try to load external\n", text->fontFace->fontFamily);
    XStringW FontFileName = XStringW().takeValueFrom(text->fontFace->fontFamily) + L".svg"_XSW;
//    DBG(" file name =%ls\n", FontFileName.wc_str());
    Status = egLoadFile(&ThemeX->getThemeDir(), FontFileName.wc_str(), &FileData, &FileDataLength);
//    DBG(" font %s loaded status=%lld, %s\n", text->fontFace->fontFamily, Status, efiStrError(Status));
    if (!EFI_ERROR(Status)) {
      p1 = nsvg__parse((CHAR8*)FileData, 72, 1.0f);  //later we will free parser p1
      if (!p1) {
 //       DBG("font %s not parsed\n", text->fontFace->fontFamily);
      } else {
        // Jief : this is only taking the first font from the file. It would not be hard to take the whole p1->fontsDB and to link it on p->fontsDB
        NSVGfontChain* fc = p1->fontsDB;
        p1->fontsDB = p1->fontsDB->next;
        fc->next = p->fontsDB;
        p->fontsDB = fc;

        fontSVG = p->fontsDB->font; //last added during parse file data
        text->font = fontSVG;
        nsvg__deleteParser(p1);
      }
      FreePool(FileData); //after load // don not use nsvg__delete because it's not allocated by nsvg__alloc...
      FileData = NULL;
    } else {
//      DBG("set embedded font\n");
      text->font = p->currentFont; //else embedded if present which is also double fontChain
    }
  } else {
//    DBG("set found font %s\n", fontSVG->fontFamily);
    text->font = fontSVG;  //the font found in fontChain
  }
  
  //instead of embedded
  if (fontSVG && fontSVG->glyphs) {
    NSVGgroup* group = attr->group;
    while (group) {
      if (strcmp(group->id, "MessageRow") == 0) {
        if (!p->textFace[1].valid) {
          //here we want to set text->font as p->font if text->groupID == MessageRow
          p->currentFont = fontSVG;
          p->fontSize = text->fontSize;
          p->fontColor = text->fontColor;
          p->textFace[1].font = fontSVG;
          p->textFace[1].size = (INTN)text->fontSize;
          p->textFace[1].color = text->fontColor;
          p->textFace[1].valid = true;
 //         DBG("set message->font=%s color=%X size=%f as in MessageRow\n", fontSVG->fontFamily, text->fontColor, text->fontSize);
        }
        break;
      } else if (!ThemeX->Daylight && strcmp(group->id, "MessageRow_night") == 0) {
          //replace ThemeX->Daylight settings
          p->currentFont = fontSVG;
          p->fontSize = text->fontSize;
          p->fontColor = text->fontColor;
          p->textFace[1].font = fontSVG;
          p->textFace[1].size = (INTN)text->fontSize;
          p->textFace[1].color = text->fontColor;
          p->textFace[1].valid = true;
  //        DBG("set message_night->font=%s color=%X size=%f as in MessageRow\n", fontSVG->fontFamily, text->fontColor, text->fontSize);
          break;
      } else if (strcmp(group->id, "MenuRows") == 0) {
        if (!p->textFace[2].valid) {
          p->textFace[2].font = fontSVG;
          p->textFace[2].size = (INTN)text->fontSize;
          p->textFace[2].color = text->fontColor;
          p->textFace[2].valid = true;
 //         DBG("set menu->font=%s color=%X size=%f as in MenuRows\n", fontSVG->fontFamily, text->fontColor, text->fontSize);
        }
        break;
      } else if (!ThemeX->Daylight && strcmp(group->id, "MenuRows_night") == 0) {
          p->textFace[2].font = fontSVG;
          p->textFace[2].size = (INTN)text->fontSize;
          p->textFace[2].color = text->fontColor;
          p->textFace[2].valid = true;
        break;
      } else if (strcmp(group->id, "HelpRows") == 0) {
        if (!p->textFace[0].valid) {
          p->textFace[0].font = fontSVG;
          p->textFace[0].size = (INTN)text->fontSize;
          p->textFace[0].color = text->fontColor;
          p->textFace[0].valid = true;
 //         DBG("set help->font=%s color=%X size=%f as in HelpRows\n", fontSVG->fontFamily, text->fontColor, text->fontSize);
        }
        break;
      } else if (!ThemeX->Daylight && strstr(group->id, "HelpRows_night") != NULL) {
          p->textFace[0].font = fontSVG;
          p->textFace[0].size = (INTN)text->fontSize;
          p->textFace[0].color = text->fontColor;
          p->textFace[0].valid = true;
 //         DBG("set help_night->font=%s color=%X size=%f as in HelpRows\n", fontSVG->fontFamily, text->fontColor, text->fontSize);
          break;
      }
      group = group->parent;
    }
  }

  //add to head
  text->next = p->text;
  p->text = text;
  p->isText = true;
}

static void nsvg__parseCircle(NSVGparser* p, char** attr)
{
  float cx = 0.0f;
  float cy = 0.0f;
  float r = 0.0f;
  int i;

  for (i = 0; attr[i]; i += 2) {
    if (!nsvg__parseAttr(p, attr[i], attr[i + 1])) {
      if (strcmp(attr[i], "cx") == 0) cx = nsvg__parseCoordinate(p, attr[i+1], nsvg__actualOrigX(p), nsvg__actualWidth(p));
      else if (strcmp(attr[i], "cy") == 0) cy = nsvg__parseCoordinate(p, attr[i+1], nsvg__actualOrigY(p), nsvg__actualHeight(p));
      else if (strcmp(attr[i], "r") == 0) r = fabsf(nsvg__parseCoordinate(p, attr[i+1], 0.0f, nsvg__actualLength(p)));
    }
  }

  if (r > 0.0f) {
    nsvg__resetPath(p);
    nsvg__moveTo(p, cx+r, cy);
    nsvg__cubicBezTo(p, cx+r, cy+r*NSVG_KAPPA90, cx+r*NSVG_KAPPA90, cy+r, cx, cy+r);
    nsvg__cubicBezTo(p, cx-r*NSVG_KAPPA90, cy+r, cx-r, cy+r*NSVG_KAPPA90, cx-r, cy);
    nsvg__cubicBezTo(p, cx-r, cy-r*NSVG_KAPPA90, cx-r*NSVG_KAPPA90, cy-r, cx, cy-r);
    nsvg__cubicBezTo(p, cx+r*NSVG_KAPPA90, cy-r, cx+r, cy-r*NSVG_KAPPA90, cx+r, cy);
    nsvg__addPath(p, 1, "parseCircle");
    nsvg__addShape(p);
  }
}

static void nsvg__parseEllipse(NSVGparser* p, char** attr)
{
  float cx = 0.0f;
  float cy = 0.0f;
  float rx = 0.0f;
  float ry = 0.0f;
  int i;

  for (i = 0; attr[i]; i += 2) {
    if (!nsvg__parseAttr(p, attr[i], attr[i + 1])) {
      if (strcmp(attr[i], "cx") == 0) cx = nsvg__parseCoordinate(p, attr[i+1], nsvg__actualOrigX(p), nsvg__actualWidth(p));
      else if (strcmp(attr[i], "cy") == 0) cy = nsvg__parseCoordinate(p, attr[i+1], nsvg__actualOrigY(p), nsvg__actualHeight(p));
      else if (strcmp(attr[i], "rx") == 0) rx = fabsf(nsvg__parseCoordinate(p, attr[i+1], 0.0f, nsvg__actualWidth(p)));
      else if (strcmp(attr[i], "ry") == 0) ry = fabsf(nsvg__parseCoordinate(p, attr[i+1], 0.0f, nsvg__actualHeight(p)));
    }
  }

  if (rx > 0.0f && ry > 0.0f) {
    nsvg__resetPath(p);
    nsvg__moveTo(p, cx+rx, cy);
    nsvg__cubicBezTo(p, cx+rx, cy+ry*NSVG_KAPPA90, cx+rx*NSVG_KAPPA90, cy+ry, cx, cy+ry);
    nsvg__cubicBezTo(p, cx-rx*NSVG_KAPPA90, cy+ry, cx-rx, cy+ry*NSVG_KAPPA90, cx-rx, cy);
    nsvg__cubicBezTo(p, cx-rx, cy-ry*NSVG_KAPPA90, cx-rx*NSVG_KAPPA90, cy-ry, cx, cy-ry);
    nsvg__cubicBezTo(p, cx+rx*NSVG_KAPPA90, cy-ry, cx+rx, cy-ry*NSVG_KAPPA90, cx+rx, cy);
    nsvg__addPath(p, 1, "nsvg__parseEllipse");
    nsvg__addShape(p);
  }
}

static void nsvg__parseLine(NSVGparser* p, char** attr)
{
  float x1 = 0.0;
  float y1 = 0.0;
  float x2 = 0.0;
  float y2 = 0.0;
  int i;

  for (i = 0; attr[i]; i += 2) {
    if (!nsvg__parseAttr(p, attr[i], attr[i + 1])) {
      if (strcmp(attr[i], "x1") == 0) x1 = nsvg__parseCoordinate(p, attr[i + 1], nsvg__actualOrigX(p), nsvg__actualWidth(p));
      else if (strcmp(attr[i], "y1") == 0) y1 = nsvg__parseCoordinate(p, attr[i + 1], nsvg__actualOrigY(p), nsvg__actualHeight(p));
      else if (strcmp(attr[i], "x2") == 0) x2 = nsvg__parseCoordinate(p, attr[i + 1], nsvg__actualOrigX(p), nsvg__actualWidth(p));
      else if (strcmp(attr[i], "y2") == 0) y2 = nsvg__parseCoordinate(p, attr[i + 1], nsvg__actualOrigY(p), nsvg__actualHeight(p));
    }
  }
  nsvg__resetPath(p);
  nsvg__moveTo(p, x1, y1);
  nsvg__lineTo(p, x2, y2);
  nsvg__addPath(p, 0, "nsvg__parseLine");
  nsvg__addShape(p);
}

static void nsvg__parsePoly(NSVGparser* p, char** attr, int closeFlag)
{
  int i;
  const char* s;
  float args[2];
  int nargs, npts = 0;
  char item[64];

  nsvg__resetPath(p);

  for (i = 0; attr[i]; i += 2) {
    if (!nsvg__parseAttr(p, attr[i], attr[i + 1])) {
      if (strcmp(attr[i], "points") == 0) {
        s = attr[i + 1];
        nargs = 0;
        while (*s) {
          s = nsvg__getNextPathItem(s, item);
          args[nargs++] = (float)nsvg__atof(item);
          if (nargs >= 2) {
            if (npts == 0)
              nsvg__moveTo(p, args[0], args[1]);
            else
              nsvg__lineTo(p, args[0], args[1]);
            nargs = 0;
            npts++;
          }
        }
      }
    }
  }

  nsvg__addPath(p, (char)closeFlag, "nsvg__parsePoly");

  nsvg__addShape(p);
}

/* Slice - I dont know what it should be
 static void nsvg__parseIMAGE(NSVGparser* p, const char** attr)
 {
 float x = 0.0f;
 float y = 0.0f;
 float w = 0.0f;
 float h = 0.0f;
 int i;
 const char *href = NULL;

 for (i = 0; attr[i]; i += 2) {
 if (!nsvg__parseAttr(p, attr[i], attr[i + 1])) {
 if (strcmp(attr[i], "x") == 0) {
 x = nsvg__parseCoordinate(p, attr[i+1], nsvg__actualOrigX(p), nsvg__actualWidth(p));
 } else
 if (strcmp(attr[i], "y") == 0) {
 y = nsvg__parseCoordinate(p, attr[i+1], nsvg__actualOrigY(p), nsvg__actualHeight(p));
 } else
 if (strcmp(attr[i], "width") == 0) {
 w = nsvg__parseCoordinate(p, attr[i+1], 0.0f, nsvg__actualWidth(p));
 } else
 if (strcmp(attr[i], "height") == 0) {
 h = nsvg__parseCoordinate(p, attr[i+1], 0.0f, nsvg__actualHeight(p));
 } else
 if (strcmp(attr[i], "xlink:href") == 0) {
 href = attr[i+1];
 }
 }
 }

 if (w != 0.0f && h != 0.0f) {
 nsvg__resetPath(p);

 NSVGattrib* attr = nsvg__getAttr(p);
 float scale = 1.0f;
 NSVGshape *shape, *cur, *prev;

 if (href == NULL)
 return;

 shape = (NSVGshape*)nsvg__alloczero(sizeof(NSVGshape));
 if (shape == NULL) return;

 memcpy(shape->id, attr->id, sizeof shape->id);
 memcpy(shape->title, attr->title, sizeof shape->title);
 //    DBG("shapeID=%s\n", shape->id);
 shape->group = attr->group;
 scale = nsvg__getAverageScale(attr->xform);
 shape->opacity = attr->opacity;
 shape->image_href = href;
 p->plist = NULL;

 shape->bounds[0] = x;
 shape->bounds[1] = y;
 shape->bounds[2] = x+w;
 shape->bounds[3] = y+h;

 nsvg__xformIdentity(shape->xform);

 // Set flags
 shape->flags = (attr->visible ? NSVG_VIS_DISPLAY | NSVG_VIS_VISIBLE : 0x00);

 // Add to tail
 prev = NULL;
 cur = p->image->shapes;
 while (cur != NULL) {
 prev = cur;
 cur = cur->next;
 }
 if (prev == NULL)
 p->image->shapes = shape;
 else
 prev->next = shape;

 return;
 }
 }
 */

//parse embedded PNG image
static void nsvg__parseEmbeddedPNG(NSVGparser* p, char** dict)
{
  //  NSVGattrib* attr = nsvg__getAttr(p);
  NSVGpattern *pt = NULL;
  int i;
  UINTN len = 0;
//  float w,h;
  const char *href = NULL;
  UINT8 *tmpData = NULL;
//  EG_IMAGE *NewImage = NULL;
  XImage *NewImage = new XImage;

  for (i = 0; dict[i]; i += 2) {
    if (strcmp(dict[i], "width") == 0) {
      /*w =*/ nsvg__parseCoordinate(p, dict[i+1], 0.0f, nsvg__actualWidth(p));
    } else if (strcmp(dict[i], "height") == 0) {
      /*h =*/ nsvg__parseCoordinate(p, dict[i+1], 0.0f, nsvg__actualHeight(p));
    } else if (strcmp(dict[i], "xlink:href") == 0) {
      href = dict[i+1];
    } else {
      nsvg__parseAttr(p, dict[i], dict[i + 1]);
    }
  }
  if (!href || (strstr(href, "data:image/png;") == NULL)) {
    return;
  }
  href = strstr(href, "base64,") + 7;
  if (p->patternFlag) {
    pt = p->patterns; //the last one
  }
  tmpData = (UINT8 *)Base64DecodeClover((char*)href, &len);
  if (len == 0) {
    DBG("image not decoded from base64\n");
  }
//  NewImage = egDecodePNG(tmpData, len, true);
  NewImage->FromPNG(tmpData, len);
  pt->image = (void *)NewImage;
  if (tmpData) {
    nsvg__delete(tmpData, "parseImage tmpData"_XS8);
  }
}

static void nsvg__parsePattern(NSVGparser* p, char** dict)
{
  NSVGattrib* attr = nsvg__getAttr(p);
  int i;
  float w = 0.f, h=0.f;
  NSVGpattern *pt;

  for (i = 0; dict[i]; i += 2) {
    if (strcmp(dict[i], "width") == 0) {
      w = nsvg__parseCoordinate(p, dict[i+1], 0.0f, nsvg__actualWidth(p));
    } else if (strcmp(dict[i], "height") == 0) {
      h = nsvg__parseCoordinate(p, dict[i+1], 0.0f, nsvg__actualHeight(p));
    } else {
      nsvg__parseAttr(p, dict[i], dict[i + 1]);
    }
  }

  pt = (decltype(pt))nsvg__alloczero(sizeof(NSVGpattern), "parsePattern"_XS8);
  AsciiStrCpyS(pt->id, 64, attr->id);
  pt->width = w;
  pt->height = h;
  pt->next = p->patterns;
  p->patterns = pt;
}

static void nsvg__parseSVG(NSVGparser* p, char** attr)
{
  int i;
  for (i = 0; attr[i]; i += 2) {
    if (!nsvg__parseAttr(p, attr[i], attr[i + 1])) {
      if (strcmp(attr[i], "width") == 0) {
        p->image->width = nsvg__parseCoordinate(p, attr[i + 1], 0.0f, 0.0f);
      } else if (strcmp(attr[i], "height") == 0) {
        p->image->height = nsvg__parseCoordinate(p, attr[i + 1], 0.0f, 0.0f);
      } else if (strcmp(attr[i], "viewBox") == 0) {
        char* Next = 0;
        AsciiStrToFloat(attr[i + 1], &Next, &p->viewMinx);
        AsciiStrToFloat((const char*)Next, &Next, &p->viewMiny);
        AsciiStrToFloat((const char*)Next, &Next, &p->viewWidth);
        AsciiStrToFloat((const char*)Next, &Next, &p->viewHeight);
      } else if (strcmp(attr[i], "preserveAspectRatio") == 0) {
        if (strstr(attr[i + 1], "none") != 0) {
          // No uniform scaling
          p->alignType = NSVG_ALIGN_NONE;
        } else {
          // Parse X align
          if (strstr(attr[i + 1], "xMin") != 0)
            p->alignX = NSVG_ALIGN_MIN;
          else if (strstr(attr[i + 1], "xMid") != 0)
            p->alignX = NSVG_ALIGN_MID;
          else if (strstr(attr[i + 1], "xMax") != 0)
            p->alignX = NSVG_ALIGN_MAX;
          // Parse X align
          if (strstr(attr[i + 1], "yMin") != 0)
            p->alignY = NSVG_ALIGN_MIN;
          else if (strstr(attr[i + 1], "yMid") != 0)
            p->alignY = NSVG_ALIGN_MID;
          else if (strstr(attr[i + 1], "yMax") != 0)
            p->alignY = NSVG_ALIGN_MAX;
          // Parse meet/slice
          p->alignType = NSVG_ALIGN_MEET;
          if (strstr(attr[i + 1], "slice") != 0)
            p->alignType = NSVG_ALIGN_SLICE;
        }
      }
    }
  }
}

static void nsvg__parseGradient(NSVGparser* p, char** attr, char type)
{
  int i;
  NSVGgradientData* grad = (NSVGgradientData*)nsvg__alloczero(sizeof(NSVGgradientData), "nsvg__parseGradient"_XS8);
  if (grad == NULL) return;
  //defaults
  grad->units = NSVG_USER_SPACE; //NSVG_OBJECT_SPACE;
  grad->type = type;
  grad->ditherCoarse = 0; //default value
  if (grad->type == NSVG_PAINT_LINEAR_GRADIENT) {
    grad->direction.linear.x1 = nsvg__coord(0.0f, NSVG_UNITS_PERCENT);
    grad->direction.linear.y1 = nsvg__coord(0.0f, NSVG_UNITS_PERCENT);
    grad->direction.linear.x2 = nsvg__coord(100.0f, NSVG_UNITS_PERCENT);
    grad->direction.linear.y2 = nsvg__coord(100.0f, NSVG_UNITS_PERCENT);
  } else if (grad->type == NSVG_PAINT_RADIAL_GRADIENT) {
    grad->direction.radial.cx = nsvg__coord(50.0f, NSVG_UNITS_PERCENT);
    grad->direction.radial.cy = nsvg__coord(50.0f, NSVG_UNITS_PERCENT);
    grad->direction.radial.r = nsvg__coord(50.0f, NSVG_UNITS_PERCENT);
  } else if (grad->type == NSVG_PAINT_CONIC_GRADIENT) {
    grad->direction.radial.cx = nsvg__coord(50.0f, NSVG_UNITS_PERCENT);
    grad->direction.radial.cy = nsvg__coord(50.0f, NSVG_UNITS_PERCENT);
    grad->direction.radial.r = nsvg__coord(50.0f, NSVG_UNITS_PERCENT);
  }

  nsvg__xformIdentity(grad->xform);

  for (i = 0; attr[i]; i += 2) {
    if (strcmp(attr[i], "xml:id") == 0) {
      //      DBG("xml:id ?\n");
      strncpy(grad->id, attr[i+1], 63);
      grad->id[63] = '\0';
    } else if (strcmp(attr[i], "id") == 0) {
      strncpy(grad->id, attr[i+1], 63);
      grad->id[63] = '\0';
    } else if (!nsvg__parseAttr(p, attr[i], attr[i + 1])) {
      if (strcmp(attr[i], "gradientUnits") == 0) {
        if (strcmp(attr[i+1], "objectBoundingBox") == 0)
          grad->units = NSVG_OBJECT_SPACE;
        else
          grad->units = NSVG_USER_SPACE;
      } else if (strcmp(attr[i], "gradientTransform") == 0) {
        nsvg__parseTransform(grad->xform, attr[i + 1]);
      } else if (strcmp(attr[i], "cx") == 0) {
        grad->direction.radial.cx = nsvg__parseCoordinateRaw(attr[i + 1]);
      } else if (strcmp(attr[i], "cy") == 0) {
        grad->direction.radial.cy = nsvg__parseCoordinateRaw(attr[i + 1]);
      } else if (strcmp(attr[i], "r") == 0) {
        grad->direction.radial.r = nsvg__parseCoordinateRaw(attr[i + 1]);
      } else if (strcmp(attr[i], "fx") == 0) {
        grad->direction.radial.fx = nsvg__parseCoordinateRaw(attr[i + 1]);
      } else if (strcmp(attr[i], "fy") == 0) {
        grad->direction.radial.fy = nsvg__parseCoordinateRaw(attr[i + 1]);
      } else if (strcmp(attr[i], "x1") == 0) {
        grad->direction.linear.x1 = nsvg__parseCoordinateRaw(attr[i + 1]);
      } else if (strcmp(attr[i], "y1") == 0) {
        grad->direction.linear.y1 = nsvg__parseCoordinateRaw(attr[i + 1]);
      } else if (strcmp(attr[i], "x2") == 0) {
        grad->direction.linear.x2 = nsvg__parseCoordinateRaw(attr[i + 1]);
      } else if (strcmp(attr[i], "y2") == 0) {
        grad->direction.linear.y2 = nsvg__parseCoordinateRaw(attr[i + 1]);
      } else if (strcmp(attr[i], "clover:ditherCoarse") == 0) {
        grad->ditherCoarse = nsvg__getIntegerDict(attr[i + 1]);
      } else if (strcmp(attr[i], "clover:conic") == 0) {
        int conic = nsvg__getIntegerDict(attr[i + 1]);
        if (conic > 0) {
          grad->type = NSVG_PAINT_CONIC_GRADIENT;
        }
      } else if (strcmp(attr[i], "spreadMethod") == 0) {
        if (strcmp(attr[i+1], "pad") == 0)
          grad->spread = NSVG_SPREAD_PAD;
        else if (strcmp(attr[i+1], "reflect") == 0)
          grad->spread = NSVG_SPREAD_REFLECT;
        else if (strcmp(attr[i+1], "repeat") == 0)
          grad->spread = NSVG_SPREAD_REPEAT;
      } else if (strcmp(attr[i], "xlink:href") == 0) {
        const char *href = attr[i+1];
        strncpy(grad->ref, href+1, 62);
        grad->ref[62] = '\0';
      }
    }
  }

  grad->next = p->gradients;
  p->gradients = grad;
}

static void nsvg__parseGradientStop(NSVGparser* p, char** dict)
{
  NSVGattrib* curAttr = nsvg__getAttr(p);
  NSVGgradientData* grad;
  NSVGgradientStop* stop;
  int i, idx = 0, nsize;

  curAttr->stopOffset = 0.f;
  curAttr->stopColor = 0;
  curAttr->stopOpacity = 1.0f;

  for (i = 0; dict[i]; i += 2) {
    nsvg__parseAttr(p, dict[i], dict[i + 1]);
  }

  // Add stop to the last gradient.
  grad = p->gradients;
  if (grad == NULL) return;

  nsize = sizeof(NSVGgradientStop) * grad->nstops;
  if (nsize == 0) {
    grad->stops = (NSVGgradientStop*)nsvg__alloc(sizeof(NSVGgradientStop), "nsvg__parseGradientStop"_XS8);
    grad->nstops = 1;
  } else {
    grad->nstops++;
    grad->stops = (NSVGgradientStop*)nsvg__realloc(nsize, sizeof(NSVGgradientStop)*grad->nstops, grad->stops, "nsvg__parseGradientStop"_XS8);
  }
  if (grad->stops == NULL) return;

  // Insert
  idx = grad->nstops-1;
  for (i = 0; i < grad->nstops-1; i++) {
    if (curAttr->stopOffset < grad->stops[i].offset) {
      idx = i;
      break;
    }
  }
  if (idx != grad->nstops-1) {
    for (i = grad->nstops-1; i > idx; i--)
      memcpy(&grad->stops[i], &grad->stops[i-1], sizeof(NSVGgradientStop));
  }

  stop = &grad->stops[idx];
  stop->color = ((unsigned int)(curAttr->stopOpacity*255) << 24) | curAttr->stopColor;
  stop->offset = curAttr->stopOffset;
}

static void nsvg__parseSymbol(NSVGparser* p, char** dict)
{
  NSVGsymbol* symbol;
  NSVGattrib* curAttr = nsvg__getAttr(p);
  int i;
  symbol = (NSVGsymbol*)nsvg__alloczero(sizeof(NSVGsymbol), "nsvg__parseSymbol"_XS8);
  for (i = 0; dict[i]; i += 2) {
    if (strcmp(dict[i], "viewBox") == 0) {
      char* Next = 0;
      AsciiStrToFloat(dict[i + 1], &Next, &symbol->viewBox[0]);
      AsciiStrToFloat((const char*)Next, &Next, &symbol->viewBox[1]);
      AsciiStrToFloat((const char*)Next, &Next, &symbol->viewBox[2]);
      AsciiStrToFloat((const char*)Next, &Next, &symbol->viewBox[3]);
    } else nsvg__parseAttr(p, dict[i], dict[i + 1]);
  }
  AsciiStrCpyS(symbol->id, 64, curAttr->id);
//  memcpy(symbol->xform, curAttr->xform, 6*sizeof(float));
  symbol->next = p->symbols;
  p->symbols = symbol;
}

static void nsvg__parseGroup(NSVGparser* p, char** dict)
{
  NSVGattrib* oldAttr = nsvg__getAttr(p);
  nsvg__pushAttr(p);
  NSVGattrib* curAttr = nsvg__getAttr(p);
  int i;
  int visSet = 0;
  if (!curAttr) {
    return;
  }
  //  DBG("parse group\n");
  NSVGgroup* group = (NSVGgroup*)nsvg__alloczero(sizeof(NSVGgroup), "nsvg__parseGroup"_XS8);
  group->next = p->image->groups;
  p->image->groups = group;

  //  if (curAttr->id[0] == '\0') //skip anonymous groups
  //    return;
  for (i = 0; dict[i]; i += 2) {
    if (strcmp(dict[i], "visibility") == 0) {
      visSet = 1;
      if (strcmp(dict[i+1], "hidden") == 0) {
        group->visibility &= ~NSVG_VIS_VISIBLE;
      } else if (strcmp(dict[i+1], "visible") == 0) {
        group->visibility |= NSVG_VIS_VISIBLE;
      }
    } else nsvg__parseAttr(p, dict[i], dict[i + 1]);
  }
  AsciiStrCpyS(group->id, 64, curAttr->id);
  //  DBG("parsed groupID=%s\n", group->id);

  if (oldAttr != NULL) {
    group->parent = oldAttr->group;
  }
  curAttr->group = group;

  if (!visSet) {
    if (group->parent != NULL) {
      group->visibility = group->parent->visibility;
    } else {
      group->visibility = NSVG_VIS_VISIBLE;
    }
  }
}

//parse Clover settings for theme
// What's this doing here ???? XTheme method are supposed to be in XTheme.cpp.
void XTheme::parseTheme(void* parser, char** dict)
{
  NSVGparser* p = (NSVGparser*)parser;
  XBool found = false;
  UINT32 Color = 0x80808080; //default value
  for (int i = 0; dict[i]; i += 2) {
    if (strcmp(dict[i], "SelectionOnTop") == 0) {
      SelectionOnTop = nsvg__getIntegerDict(dict[i+1]) > 0;
    } else if (strcmp(dict[i], "BadgeOffsetX") == 0) {
      BadgeOffsetX = nsvg__getIntegerDict(dict[i + 1]);
    } else if (strcmp(dict[i], "BadgeOffsetY") == 0) {
      BadgeOffsetY = nsvg__getIntegerDict(dict[i + 1]);
    } else if (strcmp(dict[i], "LayoutBannerOffset") == 0) {
      LayoutBannerOffset = nsvg__getIntegerDict(dict[i + 1]);
    } else if (strcmp(dict[i], "LayoutButtonOffset") == 0) {
      LayoutButtonOffset = nsvg__getIntegerDict(dict[i + 1]);
      
    } else if (strcmp(dict[i], "NonSelectedGrey") == 0) {
      NonSelectedGrey = nsvg__getIntegerDict(dict[i + 1]) > 0;
    } else if (strcmp(dict[i], "CharWidth") == 0) {
      CharWidth = nsvg__getIntegerDict(dict[i + 1]);
    } else if (strcmp(dict[i], "BackgroundDark") == 0) {
      BackgroundDark = nsvg__getIntegerDict(dict[i + 1]) > 0;
    } else if (strcmp(dict[i], "BackgroundSharp") == 0) {
      BackgroundSharp = nsvg__getIntegerDict(dict[i + 1]);
    } else if (strcmp(dict[i], "BackgroundScale") == 0) {
      BackgroundScale = imNone;
      if (strstr(dict[i+1], "scale") != NULL)  {
        BackgroundScale = imScale;
      }
      if (strstr(dict[i+1], "crop") != NULL)  {
        BackgroundScale = imCrop;
      }
      if (strstr(dict[i+1], "tile") != NULL)  {
        BackgroundScale = imTile;
      }
    } else if (strcmp(dict[i], "Badges") == 0) {
      HideBadges = 0;
      if (strstr(dict[i+1], "show") != NULL)  {
        HideBadges |= HDBADGES_SHOW;
      }
      if (strstr(dict[i+1], "swap") != NULL)  {
        HideBadges |= HDBADGES_SWAP;
      }
      if (strstr(dict[i+1], "inline") != NULL)  {
        HideBadges |= HDBADGES_INLINE;
      }
    } else if (strcmp(dict[i], "BadgeScale") == 0) {
      BadgeScale = nsvg__getIntegerDict(dict[i + 1]);
    } else if (strcmp(dict[i], "SelectionColor") == 0) {
      Color = nsvg__getIntegerDict(dict[i + 1]);
      if (ThemeX->Daylight) {
        SelectionColor = Color;
      }
    } else if (strcmp(dict[i], "SelectionColor_night") == 0) {
      found = true;
      if (!ThemeX->Daylight) {
        SelectionColor = nsvg__getIntegerDict(dict[i + 1]);
      }
    } else if (strcmp(dict[i], "VerticalLayout") == 0) {
      VerticalLayout = nsvg__getIntegerDict(dict[i + 1]) > 0;
    } else if (strcmp(dict[i], "BootCampStyle") == 0) {
      BootCampStyle = nsvg__getIntegerDict(dict[i + 1]) > 0;
    } else if (strcmp(dict[i], "AnimeFrames") == 0) {
      NumFrames = nsvg__getIntegerDict(dict[i + 1]);
      if (NumFrames == 0xFFFF) {
        NumFrames = 0;
      }
    } else if (strcmp(dict[i], "FrameTime") == 0) {
      FrameTime = nsvg__getIntegerDict(dict[i + 1]);
    } else nsvg__parseAttr(p, dict[i], dict[i + 1]);
  }
  if (!found) {
    SelectionColor = Color;
  }
}


// parse embedded font
static void nsvg__parseFont(NSVGparser* p, char** dict)
{
  int i;
  NSVGfont* font;
  NSVGattrib* curAttr = nsvg__getAttr(p);
  if (!curAttr) {
    return;
  }

  font = (decltype(font))nsvg__alloczero(sizeof(*font), "nsvg__parseFont"_XS8);

  for (i = 0; dict[i]; i += 2) {
    if (strcmp(dict[i], "horiz-adv-x") == 0) {
      font->horizAdvX = (int)AsciiStrDecimalToUintn(dict[i+1]);
    } else if (strcmp(dict[i], "font-family") == 0) { //usually absent here
        AsciiStrCpyS(font->fontFamily, kMaxIDLength, dict[i+1]);
    } else {
      nsvg__parseAttr(p, dict[i], dict[i + 1]);
    }
  }

  AsciiStrCpyS(font->id, kMaxIDLength, curAttr->id);
  if (!font->horizAdvX) {
    font->horizAdvX = 1000;
  }
//  DBG("found font id=%s family=%s\n", font->id, font->fontFamily);

  NSVGfontChain* fontChain = (decltype(fontChain))nsvg__alloc(sizeof(*fontChain), "nsvg__parseFont fontChain"_XS8);
  fontChain->font = font;
  fontChain->next = p->fontsDB;
  p->currentFont = font;

  p->fontsDB = fontChain;
}

static void nsvg__parseFontFace(NSVGparser* p, char** dict)
{
  int i;
  if (!p) {
//    DBG("no parser\n");
    return;
  }
  NSVGfont* font = p->currentFont;  //if present??? assumed good svg structure
  if (!font) {
    return;
  }
//  DBG("begin parse font face, font->id=%s\n", font->id);
  for (i = 0; dict[i]; i += 2) {
    if (strcmp(dict[i], "font-family") == 0) {
      AsciiStrCpyS(font->fontFamily, 64, dict[i+1]);
 //             DBG("font-family %s\n", font->fontFamily);
    }
    else if (strcmp(dict[i], "font-weight") == 0) {
      float fontWeight = 0.0f;
      AsciiStrToFloat(dict[i+1], NULL /*&Next*/, &fontWeight);
      font->fontWeight = fontWeight;
    }
    else if (strcmp(dict[i], "font-style") == 0) {
      if (strstr(dict[i+1], "italic") != NULL)  {
        font->fontStyle = 'i';
      } else if (strstr(dict[i+1], "bold") != NULL)  {
        font->fontStyle = 'b';
      } else if (strstr(dict[i + 1], "light") != NULL) {
        font->fontStyle = 'l';
      } else {
        font->fontStyle = 'n'; //normal
      }
    }
    else if (strcmp(dict[i], "units-per-em") == 0) {
      float unitsPerEm = 0.0f;
      AsciiStrToFloat(dict[i+1], NULL /*&Next*/, &unitsPerEm);
      font->unitsPerEm = unitsPerEm;
    }
    else if (strcmp(dict[i], "ascent") == 0) {
      font->ascent = (int)AsciiStrDecimalToUintn(dict[i+1]);
    }
    else if (strcmp(dict[i], "descent") == 0) {
      font->descent = (int)AsciiStrDecimalToUintn(dict[i+1]);
    }
    else if (strcmp(dict[i], "x-height") == 0) {
      font->xHeight = (int)AsciiStrDecimalToUintn(dict[i+1]);
    }
    else if (strcmp(dict[i], "cap-height") == 0) {
      font->capHeight = (int)AsciiStrDecimalToUintn(dict[i+1]);
    }
    else if (strcmp(dict[i], "underline-thickness") == 0) {
      font->underlineThickness = (int)AsciiStrDecimalToUintn(dict[i+1]);
    }
    else if (strcmp(dict[i], "underline-position") == 0) {
      font->underlinePosition = (int)AsciiStrDecimalToUintn(dict[i+1]);
    }
    else if (strcmp(dict[i], "slope") == 0) {
      font->slope = (int)AsciiStrDecimalToUintn(dict[i+1]);
    }
    else if (strcmp(dict[i], "bbox") == 0) {
      char* Next = 0;
      AsciiStrToFloat(dict[i + 1], &Next, &font->bbox[0]);
      AsciiStrToFloat((const char*)Next, &Next, &font->bbox[1]);
      AsciiStrToFloat((const char*)Next, &Next, &font->bbox[2]);
      AsciiStrToFloat((const char*)Next, &Next, &font->bbox[3]);
//      nsvg__dumpFloat("font bbox=", font->bbox, 4);
    }
    else if (strcmp(dict[i], "unicode-range") == 0) {
      const char * a = dict[i + 1];
      if (*a == 'U') {
        font->unicodeRange[0] = (int)AsciiStrHexToUintn(a+2);
        font->unicodeRange[1] = (int)AsciiStrHexToUintn(a+7);
      }
    } else nsvg__parseAttr(p, dict[i], dict[i + 1]);

  }
  if (font->unitsPerEm < 1.f) {
    font->unitsPerEm = 1000.f;
  }
  if ((font->bbox[3] - font->bbox[1]) < 1.) {
    font->bbox[0] = 0;
    font->bbox[1] = 0;
    font->bbox[2] = font->unitsPerEm;
    font->bbox[3] = font->unitsPerEm;
  }
  if (font->fontWeight < 1.f) {
    font->fontWeight = font->unitsPerEm;
  }
  if (font->fontStyle < 0x30) {
    font->fontStyle = 'n';
  }
}

CHAR16 nsvg__parseUnicode(const char *s)
{
  CHAR16 A = L'\0';
  if (*s != '&') {
    if (strlen(s) == 2) {
      A = (*s << 8) + *(s+1);
    } else {
      A = *s;
    }
  } else if (strstr(s, "&#x") !=0 ) {
    s += 3;
    while (IS_HEX(*s) || IS_DIGIT(*s)) {
      A <<= 4;
      if (IS_DIGIT(*s)) {
        A += *s - 0x30;
      } else if (IS_UPPER(*s)) {
        A += *s - 0x41 + 10;
      } else {
        A += *s - 0x61 + 10;
      }
      s++;
    }
  } else if (strstr(s, "&amp;") !=0 ) {
    A = 0x26; //&
  } else if (strstr(s, "&quot;") !=0 ) {
    A = 0x22; //"
  } else if (strstr(s, "&lt;") !=0 ) {
    A = 0x3C; //<
  } else if (strstr(s, "&gt;") !=0 ) {
    A = 0x3E; //>
  } else if (strstr(s, "&nbsp;") !=0 ) {
    A = 0xA0; //>
  } else if (strstr(s, "&copy;") !=0 ) {
    A = 0xA9; //>
  }
  return A;
}

static void nsvg__parseGlyph(NSVGparser* p, char** dict, XBool missing)
{
  //glyph-name="E_d" unicode="Ed" horiz-adv-x="1289" d="M679 ..."/>
  /*
   typedef struct NSVGglyph {
     char name[16];
     CHAR16 unicode;
     int horizAdvX;
     NSVGpath* path;
     struct NSVGglyph *next;
   } NSVGglyph;
   */
  int i;
  NSVGglyph *glyph;
  if (!p) {
    return;
  }

  p->pathList = NULL;

  glyph = (NSVGglyph*)nsvg__alloczero(sizeof(NSVGglyph), "nsvg__parseGlyph"_XS8);
  if (!glyph) {
    return;
  }
  for (i = 0; dict[i]; i += 2) {
    if (!nsvg__parseAttr(p, dict[i], dict[i + 1])) {
      if (strcmp(dict[i], "unicode") == 0) {
        glyph->unicode = nsvg__parseUnicode(dict[i+1]);
      } else if (strcmp(dict[i], "horiz-adv-x") == 0) {
        glyph->horizAdvX = (int)AsciiStrDecimalToUintn(dict[i+1]);
      } else if (strcmp(dict[i], "glyph-name") == 0) {
        strncpy(glyph->name, dict[i+1], 16);
        glyph->name[15] = '\0';
        //DBG("nsvg__parseGlyph name=%s\n", glyph->name);
        if (strcmp(dict[i+1], "nonmarkingreturn") == 0) {
          glyph->unicode = L'\n';
        } else if (strcmp(dict[i+1], ".notdef") == 0) {
          missing = true;
        }
      }
    }
  }
  nsvg__parsePath(p, dict);

  glyph->path = p->pathList;
  p->pathList = 0;

  if (p->currentFont) {
    //DBG("nsvg__parseGlyph name=%s missign=%d currentfont=%s\n", glyph->name, (bool)missing, p->currentFont->id);
    if (missing) {
      //Jief : Having more than one missing glyph happen at least with cesium theme.
      // That's why I add them in the chain instead of just reassign p->currentFont->missingGlyph
      glyph->next = p->currentFont->missingGlyph;
      p->currentFont->missingGlyph = glyph;
      if (!glyph->horizAdvX && p->currentFont->horizAdvX) {
        p->currentFont->missingGlyph->horizAdvX = p->currentFont->horizAdvX;
      }
    } else {
      if (!glyph->horizAdvX) {
        if (p->currentFont->missingGlyph) {
          glyph->horizAdvX = p->currentFont->missingGlyph->horizAdvX;
        } else if (p->currentFont->horizAdvX) {
          glyph->horizAdvX = p->currentFont->horizAdvX;
        }
      }
      glyph->next = p->currentFont->glyphs;
      p->currentFont->glyphs = glyph;
    }
  }
  //  DBG("glyph %X parsed\n", glyph->unicode);
}

static void nsvg__startElement(void* ud, const char* el, char** dict)
{
  NSVGparser* p = (NSVGparser*)ud;
  if (!p) {
    return;
  }

  if (strcmp(el, "linearGradient") == 0) {
    nsvg__parseGradient(p, dict, NSVG_PAINT_LINEAR_GRADIENT);
  } else if (strcmp(el, "radialGradient") == 0) {
    nsvg__parseGradient(p, dict, NSVG_PAINT_RADIAL_GRADIENT);
  } else if (strcmp(el, "conicGradient") == 0) {
    nsvg__parseGradient(p, dict, NSVG_PAINT_CONIC_GRADIENT);
  } else if (strcmp(el, "stop") == 0) {
    nsvg__parseGradientStop(p, dict);
  } else if (strcmp(el, "font") == 0) {
    nsvg__parseFont(p, dict);
  } else if (strcmp(el, "font-face") == 0) {
    nsvg__parseFontFace(p, dict);
  } else if (strcmp(el, "missing-glyph") == 0) {
    nsvg__parseGlyph(p, dict, true);
  } else if (strcmp(el, "glyph") == 0) {
    nsvg__parseGlyph(p, dict, false);
  } else if (strcmp(el, "style") == 0) {
    p->styleFlag = 1;
  } else if (strcmp(el, "g") == 0) {
    //    nsvg__pushAttr(p);
    nsvg__parseGroup(p, dict);
  } else if (strcmp(el, "text") == 0) {
    nsvg__pushAttr(p);
    p->isText = true;
    nsvg__parseText(p, dict);
  } else if (strcmp(el, "tspan") == 0) {
    nsvg__pushAttr(p);
    nsvg__parseTextSpan(p, dict);
    nsvg__popAttr(p);
  } else if (strcmp(el, "path") == 0) {
    if (p->pathFlag)  {  // Do not allow nested paths.
      return;
    }
    nsvg__pushAttr(p);
    p->pathFlag = 1;
    p->shapeFlag = 1;
    nsvg__parsePath(p, dict);
    nsvg__addShape(p);
    nsvg__popAttr(p);
  } else if (strcmp(el, "rect") == 0) {
    nsvg__pushAttr(p);
    p->shapeFlag = 1;
    nsvg__parseRect(p, dict);
    nsvg__popAttr(p);
  } else if (strcmp(el, "circle") == 0) {
    nsvg__pushAttr(p);
    p->shapeFlag = 1;
    nsvg__parseCircle(p, dict);
    nsvg__popAttr(p);
  } else if (strcmp(el, "ellipse") == 0) {
    nsvg__pushAttr(p);
    p->shapeFlag = 1;
    nsvg__parseEllipse(p, dict);
    nsvg__popAttr(p);
  } else if (strcmp(el, "line") == 0)  {
    nsvg__pushAttr(p);
    p->shapeFlag = 1;
    nsvg__parseLine(p, dict);
    nsvg__popAttr(p);
  } else if (strcmp(el, "polyline") == 0)  {
    nsvg__pushAttr(p);
    p->shapeFlag = 1;
    nsvg__parsePoly(p, dict, 0);
    nsvg__popAttr(p);
  } else if (strcmp(el, "polygon") == 0)  {
    nsvg__pushAttr(p);
    p->shapeFlag = 1;
    nsvg__parsePoly(p, dict, 1);
    nsvg__popAttr(p);
  } else if (strcmp(el, "use") == 0) {
    nsvg__pushAttr(p);
    p->shapeFlag = 1;
    nsvg__parseUse(p, dict);
    nsvg__popAttr(p);
  } else if (strcmp(el, "defs") == 0) {
    p->defsFlag = 1;
  } else if (strcmp(el, "symbol") == 0) {
    nsvg__pushAttr(p);
    p->symbolFlag = 1;
    nsvg__parseSymbol(p, dict);
  } else if (strcmp(el, "svg") == 0) {
    nsvg__pushAttr(p);
    nsvg__parseSVG(p, dict);
  } else if (strcmp(el, "clipPath") == 0) {
    int i;
    nsvg__pushAttr(p);
    for (i = 0; dict[i]; i += 2) {
      if (strcmp(dict[i], "id") == 0) {
        p->clipPath = nsvg__findClipPath(p, dict[i+1]);
        break;
      }
    }
  } else if (strcmp(el, "title") == 0) {
    p->titleFlag = 1;
  } else if (strcmp(el, "image") == 0) {
    //    nsvg__pushAttr(p);
    //    nsvg__parseIMAGE(p, dict);
    nsvg__parseEmbeddedPNG(p, dict);
    //    nsvg__popAttr(p);
  } else if (strcmp(el, "pattern") == 0) {
    nsvg__parsePattern(p, dict);
    p->patternFlag = 1;

  } else if (strcmp(el, "clover:theme") == 0) {
    ThemeX->parseTheme((void*)p, dict);

  } else {
    strncpy(p->unknown, el, 63);
  }
}

static void nsvg__endElement(void* ud, const char* el)
{
  NSVGparser* p = (NSVGparser*)ud;
  //  NSVGattrib* curAttr = nsvg__getAttr(p);

  if (strcmp(el, "g") == 0) {
    nsvg__popAttr(p);
  } else if (strcmp(el, "path") == 0) {
    p->pathFlag = 0;
  } else if (strcmp(el, "defs") == 0) {
    p->defsFlag = 0;
  } else if (strcmp(el, "pattern") == 0) {
    p->patternFlag = 0;
  } else if (strcmp(el, "symbol") == 0) {
//    nsvg__getSymbolBounds(p); //no sense
    nsvg__popAttr(p);
    p->symbolFlag = 0;
  } else if (strcmp(el, "svg") == 0) {
    nsvg__popAttr(p);
  } else if (strcmp(el, "clipPath") == 0) {
    if (p->clipPath != NULL) {
      NSVGshape* shape = p->clipPath->shapes;
      while (shape != NULL) {
        shape->fill.type = NSVG_PAINT_COLOR;
        shape->stroke.type = NSVG_PAINT_NONE;
        shape = shape->next;
      }
      p->clipPath = NULL;
    }
    nsvg__popAttr(p);
  } else if (strcmp(el, "text") == 0) {
    nsvg__popAttr(p);
    p->isText = false;
    //  } else if (strcmp(el, "tspan") == 0) {
    //    nsvg__popAttr(p);

  } else if (strcmp(el, "title") == 0) {
    p->titleFlag = 0;
  } else if (strcmp(el, "style") == 0) {
    p->styleFlag = 0;
  } else if (strcmp(el, "rect") == 0 ||
             strcmp(el, "circle") == 0 ||
             strcmp(el, "ellipse") == 0 ||
             strcmp(el, "line") == 0 ||
             strcmp(el, "polyline") == 0 ||
             strcmp(el, "polygon") == 0 ||
             strcmp(el, "use") == 0
             ) {
    p->shapeFlag = 0;
  } else if (strcmp(el, p->unknown) == 0) {
    //    p->defsFlag = 0;
  }
}

float nsvg__addLetter(NSVGparser* p, CHAR16 letter, float x, float y, float scale, UINT32 color)
{
  float x1 = x; //initial position
  //  INTN y = 0;
  NSVGshape *shape;
  NSVGattrib* attr = nsvg__getAttr(p);
  NSVGglyph* g;
  if (!p->text || !p->text->font) {
    DBG("font absent\n");
    return x;
  }
  if ( scale == 0 ) { // doing "if (!scale)" generates a warning
    return x;
  }

  shape = (NSVGshape*)nsvg__alloczero(sizeof(NSVGshape), S8Printf("addLetter %lc (shape)", letter));
  if (shape == NULL) return x;

  g = p->text->font->glyphs;
  while (g) {
    if (g->unicode == letter) {
      shape->paths = g->path;
      /*
      if (shape->paths) {
        if (letter == L'C') {
          DBG("Found glyph %X, point[0]=(%d,%d) points=%d\n", letter,
              (int)shape->paths->pts[0], (int)shape->paths->pts[1], shape->paths->npts);
          shape->debug = true;
        }
      } */
      break;
    }
    g = g->next;
  }
  if (!g) {
    //missing glyph
    g = p->text->font->missingGlyph;
    shape->paths = g->path;
    //    if (shape->paths) {
    //      DBG("Missing glyph %X, path[0]=%d\n", letter, (int)shape->paths->pts[0]);
    //    }
  }
  if (!shape->paths) {
    if (g) {
      x1 += g->horizAdvX * scale; //user space
    }
    if (shape) {
      nsvg__delete(shape, "addLetter shape"_XS8);
    }
    return x1;
  }
  //fill shape
  //  DBG("fill shape\n");   //ssss
  shape->group = p->text->group;
  shape->id[0] = (char)(letter & 0xff);
  shape->id[1] = (char)((letter >> 8) & 0xff);
  shape->strokeWidth = p->text->strokeWidth / scale;
  shape->strokeLineJoin = attr->strokeLineJoin;
  shape->strokeLineCap = attr->strokeLineCap;
  shape->miterLimit = attr->miterLimit;
  shape->fillRule = NSVG_FILLRULE_NONZERO;
  shape->opacity = 1.f;

  shape->fill.type = NSVG_PAINT_NONE;
  if (attr->hasFill == 1) {
    shape->fill.type = NSVG_PAINT_COLOR;
    shape->fill.paint.color = color;
  }
  shape->stroke.type = NSVG_PAINT_NONE;
  if (attr->hasStroke == 1) {
    shape->stroke.type = NSVG_PAINT_COLOR;
    shape->stroke.paint.color = p->text->strokeColor;
  }

  shape->flags = NSVG_VIS_DISPLAY | NSVG_VIS_VISIBLE;
  shape->isText = true;
  nsvg__xformIdentity(shape->xform);
  //scale convert shape from glyph size to user's font-size
  shape->xform[0] = scale; //1.f;
  shape->xform[3] = -scale; //-1.f; //glyphs are mirrored by Y
  shape->xform[4] = x - p->text->font->bbox[0] * scale;
  shape->xform[5] = y + p->text->font->bbox[3] * scale; // Y3 is a floor for a letter, so Y+x[5]=realY
// then apply text transform
  nsvg__xformMultiply(shape->xform, p->text->xform);


//   if (letter == L'C') {
//	 DBG("bbox0=%f ", p->text->font->bbox[0]);
//     DBG("bbox3=%f \n", p->text->font->bbox[3]);
//     nsvg__dumpFloat("glyph xform:", shape->xform, 6);
//     DBG("stroke-color=%X ", shape->stroke.paint.color);
//     DBG("stroke-width=%f\n", shape->strokeWidth);
//   }

  //in glyph units
  shape->bounds[0] = p->text->font->bbox[0] + x/scale; //x + p->font->bbox[0] * scale;
  shape->bounds[1] = p->text->font->bbox[1] + y/scale; //y + p->font->bbox[1] * scale;
  shape->bounds[2] = p->text->font->bbox[2] + x/scale; //x + p->font->bbox[2] * scale;
  shape->bounds[3] = p->text->font->bbox[3] + y/scale; //y + p->font->bbox[3] * scale;
  //  if (letter == L'C') {
  //      nsvg__dumpFloat("glyph bounds in text", shape->bounds, 4);
  //  }
  //  if (color == NSVG_RGBA(0x80, 0xFF, 0, 255)) {
  //    DBG("glyph code=%X\n", letter);
  //    nsvg__dumpFloat("glyph xform", shape->xform, 6);
  //    nsvg__dumpFloat("glyph bounds", shape->bounds, 4);
  //    DBG("glyph width=%d\n", g->horizAdvX);
  //  }

  x1 += g->horizAdvX * scale; //position for next letter in user's units

  // Add to tail
  if (p->image->shapes == NULL)
    p->image->shapes = shape;
  else
    p->shapesTail->next = shape;
  p->shapesTail = shape;
  return x1;
}

static void nsvg__addString(NSVGparser* p, char* s)
{
  //text support should create shape for each letter
  UINTN len = strlen(s);
  UINTN i;
  if (!p->text->font) {
    DBG("font for the text is not loaded\n");
    return; //use external fonts
  }
  //  DBG("the text %ls uses font %s\n", s, p->text->fontFace->fontFamily);

  //calculate letter size
  float sy = p->text->font->bbox[3] - p->text->font->bbox[1];
  sy = (sy <= 0.f)? p->text->font->fontWeight: sy;
  //required height
  float h = p->text->fontSize;
  float scale = h / sy;  //scale to font size
  //text position based on ?
  float x = p->text->x;  //user space
  float y = p->text->y - h;
  for (i = 0; i < len; i++) {
    CHAR16 letter = 0;
    s = GetUnicodeChar(s, &letter);
    if (!letter) {
      break;
    }
    x = nsvg__addLetter(p, letter, x, y, scale, p->text->fontColor);
  }
}

static void nsvg__content(void* ud, char* s)
{
  NSVGparser* p = (NSVGparser*)ud;
  if (p->titleFlag) {
    int len = (int)strlen(s);
    NSVGshape *shape = p->image->shapes;
    const int lim = sizeof(shape->title);
    if(len > lim-1)
      len = lim-1;
    if (p->shapeFlag) {
      while (shape->next)
        shape = shape->next;
//      if (shape) {
      memcpy(shape->title, s, len);
      memset(shape->title + len, 0, lim-len);
//      }
    } else { //not shape
      NSVGattrib* attr = nsvg__getAttr(p);
      memcpy(attr->title, s, len);
      memset(attr->title + len, 0, lim-len);
    }
  } else
    if (p->styleFlag) {
      // decrease string to cdata content (if present)
      char* rv = strstr(s, "<![CDATA[");
      if (rv) {
        s = rv + 9;
        rv = strstr(s, "]]>");
        if (!rv)
          return;
        else *rv = '\0';
      }

      //.cls-1{fill:url(#linear-gradient);}
      const char* start = s;
      int state = 0;
      while (*s) {
        char c = *s;
        if (state == 1) {
          if (nsvg__isspace(c) || c == '{') {
            NSVGstyles* next = p->styles;
            p->styles = (NSVGstyles*)nsvg__alloc(sizeof(NSVGstyles), "nsvg__content"_XS8);
            p->styles->next = next;
            p->styles->name = nsvg__strndup(start, (size_t)(s - start)); //style->name=cls-1
            p->styles->description = NULL;
            if (c == '{') {
              start = s + 1;
              state = 3;
            } else {
              state = 2;
            }
          }
        } else if (state == 2 && c == '{') {
          start = s + 1;
          state = 3;
        } else if (state == 3 && c == '}') {
          p->styles->description = nsvg__strndup(start, (size_t)(s - start));
          //     nsvg__parseStyle(p, p->styles->description);
          state = 0;
        } else if (state == 0 && c == '.') {
          start = s + 1;
          state = 1;
        }
        s++;
      }
    }
    else if (p->isText) { //text support
      nsvg__addString(p, s);
    }
}

static void nsvg__assignGradients(NSVGparser* p, NSVGshape* shapes)
{
  for (NSVGshape* shape = shapes; shape != NULL; shape = shape->next) {

    if (shape->fill.type == NSVG_PAINT_GRADIENT_LINK) {
      NSVGgradientLink* link = shape->fill.paint.gradientLink;
      shape->fill.paint.gradient = nsvg__createGradient(p, shape, link, &shape->fill.type);
      if (link != NULL) {
        nsvg__delete(link, "nsvg__assignGradients"_XS8);
      }
      if (shape->fill.paint.gradient == NULL) {
        shape->fill.type = NSVG_PAINT_NONE;
      }
    }
    if (shape->stroke.type == NSVG_PAINT_GRADIENT_LINK) {
      NSVGgradientLink* link = shape->stroke.paint.gradientLink;
      shape->stroke.paint.gradient = nsvg__createGradient(p, shape, link, &shape->stroke.type);
      if (link != NULL) {
        nsvg__delete(link, "nsvg__assignGradients"_XS8);
      }
      if (shape->stroke.paint.gradient == NULL) {
        shape->stroke.type = NSVG_PAINT_NONE;
      }
    }
  }
}

static char *nsvg__strndup(const char *s, size_t n)
{
  char *result;
  size_t len = strlen(s);

  if (n < len)
    len = n;

  result = (char *)nsvg__alloccopy(len + 1, s, "nsvg__strndup"_XS8);
  if (!result)
    return 0;

  result[len] = '\0';
  return result;
}

void nsvg__takeXformBounds(NSVGshape *shape, float *xform, float *bounds)
{
  float newBounds[8]; //(x1, y1), (x2, y2), (x2, y1), (x1, y2)
  nsvg__xformPoint(&newBounds[0], &newBounds[1], shape->bounds[0], shape->bounds[1], xform);
  nsvg__xformPoint(&newBounds[2], &newBounds[3], shape->bounds[2], shape->bounds[3], xform);
  nsvg__xformPoint(&newBounds[4], &newBounds[5], shape->bounds[2], shape->bounds[1], xform);
  nsvg__xformPoint(&newBounds[6], &newBounds[7], shape->bounds[0], shape->bounds[3], xform);
//we have to take into account all points, as x1 can be > x2 etc.
  bounds[0] = nsvg__minf(bounds[0], newBounds[0]);
  bounds[0] = nsvg__minf(bounds[0], newBounds[2]);
  bounds[0] = nsvg__minf(bounds[0], newBounds[4]);
  bounds[0] = nsvg__minf(bounds[0], newBounds[6]);

  bounds[1] = nsvg__minf(bounds[1], newBounds[1]);
  bounds[1] = nsvg__minf(bounds[1], newBounds[3]);
  bounds[1] = nsvg__minf(bounds[1], newBounds[5]);
  bounds[1] = nsvg__minf(bounds[1], newBounds[7]);

  bounds[2] = nsvg__maxf(bounds[2], newBounds[0]);
  bounds[2] = nsvg__maxf(bounds[2], newBounds[2]);
  bounds[2] = nsvg__maxf(bounds[2], newBounds[4]);
  bounds[2] = nsvg__maxf(bounds[2], newBounds[6]);
  
  bounds[3] = nsvg__maxf(bounds[3], newBounds[1]);
  bounds[3] = nsvg__maxf(bounds[3], newBounds[3]);
  bounds[3] = nsvg__maxf(bounds[3], newBounds[5]);
  bounds[3] = nsvg__maxf(bounds[3], newBounds[7]);
}

bool nsvg__isShapeInGroup(NSVGshape* shape, const char* groupName)
{
    NSVGgroup* group = shape->group;
    while (group) {
      if (strcmp(group->id, groupName) == 0) {
        return true;
      }
      group = group->parent;
    }
    return false;
}

//image bounds for a shape group
//bounds inited before use, called from nsvgParse
//assumed each shape already has bounds calculated.
int nsvg__shapesBound(/*NSVGimage* image,*/ NSVGshape *shapes, float* bounds, const char* groupName)
{
  NSVGshape *shape, *shapeLink;
  float xform[6];
  float xform2[6];
  int count = 0;
  int visibility;
  for (shapeLink = shapes; shapeLink != NULL; shapeLink = shapeLink->next) {
    if ( groupName && !nsvg__isShapeInGroup(shapeLink, groupName) ) {
      continue;
    }
    memcpy(&xform[0], shapeLink->xform, sizeof(float)*6);
    visibility = (shapeLink->flags & NSVG_VIS_VISIBLE); //check origin visibility, not link

    if (/*shapeLink->isText ||*/ !visibility) { //dont count text
      continue;
    }
    shape = shapeLink->link;  //this is <use>
    if (!shape) {
      nsvg__takeXformBounds(shapeLink, &xform[0], bounds);
    }
    while (shape) { //take bounds from symbol's shapes
      memcpy(xform2, xform, sizeof(float)*6);
      nsvg__xformPremultiply(&xform2[0], shape->xform);
      nsvg__takeXformBounds(shape, &xform2[0], bounds);
      shape = shape->next;
    }
/*
    if (shapeLink->isText) { //strstr(shapeLink->id, "shar")) {
       DBG("take Bounds: shapeID=%s\n", shapeLink->id);
       nsvg__dumpFloat("  transform", xform, 6);
       nsvg__dumpFloat("  shape initial bounds", shapeLink->bounds, 4);
     }
*/
    count++; //count visible
  }
  return count;
}

void nsvg__imageBounds(NSVGimage* image, float* bounds)
{
  NSVGclipPath* clipPath;
  if (!bounds || !image) {
    return;
  }
  bounds[0] = FLT_MAX;
  bounds[1] = FLT_MAX;
  bounds[2] = -FLT_MAX;
  bounds[3] = -FLT_MAX;

  int count = 0;
  clipPath = image->clipPaths;
  while (clipPath != NULL) {
    for (int i = 0; i < image->clip.count; i++) {
      if (clipPath->index == image->clip.index[i]) {
        count += nsvg__shapesBound(clipPath->shapes, bounds, NULL);
        break;
      }
    }
    clipPath = clipPath->next;
  }
  count += nsvg__shapesBound(image->shapes, bounds, NULL);
//  DBG("found shapes=%d\n", count);
  if (count == 0) {
    bounds[0] = bounds[1] = 0.0f;
    bounds[2] = bounds[3] = 1.0f;
  }
}

NSVGclipPath* nsvg__getClipPathWithIndex(NSVGimage* image, NSVGclipPathIndex idx)
{
  NSVGclipPath* clipPath = image->clipPaths;
  for (NSVGclipPathIndex i = 0; i < idx; i++) clipPath = clipPath->next;
  return clipPath;

}

void nsvg__imageBounds(NSVGimage* image, float* bounds, const char* groupName)
{
  if (!bounds || !image) {
    return;
  }
  bounds[0] = FLT_MAX;
  bounds[1] = FLT_MAX;
  bounds[2] = -FLT_MAX;
  bounds[3] = -FLT_MAX;

  int count = 0;

  NSVGshape *shape;
  for (shape = image->shapes; shape != NULL; shape = shape->next) {
    if ( groupName && !nsvg__isShapeInGroup(shape, groupName) ) {
      continue;
    }
//    DBG("nsvg__imageBounds2 found shapes=%s shape->clip.count=%d\n", shape->id, shape->clip.count);
    for (int i = 0; i < shape->clip.count; i++) {
      NSVGclipPath* clipPath = nsvg__getClipPathWithIndex(image, shape->clip.index[i]);
      if ( clipPath ) {
//        DBG("nsvg__imageBounds found clipPath %s\n", clipPath->id);
        count += nsvg__shapesBound(clipPath->shapes, bounds, NULL);
      }
    }
  }
  count += nsvg__shapesBound(image->shapes, bounds, groupName);
  if (count == 0) {
    bounds[0] = bounds[1] = 0.0f;
    bounds[2] = bounds[3] = 1.0f;
  }
}

// units like "px" is not used so just exclude it
NSVGparser* nsvg__parse(char* input, /* const char* units,*/ float dpi, float opacity)
{
  NSVGparser* p;
  NSVGclipPath* clipPath;
  NSVGsymbol* symbol;
  float bounds[4];

  p = nsvg__createParser();
  if (p == NULL) {
    return NULL;
  }
  p->dpi = dpi;
  p->opacity = opacity;
//  DBG("fontDb=%X\n", (UINTN)fontsDB);
  nsvg__parseXML(input, nsvg__startElement, nsvg__endElement, nsvg__content, p);
//  DBG("fontDb after parse=%X\n", (UINTN)fontsDB);
//  if (fontsDB && fontsDB->font) {
//    DBG("added font=%s\n", fontsDB->font->fontFamily); //yes, fonts added here
//  }
//assign gradients
  clipPath = p->image->clipPaths;
  while (clipPath != NULL) {
    nsvg__assignGradients(p, clipPath->shapes);
    clipPath = clipPath->next;
  }
  symbol = p->symbols;
  while (symbol) {
    nsvg__assignGradients(p, symbol->shapes);
    symbol = symbol->next;
  }
  nsvg__assignGradients(p, p->image->shapes);
  nsvg__imageBounds(p->image, bounds);
#if 1
  memcpy(p->image->realBounds, bounds, 4*sizeof(float));

//  nsvg__dumpFloat("image real bounds", bounds, 4);
  p->image->width = bounds[2] - bounds[0];
  p->image->height = bounds[3] - bounds[1];
#endif
//   DBG("scaled width=%f height=%f\n", p->image->width, p->image->height);
  return p;
}

void nsvg__deleteShapes(NSVGshape* shape)
{
  NSVGshape *snext;
  while (shape != NULL) {
    snext = shape->next;
    if (!shape->link) { //don't touch fake shape!
      nsvg__deleteFont(shape->fontFace);
      shape->fontFace = NULL;
      nsvg__deletePaint(&shape->fill);
      nsvg__deletePaint(&shape->stroke);
      if ( !shape->isText ) {
        nsvg__deletePaths(shape->paths);
      }
    }
    nsvg__delete(shape, "nsvg__deleteShapes"_XS8);
    shape = snext;
  }
}

void nsvg__deleteClipPaths(NSVGclipPath* path)
{
  NSVGclipPath *pnext;
  while (path != NULL) {
    pnext = path->next;
    nsvg__deleteShapes(path->shapes);
    nsvg__delete(path, "nsvg__deleteClipPaths"_XS8);
    path = pnext;
  }
}

void nsvg__deleteImage(NSVGimage* image)
{
  NSVGgroup *group, *gnext;
  if (image == NULL) return;
  nsvg__deleteShapes(image->shapes);
  nsvg__deleteClipPaths(image->clipPaths);
  group = image->groups;
  while (group != NULL) {
    gnext = group->next;
    nsvg__delete(group, "nsvgDelete group"_XS8);
    group = gnext;
  }
  nsvg__delete(image, "nsvgDelete image"_XS8);
}



