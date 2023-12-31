/** @file
  Support for functions common to all Image formats.

  Copyright (c) 2021, Marvin Häuser. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-3-Clause
**/

#include <Base.h>
#include <Uefi/UefiBaseType.h>
#include <Uefi/UefiSpec.h>

#include <Library/BaseOverflowLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiImageLib.h>

RETURN_STATUS
UefiImageRelocateImageInplaceForExecution (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  RETURN_STATUS Status;
  UINTN         ImageAddress;
  UINTN         ImageSize;

  Status = UefiImageRelocateImageInplace (Context);
  if (RETURN_ERROR (Status)) {
    return Status;
  }

  ImageAddress = UefiImageLoaderGetImageAddress (Context);
  ImageSize    = UefiImageGetImageSize (Context);
  //
  // Flush the instruction cache so the image data is written before
  // execution.
  //
  InvalidateInstructionCacheRange ((VOID *) ImageAddress, ImageSize);

  return RETURN_SUCCESS;
}

// FIXME: Check Subsystem here
RETURN_STATUS
UefiImageLoadImageForExecution (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT    *Context,
  OUT    VOID                               *Destination,
  IN     UINT32                             DestinationSize,
  OUT    UEFI_IMAGE_LOADER_RUNTIME_CONTEXT  *RuntimeContext OPTIONAL,
  IN     UINT32                             RuntimeContextSize
  )
{
  RETURN_STATUS Status;
  UINTN         BaseAddress;
  UINTN         SizeOfImage;
  //
  // Load the Image into the memory space.
  //
  Status = UefiImageLoadImage (Context, Destination, DestinationSize);
  if (RETURN_ERROR (Status)) {
    return Status;
  }
  //
  // Relocate the Image to the address it has been loaded to.
  //
  BaseAddress = UefiImageLoaderGetImageAddress (Context);
  Status = UefiImageRelocateImage (
             Context,
             BaseAddress,
             RuntimeContext,
             RuntimeContextSize
             );
  if (RETURN_ERROR (Status)) {
    return Status;
  }

  SizeOfImage = UefiImageGetImageSize (Context);
  //
  // Flush the instruction cache so the image data is written before execution.
  //
  InvalidateInstructionCacheRange ((VOID *) BaseAddress, SizeOfImage);

  return RETURN_SUCCESS;
}

RETURN_STATUS
UefiImageRuntimeRelocateImageForExecution (
  IN OUT VOID                                     *Image,
  IN     UINT32                                   ImageSize,
  IN     UINT64                                   BaseAddress,
  IN     CONST UEFI_IMAGE_LOADER_RUNTIME_CONTEXT  *RuntimeContext
  )
{
  RETURN_STATUS Status;
  //
  // Relocate the Image to the new address.
  //
  Status = UefiImageRuntimeRelocateImage (
             Image,
             ImageSize,
             BaseAddress,
             RuntimeContext
             );
  if (RETURN_ERROR (Status)) {
    return Status;
  }
  //
  // Flush the instruction cache so the image data is written before execution.
  //
  InvalidateInstructionCacheRange (Image, ImageSize);

  return RETURN_SUCCESS;
}
