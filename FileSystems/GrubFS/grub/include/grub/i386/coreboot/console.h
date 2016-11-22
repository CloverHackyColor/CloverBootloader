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

#ifndef GRUB_MACHINE_CONSOLE_HEADER
#define GRUB_MACHINE_CONSOLE_HEADER	1

void grub_vga_text_init (void);
void grub_vga_text_fini (void);

void grub_video_coreboot_fb_init (void);
void grub_video_coreboot_fb_early_init (void);
void grub_video_coreboot_fb_late_init (void);
void grub_video_coreboot_fb_fini (void);

extern struct grub_linuxbios_table_framebuffer *grub_video_coreboot_fbtable;

#endif /* ! GRUB_MACHINE_CONSOLE_HEADER */
