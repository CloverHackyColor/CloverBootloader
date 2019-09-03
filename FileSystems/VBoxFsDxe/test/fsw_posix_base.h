/**
 * \file fsw_posix_base.h
 * Base definitions for the POSIX user space host environment.
 */

/*-
 * Copyright (c) 2006 Christoph Pfisterer
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Christoph Pfisterer nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _FSW_POSIX_BASE_H_
#define _FSW_POSIX_BASE_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define FSW_LITTLE_ENDIAN (1)
// TODO: use info from the headers to define FSW_LITTLE_ENDIAN or FSW_BIG_ENDIAN


// types

typedef signed char         fsw_s8;
typedef unsigned char       fsw_u8;
typedef short               fsw_s16;
typedef unsigned short      fsw_u16;
typedef long                fsw_s32;
typedef unsigned long       fsw_u32;
typedef long long           fsw_s64;
typedef unsigned long long  fsw_u64;


// allocation functions

#define fsw_alloc(size, ptrptr) (((*(ptrptr) = malloc(size)) == NULL) ? FSW_OUT_OF_MEMORY : FSW_SUCCESS)
#define fsw_free(ptr) free(ptr)

// memory functions

#define fsw_memzero(dest,size) memset(dest,0,size)
#define fsw_memcpy(dest,src,size) memcpy(dest,src,size)
#define fsw_memeq(p1,p2,size) (memcmp(p1,p2,size) == 0)

// message printing

#define FSW_MSGSTR(s) s
#define FSW_MSGFUNC printf

// 64-bit hooks

#define FSW_U64_SHR(val,shiftbits) ((val) >> (shiftbits))
#define FSW_U64_DIV(val,divisor) ((val) / (divisor))
#define DEBUG(x)

#define RShiftU64(val, shift) ((val) >> (shift))
#define LShiftU64(val, shift) ((val) << (shift))

#endif
