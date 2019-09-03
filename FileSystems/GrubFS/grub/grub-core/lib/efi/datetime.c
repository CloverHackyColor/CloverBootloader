/* kern/efi/datetime.c - efi datetime function.
 *
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

#include <grub/types.h>
#include <grub/symbol.h>
#include <grub/efi/api.h>
#include <grub/efi/efi.h>
#include <grub/datetime.h>
#include <grub/dl.h>

GRUB_MOD_LICENSE ("GPLv3+");

grub_err_t
grub_get_datetime (struct grub_datetime *datetime)
{
  grub_efi_status_t status;
  struct grub_efi_time efi_time;

  status = efi_call_2 (grub_efi_system_table->runtime_services->get_time,
                       &efi_time, 0);

  if (status)
    return grub_error (GRUB_ERR_INVALID_COMMAND,
                       "can\'t get datetime using efi");
  else
    {
      datetime->year = efi_time.year;
      datetime->month = efi_time.month;
      datetime->day = efi_time.day;
      datetime->hour = efi_time.hour;
      datetime->minute = efi_time.minute;
      datetime->second = efi_time.second;
    }

  return 0;
}

grub_err_t
grub_set_datetime (struct grub_datetime *datetime)
{
  grub_efi_status_t status;
  struct grub_efi_time efi_time;

  status = efi_call_2 (grub_efi_system_table->runtime_services->get_time,
                       &efi_time, 0);

  if (status)
    return grub_error (GRUB_ERR_INVALID_COMMAND,
                       "can\'t get datetime using efi");

  efi_time.year = datetime->year;
  efi_time.month = datetime->month;
  efi_time.day = datetime->day;
  efi_time.hour = datetime->hour;
  efi_time.minute = datetime->minute;
  efi_time.second = datetime->second;

  status = efi_call_1 (grub_efi_system_table->runtime_services->set_time,
                       &efi_time);

  if (status)
    return grub_error (GRUB_ERR_INVALID_COMMAND,
                       "can\'t set datetime using efi");

  return 0;
}
