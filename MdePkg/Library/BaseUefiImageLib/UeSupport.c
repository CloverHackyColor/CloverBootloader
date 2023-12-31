/** @file
  UEFI Image Loader library implementation for UE Images.

  Copyright (c) 2023, Marvin Häuser. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
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
#include <Library/UeImageLib.h>
#include <Library/UefiImageLib.h>
#include <Library/UefiImageExtraActionLib.h>

#include "UeSupport.h"

STATIC CONST UINT16 mPeMachines[] = {
  IMAGE_FILE_MACHINE_I386,
  IMAGE_FILE_MACHINE_X64,
  IMAGE_FILE_MACHINE_ARMTHUMB_MIXED,
  IMAGE_FILE_MACHINE_ARM64,
  IMAGE_FILE_MACHINE_RISCV32,
  IMAGE_FILE_MACHINE_RISCV64,
  IMAGE_FILE_MACHINE_RISCV128
};

RETURN_STATUS
UefiImageInitializeContextPreHashUe (
  OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN  CONST VOID                       *FileBuffer,
  IN  UINT32                           FileSize,
  IN  UINT8                            ImageOrigin
  )
{
  return UeInitializeContextPreHash (&Context->Ctx.Ue, FileBuffer, FileSize);
}

RETURN_STATUS
UefiImageInitializeContextPostHashUe (
  OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return UeInitializeContextPostHash (&Context->Ctx.Ue);
}

BOOLEAN
UefiImageHashImageDefaultUe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN OUT VOID                             *HashContext,
  IN     UEFI_IMAGE_LOADER_HASH_UPDATE    HashUpdate
  )
{
  return UeHashImageDefault (&Context->Ctx.Ue, HashContext, HashUpdate);
}

RETURN_STATUS
UefiImageLoadImageUe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    VOID                             *Destination,
  IN     UINT32                           DestinationSize
  )
{
  return UeLoadImage (&Context->Ctx.Ue, Destination, DestinationSize);
}

//
// In-place semantics are currently unsupported.
//

// LCOV_EXCL_START
BOOLEAN
UefiImageImageIsInplaceUe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return FALSE;
}

RETURN_STATUS
UefiImageLoadImageInplaceUe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  ASSERT (Context != NULL);

  if (Context->Ctx.Ue.XIP) {
    Context->Ctx.Ue.ImageBuffer = (UINT8 *) Context->Ctx.Ue.FileBuffer;

    return RETURN_SUCCESS;
  }

  return RETURN_UNSUPPORTED;
}

RETURN_STATUS
UefiImageRelocateImageInplaceUe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return RETURN_UNSUPPORTED;
}
// LCOV_EXCL_STOP

RETURN_STATUS
UefiImageLoaderGetRuntimeContextSizeUe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    UINT32                           *Size
  )
{
  return UeLoaderGetRuntimeContextSize (&Context->Ctx.Ue, Size);
}

RETURN_STATUS
UefiImageRelocateImageUe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN     UINT64                           BaseAddress,
  OUT    VOID                             *RuntimeContext OPTIONAL,
  IN     UINT32                           RuntimeContextSize
  )
{
  return UeRelocateImage (
           &Context->Ctx.Ue,
           BaseAddress,
           (UE_LOADER_RUNTIME_CONTEXT *)RuntimeContext,
           RuntimeContextSize
           );
}

RETURN_STATUS
UefiImageRuntimeRelocateImageUe (
  IN OUT VOID        *Image,
  IN     UINT32      ImageSize,
  IN     UINT64      BaseAddress,
  IN     CONST VOID  *RuntimeContext
  )
{
  return UeRelocateImageForRuntime (
           Image,
           ImageSize,
           (UE_LOADER_RUNTIME_CONTEXT *)RuntimeContext,
           BaseAddress
           );
}

VOID
UefiImageDiscardSegmentsUe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  //
  // Anything discardable is not loaded in the first place.
  //
}

RETURN_STATUS
UefiImageGetSymbolsPathUe (
  IN  CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT CONST CHAR8                            **SymbolsPath,
  OUT UINT32                                 *SymbolsPathSize
  )
{
  return UeGetSymbolsPath (&Context->Ctx.Ue, SymbolsPath, SymbolsPathSize);
}

//
// UE does not support embedded certificates (yet).
//

// LCOV_EXCL_START
RETURN_STATUS
UefiImageGetFirstCertificateUe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    CONST WIN_CERTIFICATE            **Certificate
  )
{
  return RETURN_UNSUPPORTED;
}

RETURN_STATUS
UefiImageGetNextCertificateUe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN OUT CONST WIN_CERTIFICATE            **Certificate
  )
{
  return RETURN_UNSUPPORTED;
}

RETURN_STATUS
UefiImageGetHiiDataRvaUe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    UINT32                           *HiiRva,
  OUT    UINT32                           *HiiSize
  )
{
  //
  // UE does not support legacy HII.
  //
  return RETURN_NOT_FOUND;
}
// LCOV_EXCL_STOP

UINT32
UefiImageGetEntryPointAddressUe (
  IN CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return UeGetEntryPointAddress (&Context->Ctx.Ue);
}

UINT16
UefiImageGetMachineUe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  UINT16  UeMachine;

  UeMachine = UeGetMachine (&Context->Ctx.Ue);
  if (UeMachine >= ARRAY_SIZE (mPeMachines)) {
    DEBUG_RAISE ();
    return 0xFFFF;
  }

  return mPeMachines[UeMachine];
}

UINT16
UefiImageGetSubsystemUe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return UeGetSubsystem (&Context->Ctx.Ue) + 10;
}

UINT32
UefiImageGetSegmentAlignmentUe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return UeGetSegmentAlignment (&Context->Ctx.Ue);
}

UINT32
UefiImageGetImageSizeUe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return UeGetImageSize (&Context->Ctx.Ue);
}

UINT64
UefiImageGetBaseAddressUe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return UeGetBaseAddress (&Context->Ctx.Ue);
}

BOOLEAN
UefiImageGetRelocsStrippedUe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return UeGetRelocsStripped (&Context->Ctx.Ue);
}

UINTN
UefiImageLoaderGetImageAddressUe (
  IN CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return UeLoaderGetImageAddress (&Context->Ctx.Ue);
}

UINTN
UefiImageLoaderGetDebugAddressUe (
  IN CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return UeLoaderGetImageDebugAddress (&Context->Ctx.Ue);
}

STATIC
UINT32
InternalPermissionsToAttributes (
  IN UINT8  Permissions
  )
{
  switch (Permissions) {
    case UeSegmentPermX:
    {
      return EFI_MEMORY_RP | EFI_MEMORY_RO;
    }

    case UeSegmentPermRX:
    {
      return EFI_MEMORY_RO;
    }

    case UeSegmentPermRW:
    {
      return EFI_MEMORY_XP;
    }

    case UeSegmentPermR:
    default:
    {
      ASSERT (Permissions == UeSegmentPermR);
      return EFI_MEMORY_XP | EFI_MEMORY_RO;
    }
  }
}

UEFI_IMAGE_RECORD *
UefiImageLoaderGetImageRecordUe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  UEFI_IMAGE_RECORD          *ImageRecord;
  UINTN                      ImageAddress;
  UINT32                     ImageSize;
  UINT32                     ImageSizeRecord;
  UINT32                     NumRecordSegments;
  UEFI_IMAGE_RECORD_SEGMENT  *RecordSegment;
  UINT16                     NumSegments;
  UINT8                      SegmentIterSize;
  CONST UINT32               *SegmentImageInfos;
  CONST UINT8                *SegmentImageInfoPtr;
  UINT32                     SegmentImageInfo;
  UINT32                     SegmentSize;
  UINT8                      SegmentPermissions;
  UINT32                     RangeSize;
  UINT8                      Permissions;

  ASSERT (Context != NULL);

  NumSegments = UeGetSegmentImageInfos (
                  &Context->Ctx.Ue,
                  &SegmentImageInfos,
                  &SegmentIterSize
                  );

  ImageRecord = AllocatePool (
                  sizeof (*ImageRecord)
                    + NumSegments * sizeof (*ImageRecord->Segments)
                  );
  if (ImageRecord == NULL) {
    return NULL;
  }

  ImageRecord->Signature = UEFI_IMAGE_RECORD_SIGNATURE;
  InitializeListHead (&ImageRecord->Link);

  SegmentImageInfo = *SegmentImageInfos;

  RangeSize   = UE_SEGMENT_SIZE (SegmentImageInfo);
  Permissions = UE_SEGMENT_PERMISSIONS (SegmentImageInfo);

  ImageSizeRecord   = RangeSize;
  NumRecordSegments = 0;

  STATIC_ASSERT (
    OFFSET_OF (UE_SEGMENT, ImageInfo) == 0 &&
    OFFSET_OF (UE_SEGMENT, ImageInfo) == OFFSET_OF (UE_SEGMENT_XIP, ImageInfo),
    "Below's logic assumes the given layout."
    );

  for (
    SegmentImageInfoPtr = (CONST UINT8 *) SegmentImageInfos + SegmentIterSize;
    SegmentImageInfoPtr < (CONST UINT8 *) SegmentImageInfos + (UINT32) SegmentIterSize * NumSegments;
    ImageSizeRecord += SegmentSize,
    SegmentImageInfoPtr += SegmentIterSize
    ) {
    SegmentImageInfo = *(CONST UINT32 *) SegmentImageInfoPtr;

    SegmentSize        = UE_SEGMENT_SIZE (SegmentImageInfo);
    SegmentPermissions = UE_SEGMENT_PERMISSIONS (SegmentImageInfo);
    //
    // Skip Image segments with the same memory permissions as the current range
    // as they can be merged.
    //
    if (SegmentPermissions == Permissions) {
      RangeSize += SegmentSize;
      continue;
    }
    //
    // Create an Image record LoadTable for the current memory permission range.
    //
    RecordSegment = &ImageRecord->Segments[NumRecordSegments];
    RecordSegment->Size       = RangeSize;
    RecordSegment->Attributes = InternalPermissionsToAttributes (Permissions);
    ++NumRecordSegments;
    //
    // Start a Image record LoadTable with the current Image LoadTable.
    //
    RangeSize   = SegmentSize;
    Permissions = SegmentPermissions;
  }
  //
  // Create an Image record LoadTable for the current memory permission range.
  //
  RecordSegment = &ImageRecord->Segments[NumRecordSegments];
  RecordSegment->Size       = RangeSize;
  RecordSegment->Attributes = InternalPermissionsToAttributes (Permissions);
  ++NumRecordSegments;

  ImageAddress = UeLoaderGetImageAddress (&Context->Ctx.Ue);
  ImageSize    = UeGetImageSize (&Context->Ctx.Ue);
  ASSERT (ImageSize == ImageSizeRecord);

  ImageRecord->NumSegments  = NumRecordSegments;
  ImageRecord->StartAddress = ImageAddress;
  ImageRecord->EndAddress   = ImageAddress + ImageSize;
  //
  // Zero the remaining array entries to avoid uninitialised data.
  //
  ZeroMem (
    ImageRecord->Segments + NumRecordSegments,
    (NumSegments - NumRecordSegments) * sizeof (*ImageRecord->Segments)
    );

  return ImageRecord;
}

// LCOV_EXCL_START
RETURN_STATUS
UefiImageDebugLocateImageUe (
  OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN  UINTN                            Address,
  IN  UINT8                            ImageOrigin
  )
{
  ASSERT (Context != NULL);
  (VOID) Address;
  //
  // UE does not support this feature.
  //
  return RETURN_NOT_FOUND;
}
// LCOV_EXCL_STOP

RETURN_STATUS
UefiImageGetFixedAddressUe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    UINT64                           *Address
  )
{
  BOOLEAN  FixedAddress;

  ASSERT (Context != NULL);
  ASSERT (Address != NULL);

  FixedAddress = UeGetFixedAddress (&Context->Ctx.Ue);
  if (!FixedAddress) {
    return RETURN_NOT_FOUND;
  }

  *Address = Context->Ctx.Ue.BaseAddress;
  return RETURN_SUCCESS;
}

VOID
UefiImageDebugPrintSegmentsUe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  RETURN_STATUS          Status;
  CONST CHAR8            *Name;
  CONST UE_SEGMENT       *Segments;
  UINT16                 NumSegments;
  UINT16                 SegmentIndex;
  UINT32                 SegmentFileOffset;
  UINT32                 SegmentImageAddress;
  CONST UE_SEGMENT_NAME  *NameTable;
  UINT32                 ImageSize;

  NumSegments = UeGetSegments (&Context->Ctx.Ue, &Segments);

  Status = UeGetSegmentNames (&Context->Ctx.Ue, &NameTable);
  if (RETURN_ERROR (Status)) {
    NameTable = NULL;
  }

  SegmentFileOffset = Context->Ctx.Ue.SegmentsFileOffset;
  //
  // The first Image segment must begin the Image memory space.
  //
  SegmentImageAddress = 0;

  for (SegmentIndex = 0; SegmentIndex < NumSegments; ++SegmentIndex) {
    if (NameTable != NULL) {
      Name = (CONST CHAR8 *)NameTable[SegmentIndex];
    } else {
      STATIC_ASSERT (
        sizeof (*NameTable) == sizeof ("Unknown"),
        "The following may cause prohibited memory accesses."
        );

      Name = "Unknown";
    }

    ImageSize = UE_SEGMENT_SIZE (Segments[SegmentIndex].ImageInfo);

    DEBUG ((
      DEBUG_VERBOSE,
      "  Segment - '%c%c%c%c%c%c%c%c'\n"
      "  ImageAddress - 0x%08x\n"
      "  ImageSize    - 0x%08x\n"
      "  FileOffset    - 0x%08x\n"
      "  FileSize      - 0x%08x\n"
      "  Permissions   - 0x%08x\n",
      Name[0], Name[1], Name[2], Name[3], Name[4], Name[5], Name[6], Name[7],
      SegmentImageAddress,
      ImageSize,
      SegmentFileOffset,
      Segments[SegmentIndex].FileSize,
      UE_SEGMENT_PERMISSIONS (Segments[SegmentIndex].ImageInfo)
      ));

    SegmentImageAddress += ImageSize;
    SegmentFileOffset   += Segments[SegmentIndex].FileSize;
  }
}

GLOBAL_REMOVE_IF_UNREFERENCED CONST UEFI_IMAGE_FORMAT_SUPPORT mUeSupport = {
  UefiImageInitializeContextPreHashUe,
  UefiImageHashImageDefaultUe,
  UefiImageInitializeContextPostHashUe,
  UefiImageLoadImageUe,
  UefiImageImageIsInplaceUe,
  UefiImageLoadImageInplaceUe,
  UefiImageRelocateImageInplaceUe,
  UefiImageLoaderGetRuntimeContextSizeUe,
  UefiImageRelocateImageUe,
  UefiImageRuntimeRelocateImageUe,
  UefiImageDiscardSegmentsUe,
  UefiImageGetSymbolsPathUe,
  UefiImageGetFirstCertificateUe,
  UefiImageGetNextCertificateUe,
  UefiImageGetHiiDataRvaUe,
  UefiImageGetEntryPointAddressUe,
  UefiImageGetMachineUe,
  UefiImageGetSubsystemUe,
  UefiImageGetSegmentAlignmentUe,
  UefiImageGetImageSizeUe,
  UefiImageGetBaseAddressUe,
  UefiImageGetRelocsStrippedUe,
  UefiImageLoaderGetImageAddressUe,
  UefiImageLoaderGetDebugAddressUe,
  UefiImageLoaderGetImageRecordUe,
  UefiImageDebugLocateImageUe,
  UefiImageGetFixedAddressUe,
  UefiImageDebugPrintSegmentsUe
};
