/* gzio.c - decompression support for gzip */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2005,2006,2007,2009  Free Software Foundation, Inc.
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

/*
 * Most of this file was originally the source file "inflate.c", written
 * by Mark Adler.  It has been very heavily modified.  In particular, the
 * original would run through the whole file at once, and this version can
 * be stopped and restarted on any boundary during the decompression process.
 *
 * The license and header comments that file are included here.
 */

/* inflate.c -- Not copyrighted 1992 by Mark Adler
   version c10p1, 10 January 1993 */

/* You can do whatever you like with this source file, though I would
   prefer that if you modify it and redistribute it that you include
   comments to that effect with your name and the date.  Thank you.
 */

#include <grub/err.h>
#include <grub/types.h>
#include <grub/mm.h>
#include <grub/misc.h>
#include <grub/fs.h>
#include <grub/file.h>
#include <grub/dl.h>
#include <grub/deflate.h>
#include <grub/i18n.h>

GRUB_MOD_LICENSE ("GPLv3+");

/*
 *  Window Size
 *
 *  This must be a power of two, and at least 32K for zip's deflate method
 */

#define WSIZE	0x8000


#define INBUFSIZ  0x2000

/* The state stored in filesystem-specific data.  */
struct grub_gzio
{
  /* The underlying file object.  */
  grub_file_t file;
  /* If input is in memory following fields are used instead of file.  */
  grub_size_t mem_input_size, mem_input_off;
  grub_uint8_t *mem_input;
  /* The offset at which the data starts in the underlying file.  */
  grub_off_t data_offset;
  /* The type of current block.  */
  int block_type;
  /* The length of current block.  */
  int block_len;
  /* The flag of the last block.  */
  int last_block;
  /* The flag of codes.  */
  int code_state;
  /* The length of a copy.  */
  unsigned inflate_n;
  /* The index of a copy.  */
  unsigned inflate_d;
  /* The input buffer.  */
  grub_uint8_t inbuf[INBUFSIZ];
  int inbuf_d;
  /* The bit buffer.  */
  unsigned long bb;
  /* The bits in the bit buffer.  */
  unsigned bk;
  /* The sliding window in uncompressed data.  */
  grub_uint8_t slide[WSIZE];
  /* Current position in the slide.  */
  unsigned wp;
  /* The literal/length code table.  */
  struct huft *tl;
  /* The distance code table.  */
  struct huft *td;
  /* The lookup bits for the literal/length code table. */
  int bl;
  /* The lookup bits for the distance code table.  */
  int bd;
  /* The original offset value.  */
  grub_off_t saved_offset;
};
typedef struct grub_gzio *grub_gzio_t;

/* Declare the filesystem structure for grub_gzio_open.  */
static struct grub_fs grub_gzio_fs;

/* Function prototypes */
static void initialize_tables (grub_gzio_t);

/* Eat variable-length header fields.  */
static int
eat_field (grub_file_t file, int len)
{
  char ch = 1;
  int not_retval = 1;

  do
    {
      if (len >= 0)
	{
	  if (! (len--))
	    break;
	}
      else
	{
	  if (! ch)
	    break;
	}
    }
  while ((not_retval = grub_file_read (file, &ch, 1)) == 1);

  return ! not_retval;
}


/* Little-Endian defines for the 2-byte magic numbers for gzip files.  */
#define GZIP_MAGIC	grub_le_to_cpu16 (0x8B1F)
#define OLD_GZIP_MAGIC	grub_le_to_cpu16 (0x9E1F)

/* Compression methods (see algorithm.doc) */
#define STORED      0
#define COMPRESSED  1
#ifndef PACKED
#define PACKED      2
#else /* PACKED is in use */
#define GZPACKED    2
#endif
#define LZHED       3
/* methods 4 to 7 reserved */
#define DEFLATED    8
#define MAX_METHODS 9

/* gzip flag byte */
#define ASCII_FLAG   0x01	/* bit 0 set: file probably ascii text */
#define CONTINUATION 0x02	/* bit 1 set: continuation of multi-part gzip file */
#define EXTRA_FIELD  0x04	/* bit 2 set: extra field present */
#define ORIG_NAME    0x08	/* bit 3 set: original file name present */
#define COMMENT      0x10	/* bit 4 set: file comment present */
#define ENCRYPTED    0x20	/* bit 5 set: file is encrypted */
#define RESERVED     0xC0	/* bit 6,7:   reserved */

#define UNSUPPORTED_FLAGS	(CONTINUATION | ENCRYPTED | RESERVED)

/* inflate block codes */
#define INFLATE_STORED	0
#define INFLATE_FIXED	1
#define INFLATE_DYNAMIC	2

typedef unsigned char uch;
typedef unsigned short ush;
typedef unsigned long ulg;

static int
test_gzip_header (grub_file_t file)
{
  struct {
    grub_uint16_t magic;
    grub_uint8_t method;
    grub_uint8_t flags;
    grub_uint32_t timestamp;
    grub_uint8_t extra_flags;
    grub_uint8_t os_type;
  } hdr;
  grub_uint16_t extra_len;
  grub_uint32_t orig_len;
  grub_gzio_t gzio = file->data;

  if (grub_file_tell (gzio->file) != 0)
    grub_file_seek (gzio->file, 0);

  /*
   *  This checks if the file is gzipped.  If a problem occurs here
   *  (other than a real error with the disk) then we don't think it
   *  is a compressed file, and simply mark it as such.
   */
  if (grub_file_read (gzio->file, &hdr, 10) != 10
      || ((hdr.magic != GZIP_MAGIC)
	  && (hdr.magic != OLD_GZIP_MAGIC)))
    return 0;

  /*
   *  This does consistency checking on the header data.  If a
   *  problem occurs from here on, then we have corrupt or otherwise
   *  bad data, and the error should be reported to the user.
   */
  if (hdr.method != DEFLATED
      || (hdr.flags & UNSUPPORTED_FLAGS)
      || ((hdr.flags & EXTRA_FIELD)
	  && (grub_file_read (gzio->file, &extra_len, 2) != 2
	      || eat_field (gzio->file,
			    grub_le_to_cpu16 (extra_len))))
      || ((hdr.flags & ORIG_NAME) && eat_field (gzio->file, -1))
      || ((hdr.flags & COMMENT) && eat_field (gzio->file, -1)))
    return 0;

  gzio->data_offset = grub_file_tell (gzio->file);

  /* FIXME: don't do this on not easily seekable files.  */
  {
    grub_file_seek (gzio->file, grub_file_size (gzio->file) - 4);
    if (grub_file_read (gzio->file, &orig_len, 4) != 4)
      return 0;
    /* FIXME: this does not handle files whose original size is over 4GB.
       But how can we know the real original size?  */
    file->size = grub_le_to_cpu32 (orig_len);
  }

  initialize_tables (gzio);

  return 1;
}


/* Huffman code lookup table entry--this entry is four bytes for machines
   that have 16-bit pointers (e.g. PC's in the small or medium model).
   Valid extra bits are 0..13.  e == 15 is EOB (end of block), e == 16
   means that v is a literal, 16 < e < 32 means that v is a pointer to
   the next table, which codes e - 16 bits, and lastly e == 99 indicates
   an unused code.  If a code with e == 99 is looked up, this implies an
   error in the data. */
struct huft
{
  uch e;			/* number of extra bits or operation */
  uch b;			/* number of bits in this code or subcode */
  union
    {
      ush n;			/* literal, length base, or distance base */
      struct huft *t;		/* pointer to next level of table */
    }
  v;
};


/* The inflate algorithm uses a sliding 32K byte window on the uncompressed
   stream to find repeated byte strings.  This is implemented here as a
   circular buffer.  The index is updated simply by incrementing and then
   and'ing with 0x7fff (32K-1). */
/* It is left to other modules to supply the 32K area.  It is assumed
   to be usable as if it were declared "uch slide[32768];" or as just
   "uch *slide;" and then malloc'ed in the latter case.  The definition
   must be in unzip.h, included above. */


/* Tables for deflate from PKZIP's appnote.txt. */
static unsigned bitorder[] =
{				/* Order of the bit length code lengths */
  16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
static ush cplens[] =
{				/* Copy lengths for literal codes 257..285 */
  3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
  35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};
	/* note: see note #13 above about the 258 in this list. */
static ush cplext[] =
{				/* Extra bits for literal codes 257..285 */
  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
  3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99};	/* 99==invalid */
static ush cpdist[] =
{				/* Copy offsets for distance codes 0..29 */
  1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
  257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
  8193, 12289, 16385, 24577};
static ush cpdext[] =
{				/* Extra bits for distance codes */
  0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
  7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
  12, 12, 13, 13};


/*
   Huffman code decoding is performed using a multi-level table lookup.
   The fastest way to decode is to simply build a lookup table whose
   size is determined by the longest code.  However, the time it takes
   to build this table can also be a factor if the data being decoded
   is not very long.  The most common codes are necessarily the
   shortest codes, so those codes dominate the decoding time, and hence
   the speed.  The idea is you can have a shorter table that decodes the
   shorter, more probable codes, and then point to subsidiary tables for
   the longer codes.  The time it costs to decode the longer codes is
   then traded against the time it takes to make longer tables.

   This results of this trade are in the variables lbits and dbits
   below.  lbits is the number of bits the first level table for literal/
   length codes can decode in one step, and dbits is the same thing for
   the distance codes.  Subsequent tables are also less than or equal to
   those sizes.  These values may be adjusted either when all of the
   codes are shorter than that, in which case the longest code length in
   bits is used, or when the shortest code is *longer* than the requested
   table size, in which case the length of the shortest code in bits is
   used.

   There are two different values for the two tables, since they code a
   different number of possibilities each.  The literal/length table
   codes 286 possible values, or in a flat code, a little over eight
   bits.  The distance table codes 30 possible values, or a little less
   than five bits, flat.  The optimum values for speed end up being
   about one bit more than those, so lbits is 8+1 and dbits is 5+1.
   The optimum values may differ though from machine to machine, and
   possibly even between compilers.  Your mileage may vary.
 */


static int lbits = 9;		/* bits in base literal/length lookup table */
static int dbits = 6;		/* bits in base distance lookup table */


/* If BMAX needs to be larger than 16, then h and x[] should be ulg. */
#define BMAX 16			/* maximum bit length of any code (16 for explode) */
#define N_MAX 288		/* maximum number of codes in any set */


/* Macros for inflate() bit peeking and grabbing.
   The usage is:

        NEEDBITS(j)
        x = b & mask_bits[j];
        DUMPBITS(j)

   where NEEDBITS makes sure that b has at least j bits in it, and
   DUMPBITS removes the bits from b.  The macros use the variable k
   for the number of bits in b.  Normally, b and k are register
   variables for speed, and are initialized at the beginning of a
   routine that uses these macros from a global bit buffer and count.

   If we assume that EOB will be the longest code, then we will never
   ask for bits with NEEDBITS that are beyond the end of the stream.
   So, NEEDBITS should not read any more bytes than are needed to
   meet the request.  Then no bytes need to be "returned" to the buffer
   at the end of the last block.

   However, this assumption is not true for fixed blocks--the EOB code
   is 7 bits, but the other literal/length codes can be 8 or 9 bits.
   (The EOB code is shorter than other codes because fixed blocks are
   generally short.  So, while a block always has an EOB, many other
   literal/length codes have a significantly lower probability of
   showing up at all.)  However, by making the first table have a
   lookup of seven bits, the EOB code will be found in that first
   lookup, and so will not require that too many bits be pulled from
   the stream.
 */

static ush mask_bits[] =
{
  0x0000,
  0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
  0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

#define NEEDBITS(n) do {while(k<(n)){b|=((ulg)get_byte(gzio))<<k;k+=8;}} while (0)
#define DUMPBITS(n) do {b>>=(n);k-=(n);} while (0)

static int
get_byte (grub_gzio_t gzio)
{
  if (gzio->mem_input)
    {
      if (gzio->mem_input_off < gzio->mem_input_size)
	return gzio->mem_input[gzio->mem_input_off++];
      return 0;
    }

  if (gzio->file && (grub_file_tell (gzio->file)
		     == (grub_off_t) gzio->data_offset
		     || gzio->inbuf_d == INBUFSIZ))
    {
      gzio->inbuf_d = 0;
      grub_file_read (gzio->file, gzio->inbuf, INBUFSIZ);
    }

  return gzio->inbuf[gzio->inbuf_d++];
}

static void
gzio_seek (grub_gzio_t gzio, grub_off_t off)
{
  if (gzio->mem_input)
    {
      if (off > gzio->mem_input_size)
	grub_error (GRUB_ERR_OUT_OF_RANGE,
		    N_("attempt to seek outside of the file"));
      else
	gzio->mem_input_off = off;
    }
  else
    grub_file_seek (gzio->file, off);
}

/* more function prototypes */
static int huft_build (unsigned *, unsigned, unsigned, ush *, ush *,
		       struct huft **, int *);
static int huft_free (struct huft *);
static int inflate_codes_in_window (grub_gzio_t);


/* Given a list of code lengths and a maximum table size, make a set of
   tables to decode that set of codes.  Return zero on success, one if
   the given code set is incomplete (the tables are still built in this
   case), two if the input is invalid (all zero length codes or an
   oversubscribed set of lengths), and three if not enough memory. */

static int
huft_build (unsigned *b,	/* code lengths in bits (all assumed <= BMAX) */
	    unsigned n,		/* number of codes (assumed <= N_MAX) */
	    unsigned s,		/* number of simple-valued codes (0..s-1) */
	    ush * d,		/* list of base values for non-simple codes */
	    ush * e,		/* list of extra bits for non-simple codes */
	    struct huft **t,	/* result: starting table */
	    int *m)		/* maximum lookup bits, returns actual */
{
  unsigned a;			/* counter for codes of length k */
  unsigned c[BMAX + 1];		/* bit length count table */
  unsigned f;			/* i repeats in table every f entries */
  int g;			/* maximum code length */
  int h;			/* table level */
  register unsigned i;		/* counter, current code */
  register unsigned j;		/* counter */
  register int k;		/* number of bits in current code */
  int l;			/* bits per table (returned in m) */
  register unsigned *p;		/* pointer into c[], b[], or v[] */
  register struct huft *q;	/* points to current table */
  struct huft r;		/* table entry for structure assignment */
  struct huft *u[BMAX];		/* table stack */
  unsigned v[N_MAX];		/* values in order of bit length */
  register int w;		/* bits before this table == (l * h) */
  unsigned x[BMAX + 1];		/* bit offsets, then code stack */
  unsigned *xp;			/* pointer into x */
  int y;			/* number of dummy codes added */
  unsigned z;			/* number of entries in current table */

  /* Generate counts for each bit length */
  grub_memset ((char *) c, 0, sizeof (c));
  p = b;
  i = n;
  do
    {
      c[*p]++;			/* assume all entries <= BMAX */
      p++;			/* Can't combine with above line (Solaris bug) */
    }
  while (--i);
  if (c[0] == n)		/* null input--all zero length codes */
    {
      *t = (struct huft *) NULL;
      *m = 0;
      return 0;
    }

  /* Find minimum and maximum length, bound *m by those */
  l = *m;
  for (j = 1; j <= BMAX; j++)
    if (c[j])
      break;
  k = j;			/* minimum code length */
  if ((unsigned) l < j)
    l = j;
  for (i = BMAX; i; i--)
    if (c[i])
      break;
  g = i;			/* maximum code length */
  if ((unsigned) l > i)
    l = i;
  *m = l;

  /* Adjust last length count to fill out codes, if needed */
  for (y = 1 << j; j < i; j++, y <<= 1)
    if ((y -= c[j]) < 0)
      return 2;			/* bad input: more codes than bits */
  if ((y -= c[i]) < 0)
    return 2;
  c[i] += y;

  /* Generate starting offsets into the value table for each length */
  x[1] = j = 0;
  p = c + 1;
  xp = x + 2;
  while (--i)
    {				/* note that i == g from above */
      *xp++ = (j += *p++);
    }

  /* Make a table of values in order of bit lengths */
  p = b;
  i = 0;
  do
    {
      if ((j = *p++) != 0)
	v[x[j]++] = i;
    }
  while (++i < n);

  /* Generate the Huffman codes and for each, make the table entries */
  x[0] = i = 0;			/* first Huffman code is zero */
  p = v;			/* grab values in bit order */
  h = -1;			/* no tables yet--level -1 */
  w = -l;			/* bits decoded == (l * h) */
  u[0] = (struct huft *) NULL;	/* just to keep compilers happy */
  q = (struct huft *) NULL;	/* ditto */
  z = 0;			/* ditto */

  /* go through the bit lengths (k already is bits in shortest code) */
  for (; k <= g; k++)
    {
      a = c[k];
      while (a--)
	{
	  /* here i is the Huffman code of length k bits for value *p */
	  /* make tables up to required level */
	  while (k > w + l)
	    {
	      h++;
	      w += l;		/* previous table always l bits */

	      /* compute minimum size table less than or equal to l bits */
	      z = (z = (unsigned) (g - w)) > (unsigned) l ? (unsigned) l : z;	/* upper limit on table size */
	      if ((f = 1 << (j = k - w)) > a + 1)	/* try a k-w bit table */
		{		/* too few codes for k-w bit table */
		  f -= a + 1;	/* deduct codes from patterns left */
		  xp = c + k;
		  while (++j < z)	/* try smaller tables up to z bits */
		    {
		      if ((f <<= 1) <= *++xp)
			break;	/* enough codes to use up j bits */
		      f -= *xp;	/* else deduct codes from patterns */
		    }
		}
	      z = 1 << j;	/* table entries for j-bit table */

	      /* allocate and link in new table */
	      q = (struct huft *) grub_zalloc ((z + 1) * sizeof (struct huft));
	      if (! q)
		{
		  if (h)
		    huft_free (u[0]);
		  return 3;
		}

	      *t = q + 1;	/* link to list for huft_free() */
	      *(t = &(q->v.t)) = (struct huft *) NULL;
	      u[h] = ++q;	/* table starts after link */

	      /* connect to last table, if there is one */
	      if (h)
		{
		  x[h] = i;	/* save pattern for backing up */
		  r.b = (uch) l;	/* bits to dump before this table */
		  r.e = (uch) (16 + j);		/* bits in this table */
		  r.v.t = q;	/* pointer to this table */
		  j = i >> (w - l);	/* (get around Turbo C bug) */
		  u[h - 1][j] = r;	/* connect to last table */
		}
	    }

	  /* set up table entry in r */
	  r.b = (uch) (k - w);
	  if (p >= v + n)
	    r.e = 99;		/* out of values--invalid code */
	  else if (*p < s)
	    {
	      r.e = (uch) (*p < 256 ? 16 : 15);		/* 256 is end-of-block code */
	      r.v.n = (ush) (*p);	/* simple code is just the value */
	      p++;		/* one compiler does not like *p++ */
	    }
	  else
	    {
	      r.e = (uch) e[*p - s];	/* non-simple--look up in lists */
	      r.v.n = d[*p++ - s];
	    }

	  /* fill code-like entries with r */
	  f = 1 << (k - w);
	  for (j = i >> w; j < z; j += f)
	    q[j] = r;

	  /* backwards increment the k-bit code i */
	  for (j = 1 << (k - 1); i & j; j >>= 1)
	    i ^= j;
	  i ^= j;

	  /* backup over finished tables */
	  while ((i & ((1 << w) - 1)) != x[h])
	    {
	      h--;		/* don't need to update q */
	      w -= l;
	    }
	}
    }

  /* Return true (1) if we were given an incomplete table */
  return y != 0 && g != 1;
}


/* Free the malloc'ed tables built by huft_build(), which makes a linked
   list of the tables it made, with the links in a dummy first entry of
   each table.  */
static int
huft_free (struct huft *t)
{
  register struct huft *p, *q;


  /* Go through linked list, freeing from the malloced (t[-1]) address. */
  p = t;
  while (p != (struct huft *) NULL)
    {
      q = (--p)->v.t;
      grub_free ((char *) p);
      p = q;
    }
  return 0;
}


/*
 *  inflate (decompress) the codes in a deflated (compressed) block.
 *  Return an error code or zero if it all goes ok.
 */

static int
inflate_codes_in_window (grub_gzio_t gzio)
{
  register unsigned e;		/* table entry flag/number of extra bits */
  unsigned n, d;		/* length and index for copy */
  unsigned w;			/* current window position */
  struct huft *t;		/* pointer to table entry */
  unsigned ml, md;		/* masks for bl and bd bits */
  register ulg b;		/* bit buffer */
  register unsigned k;		/* number of bits in bit buffer */

  /* make local copies of globals */
  d = gzio->inflate_d;
  n = gzio->inflate_n;
  b = gzio->bb;			/* initialize bit buffer */
  k = gzio->bk;
  w = gzio->wp;			/* initialize window position */

  /* inflate the coded data */
  ml = mask_bits[gzio->bl];		/* precompute masks for speed */
  md = mask_bits[gzio->bd];
  for (;;)			/* do until end of block */
    {
      if (! gzio->code_state)
	{
	  NEEDBITS ((unsigned) gzio->bl);
	  if ((e = (t = gzio->tl + ((unsigned) b & ml))->e) > 16)
	    do
	      {
		if (e == 99)
		  {
		    grub_error (GRUB_ERR_BAD_COMPRESSED_DATA,
				"an unused code found");
		    return 1;
		  }
		DUMPBITS (t->b);
		e -= 16;
		NEEDBITS (e);
	      }
	    while ((e = (t = t->v.t + ((unsigned) b & mask_bits[e]))->e) > 16);
	  DUMPBITS (t->b);

	  if (e == 16)		/* then it's a literal */
	    {
	      gzio->slide[w++] = (uch) t->v.n;
	      if (w == WSIZE)
		break;
	    }
	  else
	    /* it's an EOB or a length */
	    {
	      /* exit if end of block */
	      if (e == 15)
		{
		  gzio->block_len = 0;
		  break;
		}

	      /* get length of block to copy */
	      NEEDBITS (e);
	      n = t->v.n + ((unsigned) b & mask_bits[e]);
	      DUMPBITS (e);

	      /* decode distance of block to copy */
	      NEEDBITS ((unsigned) gzio->bd);
	      if ((e = (t = gzio->td + ((unsigned) b & md))->e) > 16)
		do
		  {
		    if (e == 99)
		      {
			grub_error (GRUB_ERR_BAD_COMPRESSED_DATA,
				    "an unused code found");
			return 1;
		      }
		    DUMPBITS (t->b);
		    e -= 16;
		    NEEDBITS (e);
		  }
		while ((e = (t = t->v.t + ((unsigned) b & mask_bits[e]))->e)
		       > 16);
	      DUMPBITS (t->b);
	      NEEDBITS (e);
	      d = w - t->v.n - ((unsigned) b & mask_bits[e]);
	      DUMPBITS (e);
	      gzio->code_state++;
	    }
	}

      if (gzio->code_state)
	{
	  /* do the copy */
	  do
	    {
	      n -= (e = (e = WSIZE - ((d &= WSIZE - 1) > w ? d : w)) > n ? n
		    : e);

	      if (w - d >= e)
		{
		  grub_memmove (gzio->slide + w, gzio->slide + d, e);
		  w += e;
		  d += e;
		}
	      else
		/* purposefully use the overlap for extra copies here!! */
		{
		  while (e--)
		    gzio->slide[w++] = gzio->slide[d++];
		}

	      if (w == WSIZE)
		break;
	    }
	  while (n);

	  if (! n)
	    gzio->code_state--;

	  /* did we break from the loop too soon? */
	  if (w == WSIZE)
	    break;
	}
    }

  /* restore the globals from the locals */
  gzio->inflate_d = d;
  gzio->inflate_n = n;
  gzio->wp = w;			/* restore global window pointer */
  gzio->bb = b;			/* restore global bit buffer */
  gzio->bk = k;

  return ! gzio->block_len;
}


/* get header for an inflated type 0 (stored) block. */

static void
init_stored_block (grub_gzio_t gzio)
{
  register ulg b;		/* bit buffer */
  register unsigned k;		/* number of bits in bit buffer */

  /* make local copies of globals */
  b = gzio->bb;			/* initialize bit buffer */
  k = gzio->bk;

  /* go to byte boundary */
  DUMPBITS (k & 7);

  /* get the length and its complement */
  NEEDBITS (16);
  gzio->block_len = ((unsigned) b & 0xffff);
  DUMPBITS (16);
  NEEDBITS (16);
  if (gzio->block_len != (int) ((~b) & 0xffff))
    grub_error (GRUB_ERR_BAD_COMPRESSED_DATA,
		"the length of a stored block does not match");
  DUMPBITS (16);

  /* restore global variables */
  gzio->bb = b;
  gzio->bk = k;
}


/* get header for an inflated type 1 (fixed Huffman codes) block.  We should
   either replace this with a custom decoder, or at least precompute the
   Huffman tables. */

static void
init_fixed_block (grub_gzio_t gzio)
{
  int i;			/* temporary variable */
  unsigned l[288];		/* length list for huft_build */

  /* set up literal table */
  for (i = 0; i < 144; i++)
    l[i] = 8;
  for (; i < 256; i++)
    l[i] = 9;
  for (; i < 280; i++)
    l[i] = 7;
  for (; i < 288; i++)		/* make a complete, but wrong code set */
    l[i] = 8;
  gzio->bl = 7;
  if (huft_build (l, 288, 257, cplens, cplext, &gzio->tl, &gzio->bl) != 0)
    {
      if (grub_errno == GRUB_ERR_NONE)
	grub_error (GRUB_ERR_BAD_COMPRESSED_DATA,
		    "failed in building a Huffman code table");
      return;
    }

  /* set up distance table */
  for (i = 0; i < 30; i++)	/* make an incomplete code set */
    l[i] = 5;
  gzio->bd = 5;
  if (huft_build (l, 30, 0, cpdist, cpdext, &gzio->td, &gzio->bd) > 1)
    {
      if (grub_errno == GRUB_ERR_NONE)
	grub_error (GRUB_ERR_BAD_COMPRESSED_DATA,
		    "failed in building a Huffman code table");
      huft_free (gzio->tl);
      gzio->tl = 0;
      return;
    }

  /* indicate we're now working on a block */
  gzio->code_state = 0;
  gzio->block_len++;
}


/* get header for an inflated type 2 (dynamic Huffman codes) block. */

static void
init_dynamic_block (grub_gzio_t gzio)
{
  int i;			/* temporary variables */
  unsigned j;
  unsigned l;			/* last length */
  unsigned m;			/* mask for bit lengths table */
  unsigned n;			/* number of lengths to get */
  unsigned nb;			/* number of bit length codes */
  unsigned nl;			/* number of literal/length codes */
  unsigned nd;			/* number of distance codes */
  unsigned ll[286 + 30];	/* literal/length and distance code lengths */
  register ulg b;		/* bit buffer */
  register unsigned k;		/* number of bits in bit buffer */

  /* make local bit buffer */
  b = gzio->bb;
  k = gzio->bk;

  /* read in table lengths */
  NEEDBITS (5);
  nl = 257 + ((unsigned) b & 0x1f);	/* number of literal/length codes */
  DUMPBITS (5);
  NEEDBITS (5);
  nd = 1 + ((unsigned) b & 0x1f);	/* number of distance codes */
  DUMPBITS (5);
  NEEDBITS (4);
  nb = 4 + ((unsigned) b & 0xf);	/* number of bit length codes */
  DUMPBITS (4);
  if (nl > 286 || nd > 30)
    {
      grub_error (GRUB_ERR_BAD_COMPRESSED_DATA, "too much data");
      return;
    }

  /* read in bit-length-code lengths */
  for (j = 0; j < nb; j++)
    {
      NEEDBITS (3);
      ll[bitorder[j]] = (unsigned) b & 7;
      DUMPBITS (3);
    }
  for (; j < 19; j++)
    ll[bitorder[j]] = 0;

  /* build decoding table for trees--single level, 7 bit lookup */
  gzio->bl = 7;
  if (huft_build (ll, 19, 19, NULL, NULL, &gzio->tl, &gzio->bl) != 0)
    {
      grub_error (GRUB_ERR_BAD_COMPRESSED_DATA,
		  "failed in building a Huffman code table");
      return;
    }

  /* read in literal and distance code lengths */
  n = nl + nd;
  m = mask_bits[gzio->bl];
  i = l = 0;
  while ((unsigned) i < n)
    {
      NEEDBITS ((unsigned) gzio->bl);
      j = (gzio->td = gzio->tl + ((unsigned) b & m))->b;
      DUMPBITS (j);
      j = gzio->td->v.n;
      if (j < 16)		/* length of code in bits (0..15) */
	ll[i++] = l = j;	/* save last length in l */
      else if (j == 16)		/* repeat last length 3 to 6 times */
	{
	  NEEDBITS (2);
	  j = 3 + ((unsigned) b & 3);
	  DUMPBITS (2);
	  if ((unsigned) i + j > n)
	    {
	      grub_error (GRUB_ERR_BAD_COMPRESSED_DATA, "too many codes found");
	      return;
	    }
	  while (j--)
	    ll[i++] = l;
	}
      else if (j == 17)		/* 3 to 10 zero length codes */
	{
	  NEEDBITS (3);
	  j = 3 + ((unsigned) b & 7);
	  DUMPBITS (3);
	  if ((unsigned) i + j > n)
	    {
	      grub_error (GRUB_ERR_BAD_COMPRESSED_DATA, "too many codes found");
	      return;
	    }
	  while (j--)
	    ll[i++] = 0;
	  l = 0;
	}
      else
	/* j == 18: 11 to 138 zero length codes */
	{
	  NEEDBITS (7);
	  j = 11 + ((unsigned) b & 0x7f);
	  DUMPBITS (7);
	  if ((unsigned) i + j > n)
	    {
	      grub_error (GRUB_ERR_BAD_COMPRESSED_DATA, "too many codes found");
	      return;
	    }
	  while (j--)
	    ll[i++] = 0;
	  l = 0;
	}
    }

  /* free decoding table for trees */
  huft_free (gzio->tl);
  gzio->td = 0;
  gzio->tl = 0;

  /* restore the global bit buffer */
  gzio->bb = b;
  gzio->bk = k;

  /* build the decoding tables for literal/length and distance codes */
  gzio->bl = lbits;
  if (huft_build (ll, nl, 257, cplens, cplext, &gzio->tl, &gzio->bl) != 0)
    {
      grub_error (GRUB_ERR_BAD_COMPRESSED_DATA,
		  "failed in building a Huffman code table");
      return;
    }
  gzio->bd = dbits;
  if (huft_build (ll + nl, nd, 0, cpdist, cpdext, &gzio->td, &gzio->bd) != 0)
    {
      huft_free (gzio->tl);
      gzio->tl = 0;
      grub_error (GRUB_ERR_BAD_COMPRESSED_DATA,
		  "failed in building a Huffman code table");
      return;
    }

  /* indicate we're now working on a block */
  gzio->code_state = 0;
  gzio->block_len++;
}


static void
get_new_block (grub_gzio_t gzio)
{
  register ulg b;		/* bit buffer */
  register unsigned k;		/* number of bits in bit buffer */

  /* make local bit buffer */
  b = gzio->bb;
  k = gzio->bk;

  /* read in last block bit */
  NEEDBITS (1);
  gzio->last_block = (int) b & 1;
  DUMPBITS (1);

  /* read in block type */
  NEEDBITS (2);
  gzio->block_type = (unsigned) b & 3;
  DUMPBITS (2);

  /* restore the global bit buffer */
  gzio->bb = b;
  gzio->bk = k;

  switch (gzio->block_type)
    {
    case INFLATE_STORED:
      init_stored_block (gzio);
      break;
    case INFLATE_FIXED:
      init_fixed_block (gzio);
      break;
    case INFLATE_DYNAMIC:
      init_dynamic_block (gzio);
      break;
    default:
      break;
    }
}


static void
inflate_window (grub_gzio_t gzio)
{
  /* initialize window */
  gzio->wp = 0;

  /*
   *  Main decompression loop.
   */

  while (gzio->wp < WSIZE && grub_errno == GRUB_ERR_NONE)
    {
      if (! gzio->block_len)
	{
	  if (gzio->last_block)
	    break;

	  get_new_block (gzio);
	}

      if (gzio->block_type > INFLATE_DYNAMIC)
	grub_error (GRUB_ERR_BAD_COMPRESSED_DATA,
		    "unknown block type %d", gzio->block_type);

      if (grub_errno != GRUB_ERR_NONE)
	return;

      /*
       *  Expand stored block here.
       */
      if (gzio->block_type == INFLATE_STORED)
	{
	  int w = gzio->wp;

	  /*
	   *  This is basically a glorified pass-through
	   */

	  while (gzio->block_len && w < WSIZE && grub_errno == GRUB_ERR_NONE)
	    {
	      gzio->slide[w++] = get_byte (gzio);
	      gzio->block_len--;
	    }

	  gzio->wp = w;

	  continue;
	}

      /*
       *  Expand other kind of block.
       */

      if (inflate_codes_in_window (gzio))
	{
	  huft_free (gzio->tl);
	  huft_free (gzio->td);
	  gzio->tl = 0;
	  gzio->td = 0;
	}
    }

  gzio->saved_offset += gzio->wp;

  /* XXX do CRC calculation here! */
}


static void
initialize_tables (grub_gzio_t gzio)
{
  gzio->saved_offset = 0;
  gzio_seek (gzio, gzio->data_offset);

  /* Initialize the bit buffer.  */
  gzio->bk = 0;
  gzio->bb = 0;

  /* Reset partial decompression code.  */
  gzio->last_block = 0;
  gzio->block_len = 0;

  /* Reset memory allocation stuff.  */
  huft_free (gzio->tl);
  huft_free (gzio->td);
  gzio->tl = NULL;
  gzio->td = NULL;
}


/* Open a new decompressing object on the top of IO. If TRANSPARENT is true,
   even if IO does not contain data compressed by gzip, return a valid file
   object. Note that this function won't close IO, even if an error occurs.  */
static grub_file_t
grub_gzio_open (grub_file_t io, const char *name __attribute__ ((unused)))
{
  grub_file_t file;
  grub_gzio_t gzio = 0;

  file = (grub_file_t) grub_zalloc (sizeof (*file));
  if (! file)
    return 0;

  gzio = grub_zalloc (sizeof (*gzio));
  if (! gzio)
    {
      grub_free (file);
      return 0;
    }

  gzio->file = io;

  file->device = io->device;
  file->data = gzio;
  file->fs = &grub_gzio_fs;
  file->not_easily_seekable = 1;

  if (! test_gzip_header (file))
    {
      grub_errno = GRUB_ERR_NONE;
      grub_free (gzio);
      grub_free (file);
      grub_file_seek (io, 0);

      return io;
    }

  return file;
}

static int
test_zlib_header (grub_gzio_t gzio)
{
  grub_uint8_t cmf, flg;
  
  cmf = get_byte (gzio);
  flg = get_byte (gzio);

  /* Check that compression method is DEFLATE.  */
  if ((cmf & 0xf) != DEFLATED)
    {
      /* TRANSLATORS: It's about given file having some strange format, not
	 complete lack of gzip support.  */
      grub_error (GRUB_ERR_BAD_COMPRESSED_DATA, N_("unsupported gzip format"));
      return 0;
    }

  if ((cmf * 256U + flg) % 31U)
    {
      grub_error (GRUB_ERR_BAD_COMPRESSED_DATA, N_("unsupported gzip format"));
      return 0;
    }

  /* Dictionary isn't supported.  */
  if (flg & 0x20)
    {
      grub_error (GRUB_ERR_BAD_COMPRESSED_DATA, N_("unsupported gzip format"));
      return 0;
    }

  gzio->data_offset = 2;
  initialize_tables (gzio);

  return 1;
}

static grub_ssize_t
grub_gzio_read_real (grub_gzio_t gzio, grub_off_t offset,
		     char *buf, grub_size_t len)
{
  grub_ssize_t ret = 0;

  /* Do we reset decompression to the beginning of the file?  */
  if (gzio->saved_offset > offset + WSIZE)
    initialize_tables (gzio);

  /*
   *  This loop operates upon uncompressed data only.  The only
   *  special thing it does is to make sure the decompression
   *  window is within the range of data it needs.
   */

  while (len > 0 && grub_errno == GRUB_ERR_NONE)
    {
      register grub_size_t size;
      register char *srcaddr;

      while (offset >= gzio->saved_offset)
	{
	  inflate_window (gzio);
	  if (gzio->wp == 0)
	    goto out;
	}

      if (gzio->wp == 0)
	goto out;

      srcaddr = (char *) ((offset & (WSIZE - 1)) + gzio->slide);
      size = gzio->saved_offset - offset;
      if (size > len)
	size = len;

      grub_memmove (buf, srcaddr, size);

      buf += size;
      len -= size;
      ret += size;
      offset += size;
    }

 out:
  if (grub_errno != GRUB_ERR_NONE)
    ret = -1;

  return ret;
}

static grub_ssize_t
grub_gzio_read (grub_file_t file, char *buf, grub_size_t len)
{
  grub_ssize_t ret;
  ret = grub_gzio_read_real (file->data, file->offset, buf, len);

  if (!grub_errno && ret != (grub_ssize_t) len)
    {
      grub_error (GRUB_ERR_BAD_COMPRESSED_DATA, "premature end of compressed");
      ret = -1;
    }
  return ret;
}

/* Release everything, including the underlying file object.  */
static grub_err_t
grub_gzio_close (grub_file_t file)
{
  grub_gzio_t gzio = file->data;

  grub_file_close (gzio->file);
  huft_free (gzio->tl);
  huft_free (gzio->td);
  grub_free (gzio);

  /* No need to close the same device twice.  */
  file->device = 0;
  file->name = 0;

  return grub_errno;
}

grub_ssize_t
grub_zlib_decompress (char *inbuf, grub_size_t insize, grub_off_t off,
		      char *outbuf, grub_size_t outsize)
{
  grub_gzio_t gzio = 0;
  grub_ssize_t ret;

  gzio = grub_zalloc (sizeof (*gzio));
  if (! gzio)
    return -1;
  gzio->mem_input = (grub_uint8_t *) inbuf;
  gzio->mem_input_size = insize;
  gzio->mem_input_off = 0;

  if (!test_zlib_header (gzio))
    {
      grub_free (gzio);
      return -1;
    }

  ret = grub_gzio_read_real (gzio, off, outbuf, outsize);
  grub_free (gzio);

  /* FIXME: Check Adler.  */
  return ret;
}

grub_ssize_t
grub_deflate_decompress (char *inbuf, grub_size_t insize, grub_off_t off,
			 char *outbuf, grub_size_t outsize)
{
  grub_gzio_t gzio = 0;
  grub_ssize_t ret;

  gzio = grub_zalloc (sizeof (*gzio));
  if (! gzio)
    return -1;
  gzio->mem_input = (grub_uint8_t *) inbuf;
  gzio->mem_input_size = insize;
  gzio->mem_input_off = 0;

  initialize_tables (gzio);

  ret = grub_gzio_read_real (gzio, off, outbuf, outsize);
  grub_free (gzio);

  return ret;
}



static struct grub_fs grub_gzio_fs =
  {
    .name = "gzio",
    .dir = 0,
    .open = 0,
    .read = grub_gzio_read,
    .close = grub_gzio_close,
    .label = 0,
    .next = 0
  };

GRUB_MOD_INIT(gzio)
{
  grub_file_filter_register (GRUB_FILE_FILTER_GZIO, grub_gzio_open);
}

GRUB_MOD_FINI(gzio)
{
  grub_file_filter_unregister (GRUB_FILE_FILTER_GZIO);
}
