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
 *  jmp_buf:
 *   rbx rsp rbp r12 r13 r14 r15 rip
 *   0   8   16  24  32  40  48  56
 */

/*
 * int grub_setjmp (grub_jmp_buf env)
 */
FUNCTION(grub_setjmp)
	pop	%rsi		/* Return address, and adjust the stack */
	xorq	%rax, %rax
	movq	%rbx, 0(%rdi)	/* RBX */
	movq	%rsp, 8(%rdi)   /* RSP */
	push	%rsi
	movq	%rbp, 16(%rdi)	/* RBP */
	movq	%r12, 24(%rdi)	/* R12 */
	movq	%r13, 32(%rdi)	/* R13 */
	movq	%r14, 40(%rdi)	/* R14 */
	movq	%r15, 48(%rdi)	/* R15 */
	movq	%rsi, 56(%rdi)	/* RSI */
	ret

/*
 * int grub_longjmp (grub_jmp_buf env, int val)
 */
FUNCTION(grub_longjmp)
	movl	%esi, %eax
	orl	%eax, %eax
	jnz	1f
	incl	%eax
1:

	movq	(%rdi), %rbx
	movq	8(%rdi), %rsp
	movq	16(%rdi), %rbp
	movq	24(%rdi), %r12
	movq    32(%rdi), %r13
	movq    40(%rdi), %r14
	movq    48(%rdi), %r15
	jmp	*56(%rdi)
