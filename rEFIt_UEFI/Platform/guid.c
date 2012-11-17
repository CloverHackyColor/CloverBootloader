/**
 guid.c
 **/

#include "Platform.h"

//EFI_GUID gEfiConsoleControlProtocolGuid         = {0xF42F7782, 0x012E, 0x4C12, {0x99, 0x56, 0x49, 0xF9, 0x43, 0x04, 0xF7, 0x21}};
//EFI_GUID gEfiAppleFirmwarePasswordProtocolGuid  = {0x8FFEEB3A, 0x4C98, 0x4630, {0x80, 0x3F, 0x74, 0x0F, 0x95, 0x67, 0x09, 0x1D}};
//EFI_GUID gEfiGlobalVarGuid                      = {0x8BE4DF61, 0x93CA, 0x11D2, {0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C}};
//EFI_GUID AppleDevicePropertyProtocolGuid        = {0x91BD12FE, 0xF6C3, 0x44FB, {0xA5, 0xB7, 0x51, 0x22, 0xAB, 0x30, 0x3A, 0xE0}};
//EFI_GUID gEfiAppleBootGuid                      = {0x7C436110, 0xAB2A, 0x4BBB, {0xA8, 0x80, 0xFE, 0x41, 0x99, 0x5C, 0x9F, 0x82}};
//EFI_GUID gEfiAppleNvramGuid                     = {0x4D1EDE05, 0x38C7, 0x4A6A, {0x9C, 0xC6, 0x4B, 0xCC, 0xA8, 0xB3, 0x8C, 0x14}};
//EFI_GUID gEfiAppleScreenInfoGuid                = {0xE316E100, 0x0751, 0x4C49, {0x90, 0x56, 0x48, 0x6C, 0x7E, 0x47, 0x29, 0x03}};
//EFI_GUID AppleBootKeyPressProtocolGuid          = {0x5B213447, 0x6E73, 0x4901, {0xA4, 0xF1, 0xB8, 0x64, 0xF3, 0xB7, 0xA1, 0x72}};
//EFI_GUID AppleNetBootProtocolGuid               = {0x78EE99FB, 0x6A5E, 0x4186, {0x97, 0xDE, 0xCD, 0x0A, 0xBA, 0x34, 0x5A, 0x74}};
//EFI_GUID AppleImageCodecProtocolGuid            = {0x0DFCE9F6, 0xC4E3, 0x45EE, {0xA0, 0x6A, 0xA8, 0x61, 0x3B, 0x98, 0xA5, 0x07}};
//EFI_GUID gEfiAppleVendorGuid                    = {0xAC39C713, 0x7E50, 0x423D, {0x88, 0x9D, 0x27, 0x8F, 0xCC, 0x34, 0x22, 0xB6}};
//EFI_GUID gAppleEFINVRAMTRBSecureGuid            = {0xF68DA75E, 0x1B55, 0x4E70, {0xB4, 0x1B, 0xA7, 0xB7, 0xA5, 0xB7, 0x58, 0xEA}};
//EFI_GUID gDataHubOptionsGuid                    = {0x0021001C, 0x3CE3, 0x41F8, {0x99, 0xC6, 0xEC, 0xF5, 0xDA, 0x75, 0x47, 0x31}};
EFI_GUID gNotifyMouseActivity                   = {0xF913C2C2, 0x5351, 0x4FDB, {0x93, 0x44, 0x70, 0xFF, 0xED, 0xB8, 0x42, 0x25}};
EFI_GUID gEfiDataHubProtocolGuid                = {0xae80d021, 0x618e, 0x11d4, {0xbc, 0xd7, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81}};
EFI_GUID gEfiMiscSubClassGuid                   = {0x772484B2, 0x7482, 0x4b91, {0x9F, 0x9A, 0xAD, 0x43, 0xF8, 0x1C, 0x58, 0x81}};
EFI_GUID gEfiProcessorSubClassGuid              = {0x26fdeb7e, 0xb8af, 0x4ccf, {0xaa, 0x97, 0x02, 0x63, 0x3c, 0xe4, 0x8c, 0xa7}};
EFI_GUID gEfiMemorySubClassGuid                 = {0x4E8F4EBB, 0x64B9, 0x4e05, {0x9B, 0x18, 0x4C, 0xFE, 0x49, 0x23, 0x50, 0x97}};
//EFI_GUID gMsgLogProtocolGuid                    = {0x511CE018, 0x0018, 0x4002, {0x20, 0x12, 0x17, 0x38, 0x05, 0x01, 0x02, 0x03}};
//EFI_GUID gEfiLegacy8259ProtocolGuid             = {0x38321dba, 0x4fe0, 0x4e17, {0x8a, 0xec, 0x41, 0x30, 0x55, 0xea, 0xed, 0xc1}};

             
//all these codes are still under the question
EFI_GUID GPT_MSDOS_PARTITION = \
{ 0xEBD0A0A2, 0xB9E5, 0x4433,{ 0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7 }};  //ExFAT
// HFS+ partition - 48465300-0000-11AA-AA11-00306543ECAC
EFI_GUID GPT_HFSPLUS_PARTITION = \
{ 0x48465300, 0x0000, 0x11AA, {0xAA, 0x11, 0x00, 0x30, 0x65, 0x43, 0xEC, 0xAC }};
EFI_GUID GPT_EMPTY_PARTITION = \
{ 0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
// turbo - Apple Boot Partition - 426F6F74-0000-11AA-AA11-00306543ECAC  //RecoveryHD
// Microsoft Reserved Partition - E3C9E316-0B5C-4DB8-817DF92DF00215AE
//EFI_PART_TYPE_LEGACY_MBR_GUID {0x024DEE41, 0x33E7, 0x11D3, {0x9D, 0x69, 0x00, 0x08, 0xC7, 0x81, 0xF3, 0x9F }};

//TODO - discover the follow guids
//gBS->LocateProtocol(8ECE08D8-A6D4-430B-A7B0-2DF318E7884A)
//efi/configuration-table/5751DA6E-1376-4E02-BA92-D294FDD30901
//efi/configuration-table/F76761DC-FF89-44E4-9C0C-CD0ADA4EF983
/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * string_to_uuid() creates a 128-bit uuid from a well-formatted UUID string
 * (i.e. aabbccdd-eeff-gghh-iijj-kkllmmnnoopp)
 */
#if 0

VOID
string_to_uuid(
			   CHAR8 *string,
			   UINT8 *uuid)
{
    UINT8 count;
	
	/*
	 * scanned bytewise to ensure correct endianness of fields
	 */
	count = sscanf (string, UUID_FORMAT_STRING,
					&uuid[3], &uuid[2], &uuid[1], &uuid[0],
					&uuid[5], &uuid[4],
					&uuid[7], &uuid[6],
					&uuid[8], &uuid[9], &uuid[10], &uuid[11],
					&uuid[12], &uuid[13], &uuid[14], &uuid[15]);
	
	if (count != 16) {
	    fatal ("invalid UUID specified for -u option");
	}
}
#endif
/**
 Copyed from DevicePathFromText.c 
 Copyright (c) 2006 - 2011, Intel Corporation. All rights reserved.<BR>
 
 Converts a string to GUID value.
 Guid Format is xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 
 @param Str              The registry format GUID string that contains the GUID value.
 @param Guid             A pointer to the converted GUID value.
 
 @retval EFI_SUCCESS     The GUID string was successfully converted to the GUID value.
 @retval EFI_UNSUPPORTED The input string is not in registry format.
 @return others          Some error occurred when converting part of GUID value.
 
 **/
//#define IS_HYPHEN(a)               ((a) == L'-')
//#define IS_NULL(a)                 ((a) == L'\0')
/**
 Converts a list of string to a specified buffer.
 
 @param Buf             The output buffer that contains the string.
 @param BufferLength    The length of the buffer
 @param Str             The input string that contains the hex number
 
 @retval EFI_SUCCESS    The string was successfully converted to the buffer.
 
 **/
EFI_STATUS
StrToBuf (
          OUT UINT8    *Buf,
          IN  UINTN    BufferLength,
          IN  CHAR16   *Str
          )
{
  UINTN       Index;
  UINTN       StrLength;
  UINT8       Digit;
  UINT8       Byte;
  
  Digit = 0;
  
  //
  // Two hex char make up one byte
  //
  StrLength = BufferLength * sizeof (CHAR16);
  
  for(Index = 0; Index < StrLength; Index++, Str++) {
    
    if ((*Str >= L'a') && (*Str <= L'f')) {
      Digit = (UINT8) (*Str - L'a' + 0x0A);
    } else if ((*Str >= L'A') && (*Str <= L'F')) {
      Digit = (UINT8) (*Str - L'A' + 0x0A);
    } else if ((*Str >= L'0') && (*Str <= L'9')) {
      Digit = (UINT8) (*Str - L'0');
    } else {
      return EFI_INVALID_PARAMETER;
    }
    
    //
    // For odd characters, write the upper nibble for each buffer byte,
    // and for even characters, the lower nibble.
    //
    if ((Index & 1) == 0) {
      Byte = (UINT8) (Digit << 4);
    } else {
      Byte = Buf[Index / 2];
      Byte &= 0xF0;
      Byte = (UINT8) (Byte | Digit);
    }
    
    Buf[Index / 2] = Byte;
  }
  
  return EFI_SUCCESS;
}

/**
 Converts a string to GUID value.
 Guid Format is xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 
 @param Str              The registry format GUID string that contains the GUID value.
 @param Guid             A pointer to the converted GUID value.
 
 @retval EFI_SUCCESS     The GUID string was successfully converted to the GUID value.
 @retval EFI_UNSUPPORTED The input string is not in registry format.
 @return others          Some error occurred when converting part of GUID value.
 
 **/

EFI_STATUS
StrToGuidLE (
           IN  CHAR16   *Str,
           OUT EFI_GUID *Guid
           )
{
  UINT8 GuidLE[16];
  StrToBuf (&GuidLE[0], 4, Str);
	while (!IS_HYPHEN (*Str) && !IS_NULL (*Str)) {
		Str ++;
	}
	
	if (IS_HYPHEN (*Str)) {
		Str++;
	} else {
		return EFI_UNSUPPORTED;
	}
	
  StrToBuf (&GuidLE[4], 2, Str);
	while (!IS_HYPHEN (*Str) && !IS_NULL (*Str)) {
		Str ++;
	}
	
	if (IS_HYPHEN (*Str)) {
		Str++;
	} else {
		return EFI_UNSUPPORTED;
	}
	
  StrToBuf (&GuidLE[6], 2, Str);
	while (!IS_HYPHEN (*Str) && !IS_NULL (*Str)) {
		Str ++;
	}
	
	if (IS_HYPHEN (*Str)) {
		Str++;
	} else {
		return EFI_UNSUPPORTED;
	}

  StrToBuf (&GuidLE[8], 2, Str);
	while (!IS_HYPHEN (*Str) && !IS_NULL (*Str)) {
		Str ++;
	}
	
	if (IS_HYPHEN (*Str)) {
		Str++;
	} else {
		return EFI_UNSUPPORTED;
	}
  
  StrToBuf (&GuidLE[10], 6, Str);

  CopyMem((UINT8*)Guid, &GuidLE[0], 16);
  return EFI_SUCCESS;
}

EFI_STATUS
StrToGuid (
		   IN  CHAR16   *Str,
		   OUT EFI_GUID *Guid
		   )
{
	//
	// Get the first UINT32 data
	//
	Guid->Data1 = (UINT32) StrHexToUint64  (Str);
	while (!IS_HYPHEN (*Str) && !IS_NULL (*Str)) {
		Str ++;
	}
	
	if (IS_HYPHEN (*Str)) {
		Str++;
	} else {
		return EFI_UNSUPPORTED;
	}
	
	//
	// Get the second UINT16 data
	//
	Guid->Data2 = (UINT16) StrHexToUint64  (Str);
	while (!IS_HYPHEN (*Str) && !IS_NULL (*Str)) {
		Str ++;
	}
	
	if (IS_HYPHEN (*Str)) {
		Str++;
	} else {
		return EFI_UNSUPPORTED;
	}
	
	//
	// Get the third UINT16 data
	//
	Guid->Data3 = (UINT16) StrHexToUint64  (Str);
	while (!IS_HYPHEN (*Str) && !IS_NULL (*Str)) {
		Str ++;
	}
	
	if (IS_HYPHEN (*Str)) {
		Str++;
	} else {
		return EFI_UNSUPPORTED;
	}
	
	//
	// Get the following 8 bytes data
	//  
	StrToBuf (&Guid->Data4[0], 2, Str);
	//
	// Skip 2 byte hex chars
	//
	Str += 2 * 2;
	
	if (IS_HYPHEN (*Str)) {
		Str++;
	} else {
		return EFI_UNSUPPORTED;
	}
	StrToBuf (&Guid->Data4[2], 6, Str);
	
	return EFI_SUCCESS;
}



#if TEST
enum E { x = -1, y = 2, z = 10000 };
void f9(__builtin_va_list args)
{
  (void)__builtin_va_arg(args, float); // expected-warning {{second argument to 'va_arg' is of promotable type 'float'}}
  (void)__builtin_va_arg(args, enum E); // Don't warn here in C
  (void)__builtin_va_arg(args, short); // expected-warning {{second argument to 'va_arg' is of promotable type 'short'}}
  (void)__builtin_va_arg(args, char); // expected-warning {{second argument to 'va_arg' is of promotable type 'char'}}
}
#endif
