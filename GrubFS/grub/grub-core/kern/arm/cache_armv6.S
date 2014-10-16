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

	.file	"cache_armv6.S"
	.text
	.syntax	unified
	.arm

# define DMB	mcr	p15, 0, r0, c7, c10, 5
# define DSB	mcr	p15, 0, r0, c7, c10, 4
# define ISB	mcr	p15, 0, r0, c7, c5, 4
#define ARMV6 1

clean_invalidate_dcache:
	mcr	p15, 0, r0, c7, c14, 0	@ Clean/Invalidate D-cache
	bx	lr

#include "cache.S"

FUNCTION(grub_arm_main_id)
	mrc 	p15, 0, r0, c0, c0, 0
	bx lr

FUNCTION(grub_arm_cache_type)
	mrc 	p15, 0, r0, c0, c0, 1
	bx lr

FUNCTION(grub_arm_clear_mmu_v6)
	mov	r0, #0
	mcr	p15, 0, r0, c2, c0, 2
	bx	lr

FUNCTION(grub_arm_enable_mmu)
	mcr	p15, 0, r0, c2, c0, 0

	mvn	r0, #0
	mcr	p15, 0, r0, c3, c0, 0

	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #(1 << 23)
	mcr	p15, 0, r0, c1, c0, 0

	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #(1 << 0)
	mcr	p15, 0, r0, c1, c0, 0

	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #(1 << 2)
	mcr	p15, 0, r0, c1, c0, 0

	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #(1 << 12)
	mcr	p15, 0, r0, c1, c0, 0

	bx	lr
