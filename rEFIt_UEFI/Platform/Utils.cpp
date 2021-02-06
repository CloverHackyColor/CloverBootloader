///*++
//
//All rights reserved. This program and the accompanying materials
//are licensed and made available under the terms and conditions of the BSD License
//which accompanies this distribution. The full text of the license may be found at
//http://opensource.org/licenses/bsd-license.php
//
//THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
//WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
//
//Module Name:
//
//  IO.c
//
//Abstract:
//
//  the IO library function
//
//Revision History
//
//--*/
//
#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include <Efi.h>
//
//void LowCase (IN OUT CHAR8 *Str)
//{
//  while (*Str) {
//    if (IS_UPPER(*Str)) {
//      *Str |= 0x20;
//    }
//    Str++;
//  }
//}

UINT8 hexstrtouint8 (const CHAR8* buf)
{
	INT8 i = 0;
	if (IS_DIGIT(buf[0]))
		i = buf[0]-'0';
	else if (IS_HEX(buf[0]))
		i = (buf[0] | 0x20) - 'a' + 10;

	if (AsciiStrLen(buf) == 1) {
		return i;
	}
	i <<= 4;
	if (IS_DIGIT(buf[1]))
		i += buf[1]-'0';
	else if (IS_HEX(buf[1]))
		i += (buf[1] | 0x20) - 'a' + 10;

	return i;
}

BOOLEAN IsHexDigit (CHAR8 c) {
	return (IS_DIGIT(c) || (IS_HEX(c)))?TRUE:FALSE;
}

//out value is a number of byte.  out = len

UINT32 hex2bin(IN const CHAR8 *hex, OUT UINT8 *bin, UINT32 len) //assume len = number of UINT8 values
{
	CHAR8	*p;
	UINT32	i, outlen = 0;
	CHAR8	buf[3];

	if (hex == NULL || bin == NULL || len <= 0 || AsciiStrLen(hex) < len * 2) {
    //		DBG("[ERROR] bin2hex input error\n"); //this is not error, this is empty value
		return FALSE;
	}

	buf[2] = '\0';
	p = (CHAR8 *) hex;

	for (i = 0; i < len; i++)
	{
		while ( *p == 0x20  ||  *p == ','  ||  *p == '\n'  ||  *p == '\r' ) {
			p++; //skip spaces and commas
		}
		if (*p == 0) {
			break;
		}
		if (!IsHexDigit(p[0]) || !IsHexDigit(p[1])) {
			MsgLog("[ERROR] bin2hex '%s' syntax error\n", hex);
			return 0;
		}
		buf[0] = *p++;
		buf[1] = *p++;
		bin[i] = hexstrtouint8(buf);
		outlen++;
	}
	//bin[outlen] = 0;
	return outlen;
}

XString8 Bytes2HexStr(UINT8 *data, UINTN len)
{
  UINTN i, j, b = 0;
  XString8 result;

  result.dataSized(len*2+1);
  for (i = j = 0; i < len; i++) {
    b = data[i] >> 4;
    result += (CHAR8) (87 + b + (((b - 10) >> 31) & -39));
    b = data[i] & 0xf;
    result += (CHAR8) (87 + b + (((b - 10) >> 31) & -39));
  }
  return result;
}


BOOLEAN haveError = FALSE;


BOOLEAN CheckFatalError(IN EFI_STATUS Status, IN CONST CHAR16 *where)
{
//    CHAR16 ErrorName[64];

    if (!EFI_ERROR(Status))
        return FALSE;

    MsgLog("Fatal Error: %s %ls\n", efiStrError(Status), where);

//    StatusToString(ErrorName, Status);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLACK);
    printf("Fatal Error: %s %ls\n", efiStrError(Status), where);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK);
    haveError = TRUE;

    //gBS->Exit(ImageHandle, ExitStatus, ExitDataSize, ExitData);

    return TRUE;
}

BOOLEAN CheckError(IN EFI_STATUS Status, IN CONST CHAR16 *where)
{
//    CHAR16 ErrorName[64];

    if (!EFI_ERROR(Status))
        return FALSE;

    MsgLog("Fatal Error: %s %ls\n", efiStrError(Status), where);

//    StatusToString(ErrorName, Status);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLACK);
    printf("Error: %s %ls\n", efiStrError(Status), where);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK);
    haveError = TRUE;

    return TRUE;
}


