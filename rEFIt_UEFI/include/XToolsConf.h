#ifndef __XTOOLSCOMMON_H__
#define __XTOOLSCOMMON_H__

#include <posix.h>


#define xsize size_t
//#define xisize INTN
#define MAX_XSIZE SIZE_T_MAX
//#define MAX_XISIZE MAX_INTN

#define XStringGrowByDefault 10
#define XArrayGrowByDefault 8
#define XBufferGrowByDefault 16

/* For convience, operator [] is define with int parameter.
 * Defining __XTOOLS_CHECK_OVERFLOW__ make a check that the parameter is >= 0
 * TODO : make new XString using __XTOOLS_CHECK_OVERFLOW__
 */
#define __XTOOLS_CHECK_OVERFLOW__

#define Xrealloc(ptr, newsize, oldsize) reallocWithOldSize(ptr, newsize, oldsize)


#endif
