/** @file

  Simple File System overrides module.

**/

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include "Common.h"


////////////////////////////////////////////
//
// Some helper function
//


/** Composes file name from Parent and FName. Allocates memory for result which should be released by caller. */
CHAR16*
GetNormalizedFName(IN CHAR16 *Parent, IN CHAR16 *FName)
{
	CHAR16			*TmpStr;
	CHAR16			*TmpStr2;
	UINTN			Len;
	
	//DBG("NormFName('%s' + '%s')", Parent, FName);
	// case: FName starts with \ "\System\Xx"
	// we'll just use it as is, but we are wrong if "\System\Xx\..\Yy\.\Zz" or similar
	if (FName[0] == L'\\') {
		FName = AllocateCopyPool(StrSize(FName), FName); // reusing FName
	}
	
	// case: FName is "."
	// we'll just copy Parent assuming Parent is normalized, which will be the case if this func will be correct once
	else if (FName[0] == L'.' && FName[1] == L'\0') {
		FName = AllocateCopyPool(StrSize(Parent), Parent);
	}
	
	// case: FName is ".."
	// we'll extract Parent's parent - also assuming Parent is normalized
	else if (FName[0] == L'.' && FName[1] == L'.' && FName[2] == L'\0') {
		TmpStr = GetStrLastCharOccurence(Parent, L'\\');
		// if there is L'\\' and not at the beginning ...
		if (TmpStr != NULL && TmpStr != Parent) {
			*TmpStr = L'\0'; // terminating Parent; will return L'\\' back
			FName = AllocateCopyPool(StrSize(Parent), Parent);
			*TmpStr = L'\\'; // return L'\\' back to Parent
		} else {
			// caller is doing something wrong - we'll default to L"\\"
			FName = AllocateCopyPool(StrSize(L"\\"), L"\\");
		}
	}
	
	// other cases: for now just do Parent + \ + FName
	// but check if Parent already ends with backslash
	else {
		Len = StrSize(Parent) + StrSize(FName); // has place for extra char (\\) if needed
		TmpStr = AllocateZeroPool(Len);
		StrCpyS(TmpStr, Len/sizeof(CHAR16), Parent);
		TmpStr2 = GetStrLastChar(Parent);
		if (TmpStr2 == NULL || *TmpStr2 != L'\\') {
			StrCatS(TmpStr, Len/sizeof(CHAR16), L"\\");
		}
		StrCatS(TmpStr, Len/sizeof(CHAR16), FName);
    return TmpStr;
	}
	//DBG("='%s' ", FName);
	return FName;
}



////////////////////////////////////////////
// 
// DUC_FILE_PROTOCOL
// our implementation of EFI_FILE_PROTOCOL
//

DUC_FILE_PROTOCOL* CreateOurFp(VOID);

/** EFI_FILE_PROTOCOL.Open - Opens a new file relative to the source file's location. */
EFI_STATUS
EFIAPI
DUC_FP_Open(
			IN EFI_FILE_PROTOCOL        *This,
			OUT EFI_FILE_PROTOCOL       **NewHandle,
			IN CHAR16                   *FileName,
			IN UINT64                   OpenMode,
			IN UINT64                   Attributes
			)
{
	EFI_STATUS				Status; // = EFI_DEVICE_ERROR;
	DUC_FILE_PROTOCOL		*OurFp;
	DUC_FILE_PROTOCOL		*NewFp;
	
	OurFp = DUC_FROM_FILE_PROTOCOL(This);
	PRINT("FP.%p[%s].Open('%s', %x, %x) ", This, OurFp->FName, FileName, OpenMode, Attributes);
	
	// call original implementation to get NewHandle
	Status = OurFp->OrigFp->Open(OurFp->OrigFp, NewHandle, FileName, OpenMode, Attributes);
	if (EFI_ERROR(Status)) {
		PRINT("= %r\n", Status);
		return Status;
	}
	
	// create our FP implementation
	NewFp = CreateOurFp();
	if (NewFp == NULL) {
		Status = EFI_OUT_OF_RESOURCES;
		PRINT("= %r\n", Status);
		return Status;
	}
	NewFp->FName = GetNormalizedFName(OurFp->FName, FileName);
	NewFp->OrigFp = *NewHandle;
	
	// set our implementation as a result
	*NewHandle = &(NewFp->Fp);
	
	PRINT("= EFI_SUCCESS -> FP.%p[%s]\n", *NewHandle, NewFp->FName);
	return EFI_SUCCESS;
}

/** EFI_FILE_PROTOCOL.Close - Closes a specified file handle. */
EFI_STATUS
EFIAPI
DUC_FP_Close(IN EFI_FILE_PROTOCOL  *This)
{
	EFI_STATUS				Status; // = EFI_SUCCESS;
	DUC_FILE_PROTOCOL		*OurFp;
	
	OurFp = DUC_FROM_FILE_PROTOCOL(This);
	PRINT("FP.%p[%s].Close() ", This, OurFp->FName);
	
	// close target FP
	Status = OurFp->OrigFp->Close(OurFp->OrigFp);
	OurFp->OrigFp = NULL;
	if (OurFp->FName != NULL) {
		FreePool(OurFp->FName);
		OurFp->FName = NULL;
	}
	FreePool(OurFp);
	PRINT("= %r\n", Status);
	return Status;
}

/** EFI_FILE_PROTOCOL.Delete - Close and delete the file handle. */
EFI_STATUS
EFIAPI
DUC_FP_Delete(IN EFI_FILE_PROTOCOL  *This)
{
	EFI_STATUS				Status; // = EFI_WARN_DELETE_FAILURE;
	DUC_FILE_PROTOCOL		*OurFp;
	
	OurFp = DUC_FROM_FILE_PROTOCOL(This);
	PRINT("FP.%p[%s].Delete() ", This, OurFp->FName);
	
	Status = OurFp->OrigFp->Delete(OurFp->OrigFp);
	OurFp->OrigFp = NULL;
	if (OurFp->FName != NULL) {
		FreePool(OurFp->FName);
		OurFp->FName = NULL;
	}
	FreePool(OurFp);
	PRINT("= %r\n", Status);
	return Status;
}

/** EFI_FILE_PROTOCOL.Read - Reads data from a file. */
EFI_STATUS
EFIAPI
DUC_FP_Read(
			IN EFI_FILE_PROTOCOL	*This,
			IN OUT UINTN			*BufferSize,
			OUT VOID				*Buffer
			)
{
	EFI_STATUS				Status; // = EFI_DEVICE_ERROR;
	DUC_FILE_PROTOCOL		*OurFp;
	
	OurFp = DUC_FROM_FILE_PROTOCOL(This);
	PRINT("FP.%p[%s].Read(%d, %p) ", This, OurFp->FName, *BufferSize, Buffer);
	
	Status = OurFp->OrigFp->Read(OurFp->OrigFp, BufferSize, Buffer);
	PRINT("= %r\n", Status);
	return Status;
}

/** EFI_FILE_PROTOCOL.Write - Writes data to a file. */
EFI_STATUS
EFIAPI
DUC_FP_Write(
			 IN EFI_FILE_PROTOCOL	*This,
			 IN OUT UINTN			*BufferSize,
			 IN VOID				*Buffer
			 )
{
	EFI_STATUS				Status; // = EFI_DEVICE_ERROR;
	DUC_FILE_PROTOCOL		*OurFp;
	UINTN					InBufferSize = *BufferSize;
	
	OurFp = DUC_FROM_FILE_PROTOCOL(This);
	Status = OurFp->OrigFp->Write(OurFp->OrigFp, BufferSize, Buffer);
	PRINT("FP.%p[%s].Write(%d/%d, %p) = %r\n", This, OurFp->FName, InBufferSize, *BufferSize, Buffer, Status);
	return Status;
}

/** EFI_FILE_PROTOCOL.SetPosition - Sets a file's current position. */
EFI_STATUS
EFIAPI
DUC_FP_SetPosition(
				   IN EFI_FILE_PROTOCOL	*This,
				   IN UINT64			Position
				   )
{
	EFI_STATUS				Status; // = EFI_DEVICE_ERROR;
	DUC_FILE_PROTOCOL		*OurFp;
	
	OurFp = DUC_FROM_FILE_PROTOCOL(This);
	Status = OurFp->OrigFp->SetPosition(OurFp->OrigFp, Position);
	PRINT("FP.%p[%s].SetPosition(%ld) = %r\n", This, OurFp->FName, Position, Status);
	return Status;
}

/** EFI_FILE_PROTOCOL.GetPosition - Returns a file's current position. */
EFI_STATUS
EFIAPI
DUC_FP_GetPosition(
				   IN EFI_FILE_PROTOCOL	*This,
				   IN UINT64			*Position
				   )
{
	EFI_STATUS				Status; // = EFI_DEVICE_ERROR;
	DUC_FILE_PROTOCOL		*OurFp;
	
	OurFp = DUC_FROM_FILE_PROTOCOL(This);
	Status = OurFp->OrigFp->GetPosition(OurFp->OrigFp, Position);
	PRINT("FP.%p[%s].GetPosition(%ld) = %r\n", This, OurFp->FName, *Position, Status);
	return Status;
}

/** EFI_FILE_PROTOCOL.GetInfo - Returns information about a file. */
EFI_STATUS
EFIAPI
DUC_FP_GetInfo(
			   IN EFI_FILE_PROTOCOL	*This,
			   IN EFI_GUID			*InformationType,
			   IN OUT UINTN			*BufferSize,
			   OUT VOID				*Buffer
			   )
{
	EFI_STATUS				Status; // = EFI_DEVICE_ERROR;
	DUC_FILE_PROTOCOL		*OurFp;
	
	OurFp = DUC_FROM_FILE_PROTOCOL(This);
	Status = OurFp->OrigFp->GetInfo(OurFp->OrigFp, InformationType, BufferSize, Buffer);
	PRINT("FP.%p[%s].GetInfo(%s, %d, %p) = %r\n", This, OurFp->FName, GuidStr(InformationType), *BufferSize, Buffer, Status);
	return Status;
}

/** EFI_FILE_PROTOCOL.SetInfo - Sets information about a file. */
EFI_STATUS
EFIAPI
DUC_FP_SetInfo(
			   IN EFI_FILE_PROTOCOL	*This,
			   IN EFI_GUID			*InformationType,
			   IN UINTN				BufferSize,
			   IN VOID				*Buffer
			   )
{
	EFI_STATUS				Status; // = EFI_DEVICE_ERROR;
	DUC_FILE_PROTOCOL		*OurFp;
	
	OurFp = DUC_FROM_FILE_PROTOCOL(This);
	Status = OurFp->OrigFp->SetInfo(OurFp->OrigFp, InformationType, BufferSize, Buffer);
	PRINT("FP.%p[%s].SetInfo(%s, %d, %p) = %r\n", This, OurFp->FName, GuidStr(InformationType), BufferSize, Buffer, Status);
	return Status;
}

/** EFI_FILE_PROTOCOL.Flush - Flushes all modified data associated with a file to a device. */
EFI_STATUS
EFIAPI
DUC_FP_Flush(
			 IN EFI_FILE_PROTOCOL	*This
			 )
{
	EFI_STATUS				Status; // = EFI_DEVICE_ERROR;
	DUC_FILE_PROTOCOL		*OurFp;
	
	OurFp = DUC_FROM_FILE_PROTOCOL(This);
	Status = OurFp->OrigFp->Flush(OurFp->OrigFp);
	PRINT("FP.%p[%s].Flush() = %r\n", This, OurFp->FName, Status);
	return Status;
}

/** Creates our DUC_FILE_PROTOCOL. */
DUC_FILE_PROTOCOL*
CreateOurFp(VOID)
{
	DUC_FILE_PROTOCOL		*OurFp;
	
	OurFp = AllocateZeroPool(sizeof(DUC_FILE_PROTOCOL));
	if (OurFp == NULL) {
		return NULL;
	}
	
	OurFp->Signature = DUC_FILE_PROTOCOL_SIGNATURE;
	OurFp->Fp.Revision = EFI_FILE_PROTOCOL_REVISION;
	OurFp->Fp.Open = DUC_FP_Open;
	OurFp->Fp.Close = DUC_FP_Close;
	OurFp->Fp.Delete = DUC_FP_Delete;
	OurFp->Fp.Read = DUC_FP_Read;
	OurFp->Fp.Write = DUC_FP_Write;
	OurFp->Fp.GetPosition = DUC_FP_GetPosition;
	OurFp->Fp.SetPosition = DUC_FP_SetPosition;
	OurFp->Fp.GetInfo = DUC_FP_GetInfo;
	OurFp->Fp.SetInfo = DUC_FP_SetInfo;
	OurFp->Fp.Flush = DUC_FP_Flush;
	OurFp->FName = NULL;
	OurFp->OrigFp = NULL;
	
	return OurFp;
}



////////////////////////////////////////////
//
// DUC_SIMPLE_FILE_SYSTEM_PROTOCOL
// our implementation of EFI_SIMPLE_FILE_SYSTEM_PROTOCOL
//

/**
 * EFI_SIMPLE_FILE_SYSTEM_PROTOCOL.OpenVolume implementation.
 */
EFI_STATUS
EFIAPI
DUC_FS_OpenVolume(
				   IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL	*This,
				   OUT EFI_FILE_PROTOCOL				**Root
				   )
{
	EFI_STATUS							Status;
	DUC_SIMPLE_FILE_SYSTEM_PROTOCOL		*OurFs;
	DUC_FILE_PROTOCOL					*OurFp;
	
	PRINT("FS.%p.OpenVolume() ", This);
	OurFs = DUC_FROM_SIMPLE_FILE_SYSTEM(This);
	
	// Open with orig FS
	Status = OurFs->OrigFs->OpenVolume(OurFs->OrigFs, Root);
	if (EFI_ERROR(Status)) {
		PRINT("= %r\n", Status);
		return Status;
	}
	
	// Wrap it into our implementation
	OurFp = CreateOurFp();
	if (OurFp == NULL) {
		Status = EFI_OUT_OF_RESOURCES;
		return Status;
	}
	OurFp->FName = AllocateCopyPool(StrSize(L"\\"), L"\\");
	OurFp->OrigFp = *Root;
	
	// set it as result
	*Root = &OurFp->Fp;
	PRINT("= %r -> FP.%p[%s]\n\n", Status, *Root, OurFp->FName);
	
	return Status;
} 


////////////////////////////////////////////
//
// Init functions
//


/** Installs our SFS overrides for given handle. */
EFI_STATUS
OvrFsForHandle (IN EFI_HANDLE Handle)
{
	EFI_STATUS							Status;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL		*OrigFs;
	DUC_SIMPLE_FILE_SYSTEM_PROTOCOL		*OurFs;
	
	PRINT("Overriding Simple File System for handle %p ", Handle);
	
	// Get existing EFI_SIMPLE_FILE_SYSTEM_PROTOCOL
	Status = gBS->OpenProtocol(Handle, &gEfiSimpleFileSystemProtocolGuid, (VOID **)&OrigFs, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (EFI_ERROR(Status)) {
		PRINT(" - OpenProtocol(gEfiSimpleFileSystemProtocolGuid): %r\n", Status);
		return Status;
	}
	
	// Check if this is already our implementation
	OurFs = BASE_CR(OrigFs, DUC_SIMPLE_FILE_SYSTEM_PROTOCOL, Fs);
	if (OurFs->Signature == DUC_SIMPLE_FILE_SYSTEM_PROTOCOL_SIGNATURE) {
		PRINT(" - Already overriden\n");
		return EFI_SUCCESS;
	}
	
	// Create our implementation
	OurFs = AllocateZeroPool(sizeof(DUC_SIMPLE_FILE_SYSTEM_PROTOCOL));
	if (OurFs == NULL) {
		Status = EFI_OUT_OF_RESOURCES;
		PRINT("-  AllocateZeroPool(DUC_SIMPLE_FILE_SYSTEM_PROTOCOL): %r\n", Status);
		return Status;
	}
	OurFs->Signature = DUC_SIMPLE_FILE_SYSTEM_PROTOCOL_SIGNATURE;
	OurFs->Fs.Revision = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION;
	OurFs->Fs.OpenVolume = DUC_FS_OpenVolume;
	OurFs->Handle = Handle;
	OurFs->OrigFs = OrigFs;
	
	// Replace existing orig EFI_SIMPLE_FILE_SYSTEM_PROTOCOL with out implementation
	Status = gBS->ReinstallProtocolInterface(Handle, &gEfiSimpleFileSystemProtocolGuid, OrigFs, &OurFs->Fs);
	if (EFI_ERROR(Status)) {
		FreePool(OurFs);
		PRINT(" - ReinstallProtocolInterface(): %r\n", Status);
		return Status;
	}
	
	PRINT(" - installed\n");
	return EFI_SUCCESS;
}

/** SFS protocol notify registration. */
VOID          *gEfiSFSProtocolNotifyReg;

/** SFS protocol notify event. */
EFI_EVENT     gEfiSFSProtocolEvent;

/** SFS protocol notify event handler. Get's called when new SFS appears in the system. */
VOID
EFIAPI
OnSimpleFileSystemInstall (
						   IN EFI_EVENT        Event,
						   IN VOID             *Context
						   )
{
	EFI_STATUS				Status;
	UINTN					NoHandles = 0;
	EFI_HANDLE				*Handle;
	UINTN					Index;
	
	//
	// Locate Handles with our gEfiSFSProtocolNotifyReg
	//
	Status = gBS->LocateHandleBuffer (
									  ByRegisterNotify,
									  &gEfiSimpleFileSystemProtocolGuid,
									  gEfiSFSProtocolNotifyReg,
									  &NoHandles,
									  &Handle
									  );
	if (EFI_ERROR(Status)) {
		PRINT("Error overriding Simple File Systems: %r\n", Status);
	}
	
	for (Index = 0; Index < NoHandles; Index++) {
		OvrFsForHandle(Handle[Index]);
	}
	
	if (NoHandles != 0) {
		FreePool(Handle);
	}
}

/** Installs our Simple File System overrides. */
EFI_STATUS EFIAPI
OvrFs(VOID)
{
	//
	// Install SFS event notification
	//
	gEfiSFSProtocolEvent = EfiCreateProtocolNotifyEvent (
														&gEfiSimpleFileSystemProtocolGuid,
														TPL_CALLBACK,
														OnSimpleFileSystemInstall,
														NULL,
														&gEfiSFSProtocolNotifyReg
														);
	
	return EFI_SUCCESS;
}

