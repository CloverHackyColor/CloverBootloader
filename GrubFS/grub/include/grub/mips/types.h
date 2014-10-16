/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2006,2007,2009  Free Software Foundation, Inc.
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

#ifndef GRUB_TYPES_CPU_HEADER
#define GRUB_TYPES_CPU_HEADER	1

/* The size of void *.  */
#define GRUB_TARGET_SIZEOF_VOID_P	4

/* The size of long.  */
#define GRUB_TARGET_SIZEOF_LONG		4

#ifdef GRUB_CPU_MIPSEL
/* mipsEL is little-endian.  */
#undef GRUB_TARGET_WORDS_BIGENDIAN
#elif defined (GRUB_CPU_MIPS)
/* mips is big-endian.  */
#define GRUB_TARGET_WORDS_BIGENDIAN
#elif !defined (GRUB_SYMBOL_GENERATOR)
#error Neither GRUB_CPU_MIPS nor GRUB_CPU_MIPSEL is defined
#endif

#endif /* ! GRUB_TYPES_CPU_HEADER */
