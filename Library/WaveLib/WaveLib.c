/*
 * File: WaveLib.c
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

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/WaveLib.h>
#include <Library/MemoryAllocationLib.h>

EFI_STATUS
EFIAPI
WaveGetFileData(
    IN  CONST VOID *FileData,
    IN  UINTN FileLength,
    OUT WAVE_FILE_DATA *WaveFileData)
{

    // Create variables.
    UINT8 *FilePtr = NULL;
    RIFF_CHUNK *RiffChunk = NULL;
    RIFF_CHUNK *FormatChunk = NULL;
    RIFF_CHUNK *DataChunk = NULL;

    // Ensure parameters are valid.
    if ((FileData == NULL) || (FileLength == 0) || (WaveFileData == NULL))
        return EFI_INVALID_PARAMETER;
    FilePtr = (UINT8*)FileData;

    // Get RIFF chunk (should be first).
    RiffChunk = (RIFF_CHUNK*)FilePtr;

    // Ensure chunk ID is RIFF and the first 4 bytes of data are WAVE.
    if (AsciiStrnCmp(RiffChunk->Id, RIFF_CHUNK_ID, RIFF_CHUNK_ID_SIZE) ||
        AsciiStrnCmp((CHAR8*)RiffChunk->Data, WAVE_CHUNK_ID, RIFF_CHUNK_ID_SIZE) || (
        (RiffChunk->Size + sizeof(RiffChunk->Id) + sizeof(RiffChunk->Size)) != FileLength))
        return EFI_UNSUPPORTED;

    // Move to data area.
    FilePtr += sizeof(RIFF_CHUNK);

    // Get index of format chunk.
    for (UINTN i = FilePtr - (UINT8*)FileData; i < FileLength; i++) {
        if (AsciiStrnCmp((CHAR8*)(((UINT8*)FileData) + i), WAVE_FORMAT_CHUNK_ID, RIFF_CHUNK_ID_SIZE) == 0) {
            FilePtr = (UINT8*)FileData + i;
            FormatChunk = (RIFF_CHUNK*)FilePtr;
            break;
        }
    }
    if (FormatChunk == NULL)
        return EFI_UNSUPPORTED;

    // Get index of data chunk.
    for (UINTN i = FilePtr - (UINT8*)FileData; i < FileLength; i++) {
        if (AsciiStrnCmp((CHAR8*)(((UINT8*)FileData) + i), WAVE_DATA_CHUNK_ID, RIFF_CHUNK_ID_SIZE) == 0) {
            FilePtr = (UINT8*)FileData + i;
            DataChunk = (RIFF_CHUNK*)FilePtr;
            break;
        }
    }
    if (DataChunk == NULL)
        return EFI_UNSUPPORTED;

    // Copy to output structure.
    ZeroMem(WaveFileData, sizeof(WAVE_FILE_DATA));
    WaveFileData->FileLength = FileLength;
    WaveFileData->DataLength = RiffChunk->Size;
    WaveFileData->Format = (WAVE_FORMAT_DATA*)AllocateCopyPool(sizeof(WAVE_FORMAT_DATA), FormatChunk->Data);
    WaveFileData->FormatLength = FormatChunk->Size;
//    WaveFileData->Samples = AllocateCopyPool(DataChunk->Size, DataChunk->Data);
    WaveFileData->Samples = AllocateAlignedPages(EFI_SIZE_TO_PAGES(DataChunk->Size+4095), 128);
    CopyMem(WaveFileData->Samples, DataChunk->Data, DataChunk->Size);
    WaveFileData->SamplesLength = DataChunk->Size;
    return EFI_SUCCESS;
}
