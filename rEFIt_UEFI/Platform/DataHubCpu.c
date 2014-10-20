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

/*
  Removed commented out code in rev 2693
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

#include "Platform.h"
#include "Version.h"

#include <Guid/DataHubRecords.h>

#define EFI_CPU_DATA_MAXIMUM_LENGTH 0x100

EFI_DATA_HUB_PROTOCOL     *gDataHub;

EFI_SUBCLASS_TYPE1_HEADER mCpuDataRecordHeader = {
  EFI_PROCESSOR_SUBCLASS_VERSION,       // Version
  sizeof (EFI_SUBCLASS_TYPE1_HEADER),   // Header Size
  0,                                    // Instance, Initialize later
  EFI_SUBCLASS_INSTANCE_NON_APPLICABLE, // SubInstance
  0                                     // RecordType, Initialize later
};

EFI_GUID gDataHubPlatformGuid = {
  0x64517cc8, 0x6561, 0x4051, { 0xb0, 0x3c, 0x59, 0x64, 0xb6, 0x0f, 0x4c, 0x7a }
};

extern EFI_GUID gDataHubPlatformGuid;

typedef union {
  EFI_CPU_DATA_RECORD *DataRecord;
  UINT8               *Raw;
} EFI_CPU_DATA_RECORD_BUFFER;

#pragma pack(1)
typedef struct {
  EFI_SUBCLASS_TYPE1_HEADER Hdr;      /* 0x48 */
  UINT32                    NameLen;  /* 0x58 , in bytes */
  UINT32                    ValLen;   /* 0x5c */
  UINT8                     Data[1];  /* 0x60 Name Value */
} PLATFORM_DATA;
#pragma pack()

UINT32
CopyRecord (
  PLATFORM_DATA *Rec,
  const CHAR16  *Name,
  VOID          *Val,
  UINT32        ValLen
  )
{
  CopyMem (&Rec->Hdr, &mCpuDataRecordHeader, sizeof(EFI_SUBCLASS_TYPE1_HEADER));
  Rec->NameLen = (UINT32)StrLen (Name) * sizeof(CHAR16);
  Rec->ValLen  = ValLen;
  CopyMem (Rec->Data,                Name, Rec->NameLen);
  CopyMem (Rec->Data + Rec->NameLen, Val,  ValLen);

  return (sizeof(EFI_SUBCLASS_TYPE1_HEADER) + 8 + Rec->NameLen + Rec->ValLen);
}

EFI_STATUS
LogDataHub(EFI_GUID *TypeGuid,
  CHAR16  *Name,
  VOID    *Data,
  UINT32  DataSize
  )
{
  UINT32        RecordSize;
  EFI_STATUS    Status;
  PLATFORM_DATA *PlatformData;
  
  PlatformData = (PLATFORM_DATA*)AllocatePool (sizeof(PLATFORM_DATA) + DataSize + EFI_CPU_DATA_MAXIMUM_LENGTH);
  if (PlatformData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  RecordSize = CopyRecord (PlatformData, Name, Data, DataSize);
  Status = gDataHub->LogData (
                       gDataHub,
                       TypeGuid,                    /* DataRecordGuid */				
                       &gDataHubPlatformGuid,       /* ProducerName */  // always						   
                       EFI_DATA_RECORD_CLASS_DATA,
                       PlatformData,
                       RecordSize
                       );

 // ASSERT_EFI_ERROR (Status);
  
  FreePool (PlatformData);
  return Status;
}

EFI_STATUS
SetVariablesForOSX ()
{
  // The variable names used should be made global constants to prevent them being allocated multiple times

  UINT32 BackgroundClear;
  UINT32 FwFeaturesMask;

  CHAR8  *None;
  CHAR16 *KbdPrevLang;

  UINTN  LangLen;

//CHAR8  *VariablePtr;
  UINT32 Attributes;
  UINT32 FwAttributes;

  VOID   *OldData;

  //
  // firmware Runtime Variables
  //

  // As found on a real Mac, the system-id variable solely has the BS flag
  SetNvramVariable (
    L"system-id",
    &gEfiAppleNvramGuid,
    EFI_VARIABLE_BOOTSERVICE_ACCESS,
    sizeof(gUuid),
    &gUuid
    );

  FwAttributes = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
  
  if (gSettings.RtMLB != NULL) {
    if (AsciiStrLen (gSettings.RtMLB) != 17) {
      DBG ("** Warning: Your MLB is not suitable for iMessage (must be 17 chars long) !\n");
    }

	SetNvramVariable (
      L"MLB",
      &gEfiAppleNvramGuid,
      FwAttributes,
      AsciiStrLen(gSettings.RtMLB),
      gSettings.RtMLB
	  );
  }
  
  if (gSettings.RtROM != NULL) {
    SetNvramVariable (
      L"ROM",
      &gEfiAppleNvramGuid,
      FwAttributes,
      gSettings.RtROMLen,
      gSettings.RtROM
	  );
  }

  BackgroundClear = 0x00000000;
  AddNvramVariable (
    L"BackgroundClear",
    &gEfiAppleNvramGuid,
    FwAttributes,
    sizeof(BackgroundClear),
    &BackgroundClear
	);

  if (gFwFeaturesConfig) {
	  SetNvramVariable (
		  L"FirmwareFeatures",
		  &gEfiAppleNvramGuid,
          FwAttributes,
		  sizeof(gFwFeatures),
		  &gFwFeatures
		  );
  } else {
	AddNvramVariable (
      L"FirmwareFeatures",
      &gEfiAppleNvramGuid,
      FwAttributes,
      sizeof(gFwFeatures),
      &gFwFeatures
	  );
  }

  FwFeaturesMask  = 0xC003ffff;
  AddNvramVariable (
    L"FirmwareFeaturesMask",
    &gEfiAppleNvramGuid,
    FwAttributes,
    sizeof(FwFeaturesMask),
    &FwFeaturesMask
	);

  // reserved for a future. Should be tested on Yosemite
/*
  AddNvramVariable (L"HW_BID", &gEfiAppleNvramGuid, FwAttributes, AsciiStrLen (gSettings.BoardNumber), gSettings.BoardNumber);
*/

  //
  // OS X non-volatile Runtime Variables
  //

  // note: some gEfiAppleBootGuid vars present in nvram.plist are already set by PutNvramPlistToRtVars()
  // we should think how to handle those vars from nvram.plist and ones set here from gSettings

  if (!gFirmwareClover && !gDriversFlags.EmuVariableLoaded) {
    Attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
  } else {
    Attributes = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
  }

  None  = "none";
  AddNvramVariable (L"security-mode", &gEfiAppleBootGuid, Attributes, 5, (VOID*)None);

  // we should have two UUID: platform and system
  // NO! Only Platform is the best solution

  if (!gSettings.InjectSystemID) {
    if (gSettings.SmUUIDConfig) {
      SetNvramVariable (L"platform-uuid", &gEfiAppleBootGuid, Attributes, 16, &gUuid);
    } else {
      AddNvramVariable (L"platform-uuid", &gEfiAppleBootGuid, Attributes, 16, &gUuid);
    }
  }
  
  // Download-Fritz: Do not mess with BacklightLevel; it's OS X's business
  if (gMobile) {
    if (gSettings.BacklightLevelConfig) {
      SetNvramVariable (L"backlight-level", &gEfiAppleBootGuid, Attributes, sizeof(gSettings.BacklightLevel), &gSettings.BacklightLevel);
    } else {
      AddNvramVariable (L"backlight-level", &gEfiAppleBootGuid, Attributes, sizeof(gSettings.BacklightLevel), &gSettings.BacklightLevel);
    }
  }

  // using AddNvramVariable content instead of calling the function to do LangLen calculation only when necessary
  // Download-Fritz: Do not mess with prev-lang:kbd; it's OS X's business
/*
  KbdPrevLang     = L"prev-lang:kbd";
  OldData = GetNvramVariable (KbdPrevLang, &gEfiAppleBootGuid, NULL, NULL);
  if (OldData == NULL)
  {
    LangLen      = 16;
    VariablePtr  = &gSettings.Language[15];
    while (((*VariablePtr == ' ') || (*VariablePtr == 0)) && (LangLen != 0))
    {
      --VariablePtr;
      --LangLen;
    }

    gRT->SetVariable (KbdPrevLang, &gEfiAppleBootGuid, Attributes, LangLen, &gSettings.Language);
  } else {
    FreePool(OldData);
  }
*/
  return EFI_SUCCESS;
}

VOID
SetupDataForOSX ()
{
  EFI_STATUS Status;	
  
  UINT32     DevPathSupportedVal;
  UINT64     FrontSideBus;
  UINT64     CpuSpeed;
  UINT64     TSCFrequency;
  CHAR16*    ProductName;
  CHAR16*    SerialNumber;
  UINTN      Revision;

#ifdef FIRMWARE_REVISION
  Revision = StrDecimalToUintn (FIRMWARE_REVISION);
#else
  Revision = StrDecimalToUintn (gST->FirmwareRevision);
#endif

  // fool proof
  FrontSideBus = gCPUStructure.FSBFrequency;
  if ((FrontSideBus < (50 * Mega)) || (FrontSideBus > (1000 * Mega))){
    DBG ("Wrong FrontSideBus=%d, set to 100MHz\n", FrontSideBus);
    FrontSideBus = 100 * Mega;
  }

  // Save values into gSettings for the genconfig aim
  gSettings.BusSpeed   = (UINT32)DivU64x32 (FrontSideBus, kilo);

  CpuSpeed = gCPUStructure.CPUFrequency;
  gSettings.CpuFreqMHz = (UINT32)DivU64x32 (CpuSpeed,     Mega);
  
  // Locate DataHub Protocol
  Status = gBS->LocateProtocol (&gEfiDataHubProtocolGuid, NULL, (VOID**)&gDataHub);
  if (!EFI_ERROR (Status)) {
    ProductName         = AllocateZeroPool (64);
    AsciiStrToUnicodeStr (gSettings.ProductName, ProductName);

    SerialNumber        = AllocateZeroPool (64);
    AsciiStrToUnicodeStr (gSettings.SerialNr,    SerialNumber);   
    
    LogDataHub (&gEfiProcessorSubClassGuid, L"FSBFrequency",         &FrontSideBus,        sizeof(UINT64));

    TSCFrequency        = gCPUStructure.TSCFrequency;
    LogDataHub (&gEfiProcessorSubClassGuid, L"TSCFrequency",         &TSCFrequency,        sizeof(UINT64));
    LogDataHub (&gEfiProcessorSubClassGuid, L"CPUFrequency",         &CpuSpeed,            sizeof(UINT64));
    
    DevPathSupportedVal = 1;
    LogDataHub (&gEfiMiscSubClassGuid,      L"DevicePathsSupported", &DevPathSupportedVal, sizeof(UINT32));
    LogDataHub (&gEfiMiscSubClassGuid,      L"Model",                ProductName,          (UINT32)StrSize (ProductName));
    LogDataHub (&gEfiMiscSubClassGuid,      L"SystemSerialNumber",   SerialNumber,         (UINT32)StrSize (SerialNumber));

    if (gSettings.InjectSystemID) {
      LogDataHub (&gEfiMiscSubClassGuid, L"system-id", &gUuid, sizeof(EFI_GUID));
    }    		

    LogDataHub (&gEfiProcessorSubClassGuid, L"clovergui-revision", &Revision, sizeof(UINT32));

    // collect info about real hardware
    LogDataHub (&gEfiMiscSubClassGuid, L"OEMVendor",  &gSettings.OEMVendor,  (UINT32)iStrLen (gSettings.OEMVendor, 64)  + 1);
    LogDataHub (&gEfiMiscSubClassGuid, L"OEMProduct", &gSettings.OEMProduct, (UINT32)iStrLen (gSettings.OEMProduct, 64) + 1);
    LogDataHub (&gEfiMiscSubClassGuid, L"OEMBoard",   &gSettings.OEMBoard,   (UINT32)iStrLen (gSettings.OEMBoard, 64)   + 1);
    
    // SMC helper
    LogDataHub (&gEfiMiscSubClassGuid, L"RPlt", &gSettings.RPlt,   8);
    LogDataHub (&gEfiMiscSubClassGuid, L"RBr",  &gSettings.RBr,    8);
    LogDataHub (&gEfiMiscSubClassGuid, L"EPCI", &gSettings.EPCI,   4);
    LogDataHub (&gEfiMiscSubClassGuid, L"REV",  &gSettings.REV,    6);
    LogDataHub (&gEfiMiscSubClassGuid, L"BEMB", &gSettings.Mobile, 1);
    
    // all current settings
    LogDataHub (&gEfiMiscSubClassGuid, L"Settings", &gSettings, sizeof(gSettings));
  } else {
    // this is the error message that we want user to see on the screen!
    Print (L"DataHubProtocol is not found! Load the module DataHubDxe manually!\n");
    DBG ("DataHubProtocol is not found! Load the module DataHubDxe manually!\n");
    gBS->Stall (5000000);
  }  
}
