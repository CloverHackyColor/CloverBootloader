/** @file
  UEFI Image Loader library implementation.

  Copyright (c) 2023, Marvin Häuser. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-3-Clause
**/

#include <Base.h>
#include <Uefi/UefiBaseType.h>
#include <Uefi/UefiSpec.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseOverflowLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiImageLib.h>
#include <Library/UefiImageExtraActionLib.h>

#include "UeSupport.h"
#include "PeSupport.h"

STATIC_ASSERT (
  UEFI_IMAGE_FORMAT_SUPPORT_SOURCES != 0,
  "At least one UEFI image format support source must be enabled."
  );

#if (UEFI_IMAGE_FORMAT_SUPPORT_SOURCES & (1 << UEFI_IMAGE_SOURCE_NON_FV)) != 0
#define UEFI_IMAGE_FORMAT_SUPPORT_NON_FV  PcdGet8 (PcdUefiImageFormatSupportNonFv)
#else
#define UEFI_IMAGE_FORMAT_SUPPORT_NON_FV  0
#endif

#if (UEFI_IMAGE_FORMAT_SUPPORT_SOURCES & (1 << UEFI_IMAGE_SOURCE_FV)) != 0
#define UEFI_IMAGE_FORMAT_SUPPORT_FV  PcdGet8 (PcdUefiImageFormatSupportFv)
#else
#define UEFI_IMAGE_FORMAT_SUPPORT_FV  0
#endif

#define UEFI_IMAGE_FORMAT_SUPPORT  \
  (UEFI_IMAGE_FORMAT_SUPPORT_NON_FV | UEFI_IMAGE_FORMAT_SUPPORT_FV)

#define FORMAT_EQ(FormatIndex, Format)                      \
  (UEFI_IMAGE_FORMAT_SUPPORT == (1U << (Format)) ||         \
   ((UEFI_IMAGE_FORMAT_SUPPORT & (1U << (Format))) != 0 &&  \
    (FormatIndex) == (Format)))

#define UEFI_IMAGE_EXEC(Result, FormatIndex, Func, ...)  \
  do {                                                   \
    if (FORMAT_EQ ((FormatIndex), UefiImageFormatUe)) {  \
      ASSERT ((FormatIndex) == UefiImageFormatUe);       \
      Result = mUeSupport.Func (__VA_ARGS__);            \
    } else {                                             \
      ASSERT ((FormatIndex) == UefiImageFormatPe);       \
      Result = mPeSupport.Func (__VA_ARGS__);            \
    }                                                    \
  } while (FALSE)

#define UEFI_IMAGE_EXEC_VOID(FormatIndex, Func, ...)     \
  do {                                                   \
    if (FORMAT_EQ ((FormatIndex), UefiImageFormatUe)) {  \
      ASSERT ((FormatIndex) == UefiImageFormatUe);       \
      mUeSupport.Func (__VA_ARGS__);                     \
    } else {                                             \
      ASSERT ((FormatIndex) == UefiImageFormatPe);       \
      mPeSupport.Func (__VA_ARGS__);                     \
    }                                                    \
  } while (FALSE)

STATIC_ASSERT (
  UefiImageFormatMax == 2,
  "Support for more formats needs to be added above."
  );

#define FORMAT_SUPPORTED(Format, Source)                                                                     \
  ((UEFI_IMAGE_FORMAT_SUPPORT & (1U << (Format))) != 0 &&                                                    \
   (((Source) == UEFI_IMAGE_SOURCE_NON_FV && (UEFI_IMAGE_FORMAT_SUPPORT_NON_FV & (1U << (Format))) != 0) ||  \
    ((Source) == UEFI_IMAGE_SOURCE_FV && (UEFI_IMAGE_FORMAT_SUPPORT_FV & (1U << (Format))) != 0)))

STATIC_ASSERT (
  UEFI_IMAGE_SOURCE_MAX == 3,
  "Support for more sources needs to be added above."
  );

STATIC
RETURN_STATUS
InternalInitializeContextPreHash (
  OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN  CONST VOID                       *FileBuffer,
  IN  UINT32                           FileSize,
  IN  UINT8                            FormatIndex,
  IN  UINT8                            ImageOrigin
  )
{
  RETURN_STATUS  Status;

  UEFI_IMAGE_EXEC (
    Status,
    FormatIndex,
    InitializeContextPreHash,
    Context,
    FileBuffer,
    FileSize,
    ImageOrigin
    );

  return Status;
}

RETURN_STATUS
UefiImageInitializeContextPreHash (
  OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN  CONST VOID                       *FileBuffer,
  IN  UINT32                           FileSize,
  IN  UEFI_IMAGE_SOURCE                Source,
  IN  UINT8                            ImageOrigin
  )
{
  RETURN_STATUS  Status;

#if (UEFI_IMAGE_FORMAT_SUPPORT_SOURCES & (1U << UEFI_IMAGE_SOURCE_NON_FV)) != 0
  ASSERT ((PcdGet8 (PcdUefiImageFormatSupportNonFv) & ~((1ULL << UefiImageFormatMax) - 1ULL)) == 0);
  ASSERT (PcdGet8 (PcdUefiImageFormatSupportNonFv) != 0);
#else
  ASSERT (Source != UEFI_IMAGE_SOURCE_NON_FV);
#endif

#if (UEFI_IMAGE_FORMAT_SUPPORT_SOURCES & (1U << UEFI_IMAGE_SOURCE_FV)) != 0
  ASSERT ((PcdGet8 (PcdUefiImageFormatSupportFv) & ~((1ULL << UefiImageFormatMax) - 1ULL)) == 0);
  ASSERT (PcdGet8 (PcdUefiImageFormatSupportFv) != 0);
#else
  ASSERT (Source != UEFI_IMAGE_SOURCE_FV);
#endif

  Status = RETURN_UNSUPPORTED;

  STATIC_ASSERT (
    UefiImageFormatUe == UefiImageFormatMax - 1,
    "Support for more formats needs to be added above."
    );

  if (FORMAT_SUPPORTED (UefiImageFormatUe, Source)) {
    Status = InternalInitializeContextPreHash (
               Context,
               FileBuffer,
               FileSize,
               UefiImageFormatUe,
               ImageOrigin
               );
    if (!RETURN_ERROR (Status)) {
      Context->FormatIndex = UefiImageFormatUe;
    }
  }

  if (RETURN_ERROR (Status) && FORMAT_SUPPORTED (UefiImageFormatPe, Source)) {
    Status = InternalInitializeContextPreHash (
               Context,
               FileBuffer,
               FileSize,
               UefiImageFormatPe,
               ImageOrigin
               );
    if (!RETURN_ERROR (Status)) {
      Context->FormatIndex = UefiImageFormatPe;
    }
  }

  return Status;
}

BOOLEAN
UefiImageHashImageDefault (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN OUT VOID                             *HashContext,
  IN     UEFI_IMAGE_LOADER_HASH_UPDATE    HashUpdate
  )
{
  BOOLEAN  Success;

  UEFI_IMAGE_EXEC (
    Success,
    Context->FormatIndex,
    HashImageDefault,
    Context,
    HashContext,
    HashUpdate
    );

  return Success;
}

RETURN_STATUS
UefiImageInitializeContextPostHash (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  RETURN_STATUS  Status;

  UEFI_IMAGE_EXEC (
    Status,
    Context->FormatIndex,
    InitializeContextPostHash,
    Context
    );

  return Status;
}

RETURN_STATUS
UefiImageLoadImage (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    VOID                             *Destination,
  IN     UINT32                           DestinationSize
  )
{
  RETURN_STATUS  Status;

  UEFI_IMAGE_EXEC (
    Status,
    Context->FormatIndex,
    LoadImage,
    Context,
    Destination,
    DestinationSize
    );

  return Status;
}

BOOLEAN
UefiImageImageIsInplace (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  BOOLEAN Result;

  UEFI_IMAGE_EXEC (
    Result,
    Context->FormatIndex,
    ImageIsInplace,
    Context
    );

  return Result;
}

RETURN_STATUS
UefiImageLoadImageInplace (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  RETURN_STATUS  Status;

  UEFI_IMAGE_EXEC (
    Status,
    Context->FormatIndex,
    LoadImageInplace,
    Context
    );

  return Status;
}

RETURN_STATUS
UefiImageRelocateImageInplace (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  RETURN_STATUS  Status;

  UEFI_IMAGE_EXEC (
    Status,
    Context->FormatIndex,
    RelocateImageInplace,
    Context
    );

  return Status;
}

RETURN_STATUS
UefiImageLoaderGetRuntimeContextSize (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    UINT32                           *Size
  )
{
  RETURN_STATUS  Status;

  UEFI_IMAGE_EXEC (
    Status,
    Context->FormatIndex,
    LoaderGetRuntimeContextSize,
    Context,
    Size
    );
  if (!RETURN_ERROR (Status)) {
    *Size += 8;
  }

  return Status;
}

RETURN_STATUS
UefiImageRelocateImage (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT    *Context,
  IN     UINT64                             BaseAddress,
  OUT    UEFI_IMAGE_LOADER_RUNTIME_CONTEXT  *RuntimeContext OPTIONAL,
  IN     UINT32                             RuntimeContextSize
  )
{
  RETURN_STATUS  Status;
  VOID           *FormatContext;
  UINT32         FormatContextSize;

  FormatContext     = RuntimeContext;
  FormatContextSize = RuntimeContextSize;
  if (RuntimeContext != NULL) {
    *(UINT64 *)RuntimeContext = Context->FormatIndex;
    FormatContext = (VOID *)((UINT64 *)RuntimeContext + 1);
    ASSERT (FormatContextSize >= 8);
    FormatContextSize -= 8;
  }

  UEFI_IMAGE_EXEC (
    Status,
    Context->FormatIndex,
    RelocateImage,
    Context,
    BaseAddress,
    FormatContext,
    FormatContextSize
    );

  if (!RETURN_ERROR (Status)) {
    UefiImageLoaderRelocateImageExtraAction (Context);
  }

  return Status;
}

RETURN_STATUS
UefiImageRuntimeRelocateImage (
  IN OUT VOID                                     *Image,
  IN     UINT32                                   ImageSize,
  IN     UINT64                                   BaseAddress,
  IN     CONST UEFI_IMAGE_LOADER_RUNTIME_CONTEXT  *RuntimeContext
  )
{
  RETURN_STATUS  Status;

  UEFI_IMAGE_EXEC (
    Status,
    *(UINT8 *)RuntimeContext,
    RuntimeRelocateImage,
    Image,
    ImageSize,
    BaseAddress,
    ((UINT64 *)RuntimeContext + 1)
    );

  return Status;
}

VOID
UefiImageDiscardSegments (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  UEFI_IMAGE_EXEC_VOID (
    Context->FormatIndex,
    DiscardSegments,
    Context
    );
}

RETURN_STATUS
UefiImageGetSymbolsPath (
  IN  CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT CONST CHAR8                            **SymbolsPath,
  OUT UINT32                                 *SymbolsPathSize
  )
{
  RETURN_STATUS  Status;

  UEFI_IMAGE_EXEC (
    Status,
    Context->FormatIndex,
    GetSymbolsPath,
    Context,
    SymbolsPath,
    SymbolsPathSize
    );

  return Status;
}

RETURN_STATUS
UefiImageGetFirstCertificate (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    CONST WIN_CERTIFICATE            **Certificate
  )
{
  RETURN_STATUS  Status;

  UEFI_IMAGE_EXEC (
    Status,
    Context->FormatIndex,
    GetFirstCertificate,
    Context,
    Certificate
    );

  return Status;
}

RETURN_STATUS
UefiImageGetNextCertificate (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN OUT CONST WIN_CERTIFICATE            **Certificate
  )
{
  RETURN_STATUS  Status;

  UEFI_IMAGE_EXEC (
    Status,
    Context->FormatIndex,
    GetNextCertificate,
    Context,
    Certificate
    );

  return Status;
}

RETURN_STATUS
UefiImageGetHiiDataRva (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    UINT32                           *HiiRva,
  OUT    UINT32                           *HiiSize
  )
{
  RETURN_STATUS  Status;

  UEFI_IMAGE_EXEC (
    Status,
    Context->FormatIndex,
    GetHiiDataRva,
    Context,
    HiiRva,
    HiiSize
    );

  return Status;
}

UINT32
UefiImageGetEntryPointAddress (
  IN CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  UINT32  Result;

  UEFI_IMAGE_EXEC (
    Result,
    Context->FormatIndex,
    GetEntryPointAddress,
    Context
    );

  return Result;
}

UINT16
UefiImageGetMachine (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  UINT16  Result;

  UEFI_IMAGE_EXEC (
    Result,
    Context->FormatIndex,
    GetMachine,
    Context
    );

  return Result;
}

UINT16
UefiImageGetSubsystem (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  UINT16  Result;

  UEFI_IMAGE_EXEC (
    Result,
    Context->FormatIndex,
    GetSubsystem,
    Context
    );

  return Result;
}

UINT32
UefiImageGetSegmentAlignment (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  UINT32  Result;

  UEFI_IMAGE_EXEC (
    Result,
    Context->FormatIndex,
    GetSegmentAlignment,
    Context
    );

  return Result;
}

UINT32
UefiImageGetImageSize (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  UINT32  Result;

  UEFI_IMAGE_EXEC (
    Result,
    Context->FormatIndex,
    GetImageSize,
    Context
    );

  return Result;
}

UINT64
UefiImageGetBaseAddress (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  UINT64  Result;

  UEFI_IMAGE_EXEC (
    Result,
    Context->FormatIndex,
    GetBaseAddress,
    Context
    );

  return Result;
}

BOOLEAN
UefiImageGetRelocsStripped (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  BOOLEAN Result;

  UEFI_IMAGE_EXEC (
    Result,
    Context->FormatIndex,
    GetRelocsStripped,
    Context
    );

  return Result;
}

UINTN
UefiImageLoaderGetImageAddress (
  IN CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  UINTN Result;

  UEFI_IMAGE_EXEC (
    Result,
    Context->FormatIndex,
    LoaderGetImageAddress,
    Context
    );

  return Result;
}

UINTN
UefiImageLoaderGetDebugAddress (
  IN CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  UINTN Result;

  UEFI_IMAGE_EXEC (
    Result,
    Context->FormatIndex,
    LoaderGetDebugAddress,
    Context
    );

  return Result;
}

UEFI_IMAGE_RECORD *
UefiImageLoaderGetImageRecord (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  UEFI_IMAGE_RECORD *Result;

  UEFI_IMAGE_EXEC (
    Result,
    Context->FormatIndex,
    LoaderGetImageRecord,
    Context
    );

  return Result;
}

RETURN_STATUS
UefiImageDebugLocateImage (
  OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN  UINTN                            Address,
  IN  UINT8                            ImageOrigin
  )
{
  RETURN_STATUS  Status;

  UEFI_IMAGE_EXEC (
    Status,
    Context->FormatIndex,
    DebugLocateImage,
    Context,
    Address,
    ImageOrigin
    );

  return Status;
}

RETURN_STATUS
UefiImageGetFixedAddress (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    UINT64                           *Address
  )
{
  RETURN_STATUS  Status;

  UEFI_IMAGE_EXEC (
    Status,
    Context->FormatIndex,
    GetFixedAddress,
    Context,
    Address
    );

  return Status;
}

VOID
UefiImageDebugPrintSegments (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  UEFI_IMAGE_EXEC_VOID (
    Context->FormatIndex,
    DebugPrintSegments,
    Context
    );
}
