#ifndef __PANIC_H__
#define __PANIC_H__

#ifdef _MSC_VER
#   define __attribute__(x)
#endif

extern bool stop_at_panic;
extern bool i_have_panicked;

#ifdef __cplusplus // C cannot accept 2 functions with same name and different parameters.
void panic(void)
#ifndef PANIC_CAN_RETURN
    __attribute__ ((noreturn))
#endif
;
#endif

void panic(const char* format, ...) __attribute__((__format__(__printf__, 1, 2)))
#ifndef PANIC_CAN_RETURN
    __attribute__ ((noreturn))
#endif
;

#define assert(expr) _assert(expr, "Expression \"%s\" failed in %s", #expr, __PRETTY_FUNCTION__)
#define assertf(...) _assert(__VA_ARGS__)

void _assert(bool b, const char* format, ...) __attribute__((__format__(__printf__, 2, 3)));

#ifdef __cplusplus
class DontStopAtPanic
{
  public:
	DontStopAtPanic() { stop_at_panic = false; i_have_panicked = false; }
	~DontStopAtPanic() { stop_at_panic = true; i_have_panicked = false; }
};
#endif

#endif
