/** @file
  UEFI Image Loader library implementation for PE/COFF Images.

  Copyright (c) 2021, Marvin Häuser. All rights reserved.<BR>

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
#include <Library/PeCoffLib2.h>
#include <Library/UefiImageLib.h>
#include <Library/PcdLib.h>

#include "PeSupport.h"

RETURN_STATUS
UefiImageInitializeContextPreHashPe (
  OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN  CONST VOID                       *FileBuffer,
  IN  UINT32                           FileSize,
  IN  UINT8                            ImageOrigin
  )
{
  return PeCoffInitializeContext (&Context->Ctx.Pe, FileBuffer, FileSize, ImageOrigin);
}

BOOLEAN
UefiImageHashImageDefaultPe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN OUT VOID                             *HashContext,
  IN     UEFI_IMAGE_LOADER_HASH_UPDATE    HashUpdate
  )
{
  return PeCoffHashImageAuthenticode (&Context->Ctx.Pe, HashContext, HashUpdate);
}

RETURN_STATUS
UefiImageInitializeContextPostHashPe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return RETURN_SUCCESS;
}

RETURN_STATUS
UefiImageLoadImagePe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    VOID                             *Destination,
  IN     UINT32                           DestinationSize
  )
{
  return PeCoffLoadImage (&Context->Ctx.Pe, Destination, DestinationSize);
}

BOOLEAN
UefiImageImageIsInplacePe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return PeCoffImageIsInplace (&Context->Ctx.Pe);
}

RETURN_STATUS
UefiImageLoadImageInplacePe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return PeCoffLoadImageInplace (&Context->Ctx.Pe);
}

RETURN_STATUS
UefiImageRelocateImageInplacePe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return PeCoffRelocateImageInplace (&Context->Ctx.Pe);
}

RETURN_STATUS
UefiImageLoaderGetRuntimeContextSizePe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    UINT32                           *Size
  )
{
  return PeCoffLoaderGetRuntimeContextSize (&Context->Ctx.Pe, Size);
}

RETURN_STATUS
UefiImageRelocateImagePe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN     UINT64                           BaseAddress,
  OUT    VOID                             *RuntimeContext OPTIONAL,
  IN     UINT32                           RuntimeContextSize
  )
{
  return PeCoffRelocateImage (
           &Context->Ctx.Pe,
           BaseAddress,
           (PE_COFF_LOADER_RUNTIME_CONTEXT *)RuntimeContext,
           RuntimeContextSize
           );
}

RETURN_STATUS
UefiImageRuntimeRelocateImagePe (
  IN OUT VOID        *Image,
  IN     UINT32      ImageSize,
  IN     UINT64      BaseAddress,
  IN     CONST VOID  *RuntimeContext
  )
{
  return PeCoffRuntimeRelocateImage (
           Image,
           ImageSize,
           BaseAddress,
           (CONST PE_COFF_LOADER_RUNTIME_CONTEXT *)RuntimeContext
           );
}

VOID
UefiImageDiscardSegmentsPe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  PeCoffDiscardSections (&Context->Ctx.Pe);
}

RETURN_STATUS
UefiImageGetSymbolsPathPe (
  IN  CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT CONST CHAR8                            **SymbolsPath,
  OUT UINT32                                 *SymbolsPathSize
  )
{
  return PeCoffGetPdbPath (&Context->Ctx.Pe, SymbolsPath, SymbolsPathSize);
}

RETURN_STATUS
UefiImageGetFirstCertificatePe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    CONST WIN_CERTIFICATE            **Certificate
  )
{
  return PeCoffGetFirstCertificate (&Context->Ctx.Pe, Certificate);
}

RETURN_STATUS
UefiImageGetNextCertificatePe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN OUT CONST WIN_CERTIFICATE            **Certificate
  )
{
  return PeCoffGetNextCertificate (&Context->Ctx.Pe, Certificate);
}

RETURN_STATUS
UefiImageGetHiiDataRvaPe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    UINT32                           *HiiRva,
  OUT    UINT32                           *HiiSize
  )
{
  return PeCoffGetHiiDataRva (&Context->Ctx.Pe, HiiRva, HiiSize);
}

UINT32
UefiImageGetEntryPointAddressPe (
  IN CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return PeCoffGetAddressOfEntryPoint (&Context->Ctx.Pe);
}

UINT16
UefiImageGetMachinePe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return PeCoffGetMachine (&Context->Ctx.Pe);
}

UINT16
UefiImageGetSubsystemPe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return PeCoffGetSubsystem (&Context->Ctx.Pe);
}

UINT32
UefiImageGetSegmentAlignmentPe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return PeCoffGetSectionAlignment (&Context->Ctx.Pe);
}

UINT32
UefiImageGetImageSizePe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return PeCoffGetSizeOfImage (&Context->Ctx.Pe);
}

UINT64
UefiImageGetBaseAddressPe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return PeCoffGetImageBase (&Context->Ctx.Pe);
}

BOOLEAN
UefiImageGetRelocsStrippedPe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return PeCoffGetRelocsStripped (&Context->Ctx.Pe);
}

UINTN
UefiImageLoaderGetImageAddressPe (
  IN CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return PeCoffLoaderGetImageAddress (&Context->Ctx.Pe);
}

UINTN
UefiImageLoaderGetDebugAddressPe (
  IN CONST UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  return PeCoffLoaderGetImageAddress (&Context->Ctx.Pe);
}

/**
  Retrieves the memory protection attributes corresponding to PE/COFF Image
  section permissions.

  @param[in] Characteristics  The PE/COFF Image section permissions

  @returns  The memory protection attributes corresponding to the PE/COFF Image
            section permissions.
**/
STATIC
UINT32
InternalCharacteristicsToAttributes (
  IN UINT32  Characteristics
  )
{
  UINT32 Attributes;

  if (PcdGetBool (PcdImageLoaderRemoveXForWX) && (Characteristics & (EFI_IMAGE_SCN_MEM_EXECUTE | EFI_IMAGE_SCN_MEM_WRITE)) == (EFI_IMAGE_SCN_MEM_EXECUTE | EFI_IMAGE_SCN_MEM_WRITE)) {
    Characteristics &= ~EFI_IMAGE_SCN_MEM_EXECUTE;
  }

  Attributes = 0;
  if ((Characteristics & EFI_IMAGE_SCN_MEM_READ) == 0) {
    Attributes |= EFI_MEMORY_RP;
  }
  if ((Characteristics & EFI_IMAGE_SCN_MEM_EXECUTE) == 0) {
    Attributes |= EFI_MEMORY_XP;
  }
  if ((Characteristics & EFI_IMAGE_SCN_MEM_WRITE) == 0) {
    Attributes |= EFI_MEMORY_RO;
  }

  return Attributes;
}

/**
  Index the read-only padding following an Image record section, if existent.

  @param[in,out] RecordSegment  The Image record section for the current memory
                                protection range. May be extended if it is of
                                the same type as the adjacent padding. At least
                                one more record section must be reserved after
                                it in order to index the read-only padding.
  @param[in]     NextAddress    The start address of the next memory permission
                                range. This may be the end address of the Image
                                in order to cover the Image trailer.
  @param[in]     EndAddress     The end address of the current memory permission
                                range. This also denotes the start of the added
                                read-only padding.
  @param[in]     Attributes     The memory protection attributes of the current
                                memory permission range.

  @returns  The amount of Image record sections that have been appended.
**/
STATIC
UINT8
InternalInsertImageRecordSegmentPadding (
  IN OUT UEFI_IMAGE_RECORD_SEGMENT  *RecordSegment,
  IN     UINT32                     EndAddress,
  IN     UINT32                     NextAddress,
  IN     UINT32                     Attributes
  )
{
  ASSERT (RecordSegment != NULL);
  ASSERT (EndAddress <= NextAddress);

  if (NextAddress == EndAddress) {
    return 0;
  }
  //
  // Add a new Image record section or expand the previous one depending on
  // the the permissions of the previous Image record section.
  //
  if (Attributes == (EFI_MEMORY_XP | EFI_MEMORY_RO)) {
    RecordSegment->Size += NextAddress - EndAddress;

    return 0;
  }

  ++RecordSegment;
  RecordSegment->Size       = NextAddress - EndAddress;
  RecordSegment->Attributes = EFI_MEMORY_XP | EFI_MEMORY_RO;

  return 1;
}

UEFI_IMAGE_RECORD *
UefiImageLoaderGetImageRecordPe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  UEFI_IMAGE_RECORD              *ImageRecord;
  UINT32                         MaxNumRecordSegments;
  UINT32                         NumRecordSegments;
  UEFI_IMAGE_RECORD_SEGMENT      *RecordSegment;
  UINTN                          ImageAddress;
  UINT32                         SizeOfImage;
  UINT32                         SectionAlignment;
  CONST EFI_IMAGE_SECTION_HEADER *Sections;
  UINT16                         NumberOfSections;
  UINT16                         SectionIndex;
  CONST EFI_IMAGE_SECTION_HEADER *Section;
  UINT32                         SectionAddress;
  UINT32                         SectionSize;
  UINT32                         SectionCharacteristics;
  UINT32                         StartAddress;
  UINT32                         EndAddress;
  UINT32                         Characteristics;
  UINT32                         Attributes;

  ASSERT (Context != NULL);
  //
  // Determine the maximum amount of Image record sections and allocate the
  // Image record.
  //
  NumberOfSections = PeCoffGetSectionTable (&Context->Ctx.Pe, &Sections);

  STATIC_ASSERT (
    MAX_UINT16 <= MAX_UINT32 / 2 - 1,
    "The following arithmetic may overflow."
    );

  if ((PcdGet32 (PcdImageLoaderAlignmentPolicy) & PCD_ALIGNMENT_POLICY_CONTIGUOUS_SECTIONS) == 0) {
    //
    // In case of contiguous Image sections, there can be two additional record
    // sections (Image Headers and trailer, e.g. debug information).
    //
    MaxNumRecordSegments = (UINT32) NumberOfSections + 2;
  } else {
    //
    // In case of possibly non-contiguous Image sections, there can be a trailer
    // per Image section (the last Image section's trailer is the same as the
    // Image trailer), as well as additionally the Image Headers.
    //
    MaxNumRecordSegments = (UINT32) NumberOfSections * 2 + 1;
  }

  ImageRecord = AllocatePool (
                  sizeof (*ImageRecord)
                    + MaxNumRecordSegments * sizeof (*ImageRecord->Segments)
                  );
  if (ImageRecord == NULL) {
    return NULL;
  }

  ImageRecord->Signature = UEFI_IMAGE_RECORD_SIGNATURE;
  InitializeListHead (&ImageRecord->Link);

  SectionAlignment = PeCoffGetSectionAlignment (&Context->Ctx.Pe);
  //
  // Map the Image Headers as read-only data. If the first Image section is
  // loaded at the start of the Image memory space, the condition
  // SectionAddress != StartAddress does not hold and these definitions will be
  // ignored.
  //
  StartAddress    = 0;
  EndAddress      = PeCoffGetSizeOfHeaders (&Context->Ctx.Pe);
  Characteristics = EFI_IMAGE_SCN_MEM_READ;
  Attributes      = EFI_MEMORY_XP | EFI_MEMORY_RO;
  ASSERT (Attributes == InternalCharacteristicsToAttributes (Characteristics));
  //
  // Create an Image record section for every permission-distinct range of the
  // Image. The current range [StartAddress, EndAddress) shares the memory
  // permissions Charactersitics/Attributes and is extended till a new
  // memory permission configuration is required. Headers and trailers treated
  // as read-only data.
  //
  NumRecordSegments = 0;
  for (SectionIndex = 0; SectionIndex < NumberOfSections; ++SectionIndex) {
    Section = Sections + SectionIndex;
    //
    // Skip empty Image sections to avoid unnecessary splitting.
    //
    if (Section->VirtualSize == 0) {
      continue;
    }
    //
    // These arithmetics are safe as guaranteed by PeCoffInitializeContext().
    //
    SectionAddress = Section->VirtualAddress;
    SectionSize    = ALIGN_VALUE (Section->VirtualSize, SectionAlignment);
    SectionCharacteristics = Section->Characteristics & (EFI_IMAGE_SCN_MEM_EXECUTE | EFI_IMAGE_SCN_MEM_READ | EFI_IMAGE_SCN_MEM_WRITE);
    //
    // Skip Image sections with the same memory permissions as the current range
    // as they can be merged. For this, the Image sections must be adjacent, or
    // the range must have the same memory permissions as the padding inbetween
    // (read-only).
    //
    if ((PcdGet32 (PcdImageLoaderAlignmentPolicy) & PCD_ALIGNMENT_POLICY_CONTIGUOUS_SECTIONS) == 0
     || SectionAddress == EndAddress
     || Characteristics == EFI_IMAGE_SCN_MEM_READ) {
      if (SectionCharacteristics == Characteristics) {
        EndAddress = SectionAddress + SectionSize;
        continue;
      }
    }
    //
    // Only create an entry if the range is not empty, otherwise discard it and
    // start a new one. Even with skipping empty Image sections, this can still
    // happen for the Image Headers when the first Image section starts at 0.
    //
    if (SectionAddress != StartAddress) {
      //
      // Create an Image record section for the current memory permission range.
      //
      RecordSegment = &ImageRecord->Segments[NumRecordSegments];
      RecordSegment->Size       = EndAddress - StartAddress;
      RecordSegment->Attributes = Attributes;
      ++NumRecordSegments;
      //
      // If the previous range is not adjacent to the current Image section,
      // report the padding as read-only data.
      //
      if ((PcdGet32 (PcdImageLoaderAlignmentPolicy) & PCD_ALIGNMENT_POLICY_CONTIGUOUS_SECTIONS) != 0) {
        NumRecordSegments += InternalInsertImageRecordSegmentPadding (
                               RecordSegment,
                               EndAddress,
                               SectionAddress,
                               Attributes
                               );
      }

      StartAddress = SectionAddress;
    }
    //
    // Start a Image record section with the current Image section.
    //
    EndAddress      = SectionAddress + SectionSize;
    Characteristics = SectionCharacteristics;
    Attributes      = InternalCharacteristicsToAttributes (Characteristics);
  }
  //
  // Image Record sections are only created once a non-empty Image section is
  // encountered that requests a different memory permission configuration.
  // As such, the last memory permission range is never converted in the loop.
  // If the loop never produced such, this is true for the Image Headers, which
  // cannot be empty.
  //
  ASSERT (StartAddress < EndAddress);
  //
  // Create an Image record section for the last Image memory permission range.
  //
  RecordSegment = &ImageRecord->Segments[NumRecordSegments];
  RecordSegment->Size       = EndAddress - StartAddress;
  RecordSegment->Attributes = Attributes;
  ++NumRecordSegments;

  ImageAddress = PeCoffLoaderGetImageAddress (&Context->Ctx.Pe);
  SizeOfImage  = PeCoffGetSizeOfImage (&Context->Ctx.Pe);
  //
  // The Image trailer, if existent, is treated as padding and as such is
  // reported as read-only data, as intended. Because it is not part of the
  // original Image memory space, this needs to happen whether Image sections
  // are guaranteed to be contiguously form the entire Image memory space or
  // not.
  //
  NumRecordSegments += InternalInsertImageRecordSegmentPadding (
                         RecordSegment,
                         EndAddress,
                         SizeOfImage,
                         Attributes
                         );

  ImageRecord->NumSegments  = NumRecordSegments;
  ImageRecord->StartAddress = ImageAddress;
  ImageRecord->EndAddress   = ImageAddress + SizeOfImage;
  //
  // Zero the remaining array entries to avoid uninitialised data.
  //
  ZeroMem (
    ImageRecord->Segments + NumRecordSegments,
    (MaxNumRecordSegments - NumRecordSegments) * sizeof (*ImageRecord->Segments)
    );

  return ImageRecord;
}

// FIXME: Docs
STATIC
RETURN_STATUS
InternalDebugLocateImage (
  OUT PE_COFF_LOADER_IMAGE_CONTEXT  *Context,
  IN  CHAR8                         *Buffer,
  IN  UINTN                         Address,
  IN  BOOLEAN                       Recurse,
  IN  UINT8                         ImageOrigin
  )
{
  RETURN_STATUS                Status;
  RETURN_STATUS                DosStatus;
  PE_COFF_LOADER_IMAGE_CONTEXT DosContext;

  ASSERT (((UINTN) Buffer & 3U) == 0);
  //
  // Align the search buffer to a 4 Byte boundary.
  //
  // Search for the Image Header in 4 Byte steps. All dynamically loaded
  // Images start at a page boundary to allow for Image section protection,
  // but XIP Images may not. As all Image Headers are at least 4 Byte aligned
  // due to natural alignment, even XIP TE Image Headers should start at a
  // 4 Byte boundary.
  //
  // Do not attempt to access memory of the first page as it may be protected as
  // part of NULL dereference detection.
  //
  for (; EFI_PAGE_SIZE <= (UINTN) Buffer; Buffer -= 4) {
    //
    // Try to parse the current memory as PE/COFF or TE Image. Pass MAX_UINT32
    // as the file size as there isn't any more information available. Only the
    // Image Header memory will be accessed as part of initialisation.
    //
    Status = PeCoffInitializeContext (
               Context,
               Buffer,
               MAX_UINT32,
               ImageOrigin
               );
    if (RETURN_ERROR (Status)) {
      continue;
    }

    if (!Recurse) {
      //
      // For PE/COFF Images, the PE/COFF Image Header may be discovered while
      // there may still be a preceeding DOS Image Header. All RVAs are relatvie
      // to the start of the Image memory space, of which the DOS Image Header
      // is a part of, if existent. Allow one level of recursion to find a lower
      // Image Base including the DOS Image Header.
      //
      if (Context->ExeHdrOffset == 0) {
        DosStatus = InternalDebugLocateImage (
                      &DosContext,
                      Buffer - 4,
                      Address,
                      TRUE,
                      ImageOrigin
                      );
        if (!RETURN_ERROR (DosStatus)) {
          Buffer = DosContext.ImageBuffer;
          CopyMem (Context, &DosContext, sizeof (*Context));
        }
      }
    }
    //
    // We know that (UINTN) Buffer <= Address from the initialisation.
    //
    // FIXME: Set to non-stripped base for XIP TE Images.
    if (Address < (UINTN) Buffer + PeCoffGetSizeOfImage (Context)) {
      Context->ImageBuffer = Buffer;
      //
      // Zero the raw file information as we are initialising from a potentially
      // non-XIP in-memory Image.
      //
      Context->FileBuffer  = NULL;
      Context->FileSize    = 0;

      return RETURN_SUCCESS;
    }
    //
    // Continue for the unlikely case that a PE/COFF or TE Image embeds another
    // one within its data, the outer Image may still follow.
    //
  }

  return RETURN_NOT_FOUND;
}

RETURN_STATUS
UefiImageDebugLocateImagePe (
  OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  IN  UINTN                            Address,
  IN  UINT8                            ImageOrigin
  )
{
  RETURN_STATUS Status;

  Status = RETURN_NOT_FOUND;
  //
  // As this function is intrinsically unsafe, do not allow its usage outside of
  // DEBUG-enabled code.
  //
  DEBUG_CODE_BEGIN ();

  //
  // If the Image Headers are not loaded explicitly, only XIP Images and Images
  // that embed the Image Header in the first Image section can be located. As
  // this is not the case for the majority of Images, don't attempt to locate
  // the Image base to not access too much (potentially protected) memory.
  //
  if (!PcdGetBool (PcdImageLoaderLoadHeader)) {
    return RETURN_NOT_FOUND;
  }

  Context->FormatIndex = UefiImageFormatPe;
  //
  // Align the search buffer to a 4 Byte boundary.
  //
  Status = InternalDebugLocateImage (
             &Context->Ctx.Pe,
             (CHAR8 *) (Address & ~(UINTN) 3U),
             Address,
             FALSE,
             ImageOrigin
             );

  DEBUG_CODE_END ();

  return Status;
}

RETURN_STATUS
UefiImageGetFixedAddressPe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    UINT64                           *Address
  )
{
  UINT32                         SectionAlignment;
  CONST EFI_IMAGE_SECTION_HEADER *Sections;
  UINT16                         NumberOfSections;
  UINT16                         SectionIndex;
  UINT64                         FixedAddress;

  ASSERT (Address != NULL);

  SectionAlignment = PeCoffGetSectionAlignment (&Context->Ctx.Pe);
  //
  // If this feature is enabled, the build tool will save the address in the
  // PointerToRelocations and PointerToLineNumbers fields of the first Image
  // section header that doesn't hold code. The 64-bit value across those fields
  // will be non-zero if and only if the module has been assigned an address.
  //
  NumberOfSections = PeCoffGetSectionTable (&Context->Ctx.Pe, &Sections);
  for (SectionIndex = 0; SectionIndex < NumberOfSections; ++SectionIndex) {
    if ((Sections[SectionIndex].Characteristics & EFI_IMAGE_SCN_CNT_CODE) != 0) {
      continue;
    }

    FixedAddress = ReadUnaligned64 (
                     (CONST VOID *) &Sections[SectionIndex].PointerToRelocations
                     );
    if (FixedAddress != 0) {
      if (!IS_ALIGNED (FixedAddress, SectionAlignment)) {
        return RETURN_UNSUPPORTED;
      }

      *Address = FixedAddress;
      return RETURN_SUCCESS;
    }

    break;
  }

  return RETURN_NOT_FOUND;
}

VOID
UefiImageDebugPrintSegmentsPe (
  IN OUT UEFI_IMAGE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  CONST EFI_IMAGE_SECTION_HEADER *Sections;
  UINT16                         NumberOfSections;
  UINT16                         SectionIndex;
  CONST UINT8                    *Name;

  NumberOfSections = PeCoffGetSectionTable (&Context->Ctx.Pe, &Sections);

  for (SectionIndex = 0; SectionIndex < NumberOfSections; ++SectionIndex) {
    Name = Sections[SectionIndex].Name;
    DEBUG ((
      DEBUG_VERBOSE,
      "  Section - '%c%c%c%c%c%c%c%c'\n"
      "  VirtualSize          - 0x%08x\n"
      "  VirtualAddress       - 0x%08x\n"
      "  SizeOfRawData        - 0x%08x\n"
      "  PointerToRawData     - 0x%08x\n"
      "  PointerToRelocations - 0x%08x\n"
      "  PointerToLinenumbers - 0x%08x\n"
      "  NumberOfRelocations  - 0x%08x\n"
      "  NumberOfLinenumbers  - 0x%08x\n"
      "  Characteristics      - 0x%08x\n",
      Name[0], Name[1], Name[2], Name[3], Name[4], Name[5], Name[6], Name[7],
      Sections[SectionIndex].VirtualSize,
      Sections[SectionIndex].VirtualAddress,
      Sections[SectionIndex].SizeOfRawData,
      Sections[SectionIndex].PointerToRawData,
      Sections[SectionIndex].PointerToRelocations,
      Sections[SectionIndex].PointerToLinenumbers,
      Sections[SectionIndex].NumberOfRelocations,
      Sections[SectionIndex].NumberOfLinenumbers,
      Sections[SectionIndex].Characteristics
      ));
  }
}

GLOBAL_REMOVE_IF_UNREFERENCED CONST UEFI_IMAGE_FORMAT_SUPPORT mPeSupport = {
  UefiImageInitializeContextPreHashPe,
  UefiImageHashImageDefaultPe,
  UefiImageInitializeContextPostHashPe,
  UefiImageLoadImagePe,
  UefiImageImageIsInplacePe,
  UefiImageLoadImageInplacePe,
  UefiImageRelocateImageInplacePe,
  UefiImageLoaderGetRuntimeContextSizePe,
  UefiImageRelocateImagePe,
  UefiImageRuntimeRelocateImagePe,
  UefiImageDiscardSegmentsPe,
  UefiImageGetSymbolsPathPe,
  UefiImageGetFirstCertificatePe,
  UefiImageGetNextCertificatePe,
  UefiImageGetHiiDataRvaPe,
  UefiImageGetEntryPointAddressPe,
  UefiImageGetMachinePe,
  UefiImageGetSubsystemPe,
  UefiImageGetSegmentAlignmentPe,
  UefiImageGetImageSizePe,
  UefiImageGetBaseAddressPe,
  UefiImageGetRelocsStrippedPe,
  UefiImageLoaderGetImageAddressPe,
  UefiImageLoaderGetDebugAddressPe,
  UefiImageLoaderGetImageRecordPe,
  UefiImageDebugLocateImagePe,
  UefiImageGetFixedAddressPe,
  UefiImageDebugPrintSegmentsPe
};
