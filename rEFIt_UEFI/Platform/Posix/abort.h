#ifndef __PANIC_H__
#define __PANIC_H__

#ifdef _MSC_VER
#   define __attribute__(x)
#endif

extern bool stop_at_panic;
extern bool i_have_panicked;

void panic(void)
#ifndef PANIC_CAN_RETURN
    __attribute__ ((noreturn));
#endif
;

void panic(const char* format, ...) __attribute__((__format__(__printf__, 1, 2)))
#ifndef PANIC_CAN_RETURN
    __attribute__ ((noreturn));
#endif
;


class DontStopAtPanic
{
  public:
	DontStopAtPanic() { stop_at_panic = false; i_have_panicked = false; }
	~DontStopAtPanic() { stop_at_panic = true; i_have_panicked = false; }
};

#endif
