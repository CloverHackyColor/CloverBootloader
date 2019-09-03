/* hexdump.c - hexdump function */
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

#include <grub/types.h>
#include <grub/misc.h>
#include <grub/lib/hexdump.h>

void
hexdump (unsigned long bse, char *buf, int len)
{
  int pos;
  char line[80];

  while (len > 0)
    {
      int cnt, i;

      pos = grub_snprintf (line, sizeof (line), "%08lx  ", bse);
      cnt = 16;
      if (cnt > len)
	cnt = len;

      for (i = 0; i < cnt; i++)
	{
	  pos += grub_snprintf (&line[pos], sizeof (line) - pos,
				"%02x ", (unsigned char) buf[i]);
	  if ((i & 7) == 7)
	    line[pos++] = ' ';
	}

      for (; i < 16; i++)
	{
	  pos += grub_snprintf (&line[pos], sizeof (line) - pos, "   ");
	  if ((i & 7) == 7)
	    line[pos++] = ' ';
	}

      line[pos++] = '|';

      for (i = 0; i < cnt; i++)
	line[pos++] = ((buf[i] >= 32) && (buf[i] < 127)) ? buf[i] : '.';

      line[pos++] = '|';

      line[pos] = 0;

      grub_printf ("%s\n", line);

      /* Print only first and last line if more than 3 lines are identical.  */
      if (len >= 4 * 16
	  && ! grub_memcmp (buf, buf + 1 * 16, 16)
	  && ! grub_memcmp (buf, buf + 2 * 16, 16)
	  && ! grub_memcmp (buf, buf + 3 * 16, 16))
	{
	  grub_printf ("*\n");
	  do
	    {
	      bse += 16;
	      buf += 16;
	      len -= 16;
	    }
	  while (len >= 3 * 16 && ! grub_memcmp (buf, buf + 2 * 16, 16));
	}

      bse += 16;
      buf += 16;
      len -= cnt;
    }
}
