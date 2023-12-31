/** @file
  Null PE/Coff Extra Action library instances with empty functions.

  Copyright (c) 2009 - 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <Library/UefiImageExtraActionLib.h>
#include <Library/DebugLib.h>

/**
  Performs additional actions after a PE/COFF image has been loaded and relocated.

  If ImageContext is NULL, then ASSERT().

  @param  ImageContext  The pointer to the image context structure that describes the
                        PE/COFF image that has already been loaded and relocated.

**/
VOID
EFIAPI
UefiImageLoaderRelocateImageExtraAction (
  IN CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *ImageContext
  )
{
  ASSERT (ImageContext != NULL);
}

/**
  Performs additional actions just before a PE/COFF image is unloaded.  Any resources
  that were allocated by UefiImageLoaderRelocateImageExtraAction() must be freed.

  If ImageContext is NULL, then ASSERT().

  @param  ImageContext  The pointer to the image context structure that describes the
                        PE/COFF image that is being unloaded.

**/
VOID
EFIAPI
UefiImageLoaderUnloadImageExtraAction (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *ImageContext
  )
{
  ASSERT (ImageContext != NULL);
}
