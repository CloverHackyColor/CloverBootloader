/* ntfscomp.c - compression support for the NTFS filesystem */
/*
 *  Copyright (C) 2007 Free Software Foundation, Inc.
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

#include <grub/file.h>
#include <grub/mm.h>
#include <grub/misc.h>
#include <grub/disk.h>
#include <grub/dl.h>
#include <grub/ntfs.h>

GRUB_MOD_LICENSE ("GPLv3+");

static grub_err_t
decomp_nextvcn (struct grub_ntfs_comp *cc)
{
  if (cc->comp_head >= cc->comp_tail)
    return grub_error (GRUB_ERR_BAD_FS, "compression block overflown");
  if (grub_disk_read
      (cc->disk,
       (cc->comp_table[cc->comp_head].next_lcn -
	(cc->comp_table[cc->comp_head].next_vcn - cc->cbuf_vcn)) << cc->log_spc,
       0,
       1 << (cc->log_spc + GRUB_NTFS_BLK_SHR), cc->cbuf))
    return grub_errno;
  cc->cbuf_vcn++;
  if ((cc->cbuf_vcn >= cc->comp_table[cc->comp_head].next_vcn))
    cc->comp_head++;
  cc->cbuf_ofs = 0;
  return 0;
}

static grub_err_t
decomp_getch (struct grub_ntfs_comp *cc, grub_uint8_t *res)
{
  if (cc->cbuf_ofs >= (1U << (cc->log_spc + GRUB_NTFS_BLK_SHR)))
    {
      if (decomp_nextvcn (cc))
	return grub_errno;
    }
  *res = cc->cbuf[cc->cbuf_ofs++];
  return 0;
}

static grub_err_t
decomp_get16 (struct grub_ntfs_comp *cc, grub_uint16_t * res)
{
  grub_uint8_t c1 = 0, c2 = 0;

  if ((decomp_getch (cc, &c1)) || (decomp_getch (cc, &c2)))
    return grub_errno;
  *res = ((grub_uint16_t) c2) * 256 + ((grub_uint16_t) c1);
  return 0;
}

/* Decompress a block (4096 bytes) */
static grub_err_t
decomp_block (struct grub_ntfs_comp *cc, grub_uint8_t *dest)
{
  grub_uint16_t flg, cnt;

  if (decomp_get16 (cc, &flg))
    return grub_errno;
  cnt = (flg & 0xFFF) + 1;

  if (dest)
    {
      if (flg & 0x8000)
	{
	  grub_uint8_t tag;
	  grub_uint32_t bits, copied;

	  bits = copied = tag = 0;
	  while (cnt > 0)
	    {
	      if (copied > GRUB_NTFS_COM_LEN)
		return grub_error (GRUB_ERR_BAD_FS,
				   "compression block too large");

	      if (!bits)
		{
		  if (decomp_getch (cc, &tag))
		    return grub_errno;

		  bits = 8;
		  cnt--;
		  if (cnt <= 0)
		    break;
		}
	      if (tag & 1)
		{
		  grub_uint32_t i, len, delta, code, lmask, dshift;
		  grub_uint16_t word;

		  if (decomp_get16 (cc, &word))
		    return grub_errno;

		  code = word;
		  cnt -= 2;

		  if (!copied)
		    {
		      grub_error (GRUB_ERR_BAD_FS, "nontext window empty");
		      return 0;
		    }

		  for (i = copied - 1, lmask = 0xFFF, dshift = 12; i >= 0x10;
		       i >>= 1)
		    {
		      lmask >>= 1;
		      dshift--;
		    }

		  delta = code >> dshift;
		  len = (code & lmask) + 3;

		  for (i = 0; i < len; i++)
		    {
		      dest[copied] = dest[copied - delta - 1];
		      copied++;
		    }
		}
	      else
		{
		  grub_uint8_t ch = 0;

		  if (decomp_getch (cc, &ch))
		    return grub_errno;
		  dest[copied++] = ch;
		  cnt--;
		}
	      tag >>= 1;
	      bits--;
	    }
	  return 0;
	}
      else
	{
	  if (cnt != GRUB_NTFS_COM_LEN)
	    return grub_error (GRUB_ERR_BAD_FS,
			       "invalid compression block size");
	}
    }

  while (cnt > 0)
    {
      int n;

      n = (1 << (cc->log_spc + GRUB_NTFS_BLK_SHR)) - cc->cbuf_ofs;
      if (n > cnt)
	n = cnt;
      if ((dest) && (n))
	{
	  grub_memcpy (dest, &cc->cbuf[cc->cbuf_ofs], n);
	  dest += n;
	}
      cnt -= n;
      cc->cbuf_ofs += n;
      if ((cnt) && (decomp_nextvcn (cc)))
	return grub_errno;
    }
  return 0;
}

static grub_err_t
read_block (struct grub_ntfs_rlst *ctx, grub_uint8_t *buf, grub_size_t num)
{
  int log_cpb = GRUB_NTFS_LOG_COM_SEC - ctx->comp.log_spc;

  while (num)
    {
      grub_size_t nn;

      if ((ctx->target_vcn & 0xF) == 0)
	{

	  if (ctx->comp.comp_head != ctx->comp.comp_tail
	      && !(ctx->flags & GRUB_NTFS_RF_BLNK))
	    return grub_error (GRUB_ERR_BAD_FS, "invalid compression block");
	  ctx->comp.comp_head = ctx->comp.comp_tail = 0;
	  ctx->comp.cbuf_vcn = ctx->target_vcn;
	  ctx->comp.cbuf_ofs = (1 << (ctx->comp.log_spc + GRUB_NTFS_BLK_SHR));
	  if (ctx->target_vcn >= ctx->next_vcn)
	    {
	      if (grub_ntfs_read_run_list (ctx))
		return grub_errno;
	    }
	  while (ctx->target_vcn + 16 > ctx->next_vcn)
	    {
	      if (ctx->flags & GRUB_NTFS_RF_BLNK)
		break;
	      ctx->comp.comp_table[ctx->comp.comp_tail].next_vcn = ctx->next_vcn;
	      ctx->comp.comp_table[ctx->comp.comp_tail].next_lcn =
		ctx->curr_lcn + ctx->next_vcn - ctx->curr_vcn;
	      ctx->comp.comp_tail++;
	      if (grub_ntfs_read_run_list (ctx))
		return grub_errno;
	    }
	}

      nn = (16 - (unsigned) (ctx->target_vcn & 0xF)) >> log_cpb;
      if (nn > num)
	nn = num;
      num -= nn;

      if (ctx->flags & GRUB_NTFS_RF_BLNK)
	{
	  ctx->target_vcn += nn << log_cpb;
	  if (ctx->comp.comp_tail == 0)
	    {
	      if (buf)
		{
		  grub_memset (buf, 0, nn * GRUB_NTFS_COM_LEN);
		  buf += nn * GRUB_NTFS_COM_LEN;
		  if (grub_file_progress_hook && ctx->file)
		    grub_file_progress_hook (0, 0, nn * GRUB_NTFS_COM_LEN,
					     ctx->file);
		}
	    }
	  else
	    {
	      while (nn)
		{
		  if (decomp_block (&ctx->comp, buf))
		    return grub_errno;
		  if (buf)
		    buf += GRUB_NTFS_COM_LEN;
		  if (grub_file_progress_hook && ctx->file)
		    grub_file_progress_hook (0, 0, GRUB_NTFS_COM_LEN,
					     ctx->file);
		  nn--;
		}
	    }
	}
      else
	{
	  nn <<= log_cpb;
	  while ((ctx->comp.comp_head < ctx->comp.comp_tail) && (nn))
	    {
	      grub_disk_addr_t tt;

	      tt =
		ctx->comp.comp_table[ctx->comp.comp_head].next_vcn -
		ctx->target_vcn;
	      if (tt > nn)
		tt = nn;
	      ctx->target_vcn += tt;
	      if (buf)
		{
		  if (grub_disk_read
		      (ctx->comp.disk,
		       (ctx->comp.comp_table[ctx->comp.comp_head].next_lcn -
			(ctx->comp.comp_table[ctx->comp.comp_head].next_vcn -
			 ctx->target_vcn)) << ctx->comp.log_spc, 0,
		       tt << (ctx->comp.log_spc + GRUB_NTFS_BLK_SHR), buf))
		    return grub_errno;
		  if (grub_file_progress_hook && ctx->file)
		    grub_file_progress_hook (0, 0,
					     tt << (ctx->comp.log_spc
						    + GRUB_NTFS_BLK_SHR),
					     ctx->file);
		  buf += tt << (ctx->comp.log_spc + GRUB_NTFS_BLK_SHR);
		}
	      nn -= tt;
	      if (ctx->target_vcn >=
		  ctx->comp.comp_table[ctx->comp.comp_head].next_vcn)
		ctx->comp.comp_head++;
	    }
	  if (nn)
	    {
	      if (buf)
		{
		  if (grub_disk_read
		      (ctx->comp.disk,
		       (ctx->target_vcn - ctx->curr_vcn +
			ctx->curr_lcn) << ctx->comp.log_spc, 0,
		       nn << (ctx->comp.log_spc + GRUB_NTFS_BLK_SHR), buf))
		    return grub_errno;
		  buf += nn << (ctx->comp.log_spc + GRUB_NTFS_BLK_SHR);
		  if (grub_file_progress_hook && ctx->file)
		    grub_file_progress_hook (0, 0,
					     nn << (ctx->comp.log_spc
						    + GRUB_NTFS_BLK_SHR),
					     ctx->file);
		}
	      ctx->target_vcn += nn;
	    }
	}
    }
  return 0;
}

static grub_err_t
ntfscomp (grub_uint8_t *dest, grub_disk_addr_t ofs,
	  grub_size_t len, struct grub_ntfs_rlst *ctx)
{
  grub_err_t ret;
  grub_disk_addr_t vcn;

  if (ctx->attr->sbuf)
    {
      if ((ofs & (~(GRUB_NTFS_COM_LEN - 1))) == ctx->attr->save_pos)
	{
	  grub_disk_addr_t n;

	  n = GRUB_NTFS_COM_LEN - (ofs - ctx->attr->save_pos);
	  if (n > len)
	    n = len;

	  grub_memcpy (dest, ctx->attr->sbuf + ofs - ctx->attr->save_pos, n);
	  if (grub_file_progress_hook && ctx->file)
	    grub_file_progress_hook (0, 0, n, ctx->file);
	  if (n == len)
	    return 0;

	  dest += n;
	  len -= n;
	  ofs += n;
	}
    }
  else
    {
      ctx->attr->sbuf = grub_malloc (GRUB_NTFS_COM_LEN);
      if (ctx->attr->sbuf == NULL)
	return grub_errno;
      ctx->attr->save_pos = 1;
    }

  vcn = ctx->target_vcn = (ofs >> GRUB_NTFS_COM_LOG_LEN) * (GRUB_NTFS_COM_SEC >> ctx->comp.log_spc);
  ctx->target_vcn &= ~0xFULL;
  while (ctx->next_vcn <= ctx->target_vcn)
    {
      if (grub_ntfs_read_run_list (ctx))
	return grub_errno;
    }

  ctx->comp.comp_head = ctx->comp.comp_tail = 0;
  ctx->comp.cbuf = grub_malloc (1 << (ctx->comp.log_spc + GRUB_NTFS_BLK_SHR));
  if (!ctx->comp.cbuf)
    return 0;

  ret = 0;

  //ctx->comp.disk->read_hook = read_hook;
  //ctx->comp.disk->read_hook_data = read_hook_data;

  if ((vcn > ctx->target_vcn) &&
      (read_block
       (ctx, NULL, (vcn - ctx->target_vcn) >> (GRUB_NTFS_LOG_COM_SEC - ctx->comp.log_spc))))
    {
      ret = grub_errno;
      goto quit;
    }

  if (ofs % GRUB_NTFS_COM_LEN)
    {
      grub_uint32_t t, n, o;
      void *file = ctx->file;

      ctx->file = 0;

      t = ctx->target_vcn << (ctx->comp.log_spc + GRUB_NTFS_BLK_SHR);
      if (read_block (ctx, ctx->attr->sbuf, 1))
	{
	  ret = grub_errno;
	  goto quit;
	}

      ctx->file = file;

      ctx->attr->save_pos = t;

      o = ofs % GRUB_NTFS_COM_LEN;
      n = GRUB_NTFS_COM_LEN - o;
      if (n > len)
	n = len;
      grub_memcpy (dest, &ctx->attr->sbuf[o], n);
      if (grub_file_progress_hook && ctx->file)
	grub_file_progress_hook (0, 0, n, ctx->file);
      if (n == len)
	goto quit;
      dest += n;
      len -= n;
    }

  if (read_block (ctx, dest, len / GRUB_NTFS_COM_LEN))
    {
      ret = grub_errno;
      goto quit;
    }

  dest += (len / GRUB_NTFS_COM_LEN) * GRUB_NTFS_COM_LEN;
  len = len % GRUB_NTFS_COM_LEN;
  if (len)
    {
      grub_uint32_t t;
      void *file = ctx->file;

      ctx->file = 0;
      t = ctx->target_vcn << (ctx->comp.log_spc + GRUB_NTFS_BLK_SHR);
      if (read_block (ctx, ctx->attr->sbuf, 1))
	{
	  ret = grub_errno;
	  goto quit;
	}

      ctx->attr->save_pos = t;

      grub_memcpy (dest, ctx->attr->sbuf, len);
      if (grub_file_progress_hook && file)
	grub_file_progress_hook (0, 0, len, file);
    }

quit:
  //ctx->comp.disk->read_hook = 0;
  if (ctx->comp.cbuf)
    grub_free (ctx->comp.cbuf);
  return ret;
}

grub_err_t
grub_ntfscomp_func (grub_uint8_t *dest, grub_disk_addr_t ofs,
          grub_size_t len, struct grub_ntfs_rlst *ctx)
{
    return ntfscomp(dest, ofs, len, ctx);
}
