/* ieee1275.h - Access the Open Firmware client interface.  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2003,2004,2005,2007,2009  Free Software Foundation, Inc.
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

#ifndef GRUB_IEEE1275_MACHINE_HEADER
#define GRUB_IEEE1275_MACHINE_HEADER	1

#include <grub/types.h>

#define GRUB_IEEE1275_CELL_SIZEOF 8
typedef grub_uint64_t grub_ieee1275_cell_t;

/* Encoding of 'mode' argument to grub_ieee1275_map_physical() */
#define IEEE1275_MAP_WRITE	0x0001 /* Writable */
#define IEEE1275_MAP_READ	0x0002 /* Readable */
#define IEEE1275_MAP_EXEC	0x0004 /* Executable */
#define IEEE1275_MAP_LOCKED	0x0010 /* Locked in TLB */
#define IEEE1275_MAP_CACHED	0x0020 /* Cacheable */
#define IEEE1275_MAP_SE		0x0040 /* Side-effects */
#define IEEE1275_MAP_GLOBAL	0x0080 /* Global */
#define IEEE1275_MAP_IE		0x0100 /* Invert Endianness */
#define IEEE1275_MAP_DEFAULT	(IEEE1275_MAP_WRITE | IEEE1275_MAP_READ | \
				 IEEE1275_MAP_EXEC | IEEE1275_MAP_CACHED)

extern int EXPORT_FUNC(grub_ieee1275_claim_vaddr) (grub_addr_t vaddr,
						   grub_size_t size);
extern int EXPORT_FUNC(grub_ieee1275_alloc_physmem) (grub_addr_t *paddr,
						     grub_size_t size,
						     grub_uint32_t align);

extern grub_addr_t EXPORT_VAR (grub_ieee1275_original_stack);

#endif /* ! GRUB_IEEE1275_MACHINE_HEADER */
