/* Export pnvram and some variables for runtime */
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

#include <grub/file.h>
#include <grub/err.h>
#include <grub/normal.h>
#include <grub/mm.h>
#include <grub/misc.h>
#include <grub/charset.h>
#include <grub/efiemu/efiemu.h>
#include <grub/efiemu/runtime.h>
#include <grub/extcmd.h>

/* Place for final location of variables */
static int nvram_handle = 0;
static int nvramsize_handle = 0;
static int high_monotonic_count_handle = 0;
static int timezone_handle = 0;
static int accuracy_handle = 0;
static int daylight_handle = 0;

static grub_size_t nvramsize;

/* Parse signed value */
static int
grub_strtosl (const char *arg, char **end, int base)
{
  if (arg[0] == '-')
    return -grub_strtoul (arg + 1, end, base);
  return grub_strtoul (arg, end, base);
}

static inline int
hextoval (char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'z')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'Z')
    return c - 'A' + 10;
  return 0;
}

static inline grub_err_t
unescape (char *in, char *out, char *outmax, int *len)
{
  char *ptr, *dptr;
  dptr = out;
  for (ptr = in; *ptr && dptr < outmax; )
    if (*ptr == '%' && ptr[1] && ptr[2])
      {
	*dptr = (hextoval (ptr[1]) << 4) | (hextoval (ptr[2]));
	ptr += 3;
	dptr++;
      }
    else
      {
	*dptr = *ptr;
	ptr++;
	dptr++;
      }
  if (dptr == outmax)
    return grub_error (GRUB_ERR_OUT_OF_MEMORY,
		       "too many NVRAM variables for reserved variable space."
		       " Try increasing EfiEmu.pnvram.size");
  *len = dptr - out;
  return 0;
}

/* Export stuff for efiemu */
static grub_err_t
nvram_set (void * data __attribute__ ((unused)))
{
  const char *env;
  /* Take definitive pointers */
  char *nvram = grub_efiemu_mm_obtain_request (nvram_handle);
  grub_uint32_t *nvramsize_def
    = grub_efiemu_mm_obtain_request (nvramsize_handle);
  grub_uint32_t *high_monotonic_count
    = grub_efiemu_mm_obtain_request (high_monotonic_count_handle);
  grub_int16_t *timezone
    = grub_efiemu_mm_obtain_request (timezone_handle);
  grub_uint8_t *daylight
    = grub_efiemu_mm_obtain_request (daylight_handle);
  grub_uint32_t *accuracy
    = grub_efiemu_mm_obtain_request (accuracy_handle);
  char *nvramptr;
  struct grub_env_var *var;

  /* Copy to definitive loaction */
  grub_dprintf ("efiemu", "preparing pnvram\n");

  env = grub_env_get ("EfiEmu.pnvram.high_monotonic_count");
  *high_monotonic_count = env ? grub_strtoul (env, 0, 0) : 1;
  env = grub_env_get ("EfiEmu.pnvram.timezone");
  *timezone = env ? grub_strtosl (env, 0, 0) : GRUB_EFI_UNSPECIFIED_TIMEZONE;
  env = grub_env_get ("EfiEmu.pnvram.accuracy");
  *accuracy = env ? grub_strtoul (env, 0, 0) : 50000000;
  env = grub_env_get ("EfiEmu.pnvram.daylight");
  *daylight = env ? grub_strtoul (env, 0, 0) : 0;

  nvramptr = nvram;
  grub_memset (nvram, 0, nvramsize);
  FOR_SORTED_ENV (var)
  {
    char *guid, *attr, *name, *varname;
    struct efi_variable *efivar;
    int len = 0;
    int i;
    grub_uint64_t guidcomp;

    if (grub_memcmp (var->name, "EfiEmu.pnvram.",
		     sizeof ("EfiEmu.pnvram.") - 1) != 0)
      continue;

    guid = var->name + sizeof ("EfiEmu.pnvram.") - 1;

    attr = grub_strchr (guid, '.');
    if (!attr)
      continue;
    attr++;

    name = grub_strchr (attr, '.');
    if (!name)
      continue;
    name++;

    efivar = (struct efi_variable *) nvramptr;
    if (nvramptr - nvram + sizeof (struct efi_variable) > nvramsize)
      return grub_error (GRUB_ERR_OUT_OF_MEMORY,
			 "too many NVRAM variables for reserved variable space."
			 " Try increasing EfiEmu.pnvram.size");

    nvramptr += sizeof (struct efi_variable);

    efivar->guid.data1 = grub_cpu_to_le32 (grub_strtoul (guid, &guid, 16));
    if (*guid != '-')
      continue;
    guid++;

    efivar->guid.data2 = grub_cpu_to_le16 (grub_strtoul (guid, &guid, 16));
    if (*guid != '-')
      continue;
    guid++;

    efivar->guid.data3 = grub_cpu_to_le16 (grub_strtoul (guid, &guid, 16));
    if (*guid != '-')
      continue;
    guid++;

    guidcomp = grub_strtoull (guid, 0, 16);
    for (i = 0; i < 8; i++)
      efivar->guid.data4[i] = (guidcomp >> (56 - 8 * i)) & 0xff;

    efivar->attributes = grub_strtoull (attr, 0, 16);

    varname = grub_malloc (grub_strlen (name) + 1);
    if (! varname)
      return grub_errno;

    if (unescape (name, varname, varname + grub_strlen (name) + 1, &len))
      break;

    len = grub_utf8_to_utf16 ((grub_uint16_t *) nvramptr,
			      (nvramsize - (nvramptr - nvram)) / 2,
			      (grub_uint8_t *) varname, len, NULL);

    nvramptr += 2 * len;
    *((grub_uint16_t *) nvramptr) = 0;
    nvramptr += 2;
    efivar->namelen = 2 * len + 2;

    if (unescape (var->value, nvramptr, nvram + nvramsize, &len))
      {
	efivar->namelen = 0;
	break;
      }

    nvramptr += len;

    efivar->size = len;
  }
  if (grub_errno)
    return grub_errno;

  *nvramsize_def = nvramsize;

  /* Register symbols */
  grub_efiemu_register_symbol ("efiemu_variables", nvram_handle, 0);
  grub_efiemu_register_symbol ("efiemu_varsize", nvramsize_handle, 0);
  grub_efiemu_register_symbol ("efiemu_high_monotonic_count",
			       high_monotonic_count_handle, 0);
  grub_efiemu_register_symbol ("efiemu_time_zone", timezone_handle, 0);
  grub_efiemu_register_symbol ("efiemu_time_daylight", daylight_handle, 0);
  grub_efiemu_register_symbol ("efiemu_time_accuracy",
			       accuracy_handle, 0);

  return GRUB_ERR_NONE;
}

static void
nvram_unload (void * data __attribute__ ((unused)))
{
  grub_efiemu_mm_return_request (nvram_handle);
  grub_efiemu_mm_return_request (nvramsize_handle);
  grub_efiemu_mm_return_request (high_monotonic_count_handle);
  grub_efiemu_mm_return_request (timezone_handle);
  grub_efiemu_mm_return_request (accuracy_handle);
  grub_efiemu_mm_return_request (daylight_handle);
}

grub_err_t
grub_efiemu_pnvram (void)
{
  const char *size;
  grub_err_t err;

  nvramsize = 0;

  size = grub_env_get ("EfiEmu.pnvram.size");
  if (size)
    nvramsize = grub_strtoul (size, 0, 0);

  if (!nvramsize)
    nvramsize = 2048;

  err = grub_efiemu_register_prepare_hook (nvram_set, nvram_unload, 0);
  if (err)
    return err;

  nvram_handle
    = grub_efiemu_request_memalign (1, nvramsize,
				    GRUB_EFI_RUNTIME_SERVICES_DATA);
  nvramsize_handle
    = grub_efiemu_request_memalign (1, sizeof (grub_uint32_t),
				    GRUB_EFI_RUNTIME_SERVICES_DATA);
  high_monotonic_count_handle
    = grub_efiemu_request_memalign (1, sizeof (grub_uint32_t),
				    GRUB_EFI_RUNTIME_SERVICES_DATA);
  timezone_handle
    = grub_efiemu_request_memalign (1, sizeof (grub_uint16_t),
				    GRUB_EFI_RUNTIME_SERVICES_DATA);
  daylight_handle
    = grub_efiemu_request_memalign (1, sizeof (grub_uint8_t),
				    GRUB_EFI_RUNTIME_SERVICES_DATA);
  accuracy_handle
    = grub_efiemu_request_memalign (1, sizeof (grub_uint32_t),
				    GRUB_EFI_RUNTIME_SERVICES_DATA);

  return GRUB_ERR_NONE;
}
