/*
This class will replace EG_IMAGE structure and methods
*/

#if !defined(__XSTRINGW_H__)
#define __XSTRINGW_H__

#include "XToolsCommon.h"
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
  int Xpos;
  int Ypos;
  int Width;
  int Height;
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

public:
  size_t GetSize();  //in bytes
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL*    GetData();
  UINTN      GetWidth();
  UINTN      GetHeight();

  void Fill(EFI_GRAPHICS_OUTPUT_BLT_PIXEL Color = { 0, 0, 0, 0 });
  void FillArea(EFI_GRAPHICS_OUTPUT_BLT_PIXEL Color, const EgRect& Rect);
  void Compose(XImage& LowImage, XImage& TopImage, int PosX, int PosY, bool Lowest);
};

#endif //__XSTRINGW_H__