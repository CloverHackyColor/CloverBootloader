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

#include "picopng.h"

//#define DBG(...) AsciiPrint(__VA_ARGS__);
#define DBG(...)

static EG_IMAGE * egDecodeAny(IN UINT8 *FileData, IN UINTN FileDataLength,
                              IN BOOLEAN WantAlpha)
{
  EG_IMAGE        *NewImage;
  
  //automatic choose format
  NewImage = egDecodePNG(FileData, FileDataLength, WantAlpha);
  
  if (!NewImage) {
    DBG(" ..png is wrong try to decode icns\n");
    NewImage = egDecodeICNS(FileData, FileDataLength, 128, WantAlpha);
  }
  
  if (!NewImage) {
    DBG(" ..png and icns is wrong try to decode bmp\n");
    NewImage = egDecodeBMP(FileData, FileDataLength, WantAlpha);
  }
  
  return NewImage;
}

//
// PNG Image codec protocol instance implementation
//

EFI_STATUS
EFIAPI
RecognizeImageData (IN APPLE_IMAGE_CODEC_PROTOCOL* This,
  VOID         *ImageBuffer,
  UINTN         ImageSize
  )
{
  EG_IMAGE      *Image;
  
  DBG("AppleImageCodec PngRecognizeImageData: Status = ");
  Image = egDecodeAny((UINT8*)ImageBuffer, ImageSize, FALSE);
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
GetImageDims (IN APPLE_IMAGE_CODEC_PROTOCOL* This,
  VOID          *ImageBuffer,
  UINTN         ImageSize,
  UINT32         *ImageWidth,
  UINT32         *ImageHeight
  )
{
  EG_IMAGE      *Image;
  
  DBG("AppleImageCodec GetImageDims: Status = ");
  Image = egDecodeAny((UINT8*)ImageBuffer, ImageSize, FALSE);
  if (Image == NULL) {
    DBG("EFI_UNSUPPORTED\n");
    return EFI_UNSUPPORTED;
  }
  
  *ImageWidth = (UINT32)Image->Width;
  *ImageHeight = (UINT32)Image->Height;
  
  DBG("EFI_SUCCESS, Width=%d, Height=%d\n", *ImageWidth, *ImageHeight);
  DBG("ImageBuffer=%p, ImageSize=%d\n", ImageBuffer, ImageSize);
  DBG("ImageWidth=%p, ImageHeight=%p\n", ImageWidth, ImageHeight);
  DBG("Decoded: W=%d, H=%d\n", Image->Width, Image->Height);
  egFreeImage(Image);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
DecodeImageData (IN APPLE_IMAGE_CODEC_PROTOCOL* This,
  VOID          *ImageBuffer,
  UINTN         ImageSize,
  EFI_UGA_PIXEL **RawImageData,
  UINTN         *RawImageDataSize
  )
{
  EG_IMAGE      *Image;
  INTN          Index;
  //automatic choose format
  if (!RawImageData || !*RawImageData || !RawImageDataSize) {
    return EFI_INVALID_PARAMETER;
  }
  
  DBG("AppleImageCodec DecodeImageData: Status = ");
  Image = egDecodeAny((UINT8*)ImageBuffer, ImageSize, FALSE);
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

/** Image codec protocol instance. */
APPLE_IMAGE_CODEC_PROTOCOL gAppleImageCodec = {
  // Version
  1,
  
  // FileExt
  0,
  
  RecognizeImageData,
  //PngRecognizeImageData,
  
  GetImageDims,
  //PngGetImageDims,
  
  DecodeImageData,
  //PngDecodeImageData
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
  Status = gBS->InstallMultipleProtocolInterfaces(&NewHandle, &gAppleImageCodecProtocolGuid, &gAppleImageCodec, NULL);
  if (EFI_ERROR(Status)) {
    DBG("AppleImageCodec: error installing protocol, Status = %r\n", Status);
  }
  return Status;
}
