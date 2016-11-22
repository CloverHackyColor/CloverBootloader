/* rescue_reader.c - rescue mode reader  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2009,2010  Free Software Foundation, Inc.
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

#include <grub/types.h>
#include <grub/reader.h>
#include <grub/parser.h>
#include <grub/misc.h>
#include <grub/term.h>
#include <grub/mm.h>

#define GRUB_RESCUE_BUF_SIZE	256

static char linebuf[GRUB_RESCUE_BUF_SIZE];

/* Prompt to input a command and read the line.  */
static grub_err_t
grub_rescue_read_line (char **line, int cont,
		       void *data __attribute__ ((unused)))
{
  int c;
  int pos = 0;
  char str[4];

  grub_printf ((cont) ? "> " : "grub rescue> ");
  grub_memset (linebuf, 0, GRUB_RESCUE_BUF_SIZE);

  while ((c = grub_getkey ()) != '\n' && c != '\r')
    {
      if (grub_isprint (c))
	{
	  if (pos < GRUB_RESCUE_BUF_SIZE - 1)
	    {
	      str[0] = c;
	      str[1] = 0;
	      linebuf[pos++] = c;
	      grub_xputs (str);
	    }
	}
      else if (c == '\b')
	{
	  if (pos > 0)
	    {
	      str[0] = c;
	      str[1] = ' ';
	      str[2] = c;
	      str[3] = 0;
	      linebuf[--pos] = 0;
	      grub_xputs (str);
	    }
	}
      grub_refresh ();
    }

  grub_xputs ("\n");
  grub_refresh ();

  *line = grub_strdup (linebuf);

  return 0;
}

void __attribute__ ((noreturn))
grub_rescue_run (void)
{
  grub_printf ("Entering rescue mode...\n");

  while (1)
    {
      char *line;

      /* Print an error, if any.  */
      grub_print_error ();
      grub_errno = GRUB_ERR_NONE;

      grub_rescue_read_line (&line, 0, NULL);
      if (! line || line[0] == '\0')
	continue;

      grub_rescue_parse_line (line, grub_rescue_read_line, NULL);
      grub_free (line);
    }
}
