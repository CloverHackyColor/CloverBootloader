#ifdef CLOVER_BUILD
#include "../../Platform/BootLog.h"


#ifdef __cplusplus
extern "C" {
#endif

#include <Library/BaseLib.h> // for CpuDeadLoop

#ifdef __cplusplus
} // extern "C"
#endif

#else

#include <Platform.h>

#endif

void abort(void)
{
		DebugLog(2, "A fatal error happened. System halted\n");
	    while (1) { // tis will avoid warning : Function declared 'noreturn' should not return
		CpuDeadLoop();
}
}
