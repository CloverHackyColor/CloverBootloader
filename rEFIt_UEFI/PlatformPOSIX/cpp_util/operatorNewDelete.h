#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile

#if defined(_MSC_VER)
void* operator new  (size_t count);
#else
void* operator new(unsigned long count);
#endif

#if defined(_MSC_VER)
void* operator new[](size_t count);
#else
void* operator new[](unsigned long count);
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4577)
#endif
void operator delete  ( void* ptr ) noexcept;

#ifdef _MSC_VER
void _cdecl operator delete (void * ptr, unsigned __int64 count);
#else
void operator delete (void * ptr, UINTN count);
#endif

#ifdef _MSC_VER
void _cdecl operator delete[](void * ptr, unsigned __int64 count)
#else
void operator delete[](void * ptr, UINTN count);
#endif


