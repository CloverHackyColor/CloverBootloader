/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2005,2006,2007,2008,2009  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GRUB_TYPES_HEADER
#define GRUB_TYPES_HEADER	1

#include <config.h>
#ifndef GRUB_UTIL
#include <grub/cpu/types.h>
#endif

#ifdef __MINGW32__
#define GRUB_PACKED __attribute__ ((packed,gcc_struct))
#else
#define GRUB_PACKED __attribute__ ((packed))
#endif

#ifdef GRUB_BUILD
# define GRUB_CPU_SIZEOF_VOID_P	BUILD_SIZEOF_VOID_P
# define GRUB_CPU_SIZEOF_LONG	BUILD_SIZEOF_LONG
# if BUILD_WORDS_BIGENDIAN
#  define GRUB_CPU_WORDS_BIGENDIAN	1
# else
#  undef GRUB_CPU_WORDS_BIGENDIAN
# endif
#elif defined (GRUB_UTIL)
# define GRUB_CPU_SIZEOF_VOID_P	SIZEOF_VOID_P
# define GRUB_CPU_SIZEOF_LONG SIZEOF_LONG
# ifdef WORDS_BIGENDIAN
#  define GRUB_CPU_WORDS_BIGENDIAN	1
# else
#  undef GRUB_CPU_WORDS_BIGENDIAN
# endif
#else /* ! GRUB_UTIL */
# define GRUB_CPU_SIZEOF_VOID_P	GRUB_TARGET_SIZEOF_VOID_P
# define GRUB_CPU_SIZEOF_LONG	GRUB_TARGET_SIZEOF_LONG
# ifdef GRUB_TARGET_WORDS_BIGENDIAN
#  define GRUB_CPU_WORDS_BIGENDIAN	1
# else
#  undef GRUB_CPU_WORDS_BIGENDIAN
# endif
#endif /* ! GRUB_UTIL */

#if GRUB_CPU_SIZEOF_VOID_P != 4 && GRUB_CPU_SIZEOF_VOID_P != 8
# error "This architecture is not supported because sizeof(void *) != 4 and sizeof(void *) != 8"
#endif

#if GRUB_CPU_SIZEOF_LONG != 4 && GRUB_CPU_SIZEOF_LONG != 8
# error "This architecture is not supported because sizeof(long) != 4 and sizeof(long) != 8"
#endif

#if !defined (GRUB_UTIL) && !defined (GRUB_TARGET_WORDSIZE)
# if GRUB_TARGET_SIZEOF_VOID_P == 4
#  define GRUB_TARGET_WORDSIZE 32
# elif GRUB_TARGET_SIZEOF_VOID_P == 8
#  define GRUB_TARGET_WORDSIZE 64
# endif
#endif

/* Define various wide integers.  */
typedef signed char		grub_int8_t;
typedef short			grub_int16_t;
typedef int			grub_int32_t;
#if GRUB_CPU_SIZEOF_LONG == 8
typedef long			grub_int64_t;
#else
typedef long long		grub_int64_t;
#endif

typedef unsigned char		grub_uint8_t;
typedef unsigned short		grub_uint16_t;
typedef unsigned		grub_uint32_t;
# define PRIxGRUB_UINT32_T	"x"
# define PRIuGRUB_UINT32_T	"u"
#if GRUB_CPU_SIZEOF_LONG == 8
typedef unsigned long		grub_uint64_t;
# define PRIxGRUB_UINT64_T	"lx"
# define PRIuGRUB_UINT64_T	"lu"
#else
typedef unsigned long long	grub_uint64_t;
# define PRIxGRUB_UINT64_T	"llx"
# define PRIuGRUB_UINT64_T	"llu"
#endif

/* Misc types.  */

#if GRUB_CPU_SIZEOF_VOID_P == 8
typedef grub_uint64_t	grub_addr_t;
typedef grub_uint64_t	grub_size_t;
typedef grub_int64_t	grub_ssize_t;

# define GRUB_SIZE_MAX 18446744073709551615UL

# if GRUB_CPU_SIZEOF_LONG == 8
#  define PRIxGRUB_SIZE	 "lx"
#  define PRIxGRUB_ADDR	 "lx"
#  define PRIuGRUB_SIZE	 "lu"
#  define PRIdGRUB_SSIZE "ld"
# else
#  define PRIxGRUB_SIZE	 "llx"
#  define PRIxGRUB_ADDR	 "llx"
#  define PRIuGRUB_SIZE  "llu"
#  define PRIdGRUB_SSIZE "lld"
# endif
#else
typedef grub_uint32_t	grub_addr_t;
typedef grub_uint32_t	grub_size_t;
typedef grub_int32_t	grub_ssize_t;

# define GRUB_SIZE_MAX 4294967295UL

# define PRIxGRUB_SIZE	"x"
# define PRIxGRUB_ADDR	"x"
# define PRIuGRUB_SIZE	"u"
# define PRIdGRUB_SSIZE	"d"
#endif

#define GRUB_UCHAR_MAX 0xFF
#define GRUB_USHRT_MAX 65535
#define GRUB_SHRT_MAX 0x7fff
#define GRUB_UINT_MAX 4294967295U
#define GRUB_INT_MAX 0x7fffffff
#define GRUB_INT32_MIN (-2147483647 - 1)
#define GRUB_INT32_MAX 2147483647

#if GRUB_CPU_SIZEOF_LONG == 8
# define GRUB_ULONG_MAX 18446744073709551615UL
# define GRUB_LONG_MAX 9223372036854775807L
# define GRUB_LONG_MIN (-9223372036854775807L - 1)
#else
# define GRUB_ULONG_MAX 4294967295UL
# define GRUB_LONG_MAX 2147483647L
# define GRUB_LONG_MIN (-2147483647L - 1)
#endif

typedef grub_uint64_t grub_properly_aligned_t;

#define GRUB_PROPERLY_ALIGNED_ARRAY(name, size) grub_properly_aligned_t name[((size) + sizeof (grub_properly_aligned_t) - 1) / sizeof (grub_properly_aligned_t)]

/* The type for representing a file offset.  */
typedef grub_uint64_t	grub_off_t;

/* The type for representing a disk block address.  */
typedef grub_uint64_t	grub_disk_addr_t;

/* Byte-orders.  */
static inline grub_uint16_t grub_swap_bytes16(grub_uint16_t _x)
{
   return (grub_uint16_t) ((_x << 8) | (_x >> 8));
}

#define grub_swap_bytes16_compile_time(x) ((((x) & 0xff) << 8) | (((x) & 0xff00) >> 8))
#define grub_swap_bytes32_compile_time(x) ((((x) & 0xff) << 24) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | (((x) & 0xff000000UL) >> 24))
#define grub_swap_bytes64_compile_time(x)	\
({ \
   grub_uint64_t _x = (x); \
   (grub_uint64_t) ((_x << 56) \
                    | ((_x & (grub_uint64_t) 0xFF00ULL) << 40) \
                    | ((_x & (grub_uint64_t) 0xFF0000ULL) << 24) \
                    | ((_x & (grub_uint64_t) 0xFF000000ULL) << 8) \
                    | ((_x & (grub_uint64_t) 0xFF00000000ULL) >> 8) \
                    | ((_x & (grub_uint64_t) 0xFF0000000000ULL) >> 24) \
                    | ((_x & (grub_uint64_t) 0xFF000000000000ULL) >> 40) \
                    | (_x >> 56)); \
})

#if (defined(__GNUC__) && (__GNUC__ > 3) && (__GNUC__ > 4 || __GNUC_MINOR__ >= 3)) || defined(__clang__)
static inline grub_uint32_t grub_swap_bytes32(grub_uint32_t x)
{
	return __builtin_bswap32(x);
}

static inline grub_uint64_t grub_swap_bytes64(grub_uint64_t x)
{
	return __builtin_bswap64(x);
}
#else					/* not gcc 4.3 or newer */
static inline grub_uint32_t grub_swap_bytes32(grub_uint32_t _x)
{
   return ((_x << 24)
	   | ((_x & (grub_uint32_t) 0xFF00UL) << 8)
	   | ((_x & (grub_uint32_t) 0xFF0000UL) >> 8)
	   | (_x >> 24));
}

static inline grub_uint64_t grub_swap_bytes64(grub_uint64_t _x)
{
   return ((_x << 56)
	   | ((_x & (grub_uint64_t) 0xFF00ULL) << 40)
	   | ((_x & (grub_uint64_t) 0xFF0000ULL) << 24)
	   | ((_x & (grub_uint64_t) 0xFF000000ULL) << 8)
	   | ((_x & (grub_uint64_t) 0xFF00000000ULL) >> 8)
	   | ((_x & (grub_uint64_t) 0xFF0000000000ULL) >> 24)
	   | ((_x & (grub_uint64_t) 0xFF000000000000ULL) >> 40)
	   | (_x >> 56));
}
#endif					/* not gcc 4.3 or newer */

#ifdef GRUB_CPU_WORDS_BIGENDIAN
# define grub_cpu_to_le16(x)	grub_swap_bytes16(x)
# define grub_cpu_to_le32(x)	grub_swap_bytes32(x)
# define grub_cpu_to_le64(x)	grub_swap_bytes64(x)
# define grub_le_to_cpu16(x)	grub_swap_bytes16(x)
# define grub_le_to_cpu32(x)	grub_swap_bytes32(x)
# define grub_le_to_cpu64(x)	grub_swap_bytes64(x)
# define grub_cpu_to_be16(x)	((grub_uint16_t) (x))
# define grub_cpu_to_be32(x)	((grub_uint32_t) (x))
# define grub_cpu_to_be64(x)	((grub_uint64_t) (x))
# define grub_be_to_cpu16(x)	((grub_uint16_t) (x))
# define grub_be_to_cpu32(x)	((grub_uint32_t) (x))
# define grub_be_to_cpu64(x)	((grub_uint64_t) (x))
# define grub_cpu_to_be16_compile_time(x)	((grub_uint16_t) (x))
# define grub_cpu_to_be32_compile_time(x)	((grub_uint32_t) (x))
# define grub_cpu_to_be64_compile_time(x)	((grub_uint64_t) (x))
# define grub_be_to_cpu64_compile_time(x)	((grub_uint64_t) (x))
# define grub_cpu_to_le32_compile_time(x)	grub_swap_bytes32_compile_time(x)
# define grub_cpu_to_le64_compile_time(x)	grub_swap_bytes64_compile_time(x)
# define grub_cpu_to_le16_compile_time(x)	grub_swap_bytes16_compile_time(x)
#else /* ! WORDS_BIGENDIAN */
# define grub_cpu_to_le16(x)	((grub_uint16_t) (x))
# define grub_cpu_to_le32(x)	((grub_uint32_t) (x))
# define grub_cpu_to_le64(x)	((grub_uint64_t) (x))
# define grub_le_to_cpu16(x)	((grub_uint16_t) (x))
# define grub_le_to_cpu32(x)	((grub_uint32_t) (x))
# define grub_le_to_cpu64(x)	((grub_uint64_t) (x))
# define grub_cpu_to_be16(x)	grub_swap_bytes16(x)
# define grub_cpu_to_be32(x)	grub_swap_bytes32(x)
# define grub_cpu_to_be64(x)	grub_swap_bytes64(x)
# define grub_be_to_cpu16(x)	grub_swap_bytes16(x)
# define grub_be_to_cpu32(x)	grub_swap_bytes32(x)
# define grub_be_to_cpu64(x)	grub_swap_bytes64(x)
# define grub_cpu_to_be16_compile_time(x)	grub_swap_bytes16_compile_time(x)
# define grub_cpu_to_be32_compile_time(x)	grub_swap_bytes32_compile_time(x)
# define grub_cpu_to_be64_compile_time(x)	grub_swap_bytes64_compile_time(x)
# define grub_be_to_cpu64_compile_time(x)	grub_swap_bytes64_compile_time(x)
# define grub_cpu_to_le16_compile_time(x)	((grub_uint16_t) (x))
# define grub_cpu_to_le32_compile_time(x)	((grub_uint32_t) (x))
# define grub_cpu_to_le64_compile_time(x)	((grub_uint64_t) (x))

#endif /* ! WORDS_BIGENDIAN */

static inline grub_uint16_t grub_get_unaligned16 (const void *ptr)
{
  struct grub_unaligned_uint16_t
  {
    grub_uint16_t d;
  } GRUB_PACKED;
  const struct grub_unaligned_uint16_t *dd
    = (const struct grub_unaligned_uint16_t *) ptr;
  return dd->d;
}

static inline void grub_set_unaligned16 (void *ptr, grub_uint16_t val)
{
  struct grub_unaligned_uint16_t
  {
    grub_uint16_t d;
  } GRUB_PACKED;
  struct grub_unaligned_uint16_t *dd = (struct grub_unaligned_uint16_t *) ptr;
  dd->d = val;
}

static inline grub_uint32_t grub_get_unaligned32 (const void *ptr)
{
  struct grub_unaligned_uint32_t
  {
    grub_uint32_t d;
  } GRUB_PACKED;
  const struct grub_unaligned_uint32_t *dd
    = (const struct grub_unaligned_uint32_t *) ptr;
  return dd->d;
}

static inline void grub_set_unaligned32 (void *ptr, grub_uint32_t val)
{
  struct grub_unaligned_uint32_t
  {
    grub_uint32_t d;
  } GRUB_PACKED;
  struct grub_unaligned_uint32_t *dd = (struct grub_unaligned_uint32_t *) ptr;
  dd->d = val;
}

struct grub_unaligned_uint64
{
  grub_uint64_t val;
} GRUB_PACKED;

typedef struct grub_unaligned_uint64 grub_unaligned_uint64_t;

static inline grub_uint64_t grub_get_unaligned64 (const void *ptr)
{
  const struct grub_unaligned_uint64 *dd
    = (const struct grub_unaligned_uint64 *) ptr;
  return dd->val;
}

static inline void grub_set_unaligned64 (void *ptr, grub_uint64_t val)
{
  struct grub_unaligned_uint64_t
  {
    grub_uint64_t d;
  } GRUB_PACKED;
  struct grub_unaligned_uint64_t *dd = (struct grub_unaligned_uint64_t *) ptr;
  dd->d = val;
}

#define GRUB_CHAR_BIT 8

#endif /* ! GRUB_TYPES_HEADER */
