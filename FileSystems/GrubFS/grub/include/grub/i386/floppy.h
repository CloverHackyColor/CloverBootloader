/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2003,2004,2005,2006,2007,2008,2009,2010  Free Software Foundation, Inc.
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

#ifndef GRUB_FLOPPY_CPU_HEADER
#define GRUB_FLOPPY_CPU_HEADER	1

#define GRUB_FLOPPY_REG_DIGITAL_OUTPUT		0x3f2

#ifndef ASM_FILE
#include <grub/cpu/io.h>

/* Stop the floppy drive from spinning, so that other software is
   jumped to with a known state.  */
static inline void
grub_stop_floppy (void)
{
  grub_outb (0, GRUB_FLOPPY_REG_DIGITAL_OUTPUT);
}
#endif

#endif
