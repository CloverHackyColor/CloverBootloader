/* diskfilter.h - On disk structures for RAID. */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2006,2007,2008,2010  Free Software Foundation, Inc.
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

#ifndef GRUB_DISKFILTER_H
#define GRUB_DISKFILTER_H	1

#include <grub/types.h>
#include <grub/list.h>

enum
  {
    GRUB_RAID_LAYOUT_RIGHT_MASK	= 1,
    GRUB_RAID_LAYOUT_SYMMETRIC_MASK = 2,
    GRUB_RAID_LAYOUT_MUL_FROM_POS = 4,

    GRUB_RAID_LAYOUT_LEFT_ASYMMETRIC = 0,
    GRUB_RAID_LAYOUT_RIGHT_ASYMMETRIC = GRUB_RAID_LAYOUT_RIGHT_MASK,
    GRUB_RAID_LAYOUT_LEFT_SYMMETRIC = GRUB_RAID_LAYOUT_SYMMETRIC_MASK,
    GRUB_RAID_LAYOUT_RIGHT_SYMMETRIC = (GRUB_RAID_LAYOUT_RIGHT_MASK
					| GRUB_RAID_LAYOUT_SYMMETRIC_MASK)
  };


struct grub_diskfilter_vg {
  char *uuid;
  grub_size_t uuid_len;
  /* Optional.  */
  char *name;
  grub_uint64_t extent_size;
  struct grub_diskfilter_pv *pvs;
  struct grub_diskfilter_lv *lvs;
  struct grub_diskfilter_vg *next;

#ifdef GRUB_UTIL
  struct grub_diskfilter *driver;
#endif
};

struct grub_diskfilter_pv_id {
  union
  {
    char *uuid;
    int id;
  };
  grub_size_t uuidlen;
};

struct grub_diskfilter_pv {
  struct grub_diskfilter_pv_id id;
  /* Optional.  */
  char *name;
  grub_disk_t disk;
  grub_disk_addr_t part_start;
  grub_disk_addr_t part_size;
  grub_disk_addr_t start_sector; /* Sector number where the data area starts. */
  struct grub_diskfilter_pv *next;
  /* Optional.  */
  grub_uint8_t *internal_id;
#ifdef GRUB_UTIL
  char **partmaps;
#endif
};

struct grub_diskfilter_lv {
  /* Name used for disk.  */
  char *fullname;
  char *idname;
  /* Optional.  */
  char *name;
  int number;
  unsigned int segment_count;
  grub_size_t segment_alloc;
  grub_uint64_t size;
  int became_readable_at;
  int scanned;
  int visible;

  /* Pointer to segment_count segments. */
  struct grub_diskfilter_segment *segments;
  struct grub_diskfilter_vg *vg;
  struct grub_diskfilter_lv *next;

  /* Optional.  */
  char *internal_id;
};

struct grub_diskfilter_segment {
  grub_uint64_t start_extent;
  grub_uint64_t extent_count;
  enum 
    {
      GRUB_DISKFILTER_STRIPED = 0,
      GRUB_DISKFILTER_MIRROR = 1,
      GRUB_DISKFILTER_RAID4 = 4,
      GRUB_DISKFILTER_RAID5 = 5,      
      GRUB_DISKFILTER_RAID6 = 6,
      GRUB_DISKFILTER_RAID10 = 10,
  } type;
  int layout;
  /* valid only for raid10.  */
  grub_uint64_t raid_member_size;

  unsigned int node_count;
  unsigned int node_alloc;
  struct grub_diskfilter_node *nodes;

  unsigned int stripe_size;
};

struct grub_diskfilter_node {
  grub_disk_addr_t start;
  /* Optional.  */
  char *name;
  struct grub_diskfilter_pv *pv;
  struct grub_diskfilter_lv *lv;
};

struct grub_diskfilter_vg *
grub_diskfilter_get_vg_by_uuid (grub_size_t uuidlen, char *uuid);

struct grub_diskfilter
{
  struct grub_diskfilter *next;
  struct grub_diskfilter **prev;

  const char *name;

  struct grub_diskfilter_vg * (*detect) (grub_disk_t disk,
					 struct grub_diskfilter_pv_id *id,
					 grub_disk_addr_t *start_sector);
};
typedef struct grub_diskfilter *grub_diskfilter_t;

extern grub_diskfilter_t grub_diskfilter_list;
static inline void
grub_diskfilter_register_front (grub_diskfilter_t diskfilter)
{
  grub_list_push (GRUB_AS_LIST_P (&grub_diskfilter_list),
		  GRUB_AS_LIST (diskfilter));
}

static inline void
grub_diskfilter_register_back (grub_diskfilter_t diskfilter)
{
  grub_diskfilter_t p, *q;
  for (q = &grub_diskfilter_list, p = *q; p; q = &p->next, p = *q);
  diskfilter->next = NULL;
  diskfilter->prev = q;
  *q = diskfilter;
}
static inline void
grub_diskfilter_unregister (grub_diskfilter_t diskfilter)
{
  grub_list_remove (GRUB_AS_LIST (diskfilter));
}

struct grub_diskfilter_vg *
grub_diskfilter_make_raid (grub_size_t uuidlen, char *uuid, int nmemb,
			   char *name, grub_uint64_t disk_size,
			   grub_uint64_t stripe_size,
			   int layout, int level);

typedef grub_err_t (*grub_raid5_recover_func_t) (struct grub_diskfilter_segment *array,
                                                 int disknr, char *buf,
                                                 grub_disk_addr_t sector,
                                                 grub_size_t size);

typedef grub_err_t (*grub_raid6_recover_func_t) (struct grub_diskfilter_segment *array,
                                                 int disknr, int p, char *buf,
                                                 grub_disk_addr_t sector,
                                                 grub_size_t size);

extern grub_raid5_recover_func_t grub_raid5_recover_func;
extern grub_raid6_recover_func_t grub_raid6_recover_func;

grub_err_t grub_diskfilter_vg_register (struct grub_diskfilter_vg *vg);

grub_err_t
grub_diskfilter_read_node (const struct grub_diskfilter_node *node,
			   grub_disk_addr_t sector,
			   grub_size_t size, char *buf);

#ifdef GRUB_UTIL
struct grub_diskfilter_pv *
grub_diskfilter_get_pv_from_disk (grub_disk_t disk,
				  struct grub_diskfilter_vg **vg);
void
grub_diskfilter_get_partmap (grub_disk_t disk,
			     void (*cb) (const char *val, void *data),
			     void *data);
#endif

#endif /* ! GRUB_RAID_H */
