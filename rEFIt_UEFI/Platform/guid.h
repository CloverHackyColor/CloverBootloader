/*
 * guid.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_GUID_H_
#define PLATFORM_GUID_H_

#include "Utils.h"
#include "../cpp_foundation/XStringArray.h"
#include "../cpp_foundation/unicode_conversions.h"

extern "C" {
#include <Uefi/UefiBaseType.h> // for EFI_GUID
}

//
//
///** Returns true is Str is ascii Guid in format xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx */
//template <typename T, typename IntegralType, enable_if( is_char(T) && is_integral(IntegralType) ) >
//XBool IsValidGuidString(const T* Str, IntegralType n)
//{
//  UINTN   Index4IsValidGuidAsciiString; // stupid name to avoid warning : Declaration shadows a variable in the global namespace
//
//  if ( n != 36 ) {
//    return false;
//  }
//
//  for (Index4IsValidGuidAsciiString = 0; Index4IsValidGuidAsciiString < 36; Index4IsValidGuidAsciiString++) {
//    if (Index4IsValidGuidAsciiString == 8 || Index4IsValidGuidAsciiString == 13 || Index4IsValidGuidAsciiString == 18 || Index4IsValidGuidAsciiString == 23) {
//      if (Str[Index4IsValidGuidAsciiString] != '-') {
//        return false;
//      }
//    } else {
//      if (!(
//            (Str[Index4IsValidGuidAsciiString] >= '0' && Str[Index4IsValidGuidAsciiString] <= '9')
//            || (Str[Index4IsValidGuidAsciiString] >= 'a' && Str[Index4IsValidGuidAsciiString] <= 'f')
//            || (Str[Index4IsValidGuidAsciiString] >= 'A' && Str[Index4IsValidGuidAsciiString] <= 'F')
//            )
//          )
//      {
//        return false;
//      }
//    }
//  }
//
//  return true;
//}
//
///** Returns true is Str is ascii Guid in format xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx */
//template <typename T, enable_if( is___String(T) )>
//XBool IsValidGuidString(const T& Str)
//{
//  if ( Str.isEmpty() ) return false;
//  return IsValidGuidString(Str.data(), Str.length());
//}

///** Returns true is Str is ascii Guid in format xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx */
//template <typename T, enable_if( is___String(T) )>
//XBool IsValidGuidAsciiString(const T& Str)
//{
//  UINTN   Index4IsValidGuidAsciiString; // stupid name to avoid warning : Declaration shadows a variable in the global namespace
//
//  if ( Str.length() != 36 ) {
//    return false;
//  }
//
//  for (Index4IsValidGuidAsciiString = 0; Index4IsValidGuidAsciiString < 36; Index4IsValidGuidAsciiString++) {
//    if (Index4IsValidGuidAsciiString == 8 || Index4IsValidGuidAsciiString == 13 || Index4IsValidGuidAsciiString == 18 || Index4IsValidGuidAsciiString == 23) {
//      if (Str[Index4IsValidGuidAsciiString] != '-') {
//        return false;
//      }
//    } else {
//      if (!(
//            (Str[Index4IsValidGuidAsciiString] >= '0' && Str[Index4IsValidGuidAsciiString] <= '9')
//            || (Str[Index4IsValidGuidAsciiString] >= 'a' && Str[Index4IsValidGuidAsciiString] <= 'f')
//            || (Str[Index4IsValidGuidAsciiString] >= 'A' && Str[Index4IsValidGuidAsciiString] <= 'F')
//            )
//          )
//      {
//        return false;
//      }
//    }
//  }
//
//  return true;
//}

//
//template <typename T, enable_if( is_char_ptr(T)  ||  is___String(T) )>
//EFI_STATUS
//StrHToBuf (
//          OUT UINT8    *Buf,
//          IN  UINTN    BufferLength,
//          const T& t
//          )
//{
//  UINTN       Index4IsValidGuidAsciiString; // stupid name to avoid warning : Declaration shadows a variable in the global namespace
//  UINTN       StrLength;
//  UINT8       Digit;
//  UINT8       Byte;
//
//  Digit = 0;
//  auto Str = _xstringarray__char_type<T>::getCharPtr(t);
//
//  //
//  // Two hex char make up one byte
//  //
//  StrLength = BufferLength * sizeof (CHAR16);
//
//  for(Index4IsValidGuidAsciiString = 0; Index4IsValidGuidAsciiString < StrLength; Index4IsValidGuidAsciiString++) {
//
//    if ((Str[Index4IsValidGuidAsciiString] >= (decltype(*Str))'a') && (Str[Index4IsValidGuidAsciiString] <= (decltype(*Str))'f')) {
//      Digit = (UINT8) (Str[Index4IsValidGuidAsciiString] - (decltype(*Str))'a' + 0x0A);
//    } else if ((Str[Index4IsValidGuidAsciiString] >= (decltype(*Str))'A') && (Str[Index4IsValidGuidAsciiString] <= (decltype(*Str))'F')) {
//      Digit = (UINT8) (Str[Index4IsValidGuidAsciiString] - L'A' + 0x0A);
//    } else if ((Str[Index4IsValidGuidAsciiString] >= (decltype(*Str))'0') && (Str[Index4IsValidGuidAsciiString] <= (decltype(*Str))'9')) {
//      Digit = (UINT8) (Str[Index4IsValidGuidAsciiString] - (decltype(*Str))'0');
//    } else {
//      return EFI_INVALID_PARAMETER;
//    }
//
//    //
//    // For odd characters, write the upper nibble for each buffer byte,
//    // and for even characters, the lower nibble.
//    //
//    if ((Index4IsValidGuidAsciiString & 1) == 0) {
//      Byte = (UINT8) (Digit << 4);
//    } else {
//      Byte = Buf[Index4IsValidGuidAsciiString / 2];
//      Byte &= 0xF0;
//      Byte = (UINT8) (Byte | Digit);
//    }
//
//    Buf[Index4IsValidGuidAsciiString / 2] = Byte;
//  }
//
//  return EFI_SUCCESS;
//}

///*
// * Convert a string to a EFI_GUID.
// * First parameter can by of one of these type : char*, char16_t*, char32_t*, wchar_t*, XString8, XString16, XString32, XStringW
// */
//template <typename T, enable_if( is_char_ptr(T)  ||  is___String(T) )>
//EFI_STATUS
//StrToGuidBE (
//           const T& t,
//           OUT EFI_GUID *Guid
//           )
//{
//  EFI_STATUS Status;
//  UINT8 GuidLE[16];
//
//  if ( Guid == NULL ) {
//    log_technical_bug("%s : call with Guid==NULL", __PRETTY_FUNCTION__);
//    return EFI_UNSUPPORTED;
//  }
//
//  auto Str = _xstringarray__char_type<T>::getCharPtr(t);
//
//  if ( Str == NULL ) {
//    memset(Guid, 0, sizeof(EFI_GUID));
//    return EFI_SUCCESS;
//  }
//
//
//  Status = StrHToBuf (&GuidLE[0], 4, Str);
//  if ( Status != EFI_SUCCESS ) return Status;
//
//  while (!IS_HYPHEN (*Str) && !IS_NULL (*Str)) {
//    Str ++;
//  }
//
//  if (IS_HYPHEN (*Str)) {
//    Str++;
//  } else {
//    return EFI_UNSUPPORTED;
//  }
//
//  Status = StrHToBuf (&GuidLE[4], 2, Str);
//  if ( Status != EFI_SUCCESS ) return Status;
//  while (!IS_HYPHEN (*Str) && !IS_NULL (*Str)) {
//    Str ++;
//  }
//
//  if (IS_HYPHEN (*Str)) {
//    Str++;
//  } else {
//    return EFI_UNSUPPORTED;
//  }
//
//  Status = StrHToBuf (&GuidLE[6], 2, Str);
//  if ( Status != EFI_SUCCESS ) return Status;
//  while (!IS_HYPHEN (*Str) && !IS_NULL (*Str)) {
//    Str ++;
//  }
//
//  if (IS_HYPHEN (*Str)) {
//    Str++;
//  } else {
//    return EFI_UNSUPPORTED;
//  }
//
//  Status = StrHToBuf (&GuidLE[8], 2, Str);
//  if ( Status != EFI_SUCCESS ) return Status;
//  while (!IS_HYPHEN (*Str) && !IS_NULL (*Str)) {
//    Str ++;
//  }
//
//  if (IS_HYPHEN (*Str)) {
//    Str++;
//  } else {
//    return EFI_UNSUPPORTED;
//  }
//
//  Status = StrHToBuf (&GuidLE[10], 6, Str);
//  if ( Status != EFI_SUCCESS ) return Status;
//
//  if (Guid) {
//    memcpy((UINT8*)Guid, &GuidLE[0], sizeof(EFI_GUID));
//  }
//
//  return EFI_SUCCESS;
//}


//XString8 GuidBeToXString8(const EFI_GUID& Guid); // not used
//XStringW GuidBeToXStringW(const EFI_GUID& Guid); // not used
//XString8 GuidLEToXString8(const EFI_GUID& Guid);
//XStringW GuidLEToXStringW(const EFI_GUID& Guid);







//this is standard
//constexpr const EFI_GUID gEfiSmbios3TableGuid = { 0xF2FD1544, 0x9794, 0x4A2C, { 0x99, 0x2E, 0xE5, 0xBB, 0xCF, 0x20, 0xE3, 0x94 }};

//constexpr const EFI_GUID gEfiConsoleControlProtocolGuid         = {0xF42F7782, 0x012E, 0x4C12, {0x99, 0x56, 0x49, 0xF9, 0x43, 0x04, 0xF7, 0x21}};
//constexpr const EFI_GUID gAppleFirmwarePasswordProtocolGuid     = {0x8FFEEB3A, 0x4C98, 0x4630, {0x80, 0x3F, 0x74, 0x0F, 0x95, 0x67, 0x09, 0x1D}};
//gEfiGlobalVariableGuid
//constexpr const EFI_GUID gEfiGlobalVariableGuid                      = {0x8BE4DF61, 0x93CA, 0x11D2, {0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C}};
//constexpr const EFI_GUID gEfiDevicePathPropertyDatabaseProtocolGuid       = {0x91BD12FE, 0xF6C3, 0x44FB, {0xA5, 0xB7, 0x51, 0x22, 0xAB, 0x30, 0x3A, 0xE0}};
//constexpr const EFI_GUID gEfi AppleBootGuid                      = {0x7C436110, 0xAB2A, 0x4BBB, {0xA8, 0x80, 0xFE, 0x41, 0x99, 0x5C, 0x9F, 0x82}}; //gAppleBootVariableGuid
//constexpr const EFI_GUID gEfiAppleNvramGuid                     = {0x4D1EDE05, 0x38C7, 0x4A6A, {0x9C, 0xC6, 0x4B, 0xCC, 0xA8, 0xB3, 0x8C, 0x14}}; //gAppleVendorVariableGuid
//constexpr const EFI_GUID gAppleFramebufferInfoProtocolGuid      = {0xE316E100, 0x0751, 0x4C49, {0x90, 0x56, 0x48, 0x6C, 0x7E, 0x47, 0x29, 0x03}};
//constexpr const EFI_GUID gAppleKeyStateProtocolGuid             = {0x5B213447, 0x6E73, 0x4901, {0xA4, 0xF1, 0xB8, 0x64, 0xF3, 0xB7, 0xA1, 0x72}};
//constexpr const EFI_GUID gAppleNetBootProtocolGuid              = {0x78EE99FB, 0x6A5E, 0x4186, {0x97, 0xDE, 0xCD, 0x0A, 0xBA, 0x34, 0x5A, 0x74}};
//constexpr const EFI_GUID gAppleImageCodecProtocolGuid           = {0x0DFCE9F6, 0xC4E3, 0x45EE, {0xA0, 0x6A, 0xA8, 0x61, 0x3B, 0x98, 0xA5, 0x07}};
//constexpr const EFI_GUID gEfiAppleVendorGuid                    = {0xAC39C713, 0x7E50, 0x423D, {0x88, 0x9D, 0x27, 0x8F, 0xCC, 0x34, 0x22, 0xB6}};
//constexpr const EFI_GUID gAppleEFINVRAMTRBSecureGuid            = {0xF68DA75E, 0x1B55, 0x4E70, {0xB4, 0x1B, 0xA7, 0xB7, 0xA5, 0xB7, 0x58, 0xEA}};
//constexpr const EFI_GUID gDataHubOptionsGuid                    = {0x0021001C, 0x3CE3, 0x41F8, {0x99, 0xC6, 0xEC, 0xF5, 0xDA, 0x75, 0x47, 0x31}};
constexpr const EFI_GUID gNotifyMouseActivity                   = {0xF913C2C2, 0x5351, 0x4FDB, {0x93, 0x44, 0x70, 0xFF, 0xED, 0xB8, 0x42, 0x25}};
//constexpr const EFI_GUID gEfiDataHubProtocolGuid                = {0xAE80D021, 0x618E, 0x11D4, {0xBC, 0xD7, 0x00, 0x80, 0xC7, 0x3C, 0x88, 0x81}};
//constexpr const EFI_GUID gEfiMiscSubClassGuid                   = {0x772484B2, 0x7482, 0x4b91, {0x9F, 0x9A, 0xAD, 0x43, 0xF8, 0x1C, 0x58, 0x81}};
//constexpr const EFI_GUID gEfiProcessorSubClassGuid              = {0x26FDEB7E, 0xB8AF, 0x4CCF, {0xAA, 0x97, 0x02, 0x63, 0x3C, 0xE4, 0x8C, 0xA7}};
//constexpr const EFI_GUID gEfiMemorySubClassGuid                 = {0x4E8F4EBB, 0x64B9, 0x4e05, {0x9B, 0x18, 0x4C, 0xFE, 0x49, 0x23, 0x50, 0x97}};
//constexpr const EFI_GUID gMsgLogProtocolGuid                    = {0x511CE018, 0x0018, 0x4002, {0x20, 0x12, 0x17, 0x38, 0x05, 0x01, 0x02, 0x03}};
//constexpr const EFI_GUID gEfiLegacy8259ProtocolGuid             = {0x38321DBA, 0x4FE0, 0x4E17, {0x8A, 0xEC, 0x41, 0x30, 0x55, 0xEA, 0xED, 0xC1}};
constexpr const EFI_GUID gAppleDeviceControlProtocolGuid  = {0x8ECE08D8, 0xA6D4, 0x430B, {0xA7, 0xB0, 0x2D, 0xF3, 0x18, 0xE7, 0x88, 0x4A}};

//from reza jelveh
constexpr const EFI_GUID gAppleSystemInfoProducerNameGuid = {0x64517CC8, 0x6561, 0x4051, {0xB0, 0x3C, 0x59, 0x64, 0xB6, 0x0F, 0x4C, 0x7A}};
constexpr const EFI_GUID gAppleFsbFrequencyPropertyGuid   = {0xD1A04D55, 0x75B9, 0x41A3, {0x90, 0x36, 0x8F, 0x4A, 0x26, 0x1C, 0xBB, 0xA2}};
constexpr const EFI_GUID gAppleDevicePathsSupportedGuid   = {0x5BB91CF7, 0xD816, 0x404B, {0x86, 0x72, 0x68, 0xF2, 0x7F, 0x78, 0x31, 0xDC}};
//constexpr const EFI_GUID gAppleSMCProtocolGuid            = {0x17407e5a, 0xaf6c, 0x4ee8, {0x98, 0xa8, 0x00, 0x21, 0x04, 0x53, 0xcd, 0xd9}};

constexpr const EFI_GUID gAppleCursorImageGuid            = {0x1A10742F, 0xFA80, 0x4B79, {0x9D, 0xA6, 0x35, 0x70, 0x58, 0xCC, 0x39, 0x7B}};

//all these codes are still under the question
/*
constexpr const EFI_GUID GPT_MSDOS_PARTITION = \
{ 0xEBD0A0A2, 0xB9E5, 0x4433,{ 0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7 }};  //ExFAT
// HFS+ partition - 48465300-0000-11AA-AA11-00306543ECAC
constexpr const EFI_GUID GPT_HFSPLUS_PARTITION = \
{ 0x48465300, 0x0000, 0x11AA, {0xAA, 0x11, 0x00, 0x30, 0x65, 0x43, 0xEC, 0xAC }};
constexpr const EFI_GUID GPT_EMPTY_PARTITION = \
{ 0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
// turbo - Apple Boot Partition - 426F6F74-0000-11AA-AA11-00306543ECAC  //RecoveryHD
// Microsoft Reserved Partition - E3C9E316-0B5C-4DB8-817DF92DF00215AE
EFI_PART_TYPE_LEGACY_MBR_GUID {0x024DEE41, 0x33E7, 0x11D3, {0x9D, 0x69, 0x00, 0x08, 0xC7, 0x81, 0xF3, 0x9F }};
*/
//constexpr const EFI_GUID ESPGuid = { 0xc12a7328, 0xf81f, 0x11d2, { 0xba, 0x4b, 0x00, 0xa0, 0xc9, 0x3e, 0xc9, 0x3b } };

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
// 24B73556-2197-4702-82A8-3E1337DAFBF2 Apple Secure Boot enabled configuration
// 24B73556-2197-4702-82A8-3E1337DAFBF3 Apple Trusted Boot enabled configuration
// 1BAD711C-D451-4241-B1F3-8537812E0C70 EFI_GUID for MeBiosExtensionSetup variable
// 36C28AB5-6566-4C50-9EBD-CBB920F83843:preferred-networks gAppleWirelessNetworkVariableGuid
// ->SetVariable(boot-feature-usage, 62BF9B1C-8568-48EE-85DC-DD3057660863, 7, 8, 4C4ABBE8) = Success
// 00 00 08 00 00 00 00 00                         | ........
// gAppleFpfConfigurationHobGuid  = { 0xE3CC8EC6, 0x81C1, 0x4271, { 0xAC, 0xBC, 0xDB, 0x65, 0x08, 0x6E, 0x8D, 0xC8 }}
// 59D76AE4-37E3-55A7-B460-EF13D46E6020 AppleEncryptedPartitionProtocolGuid

/*
#define APPLE_SECURE_BOOT_VARIABLE_GUID  \
{ 0x94B73556, 0x2197, 0x4702,          \
  { 0x82, 0xA8, 0x3E, 0x13, 0x37, 0xDA, 0xFB, 0xFB } }

->SetVariable(ApECID, 94B73556-2197-4702-82A8-3E1337DAFBFB, 6, 8, 4C4ABC90) = Success
1C 02 1B 03 0D 04 66 05                         | ......f.
->SetVariable(ApChipID, 94B73556-2197-4702-82A8-3E1337DAFBFB, 6, 4, 4C4ABCB4) = Success
12 80 00 00                                     | ....
->SetVariable(ApBoardID, 94B73556-2197-4702-82A8-3E1337DAFBFB, 6, 4, 4C4ABCB0) = Success
F0 00 00 00                                     | ....
->SetVariable(ApSecurityDomain, 94B73556-2197-4702-82A8-3E1337DAFBFB, 6, 4, 4C4ABCB8) = Success
01 00 00 00                                     | ....
->SetVariable(ApProductionStatus, 94B73556-2197-4702-82A8-3E1337DAFBFB, 6, 1, 4C4ABCBD) = Success
01                                              | .
->SetVariable(ApSecurityMode, 94B73556-2197-4702-82A8-3E1337DAFBFB, 6, 1, 4C4ABCBD) = Success
01                                              | .
->SetVariable(EffectiveProductionStatus, 94B73556-2197-4702-82A8-3E1337DAFBFB, 6, 1, 4C4ABCBD) = Success
01                                              | .
->SetVariable(EffectiveSecurityMode, 94B73556-2197-4702-82A8-3E1337DAFBFB, 6, 1, 4C4ABCBD) = Success
01                                              | .
->SetVariable(CertificateEpoch, 94B73556-2197-4702-82A8-3E1337DAFBFB, 6, 1, 4C4ABCBF) = Success
02                                              | .
->SetVariable(MixNMatchPreventionStatus, 94B73556-2197-4702-82A8-3E1337DAFBFB, 6, 1, 4C4ABCBE) = Success
00                                              | .
->SetVariable(CryptoDigestMethod, 94B73556-2197-4702-82A8-3E1337DAFBFB, 6, 10, 4C4ABC70) = Success
73 68 61 32 2D 33 38 34 00 00 00 00 00 00 00 00 | sha2-384........
->SetVariable(HardwareModel, 94B73556-2197-4702-82A8-3E1337DAFBFB, 6, 10, 4C4ABC60) = Success
78 38 36 6C 65 67 61 63 79 61 70 00 00 00 00 00 | x86legacyap.....
->SetVariable(InternalUseOnlyUnit, 94B73556-2197-4702-82A8-3E1337DAFBFB, 6, 1, 4C4ABCBD) = Success
01                                              | .
*/




#endif /* PLATFORM_GUID_H_ */
