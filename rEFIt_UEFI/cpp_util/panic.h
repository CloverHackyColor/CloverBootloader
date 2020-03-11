#ifndef __PANIC_H__
#define __PANIC_H__

extern bool stop_at_panic;
extern bool i_have_panicked;

void panic(void);
void panic(const char* s);

#endif
