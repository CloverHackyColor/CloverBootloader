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
#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include <Protocol/LoadedImage.h>

//#define DUMP_EFI_FROM_MEMORY

#ifdef DUMP_EFI_FROM_MEMORY
#include "../../libeg/libeg.h"
#include "../../Settings/Self.h"
#endif

typedef void (ctor)(void);
typedef ctor* ctor_ptr;

#if defined(__clang__)


void construct_globals_objects(EFI_HANDLE ImageHandle)
{
  EFI_LOADED_IMAGE* LoadedImage;
  EFI_STATUS Status = gBS->HandleProtocol(gImageHandle, &gEfiLoadedImageProtocolGuid, (void **) &LoadedImage);
  if ( EFI_ERROR(Status) ) {
    panic("construct_globals_objects: Cannot get LoadedImage protocol");
  }

  UINT32 PeCoffHeaderOffset = 0;
  EFI_IMAGE_DOS_HEADER* DosHdr = (EFI_IMAGE_DOS_HEADER*)LoadedImage->ImageBase;
  if (DosHdr->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
    // DOS image header is present, so read the PE header after the DOS image header
    PeCoffHeaderOffset = DosHdr->e_lfanew;
  }
  DBG("ImageContext.PeCoffHeaderOffset: %08X %d\n", PeCoffHeaderOffset, PeCoffHeaderOffset);


  EFI_IMAGE_OPTIONAL_HEADER_UNION* ImgHdr = (EFI_IMAGE_OPTIONAL_HEADER_UNION *) ((UINTN) (LoadedImage->ImageBase) + PeCoffHeaderOffset);
  EFI_IMAGE_SECTION_HEADER* SectionHeader = (EFI_IMAGE_SECTION_HEADER *) ((UINTN) ImgHdr + sizeof(UINT32) + sizeof(EFI_IMAGE_FILE_HEADER) + ImgHdr->Pe32.FileHeader.SizeOfOptionalHeader);

  for (int Index = 0; Index < ImgHdr->Pe32.FileHeader.NumberOfSections; Index++, SectionHeader++)
  {
    DBG("SectionHeader->Name=%s\n", SectionHeader->Name);
//    DBG("SectionHeader->PointerToRawData=%8X\n", SectionHeader->PointerToRawData);
//    DBG("SectionHeader->SizeOfRawData=%8X\n", SectionHeader->SizeOfRawData);
    DBG("SectionHeader->VirtualSize=%8X\n", SectionHeader->Misc.VirtualSize);
    if (strcmp((CONST CHAR8*) SectionHeader->Name, ".ctorss") == 0)
    {

      ctor_ptr* currentCtor = (ctor_ptr*) (((UINTN) (LoadedImage->ImageBase)) + SectionHeader->PointerToRawData);
      ctor_ptr* ctorEnd = (ctor_ptr*) (((UINTN) (LoadedImage->ImageBase)) + SectionHeader->PointerToRawData + SectionHeader->Misc.VirtualSize);
      DBG("currentBegin %llX, ctorEnd %llX, %lld ctors to call\n", (UINTN)(currentCtor), (UINTN)(ctorEnd), (UINTN)(ctorEnd-currentCtor));
      size_t i = 0;
      while (currentCtor < ctorEnd)
      {
        DBG("[%03zu] &ctor %08llX, will call %08llX\n", i, (UINTN)(currentCtor), (UINTN)(*currentCtor));
        if (*currentCtor != NULL) (*currentCtor)();
        currentCtor++;
        i++;
      }
    }
  }
}

#elif defined(__GNUC__)

__attribute__((visibility("hidden"))) void *__dso_handle = &__dso_handle;

extern int __beginning_of_section_ctors, __end_of_section_ctors;

ctor_ptr* p = (ctor_ptr*)&__beginning_of_section_ctors;
ctor_ptr* pend = (ctor_ptr*)&__end_of_section_ctors;


void construct_globals_objects(EFI_HANDLE ImageHandle)
{
    (void)ImageHandle;

#ifdef DUMP_EFI_FROM_MEMORY
  EFI_LOADED_IMAGE* LoadedImage;
  EFI_STATUS Status = gBS->HandleProtocol(gImageHandle, &gEfiLoadedImageProtocolGuid, (void **) &LoadedImage);
  if ( EFI_ERROR(Status) ) {
    panic("construct_globals_objects: Cannot get LoadedImage protocol");
  }

  Self self;
  self.initialize(ImageHandle);
//  for (size_t i=0 ; i<2467264 ; i +=16) DBG("%08llx %08llx %08llx %08llx %08llx %08llx %08llx %08llx %08llx %08llx %08llx %08llx %08llx %08llx %08llx %08llx\n", (UINTN)p[i], (UINTN)p[i+1], (UINTN)p[i+2], (UINTN)p[i+3], (UINTN)p[i+4], (UINTN)p[i+5], (UINTN)p[i+6], (UINTN)p[i+7], (UINTN)p[i+8], (UINTN)p[i+9], (UINTN)p[i+10], (UINTN)p[i+11], (UINTN)p[i+12], (UINTN)p[i+13], (UINTN)p[i+14], (UINTN)p[i+15]);
//  unsigned char* cp = (unsigned char*)p;
//  for (size_t i=0 ; i<2467264 ; i +=16) DBG("%c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c\n",cp[0]>=32 ? cp[i+0] : '.',cp[i+1]>=32 ? cp[i+1] : '.',cp[i+2]>=32 ? cp[i+2] : '.',cp[i+3]>=32 ? cp[i+3] : '.',cp[i+4]>=32 ? cp[i+4] : '.',cp[i+5]>=32 ? cp[i+5] : '.',cp[i+6]>=32 ? cp[i+6] : '.',cp[i+7]>=32 ? cp[i+7] : '.',cp[i+8]>=32 ? cp[i+8] : '.',cp[i+9]>=32 ? cp[i+9] : '.',cp[i+10]>=32 ? cp[i+10] : '.',cp[i+11]>=32 ? cp[i+11] : '.',cp[i+12]>=32 ? cp[i+12] : '.',cp[i+13]>=32 ? cp[i+13] : '.',cp[i+14]>=32 ? cp[i+14] : '.',cp[i+15]>=32 ? cp[i+15] : '.');
//  egSaveFile(&self.getCloverDir(), L"dump.bin", LoadedImage->ImageBase, 3000000);
#endif

    DBG("CTORS %llX(%lld), offset from file beginning %llX(%lld) size %ld\n", (UINTN)p, (UINTN)p, (UINTN)p - (UINTN)LoadedImage->ImageBase, (UINTN)p - (UINTN)LoadedImage->ImageBase, pend-p );
    while ( p < pend ) {
      DBG("CTOR %llX(%lld)\n", (UINTN)p[0], (UINTN)p[0]);
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

void construct_globals_objects(EFI_HANDLE ImageHandle)
{
    DBG("Work in progress\n");

  EFI_LOADED_IMAGE* LoadedImage;
  EFI_STATUS Status = gBS->HandleProtocol(gImageHandle, &gEfiLoadedImageProtocolGuid, (void **)&LoadedImage);
  if (EFI_ERROR(Status)) {
    panic("construct_globals_objects: Cannot get LoadedImage protocol");
  }

    UINT32 PeCoffHeaderOffset = 0;
    EFI_IMAGE_DOS_HEADER* DosHdr = (EFI_IMAGE_DOS_HEADER*)LoadedImage->ImageBase;
    if (DosHdr->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
      // DOS image header is present, so read the PE header after the DOS image header
      PeCoffHeaderOffset = DosHdr->e_lfanew;
    }
    DBG("ImageContext.PeCoffHeaderOffset: %08X %d\n", PeCoffHeaderOffset, PeCoffHeaderOffset);


    EFI_IMAGE_OPTIONAL_HEADER_UNION* ImgHdr = (EFI_IMAGE_OPTIONAL_HEADER_UNION *) ((UINTN) (LoadedImage->ImageBase) + PeCoffHeaderOffset);
    EFI_IMAGE_SECTION_HEADER* SectionHeader = (EFI_IMAGE_SECTION_HEADER *) ((UINTN) ImgHdr + sizeof(UINT32) + sizeof(EFI_IMAGE_FILE_HEADER) + ImgHdr->Pe32.FileHeader.SizeOfOptionalHeader);

    for (int Index = 0; Index < ImgHdr->Pe32.FileHeader.NumberOfSections; Index++, SectionHeader++)
    {
      DBG("SectionHeader->Name=%s\n", SectionHeader->Name);
  //    DBG("SectionHeader->PointerToRawData=%8X\n", SectionHeader->PointerToRawData);
  //    DBG("SectionHeader->SizeOfRawData=%8X\n", SectionHeader->SizeOfRawData);
      DBG("SectionHeader->VirtualSize=%8X\n", SectionHeader->Misc.VirtualSize);
      if (strcmp((CONST CHAR8*) SectionHeader->Name, ".CRT") == 0)
      {

        ctor_ptr* currentCtor = (ctor_ptr*) (((UINTN) (LoadedImage->ImageBase)) + SectionHeader->PointerToRawData);
        ctor_ptr* ctorend = (ctor_ptr*) (((UINTN) (LoadedImage->ImageBase)) + SectionHeader->PointerToRawData + SectionHeader->Misc.VirtualSize);
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
