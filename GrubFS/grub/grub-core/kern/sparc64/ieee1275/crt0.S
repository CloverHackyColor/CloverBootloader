/* crt0.S - Startup code for the Sparc64.  */
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
#include <grub/symbol.h>
#include <grub/machine/kernel.h>
#include <grub/offsets.h>

	.text
	.align	4
	.globl	_start
_start:
	ba	codestart
	 mov  %o4, %o0

	. = EXT_C(_start) + GRUB_KERNEL_MACHINE_TOTAL_MODULE_SIZE

VARIABLE(grub_total_module_size)
	.word	0

codestart:
	/* Copy the modules past the end of the kernel image.
	 * They are currently sitting in the BSS.
	 */
	sethi	%hi(__bss_start + GRUB_KERNEL_SPARC64_IEEE1275_MOD_ALIGN - 1), %o2
	or	%o2, %lo(__bss_start + GRUB_KERNEL_SPARC64_IEEE1275_MOD_ALIGN - 1), %o2
	srl	%o2, GRUB_KERNEL_SPARC64_IEEE1275_LOG_MOD_ALIGN, %o2
	sll	%o2, GRUB_KERNEL_SPARC64_IEEE1275_LOG_MOD_ALIGN, %o2
	sethi	%hi(_end + GRUB_KERNEL_SPARC64_IEEE1275_MOD_ALIGN - 1), %o3
	or	%o3, %lo(_end + GRUB_KERNEL_SPARC64_IEEE1275_MOD_ALIGN - 1), %o3
	srl	%o3, GRUB_KERNEL_SPARC64_IEEE1275_LOG_MOD_ALIGN, %o3
	sll	%o3, GRUB_KERNEL_SPARC64_IEEE1275_LOG_MOD_ALIGN, %o3
	sethi	%hi(grub_total_module_size), %o4
	lduw	[%o4 + %lo(grub_total_module_size)], %o4

	add     %o2, %o4, %o2
	add     %o3, %o4, %o3

	/* Save ieee1275 stack for future use by booter.  */
	mov     %o6, %o1
	/* Our future stack.  */
	sethi	%hi(GRUB_KERNEL_MACHINE_STACK_SIZE), %o5
	or	%o5, %lo(GRUB_KERNEL_MACHINE_STACK_SIZE), %o5
	add     %o3, %o5, %o6
	and     %o6, ~0xff, %o6
	sub     %o6, 2047, %o6
	
	sub	%o2, 4, %o2
	sub	%o3, 4, %o3
1:	lduw	[%o2], %o5
	stw	%o5, [%o3]
	subcc	%o4, 4, %o4
	sub	%o2, 4, %o2
	bne,pt	%icc, 1b
	 sub	%o3, 4, %o3

	/* Now it's safe to clear out the BSS.  */
	sethi	%hi(__bss_start), %o2
	or	%o2, %lo(__bss_start), %o2
1:	stb	%g0, [%o2]
	add	%o2, 1, %o2
	and	%o2, 7, %o3
	brnz	%o3, 1b
	 nop

	sethi	%hi(_end - 1), %o3
	or	%o3, %lo(_end - 1), %o3
	srl	%o3, 3, %o3
	sll	%o3, 3, %o3
1:	stx	%g0, [%o2]
	add	%o2, 8, %o2
	cmp	%o2, %o3
	blt,pt	%xcc, 1b
	 nop

	sethi	%hi(_end), %o3
	or	%o3, %lo(_end), %o3
1:	stb	%g0, [%o2]
	add	%o2, 1, %o2
	cmp	%o2, %o3
	blt,pt	%xcc, 1b
	 nop

	sethi	%hi(grub_ieee1275_original_stack), %o2
	stx	%o1, [%o2 + %lo(grub_ieee1275_original_stack)]
	sethi	%hi(grub_ieee1275_entry_fn), %o2
	call	grub_main
	 stx	%o0, [%o2 + %lo(grub_ieee1275_entry_fn)]
1:	ba,a	1b
	 nop
