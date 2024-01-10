//#define UINT_MAX
#define	SIZE_T_MAX MAX_UINTN

#ifndef INT8_MIN
#define INT8_MIN   (-128)
#endif
#ifndef INT16_MIN
#ifndef INT16_MIN
#endif
#define INT16_MIN  (-32768)
#endif
#ifndef INT32_MIN
#define INT32_MIN  (-2147483647 - 1)
#endif
#ifndef INT64_MIN
#define INT64_MIN  (-9223372036854775807LL - 1)
#endif

#ifndef INT8_MAX
#define INT8_MAX   127
#endif
#ifndef INT16_MAX
#define INT16_MAX  32767
#endif
#ifndef INT32_MAX
#define INT32_MAX  2147483647
#endif
#ifndef INT64_MAX
#define INT64_MAX  9223372036854775807LL
#endif

#ifndef UINT8_MAX
#define UINT8_MAX  0xff /* 255U */
#endif
#ifndef UINT16_MAX
#define UINT16_MAX 0xffff /* 65535U */
#endif
#ifndef UINT32_MAX
#define UINT32_MAX 0xffffffff  /* 4294967295U */
#endif
#ifndef UINT64_MAX
#define UINT64_MAX 0xffffffffffffffffULL /* 18446744073709551615ULL */
#endif

#ifndef CHAR_MIN
#define CHAR_MIN   (-128)
#endif
#ifndef SCHAR_MIN
#define SCHAR_MIN  (-128)
#endif
#ifndef SHRT_MIN
#define SHRT_MIN   (-32768)
#endif
#ifndef INT_MIN
#define INT_MIN    INT32_MIN
#endif
#ifndef LONG_MIN
#define LONG_MIN   INT64_MIN
#endif
#ifndef LLONG_MIN
#define LLONG_MIN  INT64_MIN
#endif

#ifndef CHAR_MAX
#define CHAR_MAX   127
#endif
#ifndef SCHAR_MAX
#define SCHAR_MAX  127
#endif
#ifndef SHRT_MAX
#define SHRT_MAX   32767
#endif
#ifndef INT_MAX
#define INT_MAX    INT32_MAX
#endif
#ifndef LONG_MAX
#define LONG_MAX   INT64_MAX
#endif
#ifndef LLONG_MAX
#define LLONG_MAX  INT64_MAX
#endif

#ifndef UCHAR_MAX
#define UCHAR_MAX  255
#endif
#ifndef USHRT_MAX
#define USHRT_MAX  65535
#endif
#ifndef UINT_MAX
#define UINT_MAX   UINT32_MAX
#endif
#ifndef ULONG_MAX
#define ULONG_MAX  UINT64_MAX
#endif
#ifndef ULLONG_MAX
#define ULLONG_MAX UINT64_MAX
#endif
