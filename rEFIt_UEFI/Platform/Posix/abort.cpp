//#ifdef CLOVER_BUILD
//#include "../../Platform/BootLog.h"

#include <Platform.h>

//
//#ifdef __cplusplus
//extern "C" {
//#endif
//
//#include <Library/BaseLib.h> // for CpuDeadLoop
//
//#ifdef __cplusplus
//} // extern "C"
//#endif
//
//#else
//
//#include <Platform.h>
//
//#endif

void abort(void)
{
	printf("A fatal error happened. System halted\n");
	while (1) { // tis will avoid warning : Function declared 'noreturn' should not return
		CpuDeadLoop();
	}
}


bool stop_at_panic = true;
bool i_have_panicked = false;

/*
 *
 * Function panic_ seems useless. It's same as panic(). It's to be able to put a breakpoint in gdb with br panic_(). This is done in gdb_launch script in Qemu
 */
static void panic_(const char* format, VA_LIST va)
{
	if ( format ) {
		vprintf(format, va);
	}
	printf("A fatal error happened. System halted\n");
	while (1) { // this will avoid warning : Function declared 'noreturn' should not return
		CpuDeadLoop();
	}
}

void panic(const char* format, ...)
{
	if ( stop_at_panic ) {
		VA_LIST va;
		VA_START(va, format);
		panic_(format, va); // panic doesn't return
		//VA_END(va);
	}else{
		i_have_panicked = true;
	}
}


void panic(void)
{
	panic(nullptr);
}
