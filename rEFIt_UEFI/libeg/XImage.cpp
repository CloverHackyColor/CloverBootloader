#include "XImage.h"

XImage::XImage()
{
  Width = 0;
  Height = 0;
  PixelData = nullptr;
  HasAlpha = true;
}

XImage::XImage(UINTN W, UINTN H, bool A)
{
  Width = W;
  Height = H;
  HasAlpha = A;
  PixelData = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL*)Xalloc(GetSize());
}

XImage::~XImage()
{
  Xfree(PixelData);
}

EFI_GRAPHICS_OUTPUT_BLT_PIXEL*    XImage::GetData()
{
  return PixelData;
}

UINTN      XImage::GetWidth()
{
  return Width;
}

UINTN      XImage::GetHeight()
{
  return Height;
}

size_t XImage::GetSize()
{
  return Width * Height * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
}

void XImage::Fill(EFI_GRAPHICS_OUTPUT_BLT_PIXEL Color)
{
  for (int i = 0; i < Height; ++i)
    for (int j = 0; j < Width; ++j)
      *PixelData++ = Color;
}

void XImage::FillArea(EFI_GRAPHICS_OUTPUT_BLT_PIXEL Color, const EgRect& Rect)
{
  for (int y = Rect.Ypos; y < Rect.Height && y < Height; ++y) {
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL* Ptr = PixelData + y * Width + Rect.Xpos;
    for (int x = Rect.Xpos; x < Rect.Width && x < Width; ++x)
      *PixelData++ = Color;
  }
}



void XImage::Compose(XImage& LowImage, XImage& TopImage, int PosX, int PosY, bool Lowest) //lowest image is opaque
{
  UINT32      TopAlpha;
  UINT32      RevAlpha;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL    *TopPtr, *CompPtr;

  for (int y = PosY; y < LowImage.GetHeight() && (y - PosY) < TopImage.GetHeight(); y++) {
    TopPtr = TopImage.GetData();
    CompPtr = LowImage.GetData() + y * LowImage.GetWidth() + PosX;
    for (int x = PosX; x < LowImage.GetWidth() && (x - PosX) < TopImage.GetWidth(); x++) {
      TopAlpha = TopPtr->Reserved;
      RevAlpha = 255 - TopAlpha;
    }
  }
}
