/*
 * libeg/load_icns.c
 * Loading function for .icns Apple icon images
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
//#if !defined(LODEPNG)
#include "libegint.h"
#include "XImage.h"

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

#define PLPTR(imagevar, colorname) ((UINT8 *) &((imagevar).GetPixelPtr(0,0)->colorname))


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
    DestPlanePtr += 4; SrcPlanePtr += 4;
  }
}


//
// Decompress .icns RLE data
//

VOID egDecompressIcnsRLE(IN OUT UINT8 **CompData, IN OUT UINTN *CompLen, IN UINT8 *PixelData, IN UINTN PixelCount)
{
    UINT8 *cp;
    UINT8 *cp_end;
    UINT8 *pp;
    UINTN pp_left;
    UINTN len, i;
    UINT8 value;
    
    // setup variables
    cp = *CompData;
    cp_end = cp + *CompLen;
    pp = PixelData;
    pp_left = PixelCount;
    
    // decode
    while (cp + 1 < cp_end && pp_left > 0) {
        len = *cp++;
        if (len & 0x80) {   // compressed data: repeat next byte
            len -= 125;
            if (len > pp_left)
                break;
            value = *cp++;
            for (i = 0; i < len; i++) {
                *pp = value;
                pp += 4;
            }
        } else {            // uncompressed data: copy bytes
            len++;
            if (len > pp_left || cp + len > cp_end)
                break;
            for (i = 0; i < len; i++) {
                *pp = *cp++;
                pp += 4;
            }
        }
        pp_left -= len;
    }
    
  if (pp_left > 0) {
	  DBG(" egDecompressIcnsRLE: still need %llu bytes of pixel data\n", pp_left);
  }
    
    // record what's left of the compressed data stream
    *CompData = cp;
    *CompLen = (UINTN)(cp_end - cp);
}

//
// Load Apple .icns icons
//

EFI_STATUS XImage::FromICNS(IN UINT8 *FileData, IN UINTN FileDataLength, IN UINTN IconSize)
{
    UINT8               *Ptr, *BufferEnd, *DataPtr, *MaskPtr;
    UINT32              BlockLen, DataLen, MaskLen;
    UINTN               FetchPixelSize, PixelCount, i;
    UINT8               *CompData;
    UINTN               CompLen;
    UINT8               *SrcPtr;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL            *DestPtr;
    
    if (FileDataLength < 8 || FileData == NULL ||
        FileData[0] != 'i' || FileData[1] != 'c' || FileData[2] != 'n' || FileData[3] != 's') {
        // not an icns file...
      DBG("not icns\n");
      return EFI_NOT_FOUND; //it is null at this moment
    }
    
    FetchPixelSize = IconSize;
    for (;;) {
        DataPtr = NULL;
        DataLen = 0;
        MaskPtr = NULL;
        MaskLen = 0;
        
        Ptr = FileData + 8;
        BufferEnd = FileData + FileDataLength;
        // iterate over tagged blocks in the file
        while (Ptr + 8 <= BufferEnd) {
            BlockLen = ((UINT32)Ptr[4] << 24) + ((UINT32)Ptr[5] << 16) + ((UINT32)Ptr[6] << 8) + (UINT32)Ptr[7];
            if (Ptr + BlockLen > BufferEnd)   // block continues beyond end of file
                break;
            
            // extract the appropriate blocks for each pixel size
            if (FetchPixelSize == 128) {
                if (Ptr[0] == 'i' && Ptr[1] == 't' && Ptr[2] == '3' && Ptr[3] == '2') {
                    if (Ptr[8] == 0 && Ptr[9] == 0 && Ptr[10] == 0 && Ptr[11] == 0) {
                        DataPtr = Ptr + 12;
                        DataLen = BlockLen - 12;
                    }
                } else if (Ptr[0] == 't' && Ptr[1] == '8' && Ptr[2] == 'm' && Ptr[3] == 'k') {
                    MaskPtr = Ptr + 8;
                    MaskLen = BlockLen - 8;
                }
                
            } else if (FetchPixelSize == 48) {
                if (Ptr[0] == 'i' && Ptr[1] == 'h' && Ptr[2] == '3' && Ptr[3] == '2') {
                    DataPtr = Ptr + 8;
                    DataLen = BlockLen - 8;
                } else if (Ptr[0] == 'h' && Ptr[1] == '8' && Ptr[2] == 'm' && Ptr[3] == 'k') {
                    MaskPtr = Ptr + 8;
                    MaskLen = BlockLen - 8;
                }
                
            } else if (FetchPixelSize == 32) {
                if (Ptr[0] == 'i' && Ptr[1] == 'l' && Ptr[2] == '3' && Ptr[3] == '2') {
                    DataPtr = Ptr + 8;
                    DataLen = BlockLen - 8;
                } else if (Ptr[0] == 'l' && Ptr[1] == '8' && Ptr[2] == 'm' && Ptr[3] == 'k') {
                    MaskPtr = Ptr + 8;
                    MaskLen = BlockLen - 8;
                }
                
            } else if (FetchPixelSize == 16) {
                if (Ptr[0] == 'i' && Ptr[1] == 's' && Ptr[2] == '3' && Ptr[3] == '2') {
                    DataPtr = Ptr + 8;
                    DataLen = BlockLen - 8;
                } else if (Ptr[0] == 's' && Ptr[1] == '8' && Ptr[2] == 'm' && Ptr[3] == 'k') {
                    MaskPtr = Ptr + 8;
                    MaskLen = BlockLen - 8;
                }
                
            }
            
            Ptr += BlockLen;
        }
        //TODO - take different larger size if not found
        // FUTURE: try to load a different size and scale it later
            if (DataPtr == NULL && FetchPixelSize == 128) {
                FetchPixelSize = 32;
                continue;
            }
        
        break;
    }
    
  if (DataPtr == NULL) {
    DBG("not found such IconSize\n");
    return EFI_NOT_FOUND;   // no image found
  }
    
    // allocate image structure and buffer
//    NewImage = egCreateImage(FetchPixelSize, FetchPixelSize, WantAlpha);
 //   if (NewImage == NULL)
 //       return NULL;
    setSizeInPixels(FetchPixelSize, FetchPixelSize);
    PixelCount = FetchPixelSize * FetchPixelSize;
    
    if (DataLen < PixelCount * 3) {
        
        // pixel data is compressed, RGB planar
        CompData = DataPtr;
        CompLen  = DataLen;
        egDecompressIcnsRLE(&CompData, &CompLen, PLPTR(*this, Red), PixelCount);
        egDecompressIcnsRLE(&CompData, &CompLen, PLPTR(*this, Green), PixelCount);
        egDecompressIcnsRLE(&CompData, &CompLen, PLPTR(*this, Blue), PixelCount);
        // possible assertion: CompLen == 0
        if (CompLen > 0) {
          DBG(" egLoadICNSIcon: %llu bytes of compressed data left\n", CompLen);
        }
        
    } else {
        
        // pixel data is uncompressed, RGB interleaved
        SrcPtr  = DataPtr;
        DestPtr = GetPixelPtr(0,0);
        for (i = 0; i < PixelCount; i++, DestPtr++) {
            DestPtr->Red = *SrcPtr++;
            DestPtr->Green = *SrcPtr++;
            DestPtr->Blue = *SrcPtr++;
        }
        
    }
    
    // add/set alpha plane
    if (MaskPtr != NULL && MaskLen >= PixelCount)
        egInsertPlane(MaskPtr, PLPTR(*this, Reserved), PixelCount);
    else
        egSetPlane(PLPTR(*this, Reserved),  255, PixelCount);
    
    // FUTURE: scale to originally requested size if we had to load another size
    
    return EFI_SUCCESS;
}
//#endif
/* EOF */
