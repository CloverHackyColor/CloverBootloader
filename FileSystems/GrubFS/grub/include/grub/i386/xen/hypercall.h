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

#include <grub/types.h>

int
EXPORT_FUNC (grub_xen_hypercall) (grub_uint32_t callno, grub_uint32_t a0,
				  grub_uint32_t a1, grub_uint32_t a2,
				  grub_uint32_t a3, grub_uint32_t a4,
				  grub_uint32_t a5)
__attribute__ ((regparm (3), cdecl));

static inline int
grub_xen_sched_op (int cmd, void *arg)
{
  return grub_xen_hypercall (__HYPERVISOR_sched_op, cmd, (grub_uint32_t) arg,
			     0, 0, 0, 0);
}

static inline int
grub_xen_mmu_update (const struct mmu_update *reqs,
		     unsigned count, unsigned *done_out, unsigned foreigndom)
{
  return grub_xen_hypercall (__HYPERVISOR_mmu_update, (grub_uint32_t) reqs,
			     (grub_uint32_t) count, (grub_uint32_t) done_out,
			     (grub_uint32_t) foreigndom, 0, 0);
}

static inline int
grub_xen_mmuext_op (mmuext_op_t * ops,
		    unsigned int count,
		    unsigned int *pdone, unsigned int foreigndom)
{
  return grub_xen_hypercall (__HYPERVISOR_mmuext_op, (grub_uint32_t) ops,
			     count, (grub_uint32_t) pdone, foreigndom, 0, 0);
}

static inline int
grub_xen_event_channel_op (int op, void *arg)
{
  return grub_xen_hypercall (__HYPERVISOR_event_channel_op, op,
			     (grub_uint32_t) arg, 0, 0, 0, 0);
}


static inline int
grub_xen_update_va_mapping (void *addr, uint64_t pte, uint32_t flags)
{
  return grub_xen_hypercall (__HYPERVISOR_update_va_mapping,
			     (grub_uint32_t) addr, pte, pte >> 32, flags, 0,
			     0);
}

static inline int
grub_xen_grant_table_op (int a, void *b, int c)
{
  return grub_xen_hypercall (__HYPERVISOR_grant_table_op, a,
			     (grub_uint32_t) b, c, 0, 0, 0);
}

static inline int
grub_xen_vm_assist (int cmd, int type)
{
  return grub_xen_hypercall (__HYPERVISOR_vm_assist, cmd, type, 0, 0, 0, 0);
}

#endif
