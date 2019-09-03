/*
 *  MsgLog.h
 *  
 *
 *  Created by Slice on 10.04.12.
 *  Copyright 2012 Home. All rights reserved.
 *
 */

#ifndef __MESSAGE_LOG_H__
#define __MESSAGE_LOG_H__

#include <Uefi.h>
/*#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>
*/

#define BOOTER_LOG_SIZE (4 * 1024)
#define MESSAGE_LOG_PROTOCOL_GUID \
{ \
0x511CE018, 0x0018, 0x4002, {0x20, 0x12, 0x17, 0x38, 0x05, 0x01, 0x02, 0x03} \
}

#define BootLog(...) if(msgCursor){AsciiSPrint(msgCursor, BOOTER_LOG_SIZE, __VA_ARGS__); while(*msgCursor){msgCursor++;} Msg->Cursor = msgCursor;}

/* sample using
#include <Protocol/MsgLog.h> 
#include <Library/PrintLib.h>

CHAR8 *msgCursor;
MESSAGE_LOG_PROTOCOL *Msg; 
 
 Msg = NULL;
 Status = gBS->LocateProtocol(&gMsgLogProtocolGuid, NULL, (VOID **) &Msg);
 if (!EFI_ERROR(Status) && (Msg != NULL)) {
   msgCursor = Msg->Cursor;
 }
 
*/
typedef struct {
  UINT32    SizeOfLog;
  BOOLEAN   Dirty;
  CHAR8     *Log;
  CHAR8     *Cursor;
} MESSAGE_LOG_PROTOCOL;

extern EFI_GUID gMsgLogProtocolGuid;

#endif
