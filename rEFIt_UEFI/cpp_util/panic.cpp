#include "panic.h"

extern "C" {
#include <Library/BaseLib.h> // for CpuDeadLoop
}

void panic(void)
{
	CpuDeadLoop();
}
