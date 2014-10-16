/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2003,2004,2007,2008,2009  Free Software Foundation, Inc.
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

#ifndef GRUB_MULTIBOOT_CPU_HEADER
#define GRUB_MULTIBOOT_CPU_HEADER	1

#define MULTIBOOT_INITIAL_STATE  { .gpr[4] = MULTIBOOT_BOOTLOADER_MAGIC, \
    .jumpreg = 1 }
#define MULTIBOOT_ENTRY_REGISTER gpr[1]
#define MULTIBOOT_MBI_REGISTER gpr[5]
#define MULTIBOOT_ARCHITECTURE_CURRENT MULTIBOOT_ARCHITECTURE_MIPS32

#define MULTIBOOT_ELF32_MACHINE EM_MIPS
#define MULTIBOOT_ELF64_MACHINE EM_MIPS

#endif /* ! GRUB_MULTIBOOT_CPU_HEADER */
