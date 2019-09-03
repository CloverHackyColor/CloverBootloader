/* kern/cmos_datetime.c - CMOS datetime function.
 *
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

#include <grub/datetime.h>
#include <grub/ieee1275/ieee1275.h>
#include <grub/misc.h>
#include <grub/dl.h>
#if defined (__powerpc__) || defined (__sparc__)
#include <grub/cmos.h>
#endif

GRUB_MOD_LICENSE ("GPLv3+");

static char *rtc = 0;
static int no_ieee1275_rtc = 0;

/* Helper for find_rtc.  */
static int
find_rtc_iter (struct grub_ieee1275_devalias *alias)
{
  if (grub_strcmp (alias->type, "rtc") == 0)
    {
      grub_dprintf ("datetime", "Found RTC %s\n", alias->path);
      rtc = grub_strdup (alias->path);
      return 1;
    }
  return 0;
}

static void
find_rtc (void)
{
  grub_ieee1275_devices_iterate (find_rtc_iter);
  if (!rtc)
    no_ieee1275_rtc = 1;
}

grub_err_t
grub_get_datetime (struct grub_datetime *datetime)
{
  struct get_time_args
  {
    struct grub_ieee1275_common_hdr common;
    grub_ieee1275_cell_t method;
    grub_ieee1275_cell_t device;
    grub_ieee1275_cell_t catch_result;
    grub_ieee1275_cell_t year;
    grub_ieee1275_cell_t month;
    grub_ieee1275_cell_t day;
    grub_ieee1275_cell_t hour;
    grub_ieee1275_cell_t minute;
    grub_ieee1275_cell_t second;
  }
  args;
  int status;
  grub_ieee1275_ihandle_t ihandle;

  if (no_ieee1275_rtc)
    return grub_get_datetime_cmos (datetime);
  if (!rtc)
    find_rtc ();
  if (!rtc)
    return grub_get_datetime_cmos (datetime);

  status = grub_ieee1275_open (rtc, &ihandle);
  if (status == -1)
    return grub_error (GRUB_ERR_IO, "couldn't open RTC");

  INIT_IEEE1275_COMMON (&args.common, "call-method", 2, 7);
  args.device = (grub_ieee1275_cell_t) ihandle;
  args.method = (grub_ieee1275_cell_t) "get-time";

  status = IEEE1275_CALL_ENTRY_FN (&args);

  grub_ieee1275_close (ihandle);

  if (status == -1 || args.catch_result)
    return grub_error (GRUB_ERR_IO, "get-time failed");

  datetime->year = args.year;
  datetime->month = args.month;
  datetime->day = args.day;
  datetime->hour = args.hour;
  datetime->minute = args.minute;
  datetime->second = args.second;

  return GRUB_ERR_NONE;
}

grub_err_t
grub_set_datetime (struct grub_datetime *datetime)
{
  struct set_time_args
  {
    struct grub_ieee1275_common_hdr common;
    grub_ieee1275_cell_t method;
    grub_ieee1275_cell_t device;
    grub_ieee1275_cell_t year;
    grub_ieee1275_cell_t month;
    grub_ieee1275_cell_t day;
    grub_ieee1275_cell_t hour;
    grub_ieee1275_cell_t minute;
    grub_ieee1275_cell_t second;
    grub_ieee1275_cell_t catch_result;
  }
  args;
  int status;
  grub_ieee1275_ihandle_t ihandle;

  if (no_ieee1275_rtc)
    return grub_set_datetime_cmos (datetime);
  if (!rtc)
    find_rtc ();
  if (!rtc)
    return grub_set_datetime_cmos (datetime);

  status = grub_ieee1275_open (rtc, &ihandle);
  if (status == -1)
    return grub_error (GRUB_ERR_IO, "couldn't open RTC");

  INIT_IEEE1275_COMMON (&args.common, "call-method", 8, 1);
  args.device = (grub_ieee1275_cell_t) ihandle;
  args.method = (grub_ieee1275_cell_t) "set-time";

  args.year = datetime->year;
  args.month = datetime->month;
  args.day = datetime->day;
  args.hour = datetime->hour;
  args.minute = datetime->minute;
  args.second = datetime->second;

  status = IEEE1275_CALL_ENTRY_FN (&args);

  grub_ieee1275_close (ihandle);

  if (status == -1 || args.catch_result)
    return grub_error (GRUB_ERR_IO, "set-time failed");

  return GRUB_ERR_NONE;
}
