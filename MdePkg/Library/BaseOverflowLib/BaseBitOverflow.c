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

#include <Base.h>

#include <Library/BaseLib.h>
#include <Library/BaseOverflowLib.h>

#include "BaseOverflowInternals.h"

//
// Software implementations provided try not to be obviously slow, but primarily
// target C99 compliance rather than performance.
//

BOOLEAN
BaseOverflowAddU16 (
  UINT16  A,
  UINT16  B,
  UINT16  *Result
  )
{
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS)
  return __builtin_add_overflow (A, B, Result);
 #else
  UINT32  Temp;

  //
  // I believe casting will be faster on X86 at least.
  //
  Temp    = A + B;
  *Result = (UINT16)Temp;
  if (Temp <= MAX_UINT16) {
    return FALSE;
  }

  return TRUE;
 #endif
}

BOOLEAN
BaseOverflowSubU16 (
  UINT16  A,
  UINT16  B,
  UINT16  *Result
  )
{
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS)
  return __builtin_sub_overflow (A, B, Result);
 #else
  *Result = (UINT16)(A - B);

  if (A >= B) {
    return FALSE;
  }

  return TRUE;
 #endif
}

BOOLEAN
BaseOverflowMulU16 (
  UINT16  A,
  UINT16  B,
  UINT16  *Result
  )
{
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS)
  return __builtin_mul_overflow (A, B, Result);
 #else
  UINT32  Temp;

  Temp    = (UINT32)A * B;
  *Result = (UINT16)Temp;
  if (Temp <= MAX_UINT32) {
    return FALSE;
  }

  return TRUE;
 #endif
}

BOOLEAN
BaseOverflowAddU32 (
  UINT32  A,
  UINT32  B,
  UINT32  *Result
  )
{
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS)
  return __builtin_add_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS_64)
  return __builtin_uadd_overflow (A, B, Result);
 #else
  UINT32  Temp;

  //
  // I believe casting will be faster on X86 at least.
  //
  Temp    = A + B;
  *Result = Temp;
  if (Temp >= A) {
    return FALSE;
  }

  return TRUE;
 #endif
}

BOOLEAN
BaseOverflowSubU32 (
  UINT32  A,
  UINT32  B,
  UINT32  *Result
  )
{
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS)
  return __builtin_sub_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS)
  return __builtin_usub_overflow (A, B, Result);
 #else
  *Result = A - B;
  if (B <= A) {
    return FALSE;
  }

  return TRUE;
 #endif
}

BOOLEAN
BaseOverflowMulU32 (
  UINT32  A,
  UINT32  B,
  UINT32  *Result
  )
{
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS)
  return __builtin_mul_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS)
  return __builtin_umul_overflow (A, B, Result);
 #else
  UINT64  Temp;

  Temp    = (UINT64)A * B;
  *Result = (UINT32)Temp;
  if (Temp <= MAX_UINT32) {
    return FALSE;
  }

  return TRUE;
 #endif
}

BOOLEAN
BaseOverflowAddS32 (
  INT32  A,
  INT32  B,
  INT32  *Result
  )
{
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS)
  return __builtin_add_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS)
  return __builtin_sadd_overflow (A, B, Result);
 #else
  INT64  Temp;

  Temp    = (INT64)A + B;
  *Result = (INT32)Temp;
  if ((Temp >= MIN_INT32) && (Temp <= MAX_INT32)) {
    return FALSE;
  }

  return TRUE;
 #endif
}

BOOLEAN
BaseOverflowSubS32 (
  INT32  A,
  INT32  B,
  INT32  *Result
  )
{
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS)
  return __builtin_sub_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS)
  return __builtin_ssub_overflow (A, B, Result);
 #else
  INT64  Temp;

  Temp    = (INT64)A - B;
  *Result = (INT32)Temp;
  if ((Temp >= MIN_INT32) && (Temp <= MAX_INT32)) {
    return FALSE;
  }

  return TRUE;
 #endif
}

BOOLEAN
BaseOverflowMulS32 (
  INT32  A,
  INT32  B,
  INT32  *Result
  )
{
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS)
  return __builtin_mul_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS)
  return __builtin_smul_overflow (A, B, Result);
 #else
  INT64  Temp;

  Temp    = MultS64x64 (A, B);
  *Result = (INT32)Temp;
  if ((Temp >= MIN_INT32) && (Temp <= MAX_INT32)) {
    return FALSE;
  }

  return TRUE;
 #endif
}

BOOLEAN
BaseOverflowAddU64 (
  UINT64  A,
  UINT64  B,
  UINT64  *Result
  )
{
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS)
  return __builtin_add_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS)
  return __builtin_uaddll_overflow (A, B, Result);
 #else
  UINT64  Temp;

  Temp    = A + B;
  *Result = Temp;
  if (Temp >= A) {
    return FALSE;
  }

  return TRUE;
 #endif
}

BOOLEAN
BaseOverflowSubU64 (
  UINT64  A,
  UINT64  B,
  UINT64  *Result
  )
{
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS)
  return __builtin_sub_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS)
  return __builtin_usubll_overflow (A, B, Result);
 #else
  *Result = A - B;
  if (B <= A) {
    return FALSE;
  }

  return TRUE;
 #endif
}

BOOLEAN
BaseOverflowMulU64 (
  UINT64  A,
  UINT64  B,
  UINT64  *Result
  )
{
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS)
  return __builtin_mul_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS)
  return __builtin_umulll_overflow (A, B, Result);
 #else
  UINT64   AHi;
  UINT64   ALo;
  UINT64   BHi;
  UINT64   BLo;
  UINT64   LoBits;
  UINT64   HiBits1;
  UINT64   HiBits2;
  BOOLEAN  Overflow;

  //
  // Based on the 2nd option written by Charphacy, believed to be the fastest portable on x86.
  // See: https://stackoverflow.com/a/26320664
  // Implements overflow checking by a series of up to 3 multiplications.
  //

  AHi = RShiftU64 (A, 32);
  ALo = A & MAX_UINT32;
  BHi = RShiftU64 (B, 32);
  BLo = B & MAX_UINT32;

  LoBits = MultU64x64 (ALo, BLo);
  if ((AHi == 0) && (BHi == 0)) {
    *Result = LoBits;
    return FALSE;
  }

  Overflow = AHi > 0 && BHi > 0;
  HiBits1  = MultU64x64 (ALo, BHi);
  HiBits2  = MultU64x64 (AHi, BLo);

  *Result = LoBits + LShiftU64 (HiBits1 + HiBits2, 32);
  return Overflow || *Result < LoBits || RShiftU64 (HiBits1, 32) != 0 || RShiftU64 (HiBits2, 32) != 0;
 #endif
}

BOOLEAN
BaseOverflowAddS64 (
  INT64  A,
  INT64  B,
  INT64  *Result
  )
{
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS)
  return __builtin_add_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS)
  return __builtin_saddll_overflow (A, B, Result);
 #else
  if (((B <= 0) || (A <= MAX_INT64 - B)) && ((B >= 0) || (A >= MIN_INT64 - B))) {
    *Result = A + B;
    return FALSE;
  }

  //
  // Assign some defined value to *Result.
  //
  *Result = 0;
  return TRUE;
 #endif
}

BOOLEAN
BaseOverflowSubS64 (
  INT64  A,
  INT64  B,
  INT64  *Result
  )
{
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS)
  return __builtin_sub_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS)
  return __builtin_ssubll_overflow (A, B, Result);
 #else
  if (((B >= 0) || (A <= MAX_INT64 + B)) && ((B <= 0) || (A >= MIN_INT64 + B))) {
    *Result = A - B;
    return FALSE;
  }

  //
  // Assign some defined value to *Result.
  //
  *Result = 0;
  return TRUE;
 #endif
}

BOOLEAN
BaseOverflowMulS64 (
  INT64  A,
  INT64  B,
  INT64  *Result
  )
{
  //
  // Intel 32-bit architectures do not have hardware signed 64-bit
  // multiplication with overflow.
  //
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS) && !defined (MDE_CPU_IA32)
  return __builtin_mul_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS) && !defined (MDE_CPU_IA32)
  return __builtin_smulll_overflow (A, B, Result);
 #else
  UINT64  AU;
  UINT64  BU;
  UINT64  ResultU;

  //
  // It hurts to implement it without unsigned multiplication, maybe rewrite it one day.
  // The idea taken from BaseSafeIntLib.
  //

  #define OC_ABS_64(X)  (((X) < 0) ? (((UINT64) (-((X) + 1))) + 1) : (UINT64) (X))

  AU = OC_ABS_64 (A);
  BU = OC_ABS_64 (B);

  if (BaseOverflowMulU64 (AU, BU, &ResultU)) {
    *Result = 0;
    return TRUE;
  }

  //
  // Split into positive and negative results and check just one range.
  //
  if ((A < 0) == (B < 0)) {
    if (ResultU <= MAX_INT64) {
      *Result = (INT64)ResultU;
      return FALSE;
    }
  } else {
    if (ResultU < OC_ABS_64 (MIN_INT64)) {
      *Result = -((INT64)ResultU);
      return FALSE;
    } else if (ResultU == OC_ABS_64 (MIN_INT64)) {
      *Result = MIN_INT64;
      return FALSE;
    }
  }

  #undef OC_ABS_64

  *Result = 0;
  return TRUE;
 #endif
}
