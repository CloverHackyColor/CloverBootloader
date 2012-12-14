/**

  Override for fat binary loading.
  For installation to Clover's \EFI\drivers dir for UEFIs where
  fat binary support is not present.
  
  By Kabyl, rafirafi 

**/

//#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/LoadFile.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>     

/* adresse de la fonction EFI_IMAGE_LOAD*/
EFI_IMAGE_LOAD gBS_LoadImage;

/*définition de la structure du fat binary*/
typedef struct fat_header {
	#define FAT_BINARY_MAGIC 0x0ef1fab9
	UINT32 magic; /* FAT_MAGIC */
	UINT32 nfat_arch; /* number of structs that follow */
} fat_header;

/*définition de la structure qui va stocker les données lu du header, 
on va stocker les données uniquement pour l'architecture active, 
le header contient les données pour  32 et 64 */
typedef struct fat_arch {
	#define CPU_TYPE_X86 0x07
	#define CPU_TYPE_X86_64 0x01000007
	UINT32 cputype; /* cpu specifier (int) */
	#define CPU_SUBTYPE_I386_ALL 0x03
	UINT32 cpusubtype; /* machine specifier (int) */
	UINT32 offset; /* file offset to this object file */
	UINT32 size; /* size of this object file */
	UINT32 align; /* alignment as a power of 2, nécessaire pour 64 */
} fat_arch;

/*********************************/

EFI_STATUS EFIAPI ovrLoadImage(
				BOOLEAN BootPolicy, 
				EFI_HANDLE ParentImageHandle, 
				EFI_DEVICE_PATH_PROTOCOL *FilePath,
				VOID *SourceBuffer, 
				UINTN SourceSize, 
				EFI_HANDLE *ImageHandle);

VOID OverrideFunctions(VOID);



/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.  
  @param[in] SystemTable    A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
UefiMain (
	IN EFI_HANDLE        ImageHandle,
	IN EFI_SYSTEM_TABLE  *SystemTable
)
{
	EFI_STATUS              Status;


	Status = EFI_SUCCESS;

	OverrideFunctions();

	return Status;
}


/****************************************************/

/* La fonction de Load qui va overwrite la fonction de LoadImage de base*/
EFI_STATUS EFIAPI ovrLoadImage(
				BOOLEAN						BootPolicy, 
				EFI_HANDLE					ParentImageHandle, 
				EFI_DEVICE_PATH_PROTOCOL	*FilePath,
				VOID						*SourceBuffer, 
				UINTN						SourceSize, 
				EFI_HANDLE					*ImageHandle)
{

	EFI_DEVICE_PATH_PROTOCOL	*RemainingDevicePath;
	EFI_STATUS					Status = EFI_INVALID_PARAMETER;
	FILEPATH_DEVICE_PATH		*FilePathNode;
	EFI_HANDLE					DeviceHandle;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL	*Volume;
	EFI_FILE_HANDLE				FileHandle;
	EFI_FILE_HANDLE				LastHandle;
	EFI_LOAD_FILE_PROTOCOL		*LoadFile;
	EFI_FILE_INFO				*FileInfo;
	UINTN						FileInfoSize;
	FILEPATH_DEVICE_PATH		*OriginalFilePathNode;

	BOOLEAN						FreeSourceBuffer = FALSE;
	BOOLEAN						FreeSrcBuffer = FALSE;
	VOID						*SrcBuffer = 0;
	fat_header					*FatHeader;
	fat_arch					*FatArch;
	UINT32						i;

	EFI_LOADED_IMAGE_PROTOCOL	*Image;
	EFI_STATUS					Status2;
	
	/*Print(L"Entering ovrLoadImage\n");*/


	OriginalFilePathNode = NULL;
	DeviceHandle = 0;
	RemainingDevicePath = NULL;

	while (SourceBuffer == NULL)
	{
		//
		// Make sure FilePath is valid
		//
		if (FilePath == NULL){
			/*Print(L"filePath not valid\n");*/
			return EFI_INVALID_PARAMETER;
		}

		//
		// Attempt to access the file via a file system interface
		//
		FilePathNode = (FILEPATH_DEVICE_PATH *) FilePath;
		/*Status = DevicePathToInterface(&gEfiSimpleFileSystemProtocolGuid, (EFI_DEVICE_PATH_PROTOCOL **)&FilePathNode,
		(VOID*)&Volume, &DeviceHandle);*/
		Status = gBS->LocateDevicePath (&gEfiSimpleFileSystemProtocolGuid, (EFI_DEVICE_PATH_PROTOCOL **)&FilePathNode, &DeviceHandle);
		if (!EFI_ERROR (Status)) {
		
			RemainingDevicePath = (EFI_DEVICE_PATH_PROTOCOL *)FilePathNode;
			
			Status = gBS->HandleProtocol (DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID**)&Volume);

			if (!EFI_ERROR(Status))
			{
				//
				// Open the Volume to get the File System handle
				//
				Status = Volume->OpenVolume(Volume, &FileHandle);
				if (!EFI_ERROR (Status))
				{
					//
					// Duplicate the device path to avoid the access to unaligned device path node.
					// Because the device path consists of one or more FILE PATH MEDIA DEVICE PATH
					// nodes, It assures the fields in device path nodes are 2 byte aligned.
					//
					FilePathNode = (FILEPATH_DEVICE_PATH *)DuplicateDevicePath((EFI_DEVICE_PATH_PROTOCOL *)FilePathNode);
					if (FilePathNode == NULL)
					{
						FileHandle->Close(FileHandle);
						Status = EFI_OUT_OF_RESOURCES;
					}
					else
					{
						OriginalFilePathNode = FilePathNode;
						//
						// Parse each MEDIA_FILEPATH_DP node. There may be more than one, since the
						// directory information and filename can be seperate. The goal is to inch
						// our way down each device path node and close the previous node
						//
						while (!IsDevicePathEnd(&FilePathNode->Header))
						{
							if (DevicePathType (&FilePathNode->Header) != MEDIA_DEVICE_PATH
							|| DevicePathSubType (&FilePathNode->Header) != MEDIA_FILEPATH_DP)
							Status = EFI_UNSUPPORTED;

							if (EFI_ERROR (Status))
							//
							// Exit loop on Error
							//
							break;

							LastHandle = FileHandle;
							FileHandle = NULL;

							Status = LastHandle->Open(LastHandle, &FileHandle, FilePathNode->PathName, EFI_FILE_MODE_READ, 0);

							//
							// Close the previous node
							//
							LastHandle->Close(LastHandle);

							FilePathNode = (FILEPATH_DEVICE_PATH *) NextDevicePathNode(&FilePathNode->Header);
						}
						//
						// Free the allocated memory pool
						//
						gBS->FreePool(OriginalFilePathNode);
					}

					if (!EFI_ERROR(Status))
					{
						//
						// We have found the file. Now we need to read it. Before we can read the file we need to
						// figure out how big the file is.
						//
						FileInfo = NULL;
						FileInfoSize = 0;
						Status = FileHandle->GetInfo(FileHandle, &gEfiFileInfoGuid, &FileInfoSize, FileInfo);
						if (Status == EFI_BUFFER_TOO_SMALL)
						{
							/*FileInfo = EfiLibAllocatePool(FileInfoSize);*/
                            // inc size by 2 because some drivers (HFSPlus.efi) do not count 0 at the end of file name
                            FileInfoSize += 2;
							gBS->AllocatePool (EfiBootServicesData, FileInfoSize, (VOID **)&FileInfo);
							if (FileInfo != NULL)
								Status = FileHandle->GetInfo(FileHandle, &gEfiFileInfoGuid, &FileInfoSize, FileInfo);
							else
							{
								Status = EFI_OUT_OF_RESOURCES;
								break;
							}
						}

						if (!EFI_ERROR (Status))
						{
							//
							// Allocate space for the file
							//
							ASSERT (FileInfo != NULL);
							gBS->AllocatePool (EfiBootServicesData, (UINTN)FileInfo->FileSize, &SourceBuffer);   

							if (SourceBuffer != NULL)
							{
								//
								// Read the file into the buffer we allocated
								//
								SourceSize = (UINTN) FileInfo->FileSize;
								FreeSourceBuffer = TRUE;
								Status = FileHandle->Read(FileHandle, &SourceSize, SourceBuffer);

								//
								// Close the file since we are done
								//
								FileHandle->Close(FileHandle);
								gBS->FreePool(FileInfo);
							}
							else
								Status = EFI_OUT_OF_RESOURCES;

							break;
						}
					}
				}
			}
		}

		//
		// Try LoadFile style
		//

		RemainingDevicePath = FilePath;
		Status = gBS->LocateDevicePath (&gEfiLoadFileProtocolGuid, &RemainingDevicePath, &DeviceHandle);
		if (!EFI_ERROR (Status)) {
			Status = gBS->HandleProtocol (DeviceHandle, &gEfiLoadFileProtocolGuid, (VOID**)&LoadFile);

			if (!EFI_ERROR(Status))
			{
				//
				// Call LoadFile with the correct buffer size
				//
				ASSERT(SourceSize == 0);
				ASSERT(SourceBuffer == NULL);
				Status = LoadFile->LoadFile(LoadFile, RemainingDevicePath, BootPolicy, &SourceSize, SourceBuffer);
				if (Status == EFI_BUFFER_TOO_SMALL)
				{
					/*SourceBuffer = EfiLibAllocatePool(SourceSize);*/
					gBS->AllocatePool (EfiBootServicesData, SourceSize, &SourceBuffer);   
					if (SourceBuffer == NULL)
						Status = EFI_OUT_OF_RESOURCES;
					else
						Status = LoadFile->LoadFile(LoadFile, RemainingDevicePath, BootPolicy, &SourceSize, SourceBuffer);
				}

				if (!EFI_ERROR(Status))
				{
					FreeSourceBuffer = TRUE;
					break;
				}
			}
		}

		break;
	} // while

	if (SourceBuffer != NULL)
	{
		FatHeader = (fat_header *)SourceBuffer;
		if (FatHeader->magic == FAT_BINARY_MAGIC)
		{
			/*Print(L"FatHeader->magic == FAT_BINARY_MAGIC\n");*/
			FatArch = (fat_arch *)((UINT8 *)SourceBuffer + sizeof(fat_header));
			for (i = 0; i < FatHeader->nfat_arch; i++, FatArch++)
				#if defined(EFI32) || defined(MDE_CPU_IA32)
				if (FatArch->cputype == CPU_TYPE_X86 && FatArch->cpusubtype == CPU_SUBTYPE_I386_ALL)
				#elif defined(EFIX64) || defined(MDE_CPU_X64)
				if (FatArch->cputype == CPU_TYPE_X86_64 && FatArch->cpusubtype == CPU_SUBTYPE_I386_ALL)
				#else
				#error "Undefined Platform"
				#endif
					break;

			SourceSize = FatArch->size;
			/*SrcBuffer = EfiLibAllocatePool(SourceSize);*/
			gBS->AllocatePool (EfiBootServicesData, SourceSize, &SrcBuffer);   
			ASSERT (SrcBuffer != NULL);
			/*EfiCopyMem(SrcBuffer, (UINT8 *)SourceBuffer + FatArch->offset, SourceSize);*/
			gBS->CopyMem(SrcBuffer, (UINT8 *)SourceBuffer + FatArch->offset, SourceSize);

			FreeSrcBuffer = TRUE;
		}
		else {
			/*Print(L"FatHeader->magic = %x\n", FatHeader->magic);*/
			SrcBuffer = SourceBuffer;
		}
	}

	Status = gBS_LoadImage(BootPolicy, ParentImageHandle, FilePath, SrcBuffer, SourceSize, ImageHandle);

	if (FreeSrcBuffer)
		if (SrcBuffer)
			gBS->FreePool(SrcBuffer);
	if (FreeSourceBuffer)
		if (SourceBuffer)
			gBS->FreePool(SourceBuffer);

	//
	// dmazar: some boards do not put device handle to EfiLoadedImageProtocol->DeviceHandle
	// when image is loaded from SrcBuffer, and then boot.efi fails.
	// we'll fix EfiLoadedImageProtocol here.
	//
	if (!EFI_ERROR(Status) && DeviceHandle != 0)
	{
		Status2 = gBS->OpenProtocol (
			*ImageHandle,
			&gEfiLoadedImageProtocolGuid,
			(VOID **) &Image,
			gImageHandle,
			NULL,
			EFI_OPEN_PROTOCOL_GET_PROTOCOL
		);
		if (!EFI_ERROR(Status2))
		{
			if (Image->DeviceHandle != DeviceHandle)
			{
				Image->DeviceHandle = DeviceHandle;
				Image->FilePath = DuplicateDevicePath(RemainingDevicePath);
			}
		}
	}
	/*Print(L"Exiting ovrLoadImage\n");*/
	return Status;
}


VOID OverrideFunctions(VOID)
{
	/*Print(L"Overriding Functions\n");*/

	gBS_LoadImage = gBS->LoadImage;
	gBS->LoadImage = ovrLoadImage;

	gBS->Hdr.CRC32 = 0;
	gBS->CalculateCrc32(gBS, sizeof(EFI_BOOT_SERVICES), &gBS->Hdr.CRC32);

}
