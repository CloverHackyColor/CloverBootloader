/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2003,2004,2005,2006,2007,2008,2009,2013  Free Software Foundation, Inc.
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

#include <grub/kernel.h>
#include <grub/mm.h>
#include <grub/machine/time.h>
#include <grub/machine/memory.h>
#include <grub/machine/console.h>
#include <grub/offsets.h>
#include <grub/types.h>
#include <grub/err.h>
#include <grub/dl.h>
#include <grub/misc.h>
#include <grub/loader.h>
#include <grub/env.h>
#include <grub/cache.h>
#include <grub/time.h>
#include <grub/symbol.h>
#include <grub/cpu/io.h>
#include <grub/cpu/floppy.h>
#include <grub/cpu/tsc.h>
#include <grub/machine/kernel.h>
#include <grub/pci.h>

extern grub_uint8_t _start[];
extern grub_uint8_t _end[];
extern grub_uint8_t _edata[];

void  __attribute__ ((noreturn))
grub_exit (void)
{
  /* We can't use grub_fatal() in this function.  This would create an infinite
     loop, since grub_fatal() calls grub_abort() which in turn calls grub_exit().  */
  while (1)
    grub_cpu_idle ();
}

grub_addr_t grub_modbase;

/* Helper for grub_machine_init.  */
static int
heap_init (grub_uint64_t addr, grub_uint64_t size, grub_memory_type_t type,
	   void *data __attribute__ ((unused)))
{
  grub_uint64_t begin = addr, end = addr + size;

#if GRUB_CPU_SIZEOF_VOID_P == 4
  /* Restrict ourselves to 32-bit memory space.  */
  if (begin > GRUB_ULONG_MAX)
    return 0;
  if (end > GRUB_ULONG_MAX)
    end = GRUB_ULONG_MAX;
#endif

  if (type != GRUB_MEMORY_AVAILABLE)
    return 0;

  /* Avoid the lower memory.  */
  if (begin < GRUB_MEMORY_MACHINE_LOWER_SIZE)
    begin = GRUB_MEMORY_MACHINE_LOWER_SIZE;

  if (end <= begin)
    return 0;

  grub_mm_init_region ((void *) (grub_addr_t) begin, (grub_size_t) (end - begin));

  return 0;
}

struct resource
{
  grub_pci_device_t dev;
  grub_size_t size;
  unsigned type:4;
  unsigned bar:3;
};

struct iterator_ctx
{
  struct resource *resources;
  grub_size_t nresources;
};

/* We don't support bridges, so can't have more than 32 devices.  */
#define MAX_DEVICES 32

static int
find_resources (grub_pci_device_t dev,
		grub_pci_id_t pciid __attribute__ ((unused)),
		void *data)
{
  struct iterator_ctx *ctx = data;
  int bar;

  if (ctx->nresources >= MAX_DEVICES * 6)
    return 1;

  for (bar = 0; bar < 6; bar++)
    {
      grub_pci_address_t addr;
      grub_uint32_t ones, zeros, mask;
      struct resource *res;
      addr = grub_pci_make_address (dev, GRUB_PCI_REG_ADDRESS_REG0
				    + 4 * bar);
      grub_pci_write (addr, 0xffffffff);
      grub_pci_read (addr);
      ones = grub_pci_read (addr);
      grub_pci_write (addr, 0);
      grub_pci_read (addr);
      zeros = grub_pci_read (addr);
      if (ones == zeros)
	continue;
      res = &ctx->resources[ctx->nresources++];
      if ((zeros & GRUB_PCI_ADDR_SPACE_MASK) == GRUB_PCI_ADDR_SPACE_IO)
	mask = GRUB_PCI_ADDR_SPACE_MASK;
      else
	mask = (GRUB_PCI_ADDR_MEM_TYPE_MASK | GRUB_PCI_ADDR_SPACE_MASK | GRUB_PCI_ADDR_MEM_PREFETCH);

      res->type = ones & mask;
      res->dev = dev;
      res->bar = bar;
      res->size = (~((zeros ^ ones)) | mask) + 1;
      if ((zeros & (GRUB_PCI_ADDR_MEM_TYPE_MASK | GRUB_PCI_ADDR_SPACE_MASK))
	  == (GRUB_PCI_ADDR_SPACE_MEMORY | GRUB_PCI_ADDR_MEM_TYPE_64))
	bar++;
    }
  return 0;
}

static int
enable_cards (grub_pci_device_t dev,
	      grub_pci_id_t pciid __attribute__ ((unused)),
	      void *data __attribute__ ((unused)))
{
  grub_uint16_t cmd = 0;
  grub_pci_address_t addr;
  grub_uint32_t class;
  int bar;

  for (bar = 0; bar < 6; bar++)
    {
      grub_uint32_t val;
      addr = grub_pci_make_address (dev, GRUB_PCI_REG_ADDRESS_REG0
				    + 4 * bar);
      val = grub_pci_read (addr);
      if (!val)
	continue;
      if ((val & GRUB_PCI_ADDR_SPACE_MASK) == GRUB_PCI_ADDR_SPACE_IO)
	cmd |= GRUB_PCI_COMMAND_IO_ENABLED;
      else
	cmd |= GRUB_PCI_COMMAND_MEM_ENABLED;
    }

  class = (grub_pci_read (addr) >> 16) & 0xffff;

  if (class == GRUB_PCI_CLASS_SUBCLASS_VGA)
    cmd |= GRUB_PCI_COMMAND_IO_ENABLED
      | GRUB_PCI_COMMAND_MEM_ENABLED;

  if (class == GRUB_PCI_CLASS_SUBCLASS_USB)
    return 0;
  
  addr = grub_pci_make_address (dev, GRUB_PCI_REG_COMMAND);
  grub_pci_write (addr, cmd);

  return 0;
}

static void
grub_pci_assign_addresses (void)
{
  struct iterator_ctx ctx;

  {
    struct resource resources[MAX_DEVICES * 6];
    int done;
    unsigned i;
    ctx.nresources = 0;
    ctx.resources = resources;
    grub_uint32_t memptr = 0xf0000000;
    grub_uint16_t ioptr = 0x1000;

    grub_pci_iterate (find_resources, &ctx);
    /* FIXME: do we need a better sort here?  */
    do
      {
	done = 0;
	for (i = 0; i + 1 < ctx.nresources; i++)
	  if (resources[i].size < resources[i+1].size)
	    {
	      struct resource t;
	      t = resources[i];
	      resources[i] = resources[i+1];
	      resources[i+1] = t;
	      done = 1;
	    }
      }
    while (done);

    for (i = 0; i < ctx.nresources; i++)
      {
	grub_pci_address_t addr;
	addr = grub_pci_make_address (resources[i].dev,
				      GRUB_PCI_REG_ADDRESS_REG0
				      + 4 * resources[i].bar);
	if ((resources[i].type & GRUB_PCI_ADDR_SPACE_MASK)
	    == GRUB_PCI_ADDR_SPACE_IO)
	  {
	    grub_pci_write (addr, ioptr | resources[i].type);
	    ioptr += resources[i].size;
	  }
	else
	  {
	    grub_pci_write (addr, memptr | resources[i].type);
	    memptr += resources[i].size;
	    if ((resources[i].type & (GRUB_PCI_ADDR_MEM_TYPE_MASK
				      | GRUB_PCI_ADDR_SPACE_MASK))
		== (GRUB_PCI_ADDR_SPACE_MEMORY | GRUB_PCI_ADDR_MEM_TYPE_64))
	      {
		addr = grub_pci_make_address (resources[i].dev,
					      GRUB_PCI_REG_ADDRESS_REG0
					      + 4 * resources[i].bar + 4);
		grub_pci_write (addr, 0);
	      }
	  }	  
      }
    grub_pci_iterate (enable_cards, NULL);
  }
}

void
grub_machine_init (void)
{
  grub_modbase = grub_core_entry_addr + (_edata - _start);

  grub_pci_assign_addresses ();

  grub_qemu_init_cirrus ();

  grub_vga_text_init ();

  grub_machine_mmap_init ();
  grub_machine_mmap_iterate (heap_init, NULL);


  grub_tsc_init ();
}

void
grub_machine_get_bootlocation (char **device __attribute__ ((unused)),
			       char **path __attribute__ ((unused)))
{
}

void
grub_machine_fini (int flags)
{
  if (flags & GRUB_LOADER_FLAG_NORETURN)
    grub_vga_text_fini ();
  grub_stop_floppy ();
}
