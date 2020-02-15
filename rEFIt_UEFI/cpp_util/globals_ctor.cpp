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
  #include <stddef.h>
}

#include <Platform/Platform.h>



typedef void (ctor)(void);
typedef ctor* ctor_ptr;

#if defined(__clang__)

extern "C" {
  /*
   * This symbol will be placed at the beginning of the section following the __mod_init_func section.
   * This way, will know the size of the __mod_init_func section.
   * In the efi file, __mod_init_func section has been merged with the following section. That's why we need this trick.
   */
  void __attribute__((section ("__DATA,__const"))) __attribute__((optnone)) beginning_of_section_next_to_mod_init_func() {};
}

void construct_globals_objects()
{

//	beginning_of_section_next_to_mod_init_func(); // to not be optimized out

	ctor_ptr* beginning_of_section_next_to_mod_init_func_ptr = (ctor_ptr*)&beginning_of_section_next_to_mod_init_func;
DBG("beginning_of_section_next_to_mod_init_func_ptr=%08x\n", (UINTN)beginning_of_section_next_to_mod_init_func_ptr);

  UINT32 PeCoffHeaderOffset = 0;
  EFI_IMAGE_DOS_HEADER* DosHdr = (EFI_IMAGE_DOS_HEADER*)SelfLoadedImage->ImageBase;
  if (DosHdr->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
    // DOS image header is present, so read the PE header after the DOS image header
    PeCoffHeaderOffset = DosHdr->e_lfanew;
  }
  DBG("ImageContext.PeCoffHeaderOffset: %08x %d\n", PeCoffHeaderOffset, PeCoffHeaderOffset);


	EFI_IMAGE_OPTIONAL_HEADER_UNION* ImgHdr = (EFI_IMAGE_OPTIONAL_HEADER_UNION *) ((UINTN) (SelfLoadedImage->ImageBase) + PeCoffHeaderOffset);
	EFI_IMAGE_SECTION_HEADER* SectionHeader = (EFI_IMAGE_SECTION_HEADER *) ((UINTN) ImgHdr + sizeof(UINT32) + sizeof(EFI_IMAGE_FILE_HEADER) + ImgHdr->Pe32.FileHeader.SizeOfOptionalHeader);

	for (int Index = 0; Index < ImgHdr->Pe32.FileHeader.NumberOfSections; Index++, SectionHeader++)
	{
		DBG("name=%a\n", SectionHeader->Name);
		if (AsciiStrCmp((CONST CHAR8*) SectionHeader->Name, ".data") == 0)
		{
			DBG("SectionHeader->PointerToRawData=%8x\n", SectionHeader->PointerToRawData);

			ctor_ptr* myctor = (ctor_ptr*) (((UINTN) (SelfLoadedImage->ImageBase)) + SectionHeader->PointerToRawData);
			while (myctor < beginning_of_section_next_to_mod_init_func_ptr)
			{
				DBG("&myctor %x %d\n", (UINTN) (myctor), (UINTN) (myctor));
				DBG("myctor %x %d\n", (UINTN) (*myctor), (UINTN) (*myctor));
				if (*myctor != NULL) (*myctor)();
				myctor++;
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
    DBG("CTOR %x %d\n", (UINTN)p, (UINTN)p);
//    DBG("CTOR %x %d\n", (UINTN)p[0], (UINTN)p[0]);
    while ( p < pend ) {
    	DBG("CTOR %x %d\n", (UINTN)p[0], (UINTN)p[0]);
    	(*p)();
    	p++;
    }
//    DBG("CTOR %x %d\n", (UINTN)__CTOR_LIST__, (UINTN)__CTOR_LIST__);
//   __do_init();
//  const size_t n = __CTOR_LIST_END__ - __CTOR_LIST__ - 1;
//  size_t n = 10;
//  for (size_t i = 0; i < n; i++) {
//    DBG("CTOR %x %d\n", (UINTN)__CTOR_LIST__[i], (UINTN)__CTOR_LIST__[i]);
//  }
//  __CTOR_LIST__[0]();
}

#elif defined(_MSC_VER)

#endif
