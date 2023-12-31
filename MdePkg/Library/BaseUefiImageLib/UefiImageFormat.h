/** @file
  UEFI Image Loader library fornat support infrastructure.

  Copyright (c) 2023, Marvin Häuser. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-3-Clause
**/

#ifndef UEFI_IMAGE_FORMAT_H
#define UEFI_IMAGE_FORMAT_H

#include <Base.h>

#include <Library/UefiImageLib.h>

typedef
RETURN_STATUS
(*UEFI_IMAGE_INITIALIZE_CONTEXT_PRE_HASH) (
  OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN  CONST VOID                       *FileBuffer,
  IN  UINT32                           FileSize,
  IN  UINT8                            ImageOrigin
  );

typedef
BOOLEAN
(*UEFI_IMAGE_HASH_IMAGE_DEFAULT) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN OUT VOID                             *HashContext,
  IN     UEFI_IMAGE_LOADER_HASH_UPDATE    HashUpdate
  );

typedef
RETURN_STATUS
(*UEFI_IMAGE_INITIALIZE_CONTEXT_POST_HASH) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

typedef
RETURN_STATUS
(*UEFI_IMAGE_LOAD_IMAGE) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    VOID                             *Destination,
  IN     UINT32                           DestinationSize
  );

typedef
BOOLEAN
(*UEFI_IMAGE_IMAGE_IS_IN_PLACE) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

typedef
RETURN_STATUS
(*UEFI_IMAGE_LOAD_IMAGE_INPLACE) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

typedef
RETURN_STATUS
(*UEFI_IMAGE_RELOCAE_IMAGE_INPLACE) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

typedef
RETURN_STATUS
(*UEFI_IMAGE_LOADER_GET_RUNTIME_CONTEXT_SIZE) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    UINT32                           *Size
  );

typedef
RETURN_STATUS
(*UEFI_IMAGE_RELOCARE_IMAGE) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN     UINT64                           BaseAddress,
  OUT    VOID                             *RuntimeContext OPTIONAL,
  IN     UINT32                           RuntimeContextSize
  );

typedef
RETURN_STATUS
(*UEFI_IMAGE_RUNTIME_RELOCATE_IMAGE) (
  IN OUT VOID        *Image,
  IN     UINT32      ImageSize,
  IN     UINT64      BaseAddress,
  IN     CONST VOID  *RuntimeContext
  );

typedef
VOID
(*UEFI_IMAGE_DISCARD_SEGMENTS) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

typedef
RETURN_STATUS
(*UEFI_IMAGE_GET_SYMBOLS_PATH) (
  IN  CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT CONST CHAR8                            **SymbolsPath,
  OUT UINT32                                 *SymbolsPathSize
  );

typedef
RETURN_STATUS
(*UEFI_IMAGE_GET_FIRST_CERTIFICATE) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    CONST WIN_CERTIFICATE            **Certificate
  );

typedef
RETURN_STATUS
(*UEFI_IMAGE_GET_NEXT_CERTIFICATE) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN OUT CONST WIN_CERTIFICATE            **Certificate
  );

typedef
RETURN_STATUS
(*UEFI_IMAGE_GET_HII_DATA_RVA) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    UINT32                           *HiiRva,
  OUT    UINT32                           *HiiSize
  );

typedef
UINT32
(*UEFI_IMAGE_GET_ENTRY_POINT_ADDRESS) (
  IN CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

typedef
UINT16
(*UEFI_IMAGE_GET_MACHINE) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

typedef
UINT16
(*UEFI_IMAGE_GET_SUBSYSTEM) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

typedef
UINT32
(*UEFI_IMAGE_GET_SEGMENT_ALIGNMENT) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

typedef
UINT32
(*UEFI_IMAGE_GET_IMAGE_SIZE) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

typedef
UINT32
(*UEFI_IMAGE_GET_IMAGE_SIZE_INPLACE) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

typedef
UINT64
(*UEFI_IMAGE_GET_BASE_ADDRESS) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

typedef
BOOLEAN
(*UEFI_IMAGE_GET_RELOCS_STRIPPED) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

typedef
UINTN
(*UEFI_IMAGE_LOADER_GET_IMAGE_ADDRESS) (
  IN CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

typedef
UINTN
(*UEFI_IMAGE_LOADER_GET_DEBUG_ADDRESS) (
  IN CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

typedef
UEFI_IMAGE_RECORD *
(*UEFI_IMAGE_LOADER_GET_IMAGE_RECORD) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

typedef
RETURN_STATUS
(*UEFI_IMAGE_DEBUG_LOCATE_IMAGE) (
  OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN  UINTN                            Address,
  IN  UINT8                            ImageOrigin
  );

typedef
RETURN_STATUS
(*UEFI_IMAGE_GET_FIXED_ADDRESS) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    UINT64                           *Address
  );

typedef
VOID
(*UEFI_IMAGE_DEBUG_PRINT_SEGMENTS) (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  );

typedef struct {
  UEFI_IMAGE_INITIALIZE_CONTEXT_PRE_HASH      InitializeContextPreHash;
  UEFI_IMAGE_HASH_IMAGE_DEFAULT               HashImageDefault;
  UEFI_IMAGE_INITIALIZE_CONTEXT_POST_HASH     InitializeContextPostHash;
  UEFI_IMAGE_LOAD_IMAGE                       LoadImage;
  UEFI_IMAGE_IMAGE_IS_IN_PLACE                ImageIsInplace;
  UEFI_IMAGE_LOAD_IMAGE_INPLACE               LoadImageInplace;
  UEFI_IMAGE_RELOCAE_IMAGE_INPLACE            RelocateImageInplace;
  UEFI_IMAGE_LOADER_GET_RUNTIME_CONTEXT_SIZE  LoaderGetRuntimeContextSize;
  UEFI_IMAGE_RELOCARE_IMAGE                   RelocateImage;
  UEFI_IMAGE_RUNTIME_RELOCATE_IMAGE           RuntimeRelocateImage;
  UEFI_IMAGE_DISCARD_SEGMENTS                 DiscardSegments;
  UEFI_IMAGE_GET_SYMBOLS_PATH                 GetSymbolsPath;
  UEFI_IMAGE_GET_FIRST_CERTIFICATE            GetFirstCertificate;
  UEFI_IMAGE_GET_NEXT_CERTIFICATE             GetNextCertificate;
  UEFI_IMAGE_GET_HII_DATA_RVA                 GetHiiDataRva;
  UEFI_IMAGE_GET_ENTRY_POINT_ADDRESS          GetEntryPointAddress;
  UEFI_IMAGE_GET_MACHINE                      GetMachine;
  UEFI_IMAGE_GET_SUBSYSTEM                    GetSubsystem;
  UEFI_IMAGE_GET_SEGMENT_ALIGNMENT            GetSegmentAlignment;
  UEFI_IMAGE_GET_IMAGE_SIZE                   GetImageSize;
  UEFI_IMAGE_GET_BASE_ADDRESS                 GetBaseAddress;
  UEFI_IMAGE_GET_RELOCS_STRIPPED              GetRelocsStripped;
  UEFI_IMAGE_LOADER_GET_IMAGE_ADDRESS         LoaderGetImageAddress;
  UEFI_IMAGE_LOADER_GET_DEBUG_ADDRESS         LoaderGetDebugAddress;
  UEFI_IMAGE_LOADER_GET_IMAGE_RECORD          LoaderGetImageRecord;
  UEFI_IMAGE_DEBUG_LOCATE_IMAGE               DebugLocateImage;
  UEFI_IMAGE_GET_FIXED_ADDRESS                GetFixedAddress;
  UEFI_IMAGE_DEBUG_PRINT_SEGMENTS             DebugPrintSegments;
} UEFI_IMAGE_FORMAT_SUPPORT;

#endif // UEFI_IMAGE_FORMAT_H
