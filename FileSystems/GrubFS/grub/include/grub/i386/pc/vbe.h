/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2005,2006,2007,2008,2009  Free Software Foundation, Inc.
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

#ifndef GRUB_VBE_MACHINE_HEADER
#define GRUB_VBE_MACHINE_HEADER	1

#include <grub/video.h>

/* Default video mode to be used.  */
#define GRUB_VBE_DEFAULT_VIDEO_MODE     0x101

/* VBE status codes.  */
#define GRUB_VBE_STATUS_OK		0x004f

#define GRUB_VBE_CAPABILITY_DACWIDTH	(1 << 0)

/* Bits from the GRUB_VBE "mode_attributes" field in the mode info struct.  */
#define GRUB_VBE_MODEATTR_SUPPORTED                 (1 << 0)
#define GRUB_VBE_MODEATTR_RESERVED_1                (1 << 1)
#define GRUB_VBE_MODEATTR_BIOS_TTY_OUTPUT_SUPPORT   (1 << 2)
#define GRUB_VBE_MODEATTR_COLOR                     (1 << 3)
#define GRUB_VBE_MODEATTR_GRAPHICS                  (1 << 4)
#define GRUB_VBE_MODEATTR_VGA_COMPATIBLE            (1 << 5)
#define GRUB_VBE_MODEATTR_VGA_WINDOWED_AVAIL        (1 << 6)
#define GRUB_VBE_MODEATTR_LFB_AVAIL                 (1 << 7)
#define GRUB_VBE_MODEATTR_DOUBLE_SCAN_AVAIL         (1 << 8)
#define GRUB_VBE_MODEATTR_INTERLACED_AVAIL          (1 << 9)
#define GRUB_VBE_MODEATTR_TRIPLE_BUF_AVAIL          (1 << 10)
#define GRUB_VBE_MODEATTR_STEREO_AVAIL              (1 << 11)
#define GRUB_VBE_MODEATTR_DUAL_DISPLAY_START        (1 << 12)

/* Values for the GRUB_VBE memory_model field in the mode info struct.  */
#define GRUB_VBE_MEMORY_MODEL_TEXT           0x00
#define GRUB_VBE_MEMORY_MODEL_CGA            0x01
#define GRUB_VBE_MEMORY_MODEL_HERCULES       0x02
#define GRUB_VBE_MEMORY_MODEL_PLANAR         0x03
#define GRUB_VBE_MEMORY_MODEL_PACKED_PIXEL   0x04
#define GRUB_VBE_MEMORY_MODEL_NONCHAIN4_256  0x05
#define GRUB_VBE_MEMORY_MODEL_DIRECT_COLOR   0x06
#define GRUB_VBE_MEMORY_MODEL_YUV            0x07

/* Note:

   Please refer to VESA BIOS Extension 3.0 Specification for more descriptive
   meanings of following structures and how they should be used.

   I have tried to maintain field name compatibility against specification
   while following naming conventions used in GRUB.  */

typedef grub_uint32_t grub_vbe_farptr_t;
typedef grub_uint32_t grub_vbe_physptr_t;
typedef grub_uint32_t grub_vbe_status_t;

struct grub_vbe_info_block
{
  grub_uint8_t signature[4];
  grub_uint16_t version;

  grub_vbe_farptr_t oem_string_ptr;
  grub_uint32_t capabilities;
  grub_vbe_farptr_t video_mode_ptr;
  grub_uint16_t total_memory;

  grub_uint16_t oem_software_rev;
  grub_vbe_farptr_t oem_vendor_name_ptr;
  grub_vbe_farptr_t oem_product_name_ptr;
  grub_vbe_farptr_t oem_product_rev_ptr;

  grub_uint8_t reserved[222];

  grub_uint8_t oem_data[256];
} GRUB_PACKED;

struct grub_vbe_mode_info_block
{
  /* Mandatory information for all VBE revisions.  */
  grub_uint16_t mode_attributes;
  grub_uint8_t win_a_attributes;
  grub_uint8_t win_b_attributes;
  grub_uint16_t win_granularity;
  grub_uint16_t win_size;
  grub_uint16_t win_a_segment;
  grub_uint16_t win_b_segment;
  grub_vbe_farptr_t win_func_ptr;
  grub_uint16_t bytes_per_scan_line;

  /* Mandatory information for VBE 1.2 and above.  */
  grub_uint16_t x_resolution;
  grub_uint16_t y_resolution;
  grub_uint8_t x_char_size;
  grub_uint8_t y_char_size;
  grub_uint8_t number_of_planes;
  grub_uint8_t bits_per_pixel;
  grub_uint8_t number_of_banks;
  grub_uint8_t memory_model;
  grub_uint8_t bank_size;
  grub_uint8_t number_of_image_pages;
  grub_uint8_t reserved;

  /* Direct Color fields (required for direct/6 and YUV/7 memory models).  */
  grub_uint8_t red_mask_size;
  grub_uint8_t red_field_position;
  grub_uint8_t green_mask_size;
  grub_uint8_t green_field_position;
  grub_uint8_t blue_mask_size;
  grub_uint8_t blue_field_position;
  grub_uint8_t rsvd_mask_size;
  grub_uint8_t rsvd_field_position;
  grub_uint8_t direct_color_mode_info;

  /* Mandatory information for VBE 2.0 and above.  */
  grub_vbe_physptr_t phys_base_addr;
  grub_uint32_t reserved2;
  grub_uint16_t reserved3;

  /* Mandatory information for VBE 3.0 and above.  */
  grub_uint16_t lin_bytes_per_scan_line;
  grub_uint8_t bnk_number_of_image_pages;
  grub_uint8_t lin_number_of_image_pages;
  grub_uint8_t lin_red_mask_size;
  grub_uint8_t lin_red_field_position;
  grub_uint8_t lin_green_mask_size;
  grub_uint8_t lin_green_field_position;
  grub_uint8_t lin_blue_mask_size;
  grub_uint8_t lin_blue_field_position;
  grub_uint8_t lin_rsvd_mask_size;
  grub_uint8_t lin_rsvd_field_position;
  grub_uint32_t max_pixel_clock;

  /* Reserved field to make structure to be 256 bytes long, VESA BIOS
     Extension 3.0 Specification says to reserve 189 bytes here but
     that doesn't make structure to be 256 bytes.  So additional one is
     added here.  */
  grub_uint8_t reserved4[189 + 1];
} GRUB_PACKED;

struct grub_vbe_crtc_info_block
{
  grub_uint16_t horizontal_total;
  grub_uint16_t horizontal_sync_start;
  grub_uint16_t horizontal_sync_end;
  grub_uint16_t vertical_total;
  grub_uint16_t vertical_sync_start;
  grub_uint16_t vertical_sync_end;
  grub_uint8_t flags;
  grub_uint32_t pixel_clock;
  grub_uint16_t refresh_rate;
  grub_uint8_t reserved[40];
} GRUB_PACKED;

struct grub_vbe_palette_data
{
  grub_uint8_t blue;
  grub_uint8_t green;
  grub_uint8_t red;
  grub_uint8_t alignment;
} GRUB_PACKED;

struct grub_vbe_flat_panel_info
{
  grub_uint16_t horizontal_size;
  grub_uint16_t vertical_size;
  grub_uint16_t panel_type;
  grub_uint8_t red_bpp;
  grub_uint8_t green_bpp;
  grub_uint8_t blue_bpp;
  grub_uint8_t reserved_bpp;
  grub_uint32_t reserved_offscreen_mem_size;
  grub_vbe_farptr_t reserved_offscreen_mem_ptr;

  grub_uint8_t reserved[14];
} GRUB_PACKED;

/* Prototypes for helper functions.  */
/* Call VESA BIOS 0x4f00 to get VBE Controller Information, return status.  */
grub_vbe_status_t 
grub_vbe_bios_get_controller_info (struct grub_vbe_info_block *controller_info);
/* Call VESA BIOS 0x4f01 to get VBE Mode Information, return status.  */
grub_vbe_status_t 
grub_vbe_bios_get_mode_info (grub_uint32_t mode,
			     struct grub_vbe_mode_info_block *mode_info);
/* Call VESA BIOS 0x4f03 to return current VBE Mode, return status.  */
grub_vbe_status_t 
grub_vbe_bios_get_mode (grub_uint32_t *mode);
/* Call VESA BIOS 0x4f05 to set memory window, return status.  */
grub_vbe_status_t
grub_vbe_bios_set_memory_window (grub_uint32_t window, grub_uint32_t position);
/* Call VESA BIOS 0x4f05 to return memory window, return status.  */
grub_vbe_status_t
grub_vbe_bios_get_memory_window (grub_uint32_t window,
				 grub_uint32_t *position);
/* Call VESA BIOS 0x4f06 to set scanline length (in bytes), return status.  */
grub_vbe_status_t 
grub_vbe_bios_set_scanline_length (grub_uint32_t length);
/* Call VESA BIOS 0x4f06 to return scanline length (in bytes), return status.  */
grub_vbe_status_t 
grub_vbe_bios_get_scanline_length (grub_uint32_t *length);
/* Call VESA BIOS 0x4f07 to get display start, return status.  */
grub_vbe_status_t 
grub_vbe_bios_get_display_start (grub_uint32_t *x,
				 grub_uint32_t *y);

grub_vbe_status_t grub_vbe_bios_getset_dac_palette_width (int set, int *width);

#define grub_vbe_bios_get_dac_palette_width(width)	grub_vbe_bios_getset_dac_palette_width(0, (width))
#define grub_vbe_bios_set_dac_palette_width(width)	grub_vbe_bios_getset_dac_palette_width(1, (width))

grub_err_t grub_vbe_probe (struct grub_vbe_info_block *info_block);
grub_err_t grub_vbe_get_video_mode (grub_uint32_t *mode);
grub_err_t grub_vbe_get_video_mode_info (grub_uint32_t mode,
                                         struct grub_vbe_mode_info_block *mode_info);
grub_vbe_status_t 
grub_vbe_bios_get_pm_interface (grub_uint16_t *seg, grub_uint16_t *offset,
				grub_uint16_t *length);


#endif /* ! GRUB_VBE_MACHINE_HEADER */
