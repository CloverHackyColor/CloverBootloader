// FIXME: Docs
#ifndef UEFI_IMAGE_LIB_H_
#define UEFI_IMAGE_LIB_H_

#include <Library/UeImageLib.h>
#include <Library/PeCoffLib2.h>

typedef enum {
  UefiImageFormatPe = 0,
  UefiImageFormatUe = 1,
  UefiImageFormatMax
} UEFI_IMAGE_FORMAT;

#define UEFI_IMAGE_SOURCE_NON_FV  0U
#define UEFI_IMAGE_SOURCE_FV      1U
#define UEFI_IMAGE_SOURCE_ALL     2U
#define UEFI_IMAGE_SOURCE_MAX     3U

// FIXME: Get rid of pointers.
typedef struct {
  UINT32     ImageBuffer;
  UINT32     AddressOfEntryPoint;
  UINT8      ImageType;
  UINT32     FileBuffer;
  UINT32     ExeHdrOffset;
  UINT32     SizeOfImage;
  UINT32     FileSize;
  UINT16     Subsystem;
  UINT32     SectionAlignment;
  UINT32     SectionsOffset;
  UINT16     NumberOfSections;
  UINT32     SizeOfHeaders;
} PE_HOB_IMAGE_CONTEXT;

typedef struct {
  UINT32     ImageBuffer;
  UINT32     FileBuffer;
  UINT32     EntryPointAddress;
  UINT32     LoadTablesFileOffset;
  UINT8      NumLoadTables;
  UINT32     LoadTables;
  UINT32     Segments;
  UINT8      LastSegmentIndex;
  UINT32     SegmentAlignment;
  UINT32     ImageSize;
  UINT8      Subsystem;
  UINT8      SegmentImageInfoIterSize;
  UINT32     SegmentsFileOffset;
} UE_HOB_IMAGE_CONTEXT;

typedef struct {
  UINT8                   FormatIndex;
  union {
    UE_HOB_IMAGE_CONTEXT  Ue;
    PE_HOB_IMAGE_CONTEXT  Pe;
  }                       Ctx;
} HOB_IMAGE_CONTEXT;

typedef UINT8 UEFI_IMAGE_SOURCE;

typedef struct {
  UINT8                           FormatIndex;
  union {
    UE_LOADER_IMAGE_CONTEXT       Ue;
    PE_COFF_LOADER_IMAGE_CONTEXT  Pe;
  }                               Ctx;
} UEFI_IMAGE_LOADER_IMAGE_CONTEXT;

typedef struct UEFI_IMAGE_LOADER_RUNTIME_CONTEXT_ UEFI_IMAGE_LOADER_RUNTIME_CONTEXT;

///
/// Image record segment that desribes the UEFI memory permission configuration
/// for one segment of the Image.
///
typedef struct {
  ///
  /// The size, in Bytes, of the Image record segment.
  ///
  UINT32 Size;
  ///
  /// The UEFI memory permission attributes corresponding to this Image record
  /// segment.
  ///
  UINT32 Attributes;
} UEFI_IMAGE_RECORD_SEGMENT;

///
/// The 32-bit signature that identifies a UEFI_IMAGE_RECORD structure.
///
#define UEFI_IMAGE_RECORD_SIGNATURE  SIGNATURE_32 ('U','I','I','R')

///
/// Image record that describes the UEFI memory permission configuration for
/// every segment of the Image.
///
typedef struct {
  ///
  /// The signature of the Image record structure. Must be set to
  /// UEFI_IMAGE_RECORD_SIGNATURE.
  ///
  UINT32                       Signature;
  ///
  /// The number of Image records. Must be at least 1.
  ///
  UINT32                       NumSegments;
  ///
  /// A link to allow insertion of the Image record into a doubly-linked list.
  ///
  LIST_ENTRY                   Link;
  ///
  /// The start address of the Image memory space.
  ///
  UINTN                        StartAddress;
  ///
  /// The end address of the Image memory space. Must be equal to StartAddress
  /// plus the sum of Segments[i].Size for 0 <= i < NumSegments.
  ///
  UINTN                        EndAddress;
  ///
  /// The Image record segments with their corresponding memory permission
  /// attributes. All Image record segments are contiguous and cover the entire
  /// Image memory space. The address of an Image record segment can be
  /// determined by adding the sum of all previous sizes to StartAddress.
  ///
  UEFI_IMAGE_RECORD_SEGMENT    Segments[];
} UEFI_IMAGE_RECORD;

/**
  Adds the digest of Data to HashContext. This function can be called multiple
  times to compute the digest of discontinuous data.

  @param[in,out] HashContext  The context of the current hash.
  @param[in]     Data         The data to be hashed.
  @param[in]     DataSize     The size, in Bytes, of Data.

  @returns  Whether hashing has been successful.
**/
typedef
BOOLEAN
(EFIAPI *UEFI_IMAGE_LOADER_HASH_UPDATE)(
  IN OUT VOID        *HashContext,
  IN     CONST VOID  *Data,
  IN     UINTN       DataSize
  );

// FIXME: Docs
RETURN_STATUS
UefiImageInitializeContextPreHash (
  OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN  CONST VOID                       *FileBuffer,
  IN  UINT32                           FileSize,
  IN  UEFI_IMAGE_SOURCE                Source,
  IN  UINT8                            ImageOrigin
  );

RETURN_STATUS
UefiImageInitializeContextPostHash (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

/**
  Verify the UEFI Image and initialise Context.

  Used offsets and ranges must be aligned and in the bounds of the raw file.
  Image headers and basic Relocation information must be well-formed.

  FileBuffer must remain valid for the entire lifetime of Context.

  @param[out] Context     The context describing the Image.
  @param[in]  FileBuffer  The file data to parse as UEFI Image.
  @param[in]  FileSize    The size, in Bytes, of FileBuffer.
  @param[in]  Source      Determines supported loaders (PE/UE).
  @param[in]  ImageOrigin Determines image protection policy.
  
  @retval RETURN_SUCCESS  The Image context has been initialised successfully.
  @retval other           The file data is malformed.
**/
RETURN_STATUS
UefiImageInitializeContext (
  OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN  CONST VOID                       *FileBuffer,
  IN  UINT32                           FileSize,
  IN  UEFI_IMAGE_SOURCE                Source,
  IN  UINT8                            ImageOrigin
  );

/**
  Retrieves the UefiImageLib Image format identifier.

  @param[out] Context     The context describing the Image.

  @returns  The UefiImageLib format identifier.
**/
UINT8
UefiImageGetFormat (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

/**
  Hashes the Image using the format's default hashing algorithm.

  @param[in,out] Context      The context describing the Image. Must have been
                              initialised by UefiImageInitializeContext().
  @param[in,out] HashContext  The context of the current hash. Must have been
                              initialised for usage with the HashUpdate
                              function.
  @param[in]     HashUpdate   The data hashing function.

  @returns  Whether hashing has been successful.
**/
BOOLEAN
UefiImageHashImageDefault (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN OUT VOID                             *HashContext,
  IN     UEFI_IMAGE_LOADER_HASH_UPDATE    HashUpdate
  );

/**
  Load the Image into the destination memory space.

  @param[in,out] Context       The context describing the Image. Must have been
                               initialised by UefiImageInitializeContext().
  @param[out] Destination      The Image destination memory. Must be allocated
                               from page memory.
  @param[in]  DestinationSize  The size, in Bytes, of Destination. Must be at
                               least as large as the size returned by
                               UefiImageGetImageSize().

  @retval RETURN_SUCCESS  The Image was loaded successfully.
  @retval other           The Image could not be loaded successfully.
**/
RETURN_STATUS
UefiImageLoadImage (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    VOID                             *Destination,
  IN     UINT32                           DestinationSize
  );

// FIXME: Docs
BOOLEAN
UefiImageImageIsInplace (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

/**
  Equivalent to the UefiImageLoadImage() function for inplace-loading. Ensures that
  all important raw file offsets match the respective RVAs.

  @param[in,out] Context  The context describing the Image. Must have been
                          initialised by UefiImageInitializeContext().

  @retval RETURN_SUCCESS  The Image has been inplace-loaded successfully.
  @retval other           The Image is not suitable for inplace-loading.
**/
RETURN_STATUS
UefiImageLoadImageInplace (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

// FIXME: Docs
RETURN_STATUS
UefiImageRelocateImageInplace (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

// FIXME:
RETURN_STATUS
UefiImageRelocateImageInplaceForExecution (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

/**
  Retrieves the size required to bookkeep Image runtime relocation information.

  May only be called when UefiImageGetRelocsStripped() returns FALSE.

  @param[in,out] Context  The context describing the Image. Must have been
                          loaded by UefiImageLoadImage().
  @param[out]    Size     On output, the size, in Bytes, required for the
                          bookkeeping buffer.

  @retval RETURN_SUCCESS  The Image runtime context size for the Image was
                          retrieved successfully.
  @retval other           The Image runtime context size for the Image could not
                          be retrieved successfully.
**/
RETURN_STATUS
UefiImageLoaderGetRuntimeContextSize (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    UINT32                           *Size
  );

/**
  Relocate the Image for boot-time usage.

  May only be called when UefiImageGetRelocsStripped() returns FALSE, or with
  BaseAddress == UefiImageGetBaseAddress().

  @param[in,out] Context             The context describing the Image. Must have
                                     been loaded by UefiImageLoadImage().
  @param[in]     BaseAddress         The address to relocate the Image to.
  @param[out]    RuntimeContext      If not NULL, on output, a bookkeeping data
                                     required for Image runtime relocation.
  @param[in]     RuntimeContextSize  The size, in Bytes, of RuntimeContext. Must
                                     be at least as big as the size returned by
                                     UefiImageLoaderGetRuntimeContextSize().

  @retval RETURN_SUCCESS  The Image has been relocated successfully.
  @retval other           The Image Relocation Directory is malformed.
**/
RETURN_STATUS
UefiImageRelocateImage (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT    *Context,
  IN     UINT64                             BaseAddress,
  OUT    UEFI_IMAGE_LOADER_RUNTIME_CONTEXT  *RuntimeContext OPTIONAL,
  IN     UINT32                             RuntimeContextSize
  );

/**
  Load the Image into the destination memory space, relocate Image for boot-time
  usage, and perform environment-specific actions required to execute code from
  the Image.

  May only be called when UefiImageGetRelocsStripped() returns FALSE, or with
  BaseAddress == UefiImageGetBaseAddress().

  @param[in,out] Context             The context describing the Image. Must have
                                     been initialised by
                                     UefiImageInitializeContext().
  @param[out]    Destination         The Image destination memory. Must be
                                     allocated from page memory.
  @param[in]     DestinationSize     The size, in Bytes, of Destination. Must be
                                     at least as large as the size returned by
                                     UefiImageGetImageSize().
  @param[out]    RuntimeContext      If not NULL, on output, a buffer
                                     bookkeeping data required for Image runtime
                                     relocation.
  @param[in]     RuntimeContextSize  The size, in Bytes, of RuntimeContext. Must
                                     be at least as big as the size returned by
                                     UefiImageLoaderGetRuntimeContextSize().

  @retval RETURN_SUCCESS  The Image was loaded successfully.
  @retval other           The Image could not be loaded successfully.
**/
RETURN_STATUS
UefiImageLoadImageForExecution (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT    *Context,
  OUT    VOID                               *Destination,
  IN     UINT32                             DestinationSize,
  OUT    UEFI_IMAGE_LOADER_RUNTIME_CONTEXT  *RuntimeContext OPTIONAL,
  IN     UINT32                             RuntimeContextSize
  );

/**
  Relocate Image for Runtime usage.

  May only be called when UefiImageGetRelocsStripped() returns FALSE, or with
  BaseAddress == UefiImageGetBaseAddress().

  @param[in,out] Image           The Image destination memory. Must have been
                                 relocated by UefiImageRelocateImage().
  @param[in]     ImageSize       The size, in Bytes, of Image.
  @param[in]     BaseAddress     The address to relocate the Image to.
  @param[in]     RuntimeContext  The Relocation context obtained by
                                 UefiImageRelocateImage().

  @retval RETURN_SUCCESS  The Image has been relocated successfully.
  @retval other           The Image could not be relocated successfully.
**/
RETURN_STATUS
UefiImageRuntimeRelocateImage (
  IN OUT VOID                                     *Image,
  IN     UINT32                                   ImageSize,
  IN     UINT64                                   BaseAddress,
  IN     CONST UEFI_IMAGE_LOADER_RUNTIME_CONTEXT  *RuntimeContext
  );

/**
  Relocate Image for Runtime usage, and perform environment-specific actions
  required to execute code from the Image.

  May only be called when UefiImageGetRelocsStripped() returns FALSE, or with
  BaseAddress == UefiImageGetBaseAddress().

  @param[in,out] Image           The Image destination memory. Must have been
                                 relocated by UefiImageRelocateImage().
  @param[in]     ImageSize       The size, in Bytes, of Image.
  @param[in]     BaseAddress     The address to relocate the Image to.
  @param[in]     RuntimeContext  The Relocation context obtained by
                                 UefiImageRelocateImage().

  @retval RETURN_SUCCESS  The Image has been relocated successfully.
  @retval other           The Image could not be relocated successfully.
**/
RETURN_STATUS
UefiImageRuntimeRelocateImageForExecution (
  IN OUT VOID                                     *Image,
  IN     UINT32                                   ImageSize,
  IN     UINT64                                   BaseAddress,
  IN     CONST UEFI_IMAGE_LOADER_RUNTIME_CONTEXT  *RuntimeContext
  );

/**
  Discards optional Image segments to disguise sensitive data.

  This may destruct the Image Relocation Directory and as such, no function that
  performs Image relocation may be called after this function has been invoked.

  @param[in,out] Context  The context describing the Image. Must have been
                          loaded by UefiImageLoadImage().
**/
VOID
UefiImageDiscardSegments (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

/**
  Retrieves the Image symbols path.

  @param[in,out] Context          The context describing the Image. Must have
                                  been initialised by
                                  UefiImageInitializeContext().
  @param[out]    SymbolsPath      On output, a pointer to the Image symbols
                                  path.
  @param[out]    SymbolsPathSize  On output, the size, in Bytes, of *
                                  SymbolsPath.

  @retval RETURN_SUCCESS  The Image symbols path was retrieved successfully.
  @retval other           The Image symbols path could not be retrieved
                          successfully.
**/
RETURN_STATUS
UefiImageGetSymbolsPath (
  IN  CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT CONST CHAR8                            **SymbolsPath,
  OUT UINT32                                 *SymbolsPathSize
  );

/**
  Retrieves the Image module name from the Image symbols path.

  @param[in,out] Context         The context describing the Image. Must have
                                 been initialised by
                                 UefiImageInitializeContext().
  @param[out]    ModuleName      Buffer the Image module name is written into.
                                 If the name exceeds ModuleNameSize, it will be
                                 truncated.
  @param[out]    ModuleNameSize  The size, in Bytes, of ModuleName.

  @retval RETURN_SUCCESS  The Image module name was retrieved successfully.
  @retval other           The Image module name could not be retrieved
                          successfully.
**/
RETURN_STATUS
UefiImageGetModuleNameFromSymbolsPath (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    CHAR8                            *ModuleName,
  IN     UINT32                           ModuleNameSize
  );

// FIXME: Abstract certificates somehow?
/**
  Retrieves the first certificate from the Image Certificate Directory.

  @param[in,out] Context      The context describing the Image. Must have been
                              initialised by UefiImageInitializeContext().
  @param[out]    Certificate  On output, the first certificate of the Image.

  @retval RETURN_SUCCESS    The certificate has been retrieved successfully.
  @retval RETURN_NOT_FOUND  There is no such certificate.
  @retval other             The Image Certificate Directory is malformed.
**/
RETURN_STATUS
UefiImageGetFirstCertificate (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    CONST WIN_CERTIFICATE            **Certificate
  );

/**
  Retrieves the next certificate from the Image Certificate Directory.

  @param[in,out] Context      The context describing the Image. Must have been
                              initialised by UefiImageInitializeContext().
  @param[out]    Certificate  On input, the current certificate of the Image.
                              Must have been retrieved by
                              UefiImageGetFirstCertificate().
                              On output, the next certificate of the Image.

  @retval RETURN_SUCCESS    The certificate has been retrieved successfully.
  @retval RETURN_NOT_FOUND  There is no such certificate.
  @retval other             The Image Certificate Directory is malformed.
**/
RETURN_STATUS
UefiImageGetNextCertificate (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN OUT CONST WIN_CERTIFICATE            **Certificate
  );

/**
  Retrieves the Image HII data RVA.

  @param[in,out] Context  The context describing the Image. Must have been
                          initialised by UefiImageInitializeContext().
  @param[out]    HiiRva   On output, the RVA of the HII resource data.
  @param[out]    HiiSize  On output, the size, in Bytes, of HiiRva.

  @retval RETURN_SUCCESS    The Image HII data has been retrieved successfully.
  @retval RETURN_NOT_FOUND  The Image HII data could not be found.
  @retval other             The Image Resource Directory is malformed.
**/
RETURN_STATUS
UefiImageGetHiiDataRva (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    UINT32                           *HiiRva,
  OUT    UINT32                           *HiiSize
  );

/**
  Retrieve the Image entry point RVA.

  @param[in,out] Context  The context describing the Image. Must have been
                          initialised by UefiImageInitializeContext().

  @returns  The Image entry point RVA.
**/
UINT32
UefiImageGetEntryPointAddress (
  IN CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

/**
  Retrieves the Image machine type.

  @param[in,out] Context  The context describing the Image. Must have been
                          initialised by UefiImageInitializeContext().

  @returns  The Image machine type.
**/
UINT16
UefiImageGetMachine (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

/**
  Retrieves the Image subsystem type.

  @param[in,out] Context  The context describing the Image. Must have been
                          initialised by UefiImageInitializeContext().

  @returns  The Image subsystem type.
**/
UINT16
UefiImageGetSubsystem (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

/**
  Retrieves the Image segment alignment.

  @param[in,out] Context  The context describing the Image. Must have been
                          initialised by UefiImageInitializeContext().

  @returns  The Image segment alignment.
**/
UINT32
UefiImageGetSegmentAlignment (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

/**
  Retrieves the size, in Bytes, of the Image memory space.

  @param[in,out] Context  The context describing the Image. Must have been
                          initialised by UefiImageInitializeContext().

  @returns  The size of the Image memory space.
**/
UINT32
UefiImageGetImageSize (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

/**
  Retrieves the Image preferred load address.

  @param[in,out] Context  The context describing the Image. Must have been
                          initialised by UefiImageInitializeContext().

  @returns  The Image preferred load address.
**/
UINT64
UefiImageGetBaseAddress (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

/**
  Returns whether the Image relocations have been stripped.

  @param[in,out] Context  The context describing the Image. Must have been
                          initialised by UefiImageInitializeContext().

  @returns  Whether the Image relocations have been stripped.
**/
BOOLEAN
UefiImageGetRelocsStripped (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

/**
  Retrieves the Image load address UefiImageLoadImage() has loaded the Image to.

  May be called only after UefiImageLoadImage() has succeeded.

  @param[in,out] Context  The context describing the Image. Must have been
                          initialised by UefiImageInitializeContext().

  @returns  The Image load address.
**/
UINTN
UefiImageLoaderGetImageAddress (
  IN CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

/**
  Retrieves the Image debug address. Due to post-processing, the debug address
  may deviate from the load address. Symbolication must use this address.

  May be called only after UefiImageLoadImage() has succeeded.

  @param[in,out] Context  The context describing the Image. Must have been
                          initialised by UefiImageInitializeContext().

  @returns  The Image debug address.
**/
UINTN
UefiImageLoaderGetDebugAddress (
  IN CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

/**
  Retrieve the Image entry point address.

  May be called only after UefiImageLoadImage() has succeeded.

  @param[in,out] Context  The context describing the Image. Must have been
                          initialised by UefiImageInitializeContext().

  @returns  The Image entry point addres.
**/
UINTN
UefiImageLoaderGetImageEntryPoint (
  IN CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

/**
  Constructs an Image record from the Image. Any headers, gaps, or trailers are
  described as read-only data.

  May be called only after UefiImageLoadImage() has succeeded.

  @param[in,out] Context  The context describing the Image. Must have been
                          initialised by UefiImageInitializeContext().

  @retval NULL   The Image record could not constructed successfully.
  @retval other  The Image record was constructed successfully and is returned.
                 It is allocated using the AllocatePool() API and is
                 caller-owned as soon as this function returns.
**/
UEFI_IMAGE_RECORD *
UefiImageLoaderGetImageRecord (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

// FIXME: Docs
RETURN_STATUS
UefiImageDebugLocateImage (
  OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN  UINTN                            Address,
  IN  UINT8                            ImageOrigin
  );

/**
  Retrieve the Image fixed loading address.

  This function is only guaranteed to function correctly if the Image was built
  by a tool with this feature enabled.

  @param[in,out] Context  The context describing the Image. Must have been
                          initialised by UefiImageInitializeContext().
  @param[out]    Address  On output, the fixed loading address of the Image.
                          *Address is guaranteed to by aligned by the Image
                          segment alignment, and thus the size returned by
                          UefiImageGetImageSize is sufficient to hold the
                          Image.

  @retval RETURN_SUCCESS      The Image has a fixed loading address.
  @retval RETURN_NOT_FOUND    The Image does not have a fixed loading address.
  @retval RETURN_UNSUPPORTED  The Image fixed loading address is unaligned.
**/
RETURN_STATUS
UefiImageGetFixedAddress (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    UINT64                           *Address
  );

// FIXME: Docs
VOID
UefiImageDebugPrintSegments (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

VOID
UefiImageDebugPrintImageRecord (
  IN CONST UEFI_IMAGE_RECORD  *ImageRecord
  );

UINTN
UefiImageLoaderGetDebugAddress (
  IN CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

#endif // UEFI_IMAGE_LIB_H_
