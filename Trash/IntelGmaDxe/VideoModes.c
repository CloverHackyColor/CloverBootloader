/*++

  Copyright (c)  2006 - 2010 Intel Corporation. All rights reserved
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  VideoModes.c
    
Abstract:

  Intel Video Controller Driver

Revision History

--*/

#include "Gop.h"

//
// MODE 0 - Turns off display controller.
//
MODE_FORMAT DS_0_0_0_0[] = {
  {
    DPLLADivisor,
    0x00000000,
    FALSE
  },
  {
    DPLLAControl,
    0x00000000,
    FALSE
  },
  {
    HTOTAL_A,
    0x00000000,
    FALSE
  },
  {
    HBLANK_A,
    0x00000000,
    FALSE
  },
  {
    HSYNC_A,
    0x00000000,
    FALSE
  },
  {
    VTOTAL_A,
    0x00000000,
    FALSE
  },
  {
    VBLANK_A,
    0x00000000,
    FALSE
  },
  {
    VSYNC_A,
    0x00000000,
    FALSE
  },
  {
    PIPESRC_A,
    0x00000000,
    FALSE
  },
  {
    BDRCOLRPTRN_A,
    0x00000000,
    FALSE
  },
  {
    ColorChannel_Red_A,
    0x00000000,
    FALSE
  },
  {
    ColorChannel_Grn_A,
    0x00000000,
    FALSE
  },
  {
    ColorChannel_Blue_A,
    0x00000000,
    FALSE
  },
  {
    PIPEASTAT,
    0x00000000,
    FALSE
  },
  {
    DSPASTRIDE,
    0x00000000,
    FALSE
  },
  {
    ADPA,
    0x00000000,
    FALSE
  },
};

//
// Make sure there are NUM_DS_ENTRIES in the structure
//
//C_ASSERT(sizeof (DS_0_0_0_0) == NUM_DS_ENTRIES * sizeof (MODE_FORMAT));

//
// 640x480 Modes 60Hz
// MODE 640x480x32x60
//
MODE_FORMAT DS_640_480_32_60[] = {
  {
    HTOTAL_A,
    0x031f027f,
    FALSE
  },
  {
    HBLANK_A,
    0x03170287,
    FALSE
  },
  {
    HSYNC_A,
    0x02ef028f,
    FALSE
  },
  {
    VTOTAL_A,
    0x020c01df,
    FALSE
  },
  {
    VBLANK_A,
    0x020401e7,
    FALSE
  },
  {
    VSYNC_A,
    0x01eb01e9,
    FALSE
  },
  {
    PIPESRC_A,
    0x027f01df,
    FALSE
  },
  {
    BDRCOLRPTRN_A,
    0x00000000,
    FALSE
  },
  {
    ColorChannel_Red_A,
    0x00000000,
    FALSE
  },
  {
    ColorChannel_Grn_A,
    0x00000000,
    FALSE
  },
  {
    ColorChannel_Blue_A,
    0x00000000,
    FALSE
  },
  {
    DPLLADivisor,
    0x00200067,
    FALSE
  },
  {
    DPLLAControl,
    0x94400000,
    FALSE
  },
  {
    PIPEASTAT,
    0x00000203,
    FALSE
  },
  {
    DSPASTRIDE,
    0x00000a00,
    FALSE
  },
  {
    ADPA,
    0x80000000,
    FALSE
  },
};
//C_ASSERT(sizeof (DS_640_480_32_60) == NUM_DS_ENTRIES * sizeof (MODE_FORMAT));

//
// 800x600 Modes 60Hz
// MODE 800x600x32x60
//
MODE_FORMAT DS_800_600_32_60[] = {
  {
    HTOTAL_A,
    0x041f031f,
    FALSE
  },
  {
    HBLANK_A,
    0x041f031f,
    FALSE
  },
  {
    HSYNC_A,
    0x03c70347,
    FALSE
  },
  {
    VTOTAL_A,
    0x02730257,
    FALSE
  },
  {
    VBLANK_A,
    0x02730257,
    FALSE
  },
  {
    VSYNC_A,
    0x025c0258,
    FALSE
  },
  {
    PIPESRC_A,
    0x031f0257,
    FALSE
  },
  {
    BDRCOLRPTRN_A,
    0x00000000,
    FALSE
  },
  {
    ColorChannel_Red_A,
    0x00000000,
    FALSE
  },
  {
    ColorChannel_Grn_A,
    0x00000000,
    FALSE
  },
  {
    ColorChannel_Blue_A,
    0x00000000,
    FALSE
  },
  {
    DPLLADivisor,
    0x0020007B,
    FALSE
  },
  {
    DPLLAControl,
    0x94100000,
    FALSE
  },
  {
    PIPEASTAT,
    0x00000203,
    FALSE
  },
  {
    DSPASTRIDE,
    0x00000c80,
    FALSE
  },
  {
    ADPA,
    0x80000018,
    FALSE
  },
};
//C_ASSERT(sizeof (DS_800_600_32_60) == NUM_DS_ENTRIES * sizeof (MODE_FORMAT));

//
// 1024x768 Modes 60Hz
// MODE 1024x768x32x60
//
MODE_FORMAT DS_1024_768_32_60[] = {
  {
    HTOTAL_A,
    0x053F03FF,
    FALSE
  },
  {
    HBLANK_A,
    0x053F03FF,
    FALSE
  },
  {
    HSYNC_A,
    0x049F0417,
    FALSE
  },
  {
    VTOTAL_A,
    0x032502FF,
    FALSE
  },
  {
    VBLANK_A,
    0x032502FF,
    FALSE
  },
  {
    VSYNC_A,
    0x03080302,
    FALSE
  },
  {
    PIPESRC_A,
    0x03FF02FF,
    FALSE
  },
  {
    BDRCOLRPTRN_A,
    0x00000000,
    FALSE
  },
  {
    ColorChannel_Red_A,
    0x00000000,
    FALSE
  },
  {
    ColorChannel_Grn_A,
    0x00000000,
    FALSE
  },
  {
    ColorChannel_Blue_A,
    0x00000000,
    FALSE
  },
  {
    DPLLADivisor,
    0x0010006A,
    FALSE
  },
  {
    DPLLAControl,
    0x94040000,
    FALSE
  },
  {
    PIPEASTAT,
    0x00000203,
    FALSE
  },
  {
    DSPASTRIDE,
    0x00001000,
    FALSE
  },
  {
    ADPA,
    0x80000000,
    FALSE
  },
};
//C_ASSERT(sizeof (DS_1024_768_32_60) == NUM_DS_ENTRIES * sizeof (MODE_FORMAT));

//
// Generic shutdown controller;  this is used to turn off the controller
// before changing the mode.  These must be done in this order.  The order
// in which these entries appear in the table is the order in which the
// values are written to the h/w.
//
MODE_FORMAT           mDISPLAY_SHUTDOWN[] = {
  {
    DSPACNTR,
    0x00000000,
    FALSE
  },
  //
  // Turn off display plane A
  //
  {
    ADPA,
    0x00000000,
    FALSE
  },
  //
  // Turn off port (disable sync signals)
  //
  {
    PIPEACONF,
    0x00000000,
    FALSE
  },
  //
  // Shutdown pipe
  //
  {
    VGACNTRL,
    0x00000000,
    FALSE
  },
  //
  // Turn off VGA display register
  //
  {
    DPLLAControl,
    0x00000000,
    FALSE
  },
  //
  // Turn off PLL
  //
  {
    PGTBL_CTL,
    0x00000000,
    FALSE
  },
  //
  //   { PGTBL_CTL,    0x00000000, TRUE  },
  //
};

UINT16                mNUM_SHUTDOWN_ENTRIES = sizeof (mDISPLAY_SHUTDOWN) / sizeof (mDISPLAY_SHUTDOWN[0]);

//
// Generic start-up controller;  This is used to turn on the controller
// before changing the mode.  These must be done in this order.  The order
// in which these entries appear in the table is the order in which the
// values are written to the h/w.
//
MODE_FORMAT           mDISPLAY_STARTUP[] = {
  {
    PIPEACONF,
    0x80000000,
    FALSE
  },
  //
  // Turn on the display pipe
  //
  {
    VGACNTRL,
    0x80100000,
    FALSE
  },
  //
  // 8-bit DAC, disable VGA
  //
  {
    DSPACNTR,
    0x98000000,
    FALSE
  },
  //
  // Enable plane A & set x:8:8:8 format
  //
  {
    DSPABASE,
    0x00000000,
    FALSE
  },
  //
  // Display starts at base address of GTT
  //
};

UINT16                mNUM_STARTUP_ENTRIES = sizeof (mDISPLAY_STARTUP) / sizeof (mDISPLAY_STARTUP[0]);

MODE_FORMAT LVDS_SHUTDOWN[] = {
{PP_CONTROL    , 0xabcd0000, FALSE},
{PIPEBSTAT     , 0         , FALSE}, 
{BLC_PWM_CTL   , 0         , FALSE},
{PFIT_CONTROL  , 0         , FALSE}, 
{LVDSPC        , 0         , FALSE},
{DSPBCNTR      , 0         , TRUE},
{DSPBLINOFFSET , 0         , TRUE},
{DSPBSTRIDE    , 0         , TRUE},
{DSPBSIZE      , 0         , TRUE},
{VGACNTRL      , 0         , FALSE},
{PIPEBCONF     , 0         , TRUE},
{CRCCtrlColorBB, 0         , FALSE},
{CRCCtrlColorBG, 0         , FALSE},
{CRCCtrlColorBR, 0         , FALSE},
{BCLRPAT_B     , 0         , FALSE},
{PIPEBSRC      , 0         , FALSE},
{DPLLB_CTRL    , 0         , FALSE}, 
{FPB0          , 0         , FALSE}, 
{VSYNC_B       , 0         , FALSE},
{VBLANK_B      , 0         , FALSE},
{VTOTAL_B      , 0         , FALSE},
{HSYNC_B       , 0         , FALSE},
{HBLANK_B      , 0         , FALSE},
{HTOTAL_B      , 0         , FALSE},
{PP_DIVISOR    , 0         , FALSE},
{PP_OFF_DELAYS , 0         , FALSE},
{PP_ON_DELAYS  , 0         , TRUE},
{PP_CONTROL    , 0         , FALSE}, 
{0,0,0}

};
MODE_FORMAT LVDS_MODE_DATA_640_480[] = {
{PP_CONTROL    , 0xabcd0000, FALSE},
{PP_ON_DELAYS  , 0x25807d0 , TRUE},
{PP_OFF_DELAYS , 0x1f407d0 , FALSE},
{PP_DIVISOR    , 0x209d05  , FALSE},
{HTOTAL_B      , 0x4af03ff , FALSE},
{HBLANK_B      , 0x4af03ff , FALSE},
{HSYNC_B       , 0x44f042f , FALSE},
{VTOTAL_B      , 0x26e0257 , FALSE},
{VBLANK_B      , 0x26e0257 , FALSE},
{VSYNC_B       , 0x260025a , FALSE},
{FPB0          , 0x100067  , FALSE}, 
{DPLLB_CTRL    , 0x98040000, FALSE}, 
{PIPEBSRC      , 0x27f01df , FALSE},
{BCLRPAT_B     , 0x0       , FALSE},
{CRCCtrlColorBR, 0x0       , FALSE},
{CRCCtrlColorBG, 0x0       , FALSE},
{CRCCtrlColorBB, 0x0       , FALSE},
{PIPEBCONF     , 0x80000000, TRUE },
{VGACNTRL      , 0xa2c4008e, FALSE},
{DSPBSIZE      , 0x1df027f , TRUE },
{DSPBSTRIDE    , 0xa00     , TRUE },
{DSPBLINOFFSET , 0x0       , TRUE },
{DSPBCNTR      , 0x99000000, TRUE },
{LVDSPC        , 0xc0300300, FALSE},
{PFIT_CONTROL  , 0x80002668, FALSE}, 
{BLC_PWM_CTL   , 0x65ed65ed, FALSE},
{PIPEBSTAT     , 0x00020202, FALSE}, 
{PP_CONTROL    , 0x00000001, FALSE}, 
{0,0,0}

};


MODE_FORMAT LVDS_MODE_DATA_800_600[] = {
{PP_CONTROL    , 0xabcd0000, FALSE},
{PP_ON_DELAYS  , 0x25807d0 , TRUE },
{PP_OFF_DELAYS , 0x1f407d0 , FALSE},
{PP_DIVISOR    , 0x209d05  , FALSE},
{HTOTAL_B      , 0x4af03ff , FALSE},
{HBLANK_B      , 0x4af03ff , FALSE},
{HSYNC_B       , 0x44f042f , FALSE},
{VTOTAL_B      , 0x26e0257 , FALSE},
{VBLANK_B      , 0x26e0257 , FALSE},
{VSYNC_B       , 0x260025a , FALSE},
{FPB0          , 0x100067  , FALSE}, 
{DPLLB_CTRL    , 0x98040000, FALSE}, 
{PIPEBSRC      , 0x31f0257 , FALSE},
{BCLRPAT_B     , 0x0       , FALSE},
{CRCCtrlColorBR, 0x0       , FALSE},
{CRCCtrlColorBG, 0x0       , FALSE},
{CRCCtrlColorBB, 0x0       , FALSE},
{PIPEBCONF     , 0x80000000, TRUE },
{VGACNTRL      , 0xa2c4008e, FALSE},
{DSPBSIZE      , 0x257031f , TRUE },
{DSPBSTRIDE    , 0xc80     , TRUE },
{DSPBLINOFFSET , 0x0       , TRUE },
{DSPBCNTR      , 0x99000000, TRUE },
{LVDSPC        , 0xc0300300, FALSE},
{PFIT_CONTROL  , 0x80002668, FALSE}, 
{BLC_PWM_CTL   , 0x65ed65ed, FALSE},
{PIPEBSTAT     , 0x00020202, FALSE}, 
{PP_CONTROL    , 0x00000001, FALSE}, 
{0,0,0}

};

MODE_FORMAT LVDS_MODE_DATA_1024_768[] = {
{PP_CONTROL    , 0xabcd0000, FALSE},
{PP_ON_DELAYS  , 0x25807d0 , TRUE},
{PP_OFF_DELAYS , 0x1f407d0 , FALSE},
{PP_DIVISOR    , 0x209d05  , FALSE},
{HTOTAL_B      , 0x4af03ff , FALSE},
{HBLANK_B      , 0x4af03ff , FALSE},
{HSYNC_B       , 0x44f042f , FALSE},
{VTOTAL_B      , 0x26e0257 , FALSE},
{VBLANK_B      , 0x26e0257 , FALSE},
{VSYNC_B       , 0x260025a , FALSE},
{FPB0          , 0x100067  , FALSE}, 
{DPLLB_CTRL    , 0x98040000, FALSE}, 
{PIPEBSRC      , 0x3ff02fF , FALSE},
{BCLRPAT_B     , 0x0       , FALSE},
{CRCCtrlColorBR, 0x0       , FALSE},
{CRCCtrlColorBG, 0x0       , FALSE},
{CRCCtrlColorBB, 0x0       , FALSE},
{PIPEBCONF     , 0x80000000, TRUE },
{VGACNTRL      , 0xa2c4008e, FALSE},
{DSPBSIZE      , 0x25f03ff , TRUE },
{DSPBSTRIDE    , 0x1000    , TRUE },
{DSPBLINOFFSET , 0x0       , TRUE },
{DSPBCNTR      , 0x99000000, TRUE },
{LVDSPC        , 0xc0300300, FALSE},
{PFIT_CONTROL  , 0x80002668, FALSE}, 
{BLC_PWM_CTL   , 0x65ed65ed, FALSE},
{PIPEBSTAT     , 0x00020202, FALSE}, 
{PP_CONTROL    , 0x00000001, FALSE}, 
{0,0,0}

};

MODE_FORMAT LVDS_MODE_DATA_0_0[] = {
{PP_CONTROL    , 0xabcd0000, FALSE},
{PP_ON_DELAYS  , 0, FALSE},
{PP_OFF_DELAYS , 0, FALSE},
{PP_DIVISOR    , 0, FALSE},
{HTOTAL_B      , 0, FALSE},
{HBLANK_B      , 0, FALSE},
{HSYNC_B       , 0, FALSE},
{VTOTAL_B      , 0, FALSE},
{VBLANK_B      , 0, FALSE},
{VSYNC_B       , 0, FALSE},
{FPB0          , 0, FALSE}, 
{DPLLB_CTRL    , 0, FALSE}, 
{PIPEBSRC      , 0, FALSE},
{BCLRPAT_B     , 0, FALSE},
{CRCCtrlColorBR, 0, FALSE},
{CRCCtrlColorBG, 0, FALSE},
{CRCCtrlColorBB, 0, FALSE},
{PIPEBCONF     , 0, FALSE},
{VGACNTRL      , 0, FALSE},
{DSPBSIZE      , 0, FALSE},
{DSPBSTRIDE    , 0, FALSE},
{DSPBLINOFFSET , 0, FALSE},
{DSPBCNTR      , 0, FALSE},
{LVDSPC        , 0, FALSE},
{PFIT_CONTROL  , 0, FALSE}, 
{BLC_PWM_CTL   , 0, FALSE},
{PIPEBSTAT     , 0, FALSE}, 
{PP_CONTROL    , 0, FALSE}, 
{0,0,0}

};

INTEL_VIDEO_MODES mVideoModes[] = {
  {
    0,
    0,
    0,
    0,
    &DS_0_0_0_0[0],
    &LVDS_MODE_DATA_0_0[0]
  },
  //
  // Mode #0: turns off monitor
  //
  {
    640,
    480,
    32,
    60,
    &DS_640_480_32_60[0],
    &LVDS_MODE_DATA_640_480[0]
  },
  //
  // Mode #1: sets 640x480x32x60
  //
  {
    800,
    600,
    32,
    60,
    &DS_800_600_32_60[0],
    &LVDS_MODE_DATA_800_600[0]
  },
  //
  // Mode #2: sets 800x600x32x60
  //
  {
    1024,
    768,
    32,
    60,
    &DS_1024_768_32_60[0],
    &LVDS_MODE_DATA_1024_768[0]
  },
  //
  // Mode #3: sets 1024x768x32x60
  //
};
