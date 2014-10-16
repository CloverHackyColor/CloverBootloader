/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2008  Free Software Foundation, Inc.
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
#include <grub/offsets.h>

	.text
	.psr abi64
	.psr lsb
	.lsb

	.global _start
	.proc _start
_start:
	alloc loc0=ar.pfs,2,4,0,0
	mov loc1=rp
	addl loc2=@gprel(grub_efi_image_handle),gp
	addl loc3=@gprel(grub_efi_system_table),gp
	;;
	st8 [loc2]=in0
	st8 [loc3]=in1
	br.call.sptk.few rp=grub_main
	;;
	mov ar.pfs=loc0
	mov rp=loc1
	;;
	br.ret.sptk.few rp

	.endp _start
