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

#ifndef GRUB_MACHINE_AT_KEYBOARD_HEADER
#define GRUB_MACHINE_AT_KEYBOARD_HEADER	1

#include <grub/pci.h>

#define KEYBOARD_REG_DATA	(GRUB_MACHINE_PCI_IO_BASE | 0x60)
#define KEYBOARD_REG_STATUS	(GRUB_MACHINE_PCI_IO_BASE | 0x64)

#endif
