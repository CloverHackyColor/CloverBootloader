//
//  Header.h
//  cpp_tests
//
//  Created by Jief on 30/01/2021.
//  Copyright © 2021 JF Knudsen. All rights reserved.
//

#ifndef PrintLib_h
#define PrintLib_h

/**
  Produces a Null-terminated ASCII string in an output buffer based on a Null-terminated
  ASCII format string and  variable argument list.

  This function is similar as snprintf_s defined in C11.

  Produces a Null-terminated ASCII string in the output buffer specified by StartOfBuffer
  and BufferSize.
  The ASCII string is produced by parsing the format string specified by FormatString.
  Arguments are pulled from the variable argument list based on the contents of the
  format string.
  The number of ASCII characters in the produced output buffer is returned not including
  the Null-terminator.

  If BufferSize > 0 and StartOfBuffer is NULL, then ASSERT(). Also, the output buffer is
  unmodified and 0 is returned.
  If BufferSize > 0 and FormatString is NULL, then ASSERT(). Also, the output buffer is
  unmodified and 0 is returned.
  If PcdMaximumAsciiStringLength is not zero, and BufferSize >
  (PcdMaximumAsciiStringLength * sizeof (CHAR8)), then ASSERT(). Also, the output buffer
  is unmodified and 0 is returned.
  If PcdMaximumAsciiStringLength is not zero, and FormatString contains more than
  PcdMaximumAsciiStringLength Ascii characters not including the Null-terminator, then
  ASSERT(). Also, the output buffer is unmodified and 0 is returned.

  If BufferSize is 0, then no output buffer is produced and 0 is returned.

  @param  StartOfBuffer   A pointer to the output buffer for the produced Null-terminated
                          ASCII string.
  @param  BufferSize      The size, in bytes, of the output buffer specified by StartOfBuffer.
  @param  FormatString    A Null-terminated ASCII format string.
  @param  ...             Variable argument list whose contents are accessed based on the
                          format string specified by FormatString.

  @return The number of ASCII characters in the produced output buffer not including the
          Null-terminator.

**/
UINTN
EFIAPI
AsciiSPrint (
  OUT CHAR8        *StartOfBuffer,
  IN  UINTN        BufferSize,
  IN  CONST CHAR8  *FormatString,
  ...
  );


#endif /* PrintLib_h */
