
#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile

#include <stdlib.h> // for abort()

//#if defined(CLOVER_BUILD) || !defined(_MSC_VER)
//void abort(void)
//{
//  printf("A fatal error happened. System halted\n");
//  while (1) { // tis will avoid warning : Function declared 'noreturn' should not return
//    abort();
//  }
//}
//#endif

bool stop_at_panic = true;
bool i_have_panicked = false;

/*
 *
 * Function panic_ seems useless. It's same as panic(). It's to be able to put a breakpoint in gdb with br panic_(). This is done in gdb_launch script in Qemu
 */
static void panic_(const char* format, VA_LIST va)
#ifndef PANIC_CAN_RETURN
    __attribute__ ((noreturn));
#endif
;


#define FATAL_ERROR_MSG "\nA fatal error happened. System halted.\n"
static void panic_(const char* format, VA_LIST va)
{
  if ( format ) {
    vprintf(format, va);
  }
  printf(FATAL_ERROR_MSG);
  abort();
}

void panic(const char* format, ...)
{
#ifdef PANIC_CAN_RETURN
  if ( stop_at_panic ) {
    VA_LIST va;
    VA_START(va, format);
    panic_(format, va); // panic doesn't return
//    VA_END(va);
  }else{
    i_have_panicked = true;
  }
#else
  VA_LIST va;
  VA_START(va, format);
  panic_(format, va); // panic doesn't return
#endif
}

/*
 * Future version to warn about problem but offer the possibility to try to continue
 * It's not done yes. So far, it's just panic
 * TODO:
 */
void panic_ask(const char* format, ...)
{
#ifdef PANIC_CAN_RETURN
  if ( stop_at_panic ) {
    VA_LIST va;
    VA_START(va, format);
    panic_(format, va); // panic doesn't return
//    VA_END(va);
  }else{
    i_have_panicked = true;
  }
#else
  VA_LIST va;
  VA_START(va, format);
  panic_(format, va); // panic doesn't return
#endif
}

/*
 * Future version to log about pontential technical bugs
 * It's not done yes. So far, it's just panic
 * TODO:
 */
void log_technical_bug(const char* format, ...)
{
#ifdef PANIC_CAN_RETURN
  if ( stop_at_panic ) {
    VA_LIST va;
    VA_START(va, format);
    panic_(format, va); // panic doesn't return
//    VA_END(va);
  }else{
    i_have_panicked = true;
  }
#else
  VA_LIST va;
  VA_START(va, format);
  panic_(format, va); // panic doesn't return
#endif
}


void panic(void)
{
  panic(nullptr);
}


void _assert(bool b, const char* format, ...)
{
  if ( !b ) {
    VA_LIST va;
    VA_START(va, format);
    panic_(format, va); // panic doesn't return
  }
}
