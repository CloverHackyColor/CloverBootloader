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

#ifndef GRUB_MACHINE_SERIAL_HEADER
#define GRUB_MACHINE_SERIAL_HEADER	1

#define GRUB_MACHINE_SERIAL_PORT0_DIVISOR_115200 2
#define GRUB_MACHINE_SERIAL_PORT2_DIVISOR_115200 1
#define GRUB_MACHINE_SERIAL_PORT0  0xbff003f8
#define GRUB_MACHINE_SERIAL_PORT1  0xbfd003f8
#define GRUB_MACHINE_SERIAL_PORT2  0xbfd002f8

#ifndef ASM_FILE
#define GRUB_MACHINE_SERIAL_PORTS { GRUB_MACHINE_SERIAL_PORT0, GRUB_MACHINE_SERIAL_PORT1, GRUB_MACHINE_SERIAL_PORT2 }
#else
#endif

#endif 
