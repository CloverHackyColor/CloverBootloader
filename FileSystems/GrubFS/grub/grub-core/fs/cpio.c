/* cpio.c - cpio and tar filesystem.  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2007,2008,2009,2013 Free Software Foundation, Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <grub/misc.h>

/* cpio support */
#define ALIGN_CPIO(x) (ALIGN_UP ((x), 2))
#define	MAGIC       "\xc7\x71"
struct head
{
  grub_uint16_t magic[1];
  grub_uint16_t dev;
  grub_uint16_t ino;
  grub_uint16_t mode[1];
  grub_uint16_t uid;
  grub_uint16_t gid;
  grub_uint16_t nlink;
  grub_uint16_t rdev;
  grub_uint16_t mtime[2];
  grub_uint16_t namesize[1];
  grub_uint16_t filesize[2];
} GRUB_PACKED;

static inline unsigned long long
read_number (const grub_uint16_t *arr, grub_size_t size)
{
  long long ret = 0;
  while (size--)
    ret = (ret << 16) | grub_le_to_cpu16 (*arr++);
  return ret;
}

#define FSNAME "cpiofs"

#include "cpio_common.c"

GRUB_MOD_INIT (cpio)
{
  grub_fs_register (&grub_cpio_fs);
}

GRUB_MOD_FINI (cpio)
{
  grub_fs_unregister (&grub_cpio_fs);
}
