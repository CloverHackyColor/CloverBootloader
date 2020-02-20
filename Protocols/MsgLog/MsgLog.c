/*
 *  MsgLog.c
 *  
 *
 *  Created by Slice on 10.04.12.
 *  Copyright 2012 Home. All rights reserved.
 *
 */

#include <Protocol/MsgLog.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>

MESSAGE_LOG_PROTOCOL MsgLogProtocol;
EFI_HANDLE              mHandle = NULL;

//define BOOTER_LOG_SIZE	(4 * 1024)

/* Sample using inside other module:
@header.h file 
 #include <Protocol/MsgLog.h> 
 #include <Library/PrintLib.h>
extern  CHAR8 *msgCursor;
extern  MESSAGE_LOG_PROTOCOL *Msg; 

@main.c
 #if DEBUG_ACPI==2
 #define DBG(...)  AsciiPrint(__VA_ARGS__)
 #elif DEBUG_ACPI==1
 #define DBG(...)  BootLog(__VA_ARGS__)
 #else
 #define DBG(...)	
 #endif
 
 CHAR8 *msgCursor;
 MESSAGE_LOG_PROTOCOL *Msg; 
@entry point
 Msg = NULL;
 Status = gBS->LocateProtocol(&gMsgLogProtocolGuid, NULL, (VOID **) &Msg);
 if (!EFI_ERROR(Status) && (Msg != NULL)) {
   msgCursor = Msg->Cursor;
   BootLog("MsgLog Protocol installed in AcpiPlatform\n");
 }
@inf
 [Packages]
   CloverPkg.dec
 
 [LibraryClasses]
    PrintLib

 [Protocols]
    gMsgLogProtocolGuid

 [Depex]
 ...
 AND gMsgLogProtocolGuid  -- no!

 
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
                    IN EFI_SYSTEM_TABLE		*SystemTable
                    )
{
  EFI_STATUS					Status; // = EFI_SUCCESS;
 // EFI_BOOT_SERVICES*			gBS; 
  CHAR8    *tmp;
  
//  gBS				= SystemTable->BootServices;
  mHandle = NULL;
  Status = gBS->AllocatePool (
                 EfiBootServicesData,
                 BOOTER_LOG_SIZE,
                 (VOID**) &tmp
                 );
	if (EFI_ERROR (Status)) {
		return Status;
	}
//  Print(L"MsgLogProtocol installed!\n");
  SetMem(tmp, BOOTER_LOG_SIZE, 0);
  MsgLogProtocol.Log = tmp;	
  MsgLogProtocol.Cursor = tmp;
  MsgLogProtocol.SizeOfLog = 0;
  MsgLogProtocol.Dirty = FALSE;
  
  Status = gBS->InstallMultipleProtocolInterfaces (
                &mHandle,
                &gMsgLogProtocolGuid,
                &MsgLogProtocol,
                NULL
                );
  
  return Status;
}
