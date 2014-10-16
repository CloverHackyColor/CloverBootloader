/* memory.h - describe the memory map */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2007,2010  Free Software Foundation, Inc.
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

#ifndef _GRUB_MACHINE_LBIO_HEADER
#define _GRUB_MACHINE_LBIO_HEADER      1

struct grub_linuxbios_table_header
{
  grub_uint8_t signature[4];
  grub_uint32_t header_size;
  grub_uint32_t header_checksum;
  grub_uint32_t table_size;
  grub_uint32_t table_checksum;
  grub_uint32_t table_entries;
};
typedef struct grub_linuxbios_table_header *grub_linuxbios_table_header_t;

struct grub_linuxbios_timestamp_entry
{
  grub_uint32_t id;
  grub_uint64_t tsc;
} GRUB_PACKED;

struct grub_linuxbios_timestamp_table
{
  grub_uint64_t base_tsc;
  grub_uint32_t capacity;
  grub_uint32_t used;
  struct grub_linuxbios_timestamp_entry entries[0];
} GRUB_PACKED;

struct grub_linuxbios_mainboard
{
  grub_uint8_t vendor;
  grub_uint8_t part_number;
  char strings[0];
};

struct grub_linuxbios_table_item
{
  grub_uint32_t tag;
  grub_uint32_t size;
};
typedef struct grub_linuxbios_table_item *grub_linuxbios_table_item_t;

enum
  {
    GRUB_LINUXBIOS_MEMBER_UNUSED      = 0x00,
    GRUB_LINUXBIOS_MEMBER_MEMORY      = 0x01,
    GRUB_LINUXBIOS_MEMBER_MAINBOARD   = 0x03,
    GRUB_LINUXBIOS_MEMBER_CONSOLE     = 0x10,
    GRUB_LINUXBIOS_MEMBER_LINK        = 0x11,
    GRUB_LINUXBIOS_MEMBER_FRAMEBUFFER = 0x12,
    GRUB_LINUXBIOS_MEMBER_TIMESTAMPS  = 0x16,
    GRUB_LINUXBIOS_MEMBER_CBMEMC      = 0x17
  };

struct grub_linuxbios_table_framebuffer {
  grub_uint64_t lfb;
  grub_uint32_t width;
  grub_uint32_t height;
  grub_uint32_t pitch;
  grub_uint8_t bpp;

  grub_uint8_t red_field_pos;
  grub_uint8_t red_mask_size;
  grub_uint8_t green_field_pos;
  grub_uint8_t green_mask_size;
  grub_uint8_t blue_field_pos;
  grub_uint8_t blue_mask_size;
  grub_uint8_t reserved_field_pos;
  grub_uint8_t reserved_mask_size;
} GRUB_PACKED;

struct grub_linuxbios_mem_region
{
  grub_uint64_t addr;
  grub_uint64_t size;
#define GRUB_MACHINE_MEMORY_AVAILABLE		1
  grub_uint32_t type;
} GRUB_PACKED;
typedef struct grub_linuxbios_mem_region *mem_region_t;

grub_err_t
EXPORT_FUNC(grub_linuxbios_table_iterate) (int (*hook) (grub_linuxbios_table_item_t,
					   void *),
					   void *hook_data);

#endif
