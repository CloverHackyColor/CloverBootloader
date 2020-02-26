/*
This class will replace EG_IMAGE structure and methods
*/

#if !defined(__XSTRINGW_H__)
#define __XSTRINGW_H__

#include "../cpp_foundation/XToolsCommon.h"
#include "../cpp_foundation/XArray.h"
#include "lodepng.h"
#include <Platform.h>

#if 0 //ndef EFI_GRAPHICS_OUTPUT_BLT_PIXEL
typedef struct {
  UINT8 Blue;
  UINT8 Green;
  UINT8 Red;
  UINT8 Reserved;  //this is Alpha. 0 means full transparent, 0xFF means opaque
} EFI_GRAPHICS_OUTPUT_BLT_PIXEL;
#endif
/*
typedef union {
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL Pixel;
  UINT32                        Raw;
} EFI_GRAPHICS_OUTPUT_BLT_PIXEL_UNION;
*/

typedef struct {
  UINTN Xpos;
  UINTN Ypos;
  UINTN Width;
  UINTN Height;
} EgRect;

class XImage
{
protected:
  UINTN      Width;
  UINTN      Height;
  XArray<EFI_GRAPHICS_OUTPUT_BLT_PIXEL> PixelData;
  bool       HasAlpha;   
public:
  XImage();
  XImage(UINTN W, UINTN H, bool HasAlpha);
//  XImage(UINTN W, UINTN H, bool HasAlpha, UINT32 Color); //egCreateFilledImage
//  XImage(VOID *Data); 
  ~XImage();

protected:
  UINTN GetSize() const;  //in bytes

public:

  const XArray<EFI_GRAPHICS_OUTPUT_BLT_PIXEL>& GetData() const;

  const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& GetPixel(UINTN x, UINTN y) const;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL* GetPixelPtr(UINTN x, UINTN y);
  UINTN      GetWidth() const;
  UINTN      GetHeight() const;

  void Fill(EFI_GRAPHICS_OUTPUT_BLT_PIXEL Color = { 0, 0, 0, 0 });
  void FillArea(EFI_GRAPHICS_OUTPUT_BLT_PIXEL Color, const EgRect& Rect);
  void Compose(int PosX, int PosY, const XImage& TopImage, bool Lowest);
  void FlipRB(bool WantAlpha);
  unsigned FromPNG(const uint8_t * Data, UINTN Lenght, bool WantAlpha); //WantAlpha always true?
  unsigned ToPNG(uint8_t** Data, UINTN& OutSize);
};

#endif //__XSTRINGW_H__
