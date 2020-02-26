#include "XImage.h"

XImage::XImage()
{
  Width = 0;
  Height = 0;
  HasAlpha = true;
}

XImage::XImage(UINTN W, UINTN H, bool A)
{
  Width = W;
  Height = H;
  HasAlpha = A;
  PixelData.CheckSize(GetWidth()*GetHeight());
}

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

void XImage::Fill(EFI_GRAPHICS_OUTPUT_BLT_PIXEL Color)
{
  for (UINTN y = 0; y < Height; ++y)
    for (UINTN x = 0; x < Width; ++x)
      PixelData[y * Width + x] = Color;
}

void XImage::FillArea(EFI_GRAPHICS_OUTPUT_BLT_PIXEL Color, const EgRect& Rect)
{
  for (UINTN y = Rect.Ypos; y < Height && (y - Rect.Ypos) < Rect.Height; ++y) {
//    EFI_GRAPHICS_OUTPUT_BLT_PIXEL* Ptr = PixelData + y * Width + Rect.Xpos;
    for (UINTN x = Rect.Xpos; x < Width && (x - Rect.Xpos) < Rect.Width; ++x)
//      *Ptr++ = Color;
      PixelData[y * Width + x] = Color;
  }
}



void XImage::Compose(int PosX, int PosY, const XImage& TopImage, bool Lowest) //lowest image is opaque
{
  UINT32      TopAlpha;
  UINT32      RevAlpha;
  UINT32      FinalAlpha;
  UINT32      Temp;

  for (UINTN y = PosY; y < Height && (y - PosY) < TopImage.GetHeight(); ++y) {
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL& CompPtr = *GetPixelPtr(PosX, y); // I assign a ref to avoid the operator ->. Compiler will produce the same anyway.
    for (UINTN x = PosX; x < Width && (x - PosX) < TopImage.GetWidth(); ++x) {
      TopAlpha = TopImage.GetPixel(x-PosX, y-PosY).Reserved;
      RevAlpha = 255 - TopAlpha;
      FinalAlpha = (255*255 - RevAlpha*(255 - CompPtr.Reserved)) / 255;

//final alpha =(1-(1-x)*(1-y)) =(255*255-(255-topA)*(255-compA))/255
      Temp = (CompPtr.Blue * RevAlpha) + (TopImage.GetPixel(x-PosX, y-PosY).Blue * TopAlpha);
      CompPtr.Blue = (UINT8)(Temp / 255);

      Temp = (CompPtr.Green * RevAlpha) + (TopImage.GetPixel(x-PosX, y-PosY).Green * TopAlpha);
      CompPtr.Green = (UINT8)(Temp / 255);

      Temp = (CompPtr.Red * RevAlpha) + (TopImage.GetPixel(x-PosX, y-PosY).Red * TopAlpha);
      CompPtr.Red = (UINT8)(Temp / 255);

      if (Lowest) {
        CompPtr.Reserved = 255;
      } else {
        CompPtr.Reserved = (UINT8)FinalAlpha;
      }

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

unsigned XImage::FromPNG(const uint8_t * Data, UINTN Length, bool WantAlpha)
{
  unsigned Error = 0;
  uint8_t * PixelPtr = (uint8_t *)&PixelData[0];
  Error = eglodepng_decode(&PixelPtr, &Width, &Height, Data, Length);

  FlipRB(WantAlpha);
  return Error;
}

unsigned XImage::ToPNG(uint8_t** Data, UINTN& OutSize)
{
  size_t           FileDataLength = 0;
  FlipRB(false);
  uint8_t * PixelPtr = (uint8_t *)&PixelData[0];
  unsigned lode_return = eglodepng_encode(Data, &FileDataLength, PixelPtr, Width, Height);
  OutSize = FileDataLength;
  return lode_return;
}


