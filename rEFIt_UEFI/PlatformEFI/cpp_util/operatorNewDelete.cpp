#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile

#if 0
#define DBG(...) DebugLog(2, __VA_ARGS__)
#else
#define DBG(...)
#endif

#if defined(_MSC_VER)
void* operator new  (size_t count)
#else
void* operator new(unsigned long count)
#endif
{
	void* ptr = AllocatePool(count);
	if ( !ptr ) {
		DebugLog(2, "AllocatePool(%lu) returned NULL. Cpu halted\n", count);
		CpuDeadLoop();
	}
//  MemLogf(false, 0, "operator new(%lu) %llx\n", count, uintptr_t(ptr));
	return ptr;
}

#if defined(_MSC_VER)
void* operator new[](size_t count)
#else
void* operator new[](unsigned long count)
#endif
{
  void* ptr = AllocatePool(count);
  if ( !ptr ) {
    DebugLog(2, "AllocatePool(%lu) returned NULL. Cpu halted\n", count);
    CpuDeadLoop();
  }
//  MemLogf(false, 0, "operator new[](%lu) %llx\n", count, uintptr_t(ptr));
  return ptr;
}

uint64_t operator_delete_count1 = 0;
uint64_t operator_delete_count2 = 0;
uint64_t operator_delete_count3 = 0;


#ifdef _MSC_VER
#pragma warning(disable : 4577)
#endif
void operator delete  ( void* ptr ) noexcept
{
//  ++operator_delete_count1;
//  MemLogf(false, 0, "operator delete(%llx) %lld\n", uintptr_t(ptr), operator_delete_count1);
  FreePool(ptr);
}

void operator delete[](void* ptr) noexcept
{
  FreePool(ptr);
}

#ifdef _MSC_VER
void _cdecl operator delete (void * ptr, unsigned __int64 count)
#else
void operator delete (void * ptr, UINTN count)
#endif
{
//  ++operator_delete_count2;
//  MemLogf(false, 0, "operator delete(%llx, %lld) %lld\n", uintptr_t(ptr), count, operator_delete_count2);
  FreePool(ptr);
}


#ifdef _MSC_VER
void _cdecl operator delete[](void * ptr, unsigned __int64 count)
#else
void operator delete[](void * ptr, UINTN count)
#endif
{
//  ++operator_delete_count3;
//  MemLogf(false, 0, "operator delete[](%llx, %lld) %lld\n", uintptr_t(ptr), count, operator_delete_count3);
  FreePool(ptr);
}



