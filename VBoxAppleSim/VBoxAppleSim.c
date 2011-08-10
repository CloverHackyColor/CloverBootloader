/* $Id: VBoxAppleSim.c $ */
/** @file
 * VBoxAppleSim.c - VirtualBox Apple Firmware simulation support
 */

/*
 * Copyright (C) 2010 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */
/*
  Slice 2011, 
  some more adoptations for Apple's OS
 */

/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include <Framework/FrameworkInternalFormRepresentation.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/DevicePathToText.h>
#include <Protocol/Smbios.h>
#include <Protocol/DataHub.h>

#include <IndustryStandard/Acpi10.h>
#include <IndustryStandard/Acpi20.h>
//#include <IndustryStandard/SmBios.h>
#include "SmBios.h"

#include <Guid/SmBios.h>
#include <Guid/Acpi.h>
#include <Guid/Mps.h>
//#include "DataHubRecords.h"
#include <Guid/DataHubRecords.h>

EFI_SYSTEM_TABLE *gSystemTable;
/*
 * External functions
 */
EFI_STATUS EFIAPI
CpuUpdateDataHub(EFI_DATA_HUB_PROTOCOL       *DataHub,
                 UINT64						FSBFrequency,
                 UINT64						TSCFrequency,
                 UINT64						CPUFrequency);

EFI_STATUS EFIAPI
LogData(EFI_DATA_HUB_PROTOCOL       *DataHub,
		EFI_GUID					*Guid1, ///* DataRecordGuid */
		EFI_GUID					*Guid2,
        CHAR16                      *Name,
        VOID                        *Data,
        UINT32                       DataSize);


EFI_STATUS EFIAPI
InitializeConsoleSim (IN EFI_HANDLE           ImageHandle,
                      IN EFI_SYSTEM_TABLE     *SystemTable);

EFI_GUID gDataHubPlatformGuid = {0x64517cc8, 0x6561, 0x4051, {0xb0, 0x3c, 0x59, 0x64, 0xb6, 0x0f, 0x4c, 0x7a}};
//EFI_GUID gDataHubOptionsGuid  = {0x0021001C, 0x3CE3, 0x41F8, {0x99, 0xc6, 0xec, 0xf5, 0xda, 0x75, 0x47, 0x31}};

#define TEST 0

// Example
/*
 //GUIDs from Ninja and more
 EFI_GUID gEfiLegacyBiosThunkProtocolGuid	= {0x4c51a7ba, 0x7195, 0x442d, {0x87, 0x92, 0xbe, 0xea, 0x6e, 0x2f, 0xf6, 0xec}};
 EFI_GUID gEfiLegacyBiosProtocolGuid		= {0xdb9a1e3d, 0x45cb, 0x4abb, {0x85, 0x3b, 0xe5, 0x38, 0x7f, 0xdb, 0x2e, 0x2d}};
 EFI_GUID gEfiLegacy8259ProtocolGuid		= {0x38321dba, 0x4fe0, 0x4e17, {0x8a, 0xec, 0x41, 0x30, 0x55, 0xea, 0xed, 0xc1}}; - 
 EFI_GUID gDataHubPlatformGuid				= {0x64517cc8, 0x6561, 0x4051, {0xb0, 0x3c, 0x59, 0x64, 0xb6, 0x0f, 0x4c, 0x7a}}; +
 EFI_GUID gEfiDataHubProtocolGuid			= {0xae80d021, 0x618e, 0x11d4, {0xbc, 0xd7, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81}}; +
 EFI_GUID gDataHubOptionsGuid				= {0x0021001C, 0x3CE3, 0x41F8, {0x99, 0xc6, 0xec, 0xf5, 0xda, 0x75, 0x47, 0x31}}; -
 EFI_GUID gEfiMiscSubClassGuid				= {0x772484B2, 0x7482, 0x4b91, {0x9F, 0x9A, 0xAD, 0x43, 0xF8, 0x1C, 0x58, 0x81}}; -
 EFI_GUID gEfiProcessorSubClassGuid			= {0x26fdeb7e, 0xb8af, 0x4ccf, {0xaa, 0x97, 0x02, 0x63, 0x3c, 0xe4, 0x8c, 0xa7}}; -
 EFI_GUID gEfiMemorySubClassGuid			= {0x4E8F4EBB, 0x64B9, 0x4e05, {0x9B, 0x18, 0x4C, 0xFE, 0x49, 0x23, 0x50, 0x97}}; -
 EFI_GUID gNotifyMouseActivity				= {0xF913C2C2, 0x5351, 0x4fdb, {0x93, 0x44, 0x70, 0xFF, 0xED, 0xB8, 0x42, 0x25}}; -
 EFI_GUID gConsoleControlGuid				= {0xf42f7782, 0x012e, 0x4c12, {0x99, 0x56, 0x49, 0xf9, 0x43, 0x04, 0xf7, 0x21}}; +
  -> gEfiConsoleControlProtocolGuid
 EFI_GUID gDevicePropertiesGuid				= {0x91BD12FE, 0xF6C3, 0x44FB, {0xA5, 0xB7, 0x51, 0x22, 0xAB, 0x30, 0x3A, 0xE0}}; + 
 EFI_GUID gEfiAppleBootGuid					= {0x7C436110, 0xAB2A, 0x4BBB, {0xA8, 0x80, 0xFE, 0x41, 0x99, 0x5C, 0x9F, 0x82}}; +//gEfiAppleVarGuid -> gAppleEFINVRAMGuid
 EFI_GUID gAppleScreenInfoGuid				= {0xe316e100, 0x0751, 0x4c49, {0x90, 0x56, 0x48, 0x6c, 0x7e, 0x47, 0x29, 0x03}}; +
 EFI_GUID gEfiCacheSubClassGuid				= {0x7f0013a7, 0xdc79, 0x4b22, {0x80, 0x99, 0x11, 0xf7, 0x5f, 0xdc, 0x82, 0x9d}}; - 
 EFI_GUID gEfiUnknown1ProtocolGuid			= {0xDD8E06AC, 0x00E2, 0x49A9, {0x88, 0x8F, 0xFA, 0x46, 0xDE, 0xD4, 0x0A, 0x52}}; -
 EFI_GUID gEfiAppleNvramGuid				= {0x4D1EDE05, 0x38C7, 0x4A6A, {0x9C, 0xC6, 0x4B, 0xCC, 0xA8, 0xB3, 0x8C, 0x14}}; +
 EFI_GUID FsbFrequencyPropertyGuid			= {0xD1A04D55, 0x75B9, 0x41A3, {0x90, 0x36, 0x8F, 0x4A, 0x26, 0x1C, 0xBB, 0xA2}}; //not found in boot.efi
 EFI_GUID DevicePathsSupportedGuid			= {0x5BB91CF7, 0xD816, 0x404B, {0x86, 0x72, 0x68, 0xF2, 0x7F, 0x78, 0x31, 0xDC}}; //not found
 EFI_GUID gNotifyExitBootServices			= {0xd2b2b828, 0x0826, 0x48a7, {0xb3, 0xdf, 0x98, 0x3c, 0x00, 0x60, 0x24, 0xf0}};
 // -> gEfiStatusCodeRuntimeProtocolGuid
  //Unknown protocols from Kabyl
 5B213447-6E73-4901-A4F1-B864F3B7A172  //GUID_001 efiboot loaded from device
 8FFEEB3A-4C98-4630-803F-740F9567091D  //GUID_003 recovery-boot, boot-args, efi-boot-kernelcache-data, efi-boot-file-data  /options?
 8ECE08D8-A6D4-430B-A7B0-2DF318E7884A  //gfx-saved-config-restore-status
 78EE99FB-6A5E-4186-97DE-CD0ABA345A74  //GUID_002 before device-properties
 //
 E3E9FD4F-1C1D-48AC-A86406E06547ADEE - unk_214E0 sub_15370+7E gAppleUn5Guid
 9BB9FD8E-5AAA-4ECD-AA5B5AC1B6136070 - unk_21510 sub_15500+E gAppleMkextProtocolGuid
 
 */

/*
 * GUIDs
 */
EFI_GUID gEfiAppleNvramGuid = {
    0x4D1EDE05, 0x38C7, 0x4A6A, {0x9C, 0xC6, 0x4B, 0xCC, 0xA8, 0xB3, 0x8C, 0x14 }
};

EFI_GUID gEfiAppleBootGuid = {
    0x7C436110, 0xAB2A, 0x4BBB, {0xA8, 0x80, 0xFE, 0x41, 0x99, 0x5C, 0x9F, 0x82}
};

EFI_GUID gDevicePropertiesGuid = {
    0x91BD12FE, 0xF6C3, 0x44FB, {0xA5, 0xB7, 0x51, 0x22, 0xAB, 0x30, 0x3A, 0xE0}
};
// used for boot.efi GUI but we can't use it
EFI_GUID gAppleScreenInfoGuid = {
	0xe316e100, 0x0751, 0x4c49, {0x90, 0x56, 0x48, 0x6c, 0x7e, 0x47, 0x29, 0x03}
};


#if TEST
EFI_GUID gEfiUnknown1ProtocolGuid = {
	0x78EE99FB, 0x6A5E, 0x4186, {0x97, 0xDE, 0xCD, 0x0A, 0xBA, 0x34, 0x5A, 0x74}
//    0xDD8E06AC, 0x00E2, 0x49A9, {0x88, 0x8F, 0xFA, 0x46, 0xDE, 0xD4, 0x0A, 0x52}
};
#endif
/*
 * Typedefs
 */
typedef struct _APPLE_GETVAR_PROTOCOL APPLE_GETVAR_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *APPLE_GETVAR_PROTOCOL_GET_DEVICE_PROPS) (
    IN     APPLE_GETVAR_PROTOCOL   *This,
    IN     CHAR8                   *Buffer,
    IN OUT UINT32                  *BufferSize);

static UINT32 mCount = 0;
static UINTN mAddr = 0;

struct _APPLE_GETVAR_PROTOCOL {
    EFI_STATUS(EFIAPI *Unknown0)(IN VOID *);
    EFI_STATUS(EFIAPI *Unknown1)(IN VOID *);
    EFI_STATUS(EFIAPI *Unknown2)(IN VOID *);
    EFI_STATUS(EFIAPI *Unknown3)(IN VOID *);
    APPLE_GETVAR_PROTOCOL_GET_DEVICE_PROPS  GetDevProps;
	APPLE_GETVAR_PROTOCOL_GET_DEVICE_PROPS  GetDevProps2;
};

#define EFI_INFO_INDEX_DEVICE_PROPS 0
#define IMPL_STUB(iface, num)                                   \
    EFI_STATUS EFIAPI                                           \
    iface##Unknown##num(IN  VOID   *This)                       \
    {                                                           \
		mCount = (num) + 0x10;									\
		mAddr = (UINTN)This;									\
        return EFI_SUCCESS;                                     \
    }

//        Print(L"Unknown%d of %a called", num, #iface);

IMPL_STUB(GetVar, 0)
IMPL_STUB(GetVar, 1)
IMPL_STUB(GetVar, 2)
IMPL_STUB(GetVar, 3)
//IMPL_STUB(GetVar, 4)
//IMPL_STUB(GetVar, 5)

EFI_STATUS EFIAPI
GetDeviceProps(IN     APPLE_GETVAR_PROTOCOL   *This,
               IN     CHAR8                   *Buffer,
               IN OUT UINT32                  *BufferSize)
{
    UINT32 BufLen = *BufferSize;
	UINT32 DataLen;

    //DataLen = GetVmVariable(EFI_INFO_INDEX_DEVICE_PROPS, Buffer, BufLen);
	DataLen = 1+sizeof(UINTN);
    *BufferSize = DataLen;
//Print(L"GetDeviceProps called with bufferlen=%d\n", BufLen);
    if (DataLen > BufLen)
        return EFI_BUFFER_TOO_SMALL;

	Buffer[0] = (CHAR8)(mCount & 0xFF);
//	Buffer[1] = 0x35;  
	CopyMem(&Buffer[1], &mAddr,  sizeof(UINTN));
	return EFI_SUCCESS;
}
#if TEST
/* system-type=02
 35000000010000000100000029000000010000001c000000730079007300740065006d002d00740079007000650000000500000002
 */
EFI_STATUS EFIAPI
GetDeviceProps2(IN     APPLE_GETVAR_PROTOCOL   *This,
               IN     CHAR8                   *Buffer,
               IN OUT UINT32                  *BufferSize)
{
    UINT32 BufLen = *BufferSize;
	UINT32 DataLen;
	
    //DataLen = GetVmVariable(EFI_INFO_INDEX_DEVICE_PROPS, Buffer, BufLen);
	DataLen = 2; //sizeof(UINT64);
    *BufferSize = DataLen;
	//Print(L"GetDeviceProps called with bufferlen=%d\n", BufLen);
    if (DataLen > BufLen)
        return EFI_BUFFER_TOO_SMALL;
	Buffer[0] = 0x36;
	Buffer[1] = (CHAR8)(mCount & 0xFF);
	mAddr = (UINT32)Buffer;
    return EFI_SUCCESS;
}
#endif

APPLE_GETVAR_PROTOCOL gPrivateVarHandler =
{
    GetVarUnknown0,
    GetVarUnknown1,
    GetVarUnknown2,
    GetVarUnknown3,
    GetDeviceProps,
	GetDeviceProps
};
#if TEST
EFI_STATUS EFIAPI
UnknownHandlerImpl()
{
//    Print(L"Unknown called\n");
	mCount |=0x40;
    return EFI_SUCCESS;
}

IMPL_STUB(Proto, 0)
IMPL_STUB(Proto, 1)
IMPL_STUB(Proto, 2)
IMPL_STUB(Proto, 3)
IMPL_STUB(Proto, 4)
IMPL_STUB(Proto, 5)
IMPL_STUB(Proto, 6)
IMPL_STUB(Proto, 7)
IMPL_STUB(Proto, 8)
IMPL_STUB(Proto, 9)

/* array of pointers to function */  // 18 procs
EFI_STATUS (EFIAPI *gUnknownProtoHandler[])() =
{
    GetDeviceProps2,
    ProtoUnknown1,
    ProtoUnknown2,
    ProtoUnknown3,
    ProtoUnknown4,
    ProtoUnknown5,
    ProtoUnknown6,
    ProtoUnknown7,
    ProtoUnknown8,
    ProtoUnknown9,
    UnknownHandlerImpl,
    UnknownHandlerImpl,
    UnknownHandlerImpl,
    UnknownHandlerImpl,
    UnknownHandlerImpl,
    UnknownHandlerImpl,
    UnknownHandlerImpl,
    UnknownHandlerImpl
};
#endif
//This part of codes origin from iBoot, placed here to test purpose
// screen
#if TEST
typedef	EFI_STATUS (EFIAPI *EFI_SCREEN_INFO_FUNCTION)(
													  VOID* This, 
													  UINT64* baseAddress,
													  UINT64* frameBufferSize,
													  UINT32* byterPerRow,
													  UINT32* Width,
													  UINT32* Height,
													  UINT32* colorDepth
													  );

typedef struct {	
	EFI_SCREEN_INFO_FUNCTION GetScreenInfo;	
} EFI_INTERFACE_SCREEN_INFO;

EFI_STATUS GetScreenInfo(VOID* This, UINT64* baseAddress, UINT64* frameBufferSize, UINT32* bpr, UINT32* w, UINT32* h, UINT32* colorDepth)
{
	EFI_GRAPHICS_OUTPUT_PROTOCOL	*GraphicsOutput=NULL;
	EFI_STATUS						Status;
	mCount |= 0x80;
	
	Status=gBS->HandleProtocol (
								gSystemTable->ConsoleOutHandle,
								&gEfiGraphicsOutputProtocolGuid,
								(VOID **) &GraphicsOutput);
	if(EFI_ERROR(Status))
		return EFI_UNSUPPORTED;
	
	*frameBufferSize=(UINT64)GraphicsOutput->Mode->FrameBufferSize;
	*baseAddress=(UINT64)GraphicsOutput->Mode->FrameBufferBase;
	*w=(UINT32)GraphicsOutput->Mode->Info->HorizontalResolution;
	*h=(UINT32)GraphicsOutput->Mode->Info->VerticalResolution;
	*colorDepth=32;
	*bpr=(UINT32)(GraphicsOutput->Mode->Info->PixelsPerScanLine*32)/8;
	
	return EFI_SUCCESS;
}


EFI_INTERFACE_SCREEN_INFO mScreenInfo=
{
	GetScreenInfo
};
#endif
//

EFI_STATUS EFIAPI
SetPrivateVarProto(IN EFI_HANDLE ImageHandle, EFI_BOOT_SERVICES * bs)
{
    EFI_STATUS  Status;
	
    Status = gBS->InstallMultipleProtocolInterfaces (
												 &ImageHandle,
												 &gDevicePropertiesGuid,
												 &gPrivateVarHandler,
												 NULL
                                                 );
    ASSERT_EFI_ERROR (Status);
#if TEST	
	Status=gBS->InstallProtocolInterface(
									 &gImageHandle,
									 &gAppleScreenInfoGuid,
									 EFI_NATIVE_INTERFACE,
									 &mScreenInfo
									 );
	ASSERT_EFI_ERROR (Status);
#endif	
    return EFI_SUCCESS;
}

CHAR8 *GetSmbiosString (SMBIOS_STRUCTURE_POINTER SmbiosTable,SMBIOS_TABLE_STRING String)
{
	CHAR8      *AString;
	UINT8      Index;
	
	Index = 1;
	AString = (CHAR8 *)(SmbiosTable.Raw + SmbiosTable.Hdr->Length);
	while (Index != String) {
		while (*AString != 0) {
			AString ++;
		}
		AString ++;
		if (*AString == 0) {
			return AString;
		}
		Index ++;
	}
	
	return AString;
}


EFI_STATUS EFIAPI
SetProperVariables(IN EFI_HANDLE ImageHandle,  IN EFI_SYSTEM_TABLE *SystemTable)
{
     EFI_STATUS          Status;
     UINT32              vBackgroundClear = 0x00000000;
	EFI_RUNTIME_SERVICES * rs = SystemTable->RuntimeServices;
/*	
     UINT32              vFwFeatures      = 0x80000015;
     UINT32              vFwFeaturesMask  = 0x800003ff;
*/
     // -legacy acpi=0xffffffff acpi_debug=0xfffffff panic_io_port=0xef11 io=0xfffffffe trace=4096  io=0xffffffef -v serial=2 serialbaud=9600
     // 0x10 makes kdb default, thus 0x15e for kdb, 0x14e for gdb
     //static const CHAR8  vBootArgs[]      = "debug=0x15e keepsyms=1 acpi=0xffffffff acpi_debug=0xff acpi_level=7 -v -x32 -s";
	// or just "debug=0x8 -legacy",  0x14e for serial output
     
     static const CHAR8  vDefBootArgs[]      = "-v arch=i386 kernel=mach_kernel";
     CHAR8  vBootArgs[256];
     UINT32 BootArgsLen;

	BootArgsLen = sizeof vDefBootArgs;
    CopyMem(vBootArgs, vDefBootArgs, BootArgsLen);

     Status = rs->SetVariable(L"BackgroundClear",
                          &gEfiAppleNvramGuid,
                          /* EFI_VARIABLE_NON_VOLATILE | */ EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                          sizeof(vBackgroundClear), &vBackgroundClear);
/*
     Status = rs->SetVariable(L"FirmwareFeatures",
                          &gEfiAppleNvramGuid,
                          EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                          sizeof(vFwFeatures), &vFwFeatures);

     Status = rs->SetVariable(L"FirmwareFeaturesMask",
                          &gEfiAppleNvramGuid,
                          EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                          sizeof(vFwFeaturesMask), &vFwFeaturesMask);
*/
     Status = rs->SetVariable(L"boot-args",
                          &gEfiAppleBootGuid,
                          EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                          BootArgsLen, &vBootArgs);

     return EFI_SUCCESS;
}

/**
 * VBoxInitAppleSim entry point.
 *
 * @returns EFI status code.
 *
 * @param   ImageHandle     The image handle.
 * @param   SystemTable     The system table pointer.
 */
EFI_STATUS EFIAPI
VBoxInitAppleSim(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS          Status;
    UINT64              FSBFrequency =  200000000ull;
    UINT64              TSCFrequency = 2400000000ull;
    UINT64              CPUFrequency = 2400000000ull;
	//Slice 
	UINT8								Find = 0;
	EFI_SMBIOS_HANDLE					SmbiosHandle;
	EFI_SMBIOS_PROTOCOL					*Smbios;
	EFI_SMBIOS_TABLE_HEADER				*Record;
	SMBIOS_STRUCTURE_POINTER			SmbiosTable;
	EFI_DATA_HUB_PROTOCOL				*DataHub;
	UINT32              vFwFeatures      = 0x80000015;
	UINT32              vFwFeaturesMask  = 0x800003ff;
	UINT32				devPathSupportedVal = 1;
	CHAR8		*TmpString;
	EFI_GUID	SystemID;
	UINT8		SystemType;
	CHAR16      *UString;
	UINTN		UStrSize;
	UINT16		Counter = 0;
	
	EFI_RUNTIME_SERVICES * rs = SystemTable->RuntimeServices;
	gSystemTable = SystemTable;

    //
    // Locate DataHub protocol.
    //
    Status = gBS->LocateProtocol (&gEfiDataHubProtocolGuid, NULL, (VOID**)&DataHub);
    ASSERT_EFI_ERROR (Status);
	
    Status = SetProperVariables(ImageHandle, SystemTable);
    ASSERT_EFI_ERROR (Status);

    Status = SetPrivateVarProto(ImageHandle, gBS);
    ASSERT_EFI_ERROR (Status);

//Slice - take values from DMI
	Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID **) &Smbios);
	ASSERT_EFI_ERROR (Status);
//TODO -add "SystemSerialNumber", "DevicePathsSupported", "system-id"
// "bootOrder"? "board-id", "system-type"
	SmbiosHandle = 0;
	do {
		Status = Smbios->GetNext (Smbios, &SmbiosHandle, NULL, &Record, NULL);
		if (EFI_ERROR(Status)) {
			break;
		}
		if (Record->Type == EFI_SMBIOS_TYPE_SYSTEM_INFORMATION) {
			SmbiosTable = (SMBIOS_STRUCTURE_POINTER)(SMBIOS_TABLE_TYPE1*)Record;
			TmpString = GetSmbiosString (SmbiosTable, SmbiosTable.Type1->SerialNumber);
			UStrSize = (AsciiStrLen(TmpString) + 1) * sizeof(CHAR16);
			UString = AllocateZeroPool (UStrSize);
			ASSERT (UString != NULL);
			AsciiStrToUnicodeStr (TmpString, UString);			
			LogData(DataHub, &gEfiMiscSubClassGuid, &gDataHubPlatformGuid,
					L"SystemSerialNumber", (VOID*)UString, UStrSize);
			CopyMem(&SystemID, &SmbiosTable.Type1->Uuid, 16);
			LogData(DataHub, &gEfiMiscSubClassGuid, &gDataHubPlatformGuid,
					L"system-id", &SystemID, 16);
/*			Status = rs->SetVariable(L"system-id",
									 &gEfiAppleBootGuid,
									 EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
									 16, &SystemID);
*/			
			TmpString = GetSmbiosString (SmbiosTable, SmbiosTable.Type1->ProductName);
			UStrSize = (AsciiStrLen(TmpString) + 1) * sizeof(CHAR16);
			UString = AllocateZeroPool (UStrSize);
			ASSERT (UString != NULL);
			AsciiStrToUnicodeStr (TmpString, UString);			
			LogData(DataHub, &gEfiMiscSubClassGuid, &gDataHubPlatformGuid,
					L"Model", (VOID*)UString, UStrSize);
			
			Find |= 1;
		}
		if (Record->Type == EFI_SMBIOS_TYPE_BASEBOARD_INFORMATION) {
			SmbiosTable = (SMBIOS_STRUCTURE_POINTER)(SMBIOS_TABLE_TYPE2 *)Record;
			TmpString = GetSmbiosString (SmbiosTable, SmbiosTable.Type2->ProductName);
			UStrSize = (AsciiStrLen(TmpString) + 1) * sizeof(CHAR16);
			UString = AllocateZeroPool (UStrSize);
			ASSERT (UString != NULL);
			AsciiStrToUnicodeStr (TmpString, UString);			
			LogData(DataHub, &gEfiAppleBootGuid, &gDataHubPlatformGuid,
					L"board-id", (VOID*)UString, UStrSize);
/*			Status = rs->SetVariable(L"board-id",
									 &gEfiAppleBootGuid,
									 EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
									 AsciiStrLen(TmpString), &TmpString);
 */
			Find |= 2;
		}
		if (Record->Type == EFI_SMBIOS_TYPE_SYSTEM_ENCLOSURE) {
			SmbiosTable = (SMBIOS_STRUCTURE_POINTER)(SMBIOS_TABLE_TYPE3 *)Record;
			SystemType = (SmbiosTable.Type3->Type >= 8)?2:1;
//			LogData(DataHub, &gEfiAppleBootGuid, &gDataHubOptionsGuid, 
//					L"system-type", &SystemType, 1);
			Status = rs->SetVariable(L"system-type",
									 &gEfiAppleBootGuid,
									 EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
									 1, &SystemType);			
			Find |= 4;
		}
				
		if (Record->Type == EFI_SMBIOS_TYPE_PROCESSOR_INFORMATION) {
			SmbiosTable = (SMBIOS_STRUCTURE_POINTER)(SMBIOS_TABLE_TYPE4 *)Record;
			CPUFrequency = SmbiosTable.Type4->CurrentSpeed * 1000000ull;
			TSCFrequency = SmbiosTable.Type4->MaxSpeed * 1000000ull;
			FSBFrequency = SmbiosTable.Type4->ExternalClock * 1000000ull;
			Find |= 8;
		}
		if (Record->Type == 128) {
			SmbiosTable = (SMBIOS_STRUCTURE_POINTER)(SMBIOS_TABLE_TYPE128 *)Record;
			vFwFeatures = SmbiosTable.Type128->FirmwareFeatures;
			vFwFeaturesMask = SmbiosTable.Type128->FirmwareFeaturesMask;
			Find |= 0x10;
		}
		Counter++;
	} while ((Find != 0x1F) && (Counter < 300)); //restrict 300 attempts
	
	Status = rs->SetVariable(L"FirmwareFeatures",
						 &gEfiAppleNvramGuid,
						 EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
						 sizeof(vFwFeatures), &vFwFeatures);
	
	Status = rs->SetVariable(L"FirmwareFeaturesMask",
						 &gEfiAppleNvramGuid,
						 EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
						 sizeof(vFwFeaturesMask), &vFwFeaturesMask);
// Here I want to double this variable to datahub test values
	LogData(DataHub, &gEfiAppleNvramGuid, &gDataHubPlatformGuid,
			L"FirmwareFeatures", &vFwFeatures, sizeof(vFwFeatures));
	LogData(DataHub, &gEfiAppleNvramGuid, &gDataHubPlatformGuid, 
			L"FirmwareFeaturesMask", &vFwFeaturesMask, sizeof(vFwFeaturesMask));	
	LogData(DataHub, &gDataHubPlatformGuid, &gDataHubPlatformGuid,
			L"DevicePathsSupported", &devPathSupportedVal, sizeof(devPathSupportedVal));
//	
	Status = CpuUpdateDataHub(DataHub, FSBFrequency, TSCFrequency, CPUFrequency);
    ASSERT_EFI_ERROR (Status);

    Status = InitializeConsoleSim(ImageHandle, SystemTable);
    ASSERT_EFI_ERROR (Status);
#if TEST
    Status = gBS->InstallMultipleProtocolInterfaces (
                                                 &ImageHandle,
                                                 &gEfiUnknown1ProtocolGuid,
                                                 gUnknownProtoHandler,
                                                 NULL
                                                 );
    ASSERT_EFI_ERROR (Status);
#endif
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
VBoxDeinitAppleSim(IN EFI_HANDLE         ImageHandle)
{
    return EFI_SUCCESS;
}
