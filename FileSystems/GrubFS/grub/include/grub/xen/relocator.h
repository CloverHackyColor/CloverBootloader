/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2013  Free Software Foundation, Inc.
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

#ifndef GRUB_RELOCATOR_XEN_HEADER
#define GRUB_RELOCATOR_XEN_HEADER	1

#include <grub/types.h>
#include <grub/err.h>
#include <grub/relocator.h>

struct grub_relocator_xen_state
{
  grub_addr_t start_info;
  grub_addr_t paging_start;
  grub_addr_t paging_size;
  grub_addr_t mfn_list;
  grub_addr_t stack;
  grub_addr_t entry_point;
};

grub_err_t
grub_relocator_xen_boot (struct grub_relocator *rel,
			 struct grub_relocator_xen_state state,
			 grub_uint64_t remapper_pfn,
			 grub_addr_t remapper_virt,
			 grub_uint64_t trampoline_pfn,
			 grub_addr_t trampoline_virt);

#endif
