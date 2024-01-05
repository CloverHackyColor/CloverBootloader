#ifndef __CLOVER_POSIX_H__
#define __CLOVER_POSIX_H__

//
//#ifdef __cplusplus
//extern "C" {
//#endif



#include "stdio.h"
#include "stdint.h"
#include "stddef.h"
#include "limits.h"
#include "stdarg.h"
#include "stdlib.h"
#include "string.h"
#include "strings.h"
#include "wchar.h"
#include "abort.h"
#include "posix_additions.h"
//
// Macros that directly map functions to BaseLib, BaseMemoryLib, and DebugLib functions
// originally from OpensslLib

//#define memset(dest,ch,count)             SetMem(dest,(UINTN)(count),(UINT8)(ch))
//#define memchr(buf,ch,count)              ScanMem8(buf,(UINTN)(count),(UINT8)ch)
//#define memcmp(buf1,buf2,count)           (int)(CompareMem(buf1,buf2,(UINTN)(count)))
//#define memmove(dest,source,count)        CopyMem(dest,source,(UINTN)(count))
//#define strcmp                            AsciiStrCmp
//#define strncmp(string1,string2,count)    (int)(AsciiStrnCmp(string1,string2,(UINTN)(count)))
//#define strcpy(strDest,strSource)         AsciiStrCpyS(strDest,AsciiStrLen(strDest)+1,strSource)
//#define strncpy(strDest,strSource,count)  AsciiStrnCpyS(strDest,(UINTN)count+1,strSource,(UINTN)count)
//#define strlen(str)                       (size_t)(AsciiStrLen(str))
//#define strcat(strDest,strSource)         AsciiStrCatS(strDest,AsciiStrLen(strDest)+1,strSource)
//#define strchr(str,ch)                    ScanMem8((void *)(str),AsciiStrSize(str),(UINT8)ch)
//#define strstr(a,b)                       AsciiStrStr(a,b)


#ifdef __cplusplus
extern "C" {
#endif

void abort(void);

#ifdef __cplusplus
}
#endif


inline float fabsf(float x) {
  if (x < 0.f) return -x;
  return x;
}

//
//
//#ifdef __cplusplus
//} // extern "C"
//#endif


#endif
