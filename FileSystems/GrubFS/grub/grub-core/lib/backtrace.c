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

GRUB_MOD_LICENSE ("GPLv3+");

void
grub_backtrace_print_address (void *addr)
{
  grub_dl_t mod;

  FOR_DL_MODULES (mod)
  {
    grub_dl_segment_t segment;
    for (segment = mod->segment; segment; segment = segment->next)
      if (segment->addr <= addr && (grub_uint8_t *) segment->addr
	  + segment->size > (grub_uint8_t *) addr)
	{
	  grub_printf ("%s.%x+%" PRIxGRUB_SIZE, mod->name, segment->section,
		       (grub_size_t) ((grub_uint8_t *) addr - (grub_uint8_t *) segment->addr));
	  return;
	}
  }

  grub_printf ("%p", addr);
}

static grub_err_t
grub_cmd_backtrace (grub_command_t cmd __attribute__ ((unused)),
		    int argc __attribute__ ((unused)),
		    char **args __attribute__ ((unused)))
{
  grub_backtrace ();
  return 0;
}

static grub_command_t cmd;

GRUB_MOD_INIT(backtrace)
{
  cmd = grub_register_command ("backtrace", grub_cmd_backtrace,
			       0, N_("Print backtrace."));
}

GRUB_MOD_FINI(backtrace)
{
  grub_unregister_command (cmd);
}
