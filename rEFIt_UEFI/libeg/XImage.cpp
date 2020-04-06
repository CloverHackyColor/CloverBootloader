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

EFI_GRAPHICS_OUTPUT_BLT_PIXEL NullColor = {0,0,0,0};

XImage::XImage()
{
  Width = 0;
  Height = 0;
}

XImage::XImage(UINTN W, UINTN H)
{
//  Width = W;
//  Height = H; //included below
  setSizeInPixels(W, H);
}

XImage::XImage(EG_IMAGE* egImage)
{
  if ( egImage) {
//	  Width = egImage->Width;
//	  Height = egImage->Height;
	  setSizeInPixels(egImage->Width, egImage->Height); // change the size, ie the number of element in the array. Reaalocate buffer if needed
	  CopyMem(&PixelData[0], egImage->PixelData, GetSizeInBytes());
  }else{
//	  Width = 0;
//	  Height = 0;
	  setSizeInPixels(0, 0); // change the size, ie the number of element in the array. Reallocate buffer if needed
  }
}

EFI_STATUS XImage::FromEGImage(const EG_IMAGE* egImage)
{
  if ( egImage) {
    setSizeInPixels(egImage->Width, egImage->Height);
    CopyMem(&PixelData[0], egImage->PixelData, GetSizeInBytes());
  } else {
    setSizeInPixels(0, 0);
  }
  if (GetSizeInBytes() == 0) {
    return EFI_NOT_FOUND;
  }
  return EFI_SUCCESS;
}

XImage& XImage::operator= (const XImage& other)
{
	setSizeInPixels(other.GetWidth(), other.GetHeight()); // change the size, ie the number of element in the array. Reaalocate buffer if needed
	PixelData = other.PixelData;
	return *this;
}

UINT8 XImage::Smooth(const UINT8* p, int a01, int a10, int a21, int a12,  int dx, int dy, float scale)
{
  return (UINT8)((*(p + a01) * (scale - dx) * 3.f + *(p + a10) * (scale - dy) * 3.f + *(p + a21) * dx * 3.f +
    *(p + a12) * dy * 3.f + *(p) * 2.f *scale) / (scale * 8.f));
}

XImage::XImage(const XImage& Image, float scale)
{
  UINTN SrcWidth = Image.GetWidth();
  UINTN SrcHeight = Image.GetHeight();

  if (scale < 1.e-4) {
//    Width = SrcWidth;
//    Height = SrcHeight;
    setSizeInPixels(SrcWidth, SrcHeight);
    for (UINTN y = 0; y < Height; ++y)
      for (UINTN x = 0; x < Width; ++x)
        PixelData[y * Width + x] = Image.GetPixel(x, y);

  } else {
//    Width = (UINTN)(SrcWidth * scale);
//    Height = (UINTN)(SrcHeight * scale);
    setSizeInPixels((UINTN)(SrcWidth * scale), (UINTN)(SrcHeight * scale));
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

EFI_GRAPHICS_OUTPUT_BLT_PIXEL* XImage::GetPixelPtr(INTN x, INTN y)
{
	return &PixelData[x + y * Width];
}

const EFI_GRAPHICS_OUTPUT_BLT_PIXEL* XImage::GetPixelPtr(INTN x, INTN y) const
{
	return &PixelData[x + y * Width];
}

const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& XImage::GetPixel(INTN x, INTN y) const
{
	return PixelData[x + y * Width];
}

/*
UINTN      XImage::GetWidth() const
{
  return Width;
}

UINTN      XImage::GetHeight() const
{
  return Height;
}
*/
UINTN XImage::GetSizeInBytes() const
{
  return PixelData.size() * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
}

void XImage::setSizeInPixels(UINTN W, UINTN H) //unused arguments?
{
  Width = W;
  Height = H;
	PixelData.setSize(Width * Height);
}

void XImage::Fill(const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& Color)
{
  for (UINTN y = 0; y < Height; ++y)
    for (UINTN x = 0; x < Width; ++x)
      PixelData[y * Width + x] = Color;
}

void XImage::Fill(const EG_PIXEL* Color)
{
  Fill((const EFI_GRAPHICS_OUTPUT_BLT_PIXEL&)Color);
}

void XImage::FillArea(const EG_PIXEL* Color, EG_RECT& Rect)
{
  FillArea((const EFI_GRAPHICS_OUTPUT_BLT_PIXEL&)*Color, Rect);
}

void XImage::FillArea(const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& Color, EG_RECT& Rect)
{
  for (INTN y = Rect.YPos; y < GetHeight() && (y - Rect.YPos) < Rect.Height; ++y) {
    for (INTN x = Rect.XPos; x < GetWidth() && (x - Rect.XPos) < Rect.Width; ++x) {
      PixelData[y * Width + x] = Color;
    }
  }
}

//sizes remain as were assumed input image is large enough?
void XImage::CopyScaled(const XImage& Image, float scale)
{
  int SrcWidth = (int)Image.GetWidth(); //because Source[] requires int argument, why not long long int?

  int Pixel = sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  int Row = SrcWidth * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  int W = (int)GetWidth();
  int H = (int)GetHeight();

  const XArray<EFI_GRAPHICS_OUTPUT_BLT_PIXEL>& Source = Image.GetData();

  for (INTN y = 0; y < H; y++)
  {
    int ly = (int)(y / scale);
    int dy = (int)(y - ly * scale);
    for (INTN x = 0; x < W; x++)
    {
      int lx = (int)(x / scale);
      int dx = (int)(x - lx * scale);
      int a01 = (x == 0) ? 0 : -Pixel;
      int a10 = (y == 0) ? 0 : -Row;
      int a21 = (x == W - 1) ? 0 : Pixel;
      int a12 = (y == H - 1) ? 0 : Row;
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
  UINT32      CompAlpha;
  UINT32      TempAlpha;
  UINT32      Temp;
//change only affected pixels
  for (INTN y = PosY; y < GetHeight() && (y - PosY) < TopImage.GetHeight(); ++y) {
 //   EFI_GRAPHICS_OUTPUT_BLT_PIXEL& CompPtr = *GetPixelPtr(PosX, y); // I assign a ref to avoid the operator ->. Compiler will produce the same anyway.
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL* CompPtr = GetPixelPtr(PosX, y);
    for (INTN x = PosX; x < GetWidth() && (x - PosX) < TopImage.GetWidth(); ++x) {
      //------
      // test compAlpha = 255; TopAlpha = 0 -> only Comp, TopAplha = 255 -> only Top
      TopAlpha = TopImage.GetPixel(x-PosX, y-PosY).Reserved & 0xFF; //0, 255
      CompAlpha = CompPtr->Reserved & 0xFF; //255
      RevAlpha = 255 - TopAlpha; //2<<8; 255, 0
      TempAlpha = CompAlpha * RevAlpha; //2<<16; 255*255, 0
      TopAlpha *= 255; //2<<16; 0, 255*255
      FinalAlpha = TopAlpha + TempAlpha; //2<<16; 255*255, 255*255
//final alpha =(1-(1-x)*(1-y)) =(255*255-(255-topA)*(255-compA))/255 = topA+compA*(1-topA)

      if (FinalAlpha != 0) {
        Temp = (CompPtr->Blue * TempAlpha) + (TopImage.GetPixel(x-PosX, y-PosY).Blue * TopAlpha);
        CompPtr->Blue = (UINT8)(Temp / FinalAlpha);

        Temp = (CompPtr->Green * TempAlpha) + (TopImage.GetPixel(x-PosX, y-PosY).Green * TopAlpha);
        CompPtr->Green = (UINT8)(Temp / FinalAlpha);

        Temp = (CompPtr->Red * TempAlpha) + (TopImage.GetPixel(x-PosX, y-PosY).Red * TopAlpha);
        CompPtr->Red = (UINT8)(Temp / FinalAlpha);
      }

      if (Lowest) {
        CompPtr->Reserved = 255;
      } else {
        CompPtr->Reserved = (UINT8)(FinalAlpha / 255);
      }
      CompPtr++; //faster way to move to next pixel
    }
  }
}

/* Place this image over Back image at PosX,PosY
 * and result will be in this image
 * But pixels will be moved anyway so it's impossible without double copy
 *
 */
//void XImage::ComposeOnBack(INTN PosX, INTN PosY, const XImage& BackImage, bool Lowest)


void XImage::FlipRB()
{
  UINTN ImageSize = (Width * Height);
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL* Pixel = GetPixelPtr(0,0);
  for (UINTN i = 0; i < ImageSize; ++i) {
    UINT8 Temp = Pixel->Blue;
    Pixel->Blue = Pixel->Red;
    Pixel->Red = Temp;
//    if (!WantAlpha) Pixel->Reserved = 0xFF;
    Pixel++;
  }
}

/*
 * The function converted plain array into XImage object
 * Error = 0 - Success
 * Error = 28 - invalid signature
 */
EFI_STATUS XImage::FromPNG(const UINT8 * Data, UINTN Length)
{
//  DBG("XImage len=%llu\n", Length);
  if (Data == NULL) return EFI_INVALID_PARAMETER;
  UINT8 * PixelPtr; // = (UINT8 *)&PixelData[0];
  unsigned Error = eglodepng_decode(&PixelPtr, &Width, &Height, Data, Length);
  if (Error != 0 && Error != 28) {
    return EFI_NOT_FOUND;
  }
  if ( !PixelPtr ) return EFI_UNSUPPORTED; // It's possible to get error 28 and PixelPtr == NULL
  setSizeInPixels(Width, Height);
  //now we have a new pointer and want to move data
  INTN NewLength = Width * Height * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  CopyMem(GetPixelPtr(0,0), PixelPtr, NewLength);
  FreePool(PixelPtr); //allocated by lodepng

  FlipRB();
  return EFI_SUCCESS;
}

/*
 * The function creates new array Data and inform about it size to be saved
 * as a file.
 * The caller is responsible to free the array.
 */

EFI_STATUS XImage::ToPNG(UINT8** Data, UINTN& OutSize)
{
  size_t           FileDataLength = 0;
  FlipRB(); //commomly we want alpha for PNG, but not for screenshot, fix alpha there
  UINT8 * PixelPtr = (UINT8 *)&PixelData[0];
  unsigned Error = eglodepng_encode(Data, &FileDataLength, PixelPtr, Width, Height);
  OutSize = FileDataLength;
  if (Error) return EFI_UNSUPPORTED;
  return EFI_SUCCESS;
}

/*
 * fill XImage object by raster data described in SVG file
 * caller should create the object with Width and Height and calculate scale
 * scale = 1 correspond to fill the rect with the image
 * scale = 0.5 will reduce image
 * but this procedure is mostly for testing purpose. Real SVG theme can't be divided to separate SVG files
 */
EFI_STATUS XImage::FromSVG(const CHAR8 *SVGData, float scale)
{
  NSVGimage       *SVGimage;
  NSVGparser* p;

  NSVGrasterizer* rast = nsvgCreateRasterizer();
  if (!rast) return EFI_UNSUPPORTED;
  //we have to copy input data because nanosvg wants to change it
  char *input = (__typeof__(input))AllocateCopyPool(AsciiStrSize(SVGData), SVGData);
  if (!input) return EFI_DEVICE_ERROR;

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
//  nsvg__deleteParser(p); //can't delete raster until we make imageChain
  nsvgDeleteRasterizer(rast);
  FreePool(input);
  return EFI_SUCCESS;
}

// Screen operations
/*
 * The function to get image from screen. Used in  screenshot (full screen), Pointer (small area) and Draw (small area)
 * XImage must be created with Width, Height of Rect
 * the rect will be clipped if it intersects the screen edge
 *
 * be careful about alpha. This procedure can produce alpha = 0 which means full transparent
 * No! Anuway alpha should be corrected to 0xFF
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

  setSizeInPixels(Width, Height); // setSizeInPixels BEFORE, so &PixelData[0]
  if ( Width == 0 || Height == 0 ) return; // nothing to get, area is zero. &PixelData[0] would crash
/*
 * Blt(...Width, Height, Delta);
 * if (Delta == 0) {
 *   Delta = Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
 *  }
 *
 */
  if (GraphicsOutput != NULL) {
    GraphicsOutput->Blt(GraphicsOutput,
      &PixelData[0],
      EfiBltVideoToBltBuffer,
      x, y, 0, 0, Width, Height, 0); // Width*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  }
  else if (UgaDraw != NULL) {
    UgaDraw->Blt(UgaDraw,
      (EFI_UGA_PIXEL *)GetPixelPtr(0,0),
      EfiUgaVideoToBltBuffer,
      x, y, 0, 0, Width, Height, 0); //Width*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  }
  //fix alpha
  UINTN ImageSize = (Width * Height);
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL* Pixel = GetPixelPtr(0,0);
  for (UINTN i = 0; i < ImageSize; ++i) {
    (Pixel++)->Reserved = 0xFF;
  }
}

void XImage::DrawWithoutCompose(INTN x, INTN y, UINTN width, UINTN height)
{
  if (isEmpty()) {
    return;
  }

  if ( width == 0 ) width = Width;
  if ( height == 0 ) height = Height;
  UINTN AreaWidth = (x + width > (UINTN)UGAWidth) ? (UGAWidth - x) : width;
  UINTN AreaHeight = (y + height > (UINTN)UGAHeight) ? (UGAHeight - y) : height;
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
    GraphicsOutput->Blt(GraphicsOutput, (*this).GetPixelPtr(0, 0),
      EfiBltBufferToVideo,
      0, 0, x, y, AreaWidth, AreaHeight, GetWidth()*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  }
  else if (UgaDraw != NULL) {
    UgaDraw->Blt(UgaDraw, (EFI_UGA_PIXEL *)(*this).GetPixelPtr(0, 0), EfiUgaBltBufferToVideo,
      0, 0, x, y, AreaWidth, AreaHeight, GetWidth()*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  }
}

void XImage::Draw(INTN x, INTN y)
{
  Draw(x, y, 0, true);
}

void XImage::Draw(INTN x, INTN y, float scale)
{
  Draw(x, y, scale, true);
}

void XImage::Draw(INTN x, INTN y, float scale, bool Opaque)
{
  //prepare images
  if (isEmpty()) {
    return;
  }

  XImage Top(*this, scale); //can accept 0 as scale
  XImage Background(Width, Height);
  UINTN AreaWidth = (x + Width > (UINTN)UGAWidth) ? (UGAWidth - x) : Width;
  UINTN AreaHeight = (y + Height > (UINTN)UGAHeight) ? (UGAHeight - y) : Height;
  Background.GetArea(x, y, AreaWidth, AreaHeight); //it will resize the Background image
  Background.Compose(0, 0, Top, Opaque);
  Background.DrawWithoutCompose(x, y);
#if 0
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
    GraphicsOutput->Blt(GraphicsOutput, Background.GetPixelPtr(0, 0),
      EfiBltBufferToVideo,
      0, 0, x, y, AreaWidth, AreaHeight, AreaWidth*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  }
  else if (UgaDraw != NULL) {
    UgaDraw->Blt(UgaDraw, (EFI_UGA_PIXEL *)Background.GetPixelPtr(0, 0), EfiUgaBltBufferToVideo,
      0, 0, x, y, AreaWidth, AreaHeight, AreaWidth*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  }
#endif
}

void XImage::DrawOnBack(INTN XPos, INTN YPos, const XImage& Plate)
{
  XImage BackLayer(Width, Height);
  BackLayer.CopyRect(Plate, XPos, YPos); //assume Plate is big enough [XPos+Width, YPos+Height]
  BackLayer.Compose(0, 0, *this, true);
  BackLayer.DrawWithoutCompose(XPos, YPos);
}

/*
 * IconName is just func_about for example
 * will search files
 * icons/iconname.icns - existing themes
 * icons/iconname.png - it will be more correct
 * iconname.png - for example checkbox.png
 * if not found use embedded. It should be decoded again after theme change
 * SVG themes filled separately after ThemeName defined so the procedure just return EFI_SUCCESS
 * The function always create new image and will not be used to get a link to existing image
 */
EFI_STATUS XImage::LoadXImage(EFI_FILE *BaseDir, const char* IconName)
{
  return LoadXImage(BaseDir, XStringW().takeValueFrom(IconName));
}

EFI_STATUS XImage::LoadXImage(EFI_FILE *BaseDir, const wchar_t* LIconName)
{
  return LoadXImage(BaseDir, XStringW().takeValueFrom(LIconName));
}
//dont call this procedure for SVG theme BaseDir == NULL?
EFI_STATUS XImage::LoadXImage(EFI_FILE *BaseDir, const XStringW& IconName)
{
  EFI_STATUS      Status = EFI_NOT_FOUND;
  UINT8           *FileData = NULL;
  UINTN           FileDataLength = 0;

//  if (TypeSVG) { //make a copy of SVG image
//    XImage NewImage = Theme.GetIcon(IconName);
//    setSizeInPixels(NewImage.GetWidth(), NewImage.GetHeight());
//    CopyMem(&PixelData[0], &NewImage.PixelData[0], GetSizeInBytes());
//    return EFI_SUCCESS;
//  }
  
  if (BaseDir == NULL || IconName.isEmpty())
    return EFI_NOT_FOUND;
  
  // load file
  XStringW FileName = L"icons\\" + IconName + L".icns";
  Status = egLoadFile(BaseDir, FileName.data(), &FileData, &FileDataLength);
  if (EFI_ERROR(Status)) {
    FileName = L"icons\\" + IconName + L".png";
    Status = egLoadFile(BaseDir, FileName.data(), &FileData, &FileDataLength);
    if (EFI_ERROR(Status)) {
      FileName = IconName + L".png";
      if (EFI_ERROR(Status)) {
        FileName = IconName; //may be it already contain extension, for example Logo.png
        Status = egLoadFile(BaseDir, FileName.data(), &FileData, &FileDataLength);
        if (EFI_ERROR(Status)) {
          return Status;
        }
      }
    }
  }

  // decode it
  Status = FromPNG(FileData, FileDataLength);  
  if (EFI_ERROR(Status)) {
    DBG("%ls not decoded\n", IconName.data());
  }
  FreePool(FileData);
  return Status;
}

//EnsureImageSize should create new object with new sizes
//while compose uses old object
void XImage::EnsureImageSize(IN UINTN NewWidth, IN UINTN NewHeight)
{

  EnsureImageSize(NewWidth, NewHeight, NullColor);
}
void XImage::EnsureImageSize(IN UINTN NewWidth, IN UINTN NewHeight, IN CONST EFI_GRAPHICS_OUTPUT_BLT_PIXEL& Color)
{
  if (NewWidth == Width && NewHeight == Height)
    return;

  XImage NewImage(NewWidth, NewHeight);
  NewImage.Fill(Color);
  NewImage.Compose(0, 0, (*this), false); //should keep existing opacity
  setSizeInPixels(NewWidth, NewHeight); //include reallocate but loose data
  CopyMem(&PixelData[0], &NewImage.PixelData[0], GetSizeInBytes());
  //we have to copy pixels twice? because we can't return newimage instead of this
}

void XImage::DummyImage(IN UINTN PixelSize)
{

  UINTN           LineOffset;
  CHAR8           *Ptr, *YPtr;

  setSizeInPixels(PixelSize, PixelSize);

  LineOffset = PixelSize * 4;

  YPtr = (CHAR8 *)GetPixelPtr(0,0) + ((PixelSize - 32) >> 1) * (LineOffset + 4);
  for (UINTN y = 0; y < 32; y++) {
    Ptr = YPtr;
    for (UINTN x = 0; x < 32; x++) {
      if (((x + y) % 12) < 6) {
        *Ptr++ = 0;
        *Ptr++ = 0;
        *Ptr++ = 0;
      } else {
        *Ptr++ = 0;
        *Ptr++ = ~0; //yellow
        *Ptr++ = ~0;
      }
      *Ptr++ = ~111; //opacity
    }
    YPtr += LineOffset;
  }
}

void XImage::Copy(XImage* Image)
{
  CopyRect(*Image, 0, 0);
}
void XImage::CopyRect(const XImage& Image, INTN XPos, INTN YPos)
{
  for (INTN y = 0; y < GetHeight() && (y + YPos) < Image.GetHeight(); ++y) {
    for (INTN x = 0; x < GetWidth() && (x + XPos) < Image.GetWidth(); ++x) {
      PixelData[y * Width + x] = Image.GetPixel(x + XPos, y + YPos);
    }
  }
}

/*
 * copy rect InputRect from the input Image and place to OwnRect in this image
 * width and height will be the smaller of the two rect
 * taking into account boundary intersect
 */
void XImage::CopyRect(const XImage& Image, const EG_RECT& OwnPlace, const EG_RECT& InputRect)
{
  INTN Dx = OwnPlace.XPos - InputRect.XPos;
  INTN Dy = OwnPlace.YPos - InputRect.YPos;
  INTN W = MIN(OwnPlace.Width, InputRect.Width);
  INTN H = MIN(OwnPlace.Height, InputRect.Height);
  for (INTN y = OwnPlace.YPos; y - OwnPlace.YPos < H && y < GetHeight() && (y - Dy) < Image.GetHeight(); ++y) {
    for (INTN x = OwnPlace.XPos; x - OwnPlace.XPos < W && x < GetWidth() && (x - Dx) < Image.GetWidth(); ++x) {
      PixelData[y * Width + x] = Image.GetPixel(x - Dx, y - Dy);
    }
  }
}


EG_IMAGE* XImage::ToEGImage()
{
  if (isEmpty()) {
    return NULL; // what is better, return NULL or empty image?
  }
  EG_IMAGE* Tmp = egCreateImage(Width, Height, TRUE);  //memory leak
  CopyMem(&Tmp->PixelData[0], &PixelData[0], GetSizeInBytes());
  return Tmp;
}
