/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2003,2004,2005,2006,2007,2008,2009,2010  Free Software Foundation, Inc.
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

#include <config.h>
#include <config-util.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#include <grub/dl.h>
#include <grub/mm.h>
#include <grub/setjmp.h>
#include <grub/fs.h>
#include <grub/emu/hostdisk.h>
#include <grub/time.h>
#include <grub/emu/console.h>
#include <grub/emu/misc.h>
#include <grub/kernel.h>
#include <grub/normal.h>
#include <grub/emu/getroot.h>
#include <grub/env.h>
#include <grub/partition.h>
#include <grub/i18n.h>
#include <grub/loader.h>
#include <grub/util/misc.h>

#pragma GCC diagnostic ignored "-Wmissing-prototypes"

#include "progname.h"
#include <argp.h>

#define ENABLE_RELOCATABLE 0

/* Used for going back to the main function.  */
static jmp_buf main_env;

/* Store the prefix specified by an argument.  */
static char *root_dev = NULL, *dir = NULL;

grub_addr_t grub_modbase = 0;

void
grub_reboot (void)
{
  longjmp (main_env, 1);
}

void
grub_machine_init (void)
{
}

void
grub_machine_get_bootlocation (char **device, char **path)
{
  *device = root_dev;
  *path = dir;
}

void
grub_machine_fini (int flags)
{
  if (flags & GRUB_LOADER_FLAG_NORETURN)
    grub_console_fini ();
}



static struct argp_option options[] = {
  {"root",      'r', N_("DEVICE_NAME"), 0, N_("Set root device."), 2},
  {"device-map",  'm', N_("FILE"), 0,
   /* TRANSLATORS: There are many devices in device map.  */
   N_("use FILE as the device map [default=%s]"), 0},
  {"directory",  'd', N_("DIR"), 0,
   N_("use GRUB files in the directory DIR [default=%s]"), 0},
  {"verbose",     'v', 0,      0, N_("print verbose messages."), 0},
  {"hold",     'H', N_("SECS"),      OPTION_ARG_OPTIONAL, N_("wait until a debugger will attach"), 0},
  { 0, 0, 0, 0, 0, 0 }
};

#pragma GCC diagnostic ignored "-Wformat-nonliteral"

static char *
help_filter (int key, const char *text, void *input __attribute__ ((unused)))
{
  switch (key)
    {
    case 'd':
      return xasprintf (text, DEFAULT_DIRECTORY);
    case 'm':
      return xasprintf (text, DEFAULT_DEVICE_MAP);
    default:
      return (char *) text;
    }
}

#pragma GCC diagnostic error "-Wformat-nonliteral"

struct arguments
{
  const char *dev_map;
  int hold;
};

static error_t
argp_parser (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;

  switch (key)
    {
    case 'r':
      free (root_dev);
      root_dev = xstrdup (arg);
      break;
    case 'd':
      free (dir);
      dir = xstrdup (arg);
      break;
    case 'm':
      arguments->dev_map = arg;
      break;
    case 'H':
      arguments->hold = (arg ? atoi (arg) : -1);
      break;
    case 'v':
      verbosity++;
      break;

    case ARGP_KEY_ARG:
      {
	/* Too many arguments. */
	fprintf (stderr, _("Unknown extra argument `%s'."), arg);
	fprintf (stderr, "\n");
	argp_usage (state);
      }
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

static struct argp argp = {
  options, argp_parser, NULL,
  N_("GRUB emulator."),
  NULL, help_filter, NULL
};



#pragma GCC diagnostic ignored "-Wmissing-prototypes"

int
main (int argc, char *argv[])
{
  struct arguments arguments =
    { 
      .dev_map = DEFAULT_DEVICE_MAP,
      .hold = 0
    };
  volatile int hold = 0;

  grub_util_host_init (&argc, &argv);

  dir = xstrdup (DEFAULT_DIRECTORY);

  if (argp_parse (&argp, argc, argv, 0, 0, &arguments) != 0)
    {
      fprintf (stderr, "%s", _("Error in parsing command line arguments\n"));
      exit(1);
    }

  hold = arguments.hold;
  /* Wait until the ARGS.HOLD variable is cleared by an attached debugger. */
  if (hold && verbosity > 0)
    /* TRANSLATORS: In this case GRUB tells user what he has to do.  */
    printf (_("Run `gdb %s %d', and set ARGS.HOLD to zero.\n"),
            program_name, (int) getpid ());
  while (hold)
    {
      if (hold > 0)
        hold--;

      sleep (1);
    }

  signal (SIGINT, SIG_IGN);
  grub_console_init ();
  grub_host_init ();

  /* XXX: This is a bit unportable.  */
  grub_util_biosdisk_init (arguments.dev_map);

  grub_init_all ();

  grub_hostfs_init ();

  /* Make sure that there is a root device.  */
  if (! root_dev)
    root_dev = grub_strdup ("host");

  dir = xstrdup (dir);

  /* Start GRUB!  */
  if (setjmp (main_env) == 0)
    grub_main ();

  grub_fini_all ();
  grub_hostfs_fini ();
  grub_host_fini ();

  grub_machine_fini (GRUB_LOADER_FLAG_NORETURN);

  return 0;
}
