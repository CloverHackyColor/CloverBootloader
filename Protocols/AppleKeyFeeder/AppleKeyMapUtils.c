/*
 *  AppleKeyMapUtils.c
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



EFI_GUID gAppleKeyMapDatabaseProtocolGuid = {0x584B9EBE, 0x80C1, 0x4BD6, {0x98, 0xB0, 0xA7, 0x78, 0x6E, 0xC2, 0xF2, 0xE2}};
APPLE_KEY_MAP_DATABASE_PROTOCOL *AppleKeyMapDb = NULL;
UINTN AppleKeyMapDbIndex = 0;

static EFI_STATUS getAppleKeyMapDb()
{
  EFI_STATUS Status = EFI_SUCCESS;

    Status = gBS->LocateProtocol (
                  &gAppleKeyMapDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&AppleKeyMapDb
                  );
	if (!EFI_ERROR(Status))
	{
		Status = AppleKeyMapDb->CreateKeyStrokesBuffer (
								AppleKeyMapDb,
								6,
								&AppleKeyMapDbIndex
								);
		if (EFI_ERROR(Status)) {
			DBG("AppleKeyFeederEntrypoint: CreateKeyStrokesBuffer failed, Status=%x\n", Status);
		}
	}else{
		DBG("AppleKeyFeederEntrypoint: LocateProtocol failed, Status=%x\n", Status);
	}
	return Status;
}



static EFI_EVENT eventKeyRelease;

static VOID
EFIAPI
SendKeyRelease (
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  )
{
  EFI_STATUS Status;
//DBG("SendKeyRelease\n");

  APPLE_KEY_CODE appleKey; // APPLE_KEY_CODE is UINT16
    	Status = AppleKeyMapDb->SetKeyStrokeBufferKeys (
							   AppleKeyMapDb,
							   AppleKeyMapDbIndex,
							   (APPLE_MODIFIER_MAP)0,
							   0,
							   &appleKey
							   );
		if (EFI_ERROR(Status)) {
    		DBG("SendKeyRelease: SetKeyStrokeBufferKeys failed, Status=%x\n", Status);
		}

    	Status = gBS->SetTimer (
					  eventKeyRelease,
					  TimerCancel,
					  10*1000
					  );
		if (EFI_ERROR(Status)) {
    		DBG("SendKeyRelease: SetTimer failed, Status=%x\n", Status);
		}
}



static int MapKeyData2AppleKey(EFI_KEY_DATA* KeyData, APPLE_KEY_CODE* pKey, UINT8* pCurModifierMap)
{
	if ( KeyData->Key.UnicodeChar == 0 )
	{
		*pCurModifierMap = 0;
		switch (KeyData->Key.ScanCode) {
			case 0x01: // up arrow
				*pKey = 0x7052;
				return 1;
			case 0x02: // down arrow
				*pKey = 0x7051;
				return 1;
			case 0x03: // left arrow
				*pKey = 0x7050;
				return 1;
			case 0x04: // right arrow
				*pKey = 0x704F;
				return 1;
		}
		return 0;
	}
	if ( KeyData->Key.UnicodeChar >= 'a'  &&  KeyData->Key.UnicodeChar <= 'z' ) {
		*pKey = 0x7004 + ( KeyData->Key.UnicodeChar - 'a');
		*pCurModifierMap = 0;
		return 1;
	}
	if ( KeyData->Key.UnicodeChar >= 'A'  &&  KeyData->Key.UnicodeChar <= 'Z' ) {
		*pKey = 0x7004 + ( KeyData->Key.UnicodeChar - 'A');
		*pCurModifierMap = 2;
		return 1;
	}
	if ( KeyData->Key.UnicodeChar == '0' ) { // Could have put that in the switch, but wanted to make very clear that the 0 wasn't forgotten and that this : "KeyData->Key.UnicodeChar >= '1'" (instead of '0') is not a mistake !!!
		*pKey = 0x7027;
		return 1;
	}
	if ( KeyData->Key.UnicodeChar >= '1'  &&  KeyData->Key.UnicodeChar <= '9' ) {
		*pKey = 0x701E + ( KeyData->Key.UnicodeChar - '1');
		return 1;
	}

	*pCurModifierMap = 0;
	switch (KeyData->Key.UnicodeChar) {
		case 0x01: // up arrow
			*pKey = 0x7052;
			return 1;
		case 0x02: // down arrow
			*pKey = 0x7051;
			return 1;
		case 0x03: // left arrow
			*pKey = 0x7050;
			return 1;
		case 0x04: // right arrow
			*pKey = 0x704F;
			return 1;
		case 0x08: // backspace
			*pKey = 0x702A;
			return 1;
		case 0x0D: // return
			*pKey = 0x7028;
			return 1;
		case ' ': // return
			*pKey = 0x702C;
			return 1;
		case '!': // return
			*pKey = 0x701E;
			*pCurModifierMap = 2;
			return 1;
		case '"': // return
			*pKey = 0x7034;
			*pCurModifierMap = 2;
			return 1;
		case '#': // return
			*pKey = 0x7020;
			*pCurModifierMap = 2;
			return 1;
		case '$': // return
			*pKey = 0x7021;
			*pCurModifierMap = 2;
			return 1;
		case '%': // return
			*pKey = 0x7022;
			*pCurModifierMap = 2;
			return 1;
		case '&': // return
			*pKey = 0x7024;
			*pCurModifierMap = 2;
			return 1;
		case '\'': // return
			*pKey = 0x7034;
			return 1;
		case '(': // return
			*pKey = 0x7026;
			*pCurModifierMap = 2;
			return 1;
		case ')': // return
			*pKey = 0x7027;
			*pCurModifierMap = 2;
			return 1;
		case '*': // return
			*pKey = 0x7025;
			*pCurModifierMap = 2;
			return 1;
		case '+': // return
			*pKey = 0x702E;
			*pCurModifierMap = 2;
			return 1;
		case ',': // return
			*pKey = 0x7036;
			return 1;
		case '-': // return
			*pKey = 0x702D;
			return 1;
		case '.': // return
			*pKey = 0x7037;
			return 1;
		case '/': // return
			*pKey = 0x7038;
			return 1;

		case ':': // return
			*pKey = 0x7033;
			*pCurModifierMap = 2;
			return 1;
		case ';': // return
			*pKey = 0x7033;
			return 1;
		case '<': // return
			*pKey = 0x7036;
			*pCurModifierMap = 2;
			return 1;
		case '=': // return
			*pKey = 0x702E;
			return 1;
		case '>': // return
			*pKey = 0x7037;
			*pCurModifierMap = 2;
			return 1;
		case '?': // return
			*pKey = 0x7038;
			*pCurModifierMap = 2;
			return 1;
		case '@': // return
			*pKey = 0x701F;
			*pCurModifierMap = 2;
			return 1;


		case '[': // return
			*pKey = 0x702F;
			return 1;
		case '\\': // return
			*pKey = 0x7031;
			return 1;
		case ']': // return
			*pKey = 0x7030;
			return 1;
		case '^': // return
			*pKey = 0x7023;
			*pCurModifierMap = 2;
			return 1;
		case '_': // return
			*pKey = 0x702D;
			*pCurModifierMap = 2;
			return 1;
		case '`': // return
			*pKey = 0x7035;
			return 1;

		case '{': // return
			*pKey = 0x702F;
			*pCurModifierMap = 2;
			return 1;
		case '|': // return
			*pKey = 0x7031;
			*pCurModifierMap = 2;
			return 1;
		case '}': // return
			*pKey = 0x7030;
			*pCurModifierMap = 2;
			return 1;
		case '~': // return
			*pKey = 0x7035;
			*pCurModifierMap = 2;
			return 1;
	}
	return 0;
}


EFI_STATUS
EFIAPI
SendDataToAppleMap(IN EFI_KEY_DATA *KeyData)
{
//DBG("SendDataToAppleMap ScanCode=%x, Uchar=%x, ShiftState=%x, ToogleState=%x\n", KeyData->Key.ScanCode, KeyData->Key.UnicodeChar, KeyData->KeyState.KeyShiftState, KeyData->KeyState.KeyToggleState);

  EFI_STATUS Status;
  UINT8 CurModifierMap  = 0;
  UINTN NumberOfKeys = 1;
  APPLE_KEY_CODE appleKey; // APPLE_KEY_CODE is UINT16

	if ( !AppleKeyMapDb ) {
		Status = getAppleKeyMapDb();
    	if (EFI_ERROR(Status)) {
    		DBG("SendDataToAppleMap: getAppleKeyMapDb() failed, Status=%x\n", Status);
    	}
	}
	if ( !AppleKeyMapDb ) {
		DBG("SendDataToAppleMap: AppleKeyMapDb==NULL, Status=%x\n", EFI_NOT_FOUND);
		return EFI_NOT_FOUND;
	}

    if ( MapKeyData2AppleKey(KeyData, &appleKey, &CurModifierMap) )
    {
    	Status = AppleKeyMapDb->SetKeyStrokeBufferKeys (
							   AppleKeyMapDb,
							   AppleKeyMapDbIndex,
							   (APPLE_MODIFIER_MAP)CurModifierMap,
							   NumberOfKeys,
							   &appleKey
							   );
    	if (EFI_ERROR(Status)) {
    		DBG("SendDataToAppleMap: SetKeyStrokeBufferKeys failed, Status=%x\n", Status);
    		return Status;
    	}


		Status = gBS->SetTimer (
					  eventKeyRelease,
					  TimerPeriodic,
					  10*1000*15 // 15 ms
					  );
		if (EFI_ERROR(Status)) {
			// if this fail, we end up having the key repeated.
    		DBG("SendDataToAppleMap: SetTimer failed, Status=%x\n", Status);
    		return Status;
		}

    	return Status;
    }
DBG("SendDataToAppleMap: key not mapped, sorry\n");
    return EFI_NOT_FOUND;
}


/****************************************************************
 * Entry point
 ***************************************************************/

EFI_STATUS
EFIAPI
AppleKeyMapUtilsInit (
                    IN EFI_HANDLE           ImageHandle,
                    IN EFI_SYSTEM_TABLE		*SystemTable
                    )
{
DBG("AppleKeyMapUtilsInit\n");
  EFI_STATUS Status = EFI_SUCCESS;
  
	//
	// Setup a periodic timer, used for send a key release to apple key map
	//
	Status = gBS->CreateEvent (
				  EVT_TIMER | EVT_NOTIFY_SIGNAL,
				  TPL_NOTIFY,
				  SendKeyRelease,
				  NULL,
				  &eventKeyRelease
				  );
	if ( EFI_ERROR(Status) ) {
		DBG("AppleKeyMapUtilsInit: CreateEvent failed, Status=%x\n", Status);
	}

DBG("AppleKeyMapUtilsInit returns %x\n", Status);
  return Status;
}

