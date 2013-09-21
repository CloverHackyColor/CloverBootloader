/* Print i386 instructions for GDB, the GNU debugger.
   Copyright 1988, 1989, 1991, 1993, 1994, 1995, 1996, 1997, 1998, 1999,
   2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011
   Free Software Foundation, Inc.

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */


/* 80386 instruction printer by Pace Willisson (pace@prep.ai.mit.edu)
   July 1988
    modified by John Hassey (hassey@dg-rtp.dg.com)
    x86-64 support added by Jan Hubicka (jh@suse.cz)
    VIA PadLock support by Michal Ludvig (mludvig@suse.cz).  */

/* The main tables describing the instructions is essentially a copy
   of the "Opcode Map" chapter (Appendix A) of the Intel 80386
   Programmers Manual.  Usually, there is a capital letter, followed
   by a small letter.  The capital letter tell the addressing mode,
   and the small letter tells about the operand size.  Refer to
   the Intel manual for details.  */

#include "sysdep.h"
#include "dis-asm.h"
#include "opintl.h"
#include "opcode/i386.h"
#include "libiberty.h"

#include <setjmp.h>

static int print_insn (bfd_vma, disassemble_info *);
static void dofloat (int);
static void OP_ST (int, int);
static void OP_STi (int, int);
static int putop (const char *, int);
static void oappend (const char *);
static void append_seg (void);
static void OP_indirE (int, int);
static void print_operand_value (char *, int, bfd_vma);
static void OP_E_register (int, int);
static void OP_E_memory (int, int);
static void print_displacement (char *, bfd_vma);
static void OP_E (int, int);
static void OP_G (int, int);
static bfd_vma get64 (void);
static bfd_signed_vma get32 (void);
static bfd_signed_vma get32s (void);
static int get16 (void);
static void set_op (bfd_vma, int);
static void OP_Skip_MODRM (int, int);
static void OP_REG (int, int);
static void OP_IMREG (int, int);
static void OP_I (int, int);
static void OP_I64 (int, int);
static void OP_sI (int, int);
static void OP_J (int, int);
static void OP_SEG (int, int);
static void OP_DIR (int, int);
static void OP_OFF (int, int);
static void OP_OFF64 (int, int);
static void ptr_reg (int, int);
static void OP_ESreg (int, int);
static void OP_DSreg (int, int);
static void OP_C (int, int);
static void OP_D (int, int);
static void OP_T (int, int);
static void OP_R (int, int);
static void OP_MMX (int, int);
static void OP_XMM (int, int);
static void OP_EM (int, int);
static void OP_EX (int, int);
static void OP_EMC (int,int);
static void OP_MXC (int,int);
static void OP_MS (int, int);
static void OP_XS (int, int);
static void OP_M (int, int);
static void OP_VEX (int, int);
static void OP_EX_Vex (int, int);
static void OP_EX_VexW (int, int);
static void OP_EX_VexImmW (int, int);
static void OP_XMM_Vex (int, int);
static void OP_XMM_VexW (int, int);
static void OP_REG_VexI4 (int, int);
static void PCLMUL_Fixup (int, int);
static void VEXI4_Fixup (int, int);
static void VZERO_Fixup (int, int);
static void VCMP_Fixup (int, int);
static void OP_0f07 (int, int);
static void OP_Monitor (int, int);
static void OP_Mwait (int, int);
static void NOP_Fixup1 (int, int);
static void NOP_Fixup2 (int, int);
static void OP_3DNowSuffix (int, int);
static void CMP_Fixup (int, int);
static void BadOp (void);
static void REP_Fixup (int, int);
static void HLE_Fixup1 (int, int);
static void HLE_Fixup2 (int, int);
static void HLE_Fixup3 (int, int);
static void CMPXCHG8B_Fixup (int, int);
static void XMM_Fixup (int, int);
static void CRC32_Fixup (int, int);
static void FXSAVE_Fixup (int, int);
static void OP_LWPCB_E (int, int);
static void OP_LWP_E (int, int);
static void OP_Vex_2src_1 (int, int);
static void OP_Vex_2src_2 (int, int);

static void MOVBE_Fixup (int, int);

struct dis_private {
  /* Points to first byte not fetched.  */
  bfd_byte *max_fetched;
  bfd_byte the_buffer[MAX_MNEM_SIZE];
  bfd_vma insn_start;
  int orig_sizeflag;
  jmp_buf bailout;
};

enum address_mode
{
  mode_16bit,
  mode_32bit,
  mode_64bit
};

enum address_mode address_mode;

/* Flags for the prefixes for the current instruction.  See below.  */
static int prefixes;

/* REX prefix the current instruction.  See below.  */
static int rex;
/* Bits of REX we've already used.  */
static int rex_used;
/* REX bits in original REX prefix ignored.  */
static int rex_ignored;
/* Mark parts used in the REX prefix.  When we are testing for
   empty prefix (for 8bit register REX extension), just mask it
   out.  Otherwise test for REX bit is excuse for existence of REX
   only in case value is nonzero.  */
#define USED_REX(value)					\
  {							\
    if (value)						\
      {							\
	if ((rex & value))				\
	  rex_used |= (value) | REX_OPCODE;		\
      }							\
    else						\
      rex_used |= REX_OPCODE;				\
  }

/* Flags for prefixes which we somehow handled when printing the
   current instruction.  */
static int used_prefixes;

/* Flags stored in PREFIXES.  */
#define PREFIX_REPZ 1
#define PREFIX_REPNZ 2
#define PREFIX_LOCK 4
#define PREFIX_CS 8
#define PREFIX_SS 0x10
#define PREFIX_DS 0x20
#define PREFIX_ES 0x40
#define PREFIX_FS 0x80
#define PREFIX_GS 0x100
#define PREFIX_DATA 0x200
#define PREFIX_ADDR 0x400
#define PREFIX_FWAIT 0x800

/* Make sure that bytes from INFO->PRIVATE_DATA->BUFFER (inclusive)
   to ADDR (exclusive) are valid.  Returns 1 for success, longjmps
   on error.  */
#define FETCH_DATA(info, addr) \
  ((addr) <= ((struct dis_private *) (info->private_data))->max_fetched \
   ? 1 : fetch_data ((info), (addr)))

static int
fetch_data (struct disassemble_info *info, bfd_byte *addr)
{
  int status;
  struct dis_private *priv = (struct dis_private *) info->private_data;
  bfd_vma start = priv->insn_start + (priv->max_fetched - priv->the_buffer);

  if (addr <= priv->the_buffer + MAX_MNEM_SIZE)
    status = (*info->read_memory_func) (start,
					priv->max_fetched,
					addr - priv->max_fetched,
					info);
  else
    status = -1;
  if (status != 0)
    {
      /* If we did manage to read at least one byte, then
	 print_insn_i386 will do something sensible.  Otherwise, print
	 an error.  We do that here because this is where we know
	 STATUS.  */
      if (priv->max_fetched == priv->the_buffer)
	(*info->memory_error_func) (status, start, info);
      longjmp (priv->bailout, 1);
    }
  else
    priv->max_fetched = addr;
  return 1;
}

#define XX { NULL, 0 }
#define Bad_Opcode NULL, { { NULL, 0 } }

#define Eb { OP_E, b_mode }
#define EbS { OP_E, b_swap_mode }
#define Ev { OP_E, v_mode }
#define EvS { OP_E, v_swap_mode }
#define Ed { OP_E, d_mode }
#define Edq { OP_E, dq_mode }
#define Edqw { OP_E, dqw_mode }
#define Edqb { OP_E, dqb_mode }
#define Edqd { OP_E, dqd_mode }
#define Eq { OP_E, q_mode }
#define indirEv { OP_indirE, stack_v_mode }
#define indirEp { OP_indirE, f_mode }
#define stackEv { OP_E, stack_v_mode }
#define Em { OP_E, m_mode }
#define Ew { OP_E, w_mode }
#define M { OP_M, 0 }		/* lea, lgdt, etc. */
#define Ma { OP_M, a_mode }
#define Mb { OP_M, b_mode }
#define Md { OP_M, d_mode }
#define Mo { OP_M, o_mode }
#define Mp { OP_M, f_mode }		/* 32 or 48 bit memory operand for LDS, LES etc */
#define Mq { OP_M, q_mode }
#define Mx { OP_M, x_mode }
#define Mxmm { OP_M, xmm_mode }
#define Gb { OP_G, b_mode }
#define Gv { OP_G, v_mode }
#define Gd { OP_G, d_mode }
#define Gdq { OP_G, dq_mode }
#define Gm { OP_G, m_mode }
#define Gw { OP_G, w_mode }
#define Rd { OP_R, d_mode }
#define Rm { OP_R, m_mode }
#define Ib { OP_I, b_mode }
#define sIb { OP_sI, b_mode }	/* sign extened byte */
#define sIbT { OP_sI, b_T_mode } /* sign extened byte like 'T' */
#define Iv { OP_I, v_mode }
#define sIv { OP_sI, v_mode } 
#define Iq { OP_I, q_mode }
#define Iv64 { OP_I64, v_mode }
#define Iw { OP_I, w_mode }
#define I1 { OP_I, const_1_mode }
#define Jb { OP_J, b_mode }
#define Jv { OP_J, v_mode }
#define Cm { OP_C, m_mode }
#define Dm { OP_D, m_mode }
#define Td { OP_T, d_mode }
#define Skip_MODRM { OP_Skip_MODRM, 0 }

#define RMeAX { OP_REG, eAX_reg }
#define RMeBX { OP_REG, eBX_reg }
#define RMeCX { OP_REG, eCX_reg }
#define RMeDX { OP_REG, eDX_reg }
#define RMeSP { OP_REG, eSP_reg }
#define RMeBP { OP_REG, eBP_reg }
#define RMeSI { OP_REG, eSI_reg }
#define RMeDI { OP_REG, eDI_reg }
#define RMrAX { OP_REG, rAX_reg }
#define RMrBX { OP_REG, rBX_reg }
#define RMrCX { OP_REG, rCX_reg }
#define RMrDX { OP_REG, rDX_reg }
#define RMrSP { OP_REG, rSP_reg }
#define RMrBP { OP_REG, rBP_reg }
#define RMrSI { OP_REG, rSI_reg }
#define RMrDI { OP_REG, rDI_reg }
#define RMAL { OP_REG, al_reg }
#define RMCL { OP_REG, cl_reg }
#define RMDL { OP_REG, dl_reg }
#define RMBL { OP_REG, bl_reg }
#define RMAH { OP_REG, ah_reg }
#define RMCH { OP_REG, ch_reg }
#define RMDH { OP_REG, dh_reg }
#define RMBH { OP_REG, bh_reg }
#define RMAX { OP_REG, ax_reg }
#define RMDX { OP_REG, dx_reg }

#define eAX { OP_IMREG, eAX_reg }
#define eBX { OP_IMREG, eBX_reg }
#define eCX { OP_IMREG, eCX_reg }
#define eDX { OP_IMREG, eDX_reg }
#define eSP { OP_IMREG, eSP_reg }
#define eBP { OP_IMREG, eBP_reg }
#define eSI { OP_IMREG, eSI_reg }
#define eDI { OP_IMREG, eDI_reg }
#define AL { OP_IMREG, al_reg }
#define CL { OP_IMREG, cl_reg }
#define DL { OP_IMREG, dl_reg }
#define BL { OP_IMREG, bl_reg }
#define AH { OP_IMREG, ah_reg }
#define CH { OP_IMREG, ch_reg }
#define DH { OP_IMREG, dh_reg }
#define BH { OP_IMREG, bh_reg }
#define AX { OP_IMREG, ax_reg }
#define DX { OP_IMREG, dx_reg }
#define zAX { OP_IMREG, z_mode_ax_reg }
#define indirDX { OP_IMREG, indir_dx_reg }

#define Sw { OP_SEG, w_mode }
#define Sv { OP_SEG, v_mode }
#define Ap { OP_DIR, 0 }
#define Ob { OP_OFF64, b_mode }
#define Ov { OP_OFF64, v_mode }
#define Xb { OP_DSreg, eSI_reg }
#define Xv { OP_DSreg, eSI_reg }
#define Xz { OP_DSreg, eSI_reg }
#define Yb { OP_ESreg, eDI_reg }
#define Yv { OP_ESreg, eDI_reg }
#define DSBX { OP_DSreg, eBX_reg }

#define es { OP_REG, es_reg }
#define ss { OP_REG, ss_reg }
#define cs { OP_REG, cs_reg }
#define ds { OP_REG, ds_reg }
#define fs { OP_REG, fs_reg }
#define gs { OP_REG, gs_reg }

#define MX { OP_MMX, 0 }
#define XM { OP_XMM, 0 }
#define XMScalar { OP_XMM, scalar_mode }
#define XMGatherQ { OP_XMM, vex_vsib_q_w_dq_mode }
#define XMM { OP_XMM, xmm_mode }
#define EM { OP_EM, v_mode }
#define EMS { OP_EM, v_swap_mode }
#define EMd { OP_EM, d_mode }
#define EMx { OP_EM, x_mode }
#define EXw { OP_EX, w_mode }
#define EXd { OP_EX, d_mode }
#define EXdScalar { OP_EX, d_scalar_mode }
#define EXdS { OP_EX, d_swap_mode }
#define EXq { OP_EX, q_mode }
#define EXqScalar { OP_EX, q_scalar_mode }
#define EXqScalarS { OP_EX, q_scalar_swap_mode }
#define EXqS { OP_EX, q_swap_mode }
#define EXx { OP_EX, x_mode }
#define EXxS { OP_EX, x_swap_mode }
#define EXxmm { OP_EX, xmm_mode }
#define EXxmmq { OP_EX, xmmq_mode }
#define EXxmm_mb { OP_EX, xmm_mb_mode }
#define EXxmm_mw { OP_EX, xmm_mw_mode }
#define EXxmm_md { OP_EX, xmm_md_mode }
#define EXxmm_mq { OP_EX, xmm_mq_mode }
#define EXxmmdw { OP_EX, xmmdw_mode }
#define EXxmmqd { OP_EX, xmmqd_mode }
#define EXymmq { OP_EX, ymmq_mode }
#define EXVexWdq { OP_EX, vex_w_dq_mode }
#define EXVexWdqScalar { OP_EX, vex_scalar_w_dq_mode }
#define MS { OP_MS, v_mode }
#define XS { OP_XS, v_mode }
#define EMCq { OP_EMC, q_mode }
#define MXC { OP_MXC, 0 }
#define OPSUF { OP_3DNowSuffix, 0 }
#define CMP { CMP_Fixup, 0 }
#define XMM0 { XMM_Fixup, 0 }
#define FXSAVE { FXSAVE_Fixup, 0 }
#define Vex_2src_1 { OP_Vex_2src_1, 0 }
#define Vex_2src_2 { OP_Vex_2src_2, 0 }

#define Vex { OP_VEX, vex_mode }
#define VexScalar { OP_VEX, vex_scalar_mode }
#define VexGatherQ { OP_VEX, vex_vsib_q_w_dq_mode }
#define Vex128 { OP_VEX, vex128_mode }
#define Vex256 { OP_VEX, vex256_mode }
#define VexGdq { OP_VEX, dq_mode }
#define VexI4 { VEXI4_Fixup, 0}
#define EXdVex { OP_EX_Vex, d_mode }
#define EXdVexS { OP_EX_Vex, d_swap_mode }
#define EXdVexScalarS { OP_EX_Vex, d_scalar_swap_mode }
#define EXqVex { OP_EX_Vex, q_mode }
#define EXqVexS { OP_EX_Vex, q_swap_mode }
#define EXqVexScalarS { OP_EX_Vex, q_scalar_swap_mode }
#define EXVexW { OP_EX_VexW, x_mode }
#define EXdVexW { OP_EX_VexW, d_mode }
#define EXqVexW { OP_EX_VexW, q_mode }
#define EXVexImmW { OP_EX_VexImmW, x_mode }
#define XMVex { OP_XMM_Vex, 0 }
#define XMVexScalar { OP_XMM_Vex, scalar_mode }
#define XMVexW { OP_XMM_VexW, 0 }
#define XMVexI4 { OP_REG_VexI4, x_mode }
#define PCLMUL { PCLMUL_Fixup, 0 }
#define VZERO { VZERO_Fixup, 0 }
#define VCMP { VCMP_Fixup, 0 }

#define MVexVSIBDWpX { OP_M, vex_vsib_d_w_dq_mode }
#define MVexVSIBQWpX { OP_M, vex_vsib_q_w_dq_mode }

/* Used handle "rep" prefix for string instructions.  */
#define Xbr { REP_Fixup, eSI_reg }
#define Xvr { REP_Fixup, eSI_reg }
#define Ybr { REP_Fixup, eDI_reg }
#define Yvr { REP_Fixup, eDI_reg }
#define Yzr { REP_Fixup, eDI_reg }
#define indirDXr { REP_Fixup, indir_dx_reg }
#define ALr { REP_Fixup, al_reg }
#define eAXr { REP_Fixup, eAX_reg }

/* Used handle HLE prefix for lockable instructions.  */
#define Ebh1 { HLE_Fixup1, b_mode }
#define Evh1 { HLE_Fixup1, v_mode }
#define Ebh2 { HLE_Fixup2, b_mode }
#define Evh2 { HLE_Fixup2, v_mode }
#define Ebh3 { HLE_Fixup3, b_mode }
#define Evh3 { HLE_Fixup3, v_mode }

#define cond_jump_flag { NULL, cond_jump_mode }
#define loop_jcxz_flag { NULL, loop_jcxz_mode }

/* bits in sizeflag */
#define SUFFIX_ALWAYS 4
#define AFLAG 2
#define DFLAG 1

enum
{
  /* byte operand */
  b_mode = 1,
  /* byte operand with operand swapped */
  b_swap_mode,
  /* byte operand, sign extend like 'T' suffix */
  b_T_mode,
  /* operand size depends on prefixes */
  v_mode,
  /* operand size depends on prefixes with operand swapped */
  v_swap_mode,
  /* word operand */
  w_mode,
  /* double word operand  */
  d_mode,
  /* double word operand with operand swapped */
  d_swap_mode,
  /* quad word operand */
  q_mode,
  /* quad word operand with operand swapped */
  q_swap_mode,
  /* ten-byte operand */
  t_mode,
  /* 16-byte XMM or 32-byte YMM operand */
  x_mode,
  /* 16-byte XMM or 32-byte YMM operand with operand swapped */
  x_swap_mode,
  /* 16-byte XMM operand */
  xmm_mode,
  /* 16-byte XMM or quad word operand */
  xmmq_mode,
  /* XMM register or byte memory operand */
  xmm_mb_mode,
  /* XMM register or word memory operand */
  xmm_mw_mode,
  /* XMM register or double word memory operand */
  xmm_md_mode,
  /* XMM register or quad word memory operand */
  xmm_mq_mode,
  /* 16-byte XMM, word or double word operand  */
  xmmdw_mode,
  /* 16-byte XMM, double word or quad word operand */
  xmmqd_mode,
  /* 32-byte YMM or quad word operand */
  ymmq_mode,
  /* 32-byte YMM or 16-byte word operand */
  ymmxmm_mode,
  /* d_mode in 32bit, q_mode in 64bit mode.  */
  m_mode,
  /* pair of v_mode operands */
  a_mode,
  cond_jump_mode,
  loop_jcxz_mode,
  /* operand size depends on REX prefixes.  */
  dq_mode,
  /* registers like dq_mode, memory like w_mode.  */
  dqw_mode,
  /* 4- or 6-byte pointer operand */
  f_mode,
  const_1_mode,
  /* v_mode for stack-related opcodes.  */
  stack_v_mode,
  /* non-quad operand size depends on prefixes */
  z_mode,
  /* 16-byte operand */
  o_mode,
  /* registers like dq_mode, memory like b_mode.  */
  dqb_mode,
  /* registers like dq_mode, memory like d_mode.  */
  dqd_mode,
  /* normal vex mode */
  vex_mode,
  /* 128bit vex mode */
  vex128_mode,
  /* 256bit vex mode */
  vex256_mode,
  /* operand size depends on the VEX.W bit.  */
  vex_w_dq_mode,

  /* Similar to vex_w_dq_mode, with VSIB dword indices.  */
  vex_vsib_d_w_dq_mode,
  /* Similar to vex_w_dq_mode, with VSIB qword indices.  */
  vex_vsib_q_w_dq_mode,

  /* scalar, ignore vector length.  */
  scalar_mode,
  /* like d_mode, ignore vector length.  */
  d_scalar_mode,
  /* like d_swap_mode, ignore vector length.  */
  d_scalar_swap_mode,
  /* like q_mode, ignore vector length.  */
  q_scalar_mode,
  /* like q_swap_mode, ignore vector length.  */
  q_scalar_swap_mode,
  /* like vex_mode, ignore vector length.  */
  vex_scalar_mode,
  /* like vex_w_dq_mode, ignore vector length.  */
  vex_scalar_w_dq_mode,

  es_reg,
  cs_reg,
  ss_reg,
  ds_reg,
  fs_reg,
  gs_reg,

  eAX_reg,
  eCX_reg,
  eDX_reg,
  eBX_reg,
  eSP_reg,
  eBP_reg,
  eSI_reg,
  eDI_reg,

  al_reg,
  cl_reg,
  dl_reg,
  bl_reg,
  ah_reg,
  ch_reg,
  dh_reg,
  bh_reg,

  ax_reg,
  cx_reg,
  dx_reg,
  bx_reg,
  sp_reg,
  bp_reg,
  si_reg,
  di_reg,

  rAX_reg,
  rCX_reg,
  rDX_reg,
  rBX_reg,
  rSP_reg,
  rBP_reg,
  rSI_reg,
  rDI_reg,

  z_mode_ax_reg,
  indir_dx_reg
};

enum
{
  FLOATCODE = 1,
  USE_REG_TABLE,
  USE_MOD_TABLE,
  USE_RM_TABLE,
  USE_PREFIX_TABLE,
  USE_X86_64_TABLE,
  USE_3BYTE_TABLE,
  USE_XOP_8F_TABLE,
  USE_VEX_C4_TABLE,
  USE_VEX_C5_TABLE,
  USE_VEX_LEN_TABLE,
  USE_VEX_W_TABLE
};

#define FLOAT			NULL, { { NULL, FLOATCODE } }

#define DIS386(T, I)		NULL, { { NULL, (T)}, { NULL,  (I) } }
#define REG_TABLE(I)		DIS386 (USE_REG_TABLE, (I))
#define MOD_TABLE(I)		DIS386 (USE_MOD_TABLE, (I))
#define RM_TABLE(I)		DIS386 (USE_RM_TABLE, (I))
#define PREFIX_TABLE(I)		DIS386 (USE_PREFIX_TABLE, (I))
#define X86_64_TABLE(I)		DIS386 (USE_X86_64_TABLE, (I))
#define THREE_BYTE_TABLE(I)	DIS386 (USE_3BYTE_TABLE, (I))
#define XOP_8F_TABLE(I)		DIS386 (USE_XOP_8F_TABLE, (I))
#define VEX_C4_TABLE(I)		DIS386 (USE_VEX_C4_TABLE, (I))
#define VEX_C5_TABLE(I)		DIS386 (USE_VEX_C5_TABLE, (I))
#define VEX_LEN_TABLE(I)	DIS386 (USE_VEX_LEN_TABLE, (I))
#define VEX_W_TABLE(I)		DIS386 (USE_VEX_W_TABLE, (I))

enum
{
  REG_80 = 0,
  REG_81,
  REG_82,
  REG_8F,
  REG_C0,
  REG_C1,
  REG_C6,
  REG_C7,
  REG_D0,
  REG_D1,
  REG_D2,
  REG_D3,
  REG_F6,
  REG_F7,
  REG_FE,
  REG_FF,
  REG_0F00,
  REG_0F01,
  REG_0F0D,
  REG_0F18,
  REG_0F71,
  REG_0F72,
  REG_0F73,
  REG_0FA6,
  REG_0FA7,
  REG_0FAE,
  REG_0FBA,
  REG_0FC7,
  REG_VEX_0F71,
  REG_VEX_0F72,
  REG_VEX_0F73,
  REG_VEX_0FAE,
  REG_VEX_0F38F3,
  REG_XOP_LWPCB,
  REG_XOP_LWP,
  REG_XOP_TBM_01,
  REG_XOP_TBM_02
};

enum
{
  MOD_8D = 0,
  MOD_C6_REG_7,
  MOD_C7_REG_7,
  MOD_0F01_REG_0,
  MOD_0F01_REG_1,
  MOD_0F01_REG_2,
  MOD_0F01_REG_3,
  MOD_0F01_REG_7,
  MOD_0F12_PREFIX_0,
  MOD_0F13,
  MOD_0F16_PREFIX_0,
  MOD_0F17,
  MOD_0F18_REG_0,
  MOD_0F18_REG_1,
  MOD_0F18_REG_2,
  MOD_0F18_REG_3,
  MOD_0F20,
  MOD_0F21,
  MOD_0F22,
  MOD_0F23,
  MOD_0F24,
  MOD_0F26,
  MOD_0F2B_PREFIX_0,
  MOD_0F2B_PREFIX_1,
  MOD_0F2B_PREFIX_2,
  MOD_0F2B_PREFIX_3,
  MOD_0F51,
  MOD_0F71_REG_2,
  MOD_0F71_REG_4,
  MOD_0F71_REG_6,
  MOD_0F72_REG_2,
  MOD_0F72_REG_4,
  MOD_0F72_REG_6,
  MOD_0F73_REG_2,
  MOD_0F73_REG_3,
  MOD_0F73_REG_6,
  MOD_0F73_REG_7,
  MOD_0FAE_REG_0,
  MOD_0FAE_REG_1,
  MOD_0FAE_REG_2,
  MOD_0FAE_REG_3,
  MOD_0FAE_REG_4,
  MOD_0FAE_REG_5,
  MOD_0FAE_REG_6,
  MOD_0FAE_REG_7,
  MOD_0FB2,
  MOD_0FB4,
  MOD_0FB5,
  MOD_0FC7_REG_6,
  MOD_0FC7_REG_7,
  MOD_0FD7,
  MOD_0FE7_PREFIX_2,
  MOD_0FF0_PREFIX_3,
  MOD_0F382A_PREFIX_2,
  MOD_62_32BIT,
  MOD_C4_32BIT,
  MOD_C5_32BIT,
  MOD_VEX_0F12_PREFIX_0,
  MOD_VEX_0F13,
  MOD_VEX_0F16_PREFIX_0,
  MOD_VEX_0F17,
  MOD_VEX_0F2B,
  MOD_VEX_0F50,
  MOD_VEX_0F71_REG_2,
  MOD_VEX_0F71_REG_4,
  MOD_VEX_0F71_REG_6,
  MOD_VEX_0F72_REG_2,
  MOD_VEX_0F72_REG_4,
  MOD_VEX_0F72_REG_6,
  MOD_VEX_0F73_REG_2,
  MOD_VEX_0F73_REG_3,
  MOD_VEX_0F73_REG_6,
  MOD_VEX_0F73_REG_7,
  MOD_VEX_0FAE_REG_2,
  MOD_VEX_0FAE_REG_3,
  MOD_VEX_0FD7_PREFIX_2,
  MOD_VEX_0FE7_PREFIX_2,
  MOD_VEX_0FF0_PREFIX_3,
  MOD_VEX_0F381A_PREFIX_2,
  MOD_VEX_0F382A_PREFIX_2,
  MOD_VEX_0F382C_PREFIX_2,
  MOD_VEX_0F382D_PREFIX_2,
  MOD_VEX_0F382E_PREFIX_2,
  MOD_VEX_0F382F_PREFIX_2,
  MOD_VEX_0F385A_PREFIX_2,
  MOD_VEX_0F388C_PREFIX_2,
  MOD_VEX_0F388E_PREFIX_2,
};

enum
{
  RM_C6_REG_7 = 0,
  RM_C7_REG_7,
  RM_0F01_REG_0,
  RM_0F01_REG_1,
  RM_0F01_REG_2,
  RM_0F01_REG_3,
  RM_0F01_REG_7,
  RM_0FAE_REG_5,
  RM_0FAE_REG_6,
  RM_0FAE_REG_7
};

enum
{
  PREFIX_90 = 0,
  PREFIX_0F10,
  PREFIX_0F11,
  PREFIX_0F12,
  PREFIX_0F16,
  PREFIX_0F2A,
  PREFIX_0F2B,
  PREFIX_0F2C,
  PREFIX_0F2D,
  PREFIX_0F2E,
  PREFIX_0F2F,
  PREFIX_0F51,
  PREFIX_0F52,
  PREFIX_0F53,
  PREFIX_0F58,
  PREFIX_0F59,
  PREFIX_0F5A,
  PREFIX_0F5B,
  PREFIX_0F5C,
  PREFIX_0F5D,
  PREFIX_0F5E,
  PREFIX_0F5F,
  PREFIX_0F60,
  PREFIX_0F61,
  PREFIX_0F62,
  PREFIX_0F6C,
  PREFIX_0F6D,
  PREFIX_0F6F,
  PREFIX_0F70,
  PREFIX_0F73_REG_3,
  PREFIX_0F73_REG_7,
  PREFIX_0F78,
  PREFIX_0F79,
  PREFIX_0F7C,
  PREFIX_0F7D,
  PREFIX_0F7E,
  PREFIX_0F7F,
  PREFIX_0FAE_REG_0,
  PREFIX_0FAE_REG_1,
  PREFIX_0FAE_REG_2,
  PREFIX_0FAE_REG_3,
  PREFIX_0FB8,
  PREFIX_0FBC,
  PREFIX_0FBD,
  PREFIX_0FC2,
  PREFIX_0FC3,
  PREFIX_0FC7_REG_6,
  PREFIX_0FD0,
  PREFIX_0FD6,
  PREFIX_0FE6,
  PREFIX_0FE7,
  PREFIX_0FF0,
  PREFIX_0FF7,
  PREFIX_0F3810,
  PREFIX_0F3814,
  PREFIX_0F3815,
  PREFIX_0F3817,
  PREFIX_0F3820,
  PREFIX_0F3821,
  PREFIX_0F3822,
  PREFIX_0F3823,
  PREFIX_0F3824,
  PREFIX_0F3825,
  PREFIX_0F3828,
  PREFIX_0F3829,
  PREFIX_0F382A,
  PREFIX_0F382B,
  PREFIX_0F3830,
  PREFIX_0F3831,
  PREFIX_0F3832,
  PREFIX_0F3833,
  PREFIX_0F3834,
  PREFIX_0F3835,
  PREFIX_0F3837,
  PREFIX_0F3838,
  PREFIX_0F3839,
  PREFIX_0F383A,
  PREFIX_0F383B,
  PREFIX_0F383C,
  PREFIX_0F383D,
  PREFIX_0F383E,
  PREFIX_0F383F,
  PREFIX_0F3840,
  PREFIX_0F3841,
  PREFIX_0F3880,
  PREFIX_0F3881,
  PREFIX_0F3882,
  PREFIX_0F38DB,
  PREFIX_0F38DC,
  PREFIX_0F38DD,
  PREFIX_0F38DE,
  PREFIX_0F38DF,
  PREFIX_0F38F0,
  PREFIX_0F38F1,
  PREFIX_0F38F6,
  PREFIX_0F3A08,
  PREFIX_0F3A09,
  PREFIX_0F3A0A,
  PREFIX_0F3A0B,
  PREFIX_0F3A0C,
  PREFIX_0F3A0D,
  PREFIX_0F3A0E,
  PREFIX_0F3A14,
  PREFIX_0F3A15,
  PREFIX_0F3A16,
  PREFIX_0F3A17,
  PREFIX_0F3A20,
  PREFIX_0F3A21,
  PREFIX_0F3A22,
  PREFIX_0F3A40,
  PREFIX_0F3A41,
  PREFIX_0F3A42,
  PREFIX_0F3A44,
  PREFIX_0F3A60,
  PREFIX_0F3A61,
  PREFIX_0F3A62,
  PREFIX_0F3A63,
  PREFIX_0F3ADF,
  PREFIX_VEX_0F10,
  PREFIX_VEX_0F11,
  PREFIX_VEX_0F12,
  PREFIX_VEX_0F16,
  PREFIX_VEX_0F2A,
  PREFIX_VEX_0F2C,
  PREFIX_VEX_0F2D,
  PREFIX_VEX_0F2E,
  PREFIX_VEX_0F2F,
  PREFIX_VEX_0F51,
  PREFIX_VEX_0F52,
  PREFIX_VEX_0F53,
  PREFIX_VEX_0F58,
  PREFIX_VEX_0F59,
  PREFIX_VEX_0F5A,
  PREFIX_VEX_0F5B,
  PREFIX_VEX_0F5C,
  PREFIX_VEX_0F5D,
  PREFIX_VEX_0F5E,
  PREFIX_VEX_0F5F,
  PREFIX_VEX_0F60,
  PREFIX_VEX_0F61,
  PREFIX_VEX_0F62,
  PREFIX_VEX_0F63,
  PREFIX_VEX_0F64,
  PREFIX_VEX_0F65,
  PREFIX_VEX_0F66,
  PREFIX_VEX_0F67,
  PREFIX_VEX_0F68,
  PREFIX_VEX_0F69,
  PREFIX_VEX_0F6A,
  PREFIX_VEX_0F6B,
  PREFIX_VEX_0F6C,
  PREFIX_VEX_0F6D,
  PREFIX_VEX_0F6E,
  PREFIX_VEX_0F6F,
  PREFIX_VEX_0F70,
  PREFIX_VEX_0F71_REG_2,
  PREFIX_VEX_0F71_REG_4,
  PREFIX_VEX_0F71_REG_6,
  PREFIX_VEX_0F72_REG_2,
  PREFIX_VEX_0F72_REG_4,
  PREFIX_VEX_0F72_REG_6,
  PREFIX_VEX_0F73_REG_2,
  PREFIX_VEX_0F73_REG_3,
  PREFIX_VEX_0F73_REG_6,
  PREFIX_VEX_0F73_REG_7,
  PREFIX_VEX_0F74,
  PREFIX_VEX_0F75,
  PREFIX_VEX_0F76,
  PREFIX_VEX_0F77,
  PREFIX_VEX_0F7C,
  PREFIX_VEX_0F7D,
  PREFIX_VEX_0F7E,
  PREFIX_VEX_0F7F,
  PREFIX_VEX_0FC2,
  PREFIX_VEX_0FC4,
  PREFIX_VEX_0FC5,
  PREFIX_VEX_0FD0,
  PREFIX_VEX_0FD1,
  PREFIX_VEX_0FD2,
  PREFIX_VEX_0FD3,
  PREFIX_VEX_0FD4,
  PREFIX_VEX_0FD5,
  PREFIX_VEX_0FD6,
  PREFIX_VEX_0FD7,
  PREFIX_VEX_0FD8,
  PREFIX_VEX_0FD9,
  PREFIX_VEX_0FDA,
  PREFIX_VEX_0FDB,
  PREFIX_VEX_0FDC,
  PREFIX_VEX_0FDD,
  PREFIX_VEX_0FDE,
  PREFIX_VEX_0FDF,
  PREFIX_VEX_0FE0,
  PREFIX_VEX_0FE1,
  PREFIX_VEX_0FE2,
  PREFIX_VEX_0FE3,
  PREFIX_VEX_0FE4,
  PREFIX_VEX_0FE5,
  PREFIX_VEX_0FE6,
  PREFIX_VEX_0FE7,
  PREFIX_VEX_0FE8,
  PREFIX_VEX_0FE9,
  PREFIX_VEX_0FEA,
  PREFIX_VEX_0FEB,
  PREFIX_VEX_0FEC,
  PREFIX_VEX_0FED,
  PREFIX_VEX_0FEE,
  PREFIX_VEX_0FEF,
  PREFIX_VEX_0FF0,
  PREFIX_VEX_0FF1,
  PREFIX_VEX_0FF2,
  PREFIX_VEX_0FF3,
  PREFIX_VEX_0FF4,
  PREFIX_VEX_0FF5,
  PREFIX_VEX_0FF6,
  PREFIX_VEX_0FF7,
  PREFIX_VEX_0FF8,
  PREFIX_VEX_0FF9,
  PREFIX_VEX_0FFA,
  PREFIX_VEX_0FFB,
  PREFIX_VEX_0FFC,
  PREFIX_VEX_0FFD,
  PREFIX_VEX_0FFE,
  PREFIX_VEX_0F3800,
  PREFIX_VEX_0F3801,
  PREFIX_VEX_0F3802,
  PREFIX_VEX_0F3803,
  PREFIX_VEX_0F3804,
  PREFIX_VEX_0F3805,
  PREFIX_VEX_0F3806,
  PREFIX_VEX_0F3807,
  PREFIX_VEX_0F3808,
  PREFIX_VEX_0F3809,
  PREFIX_VEX_0F380A,
  PREFIX_VEX_0F380B,
  PREFIX_VEX_0F380C,
  PREFIX_VEX_0F380D,
  PREFIX_VEX_0F380E,
  PREFIX_VEX_0F380F,
  PREFIX_VEX_0F3813,
  PREFIX_VEX_0F3816,
  PREFIX_VEX_0F3817,
  PREFIX_VEX_0F3818,
  PREFIX_VEX_0F3819,
  PREFIX_VEX_0F381A,
  PREFIX_VEX_0F381C,
  PREFIX_VEX_0F381D,
  PREFIX_VEX_0F381E,
  PREFIX_VEX_0F3820,
  PREFIX_VEX_0F3821,
  PREFIX_VEX_0F3822,
  PREFIX_VEX_0F3823,
  PREFIX_VEX_0F3824,
  PREFIX_VEX_0F3825,
  PREFIX_VEX_0F3828,
  PREFIX_VEX_0F3829,
  PREFIX_VEX_0F382A,
  PREFIX_VEX_0F382B,
  PREFIX_VEX_0F382C,
  PREFIX_VEX_0F382D,
  PREFIX_VEX_0F382E,
  PREFIX_VEX_0F382F,
  PREFIX_VEX_0F3830,
  PREFIX_VEX_0F3831,
  PREFIX_VEX_0F3832,
  PREFIX_VEX_0F3833,
  PREFIX_VEX_0F3834,
  PREFIX_VEX_0F3835,
  PREFIX_VEX_0F3836,
  PREFIX_VEX_0F3837,
  PREFIX_VEX_0F3838,
  PREFIX_VEX_0F3839,
  PREFIX_VEX_0F383A,
  PREFIX_VEX_0F383B,
  PREFIX_VEX_0F383C,
  PREFIX_VEX_0F383D,
  PREFIX_VEX_0F383E,
  PREFIX_VEX_0F383F,
  PREFIX_VEX_0F3840,
  PREFIX_VEX_0F3841,
  PREFIX_VEX_0F3845,
  PREFIX_VEX_0F3846,
  PREFIX_VEX_0F3847,
  PREFIX_VEX_0F3858,
  PREFIX_VEX_0F3859,
  PREFIX_VEX_0F385A,
  PREFIX_VEX_0F3878,
  PREFIX_VEX_0F3879,
  PREFIX_VEX_0F388C,
  PREFIX_VEX_0F388E,
  PREFIX_VEX_0F3890,
  PREFIX_VEX_0F3891,
  PREFIX_VEX_0F3892,
  PREFIX_VEX_0F3893,
  PREFIX_VEX_0F3896,
  PREFIX_VEX_0F3897,
  PREFIX_VEX_0F3898,
  PREFIX_VEX_0F3899,
  PREFIX_VEX_0F389A,
  PREFIX_VEX_0F389B,
  PREFIX_VEX_0F389C,
  PREFIX_VEX_0F389D,
  PREFIX_VEX_0F389E,
  PREFIX_VEX_0F389F,
  PREFIX_VEX_0F38A6,
  PREFIX_VEX_0F38A7,
  PREFIX_VEX_0F38A8,
  PREFIX_VEX_0F38A9,
  PREFIX_VEX_0F38AA,
  PREFIX_VEX_0F38AB,
  PREFIX_VEX_0F38AC,
  PREFIX_VEX_0F38AD,
  PREFIX_VEX_0F38AE,
  PREFIX_VEX_0F38AF,
  PREFIX_VEX_0F38B6,
  PREFIX_VEX_0F38B7,
  PREFIX_VEX_0F38B8,
  PREFIX_VEX_0F38B9,
  PREFIX_VEX_0F38BA,
  PREFIX_VEX_0F38BB,
  PREFIX_VEX_0F38BC,
  PREFIX_VEX_0F38BD,
  PREFIX_VEX_0F38BE,
  PREFIX_VEX_0F38BF,
  PREFIX_VEX_0F38DB,
  PREFIX_VEX_0F38DC,
  PREFIX_VEX_0F38DD,
  PREFIX_VEX_0F38DE,
  PREFIX_VEX_0F38DF,
  PREFIX_VEX_0F38F2,
  PREFIX_VEX_0F38F3_REG_1,
  PREFIX_VEX_0F38F3_REG_2,
  PREFIX_VEX_0F38F3_REG_3,
  PREFIX_VEX_0F38F5,
  PREFIX_VEX_0F38F6,
  PREFIX_VEX_0F38F7,
  PREFIX_VEX_0F3A00,
  PREFIX_VEX_0F3A01,
  PREFIX_VEX_0F3A02,
  PREFIX_VEX_0F3A04,
  PREFIX_VEX_0F3A05,
  PREFIX_VEX_0F3A06,
  PREFIX_VEX_0F3A08,
  PREFIX_VEX_0F3A09,
  PREFIX_VEX_0F3A0A,
  PREFIX_VEX_0F3A0B,
  PREFIX_VEX_0F3A0C,
  PREFIX_VEX_0F3A0D,
  PREFIX_VEX_0F3A0E,
  PREFIX_VEX_0F3A0F,
  PREFIX_VEX_0F3A14,
  PREFIX_VEX_0F3A15,
  PREFIX_VEX_0F3A16,
  PREFIX_VEX_0F3A17,
  PREFIX_VEX_0F3A18,
  PREFIX_VEX_0F3A19,
  PREFIX_VEX_0F3A1D,
  PREFIX_VEX_0F3A20,
  PREFIX_VEX_0F3A21,
  PREFIX_VEX_0F3A22,
  PREFIX_VEX_0F3A38,
  PREFIX_VEX_0F3A39,
  PREFIX_VEX_0F3A40,
  PREFIX_VEX_0F3A41,
  PREFIX_VEX_0F3A42,
  PREFIX_VEX_0F3A44,
  PREFIX_VEX_0F3A46,
  PREFIX_VEX_0F3A48,
  PREFIX_VEX_0F3A49,
  PREFIX_VEX_0F3A4A,
  PREFIX_VEX_0F3A4B,
  PREFIX_VEX_0F3A4C,
  PREFIX_VEX_0F3A5C,
  PREFIX_VEX_0F3A5D,
  PREFIX_VEX_0F3A5E,
  PREFIX_VEX_0F3A5F,
  PREFIX_VEX_0F3A60,
  PREFIX_VEX_0F3A61,
  PREFIX_VEX_0F3A62,
  PREFIX_VEX_0F3A63,
  PREFIX_VEX_0F3A68,
  PREFIX_VEX_0F3A69,
  PREFIX_VEX_0F3A6A,
  PREFIX_VEX_0F3A6B,
  PREFIX_VEX_0F3A6C,
  PREFIX_VEX_0F3A6D,
  PREFIX_VEX_0F3A6E,
  PREFIX_VEX_0F3A6F,
  PREFIX_VEX_0F3A78,
  PREFIX_VEX_0F3A79,
  PREFIX_VEX_0F3A7A,
  PREFIX_VEX_0F3A7B,
  PREFIX_VEX_0F3A7C,
  PREFIX_VEX_0F3A7D,
  PREFIX_VEX_0F3A7E,
  PREFIX_VEX_0F3A7F,
  PREFIX_VEX_0F3ADF,
  PREFIX_VEX_0F3AF0
};

enum
{
  X86_64_06 = 0,
  X86_64_07,
  X86_64_0D,
  X86_64_16,
  X86_64_17,
  X86_64_1E,
  X86_64_1F,
  X86_64_27,
  X86_64_2F,
  X86_64_37,
  X86_64_3F,
  X86_64_60,
  X86_64_61,
  X86_64_62,
  X86_64_63,
  X86_64_6D,
  X86_64_6F,
  X86_64_9A,
  X86_64_C4,
  X86_64_C5,
  X86_64_CE,
  X86_64_D4,
  X86_64_D5,
  X86_64_EA,
  X86_64_0F01_REG_0,
  X86_64_0F01_REG_1,
  X86_64_0F01_REG_2,
  X86_64_0F01_REG_3
};

enum
{
  THREE_BYTE_0F38 = 0,
  THREE_BYTE_0F3A,
  THREE_BYTE_0F7A
};

enum
{
  XOP_08 = 0,
  XOP_09,
  XOP_0A
};

enum
{
  VEX_0F = 0,
  VEX_0F38,
  VEX_0F3A
};

enum
{
  VEX_LEN_0F10_P_1 = 0,
  VEX_LEN_0F10_P_3,
  VEX_LEN_0F11_P_1,
  VEX_LEN_0F11_P_3,
  VEX_LEN_0F12_P_0_M_0,
  VEX_LEN_0F12_P_0_M_1,
  VEX_LEN_0F12_P_2,
  VEX_LEN_0F13_M_0,
  VEX_LEN_0F16_P_0_M_0,
  VEX_LEN_0F16_P_0_M_1,
  VEX_LEN_0F16_P_2,
  VEX_LEN_0F17_M_0,
  VEX_LEN_0F2A_P_1,
  VEX_LEN_0F2A_P_3,
  VEX_LEN_0F2C_P_1,
  VEX_LEN_0F2C_P_3,
  VEX_LEN_0F2D_P_1,
  VEX_LEN_0F2D_P_3,
  VEX_LEN_0F2E_P_0,
  VEX_LEN_0F2E_P_2,
  VEX_LEN_0F2F_P_0,
  VEX_LEN_0F2F_P_2,
  VEX_LEN_0F51_P_1,
  VEX_LEN_0F51_P_3,
  VEX_LEN_0F52_P_1,
  VEX_LEN_0F53_P_1,
  VEX_LEN_0F58_P_1,
  VEX_LEN_0F58_P_3,
  VEX_LEN_0F59_P_1,
  VEX_LEN_0F59_P_3,
  VEX_LEN_0F5A_P_1,
  VEX_LEN_0F5A_P_3,
  VEX_LEN_0F5C_P_1,
  VEX_LEN_0F5C_P_3,
  VEX_LEN_0F5D_P_1,
  VEX_LEN_0F5D_P_3,
  VEX_LEN_0F5E_P_1,
  VEX_LEN_0F5E_P_3,
  VEX_LEN_0F5F_P_1,
  VEX_LEN_0F5F_P_3,
  VEX_LEN_0F6E_P_2,
  VEX_LEN_0F7E_P_1,
  VEX_LEN_0F7E_P_2,
  VEX_LEN_0FAE_R_2_M_0,
  VEX_LEN_0FAE_R_3_M_0,
  VEX_LEN_0FC2_P_1,
  VEX_LEN_0FC2_P_3,
  VEX_LEN_0FC4_P_2,
  VEX_LEN_0FC5_P_2,
  VEX_LEN_0FD6_P_2,
  VEX_LEN_0FF7_P_2,
  VEX_LEN_0F3816_P_2,
  VEX_LEN_0F3819_P_2,
  VEX_LEN_0F381A_P_2_M_0,
  VEX_LEN_0F3836_P_2,
  VEX_LEN_0F3841_P_2,
  VEX_LEN_0F385A_P_2_M_0,
  VEX_LEN_0F38DB_P_2,
  VEX_LEN_0F38DC_P_2,
  VEX_LEN_0F38DD_P_2,
  VEX_LEN_0F38DE_P_2,
  VEX_LEN_0F38DF_P_2,
  VEX_LEN_0F38F2_P_0,
  VEX_LEN_0F38F3_R_1_P_0,
  VEX_LEN_0F38F3_R_2_P_0,
  VEX_LEN_0F38F3_R_3_P_0,
  VEX_LEN_0F38F5_P_0,
  VEX_LEN_0F38F5_P_1,
  VEX_LEN_0F38F5_P_3,
  VEX_LEN_0F38F6_P_3,
  VEX_LEN_0F38F7_P_0,
  VEX_LEN_0F38F7_P_1,
  VEX_LEN_0F38F7_P_2,
  VEX_LEN_0F38F7_P_3,
  VEX_LEN_0F3A00_P_2,
  VEX_LEN_0F3A01_P_2,
  VEX_LEN_0F3A06_P_2,
  VEX_LEN_0F3A0A_P_2,
  VEX_LEN_0F3A0B_P_2,
  VEX_LEN_0F3A14_P_2,
  VEX_LEN_0F3A15_P_2,
  VEX_LEN_0F3A16_P_2,
  VEX_LEN_0F3A17_P_2,
  VEX_LEN_0F3A18_P_2,
  VEX_LEN_0F3A19_P_2,
  VEX_LEN_0F3A20_P_2,
  VEX_LEN_0F3A21_P_2,
  VEX_LEN_0F3A22_P_2,
  VEX_LEN_0F3A38_P_2,
  VEX_LEN_0F3A39_P_2,
  VEX_LEN_0F3A41_P_2,
  VEX_LEN_0F3A44_P_2,
  VEX_LEN_0F3A46_P_2,
  VEX_LEN_0F3A60_P_2,
  VEX_LEN_0F3A61_P_2,
  VEX_LEN_0F3A62_P_2,
  VEX_LEN_0F3A63_P_2,
  VEX_LEN_0F3A6A_P_2,
  VEX_LEN_0F3A6B_P_2,
  VEX_LEN_0F3A6E_P_2,
  VEX_LEN_0F3A6F_P_2,
  VEX_LEN_0F3A7A_P_2,
  VEX_LEN_0F3A7B_P_2,
  VEX_LEN_0F3A7E_P_2,
  VEX_LEN_0F3A7F_P_2,
  VEX_LEN_0F3ADF_P_2,
  VEX_LEN_0F3AF0_P_3,
  VEX_LEN_0FXOP_08_CC,
  VEX_LEN_0FXOP_08_CD,
  VEX_LEN_0FXOP_08_CE,
  VEX_LEN_0FXOP_08_CF,
  VEX_LEN_0FXOP_08_EC,
  VEX_LEN_0FXOP_08_ED,
  VEX_LEN_0FXOP_08_EE,
  VEX_LEN_0FXOP_08_EF,
  VEX_LEN_0FXOP_09_80,
  VEX_LEN_0FXOP_09_81
};

enum
{
  VEX_W_0F10_P_0 = 0,
  VEX_W_0F10_P_1,
  VEX_W_0F10_P_2,
  VEX_W_0F10_P_3,
  VEX_W_0F11_P_0,
  VEX_W_0F11_P_1,
  VEX_W_0F11_P_2,
  VEX_W_0F11_P_3,
  VEX_W_0F12_P_0_M_0,
  VEX_W_0F12_P_0_M_1,
  VEX_W_0F12_P_1,
  VEX_W_0F12_P_2,
  VEX_W_0F12_P_3,
  VEX_W_0F13_M_0,
  VEX_W_0F14,
  VEX_W_0F15,
  VEX_W_0F16_P_0_M_0,
  VEX_W_0F16_P_0_M_1,
  VEX_W_0F16_P_1,
  VEX_W_0F16_P_2,
  VEX_W_0F17_M_0,
  VEX_W_0F28,
  VEX_W_0F29,
  VEX_W_0F2B_M_0,
  VEX_W_0F2E_P_0,
  VEX_W_0F2E_P_2,
  VEX_W_0F2F_P_0,
  VEX_W_0F2F_P_2,
  VEX_W_0F50_M_0,
  VEX_W_0F51_P_0,
  VEX_W_0F51_P_1,
  VEX_W_0F51_P_2,
  VEX_W_0F51_P_3,
  VEX_W_0F52_P_0,
  VEX_W_0F52_P_1,
  VEX_W_0F53_P_0,
  VEX_W_0F53_P_1,
  VEX_W_0F58_P_0,
  VEX_W_0F58_P_1,
  VEX_W_0F58_P_2,
  VEX_W_0F58_P_3,
  VEX_W_0F59_P_0,
  VEX_W_0F59_P_1,
  VEX_W_0F59_P_2,
  VEX_W_0F59_P_3,
  VEX_W_0F5A_P_0,
  VEX_W_0F5A_P_1,
  VEX_W_0F5A_P_3,
  VEX_W_0F5B_P_0,
  VEX_W_0F5B_P_1,
  VEX_W_0F5B_P_2,
  VEX_W_0F5C_P_0,
  VEX_W_0F5C_P_1,
  VEX_W_0F5C_P_2,
  VEX_W_0F5C_P_3,
  VEX_W_0F5D_P_0,
  VEX_W_0F5D_P_1,
  VEX_W_0F5D_P_2,
  VEX_W_0F5D_P_3,
  VEX_W_0F5E_P_0,
  VEX_W_0F5E_P_1,
  VEX_W_0F5E_P_2,
  VEX_W_0F5E_P_3,
  VEX_W_0F5F_P_0,
  VEX_W_0F5F_P_1,
  VEX_W_0F5F_P_2,
  VEX_W_0F5F_P_3,
  VEX_W_0F60_P_2,
  VEX_W_0F61_P_2,
  VEX_W_0F62_P_2,
  VEX_W_0F63_P_2,
  VEX_W_0F64_P_2,
  VEX_W_0F65_P_2,
  VEX_W_0F66_P_2,
  VEX_W_0F67_P_2,
  VEX_W_0F68_P_2,
  VEX_W_0F69_P_2,
  VEX_W_0F6A_P_2,
  VEX_W_0F6B_P_2,
  VEX_W_0F6C_P_2,
  VEX_W_0F6D_P_2,
  VEX_W_0F6F_P_1,
  VEX_W_0F6F_P_2,
  VEX_W_0F70_P_1,
  VEX_W_0F70_P_2,
  VEX_W_0F70_P_3,
  VEX_W_0F71_R_2_P_2,
  VEX_W_0F71_R_4_P_2,
  VEX_W_0F71_R_6_P_2,
  VEX_W_0F72_R_2_P_2,
  VEX_W_0F72_R_4_P_2,
  VEX_W_0F72_R_6_P_2,
  VEX_W_0F73_R_2_P_2,
  VEX_W_0F73_R_3_P_2,
  VEX_W_0F73_R_6_P_2,
  VEX_W_0F73_R_7_P_2,
  VEX_W_0F74_P_2,
  VEX_W_0F75_P_2,
  VEX_W_0F76_P_2,
  VEX_W_0F77_P_0,
  VEX_W_0F7C_P_2,
  VEX_W_0F7C_P_3,
  VEX_W_0F7D_P_2,
  VEX_W_0F7D_P_3,
  VEX_W_0F7E_P_1,
  VEX_W_0F7F_P_1,
  VEX_W_0F7F_P_2,
  VEX_W_0FAE_R_2_M_0,
  VEX_W_0FAE_R_3_M_0,
  VEX_W_0FC2_P_0,
  VEX_W_0FC2_P_1,
  VEX_W_0FC2_P_2,
  VEX_W_0FC2_P_3,
  VEX_W_0FC4_P_2,
  VEX_W_0FC5_P_2,
  VEX_W_0FD0_P_2,
  VEX_W_0FD0_P_3,
  VEX_W_0FD1_P_2,
  VEX_W_0FD2_P_2,
  VEX_W_0FD3_P_2,
  VEX_W_0FD4_P_2,
  VEX_W_0FD5_P_2,
  VEX_W_0FD6_P_2,
  VEX_W_0FD7_P_2_M_1,
  VEX_W_0FD8_P_2,
  VEX_W_0FD9_P_2,
  VEX_W_0FDA_P_2,
  VEX_W_0FDB_P_2,
  VEX_W_0FDC_P_2,
  VEX_W_0FDD_P_2,
  VEX_W_0FDE_P_2,
  VEX_W_0FDF_P_2,
  VEX_W_0FE0_P_2,
  VEX_W_0FE1_P_2,
  VEX_W_0FE2_P_2,
  VEX_W_0FE3_P_2,
  VEX_W_0FE4_P_2,
  VEX_W_0FE5_P_2,
  VEX_W_0FE6_P_1,
  VEX_W_0FE6_P_2,
  VEX_W_0FE6_P_3,
  VEX_W_0FE7_P_2_M_0,
  VEX_W_0FE8_P_2,
  VEX_W_0FE9_P_2,
  VEX_W_0FEA_P_2,
  VEX_W_0FEB_P_2,
  VEX_W_0FEC_P_2,
  VEX_W_0FED_P_2,
  VEX_W_0FEE_P_2,
  VEX_W_0FEF_P_2,
  VEX_W_0FF0_P_3_M_0,
  VEX_W_0FF1_P_2,
  VEX_W_0FF2_P_2,
  VEX_W_0FF3_P_2,
  VEX_W_0FF4_P_2,
  VEX_W_0FF5_P_2,
  VEX_W_0FF6_P_2,
  VEX_W_0FF7_P_2,
  VEX_W_0FF8_P_2,
  VEX_W_0FF9_P_2,
  VEX_W_0FFA_P_2,
  VEX_W_0FFB_P_2,
  VEX_W_0FFC_P_2,
  VEX_W_0FFD_P_2,
  VEX_W_0FFE_P_2,
  VEX_W_0F3800_P_2,
  VEX_W_0F3801_P_2,
  VEX_W_0F3802_P_2,
  VEX_W_0F3803_P_2,
  VEX_W_0F3804_P_2,
  VEX_W_0F3805_P_2,
  VEX_W_0F3806_P_2,
  VEX_W_0F3807_P_2,
  VEX_W_0F3808_P_2,
  VEX_W_0F3809_P_2,
  VEX_W_0F380A_P_2,
  VEX_W_0F380B_P_2,
  VEX_W_0F380C_P_2,
  VEX_W_0F380D_P_2,
  VEX_W_0F380E_P_2,
  VEX_W_0F380F_P_2,
  VEX_W_0F3816_P_2,
  VEX_W_0F3817_P_2,
  VEX_W_0F3818_P_2,
  VEX_W_0F3819_P_2,
  VEX_W_0F381A_P_2_M_0,
  VEX_W_0F381C_P_2,
  VEX_W_0F381D_P_2,
  VEX_W_0F381E_P_2,
  VEX_W_0F3820_P_2,
  VEX_W_0F3821_P_2,
  VEX_W_0F3822_P_2,
  VEX_W_0F3823_P_2,
  VEX_W_0F3824_P_2,
  VEX_W_0F3825_P_2,
  VEX_W_0F3828_P_2,
  VEX_W_0F3829_P_2,
  VEX_W_0F382A_P_2_M_0,
  VEX_W_0F382B_P_2,
  VEX_W_0F382C_P_2_M_0,
  VEX_W_0F382D_P_2_M_0,
  VEX_W_0F382E_P_2_M_0,
  VEX_W_0F382F_P_2_M_0,
  VEX_W_0F3830_P_2,
  VEX_W_0F3831_P_2,
  VEX_W_0F3832_P_2,
  VEX_W_0F3833_P_2,
  VEX_W_0F3834_P_2,
  VEX_W_0F3835_P_2,
  VEX_W_0F3836_P_2,
  VEX_W_0F3837_P_2,
  VEX_W_0F3838_P_2,
  VEX_W_0F3839_P_2,
  VEX_W_0F383A_P_2,
  VEX_W_0F383B_P_2,
  VEX_W_0F383C_P_2,
  VEX_W_0F383D_P_2,
  VEX_W_0F383E_P_2,
  VEX_W_0F383F_P_2,
  VEX_W_0F3840_P_2,
  VEX_W_0F3841_P_2,
  VEX_W_0F3846_P_2,
  VEX_W_0F3858_P_2,
  VEX_W_0F3859_P_2,
  VEX_W_0F385A_P_2_M_0,
  VEX_W_0F3878_P_2,
  VEX_W_0F3879_P_2,
  VEX_W_0F38DB_P_2,
  VEX_W_0F38DC_P_2,
  VEX_W_0F38DD_P_2,
  VEX_W_0F38DE_P_2,
  VEX_W_0F38DF_P_2,
  VEX_W_0F3A00_P_2,
  VEX_W_0F3A01_P_2,
  VEX_W_0F3A02_P_2,
  VEX_W_0F3A04_P_2,
  VEX_W_0F3A05_P_2,
  VEX_W_0F3A06_P_2,
  VEX_W_0F3A08_P_2,
  VEX_W_0F3A09_P_2,
  VEX_W_0F3A0A_P_2,
  VEX_W_0F3A0B_P_2,
  VEX_W_0F3A0C_P_2,
  VEX_W_0F3A0D_P_2,
  VEX_W_0F3A0E_P_2,
  VEX_W_0F3A0F_P_2,
  VEX_W_0F3A14_P_2,
  VEX_W_0F3A15_P_2,
  VEX_W_0F3A18_P_2,
  VEX_W_0F3A19_P_2,
  VEX_W_0F3A20_P_2,
  VEX_W_0F3A21_P_2,
  VEX_W_0F3A38_P_2,
  VEX_W_0F3A39_P_2,
  VEX_W_0F3A40_P_2,
  VEX_W_0F3A41_P_2,
  VEX_W_0F3A42_P_2,
  VEX_W_0F3A44_P_2,
  VEX_W_0F3A46_P_2,
  VEX_W_0F3A48_P_2,
  VEX_W_0F3A49_P_2,
  VEX_W_0F3A4A_P_2,
  VEX_W_0F3A4B_P_2,
  VEX_W_0F3A4C_P_2,
  VEX_W_0F3A60_P_2,
  VEX_W_0F3A61_P_2,
  VEX_W_0F3A62_P_2,
  VEX_W_0F3A63_P_2,
  VEX_W_0F3ADF_P_2
};

typedef void (*op_rtn) (int bytemode, int sizeflag);

struct dis386 {
  const char *name;
  struct
    {
      op_rtn rtn;
      int bytemode;
    } op[MAX_OPERANDS];
};

/* Upper case letters in the instruction names here are macros.
   'A' => print 'b' if no register operands or suffix_always is true
   'B' => print 'b' if suffix_always is true
   'C' => print 's' or 'l' ('w' or 'd' in Intel mode) depending on operand
	  size prefix
   'D' => print 'w' if no register operands or 'w', 'l' or 'q', if
	  suffix_always is true
   'E' => print 'e' if 32-bit form of jcxz
   'F' => print 'w' or 'l' depending on address size prefix (loop insns)
   'G' => print 'w' or 'l' depending on operand size prefix (i/o insns)
   'H' => print ",pt" or ",pn" branch hint
   'I' => honor following macro letter even in Intel mode (implemented only
	  for some of the macro letters)
   'J' => print 'l'
   'K' => print 'd' or 'q' if rex prefix is present.
   'L' => print 'l' if suffix_always is true
   'M' => print 'r' if intel_mnemonic is false.
   'N' => print 'n' if instruction has no wait "prefix"
   'O' => print 'd' or 'o' (or 'q' in Intel mode)
   'P' => print 'w', 'l' or 'q' if instruction has an operand size prefix,
	  or suffix_always is true.  print 'q' if rex prefix is present.
   'Q' => print 'w', 'l' or 'q' for memory operand or suffix_always
	  is true
   'R' => print 'w', 'l' or 'q' ('d' for 'l' and 'e' in Intel mode)
   'S' => print 'w', 'l' or 'q' if suffix_always is true
   'T' => print 'q' in 64bit mode and behave as 'P' otherwise
   'U' => print 'q' in 64bit mode and behave as 'Q' otherwise
   'V' => print 'q' in 64bit mode and behave as 'S' otherwise
   'W' => print 'b', 'w' or 'l' ('d' in Intel mode)
   'X' => print 's', 'd' depending on data16 prefix (for XMM)
   'Y' => 'q' if instruction has an REX 64bit overwrite prefix and
	  suffix_always is true.
   'Z' => print 'q' in 64bit mode and behave as 'L' otherwise
   '!' => change condition from true to false or from false to true.
   '%' => add 1 upper case letter to the macro.

   2 upper case letter macros:
   "XY" => print 'x' or 'y' if no register operands or suffix_always
	   is true.
   "XW" => print 's', 'd' depending on the VEX.W bit (for FMA)
   "LQ" => print 'l' ('d' in Intel mode) or 'q' for memory operand
	   or suffix_always is true
   "LB" => print "abs" in 64bit mode and behave as 'B' otherwise
   "LS" => print "abs" in 64bit mode and behave as 'S' otherwise
   "LV" => print "abs" for 64bit operand and behave as 'S' otherwise
   "LW" => print 'd', 'q' depending on the VEX.W bit

   Many of the above letters print nothing in Intel mode.  See "putop"
   for the details.

   Braces '{' and '}', and vertical bars '|', indicate alternative
   mnemonic strings for AT&T and Intel.  */

static const struct dis386 dis386[] = {
  /* 00 */
  { "addB",		{ Ebh1, Gb } },
  { "addS",		{ Evh1, Gv } },
  { "addB",		{ Gb, EbS } },
  { "addS",		{ Gv, EvS } },
  { "addB",		{ AL, Ib } },
  { "addS",		{ eAX, Iv } },
  { X86_64_TABLE (X86_64_06) },
  { X86_64_TABLE (X86_64_07) },
  /* 08 */
  { "orB",		{ Ebh1, Gb } },
  { "orS",		{ Evh1, Gv } },
  { "orB",		{ Gb, EbS } },
  { "orS",		{ Gv, EvS } },
  { "orB",		{ AL, Ib } },
  { "orS",		{ eAX, Iv } },
  { X86_64_TABLE (X86_64_0D) },
  { Bad_Opcode },	/* 0x0f extended opcode escape */
  /* 10 */
  { "adcB",		{ Ebh1, Gb } },
  { "adcS",		{ Evh1, Gv } },
  { "adcB",		{ Gb, EbS } },
  { "adcS",		{ Gv, EvS } },
  { "adcB",		{ AL, Ib } },
  { "adcS",		{ eAX, Iv } },
  { X86_64_TABLE (X86_64_16) },
  { X86_64_TABLE (X86_64_17) },
  /* 18 */
  { "sbbB",		{ Ebh1, Gb } },
  { "sbbS",		{ Evh1, Gv } },
  { "sbbB",		{ Gb, EbS } },
  { "sbbS",		{ Gv, EvS } },
  { "sbbB",		{ AL, Ib } },
  { "sbbS",		{ eAX, Iv } },
  { X86_64_TABLE (X86_64_1E) },
  { X86_64_TABLE (X86_64_1F) },
  /* 20 */
  { "andB",		{ Ebh1, Gb } },
  { "andS",		{ Evh1, Gv } },
  { "andB",		{ Gb, EbS } },
  { "andS",		{ Gv, EvS } },
  { "andB",		{ AL, Ib } },
  { "andS",		{ eAX, Iv } },
  { Bad_Opcode },	/* SEG ES prefix */
  { X86_64_TABLE (X86_64_27) },
  /* 28 */
  { "subB",		{ Ebh1, Gb } },
  { "subS",		{ Evh1, Gv } },
  { "subB",		{ Gb, EbS } },
  { "subS",		{ Gv, EvS } },
  { "subB",		{ AL, Ib } },
  { "subS",		{ eAX, Iv } },
  { Bad_Opcode },	/* SEG CS prefix */
  { X86_64_TABLE (X86_64_2F) },
  /* 30 */
  { "xorB",		{ Ebh1, Gb } },
  { "xorS",		{ Evh1, Gv } },
  { "xorB",		{ Gb, EbS } },
  { "xorS",		{ Gv, EvS } },
  { "xorB",		{ AL, Ib } },
  { "xorS",		{ eAX, Iv } },
  { Bad_Opcode },	/* SEG SS prefix */
  { X86_64_TABLE (X86_64_37) },
  /* 38 */
  { "cmpB",		{ Eb, Gb } },
  { "cmpS",		{ Ev, Gv } },
  { "cmpB",		{ Gb, EbS } },
  { "cmpS",		{ Gv, EvS } },
  { "cmpB",		{ AL, Ib } },
  { "cmpS",		{ eAX, Iv } },
  { Bad_Opcode },	/* SEG DS prefix */
  { X86_64_TABLE (X86_64_3F) },
  /* 40 */
  { "inc{S|}",		{ RMeAX } },
  { "inc{S|}",		{ RMeCX } },
  { "inc{S|}",		{ RMeDX } },
  { "inc{S|}",		{ RMeBX } },
  { "inc{S|}",		{ RMeSP } },
  { "inc{S|}",		{ RMeBP } },
  { "inc{S|}",		{ RMeSI } },
  { "inc{S|}",		{ RMeDI } },
  /* 48 */
  { "dec{S|}",		{ RMeAX } },
  { "dec{S|}",		{ RMeCX } },
  { "dec{S|}",		{ RMeDX } },
  { "dec{S|}",		{ RMeBX } },
  { "dec{S|}",		{ RMeSP } },
  { "dec{S|}",		{ RMeBP } },
  { "dec{S|}",		{ RMeSI } },
  { "dec{S|}",		{ RMeDI } },
  /* 50 */
  { "pushV",		{ RMrAX } },
  { "pushV",		{ RMrCX } },
  { "pushV",		{ RMrDX } },
  { "pushV",		{ RMrBX } },
  { "pushV",		{ RMrSP } },
  { "pushV",		{ RMrBP } },
  { "pushV",		{ RMrSI } },
  { "pushV",		{ RMrDI } },
  /* 58 */
  { "popV",		{ RMrAX } },
  { "popV",		{ RMrCX } },
  { "popV",		{ RMrDX } },
  { "popV",		{ RMrBX } },
  { "popV",		{ RMrSP } },
  { "popV",		{ RMrBP } },
  { "popV",		{ RMrSI } },
  { "popV",		{ RMrDI } },
  /* 60 */
  { X86_64_TABLE (X86_64_60) },
  { X86_64_TABLE (X86_64_61) },
  { X86_64_TABLE (X86_64_62) },
  { X86_64_TABLE (X86_64_63) },
  { Bad_Opcode },	/* seg fs */
  { Bad_Opcode },	/* seg gs */
  { Bad_Opcode },	/* op size prefix */
  { Bad_Opcode },	/* adr size prefix */
  /* 68 */
  { "pushT",		{ sIv } },
  { "imulS",		{ Gv, Ev, Iv } },
  { "pushT",		{ sIbT } },
  { "imulS",		{ Gv, Ev, sIb } },
  { "ins{b|}",		{ Ybr, indirDX } },
  { X86_64_TABLE (X86_64_6D) },
  { "outs{b|}",		{ indirDXr, Xb } },
  { X86_64_TABLE (X86_64_6F) },
  /* 70 */
  { "joH",		{ Jb, XX, cond_jump_flag } },
  { "jnoH",		{ Jb, XX, cond_jump_flag } },
  { "jbH",		{ Jb, XX, cond_jump_flag } },
  { "jaeH",		{ Jb, XX, cond_jump_flag } },
  { "jeH",		{ Jb, XX, cond_jump_flag } },
  { "jneH",		{ Jb, XX, cond_jump_flag } },
  { "jbeH",		{ Jb, XX, cond_jump_flag } },
  { "jaH",		{ Jb, XX, cond_jump_flag } },
  /* 78 */
  { "jsH",		{ Jb, XX, cond_jump_flag } },
  { "jnsH",		{ Jb, XX, cond_jump_flag } },
  { "jpH",		{ Jb, XX, cond_jump_flag } },
  { "jnpH",		{ Jb, XX, cond_jump_flag } },
  { "jlH",		{ Jb, XX, cond_jump_flag } },
  { "jgeH",		{ Jb, XX, cond_jump_flag } },
  { "jleH",		{ Jb, XX, cond_jump_flag } },
  { "jgH",		{ Jb, XX, cond_jump_flag } },
  /* 80 */
  { REG_TABLE (REG_80) },
  { REG_TABLE (REG_81) },
  { Bad_Opcode },
  { REG_TABLE (REG_82) },
  { "testB",		{ Eb, Gb } },
  { "testS",		{ Ev, Gv } },
  { "xchgB",		{ Ebh2, Gb } },
  { "xchgS",		{ Evh2, Gv } },
  /* 88 */
  { "movB",		{ Ebh3, Gb } },
  { "movS",		{ Evh3, Gv } },
  { "movB",		{ Gb, EbS } },
  { "movS",		{ Gv, EvS } },
  { "movD",		{ Sv, Sw } },
  { MOD_TABLE (MOD_8D) },
  { "movD",		{ Sw, Sv } },
  { REG_TABLE (REG_8F) },
  /* 90 */
  { PREFIX_TABLE (PREFIX_90) },
  { "xchgS",		{ RMeCX, eAX } },
  { "xchgS",		{ RMeDX, eAX } },
  { "xchgS",		{ RMeBX, eAX } },
  { "xchgS",		{ RMeSP, eAX } },
  { "xchgS",		{ RMeBP, eAX } },
  { "xchgS",		{ RMeSI, eAX } },
  { "xchgS",		{ RMeDI, eAX } },
  /* 98 */
  { "cW{t|}R",		{ XX } },
  { "cR{t|}O",		{ XX } },
  { X86_64_TABLE (X86_64_9A) },
  { Bad_Opcode },	/* fwait */
  { "pushfT",		{ XX } },
  { "popfT",		{ XX } },
  { "sahf",		{ XX } },
  { "lahf",		{ XX } },
  /* a0 */
  { "mov%LB",		{ AL, Ob } },
  { "mov%LS",		{ eAX, Ov } },
  { "mov%LB",		{ Ob, AL } },
  { "mov%LS",		{ Ov, eAX } },
  { "movs{b|}",		{ Ybr, Xb } },
  { "movs{R|}",		{ Yvr, Xv } },
  { "cmps{b|}",		{ Xb, Yb } },
  { "cmps{R|}",		{ Xv, Yv } },
  /* a8 */
  { "testB",		{ AL, Ib } },
  { "testS",		{ eAX, Iv } },
  { "stosB",		{ Ybr, AL } },
  { "stosS",		{ Yvr, eAX } },
  { "lodsB",		{ ALr, Xb } },
  { "lodsS",		{ eAXr, Xv } },
  { "scasB",		{ AL, Yb } },
  { "scasS",		{ eAX, Yv } },
  /* b0 */
  { "movB",		{ RMAL, Ib } },
  { "movB",		{ RMCL, Ib } },
  { "movB",		{ RMDL, Ib } },
  { "movB",		{ RMBL, Ib } },
  { "movB",		{ RMAH, Ib } },
  { "movB",		{ RMCH, Ib } },
  { "movB",		{ RMDH, Ib } },
  { "movB",		{ RMBH, Ib } },
  /* b8 */
  { "mov%LV",		{ RMeAX, Iv64 } },
  { "mov%LV",		{ RMeCX, Iv64 } },
  { "mov%LV",		{ RMeDX, Iv64 } },
  { "mov%LV",		{ RMeBX, Iv64 } },
  { "mov%LV",		{ RMeSP, Iv64 } },
  { "mov%LV",		{ RMeBP, Iv64 } },
  { "mov%LV",		{ RMeSI, Iv64 } },
  { "mov%LV",		{ RMeDI, Iv64 } },
  /* c0 */
  { REG_TABLE (REG_C0) },
  { REG_TABLE (REG_C1) },
  { "retT",		{ Iw } },
  { "retT",		{ XX } },
  { X86_64_TABLE (X86_64_C4) },
  { X86_64_TABLE (X86_64_C5) },
  { REG_TABLE (REG_C6) },
  { REG_TABLE (REG_C7) },
  /* c8 */
  { "enterT",		{ Iw, Ib } },
  { "leaveT",		{ XX } },
  { "Jret{|f}P",	{ Iw } },
  { "Jret{|f}P",	{ XX } },
  { "int3",		{ XX } },
  { "int",		{ Ib } },
  { X86_64_TABLE (X86_64_CE) },
  { "iretP",		{ XX } },
  /* d0 */
  { REG_TABLE (REG_D0) },
  { REG_TABLE (REG_D1) },
  { REG_TABLE (REG_D2) },
  { REG_TABLE (REG_D3) },
  { X86_64_TABLE (X86_64_D4) },
  { X86_64_TABLE (X86_64_D5) },
  { Bad_Opcode },
  { "xlat",		{ DSBX } },
  /* d8 */
  { FLOAT },
  { FLOAT },
  { FLOAT },
  { FLOAT },
  { FLOAT },
  { FLOAT },
  { FLOAT },
  { FLOAT },
  /* e0 */
  { "loopneFH",		{ Jb, XX, loop_jcxz_flag } },
  { "loopeFH",		{ Jb, XX, loop_jcxz_flag } },
  { "loopFH",		{ Jb, XX, loop_jcxz_flag } },
  { "jEcxzH",		{ Jb, XX, loop_jcxz_flag } },
  { "inB",		{ AL, Ib } },
  { "inG",		{ zAX, Ib } },
  { "outB",		{ Ib, AL } },
  { "outG",		{ Ib, zAX } },
  /* e8 */
  { "callT",		{ Jv } },
  { "jmpT",		{ Jv } },
  { X86_64_TABLE (X86_64_EA) },
  { "jmp",		{ Jb } },
  { "inB",		{ AL, indirDX } },
  { "inG",		{ zAX, indirDX } },
  { "outB",		{ indirDX, AL } },
  { "outG",		{ indirDX, zAX } },
  /* f0 */
  { Bad_Opcode },	/* lock prefix */
  { "icebp",		{ XX } },
  { Bad_Opcode },	/* repne */
  { Bad_Opcode },	/* repz */
  { "hlt",		{ XX } },
  { "cmc",		{ XX } },
  { REG_TABLE (REG_F6) },
  { REG_TABLE (REG_F7) },
  /* f8 */
  { "clc",		{ XX } },
  { "stc",		{ XX } },
  { "cli",		{ XX } },
  { "sti",		{ XX } },
  { "cld",		{ XX } },
  { "std",		{ XX } },
  { REG_TABLE (REG_FE) },
  { REG_TABLE (REG_FF) },
};

static const struct dis386 dis386_twobyte[] = {
  /* 00 */
  { REG_TABLE (REG_0F00 ) },
  { REG_TABLE (REG_0F01 ) },
  { "larS",		{ Gv, Ew } },
  { "lslS",		{ Gv, Ew } },
  { Bad_Opcode },
  { "syscall",		{ XX } },
  { "clts",		{ XX } },
  { "sysretP",		{ XX } },
  /* 08 */
  { "invd",		{ XX } },
  { "wbinvd",		{ XX } },
  { Bad_Opcode },
  { "ud2",		{ XX } },
  { Bad_Opcode },
  { REG_TABLE (REG_0F0D) },
  { "femms",		{ XX } },
  { "",			{ MX, EM, OPSUF } }, /* See OP_3DNowSuffix.  */
  /* 10 */
  { PREFIX_TABLE (PREFIX_0F10) },
  { PREFIX_TABLE (PREFIX_0F11) },
  { PREFIX_TABLE (PREFIX_0F12) },
  { MOD_TABLE (MOD_0F13) },
  { "unpcklpX",		{ XM, EXx } },
  { "unpckhpX",		{ XM, EXx } },
  { PREFIX_TABLE (PREFIX_0F16) },
  { MOD_TABLE (MOD_0F17) },
  /* 18 */
  { REG_TABLE (REG_0F18) },
  { "nopQ",		{ Ev } },
  { "nopQ",		{ Ev } },
  { "nopQ",		{ Ev } },
  { "nopQ",		{ Ev } },
  { "nopQ",		{ Ev } },
  { "nopQ",		{ Ev } },
  { "nopQ",		{ Ev } },
  /* 20 */
  { MOD_TABLE (MOD_0F20) },
  { MOD_TABLE (MOD_0F21) },
  { MOD_TABLE (MOD_0F22) },
  { MOD_TABLE (MOD_0F23) },
  { MOD_TABLE (MOD_0F24) },
  { Bad_Opcode },
  { MOD_TABLE (MOD_0F26) },
  { Bad_Opcode },
  /* 28 */
  { "movapX",		{ XM, EXx } },
  { "movapX",		{ EXxS, XM } },
  { PREFIX_TABLE (PREFIX_0F2A) },
  { PREFIX_TABLE (PREFIX_0F2B) },
  { PREFIX_TABLE (PREFIX_0F2C) },
  { PREFIX_TABLE (PREFIX_0F2D) },
  { PREFIX_TABLE (PREFIX_0F2E) },
  { PREFIX_TABLE (PREFIX_0F2F) },
  /* 30 */
  { "wrmsr",		{ XX } },
  { "rdtsc",		{ XX } },
  { "rdmsr",		{ XX } },
  { "rdpmc",		{ XX } },
  { "sysenter",		{ XX } },
  { "sysexit",		{ XX } },
  { Bad_Opcode },
  { "getsec",		{ XX } },
  /* 38 */
  { THREE_BYTE_TABLE (THREE_BYTE_0F38) },
  { Bad_Opcode },
  { THREE_BYTE_TABLE (THREE_BYTE_0F3A) },
  { Bad_Opcode },
  { Bad_Opcode },
  { Bad_Opcode },
  { Bad_Opcode },
  { Bad_Opcode },
  /* 40 */
  { "cmovoS",		{ Gv, Ev } },
  { "cmovnoS",		{ Gv, Ev } },
  { "cmovbS",		{ Gv, Ev } },
  { "cmovaeS",		{ Gv, Ev } },
  { "cmoveS",		{ Gv, Ev } },
  { "cmovneS",		{ Gv, Ev } },
  { "cmovbeS",		{ Gv, Ev } },
  { "cmovaS",		{ Gv, Ev } },
  /* 48 */
  { "cmovsS",		{ Gv, Ev } },
  { "cmovnsS",		{ Gv, Ev } },
  { "cmovpS",		{ Gv, Ev } },
  { "cmovnpS",		{ Gv, Ev } },
  { "cmovlS",		{ Gv, Ev } },
  { "cmovgeS",		{ Gv, Ev } },
  { "cmovleS",		{ Gv, Ev } },
  { "cmovgS",		{ Gv, Ev } },
  /* 50 */
  { MOD_TABLE (MOD_0F51) },
  { PREFIX_TABLE (PREFIX_0F51) },
  { PREFIX_TABLE (PREFIX_0F52) },
  { PREFIX_TABLE (PREFIX_0F53) },
  { "andpX",		{ XM, EXx } },
  { "andnpX",		{ XM, EXx } },
  { "orpX",		{ XM, EXx } },
  { "xorpX",		{ XM, EXx } },
  /* 58 */
  { PREFIX_TABLE (PREFIX_0F58) },
  { PREFIX_TABLE (PREFIX_0F59) },
  { PREFIX_TABLE (PREFIX_0F5A) },
  { PREFIX_TABLE (PREFIX_0F5B) },
  { PREFIX_TABLE (PREFIX_0F5C) },
  { PREFIX_TABLE (PREFIX_0F5D) },
  { PREFIX_TABLE (PREFIX_0F5E) },
  { PREFIX_TABLE (PREFIX_0F5F) },
  /* 60 */
  { PREFIX_TABLE (PREFIX_0F60) },
  { PREFIX_TABLE (PREFIX_0F61) },
  { PREFIX_TABLE (PREFIX_0F62) },
  { "packsswb",		{ MX, EM } },
  { "pcmpgtb",		{ MX, EM } },
  { "pcmpgtw",		{ MX, EM } },
  { "pcmpgtd",		{ MX, EM } },
  { "packuswb",		{ MX, EM } },
  /* 68 */
  { "punpckhbw",	{ MX, EM } },
  { "punpckhwd",	{ MX, EM } },
  { "punpckhdq",	{ MX, EM } },
  { "packssdw",		{ MX, EM } },
  { PREFIX_TABLE (PREFIX_0F6C) },
  { PREFIX_TABLE (PREFIX_0F6D) },
  { "movK",		{ MX, Edq } },
  { PREFIX_TABLE (PREFIX_0F6F) },
  /* 70 */
  { PREFIX_TABLE (PREFIX_0F70) },
  { REG_TABLE (REG_0F71) },
  { REG_TABLE (REG_0F72) },
  { REG_TABLE (REG_0F73) },
  { "pcmpeqb",		{ MX, EM } },
  { "pcmpeqw",		{ MX, EM } },
  { "pcmpeqd",		{ MX, EM } },
  { "emms",		{ XX } },
  /* 78 */
  { PREFIX_TABLE (PREFIX_0F78) },
  { PREFIX_TABLE (PREFIX_0F79) },
  { THREE_BYTE_TABLE (THREE_BYTE_0F7A) },
  { Bad_Opcode },
  { PREFIX_TABLE (PREFIX_0F7C) },
  { PREFIX_TABLE (PREFIX_0F7D) },
  { PREFIX_TABLE (PREFIX_0F7E) },
  { PREFIX_TABLE (PREFIX_0F7F) },
  /* 80 */
  { "joH",		{ Jv, XX, cond_jump_flag } },
  { "jnoH",		{ Jv, XX, cond_jump_flag } },
  { "jbH",		{ Jv, XX, cond_jump_flag } },
  { "jaeH",		{ Jv, XX, cond_jump_flag } },
  { "jeH",		{ Jv, XX, cond_jump_flag } },
  { "jneH",		{ Jv, XX, cond_jump_flag } },
  { "jbeH",		{ Jv, XX, cond_jump_flag } },
  { "jaH",		{ Jv, XX, cond_jump_flag } },
  /* 88 */
  { "jsH",		{ Jv, XX, cond_jump_flag } },
  { "jnsH",		{ Jv, XX, cond_jump_flag } },
  { "jpH",		{ Jv, XX, cond_jump_flag } },
  { "jnpH",		{ Jv, XX, cond_jump_flag } },
  { "jlH",		{ Jv, XX, cond_jump_flag } },
  { "jgeH",		{ Jv, XX, cond_jump_flag } },
  { "jleH",		{ Jv, XX, cond_jump_flag } },
  { "jgH",		{ Jv, XX, cond_jump_flag } },
  /* 90 */
  { "seto",		{ Eb } },
  { "setno",		{ Eb } },
  { "setb",		{ Eb } },
  { "setae",		{ Eb } },
  { "sete",		{ Eb } },
  { "setne",		{ Eb } },
  { "setbe",		{ Eb } },
  { "seta",		{ Eb } },
  /* 98 */
  { "sets",		{ Eb } },
  { "setns",		{ Eb } },
  { "setp",		{ Eb } },
  { "setnp",		{ Eb } },
  { "setl",		{ Eb } },
  { "setge",		{ Eb } },
  { "setle",		{ Eb } },
  { "setg",		{ Eb } },
  /* a0 */
  { "pushT",		{ fs } },
  { "popT",		{ fs } },
  { "cpuid",		{ XX } },
  { "btS",		{ Ev, Gv } },
  { "shldS",		{ Ev, Gv, Ib } },
  { "shldS",		{ Ev, Gv, CL } },
  { REG_TABLE (REG_0FA6) },
  { REG_TABLE (REG_0FA7) },
  /* a8 */
  { "pushT",		{ gs } },
  { "popT",		{ gs } },
  { "rsm",		{ XX } },
  { "btsS",		{ Evh1, Gv } },
  { "shrdS",		{ Ev, Gv, Ib } },
  { "shrdS",		{ Ev, Gv, CL } },
  { REG_TABLE (REG_0FAE) },
  { "imulS",		{ Gv, Ev } },
  /* b0 */
  { "cmpxchgB",		{ Ebh1, Gb } },
  { "cmpxchgS",		{ Evh1, Gv } },
  { MOD_TABLE (MOD_0FB2) },
  { "btrS",		{ Evh1, Gv } },
  { MOD_TABLE (MOD_0FB4) },
  { MOD_TABLE (MOD_0FB5) },
  { "movz{bR|x}",	{ Gv, Eb } },
  { "movz{wR|x}",	{ Gv, Ew } }, /* yes, there really is movzww ! */
  /* b8 */
  { PREFIX_TABLE (PREFIX_0FB8) },
  { "ud1",		{ XX } },
  { REG_TABLE (REG_0FBA) },
  { "btcS",		{ Evh1, Gv } },
  { PREFIX_TABLE (PREFIX_0FBC) },
  { PREFIX_TABLE (PREFIX_0FBD) },
  { "movs{bR|x}",	{ Gv, Eb } },
  { "movs{wR|x}",	{ Gv, Ew } }, /* yes, there really is movsww ! */
  /* c0 */
  { "xaddB",		{ Ebh1, Gb } },
  { "xaddS",		{ Evh1, Gv } },
  { PREFIX_TABLE (PREFIX_0FC2) },
  { PREFIX_TABLE (PREFIX_0FC3) },
  { "pinsrw",		{ MX, Edqw, Ib } },
  { "pextrw",		{ Gdq, MS, Ib } },
  { "shufpX",		{ XM, EXx, Ib } },
  { REG_TABLE (REG_0FC7) },
  /* c8 */
  { "bswap",		{ RMeAX } },
  { "bswap",		{ RMeCX } },
  { "bswap",		{ RMeDX } },
  { "bswap",		{ RMeBX } },
  { "bswap",		{ RMeSP } },
  { "bswap",		{ RMeBP } },
  { "bswap",		{ RMeSI } },
  { "bswap",		{ RMeDI } },
  /* d0 */
  { PREFIX_TABLE (PREFIX_0FD0) },
  { "psrlw",		{ MX, EM } },
  { "psrld",		{ MX, EM } },
  { "psrlq",		{ MX, EM } },
  { "paddq",		{ MX, EM } },
  { "pmullw",		{ MX, EM } },
  { PREFIX_TABLE (PREFIX_0FD6) },
  { MOD_TABLE (MOD_0FD7) },
  /* d8 */
  { "psubusb",		{ MX, EM } },
  { "psubusw",		{ MX, EM } },
  { "pminub",		{ MX, EM } },
  { "pand",		{ MX, EM } },
  { "paddusb",		{ MX, EM } },
  { "paddusw",		{ MX, EM } },
  { "pmaxub",		{ MX, EM } },
  { "pandn",		{ MX, EM } },
  /* e0 */
  { "pavgb",		{ MX, EM } },
  { "psraw",		{ MX, EM } },
  { "psrad",		{ MX, EM } },
  { "pavgw",		{ MX, EM } },
  { "pmulhuw",		{ MX, EM } },
  { "pmulhw",		{ MX, EM } },
  { PREFIX_TABLE (PREFIX_0FE6) },
  { PREFIX_TABLE (PREFIX_0FE7) },
  /* e8 */
  { "psubsb",		{ MX, EM } },
  { "psubsw",		{ MX, EM } },
  { "pminsw",		{ MX, EM } },
  { "por",		{ MX, EM } },
  { "paddsb",		{ MX, EM } },
  { "paddsw",		{ MX, EM } },
  { "pmaxsw",		{ MX, EM } },
  { "pxor",		{ MX, EM } },
  /* f0 */
  { PREFIX_TABLE (PREFIX_0FF0) },
  { "psllw",		{ MX, EM } },
  { "pslld",		{ MX, EM } },
  { "psllq",		{ MX, EM } },
  { "pmuludq",		{ MX, EM } },
  { "pmaddwd",		{ MX, EM } },
  { "psadbw",		{ MX, EM } },
  { PREFIX_TABLE (PREFIX_0FF7) },
  /* f8 */
  { "psubb",		{ MX, EM } },
  { "psubw",		{ MX, EM } },
  { "psubd",		{ MX, EM } },
  { "psubq",		{ MX, EM } },
  { "paddb",		{ MX, EM } },
  { "paddw",		{ MX, EM } },
  { "paddd",		{ MX, EM } },
  { Bad_Opcode },
};

static const unsigned char onebyte_has_modrm[256] = {
  /*       0 1 2 3 4 5 6 7 8 9 a b c d e f        */
  /*       -------------------------------        */
  /* 00 */ 1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0, /* 00 */
  /* 10 */ 1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0, /* 10 */
  /* 20 */ 1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0, /* 20 */
  /* 30 */ 1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0, /* 30 */
  /* 40 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 40 */
  /* 50 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 50 */
  /* 60 */ 0,0,1,1,0,0,0,0,0,1,0,1,0,0,0,0, /* 60 */
  /* 70 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 70 */
  /* 80 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 80 */
  /* 90 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 90 */
  /* a0 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* a0 */
  /* b0 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* b0 */
  /* c0 */ 1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0, /* c0 */
  /* d0 */ 1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1, /* d0 */
  /* e0 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* e0 */
  /* f0 */ 0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1  /* f0 */
  /*       -------------------------------        */
  /*       0 1 2 3 4 5 6 7 8 9 a b c d e f        */
};

static const unsigned char twobyte_has_modrm[256] = {
  /*       0 1 2 3 4 5 6 7 8 9 a b c d e f        */
  /*       -------------------------------        */
  /* 00 */ 1,1,1,1,0,0,0,0,0,0,0,0,0,1,0,1, /* 0f */
  /* 10 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 1f */
  /* 20 */ 1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1, /* 2f */
  /* 30 */ 0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0, /* 3f */
  /* 40 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 4f */
  /* 50 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 5f */
  /* 60 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 6f */
  /* 70 */ 1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1, /* 7f */
  /* 80 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 8f */
  /* 90 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 9f */
  /* a0 */ 0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1, /* af */
  /* b0 */ 1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1, /* bf */
  /* c0 */ 1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0, /* cf */
  /* d0 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* df */
  /* e0 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* ef */
  /* f0 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0  /* ff */
  /*       -------------------------------        */
  /*       0 1 2 3 4 5 6 7 8 9 a b c d e f        */
};

static char obuf[100];
static char *obufp;
static char *mnemonicendp;
static char scratchbuf[100];
static unsigned char *start_codep;
static unsigned char *insn_codep;
static unsigned char *codep;
static int last_lock_prefix;
static int last_repz_prefix;
static int last_repnz_prefix;
static int last_data_prefix;
static int last_addr_prefix;
static int last_rex_prefix;
static int last_seg_prefix;
#define MAX_CODE_LENGTH 15
/* We can up to 14 prefixes since the maximum instruction length is
   15bytes.  */
static int all_prefixes[MAX_CODE_LENGTH - 1];
static disassemble_info *the_info;
static struct
  {
    int mod;
    int reg;
    int rm;
  }
modrm;
static unsigned char need_modrm;
static struct
  {
    int scale;
    int index;
    int base;
  }
sib;
static struct
  {
    int register_specifier;
    int length;
    int prefix;
    int w;
  }
vex;
static unsigned char need_vex;
static unsigned char need_vex_reg;
static unsigned char vex_w_done;

struct op
  {
    const char *name;
    unsigned int len;
  };

/* If we are accessing mod/rm/reg without need_modrm set, then the
   values are stale.  Hitting this abort likely indicates that you
   need to update onebyte_has_modrm or twobyte_has_modrm.  */
#define MODRM_CHECK  if (!need_modrm) abort ()

static const char **names64;
static const char **names32;
static const char **names16;
static const char **names8;
static const char **names8rex;
static const char **names_seg;
static const char *index64;
static const char *index32;
static const char **index16;

static const char *intel_names64[] = {
  "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
};
static const char *intel_names32[] = {
  "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
  "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d"
};
static const char *intel_names16[] = {
  "ax", "cx", "dx", "bx", "sp", "bp", "si", "di",
  "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w"
};
static const char *intel_names8[] = {
  "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh",
};
static const char *intel_names8rex[] = {
  "al", "cl", "dl", "bl", "spl", "bpl", "sil", "dil",
  "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b"
};
static const char *intel_names_seg[] = {
  "es", "cs", "ss", "ds", "fs", "gs", "?", "?",
};
static const char *intel_index64 = "riz";
static const char *intel_index32 = "eiz";
static const char *intel_index16[] = {
  "bx+si", "bx+di", "bp+si", "bp+di", "si", "di", "bp", "bx"
};

static const char *att_names64[] = {
  "%rax", "%rcx", "%rdx", "%rbx", "%rsp", "%rbp", "%rsi", "%rdi",
  "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"
};
static const char *att_names32[] = {
  "%eax", "%ecx", "%edx", "%ebx", "%esp", "%ebp", "%esi", "%edi",
  "%r8d", "%r9d", "%r10d", "%r11d", "%r12d", "%r13d", "%r14d", "%r15d"
};
static const char *att_names16[] = {
  "%ax", "%cx", "%dx", "%bx", "%sp", "%bp", "%si", "%di",
  "%r8w", "%r9w", "%r10w", "%r11w", "%r12w", "%r13w", "%r14w", "%r15w"
};
static const char *att_names8[] = {
  "%al", "%cl", "%dl", "%bl", "%ah", "%ch", "%dh", "%bh",
};
static const char *att_names8rex[] = {
  "%al", "%cl", "%dl", "%bl", "%spl", "%bpl", "%sil", "%dil",
  "%r8b", "%r9b", "%r10b", "%r11b", "%r12b", "%r13b", "%r14b", "%r15b"
};
static const char *att_names_seg[] = {
  "%es", "%cs", "%ss", "%ds", "%fs", "%gs", "%?", "%?",
};
static const char *att_index64 = "%riz";
static const char *att_index32 = "%eiz";
static const char *att_index16[] = {
  "%bx,%si", "%bx,%di", "%bp,%si", "%bp,%di", "%si", "%di", "%bp", "%bx"
};

static const char **names_mm;
static const char *intel_names_mm[] = {
  "mm0", "mm1", "mm2", "mm3",
  "mm4", "mm5", "mm6", "mm7"
};
static const char *att_names_mm[] = {
  "%mm0", "%mm1", "%mm2", "%mm3",
  "%mm4", "%mm5", "%mm6", "%mm7"
};

static const char **names_xmm;
static const char *intel_names_xmm[] = {
  "xmm0", "xmm1", "xmm2", "xmm3",
  "xmm4", "xmm5", "xmm6", "xmm7",
  "xmm8", "xmm9", "xmm10", "xmm11",
  "xmm12", "xmm13", "xmm14", "xmm15"
};
static const char *att_names_xmm[] = {
  "%xmm0", "%xmm1", "%xmm2", "%xmm3",
  "%xmm4", "%xmm5", "%xmm6", "%xmm7",
  "%xmm8", "%xmm9", "%xmm10", "%xmm11",
  "%xmm12", "%xmm13", "%xmm14", "%xmm15"
};

static const char **names_ymm;
static const char *intel_names_ymm[] = {
  "ymm0", "ymm1", "ymm2", "ymm3",
  "ymm4", "ymm5", "ymm6", "ymm7",
  "ymm8", "ymm9", "ymm10", "ymm11",
  "ymm12", "ymm13", "ymm14", "ymm15"
};
static const char *att_names_ymm[] = {
  "%ymm0", "%ymm1", "%ymm2", "%ymm3",
  "%ymm4", "%ymm5", "%ymm6", "%ymm7",
  "%ymm8", "%ymm9", "%ymm10", "%ymm11",
  "%ymm12", "%ymm13", "%ymm14", "%ymm15"
};

static const struct dis386 reg_table[][8] = {
  /* REG_80 */
  {
    { "addA",	{ Ebh1, Ib } },
    { "orA",	{ Ebh1, Ib } },
    { "adcA",	{ Ebh1, Ib } },
    { "sbbA",	{ Ebh1, Ib } },
    { "andA",	{ Ebh1, Ib } },
    { "subA",	{ Ebh1, Ib } },
    { "xorA",	{ Ebh1, Ib } },
    { "cmpA",	{ Eb, Ib } },
  },
  /* REG_81 */
  {
    { "addQ",	{ Evh1, Iv } },
    { "orQ",	{ Evh1, Iv } },
    { "adcQ",	{ Evh1, Iv } },
    { "sbbQ",	{ Evh1, Iv } },
    { "andQ",	{ Evh1, Iv } },
    { "subQ",	{ Evh1, Iv } },
    { "xorQ",	{ Evh1, Iv } },
    { "cmpQ",	{ Ev, Iv } },
  },
  /* REG_82 */
  {
    { "addQ",	{ Evh1, sIb } },
    { "orQ",	{ Evh1, sIb } },
    { "adcQ",	{ Evh1, sIb } },
    { "sbbQ",	{ Evh1, sIb } },
    { "andQ",	{ Evh1, sIb } },
    { "subQ",	{ Evh1, sIb } },
    { "xorQ",	{ Evh1, sIb } },
    { "cmpQ",	{ Ev, sIb } },
  },
  /* REG_8F */
  {
    { "popU",	{ stackEv } },
    { XOP_8F_TABLE (XOP_09) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { XOP_8F_TABLE (XOP_09) },
  },
  /* REG_C0 */
  {
    { "rolA",	{ Eb, Ib } },
    { "rorA",	{ Eb, Ib } },
    { "rclA",	{ Eb, Ib } },
    { "rcrA",	{ Eb, Ib } },
    { "shlA",	{ Eb, Ib } },
    { "shrA",	{ Eb, Ib } },
    { Bad_Opcode },
    { "sarA",	{ Eb, Ib } },
  },
  /* REG_C1 */
  {
    { "rolQ",	{ Ev, Ib } },
    { "rorQ",	{ Ev, Ib } },
    { "rclQ",	{ Ev, Ib } },
    { "rcrQ",	{ Ev, Ib } },
    { "shlQ",	{ Ev, Ib } },
    { "shrQ",	{ Ev, Ib } },
    { Bad_Opcode },
    { "sarQ",	{ Ev, Ib } },
  },
  /* REG_C6 */
  {
    { "movA",	{ Ebh3, Ib } },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_C6_REG_7) },
  },
  /* REG_C7 */
  {
    { "movQ",	{ Evh3, Iv } },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_C7_REG_7) },
  },
  /* REG_D0 */
  {
    { "rolA",	{ Eb, I1 } },
    { "rorA",	{ Eb, I1 } },
    { "rclA",	{ Eb, I1 } },
    { "rcrA",	{ Eb, I1 } },
    { "shlA",	{ Eb, I1 } },
    { "shrA",	{ Eb, I1 } },
    { Bad_Opcode },
    { "sarA",	{ Eb, I1 } },
  },
  /* REG_D1 */
  {
    { "rolQ",	{ Ev, I1 } },
    { "rorQ",	{ Ev, I1 } },
    { "rclQ",	{ Ev, I1 } },
    { "rcrQ",	{ Ev, I1 } },
    { "shlQ",	{ Ev, I1 } },
    { "shrQ",	{ Ev, I1 } },
    { Bad_Opcode },
    { "sarQ",	{ Ev, I1 } },
  },
  /* REG_D2 */
  {
    { "rolA",	{ Eb, CL } },
    { "rorA",	{ Eb, CL } },
    { "rclA",	{ Eb, CL } },
    { "rcrA",	{ Eb, CL } },
    { "shlA",	{ Eb, CL } },
    { "shrA",	{ Eb, CL } },
    { Bad_Opcode },
    { "sarA",	{ Eb, CL } },
  },
  /* REG_D3 */
  {
    { "rolQ",	{ Ev, CL } },
    { "rorQ",	{ Ev, CL } },
    { "rclQ",	{ Ev, CL } },
    { "rcrQ",	{ Ev, CL } },
    { "shlQ",	{ Ev, CL } },
    { "shrQ",	{ Ev, CL } },
    { Bad_Opcode },
    { "sarQ",	{ Ev, CL } },
  },
  /* REG_F6 */
  {
    { "testA",	{ Eb, Ib } },
    { Bad_Opcode },
    { "notA",	{ Ebh1 } },
    { "negA",	{ Ebh1 } },
    { "mulA",	{ Eb } },	/* Don't print the implicit %al register,  */
    { "imulA",	{ Eb } },	/* to distinguish these opcodes from other */
    { "divA",	{ Eb } },	/* mul/imul opcodes.  Do the same for div  */
    { "idivA",	{ Eb } },	/* and idiv for consistency.		   */
  },
  /* REG_F7 */
  {
    { "testQ",	{ Ev, Iv } },
    { Bad_Opcode },
    { "notQ",	{ Evh1 } },
    { "negQ",	{ Evh1 } },
    { "mulQ",	{ Ev } },	/* Don't print the implicit register.  */
    { "imulQ",	{ Ev } },
    { "divQ",	{ Ev } },
    { "idivQ",	{ Ev } },
  },
  /* REG_FE */
  {
    { "incA",	{ Ebh1 } },
    { "decA",	{ Ebh1 } },
  },
  /* REG_FF */
  {
    { "incQ",	{ Evh1 } },
    { "decQ",	{ Evh1 } },
    { "call{T|}", { indirEv } },
    { "Jcall{T|}", { indirEp } },
    { "jmp{T|}", { indirEv } },
    { "Jjmp{T|}", { indirEp } },
    { "pushU",	{ stackEv } },
    { Bad_Opcode },
  },
  /* REG_0F00 */
  {
    { "sldtD",	{ Sv } },
    { "strD",	{ Sv } },
    { "lldt",	{ Ew } },
    { "ltr",	{ Ew } },
    { "verr",	{ Ew } },
    { "verw",	{ Ew } },
    { Bad_Opcode },
    { Bad_Opcode },
  },
  /* REG_0F01 */
  {
    { MOD_TABLE (MOD_0F01_REG_0) },
    { MOD_TABLE (MOD_0F01_REG_1) },
    { MOD_TABLE (MOD_0F01_REG_2) },
    { MOD_TABLE (MOD_0F01_REG_3) },
    { "smswD",	{ Sv } },
    { Bad_Opcode },
    { "lmsw",	{ Ew } },
    { MOD_TABLE (MOD_0F01_REG_7) },
  },
  /* REG_0F0D */
  {
    { "prefetch",	{ Mb } },
    { "prefetchw",	{ Mb } },
  },
  /* REG_0F18 */
  {
    { MOD_TABLE (MOD_0F18_REG_0) },
    { MOD_TABLE (MOD_0F18_REG_1) },
    { MOD_TABLE (MOD_0F18_REG_2) },
    { MOD_TABLE (MOD_0F18_REG_3) },
  },
  /* REG_0F71 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_0F71_REG_2) },
    { Bad_Opcode },
    { MOD_TABLE (MOD_0F71_REG_4) },
    { Bad_Opcode },
    { MOD_TABLE (MOD_0F71_REG_6) },
  },
  /* REG_0F72 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_0F72_REG_2) },
    { Bad_Opcode },
    { MOD_TABLE (MOD_0F72_REG_4) },
    { Bad_Opcode },
    { MOD_TABLE (MOD_0F72_REG_6) },
  },
  /* REG_0F73 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_0F73_REG_2) },
    { MOD_TABLE (MOD_0F73_REG_3) },
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_0F73_REG_6) },
    { MOD_TABLE (MOD_0F73_REG_7) },
  },
  /* REG_0FA6 */
  {
    { "montmul",	{ { OP_0f07, 0 } } },
    { "xsha1",		{ { OP_0f07, 0 } } },
    { "xsha256",	{ { OP_0f07, 0 } } },
  },
  /* REG_0FA7 */
  {
    { "xstore-rng",	{ { OP_0f07, 0 } } },
    { "xcrypt-ecb",	{ { OP_0f07, 0 } } },
    { "xcrypt-cbc",	{ { OP_0f07, 0 } } },
    { "xcrypt-ctr",	{ { OP_0f07, 0 } } },
    { "xcrypt-cfb",	{ { OP_0f07, 0 } } },
    { "xcrypt-ofb",	{ { OP_0f07, 0 } } },
  },
  /* REG_0FAE */
  {
    { MOD_TABLE (MOD_0FAE_REG_0) },
    { MOD_TABLE (MOD_0FAE_REG_1) },
    { MOD_TABLE (MOD_0FAE_REG_2) },
    { MOD_TABLE (MOD_0FAE_REG_3) },
    { MOD_TABLE (MOD_0FAE_REG_4) },
    { MOD_TABLE (MOD_0FAE_REG_5) },
    { MOD_TABLE (MOD_0FAE_REG_6) },
    { MOD_TABLE (MOD_0FAE_REG_7) },
  },
  /* REG_0FBA */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "btQ",	{ Ev, Ib } },
    { "btsQ",	{ Evh1, Ib } },
    { "btrQ",	{ Evh1, Ib } },
    { "btcQ",	{ Evh1, Ib } },
  },
  /* REG_0FC7 */
  {
    { Bad_Opcode },
    { "cmpxchg8b", { { CMPXCHG8B_Fixup, q_mode } } },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_0FC7_REG_6) },
    { MOD_TABLE (MOD_0FC7_REG_7) },
  },
  /* REG_VEX_0F71 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F71_REG_2) },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F71_REG_4) },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F71_REG_6) },
  },
  /* REG_VEX_0F72 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F72_REG_2) },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F72_REG_4) },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F72_REG_6) },
  },
  /* REG_VEX_0F73 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F73_REG_2) },
    { MOD_TABLE (MOD_VEX_0F73_REG_3) },
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F73_REG_6) },
    { MOD_TABLE (MOD_VEX_0F73_REG_7) },
  },
  /* REG_VEX_0FAE */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0FAE_REG_2) },
    { MOD_TABLE (MOD_VEX_0FAE_REG_3) },
  },
  /* REG_VEX_0F38F3 */
  {
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F38F3_REG_1) },
    { PREFIX_TABLE (PREFIX_VEX_0F38F3_REG_2) },
    { PREFIX_TABLE (PREFIX_VEX_0F38F3_REG_3) },
  },
  /* REG_XOP_LWPCB */
  {
    { "llwpcb", { { OP_LWPCB_E, 0 } } },
    { "slwpcb",	{ { OP_LWPCB_E, 0 } } },
  },
  /* REG_XOP_LWP */
  {
    { "lwpins", { { OP_LWP_E, 0 }, Ed, Iq } },
    { "lwpval",	{ { OP_LWP_E, 0 }, Ed, Iq } },
  },
  /* REG_XOP_TBM_01 */
  {
    { Bad_Opcode },
    { "blcfill",	{ { OP_LWP_E, 0 }, Ev } },
    { "blsfill",	{ { OP_LWP_E, 0 }, Ev } },
    { "blcs",	{ { OP_LWP_E, 0 }, Ev } },
    { "tzmsk",	{ { OP_LWP_E, 0 }, Ev } },
    { "blcic",	{ { OP_LWP_E, 0 }, Ev } },
    { "blsic",	{ { OP_LWP_E, 0 }, Ev } },
    { "t1mskc",	{ { OP_LWP_E, 0 }, Ev } },
  },
  /* REG_XOP_TBM_02 */
  {
    { Bad_Opcode },
    { "blcmsk",	{ { OP_LWP_E, 0 }, Ev } },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "blci",	{ { OP_LWP_E, 0 }, Ev } },
  },
};

static const struct dis386 prefix_table[][4] = {
  /* PREFIX_90 */
  {
    { "xchgS", { { NOP_Fixup1, eAX_reg }, { NOP_Fixup2, eAX_reg } } },
    { "pause", { XX } },
    { "xchgS", { { NOP_Fixup1, eAX_reg }, { NOP_Fixup2, eAX_reg } } },
  },

  /* PREFIX_0F10 */
  {
    { "movups",	{ XM, EXx } },
    { "movss",	{ XM, EXd } },
    { "movupd",	{ XM, EXx } },
    { "movsd",	{ XM, EXq } },
  },

  /* PREFIX_0F11 */
  {
    { "movups",	{ EXxS, XM } },
    { "movss",	{ EXdS, XM } },
    { "movupd",	{ EXxS, XM } },
    { "movsd",	{ EXqS, XM } },
  },

  /* PREFIX_0F12 */
  {
    { MOD_TABLE (MOD_0F12_PREFIX_0) },
    { "movsldup", { XM, EXx } },
    { "movlpd",	{ XM, EXq } },
    { "movddup", { XM, EXq } },
  },

  /* PREFIX_0F16 */
  {
    { MOD_TABLE (MOD_0F16_PREFIX_0) },
    { "movshdup", { XM, EXx } },
    { "movhpd",	{ XM, EXq } },
  },

  /* PREFIX_0F2A */
  {
    { "cvtpi2ps", { XM, EMCq } },
    { "cvtsi2ss%LQ", { XM, Ev } },
    { "cvtpi2pd", { XM, EMCq } },
    { "cvtsi2sd%LQ", { XM, Ev } },
  },

  /* PREFIX_0F2B */
  {
    { MOD_TABLE (MOD_0F2B_PREFIX_0) },
    { MOD_TABLE (MOD_0F2B_PREFIX_1) },
    { MOD_TABLE (MOD_0F2B_PREFIX_2) },
    { MOD_TABLE (MOD_0F2B_PREFIX_3) },
  },

  /* PREFIX_0F2C */
  {
    { "cvttps2pi", { MXC, EXq } },
    { "cvttss2siY", { Gv, EXd } },
    { "cvttpd2pi", { MXC, EXx } },
    { "cvttsd2siY", { Gv, EXq } },
  },

  /* PREFIX_0F2D */
  {
    { "cvtps2pi", { MXC, EXq } },
    { "cvtss2siY", { Gv, EXd } },
    { "cvtpd2pi", { MXC, EXx } },
    { "cvtsd2siY", { Gv, EXq } },
  },

  /* PREFIX_0F2E */
  {
    { "ucomiss",{ XM, EXd } }, 
    { Bad_Opcode },
    { "ucomisd",{ XM, EXq } }, 
  },

  /* PREFIX_0F2F */
  {
    { "comiss",	{ XM, EXd } },
    { Bad_Opcode },
    { "comisd",	{ XM, EXq } },
  },

  /* PREFIX_0F51 */
  {
    { "sqrtps", { XM, EXx } },
    { "sqrtss", { XM, EXd } },
    { "sqrtpd", { XM, EXx } },
    { "sqrtsd",	{ XM, EXq } },
  },

  /* PREFIX_0F52 */
  {
    { "rsqrtps",{ XM, EXx } },
    { "rsqrtss",{ XM, EXd } },
  },

  /* PREFIX_0F53 */
  {
    { "rcpps",	{ XM, EXx } },
    { "rcpss",	{ XM, EXd } },
  },

  /* PREFIX_0F58 */
  {
    { "addps", { XM, EXx } },
    { "addss", { XM, EXd } },
    { "addpd", { XM, EXx } },
    { "addsd", { XM, EXq } },
  },

  /* PREFIX_0F59 */
  {
    { "mulps",	{ XM, EXx } },
    { "mulss",	{ XM, EXd } },
    { "mulpd",	{ XM, EXx } },
    { "mulsd",	{ XM, EXq } },
  },

  /* PREFIX_0F5A */
  {
    { "cvtps2pd", { XM, EXq } },
    { "cvtss2sd", { XM, EXd } },
    { "cvtpd2ps", { XM, EXx } },
    { "cvtsd2ss", { XM, EXq } },
  },

  /* PREFIX_0F5B */
  {
    { "cvtdq2ps", { XM, EXx } },
    { "cvttps2dq", { XM, EXx } },
    { "cvtps2dq", { XM, EXx } },
  },

  /* PREFIX_0F5C */
  {
    { "subps",	{ XM, EXx } },
    { "subss",	{ XM, EXd } },
    { "subpd",	{ XM, EXx } },
    { "subsd",	{ XM, EXq } },
  },

  /* PREFIX_0F5D */
  {
    { "minps",	{ XM, EXx } },
    { "minss",	{ XM, EXd } },
    { "minpd",	{ XM, EXx } },
    { "minsd",	{ XM, EXq } },
  },

  /* PREFIX_0F5E */
  {
    { "divps",	{ XM, EXx } },
    { "divss",	{ XM, EXd } },
    { "divpd",	{ XM, EXx } },
    { "divsd",	{ XM, EXq } },
  },

  /* PREFIX_0F5F */
  {
    { "maxps",	{ XM, EXx } },
    { "maxss",	{ XM, EXd } },
    { "maxpd",	{ XM, EXx } },
    { "maxsd",	{ XM, EXq } },
  },

  /* PREFIX_0F60 */
  {
    { "punpcklbw",{ MX, EMd } },
    { Bad_Opcode },
    { "punpcklbw",{ MX, EMx } },
  },

  /* PREFIX_0F61 */
  {
    { "punpcklwd",{ MX, EMd } },
    { Bad_Opcode },
    { "punpcklwd",{ MX, EMx } },
  },

  /* PREFIX_0F62 */
  {
    { "punpckldq",{ MX, EMd } },
    { Bad_Opcode },
    { "punpckldq",{ MX, EMx } },
  },

  /* PREFIX_0F6C */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "punpcklqdq", { XM, EXx } },
  },

  /* PREFIX_0F6D */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "punpckhqdq", { XM, EXx } },
  },

  /* PREFIX_0F6F */
  {
    { "movq",	{ MX, EM } },
    { "movdqu",	{ XM, EXx } },
    { "movdqa",	{ XM, EXx } },
  },

  /* PREFIX_0F70 */
  {
    { "pshufw",	{ MX, EM, Ib } },
    { "pshufhw",{ XM, EXx, Ib } },
    { "pshufd",	{ XM, EXx, Ib } },
    { "pshuflw",{ XM, EXx, Ib } },
  },

  /* PREFIX_0F73_REG_3 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "psrldq",	{ XS, Ib } },
  },

  /* PREFIX_0F73_REG_7 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pslldq",	{ XS, Ib } },
  },

  /* PREFIX_0F78 */
  {
    {"vmread",	{ Em, Gm } },
    { Bad_Opcode },
    {"extrq",	{ XS, Ib, Ib } },
    {"insertq",	{ XM, XS, Ib, Ib } },
  },

  /* PREFIX_0F79 */
  {
    {"vmwrite",	{ Gm, Em } },
    { Bad_Opcode },
    {"extrq",	{ XM, XS } },
    {"insertq",	{ XM, XS } },
  },

  /* PREFIX_0F7C */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "haddpd",	{ XM, EXx } },
    { "haddps",	{ XM, EXx } },
  },

  /* PREFIX_0F7D */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "hsubpd",	{ XM, EXx } },
    { "hsubps",	{ XM, EXx } },
  },

  /* PREFIX_0F7E */
  {
    { "movK",	{ Edq, MX } },
    { "movq",	{ XM, EXq } },
    { "movK",	{ Edq, XM } },
  },

  /* PREFIX_0F7F */
  {
    { "movq",	{ EMS, MX } },
    { "movdqu",	{ EXxS, XM } },
    { "movdqa",	{ EXxS, XM } },
  },

  /* PREFIX_0FAE_REG_0 */
  {
    { Bad_Opcode },
    { "rdfsbase", { Ev } },
  },

  /* PREFIX_0FAE_REG_1 */
  {
    { Bad_Opcode },
    { "rdgsbase", { Ev } },
  },

  /* PREFIX_0FAE_REG_2 */
  {
    { Bad_Opcode },
    { "wrfsbase", { Ev } },
  },

  /* PREFIX_0FAE_REG_3 */
  {
    { Bad_Opcode },
    { "wrgsbase", { Ev } },
  },

  /* PREFIX_0FB8 */
  {
    { Bad_Opcode },
    { "popcntS", { Gv, Ev } },
  },

  /* PREFIX_0FBC */
  {
    { "bsfS",	{ Gv, Ev } },
    { "tzcntS",	{ Gv, Ev } },
    { "bsfS",	{ Gv, Ev } },
  },

  /* PREFIX_0FBD */
  {
    { "bsrS",	{ Gv, Ev } },
    { "lzcntS",	{ Gv, Ev } },
    { "bsrS",	{ Gv, Ev } },
  },

  /* PREFIX_0FC2 */
  {
    { "cmpps",	{ XM, EXx, CMP } },
    { "cmpss",	{ XM, EXd, CMP } },
    { "cmppd",	{ XM, EXx, CMP } },
    { "cmpsd",	{ XM, EXq, CMP } },
  },

  /* PREFIX_0FC3 */
  {
    { "movntiS", { Ma, Gv } },
  },

  /* PREFIX_0FC7_REG_6 */
  {
    { "vmptrld",{ Mq } },
    { "vmxon",	{ Mq } },
    { "vmclear",{ Mq } },
  },

  /* PREFIX_0FD0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "addsubpd", { XM, EXx } },
    { "addsubps", { XM, EXx } },
  },

  /* PREFIX_0FD6 */
  {
    { Bad_Opcode },
    { "movq2dq",{ XM, MS } },
    { "movq",	{ EXqS, XM } },
    { "movdq2q",{ MX, XS } },
  },

  /* PREFIX_0FE6 */
  {
    { Bad_Opcode },
    { "cvtdq2pd", { XM, EXq } },
    { "cvttpd2dq", { XM, EXx } },
    { "cvtpd2dq", { XM, EXx } },
  },

  /* PREFIX_0FE7 */
  {
    { "movntq",	{ Mq, MX } },
    { Bad_Opcode },
    { MOD_TABLE (MOD_0FE7_PREFIX_2) },
  },

  /* PREFIX_0FF0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_0FF0_PREFIX_3) },
  },

  /* PREFIX_0FF7 */
  {
    { "maskmovq", { MX, MS } },
    { Bad_Opcode },
    { "maskmovdqu", { XM, XS } },
  },

  /* PREFIX_0F3810 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pblendvb", { XM, EXx, XMM0 } },
  },

  /* PREFIX_0F3814 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "blendvps", { XM, EXx, XMM0 } },
  },

  /* PREFIX_0F3815 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "blendvpd", { XM, EXx, XMM0 } },
  },

  /* PREFIX_0F3817 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "ptest",  { XM, EXx } },
  },

  /* PREFIX_0F3820 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pmovsxbw", { XM, EXq } },
  },

  /* PREFIX_0F3821 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pmovsxbd", { XM, EXd } },
  },

  /* PREFIX_0F3822 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pmovsxbq", { XM, EXw } },
  },

  /* PREFIX_0F3823 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pmovsxwd", { XM, EXq } },
  },

  /* PREFIX_0F3824 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pmovsxwq", { XM, EXd } },
  },

  /* PREFIX_0F3825 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pmovsxdq", { XM, EXq } },
  },

  /* PREFIX_0F3828 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pmuldq", { XM, EXx } },
  },

  /* PREFIX_0F3829 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pcmpeqq", { XM, EXx } },
  },

  /* PREFIX_0F382A */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_0F382A_PREFIX_2) },
  },

  /* PREFIX_0F382B */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "packusdw", { XM, EXx } },
  },

  /* PREFIX_0F3830 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pmovzxbw", { XM, EXq } },
  },

  /* PREFIX_0F3831 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pmovzxbd", { XM, EXd } },
  },

  /* PREFIX_0F3832 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pmovzxbq", { XM, EXw } },
  },

  /* PREFIX_0F3833 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pmovzxwd", { XM, EXq } },
  },

  /* PREFIX_0F3834 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pmovzxwq", { XM, EXd } },
  },

  /* PREFIX_0F3835 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pmovzxdq", { XM, EXq } },
  },

  /* PREFIX_0F3837 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pcmpgtq", { XM, EXx } },
  },

  /* PREFIX_0F3838 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pminsb",	{ XM, EXx } },
  },

  /* PREFIX_0F3839 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pminsd",	{ XM, EXx } },
  },

  /* PREFIX_0F383A */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pminuw",	{ XM, EXx } },
  },

  /* PREFIX_0F383B */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pminud",	{ XM, EXx } },
  },

  /* PREFIX_0F383C */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pmaxsb",	{ XM, EXx } },
  },

  /* PREFIX_0F383D */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pmaxsd",	{ XM, EXx } },
  },

  /* PREFIX_0F383E */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pmaxuw", { XM, EXx } },
  },

  /* PREFIX_0F383F */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pmaxud", { XM, EXx } },
  },

  /* PREFIX_0F3840 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pmulld", { XM, EXx } },
  },

  /* PREFIX_0F3841 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "phminposuw", { XM, EXx } },
  },

  /* PREFIX_0F3880 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "invept",	{ Gm, Mo } },
  },

  /* PREFIX_0F3881 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "invvpid", { Gm, Mo } },
  },

  /* PREFIX_0F3882 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "invpcid", { Gm, M } },
  },

  /* PREFIX_0F38DB */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "aesimc", { XM, EXx } },
  },

  /* PREFIX_0F38DC */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "aesenc", { XM, EXx } },
  },

  /* PREFIX_0F38DD */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "aesenclast", { XM, EXx } },
  },

  /* PREFIX_0F38DE */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "aesdec", { XM, EXx } },
  },

  /* PREFIX_0F38DF */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "aesdeclast", { XM, EXx } },
  },

  /* PREFIX_0F38F0 */
  {
    { "movbeS",	{ Gv, { MOVBE_Fixup, v_mode } } },
    { Bad_Opcode },
    { "movbeS",	{ Gv, { MOVBE_Fixup, v_mode } } },
    { "crc32",	{ Gdq, { CRC32_Fixup, b_mode } } },	
  },

  /* PREFIX_0F38F1 */
  {
    { "movbeS",	{ { MOVBE_Fixup, v_mode }, Gv } },
    { Bad_Opcode },
    { "movbeS",	{ { MOVBE_Fixup, v_mode }, Gv } },
    { "crc32",	{ Gdq, { CRC32_Fixup, v_mode } } },	
  },

  /* PREFIX_0F38F6 */
  {
    { Bad_Opcode },
    { "adoxS",	{ Gdq, Edq} },
    { "adcxS",	{ Gdq, Edq} },
    { Bad_Opcode },
  },

  /* PREFIX_0F3A08 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "roundps", { XM, EXx, Ib } },
  },

  /* PREFIX_0F3A09 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "roundpd", { XM, EXx, Ib } },
  },

  /* PREFIX_0F3A0A */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "roundss", { XM, EXd, Ib } },
  },

  /* PREFIX_0F3A0B */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "roundsd", { XM, EXq, Ib } },
  },

  /* PREFIX_0F3A0C */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "blendps", { XM, EXx, Ib } },
  },

  /* PREFIX_0F3A0D */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "blendpd", { XM, EXx, Ib } },
  },

  /* PREFIX_0F3A0E */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pblendw", { XM, EXx, Ib } },
  },

  /* PREFIX_0F3A14 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pextrb",	{ Edqb, XM, Ib } },
  },

  /* PREFIX_0F3A15 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pextrw",	{ Edqw, XM, Ib } },
  },

  /* PREFIX_0F3A16 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pextrK",	{ Edq, XM, Ib } },
  },

  /* PREFIX_0F3A17 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "extractps", { Edqd, XM, Ib } },
  },

  /* PREFIX_0F3A20 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pinsrb",	{ XM, Edqb, Ib } },
  },

  /* PREFIX_0F3A21 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "insertps", { XM, EXd, Ib } },
  },

  /* PREFIX_0F3A22 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pinsrK",	{ XM, Edq, Ib } },
  },

  /* PREFIX_0F3A40 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "dpps",	{ XM, EXx, Ib } },
  },

  /* PREFIX_0F3A41 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "dppd",	{ XM, EXx, Ib } },
  },

  /* PREFIX_0F3A42 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "mpsadbw", { XM, EXx, Ib } },
  },

  /* PREFIX_0F3A44 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pclmulqdq", { XM, EXx, PCLMUL } },
  },

  /* PREFIX_0F3A60 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pcmpestrm", { XM, EXx, Ib } },
  },

  /* PREFIX_0F3A61 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pcmpestri", { XM, EXx, Ib } },
  },

  /* PREFIX_0F3A62 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pcmpistrm", { XM, EXx, Ib } },
  },

  /* PREFIX_0F3A63 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "pcmpistri", { XM, EXx, Ib } },
  },

  /* PREFIX_0F3ADF */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "aeskeygenassist", { XM, EXx, Ib } },
  },

  /* PREFIX_VEX_0F10 */
  {
    { VEX_W_TABLE (VEX_W_0F10_P_0) },
    { VEX_LEN_TABLE (VEX_LEN_0F10_P_1) },
    { VEX_W_TABLE (VEX_W_0F10_P_2) },
    { VEX_LEN_TABLE (VEX_LEN_0F10_P_3) },
  },

  /* PREFIX_VEX_0F11 */
  {
    { VEX_W_TABLE (VEX_W_0F11_P_0) },
    { VEX_LEN_TABLE (VEX_LEN_0F11_P_1) },
    { VEX_W_TABLE (VEX_W_0F11_P_2) },
    { VEX_LEN_TABLE (VEX_LEN_0F11_P_3) },
  },

  /* PREFIX_VEX_0F12 */
  {
    { MOD_TABLE (MOD_VEX_0F12_PREFIX_0) },
    { VEX_W_TABLE (VEX_W_0F12_P_1) },
    { VEX_LEN_TABLE (VEX_LEN_0F12_P_2) },
    { VEX_W_TABLE (VEX_W_0F12_P_3) },
  },

  /* PREFIX_VEX_0F16 */
  {
    { MOD_TABLE (MOD_VEX_0F16_PREFIX_0) },
    { VEX_W_TABLE (VEX_W_0F16_P_1) },
    { VEX_LEN_TABLE (VEX_LEN_0F16_P_2) },
  },

  /* PREFIX_VEX_0F2A */
  {
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F2A_P_1) },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F2A_P_3) },
  },

  /* PREFIX_VEX_0F2C */
  {
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F2C_P_1) },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F2C_P_3) },
  },

  /* PREFIX_VEX_0F2D */
  {
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F2D_P_1) },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F2D_P_3) },
  },

  /* PREFIX_VEX_0F2E */
  {
    { VEX_LEN_TABLE (VEX_LEN_0F2E_P_0) },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F2E_P_2) },
  },

  /* PREFIX_VEX_0F2F */
  {
    { VEX_LEN_TABLE (VEX_LEN_0F2F_P_0) },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F2F_P_2) },
  },

  /* PREFIX_VEX_0F51 */
  {
    { VEX_W_TABLE (VEX_W_0F51_P_0) },
    { VEX_LEN_TABLE (VEX_LEN_0F51_P_1) },
    { VEX_W_TABLE (VEX_W_0F51_P_2) },
    { VEX_LEN_TABLE (VEX_LEN_0F51_P_3) },
  },

  /* PREFIX_VEX_0F52 */
  {
    { VEX_W_TABLE (VEX_W_0F52_P_0) },
    { VEX_LEN_TABLE (VEX_LEN_0F52_P_1) },
  },

  /* PREFIX_VEX_0F53 */
  {
    { VEX_W_TABLE (VEX_W_0F53_P_0) },
    { VEX_LEN_TABLE (VEX_LEN_0F53_P_1) },
  },

  /* PREFIX_VEX_0F58 */
  {
    { VEX_W_TABLE (VEX_W_0F58_P_0) },
    { VEX_LEN_TABLE (VEX_LEN_0F58_P_1) },
    { VEX_W_TABLE (VEX_W_0F58_P_2) },
    { VEX_LEN_TABLE (VEX_LEN_0F58_P_3) },
  },

  /* PREFIX_VEX_0F59 */
  {
    { VEX_W_TABLE (VEX_W_0F59_P_0) },
    { VEX_LEN_TABLE (VEX_LEN_0F59_P_1) },
    { VEX_W_TABLE (VEX_W_0F59_P_2) },
    { VEX_LEN_TABLE (VEX_LEN_0F59_P_3) },
  },

  /* PREFIX_VEX_0F5A */
  {
    { VEX_W_TABLE (VEX_W_0F5A_P_0) },
    { VEX_LEN_TABLE (VEX_LEN_0F5A_P_1) },
    { "vcvtpd2ps%XY", { XMM, EXx } },
    { VEX_LEN_TABLE (VEX_LEN_0F5A_P_3) },
  },

  /* PREFIX_VEX_0F5B */
  {
    { VEX_W_TABLE (VEX_W_0F5B_P_0) },
    { VEX_W_TABLE (VEX_W_0F5B_P_1) },
    { VEX_W_TABLE (VEX_W_0F5B_P_2) },
  },

  /* PREFIX_VEX_0F5C */
  {
    { VEX_W_TABLE (VEX_W_0F5C_P_0) },
    { VEX_LEN_TABLE (VEX_LEN_0F5C_P_1) },
    { VEX_W_TABLE (VEX_W_0F5C_P_2) },
    { VEX_LEN_TABLE (VEX_LEN_0F5C_P_3) },
  },

  /* PREFIX_VEX_0F5D */
  {
    { VEX_W_TABLE (VEX_W_0F5D_P_0) },
    { VEX_LEN_TABLE (VEX_LEN_0F5D_P_1) },
    { VEX_W_TABLE (VEX_W_0F5D_P_2) },
    { VEX_LEN_TABLE (VEX_LEN_0F5D_P_3) },
  },

  /* PREFIX_VEX_0F5E */
  {
    { VEX_W_TABLE (VEX_W_0F5E_P_0) },
    { VEX_LEN_TABLE (VEX_LEN_0F5E_P_1) },
    { VEX_W_TABLE (VEX_W_0F5E_P_2) },
    { VEX_LEN_TABLE (VEX_LEN_0F5E_P_3) },
  },

  /* PREFIX_VEX_0F5F */
  {
    { VEX_W_TABLE (VEX_W_0F5F_P_0) },
    { VEX_LEN_TABLE (VEX_LEN_0F5F_P_1) },
    { VEX_W_TABLE (VEX_W_0F5F_P_2) },
    { VEX_LEN_TABLE (VEX_LEN_0F5F_P_3) },
  },

  /* PREFIX_VEX_0F60 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F60_P_2) },
  },

  /* PREFIX_VEX_0F61 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F61_P_2) },
  },

  /* PREFIX_VEX_0F62 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F62_P_2) },
  },

  /* PREFIX_VEX_0F63 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F63_P_2) },
  },

  /* PREFIX_VEX_0F64 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F64_P_2) },
  },

  /* PREFIX_VEX_0F65 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F65_P_2) },
  },

  /* PREFIX_VEX_0F66 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F66_P_2) },
  },

  /* PREFIX_VEX_0F67 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F67_P_2) },
  },

  /* PREFIX_VEX_0F68 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F68_P_2) },
  },

  /* PREFIX_VEX_0F69 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F69_P_2) },
  },

  /* PREFIX_VEX_0F6A */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F6A_P_2) },
  },

  /* PREFIX_VEX_0F6B */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F6B_P_2) },
  },

  /* PREFIX_VEX_0F6C */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F6C_P_2) },
  },

  /* PREFIX_VEX_0F6D */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F6D_P_2) },
  },

  /* PREFIX_VEX_0F6E */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F6E_P_2) },
  },

  /* PREFIX_VEX_0F6F */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F6F_P_1) },
    { VEX_W_TABLE (VEX_W_0F6F_P_2) },
  },

  /* PREFIX_VEX_0F70 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F70_P_1) },
    { VEX_W_TABLE (VEX_W_0F70_P_2) },
    { VEX_W_TABLE (VEX_W_0F70_P_3) },
  },

  /* PREFIX_VEX_0F71_REG_2 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F71_R_2_P_2) },
  },

  /* PREFIX_VEX_0F71_REG_4 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F71_R_4_P_2) },
  },

  /* PREFIX_VEX_0F71_REG_6 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F71_R_6_P_2) },
  },

  /* PREFIX_VEX_0F72_REG_2 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F72_R_2_P_2) },
  },

  /* PREFIX_VEX_0F72_REG_4 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F72_R_4_P_2) },
  },

  /* PREFIX_VEX_0F72_REG_6 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F72_R_6_P_2) },
  },

  /* PREFIX_VEX_0F73_REG_2 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F73_R_2_P_2) },
  },

  /* PREFIX_VEX_0F73_REG_3 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F73_R_3_P_2) },
  },

  /* PREFIX_VEX_0F73_REG_6 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F73_R_6_P_2) },
  },

  /* PREFIX_VEX_0F73_REG_7 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F73_R_7_P_2) },
  },

  /* PREFIX_VEX_0F74 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F74_P_2) },
  },

  /* PREFIX_VEX_0F75 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F75_P_2) },
  },

  /* PREFIX_VEX_0F76 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F76_P_2) },
  },

  /* PREFIX_VEX_0F77 */
  {
    { VEX_W_TABLE (VEX_W_0F77_P_0) },
  },

  /* PREFIX_VEX_0F7C */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F7C_P_2) },
    { VEX_W_TABLE (VEX_W_0F7C_P_3) },
  },

  /* PREFIX_VEX_0F7D */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F7D_P_2) },
    { VEX_W_TABLE (VEX_W_0F7D_P_3) },
  },

  /* PREFIX_VEX_0F7E */
  {
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F7E_P_1) },
    { VEX_LEN_TABLE (VEX_LEN_0F7E_P_2) },
  },

  /* PREFIX_VEX_0F7F */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F7F_P_1) },
    { VEX_W_TABLE (VEX_W_0F7F_P_2) },
  },

  /* PREFIX_VEX_0FC2 */
  {
    { VEX_W_TABLE (VEX_W_0FC2_P_0) },
    { VEX_LEN_TABLE (VEX_LEN_0FC2_P_1) },
    { VEX_W_TABLE (VEX_W_0FC2_P_2) },
    { VEX_LEN_TABLE (VEX_LEN_0FC2_P_3) },
  },

  /* PREFIX_VEX_0FC4 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FC4_P_2) },
  },

  /* PREFIX_VEX_0FC5 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FC5_P_2) },
  },

  /* PREFIX_VEX_0FD0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FD0_P_2) },
    { VEX_W_TABLE (VEX_W_0FD0_P_3) },
  },

  /* PREFIX_VEX_0FD1 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FD1_P_2) },
  },

  /* PREFIX_VEX_0FD2 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FD2_P_2) },
  },

  /* PREFIX_VEX_0FD3 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FD3_P_2) },
  },

  /* PREFIX_VEX_0FD4 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FD4_P_2) },
  },

  /* PREFIX_VEX_0FD5 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FD5_P_2) },
  },

  /* PREFIX_VEX_0FD6 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FD6_P_2) },
  },

  /* PREFIX_VEX_0FD7 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0FD7_PREFIX_2) },
  },

  /* PREFIX_VEX_0FD8 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FD8_P_2) },
  },

  /* PREFIX_VEX_0FD9 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FD9_P_2) },
  },

  /* PREFIX_VEX_0FDA */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FDA_P_2) },
  },

  /* PREFIX_VEX_0FDB */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FDB_P_2) },
  },

  /* PREFIX_VEX_0FDC */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FDC_P_2) },
  },

  /* PREFIX_VEX_0FDD */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FDD_P_2) },
  },

  /* PREFIX_VEX_0FDE */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FDE_P_2) },
  },

  /* PREFIX_VEX_0FDF */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FDF_P_2) },
  },

  /* PREFIX_VEX_0FE0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FE0_P_2) },
  },

  /* PREFIX_VEX_0FE1 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FE1_P_2) },
  },

  /* PREFIX_VEX_0FE2 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FE2_P_2) },
  },

  /* PREFIX_VEX_0FE3 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FE3_P_2) },
  },

  /* PREFIX_VEX_0FE4 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FE4_P_2) },
  },

  /* PREFIX_VEX_0FE5 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FE5_P_2) },
  },

  /* PREFIX_VEX_0FE6 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FE6_P_1) },
    { VEX_W_TABLE (VEX_W_0FE6_P_2) },
    { VEX_W_TABLE (VEX_W_0FE6_P_3) },
  },

  /* PREFIX_VEX_0FE7 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0FE7_PREFIX_2) },
  },

  /* PREFIX_VEX_0FE8 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FE8_P_2) },
  },

  /* PREFIX_VEX_0FE9 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FE9_P_2) },
  },

  /* PREFIX_VEX_0FEA */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FEA_P_2) },
  },

  /* PREFIX_VEX_0FEB */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FEB_P_2) },
  },

  /* PREFIX_VEX_0FEC */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FEC_P_2) },
  },

  /* PREFIX_VEX_0FED */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FED_P_2) },
  },

  /* PREFIX_VEX_0FEE */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FEE_P_2) },
  },

  /* PREFIX_VEX_0FEF */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FEF_P_2) },
  },

  /* PREFIX_VEX_0FF0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0FF0_PREFIX_3) },
  },

  /* PREFIX_VEX_0FF1 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FF1_P_2) },
  },

  /* PREFIX_VEX_0FF2 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FF2_P_2) },
  },

  /* PREFIX_VEX_0FF3 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FF3_P_2) },
  },

  /* PREFIX_VEX_0FF4 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FF4_P_2) },
  },

  /* PREFIX_VEX_0FF5 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FF5_P_2) },
  },

  /* PREFIX_VEX_0FF6 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FF6_P_2) },
  },

  /* PREFIX_VEX_0FF7 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FF7_P_2) },
  },

  /* PREFIX_VEX_0FF8 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FF8_P_2) },
  },

  /* PREFIX_VEX_0FF9 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FF9_P_2) },
  },

  /* PREFIX_VEX_0FFA */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FFA_P_2) },
  },

  /* PREFIX_VEX_0FFB */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FFB_P_2) },
  },

  /* PREFIX_VEX_0FFC */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FFC_P_2) },
  },

  /* PREFIX_VEX_0FFD */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FFD_P_2) },
  },

  /* PREFIX_VEX_0FFE */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FFE_P_2) },
  },

  /* PREFIX_VEX_0F3800 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3800_P_2) },
  },

  /* PREFIX_VEX_0F3801 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3801_P_2) },
  },

  /* PREFIX_VEX_0F3802 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3802_P_2) },
  },

  /* PREFIX_VEX_0F3803 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3803_P_2) },
  },

  /* PREFIX_VEX_0F3804 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3804_P_2) },
  },

  /* PREFIX_VEX_0F3805 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3805_P_2) },
  },

  /* PREFIX_VEX_0F3806 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3806_P_2) },
  },

  /* PREFIX_VEX_0F3807 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3807_P_2) },
  },

  /* PREFIX_VEX_0F3808 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3808_P_2) },
  },

  /* PREFIX_VEX_0F3809 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3809_P_2) },
  },

  /* PREFIX_VEX_0F380A */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F380A_P_2) },
  },

  /* PREFIX_VEX_0F380B */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F380B_P_2) },
  },

  /* PREFIX_VEX_0F380C */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F380C_P_2) },
  },

  /* PREFIX_VEX_0F380D */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F380D_P_2) },
  },

  /* PREFIX_VEX_0F380E */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F380E_P_2) },
  },

  /* PREFIX_VEX_0F380F */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F380F_P_2) },
  },

  /* PREFIX_VEX_0F3813 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vcvtph2ps", { XM, EXxmmq } },
  },

  /* PREFIX_VEX_0F3816 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3816_P_2) },
  },

  /* PREFIX_VEX_0F3817 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3817_P_2) },
  },

  /* PREFIX_VEX_0F3818 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3818_P_2) },
  },

  /* PREFIX_VEX_0F3819 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3819_P_2) },
  },

  /* PREFIX_VEX_0F381A */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F381A_PREFIX_2) },
  },

  /* PREFIX_VEX_0F381C */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F381C_P_2) },
  },

  /* PREFIX_VEX_0F381D */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F381D_P_2) },
  },

  /* PREFIX_VEX_0F381E */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F381E_P_2) },
  },

  /* PREFIX_VEX_0F3820 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3820_P_2) },
  },

  /* PREFIX_VEX_0F3821 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3821_P_2) },
  },

  /* PREFIX_VEX_0F3822 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3822_P_2) },
  },

  /* PREFIX_VEX_0F3823 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3823_P_2) },
  },

  /* PREFIX_VEX_0F3824 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3824_P_2) },
  },

  /* PREFIX_VEX_0F3825 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3825_P_2) },
  },

  /* PREFIX_VEX_0F3828 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3828_P_2) },
  },

  /* PREFIX_VEX_0F3829 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3829_P_2) },
  },

  /* PREFIX_VEX_0F382A */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F382A_PREFIX_2) },
  },

  /* PREFIX_VEX_0F382B */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F382B_P_2) },
  },

  /* PREFIX_VEX_0F382C */
  {
    { Bad_Opcode },
    { Bad_Opcode },
     { MOD_TABLE (MOD_VEX_0F382C_PREFIX_2) },
  },

  /* PREFIX_VEX_0F382D */
  {
    { Bad_Opcode },
    { Bad_Opcode },
     { MOD_TABLE (MOD_VEX_0F382D_PREFIX_2) },
  },

  /* PREFIX_VEX_0F382E */
  {
    { Bad_Opcode },
    { Bad_Opcode },
     { MOD_TABLE (MOD_VEX_0F382E_PREFIX_2) },
  },

  /* PREFIX_VEX_0F382F */
  {
    { Bad_Opcode },
    { Bad_Opcode },
     { MOD_TABLE (MOD_VEX_0F382F_PREFIX_2) },
  },

  /* PREFIX_VEX_0F3830 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3830_P_2) },
  },

  /* PREFIX_VEX_0F3831 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3831_P_2) },
  },

  /* PREFIX_VEX_0F3832 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3832_P_2) },
  },

  /* PREFIX_VEX_0F3833 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3833_P_2) },
  },

  /* PREFIX_VEX_0F3834 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3834_P_2) },
  },

  /* PREFIX_VEX_0F3835 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3835_P_2) },
  },

  /* PREFIX_VEX_0F3836 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3836_P_2) },
  },

  /* PREFIX_VEX_0F3837 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3837_P_2) },
  },

  /* PREFIX_VEX_0F3838 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3838_P_2) },
  },

  /* PREFIX_VEX_0F3839 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3839_P_2) },
  },

  /* PREFIX_VEX_0F383A */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F383A_P_2) },
  },

  /* PREFIX_VEX_0F383B */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F383B_P_2) },
  },

  /* PREFIX_VEX_0F383C */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F383C_P_2) },
  },

  /* PREFIX_VEX_0F383D */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F383D_P_2) },
  },

  /* PREFIX_VEX_0F383E */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F383E_P_2) },
  },

  /* PREFIX_VEX_0F383F */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F383F_P_2) },
  },

  /* PREFIX_VEX_0F3840 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3840_P_2) },
  },

  /* PREFIX_VEX_0F3841 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3841_P_2) },
  },

  /* PREFIX_VEX_0F3845 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vpsrlv%LW", { XM, Vex, EXx } },
  },

  /* PREFIX_VEX_0F3846 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3846_P_2) },
  },

  /* PREFIX_VEX_0F3847 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vpsllv%LW", { XM, Vex, EXx } },
  },

  /* PREFIX_VEX_0F3858 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3858_P_2) },
  },

  /* PREFIX_VEX_0F3859 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3859_P_2) },
  },

  /* PREFIX_VEX_0F385A */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F385A_PREFIX_2) },
  },

  /* PREFIX_VEX_0F3878 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3878_P_2) },
  },

  /* PREFIX_VEX_0F3879 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3879_P_2) },
  },

  /* PREFIX_VEX_0F388C */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F388C_PREFIX_2) },
  },

  /* PREFIX_VEX_0F388E */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F388E_PREFIX_2) },
  },

  /* PREFIX_VEX_0F3890 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vpgatherd%LW", { XM, MVexVSIBDWpX, Vex } },
  },

  /* PREFIX_VEX_0F3891 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vpgatherq%LW", { XMGatherQ, MVexVSIBQWpX, VexGatherQ } },
  },

  /* PREFIX_VEX_0F3892 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vgatherdp%XW", { XM, MVexVSIBDWpX, Vex } },
  },

  /* PREFIX_VEX_0F3893 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vgatherqp%XW", { XMGatherQ, MVexVSIBQWpX, VexGatherQ } },
  },

  /* PREFIX_VEX_0F3896 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmaddsub132p%XW", { XM, Vex, EXx } },
  },

  /* PREFIX_VEX_0F3897 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmsubadd132p%XW", { XM, Vex, EXx } },
  },

  /* PREFIX_VEX_0F3898 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmadd132p%XW", { XM, Vex, EXx } },
  },

  /* PREFIX_VEX_0F3899 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmadd132s%XW", { XMScalar, VexScalar, EXVexWdqScalar } },
  },

  /* PREFIX_VEX_0F389A */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmsub132p%XW", { XM, Vex, EXx } },
  },

  /* PREFIX_VEX_0F389B */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmsub132s%XW", { XMScalar, VexScalar, EXVexWdqScalar } },
  },

  /* PREFIX_VEX_0F389C */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfnmadd132p%XW", { XM, Vex, EXx } },
  },

  /* PREFIX_VEX_0F389D */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfnmadd132s%XW", { XMScalar, VexScalar, EXVexWdqScalar } },
  },

  /* PREFIX_VEX_0F389E */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfnmsub132p%XW", { XM, Vex, EXx } },
  },

  /* PREFIX_VEX_0F389F */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfnmsub132s%XW", { XMScalar, VexScalar, EXVexWdqScalar } },
  },

  /* PREFIX_VEX_0F38A6 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmaddsub213p%XW", { XM, Vex, EXx } },
    { Bad_Opcode },
  },

  /* PREFIX_VEX_0F38A7 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmsubadd213p%XW", { XM, Vex, EXx } },
  },

  /* PREFIX_VEX_0F38A8 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmadd213p%XW", { XM, Vex, EXx } },
  },

  /* PREFIX_VEX_0F38A9 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmadd213s%XW", { XMScalar, VexScalar, EXVexWdqScalar } },
  },

  /* PREFIX_VEX_0F38AA */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmsub213p%XW", { XM, Vex, EXx } },
  },

  /* PREFIX_VEX_0F38AB */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmsub213s%XW", { XMScalar, VexScalar, EXVexWdqScalar } },
  },

  /* PREFIX_VEX_0F38AC */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfnmadd213p%XW", { XM, Vex, EXx } },
  },

  /* PREFIX_VEX_0F38AD */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfnmadd213s%XW", { XMScalar, VexScalar, EXVexWdqScalar } },
  },

  /* PREFIX_VEX_0F38AE */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfnmsub213p%XW", { XM, Vex, EXx } },
  },

  /* PREFIX_VEX_0F38AF */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfnmsub213s%XW", { XMScalar, VexScalar, EXVexWdqScalar } },
  },

  /* PREFIX_VEX_0F38B6 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmaddsub231p%XW", { XM, Vex, EXx } },
  },

  /* PREFIX_VEX_0F38B7 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmsubadd231p%XW", { XM, Vex, EXx } },
  },

  /* PREFIX_VEX_0F38B8 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmadd231p%XW", { XM, Vex, EXx } },
  },

  /* PREFIX_VEX_0F38B9 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmadd231s%XW", { XMScalar, VexScalar, EXVexWdqScalar } },
  },

  /* PREFIX_VEX_0F38BA */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmsub231p%XW", { XM, Vex, EXx } },
  },

  /* PREFIX_VEX_0F38BB */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmsub231s%XW", { XMScalar, VexScalar, EXVexWdqScalar } },
  },

  /* PREFIX_VEX_0F38BC */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfnmadd231p%XW", { XM, Vex, EXx } },
  },

  /* PREFIX_VEX_0F38BD */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfnmadd231s%XW", { XMScalar, VexScalar, EXVexWdqScalar } },
  },

  /* PREFIX_VEX_0F38BE */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfnmsub231p%XW", { XM, Vex, EXx } },
  },

  /* PREFIX_VEX_0F38BF */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfnmsub231s%XW", { XMScalar, VexScalar, EXVexWdqScalar } },
  },

  /* PREFIX_VEX_0F38DB */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F38DB_P_2) },
  },

  /* PREFIX_VEX_0F38DC */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F38DC_P_2) },
  },

  /* PREFIX_VEX_0F38DD */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F38DD_P_2) },
  },

  /* PREFIX_VEX_0F38DE */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F38DE_P_2) },
  },

  /* PREFIX_VEX_0F38DF */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F38DF_P_2) },
  },

  /* PREFIX_VEX_0F38F2 */
  {
    { VEX_LEN_TABLE (VEX_LEN_0F38F2_P_0) },
  },

  /* PREFIX_VEX_0F38F3_REG_1 */
  {
    { VEX_LEN_TABLE (VEX_LEN_0F38F3_R_1_P_0) },
  },

  /* PREFIX_VEX_0F38F3_REG_2 */
  {
    { VEX_LEN_TABLE (VEX_LEN_0F38F3_R_2_P_0) },
  },

  /* PREFIX_VEX_0F38F3_REG_3 */
  {
    { VEX_LEN_TABLE (VEX_LEN_0F38F3_R_3_P_0) },
  },

  /* PREFIX_VEX_0F38F5 */
  {
    { VEX_LEN_TABLE (VEX_LEN_0F38F5_P_0) },
    { VEX_LEN_TABLE (VEX_LEN_0F38F5_P_1) },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F38F5_P_3) },
  },

  /* PREFIX_VEX_0F38F6 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F38F6_P_3) },
  },

  /* PREFIX_VEX_0F38F7 */
  {
    { VEX_LEN_TABLE (VEX_LEN_0F38F7_P_0) },
    { VEX_LEN_TABLE (VEX_LEN_0F38F7_P_1) },
    { VEX_LEN_TABLE (VEX_LEN_0F38F7_P_2) },
    { VEX_LEN_TABLE (VEX_LEN_0F38F7_P_3) },
  },

  /* PREFIX_VEX_0F3A00 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A00_P_2) },
  },

  /* PREFIX_VEX_0F3A01 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A01_P_2) },
  },

  /* PREFIX_VEX_0F3A02 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A02_P_2) },
  },

  /* PREFIX_VEX_0F3A04 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A04_P_2) },
  },

  /* PREFIX_VEX_0F3A05 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A05_P_2) },
  },

  /* PREFIX_VEX_0F3A06 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A06_P_2) },
  },

  /* PREFIX_VEX_0F3A08 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A08_P_2) },
  },

  /* PREFIX_VEX_0F3A09 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A09_P_2) },
  },

  /* PREFIX_VEX_0F3A0A */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A0A_P_2) },
  },

  /* PREFIX_VEX_0F3A0B */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A0B_P_2) },
  },

  /* PREFIX_VEX_0F3A0C */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A0C_P_2) },
  },

  /* PREFIX_VEX_0F3A0D */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A0D_P_2) },
  },

  /* PREFIX_VEX_0F3A0E */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A0E_P_2) },
  },

  /* PREFIX_VEX_0F3A0F */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A0F_P_2) },
  },

  /* PREFIX_VEX_0F3A14 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A14_P_2) },
  },

  /* PREFIX_VEX_0F3A15 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A15_P_2) },
  },

  /* PREFIX_VEX_0F3A16 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A16_P_2) },
  },

  /* PREFIX_VEX_0F3A17 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A17_P_2) },
  },

  /* PREFIX_VEX_0F3A18 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A18_P_2) },
  },

  /* PREFIX_VEX_0F3A19 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A19_P_2) },
  },

  /* PREFIX_VEX_0F3A1D */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vcvtps2ph", { EXxmmq, XM, Ib } },
  },

  /* PREFIX_VEX_0F3A20 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A20_P_2) },
  },

  /* PREFIX_VEX_0F3A21 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A21_P_2) },
  },

  /* PREFIX_VEX_0F3A22 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A22_P_2) },
  },

  /* PREFIX_VEX_0F3A38 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A38_P_2) },
  },

  /* PREFIX_VEX_0F3A39 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A39_P_2) },
  },

  /* PREFIX_VEX_0F3A40 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A40_P_2) },
  },

  /* PREFIX_VEX_0F3A41 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A41_P_2) },
  },

  /* PREFIX_VEX_0F3A42 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A42_P_2) },
  },

  /* PREFIX_VEX_0F3A44 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A44_P_2) },
  },

  /* PREFIX_VEX_0F3A46 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A46_P_2) },
  },

  /* PREFIX_VEX_0F3A48 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A48_P_2) },
  },

  /* PREFIX_VEX_0F3A49 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A49_P_2) },
  },

  /* PREFIX_VEX_0F3A4A */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A4A_P_2) },
  },

  /* PREFIX_VEX_0F3A4B */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A4B_P_2) },
  },

  /* PREFIX_VEX_0F3A4C */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A4C_P_2) },
  },

  /* PREFIX_VEX_0F3A5C */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmaddsubps", { XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
  },

  /* PREFIX_VEX_0F3A5D */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmaddsubpd", { XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
  },

  /* PREFIX_VEX_0F3A5E */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmsubaddps", { XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
  },

  /* PREFIX_VEX_0F3A5F */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmsubaddpd", { XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
  },

  /* PREFIX_VEX_0F3A60 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A60_P_2) },
    { Bad_Opcode },
  },

  /* PREFIX_VEX_0F3A61 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A61_P_2) },
  },

  /* PREFIX_VEX_0F3A62 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A62_P_2) },
  },

  /* PREFIX_VEX_0F3A63 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A63_P_2) },
  },

  /* PREFIX_VEX_0F3A68 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmaddps", { XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
  },

  /* PREFIX_VEX_0F3A69 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmaddpd", { XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
  },

  /* PREFIX_VEX_0F3A6A */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A6A_P_2) },
  },

  /* PREFIX_VEX_0F3A6B */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A6B_P_2) },
  },

  /* PREFIX_VEX_0F3A6C */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmsubps", { XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
  },

  /* PREFIX_VEX_0F3A6D */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmsubpd", { XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
  },

  /* PREFIX_VEX_0F3A6E */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A6E_P_2) },
  },

  /* PREFIX_VEX_0F3A6F */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A6F_P_2) },
  },

  /* PREFIX_VEX_0F3A78 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfnmaddps", { XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
  },

  /* PREFIX_VEX_0F3A79 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfnmaddpd", { XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
  },

  /* PREFIX_VEX_0F3A7A */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A7A_P_2) },
  },

  /* PREFIX_VEX_0F3A7B */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A7B_P_2) },
  },

  /* PREFIX_VEX_0F3A7C */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfnmsubps", { XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
    { Bad_Opcode },
  },

  /* PREFIX_VEX_0F3A7D */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfnmsubpd", { XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
  },

  /* PREFIX_VEX_0F3A7E */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A7E_P_2) },
  },

  /* PREFIX_VEX_0F3A7F */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A7F_P_2) },
  },

  /* PREFIX_VEX_0F3ADF */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3ADF_P_2) },
  },

  /* PREFIX_VEX_0F3AF0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3AF0_P_3) },
  },
};

static const struct dis386 x86_64_table[][2] = {
  /* X86_64_06 */
  {
    { "pushP", { es } },
  },

  /* X86_64_07 */
  {
    { "popP", { es } },
  },

  /* X86_64_0D */
  {
    { "pushP", { cs } },
  },

  /* X86_64_16 */
  {
    { "pushP", { ss } },
  },

  /* X86_64_17 */
  {
    { "popP", { ss } },
  },

  /* X86_64_1E */
  {
    { "pushP", { ds } },
  },

  /* X86_64_1F */
  {
    { "popP", { ds } },
  },

  /* X86_64_27 */
  {
    { "daa", { XX } },
  },

  /* X86_64_2F */
  {
    { "das", { XX } },
  },

  /* X86_64_37 */
  {
    { "aaa", { XX } },
  },

  /* X86_64_3F */
  {
    { "aas", { XX } },
  },

  /* X86_64_60 */
  {
    { "pushaP", { XX } },
  },

  /* X86_64_61 */
  {
    { "popaP", { XX } },
  },

  /* X86_64_62 */
  {
    { MOD_TABLE (MOD_62_32BIT) },
  },

  /* X86_64_63 */
  {
    { "arpl", { Ew, Gw } },
    { "movs{lq|xd}", { Gv, Ed } },
  },

  /* X86_64_6D */
  {
    { "ins{R|}", { Yzr, indirDX } },
    { "ins{G|}", { Yzr, indirDX } },
  },

  /* X86_64_6F */
  {
    { "outs{R|}", { indirDXr, Xz } },
    { "outs{G|}", { indirDXr, Xz } },
  },

  /* X86_64_9A */
  {
    { "Jcall{T|}", { Ap } },
  },

  /* X86_64_C4 */
  {
    { MOD_TABLE (MOD_C4_32BIT) },
    { VEX_C4_TABLE (VEX_0F) },
  },

  /* X86_64_C5 */
  {
    { MOD_TABLE (MOD_C5_32BIT) },
    { VEX_C5_TABLE (VEX_0F) },
  },

  /* X86_64_CE */
  {
    { "into", { XX } },
  },

  /* X86_64_D4 */
  {
    { "aam", { Ib } },
  },

  /* X86_64_D5 */
  {
    { "aad", { Ib } },
  },

  /* X86_64_EA */
  {
    { "Jjmp{T|}", { Ap } },
  },

  /* X86_64_0F01_REG_0 */
  {
    { "sgdt{Q|IQ}", { M } },
    { "sgdt", { M } },
  },

  /* X86_64_0F01_REG_1 */
  {
    { "sidt{Q|IQ}", { M } },
    { "sidt", { M } },
  },

  /* X86_64_0F01_REG_2 */
  {
    { "lgdt{Q|Q}", { M } },
    { "lgdt", { M } },
  },

  /* X86_64_0F01_REG_3 */
  {
    { "lidt{Q|Q}", { M } },
    { "lidt", { M } },
  },
};

static const struct dis386 three_byte_table[][256] = {

  /* THREE_BYTE_0F38 */
  {
    /* 00 */
    { "pshufb",		{ MX, EM } },
    { "phaddw",		{ MX, EM } },
    { "phaddd",		{ MX, EM } },
    { "phaddsw",	{ MX, EM } },
    { "pmaddubsw",	{ MX, EM } },
    { "phsubw",		{ MX, EM } },
    { "phsubd",		{ MX, EM } },
    { "phsubsw",	{ MX, EM } },
    /* 08 */
    { "psignb",		{ MX, EM } },
    { "psignw",		{ MX, EM } },
    { "psignd",		{ MX, EM } },
    { "pmulhrsw",	{ MX, EM } },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 10 */
    { PREFIX_TABLE (PREFIX_0F3810) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_0F3814) },
    { PREFIX_TABLE (PREFIX_0F3815) },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_0F3817) },
    /* 18 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "pabsb",		{ MX, EM } },
    { "pabsw",		{ MX, EM } },
    { "pabsd",		{ MX, EM } },
    { Bad_Opcode },
    /* 20 */
    { PREFIX_TABLE (PREFIX_0F3820) },
    { PREFIX_TABLE (PREFIX_0F3821) },
    { PREFIX_TABLE (PREFIX_0F3822) },
    { PREFIX_TABLE (PREFIX_0F3823) },
    { PREFIX_TABLE (PREFIX_0F3824) },
    { PREFIX_TABLE (PREFIX_0F3825) },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 28 */
    { PREFIX_TABLE (PREFIX_0F3828) },
    { PREFIX_TABLE (PREFIX_0F3829) },
    { PREFIX_TABLE (PREFIX_0F382A) },
    { PREFIX_TABLE (PREFIX_0F382B) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 30 */
    { PREFIX_TABLE (PREFIX_0F3830) },
    { PREFIX_TABLE (PREFIX_0F3831) },
    { PREFIX_TABLE (PREFIX_0F3832) },
    { PREFIX_TABLE (PREFIX_0F3833) },
    { PREFIX_TABLE (PREFIX_0F3834) },
    { PREFIX_TABLE (PREFIX_0F3835) },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_0F3837) },
    /* 38 */
    { PREFIX_TABLE (PREFIX_0F3838) },
    { PREFIX_TABLE (PREFIX_0F3839) },
    { PREFIX_TABLE (PREFIX_0F383A) },
    { PREFIX_TABLE (PREFIX_0F383B) },
    { PREFIX_TABLE (PREFIX_0F383C) },
    { PREFIX_TABLE (PREFIX_0F383D) },
    { PREFIX_TABLE (PREFIX_0F383E) },
    { PREFIX_TABLE (PREFIX_0F383F) },
    /* 40 */
    { PREFIX_TABLE (PREFIX_0F3840) },
    { PREFIX_TABLE (PREFIX_0F3841) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 48 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 50 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 58 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 60 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 68 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 70 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 78 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 80 */
    { PREFIX_TABLE (PREFIX_0F3880) },
    { PREFIX_TABLE (PREFIX_0F3881) },
    { PREFIX_TABLE (PREFIX_0F3882) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 88 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 90 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 98 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_0F38DB) },
    { PREFIX_TABLE (PREFIX_0F38DC) },
    { PREFIX_TABLE (PREFIX_0F38DD) },
    { PREFIX_TABLE (PREFIX_0F38DE) },
    { PREFIX_TABLE (PREFIX_0F38DF) },
    /* e0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f0 */
    { PREFIX_TABLE (PREFIX_0F38F0) },
    { PREFIX_TABLE (PREFIX_0F38F1) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_0F38F6) },
    { Bad_Opcode },
    /* f8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
  },
  /* THREE_BYTE_0F3A */
  {
    /* 00 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 08 */
    { PREFIX_TABLE (PREFIX_0F3A08) },
    { PREFIX_TABLE (PREFIX_0F3A09) },
    { PREFIX_TABLE (PREFIX_0F3A0A) },
    { PREFIX_TABLE (PREFIX_0F3A0B) },
    { PREFIX_TABLE (PREFIX_0F3A0C) },
    { PREFIX_TABLE (PREFIX_0F3A0D) },
    { PREFIX_TABLE (PREFIX_0F3A0E) },
    { "palignr",	{ MX, EM, Ib } },
    /* 10 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_0F3A14) },
    { PREFIX_TABLE (PREFIX_0F3A15) },
    { PREFIX_TABLE (PREFIX_0F3A16) },
    { PREFIX_TABLE (PREFIX_0F3A17) },
    /* 18 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 20 */
    { PREFIX_TABLE (PREFIX_0F3A20) },
    { PREFIX_TABLE (PREFIX_0F3A21) },
    { PREFIX_TABLE (PREFIX_0F3A22) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 28 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 30 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 38 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 40 */
    { PREFIX_TABLE (PREFIX_0F3A40) },
    { PREFIX_TABLE (PREFIX_0F3A41) },
    { PREFIX_TABLE (PREFIX_0F3A42) },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_0F3A44) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 48 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 50 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 58 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 60 */
    { PREFIX_TABLE (PREFIX_0F3A60) },
    { PREFIX_TABLE (PREFIX_0F3A61) },
    { PREFIX_TABLE (PREFIX_0F3A62) },
    { PREFIX_TABLE (PREFIX_0F3A63) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 68 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 70 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 78 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 80 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 88 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 90 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 98 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_0F3ADF) },
    /* e0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
  },

  /* THREE_BYTE_0F7A */
  {
    /* 00 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 08 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 10 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 18 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 20 */
    { "ptest",		{ XX } },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 28 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 30 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 38 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 40 */
    { Bad_Opcode },
    { "phaddbw",	{ XM, EXq } },
    { "phaddbd",	{ XM, EXq } },
    { "phaddbq",	{ XM, EXq } },
    { Bad_Opcode },
    { Bad_Opcode },
    { "phaddwd",	{ XM, EXq } },
    { "phaddwq",	{ XM, EXq } },
    /* 48 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "phadddq",	{ XM, EXq } },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 50 */
    { Bad_Opcode },
    { "phaddubw",	{ XM, EXq } },
    { "phaddubd",	{ XM, EXq } },
    { "phaddubq",	{ XM, EXq } },
    { Bad_Opcode },
    { Bad_Opcode },
    { "phadduwd",	{ XM, EXq } },
    { "phadduwq",	{ XM, EXq } },
    /* 58 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "phaddudq",	{ XM, EXq } },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 60 */
    { Bad_Opcode },
    { "phsubbw",	{ XM, EXq } },
    { "phsubbd",	{ XM, EXq } },
    { "phsubbq",	{ XM, EXq } },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 68 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 70 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 78 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 80 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 88 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 90 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 98 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
  },
};

static const struct dis386 xop_table[][256] = {
  /* XOP_08 */
  {
    /* 00 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 08 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 10 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 18 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 20 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 28 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 30 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 38 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 40 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 48 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 50 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 58 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 60 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 68 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 70 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 78 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 80 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "vpmacssww", 	{ XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
    { "vpmacsswd", 	{ XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
    { "vpmacssdql", 	{ XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
    /* 88 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "vpmacssdd", 	{ XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
    { "vpmacssdqh", 	{ XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
    /* 90 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "vpmacsww", 	{ XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
    { "vpmacswd", 	{ XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
    { "vpmacsdql", 	{ XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
    /* 98 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "vpmacsdd", 	{ XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
    { "vpmacsdqh", 	{ XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
    /* a0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { "vpcmov", 	{ XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
    { "vpperm", 	{ XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
    { Bad_Opcode },
    { Bad_Opcode },
    { "vpmadcsswd", 	{ XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
    { Bad_Opcode },
    /* a8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "vpmadcswd", 	{ XMVexW, Vex, EXVexW, EXVexW, VexI4 } },
    { Bad_Opcode },
    /* b8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c0 */
    { "vprotb", 	{ XM, Vex_2src_1, Ib } },
    { "vprotw", 	{ XM, Vex_2src_1, Ib } },
    { "vprotd", 	{ XM, Vex_2src_1, Ib } },
    { "vprotq", 	{ XM, Vex_2src_1, Ib } },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_CC) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_CD) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_CE) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_CF) },
    /* d0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_EC) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_ED) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_EE) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_EF) },
    /* f0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
  },
  /* XOP_09 */
  {
    /* 00 */
    { Bad_Opcode },
    { REG_TABLE (REG_XOP_TBM_01) },
    { REG_TABLE (REG_XOP_TBM_02) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 08 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 10 */
    { Bad_Opcode },
    { Bad_Opcode },
    { REG_TABLE (REG_XOP_LWPCB) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 18 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 20 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 28 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 30 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 38 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 40 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 48 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 50 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 58 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 60 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 68 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 70 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 78 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 80 */
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_80) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_81) },
    { "vfrczss", 	{ XM, EXd } },
    { "vfrczsd", 	{ XM, EXq } },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 88 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 90 */
    { "vprotb",		{ XM, Vex_2src_1, Vex_2src_2 } },
    { "vprotw",		{ XM, Vex_2src_1, Vex_2src_2 } },
    { "vprotd",		{ XM, Vex_2src_1, Vex_2src_2 } },
    { "vprotq",		{ XM, Vex_2src_1, Vex_2src_2 } },
    { "vpshlb",		{ XM, Vex_2src_1, Vex_2src_2 } },
    { "vpshlw",		{ XM, Vex_2src_1, Vex_2src_2 } },
    { "vpshld",		{ XM, Vex_2src_1, Vex_2src_2 } },
    { "vpshlq",		{ XM, Vex_2src_1, Vex_2src_2 } },
    /* 98 */
    { "vpshab",		{ XM, Vex_2src_1, Vex_2src_2 } },
    { "vpshaw",		{ XM, Vex_2src_1, Vex_2src_2 } },
    { "vpshad",		{ XM, Vex_2src_1, Vex_2src_2 } },
    { "vpshaq",		{ XM, Vex_2src_1, Vex_2src_2 } },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c0 */
    { Bad_Opcode },
    { "vphaddbw",	{ XM, EXxmm } },
    { "vphaddbd",	{ XM, EXxmm } },
    { "vphaddbq",	{ XM, EXxmm } },
    { Bad_Opcode },
    { Bad_Opcode },
    { "vphaddwd",	{ XM, EXxmm } },
    { "vphaddwq",	{ XM, EXxmm } },
    /* c8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "vphadddq",	{ XM, EXxmm } },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d0 */
    { Bad_Opcode },
    { "vphaddubw",	{ XM, EXxmm } },
    { "vphaddubd",	{ XM, EXxmm } },
    { "vphaddubq",	{ XM, EXxmm } },
    { Bad_Opcode },
    { Bad_Opcode },
    { "vphadduwd",	{ XM, EXxmm } },
    { "vphadduwq",	{ XM, EXxmm } },
    /* d8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "vphaddudq",	{ XM, EXxmm } },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e0 */
    { Bad_Opcode },
    { "vphsubbw",	{ XM, EXxmm } },
    { "vphsubwd",	{ XM, EXxmm } },
    { "vphsubdq",	{ XM, EXxmm } },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
  },
  /* XOP_0A */
  {
    /* 00 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 08 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 10 */
    { "bextr",	{ Gv, Ev, Iq } },
    { Bad_Opcode },
    { REG_TABLE (REG_XOP_LWP) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 18 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 20 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 28 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 30 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 38 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 40 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 48 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 50 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 58 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 60 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 68 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 70 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 78 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 80 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 88 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 90 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 98 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
  },
};

static const struct dis386 vex_table[][256] = {
  /* VEX_0F */
  {
    /* 00 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 08 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 10 */
    { PREFIX_TABLE (PREFIX_VEX_0F10) },
    { PREFIX_TABLE (PREFIX_VEX_0F11) },
    { PREFIX_TABLE (PREFIX_VEX_0F12) },
    { MOD_TABLE (MOD_VEX_0F13) },
    { VEX_W_TABLE (VEX_W_0F14) },
    { VEX_W_TABLE (VEX_W_0F15) },
    { PREFIX_TABLE (PREFIX_VEX_0F16) },
    { MOD_TABLE (MOD_VEX_0F17) },
    /* 18 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 20 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 28 */
    { VEX_W_TABLE (VEX_W_0F28) },
    { VEX_W_TABLE (VEX_W_0F29) },
    { PREFIX_TABLE (PREFIX_VEX_0F2A) },
    { MOD_TABLE (MOD_VEX_0F2B) },
    { PREFIX_TABLE (PREFIX_VEX_0F2C) },
    { PREFIX_TABLE (PREFIX_VEX_0F2D) },
    { PREFIX_TABLE (PREFIX_VEX_0F2E) },
    { PREFIX_TABLE (PREFIX_VEX_0F2F) },
    /* 30 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 38 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 40 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 48 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 50 */
    { MOD_TABLE (MOD_VEX_0F50) },
    { PREFIX_TABLE (PREFIX_VEX_0F51) },
    { PREFIX_TABLE (PREFIX_VEX_0F52) },
    { PREFIX_TABLE (PREFIX_VEX_0F53) },
    { "vandpX",		{ XM, Vex, EXx } },
    { "vandnpX",	{ XM, Vex, EXx } },
    { "vorpX",		{ XM, Vex, EXx } },
    { "vxorpX",		{ XM, Vex, EXx } },
    /* 58 */
    { PREFIX_TABLE (PREFIX_VEX_0F58) },
    { PREFIX_TABLE (PREFIX_VEX_0F59) },
    { PREFIX_TABLE (PREFIX_VEX_0F5A) },
    { PREFIX_TABLE (PREFIX_VEX_0F5B) },
    { PREFIX_TABLE (PREFIX_VEX_0F5C) },
    { PREFIX_TABLE (PREFIX_VEX_0F5D) },
    { PREFIX_TABLE (PREFIX_VEX_0F5E) },
    { PREFIX_TABLE (PREFIX_VEX_0F5F) },
    /* 60 */
    { PREFIX_TABLE (PREFIX_VEX_0F60) },
    { PREFIX_TABLE (PREFIX_VEX_0F61) },
    { PREFIX_TABLE (PREFIX_VEX_0F62) },
    { PREFIX_TABLE (PREFIX_VEX_0F63) },
    { PREFIX_TABLE (PREFIX_VEX_0F64) },
    { PREFIX_TABLE (PREFIX_VEX_0F65) },
    { PREFIX_TABLE (PREFIX_VEX_0F66) },
    { PREFIX_TABLE (PREFIX_VEX_0F67) },
    /* 68 */
    { PREFIX_TABLE (PREFIX_VEX_0F68) },
    { PREFIX_TABLE (PREFIX_VEX_0F69) },
    { PREFIX_TABLE (PREFIX_VEX_0F6A) },
    { PREFIX_TABLE (PREFIX_VEX_0F6B) },
    { PREFIX_TABLE (PREFIX_VEX_0F6C) },
    { PREFIX_TABLE (PREFIX_VEX_0F6D) },
    { PREFIX_TABLE (PREFIX_VEX_0F6E) },
    { PREFIX_TABLE (PREFIX_VEX_0F6F) },
    /* 70 */
    { PREFIX_TABLE (PREFIX_VEX_0F70) },
    { REG_TABLE (REG_VEX_0F71) },
    { REG_TABLE (REG_VEX_0F72) },
    { REG_TABLE (REG_VEX_0F73) },
    { PREFIX_TABLE (PREFIX_VEX_0F74) },
    { PREFIX_TABLE (PREFIX_VEX_0F75) },
    { PREFIX_TABLE (PREFIX_VEX_0F76) },
    { PREFIX_TABLE (PREFIX_VEX_0F77) },
    /* 78 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F7C) },
    { PREFIX_TABLE (PREFIX_VEX_0F7D) },
    { PREFIX_TABLE (PREFIX_VEX_0F7E) },
    { PREFIX_TABLE (PREFIX_VEX_0F7F) },
    /* 80 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 88 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 90 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 98 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { REG_TABLE (REG_VEX_0FAE) },
    { Bad_Opcode },
    /* b0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0FC2) },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0FC4) },
    { PREFIX_TABLE (PREFIX_VEX_0FC5) },
    { "vshufpX",	{ XM, Vex, EXx, Ib } },
    { Bad_Opcode },
    /* c8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d0 */
    { PREFIX_TABLE (PREFIX_VEX_0FD0) },
    { PREFIX_TABLE (PREFIX_VEX_0FD1) },
    { PREFIX_TABLE (PREFIX_VEX_0FD2) },
    { PREFIX_TABLE (PREFIX_VEX_0FD3) },
    { PREFIX_TABLE (PREFIX_VEX_0FD4) },
    { PREFIX_TABLE (PREFIX_VEX_0FD5) },
    { PREFIX_TABLE (PREFIX_VEX_0FD6) },
    { PREFIX_TABLE (PREFIX_VEX_0FD7) },
    /* d8 */
    { PREFIX_TABLE (PREFIX_VEX_0FD8) },
    { PREFIX_TABLE (PREFIX_VEX_0FD9) },
    { PREFIX_TABLE (PREFIX_VEX_0FDA) },
    { PREFIX_TABLE (PREFIX_VEX_0FDB) },
    { PREFIX_TABLE (PREFIX_VEX_0FDC) },
    { PREFIX_TABLE (PREFIX_VEX_0FDD) },
    { PREFIX_TABLE (PREFIX_VEX_0FDE) },
    { PREFIX_TABLE (PREFIX_VEX_0FDF) },
    /* e0 */
    { PREFIX_TABLE (PREFIX_VEX_0FE0) },
    { PREFIX_TABLE (PREFIX_VEX_0FE1) },
    { PREFIX_TABLE (PREFIX_VEX_0FE2) },
    { PREFIX_TABLE (PREFIX_VEX_0FE3) },
    { PREFIX_TABLE (PREFIX_VEX_0FE4) },
    { PREFIX_TABLE (PREFIX_VEX_0FE5) },
    { PREFIX_TABLE (PREFIX_VEX_0FE6) },
    { PREFIX_TABLE (PREFIX_VEX_0FE7) },
    /* e8 */
    { PREFIX_TABLE (PREFIX_VEX_0FE8) },
    { PREFIX_TABLE (PREFIX_VEX_0FE9) },
    { PREFIX_TABLE (PREFIX_VEX_0FEA) },
    { PREFIX_TABLE (PREFIX_VEX_0FEB) },
    { PREFIX_TABLE (PREFIX_VEX_0FEC) },
    { PREFIX_TABLE (PREFIX_VEX_0FED) },
    { PREFIX_TABLE (PREFIX_VEX_0FEE) },
    { PREFIX_TABLE (PREFIX_VEX_0FEF) },
    /* f0 */
    { PREFIX_TABLE (PREFIX_VEX_0FF0) },
    { PREFIX_TABLE (PREFIX_VEX_0FF1) },
    { PREFIX_TABLE (PREFIX_VEX_0FF2) },
    { PREFIX_TABLE (PREFIX_VEX_0FF3) },
    { PREFIX_TABLE (PREFIX_VEX_0FF4) },
    { PREFIX_TABLE (PREFIX_VEX_0FF5) },
    { PREFIX_TABLE (PREFIX_VEX_0FF6) },
    { PREFIX_TABLE (PREFIX_VEX_0FF7) },
    /* f8 */
    { PREFIX_TABLE (PREFIX_VEX_0FF8) },
    { PREFIX_TABLE (PREFIX_VEX_0FF9) },
    { PREFIX_TABLE (PREFIX_VEX_0FFA) },
    { PREFIX_TABLE (PREFIX_VEX_0FFB) },
    { PREFIX_TABLE (PREFIX_VEX_0FFC) },
    { PREFIX_TABLE (PREFIX_VEX_0FFD) },
    { PREFIX_TABLE (PREFIX_VEX_0FFE) },
    { Bad_Opcode },
  },
  /* VEX_0F38 */
  {
    /* 00 */
    { PREFIX_TABLE (PREFIX_VEX_0F3800) },
    { PREFIX_TABLE (PREFIX_VEX_0F3801) },
    { PREFIX_TABLE (PREFIX_VEX_0F3802) },
    { PREFIX_TABLE (PREFIX_VEX_0F3803) },
    { PREFIX_TABLE (PREFIX_VEX_0F3804) },
    { PREFIX_TABLE (PREFIX_VEX_0F3805) },
    { PREFIX_TABLE (PREFIX_VEX_0F3806) },
    { PREFIX_TABLE (PREFIX_VEX_0F3807) },
    /* 08 */
    { PREFIX_TABLE (PREFIX_VEX_0F3808) },
    { PREFIX_TABLE (PREFIX_VEX_0F3809) },
    { PREFIX_TABLE (PREFIX_VEX_0F380A) },
    { PREFIX_TABLE (PREFIX_VEX_0F380B) },
    { PREFIX_TABLE (PREFIX_VEX_0F380C) },
    { PREFIX_TABLE (PREFIX_VEX_0F380D) },
    { PREFIX_TABLE (PREFIX_VEX_0F380E) },
    { PREFIX_TABLE (PREFIX_VEX_0F380F) },
    /* 10 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F3813) },
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F3816) },
    { PREFIX_TABLE (PREFIX_VEX_0F3817) },
    /* 18 */
    { PREFIX_TABLE (PREFIX_VEX_0F3818) },
    { PREFIX_TABLE (PREFIX_VEX_0F3819) },
    { PREFIX_TABLE (PREFIX_VEX_0F381A) },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F381C) },
    { PREFIX_TABLE (PREFIX_VEX_0F381D) },
    { PREFIX_TABLE (PREFIX_VEX_0F381E) },
    { Bad_Opcode },
    /* 20 */
    { PREFIX_TABLE (PREFIX_VEX_0F3820) },
    { PREFIX_TABLE (PREFIX_VEX_0F3821) },
    { PREFIX_TABLE (PREFIX_VEX_0F3822) },
    { PREFIX_TABLE (PREFIX_VEX_0F3823) },
    { PREFIX_TABLE (PREFIX_VEX_0F3824) },
    { PREFIX_TABLE (PREFIX_VEX_0F3825) },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 28 */
    { PREFIX_TABLE (PREFIX_VEX_0F3828) },
    { PREFIX_TABLE (PREFIX_VEX_0F3829) },
    { PREFIX_TABLE (PREFIX_VEX_0F382A) },
    { PREFIX_TABLE (PREFIX_VEX_0F382B) },
    { PREFIX_TABLE (PREFIX_VEX_0F382C) },
    { PREFIX_TABLE (PREFIX_VEX_0F382D) },
    { PREFIX_TABLE (PREFIX_VEX_0F382E) },
    { PREFIX_TABLE (PREFIX_VEX_0F382F) },
    /* 30 */
    { PREFIX_TABLE (PREFIX_VEX_0F3830) },
    { PREFIX_TABLE (PREFIX_VEX_0F3831) },
    { PREFIX_TABLE (PREFIX_VEX_0F3832) },
    { PREFIX_TABLE (PREFIX_VEX_0F3833) },
    { PREFIX_TABLE (PREFIX_VEX_0F3834) },
    { PREFIX_TABLE (PREFIX_VEX_0F3835) },
    { PREFIX_TABLE (PREFIX_VEX_0F3836) },
    { PREFIX_TABLE (PREFIX_VEX_0F3837) },
    /* 38 */
    { PREFIX_TABLE (PREFIX_VEX_0F3838) },
    { PREFIX_TABLE (PREFIX_VEX_0F3839) },
    { PREFIX_TABLE (PREFIX_VEX_0F383A) },
    { PREFIX_TABLE (PREFIX_VEX_0F383B) },
    { PREFIX_TABLE (PREFIX_VEX_0F383C) },
    { PREFIX_TABLE (PREFIX_VEX_0F383D) },
    { PREFIX_TABLE (PREFIX_VEX_0F383E) },
    { PREFIX_TABLE (PREFIX_VEX_0F383F) },
    /* 40 */
    { PREFIX_TABLE (PREFIX_VEX_0F3840) },
    { PREFIX_TABLE (PREFIX_VEX_0F3841) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F3845) },
    { PREFIX_TABLE (PREFIX_VEX_0F3846) },
    { PREFIX_TABLE (PREFIX_VEX_0F3847) },
    /* 48 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 50 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 58 */
    { PREFIX_TABLE (PREFIX_VEX_0F3858) },
    { PREFIX_TABLE (PREFIX_VEX_0F3859) },
    { PREFIX_TABLE (PREFIX_VEX_0F385A) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 60 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 68 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 70 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 78 */
    { PREFIX_TABLE (PREFIX_VEX_0F3878) },
    { PREFIX_TABLE (PREFIX_VEX_0F3879) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 80 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 88 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F388C) },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F388E) },
    { Bad_Opcode },
    /* 90 */
    { PREFIX_TABLE (PREFIX_VEX_0F3890) },
    { PREFIX_TABLE (PREFIX_VEX_0F3891) },
    { PREFIX_TABLE (PREFIX_VEX_0F3892) },
    { PREFIX_TABLE (PREFIX_VEX_0F3893) },
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F3896) },
    { PREFIX_TABLE (PREFIX_VEX_0F3897) },
    /* 98 */
    { PREFIX_TABLE (PREFIX_VEX_0F3898) },
    { PREFIX_TABLE (PREFIX_VEX_0F3899) },
    { PREFIX_TABLE (PREFIX_VEX_0F389A) },
    { PREFIX_TABLE (PREFIX_VEX_0F389B) },
    { PREFIX_TABLE (PREFIX_VEX_0F389C) },
    { PREFIX_TABLE (PREFIX_VEX_0F389D) },
    { PREFIX_TABLE (PREFIX_VEX_0F389E) },
    { PREFIX_TABLE (PREFIX_VEX_0F389F) },
    /* a0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F38A6) },
    { PREFIX_TABLE (PREFIX_VEX_0F38A7) },
    /* a8 */
    { PREFIX_TABLE (PREFIX_VEX_0F38A8) },
    { PREFIX_TABLE (PREFIX_VEX_0F38A9) },
    { PREFIX_TABLE (PREFIX_VEX_0F38AA) },
    { PREFIX_TABLE (PREFIX_VEX_0F38AB) },
    { PREFIX_TABLE (PREFIX_VEX_0F38AC) },
    { PREFIX_TABLE (PREFIX_VEX_0F38AD) },
    { PREFIX_TABLE (PREFIX_VEX_0F38AE) },
    { PREFIX_TABLE (PREFIX_VEX_0F38AF) },
    /* b0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F38B6) },
    { PREFIX_TABLE (PREFIX_VEX_0F38B7) },
    /* b8 */
    { PREFIX_TABLE (PREFIX_VEX_0F38B8) },
    { PREFIX_TABLE (PREFIX_VEX_0F38B9) },
    { PREFIX_TABLE (PREFIX_VEX_0F38BA) },
    { PREFIX_TABLE (PREFIX_VEX_0F38BB) },
    { PREFIX_TABLE (PREFIX_VEX_0F38BC) },
    { PREFIX_TABLE (PREFIX_VEX_0F38BD) },
    { PREFIX_TABLE (PREFIX_VEX_0F38BE) },
    { PREFIX_TABLE (PREFIX_VEX_0F38BF) },
    /* c0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F38DB) },
    { PREFIX_TABLE (PREFIX_VEX_0F38DC) },
    { PREFIX_TABLE (PREFIX_VEX_0F38DD) },
    { PREFIX_TABLE (PREFIX_VEX_0F38DE) },
    { PREFIX_TABLE (PREFIX_VEX_0F38DF) },
    /* e0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F38F2) },
    { REG_TABLE (REG_VEX_0F38F3) },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F38F5) },
    { PREFIX_TABLE (PREFIX_VEX_0F38F6) },
    { PREFIX_TABLE (PREFIX_VEX_0F38F7) },
    /* f8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
  },
  /* VEX_0F3A */
  {
    /* 00 */
    { PREFIX_TABLE (PREFIX_VEX_0F3A00) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A01) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A02) },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F3A04) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A05) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A06) },
    { Bad_Opcode },
    /* 08 */
    { PREFIX_TABLE (PREFIX_VEX_0F3A08) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A09) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A0A) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A0B) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A0C) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A0D) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A0E) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A0F) },
    /* 10 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F3A14) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A15) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A16) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A17) },
    /* 18 */
    { PREFIX_TABLE (PREFIX_VEX_0F3A18) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A19) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F3A1D) },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 20 */
    { PREFIX_TABLE (PREFIX_VEX_0F3A20) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A21) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A22) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 28 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 30 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 38 */
    { PREFIX_TABLE (PREFIX_VEX_0F3A38) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A39) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 40 */
    { PREFIX_TABLE (PREFIX_VEX_0F3A40) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A41) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A42) },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F3A44) },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F3A46) },
    { Bad_Opcode },
    /* 48 */
    { PREFIX_TABLE (PREFIX_VEX_0F3A48) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A49) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A4A) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A4B) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A4C) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 50 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 58 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F3A5C) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A5D) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A5E) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A5F) },
    /* 60 */
    { PREFIX_TABLE (PREFIX_VEX_0F3A60) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A61) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A62) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A63) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 68 */
    { PREFIX_TABLE (PREFIX_VEX_0F3A68) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A69) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A6A) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A6B) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A6C) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A6D) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A6E) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A6F) },
    /* 70 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 78 */
    { PREFIX_TABLE (PREFIX_VEX_0F3A78) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A79) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A7A) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A7B) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A7C) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A7D) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A7E) },
    { PREFIX_TABLE (PREFIX_VEX_0F3A7F) },
    /* 80 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 88 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 90 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 98 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F3ADF) },
    /* e0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f0 */
    { PREFIX_TABLE (PREFIX_VEX_0F3AF0) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
  },
};

static const struct dis386 vex_len_table[][2] = {
  /* VEX_LEN_0F10_P_1 */
  {
    { VEX_W_TABLE (VEX_W_0F10_P_1) },
    { VEX_W_TABLE (VEX_W_0F10_P_1) },
  },

  /* VEX_LEN_0F10_P_3 */
  {
    { VEX_W_TABLE (VEX_W_0F10_P_3) },
    { VEX_W_TABLE (VEX_W_0F10_P_3) },
  },

  /* VEX_LEN_0F11_P_1 */
  {
    { VEX_W_TABLE (VEX_W_0F11_P_1) },
    { VEX_W_TABLE (VEX_W_0F11_P_1) },
  },

  /* VEX_LEN_0F11_P_3 */
  {
    { VEX_W_TABLE (VEX_W_0F11_P_3) },
    { VEX_W_TABLE (VEX_W_0F11_P_3) },
  },

  /* VEX_LEN_0F12_P_0_M_0 */
  {
    { VEX_W_TABLE (VEX_W_0F12_P_0_M_0) },
  },

  /* VEX_LEN_0F12_P_0_M_1 */
  {
    { VEX_W_TABLE (VEX_W_0F12_P_0_M_1) },
  },

  /* VEX_LEN_0F12_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F12_P_2) },
  },

  /* VEX_LEN_0F13_M_0 */
  {
    { VEX_W_TABLE (VEX_W_0F13_M_0) },
  },

  /* VEX_LEN_0F16_P_0_M_0 */
  {
    { VEX_W_TABLE (VEX_W_0F16_P_0_M_0) },
  },

  /* VEX_LEN_0F16_P_0_M_1 */
  {
    { VEX_W_TABLE (VEX_W_0F16_P_0_M_1) },
  },

  /* VEX_LEN_0F16_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F16_P_2) },
  },

  /* VEX_LEN_0F17_M_0 */
  {
    { VEX_W_TABLE (VEX_W_0F17_M_0) },
  },

  /* VEX_LEN_0F2A_P_1 */
  {
    { "vcvtsi2ss%LQ",	{ XMScalar, VexScalar, Ev } },
    { "vcvtsi2ss%LQ",	{ XMScalar, VexScalar, Ev } },
  },

  /* VEX_LEN_0F2A_P_3 */
  {
    { "vcvtsi2sd%LQ",	{ XMScalar, VexScalar, Ev } },
    { "vcvtsi2sd%LQ",	{ XMScalar, VexScalar, Ev } },
  },

  /* VEX_LEN_0F2C_P_1 */
  {
    { "vcvttss2siY",	{ Gv, EXdScalar } },
    { "vcvttss2siY",	{ Gv, EXdScalar } },
  },

  /* VEX_LEN_0F2C_P_3 */
  {
    { "vcvttsd2siY",	{ Gv, EXqScalar } },
    { "vcvttsd2siY",	{ Gv, EXqScalar } },
  },

  /* VEX_LEN_0F2D_P_1 */
  {
    { "vcvtss2siY",	{ Gv, EXdScalar } },
    { "vcvtss2siY",	{ Gv, EXdScalar } },
  },

  /* VEX_LEN_0F2D_P_3 */
  {
    { "vcvtsd2siY",	{ Gv, EXqScalar } },
    { "vcvtsd2siY",	{ Gv, EXqScalar } },
  },

  /* VEX_LEN_0F2E_P_0 */
  {
    { VEX_W_TABLE (VEX_W_0F2E_P_0) },
    { VEX_W_TABLE (VEX_W_0F2E_P_0) },
  },

  /* VEX_LEN_0F2E_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F2E_P_2) },
    { VEX_W_TABLE (VEX_W_0F2E_P_2) },
  },

  /* VEX_LEN_0F2F_P_0 */
  {
    { VEX_W_TABLE (VEX_W_0F2F_P_0) },
    { VEX_W_TABLE (VEX_W_0F2F_P_0) },
  },

  /* VEX_LEN_0F2F_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F2F_P_2) },
    { VEX_W_TABLE (VEX_W_0F2F_P_2) },
  },

  /* VEX_LEN_0F51_P_1 */
  {
    { VEX_W_TABLE (VEX_W_0F51_P_1) },
    { VEX_W_TABLE (VEX_W_0F51_P_1) },
  },

  /* VEX_LEN_0F51_P_3 */
  {
    { VEX_W_TABLE (VEX_W_0F51_P_3) },
    { VEX_W_TABLE (VEX_W_0F51_P_3) },
  },

  /* VEX_LEN_0F52_P_1 */
  {
    { VEX_W_TABLE (VEX_W_0F52_P_1) },
    { VEX_W_TABLE (VEX_W_0F52_P_1) },
  },

  /* VEX_LEN_0F53_P_1 */
  {
    { VEX_W_TABLE (VEX_W_0F53_P_1) },
    { VEX_W_TABLE (VEX_W_0F53_P_1) },
  },

  /* VEX_LEN_0F58_P_1 */
  {
    { VEX_W_TABLE (VEX_W_0F58_P_1) },
    { VEX_W_TABLE (VEX_W_0F58_P_1) },
  },

  /* VEX_LEN_0F58_P_3 */
  {
    { VEX_W_TABLE (VEX_W_0F58_P_3) },
    { VEX_W_TABLE (VEX_W_0F58_P_3) },
  },

  /* VEX_LEN_0F59_P_1 */
  {
    { VEX_W_TABLE (VEX_W_0F59_P_1) },
    { VEX_W_TABLE (VEX_W_0F59_P_1) },
  },

  /* VEX_LEN_0F59_P_3 */
  {
    { VEX_W_TABLE (VEX_W_0F59_P_3) },
    { VEX_W_TABLE (VEX_W_0F59_P_3) },
  },

  /* VEX_LEN_0F5A_P_1 */
  {
    { VEX_W_TABLE (VEX_W_0F5A_P_1) },
    { VEX_W_TABLE (VEX_W_0F5A_P_1) },
  },

  /* VEX_LEN_0F5A_P_3 */
  {
    { VEX_W_TABLE (VEX_W_0F5A_P_3) },
    { VEX_W_TABLE (VEX_W_0F5A_P_3) },
  },

  /* VEX_LEN_0F5C_P_1 */
  {
    { VEX_W_TABLE (VEX_W_0F5C_P_1) },
    { VEX_W_TABLE (VEX_W_0F5C_P_1) },
  },

  /* VEX_LEN_0F5C_P_3 */
  {
    { VEX_W_TABLE (VEX_W_0F5C_P_3) },
    { VEX_W_TABLE (VEX_W_0F5C_P_3) },
  },

  /* VEX_LEN_0F5D_P_1 */
  {
    { VEX_W_TABLE (VEX_W_0F5D_P_1) },
    { VEX_W_TABLE (VEX_W_0F5D_P_1) },
  },

  /* VEX_LEN_0F5D_P_3 */
  {
    { VEX_W_TABLE (VEX_W_0F5D_P_3) },
    { VEX_W_TABLE (VEX_W_0F5D_P_3) },
  },

  /* VEX_LEN_0F5E_P_1 */
  {
    { VEX_W_TABLE (VEX_W_0F5E_P_1) },
    { VEX_W_TABLE (VEX_W_0F5E_P_1) },
  },

  /* VEX_LEN_0F5E_P_3 */
  {
    { VEX_W_TABLE (VEX_W_0F5E_P_3) },
    { VEX_W_TABLE (VEX_W_0F5E_P_3) },
  },

  /* VEX_LEN_0F5F_P_1 */
  {
    { VEX_W_TABLE (VEX_W_0F5F_P_1) },
    { VEX_W_TABLE (VEX_W_0F5F_P_1) },
  },

  /* VEX_LEN_0F5F_P_3 */
  {
    { VEX_W_TABLE (VEX_W_0F5F_P_3) },
    { VEX_W_TABLE (VEX_W_0F5F_P_3) },
  },

  /* VEX_LEN_0F6E_P_2 */
  {
    { "vmovK",		{ XMScalar, Edq } },
    { "vmovK",		{ XMScalar, Edq } },
  },

  /* VEX_LEN_0F7E_P_1 */
  {
    { VEX_W_TABLE (VEX_W_0F7E_P_1) },
    { VEX_W_TABLE (VEX_W_0F7E_P_1) },
  },

  /* VEX_LEN_0F7E_P_2 */
  {
    { "vmovK",		{ Edq, XMScalar } },
    { "vmovK",		{ Edq, XMScalar } },
  },

  /* VEX_LEN_0FAE_R_2_M_0 */
  {
    { VEX_W_TABLE (VEX_W_0FAE_R_2_M_0) },
  },

  /* VEX_LEN_0FAE_R_3_M_0 */
  {
    { VEX_W_TABLE (VEX_W_0FAE_R_3_M_0) },
  },

  /* VEX_LEN_0FC2_P_1 */
  {
    { VEX_W_TABLE (VEX_W_0FC2_P_1) },
    { VEX_W_TABLE (VEX_W_0FC2_P_1) },
  },

  /* VEX_LEN_0FC2_P_3 */
  {
    { VEX_W_TABLE (VEX_W_0FC2_P_3) },
    { VEX_W_TABLE (VEX_W_0FC2_P_3) },
  },

  /* VEX_LEN_0FC4_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0FC4_P_2) },
  },

  /* VEX_LEN_0FC5_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0FC5_P_2) },
  },

  /* VEX_LEN_0FD6_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0FD6_P_2) },
    { VEX_W_TABLE (VEX_W_0FD6_P_2) },
  },

  /* VEX_LEN_0FF7_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0FF7_P_2) },
  },

  /* VEX_LEN_0F3816_P_2 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3816_P_2) },
  },

  /* VEX_LEN_0F3819_P_2 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3819_P_2) },
  },

  /* VEX_LEN_0F381A_P_2_M_0 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F381A_P_2_M_0) },
  },

  /* VEX_LEN_0F3836_P_2 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3836_P_2) },
  },

  /* VEX_LEN_0F3841_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F3841_P_2) },
  },

  /* VEX_LEN_0F385A_P_2_M_0 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F385A_P_2_M_0) },
  },

  /* VEX_LEN_0F38DB_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F38DB_P_2) },
  },

  /* VEX_LEN_0F38DC_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F38DC_P_2) },
  },

  /* VEX_LEN_0F38DD_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F38DD_P_2) },
  },

  /* VEX_LEN_0F38DE_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F38DE_P_2) },
  },

  /* VEX_LEN_0F38DF_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F38DF_P_2) },
  },

  /* VEX_LEN_0F38F2_P_0 */
  {
    { "andnS",		{ Gdq, VexGdq, Edq } },
  },

  /* VEX_LEN_0F38F3_R_1_P_0 */
  {
    { "blsrS",		{ VexGdq, Edq } },
  },

  /* VEX_LEN_0F38F3_R_2_P_0 */
  {
    { "blsmskS",	{ VexGdq, Edq } },
  },

  /* VEX_LEN_0F38F3_R_3_P_0 */
  {
    { "blsiS",		{ VexGdq, Edq } },
  },

  /* VEX_LEN_0F38F5_P_0 */
  {
    { "bzhiS",		{ Gdq, Edq, VexGdq } },
  },

  /* VEX_LEN_0F38F5_P_1 */
  {
    { "pextS",		{ Gdq, VexGdq, Edq } },
  },

  /* VEX_LEN_0F38F5_P_3 */
  {
    { "pdepS",		{ Gdq, VexGdq, Edq } },
  },

  /* VEX_LEN_0F38F6_P_3 */
  {
    { "mulxS",		{ Gdq, VexGdq, Edq } },
  },

  /* VEX_LEN_0F38F7_P_0 */
  {
    { "bextrS",		{ Gdq, Edq, VexGdq } },
  },

  /* VEX_LEN_0F38F7_P_1 */
  {
    { "sarxS",		{ Gdq, Edq, VexGdq } },
  },

  /* VEX_LEN_0F38F7_P_2 */
  {
    { "shlxS",		{ Gdq, Edq, VexGdq } },
  },

  /* VEX_LEN_0F38F7_P_3 */
  {
    { "shrxS",		{ Gdq, Edq, VexGdq } },
  },

  /* VEX_LEN_0F3A00_P_2 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A00_P_2) },
  },

  /* VEX_LEN_0F3A01_P_2 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A01_P_2) },
  },

  /* VEX_LEN_0F3A06_P_2 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A06_P_2) },
  },

  /* VEX_LEN_0F3A0A_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F3A0A_P_2) },
    { VEX_W_TABLE (VEX_W_0F3A0A_P_2) },
  },

  /* VEX_LEN_0F3A0B_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F3A0B_P_2) },
    { VEX_W_TABLE (VEX_W_0F3A0B_P_2) },
  },

  /* VEX_LEN_0F3A14_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F3A14_P_2) },
  },

  /* VEX_LEN_0F3A15_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F3A15_P_2) },
  },

  /* VEX_LEN_0F3A16_P_2  */
  {
    { "vpextrK",	{ Edq, XM, Ib } },
  },

  /* VEX_LEN_0F3A17_P_2 */
  {
    { "vextractps",	{ Edqd, XM, Ib } },
  },

  /* VEX_LEN_0F3A18_P_2 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A18_P_2) },
  },

  /* VEX_LEN_0F3A19_P_2 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A19_P_2) },
  },

  /* VEX_LEN_0F3A20_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F3A20_P_2) },
  },

  /* VEX_LEN_0F3A21_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F3A21_P_2) },
  },

  /* VEX_LEN_0F3A22_P_2 */
  {
    { "vpinsrK",	{ XM, Vex128, Edq, Ib } },
  },

  /* VEX_LEN_0F3A38_P_2 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A38_P_2) },
  },

  /* VEX_LEN_0F3A39_P_2 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A39_P_2) },
  },

  /* VEX_LEN_0F3A41_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F3A41_P_2) },
  },

  /* VEX_LEN_0F3A44_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F3A44_P_2) },
  },

  /* VEX_LEN_0F3A46_P_2 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A46_P_2) },
  },

  /* VEX_LEN_0F3A60_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F3A60_P_2) },
  },

  /* VEX_LEN_0F3A61_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F3A61_P_2) },
  },

  /* VEX_LEN_0F3A62_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F3A62_P_2) },
  },

  /* VEX_LEN_0F3A63_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F3A63_P_2) },
  },

  /* VEX_LEN_0F3A6A_P_2 */
  {
    { "vfmaddss",	{ XMVexW, Vex128, EXdVexW, EXdVexW, VexI4 } },
  },

  /* VEX_LEN_0F3A6B_P_2 */
  {
    { "vfmaddsd",	{ XMVexW, Vex128, EXqVexW, EXqVexW, VexI4 } },
  },

  /* VEX_LEN_0F3A6E_P_2 */
  {
    { "vfmsubss",	{ XMVexW, Vex128, EXdVexW, EXdVexW, VexI4 } },
  },

  /* VEX_LEN_0F3A6F_P_2 */
  {
    { "vfmsubsd",	{ XMVexW, Vex128, EXqVexW, EXqVexW, VexI4 } },
  },

  /* VEX_LEN_0F3A7A_P_2 */
  {
    { "vfnmaddss",	{ XMVexW, Vex128, EXdVexW, EXdVexW, VexI4 } },
  },

  /* VEX_LEN_0F3A7B_P_2 */
  {
    { "vfnmaddsd",	{ XMVexW, Vex128, EXqVexW, EXqVexW, VexI4 } },
  },

  /* VEX_LEN_0F3A7E_P_2 */
  {
    { "vfnmsubss",	{ XMVexW, Vex128, EXdVexW, EXdVexW, VexI4 } },
  },

  /* VEX_LEN_0F3A7F_P_2 */
  {
    { "vfnmsubsd",	{ XMVexW, Vex128, EXqVexW, EXqVexW, VexI4 } },
  },

  /* VEX_LEN_0F3ADF_P_2 */
  {
    { VEX_W_TABLE (VEX_W_0F3ADF_P_2) },
  },

  /* VEX_LEN_0F3AF0_P_3 */
  {
    { "rorxS",		{ Gdq, Edq, Ib } },
  },

  /* VEX_LEN_0FXOP_08_CC */
  {
     { "vpcomb",	{ XM, Vex128, EXx, Ib } },
  },

  /* VEX_LEN_0FXOP_08_CD */
  {
     { "vpcomw",	{ XM, Vex128, EXx, Ib } },
  },

  /* VEX_LEN_0FXOP_08_CE */
  {
     { "vpcomd",	{ XM, Vex128, EXx, Ib } },
  },

  /* VEX_LEN_0FXOP_08_CF */
  {
     { "vpcomq",	{ XM, Vex128, EXx, Ib } },
  },

  /* VEX_LEN_0FXOP_08_EC */
  {
     { "vpcomub",	{ XM, Vex128, EXx, Ib } },
  },

  /* VEX_LEN_0FXOP_08_ED */
  {
     { "vpcomuw",	{ XM, Vex128, EXx, Ib } },
  },

  /* VEX_LEN_0FXOP_08_EE */
  {
     { "vpcomud",	{ XM, Vex128, EXx, Ib } },
  },

  /* VEX_LEN_0FXOP_08_EF */
  {
     { "vpcomuq",	{ XM, Vex128, EXx, Ib } },
  },

  /* VEX_LEN_0FXOP_09_80 */
  {
    { "vfrczps",	{ XM, EXxmm } },
    { "vfrczps",	{ XM, EXymmq } },
  },

  /* VEX_LEN_0FXOP_09_81 */
  {
    { "vfrczpd",	{ XM, EXxmm } },
    { "vfrczpd",	{ XM, EXymmq } },
  },
};

static const struct dis386 vex_w_table[][2] = {
  {
    /* VEX_W_0F10_P_0 */
    { "vmovups",	{ XM, EXx } },
  },
  {
    /* VEX_W_0F10_P_1 */
    { "vmovss",		{ XMVexScalar, VexScalar, EXdScalar } },
  },
  {
    /* VEX_W_0F10_P_2 */
    { "vmovupd",	{ XM, EXx } },
  },
  {
    /* VEX_W_0F10_P_3 */
    { "vmovsd",		{ XMVexScalar, VexScalar, EXqScalar } },
  },
  {
    /* VEX_W_0F11_P_0 */
    { "vmovups",	{ EXxS, XM } },
  },
  {
    /* VEX_W_0F11_P_1 */
    { "vmovss",		{ EXdVexScalarS, VexScalar, XMScalar } },
  },
  {
    /* VEX_W_0F11_P_2 */
    { "vmovupd",	{ EXxS, XM } },
  },
  {
    /* VEX_W_0F11_P_3 */
    { "vmovsd",		{ EXqVexScalarS, VexScalar, XMScalar } },
  },
  {
    /* VEX_W_0F12_P_0_M_0 */
    { "vmovlps",	{ XM, Vex128, EXq } },
  },
  {
    /* VEX_W_0F12_P_0_M_1 */
    { "vmovhlps",	{ XM, Vex128, EXq } },
  },
  {
    /* VEX_W_0F12_P_1 */
    { "vmovsldup",	{ XM, EXx } },
  },
  {
    /* VEX_W_0F12_P_2 */
    { "vmovlpd",	{ XM, Vex128, EXq } },
  },
  {
    /* VEX_W_0F12_P_3 */
    { "vmovddup",	{ XM, EXymmq } },
  },
  {
    /* VEX_W_0F13_M_0 */
    { "vmovlpX",	{ EXq, XM } },
  },
  {
    /* VEX_W_0F14 */
    { "vunpcklpX",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F15 */
    { "vunpckhpX",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F16_P_0_M_0 */
    { "vmovhps",	{ XM, Vex128, EXq } },
  },
  {
    /* VEX_W_0F16_P_0_M_1 */
    { "vmovlhps",	{ XM, Vex128, EXq } },
  },
  {
    /* VEX_W_0F16_P_1 */
    { "vmovshdup",	{ XM, EXx } },
  },
  {
    /* VEX_W_0F16_P_2 */
    { "vmovhpd",	{ XM, Vex128, EXq } },
  },
  {
    /* VEX_W_0F17_M_0 */
    { "vmovhpX",	{ EXq, XM } },
  },
  {
    /* VEX_W_0F28 */
    { "vmovapX",	{ XM, EXx } },
  },
  {
    /* VEX_W_0F29 */
    { "vmovapX",	{ EXxS, XM } },
  },
  {
    /* VEX_W_0F2B_M_0 */
    { "vmovntpX",	{ Mx, XM } },
  },
  {
    /* VEX_W_0F2E_P_0 */
    { "vucomiss",	{ XMScalar, EXdScalar } }, 
  },
  {
    /* VEX_W_0F2E_P_2 */
    { "vucomisd",	{ XMScalar, EXqScalar } }, 
  },
  {
    /* VEX_W_0F2F_P_0 */
    { "vcomiss",	{ XMScalar, EXdScalar } },
  },
  {
    /* VEX_W_0F2F_P_2 */
    { "vcomisd",	{ XMScalar, EXqScalar } },
  },
  {
    /* VEX_W_0F50_M_0 */
    { "vmovmskpX",	{ Gdq, XS } },
  },
  {
    /* VEX_W_0F51_P_0 */
    { "vsqrtps",	{ XM, EXx } },
  },
  {
    /* VEX_W_0F51_P_1 */
    { "vsqrtss",	{ XMScalar, VexScalar, EXdScalar } },
  },
  {
    /* VEX_W_0F51_P_2  */
    { "vsqrtpd",	{ XM, EXx } },
  },
  {
    /* VEX_W_0F51_P_3 */
    { "vsqrtsd",	{ XMScalar, VexScalar, EXqScalar } },
  },
  {
    /* VEX_W_0F52_P_0 */
    { "vrsqrtps",	{ XM, EXx } },
  },
  {
    /* VEX_W_0F52_P_1 */
    { "vrsqrtss",	{ XMScalar, VexScalar, EXdScalar } },
  },
  {
    /* VEX_W_0F53_P_0  */
    { "vrcpps",		{ XM, EXx } },
  },
  {
    /* VEX_W_0F53_P_1  */
    { "vrcpss",		{ XMScalar, VexScalar, EXdScalar } },
  },
  {
    /* VEX_W_0F58_P_0  */
    { "vaddps",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F58_P_1  */
    { "vaddss",		{ XMScalar, VexScalar, EXdScalar } },
  },
  {
    /* VEX_W_0F58_P_2  */
    { "vaddpd",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F58_P_3  */
    { "vaddsd",		{ XMScalar, VexScalar, EXqScalar } },
  },
  {
    /* VEX_W_0F59_P_0  */
    { "vmulps",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F59_P_1  */
    { "vmulss",		{ XMScalar, VexScalar, EXdScalar } },
  },
  {
    /* VEX_W_0F59_P_2  */
    { "vmulpd",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F59_P_3  */
    { "vmulsd",		{ XMScalar, VexScalar, EXqScalar } },
  },
  {
    /* VEX_W_0F5A_P_0  */
    { "vcvtps2pd",	{ XM, EXxmmq } },
  },
  {
    /* VEX_W_0F5A_P_1  */
    { "vcvtss2sd",	{ XMScalar, VexScalar, EXdScalar } },
  },
  {
    /* VEX_W_0F5A_P_3  */
    { "vcvtsd2ss",	{ XMScalar, VexScalar, EXqScalar } },
  },
  {
    /* VEX_W_0F5B_P_0  */
    { "vcvtdq2ps",	{ XM, EXx } },
  },
  {
    /* VEX_W_0F5B_P_1  */
    { "vcvttps2dq",	{ XM, EXx } },
  },
  {
    /* VEX_W_0F5B_P_2  */
    { "vcvtps2dq",	{ XM, EXx } },
  },
  {
    /* VEX_W_0F5C_P_0  */
    { "vsubps",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F5C_P_1  */
    { "vsubss",		{ XMScalar, VexScalar, EXdScalar } },
  },
  {
    /* VEX_W_0F5C_P_2  */
    { "vsubpd",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F5C_P_3  */
    { "vsubsd",		{ XMScalar, VexScalar, EXqScalar } },
  },
  {
    /* VEX_W_0F5D_P_0  */
    { "vminps",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F5D_P_1  */
    { "vminss",		{ XMScalar, VexScalar, EXdScalar } },
  },
  {
    /* VEX_W_0F5D_P_2  */
    { "vminpd",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F5D_P_3  */
    { "vminsd",		{ XMScalar, VexScalar, EXqScalar } },
  },
  {
    /* VEX_W_0F5E_P_0  */
    { "vdivps",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F5E_P_1  */
    { "vdivss",		{ XMScalar, VexScalar, EXdScalar } },
  },
  {
    /* VEX_W_0F5E_P_2  */
    { "vdivpd",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F5E_P_3  */
    { "vdivsd",		{ XMScalar, VexScalar, EXqScalar } },
  },
  {
    /* VEX_W_0F5F_P_0  */
    { "vmaxps",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F5F_P_1  */
    { "vmaxss",		{ XMScalar, VexScalar, EXdScalar } },
  },
  {
    /* VEX_W_0F5F_P_2  */
    { "vmaxpd",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F5F_P_3  */
    { "vmaxsd",		{ XMScalar, VexScalar, EXqScalar } },
  },
  {
    /* VEX_W_0F60_P_2  */
    { "vpunpcklbw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F61_P_2  */
    { "vpunpcklwd",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F62_P_2  */
    { "vpunpckldq",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F63_P_2  */
    { "vpacksswb",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F64_P_2  */
    { "vpcmpgtb",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F65_P_2  */
    { "vpcmpgtw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F66_P_2  */
    { "vpcmpgtd",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F67_P_2  */
    { "vpackuswb",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F68_P_2  */
    { "vpunpckhbw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F69_P_2  */
    { "vpunpckhwd",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F6A_P_2  */
    { "vpunpckhdq",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F6B_P_2  */
    { "vpackssdw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F6C_P_2  */
    { "vpunpcklqdq",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F6D_P_2  */
    { "vpunpckhqdq",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F6F_P_1  */
    { "vmovdqu",	{ XM, EXx } },
  },
  {
    /* VEX_W_0F6F_P_2  */
    { "vmovdqa",	{ XM, EXx } },
  },
  {
    /* VEX_W_0F70_P_1 */
    { "vpshufhw",	{ XM, EXx, Ib } },
  },
  {
    /* VEX_W_0F70_P_2 */
    { "vpshufd",	{ XM, EXx, Ib } },
  },
  {
    /* VEX_W_0F70_P_3 */
    { "vpshuflw",	{ XM, EXx, Ib } },
  },
  {
    /* VEX_W_0F71_R_2_P_2  */
    { "vpsrlw",		{ Vex, XS, Ib } },
  },
  {
    /* VEX_W_0F71_R_4_P_2  */
    { "vpsraw",		{ Vex, XS, Ib } },
  },
  {
    /* VEX_W_0F71_R_6_P_2  */
    { "vpsllw",		{ Vex, XS, Ib } },
  },
  {
    /* VEX_W_0F72_R_2_P_2  */
    { "vpsrld",		{ Vex, XS, Ib } },
  },
  {
    /* VEX_W_0F72_R_4_P_2  */
    { "vpsrad",		{ Vex, XS, Ib } },
  },
  {
    /* VEX_W_0F72_R_6_P_2  */
    { "vpslld",		{ Vex, XS, Ib } },
  },
  {
    /* VEX_W_0F73_R_2_P_2  */
    { "vpsrlq",		{ Vex, XS, Ib } },
  },
  {
    /* VEX_W_0F73_R_3_P_2  */
    { "vpsrldq",	{ Vex, XS, Ib } },
  },
  {
    /* VEX_W_0F73_R_6_P_2  */
    { "vpsllq",		{ Vex, XS, Ib } },
  },
  {
    /* VEX_W_0F73_R_7_P_2  */
    { "vpslldq",	{ Vex, XS, Ib } },
  },
  {
    /* VEX_W_0F74_P_2 */
    { "vpcmpeqb",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F75_P_2 */
    { "vpcmpeqw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F76_P_2 */
    { "vpcmpeqd",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F77_P_0 */
    { "",		{ VZERO } },
  },
  {
    /* VEX_W_0F7C_P_2 */
    { "vhaddpd",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F7C_P_3 */
    { "vhaddps",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F7D_P_2 */
    { "vhsubpd",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F7D_P_3 */
    { "vhsubps",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F7E_P_1 */
    { "vmovq",		{ XMScalar, EXqScalar } },
  },
  {
    /* VEX_W_0F7F_P_1 */
    { "vmovdqu",	{ EXxS, XM } },
  },
  {
    /* VEX_W_0F7F_P_2 */
    { "vmovdqa",	{ EXxS, XM } },
  },
  {
    /* VEX_W_0FAE_R_2_M_0 */
    { "vldmxcsr",	{ Md } },
  },
  {
    /* VEX_W_0FAE_R_3_M_0 */
    { "vstmxcsr",	{ Md } },
  },
  {
    /* VEX_W_0FC2_P_0 */
    { "vcmpps",		{ XM, Vex, EXx, VCMP } },
  },
  {
    /* VEX_W_0FC2_P_1 */
    { "vcmpss",		{ XMScalar, VexScalar, EXdScalar, VCMP } },
  },
  {
    /* VEX_W_0FC2_P_2 */
    { "vcmppd",		{ XM, Vex, EXx, VCMP } },
  },
  {
    /* VEX_W_0FC2_P_3 */
    { "vcmpsd",		{ XMScalar, VexScalar, EXqScalar, VCMP } },
  },
  {
    /* VEX_W_0FC4_P_2 */
    { "vpinsrw",	{ XM, Vex128, Edqw, Ib } },
  },
  {
    /* VEX_W_0FC5_P_2 */
    { "vpextrw",	{ Gdq, XS, Ib } },
  },
  {
    /* VEX_W_0FD0_P_2 */
    { "vaddsubpd",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FD0_P_3 */
    { "vaddsubps",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FD1_P_2 */
    { "vpsrlw",		{ XM, Vex, EXxmm } },
  },
  {
    /* VEX_W_0FD2_P_2 */
    { "vpsrld",		{ XM, Vex, EXxmm } },
  },
  {
    /* VEX_W_0FD3_P_2 */
    { "vpsrlq",		{ XM, Vex, EXxmm } },
  },
  {
    /* VEX_W_0FD4_P_2 */
    { "vpaddq",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FD5_P_2 */
    { "vpmullw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FD6_P_2 */
    { "vmovq",		{ EXqScalarS, XMScalar } },
  },
  {
    /* VEX_W_0FD7_P_2_M_1 */
    { "vpmovmskb",	{ Gdq, XS } },
  },
  {
    /* VEX_W_0FD8_P_2 */
    { "vpsubusb",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FD9_P_2 */
    { "vpsubusw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FDA_P_2 */
    { "vpminub",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FDB_P_2 */
    { "vpand",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FDC_P_2 */
    { "vpaddusb",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FDD_P_2 */
    { "vpaddusw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FDE_P_2 */
    { "vpmaxub",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FDF_P_2 */
    { "vpandn",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FE0_P_2  */
    { "vpavgb",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FE1_P_2  */
    { "vpsraw",		{ XM, Vex, EXxmm } },
  },
  {
    /* VEX_W_0FE2_P_2  */
    { "vpsrad",		{ XM, Vex, EXxmm } },
  },
  {
    /* VEX_W_0FE3_P_2  */
    { "vpavgw",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FE4_P_2  */
    { "vpmulhuw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FE5_P_2  */
    { "vpmulhw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FE6_P_1  */
    { "vcvtdq2pd",	{ XM, EXxmmq } },
  },
  {
    /* VEX_W_0FE6_P_2  */
    { "vcvttpd2dq%XY",	{ XMM, EXx } },
  },
  {
    /* VEX_W_0FE6_P_3  */
    { "vcvtpd2dq%XY",	{ XMM, EXx } },
  },
  {
    /* VEX_W_0FE7_P_2_M_0 */
    { "vmovntdq",	{ Mx, XM } },
  },
  {
    /* VEX_W_0FE8_P_2  */
    { "vpsubsb",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FE9_P_2  */
    { "vpsubsw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FEA_P_2  */
    { "vpminsw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FEB_P_2  */
    { "vpor",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FEC_P_2  */
    { "vpaddsb",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FED_P_2  */
    { "vpaddsw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FEE_P_2  */
    { "vpmaxsw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FEF_P_2  */
    { "vpxor",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FF0_P_3_M_0 */
    { "vlddqu",		{ XM, M } },
  },
  {
    /* VEX_W_0FF1_P_2 */
    { "vpsllw",		{ XM, Vex, EXxmm } },
  },
  {
    /* VEX_W_0FF2_P_2 */
    { "vpslld",		{ XM, Vex, EXxmm } },
  },
  {
    /* VEX_W_0FF3_P_2 */
    { "vpsllq",		{ XM, Vex, EXxmm } },
  },
  {
    /* VEX_W_0FF4_P_2 */
    { "vpmuludq",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FF5_P_2 */
    { "vpmaddwd",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FF6_P_2 */
    { "vpsadbw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FF7_P_2 */
    { "vmaskmovdqu",	{ XM, XS } },
  },
  {
    /* VEX_W_0FF8_P_2 */
    { "vpsubb",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FF9_P_2 */
    { "vpsubw",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FFA_P_2 */
    { "vpsubd",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FFB_P_2 */
    { "vpsubq",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FFC_P_2 */
    { "vpaddb",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FFD_P_2 */
    { "vpaddw",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0FFE_P_2 */
    { "vpaddd",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F3800_P_2  */
    { "vpshufb",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F3801_P_2  */
    { "vphaddw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F3802_P_2  */
    { "vphaddd",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F3803_P_2  */
    { "vphaddsw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F3804_P_2  */
    { "vpmaddubsw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F3805_P_2  */
    { "vphsubw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F3806_P_2  */
    { "vphsubd",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F3807_P_2  */
    { "vphsubsw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F3808_P_2  */
    { "vpsignb",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F3809_P_2  */
    { "vpsignw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F380A_P_2  */
    { "vpsignd",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F380B_P_2  */
    { "vpmulhrsw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F380C_P_2  */
    { "vpermilps",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F380D_P_2  */
    { "vpermilpd",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F380E_P_2  */
    { "vtestps",	{ XM, EXx } },
  },
  {
    /* VEX_W_0F380F_P_2  */
    { "vtestpd",	{ XM, EXx } },
  },
  {
    /* VEX_W_0F3816_P_2  */
    { "vpermps",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F3817_P_2 */
    { "vptest",		{ XM, EXx } },
  },
  {
    /* VEX_W_0F3818_P_2 */
    { "vbroadcastss",	{ XM, EXxmm_md } },
  },
  {
    /* VEX_W_0F3819_P_2 */
    { "vbroadcastsd",	{ XM, EXxmm_mq } },
  },
  {
    /* VEX_W_0F381A_P_2_M_0 */
    { "vbroadcastf128",	{ XM, Mxmm } },
  },
  {
    /* VEX_W_0F381C_P_2 */
    { "vpabsb",		{ XM, EXx } },
  },
  {
    /* VEX_W_0F381D_P_2 */
    { "vpabsw",		{ XM, EXx } },
  },
  {
    /* VEX_W_0F381E_P_2 */
    { "vpabsd",		{ XM, EXx } },
  },
  {
    /* VEX_W_0F3820_P_2 */
    { "vpmovsxbw",	{ XM, EXxmmq } },
  },
  {
    /* VEX_W_0F3821_P_2 */
    { "vpmovsxbd",	{ XM, EXxmmqd } },
  },
  {
    /* VEX_W_0F3822_P_2 */
    { "vpmovsxbq",	{ XM, EXxmmdw } },
  },
  {
    /* VEX_W_0F3823_P_2 */
    { "vpmovsxwd",	{ XM, EXxmmq } },
  },
  {
    /* VEX_W_0F3824_P_2 */
    { "vpmovsxwq",	{ XM, EXxmmqd } },
  },
  {
    /* VEX_W_0F3825_P_2 */
    { "vpmovsxdq",	{ XM, EXxmmq } },
  },
  {
    /* VEX_W_0F3828_P_2 */
    { "vpmuldq",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F3829_P_2 */
    { "vpcmpeqq",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F382A_P_2_M_0 */
    { "vmovntdqa",	{ XM, Mx } },
  },
  {
    /* VEX_W_0F382B_P_2 */
    { "vpackusdw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F382C_P_2_M_0 */
    { "vmaskmovps",	{ XM, Vex, Mx } },
  },
  {
    /* VEX_W_0F382D_P_2_M_0 */
    { "vmaskmovpd",	{ XM, Vex, Mx } },
  },
  {
    /* VEX_W_0F382E_P_2_M_0 */
    { "vmaskmovps",	{ Mx, Vex, XM } },
  },
  {
    /* VEX_W_0F382F_P_2_M_0 */
    { "vmaskmovpd",	{ Mx, Vex, XM } },
  },
  {
    /* VEX_W_0F3830_P_2 */
    { "vpmovzxbw",	{ XM, EXxmmq } },
  },
  {
    /* VEX_W_0F3831_P_2 */
    { "vpmovzxbd",	{ XM, EXxmmqd } },
  },
  {
    /* VEX_W_0F3832_P_2 */
    { "vpmovzxbq",	{ XM, EXxmmdw } },
  },
  {
    /* VEX_W_0F3833_P_2 */
    { "vpmovzxwd",	{ XM, EXxmmq } },
  },
  {
    /* VEX_W_0F3834_P_2 */
    { "vpmovzxwq",	{ XM, EXxmmqd } },
  },
  {
    /* VEX_W_0F3835_P_2 */
    { "vpmovzxdq",	{ XM, EXxmmq } },
  },
  {
    /* VEX_W_0F3836_P_2  */
    { "vpermd",		{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F3837_P_2 */
    { "vpcmpgtq",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F3838_P_2 */
    { "vpminsb",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F3839_P_2 */
    { "vpminsd",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F383A_P_2 */
    { "vpminuw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F383B_P_2 */
    { "vpminud",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F383C_P_2 */
    { "vpmaxsb",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F383D_P_2 */
    { "vpmaxsd",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F383E_P_2 */
    { "vpmaxuw",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F383F_P_2 */
    { "vpmaxud",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F3840_P_2 */
    { "vpmulld",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F3841_P_2 */
    { "vphminposuw",	{ XM, EXx } },
  },
  {
    /* VEX_W_0F3846_P_2 */
    { "vpsravd",	{ XM, Vex, EXx } },
  },
  {
    /* VEX_W_0F3858_P_2 */
    { "vpbroadcastd", { XM, EXxmm_md } },
  },
  {
    /* VEX_W_0F3859_P_2 */
    { "vpbroadcastq",	{ XM, EXxmm_mq } },
  },
  {
    /* VEX_W_0F385A_P_2_M_0 */
    { "vbroadcasti128", { XM, Mxmm } },
  },
  {
    /* VEX_W_0F3878_P_2 */
    { "vpbroadcastb",	{ XM, EXxmm_mb } },
  },
  {
    /* VEX_W_0F3879_P_2 */
    { "vpbroadcastw",	{ XM, EXxmm_mw } },
  },
  {
    /* VEX_W_0F38DB_P_2 */
    { "vaesimc",	{ XM, EXx } },
  },
  {
    /* VEX_W_0F38DC_P_2 */
    { "vaesenc",	{ XM, Vex128, EXx } },
  },
  {
    /* VEX_W_0F38DD_P_2 */
    { "vaesenclast",	{ XM, Vex128, EXx } },
  },
  {
    /* VEX_W_0F38DE_P_2 */
    { "vaesdec",	{ XM, Vex128, EXx } },
  },
  {
    /* VEX_W_0F38DF_P_2 */
    { "vaesdeclast",	{ XM, Vex128, EXx } },
  },
  {
    /* VEX_W_0F3A00_P_2 */
    { Bad_Opcode },
    { "vpermq",		{ XM, EXx, Ib } },
  },
  {
    /* VEX_W_0F3A01_P_2 */
    { Bad_Opcode },
    { "vpermpd",	{ XM, EXx, Ib } },
  },
  {
    /* VEX_W_0F3A02_P_2 */
    { "vpblendd",	{ XM, Vex, EXx, Ib } },
  },
  {
    /* VEX_W_0F3A04_P_2 */
    { "vpermilps",	{ XM, EXx, Ib } },
  },
  {
    /* VEX_W_0F3A05_P_2 */
    { "vpermilpd",	{ XM, EXx, Ib } },
  },
  {
    /* VEX_W_0F3A06_P_2 */
    { "vperm2f128",	{ XM, Vex256, EXx, Ib } },
  },
  {
    /* VEX_W_0F3A08_P_2 */
    { "vroundps",	{ XM, EXx, Ib } },
  },
  {
    /* VEX_W_0F3A09_P_2 */
    { "vroundpd",	{ XM, EXx, Ib } },
  },
  {
    /* VEX_W_0F3A0A_P_2 */
    { "vroundss",	{ XMScalar, VexScalar, EXdScalar, Ib } },
  },
  {
    /* VEX_W_0F3A0B_P_2 */
    { "vroundsd",	{ XMScalar, VexScalar, EXqScalar, Ib } },
  },
  {
    /* VEX_W_0F3A0C_P_2 */
    { "vblendps",	{ XM, Vex, EXx, Ib } },
  },
  {
    /* VEX_W_0F3A0D_P_2 */
    { "vblendpd",	{ XM, Vex, EXx, Ib } },
  },
  {
    /* VEX_W_0F3A0E_P_2 */
    { "vpblendw",	{ XM, Vex, EXx, Ib } },
  },
  {
    /* VEX_W_0F3A0F_P_2 */
    { "vpalignr",	{ XM, Vex, EXx, Ib } },
  },
  {
    /* VEX_W_0F3A14_P_2 */
    { "vpextrb",	{ Edqb, XM, Ib } },
  },
  {
    /* VEX_W_0F3A15_P_2 */
    { "vpextrw",	{ Edqw, XM, Ib } },
  },
  {
    /* VEX_W_0F3A18_P_2 */
    { "vinsertf128",	{ XM, Vex256, EXxmm, Ib } },
  },
  {
    /* VEX_W_0F3A19_P_2 */
    { "vextractf128",	{ EXxmm, XM, Ib } },
  },
  {
    /* VEX_W_0F3A20_P_2 */
    { "vpinsrb",	{ XM, Vex128, Edqb, Ib } },
  },
  {
    /* VEX_W_0F3A21_P_2 */
    { "vinsertps",	{ XM, Vex128, EXd, Ib } },
  },
  {
    /* VEX_W_0F3A38_P_2 */
    { "vinserti128",	{ XM, Vex256, EXxmm, Ib } },
  },
  {
    /* VEX_W_0F3A39_P_2 */
    { "vextracti128",	{ EXxmm, XM, Ib } },
  },
  {
    /* VEX_W_0F3A40_P_2 */
    { "vdpps",		{ XM, Vex, EXx, Ib } },
  },
  {
    /* VEX_W_0F3A41_P_2 */
    { "vdppd",		{ XM, Vex128, EXx, Ib } },
  },
  {
    /* VEX_W_0F3A42_P_2 */
    { "vmpsadbw",	{ XM, Vex, EXx, Ib } },
  },
  {
    /* VEX_W_0F3A44_P_2 */
    { "vpclmulqdq",	{ XM, Vex128, EXx, PCLMUL } },
  },
  {
    /* VEX_W_0F3A46_P_2 */
    { "vperm2i128",	{ XM, Vex256, EXx, Ib } },
  },
  {
    /* VEX_W_0F3A48_P_2 */
    { "vpermil2ps",	{ XMVexW, Vex, EXVexImmW, EXVexImmW, EXVexImmW } },
    { "vpermil2ps",	{ XMVexW, Vex, EXVexImmW, EXVexImmW, EXVexImmW } },
  },
  {
    /* VEX_W_0F3A49_P_2 */
    { "vpermil2pd",	{ XMVexW, Vex, EXVexImmW, EXVexImmW, EXVexImmW } },
    { "vpermil2pd",	{ XMVexW, Vex, EXVexImmW, EXVexImmW, EXVexImmW } },
  },
  {
    /* VEX_W_0F3A4A_P_2 */
    { "vblendvps",	{ XM, Vex, EXx, XMVexI4 } },
  },
  {
    /* VEX_W_0F3A4B_P_2 */
    { "vblendvpd",	{ XM, Vex, EXx, XMVexI4 } },
  },
  {
    /* VEX_W_0F3A4C_P_2 */
    { "vpblendvb",	{ XM, Vex, EXx, XMVexI4 } },
  },
  {
    /* VEX_W_0F3A60_P_2 */
    { "vpcmpestrm",	{ XM, EXx, Ib } },
  },
  {
    /* VEX_W_0F3A61_P_2 */
    { "vpcmpestri",	{ XM, EXx, Ib } },
  },
  {
    /* VEX_W_0F3A62_P_2 */
    { "vpcmpistrm",	{ XM, EXx, Ib } },
  },
  {
    /* VEX_W_0F3A63_P_2 */
    { "vpcmpistri",	{ XM, EXx, Ib } },
  },
  {
    /* VEX_W_0F3ADF_P_2 */
    { "vaeskeygenassist", { XM, EXx, Ib } },
  },
};

static const struct dis386 mod_table[][2] = {
  {
    /* MOD_8D */
    { "leaS",		{ Gv, M } },
  },
  {
    /* MOD_C6_REG_7 */
    { Bad_Opcode },
    { RM_TABLE (RM_C6_REG_7) },
  },
  {
    /* MOD_C7_REG_7 */
    { Bad_Opcode },
    { RM_TABLE (RM_C7_REG_7) },
  },
  {
    /* MOD_0F01_REG_0 */
    { X86_64_TABLE (X86_64_0F01_REG_0) },
    { RM_TABLE (RM_0F01_REG_0) },
  },
  {
    /* MOD_0F01_REG_1 */
    { X86_64_TABLE (X86_64_0F01_REG_1) },
    { RM_TABLE (RM_0F01_REG_1) },
  },
  {
    /* MOD_0F01_REG_2 */
    { X86_64_TABLE (X86_64_0F01_REG_2) },
    { RM_TABLE (RM_0F01_REG_2) },
  },
  {
    /* MOD_0F01_REG_3 */
    { X86_64_TABLE (X86_64_0F01_REG_3) },
    { RM_TABLE (RM_0F01_REG_3) },
  },
  {
    /* MOD_0F01_REG_7 */
    { "invlpg",		{ Mb } },
    { RM_TABLE (RM_0F01_REG_7) },
  },
  {
    /* MOD_0F12_PREFIX_0 */
    { "movlps",		{ XM, EXq } },
    { "movhlps",	{ XM, EXq } },
  },
  {
    /* MOD_0F13 */
    { "movlpX",		{ EXq, XM } },
  },
  {
    /* MOD_0F16_PREFIX_0 */
    { "movhps",		{ XM, EXq } },
    { "movlhps",	{ XM, EXq } },
  },
  {
    /* MOD_0F17 */
    { "movhpX",		{ EXq, XM } },
  },
  {
    /* MOD_0F18_REG_0 */
    { "prefetchnta",	{ Mb } },
  },
  {
    /* MOD_0F18_REG_1 */
    { "prefetcht0",	{ Mb } },
  },
  {
    /* MOD_0F18_REG_2 */
    { "prefetcht1",	{ Mb } },
  },
  {
    /* MOD_0F18_REG_3 */
    { "prefetcht2",	{ Mb } },
  },
  {
    /* MOD_0F20 */
    { Bad_Opcode },
    { "movZ",		{ Rm, Cm } },
  },
  {
    /* MOD_0F21 */
    { Bad_Opcode },
    { "movZ",		{ Rm, Dm } },
  },
  {
    /* MOD_0F22 */
    { Bad_Opcode },
    { "movZ",		{ Cm, Rm } },
  },
  {
    /* MOD_0F23 */
    { Bad_Opcode },
    { "movZ",		{ Dm, Rm } },
  },
  {
    /* MOD_0F24 */
    { Bad_Opcode },    
    { "movL",		{ Rd, Td } },
  },
  {
    /* MOD_0F26 */
    { Bad_Opcode },
    { "movL",		{ Td, Rd } },
  },
  {
    /* MOD_0F2B_PREFIX_0 */
    {"movntps",		{ Mx, XM } },
  },
  {
    /* MOD_0F2B_PREFIX_1 */
    {"movntss",		{ Md, XM } },
  },
  {
    /* MOD_0F2B_PREFIX_2 */
    {"movntpd",		{ Mx, XM } },
  },
  {
    /* MOD_0F2B_PREFIX_3 */
    {"movntsd",		{ Mq, XM } },
  },
  {
    /* MOD_0F51 */
    { Bad_Opcode },
    { "movmskpX",	{ Gdq, XS } },
  },
  {
    /* MOD_0F71_REG_2 */
    { Bad_Opcode },
    { "psrlw",		{ MS, Ib } },
  },
  {
    /* MOD_0F71_REG_4 */
    { Bad_Opcode },
    { "psraw",		{ MS, Ib } },
  },
  {
    /* MOD_0F71_REG_6 */
    { Bad_Opcode },
    { "psllw",		{ MS, Ib } },
  },
  {
    /* MOD_0F72_REG_2 */
    { Bad_Opcode },
    { "psrld",		{ MS, Ib } },
  },
  {
    /* MOD_0F72_REG_4 */
    { Bad_Opcode },
    { "psrad",		{ MS, Ib } },
  },
  {
    /* MOD_0F72_REG_6 */
    { Bad_Opcode },
    { "pslld",		{ MS, Ib } },
  },
  {
    /* MOD_0F73_REG_2 */
    { Bad_Opcode },
    { "psrlq",		{ MS, Ib } },
  },
  {
    /* MOD_0F73_REG_3 */
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_0F73_REG_3) },
  },
  {
    /* MOD_0F73_REG_6 */
    { Bad_Opcode },
    { "psllq",		{ MS, Ib } },
  },
  {
    /* MOD_0F73_REG_7 */
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_0F73_REG_7) },
  },
  {
    /* MOD_0FAE_REG_0 */
    { "fxsave",		{ FXSAVE } },
    { PREFIX_TABLE (PREFIX_0FAE_REG_0) },
  },
  {
    /* MOD_0FAE_REG_1 */
    { "fxrstor",	{ FXSAVE } },
    { PREFIX_TABLE (PREFIX_0FAE_REG_1) },
  },
  {
    /* MOD_0FAE_REG_2 */
    { "ldmxcsr",	{ Md } },
    { PREFIX_TABLE (PREFIX_0FAE_REG_2) },
  },
  {
    /* MOD_0FAE_REG_3 */
    { "stmxcsr",	{ Md } },
    { PREFIX_TABLE (PREFIX_0FAE_REG_3) },
  },
  {
    /* MOD_0FAE_REG_4 */
    { "xsave",		{ FXSAVE } },
  },
  {
    /* MOD_0FAE_REG_5 */
    { "xrstor",		{ FXSAVE } },
    { RM_TABLE (RM_0FAE_REG_5) },
  },
  {
    /* MOD_0FAE_REG_6 */
    { "xsaveopt",	{ FXSAVE } },
    { RM_TABLE (RM_0FAE_REG_6) },
  },
  {
    /* MOD_0FAE_REG_7 */
    { "clflush",	{ Mb } },
    { RM_TABLE (RM_0FAE_REG_7) },
  },
  {
    /* MOD_0FB2 */
    { "lssS",		{ Gv, Mp } },
  },
  {
    /* MOD_0FB4 */
    { "lfsS",		{ Gv, Mp } },
  },
  {
    /* MOD_0FB5 */
    { "lgsS",		{ Gv, Mp } },
  },
  {
    /* MOD_0FC7_REG_6 */
    { PREFIX_TABLE (PREFIX_0FC7_REG_6) },
    { "rdrand",		{ Ev } },
  },
  {
    /* MOD_0FC7_REG_7 */
    { "vmptrst",	{ Mq } },
    { "rdseed",		{ Ev } },
  },
  {
    /* MOD_0FD7 */
    { Bad_Opcode },
    { "pmovmskb",	{ Gdq, MS } },
  },
  {
    /* MOD_0FE7_PREFIX_2 */
    { "movntdq",	{ Mx, XM } },
  },
  {
    /* MOD_0FF0_PREFIX_3 */
    { "lddqu",		{ XM, M } },
  },
  {
    /* MOD_0F382A_PREFIX_2 */
    { "movntdqa",	{ XM, Mx } },
  },
  {
    /* MOD_62_32BIT */
    { "bound{S|}",	{ Gv, Ma } },
  },
  {
    /* MOD_C4_32BIT */
    { "lesS",		{ Gv, Mp } },
    { VEX_C4_TABLE (VEX_0F) },
  },
  {
    /* MOD_C5_32BIT */
    { "ldsS",		{ Gv, Mp } },
    { VEX_C5_TABLE (VEX_0F) },
  },
  {
    /* MOD_VEX_0F12_PREFIX_0 */
    { VEX_LEN_TABLE (VEX_LEN_0F12_P_0_M_0) },
    { VEX_LEN_TABLE (VEX_LEN_0F12_P_0_M_1) },
  },
  {
    /* MOD_VEX_0F13 */
    { VEX_LEN_TABLE (VEX_LEN_0F13_M_0) },
  },
  {
    /* MOD_VEX_0F16_PREFIX_0 */
    { VEX_LEN_TABLE (VEX_LEN_0F16_P_0_M_0) },
    { VEX_LEN_TABLE (VEX_LEN_0F16_P_0_M_1) },
  },
  {
    /* MOD_VEX_0F17 */
    { VEX_LEN_TABLE (VEX_LEN_0F17_M_0) },
  },
  {
    /* MOD_VEX_0F2B */
    { VEX_W_TABLE (VEX_W_0F2B_M_0) },
  },
  {
    /* MOD_VEX_0F50 */
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F50_M_0) },
  },
  {
    /* MOD_VEX_0F71_REG_2 */
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F71_REG_2) },
  },
  {
    /* MOD_VEX_0F71_REG_4 */
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F71_REG_4) },
  },
  {
    /* MOD_VEX_0F71_REG_6 */
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F71_REG_6) },
  },
  {
    /* MOD_VEX_0F72_REG_2 */
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F72_REG_2) },
  },
  {
    /* MOD_VEX_0F72_REG_4 */
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F72_REG_4) },
  },
  {
    /* MOD_VEX_0F72_REG_6 */
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F72_REG_6) },
  },
  {
    /* MOD_VEX_0F73_REG_2 */
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F73_REG_2) },
  },
  {
    /* MOD_VEX_0F73_REG_3 */
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F73_REG_3) },
  },
  {
    /* MOD_VEX_0F73_REG_6 */
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F73_REG_6) },
  },
  {
    /* MOD_VEX_0F73_REG_7 */
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F73_REG_7) },
  },
  {
    /* MOD_VEX_0FAE_REG_2 */
    { VEX_LEN_TABLE (VEX_LEN_0FAE_R_2_M_0) },
  },
  {
    /* MOD_VEX_0FAE_REG_3 */
    { VEX_LEN_TABLE (VEX_LEN_0FAE_R_3_M_0) },
  },
  {
    /* MOD_VEX_0FD7_PREFIX_2 */
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0FD7_P_2_M_1) },
  },
  {
    /* MOD_VEX_0FE7_PREFIX_2 */
    { VEX_W_TABLE (VEX_W_0FE7_P_2_M_0) },
  },
  {
    /* MOD_VEX_0FF0_PREFIX_3 */
    { VEX_W_TABLE (VEX_W_0FF0_P_3_M_0) },
  },
  {
    /* MOD_VEX_0F381A_PREFIX_2 */
    { VEX_LEN_TABLE (VEX_LEN_0F381A_P_2_M_0) },
  },
  {
    /* MOD_VEX_0F382A_PREFIX_2 */
    { VEX_W_TABLE (VEX_W_0F382A_P_2_M_0) },
  },
  {
    /* MOD_VEX_0F382C_PREFIX_2 */
    { VEX_W_TABLE (VEX_W_0F382C_P_2_M_0) },
  },
  {
    /* MOD_VEX_0F382D_PREFIX_2 */
    { VEX_W_TABLE (VEX_W_0F382D_P_2_M_0) },
  },
  {
    /* MOD_VEX_0F382E_PREFIX_2 */
    { VEX_W_TABLE (VEX_W_0F382E_P_2_M_0) },
  },
  {
    /* MOD_VEX_0F382F_PREFIX_2 */
    { VEX_W_TABLE (VEX_W_0F382F_P_2_M_0) },
  },
  {
    /* MOD_VEX_0F385A_PREFIX_2 */
    { VEX_LEN_TABLE (VEX_LEN_0F385A_P_2_M_0) },
  },
  {
    /* MOD_VEX_0F388C_PREFIX_2 */
    { "vpmaskmov%LW",	{ XM, Vex, Mx } },
  },
  {
    /* MOD_VEX_0F388E_PREFIX_2 */
    { "vpmaskmov%LW",	{ Mx, Vex, XM } },
  },
};

static const struct dis386 rm_table[][8] = {
  {
    /* RM_C6_REG_7 */
    { "xabort",		{ Skip_MODRM, Ib } },
  },
  {
    /* RM_C7_REG_7 */
    { "xbeginT",	{ Skip_MODRM, Jv } },
  },
  {
    /* RM_0F01_REG_0 */
    { Bad_Opcode },
    { "vmcall",		{ Skip_MODRM } },
    { "vmlaunch",	{ Skip_MODRM } },
    { "vmresume",	{ Skip_MODRM } },
    { "vmxoff",		{ Skip_MODRM } },
  },
  {
    /* RM_0F01_REG_1 */
    { "monitor",	{ { OP_Monitor, 0 } } },
    { "mwait",		{ { OP_Mwait, 0 } } },
  },
  {
    /* RM_0F01_REG_2 */
    { "xgetbv",		{ Skip_MODRM } },
    { "xsetbv",		{ Skip_MODRM } },
    { Bad_Opcode },
    { Bad_Opcode },
    { "vmfunc",		{ Skip_MODRM } },
    { "xend",		{ Skip_MODRM } },
    { "xtest",		{ Skip_MODRM } },
    { Bad_Opcode },
  },
  {
    /* RM_0F01_REG_3 */
    { "vmrun",		{ Skip_MODRM } },
    { "vmmcall",	{ Skip_MODRM } },
    { "vmload",		{ Skip_MODRM } },
    { "vmsave",		{ Skip_MODRM } },
    { "stgi",		{ Skip_MODRM } },
    { "clgi",		{ Skip_MODRM } },
    { "skinit",		{ Skip_MODRM } },
    { "invlpga",	{ Skip_MODRM } },
  },
  {
    /* RM_0F01_REG_7 */
    { "swapgs",		{ Skip_MODRM } },
    { "rdtscp",		{ Skip_MODRM } },
  },
  {
    /* RM_0FAE_REG_5 */
    { "lfence",		{ Skip_MODRM } },
  },
  {
    /* RM_0FAE_REG_6 */
    { "mfence",		{ Skip_MODRM } },
  },
  {
    /* RM_0FAE_REG_7 */
    { "sfence",		{ Skip_MODRM } },
  },
};

#define INTERNAL_DISASSEMBLER_ERROR _("<internal disassembler error>")

/* We use the high bit to indicate different name for the same
   prefix.  */
#define ADDR16_PREFIX	(0x67 | 0x100)
#define ADDR32_PREFIX	(0x67 | 0x200)
#define DATA16_PREFIX	(0x66 | 0x100)
#define DATA32_PREFIX	(0x66 | 0x200)
#define REP_PREFIX	(0xf3 | 0x100)
#define XACQUIRE_PREFIX	(0xf2 | 0x200)
#define XRELEASE_PREFIX	(0xf3 | 0x400)

static int
ckprefix (void)
{
  int newrex, i, length;
  rex = 0;
  rex_ignored = 0;
  prefixes = 0;
  used_prefixes = 0;
  rex_used = 0;
  last_lock_prefix = -1;
  last_repz_prefix = -1;
  last_repnz_prefix = -1;
  last_data_prefix = -1;
  last_addr_prefix = -1;
  last_rex_prefix = -1;
  last_seg_prefix = -1;
  for (i = 0; i < (int) ARRAY_SIZE (all_prefixes); i++)
    all_prefixes[i] = 0;
  i = 0;
  length = 0;
  /* The maximum instruction length is 15bytes.  */
  while (length < MAX_CODE_LENGTH - 1)
    {
      FETCH_DATA (the_info, codep + 1);
      newrex = 0;
      switch (*codep)
	{
	/* REX prefixes family.  */
	case 0x40:
	case 0x41:
	case 0x42:
	case 0x43:
	case 0x44:
	case 0x45:
	case 0x46:
	case 0x47:
	case 0x48:
	case 0x49:
	case 0x4a:
	case 0x4b:
	case 0x4c:
	case 0x4d:
	case 0x4e:
	case 0x4f:
	  if (address_mode == mode_64bit)
	    newrex = *codep;
	  else
	    return 1;
	  last_rex_prefix = i;
	  break;
	case 0xf3:
	  prefixes |= PREFIX_REPZ;
	  last_repz_prefix = i;
	  break;
	case 0xf2:
	  prefixes |= PREFIX_REPNZ;
	  last_repnz_prefix = i;
	  break;
	case 0xf0:
	  prefixes |= PREFIX_LOCK;
	  last_lock_prefix = i;
	  break;
	case 0x2e:
	  prefixes |= PREFIX_CS;
	  last_seg_prefix = i;
	  break;
	case 0x36:
	  prefixes |= PREFIX_SS;
	  last_seg_prefix = i;
	  break;
	case 0x3e:
	  prefixes |= PREFIX_DS;
	  last_seg_prefix = i;
	  break;
	case 0x26:
	  prefixes |= PREFIX_ES;
	  last_seg_prefix = i;
	  break;
	case 0x64:
	  prefixes |= PREFIX_FS;
	  last_seg_prefix = i;
	  break;
	case 0x65:
	  prefixes |= PREFIX_GS;
	  last_seg_prefix = i;
	  break;
	case 0x66:
	  prefixes |= PREFIX_DATA;
	  last_data_prefix = i;
	  break;
	case 0x67:
	  prefixes |= PREFIX_ADDR;
	  last_addr_prefix = i;
	  break;
	case FWAIT_OPCODE:
	  /* fwait is really an instruction.  If there are prefixes
	     before the fwait, they belong to the fwait, *not* to the
	     following instruction.  */
	  if (prefixes || rex)
	    {
	      prefixes |= PREFIX_FWAIT;
	      codep++;
	      return 1;
	    }
	  prefixes = PREFIX_FWAIT;
	  break;
	default:
	  return 1;
	}
      /* Rex is ignored when followed by another prefix.  */
      if (rex)
	{
	  rex_used = rex;
	  return 1;
	}
      if (*codep != FWAIT_OPCODE)
	all_prefixes[i++] = *codep;
      rex = newrex;
      codep++;
      length++;
    }
  return 0;
}

static int
seg_prefix (int pref)
{
  switch (pref)
    {
    case 0x2e:
      return PREFIX_CS;
    case 0x36:
      return PREFIX_SS;
    case 0x3e:
      return PREFIX_DS;
    case 0x26:
      return PREFIX_ES;
    case 0x64:
      return PREFIX_FS;
    case 0x65:
      return PREFIX_GS;
    default:
      return 0;
    }
}

/* Return the name of the prefix byte PREF, or NULL if PREF is not a
   prefix byte.  */

static const char *
prefix_name (int pref, int sizeflag)
{
  static const char *rexes [16] =
    {
      "rex",		/* 0x40 */
      "rex.B",		/* 0x41 */
      "rex.X",		/* 0x42 */
      "rex.XB",		/* 0x43 */
      "rex.R",		/* 0x44 */
      "rex.RB",		/* 0x45 */
      "rex.RX",		/* 0x46 */
      "rex.RXB",	/* 0x47 */
      "rex.W",		/* 0x48 */
      "rex.WB",		/* 0x49 */
      "rex.WX",		/* 0x4a */
      "rex.WXB",	/* 0x4b */
      "rex.WR",		/* 0x4c */
      "rex.WRB",	/* 0x4d */
      "rex.WRX",	/* 0x4e */
      "rex.WRXB",	/* 0x4f */
    };

  switch (pref)
    {
    /* REX prefixes family.  */
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x46:
    case 0x47:
    case 0x48:
    case 0x49:
    case 0x4a:
    case 0x4b:
    case 0x4c:
    case 0x4d:
    case 0x4e:
    case 0x4f:
      return rexes [pref - 0x40];
    case 0xf3:
      return "repz";
    case 0xf2:
      return "repnz";
    case 0xf0:
      return "lock";
    case 0x2e:
      return "cs";
    case 0x36:
      return "ss";
    case 0x3e:
      return "ds";
    case 0x26:
      return "es";
    case 0x64:
      return "fs";
    case 0x65:
      return "gs";
    case 0x66:
      return (sizeflag & DFLAG) ? "data16" : "data32";
    case 0x67:
      if (address_mode == mode_64bit)
	return (sizeflag & AFLAG) ? "addr32" : "addr64";
      else
	return (sizeflag & AFLAG) ? "addr16" : "addr32";
    case FWAIT_OPCODE:
      return "fwait";
    case ADDR16_PREFIX:
      return "addr16";
    case ADDR32_PREFIX:
      return "addr32";
    case DATA16_PREFIX:
      return "data16";
    case DATA32_PREFIX:
      return "data32";
    case REP_PREFIX:
      return "rep";
    case XACQUIRE_PREFIX:
      return "xacquire";
    case XRELEASE_PREFIX:
      return "xrelease";
    default:
      return NULL;
    }
}

static char op_out[MAX_OPERANDS][100];
static int op_ad, op_index[MAX_OPERANDS];
static int two_source_ops;
static bfd_vma op_address[MAX_OPERANDS];
static bfd_vma op_riprel[MAX_OPERANDS];
static bfd_vma start_pc;

/*
 *   On the 386's of 1988, the maximum length of an instruction is 15 bytes.
 *   (see topic "Redundant prefixes" in the "Differences from 8086"
 *   section of the "Virtual 8086 Mode" chapter.)
 * 'pc' should be the address of this instruction, it will
 *   be used to print the target address if this is a relative jump or call
 * The function returns the length of this instruction in bytes.
 */

static char intel_syntax;
static char intel_mnemonic = !SYSV386_COMPAT;
static char open_char;
static char close_char;
static char separator_char;
static char scale_char;

/* Here for backwards compatibility.  When gdb stops using
   print_insn_i386_att and print_insn_i386_intel these functions can
   disappear, and print_insn_i386 be merged into print_insn.  */
int
print_insn_i386_att (bfd_vma pc, disassemble_info *info)
{
  intel_syntax = 0;

  return print_insn (pc, info);
}

int
print_insn_i386_intel (bfd_vma pc, disassemble_info *info)
{
  intel_syntax = 1;

  return print_insn (pc, info);
}

int
print_insn_i386 (bfd_vma pc, disassemble_info *info)
{
  intel_syntax = -1;

  return print_insn (pc, info);
}

void
print_i386_disassembler_options (FILE *stream)
{
  fprintf (stream, _("\n\
The following i386/x86-64 specific disassembler options are supported for use\n\
with the -M switch (multiple options should be separated by commas):\n"));

  fprintf (stream, _("  x86-64      Disassemble in 64bit mode\n"));
  fprintf (stream, _("  i386        Disassemble in 32bit mode\n"));
  fprintf (stream, _("  i8086       Disassemble in 16bit mode\n"));
  fprintf (stream, _("  att         Display instruction in AT&T syntax\n"));
  fprintf (stream, _("  intel       Display instruction in Intel syntax\n"));
  fprintf (stream, _("  att-mnemonic\n"
		     "              Display instruction in AT&T mnemonic\n"));
  fprintf (stream, _("  intel-mnemonic\n"
		     "              Display instruction in Intel mnemonic\n"));
  fprintf (stream, _("  addr64      Assume 64bit address size\n"));
  fprintf (stream, _("  addr32      Assume 32bit address size\n"));
  fprintf (stream, _("  addr16      Assume 16bit address size\n"));
  fprintf (stream, _("  data32      Assume 32bit data size\n"));
  fprintf (stream, _("  data16      Assume 16bit data size\n"));
  fprintf (stream, _("  suffix      Always display instruction suffix in AT&T syntax\n"));
}

/* Bad opcode.  */
static const struct dis386 bad_opcode = { "(bad)", { XX } };

/* Get a pointer to struct dis386 with a valid name.  */

static const struct dis386 *
get_valid_dis386 (const struct dis386 *dp, disassemble_info *info)
{
  int vindex, vex_table_index;

  if (dp->name != NULL)
    return dp;

  switch (dp->op[0].bytemode)
    {
    case USE_REG_TABLE:
      dp = &reg_table[dp->op[1].bytemode][modrm.reg];
      break;

    case USE_MOD_TABLE:
      vindex = modrm.mod == 0x3 ? 1 : 0;
      dp = &mod_table[dp->op[1].bytemode][vindex];
      break;

    case USE_RM_TABLE:
      dp = &rm_table[dp->op[1].bytemode][modrm.rm];
      break;

    case USE_PREFIX_TABLE:
      if (need_vex)
	{
	  /* The prefix in VEX is implicit.  */
	  switch (vex.prefix)
	    {
	    case 0:
	      vindex = 0;
	      break;
	    case REPE_PREFIX_OPCODE:
	      vindex = 1;
	      break;
	    case DATA_PREFIX_OPCODE:
	      vindex = 2;
	      break;
	    case REPNE_PREFIX_OPCODE:
	      vindex = 3;
	      break;
	    default:
	      abort ();
	      break;
	    }
	}
      else 
	{
	  vindex = 0;
	  used_prefixes |= (prefixes & PREFIX_REPZ);
	  if (prefixes & PREFIX_REPZ)
	    {
	      vindex = 1;
	      all_prefixes[last_repz_prefix] = 0;
	    }
	  else
	    {
	      /* We should check PREFIX_REPNZ and PREFIX_REPZ before
		 PREFIX_DATA.  */
	      used_prefixes |= (prefixes & PREFIX_REPNZ);
	      if (prefixes & PREFIX_REPNZ)
		{
		  vindex = 3;
		  all_prefixes[last_repnz_prefix] = 0;
		}
	      else
		{
		  used_prefixes |= (prefixes & PREFIX_DATA);
		  if (prefixes & PREFIX_DATA)
		    {
		      vindex = 2;
		      all_prefixes[last_data_prefix] = 0;
		    }
		}
	    }
	}
      dp = &prefix_table[dp->op[1].bytemode][vindex];
      break;

    case USE_X86_64_TABLE:
      vindex = address_mode == mode_64bit ? 1 : 0;
      dp = &x86_64_table[dp->op[1].bytemode][vindex];
      break;

    case USE_3BYTE_TABLE:
      FETCH_DATA (info, codep + 2);
      vindex = *codep++;
      dp = &three_byte_table[dp->op[1].bytemode][vindex];
      modrm.mod = (*codep >> 6) & 3;
      modrm.reg = (*codep >> 3) & 7;
      modrm.rm = *codep & 7;
      break;

    case USE_VEX_LEN_TABLE:
      if (!need_vex)
	abort ();

      switch (vex.length)
	{
	case 128:
	  vindex = 0;
	  break;
	case 256:
	  vindex = 1;
	  break;
	default:
	  abort ();
	  break;
	}

      dp = &vex_len_table[dp->op[1].bytemode][vindex];
      break;

    case USE_XOP_8F_TABLE:
      FETCH_DATA (info, codep + 3);
      /* All bits in the REX prefix are ignored.  */
      rex_ignored = rex;
      rex = ~(*codep >> 5) & 0x7;

      /* VEX_TABLE_INDEX is the mmmmm part of the XOP byte 1 "RCB.mmmmm".  */
      switch ((*codep & 0x1f))
	{
	default:
	  dp = &bad_opcode;
	  return dp;
	case 0x8:
	  vex_table_index = XOP_08;
	  break;
	case 0x9:
	  vex_table_index = XOP_09;
	  break;
	case 0xa:
	  vex_table_index = XOP_0A;
	  break;
	}
      codep++;
      vex.w = *codep & 0x80;
      if (vex.w && address_mode == mode_64bit)
	rex |= REX_W;

      vex.register_specifier = (~(*codep >> 3)) & 0xf;
      if (address_mode != mode_64bit
	  && vex.register_specifier > 0x7)
	{
	  dp = &bad_opcode;
	  return dp;
	}

      vex.length = (*codep & 0x4) ? 256 : 128;
      switch ((*codep & 0x3))
	{
	case 0:
	  vex.prefix = 0;
	  break;
	case 1:
	  vex.prefix = DATA_PREFIX_OPCODE;
	  break;
	case 2:
	  vex.prefix = REPE_PREFIX_OPCODE;
	  break;
	case 3:
	  vex.prefix = REPNE_PREFIX_OPCODE;
	  break;
	}
      need_vex = 1;
      need_vex_reg = 1;
      codep++;
      vindex = *codep++;
      dp = &xop_table[vex_table_index][vindex];

      FETCH_DATA (info, codep + 1);
      modrm.mod = (*codep >> 6) & 3;
      modrm.reg = (*codep >> 3) & 7;
      modrm.rm = *codep & 7;
      break;

    case USE_VEX_C4_TABLE:
      FETCH_DATA (info, codep + 3);
      /* All bits in the REX prefix are ignored.  */
      rex_ignored = rex;
      rex = ~(*codep >> 5) & 0x7;
      switch ((*codep & 0x1f))
	{
	default:
	  dp = &bad_opcode;
	  return dp;
	case 0x1:
	  vex_table_index = VEX_0F;
	  break;
	case 0x2:
	  vex_table_index = VEX_0F38;
	  break;
	case 0x3:
	  vex_table_index = VEX_0F3A;
	  break;
	}
      codep++;
      vex.w = *codep & 0x80;
      if (vex.w && address_mode == mode_64bit)
	rex |= REX_W;

      vex.register_specifier = (~(*codep >> 3)) & 0xf;
      if (address_mode != mode_64bit
	  && vex.register_specifier > 0x7)
	{
	  dp = &bad_opcode;
	  return dp;
	}

      vex.length = (*codep & 0x4) ? 256 : 128;
      switch ((*codep & 0x3))
	{
	case 0:
	  vex.prefix = 0;
	  break;
	case 1:
	  vex.prefix = DATA_PREFIX_OPCODE;
	  break;
	case 2:
	  vex.prefix = REPE_PREFIX_OPCODE;
	  break;
	case 3:
	  vex.prefix = REPNE_PREFIX_OPCODE;
	  break;
	}
      need_vex = 1;
      need_vex_reg = 1;
      codep++;
      vindex = *codep++;
      dp = &vex_table[vex_table_index][vindex];
      /* There is no MODRM byte for VEX [82|77].  */
      if (vindex != 0x77 && vindex != 0x82)
	{
	  FETCH_DATA (info, codep + 1);
	  modrm.mod = (*codep >> 6) & 3;
	  modrm.reg = (*codep >> 3) & 7;
	  modrm.rm = *codep & 7;
	}
      break;

    case USE_VEX_C5_TABLE:
      FETCH_DATA (info, codep + 2);
      /* All bits in the REX prefix are ignored.  */
      rex_ignored = rex;
      rex = (*codep & 0x80) ? 0 : REX_R;

      vex.register_specifier = (~(*codep >> 3)) & 0xf;
      if (address_mode != mode_64bit
	  && vex.register_specifier > 0x7)
	{
	  dp = &bad_opcode;
	  return dp;
	}

      vex.w = 0;

      vex.length = (*codep & 0x4) ? 256 : 128;
      switch ((*codep & 0x3))
	{
	case 0:
	  vex.prefix = 0;
	  break;
	case 1:
	  vex.prefix = DATA_PREFIX_OPCODE;
	  break;
	case 2:
	  vex.prefix = REPE_PREFIX_OPCODE;
	  break;
	case 3:
	  vex.prefix = REPNE_PREFIX_OPCODE;
	  break;
	}
      need_vex = 1;
      need_vex_reg = 1;
      codep++;
      vindex = *codep++;
      dp = &vex_table[dp->op[1].bytemode][vindex];
      /* There is no MODRM byte for VEX [82|77].  */
      if (vindex != 0x77 && vindex != 0x82)
	{
	  FETCH_DATA (info, codep + 1);
	  modrm.mod = (*codep >> 6) & 3;
	  modrm.reg = (*codep >> 3) & 7;
	  modrm.rm = *codep & 7;
	}
      break;

    case USE_VEX_W_TABLE:
      if (!need_vex)
	abort ();

      dp = &vex_w_table[dp->op[1].bytemode][vex.w ? 1 : 0];
      break;

    case 0:
      dp = &bad_opcode;
      break;

    default:
      abort ();
    }

  if (dp->name != NULL)
    return dp;
  else
    return get_valid_dis386 (dp, info);
}

static void
get_sib (disassemble_info *info)
{
  /* If modrm.mod == 3, operand must be register.  */
  if (need_modrm
      && address_mode != mode_16bit
      && modrm.mod != 3
      && modrm.rm == 4)
    {
      FETCH_DATA (info, codep + 2);
      sib.index = (codep [1] >> 3) & 7;
      sib.scale = (codep [1] >> 6) & 3;
      sib.base = codep [1] & 7;
    }
}

static int
print_insn (bfd_vma pc, disassemble_info *info)
{
  const struct dis386 *dp;
  int i;
  char *op_txt[MAX_OPERANDS];
  int needcomma;
  int sizeflag;
  const char *p;
  struct dis_private priv;
  int prefix_length;
  int default_prefixes;

  priv.orig_sizeflag = AFLAG | DFLAG;
  if ((info->mach & bfd_mach_i386_i386) != 0)
    address_mode = mode_32bit;
  else if (info->mach == bfd_mach_i386_i8086)
    {
      address_mode = mode_16bit;
      priv.orig_sizeflag = 0;
    }
  else
    address_mode = mode_64bit;

  if (intel_syntax == (char) -1)
    intel_syntax = (info->mach & bfd_mach_i386_intel_syntax) != 0;

  for (p = info->disassembler_options; p != NULL; )
    {
      if (CONST_STRNEQ (p, "x86-64"))
	{
	  address_mode = mode_64bit;
	  priv.orig_sizeflag = AFLAG | DFLAG;
	}
      else if (CONST_STRNEQ (p, "i386"))
	{
	  address_mode = mode_32bit;
	  priv.orig_sizeflag = AFLAG | DFLAG;
	}
      else if (CONST_STRNEQ (p, "i8086"))
	{
	  address_mode = mode_16bit;
	  priv.orig_sizeflag = 0;
	}
      else if (CONST_STRNEQ (p, "intel"))
	{
	  intel_syntax = 1;
	  if (CONST_STRNEQ (p + 5, "-mnemonic"))
	    intel_mnemonic = 1;
	}
      else if (CONST_STRNEQ (p, "att"))
	{
	  intel_syntax = 0;
	  if (CONST_STRNEQ (p + 3, "-mnemonic"))
	    intel_mnemonic = 0;
	}
      else if (CONST_STRNEQ (p, "addr"))
	{
	  if (address_mode == mode_64bit)
	    {
	      if (p[4] == '3' && p[5] == '2')
		priv.orig_sizeflag &= ~AFLAG;
	      else if (p[4] == '6' && p[5] == '4')
		priv.orig_sizeflag |= AFLAG;
	    }
	  else
	    {
	      if (p[4] == '1' && p[5] == '6')
		priv.orig_sizeflag &= ~AFLAG;
	      else if (p[4] == '3' && p[5] == '2')
		priv.orig_sizeflag |= AFLAG;
	    }
	}
      else if (CONST_STRNEQ (p, "data"))
	{
	  if (p[4] == '1' && p[5] == '6')
	    priv.orig_sizeflag &= ~DFLAG;
	  else if (p[4] == '3' && p[5] == '2')
	    priv.orig_sizeflag |= DFLAG;
	}
      else if (CONST_STRNEQ (p, "suffix"))
	priv.orig_sizeflag |= SUFFIX_ALWAYS;

      p = strchr (p, ',');
      if (p != NULL)
	p++;
    }

  if (intel_syntax)
    {
      names64 = intel_names64;
      names32 = intel_names32;
      names16 = intel_names16;
      names8 = intel_names8;
      names8rex = intel_names8rex;
      names_seg = intel_names_seg;
      names_mm = intel_names_mm;
      names_xmm = intel_names_xmm;
      names_ymm = intel_names_ymm;
      index64 = intel_index64;
      index32 = intel_index32;
      index16 = intel_index16;
      open_char = '[';
      close_char = ']';
      separator_char = '+';
      scale_char = '*';
    }
  else
    {
      names64 = att_names64;
      names32 = att_names32;
      names16 = att_names16;
      names8 = att_names8;
      names8rex = att_names8rex;
      names_seg = att_names_seg;
      names_mm = att_names_mm;
      names_xmm = att_names_xmm;
      names_ymm = att_names_ymm;
      index64 = att_index64;
      index32 = att_index32;
      index16 = att_index16;
      open_char = '(';
      close_char =  ')';
      separator_char = ',';
      scale_char = ',';
    }

  /* The output looks better if we put 7 bytes on a line, since that
     puts most long word instructions on a single line.  Use 8 bytes
     for Intel L1OM.  */
  if ((info->mach & bfd_mach_l1om) != 0)
    info->bytes_per_line = 8;
  else
    info->bytes_per_line = 7;

  info->private_data = &priv;
  priv.max_fetched = priv.the_buffer;
  priv.insn_start = pc;

  obuf[0] = 0;
  for (i = 0; i < MAX_OPERANDS; ++i)
    {
      op_out[i][0] = 0;
      op_index[i] = -1;
    }

  the_info = info;
  start_pc = pc;
  start_codep = priv.the_buffer;
  codep = priv.the_buffer;

  if (setjmp (priv.bailout) != 0)
    {
      const char *name;

      /* Getting here means we tried for data but didn't get it.  That
	 means we have an incomplete instruction of some sort.  Just
	 print the first byte as a prefix or a .byte pseudo-op.  */
      if (codep > priv.the_buffer)
	{
	  name = prefix_name (priv.the_buffer[0], priv.orig_sizeflag);
	  if (name != NULL)
	    (*info->fprintf_func) (info->stream, "%s", name);
	  else
	    {
	      /* Just print the first byte as a .byte instruction.  */
	      (*info->fprintf_func) (info->stream, ".byte 0x%x",
				     (unsigned int) priv.the_buffer[0]);
	    }

	  return 1;
	}

      return -1;
    }

  obufp = obuf;
  sizeflag = priv.orig_sizeflag;

  if (!ckprefix () || rex_used)
    {
      /* Too many prefixes or unused REX prefixes.  */
      for (i = 0;
	   i < (int) ARRAY_SIZE (all_prefixes) && all_prefixes[i];
	   i++)
	(*info->fprintf_func) (info->stream, "%s",
			       prefix_name (all_prefixes[i], sizeflag));
      return 1;
    }

  insn_codep = codep;

  FETCH_DATA (info, codep + 1);
  two_source_ops = (*codep == 0x62) || (*codep == 0xc8);

  if (((prefixes & PREFIX_FWAIT)
       && ((*codep < 0xd8) || (*codep > 0xdf))))
    {
      (*info->fprintf_func) (info->stream, "fwait");
      return 1;
    }

  if (*codep == 0x0f)
    {
      unsigned char threebyte;
      FETCH_DATA (info, codep + 2);
      threebyte = *++codep;
      dp = &dis386_twobyte[threebyte];
      need_modrm = twobyte_has_modrm[*codep];
      codep++;
    }
  else
    {
      dp = &dis386[*codep];
      need_modrm = onebyte_has_modrm[*codep];
      codep++;
    }

  if ((prefixes & PREFIX_REPZ))
    used_prefixes |= PREFIX_REPZ;
  if ((prefixes & PREFIX_REPNZ))
    used_prefixes |= PREFIX_REPNZ;
  if ((prefixes & PREFIX_LOCK))
    used_prefixes |= PREFIX_LOCK;

  default_prefixes = 0;
  if (prefixes & PREFIX_ADDR)
    {
      sizeflag ^= AFLAG;
      if (dp->op[2].bytemode != loop_jcxz_mode || intel_syntax)
	{
	  if ((sizeflag & AFLAG) || address_mode == mode_64bit)
	    all_prefixes[last_addr_prefix] = ADDR32_PREFIX;
	  else
	    all_prefixes[last_addr_prefix] = ADDR16_PREFIX;
	  default_prefixes |= PREFIX_ADDR;
	}
    }

  if ((prefixes & PREFIX_DATA))
    {
      sizeflag ^= DFLAG;
      if (dp->op[2].bytemode == cond_jump_mode
	  && dp->op[0].bytemode == v_mode
	  && !intel_syntax)
	{
	  if (sizeflag & DFLAG)
	    all_prefixes[last_data_prefix] = DATA32_PREFIX;
	  else
	    all_prefixes[last_data_prefix] = DATA16_PREFIX;
	  default_prefixes |= PREFIX_DATA;
	}
      else if (rex & REX_W)
	{
	  /* REX_W will override PREFIX_DATA.  */
	  default_prefixes |= PREFIX_DATA;
	}
    }

  if (need_modrm)
    {
      FETCH_DATA (info, codep + 1);
      modrm.mod = (*codep >> 6) & 3;
      modrm.reg = (*codep >> 3) & 7;
      modrm.rm = *codep & 7;
    }

  need_vex = 0;
  need_vex_reg = 0;
  vex_w_done = 0;

  if (dp->name == NULL && dp->op[0].bytemode == FLOATCODE)
    {
      get_sib (info);
      dofloat (sizeflag);
    }
  else
    {
      dp = get_valid_dis386 (dp, info);
      if (dp != NULL && putop (dp->name, sizeflag) == 0)
        {
	  get_sib (info);
	  for (i = 0; i < MAX_OPERANDS; ++i)
	    {
	      obufp = op_out[i];
	      op_ad = MAX_OPERANDS - 1 - i;
	      if (dp->op[i].rtn)
		(*dp->op[i].rtn) (dp->op[i].bytemode, sizeflag);
	    }
	}
    }

  /* See if any prefixes were not used.  If so, print the first one
     separately.  If we don't do this, we'll wind up printing an
     instruction stream which does not precisely correspond to the
     bytes we are disassembling.  */
  if ((prefixes & ~(used_prefixes | default_prefixes)) != 0)
    {
      for (i = 0; i < (int) ARRAY_SIZE (all_prefixes); i++)
	if (all_prefixes[i])
	  {
	    const char *name;
	    name = prefix_name (all_prefixes[i], priv.orig_sizeflag);
	    if (name == NULL)
	      name = INTERNAL_DISASSEMBLER_ERROR;
	    (*info->fprintf_func) (info->stream, "%s", name);
	    return 1;
	  }
    }

  /* Check if the REX prefix is used.  */
  if (rex_ignored == 0 && (rex ^ rex_used) == 0)
    all_prefixes[last_rex_prefix] = 0;

  /* Check if the SEG prefix is used.  */
  if ((prefixes & (PREFIX_CS | PREFIX_SS | PREFIX_DS | PREFIX_ES
		   | PREFIX_FS | PREFIX_GS)) != 0
      && (used_prefixes
	  & seg_prefix (all_prefixes[last_seg_prefix])) != 0)
    all_prefixes[last_seg_prefix] = 0;

  /* Check if the ADDR prefix is used.  */
  if ((prefixes & PREFIX_ADDR) != 0
      && (used_prefixes & PREFIX_ADDR) != 0)
    all_prefixes[last_addr_prefix] = 0;

  /* Check if the DATA prefix is used.  */
  if ((prefixes & PREFIX_DATA) != 0
      && (used_prefixes & PREFIX_DATA) != 0)
    all_prefixes[last_data_prefix] = 0;

  prefix_length = 0;
  for (i = 0; i < (int) ARRAY_SIZE (all_prefixes); i++)
    if (all_prefixes[i])
      {
	const char *name;
	name = prefix_name (all_prefixes[i], sizeflag);
	if (name == NULL)
	  abort ();
	prefix_length += strlen (name) + 1;
	(*info->fprintf_func) (info->stream, "%s ", name);
      }

  /* Check maximum code length.  */
  if ((codep - start_codep) > MAX_CODE_LENGTH)
    {
      (*info->fprintf_func) (info->stream, "(bad)");
      return MAX_CODE_LENGTH;
    }

  obufp = mnemonicendp;
  for (i = strlen (obuf) + prefix_length; i < 6; i++)
    oappend (" ");
  oappend (" ");
  (*info->fprintf_func) (info->stream, "%s", obuf);

  /* The enter and bound instructions are printed with operands in the same
     order as the intel book; everything else is printed in reverse order.  */
  if (intel_syntax || two_source_ops)
    {
      bfd_vma riprel;

      for (i = 0; i < MAX_OPERANDS; ++i)
        op_txt[i] = op_out[i];

      for (i = 0; i < (MAX_OPERANDS >> 1); ++i)
	{
          op_ad = op_index[i];
          op_index[i] = op_index[MAX_OPERANDS - 1 - i];
          op_index[MAX_OPERANDS - 1 - i] = op_ad;
	  riprel = op_riprel[i];
	  op_riprel[i] = op_riprel [MAX_OPERANDS - 1 - i];
	  op_riprel[MAX_OPERANDS - 1 - i] = riprel;
	}
    }
  else
    {
      for (i = 0; i < MAX_OPERANDS; ++i)
        op_txt[MAX_OPERANDS - 1 - i] = op_out[i];
    }

  needcomma = 0;
  for (i = 0; i < MAX_OPERANDS; ++i)
    if (*op_txt[i])
      {
	if (needcomma)
	  (*info->fprintf_func) (info->stream, ",");
	if (op_index[i] != -1 && !op_riprel[i])
	  (*info->print_address_func) ((bfd_vma) op_address[op_index[i]], info);
	else
	  (*info->fprintf_func) (info->stream, "%s", op_txt[i]);
	needcomma = 1;
      }

  for (i = 0; i < MAX_OPERANDS; i++)
    if (op_index[i] != -1 && op_riprel[i])
      {
	(*info->fprintf_func) (info->stream, "        # ");
	(*info->print_address_func) ((bfd_vma) (start_pc + codep - start_codep
						+ op_address[op_index[i]]), info);
	break;
      }
  return codep - priv.the_buffer;
}

static const char *float_mem[] = {
  /* d8 */
  "fadd{s|}",
  "fmul{s|}",
  "fcom{s|}",
  "fcomp{s|}",
  "fsub{s|}",
  "fsubr{s|}",
  "fdiv{s|}",
  "fdivr{s|}",
  /* d9 */
  "fld{s|}",
  "(bad)",
  "fst{s|}",
  "fstp{s|}",
  "fldenvIC",
  "fldcw",
  "fNstenvIC",
  "fNstcw",
  /* da */
  "fiadd{l|}",
  "fimul{l|}",
  "ficom{l|}",
  "ficomp{l|}",
  "fisub{l|}",
  "fisubr{l|}",
  "fidiv{l|}",
  "fidivr{l|}",
  /* db */
  "fild{l|}",
  "fisttp{l|}",
  "fist{l|}",
  "fistp{l|}",
  "(bad)",
  "fld{t||t|}",
  "(bad)",
  "fstp{t||t|}",
  /* dc */
  "fadd{l|}",
  "fmul{l|}",
  "fcom{l|}",
  "fcomp{l|}",
  "fsub{l|}",
  "fsubr{l|}",
  "fdiv{l|}",
  "fdivr{l|}",
  /* dd */
  "fld{l|}",
  "fisttp{ll|}",
  "fst{l||}",
  "fstp{l|}",
  "frstorIC",
  "(bad)",
  "fNsaveIC",
  "fNstsw",
  /* de */
  "fiadd",
  "fimul",
  "ficom",
  "ficomp",
  "fisub",
  "fisubr",
  "fidiv",
  "fidivr",
  /* df */
  "fild",
  "fisttp",
  "fist",
  "fistp",
  "fbld",
  "fild{ll|}",
  "fbstp",
  "fistp{ll|}",
};

static const unsigned char float_mem_mode[] = {
  /* d8 */
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  /* d9 */
  d_mode,
  0,
  d_mode,
  d_mode,
  0,
  w_mode,
  0,
  w_mode,
  /* da */
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  /* db */
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  0,
  t_mode,
  0,
  t_mode,
  /* dc */
  q_mode,
  q_mode,
  q_mode,
  q_mode,
  q_mode,
  q_mode,
  q_mode,
  q_mode,
  /* dd */
  q_mode,
  q_mode,
  q_mode,
  q_mode,
  0,
  0,
  0,
  w_mode,
  /* de */
  w_mode,
  w_mode,
  w_mode,
  w_mode,
  w_mode,
  w_mode,
  w_mode,
  w_mode,
  /* df */
  w_mode,
  w_mode,
  w_mode,
  w_mode,
  t_mode,
  q_mode,
  t_mode,
  q_mode
};

#define ST { OP_ST, 0 }
#define STi { OP_STi, 0 }

#define FGRPd9_2 NULL, { { NULL, 0 } }
#define FGRPd9_4 NULL, { { NULL, 1 } }
#define FGRPd9_5 NULL, { { NULL, 2 } }
#define FGRPd9_6 NULL, { { NULL, 3 } }
#define FGRPd9_7 NULL, { { NULL, 4 } }
#define FGRPda_5 NULL, { { NULL, 5 } }
#define FGRPdb_4 NULL, { { NULL, 6 } }
#define FGRPde_3 NULL, { { NULL, 7 } }
#define FGRPdf_4 NULL, { { NULL, 8 } }

static const struct dis386 float_reg[][8] = {
  /* d8 */
  {
    { "fadd",	{ ST, STi } },
    { "fmul",	{ ST, STi } },
    { "fcom",	{ STi } },
    { "fcomp",	{ STi } },
    { "fsub",	{ ST, STi } },
    { "fsubr",	{ ST, STi } },
    { "fdiv",	{ ST, STi } },
    { "fdivr",	{ ST, STi } },
  },
  /* d9 */
  {
    { "fld",	{ STi } },
    { "fxch",	{ STi } },
    { FGRPd9_2 },
    { Bad_Opcode },
    { FGRPd9_4 },
    { FGRPd9_5 },
    { FGRPd9_6 },
    { FGRPd9_7 },
  },
  /* da */
  {
    { "fcmovb",	{ ST, STi } },
    { "fcmove",	{ ST, STi } },
    { "fcmovbe",{ ST, STi } },
    { "fcmovu",	{ ST, STi } },
    { Bad_Opcode },
    { FGRPda_5 },
    { Bad_Opcode },
    { Bad_Opcode },
  },
  /* db */
  {
    { "fcmovnb",{ ST, STi } },
    { "fcmovne",{ ST, STi } },
    { "fcmovnbe",{ ST, STi } },
    { "fcmovnu",{ ST, STi } },
    { FGRPdb_4 },
    { "fucomi",	{ ST, STi } },
    { "fcomi",	{ ST, STi } },
    { Bad_Opcode },
  },
  /* dc */
  {
    { "fadd",	{ STi, ST } },
    { "fmul",	{ STi, ST } },
    { Bad_Opcode },
    { Bad_Opcode },
    { "fsub!M",	{ STi, ST } },
    { "fsubM",	{ STi, ST } },
    { "fdiv!M",	{ STi, ST } },
    { "fdivM",	{ STi, ST } },
  },
  /* dd */
  {
    { "ffree",	{ STi } },
    { Bad_Opcode },
    { "fst",	{ STi } },
    { "fstp",	{ STi } },
    { "fucom",	{ STi } },
    { "fucomp",	{ STi } },
    { Bad_Opcode },
    { Bad_Opcode },
  },
  /* de */
  {
    { "faddp",	{ STi, ST } },
    { "fmulp",	{ STi, ST } },
    { Bad_Opcode },
    { FGRPde_3 },
    { "fsub!Mp", { STi, ST } },
    { "fsubMp",	{ STi, ST } },
    { "fdiv!Mp", { STi, ST } },
    { "fdivMp",	{ STi, ST } },
  },
  /* df */
  {
    { "ffreep",	{ STi } },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { FGRPdf_4 },
    { "fucomip", { ST, STi } },
    { "fcomip", { ST, STi } },
    { Bad_Opcode },
  },
};

static char *fgrps[][8] = {
  /* d9_2  0 */
  {
    "fnop","(bad)","(bad)","(bad)","(bad)","(bad)","(bad)","(bad)",
  },

  /* d9_4  1 */
  {
    "fchs","fabs","(bad)","(bad)","ftst","fxam","(bad)","(bad)",
  },

  /* d9_5  2 */
  {
    "fld1","fldl2t","fldl2e","fldpi","fldlg2","fldln2","fldz","(bad)",
  },

  /* d9_6  3 */
  {
    "f2xm1","fyl2x","fptan","fpatan","fxtract","fprem1","fdecstp","fincstp",
  },

  /* d9_7  4 */
  {
    "fprem","fyl2xp1","fsqrt","fsincos","frndint","fscale","fsin","fcos",
  },

  /* da_5  5 */
  {
    "(bad)","fucompp","(bad)","(bad)","(bad)","(bad)","(bad)","(bad)",
  },

  /* db_4  6 */
  {
    "fNeni(8087 only)","fNdisi(8087 only)","fNclex","fNinit",
    "fNsetpm(287 only)","frstpm(287 only)","(bad)","(bad)",
  },

  /* de_3  7 */
  {
    "(bad)","fcompp","(bad)","(bad)","(bad)","(bad)","(bad)","(bad)",
  },

  /* df_4  8 */
  {
    "fNstsw","(bad)","(bad)","(bad)","(bad)","(bad)","(bad)","(bad)",
  },
};

static void
swap_operand (void)
{
  mnemonicendp[0] = '.';
  mnemonicendp[1] = 's';
  mnemonicendp += 2;
}

static void
OP_Skip_MODRM (int bytemode ATTRIBUTE_UNUSED,
	       int sizeflag ATTRIBUTE_UNUSED)
{
  /* Skip mod/rm byte.  */
  MODRM_CHECK;
  codep++;
}

static void
dofloat (int sizeflag)
{
  const struct dis386 *dp;
  unsigned char floatop;

  floatop = codep[-1];

  if (modrm.mod != 3)
    {
      int fp_indx = (floatop - 0xd8) * 8 + modrm.reg;

      putop (float_mem[fp_indx], sizeflag);
      obufp = op_out[0];
      op_ad = 2;
      OP_E (float_mem_mode[fp_indx], sizeflag);
      return;
    }
  /* Skip mod/rm byte.  */
  MODRM_CHECK;
  codep++;

  dp = &float_reg[floatop - 0xd8][modrm.reg];
  if (dp->name == NULL)
    {
      putop (fgrps[dp->op[0].bytemode][modrm.rm], sizeflag);

      /* Instruction fnstsw is only one with strange arg.  */
      if (floatop == 0xdf && codep[-1] == 0xe0)
	strcpy (op_out[0], names16[0]);
    }
  else
    {
      putop (dp->name, sizeflag);

      obufp = op_out[0];
      op_ad = 2;
      if (dp->op[0].rtn)
	(*dp->op[0].rtn) (dp->op[0].bytemode, sizeflag);

      obufp = op_out[1];
      op_ad = 1;
      if (dp->op[1].rtn)
	(*dp->op[1].rtn) (dp->op[1].bytemode, sizeflag);
    }
}

static void
OP_ST (int bytemode ATTRIBUTE_UNUSED, int sizeflag ATTRIBUTE_UNUSED)
{
//  oappend ("%st" + intel_syntax);
  sprintf (scratchbuf, "%%st");
  oappend (scratchbuf + intel_syntax);
}

static void
OP_STi (int bytemode ATTRIBUTE_UNUSED, int sizeflag ATTRIBUTE_UNUSED)
{
  sprintf (scratchbuf, "%%st(%d)", modrm.rm);
  oappend (scratchbuf + intel_syntax);
}

/* Capital letters in template are macros.  */
static int
putop (const char *in_template, int sizeflag)
{
  const char *p;
  int alt = 0;
  int cond = 1;
  unsigned int l = 0, len = 1;
  char last[4];

#define SAVE_LAST(c)			\
  if (l < len && l < sizeof (last))	\
    last[l++] = c;			\
  else					\
    abort ();

  for (p = in_template; *p; p++)
    {
      switch (*p)
	{
	default:
	  *obufp++ = *p;
	  break;
	case '%':
	  len++;
	  break;
	case '!':
	  cond = 0;
	  break;
	case '{':
	  alt = 0;
	  if (intel_syntax)
	    {
	      while (*++p != '|')
		if (*p == '}' || *p == '\0')
		  abort ();
	    }
	  /* Fall through.  */
	case 'I':
	  alt = 1;
	  continue;
	case '|':
	  while (*++p != '}')
	    {
	      if (*p == '\0')
		abort ();
	    }
	  break;
	case '}':
	  break;
	case 'A':
	  if (intel_syntax)
	    break;
	  if (modrm.mod != 3 || (sizeflag & SUFFIX_ALWAYS))
	    *obufp++ = 'b';
	  break;
	case 'B':
	  if (l == 0 && len == 1)
	    {
case_B:
	      if (intel_syntax)
		break;
	      if (sizeflag & SUFFIX_ALWAYS)
		*obufp++ = 'b';
	    }
	  else
	    {
	      if (l != 1
		  || len != 2
		  || last[0] != 'L')
		{
		  SAVE_LAST (*p);
		  break;
		}

	      if (address_mode == mode_64bit
		  && !(prefixes & PREFIX_ADDR))
		{
		  *obufp++ = 'a';
		  *obufp++ = 'b';
		  *obufp++ = 's';
		}

	      goto case_B;
	    }
	  break;
	case 'C':
	  if (intel_syntax && !alt)
	    break;
	  if ((prefixes & PREFIX_DATA) || (sizeflag & SUFFIX_ALWAYS))
	    {
	      if (sizeflag & DFLAG)
		*obufp++ = intel_syntax ? 'd' : 'l';
	      else
		*obufp++ = intel_syntax ? 'w' : 's';
	      used_prefixes |= (prefixes & PREFIX_DATA);
	    }
	  break;
	case 'D':
	  if (intel_syntax || !(sizeflag & SUFFIX_ALWAYS))
	    break;
	  USED_REX (REX_W);
	  if (modrm.mod == 3)
	    {
	      if (rex & REX_W)
		*obufp++ = 'q';
	      else
		{
		  if (sizeflag & DFLAG)
		    *obufp++ = intel_syntax ? 'd' : 'l';
		  else
		    *obufp++ = 'w';
		  used_prefixes |= (prefixes & PREFIX_DATA);
		}
	    }
	  else
	    *obufp++ = 'w';
	  break;
	case 'E':		/* For jcxz/jecxz */
	  if (address_mode == mode_64bit)
	    {
	      if (sizeflag & AFLAG)
		*obufp++ = 'r';
	      else
		*obufp++ = 'e';
	    }
	  else
	    if (sizeflag & AFLAG)
	      *obufp++ = 'e';
	  used_prefixes |= (prefixes & PREFIX_ADDR);
	  break;
	case 'F':
	  if (intel_syntax)
	    break;
	  if ((prefixes & PREFIX_ADDR) || (sizeflag & SUFFIX_ALWAYS))
	    {
	      if (sizeflag & AFLAG)
		*obufp++ = address_mode == mode_64bit ? 'q' : 'l';
	      else
		*obufp++ = address_mode == mode_64bit ? 'l' : 'w';
	      used_prefixes |= (prefixes & PREFIX_ADDR);
	    }
	  break;
	case 'G':
	  if (intel_syntax || (obufp[-1] != 's' && !(sizeflag & SUFFIX_ALWAYS)))
	    break;
	  if ((rex & REX_W) || (sizeflag & DFLAG))
	    *obufp++ = 'l';
	  else
	    *obufp++ = 'w';
	  if (!(rex & REX_W))
	    used_prefixes |= (prefixes & PREFIX_DATA);
	  break;
	case 'H':
	  if (intel_syntax)
	    break;
	  if ((prefixes & (PREFIX_CS | PREFIX_DS)) == PREFIX_CS
	      || (prefixes & (PREFIX_CS | PREFIX_DS)) == PREFIX_DS)
	    {
	      used_prefixes |= prefixes & (PREFIX_CS | PREFIX_DS);
	      *obufp++ = ',';
	      *obufp++ = 'p';
	      if (prefixes & PREFIX_DS)
		*obufp++ = 't';
	      else
		*obufp++ = 'n';
	    }
	  break;
	case 'J':
	  if (intel_syntax)
	    break;
	  *obufp++ = 'l';
	  break;
	case 'K':
	  USED_REX (REX_W);
	  if (rex & REX_W)
	    *obufp++ = 'q';
	  else
	    *obufp++ = 'd';
	  break;
	case 'Z':
	  if (intel_syntax)
	    break;
	  if (address_mode == mode_64bit && (sizeflag & SUFFIX_ALWAYS))
	    {
	      *obufp++ = 'q';
	      break;
	    }
	  /* Fall through.  */
	  goto case_L;
	case 'L':
	  if (l != 0 || len != 1)
	    {
	      SAVE_LAST (*p);
	      break;
	    }
case_L:
	  if (intel_syntax)
	    break;
	  if (sizeflag & SUFFIX_ALWAYS)
	    *obufp++ = 'l';
	  break;
	case 'M':
	  if (intel_mnemonic != cond)
	    *obufp++ = 'r';
	  break;
	case 'N':
	  if ((prefixes & PREFIX_FWAIT) == 0)
	    *obufp++ = 'n';
	  else
	    used_prefixes |= PREFIX_FWAIT;
	  break;
	case 'O':
	  USED_REX (REX_W);
	  if (rex & REX_W)
	    *obufp++ = 'o';
	  else if (intel_syntax && (sizeflag & DFLAG))
	    *obufp++ = 'q';
	  else
	    *obufp++ = 'd';
	  if (!(rex & REX_W))
	    used_prefixes |= (prefixes & PREFIX_DATA);
	  break;
	case 'T':
	  if (!intel_syntax
	      && address_mode == mode_64bit
	      && (sizeflag & DFLAG))
	    {
	      *obufp++ = 'q';
	      break;
	    }
	  /* Fall through.  */
	case 'P':
	  if (intel_syntax)
	    {
	      if ((rex & REX_W) == 0
		  && (prefixes & PREFIX_DATA))
		{
		  if ((sizeflag & DFLAG) == 0)
		    *obufp++ = 'w';
		   used_prefixes |= (prefixes & PREFIX_DATA);
		}
	      break;
	    }
	  if ((prefixes & PREFIX_DATA)
	      || (rex & REX_W)
	      || (sizeflag & SUFFIX_ALWAYS))
	    {
	      USED_REX (REX_W);
	      if (rex & REX_W)
		*obufp++ = 'q';
	      else
		{
		   if (sizeflag & DFLAG)
		      *obufp++ = 'l';
		   else
		     *obufp++ = 'w';
		   used_prefixes |= (prefixes & PREFIX_DATA);
		}
	    }
	  break;
	case 'U':
	  if (intel_syntax)
	    break;
	  if (address_mode == mode_64bit && (sizeflag & DFLAG))
	    {
	      if (modrm.mod != 3 || (sizeflag & SUFFIX_ALWAYS))
		*obufp++ = 'q';
	      break;
	    }
	  /* Fall through.  */
	  goto case_Q;
	case 'Q':
	  if (l == 0 && len == 1)
	    {
case_Q:
	      if (intel_syntax && !alt)
		break;
	      USED_REX (REX_W);
	      if (modrm.mod != 3 || (sizeflag & SUFFIX_ALWAYS))
		{
		  if (rex & REX_W)
		    *obufp++ = 'q';
		  else
		    {
		      if (sizeflag & DFLAG)
			*obufp++ = intel_syntax ? 'd' : 'l';
		      else
			*obufp++ = 'w';
		      used_prefixes |= (prefixes & PREFIX_DATA);
		    }
		}
	    }
	  else
	    {
	      if (l != 1 || len != 2 || last[0] != 'L')
		{
		  SAVE_LAST (*p);
		  break;
		}
	      if (intel_syntax
		  || (modrm.mod == 3 && !(sizeflag & SUFFIX_ALWAYS)))
		break;
	      if ((rex & REX_W))
		{
		  USED_REX (REX_W);
		  *obufp++ = 'q';
		}
	      else
		*obufp++ = 'l';
	    }
	  break;
	case 'R':
	  USED_REX (REX_W);
	  if (rex & REX_W)
	    *obufp++ = 'q';
	  else if (sizeflag & DFLAG)
	    {
	      if (intel_syntax)
		  *obufp++ = 'd';
	      else
		  *obufp++ = 'l';
	    }
	  else
	    *obufp++ = 'w';
	  if (intel_syntax && !p[1]
	      && ((rex & REX_W) || (sizeflag & DFLAG)))
	    *obufp++ = 'e';
	  if (!(rex & REX_W))
	    used_prefixes |= (prefixes & PREFIX_DATA);
	  break;
	case 'V':
	  if (l == 0 && len == 1)
	    {
	      if (intel_syntax)
		break;
	      if (address_mode == mode_64bit && (sizeflag & DFLAG))
		{
		  if (sizeflag & SUFFIX_ALWAYS)
		    *obufp++ = 'q';
		  break;
		}
	    }
	  else
	    {
	      if (l != 1
		  || len != 2
		  || last[0] != 'L')
		{
		  SAVE_LAST (*p);
		  break;
		}

	      if (rex & REX_W)
		{
		  *obufp++ = 'a';
		  *obufp++ = 'b';
		  *obufp++ = 's';
		}
	    }
	  /* Fall through.  */
	  goto case_S;
	case 'S':
	  if (l == 0 && len == 1)
	    {
case_S:
	      if (intel_syntax)
		break;
	      if (sizeflag & SUFFIX_ALWAYS)
		{
		  if (rex & REX_W)
		    *obufp++ = 'q';
		  else
		    {
		      if (sizeflag & DFLAG)
			*obufp++ = 'l';
		      else
			*obufp++ = 'w';
		      used_prefixes |= (prefixes & PREFIX_DATA);
		    }
		}
	    }
	  else
	    {
	      if (l != 1
		  || len != 2
		  || last[0] != 'L')
		{
		  SAVE_LAST (*p);
		  break;
		}

	      if (address_mode == mode_64bit
		  && !(prefixes & PREFIX_ADDR))
		{
		  *obufp++ = 'a';
		  *obufp++ = 'b';
		  *obufp++ = 's';
		}

	      goto case_S;
	    }
	  break;
	case 'X':
	  if (l != 0 || len != 1)
	    {
	      SAVE_LAST (*p);
	      break;
	    }
	  if (need_vex && vex.prefix)
	    {
	      if (vex.prefix == DATA_PREFIX_OPCODE)
		*obufp++ = 'd';
	      else
		*obufp++ = 's';
	    }
	  else
	    {
	      if (prefixes & PREFIX_DATA)
		*obufp++ = 'd';
	      else
		*obufp++ = 's';
	      used_prefixes |= (prefixes & PREFIX_DATA);
	    }
	  break;
	case 'Y':
	  if (l == 0 && len == 1)
	    {
	      if (intel_syntax || !(sizeflag & SUFFIX_ALWAYS))
		break;
	      if (rex & REX_W)
		{
		  USED_REX (REX_W);
		  *obufp++ = 'q';
		}
	      break;
	    }
	  else
	    {
	      if (l != 1 || len != 2 || last[0] != 'X')
		{
		  SAVE_LAST (*p);
		  break;
		}
	      if (!need_vex)
		abort ();
	      if (intel_syntax
		  || (modrm.mod == 3 && !(sizeflag & SUFFIX_ALWAYS)))
		break;
	      switch (vex.length)
		{
		case 128:
		  *obufp++ = 'x';
		  break;
		case 256:
		  *obufp++ = 'y';
		  break;
		default:
		  abort ();
		}
	    }
	  break;
	case 'W':
	  if (l == 0 && len == 1)
	    {
	      /* operand size flag for cwtl, cbtw */
	      USED_REX (REX_W);
	      if (rex & REX_W)
		{
		  if (intel_syntax)
		    *obufp++ = 'd';
		  else
		    *obufp++ = 'l';
		}
	      else if (sizeflag & DFLAG)
		*obufp++ = 'w';
	      else
		*obufp++ = 'b';
	      if (!(rex & REX_W))
		used_prefixes |= (prefixes & PREFIX_DATA);
	    }
	  else
	    {
	      if (l != 1
		  || len != 2
		  || (last[0] != 'X'
		      && last[0] != 'L'))
		{
		  SAVE_LAST (*p);
		  break;
		}
	      if (!need_vex)
		abort ();
	      if (last[0] == 'X')
		*obufp++ = vex.w ? 'd': 's';
	      else
		*obufp++ = vex.w ? 'q': 'd';
	    }
	  break;
	}
      alt = 0;
    }
  *obufp = 0;
  mnemonicendp = obufp;
  return 0;
}

static void
oappend (const char *s)
{
  obufp = stpcpy (obufp, s);
}

static void
append_seg (void)
{
  if (prefixes & PREFIX_CS)
    {
      used_prefixes |= PREFIX_CS;
//      oappend ("%cs:" + intel_syntax);
      sprintf (scratchbuf, "%%cs:");
      oappend (scratchbuf + intel_syntax);
    }
  if (prefixes & PREFIX_DS)
    {
      used_prefixes |= PREFIX_DS;
//      oappend ("%ds:" + intel_syntax);
      sprintf (scratchbuf, "%%ds:");
      oappend (scratchbuf + intel_syntax);
    }
  if (prefixes & PREFIX_SS)
    {
      used_prefixes |= PREFIX_SS;
//      oappend ("%ss:" + intel_syntax);
      sprintf (scratchbuf, "%%ss:");
      oappend (scratchbuf + intel_syntax);
    }
  if (prefixes & PREFIX_ES)
    {
      used_prefixes |= PREFIX_ES;
//      oappend ("%es:" + intel_syntax);
      sprintf (scratchbuf, "%%es:");
      oappend (scratchbuf + intel_syntax);
    }
  if (prefixes & PREFIX_FS)
    {
      used_prefixes |= PREFIX_FS;
//      oappend ("%fs:" + intel_syntax);
      sprintf (scratchbuf, "%%fs:");
      oappend (scratchbuf + intel_syntax);
    }
  if (prefixes & PREFIX_GS)
    {
      used_prefixes |= PREFIX_GS;
//      oappend ("%gs:" + intel_syntax);
      sprintf (scratchbuf, "%%gs:");
      oappend (scratchbuf + intel_syntax);
    }
}

static void
OP_indirE (int bytemode, int sizeflag)
{
  if (!intel_syntax)
    oappend ("*");
  OP_E (bytemode, sizeflag);
}

static void
print_operand_value (char *buf, int hex, bfd_vma disp)
{
  if (address_mode == mode_64bit)
    {
      if (hex)
	{
	  char tmp[30];
	  int i;
	  buf[0] = '0';
	  buf[1] = 'x';
	  sprintf_vma (tmp, disp);
	  for (i = 0; tmp[i] == '0' && tmp[i + 1]; i++);
	  strcpy (buf + 2, tmp + i);
	}
      else
	{
	  bfd_signed_vma v = disp;
	  char tmp[30];
	  int i;
	  if (v < 0)
	    {
	      *(buf++) = '-';
	      v = -disp;
	      /* Check for possible overflow on 0x8000000000000000.  */
	      if (v < 0)
		{
		  strcpy (buf, "9223372036854775808");
		  return;
		}
	    }
	  if (!v)
	    {
	      strcpy (buf, "0");
	      return;
	    }

	  i = 0;
	  tmp[29] = 0;
	  while (v)
	    {
	      tmp[28 - i] = (v % 10) + '0';
	      v /= 10;
	      i++;
	    }
	  strcpy (buf, tmp + 29 - i);
	}
    }
  else
    {
      if (hex)
	sprintf (buf, "0x%x", (unsigned int) disp);
      else
	sprintf (buf, "%d", (int) disp);
    }
}

/* Put DISP in BUF as signed hex number.  */

static void
print_displacement (char *buf, bfd_vma disp)
{
  bfd_signed_vma val = disp;
  char tmp[30];
  int i, j = 0;

  if (val < 0)
    {
      buf[j++] = '-';
      val = -disp;

      /* Check for possible overflow.  */
      if (val < 0)
	{
	  switch (address_mode)
	    {
	    case mode_64bit:
	      strcpy (buf + j, "0x8000000000000000");
	      break;
	    case mode_32bit:
	      strcpy (buf + j, "0x80000000");
	      break;
	    case mode_16bit:
	      strcpy (buf + j, "0x8000");
	      break;
	    }
	  return;
	}
    }

  buf[j++] = '0';
  buf[j++] = 'x';

  sprintf_vma (tmp, (bfd_vma) val);
  for (i = 0; tmp[i] == '0'; i++)
    continue;
  if (tmp[i] == '\0')
    i--;
  strcpy (buf + j, tmp + i);
}

static void
intel_operand_size (int bytemode, int sizeflag)
{
  switch (bytemode)
    {
    case b_mode:
    case b_swap_mode:
    case dqb_mode:
      oappend ("BYTE PTR ");
      break;
    case w_mode:
    case dqw_mode:
      oappend ("WORD PTR ");
      break;
    case stack_v_mode:
      if (address_mode == mode_64bit && (sizeflag & DFLAG))
	{
	  oappend ("QWORD PTR ");
	  break;
	}
      /* FALLTHRU */
    case v_mode:
    case v_swap_mode:
    case dq_mode:
      USED_REX (REX_W);
      if (rex & REX_W)
	oappend ("QWORD PTR ");
      else
	{
	  if ((sizeflag & DFLAG) || bytemode == dq_mode)
	    oappend ("DWORD PTR ");
	  else
	    oappend ("WORD PTR ");
	  used_prefixes |= (prefixes & PREFIX_DATA);
	}
      break;
    case z_mode:
      if ((rex & REX_W) || (sizeflag & DFLAG))
	*obufp++ = 'D';
      oappend ("WORD PTR ");
      if (!(rex & REX_W))
	used_prefixes |= (prefixes & PREFIX_DATA);
      break;
    case a_mode:
      if (sizeflag & DFLAG)
	oappend ("QWORD PTR ");
      else
	oappend ("DWORD PTR ");
      used_prefixes |= (prefixes & PREFIX_DATA);
      break;
    case d_mode:
    case d_scalar_mode:
    case d_scalar_swap_mode:
    case d_swap_mode:
    case dqd_mode:
      oappend ("DWORD PTR ");
      break;
    case q_mode:
    case q_scalar_mode:
    case q_scalar_swap_mode:
    case q_swap_mode:
      oappend ("QWORD PTR ");
      break;
    case m_mode:
      if (address_mode == mode_64bit)
	oappend ("QWORD PTR ");
      else
	oappend ("DWORD PTR ");
      break;
    case f_mode:
      if (sizeflag & DFLAG)
	oappend ("FWORD PTR ");
      else
	oappend ("DWORD PTR ");
      used_prefixes |= (prefixes & PREFIX_DATA);
      break;
    case t_mode:
      oappend ("TBYTE PTR ");
      break;
    case x_mode:
    case x_swap_mode:
      if (need_vex)
	{
	  switch (vex.length)
	    {
	    case 128:
	      oappend ("XMMWORD PTR ");
	      break;
	    case 256:
	      oappend ("YMMWORD PTR ");
	      break;
	    default:
	      abort ();
	    }
	}
      else
	oappend ("XMMWORD PTR ");
      break;
    case xmm_mode:
      oappend ("XMMWORD PTR ");
      break;
    case xmmq_mode:
      if (!need_vex)
	abort ();

      switch (vex.length)
	{
	case 128:
	  oappend ("QWORD PTR ");
	  break;
	case 256:
	  oappend ("XMMWORD PTR ");
	  break;
	default:
	  abort ();
	}
      break;
    case xmm_mb_mode:
      if (!need_vex)
	abort ();

      switch (vex.length)
	{
	case 128:
	case 256:
	  oappend ("BYTE PTR ");
	  break;
	default:
	  abort ();
	}
      break;
    case xmm_mw_mode:
      if (!need_vex)
	abort ();

      switch (vex.length)
	{
	case 128:
	case 256:
	  oappend ("WORD PTR ");
	  break;
	default:
	  abort ();
	}
      break;
    case xmm_md_mode:
      if (!need_vex)
	abort ();

      switch (vex.length)
	{
	case 128:
	case 256:
	  oappend ("DWORD PTR ");
	  break;
	default:
	  abort ();
	}
      break;
    case xmm_mq_mode:
      if (!need_vex)
	abort ();

      switch (vex.length)
	{
	case 128:
	case 256:
	  oappend ("QWORD PTR ");
	  break;
	default:
	  abort ();
	}
      break;
    case xmmdw_mode:
      if (!need_vex)
	abort ();

      switch (vex.length)
	{
	case 128:
	  oappend ("WORD PTR ");
	  break;
	case 256:
	  oappend ("DWORD PTR ");
	  break;
	default:
	  abort ();
	}
      break;
    case xmmqd_mode:
      if (!need_vex)
	abort ();

      switch (vex.length)
	{
	case 128:
	  oappend ("DWORD PTR ");
	  break;
	case 256:
	  oappend ("QWORD PTR ");
	  break;
	default:
	  abort ();
	}
      break;
    case ymmq_mode:
      if (!need_vex)
	abort ();

      switch (vex.length)
	{
	case 128:
	  oappend ("QWORD PTR ");
	  break;
	case 256:
	  oappend ("YMMWORD PTR ");
	  break;
	default:
	  abort ();
	}
      break;
    case ymmxmm_mode:
      if (!need_vex)
	abort ();

      switch (vex.length)
	{
	case 128:
	case 256:
	  oappend ("XMMWORD PTR ");
	  break;
	default:
	  abort ();
	}
      break;
    case o_mode:
      oappend ("OWORD PTR ");
      break;
    case vex_w_dq_mode:
    case vex_scalar_w_dq_mode:
    case vex_vsib_d_w_dq_mode:
    case vex_vsib_q_w_dq_mode:
      if (!need_vex)
	abort ();

      if (vex.w)
	oappend ("QWORD PTR ");
      else
	oappend ("DWORD PTR ");
      break;
    default:
      break;
    }
}

static void
OP_E_register (int bytemode, int sizeflag)
{
  int reg = modrm.rm;
  const char **names;

  USED_REX (REX_B);
  if ((rex & REX_B))
    reg += 8;

  if ((sizeflag & SUFFIX_ALWAYS)
      && (bytemode == b_swap_mode || bytemode == v_swap_mode))
    swap_operand ();

  switch (bytemode)
    {
    case b_mode:
    case b_swap_mode:
      USED_REX (0);
      if (rex)
	names = names8rex;
      else
	names = names8;
      break;
    case w_mode:
      names = names16;
      break;
    case d_mode:
      names = names32;
      break;
    case q_mode:
      names = names64;
      break;
    case m_mode:
      names = address_mode == mode_64bit ? names64 : names32;
      break;
    case stack_v_mode:
      if (address_mode == mode_64bit && (sizeflag & DFLAG))
	{
	  names = names64;
	  break;
	}
      bytemode = v_mode;
      /* FALLTHRU */
    case v_mode:
    case v_swap_mode:
    case dq_mode:
    case dqb_mode:
    case dqd_mode:
    case dqw_mode:
      USED_REX (REX_W);
      if (rex & REX_W)
	names = names64;
      else
	{
	  if ((sizeflag & DFLAG) 
	      || (bytemode != v_mode
		  && bytemode != v_swap_mode))
	    names = names32;
	  else
	    names = names16;
	  used_prefixes |= (prefixes & PREFIX_DATA);
	}
      break;
    case 0:
      return;
    default:
      oappend (INTERNAL_DISASSEMBLER_ERROR);
      return;
    }
  oappend (names[reg]);
}

static void
OP_E_memory (int bytemode, int sizeflag)
{
  bfd_vma disp = 0;
  int add = (rex & REX_B) ? 8 : 0;
  int riprel = 0;

  USED_REX (REX_B);
  if (intel_syntax)
    intel_operand_size (bytemode, sizeflag);
  append_seg ();

  if ((sizeflag & AFLAG) || address_mode == mode_64bit)
    {
      /* 32/64 bit address mode */
      int havedisp;
      int havesib;
      int havebase;
      int haveindex;
      int needindex;
      int base, rbase;
      int vindex = 0;
      int scale = 0;
      const char **indexes64 = names64;
      const char **indexes32 = names32;

      havesib = 0;
      havebase = 1;
      haveindex = 0;
      base = modrm.rm;

      if (base == 4)
	{
	  havesib = 1;
	  vindex = sib.index;
	  USED_REX (REX_X);
	  if (rex & REX_X)
	    vindex += 8;
	  switch (bytemode)
	    {
	    case vex_vsib_d_w_dq_mode:
	    case vex_vsib_q_w_dq_mode:
	      if (!need_vex)
		abort ();

	      haveindex = 1;
	      switch (vex.length)
		{
		case 128:
		  indexes64 = indexes32 = names_xmm; 
		  break;
		case 256:
		  if (!vex.w || bytemode == vex_vsib_q_w_dq_mode)
		    indexes64 = indexes32 = names_ymm; 
		  else
		    indexes64 = indexes32 = names_xmm; 
		  break;
		default:
		  abort ();
		}
	      break;
	    default:
	      haveindex = vindex != 4;
	      break;
	    }
	  scale = sib.scale;
	  base = sib.base;
	  codep++;
	}
      rbase = base + add;

      switch (modrm.mod)
	{
	case 0:
	  if (base == 5)
	    {
	      havebase = 0;
	      if (address_mode == mode_64bit && !havesib)
		riprel = 1;
	      disp = get32s ();
	    }
	  break;
	case 1:
	  FETCH_DATA (the_info, codep + 1);
	  disp = *codep++;
	  if ((disp & 0x80) != 0)
	    disp -= 0x100;
	  break;
	case 2:
	  disp = get32s ();
	  break;
	}

      /* In 32bit mode, we need index register to tell [offset] from
	 [eiz*1 + offset].  */
      needindex = (havesib
		   && !havebase
		   && !haveindex
		   && address_mode == mode_32bit);
      havedisp = (havebase
		  || needindex
		  || (havesib && (haveindex || scale != 0)));

      if (!intel_syntax)
	if (modrm.mod != 0 || base == 5)
	  {
	    if (havedisp || riprel)
	      print_displacement (scratchbuf, disp);
	    else
	      print_operand_value (scratchbuf, 1, disp);
	    oappend (scratchbuf);
	    if (riprel)
	      {
		set_op (disp, 1);
		oappend (sizeflag & AFLAG ? "(%rip)" : "(%eip)");
	      }
	  }

      if (havebase || haveindex || riprel)
	used_prefixes |= PREFIX_ADDR;

      if (havedisp || (intel_syntax && riprel))
	{
	  *obufp++ = open_char;
	  if (intel_syntax && riprel)
	    {
	      set_op (disp, 1);
	      oappend (sizeflag & AFLAG ? "rip" : "eip");
	    }
	  *obufp = '\0';
	  if (havebase)
	    oappend (address_mode == mode_64bit && (sizeflag & AFLAG)
		     ? names64[rbase] : names32[rbase]);
	  if (havesib)
	    {
	      /* ESP/RSP won't allow index.  If base isn't ESP/RSP,
		 print index to tell base + index from base.  */
	      if (scale != 0
		  || needindex
		  || haveindex
		  || (havebase && base != ESP_REG_NUM))
		{
		  if (!intel_syntax || havebase)
		    {
		      *obufp++ = separator_char;
		      *obufp = '\0';
		    }
		  if (haveindex)
		    oappend (address_mode == mode_64bit 
			     && (sizeflag & AFLAG)
			     ? indexes64[vindex] : indexes32[vindex]);
		  else
		    oappend (address_mode == mode_64bit 
			     && (sizeflag & AFLAG)
			     ? index64 : index32);

		  *obufp++ = scale_char;
		  *obufp = '\0';
		  sprintf (scratchbuf, "%d", 1 << scale);
		  oappend (scratchbuf);
		}
	    }
	  if (intel_syntax
	      && (disp || modrm.mod != 0 || base == 5))
	    {
	      if (!havedisp || (bfd_signed_vma) disp >= 0)
		{
		  *obufp++ = '+';
		  *obufp = '\0';
		}
	      else if (modrm.mod != 1 && disp != -disp)
		{
		  *obufp++ = '-';
		  *obufp = '\0';
		  disp = - (bfd_signed_vma) disp;
		}

	      if (havedisp)
		print_displacement (scratchbuf, disp);
	      else
		print_operand_value (scratchbuf, 1, disp);
	      oappend (scratchbuf);
	    }

	  *obufp++ = close_char;
	  *obufp = '\0';
	}
      else if (intel_syntax)
	{
	  if (modrm.mod != 0 || base == 5)
	    {
	      if (prefixes & (PREFIX_CS | PREFIX_SS | PREFIX_DS
			      | PREFIX_ES | PREFIX_FS | PREFIX_GS))
		;
	      else
		{
		  oappend (names_seg[ds_reg - es_reg]);
		  oappend (":");
		}
	      print_operand_value (scratchbuf, 1, disp);
	      oappend (scratchbuf);
	    }
	}
    }
  else
    {
      /* 16 bit address mode */
      used_prefixes |= prefixes & PREFIX_ADDR;
      switch (modrm.mod)
	{
	case 0:
	  if (modrm.rm == 6)
	    {
	      disp = get16 ();
	      if ((disp & 0x8000) != 0)
		disp -= 0x10000;
	    }
	  break;
	case 1:
	  FETCH_DATA (the_info, codep + 1);
	  disp = *codep++;
	  if ((disp & 0x80) != 0)
	    disp -= 0x100;
	  break;
	case 2:
	  disp = get16 ();
	  if ((disp & 0x8000) != 0)
	    disp -= 0x10000;
	  break;
	}

      if (!intel_syntax)
	if (modrm.mod != 0 || modrm.rm == 6)
	  {
	    print_displacement (scratchbuf, disp);
	    oappend (scratchbuf);
	  }

      if (modrm.mod != 0 || modrm.rm != 6)
	{
	  *obufp++ = open_char;
	  *obufp = '\0';
	  oappend (index16[modrm.rm]);
	  if (intel_syntax
	      && (disp || modrm.mod != 0 || modrm.rm == 6))
	    {
	      if ((bfd_signed_vma) disp >= 0)
		{
		  *obufp++ = '+';
		  *obufp = '\0';
		}
	      else if (modrm.mod != 1)
		{
		  *obufp++ = '-';
		  *obufp = '\0';
		  disp = - (bfd_signed_vma) disp;
		}

	      print_displacement (scratchbuf, disp);
	      oappend (scratchbuf);
	    }

	  *obufp++ = close_char;
	  *obufp = '\0';
	}
      else if (intel_syntax)
	{
	  if (prefixes & (PREFIX_CS | PREFIX_SS | PREFIX_DS
			  | PREFIX_ES | PREFIX_FS | PREFIX_GS))
	    ;
	  else
	    {
	      oappend (names_seg[ds_reg - es_reg]);
	      oappend (":");
	    }
	  print_operand_value (scratchbuf, 1, disp & 0xffff);
	  oappend (scratchbuf);
	}
    }
}

static void
OP_E (int bytemode, int sizeflag)
{
  /* Skip mod/rm byte.  */
  MODRM_CHECK;
  codep++;

  if (modrm.mod == 3)
    OP_E_register (bytemode, sizeflag);
  else
    OP_E_memory (bytemode, sizeflag);
}

static void
OP_G (int bytemode, int sizeflag)
{
  int add = 0;
  USED_REX (REX_R);
  if (rex & REX_R)
    add += 8;
  switch (bytemode)
    {
    case b_mode:
      USED_REX (0);
      if (rex)
	oappend (names8rex[modrm.reg + add]);
      else
	oappend (names8[modrm.reg + add]);
      break;
    case w_mode:
      oappend (names16[modrm.reg + add]);
      break;
    case d_mode:
      oappend (names32[modrm.reg + add]);
      break;
    case q_mode:
      oappend (names64[modrm.reg + add]);
      break;
    case v_mode:
    case dq_mode:
    case dqb_mode:
    case dqd_mode:
    case dqw_mode:
      USED_REX (REX_W);
      if (rex & REX_W)
	oappend (names64[modrm.reg + add]);
      else
	{
	  if ((sizeflag & DFLAG) || bytemode != v_mode)
	    oappend (names32[modrm.reg + add]);
	  else
	    oappend (names16[modrm.reg + add]);
	  used_prefixes |= (prefixes & PREFIX_DATA);
	}
      break;
    case m_mode:
      if (address_mode == mode_64bit)
	oappend (names64[modrm.reg + add]);
      else
	oappend (names32[modrm.reg + add]);
      break;
    default:
      oappend (INTERNAL_DISASSEMBLER_ERROR);
      break;
    }
}

static bfd_vma
get64 (void)
{
  bfd_vma x;
#ifdef BFD64
  unsigned int a;
  unsigned int b;

  FETCH_DATA (the_info, codep + 8);
  a = *codep++ & 0xff;
  a |= (*codep++ & 0xff) << 8;
  a |= (*codep++ & 0xff) << 16;
  a |= (*codep++ & 0xff) << 24;
  b = *codep++ & 0xff;
  b |= (*codep++ & 0xff) << 8;
  b |= (*codep++ & 0xff) << 16;
  b |= (*codep++ & 0xff) << 24;
  x = a + ((bfd_vma) b << 32);
#else
  abort ();
  x = 0;
#endif
  return x;
}

static bfd_signed_vma
get32 (void)
{
  bfd_signed_vma x = 0;

  FETCH_DATA (the_info, codep + 4);
  x = *codep++ & (bfd_signed_vma) 0xff;
  x |= (*codep++ & (bfd_signed_vma) 0xff) << 8;
  x |= (*codep++ & (bfd_signed_vma) 0xff) << 16;
  x |= (*codep++ & (bfd_signed_vma) 0xff) << 24;
  return x;
}

static bfd_signed_vma
get32s (void)
{
  bfd_signed_vma x = 0;

  FETCH_DATA (the_info, codep + 4);
  x = *codep++ & (bfd_signed_vma) 0xff;
  x |= (*codep++ & (bfd_signed_vma) 0xff) << 8;
  x |= (*codep++ & (bfd_signed_vma) 0xff) << 16;
  x |= (*codep++ & (bfd_signed_vma) 0xff) << 24;

  x = (x ^ ((bfd_signed_vma) 1 << 31)) - ((bfd_signed_vma) 1 << 31);

  return x;
}

static int
get16 (void)
{
  int x = 0;

  FETCH_DATA (the_info, codep + 2);
  x = *codep++ & 0xff;
  x |= (*codep++ & 0xff) << 8;
  return x;
}

static void
set_op (bfd_vma op, int riprel)
{
  op_index[op_ad] = op_ad;
  if (address_mode == mode_64bit)
    {
      op_address[op_ad] = op;
      op_riprel[op_ad] = riprel;
    }
  else
    {
      /* Mask to get a 32-bit address.  */
      op_address[op_ad] = op & 0xffffffff;
      op_riprel[op_ad] = riprel & 0xffffffff;
    }
}

static void
OP_REG (int code, int sizeflag)
{
  const char *s;
  int add;
  USED_REX (REX_B);
  if (rex & REX_B)
    add = 8;
  else
    add = 0;

  switch (code)
    {
    case ax_reg: case cx_reg: case dx_reg: case bx_reg:
    case sp_reg: case bp_reg: case si_reg: case di_reg:
      s = names16[code - ax_reg + add];
      break;
    case es_reg: case ss_reg: case cs_reg:
    case ds_reg: case fs_reg: case gs_reg:
      s = names_seg[code - es_reg + add];
      break;
    case al_reg: case ah_reg: case cl_reg: case ch_reg:
    case dl_reg: case dh_reg: case bl_reg: case bh_reg:
      USED_REX (0);
      if (rex)
	s = names8rex[code - al_reg + add];
      else
	s = names8[code - al_reg];
      break;
    case rAX_reg: case rCX_reg: case rDX_reg: case rBX_reg:
    case rSP_reg: case rBP_reg: case rSI_reg: case rDI_reg:
      if (address_mode == mode_64bit && (sizeflag & DFLAG))
	{
	  s = names64[code - rAX_reg + add];
	  break;
	}
      code += eAX_reg - rAX_reg;
      /* Fall through.  */
    case eAX_reg: case eCX_reg: case eDX_reg: case eBX_reg:
    case eSP_reg: case eBP_reg: case eSI_reg: case eDI_reg:
      USED_REX (REX_W);
      if (rex & REX_W)
	s = names64[code - eAX_reg + add];
      else
	{
	  if (sizeflag & DFLAG)
	    s = names32[code - eAX_reg + add];
	  else
	    s = names16[code - eAX_reg + add];
	  used_prefixes |= (prefixes & PREFIX_DATA);
	}
      break;
    default:
      s = INTERNAL_DISASSEMBLER_ERROR;
      break;
    }
  oappend (s);
}

static void
OP_IMREG (int code, int sizeflag)
{
  const char *s;

  switch (code)
    {
    case indir_dx_reg:
      if (intel_syntax)
	s = "dx";
      else
	s = "(%dx)";
      break;
    case ax_reg: case cx_reg: case dx_reg: case bx_reg:
    case sp_reg: case bp_reg: case si_reg: case di_reg:
      s = names16[code - ax_reg];
      break;
    case es_reg: case ss_reg: case cs_reg:
    case ds_reg: case fs_reg: case gs_reg:
      s = names_seg[code - es_reg];
      break;
    case al_reg: case ah_reg: case cl_reg: case ch_reg:
    case dl_reg: case dh_reg: case bl_reg: case bh_reg:
      USED_REX (0);
      if (rex)
	s = names8rex[code - al_reg];
      else
	s = names8[code - al_reg];
      break;
    case eAX_reg: case eCX_reg: case eDX_reg: case eBX_reg:
    case eSP_reg: case eBP_reg: case eSI_reg: case eDI_reg:
      USED_REX (REX_W);
      if (rex & REX_W)
	s = names64[code - eAX_reg];
      else
	{
	  if (sizeflag & DFLAG)
	    s = names32[code - eAX_reg];
	  else
	    s = names16[code - eAX_reg];
	  used_prefixes |= (prefixes & PREFIX_DATA);
	}
      break;
    case z_mode_ax_reg:
      if ((rex & REX_W) || (sizeflag & DFLAG))
	s = *names32;
      else
	s = *names16;
      if (!(rex & REX_W))
	used_prefixes |= (prefixes & PREFIX_DATA);
      break;
    default:
      s = INTERNAL_DISASSEMBLER_ERROR;
      break;
    }
  oappend (s);
}

static void
OP_I (int bytemode, int sizeflag)
{
  bfd_signed_vma op;
  bfd_signed_vma mask = -1;

  switch (bytemode)
    {
    case b_mode:
      FETCH_DATA (the_info, codep + 1);
      op = *codep++;
      mask = 0xff;
      break;
    case q_mode:
      if (address_mode == mode_64bit)
	{
	  op = get32s ();
	  break;
	}
      /* Fall through.  */
    case v_mode:
      USED_REX (REX_W);
      if (rex & REX_W)
	op = get32s ();
      else
	{
	  if (sizeflag & DFLAG)
	    {
	      op = get32 ();
	      mask = 0xffffffff;
	    }
	  else
	    {
	      op = get16 ();
	      mask = 0xfffff;
	    }
	  used_prefixes |= (prefixes & PREFIX_DATA);
	}
      break;
    case w_mode:
      mask = 0xfffff;
      op = get16 ();
      break;
    case const_1_mode:
      if (intel_syntax)
        oappend ("1");
      return;
    default:
      oappend (INTERNAL_DISASSEMBLER_ERROR);
      return;
    }

  op &= mask;
  scratchbuf[0] = '$';
  print_operand_value (scratchbuf + 1, 1, op);
  oappend (scratchbuf + intel_syntax);
  scratchbuf[0] = '\0';
}

static void
OP_I64 (int bytemode, int sizeflag)
{
  bfd_signed_vma op;
  bfd_signed_vma mask = -1;

  if (address_mode != mode_64bit)
    {
      OP_I (bytemode, sizeflag);
      return;
    }

  switch (bytemode)
    {
    case b_mode:
      FETCH_DATA (the_info, codep + 1);
      op = *codep++;
      mask = 0xff;
      break;
    case v_mode:
      USED_REX (REX_W);
      if (rex & REX_W)
	op = get64 ();
      else
	{
	  if (sizeflag & DFLAG)
	    {
	      op = get32 ();
	      mask = 0xffffffff;
	    }
	  else
	    {
	      op = get16 ();
	      mask = 0xfffff;
	    }
	  used_prefixes |= (prefixes & PREFIX_DATA);
	}
      break;
    case w_mode:
      mask = 0xfffff;
      op = get16 ();
      break;
    default:
      oappend (INTERNAL_DISASSEMBLER_ERROR);
      return;
    }

  op &= mask;
  scratchbuf[0] = '$';
  print_operand_value (scratchbuf + 1, 1, op);
  oappend (scratchbuf + intel_syntax);
  scratchbuf[0] = '\0';
}

static void
OP_sI (int bytemode, int sizeflag)
{
  bfd_signed_vma op;

  switch (bytemode)
    {
    case b_mode:
    case b_T_mode:
      FETCH_DATA (the_info, codep + 1);
      op = *codep++;
      if ((op & 0x80) != 0)
	op -= 0x100;
      if (bytemode == b_T_mode)
	{
	  if (address_mode != mode_64bit
	      || !(sizeflag & DFLAG))
	    {
	      if (sizeflag & DFLAG)
		op &= 0xffffffff;
	      else
		op &= 0xffff;
	  }
	}
      else
	{
	  if (!(rex & REX_W))
	    {
	      if (sizeflag & DFLAG)
		op &= 0xffffffff;
	      else
		op &= 0xffff;
	    }
	}
      break;
    case v_mode:
      if (sizeflag & DFLAG)
	op = get32s ();
      else
	op = get16 ();
      break;
    default:
      oappend (INTERNAL_DISASSEMBLER_ERROR);
      return;
    }

  scratchbuf[0] = '$';
  print_operand_value (scratchbuf + 1, 1, op);
  oappend (scratchbuf + intel_syntax);
}

static void
OP_J (int bytemode, int sizeflag)
{
  bfd_vma disp;
  bfd_vma mask = -1;
  bfd_vma segment = 0;

  switch (bytemode)
    {
    case b_mode:
      FETCH_DATA (the_info, codep + 1);
      disp = *codep++;
      if ((disp & 0x80) != 0)
	disp -= 0x100;
      break;
    case v_mode:
      USED_REX (REX_W);
      if ((sizeflag & DFLAG) || (rex & REX_W))
	disp = get32s ();
      else
	{
	  disp = get16 ();
	  if ((disp & 0x8000) != 0)
	    disp -= 0x10000;
	  /* In 16bit mode, address is wrapped around at 64k within
	     the same segment.  Otherwise, a data16 prefix on a jump
	     instruction means that the pc is masked to 16 bits after
	     the displacement is added!  */
	  mask = 0xffff;
	  if ((prefixes & PREFIX_DATA) == 0)
	    segment = ((start_pc + codep - start_codep)
		       & ~((bfd_vma) 0xffff));
	}
      if (!(rex & REX_W))
	used_prefixes |= (prefixes & PREFIX_DATA);
      break;
    default:
      oappend (INTERNAL_DISASSEMBLER_ERROR);
      return;
    }
  disp = ((start_pc + (codep - start_codep) + disp) & mask) | segment;
  set_op (disp, 0);
  print_operand_value (scratchbuf, 1, disp);
  oappend (scratchbuf);
}

static void
OP_SEG (int bytemode, int sizeflag)
{
  if (bytemode == w_mode)
    oappend (names_seg[modrm.reg]);
  else
    OP_E (modrm.mod == 3 ? bytemode : w_mode, sizeflag);
}

static void
OP_DIR (int dummy ATTRIBUTE_UNUSED, int sizeflag)
{
  int seg, offset;

  if (sizeflag & DFLAG)
    {
      offset = get32 ();
      seg = get16 ();
    }
  else
    {
      offset = get16 ();
      seg = get16 ();
    }
  used_prefixes |= (prefixes & PREFIX_DATA);
  if (intel_syntax)
    sprintf (scratchbuf, "0x%x:0x%x", seg, offset);
  else
    sprintf (scratchbuf, "$0x%x,$0x%x", seg, offset);
  oappend (scratchbuf);
}

static void
OP_OFF (int bytemode, int sizeflag)
{
  bfd_vma off;

  if (intel_syntax && (sizeflag & SUFFIX_ALWAYS))
    intel_operand_size (bytemode, sizeflag);
  append_seg ();

  if ((sizeflag & AFLAG) || address_mode == mode_64bit)
    off = get32 ();
  else
    off = get16 ();

  if (intel_syntax)
    {
      if (!(prefixes & (PREFIX_CS | PREFIX_SS | PREFIX_DS
			| PREFIX_ES | PREFIX_FS | PREFIX_GS)))
	{
	  oappend (names_seg[ds_reg - es_reg]);
	  oappend (":");
	}
    }
  print_operand_value (scratchbuf, 1, off);
  oappend (scratchbuf);
}

static void
OP_OFF64 (int bytemode, int sizeflag)
{
  bfd_vma off;

  if (address_mode != mode_64bit
      || (prefixes & PREFIX_ADDR))
    {
      OP_OFF (bytemode, sizeflag);
      return;
    }

  if (intel_syntax && (sizeflag & SUFFIX_ALWAYS))
    intel_operand_size (bytemode, sizeflag);
  append_seg ();

  off = get64 ();

  if (intel_syntax)
    {
      if (!(prefixes & (PREFIX_CS | PREFIX_SS | PREFIX_DS
			| PREFIX_ES | PREFIX_FS | PREFIX_GS)))
	{
	  oappend (names_seg[ds_reg - es_reg]);
	  oappend (":");
	}
    }
  print_operand_value (scratchbuf, 1, off);
  oappend (scratchbuf);
}

static void
ptr_reg (int code, int sizeflag)
{
  const char *s;

  *obufp++ = open_char;
  used_prefixes |= (prefixes & PREFIX_ADDR);
  if (address_mode == mode_64bit)
    {
      if (!(sizeflag & AFLAG))
	s = names32[code - eAX_reg];
      else
	s = names64[code - eAX_reg];
    }
  else if (sizeflag & AFLAG)
    s = names32[code - eAX_reg];
  else
    s = names16[code - eAX_reg];
  oappend (s);
  *obufp++ = close_char;
  *obufp = 0;
}

static void
OP_ESreg (int code, int sizeflag)
{
  if (intel_syntax)
    {
      switch (codep[-1])
	{
	case 0x6d:	/* insw/insl */
	  intel_operand_size (z_mode, sizeflag);
	  break;
	case 0xa5:	/* movsw/movsl/movsq */
	case 0xa7:	/* cmpsw/cmpsl/cmpsq */
	case 0xab:	/* stosw/stosl */
	case 0xaf:	/* scasw/scasl */
	  intel_operand_size (v_mode, sizeflag);
	  break;
	default:
	  intel_operand_size (b_mode, sizeflag);
	}
    }
//  oappend ("%es:" + intel_syntax);
  sprintf (scratchbuf, "%%es:");
  oappend (scratchbuf + intel_syntax);
  ptr_reg (code, sizeflag);
}

static void
OP_DSreg (int code, int sizeflag)
{
  if (intel_syntax)
    {
      switch (codep[-1])
	{
	case 0x6f:	/* outsw/outsl */
	  intel_operand_size (z_mode, sizeflag);
	  break;
	case 0xa5:	/* movsw/movsl/movsq */
	case 0xa7:	/* cmpsw/cmpsl/cmpsq */
	case 0xad:	/* lodsw/lodsl/lodsq */
	  intel_operand_size (v_mode, sizeflag);
	  break;
	default:
	  intel_operand_size (b_mode, sizeflag);
	}
    }
  if ((prefixes
       & (PREFIX_CS
	  | PREFIX_DS
	  | PREFIX_SS
	  | PREFIX_ES
	  | PREFIX_FS
	  | PREFIX_GS)) == 0)
    prefixes |= PREFIX_DS;
  append_seg ();
  ptr_reg (code, sizeflag);
}

static void
OP_C (int dummy ATTRIBUTE_UNUSED, int sizeflag ATTRIBUTE_UNUSED)
{
  int add;
  if (rex & REX_R)
    {
      USED_REX (REX_R);
      add = 8;
    }
  else if (address_mode != mode_64bit && (prefixes & PREFIX_LOCK))
    {
      all_prefixes[last_lock_prefix] = 0;
      used_prefixes |= PREFIX_LOCK;
      add = 8;
    }
  else
    add = 0;
  sprintf (scratchbuf, "%%cr%d", modrm.reg + add);
  oappend (scratchbuf + intel_syntax);
}

static void
OP_D (int dummy ATTRIBUTE_UNUSED, int sizeflag ATTRIBUTE_UNUSED)
{
  int add;
  USED_REX (REX_R);
  if (rex & REX_R)
    add = 8;
  else
    add = 0;
  if (intel_syntax)
    sprintf (scratchbuf, "db%d", modrm.reg + add);
  else
    sprintf (scratchbuf, "%%db%d", modrm.reg + add);
  oappend (scratchbuf);
}

static void
OP_T (int dummy ATTRIBUTE_UNUSED, int sizeflag ATTRIBUTE_UNUSED)
{
  sprintf (scratchbuf, "%%tr%d", modrm.reg);
  oappend (scratchbuf + intel_syntax);
}

static void
OP_R (int bytemode, int sizeflag)
{
  if (modrm.mod == 3)
    OP_E (bytemode, sizeflag);
  else
    BadOp ();
}

static void
OP_MMX (int bytemode ATTRIBUTE_UNUSED, int sizeflag ATTRIBUTE_UNUSED)
{
  int reg = modrm.reg;
  const char **names;

  used_prefixes |= (prefixes & PREFIX_DATA);
  if (prefixes & PREFIX_DATA)
    {
      names = names_xmm;
      USED_REX (REX_R);
      if (rex & REX_R)
	reg += 8;
    }
  else
    names = names_mm;
  oappend (names[reg]);
}

static void
OP_XMM (int bytemode, int sizeflag ATTRIBUTE_UNUSED)
{
  int reg = modrm.reg;
  const char **names;

  USED_REX (REX_R);
  if (rex & REX_R)
    reg += 8;
  if (need_vex
      && bytemode != xmm_mode
      && bytemode != scalar_mode)
    {
      switch (vex.length)
	{
	case 128:
	  names = names_xmm;
	  break;
	case 256:
	  if (vex.w || bytemode != vex_vsib_q_w_dq_mode)
	    names = names_ymm;
	  else
	    names = names_xmm;
	  break;
	default:
	  abort ();
	}
    }
  else
    names = names_xmm;
  oappend (names[reg]);
}

static void
OP_EM (int bytemode, int sizeflag)
{
  int reg;
  const char **names;

  if (modrm.mod != 3)
    {
      if (intel_syntax
	  && (bytemode == v_mode || bytemode == v_swap_mode))
	{
	  bytemode = (prefixes & PREFIX_DATA) ? x_mode : q_mode;
	  used_prefixes |= (prefixes & PREFIX_DATA);
 	}
      OP_E (bytemode, sizeflag);
      return;
    }

  if ((sizeflag & SUFFIX_ALWAYS) && bytemode == v_swap_mode)
    swap_operand ();

  /* Skip mod/rm byte.  */
  MODRM_CHECK;
  codep++;
  used_prefixes |= (prefixes & PREFIX_DATA);
  reg = modrm.rm;
  if (prefixes & PREFIX_DATA)
    {
      names = names_xmm;
      USED_REX (REX_B);
      if (rex & REX_B)
	reg += 8;
    }
  else
    names = names_mm;
  oappend (names[reg]);
}

/* cvt* are the only instructions in sse2 which have
   both SSE and MMX operands and also have 0x66 prefix
   in their opcode. 0x66 was originally used to differentiate
   between SSE and MMX instruction(operands). So we have to handle the
   cvt* separately using OP_EMC and OP_MXC */
static void
OP_EMC (int bytemode, int sizeflag)
{
  if (modrm.mod != 3)
    {
      if (intel_syntax && bytemode == v_mode)
	{
	  bytemode = (prefixes & PREFIX_DATA) ? x_mode : q_mode;
	  used_prefixes |= (prefixes & PREFIX_DATA);
 	}
      OP_E (bytemode, sizeflag);
      return;
    }

  /* Skip mod/rm byte.  */
  MODRM_CHECK;
  codep++;
  used_prefixes |= (prefixes & PREFIX_DATA);
  oappend (names_mm[modrm.rm]);
}

static void
OP_MXC (int bytemode ATTRIBUTE_UNUSED, int sizeflag ATTRIBUTE_UNUSED)
{
  used_prefixes |= (prefixes & PREFIX_DATA);
  oappend (names_mm[modrm.reg]);
}

static void
OP_EX (int bytemode, int sizeflag)
{
  int reg;
  const char **names;

  /* Skip mod/rm byte.  */
  MODRM_CHECK;
  codep++;

  if (modrm.mod != 3)
    {
      OP_E_memory (bytemode, sizeflag);
      return;
    }

  reg = modrm.rm;
  USED_REX (REX_B);
  if (rex & REX_B)
    reg += 8;

  if ((sizeflag & SUFFIX_ALWAYS)
      && (bytemode == x_swap_mode
	  || bytemode == d_swap_mode
	  || bytemode == d_scalar_swap_mode 
	  || bytemode == q_swap_mode
	  || bytemode == q_scalar_swap_mode))
    swap_operand ();

  if (need_vex
      && bytemode != xmm_mode
      && bytemode != xmmdw_mode
      && bytemode != xmmqd_mode
      && bytemode != xmm_mb_mode
      && bytemode != xmm_mw_mode
      && bytemode != xmm_md_mode
      && bytemode != xmm_mq_mode
      && bytemode != xmmq_mode
      && bytemode != d_scalar_mode
      && bytemode != d_scalar_swap_mode 
      && bytemode != q_scalar_mode
      && bytemode != q_scalar_swap_mode
      && bytemode != vex_scalar_w_dq_mode)
    {
      switch (vex.length)
	{
	case 128:
	  names = names_xmm;
	  break;
	case 256:
	  names = names_ymm;
	  break;
	default:
	  abort ();
	}
    }
  else
    names = names_xmm;
  oappend (names[reg]);
}

static void
OP_MS (int bytemode, int sizeflag)
{
  if (modrm.mod == 3)
    OP_EM (bytemode, sizeflag);
  else
    BadOp ();
}

static void
OP_XS (int bytemode, int sizeflag)
{
  if (modrm.mod == 3)
    OP_EX (bytemode, sizeflag);
  else
    BadOp ();
}

static void
OP_M (int bytemode, int sizeflag)
{
  if (modrm.mod == 3)
    /* bad bound,lea,lds,les,lfs,lgs,lss,cmpxchg8b,vmptrst modrm */
    BadOp ();
  else
    OP_E (bytemode, sizeflag);
}

static void
OP_0f07 (int bytemode, int sizeflag)
{
  if (modrm.mod != 3 || modrm.rm != 0)
    BadOp ();
  else
    OP_E (bytemode, sizeflag);
}

/* NOP is an alias of "xchg %ax,%ax" in 16bit mode, "xchg %eax,%eax" in
   32bit mode and "xchg %rax,%rax" in 64bit mode.  */

static void
NOP_Fixup1 (int bytemode, int sizeflag)
{
  if ((prefixes & PREFIX_DATA) != 0
      || (rex != 0
	  && rex != 0x48
	  && address_mode == mode_64bit))
    OP_REG (bytemode, sizeflag);
  else
    strcpy (obuf, "nop");
}

static void
NOP_Fixup2 (int bytemode, int sizeflag)
{
  if ((prefixes & PREFIX_DATA) != 0
      || (rex != 0
	  && rex != 0x48
	  && address_mode == mode_64bit))
    OP_IMREG (bytemode, sizeflag);
}

static const char *const Suffix3DNow[] = {
/* 00 */	NULL,		NULL,		NULL,		NULL,
/* 04 */	NULL,		NULL,		NULL,		NULL,
/* 08 */	NULL,		NULL,		NULL,		NULL,
/* 0C */	"pi2fw",	"pi2fd",	NULL,		NULL,
/* 10 */	NULL,		NULL,		NULL,		NULL,
/* 14 */	NULL,		NULL,		NULL,		NULL,
/* 18 */	NULL,		NULL,		NULL,		NULL,
/* 1C */	"pf2iw",	"pf2id",	NULL,		NULL,
/* 20 */	NULL,		NULL,		NULL,		NULL,
/* 24 */	NULL,		NULL,		NULL,		NULL,
/* 28 */	NULL,		NULL,		NULL,		NULL,
/* 2C */	NULL,		NULL,		NULL,		NULL,
/* 30 */	NULL,		NULL,		NULL,		NULL,
/* 34 */	NULL,		NULL,		NULL,		NULL,
/* 38 */	NULL,		NULL,		NULL,		NULL,
/* 3C */	NULL,		NULL,		NULL,		NULL,
/* 40 */	NULL,		NULL,		NULL,		NULL,
/* 44 */	NULL,		NULL,		NULL,		NULL,
/* 48 */	NULL,		NULL,		NULL,		NULL,
/* 4C */	NULL,		NULL,		NULL,		NULL,
/* 50 */	NULL,		NULL,		NULL,		NULL,
/* 54 */	NULL,		NULL,		NULL,		NULL,
/* 58 */	NULL,		NULL,		NULL,		NULL,
/* 5C */	NULL,		NULL,		NULL,		NULL,
/* 60 */	NULL,		NULL,		NULL,		NULL,
/* 64 */	NULL,		NULL,		NULL,		NULL,
/* 68 */	NULL,		NULL,		NULL,		NULL,
/* 6C */	NULL,		NULL,		NULL,		NULL,
/* 70 */	NULL,		NULL,		NULL,		NULL,
/* 74 */	NULL,		NULL,		NULL,		NULL,
/* 78 */	NULL,		NULL,		NULL,		NULL,
/* 7C */	NULL,		NULL,		NULL,		NULL,
/* 80 */	NULL,		NULL,		NULL,		NULL,
/* 84 */	NULL,		NULL,		NULL,		NULL,
/* 88 */	NULL,		NULL,		"pfnacc",	NULL,
/* 8C */	NULL,		NULL,		"pfpnacc",	NULL,
/* 90 */	"pfcmpge",	NULL,		NULL,		NULL,
/* 94 */	"pfmin",	NULL,		"pfrcp",	"pfrsqrt",
/* 98 */	NULL,		NULL,		"pfsub",	NULL,
/* 9C */	NULL,		NULL,		"pfadd",	NULL,
/* A0 */	"pfcmpgt",	NULL,		NULL,		NULL,
/* A4 */	"pfmax",	NULL,		"pfrcpit1",	"pfrsqit1",
/* A8 */	NULL,		NULL,		"pfsubr",	NULL,
/* AC */	NULL,		NULL,		"pfacc",	NULL,
/* B0 */	"pfcmpeq",	NULL,		NULL,		NULL,
/* B4 */	"pfmul",	NULL,		"pfrcpit2",	"pmulhrw",
/* B8 */	NULL,		NULL,		NULL,		"pswapd",
/* BC */	NULL,		NULL,		NULL,		"pavgusb",
/* C0 */	NULL,		NULL,		NULL,		NULL,
/* C4 */	NULL,		NULL,		NULL,		NULL,
/* C8 */	NULL,		NULL,		NULL,		NULL,
/* CC */	NULL,		NULL,		NULL,		NULL,
/* D0 */	NULL,		NULL,		NULL,		NULL,
/* D4 */	NULL,		NULL,		NULL,		NULL,
/* D8 */	NULL,		NULL,		NULL,		NULL,
/* DC */	NULL,		NULL,		NULL,		NULL,
/* E0 */	NULL,		NULL,		NULL,		NULL,
/* E4 */	NULL,		NULL,		NULL,		NULL,
/* E8 */	NULL,		NULL,		NULL,		NULL,
/* EC */	NULL,		NULL,		NULL,		NULL,
/* F0 */	NULL,		NULL,		NULL,		NULL,
/* F4 */	NULL,		NULL,		NULL,		NULL,
/* F8 */	NULL,		NULL,		NULL,		NULL,
/* FC */	NULL,		NULL,		NULL,		NULL,
};

static void
OP_3DNowSuffix (int bytemode ATTRIBUTE_UNUSED, int sizeflag ATTRIBUTE_UNUSED)
{
  const char *mnemonic;

  FETCH_DATA (the_info, codep + 1);
  /* AMD 3DNow! instructions are specified by an opcode suffix in the
     place where an 8-bit immediate would normally go.  ie. the last
     byte of the instruction.  */
  obufp = mnemonicendp;
  mnemonic = Suffix3DNow[*codep++ & 0xff];
  if (mnemonic)
    oappend (mnemonic);
  else
    {
      /* Since a variable sized modrm/sib chunk is between the start
	 of the opcode (0x0f0f) and the opcode suffix, we need to do
	 all the modrm processing first, and don't know until now that
	 we have a bad opcode.  This necessitates some cleaning up.  */
      op_out[0][0] = '\0';
      op_out[1][0] = '\0';
      BadOp ();
    }
  mnemonicendp = obufp;
}

static struct op simd_cmp_op[] =
{
  { STRING_COMMA_LEN ("eq") },
  { STRING_COMMA_LEN ("lt") },
  { STRING_COMMA_LEN ("le") },
  { STRING_COMMA_LEN ("unord") },
  { STRING_COMMA_LEN ("neq") },
  { STRING_COMMA_LEN ("nlt") },
  { STRING_COMMA_LEN ("nle") },
  { STRING_COMMA_LEN ("ord") }
};

static void
CMP_Fixup (int bytemode ATTRIBUTE_UNUSED, int sizeflag ATTRIBUTE_UNUSED)
{
  unsigned int cmp_type;

  FETCH_DATA (the_info, codep + 1);
  cmp_type = *codep++ & 0xff;
  if (cmp_type < ARRAY_SIZE (simd_cmp_op))
    {
      char suffix [3];
      char *p = mnemonicendp - 2;
      suffix[0] = p[0];
      suffix[1] = p[1];
      suffix[2] = '\0';
      sprintf (p, "%s%s", simd_cmp_op[cmp_type].name, suffix);
      mnemonicendp += simd_cmp_op[cmp_type].len;
    }
  else
    {
      /* We have a reserved extension byte.  Output it directly.  */
      scratchbuf[0] = '$';
      print_operand_value (scratchbuf + 1, 1, cmp_type);
      oappend (scratchbuf + intel_syntax);
      scratchbuf[0] = '\0';
    }
}

static void
OP_Mwait (int bytemode ATTRIBUTE_UNUSED,
	  int sizeflag ATTRIBUTE_UNUSED)
{
  /* mwait %eax,%ecx  */
  if (!intel_syntax)
    {
      const char **names = (address_mode == mode_64bit
			    ? names64 : names32);
      strcpy (op_out[0], names[0]);
      strcpy (op_out[1], names[1]);
      two_source_ops = 1;
    }
  /* Skip mod/rm byte.  */
  MODRM_CHECK;
  codep++;
}

static void
OP_Monitor (int bytemode ATTRIBUTE_UNUSED,
	    int sizeflag ATTRIBUTE_UNUSED)
{
  /* monitor %eax,%ecx,%edx"  */
  if (!intel_syntax)
    {
      const char **op1_names;
      const char **names = (address_mode == mode_64bit
			    ? names64 : names32);

      if (!(prefixes & PREFIX_ADDR))
	op1_names = (address_mode == mode_16bit
		     ? names16 : names);
      else
	{
	  /* Remove "addr16/addr32".  */
	  all_prefixes[last_addr_prefix] = 0;
	  op1_names = (address_mode != mode_32bit
		       ? names32 : names16);
	  used_prefixes |= PREFIX_ADDR;
	}
      strcpy (op_out[0], op1_names[0]);
      strcpy (op_out[1], names[1]);
      strcpy (op_out[2], names[2]);
      two_source_ops = 1;
    }
  /* Skip mod/rm byte.  */
  MODRM_CHECK;
  codep++;
}

static void
BadOp (void)
{
  /* Throw away prefixes and 1st. opcode byte.  */
  codep = insn_codep + 1;
  oappend ("(bad)");
}

static void
REP_Fixup (int bytemode, int sizeflag)
{
  /* The 0xf3 prefix should be displayed as "rep" for ins, outs, movs,
     lods and stos.  */
  if (prefixes & PREFIX_REPZ)
    all_prefixes[last_repz_prefix] = REP_PREFIX;

  switch (bytemode)
    {
    case al_reg:
    case eAX_reg:
    case indir_dx_reg:
      OP_IMREG (bytemode, sizeflag);
      break;
    case eDI_reg:
      OP_ESreg (bytemode, sizeflag);
      break;
    case eSI_reg:
      OP_DSreg (bytemode, sizeflag);
      break;
    default:
      abort ();
      break;
    }
}

/* Similar to OP_E.  But the 0xf2/0xf3 prefixes should be displayed as
   "xacquire"/"xrelease" for memory operand if there is a LOCK prefix.
 */

static void
HLE_Fixup1 (int bytemode, int sizeflag)
{
  if (modrm.mod != 3
      && (prefixes & PREFIX_LOCK) != 0)
    {
      if (prefixes & PREFIX_REPZ)
	all_prefixes[last_repz_prefix] = XRELEASE_PREFIX;
      if (prefixes & PREFIX_REPNZ)
	all_prefixes[last_repnz_prefix] = XACQUIRE_PREFIX;
    }

  OP_E (bytemode, sizeflag);
}

/* Similar to OP_E.  But the 0xf2/0xf3 prefixes should be displayed as
   "xacquire"/"xrelease" for memory operand.  No check for LOCK prefix.
 */

static void
HLE_Fixup2 (int bytemode, int sizeflag)
{
  if (modrm.mod != 3)
    {
      if (prefixes & PREFIX_REPZ)
	all_prefixes[last_repz_prefix] = XRELEASE_PREFIX;
      if (prefixes & PREFIX_REPNZ)
	all_prefixes[last_repnz_prefix] = XACQUIRE_PREFIX;
    }

  OP_E (bytemode, sizeflag);
}

/* Similar to OP_E.  But the 0xf3 prefixes should be displayed as
   "xrelease" for memory operand.  No check for LOCK prefix.   */

static void
HLE_Fixup3 (int bytemode, int sizeflag)
{
  if (modrm.mod != 3
      && last_repz_prefix > last_repnz_prefix
      && (prefixes & PREFIX_REPZ) != 0)
    all_prefixes[last_repz_prefix] = XRELEASE_PREFIX;

  OP_E (bytemode, sizeflag);
}

static void
CMPXCHG8B_Fixup (int bytemode, int sizeflag)
{
  USED_REX (REX_W);
  if (rex & REX_W)
    {
      /* Change cmpxchg8b to cmpxchg16b.  */
      char *p = mnemonicendp - 2;
      mnemonicendp = stpcpy (p, "16b");
      bytemode = o_mode;
    }
  else if ((prefixes & PREFIX_LOCK) != 0)
    {
      if (prefixes & PREFIX_REPZ)
	all_prefixes[last_repz_prefix] = XRELEASE_PREFIX;
      if (prefixes & PREFIX_REPNZ)
	all_prefixes[last_repnz_prefix] = XACQUIRE_PREFIX;
    }

  OP_M (bytemode, sizeflag);
}

static void
XMM_Fixup (int reg, int sizeflag ATTRIBUTE_UNUSED)
{
  const char **names;

  if (need_vex)
    {
      switch (vex.length)
	{
	case 128:
	  names = names_xmm;
	  break;
	case 256:
	  names = names_ymm;
	  break;
	default:
	  abort ();
	}
    }
  else
    names = names_xmm;
  oappend (names[reg]);
}

static void
CRC32_Fixup (int bytemode, int sizeflag)
{
  /* Add proper suffix to "crc32".  */
  char *p = mnemonicendp;

  switch (bytemode)
    {
    case b_mode:
      if (intel_syntax)
	goto skip;

      *p++ = 'b';
      break;
    case v_mode:
      if (intel_syntax)
	goto skip;

      USED_REX (REX_W);
      if (rex & REX_W)
	*p++ = 'q';
      else 
	{
	  if (sizeflag & DFLAG)
	    *p++ = 'l';
	  else
	    *p++ = 'w';
	  used_prefixes |= (prefixes & PREFIX_DATA);
	}
      break;
    default:
      oappend (INTERNAL_DISASSEMBLER_ERROR);
      break;
    }
  mnemonicendp = p;
  *p = '\0';

skip:
  if (modrm.mod == 3)
    {
      int add;

      /* Skip mod/rm byte.  */
      MODRM_CHECK;
      codep++;

      USED_REX (REX_B);
      add = (rex & REX_B) ? 8 : 0;
      if (bytemode == b_mode)
	{
	  USED_REX (0);
	  if (rex)
	    oappend (names8rex[modrm.rm + add]);
	  else
	    oappend (names8[modrm.rm + add]);
	}
      else
	{
	  USED_REX (REX_W);
	  if (rex & REX_W)
	    oappend (names64[modrm.rm + add]);
	  else if ((prefixes & PREFIX_DATA))
	    oappend (names16[modrm.rm + add]);
	  else
	    oappend (names32[modrm.rm + add]);
	}
    }
  else
    OP_E (bytemode, sizeflag);
}

static void
FXSAVE_Fixup (int bytemode, int sizeflag)
{
  /* Add proper suffix to "fxsave" and "fxrstor".  */
  USED_REX (REX_W);
  if (rex & REX_W)
    {
      char *p = mnemonicendp;
      *p++ = '6';
      *p++ = '4';
      *p = '\0';
      mnemonicendp = p;
    }
  OP_M (bytemode, sizeflag);
}

/* Display the destination register operand for instructions with
   VEX. */

static void
OP_VEX (int bytemode, int sizeflag ATTRIBUTE_UNUSED)
{
  int reg;
  const char **names;

  if (!need_vex)
    abort ();

  if (!need_vex_reg)
    return;

  reg = vex.register_specifier;
  if (bytemode == vex_scalar_mode)
    {
      oappend (names_xmm[reg]);
      return;
    }

  switch (vex.length)
    {
    case 128:
      switch (bytemode)
	{
	case vex_mode:
	case vex128_mode:
	case vex_vsib_q_w_dq_mode:
	  names = names_xmm;
	  break;
	case dq_mode:
	  if (vex.w)
	    names = names64;
	  else
	    names = names32;
	  break;
	default:
	  abort ();
	  return;
	}
      break;
    case 256:
      switch (bytemode)
	{
	case vex_mode:
	case vex256_mode:
	  names = names_ymm;
	  break;
	case vex_vsib_q_w_dq_mode:
	  names = vex.w ? names_ymm : names_xmm;
	  break;
	default:
	  abort ();
	  return;
	}
      break;
    default:
      abort ();
      break;
    }
  oappend (names[reg]);
}

/* Get the VEX immediate byte without moving codep.  */

static unsigned char
get_vex_imm8 (int sizeflag, int opnum)
{
  int bytes_before_imm = 0;

  if (modrm.mod != 3)
    {
      /* There are SIB/displacement bytes.  */
      if ((sizeflag & AFLAG) || address_mode == mode_64bit)
        {
	  /* 32/64 bit address mode */
          int base = modrm.rm;

	  /* Check SIB byte.  */
          if (base == 4)
            {
              FETCH_DATA (the_info, codep + 1);
              base = *codep & 7;
              /* When decoding the third source, don't increase
                 bytes_before_imm as this has already been incremented
                 by one in OP_E_memory while decoding the second
                 source operand.  */
              if (opnum == 0)
                bytes_before_imm++;
            }

          /* Don't increase bytes_before_imm when decoding the third source,
             it has already been incremented by OP_E_memory while decoding
             the second source operand.  */
          if (opnum == 0)
            {
              switch (modrm.mod)
                {
                  case 0:
                    /* When modrm.rm == 5 or modrm.rm == 4 and base in
                       SIB == 5, there is a 4 byte displacement.  */
                    if (base != 5)
                      /* No displacement. */
                      break;
                  case 2:
                    /* 4 byte displacement.  */
                    bytes_before_imm += 4;
                    break;
                  case 1:
                    /* 1 byte displacement.  */
                    bytes_before_imm++;
                    break;
                }
            }
        }
      else
	{
	  /* 16 bit address mode */
          /* Don't increase bytes_before_imm when decoding the third source,
             it has already been incremented by OP_E_memory while decoding
             the second source operand.  */
          if (opnum == 0)
            {
	      switch (modrm.mod)
		{
		case 0:
		  /* When modrm.rm == 6, there is a 2 byte displacement.  */
		  if (modrm.rm != 6)
		    /* No displacement. */
		    break;
		case 2:
		  /* 2 byte displacement.  */
		  bytes_before_imm += 2;
		  break;
		case 1:
		  /* 1 byte displacement: when decoding the third source,
		     don't increase bytes_before_imm as this has already
		     been incremented by one in OP_E_memory while decoding
		     the second source operand.  */
		  if (opnum == 0)
		    bytes_before_imm++;

		  break;
		}
	    }
	}
    }

  FETCH_DATA (the_info, codep + bytes_before_imm + 1);
  return codep [bytes_before_imm];
}

static void
OP_EX_VexReg (int bytemode, int sizeflag, int reg)
{
  const char **names;

  if (reg == -1 && modrm.mod != 3)
    {
      OP_E_memory (bytemode, sizeflag);
      return;
    }
  else
    {
      if (reg == -1)
	{
	  reg = modrm.rm;
	  USED_REX (REX_B);
	  if (rex & REX_B)
	    reg += 8;
	}
      else if (reg > 7 && address_mode != mode_64bit)
	BadOp ();
    }

  switch (vex.length)
    {
    case 128:
      names = names_xmm;
      break;
    case 256:
      names = names_ymm;
      break;
    default:
      abort ();
    }
  oappend (names[reg]);
}

static void
OP_EX_VexImmW (int bytemode, int sizeflag)
{
  int reg = -1;
  static unsigned char vex_imm8;

  if (vex_w_done == 0)
    {
      vex_w_done = 1;

      /* Skip mod/rm byte.  */
      MODRM_CHECK;
      codep++;

      vex_imm8 = get_vex_imm8 (sizeflag, 0);

      if (vex.w)
	  reg = vex_imm8 >> 4;

      OP_EX_VexReg (bytemode, sizeflag, reg);
    }
  else if (vex_w_done == 1)
    {
      vex_w_done = 2;

      if (!vex.w)
	  reg = vex_imm8 >> 4;

      OP_EX_VexReg (bytemode, sizeflag, reg);
    }
  else
    {
      /* Output the imm8 directly.  */
      scratchbuf[0] = '$';
      print_operand_value (scratchbuf + 1, 1, vex_imm8 & 0xf);
      oappend (scratchbuf + intel_syntax);
      scratchbuf[0] = '\0';
      codep++;
    }
}

static void
OP_Vex_2src (int bytemode, int sizeflag)
{
  if (modrm.mod == 3)
    {
      int reg = modrm.rm;
      USED_REX (REX_B);
      if (rex & REX_B)
	reg += 8;
      oappend (names_xmm[reg]);
    }
  else
    {
      if (intel_syntax
	  && (bytemode == v_mode || bytemode == v_swap_mode))
	{
	  bytemode = (prefixes & PREFIX_DATA) ? x_mode : q_mode;
	  used_prefixes |= (prefixes & PREFIX_DATA);
	}
      OP_E (bytemode, sizeflag);
    }
}

static void
OP_Vex_2src_1 (int bytemode, int sizeflag)
{
  if (modrm.mod == 3)
    {
      /* Skip mod/rm byte.   */
      MODRM_CHECK;
      codep++;
    }

  if (vex.w)
    oappend (names_xmm[vex.register_specifier]);
  else
    OP_Vex_2src (bytemode, sizeflag);
}

static void
OP_Vex_2src_2 (int bytemode, int sizeflag)
{
  if (vex.w)
    OP_Vex_2src (bytemode, sizeflag);
  else
    oappend (names_xmm[vex.register_specifier]);
}

static void
OP_EX_VexW (int bytemode, int sizeflag)
{
  int reg = -1;

  if (!vex_w_done)
    {
      vex_w_done = 1;

      /* Skip mod/rm byte.  */
      MODRM_CHECK;
      codep++;

      if (vex.w)
	reg = get_vex_imm8 (sizeflag, 0) >> 4;
    }
  else
    {
      if (!vex.w)
	reg = get_vex_imm8 (sizeflag, 1) >> 4;
    }

  OP_EX_VexReg (bytemode, sizeflag, reg);
}

static void
VEXI4_Fixup (int bytemode ATTRIBUTE_UNUSED,
	     int sizeflag ATTRIBUTE_UNUSED)
{
  /* Skip the immediate byte and check for invalid bits.  */
  FETCH_DATA (the_info, codep + 1);
  if (*codep++ & 0xf)
    BadOp ();
}

static void
OP_REG_VexI4 (int bytemode, int sizeflag ATTRIBUTE_UNUSED)
{
  int reg;
  const char **names;

  FETCH_DATA (the_info, codep + 1);
  reg = *codep++;

  if (bytemode != x_mode)
    abort ();

  if (reg & 0xf)
      BadOp ();

  reg >>= 4;
  if (reg > 7 && address_mode != mode_64bit)
    BadOp ();

  switch (vex.length)
    {
    case 128:
      names = names_xmm;
      break;
    case 256:
      names = names_ymm;
      break;
    default:
      abort ();
    }
  oappend (names[reg]);
}

static void
OP_XMM_VexW (int bytemode, int sizeflag)
{
  /* Turn off the REX.W bit since it is used for swapping operands
     now.  */
  rex &= ~REX_W;
  OP_XMM (bytemode, sizeflag);
}

static void
OP_EX_Vex (int bytemode, int sizeflag)
{
  if (modrm.mod != 3)
    {
      if (vex.register_specifier != 0)
	BadOp ();
      need_vex_reg = 0;
    }
  OP_EX (bytemode, sizeflag);
}

static void
OP_XMM_Vex (int bytemode, int sizeflag)
{
  if (modrm.mod != 3)
    {
      if (vex.register_specifier != 0)
	BadOp ();
      need_vex_reg = 0;
    }
  OP_XMM (bytemode, sizeflag);
}

static void
VZERO_Fixup (int bytemode ATTRIBUTE_UNUSED, int sizeflag ATTRIBUTE_UNUSED)
{
  switch (vex.length)
    {
    case 128:
      mnemonicendp = stpcpy (obuf, "vzeroupper");
      break;
    case 256:
      mnemonicendp = stpcpy (obuf, "vzeroall");
      break;
    default:
      abort ();
    }
}

static struct op vex_cmp_op[] =
{
  { STRING_COMMA_LEN ("eq") },
  { STRING_COMMA_LEN ("lt") },
  { STRING_COMMA_LEN ("le") },
  { STRING_COMMA_LEN ("unord") },
  { STRING_COMMA_LEN ("neq") },
  { STRING_COMMA_LEN ("nlt") },
  { STRING_COMMA_LEN ("nle") },
  { STRING_COMMA_LEN ("ord") },
  { STRING_COMMA_LEN ("eq_uq") },
  { STRING_COMMA_LEN ("nge") },
  { STRING_COMMA_LEN ("ngt") },
  { STRING_COMMA_LEN ("false") },
  { STRING_COMMA_LEN ("neq_oq") },
  { STRING_COMMA_LEN ("ge") },
  { STRING_COMMA_LEN ("gt") },
  { STRING_COMMA_LEN ("true") },
  { STRING_COMMA_LEN ("eq_os") },
  { STRING_COMMA_LEN ("lt_oq") },
  { STRING_COMMA_LEN ("le_oq") },
  { STRING_COMMA_LEN ("unord_s") },
  { STRING_COMMA_LEN ("neq_us") },
  { STRING_COMMA_LEN ("nlt_uq") },
  { STRING_COMMA_LEN ("nle_uq") },
  { STRING_COMMA_LEN ("ord_s") },
  { STRING_COMMA_LEN ("eq_us") },
  { STRING_COMMA_LEN ("nge_uq") },
  { STRING_COMMA_LEN ("ngt_uq") },
  { STRING_COMMA_LEN ("false_os") },
  { STRING_COMMA_LEN ("neq_os") },
  { STRING_COMMA_LEN ("ge_oq") },
  { STRING_COMMA_LEN ("gt_oq") },
  { STRING_COMMA_LEN ("true_us") },
};

static void
VCMP_Fixup (int bytemode ATTRIBUTE_UNUSED, int sizeflag ATTRIBUTE_UNUSED)
{
  unsigned int cmp_type;

  FETCH_DATA (the_info, codep + 1);
  cmp_type = *codep++ & 0xff;
  if (cmp_type < ARRAY_SIZE (vex_cmp_op))
    {
      char suffix [3];
      char *p = mnemonicendp - 2;
      suffix[0] = p[0];
      suffix[1] = p[1];
      suffix[2] = '\0';
      sprintf (p, "%s%s", vex_cmp_op[cmp_type].name, suffix);
      mnemonicendp += vex_cmp_op[cmp_type].len;
    }
  else
    {
      /* We have a reserved extension byte.  Output it directly.  */
      scratchbuf[0] = '$';
      print_operand_value (scratchbuf + 1, 1, cmp_type);
      oappend (scratchbuf + intel_syntax);
      scratchbuf[0] = '\0';
    }
}

static const struct op pclmul_op[] =
{
  { STRING_COMMA_LEN ("lql") },
  { STRING_COMMA_LEN ("hql") },
  { STRING_COMMA_LEN ("lqh") },
  { STRING_COMMA_LEN ("hqh") }
};

static void
PCLMUL_Fixup (int bytemode ATTRIBUTE_UNUSED,
	      int sizeflag ATTRIBUTE_UNUSED)
{
  unsigned int pclmul_type;

  FETCH_DATA (the_info, codep + 1);
  pclmul_type = *codep++ & 0xff;
  switch (pclmul_type)
    {
    case 0x10:
      pclmul_type = 2;
      break;
    case 0x11:
      pclmul_type = 3;
      break;
    default:
      break;
    } 
  if (pclmul_type < ARRAY_SIZE (pclmul_op))
    {
      char suffix [4];
      char *p = mnemonicendp - 3;
      suffix[0] = p[0];
      suffix[1] = p[1];
      suffix[2] = p[2];
      suffix[3] = '\0';
      sprintf (p, "%s%s", pclmul_op[pclmul_type].name, suffix);
      mnemonicendp += pclmul_op[pclmul_type].len;
    }
  else
    {
      /* We have a reserved extension byte.  Output it directly.  */
      scratchbuf[0] = '$';
      print_operand_value (scratchbuf + 1, 1, pclmul_type);
      oappend (scratchbuf + intel_syntax);
      scratchbuf[0] = '\0';
    }
}

static void
MOVBE_Fixup (int bytemode, int sizeflag)
{
  /* Add proper suffix to "movbe".  */
  char *p = mnemonicendp;

  switch (bytemode)
    {
    case v_mode:
      if (intel_syntax)
	goto skip;

      USED_REX (REX_W);
      if (sizeflag & SUFFIX_ALWAYS)
	{
	  if (rex & REX_W)
	    *p++ = 'q';
	  else
	    {
	      if (sizeflag & DFLAG)
		*p++ = 'l';
	      else
		*p++ = 'w';
	      used_prefixes |= (prefixes & PREFIX_DATA);
	    }
	}
      break;
    default:
      oappend (INTERNAL_DISASSEMBLER_ERROR);
      break;
    }
  mnemonicendp = p;
  *p = '\0';

skip:
  OP_M (bytemode, sizeflag);
}

static void
OP_LWPCB_E (int bytemode ATTRIBUTE_UNUSED, int sizeflag ATTRIBUTE_UNUSED)
{
  int reg;
  const char **names;

  /* Skip mod/rm byte.  */
  MODRM_CHECK;
  codep++;

  if (vex.w)
    names = names64;
  else
    names = names32;

  reg = modrm.rm;
  USED_REX (REX_B);
  if (rex & REX_B)
    reg += 8;

  oappend (names[reg]);
}

static void
OP_LWP_E (int bytemode ATTRIBUTE_UNUSED, int sizeflag ATTRIBUTE_UNUSED)
{
  const char **names;

  if (vex.w)
    names = names64;
  else
    names = names32;

  oappend (names[vex.register_specifier]);
}

