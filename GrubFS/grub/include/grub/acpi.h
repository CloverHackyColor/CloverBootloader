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

#ifndef GRUB_ACPI_HEADER
#define GRUB_ACPI_HEADER	1

#ifndef GRUB_DSDT_TEST
#include <grub/types.h>
#include <grub/err.h>
#endif

#define GRUB_RSDP_SIGNATURE "RSD PTR "
#define GRUB_RSDP_SIGNATURE_SIZE 8

struct grub_acpi_rsdp_v10
{
  grub_uint8_t signature[GRUB_RSDP_SIGNATURE_SIZE];
  grub_uint8_t checksum;
  grub_uint8_t oemid[6];
  grub_uint8_t revision;
  grub_uint32_t rsdt_addr;
} GRUB_PACKED;

struct grub_acpi_rsdp_v20
{
  struct grub_acpi_rsdp_v10 rsdpv1;
  grub_uint32_t length;
  grub_uint64_t xsdt_addr;
  grub_uint8_t checksum;
  grub_uint8_t reserved[3];
} GRUB_PACKED;

struct grub_acpi_table_header
{
  grub_uint8_t signature[4];
  grub_uint32_t length;
  grub_uint8_t revision;
  grub_uint8_t checksum;
  grub_uint8_t oemid[6];
  grub_uint8_t oemtable[8];
  grub_uint32_t oemrev;
  grub_uint8_t creator_id[4];
  grub_uint32_t creator_rev;
} GRUB_PACKED;

#define GRUB_ACPI_FADT_SIGNATURE "FACP"

struct grub_acpi_fadt
{
  struct grub_acpi_table_header hdr;
  grub_uint32_t facs_addr;
  grub_uint32_t dsdt_addr;
  grub_uint8_t somefields1[20];
  grub_uint32_t pm1a;
  grub_uint8_t somefields2[64];
  grub_uint64_t facs_xaddr;
  grub_uint64_t dsdt_xaddr;
  grub_uint8_t somefields3[96];
} GRUB_PACKED;

#define GRUB_ACPI_MADT_SIGNATURE "APIC"

struct grub_acpi_madt_entry_header
{
  grub_uint8_t type;
  grub_uint8_t len;
};

struct grub_acpi_madt
{
  struct grub_acpi_table_header hdr;
  grub_uint32_t lapic_addr;
  grub_uint32_t flags;
  struct grub_acpi_madt_entry_header entries[0];
};

enum
  {
    GRUB_ACPI_MADT_ENTRY_TYPE_LAPIC = 0,
    GRUB_ACPI_MADT_ENTRY_TYPE_IOAPIC = 1,
    GRUB_ACPI_MADT_ENTRY_TYPE_INTERRUPT_OVERRIDE = 2,
    GRUB_ACPI_MADT_ENTRY_TYPE_LAPIC_NMI = 4,
    GRUB_ACPI_MADT_ENTRY_TYPE_SAPIC = 6,
    GRUB_ACPI_MADT_ENTRY_TYPE_LSAPIC = 7,
    GRUB_ACPI_MADT_ENTRY_TYPE_PLATFORM_INT_SOURCE = 8
  };

struct grub_acpi_madt_entry_lapic
{
  struct grub_acpi_madt_entry_header hdr;
  grub_uint8_t acpiid;
  grub_uint8_t apicid;
  grub_uint32_t flags;
};

struct grub_acpi_madt_entry_ioapic
{
  struct grub_acpi_madt_entry_header hdr;
  grub_uint8_t id;
  grub_uint8_t pad;
  grub_uint32_t address;
  grub_uint32_t global_sys_interrupt;
};

struct grub_acpi_madt_entry_interrupt_override
{
  struct grub_acpi_madt_entry_header hdr;
  grub_uint8_t bus;
  grub_uint8_t source;
  grub_uint32_t global_sys_interrupt;
  grub_uint16_t flags;
} GRUB_PACKED;


struct grub_acpi_madt_entry_lapic_nmi
{
  struct grub_acpi_madt_entry_header hdr;
  grub_uint8_t acpiid;
  grub_uint16_t flags;
  grub_uint8_t lint;
} GRUB_PACKED;

struct grub_acpi_madt_entry_sapic
{
  struct grub_acpi_madt_entry_header hdr;
  grub_uint8_t id;
  grub_uint8_t pad;
  grub_uint32_t global_sys_interrupt_base;
  grub_uint64_t addr;
};

struct grub_acpi_madt_entry_lsapic
{
  struct grub_acpi_madt_entry_header hdr;
  grub_uint8_t cpu_id;
  grub_uint8_t id;
  grub_uint8_t eid;
  grub_uint8_t pad[3];
  grub_uint32_t flags;
  grub_uint32_t cpu_uid;
  grub_uint8_t cpu_uid_str[0];
};

struct grub_acpi_madt_entry_platform_int_source
{
  struct grub_acpi_madt_entry_header hdr;
  grub_uint16_t flags;
  grub_uint8_t inttype;
  grub_uint8_t cpu_id;
  grub_uint8_t cpu_eid;
  grub_uint8_t sapic_vector;
  grub_uint32_t global_sys_int;
  grub_uint32_t src_flags;
};

enum
  {
    GRUB_ACPI_MADT_ENTRY_SAPIC_FLAGS_ENABLED = 1
  };

#ifndef GRUB_DSDT_TEST
struct grub_acpi_rsdp_v10 *grub_acpi_get_rsdpv1 (void);
struct grub_acpi_rsdp_v20 *grub_acpi_get_rsdpv2 (void);
struct grub_acpi_rsdp_v10 *grub_machine_acpi_get_rsdpv1 (void);
struct grub_acpi_rsdp_v20 *grub_machine_acpi_get_rsdpv2 (void);
grub_uint8_t grub_byte_checksum (void *base, grub_size_t size);

grub_err_t grub_acpi_create_ebda (void);

void grub_acpi_halt (void);
#endif

#define GRUB_ACPI_SLP_EN (1 << 13)
#define GRUB_ACPI_SLP_TYP_OFFSET 10

enum
  {
    GRUB_ACPI_OPCODE_ZERO = 0, GRUB_ACPI_OPCODE_ONE = 1,
    GRUB_ACPI_OPCODE_NAME = 8, GRUB_ACPI_OPCODE_BYTE_CONST = 0x0a,
    GRUB_ACPI_OPCODE_WORD_CONST = 0x0b,
    GRUB_ACPI_OPCODE_DWORD_CONST = 0x0c,
    GRUB_ACPI_OPCODE_STRING_CONST = 0x0d,
    GRUB_ACPI_OPCODE_SCOPE = 0x10,
    GRUB_ACPI_OPCODE_BUFFER = 0x11,
    GRUB_ACPI_OPCODE_PACKAGE = 0x12,
    GRUB_ACPI_OPCODE_METHOD = 0x14, GRUB_ACPI_OPCODE_EXTOP = 0x5b,
    GRUB_ACPI_OPCODE_CREATE_WORD_FIELD = 0x8b,
    GRUB_ACPI_OPCODE_CREATE_BYTE_FIELD = 0x8c,
    GRUB_ACPI_OPCODE_IF = 0xa0, GRUB_ACPI_OPCODE_ONES = 0xff
  };
enum
  {
    GRUB_ACPI_EXTOPCODE_MUTEX = 0x01,
    GRUB_ACPI_EXTOPCODE_EVENT_OP = 0x02,
    GRUB_ACPI_EXTOPCODE_OPERATION_REGION = 0x80,
    GRUB_ACPI_EXTOPCODE_FIELD_OP = 0x81,
    GRUB_ACPI_EXTOPCODE_DEVICE_OP = 0x82,
    GRUB_ACPI_EXTOPCODE_PROCESSOR_OP = 0x83,
    GRUB_ACPI_EXTOPCODE_POWER_RES_OP = 0x84,
    GRUB_ACPI_EXTOPCODE_THERMAL_ZONE_OP = 0x85,
    GRUB_ACPI_EXTOPCODE_INDEX_FIELD_OP = 0x86,
    GRUB_ACPI_EXTOPCODE_BANK_FIELD_OP = 0x87,
  };

#endif /* ! GRUB_ACPI_HEADER */
