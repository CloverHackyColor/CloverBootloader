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

#ifndef GRUB_APM_MACHINE_HEADER
#define GRUB_APM_MACHINE_HEADER	1

#include <grub/types.h>

struct grub_apm_info
{
  grub_uint16_t cseg;
  grub_uint32_t offset;
  grub_uint16_t cseg_16;
  grub_uint16_t dseg;
  grub_uint16_t flags;
  grub_uint16_t cseg_len;
  grub_uint16_t cseg_16_len;
  grub_uint16_t dseg_len;
  grub_uint16_t version;
};

enum
  {
    GRUB_APM_FLAGS_16BITPROTECTED_SUPPORTED = 1,
    GRUB_APM_FLAGS_32BITPROTECTED_SUPPORTED = 2,
    GRUB_APM_FLAGS_CPUIDLE_SLOWS_DOWN = 4,
    GRUB_APM_FLAGS_DISABLED = 8,
    GRUB_APM_FLAGS_DISENGAGED = 16,
  };

int grub_apm_get_info (struct grub_apm_info *info);

#endif
