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

#ifndef DEBUG_ALL
#define DEBUG_DH 1
#else
#define DEBUG_DH DEBUG_ALL
#endif

#if DEBUG_DH == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_DH, __VA_ARGS__)	
#endif


/*******************************************************************************
 *   Header Files                                                               *
 *******************************************************************************/
//#include <Framework/FrameworkInternalFormRepresentation.h>
#include "Platform.h"
#include "Version.h"

//#include "DataHubRecords.h"
#include <Guid/DataHubRecords.h>

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

EFI_STATUS
LogDataHub(
           EFI_GUID					  *TypeGuid,
           CHAR16             *Name,
           VOID               *Data,
           UINT32             DataSize)
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
 // ASSERT_EFI_ERROR (Status);
	
  FreePool (PlatformData);
  return Status;
}

EFI_STATUS SetVariablesForOSX()
{
//  EFI_STATUS  Status;
  
	UINT32      BackgroundClear = 0x00000000;
	UINT32      FwFeatures      = gFwFeatures; //0x80001417; //Slice - get it from SMBIOS
	UINT32      FwFeaturesMask  = 0xC003ffff;
	CHAR8*      None	= "none";
	CHAR8       Buffer[32];
  UINTN       SNLen = 20;
	UINTN       bootArgsLen = 256;
  UINTN       LangLen = 16;

//  CHAR8*      FmmName = &gSettings.FamilyName[0];
//  UINTN       FmmLen  = AsciiStrLen(FmmName);
  UINT16      BacklightLevel = 0x0503;
  CHAR8*      BA = &gSettings.BootArgs[255];

	while (((*BA == ' ') || (*BA == 0)) && (bootArgsLen != 0)) {
		BA--; bootArgsLen--;
	}
  BA = &gSettings.Language[15];
  while (((*BA == ' ') || (*BA == 0)) && (LangLen != 0)) {
		BA--; LangLen--;
	}
  BA = &gSettings.BoardSerialNumber[19];
  while (((*BA == ' ') || (*BA == 0)) && (SNLen != 0)) {
		BA--; SNLen--;
	}
  
  if (gSettings.RtMLB == NULL && SNLen > 0) {
    gSettings.RtMLB = AllocateCopyPool(SNLen + 1, gSettings.BoardSerialNumber);
  }
  
  if (gSettings.RtROM == NULL) {
    // we can try to set it to MAC address from SMBIOS UUID - some boards have it there
    gSettings.RtROMLen = 8;
    gSettings.RtROM = AllocateCopyPool(gSettings.RtROMLen, ((UINT8*)&gSettings.SmUUID) + 8);
  }
  
  //
  // some NVRAM variables
  //
  
	/*Status = */gRS->SetVariable(L"BackgroundClear", &gEfiAppleNvramGuid,
                                         /*	EFI_VARIABLE_NON_VOLATILE |*/ EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                         sizeof(BackgroundClear), &BackgroundClear);
	/*Status = */gRS->SetVariable(L"FirmwareFeatures", &gEfiAppleNvramGuid,
                                         /*	EFI_VARIABLE_NON_VOLATILE |*/ EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                         sizeof(FwFeatures), &FwFeatures);
	/*Status = */gRS->SetVariable(L"FirmwareFeaturesMask", &gEfiAppleNvramGuid,
                                         /*	EFI_VARIABLE_NON_VOLATILE |*/ EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                         sizeof(FwFeaturesMask), &FwFeaturesMask);

// should set anyway  
  /*Status = */gRS->SetVariable(L"MLB", &gEfiAppleNvramGuid,
                            /*	EFI_VARIABLE_NON_VOLATILE |*/ EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                            AsciiStrLen(gSettings.RtMLB), gSettings.RtMLB);
 
  /*Status = */gRS->SetVariable(L"ROM", &gEfiAppleNvramGuid,
                            /*	EFI_VARIABLE_NON_VOLATILE |*/ EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                            gSettings.RtROMLen, gSettings.RtROM);
  
  /*Status = */gRS->SetVariable(L"system-id", &gEfiAppleNvramGuid,
                            /*	EFI_VARIABLE_NON_VOLATILE |*/ EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                            sizeof(EFI_GUID), &gUuid);

// reserved for a future. Should be tested on Yosemite
/*  Status = gRS->SetVariable(L"HW_BID", &gEfiAppleNvramGuid,
                            EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                            AsciiStrLen(gSettings.BoardNumber), gSettings.BoardNumber);

*/
    // options variables
    // note: some gEfiAppleBootGuid vars present in nvram.plist are already set by PutNvramPlistToRtVars()
    // we should think how to handle those vars from nvram.plist and ones set here from gSettings

    // Don't overwrite boot-args var as it was already set by PutNvramPlistToRtVars()
    // boot-args nvram var contain ONLY parameters to be merged with the boot-args global variable
 /*Status = */gRS->SetVariable(L"boot-args", &gEfiAppleBootGuid,
                            /*   EFI_VARIABLE_NON_VOLATILE |*/ EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                            bootArgsLen , NULL); //&gSettings.BootArgs);


	/*Status = */gRS->SetVariable(L"security-mode", &gEfiAppleBootGuid,
                                         /*   EFI_VARIABLE_NON_VOLATILE |*/ EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                         AsciiStrLen(None), (VOID*)None);
  
  // we should have two UUID: platform and system
  // NO! Only Platform is the best solution
  if (!gSettings.InjectSystemID) {
    /*Status = */gRS->SetVariable(L"platform-uuid", &gEfiAppleBootGuid,
                              /*   EFI_VARIABLE_NON_VOLATILE |*/ EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                              16, &gUuid);
  }
  
  /*Status = */gRS->SetVariable(L"prev-lang:kbd", &gEfiAppleBootGuid,
                            /*   EFI_VARIABLE_NON_VOLATILE |*/ EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                LangLen, &gSettings.Language);
  
  
  if (gMobile && (gSettings.BacklightLevel != 0xFFFF)) {
    /*Status = */gRS->SetVariable(L"backlight-level", &gEfiAppleBootGuid, 
                              /*   EFI_VARIABLE_NON_VOLATILE |*/ EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                              sizeof(BacklightLevel), &gSettings.BacklightLevel);    
  }
  
  //Helper for rc.local script
  AsciiSPrint(Buffer, sizeof(Buffer), "%d", gSettings.LogLineCount);
  /*Status = */gRS->SetVariable(L"Clover.LogLineCount", &gEfiAppleBootGuid, 
                            EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                            AsciiStrLen(Buffer), Buffer);

  if (gSettings.LogEveryBoot) { //not NULL
    /*Status = */gRS->SetVariable(L"Clover.LogEveryBoot", &gEfiAppleBootGuid, 
                              EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                              AsciiStrLen(gSettings.LogEveryBoot), gSettings.LogEveryBoot);
  }

  if (gSettings.MountEFI) { //not NULL
    /*Status = */gRS->SetVariable(L"Clover.MountEFI", &gEfiAppleBootGuid, 
                              EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                              AsciiStrLen(gSettings.MountEFI), gSettings.MountEFI);
  }

  return EFI_SUCCESS;
}

VOID SetupDataForOSX()
{
  EFI_STATUS			Status;	
//	CHAR16*				CloverVersion = L"2.1";
  
	UINT32				devPathSupportedVal = 1;
	UINT64				FrontSideBus		= gCPUStructure.FSBFrequency;
	UINT64				CpuSpeed        = gCPUStructure.CPUFrequency;
	UINT64				TSCFrequency		= gCPUStructure.TSCFrequency;
	CHAR16*				productName			= AllocateZeroPool(64);
	CHAR16*				serialNumber		= AllocateZeroPool(64);
//  UINT32        Size;
  UINTN         revision;

 //revision = StrDecimalToUintn(
#ifdef FIRMWARE_REVISION
  revision = StrDecimalToUintn(FIRMWARE_REVISION);
#else
  revision = StrDecimalToUintn(gST->FirmwareRevision);
#endif

  //fool proof
  if ((FrontSideBus < (50 * Mega)) ||  (FrontSideBus > (1000 * Mega))){
    DBG("Wrong FrontSideBus=%d, set to 100MHz\n", FrontSideBus);
    FrontSideBus = 100 * Mega;
  }

  //Save values into gSettings for the genconfig aim
  gSettings.BusSpeed = (UINT32)DivU64x32(FrontSideBus, kilo);
  gSettings.CpuFreqMHz = (UINT32)DivU64x32(CpuSpeed, Mega);
  
  
	// Locate DataHub Protocol
	Status = gBS->LocateProtocol(&gEfiDataHubProtocolGuid, NULL, (VOID**)&gDataHub);
	if (!EFI_ERROR (Status)) {
      AsciiStrToUnicodeStr(gSettings.ProductName, productName);
		AsciiStrToUnicodeStr(gSettings.SerialNr, serialNumber);
    
		
		/*Status = */LogDataHub(&gEfiProcessorSubClassGuid, L"FSBFrequency", &FrontSideBus, sizeof(UINT64));
		/*Status = */LogDataHub(&gEfiProcessorSubClassGuid, L"TSCFrequency", &TSCFrequency, sizeof(UINT64));
		/*Status = */LogDataHub(&gEfiProcessorSubClassGuid, L"CPUFrequency", &CpuSpeed, sizeof(UINT64));
		
		/*Status = */LogDataHub(&gEfiMiscSubClassGuid, L"DevicePathsSupported", &devPathSupportedVal, sizeof(UINT32));
		/*Status = */LogDataHub(&gEfiMiscSubClassGuid, L"Model", productName, (UINT32)StrSize(productName));
		/*Status = */LogDataHub(&gEfiMiscSubClassGuid, L"SystemSerialNumber", serialNumber, (UINT32)StrSize(serialNumber));
//    DBG("Custom UUID=%g\n", gUuid);
    if (gSettings.InjectSystemID) {
      /*Status = */LogDataHub(&gEfiMiscSubClassGuid, L"system-id", &gUuid, sizeof(EFI_GUID));
    }    		
//		Status = LogDataHub(&gEfiMiscSubClassGuid, L"Clover", CloverVersion, StrSize(CloverVersion));

    /*Status = */LogDataHub(&gEfiProcessorSubClassGuid, L"clovergui-revision", &revision, sizeof(UINT32));

    //collect info about real hardware
    /*Status = */LogDataHub(&gEfiMiscSubClassGuid, L"OEMVendor",  &gSettings.OEMVendor,  (UINT32)iStrLen(gSettings.OEMVendor, 64) + 1);
    /*Status = */LogDataHub(&gEfiMiscSubClassGuid, L"OEMProduct", &gSettings.OEMProduct, (UINT32)iStrLen(gSettings.OEMProduct, 64) + 1);
    /*Status = */LogDataHub(&gEfiMiscSubClassGuid, L"OEMBoard",   &gSettings.OEMBoard,   (UINT32)iStrLen(gSettings.OEMBoard, 64) + 1);
    //smc helper
    /*Status = */LogDataHub(&gEfiMiscSubClassGuid, L"RPlt", &gSettings.RPlt, 8);
    /*Status = */LogDataHub(&gEfiMiscSubClassGuid, L"RBr",  &gSettings.RBr,  8);
    /*Status = */LogDataHub(&gEfiMiscSubClassGuid, L"EPCI", &gSettings.EPCI, 4);
    /*Status = */LogDataHub(&gEfiMiscSubClassGuid, L"REV",  &gSettings.REV,  6);
    /*Status = */LogDataHub(&gEfiMiscSubClassGuid, L"BEMB", &gSettings.Mobile, 1);
    //all current settings
    /*Status = */LogDataHub(&gEfiMiscSubClassGuid, L"Settings", &gSettings, sizeof(gSettings));

	}
  else {
    // this is the error message that we want user to see on the screen!
    Print(L"DataHubProtocol is not found! Load the module DataHubDxe manually!\n");
    DBG("DataHubProtocol is not found! Load the module DataHubDxe manually!\n");
    gBS->Stall(5000000);
  }  
}
