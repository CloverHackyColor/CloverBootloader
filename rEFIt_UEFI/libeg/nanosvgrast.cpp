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
 * The polygon rasterization is heavily based on stb_truetype rasterizer
 * by Sean Barrett - http://nothings.org/
 *
 */

/* Example Usage:
 // Load SVG
 struct SNVGImage* image = nsvgParseFromFile("test.svg.");

 // Create rasterizer (can be used to render multiple images).
 struct NSVGrasterizer* rast = nsvgCreateRasterizer();
 // Allocate memory for image
 UINT8* img = malloc(w*h*4);
 // Rasterize
 scaleX = width_to_see / design_width
 nsvgRasterize(rast, image, 0,0, scaleX, scaleY, img, w, h, w*4);
 */

#include "nanosvg.h"
#include "FloatLib.h"
#include "XImage.h"
#include "../Platform/Utils.h"

#ifndef DEBUG_ALL
#define DEBUG_SVG 0
#else
#define DEBUG_SVG DEBUG_ALL
#endif

#if DEBUG_SVG == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_SVG, __VA_ARGS__)
//#define DEBUG_TRACE
#endif


#define pow(x,n) PowF(x,n)
#define sqrtf(x) SqrtF(x)
#define sinf(x) SinF(x)
#define cosf(x) CosF(x)
#define tanf(x) TanF(x)
#define ceilf(x) CeilF(x)
#define floorf(x) FloorF(x)
#define fmodf(x,y) ModF(x,y)
#define acosf(x) AcosF(x)
#define atan2f(y,x) Atan2F(y,x)
#define fabsf(x) FabsF(x)


static void renderShape(NSVGrasterizer* r,
                        NSVGshape* shape, float *xform, float min_scale);


 static int compare_edges(const void* a, const void* b) {
     const NSVGedge* ea = (const NSVGedge*)a;
     const NSVGedge* eb = (const NSVGedge*)b;
     // Сначала по Y0
     if (ea->y0 < eb->y0) return -1;
     if (ea->y0 > eb->y0) return 1;
    
     // При равных Y0 — по X0 (это делает сортировку стабильной для горизонтальных серпов)
     if (ea->x0 < eb->x0) return -1;
     if (ea->x0 > eb->x0) return 1;

     // При равных X0 — по Y1
     if (ea->y1 < eb->y1) return -1;
     if (ea->y1 > eb->y1) return 1;
     return 0;
 }

//caller is responsible for free memory
NSVGrasterizer* nsvg__createRasterizer()
{
  NSVGrasterizer* r = (NSVGrasterizer*)AllocateZeroPool(sizeof(NSVGrasterizer));
  if (r == NULL) return NULL;
  r->tessTol = 0.25f;
  r->distTol = 0.01f;
  r->stencilList = NULL;
  return r;
}

void nsvg__deleteRasterizer(NSVGrasterizer* r)
{
  NSVGmemPage* p;

  if (r == NULL) return;

  p = r->pages;
  while (p != NULL) {
    NSVGmemPage* next = p->next;
    FreePool(p);
    p = next;
  }

  if (r->edges) FreePool(r->edges);
  if (r->points) FreePool(r->points);
  if (r->points2) FreePool(r->points2);
  if (r->scanline) FreePool(r->scanline);
  if (r->stencil) FreePool(r->stencil);

  NSVGstencil* s = r->stencilList;
  while ( s != NULL) {
    NSVGstencil* next = s->next;
    if (s->square) FreePool(s->square);
    FreePool(s);
    s = next;
  }

  FreePool(r);
}

static NSVGmemPage* nsvg__nextPage(NSVGrasterizer* r, NSVGmemPage* cur)
{
  NSVGmemPage *newp;

  // If using existing chain, return the next page in chain
  if (cur != NULL && cur->next != NULL) {
    return cur->next;
  }

  // Alloc new page
  newp = (NSVGmemPage*)AllocateZeroPool(sizeof(NSVGmemPage));
  if (newp == NULL) return NULL;


  // Add to linked list
  if (cur != NULL)
    cur->next = newp;
  else
    r->pages = newp;

  return newp;
}

static void nsvg__resetPool(NSVGrasterizer* r)
{
  NSVGmemPage* p = r->pages;
  while (p != NULL) {
    p->size = 0;
    p = p->next;
  }
  r->curpage = r->pages;
}

static UINT8* nsvgrast__alloc(NSVGrasterizer* r, int size)
{
  UINT8* buf;
  if (size > NSVG__MEMPAGE_SIZE) return NULL;
  if (r->curpage == NULL || r->curpage->size+size > NSVG__MEMPAGE_SIZE) {
    r->curpage = nsvg__nextPage(r, r->curpage);
  }
  buf = &r->curpage->mem[r->curpage->size];
  r->curpage->size += size;
  SetMem(buf, size, 0);  // ← ОБНУЛЯЕМ ВСЮ ВЫДЕЛЕННУЮ ПАМЯТЬ!
  return buf;
}

static int nsvg__ptEquals(NSVGpoint* pt1, NSVGpoint* pt2, float tol)
{
  float dx = pt2->x - pt1->x;
  float dy = pt2->y - pt1->y;
  return SqrF(dx) + SqrF(dy) < SqrF(tol);
}

// t is a matrix xform
static void nsvg__addPathPoint(NSVGrasterizer* r, NSVGpoint* pt, float* t, int flags)
{
  NSVGpoint* pt1;
  NSVGpoint pt2;
  if (!t) {
    pt2 = *pt;
  } else {
    pt2.x = pt->x*t[0] + pt->y*t[2] + t[4];
    pt2.y = pt->x*t[1] + pt->y*t[3] + t[5];
  }

  if (r->npoints > 0) {
    pt1 = &r->points[r->npoints-1];
    if (nsvg__ptEquals(pt1, &pt2, r->distTol)) {
      r->points[r->npoints-1].flags |= (UINT8)flags;
      return;
    }
  }

  if (r->npoints+1 > r->cpoints) {
    int OldSize = r->cpoints * sizeof(NSVGpoint);
    r->cpoints = r->cpoints > 0 ? r->cpoints * 2 : 64;
    if (OldSize == 0) {
      r->points = (NSVGpoint*)AllocatePool(64 * sizeof(NSVGpoint));
    } else {
      r->points = (NSVGpoint*)ReallocatePool(OldSize, sizeof(NSVGpoint) * r->cpoints, r->points);
    }
    if (r->points == NULL) return;
  }

  pt1 = &r->points[r->npoints];

  pt1->x = pt2.x;
  pt1->y = pt2.y;
  pt1->flags = (UINT8)flags;
  r->npoints++;
}

static void nsvg__appendPathPoint(NSVGrasterizer* r, NSVGpoint* pt)
{
  if (r->npoints+1 > r->cpoints) {
    int OldSize = r->cpoints * sizeof(NSVGpoint);
    r->cpoints = r->cpoints > 0 ? r->cpoints * 2 : 64;
    if (OldSize == 0) {
      r->points = (NSVGpoint*)AllocatePool(64 * sizeof(NSVGpoint));
    } else
      r->points = (NSVGpoint*)ReallocatePool(OldSize, sizeof(NSVGpoint) * r->cpoints, r->points);
    if (r->points == NULL) return;
  }
  r->points[r->npoints] = *pt;
  r->npoints++;
}

static void nsvg__duplicatePoints(NSVGrasterizer* r)
{
  if (r->npoints > r->cpoints2) {
    int OldSize = r->cpoints2 * sizeof(NSVGpoint);
    r->cpoints2 = r->npoints;
    if (OldSize == 0) {
      r->points2 = (NSVGpoint*)AllocatePool(r->npoints * sizeof(NSVGpoint));
    } else
      r->points2 = (NSVGpoint*)ReallocatePool(OldSize, sizeof(NSVGpoint) * r->cpoints2, r->points2);
    if (r->points2 == NULL) return;
  }

  if (r->npoints) {
    memcpy(r->points2, r->points, sizeof(NSVGpoint) * r->npoints);
  }

  r->npoints2 = r->npoints;
}

static void nsvg__addEdge(NSVGrasterizer* r, float x0, float y0, float x1, float y1)
{
  NSVGedge* e;

  // Skip horizontal edges
  if (y0 == y1)
    return;

  if (r->nedges+1 > r->cedges) {
    int OldSize = r->cedges * sizeof(NSVGedge);
    r->cedges = r->cedges > 0 ? r->cedges * 2 : 64;
    if (OldSize == 0) {
      r->edges = (NSVGedge*)AllocatePool(64 * sizeof(NSVGedge));
    } else
      r->edges = (NSVGedge*)ReallocatePool(OldSize, sizeof(NSVGedge) * r->cedges, r->edges);
    if (r->edges == NULL) return;
  }

  e = &r->edges[r->nedges];
  r->nedges++;

  if (y0 < y1) {
    e->x0 = x0;
    e->y0 = y0;
    e->x1 = x1;
    e->y1 = y1;
    e->dir = 1;
  } else {
    e->x0 = x1;
    e->y0 = y1;
    e->x1 = x0;
    e->y1 = y0;
    e->dir = -1;
  }
}

static float nsvg__normalize(float *x, float* y)
{
//  float d = sqrtf((*x)*(*x) + (*y)*(*y));
//  float d = SqrtF(SqrF(*x) + SqrF(*y));
  float d = hypot(*x, *y);
  if (d > 1e-6f) {
    float id = 1.0f / d;
    *x *= id;
    *y *= id;
  }
  return d;
}

#define nsvg__absf(x) FabsF(x)
//#define nsvg__sqr(x) SqrF(x)

                    //                   0         1         2         3         4         5         6         7
static float nsvg__controlPathLength(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
{
  float l1, l2, l3;

  l1 = hypot(x2 - x1, y2 - y1);
  l2 = hypot(x3 - x2, y3 - y2);
  l3 = hypot(x4 - x3, y4 - y3);

  return l1 + l2 + l3;
}

static void nsvg__flattenCubicBez2(NSVGrasterizer* r, float* x, float* t, int type)
{
  float ax, ay, bx, by, cx, cy, dx, dy;
  float x1, y1, x2, y2, x3, y3, x4, y4;

  NSVGpoint p;
  float firstFDX, firstFDY, secondFDX, secondFDY, thirdFDX, thirdFDY;
  float h, h2, h3;

  x1 = x[0]*t[0] + x[1]*t[2] + t[4];
  y1 = x[0]*t[1] + x[1]*t[3] + t[5];
  x2 = x[2]*t[0] + x[3]*t[2] + t[4];
  y2 = x[2]*t[1] + x[3]*t[3] + t[5];
  x3 = x[4]*t[0] + x[5]*t[2] + t[4];
  y3 = x[4]*t[1] + x[5]*t[3] + t[5];
  x4 = x[6]*t[0] + x[7]*t[2] + t[4];
  y4 = x[6]*t[1] + x[7]*t[3] + t[5];

  float control_path_len = nsvg__controlPathLength(x1, y1, x2, y2, x3, y3, x4, y4);

  // Безопасное вычисление N с защитой от переполнения
  int N = (int)(control_path_len / 8.0f) + 4;  // было /1.0f + 16
  if (N < 4) N = 4;
  if (N > 256) N = 256;  // было 1024

  /* This is going to need tweaking, gives approximate same number of divisons
   as old code on the test image */
 // N = (int)(control_path_len / ( 32 * r->tessTol)) + 2;


  /* Compute polynomial coefficients from Bezier points */

  ax = -x1 + 3.f * x2 + -3.f * x3 + x4;
  ay = -y1 + 3.f * y2 + -3.f * y3 + y4;

  bx = 3.f * x1 - 6.f * x2 + 3.f * x3;
  by = 3.f * y1 - 6.f * y2 + 3.f * y3;

  cx = 3.0f * (x2 - x1); //-3 * x1 + 3 * x2;
  cy = 3.0f * (y2 - y1); //-3 * y1 + 3 * y2;

  dx = x1;
  dy = y1;

  /* Set up  step size */

  h = 1.0f / (N-1);
  h2 = h * h;
  h3 = h2 * h;

  /* Compute forward differences from Bezier points and "h" */

  p.x = dx;
  p.y = dy;

  firstFDX = ((ax * h + bx) * h + cx) * h;
  firstFDY = ((ay * h + by) * h + cy) * h;

  secondFDX = (6.0f * ax * h + 2.0f * bx) * h2;
  secondFDY = (6.0f * ay * h + 2.0f * by) * h2;

  thirdFDX = 6.0f * ax * h3;
  thirdFDY = 6.0f * ay * h3;

  /* Compute points at each step */
  for (int i = 0; i < N-1; i++)  {
    nsvg__addPathPoint(r, &p, NULL, 0);
    p.x += firstFDX;
    p.y += firstFDY;

    firstFDX += secondFDX;
    firstFDY += secondFDY;

    secondFDX += thirdFDX;
    secondFDY += thirdFDY;

  }
  nsvg__addPathPoint(r, &p, NULL, type);

  return;

}

static void nsvg__flattenShape(NSVGrasterizer* r, NSVGshape* shape, float* xform)
{
  NSVGpath* path;
  NSVGpoint pt;
  //DBG("nsvg__flattenShape\n");

  //  nsvg__dumpFloat("flattenShape with", xform, 6);
  for (path = shape->paths; path != NULL; path = path->next) {
    r->npoints = 0;
    // Flatten path
    pt.x = path->pts[0];
    pt.y = path->pts[1];
    nsvg__addPathPoint(r, &pt, xform, 0);
    for (int i = 0; i < path->npts-1; i += 3) {
      float* p = &path->pts[i*2];
      nsvg__flattenCubicBez2(r, p, xform, 0);
    }
    // Close path
    nsvg__addPathPoint(r, &pt, xform, 0);

    // Build edges
    for (int i = 0, j = r->npoints-1; i < r->npoints; j = i++)
      nsvg__addEdge(r, r->points[j].x, r->points[j].y, r->points[i].x, r->points[i].y);
  }
//  DBG("nsvg__flattenShape: created %d edges for shape %s\n", r->nedges, shape->id);
}

enum NSVGpointFlags
{
  NSVG_PT_CORNER = 0x01,
  NSVG_PT_BEVEL = 0x02,
  NSVG_PT_LEFT = 0x04
};

static void nsvg__initClosed(NSVGpoint* left, NSVGpoint* right, NSVGpoint* p0, NSVGpoint* p1, float lineWidth)
{
  float w = lineWidth * 0.5f;
  float dx = p1->x - p0->x;
  float dy = p1->y - p0->y;
  float len = nsvg__normalize(&dx, &dy);
  float px = p0->x + dx*len*0.5f, py = p0->y + dy*len*0.5f;
  float dlx = dy, dly = -dx;
  float lx = px - dlx*w, ly = py - dly*w;
  float rx = px + dlx*w, ry = py + dly*w;
  left->x = lx; left->y = ly;
  right->x = rx; right->y = ry;
}

static void nsvg__buttCap(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right, NSVGpoint* p, float dx, float dy, float lineWidth, int connect)
{
  float w = lineWidth * 0.5f;
  float px = p->x, py = p->y;
  float dlx = dy, dly = -dx;
  float lx = px - dlx*w, ly = py - dly*w;
  float rx = px + dlx*w, ry = py + dly*w;

  nsvg__addEdge(r, lx, ly, rx, ry);

  if (connect) {
    nsvg__addEdge(r, left->x, left->y, lx, ly);
    nsvg__addEdge(r, rx, ry, right->x, right->y);
  }
  left->x = lx; left->y = ly;
  right->x = rx; right->y = ry;
}

static void nsvg__squareCap(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right, NSVGpoint* p, float dx, float dy, float lineWidth, int connect)
{
  float w = lineWidth * 0.5f;
  float px = p->x - dx*w, py = p->y - dy*w;
  float dlx = dy, dly = -dx;
  float lx = px - dlx*w, ly = py - dly*w;
  float rx = px + dlx*w, ry = py + dly*w;

  nsvg__addEdge(r, lx, ly, rx, ry);

  if (connect) {
    nsvg__addEdge(r, left->x, left->y, lx, ly);
    nsvg__addEdge(r, rx, ry, right->x, right->y);
  }
  left->x = lx; left->y = ly;
  right->x = rx; right->y = ry;
}

#ifndef NSVG_PI
const float NSVG_PI = 3.141592653589793f;
#endif

static void nsvg__roundCap(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right, NSVGpoint* p, float dx, float dy, float lineWidth, int ncap, int connect)
{
  float w = lineWidth * 0.5f;
  float px = p->x, py = p->y;
  float dlx = dy, dly = -dx;
  float lx = 0, ly = 0, rx = 0, ry = 0, prevx = 0, prevy = 0;

  for (int i = 0; i < ncap; i++) {
    float a = (float)i/(float)(ncap-1)*NSVG_PI;
    float ax = cosf(a) * w, ay = sinf(a) * w;
    float x = px - dlx*ax - dx*ay;
    float y = py - dly*ax - dy*ay;

    if (i > 0)
      nsvg__addEdge(r, prevx, prevy, x, y);

    prevx = x;
    prevy = y;

    if (i == 0) {
      lx = x; ly = y;
    } else if (i == ncap-1) {
      rx = x; ry = y;
    }
  }

  if (connect) {
    nsvg__addEdge(r, left->x, left->y, lx, ly);
    nsvg__addEdge(r, rx, ry, right->x, right->y);
  }

  left->x = lx; left->y = ly;
  right->x = rx; right->y = ry;
}

static void nsvg__bevelJoin(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right, NSVGpoint* p0, NSVGpoint* p1, float lineWidth)
{
  float w = lineWidth * 0.5f;
  float dlx0 = p0->dy, dly0 = -p0->dx;
  float dlx1 = p1->dy, dly1 = -p1->dx;
  float lx0 = p1->x - (dlx0 * w), ly0 = p1->y - (dly0 * w);
  float rx0 = p1->x + (dlx0 * w), ry0 = p1->y + (dly0 * w);
  float lx1 = p1->x - (dlx1 * w), ly1 = p1->y - (dly1 * w);
  float rx1 = p1->x + (dlx1 * w), ry1 = p1->y + (dly1 * w);

  nsvg__addEdge(r, lx0, ly0, left->x, left->y);
  nsvg__addEdge(r, lx1, ly1, lx0, ly0);

  nsvg__addEdge(r, right->x, right->y, rx0, ry0);
  nsvg__addEdge(r, rx0, ry0, rx1, ry1);

  left->x = lx1; left->y = ly1;
  right->x = rx1; right->y = ry1;
}

static void nsvg__miterJoin(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right, NSVGpoint* p0, NSVGpoint* p1, float lineWidth)
{
  float w = lineWidth * 0.5f;
  float dlx0 = p0->dy, dly0 = -p0->dx;
  float dlx1 = p1->dy, dly1 = -p1->dx;
  float lx0, rx0, lx1, rx1;
  float ly0, ry0, ly1, ry1;

  if (p1->flags & NSVG_PT_LEFT) {
    lx0 = lx1 = p1->x - p1->dmx * w;
    ly0 = ly1 = p1->y - p1->dmy * w;
    nsvg__addEdge(r, lx1, ly1, left->x, left->y);

    rx0 = p1->x + (dlx0 * w);
    ry0 = p1->y + (dly0 * w);
    rx1 = p1->x + (dlx1 * w);
    ry1 = p1->y + (dly1 * w);
    nsvg__addEdge(r, right->x, right->y, rx0, ry0);
    nsvg__addEdge(r, rx0, ry0, rx1, ry1);
  } else {
    lx0 = p1->x - (dlx0 * w);
    ly0 = p1->y - (dly0 * w);
    lx1 = p1->x - (dlx1 * w);
    ly1 = p1->y - (dly1 * w);
    nsvg__addEdge(r, lx0, ly0, left->x, left->y);
    nsvg__addEdge(r, lx1, ly1, lx0, ly0);

    rx0 = rx1 = p1->x + p1->dmx * w;
    ry0 = ry1 = p1->y + p1->dmy * w;
    nsvg__addEdge(r, right->x, right->y, rx1, ry1);
  }

  left->x = lx1; left->y = ly1;
  right->x = rx1; right->y = ry1;
}

static void nsvg__roundJoin(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right, NSVGpoint* p0, NSVGpoint* p1, float lineWidth, int ncap)
{
  int n;
  float w = lineWidth * 0.5f;
  float dlx0 = p0->dy, dly0 = -p0->dx;
  float dlx1 = p1->dy, dly1 = -p1->dx;
  float a0 = atan2f(dly0, dlx0);
  float a1 = atan2f(dly1, dlx1);
  float da = a1 - a0;
  float lx, ly, rx, ry;

  if (da < -NSVG_PI) da += PI2; //NSVG_PI*2;
  if (da > NSVG_PI) da -= PI2; //NSVG_PI*2;

  n = (int)ceilf((nsvg__absf(da) / NSVG_PI) * (float)ncap);
  if (n < 2) n = 2;
  if (n > ncap) n = ncap;

  lx = left->x;
  ly = left->y;
  rx = right->x;
  ry = right->y;

  for (int i = 0; i < n; i++) {
    float u = (float)i/(float)(n-1);
    float a = a0 + u*da;
    float ax = cosf(a) * w, ay = sinf(a) * w;
    float lx1 = p1->x - ax, ly1 = p1->y - ay;
    float rx1 = p1->x + ax, ry1 = p1->y + ay;

    nsvg__addEdge(r, lx1, ly1, lx, ly);
    nsvg__addEdge(r, rx, ry, rx1, ry1);

    lx = lx1; ly = ly1;
    rx = rx1; ry = ry1;
  }

  left->x = lx; left->y = ly;
  right->x = rx; right->y = ry;
}

static void nsvg__straightJoin(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right, NSVGpoint* p1, float lineWidth)
{
  float w = lineWidth * 0.5f;
  float lx = p1->x - (p1->dmx * w), ly = p1->y - (p1->dmy * w);
  float rx = p1->x + (p1->dmx * w), ry = p1->y + (p1->dmy * w);

  nsvg__addEdge(r, lx, ly, left->x, left->y);
  nsvg__addEdge(r, right->x, right->y, rx, ry);

  left->x = lx; left->y = ly;
  right->x = rx; right->y = ry;
}

static int nsvg__curveDivs(float r, float arc, float tol)
{
  float da = acosf(r / (r + tol)) * 2.0f;
  int divs = (int)ceilf(arc / da);
  if (divs < 2) divs = 2;
  return divs;
}

static void nsvg__expandStroke(NSVGrasterizer* r, NSVGpoint* points, int npoints, int closed, int lineJoin, int lineCap, float lineWidth)
{
  int ncap = nsvg__curveDivs(lineWidth*0.5f, NSVG_PI, r->tessTol);  // Calculate divisions per half circle.
  NSVGpoint left = {0,0,0,0,0,0,0,0,{0,0,0}}, right = {0,0,0,0,0,0,0,0,{0,0,0}}, firstLeft = {0,0,0,0,0,0,0,0,{0,0,0}}, firstRight = {0,0,0,0,0,0,0,0,{0,0,0}};
  NSVGpoint* p0, *p1;
  int s, e;

  // Build stroke edges
  if (closed) {
    // Looping
    p0 = &points[npoints-1];
    p1 = &points[0];
    s = 0;
    e = npoints;
  } else {
    // Add cap
    p0 = &points[0];
    p1 = &points[1];
    s = 1;
    e = npoints-1;
  }

  if (closed) {
    nsvg__initClosed(&left, &right, p0, p1, lineWidth);
    firstLeft = left;
    firstRight = right;
  } else {
    // Add cap
    float dx = p1->x - p0->x;
    float dy = p1->y - p0->y;
    nsvg__normalize(&dx, &dy);
    if (lineCap == NSVG_CAP_BUTT)
      nsvg__buttCap(r, &left, &right, p0, dx, dy, lineWidth, 0);
    else if (lineCap == NSVG_CAP_SQUARE)
      nsvg__squareCap(r, &left, &right, p0, dx, dy, lineWidth, 0);
    else if (lineCap == NSVG_CAP_ROUND)
      nsvg__roundCap(r, &left, &right, p0, dx, dy, lineWidth, ncap, 0);
  }

  for (int j = s; j < e; ++j) {
    if (p1->flags & NSVG_PT_CORNER) {
      if (lineJoin == NSVG_JOIN_ROUND)
        nsvg__roundJoin(r, &left, &right, p0, p1, lineWidth, ncap);
      else if (lineJoin == NSVG_JOIN_BEVEL || (p1->flags & NSVG_PT_BEVEL))
        nsvg__bevelJoin(r, &left, &right, p0, p1, lineWidth);
      else
        nsvg__miterJoin(r, &left, &right, p0, p1, lineWidth);
    } else {
      nsvg__straightJoin(r, &left, &right, p1, lineWidth);
    }
    p0 = p1++;
  }

  if (closed) {
    // Loop it
    nsvg__addEdge(r, firstLeft.x, firstLeft.y, left.x, left.y);
    nsvg__addEdge(r, right.x, right.y, firstRight.x, firstRight.y);
  } else {
    // Add cap
    float dx = p1->x - p0->x;
    float dy = p1->y - p0->y;
    nsvg__normalize(&dx, &dy);
    if (lineCap == NSVG_CAP_BUTT)
      nsvg__buttCap(r, &right, &left, p1, -dx, -dy, lineWidth, 1);
    else if (lineCap == NSVG_CAP_SQUARE)
      nsvg__squareCap(r, &right, &left, p1, -dx, -dy, lineWidth, 1);
    else if (lineCap == NSVG_CAP_ROUND)
      nsvg__roundCap(r, &right, &left, p1, -dx, -dy, lineWidth, ncap, 1);
  }
}

static void nsvg__prepareStroke(NSVGrasterizer* r, float miterLimit, int lineJoin)
{
  NSVGpoint *p0, *p1;

  p0 = &r->points[r->npoints-1];
  p1 = &r->points[0];
  for (int i = 0; i < r->npoints; i++) {
    // Calculate segment direction and length
    p0->dx = p1->x - p0->x;
    p0->dy = p1->y - p0->y;
    p0->len = nsvg__normalize(&p0->dx, &p0->dy);
    // Advance
    p0 = p1++;
  }

  // calculate joins
  p0 = &r->points[r->npoints-1];
  p1 = &r->points[0];
  for (int j = 0; j < r->npoints; j++) {
    float dlx0, dly0, dlx1, dly1, dmr2, cross;
    dlx0 = p0->dy;
    dly0 = -p0->dx;
    dlx1 = p1->dy;
    dly1 = -p1->dx;
    // Calculate extrusions
    p1->dmx = (dlx0 + dlx1) * 0.5f;
    p1->dmy = (dly0 + dly1) * 0.5f;
    dmr2 = p1->dmx*p1->dmx + p1->dmy*p1->dmy;
    if (dmr2 > 0.000001f) {
      float s2 = 1.0f / dmr2;
      if (s2 > 600.0f) {
        s2 = 600.0f;
      }
      p1->dmx *= s2;
      p1->dmy *= s2;
    }

    // Clear flags, but keep the corner.
    p1->flags = (p1->flags & NSVG_PT_CORNER) ? NSVG_PT_CORNER : 0;

    // Keep track of left turns.
    cross = p1->dx * p0->dy - p0->dx * p1->dy;
    if (cross > 0.0f)
      p1->flags |= NSVG_PT_LEFT;

    // Check to see if the corner needs to be beveled.
    if (p1->flags & NSVG_PT_CORNER) {
      if ((dmr2 * miterLimit*miterLimit) < 1.0f || lineJoin == NSVG_JOIN_BEVEL || lineJoin == NSVG_JOIN_ROUND) {
        p1->flags |= NSVG_PT_BEVEL;
      }
    }

    p0 = p1++;
  }
}

static void nsvg__flattenShapeStroke(NSVGrasterizer* r, NSVGshape* shape, float* xform)
{
  int closed;
  NSVGpath* path;
  NSVGpoint* p0, *p1;
  NSVGpoint p;
  float lineWidth = 0.5;
  float scalex1 = fabsf(xform[0]);
  float scalex2 = fabsf(xform[2]);
  float scaley1 = fabsf(xform[1]);
  float scaley2 = fabsf(xform[3]);

  float scale = (hypot(scalex1, scalex2) + hypot(scaley1, scaley2)) * 0.5f;

  float miterLimit = shape->miterLimit;
  int lineJoin = shape->strokeLineJoin;
  int lineCap = shape->strokeLineCap;

  lineWidth = shape->strokeWidth * scale;

  for (path = shape->paths; path != NULL; path = path->next) {
    // Flatten path
    r->npoints = 0;
    p.x = path->pts[0];
    p.y = path->pts[1];
    nsvg__addPathPoint(r, &p, xform, NSVG_PT_CORNER);
    for (int i = 0; i < path->npts-1; i += 3) {
      float* pt = &path->pts[i*2];
      nsvg__flattenCubicBez2(r, pt, xform, NSVG_PT_CORNER);
    }
    if (r->npoints < 2)
      continue;

    closed = path->closed;

    // If the first and last points are the same, remove the last, mark as closed path.
    p0 = &r->points[r->npoints-1];
    p1 = &r->points[0];
    if (nsvg__ptEquals(p0, p1, r->distTol)) {
      r->npoints--;
      p0 = &r->points[r->npoints-1];
      closed = 1;
    }

    if (shape->strokeDashCount > 0) {
      int idash = 0, dashState = 1;
      float totalDist = 0, dashLen, allDashLen, dashOffset;
      NSVGpoint* cur;

      if (closed)
        nsvg__appendPathPoint(r, p1);

      // Duplicate points -> points2.
      nsvg__duplicatePoints(r);

      r->npoints = 0;
      cur = &r->points2[0];
      nsvg__appendPathPoint(r, cur);

      // Figure out dash offset.
      allDashLen = 0;
      for (int j = 0; j < shape->strokeDashCount; j++)
        allDashLen += shape->strokeDashArray[j];
      if (shape->strokeDashCount & 1)
        allDashLen *= 2.0f;
      // Find location inside pattern
      dashOffset = fmodf(shape->strokeDashOffset, allDashLen);
      if (dashOffset < 0.0f)
        dashOffset += allDashLen;

      while (dashOffset > shape->strokeDashArray[idash]) {
        dashOffset -= shape->strokeDashArray[idash];
        idash = (idash + 1) % shape->strokeDashCount;
      }
      dashLen = (shape->strokeDashArray[idash] - dashOffset) * scale;

      for (int j = 1; j < r->npoints2; ) {
        float dx = r->points2[j].x - cur->x;
        float dy = r->points2[j].y - cur->y;
        float dist = hypot(dx, dy);

        if ((totalDist + dist) > dashLen) {
          // Calculate intermediate point
          float d = (dashLen - totalDist) / dist;
          NSVGpoint pc;
          pc.x = cur->x + dx * d;
          pc.y = cur->y + dy * d;
          nsvg__addPathPoint(r, &pc, NULL, NSVG_PT_CORNER);

          // Stroke
          if (r->npoints > 1 && dashState) {
            nsvg__prepareStroke(r, miterLimit, lineJoin);
            nsvg__expandStroke(r, r->points, r->npoints, 0, lineJoin, lineCap, lineWidth);
          }
          // Advance dash pattern
          dashState = !dashState;
          idash = (idash+1) % shape->strokeDashCount;
          dashLen = shape->strokeDashArray[idash] * scale;
          // Restart
          cur->x = pc.x;
          cur->y = pc.y;
          cur->flags = NSVG_PT_CORNER;
          totalDist = 0.0f;
          r->npoints = 0;
          nsvg__appendPathPoint(r, cur);
        } else {
          totalDist += dist;
          cur = &r->points2[j];
          nsvg__appendPathPoint(r, cur);
          j++;
        }
      }
      // Stroke any leftover path
      if (r->npoints > 1 && dashState)
        nsvg__expandStroke(r, r->points, r->npoints, 0, lineJoin, lineCap, lineWidth);
    } else {
      nsvg__prepareStroke(r, miterLimit, lineJoin);
      nsvg__expandStroke(r, r->points, r->npoints, closed, lineJoin, lineCap, lineWidth);
    }
  }
}
/*
 static int nsvg__cmpEdge(const void *p, const void *q)
 {
 const NSVGedge* a = (const NSVGedge*)p;
 const NSVGedge* b = (const NSVGedge*)q;

 if (a->y0 < b->y0) return -1;
 if (a->y0 > b->y0) return  1;
 return 0;
 }
 */

static NSVGactiveEdge* nsvg__addActive(NSVGrasterizer* r, NSVGedge* e, float startPoint)
{
  NSVGactiveEdge* z;

  if (r->freelist != NULL) {
    // Restore from freelist.
    z = r->freelist;
    r->freelist = z->next;
  } else {
    // Alloc new edge.
    z = (NSVGactiveEdge*)nsvgrast__alloc(r, sizeof(NSVGactiveEdge));
    if (z == NULL) return NULL;
  }

  float dxdy = (e->x1 - e->x0) / (e->y1 - e->y0);
  //  STBTT_assert(e->y0 <= start_point);
  // round dx down to avoid going too far
  if (dxdy < 0)
    z->dx = (int)(-floorf(NSVG__FIX * -dxdy));
  else
    z->dx = (int)floorf(NSVG__FIX * dxdy);
  z->x = (int)floorf(NSVG__FIX * (e->x0 + dxdy * (startPoint - e->y0)));
  //  z->x -= off_x * FIX;
  z->ey = e->y1;
  z->next = 0;
  z->dir = e->dir;

  return z;
}

static void nsvg__freeActive(NSVGrasterizer* r, NSVGactiveEdge* z)
{
  z->next = r->freelist;
  r->freelist = z;
}

static void nsvg__fillScanline(UINT8* scanline, int len, int x0, int x1, int maxWeight, int* xmin, int* xmax)
{
//  DBG("nsvg__fillScanline\n");
  int i = x0 >> NSVG__FIXSHIFT;
  int j = x1 >> NSVG__FIXSHIFT;

  if (i < *xmin) *xmin = i;
  if (j > *xmax) *xmax = j;
  if (i < len && j >= 0) {
    if (i == j) {
      // x0,x1 are the same pixel, so compute combined coverage
      scanline[i] = (UINT8)(scanline[i] + ((x1 - x0) * maxWeight >> NSVG__FIXSHIFT));
    } else {
      if (i >= 0) // add antialiasing for x0
        scanline[i] = (UINT8)(scanline[i] + (((NSVG__FIX - (x0 & NSVG__FIXMASK)) * maxWeight) >> NSVG__FIXSHIFT));
      else
        i = -1; // clip

      if (j < len) // add antialiasing for x1
        scanline[j] = (UINT8)(scanline[j] + (((x1 & NSVG__FIXMASK) * maxWeight) >> NSVG__FIXSHIFT));
      else
        j = len; // clip

      for (++i; i < j; ++i) // fill pixels between x0 and x1
        scanline[i] = (UINT8)(scanline[i] + maxWeight);
    }
  }
}

//static void nsvg__sortActiveEdges(NSVGactiveEdge **active)
//{
//    if (!active || !*active) return;
//
//    NSVGactiveEdge *result = NULL;
//    NSVGactiveEdge *current = *active;
//
//    while (current != NULL) {
//        NSVGactiveEdge *next = current->next;
//        current->next = NULL;
//
//        // Вставляем current в result с сортировкой по x
//        if (result == NULL || current->x < result->x) {
//            current->next = result;
//            result = current;
//        } else {
//            NSVGactiveEdge *p = result;
//            while (p->next != NULL && p->next->x < current->x) {
//                p = p->next;
//            }
//            current->next = p->next;
//            p->next = current;
//        }
//        current = next;
//    }
//
//    *active = result;
//}

// note: this routine clips fills that extend off the edges... ideally this
// wouldn't happen, but it could happen if the truetype glyph bounding boxes
// are wrong, or if the user supplies a too-small bitmap

static void nsvg__fillActiveEdges(UINT8 *scanline, int len, NSVGactiveEdge *e,
    int maxWeight, int *xmin, int *xmax, char fillRule)
{
  // non-zero winding fill
//  DBG("nsvg__fillActiveEdges\n");
  int x0 = 0, w = 0;
  int iteration = 0;
  const int MAX_ITERATIONS = 5000;

  if (fillRule == NSVG_FILLRULE_NONZERO) {
    // Non-zero
    while (e != NULL && iteration++ < MAX_ITERATIONS) {
      if (w == 0) {
        // if we're currently at zero, we need to record the edge start point
        x0 = e->x;
        w += e->dir;
      }
      else {
        int x1 = e->x;
        w += e->dir;
        // if we went to zero, we need to draw
        if (w == 0) {
          // ОТЛАДКА: показываем заливку
          if (x0 < x1) {
            nsvg__fillScanline(scanline, len, x0, x1, maxWeight, xmin, xmax);
          }
          else {
            nsvg__fillScanline(scanline, len, x1, x0, maxWeight, xmin, xmax);
          }
        }
      }
      e = e->next;
    }
  }
  else if (fillRule == NSVG_FILLRULE_EVENODD) {
    // Even-odd
#if DEBUG_SVG
    int pair_count = 0;
#endif
    while (e != NULL && iteration++ < MAX_ITERATIONS) {
      if (w == 0) {
        x0 = e->x;
        w = 1;
      }
      else {
        int x1 = e->x;
        w = 0;
#if DEBUG_SVG
        pair_count++;
#endif
        // Всегда заполняем, но с проверкой на корректную пару
        if (x0 < x1 && (x1 - x0) > 16) {
          nsvg__fillScanline(scanline, len, x0, x1, maxWeight, xmin, xmax);
        }
      }
      e = e->next;
    }
    DBG("EVENODD: total %d pairs for this scanline\n", pair_count);
    if (iteration >= MAX_ITERATIONS) {
        DBG("WARNING: nsvg__fillActiveEdges exceeded MAX_ITERATIONS!\n");
    }
  }
}


static float nsvg__clampf(float a, float mn, float mx)
{
  return a < mn ? mn : (a > mx ? mx : a);
}

static UINT32 nsvg__RGBA(UINT8 r, UINT8 g, UINT8 b, UINT8 a)
{
  return (b) | (g << 8) | (r << 16) | (a << 24);
}

static unsigned int nsvg__lerpRGBA(unsigned int c0, unsigned int c1, float u, float opacity)
{
  float xu = nsvg__clampf(u, 0.0f, 1.0f) * 256.0f;
  int iu = (int)(xu); //0..256
  int ia = (int)(nsvg__clampf(opacity, 0.0f, 1.0f) * 256.0f);
  int b = (((c0) & 0xff)*(256-iu) + (((c1) & 0xff)*iu)) >> 8;
  int g = (((c0>>8) & 0xff)*(256-iu) + (((c1>>8) & 0xff)*iu)) >> 8;
  int r = (((c0>>16) & 0xff)*(256-iu) + (((c1>>16) & 0xff)*iu)) >> 8;
  int a = ((((c0>>24) & 0xff)*(256-iu) + (((c1>>24) & 0xff)*iu)) * ia) >> 16;
  return nsvg__RGBA((UINT8)r, (UINT8)g, (UINT8)b, (UINT8)a);
}

static unsigned int nsvg__applyOpacity(unsigned int c, float u)
{
  int iu = (int)(nsvg__clampf(u, 0.0f, 1.0f) * 256.0f);
  int b = (c) & 0xff;
  int g = (c>>8) & 0xff;
  int r = (c>>16) & 0xff;
  int a = (((c>>24) & 0xff)*iu) >> 8;
  return nsvg__RGBA((UINT8)r, (UINT8)g, (UINT8)b, (UINT8)a);
}

static inline int nsvg__div255(int x)
{
  return ((x+1) * 257) >> 16;
}

static void nsvg__scanlineBit(
                              UINT8* row, int count, UINT8* cover, int x, int y,
                              NSVGcachedPaint* cache)
{

    //xxx where is security check that x/8 and (x+count)/8 is inside row[] index?
    // called by       r->fscanline(&r->bitmap[y * r->stride], xmax-xmin+1, &r->scanline[xmin], xmin, y,/* tx,ty, scalex, scaley, */ cache);
  int x1 = x + count;
  for (; x < x1; x++) {
    row[x / 8] |= 1 << (x % 8);
  }
}

static void nsvg__scanlineSolid(UINT8* row, int count, UINT8* cover, int x, int y,
                                NSVGcachedPaint* cache)
{
  //  static int once = 0;
//  DBG("nsvg__scanlineSolid\n");
  UINT8* dst = row + x*4;
  if (cache->type == NSVG_PAINT_COLOR) {
    int cr, cg, cb, ca;
    cr = cache->colors[0] & 0xff;
    cg = (cache->colors[0] >> 8) & 0xff;
    cb = (cache->colors[0] >> 16) & 0xff;
    ca = (cache->colors[0] >> 24) & 0xff;

    for (int i = 0; i < count; i++) {
      int r,g,b;
      int a = nsvg__div255((int)cover[0] * ca);
      int ia = 255 - a;
      // Premultiply
      r = nsvg__div255(cr * a);
      g = nsvg__div255(cg * a);
      b = nsvg__div255(cb * a);

      // Blend over
      r += nsvg__div255(ia * (int)dst[0]);
      g += nsvg__div255(ia * (int)dst[1]);
      b += nsvg__div255(ia * (int)dst[2]);
      a += nsvg__div255(ia * (int)dst[3]);

      dst[0] = (UINT8)r;
      dst[1] = (UINT8)g;
      dst[2] = (UINT8)b;
      dst[3] = (UINT8)a;

      cover++;
      dst += 4;
    }
  } else if (cache->type == NSVG_PAINT_LINEAR_GRADIENT) {
    // TODO: spread modes.
    // TODO: plenty of opportunities to optimize.
    float fx, fy, gy;
    float* t = cache->xform;

    //    nsvg__dumpFloat("cache grad xform", t, 6);
    int cr, cg, cb, ca;
    unsigned int c;
    //x,y - pixels
    fx = (float)x;
    fy = (float)y;
    //    dx = 1.0f;
    gy = fx*t[1] + fy*t[3] + t[5]; //gradient direction. Point at cut

    for (int i = 0; i < count; i++) {
      int r,g,b,a,ia;
      int level = cache->coarse;
      c = cache->colors[dither(nsvg__clampf(gy*(255.0f-level), 0, (float)(255-level)), level)]; //assumed gy = 0.0 ... 1.0f
      cr = (c) & 0xff;
      cg = (c >> 8) & 0xff;
      cb = (c >> 16) & 0xff;
      ca = (c >> 24) & 0xff;
      a = nsvg__div255((int)cover[0] * ca);
      ia = 255 - a;

      // Premultiply
      r = nsvg__div255(cr * a);
      g = nsvg__div255(cg * a);
      b = nsvg__div255(cb * a);
      // Blend over
      r += nsvg__div255(ia * (int)dst[0]);
      g += nsvg__div255(ia * (int)dst[1]);
      b += nsvg__div255(ia * (int)dst[2]);
      a += nsvg__div255(ia * (int)dst[3]);
      dst[0] = (UINT8)r;
      dst[1] = (UINT8)g;
      dst[2] = (UINT8)b;
      dst[3] = (UINT8)a;

      cover++;
      dst += 4;
      //      fx += dx;
      gy += t[1];
    }
  } else if (cache->type == NSVG_PAINT_RADIAL_GRADIENT) {
    // TODO: spread modes.
    // TODO: plenty of opportunities to optimize.
    // TODO: focus (fx,fy)
    float fx, fy, gx, gy, gd;
    float* t = cache->xform;

    int cr, cg, cb, ca;
    unsigned int c;
    fx = (float)x;
    fy = (float)y;
    //    dx = 1.0f;
    gx = fx*t[0] + fy*t[2] + t[4];
    gy = fx*t[1] + fy*t[3] + t[5];

    for (int i = 0; i < count; i++) {
      int r,g,b,a,ia;
      gd = hypot(gx, gy);

      int level = cache->coarse;
      c = cache->colors[dither(nsvg__clampf(gd*(255.0f-level*2), 0, (254.99f-level*2)), level)];
      cr = (c) & 0xff;
      cg = (c >> 8) & 0xff;
      cb = (c >> 16) & 0xff;
      ca = (c >> 24) & 0xff;

      a = nsvg__div255((int)cover[0] * ca);
      ia = 255 - a;

      // Premultiply
      r = nsvg__div255(cr * a);
      g = nsvg__div255(cg * a);
      b = nsvg__div255(cb * a);

      // Blend over
      r += nsvg__div255(ia * (int)dst[0]);
      g += nsvg__div255(ia * (int)dst[1]);
      b += nsvg__div255(ia * (int)dst[2]);
      a += nsvg__div255(ia * (int)dst[3]);

      dst[0] = (UINT8)r;
      dst[1] = (UINT8)g;
      dst[2] = (UINT8)b;
      dst[3] = (UINT8)a;

      cover++;
      dst += 4;
      //      fx += dx;
      gx += t[0];
      gy += t[1];
    }
  } else if (cache->type == NSVG_PAINT_PATTERN) {
    // TODO
    float fx, fy, dx, gx, gy;
    float* t = cache->xform;

    XImage *Pattern = (XImage*)cache->image;
    if (!Pattern) {
      DBG("no pattern to fill\n");
      return;
    }
    INTN Width = Pattern->GetWidth();
    INTN Height = Pattern->GetHeight();
    int ix, iy;

    fx = (float)x;
    fy = (float)y;
    dx = 1.0f;

    //    unsigned int c;
    for (int i = 0; i < count; i++) {
      int r,g,b,a,ia;
      gx = fx*t[0] + fy*t[2] + t[4];
      gy = fx*t[1] + fy*t[3] + t[5];
      ix = dither(gx * Width, 2) % Width;
      iy = dither(gy * Height, 2) % Height;
//      j = iy * Width + ix;
      EFI_GRAPHICS_OUTPUT_BLT_PIXEL cp = Pattern->GetPixel(ix, iy);
//      cr = Pattern->PixelData[j].r;
//      cb = Pattern->PixelData[j].b;
//      cg = Pattern->PixelData[j].g;
//      ca = Pattern->PixelData[j].a;
 //     cr = cp.Red;
      a = nsvg__div255((int)cover[0] * cp.Reserved);
      ia = 255 - a;
      // Premultiply
      r = nsvg__div255(cp.Red * a);
      g = nsvg__div255(cp.Green * a);
      b = nsvg__div255(cp.Blue * a);

      // Blend over
      r += nsvg__div255(ia * (int)dst[0]);
      g += nsvg__div255(ia * (int)dst[1]);
      b += nsvg__div255(ia * (int)dst[2]);
      a += nsvg__div255(ia * (int)dst[3]);

      dst[0] = (UINT8)r;
      dst[1] = (UINT8)g;
      dst[2] = (UINT8)b;
      dst[3] = (UINT8)a;

      cover++;
      dst += 4;
      fx += dx;
    }

  } else if (cache->type == NSVG_PAINT_CONIC_GRADIENT) {
    // TODO: spread modes.
    // TODO: plenty of opportunities to optimize.
    // TODO: focus (fx,fy)
    float fx, fy, gx, gy, gd;
    float* t = cache->xform;
    //    nsvg__dumpFloat("cache grad xform", t, 6);
    int cr, cg, cb, ca;
    unsigned int c;

    fx = (float)x;
    fy = (float)y;
    //     dx = 1.0f;
    gx = fx*t[0] + fy*t[2] + t[4];
    gy = fx*t[1] + fy*t[3] + t[5];

    for (int i = 0; i < count; i++) {
      int r,g,b,a,ia;
      if ((gx == 0.f) && (gy == 0.f)) {
        c = 0;
      } else {
        gd = (Atan2F(gy, gx) + PI) / PI2;
        c = cache->colors[dither(nsvg__clampf(gd*254.0f, 0, 253.99f), 1)];
      }
      cr = (c) & 0xff;
      cg = (c >> 8) & 0xff;
      cb = (c >> 16) & 0xff;
      ca = (c >> 24) & 0xff;

      a = nsvg__div255((int)cover[0] * ca);
      ia = 255 - a;

      // Premultiply
      r = nsvg__div255(cr * a);
      g = nsvg__div255(cg * a);
      b = nsvg__div255(cb * a);

      // Blend over
      r += nsvg__div255(ia * (int)dst[0]);
      g += nsvg__div255(ia * (int)dst[1]);
      b += nsvg__div255(ia * (int)dst[2]);
      a += nsvg__div255(ia * (int)dst[3]);

      dst[0] = (UINT8)r;
      dst[1] = (UINT8)g;
      dst[2] = (UINT8)b;
      dst[3] = (UINT8)a;

      cover++;
      dst += 4;
      //        fx += dx;
      gx += t[0];
      gy += t[1];
    }
  }
}

UINT8* nsvg__findStencil(NSVGrasterizer *r, int index)
{
  NSVGstencil* sl = r->stencilList;
  while (sl != NULL) {
    if (sl->index ==  index) return sl->square;
    sl = sl->next;
  }
  return NULL;
}

// Сортировка вставками для малых массивов (быстрее для < 32 элементов)
//static void nsvg__insertionSortEdges(NSVGedge* edges, int n)
//{
//    for (int i = 1; i < n; i++) {
//        NSVGedge key = edges[i];
//        int j = i - 1;
//
//        while (j >= 0 && edges[j].y0 > key.y0) {
//            edges[j + 1] = edges[j];
//            j--;
//        }
//        edges[j + 1] = key;
//    }
//}

// Гибридная сортировка: вставками для малых массивов, быстрая для больших
static void nsvg__hybridSortEdges(NSVGedge* edges, int left, int right)
{
    const int INSERTION_THRESHOLD = 32;

    if (right - left < INSERTION_THRESHOLD) {
        // Сортируем вставками
        for (int i = left + 1; i <= right; i++) {
            NSVGedge key = edges[i];
            int j = i - 1;

            while (j >= left && edges[j].y0 > key.y0) {
                edges[j + 1] = edges[j];
                j--;
            }
            edges[j + 1] = key;
        }
        return;
    }

    // Быстрая сортировка для больших массивов
    // Медиана из трех
    int mid = (left + right) / 2;

    if (edges[left].y0 > edges[mid].y0) {
        NSVGedge tmp = edges[left];
        edges[left] = edges[mid];
        edges[mid] = tmp;
    }
    if (edges[left].y0 > edges[right].y0) {
        NSVGedge tmp = edges[left];
        edges[left] = edges[right];
        edges[right] = tmp;
    }
    if (edges[mid].y0 > edges[right].y0) {
        NSVGedge tmp = edges[mid];
        edges[mid] = edges[right];
        edges[right] = tmp;
    }

    NSVGedge pivot = edges[mid];

    int i = left;
    int j = right;

    while (i <= j) {
        while (i <= right && edges[i].y0 < pivot.y0) i++;
        while (j >= left && edges[j].y0 > pivot.y0) j--;

        if (i <= j) {
            NSVGedge tmp = edges[i];
            edges[i] = edges[j];
            edges[j] = tmp;
            i++;
            j--;
        }
    }

    if (left < j) nsvg__hybridSortEdges(edges, left, j);
    if (i < right) nsvg__hybridSortEdges(edges, i, right);
}

static void nsvg__rasterizeSortedEdges(NSVGrasterizer *r,
    NSVGcachedPaint *cache, char fillRule, NSVGclipNode *clipList)
{
//  DBG("nsvg__rasterizeSortedEdges\n");
  NSVGactiveEdge *active = NULL;
  int e = 0;
  int maxWeight = (255 / NSVG__SUBSAMPLES);
  int xmin = 0, xmax = 0;

  // СОРТИРУЕМ РЕБРА СПЕЦИАЛИЗИРОВАННОЙ ФУНКЦИЕЙ (БЕЗ ВЫДЕЛЕНИЯ ПАМЯТИ)
  if (r->nedges > 1) {
      nsvg__hybridSortEdges(r->edges, 0, r->nedges - 1);
  }

  for (int y = 0; y < r->height; y++) {
    SetMem(r->scanline, r->width, 0);
    xmin = r->width;
    xmax = 0;

    for (int s = 0; s < NSVG__SUBSAMPLES; ++s) {
      float scany = (float) (y * NSVG__SUBSAMPLES + s) + 0.5f;
      NSVGactiveEdge **step = &active;

      // Удаляем завершенные ребра
      while (*step) {
        NSVGactiveEdge *z = *step;
        if (z->ey <= scany) {
          *step = z->next;
          nsvg__freeActive(r, z);
        }
        else {
          z->x += z->dx;
          step = &((*step)->next);
        }
      }
      // Добавляем новые ребра
      while (e < r->nedges && r->edges[e].y0 <= scany) {
        if (r->edges[e].y1 > scany) {
          NSVGactiveEdge *z = nsvg__addActive(r, &r->edges[e], scany);
          if (z == NULL) break;
          z->next = active;
          active = z;
        }
        e++;
      }

      // ПРОСТАЯ ПУЗЫРЬКОВАЯ СОРТИРОВКА (без выделения памяти)
      if (active) {
        NSVGactiveEdge *p;
        int changed;
        do {
          changed = 0;
          p = active;
          while (p && p->next) {
            if (p->x > p->next->x) {
              // ПРЯМОЙ ОБМЕН
              int x = p->x;
              int dx = p->dx;
              float ey = p->ey;
              int dir = p->dir;

              p->x = p->next->x;
              p->dx = p->next->dx;
              p->ey = p->next->ey;
              p->dir = p->next->dir;

              p->next->x = x;
              p->next->dx = dx;
              p->next->ey = ey;
              p->next->dir = dir;

              changed = 1;
            }
            p = p->next;
          }
        } while (changed);
      }

      if (active != NULL) {
        nsvg__fillActiveEdges(r->scanline, r->width, active, maxWeight, &xmin,
            &xmax, fillRule);
      }
    }

    if (xmin < 0) xmin = 0;
    if (xmax > r->width - 1) xmax = r->width - 1;

    if (xmin <= xmax) {
      if (clipList && r->stencil != NULL) {
        UINT8 *tempScanline = (UINT8*) AllocateZeroPool(r->width);
        if (!tempScanline) {
          r->fscanline(&r->bitmap[y * r->stride], xmax - xmin + 1,
              &r->scanline[xmin], xmin, y, cache);
          continue;
        }
        memcpy(tempScanline, r->scanline, r->width);
        NSVGclipNode *node = clipList;
        while (node) {
          if (node->index < r->stencilCount) {
            UINT8 *stencil = &r->stencil[r->stencilSize * node->index
                + y * r->stencilStride];
            if (stencil) {
              for (int j = xmin; j <= xmax; j++) {
                if (tempScanline[j] == 0) continue;
                if ((stencil[j / 8] & (1 << (j % 8))) == 0) {
                  tempScanline[j] = 0;
                }
              }
            }
          }
          node = node->next;
        }
        memcpy(r->scanline, tempScanline, r->width);
        FreePool(tempScanline);
      }
      r->fscanline(&r->bitmap[y * r->stride], xmax - xmin + 1,
          &r->scanline[xmin], xmin, y, cache);
    }
  }
}

static void nsvg__unpremultiplyAlpha(UINT8* image, int w, int h, int stride)
{
  // Unpremultiply
  for (int y = 0; y < h; y++) {
    UINT8 *row = &image[y*stride];
    for (int x = 0; x < w; x++) {
      int r = row[0], g = row[1], b = row[2], a = row[3];
      if (a != 0) {
        row[0] = (UINT8)(r*255/a);
        row[1] = (UINT8)(g*255/a);
        row[2] = (UINT8)(b*255/a);
      }
      row += 4;
    }
  }

  // Defringe
  for (int y = 0; y < h; y++) {
    UINT8 *row = &image[y*stride];
    for (int x = 0; x < w; x++) {
      int r = 0, g = 0, b = 0, a = row[3], n = 0;
      if (a == 0) {
        if (x-1 > 0 && row[-1] != 0) {
          r += row[-4];
          g += row[-3];
          b += row[-2];
          n++;
        }
        if (x+1 < w && row[7] != 0) {
          r += row[4];
          g += row[5];
          b += row[6];
          n++;
        }
        if (y-1 > 0 && row[-stride+3] != 0) {
          r += row[-stride];
          g += row[-stride+1];
          b += row[-stride+2];
          n++;
        }
        if (y+1 < h && row[stride+3] != 0) {
          r += row[stride];
          g += row[stride+1];
          b += row[stride+2];
          n++;
        }
        if (n > 0) {
          row[0] = (UINT8)(r/n);
          row[1] = (UINT8)(g/n);
          row[2] = (UINT8)(b/n);
        }
      }
      row += 4;
    }
  }
}


static void nsvg__initPaint(NSVGcachedPaint* cache, NSVGpaint* paint, NSVGshape* shape, float *xformShape)
{
  NSVGgradient* grad = paint->paint.gradient;

  float opacity = shape->opacity;
  cache->type = paint->type;

  if (cache->type == NSVG_PAINT_COLOR) {
    cache->colors[0] = nsvg__applyOpacity(paint->paint.color, opacity);
    return;
  }
  if (grad) {
    cache->coarse = grad->ditherCoarse;
  }
  if (cache->type == NSVG_PAINT_PATTERN) {
    cache->colors[0] = nsvg__applyOpacity(0, opacity);
    if (grad) {
      cache->image = ((NSVGpattern*)grad)->image;
    }
    float xform[6];
    nsvg__xformIdentity(xform);
    xform[0] = shape->bounds[2] - shape->bounds[0];
    xform[3] = shape->bounds[3] - shape->bounds[1];
    xform[4] = shape->bounds[0];
    xform[5] = shape->bounds[1];
    nsvg__xformMultiply(xform, xformShape);
    nsvg__xformInverse(cache->xform, xform);
    return;
  }

  cache->spread = grad->spread;
  nsvg__xformInverse(cache->xform, xformShape);
  nsvg__xformMultiply(cache->xform, grad->xform);

  if (grad->nstops == 0) {
    SetMem(cache->colors, sizeof(cache->colors), 0);
  } else if (grad->nstops == 1) {
    for (int i = 0; i < 256; i++) {
      cache->colors[i] = nsvg__applyOpacity(grad->stops[i].color, opacity);
    }
  } else {  //nstops=2 as usual gradient
    unsigned int ca, cb = 0;
    float ua, ub, du, u;
    int ia, ib, count;

    ca = nsvg__applyOpacity(grad->stops[0].color, opacity);
    ua = nsvg__clampf(grad->stops[0].offset, 0, 1);
    ub = nsvg__clampf(grad->stops[grad->nstops-1].offset, ua, 1);
    ia = (int)(ua * 255.0f);
    ib = (int)(ub * 255.0f);
    for (int i = 0; i < ia; i++) {
      cache->colors[i] = ca; //color from stop0
    }

    for (int i = 0; i < grad->nstops-1; i++) {
      ca = grad->stops[i].color;
      cb = grad->stops[i+1].color;
      ua = nsvg__clampf(grad->stops[i].offset, 0, 1); //=0
      ub = nsvg__clampf(grad->stops[i+1].offset, 0, 1); //=1
      ia = (int)(ua * 255.0f);  //=0
      ib = (int)(ub * 255.0f);  //=255
      count = ib - ia;
      if (count <= 0) continue;
      u = 0;
      du = 1.0f / (float)count;
      for (int j = 0; j < count; j++) {
        cache->colors[ia+j] = nsvg__lerpRGBA(ca,cb,u, opacity);
        u += du;
      }
    }

    for (int i = ib; i < 256; i++) { //tail
      cache->colors[i] = cb;
    }
  }
}



static void nsvg__rasterizeShapes(NSVGrasterizer* r,
                                  NSVGshape* shapes, const char* groupName,
                                  float tx, float ty, float scalex, float scaley,
                                  UINT8* dst, int w, int h, int stride,
                                  NSVGscanlineFunction fscanline)
{
  NSVGshape *shape = NULL, *shapeLink = NULL;
  float xform[6], xform2[6];
  float min_scale = fabsf(scalex) < fabsf(scaley) ? fabsf(scalex) : fabsf(scaley);

  r->bitmap = dst;
  r->width = w;
  r->height = h;
  r->stride = stride;
  r->fscanline = fscanline;

  if (w > r->cscanline) {
    int oldw = r->cscanline;
    r->cscanline = w;
    if (oldw == 0) {
      r->scanline = (UINT8*)AllocatePool(w);
    } else {
      r->scanline = (UINT8*)ReallocatePool(oldw, w, r->scanline);
    }
    if (r->scanline == NULL) return;
  }

  nsvg__xformSetScale(&xform2[0], scalex, scaley);
  xform2[4] = tx; xform2[5] = ty;

  for (shape = shapes; shape != NULL; shape = shape->next) {
    if (!(shape->flags & NSVG_VIS_VISIBLE))
      continue;
    if ( groupName && !nsvg__isShapeInGroup(shape, groupName) ) {
      continue;
    }

    memcpy(&xform[0], shape->xform, sizeof(float)*6);

    xform[0] *= scalex;
    xform[1] *= scaley;
    xform[2] *= scalex;
    xform[3] *= scaley;
    xform[4] = xform[4] * scalex + tx;
    xform[5] = xform[5] * scaley + ty;

    if (!shape->link) {
      renderShape(r, shape, &xform[0], min_scale);
    }
    shapeLink = shape->link;  //this is <use>
    while (shapeLink) {
      memcpy(&xform2[0], &xform[0], sizeof(float)*6);
      nsvg__xformPremultiply(&xform2[0], shapeLink->xform);
      renderShape(r, shapeLink, &xform2[0], min_scale);
      if (!shape->isSymbol) {
        break;
      }
      shapeLink = shapeLink->next;
    }
  }

  r->bitmap = NULL;
  r->width = 0;
  r->height = 0;
  r->stride = 0;
  r->fscanline = NULL;
}

static void renderShape(NSVGrasterizer* r,
                        NSVGshape* shape, float *xform, float min_scale)
{
  NSVGedge *e = NULL;
  NSVGcachedPaint cache;

  SetMem(&cache, sizeof(NSVGcachedPaint), 0);

    // Для stencil-буфера используем EVENODD
  char fillRule = shape->fillRule;
  if (shape->clipList) {
      fillRule = NSVG_FILLRULE_EVENODD;
  }

  if (shape->fill.type != NSVG_PAINT_NONE) {
    nsvg__resetPool(r);
    r->freelist = NULL;
    r->nedges = 0;

    nsvg__flattenShape(r, shape, xform);
    // Scale and translate edges
    for (int i = 0; i < r->nedges; i++) {
      e = &r->edges[i];
      e->y0 *= NSVG__SUBSAMPLES;
      e->y1 *= NSVG__SUBSAMPLES;
    }

    // Rasterize edges
    QuickSort(r->edges, r->nedges, sizeof(NSVGedge), compare_edges);

    // now, traverse the scanlines and find the intersections on each scanline, use non-zero rule
    nsvg__initPaint(&cache, &shape->fill, shape, xform);
    nsvg__rasterizeSortedEdges(r, &cache, fillRule, shape->clipList);
  }
  // Рендерим stroke (для текста это обводка)
  if (shape->stroke.type != NSVG_PAINT_NONE && (shape->strokeWidth * min_scale) > 0.01f) {
    nsvg__resetPool(r);
    r->freelist = NULL;
    r->nedges = 0;
    nsvg__flattenShapeStroke(r, shape, xform);

    // Scale and translate edges
    for (int i = 0; i < r->nedges; i++) {
      e = &r->edges[i];
      e->y0 *= NSVG__SUBSAMPLES;
      e->y1 *= NSVG__SUBSAMPLES;
    }

    // Rasterize edges
    QuickSort(r->edges, r->nedges, sizeof(NSVGedge), compare_edges);

    // now, traverse the scanlines and find the intersections on each scanline, use non-zero rule
    nsvg__initPaint(&cache, &shape->stroke, shape, xform);
    nsvg__rasterizeSortedEdges(r, &cache, NSVG_FILLRULE_NONZERO, shape->clipList);
  }
}

// Специальная функция для рендеринга stencil-буфера
// Использует EVENODD для правильного вырезания дырок
static void renderShapeForStencil(NSVGrasterizer* r,
                                  NSVGshape* shape, 
                                  float tx, float ty, float scalex, float scaley,
                                  UINT8* stencil, int w, int h, int stride)
{
  NSVGedge *e = NULL;
  NSVGcachedPaint cache;
  float xform[6];

    // Сохраняем ID текущего shape
    strncpy(r->currentShapeId, shape->id, 63);
    r->currentShapeId[63] = '\0';
  
  // Устанавливаем трансформацию
  nsvg__xformSetScale(&xform[0], scalex, scaley);
  xform[4] = tx; 
  xform[5] = ty;
  
  // Умножаем на трансформацию самого shape
  nsvg__xformPremultiply(&xform[0], shape->xform);
  
  // Для stencil всегда используем EVENODD
  char fillRule = shape->fillRule;

  // Dummy clip - чтобы stencil не применялся рекурсивно
  //  NSVGclip dummyClip = {0};
  //  dummyClip.count = 0;
  
  // Настраиваем растеризатор для stencil-буфера
  r->bitmap = stencil;
  r->width = w;
  r->height = h;
  r->stride = stride;
  r->fscanline = nsvg__scanlineBit;
  
  if (w > r->cscanline) {
    int oldw = r->cscanline;
    r->cscanline = w;
    if (oldw == 0) {
      r->scanline = (UINT8*)AllocatePool(w);
    } else {
      r->scanline = (UINT8*)ReallocatePool(oldw, w, r->scanline);
    }
    if (r->scanline == NULL) return;
  }

  SetMem(&cache, sizeof(NSVGcachedPaint), 0);

  if (shape->fill.type != NSVG_PAINT_NONE) {
    nsvg__resetPool(r);
    r->freelist = NULL;
    r->nedges = 0;

    nsvg__flattenShape(r, shape, xform);

    for (int i = 0; i < r->nedges; i++) {
      e = &r->edges[i];
      e->y0 *= NSVG__SUBSAMPLES;
      e->y1 *= NSVG__SUBSAMPLES;
    }

    // Используем compare_edges для сортировки
    if (r->nedges > 1) {
        nsvg__hybridSortEdges(r->edges, 0, r->nedges - 1);
    }

    nsvg__initPaint(&cache, &shape->fill, shape, xform);
    // Используем dummyClip - без применения stencil
    nsvg__rasterizeSortedEdges(r, &cache, fillRule, NULL);

  }
}

void nsvg__rasterizeClipPaths(
                              NSVGrasterizer* r, NSVGimage* image, int w, int h,
                              float tx, float ty, float scalex, float scaley)
{
  int clipPathCount = 0;

  NSVGclipPath* clipPath = image->clipPaths;
  if (clipPath == NULL) {
    r->stencil = NULL;
    r->stencilCount = 0;
    return;
  }

  while (clipPath != NULL) {
    clipPathCount++;
    clipPath = clipPath->next;
  }

  r->stencilStride = w / 8 + (w % 8 != 0 ? 1 : 0);
  r->stencilSize = h * r->stencilStride;

  UINTN totalSize = r->stencilSize * clipPathCount;

  // Ограничиваем размер для дракона, но оставляем рабочим для клевера
  if (totalSize > 2 * 1024 * 1024) {  // 2 МБ максимум
    DBG("WARNING: Stencil buffer too large (%lld bytes), limiting\n", totalSize);
    clipPathCount = (2 * 1024 * 1024) / r->stencilSize;
    if (clipPathCount < 1) clipPathCount = 1;
    totalSize = r->stencilSize * clipPathCount;
  }

  //DBG("STENCIL: allocating %lld bytes for %d clipPaths\n", totalSize, clipPathCount);

  r->stencil = (unsigned char*)AllocateZeroPool(totalSize);
  if (r->stencil == NULL) {
    DBG("ERROR: Failed to allocate stencil buffer!\n");
    r->stencil = NULL;
    r->stencilCount = 0;
    return;
  }
  r->stencilCount = clipPathCount;

  // Рендерим каждый clipPath в свой участок буфера
  clipPath = image->clipPaths;
  int idx = 0;
  while (clipPath != NULL && idx < clipPathCount) {
    NSVGshape* shape = clipPath->shapes;
    while (shape != NULL) {
      renderShapeForStencil(r,
                          shape,
                          tx, ty, scalex, scaley,
                          &r->stencil[r->stencilSize * idx],
                          w, h, r->stencilStride);
      shape = shape->next;
    }
    clipPath = clipPath->next;
    idx++;
  }
}

void nsvgRasterize(NSVGrasterizer* r,
                   NSVGimage* image, float tx, float ty, float scalex, float scaley,
                   UINT8* dst, int w, int h, int stride)
{
  nsvgRasterize(r, image, &image->realBounds[0], NULL, tx, ty, scalex, scaley, dst, w, h, stride);
}

void nsvgRasterize(NSVGrasterizer* r,
                   NSVGimage* image, float* bounds, const char* groupName,
                   float tx, float ty, float scalex, float scaley,
                   UINT8* dst, int w, int h, int stride)
{
  tx -= bounds[0] * scalex;
  ty -= bounds[1] * scaley;

  nsvg__rasterizeClipPaths(r, image, w, h, tx, ty, scalex, scaley);
  nsvg__rasterizeShapes(r, image->shapes, groupName, tx, ty, scalex, scaley,
                        dst, w, h, stride, nsvg__scanlineSolid);
  nsvg__unpremultiplyAlpha(dst, w, h, stride);
}

