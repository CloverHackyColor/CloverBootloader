/** @file

  Central logging  module.
  Dispatches logs to defined loggers.
  Loggers can be enabled/disabled by PRINT_TO_* definitions in Common.h.

  By dmazar, 26/09/2012             

**/

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Library/DebugLib.h>

#include "Common.h"
#include "MemLog.h"


/** The size of buffer for one log line. */
#define LOG_LINE_BUFFER_SIZE	1024

/** Buffer for one log line. */
CHAR8	*gLogLineBuffer = NULL;

/** Pool for log into memory. */
MEM_LOG gMemLog;


/** Prints log messages to outputs defined by PRINT_TO_* defs in Common.h. */
VOID
LogPrint(CHAR8 *Format, ...)
{
	VA_LIST		Marker;
	UINTN		DataWritten;
	
	if (gLogLineBuffer == NULL) {
		gLogLineBuffer = AllocateRuntimePool(LOG_LINE_BUFFER_SIZE);
		if (gLogLineBuffer == NULL) {
			return;
		}		
	}
	
	//
	// Print log to buffer
	//
	VA_START (Marker, Format);
	DataWritten = AsciiVSPrint(
		gLogLineBuffer,
		LOG_LINE_BUFFER_SIZE,
		Format,
		Marker);
	VA_END (Marker);
	
	//
	// Dispatch to various loggers
	//
	#if LOG_TO_SCREEN == 1
	if (InBootServices) {
		AsciiPrint(gLogLineBuffer);
	}
	#endif
	
	#if LOG_TO_SERIAL == 1
	DebugPrint(1, gLogLineBuffer);
	#endif
	
	#if LOG_TO_FILE == 1
	if (InBootServices) {
		MemLogPrint(&gMemLog, gLogLineBuffer);
	}
	#endif
}


/** Called when ExitBootServices() is executed to give loggers a chance
 *  to use boot services (to save log to file for example).
 */
VOID
LogOnExitBootServices(VOID)
{
	
	#if LOG_TO_FILE == 1
	{
		EFI_STATUS	Status;
		
		Status = MemLogSave(&gMemLog);
		if (EFI_ERROR(Status)) {
			Print(L"ERROR saving log to a file: %r\n", Status);
			gBS->Stall(3000000);
		}
	}
	#endif
}	
