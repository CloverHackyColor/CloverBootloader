/* gc-pbkdf2-sha1.c --- Password-Based Key Derivation Function a'la PKCS#5
   Copyright (C) 2002, 2003, 2004, 2005, 2006, 2009 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

/* Written by Simon Josefsson.  */
/* Imported from gnulib.  */

#include <grub/crypto.h>
#include <grub/mm.h>
#include <grub/misc.h>
#include <grub/dl.h>

GRUB_MOD_LICENSE ("GPLv2+");

/* Implement PKCS#5 PBKDF2 as per RFC 2898.  The PRF to use is HMAC variant
   of digest supplied by MD.  Inputs are the password P of length PLEN,
   the salt S of length SLEN, the iteration counter C (> 0), and the
   desired derived output length DKLEN.  Output buffer is DK which
   must have room for at least DKLEN octets.  The output buffer will
   be filled with the derived data.  */
#pragma GCC diagnostic ignored "-Wunreachable-code"

gcry_err_code_t
grub_crypto_pbkdf2 (const struct gcry_md_spec *md,
		    const grub_uint8_t *P, grub_size_t Plen,
		    const grub_uint8_t *S, grub_size_t Slen,
		    unsigned int c,
		    grub_uint8_t *DK, grub_size_t dkLen)
{
  unsigned int hLen = md->mdlen;
  grub_uint8_t U[GRUB_CRYPTO_MAX_MDLEN];
  grub_uint8_t T[GRUB_CRYPTO_MAX_MDLEN];
  unsigned int u;
  unsigned int l;
  unsigned int r;
  unsigned int i;
  unsigned int k;
  gcry_err_code_t rc;
  grub_uint8_t *tmp;
  grub_size_t tmplen = Slen + 4;

  if (md->mdlen > GRUB_CRYPTO_MAX_MDLEN)
    return GPG_ERR_INV_ARG;

  if (c == 0)
    return GPG_ERR_INV_ARG;

  if (dkLen == 0)
    return GPG_ERR_INV_ARG;

  if (dkLen > 4294967295U)
    return GPG_ERR_INV_ARG;

  l = ((dkLen - 1) / hLen) + 1;
  r = dkLen - (l - 1) * hLen;

  tmp = grub_malloc (tmplen);
  if (tmp == NULL)
    return GPG_ERR_OUT_OF_MEMORY;

  grub_memcpy (tmp, S, Slen);

  for (i = 1; i - 1 < l; i++)
    {
      grub_memset (T, 0, hLen);

      for (u = 0; u < c; u++)
	{
	  if (u == 0)
	    {
	      tmp[Slen + 0] = (i & 0xff000000) >> 24;
	      tmp[Slen + 1] = (i & 0x00ff0000) >> 16;
	      tmp[Slen + 2] = (i & 0x0000ff00) >> 8;
	      tmp[Slen + 3] = (i & 0x000000ff) >> 0;

	      rc = grub_crypto_hmac_buffer (md, P, Plen, tmp, tmplen, U);
	    }
	  else
	    rc = grub_crypto_hmac_buffer (md, P, Plen, U, hLen, U);

	  if (rc != GPG_ERR_NO_ERROR)
	    {
	      grub_free (tmp);
	      return rc;
	    }

	  for (k = 0; k < hLen; k++)
	    T[k] ^= U[k];
	}

      grub_memcpy (DK + (i - 1) * hLen, T, i == l ? r : hLen);
    }

  grub_free (tmp);

  return GPG_ERR_NO_ERROR;
}
