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


#include <grub/symbol.h>
#include <grub/i386/memory.h>

#ifdef __x86_64__
#define RAX %rax
#define RSI %rsi
#else
#define RAX %eax
#define RSI %esi
#endif

	.macro DISABLE_PAGING
#ifdef GRUB_MACHINE_IEEE1275
#endif

	movl	%cr0, %eax
	andl	$(~GRUB_MEMORY_CPU_CR0_PAGING_ON), %eax
	movl	%eax, %cr0
	.endm

	.macro PREAMBLE
LOCAL(base):
	/* %rax contains now our new 'base'.  */
	mov	RAX, RSI

#if defined (__APPLE__) && defined (__x86_64__)
	leaq	LOCAL(cont0) (%rip), RAX
#elif defined (__APPLE__)
	LOCAL(cont0_offset) = LOCAL(cont0) - LOCAL(base)
	add	$LOCAL(cont0_offset), RAX
#else
	add	$(LOCAL(cont0) - LOCAL(base)), RAX
#endif
	jmp	*RAX
LOCAL(cont0):
	.endm

	.macro RELOAD_GDT
#ifdef __APPLE__
	LOCAL(cont1_offset) = LOCAL(cont1) - LOCAL(base)
	LOCAL(jump_vector_offset) = LOCAL(jump_vector) - LOCAL(base)
	LOCAL(gdt_offset) = LOCAL(gdt) - LOCAL(base)
	LOCAL(gdt_addr_offset) = LOCAL(gdt_addr) - LOCAL(base)
	LOCAL(gdtdesc_offset) = LOCAL(gdtdesc) - LOCAL(base)

	lea	LOCAL(cont1_offset) (RSI, 1), RAX
	movl	%eax, LOCAL(jump_vector_offset) (RSI, 1)

	lea	LOCAL(gdt_offset) (RSI, 1), RAX
	mov	RAX, (LOCAL(gdt_addr_offset)) (RSI, 1)
	
	/* Switch to compatibility mode. */
	lgdt	(LOCAL(gdtdesc_offset)) (RSI, 1)

	/* Update %cs.  */
	ljmp	*(LOCAL(jump_vector_offset)) (RSI, 1)
	.p2align	4
LOCAL(gdtdesc):
	LOCAL(gdtsize) = LOCAL(gdt_end) - LOCAL(gdt)
	.word	LOCAL(gdtsize)
#else
	lea	(LOCAL(cont1) - LOCAL(base)) (RSI, 1), RAX
	movl	%eax, (LOCAL(jump_vector) - LOCAL(base)) (RSI, 1)

	lea	(LOCAL(gdt) - LOCAL(base)) (RSI, 1), RAX
	mov	RAX, (LOCAL(gdt_addr) - LOCAL(base)) (RSI, 1)
	
	/* Switch to compatibility mode. */
	lgdt	(LOCAL(gdtdesc) - LOCAL(base)) (RSI, 1)

	/* Update %cs.  */
	ljmp	*(LOCAL(jump_vector) - LOCAL(base)) (RSI, 1)

	.p2align	4
LOCAL(gdtdesc):
	.word	LOCAL(gdt_end) - LOCAL(gdt)
#endif
LOCAL(gdt_addr):
#ifdef __x86_64__
	/* Filled by the code. */
	.quad	0
#else
	/* Filled by the code. */
	.long	0
#endif

	.p2align	4
LOCAL(jump_vector):
	/* Jump location. Is filled by the code */
	.long	0
	.long	CODE_SEGMENT
	
LOCAL(cont1):
	.endm
