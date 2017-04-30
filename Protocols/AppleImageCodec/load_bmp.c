/*
 * libeg/load_bmp.c
 * Loading function for BMP images
 *
 * Copyright (c) 2006 Christoph Pfisterer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Christoph Pfisterer nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
//#include "libegint.h"

#include "picopng.h"

#define DBG(...)

// BMP structures

#pragma pack(1)

typedef struct {
    UINT8   Blue;
    UINT8   Green;
    UINT8   Red;
    UINT8   Alpha;
} BMP_COLOR_MAP;

typedef struct {
    CHAR8         CharB;
    CHAR8         CharM;
    UINT32        Size;
    UINT16        Reserved[2];
    UINT32        ImageOffset;
    UINT32        HeaderSize;
    INT32         PixelWidth;
    INT32         PixelHeight;
    UINT16        Planes;       // Must be 1
    UINT16        BitPerPixel;  // 1, 4, 8, 24, or 32
    UINT32        CompressionType;
    UINT32        ImageSize;    // Compressed image size in bytes
    UINT32        XPixelsPerMeter;
    UINT32        YPixelsPerMeter;
    UINT32        NumberOfColors;
    UINT32        ImportantColors;
} BMP_IMAGE_HEADER;

#pragma pack()

//
// Load BMP image
//

EG_IMAGE * egDecodeBMP(IN UINT8 *FileData, IN UINTN FileDataLength, IN BOOLEAN WantAlpha)
{
  EG_IMAGE            *NewImage;
  BMP_IMAGE_HEADER    *BmpHeader;
  BMP_COLOR_MAP       *BmpColorMap;
  INT32               x, y;
  INT32               RealPixelHeight, RealPixelWidth;
  UINT8               *ImagePtr;
  UINT8               *ImagePtrBase;
  UINTN               ImageLineOffset;
  UINT8               ImageValue = 0;
  UINT8               AlphaValue;
  EFI_UGA_PIXEL       *PixelPtr;
  UINTN               Index, BitIndex;
  
  // read and check header
  if (FileDataLength < sizeof(BMP_IMAGE_HEADER) || FileData == NULL)
    return NULL;
  BmpHeader = (BMP_IMAGE_HEADER *) FileData;
  if (BmpHeader->CharB != 'B' || BmpHeader->CharM != 'M')
    return NULL;
  if (BmpHeader->BitPerPixel != 1 && BmpHeader->BitPerPixel != 4 &&
      BmpHeader->BitPerPixel != 8 && BmpHeader->BitPerPixel != 24 &&
      BmpHeader->BitPerPixel != 32)
    return NULL;
  // 32-bit images are always stored uncompressed
  if (BmpHeader->CompressionType > 0 && BmpHeader->BitPerPixel != 32)
    return NULL;
  
  // calculate parameters
  ImageLineOffset = BmpHeader->PixelWidth;
  if (BmpHeader->BitPerPixel == 32)
    ImageLineOffset *= 4;
  else if (BmpHeader->BitPerPixel == 24)
    ImageLineOffset *= 3;
  else if (BmpHeader->BitPerPixel == 1)
    ImageLineOffset = (ImageLineOffset + 7) >> 3;
  else if (BmpHeader->BitPerPixel == 4)
    ImageLineOffset = (ImageLineOffset + 1) >> 1;
  if ((ImageLineOffset % 4) != 0)
    ImageLineOffset = ImageLineOffset + (4 - (ImageLineOffset % 4));
  
  // check bounds
  RealPixelHeight = BmpHeader->PixelHeight > 0 ? BmpHeader->PixelHeight : -BmpHeader->PixelHeight;
  RealPixelWidth = BmpHeader->PixelWidth > 0 ? BmpHeader->PixelWidth : -BmpHeader->PixelWidth;
  if (BmpHeader->ImageOffset + ImageLineOffset * RealPixelHeight > FileDataLength)
    return NULL;
  
  // allocate image structure and buffer
  NewImage = egCreateImage(RealPixelWidth, RealPixelHeight, WantAlpha);
  if (NewImage == NULL)
    return NULL;
  AlphaValue = WantAlpha ? 255 : 0;
  
  // convert image
  BmpColorMap = (BMP_COLOR_MAP *)(FileData + sizeof(BMP_IMAGE_HEADER));
  ImagePtrBase = FileData + BmpHeader->ImageOffset;
  for (y = 0; y < RealPixelHeight; y++) {
    ImagePtr = ImagePtrBase;
    ImagePtrBase += ImageLineOffset;
    // vertically mirror
    if (BmpHeader->PixelHeight != RealPixelHeight) {
      PixelPtr = NewImage->PixelData + y * RealPixelWidth;
    } else {
      PixelPtr = NewImage->PixelData + (RealPixelHeight - 1 - y) * RealPixelWidth;
    }    
    
    switch (BmpHeader->BitPerPixel) {
        
      case 1:
        for (x = 0; x < RealPixelWidth; x++) {
          BitIndex = x & 0x07;
          if (BitIndex == 0)
            ImageValue = *ImagePtr++;
          
          Index = (ImageValue >> (7 - BitIndex)) & 0x01;
          PixelPtr->Blue = BmpColorMap[Index].Blue;
          PixelPtr->Green = BmpColorMap[Index].Green;
          PixelPtr->Red = BmpColorMap[Index].Red;
          PixelPtr->Reserved = AlphaValue;
          PixelPtr++;
        }
        break;
        
      case 4:
        for (x = 0; x <= RealPixelWidth - 2; x += 2) {
          ImageValue = *ImagePtr++;
          
          Index = ImageValue >> 4;
          PixelPtr->Blue = BmpColorMap[Index].Blue;
          PixelPtr->Green = BmpColorMap[Index].Green;
          PixelPtr->Red = BmpColorMap[Index].Red;
          PixelPtr->Reserved = AlphaValue;
          PixelPtr++;
          
          Index = ImageValue & 0x0f;
          PixelPtr->Blue = BmpColorMap[Index].Blue;
          PixelPtr->Green = BmpColorMap[Index].Green;
          PixelPtr->Red = BmpColorMap[Index].Red;
          PixelPtr->Reserved = AlphaValue;
          PixelPtr++;
        }
        if (x < RealPixelWidth) {
          ImageValue = *ImagePtr++;
          
          Index = ImageValue >> 4;
          PixelPtr->Blue = BmpColorMap[Index].Blue;
          PixelPtr->Green = BmpColorMap[Index].Green;
          PixelPtr->Red = BmpColorMap[Index].Red;
          PixelPtr->Reserved = AlphaValue;
          PixelPtr++;
        }
        break;
        
      case 8:
        for (x = 0; x < RealPixelWidth; x++) {
          Index = *ImagePtr++;
          PixelPtr->Blue = BmpColorMap[Index].Blue;
          PixelPtr->Green = BmpColorMap[Index].Green;
          PixelPtr->Red = BmpColorMap[Index].Red;
          PixelPtr->Reserved = AlphaValue;
          PixelPtr++;
        }
        break;
        
      case 24:
        for (x = 0; x < RealPixelWidth; x++) {
          PixelPtr->Blue = *ImagePtr++;
          PixelPtr->Green = *ImagePtr++;
          PixelPtr->Red = *ImagePtr++;
          PixelPtr->Reserved = AlphaValue;
          PixelPtr++;
        }
        break;
      case 32:
        for (x = 0; x < RealPixelWidth; x++) {
          PixelPtr->Blue = *ImagePtr++;
          PixelPtr->Green = *ImagePtr++;
          PixelPtr->Red = *ImagePtr++;
          PixelPtr->Reserved = *ImagePtr++;
          if (!WantAlpha)
            PixelPtr->Reserved = 255 - PixelPtr->Reserved;
          PixelPtr++;
        }
        
    }
  }
  
  return NewImage;
}

//
// Save BMP image
//

VOID egEncodeBMP(IN EG_IMAGE *Image, OUT UINT8 **FileDataReturn, OUT UINTN *FileDataLengthReturn)
{
    BMP_IMAGE_HEADER    *BmpHeader;
    UINT8               *FileData;
    UINT64               FileDataLength;
    UINT8               *ImagePtr;
    UINT8               *ImagePtrBase;
    UINT64               ImageLineOffset;
    EFI_UGA_PIXEL       *PixelPtr;
    INT64                x, y;
    
    ImageLineOffset = MultU64x32(Image->Width, 3);
//    if ((ImageLineOffset % 4) != 0)
//        ImageLineOffset = ImageLineOffset + (4 - (ImageLineOffset % 4));
  if ((ImageLineOffset & 3) != 0) {
            ImageLineOffset = ImageLineOffset + (4 - (ImageLineOffset & 3));
  }

    
    // allocate buffer for file data
    FileDataLength = sizeof(BMP_IMAGE_HEADER) + MultU64x64(Image->Height, ImageLineOffset);
    FileData = AllocateZeroPool((UINTN)FileDataLength);
    if (FileData == NULL) {
        Print(L"Error allocate %d bytes\n", FileDataLength);
        *FileDataReturn = NULL;
        *FileDataLengthReturn = 0;
        return;
    }
    
    // fill header
    BmpHeader = (BMP_IMAGE_HEADER *)FileData;
    BmpHeader->CharB = 'B';
    BmpHeader->CharM = 'M';
    BmpHeader->Size = (UINT32)FileDataLength;
    BmpHeader->ImageOffset = sizeof(BMP_IMAGE_HEADER);
    BmpHeader->HeaderSize = 40;
    BmpHeader->PixelWidth = (UINT32)Image->Width;
    BmpHeader->PixelHeight = (UINT32)Image->Height;
    BmpHeader->Planes = 1;
    BmpHeader->BitPerPixel = 24;
    BmpHeader->CompressionType = 0;
    BmpHeader->XPixelsPerMeter = 0xb13;
    BmpHeader->YPixelsPerMeter = 0xb13;
    
    // fill pixel buffer
    ImagePtrBase = FileData + BmpHeader->ImageOffset;
    for (y = 0; y < Image->Height; y++) {
        ImagePtr = ImagePtrBase;
        ImagePtrBase += ImageLineOffset;
//        PixelPtr = Image->PixelData + (Image->Height - 1 - y) * Image->Width;
      PixelPtr = Image->PixelData + (INT32)(Image->Height - 1 - y) * (INT32)Image->Width;
        
        for (x = 0; x < Image->Width; x++) {
            *ImagePtr++ = PixelPtr->Blue;
            *ImagePtr++ = PixelPtr->Green;
            *ImagePtr++ = PixelPtr->Red;
            PixelPtr++;
        }
    }
    
    *FileDataReturn = FileData;
    *FileDataLengthReturn = (UINTN)FileDataLength;
}
/* EOF */
