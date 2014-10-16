/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2004,2007  Free Software Foundation, Inc.
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
#include <grub/mm.h>
#include <grub/partition.h>
#include <grub/disk.h>
#include <grub/i18n.h>

#ifdef GRUB_UTIL
#include <grub/util/misc.h>
#endif

grub_partition_map_t grub_partition_map_list;

/*
 * Checks that disk->partition contains part.  This function assumes that the
 * start of part is relative to the start of disk->partition.  Returns 1 if
 * disk->partition is null.
 */
static int
grub_partition_check_containment (const grub_disk_t disk,
				  const grub_partition_t part)
{
  if (disk->partition == NULL)
    return 1;

  if (part->start + part->len > disk->partition->len)
    {
      char *partname;

      partname = grub_partition_get_name (disk->partition);
      grub_dprintf ("partition", "sub-partition %s%d of (%s,%s) ends after parent.\n",
		    part->partmap->name, part->number + 1, disk->name, partname);
#ifdef GRUB_UTIL
      grub_util_warn (_("Discarding improperly nested partition (%s,%s,%s%d)"),
		      disk->name, partname, part->partmap->name, part->number + 1);
#endif
      grub_free (partname);

      return 0;
    }

  return 1;
}

/* Context for grub_partition_map_probe.  */
struct grub_partition_map_probe_ctx
{
  int partnum;
  grub_partition_t p;
};

/* Helper for grub_partition_map_probe.  */
static int
probe_iter (grub_disk_t dsk, const grub_partition_t partition, void *data)
{
  struct grub_partition_map_probe_ctx *ctx = data;

  if (ctx->partnum != partition->number)
    return 0;

  if (!(grub_partition_check_containment (dsk, partition)))
    return 0;

  ctx->p = (grub_partition_t) grub_malloc (sizeof (*ctx->p));
  if (! ctx->p)
    return 1;

  grub_memcpy (ctx->p, partition, sizeof (*ctx->p));
  return 1;
}

static grub_partition_t
grub_partition_map_probe (const grub_partition_map_t partmap,
			  grub_disk_t disk, int partnum)
{
  struct grub_partition_map_probe_ctx ctx = {
    .partnum = partnum,
    .p = 0
  };

  partmap->iterate (disk, probe_iter, &ctx);
  if (grub_errno)
    goto fail;

  return ctx.p;

 fail:
  grub_free (ctx.p);
  return 0;
}

grub_partition_t
grub_partition_probe (struct grub_disk *disk, const char *str)
{
  grub_partition_t part = 0;
  grub_partition_t curpart = 0;
  grub_partition_t tail;
  const char *ptr;

  part = tail = disk->partition;

  for (ptr = str; *ptr;)
    {
      grub_partition_map_t partmap;
      int num;
      const char *partname, *partname_end;

      partname = ptr;
      while (*ptr && grub_isalpha (*ptr))
	ptr++;
      partname_end = ptr; 
      num = grub_strtoul (ptr, (char **) &ptr, 0) - 1;

      curpart = 0;
      /* Use the first partition map type found.  */
      FOR_PARTITION_MAPS(partmap)
      {
	if (partname_end != partname &&
	    (grub_strncmp (partmap->name, partname, partname_end - partname)
	     != 0 || partmap->name[partname_end - partname] != 0))
	  continue;

	disk->partition = part;
	curpart = grub_partition_map_probe (partmap, disk, num);
	disk->partition = tail;
	if (curpart)
	  break;

	if (grub_errno == GRUB_ERR_BAD_PART_TABLE)
	  {
	    /* Continue to next partition map type.  */
	    grub_errno = GRUB_ERR_NONE;
	    continue;
	  }

	break;
      }

      if (! curpart)
	{
	  while (part)
	    {
	      curpart = part->parent;
	      grub_free (part);
	      part = curpart;
	    }
	  return 0;
	}
      curpart->parent = part;
      part = curpart;
      if (! ptr || *ptr != ',')
	break;
      ptr++;
    }

  return part;
}

/* Context for grub_partition_iterate.  */
struct grub_partition_iterate_ctx
{
  int ret;
  grub_partition_iterate_hook_t hook;
  void *hook_data;
};

/* Helper for grub_partition_iterate.  */
static int
part_iterate (grub_disk_t dsk, const grub_partition_t partition, void *data)
{
  struct grub_partition_iterate_ctx *ctx = data;
  struct grub_partition p = *partition;

  if (!(grub_partition_check_containment (dsk, partition)))
    return 0;

  p.parent = dsk->partition;
  dsk->partition = 0;
  if (ctx->hook (dsk, &p, ctx->hook_data))
    {
      ctx->ret = 1;
      return 1;
    }
  if (p.start != 0)
    {
      const struct grub_partition_map *partmap;
      dsk->partition = &p;
      FOR_PARTITION_MAPS(partmap)
      {
	grub_err_t err;
	err = partmap->iterate (dsk, part_iterate, ctx);
	if (err)
	  grub_errno = GRUB_ERR_NONE;
	if (ctx->ret)
	  break;
      }
    }
  dsk->partition = p.parent;
  return ctx->ret;
}

int
grub_partition_iterate (struct grub_disk *disk,
			grub_partition_iterate_hook_t hook, void *hook_data)
{
  struct grub_partition_iterate_ctx ctx = {
    .ret = 0,
    .hook = hook,
    .hook_data = hook_data
  };
  const struct grub_partition_map *partmap;

  FOR_PARTITION_MAPS(partmap)
  {
    grub_err_t err;
    err = partmap->iterate (disk, part_iterate, &ctx);
    if (err)
      grub_errno = GRUB_ERR_NONE;
    if (ctx.ret)
      break;
  }

  return ctx.ret;
}

char *
grub_partition_get_name (const grub_partition_t partition)
{
  char *out = 0, *ptr;
  grub_size_t needlen = 0;
  grub_partition_t part;
  if (!partition)
    return grub_strdup ("");
  for (part = partition; part; part = part->parent)
    /* Even on 64-bit machines this buffer is enough to hold
       longest number.  */
    needlen += grub_strlen (part->partmap->name) + 1 + 27;
  out = grub_malloc (needlen + 1);
  if (!out)
    return NULL;

  ptr = out + needlen;
  *ptr = 0;
  for (part = partition; part; part = part->parent)
    {
      char buf[27];
      grub_size_t len;
      grub_snprintf (buf, sizeof (buf), "%d", part->number + 1);
      len = grub_strlen (buf);
      ptr -= len;
      grub_memcpy (ptr, buf, len);
      len = grub_strlen (part->partmap->name);
      ptr -= len;
      grub_memcpy (ptr, part->partmap->name, len);
      *--ptr = ',';
    }
  grub_memmove (out, ptr + 1, out + needlen - ptr);
  return out;
}
