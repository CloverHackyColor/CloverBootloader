/** @file
  Default instance of MemLogLib library for simple log services to memory buffer.             
**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemLogLib.h>

//
// Mem log sizes
//
#define MEM_LOG_INITIAL_SIZE    (256 * 1024)
#define MEM_LOG_MAX_SIZE        (2 * 1024 * 1024)
#define MEM_LOG_MAX_LINE_SIZE   1024

//
// Struct for holding mem buffer.
//
typedef struct {
  CHAR8             *Buffer;
  CHAR8             *Cursor;
  UINTN             BufferSize;
  MEM_LOG_CALLBACK  Callback;
} MEM_LOG;


//
// Guid for internal protocol for publishing mem log buffer.
//
EFI_GUID  mMemLogProtocolGuid = { 0x74B91DA4, 0x2B4C, 0x11E2, {0x99, 0x03, 0x22, 0xF0, 0x61, 0x88, 0x70, 0x9B } };

//
// Pointer to mem log buffer.
//
MEM_LOG   *mMemLog = NULL;


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
  EFI_STATUS      Status;
  
  if (mMemLog != NULL) {
    return  EFI_SUCCESS;
  }
  
  //
  // Try to use existing MEM_LOG
  //
  Status = gBS->LocateProtocol (&mMemLogProtocolGuid, NULL, (VOID **) &mMemLog);
  if (Status == EFI_SUCCESS && mMemLog != NULL) {
    //
    // We are inited with existing MEM_LOG
    //
    return EFI_SUCCESS;
  }
  
  //
  // Set up and publish new MEM_LOG
  //
  mMemLog = AllocateZeroPool ( sizeof (MEM_LOG) );
  if (mMemLog == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  mMemLog->BufferSize = MEM_LOG_INITIAL_SIZE;
  mMemLog->Buffer = AllocateZeroPool (MEM_LOG_INITIAL_SIZE);
  mMemLog->Cursor = mMemLog->Buffer;
  mMemLog->Callback = NULL;
  
  Status = gBS->InstallMultipleProtocolInterfaces (
                                                   &gImageHandle,
                                                   &mMemLogProtocolGuid,
                                                   mMemLog,
                                                   NULL
                                                   );
  return Status;
}

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
  )
{
  EFI_STATUS      Status;
  UINTN           DataWritten;
  CHAR8           *LastMessage;
  
  if (Format == NULL) {
    return;
  }
  
  if (mMemLog == NULL) {
    Status = MemLogInit ();
    if (EFI_ERROR (Status)) {
      return;
    }
  }
  
  //
  // Check if buffer can accept MEM_LOG_MAX_LINE_SIZE chars.
  // Increase buffer if not.
  //
  if ((UINTN)(mMemLog->Cursor - mMemLog->Buffer) + MEM_LOG_MAX_LINE_SIZE > mMemLog->BufferSize) {
      // not enough place for max line - make buffer bigger
      // but not too big (if something gets out of controll)
      if (mMemLog->BufferSize + MEM_LOG_INITIAL_SIZE > MEM_LOG_MAX_SIZE) {
      // Out of resources!
        return;
      }
      mMemLog->Buffer = ReallocatePool(mMemLog->BufferSize, mMemLog->BufferSize + MEM_LOG_INITIAL_SIZE, mMemLog->Buffer);
      mMemLog->BufferSize += MEM_LOG_INITIAL_SIZE;
    }
  
  //
  // Add log to buffer
  //
  LastMessage = mMemLog->Cursor;
  DataWritten = AsciiVSPrint(
                             mMemLog->Cursor,
                             mMemLog->BufferSize - (mMemLog->Cursor - mMemLog->Buffer),
                             Format,
                             Marker);
  mMemLog->Cursor += DataWritten;
  
  //
  // Pass this last message to callback if defined
  //
  if (mMemLog->Callback != NULL) {
    mMemLog->Callback(DebugMode, LastMessage);
  }
}

/**
  Prints a log to message memory buffer.
 
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
  )
{
  VA_LIST           Marker;
  
  if (Format == NULL) {
    return;
  }
  
  VA_START (Marker, Format);
  MemLogVA (DebugMode, Format, Marker);
  VA_END (Marker);
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
  EFI_STATUS        Status;
  
  if (mMemLog == NULL) {
    Status = MemLogInit ();
    if (EFI_ERROR (Status)) {
      return NULL;
    }
  }
  
  return mMemLog != NULL ? mMemLog->Buffer : NULL;
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
  EFI_STATUS        Status;
  
  if (mMemLog == NULL) {
    Status = MemLogInit ();
    if (EFI_ERROR (Status)) {
      return 0;
    }
  }
  
  return mMemLog != NULL ? mMemLog->Cursor - mMemLog->Buffer : 0;
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
  EFI_STATUS        Status;
  
  if (mMemLog == NULL) {
    Status = MemLogInit ();
    if (EFI_ERROR (Status)) {
      return;
    }
  }
  mMemLog->Callback = Callback;
}
