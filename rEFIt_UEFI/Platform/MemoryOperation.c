/*
 * Copyright (c) 2011-2012 Frank Peng. All rights reserved.
 *
 */

#include "MemoryOperation.h"
#include "BootLog.h"

#include <Library/BaseMemoryLib.h>

#ifndef DEBUG_MEMORYOPERATION
# ifdef UNIT_TESTS_MACOS
#   define DEBUG_MEMORYOPERATION 0
# else
#   define DEBUG_MEMORYOPERATION 1
# endif
#else
#define DEBUG_MAIN DEBUG_ALL
#endif

#if DEBUG_MEMORYOPERATION == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_MEMORYOPERATION, __VA_ARGS__)
#endif


//
// Searches Source for Search pattern of size SearchSize
// and returns the number of occurences.
//
UINTN SearchAndCount(const UINT8 *Source, UINT64 SourceSize, const UINT8 *Search, UINTN SearchSize)
{
  UINTN        NumFounds = 0;
  const UINT8  *End = Source + SourceSize;
  
  while (Source < End) {
    if (CompareMem(Source, Search, SearchSize) == 0) {
      NumFounds++;
      Source += SearchSize;
    } else {
      Source++;
    }
  }
  return NumFounds;
}

//
// Searches Source for Search pattern of size SearchSize
// and replaces it with Replace up to MaxReplaces times.
// If MaxReplaces <= 0, then there is no restriction on number of replaces.
// Replace should have the same size as Search.
// Returns number of replaces done.
//
UINTN SearchAndReplace(UINT8 *Source, UINT64 SourceSize, const UINT8 *Search, UINTN SearchSize, const UINT8 *Replace, INTN MaxReplaces)
{
  UINTN     NumReplaces = 0;
  BOOLEAN   NoReplacesRestriction = MaxReplaces <= 0;
//  UINT8     *Begin = Source;
  UINT8     *End = Source + SourceSize;
  if (!Source || !Search || !Replace || !SearchSize) {
    return 0;
  }
  
  while ((Source < End) && (NoReplacesRestriction || (MaxReplaces > 0))) {
    if (CompareMem(Source, Search, SearchSize) == 0) {
 //     printf("  found pattern at %llx\n", (UINTN)(Source - Begin));

      DBG("Replace " );
      for (UINTN Index = 0; Index < SearchSize; ++Index) {
        DBG("%02X", Search[Index]);
      }
      DBG(" by " );


      CopyMem(Source, Replace, SearchSize);


      for (UINTN Index = 0; Index < SearchSize; ++Index) {
        DBG("%02X", Replace[Index]);
      }

      NumReplaces++;
      MaxReplaces--;
      Source += SearchSize;
    } else {
      Source++;
    }
  }
  return NumReplaces;
}

BOOLEAN CompareMemMask(const UINT8 *Source, const UINT8 *Search, UINTN SearchSize, const UINT8 *Mask, UINTN MaskSize)
{
  UINT8 M;
 
  if (!Mask || MaskSize == 0) {
    return !CompareMem(Source, Search, SearchSize);
  }
  for (UINTN Ind = 0; Ind < SearchSize; Ind++) {
    if (Ind < MaskSize)
      M = *Mask++;
    else M = 0xFF;
    if ((*Source++ & M) != (*Search++ & M)) {
      return FALSE;
    }
  }
  return TRUE;
}

void CopyMemMask(UINT8 *Dest, const UINT8 *Replace, const UINT8 *Mask, UINTN SearchSize)
{
  UINT8 M, D;
  // the procedure is called from SearchAndReplaceMask with own check but for future it is better to check twice
  if (!Dest || !Replace) { 
    return;
  }

  if (!Mask) {
    CopyMem(Dest, Replace, SearchSize); //old behavior
    return;
  }
  for (UINTN Ind = 0; Ind < SearchSize; Ind++) {
    M = *Mask++;
    D = *Dest;
    *Dest++ = ((D ^ *Replace++) & M) ^ D;
  }
}

UINTN FindMemMask(const UINT8 *Source, UINTN SourceSize, const UINT8 *Search, UINTN SearchSize, const UINT8 *MaskSearch, UINTN MaskSize)
{
  if (!Source || !Search || !SearchSize) {
    return MAX_UINTN;
  }

  for (UINTN i = 0; i < SourceSize - SearchSize; ++i) {
    if (CompareMemMask(&Source[i], Search, SearchSize, MaskSearch, MaskSize)) {
      return i;
    }
  }
  return MAX_UINTN;
}

UINTN SearchAndReplaceMask(UINT8 *Source, UINT64 SourceSize, const UINT8 *Search, const UINT8 *MaskSearch, UINTN SearchSize,
                           const UINT8 *Replace, const UINT8 *MaskReplace, INTN MaxReplaces, INTN Skip)
{
  UINTN     NumReplaces = 0;
  BOOLEAN   NoReplacesRestriction = MaxReplaces <= 0;
#if DEBUG_MEMORYOPERATION > 0
  UINT8*    SourceBak = Source;
#endif
  UINT8     *End = Source + SourceSize;
  if (!Source || !Search || !Replace || !SearchSize) {
    return 0;
  }
  while ((Source < End) && (NoReplacesRestriction || (MaxReplaces > 0))) {
    if (CompareMemMask((const UINT8 *)Source, Search, SearchSize, MaskSearch, SearchSize)) {
      if ( Skip == 0 ) {
        DBG("Replace " );
        for (UINTN Index = 0; Index < SearchSize; ++Index) {
          DBG("%02X", Search[Index]);
        }
        if ( MaskSearch ) {
          DBG("/" );
          for (UINTN Index = 0; Index < SearchSize; ++Index) {
            DBG("%02X", MaskSearch[Index]);
          }
          DBG("(" );
          for (UINTN Index = 0; Index < SearchSize; ++Index) {
            DBG("%02X", Source[Index]);
          }
          DBG(")" );
        }
        DBG(" by " );

        CopyMemMask(Source, Replace, MaskReplace, SearchSize);

        for (UINTN Index = 0; Index < SearchSize; ++Index) {
          DBG("%02X", Replace[Index]);
        }
        if ( MaskReplace ) {
          DBG("/");
          for (UINTN Index = 0; Index < SearchSize; ++Index) {
            DBG("%02X", MaskReplace[Index]);
          }
          DBG("(");
          for (UINTN Index = 0; Index < SearchSize; ++Index) {
            DBG("%02X", Source[Index]);
          }
          DBG(")");
        }

        DBG(" at ofs:%lX\n", Source-SourceBak);

        NumReplaces++;
        MaxReplaces--;
      }else{
        --Skip;
      }
      Source += SearchSize;
    } else {
      Source++;
    }

  }

  return NumReplaces;
}


UINTN SearchAndReplaceTxt(UINT8 *Source, UINT64 SourceSize, const UINT8 *Search, UINTN SearchSize, const UINT8 *Replace, INTN MaxReplaces)
{
  UINTN     NumReplaces = 0;
  UINTN     Skip = 0;
  BOOLEAN   NoReplacesRestriction = MaxReplaces <= 0;
  UINT8     *End = Source + SourceSize;
  const UINT8     *SearchEnd = Search + SearchSize;
  const UINT8     *Pos = NULL;
  UINT8     *SourcePos = NULL;
  UINT8     *FirstMatch = Source;
  if (!Source || !Search || !Replace || !SearchSize) {
    return 0;
  }
  
  while (((Source + SearchSize) <= End) &&
         (NoReplacesRestriction || (MaxReplaces > 0))) { // num replaces
    while (*Source != '\0') {  //comparison
      Pos = Search;
      FirstMatch = Source;
      Skip = 0;
      while (*Source != '\0' && Pos != SearchEnd) {
        if (*Source <= 0x20) { //skip invisibles in sources
          Source++;
          Skip++;
          continue;
        }
        if (*Source != *Pos) {
          break;
        }
 //       printf("%c", *Source);
        Source++;
        Pos++;
      }
      
      if (Pos == SearchEnd) { // pattern found
        SourcePos = FirstMatch;
        break;
      }
      else
        SourcePos = NULL;
      
      Source = FirstMatch + 1;
/*      if (Pos != Search) {
        printf("\n");
      } */
      
    }

    if (!SourcePos) {
      break;
    }
    CopyMem(SourcePos, Replace, SearchSize);
    SetMem(SourcePos + SearchSize, Skip, 0x20); //fill skip places with spaces
    NumReplaces++;
    MaxReplaces--;
    Source = FirstMatch + SearchSize + Skip;
  }
  return NumReplaces;
}
