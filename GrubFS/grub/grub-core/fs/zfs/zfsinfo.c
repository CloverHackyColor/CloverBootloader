/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2000,2001,2002,2003,2004,2009  Free Software Foundation, Inc.
 *  Copyright 2008  Sun Microsystems, Inc.
 *
 *  GRUB is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
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

#include <grub/zfs/zfs.h>
#include <grub/device.h>
#include <grub/file.h>
#include <grub/command.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/dl.h>
#include <grub/env.h>
#include <grub/i18n.h>

GRUB_MOD_LICENSE ("GPLv3+");

static inline void
print_tabs (int n)
{
  int i;

  for (i = 0; i < n; i++)
    grub_printf (" ");
}

static grub_err_t
print_state (char *nvlist, int tab)
{
  grub_uint64_t ival;
  int isok = 1;

  print_tabs (tab);

  if (grub_zfs_nvlist_lookup_uint64 (nvlist, ZPOOL_CONFIG_REMOVED, &ival))
    {
      grub_puts_ (N_("Virtual device is removed"));
      isok = 0;
    }

  if (grub_zfs_nvlist_lookup_uint64 (nvlist, ZPOOL_CONFIG_FAULTED, &ival))
    {
      grub_puts_ (N_("Virtual device is faulted"));
      isok = 0;
    }

  if (grub_zfs_nvlist_lookup_uint64 (nvlist, ZPOOL_CONFIG_OFFLINE, &ival))
    {
      grub_puts_ (N_("Virtual device is offline"));
      isok = 0;
    }

  if (grub_zfs_nvlist_lookup_uint64 (nvlist, ZPOOL_CONFIG_FAULTED, &ival))
    /* TRANSLATORS: degraded doesn't mean broken but that some of
       component are missing but virtual device as whole is still usable.  */
    grub_puts_ (N_("Virtual device is degraded"));

  if (isok)
    grub_puts_ (N_("Virtual device is online"));
  grub_xputs ("\n");

  return GRUB_ERR_NONE;
}

static grub_err_t
print_vdev_info (char *nvlist, int tab)
{
  char *type = 0;

  type = grub_zfs_nvlist_lookup_string (nvlist, ZPOOL_CONFIG_TYPE);

  if (!type)
    {
      print_tabs (tab);
      grub_puts_ (N_("Incorrect virtual device: no type available"));
      return grub_errno;
    }

  if (grub_strcmp (type, VDEV_TYPE_DISK) == 0)
    {
      char *bootpath = 0;
      char *path = 0;
      char *devid = 0;

      print_tabs (tab);
      /* TRANSLATORS: The virtual devices form a tree (in graph-theoretical
	 sense). The nodes like mirror or raidz have children: member devices.
	 The "real" devices which actually store data are called "leafs"
	 (again borrowed from graph theory) and can be either disks
	 (or partitions) or files.  */
      grub_puts_ (N_("Leaf virtual device (file or disk)"));

      print_state (nvlist, tab);

      bootpath =
	grub_zfs_nvlist_lookup_string (nvlist, ZPOOL_CONFIG_PHYS_PATH);
      print_tabs (tab);
      if (!bootpath)
	grub_puts_ (N_("Bootpath: unavailable\n"));
      else
	grub_printf_ (N_("Bootpath: %s\n"), bootpath);

      path = grub_zfs_nvlist_lookup_string (nvlist, "path");
      print_tabs (tab);
      if (!path)
	grub_puts_ (N_("Path: unavailable"));
      else
	grub_printf_ (N_("Path: %s\n"), path);

      devid = grub_zfs_nvlist_lookup_string (nvlist, ZPOOL_CONFIG_DEVID);
      print_tabs (tab);
      if (!devid)
	grub_puts_ (N_("Devid: unavailable"));
      else
	grub_printf_ (N_("Devid: %s\n"), devid);
      grub_free (bootpath);
      grub_free (devid);
      grub_free (path);
      return GRUB_ERR_NONE;
    }
  char is_mirror=(grub_strcmp(type,VDEV_TYPE_MIRROR) == 0);
  char is_raidz=(grub_strcmp(type,VDEV_TYPE_RAIDZ) == 0);

  if (is_mirror || is_raidz)
    {
      int nelm, i;

      nelm = grub_zfs_nvlist_lookup_nvlist_array_get_nelm
	(nvlist, ZPOOL_CONFIG_CHILDREN);

      if(is_mirror){
	 grub_puts_ (N_("This VDEV is a mirror"));
      }
      else if(is_raidz){
	 grub_uint64_t parity;
	 grub_zfs_nvlist_lookup_uint64(nvlist,"nparity",&parity);
	 grub_printf_ (N_("This VDEV is a RAIDZ%llu\n"),(unsigned long long)parity);
      }
      print_tabs (tab);
      if (nelm <= 0)
	{
	  grub_puts_ (N_("Incorrect VDEV"));
	  return GRUB_ERR_NONE;
	}
      grub_printf_ (N_("VDEV with %d children\n"), nelm);
      print_state (nvlist, tab);
      for (i = 0; i < nelm; i++)
	{
	  char *child;

	  child = grub_zfs_nvlist_lookup_nvlist_array
	    (nvlist, ZPOOL_CONFIG_CHILDREN, i);

	  print_tabs (tab);
	  if (!child)
	    {
	      /* TRANSLATORS: it's the element carying the number %d, not
		 total element number. And the number itself is fine,
		 only the element isn't.
	      */
	      grub_printf_ (N_("VDEV element number %d isn't correct\n"), i);
	      continue;
	    }

	  /* TRANSLATORS: it's the element carying the number %d, not
	     total element number. This is used in enumeration
	     "Element number 1", "Element number 2", ... */
	  grub_printf_ (N_("VDEV element number %d:\n"), i);
	  print_vdev_info (child, tab + 1);

	  grub_free (child);
	}
      return GRUB_ERR_NONE;
    }

  print_tabs (tab);
  grub_printf_ (N_("Unknown virtual device type: %s\n"), type);

  return GRUB_ERR_NONE;
}

static grub_err_t
get_bootpath (char *nvlist, char **bootpath, char **devid)
{
  char *type = 0;

  type = grub_zfs_nvlist_lookup_string (nvlist, ZPOOL_CONFIG_TYPE);

  if (!type)
    return grub_errno;

  if (grub_strcmp (type, VDEV_TYPE_DISK) == 0)
    {
      *bootpath = grub_zfs_nvlist_lookup_string (nvlist,
						 ZPOOL_CONFIG_PHYS_PATH);
      *devid = grub_zfs_nvlist_lookup_string (nvlist, ZPOOL_CONFIG_DEVID);
      if (!*bootpath || !*devid)
	{
	  grub_free (*bootpath);
	  grub_free (*devid);
	  *bootpath = 0;
	  *devid = 0;
	}
      return GRUB_ERR_NONE;
    }

  if (grub_strcmp (type, VDEV_TYPE_MIRROR) == 0)
    {
      int nelm, i;

      nelm = grub_zfs_nvlist_lookup_nvlist_array_get_nelm
	(nvlist, ZPOOL_CONFIG_CHILDREN);

      for (i = 0; i < nelm; i++)
	{
	  char *child;

	  child = grub_zfs_nvlist_lookup_nvlist_array (nvlist,
						       ZPOOL_CONFIG_CHILDREN,
						       i);

	  get_bootpath (child, bootpath, devid);

	  grub_free (child);

	  if (*bootpath && *devid)
	    return GRUB_ERR_NONE;
	}
    }

  return GRUB_ERR_NONE;
}

static const char *poolstates[] = {
  /* TRANSLATORS: Here we speak about ZFS pools it's semi-marketing,
     semi-technical term by Sun/Oracle and should be translated in sync with
     other ZFS-related software and documentation.  */
  [POOL_STATE_ACTIVE] = N_("Pool state: active"),
  [POOL_STATE_EXPORTED] = N_("Pool state: exported"),
  [POOL_STATE_DESTROYED] = N_("Pool state: destroyed"),
  [POOL_STATE_SPARE] = N_("Pool state: reserved for hot spare"),
  [POOL_STATE_L2CACHE] = N_("Pool state: level 2 ARC device"),
  [POOL_STATE_UNINITIALIZED] = N_("Pool state: uninitialized"),
  [POOL_STATE_UNAVAIL] = N_("Pool state: unavailable"),
  [POOL_STATE_POTENTIALLY_ACTIVE] = N_("Pool state: potentially active")
};

static grub_err_t
grub_cmd_zfsinfo (grub_command_t cmd __attribute__ ((unused)), int argc,
		  char **args)
{
  grub_device_t dev;
  char *devname;
  grub_err_t err;
  char *nvlist = 0;
  char *nv = 0;
  char *poolname;
  grub_uint64_t guid;
  grub_uint64_t pool_state;
  int found;

  if (argc < 1)
    return grub_error (GRUB_ERR_BAD_ARGUMENT, N_("one argument expected"));

  if (args[0][0] == '(' && args[0][grub_strlen (args[0]) - 1] == ')')
    {
      devname = grub_strdup (args[0] + 1);
      if (devname)
	devname[grub_strlen (devname) - 1] = 0;
    }
  else
    devname = grub_strdup (args[0]);
  if (!devname)
    return grub_errno;

  dev = grub_device_open (devname);
  grub_free (devname);
  if (!dev)
    return grub_errno;

  err = grub_zfs_fetch_nvlist (dev, &nvlist);

  grub_device_close (dev);

  if (err)
    return err;

  poolname = grub_zfs_nvlist_lookup_string (nvlist, ZPOOL_CONFIG_POOL_NAME);
  if (!poolname)
    grub_puts_ (N_("Pool name: unavailable"));
  else
    grub_printf_ (N_("Pool name: %s\n"), poolname);

  found =
    grub_zfs_nvlist_lookup_uint64 (nvlist, ZPOOL_CONFIG_POOL_GUID, &guid);
  if (!found)
    grub_puts_ (N_("Pool GUID: unavailable"));
  else
    grub_printf_ (N_("Pool GUID: %016llx\n"), (long long unsigned) guid);

  found = grub_zfs_nvlist_lookup_uint64 (nvlist, ZPOOL_CONFIG_POOL_STATE,
					 &pool_state);
  if (!found)
    grub_puts_ (N_("Unable to retrieve pool state"));
  else if (pool_state >= ARRAY_SIZE (poolstates))
    grub_puts_ (N_("Unrecognized pool state"));
  else
    grub_puts_ (poolstates[pool_state]);

  nv = grub_zfs_nvlist_lookup_nvlist (nvlist, ZPOOL_CONFIG_VDEV_TREE);

  if (!nv)
    /* TRANSLATORS: There are undetermined number of virtual devices
       in a device tree, not just one.
     */
    grub_puts_ (N_("No virtual device tree available"));
  else
    print_vdev_info (nv, 1);

  grub_free (nv);
  grub_free (nvlist);

  return GRUB_ERR_NONE;
}

static grub_err_t
grub_cmd_zfs_bootfs (grub_command_t cmd __attribute__ ((unused)), int argc,
		     char **args)
{
  grub_device_t dev;
  char *devname;
  grub_err_t err;
  char *nvlist = 0;
  char *nv = 0;
  char *bootpath = 0, *devid = 0;
  char *fsname;
  char *bootfs;
  char *poolname;
  grub_uint64_t mdnobj;

  if (argc < 1)
    return grub_error (GRUB_ERR_BAD_ARGUMENT, N_("one argument expected"));

  devname = grub_file_get_device_name (args[0]);
  if (grub_errno)
    return grub_errno;

  dev = grub_device_open (devname);
  grub_free (devname);
  if (!dev)
    return grub_errno;

  err = grub_zfs_fetch_nvlist (dev, &nvlist);

  fsname = grub_strchr (args[0], ')');
  if (fsname)
    fsname++;
  else
    fsname = args[0];

  if (!err)
    err = grub_zfs_getmdnobj (dev, fsname, &mdnobj);

  grub_device_close (dev);

  if (err)
    return err;

  poolname = grub_zfs_nvlist_lookup_string (nvlist, ZPOOL_CONFIG_POOL_NAME);
  if (!poolname)
    {
      if (!grub_errno)
	grub_error (GRUB_ERR_BAD_FS, "No poolname found");
      return grub_errno;
    }

  nv = grub_zfs_nvlist_lookup_nvlist (nvlist, ZPOOL_CONFIG_VDEV_TREE);

  if (nv)
    get_bootpath (nv, &bootpath, &devid);

  grub_free (nv);
  grub_free (nvlist);

  bootfs = grub_xasprintf ("zfs-bootfs=%s/%llu%s%s%s%s%s%s",
			   poolname, (unsigned long long) mdnobj,
			   bootpath ? ",bootpath=\"" : "",
			   bootpath ? : "",
			   bootpath ? "\"" : "",
			   devid ? ",diskdevid=\"" : "",
			   devid ? : "",
			   devid ? "\"" : "");
  if (!bootfs)
    return grub_errno;
  if (argc >= 2)
    grub_env_set (args[1], bootfs);
  else
    grub_printf ("%s\n", bootfs);

  grub_free (bootfs);
  grub_free (poolname);
  grub_free (bootpath);
  grub_free (devid);

  return GRUB_ERR_NONE;
}


static grub_command_t cmd_info, cmd_bootfs;

GRUB_MOD_INIT (zfsinfo)
{
  cmd_info = grub_register_command ("zfsinfo", grub_cmd_zfsinfo,
				    N_("DEVICE"),
				    N_("Print ZFS info about DEVICE."));
  cmd_bootfs = grub_register_command ("zfs-bootfs", grub_cmd_zfs_bootfs,
				      N_("FILESYSTEM [VARIABLE]"),
				      N_("Print ZFS-BOOTFSOBJ or store it into VARIABLE"));
}

GRUB_MOD_FINI (zfsinfo)
{
  grub_unregister_command (cmd_info);
  grub_unregister_command (cmd_bootfs);
}
