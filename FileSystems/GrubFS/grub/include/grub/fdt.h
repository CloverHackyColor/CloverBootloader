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

#ifndef GRUB_FDT_HEADER
#define GRUB_FDT_HEADER	1

#include <grub/types.h>

#define FDT_MAGIC 0xD00DFEED

typedef struct {
	grub_uint32_t magic;
	grub_uint32_t totalsize;
	grub_uint32_t off_dt_struct;
	grub_uint32_t off_dt_strings;
	grub_uint32_t off_mem_rsvmap;
	grub_uint32_t version;
	grub_uint32_t last_comp_version;
	grub_uint32_t boot_cpuid_phys;
	grub_uint32_t size_dt_strings;
	grub_uint32_t size_dt_struct;
} grub_fdt_header_t;

struct grub_fdt_empty_tree {
  grub_fdt_header_t header;
  grub_uint64_t empty_rsvmap[2];
  struct {
    grub_uint32_t node_start;
    grub_uint8_t name[1];
    grub_uint32_t node_end;
    grub_uint32_t tree_end;
  } empty_node;
};

#define GRUB_FDT_EMPTY_TREE_SZ  sizeof (struct grub_fdt_empty_tree)

#define grub_fdt_get_header(fdt, field)	\
	grub_be_to_cpu32(((const grub_fdt_header_t *)(fdt))->field)
#define grub_fdt_set_header(fdt, field, value)	\
	((grub_fdt_header_t *)(fdt))->field = grub_cpu_to_be32(value)

#define grub_fdt_get_magic(fdt)	\
	grub_fdt_get_header(fdt, magic)
#define grub_fdt_set_magic(fdt, value)	\
	grub_fdt_set_header(fdt, magic, value)
#define grub_fdt_get_totalsize(fdt)	\
	grub_fdt_get_header(fdt, totalsize)
#define grub_fdt_set_totalsize(fdt, value)	\
	grub_fdt_set_header(fdt, totalsize, value)
#define grub_fdt_get_off_dt_struct(fdt)	\
	grub_fdt_get_header(fdt, off_dt_struct)
#define grub_fdt_set_off_dt_struct(fdt, value)	\
	grub_fdt_set_header(fdt, off_dt_struct, value)
#define grub_fdt_get_off_dt_strings(fdt)	\
	grub_fdt_get_header(fdt, off_dt_strings)
#define grub_fdt_set_off_dt_strings(fdt, value)	\
	grub_fdt_set_header(fdt, off_dt_strings, value)
#define grub_fdt_get_off_mem_rsvmap(fdt)	\
	grub_fdt_get_header(fdt, off_mem_rsvmap)
#define grub_fdt_set_off_mem_rsvmap(fdt, value)	\
	grub_fdt_set_header(fdt, off_mem_rsvmap, value)
#define grub_fdt_get_version(fdt)	\
	grub_fdt_get_header(fdt, version)
#define grub_fdt_set_version(fdt, value)	\
	grub_fdt_set_header(fdt, version, value)
#define grub_fdt_get_last_comp_version(fdt)	\
	grub_fdt_get_header(fdt, last_comp_version)
#define grub_fdt_set_last_comp_version(fdt, value)	\
	grub_fdt_set_header(fdt, last_comp_version, value)
#define grub_fdt_get_boot_cpuid_phys(fdt)	\
	grub_fdt_get_header(fdt, boot_cpuid_phys)
#define grub_fdt_set_boot_cpuid_phys(fdt, value)	\
	grub_fdt_set_header(fdt, boot_cpuid_phys, value)
#define grub_fdt_get_size_dt_strings(fdt)	\
	grub_fdt_get_header(fdt, size_dt_strings)
#define grub_fdt_set_size_dt_strings(fdt, value)	\
	grub_fdt_set_header(fdt, size_dt_strings, value)
#define grub_fdt_get_size_dt_struct(fdt)	\
	grub_fdt_get_header(fdt, size_dt_struct)
#define grub_fdt_set_size_dt_struct(fdt, value)	\
	grub_fdt_set_header(fdt, size_dt_struct, value)

int grub_fdt_create_empty_tree (void *fdt, unsigned int size);
int grub_fdt_check_header (void *fdt, unsigned int size);
int grub_fdt_check_header_nosize (void *fdt);
int grub_fdt_find_subnode (const void *fdt, unsigned int parentoffset,
			   const char *name);
int grub_fdt_add_subnode (void *fdt, unsigned int parentoffset,
			  const char *name);

int grub_fdt_set_prop (void *fdt, unsigned int nodeoffset, const char *name,
		      const void *val, grub_uint32_t len);
#define grub_fdt_set_prop32(fdt, nodeoffset, name, val)	\
({ \
  grub_uint32_t _val = grub_cpu_to_be32(val); \
  grub_fdt_set_prop ((fdt), (nodeoffset), (name), &_val, 4);	\
})

#define grub_fdt_set_prop64(fdt, nodeoffset, name, val)        \
({ \
  grub_uint64_t _val = grub_cpu_to_be64(val); \
  grub_fdt_set_prop ((fdt), (nodeoffset), (name), &_val, 8);   \
})

#endif	/* ! GRUB_FDT_HEADER */
