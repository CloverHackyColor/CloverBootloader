#ifndef __CLOVER_WCHAR_H__
#define __CLOVER_WCHAR_H__

#include "stddef.h" // for size_t

size_t wcslen(const wchar_t *s);
int wcsncmp(const wchar_t *s1, const wchar_t * s2, size_t n);

#endif
