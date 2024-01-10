#ifndef __ASSERT_H__
#define __ASSERT_H__

#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifdef _MSC_VER
# define assert(expr) _assert(expr, "Expression \"%s\" failed in %s", #expr, __FUNCSIG__)
#else
# define assert(expr) _assert(expr, "Expression \"%s\" failed in %s", #expr, __PRETTY_FUNCTION__)
#endif
#define assertf(...) _assert(__VA_ARGS__)

void _assert(bool b, const char* format, ...) __attribute__((__format__(__printf__, 2, 3)));

#ifdef __cplusplus
}
#endif

#endif
