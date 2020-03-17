/**
 guid.c
 **/

#include "Platform.h"

//this is standard
//gEfiSmbios3TableGuid = { 0xF2FD1544, 0x9794, 0x4A2C, { 0x99, 0x2E, 0xE5, 0xBB, 0xCF, 0x20, 0xE3, 0x94 }};

//EFI_GUID gEfiConsoleControlProtocolGuid         = {0xF42F7782, 0x012E, 0x4C12, {0x99, 0x56, 0x49, 0xF9, 0x43, 0x04, 0xF7, 0x21}};
//EFI_GUID gAppleFirmwarePasswordProtocolGuid     = {0x8FFEEB3A, 0x4C98, 0x4630, {0x80, 0x3F, 0x74, 0x0F, 0x95, 0x67, 0x09, 0x1D}};
//gEfiGlobalVariableGuid
//EFI_GUID gEfiGlobalVarGuid                      = {0x8BE4DF61, 0x93CA, 0x11D2, {0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C}};
//EFI_GUID gAppleDevicePropertyProtocolGuid       = {0x91BD12FE, 0xF6C3, 0x44FB, {0xA5, 0xB7, 0x51, 0x22, 0xAB, 0x30, 0x3A, 0xE0}};
//EFI_GUID gEfiAppleBootGuid                      = {0x7C436110, 0xAB2A, 0x4BBB, {0xA8, 0x80, 0xFE, 0x41, 0x99, 0x5C, 0x9F, 0x82}};
//EFI_GUID gEfiAppleNvramGuid                     = {0x4D1EDE05, 0x38C7, 0x4A6A, {0x9C, 0xC6, 0x4B, 0xCC, 0xA8, 0xB3, 0x8C, 0x14}};
//EFI_GUID gAppleFramebufferInfoProtocolGuid      = {0xE316E100, 0x0751, 0x4C49, {0x90, 0x56, 0x48, 0x6C, 0x7E, 0x47, 0x29, 0x03}};
//EFI_GUID gAppleKeyStateProtocolGuid             = {0x5B213447, 0x6E73, 0x4901, {0xA4, 0xF1, 0xB8, 0x64, 0xF3, 0xB7, 0xA1, 0x72}};
//EFI_GUID gAppleNetBootProtocolGuid              = {0x78EE99FB, 0x6A5E, 0x4186, {0x97, 0xDE, 0xCD, 0x0A, 0xBA, 0x34, 0x5A, 0x74}};
//EFI_GUID gAppleImageCodecProtocolGuid           = {0x0DFCE9F6, 0xC4E3, 0x45EE, {0xA0, 0x6A, 0xA8, 0x61, 0x3B, 0x98, 0xA5, 0x07}};
//EFI_GUID gEfiAppleVendorGuid                    = {0xAC39C713, 0x7E50, 0x423D, {0x88, 0x9D, 0x27, 0x8F, 0xCC, 0x34, 0x22, 0xB6}};
//EFI_GUID gAppleEFINVRAMTRBSecureGuid            = {0xF68DA75E, 0x1B55, 0x4E70, {0xB4, 0x1B, 0xA7, 0xB7, 0xA5, 0xB7, 0x58, 0xEA}};
//EFI_GUID gDataHubOptionsGuid                    = {0x0021001C, 0x3CE3, 0x41F8, {0x99, 0xC6, 0xEC, 0xF5, 0xDA, 0x75, 0x47, 0x31}};
EFI_GUID gNotifyMouseActivity                   = {0xF913C2C2, 0x5351, 0x4FDB, {0x93, 0x44, 0x70, 0xFF, 0xED, 0xB8, 0x42, 0x25}};
EFI_GUID gEfiDataHubProtocolGuid                = {0xAE80D021, 0x618E, 0x11D4, {0xBC, 0xD7, 0x00, 0x80, 0xC7, 0x3C, 0x88, 0x81}};
EFI_GUID gEfiMiscSubClassGuid                   = {0x772484B2, 0x7482, 0x4b91, {0x9F, 0x9A, 0xAD, 0x43, 0xF8, 0x1C, 0x58, 0x81}};
EFI_GUID gEfiProcessorSubClassGuid              = {0x26FDEB7E, 0xB8AF, 0x4CCF, {0xAA, 0x97, 0x02, 0x63, 0x3C, 0xE4, 0x8C, 0xA7}};
EFI_GUID gEfiMemorySubClassGuid                 = {0x4E8F4EBB, 0x64B9, 0x4e05, {0x9B, 0x18, 0x4C, 0xFE, 0x49, 0x23, 0x50, 0x97}};
//EFI_GUID gMsgLogProtocolGuid                    = {0x511CE018, 0x0018, 0x4002, {0x20, 0x12, 0x17, 0x38, 0x05, 0x01, 0x02, 0x03}};
//EFI_GUID gEfiLegacy8259ProtocolGuid             = {0x38321DBA, 0x4FE0, 0x4E17, {0x8A, 0xEC, 0x41, 0x30, 0x55, 0xEA, 0xED, 0xC1}};
EFI_GUID gAppleDeviceControlProtocolGuid  = {0x8ECE08D8, 0xA6D4, 0x430B, {0xA7, 0xB0, 0x2D, 0xF3, 0x18, 0xE7, 0x88, 0x4A}};

//from reza jelveh
EFI_GUID gAppleSystemInfoProducerNameGuid = {0x64517CC8, 0x6561, 0x4051, {0xB0, 0x3C, 0x59, 0x64, 0xB6, 0x0F, 0x4C, 0x7A}};
EFI_GUID gAppleFsbFrequencyPropertyGuid   = {0xD1A04D55, 0x75B9, 0x41A3, {0x90, 0x36, 0x8F, 0x4A, 0x26, 0x1C, 0xBB, 0xA2}};
EFI_GUID gAppleDevicePathsSupportedGuid   = {0x5BB91CF7, 0xD816, 0x404B, {0x86, 0x72, 0x68, 0xF2, 0x7F, 0x78, 0x31, 0xDC}};
EFI_GUID gAppleSMCProtocolGuid            = {0x17407e5a, 0xaf6c, 0x4ee8, {0x98, 0xa8, 0x00, 0x21, 0x04, 0x53, 0xcd, 0xd9}};

EFI_GUID gAppleCursorImageGuid            = {0x1A10742F, 0xFA80, 0x4B79, {0x9D, 0xA6, 0x35, 0x70, 0x58, 0xCC, 0x39, 0x7B}};
             
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

//EFI_GUID ESPGuid = { 0xc12a7328, 0xf81f, 0x11d2, { 0xba, 0x4b, 0x00, 0xa0, 0xc9, 0x3e, 0xc9, 0x3b } };

//TODO - discover the follow guids
//efi/configuration-table/5751DA6E-1376-4E02-BA92-D294FDD30901
//efi/configuration-table/F76761DC-FF89-44E4-9C0C-CD0ADA4EF983
// C5C5DA95-7D5C-45E6-B2F1-3FD52BB10077 - EfiOSInfo
// 03622D6D-362A-4E47-9710-C238B23755C1 - GraphConfig?
// 71B4903C-14EC-42C4-BDC6-CE1449930E49 - WiFi?
// 33BE0EF1-89C9-4A6D-BB9F-69DC8DD516B9 - AppleEvent
// D5B0AC65-9A2D-4D2A-BBD6-E871A95E0435 - UI Theme
// 816749EE-FA96-4853-BF88-2C8AE53B31C9 APPLE_EFI_LOGIN_WINDOW_ENTER_GUID
// 01AAACBA-34AC-42E3-9847-66837DAC5F5E APPLE_EFI_LOGIN_WINDOW_EXIT_GUID
// C649D4F3-D502-4DAA-A139-394ACCF2A63B DEVICE_PATH_PROPERTY_DATABASE
// 5301FE59-1770-4166-A169-00C4BDE4A162 at hibernate = gAppleSMCStateProtocolGuid
// CDEA2BD3-FC25-4C1C-B97C-B31186064990 gEfiBootLogoProtocolGuid
// 13FA7698-C831-49C7-87EA-8F43FCC25196 in ->CreateEventEx(0x200,0x10, ...
// 1C0C34F6-D380-41FA-A049-8AD06C1A66AA gEfiEdidDiscoveredProtocolGuid
// 0ADFB62D-FF74-484C-8944-F85C4BEA87A8 gAmiEfiKeycodeProtocolGuid
// 5D206DD3-516A-47DC-A1BC-6DA204AABE08 OnboardRaidGuid
// AC5E4829-A8FD-440B-AF33-9FFE013B12D8 gApplePlatformInfoDatabaseProtocolGuid
// 1FEDE521-031C-4BC5-8050-F3D6161E2E92
// BD8C1056-9F36-44EC-92A8-A6337F817986 gEfiEdidActiveProtocolGuid
// 26baccba-6f42-11d4-bce7-008081883cc7
// 63FAECF2-E7EE-4CB9-8A0C-11CE5E89E33C protocol at FinalizeBootStruct or DrawBootGraphics
// 03B99B90-ECCF-451D-809E-8341FCB830AC RestartData protocol
// 24B73556-2197-4702-82A8-3E1337DAFBF2 before Firmware password
// 24B73556-2197-4702-82A8-3E1337DAFBF3
// 1BAD711C-D451-4241-B1F3-8537812E0C70 GUID for MeBiosExtensionSetup variable
// 36C28AB5-6566-4C50-9EBD-CBB920F83843:preferred-networks gAppleWirelessNetworkVariableGuid

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

/** Returns TRUE is Str is ascii Guid in format xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx */
BOOLEAN IsValidGuidAsciiString(IN CHAR8 *Str)
{
  UINTN   Index;
  
  if (Str == NULL || AsciiStrLen(Str) != 36) {
    return FALSE;
  }
  
  for (Index = 0; Index < 36; Index++, Str++) {
    if (Index == 8 || Index == 13 || Index == 18 || Index == 23) {
      if (*Str != '-') {
        return FALSE;
      }
    } else {
      if (!(
            (*Str >= '0' && *Str <= '9')
            || (*Str >= 'a' && *Str <= 'f')
            || (*Str >= 'A' && *Str <= 'F')
            )
          )
      {
        return FALSE;
      }
    }
  }
  
  return TRUE;
}

#if 1
// it is in edk2/MdePkg/Library/UefiDevicePathLib/DevicePathFromText.c
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
static EFI_STATUS
StrHToBuf (
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
#endif
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
  StrHToBuf (&GuidLE[0], 4, Str);
	while (!IS_HYPHEN (*Str) && !IS_NULL (*Str)) {
		Str ++;
	}
	
	if (IS_HYPHEN (*Str)) {
		Str++;
	} else {
		return EFI_UNSUPPORTED;
	}
	
  StrHToBuf (&GuidLE[4], 2, Str);
	while (!IS_HYPHEN (*Str) && !IS_NULL (*Str)) {
		Str ++;
	}
	
	if (IS_HYPHEN (*Str)) {
		Str++;
	} else {
		return EFI_UNSUPPORTED;
	}
	
  StrHToBuf (&GuidLE[6], 2, Str);
	while (!IS_HYPHEN (*Str) && !IS_NULL (*Str)) {
		Str ++;
	}
	
	if (IS_HYPHEN (*Str)) {
		Str++;
	} else {
		return EFI_UNSUPPORTED;
	}

  StrHToBuf (&GuidLE[8], 2, Str);
	while (!IS_HYPHEN (*Str) && !IS_NULL (*Str)) {
		Str ++;
	}
	
	if (IS_HYPHEN (*Str)) {
		Str++;
	} else {
		return EFI_UNSUPPORTED;
	}
  
  StrHToBuf (&GuidLE[10], 6, Str);

  if (Guid) {
    CopyMem((UINT8*)Guid, &GuidLE[0], sizeof(EFI_GUID));
  }
  
  return EFI_SUCCESS;
}

//Slice - I need GuidBEToStr :(
CHAR16 * GuidBeToStr(EFI_GUID *Guid)
{
  UINT8 *GuidData = (UINT8 *)Guid;
  CHAR16 *Str = PoolPrint(L"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",                          
                          GuidData[3], GuidData[2], GuidData[1], GuidData[0],
                          GuidData[5], GuidData[4],
                          GuidData[7], GuidData[6],
                          GuidData[8], GuidData[9], GuidData[10], GuidData[11],
                          GuidData[12], GuidData[13], GuidData[14], GuidData[15]);
  return Str;
}


//the caller is responsible to free the buffer
CHAR16 * GuidLEToStr(EFI_GUID *Guid)
{
  CHAR16 *Str = PoolPrint(L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
  Guid->Data1, Guid->Data2, Guid->Data3, Guid->Data4[0], Guid->Data4[1], 
  Guid->Data4[2], Guid->Data4[3], Guid->Data4[4], Guid->Data4[5], Guid->Data4[6], Guid->Data4[7]);
  return Str;
}

#if 0
EFI_STATUS
StrToGuid2 (
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
	StrHToBuf (&Guid->Data4[0], 2, Str);
	//
	// Skip 2 byte hex chars
	//
	Str += 2 * 2;
	
	if (IS_HYPHEN (*Str)) {
		Str++;
	} else {
		return EFI_UNSUPPORTED;
	}
	StrHToBuf (&Guid->Data4[2], 6, Str);
	
	return EFI_SUCCESS;
}
#endif
