/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2009   Free Software Foundation, Inc.
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

#ifndef GRUB_BOOT_MACHINE_HEADER
#define GRUB_BOOT_MACHINE_HEADER	1

#define CIF_REG				%l0
#define CHOSEN_NODE_REG			%l4
#define STDOUT_NODE_REG			%l5
#define BOOTDEV_REG			%l6
#define PIC_REG				%l7

#define	SCRATCH_PAD_BOOT		0x5000
#define	SCRATCH_PAD_DISKBOOT		0x4000
#define	SCRATCH_PAD_BOOT_SIZE		0x110

#define GET_ABS(symbol, reg)	\
	add	PIC_REG, (symbol - pic_base), reg
#define LDUW_ABS(symbol, offset, reg)	\
	lduw	[PIC_REG + (symbol - pic_base) + (offset)], reg
#define LDX_ABS(symbol, offset, reg)	\
	ldx	[PIC_REG + (symbol - pic_base) + (offset)], reg

#define GRUB_BOOT_AOUT_HEADER_SIZE	32

#define GRUB_BOOT_MACHINE_SIGNATURE	0xbb44aa55

#define GRUB_BOOT_MACHINE_BOOT_DEVPATH	0x08

#define GRUB_BOOT_MACHINE_BOOT_DEVPATH_END 0x80

#define GRUB_BOOT_MACHINE_KERNEL_BYTE 0x80

#define GRUB_BOOT_MACHINE_CODE_END \
	(0x1fc - GRUB_BOOT_AOUT_HEADER_SIZE)

#define GRUB_BOOT_MACHINE_KERNEL_ADDR 0x4200

#ifndef ASM_FILE
/* This is the blocklist used in the diskboot image.  */
struct grub_boot_blocklist
{
  grub_uint64_t start;
  grub_uint32_t len;
} GRUB_PACKED;
#endif

#endif /* ! BOOT_MACHINE_HEADER */
