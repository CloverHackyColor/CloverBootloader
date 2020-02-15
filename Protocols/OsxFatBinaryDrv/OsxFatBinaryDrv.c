//
/// @file OsxFatBinaryDrv/OsxFatBinary.c
///
/// Fat Binary driver
///
/// Fat Binary driver to add Fat Binary support to the LoadImage-function
///

//
// CHANGELOG:
//
// UNKNOWN DATE
// Kabyl and rafirafi 
// Initial implementation
//

#include "OsxFatBinaryDrv.h"

#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/LoadFile.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>

// UefiMain
/// The user Entry Point for Application. The user code starts with this function
/// as the real entry point for the application.
///
/// @param[in] ImageHandle    The firmware allocated handle for the EFI image.  
/// @param[in] SystemTable    A pointer to the EFI System Table.
///
/// @retval EFI_SUCCESS       The entry point is executed successfully.
/// @retval other             Some error occurs when executing this entry point.
EFI_STATUS
EFIAPI
UefiMain(IN  EFI_HANDLE       ImageHandle,
         IN  EFI_SYSTEM_TABLE *SystemTable)
{
  OverrideFunctions ();
  return EFI_SUCCESS;
}

// OverrideFunctions
/// Overrides the original LoadImage function with the Fat Binary-compatible
/// and saves a pointer to the old.
///
/// @retval EFI_SUCCESS  The override is executed successfully.
EFI_STATUS
EFIAPI
OverrideFunctions()
{
  OrigLoadImage  = gBS->LoadImage;
  gBS->LoadImage = OvrLoadImage;

  gBS->Hdr.CRC32 = 0;
  gBS->CalculateCrc32(gBS, sizeof(EFI_BOOT_SERVICES), &gBS->Hdr.CRC32);

  return EFI_SUCCESS;
}

// OvrLoadImage
/// Loads an EFI image into memory. Supports the Fat Binary format.
///
/// @param[in]   BootPolicy               If TRUE, indicates that the request originates from the boot
///                                       manager, and that the boot manager is attempting to load
///                                       FilePath as a boot selection. Ignored if SourceBuffer is
///                                       not NULL.
/// @param[in]   ParentImageHandle        The caller's image handle.
/// @param[in]   DevicePath               The DeviceHandle specific file path from which the image is
///                                       loaded.
/// @param[in]   SourceBuffer             If not NULL, a pointer to the memory location containing a copy
///                                       of the image to be loaded.
/// @param[in]   SourceSize               The size in bytes of SourceBuffer. Ignored if SourceBuffer is NULL.
/// @param[out]  ImageHandle              The pointer to the returned image handle that is created when the
///                                       image is successfully loaded.
///
/// @retval EFI_SUCCESS                   Image was loaded into memory correctly.
/// @retval EFI_NOT_FOUND                 Both SourceBuffer and DevicePath are NULL.
/// @retval EFI_INVALID_PARAMETER         One or more parameters are invalid.
/// @retval EFI_UNSUPPORTED               The image type is not supported.
/// @retval EFI_OUT_OF_RESOURCES          Image was not loaded due to insufficient resources.
/// @retval EFI_LOAD_ERROR                Image was not loaded because the image format was corrupt or not
///                                       understood.
/// @retval EFI_DEVICE_ERROR              Image was not loaded because the device returned a read error.
/// @retval EFI_ACCESS_DENIED             Image was not loaded because the platform policy prohibits the
///                                       image from being loaded. NULL is returned in *ImageHandle.
/// @retval EFI_SECURITY_VIOLATION Image  was loaded and an ImageHandle was created with a
///                                       valid EFI_LOADED_IMAGE_PROTOCOL. However, the current
///                                       platform policy specifies that the image should not be started.
EFI_STATUS
EFIAPI
OvrLoadImage(IN      BOOLEAN                  BootPolicy, 
             IN      EFI_HANDLE               ParentImageHandle, 
             IN      EFI_DEVICE_PATH_PROTOCOL *FilePath,
             IN      VOID                     *SourceBuffer      OPTIONAL,
             IN      UINTN                    SourceSize, 
                OUT  EFI_HANDLE               *ImageHandle)
{
  EFI_STATUS                      Status;
  EFI_STATUS                      Status2;

  FAT_HEADER                      *FatHeader;
  FAT_ARCH                        *FatArch;

  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
  EFI_LOAD_FILE_PROTOCOL          *LoadFile;

  VOID                            *SrcBuffer;
  BOOLEAN                         FreeSourceBuffer;
  BOOLEAN                         FreeSrcBuffer;

  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath;
  EFI_HANDLE                      DeviceHandle;
  EFI_FILE_HANDLE                 FileHandle;
  EFI_FILE_HANDLE                 LastHandle;
  FILEPATH_DEVICE_PATH            *FilePathNode;
  EFI_FILE_INFO                   *FileInfo;
  UINTN                           FileInfoSize;
  FILEPATH_DEVICE_PATH            *OrigFilePathNode;
  
  UINT32                          Index;

  EFI_LOADED_IMAGE_PROTOCOL       *Image;

  FreeSourceBuffer    = FALSE;
  RemainingDevicePath = NULL;
  DeviceHandle        = 0;

  Status = EFI_INVALID_PARAMETER;
  while (SourceBuffer == NULL) {
    if (FilePath == NULL) {
      return Status;
    }

    // Attempt to access the file via a file system interface
    FilePathNode = (FILEPATH_DEVICE_PATH *)FilePath;
    Status       = gBS->LocateDevicePath(&gEfiSimpleFileSystemProtocolGuid, (EFI_DEVICE_PATH_PROTOCOL **)&FilePathNode, &DeviceHandle);
    if (!EFI_ERROR(Status)) {
      RemainingDevicePath = (EFI_DEVICE_PATH_PROTOCOL *)FilePathNode;
      Status              = gBS->HandleProtocol(DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID**)&Volume);

      if (!EFI_ERROR(Status)) {
        // Open the volume to get the File System handle
        Status = Volume->OpenVolume(Volume, &FileHandle);
        if (!EFI_ERROR(Status)) {
          //
          // Duplicate the device path to avoid the access to unaligned device path node.
          // Because the device path consists of one or more FILE_PATH_MEDIA_DEVICE_PATH
          // nodes, It assures the fields in device path nodes are 2 byte aligned.
          //
          FilePathNode = (FILEPATH_DEVICE_PATH *)DuplicateDevicePath((EFI_DEVICE_PATH_PROTOCOL *)FilePathNode);
          if (FilePathNode == NULL) {
            FileHandle->Close(FileHandle);
            Status = EFI_OUT_OF_RESOURCES;
          } else {
            OrigFilePathNode = FilePathNode;
            //
            // Parse each MEDIA_FILEPATH_DP node. There may be more than one, since the
            // directory information and filename can be separate. The goal is to inch
            // our way down each device path node and close the previous node
            //
            while (!IsDevicePathEnd(&FilePathNode->Header)) {
              if (DevicePathType(&FilePathNode->Header) != MEDIA_DEVICE_PATH ||
                  DevicePathSubType(&FilePathNode->Header) != MEDIA_FILEPATH_DP) {
                Status = EFI_UNSUPPORTED;
              }

              if (EFI_ERROR(Status)) {
                break;
              }

              LastHandle   = FileHandle;
              FileHandle   = NULL;

              Status       = LastHandle->Open(LastHandle, &FileHandle, FilePathNode->PathName, EFI_FILE_MODE_READ, 0);

              // Close the previous node
              LastHandle->Close(LastHandle);

              FilePathNode = (FILEPATH_DEVICE_PATH *)NextDevicePathNode(&FilePathNode->Header);
            }

            // Free the allocated memory pool
            gBS->FreePool(OrigFilePathNode);
          }

          if (!EFI_ERROR(Status)) {
            //
            // We have found the file. Now we need to read it. Before we can read the file we need to
            // figure out how big the file is.
            //
            FileInfo     = NULL;
            FileInfoSize = 0;
            Status       = FileHandle->GetInfo(FileHandle, &gEfiFileInfoGuid, &FileInfoSize, FileInfo);
            if (Status == EFI_BUFFER_TOO_SMALL) {
              // inc size by 2 because some drivers (HFSPlus.efi) do not count 0 at the end of file name
              FileInfoSize += 2;
              gBS->AllocatePool(EfiBootServicesData, FileInfoSize, (VOID **)&FileInfo);
              if (FileInfo != NULL) {
                Status = FileHandle->GetInfo(FileHandle, &gEfiFileInfoGuid, &FileInfoSize, FileInfo);
              } else {
                Status = EFI_OUT_OF_RESOURCES;
                break;
              }
            }

            if (!EFI_ERROR(Status)) {
 //             ASSERT(FileInfo != NULL);
              if (FileInfo == NULL) {
                return EFI_OUT_OF_RESOURCES;
              }

              gBS->AllocatePool(EfiBootServicesData, (UINTN)FileInfo->FileSize, &SourceBuffer);

              if (SourceBuffer != NULL) {
                SourceSize = (UINTN)FileInfo->FileSize;
                FreeSourceBuffer = TRUE;
                Status     = FileHandle->Read(FileHandle, &SourceSize, SourceBuffer);

                FileHandle->Close(FileHandle);
                gBS->FreePool(FileInfo);
                FileInfo = NULL; //FreePoll will not zero pointer
              } else {
                Status = EFI_OUT_OF_RESOURCES;
              }

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
    Status              = gBS->LocateDevicePath(&gEfiLoadFileProtocolGuid, &RemainingDevicePath, &DeviceHandle);
    if (!EFI_ERROR(Status)) {
      Status = gBS->HandleProtocol(DeviceHandle, &gEfiLoadFileProtocolGuid, (VOID**)&LoadFile);

      if (!EFI_ERROR(Status)) {
//        ASSERT(SourceSize == 0);
//        ASSERT(SourceBuffer == NULL);

        // Call LoadFile with the correct buffer size
        Status = LoadFile->LoadFile(LoadFile, RemainingDevicePath, BootPolicy, &SourceSize, SourceBuffer);
        if (Status == EFI_BUFFER_TOO_SMALL) {
          gBS->AllocatePool(EfiBootServicesData, SourceSize, &SourceBuffer);   
          Status = (SourceBuffer == NULL) ?
            EFI_OUT_OF_RESOURCES :
            LoadFile->LoadFile(LoadFile, RemainingDevicePath, BootPolicy, &SourceSize, SourceBuffer);
        }

        if (!EFI_ERROR(Status)) {
          FreeSourceBuffer = TRUE;
          break;
        }
      }
    }

    break;
  } // while

  SrcBuffer     = NULL;
  FreeSrcBuffer = FALSE;

  if (SourceBuffer != NULL) {
    FatHeader = (FAT_HEADER *)SourceBuffer;
    if (FatHeader->Magic == FAT_BINARY_MAGIC) {
      FatArch = (FAT_ARCH *)(FatHeader + 1);
      for (Index = 0; Index < FatHeader->NumFatArch; Index++, FatArch++) {
#if defined(EFI32) || defined(MDE_CPU_IA32)
        if (FatArch->CpuType == CPU_TYPE_X86 && FatArch->CpuSubtype == CPU_SUBTYPE_I386_ALL)
#elif defined(EFIX64) || defined(MDE_CPU_X64)
        if (FatArch->CpuType == CPU_TYPE_X86_64 && FatArch->CpuSubtype == CPU_SUBTYPE_I386_ALL)
#else
#error "Undefined Platform"
#endif
        {
          break;
        }
      }

      SourceSize     = FatArch->Size;
      gBS->AllocatePool(EfiBootServicesData, SourceSize, &SrcBuffer);   
//      ASSERT(SrcBuffer != NULL);
      if (SrcBuffer == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      CopyMem(SrcBuffer, (UINT8 *)SourceBuffer + FatArch->Offset, SourceSize);

      FreeSrcBuffer = TRUE;
    } else {
      SrcBuffer     = SourceBuffer;
    }
  }

  Status = OrigLoadImage(BootPolicy, ParentImageHandle, FilePath, SrcBuffer, SourceSize, ImageHandle);

  if (FreeSrcBuffer && SrcBuffer) {
    gBS->FreePool(SrcBuffer);
  }

  if (FreeSourceBuffer && SourceBuffer) {
    gBS->FreePool(SourceBuffer);
  }

  if (FileInfo) {
    gBS->FreePool(FileInfo);
  }

  //
  // dmazar: some boards do not put device handle to EfiLoadedImageProtocol->DeviceHandle
  // when image is loaded from SrcBuffer, and then boot.efi fails.
  // we'll fix EfiLoadedImageProtocol here.
  //
  if (!EFI_ERROR(Status) && DeviceHandle != 0) {
    Status2 = gBS->OpenProtocol(
      *ImageHandle,
      &gEfiLoadedImageProtocolGuid,
      (VOID **)&Image,
      gImageHandle,
      NULL,
      EFI_OPEN_PROTOCOL_GET_PROTOCOL
      );

    if (!EFI_ERROR(Status2) && Image->DeviceHandle != DeviceHandle) {
      Image->DeviceHandle = DeviceHandle;
      Image->FilePath     = DuplicateDevicePath(RemainingDevicePath);
    }
  }

  return Status;
}
