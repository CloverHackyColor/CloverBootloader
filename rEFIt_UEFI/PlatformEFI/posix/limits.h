//#define UINT_MAX
#define	SIZE_T_MAX MAX_UINTN

#define INT8_MIN   (-128)
#define INT16_MIN  (-32768)
#define INT32_MIN  (-2147483647 - 1)
#define INT64_MIN  (-9223372036854775807LL - 1)

#define INT8_MAX   127
#define INT16_MAX  32767
#define INT32_MAX  2147483647
#define INT64_MAX  9223372036854775807LL

#define UINT8_MAX  0xff /* 255U */
#define UINT16_MAX 0xffff /* 65535U */
#define UINT32_MAX 0xffffffff  /* 4294967295U */
#define UINT64_MAX 0xffffffffffffffffULL /* 18446744073709551615ULL */

#define CHAR_MIN   (-128)
#define SCHAR_MIN  (-128)
#define SHRT_MIN   (-32768)
#define INT_MIN    INT32_MIN
#define LONG_MIN   INT64_MIN
#define LLONG_MIN  INT64_MIN

#define CHAR_MAX   127
#define SCHAR_MAX  127
#define SHRT_MAX   32767
#define INT_MAX    INT32_MAX
#define LONG_MAX   INT64_MAX
#define LLONG_MAX  INT64_MAX

#define UCHAR_MAX  255
#define USHRT_MAX  65535
#define UINT_MAX   UINT32_MAX
#define ULONG_MAX  UINT64_MAX
#define ULLONG_MAX UINT64_MAX
