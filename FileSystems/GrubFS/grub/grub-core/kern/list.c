/* list.c - grub list function */
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

#include <grub/list.h>
#include <grub/misc.h>
#include <grub/mm.h>

void *
grub_named_list_find (grub_named_list_t head, const char *name)
{
  grub_named_list_t item;

  FOR_LIST_ELEMENTS (item, head)
    if (grub_strcmp (item->name, name) == 0)
      return item;

  return NULL;
}

void
grub_list_push (grub_list_t *head, grub_list_t item)
{
  item->prev = head;
  if (*head)
    (*head)->prev = &item->next;
  item->next = *head;
  *head = item;
}

void
grub_list_remove (grub_list_t item)
{
  if (item->prev)
    *item->prev = item->next;
  if (item->next)
    item->next->prev = item->prev;
  item->next = 0;
  item->prev = 0;
}
