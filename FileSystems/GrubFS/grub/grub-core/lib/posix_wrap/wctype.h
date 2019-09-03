/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2009, 2010, 2011  Free Software Foundation, Inc.
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

#ifndef GRUB_POSIX_WCTYPE_H
#define GRUB_POSIX_WCTYPE_H	1

#include <grub/misc.h>
#include <wchar.h>

#define wctype_t grub_posix_wctype_t

typedef enum { GRUB_CTYPE_INVALID,
	       GRUB_CTYPE_ALNUM, GRUB_CTYPE_CNTRL, GRUB_CTYPE_LOWER,
	       GRUB_CTYPE_SPACE, GRUB_CTYPE_ALPHA, GRUB_CTYPE_DIGIT,
	       GRUB_CTYPE_PRINT, GRUB_CTYPE_UPPER, GRUB_CTYPE_BLANK,
	       GRUB_CTYPE_GRAPH, GRUB_CTYPE_PUNCT, GRUB_CTYPE_XDIGIT,
	       GRUB_CTYPE_MAX} wctype_t;

#define CHARCLASS_NAME_MAX (sizeof ("xdigit") - 1)

static inline wctype_t
wctype (const char *name)
{
  wctype_t i;
  static const char names[][10] = { "", 
				    "alnum", "cntrl", "lower",
				    "space", "alpha", "digit",
				    "print", "upper", "blank",
				    "graph", "punct", "xdigit" };
  for (i = GRUB_CTYPE_INVALID; i < GRUB_CTYPE_MAX; i++)
    if (grub_strcmp (names[i], name) == 0)
      return i;
  return GRUB_CTYPE_INVALID;
}

/* FIXME: take into account international lowercase characters.  */
static inline int
iswlower (wint_t wc)
{
  return grub_islower (wc);
}

static inline wint_t
towlower (wint_t c)
{
  return grub_tolower (c);
}

static inline wint_t
towupper (wint_t c)
{
  return grub_toupper (c);
}

static inline int
iswalnum (wint_t c)
{
  return grub_isalpha (c) || grub_isdigit (c);
}

static inline int
iswctype (wint_t wc, wctype_t desc)
{
  switch (desc)
    {
    case GRUB_CTYPE_ALNUM:
      return iswalnum (wc);
    case GRUB_CTYPE_CNTRL:
      return grub_iscntrl (wc);
    case GRUB_CTYPE_LOWER:
      return iswlower (wc);
    case GRUB_CTYPE_SPACE:
      return grub_isspace (wc);
    case GRUB_CTYPE_ALPHA:
      return grub_isalpha (wc);
    case GRUB_CTYPE_DIGIT:
      return grub_isdigit (wc);
    case GRUB_CTYPE_PRINT:
      return grub_isprint (wc);
    case GRUB_CTYPE_UPPER:
      return grub_isupper (wc);
    case GRUB_CTYPE_BLANK:
      return wc == ' ' || wc == '\t';
    case GRUB_CTYPE_GRAPH:
      return grub_isgraph (wc);
    case GRUB_CTYPE_PUNCT:
      return grub_isprint (wc) && !grub_isspace (wc) && !iswalnum (wc);
    case GRUB_CTYPE_XDIGIT:
      return grub_isxdigit (wc);
    default:
      return 0;
    }
}

#endif
