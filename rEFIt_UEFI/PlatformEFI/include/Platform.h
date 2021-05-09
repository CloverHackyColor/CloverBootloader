/*
Headers collection for procedures
*/

#ifndef __REFIT_PLATFORM_H__
#define __REFIT_PLATFORM_H__

// Set all debug options - apianti
// Uncomment to set all debug options
// Comment to use source debug options
//#define DEBUG_ALL 2

#include "posix/posix.h"
#include <Efi.h>

#ifdef __cplusplus
#include <Library/printf_lite.h>
#include "../cpp_foundation/XString.h"
#include "../cpp_foundation/XArray.h"
#include "../cpp_foundation/XObjArray.h"
#include "../include/remove_ref.h"
#include "../cpp_lib/undefinable.h"
#endif

#include "../include/OneLinerMacros.h"

//#include "../Settings/Self.h"
#include "../entry_scan/common.h"
#include "../libeg/BmLib.h"
#include "../Platform/BootLog.h"
#include "../Platform/BasicIO.h"
#include "../Platform/VersionString.h"
#include "../Platform/Utils.h"
//#include "Settings.h"

#ifndef CLOVERAPPLICATION
#include "../Platform/Utils.h"
#endif


#if defined(_MSC_VER) && !defined(__PRETTY_FUNCTION__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif


// Jief : temporary rename
#define AllocateZeroPool AllocateZeroPool

#endif
