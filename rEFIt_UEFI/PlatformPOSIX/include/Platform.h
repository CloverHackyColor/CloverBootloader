//
//  Platform.h.h
//  cpp_tests
//
//  Created by jief on 23.02.20.
//  Copyright © 2020 Jief_Machak. All rights reserved.
//

#ifndef Platform_h_h
#define Platform_h_h


// Replacing uintptr_t with unsigned long long doesn't work anymore with stdlic++ with xcode 12.5 (or probably some before)
// seems to work again in xcode 14. (don't know for 13)

#ifndef _UINTPTR_T
#define _UINTPTR_T // to prevent macOS/Clang definition of uintptr_t (map to a long). We prefer long long so we can use %llu on all platform (including microsoft)
#endif
#ifndef _PTRDIFF_T_DECLARED
#define _PTRDIFF_T_DECLARED // to prevent macOS/GCC definition of uintptr_t (map to a long). We prefer long long so we can use %llu on all platform (including microsoft)
#endif

#ifdef _MSC_VER
#include <Windows.h>
#endif

//#pragma clang diagnostic ignored "-Wc99-extensions"

// Replacement of uintptr_t to avoid warning in printf. It needs macro _UINTPTR_T to avoid to standard definition
typedef unsigned long long  uintptr_t;
// Replacement of printf format
#include <inttypes.h>
#undef PRIdPTR
#undef PRIiPTR
#undef PRIoPTR
#undef PRIuPTR
#undef PRIxPTR
#undef PRIXPTR
#  define PRIdPTR       "lld"
#  define PRIiPTR       "lli"
#  define PRIoPTR       "llo"
#  define PRIuPTR       "llu"
#  define PRIxPTR       "llx"
#  define PRIXPTR       "llX"

#include "posix/posix.h"
#include <Efi.h>
#include "posix/posix_additions.h"

#ifdef __cplusplus
extern "C" {
#endif

#define malloc(size) my_malloc(size)
#define free(size) my_free(size)
void* my_malloc(size_t size);
void my_free(void* p);

#ifdef __cplusplus
}
#endif

#include "../../../../rEFIt_UEFI/include/OneLinerMacros.h"


#include "./posix/abort.h"
#include "../../../rEFIt_UEFI/cpp_foundation/unicode_conversions.h"

#ifdef __cplusplus
#include "../../../rEFIt_UEFI/cpp_foundation/apd.h"
#include "../../../rEFIt_UEFI/cpp_foundation/XString.h"
#include "../../../rEFIt_UEFI/cpp_foundation/XArray.h"
#include "../../../rEFIt_UEFI/cpp_foundation/XObjArray.h"
#include "../include/remove_ref.h"
#include "../cpp_lib/undefinable.h"

#include "../include/OneLinerMacros.h"

#include "../../../rEFIt_UEFI/entry_scan/common.h"
#include "../../../rEFIt_UEFI/libeg/BmLib.h"
#include "../Platform/BootLog.h"
#include "../Platform/BasicIO.h"
#include "../Platform/VersionString.h"
#include "../Platform/Utils.h"
#include "../../../rEFIt_UEFI/Platform/Utils.h"
#include "../cpp_util/operatorNewDelete.h"
#endif


#if defined(__clang__) // it works to include <Foundation/Foundation.h> and that allows to use NS... objects. Could be useful for a tool running on macOS.
//#ifdef __OBJC__
//#define _MACH_H_
//#define __DEBUGGING__
//#import <Foundation/Foundation.h>
//#undef CMASK
//#endif
#endif

// to be able to compile AutoGen.c
#ifdef __cplusplus
extern "C" {
#endif

//#include "../../Build/Clover/DEBUGMACOS_XCODE8/X64/rEFIt_UEFI/refit/DEBUG/AutoGen.h"
EFI_STATUS
EFIAPI
RefitMain (IN EFI_HANDLE           ImageHandle,
           IN EFI_SYSTEM_TABLE     *SystemTable);

#ifdef __cplusplus
}
#endif


#endif /* Platform_h_h */
