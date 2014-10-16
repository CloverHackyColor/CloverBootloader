/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2008  Free Software Foundation, Inc.
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

#ifndef	GRUB_SCSICMD_H
#define	GRUB_SCSICMD_H	1

#include <grub/types.h>

#define GRUB_SCSI_DEVTYPE_MASK	31
#define GRUB_SCSI_REMOVABLE_BIT	7
#define GRUB_SCSI_LUN_SHIFT	5

struct grub_scsi_test_unit_ready
{
  grub_uint8_t opcode;
  grub_uint8_t lun; /* 7-5 LUN, 4-0 reserved */
  grub_uint8_t reserved1;
  grub_uint8_t reserved2;
  grub_uint8_t reserved3;
  grub_uint8_t control;
  grub_uint8_t pad[6]; /* To be ATAPI compatible */
} GRUB_PACKED;

struct grub_scsi_inquiry
{
  grub_uint8_t opcode;
  grub_uint8_t lun; /* 7-5 LUN, 4-1 reserved, 0 EVPD */
  grub_uint8_t page; /* page code if EVPD=1 */
  grub_uint8_t reserved;
  grub_uint8_t alloc_length;
  grub_uint8_t control;
  grub_uint8_t pad[6]; /* To be ATAPI compatible */
} GRUB_PACKED;

struct grub_scsi_inquiry_data
{
  grub_uint8_t devtype;
  grub_uint8_t rmb;
  grub_uint16_t reserved;
  grub_uint8_t length;
  grub_uint8_t reserved2[3];
  char vendor[8];
  char prodid[16];
  char prodrev[4];
} GRUB_PACKED;

struct grub_scsi_request_sense
{
  grub_uint8_t opcode;
  grub_uint8_t lun; /* 7-5 LUN, 4-0 reserved */
  grub_uint8_t reserved1;
  grub_uint8_t reserved2;
  grub_uint8_t alloc_length;
  grub_uint8_t control;
  grub_uint8_t pad[6]; /* To be ATAPI compatible */
} GRUB_PACKED;

struct grub_scsi_request_sense_data
{
  grub_uint8_t error_code; /* 7 Valid, 6-0 Err. code */
  grub_uint8_t segment_number;
  grub_uint8_t sense_key; /*7 FileMark, 6 EndOfMedia, 5 ILI, 4-0 sense key */
  grub_uint32_t information;
  grub_uint8_t additional_sense_length;
  grub_uint32_t cmd_specific_info;
  grub_uint8_t additional_sense_code;
  grub_uint8_t additional_sense_code_qualifier;
  grub_uint8_t field_replaceable_unit_code;
  grub_uint8_t sense_key_specific[3];
  /* there can be additional sense field */
} GRUB_PACKED;

struct grub_scsi_read_capacity10
{
  grub_uint8_t opcode;
  grub_uint8_t lun; /* 7-5 LUN, 4-1 reserved, 0 reserved */
  grub_uint32_t logical_block_addr; /* only if PMI=1 */
  grub_uint8_t reserved1;
  grub_uint8_t reserved2;
  grub_uint8_t PMI;
  grub_uint8_t control;
  grub_uint16_t pad; /* To be ATAPI compatible */
} GRUB_PACKED;

struct grub_scsi_read_capacity10_data
{
  grub_uint32_t last_block;
  grub_uint32_t blocksize;
} GRUB_PACKED;

struct grub_scsi_read_capacity16
{
  grub_uint8_t opcode;
  grub_uint8_t lun; /* 7-5 LUN, 4-0 0x10 */
  grub_uint64_t logical_block_addr; /* only if PMI=1 */
  grub_uint32_t alloc_len;
  grub_uint8_t PMI;
  grub_uint8_t control;
} GRUB_PACKED;

struct grub_scsi_read_capacity16_data
{
  grub_uint64_t last_block;
  grub_uint32_t blocksize;
  grub_uint8_t pad[20];
} GRUB_PACKED;

struct grub_scsi_read10
{
  grub_uint8_t opcode;
  grub_uint8_t lun;
  grub_uint32_t lba;
  grub_uint8_t reserved;
  grub_uint16_t size;
  grub_uint8_t reserved2;
  grub_uint16_t pad;
} GRUB_PACKED;

struct grub_scsi_read12
{
  grub_uint8_t opcode;
  grub_uint8_t lun;
  grub_uint32_t lba;
  grub_uint32_t size;
  grub_uint8_t reserved;
  grub_uint8_t control;
} GRUB_PACKED;

struct grub_scsi_read16
{
  grub_uint8_t opcode;
  grub_uint8_t lun;
  grub_uint64_t lba;
  grub_uint32_t size;
  grub_uint8_t reserved;
  grub_uint8_t control;
} GRUB_PACKED;

struct grub_scsi_write10
{
  grub_uint8_t opcode;
  grub_uint8_t lun;
  grub_uint32_t lba;
  grub_uint8_t reserved;
  grub_uint16_t size;
  grub_uint8_t reserved2;
  grub_uint16_t pad;
} GRUB_PACKED;

struct grub_scsi_write12
{
  grub_uint8_t opcode;
  grub_uint8_t lun;
  grub_uint32_t lba;
  grub_uint32_t size;
  grub_uint8_t reserved;
  grub_uint8_t control;
} GRUB_PACKED;

struct grub_scsi_write16
{
  grub_uint8_t opcode;
  grub_uint8_t lun;
  grub_uint64_t lba;
  grub_uint32_t size;
  grub_uint8_t reserved;
  grub_uint8_t control;
} GRUB_PACKED;

typedef enum
  {
    grub_scsi_cmd_test_unit_ready = 0x00,
    grub_scsi_cmd_request_sense = 0x03,
    grub_scsi_cmd_inquiry = 0x12,
    grub_scsi_cmd_read_capacity10 = 0x25,
    grub_scsi_cmd_read10 = 0x28,
    grub_scsi_cmd_write10 = 0x2a,
    grub_scsi_cmd_read16 = 0x88,
    grub_scsi_cmd_write16 = 0x8a,
    grub_scsi_cmd_read_capacity16 = 0x9e,
    grub_scsi_cmd_read12 = 0xa8,
    grub_scsi_cmd_write12 = 0xaa,
  } grub_scsi_cmd_t;

typedef enum
  {
    grub_scsi_devtype_direct = 0x00,
    grub_scsi_devtype_cdrom = 0x05
  } grub_scsi_devtype_t;

#endif /* GRUB_SCSICMD_H */
