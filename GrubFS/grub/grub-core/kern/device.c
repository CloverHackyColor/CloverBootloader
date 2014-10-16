/* device.c - device manager */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2005,2007,2008,2009  Free Software Foundation, Inc.
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

#include <grub/device.h>
#include <grub/disk.h>
#include <grub/net.h>
#include <grub/fs.h>
#include <grub/mm.h>
#include <grub/misc.h>
#include <grub/env.h>
#include <grub/partition.h>
#include <grub/i18n.h>

grub_net_t (*grub_net_open) (const char *name) = NULL;

grub_device_t
grub_device_open (const char *name)
{
  grub_device_t dev = 0;

  if (! name)
    {
      name = grub_env_get ("root");
      if (name == NULL || *name == '\0')
	{
	  grub_error (GRUB_ERR_BAD_DEVICE,  N_("variable `%s' isn't set"), "root");
	  goto fail;
	}
    }

  dev = grub_malloc (sizeof (*dev));
  if (! dev)
    goto fail;

  dev->net = NULL;
  /* Try to open a disk.  */
  dev->disk = grub_disk_open (name);
  if (dev->disk)
    return dev;
  if (grub_net_open && grub_errno == GRUB_ERR_UNKNOWN_DEVICE)
    {
      grub_errno = GRUB_ERR_NONE;
      dev->net = grub_net_open (name); 
    }

  if (dev->net)
    return dev;

 fail:
  grub_free (dev);

  return 0;
}

grub_err_t
grub_device_close (grub_device_t device)
{
  if (device->disk)
    grub_disk_close (device->disk);

  if (device->net)
    {
      grub_free (device->net->server);
      grub_free (device->net);
    }

  grub_free (device);

  return grub_errno;
}

struct part_ent
{
  struct part_ent *next;
  char *name;
};

/* Context for grub_device_iterate.  */
struct grub_device_iterate_ctx
{
  grub_device_iterate_hook_t hook;
  void *hook_data;
  struct part_ent *ents;
};

/* Helper for grub_device_iterate.  */
static int
iterate_partition (grub_disk_t disk, const grub_partition_t partition,
		   void *data)
{
  struct grub_device_iterate_ctx *ctx = data;
  struct part_ent *p;
  char *part_name;

  p = grub_malloc (sizeof (*p));
  if (!p)
    {
      return 1;
    }

  part_name = grub_partition_get_name (partition);
  if (!part_name)
    {
      grub_free (p);
      return 1;
    }
  p->name = grub_xasprintf ("%s,%s", disk->name, part_name);
  grub_free (part_name);
  if (!p->name)
    {
      grub_free (p);
      return 1;
    }

  p->next = ctx->ents;
  ctx->ents = p;

  return 0;
}

/* Helper for grub_device_iterate.  */
static int
iterate_disk (const char *disk_name, void *data)
{
  struct grub_device_iterate_ctx *ctx = data;
  grub_device_t dev;

  if (ctx->hook (disk_name, ctx->hook_data))
    return 1;

  dev = grub_device_open (disk_name);
  if (! dev)
    {
      grub_errno = GRUB_ERR_NONE;
      return 0;
    }

  if (dev->disk)
    {
      struct part_ent *p;
      int ret = 0;

      ctx->ents = NULL;
      (void) grub_partition_iterate (dev->disk, iterate_partition, ctx);
      grub_device_close (dev);

      grub_errno = GRUB_ERR_NONE;

      p = ctx->ents;
      while (p != NULL)
	{
	  struct part_ent *next = p->next;

	  if (!ret)
	    ret = ctx->hook (p->name, ctx->hook_data);
	  grub_free (p->name);
	  grub_free (p);
	  p = next;
	}

      return ret;
    }

  grub_device_close (dev);
  return 0;
}

int
grub_device_iterate (grub_device_iterate_hook_t hook, void *hook_data)
{
  struct grub_device_iterate_ctx ctx = { hook, hook_data, NULL };

  /* Only disk devices are supported at the moment.  */
  return grub_disk_dev_iterate (iterate_disk, &ctx);
}
