/** @file
  UEFI image loader library implementation for UE images.

  Copyright (c) 2021 - 2023, Marvin Häuser. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Base.h>

#include <IndustryStandard/UeImage.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseOverflowLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PeCoffLib2.h>
#include <Library/UefiImageLib.h>
#include <Library/UeImageLib.h>

struct UE_LOADER_RUNTIME_CONTEXT_ {
  UINT8   Machine;
  UINT8   Reserved[7];
  UINT32  FixupSize;
  UINT64  *FixupData;
  UINT32  UnchainedRelocsSize;
  UINT8   *UnchainedRelocs;
};

typedef union {
  UINT32  Value32;
  UINT64  Value64;
} UE_RELOC_FIXUP_VALUE;

STATIC
RETURN_STATUS
InternalVerifySegments (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  CONST UE_SEGMENT  *Segments;
  UINT8             LastSegmentIndex;
  UINT8             SegmentIndex;
  UINT32            SegmentEndFileOffset;
  UINT32            SegmentEndImageAddress;
  BOOLEAN           Overflow;
  UINT32            SegmentImageSize;

  Segments         = Context->Segments;
  LastSegmentIndex = Context->LastSegmentIndex;
  //
  // As it holds that SegmentEndFileOffset - Context->SegmentsFileOffset <= SegmentEndImageAddress,
  // and it holds that SegmentEndImageAddress % 4 KiB = 0, SegmentEndFileOffset - Context->SegmentsFileOffset
  // can be at most 0xFFFFF000. As it holds that Context->SegmentsFileOffset <= MAX_SIZE_OF_UE_HEADER <= 0xFF0,
  // this cannot overflow.
  //
  STATIC_ASSERT (
    MAX_SIZE_OF_UE_HEADER <= BASE_4KB - 8,
    "The arithmetic below may overflow."
    );

  SegmentEndFileOffset = Context->SegmentsFileOffset;
  //
  // The first image segment must begin the image address space.
  //
  SegmentEndImageAddress = 0;

  for (SegmentIndex = 0; SegmentIndex <= LastSegmentIndex; ++SegmentIndex) {
    SegmentImageSize = UE_SEGMENT_SIZE (Segments[SegmentIndex].ImageInfo);

    if (Segments[SegmentIndex].FileSize > SegmentImageSize) {
      DEBUG_RAISE ();
      return RETURN_UNSUPPORTED;
    }
    //
    // Verify the image segments are aligned.
    //
    if (!IS_ALIGNED (SegmentImageSize, Context->SegmentAlignment)) {
      DEBUG_RAISE ();
      return RETURN_UNSUPPORTED;
    }
    //
    // Determine the end of the current image segment.
    //
    Overflow = BaseOverflowAddU32 (
                 SegmentEndImageAddress,
                 SegmentImageSize,
                 &SegmentEndImageAddress
                 );
    if (Overflow) {
      DEBUG_RAISE ();
      return RETURN_UNSUPPORTED;
    }
    //
    // As it holds that SegmentFileSize <= SegmentImageSize and thus
    // SegmentEndFileOffset - Context->SegmentsFileOffset <= SegmentEndImageAddress,
    // this cannot overflow (see above).
    //
    SegmentEndFileOffset += Segments[SegmentIndex].FileSize;
  }
  //
  // As it holds that SegmentEndFileOffset <= 0xFFFFFFF0, this cannot overflow.
  //
  SegmentEndFileOffset = ALIGN_VALUE (
                           SegmentEndFileOffset,
                           UE_LOAD_TABLE_ALIGNMENT
                           );
  //
  // Verify all image segment data are in bounds of the file buffer.
  //
  ASSERT (Context->SegmentsFileOffset <= SegmentEndFileOffset);
  if (SegmentEndFileOffset > Context->UnsignedFileSize) {
    DEBUG_RAISE ();
    return RETURN_UNSUPPORTED;
  }

  Context->LoadTablesFileOffset = SegmentEndFileOffset;
  Context->ImageSize            = SegmentEndImageAddress;

  if (Context->XIP) {
    Context->ImageSize += Context->SegmentsFileOffset;
  }

  return RETURN_SUCCESS;
}

STATIC
RETURN_STATUS
InternalVerifyLoadTables (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  BOOLEAN              Overflow;
  CONST UE_LOAD_TABLE  *LoadTable;
  UINT8                LoadTableIndex;
  INT16                PrevLoadTableId;
  UINT32               LoadTableFileOffset;
  UINT32               LoadTableFileSize;
  UINT8                LoadTableId;
  UINT32               LoadTableEndFileOffset;
  UINT8                NumLoadTables;

  Context->RelocTableSize = 0;

  LoadTableEndFileOffset = Context->LoadTablesFileOffset;
  NumLoadTables          = Context->NumLoadTables;
  PrevLoadTableId    = -1;

  if (0 < NumLoadTables) {
    LoadTableIndex = 0;
    do {
      LoadTable   = Context->LoadTables + LoadTableIndex;
      LoadTableId = UE_LOAD_TABLE_ID (LoadTable->FileInfo);

      if (PrevLoadTableId >= LoadTableId) {
        DEBUG_RAISE ();
        return RETURN_UNSUPPORTED;
      }

      LoadTableFileOffset = LoadTableEndFileOffset;
      LoadTableFileSize   = UE_LOAD_TABLE_SIZE (LoadTable->FileInfo);

      Overflow = BaseOverflowAddU32 (
                   LoadTableFileOffset,
                   LoadTableFileSize,
                   &LoadTableEndFileOffset
                   );
      if (Overflow) {
        DEBUG_RAISE ();
        return RETURN_UNSUPPORTED;
      }

      PrevLoadTableId = LoadTableId;
      ++LoadTableIndex;
    } while (LoadTableIndex < NumLoadTables);

    if (UE_LOAD_TABLE_ID (Context->LoadTables[0].FileInfo) == UeLoadTableIdReloc) {
      if (Context->RelocsStripped) {
        DEBUG_RAISE ();
        return RETURN_UNSUPPORTED;
      }

      Context->RelocTableSize = UE_LOAD_TABLE_SIZE (
                                  Context->LoadTables[0].FileInfo
                                  );
    }
  }

  if (LoadTableEndFileOffset != Context->UnsignedFileSize) {
    DEBUG_RAISE ();
    return RETURN_UNSUPPORTED;
  }

  return RETURN_SUCCESS;
}

RETURN_STATUS
UeInitializeContextPreHash (
  OUT UE_LOADER_IMAGE_CONTEXT  *Context,
  IN  CONST VOID               *FileBuffer,
  IN  UINT32                   FileSize
  )
{
  //BOOLEAN         Overflow;
  CONST UE_HEADER *UeHdr;
  //UINT32          UnsignedFileSize;

  ASSERT (Context != NULL);
  ASSERT (FileBuffer != NULL || FileSize == 0);

  if (MIN_SIZE_OF_UE_HEADER > FileSize) {
    return RETURN_UNSUPPORTED;
  }

  UeHdr = (CONST UE_HEADER *) FileBuffer;

  if (UeHdr->Magic != UE_HEADER_MAGIC) {
    return RETURN_UNSUPPORTED;
  }

  ZeroMem (Context, sizeof (*Context));

  /*UnsignedFileSize = UE_HEADER_FILE_SIZE (UeHdr->FileInfo);

  Overflow = BaseOverflowSubU32 (
               FileSize,
               UnsignedFileSize,
               &Context->CertTableSize
               );
  if (Overflow) {
    DEBUG_RAISE ();
    return RETURN_UNSUPPORTED;
  }*/

  Context->FileBuffer       = (CONST UINT8 *)UeHdr;
  //Context->UnsignedFileSize = UnsignedFileSize;
  Context->UnsignedFileSize = FileSize;

  return RETURN_SUCCESS;
}

BOOLEAN
UeHashImageDefault (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context,
  IN OUT VOID                     *HashContext,
  IN     UE_LOADER_HASH_UPDATE    HashUpdate
  )
{
  BOOLEAN Result;

  ASSERT (Context != NULL);
  ASSERT (HashContext != NULL);
  ASSERT (HashUpdate != NULL);

  Result = HashUpdate (
             HashContext,
             Context->FileBuffer,
             Context->UnsignedFileSize
             );
  if (!Result) {
    DEBUG_RAISE ();
  }

  return Result;
}

STATIC
RETURN_STATUS
InternalInitializeContextLate (
  OUT UE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  if (Context->EntryPointAddress > Context->ImageSize) {
    DEBUG_RAISE ();
    return RETURN_UNSUPPORTED;
  }

  return InternalVerifyLoadTables (Context);
}

RETURN_STATUS
UeInitializeContextPostHash (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  CONST UE_HEADER *UeHdr;
  UINT8           LastSegmentIndex;
  UINT8           NumLoadTables;
  UINT32          LoadTablesFileOffset;
  UINT32          HeaderSize;
  UINT64          BaseAddress;
  RETURN_STATUS   Status;

  ASSERT (Context != NULL);

  UeHdr = (CONST UE_HEADER *)Context->FileBuffer;

  Context->FixedAddress   = (UeHdr->ImageInfo & UE_HEADER_IMAGE_INFO_FIXED_ADDRESS) != 0;
  Context->RelocsStripped = (UeHdr->ImageInfo & UE_HEADER_IMAGE_INFO_RELOCATION_FIXUPS_STRIPPED) != 0;
  Context->XIP            = (UeHdr->ImageInfo & UE_HEADER_IMAGE_INFO_XIP) != 0;

  Context->Segments                 = UeHdr->Segments;
  Context->SegmentImageInfoIterSize = sizeof (*UeHdr->Segments);
  Context->SegmentAlignment         = UE_HEADER_SEGMENT_ALIGNMENT (UeHdr->ImageInfo);

  LastSegmentIndex = UE_HEADER_LAST_SEGMENT_INDEX (UeHdr->TableCounts);
  NumLoadTables    = UE_HEADER_NUM_LOAD_TABLES (UeHdr->TableCounts);

  LoadTablesFileOffset = MIN_SIZE_OF_UE_HEADER
                           + (UINT32) LastSegmentIndex * sizeof (UE_SEGMENT);

  HeaderSize = LoadTablesFileOffset + (UINT32) NumLoadTables * sizeof (UE_LOAD_TABLE);
  ASSERT (HeaderSize <= MAX_SIZE_OF_UE_HEADER);
  if (Context->XIP) {
    HeaderSize = ALIGN_VALUE (HeaderSize, Context->SegmentAlignment);
  }

  if (HeaderSize > Context->UnsignedFileSize) {
    DEBUG_RAISE ();
    return RETURN_UNSUPPORTED;
  }

  ASSERT (IS_ALIGNED (LoadTablesFileOffset, ALIGNOF (UE_LOAD_TABLE)));

  Context->LoadTables = (CONST UE_LOAD_TABLE *)(
                          (CONST UINT8 *) UeHdr + LoadTablesFileOffset
                          );

  Context->SegmentsFileOffset = HeaderSize;
  Context->LastSegmentIndex   = LastSegmentIndex;
  Context->NumLoadTables      = NumLoadTables;

  BaseAddress = UE_HEADER_BASE_ADDRESS (UeHdr->ImageInfo);

  if (!IS_ALIGNED (BaseAddress, Context->SegmentAlignment)) {
    DEBUG_RAISE ();
    return RETURN_UNSUPPORTED;
  }

  Context->FileBuffer = (CONST UINT8 *)UeHdr;

  Status = InternalVerifySegments (Context);
  if (RETURN_ERROR (Status)) {
    return Status;
  }

  Context->BaseAddress       = BaseAddress;
  Context->EntryPointAddress = UeHdr->EntryPointAddress;

  Context->Subsystem = UE_HEADER_SUBSYSTEM (UeHdr->Type);
  Context->Machine   = UE_HEADER_ARCH (UeHdr->Type);

  return InternalInitializeContextLate (Context);
}

RETURN_STATUS
UeLoadImage (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    VOID                     *Destination,
  IN     UINT32                   DestinationSize
  )
{
  UINT8  SegmentIndex;
  UINT32 SegmentFileOffset;
  UINT32 SegmentFileSize;
  UINT32 SegmentImageAddress;
  UINT32 SegmentImageSize;
  UINT32 PrevSegmentDataEnd;

  CONST UE_SEGMENT *Segments;

  ASSERT (Context != NULL);
  ASSERT (Destination != NULL);
  ASSERT (ADDRESS_IS_ALIGNED (Destination, Context->SegmentAlignment));

  Context->ImageBuffer = Destination;

  //
  // Load all image load tables into the address space.
  //
  Segments = Context->Segments;

  //
  // Start zeroing from the start of the destination buffer.
  //
  PrevSegmentDataEnd = 0;

  SegmentFileOffset   = Context->SegmentsFileOffset;
  SegmentImageAddress = 0;

  SegmentIndex = 0;
  do {
    SegmentFileSize   = Segments[SegmentIndex].FileSize;
    SegmentImageSize = UE_SEGMENT_SIZE (Segments[SegmentIndex].ImageInfo);
    //
    // Zero from the end of the previous image segment to the start of this
    // image segment.
    //
    ZeroMem (
      (UINT8 *)Destination + PrevSegmentDataEnd,
      SegmentImageAddress - PrevSegmentDataEnd
      );
    //
    // Load the current Image segment into the address space.
    //
    ASSERT (SegmentFileSize <= SegmentImageSize);
    CopyMem (
      (UINT8 *)Destination + SegmentImageAddress,
      Context->FileBuffer + SegmentFileOffset,
      SegmentFileSize
      );
    PrevSegmentDataEnd = SegmentImageAddress + SegmentFileSize;

    SegmentFileOffset   += SegmentFileSize;
    SegmentImageAddress += SegmentImageSize;
    ++SegmentIndex;
  } while (SegmentIndex <= Context->LastSegmentIndex);
  //
  // Zero the trailing data after the last image segment.
  //
  ZeroMem (
    (UINT8 *)Destination + PrevSegmentDataEnd,
    DestinationSize - PrevSegmentDataEnd
    );

  return RETURN_SUCCESS;
}

RETURN_STATUS
UeLoaderGetRuntimeContextSize (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    UINT32                   *Size
  )
{
  ASSERT (Context != NULL);
  ASSERT (Size != NULL);

  *Size = sizeof (UE_LOADER_RUNTIME_CONTEXT);
  return RETURN_SUCCESS;
}

/**
  Apply an image relocation fixup.

  Only a subset of the PE/COFF relocation fixup types are permited.
  The relocation fixup target must be in bounds, aligned, and must not overlap
  with the Relocation Directory.

  @param[in] Context     The context describing the image. Must have been
                         loaded by PeCoffLoadImage().
  @param[in] RelocIndex  The index of the relocation fixup to apply.
  @param[in] Adjust      The delta to add to the addresses.

  @retval RETURN_SUCCESS  The relocation fixup has been applied successfully.
  @retval other           The relocation fixup could not be applied successfully.
**/
STATIC
RETURN_STATUS
InternalApplyRelocation (
  IN OUT VOID    *Image,
  IN     UINT32  ImageSize,
  IN     UINT8   Machine,
  IN     UINT16  RelocType,
  IN     UINT32  *RelocTarget,
  IN     UINT64  Adjust,
  IN OUT UINT64  *FixupData,
  IN     BOOLEAN IsRuntime
  )
{
  BOOLEAN               Overflow;

  UINT32                RemFixupTargetSize;

  UINT32                FixupTarget;
  VOID                  *Fixup;
  UINT8                 FixupSize;
  UE_RELOC_FIXUP_VALUE  FixupValue;

  ASSERT (FixupData != NULL);

  FixupTarget = *RelocTarget;
  //
  // Verify the relocation fixup target address is in bounds of the image buffer.
  //
  Overflow = BaseOverflowSubU32 (ImageSize, FixupTarget, &RemFixupTargetSize);
  if (Overflow) {
    DEBUG_RAISE ();
    return RETURN_VOLUME_CORRUPTED;
  }

  Fixup = (UINT8 *)Image + FixupTarget;
  //
  // Apply the relocation fixup per type.
  //
  if (RelocType < UeRelocGenericMax) {
    if (RelocType == UeReloc32) {
      FixupSize = sizeof (UINT32);
      //
      // Verify the relocation fixup target is in bounds of the image buffer.
      //
      if (FixupSize > RemFixupTargetSize) {
        DEBUG_RAISE ();
        return RETURN_VOLUME_CORRUPTED;
      }
      //
      // Relocate the target instruction.
      //
      FixupValue.Value32  = ReadUnaligned32 (Fixup);
      //
      // If the Image relocation target value mismatches, skip or abort.
      //
      if (IsRuntime && (FixupValue.Value32 != (UINT32)*FixupData)) {
        if (PcdGetBool (PcdImageLoaderRtRelocAllowTargetMismatch)) {
          return RETURN_SUCCESS;
        }

        return RETURN_VOLUME_CORRUPTED;
      }

      FixupValue.Value32 += (UINT32) Adjust;
      WriteUnaligned32 (Fixup, FixupValue.Value32);
    } else if (RelocType == UeReloc64) {
      FixupSize = sizeof (UINT64);
      //
      // Verify the image relocation fixup target is in bounds of the image
      // buffer.
      //
      if (FixupSize > RemFixupTargetSize) {
        DEBUG_RAISE ();
        return RETURN_VOLUME_CORRUPTED;
      }
      //
      // Relocate target the instruction.
      //
      FixupValue.Value64  = ReadUnaligned64 (Fixup);
      //
      // If the Image relocation target value mismatches, skip or abort.
      //
      if (IsRuntime && (FixupValue.Value64 != *FixupData)) {
        if (PcdGetBool (PcdImageLoaderRtRelocAllowTargetMismatch)) {
          return RETURN_SUCCESS;
        }

        return RETURN_VOLUME_CORRUPTED;
      }

      FixupValue.Value64 += Adjust;
      WriteUnaligned64 (Fixup, FixupValue.Value64);
    } else if (RelocType == UeReloc32NoMeta) {
      FixupSize = sizeof (UINT32);
      //
      // Verify the relocation fixup target is in bounds of the image buffer.
      //
      if (FixupSize > RemFixupTargetSize) {
        DEBUG_RAISE ();
        return RETURN_VOLUME_CORRUPTED;
      }
      //
      // Relocate the target instruction.
      //
      FixupValue.Value32  = ReadUnaligned32 (Fixup);
      //
      // If the Image relocation target value mismatches, skip or abort.
      //
      if (IsRuntime && (FixupValue.Value32 != (UINT32)*FixupData)) {
        if (PcdGetBool (PcdImageLoaderRtRelocAllowTargetMismatch)) {
          return RETURN_SUCCESS;
        }

        return RETURN_VOLUME_CORRUPTED;
      }

      FixupValue.Value32 += (UINT32) Adjust;
      WriteUnaligned32 (Fixup, FixupValue.Value32);

      if (!IsRuntime) {
        *FixupData = FixupValue.Value32;
      }
    } else {
      //
      // The image relocation fixup type is unknown, disallow the image.
      //
      DEBUG_RAISE ();
      return RETURN_UNSUPPORTED;
    }
  } else {
#if 0
    if (Machine == UeMachineArmThumbMixed) {
      switch (RelocType) {
        // TODO: MOVW
        case (UeMachineArmThumbMixed << 4U) | UeRelocArmMovt:
        {
          //
          // Verify the relocation fixup target is in bounds of the image buffer.
          //
          if (sizeof (UINT64) > RemFixupTargetSize) {
            DEBUG_RAISE ();
            return RETURN_VOLUME_CORRUPTED;
          }
          //
          // Verify the relocation fixup target is sufficiently aligned.
          // The ARM Thumb instruction pait must start on a 16-bit boundary.
          //
          if (!IS_ALIGNED (RelocTarget, ALIGNOF (UINT16))) {
            DEBUG_RAISE ();
            return RETURN_VOLUME_CORRUPTED;
          }
          //
          // Relocate the target instruction.
          //
          PeCoffThumbMovwMovtImmediateFixup (Fixup, Adjust);

          break;
        }

        default:
        {
          //
          // The image relocation fixup type is unknown, disallow the image.
          //
          DEBUG_RAISE ();
          return RETURN_UNSUPPORTED;
        }
      }
    }
#endif
    //
    // The image relocation fixup type is unknown, disallow the image.
    //
    DEBUG_RAISE ();
    return RETURN_UNSUPPORTED;
  }

  *RelocTarget = FixupTarget + FixupSize;

  return RETURN_SUCCESS;
}

STATIC
RETURN_STATUS
UnchainReloc (
  IN OUT UE_LOADER_RUNTIME_CONTEXT  *RuntimeContext OPTIONAL,
  IN     CONST UINT8                *MetaSource OPTIONAL,
  IN     UINT32                     MetaSize,
  IN     BOOLEAN                    IsRuntime,
  IN     UINT16                     RelocOffset,
  IN     UINT64                     *FixupData
  )
{
  UINT32  OldSize;
  UINT16  FixupHdr;
  UINT32  FixupIndex;
  UINT8   Addend;

  if ((RuntimeContext != NULL) && !IsRuntime) {
    if (MetaSource == NULL) {
      FixupIndex = RuntimeContext->UnchainedRelocsSize - sizeof (UINT16);

      FixupHdr   = *(UINT16 *)&RuntimeContext->UnchainedRelocs[FixupIndex];
      FixupHdr   = (RelocOffset << 4U) | UE_RELOC_FIXUP_TYPE(FixupHdr);

      *(UINT16 *)&RuntimeContext->UnchainedRelocs[FixupIndex] = FixupHdr;

      Addend = ALIGN_VALUE_ADDEND(RuntimeContext->UnchainedRelocsSize, ALIGNOF(UE_FIXUP_ROOT));
      if ((RelocOffset == UE_HEAD_FIXUP_OFFSET_END) && (Addend != 0)) {
        OldSize = RuntimeContext->UnchainedRelocsSize;
        RuntimeContext->UnchainedRelocs = ReallocateRuntimePool (
                                            OldSize,
                                            OldSize + Addend,
                                            RuntimeContext->UnchainedRelocs
                                            );
        if (RuntimeContext->UnchainedRelocs == NULL) {
          return RETURN_OUT_OF_RESOURCES;
        }

        ZeroMem (RuntimeContext->UnchainedRelocs + OldSize, Addend);
        RuntimeContext->UnchainedRelocsSize += Addend;
      }

      return RETURN_SUCCESS;
    }

    OldSize = RuntimeContext->UnchainedRelocsSize;
    RuntimeContext->UnchainedRelocs = ReallocateRuntimePool (
                                        OldSize,
                                        OldSize + MetaSize,
                                        RuntimeContext->UnchainedRelocs
                                        );
    if (RuntimeContext->UnchainedRelocs == NULL) {
      return RETURN_OUT_OF_RESOURCES;
    }

    CopyMem (RuntimeContext->UnchainedRelocs + OldSize, MetaSource, MetaSize);

    RuntimeContext->UnchainedRelocsSize += MetaSize;

    if (FixupData != NULL) {
      OldSize = RuntimeContext->FixupSize;
      RuntimeContext->FixupData = ReallocateRuntimePool (
                                    OldSize,
                                    OldSize + sizeof (*FixupData),
                                    RuntimeContext->FixupData
                                    );
      if (RuntimeContext->FixupData == NULL) {
        return RETURN_OUT_OF_RESOURCES;
      }

      CopyMem ((UINT8 *)RuntimeContext->FixupData + OldSize, FixupData, sizeof (*FixupData));

      RuntimeContext->FixupSize += sizeof (*FixupData);
    }
  }

  return RETURN_SUCCESS;
}

STATIC
RETURN_STATUS
InternalProcessRelocChain (
  IN OUT VOID                       *Image,
  IN     UINT32                     ImageSize,
  IN     UINT8                      Machine,
  IN     UINT16                     FirstRelocType,
  IN     UINT32                     *ChainStart,
  IN     UINT64                     Adjust,
  IN OUT UE_LOADER_RUNTIME_CONTEXT  *RuntimeContext OPTIONAL
  )
{
  RETURN_STATUS         Status;
  UINT16                RelocType;
  UINT16                RelocOffset;
  UINT32                RelocTarget;

  BOOLEAN               Overflow;
  UINT32                RemFixupTargetSize;

  UINT8                 *Fixup;
  UE_RELOC_FIXUP_VALUE  FixupInfo;
  UINT8                 FixupSize;
  UE_RELOC_FIXUP_VALUE  FixupValue;
  UINT16                FixupHdr;
  UINT64                FixupData;

  RelocType   = FirstRelocType;
  RelocTarget = *ChainStart;

  while (TRUE) {
    Overflow = BaseOverflowSubU32 (ImageSize, RelocTarget, &RemFixupTargetSize);
    if (Overflow) {
      DEBUG_RAISE ();
      return RETURN_VOLUME_CORRUPTED;
    }

    Fixup = (UINT8 *)Image + RelocTarget;

    if (RelocType < UeRelocGenericMax) {
      if (RelocType == UeReloc64) {
        FixupSize = sizeof (UINT64);
        //
        // Verify the relocation fixup target is in bounds of the image memory.
        //
        if (FixupSize > RemFixupTargetSize) {
          DEBUG_RAISE ();
          return RETURN_VOLUME_CORRUPTED;
        }
        //
        // Relocate the target instruction.
        //
        FixupInfo.Value64   = ReadUnaligned64 ((CONST VOID *)Fixup);
        FixupValue.Value64  = UE_CHAINED_RELOC_FIXUP_VALUE (FixupInfo.Value64);
        FixupValue.Value64 += Adjust;
        WriteUnaligned64 ((VOID *)Fixup, FixupValue.Value64);

        FixupData = FixupValue.Value64;
      } else if (RelocType == UeReloc32) {
        FixupSize = sizeof (UINT32);
        //
        // Verify the image relocation fixup target is in bounds of the image
        // buffer.
        //
        if (FixupSize > RemFixupTargetSize) {
          DEBUG_RAISE ();
          return RETURN_VOLUME_CORRUPTED;
        }
        //
        // Relocate the target instruction.
        //
        FixupInfo.Value32   = ReadUnaligned32 ((CONST VOID *)Fixup);
        FixupValue.Value32  = UE_CHAINED_RELOC_FIXUP_VALUE_32 (FixupInfo.Value32);
        FixupValue.Value32 += (UINT32) Adjust;
        WriteUnaligned32 ((VOID *)Fixup, FixupValue.Value32);

        FixupData = FixupValue.Value32;
        //
        // Imitate the common header of UE chained relocation fixups,
        // as for 32-bit files all relocs have the same type.
        //
        FixupInfo.Value32 = FixupInfo.Value32 << 4U;
        FixupInfo.Value32 |= UeReloc32;
      } else {
        //
        // The image relocation fixup type is unknown, disallow the image.
        //
        DEBUG_RAISE ();
        return RETURN_UNSUPPORTED;
      }
    } else {
      //
      // The image relocation fixup type is unknown, disallow the image.
      //
      DEBUG_RAISE ();
      return RETURN_UNSUPPORTED;
    }

    RelocTarget += FixupSize;

    RelocOffset = UE_CHAINED_RELOC_FIXUP_NEXT_OFFSET (FixupInfo.Value32);
    FixupHdr    = (RelocOffset << 4U) | RelocType;

    Status = UnchainReloc (
               RuntimeContext,
               (CONST UINT8 *)&FixupHdr,
               sizeof (FixupHdr),
               FALSE,
               0,
               &FixupData
               );
    if (RETURN_ERROR (Status)) {
      return Status;
    }

    if (RelocOffset == UE_CHAINED_RELOC_FIXUP_OFFSET_END) {
      *ChainStart = RelocTarget;
      return RETURN_SUCCESS;
    }
    //
    // It holds that ImageSize mod 4 KiB = 0, thus ImageSize <= 0xFFFFF000.
    // Furthermore, it holds that RelocTarget <= ImageSize.
    // Finally, it holds that RelocOffset <= 0xFFE.
    // It follows that this cannot overflow.
    //
    RelocTarget += RelocOffset;
    ASSERT (RelocOffset <= RelocTarget);

    RelocType = UE_CHAINED_RELOC_FIXUP_NEXT_TYPE (FixupInfo.Value32);
  }
}

STATIC
RETURN_STATUS
InternaRelocateImage (
  IN OUT VOID                       *Image,
  IN     UINT32                     ImageSize,
  IN     UINT8                      Machine,
  IN     UINT64                     OldBaseAddress,
  IN     CONST VOID                 *RelocTable,
  IN     UINT32                     RelocTableSize,
  IN     BOOLEAN                    Chaining,
  IN     UINT64                     BaseAddress,
     OUT UE_LOADER_RUNTIME_CONTEXT  *RuntimeContext OPTIONAL,
  IN     BOOLEAN                    IsRuntime
  )
{
  RETURN_STATUS        Status;
  BOOLEAN              Overflow;

  UINT64               Adjust;

  UINT32               RootOffsetMax;
  UINT32               EntryOffsetMax;

  UINT32               TableOffset;
  CONST UE_FIXUP_ROOT  *RelocRoot;

  UINT16               FixupInfo;
  UINT16               RelocType;
  UINT16               RelocOffset;
  UINT32               RelocTarget;

  UINT32               OldTableOffset;
  UINT64               FixupData;
  UINT64               *FixupPointer;

  ASSERT (Image != NULL);
  ASSERT (RelocTable != NULL || RelocTableSize == 0);
  //
  // Verify the Relocation Directory is not empty.
  //
  if (RelocTableSize == 0) {
    return RETURN_SUCCESS;
  }
  //
  // Calculate the image displacement from its prefered load address.
  //
  Adjust = BaseAddress - OldBaseAddress;
  //
  // FIXME: RT driver check is removed in the hope we can force no relocs in
  //        writable segments.
  //
  // Skip explicit Relocation when the image is already loaded at its base
  // address.
  //
  if (Adjust == 0 && !Chaining) {
    return RETURN_SUCCESS;
  }

  RelocTarget = 0;

  if (IsRuntime) {
    FixupPointer = RuntimeContext->FixupData;
  }

  STATIC_ASSERT (
    MIN_SIZE_OF_UE_FIXUP_ROOT <= UE_LOAD_TABLE_ALIGNMENT,
    "The following arithmetic may overflow."
    );

  RootOffsetMax  = RelocTableSize - MIN_SIZE_OF_UE_FIXUP_ROOT;
  EntryOffsetMax = RelocTableSize - sizeof (*RelocRoot->Heads);
  //
  // Apply all relocation fixups of the Image.
  //
  for (TableOffset = 0; TableOffset <= RootOffsetMax;) {
    RelocRoot = (CONST UE_FIXUP_ROOT *)(
                  (CONST UINT8 *)RelocTable + TableOffset
                  );
    TableOffset += sizeof (*RelocRoot);

    Overflow = BaseOverflowAddU32 (
                 RelocTarget,
                 RelocRoot->FirstOffset,
                 &RelocTarget
                 );
    if (Overflow) {
      DEBUG_RAISE ();
      return RETURN_VOLUME_CORRUPTED;
    }

    Status = UnchainReloc (
               RuntimeContext,
               (CONST UINT8 *)RelocRoot,
               sizeof (*RelocRoot),
               IsRuntime,
               0,
               NULL
               );
    if (RETURN_ERROR (Status)) {
      return Status;
    }
    //
    // Process all relocation fixups of the current root.
    //
    while (TRUE) {
      FixupInfo = *(CONST UINT16 *)((CONST UINT8 *)RelocTable + TableOffset);
      //
      // This cannot overflow due to the upper bound of TableOffset.
      //
      TableOffset += sizeof (*RelocRoot->Heads);
      //
      // Apply the image relocation fixup.
      //
      RelocType = UE_RELOC_FIXUP_TYPE (FixupInfo);

      if (Chaining && !IsRuntime && (RelocType != UeReloc32NoMeta)) {
        Status = InternalProcessRelocChain (
                   Image,
                   ImageSize,
                   Machine,
                   RelocType,
                   &RelocTarget,
                   Adjust,
                   RuntimeContext
                   );
      } else {
        Status = InternalApplyRelocation (
                   Image,
                   ImageSize,
                   Machine,
                   RelocType,
                   &RelocTarget,
                   Adjust,
                   IsRuntime ? FixupPointer : &FixupData,
                   IsRuntime
                   );

        if (RETURN_ERROR (Status)) {
          return Status;
        }

        Status = UnchainReloc (
                   RuntimeContext,
                   (CONST UINT8 *)&FixupInfo,
                   sizeof (FixupInfo),
                   IsRuntime,
                   0,
                   &FixupData
                   );

        if (IsRuntime) {
          ++FixupPointer;
        }
      }

      if (RETURN_ERROR (Status)) {
        return Status;
      }

      RelocOffset = UE_RELOC_FIXUP_OFFSET (FixupInfo);

      Status = UnchainReloc (
                 RuntimeContext,
                 NULL,
                 0,
                 IsRuntime,
                 RelocOffset,
                 NULL
                 );
      if (RETURN_ERROR (Status)) {
        return Status;
      }

      if (RelocOffset == UE_HEAD_FIXUP_OFFSET_END) {
        break;
      }
      //
      // It holds that ImageSize mod 4 KiB = 0, thus ImageSize <= 0xFFFFF000.
      // Furthermore, it holds that RelocTarget <= ImageSize.
      // Finally, it holds that RelocOffset <= 0xFFE.
      // It follows that this cannot overflow.
      //
      RelocTarget += RelocOffset;
      ASSERT (RelocOffset <= RelocTarget);

      if (TableOffset > EntryOffsetMax) {
        DEBUG_RAISE ();
        return RETURN_VOLUME_CORRUPTED;
      }
    }
    //
    // This cannot overflow due to the TableOffset upper bounds and the
    // alignment guarantee of RelocTableSize.
    //
    OldTableOffset = TableOffset;
    TableOffset    = ALIGN_VALUE (TableOffset, ALIGNOF (UE_FIXUP_ROOT));
    ASSERT (OldTableOffset <= TableOffset);
  }

  STATIC_ASSERT (
    sizeof (UE_FIXUP_ROOT) <= UE_LOAD_TABLE_ALIGNMENT,
    "The following ASSERT may not hold."
    );

  return RETURN_SUCCESS;
}

RETURN_STATUS
UeRelocateImage (
  IN OUT UE_LOADER_IMAGE_CONTEXT    *Context,
  IN     UINT64                     BaseAddress,
     OUT UE_LOADER_RUNTIME_CONTEXT  *RuntimeContext OPTIONAL,
  IN     UINT32                     RuntimeContextSize
  )
{
  CONST UE_HEADER  *UeHdr;
  BOOLEAN          Chaining;
  CONST VOID       *RelocTable;

  ASSERT (Context != NULL);
  ASSERT (IS_ALIGNED (Context->LoadTablesFileOffset, UE_LOAD_TABLE_ALIGNMENT));
  ASSERT (IS_ALIGNED (Context->RelocTableSize, UE_LOAD_TABLE_ALIGNMENT));
  ASSERT (IS_ALIGNED (BaseAddress, Context->SegmentAlignment));
  ASSERT (RuntimeContext != NULL || RuntimeContextSize == 0);
  ASSERT (RuntimeContextSize == 0 || sizeof (*RuntimeContext) <= RuntimeContextSize);

  RelocTable = Context->FileBuffer + Context->LoadTablesFileOffset;

  UeHdr    = (CONST UE_HEADER *)Context->FileBuffer;
  Chaining = (UeHdr->ImageInfo & UE_HEADER_IMAGE_INFO_CHAINED_FIXUPS) != 0;

  if (RuntimeContext != NULL) {
    RuntimeContext->Machine = Context->Machine;
  }

  if (Context->XIP) {
    Context->EntryPointAddress -= Context->SegmentsFileOffset;
  }

  return InternaRelocateImage (
           Context->ImageBuffer,
           Context->ImageSize,
           Context->Machine,
           Context->XIP ? (Context->BaseAddress + Context->SegmentsFileOffset) : Context->BaseAddress,
           RelocTable,
           Context->RelocTableSize,
           Chaining,
           BaseAddress,
           RuntimeContext,
           FALSE
           );
}

RETURN_STATUS
UeRelocateImageForRuntime (
  IN OUT VOID                         *Image,
  IN     UINT32                       ImageSize,
  IN CONST UE_LOADER_RUNTIME_CONTEXT  *RuntimeContext,
  IN UINT64                           BaseAddress
  )
{
  ASSERT (RuntimeContext != NULL);

  return InternaRelocateImage (
           Image,
           ImageSize,
           RuntimeContext->Machine,
           (UINTN)Image,
           RuntimeContext->UnchainedRelocs,
           RuntimeContext->UnchainedRelocsSize,
           FALSE,
           BaseAddress,
           (UE_LOADER_RUNTIME_CONTEXT *)RuntimeContext,
           TRUE
           );
}

STATIC
RETURN_STATUS
InternalGetDebugTable (
  IN  CONST UE_LOADER_IMAGE_CONTEXT  *Context,
  OUT CONST UE_DEBUG_TABLE           **DebugTable
  )
{
  BOOLEAN               Overflow;
  CONST UE_LOAD_TABLE   *LoadTable;
  UINT8                 LoadTableIndex;
  UINT8                 NumLoadTables;
  UINT32                LoadTableFileOffset;
  UINT32                LoadTableFileSize;
  CONST UE_DEBUG_TABLE  *DbgTable;
  CONST UE_SEGMENT      *Segments;
  UINT16                NumSegments;
  UINT32                LoadTableExtraSize;
  UINT32                MinLoadTableExtraSize;

  ASSERT (Context != NULL);
  ASSERT (DebugTable != NULL);

  LoadTableFileOffset = Context->LoadTablesFileOffset;
  NumLoadTables       = Context->NumLoadTables;

  for (LoadTableIndex = 0; LoadTableIndex < NumLoadTables; ++LoadTableIndex) {
    LoadTable = Context->LoadTables + LoadTableIndex;
    if (UE_LOAD_TABLE_ID (LoadTable->FileInfo) != UeLoadTableIdDebug) {
      LoadTableFileOffset += UE_LOAD_TABLE_SIZE (LoadTable->FileInfo);
      continue;
    }

    ASSERT (IS_ALIGNED (LoadTableFileOffset, ALIGNOF (UE_DEBUG_TABLE)));

    LoadTableFileSize = UE_LOAD_TABLE_SIZE (LoadTable->FileInfo);

    Overflow = BaseOverflowSubU32 (
                 LoadTableFileSize,
                 MIN_SIZE_OF_UE_DEBUG_TABLE,
                 &LoadTableExtraSize
                 );
    if (Overflow) {
      DEBUG_RAISE ();
      return RETURN_VOLUME_CORRUPTED;
    }

    DbgTable = (CONST UE_DEBUG_TABLE *)(CONST VOID *)(
                 Context->FileBuffer + LoadTableFileOffset
                 );

    NumSegments = UeGetSegments (Context, &Segments);

    MinLoadTableExtraSize = DbgTable->SymbolsPathLength +
      (UINT32)NumSegments * sizeof (UE_SEGMENT_NAME);
    if (MinLoadTableExtraSize > LoadTableExtraSize) {
      DEBUG_RAISE ();
      return RETURN_VOLUME_CORRUPTED;
    }

    *DebugTable = DbgTable;
    return RETURN_SUCCESS;
  }

  return RETURN_NOT_FOUND;
}

RETURN_STATUS
UeGetSymbolsPath (
  IN  CONST UE_LOADER_IMAGE_CONTEXT  *Context,
  OUT CONST CHAR8                    **SymbolsPath,
  OUT UINT32                         *SymbolsPathSize
  )
{
  RETURN_STATUS         Status;
  CONST UE_DEBUG_TABLE  *DebugTable;

  ASSERT (Context != NULL);
  ASSERT (SymbolsPath != NULL);
  ASSERT (SymbolsPathSize != NULL);

  Status = InternalGetDebugTable (Context, &DebugTable);
  if (RETURN_ERROR (Status)) {
    return Status;
  }

  if (DebugTable->SymbolsPathLength == 0) {
    return RETURN_NOT_FOUND;
  }

  if (DebugTable->SymbolsPath[DebugTable->SymbolsPathLength] != 0) {
    return RETURN_VOLUME_CORRUPTED;
  }

  *SymbolsPath     = (CONST CHAR8 *)DebugTable->SymbolsPath;
  *SymbolsPathSize = (UINT32)DebugTable->SymbolsPathLength + 1;
  return RETURN_SUCCESS;
}

UINTN
UeLoaderGetImageDebugAddress (
  IN CONST UE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  RETURN_STATUS         Status;
  CONST UE_DEBUG_TABLE  *DebugTable;
  UINT8                 SymOffsetFactor;
  UINT32                SymOffsetSubtrahend;

  ASSERT (Context != NULL);

  SymOffsetSubtrahend = 0;

  Status = InternalGetDebugTable (Context, &DebugTable);
  if (!RETURN_ERROR (Status)) {
    SymOffsetFactor = UE_DEBUG_TABLE_IMAGE_INFO_SYM_SUBTRAHEND_FACTOR (
                        DebugTable->ImageInfo
                        );
    SymOffsetSubtrahend = (UINT32)SymOffsetFactor * Context->SegmentAlignment;
  }

  return UeLoaderGetImageAddress (Context) - SymOffsetSubtrahend;
}

RETURN_STATUS
UeGetSegmentNames (
  IN  CONST UE_LOADER_IMAGE_CONTEXT  *Context,
  OUT CONST UE_SEGMENT_NAME          **SegmentNames
  )
{
  RETURN_STATUS          Status;
  CONST UE_DEBUG_TABLE   *DebugTable;

  ASSERT (Context != NULL);
  ASSERT (SegmentNames != NULL);

  Status = InternalGetDebugTable (Context, &DebugTable);
  if (RETURN_ERROR (Status)) {
    return Status;
  }

  *SegmentNames = UE_DEBUG_TABLE_SEGMENT_NAMES (DebugTable);

  return RETURN_SUCCESS;
}

UINT32
UeGetEntryPointAddress (
  IN CONST UE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  ASSERT (Context != NULL);

  return Context->EntryPointAddress;
}

UINT16
UeGetMachine (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  ASSERT (Context != NULL);

  return Context->Machine;
}

UINT16
UeGetSubsystem (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  ASSERT (Context != NULL);

  return Context->Subsystem;
}

UINT32
UeGetSegmentAlignment (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  ASSERT (Context != NULL);

  return Context->SegmentAlignment;
}

UINT32
UeGetImageSize (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  ASSERT (Context != NULL);

  return Context->ImageSize;
}

UINT64
UeGetBaseAddress (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  ASSERT (Context != NULL);

  return Context->BaseAddress;
}

BOOLEAN
UeGetRelocsStripped (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  ASSERT (Context != NULL);

  return Context->RelocsStripped;
}

BOOLEAN
UeGetFixedAddress(
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  ASSERT (Context != NULL);

  return Context->FixedAddress;
}

UINTN
UeLoaderGetImageAddress (
  IN CONST UE_LOADER_IMAGE_CONTEXT  *Context
  )
{
  ASSERT (Context != NULL);

  return (UINTN) Context->ImageBuffer;
}

UINT16
UeGetSegments (
  IN  CONST UE_LOADER_IMAGE_CONTEXT  *Context,
  OUT CONST UE_SEGMENT               **Segments
  )
{
  ASSERT (Context != NULL);
  ASSERT (Segments != NULL);

  *Segments = Context->Segments;
  return (UINT16)Context->LastSegmentIndex + 1;
}

UINT16
UeGetSegmentImageInfos (
  IN OUT UE_LOADER_IMAGE_CONTEXT  *Context,
  OUT    CONST UINT32             **SegmentImageInfos,
  OUT    UINT8                    *SegmentImageInfoIterSize
  )
{
  ASSERT (Context != NULL);
  ASSERT (SegmentImageInfos != NULL);
  ASSERT (SegmentImageInfoIterSize != NULL);

  ASSERT (IS_ALIGNED (Context->SegmentImageInfoIterSize, ALIGNOF (UINT32)));

  *SegmentImageInfos        = Context->Segments;
  *SegmentImageInfoIterSize = Context->SegmentImageInfoIterSize;
  return (UINT16)Context->LastSegmentIndex + 1;
}
