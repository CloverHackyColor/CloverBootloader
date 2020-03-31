#include "panic.h"
#include <Platform.h>

//extern "C" {
//#include <Library/BaseLib.h> // for CpuDeadLoop
//}

bool stop_at_panic = true;
bool i_have_panicked = false;

/*
 *
 * Function panic_ seems useless. It's same as panic(). It's to be able to put a breakpoint in gdb with br panic_(). This is done in gdb_launch script in Qemu
 */
static void panic_(const char* s)
{
	if ( s ) DebugLog(2, "%s\n", s);
	DebugLog(2, "A fatal error happened. System halted\n");
	CpuDeadLoop();
}


void panic(const char* s)
{
	if ( stop_at_panic ) {
		panic_(s);
	}else{
		i_have_panicked = true;
	}
}


void panic(void)
{
	panic(nullptr);
}
