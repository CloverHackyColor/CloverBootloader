#include "XImage.h"
#include "lodepng.h"
#include "nanosvg.h"

#ifndef DEBUG_ALL
#define DEBUG_XIMAGE 1
#else
#define DEBUG_XIMAGE DEBUG_ALL
#endif

#if DEBUG_XIMAGE == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_XIMAGE, __VA_ARGS__)
#endif


XImage::XImage()
{
  Width = 0;
  Height = 0;
}

XImage::XImage(UINTN W, UINTN H)
{
  Width = W;
  Height = H;
  PixelData.CheckSize(GetWidth()*GetHeight());
}

XImage::XImage(EG_IMAGE* egImage)
{
  if ( egImage) {
	  Width = egImage->Width;
	  Height = egImage->Height;
  }else{
	  Width = 0;
	  Height = 0;
  }
  PixelData.CheckSize(GetWidth()*GetHeight()); // change the allocated size, but not the size.
  PixelData.SetLength(GetWidth()*GetHeight()); // change the size, ie the number of element in the array
  if ( GetWidth()*GetHeight() > 0 ) {
	  CopyMem(&PixelData[0], egImage->PixelData, PixelData.size() * sizeof(*egImage->PixelData));
  }
}

UINT8 Smooth(const UINT8* p, int a01, int a10, int a21, int a12,  int dx, int dy, float scale)
{
  return (UINT8)((*(p + a01) * (scale - dx) * 3.f + *(p + a10) * (scale - dy) * 3.f + *(p + a21) * dx * 3.f +
    *(p + a12) * dy * 3.f + *(p) * 2.f *scale) / (scale * 8.f));
}

XImage::XImage(const XImage& Image, float scale)
{
  UINTN SrcWidth = Image.GetWidth();
  UINTN SrcHeight = Image.GetHeight();

  if (scale < 1.e-4) {
    Width = SrcWidth;
    Height = SrcHeight;
    PixelData.CheckSize(GetWidth()*GetHeight());
    PixelData.SetLength(GetWidth()*GetHeight());
    for (UINTN y = 0; y < Height; ++y)
      for (UINTN x = 0; x < Width; ++x)
        PixelData[y * Width + x] = Image.GetPixel(x, y);

  } else {
    Width = (UINTN)(SrcWidth * scale);
    Height = (UINTN)(SrcHeight * scale);
    PixelData.CheckSize(Width * Height);
    PixelData.SetLength(Width * Height);
    CopyScaled(Image, scale);
  }
}

#if 0
  UINTN Offset = OFFSET_OF(EFI_GRAPHICS_OUTPUT_BLT_PIXEL, Blue);

  dst.Blue = Smooth(&src.Blue, a01, a10, a11, a21, a12, dx, dy, scale);

#define SMOOTH(P) \
do { \
    ((PIXEL*)dst_ptr)->P = (BYTE)((a01.P * (cx - dx) * 3 + a10.P * (cy - dy) * 3 + \
                            a21.P * dx * 3 + a12.P * dy * 3 + a11.P * (cx + cy)) / ((cx + cy) * 4)); \
} while(0)

  UINT x, y, z;
  PIXEL a10, a11, a12, a01, a21;
  int  fx, cx, lx, dx, fy, cy, ly, dy;
  
  fx = (dst_size->width << PRECISION) / src_size->width;
  fy = (dst_size->height << PRECISION) / src_size->height;
  if (!fx || !fy) {
   return;
   
  }
  
  cx = ((fx - 1) >> PRECISION) + 1;
  cy = ((fy - 1) >> PRECISION) + 1;
  
  for (z = 0; z < dst_size->depth; z++)
  {
    BYTE * dst_slice_ptr = dst + z * dst_slice_pitch;
    const BYTE *src_slice_ptr = src + src_slice_pitch * (z * src_size->depth / dst_size->depth);
    
      for (y = 0; y < dst_size->height; y++)
       {
      BYTE * dst_ptr = dst_slice_ptr + y * dst_row_pitch;
      const BYTE *src_row_ptr = src_slice_ptr + src_row_pitch * (y * src_size->height / dst_size->height);
      ly = (y << PRECISION) / fy;
      dy = y - ((ly * fy) >> PRECISION);
      
      for (x = 0; x < dst_size->width; x++)
      {
        const BYTE *src_ptr = src_row_ptr + (x * src_size->width / dst_size->width) * src_format->bytes_per_pixel;
        
        lx = (x << PRECISION) / fx;
        dx = x - ((lx * fx) >> PRECISION);
        
        a11 = *(PIXEL*)src_ptr;
        a10 = (y == 0) ? a11 : (*(PIXEL*)(src_ptr - src_row_pitch));
        a01 = (x == 0) ? a11 : (*(PIXEL*)(src_ptr - src_format->bytes_per_pixel));
        a21 = (x == dst_size->width) ? a11 : (*(PIXEL*)(src_ptr + src_format->bytes_per_pixel));
        a12 = (y == dst_size->height) ? a11 : (*(PIXEL*)(src_ptr + src_row_pitch));
        
        SMOOTH(r);
        SMOOTH(g);
        SMOOTH(b);
        SMOOTH(a);
        
        dst_ptr += dst_format->bytes_per_pixel;
        }
      }
    }
#endif


XImage::~XImage()
{
}

const XArray<EFI_GRAPHICS_OUTPUT_BLT_PIXEL>& XImage::GetData() const
{
  return PixelData;
}

EFI_GRAPHICS_OUTPUT_BLT_PIXEL* XImage::GetPixelPtr(UINTN x, UINTN y)
{
	return &PixelData[x + y * Width];
}

const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& XImage::GetPixel(UINTN x, UINTN y) const
{
	return PixelData[x + y * Width];
}


UINTN      XImage::GetWidth() const
{
  return Width;
}

UINTN      XImage::GetHeight() const
{
  return Height;
}

UINTN XImage::GetSize() const
{
  return Width * Height * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
}

void XImage::Fill(const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& Color)
{
  for (UINTN y = 0; y < Height; ++y)
    for (UINTN x = 0; x < Width; ++x)
      PixelData[y * Width + x] = Color;
}

void XImage::FillArea(const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& Color, const EgRect& Rect)
{
  for (UINTN y = Rect.Ypos; y < Height && (y - Rect.Ypos) < Rect.Height; ++y) {
//    EFI_GRAPHICS_OUTPUT_BLT_PIXEL* Ptr = PixelData + y * Width + Rect.Xpos;
    for (UINTN x = Rect.Xpos; x < Width && (x - Rect.Xpos) < Rect.Width; ++x)
//      *Ptr++ = Color;
      PixelData[y * Width + x] = Color;
  }
}

void XImage::CopyScaled(const XImage& Image, float scale)
{
  UINTN SrcWidth = Image.GetWidth();

  int Pixel = sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  int Row = (int)SrcWidth * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);

  const XArray<EFI_GRAPHICS_OUTPUT_BLT_PIXEL>& Source = Image.GetData();

  for (UINTN y = 0; y < Height; y++)
  {
    int ly = (int)(y / scale);
    int dy = (int)(y - ly * scale);
    for (UINTN x = 0; x < Width; x++)
    {
      int lx = (int)(x / scale);
      int dx = (int)(x - lx * scale);
      int a01 = (x == 0) ? 0 : -Pixel;
      int a10 = (y == 0) ? 0 : -Row;
      int a21 = (x == Width - 1) ? 0 : Pixel;
      int a12 = (y == Height - 1) ? 0 : Row;
      EFI_GRAPHICS_OUTPUT_BLT_PIXEL& dst = *GetPixelPtr(x, y);
      dst.Blue = Smooth(&Source[lx + ly * SrcWidth].Blue, a01, a10, a21, a12, dx, dy, scale);
      dst.Green = Smooth(&Source[lx + ly * SrcWidth].Green, a01, a10, a21, a12, dx, dy, scale);
      dst.Red = Smooth(&Source[lx + ly * SrcWidth].Red, a01, a10, a21, a12, dx, dy, scale);
      dst.Reserved = Source[lx + ly * SrcWidth].Reserved;
    }
  }
}

/* Place Top image over this image at PosX,PosY
 * Lowest means final image is opaque
 * else transparency will be multiplied
 */
void XImage::Compose(INTN PosX, INTN PosY, const XImage& TopImage, bool Lowest)
{
  UINT32      TopAlpha;
  UINT32      RevAlpha;
  UINT32      FinalAlpha;
  UINT32      Temp;

  for (UINTN y = PosY; y < Height && (y - PosY) < TopImage.GetHeight(); ++y) {
 //   EFI_GRAPHICS_OUTPUT_BLT_PIXEL& CompPtr = *GetPixelPtr(PosX, y); // I assign a ref to avoid the operator ->. Compiler will produce the same anyway.
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL* CompPtr = GetPixelPtr(PosX, y);
    for (UINTN x = PosX; x < Width && (x - PosX) < TopImage.GetWidth(); ++x) {
      TopAlpha = TopImage.GetPixel(x-PosX, y-PosY).Reserved;
      RevAlpha = 255 - TopAlpha;
      FinalAlpha = (255*255 - RevAlpha*(255 - CompPtr->Reserved)) / 255;

//final alpha =(1-(1-x)*(1-y)) =(255*255-(255-topA)*(255-compA))/255
      Temp = (CompPtr->Blue * RevAlpha) + (TopImage.GetPixel(x-PosX, y-PosY).Blue * TopAlpha);
      CompPtr->Blue = (UINT8)(Temp / 255);

      Temp = (CompPtr->Green * RevAlpha) + (TopImage.GetPixel(x-PosX, y-PosY).Green * TopAlpha);
      CompPtr->Green = (UINT8)(Temp / 255);

      Temp = (CompPtr->Red * RevAlpha) + (TopImage.GetPixel(x-PosX, y-PosY).Red * TopAlpha);
      CompPtr->Red = (UINT8)(Temp / 255);

      if (Lowest) {
        CompPtr->Reserved = 255;
      } else {
        CompPtr->Reserved = (UINT8)FinalAlpha;
      }
      CompPtr++; //faster way to move to next pixel
    }
  }
}

void XImage::FlipRB(bool WantAlpha)
{
  UINTN ImageSize = (Width * Height);
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL* Pixel = GetPixelPtr(0,0);
  for (UINTN i = 0; i < ImageSize; ++i) {
    UINT8 Temp = Pixel->Blue;
    Pixel->Blue = Pixel->Red;
    Pixel->Red = Temp;
    if (!WantAlpha) Pixel->Reserved = 0xFF;
    Pixel++;
  }
}

/*
 * The function converted plain array into XImage object
 */
unsigned XImage::FromPNG(const UINT8 * Data, UINTN Length)
{
  UINT8 * PixelPtr = (UINT8 *)&PixelData[0];
  unsigned Error = eglodepng_decode(&PixelPtr, &Width, &Height, Data, Length);

  FlipRB(true);
  return Error;
}

/*
 * The function creates new array Data and inform about it size to be saved
 * as a file.
 * The caller is responsible to free the array.
 */

unsigned XImage::ToPNG(UINT8** Data, UINTN& OutSize)
{
  size_t           FileDataLength = 0;
  FlipRB(false);
  UINT8 * PixelPtr = (UINT8 *)&PixelData[0];
  unsigned Error = eglodepng_encode(Data, &FileDataLength, PixelPtr, Width, Height);
  OutSize = FileDataLength;
  return Error;
}

/*
 * fill XImage object by raster data described in SVG
 * caller should create the object with Width and Height and calculate scale
 * scale = 1 correspond to fill the rect with the image
 * scale = 0.5 will reduce image 
 */
unsigned XImage::FromSVG(const CHAR8 *SVGData, UINTN FileDataLength, float scale)
{
  NSVGimage       *SVGimage;
  NSVGparser* p;

  NSVGrasterizer* rast = nsvgCreateRasterizer();
  if (!rast) return 1;
  char *input = (__typeof__(input))AllocateCopyPool(AsciiStrSize(SVGData), SVGData);
  if (!input) return 2;

  p = nsvgParse(input, 72, 1.f); //the parse will change input contents
  SVGimage = p->image;
  if (SVGimage) {
    float ScaleX = Width / SVGimage->width;
    float ScaleY = Height / SVGimage->height;
    float Scale = (ScaleX > ScaleY) ? ScaleY : ScaleX;
    Scale *= scale;

    DBG("Test image width=%d heigth=%d\n", (int)(SVGimage->width), (int)(SVGimage->height));
    nsvgRasterize(rast, SVGimage, 0.f, 0.f, Scale, Scale, (UINT8*)&PixelData[0], (int)Width, (int)Height, (int)Width * sizeof(PixelData[0]));
    FreePool(SVGimage);
  }
  nsvg__deleteParser(p);
  nsvgDeleteRasterizer(rast);
  FreePool(input);
  return 0;
}

// Screen operations
/*
 * The function to get image from screen. Used in  screenshot (full screen), Pointer (small area) and Draw (small area)
 * XImage must be created with Width, Height of Rect
 * the rect will be clipped if it intersects the screen edge
 *
 * be careful about alpha. This procedure can produce alpha = 0 which means full transparent
 */
void XImage::GetArea(const EG_RECT& Rect)
{
  GetArea(Rect.XPos, Rect.YPos, Rect.Width, Rect.Height);
}


void XImage::GetArea(INTN x, INTN y, UINTN W, UINTN H)
{
  EFI_STATUS Status;
  EFI_GUID UgaDrawProtocolGuid = EFI_UGA_DRAW_PROTOCOL_GUID;
  EFI_UGA_DRAW_PROTOCOL *UgaDraw = NULL;
  EFI_GUID GraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput = NULL;

  Status = EfiLibLocateProtocol(&GraphicsOutputProtocolGuid, (VOID **)&GraphicsOutput);
  if (EFI_ERROR(Status)) {
    GraphicsOutput = NULL;
    Status = EfiLibLocateProtocol(&UgaDrawProtocolGuid, (VOID **)&UgaDraw);
    if (EFI_ERROR(Status))
      UgaDraw = NULL;
  }

  if (W == 0) W = Width;
  if (H == 0) H = Height;

  Width = (x + W > (UINTN)UGAWidth) ? (UGAWidth - x) : W;
  Height = (y + H > (UINTN)UGAHeight) ? ((UINTN)UGAHeight - y) : H;

  PixelData.SetLength(Width * Height); // setLength BEFORE, so &PixelData[0]

  if (GraphicsOutput != NULL) {
    GraphicsOutput->Blt(GraphicsOutput,
      (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)&PixelData[0],
      EfiBltVideoToBltBuffer,
      x, y, 0, 0, Width, Height, 0);
  }
  else if (UgaDraw != NULL) {
    UgaDraw->Blt(UgaDraw,
      (EFI_UGA_PIXEL *)GetPixelPtr(0,0),
      EfiUgaVideoToBltBuffer,
      x, y, 0, 0, Width, Height, 0);
  }
}

void XImage::Draw(INTN x, INTN y, float scale)
{
  //prepare images
//  DBG("1\n");
  XImage Top(*this, scale);
//  DBG("2\n");
  XImage Background(Width, Height);
//  DBG("3\n");
  Background.GetArea(x, y, Width, Height);
//  DBG("4\n");
  Background.Compose(0, 0, Top, true);
//  DBG("5\n");
  UINTN AreaWidth = (x + Width > (UINTN)UGAWidth) ? (UGAWidth - x) : Width;
  UINTN AreaHeight = (y + Height > (UINTN)UGAHeight) ? (UGAHeight - y) : Height;
//  DBG("area=%d,%d\n", AreaWidth, AreaHeight);
  // prepare protocols
  EFI_STATUS Status;
  EFI_GUID UgaDrawProtocolGuid = EFI_UGA_DRAW_PROTOCOL_GUID;
  EFI_UGA_DRAW_PROTOCOL *UgaDraw = NULL;
  EFI_GUID GraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput = NULL;

  Status = EfiLibLocateProtocol(&GraphicsOutputProtocolGuid, (VOID **)&GraphicsOutput);
  if (EFI_ERROR(Status)) {
    GraphicsOutput = NULL;
    Status = EfiLibLocateProtocol(&UgaDrawProtocolGuid, (VOID **)&UgaDraw);
    if (EFI_ERROR(Status))
      UgaDraw = NULL;
  }
  //output combined image
  if (GraphicsOutput != NULL) {
    GraphicsOutput->Blt(GraphicsOutput, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)Background.GetPixelPtr(0, 0),
      EfiBltBufferToVideo,
      0, 0, x, y, AreaWidth, AreaHeight, 0);
  }
  else if (UgaDraw != NULL) {
    UgaDraw->Blt(UgaDraw, (EFI_UGA_PIXEL *)Background.GetPixelPtr(0, 0), EfiUgaBltBufferToVideo,
      0, 0, x, y, AreaWidth, AreaHeight, 0);
  }
}
