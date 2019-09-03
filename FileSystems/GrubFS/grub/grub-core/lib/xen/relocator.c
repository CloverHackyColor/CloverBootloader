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

#include <grub/mm.h>
#include <grub/misc.h>

#include <grub/types.h>
#include <grub/err.h>
#include <grub/term.h>
#include <grub/xen.h>

#include <grub/xen/relocator.h>
#include <grub/relocator_private.h>

typedef grub_addr_t grub_xen_reg_t;

extern grub_uint8_t grub_relocator_xen_start;
extern grub_uint8_t grub_relocator_xen_end;
extern grub_uint8_t grub_relocator_xen_remap_start;
extern grub_uint8_t grub_relocator_xen_remap_end;
extern grub_xen_reg_t grub_relocator_xen_stack;
extern grub_xen_reg_t grub_relocator_xen_start_info;
extern grub_xen_reg_t grub_relocator_xen_entry_point;
extern grub_xen_reg_t grub_relocator_xen_paging_start;
extern grub_xen_reg_t grub_relocator_xen_paging_size;
extern grub_xen_reg_t grub_relocator_xen_remapper_virt;
extern grub_xen_reg_t grub_relocator_xen_remapper_virt2;
extern grub_xen_reg_t grub_relocator_xen_remapper_map;
extern grub_xen_reg_t grub_relocator_xen_mfn_list;
extern grub_xen_reg_t grub_relocator_xen_remap_continue;
#ifdef __i386__
extern grub_xen_reg_t grub_relocator_xen_mmu_op_addr;
extern grub_xen_reg_t grub_relocator_xen_remapper_map_high;
#endif
extern mmuext_op_t grub_relocator_xen_mmu_op[3];

#define RELOCATOR_SIZEOF(x)	(&grub_relocator##x##_end - &grub_relocator##x##_start)

grub_err_t
grub_relocator_xen_boot (struct grub_relocator *rel,
			 struct grub_relocator_xen_state state,
			 grub_uint64_t remapper_pfn,
			 grub_addr_t remapper_virt,
			 grub_uint64_t trampoline_pfn,
			 grub_addr_t trampoline_virt)
{
  grub_err_t err;
  void *relst;
  grub_relocator_chunk_t ch, ch_tramp;
  grub_xen_mfn_t *mfn_list =
    (grub_xen_mfn_t *) grub_xen_start_page_addr->mfn_list;

  err = grub_relocator_alloc_chunk_addr (rel, &ch, remapper_pfn << 12,
					 RELOCATOR_SIZEOF (_xen_remap));
  if (err)
    return err;
  err = grub_relocator_alloc_chunk_addr (rel, &ch_tramp, trampoline_pfn << 12,
					 RELOCATOR_SIZEOF (_xen));
  if (err)
    return err;

  grub_relocator_xen_stack = state.stack;
  grub_relocator_xen_start_info = state.start_info;
  grub_relocator_xen_entry_point = state.entry_point;
  grub_relocator_xen_paging_start = state.paging_start << 12;
  grub_relocator_xen_paging_size = state.paging_size;
  grub_relocator_xen_remapper_virt = remapper_virt;
  grub_relocator_xen_remapper_virt2 = remapper_virt;
  grub_relocator_xen_remap_continue = trampoline_virt;

  grub_relocator_xen_remapper_map = (mfn_list[remapper_pfn] << 12) | 5;
#ifdef __i386__
  grub_relocator_xen_remapper_map_high = (mfn_list[remapper_pfn] >> 20);
  grub_relocator_xen_mmu_op_addr = (char *) &grub_relocator_xen_mmu_op
    - (char *) &grub_relocator_xen_remap_start + remapper_virt;
#endif

  grub_relocator_xen_mfn_list = state.mfn_list
    + state.paging_start * sizeof (grub_addr_t);

  grub_memset (grub_relocator_xen_mmu_op, 0,
	       sizeof (grub_relocator_xen_mmu_op));
#ifdef __i386__
  grub_relocator_xen_mmu_op[0].cmd = MMUEXT_PIN_L3_TABLE;
#else
  grub_relocator_xen_mmu_op[0].cmd = MMUEXT_PIN_L4_TABLE;
#endif
  grub_relocator_xen_mmu_op[0].arg1.mfn = mfn_list[state.paging_start];
  grub_relocator_xen_mmu_op[1].cmd = MMUEXT_NEW_BASEPTR;
  grub_relocator_xen_mmu_op[1].arg1.mfn = mfn_list[state.paging_start];
  grub_relocator_xen_mmu_op[2].cmd = MMUEXT_UNPIN_TABLE;
  grub_relocator_xen_mmu_op[2].arg1.mfn =
    mfn_list[grub_xen_start_page_addr->pt_base >> 12];

  grub_memmove (get_virtual_current_address (ch),
		&grub_relocator_xen_remap_start,
		RELOCATOR_SIZEOF (_xen_remap));
  grub_memmove (get_virtual_current_address (ch_tramp),
		&grub_relocator_xen_start, RELOCATOR_SIZEOF (_xen));

  err = grub_relocator_prepare_relocs (rel, get_physical_target_address (ch),
				       &relst, NULL);
  if (err)
    return err;

  ((void (*)(void)) relst) ();

  /* Not reached.  */
  return GRUB_ERR_NONE;
}
