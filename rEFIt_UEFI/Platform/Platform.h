/*
Headers collection for procedures
*/

#ifndef __REFIT_PLATFORM_H__
#define __REFIT_PLATFORM_H__

// Set all debug options - apianti
// Uncomment to set all debug options
// Comment to use source debug options
//#define DEBUG_ALL 2

#include "Posix/posix.h"
#include "../include/Efi.h"

#ifdef __cplusplus
#include <Library/printf_lite.h>
#include "../cpp_foundation/XString.h"
#include "../cpp_foundation/XArray.h"
#include "../cpp_foundation/XObjArray.h"
#include "../cpp_util/remove_ref.h"
#endif

#include "../include/OneLinerMacros.h"

//#include "Self.h"
#include "../entry_scan/common.h"
#include "../libeg/BmLib.h"
#include "BootLog.h"
#include "BasicIO.h"
#include "VersionString.h"
//#include "Settings.h"

#ifndef CLOVERAPPLICATION
#include "../Platform/Utils.h"
#endif


#ifndef DEBUG_ALL
#define MsgLog(...)  DebugLog(1, __VA_ARGS__)
#else
#define MsgLog(...)  DebugLog(DEBUG_ALL, __VA_ARGS__)
#endif



// Jief : temporary rename
#define AllocateZeroPool AllocateZeroPool

#endif
