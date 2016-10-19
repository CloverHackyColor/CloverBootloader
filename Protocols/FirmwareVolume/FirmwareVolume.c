//
//  FirmwareVolume.c
//  Clover
//
//  Created by Slice on 19.10.16.
//

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <EfiImageFormat.h>
#include <Protocol/FirmwareVolume.h>

EFI_HANDLE              mHandle = NULL;

EFI_GUID gAppleCursorImageGuid =  {0x1A10742F, 0xFA80, 0x4B79, {0x9D, 0xA6, 0x35, 0x70, 0x58, 0xCC, 0x39, 0x7B}};


EFI_STATUS
EFIAPI
FvGetVolumeAttributes (
                       IN  EFI_FIRMWARE_VOLUME_PROTOCOL  *This,
                       OUT FRAMEWORK_EFI_FV_ATTRIBUTES   *Attributes
                       )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
FvSetVolumeAttributes (
                       IN EFI_FIRMWARE_VOLUME_PROTOCOL     *This,
                       IN OUT FRAMEWORK_EFI_FV_ATTRIBUTES  *Attributes
                       )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
FvReadFile (
            IN EFI_FIRMWARE_VOLUME_PROTOCOL   *This,
            IN EFI_GUID                       *NameGuid,
            IN OUT VOID                       **Buffer,
            IN OUT UINTN                      *BufferSize,
            OUT EFI_FV_FILETYPE               *FoundType,
            OUT EFI_FV_FILE_ATTRIBUTES        *FileAttributes,
            OUT UINT32                        *AuthenticationStatus
            )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
FvReadSection (
               IN EFI_FIRMWARE_VOLUME_PROTOCOL   *This,
               IN EFI_GUID                       *NameGuid,
               IN EFI_SECTION_TYPE               SectionType,
               IN UINTN                          SectionInstance,
               IN OUT VOID                       **Buffer,
               IN OUT UINTN                      *BufferSize,
               OUT UINT32                        *AuthenticationStatus
               )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
FvWriteFile (
             IN EFI_FIRMWARE_VOLUME_PROTOCOL      *This,
             IN UINT32                            NumberOfFiles,
             IN FRAMEWORK_EFI_FV_WRITE_POLICY     WritePolicy,
             IN FRAMEWORK_EFI_FV_WRITE_FILE_DATA  *FileData
             )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
FvGetNextFile (
               IN EFI_FIRMWARE_VOLUME_PROTOCOL   *This,
               IN OUT VOID                       *Key,
               IN OUT EFI_FV_FILETYPE            *FileType,
               OUT EFI_GUID                      *NameGuid,
               OUT EFI_FV_FILE_ATTRIBUTES        *Attributes,
               OUT UINTN                         *Size
               )
{
  return EFI_SUCCESS;
}

EFI_FIRMWARE_VOLUME_PROTOCOL   FirmwareVolume = {
  FvGetVolumeAttributes,
  FvSetVolumeAttributes,
  FvReadFile,
  FvReadSection,
  FvWriteFile,
  FvGetNextFile,
  16,    //KeySize
  NULL  //ParentHandle
};

/****************************************************************
 * Entry point
 ***************************************************************/

/**
 * FirmwareVolume entry point. Installs AppleGraphConfigProtocol.
 */
EFI_STATUS
EFIAPI
FVEntrypoint (IN EFI_HANDLE           ImageHandle,
              IN EFI_SYSTEM_TABLE		*SystemTable
              )
{
  EFI_STATUS					Status; 
  EFI_BOOT_SERVICES*			gBS;
  gBS				= SystemTable->BootServices;
  
  Status = gBS->InstallMultipleProtocolInterfaces (
                                                   &mHandle,
                                                   &gEfiFirmwareVolumeProtocolGuid,
                                                   &FirmwareVolume,
                                                   NULL
                                                   );
  
  return Status;
}

