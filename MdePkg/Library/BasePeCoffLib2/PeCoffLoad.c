/** @file
  Implements APIs to load PE/COFF Images.

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

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/PeCoffLib2.h>

#include "BasePeCoffLib2Internals.h"

/**
  Loads the Image sections into the memory space and initialises any padding
  with zeros.

  @param[in]  Context           The context describing the Image. Must have been
                                initialised by PeCoffInitializeContext().
  @param[in]  LoadedHeaderSize  The size, in Bytes, of the loaded Image Headers.
  @param[in]  DestinationSize   The size, in Bytes, of Destination. Must be
                                sufficent to load the Image with regards to its
                                Image section alignment.
**/
STATIC
VOID
InternalLoadSections (
  IN  CONST PE_COFF_LOADER_IMAGE_CONTEXT  *Context,
  IN  UINT32                              LoadedHeaderSize,
  IN  UINT32                              DestinationSize
  )
{
  CONST EFI_IMAGE_SECTION_HEADER *Sections;
  UINT16                         SectionIndex;
  UINT32                         EffectivePointerToRawData;
  UINT32                         DataSize;
  UINT32                         PreviousTopRva;

  Sections = (CONST EFI_IMAGE_SECTION_HEADER *) (CONST VOID *) (
               (CONST CHAR8 *) Context->FileBuffer + Context->SectionsOffset
               );
  //
  // As the loop zero's the data from the end of the previous section, start
  // with the size of the loaded Image Headers to zero their trailing data.
  //
  PreviousTopRva = LoadedHeaderSize;

  for (SectionIndex = 0; SectionIndex < Context->NumberOfSections; ++SectionIndex) {
    //
    // Zero from the end of the previous section to the start of this section.
    //
    ZeroMem (
      (CHAR8 *) Context->ImageBuffer + PreviousTopRva,
      Sections[SectionIndex].VirtualAddress - PreviousTopRva
      );
    //
    // Copy the maximum amount of data that fits both sizes.
    //
    if (Sections[SectionIndex].SizeOfRawData <= Sections[SectionIndex].VirtualSize) {
      DataSize = Sections[SectionIndex].SizeOfRawData;
    } else {
      DataSize = Sections[SectionIndex].VirtualSize;
    }
    //
    // Load the current Image section into the memory space.
    //
    EffectivePointerToRawData = Sections[SectionIndex].PointerToRawData;

    CopyMem (
      (CHAR8 *) Context->ImageBuffer + Sections[SectionIndex].VirtualAddress,
      (CONST CHAR8 *) Context->FileBuffer + EffectivePointerToRawData,
      DataSize
      );

    PreviousTopRva = Sections[SectionIndex].VirtualAddress + DataSize;
  }
  //
  // Zero the trailing data after the last Image section.
  //
  ZeroMem (
    (CHAR8 *) Context->ImageBuffer + PreviousTopRva,
    DestinationSize - PreviousTopRva
    );
}

RETURN_STATUS
PeCoffLoadImage (
  IN OUT PE_COFF_LOADER_IMAGE_CONTEXT  *Context,
  OUT    VOID                          *Destination,
  IN     UINT32                        DestinationSize
  )
{
  UINT32                         LoadedHeaderSize;
  CONST EFI_IMAGE_SECTION_HEADER *Sections;

  ASSERT (Context != NULL);
  ASSERT (Destination != NULL);
  ASSERT (ADDRESS_IS_ALIGNED (Destination, Context->SectionAlignment));
  ASSERT (Context->SizeOfImage <= DestinationSize);

  Context->ImageBuffer = Destination;
  //
  // Load the Image Headers into the memory space, if the policy demands it.
  //
  Sections = (CONST EFI_IMAGE_SECTION_HEADER *) (CONST VOID *) (
               (CONST CHAR8 *) Context->FileBuffer + Context->SectionsOffset
               );
  if (PcdGetBool (PcdImageLoaderLoadHeader) && Sections[0].VirtualAddress != 0) {
    LoadedHeaderSize = Context->SizeOfHeaders;
    CopyMem (Context->ImageBuffer, Context->FileBuffer, LoadedHeaderSize);
  } else {
    LoadedHeaderSize = 0;
  }
  //
  // Load all Image sections into the memory space.
  //
  InternalLoadSections (Context, LoadedHeaderSize, DestinationSize);

  return RETURN_SUCCESS;
}

RETURN_STATUS
PeCoffLoadImageInplaceNoBase (
  IN OUT PE_COFF_LOADER_IMAGE_CONTEXT  *Context
  )
{
  UINT32                         NumberOfSections;
  CONST EFI_IMAGE_SECTION_HEADER *Sections;
  UINT32                         AlignedSize;
  UINT16                         SectionIndex;

  ASSERT (Context != NULL);

  NumberOfSections = PeCoffGetSectionTable (Context, &Sections);
  //
  // Verify all RVAs and raw file offsets are identical for XIP Images.
  //
  for (SectionIndex = 0; SectionIndex < NumberOfSections; ++SectionIndex) {
    AlignedSize = ALIGN_VALUE (
                    Sections[SectionIndex].VirtualSize,
                    Context->SectionAlignment
                    );
    if (Sections[SectionIndex].PointerToRawData != Sections[SectionIndex].VirtualAddress
     || Sections[SectionIndex].SizeOfRawData != AlignedSize) {
      DEBUG_RAISE ();
      return RETURN_VOLUME_CORRUPTED;
    }
  }

  Context->ImageBuffer = (CHAR8 *) Context->FileBuffer;

  return RETURN_SUCCESS;
}

BOOLEAN
PeCoffImageIsInplace (
  IN OUT PE_COFF_LOADER_IMAGE_CONTEXT  *Context
  )
{
  UINT64 ImageBase;

  ASSERT (Context != NULL);

  ImageBase = PeCoffGetImageBase (Context);
  //
  // Verify the Image is located at its preferred load address.
  //
  return ImageBase == (UINTN) Context->FileBuffer;
}

RETURN_STATUS
PeCoffLoadImageInplace (
  IN OUT PE_COFF_LOADER_IMAGE_CONTEXT  *Context
  )
{
  BOOLEAN Result;

  ASSERT (Context != NULL);

  Result = PeCoffImageIsInplace (Context);
  if (!Result) {
    DEBUG_RAISE ();
    return RETURN_UNSUPPORTED;
  }

  return PeCoffLoadImageInplaceNoBase (Context);
}

//
// FIXME: Provide a runtime version of this API as well.
//
VOID
PeCoffDiscardSections (
  IN OUT PE_COFF_LOADER_IMAGE_CONTEXT  *Context
  )
{
  CONST EFI_IMAGE_SECTION_HEADER *Sections;
  UINT32                         SectionIndex;

  ASSERT (Context != NULL);
  //
  // By the PE/COFF specification, the .reloc section is supposed to be
  // discardable, so we must assume it is no longer valid.
  //
  Context->RelocDirRva  = 0;
  Context->RelocDirSize = 0;
  //
  // Zero all Image sections that are flagged as discardable.
  //
  Sections = (CONST EFI_IMAGE_SECTION_HEADER *) (CONST VOID *) (
               (CONST CHAR8 *) Context->FileBuffer + Context->SectionsOffset
               );
  for (SectionIndex = 0; SectionIndex < Context->NumberOfSections; ++SectionIndex) {
    if ((Sections[SectionIndex].Characteristics & EFI_IMAGE_SCN_MEM_DISCARDABLE) != 0) {
      ZeroMem (
        (CHAR8 *) Context->ImageBuffer + Sections[SectionIndex].VirtualAddress,
        Sections[SectionIndex].VirtualSize
        );
    }
  }
}
