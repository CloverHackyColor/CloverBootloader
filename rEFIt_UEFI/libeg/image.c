/*
 * libeg/image.c
 * Image handling functions
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

#include "libegint.h"
#include "Platform.h"

#define MAX_FILE_SIZE (1024*1024*1024)

#ifndef DEBUG_ALL
#define DEBUG_IMG 0
#else
#define DEBUG_IMG DEBUG_ALL
#endif

#if DEBUG_IMG == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_IMG, __VA_ARGS__)	
#endif

//
// Basic image handling
//

EG_IMAGE * egCreateImage(IN INT64 Width, IN INT64 Height, IN BOOLEAN HasAlpha)
{
    EG_IMAGE        *NewImage;
    
    NewImage = (EG_IMAGE *) AllocatePool(sizeof(EG_IMAGE));
    if (NewImage == NULL)
        return NULL;
    NewImage->PixelData = (EG_PIXEL *) AllocatePool((UINTN)(Width * Height * sizeof(EG_PIXEL)));
    if (NewImage->PixelData == NULL) {
        FreePool(NewImage);
        return NULL;
    }
    
    NewImage->Width = Width;
    NewImage->Height = Height;
    NewImage->HasAlpha = HasAlpha;
    return NewImage;
}

EG_IMAGE * egCreateFilledImage(IN INT64 Width, IN INT64 Height, IN BOOLEAN HasAlpha, IN EG_PIXEL *Color)
{
    EG_IMAGE        *NewImage;
    
    NewImage = egCreateImage(Width, Height, HasAlpha);
    if (NewImage == NULL)
        return NULL;
    
    egFillImage(NewImage, Color);
    return NewImage;
}

EG_IMAGE * egCopyImage(IN EG_IMAGE *Image)
{
    EG_IMAGE        *NewImage;
    
    NewImage = egCreateImage(Image->Width, Image->Height, Image->HasAlpha);
    if (NewImage == NULL)
        return NULL;
    
    CopyMem(NewImage->PixelData, Image->PixelData, (UINTN)(Image->Width * Image->Height * sizeof(EG_PIXEL)));
    return NewImage;
}

EG_IMAGE * egCopyScaledImage(IN EG_IMAGE *Image, IN INT64 Ratio) //will be N/16
{
  EG_IMAGE    *NewImage;
  INT64      x, x0, x1, x2, y, y0, y1, y2;
  INT64      NewH, NewW;
  EG_PIXEL    *Dest;
//  UINT64      b, g, r, a;
  
//  NewW = RShiftU64(MultU64x64(Image->Width, Ratio), 4);
//  NewH = RShiftU64(MultU64x64(Image->Height, Ratio), 4);
  NewW = (Image->Width * Ratio) >> 4;
  NewH = (Image->Height * Ratio) >> 4;
  
  NewImage = egCreateImage(NewW, NewH, Image->HasAlpha);
  if (NewImage == NULL)
    return NULL;
  
  Dest = NewImage->PixelData;
  for (y = 0; y < NewH; y++) {
    y1 = DivU64x64Remainder(LShiftU64(y, 4), Ratio, 0);
    y0 = MultU64x64(((y1 > 0)?(y1-1):y1), Image->Width);
    y2 = MultU64x64(((y1 < Image->Height)?(y1+1):y1), Image->Width);
    y1 = MultU64x64(y1, Image->Width);
    for (x = 0; x < NewW; x++) {
      x1 = DivU64x64Remainder(LShiftU64(x, 4), Ratio, 0);
      x0 = (x1 > 0)?(x1-1):x1;
      x2 = (x1 < Image->Width)?(x1+1):x1;
      //TODO - make sum of 5 points
     // *Dest++ = Image->PixelData[x1+y1];
      
      Dest->b = (UINT8)DivU64x64Remainder(Image->PixelData[x1+y1].b * 2 +
                                   Image->PixelData[x0+y1].b + Image->PixelData[x2+y1].b +
                                   Image->PixelData[x1+y0].b + Image->PixelData[x1+y2].b, 6, 0);
      Dest->g = (UINT8)DivU64x64Remainder(Image->PixelData[x1+y1].g * 2 +
                                   Image->PixelData[x0+y1].g + Image->PixelData[x2+y1].g +
                                   Image->PixelData[x1+y0].g + Image->PixelData[x1+y2].g, 6, 0);
      Dest->r = (UINT8)DivU64x64Remainder(Image->PixelData[x1+y1].r * 2 +
                                   Image->PixelData[x0+y1].r + Image->PixelData[x2+y1].r +
                                   Image->PixelData[x1+y0].r + Image->PixelData[x1+y2].r, 6, 0);
      Dest->a = Image->PixelData[x1+y1].a;
      Dest++;
    }
  }
  
  
  return NewImage;
}


VOID egFreeImage(IN EG_IMAGE *Image)
{
    if (Image != NULL) {
        if (Image->PixelData != NULL)
            FreePool(Image->PixelData);
        FreePool(Image);
    }
}

//
// Basic file operations
//

EFI_STATUS egLoadFile(IN EFI_FILE_HANDLE BaseDir, IN CHAR16 *FileName,
                      OUT UINT8 **FileData, OUT UINTN *FileDataLength)
{
    EFI_STATUS          Status;
    EFI_FILE_HANDLE     FileHandle;
    EFI_FILE_INFO       *FileInfo;
    UINT64              ReadSize;
    UINTN               BufferSize;
    UINT8               *Buffer;
    
    Status = BaseDir->Open(BaseDir, &FileHandle, FileName, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status))
        return Status;
    
    FileInfo = EfiLibFileInfo(FileHandle);
    if (FileInfo == NULL) {
        FileHandle->Close(FileHandle);
        return EFI_NOT_FOUND;
    }
    ReadSize = FileInfo->FileSize;
    if (ReadSize > MAX_FILE_SIZE)
        ReadSize = MAX_FILE_SIZE;
    FreePool(FileInfo);
    
    BufferSize = (UINTN)ReadSize;   // was limited to 1 GB above, so this is safe
    Buffer = (UINT8 *) AllocateZeroPool (BufferSize);
    if (Buffer == NULL) {
        FileHandle->Close(FileHandle);
        return EFI_OUT_OF_RESOURCES;
    }
    
    Status = FileHandle->Read(FileHandle, &BufferSize, Buffer);
    FileHandle->Close(FileHandle);
    if (EFI_ERROR(Status)) {
        FreePool(Buffer);
        return Status;
    }
    
    *FileData = Buffer;
    *FileDataLength = BufferSize;
    return EFI_SUCCESS;
}
//Slice - this is gEfiPartTypeSystemPartGuid
//static EFI_GUID ESPGuid = { 0xc12a7328, 0xf81f, 0x11d2, { 0xba, 0x4b, 0x00, 0xa0, 0xc9, 0x3e, 0xc9, 0x3b } };
//there is assumed only one ESP partition. What if there are two HDD gpt formatted?
static EFI_STATUS egFindESP(OUT EFI_FILE_HANDLE *RootDir)
{
    EFI_STATUS          Status;
    UINTN               HandleCount = 0;
    EFI_HANDLE          *Handles;
    
    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiPartTypeSystemPartGuid, NULL, &HandleCount, &Handles);
    if (!EFI_ERROR(Status) && HandleCount > 0) {
        *RootDir = EfiLibOpenRoot(Handles[0]);
        if (*RootDir == NULL)
            Status = EFI_NOT_FOUND;
        FreePool(Handles);
    }
    return Status;
}
//if (NULL, ...) then save to EFI partition
EFI_STATUS egSaveFile(IN EFI_FILE_HANDLE BaseDir OPTIONAL, IN CHAR16 *FileName,
                      IN UINT8 *FileData, IN UINTN FileDataLength)
{
    EFI_STATUS          Status;
    EFI_FILE_HANDLE     FileHandle;
    UINTN               BufferSize;
    
    if (BaseDir == NULL) {
        Status = egFindESP(&BaseDir);
        if (EFI_ERROR(Status))
            return Status;
    }
    
    Status = BaseDir->Open(BaseDir, &FileHandle, FileName,
                           EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
    if (EFI_ERROR(Status))
        return Status;
    
    BufferSize = FileDataLength;
    Status = FileHandle->Write(FileHandle, &BufferSize, FileData);
    FileHandle->Close(FileHandle);
    
    return Status;
}

//
// Loading images from files and embedded data
//

static CHAR16 * egFindExtension(IN CHAR16 *FileName)
{
    INTN i;
    
    for (i = StrLen(FileName); i >= 0; i--) {
        if (FileName[i] == '.')
            return FileName + i + 1;
        if (FileName[i] == '/' || FileName[i] == '\\')
            break;
    }
    return FileName + StrLen(FileName);
}

static EG_IMAGE * egDecodeAny(IN UINT8 *FileData, IN UINTN FileDataLength,
                              IN CHAR16 *Format, IN UINTN IconSize, IN BOOLEAN WantAlpha)
{
  EG_DECODE_FUNC  DecodeFunc;
  EG_IMAGE        *NewImage;
  
  // dispatch by extension
  DecodeFunc = NULL;
  if (StriCmp(Format, L"BMP") == 0)
    DecodeFunc = egDecodeBMP;
  else if (StriCmp(Format, L"ICNS") == 0)
    DecodeFunc = egDecodeICNS;
  else if (StriCmp(Format, L"PNG") == 0){
//    DBG("decode PNG\n");
    DecodeFunc = egDecodePNG;
  }
//  else if (StriCmp(Format, L"TGA") == 0)
//    DecodeFunc = egDecodeTGA;
  
  if (DecodeFunc == NULL)
    return NULL;
//  DBG("will decode data=%x len=%d icns=%d alpha=%c\n", FileData, FileDataLength, IconSize, WantAlpha?'Y':'N'); 
  // decode it
  NewImage = DecodeFunc(FileData, FileDataLength, IconSize, WantAlpha);
  
  return NewImage;
}

EG_IMAGE * egLoadImage(IN EFI_FILE_HANDLE BaseDir, IN CHAR16 *FileName, IN BOOLEAN WantAlpha)
{
    EFI_STATUS      Status;
    UINT8           *FileData;
    UINTN           FileDataLength;
    EG_IMAGE        *NewImage;
    
    if (BaseDir == NULL || FileName == NULL)
        return NULL;
    
    // load file
    Status = egLoadFile(BaseDir, FileName, &FileData, &FileDataLength);
//  DBG("File=%s loaded with status=%r\n", FileName, Status);
    if (EFI_ERROR(Status))
        return NULL;
    
    // decode it
    NewImage = egDecodeAny(FileData, FileDataLength, egFindExtension(FileName), 128, WantAlpha);
//  DBG("decoded\n");
    FreePool(FileData);
//   DBG("FreePool OK\n"); 
    return NewImage;
}

EG_IMAGE * egLoadIcon(IN EFI_FILE_HANDLE BaseDir, IN CHAR16 *FileName, IN UINTN IconSize)
{
    EFI_STATUS      Status;
    UINT8           *FileData;
    UINTN           FileDataLength;
    EG_IMAGE        *NewImage;
    
    if (BaseDir == NULL || FileName == NULL)
        return NULL;
    
    // load file
    Status = egLoadFile(BaseDir, FileName, &FileData, &FileDataLength);
    if (EFI_ERROR(Status))
        return NULL;
    
    // decode it
    NewImage = egDecodeAny(FileData, FileDataLength, egFindExtension(FileName), IconSize, TRUE);
    FreePool(FileData);
    
    return NewImage;
}

EG_IMAGE * egDecodeImage(IN UINT8 *FileData, IN UINTN FileDataLength, IN CHAR16 *Format, IN BOOLEAN WantAlpha)
{
    return egDecodeAny(FileData, FileDataLength, Format, 128, WantAlpha);
}

EG_IMAGE * egPrepareEmbeddedImage(IN EG_EMBEDDED_IMAGE *EmbeddedImage, IN BOOLEAN WantAlpha)
{
    EG_IMAGE            *NewImage;
    UINT8               *CompData;
    UINTN               CompLen;
    UINTN               PixelCount;
    
    // sanity check
    if (EmbeddedImage->PixelMode > EG_MAX_EIPIXELMODE ||
        (EmbeddedImage->CompressMode != EG_EICOMPMODE_NONE && EmbeddedImage->CompressMode != EG_EICOMPMODE_RLE))
        return NULL;
    
    // allocate image structure and pixel buffer
    NewImage = egCreateImage(EmbeddedImage->Width, EmbeddedImage->Height, WantAlpha);
    if (NewImage == NULL)
        return NULL;
    
    CompData = (UINT8 *)EmbeddedImage->Data;   // drop const
    CompLen  = EmbeddedImage->DataLength;
    PixelCount = EmbeddedImage->Width * EmbeddedImage->Height;
    
    // FUTURE: for EG_EICOMPMODE_EFICOMPRESS, decompress whole data block here
    
    if (EmbeddedImage->PixelMode == EG_EIPIXELMODE_GRAY ||
        EmbeddedImage->PixelMode == EG_EIPIXELMODE_GRAY_ALPHA) {
        
        // copy grayscale plane and expand
        if (EmbeddedImage->CompressMode == EG_EICOMPMODE_RLE) {
            egDecompressIcnsRLE(&CompData, &CompLen, PLPTR(NewImage, r), PixelCount);
        } else {
            egInsertPlane(CompData, PLPTR(NewImage, r), PixelCount);
            CompData += PixelCount;
        }
        egCopyPlane(PLPTR(NewImage, r), PLPTR(NewImage, g), PixelCount);
        egCopyPlane(PLPTR(NewImage, r), PLPTR(NewImage, b), PixelCount);
        
    } else if (EmbeddedImage->PixelMode == EG_EIPIXELMODE_COLOR ||
               EmbeddedImage->PixelMode == EG_EIPIXELMODE_COLOR_ALPHA) {
        
        // copy color planes
        if (EmbeddedImage->CompressMode == EG_EICOMPMODE_RLE) {
            egDecompressIcnsRLE(&CompData, &CompLen, PLPTR(NewImage, r), PixelCount);
            egDecompressIcnsRLE(&CompData, &CompLen, PLPTR(NewImage, g), PixelCount);
            egDecompressIcnsRLE(&CompData, &CompLen, PLPTR(NewImage, b), PixelCount);
        } else {
            egInsertPlane(CompData, PLPTR(NewImage, r), PixelCount);
            CompData += PixelCount;
            egInsertPlane(CompData, PLPTR(NewImage, g), PixelCount);
            CompData += PixelCount;
            egInsertPlane(CompData, PLPTR(NewImage, b), PixelCount);
            CompData += PixelCount;
        }
        
    } else {
        
        // set color planes to black
        egSetPlane(PLPTR(NewImage, r), 0, PixelCount);
        egSetPlane(PLPTR(NewImage, g), 0, PixelCount);
        egSetPlane(PLPTR(NewImage, b), 0, PixelCount);
        
    }
    
    if (WantAlpha && (EmbeddedImage->PixelMode == EG_EIPIXELMODE_GRAY_ALPHA ||
                      EmbeddedImage->PixelMode == EG_EIPIXELMODE_COLOR_ALPHA ||
                      EmbeddedImage->PixelMode == EG_EIPIXELMODE_ALPHA)) {
        
        // copy alpha plane
        if (EmbeddedImage->CompressMode == EG_EICOMPMODE_RLE) {
            egDecompressIcnsRLE(&CompData, &CompLen, PLPTR(NewImage, a), PixelCount);
        } else {
            egInsertPlane(CompData, PLPTR(NewImage, a), PixelCount);
            CompData += PixelCount;
        }
        
    } else {
        egSetPlane(PLPTR(NewImage, a), WantAlpha ? 255 : 0, PixelCount);
    }
    
    return NewImage;
}

//
// Compositing
//

VOID egRestrictImageArea(IN EG_IMAGE *Image,
                         IN INT64 AreaPosX, IN INT64 AreaPosY,
                         IN OUT INT64 *AreaWidth, IN OUT INT64 *AreaHeight)
{
  if (!Image || !AreaWidth || !AreaHeight) {
    return;
  }
  
  if (AreaPosX >= Image->Width || AreaPosY >= Image->Height) {
    // out of bounds, operation has no effect
    *AreaWidth  = 0;
    *AreaHeight = 0;
  } else {
    // calculate affected area
    if (*AreaWidth > Image->Width - AreaPosX)
      *AreaWidth = Image->Width - AreaPosX;
    if (*AreaHeight > Image->Height - AreaPosY)
      *AreaHeight = Image->Height - AreaPosY;
  }
}

VOID egFillImage(IN OUT EG_IMAGE *CompImage, IN EG_PIXEL *Color)
{
    INTN       i;
    EG_PIXEL    FillColor;
    EG_PIXEL    *PixelPtr;
  if (!CompImage || !Color) {
    return;
  }
  
    FillColor = *Color;
    if (!CompImage->HasAlpha)
        FillColor.a = 0;
    
    PixelPtr = CompImage->PixelData;
    for (i = 0; i < CompImage->Width * CompImage->Height; i++, PixelPtr++)
        *PixelPtr = FillColor;
}

VOID egFillImageArea(IN OUT EG_IMAGE *CompImage,
                     IN INT64 AreaPosX, IN INT64 AreaPosY,
                     IN INT64 AreaWidth, IN INT64 AreaHeight,
                     IN EG_PIXEL *Color)
{
  INTN       x, y;
  EG_PIXEL    FillColor;
  EG_PIXEL    *PixelPtr;
  EG_PIXEL    *PixelBasePtr;
  if (!CompImage || !Color) {
    return;
  }
  
  egRestrictImageArea(CompImage, AreaPosX, AreaPosY, &AreaWidth, &AreaHeight);
  
  if (AreaWidth > 0) {
    FillColor = *Color;
    if (!CompImage->HasAlpha)
      FillColor.a = 0;
    
    PixelBasePtr = CompImage->PixelData + AreaPosY * CompImage->Width + AreaPosX;
    for (y = 0; y < AreaHeight; y++) {
      PixelPtr = PixelBasePtr;
      for (x = 0; x < AreaWidth; x++, PixelPtr++)
        *PixelPtr = FillColor;
      PixelBasePtr += CompImage->Width;
    }
  }
}

VOID egRawCopy(IN OUT EG_PIXEL *CompBasePtr, IN EG_PIXEL *TopBasePtr,
               IN INT64 Width, IN INT64 Height,
               IN INT64 CompLineOffset, IN INT64 TopLineOffset)
{
  INTN       x, y;
  EG_PIXEL    *TopPtr, *CompPtr;
  if (!CompBasePtr || !TopBasePtr) {
    return;
  }
    
  for (y = 0; y < Height; y++) {
    TopPtr = TopBasePtr;
    CompPtr = CompBasePtr;
    for (x = 0; x < Width; x++) {
      *CompPtr = *TopPtr;
      TopPtr++, CompPtr++;
    }
    TopBasePtr += TopLineOffset;
    CompBasePtr += CompLineOffset;
  }
}

VOID egRawCompose(IN OUT EG_PIXEL *CompBasePtr, IN EG_PIXEL *TopBasePtr,
                  IN INT64 Width, IN INT64 Height,
                  IN INT64 CompLineOffset, IN INT64 TopLineOffset)
{
  INT64       x, y;
  EG_PIXEL    *TopPtr, *CompPtr;
  UINTN       Alpha;
  UINTN       RevAlpha;
  UINTN       Temp;
  if (!CompBasePtr || !TopBasePtr) {
    return;
  }
    
  for (y = 0; y < Height; y++) {
    TopPtr = TopBasePtr;
    CompPtr = CompBasePtr;
    for (x = 0; x < Width; x++) {
      Alpha = TopPtr->a;
      RevAlpha = 255 - Alpha;
      Temp = (UINTN)CompPtr->b * RevAlpha + (UINTN)TopPtr->b * Alpha + 0x80;
      CompPtr->b = (UINT8)((Temp + (Temp >> 8)) >> 8);
      Temp = (UINTN)CompPtr->g * RevAlpha + (UINTN)TopPtr->g * Alpha + 0x80;
      CompPtr->g = (UINT8)((Temp + (Temp >> 8)) >> 8);
      Temp = (UINTN)CompPtr->r * RevAlpha + (UINTN)TopPtr->r * Alpha + 0x80;
      CompPtr->r = (UINT8)((Temp + (Temp >> 8)) >> 8);
      /*
       CompPtr->b = ((UINTN)CompPtr->b * RevAlpha + (UINTN)TopPtr->b * Alpha) / 255;
       CompPtr->g = ((UINTN)CompPtr->g * RevAlpha + (UINTN)TopPtr->g * Alpha) / 255;
       CompPtr->r = ((UINTN)CompPtr->r * RevAlpha + (UINTN)TopPtr->r * Alpha) / 255;
       */
      TopPtr++, CompPtr++;
    }
    TopBasePtr += TopLineOffset;
    CompBasePtr += CompLineOffset;
  }
}

VOID egComposeImage(IN OUT EG_IMAGE *CompImage, IN EG_IMAGE *TopImage, IN INT64 PosX, IN INT64 PosY)
{
  INT64       CompWidth, CompHeight;
  if (!TopImage || !CompImage) {
    return;
  }
  
  CompWidth  = TopImage->Width;
  CompHeight = TopImage->Height;
  egRestrictImageArea(CompImage, PosX, PosY, &CompWidth, &CompHeight);
  
  // compose
  if (CompWidth > 0) {
    if (CompImage->HasAlpha) {
      CompImage->HasAlpha = FALSE;
      egSetPlane(PLPTR(CompImage, a), 0, MultU64x64(CompImage->Width, CompImage->Height));
    }
    
    if (TopImage->HasAlpha)
      egRawCompose(CompImage->PixelData + PosY * CompImage->Width + PosX, TopImage->PixelData,
                   CompWidth, CompHeight, CompImage->Width, TopImage->Width);
    else
      egRawCopy(CompImage->PixelData + PosY * CompImage->Width + PosX, TopImage->PixelData,
                CompWidth, CompHeight, CompImage->Width, TopImage->Width);
  }
}

EG_IMAGE * egEnsureImageSize(IN EG_IMAGE *Image, IN INTN Width, IN INTN Height, IN EG_PIXEL *Color)
{
    EG_IMAGE *NewImage;

    if (Image == NULL)
        return NULL;
    if (Image->Width == Width && Image->Height == Height)
        return Image;
    
    NewImage = egCreateFilledImage(Width, Height, Image->HasAlpha, Color);
    if (NewImage == NULL) {
        egFreeImage(Image);
        return NULL;
    }
    egComposeImage(NewImage, Image, 0, 0);
    egFreeImage(Image);
    
    return NewImage;
}

//
// misc internal functions
//

VOID egInsertPlane(IN UINT8 *SrcDataPtr, IN UINT8 *DestPlanePtr, IN UINTN PixelCount)
{
    UINTN i;
  if (!SrcDataPtr || !DestPlanePtr) {
    return;
  }
    
    for (i = 0; i < PixelCount; i++) {
        *DestPlanePtr = *SrcDataPtr++;
        DestPlanePtr += 4;
    }
}

VOID egSetPlane(IN UINT8 *DestPlanePtr, IN UINT8 Value, IN UINT64 PixelCount)
{
    UINT64 i;
  if (!DestPlanePtr) {
    return;
  }
  
    for (i = 0; i < PixelCount; i++) {
        *DestPlanePtr = Value;
        DestPlanePtr += 4;
    }
}

VOID egCopyPlane(IN UINT8 *SrcPlanePtr, IN UINT8 *DestPlanePtr, IN UINTN PixelCount)
{
    UINTN i;
  if (!SrcPlanePtr || !DestPlanePtr) {
    return;
  }
  
    for (i = 0; i < PixelCount; i++) {
        *DestPlanePtr = *SrcPlanePtr;
        DestPlanePtr += 4, SrcPlanePtr += 4;
    }
}

/* EOF */
