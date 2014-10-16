/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2003,2005,2006,2007,2008,2009,2010  Free Software Foundation, Inc.
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

#include <grub/types.h>
#include <grub/err.h>
#include <grub/mm.h>
#include <stdlib.h>
#include <string.h>
#include <grub/i18n.h>

void *
grub_malloc (grub_size_t size)
{
  void *ret;
  ret = malloc (size);
  if (!ret)
    grub_error (GRUB_ERR_OUT_OF_MEMORY, N_("out of memory"));
  return ret;
}

void *
grub_zalloc (grub_size_t size)
{
  void *ret;

  ret = grub_malloc (size);
  if (!ret)
    return NULL;
  memset (ret, 0, size);
  return ret;
}

void
grub_free (void *ptr)
{
  free (ptr);
}

void *
grub_realloc (void *ptr, grub_size_t size)
{
  void *ret;
  ret = realloc (ptr, size);
  if (!ret)
    grub_error (GRUB_ERR_OUT_OF_MEMORY, N_("out of memory"));
  return ret;
}
