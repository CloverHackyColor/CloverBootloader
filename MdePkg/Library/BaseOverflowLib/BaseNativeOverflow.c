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

BOOLEAN
BaseOverflowAddUN (
  UINTN  A,
  UINTN  B,
  UINTN  *Result
  )
{
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS)
  return __builtin_add_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS_64)
  return __builtin_uaddll_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS_32)
  return __builtin_uadd_overflow (A, B, Result);
 #else
  if (sizeof (UINTN) == sizeof (UINT64)) {
    return BaseOverflowAddU64 (A, B, (UINT64 *)Result);
  }

  return BaseOverflowAddU32 ((UINT32)A, (UINT32)B, (UINT32 *)Result);
 #endif
}

BOOLEAN
BaseOverflowSubUN (
  UINTN  A,
  UINTN  B,
  UINTN  *Result
  )
{
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS)
  return __builtin_sub_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS_64)
  return __builtin_usubll_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS_32)
  return __builtin_usub_overflow (A, B, Result);
 #else
  if (sizeof (UINTN) == sizeof (UINT64)) {
    return BaseOverflowSubU64 (A, B, (UINT64 *)Result);
  }

  return BaseOverflowSubU32 ((UINT32)A, (UINT32)B, (UINT32 *)Result);
 #endif
}

BOOLEAN
BaseOverflowMulUN (
  UINTN  A,
  UINTN  B,
  UINTN  *Result
  )
{
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS)
  return __builtin_mul_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS_64)
  return __builtin_umulll_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS_32)
  return __builtin_umul_overflow (A, B, Result);
 #else
  if (sizeof (UINTN) == sizeof (UINT64)) {
    return BaseOverflowMulU64 (A, B, (UINT64 *)Result);
  }

  return BaseOverflowMulU32 ((UINT32)A, (UINT32)B, (UINT32 *)Result);
 #endif
}

BOOLEAN
BaseOverflowAddSN (
  INTN  A,
  INTN  B,
  INTN  *Result
  )
{
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS)
  return __builtin_add_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS_64)
  return __builtin_saddll_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS_32)
  return __builtin_sadd_overflow (A, B, Result);
 #else
  if (sizeof (INTN) == sizeof (INT64)) {
    return BaseOverflowAddS64 (A, B, (INT64 *)Result);
  }

  return BaseOverflowAddS32 ((INT32)A, (INT32)B, (INT32 *)Result);
 #endif
}

BOOLEAN
BaseOverflowSubSN (
  INTN  A,
  INTN  B,
  INTN  *Result
  )
{
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS)
  return __builtin_sub_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS_64)
  return __builtin_ssubll_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS_32)
  return __builtin_ssub_overflow (A, B, Result);
 #else
  if (sizeof (INTN) == sizeof (INT64)) {
    return BaseOverflowSubS64 (A, B, (INT64 *)Result);
  }

  return BaseOverflowSubS32 ((INT32)A, (INT32)B, (INT32 *)Result);
 #endif
}

BOOLEAN
BaseOverflowMulSN (
  INTN  A,
  INTN  B,
  INTN  *Result
  )
{
 #if defined (BASE_HAS_TYPE_GENERIC_BUILTINS)
  return __builtin_mul_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS_64)
  return __builtin_smulll_overflow (A, B, Result);
 #elif defined (BASE_HAS_TYPE_SPECIFIC_BUILTINS_32)
  return __builtin_smul_overflow (A, B, Result);
 #else
  if (sizeof (INTN) == sizeof (INT64)) {
    return BaseOverflowMulS64 (A, B, (INT64 *)Result);
  }

  return BaseOverflowMulS32 ((INT32)A, (INT32)B, (INT32 *)Result);
 #endif
}
