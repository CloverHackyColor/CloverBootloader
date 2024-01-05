
//#include <posix.h> // use angled, so posix.h will vary depending of the platform

#include <wchar.h>


size_t wcslen(const wchar_t *s)
{
	const wchar_t* p = s;
	while ( *p++ );
	return (size_t)(p-s-1);
}

//int wcsncmp(const wchar_t *s1, const wchar_t * s2, size_t n);
