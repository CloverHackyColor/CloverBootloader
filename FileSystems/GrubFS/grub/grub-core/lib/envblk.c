/* envblk.c - Common functions for environment block.  */
/*
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

#include <config.h>
#include <grub/types.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/lib/envblk.h>

grub_envblk_t
grub_envblk_open (char *buf, grub_size_t size)
{
  grub_envblk_t envblk;

  if (size < sizeof (GRUB_ENVBLK_SIGNATURE)
      || grub_memcmp (buf, GRUB_ENVBLK_SIGNATURE,
                      sizeof (GRUB_ENVBLK_SIGNATURE) - 1))
    {
      grub_error (GRUB_ERR_BAD_FILE_TYPE, "invalid environment block");
      return 0;
    }

  envblk = grub_malloc (sizeof (*envblk));
  if (envblk)
    {
      envblk->buf = buf;
      envblk->size = size;
    }

  return envblk;
}

void
grub_envblk_close (grub_envblk_t envblk)
{
  grub_free (envblk->buf);
  grub_free (envblk);
}

static int
escaped_value_len (const char *value)
{
  int n = 0;
  char *p;

  for (p = (char *) value; *p; p++)
    {
      if (*p == '\\' || *p == '\n')
        n += 2;
      else
        n++;
    }

  return n;
}

static char *
find_next_line (char *p, const char *pend)
{
  while (p < pend)
    {
      if (*p == '\\')
        p += 2;
      else if (*p == '\n')
        break;
      else
        p++;
    }

  return p + 1;
}

int
grub_envblk_set (grub_envblk_t envblk, const char *name, const char *value)
{
  char *p, *pend;
  char *space;
  int found = 0;
  int nl;
  int vl;
  int i;

  nl = grub_strlen (name);
  vl = escaped_value_len (value);
  p = envblk->buf + sizeof (GRUB_ENVBLK_SIGNATURE) - 1;
  pend = envblk->buf + envblk->size;

  /* First, look at free space.  */
  for (space = pend - 1; *space == '#'; space--)
    ;

  if (*space != '\n')
    /* Broken.  */
    return 0;

  space++;

  while (p + nl + 1 < space)
    {
      if (grub_memcmp (p, name, nl) == 0 && p[nl] == '=')
        {
          int len;

          /* Found the same name.  */
          p += nl + 1;

          /* Check the length of the current value.  */
          len = 0;
          while (p + len < pend && p[len] != '\n')
            {
              if (p[len] == '\\')
                len += 2;
              else
                len++;
            }

          if (p + len >= pend)
            /* Broken.  */
            return 0;

          if (pend - space < vl - len)
            /* No space.  */
            return 0;

          if (vl < len)
            {
              /* Move the following characters backward, and fill the new
                 space with harmless characters.  */
              grub_memmove (p + vl, p + len, pend - (p + len));
              grub_memset (space + len - vl, '#', len - vl);
            }
          else
            /* Move the following characters forward.  */
            grub_memmove (p + vl, p + len, pend - (p + vl));

          found = 1;
          break;
        }

      p = find_next_line (p, pend);
    }

  if (! found)
    {
      /* Append a new variable.  */

      if (pend - space < nl + 1 + vl + 1)
        /* No space.  */
        return 0;

      grub_memcpy (space, name, nl);
      p = space + nl;
      *p++ = '=';
    }

  /* Write the value.  */
  for (i = 0; value[i]; i++)
    {
      if (value[i] == '\\' || value[i] == '\n')
        *p++ = '\\';

      *p++ = value[i];
    }

  *p = '\n';
  return 1;
}

void
grub_envblk_delete (grub_envblk_t envblk, const char *name)
{
  char *p, *pend;
  int nl;

  nl = grub_strlen (name);
  p = envblk->buf + sizeof (GRUB_ENVBLK_SIGNATURE) - 1;
  pend = envblk->buf + envblk->size;

  while (p + nl + 1 < pend)
    {
      if (grub_memcmp (p, name, nl) == 0 && p[nl] == '=')
        {
          /* Found.  */
          int len = nl + 1;

          while (p + len < pend)
            {
              if (p[len] == '\n')
                break;
              else if (p[len] == '\\')
                len += 2;
              else
                len++;
            }

          if (p + len >= pend)
            /* Broken.  */
            return;

          len++;
          grub_memmove (p, p + len, pend - (p + len));
          grub_memset (pend - len, '#', len);
          break;
        }

      p = find_next_line (p, pend);
    }
}

void
grub_envblk_iterate (grub_envblk_t envblk,
                     void *hook_data,
                     int hook (const char *name, const char *value, void *hook_data))
{
  char *p, *pend;

  p = envblk->buf + sizeof (GRUB_ENVBLK_SIGNATURE) - 1;
  pend = envblk->buf + envblk->size;

  while (p < pend)
    {
      if (*p != '#')
        {
          char *name;
          char *value;
          char *name_start, *name_end, *value_start;
          char *q;
          int ret;

          name_start = p;
          while (p < pend && *p != '=')
            p++;
          if (p == pend)
            /* Broken.  */
            return;
          name_end = p;

          p++;
          value_start = p;
          while (p < pend)
            {
              if (*p == '\n')
                break;
              else if (*p == '\\')
                p += 2;
              else
                p++;
            }

          if (p >= pend)
            /* Broken.  */
            return;

          name = grub_malloc (p - name_start + 1);
          if (! name)
            /* out of memory.  */
            return;

          value = name + (value_start - name_start);

          grub_memcpy (name, name_start, name_end - name_start);
          name[name_end - name_start] = '\0';

          for (p = value_start, q = value; *p != '\n'; ++p)
            {
              if (*p == '\\')
                *q++ = *++p;
              else
                *q++ = *p;
            }
          *q = '\0';

          ret = hook (name, value, hook_data);
          grub_free (name);
          if (ret)
            return;
        }

      p = find_next_line (p, pend);
    }
}
