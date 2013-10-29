/** @file

  File system helper functions.

  By dmazar, 26/09/2012             

**/


#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>

#include <Guid/Gpt.h>

#include "Common.h"


/** Loaded image protocol from our image. */
EFI_LOADED_IMAGE_PROTOCOL *gLoadedImage = NULL;


/** Retrieves loaded image protocol from our image. */
VOID
FsGetLoadedImage(VOID)
{
	EFI_STATUS			Status;
	
	
	if (gLoadedImage == NULL) {
		// get our EfiLoadedImageProtocol
		Status = gBS->HandleProtocol(
			gImageHandle,
			&gEfiLoadedImageProtocolGuid,
			(VOID **) &gLoadedImage
			);
		
		if (Status != EFI_SUCCESS) {
			Print(L"FsGetLoadedImage: HandleProtocol(gEfiLoadedImageProtocolGuid) = %r\n", Status);
			return;
		}
	}
}

/** Returns file system protocol from specified volume device. */
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *
FsGetFileSystem(IN EFI_HANDLE VolumeHandle)
{
	EFI_STATUS						Status;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL	*Volume;
	
	
	// open EfiSimpleFileSystemProtocol from device
	Status = gBS->HandleProtocol(
								 VolumeHandle,
								 &gEfiSimpleFileSystemProtocolGuid,
								 (VOID **) &Volume
								 );
	
	if (Status != EFI_SUCCESS) {
		Print(L"FsGetFileSystem: HandleProtocol(gEfiSimpleFileSystemProtocolGuid) = %r\n", Status);
		Volume = NULL;
	}
	
	return Volume;
}

/** Returns file system protocol from volume device we are loaded from. */
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *
FsGetSelfFileSystem(VOID)
{
	
	FsGetLoadedImage();
	if (gLoadedImage == NULL) {
		return NULL;
	}
	
	return FsGetFileSystem(gLoadedImage->DeviceHandle);
}

/** Returns root dir from given file system. */
EFI_FILE_PROTOCOL *
FsGetRootDir(IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume)
{
	EFI_STATUS			Status;
	EFI_FILE_PROTOCOL	*RootDir;
	
	
	if (Volume == NULL) {
		return NULL;
	}
	
	// open RootDir
	Status = Volume->OpenVolume(Volume, &RootDir);
	if (Status != EFI_SUCCESS) {
		Print(L"FsGetRootDir: OpenVolume() = %r\n", Status);
		return NULL;
	}
	
	return RootDir;
}

/** Returns root dir file from file system we are loaded from. */
EFI_FILE_PROTOCOL *
FsGetSelfRootDir(VOID)
{
	
	return FsGetRootDir(FsGetSelfFileSystem());
}

/** Returns dir from file system we are loaded from. */
EFI_FILE_PROTOCOL *
FsGetSelfDir(VOID)
{
	EFI_STATUS			Status;
	EFI_FILE_PROTOCOL	*RootDir;
	EFI_FILE_PROTOCOL	*File;
	EFI_FILE_PROTOCOL	*Dir;
	CHAR16				*FilePath;
	CHAR16				*DirName;
	UINTN				Index;
	
	
	// make sure we have our loaded image protocol
	FsGetLoadedImage();
	if (gLoadedImage == NULL) {
		return NULL;
	}
	
	RootDir = FsGetSelfRootDir();
	if (RootDir == NULL) {
		return NULL;
	}
	
	// extract FilePath
	FilePath = FileDevicePathToText(gLoadedImage->FilePath);
	if (FilePath == NULL) {
		Print(L"FsGetSelfDir: FileDevicePathToText = NULL\n");
		return NULL;
	}
	
	// open file
	Status = RootDir->Open(RootDir, &File, FilePath, EFI_FILE_MODE_READ, 0);
	RootDir->Close(RootDir);
	if (Status != EFI_SUCCESS) {
		Print(L"FsGetSelfDir: Open(%s) = %r\n", FilePath, Status);
		FreePool(FilePath);
		return NULL;
	}
	  
	// find parent dir by putting \0 to last \\ in file path
	for (Index = StrLen(FilePath); Index > 0 && FilePath[Index] != '\\'; Index--) {
		;
	}
	if (Index > 0) {
		FilePath[Index] = L'\0';
		DirName = FilePath;
	} else {
		DirName = L"\\";
	}

	Status = File->Open(File, &Dir, DirName, EFI_FILE_MODE_READ, 0);
	File->Close(File);
	if (Status != EFI_SUCCESS) {
		Print(L"FsGetSelfDir: Open(%s) = %r\n", DirName, Status);
		FreePool(FilePath);
		return NULL;
	}
	FreePool(FilePath);
	
	return Dir;
}

/** Finds first EFI partition and returns it's root dir. */
EFI_FILE_PROTOCOL *
FsGetEspRootDir(VOID)
{
	EFI_STATUS			Status;
	UINTN				HandleCount;
	EFI_HANDLE			*Handles;
	
	
	// find all ESP/EFI volumes
	Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiPartTypeSystemPartGuid, NULL, &HandleCount, &Handles);
	if (EFI_ERROR(Status) || HandleCount == 0) {
		return NULL;
	}
	
	// open root dir from first ESP volume
	return FsGetRootDir(FsGetFileSystem(Handles[0]));
}


/** Checks if file exists. */
BOOLEAN
FsFileExists(
	IN EFI_FILE_PROTOCOL	*Dir,
	IN CHAR16				*FileName
)
{
	EFI_STATUS				Status;
	EFI_FILE_PROTOCOL		*File;
	
	
	if (Dir == NULL || FileName == NULL) {
		return FALSE;
	}
	
	// open file for read to see if it exists
	Status = Dir->Open(Dir, &File, FileName, EFI_FILE_MODE_READ, 0);
	if (!EFI_ERROR(Status)) {
		File->Close(File);
	}
	
	return Status == EFI_SUCCESS;
}

/** Checks if file exists. */
EFI_STATUS
FsDeleteFile(
	IN EFI_FILE_PROTOCOL	*Dir,
	IN CHAR16				*FileName
)
{
	EFI_STATUS				Status;
	EFI_FILE_PROTOCOL		*File;
	
	
	if (Dir == NULL || FileName == NULL) {
		return EFI_NOT_FOUND;
	}
	
	// open file for read/write to see if it exists, need write for delete
	Status = Dir->Open(Dir, &File, FileName, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
	if (Status == EFI_SUCCESS) {
		// exists - delete it
		// note: we are not checking if it is a file or dir
		Status = File->Delete(File);
	}
	
	return Status;
}

/** Saves memory block to a file. */
EFI_STATUS
FsSaveMemToFile(
	IN EFI_FILE_PROTOCOL	*Dir,
	IN CHAR16				*FileName,
	IN VOID					*Data,
	IN UINTN				DataSize
)
{
	EFI_STATUS				Status;
	EFI_FILE_PROTOCOL		*File;
	
	
	if (Dir == NULL || FileName == NULL) {
		return EFI_NOT_FOUND;
	}
	
	// delete previous one if exists
	FsDeleteFile(Dir, FileName);
	
	// open to create it
	Status = Dir->Open(Dir, &File, FileName, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
	if (EFI_ERROR(Status)) {
		return Status;
	}
	
	Status = File->Write(File, &DataSize, Data);
	File->Close(File);
    
    return Status;
}

/** Saves memory block to a file. Tries to save in "self dir",
 *  and if this is not possible then to first ESP/EFI partition.
 */
EFI_STATUS
FsSaveMemToFileToDefaultDir(
	IN CHAR16			*FileName,
	IN VOID				*Data,
	IN UINTN			DataSize
)
{
	EFI_STATUS			Status;
	EFI_FILE_PROTOCOL	*Dir;
	
	
	if (FileName == NULL) {
		return EFI_NOT_FOUND;
	}
	
	// try saving to "self dir"
	Dir = FsGetSelfDir();
	Status = FsSaveMemToFile(Dir, FileName, Data, DataSize);
	if (Dir != NULL) {
		Dir->Close(Dir);
	}
	if (EFI_ERROR(Status)) {
		// error - try saving to ESP root dir
		Dir = FsGetEspRootDir();
		Status = FsSaveMemToFile(Dir, FileName, Data, DataSize);
		if (Dir != NULL) {
			Dir->Close(Dir);
		}
	}
    
    return Status;
}

/** Appends memory block to a file. */
EFI_STATUS
FsAppendMemToFile(
	IN EFI_FILE_PROTOCOL	*Dir,
	IN CHAR16				*FileName,
	IN VOID					*Data,
	IN UINTN				DataSize
)
{
   EFI_STATUS				Status;
	EFI_FILE_PROTOCOL		*File;
	
	
	if (Dir == NULL || FileName == NULL) {
		return EFI_NOT_FOUND;
	}
	
	// open to create it
	Status = Dir->Open(Dir, &File, FileName, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
	if (EFI_ERROR(Status)) {
		return Status;
	}
	Status = File->SetPosition(File, 0xFFFFFFFFFFFFFFFF);
	if (EFI_ERROR(Status)) {
		return Status;
	}
	Status = File->Write(File, &DataSize, Data);
	File->Close(File);
    
    return Status;
}

/** Appends memory block to a file. Tries to save in "self dir",
 *  and if this is not possible then to first ESP/EFI partition.
 */
EFI_STATUS
FsAppendMemToFileToDefaultDir(
	IN CHAR16			*FileName,
	IN VOID				*Data,
	IN UINTN			DataSize
)
{
   EFI_STATUS			Status;
	EFI_FILE_PROTOCOL	*Dir;
	
	
	if (FileName == NULL) {
		return EFI_NOT_FOUND;
	}
	
	// try saving to "self dir"
	Dir = FsGetSelfDir();
	Status = FsAppendMemToFile(Dir, FileName, Data, DataSize);
	if (Dir != NULL) {
		Dir->Close(Dir);
	}
	if (EFI_ERROR(Status)) {
		// error - try saving to ESP root dir
		Dir = FsGetEspRootDir();
		Status = FsAppendMemToFile(Dir, FileName, Data, DataSize);
		if (Dir != NULL) {
			Dir->Close(Dir);
		}
	}
    
    return Status;
}
