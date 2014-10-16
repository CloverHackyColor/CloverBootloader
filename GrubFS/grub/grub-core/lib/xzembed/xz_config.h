/* xz_config.h - Private includes and definitions for userspace use */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2010  Free Software Foundation, Inc.
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
/*
 * This file is based on code from XZ embedded project
 * http://tukaani.org/xz/embedded.html
 */

#ifndef XZ_CONFIG_H
#define XZ_CONFIG_H

#define uint8_t UINT8
#define uint16_t UINT16
#define uint32_t UINT32
#define uint64_t UINT64
#define size_t UINTN

/* Enable BCJ filter decoders. */

#ifndef GRUB_EMBED_DECOMPRESSOR

#define XZ_DEC_X86
#define XZ_DEC_POWERPC
#define XZ_DEC_IA64
#define XZ_DEC_ARM
#define XZ_DEC_ARMTHUMB
#define XZ_DEC_SPARC

#else

#if defined(__i386__) || defined(__x86_64__) || defined(MDE_CPU_IA32) || defined(MDE_CPU_X64) || defined(MDE_CPU_EBC)
  #define XZ_DEC_X86
#endif

#ifdef __powerpc__
  #define XZ_DEC_POWERPC
#endif

#if defined(__ia64__) || defined(MDE_CPU_IPF)
  #define XZ_DEC_IA64
#endif

#if defined(__arm__) || defined(MDE_CPU_ARM) || defined(MDE_CPU_AARCH64)
  #define XZ_DEC_ARM
#endif

#if defined(__arm__) || defined(MDE_CPU_ARM) || defined(MDE_CPU_AARCH64)
  #define XZ_DEC_ARMTHUMB
#endif

#ifdef __sparc__
  #define XZ_DEC_SPARC
#endif
#endif

#include "xz.h"

#define realloc(p, s) ReallocatePool ((UINTN)s, (UINTN)s, p)
#define kmalloc(size, flags) AllocateZeroPool(size)
#define kfree(ptr) if (ptr != NULL) { FreePool(ptr); ptr = NULL; }
#define vmalloc(size) AllocateZeroPool(size)
#define vfree(ptr) if (ptr != NULL) { FreePool(ptr); ptr = NULL; }

#define memeq(a, b, size) (CompareMem(a, b, size) == 0)
#define memzero(buf, size) CopyMem(buf, 0, size)
#define memcpy(tbuf, buf, size) CopyMem(tbuf, buf, size)

#define min(x, y) ((x) < (y) ? (x) : (y))
#define min_t(type, x, y) min(x, y)

/*
 * Some functions have been marked with __always_inline to keep the
 * performance reasonable even when the compiler is optimizing for
 * small code size. You may be able to save a few bytes by #defining
 * __always_inline to plain inline, but don't complain if the code
 * becomes slow.
 *
 * NOTE: System headers on GNU/Linux may #define this macro already,
 * so if you want to change it, it you need to #undef it first.
 */
#ifndef __always_inline
#	ifdef __GNUC__
#		define __always_inline \
			inline __attribute__((__always_inline__))
#	else
#		define __always_inline inline
#	endif
#endif

/*
 * Some functions are marked to never be inlined to reduce stack usage.
 * If you don't care about stack usage, you may want to modify this so
 * that noinline_for_stack is #defined to be empty even when using GCC.
 * Doing so may save a few bytes in binary size.
 */
#ifndef noinline_for_stack
#	ifdef __GNUC__
#		define noinline_for_stack __attribute__((__noinline__))
#	else
#		define noinline_for_stack
#	endif
#endif

/* Inline functions to access unaligned unsigned 32-bit integers */
static inline uint32_t get_unaligned_le32(const uint8_t *buf)
{
	return (uint32_t)buf[0]
			| ((uint32_t)buf[1] << 8)
			| ((uint32_t)buf[2] << 16)
			| ((uint32_t)buf[3] << 24);
}

static inline uint32_t get_unaligned_be32(const uint8_t *buf)
{
	return (uint32_t)(buf[0] << 24)
			| ((uint32_t)buf[1] << 16)
			| ((uint32_t)buf[2] << 8)
			| (uint32_t)buf[3];
}

static inline void put_unaligned_le32(uint32_t val, uint8_t *buf)
{
	buf[0] = (uint8_t)val;
	buf[1] = (uint8_t)(val >> 8);
	buf[2] = (uint8_t)(val >> 16);
	buf[3] = (uint8_t)(val >> 24);
}

static inline void put_unaligned_be32(uint32_t val, uint8_t *buf)
{
	buf[0] = (uint8_t)(val >> 24);
	buf[1] = (uint8_t)(val >> 16);
	buf[2] = (uint8_t)(val >> 8);
	buf[3] = (uint8_t)val;
}

/*
 * Use get_unaligned_le32() also for aligned access for simplicity. On
 * little endian systems, #define get_le32(ptr) (*(const uint32_t *)(ptr))
 * could save a few bytes in code size.
 */
#define get_le32 get_unaligned_le32

#endif
