/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2003,2007  Free Software Foundation, Inc.
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
#include <grub/dl.h>

	.file	"setjmp.S"

GRUB_MOD_LICENSE "GPLv3+"

	.text

/*
 * int grub_setjmp (grub_jmp_buf env)
 */
FUNCTION(grub_setjmp)
	movl	%ebx, 0(%eax)	/* EBX */
	movl	%esi, 4(%eax)	/* ESI */
	movl	%edi, 8(%eax)	/* EDI */
	movl	%ebp, 12(%eax)	/* EBP */
	popl	%ecx
	movl	%esp, 16(%eax)	/* ESP */
	movl	%ecx, 20(%eax)	/* EIP */
	xorl	%eax, %eax
	jmp	*%ecx


/*
 * int grub_longjmp (grub_jmp_buf env, int val)
 */
FUNCTION(grub_longjmp)
	movl	0(%eax), %ebx
	movl	4(%eax), %esi
	movl	8(%eax), %edi
	movl	12(%eax), %ebp
	movl	16(%eax), %esp
	movl	20(%eax), %ecx

	movl	%edx, %eax
	testl	%eax, %eax
	jnz	1f
	incl	%eax
1:	jmp	*%ecx

