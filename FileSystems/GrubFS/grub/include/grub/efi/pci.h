/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2006,2007,2008  Free Software Foundation, Inc.
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

#ifndef GRUB_EFI_PCI_HEADER
#define GRUB_EFI_PCI_HEADER	1

#include <grub/symbol.h>

#define GRUB_EFI_PCI_IO_GUID \
  { 0x4cf5b200, 0x68b8, 0x4ca5, { 0x9e, 0xec, 0xb2, 0x3e, 0x3f, 0x50, 0x02, 0x9a }}

#define GRUB_EFI_PCI_ROOT_IO_GUID \
  { 0x2F707EBB, 0x4A1A, 0x11d4, { 0x9A, 0x38, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D }}

typedef enum
  {
    GRUB_EFI_PCI_IO_WIDTH_UINT8,
    GRUB_EFI_PCI_IO_WIDTH_UINT16,
    GRUB_EFI_PCI_IO_WIDTH_UINT32,
    GRUB_EFI_PCI_IO_WIDTH_UINT64,
    GRUB_EFI_PCI_IO_WIDTH_FIFO_UINT8,
    GRUB_EFI_PCI_IO_WIDTH_FIFO_UINT16,
    GRUB_EFI_PCI_IO_WIDTH_FIFO_UINT32,
    GRUB_EFI_PCI_IO_WIDTH_FIFO_UINT64,
    GRUB_EFI_PCI_IO_WIDTH_FILL_UINT8,
    GRUB_EFI_PCI_IO_WIDTH_FILL_UINT16,
    GRUB_EFI_PCI_IO_WIDTH_FILL_UINT32,
    GRUB_EFI_PCI_IO_WIDTH_FILL_UINT64,
    GRUB_EFI_PCI_IO_WIDTH_MAXIMUM,
  }
  grub_efi_pci_io_width_t;

struct grub_efi_pci_io;

typedef grub_efi_status_t
(*grub_efi_pci_io_mem_t) (struct grub_efi_pci_io *this,
			  grub_efi_pci_io_width_t width,
			  grub_efi_uint8_t bar_index,
			  grub_efi_uint64_t offset,
			  grub_efi_uintn_t count,
			  void *buffer);

typedef grub_efi_status_t
(*grub_efi_pci_io_config_t) (struct grub_efi_pci_io *this,
			     grub_efi_pci_io_width_t width,
			     grub_efi_uint32_t offset,
			     grub_efi_uintn_t count,
			     void *buffer);
typedef struct
{
  grub_efi_pci_io_mem_t read;
  grub_efi_pci_io_mem_t write;
} grub_efi_pci_io_access_t;

typedef struct
{
  grub_efi_pci_io_config_t read;
  grub_efi_pci_io_config_t write;
} grub_efi_pci_io_config_access_t;

typedef enum
  {
    GRUB_EFI_PCI_IO_OPERATION_BUS_MASTER_READ,
    GRUB_EFI_PCI_IO_OPERATION_BUS_MASTER_WRITE,
    GRUB_EFI_PCI_IO_OPERATION_BUS_MASTER_COMMON_BUFFER,
    GRUB_EFI_PCI_IO_OPERATION_BUS_MASTER_MAXIMUM
  }
  grub_efi_pci_io_operation_t;

#define GRUB_EFI_PCI_IO_ATTRIBUTE_ISA_IO               0x0002
#define GRUB_EFI_PCI_IO_ATTRIBUTE_VGA_PALETTE_IO       0x0004
#define GRUB_EFI_PCI_IO_ATTRIBUTE_VGA_MEMORY           0x0008
#define GRUB_EFI_PCI_IO_ATTRIBUTE_VGA_IO               0x0010
#define GRUB_EFI_PCI_IO_ATTRIBUTE_IDE_PRIMARY_IO       0x0020
#define GRUB_EFI_PCI_IO_ATTRIBUTE_IDE_SECONDARY_IO     0x0040
#define GRUB_EFI_PCI_IO_ATTRIBUTE_MEMORY_WRITE_COMBINE 0x0080
#define GRUB_EFI_PCI_IO_ATTRIBUTE_IO                   0x0100
#define GRUB_EFI_PCI_IO_ATTRIBUTE_MEMORY               0x0200
#define GRUB_EFI_PCI_IO_ATTRIBUTE_BUS_MASTER           0x0400
#define GRUB_EFI_PCI_IO_ATTRIBUTE_MEMORY_CACHED        0x0800
#define GRUB_EFI_PCI_IO_ATTRIBUTE_MEMORY_DISABLE       0x1000
#define GRUB_EFI_PCI_IO_ATTRIBUTE_EMBEDDED_DEVICE      0x2000
#define GRUB_EFI_PCI_IO_ATTRIBUTE_EMBEDDED_ROM         0x4000
#define GRUB_EFI_PCI_IO_ATTRIBUTE_DUAL_ADDRESS_CYCLE   0x8000
#define GRUB_EFI_PCI_IO_ATTRIBUTE_ISA_IO_16            0x10000
#define GRUB_EFI_PCI_IO_ATTRIBUTE_VGA_PALETTE_IO_16    0x20000
#define GRUB_EFI_PCI_IO_ATTRIBUTE_VGA_IO_16            0x40000

typedef enum
  {
    GRUB_EFI_PCI_IO_ATTRIBUTE_OPERATION_GET,
    GRUB_EFI_PCI_IO_ATTRIBUTE_OPERATION_SET,
    GRUB_EFI_PCI_IO_ATTRIBUTE_OPERATION_ENABLE,
    GRUB_EFI_PCI_IO_ATTRIBUTE_OPERATION_DISABLE,
    GRUB_EFI_PCI_IO_ATTRIBUTE_OPERATION_SUPPORTED,
    GRUB_EFI_PCI_IO_ATTRIBUTE_OPERATION_MAXIMUM
  }
  grub_efi_pci_io_attribute_operation_t;

typedef grub_efi_status_t
(*grub_efi_pci_io_poll_io_mem_t) (struct grub_efi_pci_io *this,
				  grub_efi_pci_io_width_t  width,
				  grub_efi_uint8_t bar_ndex,
				  grub_efi_uint64_t offset,
				  grub_efi_uint64_t mask,
				  grub_efi_uint64_t value,
				  grub_efi_uint64_t delay,
				  grub_efi_uint64_t *result);

typedef grub_efi_status_t
(*grub_efi_pci_io_copy_mem_t) (struct grub_efi_pci_io *this,
			       grub_efi_pci_io_width_t width,
			       grub_efi_uint8_t dest_bar_index,
			       grub_efi_uint64_t dest_offset,
			       grub_efi_uint8_t src_bar_index,
			       grub_efi_uint64_t src_offset,
			       grub_efi_uintn_t count);

typedef grub_efi_status_t
(*grub_efi_pci_io_map_t) (struct grub_efi_pci_io *this,
			  grub_efi_pci_io_operation_t operation,
			  void *host_address,
			  grub_efi_uintn_t *number_of_bytes,
			  grub_efi_uint64_t *device_address,
			  void **mapping);

typedef grub_efi_status_t
(*grub_efi_pci_io_unmap_t) (struct grub_efi_pci_io *this,
			    void *mapping);

typedef grub_efi_status_t
(*grub_efi_pci_io_allocate_buffer_t) (struct grub_efi_pci_io *this,
				      grub_efi_allocate_type_t type,
				      grub_efi_memory_type_t memory_type,
				      grub_efi_uintn_t pages,
				      void **host_address,
				      grub_efi_uint64_t attributes);

typedef grub_efi_status_t
(*grub_efi_pci_io_free_buffer_t) (struct grub_efi_pci_io *this,
				  grub_efi_allocate_type_t type,
				  grub_efi_memory_type_t memory_type,
				  grub_efi_uintn_t pages,
				  void **host_address,
				  grub_efi_uint64_t attributes);

typedef grub_efi_status_t
(*grub_efi_pci_io_flush_t) (struct grub_efi_pci_io *this);

typedef grub_efi_status_t
(*grub_efi_pci_io_get_location_t) (struct grub_efi_pci_io *this,
				   grub_efi_uintn_t *segment_number,
				   grub_efi_uintn_t *bus_number,
				   grub_efi_uintn_t *device_number,
				   grub_efi_uintn_t *function_number);

typedef grub_efi_status_t
(*grub_efi_pci_io_attributes_t) (struct grub_efi_pci_io *this,
				 grub_efi_pci_io_attribute_operation_t operation,
				 grub_efi_uint64_t attributes,
				 grub_efi_uint64_t *result);

typedef grub_efi_status_t
(*grub_efi_pci_io_get_bar_attributes_t) (struct grub_efi_pci_io *this,
					 grub_efi_uint8_t bar_index,
					 grub_efi_uint64_t *supports,
					 void **resources);

typedef grub_efi_status_t
(*grub_efi_pci_io_set_bar_attributes_t) (struct grub_efi_pci_io *this,
					 grub_efi_uint64_t attributes,
					 grub_efi_uint8_t bar_index,
					 grub_efi_uint64_t *offset,
					 grub_efi_uint64_t *length);
struct grub_efi_pci_io {
  grub_efi_pci_io_poll_io_mem_t poll_mem;
  grub_efi_pci_io_poll_io_mem_t poll_io;
  grub_efi_pci_io_access_t mem;
  grub_efi_pci_io_access_t io;
  grub_efi_pci_io_config_access_t pci;
  grub_efi_pci_io_copy_mem_t copy_mem;
  grub_efi_pci_io_map_t map;
  grub_efi_pci_io_unmap_t unmap;
  grub_efi_pci_io_allocate_buffer_t allocate_buffer;
  grub_efi_pci_io_free_buffer_t free_buffer;
  grub_efi_pci_io_flush_t flush;
  grub_efi_pci_io_get_location_t get_location;
  grub_efi_pci_io_attributes_t attributes;
  grub_efi_pci_io_get_bar_attributes_t get_bar_attributes;
  grub_efi_pci_io_set_bar_attributes_t set_bar_attributes;
  grub_efi_uint64_t rom_size;
  void *rom_image;
};
typedef struct grub_efi_pci_io grub_efi_pci_io_t;

struct grub_efi_pci_root_io;

typedef struct
{
  grub_efi_status_t(*read) (struct grub_efi_pci_root_io *this,
			    grub_efi_pci_io_width_t width,
			    grub_efi_uint64_t address,
			    grub_efi_uintn_t count,
			    void *buffer);
  grub_efi_status_t(*write) (struct grub_efi_pci_root_io *this,
			     grub_efi_pci_io_width_t width,
			     grub_efi_uint64_t address,
			     grub_efi_uintn_t count,
			     void *buffer);
} grub_efi_pci_root_io_access_t;

typedef enum
  {
    GRUB_EFI_PCI_ROOT_IO_OPERATION_BUS_MASTER_READ,
    GRUB_EFI_PCI_ROOT_IO_OPERATION_BUS_MASTER_WRITE,
    GRUB_EFI_PCI_ROOT_IO_OPERATION_BUS_MASTER_COMMON_BUFFER,
    GRUB_EFI_PCI_ROOT_IO_OPERATION_BUS_MASTER_READ_64,
    GRUB_EFI_PCI_ROOT_IO_OPERATION_BUS_MASTER_WRITE_64,
    GRUB_EFI_PCI_ROOT_IO_OPERATION_BUS_MASTER_COMMON_BUFFER_64,
    GRUB_EFI_PCI_ROOT_IO_OPERATION_BUS_MASTER_MAXIMUM
  }
  grub_efi_pci_root_io_operation_t;

typedef grub_efi_status_t
(*grub_efi_pci_root_io_poll_io_mem_t) (struct grub_efi_pci_root_io *this,
				       grub_efi_pci_io_width_t  width,
				       grub_efi_uint64_t address,
				       grub_efi_uint64_t mask,
				       grub_efi_uint64_t value,
				       grub_efi_uint64_t delay,
				       grub_efi_uint64_t *result);

typedef grub_efi_status_t
(*grub_efi_pci_root_io_copy_mem_t) (struct grub_efi_pci_root_io *this,
				    grub_efi_pci_io_width_t width,
				    grub_efi_uint64_t dest_offset,
				    grub_efi_uint64_t src_offset,
				    grub_efi_uintn_t count);


typedef grub_efi_status_t
(*grub_efi_pci_root_io_map_t) (struct grub_efi_pci_root_io *this,
				grub_efi_pci_root_io_operation_t operation,
			       void *host_address,
			       grub_efi_uintn_t *number_of_bytes,
			       grub_efi_uint64_t *device_address,
			       void **mapping);

typedef grub_efi_status_t
(*grub_efi_pci_root_io_unmap_t) (struct grub_efi_pci_root_io *this,
				 void *mapping);

typedef grub_efi_status_t
(*grub_efi_pci_root_io_allocate_buffer_t) (struct grub_efi_pci_root_io *this,
					   grub_efi_allocate_type_t type,
					   grub_efi_memory_type_t memory_type,
					   grub_efi_uintn_t pages,
					   void **host_address,
					   grub_efi_uint64_t attributes);

typedef grub_efi_status_t
(*grub_efi_pci_root_io_free_buffer_t) (struct grub_efi_pci_root_io *this,
				       grub_efi_uintn_t pages,
				       void **host_address);

typedef grub_efi_status_t
(*grub_efi_pci_root_io_flush_t) (struct grub_efi_pci_root_io *this);

typedef grub_efi_status_t
(*grub_efi_pci_root_io_get_attributes_t) (struct grub_efi_pci_root_io *this,
					  grub_efi_uint64_t *supports,
					  void **resources);

typedef grub_efi_status_t
(*grub_efi_pci_root_io_set_attributes_t) (struct grub_efi_pci_root_io *this,
					  grub_efi_uint64_t attributes,
					  grub_efi_uint64_t *offset,
					  grub_efi_uint64_t *length);

typedef grub_efi_status_t
(*grub_efi_pci_root_io_configuration_t) (struct grub_efi_pci_root_io *this,
					 void **resources);

struct grub_efi_pci_root_io {
  grub_efi_handle_t parent;
  grub_efi_pci_root_io_poll_io_mem_t poll_mem;
  grub_efi_pci_root_io_poll_io_mem_t poll_io;
  grub_efi_pci_root_io_access_t mem;
  grub_efi_pci_root_io_access_t io;
  grub_efi_pci_root_io_access_t pci;
  grub_efi_pci_root_io_copy_mem_t copy_mem;
  grub_efi_pci_root_io_map_t map;
  grub_efi_pci_root_io_unmap_t unmap;
  grub_efi_pci_root_io_allocate_buffer_t allocate_buffer;
  grub_efi_pci_root_io_free_buffer_t free_buffer;
  grub_efi_pci_root_io_flush_t flush;
  grub_efi_pci_root_io_get_attributes_t get_attributes;
  grub_efi_pci_root_io_set_attributes_t set_attributes;
  grub_efi_pci_root_io_configuration_t configuration;
};

typedef struct grub_efi_pci_root_io grub_efi_pci_root_io_t;

#endif  /* !GRUB_EFI_PCI_HEADER */
