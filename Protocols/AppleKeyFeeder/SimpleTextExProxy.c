/*
 *  SimpleTextProxy.h
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

// DBG_TO: 0=no debug, 1=serial, 2=console 3=log
// serial requires
// [PcdsFixedAtBuild]
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x07
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0xFFFFFFFF
// in package DSC file

#ifdef JIEF_DEBUG
#define DBG_APPLEKEYFEEDER 0
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

static EFI_HANDLE OriginalConsoleInHandle;
static EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* OriginalSimpleTextEx;

EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL AppleKeyFeederSimpleTextExProxy;



/**
  Reset the input device and optionally run diagnostics

  @param  This                 Protocol instance pointer.
  @param  ExtendedVerification Driver may perform diagnostics on reset.

  @retval EFI_SUCCESS          The device was reset.
  @retval EFI_DEVICE_ERROR     The device is not functioning properly and could not be reset.

**/

EFI_STATUS
(EFIAPI SimpleTextExProxyReset)(
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
  IN BOOLEAN                           ExtendedVerification
  )
{
DBG("SimpleTextExProxyReset\n");
	return OriginalSimpleTextEx->Reset(OriginalSimpleTextEx, ExtendedVerification);
}

/*
 * Buffer for EFI_INPUT_KEY
 */
static EFI_KEY_DATA KeyDataBuffer[32]; // size must be a power of 2
#define KEYDATABUFFER_SIZE_MINUS_ONE ( ( sizeof(KeyDataBuffer) / sizeof(KeyDataBuffer[0]) ) - 1 )
static UINTN KeyDataBufferIndexStart = 0;
static UINTN KeyDataBufferIndexEnd = 0;

/**
  Reads the next keystroke from the input device. The WaitForKey Event can
  be used to test for existence of a keystroke via WaitForEvent () call.

  @param  This  Protocol instance pointer.
  @param  Key   A pointer to a buffer that is filled in with the keystroke
                information for the key that was pressed.

  @retval EFI_SUCCESS      The keystroke information was returned.
  @retval EFI_NOT_READY    There was no keystroke data available.
  @retval EFI_DEVICE_ERROR The keystroke information was not returned due to
                           hardware errors.

**/
EFI_STATUS
EFIAPI SimpleTextExProxyReadKeyStroke(
  IN  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
  OUT EFI_KEY_DATA                      *KeyData
  )
{
//DBG("SimpleTextExProxyReadKeyStroke\n");
	if ( KeyDataBufferIndexStart == KeyDataBufferIndexEnd ) {
		return EFI_NOT_READY;
//DBG("AppleKeyFeederReadKeyStroke: returns EFI_NOT_READY (%x)\n", EFI_NOT_READY);
	}
	*KeyData = KeyDataBuffer[KeyDataBufferIndexStart];
//DBG("AppleKeyFeederReadKeyStroke before KeyDataBufferIndexStart=%d, KeyDataBufferIndexEnd=%d\n", KeyDataBufferIndexStart, KeyDataBufferIndexEnd);
	KeyDataBufferIndexStart = (KeyDataBufferIndexStart + 1) & KEYDATABUFFER_SIZE_MINUS_ONE;
	return EFI_SUCCESS;
}

/*
 * Buffering keys
 */
void
EFIAPI
SimpleTextExProxyBufferKeyStroke (
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  )
{
  EFI_STATUS Status;
//DBG("AppleKeyFeederSimpleTextProxyReadKeyStroke\n");

    EFI_KEY_DATA keyData;

    Status = OriginalSimpleTextEx->ReadKeyStrokeEx(OriginalSimpleTextEx, &keyData);
	if (EFI_ERROR(Status) ) {
		//DBG("SimpleTextExProxyBufferKeyStroke ReadKeyStroke failed, Status=%x\n", Status);
	}else{
//DBG("SimpleTextExProxyBufferKeyStroke ReadKeyData Key.ScanCode=%x, Key.UnicodeChar=%x, KeyState.KeyShiftState=%x, KeyState.KeyToggleState=%x, KeyDataBufferIndexStart=%d, KeyDataBufferIndexEnd=%d\n", keyData.Key.ScanCode, keyData.Key.UnicodeChar, keyData.KeyState.KeyShiftState, keyData.KeyState.KeyToggleState, KeyDataBufferIndexStart, KeyDataBufferIndexEnd);

		KeyDataBuffer[KeyDataBufferIndexEnd] = keyData;
		// if key are not consumed, and buffer is full, the buffer will be lost.
		// That happens at Apple preboot env which doesn't use ConIn. But at preboot env, we don't care about key stored in buffer.
		KeyDataBufferIndexEnd = (KeyDataBufferIndexEnd + 1) & KEYDATABUFFER_SIZE_MINUS_ONE;
		gBS->SignalEvent(Event);

		SendDataToAppleMap(&keyData);
	}

	return;
}

/**
  The SetState() function allows the input device hardware to
  have state settings adjusted.

  @param This           A pointer to the EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL instance.

  @param KeyToggleState Pointer to the EFI_KEY_TOGGLE_STATE to
                        set the state for the input device.


  @retval EFI_SUCCESS       The device state was set appropriately.

  @retval EFI_DEVICE_ERROR  The device is not functioning
                            correctly and could not have the
                            setting adjusted.

  @retval EFI_UNSUPPORTED   The device does not support the
                            ability to have its state set.

**/
EFI_STATUS
(EFIAPI SimpleTextExProxySetState)(
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
  IN EFI_KEY_TOGGLE_STATE              *KeyToggleState
)
{
DBG("SimpleTextExProxySetState\n");
	return OriginalSimpleTextEx->SetState(OriginalSimpleTextEx, KeyToggleState);
}


/**
  The RegisterKeystrokeNotify() function registers a function
  which will be called when a specified keystroke will occur.

  @param This                     A pointer to the EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL instance.

  @param KeyData                  A pointer to a buffer that is filled in with
                                  the keystroke information for the key that was
                                  pressed. If KeyData.Key, KeyData.KeyState.KeyToggleState
                                  and KeyData.KeyState.KeyShiftState are 0, then any incomplete
                                  keystroke will trigger a notification of the KeyNotificationFunction.

  @param KeyNotificationFunction  Points to the function to be called when the key sequence
                                  is typed specified by KeyData. This notification function
                                  should be called at <=TPL_CALLBACK.


  @param NotifyHandle             Points to the unique handle assigned to
                                  the registered notification.

  @retval EFI_SUCCESS           Key notify was registered successfully.

  @retval EFI_OUT_OF_RESOURCES  Unable to allocate necessary
                                data structures.

**/
EFI_STATUS
(EFIAPI SimpleTextExProxyRegisterKeyNotify)(
  IN  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
  IN  EFI_KEY_DATA                      *KeyData,
  IN  EFI_KEY_NOTIFY_FUNCTION           KeyNotificationFunction,
  OUT VOID                              **NotifyHandle
)
{
DBG("SimpleTextExProxyRegisterKeyNotify\n");
	return OriginalSimpleTextEx->RegisterKeyNotify(OriginalSimpleTextEx, KeyData, KeyNotificationFunction, NotifyHandle);
}


/**
  The UnregisterKeystrokeNotify() function removes the
  notification which was previously registered.

  @param This               A pointer to the EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL instance.

  @param NotificationHandle The handle of the notification
                            function being unregistered.

  @retval EFI_SUCCESS           Key notify was unregistered successfully.

  @retval EFI_INVALID_PARAMETER The NotificationHandle is
                                invalid.

**/
EFI_STATUS
(EFIAPI SimpleTextExProxyUnregisterKeyNotify)(
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN VOID                               *NotificationHandle
)
{
DBG("SimpleTextExProxyUnregisterKeyNotify\n");
	return OriginalSimpleTextEx->UnregisterKeyNotify(OriginalSimpleTextEx, NotificationHandle);
}

/*
 *
 */
VOID
EFIAPI
WaitForKeyExEvent (
  IN EFI_EVENT          Event,
  IN VOID               *Context
  )
{
//DBG("WaitForKeyExEvent\n");
	if ( KeyDataBufferIndexStart != KeyDataBufferIndexEnd ) {
		gBS->SignalEvent (Event);
	}
}


/****************************************************************
 * Entry point
 ***************************************************************/

/**
 *
 */
EFI_STATUS
EFIAPI
SimpleTextExProxyInit(
                    IN EFI_HANDLE           ImageHandle,
                    IN EFI_SYSTEM_TABLE		*SystemTable
                    )
{
DBG("SimpleTextExProxyInit\n");
	EFI_STATUS Status = EFI_SUCCESS;


	OriginalConsoleInHandle = SystemTable->ConsoleInHandle;
    Status = gBS->HandleProtocol(gST->ConsoleInHandle, &gEfiSimpleTextInputExProtocolGuid, (VOID **)&OriginalSimpleTextEx);
	if ( EFI_ERROR(Status) ) {
		DBG("SimpleTextExProxyInit: CreateEvent2 failed, Status=%x\n", Status);
		goto bail;
	}

    AppleKeyFeederSimpleTextExProxy.Reset = SimpleTextExProxyReset;
    AppleKeyFeederSimpleTextExProxy.ReadKeyStrokeEx = SimpleTextExProxyReadKeyStroke;
    AppleKeyFeederSimpleTextExProxy.SetState = SimpleTextExProxySetState;
    AppleKeyFeederSimpleTextExProxy.RegisterKeyNotify = SimpleTextExProxyRegisterKeyNotify;
    AppleKeyFeederSimpleTextExProxy.UnregisterKeyNotify = SimpleTextExProxyUnregisterKeyNotify;

DBG("SimpleTextExProxyInit: AppleKeyFeederSimpleTextExProxy=%x, AppleKeyFeederSimpleTextExProxy.ReadKeyStrokeEx=%x\n", &AppleKeyFeederSimpleTextExProxy, AppleKeyFeederSimpleTextExProxy.ReadKeyStrokeEx);

//	//
//	// Setup a periodic timer, used for reading keystrokes at a fixed interval
//	//
//	Status = gBS->CreateEvent (
//				  EVT_NOTIFY_WAIT,
//				  TPL_NOTIFY,
//				  WaitForKeyExEvent,
//				  NULL,
//				  &AppleKeyFeederSimpleTextExProxy.WaitForKeyEx
//				  );
//	if ( EFI_ERROR(Status) ) {
//		DBG("SimpleTextExProxyInit: CreateEvent2 failed, Status=%x\n", Status);
//		goto bail;
//	}
//

bail:
	if ( EFI_ERROR(Status) ) {
		AppleKeyFeederSimpleTextExProxy.ReadKeyStrokeEx = NULL;
	}
DBG("SimpleTextExProxyInit returns %x\n", Status);
  return Status;
}

