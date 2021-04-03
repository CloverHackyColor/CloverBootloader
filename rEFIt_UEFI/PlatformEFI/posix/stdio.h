#ifndef __CLOVER_STDIO_H__
#define __CLOVER_STDIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#define __attribute__(x)
#endif

int vprintf(const char* format, VA_LIST ap);
int printf(const char* format, ...) __attribute__((format(printf, 1, 2)));


//int snprintf(char* str, size_t size, const char* format, ...) __attribute__((format(printf, 1, 2)));


#ifdef __cplusplus
}
#endif


#endif
