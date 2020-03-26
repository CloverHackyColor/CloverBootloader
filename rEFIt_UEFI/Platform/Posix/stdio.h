#ifndef __CLOVER_STDIO_H__
#define __CLOVER_STDIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <Library/printf_lite.h>

#ifdef _MSC_VER
#define __attribute__(x)
#endif

int printf(const char* format, ...) __attribute__((format(printf, 1, 2)));


//int snprintf(char* str, size_t size, const char* format, ...) __attribute__((format(printf, 1, 2)));;

const char* strerror(EFI_STATUS errnum);
const char* strguid(EFI_GUID* guid);




#ifdef __cplusplus
}
#endif


#endif
