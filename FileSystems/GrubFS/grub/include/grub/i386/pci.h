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

#ifndef	GRUB_CPU_PCI_H
#define	GRUB_CPU_PCI_H	1

#include <grub/types.h>
#include <grub/i386/io.h>

#define GRUB_MACHINE_PCI_IO_BASE          0
#define GRUB_PCI_ADDR_REG	0xcf8
#define GRUB_PCI_DATA_REG	0xcfc
#define GRUB_PCI_NUM_BUS        256
#define GRUB_PCI_NUM_DEVICES    32

static inline grub_uint32_t
grub_pci_read (grub_pci_address_t addr)
{
  grub_outl (addr, GRUB_PCI_ADDR_REG);
  return grub_inl (GRUB_PCI_DATA_REG);
}

static inline grub_uint16_t
grub_pci_read_word (grub_pci_address_t addr)
{
  grub_outl (addr & ~3, GRUB_PCI_ADDR_REG);
  return grub_inw (GRUB_PCI_DATA_REG + (addr & 3));
}

static inline grub_uint8_t
grub_pci_read_byte (grub_pci_address_t addr)
{
  grub_outl (addr & ~3, GRUB_PCI_ADDR_REG);
  return grub_inb (GRUB_PCI_DATA_REG + (addr & 3));
}

static inline void
grub_pci_write (grub_pci_address_t addr, grub_uint32_t data)
{
  grub_outl (addr, GRUB_PCI_ADDR_REG);
  grub_outl (data, GRUB_PCI_DATA_REG);
}

static inline void
grub_pci_write_word (grub_pci_address_t addr, grub_uint16_t data)
{
  grub_outl (addr & ~3, GRUB_PCI_ADDR_REG);
  grub_outw (data, GRUB_PCI_DATA_REG + (addr & 3));
}

static inline void
grub_pci_write_byte (grub_pci_address_t addr, grub_uint8_t data)
{
  grub_outl (addr & ~3, GRUB_PCI_ADDR_REG);
  grub_outb (data, GRUB_PCI_DATA_REG + (addr & 3));
}

#ifndef GRUB_MACHINE_IEEE1275

static inline volatile void *
grub_pci_device_map_range (grub_pci_device_t dev __attribute__ ((unused)),
			   grub_addr_t base,
			   grub_size_t size __attribute__ ((unused)))
{
  return (volatile void *) base;
}

static inline void
grub_pci_device_unmap_range (grub_pci_device_t dev __attribute__ ((unused)),
			     volatile void *mem __attribute__ ((unused)),
			     grub_size_t size __attribute__ ((unused)))
{
}

#else

volatile void *
grub_pci_device_map_range (grub_pci_device_t dev,
			   grub_addr_t base,
			   grub_size_t size);

void
grub_pci_device_unmap_range (grub_pci_device_t dev,
			     volatile void *mem,
			     grub_size_t size);

#endif


#endif /* GRUB_CPU_PCI_H */
