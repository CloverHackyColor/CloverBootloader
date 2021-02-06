//
//  DebugLib.c
//  cpp_tests UTF16 signed char
//
//  Created by Jief on 12/10/2020.
//  Copyright © 2020 JF Knudsen. All rights reserved.
//

#include <Library/DebugLib.h>
#include "../../../../rEFIt_UEFI/Platform/Posix/abort.h"

/**
  Prints an assert message containing a filename, line number, and description.
  This may be followed by a breakpoint or a dead loop.

  Print a message of the form "ASSERT <FileName>(<LineNumber>): <Description>\n"
  to the debug output device.  If DEBUG_PROPERTY_ASSERT_BREAKPOINT_ENABLED bit of
  PcdDebugProperyMask is set then CpuBreakpoint() is called. Otherwise, if
  DEBUG_PROPERTY_ASSERT_DEADLOOP_ENABLED bit of PcdDebugProperyMask is set then
  CpuDeadLoop() is called.  If neither of these bits are set, then this function
  returns immediately after the message is printed to the debug output device.
  DebugAssert() must actively prevent recursion.  If DebugAssert() is called while
  processing another DebugAssert(), then DebugAssert() must return immediately.

  If FileName is NULL, then a <FileName> string of "(NULL) Filename" is printed.
  If Description is NULL, then a <Description> string of "(NULL) Description" is printed.

  @param  FileName     The pointer to the name of the source file that generated the assert condition.
  @param  LineNumber   The line number in the source file that generated the assert condition
  @param  Description  The pointer to the description of the assert condition.

**/
VOID
EFIAPI
DebugAssert (
  IN CONST CHAR8  *FileName,
  IN UINTN        LineNumber,
  IN CONST CHAR8  *Description
  )
{
//  panic();
//  panic("ouch\n");
  panic("%s %llu %s\n", FileName, LineNumber, Description);
}


/**
  Returns TRUE if ASSERT() macros are enabled.

  This function returns TRUE if the DEBUG_PROPERTY_DEBUG_ASSERT_ENABLED bit of
  PcdDebugProperyMask is set.  Otherwise, FALSE is returned.

  @retval  TRUE    The DEBUG_PROPERTY_DEBUG_ASSERT_ENABLED bit of PcdDebugProperyMask is set.
  @retval  FALSE   The DEBUG_PROPERTY_DEBUG_ASSERT_ENABLED bit of PcdDebugProperyMask is clear.

**/
BOOLEAN
EFIAPI
DebugAssertEnabled (
  VOID
  )
{
  return TRUE;
}
RETURN_STATUS
EFIAPI
BaseDebugLibSerialPortConstructor (
  VOID
  )
{
  return EFI_SUCCESS;
}
VOID
EFIAPI
DebugPrint (
  IN  UINTN        ErrorLevel,
  IN  CONST CHAR8  *Format,
  ...
  )
{
  VA_LIST  Marker;

  VA_START (Marker, Format);
  vprintf(Format, Marker);
  VA_END (Marker);
}

BOOLEAN
EFIAPI
DebugCodeEnabled (
  VOID
  )
{
  return TRUE;
}

BOOLEAN
EFIAPI
DebugPrintEnabled (
  VOID
  )
{
  return TRUE;
}

BOOLEAN
EFIAPI
DebugPrintLevelEnabled (
  IN  CONST UINTN        ErrorLevel
  )
{
  return TRUE;
}
