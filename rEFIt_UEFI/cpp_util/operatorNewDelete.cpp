#include <Platform/Platform.h>

#if 0
#define DBG(...) DebugLog(2, __VA_ARGS__)
#else
#define DBG(...)
#endif

#if defined(_MSC_VER)
void* operator new  (size_t count)
#else
void* operator new  (unsigned long count)
#endif
{
	void* ptr = AllocatePool(count);
	if ( !ptr ) {
		DebugLog(2, "AllocatePool(%lu) returned NULL. Cpu halted\n", count);
		CpuDeadLoop();
	}
	return ptr;
}

#if defined(_MSC_VER)
void* operator new[]  (size_t count)
#else
void* operator new[]  (unsigned long count)
#endif
{
  void* ptr = AllocatePool(count);
  if ( !ptr ) {
    DebugLog(2, "AllocatePool(%lu) returned NULL. Cpu halted\n", count);
    CpuDeadLoop();
  }
  return ptr;
}

#ifdef _MSC_VER
#pragma warning(disable : 4577)
#endif
void operator delete  ( void* ptr ) noexcept
{
	return FreePool(ptr);
}
#ifdef _MSC_VER
void _cdecl operator delete (void * ptr, unsigned __int64 count)
#else
void operator delete (void * ptr, UINTN count)
#endif
{
  return FreePool(ptr);
}


