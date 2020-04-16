/*++

Copyright (c) 2005 - 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution. The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  IO.c

Abstract:

  the IO library function

Revision History

--*/

//#include "EfiShellLib.h"
#include "Platform.h"
#include "screen.h"

#define PRINT_STRING_LEN        1024
#define PRINT_ITEM_BUFFER_LEN   100
#define PRINT_JOINT_BUFFER_LEN  4

extern "C" {
#   include <Library/PrintLib.h>
}

typedef struct {
  BOOLEAN PageBreak;
  BOOLEAN AutoWrap;
  INTN    MaxRow;
  INTN    MaxColumn;
  INTN    InitRow;
  INTN    Row;
  INTN    Column;
  BOOLEAN OmitPrint;
  BOOLEAN OutputPause;
} PRINT_MODE;

PRINT_MODE mPrintMode;

//
// Internal fucntions
//
UINTN
_PPrint (
  IN PRINT_STATE     *ps
  );

EFI_STATUS
EFIAPI
_SPrint (
  IN POOL_PRINT  *Context,
  IN CHAR16      *Buffer
  );

UINTN
EFIAPI
_IPrint (
  IN UINTN                            Column,
  IN UINTN                            Row,
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *Out,
  IN CONST CHAR16                           *fmt,
  IN CHAR8                            *fmta,
  IN VA_LIST                          args
  );

VOID
EFIAPI
_PoolCatPrint (
  IN CONST CHAR16         *fmt,
  IN VA_LIST              args,
  IN OUT POOL_PRINT       *spc,
  IN EFI_STATUS
    (EFIAPI
  *Output)
    (
      POOL_PRINT *context,
      CHAR16 *str
    )
  );

EFI_STATUS
EFIAPI
_PoolPrint (
  IN POOL_PRINT     *Context,
  IN CHAR16         *Buffer
  );

VOID
PFLUSH (
  IN OUT PRINT_STATE     *ps
  );

VOID
IFlushWithPageBreak (
  IN OUT PRINT_STATE     *ps
  );

VOID
PPUTC (
  IN OUT PRINT_STATE     *ps,
  IN CHAR16              c
  );

CHAR16
PGETC (
  IN POINTER      *p
  );

VOID
PITEM (
  IN OUT PRINT_STATE  *ps
  );

VOID
PSETATTR (
  IN OUT PRINT_STATE    *ps,
  IN UINTN              Attr
  );

VOID
SetOutputPause (
  IN BOOLEAN    Pause
  );

VOID
SetCursorPosition (
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL     *ConOut,
  IN  UINTN                           Column,
  IN  INTN                            Row,
  IN  UINTN                           LineLength,
  IN  UINTN                           TotalRow,
  IN  CHAR16                          *Str,
  IN  UINTN                           StrPos,
  IN  UINTN                           Len
  );

EFI_STATUS
EFIAPI
_DbgOut (
  IN VOID     *Context,
  IN CHAR16   *Buffer
  );

EFI_STATUS
EFIAPI
_SPrint (
  IN POOL_PRINT     *Context,
  IN CHAR16   *Buffer
  )
/*++
Routine Description:

  Print function

Arguments:

  Context  - The Context
  Buffer   - The Buffer

Returns:

--*/
{
  INTN        len;
  POOL_PRINT  *spc;

//  ASSERT (Context != NULL);
//  ASSERT (Buffer != NULL);
  if (!Context || !Buffer) {
    return EFI_SUCCESS;
  }

  spc = Context;
  len = StrLen (Buffer);

  //
  // Is the string is over the max truncate it
  //
  if (spc->Len + len > spc->Maxlen) {
    len = spc->Maxlen - spc->Len;
  }
  //
  // Append the new text
  //
  CopyMem (spc->Str + spc->Len, Buffer, len * sizeof (CHAR16));
  spc->Len += len;

  //
  // Null terminate it
  //
  if (spc->Len < spc->Maxlen) {
    spc->Str[spc->Len] = 0;
  } else if (spc->Maxlen) {
    spc->Str[spc->Maxlen] = 0;
  }

  return EFI_SUCCESS;
}

VOID
EFIAPI
_PoolCatPrint (
  IN CONST CHAR16         *fmt,
  IN VA_LIST              args,
  IN OUT POOL_PRINT       *spc,
  IN EFI_STATUS
    (EFIAPI
  *Output)
    (
      POOL_PRINT *context,
      CHAR16 *str
    )
  )
/*++'

Routine Description:

    Pool print

Arguments:
    fmt    - fmt
    args   - args
    spc    - spc
    Output - Output

Returns:

--*/
{
  PRINT_STATE ps;

  SetMem (&ps, sizeof (ps), 0);
  ps.Output   = (IN EFI_STATUS (EFIAPI *)(VOID *context, CONST CHAR16 *str)) Output; // C++ doesn't cast automatically to void* for param 1
  ps.Context  = spc;
  ps.fmt.u.pw = fmt;
  //ps.args     = args;
  VA_COPY(ps.args, args);
  _PPrint (&ps);
  VA_END(ps.args);
}

CHAR16 *
EFIAPI
PoolPrint (
  IN CONST CHAR16             *fmt,
  ...
  )
/*++

Routine Description:

    Prints a formatted unicode string to allocated pool.  The caller
    must free the resulting buffer.

Arguments:

    fmt         - The format string

Returns:

    Allocated buffer with the formatted string printed in it.
    The caller must free the allocated buffer.   The buffer
    allocation is not packed.

--*/
{
  POOL_PRINT  spc;
  VA_LIST     args;

  ZeroMem (&spc, sizeof (spc));
  VA_START (args, fmt);
  _PoolCatPrint (fmt, args, &spc, _PoolPrint);
  VA_END(args);
  return spc.Str;
}

UINTN
EFIAPI
PrintAt (
  IN UINTN      Column,
  IN UINTN      Row,
  IN CONST CHAR16     *fmt,
  ...
  )
/*++

Routine Description:

  Prints a formatted unicode string to the default console, at
  the supplied cursor position

Arguments:

  fmt         - Format string
  Column, Row - The cursor position to print the string at

Returns:

  Length of string printed to the console

--*/
{
  VA_LIST args;
  UINTN ret;

  VA_START (args, fmt);
  ret = _IPrint (Column, Row, gST->ConOut, fmt, NULL, args);
  VA_END(args);
  return ret;
}

UINTN
EFIAPI
_IPrint (
  IN UINTN                            Column,
  IN UINTN                            Row,
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL     *Out,
  IN CONST CHAR16                           *fmt,
  IN CHAR8                            *fmta,
  IN VA_LIST                          args
  )
/*++
Routine Description:

  Display string worker for: Print, PrintAt, IPrint, IPrintAt

Arguments:

    Column - Column
    Row    - Row
    Out    - Out
    fmt    - fmt
    fmta   - fmta
    args   - args

Returns:


--*/
{
  PRINT_STATE ps;
  UINTN       back;

//  ASSERT (NULL != Out);
  if (!Out) {
    return 0;
  }

  SetMem (&ps, sizeof (ps), 0);
  ps.Context  = Out;
  ps.Output   = (EFI_STATUS (EFIAPI *) (VOID *, CONST CHAR16 *)) Out->OutputString;  // C++ doesn't cast automatically to void* for param 1
  ps.SetAttr  = (EFI_STATUS (EFIAPI *) (VOID *, UINTN)) Out->SetAttribute;
//  ASSERT (NULL != Out->Mode);
  if (!Out->Mode) {
    return 0;
  }
  ps.Attr           = Out->Mode->Attribute;

  back              = (ps.Attr >> 4) & 0xF;
  ps.AttrNorm       = EFI_TEXT_ATTR (((~back) & 0x07), back);
  ps.AttrHighlight  = EFI_TEXT_ATTR (EFI_WHITE, back);
  ps.AttrError      = EFI_TEXT_ATTR (EFI_YELLOW, back);
  ps.AttrBlueColor  = EFI_TEXT_ATTR (EFI_LIGHTBLUE, back);
  ps.AttrGreenColor = EFI_TEXT_ATTR (EFI_LIGHTGREEN, back);

  if (fmt) {
    ps.fmt.u.pw = fmt;
  } else {
    ps.fmt.Ascii  = TRUE;
    ps.fmt.u.pc   = fmta;
  }

  //ps.args = args;
  VA_COPY(ps.args, args);

  if (Column != (UINTN) -1) {
    Out->SetCursorPosition (Out, Column, Row);
  }

  back = _PPrint (&ps);
  VA_END(ps.args);
  return back;
}

UINTN
EFIAPI
IPrint (
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL     *Out,
  IN CHAR16                           *fmt,
  ...
  )
/*++

Routine Description:

    Prints a formatted unicode string to the specified console

Arguments:

    Out         - The console to print the string too

    fmt         - Format string

Returns:

    Length of string printed to the console

--*/
{
  VA_LIST args;
  UINTN ret;

  VA_START (args, fmt);
  ret = _IPrint ((UINTN) -1, (UINTN) -1, Out, fmt, NULL, args);
  VA_END(args);
  return ret;
}

UINTN
EFIAPI
IPrintAt (
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL       *Out,
  IN UINTN                              Column,
  IN UINTN                              Row,
  IN CHAR16                             *fmt,
  ...
  )
/*++

Routine Description:

    Prints a formatted unicode string to the specified console, at
    the supplied cursor position

Arguments:

    Out         - The console to print the string too

    Column, Row - The cursor position to print the string at

    fmt         - Format string

Returns:

    Length of string printed to the console

--*/
{
  VA_LIST args;
  UINTN ret;

  VA_START (args, fmt);
  ret = _IPrint (Column, Row, gST->ConOut, fmt, NULL, args);
  VA_END(args);
  return ret;
}

VOID
PFLUSH (
  IN OUT PRINT_STATE     *ps
  )
{
  EFI_INPUT_KEY Key;
  EFI_STATUS    Status;

  *ps->Pos = 0;
  if (((UINTN) ps->Context == (UINTN) gST->ConOut) && mPrintMode.PageBreak) {

    IFlushWithPageBreak (ps);

  } else {

    if (mPrintMode.OutputPause) {

      Status = EFI_NOT_READY;
      while (EFI_ERROR (Status)) {
        Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
      }

      SetOutputPause (FALSE);
    }

    ps->Output (ps->Context, ps->Buffer);
  }

  CopyMem (
    ((CHAR8 *) (ps->Buffer)) - PRINT_JOINT_BUFFER_LEN,
    ((CHAR8 *) (ps->Pos)) - PRINT_JOINT_BUFFER_LEN,
    PRINT_JOINT_BUFFER_LEN
    );
  ps->Pos = ps->Buffer;
}

void
PSETATTR (
  IN OUT PRINT_STATE    *ps,
  IN UINTN              Attr
  )
{
//  ASSERT (ps != NULL);
  if (!ps) {
    return;
  }
  PFLUSH (ps);

  ps->RestoreAttr = ps->Attr;
  if (ps->SetAttr) {
    ps->SetAttr (ps->Context, Attr);
  }

  ps->Attr = Attr;
}

void
PPUTC (
  IN OUT PRINT_STATE     *ps,
  IN CHAR16              c
  )
{
 // ASSERT (ps != NULL);
  if (!ps) {
    return;
  }
  //
  // If Omit print to ConOut, then return.
  //
  if (mPrintMode.OmitPrint && ((UINTN) ps->Context == (UINTN) gST->ConOut)) {
    return ;
  }
  //
  // if this is a newline and carriage return does not exist,
  // add a carriage return
  //
  if (c == '\n' && ps->Pos >= ps->Buffer && (CHAR16) *(ps->Pos - 1) != '\r') {
    PPUTC (ps, '\r');
  }

  *ps->Pos = c;
  ps->Pos += 1;
  ps->Len += 1;

  //
  // if at the end of the buffer, flush it
  //
  if (ps->Pos >= ps->End) {
    PFLUSH (ps);
  }
}

CHAR16
PGETC (
  IN POINTER      *p
  )
{
  CHAR16  c;

  ASSERT (p != NULL);
  if (!p) {
    return 0;
  }

  c = (CHAR16) (p->Ascii ? p->u.pc[p->Index] : p->u.pw[p->Index]);
  p->Index += 1;

  return c;
}

void
PITEM (
  IN OUT PRINT_STATE  *ps
  )
{
  UINTN       Len;

  UINTN       i;
  PRINT_ITEM  *Item;
  CHAR16      c;

//  ASSERT (ps != NULL);
  if (!ps) {
    return;
  }

  //
  // Get the length of the item
  //
  Item              = ps->Item;
  Item->Item.Index  = 0;
  while (Item->Item.Index < Item->FieldWidth) {
    c = PGETC (&Item->Item);
    if (!c) {
      Item->Item.Index -= 1;
      break;
    }
  }

  Len = Item->Item.Index;

  //
  // if there is no item field width, use the items width
  //
  if (Item->FieldWidth == (UINTN) -1) {
    Item->FieldWidth = Len;
  }
  //
  // if item is larger then width, update width
  //
  if (Len > Item->Width) {
    Item->Width = Len;
  }
  //
  // if pad field before, add pad char
  //
  if (Item->PadBefore) {
    for (i = Item->Width; i < Item->FieldWidth; i += 1) {
      PPUTC (ps, ' ');
    }
  }
  //
  // pad item
  //
  for (i = Len; i < Item->Width; i++) {
    PPUTC (ps, Item->Pad);
  }
  //
  // add the item
  //
  Item->Item.Index = 0;
  while (Item->Item.Index < Len) {
    PPUTC (ps, PGETC (&Item->Item));
  }
  //
  // If pad at the end, add pad char
  //
  if (!Item->PadBefore) {
    for (i = Item->Width; i < Item->FieldWidth; i += 1) {
      PPUTC (ps, ' ');
    }
  }
}

UINTN
_PPrint (
  IN PRINT_STATE     *ps
  )

/*++

Routine Description:

  %w.lF   -   w = width
                l = field width
                F = format of arg

  Args F:
    0       -   pad with zeros
    -       -   justify on left (default is on right)
    ,       -   add comma's to field
    *       -   width provided on stack
    n       -   Set output attribute to normal (for this field only)
    h       -   Set output attribute to highlight (for this field only)
    e       -   Set output attribute to error (for this field only)
    b       -   Set output attribute to blue color (for this field only)
    v       -   Set output attribute to green color (for this field only)
    l       -   Value is 64 bits

    a       -   ascii string
    s       -   unicode string
    X       -   fixed 8 byte value in hex
    x       -   hex value
    d       -   value as decimal
    c       -   Unicode char
    t       -   EFI time structure
    g       -   Pointer to GUID
    r       -   EFI status code (result code)

    N       -   Set output attribute to normal
    H       -   Set output attribute to highlight
    E       -   Set output attribute to error
    B       -   Set output attribute to blue color
    V       -   Set output attribute to green color
    %       -   Print a %

Arguments:

    ps - Ps

Returns:

  Number of charactors written

--*/

{
  CHAR16      c;
  UINTN       Attr;
  PRINT_ITEM  Item;
  CHAR16      *Buffer;
//  EFI_GUID    *TmpGUID;

//  ASSERT (ps != NULL);
  if (!ps) {
    return 0;
  }
  //
  // If Omit print to ConOut, then return 0.
  //
  if (mPrintMode.OmitPrint && ((UINTN) ps->Context == (UINTN) gST->ConOut)) {
    return 0;
  }

  Item.Scratch = (__typeof__(Item.Scratch))AllocateZeroPool (sizeof (CHAR16) * PRINT_ITEM_BUFFER_LEN);
  if (NULL == Item.Scratch) {
    return EFI_OUT_OF_RESOURCES;
  }

  Buffer = (__typeof__(Buffer))AllocateZeroPool (sizeof (CHAR16) * PRINT_STRING_LEN);
  if (NULL == Buffer) {
    FreePool (Item.Scratch);
    return EFI_OUT_OF_RESOURCES;
  }

  ps->Len       = 0;
  ps->Buffer    = (CHAR16 *) ((CHAR8 *) Buffer + PRINT_JOINT_BUFFER_LEN);
  ps->Pos       = ps->Buffer;
  ps->End       = Buffer + PRINT_STRING_LEN - 1;
  ps->Item      = &Item;

  ps->fmt.Index = 0;
  c             = PGETC (&ps->fmt);
  while (c) {

    if (c != '%') {
      PPUTC (ps, c);
      c = PGETC (&ps->fmt);
      continue;
    }
    //
    // setup for new item
    //
    Item.FieldWidth = (UINTN) -1;
    Item.Width      = 0;
    Item.WidthParse = &Item.Width;
    Item.Pad        = ' ';
    Item.PadBefore  = TRUE;
    Item.Comma      = FALSE;
    Item.Long       = FALSE;
    Item.Item.Ascii = FALSE;
    Item.Item.u.pw  = NULL;
    ps->RestoreAttr = 0;
    Attr            = 0;

    c               = PGETC (&ps->fmt);
    while (c) {

      switch (c) {

      case '%':
        //
        // %% -> %
        //
        Item.Scratch[0] = '%';
        Item.Scratch[1] = 0;
        Item.Item.u.pw    = Item.Scratch;
        break;

      case '0':
        Item.Pad = '0';
        break;

      case '-':
        Item.PadBefore = FALSE;
        break;

      case ',':
        Item.Comma = TRUE;
        break;

      case '.':
        Item.WidthParse = &Item.FieldWidth;
        break;

      case '*':
        *Item.WidthParse = VA_ARG (ps->args, UINTN);
        break;

      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        *Item.WidthParse = 0;
        do {
          *Item.WidthParse  = *Item.WidthParse * 10 + c - '0';
          c                 = PGETC (&ps->fmt);
        } while (c >= '0' && c <= '9');
        ps->fmt.Index -= 1;
        break;

      case 'a':
        Item.Item.u.pc  = VA_ARG (ps->args, CHAR8 *);
        Item.Item.Ascii = TRUE;
        if (!Item.Item.u.pc) {
          Item.Item.u.pc = "(null)";
        }

        Item.PadBefore = FALSE;
        break;

      case 's':
        Item.Item.u.pw = VA_ARG (ps->args, CHAR16 *);
        if (!Item.Item.u.pw) {
          Item.Item.u.pw = L"(null)";
        }

        Item.PadBefore = FALSE;
        break;

      case 'c':
        Item.Scratch[0] = (CHAR16) VA_ARG (ps->args, UINTN);
        Item.Scratch[1] = 0;
        Item.Item.u.pw    = Item.Scratch;
        break;

      case 'l':
        Item.Long = TRUE;
        break;

      case 'X':
        Item.Width  = Item.Long ? 16 : 8;
        Item.Pad    = '0';

      case 'x':
			  //SPrint(Buffer, 64, L"EFI Error №%r", (UINTN)Status);
     //   ValueToHex (
		UnicodeSPrint(
          Item.Scratch, 64, L"%x",
          Item.Long ? VA_ARG (ps->args, UINT64) : VA_ARG (ps->args, UINTN)
          );
        Item.Item.u.pw = Item.Scratch;

        break;
/*
      case 'g':
        TmpGUID = VA_ARG (ps->args, EFI_GUID *);
        if (TmpGUID != NULL) {
          Item.Item.u.pw = Item.Scratch;
          GuidToString (Item.Item.u.pw, TmpGUID);
        }
        break;
*/
      case 'd':
      //  ValueToString (
		UnicodeSPrint(
          Item.Scratch, 64, L"%d",
       //   Item.Comma,
          Item.Long ? VA_ARG (ps->args, UINT64) : VA_ARG (ps->args, INTN)
          );
        Item.Item.u.pw = Item.Scratch;
        break;
/*
      case 't':
        Item.Item.u.pw = Item.Scratch;
        TimeToString (Item.Item.u.pw, VA_ARG (ps->args, EFI_TIME *));
        break;
*/
      case 'r':
   //     StatusToString
		UnicodeSPrint(Item.Scratch, 64, L"%r", VA_ARG (ps->args, EFI_STATUS));
        Item.Item.u.pw = Item.Scratch;
        break;

      case 'n':
        PSETATTR (ps, ps->AttrNorm);
        break;

      case 'h':
        PSETATTR (ps, ps->AttrHighlight);
        break;

      case 'b':
        PSETATTR (ps, ps->AttrBlueColor);
        break;

      case 'v':
        PSETATTR (ps, ps->AttrGreenColor);
        break;

      case 'e':
        PSETATTR (ps, ps->AttrError);
        break;

      case 'N':
        Attr = ps->AttrNorm;
        break;

      case 'H':
        Attr = ps->AttrHighlight;
        break;

      case 'E':
        Attr = ps->AttrError;
        break;

      case 'B':
        Attr = ps->AttrBlueColor;
        break;

      case 'V':
        Attr = ps->AttrGreenColor;
        break;

      default:
        Item.Scratch[0] = '?';
        Item.Scratch[1] = 0;
        Item.Item.u.pw    = Item.Scratch;
        break;
      }
      //
      // if we have an Item
      //
      if (Item.Item.u.pw) {
        PITEM (ps);
        break;
      }
      //
      // if we have an Attr set
      //
      if (Attr) {
        PSETATTR (ps, Attr);
        ps->RestoreAttr = 0;
        break;
      }

      c = PGETC (&ps->fmt);
    }

    if (ps->RestoreAttr) {
      PSETATTR (ps, ps->RestoreAttr);
    }

    c = PGETC (&ps->fmt);
  }
  //
  // Flush buffer
  //
  PFLUSH (ps);

  FreePool (Item.Scratch);
  FreePool (Buffer);

  return ps->Len;
}


EFI_STATUS
WaitForSingleEvent (
					IN EFI_EVENT        Event,
					IN UINT64           Timeout OPTIONAL
					)
{
	EFI_STATUS					Status;
	UINTN						Index;

	EFI_EVENT					WaitList[3];
	EFI_EVENT					TimerEvent;

	if (Timeout != 0)
	{
		//
		// Create a timer event
		//
		Status = gBS->CreateEvent(EVT_TIMER, 0, NULL, NULL, &TimerEvent);
		if (!EFI_ERROR (Status))
		{
			//
			// Set the timer event
			//
			gBS->SetTimer(TimerEvent, TimerRelative, Timeout);

			//
			// Wait for the original event or the timer
			//
			WaitList[0] = Event;
			WaitList[1] = TimerEvent;

			Status = gBS->WaitForEvent(2, WaitList, &Index);
			gBS->CloseEvent (TimerEvent);
			if (!EFI_ERROR (Status) && Index == 1)
			{
				Status = EFI_TIMEOUT;
			}
		}
	}
	else
	{
		WaitList[0] = Event;
		Status = gBS->WaitForEvent (1, WaitList, &Index);
	}
	return Status;
}


//set Timeout in ms
EFI_STATUS
WaitFor2EventWithTsc (
                     IN  EFI_EVENT        Event1,
                     IN  EFI_EVENT        Event2,
                     IN  UINT64           Timeout OPTIONAL
                    )
{
	EFI_STATUS				Status;
	UINTN             Index;
	EFI_EVENT					WaitList[2];
  UINT64            t0, t1;
  //all arguments are UINT64, we can use native divide and multiply
  UINT64            Delta = DivU64x64Remainder(MultU64x64(Timeout, gCPUStructure.TSCFrequency), 1000, NULL);

  if (Timeout != 0)
  {
    t0 = AsmReadTsc();
    do {
      Status = gBS->CheckEvent(Event1);
      if (!EFI_ERROR(Status)) {
        break;
      }
      if (Event2) {
        Status = gBS->CheckEvent(Event2);
        if (!EFI_ERROR(Status)) {
          break;
        }
      }
      // Let's try to relax processor a bit
      CpuPause();
      Status = EFI_TIMEOUT;
      t1 = AsmReadTsc();
    } while ((t1 - t0) < Delta);
	}
	else
	{
    WaitList[0] = Event1;
    WaitList[1] = Event2;
    Status = gBS->WaitForEvent (2, WaitList, &Index);
	}
	return Status;
}

#define ONE_SECOND  10000000
#define ONE_MSECOND    10000

// TimeoutDefault for a wait in seconds
// return EFI_TIMEOUT if no inputs
//the function must be in menu class
//so UpdatePointer(); => mPointer.Update(&gItemID, &Screen->mAction);
EFI_STATUS WaitForInputEventPoll(REFIT_MENU_SCREEN* ScreenPtr, UINTN TimeoutDefault)
{
  REFIT_MENU_SCREEN& Screen = *ScreenPtr;
  EFI_STATUS Status = EFI_SUCCESS;
  UINTN TimeoutRemain = TimeoutDefault * 100;

  while (TimeoutRemain != 0) {
    Status = WaitFor2EventWithTsc (gST->ConIn->WaitForKey, NULL, 10);
    if (Status != EFI_TIMEOUT) {
      break;
    }
    Screen.UpdateFilm();
    if (gSettings.PlayAsync) {
      CheckSyncSound();
    }
    TimeoutRemain--;
    if (Screen.mPointer.isAlive()) {
      Screen.mPointer.UpdatePointer();
      Status = Screen.CheckMouseEvent(); //out: gItemID, gAction
      if (Status != EFI_TIMEOUT) { //this check should return timeout if no mouse events occured
        break;
      }
    }
  }
  return Status;
}


BOOLEAN
SetPageBreak (
  IN OUT PRINT_STATE     *ps
  )
{
  EFI_INPUT_KEY Key;
  CHAR16        Str[3];

//  ASSERT (ps != NULL);
  if (!ps) {
    return FALSE;
  }

  ps->Output (ps->Context, L"Press ENTER to continue, 'q' to exit:");

  //
  // Wait for user input
  //
  Str[0]  = ' ';
  Str[1]  = 0;
  Str[2]  = 0;
  for (;;) {
    WaitForSingleEvent (gST->ConIn->WaitForKey, 0);
    gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);

    //
    // handle control keys
    //
    if (Key.UnicodeChar == CHAR_NULL) {
      if (Key.ScanCode == SCAN_ESC) {
        ps->Output (ps->Context, L"\r\n");
        mPrintMode.OmitPrint = TRUE;
        break;
      }

      continue;
    }

    if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      ps->Output (ps->Context, L"\r\n");
      mPrintMode.Row = mPrintMode.InitRow;
      break;
    }
    //
    // Echo input
    //
    Str[1] = Key.UnicodeChar;
    if (Str[1] == CHAR_BACKSPACE) {
      continue;
    }

    ps->Output (ps->Context, Str);

    if ((Str[1] == L'q') || (Str[1] == L'Q')) {
      mPrintMode.OmitPrint = TRUE;
    } else {
      mPrintMode.OmitPrint = FALSE;
    }

    Str[0] = CHAR_BACKSPACE;
  }

  return mPrintMode.OmitPrint;
}

void
IFlushWithPageBreak (
  IN OUT PRINT_STATE     *ps
  )
{
  CHAR16  *Pos;
  CHAR16  *LineStart;
  CHAR16  LineEndChar;

//  ASSERT (ps != NULL);
  if (!ps) {
    return;
  }

  Pos       = ps->Buffer;
  LineStart = Pos;
  while ((*Pos != 0) && (Pos < ps->Pos)) {
    if ((*Pos == L'\n') && (*(Pos - 1) == L'\r')) {
      //
      // Output one line
      //
      LineEndChar = *(Pos + 1);
      *(Pos + 1)  = 0;
      ps->Output (ps->Context, LineStart);
      *(Pos + 1) = LineEndChar;
      //
      // restore line end char
      //
      LineStart         = Pos + 1;
      mPrintMode.Column = 0;
      mPrintMode.Row++;
      if (mPrintMode.Row == mPrintMode.MaxRow) {
        if (SetPageBreak (ps)) {
          return ;
        }
      }
    } else {
      if (*Pos == CHAR_BACKSPACE) {
        mPrintMode.Column--;
      } else {
        mPrintMode.Column++;
      }
      //
      // If column is at the end of line, output a new line feed.
      //
      if ((mPrintMode.Column == mPrintMode.MaxColumn) && (*Pos != L'\n') && (*Pos != L'\r')) {

        LineEndChar = *(Pos + 1);
        *(Pos + 1)  = 0;
        ps->Output (ps->Context, LineStart);
        *(Pos + 1) = LineEndChar;
        //
        // restore line end char
        //
        if (mPrintMode.AutoWrap) {
          ps->Output (ps->Context, L"\r\n");
        }

        LineStart         = Pos + 1;
        mPrintMode.Column = 0;
        mPrintMode.Row++;
        if (mPrintMode.Row == mPrintMode.MaxRow) {
          if (SetPageBreak (ps)) {
            return ;
          }
        }
      }
    }

    Pos++;
  }

  if (*LineStart != 0) {
    ps->Output (ps->Context, LineStart);
  }
}

VOID
SetOutputPause (
  IN BOOLEAN    Pause
  )
{
  EFI_TPL Tpl;

  Tpl                     = gBS->RaiseTPL (EFI_TPL_NOTIFY);
  mPrintMode.OutputPause  = Pause;
  gBS->RestoreTPL (Tpl);
}

EFI_STATUS
EFIAPI
_PoolPrint (
  IN POOL_PRINT     *Context,
  IN CHAR16         *Buffer
  )
/*++

Routine Description:

    Append string worker for PoolPrint and CatPrint

Arguments:
    Context - Context
    Buffer  - Buffer

Returns:

--*/
{
  UINTN       newlen;
  POOL_PRINT  *spc;

//  ASSERT (Context != NULL);
  if (!Context) {
    return EFI_SUCCESS;
  }

  spc     = Context;
  newlen  = spc->Len + StrLen (Buffer) + 1;

  //
  // Is the string is over the max, grow the buffer
  //
  if (newlen > spc->Maxlen) {
    //
    // Grow the pool buffer
    //
    newlen += PRINT_STRING_LEN;
    spc->Maxlen = newlen;
    spc->Str = (__typeof__(spc->Str))EfiReallocatePool (
                spc->Str,
                spc->Len * sizeof (CHAR16),
                spc->Maxlen * sizeof (CHAR16)
                );

    if (!spc->Str) {
      spc->Len    = 0;
      spc->Maxlen = 0;
    }
  }
  //
  // Append the new text
  //
  return _SPrint (Context, Buffer);
}

VOID
Output (
  IN CHAR16   *Str
  )
/*++

Routine Description:

    Write a string to the console at the current cursor location

Arguments:

    Str - string

Returns:


--*/
{
  gST->ConOut->OutputString (gST->ConOut, Str);
}

VOID
Input (
  IN CHAR16    *Prompt OPTIONAL,
  OUT CHAR16   *InStr,
  IN UINTN     StrLen
  )
/*++

Routine Description:

  Input a string at the current cursor location, for StrLen

Arguments:

    Prompt - Prompt
    InStr  - The input string
    StrLen - The string buffer length

Returns:


--*/
{
  IInput (
    gST->ConOut,
    gST->ConIn,
    Prompt,
    InStr,
    StrLen
    );
}

VOID
ConMoveCursorBackward (
  IN     UINTN                   LineLength,
  IN OUT UINTN                   *Column,
  IN OUT UINTN                   *Row
  )
/*++

Routine Description:
  Move the cursor position one character backward.

Arguments:
  LineLength       Length of a line. Get it by calling QueryMode
  Column           Current column of the cursor position
  Row              Current row of the cursor position

Returns:

--*/
{
//  ASSERT (Column != NULL);
//  ASSERT (Row != NULL);
  if (!Column || !Row) {
    return;
  }

  //
  // If current column is 0, move to the last column of the previous line,
  // otherwise, just decrement column.
  //
  if (*Column == 0) {
    (*Column) = LineLength - 1;
    //
    //   if (*Row > 0) {
    //
    (*Row)--;
    //
    // }
    //
  } else {
    (*Column)--;
  }
}

VOID
ConMoveCursorForward (
  IN     UINTN                   LineLength,
  IN     UINTN                   TotalRow,
  IN OUT UINTN                   *Column,
  IN OUT UINTN                   *Row
  )
/*++

Routine Description:
  Move the cursor position one character backward.

Arguments:
  LineLength       Length of a line. Get it by calling QueryMode
  TotalRow         Total row of a screen, get by calling QueryMode
  Column           Current column of the cursor position
  Row              Current row of the cursor position

Returns:

--*/
{
//  ASSERT (Column != NULL);
//  ASSERT (Row != NULL);
  if (!Column || !Row) {
    return;
  }
  //
  // If current column is at line end, move to the first column of the nest
  // line, otherwise, just increment column.
  //
  (*Column)++;
  if (*Column >= LineLength) {
    (*Column) = 0;
    if ((*Row) < TotalRow - 1) {
      (*Row)++;
    }
  }
}

VOID
IInput (
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL     * ConOut,
  IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL      * ConIn,
  IN CHAR16                           *Prompt OPTIONAL,
  OUT CHAR16                          *InStr,
  IN UINTN                            StrLength
  )
/*++

Routine Description:
  Input a string at the current cursor location, for StrLength

Arguments:
  ConOut           Console output protocol
  ConIn            Console input protocol
  Prompt           Prompt string
  InStr            Buffer to hold the input string
  StrLength        Length of the buffer

Returns:

--*/
{
  BOOLEAN       Done;
  UINTN         Column;
  INTN          Row;
  UINTN         StartColumn;
  INTN         Update;
  UINTN         Delete;
  UINTN         Len;
  UINTN         StrPos;
  UINTN         Index;
  UINTN         LineLength;
  UINTN         TotalRow;
  UINTN         SkipLength;
  UINTN         OutputLength;
  UINTN         TailRow;
  UINTN         TailColumn;
  EFI_INPUT_KEY Key;
  BOOLEAN       InsertMode;

//  ASSERT (ConOut != NULL);
//  ASSERT (ConIn != NULL);
//  ASSERT (InStr != NULL);
  if (!ConOut || !ConIn || !InStr) {
    return;
  }

  if (Prompt) {
    ConOut->OutputString (ConOut, Prompt);
  }
  //
  // Read a line from the console
  //
//  Len           = 0;
//  StrPos        = 0;
  OutputLength  = 0;
  Update        = 0;
  Delete        = 0;
  InsertMode    = TRUE;

  //
  // If buffer is not large enough to hold a CHAR16, do nothing.
  //
  if (StrLength < 1) {
    return ;
  }
  //
  // Get the screen setting and the current cursor location
  //
  StartColumn = ConOut->Mode->CursorColumn;
  Column      = StartColumn;
  Row         = ConOut->Mode->CursorRow;
  ConOut->QueryMode (ConOut, ConOut->Mode->Mode, &LineLength, &TotalRow);
  if (LineLength == 0) {
    return ;
  }

 // SetMem (InStr, StrLength * sizeof (CHAR16), 0);
  //prepare default string
  Len = StrLen(InStr);
  StrPos = 0;
  OutputLength = Len;
  Print(L"%s", InStr);
  Done = FALSE;
  ConOut->SetCursorPosition (ConOut, Column, Row);
  do {
    //
    // Read a key
    //
    WaitForSingleEvent (ConIn->WaitForKey, 0);
    ConIn->ReadKeyStroke (ConIn, &Key);

    switch (Key.UnicodeChar) {
    case CHAR_CARRIAGE_RETURN:
      //
      // All done, print a newline at the end of the string
      //
      TailRow     = Row + (Len - StrPos + Column) / LineLength;
      TailColumn  = (Len - StrPos + Column) % LineLength;
      Done        = TRUE;
      break;

    case CHAR_BACKSPACE:
      if (StrPos) {
        //
        // If not move back beyond string beginning, move all characters behind
        // the current position one character forward
        //
        StrPos -= 1;
        Update  = StrPos;
        Delete  = 1;
        CopyMem (InStr + StrPos, InStr + StrPos + 1, sizeof (CHAR16) * (Len - StrPos));

        //
        // Adjust the current column and row
        //
        ConMoveCursorBackward (LineLength, &Column, (UINTN *)&Row);
      }
      break;

    default:
      if (Key.UnicodeChar >= ' ') {
        //
        // If we are at the buffer's end, drop the key
        //
        if ((Len == StrLength - 1) && (InsertMode || StrPos == Len)) {
          break;
        }
        //
        // If in insert mode, move all characters behind the current position
        // one character backward to make space for this character. Then store
        // the character.
        //
        if (InsertMode) {
          for (Index = Len; Index > StrPos; Index--) {
            InStr[Index] = InStr[Index - 1];
          }
        }

        InStr[StrPos] = Key.UnicodeChar;
        Update        = StrPos;

        StrPos++;
        OutputLength++;
      }
      break;

    case 0:
      switch (Key.ScanCode) {
      case SCAN_DELETE:
        //
        // Move characters behind current position one character forward
        //
        if (Len) {
          Update  = StrPos;
          Delete  = 1;
          CopyMem (InStr + StrPos, InStr + StrPos + 1, sizeof (CHAR16) * (Len - StrPos));
        }
        break;

      case SCAN_LEFT:
        //
        // Adjust current cursor position
        //
        if (StrPos) {
          StrPos -= 1;
          ConMoveCursorBackward (LineLength, &Column, (UINTN *)&Row);
        }
        break;

      case SCAN_RIGHT:
        //
        // Adjust current cursor position
        //
        if (StrPos < Len) {
          StrPos += 1;
          ConMoveCursorForward (LineLength, TotalRow, &Column, (UINTN *)&Row);
        }
        break;

      case SCAN_HOME:
        //
        // Move current cursor position to the beginning of the command line
        //
        Row -= (StrPos + StartColumn) / LineLength;
        Column  = StartColumn;
        StrPos  = 0;
        break;

      case SCAN_END:
        //
        // Move current cursor position to the end of the command line
        //
        TailRow     = Row + (Len - StrPos + Column) / LineLength;
        TailColumn  = (Len - StrPos + Column) % LineLength;
        Row         = TailRow;
        Column      = TailColumn;
        StrPos      = Len;
        break;

      case SCAN_ESC:
        //
        // Prepare to clear the current command line
        //
        InStr[0]  = 0;
        Update    = 0;
        Delete    = Len;
        Row -= (StrPos + StartColumn) / LineLength;
        Column        = StartColumn;
        OutputLength  = 0;
        break;

      case SCAN_INSERT:
        //
        // Toggle the SEnvInsertMode flag
        //
        InsertMode = (BOOLEAN)!InsertMode;
        break;
      }
    }

    if (Done) {
      break;
    }
    //
    // If we need to update the output do so now
    //
    if (Update != -1) {
      PrintAt (Column, Row, L"%s%.*s", InStr + Update, Delete, L"");
      Len = StrLen (InStr);

      if (Delete) {
        SetMem (InStr + Len, Delete * sizeof (CHAR16), 0x00);
      }

      if (StrPos > Len) {
        StrPos = Len;
      }

      Update = (UINTN) -1;

      //
      // After using print to reflect newly updates, if we're not using
      // BACKSPACE and DELETE, we need to move the cursor position forward,
      // so adjust row and column here.
      //
      if (Key.UnicodeChar != CHAR_BACKSPACE && !(Key.UnicodeChar == 0 && Key.ScanCode == SCAN_DELETE)) {
        //
        // Calulate row and column of the tail of current string
        //
        TailRow     = Row + (Len - StrPos + Column + OutputLength) / LineLength;
        TailColumn  = (Len - StrPos + Column + OutputLength) % LineLength;

        //
        // If the tail of string reaches screen end, screen rolls up, so if
        // Row does not equal TailRow, Row should be decremented
        //
        // (if we are recalling commands using UPPER and DOWN key, and if the
        // old command is too long to fit the screen, TailColumn must be 79.
        //
        if (TailColumn == 0 && TailRow >= TotalRow && (UINTN) Row != TailRow) {
          Row--;
        }
        //
        // Calculate the cursor position after current operation. If cursor
        // reaches line end, update both row and column, otherwise, only
        // column will be changed.
        //
        if (Column + OutputLength >= LineLength) {
          SkipLength = OutputLength - (LineLength - Column);

          Row += SkipLength / LineLength + 1;
          if ((UINTN) Row > TotalRow - 1) {
            Row = TotalRow - 1;
          }

          Column = SkipLength % LineLength;
        } else {
          Column += OutputLength;
        }
      }

      Delete = 0;
    }
    //
    // Set the cursor position for this key
    //
    SetCursorPosition (ConOut, Column, Row, LineLength, TotalRow, InStr, StrPos, Len);
  } while (!Done);

  //
  // Return the data to the caller
  //
  return ;
}

VOID
SetCursorPosition (
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL     *ConOut,
  IN  UINTN                           Column,
  IN  INTN                            Row,
  IN  UINTN                           LineLength,
  IN  UINTN                           TotalRow,
  IN  CHAR16                          *Str,
  IN  UINTN                           StrPos,
  IN  UINTN                           Len
  )
{
  CHAR16  Backup;

//  ASSERT (ConOut != NULL);
//  ASSERT (Str != NULL);
  if (!ConOut || !Str) {
    return;
  }

  Backup = 0;
  if (Row >= 0) {
    ConOut->SetCursorPosition (ConOut, Column, Row);
    return ;
  }

  if (Len - StrPos > Column * Row) {
    Backup                          = *(Str + StrPos + Column * Row);
    *(Str + StrPos + Column * Row)  = 0;
  }

  PrintAt (0, 0, L"%s", Str + StrPos);
  if (Len - StrPos > Column * Row) {
    *(Str + StrPos + Column * Row) = Backup;
  }

  ConOut->SetCursorPosition (ConOut, 0, 0);
}
//
// //
/*
BOOLEAN
LibGetPrintOmit (
  VOID
  )
{
  return mPrintMode.OmitPrint;
}

VOID
LibSetPrintOmit (
  IN BOOLEAN    OmitPrint
  )
{
  EFI_TPL Tpl;

  Tpl                   = gBS->RaiseTPL (EFI_TPL_NOTIFY);
  mPrintMode.OmitPrint  = OmitPrint;
  gBS->RestoreTPL (Tpl);
}

VOID
LibEnablePageBreak (
  IN INT32      StartRow,
  IN BOOLEAN    AutoWrap
  )
{
  mPrintMode.PageBreak  = TRUE;
  mPrintMode.OmitPrint  = FALSE;
  mPrintMode.InitRow    = StartRow;
  mPrintMode.AutoWrap   = AutoWrap;

  //
  // Query Mode
  //
  gST->ConOut->QueryMode (
                gST->ConOut,
                gST->ConOut->Mode->Mode,
                (UINTN *)&mPrintMode.MaxColumn,
                (UINTN *)&mPrintMode.MaxRow
                );

  mPrintMode.Row = StartRow;
}

BOOLEAN
LibGetPageBreak (
  VOID
  )
{
  return mPrintMode.PageBreak;
}
*/

//
//VOID LowCase (IN OUT CHAR8 *Str)
//{
//  while (*Str) {
//    if (IS_UPPER(*Str)) {
//      *Str |= 0x20;
//    }
//    Str++;
//  }
//}

UINT8 hexstrtouint8 (CHAR8* buf)
{
	INT8 i = 0;
	if (IS_DIGIT(buf[0]))
		i = buf[0]-'0';
	else if (IS_HEX(buf[0]))
		i = (buf[0] | 0x20) - 'a' + 10;

	if (AsciiStrLen(buf) == 1) {
		return i;
	}
	i <<= 4;
	if (IS_DIGIT(buf[1]))
		i += buf[1]-'0';
	else if (IS_HEX(buf[1]))
		i += (buf[1] | 0x20) - 'a' + 10;

	return i;
}

BOOLEAN IsHexDigit (CHAR8 c) {
	return (IS_DIGIT(c) || (IS_HEX(c)))?TRUE:FALSE;
}

//out value is a number of byte.  out = len

UINT32 hex2bin(IN CHAR8 *hex, OUT UINT8 *bin, UINT32 len) //assume len = number of UINT8 values
{
	CHAR8	*p;
	UINT32	i, outlen = 0;
	CHAR8	buf[3];

	if (hex == NULL || bin == NULL || len <= 0 || AsciiStrLen(hex) < len * 2) {
    //		DBG("[ERROR] bin2hex input error\n"); //this is not error, this is empty value
		return FALSE;
	}

	buf[2] = '\0';
	p = (CHAR8 *) hex;

	for (i = 0; i < len; i++)
	{
		while ((*p == 0x20) || (*p == ',')) {
			p++; //skip spaces and commas
		}
		if (*p == 0) {
			break;
		}
		if (!IsHexDigit(p[0]) || !IsHexDigit(p[1])) {
			MsgLog("[ERROR] bin2hex '%s' syntax error\n", hex);
			return 0;
		}
		buf[0] = *p++;
		buf[1] = *p++;
		bin[i] = hexstrtouint8(buf);
		outlen++;
	}
	//bin[outlen] = 0;
	return outlen;
}

CHAR8* Bytes2HexStr(UINT8 *data, UINTN len)
{
  UINTN i, j, b = 0;
  CHAR8 *result = (CHAR8*)AllocateZeroPool((len*2)+1);
  //CHAR8 *buf = (__typeof__(buf))AllocateZeroPool(2);

  for (i = j = 0; i < len; i++) {
    b = data[i] >> 4;
    result[j++] = (CHAR8) (87 + b + (((b - 10) >> 31) & -39));
    b = data[i] & 0xf;
    result[j++] = (CHAR8) (87 + b + (((b - 10) >> 31) & -39));
    //buf = (CHAR8*)PoolPrint(L"%02x", (UINT8*)data[i]);
  }
  result[j] = '\0';
  //FreePool(buf);
  return result;
}
