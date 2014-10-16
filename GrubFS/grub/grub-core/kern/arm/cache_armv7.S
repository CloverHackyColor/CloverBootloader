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

	.file	"cache_armv7.S"
	.text
	.syntax	unified
#if !defined (__thumb2__)
	.arch	armv7a
	.arm
#else
	.arch	armv7
	.thumb
#endif
# define DMB	dmb
# define DSB	dsb
# define ISB	isb
#define ARMV7 1

	@ r0  - CLIDR
	@ r1  - LoC
	@ r2  - current level
	@ r3  - num sets
	@ r4  - num ways
	@ r5  - current set
	@ r6  - current way
	@ r7  - line size
	@ r8  - scratch
	@ r9  - scratch
	@ r10 - scratch
	@ r11 - scratch
clean_invalidate_dcache:
	push	{r4-r12, lr}
	mrc 	p15, 1, r0, c0, c0, 1	@ Read CLIDR
	lsr	r1, r0, #24		@ Extract LoC
	and	r1, r1, #0x7

	mov	r2, #0			@ First level, L1
2:	and	r8, r0, #7		@ cache type at current level
	cmp	r8, #2
	blt	5f			@ instruction only, or none, skip level

	@ set current cache level/type (for CCSIDR read)
	lsl	r8, r2, #1
	mcr	p15, 2, r8, c0, c0, 0	@ Write CSSELR (level, type: data/uni)

	@ read current cache information
	mrc	p15, 1, r8, c0, c0, 0	@ Read CCSIDR
	lsr	r3, r8, #13		@ Number of sets -1

	@ Keep only 14 bits of r3
	lsl     r3, r3, #18
	lsr     r3, r3, #18

	lsr	r4, r8, #3		@ Number of ways -1

	@ Keep only 9  bits of r4
	lsl     r4, r4, #23
	lsr     r4, r4, #23

	and	r7, r8, #7		@ log2(line size in words) - 2
	add	r7, r7, #2		@  adjust
	mov	r8, #1
	lsl	r7, r8, r7		@  -> line size in words
	lsl	r7, r7, #2		@  -> bytes

	@ set loop
	mov	r5, #0			@ current set = 0
3:	lsl	r8, r2, #1		@ insert level
	clz	r9, r7			@ calculate set field offset
	mov	r10, #31
	sub	r9, r10, r9
	lsl	r10, r5, r9
	orr	r8, r8, r10		@ insert set field

	@ way loop
	@ calculate way field offset
	mov	r6, #0			@ current way = 0
	add	r10, r4, #1
	clz	r9, r10			@ r9 = way field offset
	add	r9, r9, #1
4:	lsl	r10, r6, r9
	orr	r11, r8, r10		@ insert way field

	@ clean and invalidate line by set/way
	mcr	p15, 0, r11, c7, c14, 2	@ DCCISW

	@ next way
	add	r6, r6, #1
	cmp	r6, r4
	ble	4b

	@ next set
	add	r5, r5, #1
	cmp	r5, r3
	ble	3b

	@ next level
5:	lsr	r0, r0, #3		@ align next level CLIDR 'type' field
	add	r2, r2, #1		@ increment cache level counter
	cmp	r2, r1
	blt	2b			@ outer loop

	@ return
6:	DSB
	ISB
	pop	{r4-r12, lr}
	bx	lr

#include "cache.S"