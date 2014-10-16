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

#ifndef GRUB_CPU_MACHO_H
#define GRUB_CPU_MACHO_H 1

#include <grub/macho.h>

#define GRUB_MACHO_CPUTYPE_IS_HOST32(x) ((x) == GRUB_MACHO_CPUTYPE_IA32)
#define GRUB_MACHO_CPUTYPE_IS_HOST64(x) ((x) == GRUB_MACHO_CPUTYPE_AMD64)
#ifdef __x86_64__
#define GRUB_MACHO_CPUTYPE_IS_HOST_CURRENT(x) ((x) == GRUB_MACHO_CPUTYPE_AMD64)
#else
#define GRUB_MACHO_CPUTYPE_IS_HOST_CURRENT(x) ((x) == GRUB_MACHO_CPUTYPE_IA32)
#endif

#endif
