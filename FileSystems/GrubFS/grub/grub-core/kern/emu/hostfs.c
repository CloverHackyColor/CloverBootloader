/* hostfs.c - Dummy filesystem to provide access to the hosts filesystem  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2007,2008,2009,2010  Free Software Foundation, Inc.
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

#include <config-util.h>

#define _BSD_SOURCE
#include <grub/fs.h>
#include <grub/file.h>
#include <grub/disk.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/dl.h>
#include <grub/util/misc.h>
#include <grub/emu/hostdisk.h>
#include <grub/i18n.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>

static int
is_dir (const char *path, const char *name)
{
  int len1 = strlen(path);
  int len2 = strlen(name);
  int ret;

  char *pathname = xmalloc (len1 + 1 + len2 + 1 + 13);
  strcpy (pathname, path);

  /* Avoid UNC-path "//name" on Cygwin.  */
  if (len1 > 0 && pathname[len1 - 1] != '/')
    strcat (pathname, "/");

  strcat (pathname, name);

  ret = grub_util_is_directory (pathname);
  free (pathname);
  return ret;
}

struct grub_hostfs_data
{
  char *filename;
  grub_util_fd_t f;
};

static grub_err_t
grub_hostfs_dir (grub_device_t device, const char *path,
		 grub_fs_dir_hook_t hook, void *hook_data)
{
  grub_util_fd_dir_t dir;

  /* Check if the disk is our dummy disk.  */
  if (grub_strcmp (device->disk->name, "host"))
    return grub_error (GRUB_ERR_BAD_FS, "not a hostfs");

  dir = grub_util_fd_opendir (path);
  if (! dir)
    return grub_error (GRUB_ERR_BAD_FILENAME,
		       N_("can't open `%s': %s"), path,
		       grub_util_fd_strerror ());

  while (1)
    {
      grub_util_fd_dirent_t de;
      struct grub_dirhook_info info;
      grub_memset (&info, 0, sizeof (info));

      de = grub_util_fd_readdir (dir);
      if (! de)
	break;

      info.dir = !! is_dir (path, de->d_name);
      hook (de->d_name, &info, hook_data);

    }

  grub_util_fd_closedir (dir);

  return GRUB_ERR_NONE;
}

/* Open a file named NAME and initialize FILE.  */
static grub_err_t
grub_hostfs_open (struct grub_file *file, const char *name)
{
  grub_util_fd_t f;
  struct grub_hostfs_data *data;

  f = grub_util_fd_open (name, GRUB_UTIL_FD_O_RDONLY);
  if (! GRUB_UTIL_FD_IS_VALID (f))
    return grub_error (GRUB_ERR_BAD_FILENAME,
		       N_("can't open `%s': %s"), name,
		       strerror (errno));
  data = grub_malloc (sizeof (*data));
  if (!data)
    {
      grub_util_fd_close (f);
      return grub_errno;
    }
  data->filename = grub_strdup (name);
  if (!data->filename)
    {
      grub_free (data);
      grub_util_fd_close (f);
      return grub_errno;
    }

  data->f = f;  

  file->data = data;

  file->size = grub_util_get_fd_size (f, name, NULL);

  return GRUB_ERR_NONE;
}

static grub_ssize_t
grub_hostfs_read (grub_file_t file, char *buf, grub_size_t len)
{
  struct grub_hostfs_data *data;

  data = file->data;
  if (grub_util_fd_seek (data->f, file->offset) != 0)
    {
      grub_error (GRUB_ERR_OUT_OF_RANGE, N_("cannot seek `%s': %s"),
		  data->filename, grub_util_fd_strerror ());
      return -1;
    }

  unsigned int s = grub_util_fd_read (data->f, buf, len);
  if (s != len)
    grub_error (GRUB_ERR_FILE_READ_ERROR, N_("cannot read `%s': %s"),
		data->filename, grub_util_fd_strerror ());

  return (signed) s;
}

static grub_err_t
grub_hostfs_close (grub_file_t file)
{
  struct grub_hostfs_data *data;

  data = file->data;
  grub_util_fd_close (data->f);
  grub_free (data->filename);
  grub_free (data);

  return GRUB_ERR_NONE;
}

static grub_err_t
grub_hostfs_label (grub_device_t device __attribute ((unused)),
		   char **label __attribute ((unused)))
{
  *label = 0;
  return GRUB_ERR_NONE;
}

static struct grub_fs grub_hostfs_fs =
  {
    .name = "hostfs",
    .dir = grub_hostfs_dir,
    .open = grub_hostfs_open,
    .read = grub_hostfs_read,
    .close = grub_hostfs_close,
    .label = grub_hostfs_label,
    .next = 0
  };



GRUB_MOD_INIT(hostfs)
{
  grub_fs_register (&grub_hostfs_fs);
}

GRUB_MOD_FINI(hostfs)
{
  grub_fs_unregister (&grub_hostfs_fs);
}
