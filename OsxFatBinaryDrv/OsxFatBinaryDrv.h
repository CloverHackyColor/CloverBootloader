//
/// @file OsxFatBinaryDrv/OsxFatBinary.h
///
/// Fat Binary driver
///
/// Fat Binary driver to add Fat Binary support to the LoadImage-function
///

//
// CHANGELOG:
//
// UNKNOWN
// Kabyl and rafirafi 
// Initial implementation
//

#ifndef _FAT_BINARY_DRV_H_
#define _FAT_BINARY_DRV_H_

// FAT_BINARY_MAGIC
/// The common "Fat Binary Magic" number used to identify a Fat binary
#define FAT_BINARY_MAGIC     0x0ef1fab9

// CPU_TYPE_X86
/// The constant CPU x86-architecture code
#define CPU_TYPE_X86         0x07   

// CPU_TYPE_X86_64
/// The constant CPU x86_64-architecture code
#define CPU_TYPE_X86_64      0x01000007

// CPU_SUBTYPE_I386_ALL
/// The constant CPU x86-sub-architecture
#define CPU_SUBTYPE_I386_ALL 0x03

// OrigLoadImage
/// Stores the address of the original LoadImage function
EFI_IMAGE_LOAD OrigLoadImage;

// FAT_HEADER
/// Defintion of the Fat file header
typedef struct _FAT_HEADER {
  UINT32 Magic;       /// The assumed "Fat Binary Magic" number found in the file
  UINT32 NumFatArch;  /// The hard-coded number of architectures within the file
} FAT_HEADER;

// FAT_ARCH
/// Defintion of the the Fat architecture-specific file header
typedef struct _FAT_ARCH {
  UINT32 CpuType;     /// The found CPU architecture specifier
  UINT32 CpuSubtype;  /// The found CPU sub-architecture specifier
  UINT32 Offset;      /// The offset of the architecture-specific boot file
  UINT32 Size;        /// The size of the architecture-specific boot file
  UINT32 Align;       /// The alignment as a power of 2 (necessary for the x86_64 architecture)
} FAT_ARCH;

// UefiMain
/// The user Entry Point for Application. The user code starts with this function
/// as the real entry point for the application.
///
/// @param[in] ImageHandle  The firmware allocated handle for the EFI image.  
/// @param[in] SystemTable  A pointer to the EFI System Table.
///
/// @retval EFI_SUCCESS     The entry point is executed successfully.
/// @retval other           Some error occurs when executing this entry point.
EFI_STATUS
EFIAPI
UefiMain(IN  EFI_HANDLE       ImageHandle,
         IN  EFI_SYSTEM_TABLE *SystemTable);


// OverrideFunctions
/// Overrides the original LoadImage function with the Fat Binary-compatible
/// and saves a pointer to the old.
///
/// @retval EFI_SUCCESS  The override is executed successfully.
EFI_STATUS
EFIAPI
OverrideFunctions();

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
/// @retval EFI_INVALID_PARAMETER         One or more parametes are invalid.
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
OvrLoadImage(BOOLEAN                  BootPolicy, 
             EFI_HANDLE               ParentImageHandle, 
             EFI_DEVICE_PATH_PROTOCOL *FilePath,
             VOID                     *SourceBuffer, 
             UINTN                    SourceSize, 
             EFI_HANDLE               *ImageHandle);

#endif
