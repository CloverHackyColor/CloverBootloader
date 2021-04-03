#ifndef __CLOVER_STDINT_H__
#define __CLOVER_STDINT_H__


#define INT8_MIN (-128)
#define INT16_MIN (-32768)
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN  (-9223372036854775807LL - 1)

#define INT8_MAX 127
#define INT16_MAX 32767
#define INT32_MAX 2147483647
#define INT64_MAX 9223372036854775807LL

#define UINT8_MAX 0xff /* 255U */
#define UINT16_MAX 0xffff /* 65535U */
#define UINT32_MAX 0xffffffff  /* 4294967295U */
#define UINT64_MAX 0xffffffffffffffffULL /* 18446744073709551615ULL */


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

