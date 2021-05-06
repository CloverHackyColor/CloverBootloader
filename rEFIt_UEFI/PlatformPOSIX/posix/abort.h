#ifndef __PANIC_H__
#define __PANIC_H__

#ifdef _MSC_VER
#   define __attribute__(x)
#endif

#if !defined(__cplusplus) && !defined(bool) // C doesn't know bool
#define bool unsigned char
#endif

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

#if !defined(_MACH_H_)
#if !defined(PANIC_CAN_RETURN) && defined(_MSC_VER)
__declspec(noreturn)
#endif
void panic(const char* format, ...) __attribute__((__format__(__printf__, 1, 2)))
#ifndef PANIC_CAN_RETURN
    __attribute__ ((noreturn))
#endif
;

void panic_ask(const char* format, ...) __attribute__((__format__(__printf__, 1, 2)));
void log_technical_bug(const char* format, ...) __attribute__((__format__(__printf__, 1, 2)));


#endif

#ifndef assert
#ifdef _MSC_VER
# define assert(expr) _assert(expr, "Expression \"%s\" failed in %s", #expr, __FUNCSIG__)
#else
# define assert(expr) _assert(expr, "Expression \"%s\" failed in %s", #expr, __PRETTY_FUNCTION__)
#endif
#endif
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
