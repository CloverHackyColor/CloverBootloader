/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2000,2001,2002,2003,2005,2006,2007,2008,2009 Free Software Foundation, Inc.
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

#include <config.h>
#include <grub/symbol.h>

#include <grub/machine/memory.h>
#include <grub/machine/kernel.h>

	.text
	.code32
	.globl _start
_start:
	jmp	codestart

	. = _start + GRUB_KERNEL_I386_QEMU_CORE_ENTRY_ADDR
VARIABLE(grub_core_entry_addr)
	.long	0

codestart:
	/* Relocate to low memory.  First we figure out our location.
	   We will derive the rom start address from it.  */
	call	1f
1:	popl	%esi

	/* Rom size is a multiple of 64 kiB.  With this we get the
	   value of `grub_core_entry_addr' in %esi.  */
	xorw	%si, %si

	movl    $(_edata - _start), %ecx
	movl	$_start, %edi
	cld
	rep
	movsb
	ljmp	$GRUB_MEMORY_MACHINE_PROT_MODE_CSEG, $1f
1:

	/* clean out the bss */
	movl	$BSS_START_SYMBOL, %edi

	/* compute the bss length */
	movl	$END_SYMBOL, %ecx
	subl	%edi, %ecx
		
	/* clean out */
	xorl	%eax, %eax
	cld
	rep
	stosb

	/*
	 *  Call the start of main body of C code.
	 */
	call	EXT_C(grub_main)

	/* This should never happen.  */
	cli
1:	
	hlt
	jmp 1b

#include "../realmode.S"
