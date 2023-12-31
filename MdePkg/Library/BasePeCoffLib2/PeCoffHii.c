/** @file
  Implements APIs to retrieve UEFI HII data from PE/COFF Images.

  Copyright (c) 2020 - 2021, Marvin Häuser. All rights reserved.<BR>
  Copyright (c) 2020, Vitaly Cheptsov. All rights reserved.<BR>
  Copyright (c) 2020, ISP RAS. All rights reserved.<BR>
  Portions copyright (c) 2006 - 2019, Intel Corporation. All rights reserved.<BR>
  Portions copyright (c) 2008 - 2010, Apple Inc. All rights reserved.<BR>
  Portions copyright (c) 2020, Hewlett Packard Enterprise Development LP. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-3-Clause
**/

#include <Base.h>

#include <IndustryStandard/PeImage2.h>

#include <Library/BaseOverflowLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/PeCoffLib2.h>

#include "BasePeCoffLib2Internals.h"

RETURN_STATUS
PeCoffGetHiiDataRva (
  IN OUT PE_COFF_LOADER_IMAGE_CONTEXT  *Context,
  OUT    UINT32                        *HiiRva,
  OUT    UINT32                        *HiiSize
  )
{
  UINT16                                    Index;
  CONST EFI_IMAGE_NT_HEADERS32              *Pe32Hdr;
  CONST EFI_IMAGE_NT_HEADERS64              *Pe32PlusHdr;
  CONST EFI_IMAGE_DATA_DIRECTORY            *ResDirTable;
  CONST EFI_IMAGE_RESOURCE_DIRECTORY        *ResourceDir;
  CONST EFI_IMAGE_RESOURCE_DIRECTORY_ENTRY  *ResourceDirEntry;
  CONST EFI_IMAGE_RESOURCE_DIRECTORY_STRING *ResourceDirString;
  CONST EFI_IMAGE_RESOURCE_DATA_ENTRY       *ResourceDataEntry;
  BOOLEAN                                   Overflow;
  UINT32                                    Offset;
  UINT32                                    TopOffset;
  UINT8                                     ResourceLevel;
  UINT32                                    HiiRvaEnd;

  ASSERT (Context != NULL);
  ASSERT (HiiRva != NULL);
  ASSERT (HiiSize != NULL);
  //
  // Retrieve the Image's Resource Directory Table.
  //
  switch (Context->ImageType) {
    case PeCoffLoaderTypePe32:
      Pe32Hdr = (CONST EFI_IMAGE_NT_HEADERS32 *) (CONST VOID *) (
                  (CONST CHAR8 *) Context->FileBuffer + Context->ExeHdrOffset
                  );
      if (Pe32Hdr->NumberOfRvaAndSizes <= EFI_IMAGE_DIRECTORY_ENTRY_RESOURCE) {
        return RETURN_NOT_FOUND;
      }

      ResDirTable = &Pe32Hdr->DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_RESOURCE];
      break;

    case PeCoffLoaderTypePe32Plus:
      Pe32PlusHdr = (CONST EFI_IMAGE_NT_HEADERS64 *) (CONST VOID *) (
                      (CONST CHAR8 *) Context->FileBuffer + Context->ExeHdrOffset
                      );
      if (Pe32PlusHdr->NumberOfRvaAndSizes <= EFI_IMAGE_DIRECTORY_ENTRY_RESOURCE) {
        return RETURN_NOT_FOUND;
      }

      ResDirTable = &Pe32PlusHdr->DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_RESOURCE];
      break;

    default:
      ASSERT (FALSE);
      return RETURN_UNSUPPORTED;
  }
  //
  // Verify the Resource Directory Table contains at least one entry.
  //
  if (ResDirTable->Size < sizeof (EFI_IMAGE_RESOURCE_DIRECTORY) + sizeof (*ResourceDir->Entries)) {
    return RETURN_NOT_FOUND;
  }
  //
  // Verify the start of the Resource Directory Table is sufficiently aligned.
  //
  if (!IS_ALIGNED (ResDirTable->VirtualAddress, ALIGNOF (EFI_IMAGE_RESOURCE_DIRECTORY))) {
    DEBUG_RAISE ();
    return RETURN_VOLUME_CORRUPTED;
  }
  // FIXME: Verify against first Image section / Headers due to XIP TE.
  //
  // Verify the Resource Directory Table is in bounds of the Image buffer.
  //
  Overflow = BaseOverflowAddU32 (
               ResDirTable->VirtualAddress,
               ResDirTable->Size,
               &TopOffset
               );
  if (Overflow || TopOffset > Context->SizeOfImage) {
    DEBUG_RAISE ();
    return RETURN_VOLUME_CORRUPTED;
  }

  ResourceDir = (CONST EFI_IMAGE_RESOURCE_DIRECTORY *) (CONST VOID *) (
                  (CONST CHAR8 *) Context->ImageBuffer + ResDirTable->VirtualAddress
                  );
  //
  // Verify the Resource Directory Table can hold all of its entries.
  //
  STATIC_ASSERT (
    sizeof (*ResourceDirEntry) * MAX_UINT16 <= ((UINT64) MAX_UINT32 + 1U) / 2 - sizeof (EFI_IMAGE_RESOURCE_DIRECTORY),
    "The following arithmetic may overflow."
    );
  TopOffset = sizeof (EFI_IMAGE_RESOURCE_DIRECTORY) + sizeof (*ResourceDirEntry) *
                ((UINT32) ResourceDir->NumberOfNamedEntries + ResourceDir->NumberOfIdEntries);
  if (TopOffset > ResDirTable->Size) {
    DEBUG_RAISE ();
    return RETURN_VOLUME_CORRUPTED;
  }
  //
  // Try to locate the "HII" Resource entry.
  //
  for (Index = 0; Index < ResourceDir->NumberOfNamedEntries; ++Index) {
    ResourceDirEntry = &ResourceDir->Entries[Index];
    //
    // Filter entries with a non-Unicode name entry.
    //
    if (ResourceDirEntry->u1.s.NameIsString == 0) {
      continue;
    }
    //
    // Verify the Resource Directory String header is in bounds of the Resource
    // Directory Table.
    //
    Overflow = BaseOverflowAddU32 (
                 ResourceDirEntry->u1.s.NameOffset,
                 sizeof (*ResourceDirString),
                 &TopOffset
                 );
    if (Overflow || TopOffset > ResDirTable->Size) {
      DEBUG_RAISE ();
      return RETURN_VOLUME_CORRUPTED;
    }
    //
    // Verify the Resource Directory String offset is sufficiently aligned.
    //
    Offset = ResDirTable->VirtualAddress + ResourceDirEntry->u1.s.NameOffset;
    if (!IS_ALIGNED (Offset, ALIGNOF (EFI_IMAGE_RESOURCE_DIRECTORY_STRING))) {
      DEBUG_RAISE ();
      return RETURN_VOLUME_CORRUPTED;
    }

    ResourceDirString = (CONST EFI_IMAGE_RESOURCE_DIRECTORY_STRING *) (CONST VOID *) (
                          (CONST CHAR8 *) Context->ImageBuffer + Offset
                          );
    //
    // Verify the Resource Directory String is in bounds of the Resource
    // Directory Table.
    //
    Overflow = BaseOverflowAddU32 (
                 TopOffset,
                 (UINT32) ResourceDirString->Length * sizeof (CHAR16),
                 &TopOffset
                 );
    if (Overflow || TopOffset > ResDirTable->Size) {
      DEBUG_RAISE ();
      return RETURN_VOLUME_CORRUPTED;
    }
    //
    // Verify the type name matches "HII".
    //
    if (ResourceDirString->Length == 3
     && ResourceDirString->String[0] == L'H'
     && ResourceDirString->String[1] == L'I'
     && ResourceDirString->String[2] == L'I') {
      break;
    }
  }
  //
  // Verify the "HII" Type Resource Directory Entry exists.
  //
  if (Index == ResourceDir->NumberOfNamedEntries) {
    return RETURN_NOT_FOUND;
  }
  //
  // Walk down the conventional "Name" and "Language" levels to reach the
  // data directory.
  //
  for (ResourceLevel = 0; ResourceLevel < 2; ++ResourceLevel) {
    //
    // Succeed early if one of the levels is omitted.
    //
    if (ResourceDirEntry->u2.s.DataIsDirectory == 0) {
      break;
    }
    //
    // Verify the Resource Directory Table fits at least the Resource Directory
    // with and one Relocation Directory Entry.
    //
    if (ResourceDirEntry->u2.s.OffsetToDirectory > ResDirTable->Size - sizeof (EFI_IMAGE_RESOURCE_DIRECTORY) + sizeof (*ResourceDir->Entries)) {
      DEBUG_RAISE ();
      return RETURN_VOLUME_CORRUPTED;
    }
    //
    // Verify the next Relocation Directory offset is sufficiently aligned.
    //
    Offset = ResDirTable->VirtualAddress + ResourceDirEntry->u2.s.OffsetToDirectory;
    if (!IS_ALIGNED (Offset, ALIGNOF (EFI_IMAGE_RESOURCE_DIRECTORY))) {
      DEBUG_RAISE ();
      return RETURN_VOLUME_CORRUPTED;
    }
    //
    // Verify the Resource Directory has at least one entry.
    //
    ResourceDir = (CONST EFI_IMAGE_RESOURCE_DIRECTORY *) (CONST VOID *) (
                    (CONST CHAR8 *) Context->ImageBuffer + Offset
                    );
    if ((UINT32) ResourceDir->NumberOfIdEntries + ResourceDir->NumberOfNamedEntries == 0) {
      DEBUG_RAISE ();
      return RETURN_VOLUME_CORRUPTED;
    }
    //
    // Always take the first entry for simplicity.
    //
    ResourceDirEntry = &ResourceDir->Entries[0];
  }
  //
  // Verify the final Resource Directory Entry is of a data type.
  //
  if (ResourceDirEntry->u2.s.DataIsDirectory != 0) {
    DEBUG_RAISE ();
    return RETURN_VOLUME_CORRUPTED;
  }
  //
  // Verify the Resource Directory Table fits at least the Resource Directory.
  //
  STATIC_ASSERT (
    sizeof (EFI_IMAGE_RESOURCE_DATA_ENTRY) <= sizeof (EFI_IMAGE_RESOURCE_DIRECTORY),
    "The following arithmetic may overflow."
    );
  if (ResourceDirEntry->u2.OffsetToData > ResDirTable->Size - sizeof (EFI_IMAGE_RESOURCE_DATA_ENTRY)) {
    DEBUG_RAISE ();
    return RETURN_VOLUME_CORRUPTED;
  }
  //
  // Verify the Relocation Directory Entry offset is sufficiently aligned.
  //
  Offset = ResDirTable->VirtualAddress + ResourceDirEntry->u2.OffsetToData;
  if (!IS_ALIGNED (Offset, ALIGNOF (EFI_IMAGE_RESOURCE_DATA_ENTRY))) {
    DEBUG_RAISE ();
    return RETURN_VOLUME_CORRUPTED;
  }

  ResourceDataEntry = (CONST EFI_IMAGE_RESOURCE_DATA_ENTRY *) (CONST VOID *) (
                        (CONST CHAR8 *) Context->ImageBuffer + Offset
                        );
  //
  // Verify the "HII" data is in bounds of the Image buffer.
  //
  Overflow = BaseOverflowAddU32 (
               ResourceDataEntry->OffsetToData,
               ResourceDataEntry->Size,
               &HiiRvaEnd
               );
  if (Overflow || HiiRvaEnd > Context->SizeOfImage) {
    DEBUG_RAISE ();
    return RETURN_VOLUME_CORRUPTED;
  }

  *HiiRva  = ResourceDataEntry->OffsetToData;
  *HiiSize = ResourceDataEntry->Size;

  return RETURN_SUCCESS;
}
