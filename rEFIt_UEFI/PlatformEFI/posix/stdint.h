#ifndef __CLOVER_STDINT_H__
#define __CLOVER_STDINT_H__

// Currently only compiling 64 bits.
// If compiling for other size, #ifdef the static_assert depending of the platform and adjust constant (INT_MIN, INT_MAX)
#ifdef __cplusplus
static_assert(sizeof(char) == 1, "sizeof(char) != 1");
static_assert(sizeof(short) == 2, "sizeof(short) != 2");
static_assert(sizeof(int) == 4, "sizeof(int) != 4");
static_assert(sizeof(long) == 8, "sizeof(long) != 8"); // Jief : I think this break on Windows. Conditional compilation required to restore Windows compatibility
//so why EDK2 never used "long". It uses INT32.
static_assert(sizeof(long long) == 8, "sizeof(long long) != 8");
static_assert(true, "true");
#endif

////#if defined(OPENSSL_SMALL_FOOTPRINT) && defined(__clang__)
//// let's include the one from the compiler. Except for uintptr_t because it's define as a long and we prefer long long for microsoft compatibility. It's same on macos.
//// It's currently (Jan 2024) only needed when you compile with OpenSsl and xcode because of the inclusion of stdatomic
//#define _UINTPTR_T // for clang to avoid to declare uintptr_t
//#undef __UINTPTR_TYPE__
//#include_next <stdint.h>
////#endif


typedef UINT8 uint8_t;
typedef UINT16 uint16_t;
typedef UINT32 uint32_t;
typedef UINT64 uint64_t;


typedef INT8 int8_t;
typedef INT16 int16_t;
typedef INT32 int32_t;
typedef INT64 int64_t;

#define PRId8     "hhd"
#define PRId16    "hd"
#define PRId32    "d"
#define PRId64    "lld"

// Jief : Certainly not massively multi platform and multi target ! So far we only compile x86_64 (I think). To be extended if needed.
#ifdef _MSC_VER
#define PRIuPTR "lld"
#else
#define PRIuPTR "lld"
#endif


#define PRIud8     "hhu"
#define PRIu16    "hu"
#define PRIu32    "u"
#define PRIu64    "llu"

#endif

