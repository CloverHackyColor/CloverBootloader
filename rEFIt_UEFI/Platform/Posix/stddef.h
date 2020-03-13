
#include "stdint.h"

// How can we prevent VC to define this ?
#ifdef _MSC_VER

// VS define size_t, but can't find a macro for SIZE_T_MAX
#define	SIZE_T_MAX MAX_UINTN

#else

#define	SIZE_T_MAX MAX_UINTN
typedef UINTN size_t;

#endif

typedef INTN ptrdiff_t;
typedef UINTN uintptr_t;

