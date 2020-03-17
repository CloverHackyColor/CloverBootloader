#ifndef __POSIX_H__
#define __POSIX_H__

//
//#ifdef __cplusplus
//extern "C" {
//#endif



#include "stdint.h"
#include "stddef.h"
//
// Macros that directly map functions to BaseLib, BaseMemoryLib, and DebugLib functions
// originally from OpensslLib

#define memcpy(dest,source,count)         CopyMem(dest,source,(UINTN)(count))
#define memset(dest,ch,count)             SetMem(dest,(UINTN)(count),(UINT8)(ch))
#define memchr(buf,ch,count)              ScanMem8(buf,(UINTN)(count),(UINT8)ch)
#define memcmp(buf1,buf2,count)           (int)(CompareMem(buf1,buf2,(UINTN)(count)))
#define memmove(dest,source,count)        CopyMem(dest,source,(UINTN)(count))
//#define strcmp                            AsciiStrCmp
//#define strncmp(string1,string2,count)    (int)(AsciiStrnCmp(string1,string2,(UINTN)(count)))
#define strcpy(strDest,strSource)         AsciiStrCpyS(strDest,AsciiStrLen(strDest)+1,strSource)
#define strncpy(strDest,strSource,count)  AsciiStrnCpyS(strDest,(UINTN)count+1,strSource,(UINTN)count)
//#define strlen(str)                       (size_t)(AsciiStrLen(str))
#define strcat(strDest,strSource)         AsciiStrCatS(strDest,AsciiStrLen(strDest)+1,strSource)
#define strchr(str,ch)                    ScanMem8((VOID *)(str),AsciiStrSize(str),(UINT8)ch)
#define strstr(a,b)                       AsciiStrStr(a,b)

void abort(void);

//// edkII Strcmp seems quite inefficient, even vs a naive implementation
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
size_t strlen(const char *str);


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
