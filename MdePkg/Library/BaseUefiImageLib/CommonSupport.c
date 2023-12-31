/** @file
  Support for functions common to all Image formats.

  Copyright (c) 2021, Marvin Häuser. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-3-Clause
**/

#include <Base.h>
#include <Uefi/UefiBaseType.h>
#include <Uefi/UefiSpec.h>

#include <Library/BaseOverflowLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiImageLib.h>

RETURN_STATUS
UefiImageInitializeContext (
  OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN  CONST VOID                       *FileBuffer,
  IN  UINT32                           FileSize,
  IN  UEFI_IMAGE_SOURCE                Source,
  IN  UINT8                            ImageOrigin
  )
{
  RETURN_STATUS Status;

  Status = UefiImageInitializeContextPreHash (
             Context,
             FileBuffer,
             FileSize,
             Source,
             ImageOrigin
             );
  if (RETURN_ERROR (Status)) {
    return Status;
  }

  return UefiImageInitializeContextPostHash (Context);
}

UINT8
UefiImageGetFormat (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return Context->FormatIndex;
}

UINTN
UefiImageLoaderGetImageEntryPoint (
  IN CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  UINTN  ImageAddress;
  UINT32 EntryPointAddress;

  ASSERT (Context != NULL);

  ImageAddress      = UefiImageLoaderGetImageAddress (Context);
  EntryPointAddress = UefiImageGetEntryPointAddress (Context);

  return ImageAddress + EntryPointAddress;
}

// FIXME: Some image prints use this and some don't. Is this really needed?
RETURN_STATUS
UefiImageGetModuleNameFromSymbolsPath (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    CHAR8                            *ModuleName,
  IN     UINT32                           ModuleNameSize
  )
{
  RETURN_STATUS Status;
  CONST CHAR8   *SymbolsPath;
  UINT32        SymbolsPathSize;
  UINTN         Index;
  UINTN         StartIndex;

  ASSERT (Context != NULL);
  ASSERT (ModuleName != NULL);
  ASSERT (3 <= ModuleNameSize);

  Status = UefiImageGetSymbolsPath (
             Context,
             &SymbolsPath,
             &SymbolsPathSize
             );
  if (RETURN_ERROR (Status)) {
    return Status;
  }
  //
  // Find the last component of the symbols path, which is the file containing
  // the debug symbols for the Image.
  //
  StartIndex = 0;
  for (Index = 0; Index < SymbolsPathSize - 1; ++Index) {
    if (SymbolsPath[Index] == '\\' || SymbolsPath[Index] == '/') {
      StartIndex = Index + 1;
    }
  }
  //
  // Extract the module name from the debug symbols file and ensure the correct
  // file extensions.
  //
  for (
    Index = 0;
    Index < MIN (ModuleNameSize, SymbolsPathSize) - 1;
    ++Index
    ) {
    ModuleName[Index] = SymbolsPath[Index + StartIndex];
    if (ModuleName[Index] == '\0') {
      ModuleName[Index] = '.';
    }
    if (ModuleName[Index] == '.') {
      if (Index >= ModuleNameSize - 4) {
        break;
      }

      ModuleName[Index + 1] = 'e';
      ModuleName[Index + 2] = 'f';
      ModuleName[Index + 3] = 'i';
      Index += 4;
      break;
    }
  }
  //
  // Terminate the newly created module name string.
  //
  ModuleName[Index] = '\0';

  return RETURN_SUCCESS;
}

VOID
UefiImageDebugPrintImageRecord (
  IN CONST UEFI_IMAGE_RECORD  *ImageRecord
  )
{
  UINT32 SegmentIndex;
  UINTN  SegmentAddress;

  ASSERT (ImageRecord != NULL);

  SegmentAddress = ImageRecord->StartAddress;
  for (
    SegmentIndex = 0;
    SegmentIndex < ImageRecord->NumSegments;
    ++SegmentIndex
    ) {
    DEBUG ((
      DEBUG_VERBOSE,
      "  RecordSegment\n"
      "  Address    - 0x%016llx\n"
      "  Size       - 0x%08x\n"
      "  Attributes - 0x%08x\n",
      (UINT64) SegmentAddress,
      ImageRecord->Segments[SegmentIndex].Size,
      ImageRecord->Segments[SegmentIndex].Attributes
      ));

    SegmentAddress += ImageRecord->Segments[SegmentIndex].Size;
  }
}
