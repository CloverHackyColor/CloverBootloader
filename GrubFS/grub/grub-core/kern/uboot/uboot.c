/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2013  Free Software Foundation, Inc.
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

#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/uboot/api_public.h>
#include <grub/uboot/uboot.h>

/*
 * The main syscall entry point is not reentrant, only one call is
 * serviced until finished.
 *
 * int syscall(int call, int *retval, ...)
 * e.g. syscall(1, int *, u_int32_t, u_int32_t, u_int32_t, u_int32_t);
 *
 * call:	syscall number
 *
 * retval:	points to the return value placeholder, this is the place the
 *		syscall puts its return value, if NULL the caller does not
 *		expect a return value
 *
 * ...		syscall arguments (variable number)
 *
 * returns:	0 if the call not found, 1 if serviced
 */

extern int (*grub_uboot_syscall_ptr) (int, int *, ...);
extern int grub_uboot_syscall (int, int *, ...);
extern grub_addr_t grub_uboot_search_hint;

static struct sys_info uboot_sys_info;
static struct mem_region uboot_mem_info[5];
static struct device_info * devices;
static int num_devices;

int
grub_uboot_api_init (void)
{
  struct api_signature *start, *end;
  struct api_signature *p;

  if (grub_uboot_search_hint)
    {
      /* Extended search range to work around Trim Slice U-Boot issue */
      start = (struct api_signature *) ((grub_uboot_search_hint & ~0x000fffff)
					- 0x00500000);
      end =
	(struct api_signature *) ((grub_addr_t) start + UBOOT_API_SEARCH_LEN -
				  API_SIG_MAGLEN + 0x00500000);
    }
  else
    {
      start = 0;
      end = (struct api_signature *) (256 * 1024 * 1024);
    }

  /* Structure alignment is (at least) 8 bytes */
  for (p = start; p < end; p = (void *) ((grub_addr_t) p + 8))
    {
      if (grub_memcmp (&(p->magic), API_SIG_MAGIC, API_SIG_MAGLEN) == 0)
	{
	  grub_uboot_syscall_ptr = p->syscall;
	  return p->version;
	}
    }

  return 0;
}

/*
 * All functions below are wrappers around the grub_uboot_syscall() function
 */

int
grub_uboot_getc (void)
{
  int c;
  if (!grub_uboot_syscall (API_GETC, NULL, &c))
    return -1;

  return c;
}

int
grub_uboot_tstc (void)
{
  int c;
  if (!grub_uboot_syscall (API_TSTC, NULL, &c))
    return -1;

  return c;
}

void
grub_uboot_putc (int c)
{
  grub_uboot_syscall (API_PUTC, NULL, &c);
}

void
grub_uboot_puts (const char *s)
{
  grub_uboot_syscall (API_PUTS, NULL, s);
}

void
grub_uboot_reset (void)
{
  grub_uboot_syscall (API_RESET, NULL, 0);
}

struct sys_info *
grub_uboot_get_sys_info (void)
{
  int retval;

  grub_memset (&uboot_sys_info, 0, sizeof (uboot_sys_info));
  grub_memset (&uboot_mem_info, 0, sizeof (uboot_mem_info));
  uboot_sys_info.mr = uboot_mem_info;
  uboot_sys_info.mr_no = sizeof (uboot_mem_info) / sizeof (struct mem_region);

  if (grub_uboot_syscall (API_GET_SYS_INFO, &retval, &uboot_sys_info))
    if (retval == 0)
      return &uboot_sys_info;

  return NULL;
}

void
grub_uboot_udelay (grub_uint32_t usec)
{
  grub_uboot_syscall (API_UDELAY, NULL, &usec);
}

grub_uint32_t
grub_uboot_get_timer (grub_uint32_t base)
{
  grub_uint32_t current;

  if (!grub_uboot_syscall (API_GET_TIMER, NULL, &current, &base))
    return 0;

  return current;
}

int
grub_uboot_dev_enum (void)
{
  struct device_info * enum_devices;
  int num_enum_devices, max_devices;

  if (num_devices)
    return num_devices;

  max_devices = 2;
  enum_devices = grub_malloc (sizeof(struct device_info) * max_devices);
  if (!enum_devices)
    return 0;

  /*
   * The API_DEV_ENUM call starts a fresh enumeration when passed a
   * struct device_info with a NULL cookie, and then depends on having
   * the previously enumerated device cookie "seeded" into the target
   * structure.
   */

  enum_devices[0].cookie = NULL;
  num_enum_devices = 0;

  if (grub_uboot_syscall (API_DEV_ENUM, NULL,
			  &enum_devices[num_enum_devices]) == 0)
    goto error;

  num_enum_devices++;

  while (enum_devices[num_enum_devices - 1].cookie != NULL)
    {
      if (num_enum_devices == max_devices)
	{
	  struct device_info *tmp;
	  int new_max;
	  new_max = max_devices * 2;
	  tmp = grub_realloc (enum_devices,
			      sizeof (struct device_info) * new_max);
	  if (!tmp)
	    {
	      /* Failed to realloc, so return what we have */
	      break;
	    }
	  enum_devices = tmp;
	  max_devices = new_max;
	}

      enum_devices[num_enum_devices].cookie =
	enum_devices[num_enum_devices - 1].cookie;
      if (grub_uboot_syscall (API_DEV_ENUM, NULL,
			      &enum_devices[num_enum_devices]) == 0)
	goto error;

      if (enum_devices[num_enum_devices].cookie == NULL)
	break;

      num_enum_devices++;
    }

  devices = enum_devices;
  return num_devices = num_enum_devices;

 error:
  grub_free (enum_devices);
  return 0;
}

#define VALID_DEV(x) (((x) < num_devices) && ((x) >= 0))
#define OPEN_DEV(x) ((x->state == DEV_STA_OPEN))

struct device_info *
grub_uboot_dev_get (int index)
{
  if (VALID_DEV (index))
    return &devices[index];

  return NULL;
}


int
grub_uboot_dev_open (struct device_info *dev)
{
  int retval;

  if (!grub_uboot_syscall (API_DEV_OPEN, &retval, dev))
    return -1;

  return retval;
}

int
grub_uboot_dev_close (struct device_info *dev)
{
  int retval;

  if (!grub_uboot_syscall (API_DEV_CLOSE, &retval, dev))
    return -1;

  return retval;
}


int
grub_uboot_dev_read (struct device_info *dev, void *buf, grub_size_t blocks,
		     grub_uint32_t start, grub_size_t * real_blocks)
{
  int retval;

  if (!OPEN_DEV (dev))
    return -1;

  if (!grub_uboot_syscall (API_DEV_READ, &retval, dev, buf,
			   &blocks, &start, real_blocks))
    return -1;

  return retval;
}

int
grub_uboot_dev_recv (struct device_info *dev, void *buf,
		     int size, int *real_size)
{
  int retval;

  if (!OPEN_DEV (dev))
    return -1;

  if (!grub_uboot_syscall (API_DEV_READ, &retval, dev, buf, &size, real_size))
    return -1;

  return retval;

}

int
grub_uboot_dev_send (struct device_info *dev, void *buf, int size)
{
  int retval;

  if (!OPEN_DEV (dev))
    return -1;

  if (!grub_uboot_syscall (API_DEV_WRITE, &retval, dev, buf, &size))
    return -1;

  return retval;
}

char *
grub_uboot_env_get (const char *name)
{
  char *value;

  if (!grub_uboot_syscall (API_ENV_GET, NULL, name, &value))
    return NULL;

  return value;
}

void
grub_uboot_env_set (const char *name, const char *value)
{
  grub_uboot_syscall (API_ENV_SET, NULL, name, value);
}
