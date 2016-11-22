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

/*      $OpenBSD: bootarg.h,v 1.11 2003/06/02 20:20:54 mickey Exp $     */
     
/*
 * Copyright (c) 1996-1999 Michael Shalayeff
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR OR HIS RELATIVES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF MIND, USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GRUB_OPENBSD_BOOTARG_CPU_HEADER
#define GRUB_OPENBSD_BOOTARG_CPU_HEADER	1

#define OPENBSD_BOOTARG_APIVER	(OPENBSD_BAPIV_VECTOR | \
                                 OPENBSD_BAPIV_ENV | \
                                 OPENBSD_BAPIV_BMEMMAP)

#define OPENBSD_BAPIV_ANCIENT	0x0  /* MD old i386 bootblocks */
#define OPENBSD_BAPIV_VARS	0x1  /* MD structure w/ add info passed */
#define OPENBSD_BAPIV_VECTOR	0x2  /* MI vector of MD structures passed */
#define OPENBSD_BAPIV_ENV	0x4  /* MI environment vars vector */
#define OPENBSD_BAPIV_BMEMMAP	0x8  /* MI memory map passed is in bytes */

#define OPENBSD_BOOTARG_ENV	0x1000
#define OPENBSD_BOOTARG_END	-1

#define	OPENBSD_BOOTARG_MMAP	0
#define	OPENBSD_BOOTARG_PCIBIOS 4
#define	OPENBSD_BOOTARG_CONSOLE 5

struct grub_openbsd_bootargs
{
  grub_uint32_t ba_type;
  grub_uint32_t ba_size;
  grub_uint32_t ba_next;
} GRUB_PACKED;

struct grub_openbsd_bootarg_console
{
  grub_uint32_t device;
  grub_uint32_t speed;
  grub_uint32_t addr;
  grub_uint32_t frequency;
};

struct grub_openbsd_bootarg_pcibios
{
  grub_uint32_t characteristics;
  grub_uint32_t revision;
  grub_uint32_t pm_entry;
  grub_uint32_t last_bus;
};

#define GRUB_OPENBSD_COM_MAJOR 8
#define GRUB_OPENBSD_VGA_MAJOR 12

#endif
