/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2010  Free Software Foundation, Inc.
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

#ifndef GRUB_SMBUS_HEADER
#define GRUB_SMBUS_HEADER 1

#define GRUB_SMB_RAM_START_ADDR 0x50
#define GRUB_SMB_RAM_NUM_MAX 0x08

#define GRUB_SMBUS_SPD_MEMORY_TYPE_ADDR 2
#define GRUB_SMBUS_SPD_MEMORY_TYPE_DDR2 8
#define GRUB_SMBUS_SPD_MEMORY_NUM_BANKS_ADDR 17
#define GRUB_SMBUS_SPD_MEMORY_NUM_ROWS_ADDR 3
#define GRUB_SMBUS_SPD_MEMORY_NUM_COLUMNS_ADDR 4
#define GRUB_SMBUS_SPD_MEMORY_NUM_OF_RANKS_ADDR 5
#define GRUB_SMBUS_SPD_MEMORY_NUM_OF_RANKS_MASK 0x7
#define GRUB_SMBUS_SPD_MEMORY_CAS_LATENCY_ADDR 18
#define GRUB_SMBUS_SPD_MEMORY_CAS_LATENCY_MIN_VALUE 5
#define GRUB_SMBUS_SPD_MEMORY_TRAS_ADDR 30
#define GRUB_SMBUS_SPD_MEMORY_TRTP_ADDR 38

#ifndef ASM_FILE

struct grub_smbus_spd
{
  grub_uint8_t written_size;
  grub_uint8_t log_total_flash_size;
  grub_uint8_t memory_type;
  union
  {
    grub_uint8_t unknown[253];
    struct {
      grub_uint8_t num_rows;
      grub_uint8_t num_columns;
      grub_uint8_t num_of_ranks;
      grub_uint8_t unused1[12];
      grub_uint8_t num_of_banks;
      grub_uint8_t unused2[2];
      grub_uint8_t cas_latency;
      grub_uint8_t unused3[9];
      grub_uint8_t rank_capacity;
      grub_uint8_t unused4[1];
      grub_uint8_t tras;
      grub_uint8_t unused5[7];
      grub_uint8_t trtp;
      grub_uint8_t unused6[31];
      grub_uint8_t part_number[18];
      grub_uint8_t unused7[165];
    } ddr2;
  };
};

#endif

#endif
