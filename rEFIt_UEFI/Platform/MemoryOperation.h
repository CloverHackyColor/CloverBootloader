/*
 * MemoryOperation.h
 *
 *  Created on: Oct 12, 2020
 *      Author: jief
 */


#ifndef MEMORYOPERATION_H_
#define MEMORYOPERATION_H_





#ifdef __cplusplus
extern "C" {
#endif


//#include <Library/BaseLib.h>
//#include <Library/BaseMemoryLib.h>
//#include <Library/DebugLib.h>


//
// Searches Source for Search pattern of size SearchSize
// and returns the number of occurences.
//
UINTN SearchAndCount(const UINT8 *Source, UINT64 SourceSize, const UINT8 *Search, UINTN SearchSize);

//
// Searches Source for Search pattern of size SearchSize
// and replaces it with Replace up to MaxReplaces times.
// If MaxReplaces <= 0, then there is no restriction on number of replaces.
// Replace should have the same size as Search.
// Returns number of replaces done.
//
UINTN SearchAndReplace(UINT8 *Source, UINT64 SourceSize, const UINT8 *Search, UINTN SearchSize, const UINT8 *Replace, INTN MaxReplaces);

BOOLEAN CompareMemMask(const UINT8 *Source, const UINT8 *Search, UINTN SearchSize, const UINT8 *Mask, UINTN MaskSize);

void CopyMemMask(UINT8 *Dest, const UINT8 *Replace, const UINT8 *Mask, UINTN SearchSize);

UINTN FindMemMask(const UINT8 *Source, UINTN SourceSize, const UINT8 *Search, UINTN SearchSize, const UINT8 *MaskSearch, UINTN MaskSize);

UINTN SearchAndReplaceMask(UINT8 *Source, UINT64 SourceSize, const UINT8 *Search, const UINT8 *MaskSearch, UINTN SearchSize,
                       const UINT8 *Replace, const UINT8 *MaskReplace, INTN MaxReplaces);


UINTN SearchAndReplaceTxt(UINT8 *Source, UINT64 SourceSize, const UINT8 *Search, UINTN SearchSize, const UINT8 *Replace, INTN MaxReplaces);


#ifdef __cplusplus
}
#endif



#endif /* MEMORYOPERATION_H_ */
