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

#include <grub/symbol.h>
#include <grub/xen.h>

	.p2align	4	/* force 16-byte alignment */

VARIABLE(grub_relocator_xen_remap_start)
LOCAL(base):
	/* mov imm32, %ebx */
	.byte	0xbb
VARIABLE(grub_relocator_xen_remapper_virt)
	.long	0

	/* mov imm32, %ecx */
	.byte	0xb9
VARIABLE(grub_relocator_xen_remapper_map)
	.long	0

	/* mov imm32, %edx */
	.byte	0xba
VARIABLE(grub_relocator_xen_remapper_map_high)
	.long	0

	movl	%ebx, %ebp

	movl    $2, %esi
	movl    $__HYPERVISOR_update_va_mapping, %eax
	int     $0x82

	movl	%ebp, %ebx
	addl   $(LOCAL(cont) - LOCAL(base)), %ebx

	jmp *%ebx

LOCAL(cont):
	xorl	%eax, %eax
	movl	%eax, %ebp
1:

	/* mov imm32, %eax */
	.byte	0xb8
VARIABLE(grub_relocator_xen_mfn_list)
	.long	0
	movl	%eax, %edi
	movl	%ebp, %eax
	movl    0(%edi, %eax, 4), %ecx

	/* mov imm32, %ebx */
	.byte	0xbb
VARIABLE(grub_relocator_xen_paging_start)
	.long	0
	shll	$12, %eax
	addl	%eax, %ebx
	movl    %ecx, %edx
	shll    $12,  %ecx
	shrl    $20,  %edx
	orl     $5, %ecx
	movl    $2, %esi
	movl    $__HYPERVISOR_update_va_mapping, %eax
	int     $0x82

	incl	%ebp
	/* mov imm32, %ecx */
	.byte	0xb9
VARIABLE(grub_relocator_xen_paging_size)
	.long	0
	cmpl	%ebp, %ecx

	ja	1b

	/* mov imm32, %ebx */
	.byte	0xbb
VARIABLE(grub_relocator_xen_mmu_op_addr)
	.long  0
	movl   $3, %ecx
	movl   $0, %edx
	movl   $0x7FF0, %esi
	movl   $__HYPERVISOR_mmuext_op, %eax
	int     $0x82

	/* mov imm32, %eax */
	.byte	0xb8
VARIABLE(grub_relocator_xen_remap_continue)
	.long	0

	jmp *%eax

VARIABLE(grub_relocator_xen_mmu_op)
	.space 256

VARIABLE(grub_relocator_xen_remap_end)


VARIABLE(grub_relocator_xen_start)
	/* mov imm32, %eax */
	.byte	0xb8
VARIABLE(grub_relocator_xen_remapper_virt2)
	.long	0

	movl    %eax, %edi

	xorl    %ecx, %ecx
	xorl    %edx, %edx

	movl    $2, %esi
	movl    $__HYPERVISOR_update_va_mapping, %eax
	int     $0x82


	/* mov imm32, %eax */
	.byte	0xb8
VARIABLE(grub_relocator_xen_stack)
	.long	0

	movl	%eax, %esp

	/* mov imm32, %eax */
	.byte	0xb8
VARIABLE(grub_relocator_xen_start_info)
	.long	0

	movl	%eax, %esi

	cld

	/* mov imm32, %eax */
	.byte	0xb8
VARIABLE(grub_relocator_xen_entry_point)
	.long	0

	jmp *%eax

VARIABLE(grub_relocator_xen_end)
