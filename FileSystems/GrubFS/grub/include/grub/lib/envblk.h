/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2008,2009  Free Software Foundation, Inc.
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

#ifndef GRUB_ENVBLK_HEADER
#define GRUB_ENVBLK_HEADER	1

#define GRUB_ENVBLK_SIGNATURE	"# GRUB Environment Block\n"
#define GRUB_ENVBLK_DEFCFG	"grubenv"

#ifndef ASM_FILE

struct grub_envblk
{
  char *buf;
  grub_size_t size;
};
typedef struct grub_envblk *grub_envblk_t;

grub_envblk_t grub_envblk_open (char *buf, grub_size_t size);
int grub_envblk_set (grub_envblk_t envblk, const char *name, const char *value);
void grub_envblk_delete (grub_envblk_t envblk, const char *name);
void grub_envblk_iterate (grub_envblk_t envblk,
                          void *hook_data,
                          int hook (const char *name, const char *value, void *hook_data));
void grub_envblk_close (grub_envblk_t envblk);

static inline char *
grub_envblk_buffer (const grub_envblk_t envblk)
{
  return envblk->buf;
}

static inline grub_size_t
grub_envblk_size (const grub_envblk_t envblk)
{
  return envblk->size;
}

#endif /* ! ASM_FILE */

#endif /* ! GRUB_ENVBLK_HEADER */
