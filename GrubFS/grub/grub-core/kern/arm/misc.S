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
#include <grub/dl.h>

	.file	"misc.S"
	.text
	.syntax	unified
#if !defined (__thumb2__)
	.arm
#else
	.thumb
#endif

	.align	2

FUNCTION(__muldi3)
FUNCTION(__aeabi_lmul)
        stmfd   sp!, {r4, fp}
        add     fp, sp, #4
        sub     sp, sp, #16
        str     r0, [fp, #-12]
        str     r1, [fp, #-8]
        str     r2, [fp, #-20]
        str     r3, [fp, #-16]
        ldr     r3, [fp, #-8]
        ldr     r2, [fp, #-20]
        mul     r2, r3, r2
        ldr     r3, [fp, #-16]
        ldr     r1, [fp, #-12]
        mul     r3, r1, r3
        add     r2, r2, r3
        ldr     r0, [fp, #-12]
        ldr     r1, [fp, #-20]
        umull   r3, r4, r0, r1
        add     r2, r2, r4
        mov     r4, r2
        mov     r0, r3
        mov     r1, r4
	mov     sp, fp
	sub     sp, sp, #4
        ldmfd   sp!, {r4, fp}
        bx      lr

	.macro division parent

        stmfd   sp!, {lr}
        sub     sp, sp, #12
        mov     r2, r1
        add     r1, sp, #4
        str     r1, [sp, #0]
        mov     r1, #0
        mov     r3, #0
        bl      \parent
	ldr     r1, [sp, #4]
        add     sp, sp, #12
        ldmfd   sp!, {lr}
        bx      lr
	.endm

FUNCTION(__aeabi_uidivmod)
	division grub_divmod64


/*
 * Null divide-by-zero handler
 */
FUNCTION(__aeabi_unwind_cpp_pr0)
FUNCTION(raise)
	mov	r0, #0
	bx	lr
	
	END
