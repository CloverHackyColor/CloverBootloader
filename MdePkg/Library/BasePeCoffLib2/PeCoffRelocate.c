/** @file
  Implements APIs to relocate PE/COFF Images.

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

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseOverflowLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/PeCoffLib2.h>

#include "BasePeCoffLib2Internals.h"

struct PE_COFF_LOADER_RUNTIME_CONTEXT_ {
  ///
  /// The RVA of the Relocation Directory.
  ///
  UINT32 RelocDirRva;
  ///
  /// The size, in Bytes, of the Relocation Directory.
  ///
  UINT32 RelocDirSize;
  ///
  /// Information bookkept during the initial Image relocation.
  ///
  UINT64 FixupData[];
};

// FIXME: Add RISC-V support.
/**
  Returns whether the Base Relocation type is supported by this loader.

  @param[in] Type  The type of the Base Relocation.
**/
#define IMAGE_RELOC_TYPE_SUPPORTED(Type) \
  (((Type) == EFI_IMAGE_REL_BASED_ABSOLUTE) || \
  ((Type) == EFI_IMAGE_REL_BASED_HIGHLOW) || \
  ((Type) == EFI_IMAGE_REL_BASED_DIR64) || \
  ((PcdGet32 (PcdImageLoaderRelocTypePolicy) & PCD_RELOC_TYPE_POLICY_ARM) != 0 && (Type) == EFI_IMAGE_REL_BASED_ARM_MOV32T))

/**
  Returns whether the Base Relocation is supported by this loader.

  @param[in] Relocation  The composite Base Relocation value.
**/
#define IMAGE_RELOC_SUPPORTED(Relocation) \
  IMAGE_RELOC_TYPE_SUPPORTED (IMAGE_RELOC_TYPE (Reloc))

/**
  Retrieve the immediate data encoded in an ARM MOVT or MOVW immediate
  instruciton.

  @param[in] Instruction  Pointer to an ARM MOVT or MOVW immediate instruction.

  @returns  The Immediate address encoded in the instruction.
**/
STATIC
UINT16
ThumbMovtImmediateAddress (
  IN CONST VOID  *Instruction
  )
{
  UINT32 Movt;
  UINT16 Movt1;
  UINT16 Movt2;
  UINT16 Address;
  //
  // Thumb2 is two separate 16-bit instructions working together, e.g.
  // MOVT R0, #0 is 0x0000f2c0 or 0xf2c0 0x0000
  //
  Movt1 = *(CONST UINT16 *) (CONST VOID *) Instruction;
  Movt2 = *(CONST UINT16 *) (CONST VOID *) ((CONST CHAR8 *) Instruction + sizeof (UINT16));
  Movt  = ((UINT32) Movt1 << 16U) | (UINT32) Movt2;
  //
  // imm16 = imm4:i:imm3:imm8
  //         imm4 -> Bit19:Bit16
  //         i    -> Bit26
  //         imm3 -> Bit14:Bit12
  //         imm8 -> Bit7:Bit0
  //
  Address  = (UINT16) (Movt & 0x000000FFU);         // imm8
  Address |= (UINT16) ((Movt >> 4U) & 0x0000F700U); // imm4 imm3
  Address |= (UINT16) ((Movt & BIT26) >> 15U);      // i, Bit26->11

  return Address;
}

/**
  Update an ARM MOVT or MOVW immediate instruction immediate data.

  @param[in,out] Instruction  Pointer to an ARM MOVT or MOVW immediate
                              instruction.
  @param[in]     Address      New address to patch into the instruction.
**/
STATIC
VOID
ThumbMovtImmediatePatch (
  IN OUT VOID    *Instruction,
  IN     UINT16  Address
  )
{
  UINT16 Patch;
  UINT16 PatchedInstruction;
  //
  // First 16-bit chunk of instruction.
  //
  Patch  = (Address & 0xF000U) >> 12U; // imm4
  Patch |= (Address & BIT11) >> 1U;    // i, Bit11->10
  //
  // Mask out instruction bits and or in address.
  //
  PatchedInstruction = *(CONST UINT16 *) (CONST VOID *) Instruction;
  *(UINT16 *) (VOID *) Instruction = (PatchedInstruction & ~(UINT16) 0x040FU) | Patch;
  //
  // Second 16-bit chunk of instruction.
  //
  Patch  = Address & 0x000000FFU;                  // imm8
  Patch |= ((UINT32) Address << 4U) & 0x00007000U; // imm3
  //
  // Mask out instruction bits and or in address.
  //
  PatchedInstruction = *(CONST UINT16 *) (CONST VOID *) ((CHAR8 *) Instruction + sizeof (UINT16));
  *(UINT16 *) (VOID *) ((CHAR8 *) Instruction + sizeof (UINT16)) =
    (PatchedInstruction & ~(UINT16) 0x70FFU) | Patch;
}

UINT32
PeCoffThumbMovwMovtImmediateAddress (
  IN CONST VOID  *Instructions
  )
{
  CONST CHAR8 *Word;
  CONST CHAR8 *Top;
  //
  // Calculate the encoded address of the instruction pair.
  //
  Word = Instructions;                                        // MOVW
  Top  = (CONST CHAR8 *) Instructions + 2 * sizeof (UINT16);  // MOVT

  return (UINT32) (((UINT32) ThumbMovtImmediateAddress (Top) << 16U) | ThumbMovtImmediateAddress (Word));
}

/**
  Update an ARM MOVW/MOVT immediate instruction instruction pair.

  @param[in,out] Instructions  Pointer to ARM MOVW/MOVT instruction pair.
  @param[in]     Address       New address to patch into the instructions.
**/
STATIC
VOID
ThumbMovwMovtImmediatePatch (
  IN OUT VOID    *Instructions,
  IN     UINT32  Address
  )
{
  CHAR8 *Word;
  CHAR8 *Top;
  //
  // Patch the instruction pair's encoded address.
  //
  Word = Instructions;                                  // MOVW
  Top  = (CHAR8 *) Instructions + 2 * sizeof (UINT16);  // MOVT

  ThumbMovtImmediatePatch (Word, (UINT16) (Address & 0x0000FFFFU));
  ThumbMovtImmediatePatch (Top, (UINT16) ((Address & 0xFFFF0000U) >> 16U));
}

/**
  Relocate an ARM MOVW/MOVT immediate instruction instruction pair.

  @param[in,out] Instructions  Pointer to ARM MOVW/MOVT instruction pair.
  @param[in]     Adjust        The delta to add to the addresses.
**/
VOID
PeCoffThumbMovwMovtImmediateFixup (
  IN OUT VOID    *Instructions,
  IN     UINT64  Adjust
  )
{
  UINT32 Fixup32;
  //
  // Relocate the instruction pair.
  //
  Fixup32 = PeCoffThumbMovwMovtImmediateAddress (Instructions) + (UINT32) Adjust;
  ThumbMovwMovtImmediatePatch (Instructions, Fixup32);
}

/**
  Apply an Image Base Relocation.

  Only a subset of the PE/COFF Base Relocation types are permited.
  The Base Relocation target must be in bounds, aligned, and must not overlap
  with the Relocation Directory.

  @param[in]  Context     The context describing the Image. Must have been
                          loaded by PeCoffLoadImage().
  @param[in]  RelocBlock  The Base Relocation Block to apply from.
  @param[in]  RelocIndex  The index of the Base Relocation to apply.
  @param[in]  Adjust      The delta to add to the addresses.
  @param[out] FixupData   On input, a pointer to a bookkeeping value or NULL.
                          On output, a value to preserve for Image runtime
                          relocation.

  @retval RETURN_SUCCESS  The Base Relocation has been applied successfully.
  @retval other           The Base Relocation could not be applied successfully.
**/
STATIC
RETURN_STATUS
InternalApplyRelocation (
  IN  CONST PE_COFF_LOADER_IMAGE_CONTEXT     *Context,
  IN  CONST EFI_IMAGE_BASE_RELOCATION_BLOCK  *RelocBlock,
  IN  UINT32                                 RelocIndex,
  IN  UINT64                                 Adjust,
  OUT UINT64                                 *FixupData OPTIONAL
  )
{
  BOOLEAN Overflow;

  UINT16  RelocType;
  UINT16  RelocOffset;
  UINT32  RelocTargetRva;
  UINT32  RemRelocTargetSize;

  CHAR8   *Fixup;
  UINT32  Fixup32;
  UINT64  Fixup64;

  RelocType   = IMAGE_RELOC_TYPE (RelocBlock->Relocations[RelocIndex]);
  RelocOffset = IMAGE_RELOC_OFFSET (RelocBlock->Relocations[RelocIndex]);
  //
  // Absolute Base Relocations are used for padding any must be skipped.
  //
  if (RelocType == EFI_IMAGE_REL_BASED_ABSOLUTE) {
    if (FixupData != NULL) {
      FixupData[RelocIndex] = 0;
    }

    return RETURN_SUCCESS;
  }
  //
  // Verify the Base Relocation target address is in bounds of the Image buffer.
  //
  Overflow = BaseOverflowAddU32 (
               RelocBlock->VirtualAddress,
               RelocOffset,
               &RelocTargetRva
               );
  if (Overflow) {
    DEBUG_RAISE ();
    return RETURN_VOLUME_CORRUPTED;
  }

  Overflow = BaseOverflowSubU32 (
               Context->SizeOfImage,
               RelocTargetRva,
               &RemRelocTargetSize
               );
  if (Overflow) {
    DEBUG_RAISE ();
    return RETURN_VOLUME_CORRUPTED;
  }

  Fixup = (CHAR8 *) Context->ImageBuffer + RelocTargetRva;
  //
  // Apply the Base Relocation fixup per type.
  // If RuntimeContext is not NULL, store the current value of the fixup
  // target to determine whether it has been changed during runtime
  // execution.
  //
  // It is not clear how EFI_IMAGE_REL_BASED_HIGH and
  // EFI_IMAGE_REL_BASED_LOW are supposed to be handled. While the PE
  // specification suggests to just add the high or low part of the
  // displacement, there are concerns about how it's supposed to deal with
  // wraparounds. As they are virtually non-existent, they are unsupported for
  // the time being.
  //
  switch (RelocType) {
    case EFI_IMAGE_REL_BASED_HIGHLOW:
      //
      // Verify the Base Relocation target is in bounds of the Image buffer.
      //
      if (sizeof (UINT32) > RemRelocTargetSize) {
        DEBUG_RAISE ();
        return RETURN_VOLUME_CORRUPTED;
      }
      //
      // Verify the Image Base Relocation does not target the Image Relocation
      // Directory.
      //
      if (RelocTargetRva + sizeof (UINT32) > Context->RelocDirRva
       && Context->RelocDirRva + Context->RelocDirSize > RelocTargetRva) {
        DEBUG_RAISE ();
        return RETURN_VOLUME_CORRUPTED;
      }
      //
      // Relocate the target instruction.
      //
      Fixup32  = ReadUnaligned32 ((CONST VOID *) Fixup);
      Fixup32 += (UINT32) Adjust;
      WriteUnaligned32 ((VOID *) Fixup, Fixup32);
      //
      // Record the relocated value for Image runtime relocation.
      //
      if (FixupData != NULL) {
        FixupData[RelocIndex] = Fixup32;
      }

      break;

    case EFI_IMAGE_REL_BASED_DIR64:
      //
      // Verify the Image Base Relocation target is in bounds of the Image
      // buffer.
      //
      if (sizeof (UINT64) > RemRelocTargetSize) {
        DEBUG_RAISE ();
        return RETURN_VOLUME_CORRUPTED;
      }
      //
      // Verify the Image Base Relocation does not target the Image Relocation
      // Directory.
      //
      if (RelocTargetRva + sizeof (UINT64) > Context->RelocDirRva
       && Context->RelocDirRva + Context->RelocDirSize > RelocTargetRva) {
        DEBUG_RAISE ();
        return RETURN_VOLUME_CORRUPTED;
      }
      //
      // Relocate target the instruction.
      //
      Fixup64  = ReadUnaligned64 ((CONST VOID *) Fixup);
      Fixup64 += Adjust;
      WriteUnaligned64 ((VOID *) Fixup, Fixup64);
      //
      // Record the relocated value for Image runtime relocation.
      //
      if (FixupData != NULL) {
        FixupData[RelocIndex] = Fixup64;
      }

      break;

    case EFI_IMAGE_REL_BASED_ARM_MOV32T:
      //
      // Verify ARM Thumb mode Base Relocations are supported.
      //
      if ((PcdGet32 (PcdImageLoaderRelocTypePolicy) & PCD_RELOC_TYPE_POLICY_ARM) == 0) {
        DEBUG_RAISE ();
        return RETURN_VOLUME_CORRUPTED;
      }
      //
      // Verify the Base Relocation target is in bounds of the Image buffer.
      //
      if (sizeof (UINT64) > RemRelocTargetSize) {
        DEBUG_RAISE ();
        return RETURN_VOLUME_CORRUPTED;
      }
      //
      // Verify the Base Relocation target is sufficiently aligned.
      // The ARM Thumb instruction pair must start on a 16-bit boundary.
      //
      if (!IS_ALIGNED (RelocTargetRva, ALIGNOF (UINT16))) {
        DEBUG_RAISE ();
        return RETURN_VOLUME_CORRUPTED;
      }
      //
      // Verify the Base Relocation does not target the Relocation Directory.
      //
      if (RelocTargetRva + sizeof (UINT64) > Context->RelocDirRva
       && Context->RelocDirRva + Context->RelocDirSize > RelocTargetRva) {
        DEBUG_RAISE ();
        return RETURN_VOLUME_CORRUPTED;
      }
      //
      // Relocate the target instruction.
      //
      PeCoffThumbMovwMovtImmediateFixup (Fixup, Adjust);
      //
      // Record the relocated value for Image runtime relocation.
      //
      if (FixupData != NULL) {
        FixupData[RelocIndex] = ReadUnaligned64 ((CONST VOID *) Fixup);
      }

      break;

    default:
      //
      // The Image Base Relocation type is unknown, disallow the Image.
      //
      DEBUG_RAISE ();
      return RETURN_UNSUPPORTED;
  }

  return RETURN_SUCCESS;
}

RETURN_STATUS
PeCoffRelocateImage (
  IN OUT PE_COFF_LOADER_IMAGE_CONTEXT    *Context,
  IN     UINT64                          BaseAddress,
  OUT    PE_COFF_LOADER_RUNTIME_CONTEXT  *RuntimeContext OPTIONAL,
  IN     UINT32                          RuntimeContextSize
  )
{
  RETURN_STATUS                         Status;
  BOOLEAN                               Overflow;

  UINT64                                Adjust;

  UINT32                                RelocBlockOffsetMax;
  UINT32                                TopOfRelocDir;

  UINT32                                RelocBlockOffset;
  CONST EFI_IMAGE_BASE_RELOCATION_BLOCK *RelocBlock;
  UINT32                                RelocBlockSize;
  UINT32                                SizeOfRelocs;
  UINT32                                NumRelocs;
  UINT32                                RelocIndex;
  UINT32                                FixupDataIndex;
  UINT64                                *CurrentFixupData;

  ASSERT (Context != NULL);
  ASSERT (!Context->RelocsStripped || BaseAddress == Context->ImageBase);
  ASSERT (RuntimeContext != NULL || RuntimeContextSize == 0);
  ASSERT (RuntimeContext == NULL || RuntimeContextSize >= sizeof (PE_COFF_LOADER_RUNTIME_CONTEXT) + Context->RelocDirSize * (sizeof (UINT64) / sizeof (UINT16)));
  //
  // Initialise the Image runtime context header.
  //
  if (RuntimeContext != NULL) {
    RuntimeContext->RelocDirRva  = Context->RelocDirRva;
    RuntimeContext->RelocDirSize = Context->RelocDirSize;
  }
  //
  // Verify the Relocation Directory is not empty.
  //
  if (Context->RelocDirSize == 0) {
    return RETURN_SUCCESS;
  }
  //
  // Calculate the Image displacement from its prefered load address.
  //
  Adjust = BaseAddress - Context->ImageBase;
  //
  // Runtime drivers should unconditionally go through the full Relocation
  // procedure early to eliminate the possibility of errors later at runtime.
  // Runtime drivers don't have their Base Relocations stripped, this is
  // verified during context creation.
  // Skip explicit Relocation when the Image is already loaded at its
  // prefered location.
  //
  if (RuntimeContext == NULL && Adjust == 0) {
    return RETURN_SUCCESS;
  }

  RelocBlockOffset    = Context->RelocDirRva;
  TopOfRelocDir       = Context->RelocDirRva + Context->RelocDirSize;
  RelocBlockOffsetMax = TopOfRelocDir - sizeof (EFI_IMAGE_BASE_RELOCATION_BLOCK);
  FixupDataIndex      = 0;
  //
  // Align TopOfRelocDir because, if the policy does not demand Relocation Block
  // sizes to be aligned, the code below will manually align them. Thus, the
  // end offset of the last Relocation Block must be compared to a manually
  // aligned Relocation Directoriy end offset.
  //
  if ((PcdGet32 (PcdImageLoaderAlignmentPolicy) & PCD_ALIGNMENT_POLICY_RELOCATION_BLOCK_SIZES) != 0) {
    Overflow = BaseOverflowAlignUpU32 (
                 TopOfRelocDir,
                 ALIGNOF (EFI_IMAGE_BASE_RELOCATION_BLOCK),
                 &TopOfRelocDir
                 );
    if (Overflow) {
      DEBUG_RAISE ();
      return RETURN_VOLUME_CORRUPTED;
    }
  }
  //
  // Apply all Base Relocations of the Image.
  //
  while (RelocBlockOffset <= RelocBlockOffsetMax) {
    RelocBlock = (CONST EFI_IMAGE_BASE_RELOCATION_BLOCK *) (CONST VOID *) (
                   (CONST CHAR8 *) Context->ImageBuffer + RelocBlockOffset
                   );
    //
    // Verify the Base Relocation Block size is well-formed.
    //
    Overflow = BaseOverflowSubU32 (
                 RelocBlock->SizeOfBlock,
                 sizeof (EFI_IMAGE_BASE_RELOCATION_BLOCK),
                 &SizeOfRelocs
                 );
    if (Overflow) {
      DEBUG_RAISE ();
      return RETURN_VOLUME_CORRUPTED;
    }
    //
    // Verify the Base Relocation Block is in bounds of the Relocation
    // Directory.
    //
    if (SizeOfRelocs > RelocBlockOffsetMax - RelocBlockOffset) {
      DEBUG_RAISE ();
      return RETURN_VOLUME_CORRUPTED;
    }
    //
    // Advance to the next Base Relocation Block offset based on the alignment
    // policy.
    //
    if ((PcdGet32 (PcdImageLoaderAlignmentPolicy) & PCD_ALIGNMENT_POLICY_RELOCATION_BLOCK_SIZES) == 0) {
      RelocBlockSize = RelocBlock->SizeOfBlock;
      //
      // Verify the next Base Relocation Block offset is sufficiently aligned.
      //
      if (!IS_ALIGNED (RelocBlockSize, ALIGNOF (EFI_IMAGE_BASE_RELOCATION_BLOCK))) {
        DEBUG_RAISE ();
        return RETURN_VOLUME_CORRUPTED;
      }
    } else {
      //
      // This arithmetic cannot overflow because we know
      //   1) RelocBlock->SizeOfBlock <= RelocMax <= TopOfRelocDir
      //   2) IS_ALIGNED (TopOfRelocDir, ALIGNOF (EFI_IMAGE_BASE_RELOCATION_BLOCK)).
      //
      RelocBlockSize = ALIGN_VALUE (
                         RelocBlock->SizeOfBlock,
                         ALIGNOF (EFI_IMAGE_BASE_RELOCATION_BLOCK)
                         );
    }
    //
    // This division is safe due to the guarantee made above.
    //
    NumRelocs = SizeOfRelocs / sizeof (*RelocBlock->Relocations);

    if (RuntimeContext != NULL) {
      CurrentFixupData = &RuntimeContext->FixupData[FixupDataIndex];
      //
      // This arithmetic cannot overflow because The number of Image Base
      // Relocations cannot exceed the size of their Image Relocation Block, and
      // latter has been verified to be in bounds of the Image buffer. The Image
      // buffer size and RelocDataIndex are both bound by MAX_UINT32.
      //
      FixupDataIndex += NumRelocs;
    } else {
      CurrentFixupData = NULL;
    }
    //
    // Process all Base Relocations of the current Block.
    //
    for (RelocIndex = 0; RelocIndex < NumRelocs; ++RelocIndex) {
      //
      // Apply the Image Base Relocation fixup.
      // If RuntimeContext is not NULL, store the current value of the fixup
      // target to determine whether it has been changed during runtime
      // execution.
      //
      // It is not clear how EFI_IMAGE_REL_BASED_HIGH and
      // EFI_IMAGE_REL_BASED_LOW are supposed to be handled. While PE reference
      // suggests to just add the high or low part of the displacement, there
      // are concerns about how it's supposed to deal with wraparounds.
      // As no known linker emits them, omit support.
      //
      Status = InternalApplyRelocation (
                 Context,
                 RelocBlock,
                 RelocIndex,
                 Adjust,
                 CurrentFixupData
                 );
      if (Status != RETURN_SUCCESS) {
        DEBUG_RAISE ();
        return Status;
      }
    }
    //
    // This arithmetic cannot overflow because it has been checked that the
    // Image Base Relocation Block is in bounds of the Image buffer.
    //
    RelocBlockOffset += RelocBlockSize;
  }
  //
  // Verify the Relocation Directory size matches the contained data.
  //
  if (RelocBlockOffset != TopOfRelocDir) {
    DEBUG_RAISE ();
    return RETURN_VOLUME_CORRUPTED;
  }
  //
  // Initialise the remaining uninitialised portion of the Image runtime
  // context.
  //
  if (RuntimeContext != NULL) {
    //
    // This arithmetic cannot overflow due to the guarantee given by
    // PeCoffLoaderGetRuntimeContextSize().
    //
    ZeroMem (
      &RuntimeContext->FixupData[FixupDataIndex],
      RuntimeContextSize - sizeof (PE_COFF_LOADER_RUNTIME_CONTEXT) - FixupDataIndex * sizeof (UINT64)
      );
  }

  return RETURN_SUCCESS;
}

/**
  Apply an Image Base Relocation for Image runtime relocation.

  Well-formedness has been verified by PeCoffRelocateImage() previously.
  Fails if the Relocation target value has changed since PeCoffRelocateImage().

  @param[in]  Image       The Image destination memory. Must have been relocated
                          by PeCoffRelocateImage().
  @param[in]  RelocBlock  The Base Relocation Block to apply from.
  @param[in]  RelocIndex  The index of the Base Relocation to apply.
  @param[in]  Adjust      The delta to add to the addresses.
  @param[out] FixupData   The bookkeeping value.

  @retval RETURN_SUCCESS  The Base Relocation has been applied successfully.
  @retval other           The Base Relocation could not be applied successfully.
**/
STATIC
RETURN_STATUS
InternalApplyRelocationRuntime (
  IN OUT VOID    *Fixup,
  IN     UINT16  RelocType,
  IN     UINT64  Adjust,
  IN     UINT64  FixupData
  )
{
  UINT32 Fixup32;
  UINT64 Fixup64;

  ASSERT (Fixup != NULL);
  ASSERT (IMAGE_RELOC_TYPE_SUPPORTED (RelocType)
       && RelocType != EFI_IMAGE_REL_BASED_ABSOLUTE);
  //
  // Apply the Image Base Relocation fixup per type.
  //
  // If the Image relocation target value has changes since the initial
  // Image relocation, skip or abort based on the policy. The fixup cannot
  // be applies safely as the value might not reference an address within
  // the Image memory space, e.g. when a global variable pointer to another
  // variable is changed during runtime.
  //
  switch (RelocType) {
    case EFI_IMAGE_REL_BASED_HIGHLOW:
      Fixup32 = ReadUnaligned32 ((CONST VOID *) Fixup);
      //
      // If the Image relocation target value mismatches, skip or abort.
      //
      if (FixupData != Fixup32) {
        if (PcdGetBool (PcdImageLoaderRtRelocAllowTargetMismatch)) {
          return RETURN_SUCCESS;
        }

        return RETURN_VOLUME_CORRUPTED;
      }

      Fixup32 += (UINT32) Adjust;
      WriteUnaligned32 (Fixup, Fixup32);

      break;

    case EFI_IMAGE_REL_BASED_DIR64:
      Fixup64 = ReadUnaligned64 (Fixup);
      //
      // If the Image relocation target value mismatches, skip or abort.
      //
      if (FixupData != Fixup64) {
        if (PcdGetBool (PcdImageLoaderRtRelocAllowTargetMismatch)) {
          return RETURN_SUCCESS;
        }

        return RETURN_VOLUME_CORRUPTED;
      }

      Fixup64 += Adjust;
      WriteUnaligned64 (Fixup, Fixup64);

      break;

    case EFI_IMAGE_REL_BASED_ARM_MOV32T:
      ASSERT ((PcdGet32 (PcdImageLoaderRelocTypePolicy) & PCD_RELOC_TYPE_POLICY_ARM) != 0);

      Fixup64 = ReadUnaligned64 (Fixup);
      //
      // If the Image relocation target value mismatches, skip or abort.
      //
      if (FixupData != Fixup64) {
        if (PcdGetBool (PcdImageLoaderRtRelocAllowTargetMismatch)) {
          return RETURN_SUCCESS;
        }

        return RETURN_VOLUME_CORRUPTED;
      }

      PeCoffThumbMovwMovtImmediateFixup (Fixup, Adjust);

      break;

    default:
      //
      // Invalid Base Relocation types would have caused the Image to not be
      // loaded relocated successfully earlier.
      //
      ASSERT (FALSE);
  }

  return RETURN_SUCCESS;
}

RETURN_STATUS
PeCoffLoaderGetRuntimeContextSize (
  IN OUT PE_COFF_LOADER_IMAGE_CONTEXT  *Context,
  OUT    UINT32                        *Size
  )
{
  BOOLEAN Overflow;
  UINT32  FixupDataSize;

  ASSERT (Context != NULL);
  ASSERT (!Context->RelocsStripped);
  ASSERT (Size != NULL);

  //
  // Because the ImageBase Relocations have not been stripped,
  // PeCoffInitializeContext() has verified the Relocation Directory exists and
  // is valid.
  //

  //
  // Request 64-bit of source value per 16-bit Base Relocation.
  // This allocates too many Bytes because it assumes that every Base Relocation
  // refers to a 64-bit target and does not account for Base Relocation Block
  // headers.
  //
  Overflow = BaseOverflowMulU32 (
               Context->RelocDirSize,
               sizeof (UINT64) / sizeof (UINT16),
               &FixupDataSize
               );
  if (Overflow) {
    return RETURN_UNSUPPORTED;
  }

  Overflow = BaseOverflowAddU32 (
               FixupDataSize,
               sizeof (PE_COFF_LOADER_RUNTIME_CONTEXT),
               Size
               );
  if (Overflow) {
    return RETURN_UNSUPPORTED;
  }

  return RETURN_SUCCESS;
}

RETURN_STATUS
PeCoffRuntimeRelocateImage (
  IN OUT VOID                                  *Image,
  IN     UINT32                                ImageSize,
  IN     UINT64                                BaseAddress,
  IN     CONST PE_COFF_LOADER_RUNTIME_CONTEXT  *RuntimeContext
  )
{
  RETURN_STATUS                         Status;

  UINTN                                 ImageAddress;
  UINT32                                FixupDataIndex;
  CONST EFI_IMAGE_BASE_RELOCATION_BLOCK *RelocBlock;
  UINT64                                Adjust;

  UINT32                                TopOfRelocDir;
  UINT32                                RelocBlockOffset;
  UINT32                                SizeOfRelocs;
  UINT32                                NumRelocs;
  UINT32                                RelocIndex;
  UINT32                                RelocTarget;
  UINT32                                RelocOffset;
  UINT32                                RelocBlockSize;

  (VOID) ImageSize;

  ASSERT (Image != NULL);
  ASSERT (BaseAddress != 0);
  ASSERT (RuntimeContext != NULL);
  //
  // If the relocation directory is empty, skip relocation.
  //
  if (RuntimeContext->RelocDirSize == 0) {
    return RETURN_SUCCESS;
  }

  //
  // The arithmetics in this function generally cannot overflow due to the
  // guarantees given by PeCoffRelocateImage().
  //

  ImageAddress = (UINTN) Image;
  Adjust       = BaseAddress - ImageAddress;
  //
  // If the Image remains at the current address, skip relocation.
  //
  if (Adjust == 0) {
    return RETURN_SUCCESS;
  }
  //
  // Apply all Base Relocations of the Image.
  //
  RelocBlockOffset = RuntimeContext->RelocDirRva;
  TopOfRelocDir    = RuntimeContext->RelocDirRva + RuntimeContext->RelocDirSize;
  FixupDataIndex   = 0;
  while (RelocBlockOffset < TopOfRelocDir) {
    ASSERT (IS_ALIGNED (RelocBlockOffset, ALIGNOF (EFI_IMAGE_BASE_RELOCATION_BLOCK)));

    RelocBlock = (CONST EFI_IMAGE_BASE_RELOCATION_BLOCK *) (CONST VOID *) (
                   (CONST CHAR8 *) Image + RelocBlockOffset
                   );

    STATIC_ASSERT (
      (sizeof (UINT32) % ALIGNOF (EFI_IMAGE_BASE_RELOCATION_BLOCK)) == 0,
      "The following accesses must be performed unaligned."
      );

    ASSERT (sizeof (EFI_IMAGE_BASE_RELOCATION_BLOCK) <= RelocBlock->SizeOfBlock);
    //
    // Determine the number of Image Base Relocations in this Block.
    //
    SizeOfRelocs = RelocBlock->SizeOfBlock - sizeof (EFI_IMAGE_BASE_RELOCATION_BLOCK);
    NumRelocs    = SizeOfRelocs / sizeof (*RelocBlock->Relocations);
    //
    // Apply all Image Base Relocation fixups of the current Image Base
    // Relocation Block.
    //
    for (RelocIndex = 0; RelocIndex < NumRelocs; ++RelocIndex) {
      //
      // Skip Absolute Image Base Relocations.
      //
      if (IMAGE_RELOC_TYPE (RelocBlock->Relocations[RelocIndex]) == EFI_IMAGE_REL_BASED_ABSOLUTE) {
        ++FixupDataIndex;
        continue;
      }
      //
      // Determine the Image Base Relocation target address.
      //
      RelocOffset = IMAGE_RELOC_OFFSET (RelocBlock->Relocations[RelocIndex]);
      ASSERT (RelocOffset <= RelocBlock->VirtualAddress + RelocOffset);
      //
      // Apply the Image Base Relocation fixup.
      //
      RelocTarget = RelocBlock->VirtualAddress + RelocOffset;
      Status = InternalApplyRelocationRuntime (
                 (CHAR8 *) Image + RelocTarget,
                 IMAGE_RELOC_TYPE (RelocBlock->Relocations[RelocIndex]),
                 Adjust,
                 RuntimeContext->FixupData[FixupDataIndex]
                 );
      //
      // If the original Image Relocation target value mismatches the expected
      // value, and the policy demands it, report an error.
      //
      if (!PcdGetBool (PcdImageLoaderRtRelocAllowTargetMismatch)) {
        if (Status != RETURN_SUCCESS) {
          return Status;
        }
      } else {
        ASSERT (Status == RETURN_SUCCESS);
      }

      ++FixupDataIndex;
    }
    //
    // Advance to the next Base Relocation Block based on the alignment policy.
    //
    if ((PcdGet32 (PcdImageLoaderAlignmentPolicy) & PCD_ALIGNMENT_POLICY_RELOCATION_BLOCK_SIZES) == 0) {
      RelocBlockSize = RelocBlock->SizeOfBlock;
      ASSERT (IS_ALIGNED (RelocBlockSize, ALIGNOF (EFI_IMAGE_BASE_RELOCATION_BLOCK)));
    } else {
      RelocBlockSize = ALIGN_VALUE (
                         RelocBlock->SizeOfBlock,
                         ALIGNOF (EFI_IMAGE_BASE_RELOCATION_BLOCK)
                         );
    }

    RelocBlockOffset += RelocBlockSize;
  }
  //
  // Align TopOfRelocDir because, if the policy does not demand Relocation Block
  // sizes to be aligned, the code below will manually align them. Thus, the
  // end offset of the last Relocation Block must be compared to a manually
  // aligned Relocation Directoriy end offset.
  //
  if ((PcdGet32 (PcdImageLoaderAlignmentPolicy) & PCD_ALIGNMENT_POLICY_RELOCATION_BLOCK_SIZES) != 0) {
    TopOfRelocDir = ALIGN_VALUE (TopOfRelocDir, ALIGNOF (EFI_IMAGE_BASE_RELOCATION_BLOCK));
  }
  //
  // This condition is verified by PeCoffRelocateImage().
  //
  ASSERT (RelocBlockOffset == TopOfRelocDir);

  return RETURN_SUCCESS;
}

RETURN_STATUS
PeCoffRelocateImageInplace (
  IN OUT PE_COFF_LOADER_IMAGE_CONTEXT  *Context
  )
{
  RETURN_STATUS Status;
  UINTN         NewBase;

  Status = PeCoffLoadImageInplaceNoBase (Context);
  if (RETURN_ERROR (Status)) {
    DEBUG_RAISE ();
    return Status;
  }

  NewBase = PeCoffLoaderGetImageAddress (Context);

  Status = PeCoffRelocateImage (Context, NewBase, NULL, 0);
  if (RETURN_ERROR (Status)) {
    DEBUG_RAISE ();
  }

  return Status;
}
