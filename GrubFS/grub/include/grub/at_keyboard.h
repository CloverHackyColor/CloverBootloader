/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2007,2008,2009  Free Software Foundation, Inc.
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

#ifndef GRUB_AT_KEYBOARD_HEADER
#define GRUB_AT_KEYBOARD_HEADER	1

/* Used for sending commands to the controller.  */
#define KEYBOARD_COMMAND_ISREADY(x)	!((x) & 0x02)
#define KEYBOARD_COMMAND_READ		0x20
#define KEYBOARD_COMMAND_WRITE		0x60
#define KEYBOARD_COMMAND_REBOOT		0xfe

#define KEYBOARD_AT_TRANSLATE		0x40

#define GRUB_AT_ACK                     0xfa
#define GRUB_AT_NACK                    0xfe
#define GRUB_AT_TRIES                   5

#define KEYBOARD_ISMAKE(x)	!((x) & 0x80)
#define KEYBOARD_ISREADY(x)	((x) & 0x01)
#define KEYBOARD_SCANCODE(x)	((x) & 0x7f)

extern void grub_at_keyboard_init (void);
extern void grub_at_keyboard_fini (void);
int grub_at_keyboard_is_alive (void);

#endif
