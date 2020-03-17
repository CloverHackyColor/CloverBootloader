/*
This class will replace EG_IMAGE structure and methods
*/

#if !defined(__XIMAGE_H__)
#define __XIMAGE_H__

//#include <Platform.h>
//

extern "C" {
#include <Protocol/GraphicsOutput.h>
}
#include "../cpp_foundation/XToolsCommon.h"
#include "../cpp_foundation/XArray.h"
#include "../libeg/libeg.h"
//#include "lodepng.h"
//
//#include "nanosvg.h"
//#include "FloatLib.h"


#if 0 //ndef EFI_GRAPHICS_OUTPUT_BLT_PIXEL
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
#endif

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
 
public:
  XImage();
  XImage(UINTN W, UINTN H);
  XImage(EG_IMAGE* egImage);
  XImage(const XImage& Image, float scale);
  ~XImage();

  XImage& operator= (const XImage& other);

protected:
  UINTN GetSizeInBytes() const;  //in bytes

public:

  void setSizeInPixels(UINTN W, UINTN H);

  const XArray<EFI_GRAPHICS_OUTPUT_BLT_PIXEL>& GetData() const;

  const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& GetPixel(UINTN x, UINTN y) const;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL* GetPixelPtr(UINTN x, UINTN y);
  UINTN      GetWidth() const;
  UINTN      GetHeight() const;

  void setZero() { SetMem( (void*)GetPixelPtr(0, 0), GetSizeInBytes(), 0); }

  void setEmpty() { Width=0; Height=0; PixelData.setEmpty(); }
  bool isEmpty() const { return PixelData.size() == 0; }


  void Fill(const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& Color = { 0, 0, 0, 0 });
  void FillArea(const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& Color, const EgRect& Rect);
  void CopyScaled(const XImage& Image, float scale);
  void Compose(INTN PosX, INTN PosY, const XImage& TopImage, bool Lowest); //instead of compose we can Back.Draw(...) + Top.Draw(...)
  void FlipRB(bool WantAlpha);
  unsigned FromPNG(const UINT8 * Data, UINTN Lenght);
  unsigned ToPNG(UINT8** Data, UINTN& OutSize);
  unsigned FromSVG(const CHAR8 *SVGData, float scale);
  void GetArea(const EG_RECT& Rect);
  void GetArea(INTN x, INTN y, UINTN W, UINTN H);
  void Draw(INTN x, INTN y, float scale);
  void DrawWithoutCompose(INTN x, INTN y, UINTN width = 0, UINTN height = 0);
};

#endif //__XSTRINGW_H__
