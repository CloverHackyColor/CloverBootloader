/** @file

  Central logging  module.
  Dispatches logs to defined loggers.
  Loggers can be enabled/disabled by PRINT_TO_* definitions in Common.h.

  By dmazar, 26/09/2012             

**/

#ifndef __DMP_LOG_H__
#define __DMP_LOG_H__


/** Buffer for one log line. */
extern CHAR8	*gLogLineBuffer;

/** Prints log messages to outputs defined by PRINT_TO_* defs in Common.h. */
VOID
EFIAPI
LogPrint(CHAR8 *Format, ...);

/** Called when ExitBootServices() is executed to give loggers a chance
 *  to use boot services (to save log to file for example).
 */
VOID
LogOnExitBootServices(VOID);


#endif // __DMP_LOG_H__
