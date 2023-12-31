/** @file

BaseOverflowLib

Copyright (c) 2018, vit9696

All rights reserved.

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __BASE_OVERFLOW_LIB__
#define __BASE_OVERFLOW_LIB__

//
// The macros below provide pointer alignment checking interfaces.
// TypedPtr - pointer of a dedicated type, which alignment is to be checked.
// Align    - valid alignment for the target platform (power of two so far).
// Type     - valid complete typename.
// Ptr      - raw pointer value, must fit into UINTN, meant to be uintptr_t equivalent.
//

#define BASE_ALIGNOF(Type)            (_Alignof (Type))
#define BASE_POT_ALIGNED(Align, Ptr)  (0ULL == (((UINTN) (Ptr)) & (Align-1U)))
#define BASE_TYPE_ALIGNED(Type, Ptr)  (BASE_POT_ALIGNED (BASE_ALIGNOF (Type), Ptr))

//
// Force member alignment for the structure.
//
#if (defined (__STDC__) && __STDC_VERSION__ >= 201112L) || defined (__GNUC__) || defined (__clang__)
#define BASE_ALIGNAS(Alignment)  _Alignas(Alignment)
#else
#define BASE_ALIGNAS(Alignment)
#endif

/**
 Return the result of (Multiplicand * Multiplier / Divisor).

 @param Multiplicand A 64-bit unsigned value.
 @param Multiplier   A 64-bit unsigned value.
 @param Divisor      A 32-bit unsigned value.
 @param Remainder    A pointer to a 32-bit unsigned value. This parameter is
 optional and may be NULL.

 @return Multiplicand * Multiplier / Divisor.
 **/
UINT64
BaseMultThenDivU64x64x32 (
  IN  UINT64  Multiplicand,
  IN  UINT64  Multiplier,
  IN  UINT32  Divisor,
  OUT UINT32  *Remainder  OPTIONAL
  );

//
// The interfaces below provide base safe arithmetics, reporting
// signed integer overflow and unsigned integer wraparound similarly to
// os/overflow.h in macOS SDK.
//
// Each interface may be implemented not only as an actual function, but
// a macro as well. Macro implementations are allowed to evaluate the
// expressions no more than once, and are supposed to provide faster
// compiler builtins if available.
//
// Each interface returns FALSE when the the stored result is equal to
// the infinite precision result, otherwise TRUE. The operands should
// be read left to right with the last argument representing a non-NULL
// pointer to the resulting value of the same type.
//
// More information could be found in Clang Extensions documentation:
// http://releases.llvm.org/7.0.0/tools/clang/docs/LanguageExtensions.html#checked-arithmetic-builtins
//

//
// 32-bit integer addition, subtraction, multiplication, triple addition (A+B+C),
// triple multiplication (A*B*C), addition with multiplication ((A+B)*C),
// and multiplication with addition (A*B+C) support.
//

BOOLEAN
BaseOverflowAddU16 (
  UINT16  A,
  UINT16  B,
  UINT16  *Result
  );

BOOLEAN
BaseOverflowSubU16 (
  UINT16  A,
  UINT16  B,
  UINT16  *Result
  );

BOOLEAN
BaseOverflowMulU16 (
  UINT16  A,
  UINT16  B,
  UINT16  *Result
  );

BOOLEAN
BaseOverflowAddU32 (
  UINT32  A,
  UINT32  B,
  UINT32  *Result
  );

BOOLEAN
BaseOverflowSubU32 (
  UINT32  A,
  UINT32  B,
  UINT32  *Result
  );

BOOLEAN
BaseOverflowMulU32 (
  UINT32  A,
  UINT32  B,
  UINT32  *Result
  );

BOOLEAN
BaseOverflowTriAddU32 (
  UINT32  A,
  UINT32  B,
  UINT32  C,
  UINT32  *Result
  );

BOOLEAN
BaseOverflowTriMulU32 (
  UINT32  A,
  UINT32  B,
  UINT32  C,
  UINT32  *Result
  );

BOOLEAN
BaseOverflowAddMulU32 (
  UINT32  A,
  UINT32  B,
  UINT32  C,
  UINT32  *Result
  );

BOOLEAN
BaseOverflowMulAddU32 (
  UINT32  A,
  UINT32  B,
  UINT32  C,
  UINT32  *Result
  );

BOOLEAN
BaseOverflowAlignUpU32 (
  UINT32  Value,
  UINT32  Alignment,
  UINT32  *Result
  );

BOOLEAN
BaseOverflowAddS32 (
  INT32  A,
  INT32  B,
  INT32  *Result
  );

BOOLEAN
BaseOverflowSubS32 (
  INT32  A,
  INT32  B,
  INT32  *Result
  );

BOOLEAN
BaseOverflowMulS32 (
  INT32  A,
  INT32  B,
  INT32  *Result
  );

BOOLEAN
BaseOverflowTriAddS32 (
  INT32  A,
  INT32  B,
  INT32  C,
  INT32  *Result
  );

BOOLEAN
BaseOverflowTriMulS32 (
  INT32  A,
  INT32  B,
  INT32  C,
  INT32  *Result
  );

BOOLEAN
BaseOverflowAddMulS32 (
  INT32  A,
  INT32  B,
  INT32  C,
  INT32  *Result
  );

BOOLEAN
BaseOverflowMulAddS32 (
  INT32  A,
  INT32  B,
  INT32  C,
  INT32  *Result
  );

//
// 64-bit integer addition, subtraction, multiplication, triple addition (A+B+C),
// triple multiplication (A*B*C), addition with multiplication ((A+B)*C),
// and multiplication with addition (A*B+C) support.
//

BOOLEAN
BaseOverflowAddU64 (
  UINT64  A,
  UINT64  B,
  UINT64  *Result
  );

BOOLEAN
BaseOverflowSubU64 (
  UINT64  A,
  UINT64  B,
  UINT64  *Result
  );

BOOLEAN
BaseOverflowMulU64 (
  UINT64  A,
  UINT64  B,
  UINT64  *Result
  );

BOOLEAN
BaseOverflowTriAddU64 (
  UINT64  A,
  UINT64  B,
  UINT64  C,
  UINT64  *Result
  );

BOOLEAN
BaseOverflowTriMulU64 (
  UINT64  A,
  UINT64  B,
  UINT64  C,
  UINT64  *Result
  );

BOOLEAN
BaseOverflowAddMulU64 (
  UINT64  A,
  UINT64  B,
  UINT64  C,
  UINT64  *Result
  );

BOOLEAN
BaseOverflowMulAddU64 (
  UINT64  A,
  UINT64  B,
  UINT64  C,
  UINT64  *Result
  );

BOOLEAN
BaseOverflowAddS64 (
  INT64  A,
  INT64  B,
  INT64  *Result
  );

BOOLEAN
BaseOverflowSubS64 (
  INT64  A,
  INT64  B,
  INT64  *Result
  );

BOOLEAN
BaseOverflowMulS64 (
  INT64  A,
  INT64  B,
  INT64  *Result
  );

BOOLEAN
BaseOverflowTriAddS64 (
  INT64  A,
  INT64  B,
  INT64  C,
  INT64  *Result
  );

BOOLEAN
BaseOverflowTriMulS64 (
  INT64  A,
  INT64  B,
  INT64  C,
  INT64  *Result
  );

BOOLEAN
BaseOverflowAddMulS64 (
  INT64  A,
  INT64  B,
  INT64  C,
  INT64  *Result
  );

BOOLEAN
BaseOverflowMulAddS64 (
  INT64  A,
  INT64  B,
  INT64  C,
  INT64  *Result
  );

//
// Native integer addition, subtraction, multiplication, triple addition (A+B+C),
// triple multiplication (A*B*C), addition with multiplication ((A+B)*C),
// and multiplication with addition (A*B+C) support.
//

BOOLEAN
BaseOverflowAddUN (
  UINTN  A,
  UINTN  B,
  UINTN  *Result
  );

BOOLEAN
BaseOverflowSubUN (
  UINTN  A,
  UINTN  B,
  UINTN  *Result
  );

BOOLEAN
BaseOverflowMulUN (
  UINTN  A,
  UINTN  B,
  UINTN  *Result
  );

BOOLEAN
BaseOverflowTriAddUN (
  UINTN  A,
  UINTN  B,
  UINTN  C,
  UINTN  *Result
  );

BOOLEAN
BaseOverflowTriMulUN (
  UINTN  A,
  UINTN  B,
  UINTN  C,
  UINTN  *Result
  );

BOOLEAN
BaseOverflowAddMulUN (
  UINTN  A,
  UINTN  B,
  UINTN  C,
  UINTN  *Result
  );

BOOLEAN
BaseOverflowMulAddUN (
  UINTN  A,
  UINTN  B,
  UINTN  C,
  UINTN  *Result
  );

BOOLEAN
BaseOverflowAddSN (
  INTN  A,
  INTN  B,
  INTN  *Result
  );

BOOLEAN
BaseOverflowSubSN (
  INTN  A,
  INTN  B,
  INTN  *Result
  );

BOOLEAN
BaseOverflowMulSN (
  INTN  A,
  INTN  B,
  INTN  *Result
  );

BOOLEAN
BaseOverflowTriAddSN (
  INTN  A,
  INTN  B,
  INTN  C,
  INTN  *Result
  );

BOOLEAN
BaseOverflowTriMulSN (
  INTN  A,
  INTN  B,
  INTN  C,
  INTN  *Result
  );

BOOLEAN
BaseOverflowAddMulSN (
  INTN  A,
  INTN  B,
  INTN  C,
  INTN  *Result
  );

BOOLEAN
BaseOverflowMulAddSN (
  INTN  A,
  INTN  B,
  INTN  C,
  INTN  *Result
  );

#endif // __BASE_OVERFLOW_LIB__
