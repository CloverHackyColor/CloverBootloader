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

#ifndef GRUB_EFI_EMU_HEADER
#define GRUB_EFI_EMU_HEADER	1

#include <grub/efi/api.h>
#include <grub/file.h>
#include <grub/memory.h>

#define GRUB_EFIEMU_PAGESIZE 4096

/* EFI api defined in 32-bit and 64-bit version*/
struct grub_efi_system_table32
{
  grub_efi_table_header_t hdr;
  grub_efi_uint32_t firmware_vendor;
  grub_efi_uint32_t firmware_revision;
  grub_efi_uint32_t console_in_handler;
  grub_efi_uint32_t con_in;
  grub_efi_uint32_t console_out_handler;
  grub_efi_uint32_t con_out;
  grub_efi_uint32_t standard_error_handle;
  grub_efi_uint32_t std_err;
  grub_efi_uint32_t runtime_services;
  grub_efi_uint32_t boot_services;
  grub_efi_uint32_t num_table_entries;
  grub_efi_uint32_t configuration_table;
} GRUB_PACKED;
typedef struct grub_efi_system_table32  grub_efi_system_table32_t;

struct grub_efi_system_table64
{
  grub_efi_table_header_t hdr;
  grub_efi_uint64_t firmware_vendor;
  grub_efi_uint32_t firmware_revision;
  grub_efi_uint32_t pad;
  grub_efi_uint64_t console_in_handler;
  grub_efi_uint64_t con_in;
  grub_efi_uint64_t console_out_handler;
  grub_efi_uint64_t con_out;
  grub_efi_uint64_t standard_error_handle;
  grub_efi_uint64_t std_err;
  grub_efi_uint64_t runtime_services;
  grub_efi_uint64_t boot_services;
  grub_efi_uint64_t num_table_entries;
  grub_efi_uint64_t configuration_table;
} GRUB_PACKED;
typedef struct grub_efi_system_table64  grub_efi_system_table64_t;

struct grub_efiemu_runtime_services32
{
  grub_efi_table_header_t hdr;
  grub_efi_uint32_t get_time;
  grub_efi_uint32_t set_time;
  grub_efi_uint32_t get_wakeup_time;
  grub_efi_uint32_t set_wakeup_time;
  grub_efi_uint32_t set_virtual_address_map;
  grub_efi_uint32_t convert_pointer;
  grub_efi_uint32_t get_variable;
  grub_efi_uint32_t get_next_variable_name;
  grub_efi_uint32_t set_variable;
  grub_efi_uint32_t get_next_high_monotonic_count;
  grub_efi_uint32_t reset_system;
} GRUB_PACKED;
typedef struct grub_efiemu_runtime_services32 grub_efiemu_runtime_services32_t;

struct grub_efiemu_runtime_services64
{
  grub_efi_table_header_t hdr;
  grub_efi_uint64_t get_time;
  grub_efi_uint64_t set_time;
  grub_efi_uint64_t get_wakeup_time;
  grub_efi_uint64_t set_wakeup_time;
  grub_efi_uint64_t set_virtual_address_map;
  grub_efi_uint64_t convert_pointer;
  grub_efi_uint64_t get_variable;
  grub_efi_uint64_t get_next_variable_name;
  grub_efi_uint64_t set_variable;
  grub_efi_uint64_t get_next_high_monotonic_count;
  grub_efi_uint64_t reset_system;
} GRUB_PACKED;
typedef struct grub_efiemu_runtime_services64 grub_efiemu_runtime_services64_t;

extern grub_efi_system_table32_t *grub_efiemu_system_table32;
extern grub_efi_system_table64_t *grub_efiemu_system_table64;

/* Convenience macros to access currently loaded efiemu */
#define grub_efiemu_system_table ((grub_efiemu_sizeof_uintn_t () == 8) \
				  ? (void *) grub_efiemu_system_table64 \
				  : (void *) grub_efiemu_system_table32)
#define GRUB_EFIEMU_SIZEOF_OF_UINTN (grub_efiemu_sizeof_uintn_t ())
#define GRUB_EFIEMU_SYSTEM_TABLE(x) ((grub_efiemu_sizeof_uintn_t () == 8) \
				     ? grub_efiemu_system_table64->x \
				     : grub_efiemu_system_table32->x)
#define GRUB_EFIEMU_SYSTEM_TABLE_SET(x,y) ((grub_efiemu_sizeof_uintn_t () == 8)\
					   ? (grub_efiemu_system_table64->x \
					      = (y)) \
					   : (grub_efiemu_system_table32->x \
					      = (y)))
#define GRUB_EFIEMU_SYSTEM_TABLE_PTR(x) ((grub_efiemu_sizeof_uintn_t () == 8)\
					 ? (void *) (grub_addr_t)	\
					 (grub_efiemu_system_table64->x) \
					 : (void *) (grub_addr_t) \
					 (grub_efiemu_system_table32->x))
#define GRUB_EFIEMU_SYSTEM_TABLE_VAR(x) ((grub_efiemu_sizeof_uintn_t () == 8) \
					 ? (void *) \
					 &(grub_efiemu_system_table64->x) \
					 : (void *) \
					 &(grub_efiemu_system_table32->x))
#define GRUB_EFIEMU_SYSTEM_TABLE_SIZEOF(x) \
  ((grub_efiemu_sizeof_uintn_t () == 8) \
   ? sizeof(grub_efiemu_system_table64->x)\
   : sizeof(grub_efiemu_system_table32->x))
#define GRUB_EFIEMU_SYSTEM_TABLE_SIZEOF_TOTAL ((grub_efiemu_sizeof_uintn_t () == 8) ? sizeof(*grub_efiemu_system_table64):sizeof(*grub_efiemu_system_table32))

/* ELF management definitions and functions */

struct grub_efiemu_segment
{
  struct grub_efiemu_segment *next;
  grub_size_t size;
  unsigned section;
  int handle;
  int ptv_rel_needed;
  grub_off_t off;
  void *srcptr;
};
typedef struct grub_efiemu_segment *grub_efiemu_segment_t;

struct grub_efiemu_elf_sym
{
  int handle;
  grub_off_t off;
  unsigned section;
};

int grub_efiemu_check_header32 (void *ehdr, grub_size_t size);
int grub_efiemu_check_header64 (void *ehdr, grub_size_t size);
grub_err_t grub_efiemu_loadcore_init32 (void *core,
					const char *filename,
					grub_size_t core_size,
					grub_efiemu_segment_t *segments);
grub_err_t grub_efiemu_loadcore_init64 (void *core, const char *filename,
					grub_size_t core_size,
					grub_efiemu_segment_t *segments);
grub_err_t grub_efiemu_loadcore_load32 (void *core,
					grub_size_t core_size,
					grub_efiemu_segment_t segments);
grub_err_t grub_efiemu_loadcore_load64 (void *core,
					grub_size_t core_size,
					grub_efiemu_segment_t segments);
grub_err_t grub_efiemu_loadcore_unload32 (void);
grub_err_t grub_efiemu_loadcore_unload64 (void);
grub_err_t grub_efiemu_loadcore_unload(void);
grub_err_t grub_efiemu_loadcore_init (grub_file_t file,
				      const char *filename);
grub_err_t grub_efiemu_loadcore_load (void);

/* Configuration tables manipulation. Definitions and functions */
struct grub_efiemu_configuration_table
{
  struct grub_efiemu_configuration_table *next;
  grub_efi_guid_t guid;
  void * (*get_table) (void *data);
  void (*unload) (void *data);
  void *data;
};
struct grub_efiemu_configuration_table32
{
  grub_efi_packed_guid_t vendor_guid;
  grub_efi_uint32_t vendor_table;
} GRUB_PACKED;
typedef struct grub_efiemu_configuration_table32 grub_efiemu_configuration_table32_t;
struct grub_efiemu_configuration_table64
{
  grub_efi_packed_guid_t vendor_guid;
  grub_efi_uint64_t vendor_table;
} GRUB_PACKED;
typedef struct grub_efiemu_configuration_table64 grub_efiemu_configuration_table64_t;
grub_err_t grub_efiemu_unregister_configuration_table (grub_efi_guid_t guid);
grub_err_t
grub_efiemu_register_configuration_table (grub_efi_guid_t guid,
					  void * (*get_table) (void *data),
					  void (*unload) (void *data),
					  void *data);

/* Memory management functions */
int grub_efiemu_request_memalign (grub_size_t align, grub_size_t size,
				  grub_efi_memory_type_t type);
void *grub_efiemu_mm_obtain_request (int handle);
grub_err_t grub_efiemu_mm_unload (void);
grub_err_t grub_efiemu_mm_do_alloc (void);
grub_err_t grub_efiemu_mm_init (void);
void grub_efiemu_mm_return_request (int handle);
grub_efi_memory_type_t grub_efiemu_mm_get_type (int handle);

/* Drop-in replacements for grub_efi_* and grub_machine_* */
int grub_efiemu_get_memory_map (grub_efi_uintn_t *memory_map_size,
				grub_efi_memory_descriptor_t *memory_map,
				grub_efi_uintn_t *map_key,
				grub_efi_uintn_t *descriptor_size,
				grub_efi_uint32_t *descriptor_version);


grub_err_t
grub_efiemu_finish_boot_services (grub_efi_uintn_t *memory_map_size,
				  grub_efi_memory_descriptor_t *memory_map,
				  grub_efi_uintn_t *map_key,
				  grub_efi_uintn_t *descriptor_size,
				  grub_efi_uint32_t *descriptor_version);

grub_err_t
grub_efiemu_mmap_iterate (grub_memory_hook_t hook, void *hook_data);
int grub_efiemu_sizeof_uintn_t (void);
grub_err_t
grub_efiemu_get_lower_upper_memory (grub_uint64_t *lower, grub_uint64_t *upper);

/* efiemu main control definitions and functions*/
typedef enum {GRUB_EFIEMU_NOTLOADED,
	      GRUB_EFIEMU32, GRUB_EFIEMU64} grub_efiemu_mode_t;
struct grub_efiemu_prepare_hook
{
  struct grub_efiemu_prepare_hook *next;
  grub_err_t (*hook) (void *data);
  void (*unload) (void *data);
  void *data;
};
grub_err_t grub_efiemu_prepare32 (struct grub_efiemu_prepare_hook
				  *prepare_hooks,
				  struct grub_efiemu_configuration_table
				  *config_tables);
grub_err_t grub_efiemu_prepare64 (struct grub_efiemu_prepare_hook
				  *prepare_hooks,
				  struct grub_efiemu_configuration_table
				  *config_tables);
grub_err_t grub_efiemu_unload (void);
grub_err_t grub_efiemu_prepare (void);
grub_err_t
grub_efiemu_register_prepare_hook (grub_err_t (*hook) (void *data),
				   void (*unload) (void *data),
				   void *data);

/* symbols and pointers */
grub_err_t grub_efiemu_alloc_syms (void);
grub_err_t grub_efiemu_request_symbols (int num);
grub_err_t grub_efiemu_resolve_symbol (const char *name,
				       int *handle, grub_off_t *off);
grub_err_t grub_efiemu_register_symbol (const char *name,
					int handle, grub_off_t off);
void grub_efiemu_free_syms (void);
grub_err_t grub_efiemu_write_value (void * addr, grub_uint32_t value,
				    int plus_handle,
				    int minus_handle, int ptv_needed, int size);
grub_err_t grub_efiemu_write_sym_markers (void);
grub_err_t grub_efiemu_pnvram (void);
const char *grub_efiemu_get_default_core_name (void);
void grub_efiemu_pnvram_cmd_unregister (void);
grub_err_t grub_efiemu_autocore (void);
grub_err_t grub_efiemu_crc32 (void);
grub_err_t grub_efiemu_crc64 (void);
grub_err_t
grub_efiemu_set_virtual_address_map (grub_efi_uintn_t memory_map_size,
				     grub_efi_uintn_t descriptor_size,
				     grub_efi_uint32_t descriptor_version
				     __attribute__ ((unused)),
				     grub_efi_memory_descriptor_t *virtual_map);

grub_err_t grub_machine_efiemu_init_tables (void);

#endif /* ! GRUB_EFI_EMU_HEADER */
