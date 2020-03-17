/** @file
Elf64 convert solution

Copyright (c) 2010 - 2018, Intel Corporation. All rights reserved.<BR>
Portions copyright (c) 2013-2014, ARM Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "WinNtInclude.h"

#ifndef __GNUC__
#include <windows.h>
#include <io.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include <Common/UefiBaseTypes.h>
#include <IndustryStandard/PeImage.h>

#include "PeCoffLib.h"
#include "EfiUtilityMsgs.h"

#include "GenFw.h"
#include "ElfConvert.h"
#include "Elf64Convert.h"

STATIC
VOID
ScanSections64 (
  VOID
  );

STATIC
BOOLEAN
WriteSections64 (
  SECTION_FILTER_TYPES  FilterType
  );

STATIC
VOID
WriteRelocations64 (
  VOID
  );

STATIC
VOID
WriteDebug64 (
  VOID
  );

STATIC
VOID
SetImageSize64 (
  VOID
  );

STATIC
VOID
CleanUp64 (
  VOID
  );

//
// Rename ELF32 structures to common names to help when porting to ELF64.
//
typedef Elf64_Shdr Elf_Shdr;
typedef Elf64_Ehdr Elf_Ehdr;
typedef Elf64_Rel Elf_Rel;
typedef Elf64_Rela Elf_Rela;
typedef Elf64_Sym Elf_Sym;
typedef Elf64_Phdr Elf_Phdr;
typedef Elf64_Dyn Elf_Dyn;
#define ELFCLASS ELFCLASS64
#define ELF_R_TYPE(r) ELF64_R_TYPE(r)
#define ELF_R_SYM(r) ELF64_R_SYM(r)

//
// Well known ELF structures.
//
STATIC Elf_Ehdr *mEhdr;
STATIC Elf_Shdr *mShdrBase;
STATIC Elf_Phdr *mPhdrBase;

//
// GOT information
//
STATIC Elf_Shdr *mGOTShdr = NULL;
STATIC UINT32   mGOTShindex = 0;
STATIC UINT32   *mGOTCoffEntries = NULL;
STATIC UINT32   mGOTMaxCoffEntries = 0;
STATIC UINT32   mGOTNumCoffEntries = 0;

//
// Coff information
//
STATIC UINT32 mCoffAlignment = 0x20;

//
// PE section alignment.
//
STATIC const UINT16 mCoffNbrSections = 4;

//
// ELF sections to offset in Coff file.
//
STATIC UINT32 *mCoffSectionsOffset = NULL;

//
// Offsets in COFF file
//
STATIC UINT32 mNtHdrOffset;
STATIC UINT32 mTextOffset;
STATIC UINT32 mDataOffset;
STATIC UINT32 mHiiRsrcOffset;
STATIC UINT32 mRelocOffset;
STATIC UINT32 mDebugOffset;

//
// Initialization Function
//
BOOLEAN
InitializeElf64 (
  UINT8               *FileBuffer,
  ELF_FUNCTION_TABLE  *ElfFunctions
  )
{
  //
  // Initialize data pointer and structures.
  //
  VerboseMsg ("Set EHDR");
  mEhdr = (Elf_Ehdr*) FileBuffer;

  //
  // Check the ELF64 specific header information.
  //
  VerboseMsg ("Check ELF64 Header Information");
  if (mEhdr->e_ident[EI_CLASS] != ELFCLASS64) {
    Error (NULL, 0, 3000, "Unsupported", "ELF EI_DATA not ELFCLASS64");
    return FALSE;
  }
  if (mEhdr->e_ident[EI_DATA] != ELFDATA2LSB) {
    Error (NULL, 0, 3000, "Unsupported", "ELF EI_DATA not ELFDATA2LSB");
    return FALSE;
  }
  if ((mEhdr->e_type != ET_EXEC) && (mEhdr->e_type != ET_DYN)) {
    Error (NULL, 0, 3000, "Unsupported", "ELF e_type not ET_EXEC or ET_DYN");
    return FALSE;
  }
  if (!((mEhdr->e_machine == EM_X86_64) || (mEhdr->e_machine == EM_AARCH64))) {
    Error (NULL, 0, 3000, "Unsupported", "ELF e_machine not EM_X86_64 or EM_AARCH64");
    return FALSE;
  }
  if (mEhdr->e_version != EV_CURRENT) {
    Error (NULL, 0, 3000, "Unsupported", "ELF e_version (%u) not EV_CURRENT (%d)", (unsigned) mEhdr->e_version, EV_CURRENT);
    return FALSE;
  }

  //
  // Update section header pointers
  //
  VerboseMsg ("Update Header Pointers");
  mShdrBase  = (Elf_Shdr *)((UINT8 *)mEhdr + mEhdr->e_shoff);
  mPhdrBase = (Elf_Phdr *)((UINT8 *)mEhdr + mEhdr->e_phoff);

  //
  // Create COFF Section offset buffer and zero.
  //
  VerboseMsg ("Create COFF Section Offset Buffer");
  mCoffSectionsOffset = (UINT32 *)malloc(mEhdr->e_shnum * sizeof (UINT32));
  if (mCoffSectionsOffset == NULL) {
    Error (NULL, 0, 4001, "Resource", "memory cannot be allocated!");
    return FALSE;
  }
  memset(mCoffSectionsOffset, 0, mEhdr->e_shnum * sizeof(UINT32));

  //
  // Fill in function pointers.
  //
  VerboseMsg ("Fill in Function Pointers");
  ElfFunctions->ScanSections = ScanSections64;
  ElfFunctions->WriteSections = WriteSections64;
  ElfFunctions->WriteRelocations = WriteRelocations64;
  ElfFunctions->WriteDebug = WriteDebug64;
  ElfFunctions->SetImageSize = SetImageSize64;
  ElfFunctions->CleanUp = CleanUp64;

  return TRUE;
}


//
// Header by Index functions
//
STATIC
Elf_Shdr*
GetShdrByIndex (
  UINT32 Num
  )
{
  if (Num >= mEhdr->e_shnum) {
    Error (NULL, 0, 3000, "Invalid", "GetShdrByIndex: Index %u is too high.", Num);
    exit(EXIT_FAILURE);
  }

  return (Elf_Shdr*)((UINT8*)mShdrBase + Num * mEhdr->e_shentsize);
}

STATIC
UINT32
CoffAlign (
  UINT32 Offset
  )
{
  return (Offset + mCoffAlignment - 1) & ~(mCoffAlignment - 1);
}

STATIC
UINT32
DebugRvaAlign (
  UINT32 Offset
  )
{
  return (Offset + 3) & ~3;
}

//
// filter functions
//
STATIC
BOOLEAN
IsTextShdr (
  Elf_Shdr *Shdr
  )
{
  return (BOOLEAN) ((Shdr->sh_flags & (SHF_WRITE | SHF_ALLOC)) == SHF_ALLOC);
}

STATIC
BOOLEAN
IsHiiRsrcShdr (
  Elf_Shdr *Shdr
  )
{
  Elf_Shdr *Namedr = GetShdrByIndex(mEhdr->e_shstrndx);

  return (BOOLEAN) (strcmp((CHAR8*)mEhdr + Namedr->sh_offset + Shdr->sh_name, ELF_HII_SECTION_NAME) == 0);
}

STATIC
BOOLEAN
IsDataShdr (
  Elf_Shdr *Shdr
  )
{
  if (IsHiiRsrcShdr(Shdr)) {
    return FALSE;
  }
  return (BOOLEAN) (Shdr->sh_flags & (SHF_WRITE | SHF_ALLOC)) == (SHF_ALLOC | SHF_WRITE);
}

STATIC
BOOLEAN
IsStrtabShdr (
  Elf_Shdr *Shdr
  )
{
  Elf_Shdr *Namedr = GetShdrByIndex(mEhdr->e_shstrndx);

  return (BOOLEAN) (strcmp((CHAR8*)mEhdr + Namedr->sh_offset + Shdr->sh_name, ELF_STRTAB_SECTION_NAME) == 0);
}

STATIC
Elf_Shdr *
FindStrtabShdr (
  VOID
  )
{
  UINT32 i;
  for (i = 0; i < mEhdr->e_shnum; i++) {
    Elf_Shdr *shdr = GetShdrByIndex(i);
    if (IsStrtabShdr(shdr)) {
      return shdr;
    }
  }
  return NULL;
}

STATIC
const UINT8 *
GetSymName (
  Elf_Sym *Sym
  )
{
  Elf_Shdr *StrtabShdr;
  UINT8    *StrtabContents;
  BOOLEAN  foundEnd;
  UINT32   i;

  if (Sym->st_name == 0) {
    return NULL;
  }

  StrtabShdr = FindStrtabShdr();
  if (StrtabShdr == NULL) {
    return NULL;
  }

  assert(Sym->st_name < StrtabShdr->sh_size);

  StrtabContents = (UINT8*)mEhdr + StrtabShdr->sh_offset;

  foundEnd = FALSE;
  for (i= Sym->st_name; (i < StrtabShdr->sh_size) && !foundEnd; i++) {
    foundEnd = (BOOLEAN)(StrtabContents[i] == 0);
  }
  assert(foundEnd);

  return StrtabContents + Sym->st_name;
}

//
// Find the ELF section hosting the GOT from an ELF Rva
//   of a single GOT entry.  Normally, GOT is placed in
//   ELF .text section, so assume once we find in which
//   section the GOT is, all GOT entries are there, and
//   just verify this.
//
STATIC
VOID
FindElfGOTSectionFromGOTEntryElfRva (
  Elf64_Addr GOTEntryElfRva
  )
{
  UINT32 i;
  if (mGOTShdr != NULL) {
    if (GOTEntryElfRva >= mGOTShdr->sh_addr &&
        GOTEntryElfRva <  mGOTShdr->sh_addr + mGOTShdr->sh_size) {
      return;
    }
    Error (NULL, 0, 3000, "Unsupported", "FindElfGOTSectionFromGOTEntryElfRva: GOT entries found in multiple sections.");
    exit(EXIT_FAILURE);
  }
  for (i = 0; i < mEhdr->e_shnum; i++) {
    Elf_Shdr *shdr = GetShdrByIndex(i);
    if (GOTEntryElfRva >= shdr->sh_addr &&
        GOTEntryElfRva <  shdr->sh_addr + shdr->sh_size) {
      mGOTShdr = shdr;
      mGOTShindex = i;
      return;
    }
  }
  Error (NULL, 0, 3000, "Invalid", "FindElfGOTSectionFromGOTEntryElfRva: ElfRva 0x%016LX for GOT entry not found in any section.", GOTEntryElfRva);
  exit(EXIT_FAILURE);
}

//
// Stores locations of GOT entries in COFF image.
//   Returns TRUE if GOT entry is new.
//   Simple implementation as number of GOT
//   entries is expected to be low.
//

STATIC
BOOLEAN
AccumulateCoffGOTEntries (
  UINT32 GOTCoffEntry
  )
{
  UINT32 i;
  if (mGOTCoffEntries != NULL) {
    for (i = 0; i < mGOTNumCoffEntries; i++) {
      if (mGOTCoffEntries[i] == GOTCoffEntry) {
        return FALSE;
      }
    }
  }
  if (mGOTCoffEntries == NULL) {
    mGOTCoffEntries = (UINT32*)malloc(5 * sizeof *mGOTCoffEntries);
    if (mGOTCoffEntries == NULL) {
      Error (NULL, 0, 4001, "Resource", "memory cannot be allocated!");
    }
    assert (mGOTCoffEntries != NULL);
    mGOTMaxCoffEntries = 5;
    mGOTNumCoffEntries = 0;
  } else if (mGOTNumCoffEntries == mGOTMaxCoffEntries) {
    mGOTCoffEntries = (UINT32*)realloc(mGOTCoffEntries, 2 * mGOTMaxCoffEntries * sizeof *mGOTCoffEntries);
    if (mGOTCoffEntries == NULL) {
      Error (NULL, 0, 4001, "Resource", "memory cannot be allocated!");
    }
    assert (mGOTCoffEntries != NULL);
    mGOTMaxCoffEntries += mGOTMaxCoffEntries;
  }
  mGOTCoffEntries[mGOTNumCoffEntries++] = GOTCoffEntry;
  return TRUE;
}

//
// 32-bit Unsigned integer comparator for qsort.
//
STATIC
int
UINT32Comparator (
  const void* lhs,
  const void* rhs
  )
{
  if (*(const UINT32*)lhs < *(const UINT32*)rhs) {
    return -1;
  }
  return *(const UINT32*)lhs > *(const UINT32*)rhs;
}

//
// Emit accumulated Coff GOT entry relocations into
//   Coff image.  This function performs its job
//   once and then releases the entry list, so
//   it can safely be called multiple times.
//
STATIC
VOID
EmitGOTRelocations (
  VOID
  )
{
  UINT32 i;
  if (mGOTCoffEntries == NULL) {
    return;
  }
  //
  // Emit Coff relocations with Rvas ordered.
  //
  qsort(
    mGOTCoffEntries,
    mGOTNumCoffEntries,
    sizeof *mGOTCoffEntries,
    UINT32Comparator);
  for (i = 0; i < mGOTNumCoffEntries; i++) {
    VerboseMsg ("EFI_IMAGE_REL_BASED_DIR64 Offset: 0x%08X", mGOTCoffEntries[i]);
    CoffAddFixup(
      mGOTCoffEntries[i],
      EFI_IMAGE_REL_BASED_DIR64);
  }
  free(mGOTCoffEntries);
  mGOTCoffEntries = NULL;
  mGOTMaxCoffEntries = 0;
  mGOTNumCoffEntries = 0;
}

//
// Elf functions interface implementation
//

STATIC
VOID
ScanSections64 (
  VOID
  )
{
  UINT32                          i;
  EFI_IMAGE_DOS_HEADER            *DosHdr;
  EFI_IMAGE_OPTIONAL_HEADER_UNION *NtHdr;
  UINT32                          CoffEntry;
  UINT32                          SectionCount;
  BOOLEAN                         FoundSection;

  CoffEntry = 0;
  mCoffOffset = 0;

  //
  // Coff file start with a DOS header.
  //
  mCoffOffset = sizeof(EFI_IMAGE_DOS_HEADER) + 0x40;
  mNtHdrOffset = mCoffOffset;
  switch (mEhdr->e_machine) {
  case EM_X86_64:
  case EM_AARCH64:
    mCoffOffset += sizeof (EFI_IMAGE_NT_HEADERS64);
  break;
  default:
    VerboseMsg ("%s unknown e_machine type %hu. Assume X64", mInImageName, mEhdr->e_machine);
    mCoffOffset += sizeof (EFI_IMAGE_NT_HEADERS64);
  break;
  }

  mTableOffset = mCoffOffset;
  mCoffOffset += mCoffNbrSections * sizeof(EFI_IMAGE_SECTION_HEADER);

  //
  // Set mCoffAlignment to the maximum alignment of the input sections
  // we care about
  //
  for (i = 0; i < mEhdr->e_shnum; i++) {
    Elf_Shdr *shdr = GetShdrByIndex(i);
    if (shdr->sh_addralign <= mCoffAlignment) {
      continue;
    }
    if (IsTextShdr(shdr) || IsDataShdr(shdr) || IsHiiRsrcShdr(shdr)) {
      mCoffAlignment = (UINT32)shdr->sh_addralign;
    }
  }

  //
  // Check if mCoffAlignment is larger than MAX_COFF_ALIGNMENT
  //
  if (mCoffAlignment > MAX_COFF_ALIGNMENT) {
    Error (NULL, 0, 3000, "Invalid", "Section alignment is larger than MAX_COFF_ALIGNMENT.");
    assert (FALSE);
  }


  //
  // Move the PE/COFF header right before the first section. This will help us
  // save space when converting to TE.
  //
  if (mCoffAlignment > mCoffOffset) {
    mNtHdrOffset += mCoffAlignment - mCoffOffset;
    mTableOffset += mCoffAlignment - mCoffOffset;
    mCoffOffset = mCoffAlignment;
  }

  //
  // First text sections.
  //
  mCoffOffset = CoffAlign(mCoffOffset);
  mTextOffset = mCoffOffset;
  FoundSection = FALSE;
  SectionCount = 0;
  for (i = 0; i < mEhdr->e_shnum; i++) {
    Elf_Shdr *shdr = GetShdrByIndex(i);
    Elf_Shdr *Namedr = GetShdrByIndex(mEhdr->e_shstrndx);
    CHAR8* sectName = ((CHAR8*)mEhdr) + Namedr->sh_offset + shdr->sh_name;
    if (IsTextShdr(shdr)) {
      if ((shdr->sh_addralign != 0) && (shdr->sh_addralign != 1)) {
        // the alignment field is valid
        if ((shdr->sh_addr & (shdr->sh_addralign - 1)) == 0) {
          // if the section address is aligned we must align PE/COFF
          UINT32 mCoffOffsetNew = (UINT32) ((shdr->sh_addr + shdr->sh_addralign - 1) & ~(shdr->sh_addralign - 1));
          mCoffOffset = (UINT32) ((mCoffOffset + shdr->sh_addralign - 1) & ~(shdr->sh_addralign - 1));
printf("Section %d %s mCoffOffset=%d(0x%x) mCoffOffsetNew=%d(0x%x) diff=%d(0x%x)\n", i, sectName, mCoffOffset, mCoffOffset, mCoffOffsetNew, mCoffOffsetNew, mCoffOffsetNew-mCoffOffset, mCoffOffsetNew-mCoffOffset);
mCoffOffset=mCoffOffsetNew;
        } else {
          Error (NULL, 0, 3000, "Invalid", "Section address not aligned to its own alignment.");
        }
      }

      /* Relocate entry.  */
      if ((mEhdr->e_entry >= shdr->sh_addr) &&
          (mEhdr->e_entry < shdr->sh_addr + shdr->sh_size)) {
        CoffEntry = (UINT32) (mCoffOffset + mEhdr->e_entry - shdr->sh_addr);
      }

      //
      // Set mTextOffset with the offset of the first '.text' section
      //
      if (!FoundSection) {
        mTextOffset = mCoffOffset;
        FoundSection = TRUE;
      }

      mCoffSectionsOffset[i] = mCoffOffset;
      mCoffOffset += (UINT32) shdr->sh_size;
      SectionCount ++;
    }
  }

  if (!FoundSection) {
    Error (NULL, 0, 3000, "Invalid", "Did not find any '.text' section.");
    assert (FALSE);
  }

  mDebugOffset = DebugRvaAlign(mCoffOffset);
  mCoffOffset = CoffAlign(mCoffOffset);

  if (SectionCount > 1 && mOutImageType == FW_EFI_IMAGE) {
    Warning (NULL, 0, 0, NULL, "Multiple sections in %s are merged into 1 text section. Source level debug might not work correctly.", mInImageName);
  }

  //
  //  Then data sections.
  //
  mDataOffset = mCoffOffset;
  FoundSection = FALSE;
  SectionCount = 0;
  for (i = 0; i < mEhdr->e_shnum; i++) {
    Elf_Shdr *shdr = GetShdrByIndex(i);
    Elf_Shdr *Namedr = GetShdrByIndex(mEhdr->e_shstrndx);
    CHAR8* sectName = ((CHAR8*)mEhdr) + Namedr->sh_offset + shdr->sh_name;
    if (IsDataShdr(shdr)) {
      if ((shdr->sh_addralign != 0) && (shdr->sh_addralign != 1)) {
        // the alignment field is valid
        if ((shdr->sh_addr & (shdr->sh_addralign - 1)) == 0) {
          // if the section address is aligned we must align PE/COFF
          UINT32 mCoffOffsetNew = (UINT32) ((shdr->sh_addr + shdr->sh_addralign - 1) & ~(shdr->sh_addralign - 1));
          mCoffOffset = (UINT32) ((mCoffOffset + shdr->sh_addralign - 1) & ~(shdr->sh_addralign - 1));
printf("Section %d %s mCoffOffset=%d(0x%x) mCoffOffsetNew=%d(0x%x) diff=%d(0x%x)\n", i, sectName, mCoffOffset, mCoffOffset, mCoffOffsetNew, mCoffOffsetNew, mCoffOffsetNew-mCoffOffset, mCoffOffsetNew-mCoffOffset);
mCoffOffset=mCoffOffsetNew;
        } else {
          Error (NULL, 0, 3000, "Invalid", "Section address not aligned to its own alignment.");
        }
      }

      //
      // Set mDataOffset with the offset of the first '.data' section
      //
      if (!FoundSection) {
        mDataOffset = mCoffOffset;
        FoundSection = TRUE;
      }
      mCoffSectionsOffset[i] = mCoffOffset;
      mCoffOffset += (UINT32) shdr->sh_size;
      SectionCount ++;
    }
  }

  //
  // Make room for .debug data in .data (or .text if .data is empty) instead of
  // putting it in a section of its own. This is explicitly allowed by the
  // PE/COFF spec, and prevents bloat in the binary when using large values for
  // section alignment.
  //
  if (SectionCount > 0) {
    mDebugOffset = DebugRvaAlign(mCoffOffset);
  }
  mCoffOffset = mDebugOffset + sizeof(EFI_IMAGE_DEBUG_DIRECTORY_ENTRY) +
                sizeof(EFI_IMAGE_DEBUG_CODEVIEW_NB10_ENTRY) +
                strlen(mInImageName) + 1;

  mCoffOffset = CoffAlign(mCoffOffset);
  if (SectionCount == 0) {
    mDataOffset = mCoffOffset;
  }

  if (SectionCount > 1 && mOutImageType == FW_EFI_IMAGE) {
    Warning (NULL, 0, 0, NULL, "Multiple sections in %s are merged into 1 data section. Source level debug might not work correctly.", mInImageName);
  }

  //
  //  The HII resource sections.
  //
  mHiiRsrcOffset = mCoffOffset;
  for (i = 0; i < mEhdr->e_shnum; i++) {
    Elf_Shdr *shdr = GetShdrByIndex(i);
    if (IsHiiRsrcShdr(shdr)) {
      if ((shdr->sh_addralign != 0) && (shdr->sh_addralign != 1)) {
        // the alignment field is valid
        if ((shdr->sh_addr & (shdr->sh_addralign - 1)) == 0) {
          // if the section address is aligned we must align PE/COFF
          mCoffOffset = (UINT32) ((mCoffOffset + shdr->sh_addralign - 1) & ~(shdr->sh_addralign - 1));
        } else {
          Error (NULL, 0, 3000, "Invalid", "Section address not aligned to its own alignment.");
        }
      }
      if (shdr->sh_size != 0) {
        mHiiRsrcOffset = mCoffOffset;
        mCoffSectionsOffset[i] = mCoffOffset;
        mCoffOffset += (UINT32) shdr->sh_size;
        mCoffOffset = CoffAlign(mCoffOffset);
        SetHiiResourceHeader ((UINT8*) mEhdr + shdr->sh_offset, mHiiRsrcOffset);
      }
      break;
    }
  }

  mRelocOffset = mCoffOffset;

  //
  // Allocate base Coff file.  Will be expanded later for relocations.
  //
  mCoffFile = (UINT8 *)malloc(mCoffOffset);
  if (mCoffFile == NULL) {
    Error (NULL, 0, 4001, "Resource", "memory cannot be allocated!");
  }
  assert (mCoffFile != NULL);
  memset(mCoffFile, 0, mCoffOffset);

  //
  // Fill headers.
  //
  DosHdr = (EFI_IMAGE_DOS_HEADER *)mCoffFile;
  DosHdr->e_magic = EFI_IMAGE_DOS_SIGNATURE;
  DosHdr->e_lfanew = mNtHdrOffset;

  NtHdr = (EFI_IMAGE_OPTIONAL_HEADER_UNION*)(mCoffFile + mNtHdrOffset);

  NtHdr->Pe32Plus.Signature = EFI_IMAGE_NT_SIGNATURE;

  switch (mEhdr->e_machine) {
  case EM_X86_64:
    NtHdr->Pe32Plus.FileHeader.Machine = EFI_IMAGE_MACHINE_X64;
    NtHdr->Pe32Plus.OptionalHeader.Magic = EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC;
    break;
  case EM_AARCH64:
    NtHdr->Pe32Plus.FileHeader.Machine = EFI_IMAGE_MACHINE_AARCH64;
    NtHdr->Pe32Plus.OptionalHeader.Magic = EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC;
    break;
  default:
    VerboseMsg ("%s unknown e_machine type. Assume X64", (UINTN)mEhdr->e_machine);
    NtHdr->Pe32Plus.FileHeader.Machine = EFI_IMAGE_MACHINE_X64;
    NtHdr->Pe32Plus.OptionalHeader.Magic = EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC;
  }

  NtHdr->Pe32Plus.FileHeader.NumberOfSections = mCoffNbrSections;
  NtHdr->Pe32Plus.FileHeader.TimeDateStamp = (UINT32) time(NULL);
  mImageTimeStamp = NtHdr->Pe32Plus.FileHeader.TimeDateStamp;
  NtHdr->Pe32Plus.FileHeader.PointerToSymbolTable = 0;
  NtHdr->Pe32Plus.FileHeader.NumberOfSymbols = 0;
  NtHdr->Pe32Plus.FileHeader.SizeOfOptionalHeader = sizeof(NtHdr->Pe32Plus.OptionalHeader);
  NtHdr->Pe32Plus.FileHeader.Characteristics = EFI_IMAGE_FILE_EXECUTABLE_IMAGE
    | EFI_IMAGE_FILE_LINE_NUMS_STRIPPED
    | EFI_IMAGE_FILE_LOCAL_SYMS_STRIPPED
    | EFI_IMAGE_FILE_LARGE_ADDRESS_AWARE;

  NtHdr->Pe32Plus.OptionalHeader.SizeOfCode = mDataOffset - mTextOffset;
  NtHdr->Pe32Plus.OptionalHeader.SizeOfInitializedData = mRelocOffset - mDataOffset;
  NtHdr->Pe32Plus.OptionalHeader.SizeOfUninitializedData = 0;
  NtHdr->Pe32Plus.OptionalHeader.AddressOfEntryPoint = CoffEntry;

  NtHdr->Pe32Plus.OptionalHeader.BaseOfCode = mTextOffset;

  NtHdr->Pe32Plus.OptionalHeader.ImageBase = 0;
  NtHdr->Pe32Plus.OptionalHeader.SectionAlignment = mCoffAlignment;
  NtHdr->Pe32Plus.OptionalHeader.FileAlignment = mCoffAlignment;
  NtHdr->Pe32Plus.OptionalHeader.SizeOfImage = 0;

  NtHdr->Pe32Plus.OptionalHeader.SizeOfHeaders = mTextOffset;
  NtHdr->Pe32Plus.OptionalHeader.NumberOfRvaAndSizes = EFI_IMAGE_NUMBER_OF_DIRECTORY_ENTRIES;

  //
  // Section headers.
  //
  if ((mDataOffset - mTextOffset) > 0) {
    CreateSectionHeader (".text", mTextOffset, mDataOffset - mTextOffset,
            EFI_IMAGE_SCN_CNT_CODE
            | EFI_IMAGE_SCN_MEM_EXECUTE
            | EFI_IMAGE_SCN_MEM_READ);
  } else {
    // Don't make a section of size 0.
    NtHdr->Pe32Plus.FileHeader.NumberOfSections--;
  }

  if ((mHiiRsrcOffset - mDataOffset) > 0) {
    CreateSectionHeader (".data", mDataOffset, mHiiRsrcOffset - mDataOffset,
            EFI_IMAGE_SCN_CNT_INITIALIZED_DATA
            | EFI_IMAGE_SCN_MEM_WRITE
            | EFI_IMAGE_SCN_MEM_READ);
  } else {
    // Don't make a section of size 0.
    NtHdr->Pe32Plus.FileHeader.NumberOfSections--;
  }

  if ((mRelocOffset - mHiiRsrcOffset) > 0) {
    CreateSectionHeader (".rsrc", mHiiRsrcOffset, mRelocOffset - mHiiRsrcOffset,
            EFI_IMAGE_SCN_CNT_INITIALIZED_DATA
            | EFI_IMAGE_SCN_MEM_READ);

    NtHdr->Pe32Plus.OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_RESOURCE].Size = mRelocOffset - mHiiRsrcOffset;
    NtHdr->Pe32Plus.OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress = mHiiRsrcOffset;
  } else {
    // Don't make a section of size 0.
    NtHdr->Pe32Plus.FileHeader.NumberOfSections--;
  }

}

STATIC
BOOLEAN
WriteSections64 (
  SECTION_FILTER_TYPES  FilterType
  )
{
  UINT32      Idx;
  Elf_Shdr    *SecShdr;
  UINT32      SecOffset;
  BOOLEAN     (*Filter)(Elf_Shdr *);
  Elf64_Addr  GOTEntryRva;

  //
  // Initialize filter pointer
  //
  switch (FilterType) {
    case SECTION_TEXT:
      Filter = IsTextShdr;
      break;
    case SECTION_HII:
      Filter = IsHiiRsrcShdr;
      break;
    case SECTION_DATA:
      Filter = IsDataShdr;
      break;
    default:
      return FALSE;
  }

  //
  // First: copy sections.
  //
  for (Idx = 0; Idx < mEhdr->e_shnum; Idx++) {
    Elf_Shdr *Shdr = GetShdrByIndex(Idx);
    if ((*Filter)(Shdr)) {
      switch (Shdr->sh_type) {
      case SHT_PROGBITS:
        /* Copy.  */
        if (Shdr->sh_offset + Shdr->sh_size > mFileBufferSize) {
          return FALSE;
        }
        memcpy(mCoffFile + mCoffSectionsOffset[Idx],
              (UINT8*)mEhdr + Shdr->sh_offset,
              (size_t) Shdr->sh_size);
        break;

      case SHT_NOBITS:
        memset(mCoffFile + mCoffSectionsOffset[Idx], 0, (size_t) Shdr->sh_size);
        break;

      default:
        //
        //  Ignore for unknown section type.
        //
        VerboseMsg ("%s unknown section type %x. We ignore this unknown section type.", mInImageName, (unsigned)Shdr->sh_type);
        break;
      }
    }
  }

  //
  // Second: apply relocations.
  //
  VerboseMsg ("Applying Relocations...");
  for (Idx = 0; Idx < mEhdr->e_shnum; Idx++) {
    //
    // Determine if this is a relocation section.
    //
    Elf_Shdr *RelShdr = GetShdrByIndex(Idx);
    if ((RelShdr->sh_type != SHT_REL) && (RelShdr->sh_type != SHT_RELA)) {
      continue;
    }

    //
    // If this is a ET_DYN (PIE) executable, we will encounter a dynamic SHT_RELA
    // section that applies to the entire binary, and which will have its section
    // index set to #0 (which is a NULL section with the SHF_ALLOC bit cleared).
    //
    // In the absence of GOT based relocations,
    // this RELA section will contain redundant R_xxx_RELATIVE relocations, one
    // for every R_xxx_xx64 relocation appearing in the per-section RELA sections.
    // (i.e., .rela.text and .rela.data)
    //
    if (RelShdr->sh_info == 0) {
      continue;
    }

    //
    // Relocation section found.  Now extract section information that the relocations
    // apply to in the ELF data and the new COFF data.
    //
    SecShdr = GetShdrByIndex(RelShdr->sh_info);
    SecOffset = mCoffSectionsOffset[RelShdr->sh_info];

    //
    // Only process relocations for the current filter type.
    //
    if (RelShdr->sh_type == SHT_RELA && (*Filter)(SecShdr)) {
      UINT64 RelIdx;

      //
      // Determine the symbol table referenced by the relocation data.
      //
      Elf_Shdr *SymtabShdr = GetShdrByIndex(RelShdr->sh_link);
      UINT8 *Symtab = (UINT8*)mEhdr + SymtabShdr->sh_offset;

      //
      // Process all relocation entries for this section.
      //
      for (RelIdx = 0; RelIdx < RelShdr->sh_size; RelIdx += (UINT32) RelShdr->sh_entsize) {

        //
        // Set pointer to relocation entry
        //
        Elf_Rela *Rel = (Elf_Rela *)((UINT8*)mEhdr + RelShdr->sh_offset + RelIdx);

        //
        // Set pointer to symbol table entry associated with the relocation entry.
        //
        Elf_Sym  *Sym = (Elf_Sym *)(Symtab + ELF_R_SYM(Rel->r_info) * SymtabShdr->sh_entsize);

        Elf_Shdr *SymShdr;
        UINT8    *Targ;

        //
        // Check section header index found in symbol table and get the section
        // header location.
        //
        if (Sym->st_shndx == SHN_UNDEF
            || Sym->st_shndx >= mEhdr->e_shnum) {
          const UINT8 *SymName = GetSymName(Sym);
          if (SymName == NULL) {
            SymName = (const UINT8 *)"<unknown>";
          }

          Error (NULL, 0, 3000, "Invalid",
                 "%s: Bad definition for symbol '%s'@%#llx or unsupported symbol type.  "
                 "For example, absolute and undefined symbols are not supported.",
                 mInImageName, SymName, Sym->st_value);

          exit(EXIT_FAILURE);
        }
        SymShdr = GetShdrByIndex(Sym->st_shndx);

        //
        // Convert the relocation data to a pointer into the coff file.
        //
        // Note:
        //   r_offset is the virtual address of the storage unit to be relocated.
        //   sh_addr is the virtual address for the base of the section.
        //
        //   r_offset in a memory address.
        //   Convert it to a pointer in the coff file.
        //
        Targ = mCoffFile + SecOffset + (Rel->r_offset - SecShdr->sh_addr);

        //
        // Determine how to handle each relocation type based on the machine type.
        //
        if (mEhdr->e_machine == EM_X86_64) {
          switch (ELF_R_TYPE(Rel->r_info)) {
          case R_X86_64_NONE:
            break;
          case R_X86_64_64:
            //
            // Absolute relocation.
            //
            VerboseMsg ("R_X86_64_64");
            VerboseMsg ("Offset: 0x%08X, Addend: 0x%016LX",
              (UINT32)(SecOffset + (Rel->r_offset - SecShdr->sh_addr)),
              *(UINT64 *)Targ);
            *(UINT64 *)Targ = *(UINT64 *)Targ - SymShdr->sh_addr + mCoffSectionsOffset[Sym->st_shndx];
            VerboseMsg ("Relocation:  0x%016LX", *(UINT64*)Targ);
            break;
          case R_X86_64_32:
            VerboseMsg ("R_X86_64_32");
            VerboseMsg ("Offset: 0x%08X, Addend: 0x%08X",
              (UINT32)(SecOffset + (Rel->r_offset - SecShdr->sh_addr)),
              *(UINT32 *)Targ);
            *(UINT32 *)Targ = (UINT32)((UINT64)(*(UINT32 *)Targ) - SymShdr->sh_addr + mCoffSectionsOffset[Sym->st_shndx]);
            VerboseMsg ("Relocation:  0x%08X", *(UINT32*)Targ);
            break;
          case R_X86_64_32S:
            VerboseMsg ("R_X86_64_32S");
            VerboseMsg ("Offset: 0x%08X, Addend: 0x%08X",
              (UINT32)(SecOffset + (Rel->r_offset - SecShdr->sh_addr)),
              *(UINT32 *)Targ);
            *(INT32 *)Targ = (INT32)((INT64)(*(INT32 *)Targ) - SymShdr->sh_addr + mCoffSectionsOffset[Sym->st_shndx]);
            VerboseMsg ("Relocation:  0x%08X", *(UINT32*)Targ);
            break;

          case R_X86_64_PLT32:
            //
            // Treat R_X86_64_PLT32 relocations as R_X86_64_PC32: this is
            // possible since we know all code symbol references resolve to
            // definitions in the same module (UEFI has no shared libraries),
            // and so there is never a reason to jump via a PLT entry,
            // allowing us to resolve the reference using the symbol directly.
            //
            VerboseMsg ("Treating R_X86_64_PLT32 as R_X86_64_PC32 ...");
            /* fall through */
          case R_X86_64_PC32:
            //
            // Relative relocation: Symbol - Ip + Addend
            //
            VerboseMsg ("R_X86_64_PC32");
            VerboseMsg ("Offset: 0x%08X, Addend: 0x%08X",
              (UINT32)(SecOffset + (Rel->r_offset - SecShdr->sh_addr)),
              *(UINT32 *)Targ);
            *(UINT32 *)Targ = (UINT32) (*(UINT32 *)Targ
              + (mCoffSectionsOffset[Sym->st_shndx] - SymShdr->sh_addr)
              - (SecOffset - SecShdr->sh_addr));
            VerboseMsg ("Relocation:  0x%08X", *(UINT32 *)Targ);
            break;
          case R_X86_64_GOTPCREL:
          case R_X86_64_GOTPCRELX:
          case R_X86_64_REX_GOTPCRELX:
            VerboseMsg ("R_X86_64_GOTPCREL family");
            VerboseMsg ("Offset: 0x%08X, Addend: 0x%08X",
              (UINT32)(SecOffset + (Rel->r_offset - SecShdr->sh_addr)),
              *(UINT32 *)Targ);
            GOTEntryRva = Rel->r_offset - Rel->r_addend + *(INT32 *)Targ;
            FindElfGOTSectionFromGOTEntryElfRva(GOTEntryRva);
            *(UINT32 *)Targ = (UINT32) (*(UINT32 *)Targ
              + (mCoffSectionsOffset[mGOTShindex] - mGOTShdr->sh_addr)
              - (SecOffset - SecShdr->sh_addr));
            VerboseMsg ("Relocation:  0x%08X", *(UINT32 *)Targ);
            GOTEntryRva += (mCoffSectionsOffset[mGOTShindex] - mGOTShdr->sh_addr);  // ELF Rva -> COFF Rva
            if (AccumulateCoffGOTEntries((UINT32)GOTEntryRva)) {
              //
              // Relocate GOT entry if it's the first time we run into it
              //
              Targ = mCoffFile + GOTEntryRva;
              //
              // Limitation: The following three statements assume memory
              //   at *Targ is valid because the section containing the GOT
              //   has already been copied from the ELF image to the Coff image.
              //   This pre-condition presently holds because the GOT is placed
              //   in section .text, and the ELF text sections are all copied
              //   prior to reaching this point.
              //   If the pre-condition is violated in the future, this fixup
              //   either needs to be deferred after the GOT section is copied
              //   to the Coff image, or the fixup should be performed on the
              //   source Elf image instead of the destination Coff image.
              //
              VerboseMsg ("Offset: 0x%08X, Addend: 0x%016LX",
                (UINT32)GOTEntryRva,
                *(UINT64 *)Targ);
              *(UINT64 *)Targ = *(UINT64 *)Targ - SymShdr->sh_addr + mCoffSectionsOffset[Sym->st_shndx];
              VerboseMsg ("Relocation:  0x%016LX", *(UINT64*)Targ);
            }
            break;
          default:
            Error (NULL, 0, 3000, "Invalid", "%s unsupported ELF EM_X86_64 relocation 0x%x.", mInImageName, (unsigned) ELF_R_TYPE(Rel->r_info));
          }
        } else if (mEhdr->e_machine == EM_AARCH64) {

          switch (ELF_R_TYPE(Rel->r_info)) {
            INT64 Offset;

          case R_AARCH64_LD64_GOT_LO12_NC:
            //
            // Convert into an ADD instruction - see R_AARCH64_ADR_GOT_PAGE below.
            //
            *(UINT32 *)Targ &= 0x3ff;
            *(UINT32 *)Targ |= 0x91000000 | ((Sym->st_value & 0xfff) << 10);
            break;

          case R_AARCH64_ADR_GOT_PAGE:
            //
            // This relocation points to the GOT entry that contains the absolute
            // address of the symbol we are referring to. Since EDK2 only uses
            // fully linked binaries, we can avoid the indirection, and simply
            // refer to the symbol directly. This implies having to patch the
            // subsequent LDR instruction (covered by a R_AARCH64_LD64_GOT_LO12_NC
            // relocation) into an ADD instruction - this is handled above.
            //
            Offset = (Sym->st_value - (Rel->r_offset & ~0xfff)) >> 12;

            *(UINT32 *)Targ &= 0x9000001f;
            *(UINT32 *)Targ |= ((Offset & 0x1ffffc) << (5 - 2)) | ((Offset & 0x3) << 29);

            /* fall through */

          case R_AARCH64_ADR_PREL_PG_HI21:
            //
            // AArch64 PG_H21 relocations are typically paired with ABS_LO12
            // relocations, where a PC-relative reference with +/- 4 GB range is
            // split into a relative high part and an absolute low part. Since
            // the absolute low part represents the offset into a 4 KB page, we
            // either have to convert the ADRP into an ADR instruction, or we
            // need to use a section alignment of at least 4 KB, so that the
            // binary appears at a correct offset at runtime. In any case, we
            // have to make sure that the 4 KB relative offsets of both the
            // section containing the reference as well as the section to which
            // it refers have not been changed during PE/COFF conversion (i.e.,
            // in ScanSections64() above).
            //
            if (mCoffAlignment < 0x1000) {
              //
              // Attempt to convert the ADRP into an ADR instruction.
              // This is only possible if the symbol is within +/- 1 MB.
              //

              // Decode the ADRP instruction
              Offset = (INT32)((*(UINT32 *)Targ & 0xffffe0) << 8);
              Offset = (Offset << (6 - 5)) | ((*(UINT32 *)Targ & 0x60000000) >> (29 - 12));

              //
              // ADRP offset is relative to the previous page boundary,
              // whereas ADR offset is relative to the instruction itself.
              // So fix up the offset so it points to the page containing
              // the symbol.
              //
              Offset -= (UINTN)(Targ - mCoffFile) & 0xfff;

              if (Offset < -0x100000 || Offset > 0xfffff) {
                Error (NULL, 0, 3000, "Invalid", "WriteSections64(): %s  due to its size (> 1 MB), this module requires 4 KB section alignment.",
                  mInImageName);
                break;
              }

              // Re-encode the offset as an ADR instruction
              *(UINT32 *)Targ &= 0x1000001f;
              *(UINT32 *)Targ |= ((Offset & 0x1ffffc) << (5 - 2)) | ((Offset & 0x3) << 29);
            }
            /* fall through */

          case R_AARCH64_ADD_ABS_LO12_NC:
          case R_AARCH64_LDST8_ABS_LO12_NC:
          case R_AARCH64_LDST16_ABS_LO12_NC:
          case R_AARCH64_LDST32_ABS_LO12_NC:
          case R_AARCH64_LDST64_ABS_LO12_NC:
          case R_AARCH64_LDST128_ABS_LO12_NC:
            if (((SecShdr->sh_addr ^ SecOffset) & 0xfff) != 0 ||
                ((SymShdr->sh_addr ^ mCoffSectionsOffset[Sym->st_shndx]) & 0xfff) != 0) {
              Error (NULL, 0, 3000, "Invalid", "WriteSections64(): %s AARCH64 small code model requires identical ELF and PE/COFF section offsets modulo 4 KB.",
                mInImageName);
              break;
            }
            /* fall through */

          case R_AARCH64_ADR_PREL_LO21:
          case R_AARCH64_CONDBR19:
          case R_AARCH64_LD_PREL_LO19:
          case R_AARCH64_CALL26:
          case R_AARCH64_JUMP26:
          case R_AARCH64_PREL64:
          case R_AARCH64_PREL32:
          case R_AARCH64_PREL16:
            //
            // The GCC toolchains (i.e., binutils) may corrupt section relative
            // relocations when emitting relocation sections into fully linked
            // binaries. More specifically, they tend to fail to take into
            // account the fact that a '.rodata + XXX' relocation needs to have
            // its addend recalculated once .rodata is merged into the .text
            // section, and the relocation emitted into the .rela.text section.
            //
            // We cannot really recover from this loss of information, so the
            // only workaround is to prevent having to recalculate any relative
            // relocations at all, by using a linker script that ensures that
            // the offset between the Place and the Symbol is the same in both
            // the ELF and the PE/COFF versions of the binary.
            //
            if ((SymShdr->sh_addr - SecShdr->sh_addr) !=
                (mCoffSectionsOffset[Sym->st_shndx] - SecOffset)) {
              Error (NULL, 0, 3000, "Invalid", "WriteSections64(): %s AARCH64 relative relocations require identical ELF and PE/COFF section offsets",
                mInImageName);
            }
            break;

          // Absolute relocations.
          case R_AARCH64_ABS64:
            *(UINT64 *)Targ = *(UINT64 *)Targ - SymShdr->sh_addr + mCoffSectionsOffset[Sym->st_shndx];
            break;

          default:
            Error (NULL, 0, 3000, "Invalid", "WriteSections64(): %s unsupported ELF EM_AARCH64 relocation 0x%x.", mInImageName, (unsigned) ELF_R_TYPE(Rel->r_info));
          }
        } else {
          Error (NULL, 0, 3000, "Invalid", "Not a supported machine type");
        }
      }
    }
  }

  return TRUE;
}

STATIC
VOID
WriteRelocations64 (
  VOID
  )
{
  UINT32                           Index;
  EFI_IMAGE_OPTIONAL_HEADER_UNION  *NtHdr;
  EFI_IMAGE_DATA_DIRECTORY         *Dir;

  for (Index = 0; Index < mEhdr->e_shnum; Index++) {
    Elf_Shdr *RelShdr = GetShdrByIndex(Index);
    if ((RelShdr->sh_type == SHT_REL) || (RelShdr->sh_type == SHT_RELA)) {
      Elf_Shdr *SecShdr = GetShdrByIndex (RelShdr->sh_info);
      if (IsTextShdr(SecShdr) || IsDataShdr(SecShdr)) {
        UINT64 RelIdx;

        for (RelIdx = 0; RelIdx < RelShdr->sh_size; RelIdx += RelShdr->sh_entsize) {
          Elf_Rela *Rel = (Elf_Rela *)((UINT8*)mEhdr + RelShdr->sh_offset + RelIdx);

          if (mEhdr->e_machine == EM_X86_64) {
            switch (ELF_R_TYPE(Rel->r_info)) {
            case R_X86_64_NONE:
            case R_X86_64_PC32:
            case R_X86_64_PLT32:
            case R_X86_64_GOTPCREL:
            case R_X86_64_GOTPCRELX:
            case R_X86_64_REX_GOTPCRELX:
              break;
            case R_X86_64_64:
              VerboseMsg ("EFI_IMAGE_REL_BASED_DIR64 Offset: 0x%08X",
                mCoffSectionsOffset[RelShdr->sh_info] + (Rel->r_offset - SecShdr->sh_addr));
              CoffAddFixup(
                (UINT32) ((UINT64) mCoffSectionsOffset[RelShdr->sh_info]
                + (Rel->r_offset - SecShdr->sh_addr)),
                EFI_IMAGE_REL_BASED_DIR64);
              break;
            //
            // R_X86_64_32 and R_X86_64_32S are ELF64 relocations emitted when using
            //   the SYSV X64 ABI small non-position-independent code model.
            //   R_X86_64_32 is used for unsigned 32-bit immediates with a 32-bit operand
            //   size.  The value is either not extended, or zero-extended to 64 bits.
            //   R_X86_64_32S is used for either signed 32-bit non-rip-relative displacements
            //   or signed 32-bit immediates with a 64-bit operand size.  The value is
            //   sign-extended to 64 bits.
            //   EFI_IMAGE_REL_BASED_HIGHLOW is a PE relocation that uses 32-bit arithmetic
            //   for rebasing an image.
            //   EFI PE binaries declare themselves EFI_IMAGE_FILE_LARGE_ADDRESS_AWARE and
            //   may load above 2GB.  If an EFI PE binary with a converted R_X86_64_32S
            //   relocation is loaded above 2GB, the value will get sign-extended to the
            //   negative part of the 64-bit address space.  The negative part of the 64-bit
            //   address space is unmapped, so accessing such an address page-faults.
            //   In order to support R_X86_64_32S, it is necessary to unset
            //   EFI_IMAGE_FILE_LARGE_ADDRESS_AWARE, and the EFI PE loader must implement
            //   this flag and abstain from loading such a PE binary above 2GB.
            //   Since this feature is not supported, support for R_X86_64_32S (and hence
            //   the small non-position-independent code model) is disabled.
            //
            // case R_X86_64_32S:
            case R_X86_64_32:
              VerboseMsg ("EFI_IMAGE_REL_BASED_HIGHLOW Offset: 0x%08X",
                mCoffSectionsOffset[RelShdr->sh_info] + (Rel->r_offset - SecShdr->sh_addr));
              CoffAddFixup(
                (UINT32) ((UINT64) mCoffSectionsOffset[RelShdr->sh_info]
                + (Rel->r_offset - SecShdr->sh_addr)),
                EFI_IMAGE_REL_BASED_HIGHLOW);
              break;
            default:
              Error (NULL, 0, 3000, "Invalid", "%s unsupported ELF EM_X86_64 relocation 0x%x.", mInImageName, (unsigned) ELF_R_TYPE(Rel->r_info));
            }
          } else if (mEhdr->e_machine == EM_AARCH64) {

            switch (ELF_R_TYPE(Rel->r_info)) {
            case R_AARCH64_ADR_PREL_LO21:
            case R_AARCH64_CONDBR19:
            case R_AARCH64_LD_PREL_LO19:
            case R_AARCH64_CALL26:
            case R_AARCH64_JUMP26:
            case R_AARCH64_PREL64:
            case R_AARCH64_PREL32:
            case R_AARCH64_PREL16:
            case R_AARCH64_ADR_PREL_PG_HI21:
            case R_AARCH64_ADD_ABS_LO12_NC:
            case R_AARCH64_LDST8_ABS_LO12_NC:
            case R_AARCH64_LDST16_ABS_LO12_NC:
            case R_AARCH64_LDST32_ABS_LO12_NC:
            case R_AARCH64_LDST64_ABS_LO12_NC:
            case R_AARCH64_LDST128_ABS_LO12_NC:
            case R_AARCH64_ADR_GOT_PAGE:
            case R_AARCH64_LD64_GOT_LO12_NC:
              //
              // No fixups are required for relative relocations, provided that
              // the relative offsets between sections have been preserved in
              // the ELF to PE/COFF conversion. We have already asserted that
              // this is the case in WriteSections64 ().
              //
              break;

            case R_AARCH64_ABS64:
              CoffAddFixup(
                (UINT32) ((UINT64) mCoffSectionsOffset[RelShdr->sh_info]
                + (Rel->r_offset - SecShdr->sh_addr)),
                EFI_IMAGE_REL_BASED_DIR64);
              break;

            case R_AARCH64_ABS32:
              CoffAddFixup(
                (UINT32) ((UINT64) mCoffSectionsOffset[RelShdr->sh_info]
                + (Rel->r_offset - SecShdr->sh_addr)),
                EFI_IMAGE_REL_BASED_HIGHLOW);
             break;

            default:
                Error (NULL, 0, 3000, "Invalid", "WriteRelocations64(): %s unsupported ELF EM_AARCH64 relocation 0x%x.", mInImageName, (unsigned) ELF_R_TYPE(Rel->r_info));
            }
          } else {
            Error (NULL, 0, 3000, "Not Supported", "This tool does not support relocations for ELF with e_machine %u (processor type).", (unsigned) mEhdr->e_machine);
          }
        }
        if (mEhdr->e_machine == EM_X86_64 && RelShdr->sh_info == mGOTShindex) {
          //
          // Tack relocations for GOT entries after other relocations for
          //   the section the GOT is in, as it's usually found at the end
          //   of the section.  This is done in order to maintain Rva order
          //   of Coff relocations.
          //
          EmitGOTRelocations();
        }
      }
    }
  }

  if (mEhdr->e_machine == EM_X86_64) {
    //
    // This is a safety net just in case the GOT is in a section
    //   with no other relocations and the first invocation of
    //   EmitGOTRelocations() above was skipped.  This invocation
    //   does not maintain Rva order of Coff relocations.
    //   At present, with a single text section, all references to
    //   the GOT and the GOT itself reside in section .text, so
    //   if there's a GOT at all, the first invocation above
    //   is executed.
    //
    EmitGOTRelocations();
  }
  //
  // Pad by adding empty entries.
  //
  while (mCoffOffset & (mCoffAlignment - 1)) {
    CoffAddFixupEntry(0);
  }

  NtHdr = (EFI_IMAGE_OPTIONAL_HEADER_UNION *)(mCoffFile + mNtHdrOffset);
  Dir = &NtHdr->Pe32Plus.OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC];
  Dir->Size = mCoffOffset - mRelocOffset;
  if (Dir->Size == 0) {
    // If no relocations, null out the directory entry and don't add the .reloc section
    Dir->VirtualAddress = 0;
    NtHdr->Pe32Plus.FileHeader.NumberOfSections--;
  } else {
    Dir->VirtualAddress = mRelocOffset;
    CreateSectionHeader (".reloc", mRelocOffset, mCoffOffset - mRelocOffset,
            EFI_IMAGE_SCN_CNT_INITIALIZED_DATA
            | EFI_IMAGE_SCN_MEM_DISCARDABLE
            | EFI_IMAGE_SCN_MEM_READ);
  }
}

STATIC
VOID
WriteDebug64 (
  VOID
  )
{
  UINT32                              Len;
  EFI_IMAGE_OPTIONAL_HEADER_UNION     *NtHdr;
  EFI_IMAGE_DATA_DIRECTORY            *DataDir;
  EFI_IMAGE_DEBUG_DIRECTORY_ENTRY     *Dir;
  EFI_IMAGE_DEBUG_CODEVIEW_NB10_ENTRY *Nb10;

  Len = strlen(mInImageName) + 1;

  Dir = (EFI_IMAGE_DEBUG_DIRECTORY_ENTRY*)(mCoffFile + mDebugOffset);
  Dir->Type = EFI_IMAGE_DEBUG_TYPE_CODEVIEW;
  Dir->SizeOfData = sizeof(EFI_IMAGE_DEBUG_CODEVIEW_NB10_ENTRY) + Len;
  Dir->RVA = mDebugOffset + sizeof(EFI_IMAGE_DEBUG_DIRECTORY_ENTRY);
  Dir->FileOffset = mDebugOffset + sizeof(EFI_IMAGE_DEBUG_DIRECTORY_ENTRY);

  Nb10 = (EFI_IMAGE_DEBUG_CODEVIEW_NB10_ENTRY*)(Dir + 1);
  Nb10->Signature = CODEVIEW_SIGNATURE_NB10;
  strcpy ((char *)(Nb10 + 1), mInImageName);


  NtHdr = (EFI_IMAGE_OPTIONAL_HEADER_UNION *)(mCoffFile + mNtHdrOffset);
  DataDir = &NtHdr->Pe32Plus.OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_DEBUG];
  DataDir->VirtualAddress = mDebugOffset;
  DataDir->Size = sizeof(EFI_IMAGE_DEBUG_DIRECTORY_ENTRY);
}

STATIC
VOID
SetImageSize64 (
  VOID
  )
{
  EFI_IMAGE_OPTIONAL_HEADER_UNION *NtHdr;

  //
  // Set image size
  //
  NtHdr = (EFI_IMAGE_OPTIONAL_HEADER_UNION *)(mCoffFile + mNtHdrOffset);
  NtHdr->Pe32Plus.OptionalHeader.SizeOfImage = mCoffOffset;
}

STATIC
VOID
CleanUp64 (
  VOID
  )
{
  if (mCoffSectionsOffset != NULL) {
    free (mCoffSectionsOffset);
  }
}


