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

#define ALIGN_CPIO(x) (ALIGN_UP ((x), 4))
#define	MAGIC	"070701"
#define	MAGIC2	"070702"
struct head
{
  char magic[6];
  char ino[8];
  char mode[8];
  char uid[8];
  char gid[8];
  char nlink[8];
  char mtime[8];
  char filesize[8];
  char devmajor[8];
  char devminor[8];
  char rdevmajor[8];
  char rdevminor[8];
  char namesize[8];
  char check[8];
} GRUB_PACKED;

static inline unsigned long long
read_number (const char *str, grub_size_t size)
{
  unsigned long long ret = 0;
  while (size-- && grub_isxdigit (*str))
    {
      char dig = *str++;
      if (dig >= '0' && dig <= '9')
	dig &= 0xf;
      else if (dig >= 'a' && dig <= 'f')
	dig -= 'a' - 10;
      else
	dig -= 'A' - 10;
      ret = (ret << 4) | (dig);
    }
  return ret;
}

#define FSNAME "newc"

#include "cpio_common.c"

GRUB_MOD_INIT (newc)
{
  grub_fs_register (&grub_cpio_fs);
}

GRUB_MOD_FINI (newc)
{
  grub_fs_unregister (&grub_cpio_fs);
}
