/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2013  Free Software Foundation, Inc.
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

#include <grub/disk.h>
#include <grub/procfs.h>
#include <grub/fs.h>
#include <grub/file.h>
#include <grub/mm.h>
#include <grub/dl.h>

GRUB_MOD_LICENSE ("GPLv3+");

grub_disk_dev_t grub_disk_dev_list;

void
grub_disk_dev_register (grub_disk_dev_t dev)
{
    dev->next = grub_disk_dev_list;
    grub_disk_dev_list = dev;
}

void
grub_disk_dev_unregister (grub_disk_dev_t dev)
{
    grub_disk_dev_t *p, q;
    
    for (p = &grub_disk_dev_list, q = *p; q; p = &(q->next), q = q->next)
        if (q == dev)
        {
            *p = q->next;
            break;
        }
}

struct grub_procfs_entry *grub_procfs_entries;

static int
grub_procdev_iterate (grub_disk_dev_iterate_hook_t hook, void *hook_data,
			 grub_disk_pull_t pull)
{
  if (pull != GRUB_DISK_PULL_NONE)
    return 0;

  return hook ("proc", hook_data);
}

static grub_err_t
grub_procdev_open (const char *name, grub_disk_t disk)
{
  if (grub_strcmp (name, "proc"))
      return grub_error (GRUB_ERR_UNKNOWN_DEVICE, "not a procfs disk");

  disk->total_sectors = 0;
  disk->id = 0;

  disk->data = 0;

  return GRUB_ERR_NONE;
}

static void
grub_procdev_close (grub_disk_t disk __attribute((unused)))
{
}

static grub_err_t
grub_procdev_read (grub_disk_t disk __attribute((unused)),
		grub_disk_addr_t sector __attribute((unused)),
		grub_size_t size __attribute((unused)),
		char *buf __attribute((unused)))
{
  return GRUB_ERR_OUT_OF_RANGE;
}

static grub_err_t
grub_procdev_write (grub_disk_t disk __attribute ((unused)),
		       grub_disk_addr_t sector __attribute ((unused)),
		       grub_size_t size __attribute ((unused)),
		       const char *buf __attribute ((unused)))
{
  return GRUB_ERR_OUT_OF_RANGE;
}

static grub_ssize_t
grub_procfs_read (grub_file_t file, char *buf, grub_size_t len)
{
  char *data = file->data;

  grub_memcpy (buf, data + file->offset, len);

  return len;
}

static grub_err_t
grub_procfs_close (grub_file_t file)
{
  char *data;

  data = file->data;
  grub_free (data);

  return GRUB_ERR_NONE;
}

static grub_err_t
grub_procfs_dir (grub_device_t device, const char *path,
		 grub_fs_dir_hook_t hook, void *hook_data)
{
  const char *ptr;
  struct grub_dirhook_info info;
  struct grub_procfs_entry *entry;

  grub_memset (&info, 0, sizeof (info));

  /* Check if the disk is our dummy disk.  */
  if (grub_strcmp (device->disk->name, "proc"))
    return grub_error (GRUB_ERR_BAD_FS, "not a procfs");
  for (ptr = path; *ptr == '/'; ptr++);
  if (*ptr)
    return 0;
  FOR_LIST_ELEMENTS((entry), (grub_procfs_entries))
    if (hook (entry->name, &info, hook_data))
      return 0;
  return 0;
}

static grub_err_t
grub_procfs_open (struct grub_file *file, const char *path)
{
  const char *pathptr;
  struct grub_procfs_entry *entry;

  for (pathptr = path; *pathptr == '/'; pathptr++);

  FOR_LIST_ELEMENTS((entry), (grub_procfs_entries))
    if (grub_strcmp (pathptr, entry->name) == 0)
    {
      grub_size_t sz;
      file->data = entry->get_contents (&sz);
      if (!file->data)
	return grub_errno;
      file->size = sz;
      return GRUB_ERR_NONE;
    }

  return grub_error (GRUB_ERR_FILE_NOT_FOUND, N_("file `%s' not found"), path);
}

static struct grub_disk_dev grub_procfs_dev = {
  .name = "proc",
  .id = GRUB_DISK_DEVICE_PROCFS_ID,
  .iterate = grub_procdev_iterate,
  .open = grub_procdev_open,
  .close = grub_procdev_close,
  .read = grub_procdev_read,
  .write = grub_procdev_write,
  .next = 0
};

static struct grub_fs grub_procfs_fs =
  {
    .name = "procfs",
    .dir = grub_procfs_dir,
    .open = grub_procfs_open,
    .read = grub_procfs_read,
    .close = grub_procfs_close,
    .next = 0
  };

GRUB_MOD_INIT (procfs)
{
  grub_disk_dev_register (&grub_procfs_dev);
  grub_fs_register (&grub_procfs_fs);
}

GRUB_MOD_FINI (procfs)
{
  grub_disk_dev_unregister (&grub_procfs_dev);
  grub_fs_unregister (&grub_procfs_fs);
}
