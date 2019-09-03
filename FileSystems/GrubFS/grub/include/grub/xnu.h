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

#ifndef GRUB_XNU_H
#define GRUB_XNU_H 1

#include <grub/bitmap.h>

/* Header of a hibernation image. */
struct grub_xnu_hibernate_header
{
  /* Size of the image. Notice that file containing image is usually bigger. */
  grub_uint64_t image_size;
  grub_uint8_t unknown1[8];
  /* Where to copy launchcode? */
  grub_uint32_t launchcode_target_page;
  /* How many pages of launchcode? */
  grub_uint32_t launchcode_numpages;
  /* Where to jump? */
  grub_uint32_t entry_point;
  /* %esp at start. */
  grub_uint32_t stack;
  grub_uint8_t unknown2[44];
#define GRUB_XNU_HIBERNATE_MAGIC 0x73696d65
  grub_uint32_t magic;
  grub_uint8_t unknown3[28];
  /* This value is non-zero if page is encrypted. Unsupported. */
  grub_uint64_t encoffset;
  grub_uint8_t unknown4[360];
  /* The size of additional header used to locate image without parsing FS.
     Used only to skip it.
   */
  grub_uint32_t extmapsize;
} GRUB_PACKED;

/* In-memory structure for temporary keeping device tree. */
struct grub_xnu_devtree_key
{
  char *name;
  int datasize; /* -1 for not leaves. */
  union
  {
    struct grub_xnu_devtree_key *first_child;
    void *data;
  };
  struct grub_xnu_devtree_key *next;
};

/* A structure used in memory-map values. */
struct
grub_xnu_extdesc
{
  grub_uint32_t addr;
  grub_uint32_t size;
} GRUB_PACKED;

/* Header describing extension in the memory. */
struct grub_xnu_extheader
{
  grub_uint32_t infoplistaddr;
  grub_uint32_t infoplistsize;
  grub_uint32_t binaryaddr;
  grub_uint32_t binarysize;
  grub_uint32_t nameaddr;
  grub_uint32_t namesize;
} GRUB_PACKED;

struct grub_xnu_devtree_key *grub_xnu_create_key (struct grub_xnu_devtree_key **parent,
						  const char *name);

extern struct grub_xnu_devtree_key *grub_xnu_devtree_root;

void grub_xnu_free_devtree (struct grub_xnu_devtree_key *cur);

grub_err_t grub_xnu_writetree_toheap (grub_addr_t *target, grub_size_t *size);
struct grub_xnu_devtree_key *grub_xnu_create_value (struct grub_xnu_devtree_key **parent,
						    const char *name);

void grub_xnu_lock (void);
void grub_xnu_unlock (void);
grub_err_t grub_xnu_resume (char *imagename);
grub_err_t grub_xnu_boot_resume (void);
struct grub_xnu_devtree_key *grub_xnu_find_key (struct grub_xnu_devtree_key *parent,
						const char *name);
grub_err_t grub_xnu_align_heap (int align);
grub_err_t grub_xnu_scan_dir_for_kexts (char *dirname,
					const char *osbundlerequired,
					int maxrecursion);
grub_err_t grub_xnu_load_kext_from_dir (char *dirname,
					const char *osbundlerequired,
					int maxrecursion);
grub_err_t grub_xnu_heap_malloc (int size, void **src, grub_addr_t *target);
grub_err_t grub_xnu_fill_devicetree (void);
extern struct grub_relocator *grub_xnu_relocator;

extern grub_size_t grub_xnu_heap_size;
extern struct grub_video_bitmap *grub_xnu_bitmap;
typedef enum {GRUB_XNU_BITMAP_CENTER, GRUB_XNU_BITMAP_STRETCH}
  grub_xnu_bitmap_mode_t;
extern grub_xnu_bitmap_mode_t grub_xnu_bitmap_mode;
extern int grub_xnu_is_64bit;
extern grub_addr_t grub_xnu_heap_target_start;
extern int grub_xnu_darwin_version;
#endif
