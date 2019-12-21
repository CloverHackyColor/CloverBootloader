/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#if !defined(LODEPNG)

#ifndef __GNUC__
#pragma optimize("g", off)
#endif

#define TEST 0
#include "picopng.h"

#ifndef DEBUG_ALL
#define DEBUG_PNG 0
#else
#define DEBUG_PNG DEBUG_ALL
#endif

#if DEBUG_PNG == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_PNG, __VA_ARGS__)
#endif



CONST
UINT32 pattern[28] = {0, 4, 0, 2, 0, 1, 0,
  0, 0, 4, 0, 2, 0, 1,
  8, 8, 4, 4, 2, 2, 1,
  8, 8,	8, 4, 4, 2, 2 }; // values for the adam7 passes


/*************************************************************************************************/

typedef struct png_alloc_node
{
	struct png_alloc_node *prev, *next;
	void *addr;
	UINT32 size;
} png_alloc_node_t;

png_alloc_node_t *png_alloc_head = NULL;
png_alloc_node_t *png_alloc_tail = NULL;

png_alloc_node_t *png_alloc_find_node(void *addr, UINT32 size)
{
	png_alloc_node_t *node;
	for (node = png_alloc_head; node; node = node->next)
		if (node->addr == addr){
      if (size && (node->size != size)) {
        //       Print(L"Error: Same address %p, size: %x, requested size: %x\n", node->addr, node->size, size);
      }
			break;
    }
	return node;
}

void png_alloc_add_node(void *addr, UINT32 size)
{
	png_alloc_node_t *node;
	if (png_alloc_find_node(addr, size))
		return;
	node = (__typeof__(node))AllocateZeroPool(sizeof (png_alloc_node_t));
	node->addr = addr;
	node->size = size;
	node->prev = png_alloc_tail;
	node->next = NULL;
	png_alloc_tail = node;
	if (node->prev)
		node->prev->next = node;
	if (!png_alloc_head)
		png_alloc_head = node;
}

void png_alloc_remove_node(png_alloc_node_t *node)
{
	if (node->prev)
		node->prev->next = node->next;
	if (node->next)
		node->next->prev = node->prev;
	if (node == png_alloc_head)
		png_alloc_head = node->next;
	if (node == png_alloc_tail)
		png_alloc_tail = node->prev;
	node->prev = node->next = node->addr = NULL;
	FreePool(node);
}

void *png_alloc_malloc(UINT32 size)
{
	void *addr = (__typeof__(addr))AllocateZeroPool(size);
	png_alloc_add_node(addr, size);
	return addr;
}

void *png_alloc_realloc(void *addr, UINT32 oldSize, UINT32 newSize)
{
  if (!addr) {
		return png_alloc_malloc(newSize);
  }
  if ( newSize > oldSize ) {
  	png_alloc_node_t *old_node = png_alloc_find_node(addr, oldSize);
  if (old_node) {
    void *new_addr = (__typeof__(new_addr))ReallocatePool(oldSize, newSize, addr);
    old_node->addr = new_addr;
    old_node->size = newSize;
    return new_addr;
  }
  else {
    png_alloc_node_t* node = png_alloc_malloc(newSize);
    CopyMem(node->addr, addr, oldSize); // here, newSize is > oldSize
    return node->addr;
	}
  } else {
    return addr;
  }
}

void png_alloc_free(void *addr)
{
	png_alloc_node_t *node = png_alloc_find_node(addr, 0);
	if (!node)
		return;
	png_alloc_remove_node(node);
	FreePool(addr);
}

void png_alloc_free_all()
{
	while (png_alloc_tail) {
		void *addr = png_alloc_tail->addr;
		png_alloc_remove_node(png_alloc_tail);
		FreePool(addr);
	}
}

/*************************************************************************************************/

void vector32_cleanup(VECTOR_32 *p)
{
	p->size = p->allocsize = 0;
	if (p->data)
		png_alloc_free(p->data);
	p->data = NULL;
}

UINT32 vector32_resize(VECTOR_32 *p, UINT32 size)
{	// returns 1 if success, 0 if failure ==> nothing done
	void *data;
	if (size * sizeof (UINT32) > p->allocsize)
	{
		UINT32 newsize = size * sizeof (UINT32) * 2;
		data = png_alloc_realloc(p->data, p->allocsize, newsize);
		if (data)
		{
			p->allocsize = newsize;
			p->data = (UINT32 *) data;
			p->size = size;
		} else
			return 0;
	}
	else
		p->size = size;
	return 1;
}

UINT32 vector32_resizev(VECTOR_32 *p, UINT32 size, UINT32 value)
{	// resize and give all new elements the value
	UINT32 oldsize = p->size, i;
	if (!vector32_resize(p, size))
		return 0;
	for (i = oldsize; i < size; i++)
		p->data[i] = value;
	return 1;
}

void vector32_init(VECTOR_32 *p)
{
	p->data = NULL;
	p->size = p->allocsize = 0;
}

VECTOR_32 *vector32_new(UINT32 size, UINT32 value)
{
	VECTOR_32 *p = png_alloc_malloc(sizeof (VECTOR_32));
	vector32_init(p);
	if (size && !vector32_resizev(p, size, value))
		return NULL;
	return p;
}

/*************************************************************************************************/

void vector8_cleanup(VECTOR_8 *p)
{
	p->size = p->allocsize = 0;
	if (p->data)
		png_alloc_free(p->data);
	p->data = NULL;
}

UINT32 vector8_resize(VECTOR_8 *p, UINT32 size)
{	// returns 1 if success, 0 if failure ==> nothing done
	// xxx: the use of sizeof UINT32 here seems like a bug (this descends from the lodepng vector
	// compatibility functions which do the same). without this there is corruption in certain cases,
	// so this was probably done to cover up allocation bug(s) in the original picopng code!
	void *data;
	if (size * sizeof (UINT32) > p->allocsize)
	{
		UINT32 newsize = size * sizeof (UINT32) * 2;
		data = png_alloc_realloc(p->data, p->allocsize, newsize);
		if (data)
		{
			p->allocsize = newsize;
			p->data = (UINT8 *) data;
			p->size = size;
		} else
			return 0; // error: not enough memory
	} else
		p->size = size;
	return 1;
}

UINT32 vector8_resizev(VECTOR_8 *p, UINT32 size, UINT8 value)
{	// resize and give all new elements the value
	UINT32 oldsize = p->size, i;
	if (!vector8_resize(p, size))
		return 0;
	for (i = oldsize; i < size; i++)
		p->data[i] = value;
	return 1;
}

void vector8_init(VECTOR_8 *p)
{
	p->data = NULL;
	p->size = p->allocsize = 0;
}

VECTOR_8 *vector8_new(UINT32 size, UINT8 value)
{
	VECTOR_8 *p = png_alloc_malloc(sizeof (VECTOR_8));
	vector8_init(p);
	if (size && !vector8_resizev(p, size, value))
		return NULL;
	return p;
}

VECTOR_8 *vector8_copy(VECTOR_8 *p)
{
	VECTOR_8 *q = vector8_new(p->size, 0);
	UINT32 n;
	for (n = 0; n < q->size; n++)
		q->data[n] = p->data[n];
	return q;
}

/*************************************************************************************************/

const UINT32 LENBASE[29] = { 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51,
  59, 67, 83, 99, 115, 131, 163, 195, 227, 258 };
const UINT32 LENEXTRA[29] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4,
  4, 5, 5, 5, 5, 0 };
const UINT32 DISTBASE[30] = { 1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385,
  513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577 };
const UINT32 DISTEXTRA[30] = { 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,
  10, 10, 11, 11, 12, 12, 13, 13 };
// code length code lengths
const UINT32 CLCL[19] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

/*************************************************************************************************/

typedef struct
{
	// 2D representation of a huffman tree: The one dimension is "0" or "1", the other contains all
	// nodes and leaves of the tree.
	VECTOR_32 *tree2d;
} HuffmanTree;

HuffmanTree *HuffmanTree_new()
{
	HuffmanTree *tree = png_alloc_malloc(sizeof (HuffmanTree));
	tree->tree2d = NULL;
	return tree;
}

int HuffmanTree_makeFromLengths(HuffmanTree *tree, const VECTOR_32 *bitlen, UINT32 maxbitlen)
{	// make tree given the lengths
	VECTOR_32 *tree2d;
	UINT32 bits, n, i;
	UINT32 numcodes = (UINT32) bitlen->size, treepos = 0, nodefilled = 0;
	VECTOR_32 *tree1d, *blcount, *nextcode;
	tree1d = vector32_new(numcodes, 0);
	blcount = vector32_new(maxbitlen + 1, 0);
	nextcode = vector32_new(maxbitlen + 1, 0);
	for (bits = 0; bits < numcodes; bits++)
		blcount->data[bitlen->data[bits]]++; // count number of instances of each code length
	for (bits = 1; bits <= maxbitlen; bits++)
		nextcode->data[bits] = (nextcode->data[bits - 1] + blcount->data[bits - 1]) << 1;
	for (n = 0; n < numcodes; n++)
		if (bitlen->data[n] != 0)
			tree1d->data[n] = nextcode->data[bitlen->data[n]]++; // generate all the codes
	// 0x7fff here means the tree2d isn't filled there yet
	tree2d = vector32_new(numcodes * 2, 0x7fff);
	tree->tree2d = tree2d;
	for (n = 0; n < numcodes; n++) // the codes
		for (i = 0; i < bitlen->data[n]; i++)
		{ // the bits for this code
			UINT32 bit = (tree1d->data[n] >> (bitlen->data[n] - i - 1)) & 1;
			if (treepos > numcodes - 2)
				return 55;
			if (tree2d->data[2 * treepos + bit] == 0x7fff)
			{
				if (i + 1 == bitlen->data[n])
				{
					tree2d->data[2 * treepos + bit] = n;
					treepos = 0;
				}
				else
				{
					tree2d->data[2 * treepos + bit] = ++nodefilled + numcodes;
					treepos = nodefilled;
				}
			} else
				treepos = tree2d->data[2 * treepos + bit] - numcodes;
		}
	return 0;
}

int HuffmanTree_decode(const HuffmanTree *tree, BOOLEAN *decoded, UINT32 *result, UINT32 *treepos,
                       UINT32 bit)
{	// Decodes a symbol from the tree
	const VECTOR_32 *tree2d = tree->tree2d;
	UINT32 numcodes = (UINT32) tree2d->size / 2;
	if (*treepos >= numcodes)
		return 11;
	*result = tree2d->data[2 * (*treepos) + bit];
	*decoded = (*result < numcodes);
	*treepos = *decoded ? 0 : *result - numcodes;
	return 0;
}

/*************************************************************************************************/

int Inflator_error;

UINT32 Zlib_readBitFromStream(UINT32 *bitp, const UINT8 *bits)
{
	UINT32 result = (bits[*bitp >> 3] >> (*bitp & 0x7)) & 1;
	(*bitp)++;
	return result;
}

UINT32 Zlib_readBitsFromStream(UINT32 *bitp, const UINT8 *bits, UINT32 nbits)
{
	UINT32 i, result = 0;
	for (i = 0; i < nbits; i++)
		result += (Zlib_readBitFromStream(bitp, bits)) << i;
	return result;
}

void Inflator_generateFixedTrees(HuffmanTree *tree, HuffmanTree *treeD)
{	// get the tree of a deflated block with fixed tree
	UINT32 i;
	VECTOR_32 *bitlen, *bitlenD;
	bitlen = vector32_new(288, 8);
	bitlenD = vector32_new(32, 5);
	for (i = 144; i <= 255; i++)
		bitlen->data[i] = 9;
	for (i = 256; i <= 279; i++)
		bitlen->data[i] = 7;
	HuffmanTree_makeFromLengths(tree, bitlen, 15);
	HuffmanTree_makeFromLengths(treeD, bitlenD, 15);
}

UINT32 Inflator_huffmanDecodeSymbol(const UINT8 *in, UINT32 *bp, const HuffmanTree *codetree,
                                    UINT32 inlength)
{	// decode a single symbol from given list of bits with given code tree. returns the symbol
	BOOLEAN decoded = FALSE;
	UINT32 ct = 0;
	UINT32 treepos = 0;
	for (;;) {
		if ((*bp & 0x07) == 0 && (*bp >> 3) > inlength) {
			Inflator_error = 10; // error: end reached without endcode
			return 0;
		}
		Inflator_error = HuffmanTree_decode(codetree, &decoded, &ct, &treepos,
                                        Zlib_readBitFromStream(bp, in));
		if (Inflator_error)
			return 0; // stop, an error happened
		if (decoded)
			return ct;
	}
}

void Inflator_getTreeInflateDynamic(HuffmanTree *tree, HuffmanTree *treeD, const UINT8 *in,
                                    UINT32 *bp, UINT32 inlength)
{	// get the tree of a deflated block with dynamic tree, the tree itself is also Huffman
	// compressed with a known tree
	UINT32 i, n;
	UINT32 HLIT;
	UINT32 HDIST;
	UINT32 HCLEN;
	UINT32 replength;
	VECTOR_32 *codelengthcode;
	HuffmanTree *codelengthcodetree = HuffmanTree_new(); // the code tree for code length codes
	VECTOR_32 *bitlen, *bitlenD;
	bitlen = vector32_new(288, 0);
	bitlenD = vector32_new(32, 0);
	if (*bp >> 3 >= inlength - 2)
	{
		Inflator_error = 49; // the bit pointer is or will go past the memory
    png_alloc_free(codelengthcodetree);
		return;
	}
	HLIT = Zlib_readBitsFromStream(bp, in, 5) + 257;	// number of literal/length codes + 257
	HDIST = Zlib_readBitsFromStream(bp, in, 5) + 1;	// number of dist codes + 1
	HCLEN = Zlib_readBitsFromStream(bp, in, 4) + 4;	// number of code length codes + 4
  // lengths of tree to decode the lengths of the dynamic tree
	codelengthcode = vector32_new(19, 0);
	for (i = 0; i < 19; i++)
		codelengthcode->data[CLCL[i]] = (i < HCLEN) ? Zlib_readBitsFromStream(bp, in, 3) : 0;
  
	Inflator_error = HuffmanTree_makeFromLengths(codelengthcodetree, codelengthcode, 7);
	if (Inflator_error)
		return;
  
	for (i = 0; i < HLIT + HDIST; )
	{
		UINT32 code = Inflator_huffmanDecodeSymbol(in, bp, codelengthcodetree, inlength);
		if (Inflator_error)
			return;
		if (code <= 15)
		{ // a length code
			if (i < HLIT)
				bitlen->data[i++] = code;
			else
				bitlenD->data[i++ - HLIT] = code;
		} else if (code == 16)
		{ // repeat previous
			UINT32 value; // set value to the previous code
			if (*bp >> 3 >= inlength)
			{
				Inflator_error = 50; // error, bit pointer jumps past memory
				return;
			}
			replength = 3 + Zlib_readBitsFromStream(bp, in, 2);
			
			if ((i - 1) < HLIT)
				value = bitlen->data[i - 1];
			else
				value = bitlenD->data[i - HLIT - 1];
			for (n = 0; n < replength; n++)
			{ // repeat this value in the next lengths
				if (i >= HLIT + HDIST)
				{
					Inflator_error = 13; // error: i is larger than the amount of codes
					return;
				}
				if (i < HLIT)
					bitlen->data[i++] = value;
				else
					bitlenD->data[i++ - HLIT] = value;
			}
		} else if (code == 17)
		{
			if (*bp >> 3 >= inlength)
			{
				Inflator_error = 50; // error, bit pointer jumps past memory
				return;
			}
			replength = 3 + Zlib_readBitsFromStream(bp, in, 3);
			for (n = 0; n < replength; n++)
			{ // repeat this value in the next lengths
				if (i >= HLIT + HDIST) {
					Inflator_error = 14; // error: i is larger than the amount of codes
					return;
				}
				if (i < HLIT)
					bitlen->data[i++] = 0;
				else
					bitlenD->data[i++ - HLIT] = 0;
			}
		} else if (code == 18)
		{
			if (*bp >> 3 >= inlength)
			{
				Inflator_error = 50;
				return;
			}
			replength = 11 + Zlib_readBitsFromStream(bp, in, 7);
			for (n = 0; n < replength; n++)
			{
        
				if (i >= HLIT + HDIST)
				{
					Inflator_error = 15; // error: i is larger than the amount of codes
					return;
				}
				if (i < HLIT)
					bitlen->data[i++] = 0;
				else
					bitlenD->data[i++ - HLIT] = 0;
			}
		} else
		{
			Inflator_error = 16; // error: an nonexitent code appeared. This can never happen.
			return;
		}
	}
	if (bitlen->data[256] == 0)
	{
		Inflator_error = 64; // the length of the end code 256 must be larger than 0
		return;
	}
	// now we've finally got HLIT and HDIST, so generate the code trees, and the function is done
	Inflator_error = HuffmanTree_makeFromLengths(tree, bitlen, 15);
	if (Inflator_error)
		return;
	Inflator_error = HuffmanTree_makeFromLengths(treeD, bitlenD, 15);
	if (Inflator_error)
		return;
}

void Inflator_inflateHuffmanBlock(VECTOR_8 *out, const UINT8 *in, UINT32 *bp, UINT32 *pos,
                                  UINT32 inlength, UINT32 btype)
{
	HuffmanTree *codetree, *codetreeD; // the code tree for Huffman codes, dist codes
	codetree = HuffmanTree_new();
	codetreeD = HuffmanTree_new();
	if (btype == 1)
		Inflator_generateFixedTrees(codetree, codetreeD);
	else if (btype == 2)
	{
		Inflator_getTreeInflateDynamic(codetree, codetreeD, in, bp, inlength);
		if (Inflator_error)
			return;
	}
	for (;;)
	{
		UINT32 code = Inflator_huffmanDecodeSymbol(in, bp, codetree, inlength);
		if (Inflator_error)
			return;
		if (code == 256) // end code
			return;
		else if (code <= 255)
		{ // literal symbol
			if (*pos >= out->size)
				vector8_resize(out, (*pos + 1) * 2); // reserve more room
			out->data[(*pos)++] = (UINT8) code;
		}
		else
			
			if (code >= 257 && code <= 285)
      { // length code
        UINT32 length = LENBASE[code - 257], numextrabits = LENEXTRA[code - 257];
        UINT32 codeD;
        UINT32 dist,numextrabitsD;
        UINT32 start,back;
        UINT32 i;
        if ((*bp >> 3) >= inlength)
        {
          Inflator_error = 51; // error, bit pointer will jump past memory
          return;
        }
        length += Zlib_readBitsFromStream(bp, in, numextrabits);
        codeD = Inflator_huffmanDecodeSymbol(in, bp, codetreeD, inlength);
        if (Inflator_error)
          return;
        if (codeD > 29) {
          Inflator_error = 18; // error: invalid dist code (30-31 are never used)
          return;
        }
        dist = DISTBASE[codeD] ;
        numextrabitsD = DISTEXTRA[codeD];
        if ((*bp >> 3) >= inlength)
        {
          Inflator_error = 51; // error, bit pointer will jump past memory
          return;
        }
        dist += Zlib_readBitsFromStream(bp, in, numextrabitsD);
        start = *pos;
        back = start - dist; // backwards
        if (*pos + length >= out->size)
          vector8_resize(out, (*pos + length) * 2); // reserve more room
        
        for (i = 0; i < length; i++)
        {
          out->data[(*pos)++] = out->data[back++];
          if (back >= start)
            back = start - dist;
        }
      }
	}
}

void Inflator_inflateNoCompression(VECTOR_8 *out, const UINT8 *in, UINT32 *bp, UINT32 *pos,
                                   UINT32 inlength)
{
	UINT32 LEN,NLEN;
	UINT32 p;
	UINT32 n;
  
	while ((*bp & 0x7) != 0)
		(*bp)++; // go to first boundary of byte
	p = *bp / 8;
	if (p >= inlength - 4) {
		Inflator_error = 52; // error, bit pointer will jump past memory
		return;
	}
	LEN = in[p] + 256 * in[p + 1];
	NLEN = in[p + 2] + 256 * in[p + 3];
	
	p += 4;
	if (LEN + NLEN != 65535) {
		Inflator_error = 21; // error: NLEN is not one's complement of LEN
		return;
	}
	if (*pos + LEN >= out->size)
		vector8_resize(out, *pos + LEN);
	if (p + LEN > inlength) {
		Inflator_error = 23; // error: reading outside of in buffer
		return;
	}
	
	for (n = 0; n < LEN; n++)
		out->data[(*pos)++] = in[p++]; // read LEN bytes of literal data
	*bp = p * 8;
}

void Inflator_inflate(VECTOR_8 *out, const VECTOR_8 *in, UINT32 inpos)
{
	UINT32 bp = 0, pos = 0; // bit pointer and byte pointer
	UINT32 BFINAL = 0;
	UINT32 BTYPE;
	Inflator_error = 0;
	while (!BFINAL && !Inflator_error) {
		if (bp >> 3 >= in->size) {
			Inflator_error = 52; // error, bit pointer will jump past memory
			return;
		}
		BFINAL = Zlib_readBitFromStream(&bp, &in->data[inpos]);
		BTYPE = Zlib_readBitFromStream(&bp, &in->data[inpos]);
		BTYPE += 2 * Zlib_readBitFromStream(&bp, &in->data[inpos]);
		if (BTYPE == 3)
		{
			Inflator_error = 20; // error: invalid BTYPE
			return;
		}
		else if (BTYPE == 0)
			Inflator_inflateNoCompression(out, &in->data[inpos], &bp, &pos, in->size);
		else
			Inflator_inflateHuffmanBlock(out, &in->data[inpos], &bp, &pos, in->size, BTYPE);
	}
	if (!Inflator_error)
		vector8_resize(out, pos); // Only now we know the TRUE size of out, resize it to that
}

/*************************************************************************************************/

INT32 Zlib_decompress(VECTOR_8 *out, const VECTOR_8 *in) // returns error value
{
	UINT32 CM,CINFO,FDICT;
	if (in->size < 2) {
		return 53; // error, size of zlib data too small
	}
	if ((in->data[0] * 256 + in->data[1]) % 31 != 0) {
		// error: 256 * in->data[0] + in->data[1] must be a multiple of 31, the FCHECK value is
		// supposed to be made that way
		return 24;
	}
	CM = in->data[0] & 15;
	CINFO = (in->data[0] >> 4) & 15;
	FDICT = (in->data[1] >> 5) & 1;
  //	DBG("compression method %d and CINFO=%d FDICT=%d\n", CM, CINFO, FDICT);
	if (CM != 8 || CINFO > 7){
    //    DBG("Error 25 : compression method %d and CINFO=%d FDICT=%d\n", CM, CINFO, FDICT);
		// error: only compression method 8: inflate with sliding window of 32k is supported by
		// the PNG spec
		
		return 25;
	}
	if (FDICT != 0){
    //      DBG("Error 26 : compression method %d and CINFO=%d FDICT=%d\n", CM, CINFO, FDICT);
		// error: the specification of PNG says about the zlib stream: "The additional flags shall
		// not specify a preset dictionary."
		return 26;
	}
	Inflator_inflate(out, in, 2);
	return Inflator_error; // note: adler32 checksum was skipped and ignored
}

/*************************************************************************************************/

#define PNG_SIGNATURE	0x0a1a0a0d474e5089ull

#define CHUNK_IHDR		0x52444849
#define CHUNK_IDAT		0x54414449
#define CHUNK_IEND		0x444e4549
#define CHUNK_PLTE		0x45544c50
#define CHUNK_tRNS		0x534e5274

INT32 PNG_error;

UINT32 PNG_readBitFromReversedStream(UINT32 *bitp, const UINT8 *bits)
{
	UINT32 result = (bits[*bitp >> 3] >> (7 - (*bitp & 0x7))) & 1;
	(*bitp)++;
	return result;
}

UINT32 PNG_readBitsFromReversedStream(UINT32 *bitp, const UINT8 *bits, UINT32 nbits)
{
	UINT32 i, result = 0;
	for (i = nbits - 1; i < nbits; i--)
		result += ((PNG_readBitFromReversedStream(bitp, bits)) << i);
	return result;
}

void PNG_setBitOfReversedStream(UINT32 *bitp, UINT8 *bits, UINT32 bit)
{
	bits[*bitp >> 3] |= (bit << (7 - (*bitp & 0x7)));
	(*bitp)++;
}

UINT32 PNG_read32bitInt(const UINT8 *buffer)
{
	return (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
}

UINT32 PNG_read32bitLE(const UINT8 *buffer)
{
	return (buffer[3] << 24) | (buffer[2] << 16) | (buffer[1] << 8) | buffer[0];
}

INT32 PNG_checkColorValidity(UINT32 colorType, UINT32 bd) // return type is a LodePNG error code
{
	if ((colorType == 2 || colorType == 4 || colorType == 6)) {
		if (!(bd == 8 || bd == 16))
			return 37;
		else
			return 0;
	} else if (colorType == 0) {
		if (!(bd == 1 || bd == 2 || bd == 4 || bd == 8 || bd == 16))
			return 37;
		else
			return 0;
	} else if (colorType == 3) {
		if (!(bd == 1 || bd == 2 || bd == 4 || bd == 8))
			return 37;
		else
			return 0;
	} else
		return 31; // nonexistent color type
}

UINT32 PNG_getBpp(const PNG_INFO *info)
{
	UINT32 bitDepth, colorType;
	bitDepth = info->bitDepth;
	colorType = info->colorType;
	if (colorType == 2)
		return (3 * bitDepth);
	else if (colorType >= 4)
		return (colorType - 2) * bitDepth;
	else
		return bitDepth;
}

void PNG_readPngHeader(PNG_INFO *info, const UINT8 *in, UINT32 inlength)
{	// read the information from the header and store it in the Info
	if (inlength < 29) {
		PNG_error = 27; // error: the data length is smaller than the length of the header
		return;
	}
	if (*(UINT64 *) in != PNG_SIGNATURE) {
		PNG_error = 28; // no PNG signature
		return;
	}
	if (*(UINT32 *) &in[12] != CHUNK_IHDR) {
		PNG_error = 29; // error: it doesn't start with a IHDR chunk!
		return;
	}
	info->width = PNG_read32bitInt(&in[16]);
	info->height = PNG_read32bitInt(&in[20]);
	info->bitDepth = in[24];
	info->colorType = in[25];
	info->compressionMethod = in[26];
	if (in[26] != 0) {
		PNG_error = 32; // error: only compression method 0 is allowed in the specification
		DBG("error: only compression method 0 is allowed in the specification\n");
		return;
	}
	info->filterMethod = in[27];
	if (in[27] != 0) {
		PNG_error = 33; // error: only filter method 0 is allowed in the specification
		DBG("error: only filter method 0 is allowed in the specification\n");
		return;
	}
	info->interlaceMethod = in[28];
	if (in[28] > 1) {
		PNG_error = 34; // error: only interlace methods 0 and 1 exist in the specification
		DBG("error: only interlace method 0 and 1 exist in the specification\n");
		return;
	}
	PNG_error = PNG_checkColorValidity(info->colorType, info->bitDepth);
	if (PNG_error) {
		DBG("error: checkColorValidity =%d\n", PNG_error);
	}
}

INT32 PNG_paethPredictor(INT32 a, INT32 b, INT32 c) // Paeth predicter, used by PNG filter type 4
{
	INT32 p, pa, pb, pc;
	p = a + b - c;
	pa = p > a ? (p - a) : (a - p);
	pb = p > b ? (p - b) : (b - p);
	pc = p > c ? (p - c) : (c - p);
	return (pa <= pb && pa <= pc) ? a : (pb <= pc ? b : c);
}

void PNG_unFilterScanline(UINT8 *recon, const UINT8 *scanline, const UINT8 *precon,
                          UINT32 bytewidth, UINT32 filterType, UINT32 length)
{
	UINT32 i;
	switch (filterType) {
    case 0:
      for (i = 0; i < length; i++)
        recon[i] = scanline[i];
      break;
    case 1:
      for (i = 0; i < bytewidth; i++)
        recon[i] = scanline[i];
      for (i = bytewidth; i < length; i++)
        recon[i] = scanline[i] + recon[i - bytewidth];
      break;
    case 2:
      if (precon)
        for (i = 0; i < length; i++)
          recon[i] = scanline[i] + precon[i];
      else
        for (i = 0; i < length; i++)
          recon[i] = scanline[i];
      break;
    case 3:
      if (precon) {
        for (i = 0; i < bytewidth; i++)
          recon[i] = scanline[i] + precon[i] / 2;
        for (i = bytewidth; i < length; i++)
          recon[i] = scanline[i] + ((recon[i - bytewidth] + precon[i]) / 2);
      } else {
        for (i = 0; i < bytewidth; i++)
          recon[i] = scanline[i];
        for (i = bytewidth; i < length; i++)
          recon[i] = scanline[i] + recon[i - bytewidth] / 2;
      }
      break;
    case 4:
      if (precon) {
        for (i = 0; i < bytewidth; i++)
          recon[i] = (UINT8) (scanline[i] + PNG_paethPredictor(0, precon[i], 0));
        for (i = bytewidth; i < length; i++)
          recon[i] = (UINT8) (scanline[i] + PNG_paethPredictor(recon[i - bytewidth],
                                                               precon[i], precon[i - bytewidth]));
      } else {
        for (i = 0; i < bytewidth; i++)
          recon[i] = scanline[i];
        for (i = bytewidth; i < length; i++)
          recon[i] = (UINT8) (scanline[i] + PNG_paethPredictor(recon[i - bytewidth], 0, 0));
      }
      break;
    default:
      PNG_error = 36; // error: nonexistent filter type given
			DBG("error: nonexistent filter type given\n");
      return;
	}
}

void PNG_adam7Pass(UINT8 *out, UINT8 *linen, UINT8 *lineo, const UINT8 *in, UINT32 w,
                   UINT32 passleft, UINT32 passtop, UINT32 spacex, UINT32 spacey, UINT32 passw, UINT32 passh,
                   UINT32 bpp)
{	// filter and reposition the pixels into the output when the image is Adam7 interlaced. This
	// function can only do it after the full image is already decoded. The out buffer must have
	// the correct allocated memory size already.
	UINT32 bytewidth,linelength,y;
  
	if (passw == 0)
		return;
	bytewidth = (bpp + 7) / 8;
	linelength = 1 + ((bpp * passw + 7) / 8);
	for (y = 0; y < passh; y++)
	{
		UINT32 i, b;
		UINT8 filterType = in[y * linelength], *prevline = (y == 0) ? 0 : lineo;
		UINT8 *temp;
		PNG_unFilterScanline(linen, &in[y * linelength + 1], prevline, bytewidth, filterType,
                         (w * bpp + 7) / 8);
		if (PNG_error)
			return;
		if (bpp >= 8)
			for (i = 0; i < passw; i++)
				for (b = 0; b < bytewidth; b++) // b = current byte of this pixel
					out[bytewidth * w * (passtop + spacey * y) + bytewidth *
							(passleft + spacex * i) + b] = linen[bytewidth * i + b];
		else
			for (i = 0; i < passw; i++)
			{
				UINT32 obp, bp;
				obp = bpp * w * (passtop + spacey * y) + bpp * (passleft + spacex * i);
				bp = i * bpp;
				for (b = 0; b < bpp; b++)
					PNG_setBitOfReversedStream(&obp, out, PNG_readBitFromReversedStream(&bp, linen));
			}
		temp = linen;
		linen = lineo;
		lineo = temp; // swap the two buffer pointers "line old" and "line new"
	}
}

INT32 PNG_convert(const PNG_INFO *info, VECTOR_8 *out, const UINT8 *in)
{	// converts from any color type to 32-bit. return value = LodePNG error code
	UINT32 i, c, numpixels,bp;
	UINT32 bitDepth, colorType;
	UINT8 *out_data;
	bitDepth = info->bitDepth;
	colorType = info->colorType;
	numpixels = info->width * info->height;
	bp = 0;
	vector8_resize(out, numpixels * 4);
	out_data = out->size ? out->data : 0;
	if (bitDepth == 8 && colorType == 0) // greyscale
		for (i = 0; i < numpixels; i++)
		{
			out_data[4 * i + 0] = out_data[4 * i + 1] = out_data[4 * i + 2] = in[i];
			out_data[4 * i + 3] = (info->key_defined && (in[i] == info->key_r)) ? 0 : 255;
		}
	else if (bitDepth == 8 && colorType == 2) // RGB color
		for (i = 0; i < numpixels; i++)
		{
			for (c = 0; c < 3; c++)
				out_data[4 * i + c] = in[3 * i + c];
			out_data[4 * i + 3] = (info->key_defined && (in[3 * i + 0] == info->key_r) &&
                             (in[3 * i + 1] == info->key_g) && (in[3 * i + 2] == info->key_b)) ? 0 : 255;
		}
	else if (bitDepth == 8 && colorType == 3) // indexed color (palette)
		for (i = 0; i < numpixels; i++)
		{
			if (4U * in[i] >= info->palette->size)
				return 46;
			for (c = 0; c < 4; c++) // get rgb colors from the palette
				out_data[4 * i + c] = info->palette->data[4 * in[i] + c];
		}
	else if (bitDepth == 8 && colorType == 4) // greyscale with alpha
		for (i = 0; i < numpixels; i++) {
			out_data[4 * i + 0] = out_data[4 * i + 1] = out_data[4 * i + 2] = in[2 * i + 0];
			out_data[4 * i + 3] = in[2 * i + 1];
		}
	else if (bitDepth == 8 && colorType == 6)
		for (i = 0; i < numpixels; i++)
			for (c = 0; c < 4; c++)
				out_data[4 * i + c] = in[4 * i + c]; // RGB with alpha
	else if (bitDepth == 16 && colorType == 0) // greyscale
		for (i = 0; i < numpixels; i++) {
			out_data[4 * i + 0] = out_data[4 * i + 1] = out_data[4 * i + 2] = in[2 * i];
			out_data[4 * i + 3] = (info->key_defined && (256U * in[i] + in[i + 1] == info->key_r))
      ? 0 : 255;
		}
	else if (bitDepth == 16 && colorType == 2) // RGB color
		for (i = 0; i < numpixels; i++) {
			for (c = 0; c < 3; c++)
				out_data[4 * i + c] = in[6 * i + 2 * c];
			out_data[4 * i + 3] = (info->key_defined &&
                             (256U * in[6 * i + 0] + in[6 * i + 1] == info->key_r) &&
                             (256U * in[6 * i + 2] + in[6 * i + 3] == info->key_g) &&
                             (256U * in[6 * i + 4] + in[6 * i + 5] == info->key_b)) ? 0 : 255;
		}
	else if (bitDepth == 16 && colorType == 4) // greyscale with alpha
		for (i = 0; i < numpixels; i++) {
			out_data[4 * i + 0] = out_data[4 * i + 1] = out_data[4 * i + 2] = in[4 * i]; // msb
			out_data[4 * i + 3] = in[4 * i + 2];
		}
	else if (bitDepth == 16 && colorType == 6)
		for (i = 0; i < numpixels; i++)
			for (c = 0; c < 4; c++)
				out_data[4 * i + c] = in[8 * i + 2 * c]; // RGB with alpha
	else if (bitDepth < 8 && colorType == 0) // greyscale
		for (i = 0; i < numpixels; i++) {
			UINT32 value = (PNG_readBitsFromReversedStream(&bp, in, bitDepth) * 255) /
      ((1 << bitDepth) - 1); // scale value from 0 to 255
			out_data[4 * i + 0] = out_data[4 * i + 1] = out_data[4 * i + 2] = (UINT8) value;
			out_data[4 * i + 3] = (info->key_defined && value &&
                             (((1U << bitDepth) - 1U) == info->key_r) && ((1U << bitDepth) - 1U)) ? 0 : 255;
		}
	else if (bitDepth < 8 && colorType == 3) // palette
		for (i = 0; i < numpixels; i++)
		{
			UINT32 value = PNG_readBitsFromReversedStream(&bp, in, bitDepth);
			if (4 * value >= info->palette->size)
				return 47;
			for (c = 0; c < 4; c++) // get rgb colors from the palette
				out_data[4 * i + c] = info->palette->data[4 * value + c];
		}
	return 0;
}

PNG_INFO *PNG_info_new()
{
	PNG_INFO *info = png_alloc_malloc(sizeof (PNG_INFO));
	UINT32 i;
	for (i = 0; i < sizeof (PNG_INFO); i++)
		((UINT8 *) info)[i] = 0;
	info->palette = vector8_new(0, 0);
	info->image = vector8_new(0, 0);
	return info;
}

PNG_INFO *PNG_decode(/* const*/ UINT8 *in, UINT32 size)
{
	UINT32 pos ;
	VECTOR_8 *idat;
	BOOLEAN IEND; //,known_type;
	UINT32 bpp;
	PNG_INFO *info;
	VECTOR_8 *scanlines; // now the out buffer will be filled
	UINT32 bytewidth, outlength ;
	UINT8 *out_data;
  
	PNG_error = 0;
  
	if (size == 0 || in == 0)
	{
		PNG_error = 48; // the given data is empty
		DBG("the given data is empty\n");
		return NULL;
	}
	info = PNG_info_new();
	PNG_readPngHeader(info, in, size);
	if (PNG_error){
		DBG("readPngHeader PNG_error=%d\n", PNG_error);
		return NULL;
	}
	pos = 33; // first byte of the first chunk after the header
	idat = NULL; // the data from idat chunks
	IEND = FALSE;
  //	known_type = TRUE;
	info->key_defined = FALSE;
	// loop through the chunks, ignoring unknown chunks and stopping at IEND chunk. IDAT data is
	// put at the start of the in buffer
	while (!IEND)
	{
		UINT32 i, j;
		UINT32 chunkLength ;
		UINT32 chunkType;
    UINT32 offset = 0;
		if (pos + 8 >= size)
		{
			PNG_error = 30; // error: size of the in buffer too small to contain next chunk
			DBG("error: size of the in buffer too small to contain next chunk size=%d\n", size);
			return NULL;
		}
		chunkLength = PNG_read32bitInt(&in[pos]); //big endian? Yes!
		pos += 4;
		if (chunkLength > 0x7fffffff)
		{
			PNG_error = 63;
			return NULL;
		}
		if (pos + chunkLength >= size) {
			PNG_error = 35; // error: size of the in buffer too small to contain next chunk
			return NULL;
		}
    //		chunkType = *(UINT32 *) &in[pos];
    chunkType = PNG_read32bitLE(&in[pos]); //read as LE to compare with our constants
    switch (chunkType) {
      case CHUNK_IDAT:
        if (idat)
        {
          offset = idat->size;
          vector8_resize(idat, offset + chunkLength);
        }
        else
          idat = vector8_new(chunkLength, 0);
        for (i = 0; i < chunkLength; i++)
          idat->data[offset + i] = in[pos + 4 + i];
        pos += (4 + chunkLength);
        break;
      case CHUNK_IEND:
        pos += 4;
        IEND = TRUE;
        break;
      case CHUNK_PLTE:
        // PLTE: palette chunk
        pos += 4; // go after the 4 letters
        vector8_resize(info->palette, 4 * (chunkLength / 3));
        if (info->palette->size > (4 * 256))
        {
          PNG_error = 38; // error: palette too big
          return NULL;
        }
        for (i = 0; i < info->palette->size; i += 4) {
          for (j = 0; j < 3; j++)
            info->palette->data[i + j] = in[pos++]; // RGB
          info->palette->data[i + 3] = 255; // alpha
        }
        break;
      case CHUNK_tRNS:
        // tRNS: palette transparency chunk
        pos += 4; // go after the 4 letters
        switch (info->colorType) {
          case 0:
            if (chunkLength != 2) {
              PNG_error = 40; // error: this chunk must be 2 bytes for greyscale image
              return NULL;
            }
            info->key_defined = TRUE;
            info->key_r = info->key_g = info->key_b = 256 * in[pos] + in[pos + 1];
            pos += 2;
            break;
          case 2:
            if (chunkLength != 6) {
              PNG_error = 41; // error: this chunk must be 6 bytes for RGB image
              return NULL;
            }
            info->key_defined = TRUE;
            info->key_r = 256 * in[pos] + in[pos + 1];
            pos += 2;
            info->key_g = 256 * in[pos] + in[pos + 1];
            pos += 2;
            info->key_b = 256 * in[pos] + in[pos + 1];
            pos += 2;
            break;
          case 3:
            if (4 * chunkLength > info->palette->size) {
              PNG_error = 39; // error: more alpha values given than there are palette entries
              return NULL;
            }
            for (i = 0; i < chunkLength; i++)
              info->palette->data[4 * i + 3] = in[pos++];
            break;
          default:
            PNG_error = 42; // error: tRNS chunk not allowed for other color models
            return NULL;
            break;
        }
        break;
      default:
        // it's not an implemented chunk type, so ignore it: skip over the data
        if (!(in[pos + 0] & 0x20))
        {
          // error: unknown critical chunk (5th bit of first byte of chunk type is 0)
          PNG_error = 69;
          return NULL;
        }
        pos += (chunkLength + 4); // skip 4 letters and uninterpreted data of unimplemented chunk
        //        known_type = FALSE;
        break;
    }
	  pos += 4; // step over CRC (which is ignored)
	}
	bpp = PNG_getBpp(info);
	// now the out buffer will be filled
	scanlines = vector8_new(((info->width * (info->height * bpp + 7)) / 8) + info->height, 0);
	PNG_error = Zlib_decompress(scanlines, idat);
	if (PNG_error) {
    //      DBG("Zlib_decompress return %d", PNG_error);
		return NULL; // stop if the zlib decompressor returned an error
  }
	bytewidth = (bpp + 7) / 8;
	outlength = (info->height * info->width * bpp + 7) / 8;
	vector8_resize(info->image, outlength); // time to fill the out buffer
	out_data = outlength ? info->image->data : 0;
	if (info->interlaceMethod == 0)
	{ // no interlace, just filter
		UINT32 y, obp, bp;
		UINT32 linestart, linelength;
		linestart = 0;
		// length in bytes of a scanline, excluding the filtertype byte
		linelength = (info->width * bpp + 7) / 8;
		if (bpp >= 8) // byte per byte
			for (y = 0; y < info->height; y++)
			{
				UINT32 filterType = scanlines->data[linestart];
				const UINT8 *prevline;
				prevline = (y == 0) ? 0 : &out_data[(y - 1) * info->width * bytewidth];
				PNG_unFilterScanline(&out_data[linestart - y], &scanlines->data[linestart + 1],
                             prevline, bytewidth, filterType, linelength);
				if (PNG_error) {
          //  	        DBG("PNG_unFilterScanline 1 return %d", PNG_error);
					return NULL;
				}
				linestart += (1 + linelength); // go to start of next scanline
      } else
			{ // less than 8 bits per pixel, so fill it up bit per bit
        VECTOR_8 *templine; // only used if bpp < 8
        templine = vector8_new((info->width * bpp + 7) >> 3, 0);
        for (y = 0, obp = 0; y < info->height; y++)
        {
          UINT32 filterType = scanlines->data[linestart];
          const UINT8 *prevline;
          prevline = (y == 0) ? 0 : &out_data[(y - 1) * info->width * bytewidth];
          PNG_unFilterScanline(templine->data, &scanlines->data[linestart + 1], prevline,
                               bytewidth, filterType, linelength);
          if (PNG_error) {
            //					DBG("PNG_unFilterScanline 2 return %d", PNG_error);
            return NULL;
          }
          for (bp = 0; bp < info->width * bpp;)
            PNG_setBitOfReversedStream(&obp, out_data, PNG_readBitFromReversedStream(&bp,
                                                                                     templine->data));
          linestart += (1 + linelength); // go to start of next scanline
        }
      }
	} else
	{ // interlaceMethod is 1 (Adam7)
		INT32 i;
		VECTOR_8 *scanlineo, *scanlinen; // "old" and "new" scanline
#ifndef __GNUC__
#pragma warning(disable: 4204)
#endif
		UINT32 passw[7] = {
			(info->width + 7) / 8, (info->width + 3) / 8, (info->width + 3) / 4,
			(info->width + 1) / 4, (info->width + 1) / 2, (info->width + 0) / 2,
			(info->width + 0) / 1
		};
		UINT32 passh[7] = {
			(info->height + 7) / 8, (info->height + 7) / 8, (info->height + 3) / 8,
			(info->height + 3) / 4, (info->height + 1) / 4, (info->height + 1) / 2,
			(info->height + 0) / 2
		};
		UINT32 passstart[7]; //= { 0 };
		for (i=0; i<7; i++) {
			passstart[i] = 0;
		}
    //		UINT32 pattern[28] = { 0, 4, 0, 2, 0, 1, 0, 0, 0, 4, 0, 2, 0, 1, 8, 8, 4, 4, 2, 2, 1, 8, 8,
    //				8, 4, 4, 2, 2 }; // values for the adam7 passes
		for (i = 0; i < 6; i++)
			passstart[i + 1] = passstart[i] + passh[i] * ((passw[i] ? 1 : 0) + (passw[i] * bpp + 7) / 8);
		scanlineo = vector8_new((info->width * bpp + 7) / 8, 0);
		scanlinen = vector8_new((info->width * bpp + 7) / 8, 0);
		for (i = 0; i < 7; i++)
			PNG_adam7Pass(out_data, scanlinen->data, scanlineo->data, &scanlines->data[passstart[i]],
                    info->width, pattern[i], pattern[i + 7], pattern[i + 14], pattern[i + 21],
                    passw[i], passh[i], bpp);
	}
	if (info->colorType != 6 || info->bitDepth != 8)
	{ // conversion needed
		VECTOR_8 *copy = vector8_copy(info->image); // xxx: is this copy necessary?
		PNG_error = PNG_convert(info, info->image, copy->data);
    if (PNG_error) {
      //	            DBG("PNG_convert return %d", PNG_error);
      return NULL;
    }
	}
	return info;
}

/**********************************************************************************************/

EG_IMAGE * egDecodePNG(IN UINT8 *FileData, IN UINTN FileDataLength, IN BOOLEAN WantAlpha)
{
  EG_IMAGE            *NewImage;
  PNG_INFO            *info;
  //  UINT8               AlphaValue;
  EG_PIXEL            *Pixel;
  INTN                x, y;
  
  // read and check header
  if (FileDataLength < sizeof(BMP_IMAGE_HEADER) || FileData == NULL)
    return NULL;
  
  // read and check header
  PNG_error = 0;
  info = PNG_decode(FileData, (UINT32)FileDataLength);
  if (info == NULL || PNG_error) {
    //       DBG("PNG_decode == null, PNG_error=%d", PNG_error);
    return NULL;
  }
	
  NewImage = egCreateImage((INTN)info->width, (INTN)info->height, WantAlpha);
  if (NewImage == NULL) {
    //        DBG("egCreateImage == null");
    return NULL;
  }
  
  CopyMem(NewImage->PixelData, info->image->data, info->image->size);
  png_alloc_free_all();
  Pixel = (EG_PIXEL*)NewImage->PixelData;
  for (y = 0; y < NewImage->Height; y++) {
    for (x = 0; x < NewImage->Width; x++) {
      UINT8	Temp;
      Temp = Pixel->b;
      Pixel->b = Pixel->r;
      Pixel->r = Temp;
      Pixel++;        
    }
  }
  //      DBG("png decoded %dx%d datalenght=%d iconsize=%d\n", NewImage->Height, NewImage->Width, FileDataLength, IconSize);
  return NewImage;
}

#endif //LODEPNG


