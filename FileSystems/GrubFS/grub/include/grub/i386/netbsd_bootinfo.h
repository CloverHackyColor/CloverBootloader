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

/*	$NetBSD: bootinfo.h,v 1.16 2009/08/24 02:15:46 jmcneill Exp $	*/

/*
 * Copyright (c) 1997
 *	Matthias Drochner.  All rights reserved.
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
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef GRUB_NETBSD_BOOTINFO_CPU_HEADER
#define GRUB_NETBSD_BOOTINFO_CPU_HEADER	1

#include <grub/types.h>

#define NETBSD_BTINFO_BOOTPATH		0
#define NETBSD_BTINFO_ROOTDEVICE	1
#define NETBSD_BTINFO_BOOTDISK		3
#define NETBSD_BTINFO_CONSOLE		6
#define NETBSD_BTINFO_SYMTAB		8
#define NETBSD_BTINFO_MEMMAP		9
#define NETBSD_BTINFO_BOOTWEDGE		10
#define NETBSD_BTINFO_MODULES		11
#define NETBSD_BTINFO_FRAMEBUF		12

struct grub_netbsd_bootinfo
{
  grub_uint32_t bi_count;
  grub_uint32_t bi_data[0];
};

struct grub_netbsd_btinfo_common
{
  grub_uint32_t len;
  grub_uint32_t type;
};

#define GRUB_NETBSD_MAX_BOOTPATH_LEN 80

struct grub_netbsd_btinfo_bootdisk
{
  grub_uint32_t labelsector;  /* label valid if != 0xffffffff */
  struct
    {
      grub_uint16_t type, checksum;
      char packname[16];
    } label;
  grub_uint32_t biosdev;
  grub_uint32_t partition;
};

struct grub_netbsd_btinfo_bootwedge {
  grub_uint32_t biosdev;
  grub_disk_addr_t startblk;
  grub_uint64_t nblks;
  grub_disk_addr_t matchblk;
  grub_uint64_t matchnblks;
  grub_uint8_t matchhash[16];  /* MD5 hash */
} GRUB_PACKED;

struct grub_netbsd_btinfo_symtab
{
  grub_uint32_t nsyms;
  grub_uint32_t ssyms;
  grub_uint32_t esyms;
};


struct grub_netbsd_btinfo_serial
{
  char devname[16];
  grub_uint32_t addr;
  grub_uint32_t speed;
};

struct grub_netbsd_btinfo_modules
{
  grub_uint32_t num;
  grub_uint32_t last_addr;
  struct grub_netbsd_btinfo_module
  {
    char name[80];
#define GRUB_NETBSD_MODULE_RAW 0
#define GRUB_NETBSD_MODULE_ELF 1
    grub_uint32_t type;
    grub_uint32_t size;
    grub_uint32_t addr;
  } mods[0];
};

struct grub_netbsd_btinfo_framebuf
{
  grub_uint64_t fbaddr;
  grub_uint32_t flags;
  grub_uint32_t width;
  grub_uint32_t height;
  grub_uint16_t pitch;
  grub_uint8_t bpp;

  grub_uint8_t red_mask_size;
  grub_uint8_t green_mask_size;
  grub_uint8_t blue_mask_size;

  grub_uint8_t red_field_pos;
  grub_uint8_t green_field_pos;
  grub_uint8_t blue_field_pos;

  grub_uint8_t reserved[16];
};

#define GRUB_NETBSD_MAX_ROOTDEVICE_LEN 16

#endif
