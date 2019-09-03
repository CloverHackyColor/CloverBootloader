/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2009, 2010  Free Software Foundation, Inc.
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

#ifndef GRUB_POSIX_SYS_TYPES_H
#define GRUB_POSIX_SYS_TYPES_H	1

#include <grub/misc.h>

#include <stddef.h>

typedef grub_ssize_t ssize_t;
#ifndef GRUB_POSIX_BOOL_DEFINED
typedef enum { false = 0, true = 1 } bool;
#define GRUB_POSIX_BOOL_DEFINED 1
#endif

typedef grub_uint8_t uint8_t;
typedef grub_uint16_t uint16_t;
typedef grub_uint32_t uint32_t;
typedef grub_uint64_t uint64_t;

typedef grub_int8_t int8_t;
typedef grub_int16_t int16_t;
typedef grub_int32_t int32_t;
typedef grub_int64_t int64_t;

#define HAVE_U64_TYPEDEF 1
typedef grub_uint64_t u64;
#define HAVE_U32_TYPEDEF 1
typedef grub_uint32_t u32;
#define HAVE_U16_TYPEDEF 1
typedef grub_uint16_t u16;
#define HAVE_BYTE_TYPEDEF 1
typedef grub_uint8_t byte;

typedef grub_addr_t uintptr_t;

#define SIZEOF_UNSIGNED_LONG GRUB_CPU_SIZEOF_LONG
#define SIZEOF_UNSIGNED_INT 4
#define SIZEOF_UNSIGNED_LONG_LONG 8
#define SIZEOF_UNSIGNED_SHORT 2
#define SIZEOF_UINT64_T 8

#ifdef GRUB_CPU_WORDS_BIGENDIAN
#define WORDS_BIGENDIAN 1
#else
#undef WORDS_BIGENDIAN
#endif

#endif
