/* $Id: Cpu.c $ */
/** @file
 * Cpu.c - VirtualBox CPU descriptors
 */

/*
 * Copyright (C) 2009-2010 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */

#define DEBUG_DH 1

#if DEBUG_DH == 2
#define DBG(x...) AsciiPrint(x)
#elif DEBUG_DH == 1
#define DBG(x...) MsgLog(x)
#else
#define DBG(x...)
#endif


/*******************************************************************************
 *   Header Files                                                               *
 *******************************************************************************/
//#include <Framework/FrameworkInternalFormRepresentation.h>
#include "Platform.h"

#include "DataHubRecords.h"
//#include <Guid/DataHubRecords.h>

#define EFI_CPU_DATA_MAXIMUM_LENGTH 0x100

typedef union {
    EFI_CPU_DATA_RECORD *DataRecord;
    UINT8               *Raw;
} EFI_CPU_DATA_RECORD_BUFFER;

EFI_SUBCLASS_TYPE1_HEADER mCpuDataRecordHeader = {
    EFI_PROCESSOR_SUBCLASS_VERSION,       // Version
    sizeof (EFI_SUBCLASS_TYPE1_HEADER),   // Header Size
    0,                                    // Instance, Initialize later
    EFI_SUBCLASS_INSTANCE_NON_APPLICABLE, // SubInstance
    0                                     // RecordType, Initialize later
};

EFI_GUID gDataHubPlatformGuid = {
    0x64517cc8, 0x6561, 0x4051, {0xb0, 0x3c, 0x59, 0x64, 0xb6, 0x0f, 0x4c, 0x7a }
};

extern EFI_GUID gDataHubPlatformGuid;
#pragma pack(1)
typedef struct {
    EFI_SUBCLASS_TYPE1_HEADER   Hdr;			/* 0x48 */
    UINT32						NameLen;         /* 0x58 , in bytes */
    UINT32						ValLen;          /* 0x5c */
    UINT8						Data[1];         /* 0x60 Name Value */
} PLATFORM_DATA;
#pragma pack()

EFI_DATA_HUB_PROTOCOL					*gDataHub;


UINT32
CopyRecord(PLATFORM_DATA* Rec, const CHAR16* Name, VOID* Val, UINT32 ValLen)
{
	CopyMem(&Rec->Hdr, &mCpuDataRecordHeader, sizeof(EFI_SUBCLASS_TYPE1_HEADER));
    Rec->NameLen = (UINT32)StrLen(Name) * sizeof(CHAR16);
    Rec->ValLen = ValLen;
    CopyMem(Rec->Data, Name, Rec->NameLen);
    CopyMem(Rec->Data + Rec->NameLen, Val, ValLen);

    return (sizeof(EFI_SUBCLASS_TYPE1_HEADER) + 8 + Rec->NameLen + Rec->ValLen);
}

EFI_STATUS EFIAPI
LogDataHub(
           EFI_GUID					*TypeGuid,
           CHAR16                      *Name,
           VOID                        *Data,
           UINT32                       DataSize)
{
  UINT32                      RecordSize;
  EFI_STATUS                  Status;
	PLATFORM_DATA               *PlatformData;
	
  PlatformData = (PLATFORM_DATA*)AllocatePool (sizeof(PLATFORM_DATA) + DataSize + EFI_CPU_DATA_MAXIMUM_LENGTH);
  if (PlatformData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
	
  RecordSize = CopyRecord(PlatformData, Name, Data, DataSize);
  Status = gDataHub->LogData (
                              gDataHub,
                              TypeGuid,				/* DataRecordGuid */				
                              &gDataHubPlatformGuid,   /* ProducerName */  //always						   
                              EFI_DATA_RECORD_CLASS_DATA,
                              PlatformData,
                              RecordSize
                              );
  ASSERT_EFI_ERROR (Status);
	
  FreePool (PlatformData);
  return Status;
}

EFI_STATUS SetVariablesForOSX()
{
  EFI_STATUS  Status;
  
	UINT16		 *BootNext = NULL;	//it already presents in EFI FW. First GetVariable ?
  
	UINT32      BackgroundClear = 0x00000000;
	UINT32      FwFeatures      = 0x80000015; //Slice - get it from SMBIOS
	UINT32      FwFeaturesMask  = 0x800003ff;
	UINTN       bootArgsLen = 120; 
	CHAR8*      None	= "none";
	CHAR8*      BA = &gSettings.BootArgs[119];
  
	while ((*BA == ' ') || (*BA == 0)) {
		BA--; bootArgsLen--;
	}
	
	Status = gRS->SetVariable(L"BootNext",  &gEfiAppleNvramGuid, //&gEfiGlobalVarGuid,
                                         /*	EFI_VARIABLE_NON_VOLATILE | */EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                         sizeof(BootNext) ,&BootNext);
	Status = gRS->SetVariable(L"BackgroundClear", &gEfiAppleNvramGuid,
                                         /*	EFI_VARIABLE_NON_VOLATILE |*/ EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                         sizeof(BackgroundClear), &BackgroundClear);
	Status = gRS->SetVariable(L"FirmwareFeatures", &gEfiAppleNvramGuid,
                                         EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                         sizeof(FwFeatures),&FwFeatures);
	Status = gRS->SetVariable(L"FirmwareFeaturesMask", &gEfiAppleNvramGuid,
                                         EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                         sizeof(FwFeaturesMask), &FwFeaturesMask);
  
	Status = gRS->SetVariable(L"boot-args", &gEfiAppleBootGuid, 
                                         /*   EFI_VARIABLE_NON_VOLATILE |*/ EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                         bootArgsLen ,&gSettings.BootArgs);
	Status = gRS->SetVariable(L"security-mode", &gEfiAppleBootGuid, 
                                         /*   EFI_VARIABLE_NON_VOLATILE |*/ EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                         5 , (VOID*)None);
  // we should have two UUID: platform and system
  // NO! Only Platform is the best solution
	Status = gRS->SetVariable(L"platform-uuid", &gEfiAppleBootGuid, 
                                         /*   EFI_VARIABLE_NON_VOLATILE |*/ EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                         16 ,&gUuid);
  return Status;
}

VOID SetupDataForOSX()
{
	EFI_STATUS			Status;	
	CHAR16*				CloverVersion = L"2.0";
  
//	UINT32				KextListSize;
	UINT32				devPathSupportedVal = 1;
	UINT64				FrontSideBus		= gCPUStructure.FSBFrequency;
	UINT64				CpuSpeed        = gCPUStructure.CPUFrequency;
	UINT64				TSCFrequency		= gCPUStructure.TSCFrequency;
	CHAR16*				productName			= AllocateZeroPool(64);
	CHAR16*				serialNumber		= AllocateZeroPool(64);
	
  //fool proof
  if ((FrontSideBus < (50 * Mega)) ||  (FrontSideBus > (500 * Mega))){
    DBG("FrontSideBus=%d\n", FrontSideBus);
    FrontSideBus = 100 * Mega;
  }
  
	// Locate DataHub Protocol
	Status = gBS->LocateProtocol(&gEfiDataHubProtocolGuid, NULL, (VOID**)&gDataHub);
	if (!EFI_ERROR (Status)) 
	{
//		productName = EfiStrDuplicate(gSettings.ProductName);
//		serialNumber = EfiStrDuplicate(gSettings.SerialNr);
    AsciiStrToUnicodeStr(gSettings.ProductName, productName);
		AsciiStrToUnicodeStr(gSettings.SerialNr, serialNumber);
    
		
		Status =  LogDataHub(&gEfiProcessorSubClassGuid, L"FSBFrequency", &FrontSideBus, sizeof(UINT64));
		Status =  LogDataHub(&gEfiProcessorSubClassGuid, L"TSCFrequency", &TSCFrequency, sizeof(UINT64));
		Status =  LogDataHub(&gEfiProcessorSubClassGuid, L"CPUFrequency", &CpuSpeed, sizeof(UINT64));
		
		Status =  LogDataHub(&gEfiMiscSubClassGuid, L"DevicePathsSupported", &devPathSupportedVal, sizeof(UINT32));
		Status =  LogDataHub(&gEfiMiscSubClassGuid, L"Model", productName, StrSize(productName));
		Status =  LogDataHub(&gEfiMiscSubClassGuid, L"SystemSerialNumber", serialNumber, StrSize(serialNumber));
//    DBG("Custom UUID=%g\n", gUuid);
//    Status =  LogDataHub(&gEfiMiscSubClassGuid, L"system-id", &gUuid, sizeof(EFI_GUID));		
		Status =  LogDataHub(&gEfiMiscSubClassGuid, L"Clover", CloverVersion, StrSize(CloverVersion));
	}
}