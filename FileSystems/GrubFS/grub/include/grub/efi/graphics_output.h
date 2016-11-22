/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2009  Free Software Foundation, Inc.
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

#ifndef GRUB_EFI_GOP_HEADER
#define GRUB_EFI_GOP_HEADER	1

/* Based on UEFI specification.  */

#define GRUB_EFI_GOP_GUID \
  { 0x9042a9de, 0x23dc, 0x4a38, { 0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a }}

typedef enum
  {
    GRUB_EFI_GOT_RGBA8,
    GRUB_EFI_GOT_BGRA8,
    GRUB_EFI_GOT_BITMASK
  }
  grub_efi_gop_pixel_format_t;

typedef enum
  {
    GRUB_EFI_BLT_VIDEO_FILL,
    GRUB_EFI_BLT_VIDEO_TO_BLT_BUFFER,
    GRUB_EFI_BLT_BUFFER_TO_VIDEO,
    GRUB_EFI_BLT_VIDEO_TO_VIDEO,
    GRUB_EFI_BLT_OPERATION_MAX
  }
  grub_efi_gop_blt_operation_t;

struct grub_efi_gop_blt_pixel
{
  grub_uint8_t blue;
  grub_uint8_t green;
  grub_uint8_t red;
  grub_uint8_t reserved;
};

struct grub_efi_gop_pixel_bitmask
{
  grub_uint32_t r;
  grub_uint32_t g;
  grub_uint32_t b;
  grub_uint32_t a;
};

struct grub_efi_gop_mode_info
{
  grub_efi_uint32_t version;
  grub_efi_uint32_t width;
  grub_efi_uint32_t height;
  grub_efi_gop_pixel_format_t pixel_format;
  struct grub_efi_gop_pixel_bitmask pixel_bitmask;
  grub_efi_uint32_t pixels_per_scanline;
};

struct grub_efi_gop_mode
{
  grub_efi_uint32_t max_mode;
  grub_efi_uint32_t mode;
  struct grub_efi_gop_mode_info *info;
  grub_efi_uintn_t info_size;
  grub_efi_physical_address_t fb_base;
  grub_efi_uintn_t fb_size;
};

/* Forward declaration.  */
struct grub_efi_gop;

typedef grub_efi_status_t
(*grub_efi_gop_query_mode_t) (struct grub_efi_gop *this,
			      grub_efi_uint32_t mode_number,
			      grub_efi_uintn_t *size_of_info,
			      struct grub_efi_gop_mode_info **info);

typedef grub_efi_status_t
(*grub_efi_gop_set_mode_t) (struct grub_efi_gop *this,
			    grub_efi_uint32_t mode_number);

typedef grub_efi_status_t
(*grub_efi_gop_blt_t) (struct grub_efi_gop *this,
		       void *buffer,
		       grub_efi_uintn_t operation,
		       grub_efi_uintn_t sx,
		       grub_efi_uintn_t sy,
		       grub_efi_uintn_t dx,
		       grub_efi_uintn_t dy,
		       grub_efi_uintn_t width,
		       grub_efi_uintn_t height,
		       grub_efi_uintn_t delta);

struct grub_efi_gop
{
  grub_efi_gop_query_mode_t query_mode;
  grub_efi_gop_set_mode_t set_mode;
  grub_efi_gop_blt_t blt;
  struct grub_efi_gop_mode *mode;
};

#endif
