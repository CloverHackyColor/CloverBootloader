//********************************************************************
//	created:	28:8:2012   20:54
//	filename: 	AppleImageCodec.h
//	author:		tiamo
//	purpose:	image code
//********************************************************************
// dmazar: changed ImageWidth and ImageHeight in GET_IMAGE_DIMS
//         to UINT32 from UINTN to get it working in 64 bit
//********************************************************************

#ifndef _APPLE_IMAGE_CODEC_H_
#define _APPLE_IMAGE_CODEC_H_


#define APPLE_IMAGE_CODEC_PROTOCOL_GUID		{0x0dfce9f6, 0xc4e3, 0x45ee, {0xa0, 0x6a, 0xa8, 0x61, 0x3b, 0x98, 0xa5, 0x07}}

typedef struct _APPLE_IMAGE_CODEC_PROTOCOL APPLE_IMAGE_CODEC_PROTOCOL;

typedef EFI_STATUS (EFIAPI* RECOGNIZE_IMAGE_DATA)(VOID* ImageBuffer, UINTN ImageSize, OUT VOID  **OutBuffer);
typedef EFI_STATUS (EFIAPI* GET_IMAGE_DIMS)(VOID* ImageBuffer, UINTN ImageSize, UINT32* ImageWidth, UINT32* ImageHeight);
typedef EFI_STATUS (EFIAPI* DECODE_IMAGE_DATA)(VOID* ImageBuffer, UINTN ImageSize, EFI_UGA_PIXEL** RawImageData, UINT32* RawImageDataSize);
typedef EFI_STATUS (EFIAPI* UNKNOWN_IMAGE_DATA)(VOID* ImageBuffer, UINTN Param1, UINTN Param2, UINTN Param3);

struct _APPLE_IMAGE_CODEC_PROTOCOL
{
	UINT64																	Version;
	UINT64																	FileExt;
	RECOGNIZE_IMAGE_DATA										RecognizeImageData;
	GET_IMAGE_DIMS													GetImageDims;
	DECODE_IMAGE_DATA												DecodeImageData;
  UNKNOWN_IMAGE_DATA                      Unknown1;
  UNKNOWN_IMAGE_DATA                      Unknown2;
};

extern EFI_GUID gAppleImageCodecProtocolGuid;

#endif
