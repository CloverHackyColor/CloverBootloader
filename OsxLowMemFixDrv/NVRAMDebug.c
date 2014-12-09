/**

  NVRAM debug stuff.

  by dmazar

**/

#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>

#include "NVRAMDebug.h"


// DBG_TO: 0=no debug, 1=serial, 2=console
// serial requires
// [PcdsFixedAtBuild]
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x07
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0xFFFFFFFF
// in package DSC file
#define DBG_TO 0

#if DBG_TO == 2
	#define DBG(...) AsciiPrint(__VA_ARGS__);
#elif DBG_TO == 1
	#define DBG(...) DebugPrint(1, __VA_ARGS__);
#else
	#define DBG(...)
#endif


// NVRAM var guid.
EFI_GUID gEfiNVRAMDebugVarGuid = DEBUG_LOG_NVRAM_VAR_GUID;

// Debug log buffer.
CHAR8	DebugLogBuffer[DEBUG_LOG_SIZE];
// Pointer to the current end (beginning of a free space) of the log in the buffer.
CHAR8	*DebugLogBufferPtr = NULL;


/** Appends given data to the log and writes it to NVRAM variable. */
EFI_STATUS EFIAPI 
NVRAMDebugLog(CHAR8 *Format, ...)
{
	EFI_STATUS	Status;
	VA_LIST		Marker;
	UINTN		DataSize;
	UINTN		DataWritten;
	
	if (DebugLogBufferPtr == NULL) {
		// init pointer to the buffer start
		DebugLogBufferPtr = DebugLogBuffer;
	}
	DataSize = DEBUG_LOG_SIZE - (DebugLogBufferPtr - DebugLogBuffer);
	
	if (DataSize < 2) {
		return EFI_BUFFER_TOO_SMALL;
	}
	
	// add text to buffer
	VA_START (Marker, Format);
	DataWritten = AsciiVSPrint(DebugLogBufferPtr, DataSize, Format, Marker);
	VA_END (Marker);
	
	DebugLogBufferPtr += DataWritten;
	
	// delete prev value
	//Status = gRT->SetVariable(L"DebugLog", &gEfiNVRAMDebugVarGuid,
	//	EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
	//	0, DebugLogBuffer);
	// write new value
	Status = gRT->SetVariable(
							  DEBUG_LOG_NVRAM_VAR_NAME,
							  &gEfiNVRAMDebugVarGuid,
							  EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
							  DebugLogBufferPtr - DebugLogBuffer,
							  DebugLogBuffer
							  );
	
	return Status;
}
