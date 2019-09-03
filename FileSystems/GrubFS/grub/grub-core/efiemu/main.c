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

/* This is an emulation of EFI runtime services.
   This allows a more uniform boot on i386 machines.
   As it emulates only runtime service it isn't able
   to chainload EFI bootloader on non-EFI system. */


#include <grub/file.h>
#include <grub/err.h>
#include <grub/normal.h>
#include <grub/mm.h>
#include <grub/dl.h>
#include <grub/misc.h>
#include <grub/efiemu/efiemu.h>
#include <grub/command.h>
#include <grub/i18n.h>

GRUB_MOD_LICENSE ("GPLv3+");

/* System table. Two version depending on mode */
grub_efi_system_table32_t *grub_efiemu_system_table32 = 0;
grub_efi_system_table64_t *grub_efiemu_system_table64 = 0;
/* Modules may need to execute some actions after memory allocation happens */
static struct grub_efiemu_prepare_hook *efiemu_prepare_hooks = 0;
/* Linked list of configuration tables */
static struct grub_efiemu_configuration_table *efiemu_config_tables = 0;
static int prepared = 0;

/* Free all allocated space */
grub_err_t
grub_efiemu_unload (void)
{
  struct grub_efiemu_configuration_table *cur, *d;
  struct grub_efiemu_prepare_hook *curhook, *d2;
  grub_efiemu_loadcore_unload ();

  grub_efiemu_mm_unload ();

  for (cur = efiemu_config_tables; cur;)
    {
      d = cur->next;
      if (cur->unload)
	cur->unload (cur->data);
      grub_free (cur);
      cur = d;
    }
  efiemu_config_tables = 0;

  for (curhook = efiemu_prepare_hooks; curhook;)
    {
      d2 = curhook->next;
      if (curhook->unload)
	curhook->unload (curhook->data);
      grub_free (curhook);
      curhook = d2;
    }
  efiemu_prepare_hooks = 0;

  prepared = 0;

  return GRUB_ERR_NONE;
}

/* Remove previously registered table from the list */
grub_err_t
grub_efiemu_unregister_configuration_table (grub_efi_guid_t guid)
{
  struct grub_efiemu_configuration_table *cur, *prev;

  /* Special treating if head is to remove */
  while (efiemu_config_tables
	 && !grub_memcmp (&(efiemu_config_tables->guid), &guid, sizeof (guid)))
    {
      if (efiemu_config_tables->unload)
	  efiemu_config_tables->unload (efiemu_config_tables->data);
	cur = efiemu_config_tables->next;
	grub_free (efiemu_config_tables);
	efiemu_config_tables = cur;
    }
  if (!efiemu_config_tables)
    return GRUB_ERR_NONE;

  /* Remove from chain */
  for (prev = efiemu_config_tables, cur = prev->next; cur;)
    if (grub_memcmp (&(cur->guid), &guid, sizeof (guid)) == 0)
      {
	if (cur->unload)
	  cur->unload (cur->data);
	prev->next = cur->next;
	grub_free (cur);
	cur = prev->next;
      }
    else
      {
	prev = cur;
	cur = cur->next;
      }
  return GRUB_ERR_NONE;
}

grub_err_t
grub_efiemu_register_prepare_hook (grub_err_t (*hook) (void *data),
				   void (*unload) (void *data),
				   void *data)
{
  struct grub_efiemu_prepare_hook *nhook;
  nhook = (struct grub_efiemu_prepare_hook *) grub_malloc (sizeof (*nhook));
  if (! nhook)
    return grub_errno;
  nhook->hook = hook;
  nhook->unload = unload;
  nhook->data = data;
  nhook->next = efiemu_prepare_hooks;
  efiemu_prepare_hooks = nhook;
  return GRUB_ERR_NONE;
}

/* Register a configuration table either supplying the address directly
   or with a hook
*/
grub_err_t
grub_efiemu_register_configuration_table (grub_efi_guid_t guid,
					  void * (*get_table) (void *data),
					  void (*unload) (void *data),
					  void *data)
{
  struct grub_efiemu_configuration_table *tbl;
  grub_err_t err;

 err = grub_efiemu_unregister_configuration_table (guid);
  if (err)
    return err;

  tbl = (struct grub_efiemu_configuration_table *) grub_malloc (sizeof (*tbl));
  if (! tbl)
    return grub_errno;

  tbl->guid = guid;
  tbl->get_table = get_table;
  tbl->unload = unload;
  tbl->data = data;
  tbl->next = efiemu_config_tables;
  efiemu_config_tables = tbl;

  return GRUB_ERR_NONE;
}

static grub_err_t
grub_cmd_efiemu_unload (grub_command_t cmd __attribute__ ((unused)),
			int argc __attribute__ ((unused)),
			char *args[] __attribute__ ((unused)))
{
  return grub_efiemu_unload ();
}

static grub_err_t
grub_cmd_efiemu_prepare (grub_command_t cmd __attribute__ ((unused)),
			 int argc __attribute__ ((unused)),
			 char *args[] __attribute__ ((unused)))
{
  return grub_efiemu_prepare ();
}



/* Load the runtime from the file FILENAME.  */
static grub_err_t
grub_efiemu_load_file (const char *filename)
{
  grub_file_t file;
  grub_err_t err;

  file = grub_file_open (filename);
  if (! file)
    return grub_errno;

  err = grub_efiemu_mm_init ();
  if (err)
    {
      grub_file_close (file);
      grub_efiemu_unload ();
      return grub_errno;
    }

  grub_dprintf ("efiemu", "mm initialized\n");

  err = grub_efiemu_loadcore_init (file, filename);
  if (err)
    {
      grub_file_close (file);
      grub_efiemu_unload ();
      return err;
    }

  grub_file_close (file);

  /* For configuration tables entry in system table. */
  grub_efiemu_request_symbols (1);

  return GRUB_ERR_NONE;
}

grub_err_t
grub_efiemu_autocore (void)
{
  const char *prefix;
  char *filename;
  const char *suffix;
  grub_err_t err;

  if (grub_efiemu_sizeof_uintn_t () != 0)
    return GRUB_ERR_NONE;

  prefix = grub_env_get ("prefix");

  if (! prefix)
    return grub_error (GRUB_ERR_FILE_NOT_FOUND,
		       N_("variable `%s' isn't set"), "prefix");

  suffix = grub_efiemu_get_default_core_name ();

  filename = grub_xasprintf ("%s/" GRUB_TARGET_CPU "-" GRUB_PLATFORM "/%s",
			     prefix, suffix);
  if (! filename)
    return grub_errno;

  err = grub_efiemu_load_file (filename);
  grub_free (filename);
  if (err)
    return err;
#ifndef GRUB_MACHINE_EMU
  err = grub_machine_efiemu_init_tables ();
  if (err)
    return err;
#endif

  return GRUB_ERR_NONE;
}

grub_err_t
grub_efiemu_prepare (void)
{
  grub_err_t err;

  if (prepared)
    return GRUB_ERR_NONE;

  err = grub_efiemu_autocore ();
  if (err)
    return err;

  grub_dprintf ("efiemu", "Preparing %d-bit efiemu\n",
		8 * grub_efiemu_sizeof_uintn_t ());

  /* Create NVRAM. */
  grub_efiemu_pnvram ();

  prepared = 1;

  if (grub_efiemu_sizeof_uintn_t () == 4)
    return grub_efiemu_prepare32 (efiemu_prepare_hooks, efiemu_config_tables);
  else
    return grub_efiemu_prepare64 (efiemu_prepare_hooks, efiemu_config_tables);
}


static grub_err_t
grub_cmd_efiemu_load (grub_command_t cmd __attribute__ ((unused)),
		      int argc, char *args[])
{
  grub_err_t err;

  grub_efiemu_unload ();

  if (argc != 1)
    return grub_error (GRUB_ERR_BAD_ARGUMENT, N_("filename expected"));

  err = grub_efiemu_load_file (args[0]);
  if (err)
    return err;
#ifndef GRUB_MACHINE_EMU
  err = grub_machine_efiemu_init_tables ();
  if (err)
    return err;
#endif
  return GRUB_ERR_NONE;
}

static grub_command_t cmd_loadcore, cmd_prepare, cmd_unload;

GRUB_MOD_INIT(efiemu)
{
  cmd_loadcore = grub_register_command ("efiemu_loadcore",
					grub_cmd_efiemu_load,
					N_("FILE"),
					N_("Load and initialize EFI emulator."));
  cmd_prepare = grub_register_command ("efiemu_prepare",
				       grub_cmd_efiemu_prepare,
				       0,
				       N_("Finalize loading of EFI emulator."));
  cmd_unload = grub_register_command ("efiemu_unload", grub_cmd_efiemu_unload,
				      0,
				      N_("Unload EFI emulator."));
}

GRUB_MOD_FINI(efiemu)
{
  grub_unregister_command (cmd_loadcore);
  grub_unregister_command (cmd_prepare);
  grub_unregister_command (cmd_unload);
}
