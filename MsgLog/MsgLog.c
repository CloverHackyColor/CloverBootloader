/*
 *  MsgLog.c
 *  
 *
 *  Created by Slice on 10.04.12.
 *  Copyright 2012 Home. All rights reserved.
 *
 */

#include <Protocol/MsgLog.h>

MESSAGE_LOG_PROTOCOL MsgLogProtocol;
EFI_HANDLE              mHandle = NULL;

//CHAR8 *msgbuf = NULL;
//CHAR8 *msgCursor = NULL;
//define BOOTER_LOG_SIZE	(4 * 1024)

//Status = gBS->UninstallMultipleProtocolInterfaces (
  // Free  protocol occupied resource
  //
/*  if (msgbuf != NULL) {
    gBS->FreePool (msgbuf);
  }
 */ 
  

/**************************************************************************************
 * Entry point
 **************************************************************************************/

/**
 * MsgLog entry point. Installs MESSAGE_LOG_PROTOCOL.
 */
EFI_STATUS
EFIAPI
MsgLogEntrypoint (
                    IN EFI_HANDLE           ImageHandle,
                    IN EFI_SYSTEM_TABLE			*SystemTable
                    )
{
  EFI_STATUS					Status = EFI_SUCCESS;
  EFI_BOOT_SERVICES*			gBS; 
  CHAR8    *tmp;
  
  gBS				= SystemTable->BootServices;
  mHandle = NULL;
  Status = gBS->AllocatePool (
                 EfiBootServicesData,
                 BOOTER_LOG_SIZE,
                 (VOID**) &tmp
                 );
	if (EFI_ERROR (Status)) {
		return Status;
	}
  gBS->SetMem (tmp, BOOTER_LOG_SIZE, 0);
  MsgLogProtocol.Log = tmp;	
	MsgLogProtocol.Cursor = tmp;
  MsgLogProtocol.SizeOfLog = 0;
  
  Status = gBS->InstallMultipleProtocolInterfaces (
                &mHandle,
                &gMsgLogProtocolGuid,
                &MsgLogProtocol,
                NULL
                );
  
  return Status;
}
