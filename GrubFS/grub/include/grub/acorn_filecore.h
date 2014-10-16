/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2004,2005,2007  Free Software Foundation, Inc.
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

#ifndef GRUB_ACORN_FILECORE_HEADER
#define GRUB_ACORN_FILECORE_HEADER	1

#include <grub/types.h>

struct grub_filecore_disc_record
{
  grub_uint8_t log2secsize;
  grub_uint8_t secspertrack;
  grub_uint8_t heads;
  grub_uint8_t density;
  grub_uint8_t idlen;
  grub_uint8_t log2bpmb;
  grub_uint8_t skew;
  grub_uint8_t bootoption;
  /* In bits 0-5, flags in bits 6 and 7.  */
  grub_uint8_t lowsector;
  grub_uint8_t nzones;
  grub_uint16_t zone_spare;
  grub_uint32_t root_address;
  /* Disc size in bytes.  */
  grub_uint32_t disc_size;
  grub_uint16_t cycle_id;
  char disc_name[10];
  /* Yes, it is 32 bits!  */
  grub_uint32_t disctype;
  /* Most significant part of the disc size.  */
  grub_uint32_t disc_size2;
  grub_uint8_t share_size;
  grub_uint8_t big_flag;
  grub_uint8_t reserved[18];
};


#endif /* ! GRUB_ACORN_FILECORE_HEADER */
