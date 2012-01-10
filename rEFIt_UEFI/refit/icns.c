/*
 * refit/icns.c
 * Loader for .icns icon files
 *
 * Copyright (c) 2006-2007 Christoph Pfisterer
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

#include "lib.h"

//
// well-known icons
//

typedef struct {
    EG_IMAGE    *Image;
    CHAR16      *Path;
    UINTN       PixelSize;
} BUILTIN_ICON;

BUILTIN_ICON BuiltinIconTable[BUILTIN_ICON_COUNT] = {
    { NULL, L"icons\\func_about.icns", 48 },
    { NULL, L"icons\\func_reset.icns", 48 },
    { NULL, L"icons\\func_shutdown.icns", 48 },
    { NULL, L"icons\\tool_shell.icns", 48 },
    { NULL, L"icons\\tool_part.icns", 48 },
    { NULL, L"icons\\tool_rescue.icns", 48 },
    { NULL, L"icons\\vol_internal.icns", 32 },
    { NULL, L"icons\\vol_external.icns", 32 },
    { NULL, L"icons\\vol_optical.icns", 32 },
};

EG_IMAGE * BuiltinIcon(IN UINTN Id)
{
    if (Id >= BUILTIN_ICON_COUNT)
        return NULL;
    
    if (BuiltinIconTable[Id].Image == NULL)
        BuiltinIconTable[Id].Image = LoadIcnsFallback(SelfDir, BuiltinIconTable[Id].Path, BuiltinIconTable[Id].PixelSize);
    
    return BuiltinIconTable[Id].Image;
}

//
// Load an icon for an operating system
//

EG_IMAGE * LoadOSIcon(IN CHAR16 *OSIconName OPTIONAL, IN CHAR16 *FallbackIconName, BOOLEAN BootLogo)
{
    EG_IMAGE        *Image;
    CHAR16          CutoutName[16];
    CHAR16          FileName[256];
    UINTN           StartIndex, Index, NextIndex;
    
    if (GlobalConfig.TextOnly)      // skip loading if it's not used anyway
        return NULL;
    Image = NULL;
    
    // try the names from OSIconName
    for (StartIndex = 0; OSIconName != NULL && OSIconName[StartIndex]; StartIndex = NextIndex) {
        // find the next name in the list
        NextIndex = 0;
        for (Index = StartIndex; OSIconName[Index]; Index++) {
            if (OSIconName[Index] == ',') {
                NextIndex = Index + 1;
                break;
            }
        }
        if (OSIconName[Index] == 0)
            NextIndex = Index;
        
        // construct full path
        if (Index > StartIndex + 15)   // prevent buffer overflow
            continue;
        CopyMem(CutoutName, OSIconName + StartIndex, (Index - StartIndex) * sizeof(CHAR16));
        CutoutName[Index - StartIndex] = 0;
        SPrint(FileName, 255, L"icons\\%s_%s.icns",
               BootLogo ? L"boot" : L"os", CutoutName);
        
        // try to load it
        Image = egLoadIcon(SelfDir, FileName, 128);
        if (Image != NULL)
            return Image;
    }
    
    // try the fallback name
    SPrint(FileName, 255, L"icons\\%s_%s.icns",
           BootLogo ? L"boot" : L"os", FallbackIconName);
    Image = egLoadIcon(SelfDir, FileName, 128);
    if (Image != NULL)
        return Image;
    
    // try the fallback name with os_ instead of boot_
    if (BootLogo)
        return LoadOSIcon(NULL, FallbackIconName, FALSE);
    
    return DummyImage(128);
}

//
// Load an image from a .icns file
//

EG_IMAGE * LoadIcns(IN EFI_FILE_HANDLE BaseDir, IN CHAR16 *FileName, IN UINTN PixelSize)
{
    if (GlobalConfig.TextOnly)      // skip loading if it's not used anyway
        return NULL;
    return egLoadIcon(BaseDir, FileName, PixelSize);
}

static EG_PIXEL BlackPixel  = { 0x00, 0x00, 0x00, 0 };
//static EG_PIXEL YellowPixel = { 0x00, 0xff, 0xff, 0 };

EG_IMAGE * DummyImage(IN UINTN PixelSize)
{
    EG_IMAGE        *Image;
    UINTN           x, y, LineOffset;
    CHAR8           *Ptr, *YPtr;
    
    Image = egCreateFilledImage(PixelSize, PixelSize, TRUE, &BlackPixel);
    
    LineOffset = PixelSize * 4;
    
    YPtr = (CHAR8 *)Image->PixelData + ((PixelSize - 32) >> 1) * (LineOffset + 4);
    for (y = 0; y < 32; y++) {
        Ptr = YPtr;
        for (x = 0; x < 32; x++) {
            if (((x + y) % 12) < 6) {
                *Ptr++ = 0;
                *Ptr++ = 0;
                *Ptr++ = 0;
            } else {
                *Ptr++ = 0;
                *Ptr++ = 255;
                *Ptr++ = 255;
            }
            *Ptr++ = 144;
        }
        YPtr += LineOffset;
    }
    
    return Image;
}

EG_IMAGE * LoadIcnsFallback(IN EFI_FILE_HANDLE BaseDir, IN CHAR16 *FileName, IN UINTN PixelSize)
{
    EG_IMAGE *Image;
    
    if (GlobalConfig.TextOnly)      // skip loading if it's not used anyway
        return NULL;
    
    Image = LoadIcns(BaseDir, FileName, PixelSize);
    if (Image == NULL)
        Image = DummyImage(PixelSize);
    return Image;
}
