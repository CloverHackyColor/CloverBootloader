/** @file
  This file defines BMP file header data structures.

Copyright (c) 2006 - 2011, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _BMP_H_
#define _BMP_H_

#pragma pack(1)

typedef struct {
  UINT8   Blue;
  UINT8   Green;
  UINT8   Red;
  UINT8   Reserved;
} BMP_COLOR_MAP;

typedef struct {
  CHAR8         CharB;  //B
  CHAR8         CharM;  //M
  UINT32        Size;   // 8A 44 01 00
  UINT16        Reserved[2]; // 00 00 00 00
  UINT32        ImageOffset; // 8A 00 00 00
  UINT32        HeaderSize;  // 7C 00 00 00
  INT32        PixelWidth;  // 90 00 00 00
  INT32        PixelHeight; // 70 FF FF FF
  UINT16        Planes;          // 01 00  //< Must be 1
  UINT16        BitPerPixel;     // 20 00  //< 1, 4, 8, or 24
  UINT32        CompressionType; // 03 00 0000
  UINT32        ImageSize;       // 00 44 01 00 //< Compressed image size in bytes
  UINT32        XPixelsPerMeter; //
  UINT32        YPixelsPerMeter;
  UINT32        NumberOfColors;
  UINT32        ImportantColors;
} BMP_IMAGE_HEADER;

#pragma pack()

#endif
