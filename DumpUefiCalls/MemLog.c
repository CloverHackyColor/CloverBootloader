/** @file

  Log to memory buffer.

  By dmazar, 26/09/2012

**/


#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>

#include "Common.h"
#include "MemLog.h"

//
// Mem log sizes
//
#define MEM_LOG_INITIAL_SIZE	(32 * 1024)
#define MEM_LOG_MAX_SIZE	(16 * 1024 * 1024)
#define MEM_LOG_MAX_LINE_SIZE	1024

/** Prints log messages to memory buffer. */
EFI_STATUS EFIAPI 
MemLogPrint(IN MEM_LOG *MemLog, IN CHAR8 *Format, ...)
{
   EFI_STATUS Status = EFI_SUCCESS;
	VA_LIST		Marker;
	UINTN		DataWritten;
	
	if (MemLog->Buffer == NULL) {
		// init log buffer
		MemLog->BufferSize = MEM_LOG_INITIAL_SIZE;
		MemLog->Buffer = AllocateZeroPool(MemLog->BufferSize);
		MemLog->Size = 0;
	}
	
	if (MemLog->Size + MEM_LOG_MAX_LINE_SIZE > MemLog->BufferSize) {
		// not enough place for max line - make buffer bigger
		// but not too big (if something gets out of controll)
		if (MemLog->BufferSize * 2 > MEM_LOG_MAX_SIZE) {
			return EFI_OUT_OF_RESOURCES;
		}
		MemLog->Buffer = ReallocatePool(MemLog->BufferSize, MemLog->BufferSize * 2, MemLog->Buffer);
		MemLog->BufferSize *= 2;
	}
	
	// add log to buffer
	VA_START (Marker, Format);
	DataWritten = AsciiVSPrint(
		MemLog->Buffer + MemLog->Size,
		MemLog->BufferSize - MemLog->Size,
		Format,
		Marker);
	VA_END (Marker);
	MemLog->Size += DataWritten;
	
	#if LOG_TO_FILE >= 3
	// append to file while logging (at end of line)
	if (DataWritten >= 1 && *(MemLog->Buffer + MemLog->Size - 1) == '\n') {
		Status = FsAppendMemToFileToDefaultDir(LOG_TO_FILE_PATH, (VOID*)MemLog->Buffer, MemLog->Size);
		MemLog->Size = 0;
	}
	#endif
	
	return Status;
}


/** Saves log to Log.txt in the same dir as driver. */
EFI_STATUS
MemLogSave(IN MEM_LOG *MemLog)
{
	# if (LOG_TO_FILE == 1) || (LOG_TO_FILE == 2)
	return FsSaveMemToFileToDefaultDir(LOG_TO_FILE_PATH, (VOID*)MemLog->Buffer, MemLog->Size);
	# elif LOG_TO_FILE >= 3
	if (MemLog->Size > 0) {
		FsAppendMemToFileToDefaultDir(LOG_TO_FILE_PATH, (VOID*)MemLog->Buffer, MemLog->Size); 
	}
	return FsAppendMemClose(TRUE);
	# else
	return EFI_SUCCESS;
	# endif
}	
