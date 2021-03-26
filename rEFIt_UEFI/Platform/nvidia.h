/*
 *  NVidia injector
 *
 *  Copyright (C) 2009  Jasmin Fazlic, iNDi
 *
 *  NVidia injector modified by Fabio (ErmaC) on May 2012,
 *  for allow the cosmetics injection also based on SubVendorID and SubDeviceID.
 *
 *  NVidia injector is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  NVidia driver and injector is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NVidia injector.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Alternatively you can choose to comply with APSL
 *
 * DCB-Table parsing is based on software (nouveau driver) originally distributed under following license:
 *
 *
 * Copyright 2005-2006 Erik Waling
 * Copyright 2006 Stephane Marchesin
 * Copyright 2007-2009 Stuart Bennett
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __LIBSAIO_NVIDIA_H
#define __LIBSAIO_NVIDIA_H

//#include "device_inject.h"
#include "card_vlist.h"
#include "../include/Pci.h"

extern CHAR8* gDeviceProperties;
  
//BOOLEAN setup_nvidia_devprop(pci_dt_t *nvda_dev);

typedef struct nvidia_pci_info_t
{
  UINT32    device; // VendorID + DeviceID
  CONST CHAR8     *name_model;
} nvidia_pci_info_t;

typedef struct nvidia_card_info_t
{
  UINT32 device; // VendorID + DeviceID
  UINT32 subdev; // SubdeviceID + SubvendorID
  CONST CHAR8  *name_model;
  UINT8  *custom_NVCAP;
} nvidia_card_info_t;

#define DCB_MAX_NUM_ENTRIES 16
#define DCB_MAX_NUM_I2C_ENTRIES 16

#define DCB_LOC_ON_CHIP 0

struct bios {
  UINT16  signature;    /* 0x55AA */
  UINT8    size;      /* Size in multiples of 512 */
};

#define NVIDIA_ROM_SIZE             0x20000
#define PATCH_ROM_SUCCESS           1
#define PATCH_ROM_SUCCESS_HAS_LVDS  2
#define PATCH_ROM_FAILED            0
#define MAX_NUM_DCB_ENTRIES         16
#define TYPE_GROUPED                0xff

#define NVCAP_LEN ( sizeof(default_NVCAP) / sizeof(UINT8) )
#define NVPM_LEN ( sizeof(default_NVPM) / sizeof(UINT8) )
#define DCFG0_LEN ( sizeof(default_dcfg_0) / sizeof(UINT8) )
#define DCFG1_LEN ( sizeof(default_dcfg_1) / sizeof(UINT8) )

#define NV_SUB_IDS                  0x00000000
#define NV_PMC_OFFSET               0x000000
#define NV_PMC_SIZE                 0x2ffff
#define NV_PDISPLAY_OFFSET          0x610000
#define NV_PDISPLAY_SIZE            0x10000

#define NV_PROM_OFFSET              0x300000
#define NV_PROM_SIZE                0x0001ffff
#define NV_PRAMIN_OFFSET            0x00700000
#define NV_PRAMIN_SIZE              0x00100000
#define NV04_PFB_FIFO_DATA          0x0010020c
#define NV10_PFB_FIFO_DATA_RAM_AMOUNT_MB_MASK   0xfff00000
#define NV10_PFB_FIFO_DATA_RAM_AMOUNT_MB_SHIFT  20
#define NVC0_MEM_CTRLR_COUNT        0x00121c74
#define NVC0_MEM_CTRLR_RAM_AMOUNT   0x0010f20c

#define NV_PBUS_PCI_NV_20           0x00001850
#define NV_PBUS_PCI_NV_20_ROM_SHADOW_DISABLED  (0 << 0)
#define NV_PBUS_PCI_NV_20_ROM_SHADOW_ENABLED  (1 << 0)


#define NV_ARCH_03        0x03
#define NV_ARCH_04        0x04
#define NV_ARCH_10        0x10
#define NV_ARCH_20        0x20
#define NV_ARCH_30        0x30
#define NV_ARCH_40        0x40
#define NV_ARCH_TESLA     0x50
#define NV_ARCH_FERMI1    0xC0  // Fermi
#define NV_ARCH_FERMI2    0xD0  // Fermi
#define NV_ARCH_KEPLER1   0xE0  // Kepler - GT 6XX/GTX 6XX/GTX 6XX Ti
#define NV_ARCH_KEPLER2   0xF0  // Kepler - Tesla K20X/GTX 780/GTX TITAN/TITAN LE
#define NV_ARCH_KEPLER3   0x100 // Kepler - GT 630.Rev2/635/640.Rev2/710/720/730/740
#define NV_ARCH_MAXWELL1  0x110 // Maxwell - GTX 745/750/750 Ti
#define NV_ARCH_MAXWELL2  0x120 // Maxwell - GTX 9XX/9XX Ti/TITAN X
#define NV_ARCH_PASCAL    0x130 // Pascal - GTX 10XX/10XX Ti/TITAN X/Xp
#define NV_ARCH_VOLTA     0x140 // Volta  - Titan V/Quadro GV100
#define NV_ARCH_TURING    0x160 // Turing - GTX 16xx/RTX 20xx


#define CHIPSET_NV03     0x0010
#define CHIPSET_NV04     0x0020
#define CHIPSET_NV10     0x0100
#define CHIPSET_NV11     0x0110
#define CHIPSET_NV15     0x0150
#define CHIPSET_NV17     0x0170
#define CHIPSET_NV18     0x0180
#define CHIPSET_NFORCE   0x01A0
#define CHIPSET_NFORCE2  0x01F0
#define CHIPSET_NV20     0x0200
#define CHIPSET_NV25     0x0250
#define CHIPSET_NV28     0x0280
#define CHIPSET_NV30     0x0300
#define CHIPSET_NV31     0x0310
#define CHIPSET_NV34     0x0320
#define CHIPSET_NV35     0x0330
#define CHIPSET_NV36     0x0340
#define CHIPSET_NV40     0x0040
#define CHIPSET_NV41     0x00C0
#define CHIPSET_NV43     0x0140
#define CHIPSET_NV44     0x0160
#define CHIPSET_NV44A    0x0220
#define CHIPSET_NV45     0x0210
#define CHIPSET_NV50     0x0190
#define CHIPSET_NV84     0x0400
#define CHIPSET_MISC_BRIDGED  0x00F0
#define CHIPSET_G70      0x0090
#define CHIPSET_G71      0x0290
#define CHIPSET_G72      0x01D0
#define CHIPSET_G73      0x0390

// integrated GeForces (6100, 6150)
#define CHIPSET_C51      0x0240

// variant of C51, seems based on a G70 design
#define CHIPSET_C512        0x03D0
#define CHIPSET_G73_BRIDGED 0x02E0

extern UINT8 default_NVCAP[];
extern UINT8 default_NVPM[];
extern UINT8 default_dcfg_0[];
extern UINT8 default_dcfg_1[];

BOOLEAN
setup_nvidia_devprop (
  pci_dt_t *nvda_dev
  );

CONST CHAR8
*get_nvidia_model (
  UINT32 device_id,
  UINT32 subsys_id,
                   const SETTINGS_DATA::GraphicsClass::GRAPHIC_CARD * nvcard
  );

#endif /* !__LIBSAIO_NVIDIA_H */
