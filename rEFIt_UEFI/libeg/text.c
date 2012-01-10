/*
 * libeg/text.c
 * Text drawing functions
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

#include "egemb_font.h"
#define FONT_CELL_WIDTH (7)
#define FONT_CELL_HEIGHT (12)

static EG_IMAGE *FontImage = NULL;

//
// Text rendering
//

VOID egMeasureText(IN CHAR16 *Text, OUT UINTN *Width, OUT UINTN *Height)
{
    if (Width != NULL)
        *Width = StrLen(Text) * FONT_CELL_WIDTH;
    if (Height != NULL)
        *Height = FONT_CELL_HEIGHT;
}

VOID egRenderText(IN CHAR16 *Text, IN OUT EG_IMAGE *CompImage, IN UINTN PosX, IN UINTN PosY)
{
    EG_PIXEL        *BufferPtr;
    EG_PIXEL        *FontPixelData;
    UINTN           BufferLineOffset, FontLineOffset;
    UINTN           TextLength;
    UINTN           i, c;
    
    // clip the text
    TextLength = StrLen(Text);
    if (TextLength * FONT_CELL_WIDTH + PosX > CompImage->Width)
        TextLength = (CompImage->Width - PosX) / FONT_CELL_WIDTH;
    
    // load the font
    if (FontImage == NULL)
        FontImage = egPrepareEmbeddedImage(&egemb_font, TRUE);
    
    // render it
    BufferPtr = CompImage->PixelData;
    BufferLineOffset = CompImage->Width;
    BufferPtr += PosX + PosY * BufferLineOffset;
    FontPixelData = FontImage->PixelData;
    FontLineOffset = FontImage->Width;
    for (i = 0; i < TextLength; i++) {
        c = Text[i];
        if (c < 32 || c >= 127)
            c = 95;
        else
            c -= 32;
        egRawCompose(BufferPtr, FontPixelData + c * FONT_CELL_WIDTH,
                     FONT_CELL_WIDTH, FONT_CELL_HEIGHT,
                     BufferLineOffset, FontLineOffset);
        BufferPtr += FONT_CELL_WIDTH;
    }
}

/* EOF */
