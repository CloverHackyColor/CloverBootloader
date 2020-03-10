#include "panic.h"
#include <Platform.h>

//extern "C" {
//#include <Library/BaseLib.h> // for CpuDeadLoop
//}

/*
 *
 * If this modified, you may have to change the Qemu/gdb_launch script to adjust the breakpoint line.
 * gdb_launch put a breakpoint at CpuDeadLoop();
 * Currently line 18
 */

void panic(void)
{
	DebugLog(2, "A fatal error happened. System halted\n");
	CpuDeadLoop();
}
