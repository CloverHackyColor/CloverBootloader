//
//  memory.cpp
//
//  Created by Jief on 30/10/2020.
//


extern "C" {

#include <Library/BaseMemoryLib.h>

/*
 * memset and memcpy has to be provided for clang
 */


#ifdef __GNUC__
  void* memset(void* dst, int ch, UINTN count) __attribute__ ((used));
  //void* memcpy(void* dst, const void* src, UINT64 count) __attribute__ ((used));
#else
//  void* memset(void* dst, int ch, UINT64 count);
  //void* memcpy(void* dst, const void* src, UINT64 count);
#endif

void* memset(void* dst, int ch, UINTN count)
{
  SetMem(dst, count, (UINT8)(ch));
  return dst;
}

void* memcpy(void* dst, const void* src, UINTN count)
{
  CopyMem(dst, src, count);
  return dst;
}



} // extern "C"

