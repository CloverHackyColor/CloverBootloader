/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2009,2010  Free Software Foundation, Inc.
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

#define CODE32_SEGMENT	0x18
#define CODE_SEGMENT	0x08

/* The data segment of the protected mode.  */
#define DATA_SEGMENT	0x10

#include "relocator_common.S"

	.p2align	4	/* force 16-byte alignment */

VARIABLE(grub_relocator64_start)
	PREAMBLE
#ifndef __x86_64__
	DISABLE_PAGING

	/* Turn on PAE. */
	movl	%cr4, %eax
	orl	$(GRUB_MEMORY_CPU_CR4_PAE_ON | GRUB_MEMORY_CPU_CR4_PSE_ON), %eax
	movl	%eax, %cr4

	/* mov imm32, %eax */
	.byte	0xb8
VARIABLE(grub_relocator64_cr3)
	.long	0
	movl	%eax, %cr3

	/* Turn on amd64. */
	movl	$GRUB_MEMORY_CPU_AMD64_MSR, %ecx
	rdmsr
	orl	$GRUB_MEMORY_CPU_AMD64_MSR_ON, %eax
	wrmsr

	/* Enable paging. */
	movl	%cr0, %eax
	orl	$GRUB_MEMORY_CPU_CR0_PAGING_ON, %eax
	movl	%eax, %cr0

	RELOAD_GDT
#else
	/* mov imm64, %rax */
	.byte 	0x48
	.byte	0xb8
VARIABLE(grub_relocator64_cr3)
	.quad	0
	movq	%rax, %cr3
#endif

	.code64

	/* mov imm64, %rax */
	.byte 	0x48
	.byte	0xb8
VARIABLE(grub_relocator64_rsp)
	.quad	0

	movq	%rax, %rsp

	/* mov imm64, %rax */
	.byte 	0x48
	.byte	0xb8
VARIABLE(grub_relocator64_rsi)
	.quad	0

	movq	%rax, %rsi
	
	/* mov imm64, %rax */
	.byte 	0x48
	.byte	0xb8
VARIABLE(grub_relocator64_rax)
	.quad	0

	/* mov imm64, %rbx */
	.byte 	0x48
	.byte	0xbb
VARIABLE(grub_relocator64_rbx)
	.quad	0

	/* mov imm64, %rcx */
	.byte 	0x48
	.byte	0xb9
VARIABLE(grub_relocator64_rcx)
	.quad	0

	/* mov imm64, %rdx */
	.byte 	0x48
	.byte	0xba
VARIABLE(grub_relocator64_rdx)
	.quad	0

	/* Cleared direction flag is of no problem with any current
	   payload and makes this implementation easier.  */
	cld

#ifdef __APPLE__
	.byte 0xff, 0x25
	.quad 0
#else
	jmp *LOCAL(jump_addr) (%rip)
#endif

LOCAL(jump_addr):
VARIABLE(grub_relocator64_rip)
	.quad	0

#ifndef __x86_64__
	.p2align	4
LOCAL(gdt):
	/* NULL.  */
	.byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

	/* 64-bit segment.  */
	.word 0xffff /* Limit xffff.  */
	.word 0x0000 /* Base xxxx0000.  */
	.byte 0x00 /* Base xx00xxxx.  */
	.byte (0x8 /* Type 8.  */ | (1 << 4) /* Code.  */ \
	       | (0 << 5) /* Ring 0.  */ | (1 << 7) /* Present.  */)
	.byte (0xf /* Limit fxxxx.  */ | (0 << 4) /* AVL flag.  */ \
	       | (1 << 5) /* 64-bit.  */ | (0 << 6) \
	       | (1 << 7) /* 4K granular.  */)
	.byte 0x00 /* Base 00xxxxxx.  */

	/* Data segment*/
	.word 0xffff /* Limit xffff.  */
	.word 0x0000 /* Base xxxx0000.  */
	.byte 0x00 /* Base xx00xxxx.  */
	.byte (0x0 /* Type 0.  */ | (0 << 4) /* Data.  */ \
	       | (0 << 5) /* Ring 0.  */ | (1 << 7) /* Present.  */)
	.byte (0xf /* Limit fxxxx.  */ | (0 << 4) /* AVL flag.  */ \
	       | (0 << 5) /* Data.  */ | (0 << 6) \
	       | (1 << 7) /* 4K granular.  */)
	.byte 0x00 /* Base 00xxxxxx.  */

	/* Compatibility segment.  */
	.word 0xffff /* Limit xffff.  */
	.word 0x0000 /* Base xxxx0000.  */
	.byte 0x00 /* Base xx00xxxx.  */
	.byte (0x8 /* Type 8.  */ | (1 << 4) /* Code.  */  \
	       | (0 << 5) /* Ring 0.  */ | (1 << 7) /* Present.  */)
	.byte (0xf /* Limit fxxxx.  */ | (0 << 4) /* AVL flag.  */ \
	       | (0 << 5) /* 32-bit.  */ | (1 << 6) /* 32-bit.  */ \
	       | (1 << 7) /* 4K granular.  */)
	.byte 0x00 /* Base 00xxxxxx.  */

LOCAL(gdt_end):
#endif

VARIABLE(grub_relocator64_end)
