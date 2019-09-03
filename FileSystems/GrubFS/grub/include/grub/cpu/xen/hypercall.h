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

#ifndef GRUB_XEN_CPU_HYPERCALL_HEADER
#define GRUB_XEN_CPU_HYPERCALL_HEADER 1

#include <grub/misc.h>

int EXPORT_FUNC (grub_xen_sched_op) (int cmd, void *arg) GRUB_ASM_ATTR;
int grub_xen_update_va_mapping (void *addr, uint64_t pte, uint64_t flags) GRUB_ASM_ATTR;
int EXPORT_FUNC (grub_xen_event_channel_op) (int op, void *arg) GRUB_ASM_ATTR;

int grub_xen_mmuext_op (mmuext_op_t * ops,
			unsigned int count,
			unsigned int *pdone, unsigned int foreigndom) GRUB_ASM_ATTR;
int EXPORT_FUNC (grub_xen_mmu_update) (const struct mmu_update * reqs,
				       unsigned count, unsigned *done_out,
				       unsigned foreigndom) GRUB_ASM_ATTR;
int EXPORT_FUNC (grub_xen_grant_table_op) (int, void *, int) GRUB_ASM_ATTR;

#endif
