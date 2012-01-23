/*
 * Copyright 2000 ATI Technologies Inc., Markham, Ontario, and
 *                VA Linux Systems Inc., Fremont, California.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL ATI, VA LINUX SYSTEMS AND/OR
 * THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * Authors:
 *   Kevin E. Martin <martin@xfree86.org>
 *   Rickard E. Faith <faith@valinux.com>
 *   Alan Hourihane <alanh@fairlite.demon.co.uk>
 *
 * References:
 *
 * !!!! FIXME !!!!
 *   RAGE 128 VR/ RAGE 128 GL Register Reference Manual (Technical
 *   Reference Manual P/N RRG-G04100-C Rev. 0.04), ATI Technologies: April
 *   1999.
 *
 * !!!! FIXME !!!!
 *   RAGE 128 Software Development Manual (Technical Reference Manual P/N
 *   SDK-G04000 Rev. 0.01), ATI Technologies: June 1999.
 *
 */

/* !!!! FIXME !!!!  NOTE: THIS FILE HAS BEEN CONVERTED FROM r128_reg.h
 * AND CONTAINS REGISTERS AND REGISTER DEFINITIONS THAT ARE NOT CORRECT
 * ON THE RADEON.  A FULL AUDIT OF THIS CODE IS NEEDED!  */

#ifndef _ATI_REG_H_
#define _ATI_REG_H_

#define ATI_DATATYPE_VQ				0
#define ATI_DATATYPE_CI4			1
#define ATI_DATATYPE_CI8			2
#define ATI_DATATYPE_ARGB1555			3
#define ATI_DATATYPE_RGB565			4
#define ATI_DATATYPE_RGB888			5
#define ATI_DATATYPE_ARGB8888			6
#define ATI_DATATYPE_RGB332			7
#define ATI_DATATYPE_Y8				8
#define ATI_DATATYPE_RGB8			9
#define ATI_DATATYPE_CI16			10
#define ATI_DATATYPE_VYUY_422			11
#define ATI_DATATYPE_YVYU_422			12
#define ATI_DATATYPE_AYUV_444			14
#define ATI_DATATYPE_ARGB4444			15

				/* Registers for 2D/Video/Overlay */
#define RADEON_ADAPTER_ID                   0x0f2c /* PCI */
#define RADEON_AGP_BASE                     0x0170
#define RADEON_AGP_CNTL                     0x0174
#       define RADEON_AGP_APER_SIZE_256MB   (0x00 << 0)
#       define RADEON_AGP_APER_SIZE_128MB   (0x20 << 0)
#       define RADEON_AGP_APER_SIZE_64MB    (0x30 << 0)
#       define RADEON_AGP_APER_SIZE_32MB    (0x38 << 0)
#       define RADEON_AGP_APER_SIZE_16MB    (0x3c << 0)
#       define RADEON_AGP_APER_SIZE_8MB     (0x3e << 0)
#       define RADEON_AGP_APER_SIZE_4MB     (0x3f << 0)
#       define RADEON_AGP_APER_SIZE_MASK    (0x3f << 0)
#define RADEON_STATUS_PCI_CONFIG            0x06
#       define RADEON_CAP_LIST              0x100000
#define RADEON_CAPABILITIES_PTR_PCI_CONFIG  0x34 /* offset in PCI config*/
#       define RADEON_CAP_PTR_MASK          0xfc /* mask off reserved bits of CAP_PTR */
#       define RADEON_CAP_ID_NULL           0x00 /* End of capability list */
#       define RADEON_CAP_ID_AGP            0x02 /* AGP capability ID */
#       define RADEON_CAP_ID_EXP            0x10 /* PCI Express */
#define RADEON_AGP_COMMAND                  0x0f60 /* PCI */
#define RADEON_AGP_COMMAND_PCI_CONFIG       0x0060 /* offset in PCI config*/
#       define RADEON_AGP_ENABLE            (1<<8)
#define RADEON_AGP_PLL_CNTL                 0x000b /* PLL */
#define RADEON_AGP_STATUS                   0x0f5c /* PCI */
#       define RADEON_AGP_1X_MODE           0x01
#       define RADEON_AGP_2X_MODE           0x02
#       define RADEON_AGP_4X_MODE           0x04
#       define RADEON_AGP_FW_MODE           0x10
#       define RADEON_AGP_MODE_MASK         0x17
#       define RADEON_AGPv3_MODE            0x08
#       define RADEON_AGPv3_4X_MODE         0x01
#       define RADEON_AGPv3_8X_MODE         0x02
#define RADEON_ATTRDR                       0x03c1 /* VGA */
#define RADEON_ATTRDW                       0x03c0 /* VGA */
#define RADEON_ATTRX                        0x03c0 /* VGA */
#define RADEON_AUX_WINDOW_HORZ_CNTL         0x02d8
#define RADEON_AUX_WINDOW_VERT_CNTL         0x02dc

#define RADEON_BASE_CODE                    0x0f0b
#define RADEON_BIOS_0_SCRATCH               0x0010
#       define RADEON_FP_PANEL_SCALABLE     (1 << 16)
#       define RADEON_FP_PANEL_SCALE_EN     (1 << 17)
#       define RADEON_FP_CHIP_SCALE_EN      (1 << 18)
#       define RADEON_DRIVER_BRIGHTNESS_EN  (1 << 26)
#       define RADEON_DISPLAY_ROT_MASK      (3 << 28)
#       define RADEON_DISPLAY_ROT_00        (0 << 28)
#       define RADEON_DISPLAY_ROT_90        (1 << 28)
#       define RADEON_DISPLAY_ROT_180       (2 << 28)
#       define RADEON_DISPLAY_ROT_270       (3 << 28)
#define RADEON_BIOS_1_SCRATCH               0x0014
#define RADEON_BIOS_2_SCRATCH               0x0018
#define RADEON_BIOS_3_SCRATCH               0x001c
#define RADEON_BIOS_4_SCRATCH               0x0020
#       define RADEON_CRT1_ATTACHED_MASK    (3 << 0)
#       define RADEON_CRT1_ATTACHED_MONO    (1 << 0)
#       define RADEON_CRT1_ATTACHED_COLOR   (2 << 0)
#       define RADEON_LCD1_ATTACHED         (1 << 2)
#       define RADEON_DFP1_ATTACHED         (1 << 3)
#       define RADEON_TV1_ATTACHED_MASK     (3 << 4)
#       define RADEON_TV1_ATTACHED_COMP     (1 << 4)
#       define RADEON_TV1_ATTACHED_SVIDEO   (2 << 4)
#       define RADEON_CRT2_ATTACHED_MASK    (3 << 8)
#       define RADEON_CRT2_ATTACHED_MONO    (1 << 8)
#       define RADEON_CRT2_ATTACHED_COLOR   (2 << 8)
#       define RADEON_DFP2_ATTACHED         (1 << 11)
#define RADEON_BIOS_5_SCRATCH               0x0024
#       define RADEON_LCD1_ON               (1 << 0)
#       define RADEON_CRT1_ON               (1 << 1)
#       define RADEON_TV1_ON                (1 << 2)
#       define RADEON_DFP1_ON               (1 << 3)
#       define RADEON_CRT2_ON               (1 << 5)
#       define RADEON_CV1_ON                (1 << 6)
#       define RADEON_DFP2_ON               (1 << 7)
#       define RADEON_LCD1_CRTC_MASK        (1 << 8)
#       define RADEON_LCD1_CRTC_SHIFT       8
#       define RADEON_CRT1_CRTC_MASK        (1 << 9)
#       define RADEON_CRT1_CRTC_SHIFT       9
#       define RADEON_TV1_CRTC_MASK         (1 << 10)
#       define RADEON_TV1_CRTC_SHIFT        10
#       define RADEON_DFP1_CRTC_MASK        (1 << 11)
#       define RADEON_DFP1_CRTC_SHIFT       11
#       define RADEON_CRT2_CRTC_MASK        (1 << 12)
#       define RADEON_CRT2_CRTC_SHIFT       12
#       define RADEON_CV1_CRTC_MASK         (1 << 13)
#       define RADEON_CV1_CRTC_SHIFT        13
#       define RADEON_DFP2_CRTC_MASK        (1 << 14)
#       define RADEON_DFP2_CRTC_SHIFT       14
#define RADEON_BIOS_6_SCRATCH               0x0028
#       define RADEON_ACC_MODE_CHANGE       (1 << 2)
#       define RADEON_EXT_DESKTOP_MODE      (1 << 3)
#       define RADEON_LCD_DPMS_ON           (1 << 20)
#       define RADEON_CRT_DPMS_ON           (1 << 21)
#       define RADEON_TV_DPMS_ON            (1 << 22)
#       define RADEON_DFP_DPMS_ON           (1 << 23)
#       define RADEON_DPMS_MASK             (3 << 24)
#       define RADEON_DPMS_ON               (0 << 24)
#       define RADEON_DPMS_STANDBY          (1 << 24)
#       define RADEON_DPMS_SUSPEND          (2 << 24)
#       define RADEON_DPMS_OFF              (3 << 24)
#       define RADEON_SCREEN_BLANKING       (1 << 26)
#       define RADEON_DRIVER_CRITICAL       (1 << 27)
#       define RADEON_DISPLAY_SWITCHING_DIS (1 << 30)
#define RADEON_BIOS_7_SCRATCH               0x002c
#       define RADEON_SYS_HOTKEY            (1 << 10)
#       define RADEON_DRV_LOADED            (1 << 12)
#define RADEON_BIOS_ROM                     0x0f30 /* PCI */
#define RADEON_BIST                         0x0f0f /* PCI */
#define RADEON_BRUSH_DATA0                  0x1480
#define RADEON_BRUSH_DATA1                  0x1484
#define RADEON_BRUSH_DATA10                 0x14a8
#define RADEON_BRUSH_DATA11                 0x14ac
#define RADEON_BRUSH_DATA12                 0x14b0
#define RADEON_BRUSH_DATA13                 0x14b4
#define RADEON_BRUSH_DATA14                 0x14b8
#define RADEON_BRUSH_DATA15                 0x14bc
#define RADEON_BRUSH_DATA16                 0x14c0
#define RADEON_BRUSH_DATA17                 0x14c4
#define RADEON_BRUSH_DATA18                 0x14c8
#define RADEON_BRUSH_DATA19                 0x14cc
#define RADEON_BRUSH_DATA2                  0x1488
#define RADEON_BRUSH_DATA20                 0x14d0
#define RADEON_BRUSH_DATA21                 0x14d4
#define RADEON_BRUSH_DATA22                 0x14d8
#define RADEON_BRUSH_DATA23                 0x14dc
#define RADEON_BRUSH_DATA24                 0x14e0
#define RADEON_BRUSH_DATA25                 0x14e4
#define RADEON_BRUSH_DATA26                 0x14e8
#define RADEON_BRUSH_DATA27                 0x14ec
#define RADEON_BRUSH_DATA28                 0x14f0
#define RADEON_BRUSH_DATA29                 0x14f4
#define RADEON_BRUSH_DATA3                  0x148c
#define RADEON_BRUSH_DATA30                 0x14f8
#define RADEON_BRUSH_DATA31                 0x14fc
#define RADEON_BRUSH_DATA32                 0x1500
#define RADEON_BRUSH_DATA33                 0x1504
#define RADEON_BRUSH_DATA34                 0x1508
#define RADEON_BRUSH_DATA35                 0x150c
#define RADEON_BRUSH_DATA36                 0x1510
#define RADEON_BRUSH_DATA37                 0x1514
#define RADEON_BRUSH_DATA38                 0x1518
#define RADEON_BRUSH_DATA39                 0x151c
#define RADEON_BRUSH_DATA4                  0x1490
#define RADEON_BRUSH_DATA40                 0x1520
#define RADEON_BRUSH_DATA41                 0x1524
#define RADEON_BRUSH_DATA42                 0x1528
#define RADEON_BRUSH_DATA43                 0x152c
#define RADEON_BRUSH_DATA44                 0x1530
#define RADEON_BRUSH_DATA45                 0x1534
#define RADEON_BRUSH_DATA46                 0x1538
#define RADEON_BRUSH_DATA47                 0x153c
#define RADEON_BRUSH_DATA48                 0x1540
#define RADEON_BRUSH_DATA49                 0x1544
#define RADEON_BRUSH_DATA5                  0x1494
#define RADEON_BRUSH_DATA50                 0x1548
#define RADEON_BRUSH_DATA51                 0x154c
#define RADEON_BRUSH_DATA52                 0x1550
#define RADEON_BRUSH_DATA53                 0x1554
#define RADEON_BRUSH_DATA54                 0x1558
#define RADEON_BRUSH_DATA55                 0x155c
#define RADEON_BRUSH_DATA56                 0x1560
#define RADEON_BRUSH_DATA57                 0x1564
#define RADEON_BRUSH_DATA58                 0x1568
#define RADEON_BRUSH_DATA59                 0x156c
#define RADEON_BRUSH_DATA6                  0x1498
#define RADEON_BRUSH_DATA60                 0x1570
#define RADEON_BRUSH_DATA61                 0x1574
#define RADEON_BRUSH_DATA62                 0x1578
#define RADEON_BRUSH_DATA63                 0x157c
#define RADEON_BRUSH_DATA7                  0x149c
#define RADEON_BRUSH_DATA8                  0x14a0
#define RADEON_BRUSH_DATA9                  0x14a4
#define RADEON_BRUSH_SCALE                  0x1470
#define RADEON_BRUSH_Y_X                    0x1474
#define RADEON_BUS_CNTL                     0x0030
#       define RADEON_BUS_MASTER_DIS         (1 << 6)
#       define RADEON_BUS_BIOS_DIS_ROM       (1 << 12)
#       define RADEON_BUS_RD_DISCARD_EN      (1 << 24)
#       define RADEON_BUS_RD_ABORT_EN        (1 << 25)
#       define RADEON_BUS_MSTR_DISCONNECT_EN (1 << 28)
#       define RADEON_BUS_WRT_BURST          (1 << 29)
#       define RADEON_BUS_READ_BURST         (1 << 30)
#define RADEON_BUS_CNTL1                    0x0034
#       define RADEON_BUS_WAIT_ON_LOCK_EN    (1 << 4)

#define RADEON_PCIE_INDEX                   0x0030
#define RADEON_PCIE_DATA                    0x0034
#define R600_PCIE_PORT_INDEX                0x0038
#define R600_PCIE_PORT_DATA                 0x003c
/* PCIE_LC_LINK_WIDTH_CNTL is PCIE on r1xx-r5xx, PCIE_PORT on r6xx-r7xx */
#define RADEON_PCIE_LC_LINK_WIDTH_CNTL      0xa2 /* PCIE */
#       define RADEON_PCIE_LC_LINK_WIDTH_SHIFT     0
#       define RADEON_PCIE_LC_LINK_WIDTH_MASK      0x7
#       define RADEON_PCIE_LC_LINK_WIDTH_X0        0
#       define RADEON_PCIE_LC_LINK_WIDTH_X1        1
#       define RADEON_PCIE_LC_LINK_WIDTH_X2        2
#       define RADEON_PCIE_LC_LINK_WIDTH_X4        3
#       define RADEON_PCIE_LC_LINK_WIDTH_X8        4
#       define RADEON_PCIE_LC_LINK_WIDTH_X12       5
#       define RADEON_PCIE_LC_LINK_WIDTH_X16       6
#       define RADEON_PCIE_LC_LINK_WIDTH_RD_SHIFT  4
#       define RADEON_PCIE_LC_LINK_WIDTH_RD_MASK   0x70
#       define R600_PCIE_LC_RECONFIG_ARC_MISSING_ESCAPE   (1 << 7)
#       define RADEON_PCIE_LC_RECONFIG_NOW         (1 << 8)
#       define RADEON_PCIE_LC_RECONFIG_LATER       (1 << 9)
#       define RADEON_PCIE_LC_SHORT_RECONFIG_EN    (1 << 10)
#       define R600_PCIE_LC_RENEGOTIATE_EN         (1 << 10)
#       define R600_PCIE_LC_SHORT_RECONFIG_EN      (1 << 11)
#define R600_TARGET_AND_CURRENT_PROFILE_INDEX      0x70c
#define R700_TARGET_AND_CURRENT_PROFILE_INDEX      0x66c

#define RADEON_CACHE_CNTL                   0x1724
#define RADEON_CACHE_LINE                   0x0f0c /* PCI */
#define RADEON_CAPABILITIES_ID              0x0f50 /* PCI */
#define RADEON_CAPABILITIES_PTR             0x0f34 /* PCI */
#define RADEON_CLK_PIN_CNTL                 0x0001 /* PLL */
#       define RADEON_DONT_USE_XTALIN       (1 << 4)
#       define RADEON_SCLK_DYN_START_CNTL   (1 << 15)
#define RADEON_CLOCK_CNTL_DATA              0x000c
#define RADEON_CLOCK_CNTL_INDEX             0x0008
#       define RADEON_PLL_WR_EN             (1 << 7)
#       define RADEON_PLL_DIV_SEL           (3 << 8)
#       define RADEON_PLL2_DIV_SEL_MASK     ~(3 << 8)
#define RADEON_M_SPLL_REF_FB_DIV            0x000a /* PLL */
#       define RADEON_M_SPLL_REF_DIV_MASK   0xff
#       define RADEON_M_SPLL_REF_DIV_SHIFT  0
#       define RADEON_MPLL_FB_DIV_MASK      0xff
#       define RADEON_MPLL_FB_DIV_SHIFT     8
#       define RADEON_SPLL_FB_DIV_MASK      0xff
#       define RADEON_SPLL_FB_DIV_SHIFT     16
#define RADEON_SPLL_CNTL                    0x000c /* PLL */
#       define RADEON_SPLL_SLEEP            (1 << 0)
#       define RADEON_SPLL_RESET            (1 << 1)
#       define RADEON_SPLL_PCP_MASK         0x7
#       define RADEON_SPLL_PCP_SHIFT        8
#       define RADEON_SPLL_PVG_MASK         0x7
#       define RADEON_SPLL_PVG_SHIFT        11
#       define RADEON_SPLL_PDC_MASK         0x3
#       define RADEON_SPLL_PDC_SHIFT        14
#define RADEON_CLK_PWRMGT_CNTL              0x0014 /* PLL */
#       define RADEON_ENGIN_DYNCLK_MODE     (1 << 12)
#       define RADEON_ACTIVE_HILO_LAT_MASK  (3 << 13)
#       define RADEON_ACTIVE_HILO_LAT_SHIFT 13
#       define RADEON_DISP_DYN_STOP_LAT_MASK (1 << 12)
#       define RADEON_MC_BUSY               (1 << 16)
#       define RADEON_DLL_READY             (1 << 19)
#       define RADEON_CG_NO1_DEBUG_0        (1 << 24)
#       define RADEON_CG_NO1_DEBUG_MASK     (0x1f << 24)
#       define RADEON_DYN_STOP_MODE_MASK    (7 << 21)
#       define RADEON_TVPLL_PWRMGT_OFF      (1 << 30)
#       define RADEON_TVCLK_TURNOFF         (1 << 31)
#define RADEON_PLL_PWRMGT_CNTL              0x0015 /* PLL */
#       define RADEON_TCL_BYPASS_DISABLE    (1 << 20)
#define RADEON_CLR_CMP_CLR_3D               0x1a24
#define RADEON_CLR_CMP_CLR_DST              0x15c8
#define RADEON_CLR_CMP_CLR_SRC              0x15c4
#define RADEON_CLR_CMP_CNTL                 0x15c0
#       define RADEON_SRC_CMP_EQ_COLOR      (4 <<  0)
#       define RADEON_SRC_CMP_NEQ_COLOR     (5 <<  0)
#       define RADEON_CLR_CMP_SRC_SOURCE    (1 << 24)
#define RADEON_CLR_CMP_MASK                 0x15cc
#       define RADEON_CLR_CMP_MSK           0xffffffff
#define RADEON_CLR_CMP_MASK_3D              0x1A28
#define RADEON_COMMAND                      0x0f04 /* PCI */
#define RADEON_COMPOSITE_SHADOW_ID          0x1a0c
#define RADEON_CONFIG_APER_0_BASE           0x0100
#define RADEON_CONFIG_APER_1_BASE           0x0104
#define RADEON_CONFIG_APER_SIZE             0x0108
#define RADEON_CONFIG_BONDS                 0x00e8
#define RADEON_CONFIG_CNTL                  0x00e0
#       define RADEON_CFG_ATI_REV_A11       (0   << 16)
#       define RADEON_CFG_ATI_REV_A12       (1   << 16)
#       define RADEON_CFG_ATI_REV_A13       (2   << 16)
#       define RADEON_CFG_ATI_REV_ID_MASK   (0xf << 16)
#define RADEON_CONFIG_MEMSIZE               0x00f8
#define RADEON_CONFIG_MEMSIZE_EMBEDDED      0x0114
#define RADEON_CONFIG_REG_1_BASE            0x010c
#define RADEON_CONFIG_REG_APER_SIZE         0x0110
#define RADEON_CONFIG_XSTRAP                0x00e4
#define RADEON_CONSTANT_COLOR_C             0x1d34
#       define RADEON_CONSTANT_COLOR_MASK   0x00ffffff
#       define RADEON_CONSTANT_COLOR_ONE    0x00ffffff
#       define RADEON_CONSTANT_COLOR_ZERO   0x00000000
#define RADEON_CRC_CMDFIFO_ADDR             0x0740
#define RADEON_CRC_CMDFIFO_DOUT             0x0744
#define RADEON_GRPH_BUFFER_CNTL             0x02f0
#       define RADEON_GRPH_START_REQ_MASK          (0x7f)
#       define RADEON_GRPH_START_REQ_SHIFT         0
#       define RADEON_GRPH_STOP_REQ_MASK           (0x7f<<8)
#       define RADEON_GRPH_STOP_REQ_SHIFT          8
#       define RADEON_GRPH_CRITICAL_POINT_MASK     (0x7f<<16)
#       define RADEON_GRPH_CRITICAL_POINT_SHIFT    16
#       define RADEON_GRPH_CRITICAL_CNTL           (1<<28)
#       define RADEON_GRPH_BUFFER_SIZE             (1<<29)
#       define RADEON_GRPH_CRITICAL_AT_SOF         (1<<30)
#       define RADEON_GRPH_STOP_CNTL               (1<<31)
#define RADEON_GRPH2_BUFFER_CNTL            0x03f0
#       define RADEON_GRPH2_START_REQ_MASK         (0x7f)
#       define RADEON_GRPH2_START_REQ_SHIFT         0
#       define RADEON_GRPH2_STOP_REQ_MASK          (0x7f<<8)
#       define RADEON_GRPH2_STOP_REQ_SHIFT         8
#       define RADEON_GRPH2_CRITICAL_POINT_MASK    (0x7f<<16)
#       define RADEON_GRPH2_CRITICAL_POINT_SHIFT   16
#       define RADEON_GRPH2_CRITICAL_CNTL          (1<<28)
#       define RADEON_GRPH2_BUFFER_SIZE            (1<<29)
#       define RADEON_GRPH2_CRITICAL_AT_SOF        (1<<30)
#       define RADEON_GRPH2_STOP_CNTL              (1<<31)
#define RADEON_CRTC_CRNT_FRAME              0x0214
#define RADEON_CRTC_EXT_CNTL                0x0054
#       define RADEON_CRTC_VGA_XOVERSCAN    (1 <<  0)
#       define RADEON_VGA_ATI_LINEAR        (1 <<  3)
#       define RADEON_XCRT_CNT_EN           (1 <<  6)
#       define RADEON_CRTC_HSYNC_DIS        (1 <<  8)
#       define RADEON_CRTC_VSYNC_DIS        (1 <<  9)
#       define RADEON_CRTC_DISPLAY_DIS      (1 << 10)
#       define RADEON_CRTC_SYNC_TRISTAT     (1 << 11)
#       define RADEON_CRTC_CRT_ON           (1 << 15)
#define RADEON_CRTC_EXT_CNTL_DPMS_BYTE      0x0055
#       define RADEON_CRTC_HSYNC_DIS_BYTE   (1 <<  0)
#       define RADEON_CRTC_VSYNC_DIS_BYTE   (1 <<  1)
#       define RADEON_CRTC_DISPLAY_DIS_BYTE (1 <<  2)
#define RADEON_CRTC_GEN_CNTL                0x0050
#       define RADEON_CRTC_DBL_SCAN_EN      (1 <<  0)
#       define RADEON_CRTC_INTERLACE_EN     (1 <<  1)
#       define RADEON_CRTC_CSYNC_EN         (1 <<  4)
#       define RADEON_CRTC_ICON_EN          (1 << 15)
#       define RADEON_CRTC_CUR_EN           (1 << 16)
#       define RADEON_CRTC_CUR_MODE_MASK    (7 << 20)
#       define RADEON_CRTC_EXT_DISP_EN      (1 << 24)
#       define RADEON_CRTC_EN               (1 << 25)
#       define RADEON_CRTC_DISP_REQ_EN_B    (1 << 26)
#define RADEON_CRTC2_GEN_CNTL               0x03f8
#       define RADEON_CRTC2_DBL_SCAN_EN     (1 <<  0)
#       define RADEON_CRTC2_INTERLACE_EN    (1 <<  1)
#       define RADEON_CRTC2_SYNC_TRISTAT    (1 <<  4)
#       define RADEON_CRTC2_HSYNC_TRISTAT   (1 <<  5)
#       define RADEON_CRTC2_VSYNC_TRISTAT   (1 <<  6)
#       define RADEON_CRTC2_CRT2_ON         (1 <<  7)
#       define RADEON_CRTC2_PIX_WIDTH_SHIFT 8
#       define RADEON_CRTC2_PIX_WIDTH_MASK  (0xf << 8)
#       define RADEON_CRTC2_ICON_EN         (1 << 15)
#       define RADEON_CRTC2_CUR_EN          (1 << 16)
#       define RADEON_CRTC2_CUR_MODE_MASK   (7 << 20)
#       define RADEON_CRTC2_DISP_DIS        (1 << 23)
#       define RADEON_CRTC2_EN              (1 << 25)
#       define RADEON_CRTC2_DISP_REQ_EN_B   (1 << 26)
#       define RADEON_CRTC2_CSYNC_EN        (1 << 27)
#       define RADEON_CRTC2_HSYNC_DIS       (1 << 28)
#       define RADEON_CRTC2_VSYNC_DIS       (1 << 29)
#define RADEON_CRTC_MORE_CNTL               0x27c
#       define RADEON_CRTC_AUTO_HORZ_CENTER_EN (1<<2)
#       define RADEON_CRTC_AUTO_VERT_CENTER_EN (1<<3)
#       define RADEON_CRTC_H_CUTOFF_ACTIVE_EN (1<<4)
#       define RADEON_CRTC_V_CUTOFF_ACTIVE_EN (1<<5)
#define RADEON_CRTC_GUI_TRIG_VLINE          0x0218
#       define RADEON_CRTC_GUI_TRIG_VLINE_START_SHIFT  0
#       define RADEON_CRTC_GUI_TRIG_VLINE_INV          (1 << 15)
#       define RADEON_CRTC_GUI_TRIG_VLINE_END_SHIFT    16
#       define RADEON_CRTC_GUI_TRIG_VLINE_STALL        (1 << 30)
#define RADEON_CRTC_H_SYNC_STRT_WID         0x0204
#       define RADEON_CRTC_H_SYNC_STRT_PIX        (0x07  <<  0)
#       define RADEON_CRTC_H_SYNC_STRT_CHAR       (0x3ff <<  3)
#       define RADEON_CRTC_H_SYNC_STRT_CHAR_SHIFT 3
#       define RADEON_CRTC_H_SYNC_WID             (0x3f  << 16)
#       define RADEON_CRTC_H_SYNC_WID_SHIFT       16
#       define RADEON_CRTC_H_SYNC_POL             (1     << 23)
#define RADEON_CRTC2_H_SYNC_STRT_WID        0x0304
#       define RADEON_CRTC2_H_SYNC_STRT_PIX        (0x07  <<  0)
#       define RADEON_CRTC2_H_SYNC_STRT_CHAR       (0x3ff <<  3)
#       define RADEON_CRTC2_H_SYNC_STRT_CHAR_SHIFT 3
#       define RADEON_CRTC2_H_SYNC_WID             (0x3f  << 16)
#       define RADEON_CRTC2_H_SYNC_WID_SHIFT       16
#       define RADEON_CRTC2_H_SYNC_POL             (1     << 23)
#define RADEON_CRTC_H_TOTAL_DISP            0x0200
#       define RADEON_CRTC_H_TOTAL          (0x03ff << 0)
#       define RADEON_CRTC_H_TOTAL_SHIFT    0
#       define RADEON_CRTC_H_DISP           (0x01ff << 16)
#       define RADEON_CRTC_H_DISP_SHIFT     16
#define RADEON_CRTC2_H_TOTAL_DISP           0x0300
#       define RADEON_CRTC2_H_TOTAL         (0x03ff << 0)
#       define RADEON_CRTC2_H_TOTAL_SHIFT   0
#       define RADEON_CRTC2_H_DISP          (0x01ff << 16)
#       define RADEON_CRTC2_H_DISP_SHIFT    16

#define RADEON_CRTC_OFFSET_RIGHT	    0x0220
#define RADEON_CRTC_OFFSET                  0x0224
#	define RADEON_CRTC_OFFSET__GUI_TRIG_OFFSET (1<<30)
#	define RADEON_CRTC_OFFSET__OFFSET_LOCK 	   (1<<31)

#define RADEON_CRTC2_OFFSET                 0x0324
#	define RADEON_CRTC2_OFFSET__GUI_TRIG_OFFSET (1<<30)
#	define RADEON_CRTC2_OFFSET__OFFSET_LOCK	    (1<<31)
#define RADEON_CRTC_OFFSET_CNTL             0x0228
#       define RADEON_CRTC_TILE_LINE_SHIFT              0
#       define RADEON_CRTC_TILE_LINE_RIGHT_SHIFT        4
#	define R300_CRTC_X_Y_MODE_EN_RIGHT		(1 << 6)
#	define R300_CRTC_MICRO_TILE_BUFFER_RIGHT_MASK   (3 << 7)
#	define R300_CRTC_MICRO_TILE_BUFFER_RIGHT_AUTO   (0 << 7)
#	define R300_CRTC_MICRO_TILE_BUFFER_RIGHT_SINGLE (1 << 7)
#	define R300_CRTC_MICRO_TILE_BUFFER_RIGHT_DOUBLE (2 << 7)
#	define R300_CRTC_MICRO_TILE_BUFFER_RIGHT_DIS    (3 << 7)
#	define R300_CRTC_X_Y_MODE_EN			(1 << 9)
#	define R300_CRTC_MICRO_TILE_BUFFER_MASK   	(3 << 10)
#	define R300_CRTC_MICRO_TILE_BUFFER_AUTO   	(0 << 10)
#	define R300_CRTC_MICRO_TILE_BUFFER_SINGLE 	(1 << 10)
#	define R300_CRTC_MICRO_TILE_BUFFER_DOUBLE 	(2 << 10)
#	define R300_CRTC_MICRO_TILE_BUFFER_DIS    	(3 << 10)
#	define R300_CRTC_MICRO_TILE_EN_RIGHT		(1 << 12)
#	define R300_CRTC_MICRO_TILE_EN			(1 << 13)
#	define R300_CRTC_MACRO_TILE_EN_RIGHT		(1 << 14)
#       define R300_CRTC_MACRO_TILE_EN                  (1 << 15)
#       define RADEON_CRTC_TILE_EN_RIGHT                (1 << 14)
#       define RADEON_CRTC_TILE_EN                      (1 << 15)
#       define RADEON_CRTC_OFFSET_FLIP_CNTL             (1 << 16)
#       define RADEON_CRTC_STEREO_OFFSET_EN             (1 << 17)

#define R300_CRTC_TILE_X0_Y0	            0x0350
#define R300_CRTC2_TILE_X0_Y0	            0x0358

#define RADEON_CRTC2_OFFSET_CNTL            0x0328
#       define RADEON_CRTC2_OFFSET_FLIP_CNTL (1 << 16)
#       define RADEON_CRTC2_TILE_EN         (1 << 15)
#define RADEON_CRTC_PITCH                   0x022c
#	define RADEON_CRTC_PITCH__SHIFT		 0
#	define RADEON_CRTC_PITCH__RIGHT_SHIFT	16

#define RADEON_CRTC2_PITCH                  0x032c
#define RADEON_CRTC_STATUS                  0x005c
#       define RADEON_CRTC_VBLANK_SAVE      (1 <<  1)
#       define RADEON_CRTC_VBLANK_SAVE_CLEAR  (1 <<  1)
#define RADEON_CRTC2_STATUS                  0x03fc
#       define RADEON_CRTC2_VBLANK_SAVE      (1 <<  1)
#       define RADEON_CRTC2_VBLANK_SAVE_CLEAR  (1 <<  1)
#define RADEON_CRTC_V_SYNC_STRT_WID         0x020c
#       define RADEON_CRTC_V_SYNC_STRT        (0x7ff <<  0)
#       define RADEON_CRTC_V_SYNC_STRT_SHIFT  0
#       define RADEON_CRTC_V_SYNC_WID         (0x1f  << 16)
#       define RADEON_CRTC_V_SYNC_WID_SHIFT   16
#       define RADEON_CRTC_V_SYNC_POL         (1     << 23)
#define RADEON_CRTC2_V_SYNC_STRT_WID        0x030c
#       define RADEON_CRTC2_V_SYNC_STRT       (0x7ff <<  0)
#       define RADEON_CRTC2_V_SYNC_STRT_SHIFT 0
#       define RADEON_CRTC2_V_SYNC_WID        (0x1f  << 16)
#       define RADEON_CRTC2_V_SYNC_WID_SHIFT  16
#       define RADEON_CRTC2_V_SYNC_POL        (1     << 23)
#define RADEON_CRTC_V_TOTAL_DISP            0x0208
#       define RADEON_CRTC_V_TOTAL          (0x07ff << 0)
#       define RADEON_CRTC_V_TOTAL_SHIFT    0
#       define RADEON_CRTC_V_DISP           (0x07ff << 16)
#       define RADEON_CRTC_V_DISP_SHIFT     16
#define RADEON_CRTC2_V_TOTAL_DISP           0x0308
#       define RADEON_CRTC2_V_TOTAL         (0x07ff << 0)
#       define RADEON_CRTC2_V_TOTAL_SHIFT   0
#       define RADEON_CRTC2_V_DISP          (0x07ff << 16)
#       define RADEON_CRTC2_V_DISP_SHIFT    16
#define RADEON_CRTC_VLINE_CRNT_VLINE        0x0210
#       define RADEON_CRTC_CRNT_VLINE_MASK  (0x7ff << 16)
#define RADEON_CRTC2_CRNT_FRAME             0x0314
#define RADEON_CRTC2_GUI_TRIG_VLINE         0x0318
#define RADEON_CRTC2_STATUS                 0x03fc
#define RADEON_CRTC2_VLINE_CRNT_VLINE       0x0310
#define RADEON_CRTC8_DATA                   0x03d5 /* VGA, 0x3b5 */
#define RADEON_CRTC8_IDX                    0x03d4 /* VGA, 0x3b4 */
#define RADEON_CUR_CLR0                     0x026c
#define RADEON_CUR_CLR1                     0x0270
#define RADEON_CUR_HORZ_VERT_OFF            0x0268
#define RADEON_CUR_HORZ_VERT_POSN           0x0264
#define RADEON_CUR_OFFSET                   0x0260
#       define RADEON_CUR_LOCK              (1 << 31)
#define RADEON_CUR2_CLR0                    0x036c
#define RADEON_CUR2_CLR1                    0x0370
#define RADEON_CUR2_HORZ_VERT_OFF           0x0368
#define RADEON_CUR2_HORZ_VERT_POSN          0x0364
#define RADEON_CUR2_OFFSET                  0x0360
#       define RADEON_CUR2_LOCK             (1 << 31)

#define RADEON_DAC_CNTL                     0x0058
#       define RADEON_DAC_RANGE_CNTL        (3 <<  0)
#       define RADEON_DAC_RANGE_CNTL_PS2    (2 <<  0)
#       define RADEON_DAC_RANGE_CNTL_MASK   0x03
#       define RADEON_DAC_BLANKING          (1 <<  2)
#       define RADEON_DAC_CMP_EN            (1 <<  3)
#       define RADEON_DAC_CMP_OUTPUT        (1 <<  7)
#       define RADEON_DAC_8BIT_EN           (1 <<  8)
#       define RADEON_DAC_TVO_EN            (1 << 10)
#       define RADEON_DAC_VGA_ADR_EN        (1 << 13)
#       define RADEON_DAC_PDWN              (1 << 15)
#       define RADEON_DAC_MASK_ALL          (0xff << 24)
#define RADEON_DAC_CNTL2                    0x007c
#       define RADEON_DAC2_TV_CLK_SEL       (0 <<  1)
#       define RADEON_DAC2_DAC_CLK_SEL      (1 <<  0)
#       define RADEON_DAC2_DAC2_CLK_SEL     (1 <<  1)
#       define RADEON_DAC2_PALETTE_ACC_CTL  (1 <<  5)
#       define RADEON_DAC2_CMP_EN           (1 <<  7)
#       define RADEON_DAC2_CMP_OUT_R        (1 <<  8)
#       define RADEON_DAC2_CMP_OUT_G        (1 <<  9)
#       define RADEON_DAC2_CMP_OUT_B        (1 << 10)
#       define RADEON_DAC2_CMP_OUTPUT       (1 << 11)
#define RADEON_DAC_EXT_CNTL                 0x0280
#       define RADEON_DAC2_FORCE_BLANK_OFF_EN (1 << 0)
#       define RADEON_DAC2_FORCE_DATA_EN      (1 << 1)
#       define RADEON_DAC_FORCE_BLANK_OFF_EN  (1 << 4)
#       define RADEON_DAC_FORCE_DATA_EN       (1 << 5)
#       define RADEON_DAC_FORCE_DATA_SEL_MASK (3 << 6)
#       define RADEON_DAC_FORCE_DATA_SEL_R    (0 << 6)
#       define RADEON_DAC_FORCE_DATA_SEL_G    (1 << 6)
#       define RADEON_DAC_FORCE_DATA_SEL_B    (2 << 6)
#       define RADEON_DAC_FORCE_DATA_SEL_RGB  (3 << 6)
#       define RADEON_DAC_FORCE_DATA_MASK   0x0003ff00
#       define RADEON_DAC_FORCE_DATA_SHIFT  8
#define RADEON_DAC_MACRO_CNTL               0x0d04
#       define RADEON_DAC_PDWN_R            (1 << 16)
#       define RADEON_DAC_PDWN_G            (1 << 17)
#       define RADEON_DAC_PDWN_B            (1 << 18)
#define RADEON_TV_DAC_CNTL                  0x088c
#       define RADEON_TV_DAC_NBLANK         (1 << 0)
#       define RADEON_TV_DAC_NHOLD          (1 << 1)
#       define RADEON_TV_DAC_PEDESTAL       (1 <<  2)
#       define RADEON_TV_MONITOR_DETECT_EN  (1 <<  4)
#       define RADEON_TV_DAC_CMPOUT         (1 <<  5)
#       define RADEON_TV_DAC_STD_MASK       (3 <<  8)
#       define RADEON_TV_DAC_STD_PAL        (0 <<  8)
#       define RADEON_TV_DAC_STD_NTSC       (1 <<  8)
#       define RADEON_TV_DAC_STD_PS2        (2 <<  8)
#       define RADEON_TV_DAC_STD_RS343      (3 <<  8)
#       define RADEON_TV_DAC_BGSLEEP        (1 <<  6)
#       define RADEON_TV_DAC_BGADJ_MASK     (0xf <<  16)
#       define RADEON_TV_DAC_BGADJ_SHIFT    16
#       define RADEON_TV_DAC_DACADJ_MASK    (0xf <<  20)
#       define RADEON_TV_DAC_DACADJ_SHIFT   20
#       define RADEON_TV_DAC_RDACPD         (1 <<  24)
#       define RADEON_TV_DAC_GDACPD         (1 <<  25)
#       define RADEON_TV_DAC_BDACPD         (1 <<  26)
#       define RADEON_TV_DAC_RDACDET        (1 << 29)
#       define RADEON_TV_DAC_GDACDET        (1 << 30)
#       define RADEON_TV_DAC_BDACDET        (1 << 31)
#       define R420_TV_DAC_DACADJ_MASK      (0x1f <<  20)
#       define R420_TV_DAC_RDACPD           (1 <<  25)
#       define R420_TV_DAC_GDACPD           (1 <<  26)
#       define R420_TV_DAC_BDACPD           (1 <<  27)
#       define R420_TV_DAC_TVENABLE         (1 <<  28)
#define RADEON_DISP_HW_DEBUG                0x0d14
#       define RADEON_CRT2_DISP1_SEL        (1 <<  5)
#define RADEON_DISP_OUTPUT_CNTL             0x0d64
#       define RADEON_DISP_DAC_SOURCE_MASK  0x03
#       define RADEON_DISP_DAC2_SOURCE_MASK  0x0c
#       define RADEON_DISP_DAC_SOURCE_CRTC2 0x01
#       define RADEON_DISP_DAC_SOURCE_RMX   0x02
#       define RADEON_DISP_DAC_SOURCE_LTU   0x03
#       define RADEON_DISP_DAC2_SOURCE_CRTC2 0x04
#       define RADEON_DISP_TVDAC_SOURCE_MASK  (0x03 << 2)
#       define RADEON_DISP_TVDAC_SOURCE_CRTC  0x0
#       define RADEON_DISP_TVDAC_SOURCE_CRTC2 (0x01 << 2)
#       define RADEON_DISP_TVDAC_SOURCE_RMX   (0x02 << 2)
#       define RADEON_DISP_TVDAC_SOURCE_LTU   (0x03 << 2)
#       define RADEON_DISP_TRANS_MATRIX_MASK  (0x03 << 4)
#       define RADEON_DISP_TRANS_MATRIX_ALPHA_MSB (0x00 << 4)
#       define RADEON_DISP_TRANS_MATRIX_GRAPHICS  (0x01 << 4)
#       define RADEON_DISP_TRANS_MATRIX_VIDEO     (0x02 << 4)
#       define RADEON_DISP_TV_SOURCE_CRTC   (1 << 16) /* crtc1 or crtc2 */
#       define RADEON_DISP_TV_SOURCE_LTU    (0 << 16) /* linear transform unit */
#define RADEON_DISP_TV_OUT_CNTL             0x0d6c
#       define RADEON_DISP_TV_PATH_SRC_CRTC2 (1 << 16)
#       define RADEON_DISP_TV_PATH_SRC_CRTC1 (0 << 16)
#define RADEON_DAC_CRC_SIG                  0x02cc
#define RADEON_DAC_DATA                     0x03c9 /* VGA */
#define RADEON_DAC_MASK                     0x03c6 /* VGA */
#define RADEON_DAC_R_INDEX                  0x03c7 /* VGA */
#define RADEON_DAC_W_INDEX                  0x03c8 /* VGA */
#define RADEON_DDA_CONFIG                   0x02e0
#define RADEON_DDA_ON_OFF                   0x02e4
#define RADEON_DEFAULT_OFFSET               0x16e0
#define RADEON_DEFAULT_PITCH                0x16e4
#define RADEON_DEFAULT_SC_BOTTOM_RIGHT      0x16e8
#       define RADEON_DEFAULT_SC_RIGHT_MAX  (0x1fff <<  0)
#       define RADEON_DEFAULT_SC_BOTTOM_MAX (0x1fff << 16)
#define RADEON_DESTINATION_3D_CLR_CMP_VAL   0x1820
#define RADEON_DESTINATION_3D_CLR_CMP_MSK   0x1824
#define RADEON_DEVICE_ID                    0x0f02 /* PCI */
#define RADEON_DISP_MISC_CNTL               0x0d00
#       define RADEON_SOFT_RESET_GRPH_PP    (1 << 0)
#define RADEON_DISP_MERGE_CNTL		  0x0d60
#       define RADEON_DISP_ALPHA_MODE_MASK  0x03
#       define RADEON_DISP_ALPHA_MODE_KEY   0
#       define RADEON_DISP_ALPHA_MODE_PER_PIXEL 1
#       define RADEON_DISP_ALPHA_MODE_GLOBAL 2
#       define RADEON_DISP_RGB_OFFSET_EN    (1 << 8)
#       define RADEON_DISP_GRPH_ALPHA_MASK  (0xff << 16)
#       define RADEON_DISP_OV0_ALPHA_MASK   (0xff << 24)
#	define RADEON_DISP_LIN_TRANS_BYPASS (0x01 << 9)
#define RADEON_DISP2_MERGE_CNTL		    0x0d68
#       define RADEON_DISP2_RGB_OFFSET_EN   (1 << 8)
#define RADEON_DISP_LIN_TRANS_GRPH_A        0x0d80
#define RADEON_DISP_LIN_TRANS_GRPH_B        0x0d84
#define RADEON_DISP_LIN_TRANS_GRPH_C        0x0d88
#define RADEON_DISP_LIN_TRANS_GRPH_D        0x0d8c
#define RADEON_DISP_LIN_TRANS_GRPH_E        0x0d90
#define RADEON_DISP_LIN_TRANS_GRPH_F        0x0d98
#define RADEON_DP_BRUSH_BKGD_CLR            0x1478
#define RADEON_DP_BRUSH_FRGD_CLR            0x147c
#define RADEON_DP_CNTL                      0x16c0
#       define RADEON_DST_X_LEFT_TO_RIGHT   (1 <<  0)
#       define RADEON_DST_Y_TOP_TO_BOTTOM   (1 <<  1)
#       define RADEON_DP_DST_TILE_LINEAR    (0 <<  3)
#       define RADEON_DP_DST_TILE_MACRO     (1 <<  3)
#       define RADEON_DP_DST_TILE_MICRO     (2 <<  3)
#       define RADEON_DP_DST_TILE_BOTH      (3 <<  3)
#define RADEON_DP_CNTL_XDIR_YDIR_YMAJOR     0x16d0
#       define RADEON_DST_Y_MAJOR             (1 <<  2)
#       define RADEON_DST_Y_DIR_TOP_TO_BOTTOM (1 << 15)
#       define RADEON_DST_X_DIR_LEFT_TO_RIGHT (1 << 31)
#define RADEON_DP_DATATYPE                  0x16c4
#       define RADEON_HOST_BIG_ENDIAN_EN    (1 << 29)
#define RADEON_DP_GUI_MASTER_CNTL           0x146c
#       define RADEON_GMC_SRC_PITCH_OFFSET_CNTL   (1    <<  0)
#       define RADEON_GMC_DST_PITCH_OFFSET_CNTL   (1    <<  1)
#       define RADEON_GMC_SRC_CLIPPING            (1    <<  2)
#       define RADEON_GMC_DST_CLIPPING            (1    <<  3)
#       define RADEON_GMC_BRUSH_DATATYPE_MASK     (0x0f <<  4)
#       define RADEON_GMC_BRUSH_8X8_MONO_FG_BG    (0    <<  4)
#       define RADEON_GMC_BRUSH_8X8_MONO_FG_LA    (1    <<  4)
#       define RADEON_GMC_BRUSH_1X8_MONO_FG_BG    (4    <<  4)
#       define RADEON_GMC_BRUSH_1X8_MONO_FG_LA    (5    <<  4)
#       define RADEON_GMC_BRUSH_32x1_MONO_FG_BG   (6    <<  4)
#       define RADEON_GMC_BRUSH_32x1_MONO_FG_LA   (7    <<  4)
#       define RADEON_GMC_BRUSH_32x32_MONO_FG_BG  (8    <<  4)
#       define RADEON_GMC_BRUSH_32x32_MONO_FG_LA  (9    <<  4)
#       define RADEON_GMC_BRUSH_8x8_COLOR         (10   <<  4)
#       define RADEON_GMC_BRUSH_1X8_COLOR         (12   <<  4)
#       define RADEON_GMC_BRUSH_SOLID_COLOR       (13   <<  4)
#       define RADEON_GMC_BRUSH_NONE              (15   <<  4)
#       define RADEON_GMC_DST_8BPP_CI             (2    <<  8)
#       define RADEON_GMC_DST_15BPP               (3    <<  8)
#       define RADEON_GMC_DST_16BPP               (4    <<  8)
#       define RADEON_GMC_DST_24BPP               (5    <<  8)
#       define RADEON_GMC_DST_32BPP               (6    <<  8)
#       define RADEON_GMC_DST_8BPP_RGB            (7    <<  8)
#       define RADEON_GMC_DST_Y8                  (8    <<  8)
#       define RADEON_GMC_DST_RGB8                (9    <<  8)
#       define RADEON_GMC_DST_VYUY                (11   <<  8)
#       define RADEON_GMC_DST_YVYU                (12   <<  8)
#       define RADEON_GMC_DST_AYUV444             (14   <<  8)
#       define RADEON_GMC_DST_ARGB4444            (15   <<  8)
#       define RADEON_GMC_DST_DATATYPE_MASK       (0x0f <<  8)
#       define RADEON_GMC_DST_DATATYPE_SHIFT      8
#       define RADEON_GMC_SRC_DATATYPE_MASK       (3    << 12)
#       define RADEON_GMC_SRC_DATATYPE_MONO_FG_BG (0    << 12)
#       define RADEON_GMC_SRC_DATATYPE_MONO_FG_LA (1    << 12)
#       define RADEON_GMC_SRC_DATATYPE_COLOR      (3    << 12)
#       define RADEON_GMC_BYTE_PIX_ORDER          (1    << 14)
#       define RADEON_GMC_BYTE_MSB_TO_LSB         (0    << 14)
#       define RADEON_GMC_BYTE_LSB_TO_MSB         (1    << 14)
#       define RADEON_GMC_CONVERSION_TEMP         (1    << 15)
#       define RADEON_GMC_CONVERSION_TEMP_6500    (0    << 15)
#       define RADEON_GMC_CONVERSION_TEMP_9300    (1    << 15)
#       define RADEON_GMC_ROP3_MASK               (0xff << 16)
#       define RADEON_DP_SRC_SOURCE_MASK          (7    << 24)
#       define RADEON_DP_SRC_SOURCE_MEMORY        (2    << 24)
#       define RADEON_DP_SRC_SOURCE_HOST_DATA     (3    << 24)
#       define RADEON_GMC_3D_FCN_EN               (1    << 27)
#       define RADEON_GMC_CLR_CMP_CNTL_DIS        (1    << 28)
#       define RADEON_GMC_AUX_CLIP_DIS            (1    << 29)
#       define RADEON_GMC_WR_MSK_DIS              (1    << 30)
#       define RADEON_GMC_LD_BRUSH_Y_X            (1    << 31)
#       define RADEON_ROP3_ZERO             0x00000000
#       define RADEON_ROP3_DSa              0x00880000
#       define RADEON_ROP3_SDna             0x00440000
#       define RADEON_ROP3_S                0x00cc0000
#       define RADEON_ROP3_DSna             0x00220000
#       define RADEON_ROP3_D                0x00aa0000
#       define RADEON_ROP3_DSx              0x00660000
#       define RADEON_ROP3_DSo              0x00ee0000
#       define RADEON_ROP3_DSon             0x00110000
#       define RADEON_ROP3_DSxn             0x00990000
#       define RADEON_ROP3_Dn               0x00550000
#       define RADEON_ROP3_SDno             0x00dd0000
#       define RADEON_ROP3_Sn               0x00330000
#       define RADEON_ROP3_DSno             0x00bb0000
#       define RADEON_ROP3_DSan             0x00770000
#       define RADEON_ROP3_ONE              0x00ff0000
#       define RADEON_ROP3_DPa              0x00a00000
#       define RADEON_ROP3_PDna             0x00500000
#       define RADEON_ROP3_P                0x00f00000
#       define RADEON_ROP3_DPna             0x000a0000
#       define RADEON_ROP3_D                0x00aa0000
#       define RADEON_ROP3_DPx              0x005a0000
#       define RADEON_ROP3_DPo              0x00fa0000
#       define RADEON_ROP3_DPon             0x00050000
#       define RADEON_ROP3_PDxn             0x00a50000
#       define RADEON_ROP3_PDno             0x00f50000
#       define RADEON_ROP3_Pn               0x000f0000
#       define RADEON_ROP3_DPno             0x00af0000
#       define RADEON_ROP3_DPan             0x005f0000
#define RADEON_DP_GUI_MASTER_CNTL_C         0x1c84
#define RADEON_DP_MIX                       0x16c8
#define RADEON_DP_SRC_BKGD_CLR              0x15dc
#define RADEON_DP_SRC_FRGD_CLR              0x15d8
#define RADEON_DP_WRITE_MASK                0x16cc
#define RADEON_DST_BRES_DEC                 0x1630
#define RADEON_DST_BRES_ERR                 0x1628
#define RADEON_DST_BRES_INC                 0x162c
#define RADEON_DST_BRES_LNTH                0x1634
#define RADEON_DST_BRES_LNTH_SUB            0x1638
#define RADEON_DST_HEIGHT                   0x1410
#define RADEON_DST_HEIGHT_WIDTH             0x143c
#define RADEON_DST_HEIGHT_WIDTH_8           0x158c
#define RADEON_DST_HEIGHT_WIDTH_BW          0x15b4
#define RADEON_DST_HEIGHT_Y                 0x15a0
#define RADEON_DST_LINE_START               0x1600
#define RADEON_DST_LINE_END                 0x1604
#define RADEON_DST_LINE_PATCOUNT            0x1608
#       define RADEON_BRES_CNTL_SHIFT       8
#define RADEON_DST_OFFSET                   0x1404
#define RADEON_DST_PITCH                    0x1408
#define RADEON_DST_PITCH_OFFSET             0x142c
#define RADEON_DST_PITCH_OFFSET_C           0x1c80
#       define RADEON_PITCH_SHIFT           21
#       define RADEON_DST_TILE_LINEAR       (0 << 30)
#       define RADEON_DST_TILE_MACRO        (1 << 30)
#       define RADEON_DST_TILE_MICRO        (2 << 30)
#       define RADEON_DST_TILE_BOTH         (3 << 30)
#define RADEON_DST_WIDTH                    0x140c
#define RADEON_DST_WIDTH_HEIGHT             0x1598
#define RADEON_DST_WIDTH_X                  0x1588
#define RADEON_DST_WIDTH_X_INCY             0x159c
#define RADEON_DST_X                        0x141c
#define RADEON_DST_X_SUB                    0x15a4
#define RADEON_DST_X_Y                      0x1594
#define RADEON_DST_Y                        0x1420
#define RADEON_DST_Y_SUB                    0x15a8
#define RADEON_DST_Y_X                      0x1438

#define RADEON_FCP_CNTL                     0x0910
#      define RADEON_FCP0_SRC_PCICLK             0
#      define RADEON_FCP0_SRC_PCLK               1
#      define RADEON_FCP0_SRC_PCLKb              2
#      define RADEON_FCP0_SRC_HREF               3
#      define RADEON_FCP0_SRC_GND                4
#      define RADEON_FCP0_SRC_HREFb              5
#define RADEON_FLUSH_1                      0x1704
#define RADEON_FLUSH_2                      0x1708
#define RADEON_FLUSH_3                      0x170c
#define RADEON_FLUSH_4                      0x1710
#define RADEON_FLUSH_5                      0x1714
#define RADEON_FLUSH_6                      0x1718
#define RADEON_FLUSH_7                      0x171c
#define RADEON_FOG_3D_TABLE_START           0x1810
#define RADEON_FOG_3D_TABLE_END             0x1814
#define RADEON_FOG_3D_TABLE_DENSITY         0x181c
#define RADEON_FOG_TABLE_INDEX              0x1a14
#define RADEON_FOG_TABLE_DATA               0x1a18
#define RADEON_FP_CRTC_H_TOTAL_DISP         0x0250
#define RADEON_FP_CRTC_V_TOTAL_DISP         0x0254
#       define RADEON_FP_CRTC_H_TOTAL_MASK      0x000003ff
#       define RADEON_FP_CRTC_H_DISP_MASK       0x01ff0000
#       define RADEON_FP_CRTC_V_TOTAL_MASK      0x00000fff
#       define RADEON_FP_CRTC_V_DISP_MASK       0x0fff0000
#       define RADEON_FP_H_SYNC_STRT_CHAR_MASK  0x00001ff8
#       define RADEON_FP_H_SYNC_WID_MASK        0x003f0000
#       define RADEON_FP_V_SYNC_STRT_MASK       0x00000fff
#       define RADEON_FP_V_SYNC_WID_MASK        0x001f0000
#       define RADEON_FP_CRTC_H_TOTAL_SHIFT     0x00000000
#       define RADEON_FP_CRTC_H_DISP_SHIFT      0x00000010
#       define RADEON_FP_CRTC_V_TOTAL_SHIFT     0x00000000
#       define RADEON_FP_CRTC_V_DISP_SHIFT      0x00000010
#       define RADEON_FP_H_SYNC_STRT_CHAR_SHIFT 0x00000003
#       define RADEON_FP_H_SYNC_WID_SHIFT       0x00000010
#       define RADEON_FP_V_SYNC_STRT_SHIFT      0x00000000
#       define RADEON_FP_V_SYNC_WID_SHIFT       0x00000010
#define RADEON_FP_GEN_CNTL                  0x0284
#       define RADEON_FP_FPON                  (1 <<  0)
#       define RADEON_FP_BLANK_EN              (1 <<  1)
#       define RADEON_FP_TMDS_EN               (1 <<  2)
#       define RADEON_FP_PANEL_FORMAT          (1 <<  3)
#       define RADEON_FP_EN_TMDS               (1 <<  7)
#       define RADEON_FP_DETECT_SENSE          (1 <<  8)
#       define R200_FP_SOURCE_SEL_MASK         (3 <<  10)
#       define R200_FP_SOURCE_SEL_CRTC1        (0 <<  10)
#       define R200_FP_SOURCE_SEL_CRTC2        (1 <<  10)
#       define R200_FP_SOURCE_SEL_RMX          (2 <<  10)
#       define R200_FP_SOURCE_SEL_TRANS        (3 <<  10)
#       define RADEON_FP_SEL_CRTC1             (0 << 13)
#       define RADEON_FP_SEL_CRTC2             (1 << 13)
#       define RADEON_FP_CRTC_DONT_SHADOW_HPAR (1 << 15)
#       define RADEON_FP_CRTC_DONT_SHADOW_VPAR (1 << 16)
#       define RADEON_FP_CRTC_DONT_SHADOW_HEND (1 << 17)
#       define RADEON_FP_CRTC_USE_SHADOW_VEND  (1 << 18)
#       define RADEON_FP_RMX_HVSYNC_CONTROL_EN (1 << 20)
#       define RADEON_FP_DFP_SYNC_SEL          (1 << 21)
#       define RADEON_FP_CRTC_LOCK_8DOT        (1 << 22)
#       define RADEON_FP_CRT_SYNC_SEL          (1 << 23)
#       define RADEON_FP_USE_SHADOW_EN         (1 << 24)
#       define RADEON_FP_CRT_SYNC_ALT          (1 << 26)
#define RADEON_FP2_GEN_CNTL                 0x0288
#       define RADEON_FP2_BLANK_EN             (1 <<  1)
#       define RADEON_FP2_ON                   (1 <<  2)
#       define RADEON_FP2_PANEL_FORMAT         (1 <<  3)
#       define RADEON_FP2_DETECT_SENSE         (1 <<  8)
#       define R200_FP2_SOURCE_SEL_MASK        (3 << 10)
#       define R200_FP2_SOURCE_SEL_CRTC1       (0 << 10)
#       define R200_FP2_SOURCE_SEL_CRTC2       (1 << 10)
#       define R200_FP2_SOURCE_SEL_RMX         (2 << 10)
#       define R200_FP2_SOURCE_SEL_TRANS_UNIT  (3 << 10)
#       define RADEON_FP2_SRC_SEL_MASK         (3 << 13)
#       define RADEON_FP2_SRC_SEL_CRTC2        (1 << 13)
#       define RADEON_FP2_FP_POL               (1 << 16)
#       define RADEON_FP2_LP_POL               (1 << 17)
#       define RADEON_FP2_SCK_POL              (1 << 18)
#       define RADEON_FP2_LCD_CNTL_MASK        (7 << 19)
#       define RADEON_FP2_PAD_FLOP_EN          (1 << 22)
#       define RADEON_FP2_CRC_EN               (1 << 23)
#       define RADEON_FP2_CRC_READ_EN          (1 << 24)
#       define RADEON_FP2_DVO_EN               (1 << 25)
#       define RADEON_FP2_DVO_RATE_SEL_SDR     (1 << 26)
#       define R200_FP2_DVO_RATE_SEL_SDR       (1 << 27)
#       define R200_FP2_DVO_CLOCK_MODE_SINGLE  (1 << 28)
#       define R300_FP2_DVO_DUAL_CHANNEL_EN    (1 << 29)
#define RADEON_FP_H_SYNC_STRT_WID           0x02c4
#define RADEON_FP_H2_SYNC_STRT_WID          0x03c4
#define RADEON_FP_HORZ_STRETCH              0x028c
#define RADEON_FP_HORZ2_STRETCH             0x038c
#       define RADEON_HORZ_STRETCH_RATIO_MASK 0xffff
#       define RADEON_HORZ_STRETCH_RATIO_MAX  4096
#       define RADEON_HORZ_PANEL_SIZE         (0x1ff   << 16)
#       define RADEON_HORZ_PANEL_SHIFT        16
#       define RADEON_HORZ_STRETCH_PIXREP     (0      << 25)
#       define RADEON_HORZ_STRETCH_BLEND      (1      << 26)
#       define RADEON_HORZ_STRETCH_ENABLE     (1      << 25)
#       define RADEON_HORZ_AUTO_RATIO         (1      << 27)
#       define RADEON_HORZ_FP_LOOP_STRETCH    (0x7    << 28)
#       define RADEON_HORZ_AUTO_RATIO_INC     (1      << 31)
#define RADEON_FP_HORZ_VERT_ACTIVE          0x0278
#define RADEON_FP_V_SYNC_STRT_WID           0x02c8
#define RADEON_FP_VERT_STRETCH              0x0290
#define RADEON_FP_V2_SYNC_STRT_WID          0x03c8
#define RADEON_FP_VERT2_STRETCH             0x0390
#       define RADEON_VERT_PANEL_SIZE          (0xfff << 12)
#       define RADEON_VERT_PANEL_SHIFT         12
#       define RADEON_VERT_STRETCH_RATIO_MASK  0xfff
#       define RADEON_VERT_STRETCH_RATIO_SHIFT 0
#       define RADEON_VERT_STRETCH_RATIO_MAX   4096
#       define RADEON_VERT_STRETCH_ENABLE      (1     << 25)
#       define RADEON_VERT_STRETCH_LINEREP     (0     << 26)
#       define RADEON_VERT_STRETCH_BLEND       (1     << 26)
#       define RADEON_VERT_AUTO_RATIO_EN       (1     << 27)
#	define RADEON_VERT_AUTO_RATIO_INC      (1     << 31)
#       define RADEON_VERT_STRETCH_RESERVED    0x71000000
#define RS400_FP_2ND_GEN_CNTL               0x0384
#       define RS400_FP_2ND_ON              (1 << 0)
#       define RS400_FP_2ND_BLANK_EN        (1 << 1)
#       define RS400_TMDS_2ND_EN            (1 << 2)
#       define RS400_PANEL_FORMAT_2ND       (1 << 3)
#       define RS400_FP_2ND_EN_TMDS         (1 << 7)
#       define RS400_FP_2ND_DETECT_SENSE    (1 << 8)
#       define RS400_FP_2ND_SOURCE_SEL_MASK        (3 << 10)
#       define RS400_FP_2ND_SOURCE_SEL_CRTC1       (0 << 10)
#       define RS400_FP_2ND_SOURCE_SEL_CRTC2       (1 << 10)
#       define RS400_FP_2ND_SOURCE_SEL_RMX         (2 << 10)
#       define RS400_FP_2ND_DETECT_EN       (1 << 12)
#       define RS400_HPD_2ND_SEL            (1 << 13)
#define RS400_FP2_2_GEN_CNTL                0x0388
#       define RS400_FP2_2_BLANK_EN         (1 << 1)
#       define RS400_FP2_2_ON               (1 << 2)
#       define RS400_FP2_2_PANEL_FORMAT     (1 << 3)
#       define RS400_FP2_2_DETECT_SENSE     (1 << 8)
#       define RS400_FP2_2_SOURCE_SEL_MASK        (3 << 10)
#       define RS400_FP2_2_SOURCE_SEL_CRTC1       (0 << 10)
#       define RS400_FP2_2_SOURCE_SEL_CRTC2       (1 << 10)
#       define RS400_FP2_2_SOURCE_SEL_RMX         (2 << 10)
#       define RS400_FP2_2_DVO2_EN          (1 << 25)
#define RS400_TMDS2_CNTL                    0x0394
#define RS400_TMDS2_TRANSMITTER_CNTL        0x03a4
#       define RS400_TMDS2_PLLEN            (1 << 0)
#       define RS400_TMDS2_PLLRST           (1 << 1)

#define RADEON_GEN_INT_CNTL                 0x0040
#define RADEON_GEN_INT_STATUS               0x0044
#       define RADEON_VSYNC_INT_AK          (1 <<  2)
#       define RADEON_VSYNC_INT             (1 <<  2)
#       define RADEON_VSYNC2_INT_AK         (1 <<  6)
#       define RADEON_VSYNC2_INT            (1 <<  6)
#define RADEON_GENENB                       0x03c3 /* VGA */
#define RADEON_GENFC_RD                     0x03ca /* VGA */
#define RADEON_GENFC_WT                     0x03da /* VGA, 0x03ba */
#define RADEON_GENMO_RD                     0x03cc /* VGA */
#define RADEON_GENMO_WT                     0x03c2 /* VGA */
#define RADEON_GENS0                        0x03c2 /* VGA */
#define RADEON_GENS1                        0x03da /* VGA, 0x03ba */
#define RADEON_GPIO_MONID                   0x0068 /* DDC interface via I2C */ /* DDC3 */
#define RADEON_GPIO_MONIDB                  0x006c
#define RADEON_GPIO_CRT2_DDC                0x006c
#define RADEON_GPIO_DVI_DDC                 0x0064 /* DDC2 */
#define RADEON_GPIO_VGA_DDC                 0x0060 /* DDC1 */
#       define RADEON_GPIO_A_0              (1 <<  0)
#       define RADEON_GPIO_A_1              (1 <<  1)
#       define RADEON_GPIO_Y_0              (1 <<  8)
#       define RADEON_GPIO_Y_1              (1 <<  9)
#       define RADEON_GPIO_Y_SHIFT_0        8
#       define RADEON_GPIO_Y_SHIFT_1        9
#       define RADEON_GPIO_EN_0             (1 << 16)
#       define RADEON_GPIO_EN_1             (1 << 17)
#       define RADEON_GPIO_MASK_0           (1 << 24) /*??*/
#       define RADEON_GPIO_MASK_1           (1 << 25) /*??*/
#define RADEON_GRPH8_DATA                   0x03cf /* VGA */
#define RADEON_GRPH8_IDX                    0x03ce /* VGA */
#define RADEON_GUI_SCRATCH_REG0             0x15e0
#define RADEON_GUI_SCRATCH_REG1             0x15e4
#define RADEON_GUI_SCRATCH_REG2             0x15e8
#define RADEON_GUI_SCRATCH_REG3             0x15ec
#define RADEON_GUI_SCRATCH_REG4             0x15f0
#define RADEON_GUI_SCRATCH_REG5             0x15f4

#define RADEON_HEADER                       0x0f0e /* PCI */
#define RADEON_HOST_DATA0                   0x17c0
#define RADEON_HOST_DATA1                   0x17c4
#define RADEON_HOST_DATA2                   0x17c8
#define RADEON_HOST_DATA3                   0x17cc
#define RADEON_HOST_DATA4                   0x17d0
#define RADEON_HOST_DATA5                   0x17d4
#define RADEON_HOST_DATA6                   0x17d8
#define RADEON_HOST_DATA7                   0x17dc
#define RADEON_HOST_DATA_LAST               0x17e0
#define RADEON_HOST_PATH_CNTL               0x0130
#       define RADEON_HDP_SOFT_RESET        (1 << 26)
#       define RADEON_HDP_APER_CNTL         (1 << 23)
#define RADEON_HTOTAL_CNTL                  0x0009 /* PLL */
#       define RADEON_HTOT_CNTL_VGA_EN      (1 << 28)
#define RADEON_HTOTAL2_CNTL                 0x002e /* PLL */

       /* Multimedia I2C bus */
#define RADEON_I2C_CNTL_0		    0x0090
#define RADEON_I2C_DONE                     (1 << 0)
#define RADEON_I2C_NACK                     (1 << 1)
#define RADEON_I2C_HALT                     (1 << 2)
#define RADEON_I2C_SOFT_RST                 (1 << 5)
#define RADEON_I2C_DRIVE_EN                 (1 << 6)
#define RADEON_I2C_DRIVE_SEL                (1 << 7)
#define RADEON_I2C_START                    (1 << 8)
#define RADEON_I2C_STOP                     (1 << 9)
#define RADEON_I2C_RECEIVE                  (1 << 10)
#define RADEON_I2C_ABORT                    (1 << 11)
#define RADEON_I2C_GO                       (1 << 12)
#define RADEON_I2C_CNTL_1                   0x0094
#define RADEON_I2C_SEL                      (1 << 16)
#define RADEON_I2C_EN                       (1 << 17)
#define RADEON_I2C_DATA			    0x0098

#define RADEON_DVI_I2C_CNTL_0		    0x02e0
#       define R200_DVI_I2C_PIN_SEL(x)      ((x) << 3)
#       define R200_SEL_DDC1                0 /* 0x60 - VGA_DDC */
#       define R200_SEL_DDC2                1 /* 0x64 - DVI_DDC */
#       define R200_SEL_DDC3                2 /* 0x68 - MONID_DDC */
#define RADEON_DVI_I2C_CNTL_1               0x02e4
#define RADEON_DVI_I2C_DATA		    0x02e8

#define RADEON_INTERRUPT_LINE               0x0f3c /* PCI */
#define RADEON_INTERRUPT_PIN                0x0f3d /* PCI */
#define RADEON_IO_BASE                      0x0f14 /* PCI */

#define RADEON_LATENCY                      0x0f0d /* PCI */
#define RADEON_LEAD_BRES_DEC                0x1608
#define RADEON_LEAD_BRES_LNTH               0x161c
#define RADEON_LEAD_BRES_LNTH_SUB           0x1624
#define RADEON_LVDS_GEN_CNTL                0x02d0
#       define RADEON_LVDS_ON               (1   <<  0)
#       define RADEON_LVDS_DISPLAY_DIS      (1   <<  1)
#       define RADEON_LVDS_PANEL_TYPE       (1   <<  2)
#       define RADEON_LVDS_PANEL_FORMAT     (1   <<  3)
#       define RADEON_LVDS_RST_FM           (1   <<  6)
#       define RADEON_LVDS_EN               (1   <<  7)
#       define RADEON_LVDS_BL_MOD_LEVEL_SHIFT 8
#       define RADEON_LVDS_BL_MOD_LEVEL_MASK (0xff << 8)
#       define RADEON_LVDS_BL_MOD_EN        (1   << 16)
#       define RADEON_LVDS_DIGON            (1   << 18)
#       define RADEON_LVDS_BLON             (1   << 19)
#       define RADEON_LVDS_SEL_CRTC2        (1   << 23)
#define RADEON_LVDS_PLL_CNTL                0x02d4
#       define RADEON_HSYNC_DELAY_SHIFT     28
#       define RADEON_HSYNC_DELAY_MASK      (0xf << 28)
#       define RADEON_LVDS_PLL_EN           (1   << 16)
#       define RADEON_LVDS_PLL_RESET        (1   << 17)
#       define R300_LVDS_SRC_SEL_MASK       (3   << 18)
#       define R300_LVDS_SRC_SEL_CRTC1      (0   << 18)
#       define R300_LVDS_SRC_SEL_CRTC2      (1   << 18)
#       define R300_LVDS_SRC_SEL_RMX        (2   << 18)

#define RADEON_MAX_LATENCY                  0x0f3f /* PCI */
#define RADEON_MC_AGP_LOCATION              0x014c
#define RADEON_MC_FB_LOCATION               0x0148
#define RADEON_DISPLAY_BASE_ADDR            0x23c
#define RADEON_DISPLAY2_BASE_ADDR           0x33c
#define RADEON_OV0_BASE_ADDR                0x43c
#define RADEON_NB_TOM                       0x15c
#define R300_MC_INIT_MISC_LAT_TIMER         0x180
#       define R300_MC_DISP0R_INIT_LAT_SHIFT 8
#       define R300_MC_DISP0R_INIT_LAT_MASK  0xf
#       define R300_MC_DISP1R_INIT_LAT_SHIFT 12
#       define R300_MC_DISP1R_INIT_LAT_MASK  0xf
#define RADEON_MCLK_CNTL                    0x0012 /* PLL */
#       define RADEON_FORCEON_MCLKA         (1 << 16)
#       define RADEON_FORCEON_MCLKB         (1 << 17)
#       define RADEON_FORCEON_YCLKA         (1 << 18)
#       define RADEON_FORCEON_YCLKB         (1 << 19)
#       define RADEON_FORCEON_MC            (1 << 20)
#       define RADEON_FORCEON_AIC           (1 << 21)
#       define R300_DISABLE_MC_MCLKA        (1 << 21)
#       define R300_DISABLE_MC_MCLKB        (1 << 21)
#define RADEON_MCLK_MISC                    0x001f /* PLL */
#       define RADEON_MC_MCLK_MAX_DYN_STOP_LAT (1 << 12)
#       define RADEON_IO_MCLK_MAX_DYN_STOP_LAT (1 << 13)
#       define RADEON_MC_MCLK_DYN_ENABLE    (1 << 14)
#       define RADEON_IO_MCLK_DYN_ENABLE    (1 << 15)
#define RADEON_LCD_GPIO_MASK                0x01a0
#define RADEON_GPIOPAD_EN                   0x01a0
#define RADEON_LCD_GPIO_Y_REG               0x01a4
#define RADEON_MDGPIO_A_REG                 0x01ac
#define RADEON_MDGPIO_EN_REG                0x01b0
#define RADEON_MDGPIO_MASK                  0x0198
#define RADEON_GPIOPAD_MASK                 0x0198
#define RADEON_GPIOPAD_A		    0x019c
#define RADEON_MDGPIO_Y_REG                 0x01b4
#define RADEON_MEM_ADDR_CONFIG              0x0148
#define RADEON_MEM_BASE                     0x0f10 /* PCI */
#define RADEON_MEM_CNTL                     0x0140
#       define RADEON_MEM_NUM_CHANNELS_MASK 0x01
#       define RADEON_MEM_USE_B_CH_ONLY     (1 <<  1)
#       define RV100_HALF_MODE              (1 <<  3)
#       define R300_MEM_NUM_CHANNELS_MASK   0x03
#       define R300_MEM_USE_CD_CH_ONLY      (1 <<  2)
#define RADEON_MEM_TIMING_CNTL              0x0144 /* EXT_MEM_CNTL */
#define RADEON_MEM_INIT_LAT_TIMER           0x0154
#define RADEON_MEM_INTF_CNTL                0x014c
#define RADEON_MEM_SDRAM_MODE_REG           0x0158
#       define RADEON_SDRAM_MODE_MASK       0xffff0000
#       define RADEON_B3MEM_RESET_MASK      0x6fffffff
#       define RADEON_MEM_CFG_TYPE_DDR      (1 << 30)
#define RADEON_MEM_STR_CNTL                 0x0150
#       define RADEON_MEM_PWRUP_COMPL_A     (1 <<  0)
#       define RADEON_MEM_PWRUP_COMPL_B     (1 <<  1)
#       define R300_MEM_PWRUP_COMPL_C       (1 <<  2)
#       define R300_MEM_PWRUP_COMPL_D       (1 <<  3)
#       define RADEON_MEM_PWRUP_COMPLETE    0x03
#       define R300_MEM_PWRUP_COMPLETE      0x0f
#define RADEON_MC_STATUS                    0x0150
#       define RADEON_MC_IDLE               (1 << 2)
#       define R300_MC_IDLE                 (1 << 4)
#define RADEON_MEM_VGA_RP_SEL               0x003c
#define RADEON_MEM_VGA_WP_SEL               0x0038
#define RADEON_MIN_GRANT                    0x0f3e /* PCI */
#define RADEON_MM_DATA                      0x0004
#define RADEON_MM_INDEX                     0x0000
#define RADEON_MPLL_CNTL                    0x000e /* PLL */
#define RADEON_MPP_TB_CONFIG                0x01c0 /* ? */
#define RADEON_MPP_GP_CONFIG                0x01c8 /* ? */
#define RADEON_SEPROM_CNTL1                 0x01c0
#       define RADEON_SCK_PRESCALE_SHIFT    24
#       define RADEON_SCK_PRESCALE_MASK     (0xff << 24)
#define R300_MC_IND_INDEX                   0x01f8
#       define R300_MC_IND_ADDR_MASK        0x3f
#       define R300_MC_IND_WR_EN            (1 << 8)
#define R300_MC_IND_DATA                    0x01fc
#define R300_MC_READ_CNTL_AB                0x017c
#       define R300_MEM_RBS_POSITION_A_MASK 0x03
#define R300_MC_READ_CNTL_CD_mcind	    0x24
#       define R300_MEM_RBS_POSITION_C_MASK 0x03

#define RADEON_N_VIF_COUNT                  0x0248

#define RADEON_OV0_AUTO_FLIP_CNTL           0x0470
#       define  RADEON_OV0_AUTO_FLIP_CNTL_SOFT_BUF_NUM        0x00000007
#       define  RADEON_OV0_AUTO_FLIP_CNTL_SOFT_REPEAT_FIELD   0x00000008
#       define  RADEON_OV0_AUTO_FLIP_CNTL_SOFT_BUF_ODD        0x00000010
#       define  RADEON_OV0_AUTO_FLIP_CNTL_IGNORE_REPEAT_FIELD 0x00000020
#       define  RADEON_OV0_AUTO_FLIP_CNTL_SOFT_EOF_TOGGLE     0x00000040
#       define  RADEON_OV0_AUTO_FLIP_CNTL_VID_PORT_SELECT     0x00000300
#       define  RADEON_OV0_AUTO_FLIP_CNTL_P1_FIRST_LINE_EVEN  0x00010000
#       define  RADEON_OV0_AUTO_FLIP_CNTL_SHIFT_EVEN_DOWN     0x00040000
#       define  RADEON_OV0_AUTO_FLIP_CNTL_SHIFT_ODD_DOWN      0x00080000
#       define  RADEON_OV0_AUTO_FLIP_CNTL_FIELD_POL_SOURCE    0x00800000

#define RADEON_OV0_COLOUR_CNTL              0x04E0
#define RADEON_OV0_DEINTERLACE_PATTERN      0x0474
#define RADEON_OV0_EXCLUSIVE_HORZ           0x0408
#       define  RADEON_EXCL_HORZ_START_MASK        0x000000ff
#       define  RADEON_EXCL_HORZ_END_MASK          0x0000ff00
#       define  RADEON_EXCL_HORZ_BACK_PORCH_MASK   0x00ff0000
#       define  RADEON_EXCL_HORZ_EXCLUSIVE_EN      0x80000000
#define RADEON_OV0_EXCLUSIVE_VERT           0x040C
#       define  RADEON_EXCL_VERT_START_MASK        0x000003ff
#       define  RADEON_EXCL_VERT_END_MASK          0x03ff0000
#define RADEON_OV0_FILTER_CNTL              0x04A0
#       define RADEON_FILTER_PROGRAMMABLE_COEF            0x0
#       define RADEON_FILTER_HC_COEF_HORZ_Y               0x1
#       define RADEON_FILTER_HC_COEF_HORZ_UV              0x2
#       define RADEON_FILTER_HC_COEF_VERT_Y               0x4
#       define RADEON_FILTER_HC_COEF_VERT_UV              0x8
#       define RADEON_FILTER_HARDCODED_COEF               0xf
#       define RADEON_FILTER_COEF_MASK                    0xf

#define RADEON_OV0_FOUR_TAP_COEF_0          0x04B0
#define RADEON_OV0_FOUR_TAP_COEF_1          0x04B4
#define RADEON_OV0_FOUR_TAP_COEF_2          0x04B8
#define RADEON_OV0_FOUR_TAP_COEF_3          0x04BC
#define RADEON_OV0_FOUR_TAP_COEF_4          0x04C0
#define RADEON_OV0_FLAG_CNTL                0x04DC
#define RADEON_OV0_GAMMA_000_00F            0x0d40
#define RADEON_OV0_GAMMA_010_01F            0x0d44
#define RADEON_OV0_GAMMA_020_03F            0x0d48
#define RADEON_OV0_GAMMA_040_07F            0x0d4c
#define RADEON_OV0_GAMMA_080_0BF            0x0e00
#define RADEON_OV0_GAMMA_0C0_0FF            0x0e04
#define RADEON_OV0_GAMMA_100_13F            0x0e08
#define RADEON_OV0_GAMMA_140_17F            0x0e0c
#define RADEON_OV0_GAMMA_180_1BF            0x0e10
#define RADEON_OV0_GAMMA_1C0_1FF            0x0e14
#define RADEON_OV0_GAMMA_200_23F            0x0e18
#define RADEON_OV0_GAMMA_240_27F            0x0e1c
#define RADEON_OV0_GAMMA_280_2BF            0x0e20
#define RADEON_OV0_GAMMA_2C0_2FF            0x0e24
#define RADEON_OV0_GAMMA_300_33F            0x0e28
#define RADEON_OV0_GAMMA_340_37F            0x0e2c
#define RADEON_OV0_GAMMA_380_3BF            0x0d50
#define RADEON_OV0_GAMMA_3C0_3FF            0x0d54
#define RADEON_OV0_GRAPHICS_KEY_CLR_LOW     0x04EC
#define RADEON_OV0_GRAPHICS_KEY_CLR_HIGH    0x04F0
#define RADEON_OV0_H_INC                    0x0480
#define RADEON_OV0_KEY_CNTL                 0x04F4
#       define  RADEON_VIDEO_KEY_FN_MASK    0x00000003L
#       define  RADEON_VIDEO_KEY_FN_FALSE   0x00000000L
#       define  RADEON_VIDEO_KEY_FN_TRUE    0x00000001L
#       define  RADEON_VIDEO_KEY_FN_EQ      0x00000002L
#       define  RADEON_VIDEO_KEY_FN_NE      0x00000003L
#       define  RADEON_GRAPHIC_KEY_FN_MASK  0x00000030L
#       define  RADEON_GRAPHIC_KEY_FN_FALSE 0x00000000L
#       define  RADEON_GRAPHIC_KEY_FN_TRUE  0x00000010L
#       define  RADEON_GRAPHIC_KEY_FN_EQ    0x00000020L
#       define  RADEON_GRAPHIC_KEY_FN_NE    0x00000030L
#       define  RADEON_CMP_MIX_MASK         0x00000100L
#       define  RADEON_CMP_MIX_OR           0x00000000L
#       define  RADEON_CMP_MIX_AND          0x00000100L
#define RADEON_OV0_LIN_TRANS_A              0x0d20
#define RADEON_OV0_LIN_TRANS_B              0x0d24
#define RADEON_OV0_LIN_TRANS_C              0x0d28
#define RADEON_OV0_LIN_TRANS_D              0x0d2c
#define RADEON_OV0_LIN_TRANS_E              0x0d30
#define RADEON_OV0_LIN_TRANS_F              0x0d34
#define RADEON_OV0_P1_BLANK_LINES_AT_TOP    0x0430
#       define  RADEON_P1_BLNK_LN_AT_TOP_M1_MASK   0x00000fffL
#       define  RADEON_P1_ACTIVE_LINES_M1          0x0fff0000L
#define RADEON_OV0_P1_H_ACCUM_INIT          0x0488
#define RADEON_OV0_P1_V_ACCUM_INIT          0x0428
#       define  RADEON_OV0_P1_MAX_LN_IN_PER_LN_OUT 0x00000003L
#       define  RADEON_OV0_P1_V_ACCUM_INIT_MASK    0x01ff8000L
#define RADEON_OV0_P1_X_START_END           0x0494
#define RADEON_OV0_P2_X_START_END           0x0498
#define RADEON_OV0_P23_BLANK_LINES_AT_TOP   0x0434
#       define  RADEON_P23_BLNK_LN_AT_TOP_M1_MASK  0x000007ffL
#       define  RADEON_P23_ACTIVE_LINES_M1         0x07ff0000L
#define RADEON_OV0_P23_H_ACCUM_INIT         0x048C
#define RADEON_OV0_P23_V_ACCUM_INIT         0x042C
#define RADEON_OV0_P3_X_START_END           0x049C
#define RADEON_OV0_REG_LOAD_CNTL            0x0410
#       define  RADEON_REG_LD_CTL_LOCK                 0x00000001L
#       define  RADEON_REG_LD_CTL_VBLANK_DURING_LOCK   0x00000002L
#       define  RADEON_REG_LD_CTL_STALL_GUI_UNTIL_FLIP 0x00000004L
#       define  RADEON_REG_LD_CTL_LOCK_READBACK        0x00000008L
#       define  RADEON_REG_LD_CTL_FLIP_READBACK        0x00000010L
#define RADEON_OV0_SCALE_CNTL               0x0420
#       define  RADEON_SCALER_HORZ_PICK_NEAREST    0x00000004L
#       define  RADEON_SCALER_VERT_PICK_NEAREST    0x00000008L
#       define  RADEON_SCALER_SIGNED_UV            0x00000010L
#       define  RADEON_SCALER_GAMMA_SEL_MASK       0x00000060L
#       define  RADEON_SCALER_GAMMA_SEL_BRIGHT     0x00000000L
#       define  RADEON_SCALER_GAMMA_SEL_G22        0x00000020L
#       define  RADEON_SCALER_GAMMA_SEL_G18        0x00000040L
#       define  RADEON_SCALER_GAMMA_SEL_G14        0x00000060L
#       define  RADEON_SCALER_COMCORE_SHIFT_UP_ONE 0x00000080L
#       define  RADEON_SCALER_SURFAC_FORMAT        0x00000f00L
#       define  RADEON_SCALER_SOURCE_15BPP         0x00000300L
#       define  RADEON_SCALER_SOURCE_16BPP         0x00000400L
#       define  RADEON_SCALER_SOURCE_32BPP         0x00000600L
#       define  RADEON_SCALER_SOURCE_YUV9          0x00000900L
#       define  RADEON_SCALER_SOURCE_YUV12         0x00000A00L
#       define  RADEON_SCALER_SOURCE_VYUY422       0x00000B00L
#       define  RADEON_SCALER_SOURCE_YVYU422       0x00000C00L
#       define  RADEON_SCALER_ADAPTIVE_DEINT       0x00001000L
#       define  RADEON_SCALER_TEMPORAL_DEINT       0x00002000L
#       define  RADEON_SCALER_CRTC_SEL             0x00004000L
#       define  RADEON_SCALER_SMART_SWITCH         0x00008000L
#       define  RADEON_SCALER_BURST_PER_PLANE      0x007F0000L
#       define  RADEON_SCALER_DOUBLE_BUFFER        0x01000000L
#       define  RADEON_SCALER_DIS_LIMIT            0x08000000L
#       define  RADEON_SCALER_LIN_TRANS_BYPASS     0x10000000L
#       define  RADEON_SCALER_INT_EMU              0x20000000L
#       define  RADEON_SCALER_ENABLE               0x40000000L
#       define  RADEON_SCALER_SOFT_RESET           0x80000000L
#define RADEON_OV0_STEP_BY                  0x0484
#define RADEON_OV0_TEST                     0x04F8
#define RADEON_OV0_V_INC                    0x0424
#define RADEON_OV0_VID_BUF_PITCH0_VALUE     0x0460
#define RADEON_OV0_VID_BUF_PITCH1_VALUE     0x0464
#define RADEON_OV0_VID_BUF0_BASE_ADRS       0x0440
#       define  RADEON_VIF_BUF0_PITCH_SEL          0x00000001L
#       define  RADEON_VIF_BUF0_TILE_ADRS          0x00000002L
#       define  RADEON_VIF_BUF0_BASE_ADRS_MASK     0x03fffff0L
#       define  RADEON_VIF_BUF0_1ST_LINE_LSBS_MASK 0x48000000L
#define RADEON_OV0_VID_BUF1_BASE_ADRS       0x0444
#       define  RADEON_VIF_BUF1_PITCH_SEL          0x00000001L
#       define  RADEON_VIF_BUF1_TILE_ADRS          0x00000002L
#       define  RADEON_VIF_BUF1_BASE_ADRS_MASK     0x03fffff0L
#       define  RADEON_VIF_BUF1_1ST_LINE_LSBS_MASK 0x48000000L
#define RADEON_OV0_VID_BUF2_BASE_ADRS       0x0448
#       define  RADEON_VIF_BUF2_PITCH_SEL          0x00000001L
#       define  RADEON_VIF_BUF2_TILE_ADRS          0x00000002L
#       define  RADEON_VIF_BUF2_BASE_ADRS_MASK     0x03fffff0L
#       define  RADEON_VIF_BUF2_1ST_LINE_LSBS_MASK 0x48000000L
#define RADEON_OV0_VID_BUF3_BASE_ADRS       0x044C
#define RADEON_OV0_VID_BUF4_BASE_ADRS       0x0450
#define RADEON_OV0_VID_BUF5_BASE_ADRS       0x0454
#define RADEON_OV0_VIDEO_KEY_CLR_HIGH       0x04E8
#define RADEON_OV0_VIDEO_KEY_CLR_LOW        0x04E4
#define RADEON_OV0_Y_X_START                0x0400
#define RADEON_OV0_Y_X_END                  0x0404
#define RADEON_OV1_Y_X_START                0x0600
#define RADEON_OV1_Y_X_END                  0x0604
#define RADEON_OVR_CLR                      0x0230
#define RADEON_OVR_WID_LEFT_RIGHT           0x0234
#define RADEON_OVR_WID_TOP_BOTTOM           0x0238

/* first capture unit */

#define RADEON_CAP0_BUF0_OFFSET           0x0920
#define RADEON_CAP0_BUF1_OFFSET           0x0924
#define RADEON_CAP0_BUF0_EVEN_OFFSET      0x0928
#define RADEON_CAP0_BUF1_EVEN_OFFSET      0x092C

#define RADEON_CAP0_BUF_PITCH             0x0930
#define RADEON_CAP0_V_WINDOW              0x0934
#define RADEON_CAP0_H_WINDOW              0x0938
#define RADEON_CAP0_VBI0_OFFSET           0x093C
#define RADEON_CAP0_VBI1_OFFSET           0x0940
#define RADEON_CAP0_VBI_V_WINDOW          0x0944
#define RADEON_CAP0_VBI_H_WINDOW          0x0948
#define RADEON_CAP0_PORT_MODE_CNTL        0x094C
#define RADEON_CAP0_TRIG_CNTL             0x0950
#define RADEON_CAP0_DEBUG                 0x0954
#define RADEON_CAP0_CONFIG                0x0958
#       define RADEON_CAP0_CONFIG_CONTINUOS          0x00000001
#       define RADEON_CAP0_CONFIG_START_FIELD_EVEN   0x00000002
#       define RADEON_CAP0_CONFIG_START_BUF_GET      0x00000004
#       define RADEON_CAP0_CONFIG_START_BUF_SET      0x00000008
#       define RADEON_CAP0_CONFIG_BUF_TYPE_ALT       0x00000010
#       define RADEON_CAP0_CONFIG_BUF_TYPE_FRAME     0x00000020
#       define RADEON_CAP0_CONFIG_ONESHOT_MODE_FRAME 0x00000040
#       define RADEON_CAP0_CONFIG_BUF_MODE_DOUBLE    0x00000080
#       define RADEON_CAP0_CONFIG_BUF_MODE_TRIPLE    0x00000100
#       define RADEON_CAP0_CONFIG_MIRROR_EN          0x00000200
#       define RADEON_CAP0_CONFIG_ONESHOT_MIRROR_EN  0x00000400
#       define RADEON_CAP0_CONFIG_VIDEO_SIGNED_UV    0x00000800
#       define RADEON_CAP0_CONFIG_ANC_DECODE_EN      0x00001000
#       define RADEON_CAP0_CONFIG_VBI_EN             0x00002000
#       define RADEON_CAP0_CONFIG_SOFT_PULL_DOWN_EN  0x00004000
#       define RADEON_CAP0_CONFIG_VIP_EXTEND_FLAG_EN 0x00008000
#       define RADEON_CAP0_CONFIG_FAKE_FIELD_EN      0x00010000
#       define RADEON_CAP0_CONFIG_ODD_ONE_MORE_LINE  0x00020000
#       define RADEON_CAP0_CONFIG_EVEN_ONE_MORE_LINE 0x00040000
#       define RADEON_CAP0_CONFIG_HORZ_DIVIDE_2      0x00080000
#       define RADEON_CAP0_CONFIG_HORZ_DIVIDE_4      0x00100000
#       define RADEON_CAP0_CONFIG_VERT_DIVIDE_2      0x00200000
#       define RADEON_CAP0_CONFIG_VERT_DIVIDE_4      0x00400000
#       define RADEON_CAP0_CONFIG_FORMAT_BROOKTREE   0x00000000
#       define RADEON_CAP0_CONFIG_FORMAT_CCIR656     0x00800000
#       define RADEON_CAP0_CONFIG_FORMAT_ZV          0x01000000
#       define RADEON_CAP0_CONFIG_FORMAT_VIP         0x01800000
#       define RADEON_CAP0_CONFIG_FORMAT_TRANSPORT   0x02000000
#       define RADEON_CAP0_CONFIG_HORZ_DECIMATOR     0x04000000
#       define RADEON_CAP0_CONFIG_VIDEO_IN_YVYU422   0x00000000
#       define RADEON_CAP0_CONFIG_VIDEO_IN_VYUY422   0x20000000
#       define RADEON_CAP0_CONFIG_VBI_DIVIDE_2       0x40000000
#       define RADEON_CAP0_CONFIG_VBI_DIVIDE_4       0x80000000
#define RADEON_CAP0_ANC_ODD_OFFSET        0x095C
#define RADEON_CAP0_ANC_EVEN_OFFSET       0x0960
#define RADEON_CAP0_ANC_H_WINDOW          0x0964
#define RADEON_CAP0_VIDEO_SYNC_TEST       0x0968
#define RADEON_CAP0_ONESHOT_BUF_OFFSET    0x096C
#define RADEON_CAP0_BUF_STATUS            0x0970
/* #define RADEON_CAP0_DWNSC_XRATIO       0x0978 */
/* #define RADEON_CAP0_XSHARPNESS                 0x097C */
#define RADEON_CAP0_VBI2_OFFSET           0x0980
#define RADEON_CAP0_VBI3_OFFSET           0x0984
#define RADEON_CAP0_ANC2_OFFSET           0x0988
#define RADEON_CAP0_ANC3_OFFSET           0x098C
#define RADEON_VID_BUFFER_CONTROL         0x0900

/* second capture unit */

#define RADEON_CAP1_BUF0_OFFSET           0x0990
#define RADEON_CAP1_BUF1_OFFSET           0x0994
#define RADEON_CAP1_BUF0_EVEN_OFFSET      0x0998
#define RADEON_CAP1_BUF1_EVEN_OFFSET      0x099C

#define RADEON_CAP1_BUF_PITCH             0x09A0
#define RADEON_CAP1_V_WINDOW              0x09A4
#define RADEON_CAP1_H_WINDOW              0x09A8
#define RADEON_CAP1_VBI_ODD_OFFSET        0x09AC
#define RADEON_CAP1_VBI_EVEN_OFFSET       0x09B0
#define RADEON_CAP1_VBI_V_WINDOW                  0x09B4
#define RADEON_CAP1_VBI_H_WINDOW                  0x09B8
#define RADEON_CAP1_PORT_MODE_CNTL        0x09BC
#define RADEON_CAP1_TRIG_CNTL             0x09C0
#define RADEON_CAP1_DEBUG                         0x09C4
#define RADEON_CAP1_CONFIG                0x09C8
#define RADEON_CAP1_ANC_ODD_OFFSET        0x09CC
#define RADEON_CAP1_ANC_EVEN_OFFSET       0x09D0
#define RADEON_CAP1_ANC_H_WINDOW                  0x09D4
#define RADEON_CAP1_VIDEO_SYNC_TEST       0x09D8
#define RADEON_CAP1_ONESHOT_BUF_OFFSET    0x09DC
#define RADEON_CAP1_BUF_STATUS            0x09E0
#define RADEON_CAP1_DWNSC_XRATIO                  0x09E8
#define RADEON_CAP1_XSHARPNESS            0x09EC

/* misc multimedia registers */

#define RADEON_IDCT_RUNS                  0x1F80
#define RADEON_IDCT_LEVELS                0x1F84
#define RADEON_IDCT_CONTROL               0x1FBC
#define RADEON_IDCT_AUTH_CONTROL          0x1F88
#define RADEON_IDCT_AUTH                  0x1F8C

#define RADEON_P2PLL_CNTL                   0x002a /* P2PLL */
#       define RADEON_P2PLL_RESET                (1 <<  0)
#       define RADEON_P2PLL_SLEEP                (1 <<  1)
#       define RADEON_P2PLL_PVG_MASK             (7 << 11)
#       define RADEON_P2PLL_PVG_SHIFT            11
#       define RADEON_P2PLL_ATOMIC_UPDATE_EN     (1 << 16)
#       define RADEON_P2PLL_VGA_ATOMIC_UPDATE_EN (1 << 17)
#       define RADEON_P2PLL_ATOMIC_UPDATE_VSYNC  (1 << 18)
#define RADEON_P2PLL_DIV_0                  0x002c
#       define RADEON_P2PLL_FB0_DIV_MASK    0x07ff
#       define RADEON_P2PLL_POST0_DIV_MASK  0x00070000
#define RADEON_P2PLL_REF_DIV                0x002B /* PLL */
#       define RADEON_P2PLL_REF_DIV_MASK    0x03ff
#       define RADEON_P2PLL_ATOMIC_UPDATE_R (1 << 15) /* same as _W */
#       define RADEON_P2PLL_ATOMIC_UPDATE_W (1 << 15) /* same as _R */
#       define R300_PPLL_REF_DIV_ACC_MASK   (0x3ff << 18)
#       define R300_PPLL_REF_DIV_ACC_SHIFT  18
#define RADEON_PALETTE_DATA                 0x00b4
#define RADEON_PALETTE_30_DATA              0x00b8
#define RADEON_PALETTE_INDEX                0x00b0
#define RADEON_PCI_GART_PAGE                0x017c
#define RADEON_PIXCLKS_CNTL                 0x002d
#       define RADEON_PIX2CLK_SRC_SEL_MASK     0x03
#       define RADEON_PIX2CLK_SRC_SEL_CPUCLK   0x00
#       define RADEON_PIX2CLK_SRC_SEL_PSCANCLK 0x01
#       define RADEON_PIX2CLK_SRC_SEL_BYTECLK  0x02
#       define RADEON_PIX2CLK_SRC_SEL_P2PLLCLK 0x03
#       define RADEON_PIX2CLK_ALWAYS_ONb       (1<<6)
#       define RADEON_PIX2CLK_DAC_ALWAYS_ONb   (1<<7)
#       define RADEON_PIXCLK_TV_SRC_SEL        (1 << 8)
#       define RADEON_DISP_TVOUT_PIXCLK_TV_ALWAYS_ONb (1 << 9)
#       define R300_DVOCLK_ALWAYS_ONb          (1 << 10)
#       define RADEON_PIXCLK_BLEND_ALWAYS_ONb  (1 << 11)
#       define RADEON_PIXCLK_GV_ALWAYS_ONb     (1 << 12)
#       define RADEON_PIXCLK_DIG_TMDS_ALWAYS_ONb (1 << 13)
#       define R300_PIXCLK_DVO_ALWAYS_ONb      (1 << 13)
#       define RADEON_PIXCLK_LVDS_ALWAYS_ONb   (1 << 14)
#       define RADEON_PIXCLK_TMDS_ALWAYS_ONb   (1 << 15)
#       define R300_PIXCLK_TRANS_ALWAYS_ONb    (1 << 16)
#       define R300_PIXCLK_TVO_ALWAYS_ONb      (1 << 17)
#       define R300_P2G2CLK_ALWAYS_ONb         (1 << 18)
#       define R300_P2G2CLK_DAC_ALWAYS_ONb     (1 << 19)
#       define R300_DISP_DAC_PIXCLK_DAC2_BLANK_OFF (1 << 23)
#define RADEON_PLANE_3D_MASK_C              0x1d44
#define RADEON_PLL_TEST_CNTL                0x0013 /* PLL */
#       define RADEON_PLL_MASK_READ_B          (1 << 9)
#define RADEON_PMI_CAP_ID                   0x0f5c /* PCI */
#define RADEON_PMI_DATA                     0x0f63 /* PCI */
#define RADEON_PMI_NXT_CAP_PTR              0x0f5d /* PCI */
#define RADEON_PMI_PMC_REG                  0x0f5e /* PCI */
#define RADEON_PMI_PMCSR_REG                0x0f60 /* PCI */
#define RADEON_PMI_REGISTER                 0x0f5c /* PCI */
#define RADEON_PPLL_CNTL                    0x0002 /* PLL */
#       define RADEON_PPLL_RESET                (1 <<  0)
#       define RADEON_PPLL_SLEEP                (1 <<  1)
#       define RADEON_PPLL_PVG_MASK             (7 << 11)
#       define RADEON_PPLL_PVG_SHIFT            11
#       define RADEON_PPLL_ATOMIC_UPDATE_EN     (1 << 16)
#       define RADEON_PPLL_VGA_ATOMIC_UPDATE_EN (1 << 17)
#       define RADEON_PPLL_ATOMIC_UPDATE_VSYNC  (1 << 18)
#define RADEON_PPLL_DIV_0                   0x0004 /* PLL */
#define RADEON_PPLL_DIV_1                   0x0005 /* PLL */
#define RADEON_PPLL_DIV_2                   0x0006 /* PLL */
#define RADEON_PPLL_DIV_3                   0x0007 /* PLL */
#       define RADEON_PPLL_FB3_DIV_MASK     0x07ff
#       define RADEON_PPLL_POST3_DIV_MASK   0x00070000
#define RADEON_PPLL_REF_DIV                 0x0003 /* PLL */
#       define RADEON_PPLL_REF_DIV_MASK     0x03ff
#       define RADEON_PPLL_ATOMIC_UPDATE_R  (1 << 15) /* same as _W */
#       define RADEON_PPLL_ATOMIC_UPDATE_W  (1 << 15) /* same as _R */
#define RADEON_PWR_MNGMT_CNTL_STATUS        0x0f60 /* PCI */

#define RADEON_RBBM_GUICNTL                 0x172c
#       define RADEON_HOST_DATA_SWAP_NONE   (0 << 0)
#       define RADEON_HOST_DATA_SWAP_16BIT  (1 << 0)
#       define RADEON_HOST_DATA_SWAP_32BIT  (2 << 0)
#       define RADEON_HOST_DATA_SWAP_HDW    (3 << 0)
#define RADEON_RBBM_SOFT_RESET              0x00f0
#       define RADEON_SOFT_RESET_CP         (1 <<  0)
#       define RADEON_SOFT_RESET_HI         (1 <<  1)
#       define RADEON_SOFT_RESET_SE         (1 <<  2)
#       define RADEON_SOFT_RESET_RE         (1 <<  3)
#       define RADEON_SOFT_RESET_PP         (1 <<  4)
#       define RADEON_SOFT_RESET_E2         (1 <<  5)
#       define RADEON_SOFT_RESET_RB         (1 <<  6)
#       define RADEON_SOFT_RESET_HDP        (1 <<  7)
#define RADEON_RBBM_STATUS                  0x0e40
#       define RADEON_RBBM_FIFOCNT_MASK     0x007f
#       define RADEON_RBBM_ACTIVE           (1 << 31)
#define RADEON_RB2D_DSTCACHE_CTLSTAT        0x342c
#       define RADEON_RB2D_DC_FLUSH         (3 << 0)
#       define RADEON_RB2D_DC_FREE          (3 << 2)
#       define RADEON_RB2D_DC_FLUSH_ALL     0xf
#       define RADEON_RB2D_DC_BUSY          (1 << 31)
#define RADEON_RB2D_DSTCACHE_MODE           0x3428
#define RADEON_DSTCACHE_CTLSTAT             0x1714

#define RADEON_RB3D_ZCACHE_MODE             0x3250
#define RADEON_RB3D_ZCACHE_CTLSTAT          0x3254
#       define RADEON_RB3D_ZC_FLUSH_ALL     0x5
#define RADEON_RB3D_DSTCACHE_MODE           0x3258
# define RADEON_RB3D_DC_CACHE_ENABLE            (0)
# define RADEON_RB3D_DC_2D_CACHE_DISABLE        (1)
# define RADEON_RB3D_DC_3D_CACHE_DISABLE        (2)
# define RADEON_RB3D_DC_CACHE_DISABLE           (3)
# define RADEON_RB3D_DC_2D_CACHE_LINESIZE_128   (1 << 2)
# define RADEON_RB3D_DC_3D_CACHE_LINESIZE_128   (2 << 2)
# define RADEON_RB3D_DC_2D_CACHE_AUTOFLUSH      (1 << 8)
# define RADEON_RB3D_DC_3D_CACHE_AUTOFLUSH      (2 << 8)
# define R200_RB3D_DC_2D_CACHE_AUTOFREE         (1 << 10)
# define R200_RB3D_DC_3D_CACHE_AUTOFREE         (2 << 10)
# define RADEON_RB3D_DC_FORCE_RMW               (1 << 16)
# define RADEON_RB3D_DC_DISABLE_RI_FILL         (1 << 24)
# define RADEON_RB3D_DC_DISABLE_RI_READ         (1 << 25)

#define RADEON_RB3D_DSTCACHE_CTLSTAT            0x325C
# define RADEON_RB3D_DC_FLUSH                   (3 << 0)
# define RADEON_RB3D_DC_FREE                    (3 << 2)
# define RADEON_RB3D_DC_FLUSH_ALL               0xf
# define RADEON_RB3D_DC_BUSY                    (1 << 31)

#define RADEON_REG_BASE                     0x0f18 /* PCI */
#define RADEON_REGPROG_INF                  0x0f09 /* PCI */
#define RADEON_REVISION_ID                  0x0f08 /* PCI */

#define RADEON_SC_BOTTOM                    0x164c
#define RADEON_SC_BOTTOM_RIGHT              0x16f0
#define RADEON_SC_BOTTOM_RIGHT_C            0x1c8c
#define RADEON_SC_LEFT                      0x1640
#define RADEON_SC_RIGHT                     0x1644
#define RADEON_SC_TOP                       0x1648
#define RADEON_SC_TOP_LEFT                  0x16ec
#define RADEON_SC_TOP_LEFT_C                0x1c88
#       define RADEON_SC_SIGN_MASK_LO       0x8000
#       define RADEON_SC_SIGN_MASK_HI       0x80000000
#define RADEON_SCLK_CNTL                    0x000d /* PLL */
#       define RADEON_SCLK_SRC_SEL_MASK     0x0007
#       define RADEON_DYN_STOP_LAT_MASK     0x00007ff8
#       define RADEON_CP_MAX_DYN_STOP_LAT   0x0008
#       define RADEON_SCLK_FORCEON_MASK     0xffff8000
#       define RADEON_SCLK_FORCE_DISP2      (1<<15)
#       define RADEON_SCLK_FORCE_CP         (1<<16)
#       define RADEON_SCLK_FORCE_HDP        (1<<17)
#       define RADEON_SCLK_FORCE_DISP1      (1<<18)
#       define RADEON_SCLK_FORCE_TOP        (1<<19)
#       define RADEON_SCLK_FORCE_E2         (1<<20)
#       define RADEON_SCLK_FORCE_SE         (1<<21)
#       define RADEON_SCLK_FORCE_IDCT       (1<<22)
#       define RADEON_SCLK_FORCE_VIP        (1<<23)
#       define RADEON_SCLK_FORCE_RE         (1<<24)
#       define RADEON_SCLK_FORCE_PB         (1<<25)
#       define RADEON_SCLK_FORCE_TAM        (1<<26)
#       define RADEON_SCLK_FORCE_TDM        (1<<27)
#       define RADEON_SCLK_FORCE_RB         (1<<28)
#       define RADEON_SCLK_FORCE_TV_SCLK    (1<<29)
#       define RADEON_SCLK_FORCE_SUBPIC     (1<<30)
#       define RADEON_SCLK_FORCE_OV0        (1<<31)
#       define R300_SCLK_FORCE_VAP          (1<<21)
#       define R300_SCLK_FORCE_SR           (1<<25)
#       define R300_SCLK_FORCE_PX           (1<<26)
#       define R300_SCLK_FORCE_TX           (1<<27)
#       define R300_SCLK_FORCE_US           (1<<28)
#       define R300_SCLK_FORCE_SU           (1<<30)
#define R300_SCLK_CNTL2                     0x1e   /* PLL */
#       define R300_SCLK_TCL_MAX_DYN_STOP_LAT (1<<10)
#       define R300_SCLK_GA_MAX_DYN_STOP_LAT  (1<<11)
#       define R300_SCLK_CBA_MAX_DYN_STOP_LAT (1<<12)
#       define R300_SCLK_FORCE_TCL          (1<<13)
#       define R300_SCLK_FORCE_CBA          (1<<14)
#       define R300_SCLK_FORCE_GA           (1<<15)
#define RADEON_SCLK_MORE_CNTL               0x0035 /* PLL */
#       define RADEON_SCLK_MORE_MAX_DYN_STOP_LAT 0x0007
#       define RADEON_SCLK_MORE_FORCEON     0x0700
#define RADEON_SDRAM_MODE_REG               0x0158
#define RADEON_SEQ8_DATA                    0x03c5 /* VGA */
#define RADEON_SEQ8_IDX                     0x03c4 /* VGA */
#define RADEON_SNAPSHOT_F_COUNT             0x0244
#define RADEON_SNAPSHOT_VH_COUNTS           0x0240
#define RADEON_SNAPSHOT_VIF_COUNT           0x024c
#define RADEON_SRC_OFFSET                   0x15ac
#define RADEON_SRC_PITCH                    0x15b0
#define RADEON_SRC_PITCH_OFFSET             0x1428
#define RADEON_SRC_SC_BOTTOM                0x165c
#define RADEON_SRC_SC_BOTTOM_RIGHT          0x16f4
#define RADEON_SRC_SC_RIGHT                 0x1654
#define RADEON_SRC_X                        0x1414
#define RADEON_SRC_X_Y                      0x1590
#define RADEON_SRC_Y                        0x1418
#define RADEON_SRC_Y_X                      0x1434
#define RADEON_STATUS                       0x0f06 /* PCI */
#define RADEON_SUBPIC_CNTL                  0x0540 /* ? */
#define RADEON_SUB_CLASS                    0x0f0a /* PCI */
#define RADEON_SURFACE_CNTL                 0x0b00
#       define RADEON_SURF_TRANSLATION_DIS  (1 << 8)
#       define RADEON_NONSURF_AP0_SWP_16BPP (1 << 20)
#       define RADEON_NONSURF_AP0_SWP_32BPP (1 << 21)
#       define RADEON_NONSURF_AP1_SWP_16BPP (1 << 22)
#       define RADEON_NONSURF_AP1_SWP_32BPP (1 << 23)
#define RADEON_SURFACE0_INFO                0x0b0c
#       define RADEON_SURF_TILE_COLOR_MACRO (0 << 16)
#       define RADEON_SURF_TILE_COLOR_BOTH  (1 << 16)
#       define RADEON_SURF_TILE_DEPTH_32BPP (2 << 16)
#       define RADEON_SURF_TILE_DEPTH_16BPP (3 << 16)
#       define R200_SURF_TILE_NONE          (0 << 16)
#       define R200_SURF_TILE_COLOR_MACRO   (1 << 16)
#       define R200_SURF_TILE_COLOR_MICRO   (2 << 16)
#       define R200_SURF_TILE_COLOR_BOTH    (3 << 16)
#       define R200_SURF_TILE_DEPTH_32BPP   (4 << 16)
#       define R200_SURF_TILE_DEPTH_16BPP   (5 << 16)
#       define R300_SURF_TILE_NONE          (0 << 16)
#       define R300_SURF_TILE_COLOR_MACRO   (1 << 16)
#       define R300_SURF_TILE_DEPTH_32BPP   (2 << 16)
#       define RADEON_SURF_AP0_SWP_16BPP    (1 << 20)
#       define RADEON_SURF_AP0_SWP_32BPP    (1 << 21)
#       define RADEON_SURF_AP1_SWP_16BPP    (1 << 22)
#       define RADEON_SURF_AP1_SWP_32BPP    (1 << 23)
#define RADEON_SURFACE0_LOWER_BOUND         0x0b04
#define RADEON_SURFACE0_UPPER_BOUND         0x0b08
#define RADEON_SURFACE1_INFO                0x0b1c
#define RADEON_SURFACE1_LOWER_BOUND         0x0b14
#define RADEON_SURFACE1_UPPER_BOUND         0x0b18
#define RADEON_SURFACE2_INFO                0x0b2c
#define RADEON_SURFACE2_LOWER_BOUND         0x0b24
#define RADEON_SURFACE2_UPPER_BOUND         0x0b28
#define RADEON_SURFACE3_INFO                0x0b3c
#define RADEON_SURFACE3_LOWER_BOUND         0x0b34
#define RADEON_SURFACE3_UPPER_BOUND         0x0b38
#define RADEON_SURFACE4_INFO                0x0b4c
#define RADEON_SURFACE4_LOWER_BOUND         0x0b44
#define RADEON_SURFACE4_UPPER_BOUND         0x0b48
#define RADEON_SURFACE5_INFO                0x0b5c
#define RADEON_SURFACE5_LOWER_BOUND         0x0b54
#define RADEON_SURFACE5_UPPER_BOUND         0x0b58
#define RADEON_SURFACE6_INFO                0x0b6c
#define RADEON_SURFACE6_LOWER_BOUND         0x0b64
#define RADEON_SURFACE6_UPPER_BOUND         0x0b68
#define RADEON_SURFACE7_INFO                0x0b7c
#define RADEON_SURFACE7_LOWER_BOUND         0x0b74
#define RADEON_SURFACE7_UPPER_BOUND         0x0b78
#define RADEON_SW_SEMAPHORE                 0x013c

#define RADEON_TEST_DEBUG_CNTL              0x0120
#define RADEON_TEST_DEBUG_CNTL__TEST_DEBUG_OUT_EN 0x00000001

#define RADEON_TEST_DEBUG_MUX               0x0124
#define RADEON_TEST_DEBUG_OUT               0x012c
#define RADEON_TMDS_PLL_CNTL                0x02a8
#define RADEON_TMDS_TRANSMITTER_CNTL        0x02a4
#       define RADEON_TMDS_TRANSMITTER_PLLEN  1
#       define RADEON_TMDS_TRANSMITTER_PLLRST 2
#define RADEON_TRAIL_BRES_DEC               0x1614
#define RADEON_TRAIL_BRES_ERR               0x160c
#define RADEON_TRAIL_BRES_INC               0x1610
#define RADEON_TRAIL_X                      0x1618
#define RADEON_TRAIL_X_SUB                  0x1620

#define RADEON_VCLK_ECP_CNTL                0x0008 /* PLL */
#       define RADEON_VCLK_SRC_SEL_MASK     0x03
#       define RADEON_VCLK_SRC_SEL_CPUCLK   0x00
#       define RADEON_VCLK_SRC_SEL_PSCANCLK 0x01
#       define RADEON_VCLK_SRC_SEL_BYTECLK  0x02
#       define RADEON_VCLK_SRC_SEL_PPLLCLK  0x03
#       define RADEON_PIXCLK_ALWAYS_ONb     (1<<6)
#       define RADEON_PIXCLK_DAC_ALWAYS_ONb (1<<7)
#       define R300_DISP_DAC_PIXCLK_DAC_BLANK_OFF (1<<23)

#define RADEON_VENDOR_ID                    0x0f00 /* PCI */
#define RADEON_VGA_DDA_CONFIG               0x02e8
#define RADEON_VGA_DDA_ON_OFF               0x02ec
#define RADEON_VID_BUFFER_CONTROL           0x0900
#define RADEON_VIDEOMUX_CNTL                0x0190

                 /* VIP bus */
#define RADEON_VIPH_CH0_DATA                0x0c00
#define RADEON_VIPH_CH1_DATA                0x0c04
#define RADEON_VIPH_CH2_DATA                0x0c08
#define RADEON_VIPH_CH3_DATA                0x0c0c
#define RADEON_VIPH_CH0_ADDR                0x0c10
#define RADEON_VIPH_CH1_ADDR                0x0c14
#define RADEON_VIPH_CH2_ADDR                0x0c18
#define RADEON_VIPH_CH3_ADDR                0x0c1c
#define RADEON_VIPH_CH0_SBCNT               0x0c20
#define RADEON_VIPH_CH1_SBCNT               0x0c24
#define RADEON_VIPH_CH2_SBCNT               0x0c28
#define RADEON_VIPH_CH3_SBCNT               0x0c2c
#define RADEON_VIPH_CH0_ABCNT               0x0c30
#define RADEON_VIPH_CH1_ABCNT               0x0c34
#define RADEON_VIPH_CH2_ABCNT               0x0c38
#define RADEON_VIPH_CH3_ABCNT               0x0c3c
#define RADEON_VIPH_CONTROL                 0x0c40
#       define RADEON_VIP_BUSY 0
#       define RADEON_VIP_IDLE 1
#       define RADEON_VIP_RESET 2
#       define RADEON_VIPH_EN               (1 << 21)
#define RADEON_VIPH_DV_LAT                  0x0c44
#define RADEON_VIPH_BM_CHUNK                0x0c48
#define RADEON_VIPH_DV_INT                  0x0c4c
#define RADEON_VIPH_TIMEOUT_STAT            0x0c50
#define RADEON_VIPH_TIMEOUT_STAT__VIPH_REG_STAT 0x00000010
#define RADEON_VIPH_TIMEOUT_STAT__VIPH_REG_AK   0x00000010
#define RADEON_VIPH_TIMEOUT_STAT__VIPH_REGR_DIS 0x01000000

#define RADEON_VIPH_REG_DATA                0x0084
#define RADEON_VIPH_REG_ADDR                0x0080


#define RADEON_WAIT_UNTIL                   0x1720
#       define RADEON_WAIT_CRTC_PFLIP       (1 << 0)
#       define RADEON_WAIT_RE_CRTC_VLINE    (1 << 1)
#       define RADEON_WAIT_FE_CRTC_VLINE    (1 << 2)
#       define RADEON_WAIT_CRTC_VLINE       (1 << 3)
#       define RADEON_WAIT_DMA_VID_IDLE     (1 << 8)
#       define RADEON_WAIT_DMA_GUI_IDLE     (1 << 9)
#       define RADEON_WAIT_CMDFIFO          (1 << 10) /* wait for CMDFIFO_ENTRIES */
#       define RADEON_WAIT_OV0_FLIP         (1 << 11)
#       define RADEON_WAIT_AGP_FLUSH        (1 << 13)
#       define RADEON_WAIT_2D_IDLE          (1 << 14)
#       define RADEON_WAIT_3D_IDLE          (1 << 15)
#       define RADEON_WAIT_2D_IDLECLEAN     (1 << 16)
#       define RADEON_WAIT_3D_IDLECLEAN     (1 << 17)
#       define RADEON_WAIT_HOST_IDLECLEAN   (1 << 18)
#       define RADEON_CMDFIFO_ENTRIES_SHIFT 10
#       define RADEON_CMDFIFO_ENTRIES_MASK  0x7f
#       define RADEON_WAIT_VAP_IDLE         (1 << 28)
#       define RADEON_WAIT_BOTH_CRTC_PFLIP  (1 << 30)
#       define RADEON_ENG_DISPLAY_SELECT_CRTC0    (0 << 31)
#       define RADEON_ENG_DISPLAY_SELECT_CRTC1    (1 << 31)

#define RADEON_X_MPLL_REF_FB_DIV            0x000a /* PLL */
#define RADEON_XCLK_CNTL                    0x000d /* PLL */
#define RADEON_XDLL_CNTL                    0x000c /* PLL */
#define RADEON_XPLL_CNTL                    0x000b /* PLL */



				/* Registers for 3D/TCL */
#define RADEON_PP_BORDER_COLOR_0            0x1d40
#define RADEON_PP_BORDER_COLOR_1            0x1d44
#define RADEON_PP_BORDER_COLOR_2            0x1d48
#define RADEON_PP_CNTL                      0x1c38
#       define RADEON_STIPPLE_ENABLE        (1 <<  0)
#       define RADEON_SCISSOR_ENABLE        (1 <<  1)
#       define RADEON_PATTERN_ENABLE        (1 <<  2)
#       define RADEON_SHADOW_ENABLE         (1 <<  3)
#       define RADEON_TEX_ENABLE_MASK       (0xf << 4)
#       define RADEON_TEX_0_ENABLE          (1 <<  4)
#       define RADEON_TEX_1_ENABLE          (1 <<  5)
#       define RADEON_TEX_2_ENABLE          (1 <<  6)
#       define RADEON_TEX_3_ENABLE          (1 <<  7)
#       define RADEON_TEX_BLEND_ENABLE_MASK (0xf << 12)
#       define RADEON_TEX_BLEND_0_ENABLE    (1 << 12)
#       define RADEON_TEX_BLEND_1_ENABLE    (1 << 13)
#       define RADEON_TEX_BLEND_2_ENABLE    (1 << 14)
#       define RADEON_TEX_BLEND_3_ENABLE    (1 << 15)
#       define RADEON_PLANAR_YUV_ENABLE     (1 << 20)
#       define RADEON_SPECULAR_ENABLE       (1 << 21)
#       define RADEON_FOG_ENABLE            (1 << 22)
#       define RADEON_ALPHA_TEST_ENABLE     (1 << 23)
#       define RADEON_ANTI_ALIAS_NONE       (0 << 24)
#       define RADEON_ANTI_ALIAS_LINE       (1 << 24)
#       define RADEON_ANTI_ALIAS_POLY       (2 << 24)
#       define RADEON_ANTI_ALIAS_LINE_POLY  (3 << 24)
#       define RADEON_BUMP_MAP_ENABLE       (1 << 26)
#       define RADEON_BUMPED_MAP_T0         (0 << 27)
#       define RADEON_BUMPED_MAP_T1         (1 << 27)
#       define RADEON_BUMPED_MAP_T2         (2 << 27)
#       define RADEON_TEX_3D_ENABLE_0       (1 << 29)
#       define RADEON_TEX_3D_ENABLE_1       (1 << 30)
#       define RADEON_MC_ENABLE             (1 << 31)
#define RADEON_PP_FOG_COLOR                 0x1c18
#       define RADEON_FOG_COLOR_MASK        0x00ffffff
#       define RADEON_FOG_VERTEX            (0 << 24)
#       define RADEON_FOG_TABLE             (1 << 24)
#       define RADEON_FOG_USE_DEPTH         (0 << 25)
#       define RADEON_FOG_USE_DIFFUSE_ALPHA (2 << 25)
#       define RADEON_FOG_USE_SPEC_ALPHA    (3 << 25)
#define RADEON_PP_LUM_MATRIX                0x1d00
#define RADEON_PP_MISC                      0x1c14
#       define RADEON_REF_ALPHA_MASK        0x000000ff
#       define RADEON_ALPHA_TEST_FAIL       (0 << 8)
#       define RADEON_ALPHA_TEST_LESS       (1 << 8)
#       define RADEON_ALPHA_TEST_LEQUAL     (2 << 8)
#       define RADEON_ALPHA_TEST_EQUAL      (3 << 8)
#       define RADEON_ALPHA_TEST_GEQUAL     (4 << 8)
#       define RADEON_ALPHA_TEST_GREATER    (5 << 8)
#       define RADEON_ALPHA_TEST_NEQUAL     (6 << 8)
#       define RADEON_ALPHA_TEST_PASS       (7 << 8)
#       define RADEON_ALPHA_TEST_OP_MASK    (7 << 8)
#       define RADEON_CHROMA_FUNC_FAIL      (0 << 16)
#       define RADEON_CHROMA_FUNC_PASS      (1 << 16)
#       define RADEON_CHROMA_FUNC_NEQUAL    (2 << 16)
#       define RADEON_CHROMA_FUNC_EQUAL     (3 << 16)
#       define RADEON_CHROMA_KEY_NEAREST    (0 << 18)
#       define RADEON_CHROMA_KEY_ZERO       (1 << 18)
#       define RADEON_SHADOW_ID_AUTO_INC    (1 << 20)
#       define RADEON_SHADOW_FUNC_EQUAL     (0 << 21)
#       define RADEON_SHADOW_FUNC_NEQUAL    (1 << 21)
#       define RADEON_SHADOW_PASS_1         (0 << 22)
#       define RADEON_SHADOW_PASS_2         (1 << 22)
#       define RADEON_RIGHT_HAND_CUBE_D3D   (0 << 24)
#       define RADEON_RIGHT_HAND_CUBE_OGL   (1 << 24)
#define RADEON_PP_ROT_MATRIX_0              0x1d58
#define RADEON_PP_ROT_MATRIX_1              0x1d5c
#define RADEON_PP_TXFILTER_0                0x1c54
#define RADEON_PP_TXFILTER_1                0x1c6c
#define RADEON_PP_TXFILTER_2                0x1c84
#       define RADEON_MAG_FILTER_NEAREST                   (0  <<  0)
#       define RADEON_MAG_FILTER_LINEAR                    (1  <<  0)
#       define RADEON_MAG_FILTER_MASK                      (1  <<  0)
#       define RADEON_MIN_FILTER_NEAREST                   (0  <<  1)
#       define RADEON_MIN_FILTER_LINEAR                    (1  <<  1)
#       define RADEON_MIN_FILTER_NEAREST_MIP_NEAREST       (2  <<  1)
#       define RADEON_MIN_FILTER_NEAREST_MIP_LINEAR        (3  <<  1)
#       define RADEON_MIN_FILTER_LINEAR_MIP_NEAREST        (6  <<  1)
#       define RADEON_MIN_FILTER_LINEAR_MIP_LINEAR         (7  <<  1)
#       define RADEON_MIN_FILTER_ANISO_NEAREST             (8  <<  1)
#       define RADEON_MIN_FILTER_ANISO_LINEAR              (9  <<  1)
#       define RADEON_MIN_FILTER_ANISO_NEAREST_MIP_NEAREST (10 <<  1)
#       define RADEON_MIN_FILTER_ANISO_NEAREST_MIP_LINEAR  (11 <<  1)
#       define RADEON_MIN_FILTER_MASK                      (15 <<  1)
#       define RADEON_MAX_ANISO_1_TO_1                     (0  <<  5)
#       define RADEON_MAX_ANISO_2_TO_1                     (1  <<  5)
#       define RADEON_MAX_ANISO_4_TO_1                     (2  <<  5)
#       define RADEON_MAX_ANISO_8_TO_1                     (3  <<  5)
#       define RADEON_MAX_ANISO_16_TO_1                    (4  <<  5)
#       define RADEON_MAX_ANISO_MASK                       (7  <<  5)
#       define RADEON_LOD_BIAS_MASK                        (0xff <<  8)
#       define RADEON_LOD_BIAS_SHIFT                       8
#       define RADEON_MAX_MIP_LEVEL_MASK                   (0x0f << 16)
#       define RADEON_MAX_MIP_LEVEL_SHIFT                  16
#       define RADEON_YUV_TO_RGB                           (1  << 20)
#       define RADEON_YUV_TEMPERATURE_COOL                 (0  << 21)
#       define RADEON_YUV_TEMPERATURE_HOT                  (1  << 21)
#       define RADEON_YUV_TEMPERATURE_MASK                 (1  << 21)
#       define RADEON_WRAPEN_S                             (1  << 22)
#       define RADEON_CLAMP_S_WRAP                         (0  << 23)
#       define RADEON_CLAMP_S_MIRROR                       (1  << 23)
#       define RADEON_CLAMP_S_CLAMP_LAST                   (2  << 23)
#       define RADEON_CLAMP_S_MIRROR_CLAMP_LAST            (3  << 23)
#       define RADEON_CLAMP_S_CLAMP_BORDER                 (4  << 23)
#       define RADEON_CLAMP_S_MIRROR_CLAMP_BORDER          (5  << 23)
#       define RADEON_CLAMP_S_CLAMP_GL                     (6  << 23)
#       define RADEON_CLAMP_S_MIRROR_CLAMP_GL              (7  << 23)
#       define RADEON_CLAMP_S_MASK                         (7  << 23)
#       define RADEON_WRAPEN_T                             (1  << 26)
#       define RADEON_CLAMP_T_WRAP                         (0  << 27)
#       define RADEON_CLAMP_T_MIRROR                       (1  << 27)
#       define RADEON_CLAMP_T_CLAMP_LAST                   (2  << 27)
#       define RADEON_CLAMP_T_MIRROR_CLAMP_LAST            (3  << 27)
#       define RADEON_CLAMP_T_CLAMP_BORDER                 (4  << 27)
#       define RADEON_CLAMP_T_MIRROR_CLAMP_BORDER          (5  << 27)
#       define RADEON_CLAMP_T_CLAMP_GL                     (6  << 27)
#       define RADEON_CLAMP_T_MIRROR_CLAMP_GL              (7  << 27)
#       define RADEON_CLAMP_T_MASK                         (7  << 27)
#       define RADEON_BORDER_MODE_OGL                      (0  << 31)
#       define RADEON_BORDER_MODE_D3D                      (1  << 31)
#define RADEON_PP_TXFORMAT_0                0x1c58
#define RADEON_PP_TXFORMAT_1                0x1c70
#define RADEON_PP_TXFORMAT_2                0x1c88
#       define RADEON_TXFORMAT_I8                 (0  <<  0)
#       define RADEON_TXFORMAT_AI88               (1  <<  0)
#       define RADEON_TXFORMAT_RGB332             (2  <<  0)
#       define RADEON_TXFORMAT_ARGB1555           (3  <<  0)
#       define RADEON_TXFORMAT_RGB565             (4  <<  0)
#       define RADEON_TXFORMAT_ARGB4444           (5  <<  0)
#       define RADEON_TXFORMAT_ARGB8888           (6  <<  0)
#       define RADEON_TXFORMAT_RGBA8888           (7  <<  0)
#       define RADEON_TXFORMAT_Y8                 (8  <<  0)
#       define RADEON_TXFORMAT_VYUY422            (10 <<  0)
#       define RADEON_TXFORMAT_YVYU422            (11 <<  0)
#       define RADEON_TXFORMAT_DXT1               (12 <<  0)
#       define RADEON_TXFORMAT_DXT23              (14 <<  0)
#       define RADEON_TXFORMAT_DXT45              (15 <<  0)
#       define RADEON_TXFORMAT_FORMAT_MASK        (31 <<  0)
#       define RADEON_TXFORMAT_FORMAT_SHIFT       0
#       define RADEON_TXFORMAT_APPLE_YUV_MODE     (1  <<  5)
#       define RADEON_TXFORMAT_ALPHA_IN_MAP       (1  <<  6)
#       define RADEON_TXFORMAT_NON_POWER2         (1  <<  7)
#       define RADEON_TXFORMAT_WIDTH_MASK         (15 <<  8)
#       define RADEON_TXFORMAT_WIDTH_SHIFT        8
#       define RADEON_TXFORMAT_HEIGHT_MASK        (15 << 12)
#       define RADEON_TXFORMAT_HEIGHT_SHIFT       12
#       define RADEON_TXFORMAT_F5_WIDTH_MASK      (15 << 16)
#       define RADEON_TXFORMAT_F5_WIDTH_SHIFT     16
#       define RADEON_TXFORMAT_F5_HEIGHT_MASK     (15 << 20)
#       define RADEON_TXFORMAT_F5_HEIGHT_SHIFT    20
#       define RADEON_TXFORMAT_ST_ROUTE_STQ0      (0  << 24)
#       define RADEON_TXFORMAT_ST_ROUTE_MASK      (3  << 24)
#       define RADEON_TXFORMAT_ST_ROUTE_STQ1      (1  << 24)
#       define RADEON_TXFORMAT_ST_ROUTE_STQ2      (2  << 24)
#       define RADEON_TXFORMAT_ENDIAN_NO_SWAP     (0  << 26)
#       define RADEON_TXFORMAT_ENDIAN_16BPP_SWAP  (1  << 26)
#       define RADEON_TXFORMAT_ENDIAN_32BPP_SWAP  (2  << 26)
#       define RADEON_TXFORMAT_ENDIAN_HALFDW_SWAP (3  << 26)
#       define RADEON_TXFORMAT_ALPHA_MASK_ENABLE  (1  << 28)
#       define RADEON_TXFORMAT_CHROMA_KEY_ENABLE  (1  << 29)
#       define RADEON_TXFORMAT_CUBIC_MAP_ENABLE   (1  << 30)
#       define RADEON_TXFORMAT_PERSPECTIVE_ENABLE (1  << 31)
#define RADEON_PP_CUBIC_FACES_0             0x1d24
#define RADEON_PP_CUBIC_FACES_1             0x1d28
#define RADEON_PP_CUBIC_FACES_2             0x1d2c
#       define RADEON_FACE_WIDTH_1_SHIFT          0
#       define RADEON_FACE_HEIGHT_1_SHIFT         4
#       define RADEON_FACE_WIDTH_1_MASK           (0xf << 0)
#       define RADEON_FACE_HEIGHT_1_MASK          (0xf << 4)
#       define RADEON_FACE_WIDTH_2_SHIFT          8
#       define RADEON_FACE_HEIGHT_2_SHIFT         12
#       define RADEON_FACE_WIDTH_2_MASK           (0xf << 8)
#       define RADEON_FACE_HEIGHT_2_MASK          (0xf << 12)
#       define RADEON_FACE_WIDTH_3_SHIFT          16
#       define RADEON_FACE_HEIGHT_3_SHIFT         20
#       define RADEON_FACE_WIDTH_3_MASK           (0xf << 16)
#       define RADEON_FACE_HEIGHT_3_MASK          (0xf << 20)
#       define RADEON_FACE_WIDTH_4_SHIFT          24
#       define RADEON_FACE_HEIGHT_4_SHIFT         28
#       define RADEON_FACE_WIDTH_4_MASK           (0xf << 24)
#       define RADEON_FACE_HEIGHT_4_MASK          (0xf << 28)

#define RADEON_PP_TXOFFSET_0                0x1c5c
#define RADEON_PP_TXOFFSET_1                0x1c74
#define RADEON_PP_TXOFFSET_2                0x1c8c
#       define RADEON_TXO_ENDIAN_NO_SWAP     (0 << 0)
#       define RADEON_TXO_ENDIAN_BYTE_SWAP   (1 << 0)
#       define RADEON_TXO_ENDIAN_WORD_SWAP   (2 << 0)
#       define RADEON_TXO_ENDIAN_HALFDW_SWAP (3 << 0)
#       define RADEON_TXO_MACRO_LINEAR       (0 << 2)
#       define RADEON_TXO_MACRO_TILE         (1 << 2)
#       define RADEON_TXO_MICRO_LINEAR       (0 << 3)
#       define RADEON_TXO_MICRO_TILE_X2      (1 << 3)
#       define RADEON_TXO_MICRO_TILE_OPT     (2 << 3)
#       define RADEON_TXO_OFFSET_MASK        0xffffffe0
#       define RADEON_TXO_OFFSET_SHIFT       5

#define RADEON_PP_CUBIC_OFFSET_T0_0         0x1dd0  /* bits [31:5] */
#define RADEON_PP_CUBIC_OFFSET_T0_1         0x1dd4
#define RADEON_PP_CUBIC_OFFSET_T0_2         0x1dd8
#define RADEON_PP_CUBIC_OFFSET_T0_3         0x1ddc
#define RADEON_PP_CUBIC_OFFSET_T0_4         0x1de0
#define RADEON_PP_CUBIC_OFFSET_T1_0         0x1e00
#define RADEON_PP_CUBIC_OFFSET_T1_1         0x1e04
#define RADEON_PP_CUBIC_OFFSET_T1_2         0x1e08
#define RADEON_PP_CUBIC_OFFSET_T1_3         0x1e0c
#define RADEON_PP_CUBIC_OFFSET_T1_4         0x1e10
#define RADEON_PP_CUBIC_OFFSET_T2_0         0x1e14
#define RADEON_PP_CUBIC_OFFSET_T2_1         0x1e18
#define RADEON_PP_CUBIC_OFFSET_T2_2         0x1e1c
#define RADEON_PP_CUBIC_OFFSET_T2_3         0x1e20
#define RADEON_PP_CUBIC_OFFSET_T2_4         0x1e24

#define RADEON_PP_TEX_SIZE_0                0x1d04  /* NPOT */
#define RADEON_PP_TEX_SIZE_1                0x1d0c
#define RADEON_PP_TEX_SIZE_2                0x1d14
#       define RADEON_TEX_USIZE_MASK        (0x7ff << 0)
#       define RADEON_TEX_USIZE_SHIFT       0
#       define RADEON_TEX_VSIZE_MASK        (0x7ff << 16)
#       define RADEON_TEX_VSIZE_SHIFT       16
#       define RADEON_SIGNED_RGB_MASK       (1 << 30)
#       define RADEON_SIGNED_RGB_SHIFT      30
#       define RADEON_SIGNED_ALPHA_MASK     (1 << 31)
#       define RADEON_SIGNED_ALPHA_SHIFT    31
#define RADEON_PP_TEX_PITCH_0               0x1d08  /* NPOT */
#define RADEON_PP_TEX_PITCH_1               0x1d10  /* NPOT */
#define RADEON_PP_TEX_PITCH_2               0x1d18  /* NPOT */
/* note: bits 13-5: 32 byte aligned stride of texture map */

#define RADEON_PP_TXCBLEND_0                0x1c60
#define RADEON_PP_TXCBLEND_1                0x1c78
#define RADEON_PP_TXCBLEND_2                0x1c90
#       define RADEON_COLOR_ARG_A_SHIFT          0
#       define RADEON_COLOR_ARG_A_MASK           (0x1f << 0)
#       define RADEON_COLOR_ARG_A_ZERO           (0    << 0)
#       define RADEON_COLOR_ARG_A_CURRENT_COLOR  (2    << 0)
#       define RADEON_COLOR_ARG_A_CURRENT_ALPHA  (3    << 0)
#       define RADEON_COLOR_ARG_A_DIFFUSE_COLOR  (4    << 0)
#       define RADEON_COLOR_ARG_A_DIFFUSE_ALPHA  (5    << 0)
#       define RADEON_COLOR_ARG_A_SPECULAR_COLOR (6    << 0)
#       define RADEON_COLOR_ARG_A_SPECULAR_ALPHA (7    << 0)
#       define RADEON_COLOR_ARG_A_TFACTOR_COLOR  (8    << 0)
#       define RADEON_COLOR_ARG_A_TFACTOR_ALPHA  (9    << 0)
#       define RADEON_COLOR_ARG_A_T0_COLOR       (10   << 0)
#       define RADEON_COLOR_ARG_A_T0_ALPHA       (11   << 0)
#       define RADEON_COLOR_ARG_A_T1_COLOR       (12   << 0)
#       define RADEON_COLOR_ARG_A_T1_ALPHA       (13   << 0)
#       define RADEON_COLOR_ARG_A_T2_COLOR       (14   << 0)
#       define RADEON_COLOR_ARG_A_T2_ALPHA       (15   << 0)
#       define RADEON_COLOR_ARG_A_T3_COLOR       (16   << 0)
#       define RADEON_COLOR_ARG_A_T3_ALPHA       (17   << 0)
#       define RADEON_COLOR_ARG_B_SHIFT          5
#       define RADEON_COLOR_ARG_B_MASK           (0x1f << 5)
#       define RADEON_COLOR_ARG_B_ZERO           (0    << 5)
#       define RADEON_COLOR_ARG_B_CURRENT_COLOR  (2    << 5)
#       define RADEON_COLOR_ARG_B_CURRENT_ALPHA  (3    << 5)
#       define RADEON_COLOR_ARG_B_DIFFUSE_COLOR  (4    << 5)
#       define RADEON_COLOR_ARG_B_DIFFUSE_ALPHA  (5    << 5)
#       define RADEON_COLOR_ARG_B_SPECULAR_COLOR (6    << 5)
#       define RADEON_COLOR_ARG_B_SPECULAR_ALPHA (7    << 5)
#       define RADEON_COLOR_ARG_B_TFACTOR_COLOR  (8    << 5)
#       define RADEON_COLOR_ARG_B_TFACTOR_ALPHA  (9    << 5)
#       define RADEON_COLOR_ARG_B_T0_COLOR       (10   << 5)
#       define RADEON_COLOR_ARG_B_T0_ALPHA       (11   << 5)
#       define RADEON_COLOR_ARG_B_T1_COLOR       (12   << 5)
#       define RADEON_COLOR_ARG_B_T1_ALPHA       (13   << 5)
#       define RADEON_COLOR_ARG_B_T2_COLOR       (14   << 5)
#       define RADEON_COLOR_ARG_B_T2_ALPHA       (15   << 5)
#       define RADEON_COLOR_ARG_B_T3_COLOR       (16   << 5)
#       define RADEON_COLOR_ARG_B_T3_ALPHA       (17   << 5)
#       define RADEON_COLOR_ARG_C_SHIFT          10
#       define RADEON_COLOR_ARG_C_MASK           (0x1f << 10)
#       define RADEON_COLOR_ARG_C_ZERO           (0    << 10)
#       define RADEON_COLOR_ARG_C_CURRENT_COLOR  (2    << 10)
#       define RADEON_COLOR_ARG_C_CURRENT_ALPHA  (3    << 10)
#       define RADEON_COLOR_ARG_C_DIFFUSE_COLOR  (4    << 10)
#       define RADEON_COLOR_ARG_C_DIFFUSE_ALPHA  (5    << 10)
#       define RADEON_COLOR_ARG_C_SPECULAR_COLOR (6    << 10)
#       define RADEON_COLOR_ARG_C_SPECULAR_ALPHA (7    << 10)
#       define RADEON_COLOR_ARG_C_TFACTOR_COLOR  (8    << 10)
#       define RADEON_COLOR_ARG_C_TFACTOR_ALPHA  (9    << 10)
#       define RADEON_COLOR_ARG_C_T0_COLOR       (10   << 10)
#       define RADEON_COLOR_ARG_C_T0_ALPHA       (11   << 10)
#       define RADEON_COLOR_ARG_C_T1_COLOR       (12   << 10)
#       define RADEON_COLOR_ARG_C_T1_ALPHA       (13   << 10)
#       define RADEON_COLOR_ARG_C_T2_COLOR       (14   << 10)
#       define RADEON_COLOR_ARG_C_T2_ALPHA       (15   << 10)
#       define RADEON_COLOR_ARG_C_T3_COLOR       (16   << 10)
#       define RADEON_COLOR_ARG_C_T3_ALPHA       (17   << 10)
#       define RADEON_COMP_ARG_A                 (1 << 15)
#       define RADEON_COMP_ARG_A_SHIFT           15
#       define RADEON_COMP_ARG_B                 (1 << 16)
#       define RADEON_COMP_ARG_B_SHIFT           16
#       define RADEON_COMP_ARG_C                 (1 << 17)
#       define RADEON_COMP_ARG_C_SHIFT           17
#       define RADEON_BLEND_CTL_MASK             (7 << 18)
#       define RADEON_BLEND_CTL_ADD              (0 << 18)
#       define RADEON_BLEND_CTL_SUBTRACT         (1 << 18)
#       define RADEON_BLEND_CTL_ADDSIGNED        (2 << 18)
#       define RADEON_BLEND_CTL_BLEND            (3 << 18)
#       define RADEON_BLEND_CTL_DOT3             (4 << 18)
#       define RADEON_SCALE_SHIFT                21
#       define RADEON_SCALE_MASK                 (3 << 21)
#       define RADEON_SCALE_1X                   (0 << 21)
#       define RADEON_SCALE_2X                   (1 << 21)
#       define RADEON_SCALE_4X                   (2 << 21)
#       define RADEON_CLAMP_TX                   (1 << 23)
#       define RADEON_T0_EQ_TCUR                 (1 << 24)
#       define RADEON_T1_EQ_TCUR                 (1 << 25)
#       define RADEON_T2_EQ_TCUR                 (1 << 26)
#       define RADEON_T3_EQ_TCUR                 (1 << 27)
#       define RADEON_COLOR_ARG_MASK             0x1f
#       define RADEON_COMP_ARG_SHIFT             15
#define RADEON_PP_TXABLEND_0                0x1c64
#define RADEON_PP_TXABLEND_1                0x1c7c
#define RADEON_PP_TXABLEND_2                0x1c94
#       define RADEON_ALPHA_ARG_A_SHIFT          0
#       define RADEON_ALPHA_ARG_A_MASK           (0xf << 0)
#       define RADEON_ALPHA_ARG_A_ZERO           (0   << 0)
#       define RADEON_ALPHA_ARG_A_CURRENT_ALPHA  (1   << 0)
#       define RADEON_ALPHA_ARG_A_DIFFUSE_ALPHA  (2   << 0)
#       define RADEON_ALPHA_ARG_A_SPECULAR_ALPHA (3   << 0)
#       define RADEON_ALPHA_ARG_A_TFACTOR_ALPHA  (4   << 0)
#       define RADEON_ALPHA_ARG_A_T0_ALPHA       (5   << 0)
#       define RADEON_ALPHA_ARG_A_T1_ALPHA       (6   << 0)
#       define RADEON_ALPHA_ARG_A_T2_ALPHA       (7   << 0)
#       define RADEON_ALPHA_ARG_A_T3_ALPHA       (8   << 0)
#       define RADEON_ALPHA_ARG_B_SHIFT          4
#       define RADEON_ALPHA_ARG_B_MASK           (0xf << 4)
#       define RADEON_ALPHA_ARG_B_ZERO           (0   << 4)
#       define RADEON_ALPHA_ARG_B_CURRENT_ALPHA  (1   << 4)
#       define RADEON_ALPHA_ARG_B_DIFFUSE_ALPHA  (2   << 4)
#       define RADEON_ALPHA_ARG_B_SPECULAR_ALPHA (3   << 4)
#       define RADEON_ALPHA_ARG_B_TFACTOR_ALPHA  (4   << 4)
#       define RADEON_ALPHA_ARG_B_T0_ALPHA       (5   << 4)
#       define RADEON_ALPHA_ARG_B_T1_ALPHA       (6   << 4)
#       define RADEON_ALPHA_ARG_B_T2_ALPHA       (7   << 4)
#       define RADEON_ALPHA_ARG_B_T3_ALPHA       (8   << 4)
#       define RADEON_ALPHA_ARG_C_SHIFT          8
#       define RADEON_ALPHA_ARG_C_MASK           (0xf << 8)
#       define RADEON_ALPHA_ARG_C_ZERO           (0   << 8)
#       define RADEON_ALPHA_ARG_C_CURRENT_ALPHA  (1   << 8)
#       define RADEON_ALPHA_ARG_C_DIFFUSE_ALPHA  (2   << 8)
#       define RADEON_ALPHA_ARG_C_SPECULAR_ALPHA (3   << 8)
#       define RADEON_ALPHA_ARG_C_TFACTOR_ALPHA  (4   << 8)
#       define RADEON_ALPHA_ARG_C_T0_ALPHA       (5   << 8)
#       define RADEON_ALPHA_ARG_C_T1_ALPHA       (6   << 8)
#       define RADEON_ALPHA_ARG_C_T2_ALPHA       (7   << 8)
#       define RADEON_ALPHA_ARG_C_T3_ALPHA       (8   << 8)
#       define RADEON_DOT_ALPHA_DONT_REPLICATE   (1   << 9)
#       define RADEON_ALPHA_ARG_MASK             0xf

#define RADEON_PP_TFACTOR_0                 0x1c68
#define RADEON_PP_TFACTOR_1                 0x1c80
#define RADEON_PP_TFACTOR_2                 0x1c98

#define RADEON_RB3D_BLENDCNTL               0x1c20
#       define RADEON_COMB_FCN_MASK                    (3  << 12)
#       define RADEON_COMB_FCN_ADD_CLAMP               (0  << 12)
#       define RADEON_COMB_FCN_ADD_NOCLAMP             (1  << 12)
#       define RADEON_COMB_FCN_SUB_CLAMP               (2  << 12)
#       define RADEON_COMB_FCN_SUB_NOCLAMP             (3  << 12)
#       define RADEON_SRC_BLEND_GL_ZERO                (32 << 16)
#       define RADEON_SRC_BLEND_GL_ONE                 (33 << 16)
#       define RADEON_SRC_BLEND_GL_SRC_COLOR           (34 << 16)
#       define RADEON_SRC_BLEND_GL_ONE_MINUS_SRC_COLOR (35 << 16)
#       define RADEON_SRC_BLEND_GL_DST_COLOR           (36 << 16)
#       define RADEON_SRC_BLEND_GL_ONE_MINUS_DST_COLOR (37 << 16)
#       define RADEON_SRC_BLEND_GL_SRC_ALPHA           (38 << 16)
#       define RADEON_SRC_BLEND_GL_ONE_MINUS_SRC_ALPHA (39 << 16)
#       define RADEON_SRC_BLEND_GL_DST_ALPHA           (40 << 16)
#       define RADEON_SRC_BLEND_GL_ONE_MINUS_DST_ALPHA (41 << 16)
#       define RADEON_SRC_BLEND_GL_SRC_ALPHA_SATURATE  (42 << 16)
#       define RADEON_SRC_BLEND_MASK                   (63 << 16)
#       define RADEON_DST_BLEND_GL_ZERO                (32 << 24)
#       define RADEON_DST_BLEND_GL_ONE                 (33 << 24)
#       define RADEON_DST_BLEND_GL_SRC_COLOR           (34 << 24)
#       define RADEON_DST_BLEND_GL_ONE_MINUS_SRC_COLOR (35 << 24)
#       define RADEON_DST_BLEND_GL_DST_COLOR           (36 << 24)
#       define RADEON_DST_BLEND_GL_ONE_MINUS_DST_COLOR (37 << 24)
#       define RADEON_DST_BLEND_GL_SRC_ALPHA           (38 << 24)
#       define RADEON_DST_BLEND_GL_ONE_MINUS_SRC_ALPHA (39 << 24)
#       define RADEON_DST_BLEND_GL_DST_ALPHA           (40 << 24)
#       define RADEON_DST_BLEND_GL_ONE_MINUS_DST_ALPHA (41 << 24)
#       define RADEON_DST_BLEND_MASK                   (63 << 24)
#define RADEON_RB3D_CNTL                    0x1c3c
#       define RADEON_ALPHA_BLEND_ENABLE       (1  <<  0)
#       define RADEON_PLANE_MASK_ENABLE        (1  <<  1)
#       define RADEON_DITHER_ENABLE            (1  <<  2)
#       define RADEON_ROUND_ENABLE             (1  <<  3)
#       define RADEON_SCALE_DITHER_ENABLE      (1  <<  4)
#       define RADEON_DITHER_INIT              (1  <<  5)
#       define RADEON_ROP_ENABLE               (1  <<  6)
#       define RADEON_STENCIL_ENABLE           (1  <<  7)
#       define RADEON_Z_ENABLE                 (1  <<  8)
#       define RADEON_DEPTH_XZ_OFFEST_ENABLE   (1  <<  9)
#       define RADEON_COLOR_FORMAT_ARGB1555    (3  << 10)
#       define RADEON_COLOR_FORMAT_RGB565      (4  << 10)
#       define RADEON_COLOR_FORMAT_ARGB8888    (6  << 10)
#       define RADEON_COLOR_FORMAT_RGB332      (7  << 10)
#       define RADEON_COLOR_FORMAT_Y8          (8  << 10)
#       define RADEON_COLOR_FORMAT_RGB8        (9  << 10)
#       define RADEON_COLOR_FORMAT_YUV422_VYUY (11 << 10)
#       define RADEON_COLOR_FORMAT_YUV422_YVYU (12 << 10)
#       define RADEON_COLOR_FORMAT_aYUV444     (14 << 10)
#       define RADEON_COLOR_FORMAT_ARGB4444    (15 << 10)
#       define RADEON_CLRCMP_FLIP_ENABLE       (1  << 14)
#define RADEON_RB3D_COLOROFFSET             0x1c40
#       define RADEON_COLOROFFSET_MASK      0xfffffff0
#define RADEON_RB3D_COLORPITCH              0x1c48
#       define RADEON_COLORPITCH_MASK         0x000001ff8
#       define RADEON_COLOR_TILE_ENABLE       (1 << 16)
#       define RADEON_COLOR_MICROTILE_ENABLE  (1 << 17)
#       define RADEON_COLOR_ENDIAN_NO_SWAP    (0 << 18)
#       define RADEON_COLOR_ENDIAN_WORD_SWAP  (1 << 18)
#       define RADEON_COLOR_ENDIAN_DWORD_SWAP (2 << 18)
#define RADEON_RB3D_DEPTHOFFSET             0x1c24
#define RADEON_RB3D_DEPTHPITCH              0x1c28
#       define RADEON_DEPTHPITCH_MASK         0x00001ff8
#       define RADEON_DEPTH_ENDIAN_NO_SWAP    (0 << 18)
#       define RADEON_DEPTH_ENDIAN_WORD_SWAP  (1 << 18)
#       define RADEON_DEPTH_ENDIAN_DWORD_SWAP (2 << 18)
#define RADEON_RB3D_PLANEMASK               0x1d84
#define RADEON_RB3D_ROPCNTL                 0x1d80
#       define RADEON_ROP_MASK              (15 << 8)
#       define RADEON_ROP_CLEAR             (0  << 8)
#       define RADEON_ROP_NOR               (1  << 8)
#       define RADEON_ROP_AND_INVERTED      (2  << 8)
#       define RADEON_ROP_COPY_INVERTED     (3  << 8)
#       define RADEON_ROP_AND_REVERSE       (4  << 8)
#       define RADEON_ROP_INVERT            (5  << 8)
#       define RADEON_ROP_XOR               (6  << 8)
#       define RADEON_ROP_NAND              (7  << 8)
#       define RADEON_ROP_AND               (8  << 8)
#       define RADEON_ROP_EQUIV             (9  << 8)
#       define RADEON_ROP_NOOP              (10 << 8)
#       define RADEON_ROP_OR_INVERTED       (11 << 8)
#       define RADEON_ROP_COPY              (12 << 8)
#       define RADEON_ROP_OR_REVERSE        (13 << 8)
#       define RADEON_ROP_OR                (14 << 8)
#       define RADEON_ROP_SET               (15 << 8)
#define RADEON_RB3D_STENCILREFMASK          0x1d7c
#       define RADEON_STENCIL_REF_SHIFT       0
#       define RADEON_STENCIL_REF_MASK        (0xff << 0)
#       define RADEON_STENCIL_MASK_SHIFT      16
#       define RADEON_STENCIL_VALUE_MASK      (0xff << 16)
#       define RADEON_STENCIL_WRITEMASK_SHIFT 24
#       define RADEON_STENCIL_WRITE_MASK      (0xff << 24)
#define RADEON_RB3D_ZSTENCILCNTL            0x1c2c
#       define RADEON_DEPTH_FORMAT_MASK          (0xf << 0)
#       define RADEON_DEPTH_FORMAT_16BIT_INT_Z   (0  <<  0)
#       define RADEON_DEPTH_FORMAT_24BIT_INT_Z   (2  <<  0)
#       define RADEON_DEPTH_FORMAT_24BIT_FLOAT_Z (3  <<  0)
#       define RADEON_DEPTH_FORMAT_32BIT_INT_Z   (4  <<  0)
#       define RADEON_DEPTH_FORMAT_32BIT_FLOAT_Z (5  <<  0)
#       define RADEON_DEPTH_FORMAT_16BIT_FLOAT_W (7  <<  0)
#       define RADEON_DEPTH_FORMAT_24BIT_FLOAT_W (9  <<  0)
#       define RADEON_DEPTH_FORMAT_32BIT_FLOAT_W (11 <<  0)
#       define RADEON_Z_TEST_NEVER               (0  <<  4)
#       define RADEON_Z_TEST_LESS                (1  <<  4)
#       define RADEON_Z_TEST_LEQUAL              (2  <<  4)
#       define RADEON_Z_TEST_EQUAL               (3  <<  4)
#       define RADEON_Z_TEST_GEQUAL              (4  <<  4)
#       define RADEON_Z_TEST_GREATER             (5  <<  4)
#       define RADEON_Z_TEST_NEQUAL              (6  <<  4)
#       define RADEON_Z_TEST_ALWAYS              (7  <<  4)
#       define RADEON_Z_TEST_MASK                (7  <<  4)
#       define RADEON_STENCIL_TEST_NEVER         (0  << 12)
#       define RADEON_STENCIL_TEST_LESS          (1  << 12)
#       define RADEON_STENCIL_TEST_LEQUAL        (2  << 12)
#       define RADEON_STENCIL_TEST_EQUAL         (3  << 12)
#       define RADEON_STENCIL_TEST_GEQUAL        (4  << 12)
#       define RADEON_STENCIL_TEST_GREATER       (5  << 12)
#       define RADEON_STENCIL_TEST_NEQUAL        (6  << 12)
#       define RADEON_STENCIL_TEST_ALWAYS        (7  << 12)
#       define RADEON_STENCIL_TEST_MASK          (0x7 << 12)
#       define RADEON_STENCIL_FAIL_KEEP          (0  << 16)
#       define RADEON_STENCIL_FAIL_ZERO          (1  << 16)
#       define RADEON_STENCIL_FAIL_REPLACE       (2  << 16)
#       define RADEON_STENCIL_FAIL_INC           (3  << 16)
#       define RADEON_STENCIL_FAIL_DEC           (4  << 16)
#       define RADEON_STENCIL_FAIL_INVERT        (5  << 16)
#       define RADEON_STENCIL_FAIL_MASK          (0x7 << 16)
#       define RADEON_STENCIL_ZPASS_KEEP         (0  << 20)
#       define RADEON_STENCIL_ZPASS_ZERO         (1  << 20)
#       define RADEON_STENCIL_ZPASS_REPLACE      (2  << 20)
#       define RADEON_STENCIL_ZPASS_INC          (3  << 20)
#       define RADEON_STENCIL_ZPASS_DEC          (4  << 20)
#       define RADEON_STENCIL_ZPASS_INVERT       (5  << 20)
#       define RADEON_STENCIL_ZPASS_MASK         (0x7 << 20)
#       define RADEON_STENCIL_ZFAIL_KEEP         (0  << 24)
#       define RADEON_STENCIL_ZFAIL_ZERO         (1  << 24)
#       define RADEON_STENCIL_ZFAIL_REPLACE      (2  << 24)
#       define RADEON_STENCIL_ZFAIL_INC          (3  << 24)
#       define RADEON_STENCIL_ZFAIL_DEC          (4  << 24)
#       define RADEON_STENCIL_ZFAIL_INVERT       (5  << 24)
#       define RADEON_STENCIL_ZFAIL_MASK         (0x7 << 24)
#       define RADEON_Z_COMPRESSION_ENABLE       (1  << 28)
#       define RADEON_FORCE_Z_DIRTY              (1  << 29)
#       define RADEON_Z_WRITE_ENABLE             (1  << 30)
#define RADEON_RE_LINE_PATTERN              0x1cd0
#       define RADEON_LINE_PATTERN_MASK             0x0000ffff
#       define RADEON_LINE_REPEAT_COUNT_SHIFT       16
#       define RADEON_LINE_PATTERN_START_SHIFT      24
#       define RADEON_LINE_PATTERN_LITTLE_BIT_ORDER (0 << 28)
#       define RADEON_LINE_PATTERN_BIG_BIT_ORDER    (1 << 28)
#       define RADEON_LINE_PATTERN_AUTO_RESET       (1 << 29)
#define RADEON_RE_LINE_STATE                0x1cd4
#       define RADEON_LINE_CURRENT_PTR_SHIFT   0
#       define RADEON_LINE_CURRENT_COUNT_SHIFT 8
#define RADEON_RE_MISC                      0x26c4
#       define RADEON_STIPPLE_COORD_MASK       0x1f
#       define RADEON_STIPPLE_X_OFFSET_SHIFT   0
#       define RADEON_STIPPLE_X_OFFSET_MASK    (0x1f << 0)
#       define RADEON_STIPPLE_Y_OFFSET_SHIFT   8
#       define RADEON_STIPPLE_Y_OFFSET_MASK    (0x1f << 8)
#       define RADEON_STIPPLE_LITTLE_BIT_ORDER (0 << 16)
#       define RADEON_STIPPLE_BIG_BIT_ORDER    (1 << 16)
#define RADEON_RE_SOLID_COLOR               0x1c1c
#define RADEON_RE_TOP_LEFT                  0x26c0
#       define RADEON_RE_LEFT_SHIFT         0
#       define RADEON_RE_TOP_SHIFT          16
#define RADEON_RE_WIDTH_HEIGHT              0x1c44
#       define RADEON_RE_WIDTH_SHIFT        0
#       define RADEON_RE_HEIGHT_SHIFT       16

#define RADEON_SE_CNTL                      0x1c4c
#       define RADEON_FFACE_CULL_CW          (0 <<  0)
#       define RADEON_FFACE_CULL_CCW         (1 <<  0)
#       define RADEON_FFACE_CULL_DIR_MASK    (1 <<  0)
#       define RADEON_BFACE_CULL             (0 <<  1)
#       define RADEON_BFACE_SOLID            (3 <<  1)
#       define RADEON_FFACE_CULL             (0 <<  3)
#       define RADEON_FFACE_SOLID            (3 <<  3)
#       define RADEON_FFACE_CULL_MASK        (3 <<  3)
#       define RADEON_BADVTX_CULL_DISABLE    (1 <<  5)
#       define RADEON_FLAT_SHADE_VTX_0       (0 <<  6)
#       define RADEON_FLAT_SHADE_VTX_1       (1 <<  6)
#       define RADEON_FLAT_SHADE_VTX_2       (2 <<  6)
#       define RADEON_FLAT_SHADE_VTX_LAST    (3 <<  6)
#       define RADEON_DIFFUSE_SHADE_SOLID    (0 <<  8)
#       define RADEON_DIFFUSE_SHADE_FLAT     (1 <<  8)
#       define RADEON_DIFFUSE_SHADE_GOURAUD  (2 <<  8)
#       define RADEON_DIFFUSE_SHADE_MASK     (3 <<  8)
#       define RADEON_ALPHA_SHADE_SOLID      (0 << 10)
#       define RADEON_ALPHA_SHADE_FLAT       (1 << 10)
#       define RADEON_ALPHA_SHADE_GOURAUD    (2 << 10)
#       define RADEON_ALPHA_SHADE_MASK       (3 << 10)
#       define RADEON_SPECULAR_SHADE_SOLID   (0 << 12)
#       define RADEON_SPECULAR_SHADE_FLAT    (1 << 12)
#       define RADEON_SPECULAR_SHADE_GOURAUD (2 << 12)
#       define RADEON_SPECULAR_SHADE_MASK    (3 << 12)
#       define RADEON_FOG_SHADE_SOLID        (0 << 14)
#       define RADEON_FOG_SHADE_FLAT         (1 << 14)
#       define RADEON_FOG_SHADE_GOURAUD      (2 << 14)
#       define RADEON_FOG_SHADE_MASK         (3 << 14)
#       define RADEON_ZBIAS_ENABLE_POINT     (1 << 16)
#       define RADEON_ZBIAS_ENABLE_LINE      (1 << 17)
#       define RADEON_ZBIAS_ENABLE_TRI       (1 << 18)
#       define RADEON_WIDELINE_ENABLE        (1 << 20)
#       define RADEON_VPORT_XY_XFORM_ENABLE  (1 << 24)
#       define RADEON_VPORT_Z_XFORM_ENABLE   (1 << 25)
#       define RADEON_VTX_PIX_CENTER_D3D     (0 << 27)
#       define RADEON_VTX_PIX_CENTER_OGL     (1 << 27)
#       define RADEON_ROUND_MODE_TRUNC       (0 << 28)
#       define RADEON_ROUND_MODE_ROUND       (1 << 28)
#       define RADEON_ROUND_MODE_ROUND_EVEN  (2 << 28)
#       define RADEON_ROUND_MODE_ROUND_ODD   (3 << 28)
#       define RADEON_ROUND_PREC_16TH_PIX    (0 << 30)
#       define RADEON_ROUND_PREC_8TH_PIX     (1 << 30)
#       define RADEON_ROUND_PREC_4TH_PIX     (2 << 30)
#       define RADEON_ROUND_PREC_HALF_PIX    (3 << 30)
#define R200_RE_CNTL				0x1c50 
#       define R200_STIPPLE_ENABLE		0x1
#       define R200_SCISSOR_ENABLE		0x2
#       define R200_PATTERN_ENABLE		0x4
#       define R200_PERSPECTIVE_ENABLE		0x8
#       define R200_POINT_SMOOTH		0x20
#       define R200_VTX_STQ0_D3D		0x00010000
#       define R200_VTX_STQ1_D3D		0x00040000
#       define R200_VTX_STQ2_D3D		0x00100000
#       define R200_VTX_STQ3_D3D		0x00400000
#       define R200_VTX_STQ4_D3D		0x01000000
#       define R200_VTX_STQ5_D3D		0x04000000
#define R200_RE_SCISSOR_TL_0			0x1cd8
#define R200_RE_SCISSOR_BR_0			0x1cdc
#define R200_RE_SCISSOR_TL_1			0x1ce0
#define R200_RE_SCISSOR_BR_1			0x1ce4
#define R200_RE_SCISSOR_TL_2			0x1ce8
#define R200_RE_SCISSOR_BR_2			0x1cec
#       define R200_SCISSOR_X_SHIFT		0
#       define R200_SCISSOR_Y_SHIFT		16
#define RADEON_SE_CNTL_STATUS               0x2140
#       define RADEON_VC_NO_SWAP            (0 << 0)
#       define RADEON_VC_16BIT_SWAP         (1 << 0)
#       define RADEON_VC_32BIT_SWAP         (2 << 0)
#       define RADEON_VC_HALF_DWORD_SWAP    (3 << 0)
#       define RADEON_TCL_BYPASS            (1 << 8)
#define RADEON_SE_COORD_FMT                 0x1c50
#       define RADEON_VTX_XY_PRE_MULT_1_OVER_W0  (1 <<  0)
#       define RADEON_VTX_Z_PRE_MULT_1_OVER_W0   (1 <<  1)
#       define RADEON_VTX_ST0_NONPARAMETRIC      (1 <<  8)
#       define RADEON_VTX_ST1_NONPARAMETRIC      (1 <<  9)
#       define RADEON_VTX_ST2_NONPARAMETRIC      (1 << 10)
#       define RADEON_VTX_ST3_NONPARAMETRIC      (1 << 11)
#       define RADEON_VTX_W0_NORMALIZE           (1 << 12)
#       define RADEON_VTX_W0_IS_NOT_1_OVER_W0    (1 << 16)
#       define RADEON_VTX_ST0_PRE_MULT_1_OVER_W0 (1 << 17)
#       define RADEON_VTX_ST1_PRE_MULT_1_OVER_W0 (1 << 19)
#       define RADEON_VTX_ST2_PRE_MULT_1_OVER_W0 (1 << 21)
#       define RADEON_VTX_ST3_PRE_MULT_1_OVER_W0 (1 << 23)
#       define RADEON_TEX1_W_ROUTING_USE_W0      (0 << 26)
#       define RADEON_TEX1_W_ROUTING_USE_Q1      (1 << 26)
#define RADEON_SE_LINE_WIDTH                0x1db8
#define RADEON_SE_TCL_LIGHT_MODEL_CTL       0x226c
#       define RADEON_LIGHTING_ENABLE              (1 << 0)
#       define RADEON_LIGHT_IN_MODELSPACE          (1 << 1)
#       define RADEON_LOCAL_VIEWER                 (1 << 2)
#       define RADEON_NORMALIZE_NORMALS            (1 << 3)
#       define RADEON_RESCALE_NORMALS              (1 << 4)
#       define RADEON_SPECULAR_LIGHTS              (1 << 5)
#       define RADEON_DIFFUSE_SPECULAR_COMBINE     (1 << 6)
#       define RADEON_LIGHT_ALPHA                  (1 << 7)
#       define RADEON_LOCAL_LIGHT_VEC_GL           (1 << 8)
#       define RADEON_LIGHT_NO_NORMAL_AMBIENT_ONLY (1 << 9)
#       define RADEON_LM_SOURCE_STATE_PREMULT      0
#       define RADEON_LM_SOURCE_STATE_MULT         1
#       define RADEON_LM_SOURCE_VERTEX_DIFFUSE     2
#       define RADEON_LM_SOURCE_VERTEX_SPECULAR    3
#       define RADEON_EMISSIVE_SOURCE_SHIFT        16
#       define RADEON_AMBIENT_SOURCE_SHIFT         18
#       define RADEON_DIFFUSE_SOURCE_SHIFT         20
#       define RADEON_SPECULAR_SOURCE_SHIFT        22
#define RADEON_SE_TCL_MATERIAL_AMBIENT_RED     0x2220
#define RADEON_SE_TCL_MATERIAL_AMBIENT_GREEN   0x2224
#define RADEON_SE_TCL_MATERIAL_AMBIENT_BLUE    0x2228
#define RADEON_SE_TCL_MATERIAL_AMBIENT_ALPHA   0x222c
#define RADEON_SE_TCL_MATERIAL_DIFFUSE_RED     0x2230
#define RADEON_SE_TCL_MATERIAL_DIFFUSE_GREEN   0x2234
#define RADEON_SE_TCL_MATERIAL_DIFFUSE_BLUE    0x2238
#define RADEON_SE_TCL_MATERIAL_DIFFUSE_ALPHA   0x223c
#define RADEON_SE_TCL_MATERIAL_EMMISSIVE_RED   0x2210
#define RADEON_SE_TCL_MATERIAL_EMMISSIVE_GREEN 0x2214
#define RADEON_SE_TCL_MATERIAL_EMMISSIVE_BLUE  0x2218
#define RADEON_SE_TCL_MATERIAL_EMMISSIVE_ALPHA 0x221c
#define RADEON_SE_TCL_MATERIAL_SPECULAR_RED    0x2240
#define RADEON_SE_TCL_MATERIAL_SPECULAR_GREEN  0x2244
#define RADEON_SE_TCL_MATERIAL_SPECULAR_BLUE   0x2248
#define RADEON_SE_TCL_MATERIAL_SPECULAR_ALPHA  0x224c
#define RADEON_SE_TCL_MATRIX_SELECT_0       0x225c
#       define RADEON_MODELVIEW_0_SHIFT        0
#       define RADEON_MODELVIEW_1_SHIFT        4
#       define RADEON_MODELVIEW_2_SHIFT        8
#       define RADEON_MODELVIEW_3_SHIFT        12
#       define RADEON_IT_MODELVIEW_0_SHIFT     16
#       define RADEON_IT_MODELVIEW_1_SHIFT     20
#       define RADEON_IT_MODELVIEW_2_SHIFT     24
#       define RADEON_IT_MODELVIEW_3_SHIFT     28
#define RADEON_SE_TCL_MATRIX_SELECT_1       0x2260
#       define RADEON_MODELPROJECT_0_SHIFT     0
#       define RADEON_MODELPROJECT_1_SHIFT     4
#       define RADEON_MODELPROJECT_2_SHIFT     8
#       define RADEON_MODELPROJECT_3_SHIFT     12
#       define RADEON_TEXMAT_0_SHIFT           16
#       define RADEON_TEXMAT_1_SHIFT           20
#       define RADEON_TEXMAT_2_SHIFT           24
#       define RADEON_TEXMAT_3_SHIFT           28


#define RADEON_SE_TCL_OUTPUT_VTX_FMT        0x2254
#       define RADEON_TCL_VTX_W0                 (1 <<  0)
#       define RADEON_TCL_VTX_FP_DIFFUSE         (1 <<  1)
#       define RADEON_TCL_VTX_FP_ALPHA           (1 <<  2)
#       define RADEON_TCL_VTX_PK_DIFFUSE         (1 <<  3)
#       define RADEON_TCL_VTX_FP_SPEC            (1 <<  4)
#       define RADEON_TCL_VTX_FP_FOG             (1 <<  5)
#       define RADEON_TCL_VTX_PK_SPEC            (1 <<  6)
#       define RADEON_TCL_VTX_ST0                (1 <<  7)
#       define RADEON_TCL_VTX_ST1                (1 <<  8)
#       define RADEON_TCL_VTX_Q1                 (1 <<  9)
#       define RADEON_TCL_VTX_ST2                (1 << 10)
#       define RADEON_TCL_VTX_Q2                 (1 << 11)
#       define RADEON_TCL_VTX_ST3                (1 << 12)
#       define RADEON_TCL_VTX_Q3                 (1 << 13)
#       define RADEON_TCL_VTX_Q0                 (1 << 14)
#       define RADEON_TCL_VTX_WEIGHT_COUNT_SHIFT 15
#       define RADEON_TCL_VTX_NORM0              (1 << 18)
#       define RADEON_TCL_VTX_XY1                (1 << 27)
#       define RADEON_TCL_VTX_Z1                 (1 << 28)
#       define RADEON_TCL_VTX_W1                 (1 << 29)
#       define RADEON_TCL_VTX_NORM1              (1 << 30)
#       define RADEON_TCL_VTX_Z0                 (1 << 31)

#define RADEON_SE_TCL_OUTPUT_VTX_SEL        0x2258
#       define RADEON_TCL_COMPUTE_XYZW           (1 << 0)
#       define RADEON_TCL_COMPUTE_DIFFUSE        (1 << 1)
#       define RADEON_TCL_COMPUTE_SPECULAR       (1 << 2)
#       define RADEON_TCL_FORCE_NAN_IF_COLOR_NAN (1 << 3)
#       define RADEON_TCL_FORCE_INORDER_PROC     (1 << 4)
#       define RADEON_TCL_TEX_INPUT_TEX_0        0
#       define RADEON_TCL_TEX_INPUT_TEX_1        1
#       define RADEON_TCL_TEX_INPUT_TEX_2        2
#       define RADEON_TCL_TEX_INPUT_TEX_3        3
#       define RADEON_TCL_TEX_COMPUTED_TEX_0     8
#       define RADEON_TCL_TEX_COMPUTED_TEX_1     9
#       define RADEON_TCL_TEX_COMPUTED_TEX_2     10
#       define RADEON_TCL_TEX_COMPUTED_TEX_3     11
#       define RADEON_TCL_TEX_0_OUTPUT_SHIFT     16
#       define RADEON_TCL_TEX_1_OUTPUT_SHIFT     20
#       define RADEON_TCL_TEX_2_OUTPUT_SHIFT     24
#       define RADEON_TCL_TEX_3_OUTPUT_SHIFT     28

#define RADEON_SE_TCL_PER_LIGHT_CTL_0       0x2270
#       define RADEON_LIGHT_0_ENABLE               (1 <<  0)
#       define RADEON_LIGHT_0_ENABLE_AMBIENT       (1 <<  1)
#       define RADEON_LIGHT_0_ENABLE_SPECULAR      (1 <<  2)
#       define RADEON_LIGHT_0_IS_LOCAL             (1 <<  3)
#       define RADEON_LIGHT_0_IS_SPOT              (1 <<  4)
#       define RADEON_LIGHT_0_DUAL_CONE            (1 <<  5)
#       define RADEON_LIGHT_0_ENABLE_RANGE_ATTEN   (1 <<  6)
#       define RADEON_LIGHT_0_CONSTANT_RANGE_ATTEN (1 <<  7)
#       define RADEON_LIGHT_0_SHIFT                0
#       define RADEON_LIGHT_1_ENABLE               (1 << 16)
#       define RADEON_LIGHT_1_ENABLE_AMBIENT       (1 << 17)
#       define RADEON_LIGHT_1_ENABLE_SPECULAR      (1 << 18)
#       define RADEON_LIGHT_1_IS_LOCAL             (1 << 19)
#       define RADEON_LIGHT_1_IS_SPOT              (1 << 20)
#       define RADEON_LIGHT_1_DUAL_CONE            (1 << 21)
#       define RADEON_LIGHT_1_ENABLE_RANGE_ATTEN   (1 << 22)
#       define RADEON_LIGHT_1_CONSTANT_RANGE_ATTEN (1 << 23)
#       define RADEON_LIGHT_1_SHIFT                16
#define RADEON_SE_TCL_PER_LIGHT_CTL_1       0x2274
#       define RADEON_LIGHT_2_SHIFT            0
#       define RADEON_LIGHT_3_SHIFT            16
#define RADEON_SE_TCL_PER_LIGHT_CTL_2       0x2278
#       define RADEON_LIGHT_4_SHIFT            0
#       define RADEON_LIGHT_5_SHIFT            16
#define RADEON_SE_TCL_PER_LIGHT_CTL_3       0x227c
#       define RADEON_LIGHT_6_SHIFT            0
#       define RADEON_LIGHT_7_SHIFT            16

#define RADEON_SE_TCL_SHININESS             0x2250

#define RADEON_SE_TCL_TEXTURE_PROC_CTL      0x2268
#       define RADEON_TEXGEN_TEXMAT_0_ENABLE      (1 << 0)
#       define RADEON_TEXGEN_TEXMAT_1_ENABLE      (1 << 1)
#       define RADEON_TEXGEN_TEXMAT_2_ENABLE      (1 << 2)
#       define RADEON_TEXGEN_TEXMAT_3_ENABLE      (1 << 3)
#       define RADEON_TEXMAT_0_ENABLE             (1 << 4)
#       define RADEON_TEXMAT_1_ENABLE             (1 << 5)
#       define RADEON_TEXMAT_2_ENABLE             (1 << 6)
#       define RADEON_TEXMAT_3_ENABLE             (1 << 7)
#       define RADEON_TEXGEN_INPUT_MASK           0xf
#       define RADEON_TEXGEN_INPUT_TEXCOORD_0     0
#       define RADEON_TEXGEN_INPUT_TEXCOORD_1     1
#       define RADEON_TEXGEN_INPUT_TEXCOORD_2     2
#       define RADEON_TEXGEN_INPUT_TEXCOORD_3     3
#       define RADEON_TEXGEN_INPUT_OBJ            4
#       define RADEON_TEXGEN_INPUT_EYE            5
#       define RADEON_TEXGEN_INPUT_EYE_NORMAL     6
#       define RADEON_TEXGEN_INPUT_EYE_REFLECT    7
#       define RADEON_TEXGEN_INPUT_EYE_NORMALIZED 8
#       define RADEON_TEXGEN_0_INPUT_SHIFT        16
#       define RADEON_TEXGEN_1_INPUT_SHIFT        20
#       define RADEON_TEXGEN_2_INPUT_SHIFT        24
#       define RADEON_TEXGEN_3_INPUT_SHIFT        28

#define RADEON_SE_TCL_UCP_VERT_BLEND_CTL    0x2264
#       define RADEON_UCP_IN_CLIP_SPACE            (1 <<  0)
#       define RADEON_UCP_IN_MODEL_SPACE           (1 <<  1)
#       define RADEON_UCP_ENABLE_0                 (1 <<  2)
#       define RADEON_UCP_ENABLE_1                 (1 <<  3)
#       define RADEON_UCP_ENABLE_2                 (1 <<  4)
#       define RADEON_UCP_ENABLE_3                 (1 <<  5)
#       define RADEON_UCP_ENABLE_4                 (1 <<  6)
#       define RADEON_UCP_ENABLE_5                 (1 <<  7)
#       define RADEON_TCL_FOG_MASK                 (3 <<  8)
#       define RADEON_TCL_FOG_DISABLE              (0 <<  8)
#       define RADEON_TCL_FOG_EXP                  (1 <<  8)
#       define RADEON_TCL_FOG_EXP2                 (2 <<  8)
#       define RADEON_TCL_FOG_LINEAR               (3 <<  8)
#       define RADEON_RNG_BASED_FOG                (1 << 10)
#       define RADEON_LIGHT_TWOSIDE                (1 << 11)
#       define RADEON_BLEND_OP_COUNT_MASK          (7 << 12)
#       define RADEON_BLEND_OP_COUNT_SHIFT         12
#       define RADEON_POSITION_BLEND_OP_ENABLE     (1 << 16)
#       define RADEON_NORMAL_BLEND_OP_ENABLE       (1 << 17)
#       define RADEON_VERTEX_BLEND_SRC_0_PRIMARY   (1 << 18)
#       define RADEON_VERTEX_BLEND_SRC_0_SECONDARY (1 << 18)
#       define RADEON_VERTEX_BLEND_SRC_1_PRIMARY   (1 << 19)
#       define RADEON_VERTEX_BLEND_SRC_1_SECONDARY (1 << 19)
#       define RADEON_VERTEX_BLEND_SRC_2_PRIMARY   (1 << 20)
#       define RADEON_VERTEX_BLEND_SRC_2_SECONDARY (1 << 20)
#       define RADEON_VERTEX_BLEND_SRC_3_PRIMARY   (1 << 21)
#       define RADEON_VERTEX_BLEND_SRC_3_SECONDARY (1 << 21)
#       define RADEON_VERTEX_BLEND_WGT_MINUS_ONE   (1 << 22)
#       define RADEON_CULL_FRONT_IS_CW             (0 << 28)
#       define RADEON_CULL_FRONT_IS_CCW            (1 << 28)
#       define RADEON_CULL_FRONT                   (1 << 29)
#       define RADEON_CULL_BACK                    (1 << 30)
#       define RADEON_FORCE_W_TO_ONE               (1 << 31)

#define RADEON_SE_VPORT_XSCALE              0x1d98
#define RADEON_SE_VPORT_XOFFSET             0x1d9c
#define RADEON_SE_VPORT_YSCALE              0x1da0
#define RADEON_SE_VPORT_YOFFSET             0x1da4
#define RADEON_SE_VPORT_ZSCALE              0x1da8
#define RADEON_SE_VPORT_ZOFFSET             0x1dac
#define RADEON_SE_ZBIAS_FACTOR              0x1db0
#define RADEON_SE_ZBIAS_CONSTANT            0x1db4

#define RADEON_SE_VTX_FMT                   0x2080
#       define RADEON_SE_VTX_FMT_XY         0x00000000
#       define RADEON_SE_VTX_FMT_W0         0x00000001
#       define RADEON_SE_VTX_FMT_FPCOLOR    0x00000002
#       define RADEON_SE_VTX_FMT_FPALPHA    0x00000004
#       define RADEON_SE_VTX_FMT_PKCOLOR    0x00000008
#       define RADEON_SE_VTX_FMT_FPSPEC     0x00000010
#       define RADEON_SE_VTX_FMT_FPFOG      0x00000020
#       define RADEON_SE_VTX_FMT_PKSPEC     0x00000040
#       define RADEON_SE_VTX_FMT_ST0        0x00000080
#       define RADEON_SE_VTX_FMT_ST1        0x00000100
#       define RADEON_SE_VTX_FMT_Q1         0x00000200
#       define RADEON_SE_VTX_FMT_ST2        0x00000400
#       define RADEON_SE_VTX_FMT_Q2         0x00000800
#       define RADEON_SE_VTX_FMT_ST3        0x00001000
#       define RADEON_SE_VTX_FMT_Q3         0x00002000
#       define RADEON_SE_VTX_FMT_Q0         0x00004000
#       define RADEON_SE_VTX_FMT_BLND_WEIGHT_CNT_MASK  0x00038000
#       define RADEON_SE_VTX_FMT_N0         0x00040000
#       define RADEON_SE_VTX_FMT_XY1        0x08000000
#       define RADEON_SE_VTX_FMT_Z1         0x10000000
#       define RADEON_SE_VTX_FMT_W1         0x20000000
#       define RADEON_SE_VTX_FMT_N1         0x40000000
#       define RADEON_SE_VTX_FMT_Z          0x80000000

#define RADEON_SE_VF_CNTL                             0x2084
#       define RADEON_VF_PRIM_TYPE_POINT_LIST         1
#       define RADEON_VF_PRIM_TYPE_LINE_LIST          2
#       define RADEON_VF_PRIM_TYPE_LINE_STRIP         3
#       define RADEON_VF_PRIM_TYPE_TRIANGLE_LIST      4
#       define RADEON_VF_PRIM_TYPE_TRIANGLE_FAN       5
#       define RADEON_VF_PRIM_TYPE_TRIANGLE_STRIP     6
#       define RADEON_VF_PRIM_TYPE_TRIANGLE_FLAG      7
#       define RADEON_VF_PRIM_TYPE_RECTANGLE_LIST     8
#       define RADEON_VF_PRIM_TYPE_POINT_LIST_3       9
#       define RADEON_VF_PRIM_TYPE_LINE_LIST_3        10
#       define RADEON_VF_PRIM_TYPE_SPIRIT_LIST        11
#       define RADEON_VF_PRIM_TYPE_LINE_LOOP          12
#       define RADEON_VF_PRIM_TYPE_QUAD_LIST          13
#       define RADEON_VF_PRIM_TYPE_QUAD_STRIP         14
#       define RADEON_VF_PRIM_TYPE_POLYGON            15
#       define RADEON_VF_PRIM_WALK_STATE              (0<<4)
#       define RADEON_VF_PRIM_WALK_INDEX              (1<<4)
#       define RADEON_VF_PRIM_WALK_LIST               (2<<4)
#       define RADEON_VF_PRIM_WALK_DATA               (3<<4)
#       define RADEON_VF_COLOR_ORDER_RGBA             (1<<6)
#       define RADEON_VF_RADEON_MODE                  (1<<8)
#       define RADEON_VF_TCL_OUTPUT_CTL_ENA           (1<<9)
#       define RADEON_VF_PROG_STREAM_ENA              (1<<10)
#       define RADEON_VF_INDEX_SIZE_SHIFT             11
#       define RADEON_VF_NUM_VERTICES_SHIFT           16

#define RADEON_SE_PORT_DATA0			0x2000
 
#define R200_SE_VAP_CNTL			0x2080
#       define R200_VAP_TCL_ENABLE		0x00000001
#       define R200_VAP_SINGLE_BUF_STATE_ENABLE	0x00000010
#       define R200_VAP_FORCE_W_TO_ONE		0x00010000
#       define R200_VAP_D3D_TEX_DEFAULT		0x00020000
#       define R200_VAP_VF_MAX_VTX_NUM__SHIFT	18
#       define R200_VAP_VF_MAX_VTX_NUM		(9 << 18)
#       define R200_VAP_DX_CLIP_SPACE_DEF	0x00400000
#define R200_VF_MAX_VTX_INDX			0x210c
#define R200_VF_MIN_VTX_INDX			0x2110
#define R200_SE_VTE_CNTL			0x20b0
#       define R200_VPORT_X_SCALE_ENA			0x00000001
#       define R200_VPORT_X_OFFSET_ENA			0x00000002
#       define R200_VPORT_Y_SCALE_ENA			0x00000004
#       define R200_VPORT_Y_OFFSET_ENA			0x00000008
#       define R200_VPORT_Z_SCALE_ENA			0x00000010
#       define R200_VPORT_Z_OFFSET_ENA			0x00000020
#       define R200_VTX_XY_FMT				0x00000100
#       define R200_VTX_Z_FMT				0x00000200
#       define R200_VTX_W0_FMT				0x00000400
#       define R200_VTX_W0_NORMALIZE			0x00000800
#       define R200_VTX_ST_DENORMALIZED		0x00001000
#define R200_SE_VAP_CNTL_STATUS			0x2140
#       define R200_VC_NO_SWAP			(0 << 0)
#       define R200_VC_16BIT_SWAP		(1 << 0)
#       define R200_VC_32BIT_SWAP		(2 << 0)
#define R200_RE_AUX_SCISSOR_CNTL		0x26f0
#       define R200_EXCLUSIVE_SCISSOR_0		0x01000000
#       define R200_EXCLUSIVE_SCISSOR_1		0x02000000
#       define R200_EXCLUSIVE_SCISSOR_2		0x04000000
#       define R200_SCISSOR_ENABLE_0		0x10000000
#       define R200_SCISSOR_ENABLE_1		0x20000000
#       define R200_SCISSOR_ENABLE_2		0x40000000
#define R200_PP_TXFILTER_0			0x2c00 
#define R200_PP_TXFILTER_1			0x2c20
#define R200_PP_TXFILTER_2			0x2c40
#define R200_PP_TXFILTER_3			0x2c60
#define R200_PP_TXFILTER_4			0x2c80
#define R200_PP_TXFILTER_5			0x2ca0
#       define R200_MAG_FILTER_NEAREST		(0  <<  0)
#       define R200_MAG_FILTER_LINEAR		(1  <<  0)
#       define R200_MAG_FILTER_MASK		(1  <<  0)
#       define R200_MIN_FILTER_NEAREST		(0  <<  1)
#       define R200_MIN_FILTER_LINEAR		(1  <<  1)
#       define R200_MIN_FILTER_NEAREST_MIP_NEAREST (2  <<  1)
#       define R200_MIN_FILTER_NEAREST_MIP_LINEAR (3  <<  1)
#       define R200_MIN_FILTER_LINEAR_MIP_NEAREST (6  <<  1)
#       define R200_MIN_FILTER_LINEAR_MIP_LINEAR (7  <<  1)
#       define R200_MIN_FILTER_ANISO_NEAREST	(8  <<  1)
#       define R200_MIN_FILTER_ANISO_LINEAR	(9  <<  1)
#       define R200_MIN_FILTER_ANISO_NEAREST_MIP_NEAREST (10 <<  1)
#       define R200_MIN_FILTER_ANISO_NEAREST_MIP_LINEAR (11 <<  1)
#       define R200_MIN_FILTER_MASK		(15 <<  1)
#       define R200_MAX_ANISO_1_TO_1		(0  <<  5)
#       define R200_MAX_ANISO_2_TO_1		(1  <<  5)
#       define R200_MAX_ANISO_4_TO_1		(2  <<  5)
#       define R200_MAX_ANISO_8_TO_1		(3  <<  5)
#       define R200_MAX_ANISO_16_TO_1		(4  <<  5)
#       define R200_MAX_ANISO_MASK		(7  <<  5)
#       define R200_MAX_MIP_LEVEL_MASK		(0x0f << 16)
#       define R200_MAX_MIP_LEVEL_SHIFT		16
#       define R200_YUV_TO_RGB			(1  << 20)
#       define R200_YUV_TEMPERATURE_COOL	(0  << 21)
#       define R200_YUV_TEMPERATURE_HOT		(1  << 21)
#       define R200_YUV_TEMPERATURE_MASK	(1  << 21)
#       define R200_WRAPEN_S			(1  << 22)
#       define R200_CLAMP_S_WRAP		(0  << 23)
#       define R200_CLAMP_S_MIRROR		(1  << 23)
#       define R200_CLAMP_S_CLAMP_LAST		(2  << 23)
#       define R200_CLAMP_S_MIRROR_CLAMP_LAST	(3  << 23)
#       define R200_CLAMP_S_CLAMP_BORDER	(4  << 23)
#       define R200_CLAMP_S_MIRROR_CLAMP_BORDER	(5  << 23)
#       define R200_CLAMP_S_CLAMP_GL		(6  << 23)
#       define R200_CLAMP_S_MIRROR_CLAMP_GL	(7  << 23)
#       define R200_CLAMP_S_MASK		(7  << 23)
#       define R200_WRAPEN_T			(1  << 26)
#       define R200_CLAMP_T_WRAP		(0  << 27)
#       define R200_CLAMP_T_MIRROR		(1  << 27)
#       define R200_CLAMP_T_CLAMP_LAST		(2  << 27)
#       define R200_CLAMP_T_MIRROR_CLAMP_LAST	(3  << 27)
#       define R200_CLAMP_T_CLAMP_BORDER	(4  << 27)
#       define R200_CLAMP_T_MIRROR_CLAMP_BORDER	(5  << 27)
#       define R200_CLAMP_T_CLAMP_GL		(6  << 27)
#       define R200_CLAMP_T_MIRROR_CLAMP_GL	(7  << 27)
#       define R200_CLAMP_T_MASK		(7  << 27)
#       define R200_KILL_LT_ZERO		(1  << 30)
#       define R200_BORDER_MODE_OGL		(0  << 31)
#       define R200_BORDER_MODE_D3D		(1  << 31)
#define R200_PP_TXFORMAT_0			0x2c04
#define R200_PP_TXFORMAT_1			0x2c24
#define R200_PP_TXFORMAT_2			0x2c44
#define R200_PP_TXFORMAT_3			0x2c64
#define R200_PP_TXFORMAT_4			0x2c84
#define R200_PP_TXFORMAT_5			0x2ca4
#       define R200_TXFORMAT_I8			(0 << 0)
#       define R200_TXFORMAT_AI88		(1 << 0)
#       define R200_TXFORMAT_RGB332		(2 << 0)
#       define R200_TXFORMAT_ARGB1555		(3 << 0)
#       define R200_TXFORMAT_RGB565		(4 << 0)
#       define R200_TXFORMAT_ARGB4444		(5 << 0)
#       define R200_TXFORMAT_ARGB8888		(6 << 0)
#       define R200_TXFORMAT_RGBA8888		(7 << 0)
#       define R200_TXFORMAT_Y8			(8 << 0)
#       define R200_TXFORMAT_AVYU4444		(9 << 0)
#       define R200_TXFORMAT_VYUY422		(10 << 0)
#       define R200_TXFORMAT_YVYU422		(11 << 0)
#       define R200_TXFORMAT_DXT1		(12 << 0)
#       define R200_TXFORMAT_DXT23		(14 << 0)
#       define R200_TXFORMAT_DXT45		(15 << 0)
#       define R200_TXFORMAT_ABGR8888		(22 << 0)
#       define R200_TXFORMAT_FORMAT_MASK	(31 <<	0)
#       define R200_TXFORMAT_FORMAT_SHIFT	0
#       define R200_TXFORMAT_ALPHA_IN_MAP	(1 << 6)
#       define R200_TXFORMAT_NON_POWER2		(1 << 7)
#       define R200_TXFORMAT_WIDTH_MASK		(15 <<	8)
#       define R200_TXFORMAT_WIDTH_SHIFT	8
#       define R200_TXFORMAT_HEIGHT_MASK	(15 << 12)
#       define R200_TXFORMAT_HEIGHT_SHIFT	12
#       define R200_TXFORMAT_F5_WIDTH_MASK	(15 << 16)	/* cube face 5 */
#       define R200_TXFORMAT_F5_WIDTH_SHIFT	16
#       define R200_TXFORMAT_F5_HEIGHT_MASK	(15 << 20)
#       define R200_TXFORMAT_F5_HEIGHT_SHIFT	20
#       define R200_TXFORMAT_ST_ROUTE_STQ0	(0 << 24)
#       define R200_TXFORMAT_ST_ROUTE_STQ1	(1 << 24)
#       define R200_TXFORMAT_ST_ROUTE_STQ2	(2 << 24)
#       define R200_TXFORMAT_ST_ROUTE_STQ3	(3 << 24)
#       define R200_TXFORMAT_ST_ROUTE_STQ4	(4 << 24)
#       define R200_TXFORMAT_ST_ROUTE_STQ5	(5 << 24)
#       define R200_TXFORMAT_ST_ROUTE_MASK	(7 << 24)
#       define R200_TXFORMAT_ST_ROUTE_SHIFT	24
#       define R200_TXFORMAT_ALPHA_MASK_ENABLE	(1 << 28)
#       define R200_TXFORMAT_CHROMA_KEY_ENABLE	(1 << 29)
#       define R200_TXFORMAT_CUBIC_MAP_ENABLE		(1 << 30)
#define R200_PP_TXFORMAT_X_0                    0x2c08
#define R200_PP_TXFORMAT_X_1                    0x2c28
#define R200_PP_TXFORMAT_X_2                    0x2c48
#define R200_PP_TXFORMAT_X_3                    0x2c68
#define R200_PP_TXFORMAT_X_4                    0x2c88
#define R200_PP_TXFORMAT_X_5                    0x2ca8

#define R200_PP_TXSIZE_0			0x2c0c /* NPOT only */
#define R200_PP_TXSIZE_1			0x2c2c /* NPOT only */
#define R200_PP_TXSIZE_2			0x2c4c /* NPOT only */
#define R200_PP_TXSIZE_3			0x2c6c /* NPOT only */
#define R200_PP_TXSIZE_4			0x2c8c /* NPOT only */
#define R200_PP_TXSIZE_5			0x2cac /* NPOT only */

#define R200_PP_TXPITCH_0                       0x2c10 /* NPOT only */
#define R200_PP_TXPITCH_1			0x2c30 /* NPOT only */
#define R200_PP_TXPITCH_2			0x2c50 /* NPOT only */
#define R200_PP_TXPITCH_3			0x2c70 /* NPOT only */
#define R200_PP_TXPITCH_4			0x2c90 /* NPOT only */
#define R200_PP_TXPITCH_5			0x2cb0 /* NPOT only */

#define R200_PP_TXOFFSET_0			0x2d00
#       define R200_TXO_ENDIAN_NO_SWAP		(0 << 0)
#       define R200_TXO_ENDIAN_BYTE_SWAP	(1 << 0)
#       define R200_TXO_ENDIAN_WORD_SWAP	(2 << 0)
#       define R200_TXO_ENDIAN_HALFDW_SWAP	(3 << 0)
#       define R200_TXO_MACRO_LINEAR		(0 << 2)
#       define R200_TXO_MACRO_TILE		(1 << 2)
#       define R200_TXO_MICRO_LINEAR		(0 << 3)
#       define R200_TXO_MICRO_TILE		(1 << 3)
#       define R200_TXO_OFFSET_MASK		0xffffffe0
#       define R200_TXO_OFFSET_SHIFT		5
#define R200_PP_TXOFFSET_1			0x2d18
#define R200_PP_TXOFFSET_2			0x2d30
#define R200_PP_TXOFFSET_3			0x2d48
#define R200_PP_TXOFFSET_4			0x2d60
#define R200_PP_TXOFFSET_5			0x2d78

#define R200_PP_TFACTOR_0			0x2ee0
#define R200_PP_TFACTOR_1			0x2ee4
#define R200_PP_TFACTOR_2			0x2ee8
#define R200_PP_TFACTOR_3			0x2eec
#define R200_PP_TFACTOR_4			0x2ef0
#define R200_PP_TFACTOR_5			0x2ef4

#define R200_PP_TXCBLEND_0			0x2f00
#       define R200_TXC_ARG_A_ZERO		(0)
#       define R200_TXC_ARG_A_CURRENT_COLOR	(2)
#       define R200_TXC_ARG_A_CURRENT_ALPHA	(3)
#       define R200_TXC_ARG_A_DIFFUSE_COLOR	(4)
#       define R200_TXC_ARG_A_DIFFUSE_ALPHA	(5)
#       define R200_TXC_ARG_A_SPECULAR_COLOR	(6)
#       define R200_TXC_ARG_A_SPECULAR_ALPHA	(7)
#       define R200_TXC_ARG_A_TFACTOR_COLOR	(8)
#       define R200_TXC_ARG_A_TFACTOR_ALPHA	(9)
#       define R200_TXC_ARG_A_R0_COLOR		(10)
#       define R200_TXC_ARG_A_R0_ALPHA		(11)
#       define R200_TXC_ARG_A_R1_COLOR		(12)
#       define R200_TXC_ARG_A_R1_ALPHA		(13)
#       define R200_TXC_ARG_A_R2_COLOR		(14)
#       define R200_TXC_ARG_A_R2_ALPHA		(15)
#       define R200_TXC_ARG_A_R3_COLOR		(16)
#       define R200_TXC_ARG_A_R3_ALPHA		(17)
#       define R200_TXC_ARG_A_R4_COLOR		(18)
#       define R200_TXC_ARG_A_R4_ALPHA		(19)
#       define R200_TXC_ARG_A_R5_COLOR		(20)
#       define R200_TXC_ARG_A_R5_ALPHA		(21)
#       define R200_TXC_ARG_A_TFACTOR1_COLOR	(26)
#       define R200_TXC_ARG_A_TFACTOR1_ALPHA	(27)
#       define R200_TXC_ARG_A_MASK		(31 << 0)
#       define R200_TXC_ARG_A_SHIFT		0
#       define R200_TXC_ARG_B_ZERO		(0 << 5)
#       define R200_TXC_ARG_B_CURRENT_COLOR	(2 << 5)
#       define R200_TXC_ARG_B_CURRENT_ALPHA	(3 << 5)
#       define R200_TXC_ARG_B_DIFFUSE_COLOR	(4 << 5)
#       define R200_TXC_ARG_B_DIFFUSE_ALPHA	(5 << 5)
#       define R200_TXC_ARG_B_SPECULAR_COLOR	(6 << 5)
#       define R200_TXC_ARG_B_SPECULAR_ALPHA	(7 << 5)
#       define R200_TXC_ARG_B_TFACTOR_COLOR	(8 << 5)
#       define R200_TXC_ARG_B_TFACTOR_ALPHA	(9 << 5)
#       define R200_TXC_ARG_B_R0_COLOR		(10 << 5)
#       define R200_TXC_ARG_B_R0_ALPHA		(11 << 5)
#       define R200_TXC_ARG_B_R1_COLOR		(12 << 5)
#       define R200_TXC_ARG_B_R1_ALPHA		(13 << 5)
#       define R200_TXC_ARG_B_R2_COLOR		(14 << 5)
#       define R200_TXC_ARG_B_R2_ALPHA		(15 << 5)
#       define R200_TXC_ARG_B_R3_COLOR		(16 << 5)
#       define R200_TXC_ARG_B_R3_ALPHA		(17 << 5)
#       define R200_TXC_ARG_B_R4_COLOR		(18 << 5)
#       define R200_TXC_ARG_B_R4_ALPHA		(19 << 5)
#       define R200_TXC_ARG_B_R5_COLOR		(20 << 5)
#       define R200_TXC_ARG_B_R5_ALPHA		(21 << 5)
#       define R200_TXC_ARG_B_TFACTOR1_COLOR	(26 << 5)
#       define R200_TXC_ARG_B_TFACTOR1_ALPHA	(27 << 5)
#       define R200_TXC_ARG_B_MASK		(31 << 5)
#       define R200_TXC_ARG_B_SHIFT		5
#       define R200_TXC_ARG_C_ZERO		(0 << 10)
#       define R200_TXC_ARG_C_CURRENT_COLOR	(2 << 10)
#       define R200_TXC_ARG_C_CURRENT_ALPHA	(3 << 10)
#       define R200_TXC_ARG_C_DIFFUSE_COLOR	(4 << 10)
#       define R200_TXC_ARG_C_DIFFUSE_ALPHA	(5 << 10)
#       define R200_TXC_ARG_C_SPECULAR_COLOR	(6 << 10)
#       define R200_TXC_ARG_C_SPECULAR_ALPHA	(7 << 10)
#       define R200_TXC_ARG_C_TFACTOR_COLOR	(8 << 10)
#       define R200_TXC_ARG_C_TFACTOR_ALPHA	(9 << 10)
#       define R200_TXC_ARG_C_R0_COLOR		(10 << 10)
#       define R200_TXC_ARG_C_R0_ALPHA		(11 << 10)
#       define R200_TXC_ARG_C_R1_COLOR		(12 << 10)
#       define R200_TXC_ARG_C_R1_ALPHA		(13 << 10)
#       define R200_TXC_ARG_C_R2_COLOR		(14 << 10)
#       define R200_TXC_ARG_C_R2_ALPHA		(15 << 10)
#       define R200_TXC_ARG_C_R3_COLOR		(16 << 10)
#       define R200_TXC_ARG_C_R3_ALPHA		(17 << 10)
#       define R200_TXC_ARG_C_R4_COLOR		(18 << 10)
#       define R200_TXC_ARG_C_R4_ALPHA		(19 << 10)
#       define R200_TXC_ARG_C_R5_COLOR		(20 << 10)
#       define R200_TXC_ARG_C_R5_ALPHA		(21 << 10)
#       define R200_TXC_ARG_C_TFACTOR1_COLOR	(26 << 10)
#       define R200_TXC_ARG_C_TFACTOR1_ALPHA	(27 << 10)
#       define R200_TXC_ARG_C_MASK		(31 << 10)
#       define R200_TXC_ARG_C_SHIFT		10
#       define R200_TXC_COMP_ARG_A		(1 << 16)
#       define R200_TXC_COMP_ARG_A_SHIFT	(16)
#       define R200_TXC_BIAS_ARG_A		(1 << 17)
#       define R200_TXC_SCALE_ARG_A		(1 << 18)
#       define R200_TXC_NEG_ARG_A		(1 << 19)
#       define R200_TXC_COMP_ARG_B		(1 << 20)
#       define R200_TXC_COMP_ARG_B_SHIFT	(20)
#       define R200_TXC_BIAS_ARG_B		(1 << 21)
#       define R200_TXC_SCALE_ARG_B		(1 << 22)
#       define R200_TXC_NEG_ARG_B		(1 << 23)
#       define R200_TXC_COMP_ARG_C		(1 << 24)
#       define R200_TXC_COMP_ARG_C_SHIFT	(24)
#       define R200_TXC_BIAS_ARG_C		(1 << 25)
#       define R200_TXC_SCALE_ARG_C		(1 << 26)
#       define R200_TXC_NEG_ARG_C		(1 << 27)
#       define R200_TXC_OP_MADD			(0 << 28)
#       define R200_TXC_OP_CND0			(2 << 28)
#       define R200_TXC_OP_LERP			(3 << 28)
#       define R200_TXC_OP_DOT3			(4 << 28)
#       define R200_TXC_OP_DOT4			(5 << 28)
#       define R200_TXC_OP_CONDITIONAL		(6 << 28)
#       define R200_TXC_OP_DOT2_ADD		(7 << 28)
#       define R200_TXC_OP_MASK			(7 << 28)
#define R200_PP_TXCBLEND2_0		0x2f04
#       define R200_TXC_TFACTOR_SEL_SHIFT	0
#       define R200_TXC_TFACTOR_SEL_MASK	0x7
#       define R200_TXC_TFACTOR1_SEL_SHIFT	4
#       define R200_TXC_TFACTOR1_SEL_MASK	(0x7 << 4)
#       define R200_TXC_SCALE_SHIFT		8
#       define R200_TXC_SCALE_MASK		(7 << 8)
#       define R200_TXC_SCALE_1X		(0 << 8)
#       define R200_TXC_SCALE_2X		(1 << 8)
#       define R200_TXC_SCALE_4X		(2 << 8)
#       define R200_TXC_SCALE_8X		(3 << 8)
#       define R200_TXC_SCALE_INV2		(5 << 8)
#       define R200_TXC_SCALE_INV4		(6 << 8)
#       define R200_TXC_SCALE_INV8		(7 << 8)
#       define R200_TXC_CLAMP_SHIFT		12
#       define R200_TXC_CLAMP_MASK		(3 << 12)
#       define R200_TXC_CLAMP_WRAP		(0 << 12)
#       define R200_TXC_CLAMP_0_1		(1 << 12)
#       define R200_TXC_CLAMP_8_8		(2 << 12)
#       define R200_TXC_OUTPUT_REG_MASK		(7 << 16)
#       define R200_TXC_OUTPUT_REG_NONE		(0 << 16)
#       define R200_TXC_OUTPUT_REG_R0		(1 << 16)
#       define R200_TXC_OUTPUT_REG_R1		(2 << 16)
#       define R200_TXC_OUTPUT_REG_R2		(3 << 16)
#       define R200_TXC_OUTPUT_REG_R3		(4 << 16)
#       define R200_TXC_OUTPUT_REG_R4		(5 << 16)
#       define R200_TXC_OUTPUT_REG_R5		(6 << 16)
#       define R200_TXC_OUTPUT_MASK_MASK	(7 << 20)
#       define R200_TXC_OUTPUT_MASK_RGB		(0 << 20)
#       define R200_TXC_OUTPUT_MASK_RG		(1 << 20)
#       define R200_TXC_OUTPUT_MASK_RB		(2 << 20)
#       define R200_TXC_OUTPUT_MASK_R		(3 << 20)
#       define R200_TXC_OUTPUT_MASK_GB		(4 << 20)
#       define R200_TXC_OUTPUT_MASK_G		(5 << 20)
#       define R200_TXC_OUTPUT_MASK_B		(6 << 20)
#       define R200_TXC_OUTPUT_MASK_NONE	(7 << 20)
#       define R200_TXC_REPL_NORMAL		0
#       define R200_TXC_REPL_RED		1
#       define R200_TXC_REPL_GREEN		2
#       define R200_TXC_REPL_BLUE		3
#       define R200_TXC_REPL_ARG_A_SHIFT	26
#       define R200_TXC_REPL_ARG_A_MASK		(3 << 26)
#       define R200_TXC_REPL_ARG_B_SHIFT	28
#       define R200_TXC_REPL_ARG_B_MASK		(3 << 28)
#       define R200_TXC_REPL_ARG_C_SHIFT	30
#       define R200_TXC_REPL_ARG_C_MASK		(3 << 30)
#define R200_PP_TXABLEND_0			0x2f08
#       define R200_TXA_ARG_A_ZERO		(0)
#       define R200_TXA_ARG_A_CURRENT_ALPHA	(2) /* guess */
#       define R200_TXA_ARG_A_CURRENT_BLUE	(3) /* guess */
#       define R200_TXA_ARG_A_DIFFUSE_ALPHA	(4)
#       define R200_TXA_ARG_A_DIFFUSE_BLUE	(5)
#       define R200_TXA_ARG_A_SPECULAR_ALPHA	(6)
#       define R200_TXA_ARG_A_SPECULAR_BLUE	(7)
#       define R200_TXA_ARG_A_TFACTOR_ALPHA	(8)
#       define R200_TXA_ARG_A_TFACTOR_BLUE	(9)
#       define R200_TXA_ARG_A_R0_ALPHA		(10)
#       define R200_TXA_ARG_A_R0_BLUE		(11)
#       define R200_TXA_ARG_A_R1_ALPHA		(12)
#       define R200_TXA_ARG_A_R1_BLUE		(13)
#       define R200_TXA_ARG_A_R2_ALPHA		(14)
#       define R200_TXA_ARG_A_R2_BLUE		(15)
#       define R200_TXA_ARG_A_R3_ALPHA		(16)
#       define R200_TXA_ARG_A_R3_BLUE		(17)
#       define R200_TXA_ARG_A_R4_ALPHA		(18)
#       define R200_TXA_ARG_A_R4_BLUE		(19)
#       define R200_TXA_ARG_A_R5_ALPHA		(20)
#       define R200_TXA_ARG_A_R5_BLUE		(21)
#       define R200_TXA_ARG_A_TFACTOR1_ALPHA	(26)
#       define R200_TXA_ARG_A_TFACTOR1_BLUE	(27)
#       define R200_TXA_ARG_A_MASK		(31 << 0)
#       define R200_TXA_ARG_A_SHIFT		0
#       define R200_TXA_ARG_B_ZERO		(0 << 5)
#       define R200_TXA_ARG_B_CURRENT_ALPHA	(2 << 5) /* guess */
#       define R200_TXA_ARG_B_CURRENT_BLUE	(3 << 5) /* guess */
#       define R200_TXA_ARG_B_DIFFUSE_ALPHA	(4 << 5)
#       define R200_TXA_ARG_B_DIFFUSE_BLUE	(5 << 5)
#       define R200_TXA_ARG_B_SPECULAR_ALPHA	(6 << 5)
#       define R200_TXA_ARG_B_SPECULAR_BLUE	(7 << 5)
#       define R200_TXA_ARG_B_TFACTOR_ALPHA	(8 << 5)
#       define R200_TXA_ARG_B_TFACTOR_BLUE	(9 << 5)
#       define R200_TXA_ARG_B_R0_ALPHA		(10 << 5)
#       define R200_TXA_ARG_B_R0_BLUE		(11 << 5)
#       define R200_TXA_ARG_B_R1_ALPHA		(12 << 5)
#       define R200_TXA_ARG_B_R1_BLUE		(13 << 5)
#       define R200_TXA_ARG_B_R2_ALPHA		(14 << 5)
#       define R200_TXA_ARG_B_R2_BLUE		(15 << 5)
#       define R200_TXA_ARG_B_R3_ALPHA		(16 << 5)
#       define R200_TXA_ARG_B_R3_BLUE		(17 << 5)
#       define R200_TXA_ARG_B_R4_ALPHA		(18 << 5)
#       define R200_TXA_ARG_B_R4_BLUE		(19 << 5)
#       define R200_TXA_ARG_B_R5_ALPHA		(20 << 5)
#       define R200_TXA_ARG_B_R5_BLUE		(21 << 5)
#       define R200_TXA_ARG_B_TFACTOR1_ALPHA	(26 << 5)
#       define R200_TXA_ARG_B_TFACTOR1_BLUE	(27 << 5)
#       define R200_TXA_ARG_B_MASK		(31 << 5)
#       define R200_TXA_ARG_B_SHIFT			5
#       define R200_TXA_ARG_C_ZERO		(0 << 10)
#       define R200_TXA_ARG_C_CURRENT_ALPHA	(2 << 10) /* guess */
#       define R200_TXA_ARG_C_CURRENT_BLUE	(3 << 10) /* guess */
#       define R200_TXA_ARG_C_DIFFUSE_ALPHA	(4 << 10)
#       define R200_TXA_ARG_C_DIFFUSE_BLUE	(5 << 10)
#       define R200_TXA_ARG_C_SPECULAR_ALPHA	(6 << 10)
#       define R200_TXA_ARG_C_SPECULAR_BLUE	(7 << 10)
#       define R200_TXA_ARG_C_TFACTOR_ALPHA	(8 << 10)
#       define R200_TXA_ARG_C_TFACTOR_BLUE	(9 << 10)
#       define R200_TXA_ARG_C_R0_ALPHA		(10 << 10)
#       define R200_TXA_ARG_C_R0_BLUE		(11 << 10)
#       define R200_TXA_ARG_C_R1_ALPHA		(12 << 10)
#       define R200_TXA_ARG_C_R1_BLUE		(13 << 10)
#       define R200_TXA_ARG_C_R2_ALPHA		(14 << 10)
#       define R200_TXA_ARG_C_R2_BLUE		(15 << 10)
#       define R200_TXA_ARG_C_R3_ALPHA		(16 << 10)
#       define R200_TXA_ARG_C_R3_BLUE		(17 << 10)
#       define R200_TXA_ARG_C_R4_ALPHA		(18 << 10)
#       define R200_TXA_ARG_C_R4_BLUE		(19 << 10)
#       define R200_TXA_ARG_C_R5_ALPHA		(20 << 10)
#       define R200_TXA_ARG_C_R5_BLUE		(21 << 10)
#       define R200_TXA_ARG_C_TFACTOR1_ALPHA	(26 << 10)
#       define R200_TXA_ARG_C_TFACTOR1_BLUE	(27 << 10)
#       define R200_TXA_ARG_C_MASK		(31 << 10)
#       define R200_TXA_ARG_C_SHIFT		10
#       define R200_TXA_COMP_ARG_A		(1 << 16)
#       define R200_TXA_COMP_ARG_A_SHIFT	(16)
#       define R200_TXA_BIAS_ARG_A		(1 << 17)
#       define R200_TXA_SCALE_ARG_A		(1 << 18)
#       define R200_TXA_NEG_ARG_A		(1 << 19)
#       define R200_TXA_COMP_ARG_B		(1 << 20)
#       define R200_TXA_COMP_ARG_B_SHIFT	(20)
#       define R200_TXA_BIAS_ARG_B		(1 << 21)
#       define R200_TXA_SCALE_ARG_B		(1 << 22)
#       define R200_TXA_NEG_ARG_B		(1 << 23)
#       define R200_TXA_COMP_ARG_C		(1 << 24)
#       define R200_TXA_COMP_ARG_C_SHIFT	(24)
#       define R200_TXA_BIAS_ARG_C		(1 << 25)
#       define R200_TXA_SCALE_ARG_C		(1 << 26)
#       define R200_TXA_NEG_ARG_C		(1 << 27)
#       define R200_TXA_OP_MADD			(0 << 28)
#       define R200_TXA_OP_CND0			(2 << 28)
#       define R200_TXA_OP_LERP			(3 << 28)
#       define R200_TXA_OP_CONDITIONAL		(6 << 28)
#       define R200_TXA_OP_MASK			(7 << 28)
#define R200_PP_TXABLEND2_0			0x2f0c
#       define R200_TXA_TFACTOR_SEL_SHIFT	0
#       define R200_TXA_TFACTOR_SEL_MASK	0x7
#       define R200_TXA_TFACTOR1_SEL_SHIFT	4
#       define R200_TXA_TFACTOR1_SEL_MASK	(0x7 << 4)
#       define R200_TXA_SCALE_SHIFT		8
#       define R200_TXA_SCALE_MASK		(7 << 8)
#       define R200_TXA_SCALE_1X		(0 << 8)
#       define R200_TXA_SCALE_2X		(1 << 8)
#       define R200_TXA_SCALE_4X		(2 << 8)
#       define R200_TXA_SCALE_8X		(3 << 8)
#       define R200_TXA_SCALE_INV2		(5 << 8)
#       define R200_TXA_SCALE_INV4		(6 << 8)
#       define R200_TXA_SCALE_INV8		(7 << 8)
#       define R200_TXA_CLAMP_SHIFT		12
#       define R200_TXA_CLAMP_MASK		(3 << 12)
#       define R200_TXA_CLAMP_WRAP		(0 << 12)
#       define R200_TXA_CLAMP_0_1		(1 << 12)
#       define R200_TXA_CLAMP_8_8		(2 << 12)
#       define R200_TXA_OUTPUT_REG_MASK		(7 << 16)
#       define R200_TXA_OUTPUT_REG_NONE		(0 << 16)
#       define R200_TXA_OUTPUT_REG_R0		(1 << 16)
#       define R200_TXA_OUTPUT_REG_R1		(2 << 16)
#       define R200_TXA_OUTPUT_REG_R2		(3 << 16)
#       define R200_TXA_OUTPUT_REG_R3		(4 << 16)
#       define R200_TXA_OUTPUT_REG_R4		(5 << 16)
#       define R200_TXA_OUTPUT_REG_R5		(6 << 16)
#       define R200_TXA_DOT_ALPHA		(1 << 20)
#       define R200_TXA_REPL_NORMAL		0
#       define R200_TXA_REPL_RED		1
#       define R200_TXA_REPL_GREEN		2
#       define R200_TXA_REPL_ARG_A_SHIFT	26
#       define R200_TXA_REPL_ARG_A_MASK		(3 << 26)
#       define R200_TXA_REPL_ARG_B_SHIFT	28
#       define R200_TXA_REPL_ARG_B_MASK		(3 << 28)
#       define R200_TXA_REPL_ARG_C_SHIFT	30
#       define R200_TXA_REPL_ARG_C_MASK		(3 << 30)
#define R200_PP_TXCBLEND_1			0x2f10
#define R200_PP_TXCBLEND2_1			0x2f14
#define R200_PP_TXABLEND_1			0x2f18
#define R200_PP_TXABLEND2_1			0x2f1c
#define R200_PP_TXCBLEND_2			0x2f20
#define R200_PP_TXCBLEND2_2			0x2f24
#define R200_PP_TXABLEND_2			0x2f28
#define R200_PP_TXABLEND2_2			0x2f2c
#define R200_PP_TXCBLEND_3			0x2f30
#define R200_PP_TXCBLEND2_3			0x2f34
#define R200_PP_TXABLEND_3			0x2f38
#define R200_PP_TXABLEND2_3			0x2f3c

#define R200_SE_VTX_FMT_0			0x2088
#       define R200_VTX_XY			0 /* always have xy */
#       define R200_VTX_Z0			(1<<0)
#       define R200_VTX_W0			(1<<1)
#       define R200_VTX_WEIGHT_COUNT_SHIFT	(2)
#       define R200_VTX_PV_MATRIX_SEL		(1<<5)
#       define R200_VTX_N0			(1<<6)
#       define R200_VTX_POINT_SIZE		(1<<7)
#       define R200_VTX_DISCRETE_FOG		(1<<8)
#       define R200_VTX_SHININESS_0		(1<<9)
#       define R200_VTX_SHININESS_1		(1<<10)
#       define   R200_VTX_COLOR_NOT_PRESENT	0
#       define   R200_VTX_PK_RGBA		1
#       define   R200_VTX_FP_RGB		2
#       define   R200_VTX_FP_RGBA		3
#       define   R200_VTX_COLOR_MASK		3
#       define R200_VTX_COLOR_0_SHIFT		11
#       define R200_VTX_COLOR_1_SHIFT		13
#       define R200_VTX_COLOR_2_SHIFT		15
#       define R200_VTX_COLOR_3_SHIFT		17
#       define R200_VTX_COLOR_4_SHIFT		19
#       define R200_VTX_COLOR_5_SHIFT		21
#       define R200_VTX_COLOR_6_SHIFT		23
#       define R200_VTX_COLOR_7_SHIFT		25
#       define R200_VTX_XY1			(1<<28)
#       define R200_VTX_Z1			(1<<29)
#       define R200_VTX_W1			(1<<30)
#       define R200_VTX_N1			(1<<31)
#define R200_SE_VTX_FMT_1			0x208c
#       define R200_VTX_TEX0_COMP_CNT_SHIFT	0
#       define R200_VTX_TEX1_COMP_CNT_SHIFT	3
#       define R200_VTX_TEX2_COMP_CNT_SHIFT	6
#       define R200_VTX_TEX3_COMP_CNT_SHIFT	9
#       define R200_VTX_TEX4_COMP_CNT_SHIFT	12
#       define R200_VTX_TEX5_COMP_CNT_SHIFT	15

#define R200_SE_TCL_OUTPUT_VTX_FMT_0		0x2090
#define R200_SE_TCL_OUTPUT_VTX_FMT_1		0x2094
#define R200_SE_TCL_OUTPUT_VTX_COMP_SEL		0x2250
#       define R200_OUTPUT_XYZW			(1<<0)
#       define R200_OUTPUT_COLOR_0		(1<<8)
#       define R200_OUTPUT_COLOR_1		(1<<9)
#       define R200_OUTPUT_TEX_0		(1<<16)
#       define R200_OUTPUT_TEX_1		(1<<17)
#       define R200_OUTPUT_TEX_2		(1<<18)
#       define R200_OUTPUT_TEX_3		(1<<19)
#       define R200_OUTPUT_TEX_4		(1<<20)
#       define R200_OUTPUT_TEX_5		(1<<21)
#       define R200_OUTPUT_TEX_MASK		(0x3f<<16)
#       define R200_OUTPUT_DISCRETE_FOG		(1<<24)
#       define R200_OUTPUT_PT_SIZE		(1<<25)
#       define R200_FORCE_INORDER_PROC		(1<<31)
#define R200_PP_CNTL_X				0x2cc4
#define R200_PP_TXMULTI_CTL_0			0x2c1c
#define R200_SE_VTX_STATE_CNTL			0x2180
#       define R200_UPDATE_USER_COLOR_0_ENA_MASK (1<<16)

				/* Registers for CP and Microcode Engine */
#define RADEON_CP_ME_RAM_ADDR               0x07d4
#define RADEON_CP_ME_RAM_RADDR              0x07d8
#define RADEON_CP_ME_RAM_DATAH              0x07dc
#define RADEON_CP_ME_RAM_DATAL              0x07e0

#define RADEON_CP_RB_BASE                   0x0700
#define RADEON_CP_RB_CNTL                   0x0704
#define RADEON_CP_RB_RPTR_ADDR              0x070c
#define RADEON_CP_RB_RPTR                   0x0710
#define RADEON_CP_RB_WPTR                   0x0714

#define RADEON_CP_IB_BASE                   0x0738
#define RADEON_CP_IB_BUFSZ                  0x073c

#define RADEON_CP_CSQ_CNTL                  0x0740
#       define RADEON_CSQ_CNT_PRIMARY_MASK     (0xff << 0)
#       define RADEON_CSQ_PRIDIS_INDDIS        (0    << 28)
#       define RADEON_CSQ_PRIPIO_INDDIS        (1    << 28)
#       define RADEON_CSQ_PRIBM_INDDIS         (2    << 28)
#       define RADEON_CSQ_PRIPIO_INDBM         (3    << 28)
#       define RADEON_CSQ_PRIBM_INDBM          (4    << 28)
#       define RADEON_CSQ_PRIPIO_INDPIO        (15   << 28)
#define RADEON_CP_CSQ_STAT                  0x07f8
#       define RADEON_CSQ_RPTR_PRIMARY_MASK    (0xff <<  0)
#       define RADEON_CSQ_WPTR_PRIMARY_MASK    (0xff <<  8)
#       define RADEON_CSQ_RPTR_INDIRECT_MASK   (0xff << 16)
#       define RADEON_CSQ_WPTR_INDIRECT_MASK   (0xff << 24)
#define RADEON_CP_CSQ_ADDR                  0x07f0
#define RADEON_CP_CSQ_DATA                  0x07f4
#define RADEON_CP_CSQ_APER_PRIMARY          0x1000
#define RADEON_CP_CSQ_APER_INDIRECT         0x1300

#define RADEON_CP_RB_WPTR_DELAY             0x0718
#       define RADEON_PRE_WRITE_TIMER_SHIFT    0
#       define RADEON_PRE_WRITE_LIMIT_SHIFT    23

#define RADEON_AIC_CNTL                     0x01d0
#       define RADEON_PCIGART_TRANSLATE_EN     (1 << 0)
#define RADEON_AIC_LO_ADDR                  0x01dc



				/* Constants */
#define RADEON_LAST_FRAME_REG               RADEON_GUI_SCRATCH_REG0
#define RADEON_LAST_CLEAR_REG               RADEON_GUI_SCRATCH_REG2



				/* CP packet types */
#define RADEON_CP_PACKET0                           0x00000000
#define RADEON_CP_PACKET1                           0x40000000
#define RADEON_CP_PACKET2                           0x80000000
#define RADEON_CP_PACKET3                           0xC0000000
#       define RADEON_CP_PACKET_MASK                0xC0000000
#       define RADEON_CP_PACKET_COUNT_MASK          0x3fff0000
#       define RADEON_CP_PACKET_MAX_DWORDS          (1 << 12)
#       define RADEON_CP_PACKET0_REG_MASK           0x000007ff
#       define RADEON_CP_PACKET1_REG0_MASK          0x000007ff
#       define RADEON_CP_PACKET1_REG1_MASK          0x003ff800

#define RADEON_CP_PACKET0_ONE_REG_WR                0x00008000

#define RADEON_CP_PACKET3_NOP                       0xC0001000
#define RADEON_CP_PACKET3_NEXT_CHAR                 0xC0001900
#define RADEON_CP_PACKET3_PLY_NEXTSCAN              0xC0001D00
#define RADEON_CP_PACKET3_SET_SCISSORS              0xC0001E00
#define RADEON_CP_PACKET3_3D_RNDR_GEN_INDX_PRIM     0xC0002300
#define RADEON_CP_PACKET3_LOAD_MICROCODE            0xC0002400
#define RADEON_CP_PACKET3_WAIT_FOR_IDLE             0xC0002600
#define RADEON_CP_PACKET3_3D_DRAW_VBUF              0xC0002800
#define RADEON_CP_PACKET3_3D_DRAW_IMMD              0xC0002900
#define RADEON_CP_PACKET3_3D_DRAW_INDX              0xC0002A00
#define RADEON_CP_PACKET3_LOAD_PALETTE              0xC0002C00
#define R200_CP_PACKET3_3D_DRAW_IMMD_2              0xc0003500
#define RADEON_CP_PACKET3_3D_LOAD_VBPNTR            0xC0002F00
#define RADEON_CP_PACKET3_CNTL_PAINT                0xC0009100
#define RADEON_CP_PACKET3_CNTL_BITBLT               0xC0009200
#define RADEON_CP_PACKET3_CNTL_SMALLTEXT            0xC0009300
#define RADEON_CP_PACKET3_CNTL_HOSTDATA_BLT         0xC0009400
#define RADEON_CP_PACKET3_CNTL_POLYLINE             0xC0009500
#define RADEON_CP_PACKET3_CNTL_POLYSCANLINES        0xC0009800
#define RADEON_CP_PACKET3_CNTL_PAINT_MULTI          0xC0009A00
#define RADEON_CP_PACKET3_CNTL_BITBLT_MULTI         0xC0009B00
#define RADEON_CP_PACKET3_CNTL_TRANS_BITBLT         0xC0009C00


#define RADEON_CP_VC_FRMT_XY                        0x00000000
#define RADEON_CP_VC_FRMT_W0                        0x00000001
#define RADEON_CP_VC_FRMT_FPCOLOR                   0x00000002
#define RADEON_CP_VC_FRMT_FPALPHA                   0x00000004
#define RADEON_CP_VC_FRMT_PKCOLOR                   0x00000008
#define RADEON_CP_VC_FRMT_FPSPEC                    0x00000010
#define RADEON_CP_VC_FRMT_FPFOG                     0x00000020
#define RADEON_CP_VC_FRMT_PKSPEC                    0x00000040
#define RADEON_CP_VC_FRMT_ST0                       0x00000080
#define RADEON_CP_VC_FRMT_ST1                       0x00000100
#define RADEON_CP_VC_FRMT_Q1                        0x00000200
#define RADEON_CP_VC_FRMT_ST2                       0x00000400
#define RADEON_CP_VC_FRMT_Q2                        0x00000800
#define RADEON_CP_VC_FRMT_ST3                       0x00001000
#define RADEON_CP_VC_FRMT_Q3                        0x00002000
#define RADEON_CP_VC_FRMT_Q0                        0x00004000
#define RADEON_CP_VC_FRMT_BLND_WEIGHT_CNT_MASK      0x00038000
#define RADEON_CP_VC_FRMT_N0                        0x00040000
#define RADEON_CP_VC_FRMT_XY1                       0x08000000
#define RADEON_CP_VC_FRMT_Z1                        0x10000000
#define RADEON_CP_VC_FRMT_W1                        0x20000000
#define RADEON_CP_VC_FRMT_N1                        0x40000000
#define RADEON_CP_VC_FRMT_Z                         0x80000000

#define RADEON_CP_VC_CNTL_PRIM_TYPE_NONE            0x00000000
#define RADEON_CP_VC_CNTL_PRIM_TYPE_POINT           0x00000001
#define RADEON_CP_VC_CNTL_PRIM_TYPE_LINE            0x00000002
#define RADEON_CP_VC_CNTL_PRIM_TYPE_LINE_STRIP      0x00000003
#define RADEON_CP_VC_CNTL_PRIM_TYPE_TRI_LIST        0x00000004
#define RADEON_CP_VC_CNTL_PRIM_TYPE_TRI_FAN         0x00000005
#define RADEON_CP_VC_CNTL_PRIM_TYPE_TRI_STRIP       0x00000006
#define RADEON_CP_VC_CNTL_PRIM_TYPE_TRI_TYPE_2      0x00000007
#define RADEON_CP_VC_CNTL_PRIM_TYPE_RECT_LIST       0x00000008
#define RADEON_CP_VC_CNTL_PRIM_TYPE_3VRT_POINT_LIST 0x00000009
#define RADEON_CP_VC_CNTL_PRIM_TYPE_3VRT_LINE_LIST  0x0000000a
#define RADEON_CP_VC_CNTL_PRIM_TYPE_QUAD_LIST       0x0000000d
#define RADEON_CP_VC_CNTL_PRIM_WALK_IND             0x00000010
#define RADEON_CP_VC_CNTL_PRIM_WALK_LIST            0x00000020
#define RADEON_CP_VC_CNTL_PRIM_WALK_RING            0x00000030
#define RADEON_CP_VC_CNTL_COLOR_ORDER_BGRA          0x00000000
#define RADEON_CP_VC_CNTL_COLOR_ORDER_RGBA          0x00000040
#define RADEON_CP_VC_CNTL_MAOS_ENABLE               0x00000080
#define RADEON_CP_VC_CNTL_VTX_FMT_NON_RADEON_MODE   0x00000000
#define RADEON_CP_VC_CNTL_VTX_FMT_RADEON_MODE       0x00000100
#define RADEON_CP_VC_CNTL_TCL_DISABLE               0x00000000
#define RADEON_CP_VC_CNTL_TCL_ENABLE                0x00000200
#define RADEON_CP_VC_CNTL_NUM_SHIFT                 16

#define RADEON_VS_MATRIX_0_ADDR                   0
#define RADEON_VS_MATRIX_1_ADDR                   4
#define RADEON_VS_MATRIX_2_ADDR                   8
#define RADEON_VS_MATRIX_3_ADDR                  12
#define RADEON_VS_MATRIX_4_ADDR                  16
#define RADEON_VS_MATRIX_5_ADDR                  20
#define RADEON_VS_MATRIX_6_ADDR                  24
#define RADEON_VS_MATRIX_7_ADDR                  28
#define RADEON_VS_MATRIX_8_ADDR                  32
#define RADEON_VS_MATRIX_9_ADDR                  36
#define RADEON_VS_MATRIX_10_ADDR                 40
#define RADEON_VS_MATRIX_11_ADDR                 44
#define RADEON_VS_MATRIX_12_ADDR                 48
#define RADEON_VS_MATRIX_13_ADDR                 52
#define RADEON_VS_MATRIX_14_ADDR                 56
#define RADEON_VS_MATRIX_15_ADDR                 60
#define RADEON_VS_LIGHT_AMBIENT_ADDR             64
#define RADEON_VS_LIGHT_DIFFUSE_ADDR             72
#define RADEON_VS_LIGHT_SPECULAR_ADDR            80
#define RADEON_VS_LIGHT_DIRPOS_ADDR              88
#define RADEON_VS_LIGHT_HWVSPOT_ADDR             96
#define RADEON_VS_LIGHT_ATTENUATION_ADDR        104
#define RADEON_VS_MATRIX_EYE2CLIP_ADDR          112
#define RADEON_VS_UCP_ADDR                      116
#define RADEON_VS_GLOBAL_AMBIENT_ADDR           122
#define RADEON_VS_FOG_PARAM_ADDR                123
#define RADEON_VS_EYE_VECTOR_ADDR               124

#define RADEON_SS_LIGHT_DCD_ADDR                  0
#define RADEON_SS_LIGHT_SPOT_EXPONENT_ADDR        8
#define RADEON_SS_LIGHT_SPOT_CUTOFF_ADDR         16
#define RADEON_SS_LIGHT_SPECULAR_THRESH_ADDR     24
#define RADEON_SS_LIGHT_RANGE_CUTOFF_ADDR        32
#define RADEON_SS_VERT_GUARD_CLIP_ADJ_ADDR       48
#define RADEON_SS_VERT_GUARD_DISCARD_ADJ_ADDR    49
#define RADEON_SS_HORZ_GUARD_CLIP_ADJ_ADDR       50
#define RADEON_SS_HORZ_GUARD_DISCARD_ADJ_ADDR    51
#define RADEON_SS_SHININESS                      60

#define RADEON_TV_MASTER_CNTL                    0x0800
#       define RADEON_TV_ASYNC_RST               (1 <<  0)
#       define RADEON_CRT_ASYNC_RST              (1 <<  1)
#       define RADEON_RESTART_PHASE_FIX          (1 <<  3)
#	define RADEON_TV_FIFO_ASYNC_RST		 (1 <<  4)
#	define RADEON_VIN_ASYNC_RST		 (1 <<  5)
#	define RADEON_AUD_ASYNC_RST		 (1 <<  6)
#	define RADEON_DVS_ASYNC_RST		 (1 <<  7)
#       define RADEON_CRT_FIFO_CE_EN             (1 <<  9)
#       define RADEON_TV_FIFO_CE_EN              (1 << 10)
#       define RADEON_RE_SYNC_NOW_SEL_MASK       (3 << 14)
#       define RADEON_TVCLK_ALWAYS_ONb           (1 << 30)
#	define RADEON_TV_ON			 (1 << 31)
#define RADEON_TV_PRE_DAC_MUX_CNTL               0x0888
#       define RADEON_Y_RED_EN                   (1 << 0)
#       define RADEON_C_GRN_EN                   (1 << 1)
#       define RADEON_CMP_BLU_EN                 (1 << 2)
#       define RADEON_DAC_DITHER_EN              (1 << 3)
#       define RADEON_RED_MX_FORCE_DAC_DATA      (6 << 4)
#       define RADEON_GRN_MX_FORCE_DAC_DATA      (6 << 8)
#       define RADEON_BLU_MX_FORCE_DAC_DATA      (6 << 12)
#       define RADEON_TV_FORCE_DAC_DATA_SHIFT    16
#define RADEON_TV_RGB_CNTL                           0x0804
#       define RADEON_SWITCH_TO_BLUE		  (1 <<  4)
#       define RADEON_RGB_DITHER_EN		  (1 <<  5)
#       define RADEON_RGB_SRC_SEL_MASK		  (3 <<  8)
#       define RADEON_RGB_SRC_SEL_CRTC1		  (0 <<  8)
#       define RADEON_RGB_SRC_SEL_RMX		  (1 <<  8)
#       define RADEON_RGB_SRC_SEL_CRTC2		  (2 <<  8)
#       define RADEON_RGB_CONVERT_BY_PASS	  (1 << 10)
#       define RADEON_UVRAM_READ_MARGIN_SHIFT	  16
#       define RADEON_FIFORAM_FFMACRO_READ_MARGIN_SHIFT	  20
#	define RADEON_RGB_ATTEN_SEL(x) 		  ((x) << 24)
#	define RADEON_TVOUT_SCALE_EN 		  (1 << 26)
#	define RADEON_RGB_ATTEN_VAL(x) 		  ((x) << 28)
#define RADEON_TV_SYNC_CNTL                          0x0808
#       define RADEON_SYNC_OE                     (1 <<  0)
#       define RADEON_SYNC_OUT                    (1 <<  1)
#       define RADEON_SYNC_IN                     (1 <<  2)
#       define RADEON_SYNC_PUB                    (1 <<  3)
#       define RADEON_SYNC_PD                     (1 <<  4)
#       define RADEON_TV_SYNC_IO_DRIVE            (1 <<  5)
#define RADEON_TV_HTOTAL                             0x080c
#define RADEON_TV_HDISP                              0x0810
#define RADEON_TV_HSTART                             0x0818
#define RADEON_TV_HCOUNT                             0x081C
#define RADEON_TV_VTOTAL                             0x0820
#define RADEON_TV_VDISP                              0x0824
#define RADEON_TV_VCOUNT                             0x0828
#define RADEON_TV_FTOTAL                             0x082c
#define RADEON_TV_FCOUNT                             0x0830
#define RADEON_TV_FRESTART                           0x0834
#define RADEON_TV_HRESTART                           0x0838
#define RADEON_TV_VRESTART                           0x083c
#define RADEON_TV_HOST_READ_DATA                     0x0840
#define RADEON_TV_HOST_WRITE_DATA                    0x0844
#define RADEON_TV_HOST_RD_WT_CNTL                    0x0848
#	define RADEON_HOST_FIFO_RD		 (1 << 12)
#	define RADEON_HOST_FIFO_RD_ACK		 (1 << 13)
#	define RADEON_HOST_FIFO_WT		 (1 << 14)
#	define RADEON_HOST_FIFO_WT_ACK		 (1 << 15)
#define RADEON_TV_VSCALER_CNTL1                      0x084c
#       define RADEON_UV_INC_MASK                0xffff
#       define RADEON_UV_INC_SHIFT               0
#       define RADEON_Y_W_EN			 (1 << 24)
#       define RADEON_RESTART_FIELD              (1 << 29) /* restart on field 0 */
#       define RADEON_Y_DEL_W_SIG_SHIFT          26
#define RADEON_TV_TIMING_CNTL                        0x0850
#       define RADEON_H_INC_MASK                 0xfff
#       define RADEON_H_INC_SHIFT                0
#       define RADEON_REQ_Y_FIRST                (1 << 19)
#       define RADEON_FORCE_BURST_ALWAYS         (1 << 21)
#       define RADEON_UV_POST_SCALE_BYPASS       (1 << 23)
#       define RADEON_UV_OUTPUT_POST_SCALE_SHIFT 24
#define RADEON_TV_VSCALER_CNTL2                      0x0854
#       define RADEON_DITHER_MODE                (1 <<  0)
#       define RADEON_Y_OUTPUT_DITHER_EN         (1 <<  1)
#       define RADEON_UV_OUTPUT_DITHER_EN        (1 <<  2)
#       define RADEON_UV_TO_BUF_DITHER_EN        (1 <<  3)
#define RADEON_TV_Y_FALL_CNTL                        0x0858
#       define RADEON_Y_FALL_PING_PONG           (1 << 16)
#       define RADEON_Y_COEF_EN                  (1 << 17)
#define RADEON_TV_Y_RISE_CNTL                        0x085c
#       define RADEON_Y_RISE_PING_PONG           (1 << 16)
#define RADEON_TV_Y_SAW_TOOTH_CNTL                   0x0860
#define RADEON_TV_UPSAMP_AND_GAIN_CNTL               0x0864
#	define RADEON_YUPSAMP_EN		 (1 <<  0)
#	define RADEON_UVUPSAMP_EN		 (1 <<  2)
#define RADEON_TV_GAIN_LIMIT_SETTINGS                0x0868
#       define RADEON_Y_GAIN_LIMIT_SHIFT         0
#       define RADEON_UV_GAIN_LIMIT_SHIFT        16
#define RADEON_TV_LINEAR_GAIN_SETTINGS               0x086c
#       define RADEON_Y_GAIN_SHIFT               0
#       define RADEON_UV_GAIN_SHIFT              16
#define RADEON_TV_MODULATOR_CNTL1                    0x0870
#	define RADEON_YFLT_EN			 (1 <<  2)
#	define RADEON_UVFLT_EN			 (1 <<  3)
#       define RADEON_ALT_PHASE_EN               (1 <<  6)
#       define RADEON_SYNC_TIP_LEVEL             (1 <<  7)
#       define RADEON_BLANK_LEVEL_SHIFT          8
#       define RADEON_SET_UP_LEVEL_SHIFT         16
#	define RADEON_SLEW_RATE_LIMIT		 (1 << 23)
#       define RADEON_CY_FILT_BLEND_SHIFT        28
#define RADEON_TV_MODULATOR_CNTL2                    0x0874
#       define RADEON_TV_U_BURST_LEVEL_MASK     0x1ff
#       define RADEON_TV_V_BURST_LEVEL_MASK     0x1ff
#       define RADEON_TV_V_BURST_LEVEL_SHIFT    16
#define RADEON_TV_CRC_CNTL                           0x0890
#define RADEON_TV_UV_ADR                             0x08ac
#	define RADEON_MAX_UV_ADR_MASK		 0x000000ff
#	define RADEON_MAX_UV_ADR_SHIFT		 0
#	define RADEON_TABLE1_BOT_ADR_MASK	 0x0000ff00
#	define RADEON_TABLE1_BOT_ADR_SHIFT	 8
#	define RADEON_TABLE3_TOP_ADR_MASK	 0x00ff0000
#	define RADEON_TABLE3_TOP_ADR_SHIFT	 16
#	define RADEON_HCODE_TABLE_SEL_MASK	 0x06000000
#	define RADEON_HCODE_TABLE_SEL_SHIFT	 25
#	define RADEON_VCODE_TABLE_SEL_MASK	 0x18000000
#	define RADEON_VCODE_TABLE_SEL_SHIFT	 27
#	define RADEON_TV_MAX_FIFO_ADDR		 0x1a7
#	define RADEON_TV_MAX_FIFO_ADDR_INTERNAL	 0x1ff
#define RADEON_TV_PLL_FINE_CNTL			     0x0020	/* PLL */
#define RADEON_TV_PLL_CNTL                           0x0021	/* PLL */
#       define RADEON_TV_M0LO_MASK               0xff
#       define RADEON_TV_M0HI_MASK               0x7
#       define RADEON_TV_M0HI_SHIFT              18
#       define RADEON_TV_N0LO_MASK               0x1ff
#       define RADEON_TV_N0LO_SHIFT              8
#       define RADEON_TV_N0HI_MASK               0x3
#       define RADEON_TV_N0HI_SHIFT              21
#       define RADEON_TV_P_MASK                  0xf
#       define RADEON_TV_P_SHIFT                 24
#       define RADEON_TV_SLIP_EN                 (1 << 23)
#       define RADEON_TV_DTO_EN                  (1 << 28)
#define RADEON_TV_PLL_CNTL1                          0x0022	/* PLL */
#       define RADEON_TVPLL_RESET                (1 <<  1)
#       define RADEON_TVPLL_SLEEP                (1 <<  3)
#       define RADEON_TVPLL_REFCLK_SEL           (1 <<  4)
#       define RADEON_TVPCP_SHIFT                8
#       define RADEON_TVPCP_MASK                 (7 << 8)
#       define RADEON_TVPVG_SHIFT                11
#       define RADEON_TVPVG_MASK                 (7 << 11)
#       define RADEON_TVPDC_SHIFT                14
#       define RADEON_TVPDC_MASK                 (3 << 14)
#       define RADEON_TVPLL_TEST_DIS             (1 << 31)
#       define RADEON_TVCLK_SRC_SEL_TVPLL        (1 << 30)

#define RS400_DISP2_REQ_CNTL1			0xe30
#       define RS400_DISP2_START_REQ_LEVEL_SHIFT   0
#       define RS400_DISP2_START_REQ_LEVEL_MASK    0x3ff
#       define RS400_DISP2_STOP_REQ_LEVEL_SHIFT    12
#       define RS400_DISP2_STOP_REQ_LEVEL_MASK     0x3ff
#       define RS400_DISP2_ALLOW_FID_LEVEL_SHIFT   22
#       define RS400_DISP2_ALLOW_FID_LEVEL_MASK    0x3ff
#define RS400_DISP2_REQ_CNTL2			0xe34
#       define RS400_DISP2_CRITICAL_POINT_START_SHIFT    12
#       define RS400_DISP2_CRITICAL_POINT_START_MASK     0x3ff
#       define RS400_DISP2_CRITICAL_POINT_STOP_SHIFT     22
#       define RS400_DISP2_CRITICAL_POINT_STOP_MASK      0x3ff
#define RS400_DMIF_MEM_CNTL1			0xe38
#       define RS400_DISP2_START_ADR_SHIFT      0
#       define RS400_DISP2_START_ADR_MASK       0x3ff
#       define RS400_DISP1_CRITICAL_POINT_START_SHIFT    12
#       define RS400_DISP1_CRITICAL_POINT_START_MASK     0x3ff
#       define RS400_DISP1_CRITICAL_POINT_STOP_SHIFT     22
#       define RS400_DISP1_CRITICAL_POINT_STOP_MASK      0x3ff
#define RS400_DISP1_REQ_CNTL1			0xe3c
#       define RS400_DISP1_START_REQ_LEVEL_SHIFT   0
#       define RS400_DISP1_START_REQ_LEVEL_MASK    0x3ff
#       define RS400_DISP1_STOP_REQ_LEVEL_SHIFT    12
#       define RS400_DISP1_STOP_REQ_LEVEL_MASK     0x3ff
#       define RS400_DISP1_ALLOW_FID_LEVEL_SHIFT   22
#       define RS400_DISP1_ALLOW_FID_LEVEL_MASK    0x3ff

#define RS690_MC_INDEX				0x78
#	define RS690_MC_INDEX_MASK		0x1ff
#	define RS690_MC_INDEX_WR_EN		(1 << 9)
#	define RS690_MC_INDEX_WR_ACK		0x7f
#define RS690_MC_DATA				0x7c

#define RS690_MC_FB_LOCATION			0x100
#define RS690_MC_AGP_LOCATION			0x101
#define RS690_MC_AGP_BASE			0x102
#define RS690_MC_AGP_BASE_2                     0x103
#define RS690_MC_INIT_MISC_LAT_TIMER            0x104
#define RS690_MC_STATUS                         0x90
#define RS690_MC_STATUS_IDLE                    (1 << 0)

#define RS600_MC_INDEX                          0x70
#	define RS600_MC_ADDR_MASK		0xffff
#       define RS600_MC_IND_SEQ_RBS_0           (1 << 16)
#       define RS600_MC_IND_SEQ_RBS_1           (1 << 17)
#       define RS600_MC_IND_SEQ_RBS_2           (1 << 18)
#       define RS600_MC_IND_SEQ_RBS_3           (1 << 19)
#       define RS600_MC_IND_AIC_RBS             (1 << 20)
#       define RS600_MC_IND_CITF_ARB0           (1 << 21)
#       define RS600_MC_IND_CITF_ARB1           (1 << 22)
#       define RS600_MC_IND_WR_EN               (1 << 23)
#define RS600_MC_DATA                           0x74

#define RS600_MC_STATUS			        0x0
#	define RS600_MC_IDLE		        (1 << 1)
#define RS600_MC_FB_LOCATION                    0x4
#define RS600_MC_AGP_LOCATION                   0x5
#define RS600_AGP_BASE                          0x6
#define RS600_AGP_BASE2                         0x7

#define AVIVO_MC_INDEX				0x0070
#define R520_MC_STATUS                          0x00
#       define R520_MC_STATUS_IDLE              (1 << 1)
#define RV515_MC_STATUS                         0x08
#       define RV515_MC_STATUS_IDLE             (1 << 4)
#define RV515_MC_INIT_MISC_LAT_TIMER            0x09
#define AVIVO_MC_DATA				0x0074

#define RV515_MC_FB_LOCATION   0x1
#define RV515_MC_AGP_LOCATION  0x2
#define RV515_MC_AGP_BASE      0x3
#define RV515_MC_AGP_BASE_2    0x4
#define RV515_MC_CNTL          0x5
#	define RV515_MEM_NUM_CHANNELS_MASK  0x3
#define R520_MC_FB_LOCATION    0x4
#define R520_MC_AGP_LOCATION   0x5
#define R520_MC_AGP_BASE       0x6
#define R520_MC_AGP_BASE_2     0x7
#define R520_MC_CNTL0          0x8
#	define R520_MEM_NUM_CHANNELS_MASK  (0x3 << 24)
#	define R520_MEM_NUM_CHANNELS_SHIFT  24
#	define R520_MC_CHANNEL_SIZE  (1 << 23)

#define RS780_MC_INDEX				0x28f8
#	define RS780_MC_INDEX_MASK		0x1ff
#	define RS780_MC_INDEX_WR_EN		(1 << 9)
#define RS780_MC_DATA				0x28fc

#define R600_RAMCFG				       0x2408
#       define R600_CHANSIZE                           (1 << 7)
#       define R600_CHANSIZE_OVERRIDE                  (1 << 10)

#define R600_SRBM_STATUS			       0x0e50

#define AVIVO_CP_DYN_CNTL                              0x000f /* PLL */
#       define AVIVO_CP_FORCEON                        (1 << 0)
#define AVIVO_E2_DYN_CNTL                              0x0011 /* PLL */
#       define AVIVO_E2_FORCEON                        (1 << 0)
#define AVIVO_IDCT_DYN_CNTL                            0x0013 /* PLL */
#       define AVIVO_IDCT_FORCEON                      (1 << 0)

#define AVIVO_HDP_FB_LOCATION 0x134

#define AVIVO_VGA_RENDER_CONTROL				0x0300
#       define AVIVO_VGA_VSTATUS_CNTL_MASK                      (3 << 16)
#define AVIVO_D1VGA_CONTROL					0x0330
#       define AVIVO_DVGA_CONTROL_MODE_ENABLE (1<<0)
#       define AVIVO_DVGA_CONTROL_TIMING_SELECT (1<<8)
#       define AVIVO_DVGA_CONTROL_SYNC_POLARITY_SELECT (1<<9)
#       define AVIVO_DVGA_CONTROL_OVERSCAN_TIMING_SELECT (1<<10)
#       define AVIVO_DVGA_CONTROL_OVERSCAN_COLOR_EN (1<<16)
#       define AVIVO_DVGA_CONTROL_ROTATE (1<<24)
#define AVIVO_D2VGA_CONTROL					0x0338

#define AVIVO_VGA25_PPLL_REF_DIV_SRC				0x0360
#define AVIVO_VGA25_PPLL_REF_DIV				0x0364
#define AVIVO_VGA28_PPLL_REF_DIV_SRC				0x0368
#define AVIVO_VGA28_PPLL_REF_DIV				0x036c
#define AVIVO_VGA41_PPLL_REF_DIV_SRC				0x0370
#define AVIVO_VGA41_PPLL_REF_DIV				0x0374
#define AVIVO_VGA25_PPLL_FB_DIV				0x0378
#define AVIVO_VGA28_PPLL_FB_DIV				0x037c
#define AVIVO_VGA41_PPLL_FB_DIV				0x0380
#define AVIVO_VGA25_PPLL_POST_DIV_SRC				0x0384
#define AVIVO_VGA25_PPLL_POST_DIV				0x0388
#define AVIVO_VGA28_PPLL_POST_DIV_SRC				0x038c
#define AVIVO_VGA28_PPLL_POST_DIV				0x0390
#define AVIVO_VGA41_PPLL_POST_DIV_SRC				0x0394
#define AVIVO_VGA41_PPLL_POST_DIV				0x0398
#define AVIVO_VGA25_PPLL_CNTL					0x039c
#define AVIVO_VGA28_PPLL_CNTL					0x03a0
#define AVIVO_VGA41_PPLL_CNTL					0x03a4

#define AVIVO_EXT1_PPLL_REF_DIV_SRC                             0x400
#define AVIVO_EXT1_PPLL_REF_DIV                                 0x404
#define AVIVO_EXT1_PPLL_UPDATE_LOCK                             0x408
#define AVIVO_EXT1_PPLL_UPDATE_CNTL                             0x40c

#define AVIVO_EXT2_PPLL_REF_DIV_SRC                             0x410
#define AVIVO_EXT2_PPLL_REF_DIV                                 0x414
#define AVIVO_EXT2_PPLL_UPDATE_LOCK                             0x418
#define AVIVO_EXT2_PPLL_UPDATE_CNTL                             0x41c

#define AVIVO_EXT1_PPLL_FB_DIV                                   0x430
#define AVIVO_EXT2_PPLL_FB_DIV                                   0x434

#define AVIVO_EXT1_PPLL_POST_DIV_SRC                                 0x438
#define AVIVO_EXT1_PPLL_POST_DIV                                     0x43c

#define AVIVO_EXT2_PPLL_POST_DIV_SRC                                 0x440
#define AVIVO_EXT2_PPLL_POST_DIV                                     0x444

#define AVIVO_EXT1_PPLL_CNTL                                    0x448
#define AVIVO_EXT2_PPLL_CNTL                                    0x44c

#define AVIVO_P1PLL_CNTL                                        0x450
#define AVIVO_P2PLL_CNTL                                        0x454
#define AVIVO_P1PLL_INT_SS_CNTL                                 0x458
#define AVIVO_P2PLL_INT_SS_CNTL                                 0x45c
#define AVIVO_P1PLL_TMDSA_CNTL                                  0x460
#define AVIVO_P2PLL_LVTMA_CNTL                                  0x464

#define AVIVO_PCLK_CRTC1_CNTL                                   0x480
#define AVIVO_PCLK_CRTC2_CNTL                                   0x484

#define AVIVO_D1CRTC_H_TOTAL					0x6000
#define AVIVO_D1CRTC_H_BLANK_START_END                          0x6004
#define AVIVO_D1CRTC_H_SYNC_A                                   0x6008
#define AVIVO_D1CRTC_H_SYNC_A_CNTL                              0x600c
#define AVIVO_D1CRTC_H_SYNC_B                                   0x6010
#define AVIVO_D1CRTC_H_SYNC_B_CNTL                              0x6014

#define AVIVO_D1CRTC_V_TOTAL					0x6020
#define AVIVO_D1CRTC_V_BLANK_START_END                          0x6024
#define AVIVO_D1CRTC_V_SYNC_A                                   0x6028
#define AVIVO_D1CRTC_V_SYNC_A_CNTL                              0x602c
#define AVIVO_D1CRTC_V_SYNC_B                                   0x6030
#define AVIVO_D1CRTC_V_SYNC_B_CNTL                              0x6034

#define AVIVO_D1CRTC_CONTROL                                    0x6080
#       define AVIVO_CRTC_EN                            (1<<0)
#define AVIVO_D1CRTC_BLANK_CONTROL                              0x6084
#define AVIVO_D1CRTC_INTERLACE_CONTROL                          0x6088
#define AVIVO_D1CRTC_INTERLACE_STATUS                           0x608c
#define AVIVO_D1CRTC_STEREO_CONTROL                             0x60c4

/* master controls */
#define AVIVO_DC_CRTC_MASTER_EN                                 0x60f8
#define AVIVO_DC_CRTC_TV_CONTROL                                0x60fc

#define AVIVO_D1GRPH_ENABLE                                     0x6100
#define AVIVO_D1GRPH_CONTROL                                    0x6104
#       define AVIVO_D1GRPH_CONTROL_DEPTH_8BPP          (0<<0)
#       define AVIVO_D1GRPH_CONTROL_DEPTH_16BPP         (1<<0)
#       define AVIVO_D1GRPH_CONTROL_DEPTH_32BPP         (2<<0)
#       define AVIVO_D1GRPH_CONTROL_DEPTH_64BPP         (3<<0)

#       define AVIVO_D1GRPH_CONTROL_8BPP_INDEXED        (0<<8)

#       define AVIVO_D1GRPH_CONTROL_16BPP_ARGB1555      (0<<8)
#       define AVIVO_D1GRPH_CONTROL_16BPP_RGB565        (1<<8)
#       define AVIVO_D1GRPH_CONTROL_16BPP_ARGB4444      (2<<8)
#       define AVIVO_D1GRPH_CONTROL_16BPP_AI88          (3<<8)
#       define AVIVO_D1GRPH_CONTROL_16BPP_MONO16        (4<<8)

#       define AVIVO_D1GRPH_CONTROL_32BPP_ARGB8888      (0<<8)
#       define AVIVO_D1GRPH_CONTROL_32BPP_ARGB2101010   (1<<8)
#       define AVIVO_D1GRPH_CONTROL_32BPP_DIGITAL       (2<<8)
#       define AVIVO_D1GRPH_CONTROL_32BPP_8B_ARGB2101010 (3<<8)


#       define AVIVO_D1GRPH_CONTROL_64BPP_ARGB16161616  (0<<8)

#       define AVIVO_D1GRPH_SWAP_RB                     (1<<16)
#       define AVIVO_D1GRPH_TILED                       (1<<20)
#       define AVIVO_D1GRPH_MACRO_ADDRESS_MODE          (1<<21)

#define AVIVO_D1GRPH_LUT_SEL                                    0x6108

#define R600_D1GRPH_SWAP_CONTROL                               0x610C
#       define R600_D1GRPH_SWAP_ENDIAN_NONE                    (0 << 0)
#       define R600_D1GRPH_SWAP_ENDIAN_16BIT                   (1 << 0)
#       define R600_D1GRPH_SWAP_ENDIAN_32BIT                   (2 << 0)
#       define R600_D1GRPH_SWAP_ENDIAN_64BIT                   (3 << 0)

/* the *_HIGH surface regs are backwards; the D1 regs are in the D2
 * block and vice versa.  This applies to GRPH, CUR, etc.
 */

#define AVIVO_D1GRPH_PRIMARY_SURFACE_ADDRESS                    0x6110
#define R700_D1GRPH_PRIMARY_SURFACE_ADDRESS_HIGH                0x6914
#define R700_D2GRPH_PRIMARY_SURFACE_ADDRESS_HIGH                0x6114
#define AVIVO_D1GRPH_SECONDARY_SURFACE_ADDRESS                  0x6118
#define R700_D1GRPH_SECONDARY_SURFACE_ADDRESS_HIGH              0x691c
#define R700_D2GRPH_SECONDARY_SURFACE_ADDRESS_HIGH              0x611c
#define AVIVO_D1GRPH_PITCH                                      0x6120
#define AVIVO_D1GRPH_SURFACE_OFFSET_X                           0x6124
#define AVIVO_D1GRPH_SURFACE_OFFSET_Y                           0x6128
#define AVIVO_D1GRPH_X_START                                    0x612c
#define AVIVO_D1GRPH_Y_START                                    0x6130
#define AVIVO_D1GRPH_X_END                                      0x6134
#define AVIVO_D1GRPH_Y_END                                      0x6138
#define AVIVO_D1GRPH_UPDATE                                     0x6144
#       define AVIVO_D1GRPH_UPDATE_LOCK                 (1<<16)
#define AVIVO_D1GRPH_FLIP_CONTROL                               0x6148

#define AVIVO_D1GRPH_COLOR_MATRIX_TRANSFORMATION_CNTL           0x6380

#define AVIVO_D1CUR_CONTROL                     0x6400
#       define AVIVO_D1CURSOR_EN           (1<<0)
#       define AVIVO_D1CURSOR_MODE_SHIFT  8
#       define AVIVO_D1CURSOR_MODE_MASK   (0x3<<8)
#       define AVIVO_D1CURSOR_MODE_24BPP  (0x2)
#define AVIVO_D1CUR_SURFACE_ADDRESS             0x6408
#define R700_D1CUR_SURFACE_ADDRESS_HIGH         0x6c0c
#define R700_D2CUR_SURFACE_ADDRESS_HIGH         0x640c
#define AVIVO_D1CUR_SIZE                        0x6410
#define AVIVO_D1CUR_POSITION                    0x6414
#define AVIVO_D1CUR_HOT_SPOT                    0x6418
#define AVIVO_D1CUR_UPDATE                      0x6424
#       define AVIVO_D1CURSOR_UPDATE_LOCK (1 << 16)

#define AVIVO_DC_LUT_RW_SELECT                  0x6480
#define AVIVO_DC_LUT_RW_MODE                    0x6484
#define AVIVO_DC_LUT_RW_INDEX                   0x6488
#define AVIVO_DC_LUT_SEQ_COLOR                  0x648c
#define AVIVO_DC_LUT_PWL_DATA                   0x6490
#define AVIVO_DC_LUT_30_COLOR                   0x6494
#define AVIVO_DC_LUT_READ_PIPE_SELECT           0x6498
#define AVIVO_DC_LUT_WRITE_EN_MASK              0x649c
#define AVIVO_DC_LUT_AUTOFILL                   0x64a0

#define AVIVO_DC_LUTA_CONTROL                   0x64c0
#define AVIVO_DC_LUTA_BLACK_OFFSET_BLUE         0x64c4
#define AVIVO_DC_LUTA_BLACK_OFFSET_GREEN        0x64c8
#define AVIVO_DC_LUTA_BLACK_OFFSET_RED          0x64cc
#define AVIVO_DC_LUTA_WHITE_OFFSET_BLUE         0x64d0
#define AVIVO_DC_LUTA_WHITE_OFFSET_GREEN        0x64d4
#define AVIVO_DC_LUTA_WHITE_OFFSET_RED          0x64d8

#define AVIVO_DC_LB_MEMORY_SPLIT                0x6520
#       define AVIVO_DC_LB_MEMORY_SPLIT_MASK    0x3
#       define AVIVO_DC_LB_MEMORY_SPLIT_SHIFT   0
#       define AVIVO_DC_LB_MEMORY_SPLIT_D1HALF_D2HALF  0
#       define AVIVO_DC_LB_MEMORY_SPLIT_D1_3Q_D2_1Q    1
#       define AVIVO_DC_LB_MEMORY_SPLIT_D1_ONLY        2
#       define AVIVO_DC_LB_MEMORY_SPLIT_D1_1Q_D2_3Q    3
#       define AVIVO_DC_LB_MEMORY_SPLIT_SHIFT_MODE (1 << 2)
#       define AVIVO_DC_LB_DISP1_END_ADR_SHIFT  4
#       define AVIVO_DC_LB_DISP1_END_ADR_MASK   0x7ff
#define AVIVO_D1MODE_PRIORITY_A_CNT             0x6548
#       define AVIVO_DxMODE_PRIORITY_MARK_MASK  0x7fff
#       define AVIVO_DxMODE_PRIORITY_OFF        (1 << 16)
#       define AVIVO_DxMODE_PRIORITY_ALWAYS_ON  (1 << 20)
#       define AVIVO_DxMODE_PRIORITY_FORCE_MASK (1 << 24)
#define AVIVO_D1MODE_PRIORITY_B_CNT             0x654c
#define AVIVO_D2MODE_PRIORITY_A_CNT             0x6d48
#define AVIVO_D2MODE_PRIORITY_B_CNT             0x6d4c
#define AVIVO_LB_MAX_REQ_OUTSTANDING            0x6d58
#       define AVIVO_LB_D1_MAX_REQ_OUTSTANDING_MASK    0xf
#       define AVIVO_LB_D1_MAX_REQ_OUTSTANDING_SHIFT   0
#       define AVIVO_LB_D2_MAX_REQ_OUTSTANDING_MASK    0xf
#       define AVIVO_LB_D2_MAX_REQ_OUTSTANDING_SHIFT   16

#define AVIVO_D1MODE_DATA_FORMAT                0x6528
#       define AVIVO_D1MODE_INTERLEAVE_EN       (1 << 0)
#define AVIVO_D1MODE_DESKTOP_HEIGHT             0x652c
#define AVIVO_D1MODE_VLINE_START_END            0x6538
#       define AVIVO_D1MODE_VLINE_START_SHIFT   0
#       define AVIVO_D1MODE_VLINE_END_SHIFT     16
#       define AVIVO_D1MODE_VLINE_INV           (1 << 31)
#define AVIVO_D1MODE_VLINE_STATUS               0x653c
#       define AVIVO_D1MODE_VLINE_STAT          (1 << 12)
#define AVIVO_D1MODE_VIEWPORT_START             0x6580
#define AVIVO_D1MODE_VIEWPORT_SIZE              0x6584
#define AVIVO_D1MODE_EXT_OVERSCAN_LEFT_RIGHT    0x6588
#define AVIVO_D1MODE_EXT_OVERSCAN_TOP_BOTTOM    0x658c

#define AVIVO_D1SCL_SCALER_ENABLE               0x6590
#define AVIVO_D1SCL_SCALER_TAP_CONTROL	 	0x6594
#define AVIVO_D1SCL_UPDATE                      0x65cc
#       define AVIVO_D1SCL_UPDATE_LOCK         (1<<16)

/* second crtc */
#define AVIVO_D2CRTC_H_TOTAL					0x6800
#define AVIVO_D2CRTC_H_BLANK_START_END                          0x6804
#define AVIVO_D2CRTC_H_SYNC_A                                   0x6808
#define AVIVO_D2CRTC_H_SYNC_A_CNTL                              0x680c
#define AVIVO_D2CRTC_H_SYNC_B                                   0x6810
#define AVIVO_D2CRTC_H_SYNC_B_CNTL                              0x6814

#define AVIVO_D2CRTC_V_TOTAL					0x6820
#define AVIVO_D2CRTC_V_BLANK_START_END                          0x6824
#define AVIVO_D2CRTC_V_SYNC_A                                   0x6828
#define AVIVO_D2CRTC_V_SYNC_A_CNTL                              0x682c
#define AVIVO_D2CRTC_V_SYNC_B                                   0x6830
#define AVIVO_D2CRTC_V_SYNC_B_CNTL                              0x6834

#define AVIVO_D2CRTC_CONTROL                                    0x6880
#define AVIVO_D2CRTC_BLANK_CONTROL                              0x6884
#define AVIVO_D2CRTC_INTERLACE_CONTROL                          0x6888
#define AVIVO_D2CRTC_INTERLACE_STATUS                           0x688c
#define AVIVO_D2CRTC_STEREO_CONTROL                             0x68c4

#define AVIVO_D2GRPH_ENABLE                                     0x6900
#define AVIVO_D2GRPH_CONTROL                                    0x6904
#define AVIVO_D2GRPH_LUT_SEL                                    0x6908
#define AVIVO_D2GRPH_PRIMARY_SURFACE_ADDRESS                    0x6910
#define AVIVO_D2GRPH_SECONDARY_SURFACE_ADDRESS                  0x6918
#define AVIVO_D2GRPH_PITCH                                      0x6920
#define AVIVO_D2GRPH_SURFACE_OFFSET_X                           0x6924
#define AVIVO_D2GRPH_SURFACE_OFFSET_Y                           0x6928
#define AVIVO_D2GRPH_X_START                                    0x692c
#define AVIVO_D2GRPH_Y_START                                    0x6930
#define AVIVO_D2GRPH_X_END                                      0x6934
#define AVIVO_D2GRPH_Y_END                                      0x6938
#define AVIVO_D2GRPH_UPDATE                                     0x6944
#define AVIVO_D2GRPH_FLIP_CONTROL                               0x6948

#define AVIVO_D2CUR_CONTROL                     0x6c00
#define AVIVO_D2CUR_SURFACE_ADDRESS             0x6c08
#define AVIVO_D2CUR_SIZE                        0x6c10
#define AVIVO_D2CUR_POSITION                    0x6c14

#define RS690_DCP_CONTROL                       0x6c9c

#define AVIVO_D2MODE_DATA_FORMAT                0x6d28
#define AVIVO_D2MODE_DESKTOP_HEIGHT             0x6d2c
#define AVIVO_D2MODE_VIEWPORT_START             0x6d80
#define AVIVO_D2MODE_VIEWPORT_SIZE              0x6d84
#define AVIVO_D2MODE_EXT_OVERSCAN_LEFT_RIGHT    0x6d88
#define AVIVO_D2MODE_EXT_OVERSCAN_TOP_BOTTOM    0x6d8c

#define AVIVO_D2SCL_SCALER_ENABLE               0x6d90
#define AVIVO_D2SCL_SCALER_TAP_CONTROL	 	0x6d94
#define AVIVO_D2SCL_UPDATE                      0x6dcc

#define AVIVO_DDIA_BIT_DEPTH_CONTROL				0x7214

#define AVIVO_DACA_ENABLE					0x7800
#	define AVIVO_DAC_ENABLE				(1 << 0)
#define AVIVO_DACA_SOURCE_SELECT				0x7804
#       define AVIVO_DAC_SOURCE_CRTC1                   (0 << 0)
#       define AVIVO_DAC_SOURCE_CRTC2                   (1 << 0)
#       define AVIVO_DAC_SOURCE_TV                      (2 << 0)

#define AVIVO_DACA_FORCE_OUTPUT_CNTL				0x783c
# define AVIVO_DACA_FORCE_OUTPUT_CNTL_FORCE_DATA_EN             (1 << 0)
# define AVIVO_DACA_FORCE_OUTPUT_CNTL_DATA_SEL_SHIFT            (8)
# define AVIVO_DACA_FORCE_OUTPUT_CNTL_DATA_SEL_BLUE             (1 << 0)
# define AVIVO_DACA_FORCE_OUTPUT_CNTL_DATA_SEL_GREEN            (1 << 1)
# define AVIVO_DACA_FORCE_OUTPUT_CNTL_DATA_SEL_RED              (1 << 2)
# define AVIVO_DACA_FORCE_OUTPUT_CNTL_DATA_ON_BLANKB_ONLY       (1 << 24)
#define AVIVO_DACA_POWERDOWN					0x7850
# define AVIVO_DACA_POWERDOWN_POWERDOWN                         (1 << 0)
# define AVIVO_DACA_POWERDOWN_BLUE                              (1 << 8)
# define AVIVO_DACA_POWERDOWN_GREEN                             (1 << 16)
# define AVIVO_DACA_POWERDOWN_RED                               (1 << 24)

#define AVIVO_DACB_ENABLE					0x7a00
#define AVIVO_DACB_SOURCE_SELECT				0x7a04
#define AVIVO_DACB_FORCE_OUTPUT_CNTL				0x7a3c
# define AVIVO_DACB_FORCE_OUTPUT_CNTL_FORCE_DATA_EN             (1 << 0)
# define AVIVO_DACB_FORCE_OUTPUT_CNTL_DATA_SEL_SHIFT            (8)
# define AVIVO_DACB_FORCE_OUTPUT_CNTL_DATA_SEL_BLUE             (1 << 0)
# define AVIVO_DACB_FORCE_OUTPUT_CNTL_DATA_SEL_GREEN            (1 << 1)
# define AVIVO_DACB_FORCE_OUTPUT_CNTL_DATA_SEL_RED              (1 << 2)
# define AVIVO_DACB_FORCE_OUTPUT_CNTL_DATA_ON_BLANKB_ONLY       (1 << 24)
#define AVIVO_DACB_POWERDOWN					0x7a50
# define AVIVO_DACB_POWERDOWN_POWERDOWN                         (1 << 0)
# define AVIVO_DACB_POWERDOWN_BLUE                              (1 << 8)
# define AVIVO_DACB_POWERDOWN_GREEN                             (1 << 16)
# define AVIVO_DACB_POWERDOWN_RED 

#define AVIVO_TMDSA_CNTL                    0x7880
#   define AVIVO_TMDSA_CNTL_ENABLE               (1 << 0)
#   define AVIVO_TMDSA_CNTL_HPD_MASK             (1 << 4)
#   define AVIVO_TMDSA_CNTL_HPD_SELECT           (1 << 8)
#   define AVIVO_TMDSA_CNTL_SYNC_PHASE           (1 << 12)
#   define AVIVO_TMDSA_CNTL_PIXEL_ENCODING       (1 << 16)
#   define AVIVO_TMDSA_CNTL_DUAL_LINK_ENABLE     (1 << 24)
#   define AVIVO_TMDSA_CNTL_SWAP                 (1 << 28)
#define AVIVO_TMDSA_SOURCE_SELECT				0x7884
/* 78a8 appears to be some kind of (reasonably tolerant) clock?
 * 78d0 definitely hits the transmitter, definitely clock. */
/* MYSTERY1 This appears to control dithering? */
#define AVIVO_TMDSA_BIT_DEPTH_CONTROL		0x7894
#   define AVIVO_TMDS_BIT_DEPTH_CONTROL_TRUNCATE_EN           (1 << 0)
#   define AVIVO_TMDS_BIT_DEPTH_CONTROL_TRUNCATE_DEPTH        (1 << 4)
#   define AVIVO_TMDS_BIT_DEPTH_CONTROL_SPATIAL_DITHER_EN     (1 << 8)
#   define AVIVO_TMDS_BIT_DEPTH_CONTROL_SPATIAL_DITHER_DEPTH  (1 << 12)
#   define AVIVO_TMDS_BIT_DEPTH_CONTROL_TEMPORAL_DITHER_EN    (1 << 16)
#   define AVIVO_TMDS_BIT_DEPTH_CONTROL_TEMPORAL_DITHER_DEPTH (1 << 20)
#   define AVIVO_TMDS_BIT_DEPTH_CONTROL_TEMPORAL_LEVEL        (1 << 24)
#   define AVIVO_TMDS_BIT_DEPTH_CONTROL_TEMPORAL_DITHER_RESET (1 << 26)
#define AVIVO_TMDSA_DCBALANCER_CONTROL                  0x78d0
#   define AVIVO_TMDSA_DCBALANCER_CONTROL_EN                  (1 << 0)
#   define AVIVO_TMDSA_DCBALANCER_CONTROL_TEST_EN             (1 << 8)
#   define AVIVO_TMDSA_DCBALANCER_CONTROL_TEST_IN_SHIFT       (16)
#   define AVIVO_TMDSA_DCBALANCER_CONTROL_FORCE               (1 << 24)
#define AVIVO_TMDSA_DATA_SYNCHRONIZATION                0x78d8
#   define AVIVO_TMDSA_DATA_SYNCHRONIZATION_DSYNSEL           (1 << 0)
#   define AVIVO_TMDSA_DATA_SYNCHRONIZATION_PFREQCHG          (1 << 8)
#define AVIVO_TMDSA_CLOCK_ENABLE            0x7900
#define AVIVO_TMDSA_TRANSMITTER_ENABLE              0x7904
#   define AVIVO_TMDSA_TRANSMITTER_ENABLE_TX0_ENABLE          (1 << 0)
#   define AVIVO_TMDSA_TRANSMITTER_ENABLE_LNKC0EN             (1 << 1)
#   define AVIVO_TMDSA_TRANSMITTER_ENABLE_LNKD00EN            (1 << 2)
#   define AVIVO_TMDSA_TRANSMITTER_ENABLE_LNKD01EN            (1 << 3)
#   define AVIVO_TMDSA_TRANSMITTER_ENABLE_LNKD02EN            (1 << 4)
#   define AVIVO_TMDSA_TRANSMITTER_ENABLE_TX1_ENABLE          (1 << 8)
#   define AVIVO_TMDSA_TRANSMITTER_ENABLE_LNKD10EN            (1 << 10)
#   define AVIVO_TMDSA_TRANSMITTER_ENABLE_LNKD11EN            (1 << 11)
#   define AVIVO_TMDSA_TRANSMITTER_ENABLE_LNKD12EN            (1 << 12)
#   define AVIVO_TMDSA_TRANSMITTER_ENABLE_TX_ENABLE_HPD_MASK  (1 << 16)
#   define AVIVO_TMDSA_TRANSMITTER_ENABLE_LNKCEN_HPD_MASK     (1 << 17)
#   define AVIVO_TMDSA_TRANSMITTER_ENABLE_LNKDEN_HPD_MASK     (1 << 18)

#define AVIVO_TMDSA_TRANSMITTER_CONTROL				0x7910
#	define AVIVO_TMDSA_TRANSMITTER_CONTROL_PLL_ENABLE	(1 << 0)
#	define AVIVO_TMDSA_TRANSMITTER_CONTROL_PLL_RESET  	(1 << 1)
#	define AVIVO_TMDSA_TRANSMITTER_CONTROL_PLL_HPD_MASK_SHIFT	(2)
#	define AVIVO_TMDSA_TRANSMITTER_CONTROL_IDSCKSEL	        (1 << 4)
#       define AVIVO_TMDSA_TRANSMITTER_CONTROL_BGSLEEP          (1 << 5)
#	define AVIVO_TMDSA_TRANSMITTER_CONTROL_PLL_PWRUP_SEQ_EN	(1 << 6)
#	define AVIVO_TMDSA_TRANSMITTER_CONTROL_TMCLK	        (1 << 8)
#	define AVIVO_TMDSA_TRANSMITTER_CONTROL_TMCLK_FROM_PADS	(1 << 13)
#	define AVIVO_TMDSA_TRANSMITTER_CONTROL_TDCLK	        (1 << 14)
#	define AVIVO_TMDSA_TRANSMITTER_CONTROL_TDCLK_FROM_PADS	(1 << 15)
#       define AVIVO_TMDSA_TRANSMITTER_CONTROL_CLK_PATTERN_SHIFT (16)
#	define AVIVO_TMDSA_TRANSMITTER_CONTROL_BYPASS_PLL	(1 << 28)
#       define AVIVO_TMDSA_TRANSMITTER_CONTROL_USE_CLK_DATA     (1 << 29)
#	define AVIVO_TMDSA_TRANSMITTER_CONTROL_INPUT_TEST_CLK_SEL	(1 << 31)

#define AVIVO_LVTMA_CNTL					0x7a80
#   define AVIVO_LVTMA_CNTL_ENABLE               (1 << 0)
#   define AVIVO_LVTMA_CNTL_HPD_MASK             (1 << 4)
#   define AVIVO_LVTMA_CNTL_HPD_SELECT           (1 << 8)
#   define AVIVO_LVTMA_CNTL_SYNC_PHASE           (1 << 12)
#   define AVIVO_LVTMA_CNTL_PIXEL_ENCODING       (1 << 16)
#   define AVIVO_LVTMA_CNTL_DUAL_LINK_ENABLE     (1 << 24)
#   define AVIVO_LVTMA_CNTL_SWAP                 (1 << 28)
#define AVIVO_LVTMA_SOURCE_SELECT                               0x7a84
#define AVIVO_LVTMA_COLOR_FORMAT                                0x7a88
#define AVIVO_LVTMA_BIT_DEPTH_CONTROL                           0x7a94
#   define AVIVO_LVTMA_BIT_DEPTH_CONTROL_TRUNCATE_EN           (1 << 0)
#   define AVIVO_LVTMA_BIT_DEPTH_CONTROL_TRUNCATE_DEPTH        (1 << 4)
#   define AVIVO_LVTMA_BIT_DEPTH_CONTROL_SPATIAL_DITHER_EN     (1 << 8)
#   define AVIVO_LVTMA_BIT_DEPTH_CONTROL_SPATIAL_DITHER_DEPTH  (1 << 12)
#   define AVIVO_LVTMA_BIT_DEPTH_CONTROL_TEMPORAL_DITHER_EN    (1 << 16)
#   define AVIVO_LVTMA_BIT_DEPTH_CONTROL_TEMPORAL_DITHER_DEPTH (1 << 20)
#   define AVIVO_LVTMA_BIT_DEPTH_CONTROL_TEMPORAL_LEVEL        (1 << 24)
#   define AVIVO_LVTMA_BIT_DEPTH_CONTROL_TEMPORAL_DITHER_RESET (1 << 26)



#define AVIVO_LVTMA_DCBALANCER_CONTROL                  0x7ad0
#   define AVIVO_LVTMA_DCBALANCER_CONTROL_EN                  (1 << 0)
#   define AVIVO_LVTMA_DCBALANCER_CONTROL_TEST_EN             (1 << 8)
#   define AVIVO_LVTMA_DCBALANCER_CONTROL_TEST_IN_SHIFT       (16)
#   define AVIVO_LVTMA_DCBALANCER_CONTROL_FORCE               (1 << 24)

#define AVIVO_LVTMA_DATA_SYNCHRONIZATION                0x78d8
#   define AVIVO_LVTMA_DATA_SYNCHRONIZATION_DSYNSEL           (1 << 0)
#   define AVIVO_LVTMA_DATA_SYNCHRONIZATION_PFREQCHG          (1 << 8)
#define R500_LVTMA_CLOCK_ENABLE			0x7b00
#define R600_LVTMA_CLOCK_ENABLE			0x7b04

#define R500_LVTMA_TRANSMITTER_ENABLE              0x7b04
#define R600_LVTMA_TRANSMITTER_ENABLE              0x7b08
#   define AVIVO_LVTMA_TRANSMITTER_ENABLE_LNKC0EN             (1 << 1)
#   define AVIVO_LVTMA_TRANSMITTER_ENABLE_LNKD00EN            (1 << 2)
#   define AVIVO_LVTMA_TRANSMITTER_ENABLE_LNKD01EN            (1 << 3)
#   define AVIVO_LVTMA_TRANSMITTER_ENABLE_LNKD02EN            (1 << 4)
#   define AVIVO_LVTMA_TRANSMITTER_ENABLE_LNKD03EN            (1 << 5)
#   define AVIVO_LVTMA_TRANSMITTER_ENABLE_LNKC1EN             (1 << 9)
#   define AVIVO_LVTMA_TRANSMITTER_ENABLE_LNKD10EN            (1 << 10)
#   define AVIVO_LVTMA_TRANSMITTER_ENABLE_LNKD11EN            (1 << 11)
#   define AVIVO_LVTMA_TRANSMITTER_ENABLE_LNKD12EN            (1 << 12)
#   define AVIVO_LVTMA_TRANSMITTER_ENABLE_LNKCEN_HPD_MASK     (1 << 17)
#   define AVIVO_LVTMA_TRANSMITTER_ENABLE_LNKDEN_HPD_MASK     (1 << 18)

#define R500_LVTMA_TRANSMITTER_CONTROL			        0x7b10
#define R600_LVTMA_TRANSMITTER_CONTROL			        0x7b14
#	define AVIVO_LVTMA_TRANSMITTER_CONTROL_PLL_ENABLE	  (1 << 0)
#	define AVIVO_LVTMA_TRANSMITTER_CONTROL_PLL_RESET  	  (1 << 1)
#	define AVIVO_LVTMA_TRANSMITTER_CONTROL_PLL_HPD_MASK_SHIFT (2)
#	define AVIVO_LVTMA_TRANSMITTER_CONTROL_IDSCKSEL	          (1 << 4)
#       define AVIVO_LVTMA_TRANSMITTER_CONTROL_BGSLEEP            (1 << 5)
#	define AVIVO_LVTMA_TRANSMITTER_CONTROL_PLL_PWRUP_SEQ_EN	  (1 << 6)
#	define AVIVO_LVTMA_TRANSMITTER_CONTROL_TMCLK	          (1 << 8)
#	define AVIVO_LVTMA_TRANSMITTER_CONTROL_TMCLK_FROM_PADS	  (1 << 13)
#	define AVIVO_LVTMA_TRANSMITTER_CONTROL_TDCLK	          (1 << 14)
#	define AVIVO_LVTMA_TRANSMITTER_CONTROL_TDCLK_FROM_PADS	  (1 << 15)
#       define AVIVO_LVTMA_TRANSMITTER_CONTROL_CLK_PATTERN_SHIFT  (16)
#	define AVIVO_LVTMA_TRANSMITTER_CONTROL_BYPASS_PLL	  (1 << 28)
#       define AVIVO_LVTMA_TRANSMITTER_CONTROL_USE_CLK_DATA       (1 << 29)
#	define AVIVO_LVTMA_TRANSMITTER_CONTROL_INPUT_TEST_CLK_SEL (1 << 31)

#define R500_LVTMA_PWRSEQ_CNTL						0x7af0
#define R600_LVTMA_PWRSEQ_CNTL						0x7af4
#	define AVIVO_LVTMA_PWRSEQ_EN					    (1 << 0)
#	define AVIVO_LVTMA_PWRSEQ_PLL_ENABLE_MASK			    (1 << 2)
#	define AVIVO_LVTMA_PWRSEQ_PLL_RESET_MASK			    (1 << 3)
#	define AVIVO_LVTMA_PWRSEQ_TARGET_STATE				    (1 << 4)
#	define AVIVO_LVTMA_SYNCEN					    (1 << 8)
#	define AVIVO_LVTMA_SYNCEN_OVRD					    (1 << 9)
#	define AVIVO_LVTMA_SYNCEN_POL					    (1 << 10)
#	define AVIVO_LVTMA_DIGON					    (1 << 16)
#	define AVIVO_LVTMA_DIGON_OVRD					    (1 << 17)
#	define AVIVO_LVTMA_DIGON_POL					    (1 << 18)
#	define AVIVO_LVTMA_BLON						    (1 << 24)
#	define AVIVO_LVTMA_BLON_OVRD					    (1 << 25)
#	define AVIVO_LVTMA_BLON_POL					    (1 << 26)

#define R500_LVTMA_PWRSEQ_STATE                        0x7af4
#define R600_LVTMA_PWRSEQ_STATE                        0x7af8
#       define AVIVO_LVTMA_PWRSEQ_STATE_TARGET_STATE_R          (1 << 0)
#       define AVIVO_LVTMA_PWRSEQ_STATE_DIGON                   (1 << 1)
#       define AVIVO_LVTMA_PWRSEQ_STATE_SYNCEN                  (1 << 2)
#       define AVIVO_LVTMA_PWRSEQ_STATE_BLON                    (1 << 3)
#       define AVIVO_LVTMA_PWRSEQ_STATE_DONE                    (1 << 4)
#       define AVIVO_LVTMA_PWRSEQ_STATE_STATUS_SHIFT            (8)

#define AVIVO_LVDS_BACKLIGHT_CNTL			0x7af8
#	define AVIVO_LVDS_BACKLIGHT_CNTL_EN			(1 << 0)
#	define AVIVO_LVDS_BACKLIGHT_LEVEL_MASK		0x0000ff00
#	define AVIVO_LVDS_BACKLIGHT_LEVEL_SHIFT		8

#define AVIVO_DVOA_BIT_DEPTH_CONTROL			0x7988

#define AVIVO_GPIO_0                        0x7e30
#define AVIVO_GPIO_1                        0x7e40
#define AVIVO_GPIO_2                        0x7e50
#define AVIVO_GPIO_3                        0x7e60

#define AVIVO_DC_GPIO_HPD_MASK              0x7e90
#define AVIVO_DC_GPIO_HPD_A                 0x7e94
#define AVIVO_DC_GPIO_HPD_EN                0x7e98
#define AVIVO_DC_GPIO_HPD_Y                 0x7e9c

#define AVIVO_I2C_STATUS					0x7d30
#	define AVIVO_I2C_STATUS_DONE				(1 << 0)
#	define AVIVO_I2C_STATUS_NACK				(1 << 1)
#	define AVIVO_I2C_STATUS_HALT				(1 << 2)
#	define AVIVO_I2C_STATUS_GO				(1 << 3)
#	define AVIVO_I2C_STATUS_MASK				0x7
/* If radeon_mm_i2c is to be believed, this is HALT, NACK, and maybe
 * DONE? */
#	define AVIVO_I2C_STATUS_CMD_RESET			0x7
#	define AVIVO_I2C_STATUS_CMD_WAIT			(1 << 3)
#define AVIVO_I2C_STOP						0x7d34
#define AVIVO_I2C_START_CNTL				0x7d38
#	define AVIVO_I2C_START						(1 << 8)
#	define AVIVO_I2C_CONNECTOR0					(0 << 16)
#	define AVIVO_I2C_CONNECTOR1					(1 << 16)
#define R520_I2C_START (1<<0)
#define R520_I2C_STOP (1<<1)
#define R520_I2C_RX (1<<2)
#define R520_I2C_EN (1<<8)
#define R520_I2C_DDC1 (0<<16)
#define R520_I2C_DDC2 (1<<16)
#define R520_I2C_DDC3 (2<<16)
#define R520_I2C_DDC_MASK (3<<16)
#define AVIVO_I2C_CONTROL2					0x7d3c
#	define AVIVO_I2C_7D3C_SIZE_SHIFT			8
#	define AVIVO_I2C_7D3C_SIZE_MASK				(0xf << 8)
#define AVIVO_I2C_CONTROL3						0x7d40
/* Reading is done 4 bytes at a time: read the bottom 8 bits from
 * 7d44, four times in a row.
 * Writing is a little more complex.  First write DATA with
 * 0xnnnnnnzz, then 0xnnnnnnyy, where nnnnnn is some non-deterministic
 * magic number, zz is, I think, the slave address, and yy is the byte
 * you want to write. */
#define AVIVO_I2C_DATA						0x7d44
#define R520_I2C_ADDR_COUNT_MASK (0x7)
#define R520_I2C_DATA_COUNT_SHIFT (8)
#define R520_I2C_DATA_COUNT_MASK (0xF00)
#define AVIVO_I2C_CNTL						0x7d50
#	define AVIVO_I2C_EN							(1 << 0)
#	define AVIVO_I2C_RESET						(1 << 8)

#define R600_GENERAL_PWRMGT                                        0x618
#	define R600_OPEN_DRAIN_PADS				   (1 << 11)

#define R600_LOWER_GPIO_ENABLE                                     0x710
#define R600_CTXSW_VID_LOWER_GPIO_CNTL                             0x718
#define R600_HIGH_VID_LOWER_GPIO_CNTL                              0x71c
#define R600_MEDIUM_VID_LOWER_GPIO_CNTL                            0x720
#define R600_LOW_VID_LOWER_GPIO_CNTL                               0x724

#define R600_MC_VM_FB_LOCATION                                     0x2180
#define R600_MC_VM_AGP_TOP                                         0x2184
#define R600_MC_VM_AGP_BOT                                         0x2188
#define R600_MC_VM_AGP_BASE                                        0x218c
#define R600_MC_VM_SYSTEM_APERTURE_LOW_ADDR                        0x2190
#define R600_MC_VM_SYSTEM_APERTURE_HIGH_ADDR                       0x2194
#define R600_MC_VM_SYSTEM_APERTURE_DEFAULT_ADDR                    0x2198

#define R700_MC_VM_FB_LOCATION                                     0x2024
#define R700_MC_VM_AGP_TOP                                         0x2028
#define R700_MC_VM_AGP_BOT                                         0x202c
#define R700_MC_VM_AGP_BASE                                        0x2030

#define R600_HDP_NONSURFACE_BASE                                0x2c04

#define R600_BUS_CNTL                                           0x5420
#define R600_CONFIG_CNTL                                        0x5424
#define R600_CONFIG_MEMSIZE                                     0x5428
#define R600_CONFIG_F0_BASE                                     0x542C
#define R600_CONFIG_APER_SIZE                                   0x5430

#define R600_ROM_CNTL                              0x1600
#       define R600_SCK_OVERWRITE                  (1 << 1)
#       define R600_SCK_PRESCALE_CRYSTAL_CLK_SHIFT 28
#       define R600_SCK_PRESCALE_CRYSTAL_CLK_MASK  (0xf << 28)

#define R600_CG_SPLL_FUNC_CNTL                     0x600
#       define R600_SPLL_BYPASS_EN                 (1 << 3)
#define R600_CG_SPLL_STATUS                        0x60c
#       define R600_SPLL_CHG_STATUS                (1 << 1)

#define R600_BIOS_0_SCRATCH               0x1724
#define R600_BIOS_1_SCRATCH               0x1728
#define R600_BIOS_2_SCRATCH               0x172c
#define R600_BIOS_3_SCRATCH               0x1730
#define R600_BIOS_4_SCRATCH               0x1734
#define R600_BIOS_5_SCRATCH               0x1738
#define R600_BIOS_6_SCRATCH               0x173c
#define R600_BIOS_7_SCRATCH               0x1740

/* evergreen */
#define EVERGREEN_VGA_MEMORY_BASE_ADDRESS               0x310
#define EVERGREEN_VGA_MEMORY_BASE_ADDRESS_HIGH          0x324
#define EVERGREEN_D3VGA_CONTROL                         0x3e0
#define EVERGREEN_D4VGA_CONTROL                         0x3e4
#define EVERGREEN_D5VGA_CONTROL                         0x3e8
#define EVERGREEN_D6VGA_CONTROL                         0x3ec

#define EVERGREEN_P1PLL_SS_CNTL                         0x414
#define EVERGREEN_P2PLL_SS_CNTL                         0x454
#       define EVERGREEN_PxPLL_SS_EN                    (1 << 12)
/* GRPH blocks at 0x6800, 0x7400, 0x10000, 0x10c00, 0x11800, 0x12400 */
#define EVERGREEN_GRPH_ENABLE                           0x6800
#define EVERGREEN_GRPH_CONTROL                          0x6804
#       define EVERGREEN_GRPH_DEPTH(x)                  (((x) & 0x3) << 0)
#       define EVERGREEN_GRPH_DEPTH_8BPP                0
#       define EVERGREEN_GRPH_DEPTH_16BPP               1
#       define EVERGREEN_GRPH_DEPTH_32BPP               2
#       define EVERGREEN_GRPH_FORMAT(x)                 (((x) & 0x7) << 8)
/* 8 BPP */
#       define EVERGREEN_GRPH_FORMAT_INDEXED            0
/* 16 BPP */
#       define EVERGREEN_GRPH_FORMAT_ARGB1555           0
#       define EVERGREEN_GRPH_FORMAT_ARGB565            1
#       define EVERGREEN_GRPH_FORMAT_ARGB4444           2
#       define EVERGREEN_GRPH_FORMAT_AI88               3
#       define EVERGREEN_GRPH_FORMAT_MONO16             4
#       define EVERGREEN_GRPH_FORMAT_BGRA5551           5
/* 32 BPP */
#       define EVERGREEN_GRPH_FORMAT_ARGB8888           0
#       define EVERGREEN_GRPH_FORMAT_ARGB2101010        1
#       define EVERGREEN_GRPH_FORMAT_32BPP_DIG          2
#       define EVERGREEN_GRPH_FORMAT_8B_ARGB2101010     3
#       define EVERGREEN_GRPH_FORMAT_BGRA1010102        4
#       define EVERGREEN_GRPH_FORMAT_8B_BGRA1010102     5
#       define EVERGREEN_GRPH_FORMAT_RGB111110          6
#       define EVERGREEN_GRPH_FORMAT_BGR101111          7
#define EVERGREEN_GRPH_SWAP_CONTROL                     0x680c
#       define EVERGREEN_GRPH_ENDIAN_SWAP(x)            (((x) & 0x3) << 0)
#       define EVERGREEN_GRPH_ENDIAN_NONE               0
#       define EVERGREEN_GRPH_ENDIAN_8IN16              1
#       define EVERGREEN_GRPH_ENDIAN_8IN32              2
#       define EVERGREEN_GRPH_ENDIAN_8IN64              3
#       define EVERGREEN_GRPH_RED_CROSSBAR(x)           (((x) & 0x3) << 4)
#       define EVERGREEN_GRPH_RED_SEL_R                 0
#       define EVERGREEN_GRPH_RED_SEL_G                 1
#       define EVERGREEN_GRPH_RED_SEL_B                 2
#       define EVERGREEN_GRPH_RED_SEL_A                 3
#       define EVERGREEN_GRPH_GREEN_CROSSBAR(x)         (((x) & 0x3) << 6)
#       define EVERGREEN_GRPH_GREEN_SEL_G               0
#       define EVERGREEN_GRPH_GREEN_SEL_B               1
#       define EVERGREEN_GRPH_GREEN_SEL_A               2
#       define EVERGREEN_GRPH_GREEN_SEL_R               3
#       define EVERGREEN_GRPH_BLUE_CROSSBAR(x)          (((x) & 0x3) << 8)
#       define EVERGREEN_GRPH_BLUE_SEL_B                0
#       define EVERGREEN_GRPH_BLUE_SEL_A                1
#       define EVERGREEN_GRPH_BLUE_SEL_R                2
#       define EVERGREEN_GRPH_BLUE_SEL_G                3
#       define EVERGREEN_GRPH_ALPHA_CROSSBAR(x)         (((x) & 0x3) << 10)
#       define EVERGREEN_GRPH_ALPHA_SEL_A               0
#       define EVERGREEN_GRPH_ALPHA_SEL_R               1
#       define EVERGREEN_GRPH_ALPHA_SEL_G               2
#       define EVERGREEN_GRPH_ALPHA_SEL_B               3
#define EVERGREEN_GRPH_PRIMARY_SURFACE_ADDRESS          0x6810
#define EVERGREEN_GRPH_SECONDARY_SURFACE_ADDRESS        0x6814
#       define EVERGREEN_GRPH_DFQ_ENABLE                (1 << 0)
#       define EVERGREEN_GRPH_SURFACE_ADDRESS_MASK      0xffffff00
#define EVERGREEN_GRPH_PITCH                            0x6818
#define EVERGREEN_GRPH_PRIMARY_SURFACE_ADDRESS_HIGH     0x681c
#define EVERGREEN_GRPH_SECONDARY_SURFACE_ADDRESS_HIGH   0x6820
#define EVERGREEN_GRPH_SURFACE_OFFSET_X                 0x6824
#define EVERGREEN_GRPH_SURFACE_OFFSET_Y                 0x6828
#define EVERGREEN_GRPH_X_START                          0x682c
#define EVERGREEN_GRPH_Y_START                          0x6830
#define EVERGREEN_GRPH_X_END                            0x6834
#define EVERGREEN_GRPH_Y_END                            0x6838

/* CUR blocks at 0x6998, 0x7598, 0x10198, 0x10d98, 0x11998, 0x12598 */
#define EVERGREEN_CUR_CONTROL                           0x6998
#       define EVERGREEN_CURSOR_EN                      (1 << 0)
#       define EVERGREEN_CURSOR_MODE(x)                 (((x) & 0x3) << 8)
#       define EVERGREEN_CURSOR_MONO                    0
#       define EVERGREEN_CURSOR_24_1                    1
#       define EVERGREEN_CURSOR_24_8_PRE_MULT           2
#       define EVERGREEN_CURSOR_24_8_UNPRE_MULT         3
#       define EVERGREEN_CURSOR_2X_MAGNIFY              (1 << 16)
#       define EVERGREEN_CURSOR_FORCE_MC_ON             (1 << 20)
#       define EVERGREEN_CURSOR_URGENT_CONTROL(x)       (((x) & 0x7) << 24)
#       define EVERGREEN_CURSOR_URGENT_ALWAYS           0
#       define EVERGREEN_CURSOR_URGENT_1_8              1
#       define EVERGREEN_CURSOR_URGENT_1_4              2
#       define EVERGREEN_CURSOR_URGENT_3_8              3
#       define EVERGREEN_CURSOR_URGENT_1_2              4
#define EVERGREEN_CUR_SURFACE_ADDRESS                   0x699c
#       define EVERGREEN_CUR_SURFACE_ADDRESS_MASK       0xfffff000
#define EVERGREEN_CUR_SIZE                              0x69a0
#define EVERGREEN_CUR_SURFACE_ADDRESS_HIGH              0x69a4
#define EVERGREEN_CUR_POSITION                          0x69a8
#define EVERGREEN_CUR_HOT_SPOT                          0x69ac
#define EVERGREEN_CUR_COLOR1                            0x69b0
#define EVERGREEN_CUR_COLOR2                            0x69b4
#define EVERGREEN_CUR_UPDATE                            0x69b8
#       define EVERGREEN_CURSOR_UPDATE_PENDING          (1 << 0)
#       define EVERGREEN_CURSOR_UPDATE_TAKEN            (1 << 1)
#       define EVERGREEN_CURSOR_UPDATE_LOCK             (1 << 16)
#       define EVERGREEN_CURSOR_DISABLE_MULTIPLE_UPDATE (1 << 24)

/* LUT blocks at 0x69e0, 0x75e0, 0x101e0, 0x10de0, 0x119e0, 0x125e0 */
#define EVERGREEN_DC_LUT_RW_MODE                        0x69e0
#define EVERGREEN_DC_LUT_RW_INDEX                       0x69e4
#define EVERGREEN_DC_LUT_SEQ_COLOR                      0x69e8
#define EVERGREEN_DC_LUT_PWL_DATA                       0x69ec
#define EVERGREEN_DC_LUT_30_COLOR                       0x69f0
#define EVERGREEN_DC_LUT_VGA_ACCESS_ENABLE              0x69f4
#define EVERGREEN_DC_LUT_WRITE_EN_MASK                  0x69f8
#define EVERGREEN_DC_LUT_AUTOFILL                       0x69fc
#define EVERGREEN_DC_LUT_CONTROL                        0x6a00
#define EVERGREEN_DC_LUT_BLACK_OFFSET_BLUE              0x6a04
#define EVERGREEN_DC_LUT_BLACK_OFFSET_GREEN             0x6a08
#define EVERGREEN_DC_LUT_BLACK_OFFSET_RED               0x6a0c
#define EVERGREEN_DC_LUT_WHITE_OFFSET_BLUE              0x6a10
#define EVERGREEN_DC_LUT_WHITE_OFFSET_GREEN             0x6a14
#define EVERGREEN_DC_LUT_WHITE_OFFSET_RED               0x6a18

#define EVERGREEN_DATA_FORMAT                           0x6b00
#       define EVERGREEN_INTERLEAVE_EN                  (1 << 0)
#define EVERGREEN_DESKTOP_HEIGHT                        0x6b04

#define EVERGREEN_VIEWPORT_START                        0x6d70
#define EVERGREEN_VIEWPORT_SIZE                         0x6d74

/* display controller offsets used for crtc/cur/lut/grph/viewport/etc. */
#define EVERGREEN_CRTC0_REGISTER_OFFSET                 (0x6df0 - 0x6df0)
#define EVERGREEN_CRTC1_REGISTER_OFFSET                 (0x79f0 - 0x6df0)
#define EVERGREEN_CRTC2_REGISTER_OFFSET                 (0x105f0 - 0x6df0)
#define EVERGREEN_CRTC3_REGISTER_OFFSET                 (0x111f0 - 0x6df0)
#define EVERGREEN_CRTC4_REGISTER_OFFSET                 (0x11df0 - 0x6df0)
#define EVERGREEN_CRTC5_REGISTER_OFFSET                 (0x129f0 - 0x6df0)

/* CRTC blocks at 0x6df0, 0x79f0, 0x105f0, 0x111f0, 0x11df0, 0x129f0 */
#define EVERGREEN_CRTC_CONTROL                          0x6e70
#       define EVERGREEN_CRTC_MASTER_EN                 (1 << 0)
#define EVERGREEN_CRTC_UPDATE_LOCK                      0x6ed4

#define EVERGREEN_DC_GPIO_HPD_MASK                      0x64b0
#define EVERGREEN_DC_GPIO_HPD_A                         0x64b4
#define EVERGREEN_DC_GPIO_HPD_EN                        0x64b8
#define EVERGREEN_DC_GPIO_HPD_Y                         0x64bc

#define R300_GB_TILE_CONFIG				0x4018
#       define R300_ENABLE_TILING                       (1 << 0)
#       define R300_PIPE_COUNT_RV350                    (0 << 1)
#       define R300_PIPE_COUNT_R300                     (3 << 1)
#       define R300_PIPE_COUNT_R420_3P                  (6 << 1)
#       define R300_PIPE_COUNT_R420                     (7 << 1)
#       define R300_TILE_SIZE_8                         (0 << 4)
#       define R300_TILE_SIZE_16                        (1 << 4)
#       define R300_TILE_SIZE_32                        (2 << 4)
#       define R300_SUBPIXEL_1_12                       (0 << 16)
#       define R300_SUBPIXEL_1_16                       (1 << 16)
#define R300_GB_SELECT				        0x401c
#define R300_GB_ENABLE				        0x4008
#define R300_GB_AA_CONFIG				0x4020
#define R400_GB_PIPE_SELECT                             0x402c
#define R300_GB_MSPOS0				        0x4010
#       define R300_MS_X0_SHIFT                         0
#       define R300_MS_Y0_SHIFT                         4
#       define R300_MS_X1_SHIFT                         8
#       define R300_MS_Y1_SHIFT                         12
#       define R300_MS_X2_SHIFT                         16
#       define R300_MS_Y2_SHIFT                         20
#       define R300_MSBD0_Y_SHIFT                       24
#       define R300_MSBD0_X_SHIFT                       28
#define R300_GB_MSPOS1				        0x4014
#       define R300_MS_X3_SHIFT                         0
#       define R300_MS_Y3_SHIFT                         4
#       define R300_MS_X4_SHIFT                         8
#       define R300_MS_Y4_SHIFT                         12
#       define R300_MS_X5_SHIFT                         16
#       define R300_MS_Y5_SHIFT                         20
#       define R300_MSBD1_SHIFT                         24

#define R300_GA_ENHANCE				        0x4274
#       define R300_GA_DEADLOCK_CNTL                    (1 << 0)
#       define R300_GA_FASTSYNC_CNTL                    (1 << 1)

#define R300_GA_POLY_MODE				0x4288
#       define R300_FRONT_PTYPE_POINT                   (0 << 4)
#       define R300_FRONT_PTYPE_LINE                    (1 << 4)
#       define R300_FRONT_PTYPE_TRIANGE                 (2 << 4)
#       define R300_BACK_PTYPE_POINT                    (0 << 7)
#       define R300_BACK_PTYPE_LINE                     (1 << 7)
#       define R300_BACK_PTYPE_TRIANGE                  (2 << 7)
#define R300_GA_ROUND_MODE				0x428c
#       define R300_GEOMETRY_ROUND_TRUNC                (0 << 0)
#       define R300_GEOMETRY_ROUND_NEAREST              (1 << 0)
#       define R300_COLOR_ROUND_TRUNC                   (0 << 2)
#       define R300_COLOR_ROUND_NEAREST                 (1 << 2)
#define R300_GA_COLOR_CONTROL			        0x4278
#       define R300_RGB0_SHADING_SOLID                  (0 << 0)
#       define R300_RGB0_SHADING_FLAT                   (1 << 0)
#       define R300_RGB0_SHADING_GOURAUD                (2 << 0)
#       define R300_ALPHA0_SHADING_SOLID                (0 << 2)
#       define R300_ALPHA0_SHADING_FLAT                 (1 << 2)
#       define R300_ALPHA0_SHADING_GOURAUD              (2 << 2)
#       define R300_RGB1_SHADING_SOLID                  (0 << 4)
#       define R300_RGB1_SHADING_FLAT                   (1 << 4)
#       define R300_RGB1_SHADING_GOURAUD                (2 << 4)
#       define R300_ALPHA1_SHADING_SOLID                (0 << 6)
#       define R300_ALPHA1_SHADING_FLAT                 (1 << 6)
#       define R300_ALPHA1_SHADING_GOURAUD              (2 << 6)
#       define R300_RGB2_SHADING_SOLID                  (0 << 8)
#       define R300_RGB2_SHADING_FLAT                   (1 << 8)
#       define R300_RGB2_SHADING_GOURAUD                (2 << 8)
#       define R300_ALPHA2_SHADING_SOLID                (0 << 10)
#       define R300_ALPHA2_SHADING_FLAT                 (1 << 10)
#       define R300_ALPHA2_SHADING_GOURAUD              (2 << 10)
#       define R300_RGB3_SHADING_SOLID                  (0 << 12)
#       define R300_RGB3_SHADING_FLAT                   (1 << 12)
#       define R300_RGB3_SHADING_GOURAUD                (2 << 12)
#       define R300_ALPHA3_SHADING_SOLID                (0 << 14)
#       define R300_ALPHA3_SHADING_FLAT                 (1 << 14)
#       define R300_ALPHA3_SHADING_GOURAUD              (2 << 14)
#define R300_GA_OFFSET				        0x4290

#define R500_SU_REG_DEST                                0x42c8

#define R300_VAP_CNTL_STATUS				0x2140
#       define R300_PVS_BYPASS                          (1 << 8)
#define R300_VAP_PVS_STATE_FLUSH_REG		        0x2284
#define R300_VAP_CNTL				        0x2080
#       define R300_PVS_NUM_SLOTS_SHIFT                 0
#       define R300_PVS_NUM_CNTLRS_SHIFT                4
#       define R300_PVS_NUM_FPUS_SHIFT                  8
#       define R300_VF_MAX_VTX_NUM_SHIFT                18
#       define R300_GL_CLIP_SPACE_DEF                   (0 << 22)
#       define R300_DX_CLIP_SPACE_DEF                   (1 << 22)
#       define R500_TCL_STATE_OPTIMIZATION              (1 << 23)
#define R300_VAP_VTE_CNTL				0x20B0
#       define R300_VPORT_X_SCALE_ENA                   (1 << 0)
#       define R300_VPORT_X_OFFSET_ENA                  (1 << 1)
#       define R300_VPORT_Y_SCALE_ENA                   (1 << 2)
#       define R300_VPORT_Y_OFFSET_ENA                  (1 << 3)
#       define R300_VPORT_Z_SCALE_ENA                   (1 << 4)
#       define R300_VPORT_Z_OFFSET_ENA                  (1 << 5)
#       define R300_VTX_XY_FMT                          (1 << 8)
#       define R300_VTX_Z_FMT                           (1 << 9)
#       define R300_VTX_W0_FMT                          (1 << 10)
#define R300_VAP_VTX_STATE_CNTL		                0x2180
#define R300_VAP_PSC_SGN_NORM_CNTL		        0x21DC
#define R300_VAP_PROG_STREAM_CNTL_0		        0x2150
#       define R300_DATA_TYPE_0_SHIFT                   0
#       define R300_DATA_TYPE_FLOAT_1                   0
#       define R300_DATA_TYPE_FLOAT_2                   1
#       define R300_DATA_TYPE_FLOAT_3                   2
#       define R300_DATA_TYPE_FLOAT_4                   3
#       define R300_DATA_TYPE_BYTE                      4
#       define R300_DATA_TYPE_D3DCOLOR                  5
#       define R300_DATA_TYPE_SHORT_2                   6
#       define R300_DATA_TYPE_SHORT_4                   7
#       define R300_DATA_TYPE_VECTOR_3_TTT              8
#       define R300_DATA_TYPE_VECTOR_3_EET              9
#       define R300_SKIP_DWORDS_0_SHIFT                 4
#       define R300_DST_VEC_LOC_0_SHIFT                 8
#       define R300_LAST_VEC_0                          (1 << 13)
#       define R300_SIGNED_0                            (1 << 14)
#       define R300_NORMALIZE_0                         (1 << 15)
#       define R300_DATA_TYPE_1_SHIFT                   16
#       define R300_SKIP_DWORDS_1_SHIFT                 20
#       define R300_DST_VEC_LOC_1_SHIFT                 24
#       define R300_LAST_VEC_1                          (1 << 29)
#       define R300_SIGNED_1                            (1 << 30)
#       define R300_NORMALIZE_1                         (1 << 31)
#define R300_VAP_PROG_STREAM_CNTL_1		        0x2154
#       define R300_DATA_TYPE_2_SHIFT                   0
#       define R300_SKIP_DWORDS_2_SHIFT                 4
#       define R300_DST_VEC_LOC_2_SHIFT                 8
#       define R300_LAST_VEC_2                          (1 << 13)
#       define R300_SIGNED_2                            (1 << 14)
#       define R300_NORMALIZE_2                         (1 << 15)
#       define R300_DATA_TYPE_3_SHIFT                   16
#       define R300_SKIP_DWORDS_3_SHIFT                 20
#       define R300_DST_VEC_LOC_3_SHIFT                 24
#       define R300_LAST_VEC_3                          (1 << 29)
#       define R300_SIGNED_3                            (1 << 30)
#       define R300_NORMALIZE_3                         (1 << 31)
#define R300_VAP_PROG_STREAM_CNTL_EXT_0	                0x21e0
#       define R300_SWIZZLE_SELECT_X_0_SHIFT            0
#       define R300_SWIZZLE_SELECT_Y_0_SHIFT            3
#       define R300_SWIZZLE_SELECT_Z_0_SHIFT            6
#       define R300_SWIZZLE_SELECT_W_0_SHIFT            9
#       define R300_SWIZZLE_SELECT_X                    0
#       define R300_SWIZZLE_SELECT_Y                    1
#       define R300_SWIZZLE_SELECT_Z                    2
#       define R300_SWIZZLE_SELECT_W                    3
#       define R300_SWIZZLE_SELECT_FP_ZERO              4
#       define R300_SWIZZLE_SELECT_FP_ONE               5
#       define R300_WRITE_ENA_0_SHIFT                   12
#       define R300_WRITE_ENA_X                         1
#       define R300_WRITE_ENA_Y                         2
#       define R300_WRITE_ENA_Z                         4
#       define R300_WRITE_ENA_W                         8
#       define R300_SWIZZLE_SELECT_X_1_SHIFT            16
#       define R300_SWIZZLE_SELECT_Y_1_SHIFT            19
#       define R300_SWIZZLE_SELECT_Z_1_SHIFT            22
#       define R300_SWIZZLE_SELECT_W_1_SHIFT            25
#       define R300_WRITE_ENA_1_SHIFT                   28
#define R300_VAP_PROG_STREAM_CNTL_EXT_1	                0x21e4
#       define R300_SWIZZLE_SELECT_X_2_SHIFT            0
#       define R300_SWIZZLE_SELECT_Y_2_SHIFT            3
#       define R300_SWIZZLE_SELECT_Z_2_SHIFT            6
#       define R300_SWIZZLE_SELECT_W_2_SHIFT            9
#       define R300_WRITE_ENA_2_SHIFT                   12
#       define R300_SWIZZLE_SELECT_X_3_SHIFT            16
#       define R300_SWIZZLE_SELECT_Y_3_SHIFT            19
#       define R300_SWIZZLE_SELECT_Z_3_SHIFT            22
#       define R300_SWIZZLE_SELECT_W_3_SHIFT            25
#       define R300_WRITE_ENA_3_SHIFT                   28
#define R300_VAP_PVS_CODE_CNTL_0			0x22D0
#       define R300_PVS_FIRST_INST_SHIFT                0
#       define R300_PVS_XYZW_VALID_INST_SHIFT           10
#       define R300_PVS_LAST_INST_SHIFT                 20
#define R300_VAP_PVS_CODE_CNTL_1			0x22D8
#       define R300_PVS_LAST_VTX_SRC_INST_SHIFT         0
#define R300_VAP_PVS_VECTOR_INDX_REG		        0x2200
#       define R300_PVS_CODE_START                      0
#       define R300_PVS_CONST_START                     512
#       define R500_PVS_CONST_START                     1024
#       define R300_PVS_VECTOR_INST_INDEX(x)            ((x) + R300_PVS_CODE_START)
#       define R300_PVS_VECTOR_CONST_INDEX(x)           ((x) + R300_PVS_CONST_START)
#       define R500_PVS_VECTOR_CONST_INDEX(x)           ((x) + R500_PVS_CONST_START)
#define R300_VAP_PVS_VECTOR_DATA_REG		        0x2204
/* PVS instructions */
/* Opcode and dst instruction */
#define R300_PVS_DST_OPCODE(x)                          ((x) << 0)
/* Vector ops */
#       define R300_VECTOR_NO_OP                        0
#       define R300_VE_DOT_PRODUCT                      1
#       define R300_VE_MULTIPLY                         2
#       define R300_VE_ADD                              3
#       define R300_VE_MULTIPLY_ADD                     4
#       define R300_VE_DISTANCE_VECTOR                  5
#       define R300_VE_FRACTION                         6
#       define R300_VE_MAXIMUM                          7
#       define R300_VE_MINIMUM                          8
#       define R300_VE_SET_GREATER_THAN_EQUAL           9
#       define R300_VE_SET_LESS_THAN                    10
#       define R300_VE_MULTIPLYX2_ADD                   11
#       define R300_VE_MULTIPLY_CLAMP                   12
#       define R300_VE_FLT2FIX_DX                       13
#       define R300_VE_FLT2FIX_DX_RND                   14
/* R500 additions */
#       define R500_VE_PRED_SET_EQ_PUSH                 15
#       define R500_VE_PRED_SET_GT_PUSH                 16
#       define R500_VE_PRED_SET_GTE_PUSH                17
#       define R500_VE_PRED_SET_NEQ_PUSH                18
#       define R500_VE_COND_WRITE_EQ                    19
#       define R500_VE_COND_WRITE_GT                    20
#       define R500_VE_COND_WRITE_GTE                   21
#       define R500_VE_COND_WRITE_NEQ                   22
#       define R500_VE_COND_MUX_EQ                      23
#       define R500_VE_COND_MUX_GT                      24
#       define R500_VE_COND_MUX_GTE                     25
#       define R500_VE_SET_GREATER_THAN                 26
#       define R500_VE_SET_EQUAL                        27
#       define R500_VE_SET_NOT_EQUAL                    28
/* Math ops */
#       define R300_MATH_NO_OP                          0
#       define R300_ME_EXP_BASE2_DX                     1
#       define R300_ME_LOG_BASE2_DX                     2
#       define R300_ME_EXP_BASEE_FF                     3
#       define R300_ME_LIGHT_COEFF_DX                   4
#       define R300_ME_POWER_FUNC_FF                    5
#       define R300_ME_RECIP_DX                         6
#       define R300_ME_RECIP_FF                         7
#       define R300_ME_RECIP_SQRT_DX                    8
#       define R300_ME_RECIP_SQRT_FF                    9
#       define R300_ME_MULTIPLY                         10
#       define R300_ME_EXP_BASE2_FULL_DX                11
#       define R300_ME_LOG_BASE2_FULL_DX                12
#       define R300_ME_POWER_FUNC_FF_CLAMP_B            13
#       define R300_ME_POWER_FUNC_FF_CLAMP_B1           14
#       define R300_ME_POWER_FUNC_FF_CLAMP_01           15
#       define R300_ME_SIN                              16
#       define R300_ME_COS                              17
/* R500 additions */
#       define R500_ME_LOG_BASE2_IEEE                   18
#       define R500_ME_RECIP_IEEE                       19
#       define R500_ME_RECIP_SQRT_IEEE                  20
#       define R500_ME_PRED_SET_EQ                      21
#       define R500_ME_PRED_SET_GT                      22
#       define R500_ME_PRED_SET_GTE                     23
#       define R500_ME_PRED_SET_NEQ                     24
#       define R500_ME_PRED_SET_CLR                     25
#       define R500_ME_PRED_SET_INV                     26
#       define R500_ME_PRED_SET_POP                     27
#       define R500_ME_PRED_SET_RESTORE                 28
/* macro */
#       define R300_PVS_MACRO_OP_2CLK_MADD              0
#       define R300_PVS_MACRO_OP_2CLK_M2X_ADD           1
#define R300_PVS_DST_MATH_INST                          (1 << 6)
#define R300_PVS_DST_MACRO_INST                         (1 << 7)
#define R300_PVS_DST_REG_TYPE(x)                        ((x) << 8)
#       define R300_PVS_DST_REG_TEMPORARY               0
#       define R300_PVS_DST_REG_A0                      1
#       define R300_PVS_DST_REG_OUT                     2
#       define R500_PVS_DST_REG_OUT_REPL_X              3
#       define R300_PVS_DST_REG_ALT_TEMPORARY           4
#       define R300_PVS_DST_REG_INPUT                   5
#define R300_PVS_DST_ADDR_MODE_1                        (1 << 12)
#define R300_PVS_DST_OFFSET(x)                          ((x) << 13)
#define R300_PVS_DST_WE_X                               (1 << 20)
#define R300_PVS_DST_WE_Y                               (1 << 21)
#define R300_PVS_DST_WE_Z                               (1 << 22)
#define R300_PVS_DST_WE_W                               (1 << 23)
#define R300_PVS_DST_VE_SAT                             (1 << 24)
#define R300_PVS_DST_ME_SAT                             (1 << 25)
#define R300_PVS_DST_PRED_ENABLE                        (1 << 26)
#define R300_PVS_DST_PRED_SENSE                         (1 << 27)
#define R300_PVS_DST_DUAL_MATH_OP                       (1 << 28)
#define R300_PVS_DST_ADDR_SEL(x)                        ((x) << 29)
#define R300_PVS_DST_ADDR_MODE_0                        (1 << 31)
/* src operand instruction */
#define R300_PVS_SRC_REG_TYPE(x)                        ((x) << 0)
#       define R300_PVS_SRC_REG_TEMPORARY               0
#       define R300_PVS_SRC_REG_INPUT                   1
#       define R300_PVS_SRC_REG_CONSTANT                2
#       define R300_PVS_SRC_REG_ALT_TEMPORARY           3
#define R300_SPARE_0                                    (1 << 2)
#define R300_PVS_SRC_ABS_XYZW                           (1 << 3)
#define R300_PVS_SRC_ADDR_MODE_0                        (1 << 4)
#define R300_PVS_SRC_OFFSET(x)                          ((x) << 5)
#define R300_PVS_SRC_SWIZZLE_X(x)                       ((x) << 13)
#define R300_PVS_SRC_SWIZZLE_Y(x)                       ((x) << 16)
#define R300_PVS_SRC_SWIZZLE_Z(x)                       ((x) << 19)
#define R300_PVS_SRC_SWIZZLE_W(x)                       ((x) << 22)
#       define R300_PVS_SRC_SELECT_X                    0
#       define R300_PVS_SRC_SELECT_Y                    1
#       define R300_PVS_SRC_SELECT_Z                    2
#       define R300_PVS_SRC_SELECT_W                    3
#       define R300_PVS_SRC_SELECT_FORCE_0              4
#       define R300_PVS_SRC_SELECT_FORCE_1              5
#define R300_PVS_SRC_NEG_X                              (1 << 25)
#define R300_PVS_SRC_NEG_Y                              (1 << 26)
#define R300_PVS_SRC_NEG_Z                              (1 << 27)
#define R300_PVS_SRC_NEG_W                              (1 << 28)
#define R300_PVS_SRC_ADDR_SEL(x)                        ((x) << 29)
#define R300_PVS_SRC_ADDR_MODE_1                        (1 << 31)

#define R300_VAP_PVS_CONST_CNTL                         0x22d4
#       define R300_PVS_CONST_BASE_OFFSET(x)            ((x) << 0)
#       define R300_PVS_MAX_CONST_ADDR(x)               ((x) << 16)

#define R300_VAP_PVS_FLOW_CNTL_OPC		        0x22dc
#define R300_VAP_OUT_VTX_FMT_0			        0x2090
#       define R300_VTX_POS_PRESENT                     (1 << 0)
#       define R300_VTX_COLOR_0_PRESENT                 (1 << 1)
#       define R300_VTX_COLOR_1_PRESENT                 (1 << 2)
#       define R300_VTX_COLOR_2_PRESENT                 (1 << 3)
#       define R300_VTX_COLOR_3_PRESENT                 (1 << 4)
#       define R300_VTX_PT_SIZE_PRESENT                 (1 << 16)
#define R300_VAP_OUT_VTX_FMT_1			        0x2094
#       define R300_TEX_0_COMP_CNT_SHIFT                0
#       define R300_TEX_1_COMP_CNT_SHIFT                3
#       define R300_TEX_2_COMP_CNT_SHIFT                6
#       define R300_TEX_3_COMP_CNT_SHIFT                9
#       define R300_TEX_4_COMP_CNT_SHIFT                12
#       define R300_TEX_5_COMP_CNT_SHIFT                15
#       define R300_TEX_6_COMP_CNT_SHIFT                18
#       define R300_TEX_7_COMP_CNT_SHIFT                21
#define R300_VAP_VTX_SIZE				0x20b4
#define R300_VAP_GB_VERT_CLIP_ADJ		        0x2220
#define R300_VAP_GB_VERT_DISC_ADJ		        0x2224
#define R300_VAP_GB_HORZ_CLIP_ADJ		        0x2228
#define R300_VAP_GB_HORZ_DISC_ADJ		        0x222c
#define R300_VAP_CLIP_CNTL				0x221c
#       define R300_UCP_ENA_0                           (1 << 0)
#       define R300_UCP_ENA_1                           (1 << 1)
#       define R300_UCP_ENA_2                           (1 << 2)
#       define R300_UCP_ENA_3                           (1 << 3)
#       define R300_UCP_ENA_4                           (1 << 4)
#       define R300_UCP_ENA_5                           (1 << 5)
#       define R300_PS_UCP_MODE_SHIFT                   14
#       define R300_CLIP_DISABLE                        (1 << 16)
#       define R300_UCP_CULL_ONLY_ENA                   (1 << 17)
#       define R300_BOUNDARY_EDGE_FLAG_ENA              (1 << 18)
#define R300_VAP_PVS_STATE_FLUSH_REG			0x2284

#define R500_VAP_INDEX_OFFSET			        0x208c

#define R300_SU_TEX_WRAP				0x42a0
#define R300_SU_POLY_OFFSET_ENABLE		        0x42b4
#define R300_SU_CULL_MODE				0x42b8
#       define R300_CULL_FRONT                          (1 << 0)
#       define R300_CULL_BACK                           (1 << 1)
#       define R300_FACE_POS                            (0 << 2)
#       define R300_FACE_NEG                            (1 << 2)
#define R300_SU_DEPTH_SCALE				0x42c0
#define R300_SU_DEPTH_OFFSET			        0x42c4

#define R300_RS_COUNT				        0x4300
#	define R300_RS_COUNT_IT_COUNT_SHIFT		0
#	define R300_RS_COUNT_IC_COUNT_SHIFT		7
#	define R300_RS_COUNT_HIRES_EN			(1 << 18)

#define R300_RS_IP_0				        0x4310
#define R300_RS_IP_1				        0x4314
#	define R300_RS_TEX_PTR(x)		        ((x) << 0)
#	define R300_RS_COL_PTR(x)		        ((x) << 6)
#	define R300_RS_COL_FMT(x)		        ((x) << 9)
#	define R300_RS_COL_FMT_RGBA		        0
#	define R300_RS_COL_FMT_RGB0		        2
#	define R300_RS_COL_FMT_RGB1		        3
#	define R300_RS_COL_FMT_000A		        4
#	define R300_RS_COL_FMT_0000		        5
#	define R300_RS_COL_FMT_0001		        6
#	define R300_RS_COL_FMT_111A		        8
#	define R300_RS_COL_FMT_1110		        9
#	define R300_RS_COL_FMT_1111		        10
#	define R300_RS_SEL_S(x)		                ((x) << 13)
#	define R300_RS_SEL_T(x)		                ((x) << 16)
#	define R300_RS_SEL_R(x)		                ((x) << 19)
#	define R300_RS_SEL_Q(x)		                ((x) << 22)
#	define R300_RS_SEL_C0		                0
#	define R300_RS_SEL_C1		                1
#	define R300_RS_SEL_C2		                2
#	define R300_RS_SEL_C3		                3
#	define R300_RS_SEL_K0		                4
#	define R300_RS_SEL_K1		                5
#define R300_RS_INST_COUNT				0x4304
#	define R300_INST_COUNT_RS(x)		        ((x) << 0)
#	define R300_RS_W_EN			        (1 << 4)
#	define R300_TX_OFFSET_RS(x)		        ((x) << 5)
#define R300_RS_INST_0				        0x4330
#define R300_RS_INST_1				        0x4334
#	define R300_INST_TEX_ID(x)		        ((x) << 0)
#       define R300_RS_INST_TEX_CN_WRITE		(1 << 3)
#	define R300_INST_TEX_ADDR(x)		        ((x) << 6)

#define R300_TX_INVALTAGS				0x4100
#define R300_TX_FILTER0_0				0x4400
#define R300_TX_FILTER0_1				0x4404
#define R300_TX_FILTER0_2				0x4408
#       define R300_TX_CLAMP_S(x)                       ((x) << 0)
#       define R300_TX_CLAMP_T(x)                       ((x) << 3)
#       define R300_TX_CLAMP_R(x)                       ((x) << 6)
#       define R300_TX_CLAMP_WRAP                       0
#       define R300_TX_CLAMP_MIRROR                     1
#       define R300_TX_CLAMP_CLAMP_LAST                 2
#       define R300_TX_CLAMP_MIRROR_CLAMP_LAST          3
#       define R300_TX_CLAMP_CLAMP_BORDER               4
#       define R300_TX_CLAMP_MIRROR_CLAMP_BORDER        5
#       define R300_TX_CLAMP_CLAMP_GL                   6
#       define R300_TX_CLAMP_MIRROR_CLAMP_GL            7
#       define R300_TX_MAG_FILTER_NEAREST               (1 << 9)
#       define R300_TX_MIN_FILTER_NEAREST               (1 << 11)
#       define R300_TX_MAG_FILTER_LINEAR                (2 << 9)
#       define R300_TX_MIN_FILTER_LINEAR                (2 << 11)
#       define R300_TX_ID_SHIFT                         28
#define R300_TX_FILTER1_0				0x4440
#define R300_TX_FILTER1_1				0x4444
#define R300_TX_FILTER1_2				0x4448
#define R300_TX_FORMAT0_0				0x4480
#define R300_TX_FORMAT0_1				0x4484
#define R300_TX_FORMAT0_2				0x4488
#       define R300_TXWIDTH_SHIFT                       0
#       define R300_TXHEIGHT_SHIFT                      11
#       define R300_NUM_LEVELS_SHIFT                    26
#       define R300_NUM_LEVELS_MASK                     0x
#       define R300_TXPROJECTED                         (1 << 30)
#       define R300_TXPITCH_EN                          (1 << 31)
#define R300_TX_FORMAT1_0				0x44c0
#define R300_TX_FORMAT1_1				0x44c4
#define R300_TX_FORMAT1_2				0x44c8
#	define R300_TX_FORMAT_X8		    0x0
#	define R300_TX_FORMAT_X16		    0x1
#	define R300_TX_FORMAT_Y4X4		    0x2
#	define R300_TX_FORMAT_Y8X8		    0x3
#	define R300_TX_FORMAT_Y16X16		    0x4
#	define R300_TX_FORMAT_Z3Y3X2		    0x5
#	define R300_TX_FORMAT_Z5Y6X5		    0x6
#	define R300_TX_FORMAT_Z6Y5X5		    0x7
#	define R300_TX_FORMAT_Z11Y11X10		    0x8
#	define R300_TX_FORMAT_Z10Y11X11		    0x9
#	define R300_TX_FORMAT_W4Z4Y4X4		    0xA
#	define R300_TX_FORMAT_W1Z5Y5X5		    0xB
#	define R300_TX_FORMAT_W8Z8Y8X8		    0xC
#	define R300_TX_FORMAT_W2Z10Y10X10	    0xD
#	define R300_TX_FORMAT_W16Z16Y16X16	    0xE
#	define R300_TX_FORMAT_DXT1	    	    0xF
#	define R300_TX_FORMAT_DXT3	    	    0x10
#	define R300_TX_FORMAT_DXT5	    	    0x11
#	define R300_TX_FORMAT_D3DMFT_CxV8U8	    0x12     /* no swizzle */
#	define R300_TX_FORMAT_A8R8G8B8	    	    0x13     /* no swizzle */
#	define R300_TX_FORMAT_B8G8_B8G8	    	    0x14     /* no swizzle */
#	define R300_TX_FORMAT_G8R8_G8B8	    	    0x15     /* no swizzle */
#	define R300_TX_FORMAT_VYUY422	    	    0x14     /* no swizzle */
#	define R300_TX_FORMAT_YVYU422	    	    0x15     /* no swizzle */
#	define R300_TX_FORMAT_X24_Y8	    	    0x1e
#	define R300_TX_FORMAT_X32	    	    0x1e
	/* Floating point formats */
	/* Note - hardware supports both 16 and 32 bit floating point */
#	define R300_TX_FORMAT_FL_I16	    	    0x18
#	define R300_TX_FORMAT_FL_I16A16	    	    0x19
#	define R300_TX_FORMAT_FL_R16G16B16A16	    0x1A
#	define R300_TX_FORMAT_FL_I32	    	    0x1B
#	define R300_TX_FORMAT_FL_I32A32	    	    0x1C
#	define R300_TX_FORMAT_FL_R32G32B32A32	    0x1D
	/* alpha modes, convenience mostly */
	/* if you have alpha, pick constant appropriate to the
	   number of channels (1 for I8, 2 for I8A8, 4 for R8G8B8A8, etc */
# 	define R300_TX_FORMAT_ALPHA_1CH		    0x000
# 	define R300_TX_FORMAT_ALPHA_2CH		    0x200
# 	define R300_TX_FORMAT_ALPHA_4CH		    0x600
# 	define R300_TX_FORMAT_ALPHA_NONE	    0xA00
	/* Swizzling */
	/* constants */
#	define R300_TX_FORMAT_X		0
#	define R300_TX_FORMAT_Y		1
#	define R300_TX_FORMAT_Z		2
#	define R300_TX_FORMAT_W		3
#	define R300_TX_FORMAT_ZERO	4
#	define R300_TX_FORMAT_ONE	5
	/* 2.0*Z, everything above 1.0 is set to 0.0 */
#	define R300_TX_FORMAT_CUT_Z	6
	/* 2.0*W, everything above 1.0 is set to 0.0 */
#	define R300_TX_FORMAT_CUT_W	7

#	define R300_TX_FORMAT_B_SHIFT	18
#	define R300_TX_FORMAT_G_SHIFT	15
#	define R300_TX_FORMAT_R_SHIFT	12
#	define R300_TX_FORMAT_A_SHIFT	9

	/* Convenience macro to take care of layout and swizzling */
#	define R300_EASY_TX_FORMAT(B, G, R, A, FMT)	(		\
		((R300_TX_FORMAT_##B)<<R300_TX_FORMAT_B_SHIFT)		\
		| ((R300_TX_FORMAT_##G)<<R300_TX_FORMAT_G_SHIFT)	\
		| ((R300_TX_FORMAT_##R)<<R300_TX_FORMAT_R_SHIFT)	\
		| ((R300_TX_FORMAT_##A)<<R300_TX_FORMAT_A_SHIFT)	\
		| (R300_TX_FORMAT_##FMT)				\
		)

#       define R300_TX_FORMAT_YUV_TO_RGB_CLAMP         (1 << 22)
#       define R300_TX_FORMAT_YUV_TO_RGB_NO_CLAMP      (2 << 22)
#       define R300_TX_FORMAT_SWAP_YUV                 (1 << 24)

#       define R300_TX_FORMAT_CACHE_WHOLE              (0 << 27)
#       define R300_TX_FORMAT_CACHE_HALF_REGION_0      (2 << 27)
#       define R300_TX_FORMAT_CACHE_HALF_REGION_1      (3 << 27)
#       define R300_TX_FORMAT_CACHE_FOURTH_REGION_0    (4 << 27)
#       define R300_TX_FORMAT_CACHE_FOURTH_REGION_1    (5 << 27)
#       define R300_TX_FORMAT_CACHE_FOURTH_REGION_2    (6 << 27)
#       define R300_TX_FORMAT_CACHE_FOURTH_REGION_3    (7 << 27)

#define R300_TX_FORMAT2_0				0x4500
#define R300_TX_FORMAT2_1				0x4504
#define R300_TX_FORMAT2_2				0x4508
#       define R500_TXWIDTH_11                          (1 << 15)
#       define R500_TXHEIGHT_11                         (1 << 16)

#define R300_TX_OFFSET_0				0x4540
#define R300_TX_OFFSET_1				0x4544
#define R300_TX_OFFSET_2				0x4548
#       define R300_ENDIAN_SWAP_16_BIT                  (1 << 0)
#       define R300_ENDIAN_SWAP_32_BIT                  (2 << 0)
#       define R300_ENDIAN_SWAP_HALF_DWORD              (3 << 0)
#       define R300_MACRO_TILE                          (1 << 2)

#define R300_TX_BORDER_COLOR_0			        0x45c0

#define R300_TX_ENABLE				        0x4104
#       define R300_TEX_0_ENABLE                        (1 << 0)
#       define R300_TEX_1_ENABLE                        (1 << 1)
#       define R300_TEX_2_ENABLE                        (1 << 2)

#define R300_US_W_FMT				        0x46b4
#define R300_US_OUT_FMT_1				0x46a8
#define R300_US_OUT_FMT_2				0x46ac
#define R300_US_OUT_FMT_3				0x46b0
#define R300_US_OUT_FMT_0				0x46a4
#       define R300_OUT_FMT_C4_8                        (0 << 0)
#       define R300_OUT_FMT_C4_10                       (1 << 0)
#       define R300_OUT_FMT_C4_10_GAMMA                 (2 << 0)
#       define R300_OUT_FMT_C_16                        (3 << 0)
#       define R300_OUT_FMT_C2_16                       (4 << 0)
#       define R300_OUT_FMT_C4_16                       (5 << 0)
#       define R300_OUT_FMT_C_16_MPEG                   (6 << 0)
#       define R300_OUT_FMT_C2_16_MPEG                  (7 << 0)
#       define R300_OUT_FMT_C2_4                        (8 << 0)
#       define R300_OUT_FMT_C_3_3_2                     (9 << 0)
#       define R300_OUT_FMT_C_5_6_5                     (10 << 0)
#       define R300_OUT_FMT_C_11_11_10                  (11 << 0)
#       define R300_OUT_FMT_C_10_11_11                  (12 << 0)
#       define R300_OUT_FMT_C_2_10_10_10                (13 << 0)
#       define R300_OUT_FMT_UNUSED                      (15 << 0)
#       define R300_OUT_FMT_C_16_FP                     (16 << 0)
#       define R300_OUT_FMT_C2_16_FP                    (17 << 0)
#       define R300_OUT_FMT_C4_16_FP                    (18 << 0)
#       define R300_OUT_FMT_C_32_FP                     (19 << 0)
#       define R300_OUT_FMT_C2_32_FP                    (20 << 0)
#       define R300_OUT_FMT_C4_32_FP                    (21 << 0)
#       define R300_OUT_FMT_C0_SEL_ALPHA                (0 << 8)
#       define R300_OUT_FMT_C0_SEL_RED                  (1 << 8)
#       define R300_OUT_FMT_C0_SEL_GREEN                (2 << 8)
#       define R300_OUT_FMT_C0_SEL_BLUE                 (3 << 8)
#       define R300_OUT_FMT_C1_SEL_ALPHA                (0 << 10)
#       define R300_OUT_FMT_C1_SEL_RED                  (1 << 10)
#       define R300_OUT_FMT_C1_SEL_GREEN                (2 << 10)
#       define R300_OUT_FMT_C1_SEL_BLUE                 (3 << 10)
#       define R300_OUT_FMT_C2_SEL_ALPHA                (0 << 12)
#       define R300_OUT_FMT_C2_SEL_RED                  (1 << 12)
#       define R300_OUT_FMT_C2_SEL_GREEN                (2 << 12)
#       define R300_OUT_FMT_C2_SEL_BLUE                 (3 << 12)
#       define R300_OUT_FMT_C3_SEL_ALPHA                (0 << 14)
#       define R300_OUT_FMT_C3_SEL_RED                  (1 << 14)
#       define R300_OUT_FMT_C3_SEL_GREEN                (2 << 14)
#       define R300_OUT_FMT_C3_SEL_BLUE                 (3 << 14)
#define R300_US_CONFIG				        0x4600
#       define R300_NLEVEL_SHIFT                        0
#       define R300_FIRST_TEX                           (1 << 3)
#       define R500_ZERO_TIMES_ANYTHING_EQUALS_ZERO     (1 << 1)
#define R300_US_PIXSIZE				        0x4604
#define R300_US_CODE_OFFSET				0x4608
#       define R300_ALU_CODE_OFFSET(x)                  ((x) << 0)
#       define R300_ALU_CODE_SIZE(x)                    ((x) << 6)
#       define R300_TEX_CODE_OFFSET(x)                  ((x) << 13)
#       define R300_TEX_CODE_SIZE(x)                    ((x) << 18)
#define R300_US_CODE_ADDR_0				0x4610
#       define R300_ALU_START(x)                        ((x) << 0)
#       define R300_ALU_SIZE(x)                         ((x) << 6)
#       define R300_TEX_START(x)                        ((x) << 12)
#       define R300_TEX_SIZE(x)                         ((x) << 17)
#       define R300_RGBA_OUT                            (1 << 22)
#       define R300_W_OUT                               (1 << 23)
#define R300_US_CODE_ADDR_1				0x4614
#define R300_US_CODE_ADDR_2				0x4618
#define R300_US_CODE_ADDR_3				0x461c
#define R300_US_TEX_INST_0				0x4620
#define R300_US_TEX_INST_1				0x4624
#define R300_US_TEX_INST_2				0x4628
#define R300_US_TEX_INST(x)			        (R300_US_TEX_INST_0 + (x)*4)
#       define R300_TEX_SRC_ADDR(x)                     ((x) << 0)
#       define R300_TEX_DST_ADDR(x)                     ((x) << 6)
#       define R300_TEX_ID(x)                           ((x) << 11)
#       define R300_TEX_INST(x)                         ((x) << 15)
#       define R300_TEX_INST_NOP                        0
#       define R300_TEX_INST_LD                         1
#       define R300_TEX_INST_TEXKILL                    2
#       define R300_TEX_INST_PROJ                       3
#       define R300_TEX_INST_LODBIAS                    4
#define R300_US_ALU_RGB_ADDR_0			        0x46c0
#define R300_US_ALU_RGB_ADDR_1			        0x46c4
#define R300_US_ALU_RGB_ADDR_2			        0x46c8
#define R300_US_ALU_RGB_ADDR(x)			        (R300_US_ALU_RGB_ADDR_0 + (x)*4)
/* for ADDR0-2, values 0-31 specify a location in the pixel stack,
   values 32-63 specify a constant */
#       define R300_ALU_RGB_ADDR0(x)                    ((x) << 0)
#       define R300_ALU_RGB_ADDR1(x)                    ((x) << 6)
#       define R300_ALU_RGB_ADDR2(x)                    ((x) << 12)
#       define R300_ALU_RGB_CONST(x)                    ((x) | (1 << 5))
/* ADDRD - where on the pixel stack the result of this instruction
   will be written */
#       define R300_ALU_RGB_ADDRD(x)                    ((x) << 18)
#       define R300_ALU_RGB_WMASK(x)                    ((x) << 23)
#       define R300_ALU_RGB_OMASK(x)                    ((x) << 26)
#       define R300_ALU_RGB_MASK_NONE                   0
#       define R300_ALU_RGB_MASK_R                      1
#       define R300_ALU_RGB_MASK_G                      2
#       define R300_ALU_RGB_MASK_B                      4
#       define R300_ALU_RGB_MASK_RGB                    7
#       define R300_ALU_RGB_TARGET_A                    (0 << 29)
#       define R300_ALU_RGB_TARGET_B                    (1 << 29)
#       define R300_ALU_RGB_TARGET_C                    (2 << 29)
#       define R300_ALU_RGB_TARGET_D                    (3 << 29)
#define R300_US_ALU_RGB_INST_0			        0x48c0
#define R300_US_ALU_RGB_INST_1			        0x48c4
#define R300_US_ALU_RGB_INST_2			        0x48c8
#define R300_US_ALU_RGB_INST(x)			        (R300_US_ALU_RGB_INST_0 + (x)*4)
#       define R300_ALU_RGB_SEL_A(x)                    ((x) << 0)
#       define R300_ALU_RGB_SRC0_RGB                    0
#       define R300_ALU_RGB_SRC0_RRR                    1
#       define R300_ALU_RGB_SRC0_GGG                    2
#       define R300_ALU_RGB_SRC0_BBB                    3
#       define R300_ALU_RGB_SRC1_RGB                    4
#       define R300_ALU_RGB_SRC1_RRR                    5
#       define R300_ALU_RGB_SRC1_GGG                    6
#       define R300_ALU_RGB_SRC1_BBB                    7
#       define R300_ALU_RGB_SRC2_RGB                    8
#       define R300_ALU_RGB_SRC2_RRR                    9
#       define R300_ALU_RGB_SRC2_GGG                    10
#       define R300_ALU_RGB_SRC2_BBB                    11
#       define R300_ALU_RGB_SRC0_AAA                    12
#       define R300_ALU_RGB_SRC1_AAA                    13
#       define R300_ALU_RGB_SRC2_AAA                    14
#       define R300_ALU_RGB_SRCP_RGB                    15
#       define R300_ALU_RGB_SRCP_RRR                    16
#       define R300_ALU_RGB_SRCP_GGG                    17
#       define R300_ALU_RGB_SRCP_BBB                    18
#       define R300_ALU_RGB_SRCP_AAA                    19
#       define R300_ALU_RGB_0_0                         20
#       define R300_ALU_RGB_1_0                         21
#       define R300_ALU_RGB_0_5                         22
#       define R300_ALU_RGB_SRC0_GBR                    23
#       define R300_ALU_RGB_SRC1_GBR                    24
#       define R300_ALU_RGB_SRC2_GBR                    25
#       define R300_ALU_RGB_SRC0_BRG                    26
#       define R300_ALU_RGB_SRC1_BRG                    27
#       define R300_ALU_RGB_SRC2_BRG                    28
#       define R300_ALU_RGB_SRC0_ABG                    29
#       define R300_ALU_RGB_SRC1_ABG                    30
#       define R300_ALU_RGB_SRC2_ABG                    31
#       define R300_ALU_RGB_MOD_A(x)                    ((x) << 5)
#       define R300_ALU_RGB_MOD_NOP                     0
#       define R300_ALU_RGB_MOD_NEG                     1
#       define R300_ALU_RGB_MOD_ABS                     2
#       define R300_ALU_RGB_MOD_NAB                     3
#       define R300_ALU_RGB_SEL_B(x)                    ((x) << 7)
#       define R300_ALU_RGB_MOD_B(x)                    ((x) << 12)
#       define R300_ALU_RGB_SEL_C(x)                    ((x) << 14)
#       define R300_ALU_RGB_MOD_C(x)                    ((x) << 19)
#       define R300_ALU_RGB_SRCP_OP(x)                  ((x) << 21)
#       define R300_ALU_RGB_SRCP_OP_1_MINUS_2RGB0	0
#       define R300_ALU_RGB_SRCP_OP_RGB1_MINUS_RGB0	1
#       define R300_ALU_RGB_SRCP_OP_RGB1_PLUS_RGB0	2
#       define R300_ALU_RGB_SRCP_OP_1_MINUS_RGB0	3
#       define R300_ALU_RGB_OP(x)                       ((x) << 23)
#       define R300_ALU_RGB_OP_MAD                      0
#       define R300_ALU_RGB_OP_DP3                      1
#       define R300_ALU_RGB_OP_DP4                      2
#       define R300_ALU_RGB_OP_D2A                      3
#       define R300_ALU_RGB_OP_MIN                      4
#       define R300_ALU_RGB_OP_MAX                      5
#       define R300_ALU_RGB_OP_CND                      7
#       define R300_ALU_RGB_OP_CMP                      8
#       define R300_ALU_RGB_OP_FRC                      9
#       define R300_ALU_RGB_OP_SOP                      10
#       define R300_ALU_RGB_OMOD(x)                     ((x) << 27)
#       define R300_ALU_RGB_OMOD_NONE                   0
#       define R300_ALU_RGB_OMOD_MUL_2                  1
#       define R300_ALU_RGB_OMOD_MUL_4                  2
#       define R300_ALU_RGB_OMOD_MUL_8                  3
#       define R300_ALU_RGB_OMOD_DIV_2                  4
#       define R300_ALU_RGB_OMOD_DIV_4                  5
#       define R300_ALU_RGB_OMOD_DIV_8                  6
#       define R300_ALU_RGB_CLAMP                       (1 << 30)
#       define R300_ALU_RGB_INSERT_NOP                  (1 << 31)
#define R300_US_ALU_ALPHA_ADDR_0		        0x47c0
#define R300_US_ALU_ALPHA_ADDR_1		        0x47c4
#define R300_US_ALU_ALPHA_ADDR_2		        0x47c8
#define R300_US_ALU_ALPHA_ADDR(x)		        (R300_US_ALU_ALPHA_ADDR_0 + (x)*4)
/* for ADDR0-2, values 0-31 specify a location in the pixel stack,
   values 32-63 specify a constant */
#       define R300_ALU_ALPHA_ADDR0(x)                  ((x) << 0)
#       define R300_ALU_ALPHA_ADDR1(x)                  ((x) << 6)
#       define R300_ALU_ALPHA_ADDR2(x)                  ((x) << 12)
#       define R300_ALU_ALPHA_CONST(x)                  ((x) | (1 << 5))
/* ADDRD - where on the pixel stack the result of this instruction
   will be written */
#       define R300_ALU_ALPHA_ADDRD(x)                  ((x) << 18)
#       define R300_ALU_ALPHA_WMASK(x)                  ((x) << 23)
#       define R300_ALU_ALPHA_OMASK(x)                  ((x) << 24)
#       define R300_ALU_ALPHA_OMASK_W(x)                ((x) << 27)
#       define R300_ALU_ALPHA_MASK_NONE                 0
#       define R300_ALU_ALPHA_MASK_A                    1
#       define R300_ALU_ALPHA_TARGET_A                  (0 << 25)
#       define R300_ALU_ALPHA_TARGET_B                  (1 << 25)
#       define R300_ALU_ALPHA_TARGET_C                  (2 << 25)
#       define R300_ALU_ALPHA_TARGET_D                  (3 << 25)
#define R300_US_ALU_ALPHA_INST_0		        0x49c0
#define R300_US_ALU_ALPHA_INST_1		        0x49c4
#define R300_US_ALU_ALPHA_INST_2		        0x49c8
#define R300_US_ALU_ALPHA_INST(x)		        (R300_US_ALU_ALPHA_INST_0 + (x)*4)
#       define R300_ALU_ALPHA_SEL_A(x)                  ((x) << 0)
#       define R300_ALU_ALPHA_SRC0_R                    0
#       define R300_ALU_ALPHA_SRC0_G                    1
#       define R300_ALU_ALPHA_SRC0_B                    2
#       define R300_ALU_ALPHA_SRC1_R                    3
#       define R300_ALU_ALPHA_SRC1_G                    4
#       define R300_ALU_ALPHA_SRC1_B                    5
#       define R300_ALU_ALPHA_SRC2_R                    6
#       define R300_ALU_ALPHA_SRC2_G                    7
#       define R300_ALU_ALPHA_SRC2_B                    8
#       define R300_ALU_ALPHA_SRC0_A                    9
#       define R300_ALU_ALPHA_SRC1_A                    10
#       define R300_ALU_ALPHA_SRC2_A                    11
#       define R300_ALU_ALPHA_SRCP_R                    12
#       define R300_ALU_ALPHA_SRCP_G                    13
#       define R300_ALU_ALPHA_SRCP_B                    14
#       define R300_ALU_ALPHA_SRCP_A                    15
#       define R300_ALU_ALPHA_0_0                       16
#       define R300_ALU_ALPHA_1_0                       17
#       define R300_ALU_ALPHA_0_5                       18
#       define R300_ALU_ALPHA_MOD_A(x)                  ((x) << 5)
#       define R300_ALU_ALPHA_MOD_NOP                   0
#       define R300_ALU_ALPHA_MOD_NEG                   1
#       define R300_ALU_ALPHA_MOD_ABS                   2
#       define R300_ALU_ALPHA_MOD_NAB                   3
#       define R300_ALU_ALPHA_SEL_B(x)                  ((x) << 7)
#       define R300_ALU_ALPHA_MOD_B(x)                  ((x) << 12)
#       define R300_ALU_ALPHA_SEL_C(x)                  ((x) << 14)
#       define R300_ALU_ALPHA_MOD_C(x)                  ((x) << 19)
#       define R300_ALU_ALPHA_SRCP_OP(x)                ((x) << 21)
#       define R300_ALU_ALPHA_SRCP_OP_1_MINUS_2RGB0	0
#       define R300_ALU_ALPHA_SRCP_OP_RGB1_MINUS_RGB0	1
#       define R300_ALU_ALPHA_SRCP_OP_RGB1_PLUS_RGB0	2
#       define R300_ALU_ALPHA_SRCP_OP_1_MINUS_RGB0	3
#       define R300_ALU_ALPHA_OP(x)                     ((x) << 23)
#       define R300_ALU_ALPHA_OP_MAD                    0
#       define R300_ALU_ALPHA_OP_DP                     1
#       define R300_ALU_ALPHA_OP_MIN                    2
#       define R300_ALU_ALPHA_OP_MAX                    3
#       define R300_ALU_ALPHA_OP_CND                    5
#       define R300_ALU_ALPHA_OP_CMP                    6
#       define R300_ALU_ALPHA_OP_FRC                    7
#       define R300_ALU_ALPHA_OP_EX2                    8
#       define R300_ALU_ALPHA_OP_LN2                    9
#       define R300_ALU_ALPHA_OP_RCP                    10
#       define R300_ALU_ALPHA_OP_RSQ                    11
#       define R300_ALU_ALPHA_OMOD(x)                   ((x) << 27)
#       define R300_ALU_ALPHA_OMOD_NONE                 0
#       define R300_ALU_ALPHA_OMOD_MUL_2                1
#       define R300_ALU_ALPHA_OMOD_MUL_4                2
#       define R300_ALU_ALPHA_OMOD_MUL_8                3
#       define R300_ALU_ALPHA_OMOD_DIV_2                4
#       define R300_ALU_ALPHA_OMOD_DIV_4                5
#       define R300_ALU_ALPHA_OMOD_DIV_8                6
#       define R300_ALU_ALPHA_CLAMP                     (1 << 30)

#define R300_US_ALU_CONST_R_0                           0x4c00
#define R300_US_ALU_CONST_R(x)                          (R300_US_ALU_CONST_R_0 + (x)*16)
#define R300_US_ALU_CONST_G_0                           0x4c04
#define R300_US_ALU_CONST_G(x)                          (R300_US_ALU_CONST_G_0 + (x)*16)
#define R300_US_ALU_CONST_B_0                           0x4c08
#define R300_US_ALU_CONST_B(x)                          (R300_US_ALU_CONST_B_0 + (x)*16)
#define R300_US_ALU_CONST_A_0                           0x4c0c
#define R300_US_ALU_CONST_A(x)                          (R300_US_ALU_CONST_A_0 + (x)*16)

#define R300_FG_DEPTH_SRC				0x4bd8
#define R300_FG_FOG_BLEND				0x4bc0
#define R300_FG_ALPHA_FUNC				0x4bd4

#define R300_DST_PIPE_CONFIG		                0x170c
#       define R300_PIPE_AUTO_CONFIG                    (1 << 31)
#define R300_RB2D_DSTCACHE_MODE		                0x3428
#define R300_RB2D_DSTCACHE_MODE		                0x3428
#       define R300_DC_AUTOFLUSH_ENABLE                 (1 << 8)
#       define R300_DC_DC_DISABLE_IGNORE_PE             (1 << 17)
#define R300_RB2D_DSTCACHE_CTLSTAT		        0x342c /* use DSTCACHE_CTLSTAT instead */
#define R300_DSTCACHE_CTLSTAT		                0x1714
#       define R300_DC_FLUSH_2D                         (1 << 0)
#       define R300_DC_FREE_2D                          (1 << 2)
#       define R300_RB2D_DC_FLUSH_ALL                   (R300_DC_FLUSH_2D | R300_DC_FREE_2D)
#       define R300_RB2D_DC_BUSY                        (1 << 31)
#define R300_RB3D_DSTCACHE_CTLSTAT		        0x4e4c
#       define R300_DC_FLUSH_3D                         (2 << 0)
#       define R300_DC_FREE_3D                          (2 << 2)
#       define R300_RB3D_DC_FLUSH_ALL                   (R300_DC_FLUSH_3D | R300_DC_FREE_3D)
#       define R300_DC_FINISH_3D                        (1 << 4)
#define R300_RB3D_ZCACHE_CTLSTAT			0x4f18
#       define R300_ZC_FLUSH                            (1 << 0)
#       define R300_ZC_FREE                             (1 << 1)
#       define R300_ZC_FLUSH_ALL                        0x3
#define R300_RB3D_ZSTENCILCNTL			        0x4f04
#define R300_RB3D_ZCACHE_CTLSTAT		        0x4f18
#define R300_RB3D_BW_CNTL				0x4f1c
#define R300_RB3D_ZCNTL				        0x4f00
#define R300_RB3D_ZTOP				        0x4f14
#define R300_RB3D_ROPCNTL				0x4e18
#define R300_RB3D_BLENDCNTL				0x4e04
#       define R300_ALPHA_BLEND_ENABLE                  (1 << 0)
#       define R300_SEPARATE_ALPHA_ENABLE               (1 << 1)
#       define R300_READ_ENABLE                         (1 << 2)
#define R300_RB3D_ABLENDCNTL			        0x4e08
#define R300_RB3D_DSTCACHE_CTLSTAT		        0x4e4c
#define R300_RB3D_COLOROFFSET0			        0x4e28
#define R300_RB3D_COLORPITCH0			        0x4e38
#       define R300_COLORTILE                           (1 << 16)
#       define R300_COLORENDIAN_WORD                    (1 << 19)
#       define R300_COLORENDIAN_DWORD                   (2 << 19)
#       define R300_COLORENDIAN_HALF_DWORD              (3 << 19)
#       define R300_COLORFORMAT_ARGB1555                (3 << 21)
#       define R300_COLORFORMAT_RGB565                  (4 << 21)
#       define R300_COLORFORMAT_ARGB8888                (6 << 21)
#       define R300_COLORFORMAT_ARGB32323232            (7 << 21)
#       define R300_COLORFORMAT_I8                      (9 << 21)
#       define R300_COLORFORMAT_ARGB16161616            (10 << 21)
#       define R300_COLORFORMAT_VYUY                    (11 << 21)
#       define R300_COLORFORMAT_YVYU                    (12 << 21)
#       define R300_COLORFORMAT_UV88                    (13 << 21)
#       define R300_COLORFORMAT_ARGB4444                (15 << 21)

#define R300_RB3D_AARESOLVE_CTL			        0x4e88
#define R300_RB3D_COLOR_CHANNEL_MASK	                0x4e0c
#       define R300_BLUE_MASK_EN                        (1 << 0)
#       define R300_GREEN_MASK_EN                       (1 << 1)
#       define R300_RED_MASK_EN                         (1 << 2)
#       define R300_ALPHA_MASK_EN                       (1 << 3)
#define R300_RB3D_COLOR_CLEAR_VALUE                     0x4e14
#define R300_RB3D_DSTCACHE_CTLSTAT		        0x4e4c
#define R300_RB3D_CCTL				        0x4e00
#define R300_RB3D_DITHER_CTL			        0x4e50

#define R300_SC_EDGERULE				0x43a8
#define R300_SC_SCISSOR0				0x43e0
#define R300_SC_SCISSOR1				0x43e4
#       define R300_SCISSOR_X_SHIFT                     0
#       define R300_SCISSOR_Y_SHIFT                     13
#define R300_SC_CLIP_0_A				0x43b0
#define R300_SC_CLIP_0_B				0x43b4
#       define R300_CLIP_X_SHIFT                        0
#       define R300_CLIP_Y_SHIFT                        13
#define R300_SC_CLIP_RULE				0x43d0
#define R300_SC_SCREENDOOR				0x43e8

/* R500 US has to be loaded through an index/data pair */
#define R500_GA_US_VECTOR_INDEX				0x4250
#   define R500_US_VECTOR_TYPE_INST			(0 << 16)
#   define R500_US_VECTOR_TYPE_CONST			(1 << 16)
#   define R500_US_VECTOR_CLAMP				(1 << 17)
#   define R500_US_VECTOR_INST_INDEX(x)			((x) | R500_US_VECTOR_TYPE_INST)
#   define R500_US_VECTOR_CONST_INDEX(x)		((x) | R500_US_VECTOR_TYPE_CONST)
#define R500_GA_US_VECTOR_DATA				0x4254

/*
 * The R500 unified shader (US) registers come in banks of 512 each, one
 * for each instruction slot in the shader.  You can't touch them directly.
 * R500_US_VECTOR_INDEX() sets the base instruction to modify; successive
 * writes to R500_GA_US_VECTOR_DATA autoincrement the index after the
 * instruction is fully specified.
 */
#define R500_US_ALU_ALPHA_INST_0			0xa800
#   define R500_ALPHA_OP_MAD				0
#   define R500_ALPHA_OP_DP				1
#   define R500_ALPHA_OP_MIN				2
#   define R500_ALPHA_OP_MAX				3
/* #define R500_ALPHA_OP_RESERVED			4 */
#   define R500_ALPHA_OP_CND				5
#   define R500_ALPHA_OP_CMP				6
#   define R500_ALPHA_OP_FRC				7
#   define R500_ALPHA_OP_EX2				8
#   define R500_ALPHA_OP_LN2				9
#   define R500_ALPHA_OP_RCP				10
#   define R500_ALPHA_OP_RSQ				11
#   define R500_ALPHA_OP_SIN				12
#   define R500_ALPHA_OP_COS				13
#   define R500_ALPHA_OP_MDH				14
#   define R500_ALPHA_OP_MDV				15
#   define R500_ALPHA_ADDRD(x)				((x) << 4)
#   define R500_ALPHA_ADDRD_REL				(1 << 11)
#   define R500_ALPHA_SEL_A_SRC0			(0 << 12)
#   define R500_ALPHA_SEL_A_SRC1			(1 << 12)
#   define R500_ALPHA_SEL_A_SRC2			(2 << 12)
#   define R500_ALPHA_SEL_A_SRCP			(3 << 12)
#   define R500_ALPHA_SWIZ_A_R				(0 << 14)
#   define R500_ALPHA_SWIZ_A_G				(1 << 14)
#   define R500_ALPHA_SWIZ_A_B				(2 << 14)
#   define R500_ALPHA_SWIZ_A_A				(3 << 14)
#   define R500_ALPHA_SWIZ_A_0				(4 << 14)
#   define R500_ALPHA_SWIZ_A_HALF			(5 << 14)
#   define R500_ALPHA_SWIZ_A_1				(6 << 14)
/* #define R500_ALPHA_SWIZ_A_UNUSED			(7 << 14) */
#   define R500_ALPHA_MOD_A_NOP				(0 << 17)
#   define R500_ALPHA_MOD_A_NEG				(1 << 17)
#   define R500_ALPHA_MOD_A_ABS				(2 << 17)
#   define R500_ALPHA_MOD_A_NAB				(3 << 17)
#   define R500_ALPHA_SEL_B_SRC0			(0 << 19)
#   define R500_ALPHA_SEL_B_SRC1			(1 << 19)
#   define R500_ALPHA_SEL_B_SRC2			(2 << 19)
#   define R500_ALPHA_SEL_B_SRCP			(3 << 19)
#   define R500_ALPHA_SWIZ_B_R				(0 << 21)
#   define R500_ALPHA_SWIZ_B_G				(1 << 21)
#   define R500_ALPHA_SWIZ_B_B				(2 << 21)
#   define R500_ALPHA_SWIZ_B_A				(3 << 21)
#   define R500_ALPHA_SWIZ_B_0				(4 << 21)
#   define R500_ALPHA_SWIZ_B_HALF			(5 << 21)
#   define R500_ALPHA_SWIZ_B_1				(6 << 21)
/* #define R500_ALPHA_SWIZ_B_UNUSED			(7 << 21) */
#   define R500_ALPHA_MOD_B_NOP				(0 << 24)
#   define R500_ALPHA_MOD_B_NEG				(1 << 24)
#   define R500_ALPHA_MOD_B_ABS				(2 << 24)
#   define R500_ALPHA_MOD_B_NAB				(3 << 24)
#   define R500_ALPHA_OMOD_IDENTITY			(0 << 26)
#   define R500_ALPHA_OMOD_MUL_2			(1 << 26)
#   define R500_ALPHA_OMOD_MUL_4			(2 << 26)
#   define R500_ALPHA_OMOD_MUL_8			(3 << 26)
#   define R500_ALPHA_OMOD_DIV_2			(4 << 26)
#   define R500_ALPHA_OMOD_DIV_4			(5 << 26)
#   define R500_ALPHA_OMOD_DIV_8			(6 << 26)
#   define R500_ALPHA_OMOD_DISABLE			(7 << 26)
#   define R500_ALPHA_TARGET(x)				((x) << 29)
#   define R500_ALPHA_W_OMASK				(1 << 31)
#define R500_US_ALU_ALPHA_ADDR_0			0x9800
#   define R500_ALPHA_ADDR0(x)				((x) << 0)
#   define R500_ALPHA_ADDR0_CONST			(1 << 8)
#   define R500_ALPHA_ADDR0_REL				(1 << 9)
#   define R500_ALPHA_ADDR1(x)				((x) << 10)
#   define R500_ALPHA_ADDR1_CONST			(1 << 18)
#   define R500_ALPHA_ADDR1_REL				(1 << 19)
#   define R500_ALPHA_ADDR2(x)				((x) << 20)
#   define R500_ALPHA_ADDR2_CONST			(1 << 28)
#   define R500_ALPHA_ADDR2_REL				(1 << 29)
#   define R500_ALPHA_SRCP_OP_1_MINUS_2A0		(0 << 30)
#   define R500_ALPHA_SRCP_OP_A1_MINUS_A0		(1 << 30)
#   define R500_ALPHA_SRCP_OP_A1_PLUS_A0		(2 << 30)
#   define R500_ALPHA_SRCP_OP_1_MINUS_A0		(3 << 30)
#define R500_US_ALU_RGBA_INST_0				0xb000
#   define R500_ALU_RGBA_OP_MAD				(0 << 0)
#   define R500_ALU_RGBA_OP_DP3				(1 << 0)
#   define R500_ALU_RGBA_OP_DP4				(2 << 0)
#   define R500_ALU_RGBA_OP_D2A				(3 << 0)
#   define R500_ALU_RGBA_OP_MIN				(4 << 0)
#   define R500_ALU_RGBA_OP_MAX				(5 << 0)
/* #define R500_ALU_RGBA_OP_RESERVED			(6 << 0) */
#   define R500_ALU_RGBA_OP_CND				(7 << 0)
#   define R500_ALU_RGBA_OP_CMP				(8 << 0)
#   define R500_ALU_RGBA_OP_FRC				(9 << 0)
#   define R500_ALU_RGBA_OP_SOP				(10 << 0)
#   define R500_ALU_RGBA_OP_MDH				(11 << 0)
#   define R500_ALU_RGBA_OP_MDV				(12 << 0)
#   define R500_ALU_RGBA_ADDRD(x)			((x) << 4)
#   define R500_ALU_RGBA_ADDRD_REL			(1 << 11)
#   define R500_ALU_RGBA_SEL_C_SRC0			(0 << 12)
#   define R500_ALU_RGBA_SEL_C_SRC1			(1 << 12)
#   define R500_ALU_RGBA_SEL_C_SRC2			(2 << 12)
#   define R500_ALU_RGBA_SEL_C_SRCP			(3 << 12)
#   define R500_ALU_RGBA_R_SWIZ_R			(0 << 14)
#   define R500_ALU_RGBA_R_SWIZ_G			(1 << 14)
#   define R500_ALU_RGBA_R_SWIZ_B			(2 << 14)
#   define R500_ALU_RGBA_R_SWIZ_A			(3 << 14)
#   define R500_ALU_RGBA_R_SWIZ_0			(4 << 14)
#   define R500_ALU_RGBA_R_SWIZ_HALF			(5 << 14)
#   define R500_ALU_RGBA_R_SWIZ_1			(6 << 14)
/* #define R500_ALU_RGBA_R_SWIZ_UNUSED			(7 << 14) */
#   define R500_ALU_RGBA_G_SWIZ_R			(0 << 17)
#   define R500_ALU_RGBA_G_SWIZ_G			(1 << 17)
#   define R500_ALU_RGBA_G_SWIZ_B			(2 << 17)
#   define R500_ALU_RGBA_G_SWIZ_A			(3 << 17)
#   define R500_ALU_RGBA_G_SWIZ_0			(4 << 17)
#   define R500_ALU_RGBA_G_SWIZ_HALF			(5 << 17)
#   define R500_ALU_RGBA_G_SWIZ_1			(6 << 17)
/* #define R500_ALU_RGBA_G_SWIZ_UNUSED			(7 << 17) */
#   define R500_ALU_RGBA_B_SWIZ_R			(0 << 20)
#   define R500_ALU_RGBA_B_SWIZ_G			(1 << 20)
#   define R500_ALU_RGBA_B_SWIZ_B			(2 << 20)
#   define R500_ALU_RGBA_B_SWIZ_A			(3 << 20)
#   define R500_ALU_RGBA_B_SWIZ_0			(4 << 20)
#   define R500_ALU_RGBA_B_SWIZ_HALF			(5 << 20)
#   define R500_ALU_RGBA_B_SWIZ_1			(6 << 20)
/* #define R500_ALU_RGBA_B_SWIZ_UNUSED			(7 << 20) */
#   define R500_ALU_RGBA_MOD_C_NOP			(0 << 23)
#   define R500_ALU_RGBA_MOD_C_NEG			(1 << 23)
#   define R500_ALU_RGBA_MOD_C_ABS			(2 << 23)
#   define R500_ALU_RGBA_MOD_C_NAB			(3 << 23)
#   define R500_ALU_RGBA_ALPHA_SEL_C_SRC0		(0 << 25)
#   define R500_ALU_RGBA_ALPHA_SEL_C_SRC1		(1 << 25)
#   define R500_ALU_RGBA_ALPHA_SEL_C_SRC2		(2 << 25)
#   define R500_ALU_RGBA_ALPHA_SEL_C_SRCP		(3 << 25)
#   define R500_ALU_RGBA_A_SWIZ_R			(0 << 27)
#   define R500_ALU_RGBA_A_SWIZ_G			(1 << 27)
#   define R500_ALU_RGBA_A_SWIZ_B			(2 << 27)
#   define R500_ALU_RGBA_A_SWIZ_A			(3 << 27)
#   define R500_ALU_RGBA_A_SWIZ_0			(4 << 27)
#   define R500_ALU_RGBA_A_SWIZ_HALF			(5 << 27)
#   define R500_ALU_RGBA_A_SWIZ_1			(6 << 27)
/* #define R500_ALU_RGBA_A_SWIZ_UNUSED			(7 << 27) */
#   define R500_ALU_RGBA_ALPHA_MOD_C_NOP		(0 << 30)
#   define R500_ALU_RGBA_ALPHA_MOD_C_NEG		(1 << 30)
#   define R500_ALU_RGBA_ALPHA_MOD_C_ABS		(2 << 30)
#   define R500_ALU_RGBA_ALPHA_MOD_C_NAB		(3 << 30)
#define R500_US_ALU_RGB_INST_0				0xa000
#   define R500_ALU_RGB_SEL_A_SRC0			(0 << 0)
#   define R500_ALU_RGB_SEL_A_SRC1			(1 << 0)
#   define R500_ALU_RGB_SEL_A_SRC2			(2 << 0)
#   define R500_ALU_RGB_SEL_A_SRCP			(3 << 0)
#   define R500_ALU_RGB_R_SWIZ_A_R			(0 << 2)
#   define R500_ALU_RGB_R_SWIZ_A_G			(1 << 2)
#   define R500_ALU_RGB_R_SWIZ_A_B			(2 << 2)
#   define R500_ALU_RGB_R_SWIZ_A_A			(3 << 2)
#   define R500_ALU_RGB_R_SWIZ_A_0			(4 << 2)
#   define R500_ALU_RGB_R_SWIZ_A_HALF			(5 << 2)
#   define R500_ALU_RGB_R_SWIZ_A_1			(6 << 2)
/* #define R500_ALU_RGB_R_SWIZ_A_UNUSED			(7 << 2) */
#   define R500_ALU_RGB_G_SWIZ_A_R			(0 << 5)
#   define R500_ALU_RGB_G_SWIZ_A_G			(1 << 5)
#   define R500_ALU_RGB_G_SWIZ_A_B			(2 << 5)
#   define R500_ALU_RGB_G_SWIZ_A_A			(3 << 5)
#   define R500_ALU_RGB_G_SWIZ_A_0			(4 << 5)
#   define R500_ALU_RGB_G_SWIZ_A_HALF			(5 << 5)
#   define R500_ALU_RGB_G_SWIZ_A_1			(6 << 5)
/* #define R500_ALU_RGB_G_SWIZ_A_UNUSED			(7 << 5) */
#   define R500_ALU_RGB_B_SWIZ_A_R			(0 << 8)
#   define R500_ALU_RGB_B_SWIZ_A_G			(1 << 8)
#   define R500_ALU_RGB_B_SWIZ_A_B			(2 << 8)
#   define R500_ALU_RGB_B_SWIZ_A_A			(3 << 8)
#   define R500_ALU_RGB_B_SWIZ_A_0			(4 << 8)
#   define R500_ALU_RGB_B_SWIZ_A_HALF			(5 << 8)
#   define R500_ALU_RGB_B_SWIZ_A_1			(6 << 8)
/* #define R500_ALU_RGB_B_SWIZ_A_UNUSED			(7 << 8) */
#   define R500_ALU_RGB_MOD_A_NOP			(0 << 11)
#   define R500_ALU_RGB_MOD_A_NEG			(1 << 11)
#   define R500_ALU_RGB_MOD_A_ABS			(2 << 11)
#   define R500_ALU_RGB_MOD_A_NAB			(3 << 11)
#   define R500_ALU_RGB_SEL_B_SRC0			(0 << 13)
#   define R500_ALU_RGB_SEL_B_SRC1			(1 << 13)
#   define R500_ALU_RGB_SEL_B_SRC2			(2 << 13)
#   define R500_ALU_RGB_SEL_B_SRCP			(3 << 13)
#   define R500_ALU_RGB_R_SWIZ_B_R			(0 << 15)
#   define R500_ALU_RGB_R_SWIZ_B_G			(1 << 15)
#   define R500_ALU_RGB_R_SWIZ_B_B			(2 << 15)
#   define R500_ALU_RGB_R_SWIZ_B_A			(3 << 15)
#   define R500_ALU_RGB_R_SWIZ_B_0			(4 << 15)
#   define R500_ALU_RGB_R_SWIZ_B_HALF			(5 << 15)
#   define R500_ALU_RGB_R_SWIZ_B_1			(6 << 15)
/* #define R500_ALU_RGB_R_SWIZ_B_UNUSED			(7 << 15) */
#   define R500_ALU_RGB_G_SWIZ_B_R			(0 << 18)
#   define R500_ALU_RGB_G_SWIZ_B_G			(1 << 18)
#   define R500_ALU_RGB_G_SWIZ_B_B			(2 << 18)
#   define R500_ALU_RGB_G_SWIZ_B_A			(3 << 18)
#   define R500_ALU_RGB_G_SWIZ_B_0			(4 << 18)
#   define R500_ALU_RGB_G_SWIZ_B_HALF			(5 << 18)
#   define R500_ALU_RGB_G_SWIZ_B_1			(6 << 18)
/* #define R500_ALU_RGB_G_SWIZ_B_UNUSED			(7 << 18) */
#   define R500_ALU_RGB_B_SWIZ_B_R			(0 << 21)
#   define R500_ALU_RGB_B_SWIZ_B_G			(1 << 21)
#   define R500_ALU_RGB_B_SWIZ_B_B			(2 << 21)
#   define R500_ALU_RGB_B_SWIZ_B_A			(3 << 21)
#   define R500_ALU_RGB_B_SWIZ_B_0			(4 << 21)
#   define R500_ALU_RGB_B_SWIZ_B_HALF			(5 << 21)
#   define R500_ALU_RGB_B_SWIZ_B_1			(6 << 21)
/* #define R500_ALU_RGB_B_SWIZ_B_UNUSED			(7 << 21) */
#   define R500_ALU_RGB_MOD_B_NOP			(0 << 24)
#   define R500_ALU_RGB_MOD_B_NEG			(1 << 24)
#   define R500_ALU_RGB_MOD_B_ABS			(2 << 24)
#   define R500_ALU_RGB_MOD_B_NAB			(3 << 24)
#   define R500_ALU_RGB_OMOD_IDENTITY			(0 << 26)
#   define R500_ALU_RGB_OMOD_MUL_2			(1 << 26)
#   define R500_ALU_RGB_OMOD_MUL_4			(2 << 26)
#   define R500_ALU_RGB_OMOD_MUL_8			(3 << 26)
#   define R500_ALU_RGB_OMOD_DIV_2			(4 << 26)
#   define R500_ALU_RGB_OMOD_DIV_4			(5 << 26)
#   define R500_ALU_RGB_OMOD_DIV_8			(6 << 26)
#   define R500_ALU_RGB_OMOD_DISABLE			(7 << 26)
#   define R500_ALU_RGB_TARGET(x)			((x) << 29)
#   define R500_ALU_RGB_WMASK				(1 << 31)
#define R500_US_ALU_RGB_ADDR_0				0x9000
#   define R500_RGB_ADDR0(x)				((x) << 0)
#   define R500_RGB_ADDR0_CONST				(1 << 8)
#   define R500_RGB_ADDR0_REL				(1 << 9)
#   define R500_RGB_ADDR1(x)				((x) << 10)
#   define R500_RGB_ADDR1_CONST				(1 << 18)
#   define R500_RGB_ADDR1_REL				(1 << 19)
#   define R500_RGB_ADDR2(x)				((x) << 20)
#   define R500_RGB_ADDR2_CONST				(1 << 28)
#   define R500_RGB_ADDR2_REL				(1 << 29)
#   define R500_RGB_SRCP_OP_1_MINUS_2RGB0		(0 << 30)
#   define R500_RGB_SRCP_OP_RGB1_MINUS_RGB0		(1 << 30)
#   define R500_RGB_SRCP_OP_RGB1_PLUS_RGB0		(2 << 30)
#   define R500_RGB_SRCP_OP_1_MINUS_RGB0		(3 << 30)
#define R500_US_CMN_INST_0				0xb800
#   define R500_INST_TYPE_ALU				(0 << 0)
#   define R500_INST_TYPE_OUT				(1 << 0)
#   define R500_INST_TYPE_FC				(2 << 0)
#   define R500_INST_TYPE_TEX				(3 << 0)
#   define R500_INST_TEX_SEM_WAIT			(1 << 2)
#   define R500_INST_RGB_PRED_SEL_NONE			(0 << 3)
#   define R500_INST_RGB_PRED_SEL_RGBA			(1 << 3)
#   define R500_INST_RGB_PRED_SEL_RRRR			(2 << 3)
#   define R500_INST_RGB_PRED_SEL_GGGG			(3 << 3)
#   define R500_INST_RGB_PRED_SEL_BBBB			(4 << 3)
#   define R500_INST_RGB_PRED_SEL_AAAA			(5 << 3)
#   define R500_INST_RGB_PRED_INV			(1 << 6)
#   define R500_INST_WRITE_INACTIVE			(1 << 7)
#   define R500_INST_LAST				(1 << 8)
#   define R500_INST_NOP				(1 << 9)
#   define R500_INST_ALU_WAIT				(1 << 10)
#   define R500_INST_RGB_WMASK_R			(1 << 11)
#   define R500_INST_RGB_WMASK_G			(1 << 12)
#   define R500_INST_RGB_WMASK_B			(1 << 13)
#   define R500_INST_ALPHA_WMASK			(1 << 14)
#   define R500_INST_RGB_OMASK_R			(1 << 15)
#   define R500_INST_RGB_OMASK_G			(1 << 16)
#   define R500_INST_RGB_OMASK_B			(1 << 17)
#   define R500_INST_ALPHA_OMASK			(1 << 18)
#   define R500_INST_RGB_CLAMP				(1 << 19)
#   define R500_INST_ALPHA_CLAMP			(1 << 20)
#   define R500_INST_ALU_RESULT_SEL			(1 << 21)
#   define R500_INST_ALPHA_PRED_INV			(1 << 22)
#   define R500_INST_ALU_RESULT_OP_EQ			(0 << 23)
#   define R500_INST_ALU_RESULT_OP_LT			(1 << 23)
#   define R500_INST_ALU_RESULT_OP_GE			(2 << 23)
#   define R500_INST_ALU_RESULT_OP_NE			(3 << 23)
#   define R500_INST_ALPHA_PRED_SEL_NONE		(0 << 25)
#   define R500_INST_ALPHA_PRED_SEL_RGBA		(1 << 25)
#   define R500_INST_ALPHA_PRED_SEL_RRRR		(2 << 25)
#   define R500_INST_ALPHA_PRED_SEL_GGGG		(3 << 25)
#   define R500_INST_ALPHA_PRED_SEL_BBBB		(4 << 25)
#   define R500_INST_ALPHA_PRED_SEL_AAAA		(5 << 25)
/* XXX next four are kind of guessed */
#   define R500_INST_STAT_WE_R				(1 << 28)
#   define R500_INST_STAT_WE_G				(1 << 29)
#   define R500_INST_STAT_WE_B				(1 << 30)
#   define R500_INST_STAT_WE_A				(1 << 31)
/* note that these are 8 bit lengths, despite the offsets, at least for R500 */
#define R500_US_CODE_ADDR				0x4630
#   define R500_US_CODE_START_ADDR(x)			((x) << 0)
#   define R500_US_CODE_END_ADDR(x)			((x) << 16)
#define R500_US_CODE_OFFSET				0x4638
#   define R500_US_CODE_OFFSET_ADDR(x)			((x) << 0)
#define R500_US_CODE_RANGE				0x4634
#   define R500_US_CODE_RANGE_ADDR(x)			((x) << 0)
#   define R500_US_CODE_RANGE_SIZE(x)			((x) << 16)
#define R500_US_CONFIG					0x4600
#   define R500_ZERO_TIMES_ANYTHING_EQUALS_ZERO		(1 << 1)
#define R500_US_FC_ADDR_0				0xa000
#   define R500_FC_BOOL_ADDR(x)				((x) << 0)
#   define R500_FC_INT_ADDR(x)				((x) << 8)
#   define R500_FC_JUMP_ADDR(x)				((x) << 16)
#   define R500_FC_JUMP_GLOBAL				(1 << 31)
#define R500_US_FC_BOOL_CONST				0x4620
#   define R500_FC_KBOOL(x)				(x)
#define R500_US_FC_CTRL					0x4624
#   define R500_FC_TEST_EN				(1 << 30)
#   define R500_FC_FULL_FC_EN				(1 << 31)
#define R500_US_FC_INST_0				0x9800
#   define R500_FC_OP_JUMP				(0 << 0)
#   define R500_FC_OP_LOOP				(1 << 0)
#   define R500_FC_OP_ENDLOOP				(2 << 0)
#   define R500_FC_OP_REP				(3 << 0)
#   define R500_FC_OP_ENDREP				(4 << 0)
#   define R500_FC_OP_BREAKLOOP				(5 << 0)
#   define R500_FC_OP_BREAKREP				(6 << 0)
#   define R500_FC_OP_CONTINUE				(7 << 0)
#   define R500_FC_B_ELSE				(1 << 4)
#   define R500_FC_JUMP_ANY				(1 << 5)
#   define R500_FC_A_OP_NONE				(0 << 6)
#   define R500_FC_A_OP_POP				(1 << 6)
#   define R500_FC_A_OP_PUSH				(2 << 6)
#   define R500_FC_JUMP_FUNC(x)				((x) << 8)
#   define R500_FC_B_POP_CNT(x)				((x) << 16)
#   define R500_FC_B_OP0_NONE				(0 << 24)
#   define R500_FC_B_OP0_DECR				(1 << 24)
#   define R500_FC_B_OP0_INCR				(2 << 24)
#   define R500_FC_B_OP1_DECR				(0 << 26)
#   define R500_FC_B_OP1_NONE				(1 << 26)
#   define R500_FC_B_OP1_INCR				(2 << 26)
#   define R500_FC_IGNORE_UNCOVERED			(1 << 28)
#define R500_US_FC_INT_CONST_0				0x4c00
#   define R500_FC_INT_CONST_KR(x)			((x) << 0)
#   define R500_FC_INT_CONST_KG(x)			((x) << 8)
#   define R500_FC_INT_CONST_KB(x)			((x) << 16)
/* _0 through _15 */
#define R500_US_FORMAT0_0				0x4640
#   define R500_FORMAT_TXWIDTH(x)			((x) << 0)
#   define R500_FORMAT_TXHEIGHT(x)			((x) << 11)
#   define R500_FORMAT_TXDEPTH(x)			((x) << 22)
/* _0 through _3 */
#define R500_US_OUT_FMT_0				0x46a4
#   define R500_OUT_FMT_C4_8				(0 << 0)
#   define R500_OUT_FMT_C4_10				(1 << 0)
#   define R500_OUT_FMT_C4_10_GAMMA			(2 << 0)
#   define R500_OUT_FMT_C_16				(3 << 0)
#   define R500_OUT_FMT_C2_16				(4 << 0)
#   define R500_OUT_FMT_C4_16				(5 << 0)
#   define R500_OUT_FMT_C_16_MPEG			(6 << 0)
#   define R500_OUT_FMT_C2_16_MPEG			(7 << 0)
#   define R500_OUT_FMT_C2_4				(8 << 0)
#   define R500_OUT_FMT_C_3_3_2				(9 << 0)
#   define R500_OUT_FMT_C_6_5_6				(10 << 0)
#   define R500_OUT_FMT_C_11_11_10			(11 << 0)
#   define R500_OUT_FMT_C_10_11_11			(12 << 0)
#   define R500_OUT_FMT_C_2_10_10_10			(13 << 0)
/* #define R500_OUT_FMT_RESERVED			(14 << 0) */
#   define R500_OUT_FMT_UNUSED				(15 << 0)
#   define R500_OUT_FMT_C_16_FP				(16 << 0)
#   define R500_OUT_FMT_C2_16_FP			(17 << 0)
#   define R500_OUT_FMT_C4_16_FP			(18 << 0)
#   define R500_OUT_FMT_C_32_FP				(19 << 0)
#   define R500_OUT_FMT_C2_32_FP			(20 << 0)
#   define R500_OUT_FMT_C4_32_FP			(21 << 0)
#   define R500_C0_SEL_A				(0 << 8)
#   define R500_C0_SEL_R				(1 << 8)
#   define R500_C0_SEL_G				(2 << 8)
#   define R500_C0_SEL_B				(3 << 8)
#   define R500_C1_SEL_A				(0 << 10)
#   define R500_C1_SEL_R				(1 << 10)
#   define R500_C1_SEL_G				(2 << 10)
#   define R500_C1_SEL_B				(3 << 10)
#   define R500_C2_SEL_A				(0 << 12)
#   define R500_C2_SEL_R				(1 << 12)
#   define R500_C2_SEL_G				(2 << 12)
#   define R500_C2_SEL_B				(3 << 12)
#   define R500_C3_SEL_A				(0 << 14)
#   define R500_C3_SEL_R				(1 << 14)
#   define R500_C3_SEL_G				(2 << 14)
#   define R500_C3_SEL_B				(3 << 14)
#   define R500_OUT_SIGN(x)				((x) << 16)
#   define R500_ROUND_ADJ				(1 << 20)
#define R500_US_PIXSIZE					0x4604
#   define R500_PIX_SIZE(x)				(x)
#define R500_US_TEX_ADDR_0				0x9800
#   define R500_TEX_SRC_ADDR(x)				((x) << 0)
#   define R500_TEX_SRC_ADDR_REL			(1 << 7)
#   define R500_TEX_SRC_S_SWIZ_R			(0 << 8)
#   define R500_TEX_SRC_S_SWIZ_G			(1 << 8)
#   define R500_TEX_SRC_S_SWIZ_B			(2 << 8)
#   define R500_TEX_SRC_S_SWIZ_A			(3 << 8)
#   define R500_TEX_SRC_T_SWIZ_R			(0 << 10)
#   define R500_TEX_SRC_T_SWIZ_G			(1 << 10)
#   define R500_TEX_SRC_T_SWIZ_B			(2 << 10)
#   define R500_TEX_SRC_T_SWIZ_A			(3 << 10)
#   define R500_TEX_SRC_R_SWIZ_R			(0 << 12)
#   define R500_TEX_SRC_R_SWIZ_G			(1 << 12)
#   define R500_TEX_SRC_R_SWIZ_B			(2 << 12)
#   define R500_TEX_SRC_R_SWIZ_A			(3 << 12)
#   define R500_TEX_SRC_Q_SWIZ_R			(0 << 14)
#   define R500_TEX_SRC_Q_SWIZ_G			(1 << 14)
#   define R500_TEX_SRC_Q_SWIZ_B			(2 << 14)
#   define R500_TEX_SRC_Q_SWIZ_A			(3 << 14)
#   define R500_TEX_DST_ADDR(x)				((x) << 16)
#   define R500_TEX_DST_ADDR_REL			(1 << 23)
#   define R500_TEX_DST_R_SWIZ_R			(0 << 24)
#   define R500_TEX_DST_R_SWIZ_G			(1 << 24)
#   define R500_TEX_DST_R_SWIZ_B			(2 << 24)
#   define R500_TEX_DST_R_SWIZ_A			(3 << 24)
#   define R500_TEX_DST_G_SWIZ_R			(0 << 26)
#   define R500_TEX_DST_G_SWIZ_G			(1 << 26)
#   define R500_TEX_DST_G_SWIZ_B			(2 << 26)
#   define R500_TEX_DST_G_SWIZ_A			(3 << 26)
#   define R500_TEX_DST_B_SWIZ_R			(0 << 28)
#   define R500_TEX_DST_B_SWIZ_G			(1 << 28)
#   define R500_TEX_DST_B_SWIZ_B			(2 << 28)
#   define R500_TEX_DST_B_SWIZ_A			(3 << 28)
#   define R500_TEX_DST_A_SWIZ_R			(0 << 30)
#   define R500_TEX_DST_A_SWIZ_G			(1 << 30)
#   define R500_TEX_DST_A_SWIZ_B			(2 << 30)
#   define R500_TEX_DST_A_SWIZ_A			(3 << 30)
#define R500_US_TEX_ADDR_DXDY_0				0xa000
#   define R500_DX_ADDR(x)				((x) << 0)
#   define R500_DX_ADDR_REL				(1 << 7)
#   define R500_DX_S_SWIZ_R				(0 << 8)
#   define R500_DX_S_SWIZ_G				(1 << 8)
#   define R500_DX_S_SWIZ_B				(2 << 8)
#   define R500_DX_S_SWIZ_A				(3 << 8)
#   define R500_DX_T_SWIZ_R				(0 << 10)
#   define R500_DX_T_SWIZ_G				(1 << 10)
#   define R500_DX_T_SWIZ_B				(2 << 10)
#   define R500_DX_T_SWIZ_A				(3 << 10)
#   define R500_DX_R_SWIZ_R				(0 << 12)
#   define R500_DX_R_SWIZ_G				(1 << 12)
#   define R500_DX_R_SWIZ_B				(2 << 12)
#   define R500_DX_R_SWIZ_A				(3 << 12)
#   define R500_DX_Q_SWIZ_R				(0 << 14)
#   define R500_DX_Q_SWIZ_G				(1 << 14)
#   define R500_DX_Q_SWIZ_B				(2 << 14)
#   define R500_DX_Q_SWIZ_A				(3 << 14)
#   define R500_DY_ADDR(x)				((x) << 16)
#   define R500_DY_ADDR_REL				(1 << 17)
#   define R500_DY_S_SWIZ_R				(0 << 24)
#   define R500_DY_S_SWIZ_G				(1 << 24)
#   define R500_DY_S_SWIZ_B				(2 << 24)
#   define R500_DY_S_SWIZ_A				(3 << 24)
#   define R500_DY_T_SWIZ_R				(0 << 26)
#   define R500_DY_T_SWIZ_G				(1 << 26)
#   define R500_DY_T_SWIZ_B				(2 << 26)
#   define R500_DY_T_SWIZ_A				(3 << 26)
#   define R500_DY_R_SWIZ_R				(0 << 28)
#   define R500_DY_R_SWIZ_G				(1 << 28)
#   define R500_DY_R_SWIZ_B				(2 << 28)
#   define R500_DY_R_SWIZ_A				(3 << 28)
#   define R500_DY_Q_SWIZ_R				(0 << 30)
#   define R500_DY_Q_SWIZ_G				(1 << 30)
#   define R500_DY_Q_SWIZ_B				(2 << 30)
#   define R500_DY_Q_SWIZ_A				(3 << 30)
#define R500_US_TEX_INST_0				0x9000
#   define R500_TEX_ID(x)				((x) << 16)
#   define R500_TEX_INST_NOP				(0 << 22)
#   define R500_TEX_INST_LD				(1 << 22)
#   define R500_TEX_INST_TEXKILL			(2 << 22)
#   define R500_TEX_INST_PROJ				(3 << 22)
#   define R500_TEX_INST_LODBIAS			(4 << 22)
#   define R500_TEX_INST_LOD				(5 << 22)
#   define R500_TEX_INST_DXDY				(6 << 22)
#   define R500_TEX_SEM_ACQUIRE				(1 << 25)
#   define R500_TEX_IGNORE_UNCOVERED			(1 << 26)
#   define R500_TEX_UNSCALED				(1 << 27)
#define R500_US_W_FMT					0x46b4
#   define R500_W_FMT_W0				(0 << 0)
#   define R500_W_FMT_W24				(1 << 0)
#   define R500_W_FMT_W24FP				(2 << 0)
#   define R500_W_SRC_US				(0 << 2)
#   define R500_W_SRC_RAS				(1 << 2)

#define R500_RS_INST_0					0x4320
#define R500_RS_INST_1					0x4324
#   define R500_RS_INST_TEX_ID_SHIFT			0
#   define R500_RS_INST_TEX_CN_WRITE			(1 << 4)
#   define R500_RS_INST_TEX_ADDR_SHIFT			5
#   define R500_RS_INST_COL_ID_SHIFT			12
#   define R500_RS_INST_COL_CN_NO_WRITE			(0 << 16)
#   define R500_RS_INST_COL_CN_WRITE			(1 << 16)
#   define R500_RS_INST_COL_CN_WRITE_FBUFFER		(2 << 16)
#   define R500_RS_INST_COL_CN_WRITE_BACKFACE		(3 << 16)
#   define R500_RS_INST_COL_COL_ADDR_SHIFT		18
#   define R500_RS_INST_TEX_ADJ				(1 << 25)
#   define R500_RS_INST_W_CN				(1 << 26)

#define R500_US_FC_CTRL					0x4624
#define R500_US_CODE_ADDR				0x4630
#define R500_US_CODE_RANGE 				0x4634
#define R500_US_CODE_OFFSET 				0x4638

#define R500_RS_IP_0					0x4074
#define R500_RS_IP_1					0x4078
#   define R500_RS_IP_PTR_K0				62
#   define R500_RS_IP_PTR_K1 				63
#   define R500_RS_IP_TEX_PTR_S_SHIFT 			0
#   define R500_RS_IP_TEX_PTR_T_SHIFT 			6
#   define R500_RS_IP_TEX_PTR_R_SHIFT 			12
#   define R500_RS_IP_TEX_PTR_Q_SHIFT 			18
#   define R500_RS_IP_COL_PTR_SHIFT 			24
#   define R500_RS_IP_COL_FMT_SHIFT 			27
#   define R500_RS_IP_COL_FMT_RGBA			(0 << 27)
#   define R500_RS_IP_OFFSET_EN 			(1 << 31)

#define R500_DYN_SCLK_PWMEM_PIPE                        0x000d /* PLL */

/* r6xx/r7xx stuff */
#define R600_GRBM_STATUS                                   	   0x8010
#       define R600_CMDFIFO_AVAIL_MASK                             0x1f
#       define R700_CMDFIFO_AVAIL_MASK                             0xf
#       define R600_GUI_ACTIVE                                     (1 << 31)

#define R600_GRBM_SOFT_RESET                                    0x8020
#       define R600_SOFT_RESET_CP                               (1 << 0)

#define R600_WAIT_UNTIL                                         0x8040

#define R600_CP_ME_CNTL                                         0x86d8
#       define R600_CP_ME_HALT                                  (1 << 28)

#define R600_CP_RB_BASE                                            0xc100
#define R600_CP_RB_CNTL                                            0xc104
#       define R600_RB_NO_UPDATE                                   (1 << 27)
#       define R600_RB_RPTR_WR_ENA                                 (1 << 31)
#define R600_CP_RB_RPTR_WR                                         0xc108
#define R600_CP_RB_RPTR_ADDR                                       0xc10c
#define R600_CP_RB_RPTR_ADDR_HI                                    0xc110
#define R600_CP_RB_WPTR                                            0xc114
#define R600_CP_RB_WPTR_ADDR                                       0xc118
#define R600_CP_RB_WPTR_ADDR_HI                                    0xc11c

#define R600_CP_RB_RPTR                                            0x8700
#define R600_CP_RB_WPTR_DELAY                                      0x8704

#endif
