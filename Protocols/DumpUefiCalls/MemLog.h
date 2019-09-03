/** @file

  Log to memory buffer.

  By dmazar, 26/09/2012             

**/

#ifndef __DMP_MEM_LOG_H__
#define __DMP_MEM_LOG_H__


/** Memory log buffer and sizes. */
typedef struct {
	CHAR8	*Buffer;
	UINTN	Size;
	UINTN	BufferSize;
} MEM_LOG;


/** Prints log messages to memory buffer. */
EFI_STATUS
EFIAPI
MemLogPrint(IN MEM_LOG *MemLog, IN CHAR8 *Format, ...);

/** Saves log to Log.txt in the same dir as driver. */
EFI_STATUS
MemLogSave(IN MEM_LOG *MemLog);


#endif // __DMP_MEM_LOG_H__
