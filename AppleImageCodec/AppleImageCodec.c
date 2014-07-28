/** @file

  Driver with Apple image decode protocol implementation
  for PNG.

**/

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/UgaDraw.h>
#include <Protocol/AppleImageCodecProtocol.h>

#include <picopng.h>

//#define DBG(...) AsciiPrint(__VA_ARGS__);
#define DBG(...)


//
// PNG Image codec protocol instance implementation
//

EFI_STATUS
EFIAPI
PngRecognizeImageData (
  VOID         *ImageBuffer,
  UINTN         ImageSize
  )
{
  EG_IMAGE      *Image;
  
  DBG("AppleImageCodec PngRecognizeImageData: Status = ");
  Image = egDecodePNG((UINT8*)ImageBuffer, ImageSize, 0, FALSE);
  if (Image == NULL) {
    DBG("EFI_UNSUPPORTED\n");
    return EFI_UNSUPPORTED;
  }
  
  DBG("EFI_SUCCESS\n");
  DBG("ImageBuffer=%p, ImageSize=%d\n", ImageBuffer, ImageSize);
  DBG("Decoded: W=%d, H=%d\n", Image->Width, Image->Height);
  egFreeImage(Image);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PngGetImageDims (
  VOID          *ImageBuffer,
  UINTN         ImageSize,
  UINT32         *ImageWidth,
  UINT32         *ImageHeight
  )
{
  EG_IMAGE      *Image;
  
  DBG("AppleImageCodec PngGetImageDims: Status = ");
  Image = egDecodePNG((UINT8*)ImageBuffer, ImageSize, 0, FALSE);
  if (Image == NULL) {
    DBG("EFI_UNSUPPORTED\n");
    return EFI_UNSUPPORTED;
  }
  
  *ImageWidth = Image->Width;
  *ImageHeight = Image->Height;
  
  DBG("EFI_SUCCESS, Width=%d, Height=%d\n", *ImageWidth, *ImageHeight);
  DBG("ImageBuffer=%p, ImageSize=%d\n", ImageBuffer, ImageSize);
  DBG("ImageWidth=%p, ImageHeight=%p\n", ImageWidth, ImageHeight);
  DBG("Decoded: W=%d, H=%d\n", Image->Width, Image->Height);
  egFreeImage(Image);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PngDecodeImageData (
  VOID          *ImageBuffer,
  UINTN         ImageSize,
  EFI_UGA_PIXEL **RawImageData,
  UINTN         *RawImageDataSize
  )
{
  EG_IMAGE      *Image;
  INTN          Index;
  
  DBG("AppleImageCodec PngDecodeImageData: Status = ");
  Image = egDecodePNG((UINT8*)ImageBuffer, ImageSize, 0, FALSE);
  if (Image == NULL) {
    DBG("EFI_UNSUPPORTED\n");
    return EFI_UNSUPPORTED;
  }
  
  *RawImageData = Image->PixelData;
  *RawImageDataSize = Image->Width * Image->Height * sizeof(EFI_UGA_PIXEL);
  
  DBG("EFI_SUCCESS, RawImageDataSize=%d\n", *RawImageDataSize);
  DBG("ImageBuffer=%p, ImageSize=%d\n", ImageBuffer, ImageSize);
  DBG("Decoded: W=%d, H=%d\n", Image->Width, Image->Height);
  for (Index=0; Index<10; Index++) {
    DBG("P%d: r,g,b,a= %x, %x, %x, %x\n", Index, (*RawImageData)[Index].Red, (*RawImageData)[Index].Green, (*RawImageData)[Index].Blue, (*RawImageData)[Index].Reserved);
  }
  egFreeImage(Image);
  return EFI_SUCCESS;
}

/** PNG Image codec protocol instance. */
APPLE_IMAGE_CODEC_PROTOCOL gPngImageCodec = {
  // Version
  1,
  
  // FileExt
  0,
  
  // RecognizeImageData
  PngRecognizeImageData,
  
  // GetImageDims
  PngGetImageDims,
  
  // DecodeImageData
  PngDecodeImageData
};

/** Driver's entry point. Installs our StartImage to detect boot loader start. */
EFI_STATUS
EFIAPI
AppleImageCodecEntrypoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
)
{
  EFI_STATUS              Status;
  EFI_HANDLE              NewHandle;
  
  //
  // Install instance of Apple image codec protocol for
  // PNG files
  //
  NewHandle = NULL;  // install to a new handle
  Status = gBS->InstallMultipleProtocolInterfaces(&NewHandle, &gAppleImageCodecProtocolGuid, &gPngImageCodec, NULL);
  if (EFI_ERROR(Status)) {
    DBG("AppleImageCodec: error installing protocol, Status = %r\n", Status);
  }
  return Status;
}
