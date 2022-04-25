/** @file

  Root include file for Mde Package UEFI, UEFI_APPLICATION type modules.

  This is the include file for any module of type UEFI and UEFI_APPLICATION. Uefi modules only use
  types defined via this include file and can be ported easily to any
  environment.

Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __PI_UEFI_H__
#define __PI_UEFI_H__

// This is a hack to replace EFI_GUID by the C++ improved version (they are 100% binary compatible).
// Any reference to CLOVER or Unit tests should not be here, of course.
// Problem is that EFI_GUID ++ version refers to XString and XString is defined inside Clover instead of being of module.
// Because the module CppMemLib is not in Clover anymore (that's because of Visual Studio!) and has no access to XString, I have to not include Guid++.h for CppMemLib 
// That should be changed...
#if defined(__cplusplus) && ( defined(CLOVER_BUILD) || defined(UNIT_TESTS) )
#include <Guid++.h>
#endif

#include <Uefi/UefiBaseType.h>
#include <Uefi/UefiSpec.h>

#endif

