/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2012  Free Software Foundation, Inc.
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


#include <grub/dl.h>
#include <grub/crypto.h>

GRUB_MOD_LICENSE ("GPLv3+");

struct adler32_context
{
  grub_uint16_t a, b;
  grub_uint32_t c;
};

static void
adler32_init (void *context)
{
  struct adler32_context *ctx = context;

  ctx->a = 1;
  ctx->b = 0;
}

#define MOD 65521

static grub_uint16_t
mod_add (grub_uint16_t a, grub_uint16_t b)
{
  if ((grub_uint32_t) a + (grub_uint32_t) b >= MOD)
    return a + b - MOD;
  return a + b;
}

static void
adler32_write (void *context, const void *inbuf, grub_size_t inlen)
{
  struct adler32_context *ctx = context;
  const grub_uint8_t *ptr = inbuf;

  while (inlen)
    {
      ctx->a = mod_add (ctx->a, *ptr);
      ctx->b = mod_add (ctx->a, ctx->b);
      inlen--;
      ptr++;
    }
}

static void
adler32_final (void *context __attribute__ ((unused)))
{
}

static grub_uint8_t *
adler32_read (void *context)
{
  struct adler32_context *ctx = context;
  if (ctx->a > MOD)
    ctx->a -= MOD;
  if (ctx->b > MOD)
    ctx->b -= MOD;
  ctx->c = grub_cpu_to_be32 (ctx->a | (ctx->b << 16));
  return (grub_uint8_t *) &ctx->c;
}

static gcry_md_spec_t spec_adler32 =
  {
    "ADLER32", 0, 0, 0, 4,
    adler32_init, adler32_write, adler32_final, adler32_read,
    sizeof (struct adler32_context),
#ifdef GRUB_UTIL
    .modname = "adler32",
#endif
    .blocksize = 64
  };


GRUB_MOD_INIT(adler32)
{
  grub_md_register (&spec_adler32);
}

GRUB_MOD_FINI(adler32)
{
  grub_md_unregister (&spec_adler32);
}
