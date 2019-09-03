#if defined(__i386__)
#include "./i386/setjmp.S"
#elif defined(__x86_64__)
#include "./x86_64/setjmp.S"
#elif defined(__sparc__)
#include "./sparc64/setjmp.S"
#elif defined(__mips__)
#include "./mips/setjmp.S"
#elif defined(__powerpc__) || defined(__PPC__)
#include "./powerpc/setjmp.S"
#elif defined(__ia64__)
#include "./ia64/setjmp.S"
#include "./ia64/longjmp.S"
#elif defined(__arm__)
#include "./arm/setjmp.S"
#elif defined(__aarch64__)
#include "./arm64/setjmp.S"
#else
#error "Unknown target cpu type"
#endif
