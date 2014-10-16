/* xz_dec_stream.c - .xz Stream decoder */
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
/*
 * This file is based on code from XZ embedded project
 * http://tukaani.org/xz/embedded.html
 */

#include "xz_config.h"
#include "xz_private.h"
#include "xz_stream.h"

#include <grub/crypto.h>

/* Hash used to validate the Index field */
struct xz_dec_hash {
	vli_type unpadded;
	vli_type uncompressed;
#ifndef GRUB_EMBED_DECOMPRESSOR
	uint64_t *hash_context;
#endif
};

/* Enough for up to 512 bits.  */
#define MAX_HASH_SIZE 64

struct xz_dec {
	/* Position in dec_main() */
	enum {
		SEQ_STREAM_HEADER,
		SEQ_BLOCK_START,
		SEQ_BLOCK_HEADER,
		SEQ_BLOCK_UNCOMPRESS,
		SEQ_BLOCK_PADDING,
		SEQ_BLOCK_CHECK,
		SEQ_INDEX,
		SEQ_INDEX_PADDING,
		SEQ_INDEX_CRC32,
		SEQ_STREAM_FOOTER
	} sequence;

	/* Position in variable-length integers and Check fields */
	uint32_t pos;

	/* Variable-length integer decoded by dec_vli() */
	vli_type vli;

	/* Saved in_pos and out_pos */
	size_t in_start;
	size_t out_start;

	/* CRC32 value in Block or Index */
#ifndef GRUB_EMBED_DECOMPRESSOR
	uint8_t hash_value[MAX_HASH_SIZE]; /* need for crc32_validate*/
#endif
	int have_hash_value;
#ifndef GRUB_EMBED_DECOMPRESSOR
	uint64_t *hash_context;
	uint64_t *crc32_context;
#endif

	/* Hash function calculated from uncompressed data */
#ifndef GRUB_EMBED_DECOMPRESSOR
        const gcry_md_spec_t *hash;
        const gcry_md_spec_t *crc32;
	grub_uint8_t hash_id;
#endif
	grub_size_t hash_size;

	/* True if we are operating in single-call mode. */
	bool single_call;

	/*
	 * True if the next call to xz_dec_run() is allowed to return
	 * XZ_BUF_ERROR.
	 */
	bool allow_buf_error;

	/* Information stored in Block Header */
	struct {
		/*
		 * Value stored in the Compressed Size field, or
		 * VLI_UNKNOWN if Compressed Size is not present.
		 */
		vli_type compressed;

		/*
		 * Value stored in the Uncompressed Size field, or
		 * VLI_UNKNOWN if Uncompressed Size is not present.
		 */
		vli_type uncompressed;

		/* Size of the Block Header field */
		uint32_t size;
	} block_header;

	/* Information collected when decoding Blocks */
	struct {
		/* Observed compressed size of the current Block */
		vli_type compressed;

		/* Observed uncompressed size of the current Block */
		vli_type uncompressed;

		/* Number of Blocks decoded so far */
		vli_type count;

		/*
		 * Hash calculated from the Block sizes. This is used to
		 * validate the Index field.
		 */
		struct xz_dec_hash hash;
	} block;

	/* Variables needed when verifying the Index field */
	struct {
		/* Position in dec_index() */
		enum {
			SEQ_INDEX_COUNT,
			SEQ_INDEX_UNPADDED,
			SEQ_INDEX_UNCOMPRESSED
		} sequence;

		/* Size of the Index in bytes */
		vli_type size;

		/* Number of Records (matches block.count in valid files) */
		vli_type count;

		/*
		 * Hash calculated from the Records (matches block.hash in
		 * valid files).
		 */
		struct xz_dec_hash hash;
	} index;

	/*
	 * Temporary buffer needed to hold Stream Header, Block Header,
	 * and Stream Footer. The Block Header is the biggest (1 KiB)
	 * so we reserve space according to that. buf[] has to be aligned
	 * to a multiple of four bytes; the size_t variables before it
	 * should guarantee this.
	 */
	struct {
		size_t pos;
		size_t size;
		uint8_t buf[1024];
	} temp;

	struct xz_dec_lzma2 *lzma2;

#ifdef XZ_DEC_BCJ
	struct xz_dec_bcj *bcj;
	bool bcj_active;
#endif
};

/*
 * Fill s->temp by copying data starting from b->in[b->in_pos]. Caller
 * must have set s->temp.pos to indicate how much data we are supposed
 * to copy into s->temp.buf. Return true once s->temp.pos has reached
 * s->temp.size.
 */
static bool fill_temp(struct xz_dec *s, struct xz_buf *b)
{
	size_t copy_size = min_t(size_t,
			b->in_size - b->in_pos, s->temp.size - s->temp.pos);

	memcpy(s->temp.buf + s->temp.pos, b->in + b->in_pos, copy_size);
	b->in_pos += copy_size;
	s->temp.pos += copy_size;

	if (s->temp.pos == s->temp.size) {
		s->temp.pos = 0;
		return true;
	}

	return false;
}

/* Decode a variable-length integer (little-endian base-128 encoding) */
static enum xz_ret dec_vli(struct xz_dec *s,
		const uint8_t *in, size_t *in_pos, size_t in_size)
{
	uint8_t b;

	if (s->pos == 0)
		s->vli = 0;

	while (*in_pos < in_size) {
		b = in[*in_pos];
		++*in_pos;

		s->vli |= (vli_type)(b & 0x7F) << s->pos;

		if ((b & 0x80) == 0) {
			/* Don't allow non-minimal encodings. */
			if (b == 0 && s->pos != 0)
				return XZ_DATA_ERROR;

			s->pos = 0;
			return XZ_STREAM_END;
		}

		s->pos += 7;
		if (s->pos == 7 * VLI_BYTES_MAX)
			return XZ_DATA_ERROR;
	}

	return XZ_OK;
}

/*
 * Decode the Compressed Data field from a Block. Update and validate
 * the observed compressed and uncompressed sizes of the Block so that
 * they don't exceed the values possibly stored in the Block Header
 * (validation assumes that no integer overflow occurs, since vli_type
 * is normally uint64_t). Update the CRC32 if presence of the CRC32
 * field was indicated in Stream Header.
 *
 * Once the decoding is finished, validate that the observed sizes match
 * the sizes possibly stored in the Block Header. Update the hash and
 * Block count, which are later used to validate the Index field.
 */
static enum xz_ret dec_block(struct xz_dec *s, struct xz_buf *b)
{
	enum xz_ret ret;

	s->in_start = b->in_pos;
	s->out_start = b->out_pos;

#ifdef XZ_DEC_BCJ
	if (s->bcj_active)
		ret = xz_dec_bcj_run(s->bcj, s->lzma2, b);
	else
#endif
		ret = xz_dec_lzma2_run(s->lzma2, b);

	s->block.compressed += b->in_pos - s->in_start;
	s->block.uncompressed += b->out_pos - s->out_start;

	/*
	 * There is no need to separately check for VLI_UNKNOWN, since
	 * the observed sizes are always smaller than VLI_UNKNOWN.
	 */
	if (s->block.compressed > s->block_header.compressed
			|| s->block.uncompressed
				> s->block_header.uncompressed)
		return XZ_DATA_ERROR;

#ifndef GRUB_EMBED_DECOMPRESSOR
	if (s->hash)
	  s->hash->write(s->hash_context,b->out + s->out_start,
			 b->out_pos - s->out_start);
	if (s->crc32)
	  s->crc32->write(s->crc32_context,b->out + s->out_start,
			  b->out_pos - s->out_start);
#endif

	if (ret == XZ_STREAM_END) {
		if (s->block_header.compressed != VLI_UNKNOWN
				&& s->block_header.compressed
					!= s->block.compressed)
			return XZ_DATA_ERROR;

		if (s->block_header.uncompressed != VLI_UNKNOWN
				&& s->block_header.uncompressed
					!= s->block.uncompressed)
			return XZ_DATA_ERROR;

		s->block.hash.unpadded += s->block_header.size
				+ s->block.compressed;
		s->block.hash.unpadded += s->hash_size;

		s->block.hash.uncompressed += s->block.uncompressed;

#ifndef GRUB_EMBED_DECOMPRESSOR
		if (s->hash)
			s->hash->write(s->block.hash.hash_context,
				       (const uint8_t *)&s->block.hash,
				       2 * sizeof(vli_type));
#endif

		++s->block.count;
	}

	return ret;
}

/* Update the Index size and the CRC32 value. */
static void index_update(struct xz_dec *s, const struct xz_buf *b)
{
	size_t in_used = b->in_pos - s->in_start;
	s->index.size += in_used;
#ifndef GRUB_EMBED_DECOMPRESSOR
	if (s->hash)
		s->hash->write(s->hash_context,b->in + s->in_start, in_used);
	if (s->crc32)
		s->crc32->write(s->crc32_context,b->in + s->in_start, in_used);
#endif
}

/*
 * Decode the Number of Records, Unpadded Size, and Uncompressed Size
 * fields from the Index field. That is, Index Padding and CRC32 are not
 * decoded by this function.
 *
 * This can return XZ_OK (more input needed), XZ_STREAM_END (everything
 * successfully decoded), or XZ_DATA_ERROR (input is corrupt).
 */
static enum xz_ret dec_index(struct xz_dec *s, struct xz_buf *b)
{
	enum xz_ret ret;

	do {
		ret = dec_vli(s, b->in, &b->in_pos, b->in_size);
		if (ret != XZ_STREAM_END) {
			index_update(s, b);
			return ret;
		}

		switch (s->index.sequence) {
		case SEQ_INDEX_COUNT:
			s->index.count = s->vli;

			/*
			 * Validate that the Number of Records field
			 * indicates the same number of Records as
			 * there were Blocks in the Stream.
			 */
			if (s->index.count != s->block.count)
				return XZ_DATA_ERROR;

			s->index.sequence = SEQ_INDEX_UNPADDED;
			break;

		case SEQ_INDEX_UNPADDED:
			s->index.hash.unpadded += s->vli;
			s->index.sequence = SEQ_INDEX_UNCOMPRESSED;
			break;

		case SEQ_INDEX_UNCOMPRESSED:
			s->index.hash.uncompressed += s->vli;

#ifndef GRUB_EMBED_DECOMPRESSOR
			if (s->hash)
				s->hash->write(s->index.hash.hash_context,
					       (const uint8_t *)&s->index.hash, 2 * sizeof(vli_type));
#endif

			--s->index.count;
			s->index.sequence = SEQ_INDEX_UNPADDED;
			break;
		}
	} while (s->index.count > 0);

	return XZ_STREAM_END;
}

/*
 * Validate that the next four input bytes match the value of s->crc32.
 * s->pos must be zero when starting to validate the first byte.
 */
static enum xz_ret hash_validate(struct xz_dec *s, struct xz_buf *b,
	int crc32)
{
#ifndef GRUB_EMBED_DECOMPRESSOR
	const gcry_md_spec_t *hash = crc32 ? s->crc32 : s->hash;
	void *hash_context = crc32 ? s->crc32_context 
		: s->hash_context;
	if(!s->have_hash_value && hash
		&& sizeof (s->hash_value) >= hash->mdlen)
	{
		hash->final(hash_context);
		grub_memcpy (s->hash_value, hash->read(hash_context),
			     hash->mdlen);
		s->have_hash_value = 1;
		if (s->hash_id == 1 || crc32)
		{
			grub_uint8_t t;
			t = s->hash_value[0];
			s->hash_value[0] = s->hash_value[3];
			s->hash_value[3] = t;
			t = s->hash_value[1];
			s->hash_value[1] = s->hash_value[2];
			s->hash_value[2] = t;
		}
	}
#endif

	if (b->in_pos == b->in_size)
		return XZ_OK;

	if (!crc32 && s->hash_size == 0)
		s->pos += 8;

	while (s->pos < (crc32 ? 32 : s->hash_size * 8)) {
		if (b->in_pos == b->in_size)
			return XZ_OK;

#ifndef GRUB_EMBED_DECOMPRESSOR
		if (hash && s->hash_value[s->pos / 8] != b->in[b->in_pos])
			return XZ_DATA_ERROR;
#endif
		b->in_pos++;

		s->pos += 8;

	}

#ifndef GRUB_EMBED_DECOMPRESSOR
	if (s->hash)
		s->hash->init(s->hash_context);
	if (s->crc32)
		s->crc32->init(s->crc32_context);
#endif
	s->have_hash_value = 0;
	s->pos = 0;

	return XZ_STREAM_END;
}

static const struct
{
	const char *name;
	grub_size_t size;
} hashes[] = {
	[0x01] = { "CRC32", 4},
	[0x04] = { "CRC64", 8},
	[0x0A] = { "SHA256", 32},
};

/* Decode the Stream Header field (the first 12 bytes of the .xz Stream). */
static enum xz_ret dec_stream_header(struct xz_dec *s)
{
	if (! memeq(s->temp.buf, HEADER_MAGIC, HEADER_MAGIC_SIZE))
		return XZ_FORMAT_ERROR;

#ifndef GRUB_EMBED_DECOMPRESSOR
	s->crc32 = grub_crypto_lookup_md_by_name ("CRC32");

	if (s->crc32)
	{
		uint8_t readhash[4];
		uint8_t computed_hash[4];

		if(4 != s->crc32->mdlen)
		  return XZ_DATA_ERROR;

		grub_crypto_hash (s->crc32, computed_hash,
				  s->temp.buf + HEADER_MAGIC_SIZE, 2);

		readhash[0] = s->temp.buf[HEADER_MAGIC_SIZE + 5];
		readhash[1] = s->temp.buf[HEADER_MAGIC_SIZE + 4];
		readhash[2] = s->temp.buf[HEADER_MAGIC_SIZE + 3];
		readhash[3] = s->temp.buf[HEADER_MAGIC_SIZE + 2];

		if (grub_memcmp (readhash, computed_hash,
				 s->crc32->mdlen) != 0)
		  return XZ_DATA_ERROR;
	}
#endif

#ifndef GRUB_EMBED_DECOMPRESSOR
	/*
	 * Decode the Stream Flags field.
	 */
	if (s->temp.buf[HEADER_MAGIC_SIZE] != 0
	    || s->temp.buf[HEADER_MAGIC_SIZE + 1] >= ARRAY_SIZE (hashes)
	    || (hashes[s->temp.buf[HEADER_MAGIC_SIZE + 1]].name == 0
		&& s->temp.buf[HEADER_MAGIC_SIZE + 1] != 0))
		return XZ_OPTIONS_ERROR;

	s->hash_id = s->temp.buf[HEADER_MAGIC_SIZE + 1];

	if (s->crc32)
	{
		s->crc32_context = kmalloc(s->crc32->contextsize, GFP_KERNEL);
		if (s->crc32_context == NULL)
			return XZ_MEMLIMIT_ERROR;
		s->crc32->init(s->crc32_context);
	}
#endif

	if (s->temp.buf[HEADER_MAGIC_SIZE + 1])
	{
		s->hash_size = hashes[s->temp.buf[HEADER_MAGIC_SIZE + 1]].size;
#ifndef GRUB_EMBED_DECOMPRESSOR
		s->hash = grub_crypto_lookup_md_by_name (hashes[s->temp.buf[HEADER_MAGIC_SIZE + 1]].name);
		if (s->hash)
		{
			if (s->hash->mdlen != s->hash_size)
				return XZ_OPTIONS_ERROR;
			s->hash_context = kmalloc(s->hash->contextsize, GFP_KERNEL);
			if (s->hash_context == NULL)
			{
				kfree(s->crc32_context);
				return XZ_MEMLIMIT_ERROR;
			}
			
			s->index.hash.hash_context = kmalloc(s->hash->contextsize,
							     GFP_KERNEL);
			if (s->index.hash.hash_context == NULL)
			{
				kfree(s->hash_context);
				kfree(s->crc32_context);
				return XZ_MEMLIMIT_ERROR;
			}
			
			s->block.hash.hash_context = kmalloc(s->hash->contextsize, GFP_KERNEL);
			if (s->block.hash.hash_context == NULL)
			{
				kfree(s->index.hash.hash_context);
				kfree(s->hash_context);
				kfree(s->crc32_context);
				return XZ_MEMLIMIT_ERROR;
			}

			s->hash->init(s->hash_context);
			s->hash->init(s->index.hash.hash_context);
 			s->hash->init(s->block.hash.hash_context);
		}
#endif
	}
	else
	{
#ifndef GRUB_EMBED_DECOMPRESSOR
		s->hash = 0;
#endif
		s->hash_size = 0;
	}

	s->have_hash_value = 0;


	return XZ_OK;
}

/* Decode the Stream Footer field (the last 12 bytes of the .xz Stream) */
static enum xz_ret dec_stream_footer(struct xz_dec *s)
{
	if (! memeq(s->temp.buf + 10, FOOTER_MAGIC, FOOTER_MAGIC_SIZE))
		return XZ_DATA_ERROR;

#ifndef GRUB_EMBED_DECOMPRESSOR
	if (s->crc32)
	{
		uint8_t readhash[4];
		uint8_t computed_hash[4];

		if (4 != s->crc32->mdlen)
		  return XZ_DATA_ERROR;

		grub_crypto_hash (s->crc32, computed_hash,
				  s->temp.buf + 4, 6);

		readhash[0] = s->temp.buf[3];
		readhash[1] = s->temp.buf[2];
		readhash[2] = s->temp.buf[1];
		readhash[3] = s->temp.buf[0];

		if(grub_memcmp (readhash, computed_hash,
				s->crc32->mdlen) != 0)
			return XZ_DATA_ERROR;
	}
#endif


	/*
	 * Validate Backward Size. Note that we never added the size of the
	 * Index CRC32 field to s->index.size, thus we use s->index.size / 4
	 * instead of s->index.size / 4 - 1.
	 */
	if ((s->index.size >> 2) != get_le32(s->temp.buf + 4))
		return XZ_DATA_ERROR;

#ifndef GRUB_EMBED_DECOMPRESSOR
	if (s->temp.buf[8] != 0 || s->temp.buf[9] != s->hash_id)
		return XZ_DATA_ERROR;
#endif

	/*
	 * Use XZ_STREAM_END instead of XZ_OK to be more convenient
	 * for the caller.
	 */
	return XZ_STREAM_END;
}

/* Decode the Block Header and initialize the filter chain. */
static enum xz_ret dec_block_header(struct xz_dec *s)
{
	enum xz_ret ret;

	/*
	 * Validate the CRC32. We know that the temp buffer is at least
	 * eight bytes so this is safe.
	 */
	s->temp.size -= 4;
#ifndef GRUB_EMBED_DECOMPRESSOR
	if (s->crc32)
	{
		uint8_t readhash[4], computed_hash[4];

		if(4 != s->crc32->mdlen)
			return XZ_DATA_ERROR;

		grub_crypto_hash (s->crc32, computed_hash,
				  s->temp.buf, s->temp.size);

		readhash[3] = s->temp.buf[s->temp.size];
		readhash[2] = s->temp.buf[s->temp.size + 1];
		readhash[1] = s->temp.buf[s->temp.size + 2];
		readhash[0] = s->temp.buf[s->temp.size + 3];

		if(grub_memcmp (readhash, computed_hash,
				s->crc32->mdlen) != 0)
			return XZ_DATA_ERROR;
	}
#endif

	s->temp.pos = 2;

	/*
	 * Catch unsupported Block Flags. We support only one or two filters
	 * in the chain, so we catch that with the same test.
	 */
#ifdef XZ_DEC_BCJ
	if (s->temp.buf[1] & 0x3E)
#else
	if (s->temp.buf[1] & 0x3F)
#endif
		return XZ_OPTIONS_ERROR;

	/* Compressed Size */
	if (s->temp.buf[1] & 0x40) {
		if (dec_vli(s, s->temp.buf, &s->temp.pos, s->temp.size)
					!= XZ_STREAM_END)
			return XZ_DATA_ERROR;

		s->block_header.compressed = s->vli;
	} else {
		s->block_header.compressed = VLI_UNKNOWN;
	}

	/* Uncompressed Size */
	if (s->temp.buf[1] & 0x80) {
		if (dec_vli(s, s->temp.buf, &s->temp.pos, s->temp.size)
				!= XZ_STREAM_END)
			return XZ_DATA_ERROR;

		s->block_header.uncompressed = s->vli;
	} else {
		s->block_header.uncompressed = VLI_UNKNOWN;
	}

#ifdef XZ_DEC_BCJ
	/* If there are two filters, the first one must be a BCJ filter. */
	s->bcj_active = s->temp.buf[1] & 0x01;
	if (s->bcj_active) {
		if (s->temp.size - s->temp.pos < 2)
			return XZ_OPTIONS_ERROR;

		ret = xz_dec_bcj_reset(s->bcj, s->temp.buf[s->temp.pos++]);
		if (ret != XZ_OK)
			return ret;

		/*
		 * We don't support custom start offset,
		 * so Size of Properties must be zero.
		 */
		if (s->temp.buf[s->temp.pos++] != 0x00)
			return XZ_OPTIONS_ERROR;
	}
#endif

	/* Valid Filter Flags always take at least two bytes. */
	if (s->temp.size - s->temp.pos < 2)
		return XZ_DATA_ERROR;

	/* Filter ID = LZMA2 */
	if (s->temp.buf[s->temp.pos++] != 0x21)
		return XZ_OPTIONS_ERROR;

	/* Size of Properties = 1-byte Filter Properties */
	if (s->temp.buf[s->temp.pos++] != 0x01)
		return XZ_OPTIONS_ERROR;

	/* Filter Properties contains LZMA2 dictionary size. */
	if (s->temp.size - s->temp.pos < 1)
		return XZ_DATA_ERROR;

	ret = xz_dec_lzma2_reset(s->lzma2, s->temp.buf[s->temp.pos++]);
	if (ret != XZ_OK)
		return ret;

	/* The rest must be Header Padding. */
	while (s->temp.pos < s->temp.size)
		if (s->temp.buf[s->temp.pos++] != 0x00)
			return XZ_OPTIONS_ERROR;

	s->temp.pos = 0;
	s->block.compressed = 0;
	s->block.uncompressed = 0;

	return XZ_OK;
}

static enum xz_ret dec_main(struct xz_dec *s, struct xz_buf *b)
{
	enum xz_ret ret;

	/*
	 * Store the start position for the case when we are in the middle
	 * of the Index field.
	 */
	s->in_start = b->in_pos;

	while (true) {
		switch (s->sequence) {
		case SEQ_STREAM_HEADER:
			/*
			 * Stream Header is copied to s->temp, and then
			 * decoded from there. This way if the caller
			 * gives us only little input at a time, we can
			 * still keep the Stream Header decoding code
			 * simple. Similar approach is used in many places
			 * in this file.
			 */
			if (!fill_temp(s, b))
				return XZ_OK;

			ret = dec_stream_header(s);
			if (ret != XZ_OK)
				return ret;

			s->sequence = SEQ_BLOCK_START;

		case SEQ_BLOCK_START:
			/* We need one byte of input to continue. */
			if (b->in_pos == b->in_size)
				return XZ_OK;

			/* See if this is the beginning of the Index field. */
			if (b->in[b->in_pos] == 0) {
				s->in_start = b->in_pos++;
				s->sequence = SEQ_INDEX;
				break;
			}

			/*
			 * Calculate the size of the Block Header and
			 * prepare to decode it.
			 */
			s->block_header.size
				= ((uint32_t)b->in[b->in_pos] + 1) * 4;

			s->temp.size = s->block_header.size;
			s->temp.pos = 0;
			s->sequence = SEQ_BLOCK_HEADER;

		case SEQ_BLOCK_HEADER:
			if (!fill_temp(s, b))
				return XZ_OK;

			ret = dec_block_header(s);
			if (ret != XZ_OK)
				return ret;

			s->sequence = SEQ_BLOCK_UNCOMPRESS;

		case SEQ_BLOCK_UNCOMPRESS:
			ret = dec_block(s, b);
			if (ret != XZ_STREAM_END)
				return ret;

			s->sequence = SEQ_BLOCK_PADDING;

		case SEQ_BLOCK_PADDING:
			/*
			 * Size of Compressed Data + Block Padding
			 * must be a multiple of four. We don't need
			 * s->block.compressed for anything else
			 * anymore, so we use it here to test the size
			 * of the Block Padding field.
			 */
			while (s->block.compressed & 3) {
				if (b->in_pos == b->in_size)
					return XZ_OK;

				if (b->in[b->in_pos++] != 0)
					return XZ_DATA_ERROR;

				++s->block.compressed;
			}

			s->sequence = SEQ_BLOCK_CHECK;

		case SEQ_BLOCK_CHECK:
			ret = hash_validate(s, b, 0);
			if (ret != XZ_STREAM_END)
				return ret;

			s->sequence = SEQ_BLOCK_START;
			break;

		case SEQ_INDEX:
			ret = dec_index(s, b);
			if (ret != XZ_STREAM_END)
				return ret;

			s->sequence = SEQ_INDEX_PADDING;

		case SEQ_INDEX_PADDING:
			while ((s->index.size + (b->in_pos - s->in_start))
					& 3) {
				if (b->in_pos == b->in_size) {
					index_update(s, b);
					return XZ_OK;
				}

				if (b->in[b->in_pos++] != 0)
					return XZ_DATA_ERROR;
			}

			/* Finish the CRC32 value and Index size. */
			index_update(s, b);

#ifndef GRUB_EMBED_DECOMPRESSOR
			if (s->hash)
			{
				/* Compare the hashes to validate the Index field. */
				s->hash->final(s->block.hash.hash_context);
				s->hash->final(s->index.hash.hash_context);

				if (s->block.hash.unpadded != s->index.hash.unpadded
				    || s->block.hash.uncompressed != s->index.hash.uncompressed
				    || grub_memcmp (s->hash->read(s->block.hash.hash_context),
						    s->hash->read(s->index.hash.hash_context),
						    s->hash->mdlen) != 0)
					return XZ_DATA_ERROR;
			}
#endif

			s->sequence = SEQ_INDEX_CRC32;

		case SEQ_INDEX_CRC32:
			ret = hash_validate(s, b, 1);
			if (ret != XZ_STREAM_END)
				return ret;

			s->temp.size = STREAM_HEADER_SIZE;
			s->sequence = SEQ_STREAM_FOOTER;

		case SEQ_STREAM_FOOTER:
			if (!fill_temp(s, b))
				return XZ_OK;

			return dec_stream_footer(s);
		}
	}

	/* Never reached */
}

/*
 * xz_dec_run() is a wrapper for dec_main() to handle some special cases in
 * multi-call and single-call decoding.
 *
 * In multi-call mode, we must return XZ_BUF_ERROR when it seems clear that we
 * are not going to make any progress anymore. This is to prevent the caller
 * from calling us infinitely when the input file is truncated or otherwise
 * corrupt. Since zlib-style API allows that the caller fills the input buffer
 * only when the decoder doesn't produce any new output, we have to be careful
 * to avoid returning XZ_BUF_ERROR too easily: XZ_BUF_ERROR is returned only
 * after the second consecutive call to xz_dec_run() that makes no progress.
 *
 * In single-call mode, if we couldn't decode everything and no error
 * occurred, either the input is truncated or the output buffer is too small.
 * Since we know that the last input byte never produces any output, we know
 * that if all the input was consumed and decoding wasn't finished, the file
 * must be corrupt. Otherwise the output buffer has to be too small or the
 * file is corrupt in a way that decoding it produces too big output.
 *
 * If single-call decoding fails, we reset b->in_pos and b->out_pos back to
 * their original values. This is because with some filter chains there won't
 * be any valid uncompressed data in the output buffer unless the decoding
 * actually succeeds (that's the price to pay of using the output buffer as
 * the workspace).
 */
enum xz_ret xz_dec_run(struct xz_dec *s, struct xz_buf *b)
{
	size_t in_start;
	size_t out_start;
	enum xz_ret ret;

	if (s->single_call)
		xz_dec_reset(s);

	in_start = b->in_pos;
	out_start = b->out_pos;
	ret = dec_main(s, b);

	if (s->single_call) {
		if (ret == XZ_OK)
			ret = b->in_pos == b->in_size
					? XZ_DATA_ERROR : XZ_BUF_ERROR;

		if (ret != XZ_STREAM_END) {
			b->in_pos = in_start;
			b->out_pos = out_start;
		}

	} else if (ret == XZ_OK && in_start == b->in_pos
			&& out_start == b->out_pos) {
		if (s->allow_buf_error)
			ret = XZ_BUF_ERROR;

		s->allow_buf_error = true;
	} else {
		s->allow_buf_error = false;
	}

	return ret;
}

#ifdef GRUB_EMBED_DECOMPRESSOR
struct xz_dec decoder;
#endif

struct xz_dec * xz_dec_init(uint32_t dict_max)
{
	struct xz_dec *s;
#ifdef GRUB_EMBED_DECOMPRESSOR
	s = &decoder;
#else
	s = kmalloc(sizeof(*s), GFP_KERNEL);
	if (s == NULL)
		return NULL;
#endif

	memset (s, 0, sizeof (*s));

	s->single_call = dict_max == 0;

#ifdef XZ_DEC_BCJ
	s->bcj = xz_dec_bcj_create(s->single_call);
	if (s->bcj == NULL)
		goto error_bcj;
#endif

	s->lzma2 = xz_dec_lzma2_create(dict_max);
	if (s->lzma2 == NULL)
		goto error_lzma2;

	xz_dec_reset(s);
	return s;

error_lzma2:
#ifdef XZ_DEC_BCJ
	xz_dec_bcj_end(s->bcj);
error_bcj:
#endif
#ifndef GRUB_EMBED_DECOMPRESSOR
	kfree(s);
#endif
	return NULL;
}

void xz_dec_reset(struct xz_dec *s)
{
	s->sequence = SEQ_STREAM_HEADER;
	s->allow_buf_error = false;
	s->pos = 0;

	{
#ifndef GRUB_EMBED_DECOMPRESSOR
		uint64_t *t;
		t = s->block.hash.hash_context;
#endif
		memzero(&s->block, sizeof(s->block));
#ifndef GRUB_EMBED_DECOMPRESSOR
		s->block.hash.hash_context = t;
		t = s->index.hash.hash_context;
#endif
		memzero(&s->index, sizeof(s->index));
#ifndef GRUB_EMBED_DECOMPRESSOR
		s->index.hash.hash_context = t;
#endif
	}
	s->temp.pos = 0;
	s->temp.size = STREAM_HEADER_SIZE;

#ifndef GRUB_EMBED_DECOMPRESSOR
	if (s->hash)
	{
		s->hash->init(s->hash_context);
		s->hash->init(s->index.hash.hash_context);
		s->hash->init(s->block.hash.hash_context);
	}
#endif
	s->have_hash_value = 0;
}

void xz_dec_end(struct xz_dec *s)
{
	if (s != NULL) {
		xz_dec_lzma2_end(s->lzma2);
#ifndef GRUB_EMBED_DECOMPRESSOR
		kfree(s->index.hash.hash_context);
		kfree(s->block.hash.hash_context);
		kfree(s->hash_context);
		kfree(s->crc32_context);
#endif
#ifdef XZ_DEC_BCJ
		xz_dec_bcj_end(s->bcj);
#endif
#ifndef GRUB_EMBED_DECOMPRESSOR
		kfree(s);
#endif
	}
}
