/* hypercall.S - wrappers for Xen hypercalls */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2011  Free Software Foundation, Inc.
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

#include <grub/symbol.h>
#include <grub/xen.h>

FUNCTION(grub_xen_sched_op)
	movq $__HYPERVISOR_sched_op, %rax
	syscall
	ret

FUNCTION(grub_xen_event_channel_op)
	movq $__HYPERVISOR_event_channel_op, %rax
	syscall
	ret

FUNCTION(grub_xen_update_va_mapping)
	movq $__HYPERVISOR_update_va_mapping, %rax
	syscall
	ret

FUNCTION(grub_xen_mmuext_op)
	movq %rcx, %r10
	movq $__HYPERVISOR_mmuext_op, %rax
	syscall
	ret

FUNCTION(grub_xen_grant_table_op)
	movq $__HYPERVISOR_grant_table_op, %rax
	syscall
	ret

FUNCTION(grub_xen_mmu_update)
	movq %rcx, %r10
	movq $__HYPERVISOR_mmu_update, %rax
	syscall
	ret
