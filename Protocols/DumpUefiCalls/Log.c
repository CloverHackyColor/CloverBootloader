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

#if LOG_TO_FILE >= 1
/** Pool for log into memory. */
static MEM_LOG gMemLog;
#endif


/** Prints log messages to outputs defined by PRINT_TO_* defs in Common.h. */
VOID
EFIAPI 
LogPrint(CHAR8 *Format, ...)
{
	VA_LIST		Marker;
	UINTN		DataWritten;
	
	if (gLogLineBuffer == NULL) {
		gLogLineBuffer = AllocateRuntimePool(LOG_LINE_BUFFER_SIZE);
		if (gLogLineBuffer == NULL) {
			return;
		}
		*gLogLineBuffer='\0';
	}
	
	// Safety check to avoid some cases where LogPrint may be run recursively
	if (*gLogLineBuffer != '\0') {
		return;
	}
	*gLogLineBuffer = '!'; // Make buffer "dirty" to indicate we are using it
	
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
	
	if (DataWritten > 0) {
		//
		// Dispatch to various loggers
		//
		#if LOG_TO_SCREEN == 1
		#if CAPTURE_CONSOLE_OUTPUT >= 1
		// Print to screen only if not invoked from our OutputString() override
		if (InBootServices && !InConsolePrint) {
			InConsolePrint = TRUE;
			AsciiPrint(gLogLineBuffer);
			InConsolePrint = FALSE;
		}
		#else
		if (InBootServices) {
			AsciiPrint(gLogLineBuffer);
		}
		#endif
		#endif
		
		#if LOG_TO_SERIAL >= 1
		#if (CAPTURE_CONSOLE_OUTPUT >= 1) && (LOG_TO_SERIAL == 1)
		// Print to serial only if not invoked from our OutputString() override
		if (!InBootServices || !InConsolePrint) {
			DebugPrint(1, gLogLineBuffer);
		}
		#else
		DebugPrint(1, gLogLineBuffer);
		#endif
		#endif
		
		#if LOG_TO_FILE >= 1
		if (InBootServices) {
			MemLogPrint(&gMemLog, gLogLineBuffer);
		}
		#endif
	}
	
	*gLogLineBuffer = '\0';
}


/** Called when ExitBootServices() is executed to give loggers a chance
 *  to use boot services (to save log to file for example).
 */
VOID
LogOnExitBootServices(VOID)
{
	#if LOG_TO_FILE >= 1
	EFI_STATUS	Status;
		
	Status = MemLogSave(&gMemLog);
	if (EFI_ERROR(Status)) {
		Print(L"ERROR saving log to a file: %r\n", Status);
		gBS->Stall(3000000);
	}
	#endif
}	
