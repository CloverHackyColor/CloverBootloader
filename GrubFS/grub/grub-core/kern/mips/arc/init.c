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

#include <grub/kernel.h>
#include <grub/misc.h>
#include <grub/env.h>
#include <grub/time.h>
#include <grub/types.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/time.h>
#include <grub/machine/kernel.h>
#include <grub/machine/memory.h>
#include <grub/arc/console.h>
#include <grub/cpu/memory.h>
#include <grub/cpu/time.h>
#include <grub/memory.h>
#include <grub/term.h>
#include <grub/arc/arc.h>
#include <grub/offsets.h>
#include <grub/i18n.h>
#include <grub/disk.h>
#include <grub/partition.h>

const char *type_names[] = {
#ifdef GRUB_CPU_WORDS_BIGENDIAN
  NULL,
#endif
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  "eisa", "tc", "scsi", "dti", "multi", "disk", "tape", "cdrom", "worm",
  "serial", "net", "video", "par", "point", "key", "audio", "other",
  "rdisk", "fdisk", "tape", "modem", "monitor", "print", "pointer",
  "keyboard", "term",
#ifndef GRUB_CPU_WORDS_BIGENDIAN
  "other",
#endif
  "line", "network", NULL
};

static int
iterate_rec (const char *prefix, const struct grub_arc_component *parent,
	     grub_arc_iterate_devs_hook_t hook, void *hook_data,
	     int alt_names)
{
  const struct grub_arc_component *comp;
  FOR_ARC_CHILDREN(comp, parent)
    {
      char *name;
      const char *cname = NULL;
      if (comp->type < ARRAY_SIZE (type_names))
	cname = type_names[comp->type];
      if (!cname)
	cname = "unknown";
      if (alt_names)
	name = grub_xasprintf ("%s/%s%lu", prefix, cname, comp->key);
      else
	name = grub_xasprintf ("%s%s(%lu)", prefix, cname, comp->key);
      if (!name)
	return 1;
      if (hook (name, comp, hook_data))
	{
	  grub_free (name);
	  return 1;
	}
      if (iterate_rec ((parent ? name : prefix), comp, hook, hook_data,
		       alt_names))
	{
	  grub_free (name);
	  return 1;
	}
      grub_free (name);
    }
  return 0;
}

int
grub_arc_iterate_devs (grub_arc_iterate_devs_hook_t hook, void *hook_data,
		       int alt_names)
{
  return iterate_rec ((alt_names ? "arc" : ""), NULL, hook, hook_data,
		      alt_names);
}

grub_err_t
grub_machine_mmap_iterate (grub_memory_hook_t hook, void *hook_data)
{
  struct grub_arc_memory_descriptor *cur = NULL;
  while (1)
    {
      grub_memory_type_t type;
      cur = GRUB_ARC_FIRMWARE_VECTOR->getmemorydescriptor (cur);
      if (!cur)
	return GRUB_ERR_NONE;
      switch (cur->type)
	{
	case GRUB_ARC_MEMORY_EXCEPTION_BLOCK:
	case GRUB_ARC_MEMORY_SYSTEM_PARAMETER_BLOCK:
	case GRUB_ARC_MEMORY_FW_PERMANENT:
	default:
	  type = GRUB_MEMORY_RESERVED;
	  break;

	case GRUB_ARC_MEMORY_FW_TEMPORARY:
	case GRUB_ARC_MEMORY_FREE:
	case GRUB_ARC_MEMORY_LOADED:
	case GRUB_ARC_MEMORY_FREE_CONTIGUOUS:
	  type = GRUB_MEMORY_AVAILABLE;
	  break;
	case GRUB_ARC_MEMORY_BADRAM:
	  type = GRUB_MEMORY_BADRAM;
	  break;
	}
      if (hook (((grub_uint64_t) cur->start_page) << 12,
		((grub_uint64_t) cur->num_pages)  << 12, type, hook_data))
	return GRUB_ERR_NONE;
    }
}

char *
grub_arc_alt_name_to_norm (const char *name, const char *suffix)
{
  char *optr;
  const char *iptr;
  char * ret = grub_malloc (2 * grub_strlen (name) + grub_strlen (suffix));
  int state = 0;

  if (!ret)
    return NULL;
  optr = ret;
  for (iptr = name + 4; *iptr; iptr++)
    if (state == 0)
      {
	if (!grub_isdigit (*iptr))
	  *optr++ = *iptr;
	else
	  {
	    *optr++ = '(';
	    *optr++ = *iptr;
	    state = 1;
	  }
      }
    else
      {
	if (grub_isdigit (*iptr))
	  *optr++ = *iptr;
	else
	  {
	    *optr++ = ')';
	    state = 0;
	  }
      }
  if (state)
    *optr++ = ')';
  grub_strcpy (optr, suffix);
  return ret;
}

static char *
norm_name_to_alt (const char *name)
{
  char *optr;
  const char *iptr;
  int state = 0;
  char * ret = grub_malloc (grub_strlen (name) + sizeof ("arc/"));

  if (!ret)
    return NULL;
  optr = grub_stpcpy (ret, "arc/");
  for (iptr = name; *iptr; iptr++)
    {
      if (state == 3)
	{
	  *optr++ = '/';
	  state = 0;
	}
      if (*iptr == '(')
	{
	  state = 1;
	  continue;
	}
      if (*iptr == ')')
	{
	  if (state == 1)
	    *optr++ = '0';
	  state = 3;
	  continue;
	}
      *optr++ = *iptr;
      if (state == 1)
	state = 2;
    }
  *optr = '\0';
  return ret;
}

extern grub_uint32_t grub_total_modules_size __attribute__ ((section(".text")));
grub_addr_t grub_modbase;

extern char _end[];
static char boot_location[256];

void
grub_machine_init (void)
{
  struct grub_arc_memory_descriptor *cur = NULL;
  grub_addr_t modend;

  grub_memcpy (boot_location,
	       (char *) (GRUB_DECOMPRESSOR_LINK_ADDR - 256), 256);

  grub_modbase = ALIGN_UP ((grub_addr_t) _end, GRUB_KERNEL_MACHINE_MOD_ALIGN);
  modend = grub_modbase + grub_total_modules_size;
  grub_console_init_early ();

  /* FIXME: measure this.  */
  grub_arch_cpuclock = 150000000;
  grub_install_get_time_ms (grub_rtc_get_time_ms);

  while (1)
    {
      grub_uint64_t start, end;
      cur = GRUB_ARC_FIRMWARE_VECTOR->getmemorydescriptor (cur);
      if (!cur)
	break;
      if (cur->type != GRUB_ARC_MEMORY_FREE
	  && cur->type != GRUB_ARC_MEMORY_LOADED
	  && cur->type != GRUB_ARC_MEMORY_FREE_CONTIGUOUS)
	continue;
      start = ((grub_uint64_t) cur->start_page) << 12;
      end = ((grub_uint64_t) cur->num_pages)  << 12;
      end += start;
      if ((grub_uint64_t) start < (modend & 0x1fffffff))
	start = (modend & 0x1fffffff);
      if ((grub_uint64_t) end > 0x20000000)
	end = 0x20000000;
      if (end > start)
	grub_mm_init_region ((void *) (grub_addr_t) (start | 0x80000000),
			     end - start);
    }

  grub_console_init_lately ();

  grub_arcdisk_init ();
}

void
grub_machine_fini (int flags __attribute__ ((unused)))
{
}

void
grub_halt (void)
{
  GRUB_ARC_FIRMWARE_VECTOR->powerdown ();

  grub_millisleep (1500);

  grub_puts_ (N_("Shutdown failed"));
  grub_refresh ();
  while (1);
}

void
grub_exit (void)
{
  GRUB_ARC_FIRMWARE_VECTOR->exit ();

  grub_millisleep (1500);

  grub_puts_ (N_("Exit failed"));
  grub_refresh ();
  while (1);
}

static char *
get_part (char *dev)
{
  char *ptr;
  if (!*dev)
    return 0;
  ptr = dev + grub_strlen (dev) - 1;
  if (ptr == dev || *ptr != ')')
    return 0;
  ptr--;
  while (grub_isdigit (*ptr) && ptr > dev)
    ptr--;
  if (*ptr != '(' || ptr == dev)
    return 0;
  ptr--;
  if (ptr - dev < (int) sizeof ("partition") - 2)
    return 0;
  ptr -= sizeof ("partition") - 2;
  if (grub_memcmp (ptr, "partition", sizeof ("partition") - 1) != 0)
    return 0;
  return ptr;
}

static grub_disk_addr_t
get_partition_offset (char *part, grub_disk_addr_t *en)
{
  grub_arc_fileno_t handle;
  grub_disk_addr_t ret = -1;
  struct grub_arc_fileinfo info;
  grub_arc_err_t r;

  if (GRUB_ARC_FIRMWARE_VECTOR->open (part, GRUB_ARC_FILE_ACCESS_OPEN_RO,
				      &handle))
    return -1;

  r = GRUB_ARC_FIRMWARE_VECTOR->getfileinformation (handle, &info);
  if (!r)
    {
      ret = (info.start >> 9);
      *en = (info.end >> 9);
    }
  GRUB_ARC_FIRMWARE_VECTOR->close (handle);
  return ret;
}

struct get_device_name_ctx
{
  char *partition_name;
  grub_disk_addr_t poff, pend;
};

static int
get_device_name_iter (grub_disk_t disk __attribute__ ((unused)),
		      const grub_partition_t part, void *data)
{
  struct get_device_name_ctx *ctx = data;

  if (grub_partition_get_start (part) == ctx->poff
      && grub_partition_get_len (part) == ctx->pend)
    {
      ctx->partition_name = grub_partition_get_name (part);
      return 1;
    }

  return 0;
}

void
grub_machine_get_bootlocation (char **device, char **path)
{
  char *loaddev = boot_location;
  char *pptr, *partptr;
  char *dname;
  grub_disk_addr_t poff = -1, pend;
  struct get_device_name_ctx ctx;
  grub_disk_t parent = 0;
  unsigned i;

  for (i = 0; i < ARRAY_SIZE (type_names); i++)
    if (type_names[i]
	&& grub_memcmp (loaddev, type_names[i], grub_strlen (type_names[i])) == 0
	&& loaddev[grub_strlen (type_names[i])] == '(')
      break;
  if (i == ARRAY_SIZE (type_names))
    pptr = loaddev;
  else
    for (pptr = loaddev; *pptr && *pptr != '/' && *pptr != '\\'; pptr++);
  if (*pptr)
    {
      char *iptr, *optr;
      char sep = *pptr;
      *path = grub_malloc (grub_strlen (pptr) + 1);
      if (!*path)
	return;
      for (iptr = pptr, optr = *path; *iptr; iptr++, optr++)
	if (*iptr == sep)
	  *optr = '/';
	else
	  *optr = *iptr;
      *optr = '\0';
      *path = grub_strdup (pptr);
      *pptr = '\0';
    }

  if (*loaddev == '\0')
    {
      const char *syspart = 0;

      if (GRUB_ARC_SYSTEM_PARAMETER_BLOCK->firmware_vector_length
	  >= (unsigned) ((char *) (&GRUB_ARC_FIRMWARE_VECTOR->getenvironmentvariable + 1)
			 - (char *) GRUB_ARC_FIRMWARE_VECTOR)
	  && GRUB_ARC_FIRMWARE_VECTOR->getenvironmentvariable)
	syspart = GRUB_ARC_FIRMWARE_VECTOR->getenvironmentvariable ("SystemPartition");
      if (!syspart)
	return;
      loaddev = grub_strdup (syspart);
    }

  partptr = get_part (loaddev);
  if (partptr)
    {
      poff = get_partition_offset (loaddev, &pend);
      *partptr = '\0';
    }
  dname = norm_name_to_alt (loaddev);
  if (poff == (grub_addr_t) -1)
    {
      *device = dname;
      if (loaddev != boot_location)
	grub_free (loaddev);
      return;
    }

  parent = grub_disk_open (dname);
  if (!parent)
    {
      *device = dname;
      if (loaddev != boot_location)
	grub_free (loaddev);
      return;
    }

  if (poff == 0
      && pend == grub_disk_get_size (parent))
    {
      grub_disk_close (parent);
      *device = dname;
      if (loaddev != boot_location)
	grub_free (loaddev);
      return;
    }

  ctx.partition_name = NULL;
  ctx.poff = poff;
  ctx.pend = pend;

  grub_partition_iterate (parent, get_device_name_iter, &ctx);
  grub_disk_close (parent);

  if (! ctx.partition_name)
    {
      *device = dname;
      if (loaddev != boot_location)
	grub_free (loaddev);
      return;
    }

  *device = grub_xasprintf ("%s,%s", dname,
			    ctx.partition_name);
  grub_free (ctx.partition_name);
  grub_free (dname);
  if (loaddev != boot_location)
    grub_free (loaddev);
}
