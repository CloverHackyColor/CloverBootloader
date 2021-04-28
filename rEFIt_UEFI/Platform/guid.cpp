/**
 guid.cpp
 **/

#include "guid.h"
#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile

//this is standard
//gEfiSmbios3TableGuid = { 0xF2FD1544, 0x9794, 0x4A2C, { 0x99, 0x2E, 0xE5, 0xBB, 0xCF, 0x20, 0xE3, 0x94 }};

//EFI_GUID gEfiConsoleControlProtocolGuid         = {0xF42F7782, 0x012E, 0x4C12, {0x99, 0x56, 0x49, 0xF9, 0x43, 0x04, 0xF7, 0x21}};
//EFI_GUID gAppleFirmwarePasswordProtocolGuid     = {0x8FFEEB3A, 0x4C98, 0x4630, {0x80, 0x3F, 0x74, 0x0F, 0x95, 0x67, 0x09, 0x1D}};
//gEfiGlobalVariableGuid
//EFI_GUID gEfiGlobalVarGuid                      = {0x8BE4DF61, 0x93CA, 0x11D2, {0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C}};
//EFI_GUID gEfiDevicePathPropertyDatabaseProtocolGuid       = {0x91BD12FE, 0xF6C3, 0x44FB, {0xA5, 0xB7, 0x51, 0x22, 0xAB, 0x30, 0x3A, 0xE0}};
//EFI_GUID gEfi AppleBootGuid                      = {0x7C436110, 0xAB2A, 0x4BBB, {0xA8, 0x80, 0xFE, 0x41, 0x99, 0x5C, 0x9F, 0x82}}; //gAppleBootVariableGuid
//EFI_GUID gEfiAppleNvramGuid                     = {0x4D1EDE05, 0x38C7, 0x4A6A, {0x9C, 0xC6, 0x4B, 0xCC, 0xA8, 0xB3, 0x8C, 0x14}}; //gAppleVendorVariableGuid
//EFI_GUID gAppleFramebufferInfoProtocolGuid      = {0xE316E100, 0x0751, 0x4C49, {0x90, 0x56, 0x48, 0x6C, 0x7E, 0x47, 0x29, 0x03}};
//EFI_GUID gAppleKeyStateProtocolGuid             = {0x5B213447, 0x6E73, 0x4901, {0xA4, 0xF1, 0xB8, 0x64, 0xF3, 0xB7, 0xA1, 0x72}};
//EFI_GUID gAppleNetBootProtocolGuid              = {0x78EE99FB, 0x6A5E, 0x4186, {0x97, 0xDE, 0xCD, 0x0A, 0xBA, 0x34, 0x5A, 0x74}};
//EFI_GUID gAppleImageCodecProtocolGuid           = {0x0DFCE9F6, 0xC4E3, 0x45EE, {0xA0, 0x6A, 0xA8, 0x61, 0x3B, 0x98, 0xA5, 0x07}};
//EFI_GUID gEfiAppleVendorGuid                    = {0xAC39C713, 0x7E50, 0x423D, {0x88, 0x9D, 0x27, 0x8F, 0xCC, 0x34, 0x22, 0xB6}};
//EFI_GUID gAppleEFINVRAMTRBSecureGuid            = {0xF68DA75E, 0x1B55, 0x4E70, {0xB4, 0x1B, 0xA7, 0xB7, 0xA5, 0xB7, 0x58, 0xEA}};
//EFI_GUID gDataHubOptionsGuid                    = {0x0021001C, 0x3CE3, 0x41F8, {0x99, 0xC6, 0xEC, 0xF5, 0xDA, 0x75, 0x47, 0x31}};
EFI_GUID gNotifyMouseActivity                   = {0xF913C2C2, 0x5351, 0x4FDB, {0x93, 0x44, 0x70, 0xFF, 0xED, 0xB8, 0x42, 0x25}};
//EFI_GUID gEfiDataHubProtocolGuid                = {0xAE80D021, 0x618E, 0x11D4, {0xBC, 0xD7, 0x00, 0x80, 0xC7, 0x3C, 0x88, 0x81}};
//EFI_GUID gEfiMiscSubClassGuid                   = {0x772484B2, 0x7482, 0x4b91, {0x9F, 0x9A, 0xAD, 0x43, 0xF8, 0x1C, 0x58, 0x81}};
//EFI_GUID gEfiProcessorSubClassGuid              = {0x26FDEB7E, 0xB8AF, 0x4CCF, {0xAA, 0x97, 0x02, 0x63, 0x3C, 0xE4, 0x8C, 0xA7}};
//EFI_GUID gEfiMemorySubClassGuid                 = {0x4E8F4EBB, 0x64B9, 0x4e05, {0x9B, 0x18, 0x4C, 0xFE, 0x49, 0x23, 0x50, 0x97}};
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
/*
EFI_GUID GPT_MSDOS_PARTITION = \
{ 0xEBD0A0A2, 0xB9E5, 0x4433,{ 0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7 }};  //ExFAT
// HFS+ partition - 48465300-0000-11AA-AA11-00306543ECAC
EFI_GUID GPT_HFSPLUS_PARTITION = \
{ 0x48465300, 0x0000, 0x11AA, {0xAA, 0x11, 0x00, 0x30, 0x65, 0x43, 0xEC, 0xAC }};
EFI_GUID GPT_EMPTY_PARTITION = \
{ 0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
// turbo - Apple Boot Partition - 426F6F74-0000-11AA-AA11-00306543ECAC  //RecoveryHD
// Microsoft Reserved Partition - E3C9E316-0B5C-4DB8-817DF92DF00215AE
EFI_PART_TYPE_LEGACY_MBR_GUID {0x024DEE41, 0x33E7, 0x11D3, {0x9D, 0x69, 0x00, 0x08, 0xC7, 0x81, 0xF3, 0x9F }};
*/
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
// 24B73556-2197-4702-82A8-3E1337DAFBF2 before Firmware password APPLE_SECURE_BOOT_PROTOCOL_GUID
// 24B73556-2197-4702-82A8-3E1337DAFBF3
// 1BAD711C-D451-4241-B1F3-8537812E0C70 GUID for MeBiosExtensionSetup variable
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

const XString8 nullGuidAsString = "00000000-0000-0000-0000-000000000000"_XS8;

class _GUID_H__asserts
{
public:
  _GUID_H__asserts() {
    // Jief : I know it's a panic, even in a release version. But it's about constants !
    if ( !IsValidGuidString(nullGuidAsString) ) panic("!IsValidGuidString(nullGuidAsString)");
  }
} _GUID_H__asserts_obj;


EFI_GUID nullGuid = {0,0,0,{0,0,0,0,0,0,0,0}};

XStringW GuidBeToXStringW(const EFI_GUID& Guid)
{
  UINT8 *GuidData = (UINT8 *)&Guid;
  XStringW Str = SWPrintf("%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                          GuidData[0], GuidData[1], GuidData[2], GuidData[3],
                          GuidData[4], GuidData[5],
                          GuidData[6], GuidData[7],
                          GuidData[8], GuidData[9], GuidData[10], GuidData[11],
                          GuidData[12], GuidData[13], GuidData[14], GuidData[15]);
  return Str;
}

XString8 GuidBeToXString8(const EFI_GUID& Guid)
{
  UINT8 *GuidData = (UINT8 *)&Guid;
  XString8 Str = SWPrintf("%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                          GuidData[0], GuidData[1], GuidData[2], GuidData[3],
                          GuidData[4], GuidData[5],
                          GuidData[6], GuidData[7],
                          GuidData[8], GuidData[9], GuidData[10], GuidData[11],
                          GuidData[12], GuidData[13], GuidData[14], GuidData[15]);
  return Str;
}


XStringW GuidLEToXStringW(const EFI_GUID& Guid)
{
  XStringW returnValue;
  returnValue.SWPrintf("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
  Guid.Data1, Guid.Data2, Guid.Data3, Guid.Data4[0], Guid.Data4[1],
  Guid.Data4[2], Guid.Data4[3], Guid.Data4[4], Guid.Data4[5], Guid.Data4[6], Guid.Data4[7]);
  return returnValue;
}

XString8 GuidLEToXString8(const EFI_GUID& Guid)
{
  XString8 returnValue;
  returnValue.S8Printf("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
  Guid.Data1, Guid.Data2, Guid.Data3, Guid.Data4[0], Guid.Data4[1],
  Guid.Data4[2], Guid.Data4[3], Guid.Data4[4], Guid.Data4[5], Guid.Data4[6], Guid.Data4[7]);
  return returnValue;
}

