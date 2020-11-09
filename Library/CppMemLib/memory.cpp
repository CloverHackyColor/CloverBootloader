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


// Jief : I think __attribute__ ((used)) was needed for older version of GCC, can't remember.
//        If you want to put it back, do it only for the compiler version that really needs it.
//        Nov 2020 : With Gcc 10.2, defining it for memset is ok, but not needed. Defining it for memcpy generates an error in GenFW.
//#ifdef __GNUC__
//  void* memset(void* dst, int ch, UINTN count) __attribute__ ((used));
//  void* memcpy(void* dst, const void* src, UINTN count) __attribute__ ((used));
//#endif

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

