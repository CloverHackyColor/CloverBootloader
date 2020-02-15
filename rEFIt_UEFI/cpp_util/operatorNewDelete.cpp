#include <Platform/Platform.h>

#if 0
#define DBG(...) DebugLog(2, __VA_ARGS__)
#else
#define DBG(...)
#endif


void* operator new  ( unsigned long count )
{
	void* ptr = AllocatePool(count);
	if ( !ptr ) {
		DebugLog(2, "AllocatePool(%d) returned NULL. Cpu halted\n", count);
		CpuDeadLoop();
	}
	return ptr;
}

void operator delete  ( void* ptr ) noexcept
{
	return FreePool(ptr);
}


