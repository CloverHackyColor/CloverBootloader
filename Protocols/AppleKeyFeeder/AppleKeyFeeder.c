/*
 *  AppleKeyFeeder.c
 *  
 *  Created by Jief on 25 May 2018.
 *
 */

#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/MemLogLib.h>

#include <IndustryStandard/AppleHid.h>
#include <Protocol/AppleKeyMapDatabase.h>
#include "AppleKeyMapUtils.h"
#include "SimpleTextProxy.h"
#include "SimpleTextExProxy.h"


// DBG_TO: 0=no debug, 1=serial, 2=console 3=log
// serial requires
// [PcdsFixedAtBuild]
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x07
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0xFFFFFFFF
// in package DSC file

#ifdef JIEF_DEBUG
#define DBG_APPLEKEYFEEDER 2
#endif

#if DBG_APPLEKEYFEEDER == 3
#define DBG(...) MemLog(FALSE, 0, __VA_ARGS__)
#elif DBG_APPLEKEYFEEDER == 2
#define DBG(...) AsciiPrint(__VA_ARGS__)
#elif DBG_APPLEKEYFEEDER == 1
#define DBG(...) DebugPrint(1, __VA_ARGS__)
#else
#define DBG(...)
#endif

EFI_EVENT eventReadKeystroke;
EFI_SIMPLE_TEXT_INPUT_PROTOCOL AppleKeyFeederProxy;

/*
 * Proxying ConIn seems to be the "right" way to go.
 * But if it's not : ReadKeyStroke will happen concurrently and you might lose some keystrokes in Clover.
 * In practice, AppleKeyFeeder read a key every 30ms.
 * On my MacPro/VMWare, ReadKeyStroke takes 1,5 micro-second, on a x222 : 2 micro-second, on a Dell D630 : 1 millisecond
 */


/****************************************************************
 * Entry point
 ***************************************************************/

/**
 * AppleKeyFeeder entry point. Installs gEfiSimpleTextInProtocolGuid.
 */
EFI_STATUS
EFIAPI
AppleKeyFeederEntrypoint (
                    IN EFI_HANDLE           ImageHandle,
                    IN EFI_SYSTEM_TABLE		*SystemTable
                    )
{
DBG("AppleKeyFeederEntrypoint\n");
  EFI_STATUS Status = EFI_SUCCESS;
  
    Status = AppleKeyMapUtilsInit(ImageHandle, SystemTable);
	if ( EFI_ERROR (Status) ) {
		DBG("AppleKeyFeederEntrypoint: AppleKeyMapUtilsInit failed, Status=%x\n", Status);
	}

    Status = SimpleTextProxyInit(ImageHandle, SystemTable);
	if ( EFI_ERROR (Status) ) {
		DBG("AppleKeyFeederEntrypoint: SimpleTextProxyInit failed, Status=%x\n", Status);
	}


	//
	// Setup an event for WaitForKey
	//
	EFI_EVENT_NOTIFY notifyFunction;
	if ( AppleKeyFeederSimpleTextExProxy.ReadKeyStrokeEx ) notifyFunction = WaitForKeyExEvent;
	else notifyFunction = WaitForKeyEvent;
	Status = gBS->CreateEvent (
				  EVT_NOTIFY_WAIT,
				  TPL_NOTIFY,
				  notifyFunction,
				  NULL,
				  &AppleKeyFeederSimpleTextExProxy.WaitForKeyEx
				  );
	if ( EFI_ERROR (Status) ) {
		DBG("SimpleTextProxyInit: CreateEvent AppleKeyFeederSimpleTextExProxy.WaitForKeyEx, Status=%x\n", Status);
	}
	Status = gBS->CreateEvent (
				  EVT_NOTIFY_WAIT,
				  TPL_NOTIFY,
				  notifyFunction,
				  NULL,
				  &AppleKeyFeederSimpleTextProxy.WaitForKey
				  );
	if ( EFI_ERROR (Status) ) {
		DBG("SimpleTextProxyInit: CreateEvent AppleKeyFeederSimpleTextProxy.WaitForKey, Status=%x\n", Status);
	}

	//
	// Setup a periodic timer, used for reading keystrokes at a fixed interval
	//
	if ( AppleKeyFeederSimpleTextExProxy.ReadKeyStrokeEx ) {
		Status = gBS->CreateEvent (
					  EVT_TIMER | EVT_NOTIFY_SIGNAL,
					  TPL_NOTIFY,
					  SimpleTextExProxyBufferKeyStroke,
					  NULL,
					  &eventReadKeystroke
					  );
	}else{
		Status = gBS->CreateEvent (
					  EVT_TIMER | EVT_NOTIFY_SIGNAL,
					  TPL_NOTIFY,
					  SimpleTextProxyBufferKeyStroke,
					  NULL,
					  &eventReadKeystroke
					  );
	}
	if ( EFI_ERROR (Status) ) {
		DBG("SimpleTextProxyInit: CreateEvent2 failed, Status=%x\n", Status);
	}
	Status = gBS->SetTimer (
				  eventReadKeystroke,
				  TimerPeriodic,
				  10*1000*20 // 10*1000*1000 = 1s, so this 20ms
				  );
	if (EFI_ERROR (Status)) {
		DBG("SimpleTextProxyInit: SetTimer2 failed, Status=%x\n", Status);
	}

	if ( AppleKeyFeederSimpleTextExProxy.ReadKeyStrokeEx ) {
		Status = gBS->InstallMultipleProtocolInterfaces (
					&ImageHandle,
					&gEfiSimpleTextInProtocolGuid,
					&AppleKeyFeederSimpleTextProxy,
					&gEfiSimpleTextInputExProtocolGuid,
					&AppleKeyFeederSimpleTextExProxy,
					NULL
					);
	}else{
		Status = gBS->InstallMultipleProtocolInterfaces (
					&ImageHandle,
					&gEfiSimpleTextInProtocolGuid,
					&AppleKeyFeederSimpleTextProxy,
					NULL
					);
	}
	if (EFI_ERROR (Status)) {
		DBG("SimpleTextProxyInit: InstallMultipleProtocol failed, Status=%x\n", Status);
	}

  DBG("AppleKeyFeederEntrypoint returns %x\n", Status);
  return Status;
}

