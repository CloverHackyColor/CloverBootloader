/*  openfw.c -- Open firmware support functions.  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2003,2004,2005,2007,2008,2009 Free Software Foundation, Inc.
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
#include <grub/err.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/ieee1275/ieee1275.h>
#include <grub/net.h>

enum grub_ieee1275_parse_type
{
  GRUB_PARSE_FILENAME,
  GRUB_PARSE_PARTITION,
  GRUB_PARSE_DEVICE,
  GRUB_PARSE_DEVICE_TYPE
};

static int
fill_alias (struct grub_ieee1275_devalias *alias)
{
  grub_ssize_t actual;

  if (grub_ieee1275_get_property (alias->phandle, "device_type", alias->type,
				  IEEE1275_MAX_PROP_LEN, &actual))
    alias->type[0] = 0;

  if (alias->parent_dev == alias->phandle)
    return 0;

  if (grub_ieee1275_package_to_path (alias->phandle, alias->path,
				     IEEE1275_MAX_PATH_LEN, &actual))
    return 0;

  if (grub_strcmp (alias->parent_path, alias->path) == 0)
    return 0;

  if (grub_ieee1275_get_property (alias->phandle, "name", alias->name,
				  IEEE1275_MAX_PROP_LEN, &actual))
    return 0;
  grub_dprintf ("devalias", "device path=%s\n", alias->path);
  return 1;
}

void
grub_ieee1275_devalias_free (struct grub_ieee1275_devalias *alias)
{
  grub_free (alias->name);
  grub_free (alias->type);
  grub_free (alias->path);
  grub_free (alias->parent_path);
  alias->name = 0;
  alias->type = 0;
  alias->path = 0;
  alias->parent_path = 0;
  alias->phandle = GRUB_IEEE1275_PHANDLE_INVALID;
}

void
grub_ieee1275_children_peer (struct grub_ieee1275_devalias *alias)
{
  while (grub_ieee1275_peer (alias->phandle, &alias->phandle) != -1)
    if (fill_alias (alias))
      return;
  grub_ieee1275_devalias_free (alias);
}

void
grub_ieee1275_children_first (const char *devpath,
			      struct grub_ieee1275_devalias *alias)
{
  grub_ieee1275_phandle_t dev;

  grub_dprintf ("devalias", "iterating children of %s\n",
		devpath);

  alias->name = 0;
  alias->path = 0;
  alias->parent_path = 0;
  alias->type = 0;

  if (grub_ieee1275_finddevice (devpath, &dev))
    return;

  if (grub_ieee1275_child (dev, &alias->phandle))
    return;

  alias->type = grub_malloc (IEEE1275_MAX_PROP_LEN);
  if (!alias->type)
    return;
  alias->path = grub_malloc (IEEE1275_MAX_PATH_LEN);
  if (!alias->path)
    {
      grub_free (alias->type);
      return;
    }
  alias->parent_path = grub_strdup (devpath);
  if (!alias->parent_path)
    {
      grub_free (alias->path);
      grub_free (alias->type);
      return;
    }

  alias->name = grub_malloc (IEEE1275_MAX_PROP_LEN);
  if (!alias->name)
    {
      grub_free (alias->path);
      grub_free (alias->type);
      grub_free (alias->parent_path);
      return;
    }
  if (!fill_alias (alias))
    grub_ieee1275_children_peer (alias);
}

static int
iterate_recursively (const char *path,
		     int (*hook) (struct grub_ieee1275_devalias *alias))
{
  struct grub_ieee1275_devalias alias;
  int ret = 0;

  FOR_IEEE1275_DEVCHILDREN(path, alias)
    {
      ret = hook (&alias);
      if (ret)
	break;
      ret = iterate_recursively (alias.path, hook);
      if (ret)
	break;
    }
  grub_ieee1275_devalias_free (&alias);
  return ret;
}

int
grub_ieee1275_devices_iterate (int (*hook) (struct grub_ieee1275_devalias *alias))
{
  return iterate_recursively ("/", hook);
}

void
grub_ieee1275_devalias_init_iterator (struct grub_ieee1275_devalias *alias)
{
  alias->name = 0;
  alias->path = 0;
  alias->parent_path = 0;
  alias->type = 0;

  grub_dprintf ("devalias", "iterating aliases\n");

  if (grub_ieee1275_finddevice ("/aliases", &alias->parent_dev))
    return;

  alias->name = grub_malloc (IEEE1275_MAX_PROP_LEN);
  if (!alias->name)
    return;

  alias->type = grub_malloc (IEEE1275_MAX_PROP_LEN);
  if (!alias->type)
    {
      grub_free (alias->name);
      alias->name = 0;
      return;
    }

  alias->name[0] = '\0';
}

int
grub_ieee1275_devalias_next (struct grub_ieee1275_devalias *alias)
{
  if (!alias->name)
    return 0;
  while (1)
    {
      grub_ssize_t pathlen;
      grub_ssize_t actual;
      char *tmp;

      if (alias->path)
	{
	  grub_free (alias->path);
	  alias->path = 0;
	}
      tmp = grub_strdup (alias->name);
      if (grub_ieee1275_next_property (alias->parent_dev, tmp,
				       alias->name) <= 0)
	{
	  grub_free (tmp);
	  grub_ieee1275_devalias_free (alias);
	  return 0;
	}
      grub_free (tmp);

      grub_dprintf ("devalias", "devalias name = %s\n", alias->name);

      grub_ieee1275_get_property_length (alias->parent_dev, alias->name, &pathlen);

      /* The property `name' is a special case we should skip.  */
      if (grub_strcmp (alias->name, "name") == 0)
	continue;

      /* Sun's OpenBoot often doesn't zero terminate the device alias
	 strings, so we will add a NULL byte at the end explicitly.  */
      pathlen += 1;

      alias->path = grub_malloc (pathlen + 1);
      if (! alias->path)
	{
	  grub_ieee1275_devalias_free (alias);
	  return 0;
	}

      if (grub_ieee1275_get_property (alias->parent_dev, alias->name, alias->path,
				      pathlen, &actual) || actual < 0)
	{
	  grub_dprintf ("devalias", "get_property (%s) failed\n", alias->name);
	  grub_free (alias->path);
	  continue;
	}
      if (actual > pathlen)
	actual = pathlen;
      alias->path[actual] = '\0';
      alias->path[pathlen] = '\0';

      if (grub_ieee1275_finddevice (alias->path, &alias->phandle))
	{
	  grub_dprintf ("devalias", "finddevice (%s) failed\n", alias->path);
	  grub_free (alias->path);
	  alias->path = 0;
	  continue;
	}

      if (grub_ieee1275_get_property (alias->phandle, "device_type", alias->type,
				      IEEE1275_MAX_PROP_LEN, &actual))
	{
	  /* NAND device don't have device_type property.  */
          alias->type[0] = 0;
	}
      return 1;
    }
}

/* Call the "map" method of /chosen/mmu.  */
int
grub_ieee1275_map (grub_addr_t phys, grub_addr_t virt, grub_size_t size,
		   grub_uint32_t mode)
{
  struct map_args {
    struct grub_ieee1275_common_hdr common;
    grub_ieee1275_cell_t method;
    grub_ieee1275_cell_t ihandle;
    grub_ieee1275_cell_t mode;
    grub_ieee1275_cell_t size;
    grub_ieee1275_cell_t virt;
#ifdef __sparc__
    grub_ieee1275_cell_t phys_high;
#endif
    grub_ieee1275_cell_t phys_low;
    grub_ieee1275_cell_t catch_result;
  } args;

  INIT_IEEE1275_COMMON (&args.common, "call-method",
#ifdef __sparc__
			7,
#else
			6,
#endif
			1);
  args.method = (grub_ieee1275_cell_t) "map";
  args.ihandle = grub_ieee1275_mmu;
#ifdef __sparc__
  args.phys_high = 0;
#endif
  args.phys_low = phys;
  args.virt = virt;
  args.size = size;
  args.mode = mode; /* Format is WIMG0PP.  */
  args.catch_result = (grub_ieee1275_cell_t) -1;

  if (IEEE1275_CALL_ENTRY_FN (&args) == -1)
    return -1;

  return args.catch_result;
}

grub_err_t
grub_claimmap (grub_addr_t addr, grub_size_t size)
{
  if (grub_ieee1275_claim (addr, size, 0, 0))
    return -1;

  if (! grub_ieee1275_test_flag (GRUB_IEEE1275_FLAG_REAL_MODE)
      && grub_ieee1275_map (addr, addr, size, 0x00))
    {
      grub_error (GRUB_ERR_OUT_OF_MEMORY, "map failed: address 0x%llx, size 0x%llx\n",
		  (long long) addr, (long long) size);
      grub_ieee1275_release (addr, size);
      return grub_errno;
    }

  return GRUB_ERR_NONE;
}

/* Get the device arguments of the Open Firmware node name `path'.  */
static char *
grub_ieee1275_get_devargs (const char *path)
{
  char *colon = grub_strchr (path, ':');

  if (! colon)
    return 0;

  return grub_strdup (colon + 1);
}

/* Get the device path of the Open Firmware node name `path'.  */
char *
grub_ieee1275_get_devname (const char *path)
{
  char *colon = grub_strchr (path, ':');
  int pathlen = grub_strlen (path);
  struct grub_ieee1275_devalias curalias;
  if (colon)
    pathlen = (int)(colon - path);

  /* Try to find an alias for this device.  */
  FOR_IEEE1275_DEVALIASES (curalias)
    /* briQ firmware can change capitalization in /chosen/bootpath.  */
    if (grub_strncasecmp (curalias.path, path, pathlen) == 0
	&& curalias.path[pathlen] == 0)
      {
	char *newpath;
	newpath = grub_strdup (curalias.name);
	grub_ieee1275_devalias_free (&curalias);
	return newpath;
      }

  return grub_strndup (path, pathlen);
}

static char *
grub_ieee1275_parse_args (const char *path, enum grub_ieee1275_parse_type ptype)
{
  char type[64]; /* XXX check size.  */
  char *device = grub_ieee1275_get_devname (path);
  char *ret = 0;
  grub_ieee1275_phandle_t dev;

  /* We need to know what type of device it is in order to parse the full
     file path properly.  */
  if (grub_ieee1275_finddevice (device, &dev))
    {
      grub_error (GRUB_ERR_UNKNOWN_DEVICE, "device %s not found", device);
      goto fail;
    }
  if (grub_ieee1275_get_property (dev, "device_type", &type, sizeof type, 0))
    {
      grub_error (GRUB_ERR_UNKNOWN_DEVICE,
		  "device %s lacks a device_type property", device);
      goto fail;
    }

  switch (ptype)
    {
    case GRUB_PARSE_DEVICE:
      ret = grub_strdup (device);
      break;
    case GRUB_PARSE_DEVICE_TYPE:
      ret = grub_strdup (type);
      break;
    case GRUB_PARSE_FILENAME:
      {
	char *comma;
	char *args;

	args = grub_ieee1275_get_devargs (path);
	if (!args)
	  /* Shouldn't happen.  */
	  return 0;

	/* The syntax of the device arguments is defined in the CHRP and PReP
	   IEEE1275 bindings: "[partition][,[filename]]".  */
	comma = grub_strchr (args, ',');

	if (comma)
	  {
	    char *filepath = comma + 1;
	    
	    /* Make sure filepath has leading backslash.  */
	    if (filepath[0] != '\\')
	      ret = grub_xasprintf ("\\%s", filepath);
	    else
	      ret = grub_strdup (filepath);
	    }
	grub_free (args);
	}
      break;
    case GRUB_PARSE_PARTITION:
      {
	char *comma;
	char *args;

	if (grub_strcmp ("block", type) != 0)
	  goto unknown;

	args = grub_ieee1275_get_devargs (path);
	if (!args)
	  /* Shouldn't happen.  */
	  return 0;

	comma = grub_strchr (args, ',');
	if (!comma)
	  ret = grub_strdup (args);
	else
	  ret = grub_strndup (args, (grub_size_t)(comma - args));
	/* Consistently provide numbered partitions to GRUB.
	   OpenBOOT traditionally uses alphabetical partition
	   specifiers.  */
	if (ret[0] >= 'a' && ret[0] <= 'z')
	    ret[0] = '1' + (ret[0] - 'a');
	grub_free (args);
      }
      break;
    default:
    unknown:
      grub_error (GRUB_ERR_NOT_IMPLEMENTED_YET,
		  "unsupported type %s for device %s", type, device);
    }

fail:
  grub_free (device);
  return ret;
}

char *
grub_ieee1275_get_device_type (const char *path)
{
  return grub_ieee1275_parse_args (path, GRUB_PARSE_DEVICE_TYPE);
}

char *
grub_ieee1275_get_aliasdevname (const char *path)
{
  return grub_ieee1275_parse_args (path, GRUB_PARSE_DEVICE);
}

char *
grub_ieee1275_get_filename (const char *path)
{
  return grub_ieee1275_parse_args (path, GRUB_PARSE_FILENAME);
}

/* Convert a device name from IEEE1275 syntax to GRUB syntax.  */
char *
grub_ieee1275_encode_devname (const char *path)
{
  char *device = grub_ieee1275_get_devname (path);
  char *partition;
  char *encoding;
  char *optr;
  const char *iptr;

  encoding = grub_malloc (sizeof ("ieee1275/") + 2 * grub_strlen (device)
			  + sizeof (",XXXXXXXXXXXX"));
  if (!encoding)
    {
      grub_free (device);
      return 0;
    }

  partition = grub_ieee1275_parse_args (path, GRUB_PARSE_PARTITION);

  optr = grub_stpcpy (encoding, "ieee1275/");
  for (iptr = device; *iptr; )
    {
      if (*iptr == ',')
	*optr++ ='\\';
      *optr++ = *iptr++;
    }
  if (partition && partition[0])
    {
      unsigned int partno = grub_strtoul (partition, 0, 0);

      *optr++ = ',';

      if (grub_ieee1275_test_flag (GRUB_IEEE1275_FLAG_0_BASED_PARTITIONS))
	/* GRUB partition 1 is OF partition 0.  */
	partno++;

      grub_snprintf (optr, sizeof ("XXXXXXXXXXXX"), "%d", partno);
    }
  else
    *optr = '\0';

  grub_free (partition);
  grub_free (device);

  return encoding;
}

/* Resolve aliases.  */
char *
grub_ieee1275_canonicalise_devname (const char *path)
{
  struct canon_args
  {
    struct grub_ieee1275_common_hdr common;
    grub_ieee1275_cell_t path;
    grub_ieee1275_cell_t buf;
    grub_ieee1275_cell_t inlen;
    grub_ieee1275_cell_t outlen;
  }
  args;
  char *buf = NULL;
  grub_size_t bufsize = 64;
  int i;

  for (i = 0; i < 2; i++)
    {
      grub_free (buf);

      buf = grub_malloc (bufsize);
      if (!buf)
	return NULL;

      INIT_IEEE1275_COMMON (&args.common, "canon", 3, 1);
      args.path = (grub_ieee1275_cell_t) path;
      args.buf = (grub_ieee1275_cell_t) buf;
      args.inlen = (grub_ieee1275_cell_t) (bufsize - 1);

      if (IEEE1275_CALL_ENTRY_FN (&args) == -1)
	return 0;
      if (args.outlen > bufsize - 1)
	{
	  bufsize = args.outlen + 2;
	  continue;
	}
      return buf;
    }
  /* Shouldn't reach here.  */
  grub_free (buf);
  return NULL;
}

