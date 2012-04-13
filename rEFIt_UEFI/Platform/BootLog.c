/*
 *  BootLog.c
 *  
 *
 *  Created by Slice on 19.08.11.

 *  Initial idea from Kabyl
 */


#include "Platform.h"
extern  EFI_GUID  gEfiMiscSubClassGuid;

CHAR8 *msgbuf = 0;
CHAR8 *msgCursor = 0;


VOID InitBooterLog(VOID)
{
  EFI_STATUS		Status	= EFI_SUCCESS;
  MESSAGE_LOG_PROTOCOL*         Msg;
  INTN  N;
   
	msgbuf = AllocateZeroPool(MSG_LOG_SIZE);
	msgCursor = msgbuf;
  
  Status = gBS->LocateProtocol (&gMsgLogProtocolGuid, NULL, (VOID **)&Msg);
   
   if (!EFI_ERROR (Status)) 
   {
     if (Msg->Log && Msg->Cursor) {
       N =(INTN)(Msg->Cursor - Msg->Log);
       if ((N > 0) && (N < MSG_LOG_SIZE)) {
         MsgLog("Log from Clover size=%d:\n", N);
         MsgLog("%a\n", (CHAR8*)Msg->Log);
         FreePool(Msg->Log);         
       }
       else {
         MsgLog("no BootLog from CloverEFI\n");
       }
     }
   }
}

EFI_STATUS SetupBooterLog(VOID)
{
	EFI_STATUS		Status	= EFI_SUCCESS;
	UINTN			LogSize;
  CHAR16*    BootLogName = L"EFI\\misc\\boot.log";
	if (!msgbuf)
		return EFI_NOT_FOUND;
	LogSize  = msgCursor - msgbuf;
	Status =  LogDataHub(&gEfiMiscSubClassGuid, L"boot-log", msgbuf, LogSize);
  Status = egSaveFile(SelfRootDir, BootLogName, (UINT8*)msgbuf, LogSize);
  if (EFI_ERROR(Status)) {
    Status = egSaveFile(NULL, BootLogName, (UINT8*)msgbuf, LogSize);
  }
   
	return Status;
}
/* Kabyl: !BooterLog */


