/* This is a pseudo config.h so that types.h compiles nicely */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2009  Free Software Foundation, Inc.
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

#define GRUB_TYPES_CPU_HEADER	1

#ifdef __i386__
# define SIZEOF_VOID_P	4
# define SIZEOF_LONG	4
# define GRUB_TARGET_SIZEOF_VOID_P	4
# define GRUB_TARGET_SIZEOF_LONG	4
# define EFI_FUNC(x) x
#elif defined (__x86_64__)
# define SIZEOF_VOID_P	8
# define SIZEOF_LONG	8
# define GRUB_TARGET_SIZEOF_VOID_P	8
# define GRUB_TARGET_SIZEOF_LONG	8
# define EFI_FUNC(x) x ## _real
#else
#error "Unknown architecture"
#endif
