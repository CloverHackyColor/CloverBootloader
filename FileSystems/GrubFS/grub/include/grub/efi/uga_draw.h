/* uga_draw.h - definitions of the uga draw protocol */
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

/* The console control protocol is not a part of the EFI spec,
   but defined in Intel's Sample Implementation.  */

#ifndef GRUB_EFI_UGA_DRAW_HEADER
#define GRUB_EFI_UGA_DRAW_HEADER	1

#define GRUB_EFI_UGA_DRAW_GUID \
  { 0x982c298b, 0xf4fa, 0x41cb, { 0xb8, 0x38, 0x77, 0xaa, 0x68, 0x8f, 0xb8, 0x39 }}

enum grub_efi_uga_blt_operation
{
  GRUB_EFI_UGA_VIDEO_FILL,
  GRUB_EFI_UGA_VIDEO_TO_BLT,
  GRUB_EFI_UGA_BLT_TO_VIDEO,
  GRUB_EFI_UGA_VIDEO_TO_VIDEO,
  GRUB_EFI_UGA_BLT_MAX
};

struct grub_efi_uga_pixel
{
  grub_uint8_t Blue;
  grub_uint8_t Green;
  grub_uint8_t Red;
  grub_uint8_t Reserved;
};

struct grub_efi_uga_draw_protocol
{
  grub_efi_status_t
  (*get_mode) (struct grub_efi_uga_draw_protocol *this,
	       grub_uint32_t *width,
	       grub_uint32_t *height,
	       grub_uint32_t *depth,
	       grub_uint32_t *refresh_rate);

  grub_efi_status_t
  (*set_mode) (struct grub_efi_uga_draw_protocol *this,
	       grub_uint32_t width,
	       grub_uint32_t height,
	       grub_uint32_t depth,
	       grub_uint32_t refresh_rate);

  grub_efi_status_t
  (*blt) (struct grub_efi_uga_draw_protocol *this,
	  struct grub_efi_uga_pixel *blt_buffer,
	  enum grub_efi_uga_blt_operation blt_operation,
	  grub_efi_uintn_t src_x,
	  grub_efi_uintn_t src_y,
	  grub_efi_uintn_t dest_x,
	  grub_efi_uintn_t dest_y,
	  grub_efi_uintn_t width,
	  grub_efi_uintn_t height,
	  grub_efi_uintn_t delta);
};
typedef struct grub_efi_uga_draw_protocol grub_efi_uga_draw_protocol_t;

#endif /* ! GRUB_EFI_UGA_DRAW_HEADER */
