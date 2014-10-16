/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2010  Free Software Foundation, Inc.
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

#ifndef GRUB_EC_MACHINE_HEADER
#define GRUB_EC_MACHINE_HEADER	1

#include <grub/types.h>
#include <grub/cpu/io.h>
#include <grub/pci.h>

#define GRUB_MACHINE_EC_MAGIC_PORT1 0x381
#define GRUB_MACHINE_EC_MAGIC_PORT2 0x382
#define GRUB_MACHINE_EC_DATA_PORT 0x383

#define GRUB_MACHINE_EC_MAGIC_VAL1 0xf4
#define GRUB_MACHINE_EC_MAGIC_VAL2 0xec

#define GRUB_MACHINE_EC_COMMAND_REBOOT 1

static inline void
grub_write_ec (grub_uint8_t value)
{
  grub_outb (GRUB_MACHINE_EC_MAGIC_VAL1,
	     GRUB_MACHINE_PCI_IO_BASE + GRUB_MACHINE_EC_MAGIC_PORT1);
  grub_outb (GRUB_MACHINE_EC_MAGIC_VAL2,
	     GRUB_MACHINE_PCI_IO_BASE + GRUB_MACHINE_EC_MAGIC_PORT2);
  grub_outb (value, GRUB_MACHINE_PCI_IO_BASE + GRUB_MACHINE_EC_DATA_PORT);
}

#endif
