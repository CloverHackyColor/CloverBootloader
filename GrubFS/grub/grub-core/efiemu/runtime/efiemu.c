/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2006,2007,2009  Free Software Foundation, Inc.
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

/* This is an emulation of EFI runtime services.
   This allows a more uniform boot on i386 machines.
   As it emulates only runtime serviceit isn't able
   to chainload EFI bootloader on non-EFI system (TODO) */

#ifdef __i386__
#include <grub/i386/types.h>
#else
#include <grub/x86_64/types.h>
#endif

#include <grub/symbol.h>
#include <grub/types.h>
#include <grub/efi/api.h>
#include <grub/efiemu/runtime.h>

grub_efi_status_t
efiemu_get_time (grub_efi_time_t *time,
		 grub_efi_time_capabilities_t *capabilities);
grub_efi_status_t
efiemu_set_time (grub_efi_time_t *time);

grub_efi_status_t
efiemu_get_wakeup_time (grub_efi_boolean_t *enabled,
			grub_efi_boolean_t *pending,
			grub_efi_time_t *time);
grub_efi_status_t
efiemu_set_wakeup_time (grub_efi_boolean_t enabled,
			grub_efi_time_t *time);

#ifdef __APPLE__
#define PHYSICAL_ATTRIBUTE __attribute__ ((section("_text-physical, _text-physical")));
#else
#define PHYSICAL_ATTRIBUTE __attribute__ ((section(".text-physical")));
#endif

grub_efi_status_t
efiemu_set_virtual_address_map (grub_efi_uintn_t memory_map_size,
				grub_efi_uintn_t descriptor_size,
				grub_efi_uint32_t descriptor_version,
				grub_efi_memory_descriptor_t *virtual_map)
  PHYSICAL_ATTRIBUTE;

grub_efi_status_t
efiemu_convert_pointer (grub_efi_uintn_t debug_disposition,
			void **address)
  PHYSICAL_ATTRIBUTE;

grub_efi_status_t
efiemu_get_variable (grub_efi_char16_t *variable_name,
		     const grub_efi_guid_t *vendor_guid,
		     grub_efi_uint32_t *attributes,
		     grub_efi_uintn_t *data_size,
		     void *data);

grub_efi_status_t
efiemu_get_next_variable_name (grub_efi_uintn_t *variable_name_size,
			       grub_efi_char16_t *variable_name,
			       grub_efi_guid_t *vendor_guid);

grub_efi_status_t
efiemu_set_variable (grub_efi_char16_t *variable_name,
		     const grub_efi_guid_t *vendor_guid,
		     grub_efi_uint32_t attributes,
		     grub_efi_uintn_t data_size,
		     void *data);
grub_efi_status_t
efiemu_get_next_high_monotonic_count (grub_efi_uint32_t *high_count);
void
efiemu_reset_system (grub_efi_reset_type_t reset_type,
		     grub_efi_status_t reset_status,
		     grub_efi_uintn_t data_size,
		     grub_efi_char16_t *reset_data);

grub_efi_status_t
EFI_FUNC (efiemu_set_virtual_address_map) (grub_efi_uintn_t,
					      grub_efi_uintn_t,
					      grub_efi_uint32_t,
					      grub_efi_memory_descriptor_t *)
     PHYSICAL_ATTRIBUTE;
grub_efi_status_t
EFI_FUNC (efiemu_convert_pointer) (grub_efi_uintn_t debug_disposition,
				      void **address)
     PHYSICAL_ATTRIBUTE;
static grub_uint32_t
efiemu_getcrc32 (grub_uint32_t crc, void *buf, int size)
     PHYSICAL_ATTRIBUTE;
static void
init_crc32_table (void)
     PHYSICAL_ATTRIBUTE;
static grub_uint32_t
reflect (grub_uint32_t ref, int len)
     PHYSICAL_ATTRIBUTE;

/*
  The log. It's used when examining memory dump
*/
static grub_uint8_t loge[1000] = "EFIEMULOG";
static int logn = 9;
#define LOG(x)   { if (logn<900) loge[logn++]=x; }

/* Interface with grub */
extern grub_uint8_t efiemu_ptv_relocated;
struct grub_efi_runtime_services efiemu_runtime_services;
struct grub_efi_system_table efiemu_system_table;
extern struct grub_efiemu_ptv_rel efiemu_ptv_relloc[];
extern grub_uint8_t efiemu_variables[];
extern grub_uint32_t efiemu_varsize;
extern grub_uint32_t efiemu_high_monotonic_count;
extern grub_int16_t efiemu_time_zone;
extern grub_uint8_t efiemu_time_daylight;
extern grub_uint32_t efiemu_time_accuracy;

/* Some standard functions because we need to be standalone */
static void
efiemu_memcpy (void *to, const void *from, int count)
{
  int i;
  for (i = 0; i < count; i++)
    ((grub_uint8_t *) to)[i] = ((const grub_uint8_t *) from)[i];
}

static int
efiemu_str16equal (grub_uint16_t *a, grub_uint16_t *b)
{
  grub_uint16_t *ptr1, *ptr2;
  for (ptr1=a,ptr2=b; *ptr1 && *ptr2 == *ptr1; ptr1++, ptr2++);
  return *ptr2 == *ptr1;
}

static grub_size_t
efiemu_str16len (grub_uint16_t *a)
{
  grub_uint16_t *ptr1;
  for (ptr1 = a; *ptr1; ptr1++);
  return ptr1 - a;
}

static int
efiemu_memequal (const void *a, const void *b, grub_size_t n)
{
  grub_uint8_t *ptr1, *ptr2;
  for (ptr1 = (grub_uint8_t *) a, ptr2 = (grub_uint8_t *)b;
       ptr1 < (grub_uint8_t *)a + n && *ptr2 == *ptr1; ptr1++, ptr2++);
  return ptr1 == a + n;
}

static void
efiemu_memset (grub_uint8_t *a, grub_uint8_t b, grub_size_t n)
{
  grub_uint8_t *ptr1;
  for (ptr1=a; ptr1 < a + n; ptr1++)
    *ptr1 = b;
}

static inline void
write_cmos (grub_uint8_t addr, grub_uint8_t val)
{
  __asm__ __volatile__ ("outb %%al,$0x70\n"
			"mov %%cl, %%al\n"
			"outb %%al,$0x71": :"a" (addr), "c" (val));
}

static inline grub_uint8_t
read_cmos (grub_uint8_t addr)
{
  grub_uint8_t ret;
  __asm__ __volatile__ ("outb %%al, $0x70\n"
			"inb $0x71, %%al": "=a"(ret) :"a" (addr));
  return ret;
}

/* Needed by some gcc versions */
int __stack_chk_fail ()
{
  return 0;
}

/* The function that implement runtime services as specified in
   EFI specification */
static inline grub_uint8_t
bcd_to_hex (grub_uint8_t in)
{
  return 10 * ((in & 0xf0) >> 4) + (in & 0x0f);
}

grub_efi_status_t
EFI_FUNC (efiemu_get_time) (grub_efi_time_t *time,
			       grub_efi_time_capabilities_t *capabilities)
{
  LOG ('a');
  grub_uint8_t state;
  state = read_cmos (0xb);
  if (!(state & (1 << 2)))
    {
      time->year = 2000 + bcd_to_hex (read_cmos (0x9));
      time->month = bcd_to_hex (read_cmos (0x8));
      time->day = bcd_to_hex (read_cmos (0x7));
      time->hour = bcd_to_hex (read_cmos (0x4));
      if (time->hour >= 81)
	time->hour -= 80 - 12;
      if (time->hour == 24)
	time->hour = 0;
      time->minute = bcd_to_hex (read_cmos (0x2));
      time->second = bcd_to_hex (read_cmos (0x0));
    }
  else
    {
      time->year = 2000 + read_cmos (0x9);
      time->month = read_cmos (0x8);
      time->day = read_cmos (0x7);
      time->hour = read_cmos (0x4);
      if (time->hour >= 0x81)
	time->hour -= 0x80 - 12;
      if (time->hour == 24)
	time->hour = 0;
      time->minute = read_cmos (0x2);
      time->second = read_cmos (0x0);
    }
  time->nanosecond = 0;
  time->pad1 = 0;
  time->pad2 = 0;
  time->time_zone = efiemu_time_zone;
  time->daylight = efiemu_time_daylight;
  capabilities->resolution = 1;
  capabilities->accuracy = efiemu_time_accuracy;
  capabilities->sets_to_zero = 0;
  return GRUB_EFI_SUCCESS;
}

grub_efi_status_t
EFI_FUNC (efiemu_set_time) (grub_efi_time_t *time)
{
  LOG ('b');
  grub_uint8_t state;
  state = read_cmos (0xb);
  write_cmos (0xb, state | 0x6);
  write_cmos (0x9, time->year - 2000);
  write_cmos (0x8, time->month);
  write_cmos (0x7, time->day);
  write_cmos (0x4, time->hour);
  write_cmos (0x2, time->minute);
  write_cmos (0x0, time->second);
  efiemu_time_zone = time->time_zone;
  efiemu_time_daylight = time->daylight;
  return GRUB_EFI_SUCCESS;
}

/* Following 2 functions are vendor specific. So announce it as unsupported */
grub_efi_status_t
EFI_FUNC (efiemu_get_wakeup_time) (grub_efi_boolean_t *enabled,
				      grub_efi_boolean_t *pending,
				      grub_efi_time_t *time)
{
  LOG ('c');
  return GRUB_EFI_UNSUPPORTED;
}

grub_efi_status_t
EFI_FUNC (efiemu_set_wakeup_time) (grub_efi_boolean_t enabled,
				      grub_efi_time_t *time)
{
  LOG ('d');
  return GRUB_EFI_UNSUPPORTED;
}

static grub_uint32_t crc32_table [256];

static grub_uint32_t
reflect (grub_uint32_t ref, int len)
{
  grub_uint32_t result = 0;
  int i;

  for (i = 1; i <= len; i++)
    {
      if (ref & 1)
	result |= 1 << (len - i);
      ref >>= 1;
    }

  return result;
}

static void
init_crc32_table (void)
{
  grub_uint32_t polynomial = 0x04c11db7;
  int i, j;

  for(i = 0; i < 256; i++)
    {
      crc32_table[i] = reflect(i, 8) << 24;
      for (j = 0; j < 8; j++)
        crc32_table[i] = (crc32_table[i] << 1) ^
            (crc32_table[i] & (1 << 31) ? polynomial : 0);
      crc32_table[i] = reflect(crc32_table[i], 32);
    }
}

static grub_uint32_t
efiemu_getcrc32 (grub_uint32_t crc, void *buf, int size)
{
  int i;
  grub_uint8_t *data = buf;

  if (! crc32_table[1])
    init_crc32_table ();

  crc^= 0xffffffff;

  for (i = 0; i < size; i++)
    {
      crc = (crc >> 8) ^ crc32_table[(crc & 0xFF) ^ *data];
      data++;
    }

  return crc ^ 0xffffffff;
}


grub_efi_status_t EFI_FUNC
(efiemu_set_virtual_address_map) (grub_efi_uintn_t memory_map_size,
				  grub_efi_uintn_t descriptor_size,
				  grub_efi_uint32_t descriptor_version,
				  grub_efi_memory_descriptor_t *virtual_map)
{
  struct grub_efiemu_ptv_rel *cur_relloc;

  LOG ('e');

  /* Ensure that we are called only once */
  if (efiemu_ptv_relocated)
    return GRUB_EFI_UNSUPPORTED;
  efiemu_ptv_relocated = 1;

  /* Correct addresses using information supplied by grub */
  for (cur_relloc = efiemu_ptv_relloc; cur_relloc->size;cur_relloc++)
    {
      grub_int64_t corr = 0;
      grub_efi_memory_descriptor_t *descptr;

      /* Compute correction */
      for (descptr = virtual_map;
	   ((grub_uint8_t *) descptr - (grub_uint8_t *) virtual_map)
	     < memory_map_size;
	   descptr = (grub_efi_memory_descriptor_t *)
	     ((grub_uint8_t *) descptr + descriptor_size))
	{
	  if (descptr->type == cur_relloc->plustype)
	    corr += descptr->virtual_start - descptr->physical_start;
	  if (descptr->type == cur_relloc->minustype)
	    corr -= descptr->virtual_start - descptr->physical_start;
	}

      /* Apply correction */
      switch (cur_relloc->size)
	{
	case 8:
	  *((grub_uint64_t *) (grub_addr_t) cur_relloc->addr) += corr;
	  break;
	case 4:
	  *((grub_uint32_t *) (grub_addr_t) cur_relloc->addr) += corr;
	  break;
	case 2:
	  *((grub_uint16_t *) (grub_addr_t) cur_relloc->addr) += corr;
	  break;
	case 1:
	  *((grub_uint8_t *) (grub_addr_t) cur_relloc->addr) += corr;
	  break;
	}
    }

  /* Recompute crc32 of system table and runtime services */
  efiemu_system_table.hdr.crc32 = 0;
  efiemu_system_table.hdr.crc32 = efiemu_getcrc32
    (0, &efiemu_system_table, sizeof (efiemu_system_table));

  efiemu_runtime_services.hdr.crc32 = 0;
  efiemu_runtime_services.hdr.crc32 = efiemu_getcrc32
    (0, &efiemu_runtime_services, sizeof (efiemu_runtime_services));

  return GRUB_EFI_SUCCESS;
}

/* since efiemu_set_virtual_address_map corrects all the pointers
   we don't need efiemu_convert_pointer */
grub_efi_status_t
EFI_FUNC (efiemu_convert_pointer) (grub_efi_uintn_t debug_disposition,
				      void **address)
{
  LOG ('f');
  return GRUB_EFI_UNSUPPORTED;
}

/* Next comes variable services. Because we have no vendor-independent
   way to store these variables we have no non-volatility */

/* Find variable by name and GUID. */
static struct efi_variable *
find_variable (const grub_efi_guid_t *vendor_guid,
	       grub_efi_char16_t *variable_name)
{
  grub_uint8_t *ptr;
  struct efi_variable *efivar;

  for (ptr = efiemu_variables; ptr < efiemu_variables + efiemu_varsize; )
    {
      efivar = (struct efi_variable *) ptr;
      if (!efivar->namelen)
	return 0;
      if (efiemu_str16equal((grub_efi_char16_t *)(efivar + 1), variable_name)
	  && efiemu_memequal (&(efivar->guid), vendor_guid,
			      sizeof (efivar->guid)))
	return efivar;
      ptr += efivar->namelen + efivar->size + sizeof (*efivar);
    }
  return 0;
}

grub_efi_status_t
EFI_FUNC (efiemu_get_variable) (grub_efi_char16_t *variable_name,
				const grub_efi_guid_t *vendor_guid,
				grub_efi_uint32_t *attributes,
				grub_efi_uintn_t *data_size,
				void *data)
{
  struct efi_variable *efivar;
  LOG ('g');
  efivar = find_variable (vendor_guid, variable_name);
  if (!efivar)
    return GRUB_EFI_NOT_FOUND;
  if (*data_size < efivar->size)
    {
      *data_size = efivar->size;
      return GRUB_EFI_BUFFER_TOO_SMALL;
    }
  *data_size = efivar->size;
  efiemu_memcpy (data, (grub_uint8_t *)(efivar + 1) + efivar->namelen,
		 efivar->size);
  *attributes = efivar->attributes;

  return GRUB_EFI_SUCCESS;
}

grub_efi_status_t EFI_FUNC
(efiemu_get_next_variable_name) (grub_efi_uintn_t *variable_name_size,
				 grub_efi_char16_t *variable_name,
				 grub_efi_guid_t *vendor_guid)
{
  struct efi_variable *efivar;
  LOG ('l');

  if (!variable_name_size || !variable_name || !vendor_guid)
    return GRUB_EFI_INVALID_PARAMETER;
  if (variable_name[0])
    {
      efivar = find_variable (vendor_guid, variable_name);
      if (!efivar)
	return GRUB_EFI_NOT_FOUND;
      efivar = (struct efi_variable *)((grub_uint8_t *)efivar
				       + efivar->namelen
				       + efivar->size + sizeof (*efivar));
    }
  else
    efivar = (struct efi_variable *) (efiemu_variables);

  LOG ('m');
  if ((grub_uint8_t *)efivar >= efiemu_variables + efiemu_varsize
      || !efivar->namelen)
    return GRUB_EFI_NOT_FOUND;
  if (*variable_name_size < efivar->namelen)
    {
      *variable_name_size = efivar->namelen;
      return GRUB_EFI_BUFFER_TOO_SMALL;
    }

  efiemu_memcpy (variable_name, efivar + 1, efivar->namelen);
  efiemu_memcpy (vendor_guid, &(efivar->guid),
		 sizeof (efivar->guid));

  LOG('h');
  return GRUB_EFI_SUCCESS;
}

grub_efi_status_t
EFI_FUNC (efiemu_set_variable) (grub_efi_char16_t *variable_name,
				const grub_efi_guid_t *vendor_guid,
				grub_efi_uint32_t attributes,
				grub_efi_uintn_t data_size,
				void *data)
{
  struct efi_variable *efivar;
  grub_uint8_t *ptr;
  LOG('i');
  if (!variable_name[0])
    return GRUB_EFI_INVALID_PARAMETER;
  efivar = find_variable (vendor_guid, variable_name);

  /* Delete variable if any */
  if (efivar)
    {
      efiemu_memcpy (efivar, (grub_uint8_t *)(efivar + 1)
		     + efivar->namelen + efivar->size,
		     (efiemu_variables + efiemu_varsize)
		     - ((grub_uint8_t *)(efivar + 1)
			+ efivar->namelen + efivar->size));
      efiemu_memset (efiemu_variables + efiemu_varsize
		     - (sizeof (*efivar) + efivar->namelen + efivar->size),
		     0, (sizeof (*efivar) + efivar->namelen + efivar->size));
    }

  if (!data_size)
    return GRUB_EFI_SUCCESS;

  for (ptr = efiemu_variables; ptr < efiemu_variables + efiemu_varsize; )
    {
      efivar = (struct efi_variable *) ptr;
      ptr += efivar->namelen + efivar->size + sizeof (*efivar);
      if (!efivar->namelen)
	break;
    }
  if ((grub_uint8_t *)(efivar + 1) + data_size
      + 2 * (efiemu_str16len (variable_name) + 1)
      >= efiemu_variables + efiemu_varsize)
    return GRUB_EFI_OUT_OF_RESOURCES;

  efiemu_memcpy (&(efivar->guid), vendor_guid, sizeof (efivar->guid));
  efivar->namelen = 2 * (efiemu_str16len (variable_name) + 1);
  efivar->size = data_size;
  efivar->attributes = attributes;
  efiemu_memcpy (efivar + 1, variable_name,
		 2 * (efiemu_str16len (variable_name) + 1));
  efiemu_memcpy ((grub_uint8_t *)(efivar + 1)
		 + 2 * (efiemu_str16len (variable_name) + 1),
		 data, data_size);

  return GRUB_EFI_SUCCESS;
}

grub_efi_status_t EFI_FUNC
(efiemu_get_next_high_monotonic_count) (grub_efi_uint32_t *high_count)
{
  LOG ('j');
  if (!high_count)
    return GRUB_EFI_INVALID_PARAMETER;
  *high_count = ++efiemu_high_monotonic_count;
  return GRUB_EFI_SUCCESS;
}

/* To implement it with APM we need to go to real mode. It's too much hassle
   Besides EFI specification says that this function shouldn't be used
   on systems supporting ACPI
 */
void
EFI_FUNC (efiemu_reset_system) (grub_efi_reset_type_t reset_type,
				   grub_efi_status_t reset_status,
				   grub_efi_uintn_t data_size,
				   grub_efi_char16_t *reset_data)
{
  LOG ('k');
}

struct grub_efi_runtime_services efiemu_runtime_services =
{
  .hdr =
  {
    .signature = GRUB_EFIEMU_RUNTIME_SERVICES_SIGNATURE,
    .revision = 0x0001000a,
    .header_size = sizeof (struct grub_efi_runtime_services),
    .crc32 = 0, /* filled later*/
    .reserved = 0
  },
  .get_time = efiemu_get_time,
  .set_time = efiemu_set_time,
  .get_wakeup_time = efiemu_get_wakeup_time,
  .set_wakeup_time = efiemu_set_wakeup_time,

  .set_virtual_address_map = efiemu_set_virtual_address_map,
  .convert_pointer = efiemu_convert_pointer,

  .get_variable = efiemu_get_variable,
  .get_next_variable_name = efiemu_get_next_variable_name,
  .set_variable = efiemu_set_variable,
  .get_next_high_monotonic_count = efiemu_get_next_high_monotonic_count,

  .reset_system = efiemu_reset_system
};


static grub_uint16_t efiemu_vendor[] =
  {'G', 'R', 'U', 'B', ' ', 'E', 'F', 'I', ' ',
   'R', 'U', 'N', 'T', 'I', 'M', 'E', 0};

struct grub_efi_system_table efiemu_system_table =
{
  .hdr =
  {
    .signature = GRUB_EFIEMU_SYSTEM_TABLE_SIGNATURE,
    .revision = 0x0001000a,
    .header_size = sizeof (struct grub_efi_system_table),
    .crc32 = 0, /* filled later*/
    .reserved = 0
  },
  .firmware_vendor = efiemu_vendor,
  .firmware_revision = 0x0001000a,
  .console_in_handler = 0,
  .con_in = 0,
  .console_out_handler = 0,
  .con_out = 0,
  .standard_error_handle = 0,
  .std_err = 0,
  .runtime_services = &efiemu_runtime_services,
  .boot_services = 0,
  .num_table_entries = 0,
  .configuration_table = 0
};

