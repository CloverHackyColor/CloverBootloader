#ifndef __PANIC_H__
#define __PANIC_H__

extern bool stop_at_panic;
extern bool i_have_panicked;

void panic(void);
void panic(const char* s);

class DontStopAtPanic
{
  public:
	DontStopAtPanic() { stop_at_panic = false; i_have_panicked = false; }
	~DontStopAtPanic() { stop_at_panic = true; i_have_panicked = false; }
};

#endif
