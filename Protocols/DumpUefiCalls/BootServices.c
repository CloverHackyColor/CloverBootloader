/** @file

  BootServices overrides module.

  By dmazar, 26/09/2012

**/

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include "Common.h"


/** Flag is TRUE before ExitBootServices() is called. FALSE in runtime. */
BOOLEAN InBootServices = TRUE;

/** Original boot services. */
EFI_BOOT_SERVICES gOrgBS;

#if CAPTURE_CONSOLE_OUTPUT >= 1
EFI_TEXT_STRING gOrgConOutOutputString = 0;
BOOLEAN InConsolePrint = FALSE;
#endif

/** Helper function that calls GetMemoryMap() and returns new MapKey.
 * Uses gOrgBS.GetMemoryMap to avoid our log PRINT.
 */
EFI_STATUS
GetMemoryMapKey(OUT UINTN *MapKey)
{
	EFI_STATUS					Status;
	UINTN						MemoryMapSize;
	EFI_MEMORY_DESCRIPTOR		*MemoryMap;
	UINTN						DescriptorSize;
	UINT32						DescriptorVersion;
	
	Status = GetMemoryMapAlloc(gOrgBS.GetMemoryMap, &MemoryMapSize, &MemoryMap, MapKey, &DescriptorSize, &DescriptorVersion);
	return Status;
}


////////////////////////////////////////////
// 
// Boot services overrides
//

EFI_TPL EFIAPI
OvrRaiseTPL(IN EFI_TPL NewTpl)
{
	EFI_TPL				Status;
	
	Status = gOrgBS.RaiseTPL(NewTpl);
	// do not print - it's called by UEFI events and timers (from timer interrupts) many times
	//PRINT("->RaiseTPL(NewTpl=%d) = %d\n", NewTpl, Status);
	return Status;
}

VOID EFIAPI
OvrRestoreTPL(IN EFI_TPL OldTpl)
{
	gOrgBS.RestoreTPL(OldTpl);
	// do not print - it's called by UEFI events and timers (from timer interrupts) many times
	//PRINT("->RestoreTPL(OldTpl=%d)\n", OldTpl);
	return;
}

EFI_STATUS EFIAPI
OvrAllocatePages(
	IN EFI_ALLOCATE_TYPE		Type,
	IN EFI_MEMORY_TYPE			MemoryType,
	IN UINTN					NumberOfPages,
	IN OUT EFI_PHYSICAL_ADDRESS	*Memory
)
{
	EFI_STATUS			Status;
//	EFI_PHYSICAL_ADDRESS	inMemory = *Memory;
	
	Status = gOrgBS.AllocatePages(Type, MemoryType, NumberOfPages, Memory);
//	PRINT("-> AllocatePages(%s, %s, 0x%x, 0x%lx/0x%lx) = %r\n", EfiAllocateTypeDesc[Type], EfiMemoryTypeDesc[MemoryType], NumberOfPages, inMemory, *Memory, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrFreePages(
	IN EFI_PHYSICAL_ADDRESS Memory,
	IN UINTN Pages
)  
{
	EFI_STATUS			Status;
	
	Status = gOrgBS.FreePages(Memory, Pages);
//	PRINT("->FreePages(0x%lx, 0x%x) = %r\n", Memory, Pages, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrGetMemoryMap(
	IN OUT UINTN *MemoryMapSize,
	IN OUT EFI_MEMORY_DESCRIPTOR *MemoryMap,
	OUT UINTN *MapKey,
	OUT UINTN *DescriptorSize,
	OUT UINT32 *DescriptorVersion
)
{
	EFI_STATUS			Status;
//	UINTN				inMemoryMapSize = *MemoryMapSize;
	
	Status = gOrgBS.GetMemoryMap(MemoryMapSize, MemoryMap, MapKey, DescriptorSize, DescriptorVersion);
	// if print to console, then ExitBootServices will not work
//	PRINT("->GetMemoryMap(0x%x/0x%x, %p, 0x%x, 0x%x, 0x%x) = %r\n", inMemoryMapSize, *MemoryMapSize, MemoryMap, *MapKey, *DescriptorSize, *DescriptorVersion, Status);
	if (Status == EFI_SUCCESS) {
		#if PRINT_MEMORY_MAP == 1
		PrintMemMap(*MemoryMapSize, MemoryMap, *DescriptorSize, *DescriptorVersion);
		#endif
	}
	//Print(L"OvrGetMemoryMap\n");
	return Status;
}

EFI_STATUS EFIAPI
OvrAllocatePool(
	IN EFI_MEMORY_TYPE PoolType,
	IN UINTN Size,
	OUT VOID **Buffer
)
{
	EFI_STATUS			Status;
	
	Status = gOrgBS.AllocatePool(PoolType, Size, Buffer);
	// printing to console requires AllocatePool - recursion, but this is solved by safety check in LogPrint
	// do not print to serial - too many calls from UEFI
	//DebugPrint(1, "->AllocatePool(%s, 0x%x, %p) = %r\n", EfiMemoryTypeDesc[PoolType], Size, *Buffer, Status);
	#if PRINT_ALLOCATE_POOL == 1
	PRINT("->AllocatePool(%s, 0x%x, %p) = %r\n", EfiMemoryTypeDesc[PoolType], Size, *Buffer, Status);
	#endif
	return Status;
}

EFI_STATUS EFIAPI
OvrFreePool(
	IN VOID				*Buffer
)
{
	EFI_STATUS			Status;
	
	Status = gOrgBS.FreePool(Buffer);
	// do not print to console - requires FreePool - recursion
	// do not print to serial - too many calls from UEFI
	//DebugPrint(1, "->FreePool(%p) = %r\n", Buffer, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrCreateEvent(
	IN UINT32			Type,
	IN EFI_TPL			NotifyTpl,
	IN EFI_EVENT_NOTIFY	NotifyFunction,
	IN VOID				*NotifyContext,
	OUT EFI_EVENT		*Event
)
{
	EFI_STATUS			Status;
	
	Status = gOrgBS.CreateEvent(Type, NotifyTpl, NotifyFunction, NotifyContext, Event);
	//PRINT("->CreateEvent(0x%x, 0x%x, %p, %p, %p) = %r\n", Type, NotifyTpl, NotifyFunction, NotifyContext, *Event, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrSetTimer(
	IN EFI_EVENT		Event,
	IN EFI_TIMER_DELAY	Type,
	IN UINT64			TriggerTime
)
{
	EFI_STATUS			Status;
	
	Status = gOrgBS.SetTimer(Event, Type, TriggerTime);
//	PRINT("->SetTimer(%p, %d, 0x%x) = %r\n", Event, Type, TriggerTime, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrWaitForEvent(
	IN UINTN		NumberOfEvents,
	IN EFI_EVENT	*Event,
	OUT UINTN		*Index
)
{
	EFI_STATUS			Status;
	
	Status = gOrgBS.WaitForEvent(NumberOfEvents, Event, Index);
//	PRINT("->WaitForEvent(%d, %p, %d) = %r\n", NumberOfEvents, *Event, *Index, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrSignalEvent(
	IN EFI_EVENT		Event
)
{
	EFI_STATUS			Status;
	
	Status = gOrgBS.SignalEvent(Event);
	//PRINT("->SignalEvent(%p) = %r\n", Event, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrCloseEvent(
	IN EFI_EVENT		Event
)
{
	EFI_STATUS			Status;
	
	Status = gOrgBS.CloseEvent(Event);
	//PRINT("->CloseEvent(%p) = %r\n", Event, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrCheckEvent(
	IN EFI_EVENT		Event
)
{
	EFI_STATUS			Status;
	
	Status = gOrgBS.CheckEvent(Event);
	//PRINT("->CheckEvent(%p) = %r\n", Event, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrInstallProtocolInterface(
	IN OUT EFI_HANDLE		*Handle,
	IN EFI_GUID				*Protocol,
	IN EFI_INTERFACE_TYPE	InterfaceType,
	IN VOID					*Interface
)
{
	EFI_STATUS			Status;
	
	Status = gOrgBS.InstallProtocolInterface(Handle, Protocol, InterfaceType, Interface);
	PRINT("->InstallProtocolInterface(%p, %s, %d, %p) = %r\n", Handle, GuidStr(Protocol), InterfaceType, Interface, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrReinstallProtocolInterface(
	IN EFI_HANDLE		Handle,
	IN EFI_GUID			*Protocol,
	IN VOID				*OldInterface,
	IN VOID				*NewInterface
)
{
	EFI_STATUS			Status;
	
	Status = gOrgBS.ReinstallProtocolInterface(Handle, Protocol, OldInterface, NewInterface);
	PRINT("->ReinstallProtocolInterface(%p, %s, %p, %p) = %r\n", Handle, GuidStr(Protocol), OldInterface, NewInterface, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrUninstallProtocolInterface(
	IN EFI_HANDLE		Handle,
	IN EFI_GUID			*Protocol,
	IN VOID				*Interface
)
{
	EFI_STATUS			Status;
	
	Status = gOrgBS.UninstallProtocolInterface(Handle, Protocol, Interface);
	PRINT("->UninstallProtocolInterface(%p, %s, %p) = %r\n", Handle, GuidStr(Protocol), Interface, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrHandleProtocol(
	IN EFI_HANDLE		Handle,
	IN EFI_GUID			*Protocol,
	OUT VOID			**Interface
)
{
	EFI_STATUS			Status;
	
	Status = gOrgBS.HandleProtocol(Handle, Protocol, Interface);
#if HANDLE_PROTOCOL
	PRINT("->HandleProtocol(%p, %s, %p) = %r\n", Handle, GuidStr(Protocol), *Interface, Status);
#endif
	return Status;
}

EFI_STATUS EFIAPI
OvrRegisterProtocolNotify(
	IN EFI_GUID			*Protocol,
	IN EFI_EVENT		Event,
	OUT VOID			**Registration
)
{
	EFI_STATUS			Status;
	
	Status = gOrgBS.RegisterProtocolNotify(Protocol, Event, Registration);
	PRINT("->RegisterProtocolNotify(%s, %p, %p) = %r\n", GuidStr(Protocol), Event, *Registration, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrLocateHandle(
	IN EFI_LOCATE_SEARCH_TYPE	SearchType,
	IN EFI_GUID					*Protocol,
	IN VOID						*SearchKey,
	IN OUT UINTN				*BufferSize,
	OUT EFI_HANDLE				*Buffer
)
{
	EFI_STATUS			Status;
	UINTN				BufferSizeIn = *BufferSize;
	
	Status = gOrgBS.LocateHandle(SearchType, Protocol, SearchKey, BufferSize, Buffer);
	PRINT("->LocateHandle(%d, %s, %p, 0x%x/0x%x, %p) = %r\n", SearchType, GuidStr(Protocol), SearchKey, BufferSizeIn, *BufferSize, Buffer, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrLocateDevicePath(
	IN EFI_GUID						*Protocol,
	IN OUT EFI_DEVICE_PATH_PROTOCOL	**DevicePath,
	OUT EFI_HANDLE					*Device
)
{
	EFI_STATUS					Status;
	EFI_DEVICE_PATH_PROTOCOL	*DevicePathIn = *DevicePath;
//	PRINT("... try to do LocateDevicePath\n");
	Status = gOrgBS.LocateDevicePath(Protocol, DevicePath, Device);
	// TODO: device path to str
	PRINT("->LocateDevicePath(%s, %p/%p, %p) = %r\n", GuidStr(Protocol), DevicePathIn, DevicePath, Device, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrInstallConfigurationTable(
	IN EFI_GUID					*Guid,
	IN VOID						*Table
)
{
	EFI_STATUS					Status;
	
	Status = gOrgBS.InstallConfigurationTable(Guid, Table);
	// TODO: table guids to Lib.c
	PRINT("->InstallConfigurationTable(%s, %p) = %r\n", GuidStr(Guid), Table, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrLoadImage(
	IN BOOLEAN					BootPolicy,
	IN EFI_HANDLE				ParentImageHandle,
	IN EFI_DEVICE_PATH_PROTOCOL	*DevicePath,
	IN VOID						*SourceBuffer,
	IN UINTN					SourceSize,
	OUT EFI_HANDLE				*ImageHandle
)
{
	EFI_STATUS					Status;
	
	Status = gOrgBS.LoadImage(BootPolicy, ParentImageHandle, DevicePath, SourceBuffer, SourceSize, ImageHandle);
	// TODO: dev path to str
	PRINT("->LoadImage(%c, %p, %p, %p, 0x%x, %p) = %r\n", BootPolicy ? L'T' : L'F', ParentImageHandle, DevicePath, SourceBuffer, SourceSize, ImageHandle, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrStartImage(
	IN EFI_HANDLE				ImageHandle,
	OUT UINTN					*ExitDataSize,
	OUT CHAR16					**ExitData
)
{
	EFI_STATUS					Status;
	
	Status = gOrgBS.StartImage(ImageHandle, ExitDataSize, ExitData);
	PRINT("->StartImage(%p, 0x%x, %p) = %r\n", ImageHandle, *ExitDataSize, *ExitData, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrExit(
	IN EFI_HANDLE				ImageHandle,
	IN EFI_STATUS				ExitStatus,
	IN UINTN					ExitDataSize,
	IN CHAR16					*ExitData
)
{
	EFI_STATUS					Status;
	
	Status = gOrgBS.Exit(ImageHandle, ExitStatus, ExitDataSize, ExitData);
	PRINT("->Exit(%p, %r, 0x%x, %s) = %r\n", ImageHandle, ExitStatus, ExitDataSize, ExitData, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrUnloadImage(
	IN EFI_HANDLE				ImageHandle
)
{
	EFI_STATUS					Status;
	
	Status = gOrgBS.UnloadImage(ImageHandle);
	PRINT("->UnloadImage(%p) = %r\n", ImageHandle, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrExitBootServices(
	IN EFI_HANDLE				ImageHandle,
	IN UINTN					MapKey
)
{
	EFI_STATUS					Status;
	UINTN					 	NewMapKey;
	
	PRINT("ExitBootServices called. Doing some more dumps ...\n");
	
	#if PRINT_DUMPS >= 1
	// Print ST
	PrintSystemTable(gST);
	
	// Print RT vars
	PrintRTVariables(&gOrgRS);
	#endif
	
	// Restore RT services if we should not log calls during runtime
	#if WORK_DURING_RUNTIME == 0
	RestoreRuntimeServices(gRT);
	#endif
	
	PRINT("->ExitBootServices(%p, 0x%x) ...\n", ImageHandle, MapKey);
	
	// Set flag to FALSE to stop some loggers from messing with memory
	InBootServices = FALSE;
	
	// Restore original OutputString
	#if CAPTURE_CONSOLE_OUTPUT >= 1
	gST->ConOut->OutputString = gOrgConOutOutputString;
	#endif
	
	// Notify loggers that boot services are over
	// Saving our log file can cause a vast amount of logging output on some firmwares, so do this after stopping loggers.
	LogOnExitBootServices();
	
	// Call original
	Status = gOrgBS.ExitBootServices(ImageHandle, MapKey);
	
	if (Status == EFI_SUCCESS) {
	
		PRINT("... ExitBootServices = %r\n", Status);
		
	} else {
		//
		// Error exiting boot services. Probably because
		// some of our loggers changed memory map state.
		// We'll just force exiting by obtaining new
		// MapKey and calling ExitBootServices again.
		//
		PRINT("... ExitBootServices = %r\n", Status);
		PRINT("Forcing ExitBootServices ...\n");
		
		Status = GetMemoryMapKey(&NewMapKey);
		if (Status == EFI_SUCCESS) {
			// we have latest mem map and NewMapKey
			// we'll try again ExitBootServices with NewMapKey
			Status = gOrgBS.ExitBootServices(ImageHandle, NewMapKey);
			PRINT("ExitBootServices: 2nd try = %r\n", Status);
		} else {
			PRINT("ERROR obtaining new map key: %r\n", Status);
			Status = EFI_INVALID_PARAMETER;
		}
	}
	
	return Status;
}

EFI_STATUS EFIAPI
OvrGetNextMonotonicCount(
	OUT UINT64					*Count
)
{
	EFI_STATUS					Status;
	
	Status = gOrgBS.GetNextMonotonicCount(Count);
	PRINT("->GetNextMonotonicCount(0x%x) = %r\n", *Count, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrStall(
	IN UINTN					Microseconds
)
{
	EFI_STATUS					Status;
	
	Status = gOrgBS.Stall(Microseconds);
	// do not print - too many calls
	//PRINT("->Stall(%d) = %r\n", Microseconds, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrSetWatchdogTimer(
	IN UINTN					Timeout,
	IN UINT64					WatchdogCode,
	IN UINTN					DataSize,
	IN CHAR16					*WatchdogData
)
{
	EFI_STATUS					Status;
	
	Status = gOrgBS.SetWatchdogTimer(Timeout, WatchdogCode, DataSize, WatchdogData);
	PRINT("->SetWatchdogTimer(%d, 0x%x, %d, %s) = %r\n", Timeout, WatchdogCode, DataSize, WatchdogData, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrConnectController(
	IN EFI_HANDLE				ControllerHandle,
	IN EFI_HANDLE				*DriverImageHandle,
	IN EFI_DEVICE_PATH_PROTOCOL	*RemainingDevicePath,
	IN BOOLEAN					Recursive
)
{
	EFI_STATUS					Status;
	
	Status = gOrgBS.ConnectController(ControllerHandle, DriverImageHandle, RemainingDevicePath, Recursive);
	// TODO: dev path to str
	PRINT("->ConnectController(%p, %p, %p, %c) = %r\n", ControllerHandle, DriverImageHandle, RemainingDevicePath, Recursive ? L'T' : L'F', Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrDisconnectController(
	IN EFI_HANDLE				ControllerHandle,
	IN EFI_HANDLE				DriverImageHandle,
	IN EFI_HANDLE				ChildHandle
)
{
	EFI_STATUS					Status;
	
	Status = gOrgBS.DisconnectController(ControllerHandle, DriverImageHandle, ChildHandle);
	PRINT("->DisconnectController(%p, %p, %p) = %r\n", ControllerHandle, DriverImageHandle, ChildHandle, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrOpenProtocol(
	IN EFI_HANDLE Handle,
	IN EFI_GUID *Protocol,
	OUT VOID **Interface,
	IN EFI_HANDLE AgentHandle,
	IN EFI_HANDLE ControllerHandle,
	IN UINT32 Attributes
)
{
	EFI_STATUS			Status;
#if OPEN_PROTOCOL
	VOID				*InterfaceIn = *Interface;
#endif
	Status = gOrgBS.OpenProtocol(Handle, Protocol, Interface, AgentHandle, ControllerHandle, Attributes);
#if OPEN_PROTOCOL
	PRINT("->OpenProtocol(%p, %s, %p/%p, %p, %p, %x) = %r\n", Handle, GuidStr(Protocol), InterfaceIn, *Interface, AgentHandle, ControllerHandle, Attributes, Status);
#endif
	return Status;
}

EFI_STATUS EFIAPI
OvrCloseProtocol(
	IN EFI_HANDLE		Handle,
	IN EFI_GUID			*Protocol,
	IN EFI_HANDLE		AgentHandle,
	IN EFI_HANDLE		ControllerHandle
)
{
	EFI_STATUS			Status;
	
	Status = gOrgBS.CloseProtocol(Handle, Protocol, AgentHandle, ControllerHandle);
#if OPEN_PROTOCOL
	PRINT("->CloseProtocol(%p, %s, %p, %p) = %r\n", Handle, GuidStr(Protocol), AgentHandle, ControllerHandle, Status);
#endif
	return Status;
}

EFI_STATUS EFIAPI
OvrOpenProtocolInformation(
	IN EFI_HANDLE		Handle,
	IN EFI_GUID			*Protocol,
	OUT EFI_OPEN_PROTOCOL_INFORMATION_ENTRY	**EntryBuffer,
	OUT UINTN			*EntryCount
)
{
	EFI_STATUS			Status;
	
	Status = gOrgBS.OpenProtocolInformation(Handle, Protocol, EntryBuffer, EntryCount);
	PRINT("->OpenProtocolInformation(%p, %s, %p, %d) = %r\n", Handle, GuidStr(Protocol), *EntryBuffer, *EntryCount, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrProtocolsPerHandle(
	IN EFI_HANDLE		Handle,
	OUT EFI_GUID		***ProtocolBuffer,
	OUT UINTN			*ProtocolBufferCount
)
{
	EFI_STATUS			Status;
	
	Status = gOrgBS.ProtocolsPerHandle(Handle, ProtocolBuffer, ProtocolBufferCount);
	// TODO: print list of protocols returned
	PRINT("->ProtocolsPerHandle(%p, %p, %d) = %r\n", Handle, *ProtocolBuffer, *ProtocolBufferCount, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrLocateHandleBuffer(
	IN EFI_LOCATE_SEARCH_TYPE	SearchType,
	IN EFI_GUID					*Protocol,
	IN VOID						*SearchKey,
	IN OUT UINTN				*NoHandles,
	OUT EFI_HANDLE				**Buffer
)
{
	EFI_STATUS			Status;
  STATIC UINTN OldBuffer = 0;
	
	Status = gOrgBS.LocateHandleBuffer(SearchType, Protocol, SearchKey, NoHandles, Buffer);
  if (!CompareGuid(Protocol, &mEfiSimplePointerProtocolGuid) || (UINTN)Buffer != OldBuffer) {
    OldBuffer = (UINTN)Buffer;
    PRINT("->LocateHandleBuffer(%s, %s, %p, %d, %p) = %r\n", EfiLocateSearchType[SearchType], GuidStr(Protocol), SearchKey, *NoHandles, *Buffer, Status);
    
  }
	return Status;
}

EFI_STATUS EFIAPI
OvrLocateProtocol(
	IN EFI_GUID			*Protocol,
	IN VOID				*Registration,
	OUT VOID			**Interface
)
{
	EFI_STATUS			Status;
	VOID				*InterfaceIn = *Interface;
	
	Status = gOrgBS.LocateProtocol(Protocol, Registration, Interface);
	PRINT("->LocateProtocol(%s, %p, %p/%p) = %r\n", GuidStr(Protocol), Registration, InterfaceIn, *Interface, Status);
	return Status;
}

EFI_STATUS EFIAPI
OvrInstallMultipleProtocolInterfaces(
	IN OUT EFI_HANDLE	*Handle,
	...
)
{
	EFI_STATUS			Status;
	EFI_HANDLE			HandleIn = *Handle;
	VA_LIST				Args;
	EFI_GUID			*Protocol[4];
	VOID				*Interface[4];
	UINTN				Index;
	
	// oh my, va args ... well, we'll just have to support constant number
	VA_START(Args, Handle);
	// will start with Index=1 to cover the case when no Protocol/Interface is specified at all
	for (Index = 1; Index < 4; Index++) {
		Protocol[Index] = VA_ARG(Args, EFI_GUID *);
		if (Protocol[Index] == NULL) {
			Index--;
			break;
		}
		Interface[Index] = VA_ARG(Args, VOID *);
	}
	VA_END(Args);
	switch (Index) {
	case 0:
		Status = gOrgBS.InstallMultipleProtocolInterfaces(Handle, NULL);
		PRINT("->InstallMultipleProtocolInterfaces(%p/%p) = %r\n", HandleIn, *Handle, Status);
		break;
		
	case 1:
		Status = gOrgBS.InstallMultipleProtocolInterfaces(Handle, Protocol[1], Interface[1], NULL);
		PRINT("->InstallMultipleProtocolInterfaces(%p/%p, %s, %p) = %r\n",
			HandleIn, *Handle,
			GuidStr(Protocol[1]), Interface[1],
			Status);
		break;
		
	case 2:
		Status = gOrgBS.InstallMultipleProtocolInterfaces(Handle, Protocol[1], Interface[1], Protocol[2], Interface[2], NULL);
		PRINT("->InstallMultipleProtocolInterfaces(%p/%p, %s, %p, %s, %p) = %r\n",
			HandleIn, *Handle,
			GuidStr(Protocol[1]), Interface[1],
			GuidStr(Protocol[2]), Interface[2],
			Status);
		break;
		
	case 3:
		Status = gOrgBS.InstallMultipleProtocolInterfaces(Handle, Protocol[1], Interface[1], Protocol[2], Interface[2], Protocol[3], Interface[3], NULL);
		PRINT("->InstallMultipleProtocolInterfaces(%p/%p, %s, %p, %s, %p, %s, %p) = %r\n",
			HandleIn, *Handle,
			GuidStr(Protocol[1]), Interface[1],
			GuidStr(Protocol[2]), Interface[2],
			GuidStr(Protocol[3]), Interface[3],
			Status);
		break;
	
	default:
		Status = EFI_UNSUPPORTED;
		PRINT("->InstallMultipleProtocolInterfaces(%p, ...) = %r, too many Protocol/Interface pairs\n", *Handle, Status);
		break;
	}
	return Status;
}

EFI_STATUS EFIAPI
OvrUninstallMultipleProtocolInterfaces(
	IN EFI_HANDLE		Handle,
	...
)
{
	EFI_STATUS			Status;
	EFI_HANDLE			HandleIn = Handle;
	VA_LIST				Args;
	EFI_GUID			*Protocol[4];
	VOID				*Interface[4];
	UINTN				Index;
	
	// again, va args ... we'll just have to support constant number
	VA_START(Args, Handle);
	// will start with Index=1 to cover the case when no Protocol/Interface is specified at all
	for (Index = 1; Index < 4; Index++) {
		Protocol[Index] = VA_ARG(Args, EFI_GUID *);
		if (Protocol[Index] == NULL) {
			Index--;
			break;
		}
		Interface[Index] = VA_ARG(Args, VOID *);
	}
	VA_END(Args);
	switch (Index) {
	case 0:
		Status = gOrgBS.UninstallMultipleProtocolInterfaces(Handle);
		PRINT("->UninstallMultipleProtocolInterfaces(%p/%p) = %r\n", HandleIn, Handle, Status);
		break;
		
	case 1:
		Status = gOrgBS.UninstallMultipleProtocolInterfaces(Handle, Protocol[1], Interface[1]);
		PRINT("->UninstallMultipleProtocolInterfaces(%p/%p, %s, %p) = %r\n",
			HandleIn, Handle,
			GuidStr(Protocol[1]), Interface[1],
			Status);
		break;
		
	case 2:
		Status = gOrgBS.UninstallMultipleProtocolInterfaces(Handle, Protocol[1], Interface[1], Protocol[2], Interface[2]);
		PRINT("->UninstallMultipleProtocolInterfaces(%p/%p, %s, %p, %s, %p) = %r\n",
			HandleIn, Handle,
			GuidStr(Protocol[1]), Interface[1],
			GuidStr(Protocol[2]), Interface[2],
			Status);
		break;
		
	case 3:
		Status = gOrgBS.UninstallMultipleProtocolInterfaces(Handle, Protocol[1], Interface[1], Protocol[2], Interface[2], Protocol[3], Interface[3]);
		PRINT("->UninstallMultipleProtocolInterfaces(%p/%p, %s, %p, %s, %p, %s, %p) = %r\n",
			HandleIn, Handle,
			GuidStr(Protocol[1]), Interface[1],
			GuidStr(Protocol[2]), Interface[2],
			GuidStr(Protocol[3]), Interface[3],
			Status);
		break;
	
	default:
		Status = EFI_UNSUPPORTED;
		PRINT("->UninstallMultipleProtocolInterfaces(%p, ...) = %r, too many Protocol/Interface pairs\n", Handle, Status);
		break;
	}
	return Status;
}

EFI_STATUS EFIAPI
OvrCalculateCrc32(
	IN VOID				*Data,
	IN UINTN			DataSize,
	OUT UINT32			*Crc32
)
{
	EFI_STATUS			Status;
	Status = gOrgBS.CalculateCrc32(Data, DataSize, Crc32);
	
	// Omit printing this when using append while logging, as it can end up in calling a file operating inside another file operation
	// (some implementations of File functions use CalculateCrc32)
	#if (LOG_TO_FILE <= 2) && (CLEANER_LOG != 1)
	// Better not print this for cleaner logs - reported by XyZ
	PRINT("->CalculateCrc32(%p, %d, 0x%x) = %r\n", Data, DataSize, Crc32, Status);
	#endif
	return Status;
}

VOID EFIAPI
OvrCopyMem(
	IN VOID				*Destination,
	IN VOID				*Source,
	IN UINTN			Length
)
{
	gOrgBS.CopyMem(Destination, Source, Length);
	//PRINT("->CopyMem(%p, %p, 0x%x)\n", Destination, Source, Length);
	return;
}

VOID EFIAPI
OvrSetMem(
	IN VOID				*Buffer,
	IN UINTN			Size,
	IN UINT8			Value
)
{
	gOrgBS.SetMem(Buffer, Size, Value);
	//PRINT("->SetMem(%p, 0x%x, 0x%x)\n", Buffer, Size, Value);
	return;
}

EFI_STATUS EFIAPI
OvrCreateEventEx(
	IN UINT32			Type,
	IN EFI_TPL			NotifyTpl,
	IN EFI_EVENT_NOTIFY	NotifyFunction,
	IN CONST VOID		*NotifyContext,
	IN CONST EFI_GUID	*EventGroup,
	OUT EFI_EVENT		*Event
)
{
	EFI_STATUS			Status;
	
	Status = gOrgBS.CreateEventEx(Type, NotifyTpl, NotifyFunction, NotifyContext, EventGroup, Event);
//	PRINT("->CreateEventEx(0x%x, 0x%x, %p, %p, %g, %p) = %r\n", Type, NotifyTpl, NotifyFunction, NotifyContext, EventGroup, *Event, Status);
	return Status;
}


////////////////////////////////////////////
//
// Other functions
//

#if CAPTURE_CONSOLE_OUTPUT >= 1
EFI_STATUS EFIAPI
OvrConOutOutputString(IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, IN CHAR16 *String) {
	EFI_STATUS			Status;
	
	#if CAPTURE_CONSOLE_OUTPUT <= 1
	Status = gOrgConOutOutputString(This,String);
	#else
	Status = EFI_SUCCESS;
	#endif
	
	// Print to file only if not invoked from our PRINT()
	if (!InConsolePrint) {
		// Set var to avoid printing to screen again when overriding OutputString
		InConsolePrint = TRUE;
		PRINT("%s",String);
		InConsolePrint = FALSE;
	}
	
	return Status;
}
#endif

/** Installs our boot services overrides. */
EFI_STATUS EFIAPI
OvrBootServices(EFI_BOOT_SERVICES	*BS)
{
	
	PRINT("Overriding boot services ...\n");
	
	// store orig BS
	CopyMem(&gOrgBS, BS, sizeof(EFI_BOOT_SERVICES));
	
	BS->RaiseTPL = OvrRaiseTPL;
	BS->RestoreTPL = OvrRestoreTPL;
	
	BS->AllocatePages = OvrAllocatePages;
	BS->FreePages = OvrFreePages;
	BS->GetMemoryMap = OvrGetMemoryMap;
	BS->AllocatePool = OvrAllocatePool;
	BS->FreePool = OvrFreePool;
	
	BS->CreateEvent = OvrCreateEvent;
	BS->SetTimer = OvrSetTimer;
	BS->WaitForEvent = OvrWaitForEvent;
	BS->SignalEvent = OvrSignalEvent;
	BS->CloseEvent = OvrCloseEvent;
	BS->CheckEvent = OvrCheckEvent;
	
	BS->InstallProtocolInterface = OvrInstallProtocolInterface;
	BS->ReinstallProtocolInterface = OvrReinstallProtocolInterface;
	BS->UninstallProtocolInterface = OvrUninstallProtocolInterface;

	BS->HandleProtocol = OvrHandleProtocol;
	BS->RegisterProtocolNotify = OvrRegisterProtocolNotify;
	BS->LocateHandle = OvrLocateHandle;
	BS->LocateDevicePath = OvrLocateDevicePath;

	BS->InstallConfigurationTable = OvrInstallConfigurationTable;

	BS->LoadImage = OvrLoadImage;
	BS->StartImage = OvrStartImage;
	BS->Exit = OvrExit;
	BS->UnloadImage = OvrUnloadImage;
	
	BS->ExitBootServices = OvrExitBootServices;
	BS->GetNextMonotonicCount = OvrGetNextMonotonicCount;
	BS->Stall = OvrStall;
	BS->SetWatchdogTimer = OvrSetWatchdogTimer;
	
	BS->ConnectController = OvrConnectController;
	BS->DisconnectController = OvrDisconnectController;

	BS->OpenProtocol = OvrOpenProtocol;
	BS->CloseProtocol = OvrCloseProtocol;
	BS->OpenProtocolInformation = OvrOpenProtocolInformation;
	BS->ProtocolsPerHandle = OvrProtocolsPerHandle;
	BS->LocateHandleBuffer = OvrLocateHandleBuffer;
	BS->LocateProtocol = OvrLocateProtocol;
	BS->InstallMultipleProtocolInterfaces = OvrInstallMultipleProtocolInterfaces;
	BS->UninstallMultipleProtocolInterfaces = OvrUninstallMultipleProtocolInterfaces;
	
	BS->CalculateCrc32 = OvrCalculateCrc32;
	BS->CopyMem = OvrCopyMem;
	BS->SetMem = OvrSetMem;
	BS->CreateEventEx = OvrCreateEventEx;
	
	BS->Hdr.CRC32 = 0;
	// use orig function to avoid our PRINT
	gOrgBS.CalculateCrc32(BS, BS->Hdr.HeaderSize, &BS->Hdr.CRC32);
	PRINT("Boot services overriden!\n");
	
	#if CAPTURE_CONSOLE_OUTPUT >= 1
	PRINT("Overriding console output ...\n");
	#if CAPTURE_CONSOLE_OUTPUT == 2
	PRINT("Preventing any further console output, please wait until booting starts ...\n");
	#endif
	gOrgConOutOutputString = gST->ConOut->OutputString;
	gST->ConOut->OutputString = OvrConOutOutputString;
	PRINT("Console output overriden!\n");
	#endif
	
	return EFI_SUCCESS;
}

