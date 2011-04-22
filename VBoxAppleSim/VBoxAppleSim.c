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


/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/DevicePathToText.h>
#include <Protocol/Smbios.h>

#include <IndustryStandard/Acpi10.h>
#include <IndustryStandard/Acpi20.h>
//#include <IndustryStandard/SmBios.h>
#include "SmBios.h"

#include <Guid/SmBios.h>
#include <Guid/Acpi.h>
#include <Guid/Mps.h>


//#include "VBoxPkg.h"
//#include "DevEFI.h"
//#include "iprt/asm.h"


/*
 * External functions
 */
EFI_STATUS EFIAPI
CpuUpdateDataHub(EFI_BOOT_SERVICES * bs,
                 UINT64              FSBFrequency,
                 UINT64              TSCFrequency,
                 UINT64              CPUFrequency);

EFI_STATUS EFIAPI
InitializeConsoleSim (IN EFI_HANDLE           ImageHandle,
                      IN EFI_SYSTEM_TABLE     *SystemTable);

// Example
/*
 
 EFI_GUID gEfiLegacyBiosThunkProtocolGuid	= {0x4c51a7ba, 0x7195, 0x442d, {0x87, 0x92, 0xbe, 0xea, 0x6e, 0x2f, 0xf6, 0xec}};
 EFI_GUID gEfiLegacyBiosProtocolGuid		= {0xdb9a1e3d, 0x45cb, 0x4abb, {0x85, 0x3b, 0xe5, 0x38, 0x7f, 0xdb, 0x2e, 0x2d}};
 EFI_GUID gEfiLegacy8259ProtocolGuid		= {0x38321dba, 0x4fe0, 0x4e17, {0x8a, 0xec, 0x41, 0x30, 0x55, 0xea, 0xed, 0xc1}};
 EFI_GUID gDataHubPlatformGuid				= {0x64517cc8, 0x6561, 0x4051, {0xb0, 0x3c, 0x59, 0x64, 0xb6, 0x0f, 0x4c, 0x7a}}; 
 EFI_GUID gEfiDataHubProtocolGuid			= {0xae80d021, 0x618e, 0x11d4, {0xbc, 0xd7, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81}};
 EFI_GUID gDataHubOptionsGuid				= {0x0021001C, 0x3CE3, 0x41F8, {0x99, 0xc6, 0xec, 0xf5, 0xda, 0x75, 0x47, 0x31}};
 EFI_GUID gEfiMiscSubClassGuid				= {0x772484B2, 0x7482, 0x4b91, {0x9F, 0x9A, 0xAD, 0x43, 0xF8, 0x1C, 0x58, 0x81}};
 EFI_GUID gEfiProcessorSubClassGuid			= {0x26fdeb7e, 0xb8af, 0x4ccf, {0xaa, 0x97, 0x02, 0x63, 0x3c, 0xe4, 0x8c, 0xa7}};
 EFI_GUID gEfiMemorySubClassGuid			= {0x4E8F4EBB, 0x64B9, 0x4e05, {0x9B, 0x18, 0x4C, 0xFE, 0x49, 0x23, 0x50, 0x97}};
 EFI_GUID gNotifyMouseActivity				= {0xF913C2C2, 0x5351, 0x4fdb, {0x93, 0x44, 0x70, 0xFF, 0xED, 0xB8, 0x42, 0x25}};
 EFI_GUID gConsoleControlGuid				= {0xf42f7782, 0x012e, 0x4c12, {0x99, 0x56, 0x49, 0xf9, 0x43, 0x04, 0xf7, 0x21}};
 EFI_GUID gDevicePropertiesGuid				= {0x91BD12FE, 0xF6C3, 0x44FB, {0xA5, 0xB7, 0x51, 0x22, 0xAB, 0x30, 0x3A, 0xE0}};
 EFI_GUID gEfiAppleBootGuid					= {0x7C436110, 0xAB2A, 0x4BBB, {0xA8, 0x80, 0xFE, 0x41, 0x99, 0x5C, 0x9F, 0x82}};//gEfiAppleVarGuid
 EFI_GUID gAppleScreenInfoGuid				= {0xe316e100, 0x0751, 0x4c49, {0x90, 0x56, 0x48, 0x6c, 0x7e, 0x47, 0x29, 0x03}};
 EFI_GUID gEfiCacheSubClassGuid				= {0x7f0013a7, 0xdc79, 0x4b22, {0x80, 0x99, 0x11, 0xf7, 0x5f, 0xdc, 0x82, 0x9d}};
 EFI_GUID gEfiUnknown1ProtocolGuid			= {0xDD8E06AC, 0x00E2, 0x49A9, {0x88, 0x8F, 0xFA, 0x46, 0xDE, 0xD4, 0x0A, 0x52}};
 EFI_GUID gEfiAppleNvramGuid				= {0x4D1EDE05, 0x38C7, 0x4A6A, {0x9C, 0xC6, 0x4B, 0xCC, 0xA8, 0xB3, 0x8C, 0x14}};
 EFI_GUID FsbFrequencyPropertyGuid			= {0xD1A04D55, 0x75B9, 0x41A3, {0x90, 0x36, 0x8F, 0x4A, 0x26, 0x1C, 0xBB, 0xA2}};
 EFI_GUID DevicePathsSupportedGuid			= {0x5BB91CF7, 0xD816, 0x404B, {0x86, 0x72, 0x68, 0xF2, 0x7F, 0x78, 0x31, 0xDC}};

EFI_STATUS SetEfiPlatformProperty(EFI_STRING Name, EFI_GUID EfiPropertyGuid, VOID *Data, UINT32 DataSize)
{
	EFI_STATUS Status;
	UINT32 DataNameSize;
	EFI_PROPERTY_SUBCLASS_DATA *DataRecord;
	
	DataNameSize = (UINT32)EfiStrSize(Name);
	
	DataRecord = EfiLibAllocateZeroPool(sizeof(EFI_PROPERTY_SUBCLASS_DATA) + DataNameSize + DataSize);
	ASSERT (DataRecord != NULL);
	
	DataRecord->Header.Version = EFI_DATA_RECORD_HEADER_VERSION;
	DataRecord->Header.HeaderSize = sizeof(EFI_SUBCLASS_TYPE1_HEADER);
	DataRecord->Header.Instance = 0xFFFF;
	DataRecord->Header.SubInstance = 0xFFFF;
	DataRecord->Header.RecordType = 0xFFFFFFFF;
	DataRecord->Record.DataNameSize = DataNameSize;
	DataRecord->Record.DataSize = DataSize;
	
	
	EfiCopyMem((UINT8 *)DataRecord + sizeof(EFI_PROPERTY_SUBCLASS_DATA), Name, DataNameSize);
	EfiCopyMem((UINT8 *)DataRecord + sizeof(EFI_PROPERTY_SUBCLASS_DATA) + DataNameSize, Data, DataSize);
	
	Status = gDataHub->LogData(gDataHub, &EfiPropertyGuid, &SpecialGuid, 
							   EFI_DATA_RECORD_CLASS_DATA,
							   DataRecord,
							   sizeof(EFI_PROPERTY_SUBCLASS_DATA) + DataNameSize + DataSize);
	
	if (DataRecord)
		gBS->FreePool(DataRecord);
	
	return Status;
}

 
 VOID SetupPlatformInfo(VOID)
{
	UINT64 FsbFrequency = 200000000;
	UINT32 DevicePathsSupported = 1;
	
	SetEfiPlatformProperty(L"FSBFrequency", FsbFrequencyPropertyGuid, &FsbFrequency, sizeof(UINT64));
	SetEfiPlatformProperty(L"DevicePathsSupported", DevicePathsSupportedGuid, &DevicePathsSupported, sizeof(UINT32));
}
 */
/*
 *   Internal Functions                                                        *
 */
#if NOTCLOVER
static UINT32
GetVmVariable(UINT32 Variable, CHAR8* Buffer, UINT32 Size )
{
    UINT32 VarLen = 0;

/*
 int i;
    ASMOutU32(EFI_INFO_PORT, Variable);
    VarLen = ASMInU32(EFI_INFO_PORT);

    for (i=0; i < VarLen && i < Size; i++)
    {
        Buffer[i] = ASMInU8(EFI_INFO_PORT);
    }
*/
//#warning  function GetVmVariable should be implemented through DeviceTree	
    return VarLen;
}
#endif
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
#if NOTCLOVER
EFI_GUID gEfiUnknown1ProtocolGuid = {
    0xDD8E06AC, 0x00E2, 0x49A9, {0x88, 0x8F, 0xFA, 0x46, 0xDE, 0xD4, 0x0A, 0x52}
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

struct _APPLE_GETVAR_PROTOCOL {
    EFI_STATUS(EFIAPI *Unknown0)(IN VOID *);
    EFI_STATUS(EFIAPI *Unknown1)(IN VOID *);
    EFI_STATUS(EFIAPI *Unknown2)(IN VOID *);
    EFI_STATUS(EFIAPI *Unknown3)(IN VOID *);
    APPLE_GETVAR_PROTOCOL_GET_DEVICE_PROPS  GetDevProps;
	APPLE_GETVAR_PROTOCOL_GET_DEVICE_PROPS  GetDevProps;
};

#define EFI_INFO_INDEX_DEVICE_PROPS 0
#define IMPL_STUB(iface, num)                                   \
    EFI_STATUS EFIAPI                                           \
    iface##Unknown##num(IN  VOID   *This)                       \
    {                                                           \
		mCount = (num) + 1;											\
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
	DataLen = 2; //sizeof(UINT64);
    *BufferSize = DataLen;
//Print(L"GetDeviceProps called with bufferlen=%d\n", BufLen);
    if (DataLen > BufLen)
        return EFI_BUFFER_TOO_SMALL;

	Buffer[0] = (CHAR8)(mCount & 0xFF);
	Buffer[1] = 0x35;  
	return EFI_SUCCESS;
}
#if TEST
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
#if NOTCLOVER
EFI_STATUS EFIAPI
UnknownHandlerImpl()
{
//    Print(L"Unknown called\n");
	mCount++;
    return EFI_SUCCESS;
}

/* array of pointers to function */  // 18 procs
EFI_STATUS (EFIAPI *gUnknownProtoHandler[])() =
{
    UnknownHandlerImpl,
    UnknownHandlerImpl,
    UnknownHandlerImpl,
    UnknownHandlerImpl,
    UnknownHandlerImpl,
    UnknownHandlerImpl,
    UnknownHandlerImpl,
    UnknownHandlerImpl,
    UnknownHandlerImpl,
    UnknownHandlerImpl,
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
EFI_STATUS EFIAPI
SetPrivateVarProto(IN EFI_HANDLE ImageHandle, EFI_BOOT_SERVICES * bs)
{
    EFI_STATUS  rc;
	
    rc = gBS->InstallMultipleProtocolInterfaces (
												 &ImageHandle,
												 &gDevicePropertiesGuid,
												 &gPrivateVarHandler,
												 NULL
                                                 );
    ASSERT_EFI_ERROR (rc);
	
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
SetProperVariables(IN EFI_HANDLE ImageHandle, EFI_RUNTIME_SERVICES * rs)
{
     EFI_STATUS          rc;
     UINT32              vBackgroundClear = 0x00000000;
/*	
     UINT32              vFwFeatures      = 0x80000015;
     UINT32              vFwFeaturesMask  = 0x800003ff;
*/
     // -legacy acpi=0xffffffff acpi_debug=0xfffffff panic_io_port=0xef11 io=0xfffffffe trace=4096  io=0xffffffef -v serial=2 serialbaud=9600
     // 0x10 makes kdb default, thus 0x15e for kdb, 0x14e for gdb

     //static const CHAR8  vBootArgs[]      = "debug=0x15e keepsyms=1 acpi=0xffffffff acpi_debug=0xff acpi_level=7 -v -x32 -s"; // or just "debug=0x8 -legacy"
     // 0x14e for serial output
     //static const CHAR8  vDefBootArgs[]      = "debug=0x146 keepsyms=1 -v -serial=0x1";
     static const CHAR8  vDefBootArgs[]      = "-v arch=i386 kernel=mach_kernel";
     CHAR8  vBootArgs[256];
     UINT32 BootArgsLen;

	BootArgsLen = 0; 
	//GetVmVariable(EFI_INFO_INDEX_BOOT_ARGS, vBootArgs, sizeof vBootArgs);
     if (BootArgsLen <= 1)
     {
         BootArgsLen = sizeof vDefBootArgs;
         CopyMem(vBootArgs, vDefBootArgs, BootArgsLen);
     }
     rc = rs->SetVariable(L"BackgroundClear",
                          &gEfiAppleNvramGuid,
                          /* EFI_VARIABLE_NON_VOLATILE | */ EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                          sizeof(vBackgroundClear), &vBackgroundClear);
/*
     rc = rs->SetVariable(L"FirmwareFeatures",
                          &gEfiAppleNvramGuid,
                          EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                          sizeof(vFwFeatures), &vFwFeatures);

     rc = rs->SetVariable(L"FirmwareFeaturesMask",
                          &gEfiAppleNvramGuid,
                          EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                          sizeof(vFwFeaturesMask), &vFwFeaturesMask);
*/
     rc = rs->SetVariable(L"boot-args",
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
    EFI_STATUS          rc;
    UINT64              FSBFrequency;
    UINT64              TSCFrequency;
    UINT64              CPUFrequency;
	//Slice 
	BOOLEAN                           Find4 = FALSE;
	BOOLEAN                           Find128 = FALSE;

	EFI_SMBIOS_HANDLE                 SmbiosHandle;
	EFI_SMBIOS_PROTOCOL               *Smbios;
	EFI_SMBIOS_TABLE_HEADER           *Record;
	//SMBIOS_TABLE_TYPE0                *Type0Record;
	SMBIOS_TABLE_TYPE128              *Type128Record;
	SMBIOS_TABLE_TYPE4                *Type4Record;
	UINT32              vFwFeatures      = 0x80000015;
	UINT32              vFwFeaturesMask  = 0x800003ff;
	EFI_RUNTIME_SERVICES * rs = SystemTable->RuntimeServices;

	//
	
    rc = SetProperVariables(ImageHandle, rs);
    ASSERT_EFI_ERROR (rc);

    rc = SetPrivateVarProto(ImageHandle, gBS);
    ASSERT_EFI_ERROR (rc);
/*
    GetVmVariable(EFI_INFO_INDEX_FSB_FREQUENCY, (CHAR8*)&FSBFrequency, sizeof FSBFrequency);
    GetVmVariable(EFI_INFO_INDEX_TSC_FREQUENCY, (CHAR8*)&TSCFrequency, sizeof TSCFrequency);
    GetVmVariable(EFI_INFO_INDEX_CPU_FREQUENCY, (CHAR8*)&CPUFrequency, sizeof CPUFrequency);
*/
	//initial values
	FSBFrequency =  200000000ull;
	TSCFrequency = 2400000000ull;
	CPUFrequency = 2400000000ull;
//Slice - take values from DMI
	rc = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID **) &Smbios);
	ASSERT_EFI_ERROR (rc);
	
	SmbiosHandle = 0;
	do {
		rc = Smbios->GetNext (Smbios, &SmbiosHandle, NULL, &Record, NULL);
		if (EFI_ERROR(rc)) {
			break;
		}
		
		if (Record->Type == EFI_SMBIOS_TYPE_PROCESSOR_INFORMATION) {
			Type4Record = (SMBIOS_TABLE_TYPE4 *) Record;
			CPUFrequency = Type4Record->CurrentSpeed;
			TSCFrequency = CPUFrequency * 1000000ull;
			Type4Record->MaxSpeed = CPUFrequency; //no effect here
			CPUFrequency = TSCFrequency;
			FSBFrequency = Type4Record->ExternalClock * 1000000ull;
			Find4 = TRUE;
		}
		if (Record->Type == 128) {
			Type128Record = (SMBIOS_TABLE_TYPE128 *) Record;
			vFwFeatures = Type128Record->FirmwareFeatures;
			vFwFeaturesMask = Type128Record->FirmwareFeaturesMask;
			Find128  = TRUE;
		}
	} while (!Find4 || !Find128);
	
	rc = rs->SetVariable(L"FirmwareFeatures",
						 &gEfiAppleNvramGuid,
						 EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
						 sizeof(vFwFeatures), &vFwFeatures);
	
	rc = rs->SetVariable(L"FirmwareFeaturesMask",
						 &gEfiAppleNvramGuid,
						 EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
						 sizeof(vFwFeaturesMask), &vFwFeaturesMask);
	
//	
	rc = CpuUpdateDataHub(gBS, FSBFrequency, TSCFrequency, CPUFrequency);
    ASSERT_EFI_ERROR (rc);

    rc = InitializeConsoleSim(ImageHandle, SystemTable);
    ASSERT_EFI_ERROR (rc);
/*
    rc = gBS->InstallMultipleProtocolInterfaces (
                                                 &ImageHandle,
                                                 &gEfiUnknown1ProtocolGuid,
                                                 gUnknownProtoHandler,
                                                 NULL
                                                 );
    ASSERT_EFI_ERROR (rc);
*/
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
VBoxDeinitAppleSim(IN EFI_HANDLE         ImageHandle)
{
    return EFI_SUCCESS;
}
