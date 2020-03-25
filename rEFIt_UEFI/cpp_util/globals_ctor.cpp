//===-- crtbegin.c - Start of constructors and destructors ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#if 0
#define DBG(...) DebugLog(2, __VA_ARGS__)
#else
#define DBG(...)
#endif


extern "C" {
//  #include <stddef.h>
}
#include "globals_ctor.h"
#include <Platform/Platform.h>



typedef void (ctor)(void);
typedef ctor* ctor_ptr;

#if defined(__clang__)


void construct_globals_objects()
{

  UINT32 PeCoffHeaderOffset = 0;
  EFI_IMAGE_DOS_HEADER* DosHdr = (EFI_IMAGE_DOS_HEADER*)SelfLoadedImage->ImageBase;
  if (DosHdr->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
    // DOS image header is present, so read the PE header after the DOS image header
    PeCoffHeaderOffset = DosHdr->e_lfanew;
  }
  DBG("ImageContext.PeCoffHeaderOffset: %08X %d\n", PeCoffHeaderOffset, PeCoffHeaderOffset);


	EFI_IMAGE_OPTIONAL_HEADER_UNION* ImgHdr = (EFI_IMAGE_OPTIONAL_HEADER_UNION *) ((UINTN) (SelfLoadedImage->ImageBase) + PeCoffHeaderOffset);
	EFI_IMAGE_SECTION_HEADER* SectionHeader = (EFI_IMAGE_SECTION_HEADER *) ((UINTN) ImgHdr + sizeof(UINT32) + sizeof(EFI_IMAGE_FILE_HEADER) + ImgHdr->Pe32.FileHeader.SizeOfOptionalHeader);

	for (int Index = 0; Index < ImgHdr->Pe32.FileHeader.NumberOfSections; Index++, SectionHeader++)
	{
		DBG("SectionHeader->Name=%s\n", SectionHeader->Name);
//		DBG("SectionHeader->PointerToRawData=%8X\n", SectionHeader->PointerToRawData);
//		DBG("SectionHeader->SizeOfRawData=%8X\n", SectionHeader->SizeOfRawData);
		DBG("SectionHeader->VirtualSize=%8X\n", SectionHeader->Misc.VirtualSize);
		if (AsciiStrCmp((CONST CHAR8*) SectionHeader->Name, ".ctorss") == 0)
		{

			ctor_ptr* currentCtor = (ctor_ptr*) (((UINTN) (SelfLoadedImage->ImageBase)) + SectionHeader->PointerToRawData);
			ctor_ptr* ctorend = (ctor_ptr*) (((UINTN) (SelfLoadedImage->ImageBase)) + SectionHeader->PointerToRawData + SectionHeader->Misc.VirtualSize);
			while (currentCtor < ctorend)
			{
				DBG("&currentCtor %X %d\n", (UINTN) (currentCtor), (UINTN) (currentCtor));
				DBG("currentCtor %X %d\n", (UINTN) (*currentCtor), (UINTN) (*currentCtor));
				if (*currentCtor != NULL) (*currentCtor)();
				currentCtor++;
			}
		}
	}
}

#elif defined(__GNUC__)

__attribute__((visibility("hidden"))) void *__dso_handle = &__dso_handle;

extern int __beginning_of_section_ctors, __end_of_section_ctors;

ctor_ptr* p = (ctor_ptr*)&__beginning_of_section_ctors;
ctor_ptr* pend = (ctor_ptr*)&__end_of_section_ctors;


void construct_globals_objects() {
    DBG("CTOR %X %d\n", (UINTN)p, (UINTN)p);
//    DBG("CTOR %X %d\n", (UINTN)p[0], (UINTN)p[0]);
    while ( p < pend ) {
    	DBG("CTOR %X %d\n", (UINTN)p[0], (UINTN)p[0]);
    	(*p)();
    	p++;
    }
//    DBG("CTOR %X %d\n", (UINTN)__CTOR_LIST__, (UINTN)__CTOR_LIST__);
//   __do_init();
//  const size_t n = __CTOR_LIST_END__ - __CTOR_LIST__ - 1;
//  size_t n = 10;
//  for (size_t i = 0; i < n; i++) {
//    DBG("CTOR %X %d\n", (UINTN)__CTOR_LIST__[i], (UINTN)__CTOR_LIST__[i]);
//  }
//  __CTOR_LIST__[0]();
}

#elif defined(_MSC_VER)

void construct_globals_objects()
{
    DBG("Work in progress\n");
    UINT32 PeCoffHeaderOffset = 0;
    EFI_IMAGE_DOS_HEADER* DosHdr = (EFI_IMAGE_DOS_HEADER*)SelfLoadedImage->ImageBase;
    if (DosHdr->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
      // DOS image header is present, so read the PE header after the DOS image header
      PeCoffHeaderOffset = DosHdr->e_lfanew;
    }
    DBG("ImageContext.PeCoffHeaderOffset: %08X %d\n", PeCoffHeaderOffset, PeCoffHeaderOffset);


  	EFI_IMAGE_OPTIONAL_HEADER_UNION* ImgHdr = (EFI_IMAGE_OPTIONAL_HEADER_UNION *) ((UINTN) (SelfLoadedImage->ImageBase) + PeCoffHeaderOffset);
  	EFI_IMAGE_SECTION_HEADER* SectionHeader = (EFI_IMAGE_SECTION_HEADER *) ((UINTN) ImgHdr + sizeof(UINT32) + sizeof(EFI_IMAGE_FILE_HEADER) + ImgHdr->Pe32.FileHeader.SizeOfOptionalHeader);

  	for (int Index = 0; Index < ImgHdr->Pe32.FileHeader.NumberOfSections; Index++, SectionHeader++)
  	{
  		DBG("SectionHeader->Name=%s\n", SectionHeader->Name);
  //		DBG("SectionHeader->PointerToRawData=%8X\n", SectionHeader->PointerToRawData);
  //		DBG("SectionHeader->SizeOfRawData=%8X\n", SectionHeader->SizeOfRawData);
  		DBG("SectionHeader->VirtualSize=%8X\n", SectionHeader->Misc.VirtualSize);
  		if (AsciiStrCmp((CONST CHAR8*) SectionHeader->Name, ".CRT") == 0)
  		{

  			ctor_ptr* currentCtor = (ctor_ptr*) (((UINTN) (SelfLoadedImage->ImageBase)) + SectionHeader->PointerToRawData);
  			ctor_ptr* ctorend = (ctor_ptr*) (((UINTN) (SelfLoadedImage->ImageBase)) + SectionHeader->PointerToRawData + SectionHeader->Misc.VirtualSize);
  			while (currentCtor < ctorend)
  			{
  				DBG("&currentCtor %X %d\n", (UINTN) (currentCtor), (UINTN) (currentCtor));
  				DBG("currentCtor %X %d\n", (UINTN) (*currentCtor), (UINTN) (*currentCtor));
  				if (*currentCtor != NULL) (*currentCtor)();
  				currentCtor++;
  			}
  		}
  	}
}


#endif
