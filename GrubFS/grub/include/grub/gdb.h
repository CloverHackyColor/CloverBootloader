/* gdb.h - Various definitions for the remote GDB stub */
/*
 *  Copyright (C) 2006  Lubomir Kundrak
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef GRUB_GDB_HEADER
#define GRUB_GDB_HEADER		1

#define SIGFPE		8
#define SIGTRAP		5
#define SIGABRT		6
#define SIGSEGV		11
#define SIGILL		4
#define SIGUSR1		30
/* We probably don't need other ones.  */

struct grub_serial_port;

extern struct grub_serial_port *grub_gdb_port;

void grub_gdb_breakpoint (void);
unsigned int grub_gdb_trap2sig (int);

#endif /* ! GRUB_GDB_HEADER */

