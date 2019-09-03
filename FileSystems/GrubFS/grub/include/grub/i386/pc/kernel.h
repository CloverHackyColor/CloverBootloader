/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2003,2007,2008  Free Software Foundation, Inc.
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

#ifndef KERNEL_MACHINE_HEADER
#define KERNEL_MACHINE_HEADER	1

#include <grub/offsets.h>

/* Enable LZMA compression */
#define ENABLE_LZMA	1

#ifndef ASM_FILE

#include <grub/symbol.h>
#include <grub/types.h>

/* The total size of module images following the kernel.  */
extern grub_int32_t grub_total_module_size;

extern grub_uint32_t EXPORT_VAR(grub_boot_device);

extern void (*EXPORT_VAR(grub_pc_net_config)) (char **device, char **path);

#endif /* ! ASM_FILE */

#endif /* ! KERNEL_MACHINE_HEADER */
