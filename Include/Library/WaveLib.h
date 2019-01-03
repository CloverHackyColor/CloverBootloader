/*
 * File: WaveLib.h
 *
 * Description: WAVE file format functions.
 *
 * Copyright (c) 2018 John Davis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _EFI_WAVE_LIB_H_
#define _EFI_WAVE_LIB_H_

#include <IndustryStandard/Riff.h>

// WAVE file data.
typedef struct {
    UINTN FileLength;
    UINT32 DataLength;
    WAVE_FORMAT_DATA *Format;
    UINT32 FormatLength;

    UINT8 *Samples;
    UINT32 SamplesLength;
} WAVE_FILE_DATA;

EFI_STATUS
EFIAPI
WaveGetFileData(
    IN  CONST VOID *FileData,
    IN  UINTN FileLength,
    OUT WAVE_FILE_DATA *WaveFileData);

#endif
