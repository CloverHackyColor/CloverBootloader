/** @file

  RuntimeServices overrides module.

  By dmazar, 26/09/2012

**/

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Guid/EventGroup.h>
#include <Protocol/DataHub.h>

#include "Common.h"


/** Original runtime services. */
EFI_RUNTIME_SERVICES gOrgRS;


/** Virtual address change event. */
EFI_EVENT   gVirtualAddressChangeEvent = NULL;


////////////////////////////////////////////
// 
// Runtime services overrides
//

EFI_STATUS EFIAPI
OvrGetTime(
	OUT EFI_TIME				*Time,
	OUT EFI_TIME_CAPABILITIES	*Capabilities
)
{
	EFI_STATUS					Status;
	
	Status = gOrgRS.GetTime(Time, Capabilities);
	if (Capabilities != NULL) {
		PRINT("->GetTime(%t, {Res = %x, Acc = %x, To0: %c}) = %r\n",
			Time,
			Capabilities->Resolution, Capabilities->Accuracy, Capabilities->SetsToZero ? L'T' : 'F',
			Status);
	} else {
		PRINT("->GetTime(%t, NULL) = %r\n", Time, Status);
	}
	return Status;
}

EFI_STATUS EFIAPI
OvrSetTime(
	IN EFI_TIME			*Time
)
{
	EFI_STATUS			Status;
	
	Status = gOrgRS.SetTime(Time);
	PRINT("->SetTime(%t) = %r\n", Time, Status);
	return Status;
}


EFI_STATUS EFIAPI
OvrGetWakeupTime(
	OUT BOOLEAN			*Enabled,
	OUT BOOLEAN			*Pending,
	OUT EFI_TIME		*Time
)
{
	EFI_STATUS					Status;
	
	Status = gOrgRS.GetWakeupTime(Enabled, Pending, Time);
	PRINT("->GetWakeupTime(%c, %c, %t) = %r\n", *Enabled ? L'T' : 'F', *Pending ? L'T' : 'F', Time, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrSetWakeupTime(
	IN BOOLEAN			Enabled,
	IN EFI_TIME			*Time
)
{
	EFI_STATUS					Status;
	
	Status = gOrgRS.SetWakeupTime(Enabled, Time);
	PRINT("->SetWakeupTime(%c, %t) = %r\n", Enabled ? L'T' : 'F', Time, Status);
	return Status;
}


EFI_STATUS EFIAPI
OvrSetVirtualAddressMap(
	IN UINTN			MemoryMapSize,
	IN UINTN			DescriptorSize,
	IN UINT32			DescriptorVersion,
	IN EFI_MEMORY_DESCRIPTOR	*VirtualMap
)
{
	EFI_STATUS			Status;
	
	PRINT("->SetVirtualAddressMap(%d, %d, 0x%x, %p) START ...\n", MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap);
	//PrintSystemTable(gST);
	//PrintMemMap(MemoryMapSize, VirtualMap, DescriptorSize, DescriptorVersion);
	Status = gOrgRS.SetVirtualAddressMap(MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap);
	
	// Prints beyond this point may cause hangs, so don't print anything
	//PRINT("->SetVirtualAddressMap(%d, %d, 0x%x, %p) END = %r\n", MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap, Status);
	//PrintSystemTable(gST);
	
	return Status;
}

EFI_STATUS EFIAPI
OvrConvertPointer(
	IN UINTN			DebugDisposition,
	IN OUT VOID			**Address
)
{
	EFI_STATUS			Status;
	// We should call PRINT before actual convertion takes place, as otherwise we may call it with converted buffer
	VOID				*AddressOut = *Address;
	
	Status = gOrgRS.ConvertPointer(DebugDisposition, &AddressOut);
	PRINT("->ConvertPointer(%d, %p/%p) = %r\n", DebugDisposition, *Address, AddressOut, Status);
	*Address = AddressOut;
	
	/*
	VOID				*AddressIn = *Address;
	
	Status = gOrgRS.ConvertPointer(DebugDisposition, Address);
	PRINT("->ConvertPointer(%d, %p/%p) = %r\n", DebugDisposition, AddressIn, *Address, Status);
	*/
	
	return Status;
}


EFI_STATUS EFIAPI
OvrGetVariable(
	IN CHAR16			*VariableName,
	IN EFI_GUID			*VendorGuid,
	OUT UINT32			*Attributes,
	IN OUT UINTN		*DataSize,
	OUT VOID			*Data
)
{
	EFI_STATUS			Status;
	UINT32				OurAttributes;
	
	
	//
	// Attributes could be NULL, but it's still good
	// to get and see them.
	//
	if (Attributes != NULL) {
		OurAttributes = *Attributes;
	}
	Status = gOrgRS.GetVariable(VariableName, VendorGuid, &OurAttributes, DataSize, Data);
	if (Attributes != NULL) {
		*Attributes = OurAttributes;
	}
	
	#if CLEANER_LOG == 1
	// Better not print this to avoid EfiTime "SPAM" on some AMI firmware - reported by XyZ
	if (StrStr(VariableName, L"EfiTime") == 0) {
		PRINT("->GetVariable(%s, %s, %x/%x, %x, %p) = %r\n",
		        VariableName, GuidStr(VendorGuid), Attributes != NULL ? *Attributes : 0, OurAttributes, *DataSize, Data, Status);
	}
	#else
	PRINT("->GetVariable(%s, %s, %x/%x, %x, %p) = %r\n",
		VariableName, GuidStr(VendorGuid), Attributes != NULL ? *Attributes : 0, OurAttributes, *DataSize, Data, Status);
	//PRINT("->GetVariable(%s)\n", VariableName);
	#endif
	
	if (!EFI_ERROR(Status)) {
		PrintBytes((CHAR8 *)Data, *DataSize);
	}
	return Status;
}

EFI_STATUS EFIAPI
OvrGetNextVariableName(
	IN OUT UINTN		*VariableNameSize,
	IN OUT CHAR16		*VariableName,
	IN OUT EFI_GUID		*VendorGuid
)
{
	EFI_STATUS			Status;
	
	PRINT("->GetNextVariableName(%x, %s, %s)", *VariableNameSize, VariableName, GuidStr(VendorGuid));
	Status = gOrgRS.GetNextVariableName(VariableNameSize, VariableName, VendorGuid);
	PRINT(" -> (%x, %s, %s) = %r\n", *VariableNameSize, VariableName, GuidStr(VendorGuid), Status);
	//PRINT("->GetNextVariableName()\n");
	return Status;
}

EFI_STATUS EFIAPI
OvrSetVariable(
	IN CHAR16			*VariableName,
	IN EFI_GUID			*VendorGuid,
	IN UINT32			Attributes,
	IN UINTN			DataSize,
	IN VOID				*Data
)
{
	EFI_STATUS			Status;
	
	Status = gOrgRS.SetVariable(VariableName, VendorGuid, Attributes, DataSize, Data);
	//PRINT("->SetVariable(%s)\n", VariableName);
	PRINT("->SetVariable(%s, %s, %x, %x, %p) = %r\n", VariableName, GuidStr(VendorGuid), Attributes, DataSize, Data, Status);
	PrintBytes((CHAR8 *)Data, DataSize);
	//PRINT("->SetVariable(%s)\n", VariableName);
	return Status;
}


EFI_STATUS EFIAPI
OvrGetNextHighMonotonicCount(
	OUT UINT32			*HighCount
)
{
	EFI_STATUS			Status;
	
	Status = gOrgRS.GetNextHighMonotonicCount(HighCount);
	PRINT("->GetNextHighMonotonicCount(%x) = %r\n", *HighCount, Status);
	return Status;
}


VOID EFIAPI
OvrResetSystem(
	IN EFI_RESET_TYPE	ResetType,
	IN EFI_STATUS		ResetStatus,
	IN UINTN			DataSize,
	IN VOID				*ResetData
)
{
	
	PRINT("->ResetSystem(%s, %r, %x, %p)\n", EfiResetType[ResetType], ResetStatus, DataSize, ResetData);
	PrintBytes((CHAR8 *)ResetData, DataSize);
//	gOrgRS.ResetSystem(ResetType, ResetStatus, DataSize, ResetData);
}


EFI_STATUS EFIAPI
OvrUpdateCapsule(
	IN EFI_CAPSULE_HEADER	**CapsuleHeaderArray,
	IN UINTN				CapsuleCount,
	IN EFI_PHYSICAL_ADDRESS	ScatterGatherList
)
{
	EFI_STATUS			Status;
	
	Status = gOrgRS.UpdateCapsule(CapsuleHeaderArray, CapsuleCount, ScatterGatherList);
	PRINT("->UpdateCapsule(%p, %x, %lx) = %r\n", CapsuleHeaderArray, CapsuleCount, ScatterGatherList, Status);
	return Status;
}


EFI_STATUS EFIAPI
OvrQueryCapsuleCapabilities(
	IN EFI_CAPSULE_HEADER	**CapsuleHeaderArray,
	IN UINTN				CapsuleCount,
	OUT UINT64				*MaximumCapsuleSize,
	OUT EFI_RESET_TYPE		*ResetType
)
{
	EFI_STATUS			Status;
	
	Status = gOrgRS.QueryCapsuleCapabilities(CapsuleHeaderArray, CapsuleCount, MaximumCapsuleSize, ResetType);
	PRINT("->QueryCapsuleCapabilities(%p, %x, %lx, %s) = %r\n",
		CapsuleHeaderArray, CapsuleCount, *MaximumCapsuleSize, EfiResetType[*ResetType], Status);
	return Status;
}


EFI_STATUS EFIAPI
OvrQueryVariableInfo(
	IN UINT32			Attributes,
	OUT UINT64			*MaximumVariableStorageSize,
	OUT UINT64			*RemainingVariableStorageSize,
	OUT UINT64			*MaximumVariableSize
)
{
	EFI_STATUS			Status;
	
	Status = gOrgRS.QueryVariableInfo(Attributes, MaximumVariableStorageSize, RemainingVariableStorageSize, MaximumVariableSize);
	PRINT("->QueryVariableInfo(%x, %lx, %lx, %lx) = %r\n",
		Attributes, *MaximumVariableStorageSize, *RemainingVariableStorageSize, *MaximumVariableSize, Status);
	return Status;
}


/** Virtual address change event. Converts our pointers
 *  to be valid in runtime with new virtual addresses.
 */
VOID
EFIAPI
VirtualAddressChangeEvent(
	IN EFI_EVENT		Event,
	IN VOID				*Context
)
{
	PRINT("VirtualAddressChangeEvent\n");
	
	//
	// Convert original RT services so we cam continue use them.
	//
	OvrConvertPointer(EFI_OPTIONAL_PTR, (VOID **) &gOrgRS.GetTime);
	OvrConvertPointer(EFI_OPTIONAL_PTR, (VOID **) &gOrgRS.SetTime);
	OvrConvertPointer(EFI_OPTIONAL_PTR, (VOID **) &gOrgRS.GetWakeupTime);
	OvrConvertPointer(EFI_OPTIONAL_PTR, (VOID **) &gOrgRS.SetWakeupTime);
	
	OvrConvertPointer(EFI_OPTIONAL_PTR, (VOID **) &gOrgRS.GetVariable);
	OvrConvertPointer(EFI_OPTIONAL_PTR, (VOID **) &gOrgRS.GetNextVariableName);
	OvrConvertPointer(EFI_OPTIONAL_PTR, (VOID **) &gOrgRS.SetVariable);
	
	OvrConvertPointer(EFI_OPTIONAL_PTR, (VOID **) &gOrgRS.GetNextHighMonotonicCount);
	OvrConvertPointer(EFI_OPTIONAL_PTR, (VOID **) &gOrgRS.ResetSystem);
	OvrConvertPointer(EFI_OPTIONAL_PTR, (VOID **) &gOrgRS.UpdateCapsule);
	OvrConvertPointer(EFI_OPTIONAL_PTR, (VOID **) &gOrgRS.QueryCapsuleCapabilities);
	
	OvrConvertPointer(EFI_OPTIONAL_PTR, (VOID **) &gOrgRS.QueryVariableInfo);
	
	//
	// Convert our pointers to allocated buffers.
	//
	OvrConvertPointer(EFI_OPTIONAL_PTR, (VOID **) &GuidPrintBuffer);
	OvrConvertPointer(EFI_OPTIONAL_PTR, (VOID **) &gVariableNameBuffer);
	OvrConvertPointer(EFI_OPTIONAL_PTR, (VOID **) &gVariableDataBuffer);
	// LogPrint's buffer should be converted last, as otherwise LogPrint may be called with incorrect pointer
	OvrConvertPointer(EFI_OPTIONAL_PTR, (VOID **) &gLogLineBuffer);
}


////////////////////////////////////////////
// 
// Other functions
//

/** Installs our runtime services overrides. */
EFI_STATUS EFIAPI
OvrRuntimeServices(EFI_RUNTIME_SERVICES	*RS)
{
	EFI_STATUS Status = EFI_SUCCESS;
	
	PRINT("Overriding runtime services ...\n");
	
	// store orig BS
	CopyMem(&gOrgRS, RS, sizeof(EFI_RUNTIME_SERVICES));
	
	RS->GetTime = OvrGetTime;
	RS->SetTime = OvrSetTime;
	RS->GetWakeupTime = OvrGetWakeupTime;
	RS->SetWakeupTime = OvrSetWakeupTime;
	
	RS->SetVirtualAddressMap = OvrSetVirtualAddressMap;
	RS->ConvertPointer = OvrConvertPointer;
	
	RS->GetVariable = OvrGetVariable;
	RS->GetNextVariableName = OvrGetNextVariableName;
	RS->SetVariable = OvrSetVariable;
	
	RS->GetNextHighMonotonicCount = OvrGetNextHighMonotonicCount;
	RS->ResetSystem = OvrResetSystem;
	RS->UpdateCapsule = OvrUpdateCapsule;
	RS->QueryCapsuleCapabilities = OvrQueryCapsuleCapabilities;

	RS->QueryVariableInfo = OvrQueryVariableInfo;
	
	RS->Hdr.CRC32 = 0;
	gBS->CalculateCrc32(RS, RS->Hdr.HeaderSize, &RS->Hdr.CRC32);
	
	#if WORK_DURING_RUNTIME >= 1
	//
	// Register callback for EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE
	//
	#if WORK_DURING_RUNTIME == 2
	// new style - does not work on Phoenix UEFI
	Status = gBS->CreateEventEx (
                               EVT_NOTIFY_SIGNAL,
                               TPL_NOTIFY,
                               VirtualAddressChangeEvent,
                               NULL,
                               &gEfiEventVirtualAddressChangeGuid,
                               &gVirtualAddressChangeEvent
                               );
	#elif WORK_DURING_RUNTIME == 1
	// old style
	Status = gBS->CreateEvent (
							   EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE,
							   TPL_NOTIFY,
							   VirtualAddressChangeEvent,
							   NULL,
							   &gVirtualAddressChangeEvent
							   );
	#endif
	PRINT("Runtime services: setting VirtualAddressChangeEvent = %r\n", Status);
	#endif
	
	PRINT("Runtime services overriden!\n");
	
	return Status;
}

/** Restores original runtime services. */
EFI_STATUS EFIAPI
RestoreRuntimeServices(EFI_RUNTIME_SERVICES	*RS)
{
	
	PRINT("Restoring original runtime services ...\n");
	
	RS->GetTime = gOrgRS.GetTime;
	RS->SetTime = gOrgRS.SetTime;
	RS->GetWakeupTime = gOrgRS.GetWakeupTime;
	RS->SetWakeupTime = gOrgRS.SetWakeupTime;
	
	RS->SetVirtualAddressMap = gOrgRS.SetVirtualAddressMap;
	RS->ConvertPointer = gOrgRS.ConvertPointer;
	
	RS->GetVariable = gOrgRS.GetVariable;
	RS->GetNextVariableName = gOrgRS.GetNextVariableName;
	RS->SetVariable = gOrgRS.SetVariable;
	
	RS->GetNextHighMonotonicCount = gOrgRS.GetNextHighMonotonicCount;
	RS->ResetSystem = gOrgRS.ResetSystem;
	RS->UpdateCapsule = gOrgRS.UpdateCapsule;
	RS->QueryCapsuleCapabilities = gOrgRS.QueryCapsuleCapabilities;

	RS->QueryVariableInfo = gOrgRS.QueryVariableInfo;
	
	RS->Hdr.CRC32 = 0;
	gBS->CalculateCrc32(RS, RS->Hdr.HeaderSize, &RS->Hdr.CRC32);
	
	PRINT("Runtime services restored!\n");
	
	return EFI_SUCCESS;
}
