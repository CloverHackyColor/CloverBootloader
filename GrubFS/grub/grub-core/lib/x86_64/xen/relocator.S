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
	/* mov imm64, %rax */
	.byte 	0x48
	.byte	0xb8
VARIABLE(grub_relocator_xen_remapper_virt)
	.quad	0

	movq    %rax, %rdi
	movq    %rax, %rbx

	/* mov imm64, %rax */
	.byte 	0x48
	.byte	0xb8
VARIABLE(grub_relocator_xen_remapper_map)
	.quad	0

	movq    %rax, %rsi

	movq    $2, %rdx
	movq    $__HYPERVISOR_update_va_mapping, %rax
	syscall

	addq   $(LOCAL(cont) - LOCAL(base)), %rbx

	jmp *%rbx

LOCAL(cont):
	
	/* mov imm64, %rcx */
	.byte 	0x48
	.byte	0xb9
VARIABLE(grub_relocator_xen_paging_size)
	.quad	0

	/* mov imm64, %rax */
	.byte 	0x48
	.byte	0xb8
VARIABLE(grub_relocator_xen_paging_start)
	.quad	0

	movq	%rax, %r12

	/* mov imm64, %rax */
	.byte 	0x48
	.byte	0xb8
VARIABLE(grub_relocator_xen_mfn_list)
	.quad	0

	movq	%rax, %rsi
1:
	movq	%r12, %rdi
	movq    %rsi, %rbx
	movq    0(%rsi), %rsi
	shlq    $12,  %rsi
	orq     $5, %rsi
	movq    $2, %rdx
	movq    %rcx, %r9
	movq    $__HYPERVISOR_update_va_mapping, %rax
	syscall

	movq    %r9, %rcx
	addq    $8, %rbx
	addq    $4096, %r12
	movq    %rbx, %rsi

	loop 1b

	leaq   EXT_C(grub_relocator_xen_mmu_op) (%rip), %rdi
	movq   $3, %rsi
	movq   $0, %rdx
	movq   $0x7FF0, %r10
	movq   $__HYPERVISOR_mmuext_op, %rax
	syscall

	/* mov imm64, %rax */
	.byte 	0x48
	.byte	0xb8
VARIABLE(grub_relocator_xen_remap_continue)
	.quad	0

	jmp *%rax

VARIABLE(grub_relocator_xen_mmu_op)
	.space 256

VARIABLE(grub_relocator_xen_remap_end)


VARIABLE(grub_relocator_xen_start)
	/* mov imm64, %rax */
	.byte 	0x48
	.byte	0xb8
VARIABLE(grub_relocator_xen_remapper_virt2)
	.quad	0

	movq    %rax, %rdi

	xorq    %rax, %rax
	movq    %rax, %rsi

	movq    $2, %rdx
	movq    $__HYPERVISOR_update_va_mapping, %rax
	syscall


	/* mov imm64, %rax */
	.byte 	0x48
	.byte	0xb8
VARIABLE(grub_relocator_xen_stack)
	.quad	0

	movq	%rax, %rsp

	/* mov imm64, %rax */
	.byte 	0x48
	.byte	0xb8
VARIABLE(grub_relocator_xen_start_info)
	.quad	0

	movq	%rax, %rsi

	cld

	/* mov imm64, %rax */
	.byte 	0x48
	.byte	0xb8
VARIABLE(grub_relocator_xen_entry_point)
	.quad	0

	jmp *%rax

VARIABLE(grub_relocator_xen_end)
