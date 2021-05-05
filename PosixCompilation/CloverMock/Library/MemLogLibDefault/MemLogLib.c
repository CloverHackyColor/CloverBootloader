/** @file
  Default instance of MemLogLib library for simple log services to memory buffer.
**/

#include <Efi.h>
#include <Library/printf_lite.h>

CHAR8*
GetTiming(VOID)
{
  return "";
}



/**
  Inits mem log.

  @retval EFI_SUCCESS   The constructor always returns EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
MemLogInit (
  VOID
  )
{
  return EFI_SUCCESS;
}

/**
  Prints a log message to memory buffer.
 
  @param  Timing      TRUE to prepend timing to log.
  @param  DebugMode   DebugMode will be passed to Callback function if it is set.
  @param  Format      The format string for the debug message to print.
  @param  Marker      VA_LIST with variable arguments for Format.
 
**/
VOID
EFIAPI
MemLogVA (
  IN  CONST BOOLEAN Timing,
  IN  CONST INTN    DebugMode,
  IN  CONST CHAR8   *Format,
  IN  VA_LIST       Marker
  )
{
  panic("not yet");
}

/**
  Prints a log to message memory buffer.
 
  If Format is NULL, then does nothing.
 
  @param  Timing      TRUE to prepend timing to log.
  @param  DebugMode   DebugMode will be passed to Callback function if it is set.
  @param  Format      The format string for the debug message to print.
  @param  ...         The variable argument list whose contents are accessed
  based on the format string specified by Format.
 
 **/
VOID
EFIAPI
MemLog (
  IN  CONST BOOLEAN Timing,
  IN  CONST INTN    DebugMode,
  IN  CONST CHAR8   *Format,
  ...
  )
{
  panic("not yet");
}



/**
 Returns pointer to MemLog buffer.
 **/
CHAR8*
EFIAPI
GetMemLogBuffer (
  VOID
  )
{
  panic("not yet");
}



/**
 Returns the length of log (number of chars written) in mem buffer.
 **/
UINTN
EFIAPI
GetMemLogLen (
  VOID
  )
{
  panic("not yet");
}


/**
  Sets callback that will be called when message is added to mem log.
 **/
VOID
EFIAPI
SetMemLogCallback (
  MEM_LOG_CALLBACK  Callback
  )
{
  panic("not yet");
}


/**
  Sets callback that will be called when message is added to mem log.
 **/
MEM_LOG_CALLBACK
EFIAPI
GetMemLogCallback ()
{
  panic("not yet");
}


/**
  Returns TSC ticks per second.
 **/
UINT64
EFIAPI
GetMemLogTscTicksPerSecond (VOID)
{
  panic("not yet");
}



// Microsoft wants _fltused
#ifdef _MSC_VER
#ifdef __cplusplus
extern "C" {
#endif
int _fltused=0; // it should be a single underscore since the double one is the mangled name
#ifdef __cplusplus
}
#endif
#endif

//static int printfNewline = 1;
//static void transmitS8Printf(const char* buf, unsigned int nbchar, void* context)
//{
//}

//const char* printf_lite_get_timestamp()
//{
//  return "";
//}

/**
  Prints a log message to memory buffer.

  @param  Timing      TRUE to prepend timing to log.
  @param  DebugMode   DebugMode will be passed to Callback function if it is set.
  @param  Format      The format string for the debug message to print.
  @param  Marker      VA_LIST with variable arguments for Format.

**/
VOID
EFIAPI
MemLogfVA (
  IN  CONST BOOLEAN Timing,
  IN  CONST INTN    DebugMode,
  IN  CONST CHAR8   *Format,
  IN  VA_LIST       Marker
  )
{
  panic("not yet");
}


/**
  Prints a log to message memory buffer.

  If Format is NULL, then does nothing.

  @param  Timing      TRUE to prepend timing to log.
  @param  DebugMode   DebugMode will be passed to Callback function if it is set.
  @param  Format      The format string for the debug message to print.
  @param  ...         The variable argument list whose contents are accessed
  based on the format string specified by Format.

 **/
VOID
EFIAPI
MemLogf (
  IN  CONST BOOLEAN Timing,
  IN  CONST INTN    DebugMode,
  IN  CONST CHAR8   *Format,
  ...
  )
{
  panic("not yet");
}

