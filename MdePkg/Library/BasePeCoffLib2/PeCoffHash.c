/** @file
  Implements APIs to verify the Authenticode Signature of PE/COFF Images.

  Copyright (c) 2020 - 2021, Marvin Häuser. All rights reserved.<BR>
  Copyright (c) 2020, Vitaly Cheptsov. All rights reserved.<BR>
  Copyright (c) 2020, ISP RAS. All rights reserved.<BR>
  Portions copyright (c) 2006 - 2019, Intel Corporation. All rights reserved.<BR>
  Portions copyright (c) 2016, Hewlett Packard Enterprise Development LP. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-3-Clause
**/

#include <Base.h>

#include <IndustryStandard/PeImage2.h>

#include <Guid/WinCertificate.h>

#include <Library/BaseOverflowLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PeCoffLib2.h>

#include "BasePeCoffLib2Internals.h"

/**
  Hashes the Image section data in ascending order of raw file appearance.

  @param[in]     Context      The context describing the Image. Must have been
                              initialised by PeCoffInitializeContext().
  @param[in]     HashUpdate   The data hashing function.
  @param[in,out] HashContext  The context of the current hash.

  @returns  Whether hashing has been successful.
**/
STATIC
BOOLEAN
InternalHashSections (
  IN     CONST PE_COFF_LOADER_IMAGE_CONTEXT  *Context,
  IN OUT VOID                                *HashContext,
  IN     PE_COFF_LOADER_HASH_UPDATE          HashUpdate,
  IN OUT UINT32                              *SumBytesHashed
  )
{
  BOOLEAN                        Result;
  BOOLEAN                        Overflow;

  CONST EFI_IMAGE_SECTION_HEADER *Sections;
  CONST EFI_IMAGE_SECTION_HEADER **SortedSections;
  UINT16                         SectionIndex;
  UINT16                         SectionPos;
  UINT32                         SectionTop;
  UINT32                         CurHashSize;
  //
  // 9. Build a temporary table of pointers to all of the section headers in the
  //   image. The NumberOfSections field of COFF File Header indicates how big
  //   the table should be. Do not include any section headers in the table
  //   whose SizeOfRawData field is zero.
  //
  SortedSections = AllocatePool (
                     (UINT32) Context->NumberOfSections * sizeof (*SortedSections)
                     );
  if (SortedSections == NULL) {
    return FALSE;
  }

  Sections = (CONST EFI_IMAGE_SECTION_HEADER *) (CONST VOID *) (
               (CONST CHAR8 *) Context->FileBuffer + Context->SectionsOffset
               );
  //
  // 10. Using the PointerToRawData field (offset 20) in the referenced
  //     SectionHeader structure as a key, arrange the table's elements in
  //     ascending order. In other words, sort the section headers in ascending
  //     order according to the disk-file offset of the sections.
  //
  SortedSections[0] = Sections;
  //
  // Perform Insertion Sort to order the Sections by their raw file offset.
  //
  for (SectionIndex = 1; SectionIndex < Context->NumberOfSections; ++SectionIndex) {
    for (SectionPos = SectionIndex;
     0 < SectionPos
     && SortedSections[SectionPos - 1]->PointerToRawData > Sections[SectionIndex].PointerToRawData;
     --SectionPos) {
      SortedSections[SectionPos] = SortedSections[SectionPos - 1];
    }

    SortedSections[SectionPos] = Sections + SectionIndex;
  }

  Result      = TRUE;
  SectionTop  = 0;
  CurHashSize = 0;
  //
  // 13. Repeat steps 11 and 12 for all of the sections in the sorted table.
  //
  for (SectionIndex = 0; SectionIndex < Context->NumberOfSections; ++SectionIndex) {
    //
    // Verify the Image section does not overlap with the previous one if the
    // policy demands it. Overlapping Sections could dramatically increase the
    // hashing time.
    // FIXME: Move to init, along with a trailing data policy.
    //
    if (PcdGetBool (PcdImageLoaderHashProhibitOverlap)) {
      if (SectionTop > SortedSections[SectionIndex]->PointerToRawData) {
        Result = FALSE;
        break;
      }

      SectionTop = SortedSections[SectionIndex]->PointerToRawData + SortedSections[SectionIndex]->SizeOfRawData;
    }
    //
    // Skip Sections that contain no data.
    //
    if (SortedSections[SectionIndex]->SizeOfRawData > 0) {
      //
      // 11. Walk through the sorted table, load the corresponding section into
      //     memory, and hash the entire section. Use the SizeOfRawData field in the
      //     SectionHeader structure to determine the amount of data to hash.
      //
      Result = HashUpdate (
                 HashContext,
                 (CONST CHAR8 *) Context->FileBuffer + SortedSections[SectionIndex]->PointerToRawData,
                 SortedSections[SectionIndex]->SizeOfRawData
                 );
      if (!Result) {
        break;
      }
      //
      // 12. Add the section’s SizeOfRawData value to SUM_OF_BYTES_HASHED.
      //
      // If and only if the Sections do not overlap, we know their sizes are at
      // most MAX_UINT32 in sum because the file size is at most MAX_UINT32.
      //
      if (PcdGetBool (PcdImageLoaderHashProhibitOverlap)) {
        CurHashSize += SortedSections[SectionIndex]->SizeOfRawData;
      } else {
        //
        // Verify the hash size does not overflow.
        //
        Overflow = BaseOverflowAddU32 (
                     CurHashSize,
                     SortedSections[SectionIndex]->SizeOfRawData,
                     &CurHashSize
                     );
        if (Overflow) {
          Result = FALSE;
          break;
        }
      }
    }
  }

  *SumBytesHashed = CurHashSize;
  FreePool ((VOID *) SortedSections);

  return Result;
}

BOOLEAN
PeCoffHashImageAuthenticode (
  IN OUT PE_COFF_LOADER_IMAGE_CONTEXT  *Context,
  IN OUT VOID                          *HashContext,
  IN     PE_COFF_LOADER_HASH_UPDATE    HashUpdate
  )
{
  BOOLEAN                      Result;
  BOOLEAN                      Overflow;
  UINT32                       NumberOfRvaAndSizes;
  UINT32                       ChecksumOffset;
  UINT32                       SecurityDirOffset;
  UINT32                       SecurityDirSize;
  UINT32                       CurrentOffset;
  UINT32                       HashSize;
  CONST EFI_IMAGE_NT_HEADERS32 *Pe32;
  CONST EFI_IMAGE_NT_HEADERS64 *Pe32Plus;
  UINT32                       SumBytesHashed;
  UINT32                       FileSize;

  //
  // These conditions must be met by the caller prior to calling this function.
  //
  // 1. Load the image header into memory.
  // 2. Initialize a hash algorithm context.
  //

  //
  // This step can be moved here because steps 1 to 5 do not modify the Image.
  //
  // 6. Get the Attribute Certificate Table address and size from the
  //    Certificate Table entry. For details, see section 5.7 of the PE/COFF
  //    specification.
  //
  // Additionally, retrieve important offsets for later steps.
  //
  switch (Context->ImageType) {
    case PeCoffLoaderTypePe32:
      Pe32 = (CONST EFI_IMAGE_NT_HEADERS32 *) (CONST VOID *) (
               (CONST CHAR8 *) Context->FileBuffer + Context->ExeHdrOffset
               );
      ChecksumOffset      = Context->ExeHdrOffset + (UINT32) OFFSET_OF (EFI_IMAGE_NT_HEADERS32, CheckSum);
      SecurityDirOffset   = Context->ExeHdrOffset + (UINT32) OFFSET_OF (EFI_IMAGE_NT_HEADERS32, DataDirectory) + (UINT32) (EFI_IMAGE_DIRECTORY_ENTRY_SECURITY * sizeof (EFI_IMAGE_DATA_DIRECTORY));
      NumberOfRvaAndSizes = Pe32->NumberOfRvaAndSizes;
      //
      // Retrieve the Security Directory size depending on existence.
      //
      if (EFI_IMAGE_DIRECTORY_ENTRY_SECURITY < NumberOfRvaAndSizes) {
        SecurityDirSize = Pe32->DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY].Size;
      } else {
        SecurityDirSize = 0;
      }

      break;

    case PeCoffLoaderTypePe32Plus:
      Pe32Plus = (CONST EFI_IMAGE_NT_HEADERS64 *) (CONST VOID *) (
                   (CONST CHAR8 *) Context->FileBuffer + Context->ExeHdrOffset
                   );
      ChecksumOffset      = Context->ExeHdrOffset + (UINT32) OFFSET_OF (EFI_IMAGE_NT_HEADERS64, CheckSum);
      SecurityDirOffset   = Context->ExeHdrOffset + (UINT32) OFFSET_OF (EFI_IMAGE_NT_HEADERS64, DataDirectory) + (UINT32) (EFI_IMAGE_DIRECTORY_ENTRY_SECURITY * sizeof (EFI_IMAGE_DATA_DIRECTORY));
      NumberOfRvaAndSizes = Pe32Plus->NumberOfRvaAndSizes;
      //
      // Retrieve the Security Directory size depending on existence.
      //
      if (EFI_IMAGE_DIRECTORY_ENTRY_SECURITY < NumberOfRvaAndSizes) {
        SecurityDirSize = Pe32Plus->DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY].Size;
      } else {
        SecurityDirSize = 0;
      }

      break;

    default:
      ASSERT (FALSE);
      return FALSE;
  }
  //
  // 3. Hash the image header from its base to immediately before the start of
  //    the checksum address, as specified in Optional Header Windows-Specific
  //    Fields.
  //
  Result = HashUpdate (HashContext, Context->FileBuffer, ChecksumOffset);
  if (!Result) {
    DEBUG_RAISE ();
    return FALSE;
  }
  //
  // 4. Skip over the checksum, which is a 4-byte field.
  //
  CurrentOffset = ChecksumOffset + sizeof (UINT32);
  //
  // 5. Hash everything from the end of the checksum field to immediately before
  //    the start of the Certificate Table entry, as specified in Optional
  //    Header Data Directories.
  //
  if (EFI_IMAGE_DIRECTORY_ENTRY_SECURITY < NumberOfRvaAndSizes) {
    HashSize = SecurityDirOffset - CurrentOffset;
    Result = HashUpdate (
               HashContext,
               (CONST CHAR8 *) Context->FileBuffer + CurrentOffset,
               HashSize
               );
    if (!Result) {
      DEBUG_RAISE ();
      return FALSE;
    }
    //
    // Skip over the Security Directory.
    //
    CurrentOffset = SecurityDirOffset + sizeof (EFI_IMAGE_DATA_DIRECTORY);
  }
  //
  // 7. Exclude the Certificate Table entry from the calculation and hash
  //    everything from the end of the Certificate Table entry to the end of
  //    image header, including Section Table (headers).The Certificate Table
  //    entry is 8 Bytes long, as specified in Optional Header Data Directories.
  //
  HashSize = Context->SizeOfHeaders - CurrentOffset;
  Result = HashUpdate (
             HashContext,
             (CONST CHAR8 *) Context->FileBuffer + CurrentOffset,
             HashSize
             );
  if (!Result) {
    DEBUG_RAISE ();
    return FALSE;
  }
  //
  // Perform the Section-related steps of the algorithm.
  //
  Result = InternalHashSections (
             Context,
             HashContext,
             HashUpdate,
             &SumBytesHashed
             );
  if (!Result) {
    DEBUG_RAISE ();
    return FALSE;
  }
  //
  // 8. Create a counter called SUM_OF_BYTES_HASHED, which is not part of the
  //    signature. Set this counter to the SizeOfHeaders field, as specified in
  //    Optional Header Windows-Specific Field.
  //
  Overflow = BaseOverflowAddU32 (
               SumBytesHashed,
               Context->SizeOfHeaders,
               &SumBytesHashed
               );
  if (Overflow) {
    DEBUG_RAISE ();
    return FALSE;
  }
  //
  // 14. Create a value called FILE_SIZE, which is not part of the signature.
  //     Set this value to the image’s file size, acquired from the underlying
  //     file system. If FILE_SIZE is greater than SUM_OF_BYTES_HASHED, the file
  //     contains extra data that must be added to the hash. This data begins at
  //     the SUM_OF_BYTES_HASHED file offset, and its length is:
  //     (File Size) - ((Size of AttributeCertificateTable) + SUM_OF_BYTES_HASHED)
  //
  //     Note: The size of Attribute Certificate Table is specified in the
  //     second ULONG value in the Certificate Table entry (32 bit: offset 132,
  //     64 bit: offset 148) in Optional Header Data Directories.
  //
  FileSize = Context->FileSize - SecurityDirSize;
  if (SumBytesHashed < FileSize) {
    Result = HashUpdate (
               HashContext,
               (CONST CHAR8 *) Context->FileBuffer + SumBytesHashed,
               FileSize - SumBytesHashed
               );
  }
  //
  // This step must be performed by the caller after this function succeeded.
  //
  // 15. Finalize the hash algorithm context.
  //
  return Result;
}

RETURN_STATUS
PeCoffGetFirstCertificate (
  IN OUT PE_COFF_LOADER_IMAGE_CONTEXT  *Context,
  OUT    CONST WIN_CERTIFICATE         **Certificate
  )
{
  CONST WIN_CERTIFICATE *WinCertificate;
  //
  // These conditions are verified by PeCoffInitializeContext().
  //
  ASSERT (Context->SecDirOffset % ALIGNOF (WIN_CERTIFICATE));
  ASSERT (Context->SecDirSize == 0 || sizeof (WIN_CERTIFICATE) <= Context->SecDirSize);
  //
  // Verify the Security Directory is not empty.
  //
  if (Context->SecDirSize == 0) {
    return RETURN_NOT_FOUND;
  }
  //
  // Verify the certificate size is well-formed and that it is in bounds of the
  // Security Directory.
  //
  WinCertificate = (CONST WIN_CERTIFICATE *) (CONST VOID *) (
                     (CONST UINT8 *) Context->FileBuffer + Context->SecDirOffset
                     );
  if (WinCertificate->dwLength < sizeof (WIN_CERTIFICATE)
   || WinCertificate->dwLength > Context->SecDirSize) {
    DEBUG_RAISE ();
    return RETURN_VOLUME_CORRUPTED;
  }
  //
  // Verify the certificate size is sufficiently aligned, if the policy demands
  // it. This has been observed to not be the case with images signed with
  // pesign, such as GRUB.
  //
  if ((PcdGet32 (PcdImageLoaderAlignmentPolicy) & PCD_ALIGNMENT_POLICY_CERTIFICATE_SIZES) == 0) {
    if (!IS_ALIGNED (WinCertificate->dwLength, IMAGE_CERTIFICATE_ALIGN)) {
      DEBUG_RAISE ();
      return RETURN_VOLUME_CORRUPTED;
    }
  }

  *Certificate = WinCertificate;

  return RETURN_SUCCESS;
}

RETURN_STATUS
PeCoffGetNextCertificate (
  IN OUT PE_COFF_LOADER_IMAGE_CONTEXT  *Context,
  IN OUT CONST WIN_CERTIFICATE         **Certificate
  )
{
  BOOLEAN               Overflow;
  UINT32                CertOffset;
  UINT32                CertSize;
  UINT32                CertEnd;
  CONST WIN_CERTIFICATE *WinCertificate;
  //
  // This condition is verified by PeCoffInitializeContext().
  //
  ASSERT (IS_ALIGNED (Context->SecDirSize, IMAGE_CERTIFICATE_ALIGN));
  //
  // Retrieve the current certificate.
  //
  WinCertificate = *Certificate;
  CertOffset     = (UINT32) ((UINTN) WinCertificate - ((UINTN) Context->FileBuffer + Context->SecDirOffset));
  //
  // Retrieve the offset of the next certificate.
  //
  if ((PcdGet32 (PcdImageLoaderAlignmentPolicy) & PCD_ALIGNMENT_POLICY_CERTIFICATE_SIZES) == 0) {
    CertSize = WinCertificate->dwLength;
  } else {
    CertSize = ALIGN_VALUE (WinCertificate->dwLength, IMAGE_CERTIFICATE_ALIGN);
  }

  CertOffset += CertSize;
  //
  // This invariant is ensured by the certificate iteration functions.
  //
  ASSERT (CertOffset <= Context->SecDirSize);
  //
  // If the next offset is the end of the Directory, signal it's the end of
  // the certificate list.
  //
  if (CertOffset == Context->SecDirSize) {
    return RETURN_NOT_FOUND;
  }
  //
  // Verify the Directory fits another certificate.
  //
  if (Context->SecDirSize - CertOffset < sizeof (WIN_CERTIFICATE)) {
    DEBUG_RAISE ();
    return RETURN_VOLUME_CORRUPTED;
  }
  //
  // Verify the certificate has a well-formed size.
  //
  WinCertificate = (CONST WIN_CERTIFICATE *) (CONST VOID *) (
                     (CONST UINT8 *) Context->FileBuffer + Context->SecDirOffset + CertOffset
                     );
  if (WinCertificate->dwLength < sizeof (WIN_CERTIFICATE)) {
    DEBUG_RAISE ();
    return RETURN_VOLUME_CORRUPTED;
  }
  //
  // Verify the certificate size is sufficiently aligned, if the policy demands
  // it.
  //
  if ((PcdGet32 (PcdImageLoaderAlignmentPolicy) & PCD_ALIGNMENT_POLICY_CERTIFICATE_SIZES) == 0) {
    if (!IS_ALIGNED (WinCertificate->dwLength, IMAGE_CERTIFICATE_ALIGN)) {
      DEBUG_RAISE ();
      return RETURN_VOLUME_CORRUPTED;
    }
  }
  //
  // Verify the certificate is in bounds of the Security Directory.
  //
  Overflow = BaseOverflowAddU32 (
               CertOffset,
               WinCertificate->dwLength,
               &CertEnd
               );
  if (Overflow || CertEnd > Context->SecDirSize) {
    DEBUG_RAISE ();
    return RETURN_VOLUME_CORRUPTED;
  }

  *Certificate = WinCertificate;

  return RETURN_SUCCESS;
}
