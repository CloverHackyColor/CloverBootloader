/* crc64.c - crc64 function  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2008,2011  Free Software Foundation, Inc.
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
#include <grub/dl.h>
#include <grub/crypto.h>

GRUB_MOD_LICENSE ("GPLv3+");

static grub_uint64_t crc64_table [256];

/* Helper for init_crc64_table.  */
static grub_uint64_t
reflect (grub_uint64_t ref, int len)
{
  grub_uint64_t result = 0;
  int i;

  for (i = 1; i <= len; i++)
    {
      if (ref & 1)
	result |= 1ULL << (len - i);
      ref >>= 1;
    }

  return result;
}

static void
init_crc64_table (void)
{
  grub_uint64_t polynomial = 0x42f0e1eba9ea3693ULL;
  int i, j;

  for(i = 0; i < 256; i++)
    {
      crc64_table[i] = reflect(i, 8) << 56;
      for (j = 0; j < 8; j++)
	{
	  crc64_table[i] = (crc64_table[i] << 1) ^
            (crc64_table[i] & (1ULL << 63) ? polynomial : 0);
	}
      crc64_table[i] = reflect(crc64_table[i], 64);
    }
}

static void
crc64_init (void *context)
{
  if (! crc64_table[1])
    init_crc64_table ();
  *(grub_uint64_t *) context = 0;
}

static void
crc64_write (void *context, const void *buf, grub_size_t size)
{
  unsigned i;
  const grub_uint8_t *data = buf;
  grub_uint64_t crc = ~grub_le_to_cpu64 (*(grub_uint64_t *) context);

  for (i = 0; i < size; i++)
    {
      crc = (crc >> 8) ^ crc64_table[(crc & 0xFF) ^ *data];
      data++;
    }

  *(grub_uint64_t *) context = grub_cpu_to_le64 (~crc);
}

static grub_uint8_t *
crc64_read (void *context)
{
  return context;
}

static void
crc64_final (void *context __attribute__ ((unused)))
{
}

gcry_md_spec_t _gcry_digest_spec_crc64 =
  {
    "CRC64", 0, 0, 0, 8,
    crc64_init, crc64_write, crc64_final, crc64_read,
    sizeof (grub_uint64_t),
    .blocksize = 64
  };

GRUB_MOD_INIT(crc64)
{
  grub_md_register (&_gcry_digest_spec_crc64);
}

GRUB_MOD_FINI(crc64)
{
  grub_md_unregister (&_gcry_digest_spec_crc64);
}
