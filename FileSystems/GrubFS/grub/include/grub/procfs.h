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

#ifndef GRUB_PROCFS_HEADER
#define GRUB_PROCFS_HEADER	1

#include <grub/list.h>
#include <grub/types.h>

struct grub_procfs_entry
{
  struct grub_procfs_entry *next;
  struct grub_procfs_entry **prev;

  const char *name;
  char * (*get_contents) (grub_size_t *sz);
};

extern struct grub_procfs_entry *grub_procfs_entries;

static inline void
grub_procfs_register (const char *name __attribute__ ((unused)),
		      struct grub_procfs_entry *entry)
{
  grub_list_push (GRUB_AS_LIST_P (&grub_procfs_entries),
		  GRUB_AS_LIST (entry));
}

static inline void
grub_procfs_unregister (struct grub_procfs_entry *entry)
{
  grub_list_remove (GRUB_AS_LIST (entry));
}


#endif
