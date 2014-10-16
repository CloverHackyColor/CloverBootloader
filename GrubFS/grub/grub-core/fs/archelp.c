/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2007,2008,2009,2013 Free Software Foundation, Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <grub/archelp.h>
#include <grub/err.h>
#include <grub/fs.h>
#include <grub/disk.h>
#include <grub/dl.h>

GRUB_MOD_LICENSE ("GPLv3+");

static inline void
canonicalize (char *name)
{
  char *iptr, *optr;
  for (iptr = name, optr = name; *iptr; )
    {
      while (*iptr == '/')
	iptr++;
      if (iptr[0] == '.' && (iptr[1] == '/' || iptr[1] == 0))
	{
	  iptr += 2;
	  continue;
	}
      if (iptr[0] == '.' && iptr[1] == '.' && (iptr[2] == '/' || iptr[2] == 0))
	{
	  iptr += 3;
	  if (optr == name)
	    continue;
	  for (optr -= 2; optr >= name && *optr != '/'; optr--);
	  optr++;
	  continue;
	}
      while (*iptr && *iptr != '/')
	*optr++ = *iptr++;
      if (*iptr)
	*optr++ = *iptr++;
    }
  *optr = 0;
}

static grub_err_t
handle_symlink (struct grub_archelp_data *data,
		struct grub_archelp_ops *arcops,
		const char *fn, char **name,
		grub_uint32_t mode, int *restart)
{
  grub_size_t flen;
  char *target;
  char *ptr;
  char *lastslash;
  grub_size_t prefixlen;
  char *rest;
  char *linktarget;
  grub_size_t linktarget_len;

  *restart = 0;

  if ((mode & GRUB_ARCHELP_ATTR_TYPE) != GRUB_ARCHELP_ATTR_LNK
      || !arcops->get_link_target)
    return GRUB_ERR_NONE;
  flen = grub_strlen (fn);
  if (grub_memcmp (*name, fn, flen) != 0 
      || ((*name)[flen] != 0 && (*name)[flen] != '/'))
    return GRUB_ERR_NONE;
  rest = *name + flen;
  lastslash = rest;
  if (*rest)
    rest++;
  while (lastslash >= *name && *lastslash != '/')
    lastslash--;
  if (lastslash >= *name)
    prefixlen = lastslash - *name;
  else
    prefixlen = 0;

  if (prefixlen)
    prefixlen++;

  linktarget = arcops->get_link_target (data);
  if (!linktarget)
    return grub_errno;
  if (linktarget[0] == '\0')
    return GRUB_ERR_NONE;
  linktarget_len = grub_strlen (linktarget);
  target = grub_malloc (linktarget_len + grub_strlen (*name) + 2);
  if (!target)
    return grub_errno;

  grub_strcpy (target + prefixlen, linktarget);
  grub_free (linktarget);
  if (target[prefixlen] == '/')
    {
      ptr = grub_stpcpy (target, target + prefixlen);
      ptr = grub_stpcpy (ptr, rest);
      *ptr = 0;
      grub_dprintf ("archelp", "symlink redirected %s to %s\n",
		    *name, target);
      grub_free (*name);

      canonicalize (target);
      *name = target;
      *restart = 1;
      return GRUB_ERR_NONE;
    }
  if (prefixlen)
    {
      grub_memcpy (target, *name, prefixlen);
      target[prefixlen-1] = '/';
    }
  grub_strcpy (target + prefixlen + linktarget_len, rest);
  grub_dprintf ("archelp", "symlink redirected %s to %s\n",
		*name, target);
  grub_free (*name);
  canonicalize (target);
  *name = target;
  *restart = 1;
  return GRUB_ERR_NONE;
}

grub_err_t
grub_archelp_dir (struct grub_archelp_data *data,
		  struct grub_archelp_ops *arcops,
		  const char *path_in,
		  grub_fs_dir_hook_t hook, void *hook_data)
{
  char *prev, *name, *path, *ptr;
  grub_size_t len;
  int symlinknest = 0;

  path = grub_strdup (path_in + 1);
  if (!path)
    return grub_errno;
  canonicalize (path);
  for (ptr = path + grub_strlen (path) - 1; ptr >= path && *ptr == '/'; ptr--)
    *ptr = 0;

  prev = 0;

  len = grub_strlen (path);
  while (1)
    {
      grub_int32_t mtime;
      grub_uint32_t mode;
      grub_err_t err;

      if (arcops->find_file (data, &name, &mtime, &mode))
	goto fail;

      if (mode == GRUB_ARCHELP_ATTR_END)
	break;

      canonicalize (name);

      if (grub_memcmp (path, name, len) == 0
	  && (name[len] == 0 || name[len] == '/' || len == 0))
	{
	  char *p, *n;

	  n = name + len;
	  while (*n == '/')
	    n++;

	  p = grub_strchr (n, '/');
	  if (p)
	    *p = 0;

	  if (((!prev) || (grub_strcmp (prev, name) != 0)) && *n != 0)
	    {
	      struct grub_dirhook_info info;
	      grub_memset (&info, 0, sizeof (info));
	      info.dir = (p != NULL) || ((mode & GRUB_ARCHELP_ATTR_TYPE)
					 == GRUB_ARCHELP_ATTR_DIR);
	      if (!(mode & GRUB_ARCHELP_ATTR_NOTIME))
		{
		  info.mtime = mtime;
		  info.mtimeset = 1;
		}
	      if (hook (n, &info, hook_data))
		{
		  grub_free (name);
		  goto fail;
		}
	      grub_free (prev);
	      prev = name;
	    }
	  else
	    {
	      int restart = 0;
	      err = handle_symlink (data, arcops, name,
				    &path, mode, &restart);
	      grub_free (name);
	      if (err)
		goto fail;
	      if (restart)
		{
		  len = grub_strlen (path);
		  if (++symlinknest == 8)
		    {
		      grub_error (GRUB_ERR_SYMLINK_LOOP,
				  N_("too deep nesting of symlinks"));
		      goto fail;
		    }
		  arcops->rewind (data);
		}
	    }
	}
      else
	grub_free (name);
    }

fail:

  grub_free (path);
  grub_free (prev);

  return grub_errno;
}

grub_err_t
grub_archelp_open (struct grub_archelp_data *data,
		   struct grub_archelp_ops *arcops,
		   const char *name_in)
{
  char *fn;
  char *name = grub_strdup (name_in + 1);
  int symlinknest = 0;

  if (!name)
    return grub_errno;

  canonicalize (name);

  while (1)
    {
      grub_uint32_t mode;
      int restart;
      
      if (arcops->find_file (data, &fn, NULL, &mode))
	goto fail;

      if (mode == GRUB_ARCHELP_ATTR_END)
	{
	  grub_error (GRUB_ERR_FILE_NOT_FOUND, N_("file `%s' not found"), name_in);
	  break;
	}

      canonicalize (fn);

      if (handle_symlink (data, arcops, fn, &name, mode, &restart))
	{
	  grub_free (fn);
	  goto fail;
	}

      if (restart)
	{
	  arcops->rewind (data);
	  if (++symlinknest == 8)
	    {
	      grub_error (GRUB_ERR_SYMLINK_LOOP,
			  N_("too deep nesting of symlinks"));
	      goto fail;
	    }
	  goto no_match;
	}

      if (grub_strcmp (name, fn) != 0)
	goto no_match;

      grub_free (fn);
      grub_free (name);

      return GRUB_ERR_NONE;

    no_match:

      grub_free (fn);
    }

fail:
  grub_free (name);

  return grub_errno;
}
