/* cache.S - Flush the processor cache for a specific region.  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2005,2007,2009  Free Software Foundation, Inc.
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

        .file   "cache.S"

        .text

/*
 * void grub_arch_sync_caches (void *address, grub_size_t len)
 */
FUNCTION(grub_arch_sync_caches)
	brz,pn		%o1, 2f
	 add		%o0, %o1, %o1
	add		%o1, 7, %o1
	andn		%o1, 7, %o1
	andn		%o0, 7, %o0
	sub		%o1, %o0, %o1
1:	subcc		%o1, 8, %o1
	bne,pt		%icc, 1b
	 flush		%o0 + %o1
2:	retl
	 nop

