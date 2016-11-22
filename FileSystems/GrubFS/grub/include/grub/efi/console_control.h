/* console_control.h - definitions of the console control protocol */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2006,2007  Free Software Foundation, Inc.
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

#ifndef GRUB_EFI_CONSOLE_CONTROL_HEADER
#define GRUB_EFI_CONSOLE_CONTROL_HEADER	1

#define GRUB_EFI_CONSOLE_CONTROL_GUID	\
  { 0xf42f7782, 0x12e, 0x4c12, \
    { 0x99, 0x56, 0x49, 0xf9, 0x43, 0x4, 0xf7, 0x21 } \
  }

enum grub_efi_screen_mode
  {
    GRUB_EFI_SCREEN_TEXT,
    GRUB_EFI_SCREEN_GRAPHICS,
    GRUB_EFI_SCREEN_TEXT_MAX_VALUE
  };
typedef enum grub_efi_screen_mode grub_efi_screen_mode_t;

struct grub_efi_console_control_protocol
{
  grub_efi_status_t
  (*get_mode) (struct grub_efi_console_control_protocol *this,
	       grub_efi_screen_mode_t *mode,
	       grub_efi_boolean_t *uga_exists,
	       grub_efi_boolean_t *std_in_locked);

  grub_efi_status_t
  (*set_mode) (struct grub_efi_console_control_protocol *this,
	       grub_efi_screen_mode_t mode);

  grub_efi_status_t
  (*lock_std_in) (struct grub_efi_console_control_protocol *this,
		  grub_efi_char16_t *password);
};
typedef struct grub_efi_console_control_protocol grub_efi_console_control_protocol_t;

#endif /* ! GRUB_EFI_CONSOLE_CONTROL_HEADER */
