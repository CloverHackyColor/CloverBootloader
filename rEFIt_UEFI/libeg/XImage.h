/*
This class will replace EG_IMAGE structure and methods
*/

#if !defined(__XSTRINGW_H__)
#define __XSTRINGW_H__

#include "../cpp_foundation/XToolsCommon.h"
#include <Platform.h>

/*
typedef struct {
  UINT8 Blue;
  UINT8 Green;
  UINT8 Red;
  UINT8 Reserved;  //this is Alpha. 0 means full transparent, 0xFF means opaque
} EFI_GRAPHICS_OUTPUT_BLT_PIXEL;

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
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL    *PixelData;
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

  const EFI_GRAPHICS_OUTPUT_BLT_PIXEL*    GetData() const;
  UINTN      GetWidth() const;
  UINTN      GetHeight() const;

  void Fill(EFI_GRAPHICS_OUTPUT_BLT_PIXEL Color = { 0, 0, 0, 0 });
  void FillArea(EFI_GRAPHICS_OUTPUT_BLT_PIXEL Color, const EgRect& Rect);
  void Compose(int PosX, int PosY, const XImage& TopImage, bool Lowest);
};

#endif //__XSTRINGW_H__
