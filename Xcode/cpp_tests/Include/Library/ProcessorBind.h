//
//  ProcessorBind.h
//  cpp_tests
//
//  Created by Jief on 12/10/2020.
//  Copyright © 2020 JF Knudsen. All rights reserved.
//

#ifndef ProcessorBind_h
#define ProcessorBind_h


#include <limits.h>
//#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
//#include <string.h>
//#include <inttypes.h>
//#include <wchar.h>
#include <stdbool.h>
//
//#define MAX_UINTN ULONG_MAX
#define BOOLEAN bool


#define CHAR8  char
//#define CHAR16 char16_t
#define CHAR16 wchar_t
//
#define UINT8  uint8_t
#define UINT16 uint16_t
#define UINT32 uint32_t
#define UINT64 uint64_t
#define INT8  int8_t
#define INT16 int16_t
#define INT32 int32_t
#define INT64 int64_t

#define MAX_INT8    ((INT8)0x7F)
#define MAX_UINT8   ((UINT8)0xFF)
#define MAX_INT16   ((INT16)0x7FFF)
#define MAX_UINT16  ((UINT16)0xFFFF)
#define MAX_INT32   ((INT32)0x7FFFFFFF)
#define MAX_UINT32  ((UINT32)0xFFFFFFFF)
#define MAX_INT64   ((INT64)0x7FFFFFFFFFFFFFFFULL)
//#define MAX_UINT64  ((UINT64)0xFFFFFFFFFFFFFFFFULL)
#define MAX_UINT64  0xFFFFFFFFFFFFFFFFULL


#define UINTN uint64_t
#define INTN int64_t

#define MAX_UINTN MAX_UINT64
#define MAX_INTN MAX_UINT64


#endif /* ProcessorBind_h */
