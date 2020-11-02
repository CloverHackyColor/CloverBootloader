//
//  Base.h
//  cpp_tests
//
//  Created by Jief on 12/10/2020.
//  Copyright © 2020 JF Knudsen. All rights reserved.
//

#ifndef Base_h
#define Base_h

#include <Library/ProcessorBind.h>

#if defined(_MSC_VER)

#define VA_LIST                      va_list
#define VA_START(Marker, Parameter)  va_start (Marker, Parameter)
#define VA_ARG(Marker, TYPE)         va_arg(Marker, TYPE)
#define VA_END(Marker)               va_end (Marker)
#define VA_COPY(Dest, Start)         va_copy(Dest, Start)

#else

typedef __builtin_va_list VA_LIST;

#define VA_START(Marker, Parameter)  __builtin_va_start (Marker, Parameter)

#define VA_ARG(Marker, TYPE)         ((sizeof (TYPE) < sizeof (UINTN)) ? (TYPE)(__builtin_va_arg (Marker, UINTN)) : (TYPE)(__builtin_va_arg (Marker, TYPE)))

#define VA_END(Marker)               __builtin_va_end (Marker)

#define VA_COPY(Dest, Start)         __builtin_va_copy (Dest, Start)

#endif


#endif /* Base_h */
