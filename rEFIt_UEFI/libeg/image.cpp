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
#include "lodepng.h"

#define MAX_FILE_SIZE (1024*1024*1024)

#ifndef DEBUG_ALL
#define DEBUG_IMG 1
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

EG_IMAGE * egCreateImage(IN INTN Width, IN INTN Height, IN BOOLEAN HasAlpha)
{
  EG_IMAGE        *NewImage;
  
  NewImage = (EG_IMAGE *) AllocatePool(sizeof(EG_IMAGE));
  if (NewImage == NULL)
    return NULL;
  if (Width * Height == 0) {
    FreePool(NewImage);
    return NULL;
  }
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

EG_IMAGE * egCreateFilledImage(IN INTN Width, IN INTN Height, IN BOOLEAN HasAlpha, IN EG_PIXEL *Color)
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
  if (!Image || (Image->Width * Image->Height) == 0) {
    return NULL;
  }

  NewImage = egCreateImage(Image->Width, Image->Height, Image->HasAlpha);
  if (NewImage == NULL)
    return NULL;

  CopyMem(NewImage->PixelData, Image->PixelData, (UINTN)(Image->Width * Image->Height * sizeof(EG_PIXEL)));
  return NewImage;
}

//Scaling functions
EG_IMAGE * egCopyScaledImage(IN EG_IMAGE *OldImage, IN INTN Ratio) //will be N/16
{
  //(c)Slice 2012
  BOOLEAN Grey = FALSE;
  EG_IMAGE    *NewImage;
  INTN        x, x0, x1, x2, y, y0, y1, y2;
  INTN        NewH, NewW;
  EG_PIXEL    *Dest;
  EG_PIXEL    *Src;
  INTN        OldW;

  if (Ratio < 0) {
    Ratio = -Ratio;
    Grey = TRUE;
  }

  if (!OldImage) {
    return NULL;
  }
  Src = OldImage->PixelData;
  OldW = OldImage->Width;

  NewW = (OldImage->Width * Ratio) >> 4;
  NewH = (OldImage->Height * Ratio) >> 4;


  if (Ratio == 16) {
    NewImage = egCopyImage(OldImage);
  } else {
    NewImage = egCreateImage(NewW, NewH, OldImage->HasAlpha);
    if (NewImage == NULL)
      return NULL;

    Dest = NewImage->PixelData;
    for (y = 0; y < NewH; y++) {
      y1 = (y << 4) / Ratio;
      y0 = ((y1 > 0)?(y1-1):y1) * OldW;
      y2 = ((y1 < (OldImage->Height - 1))?(y1+1):y1) * OldW;
      y1 *= OldW;
      for (x = 0; x < NewW; x++) {
        x1 = (x << 4) / Ratio;
        x0 = (x1 > 0)?(x1-1):x1;
        x2 = (x1 < (OldW - 1))?(x1+1):x1;
        Dest->b = (UINT8)(((INTN)Src[x1+y1].b * 2 + Src[x0+y1].b +
                           Src[x2+y1].b + Src[x1+y0].b + Src[x1+y2].b) / 6);
        Dest->g = (UINT8)(((INTN)Src[x1+y1].g * 2 + Src[x0+y1].g +
                           Src[x2+y1].g + Src[x1+y0].g + Src[x1+y2].g) / 6);
        Dest->r = (UINT8)(((INTN)Src[x1+y1].r * 2 + Src[x0+y1].r +
                           Src[x2+y1].r + Src[x1+y0].r + Src[x1+y2].r) / 6);
        Dest->a = Src[x1+y1].a;
        Dest++;
      }
    }
  }
  if (Grey) {
    Dest = NewImage->PixelData;
    for (y = 0; y < NewH; y++) {
      for (x = 0; x < NewW; x++) {
        Dest->b = (UINT8)((INTN)((UINTN)Dest->b + (UINTN)Dest->g + (UINTN)Dest->r) / 3);
        Dest->g = Dest->r = Dest->b;
        Dest++;
      }
    }
  }

  return NewImage;
}

BOOLEAN BigDiff(UINT8 a, UINT8 b)
{
  if (a > b) {
    if (!GlobalConfig.BackgroundDark) {
      return (a - b) > (UINT8)(0xFF - GlobalConfig.BackgroundSharp);
    }
  } else if (GlobalConfig.BackgroundDark) {
    return (b - a) > (UINT8)(0xFF - GlobalConfig.BackgroundSharp);
  }
  return 0;
}
//(c)Slice 2013
#define EDGE(P) \
do { \
  if (BigDiff(a11.P, a10.P)) { \
    if (!BigDiff(a11.P, a01.P) && !BigDiff(a11.P, a21.P)) { \
      a10.P = a11.P; \
    } else if (BigDiff(a11.P, a01.P)) { \
      if ((dx + dy) < cell) { \
        a11.P = a21.P = a12.P = (UINT8)((a10.P * (cell - dy + dx) + a01.P * (cell - dx + dy)) / (cell * 2)); \
      } else { \
        a10.P = a01.P = a11.P; \
      } \
    } else if (BigDiff(a11.P, a21.P)) { \
      if (dx > dy) { \
        a11.P = a01.P = a12.P = (UINT8)((a10.P * (cell * 2 - dy - dx) + a21.P * (dx + dy)) / (cell * 2)); \
      }else { \
        a10.P = a21.P = a11.P; \
      } \
    } \
  } else if (BigDiff(a11.P, a21.P)) { \
    if (!BigDiff(a11.P, a12.P)){ \
      a21.P = a11.P; \
    } else { \
      if ((dx + dy) > cell) { \
        a11.P = a01.P = a10.P = (UINT8)((a21.P * (cell + dx - dy) + a12.P * (cell - dx + dy)) / (cell * 2)); \
      } else { \
        a21.P = a12.P = a11.P; \
      } \
    } \
  } else if (BigDiff(a11.P, a01.P)) { \
    if (!BigDiff(a11.P, a12.P)){ \
      a01.P = a11.P; \
    } else { \
      if (dx < dy) { \
        a11.P = a21.P = a10.P = (UINT8)((a01.P * (cell * 2 - dx - dy) + a12.P * (dy + dx )) / (cell * 2)); \
      } else { \
        a01.P = a12.P = a11.P; \
      } \
    } \
  } else if (BigDiff(a11.P, a12.P)) { \
    a12.P = a11.P; \
  } \
} while(0)

#define SMOOTH(P) \
do { \
  norm = (INTN)a01.P + a10.P + 4 * a11.P + a12.P + a21.P; \
  if (norm == 0) { \
    Dest->P = 0; \
  } else { \
    Dest->P = (UINT8)(a11.P * 2 * (a01.P * (cell - dx) + a10.P * (cell - dy) + \
                      a21.P * dx + a12.P * dy + a11.P * 2 * cell) / (cell * norm)); \
  } \
} while(0)

#define SMOOTH2(P) \
do { \
     Dest->P = (UINT8)((a01.P * (cell - dx) * 3 + a10.P * (cell - dy) * 3 + \
                        a21.P * dx * 3 + a12.P * dy * 3 + a11.P * 2 * cell) / (cell * 8)); \
} while(0)


VOID  ScaleImage(OUT EG_IMAGE *NewImage, IN EG_IMAGE *OldImage)
{
  INTN      W1, W2, H1, H2, i, j, f, cell;
  INTN      x, dx, y, y1, dy; //, norm;
  EG_PIXEL  a10, a11, a12, a01, a21;
  EG_PIXEL  *Src = OldImage->PixelData;
  EG_PIXEL  *Dest = NewImage->PixelData;

  W1 = OldImage->Width;
  H1 = OldImage->Height;
  W2 = NewImage->Width;
  H2 = NewImage->Height;
  if (H1 * W2 < H2 * W1) {
    f = (H2 << 12) / H1;
  } else {
    f = (W2 << 12) / W1;
  }
  if (f == 0) return;
  cell = ((f - 1) >> 12) + 1;

  for (j = 0; j < H2; j++) {
    y = (j << 12) / f;
    y1 = y * W1;
    dy = j - ((y * f) >> 12);

    for (i = 0; i < W2; i++) {
      x = (i << 12) / f;
      dx = i - ((x * f) >> 12);
      a11 = Src[x + y1];
      a10 = (y == 0)?a11: Src[x + y1 - W1];
      a01 = (x == 0)?a11: Src[x + y1 - 1];
      a21 = (x >= W1)?a11: Src[x + y1 + 1];
      a12 = (y >= H1)?a11: Src[x + y1 + W1];

      if (a11.a == 0) {
        Dest->r = Dest->g = Dest->b = 0x55;
      } else {

        EDGE(r);
        EDGE(g);
        EDGE(b);

        SMOOTH2(r);
        SMOOTH2(g);
        SMOOTH2(b);
      }

      Dest->a = 0xFF;
      Dest++;
    }
  }
}
//

VOID egFreeImage(IN EG_IMAGE *Image)
{
  if (Image != NULL) {
    if (Image->PixelData != NULL) {
      FreePool(Image->PixelData);
      Image->PixelData = NULL; //FreePool will not zero pointer
    }
    FreePool(Image);
  }
}

//
// Basic file operations
//
EFI_STATUS egLoadFile(IN EFI_FILE_HANDLE BaseDir, IN CONST CHAR16 *FileName,
                      OUT UINT8 **FileData, OUT UINTN *FileDataLength)
{
  EFI_STATUS          Status = EFI_NOT_FOUND;
  EFI_FILE_HANDLE     FileHandle = 0;
  EFI_FILE_INFO       *FileInfo;
  UINT64              ReadSize;
  UINTN               BufferSize;
  UINT8               *Buffer;

  if (!BaseDir) {
    goto Error;
  }

  Status = BaseDir->Open(BaseDir, &FileHandle, (CHAR16*)FileName, EFI_FILE_MODE_READ, 0); // const missing in EFI_FILE_HANDLE->Open
  if (EFI_ERROR(Status) || !FileHandle) {
    goto Error;
  }

  FileInfo = EfiLibFileInfo(FileHandle);
  if (FileInfo == NULL) {
    FileHandle->Close(FileHandle);
    goto Error;
  }
  ReadSize = FileInfo->FileSize;
  if (ReadSize > MAX_FILE_SIZE)
    ReadSize = MAX_FILE_SIZE;
  FreePool(FileInfo);

  BufferSize = (UINTN)ReadSize;   // was limited to 1 GB above, so this is safe
  Buffer = (UINT8 *) AllocatePool (BufferSize);
  if (Buffer == NULL) {
    FileHandle->Close(FileHandle);
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }

  Status = FileHandle->Read(FileHandle, &BufferSize, Buffer);
  FileHandle->Close(FileHandle);
  if (EFI_ERROR(Status)) {
    FreePool(Buffer);
    goto Error;
  }

  if(FileData) {
    *FileData = Buffer;
  }
  if (FileDataLength) {
    *FileDataLength = BufferSize;
  }
  return Status;
Error:
  if (FileData) {
    *FileData = NULL;
  }
  if (FileDataLength) {
    *FileDataLength = 0;
  }
  return Status;
}
//Slice - this is gEfiPartTypeSystemPartGuid
//static EFI_GUID ESPGuid = { 0xc12a7328, 0xf81f, 0x11d2, { 0xba, 0x4b, 0x00, 0xa0, 0xc9, 0x3e, 0xc9, 0x3b } };
//there is assumed only one ESP partition. What if there are two HDD gpt formatted?
EFI_STATUS egFindESP(OUT EFI_FILE_HANDLE *RootDir)
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
EFI_STATUS egSaveFile(IN EFI_FILE_HANDLE BaseDir OPTIONAL, IN CONST CHAR16 *FileName,
                      IN CONST VOID *FileData, IN UINTN FileDataLength)
{
  EFI_STATUS          Status;
  EFI_FILE_HANDLE     FileHandle;
  UINTN               BufferSize;
  BOOLEAN             CreateNew = TRUE;
  CONST CHAR16              *p = FileName + StrLen(FileName);
  CHAR16              DirName[256];
  UINTN               dirNameLen;

  if (BaseDir == NULL) {
    Status = egFindESP(&BaseDir);
    if (EFI_ERROR(Status)) {
      DBG("no ESP %r\n", Status);
      return Status;
    }
  }
    
  // syscl - make directory if not exist
  while (*p != L'\\' && p >= FileName) {
    // find the first '\\' traverse from the end to head of FileName
    p -= 1;
  }
  dirNameLen = p - FileName;
  StrnCpy(DirName, FileName, dirNameLen);
  Status = BaseDir->Open(BaseDir, &FileHandle, DirName,
                           EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, EFI_FILE_DIRECTORY);
    
  if (EFI_ERROR(Status)) {
      // make dir
//    DBG("no dir %r\n", Status);
      Status = BaseDir->Open(BaseDir, &FileHandle, DirName,
                               EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, EFI_FILE_DIRECTORY);
//    DBG("cant make dir %r\n", Status);
  }
  // end of folder checking

  // Delete existing file if it exists
  Status = BaseDir->Open(BaseDir, &FileHandle, FileName,
                         EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
  if (!EFI_ERROR(Status)) {
    Status = FileHandle->Delete(FileHandle);
    if (Status == EFI_WARN_DELETE_FAILURE) {
      //This is READ_ONLY file system
      CreateNew = FALSE; // will write into existing file
//      DBG("RO FS %r\n", Status);
    }
  }

  if (CreateNew) {
    // Write new file
    Status = BaseDir->Open(BaseDir, &FileHandle, FileName,
                           EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
    if (EFI_ERROR(Status)) {
//      DBG("no write %r\n", Status);
      return Status;
    }
  } else {
    //to write into existing file we must sure it size larger then our data
    EFI_FILE_INFO *Info = EfiLibFileInfo(FileHandle);
    if (Info) {
      if (Info->FileSize < FileDataLength) {
//        DBG("no old file %r\n", Status);
        return EFI_NOT_FOUND;
      }
      FreePool(Info);
    }
  }

  if (!FileHandle) {
//    DBG("no FileHandle %r\n", Status);
    return EFI_DEVICE_ERROR;
  }

  BufferSize = FileDataLength;
  Status = FileHandle->Write(FileHandle, &BufferSize, (VOID*)FileData); // CONST missing in EFI_FILE_HANDLE->write
  FileHandle->Close(FileHandle);
//  DBG("not written %r\n", Status);
  return Status;
}


EFI_STATUS egMkDir(IN EFI_FILE_HANDLE BaseDir OPTIONAL, IN CHAR16 *DirName)
{
  EFI_STATUS          Status;
  EFI_FILE_HANDLE     FileHandle;

  //DBG("Looking up dir assets (%s):", DirName);

  if (BaseDir == NULL) {
    Status = egFindESP(&BaseDir);
    if (EFI_ERROR(Status)) {
      //DBG(" %r\n", Status);
      return Status;
    }
  }

  Status = BaseDir->Open(BaseDir, &FileHandle, DirName,
                         EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, EFI_FILE_DIRECTORY);

  if (EFI_ERROR(Status)) {
    // Write new dir
    //DBG("%r, attempt to create one:", Status);
    Status = BaseDir->Open(BaseDir, &FileHandle, DirName,
                           EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, EFI_FILE_DIRECTORY);
  }

  //DBG(" %r\n", Status);
  return Status;
}

//caller is responsible for free image
EG_IMAGE * egLoadImage(IN EFI_FILE_HANDLE BaseDir, IN CONST CHAR16 *FileName, IN BOOLEAN WantAlpha)
{
  EFI_STATUS      Status;
  UINT8           *FileData = NULL;
  UINTN           FileDataLength = 0;
  EG_IMAGE        *NewImage;

  if (GlobalConfig.TypeSVG) {
    return NULL;
  }

  if (BaseDir == NULL || FileName == NULL)
    return NULL;

  // load file
  Status = egLoadFile(BaseDir, FileName, &FileData, &FileDataLength);
  if (EFI_ERROR(Status))
    return NULL;

  // decode it
  NewImage = egDecodePNG(FileData, FileDataLength, WantAlpha);

  if (!NewImage) {
    DBG("%s not decoded\n", FileName);
  }
  FreePool(FileData);
  return NewImage;
}

//caller is responsible for free image
EG_IMAGE * egLoadIcon(IN EFI_FILE_HANDLE BaseDir, IN CONST CHAR16 *FileName, IN UINTN IconSize)
{
  EFI_STATUS      Status;
  UINT8           *FileData;
  UINTN           FileDataLength;
  EG_IMAGE        *NewImage;
  CHAR8           *IconName;

  if (!BaseDir || !FileName) {
    return NULL;
  }

  if (GlobalConfig.TypeSVG) {
    INTN    i = 0;
    UINTN   Size;

    CONST CHAR16 *ptr = StrStr(FileName, L"\\");
    if (!ptr) {
      ptr = FileName;
    } else {
      ptr++;
    }
    CONST CHAR16 *ptr2 = StrStr(ptr, L".");
    Size = ptr2 - ptr + 2;
    IconName = (__typeof__(IconName))AllocateZeroPool(Size);
    UnicodeStrToAsciiStrS(ptr, IconName, Size - 1);

    while (OSIconsTable[i].name) {
      if (AsciiStrCmp(OSIconsTable[i].name, IconName) == 0) {
//        DBG("theme defined %a\n", IconName);
//        DBG(" icon size=[%d,%d]\n", OSIconsTable[i].image->Width, OSIconsTable[i].image->Height);
        FreePool(IconName);
        return OSIconsTable[i].image;
      }
      i++;
    }
    FreePool(IconName);
    return NULL;
  }
  // load file
  Status = egLoadFile(BaseDir, FileName, &FileData, &FileDataLength);
  if (EFI_ERROR(Status)) {
    return NULL;
  }

  // decode it
  NewImage = egDecodePNG(FileData, FileDataLength, TRUE);
//  if (!NewImage) {
//    NewImage = egDecodeICNS(FileData, FileDataLength, IconSize, TRUE);
//  }
  
  FreePool(FileData);
  return NewImage;
}

//
// Compositing
//

VOID egRestrictImageArea(IN EG_IMAGE *Image,
                         IN INTN AreaPosX, IN INTN AreaPosY,
                         IN OUT INTN *AreaWidth, IN OUT INTN *AreaHeight)
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
  for (i = 0; i < CompImage->Width * CompImage->Height; i++) {
    *PixelPtr++ = FillColor;
  }
}

VOID egFillImageArea(IN OUT EG_IMAGE *CompImage,
                     IN INTN AreaPosX, IN INTN AreaPosY,
                     IN INTN AreaWidth, IN INTN AreaHeight,
                     IN EG_PIXEL *Color)
{
  INTN        x, y;
  INTN    xAreaWidth = AreaWidth;
  INTN    xAreaHeight = AreaHeight;
  EG_PIXEL    FillColor;
  EG_PIXEL    *PixelBasePtr;
  if (!CompImage || !Color) {
    return;
  }

  egRestrictImageArea(CompImage, AreaPosX, AreaPosY, &xAreaWidth, &xAreaHeight);

  if (xAreaWidth > 0) {
    FillColor = *Color;
    if (!CompImage->HasAlpha)
      FillColor.a = 0;

    PixelBasePtr = CompImage->PixelData + AreaPosY * CompImage->Width + AreaPosX;
    for (y = 0; y < xAreaHeight; y++) {
      EG_PIXEL    *PixelPtr = PixelBasePtr;
      for (x = 0; x < xAreaWidth; x++) {
        *PixelPtr++ = FillColor;
      }
      PixelBasePtr += CompImage->Width;
    }
  }
}

VOID egRawCopy(IN OUT EG_PIXEL *CompBasePtr, IN EG_PIXEL *TopBasePtr,
               IN INTN Width, IN INTN Height,
               IN INTN CompLineOffset, IN INTN TopLineOffset)
{
  INTN       x, y;

  if (!CompBasePtr || !TopBasePtr) {
    return;
  }

  for (y = 0; y < Height; y++) {
    EG_PIXEL    *TopPtr = TopBasePtr;
    EG_PIXEL    *CompPtr = CompBasePtr;
    for (x = 0; x < Width; x++) {
      *CompPtr = *TopPtr;
      TopPtr++, CompPtr++;
    }
    TopBasePtr += TopLineOffset;
    CompBasePtr += CompLineOffset;
  }
}

VOID egRawCompose(IN OUT EG_PIXEL *CompBasePtr, IN EG_PIXEL *TopBasePtr,
                  IN INTN Width, IN INTN Height,
                  IN INTN CompLineOffset, IN INTN TopLineOffset)
{
  INT64       x, y;
  EG_PIXEL    *TopPtr, *CompPtr;
    //To make native division we need INTN types
  INTN      TopAlpha;
  INTN      Alpha;
  INTN      CompAlpha;
  INTN      RevAlpha;
  INTN      TempAlpha;
//  EG_PIXEL    *CompUp;
  if (!CompBasePtr || !TopBasePtr) {
    return;
  }
//  CompUp = CompBasePtr + Width * Height;
  //Slice - my opinion
//if TopAlpha=255 then draw Top - non transparent
//else if TopAlpha=0 then draw Comp - full transparent
//else draw mixture |-----comp---|--top--|
//final alpha =(1-(1-x)*(1-y)) =(255*255-(255-topA)*(255-compA))/255

  for (y = 0; y < Height; y++) {
    TopPtr = TopBasePtr;
    CompPtr = CompBasePtr;
    for (x = 0; x < Width; x++) {
      TopAlpha = TopPtr->a & 0xFF; //exclude sign

      if (TopAlpha == 255) {
        CompPtr->b = TopPtr->b;
        CompPtr->g = TopPtr->g;
        CompPtr->r = TopPtr->r;
        CompPtr->a = (UINT8)TopAlpha;
      } else if (TopAlpha != 0) {
        CompAlpha = CompPtr->a & 0xFF;
        RevAlpha = 255 - TopAlpha;
        TempAlpha = CompAlpha * RevAlpha;
        TopAlpha *= 255;
        Alpha = TopAlpha + TempAlpha;

        CompPtr->b = (UINT8)((TopPtr->b * TopAlpha + CompPtr->b * TempAlpha) / Alpha);
        CompPtr->g = (UINT8)((TopPtr->g * TopAlpha + CompPtr->g * TempAlpha) / Alpha);
        CompPtr->r = (UINT8)((TopPtr->r * TopAlpha + CompPtr->r * TempAlpha) / Alpha);
        CompPtr->a = (UINT8)(Alpha / 255);
      }
      TopPtr++, CompPtr++;
    }
    TopBasePtr += TopLineOffset;
    CompBasePtr += CompLineOffset;
  }
}

// This is simplified image composing on solid background. egComposeImage will decide which method to use
VOID egRawComposeOnFlat(IN OUT EG_PIXEL *CompBasePtr, IN EG_PIXEL *TopBasePtr,
                  IN INTN Width, IN INTN Height,
                  IN INTN CompLineOffset, IN INTN TopLineOffset)
{
  INT64       x, y;
  EG_PIXEL    *TopPtr, *CompPtr;
  UINT32      TopAlpha;
  UINT32      RevAlpha;
  UINTN       Temp;

  if (!CompBasePtr || !TopBasePtr) {
    return;
  }

  for (y = 0; y < Height; y++) {
    TopPtr = TopBasePtr;
    CompPtr = CompBasePtr;
    for (x = 0; x < Width; x++) {
      TopAlpha = TopPtr->a;
      RevAlpha = 255 - TopAlpha;

      Temp = ((UINT8)CompPtr->b * RevAlpha) + ((UINT8)TopPtr->b * TopAlpha);
      CompPtr->b = (UINT8)(Temp / 255);

      Temp = ((UINT8)CompPtr->g * RevAlpha) + ((UINT8)TopPtr->g * TopAlpha);
      CompPtr->g = (UINT8)(Temp / 255);

      Temp = ((UINT8)CompPtr->r * RevAlpha) + ((UINT8)TopPtr->r * TopAlpha);
      CompPtr->r = (UINT8)(Temp / 255);

      CompPtr->a = (UINT8)(255);

      TopPtr++, CompPtr++;
    }
    TopBasePtr += TopLineOffset;
    CompBasePtr += CompLineOffset;
  }
}

VOID egComposeImage(IN OUT EG_IMAGE *CompImage, IN EG_IMAGE *TopImage, IN INTN PosX, IN INTN PosY)
{
  INTN       CompWidth, CompHeight;
  if (!TopImage || !CompImage) {
    return;
  }

  CompWidth  = TopImage->Width;
  CompHeight = TopImage->Height;
  egRestrictImageArea(CompImage, PosX, PosY, &CompWidth, &CompHeight);

  // compose
  if (CompWidth > 0) {
    if (CompImage->HasAlpha && !BackgroundImage) {
       CompImage->HasAlpha = FALSE;
    }

    if (TopImage->HasAlpha) {
      if (CompImage->HasAlpha) {  //aaaa
        egRawCompose(CompImage->PixelData + PosY * CompImage->Width + PosX,
                     TopImage->PixelData,
                     CompWidth, CompHeight, CompImage->Width, TopImage->Width);
      } else {
        egRawComposeOnFlat(CompImage->PixelData + PosY * CompImage->Width + PosX,
                           TopImage->PixelData,
                           CompWidth, CompHeight, CompImage->Width, TopImage->Width);
      }
    } else {
      egRawCopy(CompImage->PixelData + PosY * CompImage->Width + PosX,
                TopImage->PixelData,
                CompWidth, CompHeight, CompImage->Width, TopImage->Width);
    }
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
//these functions used for icns, not with png
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


EG_IMAGE * egDecodePNG(IN UINT8 *FileData, IN UINTN FileDataLength, IN BOOLEAN WantAlpha) {
  EG_IMAGE *NewImage = NULL;
  UINTN Error, i, ImageSize, Width, Height;
  EG_PIXEL *PixelData;
  EG_PIXEL *Pixel, *PixelD;

  Error = eglodepng_decode((UINT8**) &PixelData, &Width, &Height, (CONST UINT8*) FileData, (UINTN) FileDataLength);

  if (Error) {
    /*
     * Error 28 incorrect PNG signature ok, because also called on ICNS files
     */
    if (Error != 28U) {
      DBG("egDecodePNG(%p, %lu, %c): eglodepng_decode failed with error %lu\n",
          FileData, FileDataLength, WantAlpha?'Y':'N', Error);
    }
    return NULL;
  }
  if (!PixelData || Width > 4096U || Height > 4096U) {
    DBG("egDecodePNG(%p, %lu, %c): eglodepng_decode returned suspect values, PixelData %p, Width %lu, Height %lu\n",
        FileData, FileDataLength, WantAlpha?'Y':'N', PixelData, Width, Height);
  }

  NewImage = egCreateImage(Width, Height, WantAlpha);
  if (NewImage == NULL) return NULL;

  ImageSize = (Width * Height);
//  CopyMem(NewImage->PixelData, PixelData, sizeof(EG_PIXEL) * ImageSize);
//  lodepng_free(PixelData);

  Pixel = (EG_PIXEL*)NewImage->PixelData;
  PixelD = PixelData;
  for (i = 0; i < ImageSize; i++) {
/*      UINT8 Temp;
      Temp = Pixel->b;
      Pixel->b = Pixel->r;
      Pixel->r = Temp; */
    Pixel->b = PixelD->r; //change r <-> b
    Pixel->r = PixelD->b;
    Pixel->g = PixelD->g;
    Pixel->a = PixelD->a; // 255 is opaque, 0 - transparent
    Pixel++;
    PixelD++;
  }

  lodepng_free(PixelData);
  return NewImage;
}


/* EOF */
