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

#include <grub/misc.h>
#include <grub/command.h>
#include <grub/err.h>
#include <grub/dl.h>
#include <grub/mm.h>
#include <grub/term.h>
#include <grub/backtrace.h>

#define MAX_STACK_FRAME 102400

void
grub_backtrace_pointer (void *ebp)
{
  void *ptr, *nptr;
  unsigned i;

  ptr = ebp;
  while (1)
    {
      grub_printf ("%p: ", ptr);
      grub_backtrace_print_address (((void **) ptr)[1]);
      grub_printf (" (");
      for (i = 0; i < 2; i++)
	grub_printf ("%p,", ((void **)ptr) [i + 2]);
      grub_printf ("%p)\n", ((void **)ptr) [i + 2]);
      nptr = *(void **)ptr;
      if (nptr < ptr || (void **) nptr - (void **) ptr > MAX_STACK_FRAME
	  || nptr == ptr)
	{
	  grub_printf ("Invalid stack frame at %p (%p)\n", ptr, nptr);
	  break;
	}
      ptr = nptr;
    }
}

void
grub_backtrace (void)
{
#ifdef __x86_64__
  asm volatile ("movq %rbp, %rdi\n"
		"call " EXT_C("grub_backtrace_pointer"));
#else
  asm volatile ("movl %ebp, %eax\n"
		"call " EXT_C("grub_backtrace_pointer"));
#endif
}

