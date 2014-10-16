/*
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 ********************************************************************
 * NOTE: This header file defines an interface to U-Boot. Including
 * this (unmodified) header file in another file is considered normal
 * use of U-Boot, and does *not* fall under the heading of "derived
 * work".
 ********************************************************************
 */

#ifndef __GRUB_UBOOT_IMAGE_H__
#define __GRUB_UBOOT_IMAGE_H__

/*
 * Operating System Codes
 */
#define GRUB_UBOOT_IH_OS_INVALID		0	/* Invalid OS	*/
#define GRUB_UBOOT_IH_OS_OPENBSD		1	/* OpenBSD	*/
#define GRUB_UBOOT_IH_OS_NETBSD		2	/* NetBSD	*/
#define GRUB_UBOOT_IH_OS_FREEBSD		3	/* FreeBSD	*/
#define GRUB_UBOOT_IH_OS_4_4BSD		4	/* 4.4BSD	*/
#define GRUB_UBOOT_IH_OS_LINUX		5	/* Linux	*/
#define GRUB_UBOOT_IH_OS_SVR4		6	/* SVR4		*/
#define GRUB_UBOOT_IH_OS_ESIX		7	/* Esix		*/
#define GRUB_UBOOT_IH_OS_SOLARIS		8	/* Solaris	*/
#define GRUB_UBOOT_IH_OS_IRIX		9	/* Irix		*/
#define GRUB_UBOOT_IH_OS_SCO		10	/* SCO		*/
#define GRUB_UBOOT_IH_OS_DELL		11	/* Dell		*/
#define GRUB_UBOOT_IH_OS_NCR		12	/* NCR		*/
#define GRUB_UBOOT_IH_OS_LYNXOS		13	/* LynxOS	*/
#define GRUB_UBOOT_IH_OS_VXWORKS		14	/* VxWorks	*/
#define GRUB_UBOOT_IH_OS_PSOS		15	/* pSOS		*/
#define GRUB_UBOOT_IH_OS_QNX		16	/* QNX		*/
#define GRUB_UBOOT_IH_OS_U_BOOT		17	/* Firmware	*/
#define GRUB_UBOOT_IH_OS_RTEMS		18	/* RTEMS	*/
#define GRUB_UBOOT_IH_OS_ARTOS		19	/* ARTOS	*/
#define GRUB_UBOOT_IH_OS_UNITY		20	/* Unity OS	*/
#define GRUB_UBOOT_IH_OS_INTEGRITY		21	/* INTEGRITY	*/
#define GRUB_UBOOT_IH_OS_OSE		22	/* OSE		*/

/*
 * CPU Architecture Codes (supported by Linux)
 */
#define GRUB_UBOOT_IH_ARCH_INVALID		0	/* Invalid CPU	*/
#define GRUB_UBOOT_IH_ARCH_ALPHA		1	/* Alpha	*/
#define GRUB_UBOOT_IH_ARCH_ARM		2	/* ARM		*/
#define GRUB_UBOOT_IH_ARCH_I386		3	/* Intel x86	*/
#define GRUB_UBOOT_IH_ARCH_IA64		4	/* IA64		*/
#define GRUB_UBOOT_IH_ARCH_MIPS		5	/* MIPS		*/
#define GRUB_UBOOT_IH_ARCH_MIPS64		6	/* MIPS	 64 Bit */
#define GRUB_UBOOT_IH_ARCH_PPC		7	/* PowerPC	*/
#define GRUB_UBOOT_IH_ARCH_S390		8	/* IBM S390	*/
#define GRUB_UBOOT_IH_ARCH_SH		9	/* SuperH	*/
#define GRUB_UBOOT_IH_ARCH_SPARC		10	/* Sparc	*/
#define GRUB_UBOOT_IH_ARCH_SPARC64		11	/* Sparc 64 Bit */
#define GRUB_UBOOT_IH_ARCH_M68K		12	/* M68K		*/
#define GRUB_UBOOT_IH_ARCH_MICROBLAZE	14	/* MicroBlaze   */
#define GRUB_UBOOT_IH_ARCH_NIOS2		15	/* Nios-II	*/
#define GRUB_UBOOT_IH_ARCH_BLACKFIN	16	/* Blackfin	*/
#define GRUB_UBOOT_IH_ARCH_AVR32		17	/* AVR32	*/
#define GRUB_UBOOT_IH_ARCH_ST200	        18	/* STMicroelectronics ST200  */
#define GRUB_UBOOT_IH_ARCH_SANDBOX		19	/* Sandbox architecture (test only) */
#define GRUB_UBOOT_IH_ARCH_NDS32	        20	/* ANDES Technology - NDS32  */
#define GRUB_UBOOT_IH_ARCH_OPENRISC        21	/* OpenRISC 1000  */

/*
 * Image Types
 *
 * "Standalone Programs" are directly runnable in the environment
 *	provided by U-Boot; it is expected that (if they behave
 *	well) you can continue to work in U-Boot after return from
 *	the Standalone Program.
 * "OS Kernel Images" are usually images of some Embedded OS which
 *	will take over control completely. Usually these programs
 *	will install their own set of exception handlers, device
 *	drivers, set up the MMU, etc. - this means, that you cannot
 *	expect to re-enter U-Boot except by resetting the CPU.
 * "RAMDisk Images" are more or less just data blocks, and their
 *	parameters (address, size) are passed to an OS kernel that is
 *	being started.
 * "Multi-File Images" contain several images, typically an OS
 *	(Linux) kernel image and one or more data images like
 *	RAMDisks. This construct is useful for instance when you want
 *	to boot over the network using BOOTP etc., where the boot
 *	server provides just a single image file, but you want to get
 *	for instance an OS kernel and a RAMDisk image.
 *
 *	"Multi-File Images" start with a list of image sizes, each
 *	image size (in bytes) specified by an "uint32_t" in network
 *	byte order. This list is terminated by an "(uint32_t)0".
 *	Immediately after the terminating 0 follow the images, one by
 *	one, all aligned on "uint32_t" boundaries (size rounded up to
 *	a multiple of 4 bytes - except for the last file).
 *
 * "Firmware Images" are binary images containing firmware (like
 *	U-Boot or FPGA images) which usually will be programmed to
 *	flash memory.
 *
 * "Script files" are command sequences that will be executed by
 *	U-Boot's command interpreter; this feature is especially
 *	useful when you configure U-Boot to use a real shell (hush)
 *	as command interpreter (=> Shell Scripts).
 */

#define GRUB_UBOOT_IH_TYPE_INVALID		0	/* Invalid Image		*/
#define GRUB_UBOOT_IH_TYPE_STANDALONE	1	/* Standalone Program		*/
#define GRUB_UBOOT_IH_TYPE_KERNEL		2	/* OS Kernel Image		*/
#define GRUB_UBOOT_IH_TYPE_RAMDISK		3	/* RAMDisk Image		*/
#define GRUB_UBOOT_IH_TYPE_MULTI		4	/* Multi-File Image		*/
#define GRUB_UBOOT_IH_TYPE_FIRMWARE	5	/* Firmware Image		*/
#define GRUB_UBOOT_IH_TYPE_SCRIPT		6	/* Script file			*/
#define GRUB_UBOOT_IH_TYPE_FILESYSTEM	7	/* Filesystem Image (any type)	*/
#define GRUB_UBOOT_IH_TYPE_FLATDT		8	/* Binary Flat Device Tree Blob	*/
#define GRUB_UBOOT_IH_TYPE_KWBIMAGE	9	/* Kirkwood Boot Image		*/
#define GRUB_UBOOT_IH_TYPE_IMXIMAGE	10	/* Freescale IMXBoot Image	*/
#define GRUB_UBOOT_IH_TYPE_UBLIMAGE	11	/* Davinci UBL Image		*/
#define GRUB_UBOOT_IH_TYPE_OMAPIMAGE	12	/* TI OMAP Config Header Image	*/
#define GRUB_UBOOT_IH_TYPE_AISIMAGE	13	/* TI Davinci AIS Image		*/
#define GRUB_UBOOT_IH_TYPE_KERNEL_NOLOAD	14	/* OS Kernel Image, can run from any load address */
#define GRUB_UBOOT_IH_TYPE_PBLIMAGE	15	/* Freescale PBL Boot Image	*/

/*
 * Compression Types
 */
#define GRUB_UBOOT_IH_COMP_NONE		0	/*  No	 Compression Used	*/
#define GRUB_UBOOT_IH_COMP_GZIP		1	/* gzip	 Compression Used	*/
#define GRUB_UBOOT_IH_COMP_BZIP2		2	/* bzip2 Compression Used	*/
#define GRUB_UBOOT_IH_COMP_LZMA		3	/* lzma  Compression Used	*/
#define GRUB_UBOOT_IH_COMP_LZO		4	/* lzo   Compression Used	*/

#define GRUB_UBOOT_IH_MAGIC	0x27051956	/* Image Magic Number		*/
#define GRUB_UBOOT_IH_NMLEN		32	/* Image Name Length		*/

/*
 * Legacy format image header,
 * all data in network byte order (aka natural aka bigendian).
 */
struct grub_uboot_image_header {
	grub_uint32_t	ih_magic;	/* Image Header Magic Number	*/
	grub_uint32_t	ih_hcrc;	/* Image Header CRC Checksum	*/
	grub_uint32_t	ih_time;	/* Image Creation Timestamp	*/
	grub_uint32_t	ih_size;	/* Image Data Size		*/
	grub_uint32_t	ih_load;	/* Data	 Load  Address		*/
	grub_uint32_t	ih_ep;		/* Entry Point Address		*/
	grub_uint32_t	ih_dcrc;	/* Image Data CRC Checksum	*/
	grub_uint8_t	ih_os;		/* Operating System		*/
	grub_uint8_t	ih_arch;	/* CPU architecture		*/
	grub_uint8_t	ih_type;	/* Image Type			*/
	grub_uint8_t	ih_comp;	/* Compression Type		*/
	grub_uint8_t	ih_name[GRUB_UBOOT_IH_NMLEN];	/* Image Name		*/
};

#endif	/* __IMAGE_H__ */
