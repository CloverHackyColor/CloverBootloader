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

#include <grub/ieee1275/ieee1275.h>
#include <grub/types.h>

/* Sun specific ieee1275 interfaces used by GRUB.  */

int
grub_ieee1275_claim_vaddr (grub_addr_t vaddr, grub_size_t size)
{
  struct claim_vaddr_args
  {
    struct grub_ieee1275_common_hdr common;
    grub_ieee1275_cell_t method;
    grub_ieee1275_cell_t ihandle;
    grub_ieee1275_cell_t align;
    grub_ieee1275_cell_t size;
    grub_ieee1275_cell_t virt;
    grub_ieee1275_cell_t catch_result;
  }
  args;

  INIT_IEEE1275_COMMON (&args.common, "call-method", 5, 2);
  args.method = (grub_ieee1275_cell_t) "claim";
  args.ihandle = grub_ieee1275_mmu;
  args.align = 0;
  args.size = size;
  args.virt = vaddr;
  args.catch_result = (grub_ieee1275_cell_t) -1;

  if (IEEE1275_CALL_ENTRY_FN (&args) == -1)
    return -1;
  return args.catch_result;
}

int
grub_ieee1275_alloc_physmem (grub_addr_t *paddr, grub_size_t size,
			     grub_uint32_t align)
{
  grub_uint32_t memory_ihandle;
  struct alloc_physmem_args
  {
    struct grub_ieee1275_common_hdr common;
    grub_ieee1275_cell_t method;
    grub_ieee1275_cell_t ihandle;
    grub_ieee1275_cell_t align;
    grub_ieee1275_cell_t size;
    grub_ieee1275_cell_t catch_result;
    grub_ieee1275_cell_t phys_high;
    grub_ieee1275_cell_t phys_low;
  }
  args;
  grub_ssize_t actual = 0;

  grub_ieee1275_get_property (grub_ieee1275_chosen, "memory",
			      &memory_ihandle, sizeof (memory_ihandle),
			      &actual);
  if (actual != sizeof (memory_ihandle))
    return -1;

  if (!align)
    align = 1;

  INIT_IEEE1275_COMMON (&args.common, "call-method", 4, 3);
  args.method = (grub_ieee1275_cell_t) "claim";
  args.ihandle = memory_ihandle;
  args.align = (align ? align : 1);
  args.size = size;
  if (IEEE1275_CALL_ENTRY_FN (&args) == -1)
    return -1;

  *paddr = args.phys_low;

  return args.catch_result;
}
