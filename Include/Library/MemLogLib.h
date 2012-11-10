/** @file
    Provides simple log services to memory buffer.
**/

#ifndef __MEMLOG_LIB_H__
#define __MEMLOG_LIB_H__


/**
  Prints a log message to memory buffer.
 
  @param  Format      The format string for the debug message to print.
  @param  Marker      VA_LIST with variable arguments for Format.
 
**/
VOID
EFIAPI
MemLogVA (
  IN  CONST CHAR8   *Format,
  IN  VA_LIST       Marker
  );

/**
  Prints a log message to memory buffer.

  If Format is NULL, then does nothing.

  @param  Format      The format string for the debug message to print.
  @param  ...         The variable argument list whose contents are accessed 
                      based on the format string specified by Format.

**/
VOID
EFIAPI
MemLog (
  IN  CONST CHAR8   *Format,
  ...
  );


/**
  Returns pointer to MemLog buffer.
**/
CHAR8*
EFIAPI
GetMemLogBuffer (
  VOID
  );


/**
  Returns the length of log (number of chars written) in mem buffer.
 **/
UINTN
EFIAPI
GetMemLogLen (
  VOID
  );


#endif // __MEMLOG_LIB_H__
