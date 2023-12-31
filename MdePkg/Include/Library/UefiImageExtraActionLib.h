/** @file
  Provides services to perform additional actions when an UEFI image is loaded
  or unloaded.  This is useful for environment where symbols need to be loaded
  and unloaded to support source level debugging.

  Copyright (c) 2009 - 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __UEFI_IMAGE_EXTRA_ACTION_LIB_H__
#define __UEFI_IMAGE_EXTRA_ACTION_LIB_H__

#include <Library/UefiImageLib.h>

/**
  Performs additional actions after a UEFI image has been loaded and relocated.

  If ImageContext is NULL, then ASSERT().

  @param  ImageContext  Pointer to the image context structure that describes the
                        UEFI image that has already been loaded and relocated.

**/
VOID
EFIAPI
UefiImageLoaderRelocateImageExtraAction (
  IN CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *ImageContext
  );

/**
  Performs additional actions just before a UEFI image is unloaded.  Any resources
  that were allocated by UefiImageLoaderRelocateImageExtraAction() must be freed.

  If ImageContext is NULL, then ASSERT().

  @param  ImageContext  Pointer to the image context structure that describes the
                        UEFI image that is being unloaded.

**/
VOID
EFIAPI
UefiImageLoaderUnloadImageExtraAction (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *ImageContext
  );

#endif
