/*
 *  BootLog.c
 *  
 *
 *  Created by Slice on 19.08.11.

 *  Initial concept from Kabyl
 */


#include "Platform.h"

CHAR8 *msgbuf = 0;
CHAR8 *msgCursor = 0;


VOID InitBooterLog(VOID)
{
	msgbuf = AllocateZeroPool(BOOTER_LOG_SIZE);
	msgCursor = msgbuf;
}

EFI_STATUS SetupBooterLog(VOID)
{
	EFI_STATUS		Status	= EFI_SUCCESS;
	UINTN			LogSize = msgCursor - msgbuf;
	if (!msgbuf)
		return EFI_NOT_FOUND;
		
	Status =  LogDataHub(&gEfiMiscSubClassGuid, L"boot-log", msgbuf, LogSize);
	return Status;
}
/* Kabyl: !BooterLog */


