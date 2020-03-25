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
void panic_(const char* s)
{
	if ( stop_at_panic ) {
		if ( s ) DebugLog(2, "%s\n", s);
		DebugLog(2, "A fatal error happened. System halted\n");
		CpuDeadLoop();
	}else{
//		if ( s ) DebugLog(2, "%s\n", s);
//		DebugLog(2, "A fatal error happened. Continue for testing\n");
		i_have_panicked = true;
	}
}


void panic(const char* s)
{
	panic_(s);
}


void panic(void)
{
	panic_(nullptr);
}
