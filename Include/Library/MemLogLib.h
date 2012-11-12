/** @file
    Provides simple log services to memory buffer.
**/

#ifndef __MEMLOG_LIB_H__
#define __MEMLOG_LIB_H__



/** Callback that can be installed to be called when some message is printed with MemLog() or MemLogVA(). **/
typedef VOID (EFIAPI *MEM_LOG_CALLBACK) (IN INTN DebugMode, IN CHAR8 *LastMessage);


/**
  Prints a log message to memory buffer.
 
  @param  DebugMode   DebugMode will be passed to Callback function if it is set.
  @param  Format      The format string for the debug message to print.
  @param  Marker      VA_LIST with variable arguments for Format.
 
**/
VOID
EFIAPI
MemLogVA (
  IN  CONST INTN    DebugMode,
  IN  CONST CHAR8   *Format,
  IN  VA_LIST       Marker
  );

/**
  Prints a log message to memory buffer.

  If Format is NULL, then does nothing.

  @param  DebugMode   DebugMode will be passed to Callback function if it is set.
  @param  Format      The format string for the debug message to print.
  @param  ...         The variable argument list whose contents are accessed 
                      based on the format string specified by Format.

**/
VOID
EFIAPI
MemLog (
  IN  CONST INTN    DebugMode,
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


/**
  Sets callback that will be called when message is added to mem log.
 **/
VOID
EFIAPI
SetMemLogCallback (
  MEM_LOG_CALLBACK  Callback
  );


#endif // __MEMLOG_LIB_H__
