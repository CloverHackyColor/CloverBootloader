/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2008, 2009  Free Software Foundation, Inc.
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

#ifndef	GRUB_CMOS_H
#define	GRUB_CMOS_H	1

#include <grub/types.h>
#if !defined (__powerpc__) && !defined (__sparc__)
#include <grub/cpu/io.h>
#include <grub/cpu/cmos.h>
#endif

#define GRUB_CMOS_INDEX_SECOND		0
#define GRUB_CMOS_INDEX_SECOND_ALARM	1
#define GRUB_CMOS_INDEX_MINUTE		2
#define GRUB_CMOS_INDEX_MINUTE_ALARM	3
#define GRUB_CMOS_INDEX_HOUR		4
#define GRUB_CMOS_INDEX_HOUR_ALARM	5
#define GRUB_CMOS_INDEX_DAY_OF_WEEK	6
#define GRUB_CMOS_INDEX_DAY_OF_MONTH	7
#define GRUB_CMOS_INDEX_MONTH		8
#define GRUB_CMOS_INDEX_YEAR		9

#define GRUB_CMOS_INDEX_STATUS_A	0xA
#define GRUB_CMOS_INDEX_STATUS_B	0xB
#define GRUB_CMOS_INDEX_STATUS_C	0xC
#define GRUB_CMOS_INDEX_STATUS_D	0xD

#define GRUB_CMOS_STATUS_B_DAYLIGHT	1
#define GRUB_CMOS_STATUS_B_24HOUR	2
#define GRUB_CMOS_STATUS_B_BINARY	4

static inline grub_uint8_t
grub_bcd_to_num (grub_uint8_t a)
{
  return ((a >> 4) * 10 + (a & 0xF));
}

static inline grub_uint8_t
grub_num_to_bcd (grub_uint8_t a)
{
  return (((a / 10) << 4) + (a % 10));
}

#if !defined (__powerpc__) && !defined (__sparc__)
static inline grub_err_t
grub_cmos_read (grub_uint8_t index, grub_uint8_t *val)
{
  if (index & 0x80)
    {
      grub_outb (index & 0x7f, GRUB_CMOS_ADDR_REG_HI);
      *val = grub_inb (GRUB_CMOS_DATA_REG_HI);
    }
  else
    {
      grub_outb (index & 0x7f, GRUB_CMOS_ADDR_REG);
      *val = grub_inb (GRUB_CMOS_DATA_REG);
    }
  return GRUB_ERR_NONE;
}

static inline grub_err_t
grub_cmos_write (grub_uint8_t index, grub_uint8_t value)
{
  if (index & 0x80)
    {
      grub_outb (index & 0x7f, GRUB_CMOS_ADDR_REG_HI);
      grub_outb (value, GRUB_CMOS_DATA_REG_HI);
    }
  else
    {
      grub_outb (index & 0x7f, GRUB_CMOS_ADDR_REG);
      grub_outb (value, GRUB_CMOS_DATA_REG);
    }
  return GRUB_ERR_NONE;
}
#else
grub_err_t grub_cmos_find_port (void);
extern volatile grub_uint8_t *grub_cmos_port;

static inline grub_err_t
grub_cmos_read (grub_uint8_t index, grub_uint8_t *val)
{
  if (!grub_cmos_port)
    {
      grub_err_t err;
      err = grub_cmos_find_port ();
      if (err)
	return err;
    }
  grub_cmos_port[((index & 0x80) >> 6) | 0] = index & 0x7f;
  *val = grub_cmos_port[((index & 0x80) >> 6) | 1];
  return GRUB_ERR_NONE;
}

static inline grub_err_t
grub_cmos_write (grub_uint8_t index, grub_uint8_t val)
{
  if (!grub_cmos_port)
    {
      grub_err_t err;
      err = grub_cmos_find_port ();
      if (err)
	return err;
    }
  grub_cmos_port[((index & 0x80) >> 6) | 0] = index;
  grub_cmos_port[((index & 0x80) >> 6) | 1] = val;
  return GRUB_ERR_NONE;
}

#endif

#endif /* GRUB_CMOS_H */
