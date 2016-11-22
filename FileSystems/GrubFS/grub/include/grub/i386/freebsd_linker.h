/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2008,2009  Free Software Foundation, Inc.
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

/*-
 * Copyright (c) 1997-2000 Doug Rabson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: stable/8/sys/sys/linker.h 199583 2009-11-20 15:27:52Z jhb $
 */

#ifndef GRUB_FREEBSD_LINKER_CPU_HEADER
#define GRUB_FREEBSD_LINKER_CPU_HEADER	1

#define FREEBSD_MODINFO_END		0x0000	/* End of list */
#define FREEBSD_MODINFO_NAME		0x0001	/* Name of module (string) */
#define FREEBSD_MODINFO_TYPE		0x0002	/* Type of module (string) */
#define FREEBSD_MODINFO_ADDR		0x0003	/* Loaded address */
#define FREEBSD_MODINFO_SIZE		0x0004	/* Size of module */
#define FREEBSD_MODINFO_EMPTY		0x0005	/* Has been deleted */
#define FREEBSD_MODINFO_ARGS		0x0006	/* Parameters string */
#define FREEBSD_MODINFO_METADATA	0x8000	/* Module-specfic */

#define FREEBSD_MODINFOMD_AOUTEXEC	0x0001	/* a.out exec header */
#define FREEBSD_MODINFOMD_ELFHDR	0x0002	/* ELF header */
#define FREEBSD_MODINFOMD_SSYM		0x0003	/* start of symbols */
#define FREEBSD_MODINFOMD_ESYM		0x0004	/* end of symbols */
#define FREEBSD_MODINFOMD_DYNAMIC	0x0005	/* _DYNAMIC pointer */
#define FREEBSD_MODINFOMD_ENVP		0x0006	/* envp[] */
#define FREEBSD_MODINFOMD_HOWTO		0x0007	/* boothowto */
#define FREEBSD_MODINFOMD_KERNEND	0x0008	/* kernend */
#define FREEBSD_MODINFOMD_SHDR		0x0009	/* section header table */
#define FREEBSD_MODINFOMD_NOCOPY	0x8000	/* don't copy this metadata to the kernel */

#define FREEBSD_MODINFOMD_SMAP		0x1001

#define FREEBSD_MODINFOMD_DEPLIST	(0x4001 | FREEBSD_MODINFOMD_NOCOPY)  /* depends on */

#endif
