/* Memory management for efiemu */
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
/*
  To keep efiemu runtime contiguous this mm is special.
  It uses deferred allocation.
  In the first stage you may request memory with grub_efiemu_request_memalign
  It will give you a handle with which in the second phase you can access your
  memory with grub_efiemu_mm_obtain_request (handle). It's guaranteed that
  subsequent calls with the same handle return the same result. You can't request any additional memory once you're in the second phase
*/

#include <grub/err.h>
#include <grub/normal.h>
#include <grub/mm.h>
#include <grub/misc.h>
#include <grub/efiemu/efiemu.h>
#include <grub/memory.h>

struct grub_efiemu_memrequest
{
  struct grub_efiemu_memrequest *next;
  grub_efi_memory_type_t type;
  grub_size_t size;
  grub_size_t align_overhead;
  int handle;
  void *val;
};
/* Linked list of requested memory. */
static struct grub_efiemu_memrequest *memrequests = 0;
/* Memory map. */
static grub_efi_memory_descriptor_t *efiemu_mmap = 0;
/* Pointer to allocated memory */
static void *resident_memory = 0;
/* Size of requested memory per type */
static grub_size_t requested_memory[GRUB_EFI_MAX_MEMORY_TYPE];
/* How many slots is allocated for memory_map and how many are already used */
static int mmap_reserved_size = 0, mmap_num = 0;

/* Add a memory region to map*/
static grub_err_t
grub_efiemu_add_to_mmap (grub_uint64_t start, grub_uint64_t size,
			 grub_efi_memory_type_t type)
{
  grub_uint64_t page_start, npages;

  /* Extend map if necessary*/
  if (mmap_num >= mmap_reserved_size)
    {
      void *old;
      mmap_reserved_size = 2 * (mmap_reserved_size + 1);
      old = efiemu_mmap;
      efiemu_mmap = (grub_efi_memory_descriptor_t *)
	grub_realloc (efiemu_mmap, mmap_reserved_size
		      * sizeof (grub_efi_memory_descriptor_t));
      if (!efiemu_mmap)
	{
	  grub_free (old);
	  return grub_errno;
	}
    }

  /* Fill slot*/
  page_start = start - (start % GRUB_EFIEMU_PAGESIZE);
  npages = (size + (start % GRUB_EFIEMU_PAGESIZE) + GRUB_EFIEMU_PAGESIZE - 1)
    / GRUB_EFIEMU_PAGESIZE;
  efiemu_mmap[mmap_num].physical_start = page_start;
  efiemu_mmap[mmap_num].virtual_start = page_start;
  efiemu_mmap[mmap_num].num_pages = npages;
  efiemu_mmap[mmap_num].type = type;
  mmap_num++;

  return GRUB_ERR_NONE;
}

/* Request a resident memory of type TYPE of size SIZE aligned at ALIGN
   ALIGN must be a divisor of page size (if it's a divisor of 4096
   it should be ok on all platforms)
 */
int
grub_efiemu_request_memalign (grub_size_t align, grub_size_t size,
			      grub_efi_memory_type_t type)
{
  grub_size_t align_overhead;
  struct grub_efiemu_memrequest *ret, *cur, *prev;
  /* Check that the request is correct */
  if (type >= GRUB_EFI_MAX_MEMORY_TYPE || type <= GRUB_EFI_LOADER_CODE)
    return -2;

  /* Add new size to requested size */
  align_overhead = align - (requested_memory[type]%align);
  if (align_overhead == align)
    align_overhead = 0;
  requested_memory[type] += align_overhead + size;

  /* Remember the request */
  ret = grub_zalloc (sizeof (*ret));
  if (!ret)
    return -1;
  ret->type = type;
  ret->size = size;
  ret->align_overhead = align_overhead;
  prev = 0;

  /* Add request to the end of the chain.
     It should be at the end because otherwise alignment isn't guaranteed */
  for (cur = memrequests; cur; prev = cur, cur = cur->next);
  if (prev)
    {
      ret->handle = prev->handle + 1;
      prev->next = ret;
    }
  else
    {
      ret->handle = 1; /* Avoid 0 handle*/
      memrequests = ret;
    }
  return ret->handle;
}

/* Really allocate the memory */
static grub_err_t
efiemu_alloc_requests (void)
{
  grub_size_t align_overhead = 0;
  grub_uint8_t *curptr, *typestart;
  struct grub_efiemu_memrequest *cur;
  grub_size_t total_alloc = 0;
  unsigned i;
  /* Order of memory regions */
  grub_efi_memory_type_t reqorder[] =
    {
      /* First come regions usable by OS*/
      GRUB_EFI_LOADER_CODE,
      GRUB_EFI_LOADER_DATA,
      GRUB_EFI_BOOT_SERVICES_CODE,
      GRUB_EFI_BOOT_SERVICES_DATA,
      GRUB_EFI_CONVENTIONAL_MEMORY,
      GRUB_EFI_ACPI_RECLAIM_MEMORY,

      /* Then memory used by runtime */
      /* This way all our regions are in a single block */
      GRUB_EFI_RUNTIME_SERVICES_CODE,
      GRUB_EFI_RUNTIME_SERVICES_DATA,
      GRUB_EFI_ACPI_MEMORY_NVS,

      /* And then unavailable memory types. This is more for a completeness.
	 You should double think before allocating memory of any of these types
       */
      GRUB_EFI_UNUSABLE_MEMORY,
      GRUB_EFI_MEMORY_MAPPED_IO,
      GRUB_EFI_MEMORY_MAPPED_IO_PORT_SPACE,
      GRUB_EFI_PAL_CODE
    };

  /* Compute total memory needed */
  for (i = 0; i < sizeof (reqorder) / sizeof (reqorder[0]); i++)
    {
      align_overhead = GRUB_EFIEMU_PAGESIZE
	- (requested_memory[reqorder[i]] % GRUB_EFIEMU_PAGESIZE);
      if (align_overhead == GRUB_EFIEMU_PAGESIZE)
	align_overhead = 0;
      total_alloc += requested_memory[reqorder[i]] + align_overhead;
    }

  /* Allocate the whole memory in one block */
  resident_memory = grub_memalign (GRUB_EFIEMU_PAGESIZE, total_alloc);
  if (!resident_memory)
    return grub_errno;

  /* Split the memory into blocks by type */
  curptr = resident_memory;
  for (i = 0; i < sizeof (reqorder) / sizeof (reqorder[0]); i++)
    {
      if (!requested_memory[reqorder[i]])
	continue;
      typestart = curptr;

      /* Write pointers to requests */
      for (cur = memrequests; cur; cur = cur->next)
	if (cur->type == reqorder[i])
	  {
	    curptr = ((grub_uint8_t *)curptr) + cur->align_overhead;
	    cur->val = curptr;
	    curptr = ((grub_uint8_t *)curptr) + cur->size;
	  }

      /* Ensure that the regions are page-aligned */
      align_overhead = GRUB_EFIEMU_PAGESIZE
	- (requested_memory[reqorder[i]] % GRUB_EFIEMU_PAGESIZE);
      if (align_overhead == GRUB_EFIEMU_PAGESIZE)
	align_overhead = 0;
      curptr = ((grub_uint8_t *) curptr) + align_overhead;

      /* Add the region to memory map */
      grub_efiemu_add_to_mmap ((grub_addr_t) typestart,
			       curptr - typestart, reqorder[i]);
    }

  return GRUB_ERR_NONE;
}

/* Get a pointer to requested memory from handle */
void *
grub_efiemu_mm_obtain_request (int handle)
{
  struct grub_efiemu_memrequest *cur;
  for (cur = memrequests; cur; cur = cur->next)
    if (cur->handle == handle)
      return cur->val;
  return 0;
}

/* Get type of requested memory by handle */
grub_efi_memory_type_t
grub_efiemu_mm_get_type (int handle)
{
  struct grub_efiemu_memrequest *cur;
  for (cur = memrequests; cur; cur = cur->next)
    if (cur->handle == handle)
      return cur->type;
  return 0;
}

/* Free a request */
void
grub_efiemu_mm_return_request (int handle)
{
  struct grub_efiemu_memrequest *cur, *prev;

  /* Remove head if necessary */
  while (memrequests && memrequests->handle == handle)
    {
      cur = memrequests->next;
      grub_free (memrequests);
      memrequests = cur;
    }
  if (!memrequests)
    return;

  /* Remove request from a middle of chain*/
  for (prev = memrequests, cur = prev->next; cur;)
    if (cur->handle == handle)
      {
	prev->next = cur->next;
	grub_free (cur);
	cur = prev->next;
      }
    else
      {
	prev = cur;
	cur = prev->next;
      }
}

/* Helper for grub_efiemu_mmap_init.  */
static int
bounds_hook (grub_uint64_t addr __attribute__ ((unused)),
	     grub_uint64_t size __attribute__ ((unused)),
	     grub_memory_type_t type __attribute__ ((unused)),
	     void *data __attribute__ ((unused)))
{
  mmap_reserved_size++;
  return 0;
}

/* Reserve space for memory map */
static grub_err_t
grub_efiemu_mmap_init (void)
{
  // the place for memory used by efiemu itself
  mmap_reserved_size = GRUB_EFI_MAX_MEMORY_TYPE + 1;

#ifndef GRUB_MACHINE_EMU
  grub_machine_mmap_iterate (bounds_hook, NULL);
#endif

  return GRUB_ERR_NONE;
}

/* This is a drop-in replacement of grub_efi_get_memory_map */
/* Get the memory map as defined in the EFI spec. Return 1 if successful,
   return 0 if partial, or return -1 if an error occurs.  */
int
grub_efiemu_get_memory_map (grub_efi_uintn_t *memory_map_size,
			    grub_efi_memory_descriptor_t *memory_map,
			    grub_efi_uintn_t *map_key,
			    grub_efi_uintn_t *descriptor_size,
			    grub_efi_uint32_t *descriptor_version)
{
  if (!efiemu_mmap)
    {
      grub_error (GRUB_ERR_INVALID_COMMAND,
		  "you need to first launch efiemu_prepare");
      return -1;
    }

  if (*memory_map_size < mmap_num * sizeof (grub_efi_memory_descriptor_t))
    {
      *memory_map_size = mmap_num * sizeof (grub_efi_memory_descriptor_t);
      return 0;
    }

  *memory_map_size = mmap_num * sizeof (grub_efi_memory_descriptor_t);
  grub_memcpy (memory_map, efiemu_mmap, *memory_map_size);
  if (descriptor_size)
    *descriptor_size = sizeof (grub_efi_memory_descriptor_t);
  if (descriptor_version)
    *descriptor_version = 1;
  if (map_key)
    *map_key = 0;

  return 1;
}

grub_err_t
grub_efiemu_finish_boot_services (grub_efi_uintn_t *memory_map_size,
				  grub_efi_memory_descriptor_t *memory_map,
				  grub_efi_uintn_t *map_key,
				  grub_efi_uintn_t *descriptor_size,
				  grub_efi_uint32_t *descriptor_version)
{
  int val = grub_efiemu_get_memory_map (memory_map_size,
					memory_map, map_key,
					descriptor_size,
					descriptor_version);
  if (val == 1)
    return GRUB_ERR_NONE;
  if (val == -1)
    return grub_errno;
  return grub_error (GRUB_ERR_IO, "memory map buffer is too small");
}


/* Free everything */
grub_err_t
grub_efiemu_mm_unload (void)
{
  struct grub_efiemu_memrequest *cur, *d;
  for (cur = memrequests; cur;)
    {
      d = cur->next;
      grub_free (cur);
      cur = d;
    }
  memrequests = 0;
  grub_memset (&requested_memory, 0, sizeof (requested_memory));
  grub_free (resident_memory);
  resident_memory = 0;
  grub_free (efiemu_mmap);
  efiemu_mmap = 0;
  mmap_reserved_size = mmap_num = 0;
  return GRUB_ERR_NONE;
}

/* This function should be called before doing any requests */
grub_err_t
grub_efiemu_mm_init (void)
{
  grub_err_t err;

  err = grub_efiemu_mm_unload ();
  if (err)
    return err;

  grub_efiemu_mmap_init ();

  return GRUB_ERR_NONE;
}

/* Helper for grub_efiemu_mmap_fill.  */
static int
fill_hook (grub_uint64_t addr, grub_uint64_t size, grub_memory_type_t type,
	   void *data __attribute__ ((unused)))
  {
    switch (type)
      {
      case GRUB_MEMORY_AVAILABLE:
	return grub_efiemu_add_to_mmap (addr, size,
					GRUB_EFI_CONVENTIONAL_MEMORY);

      case GRUB_MEMORY_ACPI:
	return grub_efiemu_add_to_mmap (addr, size,
					GRUB_EFI_ACPI_RECLAIM_MEMORY);

      case GRUB_MEMORY_NVS:
	return grub_efiemu_add_to_mmap (addr, size,
					GRUB_EFI_ACPI_MEMORY_NVS);

      default:
	grub_dprintf ("efiemu",
		      "Unknown memory type %d. Assuming unusable\n", type);
      case GRUB_MEMORY_RESERVED:
	return grub_efiemu_add_to_mmap (addr, size,
					GRUB_EFI_UNUSABLE_MEMORY);
      }
  }

/* Copy host memory map */
static grub_err_t
grub_efiemu_mmap_fill (void)
{
#ifndef GRUB_MACHINE_EMU
  grub_machine_mmap_iterate (fill_hook, NULL);
#endif

  return GRUB_ERR_NONE;
}

grub_err_t
grub_efiemu_mmap_iterate (grub_memory_hook_t hook, void *hook_data)
{
  unsigned i;

  for (i = 0; i < (unsigned) mmap_num; i++)
    switch (efiemu_mmap[i].type)
      {
      case GRUB_EFI_RUNTIME_SERVICES_CODE:
	hook (efiemu_mmap[i].physical_start, efiemu_mmap[i].num_pages * 4096,
	      GRUB_MEMORY_CODE, hook_data);
	break;

      case GRUB_EFI_UNUSABLE_MEMORY:
	hook (efiemu_mmap[i].physical_start, efiemu_mmap[i].num_pages * 4096,
	      GRUB_MEMORY_BADRAM, hook_data);
	break;

      case GRUB_EFI_RESERVED_MEMORY_TYPE:
      case GRUB_EFI_RUNTIME_SERVICES_DATA:
      case GRUB_EFI_MEMORY_MAPPED_IO:
      case GRUB_EFI_MEMORY_MAPPED_IO_PORT_SPACE:
      case GRUB_EFI_PAL_CODE:
      case GRUB_EFI_MAX_MEMORY_TYPE:
	hook (efiemu_mmap[i].physical_start, efiemu_mmap[i].num_pages * 4096,
	      GRUB_MEMORY_RESERVED, hook_data);
	break;

      case GRUB_EFI_LOADER_CODE:
      case GRUB_EFI_LOADER_DATA:
      case GRUB_EFI_BOOT_SERVICES_CODE:
      case GRUB_EFI_BOOT_SERVICES_DATA:
      case GRUB_EFI_CONVENTIONAL_MEMORY:
	hook (efiemu_mmap[i].physical_start, efiemu_mmap[i].num_pages * 4096,
	      GRUB_MEMORY_AVAILABLE, hook_data);
	break;

      case GRUB_EFI_ACPI_RECLAIM_MEMORY:
	hook (efiemu_mmap[i].physical_start, efiemu_mmap[i].num_pages * 4096,
	      GRUB_MEMORY_ACPI, hook_data);
	break;

      case GRUB_EFI_ACPI_MEMORY_NVS:
	hook (efiemu_mmap[i].physical_start, efiemu_mmap[i].num_pages * 4096,
	      GRUB_MEMORY_NVS, hook_data);
	break;
      }

  return 0;
}


/* This function resolves overlapping regions and sorts the memory map
   It uses scanline (sweeping) algorithm
 */
static grub_err_t
grub_efiemu_mmap_sort_and_uniq (void)
{
  /* If same page is used by multiple types it's resolved
     according to priority
     0 - free memory
     1 - memory immediately usable after ExitBootServices
     2 - memory usable after loading ACPI tables
     3 - efiemu memory
     4 - unusable memory
  */
  int priority[GRUB_EFI_MAX_MEMORY_TYPE] =
    {
      [GRUB_EFI_RESERVED_MEMORY_TYPE] = 4,
      [GRUB_EFI_LOADER_CODE] = 1,
      [GRUB_EFI_LOADER_DATA] = 1,
      [GRUB_EFI_BOOT_SERVICES_CODE] = 1,
      [GRUB_EFI_BOOT_SERVICES_DATA] = 1,
      [GRUB_EFI_RUNTIME_SERVICES_CODE] = 3,
      [GRUB_EFI_RUNTIME_SERVICES_DATA] = 3,
      [GRUB_EFI_CONVENTIONAL_MEMORY] = 0,
      [GRUB_EFI_UNUSABLE_MEMORY] = 4,
      [GRUB_EFI_ACPI_RECLAIM_MEMORY] = 2,
      [GRUB_EFI_ACPI_MEMORY_NVS] = 3,
      [GRUB_EFI_MEMORY_MAPPED_IO] = 4,
      [GRUB_EFI_MEMORY_MAPPED_IO_PORT_SPACE] = 4,
      [GRUB_EFI_PAL_CODE] = 4
    };

  int i, j, k, done;

  /* Scanline events */
  struct grub_efiemu_mmap_scan
  {
    /* At which memory address*/
    grub_uint64_t pos;
    /* 0 = region starts, 1 = region ends */
    int type;
    /* Which type of memory region */
    grub_efi_memory_type_t memtype;
  };
  struct grub_efiemu_mmap_scan *scanline_events;
  struct grub_efiemu_mmap_scan t;

  /* Previous scanline event */
  grub_uint64_t lastaddr;
  int lasttype;
  /* Current scanline event */
  int curtype;
  /* how many regions of given type overlap at current location */
  int present[GRUB_EFI_MAX_MEMORY_TYPE];
  /* Here is stored the resulting memory map*/
  grub_efi_memory_descriptor_t *result;

  /* Initialize variables*/
  grub_memset (present, 0, sizeof (int) * GRUB_EFI_MAX_MEMORY_TYPE);
  scanline_events = (struct grub_efiemu_mmap_scan *)
    grub_malloc (sizeof (struct grub_efiemu_mmap_scan) * 2 * mmap_num);

  /* Number of chunks can't increase more than by factor of 2 */
  result = (grub_efi_memory_descriptor_t *)
    grub_malloc (sizeof (grub_efi_memory_descriptor_t) * 2 * mmap_num);
  if (!result || !scanline_events)
    {
      grub_free (result);
      grub_free (scanline_events);
      return grub_errno;
    }

  /* Register scanline events */
  for (i = 0; i < mmap_num; i++)
    {
      scanline_events[2 * i].pos = efiemu_mmap[i].physical_start;
      scanline_events[2 * i].type = 0;
      scanline_events[2 * i].memtype = efiemu_mmap[i].type;
      scanline_events[2 * i + 1].pos = efiemu_mmap[i].physical_start
	+ efiemu_mmap[i].num_pages * GRUB_EFIEMU_PAGESIZE;
      scanline_events[2 * i + 1].type = 1;
      scanline_events[2 * i + 1].memtype = efiemu_mmap[i].type;
    }

  /* Primitive bubble sort. It has complexity O(n^2) but since we're
     unlikely to have more than 100 chunks it's probably one of the
     fastest for one purpose */
  done = 1;
  while (done)
    {
      done = 0;
      for (i = 0; i < 2 * mmap_num - 1; i++)
	if (scanline_events[i + 1].pos < scanline_events[i].pos)
	  {
	    t = scanline_events[i + 1];
	    scanline_events[i + 1] = scanline_events[i];
	    scanline_events[i] = t;
	    done = 1;
	  }
    }

  /* Pointer in resulting memory map */
  j = 0;
  lastaddr = scanline_events[0].pos;
  lasttype = scanline_events[0].memtype;
  for (i = 0; i < 2 * mmap_num; i++)
    {
      /* Process event */
      if (scanline_events[i].type)
	present[scanline_events[i].memtype]--;
      else
	present[scanline_events[i].memtype]++;

      /* Determine current region type */
      curtype = -1;
      for (k = 0; k < GRUB_EFI_MAX_MEMORY_TYPE; k++)
	if (present[k] && (curtype == -1 || priority[k] > priority[curtype]))
	  curtype = k;

      /* Add memory region to resulting map if necessary */
      if ((curtype == -1 || curtype != lasttype)
	  && lastaddr != scanline_events[i].pos
	  && lasttype != -1)
	{
	  result[j].virtual_start = result[j].physical_start = lastaddr;
	  result[j].num_pages = (scanline_events[i].pos - lastaddr)
	    / GRUB_EFIEMU_PAGESIZE;
	  result[j].type = lasttype;

	  /* We set runtime attribute on pages we need to be mapped */
	  result[j].attribute
	    = (lasttype == GRUB_EFI_RUNTIME_SERVICES_CODE
		   || lasttype == GRUB_EFI_RUNTIME_SERVICES_DATA)
	    ? GRUB_EFI_MEMORY_RUNTIME : 0;
	  grub_dprintf ("efiemu",
			"mmap entry: type %d start 0x%llx 0x%llx pages\n",
			result[j].type,
			result[j].physical_start, result[j].num_pages);
	  j++;
	}

      /* Update last values if necessary */
      if (curtype == -1 || curtype != lasttype)
	{
	  lasttype = curtype;
	  lastaddr = scanline_events[i].pos;
	}
    }

  grub_free (scanline_events);

  /* Shrink resulting memory map to really used size and replace efiemu_mmap
     by new value */
  grub_free (efiemu_mmap);
  efiemu_mmap = grub_realloc (result, j * sizeof (*result));
  return GRUB_ERR_NONE;
}

/* This function is called to switch from first to second phase */
grub_err_t
grub_efiemu_mm_do_alloc (void)
{
  grub_err_t err;

  /* Preallocate mmap */
  efiemu_mmap = (grub_efi_memory_descriptor_t *)
    grub_malloc (mmap_reserved_size * sizeof (grub_efi_memory_descriptor_t));
  if (!efiemu_mmap)
    {
      grub_efiemu_unload ();
      return grub_errno;
    }

  err = efiemu_alloc_requests ();
  if (err)
    return err;
  err = grub_efiemu_mmap_fill ();
  if (err)
    return err;
  return grub_efiemu_mmap_sort_and_uniq ();
}
