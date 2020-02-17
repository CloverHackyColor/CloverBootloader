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
 *  along with NVidia injector.   If not, see <http://www.gnu.org/licenses/>.
 *
 *  Alternatively you can choose to comply with APSL
 *
 *  DCB-Table parsing is based on software (nouveau driver) originally distributed under following license:
 *
 *
 *  Copyright 2005-2006 Erik Waling
 *  Copyright 2006 Stephane Marchesin
 *  Copyright 2007-2009 Stuart Bennett
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 *  OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#include "Platform.h"
#include "nvidia.h"
#include "device_inject.h"

#ifndef DEBUG_NVIDIA
#ifndef DEBUG_ALL
#define DEBUG_NVIDIA 1
#else
#define DEBUG_NVIDIA DEBUG_ALL
#endif
#endif

#if DEBUG_NVIDIA == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_NVIDIA, __VA_ARGS__)
#endif

CHAR8 generic_name[128];

const CHAR8 *nvidia_compatible_0[]        =  { "@0,compatible",  "NVDA,NVMac"    };
const CHAR8 *nvidia_compatible_1[]        =  { "@1,compatible",  "NVDA,NVMac"    };
const CHAR8 *nvidia_device_type_0[]       =  { "@0,device_type", "display"       };
const CHAR8 *nvidia_device_type_1[]       =  { "@1,device_type", "display"       };
const CHAR8 *nvidia_device_type_parent[]  =  { "device_type",    "NVDA,Parent"   };
const CHAR8 *nvidia_device_type_child[]   =  { "device_type",    "NVDA,Child"    };
const CHAR8 *nvidia_name_0[]              =  { "@0,name",        "NVDA,Display-A"};
const CHAR8 *nvidia_name_1[]              =  { "@1,name",        "NVDA,Display-B"};
//const CHAR8 *nvidia_slot_name[]           =  { "AAPL,slot-name", "Slot-1"        };

UINT8 default_NVCAP[]= {
  0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
  0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07,
  0x00, 0x00, 0x00, 0x00
};

#define NVCAP_LEN ( sizeof(default_NVCAP) / sizeof(UINT8) )

UINT8 default_dcfg_0[]= {0x03, 0x01, 0x03, 0x00};
UINT8 default_dcfg_1[]= {0xff, 0xff, 0x00, 0x01};

#define DCFG0_LEN ( sizeof(default_dcfg_0) / sizeof(UINT8) )
#define DCFG1_LEN ( sizeof(default_dcfg_1) / sizeof(UINT8) )

UINT8 default_NVPM[]= {
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

#define NVPM_LEN ( sizeof(default_NVPM) / sizeof(UINT8) )

UINT8  pwm_info[] = {
  /* 0000 */  0x01, 0x14, 0x00, 0x64, 0xA8, 0x61, 0x00, 0x00,
  /* 0008 */  0x08, 0x52, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
  /* 0010 */  0x00, 0x04, 0x00, 0x00

};
#define PWM_LEN ( sizeof(pwm_info) / sizeof(UINT8) )

static nvidia_pci_info_t nvidia_card_vendors[] = {
  { 0x10190000,  "Elitegroup" },
  { 0x10250000,  "Acer" },
  { 0x10280000,  "Dell" },
  { 0x10330000,  "NEC" },
  { 0x103C0000,  "HP" },
  { 0x10430000,  "Asus" },
  { 0x104D0000,  "Sony" },
  { 0x105B0000,  "Foxconn" },
  { 0x106B0000,  "Apple" },
  { 0x10710000,  "Mitac" },
  { 0x107B0000,  "Gateway" },
  { 0x107D0000,  "Leadtek" },
  { 0x109F0000,  "Trigem" },
  { 0x10B00000,  "Gainward" },
  { 0x10CF0000,  "Fujitsu" },
  { 0x10DE0000,  "nVidia" },
  { 0x11790000,  "Toshiba" },
  { 0x12970000,  "Shuttle" },
  { 0x13DC0000,  "Netbost" },
  { 0x144D0000,  "Samsung" },
  { 0x14580000,  "Gigabyte" },
  { 0x14620000,  "MSI" },
  { 0x14C00000,  "Compal" },
  { 0x152D0000,  "Quanta" },
  { 0x15540000,  "Prolink" },
  { 0x15580000,  "Clevo" },
  { 0x15690000,  "Palit" },
  { 0x161F0000,  "Arima" },
  { 0x16310000,  "NEC" },
  { 0x16420000,  "Bitland" },
  { 0x16820000,  "XFX" },
  { 0x17340000,  "Fujitsu" },
  { 0x174B0000,  "PC Partner" },
  { 0x17AA0000,  "Lenovo" },
  { 0x17C00000,  "Wistron" },
  { 0x17FF0000,  "Benq" },
  { 0x18490000,  "ASRock" },
  { 0x18540000,  "LG" },
  { 0x18640000,  "LG" },
  { 0x18940000,  "LG" },
  { 0x19610000,  "ESS" },
  { 0x196E0000,  "PNY" },
  { 0x19910000,  "Topstar" },
  { 0x19DA0000,  "Zotac" },
  { 0x19F10000,  "BFG" },
  { 0x1ACC0000,  "Point of View" },
  { 0x1B0A0000,  "Pegatron" },
  { 0x1B130000,  "Jaton" },
  { 0x34420000,  "Bihl" },
  { 0x38420000,  "EVGA" },
  { 0x73770000,  "Colorful" },
};

static nvidia_pci_info_t nvidia_card_generic[] = {
  // 0000 - 0040
  { 0x10DE0000,  "Unknown" },
  // 0040 - 004F
  /*  { 0x10DE0040,  "GeForce 6800 Ultra" },
   { 0x10DE0041,  "GeForce 6800" },
   { 0x10DE0042,  "GeForce 6800 LE" },
   { 0x10DE0043,  "GeForce 6800 XE" },
   { 0x10DE0044,  "GeForce 6800 XT" },
   { 0x10DE0045,  "GeForce 6800 GT" },
   { 0x10DE0046,  "GeForce 6800 GT" },
   { 0x10DE0047,  "GeForce 6800 GS" },
   { 0x10DE0048,  "GeForce 6800 XT" },
   { 0x10DE0049,  "NV40GL" },
   { 0x10DE004D,  "Quadro FX 3400" },
   { 0x10DE004E,  "Quadro FX 4000" },  */
  // 0050 - 005F
  // 0060 - 006F
  // 0070 - 007F
  // 0080 - 008F
  // 0090 - 009F
  { 0x10DE0090,  "GeForce 7800 GTX" },
  { 0x10DE0091,  "GeForce 7800 GTX" },
  { 0x10DE0092,  "GeForce 7800 GT" },
  { 0x10DE0093,  "GeForce 7800 GS" },
  { 0x10DE0094,  "GeForce 7800SE/XT/LE/LT/ZT" },
  { 0x10DE0095,  "GeForce 7800 SLI" },
  { 0x10DE0098,  "GeForce Go 7800" },
  { 0x10DE0099,  "GeForce Go 7800 GTX" },
  { 0x10DE009D,  "Quadro FX 4500" },
  // 00A0 - 00AF
  // 00B0 - 00BF
  // 00C0 - 00CF
  /*  { 0x10DE00C0,  "GeForce 6800 GS" },
   { 0x10DE00C1,  "GeForce 6800" },
   { 0x10DE00C2,  "GeForce 6800 LE" },
   { 0x10DE00C3,  "GeForce 6800 XT" },
   { 0x10DE00C8,  "GeForce Go 6800" },
   { 0x10DE00C9,  "GeForce Go 6800 Ultra" },
   { 0x10DE00CC,  "Quadro FX Go1400" },
   { 0x10DE00CD,  "Quadro FX 3450/4000 SDI" },
   { 0x10DE00CE,  "Quadro FX 1400" },
   // 00D0 - 00DF
   // 00E0 - 00EF
   // 00F0 - 00FF
   { 0x10DE00F1,  "GeForce 6600 GT" },
   { 0x10DE00F2,  "GeForce 6600" },
   { 0x10DE00F3,  "GeForce 6200" },
   { 0x10DE00F4,  "GeForce 6600 LE" },
   { 0x10DE00F5,  "GeForce 7800 GS" },
   { 0x10DE00F6,  "GeForce 6800 GS/XT" },
   { 0x10DE00F8,  "Quadro FX 3400/4400" },
   { 0x10DE00F9,  "GeForce 6800 Series GPU" },  */
  // 0100 - 010F
  // 0110 - 011F
  // 0120 - 012F
  // 0130 - 013F
  // 0140 - 014F
  /*  { 0x10DE0140,  "GeForce 6600 GT" },
   { 0x10DE0141,  "GeForce 6600" },
   { 0x10DE0142,  "GeForce 6600 LE" },
   { 0x10DE0143,  "GeForce 6600 VE" },
   { 0x10DE0144,  "GeForce Go 6600" },
   { 0x10DE0145,  "GeForce 6610 XL" },
   { 0x10DE0146,  "GeForce Go 6600 TE/6200 TE" },
   { 0x10DE0147,  "GeForce 6700 XL" },
   { 0x10DE0148,  "GeForce Go 6600" },
   { 0x10DE0149,  "GeForce Go 6600 GT" },
   { 0x10DE014A,  "Quadro NVS 440" },
   { 0x10DE014B,  "NV43" },
   { 0x10DE014C,  "Quadro FX 550" },
   { 0x10DE014D,  "Quadro FX 550" },
   { 0x10DE014E,  "Quadro FX 540" },
   { 0x10DE014F,  "GeForce 6200" },
   // 0150 - 015F
   // 0160 - 016F
   { 0x10DE0160,  "GeForce 6500" },
   { 0x10DE0161,  "GeForce 6200 TurboCache(TM)" },
   { 0x10DE0162,  "GeForce 6200SE TurboCache(TM)" },
   { 0x10DE0163,  "GeForce 6200 LE" },
   { 0x10DE0164,  "GeForce Go 6200" },
   { 0x10DE0165,  "Quadro NVS 285" },
   { 0x10DE0166,  "GeForce Go 6400" },
   { 0x10DE0167,  "GeForce Go 6200" },
   { 0x10DE0168,  "GeForce Go 6400" },
   { 0x10DE0169,  "GeForce 6250" },
   { 0x10DE016A,  "GeForce 7100 GS" },
   { 0x10DE016C,  "NVIDIA NV44GLM" },
   { 0x10DE016D,  "NVIDIA NV44GLM" },  */
  // 0170 - 017F
  // 0180 - 018F
  // 0190 - 019F
  { 0x10DE0190,  "GeForce 8800" },
  { 0x10DE0191,  "GeForce 8800 GTX" },
  { 0x10DE0192,  "GeForce 8800" },
  { 0x10DE0193,  "GeForce 8800 GTS" },
  { 0x10DE0194,  "GeForce 8800 Ultra" },
  { 0x10DE0197,  "Tesla C870" },
  { 0x10DE019D,  "Quadro FX 5600" },
  { 0x10DE019E,  "Quadro FX 4600" },
  // 01A0 - 01AF
  // 01B0 - 01BF
  // 01C0 - 01CF
  // 01D0 - 01DF
  { 0x10DE01D0,  "GeForce 7350 LE" },
  { 0x10DE01D1,  "GeForce 7300 LE" },
  { 0x10DE01D2,  "GeForce 7550 LE" },
  { 0x10DE01D3,  "GeForce 7300 SE/7200 GS" },
  { 0x10DE01D6,  "GeForce Go 7200" },
  { 0x10DE01D7,  "GeForce Go 7300" },
  { 0x10DE01D8,  "GeForce Go 7400" },
  { 0x10DE01D9,  "GeForce Go 7450" },
  { 0x10DE01DA,  "Quadro NVS 110M" },
  { 0x10DE01DB,  "Quadro NVS 120M" },
  { 0x10DE01DC,  "Quadro FX 350M" },
  { 0x10DE01DD,  "GeForce 7500 LE" },
  { 0x10DE01DE,  "Quadro FX 350" },
  { 0x10DE01DF,  "GeForce 7300 GS" },
  // 01E0 - 01EF
  // 01F0 - 01FF
  /*  { 0x10DE01F0,  "GeForce4 MX" },
   // 0200 - 020F
   // 0210 - 021F
   { 0x10DE0211,  "GeForce 6800" },
   { 0x10DE0212,  "GeForce 6800 LE" },
   { 0x10DE0215,  "GeForce 6800 GT" },
   { 0x10DE0218,  "GeForce 6800 XT" },
   // 0220 - 022F
   { 0x10DE0221,  "GeForce 6200" },
   { 0x10DE0222,  "GeForce 6200 A-LE" },
   { 0x10DE0228,  "NVIDIA NV44M" },
   // 0230 - 023F
   // 0240 - 024F
   { 0x10DE0240,  "GeForce 6150" },
   { 0x10DE0241,  "GeForce 6150 LE" },
   { 0x10DE0242,  "GeForce 6100" },
   { 0x10DE0243,  "NVIDIA C51" },
   { 0x10DE0244,  "GeForce Go 6150" },
   { 0x10DE0245,  "Quadro NVS 210S / GeForce 6150LE" },
   { 0x10DE0247,  "GeForce Go 6100" },
   // 0250 - 025F
   { 0x10DE025B,  "Quadro4 700 XGL" },  */
  // 0260 - 026F
  // 0270 - 027F
  // 0280 - 028F
  // 0290 - 029F
  { 0x10DE0290,  "GeForce 7900 GTX" },
  { 0x10DE0291,  "GeForce 7900 GT/GTO" },
  { 0x10DE0292,  "GeForce 7900 GS" },
  { 0x10DE0293,  "GeForce 7950 GX2" },
  { 0x10DE0294,  "GeForce 7950 GX2" },
  { 0x10DE0295,  "GeForce 7950 GT" },
  { 0x10DE0296,  "G71" },
  { 0x10DE0297,  "GeForce Go 7950 GTX" },
  { 0x10DE0298,  "GeForce Go 7900 GS" },
  { 0x10DE0299,  "GeForce Go 7900 GTX" },
  { 0x10DE029A,  "Quadro FX 2500M" },
  { 0x10DE029B,  "Quadro FX 1500M" },
  { 0x10DE029C,  "Quadro FX 5500" },
  { 0x10DE029D,  "Quadro FX 3500" },
  { 0x10DE029E,  "Quadro FX 1500" },
  { 0x10DE029F,  "Quadro FX 4500 X2" },
  // 02A0 - 02AF
  // 02B0 - 02BF
  // 02C0 - 02CF
  // 02D0 - 02DF
  // 02E0 - 02EF
  { 0x10DE02E0,  "GeForce 7600 GT" },
  { 0x10DE02E1,  "GeForce 7600 GS" },
  { 0x10DE02E2,  "GeForce 7300 GT" },
  { 0x10DE02E3,  "GeForce 7900 GS" },
  { 0x10DE02E4,  "GeForce 7950 GT" },
  // 02F0 - 02FF
  // 0300 - 030F
  /*
   { 0x10DE0301,  "GeForce FX 5800 Ultra" },
   { 0x10DE0302,  "GeForce FX 5800" },
   { 0x10DE0308,  "Quadro FX 2000" },
   { 0x10DE0309,  "Quadro FX 1000" },
   // 0310 - 031F
   { 0x10DE0311,  "GeForce FX 5600 Ultra" },
   { 0x10DE0312,  "GeForce FX 5600" },
   { 0x10DE0314,  "GeForce FX 5600XT" },
   { 0x10DE031A,  "GeForce FX Go5600" },
   { 0x10DE031B,  "GeForce FX Go5650" },
   { 0x10DE031C,  "Quadro FX Go700" },
   // 0320 - 032F
   { 0x10DE0320,  "GeForce FX 5200" },
   { 0x10DE0321,  "GeForce FX 5200 Ultra" },
   { 0x10DE0322,  "GeForce FX 5200" },
   { 0x10DE0323,  "GeForce FX 5200 LE" },
   { 0x10DE0324,  "GeForce FX Go5200" },
   { 0x10DE0325,  "GeForce FX Go5250" },
   { 0x10DE0326,  "GeForce FX 5500" },
   { 0x10DE0328,  "GeForce FX Go5200 32M/64M" },
   { 0x10DE0329,  "GeForce FX Go5200" },
   { 0x10DE032A,  "Quadro NVS 55/280 PCI" },
   { 0x10DE032B,  "Quadro FX 500/600 PCI" },
   { 0x10DE032C,  "GeForce FX Go53xx Series" },
   { 0x10DE032D,  "GeForce FX Go5100" },
   { 0x10DE032F,  "NV34GL" },
   // 0330 - 033F
   { 0x10DE0330,  "GeForce FX 5900 Ultra" },
   { 0x10DE0331,  "GeForce FX 5900" },
   { 0x10DE0332,  "GeForce FX 5900XT" },
   { 0x10DE0333,  "GeForce FX 5950 Ultra" },
   { 0x10DE0334,  "GeForce FX 5900ZT" },
   { 0x10DE0338,  "Quadro FX 3000" },
   { 0x10DE033F,  "Quadro FX 700" },
   // 0340 - 034F
   { 0x10DE0341,  "GeForce FX 5700 Ultra" },
   { 0x10DE0342,  "GeForce FX 5700" },
   { 0x10DE0343,  "GeForce FX 5700LE" },
   { 0x10DE0344,  "GeForce FX 5700VE" },
   { 0x10DE0345,  "NV36.5" },
   { 0x10DE0347,  "GeForce FX Go5700" },
   { 0x10DE0348,  "GeForce FX Go5700" },
   { 0x10DE0349,  "NV36M Pro" },
   { 0x10DE034B,  "NV36MAP" },
   { 0x10DE034C,  "Quadro FX Go1000" },
   { 0x10DE034E,  "Quadro FX 1100" },
   { 0x10DE034F,  "NV36GL" },
   */
  // 0350 - 035F
  // 0360 - 036F
  // 0370 - 037F
  // 0380 - 038F
  { 0x10DE038B,  "GeForce 7650 GS" },
  // 0390 - 039F
  { 0x10DE0390,  "GeForce 7650 GS" },
  { 0x10DE0391,  "GeForce 7600 GT" },
  { 0x10DE0392,  "GeForce 7600 GS" },
  { 0x10DE0393,  "GeForce 7300 GT" },
  { 0x10DE0394,  "GeForce 7600 LE" },
  { 0x10DE0395,  "GeForce 7300 GT" },
  { 0x10DE0397,  "GeForce Go 7700" },
  { 0x10DE0398,  "GeForce Go 7600" },
  { 0x10DE0399,  "GeForce Go 7600 GT"},
  { 0x10DE039A,  "Quadro NVS 300M" },
  { 0x10DE039B,  "GeForce Go 7900 SE" },
  { 0x10DE039C,  "Quadro FX 560M" },
  { 0x10DE039E,  "Quadro FX 560" },
  // 03A0 - 03AF
  // 03B0 - 03BF
  // 03C0 - 03CF
  // 03D0 - 03DF
  { 0x10DE03D0,  "GeForce 6150SE nForce 430" },
  { 0x10DE03D1,  "GeForce 6100 nForce 405" },
  { 0x10DE03D2,  "GeForce 6100 nForce 400" },
  { 0x10DE03D5,  "GeForce 6100 nForce 420" },
  { 0x10DE03D6,  "GeForce 7025 / nForce 630a" },
  // 03E0 - 03EF
  // 03F0 - 03FF
  // 0400 - 040F
  { 0x10DE0400,  "GeForce 8600 GTS" },
  { 0x10DE0401,  "GeForce 8600 GT" },
  { 0x10DE0402,  "GeForce 8600 GT" },
  { 0x10DE0403,  "GeForce 8600 GS" },
  { 0x10DE0404,  "GeForce 8400 GS" },
  { 0x10DE0405,  "GeForce 9500M GS" },
  { 0x10DE0406,  "GeForce 8300 GS" },
  { 0x10DE0407,  "GeForce 8600M GT" },
  { 0x10DE0408,  "GeForce 9650M GS" },
  { 0x10DE0409,  "GeForce 8700M GT" },
  { 0x10DE040A,  "Quadro FX 370" },
  { 0x10DE040B,  "Quadro NVS 320M" },
  { 0x10DE040C,  "Quadro FX 570M" },
  { 0x10DE040D,  "Quadro FX 1600M" },
  { 0x10DE040E,  "Quadro FX 570" },
  { 0x10DE040F,  "Quadro FX 1700" },
  // 0410 - 041F
  { 0x10DE0410,  "GeForce GT 330" },
  // 0420 - 042F
  { 0x10DE0420,  "GeForce 8400 SE" },
  { 0x10DE0421,  "GeForce 8500 GT" },
  { 0x10DE0422,  "GeForce 8400 GS" },
  { 0x10DE0423,  "GeForce 8300 GS" },
  { 0x10DE0424,  "GeForce 8400 GS" },
  { 0x10DE0425,  "GeForce 8600M GS" },
  { 0x10DE0426,  "GeForce 8400M GT" },
  { 0x10DE0427,  "GeForce 8400M GS" },
  { 0x10DE0428,  "GeForce 8400M G" },
  { 0x10DE0429,  "Quadro NVS 140M" },
  { 0x10DE042A,  "Quadro NVS 130M" },
  { 0x10DE042B,  "Quadro NVS 135M" },
  { 0x10DE042C,  "GeForce 9400 GT" },
  { 0x10DE042D,  "Quadro FX 360M" },
  { 0x10DE042E,  "GeForce 9300M G" },
  { 0x10DE042F,  "Quadro NVS 290" },
  // 0430 - 043F
  // 0440 - 044F
  // 0450 - 045F
  // 0460 - 046F
  // 0470 - 047F
  // 0480 - 048F
  // 0490 - 049F
  // 04A0 - 04AF
  // 04B0 - 04BF
  // 04C0 - 04CF
  { 0x10DE04C0,  "NVIDIA G78" },
  { 0x10DE04C1,  "NVIDIA G78" },
  { 0x10DE04C2,  "NVIDIA G78" },
  { 0x10DE04C3,  "NVIDIA G78" },
  { 0x10DE04C4,  "NVIDIA G78" },
  { 0x10DE04C5,  "NVIDIA G78" },
  { 0x10DE04C6,  "NVIDIA G78" },
  { 0x10DE04C7,  "NVIDIA G78" },
  { 0x10DE04C8,  "NVIDIA G78" },
  { 0x10DE04C9,  "NVIDIA G78" },
  { 0x10DE04CA,  "NVIDIA G78" },
  { 0x10DE04CB,  "NVIDIA G78" },
  { 0x10DE04CC,  "NVIDIA G78" },
  { 0x10DE04CD,  "NVIDIA G78" },
  { 0x10DE04CE,  "NVIDIA G78" },
  { 0x10DE04CF,  "NVIDIA G78" },
  // 04D0 - 04DF
  // 04E0 - 04EF
  // 04F0 - 04FF
  // 0500 - 050F
  // 0510 - 051F
  // 0520 - 052F
  // 0530 - 053F
  { 0x10DE0530,  "GeForce 7190M / nForce 650M" },
  { 0x10DE0531,  "GeForce 7150M / nForce 630M" },
  { 0x10DE0533,  "GeForce 7000M / nForce 610M" },
  { 0x10DE053A,  "GeForce 7050 PV / nForce 630a" },
  { 0x10DE053B,  "GeForce 7050 PV / nForce 630a" },
  { 0x10DE053E,  "GeForce 7025 / nForce 630a" },
  // 0540 - 054F
  // 0550 - 055F
  // 0560 - 056F
  // 0570 - 057F
  // 0580 - 058F
  // 0590 - 059F
  // 05A0 - 05AF
  // 05B0 - 05BF
  // 05C0 - 05CF
  // 05D0 - 05DF
  // 05E0 - 05EF
  { 0x10DE05E0,  "GeForce GTX 295" },
  { 0x10DE05E1,  "GeForce GTX 280" },
  { 0x10DE05E2,  "GeForce GTX 260" },
  { 0x10DE05E3,  "GeForce GTX 285" },
  { 0x10DE05E4,  "NVIDIA GT200" },
  { 0x10DE05E5,  "NVIDIA GT200" },
  { 0x10DE05E6,  "GeForce GTX 275" },
  { 0x10DE05E7,  "nVidia Tesla C1060" },
  { 0x10DE05E8,  "NVIDIA GT200" },
  { 0x10DE05E9,  "NVIDIA GT200" },
  { 0x10DE05EA,  "GeForce GTX 260" },
  { 0x10DE05EB,  "GeForce GTX 295" },
  { 0x10DE05EC,  "NVIDIA GT200" },
  { 0x10DE05ED,  "Quadroplex 2200 D2" },
  { 0x10DE05EE,  "NVIDIA GT200" },
  { 0x10DE05EF,  "NVIDIA GT200" },
  // 05F0 - 05FF
  { 0x10DE05F0,  "NVIDIA GT200" },
  { 0x10DE05F1,  "NVIDIA GT200" },
  { 0x10DE05F2,  "NVIDIA GT200" },
  { 0x10DE05F3,  "NVIDIA GT200" },
  { 0x10DE05F4,  "NVIDIA GT200" },
  { 0x10DE05F5,  "NVIDIA GT200" },
  { 0x10DE05F6,  "NVIDIA GT200" },
  { 0x10DE05F7,  "NVIDIA GT200" },
  { 0x10DE05F8,  "Quadroplex 2200 S4" },
  { 0x10DE05F9,  "NVIDIA Quadro CX" },
  { 0x10DE05FA,  "NVIDIA GT200" },
  { 0x10DE05FB,  "NVIDIA GT200" },
  { 0x10DE05FC,  "NVIDIA GT200" },
  { 0x10DE05FD,  "Quadro FX 5800" },
  { 0x10DE05FE,  "Quadro FX 4800" },
  { 0x10DE05FF,  "Quadro FX 3800" },
  // 0600 - 060F
  { 0x10DE0600,  "GeForce 8800 GTS" },
  { 0x10DE0601,  "GeForce 9800 GT" },
  { 0x10DE0602,  "GeForce 8800 GT" },
  { 0x10DE0603,  "GeForce GT 230" },
  { 0x10DE0604,  "GeForce 9800 GX2" },
  { 0x10DE0605,  "GeForce 9800 GT" },
  { 0x10DE0606,  "GeForce 8800 GS" },
  { 0x10DE0607,  "GeForce GTS 240" },
  { 0x10DE0608,  "GeForce 9800M GTX" },
  { 0x10DE0609,  "GeForce 8800M GTS" },
  { 0x10DE060A,  "GeForce GTX 280M" },
  { 0x10DE060B,  "GeForce 9800M GT" },
  { 0x10DE060C,  "GeForce 8800M GTX" },
  { 0x10DE060D,  "GeForce 8800 GS" },
  { 0x10DE060F,  "GeForce GTX 285M" },
  // 0610 - 061F
  { 0x10DE0610,  "GeForce 9600 GSO" },
  { 0x10DE0611,  "GeForce 8800 GT" },
  { 0x10DE0612,  "GeForce 9800 GTX" },
  { 0x10DE0613,  "GeForce 9800 GTX+" },
  { 0x10DE0614,  "GeForce 9800 GT" },
  { 0x10DE0615,  "GeForce GTS 250" },
  { 0x10DE0617,  "GeForce 9800M GTX" },
  { 0x10DE0618,  "GeForce GTX 260M" },
  { 0x10DE0619,  "Quadro FX 4700 X2" },
  { 0x10DE061A,  "Quadro FX 3700" },
  { 0x10DE061B,  "Quadro VX 200" },
  { 0x10DE061C,  "Quadro FX 3600M" },
  { 0x10DE061D,  "Quadro FX 2800M" },
  { 0x10DE061E,  "Quadro FX 3700M" },
  { 0x10DE061F,  "Quadro FX 3800M" },
  // 0620 - 062F
  { 0x10DE0620,  "NVIDIA G94" },
  { 0x10DE0621,  "GeForce GT 230" },
  { 0x10DE0622,  "GeForce 9600 GT" },
  { 0x10DE0623,  "GeForce 9600 GS" },
  { 0x10DE0624,  "NVIDIA G94" },
  { 0x10DE0625,  "GeForce 9600 GSO 512"},
  { 0x10DE0626,  "GeForce GT 130" },
  { 0x10DE0627,  "GeForce GT 140" },
  { 0x10DE0628,  "GeForce 9800M GTS" },
  { 0x10DE0629,  "NVIDIA G94" },
  { 0x10DE062A,  "GeForce 9700M GTS" },
  { 0x10DE062B,  "GeForce 9800M GS" },
  { 0x10DE062C,  "GeForce 9800M GTS" },
  { 0x10DE062D,  "GeForce 9600 GT" },
  { 0x10DE062E,  "GeForce 9600 GT" },
  { 0x10DE062F,  "GeForce 9800 S" },
  // 0630 - 063F
  { 0x10DE0630,  "GeForce 9700 S" },
  { 0x10DE0631,  "GeForce GTS 160M" },
  { 0x10DE0632,  "GeForce GTS 150M" },
  { 0x10DE0633,  "NVIDIA G94" },
  { 0x10DE0634,  "NVIDIA G94" },
  { 0x10DE0635,  "GeForce 9600 GSO" },
  { 0x10DE0636,  "NVIDIA G94" },
  { 0x10DE0637,  "GeForce 9600 GT" },
  { 0x10DE0638,  "Quadro FX 1800" },
  { 0x10DE0639,  "NVIDIA G94" },
  { 0x10DE063A,  "Quadro FX 2700M" },
  { 0x10DE063B,  "NVIDIA G94" },
  { 0x10DE063C,  "NVIDIA G94" },
  { 0x10DE063D,  "NVIDIA G94" },
  { 0x10DE063E,  "NVIDIA G94" },
  { 0x10DE063F,  "NVIDIA G94" },
  // 0640 - 064F
  { 0x10DE0640,  "GeForce 9500 GT" },
  { 0x10DE0641,  "GeForce 9400 GT" },
  { 0x10DE0642,  "GeForce 8400 GS" },
  { 0x10DE0643,  "GeForce 9500 GT" },
  { 0x10DE0644,  "GeForce 9500 GS" },
  { 0x10DE0645,  "GeForce 9500 GS" },
  { 0x10DE0646,  "GeForce GT 120" },
  { 0x10DE0647,  "GeForce 9600M GT" },
  { 0x10DE0648,  "GeForce 9600M GS" },
  { 0x10DE0649,  "GeForce 9600M GT" },
  { 0x10DE064A,  "GeForce 9700M GT" },
  { 0x10DE064B,  "GeForce 9500M G" },
  { 0x10DE064C,  "GeForce 9650M GT" },
  // 0650 - 065F
  { 0x10DE0650,  "NVIDIA G96-825" },
  { 0x10DE0651,  "GeForce G 110M" },
  { 0x10DE0652,  "GeForce GT 130M" },
  { 0x10DE0653,  "GeForce GT 120M" },
  { 0x10DE0654,  "GeForce GT 220M" },
  { 0x10DE0655,  "GeForce GT 120" },
  { 0x10DE0656,  "GeForce 9650 S" },
  { 0x10DE0657,  "NVIDIA G96" },
  { 0x10DE0658,  "Quadro FX 380" },
  { 0x10DE0659,  "Quadro FX 580" },
  { 0x10DE065A,  "Quadro FX 1700M" },
  { 0x10DE065B,  "GeForce 9400 GT" },
  { 0x10DE065C,  "Quadro FX 770M" },
  { 0x10DE065D,  "NVIDIA G96" },
  { 0x10DE065E,  "NVIDIA G96" },
  { 0x10DE065F,  "GeForce G210" },
  // 0660 - 066F
  // 0670 - 067F
  // 0680 - 068F
  // 0690 - 069F
  // 06A0 - 06AF
  { 0x10DE06A0,  "NVIDIA GT214" },
  // 06B0 - 06BF
  { 0x10DE06B0,  "NVIDIA GT214" },
  // 06C0 - 06CF
  { 0x10DE06C0,  "GeForce GTX 480" },
  { 0x10DE06C3,  "GeForce GTX D12U" },
  { 0x10DE06C4,  "GeForce GTX 465" },
  { 0x10DE06CA,  "GeForce GTX 480M" },
  { 0x10DE06CD,  "GeForce GTX 470" },
  // 06D0 - 06DF
  { 0x10DE06D1,  "Tesla C2050" },
  { 0x10DE06D2,  "Tesla M2070" },
  { 0x10DE06D8,  "Quadro 6000" },
  { 0x10DE06D9,  "Quadro 5000" },
  { 0x10DE06DA,  "Quadro 5000M" },
  { 0x10DE06DC,  "Quadro 6000" },
  { 0x10DE06DD,  "Quadro 4000" },
  { 0x10DE06DE,  "Tesla M2050" },
  { 0x10DE06DF,  "Tesla M2070-Q" },
  // 06E0 - 06EF
  { 0x10DE06E0,  "GeForce 9300 GE" },
  { 0x10DE06E1,  "GeForce 9300 GS" },
  { 0x10DE06E2,  "GeForce 8400" },
  { 0x10DE06E3,  "GeForce 8400 SE" },
  { 0x10DE06E4,  "GeForce 8400 GS" },
  { 0x10DE06E5,  "GeForce 9300M GS" },
  { 0x10DE06E6,  "GeForce G100" },
  { 0x10DE06E7,  "GeForce 9300 SE" },
  { 0x10DE06E8,  "GeForce 9200M GE" },
  { 0x10DE06E9,  "GeForce 9300M GS" },
  { 0x10DE06EA,  "Quadro NVS 150M" },
  { 0x10DE06EB,  "Quadro NVS 160M" },
  { 0x10DE06EC,  "GeForce G 105M" },
  { 0x10DE06ED,  "NVIDIA G98" },
  { 0x10DE06EF,  "GeForce G 103M" },
  // 06F0 - 06FF
  { 0x10DE06F0,  "NVIDIA G98" },
  { 0x10DE06F1,  "GeForce G105M" },
  { 0x10DE06F2,  "NVIDIA G98" },
  { 0x10DE06F3,  "NVIDIA G98" },
  { 0x10DE06F4,  "NVIDIA G98" },
  { 0x10DE06F5,  "NVIDIA G98" },
  { 0x10DE06F6,  "NVIDIA G98" },
  { 0x10DE06F7,  "NVIDIA G98" },
  { 0x10DE06F8,  "Quadro NVS 420" },
  { 0x10DE06F9,  "Quadro FX 370 LP" },
  { 0x10DE06FA,  "Quadro NVS 450" },
  { 0x10DE06FB,  "Quadro FX 370M" },
  { 0x10DE06FC,  "NVIDIA G98" },
  { 0x10DE06FD,  "Quadro NVS 295" },
  { 0x10DE06FE,  "NVIDIA G98" },
  { 0x10DE06FF,  "HICx16 + Graphics" },
  // 0700 - 070F
  // 0710 - 071F
  // 0720 - 072F
  // 0730 - 073F
  // 0740 - 074F
  // 0750 - 075F
  // 0760 - 076F
  // 0770 - 077F
  // 0780 - 078F
  // 0790 - 079F
  // 07A0 - 07AF
  // 07B0 - 07BF
  // 07C0 - 07CF
  // 07D0 - 07DF
  // 07E0 - 07EF
  { 0x10DE07E0,  "GeForce 7150 / nForce 630i" },
  { 0x10DE07E1,  "GeForce 7100 / nForce 630i" },
  { 0x10DE07E2,  "GeForce 7050 / nForce 630i" },
  { 0x10DE07E3,  "GeForce 7050 / nForce 610i" },
  { 0x10DE07E5,  "GeForce 7050 / nForce 620i" },
  // 07F0 - 07FF
  // 0800 - 080F
  // 0810 - 081F
  // 0820 - 082F
  // 0830 - 083F
  // 0840 - 084F
  { 0x10DE0840,  "GeForce 8200M" },
  { 0x10DE0844,  "GeForce 9100M G" },
  { 0x10DE0845,  "GeForce 8200M G" },
  { 0x10DE0846,  "GeForce 9200" },
  { 0x10DE0847,  "GeForce 9100" },
  { 0x10DE0848,  "GeForce 8300" },
  { 0x10DE0849,  "GeForce 8200" },
  { 0x10DE084A,  "nForce 730a" },
  { 0x10DE084B,  "GeForce 9200" },
  { 0x10DE084C,  "nForce 980a/780a SLI" },
  { 0x10DE084D,  "nForce 750a SLI" },
  { 0x10DE084F,  "GeForce 8100 / nForce 720a" },
  // 0850 - 085F
  // 0860 - 086F
  { 0x10DE0860,  "GeForce 9300" },
  { 0x10DE0861,  "GeForce 9400" },
  { 0x10DE0862,  "GeForce 9400M G" },
  { 0x10DE0863,  "GeForce 9400M" },
  { 0x10DE0864,  "GeForce 9300" },
  { 0x10DE0865,  "GeForce 9300" },
  { 0x10DE0866,  "GeForce 9400M G" },
  { 0x10DE0867,  "GeForce 9400" },
  { 0x10DE0868,  "nForce 760i SLI" },
  { 0x10DE0869,  "GeForce 9400" },
  { 0x10DE086A,  "GeForce 9400" },
  { 0x10DE086C,  "GeForce 9300 / nForce 730i" },
  { 0x10DE086D,  "GeForce 9200" },
  { 0x10DE086E,  "GeForce 9100M G" },
  { 0x10DE086F,  "GeForce 8200M G" },
  // 0870 - 087F
  { 0x10DE0870,  "GeForce 9400M" },
  { 0x10DE0871,  "GeForce 9200" },
  { 0x10DE0872,  "GeForce G102M" },
  { 0x10DE0873,  "GeForce G205M" },
  { 0x10DE0874,  "ION 9300M" },
  { 0x10DE0876,  "ION 9400M" },
  { 0x10DE087A,  "GeForce 9400" },
  { 0x10DE087D,  "ION 9400M" },
  { 0x10DE087E,  "ION LE" },
  { 0x10DE087F,  "ION LE" }, // Tesla M2070-Q ??
  // 0880 - 088F
  // 0890 - 089F
  // 08A0 - 08AF
  { 0x10DE08A0,  "GeForce 320M" },
  { 0x10DE08A1,  "MCP89-MZT" },
  { 0x10DE08A2,  "GeForce 320M" },
  { 0x10DE08A3,  "GeForce 320M" },
  { 0x10DE08A4,  "GeForce 320M" },
  { 0x10DE08A5,  "GeForce 320M" },
  // 08B0 - 08BF
  { 0x10DE08B0,  "MCP83 MMD" },
  { 0x10DE08B1,  "GeForce 300M" },
  { 0x10DE08B2,  "GeForce 300M" }, // MCP83-MJ
  { 0x10DE08B3,  "MCP89 MM9" },
  // 08C0 - 08CF
  // 08D0 - 08DF
  // 08E0 - 08EF
  // 08F0 - 08FF
  // 0900 - 090F
  // 0910 - 091F
  // 0920 - 092F
  // 0930 - 093F
  // 0940 - 094F
  // 0950 - 095F
  // 0960 - 096F
  // 0970 - 097F
  // 0980 - 098F
  // 0990 - 099F
  // 09A0 - 09AF
  // 09B0 - 09BF
  // 09C0 - 09CF
  // 09D0 - 09DF
  // 09E0 - 09EF
  // 09F0 - 09FF
  // 0A00 - 0A0F
  // { 0x10DE0A00,  "NVIDIA GT212" },
  // 0A10 - 0A1F
  // { 0x10DE0A10,  "NVIDIA GT212" },
  // 0A20 - 0A2F
  { 0x10DE0A20,  "GeForce GT 220" },
  { 0x10DE0A21,  "D10M2-20" },
  { 0x10DE0A22,  "GeForce 315" },
  { 0x10DE0A23,  "GeForce 210" },
  { 0x10DE0A26,  "GeForce 405" },
  { 0x10DE0A27,  "GeForce 405" },
  { 0x10DE0A28,  "GeForce GT 230" },
  { 0x10DE0A29,  "GeForce GT 330M" },
  { 0x10DE0A2A,  "GeForce GT 230M" },
  { 0x10DE0A2B,  "GeForce GT 330M" },
  { 0x10DE0A2C,  "NVS 5100M" },
  { 0x10DE0A2D,  "GeForce GT 320M" },
  // 0A30 - 0A3F
  { 0x10DE0A30,  "GeForce GT 330M" },
  { 0x10DE0A32,  "GeForce GT 415" },
  { 0x10DE0A34,  "GeForce GT 240M" },
  { 0x10DE0A35,  "GeForce GT 325M" },
  { 0x10DE0A38,  "Quadro 400" },
  { 0x10DE0A3C,  "Quadro FX 880M" },
  { 0x10DE0A3D,  "N10P-ES" },
  { 0x10DE0A3F,  "GT216-INT" },
  // 0A40 - 0A4F
  // 0A50 - 0A5F
  // 0A60 - 0A6F
  { 0x10DE0A60,  "GeForce G210" },
  { 0x10DE0A61,  "NVS 2100" },
  { 0x10DE0A62,  "GeForce 205" },
  { 0x10DE0A63,  "GeForce 310" },
  { 0x10DE0A64,  "ION" },
  { 0x10DE0A65,  "GeForce 210" },
  { 0x10DE0A66,  "GeForce 310" },
  { 0x10DE0A67,  "GeForce 315" },
  { 0x10DE0A68,  "GeForce G105M" },
  { 0x10DE0A69,  "GeForce G105M" },
  { 0x10DE0A6A,  "NVS 2100M" },
  { 0x10DE0A6C,  "NVS 3100M" },
  { 0x10DE0A6E,  "GeForce 305M" },
  { 0x10DE0A6F,  "ION" },
  // 0A70 - 0A7F
  { 0x10DE0A70,  "GeForce 310M" },
  { 0x10DE0A71,  "GeForce 305M" },
  { 0x10DE0A72,  "GeForce 310M" },
  { 0x10DE0A73,  "GeForce 305M" },
  { 0x10DE0A74,  "GeForce G210M" },
  { 0x10DE0A75,  "GeForce G310M" },
  { 0x10DE0A76,  "ION" },
  { 0x10DE0A78,  "Quadro FX 380 LP" },
  // { 0x10DE0A79,  "N12M-NS-S" },
  { 0x10DE0A7A,  "GeForce 315M" },
  { 0x10DE0A7B,  "GeForce 505" },
  { 0x10DE0A7C,  "Quadro FX 380M" },
  { 0x10DE0A7D,  "N11M-ES" }, //SUBIDS
  { 0x10DE0A7E,  "GT218-INT-S" },
  { 0x10DE0A7F,  "GT218-INT-B" },
  // 0A80 - 0A8F
  // 0A90 - 0A9F
  // 0AA0 - 0AAF
  // 0AB0 - 0ABF
  // 0AC0 - 0ACF
  // 0AD0 - 0ADF
  // 0AE0 - 0AEF
  // 0AF0 - 0AFF
  // 0B00 - 0B0F
  // 0B10 - 0B1F
  // 0B20 - 0B2F
  // 0B30 - 0B3F
  // 0B40 - 0B4F
  // 0B50 - 0B5F
  // 0B60 - 0B6F
  // 0B70 - 0B7F
  // 0B80 - 0B8F
  // 0B90 - 0B9F
  // 0BA0 - 0BAF
  // 0BB0 - 0BBF
  // 0BC0 - 0BCF
  // 0BD0 - 0BDF
  // 0BE0 - 0BEF
  // 0BF0 - 0BFF
  // 0C00 - 0C0F
  // 0C10 - 0C1F
  // 0C20 - 0C2F
  // 0C30 - 0C3F
  // 0C40 - 0C4F
  // 0C50 - 0C5F
  // 0C60 - 0C6F
  // 0C70 - 0C7F
  // 0C80 - 0C8F
  // 0C90 - 0C9F
  // 0CA0 - 0CAF
  { 0x10DE0CA0,  "GeForce GT 330 " },
  { 0x10DE0CA2,  "GeForce GT 320" },
  { 0x10DE0CA3,  "GeForce GT 240" },
  { 0x10DE0CA4,  "GeForce GT 340" },
  { 0x10DE0CA5,  "GeForce GT 220" },
  { 0x10DE0CA7,  "GeForce GT 330" },
  { 0x10DE0CA8,  "GeForce GTS 260M" },
  { 0x10DE0CA9,  "GeForce GTS 250M" },
  { 0x10DE0CAC,  "GeForce GT 220" },
  { 0x10DE0CAD,  "N10E-ES" }, // SUBIDS
  { 0x10DE0CAE,  "GT215-INT" },
  { 0x10DE0CAF,  "GeForce GT 335M" },
  // 0CB0 - 0CBF
  { 0x10DE0CB0,  "GeForce GTS 350M" },
  { 0x10DE0CB1,  "GeForce GTS 360M" },
  { 0x10DE0CBC,  "Quadro FX 1800M" },
  // 0CC0 - 0CCF
  // 0CD0 - 0CDF
  // 0CE0 - 0CEF
  // 0CF0 - 0CFF
  // 0D00 - 0D0F
  // 0D10 - 0D1F
  // 0D20 - 0D2F
  // 0D30 - 0D3F
  // 0D40 - 0D4F
  // 0D50 - 0D5F
  // 0D60 - 0D6F
  // 0D70 - 0D7F
  // 0D80 - 0D8F
  // 0D90 - 0D9F
  // 0DA0 - 0DAF
  // 0DB0 - 0DBF
  // 0DC0 - 0DCF
  { 0x10DE0DC0,  "GeForce GT 440" },
  //  { 0x10DE0DC1,  "D12-P1-35" },
  //  { 0x10DE0DC2,  "D12-P1-35" },
  { 0x10DE0DC4,  "GeForce GTS 450" },
  { 0x10DE0DC5,  "GeForce GTS 450" },
  { 0x10DE0DC6,  "GeForce GTS 450" },
  //  { 0x10DE0DCA,  "GF10x" },
  //  { 0x10DE0DCC,  "N12E-GS" },
  { 0x10DE0DCD,  "GeForce GT 555M" },
  { 0x10DE0DCE,  "GeForce GT 555M" },
  //  { 0x10DE0DCF,  "N12P-GT-B" },
  // 0DD0 - 0DDF
  { 0x10DE0DD0,  "N11E-GT" },
  { 0x10DE0DD1,  "GeForce GTX 460M" },
  { 0x10DE0DD2,  "GeForce GT 445M" },
  { 0x10DE0DD3,  "GeForce GT 435M" },
  { 0x10DE0DD6,  "GeForce GT 550M" },
  { 0x10DE0DD8,  "Quadro 2000" },
  { 0x10DE0DDA,  "Quadro 2000M" },
  { 0x10DE0DDE,  "GF106-ES" },
  { 0x10DE0DDF,  "GF106-INT" },
  // 0DE0 - 0DEF
  { 0x10DE0DE0,  "GeForce GT 440" },
  { 0x10DE0DE1,  "GeForce GT 430" },
  { 0x10DE0DE2,  "GeForce GT 420" },
  { 0x10DE0DE3,  "GeForce GT 635M" },
  { 0x10DE0DE4,  "GeForce GT 520" },
  { 0x10DE0DE5,  "GeForce GT 530" },
  { 0x10DE0DE8,  "GeForce GT 620M" },
  { 0x10DE0DE9,  "GeForce GT 630M" },
  { 0x10DE0DEA,  "GeForce GT 610M" },
  { 0x10DE0DEB,  "GeForce GT 555M" },
  { 0x10DE0DEC,  "GeForce GT 525M" },
  { 0x10DE0DED,  "GeForce GT 520M" },
  { 0x10DE0DEE,  "GeForce GT 415M" },
  { 0x10DE0DEF,  "NVS 5400M" },
  // 0DF0 - 0DFF
  { 0x10DE0DF0,  "GeForce GT 425M" },
  { 0x10DE0DF1,  "GeForce GT 420M" },
  { 0x10DE0DF2,  "GeForce GT 435M" },
  { 0x10DE0DF3,  "GeForce GT 420M" },
  { 0x10DE0DF4,  "GeForce GT 540M" },
  { 0x10DE0DF5,  "GeForce GT 525M" },
  { 0x10DE0DF6,  "GeForce GT 550M" },
  { 0x10DE0DF7,  "GeForce GT 520M" },
  { 0x10DE0DF8,  "Quadro 600" },
  { 0x10DE0DF9,  "Quadro 500M" },
  { 0x10DE0DFA,  "Quadro 1000M" },
  { 0x10DE0DFC,  "NVS 5200M" },
  { 0x10DE0DFE,  "GF108 ES" },
  { 0x10DE0DFF,  "GF108 INT" },
  // 0E00 - 0E0F
  // 0E10 - 0E1F
  // 0E20 - 0E2F
  { 0x10DE0E21,  "D12U-25" },
  { 0x10DE0E22,  "GeForce GTX 460" },
  { 0x10DE0E23,  "GeForce GTX 460 SE" },
  { 0x10DE0E24,  "GeForce GTX 460" },
  { 0x10DE0E25,  "D12U-50" },
  { 0x10DE0E28,  "GeForce GTX 460" },
  // 0E30 - 0E3F
  { 0x10DE0E30,  "GeForce GTX 470M" },
  { 0x10DE0E31,  "GeForce GTX 485M" },
  { 0x10DE0E32,  "N12E-GT" },
  { 0x10DE0E38,  "GF104GL" },
  { 0x10DE0E3A,  "Quadro 3000M" },
  { 0x10DE0E3B,  "Quadro 4000M" },
  { 0x10DE0E3E,  "GF104-ES" },
  { 0x10DE0E3F,  "GF104-INT" },
  // 0E40 - 0E4F
  // 0E50 - 0E5F
  // 0E60 - 0E6F
  // 0E70 - 0E7F
  // 0E80 - 0E8F
  // 0E90 - 0E9F
  // 0EA0 - 0EAF
  // 0EB0 - 0EBF
  // 0EC0 - 0ECF
  // 0ED0 - 0EDF
  // 0EE0 - 0EEF
  // 0EF0 - 0EFF
  // 0F00 - 0F0F
  { 0x10DE0F00,  "GeForce GT 630" },
  { 0x10DE0F01,  "GeForce GT 620" },
  { 0x10DE0F02,  "GeForce GT 730" },  //GF108
  // 0F10 - 0F1F
  // 0F20 - 0F2F
  // 0F30 - 0F3F
  // 0F40 - 0F4F
  // 0F50 - 0F5F
  // 0F60 - 0F6F
  // 0F70 - 0F7F
  // 0F80 - 0F8F
  // 0F90 - 0F9F
  // 0FA0 - 0FAF
  // 0FB0 - 0FBF
  { 0x10DE0FBB,  "GeForce GTX 970" },
  // 0FC0 - 0FCF
  { 0x10DE0FC0,  "GeForce GT 640" },
  { 0x10DE0FC1,  "GeForce GT 640" },
  { 0x10DE0FC2,  "GeForce GT 630" },
  { 0x10DE0FC6,  "GeForce GTX 650" },
  { 0x10DE0FC8,  "GeForce GT 740" },
  { 0x10DE0FCD,  "GeForce GT 755M" },
  { 0x10DE0FCE,  "GeForce GT 640M LE" },
  // 0FD0 - 0FDF
  { 0x10DE0FD1,  "GeForce GT 650M" },
  { 0x10DE0FD2,  "GeForce GT 640M" },
  { 0x10DE0FD3,  "GeForce GT 640M LE" },
  { 0x10DE0FD4,  "GeForce GTX 660M" },
  { 0x10DE0FD5,  "GeForce GT 650M" },
  { 0x10DE0FD8,  "GeForce GT 640M" },
  { 0x10DE0FD9,  "GeForce GT 645M" },
  { 0x10DE0FDA,  "GK107-ES-A1" },
  { 0x10DE0FDB,  "GK107-ESP-A1" },
  { 0x10DE0FDC,  "GK107-INT22-A1" },
  { 0x10DE0FDF,  "GeForce GT 740M" },
  // 0FE0 - 0FEF
  { 0x10DE0FE0,  "GeForce GTX 660M" },
  { 0x10DE0FE1,  "GeForce GT 730M" },
  { 0x10DE0FE3,  "GeForce GT 745M" },
  { 0x10DE0FE4,  "GeForce GT 750M" },
  { 0x10DE0FE5,  "GeForce K340 USM" },
  { 0x10DE0FE6,  "NVS K1 USM" },
  { 0x10DE0FE7,  "Generic K1 USM / GRID K100" },
  { 0x10DE0FE9,  "GeForce GT 750M" },
  { 0x10DE0FEA,  "GeForce GT 755M" },
  { 0x10DE0FEF,  "GRID K340" },
  // 0FF0 - 0FFF
  { 0x10DE0FF0,  "NB1Q" },
  { 0x10DE0FF1,  "NVS 1000" },
  { 0x10DE0FF2,  "GRID K1" },
  { 0x10DE0FF3,  "Quadro K420" },
  { 0x10DE0FF5,  "Tesla K1 USM" },
  { 0x10DE0FF6,  "Quadro K1100M" },
  { 0x10DE0FF7,  "Quadro K1 USM" }, // K1 USM / GRID K120Q / GRID K140Q
  { 0x10DE0FF8,  "Quadro K500M" },
  { 0x10DE0FF9,  "Quadro K2000D" },
  { 0x10DE0FFA,  "Quadro K600" },
  { 0x10DE0FFB,  "Quadro K2000M" },
  { 0x10DE0FFC,  "Quadro K1000M" },
  { 0x10DE0FFD,  "NVS 510" },
  { 0x10DE0FFE,  "Quadro K2000" },
  { 0x10DE0FFF,  "Quadro 410" },
  // 1000 - 100F
  { 0x10DE1001,  "GeForce GTX TITAN Z" },
  { 0x10DE1003,  "GeForce GTX Titan LE" },
  { 0x10DE1004,  "GeForce GTX 780" },
  { 0x10DE1005,  "GeForce GTX Titan" },
  { 0x10DE1006,  "GeForce GTX 780 Ti" },
  { 0x10DE1007,  "GeForce GTX 780" },
  { 0x10DE1008,  "GeForce GTX 780 Ti" },
  { 0x10DE100A,  "GeForce GTX 780 Ti" },
  //  { 0x10DE100B,  "Graphics Device" }, // GK110
  { 0x10DE100C,  "GeForce GTX Titan Black" },
  // 1010 - 101F
  { 0x10DE101E,  "Tesla K20X" }, // GK110GL
  { 0x10DE101F,  "Tesla K20" },
  // 1020 - 102F
  { 0x10DE1020,  "Tesla K20X" },
  { 0x10DE1021,  "Tesla K20Xm" },
  { 0x10DE1022,  "Tesla K20c" },
  { 0x10DE1023,  "Tesla K40m" },  // GK110BGL
  { 0x10DE1024,  "Tesla K40c" },  // GK110BGL
  { 0x10DE1026,  "Tesla K20s" },
  { 0x10DE1027,  "Tesla K40st" }, // GK110BGL
  { 0x10DE1028,  "Tesla K20m" },
  { 0x10DE1029,  "Tesla K40s" }, // GK110BGL
  { 0x10DE102A,  "Tesla K40t" }, // GK110BGL
  //  { 0x10DE102B,  "Graphics Device" }, // GK110BGL
  //  { 0x10DE102C,  "Graphics Device" }, // GK110BGL
  { 0x10DE102D,  "Tesla K80" }, // GK110BGL (2x)
  { 0x10DE102E,  "Tesla K40d" }, // GK110BGL
  { 0x10DE102F,  "Tesla Stella Solo" }, // GK110BGL
  // 1030 - 103F
  //  { 0x10DE1030,  "" }, // GK110
  { 0x10DE103a,  "Quadro K6000" }, // GK110GL
  { 0x10DE103c,  "Quadro K5200" }, // GK110GL
  { 0x10DE103F,  "Tesla Stella SXM" }, // GK110
  // 1040 - 104F
  { 0x10DE1040,  "GeForce GT 520" },
  //  { 0x10DE1041,  "D13M1-45" },
  { 0x10DE1042,  "GeForce 510" },
  { 0x10DE1048,  "GeForce 605" },
  { 0x10DE1049,  "GeForce GT 620" },
  { 0x10DE104A,  "GeForce GT 610" },
  { 0x10DE104B,  "GeForce GT 625 (OEM)" },
  { 0x10DE104C,  "GeForce GT 705" }, // GF119
  { 0x10DE104D,  " GeForce GT 710" }, // GF119
  // 1050 - 105F
  { 0x10DE1050,  "GeForce GT 520M" },
  { 0x10DE1051,  "GeForce GT 520MX" },
  { 0x10DE1052,  "GeForce GT 520M" },
  { 0x10DE1054,  "GeForce GT 410M" },
  { 0x10DE1055,  "GeForce 410M" },
  { 0x10DE1056,  "Quadro NVS 4200M" },
  { 0x10DE1057,  "Quadro NVS 4200M" },
  { 0x10DE1058,  "GeForce GT 610M" },
  { 0x10DE1059,  "GeForce 610M" },
  { 0x10DE105A,  "GeForce 610M" },
  { 0x10DE105B,  "GeForce 705A" },
  // 1060 - 106F
  // 1070 - 107F
  { 0x10DE107C,  "Quadro NVS 315" },
  { 0x10DE107D,  "Quadro NVS 310" },
  //  { 0x10DE107E,  "GF119-INT" },
  { 0x10DE107F,  "GF119-ES" },
  // 1080 - 108F
  { 0x10DE1080,  "GeForce GTX 580" },
  { 0x10DE1081,  "GeForce GTX 570" },
  { 0x10DE1082,  "GeForce GTX 560 Ti" },
  { 0x10DE1083,  "D13U" },
  { 0x10DE1084,  "GeForce GTX 560" },
  { 0x10DE1086,  "GeForce GTX 570 HD" },
  { 0x10DE1087,  "GeForce GTX 560 Ti-448" },
  { 0x10DE1088,  "GeForce GTX 590" },
  { 0x10DE1089,  "GeForce GTX 580" },
  { 0x10DE108B,  "GeForce GTX 590" },
  //  { 0x10DE108C,  "D13U" },
  { 0x10DE108E,  "Tesla C2090" },
  // 1090 - 109F
  { 0x10DE1091,  "Tesla M2090" }, // X2090
  { 0x10DE1094,  "Tesla M2075" },
  { 0x10DE1096,  "Tesla C2075" },
  { 0x10DE1098,  "D13U" },
  { 0x10DE109A,  "Quadro 5010M" },
  { 0x10DE109B,  "Quadro 7000" },
  // 10A0 - 10AF
  // 10B0 - 10BF
  // 10C0 - 10CF
  { 0x10DE10C0,  "GeForce 9300 GS" },
  { 0x10DE10C3,  "GeForce 8400 GS" },
  { 0x10DE10C4,  "ION" },
  { 0x10DE10C5,  "GeForce 405" },
  // 10D0 - 10DF
  { 0x10DE10D8,  "Quadro NVS 300" },
  // 10E0 - 10EF
  // 10F0 - 10FF
  // 1100 - 110F
  // 1110 - 111F
  // 1120 - 112F
  { 0x10DE1128,  "GeForce GTX 970M" },
  // 1130 - 113F
  // 1140 - 114F
  { 0x10DE1140,  "GeForce GT 610M" },
  { 0x10DE1141,  "GeForce 610M" },
  { 0x10DE1142,  "GeForce 620M" },
  { 0x10DE1143,  "N13P-GV" },
  { 0x10DE1144,  "GF117" },
  { 0x10DE1145,  "GF117" },
  { 0x10DE1146,  "GF117" },
  { 0x10DE1147,  "GF117" },
  { 0x10DE1149,  "GF117-ES" },
  { 0x10DE114A,  "GF117-INT" },
  { 0x10DE114B,  "PCI-GEN3-B" },
  { 0x10DE1150,  "N13M-NS" },
  // 1160 - 116F
  // 1170 - 117F
  // 1180 - 118F
  { 0x10DE1180,  "GeForce GTX 680" },
  { 0x10DE1182,  "GeForce GTX 760 Ti" },
  { 0x10DE1183,  "GeForce GTX 660 Ti" },
  { 0x10DE1184,  "GeForce GTX 770" },
  { 0x10DE1185,  "GeForce GTX 660 OEM" },
  { 0x10DE1187,  "GeForce GTX 760" },
  { 0x10DE1188,  "GeForce GTX 690" },
  { 0x10DE1189,  "GeForce GTX 670" },
  { 0x10DE118A,  "GRID K520" },
  { 0x10DE118B,  "GRID K200" }, // GRID K2 GeForce USM
  { 0x10DE118C,  "GRID K2 NVS USM" }, // GK104
  { 0x10DE118D,  "GRID K200 vGPU" }, // GK104GL
  { 0x10DE118E,  "GeForce GTX 760 (192-bit)" },
  { 0x10DE118F,  "Tesla K10" },
  // 1190 - 119F
  { 0x10DE1191,  "GeForce GTX 760" }, // GK104
  { 0x10DE1192,  "GeForce GK104" },
  { 0x10DE1193,  "GeForce GTX 760 Ti" },
  { 0x10DE1194,  "Tesla K8" }, // GK104
  { 0x10DE1195,  "GeForce GTX 660" },
  { 0x10DE1198,  "GeForce GTX 880M" },
  { 0x10DE1199,  "GeForce GTX 870M" },
  { 0x10DE119A,  "GeForce GTX 860M" },
  { 0x10DE119D,  "GeForce GTX 775M" }, // Mac Edition
  { 0x10DE119E,  "GeForce GTX 780M" }, // Mac Edition
  { 0x10DE119F,  "GeForce GTX 780M" },  //GK104
  // 11A0 - 11AF
  { 0x10DE11A0,  "GeForce GTX 680M" },
  { 0x10DE11A1,  "GeForce GTX 670MX" },
  { 0x10DE11A2,  "GeForce GTX 675MX" }, // Mac Edition
  { 0x10DE11A3,  "GeForce GTX 680MX" },
  { 0x10DE11A7,  "GeForce GTX 675MX" },
  { 0x10DE11AF,  "GRID IceCube" }, // GF104M
  // 11B0 - 11BF
  { 0x10DE11B0,  "GRID K240Q" }, // K260Q vGPU
  { 0x10DE11B1,  "GRID K2 Tesla USM" },
  { 0x10DE11B4,  "Quadro K4200" },
  { 0x10DE11B6,  "Quadro K3100M" },
  { 0x10DE11B7,  "Quadro K4100M" },
  { 0x10DE11B8,  "Quadro K5100M" },
  { 0x10DE11BA,  "Quadro K5000" },
  { 0x10DE11BB,  "Quadro 4100" },
  { 0x10DE11BC,  "Quadro K5000M" },
  { 0x10DE11BD,  "Quadro K4000M" },
  { 0x10DE11BE,  "Quadro K3000M" },
  { 0x10DE11BF,  "GRID K2" }, // GK104GL
  // 11C0 - 11CF
  { 0x10DE11C0,  "GeForce GTX 660" },
  { 0x10DE11C2,  "GeForce GTX 650 Ti BOOST" },
  { 0x10DE11C3,  "GeForce GTX 650 Ti" },
  { 0x10DE11C4,  "GeForce GTX 645" },
  { 0x10DE11C6,  "GeForce GTX 650 Ti" },
  { 0x10DE11C7,  "GeForce GTX 750 Ti" },
  { 0x10DE11C8,  "GeForce GTX 650 OEM" },
  // 11D0 - 11DF
  { 0x10DE11D0,  "GK106-INT353" },
  // 11E0 - 11EF
  { 0x10DE11E0,  "GeForce GTX 770M" },
  { 0x10DE11E1,  "GeForce GTX 765M" },
  { 0x10DE11E2,  "GeForce GTX 765M" },
  { 0x10DE11E3,  "GeForce GTX 760M" },
  //  { 0x10DE11E7,  "GeForce " }, // GK106M
  // 11F0 - 11FF
  { 0x10DE11FA,  "Quadro K4000" },
  { 0x10DE11FC,  "Quadro K2100M" },
  { 0x10DE11FF,  "NB1Q" }, //
  // 1200 - 120F
  { 0x10DE1200,  "GeForce GTX 560 Ti" },
  { 0x10DE1201,  "GeForce GTX 560" },
  { 0x10DE1202,  "GeForce GTX 560 Ti" },
  { 0x10DE1203,  "GeForce GTX 460 SE v2" },
  { 0x10DE1205,  "GeForce GTX 460 v2" },
  { 0x10DE1206,  "GeForce GTX 555" },
  { 0x10DE1207,  "GeForce GT 645" },
  { 0x10DE1208,  "GeForce GTX 560 SE" },
  { 0x10DE1210,  "GeForce GTX 570M" },
  { 0x10DE1211,  "GeForce GTX 580M" },
  { 0x10DE1212,  "GeForce GTX 675M" },
  { 0x10DE1213,  "GeForce GTX 670M" },
  { 0x10DE121F,  "GF114-INT" },
  { 0x10DE1240,  "GeForce GT 620M" },
  { 0x10DE1241,  "GeForce GT 545" },
  { 0x10DE1243,  "GeForce GT 545" },
  { 0x10DE1244,  "GeForce GTX 550 Ti" }, //GF116 chip=0xCF
  { 0x10DE1245,  "GeForce GTS 450" },
  { 0x10DE1246,  "GeForce GT 550M" },
  { 0x10DE1247,  "GeForce GT 555M" },
  { 0x10DE1248,  "GeForce GT 555M" },
  { 0x10DE1249,  "GeForce GTS 450" },
  { 0x10DE124B,  "GeForce GT 640" },
  { 0x10DE124D,  "GeForce GT 555M" },
  { 0x10DE1250,  "GF116-INT" },
  { 0x10DE1251,  "GeForce GTX 560M" },
  // 1260 - 126F
  // 1270 - 127F
  // 1280 - 128F
  { 0x10DE1280,  "GeForce GT 635" },
  { 0x10DE1281,  "GeForce GT 710" },
  { 0x10DE1282,  "GeForce GT 640" },
  { 0x10DE1284,  "GeForce GT 630" },
  { 0x10DE1286,  "GeForce GT 720" },
  { 0x10DE1287,  "GeForce GT 730" }, // GK208
  { 0x10DE1288,  "GeForce GT 720" }, // GK208
  { 0x10DE128b,  "GeForce GT 710" },
  // 1290 - 129F
  { 0x10DE1290,  "GeForce GT 730M" },
  { 0x10DE1291,  "GeForce GT 735M" },
  { 0x10DE1292,  "GeForce GT 740M" },
  { 0x10DE1293,  "GeForce GT 730M" },
  { 0x10DE1294,  "GeForce GT 740M" },
  { 0x10DE1295,  "GeForce GT 710M" },
  { 0x10DE1296,  "GeForce 825M" }, // GK208M
  { 0x10DE1298,  "GeForce GT 720M" },
  { 0x10DE1299,  "GeForce 920M" }, // GK208M
  // 12A0 - 12AF
  { 0x10DE12A0,  "GK208" },
  { 0x10DE12AF,  "GK208-INT" },
  { 0x10DE12B0,  "GK208-CS-Q" },
  { 0x10DE12B9,  "Quadro K610M" },
  { 0x10DE12BA,  "Quadro K510M" },
  // 12B0 - 12BF
  // 12C0 - 12CF
  // 12D0 - 12DF
  // 12E0 - 12EF
  // 12F0 - 12FF
  { 0x10DE1340,  "GeForce 830M" },
  { 0x10DE1341,  "GeForce 840M" },
  { 0x10DE1346,  "GeForce 930M" }, // GM108M
  { 0x10DE1347,  "GeForce 940M" }, // GM108M
  { 0x10DE1348,  "GeForce 945M/945A" }, // GM108M
  { 0x10DE1349,  "GeForce 930M" }, // GM108M
  { 0x10DE134D,  "GeForce 940MX" }, // GM108M
  { 0x10DE134E,  "GeForce 930MX" }, // GM108M
  { 0x10DE134F,  "GeForce 920MX" }, // GM108M
  { 0x10DE137A,  "Quadro K620M/M500M" }, // GM108GLM
  { 0x10DE137B,  "Quadro M520" },
  { 0x10DE137D,  "GeForce 940A" }, // GM108M
  { 0x10DE1380,  "GeForce GTX 750 Ti" },
  { 0x10DE1381,  "GeForce GTX 750" },
  { 0x10DE1382,  "GeForce GTX 745" },
  //  { 0x10DE1383,  "Graphics Device" }, // GM107
  { 0x10DE1389,  "GRID M3" }, // GM107
  { 0x10DE1390,  "GeForce 845M" },
  { 0x10DE1391,  "GeForce GTX 850M" },
  { 0x10DE1392,  "GeForce GTX 860M" },
  { 0x10DE1393,  "GeForce 840M" },
  { 0x10DE1398,  "GeForce 845M" }, //
  { 0x10DE139A,  "GeForce GTX 950M" }, // GM107
  { 0x10DE139B,  "GeForce GTX 960M" }, // GM107
  { 0x10DE139C,  "GeForce 940M" }, // GM107M
  { 0x10DE139D,  "GeForce GTX 750 Ti" }, // GM107M
  { 0x10DE13AD,  "GM107 INT52" }, //
  { 0x10DE13AE,  "GM107 CS1" }, //
  //  { 0x10DE13AF,  "Graphics Device" }, // GM107GLM
  { 0x10DE13B0,  "GQuadro M2000M" }, // GM107GLM
  { 0x10DE13B1,  "Quadro M1000M" }, // GM107GLM
  { 0x10DE13B2,  "Quadro M600M" }, // GM107GLM
  { 0x10DE13B3,  "Quadro K2200M" }, // GM107GLM
  { 0x10DE13B4,  "Quadro M620" },
  { 0x10DE13B6,  "Quadro M1200" },
  { 0x10DE13B9,  "NVS 810" }, // GM107GL
  { 0x10DE13BA,  "Quadro K2200" },
  { 0x10DE13BB,  "Quadro K620" },
  { 0x10DE13BC,  "Quadro K1200" },
  { 0x10DE13BD,  "Tesla M40" }, // GM107GLM
  { 0x10DE13BE,  "GM107 CS1" }, //
  { 0x10DE13BF,  "GM107 INT52" }, //
  // 12B0 - 12BF
  { 0x10DE13C0,  "GeForce GTX 980" }, // GM107GLM
  //  { 0x10DE13C1,  "Graphics Device" }, // GM107GLM
  { 0x10DE13C2,  "GeForce GTX 970" }, // GM107GLM
  //  { 0x10DE13C3,  "Graphics Device" }, // GM107GLM
  { 0x10DE13D7,  "GeForce GTX 980M" }, //
  { 0x10DE13D8,  "GeForce GTX 970M" }, //
  { 0x10DE13D9,  "GeForce GTX 965M" },
  { 0x10DE13DA,  "GeForce GTX 980" }, // GM204M
  { 0x10DE13F0,  "Quadro M5000" }, // GM204GL
  { 0x10DE13F1,  "Quadro M4000" }, // GM204GL
  { 0x10DE13F2,  "Tesla M60" }, // GM204GL
  { 0x10DE13F3,  "Tesla M6" }, // GM204GL
  { 0x10DE13F8,  "Quadro M5000M" }, // GM204GLM
  { 0x10DE13F9,  "Quadro M4000M" }, // GM204GLM
  { 0x10DE13FA,  "Quadro M3000M" }, // GM204GLM
  { 0x10DE13FB,  "Quadro M5500" }, // GM204GLM
  { 0x10DE1401,  "GeForce GTX 960" }, //
  { 0x10DE1402,  "GeForce GTX 950" }, //
  { 0x10DE1406,  "GeForce GTX 960" }, // GM206
  { 0x10DE1407,  "GeForce GTX 750 v2" }, // GM206
  { 0x10DE1427,  "GeForce GTX 965M" }, // GM206M
  { 0x10DE1430,  "Quadro M2000" }, //
  { 0x10DE1431,  "Tesla M4" }, // GM206GL
  { 0x10DE1436,  "Quadro M2200" },
  //  { 0x10DE143F,  "Graphics Device" }, //
  //  { 0x10DE1600,  "Graphics Device" }, //
  //  { 0x10DE1601,  "Graphics Device" }, //
  //  { 0x10DE1602,  "Graphics Device" }, //
  //  { 0x10DE1603,  "Graphics Device" }, //
  { 0x10DE1617,  "GeForce GTX 980M" }, //
  { 0x10DE1618,  "GeForce GTX 970M" }, // GM204M
  { 0x10DE1619,  "GeForce GTX 965M" }, // GM204M
  { 0x10DE161A,  "GeForce GTX 980" }, // GM204M
  //  { 0x10DE1630,  "Graphics Device" }, //
  //  { 0x10DE1631,  "Graphics Device" }, //
  { 0x10DE1667,  "GeForce GTX 965M" }, // GM204M
  //  { 0x10DE1780,  "Graphics Device" }, //
  //  { 0x10DE1781,  "Graphics Device" }, //
  //  { 0x10DE1782,  "Graphics Device" }, //
  //  { 0x10DE1783,  "Graphics Device" }, //
  { 0x10DE1789,  "GRID M3-3020" }, //
  { 0x10DE1790,  "N15S-GX" }, //
  { 0x10DE1791,  "N15P-GT" }, //
  { 0x10DE1792,  "N15P-GX" }, //
  //  { 0x10DE17B3,  "Quadro" }, //
  //  { 0x10DE17BA,  "Quadro" }, //
  //  { 0x10DE17BB,  "Quadro" }, //
  //  { 0x10DE17BD,  "Graphics Device" }, //
  { 0x10DE17BE,  "GM107 CS1" }, // GM107
  //  { 0x10DE17C1,  "Graphics Device" }, //
  { 0x10DE17C2,  "GeForce GTX Titan X" }, //
  { 0x10DE17C8,  "GeForce GTX 980 TI" }, //
  //  { 0x10DE17EE,  "Graphics Device" }, //
  //  { 0x10DE17EF,  "Graphics Device" }, //
  { 0x10DE17F0,  "Quadro M6000" },
  //  { 0x10DE17FF,  "Graphics Device" }, //
  { 0x10DE17F1,  "Quadro M6000" }, // GM200GL
  { 0x10DE17FD,  "Tesla M40" }, // GM200GL
  // 1B00 - 1CFFF
  { 0x10DE1B00,   "Titan X Pascal"}, // GP102
  //  { 0x10DE1B01,  "Graphics Device" }, // GP102
  { 0x10DE1B06,   "GeForce GTX 1080 Ti"}, // GP102
  { 0x10DE1B30,  "Quadro P6000" }, // GP102GL
  //  { 0x10DE1B70,  "Graphics Device" }, // GP102GL
  //  { 0x10DE1B78,  "Graphics Device" }, // GP102GL
  { 0x10DE1B80,   "GeForce GTX 1080"}, // GP104
  { 0x10DE1B81,   "GeForce GTX 1070"}, // GP104
  //  { 0x10DE1B82,  "Graphics Device" }, // GP104
  { 0x10DE1B83,  "GeForce GTX 1060" }, // GP104
  { 0x10DE1BA0,  "GeForce GTX 1080" }, // GP104M
  { 0x10DE1BA1,  "GeForce GTX 1070" }, // GP104M
  { 0x10DE1BB0,  "Quadro P5000" }, // GP104GL
  //  { 0x10DE1BB1,  "Graphics Device" }, // GP104GL
  //  { 0x10DE1BB4,  "Graphics Device" }, // GP104GL
  { 0x10DE1BB6,  "Quadro P5000" },
  { 0x10DE1BB7,  "Quadro P4000" },
  { 0x10DE1BB8,  "Quadro P3000" },
  { 0x10DE1BE0,  "GeForce GTX 1080" }, //GP104M
  { 0x10DE1BE1,  "GeForce GTX 1070" }, //GP104M
  //  { 0x10DE1C00,  "Graphics Device" }, // GP106
  //  { 0x10DE1C01,  "Graphics Device" }, // GP106
  { 0x10DE1C02,   "GeForce GTX 1060"}, // GP106
  { 0x10DE1C03,   "GeForce GTX 1060"}, // GP106
  { 0x10DE1C06,   "GeForce GTX 1060"}, // GP106
  //  { 0x10DE1C07,   "NVIDIA CMP6-1"}, //
  { 0x10DE1C20,  "GeForce GTX 1060" }, //GP106M
  //  { 0x10DE1C30,  "Graphics Device" }, // GP106GL
  { 0x10DE1C60,  "GeForce GTX 1060" }, // GP106M
  //  { 0x10DE1C70,  "Graphics Device" }, // GP106GL
  //  { 0x10DE1C80,  "Graphics Device" }, // GP107
  { 0x10DE1C81,  "GeForce GTX 1050" }, // GP107
  { 0x10DE1C82,  "GeForce GTX 1050 Ti"}, // GP107
  { 0x10DE1D01,  "GeForce GTX 1030"}, // family 138
  { 0x10DE1D10,  "GeForce MX150"},
  { 0x10DE1F06,  "GeForce RTX 2060 SUPER" }, // TU106
  // 2000 - 1EFFF
};

static nvidia_card_info_t nvidia_card_exceptions[] = {
  /* ========================================================================================
   * Layout is device(VendorId + DeviceId), subdev (SubvendorId + SubdeviceId), display name.
   * ========================================================================================
   */
  /* ------ Specific DeviceID and SubDevID. ------ */
  // 0000 - 00FF
  { 0x10DE0040,  0x10438178,  "Asus V9999 Ultra V62.11" },
  { 0x10DE0040,  0x1043817D,  "Asus V9999GT V61.21" },
  { 0x10DE0040,  0x7FFFFFFF,  "GeForce 6800 Ultra [NV40.0]" },
  // 0100 - 01FF
  { 0x10DE01D7,  0x102801C2,  "Dell Quadro NVS 110M" },
  { 0x10DE01D7,  0x102801CC,  "Dell Quadro NVS 110M" },
  { 0x10DE01D7,  0x10DE014B,  "nVidia Quadro NVS 110M" },

  { 0x10DE01D8,  0x102801CC,  "Dell Quadro NVS 120M" },
  { 0x10DE01D8,  0x10282003,  "Dell Quadro NVS 120M" },

  { 0x10DE01DA,  0x10280407,  "Dell GeForce 7300 LE" },
  // 0200 - 02FF
  { 0x10DE025B,  0x10480D23,  "ELSA Gloria4 700XGL" },
  // 0300 - 03FF
  { 0x10DE0391,  0x10DE047A,  "Galaxy GeForce 7600 GT" },
  { 0x10DE0391,  0x19F120DE,  "Galaxy GeForce 7600 GT" },

  { 0x10DE0393,  0x00000400,  "Apple GeForce 7300GT" },
  // 0400 - 04FF
  { 0x10DE0402,  0x10DE0439,  "Galaxy 8600GT" },
  { 0x10DE0402,  0x10DE0505,  "Galaxy 8600GT" },
  // 0500 - 05FF
  { 0x10DE05E2,  0x104382EB,  "ASUS ENGTX260" },
  { 0x10DE05E2,  0x16822390,  "HFX GeForce GTX 260" },
  { 0x10DE05E2,  0x17870000,  "HIS GeForce GTX 260" },

  { 0x10DE05E6,  0x10B00401,  "Gainward GeForce GTX 285" },

  { 0x10DE05E7,  0x10DE0595,  "nVidia Tesla T10 Processor" },
  { 0x10DE05E7,  0x10DE066A,  "nVidia Tesla C1060" },
  { 0x10DE05E7,  0x10DE068F,  "nVidia Tesla T10 Processor" },
  { 0x10DE05E7,  0x10DE0697,  "nVidia Tesla M1060" },
  { 0x10DE05E7,  0x10DE0714,  "nVidia Tesla M1060" },
  { 0x10DE05E7,  0x10DE0743,  "nVidia Tesla M1060" },
  // 0600 - 06FF
  { 0x10DE0600,  0x10DE0000,  "Abit GeForce 8800 GTS" },

  { 0x10DE0605,  0x145834A2,  "Gigabyte GV-N98TOC-512H" },

  { 0x10DE0608,  0x15880577,  "Solidum GeForce 9800M GTX" },

  { 0x10DE0609,  0x11700121,  "Inventec GeForce 8800M GTS" },

  { 0x10DE0612,  0x104382A6,  "Asus GeForce 9800 GTX+" },
  { 0x10DE0612,  0x10DE0571,  "nVidia GeForce 9800 GTX+" },
  { 0x10DE0612,  0x10DE0592,  "nVidia GeForce 9800 GTX+" },
  { 0x10DE0612,  0x3842C842,  "EVGA GeForce 9800 GTX+" },
  { 0x10DE0612,  0x3842C875,  "EVGA GeForce 9800 GTX+" },

  { 0x10DE0615,  0x10480F67,  "ELSA GeForce GTS 250" },
  { 0x10DE0615,  0x10DE0592,  "Palit GeForce GTS 250" },
  { 0x10DE0615,  0x10DE0593,  "Palit GeForce GTS 250" },
  { 0x10DE0615,  0x10DE0652,  "Palit GeForce GTS 250" },
  { 0x10DE0615,  0x10DE0719,  "Palit GeForce GTS 250" },
  { 0x10DE0615,  0x10DE079E,  "Palit GeForce GTS 250" },
  { 0x10DE0615,  0x11503842,  "TMC GeForce GTS 250" }, // Thinking Machines Corporation
  { 0x10DE0615,  0x11513842,  "JAE GeForce GTS 250" },
  { 0x10DE0615,  0x11553842,  "Pine GeForce GTS 250" },
  { 0x10DE0615,  0x11563842,  "Periscope GeForce GTS 250" },

  { 0x10DE0618,  0x10432028,  "Asus GeForce GTX 170M" },
  { 0x10DE0618,  0x1043202B,  "Asus GeForce GTX 680" },

  { 0x10DE0622,  0x104382AC,  "Asus EN9600GT Magic" },

  { 0x10DE0640,  0x10DE077F,  "Inno3D GeForce 9500GT HDMI" },

  { 0x10DE0649,  0x1043202D,  "Asus GeForce GT 220M" },

  { 0x10DE06CD,  0x10DE079F,  "Point of View GeForce GTX 470" },
  { 0x10DE06CD,  0x14622220,  "MSI GeForce GTX 470 Twin Frozr II" },

  { 0x10DE06D1,  0x10DE0771,  "nVidia Tesla C2050" },
  { 0x10DE06D1,  0x10DE0772,  "nVidia Tesla C2070" },

  { 0x10DE06D2,  0x10DE0774,  "nVidia Tesla M2070" },
  { 0x10DE06D2,  0x10DE0830,  "nVidia Tesla M2070" },
  { 0x10DE06D2,  0x10DE0842,  "nVidia Tesla M2070" },
  { 0x10DE06D2,  0x10DE088F,  "nVidia Tesla X2070" },
  { 0x10DE06D2,  0x10DE0908,  "nVidia Tesla M2070" },

  { 0x10DE06DE,  0x10DE0773,  "nVidia Tesla S2050" },
  { 0x10DE06DE,  0x10DE0830,  "nVidia Tesla M2070" },
  { 0x10DE06DE,  0x10DE0831,  "nVidia Tesla M2070" },
  { 0x10DE06DE,  0x10DE0832,  "nVidia Tesla M2070" },
  { 0x10DE06DE,  0x10DE0840,  "nVidia Tesla X2070" },

  { 0x10DE06E4,  0x10438322,  "Asus EN8400GS" },
  { 0x10DE06E4,  0x14583475,  "GV-NX84S256HE [GeForce 8400 GS]" },

  { 0x10DE06E8,  0x10280262,  "Dell GeForce 9200M GS" },
  { 0x10DE06E8,  0x10280271,  "Dell GeForce 9200M GS" },
  { 0x10DE06E8,  0x10280272,  "Dell GeForce 9200M GS" },
  { 0x10DE06E8,  0x103C30F4,  "HP GeForce 9200M GS" },
  { 0x10DE06E8,  0x103C30F7,  "HP GeForce 9200M GS" },
  { 0x10DE06E8,  0x103C3603,  "HP GeForce 9200M GS" },
  // 0700 - 07FF
  // 0800 - 08FF
  { 0x10DE0873,  0x104319B4,  "Asus GeForce G102M" },
  // 0900 - 09FF
  // 0A00 - 0AFF
  { 0x10DE0A6F,  0x12974003,  "Shuttle XS 3510MA" },

  { 0x10DE0A70,  0x17AA3605,  "Lenovo ION" },

  { 0x10DE0A73,  0x17AA3607,  "Lenovo ION" },
  { 0x10DE0A73,  0x17AA3610,  "Lenovo ION" },

  { 0x10DE0A75,  0x17AA3605,  "Lenovo ION" },
  // 0B00 - 0BFF
  // 0C00 - 0CFF
  { 0x10DE0CA3,  0x14628041,  "MSI VN240GT-MD1G" },
  { 0x10DE0CA3,  0x16423926,  "Bitland GeForce GT 230" },
  // 0D00 - 0DFF
  { 0x10DE0DD8,  0x10DE0914,  "nVidia Quadro 2000D" },

  { 0x10DE0DEF,  0x17AA21F3,  "Lenovo NVS 5400M" },
  { 0x10DE0DEF,  0x17AA21F4,  "Lenovo NVS 5400M" },
  { 0x10DE0DEF,  0x17AA21F5,  "Lenovo NVS 5400M" },
  { 0x10DE0DEF,  0x17AA21F6,  "Lenovo NVS 5400M" },
  { 0x10DE0DEF,  0x17AA5005,  "Lenovo NVS 5400M" },

  // 0E00 - 0EFF
  { 0x10DE0E22,  0x1043835D,  "Asus ENGTX460" },

  { 0x10DE0E23,  0x10B00401,  "Gainward GeForce GTX 460" },
  // 0F00 - 0FFF
  { 0x10DE0FBB,  0x38422974,  "EVGA GTX 970 OC" },
  { 0x10DE0FD2,  0x10280595,  "Dell GeForce GT 640M LE" },
  { 0x10DE0FD2,  0x102805B2,  "Dell GeForce GT 640M LE" },
  // 1000 - 10FF
  { 0x10DE1080,  0x14622561,  "MSI N580GTX Lightning" },
  { 0x10DE1080,  0x14622563,  "MSI N580GTX Lightning" },

  { 0x10DE1086,  0x10DE0871,  "Inno3D GeForce GTX 570" },

  { 0x10DE1087,  0x104383D6,  "Asus ENGTX560Ti448 DCII" },

  { 0x10DE1091,  0x10DE088E,  "nVidia Tesla X2090" },
  { 0x10DE1091,  0x10DE0891,  "nVidia Tesla X2090" },

  { 0x10DE1094,  0x10DE0888,  "nVidia Tesla M2075" },

  { 0x10DE1096,  0x10DE0910,  "nVidia Tesla C2075" },
  { 0x10DE1096,  0x10DE0911,  "nVidia Tesla C2050" },

  // 1100 - 11FF
  { 0x10DE1140,  0x1025064A,  "Acer GeForce GT 620M" },
  { 0x10DE1140,  0x1025064C,  "Acer GeForce GT 620M" },
  { 0x10DE1140,  0x10250680,  "Acer GeForce GT 620M" },
  { 0x10DE1140,  0x10250692,  "Acer GeForce GT 620M" },
  { 0x10DE1140,  0x10250694,  "Acer GeForce GT 620M" },
  { 0x10DE1140,  0x10250702,  "Acer GeForce GT 620M" },
  { 0x10DE1140,  0x10250719,  "Acer GeForce GT 620M" },
  { 0x10DE1140,  0x10250725,  "Acer GeForce GT 620M" },
  { 0x10DE1140,  0x10250728,  "Acer GeForce GT 620M" },
  { 0x10DE1140,  0x1025072B,  "Acer GeForce GT 620M" },
  { 0x10DE1140,  0x1025072E,  "Acer GeForce GT 620M" },
  { 0x10DE1140,  0x10250732,  "Acer GeForce GT 620M" },
  { 0x10DE1140,  0x10280565,  "Dell GeForce GT 630M" },
  { 0x10DE1140,  0x10280568,  "Dell GeForce GT 630M" },
  { 0x10DE1140,  0x144DC0D5,  "Samsung GeForce GT 630M" },
  { 0x10DE1140,  0x17AA2200,  "nVidia NVS 5200M" },
  { 0x10DE1140,  0x17AA2213,  "nVidia GeForce GT 720M" },
  { 0x10DE1140,  0x17AA500D,  "Lenovo GeForce GT 620M" },
  { 0x10DE1140,  0x1B0A20DD,  "Pegatron GeForce GT 620M" },
  { 0x10DE1140,  0x1B0A20FD,  "Pegatron GeForce GT 620M" },

  { 0x10DE1180,  0x00001255,  "Afox GTX 680" },
  { 0x10DE1180,  0x104383F0,  "Asus GTX680-2GD5" },
  { 0x10DE1180,  0x104383F6,  "Asus GTX 680 Direct CU II" },
  { 0x10DE1180,  0x104383F7,  "Asus GTX 680 Direct CU II" },
  { 0x10DE1180,  0x1458353C,  "GV-N680OC-2GD WindForce GTX 680 OC" },
  { 0x10DE1180,  0x14622820,  "MSI N680GTX TwinFrozer" },
  { 0x10DE1180,  0x14622830,  "MSI GTX 680 Lightning" },
  { 0x10DE1180,  0x14622831,  "MSI GTX 680 Lightning LN2" },
  { 0x10DE1180,  0x15691180,  "Palit GTX 680 JetStream" },
  { 0x10DE1180,  0x15691181,  "Palit GTX 680 JetStream" },
  { 0x10DE1180,  0x15691189,  "Palit GTX 680 JetStream" },
  { 0x10DE1180,  0x38422682,  "EVGA GTX 680 SC" },
  { 0x10DE1180,  0x38422683,  "EVGA GTX 680 SC" },

  { 0x10DE1185,  0x10DE106F,  "nVidia GeForce GTX 760 OEM" }, // GK104

  { 0x10DE1187,  0x14583614,  "GV-N760OC-4GD" },

  { 0x10DE1189,  0x10438405,  "Asus GTX 670 Direct CU II TOP" },
  { 0x10DE1189,  0x15691189,  "Palit GTX 670 JetStream" },
  { 0x10DE1189,  0x19DA1255,  "Zotac GTX 670 AMP! Edition" },

  { 0x10DE11A1,  0x15587102,  "Clevo N13E-GR" },

  { 0x10DE11C0,  0x10DE0995,  "Inno3D GeForce GTX660" },
  { 0x10DE11C0,  0x1458354E,  "GV-N660OC-2GD" },

  { 0x10DE11C6,  0x1043842A,  "GTX650TI-1GD5" },
  // 1200 - 12FF
  { 0x10DE1247,  0x10432119,  "Asus GeForce GT 670M" },
  { 0x10DE1247,  0x10432120,  "Asus GeForce GT 670M" },
  { 0x10DE1247,  0x1043212A,  "Asus GeForce GT 635M" },
  { 0x10DE1247,  0x1043212B,  "Asus GeForce GT 635M" },
  { 0x10DE1247,  0x1043212C,  "Asus GeForce GT 635M" },
  { 0x10DE1247,  0x152D0930,  "Quanta GeForce GT 635M" },

  { 0x10DE1248,  0x152D0930,  "Quanta GeForce GT 635M" },

  { 0x10DE124D,  0x146210CC,  "MSi GeForce GT 635M" }
};

EFI_STATUS read_nVidia_PRAMIN(pci_dt_t *nvda_dev, VOID* rom, UINT16 arch)
{
  EFI_STATUS Status;
  EFI_PCI_IO_PROTOCOL    *PciIo;
  PCI_TYPE00      Pci;

  UINT32 vbios_vram = 0;
  UINT32 old_bar0_pramin = 0;

  DBG("read_nVidia_ROM\n");
  Status = gBS->OpenProtocol(nvda_dev->DeviceHandle, &gEfiPciIoProtocolGuid, (VOID**)&PciIo, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if (EFI_ERROR(Status)) {
    return EFI_NOT_FOUND;
  }

  Status = PciIo->Pci.Read(PciIo,EfiPciIoWidthUint32, 0, sizeof(Pci) / sizeof(UINT32), &Pci);
  if (EFI_ERROR(Status)) {
    return EFI_NOT_FOUND;
  }

  if (arch >= 0x50) {
    DBG("Using PRAMIN fixups\n");
    /*Status = */PciIo->Mem.Read(
                                 PciIo,
                                 EfiPciIoWidthUint32,
                                 0,
                                 NV_PDISPLAY_OFFSET + 0x9f04,///4,
                                 1,
                                 &vbios_vram
                                 );
    vbios_vram = (vbios_vram & ~0xff) << 8;

    /*Status = */PciIo->Mem.Read(
                                 PciIo,
                                 EfiPciIoWidthUint32,
                                 0,
                                 NV_PMC_OFFSET + 0x1700,///4,
                                 1,
                                 &old_bar0_pramin
                                 );

    if (vbios_vram == 0)
      vbios_vram = (old_bar0_pramin << 16) + 0xf0000;

    vbios_vram >>= 16;

    /*Status = */PciIo->Mem.Write(
                                  PciIo,
                                  EfiPciIoWidthUint32,
                                  0,
                                  NV_PMC_OFFSET + 0x1700,///4,
                                  1,
                                  &vbios_vram
                                  );
  }

  Status = PciIo->Mem.Read(
                           PciIo,
                           EfiPciIoWidthUint8,
                           0,
                           NV_PRAMIN_OFFSET,
                           NVIDIA_ROM_SIZE,
                           rom
                           );

  if (arch >= 0x50) {
    /*Status = */PciIo->Mem.Write(
                                  PciIo,
                                  EfiPciIoWidthUint32,
                                  0,
                                  NV_PMC_OFFSET + 0x1700,///4,
                                  1,
                                  &old_bar0_pramin
                                  );
  }

  if (EFI_ERROR(Status)) {
    DBG("read_nVidia_ROM failed\n");
    return Status;
  }
  return EFI_SUCCESS;
}


EFI_STATUS read_nVidia_PROM(pci_dt_t *nvda_dev, VOID* rom)
{
  EFI_STATUS Status;
  EFI_PCI_IO_PROTOCOL    *PciIo;
  PCI_TYPE00            Pci;
  UINT32                value;

  DBG("PROM\n");
  Status = gBS->OpenProtocol(nvda_dev->DeviceHandle,
                             &gEfiPciIoProtocolGuid,
                             (VOID**)&PciIo,
                             gImageHandle,
                             NULL,
                             EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if (EFI_ERROR(Status)) {
    return EFI_NOT_FOUND;
  }

  Status = PciIo->Pci.Read(PciIo,
                           EfiPciIoWidthUint32,
                           0,
                           sizeof(Pci) / sizeof(UINT32),
                           &Pci);
  if (EFI_ERROR(Status)) {
    return EFI_NOT_FOUND;
  }

  value = NV_PBUS_PCI_NV_20_ROM_SHADOW_DISABLED;
  /*Status = */PciIo->Mem.Write(
                                PciIo,
                                EfiPciIoWidthUint32,
                                0,
                                NV_PBUS_PCI_NV_20*4,
                                1,
                                &value
                                );

  Status = PciIo->Mem.Read(
                           PciIo,
                           EfiPciIoWidthUint8,
                           0,
                           NV_PROM_OFFSET,
                           NVIDIA_ROM_SIZE,
                           rom
                           );

  value = NV_PBUS_PCI_NV_20_ROM_SHADOW_ENABLED;
  /*Status = */PciIo->Mem.Write(
                                PciIo,
                                EfiPciIoWidthUint32,
                                0,
                                NV_PBUS_PCI_NV_20*4,
                                1,
                                &value
                                );

  if (EFI_ERROR(Status)) {
    DBG("read_nVidia_ROM failed\n");
    return Status;
  }
  return EFI_SUCCESS;
}

static INT32 patch_nvidia_rom(UINT8 *rom)
{
  UINT8 num_outputs = 0, i = 0;
  UINT8 dcbtable_version;
  UINT8 headerlength   = 0;
  UINT8 numentries     = 0;
  UINT8 recordlength   = 0;
  UINT8 channel1 = 0, channel2 = 0;
  UINT16 dcbptr;
  UINT8 *dcbtable;
  UINT8 *togroup;
  INT32 has_lvds = FALSE;
  struct dcbentry {
    UINT8 type;
    UINT8 index;
    UINT8 *heads;
  } entries[MAX_NUM_DCB_ENTRIES];

  //  DBG("patch_nvidia_rom\n");
  if (!rom || (rom[0] != 0x55 && rom[1] != 0xaa)) {
    DBG("FALSE ROM signature: 0x%02x%02x\n", rom[0], rom[1]);
    return PATCH_ROM_FAILED;
  }

  //  dcbptr = SwapBytes16(read16(rom, 0x36)); //double swap !!!
  dcbptr = *(UINT16*)&rom[0x36];
  if(!dcbptr) {
    DBG("no dcb table found\n");
    return PATCH_ROM_FAILED;
  }
  //  else
  //    DBG("dcb table at offset 0x%04x\n", dcbptr);

  dcbtable    = &rom[dcbptr];
  dcbtable_version  = dcbtable[0];

  if (dcbtable_version >= 0x20) {
    UINT32 sig;

    if (dcbtable_version >= 0x30) {
      headerlength = dcbtable[1];
      numentries   = dcbtable[2];
      recordlength = dcbtable[3];
      sig = *(UINT32 *)&dcbtable[6];
    } else {
      sig = *(UINT32 *)&dcbtable[4];
      headerlength = 8;
    }

    if (sig != 0x4edcbdcb) {
      DBG("Bad display config block signature (0x%8x)\n", sig); //Azi: issue #48
      return PATCH_ROM_FAILED;
    }
  } else if (dcbtable_version >= 0x14) { /* some NV15/16, and NV11+ */
    CHAR8 sig[8]; // = { 0 };

    AsciiStrnCpyS(sig, 8, (CHAR8 *)&dcbtable[-7], 7);
    sig[7] = 0;
    recordlength = 10;

    if (AsciiStrCmp(sig, "DEV_REC")) {
      DBG("Bad Display Configuration Block signature (%a)\n", sig);
      return PATCH_ROM_FAILED;
    }
  } else {
    DBG("ERROR: dcbtable_version is 0x%X\n", dcbtable_version);
    return PATCH_ROM_FAILED;
  }

  if (numentries >= MAX_NUM_DCB_ENTRIES) {
    numentries = MAX_NUM_DCB_ENTRIES;
  }

  for (i = 0; i < numentries; i++) {
    UINT32 connection;
    connection = *(UINT32 *)&dcbtable[headerlength + recordlength * i];

    /* Should we allow discontinuous DCBs? Certainly DCB I2C tables can be discontinuous */
    if ((connection & 0x0000000f) == 0x0000000f) { /* end of records */
      continue;
    }
    if (connection == 0x00000000) { /* seen on an NV11 with DCB v1.5 */
      continue;
    }
    if ((connection & 0xf) == 0x6) { /* we skip type 6 as it doesnt appear on macbook nvcaps */
      continue;
    }

    entries[num_outputs].type = connection & 0xf;
    entries[num_outputs].index = num_outputs;
    entries[num_outputs++].heads = (UINT8*)&(dcbtable[(headerlength + recordlength * i) + 1]);
  }

  for (i = 0; i < num_outputs; i++) {
    if (entries[i].type == 3) {
      has_lvds =TRUE;
      //DBG("found LVDS\n");
      channel1 |= (0x1 << entries[i].index);
      entries[i].type = TYPE_GROUPED;
    }
  }

  // if we have a LVDS output, we group the rest to the second channel
  if (has_lvds) {
    for (i = 0; i < num_outputs; i++) {
      if (entries[i].type == TYPE_GROUPED) {
        continue;
      }
      channel2 |= (0x1 << entries[i].index);
      entries[i].type = TYPE_GROUPED;
    }
  } else {
    INT32 x;
    // we loop twice as we need to generate two channels
    for (x = 0; x <= 1; x++) {
      for (i=0; i<num_outputs; i++) {
        if (entries[i].type == TYPE_GROUPED) {
          continue;
        }
        // if type is TMDS, the prior output is ANALOG
        // we always group ANALOG and TMDS
        // if there is a TV output after TMDS, we group it to that channel as well
        if (i && entries[i].type == 0x2) {
          switch (x) {
            case 0:
              //DBG("group channel 1\n");
              channel1 |= (0x1 << entries[i].index);
              entries[i].type = TYPE_GROUPED;

              if (entries[i-1].type == 0x0) {
                channel1 |= (0x1 << entries[i-1].index);
                entries[i-1].type = TYPE_GROUPED;
              }
              // group TV as well if there is one
              if (((i+1) < num_outputs) && (entries[i+1].type == 0x1)) {
                //  DBG("group tv1\n");
                channel1 |= (0x1 << entries[i+1].index);
                entries[i+1].type = TYPE_GROUPED;
              }
              break;

            case 1:
              //DBG("group channel 2 : %d\n", i);
              channel2 |= ( 0x1 << entries[i].index);
              entries[i].type = TYPE_GROUPED;

              if (entries[i - 1].type == 0x0) {
                channel2 |= (0x1 << entries[i-1].index);
                entries[i-1].type = TYPE_GROUPED;
              }
              // group TV as well if there is one
              if (((i+1) < num_outputs) && (entries[i+1].type == 0x1)) {
                //  DBG("group tv2\n");
                channel2 |= ( 0x1 << entries[i+1].index);
                entries[i+1].type = TYPE_GROUPED;
              }
              break;
            default:
              break;

          }
          break;
        }
      }
    }
  }

  // if we have left ungrouped outputs merge them to the empty channel
  togroup = &channel2; // = (channel1 ? (channel2 ? NULL : &channel2) : &channel1);

  for (i = 0; i < num_outputs; i++) {
    if (entries[i].type != TYPE_GROUPED) {
      //DBG("%d not grouped\n", i);
      if (togroup) {
        *togroup |= (0x1 << entries[i].index);
      }
      entries[i].type = TYPE_GROUPED;
    }
  }

  if (channel1 > channel2) {
    UINT8 buff = channel1;
    channel1 = channel2;
    channel2 = buff;
  }

  default_NVCAP[6] = channel1;
  default_NVCAP[8] = channel2;

  // patching HEADS
  for (i = 0; i < num_outputs; i++) {
    if (channel1 & (1 << i)) {
      *entries[i].heads = 1;
    } else if(channel2 & (1 << i)) {
      *entries[i].heads = 2;
    }
  }
  return (has_lvds ? PATCH_ROM_SUCCESS_HAS_LVDS : PATCH_ROM_SUCCESS);
}

CONST CHAR8 *get_nvidia_model(UINT32 device_id, UINT32 subsys_id, CARDLIST * nvcard)
{
  UINTN i, j;
  //DBG("get_nvidia_model for (%08x, %08x)\n", device_id, subsys_id);

  //ErmaC added selector for nVidia "old" style in System Profiler
  //DBG("NvidiaGeneric = %s\n", gSettings.NvidiaGeneric?L"YES":L"NO");
  if (gSettings.NvidiaGeneric == FALSE) {
    // First check in the plist, (for e.g this can override any hardcoded devices)
    //CARDLIST * nvcard = FindCardWithIds(device_id, subsys_id);
    if (nvcard && (nvcard->Id == device_id) && (nvcard->SubId == subsys_id)) {
      return nvcard->Model;
    }

    // Then check the exceptions table
    if (subsys_id) {
      for (i = 0; i < (sizeof(nvidia_card_exceptions) / sizeof(nvidia_card_exceptions[0])); i++) {
        if ((nvidia_card_exceptions[i].device == device_id) &&
            (nvidia_card_exceptions[i].subdev == subsys_id)) {
          return nvidia_card_exceptions[i].name_model;
        }
      }
    }
  }

  // At last try the generic names
  for (i = 1; i < (sizeof(nvidia_card_generic) / sizeof(nvidia_card_generic[0])); i++) {
    if (nvidia_card_generic[i].device == device_id) {
      //--
      //ErmaC added selector for nVidia "old" style in System Profiler
      if (gSettings.NvidiaGeneric) {
        DBG("Apply NvidiaGeneric\n");
        AsciiSPrint(generic_name, 128, "NVIDIA %a", nvidia_card_generic[i].name_model);
        return &generic_name[0]; // generic_name;
      }
      //      DBG("Not applied NvidiaGeneric\n");
      //--
      if (subsys_id) {
        for (j = 0; j < (sizeof(nvidia_card_vendors) / sizeof(nvidia_card_vendors[0])); j++) {
          if (nvidia_card_vendors[j].device == (subsys_id & 0xffff0000)) {
            AsciiSPrint(generic_name, 128, "%a %a",
                        nvidia_card_vendors[j].name_model,
                        nvidia_card_generic[i].name_model);
            return &generic_name[0]; // generic_name;
          }
        }
      }
      return nvidia_card_generic[i].name_model;
    }
  }
  return nvidia_card_generic[0].name_model;
}

UINT8 connector_type_1[]= {0x00, 0x08, 0x00, 0x00};
UINT32  boot_display = 1;

static INT32 devprop_add_nvidia_template(DevPropDevice *device, INTN n_ports)
{
  INTN    pnum;
  CHAR8 nkey[24];
  CHAR8 nval[24];

  //  DBG("devprop_add_nvidia_template\n");

  if (!device) {
    return 0;
  }

  for (pnum = 0; pnum < n_ports; pnum++) {
    AsciiSPrint(nkey, 24, "@%d,name", pnum);
    AsciiSPrint(nval, 24, "NVDA,Display-%c", (65+pnum));
    //DBG("Nvidia: insert [%a : %a]\n", nkey, nval);
    devprop_add_value(device, nkey, (UINT8*)nval, 14);

    AsciiSPrint(nkey, 24, "@%d,compatible", pnum);
    devprop_add_value(device, nkey, (UINT8*)"NVDA,NVMac", 10);

    AsciiSPrint(nkey, 24, "@%d,device_type", pnum);
    devprop_add_value(device, nkey, (UINT8*)"display", 7);

    AsciiSPrint(nkey, 24, "@%d,display-cfg", pnum);
    if (pnum == 0) {
      devprop_add_value(device, nkey, (gSettings.Dcfg[0] != 0) ? &gSettings.Dcfg[0] : default_dcfg_0, DCFG0_LEN);
    } else {
      devprop_add_value(device, nkey, (gSettings.Dcfg[1] != 0) ? &gSettings.Dcfg[4] : default_dcfg_1, DCFG1_LEN);
    }
  }

  if (devices_number == 1) {
    devprop_add_value(device, "device_type", (UINT8*)"NVDA,Parent", 11);
  } else {
    devprop_add_value(device, "device_type", (UINT8*)"NVDA,Child", 10);
  }

  return 1;
}

UINT64 mem_detect(UINT16 nvCardType, pci_dt_t *nvda_dev)
{
  UINT64 vram_size = 0;
  /*
   // First check if any value exist in the plist
   CARDLIST * nvcard = FindCardWithIds(((nvda_dev->vendor_id << 16) | nvda_dev->device_id),((nvda_dev->subsys_id.subsys.vendor_id << 16) | nvda_dev->subsys_id.subsys.device_id));
   if (nvcard) {
   if (nvcard->VideoRam > 0) {
   // VideoRam * 1024 * 1024 == VideoRam << 20
   vram_size = LShiftU64(nvcard->VideoRam, 20);
   DBG("mem assigned %ld\n", vram_size);
   return vram_size;
   }
   }
   */
  if (nvCardType < NV_ARCH_TESLA) {
    vram_size  = (UINT64)(REG32(nvda_dev->regs, NV04_PFB_FIFO_DATA));
    vram_size &= NV10_PFB_FIFO_DATA_RAM_AMOUNT_MB_MASK;
  } else if (nvCardType < NV_ARCH_FERMI1) {
    vram_size = (UINT64)(REG32(nvda_dev->regs, NV04_PFB_FIFO_DATA));
    vram_size |= LShiftU64(vram_size & 0xff, 32);
    vram_size &= 0xffffffff00ll;
  } else if (nvCardType < NV_ARCH_KEPLER1) {
    vram_size = LShiftU64(REG32(nvda_dev->regs, NVC0_MEM_CTRLR_RAM_AMOUNT), 20);
    vram_size = MultU64x32(vram_size, REG32(nvda_dev->regs, NVC0_MEM_CTRLR_COUNT));
  } else if ((nvCardType < NV_ARCH_KEPLER3) || ((nvCardType >= NV_ARCH_MAXWELL2) &&
                                                (nvCardType < NV_ARCH_PASCAL))) {
    // Kepler - GT 6XX/GTX 6XX/GTX 6XX Ti/Tesla K20X/GTX 780/GTX TITAN/TITAN LE
    // Maxwell - GTX 9XX/9XX Ti/TITAN X
    vram_size = LShiftU64(2 * REG32(nvda_dev->regs, NVC0_MEM_CTRLR_RAM_AMOUNT), 20);
  } else {
    // Kepler - GT 630.Rev2/635/640.Rev2/710/720/730/740
    // Maxwell - GTX 745/750/750 Ti
    // TODO: need to find vram size calculation for Pascal. by Sherlocks
    vram_size = LShiftU64(REG32(nvda_dev->regs, NVC0_MEM_CTRLR_RAM_AMOUNT), 20);
  }

  // Then, Workaround for 9600M GT, GT 210/420/430/440/525M/540M & GTX 560M
  switch (nvda_dev->device_id) {
    case 0x0647: // 9600M GT 0647
      vram_size = 512*1024*1024;
      break;
    case 0x0649:  // 9600M GT 0649
      // 10DE06491043202D 1GB VRAM
      if (((nvda_dev->subsys_id.subsys.vendor_id << 16) | nvda_dev->subsys_id.subsys.device_id) == 0x1043202D) {
        vram_size = 1024*1024*1024;
      }
      break;
    case 0x0A65: // GT 210
    case 0x0DE0: // GT 440
    case 0x0DE1: // GT 430
    case 0x0DE2: // GT 420
    case 0x0DEC: // GT 525M 0DEC
    case 0x0DF4: // GT 540M
    case 0x0DF5: // GT 525M 0DF5
      vram_size = 1024*1024*1024;
      break;
    case 0x1251: // GTX 560M
      vram_size = 1536*1024*1024;
      break;
    default:
      break;
  }
  DBG("mem_detected %ldMb\n", (vram_size >> 20));
  return vram_size;
}

BOOLEAN setup_nvidia_devprop(pci_dt_t *nvda_dev)
{
  const         INT32 MAX_BIOS_VERSION_LENGTH = 32;
  EFI_STATUS    Status = EFI_NOT_FOUND;
  DevPropDevice *device = NULL;
  CHAR8         *devicepath = NULL;
  BOOLEAN       load_vbios = gSettings.LoadVBios;
  BOOLEAN       Injected = FALSE;
  UINT8         *rom = NULL;
  UINT16        nvCardType = 0;
  UINT64        videoRam = 0;
  UINT32        bar[7];
  //  UINT32        boot_display = 0;
  //UINT32        subsystem;
  INT32         nvPatch = 0;
  CONST CHAR8         *model = NULL;
  CHAR16        FileName[64];
  UINT8         *buffer = NULL;
  UINTN         bufferLen = 0;
  UINTN         j, n_ports = 0;
  UINTN         i, version_start;
  INT32         crlf_count = 0;
  option_rom_pci_header_t    *rom_pci_header;
  CHAR8*        s;
  CHAR8*        s1;
  CHAR8*        version_str = (CHAR8*)AllocateZeroPool(MAX_BIOS_VERSION_LENGTH);
  BOOLEAN       RomAssigned = FALSE;
  UINT32        device_id, subsys_id;
  CARDLIST      *nvcard;

  devicepath = get_pci_dev_path(nvda_dev);
  bar[0] = pci_config_read32(nvda_dev, PCI_BASE_ADDRESS_0);
  nvda_dev->regs = (UINT8 *)(UINTN)(bar[0] & ~0x0f);
  //  subsystem = (nvda_dev->subsys_id.subsys.vendor_id << 16) +
  //  nvda_dev->subsys_id.subsys.device_id;
  device_id = ((nvda_dev->vendor_id << 16) | nvda_dev->device_id);
  subsys_id = ((nvda_dev->subsys_id.subsys.vendor_id << 16) | nvda_dev->subsys_id.subsys.device_id);

  // get card type
  nvCardType = (REG32(nvda_dev->regs, 0) >> 20) & 0x1ff;

  // First check if any value exist in the plist
  nvcard = FindCardWithIds(device_id, subsys_id);
  if (nvcard) {
    if (nvcard->VideoRam > 0) {
      // VideoRam * 1024 * 1024 == VideoRam << 20
      //videoRam = LShiftU64(nvcard->VideoRam, 20);
      videoRam = nvcard->VideoRam;
      model = nvcard->Model;
      n_ports = nvcard->VideoPorts;
      load_vbios = nvcard->LoadVBios;
      //DBG("mem assigned %ld\n", vram_size);
    }
  } else {

    // Amount of VRAM in Mb
    if (gSettings.VRAM != 0) {
      videoRam = gSettings.VRAM << 20;
    } else {
      videoRam = mem_detect(nvCardType, nvda_dev);
    }
  }

  if (gSettings.NvidiaGeneric) {
    // Get Model from the PCI
    //    model = get_nvidia_model(((nvda_dev->vendor_id << 16) | nvda_dev->device_id), subsystem);
    if (model == NULL) { // !nvcard->Model
      model = get_nvidia_model(device_id, subsys_id, NULL);
    }
  } else {

    for (j = 0; j < NGFX; j++) {
      if ((gGraphics[j].Vendor == Nvidia) && (gGraphics[j].DeviceID == nvda_dev->device_id)) {
        model = gGraphics[j].Model; //menu setting
        if (n_ports == 0) { // !nvcard->VideoPorts
          n_ports = gGraphics[j].Ports;
        }
        if (load_vbios == FALSE) { // !nvcard->LoadVBios
          load_vbios = gGraphics[j].LoadVBios;
        }
        break;
      }
    }
  }


  if (load_vbios) {
    UnicodeSPrint(FileName, 128, L"ROM\\10de_%04x_%04x_%04x.rom", nvda_dev->device_id, nvda_dev->subsys_id.subsys.vendor_id, nvda_dev->subsys_id.subsys.device_id);

    if (FileExists(OEMDir, FileName)) {
      DBG("Found specific VBIOS ROM file (10de_%04x_%04x_%04x.rom)\n", nvda_dev->device_id, nvda_dev->subsys_id.subsys.vendor_id, nvda_dev->subsys_id.subsys.device_id);

      Status = egLoadFile(OEMDir, FileName, &buffer, &bufferLen);
    } else {
      UnicodeSPrint(FileName, 128, L"ROM\\10de_%04x.rom", nvda_dev->device_id);
      if (FileExists(OEMDir, FileName)) {
        DBG("Found generic VBIOS ROM file (10de_%04x.rom)\n", nvda_dev->device_id);

        Status = egLoadFile(OEMDir, FileName, &buffer, &bufferLen);
      }
    }

    UnicodeSPrint(FileName, 128, L"\\EFI\\CLOVER\\ROM\\10de_%04x_%04x_%04x.rom", nvda_dev->device_id, nvda_dev->subsys_id.subsys.vendor_id, nvda_dev->subsys_id.subsys.device_id);
    if (EFI_ERROR(Status)) {
      if (FileExists(SelfRootDir, FileName)) {
        DBG("Found specific VBIOS ROM file (10de_%04x_%04x_%04x.rom)\n", nvda_dev->device_id, nvda_dev->subsys_id.subsys.vendor_id, nvda_dev->subsys_id.subsys.device_id);

        Status = egLoadFile(SelfRootDir, FileName, &buffer, &bufferLen);
      } else {
        UnicodeSPrint(FileName, 128, L"\\EFI\\CLOVER\\ROM\\10de_%04x.rom", nvda_dev->device_id);

        if (FileExists(SelfRootDir, FileName)) {
          DBG("Found generic VBIOS ROM file (10de_%04x.rom)\n", nvda_dev->device_id);

          Status = egLoadFile(SelfRootDir, FileName, &buffer, &bufferLen);
        }
      }
    }
  }

  if (EFI_ERROR(Status)) {
    rom = (__typeof__(rom))AllocateZeroPool(NVIDIA_ROM_SIZE+1);
    // PRAMIN first
    read_nVidia_PRAMIN(nvda_dev, rom, nvCardType);

    //DBG("%x%x\n", rom[0], rom[1]);
    rom_pci_header = NULL;

    if (rom[0] != 0x55 || rom[1] != 0xaa) {
      read_nVidia_PROM(nvda_dev, rom);
      if (rom[0] != 0x55 || rom[1] != 0xaa) {
        DBG("ERROR: Unable to locate nVidia Video BIOS\n");
        FreePool(rom);
        rom = NULL;
      }
    }
  }

  if (!rom){
    if (buffer) {
      if (buffer[0] != 0x55 && buffer[1] != 0xaa) {
        //DBG("buffer->size: %d\n", bufferLen);
        i = 0;
        while (i < bufferLen) {
          //DBG("%x%x\n", buffer[i], buffer[i+1]);
          if (buffer[i] == 0x55 && buffer[i+1] == 0xaa) {
            DBG(" header found at: %d\n", i);
            bufferLen -= i;
            rom = (__typeof__(rom))AllocateZeroPool(bufferLen);
            for (j = 0; j < bufferLen; j++) {
              rom[j] = buffer[i+j];
            }
            break;
          }
          i += 512;
        }
      }
      if (!rom) rom = buffer;
      RomAssigned = TRUE;
      DBG(" using loaded ROM image\n");
    } else {
      DBG(" there are no ROM loaded and no VBIOS read from hardware\n");
      //      return FALSE;
    }
  }

  if(rom) {

    if ((nvPatch = patch_nvidia_rom(rom)) == PATCH_ROM_FAILED) {
      DBG("ERROR: nVidia ROM Patching Failed!\n");
    }
    rom_pci_header = (option_rom_pci_header_t*)(rom + *(UINT16 *)&rom[24]);

    // check for 'PCIR' sig
    if (rom_pci_header->signature == 0x52494350) {
      if (rom_pci_header->device_id != nvda_dev->device_id) {
        // Get Model from the OpROM
        model = get_nvidia_model(((rom_pci_header->vendor_id << 16) | rom_pci_header->device_id), subsys_id, nvcard);
        //        DBG(model);
      }
    } else {
      DBG("nVidia incorrect PCI ROM signature: 0x%x\n", rom_pci_header->signature);
    }

    // get bios version

    // only search the first 384 bytes
    for (i = 0; i < 0x180; i++) {
      if (rom[i] == 0x0D && rom[i+1] == 0x0A) {
        crlf_count++;
        // second 0x0D0A was found, extract bios version
        if (crlf_count == 2) {
          if (rom[i-1] == 0x20) i--; // strip last " "

          for (version_start = i; version_start > (i-MAX_BIOS_VERSION_LENGTH); version_start--) {
            // find start
            if (rom[version_start] == 0x00) {
              version_start++;

              // strip "Version "
              if (AsciiStrnCmp((const CHAR8*)rom + version_start, "Version ", 8) == 0) {
                version_start += 8;
              }
              s = (CHAR8*)(rom + version_start);
              s1 = version_str;
              while ((*s > ' ') && (*s < 'z') && ((INTN)(s1 - version_str) < MAX_BIOS_VERSION_LENGTH)) {
                *s1++ = *s++;
              }
              *s1 = 0;
              DBG("version %a\n", version_str);
              break;
            }
          }
          break;
        }
      }
    }
  } else {
    AsciiSPrint(version_str, sizeof(version_str), "1.0");
  }

  DBG("nVidia %a ", model);
  DBG(" %dMB NV%02x [%04x:%04x] :: %a => device #%d\n", (UINT32)(RShiftU64(videoRam, 20)),
      nvCardType, nvda_dev->vendor_id, nvda_dev->device_id,
      devicepath, devices_number);

  if (!device_inject_string) {
    device_inject_string = devprop_create_string();
  }

  if (nvda_dev && !nvda_dev->used) {
    device = devprop_add_device_pci(device_inject_string, nvda_dev, NULL);
    nvda_dev->used = TRUE;
  }

  DBG("Nvidia: VideoPorts:");
  if (n_ports > 0) {
    DBG(" user defined (GUI-menu): %d\n", n_ports);
  } else if (gSettings.VideoPorts > 0) {
    n_ports = gSettings.VideoPorts;
    DBG(" user defined from config.plist: %d\n", n_ports);
  } else {
    n_ports = 2; //default
    DBG(" undefined, default to: %d\n", n_ports);
  }

  if (gSettings.NvidiaNoEFI) {
    devprop_add_value(device, "NVDA,noEFI", (UINT8*)"true", 5);
  }

  //There are custom properties, injected if set by user
  if (gSettings.NvidiaSingle && (devices_number >=1)) {
    DBG("NVidia: NvidiaSingle :: skip injecting other then first card\n");
    goto done;
  }

  if (gSettings.NrAddProperties != 0xFFFE) {
    for (i = 0; i < gSettings.NrAddProperties; i++) {
      if (gSettings.AddProperties[i].Device != DEV_NVIDIA) {
        continue;
      }
      Injected = TRUE;

      if (!gSettings.AddProperties[i].MenuItem.BValue) {
        //DBG("  disabled property Key: %a, len: %d\n", gSettings.AddProperties[i].Key, gSettings.AddProperties[i].ValueLen);
      } else {
        devprop_add_value(device,
                          gSettings.AddProperties[i].Key,
                          (UINT8*)gSettings.AddProperties[i].Value,
                          gSettings.AddProperties[i].ValueLen);
        //DBG("  added property Key: %a, len: %d\n", gSettings.AddProperties[i].Key, gSettings.AddProperties[i].ValueLen);
      }
    }
    if (Injected) {
      DBG("custom NVIDIA properties injected, continue\n");
      //return TRUE;
    }
  }

  if (gSettings.FakeNVidia) {
    UINT32 FakeID = gSettings.FakeNVidia >> 16;
    DBG("NVidia: FakeID %x:%x\n",gSettings.FakeNVidia & 0xFFFF, FakeID);
    devprop_add_value(device, "device-id", (UINT8*)&FakeID, 4);
    FakeID = gSettings.FakeNVidia & 0xFFFF;
    devprop_add_value(device, "vendor-id", (UINT8*)&FakeID, 4);
  }

  if (gSettings.NVCAP[0] != 0) {
    devprop_add_value(device, "NVCAP", &gSettings.NVCAP[0], NVCAP_LEN);
    DBG("set NVCAP: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x\n",
        gSettings.NVCAP[0], gSettings.NVCAP[1], gSettings.NVCAP[2], gSettings.NVCAP[3],
        gSettings.NVCAP[4], gSettings.NVCAP[5], gSettings.NVCAP[6], gSettings.NVCAP[7],
        gSettings.NVCAP[8], gSettings.NVCAP[9], gSettings.NVCAP[10], gSettings.NVCAP[11],
        gSettings.NVCAP[12], gSettings.NVCAP[13], gSettings.NVCAP[14], gSettings.NVCAP[15],
        gSettings.NVCAP[16], gSettings.NVCAP[17], gSettings.NVCAP[18], gSettings.NVCAP[19]);
  }

  if (gSettings.InjectEDID && gSettings.CustomEDID) {
    devprop_add_value(device, "AAPL00,override-no-connect", gSettings.CustomEDID, 128);
  }

  if ((devices_number == 1) &&
      ((gSettings.BootDisplay >= 0) && (gSettings.BootDisplay < (INT8)n_ports))) {
    CHAR8 nkey[24];
    AsciiSPrint(nkey, 24, "@%d,AAPL,boot-display", gSettings.BootDisplay);
    devprop_add_value(device, nkey, (UINT8*)&boot_display, 4);
    DBG("Nvidia: BootDisplay: %d\n", gSettings.BootDisplay);
  }

  //there are default or calculated properties, can be skipped
  //if (gSettings.NoDefaultProperties) {
  //  DBG("Nvidia: no default properties injected\n");
 //   goto done;
  //}

  if (gSettings.BootDisplay < 0) {
    // if not set this is default property
    devprop_add_value(device, "@0,AAPL,boot-display", (UINT8*)&boot_display, 4);
  }/* else {
    DBG("Nvidia: BootDisplay: %x\n", gSettings.BootDisplay);
    }*/

  if (gSettings.UseIntelHDMI) {
    devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-2", 10);
  } else {
    devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
  }

  if (!gSettings.NoDefaultProperties) {
	  if (videoRam != 0) {
		  devprop_add_value(device, "VRAM,totalsize", (UINT8*)&videoRam, 8);
	  }
	  else {
		  DBG("Warning! VideoRAM is not detected and not set\n");
	  }
	  devprop_add_nvidia_template(device, n_ports);
  }

  //add HDMI Audio back to nvidia
  //http://forge.voodooprojects.org/p/chameleon/issues/67/
  //AsciiSPrint(nkey, 24, "@%d,connector-type", pnum);
  //devprop_add_value(device, nkey, connector_type_1, 4);
  //end Nvidia HDMI Audio

  if (nvPatch == PATCH_ROM_SUCCESS_HAS_LVDS) {
    UINT8 built_in = 0x01;
    devprop_add_value(device, "@0,built-in", &built_in, 1);
    // HDMI is not LVDS
    devprop_add_value(device, "@1,connector-type", connector_type_1, 4);
  } else {
    devprop_add_value(device, "@0,connector-type", connector_type_1, 4);
  }

  devprop_add_value(device, "NVPM", default_NVPM, NVPM_LEN);
  devprop_add_value(device, "model", (UINT8*)model, (UINT32)AsciiStrLen(model));
  devprop_add_value(device, "rom-revision", (UINT8*)version_str, (UINT32)AsciiStrLen(version_str));
  if (gMobile) {
    DBG("Nvidia Mobile backlight\n");
    devprop_add_value(device, "AAPL,backlight-control", (UINT8*)&boot_display, 4);
    devprop_add_value(device, "AAPL,HasLid", (UINT8*)&boot_display, 4);
    devprop_add_value(device, "AAPL,HasPanel", (UINT8*)&boot_display, 4);
    devprop_add_value(device, "@0,backlight-control", (UINT8*)&boot_display, 4);
    devprop_add_value(device, "@0,pwm-info", pwm_info, PWM_LEN);
  }


done:
  devices_number++;
  FreePool(version_str);
  if (buffer) {
    FreePool(buffer);
  }
  if (!RomAssigned) {
    FreePool(rom);
  }
  return TRUE;
}

