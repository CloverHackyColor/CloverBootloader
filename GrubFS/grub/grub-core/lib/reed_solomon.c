/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2010  Free Software Foundation, Inc.
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

#ifdef TEST
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define xmalloc malloc
#define grub_memset memset
#define grub_memcpy memcpy
#endif

#ifndef STANDALONE
#include <assert.h>
#endif

#ifndef STANDALONE
#ifdef TEST
typedef unsigned int grub_size_t;
typedef unsigned char grub_uint8_t;
#else
#include <grub/types.h>
#include <grub/reed_solomon.h>
#include <grub/util/misc.h>
#include <grub/misc.h>
#endif
#endif

#define SECTOR_SIZE 512
#define MAX_BLOCK_SIZE (200 * SECTOR_SIZE)

#ifdef STANDALONE
#ifdef TEST
typedef unsigned int grub_size_t;
typedef unsigned char grub_uint8_t;
#else
#include <grub/types.h>
#include <grub/misc.h>
#endif
#ifdef __i386__
#define REED_SOLOMON_ATTRIBUTE  __attribute__ ((regparm(3)))
#else
#define REED_SOLOMON_ATTRIBUTE
#endif
void
grub_reed_solomon_recover (void *ptr_, grub_size_t s, grub_size_t rs)
  REED_SOLOMON_ATTRIBUTE;
#else
#define REED_SOLOMON_ATTRIBUTE
#endif

#define GF_SIZE 8
typedef grub_uint8_t gf_single_t;
#define GF_POLYNOMIAL 0x1d
#define GF_INVERT2 0x8e
#if defined (STANDALONE) && !defined (TEST)

#ifdef __APPLE__
#define ATTRIBUTE_TEXT __attribute__ ((section("_text,_text")))
#else
#define ATTRIBUTE_TEXT __attribute__ ((section(".text")))
#endif

static gf_single_t * const gf_powx ATTRIBUTE_TEXT = (void *) 0x100000;
static gf_single_t * const gf_powx_inv ATTRIBUTE_TEXT = (void *) 0x100200;
static int *const chosenstat ATTRIBUTE_TEXT = (void *) 0x100300;
static gf_single_t *const sigma ATTRIBUTE_TEXT = (void *) 0x100700;
static gf_single_t *const errpot ATTRIBUTE_TEXT = (void *) 0x100800;
static int *const errpos ATTRIBUTE_TEXT = (void *) 0x100900;
static gf_single_t *const sy ATTRIBUTE_TEXT = (void *) 0x100d00;
static gf_single_t *const mstat ATTRIBUTE_TEXT = (void *) 0x100e00;
static gf_single_t *const errvals ATTRIBUTE_TEXT = (void *) 0x100f00;
static gf_single_t *const eqstat ATTRIBUTE_TEXT = (void *) 0x101000;
/* Next available address: (void *) 0x112000.  */
#else

static gf_single_t gf_powx[255 * 2];
static gf_single_t gf_powx_inv[256];
static int chosenstat[256];
static gf_single_t sigma[256];
static gf_single_t errpot[256];
static int errpos[256];
static gf_single_t sy[256];
static gf_single_t mstat[256];
static gf_single_t errvals[256];
static gf_single_t eqstat[65536 + 256];
#endif

static gf_single_t
gf_mul (gf_single_t a, gf_single_t b)
{
  if (a == 0 || b == 0)
    return 0;
  return gf_powx[(int) gf_powx_inv[a] + (int) gf_powx_inv[b]];
}

static inline gf_single_t
gf_invert (gf_single_t a)
{
  return gf_powx[255 - (int) gf_powx_inv[a]];
}

static void
init_powx (void)
{
  int i;
  grub_uint8_t cur = 1;

  gf_powx_inv[0] = 0;
  for (i = 0; i < 255; i++)
    {
      gf_powx[i] = cur;
      gf_powx[i + 255] = cur;
      gf_powx_inv[cur] = i;
      if (cur & (1ULL << (GF_SIZE - 1)))
	cur = (cur << 1) ^ GF_POLYNOMIAL;
      else
	cur <<= 1;
    }
}

static gf_single_t
pol_evaluate (gf_single_t *pol, grub_size_t degree, int log_x)
{
  int i;
  gf_single_t s = 0;
  int log_xn = 0;
  for (i = degree; i >= 0; i--)
    {
      if (pol[i])
	s ^= gf_powx[(int) gf_powx_inv[pol[i]] + log_xn];
      log_xn += log_x;
      if (log_xn >= ((1 << GF_SIZE) - 1))
	log_xn -= ((1 << GF_SIZE) - 1);
    }
  return s;
}

#if !defined (STANDALONE)
static void
rs_encode (gf_single_t *data, grub_size_t s, grub_size_t rs)
{
  gf_single_t *rs_polynomial;
  int i, j;
  gf_single_t *m;
  m = xmalloc ((s + rs) * sizeof (gf_single_t));
  grub_memcpy (m, data, s * sizeof (gf_single_t));
  grub_memset (m + s, 0, rs * sizeof (gf_single_t));
  rs_polynomial = xmalloc ((rs + 1) * sizeof (gf_single_t));
  grub_memset (rs_polynomial, 0, (rs + 1) * sizeof (gf_single_t));
  rs_polynomial[rs] = 1;
  /* Multiply with X - a^r */
  for (j = 0; j < rs; j++)
    {
      for (i = 0; i < rs; i++)
	if (rs_polynomial[i])
	  rs_polynomial[i] = (rs_polynomial[i + 1]
			      ^ gf_powx[j + (int) gf_powx_inv[rs_polynomial[i]]]);
	else
	  rs_polynomial[i] = rs_polynomial[i + 1];
      if (rs_polynomial[rs])
	rs_polynomial[rs] = gf_powx[j + (int) gf_powx_inv[rs_polynomial[rs]]];
    }
  for (j = 0; j < s; j++)
    if (m[j])
      {
	gf_single_t f = m[j];
	for (i = 0; i <= rs; i++)
	  m[i+j] ^= gf_mul (rs_polynomial[i], f);
      }
  free (rs_polynomial);
  grub_memcpy (data + s, m + s, rs * sizeof (gf_single_t));
  free (m);
}
#endif

static void
gauss_eliminate (gf_single_t *eq, int n, int m, int *chosen)
{
  int i, j;

  for (i = 0 ; i < n; i++)
    {
      int nzidx;
      int k;
      gf_single_t r;
      for (nzidx = 0; nzidx < m && (eq[i * (m + 1) + nzidx] == 0);
	   nzidx++);
      if (nzidx == m)
	continue;
      chosen[i] = nzidx;
      r = gf_invert (eq[i * (m + 1) + nzidx]);
      for (j = 0; j < m + 1; j++)
	eq[i * (m + 1) + j] = gf_mul (eq[i * (m + 1) + j], r);
      for (j = i + 1; j < n; j++)
	{
	  gf_single_t rr = eq[j * (m + 1) + nzidx];
	  for (k = 0; k < m + 1; k++)
	    eq[j * (m + 1) + k] ^= gf_mul (eq[i * (m + 1) + k], rr);
	}
    }
}

static void
gauss_solve (gf_single_t *eq, int n, int m, gf_single_t *sol)
{
  int i, j;

  for (i = 0; i < n; i++)
    chosenstat[i] = -1;
  for (i = 0; i < m; i++)
    sol[i] = 0;
  gauss_eliminate (eq, n, m, chosenstat);
  for (i = n - 1; i >= 0; i--)
    {
      gf_single_t s = 0;
      if (chosenstat[i] == -1)
	continue;
      for (j = 0; j < m; j++)
	s ^= gf_mul (eq[i * (m + 1) + j], sol[j]);
      s ^= eq[i * (m + 1) + m];
      sol[chosenstat[i]] = s;
    }
}

static void
rs_recover (gf_single_t *mm, grub_size_t s, grub_size_t rs)
{
  grub_size_t rs2 = rs / 2;
  int errnum = 0;
  int i, j;

  for (i = 0; i < (int) rs; i++)
    sy[i] = pol_evaluate (mm, s + rs - 1, i);

  for (i = 0; i < (int) rs; i++)
    if (sy[i] != 0)
      break;

  /* No error detected.  */
  if (i == (int) rs)
    return;

  {

    for (i = 0; i < (int) rs2; i++)
      for (j = 0; j < (int) rs2 + 1; j++)
	eqstat[i * (rs2 + 1) + j] = sy[i+j];

    for (i = 0; i < (int) rs2; i++)
      sigma[i] = 0;

    gauss_solve (eqstat, rs2, rs2, sigma);
  } 

  for (i = 0; i < (int) (rs + s); i++)
    if (pol_evaluate (sigma, rs2 - 1, 255 - i) == gf_powx[i])
      {
	errpot[errnum] = gf_powx[i];
	errpos[errnum++] = s + rs - i - 1;
      }
  {
    for (j = 0; j < errnum; j++)
      eqstat[j] = 1;
    eqstat[errnum] = sy[0];
    for (i = 1; i < (int) rs; i++)
      {
	for (j = 0; j < (int) errnum; j++)
	  eqstat[(errnum + 1) * i + j] = gf_mul (errpot[j],
						 eqstat[(errnum + 1) * (i - 1)
							+ j]);
	eqstat[(errnum + 1) * i + errnum] = sy[i];
      }

    gauss_solve (eqstat, rs, errnum, errvals);

    for (i = 0; i < (int) errnum; i++)
      mm[errpos[i]] ^= errvals[i];
  }
}

static void
decode_block (gf_single_t *ptr, grub_size_t s,
	      gf_single_t *rptr, grub_size_t rs)
{
  int i, j;
  for (i = 0; i < SECTOR_SIZE; i++)
    {
      grub_size_t ds = (s + SECTOR_SIZE - 1 - i) / SECTOR_SIZE;
      grub_size_t rr = (rs + SECTOR_SIZE - 1 - i) / SECTOR_SIZE;

      /* Nothing to do.  */
      if (!ds || !rr)
	continue;

      for (j = 0; j < (int) ds; j++)
	mstat[j] = ptr[SECTOR_SIZE * j + i];
      for (j = 0; j < (int) rr; j++)
	mstat[j + ds] = rptr[SECTOR_SIZE * j + i];

      rs_recover (mstat, ds, rr);

      for (j = 0; j < (int) ds; j++)
	ptr[SECTOR_SIZE * j + i] = mstat[j];
    }
}

#if !defined (STANDALONE)
static void
encode_block (gf_single_t *ptr, grub_size_t s,
	      gf_single_t *rptr, grub_size_t rs)
{
  int i, j;
  for (i = 0; i < SECTOR_SIZE; i++)
    {
      grub_size_t ds = (s + SECTOR_SIZE - 1 - i) / SECTOR_SIZE;
      grub_size_t rr = (rs + SECTOR_SIZE - 1 - i) / SECTOR_SIZE;
      gf_single_t *m;

      if (!ds || !rr)
	continue;

      m = xmalloc (ds + rr);
      for (j = 0; j < ds; j++)
	m[j] = ptr[SECTOR_SIZE * j + i];
      rs_encode (m, ds, rr);
      for (j = 0; j < rr; j++)      
	rptr[SECTOR_SIZE * j + i] = m[j + ds];
      free (m);
    }
}
#endif

#if !defined (STANDALONE)
void
grub_reed_solomon_add_redundancy (void *buffer, grub_size_t data_size,
				  grub_size_t redundancy)
{
  grub_size_t s = data_size;
  grub_size_t rs = redundancy;
  gf_single_t *ptr = buffer;
  gf_single_t *rptr = ptr + s;
  void *tmp;

  tmp = xmalloc (data_size);
  grub_memcpy (tmp, buffer, data_size);

  /* Nothing to do.  */
  if (!rs)
    return;

  init_powx ();

  while (s > 0)
    {
      grub_size_t tt;
      grub_size_t cs, crs;
      cs = s;
      crs = rs;
      tt = cs + crs;
      if (tt > MAX_BLOCK_SIZE)
	{
	  cs = ((cs * (MAX_BLOCK_SIZE / 512)) / tt) * 512;
	  crs = ((crs * (MAX_BLOCK_SIZE / 512)) / tt) * 512;
	}
      encode_block (ptr, cs, rptr, crs);
      ptr += cs;
      rptr += crs;
      s -= cs;
      rs -= crs;
    }

#ifndef TEST
  assert (grub_memcmp (tmp, buffer, data_size) == 0);
#endif
  free (tmp);
}
#endif

void REED_SOLOMON_ATTRIBUTE
grub_reed_solomon_recover (void *ptr_, grub_size_t s, grub_size_t rs)
{
  gf_single_t *ptr = ptr_;
  gf_single_t *rptr = ptr + s;
  grub_uint8_t *cptr;

  /* Nothing to do.  */
  if (!rs)
    return;

  for (cptr = rptr + rs - 1; cptr >= rptr; cptr--)
    if (*cptr)
      break;
  if (rptr + rs - 1 - cptr > (grub_ssize_t) rs / 2)
    return;

  init_powx ();

  while (s > 0)
    {
      grub_size_t tt;
      grub_size_t cs, crs;
      cs = s;
      crs = rs;
      tt = cs + crs;
      if (tt > MAX_BLOCK_SIZE)
	{
	  cs = ((cs * (MAX_BLOCK_SIZE / 512)) / tt) * 512;
	  crs = ((crs * (MAX_BLOCK_SIZE / 512)) / tt) * 512;
	}
      decode_block (ptr, cs, rptr, crs);
      ptr += cs;
      rptr += crs;
      s -= cs;
      rs -= crs;
    }
}

#ifdef TEST
int
main (int argc, char **argv)
{
  FILE *in, *out;
  grub_size_t s, rs;
  char *buf;

  grub_memset (gf_powx, 0xee, sizeof (gf_powx));
  grub_memset (gf_powx_inv, 0xdd, sizeof (gf_powx_inv));

#ifndef STANDALONE
  init_powx ();
#endif

#ifndef STANDALONE
  in = grub_util_fopen ("tst.bin", "rb");
  if (!in)
    return 1;
  fseek (in, 0, SEEK_END);
  s = ftell (in);
  fseek (in, 0, SEEK_SET);
  rs = 0x7007;
  buf = xmalloc (s + rs + SECTOR_SIZE);
  fread (buf, 1, s, in);
  fclose (in);

  grub_reed_solomon_add_redundancy (buf, s, rs);

  out = grub_util_fopen ("tst_rs.bin", "wb");
  fwrite (buf, 1, s + rs, out);
  fclose (out);
#else
  out = grub_util_fopen ("tst_rs.bin", "rb");
  fseek (out, 0, SEEK_END);
  s = ftell (out);
  fseek (out, 0, SEEK_SET);
  rs = 0x7007;
  s -= rs;

  buf = xmalloc (s + rs + SECTOR_SIZE);
  fread (buf, 1, s + rs, out);
  fclose (out);  
#endif
#if 1
  grub_memset (buf + 512 * 15, 0, 512);
#endif

  out = grub_util_fopen ("tst_dam.bin", "wb");
  fwrite (buf, 1, s + rs, out);
  fclose (out);
  grub_reed_solomon_recover (buf, s, rs);

  out = grub_util_fopen ("tst_rec.bin", "wb");
  fwrite (buf, 1, s, out);
  fclose (out);

  return 0;
}
#endif
