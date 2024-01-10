#ifndef __PANIC_H__
#define __PANIC_H__

#ifdef _MSC_VER
#   define __attribute__(x)
#endif

#include "stdbool.h" // C doesn't know bool, in case this header is included by a C source

extern bool stop_at_panic;
extern bool i_have_panicked;

#ifdef __cplusplus // C cannot accept 2 functions with same name and different parameters.
#if !defined(PANIC_CAN_RETURN) && defined(_MSC_VER)
__declspec(noreturn)
#endif
void panic(void)
#ifndef PANIC_CAN_RETURN
    __attribute__ ((noreturn))
#endif
;
#endif

#if !defined(PANIC_CAN_RETURN) && defined(_MSC_VER)
__declspec(noreturn)
#endif
void panic(const char* format, ...) __attribute__((__format__(__printf__, 1, 2)))
#ifndef PANIC_CAN_RETURN
    __attribute__ ((noreturn))
#endif
;


void log_technical_bug(const char* format, ...) __attribute__((__format__(__printf__, 1, 2)));


#ifdef __cplusplus
class DontStopAtPanic
{
  public:
	DontStopAtPanic() { stop_at_panic = false; i_have_panicked = false; }
	~DontStopAtPanic() { stop_at_panic = true; i_have_panicked = false; }
};
#endif

#endif
