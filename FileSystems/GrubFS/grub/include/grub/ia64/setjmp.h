/* Define the machine-dependent type `jmp_buf'.  Linux/IA-64 version.
   Copyright (C) 1999, 2000, 2008 Free Software Foundation, Inc.
   Contributed by David Mosberger-Tang <davidm@hpl.hp.com>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* User code must not depend on the internal representation of jmp_buf. */

#define _JBLEN	70

/* the __jmp_buf element type should be __float80 per ABI... */
typedef long grub_jmp_buf[_JBLEN] __attribute__ ((aligned (16))); /* guarantees 128-bit alignment! */

int grub_setjmp (grub_jmp_buf env) RETURNS_TWICE;
void grub_longjmp (grub_jmp_buf env, int val) __attribute__ ((noreturn));
