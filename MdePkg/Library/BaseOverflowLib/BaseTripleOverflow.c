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

#include <Library/BaseOverflowLib.h>

BOOLEAN
BaseOverflowTriAddU32 (
  UINT32  A,
  UINT32  B,
  UINT32  C,
  UINT32  *Result
  )
{
  UINT32   BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowAddU32 (A, B, &BaseTmp);
  BaseSecond = BaseOverflowAddU32 (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowTriMulU32 (
  UINT32  A,
  UINT32  B,
  UINT32  C,
  UINT32  *Result
  )
{
  UINT32   BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowMulU32 (A, B, &BaseTmp);
  BaseSecond = BaseOverflowMulU32 (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowAddMulU32 (
  UINT32  A,
  UINT32  B,
  UINT32  C,
  UINT32  *Result
  )
{
  UINT32   BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowAddU32 (A, B, &BaseTmp);
  BaseSecond = BaseOverflowMulU32 (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowMulAddU32 (
  UINT32  A,
  UINT32  B,
  UINT32  C,
  UINT32  *Result
  )
{
  UINT32   BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowMulU32 (A, B, &BaseTmp);
  BaseSecond = BaseOverflowAddU32 (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowTriAddS32 (
  INT32  A,
  INT32  B,
  INT32  C,
  INT32  *Result
  )
{
  INT32    BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowAddS32 (A, B, &BaseTmp);
  BaseSecond = BaseOverflowAddS32 (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowTriMulS32 (
  INT32  A,
  INT32  B,
  INT32  C,
  INT32  *Result
  )
{
  INT32    BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowMulS32 (A, B, &BaseTmp);
  BaseSecond = BaseOverflowMulS32 (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowAddMulS32 (
  INT32  A,
  INT32  B,
  INT32  C,
  INT32  *Result
  )
{
  INT32    BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowAddS32 (A, B, &BaseTmp);
  BaseSecond = BaseOverflowMulS32 (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowMulAddS32 (
  INT32  A,
  INT32  B,
  INT32  C,
  INT32  *Result
  )
{
  INT32    BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowMulS32 (A, B, &BaseTmp);
  BaseSecond = BaseOverflowAddS32 (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowTriAddU64 (
  UINT64  A,
  UINT64  B,
  UINT64  C,
  UINT64  *Result
  )
{
  UINT64   BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowAddU64 (A, B, &BaseTmp);
  BaseSecond = BaseOverflowAddU64 (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowTriMulU64 (
  UINT64  A,
  UINT64  B,
  UINT64  C,
  UINT64  *Result
  )
{
  UINT64   BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowMulU64 (A, B, &BaseTmp);
  BaseSecond = BaseOverflowMulU64 (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowAddMulU64 (
  UINT64  A,
  UINT64  B,
  UINT64  C,
  UINT64  *Result
  )
{
  UINT64   BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowAddU64 (A, B, &BaseTmp);
  BaseSecond = BaseOverflowMulU64 (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowMulAddU64 (
  UINT64  A,
  UINT64  B,
  UINT64  C,
  UINT64  *Result
  )
{
  UINT64   BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowMulU64 (A, B, &BaseTmp);
  BaseSecond = BaseOverflowAddU64 (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowTriAddS64 (
  INT64  A,
  INT64  B,
  INT64  C,
  INT64  *Result
  )
{
  INT64    BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowAddS64 (A, B, &BaseTmp);
  BaseSecond = BaseOverflowAddS64 (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowTriMulS64 (
  INT64  A,
  INT64  B,
  INT64  C,
  INT64  *Result
  )
{
  INT64    BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowMulS64 (A, B, &BaseTmp);
  BaseSecond = BaseOverflowMulS64 (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowAddMulS64 (
  INT64  A,
  INT64  B,
  INT64  C,
  INT64  *Result
  )
{
  INT64    BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowAddS64 (A, B, &BaseTmp);
  BaseSecond = BaseOverflowMulS64 (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowMulAddS64 (
  INT64  A,
  INT64  B,
  INT64  C,
  INT64  *Result
  )
{
  INT64    BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowMulS64 (A, B, &BaseTmp);
  BaseSecond = BaseOverflowAddS64 (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowTriAddUN (
  UINTN  A,
  UINTN  B,
  UINTN  C,
  UINTN  *Result
  )
{
  UINTN    BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowAddUN (A, B, &BaseTmp);
  BaseSecond = BaseOverflowAddUN (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowTriMulUN (
  UINTN  A,
  UINTN  B,
  UINTN  C,
  UINTN  *Result
  )
{
  UINTN    BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowMulUN (A, B, &BaseTmp);
  BaseSecond = BaseOverflowMulUN (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowAddMulUN (
  UINTN  A,
  UINTN  B,
  UINTN  C,
  UINTN  *Result
  )
{
  UINTN    BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowAddUN (A, B, &BaseTmp);
  BaseSecond = BaseOverflowMulUN (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowMulAddUN (
  UINTN  A,
  UINTN  B,
  UINTN  C,
  UINTN  *Result
  )
{
  UINTN    BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowMulUN (A, B, &BaseTmp);
  BaseSecond = BaseOverflowAddUN (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowTriAddSN (
  INTN  A,
  INTN  B,
  INTN  C,
  INTN  *Result
  )
{
  INTN     BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowAddSN (A, B, &BaseTmp);
  BaseSecond = BaseOverflowAddSN (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowTriMulSN (
  INTN  A,
  INTN  B,
  INTN  C,
  INTN  *Result
  )
{
  INTN     BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowMulSN (A, B, &BaseTmp);
  BaseSecond = BaseOverflowMulSN (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowAddMulSN (
  INTN  A,
  INTN  B,
  INTN  C,
  INTN  *Result
  )
{
  INTN     BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowAddSN (A, B, &BaseTmp);
  BaseSecond = BaseOverflowMulSN (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}

BOOLEAN
BaseOverflowMulAddSN (
  INTN  A,
  INTN  B,
  INTN  C,
  INTN  *Result
  )
{
  INTN     BaseTmp;
  BOOLEAN  BaseFirst;
  BOOLEAN  BaseSecond;

  BaseFirst  = BaseOverflowMulSN (A, B, &BaseTmp);
  BaseSecond = BaseOverflowAddSN (BaseTmp, C, Result);
  return BaseFirst | BaseSecond;
}
