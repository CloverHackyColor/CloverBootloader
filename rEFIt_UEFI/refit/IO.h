/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  IO.h

Abstract:

  Information about the IO library function

Revision History

--*/

#ifndef _SHELL_I_O_H
#define _SHELL_I_O_H


#define EFI_TPL_APPLICATION 4
#define EFI_TPL_CALLBACK    8
#define EFI_TPL_NOTIFY      16
#define EFI_TPL_HIGH_LEVEL  31


typedef struct {
  CHAR16  *Str;
  UINTN   Len;
  UINTN   MaxLen;
} POOL_PRINT;

VOID
Input (
  IN CHAR16   *Prompt OPTIONAL,
  OUT CHAR16  *InStr,
  IN UINTN    StrLen
  );

VOID
Output (
  IN CHAR16   *Str
  );
/*
UINTN
Print (
  IN CHAR16     *fmt,
  ...
  ); */

UINTN
PrintAt (
  IN UINTN      Column,
  IN UINTN      Row,
  IN CHAR16     *fmt,
  ...
  );

UINTN
SPrint (
  OUT CHAR16    *Str,
  IN UINTN      StrSize,
  IN CHAR16     *fmt,
  ...
  );
/*
UINTN
PrintToken (
  IN UINT16             Token,
  IN EFI_HII_HANDLE     Handle,
  ...
  );

UINTN
IPrint (
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL     *Out,
  IN CHAR16                           *fmt,
  ...
  );
*/
UINTN
VSPrint (
  OUT CHAR16                        *Str,
  IN UINTN                          StrSize,
  IN CHAR16                         *fmt,
  IN VA_LIST                        vargs
  );

CHAR16                                *
PoolPrint (
  IN CHAR16                           *fmt,
  ...
  );

CHAR16                                *
CatPrint (
  IN OUT POOL_PRINT     *Str,
  IN CHAR16             *fmt,
  ...
  );
/*
INTN
DbgPrint (
  IN INTN       mask,
  IN CHAR8      *fmt,
  ...
  );
*/
VOID
ConMoveCursorBackward (
  IN     UINTN                   LineLength,
  IN OUT UINTN                   *Column,
  IN OUT UINTN                   *Row
  );

VOID
ConMoveCursorForward (
  IN     UINTN                   LineLength,
  IN     UINTN                   TotalRow,
  IN OUT UINTN                   *Column,
  IN OUT UINTN                   *Row
  );

VOID
IInput (
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL     * ConOut,
  IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL      * ConIn,
  IN CHAR16                           *Prompt OPTIONAL,
  OUT CHAR16                          *InStr,
  IN UINTN                            StrLength
  );

UINTN
IPrintAt (
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL     *Out,
  IN UINTN                            Column,
  IN UINTN                            Row,
  IN CHAR16                           *fmt,
  ...
  );

UINTN
APrint (
  IN CHAR8                            *fmt,
  ...
  );

VOID
LibEnablePageBreak (
  IN INT32      StartRow,
  IN BOOLEAN    AutoWrap
  );

BOOLEAN
LibGetPageBreak (
  VOID
  );

EFI_STATUS
WaitForSingleEvent (
					IN EFI_EVENT        Event,
					IN UINT64           Timeout OPTIONAL
					);

#endif
