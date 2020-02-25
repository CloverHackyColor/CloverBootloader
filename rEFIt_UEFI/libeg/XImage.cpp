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

const EFI_GRAPHICS_OUTPUT_BLT_PIXEL*    XImage::GetData() const
{
  return PixelData;
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
  for (UINTN i = 0; i < Height; ++i)
    for (UINTN j = 0; j < Width; ++j)
      *PixelData++ = Color;
}

void XImage::FillArea(EFI_GRAPHICS_OUTPUT_BLT_PIXEL Color, const EgRect& Rect)
{
  for (UINTN y = Rect.Ypos; y < Rect.Height && y < Height; ++y) {
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL* Ptr = PixelData + y * Width + Rect.Xpos;
    for (UINTN x = Rect.Xpos; x < Rect.Width && x < Width; ++x)
      *Ptr++ = Color;
  }
}



void XImage::Compose(int PosX, int PosY, const XImage& TopImage, bool Lowest) //lowest image is opaque
{
  UINT32      TopAlpha;
  UINT32      RevAlpha;
  UINT32      FinalAlpha;
  UINT32      Temp;
  const EFI_GRAPHICS_OUTPUT_BLT_PIXEL  *TopPtr;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *CompPtr;

  for (UINTN y = PosY; y < Height && (y - PosY) < TopImage.GetHeight(); y++) {
    TopPtr = TopImage.GetData();
    CompPtr = PixelData + y * Width + PosX;
    for (UINTN x = PosX; x < Width && (x - PosX) < TopImage.GetWidth(); x++) {
      TopAlpha = TopPtr->Reserved;
      RevAlpha = 255 - TopAlpha;
      FinalAlpha = (255*255 - RevAlpha*(255 - CompPtr->Reserved)) / 255;

//final alpha =(1-(1-x)*(1-y)) =(255*255-(255-topA)*(255-compA))/255
      Temp = ((UINT8)CompPtr->Blue * RevAlpha) + ((UINT8)TopPtr->Blue * TopAlpha);
      CompPtr->Blue = (UINT8)(Temp / 255);

      Temp = ((UINT8)CompPtr->Green * RevAlpha) + ((UINT8)TopPtr->Green * TopAlpha);
      CompPtr->Green = (UINT8)(Temp / 255);

      Temp = ((UINT8)CompPtr->Red * RevAlpha) + ((UINT8)TopPtr->Red * TopAlpha);
      CompPtr->Red = (UINT8)(Temp / 255);

      if (Lowest) {
        CompPtr->Reserved = (UINT8)(255);
      } else {
        CompPtr->Reserved = (UINT8)FinalAlpha;
      }

      TopPtr++, CompPtr++;

    }
  }
}


