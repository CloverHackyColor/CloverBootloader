/** @file

Module Name:

  Test.c
  
  For testing FSInject driver from shell.

  initial version - dmazar

**/

#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>

#include <Protocol/SimpleFileSystem.h>
#include <Protocol/FSInjectProtocol.h>

#include "Test.h"


EFI_STATUS
EFIAPI
GetVolumeHandleWithDir(CHAR16 *SearchDir, OUT EFI_HANDLE *Handle)
{
	EFI_STATUS		Status;
	UINTN			HandlesSize;
	UINTN			Idx;
	EFI_HANDLE		*Handles;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FS = NULL;
	EFI_FILE_PROTOCOL				*FP = NULL;
	EFI_FILE_PROTOCOL				*FP2 = NULL;
	
	// get all handles with EFI_SIMPLE_FILE_SYSTEM_PROTOCOL
	Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &HandlesSize, &Handles);
	if (EFI_ERROR(Status)) {
		Print(L"LocateHandleBuffer: %r\n", Status);
		return Status;
	}
	Print(L"LocateHandleBuffer: %r, got %d handles\n", Status, HandlesSize);
	
	// find handle that contains SearchDir 
	for (Idx = 0; Idx < HandlesSize; Idx++) {
		// get EFI_SIMPLE_FILE_SYSTEM_PROTOCOL first
		Print(L"Trying handle: %p\n", Handles[Idx]);
		Status = gBS->OpenProtocol(Handles[Idx], &gEfiSimpleFileSystemProtocolGuid, (void **)&FS, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
		if (EFI_ERROR(Status)) {
			Print(L"OpenProtocol(gEfiSimpleFileSystemProtocolGuid): %r\n", Status);
			continue;
		}
		// open volume
		Status = FS->OpenVolume(FS, &FP);
		if (EFI_ERROR(Status)) {
			Print(L"OpenVolume(): %r\n", Status);
			gBS->CloseProtocol(Handles[Idx], &gEfiSimpleFileSystemProtocolGuid, gImageHandle, NULL);
			continue;
		}
		// try to open injection dir
		Status = FP->Open(FP, &FP2, SearchDir, EFI_FILE_MODE_READ, 0);
		if (EFI_ERROR(Status)) {
			Print(L"Open(%s): %r\n", SearchDir, Status);
			FP->Close(FP);
			gBS->CloseProtocol(Handles[Idx], &gEfiSimpleFileSystemProtocolGuid, gImageHandle, NULL);
			continue;
		}
		// we have found it - close it and return handle
		FP2->Close(FP2);
		FP->Close(FP);
		*Handle = Handles[Idx];
		return EFI_SUCCESS;
	}
	// not found
	return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI
InstallTestFSinjection(CHAR16 *TargetDir, CHAR16 *InjectionDir, IN FSI_STRING_LIST *Blacklist, IN FSI_STRING_LIST *ForceLoadKexts)
{
	EFI_STATUS					Status;
	FSINJECTION_PROTOCOL		*FSInjection;
	EFI_HANDLE					TargetHandle;
	EFI_HANDLE					InjectionHandle;
	
	Print(L"InstallTestFSinjection ...\n");
	
	// first get FSINJECTION_PROTOCOL
	Status = gBS->LocateProtocol(&gFSInjectProtocolGuid, NULL, (void **)&FSInjection);
	if (EFI_ERROR(Status)) {
		Print(L"- No FSINJECTION_PROTOCOL, Status = %r\n", Status);
		return Status;
	}
	
	// find our target volume
	Status = GetVolumeHandleWithDir(TargetDir, &TargetHandle);
	Print(L"GetVolumeHandleWithDir(%s): %p, Status = %r\n", TargetDir, TargetHandle, Status);
	if (EFI_ERROR(Status)) {
		Print(L"- No target volume, Status = %r\n", Status);
		return Status;
	}
	// find volume with dir to inject
	Status = GetVolumeHandleWithDir(InjectionDir, &InjectionHandle);
	Print(L"GetVolumeHandleWithDir(%s): %p, Status = %r\n", InjectionDir, InjectionHandle, Status);
	if (EFI_ERROR(Status)) {
		Print(L"- No injection volume, Status = %r\n", Status);
		return Status;
	}
	
	// install FSInjection
	Print(L"- FSInjection->Install(%X, %s, %X, %s) ...\n", TargetHandle, TargetDir, InjectionHandle, InjectionDir);
	Status = FSInjection->Install(TargetHandle, TargetDir, InjectionHandle, InjectionDir, Blacklist, ForceLoadKexts);
	if (EFI_ERROR(Status)) {
		Print(L"- error FSInjection->Install(), Status = %r\n", Status);
	}
	return Status;
	
	/*
	// let's try to uninstall FS protocol from target
	Status = gBS->OpenProtocol(TargetHandle, &gEfiSimpleFileSystemProtocolGuid, &FS, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (EFI_ERROR(Status)) {
		Print(L"OpenProtocol(gEfiSimpleFileSystemProtocolGuid): %r\n", Status);
		return Status;
	}
	Status = gBS->UninstallMultipleProtocolInterfaces(TargetHandle, &gEfiSimpleFileSystemProtocolGuid, FS, NULL);
	if (EFI_ERROR(Status)) {
		Print(L"- error UninstallMultipleProtocolInterfaces for gEfiSimpleFileSystemProtocolGuid, Status = %r\n", Status);
		return Status;
	}
	
	return EFI_SUCCESS;
	*/
}