/*
 * Copyright (c) 1999, 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and
 *    its contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */


#ifndef _EFICONTEXT_H_
#define _EFICONTEXT_H_


//
//  IA-64 processor exception types
//
#define    EXCPT_ALT_DTLB            4
#define    EXCPT_DNESTED_TLB         5
#define    EXCPT_BREAKPOINT         11
#define    EXCPT_EXTERNAL_INTERRUPT	12
#define    EXCPT_GEN_EXCEPT         24
#define    EXCPT_NAT_CONSUMPTION    26
#define    EXCPT_DEBUG_EXCEPT       29
#define    EXCPT_UNALIGNED_ACCESS   30
#define    EXCPT_FP_FAULT           32
#define    EXCPT_FP_TRAP            33
#define    EXCPT_TAKEN_BRANCH       35
#define    EXCPT_SINGLE_STEP        36

//
//  IA-64 processor context definition - must be 512 byte aligned!!!
//
typedef
struct {
	UINT64 reserved;	// necessary to preserve alignment for the correct bits in UNAT and to insure F2 is 16 byte aligned...
    
    UINT64 r1;
    UINT64 r2;
    UINT64 r3;
    UINT64 r4;
    UINT64 r5;
    UINT64 r6;
    UINT64 r7;
    UINT64 r8;
    UINT64 r9;
    UINT64 r10;
    UINT64 r11;
    UINT64 r12;
    UINT64 r13;
    UINT64 r14;
    UINT64 r15;
    UINT64 r16;
    UINT64 r17;
    UINT64 r18;
    UINT64 r19;
    UINT64 r20;
    UINT64 r21;
    UINT64 r22;
    UINT64 r23;
    UINT64 r24;
    UINT64 r25;
    UINT64 r26;
    UINT64 r27;
    UINT64 r28;
    UINT64 r29;
    UINT64 r30;
    UINT64 r31;
    
    UINT64 f2[2];
    UINT64 f3[2];
    UINT64 f4[2];
    UINT64 f5[2];
    UINT64 f6[2];
    UINT64 f7[2];
    UINT64 f8[2];
    UINT64 f9[2];
    UINT64 f10[2];
    UINT64 f11[2];
    UINT64 f12[2];
    UINT64 f13[2];
    UINT64 f14[2];
    UINT64 f15[2];
    UINT64 f16[2];
    UINT64 f17[2];
    UINT64 f18[2];
    UINT64 f19[2];
    UINT64 f20[2];
    UINT64 f21[2];
    UINT64 f22[2];
    UINT64 f23[2];
    UINT64 f24[2];
    UINT64 f25[2];
    UINT64 f26[2];
    UINT64 f27[2];
    UINT64 f28[2];
    UINT64 f29[2];
    UINT64 f30[2];
    UINT64 f31[2];
    
    UINT64 pr;
    
    UINT64 b0;
    UINT64 b1;
    UINT64 b2;
    UINT64 b3;
    UINT64 b4;
    UINT64 b5;
    UINT64 b6;
    UINT64 b7;
    
    // application registers
    UINT64 ar_rsc;
    UINT64 ar_bsp;
    UINT64 ar_bspstore;
    UINT64 ar_rnat;

    UINT64 ar_fcr;

    UINT64 ar_eflag;
    UINT64 ar_csd;
    UINT64 ar_ssd;
    UINT64 ar_cflg;
    UINT64 ar_fsr;
    UINT64 ar_fir;
    UINT64 ar_fdr;

    UINT64 ar_ccv;

    UINT64 ar_unat;

    UINT64 ar_fpsr;
    
    UINT64 ar_pfs;
    UINT64 ar_lc;
    UINT64 ar_ec;
    
    // control registers
    UINT64 cr_dcr;
    UINT64 cr_itm;
    UINT64 cr_iva;
    UINT64 cr_pta;
    UINT64 cr_ipsr;
    UINT64 cr_isr;
    UINT64 cr_iip;
    UINT64 cr_ifa;
    UINT64 cr_itir;
    UINT64 cr_iipa;
    UINT64 cr_ifs;
    UINT64 cr_iim;
    UINT64 cr_iha;
    
    // debug registers
    UINT64 dbr0;
    UINT64 dbr1;
    UINT64 dbr2;
    UINT64 dbr3;
    UINT64 dbr4;
    UINT64 dbr5;
    UINT64 dbr6;
    UINT64 dbr7;
    
    UINT64 ibr0;
    UINT64 ibr1;
    UINT64 ibr2;
    UINT64 ibr3;
    UINT64 ibr4;
    UINT64 ibr5;
    UINT64 ibr6;
    UINT64 ibr7;
    
    // virtual registers
    UINT64 int_nat;	// nat bits for R1-R31
    
} SYSTEM_CONTEXT;

#endif /* _EFI_CONTEXT_H_ */
