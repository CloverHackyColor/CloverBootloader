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

EFI_GUID gEfiAppleVarGuid = {
    0x91BD12FE, 0xF6C3, 0x44FB, {0xA5, 0xB7, 0x51, 0x22, 0xAB, 0x30, 0x3A, 0xE0}
};

EFI_GUID gEfiUnknown1ProtocolGuid = {
    0xDD8E06AC, 0x00E2, 0x49A9, {0x88, 0x8F, 0xFA, 0x46, 0xDE, 0xD4, 0x0A, 0x52}
};

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


struct _APPLE_GETVAR_PROTOCOL {
    EFI_STATUS(EFIAPI *Unknown0)(IN VOID *);
    EFI_STATUS(EFIAPI *Unknown1)(IN VOID *);
    EFI_STATUS(EFIAPI *Unknown2)(IN VOID *);
    EFI_STATUS(EFIAPI *Unknown3)(IN VOID *);
//    EFI_STATUS(EFIAPI *Unknown4)(IN VOID *);
    APPLE_GETVAR_PROTOCOL_GET_DEVICE_PROPS  GetDevProps;
	APPLE_GETVAR_PROTOCOL_GET_DEVICE_PROPS  GetDevProps2;
//	EFI_STATUS(EFIAPI *Unknown5)(IN VOID *);
};

#define EFI_INFO_INDEX_DEVICE_PROPS 0
#define IMPL_STUB(iface, num)                                   \
    EFI_STATUS EFIAPI                                           \
    iface##Unknown##num(IN  VOID   *This)                       \
    {                                                           \
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
	DataLen = 1; //sizeof(UINT64);
    *BufferSize = DataLen;
//Print(L"GetDeviceProps called with bufferlen=%d\n", BufLen);
    if (DataLen > BufLen)
        return EFI_BUFFER_TOO_SMALL;
	Buffer[0] = 0x35;
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
GetDeviceProps2(IN     APPLE_GETVAR_PROTOCOL   *This,
               IN     CHAR8                   *Buffer,
               IN OUT UINT32                  *BufferSize)
{
    UINT32 BufLen = *BufferSize;
	UINT32 DataLen;
	
    //DataLen = GetVmVariable(EFI_INFO_INDEX_DEVICE_PROPS, Buffer, BufLen);
	DataLen = 1; //sizeof(UINT64);
    *BufferSize = DataLen;
	//Print(L"GetDeviceProps called with bufferlen=%d\n", BufLen);
    if (DataLen > BufLen)
        return EFI_BUFFER_TOO_SMALL;
	Buffer[0] = 0x36;
    return EFI_SUCCESS;
}

APPLE_GETVAR_PROTOCOL gPrivateVarHandler =
{
    GetVarUnknown0,
    GetVarUnknown1,
    GetVarUnknown2,
    GetVarUnknown3,
//    GetVarUnknown4,
    GetDeviceProps,
	GetDeviceProps2
};

EFI_STATUS EFIAPI
UnknownHandlerImpl()
{
//    Print(L"Unknown called\n");
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

EFI_STATUS EFIAPI
SetPrivateVarProto(IN EFI_HANDLE ImageHandle, EFI_BOOT_SERVICES * bs)
{
    EFI_STATUS  rc;
	
    rc = gBS->InstallMultipleProtocolInterfaces (
												 &ImageHandle,
												 &gEfiAppleVarGuid,
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

    rc = gBS->InstallMultipleProtocolInterfaces (
                                                 &ImageHandle,
                                                 &gEfiUnknown1ProtocolGuid,
                                                 gUnknownProtoHandler,
                                                 NULL
                                                 );
    ASSERT_EFI_ERROR (rc);

    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
VBoxDeinitAppleSim(IN EFI_HANDLE         ImageHandle)
{
    return EFI_SUCCESS;
}
