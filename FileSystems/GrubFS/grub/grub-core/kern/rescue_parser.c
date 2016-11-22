/* rescue_parser.c - rescue mode parser  */
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

#include <grub/types.h>
#include <grub/mm.h>
#include <grub/env.h>
#include <grub/parser.h>
#include <grub/misc.h>
#include <grub/command.h>
#include <grub/i18n.h>

grub_err_t
grub_rescue_parse_line (char *line,
			grub_reader_getline_t getline, void *getline_data)
{
  char *name;
  int n;
  grub_command_t cmd;
  char **args;

  if (grub_parser_split_cmdline (line, getline, getline_data, &n, &args)
      || n < 0)
    return grub_errno;

  if (n == 0)
    return GRUB_ERR_NONE;

  /* In case of an assignment set the environment accordingly
     instead of calling a function.  */
  if (n == 1 && grub_strchr (line, '='))
    {
      char *val = grub_strchr (args[0], '=');
      val[0] = 0;
      grub_env_set (args[0], val + 1);
      val[0] = '=';
      goto quit;
    }

  /* Get the command name.  */
  name = args[0];

  /* If nothing is specified, restart.  */
  if (*name == '\0')
    goto quit;

  cmd = grub_command_find (name);
  if (cmd)
    {
      (cmd->func) (cmd, n - 1, &args[1]);
    }
  else
    {
      grub_printf_ (N_("Unknown command `%s'.\n"), name);
      if (grub_command_find ("help"))
	grub_printf ("Try `help' for usage\n");
    }

 quit:
  grub_free (args[0]);
  grub_free (args);

  return grub_errno;
}
