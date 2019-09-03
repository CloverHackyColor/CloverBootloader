/* xz.h - XZ decompressor */
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

#ifndef XZ_H
#define XZ_H

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include <grub/misc.h>

#ifndef GRUB_POSIX_BOOL_DEFINED
typedef enum { false = 0, true = 1 } bool;
#endif

/**
 * enum xz_ret - Return codes
 * @XZ_OK:              Everything is OK so far. More input or more output
 *                      space is required to continue.
 * @XZ_STREAM_END:      Operation finished successfully.
 * @XZ_MEMLIMIT_ERROR:  Not enough memory was preallocated at decoder
 *                      initialization time.
 * @XZ_FORMAT_ERROR:    File format was not recognized (wrong magic bytes).
 * @XZ_OPTIONS_ERROR:   This implementation doesn't support the requested
 *                      compression options. In the decoder this means that
 *                      the header CRC32 matches, but the header itself
 *                      specifies something that we don't support.
 * @XZ_DATA_ERROR:      Compressed data is corrupt.
 * @XZ_BUF_ERROR:       Cannot make any progress. Details are slightly
 *                      different between multi-call and single-call mode;
 *                      more information below.
 *
 * In multi-call mode, XZ_BUF_ERROR is returned when two consecutive calls
 * to XZ code cannot consume any input and cannot produce any new output.
 * This happens when there is no new input available, or the output buffer
 * is full while at least one output byte is still pending. Assuming your
 * code is not buggy, you can get this error only when decoding a compressed
 * stream that is truncated or otherwise corrupt.
 *
 * In single-call mode, XZ_BUF_ERROR is returned only when the output buffer
 * is too small, or the compressed input is corrupt in a way that makes the
 * decoder produce more output than the caller expected. When it is
 * (relatively) clear that the compressed input is truncated, XZ_DATA_ERROR
 * is used instead of XZ_BUF_ERROR.
 */
enum xz_ret {
	XZ_OK,
	XZ_STREAM_END,
	XZ_MEMLIMIT_ERROR,
	XZ_FORMAT_ERROR,
	XZ_OPTIONS_ERROR,
	XZ_DATA_ERROR,
	XZ_BUF_ERROR
};

/**
 * struct xz_buf - Passing input and output buffers to XZ code
 * @in:         Beginning of the input buffer. This may be NULL if and only
 *              if in_pos is equal to in_size.
 * @in_pos:     Current position in the input buffer. This must not exceed
 *              in_size.
 * @in_size:    Size of the input buffer
 * @out:        Beginning of the output buffer. This may be NULL if and only
 *              if out_pos is equal to out_size.
 * @out_pos:    Current position in the output buffer. This must not exceed
 *              out_size.
 * @out_size:   Size of the output buffer
 *
 * Only the contents of the output buffer from out[out_pos] onward, and
 * the variables in_pos and out_pos are modified by the XZ code.
 */
struct xz_buf {
	const uint8_t *in;
	size_t in_pos;
	size_t in_size;

	uint8_t *out;
	size_t out_pos;
	size_t out_size;
};

/**
 * struct xz_dec - Opaque type to hold the XZ decoder state
 */
struct xz_dec;

/**
 * xz_dec_init() - Allocate and initialize a XZ decoder state
 * @dict_max:   Maximum size of the LZMA2 dictionary (history buffer) for
 *              multi-call decoding, or special value of zero to indicate
 *              single-call decoding mode.
 *
 * If dict_max > 0, the decoder is initialized to work in multi-call mode.
 * dict_max number of bytes of memory is preallocated for the LZMA2
 * dictionary. This way there is no risk that xz_dec_run() could run out
 * of memory, since xz_dec_run() will never allocate any memory. Instead,
 * if the preallocated dictionary is too small for decoding the given input
 * stream, xz_dec_run() will return XZ_MEMLIMIT_ERROR. Thus, it is important
 * to know what kind of data will be decoded to avoid allocating excessive
 * amount of memory for the dictionary.
 *
 * LZMA2 dictionary is always 2^n bytes or 2^n + 2^(n-1) bytes (the latter
 * sizes are less common in practice). In the kernel, dictionary sizes of
 * 64 KiB, 128 KiB, 256 KiB, 512 KiB, and 1 MiB are probably the only
 * reasonable values.
 *
 * If dict_max == 0, the decoder is initialized to work in single-call mode.
 * In single-call mode, xz_dec_run() decodes the whole stream at once. The
 * caller must provide enough output space or the decoding will fail. The
 * output space is used as the dictionary buffer, which is why there is
 * no need to allocate the dictionary as part of the decoder's internal
 * state.
 *
 * Because the output buffer is used as the workspace, streams encoded using
 * a big dictionary are not a problem in single-call. It is enough that the
 * output buffer is is big enough to hold the actual uncompressed data; it
 * can be smaller than the dictionary size stored in the stream headers.
 *
 * On success, xz_dec_init() returns a pointer to struct xz_dec, which is
 * ready to be used with xz_dec_run(). On error, xz_dec_init() returns NULL.
 */
struct xz_dec * xz_dec_init(uint32_t dict_max);

/**
 * xz_dec_run() - Run the XZ decoder
 * @s:          Decoder state allocated using xz_dec_init()
 * @b:          Input and output buffers
 *
 * In multi-call mode, this function may return any of the values listed in
 * enum xz_ret.
 *
 * In single-call mode, this function never returns XZ_OK. If an error occurs
 * in single-call mode (return value is not XZ_STREAM_END), b->in_pos and
 * b->out_pos are not modified, and the contents of the output buffer from
 * b->out[b->out_pos] onward are undefined.
 *
 * NOTE: In single-call mode, the contents of the output buffer are undefined
 * also after XZ_BUF_ERROR. This is because with some filter chains, there
 * may be a second pass over the output buffer, and this pass cannot be
 * properly done if the output buffer is truncated. Thus, you cannot give
 * the single-call decoder a too small buffer and then expect to get that
 * amount valid data from the beginning of the stream. You must use the
 * multi-call decoder if you don't want to uncompress the whole stream.
 */
enum xz_ret xz_dec_run(struct xz_dec *s, struct xz_buf *b);

/**
 * xz_dec_reset() - Reset an already allocated decoder state
 * @s:          Decoder state allocated using xz_dec_init()
 *
 * This function can be used to reset the multi-call decoder state without
 * freeing and reallocating memory with xz_dec_end() and xz_dec_init().
 *
 * In single-call mode, xz_dec_reset() is always called in the beginning of
 * xz_dec_run(). Thus, explicit call to xz_dec_reset() is useful only in
 * multi-call mode.
 */
void xz_dec_reset(struct xz_dec *s);

/**
 * xz_dec_end() - Free the memory allocated for the decoder state
 * @s:          Decoder state allocated using xz_dec_init(). If s is NULL,
 *              this function does nothing.
 */
void xz_dec_end(struct xz_dec *s);

#endif
