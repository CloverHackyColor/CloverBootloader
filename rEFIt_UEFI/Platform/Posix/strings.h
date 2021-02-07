//
//  strings.h
//  CloverX64
//
//  Created by Jief on 01/02/2021.
//

#ifndef strings_h
#define strings_h

//#include <stdio.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int strncasecmp(const char* s1, const char* s2, size_t n);
int strcasecmp(const char* s1, const char* s2);

#ifdef __cplusplus
}
#endif

#endif /* strings_h */
