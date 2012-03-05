/*
 *	NVidia injector
 *
 *	Copyright (C) 2009	Jasmin Fazlic, iNDi
 *
 *	NVidia injector is FreePool software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the FreePool Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	NVidia driver and injector is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with NVidia injector.	 If not, see <http://www.gnu.org/licenses/>.
 */ 
/*
 * Alternatively you can choose to comply with APSL
 */
 
 
/*
 * DCB-Table parsing is based on software (nouveau driver) originally distributed under following license:
 *
 *
 * Copyright 2005-2006 Erik Waling
 * Copyright 2006 Stephane Marchesin
 * Copyright 2007-2009 Stuart Bennett
 *
 * Permission is hereby granted, FreePool of CHAR8ge, to any person obtaining a
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

#include "Platform.h"
#include "nvidia.h"
#include "device_inject.h"

#ifndef DEBUG_NVIDIA
#define DEBUG_NVIDIA 1
#endif

#if DEBUG_NVIDIA == 2
#define DBG(x...) AsciiPrint(x)
#elif DEBUG_NVIDIA == 1
#define DBG(x...) MsgLog(x)
#else
#define DBG(x...)
#endif


#define NVIDIA_ROM_SIZE				0x10000
#define PATCH_ROM_SUCCESS			1
#define PATCH_ROM_SUCCESS_HAS_LVDS	2
#define PATCH_ROM_FAILED			0
#define MAX_NUM_DCB_ENTRIES			16
#define TYPE_GROUPED				0xff




extern UINT32 devices_number;

const CHAR8 *nvidia_compatible_0[]	=	{ "@0,compatible",	"NVDA,NVMac"	 };
const CHAR8 *nvidia_compatible_1[]	=	{ "@1,compatible",	"NVDA,NVMac"	 };
const CHAR8 *nvidia_device_type_0[]	=	{ "@0,device_type", "display"		 };
const CHAR8 *nvidia_device_type_1[]	=	{ "@1,device_type", "display"		 };
const CHAR8 *nvidia_device_type[]	=	{ "device_type",	"NVDA,Parent"	 };
const CHAR8 *nvidia_name_0[]			=	{ "@0,name",		"NVDA,Display-A" };
const CHAR8 *nvidia_name_1[]			=	{ "@1,name",		"NVDA,Display-B" };
const CHAR8 *nvidia_slot_name[]		=	{ "AAPL,slot-name", "Slot-1"		 };

static UINT8 default_NVCAP[]= {
	0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a,
	0x00, 0x00, 0x00, 0x00
};

#define NVCAP_LEN ( sizeof(default_NVCAP) / sizeof(UINT8) )

static UINT8 default_dcfg_0[]= {0x03, 0x01, 0x03, 0x00};
static UINT8 default_dcfg_1[]= {0xff, 0xff, 0x00, 0x01};

#define DCFG0_LEN ( sizeof(default_dcfg_0) / sizeof(UINT8) )
#define DCFG1_LEN ( sizeof(default_dcfg_1) / sizeof(UINT8) )

static UINT8 default_NVPM[]= {
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};

#define NVPM_LEN ( sizeof(default_NVPM) / sizeof(UINT8) )

static struct nv_chipsets_t NVKnownChipsets[] = {
	{ 0x00000000, "Unknown" },
// temporary placement
	{ 0x10DE0DF4, "GeForce GT 450M" }, //Azi + issue #99
	{ 0x10DE1251, "GeForce GTX 560M" }, // Asus G74SX
//========================================
	// 0040 - 004F	
	{ 0x10DE0040, "GeForce 6800 Ultra" },
	{ 0x10DE0041, "GeForce 6800" },
	{ 0x10DE0042, "GeForce 6800 LE" },
	{ 0x10DE0043, "GeForce 6800 XE" },
	{ 0x10DE0044, "GeForce 6800 XT" },
	{ 0x10DE0045, "GeForce 6800 GT" },
	{ 0x10DE0046, "GeForce 6800 GT" },
	{ 0x10DE0047, "GeForce 6800 GS" },
	{ 0x10DE0048, "GeForce 6800 XT" },
	{ 0x10DE004D, "Quadro FX 3400" },
	{ 0x10DE004E, "Quadro FX 4000" },
	// 0050 - 005F
	// 0060 - 006F	
	// 0070 - 007F	
	// 0080 - 008F	
	// 0090 - 009F	
	{ 0x10DE0090, "GeForce 7800 GTX" },
	{ 0x10DE0091, "GeForce 7800 GTX" },
	{ 0x10DE0092, "GeForce 7800 GT" },
	{ 0x10DE0093, "GeForce 7800 GS" },
	{ 0x10DE0095, "GeForce 7800 SLI" },
	{ 0x10DE0098, "GeForce Go 7800" },
	{ 0x10DE0099, "GeForce Go 7800 GTX" },
	{ 0x10DE009D, "Quadro FX 4500" },
	// 00A0 - 00AF	
	// 00B0 - 00BF	
	// 00C0 - 00CF	
	{ 0x10DE00C0, "GeForce 6800 GS" },
	{ 0x10DE00C1, "GeForce 6800" },
	{ 0x10DE00C2, "GeForce 6800 LE" },
	{ 0x10DE00C3, "GeForce 6800 XT" },
	{ 0x10DE00C8, "GeForce Go 6800" },
	{ 0x10DE00C9, "GeForce Go 6800 Ultra" },
	{ 0x10DE00CC, "Quadro FX Go1400" },
	{ 0x10DE00CD, "Quadro FX 3450/4000 SDI" },
	{ 0x10DE00CE, "Quadro FX 1400" },
	// 00D0 - 00DF	
	// 00E0 - 00EF	
	// 00F0 - 00FF	
	{ 0x10DE00F1, "GeForce 6600 GT" },
	{ 0x10DE00F2, "GeForce 6600" },
	{ 0x10DE00F3, "GeForce 6200" },
	{ 0x10DE00F4, "GeForce 6600 LE" },
	{ 0x10DE00F5, "GeForce 7800 GS" },
	{ 0x10DE00F6, "GeForce 6800 GS/XT" },
	{ 0x10DE00F8, "Quadro FX 3400/4400" },
	{ 0x10DE00F9, "GeForce 6800 Series GPU" },
	// 0100 - 010F	
	// 0110 - 011F	
	// 0120 - 012F	
	// 0130 - 013F	
	// 0140 - 014F	
	{ 0x10DE0140, "GeForce 6600 GT" },
	{ 0x10DE0141, "GeForce 6600" },
	{ 0x10DE0142, "GeForce 6600 LE" },
	{ 0x10DE0143, "GeForce 6600 VE" },
	{ 0x10DE0144, "GeForce Go 6600" },
	{ 0x10DE0145, "GeForce 6610 XL" },
	{ 0x10DE0146, "GeForce Go 6600 TE/6200 TE" },
	{ 0x10DE0147, "GeForce 6700 XL" },
	{ 0x10DE0148, "GeForce Go 6600" },
	{ 0x10DE0149, "GeForce Go 6600 GT" },
	{ 0x10DE014A, "Quadro NVS 440" },
	{ 0x10DE014C, "Quadro FX 550" },
	{ 0x10DE014D, "Quadro FX 550" },
	{ 0x10DE014E, "Quadro FX 540" },
	{ 0x10DE014F, "GeForce 6200" },
	// 0150 - 015F	
	// 0160 - 016F	
	{ 0x10DE0160, "GeForce 6500" },
	{ 0x10DE0161, "GeForce 6200 TurboCache(TM)" },
	{ 0x10DE0162, "GeForce 6200SE TurboCache(TM)" },
	{ 0x10DE0163, "GeForce 6200 LE" },
	{ 0x10DE0164, "GeForce Go 6200" },
	{ 0x10DE0165, "Quadro NVS 285" },
	{ 0x10DE0166, "GeForce Go 6400" },
	{ 0x10DE0167, "GeForce Go 6200" },
	{ 0x10DE0168, "GeForce Go 6400" },
	{ 0x10DE0169, "GeForce 6250" },
	{ 0x10DE016A, "GeForce 7100 GS" },
	// 0170 - 017F	
	// 0180 - 018F	
	// 0190 - 019F		
	{ 0x10DE0191, "GeForce 8800 GTX" },
	{ 0x10DE0193, "GeForce 8800 GTS" },
	{ 0x10DE0194, "GeForce 8800 Ultra" },
	{ 0x10DE0197, "Tesla C870" },
	{ 0x10DE019D, "Quadro FX 5600" },
	{ 0x10DE019E, "Quadro FX 4600" },
	// 01A0 - 01AF	
	// 01B0 - 01BF	
	// 01C0 - 01CF	
	// 01D0 - 01DF		
	{ 0x10DE01D0, "GeForce 7350 LE" },
	{ 0x10DE01D1, "GeForce 7300 LE" },
	{ 0x10DE01D2, "GeForce 7550 LE" },
	{ 0x10DE01D3, "GeForce 7300 SE/7200 GS" },
	{ 0x10DE01D6, "GeForce Go 7200" },
	{ 0x10DE01D7, "GeForce Go 7300" },
	{ 0x10DE01D8, "GeForce Go 7400" },
	{ 0x10DE01D9, "GeForce Go 7400 GS" },
	{ 0x10DE01DA, "Quadro NVS 110M" },
	{ 0x10DE01DB, "Quadro NVS 120M" },
	{ 0x10DE01DC, "Quadro FX 350M" },
	{ 0x10DE01DD, "GeForce 7500 LE" },
	{ 0x10DE01DE, "Quadro FX 350" },
	{ 0x10DE01DF, "GeForce 7300 GS" },
	// 01E0 - 01EF	
	// 01F0 - 01FF
	// 0200 - 020F	
	// 0210 - 021F	
	{ 0x10DE0211, "GeForce 6800" },
	{ 0x10DE0212, "GeForce 6800 LE" },
	{ 0x10DE0215, "GeForce 6800 GT" },
	{ 0x10DE0218, "GeForce 6800 XT" },
	// 0220 - 022F	
	{ 0x10DE0221, "GeForce 6200" },
	{ 0x10DE0222, "GeForce 6200 A-LE" },
	// 0230 - 023F	
	// 0240 - 024F	
	{ 0x10DE0240, "GeForce 6150" },
	{ 0x10DE0241, "GeForce 6150 LE" },
	{ 0x10DE0242, "GeForce 6100" },
	{ 0x10DE0244, "GeForce Go 6150" },
	{ 0x10DE0245, "Quadro NVS 210S / GeForce 6150LE" },
	{ 0x10DE0247, "GeForce Go 6100" },
	// 0250 - 025F	
	// 0260 - 026F	
	// 0270 - 027F	
	// 0280 - 028F		
	// 0290 - 029F	
	{ 0x10DE0290, "GeForce 7900 GTX" },
	{ 0x10DE0291, "GeForce 7900 GT/GTO" },
	{ 0x10DE0292, "GeForce 7900 GS" },
	{ 0x10DE0293, "GeForce 7950 GX2" },
	{ 0x10DE0294, "GeForce 7950 GX2" },
	{ 0x10DE0295, "GeForce 7950 GT" },
	{ 0x10DE0298, "GeForce Go 7900 GS" },
	{ 0x10DE0299, "GeForce Go 7900 GTX" },
	{ 0x10DE029A, "Quadro FX 2500M" },
	{ 0x10DE029B, "Quadro FX 1500M" },
	{ 0x10DE029C, "Quadro FX 5500" },
	{ 0x10DE029D, "Quadro FX 3500" },
	{ 0x10DE029E, "Quadro FX 1500" },
	{ 0x10DE029F, "Quadro FX 4500 X2" },
	// 02A0 - 02AF	
	// 02B0 - 02BF	
	// 02C0 - 02CF	
	// 02D0 - 02DF		
	// 02E0 - 02EF		
	{ 0x10DE02E0, "GeForce 7600 GT" },
	{ 0x10DE02E1, "GeForce 7600 GS" },
	{ 0x10DE02E2, "GeForce 7300 GT" },
	{ 0x10DE02E3, "GeForce 7900 GS" },
	{ 0x10DE02E4, "GeForce 7950 GT" },
	// 02F0 - 02FF		
	// 0300 - 030F		
	{ 0x10DE0301, "GeForce FX 5800 Ultra" },
	{ 0x10DE0302, "GeForce FX 5800" },
	{ 0x10DE0308, "Quadro FX 2000" },
	{ 0x10DE0309, "Quadro FX 1000" },
	// 0310 - 031F		
	{ 0x10DE0311, "GeForce FX 5600 Ultra" },
	{ 0x10DE0312, "GeForce FX 5600" },
	{ 0x10DE0314, "GeForce FX 5600XT" },
	{ 0x10DE031A, "GeForce FX Go5600" },
	{ 0x10DE031B, "GeForce FX Go5650" },
	{ 0x10DE031C, "Quadro FX Go700" },
	// 0320 - 032F		
	{ 0x10DE0324, "GeForce FX Go5200" },
	{ 0x10DE0325, "GeForce FX Go5250" },
	{ 0x10DE0326, "GeForce FX 5500" },
	{ 0x10DE0328, "GeForce FX Go5200 32M/64M" },
	{ 0x10DE032A, "Quadro NVS 55/280 PCI" },
	{ 0x10DE032B, "Quadro FX 500/600 PCI" },
	{ 0x10DE032C, "GeForce FX Go53xx Series" },
	{ 0x10DE032D, "GeForce FX Go5100" },
	// 0330 - 033F
	{ 0x10DE0330, "GeForce FX 5900 Ultra" },
	{ 0x10DE0331, "GeForce FX 5900" },
	{ 0x10DE0332, "GeForce FX 5900XT" },
	{ 0x10DE0333, "GeForce FX 5950 Ultra" },
	{ 0x10DE0334, "GeForce FX 5900ZT" },
	{ 0x10DE0338, "Quadro FX 3000" },
	{ 0x10DE033F, "Quadro FX 700" },
	// 0340 - 034F
	{ 0x10DE0341, "GeForce FX 5700 Ultra" },
	{ 0x10DE0342, "GeForce FX 5700" },
	{ 0x10DE0343, "GeForce FX 5700LE" },
	{ 0x10DE0344, "GeForce FX 5700VE" },
	{ 0x10DE0347, "GeForce FX Go5700" },
	{ 0x10DE0348, "GeForce FX Go5700" },
	{ 0x10DE034C, "Quadro FX Go1000" },
	{ 0x10DE034E, "Quadro FX 1100" },
	// 0350 - 035F	
	// 0360 - 036F	
	// 0370 - 037F	
	// 0380 - 038F			
	{ 0x10DE038B, "GeForce 7650 GS" },
	// 0390 - 039F
	{ 0x10DE0390, "GeForce 7650 GS" },
	{ 0x10DE0391, "GeForce 7600 GT" },
	{ 0x10DE0392, "GeForce 7600 GS" },
	{ 0x10DE0393, "GeForce 7300 GT" },
	{ 0x10DE0394, "GeForce 7600 LE" },
	{ 0x10DE0395, "GeForce 7300 GT" },
	{ 0x10DE0397, "GeForce Go 7700" },
	{ 0x10DE0398, "GeForce Go 7600" },
	{ 0x10DE0399, "GeForce Go 7600 GT"},
	{ 0x10DE039A, "Quadro NVS 300M" },
	{ 0x10DE039B, "GeForce Go 7900 SE" },
	{ 0x10DE039C, "Quadro FX 550M" },
	{ 0x10DE039E, "Quadro FX 560" },
	// 03A0 - 03AF	
	// 03B0 - 03BF	
	// 03C0 - 03CF	
	// 03D0 - 03DF			
	{ 0x10DE03D0, "GeForce 6150SE nForce 430" },
	{ 0x10DE03D1, "GeForce 6100 nForce 405" },
	{ 0x10DE03D2, "GeForce 6100 nForce 400" },
	{ 0x10DE03D5, "GeForce 6100 nForce 420" },
	{ 0x10DE03D6, "GeForce 7025 / nForce 630a" },
	// 03E0 - 03EF
	// 03F0 - 03FF
	// 0400 - 040F		
	{ 0x10DE0400, "GeForce 8600 GTS" },
	{ 0x10DE0401, "GeForce 8600 GT" },
	{ 0x10DE0402, "GeForce 8600 GT" },
	{ 0x10DE0403, "GeForce 8600 GS" },
	{ 0x10DE0404, "GeForce 8400 GS" },
	{ 0x10DE0405, "GeForce 9500M GS" },
	{ 0x10DE0406, "GeForce 8300 GS" },
	{ 0x10DE0407, "GeForce 8600M GT" },
	{ 0x10DE0408, "GeForce 9650M GS" },
	{ 0x10DE0409, "GeForce 8700M GT" },
	{ 0x10DE040A, "Quadro FX 370" },
	{ 0x10DE040B, "Quadro NVS 320M" },
	{ 0x10DE040C, "Quadro FX 570M" },
	{ 0x10DE040D, "Quadro FX 1600M" },
	{ 0x10DE040E, "Quadro FX 570" },
	{ 0x10DE040F, "Quadro FX 1700" },
	// 0410 - 041F	
	{ 0x10DE0410, "GeForce GT 330" },
	// 0420 - 042F	
	{ 0x10DE0420, "GeForce 8400 SE" },
	{ 0x10DE0421, "GeForce 8500 GT" },
	{ 0x10DE0422, "GeForce 8400 GS" },
	{ 0x10DE0423, "GeForce 8300 GS" },
	{ 0x10DE0424, "GeForce 8400 GS" },
	{ 0x10DE0425, "GeForce 8600M GS" },
	{ 0x10DE0426, "GeForce 8400M GT" },
	{ 0x10DE0427, "GeForce 8400M GS" },
	{ 0x10DE0428, "GeForce 8400M G" },
	{ 0x10DE0429, "Quadro NVS 140M" },
	{ 0x10DE042A, "Quadro NVS 130M" },
	{ 0x10DE042B, "Quadro NVS 135M" },
	{ 0x10DE042C, "GeForce 9400 GT" },
	{ 0x10DE042D, "Quadro FX 360M" },
	{ 0x10DE042E, "GeForce 9300M G" },
	{ 0x10DE042F, "Quadro NVS 290" },
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
	// 04D0 - 04DF
	// 04E0 - 04EF
	// 04F0 - 04FF
	// 0500 - 050F	
	// 0510 - 051F	
	// 0520 - 052F	
	// 0530 - 053F	
	{ 0x10DE053A, "GeForce 7050 PV / nForce 630a" },
	{ 0x10DE053B, "GeForce 7050 PV / nForce 630a" },
	{ 0x10DE053E, "GeForce 7025 / nForce 630a" },
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
	{ 0x10DE05E0, "GeForce GTX 295" },
	{ 0x10DE05E1, "GeForce GTX 280" },
	{ 0x10DE05E2, "GeForce GTX 260" },
	{ 0x10DE05E3, "GeForce GTX 285" },
	{ 0x10DE05E6, "GeForce GTX 275" },
	{ 0x10DE05EA, "GeForce GTX 260" },
	{ 0x10DE05EB, "GeForce GTX 295" },
	{ 0x10DE05ED, "Quadroplex 2200 D2" },
	// 05F0 - 05FF		
	{ 0x10DE05F8, "Quadroplex 2200 S4" },
	{ 0x10DE05F9, "Quadro CX" },
	{ 0x10DE05FD, "Quadro FX 5800" },
	{ 0x10DE05FE, "Quadro FX 4800" },
	{ 0x10DE05FF, "Quadro FX 3800" },
	// 0600 - 060F	
	{ 0x10DE0600, "GeForce 8800 GTS 512" },
	{ 0x10DE0601, "GeForce 9800 GT" },
	{ 0x10DE0602, "GeForce 8800 GT" },
	{ 0x10DE0603, "GeForce GT 230" },
	{ 0x10DE0604, "GeForce 9800 GX2" },
	{ 0x10DE0605, "GeForce 9800 GT" },
	{ 0x10DE0606, "GeForce 8800 GS" },
	{ 0x10DE0607, "GeForce GTS 240" },
	{ 0x10DE0608, "GeForce 9800M GTX" },
	{ 0x10DE0609, "GeForce 8800M GTS" },
	{ 0x10DE060A, "GeForce GTX 280M" },
	{ 0x10DE060B, "GeForce 9800M GT" },
	{ 0x10DE060C, "GeForce 8800M GTX" },
	{ 0x10DE060D, "GeForce 8800 GS" },
	{ 0x10DE060F, "GeForce GTX 285M" },
	// 0610 - 061F	
	{ 0x10DE0610, "GeForce 9600 GSO" },
	{ 0x10DE0611, "GeForce 8800 GT" },
	{ 0x10DE0612, "GeForce 9800 GTX" },
	{ 0x10DE0613, "GeForce 9800 GTX+" },
	{ 0x10DE0614, "GeForce 9800 GT" },
	{ 0x10DE0615, "GeForce GTS 250" },
	{ 0x10DE0617, "GeForce 9800M GTX" },
	{ 0x10DE0618, "GeForce GTX 260M" },
	{ 0x10DE0619, "Quadro FX 4700 X2" },
	{ 0x10DE061A, "Quadro FX 3700" },
	{ 0x10DE061B, "Quadro VX 200" },
	{ 0x10DE061C, "Quadro FX 3600M" },
	{ 0x10DE061D, "Quadro FX 2800M" },
	{ 0x10DE061F, "Quadro FX 3800M" },
	// 0620 - 062F	
	{ 0x10DE0622, "GeForce 9600 GT" },
	{ 0x10DE0623, "GeForce 9600 GS" },
	{ 0x10DE0625, "GeForce 9600 GSO 512"},
	{ 0x10DE0626, "GeForce GT 130" },
	{ 0x10DE0627, "GeForce GT 140" },
	{ 0x10DE0628, "GeForce 9800M GTS" },
	{ 0x10DE062A, "GeForce 9700M GTS" },
	{ 0x10DE062C, "GeForce 9800M GTS" },
	{ 0x10DE062D, "GeForce 9600 GT" },
	{ 0x10DE062E, "GeForce 9600 GT" },
	// 0630 - 063F	
	{ 0x10DE0631, "GeForce GTS 160M" },
	{ 0x10DE0632, "GeForce GTS 150M" },
	{ 0x10DE0635, "GeForce 9600 GSO" },
	{ 0x10DE0637, "GeForce 9600 GT" },
	{ 0x10DE0638, "Quadro FX 1800" },
	{ 0x10DE063A, "Quadro FX 2700M" },
	// 0640 - 064F	
	{ 0x10DE0640, "GeForce 9500 GT" },
	{ 0x10DE0641, "GeForce 9400 GT" },
	{ 0x10DE0642, "GeForce 8400 GS" },
	{ 0x10DE0643, "GeForce 9500 GT" },
	{ 0x10DE0644, "GeForce 9500 GS" },
	{ 0x10DE0645, "GeForce 9500 GS" },
	{ 0x10DE0646, "GeForce GT 120" },
	{ 0x10DE0647, "GeForce 9600M GT" },
	{ 0x10DE0648, "GeForce 9600M GS" },
	{ 0x10DE0649, "GeForce 9600M GT" },
	{ 0x10DE064A, "GeForce 9700M GT" },
	{ 0x10DE064B, "GeForce 9500M G" },
	{ 0x10DE064C, "GeForce 9650M GT" },
	// 0650 - 065F		
	{ 0x10DE0651, "GeForce G 110M" },
	{ 0x10DE0652, "GeForce GT 130M" },
	{ 0x10DE0653, "GeForce GT 120M" },
	{ 0x10DE0654, "GeForce GT 220M" },
	{ 0x10DE0656, "GeForce 9650 S" },
	{ 0x10DE0658, "Quadro FX 380" },
	{ 0x10DE0659, "Quadro FX 580" },
	{ 0x10DE065A, "Quadro FX 1700M" },
	{ 0x10DE065B, "GeForce 9400 GT" },
	{ 0x10DE065C, "Quadro FX 770M" },
	{ 0x10DE065F, "GeForce G210" },
	// 0660 - 066F
	// 0670 - 067F
	// 0680 - 068F
	// 0690 - 069F
	// 06A0 - 06AF
	// 06B0 - 06BF
	// 06C0 - 06CF
	{ 0x10DE06C0, "GeForce GTX 480" },
	{ 0x10DE06C3, "GeForce GTX D12U" },
	{ 0x10DE06C4, "GeForce GTX 465" },
	{ 0x10DE06CA, "GeForce GTX 480M" },
	{ 0x10DE06CD, "GeForce GTX 470" },
	// 06D0 - 06DF
	{ 0x10DE06D1, "Tesla C2050" },	// TODO: sub-device id: 0x0771
	{ 0x10DE06D1, "Tesla C2070" },	// TODO: sub-device id: 0x0772
	{ 0x10DE06D2, "Tesla M2070" },
	{ 0x10DE06D8, "Quadro 6000" },
	{ 0x10DE06D9, "Quadro 5000" },
	{ 0x10DE06DA, "Quadro 5000M" },
	{ 0x10DE06DC, "Quadro 6000" },
	{ 0x10DE06DD, "Quadro 4000" },
	{ 0x10DE06DE, "Tesla M2050" },	// TODO: sub-device id: 0x0846
	{ 0x10DE06DE, "Tesla M2070" },	// TODO: sub-device id: ?	
	// 0x10DE06DE also applies to misc S2050, X2070, M2050, M2070
	// 06E0 - 06EF	
	{ 0x10DE06E0, "GeForce 9300 GE" },
	{ 0x10DE06E1, "GeForce 9300 GS" },
	{ 0x10DE06E2, "GeForce 8400" },
	{ 0x10DE06E3, "GeForce 8400 SE" },
	{ 0x10DE06E4, "GeForce 8400 GS" },
	{ 0x10DE06E5, "GeForce 9300M GS" },
	{ 0x10DE06E6, "GeForce G100" },
	{ 0x10DE06E7, "GeForce 9300 SE" },
	{ 0x10DE06E8, "GeForce 9200M GS" },
	{ 0x10DE06E9, "GeForce 9300M GS" },
	{ 0x10DE06EA, "Quadro NVS 150M" },
	{ 0x10DE06EB, "Quadro NVS 160M" },
	{ 0x10DE06EC, "GeForce G 105M" },
	{ 0x10DE06EF, "GeForce G 103M" },
	// 06F0 - 06FF	
	{ 0x10DE06F8, "Quadro NVS 420" },
	{ 0x10DE06F9, "Quadro FX 370 LP" },
	{ 0x10DE06FA, "Quadro NVS 450" },
	{ 0x10DE06FB, "Quadro FX 370M" },
	{ 0x10DE06FD, "Quadro NVS 295" },
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
	{ 0x10DE07E0, "GeForce 7150 / nForce 630i" },
	{ 0x10DE07E1, "GeForce 7100 / nForce 630i" },
	{ 0x10DE07E2, "GeForce 7050 / nForce 630i" },
	{ 0x10DE07E3, "GeForce 7050 / nForce 610i" },
	{ 0x10DE07E5, "GeForce 7050 / nForce 620i" },
	// 07F0 - 07FF	
	// 0800 - 080F
	// 0810 - 081F
	// 0820 - 082F
	// 0830 - 083F
	// 0840 - 084F
	{ 0x10DE0844, "GeForce 9100M G" },
	{ 0x10DE0845, "GeForce 8200M G" },
	{ 0x10DE0846, "GeForce 9200" },
	{ 0x10DE0847, "GeForce 9100" },
	{ 0x10DE0848, "GeForce 8300" },
	{ 0x10DE0849, "GeForce 8200" },
	{ 0x10DE084A, "nForce 730a" },
	{ 0x10DE084B, "GeForce 9200" },
	{ 0x10DE084C, "nForce 980a/780a SLI" },
	{ 0x10DE084D, "nForce 750a SLI" },
	{ 0x10DE084F, "GeForce 8100 / nForce 720a" },
	// 0850 - 085F
	// 0860 - 086F	
	{ 0x10DE0860, "GeForce 9400" },
	{ 0x10DE0861, "GeForce 9400" },
	{ 0x10DE0862, "GeForce 9400M G" },
	{ 0x10DE0863, "GeForce 9400M" },
	{ 0x10DE0864, "GeForce 9300" },
	{ 0x10DE0865, "ION" },
	{ 0x10DE0866, "GeForce 9400M G" },
	{ 0x10DE0867, "GeForce 9400" },
	{ 0x10DE0868, "nForce 760i SLI" },
	{ 0x10DE086A, "GeForce 9400" },
	{ 0x10DE086C, "GeForce 9300 / nForce 730i" },
	{ 0x10DE086D, "GeForce 9200" },
	{ 0x10DE086E, "GeForce 9100M G" },
	{ 0x10DE086F, "GeForce 8200M G" },
	// 0870 - 087F	
	{ 0x10DE0870, "GeForce 9400M" },
	{ 0x10DE0871, "GeForce 9200" },
	{ 0x10DE0872, "GeForce G102M" },
	{ 0x10DE0873, "GeForce G102M" },
	{ 0x10DE0874, "ION 9300M" },	
	{ 0x10DE0876, "ION" },
	{ 0x10DE087A, "GeForce 9400" },
	{ 0x10DE087D, "ION 9400M" },
	{ 0x10DE087E, "ION LE" },
	{ 0x10DE087F, "ION LE" },
	// 0880 - 088F
	// 0890 - 089F
	// 08A0 - 08AF
	// 08B0 - 08BF
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
	// 0A10 - 0A1F
	// 0A20 - 0A2F
	{ 0x10DE0A20, "GeForce GT220" },
	{ 0x10DE0A22, "GeForce 315" },
	{ 0x10DE0A23, "GeForce 210" },
	{ 0x10DE0A28, "GeForce GT 230M" },
	{ 0x10DE0A29, "GeForce GT 330M" },
	{ 0x10DE0A2A, "GeForce GT 230M" },
	{ 0x10DE0A2B, "GeForce GT 330M" },
	{ 0x10DE0A2C, "NVS 5100M" },
	{ 0x10DE0A2D, "GeForce GT 320M" },	
	// 0A30 - 0A3F	
	{ 0x10DE0A34, "GeForce GT 240M" },
	{ 0x10DE0A35, "GeForce GT 325M" },
	{ 0x10DE0A3C, "Quadro FX 880M" },
	// 0A40 - 0A4F
	// 0A50 - 0A5F
	// 0A60 - 0A6F
	{ 0x10DE0A60, "GeForce G210" },
	{ 0x10DE0A62, "GeForce 205" },
	{ 0x10DE0A63, "GeForce 310" },
	{ 0x10DE0A64, "ION" },
	{ 0x10DE0A65, "GeForce 210" },
	{ 0x10DE0A66, "GeForce 310" },
	{ 0x10DE0A67, "GeForce 315" },
	{ 0x10DE0A68, "GeForce G105M" },
	{ 0x10DE0A69, "GeForce G105M" },
	{ 0x10DE0A6A, "NVS 2100M" },
	{ 0x10DE0A6C, "NVS 3100M" },
	{ 0x10DE0A6E, "GeForce 305M" },
	{ 0x10DE0A6F, "ION" },	
	// 0A70 - 0A7F
	{ 0x10DE0A70, "GeForce 310M" },
	{ 0x10DE0A71, "GeForce 305M" },
	{ 0x10DE0A72, "GeForce 310M" },
	{ 0x10DE0A73, "GeForce 305M" },
	{ 0x10DE0A74, "GeForce G210M" },
	{ 0x10DE0A75, "GeForce G310M" },
	{ 0x10DE0A78, "Quadro FX 380 LP" },
	{ 0x10DE0A7C, "Quadro FX 380M" },
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
	{ 0x10DE0CA0, "GeForce GT 330 " },
	{ 0x10DE0CA2, "GeForce GT 320" },
	{ 0x10DE0CA3, "GeForce GT 240" },
	{ 0x10DE0CA4, "GeForce GT 340" },
	{ 0x10DE0CA7, "GeForce GT 330" },
	{ 0x10DE0CA8, "GeForce GTS 260M" },
	{ 0x10DE0CA9, "GeForce GTS 250M" },
	{ 0x10DE0CAC, "GeForce 315" },
	{ 0x10DE0CAF, "GeForce GT 335M" },
	// 0CB0 - 0CBF	
	{ 0x10DE0CB0, "GeForce GTS 350M" },
	{ 0x10DE0CB1, "GeForce GTS 360M" },
	{ 0x10DE0CBC, "Quadro FX 1800M" },
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
	{ 0x10DE0DC0, "GeForce GT 440" },
	{ 0x10DE0DC1, "D12-P1-35" },
	{ 0x10DE0DC2, "D12-P1-35" },
	{ 0x10DE0DC4, "GeForce GTS 450" },
	{ 0x10DE0DC5, "GeForce GTS 450" },
	{ 0x10DE0DC6, "GeForce GTS 450" },
	{ 0x10DE0DCA, "GF10x" },
	// 0DD0 - 0DDF	
	{ 0x10DE0DD1, "GeForce GTX 460M" },
	{ 0x10DE0DD2, "GeForce GT 445M" },
	{ 0x10DE0DD3, "GeForce GT 435M" },
	{ 0x10DE0DD8, "Quadro 2000" },
	{ 0x10DE0DDE, "GF106-ES" },
	{ 0x10DE0DDF, "GF106-INT" },
	// 0DE0 - 0DEF	
	{ 0x10DE0DE0, "GeForce GT 440" },
	{ 0x10DE0DE1, "GeForce GT 430" },
	{ 0x10DE0DE2, "GeForce GT 420" },
  { 0x10DE0DE3, "GeForce GT 525M" },
  { 0x10DE0DE4, "GeForce GT 540M" },
	{ 0x10DE0DE5, "GeForce GT 530" },
	{ 0x10DE0DEB, "GeForce GT 555M" },
	{ 0x10DE0DEE, "GeForce GT 415M" },
	// 0DF0 - 0DFF	
	{ 0x10DE0DF0, "GeForce GT 425M" },
	{ 0x10DE0DF1, "GeForce GT 420M" },
	{ 0x10DE0DF2, "GeForce GT 435M" },
	{ 0x10DE0DF3, "GeForce GT 420M" },
  { 0x10DE0DF4, "GeForce GT 450M" },
	{ 0x10DE0DF8, "Quadro 600" },
	{ 0x10DE0DFE, "GF108 ES" },
	{ 0x10DE0DFF, "GF108 INT" },
	// 0E00 - 0E0F
	// 0E10 - 0E1F
	// 0E20 - 0E2F
	{ 0x10DE0E21, "D12U-25" },
	{ 0x10DE0E22, "GeForce GTX 460" },
	{ 0x10DE0E23, "GeForce GTX 460 SE" },
	{ 0x10DE0E24, "GeForce GTX 460" },
	{ 0x10DE0E25, "D12U-50" },
	// 0E30 - 0E3F	
	{ 0x10DE0E30, "GeForce GTX 470M" },
	{ 0x10DE0E38, "GF104GL" },
	{ 0x10DE0E3E, "GF104-ES" },
	{ 0x10DE0E3F, "GF104-INT" },
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
	// 0FC0 - 0FCF
	// 0FD0 - 0FDF
	// 0FE0 - 0FEF
	// 0FF0 - 0FFF
	// 1000 - 100F
	// 1010 - 101F
	// 1020 - 102F
	// 1030 - 103F
	// 1040 - 104F
	{ 0x10DE1040, "GeForce GT 520" },	
	// 1050 - 105F
	{ 0x10DE1050, "GeForce GT 520M" },
	// 1060 - 106F
	// 1070 - 107F
	// 1080 - 108F
	{ 0x10DE1080, "GeForce GTX 580" },
	{ 0x10DE1081, "GeForce GTX 570" },
	{ 0x10DE1082, "GeForce GTX 560 Ti" },
	{ 0x10DE1083, "D13U" },
	{ 0x10DE1088, "GeForce GTX 590" },
	// 1090 - 109F	
	{ 0x10DE1098, "D13U" },
	{ 0x10DE109A, "N12E-Q5" },
	// 10A0 - 10AF
	// 10B0 - 10BF
	// 10C0 - 10CF
	{ 0x10DE10C3, "GeForce 8400 GS" },
	// 1200 - 
	{ 0x10DE1200, "GeForce GTX 560 Ti" },
	{ 0x10DE1244, "GeForce GTX 550 Ti" },
	{ 0x10DE1245, "GeForce GTS 450" },	
};





static UINT16 swap16(UINT16 x)
{
	return (((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8));
}

static UINT16 read16(UINT8 *ptr, UINT16 offset)
{
	UINT8 ret[2];
	
	ret[0] = ptr[offset+1];
	ret[1] = ptr[offset];
	
	return *((UINT16*)&ret);
}

#if 0
static UINT32 swap32(UINT32 x)
{
	return ((x & 0x000000FF) << 24) | ((x & 0x0000FF00) << 8 ) | ((x & 0x00FF0000) >> 8 ) | ((x & 0xFF000000) >> 24);
}

static UINT8	read8(UINT8 *ptr, UINT16 offset)
{ 
	return ptr[offset];
}

static UINT32 read32(UINT8 *ptr, UINT16 offset)
{
	UINT8 ret[4];
	
	ret[0] = ptr[offset+3];
	ret[1] = ptr[offset+2];
	ret[2] = ptr[offset+1];
	ret[3] = ptr[offset];
	
	return *((UINT32*)&ret);
}
#endif

EFI_STATUS read_nVidia_PRAMIN(pci_dt_t *nvda_dev, VOID* rom, UINT8 arch)
{
	DBG("read_nVidia_ROM\n");
	EFI_STATUS Status;
	EFI_PCI_IO_PROTOCOL		*PciIo;
	PCI_TYPE00				Pci;
  
  UINT32 vbios_vram = 0;
  UINT32 old_bar0_pramin = 0;
  
  Status = gBS->OpenProtocol(nvda_dev->DeviceHandle, &gEfiPciIoProtocolGuid, (VOID**)&PciIo, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if (EFI_ERROR(Status)) return EFI_NOT_FOUND;
  Status = PciIo->Pci.Read(PciIo,EfiPciIoWidthUint32, 0, sizeof(Pci) / sizeof(UINT32), &Pci);
  if (EFI_ERROR(Status)) return EFI_NOT_FOUND;
  
  if (arch>=0x50) {
    DBG("Using PRAMIN fixups\n");
    Status = PciIo->Mem.Read(
                             PciIo,
                             EfiPciIoWidthUint32,
                             0,
                             NV_PDISPLAY_OFFSET + 0x9f04,///4,
                             1,
                             &vbios_vram
                             );
    vbios_vram = (vbios_vram & ~0xff) << 8;
    
    Status = PciIo->Mem.Read(
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
    
    Status = PciIo->Mem.Write(
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
  
  if (arch>=0x50) {
    Status = PciIo->Mem.Write(
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
	DBG("PROM\n");
	EFI_STATUS Status;
	EFI_PCI_IO_PROTOCOL		*PciIo;
	PCI_TYPE00				Pci;
  UINT32 value;
  
  Status = gBS->OpenProtocol(nvda_dev->DeviceHandle, &gEfiPciIoProtocolGuid, (VOID**)&PciIo, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if (EFI_ERROR(Status)) return EFI_NOT_FOUND;
  Status = PciIo->Pci.Read(PciIo,EfiPciIoWidthUint32, 0, sizeof(Pci) / sizeof(UINT32), &Pci);
  if (EFI_ERROR(Status)) return EFI_NOT_FOUND;
  
  value = NV_PBUS_PCI_NV_20_ROM_SHADOW_DISABLED;
  
  Status = PciIo->Mem.Write(
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
  
  Status = PciIo->Mem.Write(
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
	DBG("patch_nvidia_rom\n");
	if (!rom || (rom[0] != 0x55 && rom[1] != 0xaa)) {
		DBG("FALSE ROM signature: 0x%02x%02x\n", rom[0], rom[1]);
		return PATCH_ROM_FAILED;
	}
	
	UINT16 dcbptr = swap16(read16(rom, 0x36));
	if(!dcbptr) {
		DBG("no dcb table found\n");
		return PATCH_ROM_FAILED;
	}
//	else
//		DBG("dcb table at offset 0x%04x\n", dcbptr);
	 
	UINT8 *dcbtable		 = &rom[dcbptr];
	UINT8 dcbtable_version = dcbtable[0];
	UINT8 headerlength	 = 0;
	UINT8 numentries		 = 0;
	UINT8 recordlength	 = 0;
	
	if (dcbtable_version >= 0x20)
	{
		UINT32 sig;
		
		if (dcbtable_version >= 0x30)
		{
			headerlength = dcbtable[1];
			numentries	 = dcbtable[2];
			recordlength = dcbtable[3];
			
			sig = *(UINT32 *)&dcbtable[6];
		}
		else
		{
			sig = *(UINT32 *)&dcbtable[4];
			headerlength = 8;
		}
		
		if (sig != 0x4edcbdcb)
		{
			DBG("Bad display config block signature (0x%8x)\n", sig); //Azi: issue #48
			return PATCH_ROM_FAILED;
		}
	}
	else if (dcbtable_version >= 0x14) /* some NV15/16, and NV11+ */
	{
		CHAR8 sig[8] = { 0 };
		
		AsciiStrnCpy(sig, (CHAR8 *)&dcbtable[-7], 7);
		recordlength = 10;
		
		if (AsciiStrCmp(sig, "DEV_REC"))
		{
			DBG("Bad Display Configuration Block signature (%a)\n", sig);
			return PATCH_ROM_FAILED;
		}
	}
	else
	{
		DBG("ERROR: dcbtable_version is 0x%X\n", dcbtable_version);
		return PATCH_ROM_FAILED;
	}
	
	if (numentries >= MAX_NUM_DCB_ENTRIES)
		numentries = MAX_NUM_DCB_ENTRIES;
	
	UINT8 num_outputs = 0, i = 0;
	
	struct dcbentry
	{
		UINT8 type;
		UINT8 index;
		UINT8 *heads;
	} entries[numentries];
	
	for (i = 0; i < numentries; i++)
	{
		UINT32 connection;
		connection = *(UINT32 *)&dcbtable[headerlength + recordlength * i];
		
		/* Should we allow discontinuous DCBs? Certainly DCB I2C tables can be discontinuous */
		if ((connection & 0x0000000f) == 0x0000000f) /* end of records */ 
			continue;
		if (connection == 0x00000000) /* seen on an NV11 with DCB v1.5 */ 
			continue;
		if ((connection & 0xf) == 0x6) /* we skip type 6 as it doesnt appear on macbook nvcaps */
			continue;
		
		entries[num_outputs].type = connection & 0xf;
		entries[num_outputs].index = num_outputs;
		entries[num_outputs++].heads = (UINT8*)&(dcbtable[(headerlength + recordlength * i) + 1]);
	}
	
	INT32 has_lvds = FALSE;
	UINT8 channel1 = 0, channel2 = 0;
	
	for (i = 0; i < num_outputs; i++)
	{
		if (entries[i].type == 3)
		{
			has_lvds =TRUE;
			//DBG("found LVDS\n");
			channel1 |= ( 0x1 << entries[i].index);
			entries[i].type = TYPE_GROUPED;
		}
	}
	
	// if we have a LVDS output, we group the rest to the second channel
	if (has_lvds)
	{
		for (i = 0; i < num_outputs; i++)
		{
			if (entries[i].type == TYPE_GROUPED)
				continue;
			
			channel2 |= ( 0x1 << entries[i].index);
			entries[i].type = TYPE_GROUPED;
		}
	}
	else
	{
		INT32 x;
		// we loop twice as we need to generate two channels
		for (x = 0; x <= 1; x++)
		{
			for (i=0; i<num_outputs; i++)
			{
				if (entries[i].type == TYPE_GROUPED)
					continue;
				// if type is TMDS, the prior output is ANALOG
				// we always group ANALOG and TMDS
				// if there is a TV output after TMDS, we group it to that channel as well
				if (i && entries[i].type == 0x2)
				{
					switch (x)
					{
						case 0:
							//DBG("group channel 1\n");
							channel1 |= ( 0x1 << entries[i].index);
							entries[i].type = TYPE_GROUPED;
							
							if ((entries[i-1].type == 0x0))
							{
								channel1 |= ( 0x1 << entries[i-1].index);
								entries[i-1].type = TYPE_GROUPED;
							}
							// group TV as well if there is one
							if ( ((i+1) < num_outputs) && (entries[i+1].type == 0x1) )
							{
								//	DBG("group tv1\n");
								channel1 |= ( 0x1 << entries[i+1].index);
								entries[i+1].type = TYPE_GROUPED;
							}
							break;
						
						case 1:
							//DBG("group channel 2 : %d\n", i);
							channel2 |= ( 0x1 << entries[i].index);
							entries[i].type = TYPE_GROUPED;
							
							if ((entries[i - 1].type == 0x0))
							{
								channel2 |= ( 0x1 << entries[i-1].index);
								entries[i-1].type = TYPE_GROUPED;
							}
							// group TV as well if there is one
							if ( ((i+1) < num_outputs) && (entries[i+1].type == 0x1) )
							{
								//	DBG("group tv2\n");
								channel2 |= ( 0x1 << entries[i+1].index);
								entries[i+1].type = TYPE_GROUPED;
							}
							break;
					}
					break;
				}
			}
		}
	}
	
	// if we have left ungrouped outputs merge them to the empty channel
	UINT8 *togroup;// = (channel1 ? (channel2 ? NULL : &channel2) : &channel1);
	togroup = &channel2;
	
	for (i = 0; i < num_outputs; i++)
	{
		if (entries[i].type != TYPE_GROUPED)
		{
			//DBG("%d not grouped\n", i);
			if (togroup)
			{
				*togroup |= ( 0x1 << entries[i].index);
			}
			entries[i].type = TYPE_GROUPED;
		}
	}
	
	if (channel1 > channel2)
	{
		UINT8 buff = channel1;
		channel1 = channel2;
		channel2 = buff;
	}
	
	default_NVCAP[6] = channel1;
	default_NVCAP[8] = channel2;
	
	// patching HEADS
	for (i = 0; i < num_outputs; i++)
	{
		if (channel1 & (1 << i))
		{
			*entries[i].heads = 1;
		}
		else if(channel2 & (1 << i))
		{
			*entries[i].heads = 2;
		}
	}
	return (has_lvds ? PATCH_ROM_SUCCESS_HAS_LVDS : PATCH_ROM_SUCCESS);
}


static CHAR8 *get_nvidia_model(UINT32 id)
{
	DBG("get_nvidia_model\n");
	INT32 i;
	CHAR8* name;
	
	for (i = 1; i < (sizeof(NVKnownChipsets) / sizeof(NVKnownChipsets[0])); i++) {
		if (NVKnownChipsets[i].device == id)
		{
			UINTN size = AsciiStrLen(NVKnownChipsets[i].name);
			name = AllocatePool(size);
			AsciiStrnCpy(name, NVKnownChipsets[i].name, size);
			return name;
		}
	}
	name = AllocatePool(80);
	AsciiSPrint(name, 80, "Unknown");
	return name;
}

#if 0 //loading VBIOS is now unsupported in iBoot
static UINT32 load_nvidia_bios_file(const CHAR8 *filename, UINT8 *buf, INT32 bufsize)
{
	INT32 fd;
	INT32 size;
	
	if ((fd = open_bvdev("bt(0,0)", filename, 0)) < 0)
	{
		return 0;
	}
	
	size = file_size(fd);
	
	if (size > bufsize)
	{
		DBG("Filesize of %s is bigger than expected! Truncating to 0x%x Bytes!\n",
				filename, bufsize);
		size = bufsize;
	}
	size = read(fd, (CHAR8 *)buf, size);
	close(fd);
	
	return size > 0 ? size : 0;
}
#endif

static INT32 devprop_add_nvidia_template(DevPropDevice *device)
{
	DBG("devprop_add_nvidia_template\n");
	CHAR8 tmp[16];
	
	if (!device)
		return 0;
	
	if (!DP_ADD_TEMP_VAL(device, nvidia_compatible_0))
		return 0;
	if (!DP_ADD_TEMP_VAL(device, nvidia_device_type_0))
		return 0;
	if (!DP_ADD_TEMP_VAL(device, nvidia_name_0))
		return 0;
	if (!DP_ADD_TEMP_VAL(device, nvidia_compatible_1))
		return 0;
	if (!DP_ADD_TEMP_VAL(device, nvidia_device_type_1))
		return 0;
	if (!DP_ADD_TEMP_VAL(device, nvidia_name_1))
		return 0;
	if (!DP_ADD_TEMP_VAL(device, nvidia_device_type))
		return 0;
	
	AsciiSPrint(tmp, 16, "Slot-%x",devices_number);
	devprop_add_value(device, "AAPL,slot-name", (UINT8 *) tmp, AsciiStrLen(tmp));
	devices_number++;
	
	return 1;
}


UINT8 hexstrtouint8 (CHAR8* buf) {
	INT8 i;
	if (IS_DIGIT(buf[0]))
		i = buf[0]-'0';
	else
		i = buf[0]-'A' + 10; //no error checking
	i <<= 4;
	if (IS_DIGIT(buf[1]))
		i += buf[1]-'0';
	else
		i += buf[1]-'A'+ 10; //no error checking
	return i;
}

BOOLEAN IsHexDigit (CHAR8 c) {
	return (IS_DIGIT(c) || (c>='A'&&c<='F'))?TRUE:FALSE;
}


BOOLEAN hex2bin(IN CHAR8 *hex, OUT UINT8 *bin, INT32 len)
{
	CHAR8	*p;
	INT32	i;
	CHAR8	buf[3];
	
	if (hex == NULL || bin == NULL || len <= 0 || AsciiStrLen(hex) != len * 2) {
		DBG("[ERROR] bin2hex input error\n");
		return FALSE;
	}
	
	buf[2] = '\0';
	p = (CHAR8 *) hex;
	
	for (i = 0; i < len; i++)
	{
		if (!IsHexDigit(p[0]) || !IsHexDigit(p[1])) {
			DBG("[ERROR] bin2hex '%a' syntax error\n", hex);
			return -2;
		}
		buf[0] = *p++;
		buf[1] = *p++;
		bin[i] = hexstrtouint8(buf);
	}
	return TRUE;
}

UINT32 mem_detect(UINT8 nvCardType, pci_dt_t *nvda_dev)
{
	DBG("mem_detect\n");
	UINT64 vram_size = 0;
	
	if (nvCardType < NV_ARCH_50)
	{
		vram_size  = REG32(nvda_dev->regs, NV04_PFB_FIFO_DATA);
		vram_size &= NV10_PFB_FIFO_DATA_RAM_AMOUNT_MB_MASK;
	}
	else if (nvCardType < NV_ARCH_C0)
	{
		vram_size = REG32(nvda_dev->regs, NV04_PFB_FIFO_DATA);
		vram_size |= (vram_size & 0xff) << 32;
		vram_size &= 0xffffffff00ll;
	}
	else // >= NV_ARCH_C0
	{
		vram_size = REG32(nvda_dev->regs, NVC0_MEM_CTRLR_RAM_AMOUNT) << 20;
		vram_size *= REG32(nvda_dev->regs, NVC0_MEM_CTRLR_COUNT);
	}
	
	// Workaround for GT 420/430 & 9600M GT
	switch (nvda_dev->device_id)
	{
		case 0x0DE1: vram_size = 1024*1024*1024; break; // GT 430
		case 0x0DE2: vram_size = 1024*1024*1024; break; // GT 420
		case 0x0649: vram_size = 512*1024*1024; break;	// 9600M GT
		default: break;
	}
	
	return vram_size;
}

BOOLEAN setup_nvidia_devprop(pci_dt_t *nvda_dev)
{
	//DBG("setup_nvidia_devprop\n");
	DevPropDevice	*device = NULL;
	CHAR8					*devicepath = NULL;
	
	//UINT8					*rom = NULL;
	UINT8					nvCardType = 0;
	UINT32					videoRam = 0;
	UINT32					bar[7];
	UINT32					boot_display = 0;
	INT32					nvPatch = 0;
	CHAR8					*model = NULL;
		
	DBG("%x:%x\n",nvda_dev->vendor_id, nvda_dev->device_id);
	devicepath = get_pci_dev_path(nvda_dev);
	bar[0] = pci_config_read32(nvda_dev, PCI_BASE_ADDRESS_0);
  nvda_dev->regs = (UINT8 *)(UINTN)(bar[0] & ~0x0f);
	DBG("BAR: 0x%x\n", nvda_dev->regs);
//	regs = (UINT8 *)(UINTN)(bar[0] & ~0x0f);
	
//	gBS->Stall(50);
		
	// get card type
	nvCardType = (REG32(nvda_dev->regs, 0) >> 20) & 0x1ff;
	
	// Amount of VRAM in kilobytes
	videoRam = mem_detect(nvCardType, nvda_dev);
	model = get_nvidia_model((nvda_dev->vendor_id << 16) | nvda_dev->device_id);
	
	DBG("nVidia %a\n", model);
	DBG(" %dMB NV%02x [%04x:%04x] :: ", (UINT32)(videoRam / 1024 / 1024),
			   nvCardType, nvda_dev->vendor_id, nvda_dev->device_id);
		

	const INT32 MAX_BIOS_VERSION_LENGTH = 32;
	CHAR8* version_str = (CHAR8*)AllocateZeroPool(MAX_BIOS_VERSION_LENGTH);
		
	UINT8* rom = AllocateZeroPool(NVIDIA_ROM_SIZE+1);
		// PRAMIN first
	read_nVidia_PRAMIN(nvda_dev, rom, nvCardType);
			 
	//DBG("got here\n");
		
	DBG("%x%x\n", rom[0], rom[1]);
	option_rom_pci_header_t *rom_pci_header = NULL;
			
	if (rom[0] != 0x55 || rom[1] != 0xaa) {
		read_nVidia_PROM(nvda_dev, rom);
		if (rom[0] != 0x55 || rom[1] != 0xaa)
			DBG("ERROR: Unable to locate nVidia Video BIOS\n");
			}
	
	if (rom[0] == 0x55 && rom[1] == 0xaa) {	
		if ((nvPatch = patch_nvidia_rom(rom)) == PATCH_ROM_FAILED) {
		DBG("ERROR: nVidia ROM Patching Failed!\n");
	}
	
	rom_pci_header = (option_rom_pci_header_t*)(rom + *(UINT16 *)&rom[24]);
	
	// check for 'PCIR' sig
		if (rom_pci_header->signature == 0x52494350/*50434952*/) { //for some reason, the reverse byte order
			if (rom_pci_header->device_id != nvda_dev->device_id) {
			// Get Model from the OpROM
			model = get_nvidia_model((rom_pci_header->vendor_id << 16) | rom_pci_header->device_id);
				DBG(model);
			}
		}
		else
		{
			DBG("nVidia incorrect PCI ROM signature: 0x%x\n", rom_pci_header->signature);
		}
	
	// get bios version
	
		//ZeroMem((VOID*)version_str, MAX_BIOS_VERSION_LENGTH);
	
	INT32 i, version_start;
	INT32 crlf_count = 0;
	
	// only search the first 384 bytes
	for (i = 0; i < 0x180; i++)
	{
		if (rom[i] == 0x0D && rom[i+1] == 0x0A)
		{
			crlf_count++;
			// second 0x0D0A was found, extract bios version
			if (crlf_count == 2)
			{
				if (rom[i-1] == 0x20) i--; // strip last " "
				
				for (version_start = i; version_start > (i-MAX_BIOS_VERSION_LENGTH); version_start--)
				{
					// find start
					if (rom[version_start] == 0x00)
					{
						version_start++;
						
						// strip "Version "
						if (AsciiStrnCmp((const CHAR8*)rom+version_start, "Version ", 8) == 0)
						{
							version_start += 8;
						}
						CHAR8* s = (CHAR8*)(rom + version_start);
            CHAR8* s1 = version_str;
            while ((*s > ' ') && (*s < 'z') && ((INTN)(s1 - version_str) < MAX_BIOS_VERSION_LENGTH)) {
              *s1++ = *s++;
            }
            *s1 = 0;
		//				AsciiStrnCpy(version_str, (const CHAR8*)rom+version_start, i-version_start);
            DBG(version_str);
						break;
					}
				}
				break;
			}
		}
	}
	}
//#endif
	
	DBG(devicepath);
	DBG("\n");
	
	if (!string) {
		string = devprop_create_string();
	}
	device = devprop_add_device(string, devicepath);
	devprop_add_nvidia_template(device);

	
	/* FIXME: for primary graphics card only */
	boot_display = 1;
	devprop_add_value(device, "@0,AAPL,boot-display", (UINT8*)&boot_display, 4);
	
	if (nvPatch == PATCH_ROM_SUCCESS_HAS_LVDS) {
		UINT8 built_in = 0x01;
		devprop_add_value(device, "@0,built-in", &built_in, 1);
	}

	
	//AsciiSPrint(biosVersion, 32, "%a", version_str);
	//AsciiSPrint(kNVCAP, 12, "NVCAP_%04x", nvda_dev->device_id);
	
	
//#ifdef DEBUG_NVCAP
	DBG("NVCAP: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x\n",
	default_NVCAP[0], default_NVCAP[1], default_NVCAP[2], default_NVCAP[3],
	default_NVCAP[4], default_NVCAP[5], default_NVCAP[6], default_NVCAP[7],
	default_NVCAP[8], default_NVCAP[9], default_NVCAP[10], default_NVCAP[11],
	default_NVCAP[12], default_NVCAP[13], default_NVCAP[14], default_NVCAP[15],
	default_NVCAP[16], default_NVCAP[17], default_NVCAP[18], default_NVCAP[19]);
//#endif
	if ((gSettings.NVCAP[0] != 0)) {
    devprop_add_value(device, "NVCAP", &gSettings.NVCAP[0], NVCAP_LEN);
  } else {
    devprop_add_value(device, "NVCAP", default_NVCAP, NVCAP_LEN);
  }	
	devprop_add_value(device, "NVPM", default_NVPM, NVPM_LEN);
  if ((gSettings.VRAM != 0)) {
    devprop_add_value(device, "VRAM,totalsize", (UINT8*)&gSettings.VRAM, 4);
  } else {
    devprop_add_value(device, "VRAM,totalsize", (UINT8*)&videoRam, 4);
  }	
	devprop_add_value(device, "model", (UINT8*)model, AsciiStrLen(model));
	devprop_add_value(device, "rom-revision", (UINT8*)version_str, AsciiStrLen(version_str));
  if ((gSettings.Dcfg[0] != 0) && (gSettings.Dcfg[1] != 0)) {
    devprop_add_value(device, "@0,display-cfg", &gSettings.Dcfg[0], DCFG0_LEN);
    devprop_add_value(device, "@1,display-cfg", &gSettings.Dcfg[4], DCFG1_LEN);
  } else {
    devprop_add_value(device, "@0,display-cfg", default_dcfg_0, DCFG0_LEN);
    devprop_add_value(device, "@1,display-cfg", default_dcfg_1, DCFG1_LEN);
  }
	
	//add HDMI Audio back to nvidia
	//http://forge.voodooprojects.org/p/chameleon/issues/67/
//	UINT8 connector_type_1[]= {0x00, 0x08, 0x00, 0x00};
//	devprop_add_value(device, "@1,connector-type",connector_type_1, 4);
	//end Nvidia HDMI Audio
	

//	gDeviceProperties = (VOID*)devprop_generate_string(string);
//	gBS->Stall(2000000);
	return TRUE;
}
