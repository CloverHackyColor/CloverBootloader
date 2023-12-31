/** @file
  UEFI Image Loader library implementation for UE Images.

  Copyright (c) 2021 - 2023, Marvin Häuser. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef UE_LIB_H_
#define UE_LIB_H_

#include <IndustryStandard/UeImage.h>

typedef struct {
  CONST UINT8          *FileBuffer;
  UINT32               UnsignedFileSize;
  UINT8                Subsystem;
  UINT8                Machine;
  BOOLEAN              FixedAddress;
  BOOLEAN              XIP;
  UINT8                LastSegmentIndex;
  UINT32               SegmentsFileOffset; // Unused for XIP
  UINT32               SegmentAlignment;
  CONST VOID           *Segments;
  UINT8                SegmentImageInfoIterSize;
  BOOLEAN              RelocsStripped;
  UINT8                NumLoadTables;
  UINT32               LoadTablesFileOffset;
  UINT32               RelocTableSize;
  CONST UE_LOAD_TABLE  *LoadTables;
  VOID                 *ImageBuffer;
  UINT32               ImageSize;
  UINT32               EntryPointAddress;
  UINT64               BaseAddress; // Unused for XIP
} UE_LOADER_IMAGE_CONTEXT;

typedef struct UE_LOADER_RUNTIME_CONTEXT_ UE_LOADER_RUNTIME_CONTEXT;

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
(EFIAPI *UE_LOADER_HASH_UPDATE)(
  IN OUT VOID        *HashContext,
  IN     CONST VOID  *Data,
  IN     UINTN       DataSize
  );

RETURN_STATUS
UeInitializeContextPreHash (
  OUT UE_LOADER_IMAGE_CONTEXT  *Context,
  IN  CONST VOID               *FileBuffer,
  IN  UINT32                   FileSize
  );

RETURN_STATUS
UeInitializeContextPostHash (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context
  );

BOOLEAN
UeHashImageDefault (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context,
  IN OUT VOID                     *HashContext,
  IN     UE_LOADER_HASH_UPDATE    HashUpdate
  );

RETURN_STATUS
UeLoadImage (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    VOID                     *Destination,
  IN     UINT32                   DestinationSize
  );

RETURN_STATUS
UeLoaderGetRuntimeContextSize (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    UINT32                   *Size
  );

RETURN_STATUS
UeRelocateImage (
  IN OUT UE_LOADER_IMAGE_CONTEXT    *Context,
  IN     UINT64                     BaseAddress,
  OUT    UE_LOADER_RUNTIME_CONTEXT  *RuntimeContext OPTIONAL,
  IN     UINT32                     RuntimeContextSize
  );

RETURN_STATUS
UeRelocateImageForRuntime (
  IN OUT VOID                         *Image,
  IN     UINT32                       ImageSize,
  IN CONST UE_LOADER_RUNTIME_CONTEXT  *RuntimeContext,
  IN UINT64                           BaseAddress
  );

RETURN_STATUS
UeGetSymbolsPath (
  IN  CONST UE_LOADER_IMAGE_CONTEXT  *Context,
  OUT CONST CHAR8                    **SymbolsPath,
  OUT UINT32                         *SymbolsPathSize
  );

UINTN
UeLoaderGetImageDebugAddress (
  IN CONST UE_LOADER_IMAGE_CONTEXT  *Context
  );

RETURN_STATUS
UeGetSegmentNames (
  IN  CONST UE_LOADER_IMAGE_CONTEXT  *Context,
  OUT CONST UE_SEGMENT_NAME          **SegmentNames
  );

UINT32
UeGetEntryPointAddress (
  IN CONST UE_LOADER_IMAGE_CONTEXT  *Context
  );

UINT16
UeGetMachine (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context
  );

UINT16
UeGetSubsystem (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context
  );

UINT32
UeGetSegmentAlignment (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context
  );

UINT32
UeGetImageSize (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context
  );

UINT64
UeGetBaseAddress (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context
  );

BOOLEAN
UeGetRelocsStripped (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context
  );

BOOLEAN
UeGetFixedAddress(
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context
  );

UINTN
UeLoaderGetImageAddress (
  IN CONST UE_LOADER_IMAGE_CONTEXT  *Context
  );

UINT16
UeGetSegments (
  IN  CONST UE_LOADER_IMAGE_CONTEXT  *Context,
  OUT CONST UE_SEGMENT               **Segments
  );

UINT16
UeGetSegmentImageInfos (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    CONST UINT32             **SegmentImageInfos,
  OUT    UINT8                    *SegmentImageInfoIterSize
  );

#endif // UE_LIB_H_
