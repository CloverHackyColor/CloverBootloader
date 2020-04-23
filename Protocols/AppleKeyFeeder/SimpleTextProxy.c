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

static EFI_HANDLE OriginalConsoleInHandle;
static EFI_SIMPLE_TEXT_INPUT_PROTOCOL* OriginalSimpleText;

EFI_SIMPLE_TEXT_INPUT_PROTOCOL AppleKeyFeederSimpleTextProxy;



/**
  Reset the input device and optionally run diagnostics

  @param  This                 Protocol instance pointer.
  @param  ExtendedVerification Driver may perform diagnostics on reset.

  @retval EFI_SUCCESS          The device was reset.
  @retval EFI_DEVICE_ERROR     The device is not functioning properly and could not be reset.

**/

EFI_STATUS
(EFIAPI SimpleTextProxyReset)(
  IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL       *This,
  IN BOOLEAN                              ExtendedVerification
  )
{
DBG("SimpleTextProxyReset\n");
	return OriginalSimpleText->Reset(OriginalSimpleText, ExtendedVerification);
}

/*
 * Buffer for EFI_INPUT_KEY
 */
static EFI_INPUT_KEY KeyDataBuffer[32]; // size must be a power of 2
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
EFIAPI SimpleTextProxyReadKeyStroke(
  IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL       *This,
  OUT EFI_INPUT_KEY                       *Key
  )
{
//DBG("SimpleTextProxyReadKeyStroke\n");
	if ( AppleKeyFeederSimpleTextExProxy.ReadKeyStrokeEx )
	{
		EFI_KEY_DATA KeyData;
		EFI_STATUS Status = SimpleTextExProxyReadKeyStroke(&AppleKeyFeederSimpleTextExProxy, &KeyData);
		if ( EFI_ERROR(Status) ) {
			return Status;
		}
		*Key = KeyData.Key;
	}
	else
	{
		if ( KeyDataBufferIndexStart == KeyDataBufferIndexEnd ) {
			return EFI_NOT_READY;
	//DBG("SimpleTextProxyReadKeyStroke: returns EFI_NOT_READY (%x)\n", EFI_NOT_READY);
		}
		*Key = KeyDataBuffer[KeyDataBufferIndexStart];
	//DBG("SimpleTextProxyReadKeyStroke before KeyDataBufferIndexStart=%d, KeyDataBufferIndexEnd=%d\n", KeyDataBufferIndexStart, KeyDataBufferIndexEnd);
		KeyDataBufferIndexStart = (KeyDataBufferIndexStart + 1) & KEYDATABUFFER_SIZE_MINUS_ONE;
	}
	return EFI_SUCCESS;
}

/*
 * Buffering keys
 */
VOID
EFIAPI
SimpleTextProxyBufferKeyStroke (
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  )
{
  EFI_STATUS Status;
//DBG("SimpleTextProxyBufferKeyStroke\n");

    EFI_INPUT_KEY key;

    Status = OriginalSimpleText->ReadKeyStroke(OriginalSimpleText, &key);
	if (EFI_ERROR(Status) ) {
		//DBG("SimpleTextProxyBufferKeyStroke ReadKeyStroke failed, Status=%x\n", Status);
	}else{
DBG("SimpleTextProxyBufferKeyStroke ReadKey key ScanCode=%x, UnicodeChar=%x, KeyDataBufferIndexStart=%d, KeyDataBufferIndexEnd=%d\n", key.ScanCode, key.UnicodeChar, KeyDataBufferIndexStart, KeyDataBufferIndexEnd);

		KeyDataBuffer[KeyDataBufferIndexEnd] = key;
		// if key are not consumed, and buffer is full, the buffer will be lost.
		// That happens at Apple preboot env which doesn't use ConIn. But at preboot env, we don't care about key stored in buffer.
		KeyDataBufferIndexEnd = (KeyDataBufferIndexEnd + 1) & KEYDATABUFFER_SIZE_MINUS_ONE;
		gBS->SignalEvent(Event);

		EFI_KEY_DATA KeyData;
		KeyData.Key.ScanCode = key.ScanCode;
		KeyData.Key.UnicodeChar = key.UnicodeChar;
		KeyData.KeyState.KeyShiftState = KeyData.KeyState.KeyToggleState = 0;
		SendDataToAppleMap(&KeyData);
	}

	return;
}

/*
 *
 */
VOID
EFIAPI
WaitForKeyEvent (
  IN EFI_EVENT          Event,
  IN VOID               *Context
  )
{
DBG("WaitForKeyEvent\n");
	if ( KeyDataBufferIndexStart != KeyDataBufferIndexEnd ) {
		gBS->SignalEvent (Event);
	}
}


/****************************************************************
 * Entry point
 ***************************************************************/
EFI_STATUS
EFIAPI
SimpleTextProxyInit(
                    IN EFI_HANDLE           ImageHandle,
                    IN EFI_SYSTEM_TABLE		*SystemTable
                    )
{
DBG("SimpleTextProxyInit\n");
	EFI_STATUS Status = EFI_SUCCESS;


	OriginalConsoleInHandle = SystemTable->ConsoleInHandle;
    OriginalSimpleText = SystemTable->ConIn;

    AppleKeyFeederSimpleTextProxy.Reset = SimpleTextProxyReset;
    AppleKeyFeederSimpleTextProxy.ReadKeyStroke = SimpleTextProxyReadKeyStroke;

    Status = SimpleTextExProxyInit(ImageHandle, SystemTable);
	if ( EFI_ERROR(Status) ) {
		DBG("SimpleTextProxyInit: CreateEvent2 failed, Status=%x\n", Status);
	}

    SystemTable->ConsoleInHandle = ImageHandle;
  	SystemTable->ConIn = &AppleKeyFeederSimpleTextProxy;
  	gST->ConIn = &AppleKeyFeederSimpleTextProxy;

DBG("SimpleTextProxyInit returns %x\n", Status);
  return Status;
}

