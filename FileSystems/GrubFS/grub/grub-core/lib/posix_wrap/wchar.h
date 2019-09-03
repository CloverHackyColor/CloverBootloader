/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2009, 2010  Free Software Foundation, Inc.
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

#ifndef GRUB_POSIX_WCHAR_H
#define GRUB_POSIX_WCHAR_H	1

#include <grub/charset.h>

#define wint_t grub_posix_wint_t
#define wchar_t grub_posix_wchar_t
#define mbstate_t grub_posix_mbstate_t

/* UCS-4.  */
typedef grub_int32_t wint_t;
enum
  {
    WEOF = -1
  };

/* UCS-4.  */
typedef grub_int32_t wchar_t;

typedef struct mbstate {
  grub_uint32_t code;
  int count;
} mbstate_t;

/* UTF-8. */
#define MB_CUR_MAX 4
#define MB_LEN_MAX 4

static inline size_t
mbrtowc (wchar_t *pwc, const char *s, size_t n, mbstate_t *ps)
{
  const char *ptr;
  if (!s)
    {
      pwc = 0;
      s = "";
      n = 1;
    }

  if (pwc)
    *pwc = 0;

  for (ptr = s; ptr < s + n; ptr++)
    {
      if (!grub_utf8_process (*ptr, &ps->code, &ps->count))
	return -1;
      if (ps->count)
	continue;
      if (pwc)
	*pwc = ps->code;
      if (ps->code == 0)
	return 0;
      return ptr - s + 1;
    }
  return -2;
}

static inline int
mbsinit(const mbstate_t *ps)
{
  return ps->count == 0;
}

static inline size_t
wcrtomb (char *s, wchar_t wc, mbstate_t *ps __attribute__ ((unused)))
{
  if (s == 0)
    return 1;
  return grub_encode_utf8_character ((grub_uint8_t *) s,
				     (grub_uint8_t *) s + MB_LEN_MAX,
				     wc);
}

static inline wint_t btowc (int c)
{
  if (c & ~0x7f)
    return WEOF;
  return c;
}


static inline int
wcscoll (const wchar_t *s1, const wchar_t *s2)
{
  while (*s1 && *s2)
    {
      if (*s1 != *s2)
	break;

      s1++;
      s2++;
    }

  if (*s1 < *s2)
    return -1;
  if (*s1 > *s2)
    return +1;
  return 0;
}

#endif
