/*
 *	NVidia injector
 *
 *	Copyright (C) 2009	Jasmin Fazlic, iNDi
 *
 *	NVidia injector modified by Fabio (ErmaC) on May 2012,
 *	for allow the cosmetics injection also based on SubVendorID and SubDeviceID.
 *
 *	NVidia injector is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	NVidia driver and injector is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with NVidia injector.	 If not, see <http://www.gnu.org/licenses/>.
 *
 *	Alternatively you can choose to comply with APSL
 *
 *	DCB-Table parsing is based on software (nouveau driver) originally distributed under following license:
 *
 *
 *	Copyright 2005-2006 Erik Waling
 *	Copyright 2006 Stephane Marchesin
 *	Copyright 2007-2009 Stuart Bennett
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a
 *	copy of this software and associated documentation files (the "Software"),
 *	to deal in the Software without restriction, including without limitation
 *	the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *	and/or sell copies of the Software, and to permit persons to whom the
 *	Software is furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in
 *	all copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *	THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 *	OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
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


extern UINT32 devices_number;

const CHAR8 *nvidia_compatible_0[]	=	{ "@0,compatible",	"NVDA,NVMac"	 };
const CHAR8 *nvidia_compatible_1[]	=	{ "@1,compatible",	"NVDA,NVMac"	 };
const CHAR8 *nvidia_device_type_0[]	=	{ "@0,device_type", "display"		 };
const CHAR8 *nvidia_device_type_1[]	=	{ "@1,device_type", "display"		 };
const CHAR8 *nvidia_device_type_parent[] =	{ "device_type",	"NVDA,Parent"	 };
const CHAR8 *nvidia_device_type_child[]	 =	{ "device_type",	"NVDA,Child"	 };
const CHAR8 *nvidia_name_0[]			=	{ "@0,name",		"NVDA,Display-A" };
const CHAR8 *nvidia_name_1[]			=	{ "@1,name",		"NVDA,Display-B" };
const CHAR8 *nvidia_slot_name[]		=	{ "AAPL,slot-name", "Slot-1"		 };

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

static nvidia_card_info_t nvidia_cards[] = {
    /* ========================================================================================
     * Layout is device(VendorId + DeviceId), subdev (SubvendorId + SubdeviceId), display name.
     * ========================================================================================
     */
	/*Unknown*/	{ 0x10DE0000,	NV_SUB_IDS,	"Unknown" },
    /* ------ Specific DeviceID and SubDevID. ------ */
	// 0000 - 0040
	// 0040 - 004F
	{ 0x10DE0040,	0x10438178,	"Asus V9999 Ultra V62.11" },
	{ 0x10DE0040,	0x1043817D,	"Asus V9999GT V61.21" },
	{ 0x10DE0040,	0x10DE0205,	"nVidia GeForce 6800 Ultra" },
	{ 0x10DE0040,	0x7FFFFFFF,	"GeForce 6800 Ultra [NV40.0]" },
	// 0050 - 005F
	// 0060 - 006F
	// 0070 - 007F
	// 0080 - 008F
	// 0090 - 009F
	// 00A0 - 00AF
	// 00B0 - 00BF
	// 00C0 - 00CF
	// 00D0 - 00DF
	// 00E0 - 00EF
	// 00F0 - 00FF
	// 0100 - 010F
	// 0110 - 011F
	// 0120 - 012F
	// 0130 - 013F
	// 0140 - 014F
	// 0150 - 015F
	// 0160 - 016F
	// 0170 - 017F
	// 0180 - 018F
	// 0190 - 019F
	{ 0x10DE0193,	0x10438234,	"Asus GeForce 8800 GTS" },
	{ 0x10DE0193,	0x1043823C,	"Asus GeForce 8800 GTS" },
	{ 0x10DE0193,	0x1043825F,	"Asus GeForce 8800 GTS" },
	{ 0x10DE0193,	0x10DE0420,	"nVidia GeForce 8800 GTS" },
	{ 0x10DE0193,	0x10DE0421,	"nVidia GeForce 8800 GTS" },
	{ 0x10DE0193,	0x19F104A6,	"BFG GeForce 8800 GTS" },
	{ 0x10DE019D,	0x107D2A72,	"Leadtek Quadro FX 5600" },
	{ 0x10DE019D,	0x10DE0409,	"nVidia Quadro FX 5600" },
	{ 0x10DE019E,	0x107D2A72,	"Leadtek Quadro FX 4600" },
	{ 0x10DE019E,	0x10DE0408,	"nVidia Quadro FX 4600" },
	// 01A0 - 01AF
	// 01B0 - 01BF
	// 01C0 - 01CF
	// 01D0 - 01DF
	{ 0x10DE01D7,	0x1025006C,	"Acer GeForce Go 7300" },
	{ 0x10DE01D7,	0x10250090,	"Acer GeForce Go 7300" },
	{ 0x10DE01D7,	0x10250096,	"Acer GeForce Go 7300" },
	{ 0x10DE01D7,	0x10250100,	"Acer GeForce Go 7300" },
	{ 0x10DE01D7,	0x10250107,	"Acer GeForce Go 7300" },
	{ 0x10DE01D7,	0x10250110,	"Acer GeForce Go 7300" },
	{ 0x10DE01D7,	0x10250112,	"Acer GeForce Go 7300" },
	{ 0x10DE01D7,	0x102501C2,	"Acer GeForce Go 7300" },
	{ 0x10DE01D7,	0x102501C8,	"Acer GeForce Go 7300" },
	{ 0x10DE01D7,	0x102801C2,	"Dell Quadro NVS 110M" },
	{ 0x10DE01D7,	0x102801C8,	"Dell GeForce Go 7300" },
	{ 0x10DE01D7,	0x102801CC,	"Dell Quadro NVS 110M" },
	{ 0x10DE01D7,	0x102801D7,	"Dell GeForce Go 7300" },
	{ 0x10DE01D7,	0x102801E2,	"Dell GeForce Go 7300" },
	{ 0x10DE01D7,	0x102801F9,	"Dell GeForce Go 7300" },
	{ 0x10DE01D7,	0x102801FE,	"Dell GeForce Go 7300" },
	{ 0x10DE01D7,	0x10282003,	"Dell GeForce Go 7300" },
	{ 0x10DE01D7,	0x10338848,	"NEC GeForce Go 7300" },
	{ 0x10DE01D7,	0x103C30B2,	"HP GeForce Go 7300" },
	{ 0x10DE01D7,	0x103C30B7,	"HP GeForce Go 7300" },
	{ 0x10DE01D7,	0x10431212,	"Asus GeForce Go 7300" },
	{ 0x10DE01D7,	0x104313A2,	"Asus GeForce Go 7300" },
	{ 0x10DE01D7,	0x10431441,	"Asus GeForce Go 7300" },
	{ 0x10DE01D7,	0x10DE0000,	"nVidia GeForce Go 7300" },
	{ 0x10DE01D7,	0x10DE014B,	"nVidia Quadro NVS 110M" },
	{ 0x10DE01D7,	0x11790001,	"Toshiba GeForce Go 7300" },
	{ 0x10DE01D7,	0x11790002,	"Toshiba GeForce Go 7300" },
	{ 0x10DE01D7,	0x1179FF00,	"Toshiba GeForce Go 7300" },
	{ 0x10DE01D7,	0x1179FF01,	"Toshiba GeForce Go 7300" },
	{ 0x10DE01D7,	0x1179FF02,	"Toshiba GeForce Go 7300" },
	{ 0x10DE01D7,	0x1179FF10,	"Toshiba GeForce Go 7300" },
	{ 0x10DE01D7,	0x1179FF31,	"Toshiba GeForce Go 7300" },
	{ 0x10DE01D7,	0x13DC1172,	"Netbost GeForce Go 7300" },
	{ 0x10DE01D7,	0x144D8063,	"Samsung GeForce Go 7300" },
	{ 0x10DE01D7,	0x144DC024,	"Samsung GeForce Go 7300" },
	{ 0x10DE01D7,	0x144DC026,	"Samsung GeForce Go 7300" },
	{ 0x10DE01D7,	0x144DC513,	"Samsung GeForce Go 7300" },
	{ 0x10DE01D7,	0x14C00012,	"Compal GeForce Go 7300" },

	{ 0x10DE01D7,	0xC0181631,	"GeForce Go 7300" },
	{ 0x10DE01D8,	0x10250090,	"Acer GeForce Go 7400" },
	{ 0x10DE01D8,	0x102801C8,	"Dell GeForce Go 7400" },
	{ 0x10DE01D8,	0x102801CC,	"Dell Quadro NVS 120M" },
	{ 0x10DE01D8,	0x102801D7,	"Dell GeForce Go 7400" },
	{ 0x10DE01D8,	0x102801F3,	"Dell GeForce Go 7400" },
	{ 0x10DE01D8,	0x102801F9,	"Dell GeForce Go 7400" },
	{ 0x10DE01D8,	0x102801FE,	"Dell GeForce Go 7400" },
	{ 0x10DE01D8,	0x10280209,	"Dell GeForce Go 7400" },
	{ 0x10DE01D8,	0x10282003,	"Dell Quadro NVS 120M" },
	{ 0x10DE01D8,	0x103C30A5,	"HP GeForce Go 7400" },
	{ 0x10DE01D8,	0x103C30B6,	"HP GeForce Go 7400" },
	{ 0x10DE01D8,	0x103C30B7,	"HP GeForce Go 7400" },
	{ 0x10DE01D8,	0x103C30BB,	"HP GeForce Go 7400" },
	{ 0x10DE01D8,	0x10431211,	"Asus GeForce Go 7400" },
	{ 0x10DE01D8,	0x10431214,	"Asus GeForce Go 7400" },
	{ 0x10DE01D8,	0x104D81E6,	"Sony GeForce Go 7400" },
	{ 0x10DE01D8,	0x104D81EF,	"Sony GeForce Go 7400" },
	{ 0x10DE01D8,	0x104D81FD,	"Sony GeForce Go 7400" },
	{ 0x10DE01D8,	0x104D8205,	"Sony GeForce Go 7400" },
	{ 0x10DE01D8,	0x104D820F,	"Sony GeForce Go 7400" },
	{ 0x10DE01D8,	0x109F319C,	"Trigem GeForce Go 7400" },
	{ 0x10DE01D8,	0x109F319D,	"Trigem GeForce Go 7400" },
	{ 0x10DE01D8,	0x109F3C01,	"Trigem GeForce Go 7400" },
	{ 0x10DE01D8,	0x11790001,	"Toshiba GeForce Go 7400" },
	{ 0x10DE01D8,	0x1179FF00,	"Toshiba GeForce Go 7400" },
	{ 0x10DE01D8,	0x1179FF10,	"Toshiba GeForce Go 7400" },
	{ 0x10DE01D8,	0x1179FF31,	"Toshiba GeForce Go 7400" },
	{ 0x10DE01D8,	0x144D8062,	"Samsung GeForce Go 7400" },
	{ 0x10DE01D8,	0x144DB03C,	"Samsung GeForce Go 7400" },
	{ 0x10DE01D8,	0x144DC024,	"Samsung GeForce Go 7400" },
	{ 0x10DE01D8,	0x144DC026,	"Samsung GeForce Go 7400" },
	{ 0x10DE01D8,	0x14620511,	"MSi GeForce Go 7400" },
	{ 0x10DE01D8,	0x14623FCC,	"MSi GeForce Go 7400" },
	{ 0x10DE01D8,	0x14623FDF,	"MSi GeForce Go 7400" },
	{ 0x10DE01D8,	0x14624327,	"MSi GeForce Go 7400" },
 	{ 0x10DE01D8,	0x15092A30,	"GeForce Go 7400" }, // First International Computer Inc
	{ 0x10DE01D8,	0x152D0753,	"Quanta GeForce Go 7400" },
	{ 0x10DE01D8,	0x152D0763,	"Quante GeForce Go 7400" },
	{ 0x10DE01D8,	0x161F203D,	"Arima GeForce Go 7400" },
	{ 0x10DE01D8,	0x161F2052,	"Arima GeForce Go 7400" },
	{ 0x10DE01D8,	0x161F2054,	"Arima GeForce Go 7400" },
	{ 0x10DE01D8,	0x161F205D,	"Arima GeForce Go 7400" },
 	{ 0x10DE01D8,	0x1631C022,	"NEC GeForce Go 7400" },
	{ 0x10DE01D8,	0x173410D3,	"Fujitsu GeForce Go 7400" },
	{ 0x10DE01D8,	0x17AA2075,	"Lenovo GeForce Go 7400" },
	{ 0x10DE01D8,	0x17AA3833,	"Lenovo GeForce Go 7400" },
	{ 0x10DE01D8,	0x17AA39F5,	"Lenovo GeForce Go 7400" },
	{ 0x10DE01D8,	0x17AA6666,	"Lenovo GeForce Go 7400" },
	{ 0x10DE01D8,	0x17C0207F,	"Wistron GeForce Go 7400" },
 	{ 0x10DE01D8,	0x17C02083,	"Wistron GeForce Go 7400" },
	{ 0x10DE01D8,	0x17FF500E,	"Benq GeForce Go 7400" },
	{ 0x10DE01D8,	0x18940040,	"LG GeForce Go 7400" },
	{ 0x10DE01D8,	0x18640041,	"LG GeForce Go 7400" },
	{ 0x10DE01D8,	0x18640042,	"LG GeForce Go 7400" },
	{ 0x10DE01D8,	0x18640043,	"LG GeForce Go 7400" },
	{ 0x10DE01D8,	0x18640044,	"LG GeForce Go 7400" },
 	{ 0x10DE01D8,	0x18640045,	"LG GeForce Go 7400" },
	{ 0x10DE01D8,	0x18640046,	"LG GeForce Go 7400" },
	{ 0x10DE01D8,	0x18640047,	"LG GeForce Go 7400" },
	{ 0x10DE01D8,	0x1864007A,	"LG GeForce Go 7400" },
	{ 0x10DE01D8,	0x19614605,	"ESS GeForce Go 7400" },
	{ 0x10DE01D8,	0x19615607,	"ESS GeForce Go 7400" },
	{ 0x10DE01D8,	0x19915532,	"Topstar GeForce Go 7400" },
 	{ 0x10DE01D8,	0x19DB2174,	"GeForce Go 7400" }, // ??
	{ 0x10DE01D8,	0xC0181631,	"GeForce Go 7400" }, // ??

	{ 0x10DE01DA,	0x1028017D,	"Dell Quadro NVS 110M" },
	{ 0x10DE01DA,	0x10280407,	"Dell GeForce 7300 LE" },
	{ 0x10DE01DA,	0x11790001,	"Toshiba Quadro NVS 110M" },
	{ 0x10DE01DA,	0x11790002,	"Toshiba Quadro NVS 110M" },
	{ 0x10DE01DA,	0x11790010,	"Toshiba Quadro NVS 110M" },
	{ 0x10DE01DA,	0x1179FF00,	"Toshiba Quadro NVS 110M" },
	{ 0x10DE01DA,	0x1179FF10,	"Toshiba Quadro NVS 110M" },
	// 01E0 - 01EF
	// 01F0 - 01FF
	// 0200 - 020F
	// 0210 - 021F
	// 0220 - 022F
	// 0230 - 023F
	// 0240 - 024F
	// 0250 - 025F
	{ 0x10DE025B,	0x10480D23,	"ELSA Gloria4 700XGL" },
	{ 0x10DE025B,	0x10DE013B,	"nVidia Quadro4 700 XGL" },
	{ 0x10DE025B,	0x155410F3,	"Prolink Quadro4 700 XGL" },

	// 0260 - 026F
	// 0270 - 027F
	// 0280 - 028F
	// 0290 - 029F
	// 02A0 - 02AF
	// 02B0 - 02BF
	// 02C0 - 02CF
	// 02D0 - 02DF
	// 02E0 - 02EF
	// 02F0 - 02FF
	// 0300 - 030F
	// 0310 - 031F
	// 0320 - 032F
	{ 0x10DE032F,	0x17C02068,	"Wistron NV34GL" },
	// 0330 - 033F
	// 0340 - 034F
	{ 0x10DE0345,	0x1B130343,	"Jaton NV36.5" },

	{ 0x10DE0349,	0x1179FF00,	"Toshiba NV36M Pro" },

	{ 0x10DE034B,	0x1179FF00,	"Toshiba NV36MAP" },

	{ 0x10DE034F,	0x1179FF00,	"Toshiba NV36GL" },
	// 0350 - 035F
	// 0360 - 036F
	// 0370 - 037F
	// 0380 - 038F
	// 0390 - 039F
	{ 0x10DE0391,	0x104381F7,	"Asus GeForce 7600 GT" },
	{ 0x10DE0391,	0x104D820D,	"Sony GeForce 7600 GT" },
	{ 0x10DE0391,	0x104D9004,	"Sony GeForce 7600 GT" },
	{ 0x10DE0391,	0x104D9007,	"Sony GeForce 7600 GT" },
	{ 0x10DE0391,	0x105B0E10,	"Foxconn GeForce 7600 GT" },
	{ 0x10DE0391,	0x10B00401,	"Gainward GeForce 7600 GT" },
	{ 0x10DE0391,	0x10B00803,	"Gainward GeForce 7600 GT" },
	{ 0x10DE0391,	0x10DE033D,	"nVidia GeForce 7600 GT" },
	{ 0x10DE0391,	0x10DE0403,	"nVidia GeForce 7600 GT" },
	{ 0x10DE0391,	0x10DE047A,	"Galaxy GeForce 7600 GT" },
	{ 0x10DE0391,	0x14583417,	"Gigabyte GeForce 7600 GT" },
	{ 0x10DE0391,	0x1179FF00,	"Toshiba GeForce 7600 GT" },
	{ 0x10DE0391,	0x19F1201F,	"BFG GeForce 7600 GT" },
	{ 0x10DE0391,	0x19F120DE,	"Galaxy GeForce 7600 GT" },
	{ 0x10DE0391,	0x3842C615,	"EVGA GeForce 7600 GT" },

	{ 0x10DE0393,	0x00000400,	"Apple GeForce 7300GT" },
	// 03A0 - 03AF
	// 03B0 - 03BF
	// 03C0 - 03CF
	// 03D0 - 03DF
	// 03E0 - 03EF
	// 03F0 - 03FF
	// 0400 - 040F
	{ 0x10DE0402,	0x1043034D,	"Asus GeForce 8600 GT" },
	{ 0x10DE0402,	0x1043034E,	"Asus GeForce 8600 GT" },
	{ 0x10DE0402,	0x10431618,	"Asus GeForce 8600 GT" },
	{ 0x10DE0402,	0x104381F7,	"Asus GeForce 8600 GT" },
	{ 0x10DE0402,	0x10DE0439,	"Galaxy 8600GT" },
	{ 0x10DE0402,	0x10DE0505,	"Galaxy 8600GT" },
	{ 0x10DE0402,	0x14620890,	"MSi GeForce 8600 GT" },
	{ 0x10DE0402,	0x14620964,	"MSi GeForce 8600 GT" },
	{ 0x10DE0402,	0x174B8030,	"PC Partner GeForce 8600 GT" },

	{ 0x10DE0407,	0x101922D4,	"Elitegroup GeForce 8600M GT" },
	{ 0x10DE0407,	0x1025011D,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x1025011E,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x10250121,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x10250125,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x10250126,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x10250127,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x10250129,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x1025012B,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x10250136,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x1025013D,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x1025013F,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x10250142,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x10250143,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x10250145,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x10250146,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x1025015E,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x1028019C,	"Dell GeForce 8600M GT" },
	{ 0x10DE0407,	0x102801F1,	"Dell GeForce 8600M GT" },
	{ 0x10DE0407,	0x102801F2,	"Dell GeForce 8600M GT" },
	{ 0x10DE0407,	0x10280228,	"Dell GeForce 8600M GT" },
	{ 0x10DE0407,	0x10280229,	"Dell GeForce 8600M GT" },
	{ 0x10DE0407,	0x1028022E,	"Dell GeForce 8600M GT" },
	{ 0x10DE0407,	0x10431515,	"Asus GeForce 8600M GT" },
	{ 0x10DE0407,	0x10431588,	"Asus GeForce 8600M GT" },
	{ 0x10DE0407,	0x10431618,	"Asus GeForce 8600M GT" },
	{ 0x10DE0407,	0x10431632,	"Asus GeForce 8600M GT" },
	{ 0x10DE0407,	0x104314A2,	"Asus GeForce 8600M GT" },
	{ 0x10DE0407,	0x104381F7,	"Asus GeForce 8600M GT" },
	{ 0x10DE0407,	0x104D9005,	"Sony GeForce 8600M GT" },
	{ 0x10DE0407,	0x104D9016,	"Sony GeForce 8600M GT" },
	{ 0x10DE0407,	0x104D9018,	"Sony GeForce 8600M GT" },
	{ 0x10DE0407,	0x106B00A0,	"Apple GeForce 8600M GT" },
	{ 0x10DE0407,	0x106B00A3,	"Apple GeForce 8600M GT" },
	{ 0x10DE0407,	0x106B00A4,	"Apple GeForce 8600M GT" },

	{ 0x10DE040C,	0x103C30C5,	"HP Quadro FX 570M" },
	{ 0x10DE040C,	0x104381F7,	"Asus Quadro FX 570M" },
	{ 0x10DE040C,	0x10CF1423,	"Fujitsu Quadro FX 570M" },
	{ 0x10DE040C,	0x17AA20D9,	"Lenovo Quadro FX 570M" },
	// 0410 - 041F
	{ 0x10DE0410,	0x174B3058,	"PC Partner GeForce GT 330" },

	// 0420 - 042F
	{ 0x10DE0421,	0x1043034F,	"Asus GeForce 8500 GT" },
	{ 0x10DE0421,	0x1043050D,	"Asus GeForce 8500 GT" },
	{ 0x10DE0421,	0x1043050E,	"Asus GeForce 8500 GT" },
	{ 0x10DE0421,	0x10431617,	"Asus GeForce 8500 GT" },
	{ 0x10DE0421,	0x104381F7,	"Asus GeForce 8500 GT" },
	{ 0x10DE0421,	0x14620921,	"MSi GeForce 8500GT" },
	{ 0x10DE0421,	0x14620960,	"MSi GeForce 8500GT" },
	{ 0x10DE0421,	0x14620961,	"MSi GeForce 8500GT" },
	{ 0x10DE0421,	0x174B8010,	"PC Partner GeForce 8500 GT" },

	{ 0x10DE0426,	0x10338897,	"NEC GeForce 8400M GT" },
	{ 0x10DE0426,	0x104381F7,	"Asus GeForce 8400M GT" },
	{ 0x10DE0426,	0x104D9005,	"Sony GeForce 8400M GT" },
	{ 0x10DE0426,	0x104D9016,	"Sony GeForce 8400M GT" },
	{ 0x10DE0426,	0x104D9017,	"Sony GeForce 8400M GT" },
	{ 0x10DE0426,	0x104D9018,	"Sony GeForce 8400M GT" },
	{ 0x10DE0426,	0x104D902D,	"Sony GeForce 8400M GT" },
	{ 0x10DE0426,	0x104D9030,	"Sony GeForce 8400M GT" },
	{ 0x10DE0426,	0x19915584,	"Topstar GeForce 8400M GT" },
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
	{ 0x10DE05E0,	0x10DE064E,	"nVidia GeForce GTX 295" },
	{ 0x10DE05E0,	0x38421291,	"EVGA GeForce GTX 295" },

	{ 0x10DE05E1,	0x10DE0557,	"nVidia GeForce GTX 280" },

	{ 0x10DE05E2,	0x10438291,	"Asus GeForce GTX 260" },
	{ 0x10DE05E2,	0x10438298,	"Asus GeForce GTX 260" },
	{ 0x10DE05E2,	0x104382C4,	"Asus GeForce GTX 260" },
	{ 0x10DE05E2,	0x104382CF,	"Asus GeForce GTX 260" },
	{ 0x10DE05E2,	0x104382E3,	"Asus GeForce GTX 260" },
	{ 0x10DE05E2,	0x104382EB,	"ASUS ENGTX260" },
	{ 0x10DE05E2,	0x10B00801,	"Gainward GeForce GTX 260" },
	{ 0x10DE05E2,	0x10DE0585,	"nVidia GeForce GTX 260" },
	{ 0x10DE05E2,	0x10DE0617,	"nVidia GeForce GTX 260" },
	{ 0x10DE05E2,	0x16822390,	"HFX GeForce GTX 260" },
	{ 0x10DE05E2,	0x17870000,	"HIS GeForce GTX 260" },
	{ 0x10DE05E2,	0x196E064B,	"PNY GeForce GTX 260" },
	{ 0x10DE05E2,	0x19F10FA9,	"BFG GeForce GTX 260" },
	{ 0x10DE05E2,	0x34421260,	"Bihl GeForce GTX 260" },
	{ 0x10DE05E2,	0x34421262,	"Bihl GeForce GTX 260" },
	{ 0x10DE05E2,	0x73770000,	"Colorful GeForce GTX 260" },

	{ 0x10DE05E3,	0x10438320,	"Asus GeForce GTX 285" },
	{ 0x10DE05E3,	0x106B0000,	"Apple GeForce GTX 285" },
	{ 0x10DE05E3,	0x10DE065B,	"nVidia GeForce GTX 285" },
	{ 0x10DE05E3,	0x38421080,	"EVGA GeForce GTX 285" },
	{ 0x10DE05E3,	0x38421187,	"EVGA GeForce GTX 285" },

	{ 0x10DE05E6,	0x10B00401,	"Gainward GeForce GTX 285" },
	{ 0x10DE05E6,	0x38421171,	"EVGA GeForce GTX 275" },

	{ 0x10DE05E7,	0x10DE0595,	"nVidia Tesla T10 Processor" },
	{ 0x10DE05E7,	0x10DE066A,	"nVidia Tesla C1060" },
	{ 0x10DE05E7,	0x10DE068F,	"nVidia Tesla T10 Processor" },
	{ 0x10DE05E7,	0x10DE0697,	"nVidia Tesla M1060" },
	{ 0x10DE05E7,	0x10DE0714,	"nVidia Tesla M1060" },
	{ 0x10DE05E7,	0x10DE0743,	"nVidia Tesla M1060" },

	{ 0x10DE05EA,	0x10DE0738,	"nVidia GeForce GTX 260" },
	{ 0x10DE05EA,	0x10DE0753,	"nVidia GeForce GTX 260" },
	{ 0x10DE05EA,	0x10DE8086,	"nVidia GeForce GTX 260" },

	{ 0x10DE05EB,	0x10DE0705,	"nVidia GeForce GTX 295" },
	{ 0x10DE05EB,	0x19F110C2,	"BFG GeForce GTX 295" },
	// 05F0 - 05FF
	// 0600 - 060F
	{ 0x10DE0600,	0x10438268,	"Asus GeForce 8800 GTS" },
	{ 0x10DE0600,	0x1043826C,	"Asus GeForce 8800 GTS" },
	{ 0x10DE0600,	0x10DE0000,	"Abit GeForce 8800 GTS" },
	{ 0x10DE0600,	0x10DE0502,	"nVidia GeForce 8800 GTS" },
	{ 0x10DE0600,	0x19F10719,	"BFG GeForce 8800 GTS" },

	{ 0x10DE0603,	0x174B1058,	"PC Partner GeForce GT 230" },
	{ 0x10DE0603,	0x1B0A9044,	"Pegatron GeForce GT 230" },

	{ 0x10DE0604,	0x10DE0504,	"nVidia GeForce 9800 GX2" },

	{ 0x10DE0605,	0x10DE0612,	"nVidia GeForce 9800 GT" },
	{ 0x10DE0605,	0x10DE062D,	"nVidia GeForce 9800 GT" },
	{ 0x10DE0605,	0x145834A2,	"Gigabyte GV-N98TOC-512H" },
	{ 0x10DE0605,	0x14621460,	"MSi GeForce 9800 GT" },

	{ 0x10DE0607,	0x10DE0736,	"nVidia GeForce GTS 240" },

	{ 0x10DE0608,	0x1028019C,	"Dell GeForce 9800M GTX" },
	{ 0x10DE0608,	0x102802A1,	"Dell GeForce 9800M GTX" },
	{ 0x10DE0608,	0x10432003,	"Asus GeForce 9800M GTX" },
	{ 0x10DE0608,	0x1179FF01,	"Toshiba GeForce 9800M GTX" },
	{ 0x10DE0608,	0x15580481,	"Clevo GeForce 9800M GTX" },
	{ 0x10DE0608,	0x15880577,	"Solidum GeForce 9800M GTX" },
	{ 0x10DE0608,	0x161F207A,	"Arima GeForce 9800M GTX" },

	{ 0x10DE0609,	0x1028019B,	"Dell GeForce 8800M GTS" },
	{ 0x10DE0609,	0x103C30D4,	"HP GeForce 8800M GTS" },
	{ 0x10DE0609,	0x104381F7,	"Asus GeForce 8800M GTS" },
	{ 0x10DE0609,	0x106B00A7,	"Apple GeForce 8800M GS" },
	{ 0x10DE0609,	0x107B0690,	"Gateway GeForce 8800M GTS" },
	{ 0x10DE0609,	0x11700121,	"Inventec GeForce 8800M GTS" },
	{ 0x10DE0609,	0x152D0770,	"Quanta GeForce 8800M GTS" },
	// 0610 - 061F
	{ 0x10DE0611,	0x104381F7,	"Asus GeForce 8800 GT" },
	{ 0x10DE0611,	0x10DE053C,	"nVidia GeForce 8800 GT" },
	{ 0x10DE0611,	0x14621171,	"MSi GeForce 8800 GT" },
	{ 0x10DE0611,	0x14621172,	"MSi GeForce 8800 GT" },
	{ 0x10DE0611,	0x174B9210,	"PC Partner GeForce 8800 GT" },
	{ 0x10DE0611,	0x1ACC8582,	"Point of View GeForce 8800 GT" },
	{ 0x10DE0611,	0x3842C802,	"EVGA GeForce 8800 GT" },

	{ 0x10DE0612,	0x104382A6,	"Asus GeForce 9800 GTX+" },
	{ 0x10DE0612,	0x10DE0571,	"nVidia GeForce 9800 GTX+" },
	{ 0x10DE0612,	0x10DE0592,	"nVidia GeForce 9800 GTX+" },
	{ 0x10DE0612,	0x3842C842,	"EVGA GeForce 9800 GTX+" },
	{ 0x10DE0612,	0x3842C875,	"EVGA GeForce 9800 GTX+" },

	{ 0x10DE0615,	0x104382E6,	"Asus GeForce GTS 250" },
	{ 0x10DE0615,	0x104382FB,	"Asus GeForce GTS 250" },
	{ 0x10DE0615,	0x10438303,	"Asus GeForce GTS 250" },
	{ 0x10DE0615,	0x10438305,	"Asus GeForce GTS 250" },
	{ 0x10DE0615,	0x10438312,	"Asus GeForce GTS 250" },
	{ 0x10DE0615,	0x10438338,	"Asus GeForce GTS 250" },
	{ 0x10DE0615,	0x10438339,	"Asus GeForce GTS 250" },
	{ 0x10DE0615,	0x1043833C,	"Asus GeForce GTS 250" },
	{ 0x10DE0615,	0x10438345,	"Asus GeForce GTS 250" },
	{ 0x10DE0615,	0x10480F67,	"ELSA GeForce GTS 250" },
	{ 0x10DE0615,	0x10B00401,	"Gainward GeForce GTS 250" },
	{ 0x10DE0615,	0x10190000,	"Elitegroup GeForce GTS 250" },
	{ 0x10DE0615,	0x107D2723,	"Leadteck GeForce GTS 250" },
	{ 0x10DE0615,	0x10DE0592,	"Palit GeForce GTS 250" },
	{ 0x10DE0615,	0x10DE0593,	"Palit GeForce GTS 250" },
	{ 0x10DE0615,	0x10DE0652,	"Palit GeForce GTS 250" },
	{ 0x10DE0615,	0x10DE0719,	"Palit GeForce GTS 250" },
	{ 0x10DE0615,	0x10DE079E,	"Palit GeForce GTS 250" },
	{ 0x10DE0615,	0x11503842,	"TMC GeForce GTS 250" }, // Thinking Machines Corporation
	{ 0x10DE0615,	0x11513842,	"JAE GeForce GTS 250" },
	{ 0x10DE0615,	0x11553842,	"Pine GeForce GTS 250" },
	{ 0x10DE0615,	0x11563842,	"Periscope GeForce GTS 250" },
	{ 0x10DE0615,	0x145834C7,	"Gigabyte GeForce GTS 250" },
	{ 0x10DE0615,	0x145834CB,	"Gigabyte GeForce GTS 250" },
	{ 0x10DE0615,	0x145834E7,	"Gigabyte GeForce GTS 250" },
	{ 0x10DE0615,	0x145834E9,	"Gigabyte GeForce GTS 250" },
	{ 0x10DE0615,	0x14621542,	"MSi GeForce GTS 250" },
	{ 0x10DE0615,	0x14621543,	"MSi GeForce GTS 250" },
	{ 0x10DE0615,	0x14628090,	"MSi GeForce GTS 250" },
	{ 0x10DE0615,	0x16822600,	"XFX GeForce GTS 250" },
	{ 0x10DE0615,	0x16822601,	"XFX GeForce GTS 250" },
	{ 0x10DE0615,	0x16822605,	"XFX GeForce GTS 250" },
	//{ 0x10DE0615,	0x196E0593,	" GeForce GTS 250" },
	{ 0x10DE0615,	0x19DA2103,	"Zotac GeForce GTS 250" },
	{ 0x10DE0615,	0x19DA3056,	"Zotac GeForce GTS 250" },
	{ 0x10DE0615,	0x19DA5103,	"Zotac GeForce GTS 250" },
	//{ 0x10DE0615,	0x19F1,	"BFG GeForce GTS 250" },
	{ 0x10DE0615,	0x1ACC9252,	"Point of View GeForce GTS 250" },
	{ 0x10DE0615,	0x1ACC9253,	"Point of View GeForce GTS 250" },
	{ 0x10DE0615,	0x1ACC925C,	"Point of View GeForce GTS 250" },
	{ 0x10DE0615,	0x1B0A9038,	"Pegatron GeForce GTS 250" },
	{ 0x10DE0615,	0x38421145,	"EVGA GeForce GTS 250" },
	{ 0x10DE0615,	0x38421158,	"EVGA GeForce GTS 250" },
	//{ 0x10DE0615,	0x7377,	"Colorful GeForce GTS 250" },

	{ 0x10DE0618,	0x1025028E,	"Acer GeForce GTX 260M" },
	{ 0x10DE0618,	0x102802A1,	"Dell GeForce GTX 260M" },
	{ 0x10DE0618,	0x102802A2,	"Dell GeForce GTX 260M" },
	{ 0x10DE0618,	0x10431A52,	"Asus GeForce GTX 260M" },
	{ 0x10DE0618,	0x10432028,	"Asus GeForce GTX 170M" },
	{ 0x10DE0618,	0x1043202B,	"Asus GeForce GTX 680" },
	{ 0x10DE0618,	0x10432033,	"Asus GeForce GTX 260M" },
	{ 0x10DE0618,	0x15580481,	"Clevo/Kapok GeForce GTX 260M" },
	{ 0x10DE0618,	0x15580577,	"Clevo/Kapok GeForce GTX 260M" },
	{ 0x10DE0618,	0x15580860,	"Clevo/Kapok GeForce GTX 260M" },
	// 0620 - 062F
	{ 0x10DE0622,	0x104382AC,	"Asus EN9600GT Magic" },
	{ 0x10DE0622,	0x10DE0545,	"nVidia GeForce 9600GT" },
	{ 0x10DE0622,	0x10621272,	"MSi GeForce 9600GT" },
	{ 0x10DE0622,	0x10621278,	"MSi GeForce 9600GT" },
	{ 0x10DE0622,	0x10621279,	"MSi GeForce 9600GT" },
	{ 0x10DE0622,	0x10621432,	"MSi GeForce 9600GT" },
	// 0630 - 063F
	// 0640 - 064F
	{ 0x10DE0640,	0x101910BD,	"Elitegroup GeForge 9500 GT" },
	{ 0x10DE0640,	0x101910C0,	"Elitegroup GeForge 9500 GT" },
	{ 0x10DE0640,	0x1043829A,	"Asus GeForge 9500 GT" },
	{ 0x10DE0640,	0x104382B4,	"Asus GeForge 9500 GT" },
	{ 0x10DE0640,	0x104382FD,	"Asus GeForge 9500 GT" },
	{ 0x10DE0640,	0x106B00AD,	"Apple GeForge 9500 GT" },
	{ 0x10DE0640,	0x106B00B3,	"Apple GeForge 9500 GT" },
	{ 0x10DE0640,	0x106B061B,	"Apple GeForge 9500 GT" },
	{ 0x10DE0640,	0x10B01401,	"Gainward GeForge 9500 GT" },
	{ 0x10DE0640,	0x10DE0551,	"nVidia GeForge 9500 GT" },
	{ 0x10DE0640,	0x10DE057D,	"nVidia GeForge 9500 GT" },
	{ 0x10DE0640,	0x10DE0648,	"nVidia GeForge 9500 GT" },
	{ 0x10DE0640,	0x10DE077F,	"Inno3D GeForge 9500GT HDMI" },
	{ 0x10DE0640,	0x14583498,	"GigaByte GeForge 9500 GT" },
	{ 0x10DE0640,	0x145834A9,	"GigaByte GeForge 9500 GT" },
	{ 0x10DE0640,	0x14621290,	"MSi GeForge 9500 GT" },
	{ 0x10DE0640,	0x14621291,	"MSi GeForge 9500 GT" },
	{ 0x10DE0640,	0x14621575,	"MSi GeForge 9500 GT" },
	{ 0x10DE0640,	0x16423796,	"Bitland GeForge 9500 GT" },
	{ 0x10DE0640,	0x1682400A,	"XFX GeForge 9500 GT" },
	{ 0x10DE0640,	0x196E0643,	"PNY GeForge 9500GT" },
	{ 0x10DE0640,	0x19DA7046,	"Zotac GeForge 9500 GT" },
	{ 0x10DE0640,	0x1ACC9091,	"Point of View GeForge 9500 GT" },
	{ 0x10DE0640,	0x3842C958,	"EVGA GeForge 9500 GT" },

	{ 0x10DE0647,	0x106B00A9,	"Apple GeForge 9600M GT" },
	{ 0x10DE0647,	0x106B00B0,	"Apple GeForge 9600M GT" },
	{ 0x10DE0647,	0x106B00B3,	"Apple GeForge 9600M GT" },
	{ 0x10DE0647,	0x106B00BC,	"Apple GeForge 9600M GT" },

	{ 0x10DE0648,	0x1043900F,	"Asus GeForge 9600M GS" },

	{ 0x10DE0649,	0x10439013,	"Asus GeForge 9600M GT" },
	{ 0x10DE0649,	0x1043202D,	"Asus GeForge GT 220M" },
	// 0650 - 065F
	{ 0x10DE065C,	0x10280250,	"Dell Quadro FX 770M" },
	{ 0x10DE065C,	0x103C30E7,	"HP Quadro FX 770M" },
	{ 0x10DE065C,	0x10DE058B,	"nVidia Quadro FX 770M" },
	{ 0x10DE065C,	0x10DE0734,	"nVidia Quadro FX 770M" }, // 512MB
	{ 0x10DE065C,	0x17341147,	"Fujitsu Quadro FX 770M" },
	// 0660 - 066F
	// 0670 - 067F
	// 0680 - 068F
	// 0690 - 069F
	// 06A0 - 06AF
	// 06B0 - 06BF
	// 06C0 - 06CF
	{ 0x10DE06C0,	0x10438359,	"Asus GeForce GTX 480" },
	{ 0x10DE06C0,	0x10DE075F,	"nVidia GeForce GTX 480" },
	{ 0x10DE06C0,	0x19DA1152,	"Zotac GeForce GTX 480" },
	{ 0x10DE06C0,	0x38421480,	"EVGA GTX 480" },
	{ 0x10DE06C0,	0x38421482,	"EVGA GTX 480" },

	{ 0x10DE06CD,	0x10DE079F,	"Point of View GeForce GTX 470" },
	{ 0x10DE06CD,	0x10DE979F,	"nVidia GeForce GTX 470" },
	{ 0x10DE06CD,	0x145834F5,	"GigaByte GeForce GTX 470" },
  { 0x10DE06CD,	0x14622220,	"MSi GeForce GTX 470 Twin Frozr II" },
	{ 0x10DE06CD,	0x19DA0010,	"Zotac GTX 470" },
	{ 0x10DE06CD,	0x19DA1153,	"Zotac GeForce GTX 470" },
	{ 0x10DE06CD,	0x38421472,	"EVGA GeForce GTX 470" },
	// 06D0 - 06DF
	{ 0x10DE06D1,	0x10DE0771,	"nVidia Tesla C2050" },
	{ 0x10DE06D1,	0x10DE0772,	"nVidia Tesla C2070" },

	{ 0x10DE06D2,	0x10DE0774,	"nVidia Tesla M2070" },
	{ 0x10DE06D2,	0x10DE0830,	"nVidia Tesla M2070" },
	{ 0x10DE06D2,	0x10DE0842,	"nVidia Tesla M2070" },
	{ 0x10DE06D2,	0x10DE088F,	"nVidia Tesla X2070" },
	{ 0x10DE06D2,	0x10DE0908,	"nVidia Tesla M2070" },

	{ 0x10DE06D8,	0x103C076F,	"HP Quadro 6000" },
	{ 0x10DE06D8,	0x10DE076F,	"nVidia Quadro 6000" },

	{ 0x10DE06D9,	0x103C0770,	"HP Quadro 5000" },
	{ 0x10DE06D9,	0x10DE0770,	"nVidia Quadro 5000" },

	{ 0x10DE06DA,	0x1028081A,	"Dell Quadro 5000M" },
	{ 0x10DE06DA,	0x103C1520,	"HP Quadro 5000M" },

	{ 0x10DE06DD,	0x103C0780,	"HP Quadro 4000" },
	{ 0x10DE06DD,	0x106B0000,	"Apple Quadro 4000" },
	{ 0x10DE06DD,	0x10DE0780,	"nVidia Quadro 4000" },

	{ 0x10DE06DE,	0x10DE0773,	"nVidia Tesla S2050" },
	{ 0x10DE06DE,	0x10DE077A,	"nVidia Tesla M2050" },
	{ 0x10DE06DE,	0x10DE082F,	"nVidia Tesla M2050" },
	{ 0x10DE06DE,	0x10DE0830,	"nVidia Tesla M2070" },
	{ 0x10DE06DE,	0x10DE0831,	"nVidia Tesla M2070" },
	{ 0x10DE06DE,	0x10DE0832,	"nVidia Tesla M2070" },
	{ 0x10DE06DE,	0x10DE0840,	"nVidia Tesla X2070" },
	{ 0x10DE06DE,	0x10DE0842,	"nVidia Tesla M2050" },
	{ 0x10DE06DE,	0x10DE0843,	"nVidia Tesla M2050" },
	{ 0x10DE06DE,	0x10DE0846,	"nVidia Tesla M2050" },
	{ 0x10DE06DE,	0x10DE0866,	"nVidia Tesla M2050" },
	{ 0x10DE06DE,	0x10DE0907,	"nVidia Tesla M2050" },
	{ 0x10DE06DE,	0x10DE091E,	"nVidia Tesla M2050" },

	{ 0x10DE06DF,	0x10DE0842,	"nVidia Tesla M2070-Q" },
	{ 0x10DE06DF,	0x10DE084D,	"nVidia Tesla M2070-Q" },
	{ 0x10DE06DF,	0x10DE087F,	"nVidia Tesla M2070-Q" },
	// 06E0 - 06EF
	{ 0x10DE06E4,	0x10438322,	"Asus EN8400GS" },
	{ 0x10DE06E4,	0x14583475,	"GV-NX84S256HE [GeForce 8400 GS]" },
	{ 0x10DE06E4,	0x14621160,	"MSi GeForce 8400 GS" },
	{ 0x10DE06E4,	0x14621164,	"MSi GeForce 8400 GS" },
	{ 0x10DE06E4,	0x3842C802,	"EVGA GeForce 8400 GS" },

	{ 0x10DE06E8,	0x10280262,	"Dell GeForce 9200M GS" },
	{ 0x10DE06E8,	0x10280271,	"Dell GeForce 9200M GS" },
	{ 0x10DE06E8,	0x10280272,	"Dell GeForce 9200M GS" },
	{ 0x10DE06E8,	0x103C30F4,	"HP GeForce 9200M GS" },
	{ 0x10DE06E8,	0x103C30F7,	"HP GeForce 9200M GS" },
	{ 0x10DE06E8,	0x103C3603,	"HP GeForce 9200M GS" },
	{ 0x10DE06E8,	0x103C360B,	"HP GeForce 9200M GE" },
	{ 0x10DE06E8,	0x103C3621,	"HP GeForce 9200M GE" },
	{ 0x10DE06E8,	0x103C3629,	"HP GeForce 9200M GE" },
	{ 0x10DE06E8,	0x10432008,	"Asus GeForce 9200M GE" },
	{ 0x10DE06E8,	0x107B0900,	"Gateway GeForce 9200M GE" },
	{ 0x10DE06E8,	0x11790001,	"Toshiba GeForce 9200M GE" },
	{ 0x10DE06E8,	0x144DC041,	"Samsung GeForce 9200M GE" },
	{ 0x10DE06E8,	0x144DC042,	"Samsung GeForce 9200M GE" },
	{ 0x10DE06E8,	0x144DC048,	"Samsung GeForce 9200M GE" },
	{ 0x10DE06E8,	0x144DC04A,	"Samsung GeForce 9200M GE" },
	{ 0x10DE06E8,	0x144DC521,	"Samsung GeForce 9200M GE" },
	{ 0x10DE06E8,	0x144DC524,	"Samsung GeForce 9200M GE" },
	{ 0x10DE06E8,	0x152D0772,	"Quanta GeForce 9200M GE" },
	{ 0x10DE06E8,	0x152D0773,	"Quanta GeForce 9200M GE" },
	{ 0x10DE06E8,	0x152D0774,	"Quanta GeForce 9200M GE" },
	{ 0x10DE06E8,	0x152D0775,	"Quanta GeForce 9200M GE" },
	{ 0x10DE06E8,	0x17341146,	"Fujitsu GeForce 9200M GE" },
	{ 0x10DE06E8,	0x18541772,	"LG GeForce 9200M GE" },
	{ 0x10DE06E8,	0x18541773,	"LG GeForce 9200M GE" },
	{ 0x10DE06E8,	0x18541774,	"LG GeForce 9200M GE" },
	{ 0x10DE06E8,	0x18541775,	"LG GeForce 9200M GE" },
	{ 0x10DE06E8,	0x19614605,	"ESS GeForce 9200M GE" },
	{ 0x10DE06E8,	0x19615584,	"ESS GeForce 9200M GE" },
	{ 0x10DE06E8,	0x1B0A000E,	"Pegatron GeForce 9200M GE" },

	{ 0x10DE06E9,	0x10430510,	"Asus GeForce 9300M GS" },
	// 06F0 - 06FF
	{ 0x10DE06FF,	0x10DE0711,	"nVidia HICx8 + Graphics" },
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
	// 07F0 - 07FF
	// 0800 - 080F
	// 0810 - 081F
	// 0820 - 082F
	// 0830 - 083F
	// 0840 - 084F
	{ 0x10DE084B,	0x10250227,	"Acer GeForce 9200" },
	{ 0x10DE084B,	0x10250228,	"Acer GeForce 9200" },
	{ 0x10DE084B,	0x103C2A6E,	"HP GeForce 9200" },
	{ 0x10DE084B,	0x1631E03B,	"NEC GeForce 9200" },
	// 0850 - 085F
	// 0860 - 086F
	{ 0x10DE086A,	0x1458D000,	"Gigabyte GeForce 9400" },
	// 0870 - 087F
	{ 0x10DE0873,	0x104319B4,	"Asus GeForce G102M" },

	{ 0x10DE0876,	0x103C3651,	"HP ION" },
	{ 0x10DE0876,	0x10438402,	"Asus ION" },
	{ 0x10DE0876,	0x144DC056,	"Samsung ION" },
	{ 0x10DE0876,	0x17AA38F8,	"Lenovo ION" },
	{ 0x10DE0876,	0x18491202,	"ASRock ION" },
	{ 0x10DE0876,	0x18540148,	"LG ION" },
	{ 0x10DE0876,	0x18540149,	"LG ION" },

	{ 0x10DE087D,	0x10250222,	"Acer ION" },
	{ 0x10DE087D,	0x17AA301D,	"Lenovo ION" },
	// 0880 - 088F
	// 0890 - 089F
	// 08A0 - 08AF
	{ 0x10DE08A0,	0x106B00C0,	"Apple GeForce 320M" },
	{ 0x10DE08A0,	0x106B00C2,	"Apple GeForce 320M" },
	{ 0x10DE08A0,	0x106B00C5,	"Apple GeForce 320M" },
	{ 0x10DE08A0,	0x106B00C9,	"Apple GeForce 320M" },
	{ 0x10DE08A0,	0x106B00CE,	"Apple GeForce 320M" },

	{ 0x10DE08A2,	0x106B00D4,	"Apple GeForce 320M" },
	// 08B0 - 08BF
	{ 0x10DE08B2,	0x10431592,	"Asus GeForce 300M" },
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
	{ 0x10DE0A20,	0x10438311,	"Asus GeForce GT 220" },
	{ 0x10DE0A20,	0x10DE069A,	"nVidia GeForce GT 220" },
	{ 0x10DE0A20,	0x14621910,	"MSi GeForce GT 220" },
	{ 0x10DE0A20,	0x14621911,	"MSi GeForce GT 220" },
	{ 0x10DE0A20,	0x14621990,	"MSi GeForce GT 220" },
	{ 0x10DE0A20,	0x16423920,	"Bitland GeForce GT 220" },

	{ 0x10DE0A28,	0x10338897,	"NEC GeForce GT 230" },
	{ 0x10DE0A28,	0x103C1000,	"HP GeForce GT 230" },
	{ 0x10DE0A28,	0x103C2AA7,	"HP GeForce GT 230" },
	{ 0x10DE0A28,	0x103C363C,	"HP GeForce GT 230" },
	{ 0x10DE0A28,	0x103C363E,	"HP GeForce GT 230" },
	{ 0x10DE0A28,	0x103C3659,	"HP GeForce GT 230" },
	{ 0x10DE0A28,	0x103C365C,	"HP GeForce GT 230" },
	{ 0x10DE0A28,	0x103C7001,	"HP GeForce GT 230" },
	{ 0x10DE0A28,	0x10432031,	"Asus GeForce GT 230" },
	{ 0x10DE0A28,	0x10719072,	"Mitac GeForce GT 230" },
	{ 0x10DE0A28,	0x1179FF00,	"Toshiba GeForce GT 230" },
	{ 0x10DE0A28,	0x1179FF15,	"Toshiba GeForce GT 230" },
	{ 0x10DE0A28,	0x1179FF16,	"Toshiba GeForce GT 230" },
	{ 0x10DE0A28,	0x1179FF50,	"Toshiba GeForce GT 230" },
	{ 0x10DE0A28,	0x144DC064,	"Samsung GeForce GT 230" },
	{ 0x10DE0A28,	0x152D0815,	"Quanta GeForce GT 230" },
	{ 0x10DE0A28,	0x18540807,	"LG GeForce GT 230" },
	{ 0x10DE0A28,	0x1B0A903B,	"Pegatron GeForce GT 230" },
	// 0A30 - 0A3F
	{ 0x10DE0A34,	0x10250200,	"Acer GeForce GT 240M" },
	{ 0x10DE0A34,	0x10250201,	"Acer GeForce GT 240M" },
	{ 0x10DE0A34,	0x1025020E,	"Acer GeForce GT 240M" },
	{ 0x10DE0A34,	0x10250219,	"Acer GeForce GT 240M" },
	{ 0x10DE0A34,	0x1025021E,	"Acer GeForce GT 240M" },
	{ 0x10DE0A34,	0x10250252,	"Acer GeForce GT 240M" },
	{ 0x10DE0A34,	0x10250259,	"Acer GeForce GT 240M" },
	{ 0x10DE0A34,	0x1025026B,	"Acer GeForce GT 240M" },
	{ 0x10DE0A34,	0x10250273,	"Acer GeForce GT 240M" },
	{ 0x10DE0A34,	0x10250299,	"Acer GeForce GT 240M" },
	{ 0x10DE0A34,	0x102802A2,	"Dell GeForce GT 240M" },
	{ 0x10DE0A34,	0x10431AE2,	"Asus GeForce GT 240M" },
	{ 0x10DE0A34,	0x1043202A,	"Asus GeForce GT 240M" },
	{ 0x10DE0A34,	0x10432031,	"Asus GeForce GT 240M" },
	{ 0x10DE0A34,	0x10432034,	"Asus GeForce GT 240M" },
	{ 0x10DE0A34,	0x10432036,	"Asus GeForce GT 240M" },
	{ 0x10DE0A34,	0x1043203A,	"Asus GeForce GT 240M" },
	{ 0x10DE0A34,	0x10432040,	"Asus GeForce GT 240M" },
	{ 0x10DE0A34,	0x104D905E,	"Sony GeForce GT 240M" },
	{ 0x10DE0A34,	0x104D9060,	"Sony GeForce GT 240M" },
	{ 0x10DE0A34,	0x10719072,	"Mitac GeForce GT 240M" },
	{ 0x10DE0A34,	0x14621013,	"MSi GeForce GT 240M" },
	{ 0x10DE0A34,	0x1462102E,	"MSi GeForce GT 240M" },
	{ 0x10DE0A34,	0x14621031,	"MSi GeForce GT 240M" },
	{ 0x10DE0A34,	0x14624570,	"MSi GeForce GT 240M" },
	{ 0x10DE0A34,	0x14C00042,	"Compal GeForce GT 240M" },
	{ 0x10DE0A34,	0x152D0828,	"Quanta GeForce GT 240M" },
	{ 0x10DE0A34,	0x16423928,	"Bitland GeForce GT 240M" },
	{ 0x10DE0A34,	0x1734118D,	"Fujitsu GeForce GT 240M" },
	{ 0x10DE0A34,	0x1734118E,	"Fujitsu GeForce GT 240M" },
	{ 0x10DE0A34,	0x17AA2144,	"Lenovo GeForce GT 240M" },
	{ 0x10DE0A34,	0x17AA38CD,	"Lenovo GeForce GT 240M" },
	{ 0x10DE0A34,	0x17AA38FD,	"Lenovo GeForce GT 240M" },
	{ 0x10DE0A34,	0x17AA38FF,	"Lenovo GeForce GT 240M" },
	{ 0x10DE0A34,	0x17C010D0,	"Wistron GeForce GT 240M" },
	{ 0x10DE0A34,	0x17C0208D,	"Wistron GeForce GT 240M" },

	{ 0x10DE0A3D,	0x10280443,	"Dell N11P-ES" },
	{ 0x10DE0A3D,	0x103C1521,	"HP N11P-ES" },
	{ 0x10DE0A3D,	0x104D905E,	"Sony N11P-ES" },
	{ 0x10DE0A3D,	0x104D9060,	"Sony N11P-ES" },
	{ 0x10DE0A3D,	0x104D9067,	"Sony N11P-ES" },
	{ 0x10DE0A3D,	0x17AA2143,	"Lenovo N11P-ES" },
	{ 0x10DE0A3D,	0x17AA2144,	"Lenovo N11P-ES" },
	{ 0x10DE0A3D,	0x17AA2145,	"Lenovo N11P-ES" },
	// 0A40 - 0A4F
	// 0A50 - 0A5F
	// 0A60 - 0A6F
	{ 0x10DE0A64,	0x1025063C,	"Acer ION" },
	{ 0x10DE0A64,	0x103C2AAD,	"HP ION" },
	{ 0x10DE0A64,	0x10430010,	"Asus ION2" },
	{ 0x10DE0A64,	0x1043841F,	"Asus ION" },
	{ 0x10DE0A64,	0x1043842F,	"Asus ION" },
	{ 0x10DE0A64,	0x10438455,	"Asus ION" },
	{ 0x10DE0A64,	0x1043845B,	"Asus ION" },
	{ 0x10DE0A64,	0x1043845E,	"Asus ION" },
	{ 0x10DE0A64,	0x17AA3605,	"Lenovo ION" },
	{ 0x10DE0A64,	0x18490A64,	"ASRock ION" },
	{ 0x10DE0A64,	0x1B0A00CE,	"Pegatron ION" },
   	{ 0x10DE0A64,	0x1B0A00D7,	"Pegatron ION" },

	{ 0x10DE0A65,	0x10438334,	"Asus GeForce 210" },
	{ 0x10DE0A65,	0x10438353,	"Asus GeForce 210" },
	{ 0x10DE0A65,	0x10438354,	"Asus GeForce 210" },
	{ 0x10DE0A65,	0x10DE0794,	"nVidia GeForce 210" },
	{ 0x10DE0A65,	0x10DE0847,	"nVidia GeForce 210" },
	{ 0x10DE0A65,	0x145834D5,	"GigaByte GeForce 210" },
	{ 0x10DE0A65,	0x145834EF,	"GigaByte GeForce 210" },
	{ 0x10DE0A65,	0x16822941,	"XFX GeForce 210" },
	{ 0x10DE0A6C,	0x1028040B,	"Dell NVS 3100M" },
	{ 0x10DE0A6C,	0x17AA2142,	"Lenovo NVS 3100M" },
	// 0A70 - 0A7F
	{ 0x10DE0A70,	0x10438458,	"Asus GeForce 310M" },
	{ 0x10DE0A70,	0x10438459,	"Asus GeForce 310M" },
	{ 0x10DE0A70,	0x17AA3605,	"Lenovo ION" },

	{ 0x10DE0A73,	0x17AA3607,	"Lenovo ION" },
	{ 0x10DE0A73,	0x17AA3610,	"Lenovo ION" },

	{ 0x10DE0A74,	0x16423940,	"Bitland GeForce G210M" },
	{ 0x10DE0A74,	0x1B0A903A,	"Pegatron GeForce G210" },

	{ 0x10DE0A75,	0x10DE0798,	"nVidia GeForce 310M" },
	{ 0x10DE0A75,	0x17AA3605,	"Lenovo ION" },
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
	{ 0x10DE0CA3,	0x10438326,	"Asus GeForce GT 240" },
	{ 0x10DE0CA3,	0x10438328,	"Asus GeForce GT 240" },
	{ 0x10DE0CA3,	0x1043832A,	"Asus GeForce GT 240" },
	{ 0x10DE0CA3,	0x1043832E,	"Asus GeForce GT 240" },
	{ 0x10DE0CA3,	0x10438335,	"Asus GeForce GT 240" },
	{ 0x10DE0CA3,	0x145834E2,	"GigaByte GeForce GT 240" },
	{ 0x10DE0CA3,	0x145834E5,	"GigaByte GeForce GT 240" },
	{ 0x10DE0CA3,	0x145834E6,	"GigaByte GeForce GT 240" },
	{ 0x10DE0CA3,	0x14621900,	"MSi GeForce GT 230" },
	{ 0x10DE0CA3,	0x14621913,	"MSi GeForce GT 230" },
	{ 0x10DE0CA3,	0x14622070,	"MSi GeForce GT 240" },
	{ 0x10DE0CA3,	0x14622072,	"MSi GeForce GT 240" },
	{ 0x10DE0CA3,	0x14622073,	"MSi GeForce GT 240" },
	{ 0x10DE0CA3,	0x14628010,	"MSi GeForce GT 240" },
	{ 0x10DE0CA3,	0x14628041,	"MSi VN240GT-MD1G" },
	{ 0x10DE0CA3,	0x16423926,	"Bitland GeForce GT 230" },
	{ 0x10DE0CA3,	0x196E0010,	"PNY GeForce GT 240" },
	{ 0x10DE0CA3,	0x196E069D,	"PNY GeForce GT 240" },
	{ 0x10DE0CA3,	0x196E075B,	"PNY GeForce GT 240" },
	{ 0x10DE0CA3,	0x19DA1142,	"Zotac GeForce GT 240" },
	{ 0x10DE0CA3,	0x19DA1143,	"Zotac GeForce GT 240" },
	{ 0x10DE0CA3,	0x19DA1144,	"Zotac GeForce GT 240" },
	{ 0x10DE0CA3,	0x19DA2130,	"Zotac GeForce GT 240" },
	{ 0x10DE0CA3,	0x19DA2134,	"Zotac GeForce GT 240" },

	{ 0x10DE0CA9,	0x16423942,	"Bitland GeForce GTS 250M" },

	{ 0x10DE0CAF,	0x10DE0782,	"nVidia GeForce GT 335M" },
	// 0CB0 - 0CBF
	{ 0x10DE0CB0,	0x10250367,	"Acer GeForce GTS 350M" },
	{ 0x10DE0CB0,	0x10250368,	"Acer GeForce GTS 350M" },
	{ 0x10DE0CB0,	0x10250422,	"Acer GeForce GTS 350M" },
	{ 0x10DE0CB0,	0x10250463,	"Acer GeForce GTS 350M" },
	{ 0x10DE0CB0,	0x10DE080D,	"nVidia GeForce GTS 350M" },
	{ 0x10DE0CB0,	0x1179FD30,	"Toshiba GeForce GTS 350M" },
	{ 0x10DE0CB0,	0x1179FF50,	"Toshiba GeForce GTS 350M" },
	{ 0x10DE0CB0,	0x15580511,	"Clevo GeForce GTS 350M" },
	{ 0x10DE0CB0,	0x15580512,	"Clevo GeForce GTS 350M" },
	{ 0x10DE0CB0,	0x15588687,	"Clevo GeForce GTS 350M" },
	{ 0x10DE0CB0,	0x15588689,	"Clevo GeForce GTS 350M" },
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
	{ 0x10DE0DC0,	0x10DE082D,	"nVidia GeForce GT 440" },
	{ 0x10DE0DC0,	0x14622310,	"MSi GeForce GT 440" },
	{ 0x10DE0DC0,	0x14622311,	"MSi GeForce GT 440" },
	{ 0x10DE0DC0,	0x14622312,	"MSi GeForce GT 440" },
	{ 0x10DE0DC0,	0x16423A28,	"Bitland GeForce GT 440" },
	{ 0x10DE0DC0,	0x174B1178,	"PC Partner GeForce GT 440" },
	{ 0x10DE0DC0,	0x174B2178,	"PC Partner GeForce GT 440" },

	{ 0x10DE0DC4,	0x10438365,	"Asus GeForce GTS 450" },
	{ 0x10DE0DC4,	0x10438375,	"Asus GeForce GTS 450" },
	{ 0x10DE0DC4,	0x1043837A,	"Asus GeForce GTS 450" },
	{ 0x10DE0DC4,	0x10B00401,	"Gainward GeForce GTS 450" },
	{ 0x10DE0DC4,	0x10B00801,	"Gainward GeForce GTS 450" },
	{ 0x10DE0DC4,	0x10DE085A,	"nVidia GeForce GTS 450" },
	{ 0x10DE0DC4,	0x145834FE,	"Gigabyte GeForce GTS 450" },
	{ 0x10DE0DC4,	0x14583500,	"Gigabyte GeForce GTS 450" },
	{ 0x10DE0DC4,	0x14583507,	"Gigabyte GeForce GTS 450" },
	{ 0x10DE0DC4,	0x14622360,	"MSi GeForce GTS 450" },
	{ 0x10DE0DC4,	0x14622364,	"MSi GeForce GTS 450" },
	{ 0x10DE0DC4,	0x14628096,	"MSi GeForce GTS 450" },
	{ 0x10DE0DC4,	0x196E085A,	"PNY GeForce GTS 450" },
	{ 0x10DE0DC4,	0x19DA1184,	"Zotac GeForce GTS 450" },
	{ 0x10DE0DC4,	0x19DA1194,	"Zotac GeForce GTS 450" },
	{ 0x10DE0DC4,	0x19DA2184,	"Zotac GeForce GTS 450" },
	{ 0x10DE0DC4,	0x19DA3194,	"Zotac GeForce GTS 450" },
	{ 0x10DE0DC4,	0x1ACC4513,	"Point of View GeForce GTS 450" },
	{ 0x10DE0DC4,	0x1ACC4523,	"Point of View GeForce GTS 450" },
	{ 0x10DE0DC4,	0x1ACC45C2,	"Point of View GeForce GTS 450" },
	{ 0x10DE0DC4,	0x38421351,	"EVGA GeForce GTS 450" },
	{ 0x10DE0DC4,	0x38421452,	"EVGA GeForce GTS 450" },

	{ 0x10DE0DCD,	0x10280491,	"Dell GeForce GT 555M" },
	{ 0x10DE0DCD,	0x102804B7,	"Dell GeForce GT 555M" },
	{ 0x10DE0DCD,	0x102804B8,	"Dell GeForce GT 555M" },
	{ 0x10DE0DCD,	0x146210A2,	"MSi GeForce GT 555M" },

	{ 0x10DE0DD1,	0x102802A2,	"Dell GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1028048F,	"Dell GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10280490,	"Dell GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10280491,	"Dell GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x102804BA,	"Dell GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1043203D,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1043203E,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10432040,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10432041,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10432042,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10432043,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10432044,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10432045,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10432046,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10432047,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10432048,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1043204A,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1043204B,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10438465,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10DE10DE,	"nVidia GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1179FC00,	"Toshiba GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1179FC01,	"Toshiba GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1179FC05,	"Toshiba GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1179FCB0,	"Toshiba GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1179FF50,	"Toshiba GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1179FFD6,	"Toshiba GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1179FFD7,	"Toshiba GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1179FFD8,	"Toshiba GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x14621083,	"MSi GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x15585102,	"Clevo/Kapok GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x15587100,	"Clevo/Kapok GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x15587200,	"Clevo/Kapok GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x15588100,	"Clevo/Kapok GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x15588687,	"Clevo/Kapok GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x17AA3620,	"Lenovo GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x17C010EA,	"Wistron GeForce GTX 460M" },

	{ 0x10DE0DD6,	0x10280010,	"Dell GeForce GT 550M" },
	{ 0x10DE0DD6,	0x102804B7,	"Dell GeForce GT 550M" },
	{ 0x10DE0DD6,	0x102804B8,	"Dell GeForce GT 550M" },

	{ 0x10DE0DD8,	0x103C084A,	"HP nVidia Quadro 2000" },
	{ 0x10DE0DD8,	0x10DE084A,	"nVidia Quadro 2000" },
	{ 0x10DE0DD8,	0x10DE0914,	"nVidia Quadro 2000D" },

	{ 0x10DE0DDE,	0x1043203D,	"Asus GF106-ES" },
	{ 0x10DE0DDE,	0x1043203E,	"Asus GF106-ES" },
	{ 0x10DE0DDE,	0x10432040,	"Asus GF106-ES" },
	{ 0x10DE0DDE,	0x10432041,	"Asus GF106-ES" },
	// 0DE0 - 0DEF
	{ 0x10DE0DE0,	0x10DE0828,	"nVidia GeForce GT 440" },

	{ 0x10DE0DE1,	0x1043836D,	"Asus GeForce GT 430" },
	{ 0x10DE0DE1,	0x38421430,	"EVGA GeForce GT 430" },

	{ 0x10DE0DE2,	0x1043835F,	"Asus GeForce GT 420" },
	{ 0x10DE0DE2,	0x14622302,	"MSi GeForce GT 420" },
	{ 0x10DE0DE2,	0x16423A26,	"Bitland GeForce GT 420" },
	{ 0x10DE0DE2,	0x174B1162,	"PC Partner GeForce GT 420" },
	{ 0x10DE0DE2,	0x174B2162,	"PC Partner GeForce GT 420" },
	{ 0x10DE0DE2,	0x1B0A9083,	"Pegatron GeForce GT 420" },
	{ 0x10DE0DE2,	0x1B0A9085,	"Pegatron GeForce GT 420" },
	{ 0x10DE0DE2,	0x1B0A9089,	"Pegatron GeForce GT 420" },

	{ 0x10DE0DE3,	0x1043100D,	"Asus GeForce GT 635M" },
	{ 0x10DE0DE3,	0x10431477,	"Asus GeForce GT 635M" },
	{ 0x10DE0DE3,	0x10431587,	"Asus GeForce GT 635M" },

	{ 0x10DE0DE9,	0x10250487,	"Acer GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10250488,	"Acer GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10250505,	"Acer GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10250507,	"Acer GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10250512,	"Acer GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10250573,	"Acer GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10250574,	"Acer GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10250575,	"Acer GeForce GT 630M" },
	{ 0x10DE0DE9,	0x1028055E,	"Dell GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10280563,	"Dell GeForce GT 630M" },
	{ 0x10DE0DE9,	0x103C181A,	"HP GeForce GT 630M" },
	{ 0x10DE0DE9,	0x103C181B,	"HP GeForce GT 630M" },
	{ 0x10DE0DE9,	0x103C181D,	"HP GeForce GT 630M" },
	{ 0x10DE0DE9,	0x103C1837,	"HP GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10431477,	"Asus GeForce GT 630M" },
	{ 0x10DE0DE9,	0x104310AC,	"Asus GeForce GT 630M" },
	{ 0x10DE0DE9,	0x104310BC,	"Asus GeForce GT 630M" },
	{ 0x10DE0DE9,	0x104310CC,	"Asus GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10431407,	"Asus GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10431447,	"Asus GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10431497,	"Asus GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10432104,	"Asus GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10432106,	"Asus GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10432110,	"Asus GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10432113,	"Asus GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10432114,	"Asus GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10432128,	"Asus GeForce GT 630M" },
	{ 0x10DE0DE9,	0x1043212E,	"Asus GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10432131,	"Asus GeForce GT 630M" },

	{ 0x10DE0DEF,	0x104310AC,	"Asus N13P-NS1-A1" },
	{ 0x10DE0DEF,	0x104310BC,	"Asus N13P-NS1-A1" },
	{ 0x10DE0DEF,	0x104310CC,	"Asus N13P-NS1-A1" },
	{ 0x10DE0DEF,	0x10431407,	"Asus N13P-NS1-A1" },
	{ 0x10DE0DEF,	0x10431447,	"Asus N13P-NS1-A1" },
	{ 0x10DE0DEF,	0x17AA21F3,	"Lenovo NVS 5400M" },
	{ 0x10DE0DEF,	0x17AA21F4,	"Lenovo NVS 5400M" },
	{ 0x10DE0DEF,	0x17AA21F5,	"Lenovo NVS 5400M" },
	{ 0x10DE0DEF,	0x17AA21F6,	"Lenovo NVS 5400M" },
	{ 0x10DE0DEF,	0x17AA5005,	"Lenovo NVS 5400M" },
	// 0DF0 - 0DFF
	{ 0x10DE0DF0,	0x1B0A9077,	"Pegatron GeForce GT 425M" },
	{ 0x10DE0DF0,	0x1B0A909A,	"Pegatron GeForce GT 425M" },

	{ 0x10DE0DF1,	0x1025035A,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x1025036C,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x1025036D,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x10250370,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x10250371,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x10250374,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x10250375,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x10250379,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x1025037C,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x1025037D,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x1025037E,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x10250382,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x1025040A,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x10250413,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x10250415,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x10250417,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x1025041E,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x10250423,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x10250424,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x10250434,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x10250450,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x10250464,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x10250485,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x10250486,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x10250487,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x10250488,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x10250499,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x1025049A,	"Acer GeForce GT 420M" },
	{ 0x10DE0DF1,	0x10280468,	"Dell GeForce GT 420M" },
	{ 0x10DE0DF1,	0x1028046E,	"Dell GeForce GT 420M" },
	{ 0x10DE0DF1,	0x144DC08E,	"Samsung GeForce GT 420M" },
	{ 0x10DE0DF1,	0x144DC093,	"Samsung GeForce GT 420M" },
	{ 0x10DE0DF1,	0x144DC096,	"Samsung GeForce GT 420M" },
	{ 0x10DE0DF1,	0x1B0A2036,	"Pegatron GeForce GT 420M" },
	{ 0x10DE0DF1,	0x1B0A207A,	"Pegatron GeForce GT 420M" },
	{ 0x10DE0DF1,	0x1BFD2003,	"GeForce GT 420M" }, // SUBVENDOR?

	{ 0x10DE0DF2,	0x174B5162,	"PC Partner GeForce GT 435M" },

	{ 0x10DE0DF3,	0x144DC08D,	"Samsung GeForce GT 420M" },
	{ 0x10DE0DF3,	0x144DC095,	"Samsung GeForce GT 420M" },

	{ 0x10DE0DF4,	0x1043105C,	"Asus GeForce GT 540M" },
	{ 0x10DE0DF4,	0x104315E2,	"Asus GeForce GT 540M" },
	{ 0x10DE0DF4,	0x104315F2,	"Asus GeForce GT 540M" },
	{ 0x10DE0DF4,	0x10431642,	"Asus GeForce GT 540M" },
	{ 0x10DE0DF4,	0x10431662,	"Asus GeForce GT 540M" },
	{ 0x10DE0DF4,	0x10431672,	"Asus GeForce GT 540M" },
	{ 0x10DE0DF4,	0x1043849E,	"Asus GeForce GT 540M" },
	{ 0x10DE0DF4,	0x104384EE,	"Asus GeForce GT 540M" },
	{ 0x10DE0DF4,	0x18490DF4,	"ASRock GeForce GT 540M" },

	{ 0x10DE0DF5,	0x102804CA,	"Dell GeForce GT 525M" },
	{ 0x10DE0DF5,	0x10280511,	"Dell GeForce GT 525M" },
	{ 0x10DE0DF5,	0x10280521,	"Dell GeForce GT 525M" },

	{ 0x10DE0DF6,	0x10431712,	"Asus GeForce GT 550M" },
	{ 0x10DE0DF6,	0x10432049,	"Asus GeForce GT 550M" },
	{ 0x10DE0DF6,	0x1043204D,	"Asus GeForce GT 550M" },
	{ 0x10DE0DF6,	0x14582525,	"GigaByte GeForce GT 550M" },
	{ 0x10DE0DF6,	0x14582532,	"GigaByte GeForce GT 550M" },
	{ 0x10DE0DF6,	0x14C00059,	"Compal GeForce GT 550M" },
	{ 0x10DE0DF6,	0x17AA3981,	"Lenovo GeForce GT 550M" },
	{ 0x10DE0DF6,	0x1B0A20A5,	"Pegatron GeForce GT 550M" },
	{ 0x10DE0DF6,	0x1BAB2002,	"GeForce GT 550M" },

	{ 0x10DE0DFE,	0x10431407,	"Asus GF108 ES" },
	{ 0x10DE0DFE,	0x10431447,	"Asus GF108 ES" },
	{ 0x10DE0DFE,	0x10431482,	"Asus GF108 ES" },
	{ 0x10DE0DFE,	0x10431502,	"Asus GF108 ES" },
	{ 0x10DE0DFE,	0x10431512,	"Asus GF108 ES" },
	{ 0x10DE0DFE,	0x10431522,	"Asus GF108 ES" },
	{ 0x10DE0DFE,	0x10431532,	"Asus GF108 ES" },
	{ 0x10DE0DFE,	0x10431552,	"Asus GF108 ES" },
	{ 0x10DE0DFE,	0x1B0A206C,	"Pegatron GF108 ES" },
	// 0E00 - 0E0F
	// 0E10 - 0E1F
	// 0E20 - 0E2F
	{ 0x10DE0E22,	0x1043835D,	"Asus ENGTX460" },
	{ 0x10DE0E22,	0x10B00401,	"Gainward GeForce GTX 460" },
	{ 0x10DE0E22,	0x10B00801,	"Gainward GeForce GTX 460" },
	{ 0x10DE0E22,	0x10DE0804,	"nVidia GeForce GTX 460" },
	{ 0x10DE0E22,	0x10DE0865,	"nVidia GeForce GTX 460" },
	{ 0x10DE0E22,	0x145834FA,	"GigaByte GeForce GTX 460" },
	{ 0x10DE0E22,	0x145834FC,	"GigaByte GeForce GTX 460" },
	{ 0x10DE0E22,	0x14583501,	"GigaByte GeForce GTX 460" },
	{ 0x10DE0E22,	0x14622321,	"MSi GeForce GTX 460" },
	{ 0x10DE0E22,	0x14622322,	"MSi GeForce GTX 460" },
	{ 0x10DE0E22,	0x14622381,	"MSi GeForce GTX 460" },
	{ 0x10DE0E22,	0x19DA1166,	"Zotac GeForce GTX 460" },
	{ 0x10DE0E22,	0x19DA2166,	"Zotac GeForce GTX 460" },
	{ 0x10DE0E22,	0x38421362,	"EVGA GeForce GTX 460" },
	{ 0x10DE0E22,	0x38421370,	"EVGA GeForce GTX 460" },
	{ 0x10DE0E22,	0x38421372,	"EVGA GeForce GTX 460" },
	{ 0x10DE0E22,	0x38421373,	"EVGA GeForce GTX 460" },
	{ 0x10DE0E22,	0x38421380,	"EVGA GeForce GTX 460" },

	{ 0x10DE0E23,	0x10B00401,	"Gainward GeForce GTX 460" },
	// 0E30 - 0E3F
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
	{ 0x10DE0FC2,	0x103C0936,	"HP GeForce GT 630" },
	{ 0x10DE0FC2,	0x174B0630,	"PC Panther GeForce GT 630" },

	{ 0x10DE0FC6,	0x10B00FC6,	"Gainward GeForce GTX 650" },
	{ 0x10DE0FC6,	0x10DE0973,	"nVidia GeForce GTX 650" },
	{ 0x10DE0FC6,	0x14583553,	"Gigabyte GeForce GTX 650" },
	{ 0x10DE0FC6,	0x14583555,	"Gigabyte GeForce GTX 650" },
	{ 0x10DE0FC6,	0x15690FC6,	"Palit GeForce GTX 650" },
	{ 0x10DE0FC6,	0x19DA1288,	"Zotac GeForce GTX 650" },

	// 0FD0 - 0FDF
	{ 0x10DE0FD1,	0x10280552,	"Dell GeForce GT 650M" },
	{ 0x10DE0FD1,	0x10280566,	"Dell GeForce GT 650M" },
	{ 0x10DE0FD1,	0x10280578,	"Dell GeForce GT 650M" },
	{ 0x10DE0FD1,	0x146210C7,	"MSi GeForce GT 650M" },
	{ 0x10DE0FD1,	0x146210CD,	"MSi GeForce GT 650M" },

	{ 0x10DE0FD2,	0x1028055F,	"Dell GeForce GT 640M" },
	{ 0x10DE0FD2,	0x144DC0D5,	"Samsung GeForce GT 640M" },

	{ 0x10DE0FD3,	0x10250713,	"Acer GeForce GT 640M LE" },
	{ 0x10DE0FD3,	0x10250717,	"Acer GeForce GT 640M LE" },
	{ 0x10DE0FD3,	0x104D909A,	"Sony GeForce GT 640M LE" },
	{ 0x10DE0FD3,	0x104D909C,	"Sony GeForce GT 640M LE" },

	{ 0x10DE0FD4,	0x10280551,	"Dell GeForce GTX 660M" },
	{ 0x10DE0FD4,	0x1028057B,	"Dell GeForce GTX 660M" },
	{ 0x10DE0FD4,	0x146210D7,	"MSi GeForce GTX 660M" },
  
  { 0x10DE0FD5,	0x106b0010,	"Apple GeForce GTX 650M" },

	{ 0x10DE0FDB,	0x104310AC,	"Asus GK107-ESP-A1" },
	{ 0x10DE0FDB,	0x10431447,	"Asus GK107-ESP-A1" },
	{ 0x10DE0FDB,	0x10432103,	"Asus GK107-ESP-A1" },
	{ 0x10DE0FDB,	0x10432105,	"Asus GK107-ESP-A1" },
	{ 0x10DE0FDB,	0x10432115,	"Asus GK107-ESP-A1" },
	{ 0x10DE0FDB,	0x10432116,	"Asus GK107-ESP-A1" },
	{ 0x10DE0FDB,	0x10432117,	"Asus GK107-ESP-A1" },
	{ 0x10DE0FDB,	0x10432118,	"Asus GK107-ESP-A1" },
	{ 0x10DE0FDB,	0x1043212D,	"Asus GK107-ESP-A1" },
	{ 0x10DE0FDB,	0x1179FB12,	"Toshiba GK107-ESP-A1" },
	{ 0x10DE0FDB,	0x1179FB18,	"Toshiba GK107-ESP-A1" },
	{ 0x10DE0FDB,	0x1179FB1A,	"Toshiba GK107-ESP-A1" },
	// 0FE0 - 0FEF
	// 0FF0 - 0FFF
	// 1000 - 100F
	// 1010 - 101F
	// 1020 - 102F
	// 1030 - 103F
	// 1040 - 104F
	{ 0x10DE1040,	0x104383A0,	"Asus GeForce GT 520" },
	{ 0x10DE1040,	0x104383BD,	"Asus GeForce GT 520" },
	{ 0x10DE1040,	0x104383C1,	"Asus GeForce GT 520" },
	{ 0x10DE1040,	0x14622592,	"MSi GeForce GT 520" },
	{ 0x10DE1040,	0x14622593,	"MSi GeForce GT 520" },
	{ 0x10DE1040,	0x16423A98,	"Bitland GeForce GT 520" },
	{ 0x10DE1040,	0x16423B42,	"Bitland GeForce GT 520" },
	{ 0x10DE1040,	0x174B3214,	"PC Partner GeForce GT 520" },
	{ 0x10DE1040,	0x196E0915,	"PNY GeForce GT 520" },
	{ 0x10DE1040,	0x19DA1215,	"Zotac GeForce GT 520" },
	{ 0x10DE1040,	0x19DA1222,	"Zotac GeForce GT 520" },
	{ 0x10DE1040,	0x1ACC5213,	"Point of View GeForce GT 520" },
	{ 0x10DE1040,	0x1ACC5214,	"Point of View GeForce GT 520" },
	{ 0x10DE1040,	0x1ACC522C,	"Point of View GeForce GT 520" },
	{ 0x10DE1040,	0x1B0A90AA,	"Pegatron GeForce GT 520" },

	{ 0x10DE1042,	0x14622595,	"MSi GeForce 510" },
	{ 0x10DE1042,	0x14622596,	"MSi GeForce 510" },

	{ 0x10DE1050,	0x10250487,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250488,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250501,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250503,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250505,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250507,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250509,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250512,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x1025053A,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x1025054E,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250550,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x1025055A,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x1025055C,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250568,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x1025056A,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x1025056B,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x1025056C,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250570,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250572,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250573,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250574,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250575,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250576,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250578,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250579,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x1025057A,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x1025057B,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250580,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250581,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x1025058B,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x1025058C,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250593,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x1025060D,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x1025060F,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10250611,	"Acer GeForce GT 520M" },
	{ 0x10DE1050,	0x10280522,	"Dell GeForce GT 520M" },
	{ 0x10DE1050,	0x103C184D,	"HP GeForce GT 520M" },
	{ 0x10DE1050,	0x103C338A,	"HP GeForce GT 520M" },
	{ 0x10DE1050,	0x103C338B,	"HP GeForce GT 520M" },
	{ 0x10DE1050,	0x103C338C,	"HP GeForce GT 520M" },
	{ 0x10DE1050,	0x10431622,	"Asus GeForce GT 520M" },
	{ 0x10DE1050,	0x10431652,	"Asus GeForce GT 520M" },
	{ 0x10DE1050,	0x10431662,	"Asus GeForce GT 520M" },
	{ 0x10DE1050,	0x10431682,	"Asus GeForce GT 520M" },
	{ 0x10DE1050,	0x104316F2,	"Asus GeForce GT 520M" },
	{ 0x10DE1050,	0x10431722,	"Asus GeForce GT 520M" },
	{ 0x10DE1050,	0x10431732,	"Asus GeForce GT 520M" },
	{ 0x10DE1050,	0x10431742,	"Asus GeForce GT 520M" },
	{ 0x10DE1050,	0x104384CF,	"Asus GeForce GT 520M" },
	{ 0x10DE1050,	0x104D9089,	"Sony GeForce GT 520M" },
	{ 0x10DE1050,	0x104D908A,	"Sony GeForce GT 520M" },
	{ 0x10DE1050,	0x104D908B,	"Sony GeForce GT 520M" },
	{ 0x10DE1050,	0x10CF1635,	"Fujitsu GeForce GT 520M" },
	{ 0x10DE1050,	0x10CF3655,	"Fujitsu GeForce GT 520M" },
	{ 0x10DE1050,	0x1179FC01,	"Toshiba GeForce GT 520M" },
	{ 0x10DE1050,	0x1179FC31,	"Toshiba GeForce GT 520M" },
	{ 0x10DE1050,	0x1179FC50,	"Toshiba GeForce GT 520M" },
	{ 0x10DE1050,	0x1179FC61,	"Toshiba GeForce GT 520M" },
	{ 0x10DE1050,	0x1179FC71,	"Toshiba GeForce GT 520M" },
	{ 0x10DE1050,	0x1179FC81,	"Toshiba GeForce GT 520M" },
	{ 0x10DE1050,	0x1179FC90,	"Toshiba GeForce GT 520M" },
	{ 0x10DE1050,	0x1179FCC0,	"Toshiba GeForce GT 520M" },
	{ 0x10DE1050,	0x1179FCD0,	"Toshiba GeForce GT 520M" },
	{ 0x10DE1050,	0x1179FCE2,	"Toshiba GeForce GT 520M" },
	{ 0x10DE1050,	0x1179FCF2,	"Toshiba GeForce GT 520M" },
	{ 0x10DE1050,	0x1179FD16,	"Toshiba GeForce GT 520M" },
	{ 0x10DE1050,	0x1179FD40,	"Toshiba GeForce GT 520M" },
	{ 0x10DE1050,	0x1179FD50,	"Toshiba GeForce GT 520M" },
	{ 0x10DE1050,	0x1179FD52,	"Toshiba GeForce GT 520M" },
	{ 0x10DE1050,	0x1179FD61,	"Toshiba GeForce GT 520M" },
	{ 0x10DE1050,	0x1179FD71,	"Toshiba GeForce GT 520M" },
	{ 0x10DE1050,	0x1179FDD0,	"Toshiba GeForce GT 520M" },
	{ 0x10DE1050,	0x1179FDD2,	"Toshiba GeForce GT 520M" },
	{ 0x10DE1050,	0x144DC0A0,	"Samsung GeForce GT 520M" },
	{ 0x10DE1050,	0x144DC0B2,	"Samsung GeForce GT 520M" },
	{ 0x10DE1050,	0x144DC0B6,	"Samsung GeForce GT 520M" },
	{ 0x10DE1050,	0x144DC597,	"Samsung GeForce GT 520M" },
	{ 0x10DE1050,	0x14581132,	"Gigabyte GeForce GT 520M" },
	{ 0x10DE1050,	0x1462108C,	"MSi GeForce GT 520M" },
	{ 0x10DE1050,	0x14621094,	"MSi GeForce GT 520M" },
	{ 0x10DE1050,	0x17AA3652,	"Lenovo GeForce GT 520M" },
	{ 0x10DE1050,	0x17AA397D,	"Lenovo GeForce GT 520M" },
	{ 0x10DE1050,	0x17AA397F,	"Lenovo GeForce GT 520M" },
	{ 0x10DE1050,	0x17C010E5,	"Wistron GeForce GT 520M" },
	{ 0x10DE1050,	0x17C010EC,	"Wistron GeForce GT 520M" },
	{ 0x10DE1050,	0x17C010F3,	"Wistron GeForce GT 520M" },
	{ 0x10DE1050,	0x18540865,	"LG GeForce GT 520M" },
	{ 0x10DE1050,	0x18540871,	"LG GeForce GT 520M" },
	{ 0x10DE1050,	0x18541791,	"LG GeForce GT 520M" },
	{ 0x10DE1050,	0x18543001,	"LG GeForce GT 520M" },
	{ 0x10DE1050,	0x19915584,	"GeForce GT 520M" },
	{ 0x10DE1050,	0x1BAB2002,	"GeForce GT 520M" },
	{ 0x10DE1050,	0x1BFD8005,	"GeForce GT 520M" },

	{ 0x10DE1054,	0x10280511,	"Dell GeForce 410M" },
	{ 0x10DE1054,	0x10CF1656,	"Fujitsu GeForce 410M" },
	{ 0x10DE1054,	0x10CF1657,	"Fujitsu GeForce 410M" },
	{ 0x10DE1054,	0x1179FCC0,	"Toshiba GeForce 410M" },
	{ 0x10DE1054,	0x14581100,	"GigaByte GeForce 410M" },
	{ 0x10DE1054,	0x14581125,	"GigaByte GeForce 410M" },

	{ 0x10DE1055,	0x104D908A,	"Sony GeForce 410M" },
	{ 0x10DE1055,	0x104D908B,	"Sony GeForce 410M" },

	{ 0x10DE1058,	0x10432AED,	"Asus GeForce GT 610M" },
	{ 0x10DE1058,	0x10432AF1,	"Asus GeForce GT 610M" },
	{ 0x10DE1058,	0x104310AC,	"Asus GeForce GT 610M" },
	{ 0x10DE1058,	0x104310BC,	"Asus GeForce GT 610M" },
	{ 0x10DE1058,	0x1043112D,	"Asus GeForce GT 610M" },
	{ 0x10DE1058,	0x10431457,	"Asus GeForce GT 610M" },
	{ 0x10DE1058,	0x10431652,	"Asus GeForce GT 610M" },
	{ 0x10DE1058,	0x10432130,	"Asus GeForce GT 610M" },
	{ 0x10DE1058,	0x10432133,	"Asus GeForce GT 610M" },
	{ 0x10DE1058,	0x10438536,	"Asus GeForce GT 610M" },
	{ 0x10DE1058,	0x144DC652,	"Samsung GeForce GT 610M" },
	{ 0x10DE1058,	0x17AA3901,	"Lenovo GeForce GT 610M" },
	{ 0x10DE1058,	0x17AA3902,	"Lenovo GeForce GT 610M" },
	{ 0x10DE1058,	0x17AA3977,	"Lenovo GeForce GT 610M" },
	{ 0x10DE1058,	0x17AA397D,	"Lenovo GeForce GT 610M" },
	{ 0x10DE1058,	0x17AA3983,	"Lenovo GeForce GT 610M" },
	{ 0x10DE1058,	0x17AA5001,	"Lenovo GeForce GT 610M" },
	{ 0x10DE1058,	0x17AA5003,	"Lenovo GeForce GT 610M" },
	{ 0x10DE1058,	0x17AA5005,	"Lenovo GeForce GT 610M" },
	{ 0x10DE1058,	0x17AA5007,	"Lenovo GeForce GT 610M" },
	{ 0x10DE1058,	0x17AA500F,	"Lenovo GeForce GT 610M" },
	{ 0x10DE1058,	0x17AA5012,	"Lenovo GeForce GT 610M" },

	{ 0x10DE105A,	0x10250505,	"Acer GeForce 610M" },
	{ 0x10DE105A,	0x10250507,	"Acer GeForce 610M" },
	{ 0x10DE105A,	0x10280579,	"Dell GeForce 610M" },
	{ 0x10DE105A,	0x103C1898,	"HP GeForce 610M" },
	{ 0x10DE105A,	0x10432129,	"Asus GeForce 610M" },

	// 1060 - 106F
	// 1070 - 107F
	{ 0x10DE107D,	0x103C094E,	"HP NVS 310" },
	// 1080 - 108F
	{ 0x10DE1080,	0x10438379,	"Asus GeForce GTX 580" },
	{ 0x10DE1080,	0x10438385,	"Asus GeForce GTX 580" },
	{ 0x10DE1080,	0x104383BB,	"Asus GeForce GTX 580" },
	{ 0x10DE1080,	0x10B00401,	"Gainward GeForce GTX 580" },
	{ 0x10DE1080,	0x1458350F,	"Gigabyte GeForce GTX 580" },
	{ 0x10DE1080,	0x1458351B,	"Gigabyte GeForce GTX 580" },
	{ 0x10DE1080,	0x1458351E,	"Gigabyte GeForce GTX 580" },
	{ 0x10DE1080,	0x14622550,	"MSi GeForce GTX 580" },
	{ 0x10DE1080,	0x14622561,	"MSI N580GTX Lightning" },
	{ 0x10DE1080,	0x14622563,	"MSI N580GTX Lightning" },
	{ 0x10DE1080,	0x196E086A,	"PNY GeForce GTX 580" },
	{ 0x10DE1080,	0x19DA2203,	"Zotac GeForce GTX 580" },
	{ 0x10DE1080,	0x38421582,	"EVGA GeForce GTX 580" },
	{ 0x10DE1080,	0x38421584,	"EVGA GeForce GTX 580" },

	{ 0x10DE1081,	0x10438383,	"Asus GeForce GTX 570" },
	{ 0x10DE1081,	0x10DE087E,	"nVidia GeForce GTX 570" },
	{ 0x10DE1081,	0x14583513,	"GigaByte GeForce GTX 570" },
	{ 0x10DE1081,	0x14622551,	"MSi GeForce GTX 570" },
	{ 0x10DE1081,	0x38421570,	"EVGA GeForce GTX 570" },
	{ 0x10DE1081,	0x38421572,	"EVGA GeForce GTX 570" },

	{ 0x10DE1082,	0x10DE0873,	"nVidia GeForce GTX 560 Ti" },
	{ 0x10DE1082,	0x174B5207,	"PC Partner GeForce GTX 560 Ti" },

	{ 0x10DE1084,	0x14622570,	"MSi GeForce GTX 560" },
	{ 0x10DE1084,	0x14622571,	"MSi GeForce GTX 560" },
	{ 0x10DE1084,	0x16423A96,	"Bitland GeForce GTX 560" },

	{ 0x10DE1086,	0x10438387,	"Asus GeForce GTX 570" },
	{ 0x10DE1086,	0x10DE0871,	"Inno3D GeForce GTX 570" },
	{ 0x10DE1086,	0x14583513,	"GigaByte GeForce GTX 570" },
	{ 0x10DE1086,	0x14622224,	"MSi GeForce GTX 570" },
	{ 0x10DE1086,	0x174B1207,	"PC Partner GeForce GTX 570" },
	{ 0x10DE1086,	0x196E0871,	"PNY GeForce GTX 570" },
	{ 0x10DE1086,	0x19DA1207,	"Zotac GeForce GTX 570" },
	{ 0x10DE1086,	0x38421571,	"EVGA GeForce GTX 570" },
	{ 0x10DE1086,	0x38421573,	"EVGA GeForce GTX 570" },

	{ 0x10DE1087,	0x104383D6,	"Asus ENGTX560Ti448 DCII" },
	{ 0x10DE1087,	0x1458353A,	"Gigabyte GeForce GTX 560 Ti-448" },
	{ 0x10DE1087,	0x19DA2207,	"Zotac GeForce GTX 560 Ti-448" },
	{ 0x10DE1087,	0x38422066,	"EVGA GeForce GTX 560 Ti-448" },

	{ 0x10DE1088,	0x104383A3,	"Asus GeForce GTX 590" },
	{ 0x10DE1088,	0x10DE0868,	"nVidia GeForce GTX 590" },
	{ 0x10DE1088,	0x38421598,	"EVGA GeForce GTX 590" },

	{ 0x10DE108B,	0x10438391,	"Asus GeForce GTX 590" },
	// 1090 - 109F
	{ 0x10DE1091,	0x10DE0887,	"nVidia Tesla M2090" },
	{ 0x10DE1091,	0x10DE088E,	"nVidia Tesla X2090" },
	{ 0x10DE1091,	0x10DE0891,	"nVidia Tesla X2090" },

	{ 0x10DE1094,	0x10DE0888,	"nVidia Tesla M2075" },

	{ 0x10DE109B,	0x10DE0918,	"nVidia Quadro 7000" },
	// 10A0 - 10AF
	// 10B0 - 10BF
	// 10C0 - 10CF
	{ 0x10DE10C4,	0x17AA3605,	"Lenovo ION" },

	{ 0x10DE10C5,	0x1043838D,	"Asus GeForce 405" },
	{ 0x10DE10C5,	0x1043839C,	"Asus GeForce 405" },
	{ 0x10DE10C5,	0x14621834,	"MSi GeForce 405" },
	{ 0x10DE10C5,	0x14621835,	"MSi GeForce 405" },
	{ 0x10DE10C5,	0x14621837,	"MSi GeForce 405" },
	{ 0x10DE10C5,	0x1462183B,	"MSi GeForce 405" },
	{ 0x10DE10C5,	0x14622298,	"MSi GeForce 405" },
	{ 0x10DE10C5,	0x16423899,	"Bitland GeForce 405" },
	{ 0x10DE10C5,	0x16423958,	"Bitland GeForce 405" },
	{ 0x10DE10C5,	0x174B3150,	"PC Partner GeForce 405" },
	{ 0x10DE10C5,	0x1B0A908E,	"Pegatron GeForce 405" },
	{ 0x10DE10C5,	0x1B0A90A9,	"Pegatron GeForce 405" },
	{ 0x10DE10C5,	0x1B0A90AB,	"Pegatron GeForce 405" },
	{ 0x10DE10C5,	0x1B0A90AC,	"Pegatron GeForce 405" },
	{ 0x10DE10C5,	0x1B0A90AF,	"Pegatron GeForce 405" },
	// 10D0 - 10DF

	{ 0x10DE10D8,	0x103C0862,	"HP NVS 300" },
	{ 0x10DE10D8,	0x103C0863,	"HP NVS 300" },
	{ 0x10DE10D8,	0x10DE0862,	"nVidia NVS 300" },
	{ 0x10DE10D8,	0x10DE0863,	"nVidia NVS 300" },

	// 10E0 - 10EF
	// 10F0 - 10FF
	// 1100 - 110F
	// 1110 - 111F
	// 1120 - 112F
	// 1130 - 113F
	// 1140 - 114F
	{ 0x10DE1140,	0x1025064A,	"Acer GeForce GT 620M" },
	{ 0x10DE1140,	0x1025064C,	"Acer GeForce GT 620M" },
	{ 0x10DE1140,	0x10250680,	"Acer GeForce GT 620M" },
	{ 0x10DE1140,	0x10250692,	"Acer GeForce GT 620M" },
	{ 0x10DE1140,	0x10250694,	"Acer GeForce GT 620M" },
	{ 0x10DE1140,	0x10250702,	"Acer GeForce GT 620M" },
	{ 0x10DE1140,	0x10250719,	"Acer GeForce GT 620M" },
	{ 0x10DE1140,	0x10250725,	"Acer GeForce GT 620M" },
	{ 0x10DE1140,	0x10250728,	"Acer GeForce GT 620M" },
	{ 0x10DE1140,	0x1025072B,	"Acer GeForce GT 620M" },
	{ 0x10DE1140,	0x1025072E,	"Acer GeForce GT 620M" },
	{ 0x10DE1140,	0x10250732,	"Acer GeForce GT 620M" },
	{ 0x10DE1140,	0x10280565,	"Dell GeForce GT 630M" },
	{ 0x10DE1140,	0x10280568,	"Dell GeForce GT 630M" },
	{ 0x10DE1140,	0x144DC0D5,	"Samsung GeForce GT 630M" },
	{ 0x10DE1140,	0x17AA3901,	"Lenovo GeForce GT 610M" },
	{ 0x10DE1140,	0x17AA3903,	"Lenovo GeForce GT 610M" },
	{ 0x10DE1140,	0x17AA3983,	"Lenovo GeForce GT 610M" },
	{ 0x10DE1140,	0x17AA500D,	"Lenovo GeForce GT 620M" },
	{ 0x10DE1140,	0x1B0A20DD,	"Pegatron GeForce GT 620M" },
	{ 0x10DE1140,	0x1B0A20FD,	"Pegatron GeForce GT 620M" },
	// 1150 - 115F
	// 1160 - 116F
	// 1170 - 117F
	// 1180 - 118F
	{ 0x10DE1180,	0x00001255,	"Afox GTX 680" },
	{ 0x10DE1180,	0x104383F0,	"Asus GTX680-2GD5" },
	{ 0x10DE1180,	0x104383F6,	"Asus GTX 680 Direct CU II" },
	{ 0x10DE1180,	0x104383F7,	"Asus GTX 680 Direct CU II" },
	{ 0x10DE1180,	0x10DE0969,	"nVidia GTX 680" },
	{ 0x10DE1180,	0x10DE097A,	"nVidia GeForce GTX 680" },
	{ 0x10DE1180,	0x10B01180,	"Gainward GTX 680" },
	{ 0x10DE1180,	0x1458353C,	"GV-N680OC-2GD WindForce GTX 680 OC" },
	{ 0x10DE1180,	0x14622820,	"MSi N680GTX TwinFrozer" },
	{ 0x10DE1180,	0x14622830,	"MSi GTX 680 Lightning" },
	{ 0x10DE1180,	0x14622831,	"MSi GTX 680 Lightning LN2" },
	{ 0x10DE1180,	0x15691180,	"Palit GTX 680 JetStream" },
	{ 0x10DE1180,	0x15691181,	"Palit GTX 680 JetStream" },
	{ 0x10DE1180,	0x15691189,	"Palit GTX 680 JetStream" },
	{ 0x10DE1180,	0x174B1255,	"PC Partner GeForce GTX 680" },
	{ 0x10DE1180,	0x196E0969,	"PNY GTX 680" },
	{ 0x10DE1180,	0x19DA1255,	"Zotac GTX 680" },
	{ 0x10DE1180,	0x19DA1260,	"Zotac GTX680" },
	{ 0x10DE1180,	0x1ACC684A,	"Point of View GTX 680" },
	{ 0x10DE1180,	0x38421582,	"EVGA GTX 680" },
	{ 0x10DE1180,	0x38422680,	"EVGA GTX 680" },
	{ 0x10DE1180,	0x38422682,	"EVGA GTX 680 SC" },
	{ 0x10DE1180,	0x38422683,	"EVGA GTX 680 SC" },
	{ 0x10DE1180,	0x38422686,	"EVGA GTX 680" },
	{ 0x10DE1180,	0x38422689,	"EVGA GTX 680" },

	{ 0x10DE1183,	0x10DE1000,	"nVidia GTX 660 Ti" },
	{ 0x10DE1183,	0x14622843,	"MSi GTX 660 Ti" },
	{ 0x10DE1183,	0x19DA1280,	"Zotac GTX 660 Ti" },

	{ 0x10DE1185,	0x10DE098A,	"nVidia GeForce GTX 660" },
	{ 0x10DE1185,	0x174B2260,	"PC Partner GeForce GTX 660" },

	{ 0x10DE1188,	0x10438406,	"Asus GeForce GTX 690" },
	{ 0x10DE1188,	0x10DE095B,	"nVidia GeForce GTX 690" },
	{ 0x10DE1188,	0x38422690,	"EVGA GeForce GTX 690" },

	{ 0x10DE1189,	0x10438405,	"Asus GTX 670 Direct CU II TOP" },
	{ 0x10DE1189,	0x10DE097A,	"nVidia GeForce GTX 670" },
	{ 0x10DE1189,	0x14583542,	"Gigabyte GeForce GTX 670" },
	{ 0x10DE1189,	0x14622840,	"MSi GeForce GTX 670" },
	{ 0x10DE1189,	0x15691189,	"Palit GTX 670 JetStream" },
	{ 0x10DE1189,	0x174B1260,	"PC Partner GeForce GTX 670" },
	{ 0x10DE1189,	0x19DA1255,	"Zotac GTX 670 AMP! Edition" },
	{ 0x10DE1189,	0x38422672,	"EVGA GTX 670" },
	{ 0x10DE1189,	0x38422678,	"EVGA GTX 670" },
  // 118A - 118F
	// 1190 - 119F
	// 11A0 - 11AF
  { 0x10DE11A0,	0x10280550,	"Dell GeForce GTX 680M" },
	{ 0x10DE11A0,	0x10280551,	"Dell GeForce GTX 680M" },
	{ 0x10DE11A0,	0x1028057B,	"Dell GeForce GTX 680M" },
	{ 0x10DE11A0,	0x10280580,	"Dell GeForce GTX 680M" },
	{ 0x10DE11A0,	0x146210BC,	"MSi GeForce GTX 680M" },
	{ 0x10DE11A0,	0x146210BE,	"MSi GeForce GTX 680M" },
	{ 0x10DE11A0,	0x15580270,	"Clevo GeForce GTX 680M" },
	{ 0x10DE11A0,	0x15580271,	"Clevo GeForce GTX 680M" },
	{ 0x10DE11A0,	0x15580371,	"Clevo GeForce GTX 680M" },
	{ 0x10DE11A0,	0x15580372,	"Clevo GeForce GTX 680M" },
	{ 0x10DE11A0,	0x15585105,	"Clevo GeForce GTX 680M" },
	{ 0x10DE11A0,	0x15587102,	"Clevo GeForce GTX 680M" },
	// 11A1 - 11AF
	{ 0x10DE11A1,	0x104310AD,	"Asus GeForce GTX 670MX" },
	{ 0x10DE11A1,	0x104321AB,	"Asus GeForce GTX 670MX" },
	{ 0x10DE11A1,	0x15580270,	"Clevo GeForce GTX 670MX" },
	{ 0x10DE11A1,	0x15580371,	"Clevo GeForce GTX 670MX" },
	{ 0x10DE11A1,	0x15585105,	"Clevo GeForce GTX 670MX" },
	{ 0x10DE11A1,	0x15587102,	"Clevo N13E-GR" },

	{ 0x10DE11A7,	0x15585105,	"Clevo GeForce GTX 675MX" },
	{ 0x10DE11A7,	0x15587102,	"Clevo GeForce GTX 675MX" },
	// 11B0 - 11BB
	{ 0x10DE11BC,	0x1028053F,	"Dell Quadro K5000M" },
	{ 0x10DE11BC,	0x1028153F,	"Dell Quadro K5000M" },
	{ 0x10DE11BC,	0x10CF1762,	"Fujitsu Quadro K5000M" },
	{ 0x10DE11BC,	0x15580270,	"Clevo Quadro K5000M" },
	{ 0x10DE11BC,	0x15580371,	"Clevo Quadro K5000M" },

	{ 0x10DE11BD,	0x10CF1761,	"Fujitsu Quadro K4000M" },

	{ 0x10DE11BE,	0x10CF1760,	"Fujitsu Quadro K3000M" },
	{ 0x10DE11BE,	0x15585105,	"Clevo Quadro K3000M" },
	{ 0x10DE11BE,	0x15587102,	"Clevo Quadro K3000M" },
	// 11C0 - 11CF
  { 0x10DE11C0,	0x10DE0995,	"Inno3D GeForce GTX660" },

	{ 0x10DE11C6,	0x1043842A,	"Asus GeForce GTX 650 Ti" },
	{ 0x10DE11C6,	0x10DE1016,	"nVidia GeForce GTX 650 Ti" },
	{ 0x10DE11C6,	0x156911C6,	"Palit GeForce GTX 650 Ti" },
	// 11D0 - 11DF
	// 11E0 - 11EF
	// 11F0 - 11FF	
	// 1200 - 120F
	{ 0x10DE1200,	0x1043838B,	"Asus GeForce GTX 560 Ti" },
	{ 0x10DE1200,	0x10438390,	"Asus GeForce GTX 560 Ti" },
	{ 0x10DE1200,	0x104383BF,	"Asus GeForce GTX 560 Ti" },
	{ 0x10DE1200,	0x10B00801,	"Gainward GeForce GTX 560 Ti" },
	{ 0x10DE1200,	0x14583515,	"Gigabyte GeForce GTX 560 Ti" },
	{ 0x10DE1200,	0x1458351C,	"Gigabyte GeForce GTX 560 Ti" },
	{ 0x10DE1200,	0x14622601,	"MSi GeForce GTX 560 Ti" },
	{ 0x10DE1200,	0x196E0898,	"PNY GeForce GTX 560 Ti" },
	{ 0x10DE1200,	0x38421567,	"EVGA GeForce GTX 560 Ti" },
	{ 0x10DE1200,	0x38421568,	"EVGA GeForce GTX 560 Ti" },

	{ 0x10DE1201,	0x104383B4,	"Asus GeForce GTX 560" },
	{ 0x10DE1201,	0x10DE0895,	"nVidia GeForce GTX 560" },
	{ 0x10DE1201,	0x14622383,	"MSi GeForce GTX 560" },
  // 1202 - 1205
	{ 0x10DE1206,	0x10DE0958,	"nVidia GeForce GTX 555" },

	{ 0x10DE1207,	0x174B0645,	"PC Partner GeForce GT 645" },

	{ 0x10DE1210,	0x10431487,	"Asus GeForce GTX 570M" },
	{ 0x10DE1210,	0x10432104,	"Asus GeForce GTX 570M" },
	{ 0x10DE1210,	0x1179FB12,	"Toshiba GeForce GTX 570M" },
	{ 0x10DE1210,	0x1179FB18,	"Toshiba GeForce GTX 570M" },
	{ 0x10DE1210,	0x1179FB1A,	"Toshiba GeForce GTX 570M" },
	{ 0x10DE1210,	0x146210BD,	"MSi GeForce GTX 570M" },

	{ 0x10DE1211,	0x1028048F,	"Dell GeForce GTX 580M" },
	{ 0x10DE1211,	0x10280490,	"Dell GeForce GTX 580M" },
	{ 0x10DE1211,	0x102804BA,	"Dell GeForce GTX 580M" },
	{ 0x10DE1211,	0x146210A9,	"MSi GeForce GTX 580M" },
	{ 0x10DE1211,	0x15580270,	"Clevo GeForce GTX 580M" },
	{ 0x10DE1211,	0x15580271,	"Clevo GeForce GTX 580M" },
	{ 0x10DE1211,	0x15585102,	"Clevo GeForce GTX 580M" },
	{ 0x10DE1211,	0x15587100,	"Clevo GeForce GTX 580M" },
	{ 0x10DE1211,	0x15587101,	"Clevo GeForce GTX 580M" },
	{ 0x10DE1211,	0x15587200,	"Clevo GeForce GTX 580M" },

	{ 0x10DE1212,	0x10280550,	"Dell GeForce GTX 675M" },
	{ 0x10DE1212,	0x10280551,	"Dell GeForce GTX 675M" },
	{ 0x10DE1212,	0x1028057B,	"Dell GeForce GTX 675M" },
	{ 0x10DE1212,	0x10280580,	"Dell GeForce GTX 675M" },
	{ 0x10DE1212,	0x10DE095D,	"nVidia GeForce GTX 675M" },
	{ 0x10DE1212,	0x144DC0D0,	"Samsung GeForce GTX 675M" },
	{ 0x10DE1212,	0x146210CB,	"MSi GeForce GTX 675M" },
	{ 0x10DE1212,	0x15580270,	"Clevo GeForce GTX 675M" },
	{ 0x10DE1212,	0x15580271,	"Clevo GeForce GTX 675M" },
	{ 0x10DE1212,	0x15585105,	"Clevo GeForce GTX 675M" },
	{ 0x10DE1212,	0x15587102,	"Clevo GeForce GTX 675M" },

	{ 0x10DE1213,	0x10432119,	"Asus GeForce GTX 670M" },
	{ 0x10DE1213,	0x10432120,	"Asus GeForce GTX 670M" },
	{ 0x10DE1213,	0x102804BA,	"Dell GeForce GTX 670M" },
	{ 0x10DE1213,	0x10432119,	"Dell GeForce GTX 670M" },
	{ 0x10DE1213,	0x10432120,	"Dell GeForce GTX 670M" },
	{ 0x10DE1213,	0x10DE095E,	"nVidia GeForce GTX 670M" },
	{ 0x10DE1213,	0x1179FB12,	"Toshiba GeForce GTX 670M" },
	{ 0x10DE1213,	0x1179FB18,	"Toshiba GeForce GTX 670M" },
	{ 0x10DE1213,	0x1179FB1A,	"Toshiba GeForce GTX 670M" },
	{ 0x10DE1213,	0x146210CB,	"MSi GeForce GTX 670M" },
	{ 0x10DE1213,	0x15580371,	"Clevo GeForce GTX 670M" },
	{ 0x10DE1213,	0x15585105,	"Clevo GeForce GTX 670M" },
	{ 0x10DE1213,	0x15587102,	"Clevo GeForce GTX 670M" },
	{ 0x10DE1213,	0x15588000,	"Clevo GeForce GTX 670M" },

	{ 0x10DE1241,	0x10DE091D,	"nVidia GeForce GT 545" },

	{ 0x10DE1243,	0x10438508,	"Asus GeForce GT 545" },
	{ 0x10DE1243,	0x14622315,	"MSi GeForce GT 545" },
	{ 0x10DE1243,	0x14622316,	"MSi GeForce GT 545" },
	{ 0x10DE1243,	0x16423A28,	"Bitland GeForce GT 545" },
	{ 0x10DE1243,	0x174B5178,	"PC Partner GeForce GT 545" },
	{ 0x10DE1243,	0x174B6178,	"PC Partner GeForce GT 545" },

	{ 0x10DE1244,	0x104383BC,	"Asus GeForce GTX 550 Ti" },
	{ 0x10DE1244,	0x1458351A,	"GigaByte GeForce GTX 550 Ti" },
	{ 0x10DE1244,	0x19DA5194,	"Zotac GeForce GTX 550 Ti" },
	{ 0x10DE1244,	0x1B0A90A2,	"Pegatron GeForce GTX 550 Ti" },
	{ 0x10DE1244,	0x38421556,	"EVGA GeForce GTX 550 Ti" },

	{ 0x10DE1246,	0x10280570,	"Dell GeForce GT 550M" },
	{ 0x10DE1246,	0x10280571,	"Dell GeForce GT 550M" },

	{ 0x10DE1247,	0x10431407,	"Asus GeForce GT 555M" },
	{ 0x10DE1247,	0x10431752,	"Asus GeForce GT 555M" },
	{ 0x10DE1247,	0x10432050,	"Asus GeForce GT 555M" },
	{ 0x10DE1247,	0x10432051,	"Asus GeForce GT 555M" },
	{ 0x10DE1247,	0x10432119,	"Asus GeForce GT 670M" },
	{ 0x10DE1247,	0x10432120,	"Asus GeForce GT 670M" },
	{ 0x10DE1247,	0x1043212A,	"Asus GeForce GT 635M" },
	{ 0x10DE1247,	0x1043212B,	"Asus GeForce GT 635M" },
	{ 0x10DE1247,	0x1043212C,	"Asus GeForce GT 635M" },
	{ 0x10DE1247,	0x14581532,	"GigaByte GeForce GT 555M" },
	{ 0x10DE1247,	0x14586744,	"GigaByte GeForce GT 555M" },
	{ 0x10DE1247,	0x152D0930,	"Quanta GeForce GT 635M" },

	{ 0x10DE1248,	0x152D0930,	"Quanta GeForce GT 635M" },
	{ 0x10DE1248,	0x17C010E7,	"Wistron GeForce GT 555M" },
	{ 0x10DE1248,	0x17C010E8,	"Wistron GeForce GT 555M" },
	{ 0x10DE1248,	0x17C010EA,	"Wistron GeForce GT 555M" },
	{ 0x10DE1248,	0x18540890,	"LG GeForce GT 555M" },
	{ 0x10DE1248,	0x18540891,	"LG GeForce GT 555M" },
	{ 0x10DE1248,	0x18541795,	"LG GeForce GT 555M" },
	{ 0x10DE1248,	0x18541796,	"LG GeForce GT 555M" },

	{ 0x10DE124B,	0x10438540,	"Asus GeForce GT 640" },
	{ 0x10DE124B,	0x14622319,	"MSi GeForce GT 640" },
	{ 0x10DE124B,	0x1462231A,	"MSi GeForce GT 640" },
	{ 0x10DE124B,	0x174B0640,	"PC Partner GeForce GT 640" },

	{ 0x10DE124D,	0x10280491,	"Dell GeForce GT 555M" },
	{ 0x10DE124D,	0x10280570,	"Dell GeForce GT 555M" },
	{ 0x10DE124D,	0x10280571,	"Dell GeForce GT 555M" },
	{ 0x10DE124D,	0x1462108D,	"MSi GeForce GT 555M" },
	{ 0x10DE124D,	0x146210CC,	"MSi GeForce GT 635M" },

	{ 0x10DE1251,	0x102802A2,	"Dell GeForce GTX 560M" },
	{ 0x10DE1251,	0x102802F8,	"Dell GeForce GTX 560M" },
	{ 0x10DE1251,	0x1028048F,	"Dell GeForce GTX 560M" },
	{ 0x10DE1251,	0x10280490,	"Dell GeForce GTX 560M" },
	{ 0x10DE1251,	0x102804BA,	"Dell GeForce GTX 560M" },
	{ 0x10DE1251,	0x104313B7,	"Asus GeForce GTX 560M" },
	{ 0x10DE1251,	0x1043204A,	"Asus GeForce GTX 560M" },
	{ 0x10DE1251,	0x1043204B,	"Asus GeForce GTX 560M" },
	{ 0x10DE1251,	0x10432100,	"Asus GeForce GTX 560M" },
	{ 0x10DE1251,	0x10432101,	"Asus GeForce GTX 560M" },
	{ 0x10DE1251,	0x104384BA,	"Asus GeForce GTX 560M" },
	{ 0x10DE1251,	0x1179FC00,	"Toshiba GeForce GTX 560M" },
	{ 0x10DE1251,	0x1179FC01,	"Toshiba GeForce GTX 560M" },
	{ 0x10DE1251,	0x1179FC05,	"Toshiba GeForce GTX 560M" },
	{ 0x10DE1251,	0x146210A9,	"MSi GeForce GTX 560M" },
	{ 0x10DE1251,	0x15585102,	"Clevo/Kapok GeForce GTX 560M" },
	{ 0x10DE1251,	0x15587100,	"Clevo/Kapok GeForce GTX 560M" },
	{ 0x10DE1251,	0x15587101,	"Clevo/Kapok GeForce GTX 560M" },
	{ 0x10DE1251,	0x15587200,	"Clevo/Kapok GeForce GTX 560M" },
	{ 0x10DE1251,	0x15588000,	"Clevo/Kapok GeForce GTX 560M" },
	// 1260 - 126F
	// 1270 - 127F
	// 1280 - 128F
	// 1290 - 129F
	// 12A0 - 12AF
	// 12B0 - 12BF
	// 12C0 - 12CF
	// 12D0 - 12DF
	// 12E0 - 12EF
	// 12F0 - 12FF
    /* ------ Specific DeviceID and Generic SubDevID. ------ */
	// 0000 - 0040	
	// 0040 - 004F	
	{ 0x10DE0040,	NV_SUB_IDS,	"GeForce 6800 Ultra" },
	{ 0x10DE0041,	NV_SUB_IDS,	"GeForce 6800" },
	{ 0x10DE0042,	NV_SUB_IDS,	"GeForce 6800 LE" },
	{ 0x10DE0043,	NV_SUB_IDS,	"GeForce 6800 XE" },
	{ 0x10DE0044,	NV_SUB_IDS,	"GeForce 6800 XT" },
	{ 0x10DE0045,	NV_SUB_IDS,	"GeForce 6800 GT" },
	{ 0x10DE0046,	NV_SUB_IDS,	"GeForce 6800 GT" },
	{ 0x10DE0047,	NV_SUB_IDS,	"GeForce 6800 GS" },
	{ 0x10DE0048,	NV_SUB_IDS,	"GeForce 6800 XT" },
	{ 0x10DE004D,	NV_SUB_IDS,	"Quadro FX 3400" },
	{ 0x10DE004E,	NV_SUB_IDS,	"Quadro FX 4000" },
	// 0050 - 005F
	// 0060 - 006F
	// 0070 - 007F
	// 0080 - 008F
	// 0090 - 009F
	{ 0x10DE0090,	NV_SUB_IDS,	"GeForce 7800 GTX" },
	{ 0x10DE0091,	NV_SUB_IDS,	"GeForce 7800 GTX" },
	{ 0x10DE0092,	NV_SUB_IDS,	"GeForce 7800 GT" },
	{ 0x10DE0093,	NV_SUB_IDS,	"GeForce 7800 GS" },
	{ 0x10DE0095,	NV_SUB_IDS,	"GeForce 7800 SLI" },
	{ 0x10DE0098,	NV_SUB_IDS,	"GeForce Go 7800" },
	{ 0x10DE0099,	NV_SUB_IDS,	"GeForce Go 7800 GTX" },
	{ 0x10DE009D,	NV_SUB_IDS,	"Quadro FX 4500" },
	// 00A0 - 00AF	
	// 00B0 - 00BF	
	// 00C0 - 00CF	
	{ 0x10DE00C0,	NV_SUB_IDS,	"GeForce 6800 GS" },
	{ 0x10DE00C1,	NV_SUB_IDS,	"GeForce 6800" },
	{ 0x10DE00C2,	NV_SUB_IDS,	"GeForce 6800 LE" },
	{ 0x10DE00C3,	NV_SUB_IDS,	"GeForce 6800 XT" },
	{ 0x10DE00C8,	NV_SUB_IDS,	"GeForce Go 6800" },
	{ 0x10DE00C9,	NV_SUB_IDS,	"GeForce Go 6800 Ultra" },
	{ 0x10DE00CC,	NV_SUB_IDS,	"Quadro FX Go1400" },
	{ 0x10DE00CD,	NV_SUB_IDS,	"Quadro FX 3450/4000 SDI" },
	{ 0x10DE00CE,	NV_SUB_IDS,	"Quadro FX 1400" },
	// 00D0 - 00DF	
	// 00E0 - 00EF	
	// 00F0 - 00FF	
	{ 0x10DE00F1,	NV_SUB_IDS,	"GeForce 6600 GT" },
	{ 0x10DE00F2,	NV_SUB_IDS,	"GeForce 6600" },
	{ 0x10DE00F3,	NV_SUB_IDS,	"GeForce 6200" },
	{ 0x10DE00F4,	NV_SUB_IDS,	"GeForce 6600 LE" },
	{ 0x10DE00F5,	NV_SUB_IDS,	"GeForce 7800 GS" },
	{ 0x10DE00F6,	NV_SUB_IDS,	"GeForce 6800 GS/XT" },
	{ 0x10DE00F8,	NV_SUB_IDS,	"Quadro FX 3400/4400" },
	{ 0x10DE00F9,	NV_SUB_IDS,	"GeForce 6800 Series GPU" },
	// 0100 - 010F	
	// 0110 - 011F	
	// 0120 - 012F	
	// 0130 - 013F	
	// 0140 - 014F	
	{ 0x10DE0140,	NV_SUB_IDS,	"GeForce 6600 GT" },
	{ 0x10DE0141,	NV_SUB_IDS,	"GeForce 6600" },
	{ 0x10DE0142,	NV_SUB_IDS,	"GeForce 6600 LE" },
	{ 0x10DE0143,	NV_SUB_IDS,	"GeForce 6600 VE" },
	{ 0x10DE0144,	NV_SUB_IDS,	"GeForce Go 6600" },
	{ 0x10DE0145,	NV_SUB_IDS,	"GeForce 6610 XL" },
	{ 0x10DE0146,	NV_SUB_IDS,	"GeForce Go 6600 TE/6200 TE" },
	{ 0x10DE0147,	NV_SUB_IDS,	"GeForce 6700 XL" },
	{ 0x10DE0148,	NV_SUB_IDS,	"GeForce Go 6600" },
	{ 0x10DE0149,	NV_SUB_IDS,	"GeForce Go 6600 GT" },
	{ 0x10DE014A,	NV_SUB_IDS,	"Quadro NVS 440" },
	{ 0x10DE014C,	NV_SUB_IDS,	"Quadro FX 550" },
	{ 0x10DE014D,	NV_SUB_IDS,	"Quadro FX 550" },
	{ 0x10DE014E,	NV_SUB_IDS,	"Quadro FX 540" },
	{ 0x10DE014F,	NV_SUB_IDS,	"GeForce 6200" },
	// 0150 - 015F	
	// 0160 - 016F	
	{ 0x10DE0160,	NV_SUB_IDS,	"GeForce 6500" },
	{ 0x10DE0161,	NV_SUB_IDS,	"GeForce 6200 TurboCache(TM)" },
	{ 0x10DE0162,	NV_SUB_IDS,	"GeForce 6200SE TurboCache(TM)" },
	{ 0x10DE0163,	NV_SUB_IDS,	"GeForce 6200 LE" },
	{ 0x10DE0164,	NV_SUB_IDS,	"GeForce Go 6200" },
	{ 0x10DE0165,	NV_SUB_IDS,	"Quadro NVS 285" },
	{ 0x10DE0166,	NV_SUB_IDS,	"GeForce Go 6400" },
	{ 0x10DE0167,	NV_SUB_IDS,	"GeForce Go 6200" },
	{ 0x10DE0168,	NV_SUB_IDS,	"GeForce Go 6400" },
	{ 0x10DE0169,	NV_SUB_IDS,	"GeForce 6250" },
	{ 0x10DE016A,	NV_SUB_IDS,	"GeForce 7100 GS" },
	{ 0x10DE016C,	NV_SUB_IDS,	"NVIDIA NV44GLM" },
	{ 0x10DE016D,	NV_SUB_IDS,	"NVIDIA NV44GLM" },
	// 0170 - 017F	
	// 0180 - 018F	
	// 0190 - 019F		
	{ 0x10DE0191,	NV_SUB_IDS,	"GeForce 8800 GTX" },
	{ 0x10DE0193,	NV_SUB_IDS,	"GeForce 8800 GTS" },
	{ 0x10DE0194,	NV_SUB_IDS,	"GeForce 8800 Ultra" },
	{ 0x10DE0197,	NV_SUB_IDS,	"Tesla C870" },
	{ 0x10DE019D,	NV_SUB_IDS,	"Quadro FX 5600" },
	{ 0x10DE019E,	NV_SUB_IDS,	"Quadro FX 4600" },
	// 01A0 - 01AF
	// 01B0 - 01BF
	// 01C0 - 01CF
	// 01D0 - 01DF
	{ 0x10DE01D0,	NV_SUB_IDS,	"GeForce 7350 LE" },
	{ 0x10DE01D1,	NV_SUB_IDS,	"GeForce 7300 LE" },
	{ 0x10DE01D2,	NV_SUB_IDS,	"GeForce 7550 LE" },
	{ 0x10DE01D3,	NV_SUB_IDS,	"GeForce 7300 SE/7200 GS" },
	{ 0x10DE01D6,	NV_SUB_IDS,	"GeForce Go 7200" },
	{ 0x10DE01D7,	NV_SUB_IDS,	"Quadro NVS 110M / GeForce Go 7300" },
	{ 0x10DE01D8,	NV_SUB_IDS,	"GeForce Go 7400" },
	{ 0x10DE01D9,	NV_SUB_IDS,	"GeForce Go 7450" },
	{ 0x10DE01DA,	NV_SUB_IDS,	"Quadro NVS 110M" },
	{ 0x10DE01DB,	NV_SUB_IDS,	"Quadro NVS 120M" },
	{ 0x10DE01DC,	NV_SUB_IDS,	"Quadro FX 350M" },
	{ 0x10DE01DD,	NV_SUB_IDS,	"GeForce 7500 LE" },
	{ 0x10DE01DE,	NV_SUB_IDS,	"Quadro FX 350" },
	{ 0x10DE01DF,	NV_SUB_IDS,	"GeForce 7300 GS" },
	// 01E0 - 01EF	
	// 01F0 - 01FF
	{ 0x10DE01F0,	NV_SUB_IDS,	"GeForce4 MX" },
	// 0200 - 020F	
	// 0210 - 021F	
	{ 0x10DE0211,	NV_SUB_IDS,	"GeForce 6800" },
	{ 0x10DE0212,	NV_SUB_IDS,	"GeForce 6800 LE" },
	{ 0x10DE0215,	NV_SUB_IDS,	"GeForce 6800 GT" },
	{ 0x10DE0218,	NV_SUB_IDS,	"GeForce 6800 XT" },
	// 0220 - 022F
	{ 0x10DE0221,	NV_SUB_IDS,	"GeForce 6200" },
	{ 0x10DE0222,	NV_SUB_IDS,	"GeForce 6200 A-LE" },
	{ 0x10DE0228,	NV_SUB_IDS,	"NVIDIA NV44M" },
	// 0230 - 023F
	// 0240 - 024F
	{ 0x10DE0240,	NV_SUB_IDS,	"GeForce 6150" },
	{ 0x10DE0241,	NV_SUB_IDS,	"GeForce 6150 LE" },
	{ 0x10DE0242,	NV_SUB_IDS,	"GeForce 6100" },
	{ 0x10DE0243,	NV_SUB_IDS,	"NVIDIA C51" },
	{ 0x10DE0244,	NV_SUB_IDS,	"GeForce Go 6150" },
	{ 0x10DE0245,	NV_SUB_IDS,	"Quadro NVS 210S / GeForce 6150LE" },
	{ 0x10DE0247,	NV_SUB_IDS,	"GeForce Go 6100" },
	// 0250 - 025F
	{ 0x10DE025B,	NV_SUB_IDS,	"Quadro4 700 XGL" },
	// 0260 - 026F
	// 0270 - 027F
	// 0280 - 028F
	// 0290 - 029F
	{ 0x10DE0290,	NV_SUB_IDS,	"GeForce 7900 GTX" },
	{ 0x10DE0291,	NV_SUB_IDS,	"GeForce 7900 GT/GTO" },
	{ 0x10DE0292,	NV_SUB_IDS,	"GeForce 7900 GS" },
	{ 0x10DE0293,	NV_SUB_IDS,	"GeForce 7950 GX2" },
	{ 0x10DE0294,	NV_SUB_IDS,	"GeForce 7950 GX2" },
	{ 0x10DE0295,	NV_SUB_IDS,	"GeForce 7950 GT" },
	{ 0x10DE0298,	NV_SUB_IDS,	"GeForce Go 7900 GS" },
	{ 0x10DE0299,	NV_SUB_IDS,	"GeForce Go 7900 GTX" },
	{ 0x10DE029A,	NV_SUB_IDS,	"Quadro FX 2500M" },
	{ 0x10DE029B,	NV_SUB_IDS,	"Quadro FX 1500M" },
	{ 0x10DE029C,	NV_SUB_IDS,	"Quadro FX 5500" },
	{ 0x10DE029D,	NV_SUB_IDS,	"Quadro FX 3500" },
	{ 0x10DE029E,	NV_SUB_IDS,	"Quadro FX 1500" },
	{ 0x10DE029F,	NV_SUB_IDS,	"Quadro FX 4500 X2" },
	// 02A0 - 02AF
	// 02B0 - 02BF
	// 02C0 - 02CF
	// 02D0 - 02DF
	// 02E0 - 02EF
	{ 0x10DE02E0,	NV_SUB_IDS,	"GeForce 7600 GT" },
	{ 0x10DE02E1,	NV_SUB_IDS,	"GeForce 7600 GS" },
	{ 0x10DE02E2,	NV_SUB_IDS,	"GeForce 7300 GT" },
	{ 0x10DE02E3,	NV_SUB_IDS,	"GeForce 7900 GS" },
	{ 0x10DE02E4,	NV_SUB_IDS,	"GeForce 7950 GT" },
	// 02F0 - 02FF
	// 0300 - 030F
	{ 0x10DE0301,	NV_SUB_IDS,	"GeForce FX 5800 Ultra" },
	{ 0x10DE0302,	NV_SUB_IDS,	"GeForce FX 5800" },
	{ 0x10DE0308,	NV_SUB_IDS,	"Quadro FX 2000" },
	{ 0x10DE0309,	NV_SUB_IDS,	"Quadro FX 1000" },
	// 0310 - 031F
	{ 0x10DE0311,	NV_SUB_IDS,	"GeForce FX 5600 Ultra" },
	{ 0x10DE0312,	NV_SUB_IDS,	"GeForce FX 5600" },
	{ 0x10DE0314,	NV_SUB_IDS,	"GeForce FX 5600XT" },
	{ 0x10DE031A,	NV_SUB_IDS,	"GeForce FX Go5600" },
	{ 0x10DE031B,	NV_SUB_IDS,	"GeForce FX Go5650" },
	{ 0x10DE031C,	NV_SUB_IDS,	"Quadro FX Go700" },
	// 0320 - 032F
	{ 0x10DE0320,	NV_SUB_IDS,	"GeForce FX 5200" },
	{ 0x10DE0321,	NV_SUB_IDS,	"GeForce FX 5200 Ultra" },
	{ 0x10DE0322,	NV_SUB_IDS,	"GeForce FX 5200" },
	{ 0x10DE0323,	NV_SUB_IDS,	"GeForce FX 5200 LE" },
	{ 0x10DE0324,	NV_SUB_IDS,	"GeForce FX Go5200" },
	{ 0x10DE0325,	NV_SUB_IDS,	"GeForce FX Go5250" },
	{ 0x10DE0326,	NV_SUB_IDS,	"GeForce FX 5500" },
	{ 0x10DE0328,	NV_SUB_IDS,	"GeForce FX Go5200 32M/64M" },
	{ 0x10DE0329,	NV_SUB_IDS,	"GeForce FX Go5200" },
	{ 0x10DE032A,	NV_SUB_IDS,	"Quadro NVS 55/280 PCI" },
	{ 0x10DE032B,	NV_SUB_IDS,	"Quadro FX 500/600 PCI" },
	{ 0x10DE032C,	NV_SUB_IDS,	"GeForce FX Go53xx Series" },
	{ 0x10DE032D,	NV_SUB_IDS,	"GeForce FX Go5100" },
	{ 0x10DE032F,	NV_SUB_IDS,	"NVIDIA NV34GL" },
	// 0330 - 033F
	{ 0x10DE0330,	NV_SUB_IDS,	"GeForce FX 5900 Ultra" },
	{ 0x10DE0331,	NV_SUB_IDS,	"GeForce FX 5900" },
	{ 0x10DE0332,	NV_SUB_IDS,	"GeForce FX 5900XT" },
	{ 0x10DE0333,	NV_SUB_IDS,	"GeForce FX 5950 Ultra" },
	{ 0x10DE0334,	NV_SUB_IDS,	"GeForce FX 5900ZT" },
	{ 0x10DE0338,	NV_SUB_IDS,	"Quadro FX 3000" },
	{ 0x10DE033F,	NV_SUB_IDS,	"Quadro FX 700" },
	// 0340 - 034F
	{ 0x10DE0341,	NV_SUB_IDS,	"GeForce FX 5700 Ultra" },
	{ 0x10DE0342,	NV_SUB_IDS,	"GeForce FX 5700" },
	{ 0x10DE0343,	NV_SUB_IDS,	"GeForce FX 5700LE" },
	{ 0x10DE0344,	NV_SUB_IDS,	"GeForce FX 5700VE" },
	{ 0x10DE0345,	NV_SUB_IDS,	"NV36.5" },
	{ 0x10DE0347,	NV_SUB_IDS,	"GeForce FX Go5700" },
	{ 0x10DE0348,	NV_SUB_IDS,	"GeForce FX Go5700" },
	{ 0x10DE0349,	NV_SUB_IDS,	"NV36M Pro" },
	{ 0x10DE034B,	NV_SUB_IDS,	"NV36MAP" },
	{ 0x10DE034C,	NV_SUB_IDS,	"Quadro FX Go1000" },
	{ 0x10DE034E,	NV_SUB_IDS,	"Quadro FX 1100" },
	{ 0x10DE034F,	NV_SUB_IDS,	"NV36GL" },
	// 0350 - 035F
	// 0360 - 036F
	// 0370 - 037F
	// 0380 - 038F
	{ 0x10DE038B,	NV_SUB_IDS,	"GeForce 7650 GS" },
	// 0390 - 039F
	{ 0x10DE0390,	NV_SUB_IDS,	"GeForce 7650 GS" },
	{ 0x10DE0391,	NV_SUB_IDS,	"GeForce 7600 GT" },
	{ 0x10DE0392,	NV_SUB_IDS,	"GeForce 7600 GS" },
	{ 0x10DE0393,	NV_SUB_IDS,	"GeForce 7300 GT" },
	{ 0x10DE0394,	NV_SUB_IDS,	"GeForce 7600 LE" },
	{ 0x10DE0395,	NV_SUB_IDS,	"GeForce 7300 GT" },
	{ 0x10DE0397,	NV_SUB_IDS,	"GeForce Go 7700" },
	{ 0x10DE0398,	NV_SUB_IDS,	"GeForce Go 7600" },
	{ 0x10DE0399,	NV_SUB_IDS,	"GeForce Go 7600 GT"},
	{ 0x10DE039A,	NV_SUB_IDS,	"Quadro NVS 300M" },
	{ 0x10DE039B,	NV_SUB_IDS,	"GeForce Go 7900 SE" },
	{ 0x10DE039C,	NV_SUB_IDS,	"Quadro FX 560M" },
	{ 0x10DE039E,	NV_SUB_IDS,	"Quadro FX 560" },
	// 03A0 - 03AF
	// 03B0 - 03BF
	// 03C0 - 03CF
	// 03D0 - 03DF
	{ 0x10DE03D0,	NV_SUB_IDS,	"GeForce 6150SE nForce 430" },
	{ 0x10DE03D1,	NV_SUB_IDS,	"GeForce 6100 nForce 405" },
	{ 0x10DE03D2,	NV_SUB_IDS,	"GeForce 6100 nForce 400" },
	{ 0x10DE03D5,	NV_SUB_IDS,	"GeForce 6100 nForce 420" },
	{ 0x10DE03D6,	NV_SUB_IDS,	"GeForce 7025 / nForce 630a" },
	// 03E0 - 03EF
	// 03F0 - 03FF
	// 0400 - 040F
	{ 0x10DE0400,	NV_SUB_IDS,	"GeForce 8600 GTS" },
	{ 0x10DE0401,	NV_SUB_IDS,	"GeForce 8600 GT" },
	{ 0x10DE0402,	NV_SUB_IDS,	"GeForce 8600 GT" },
	{ 0x10DE0403,	NV_SUB_IDS,	"GeForce 8600 GS" },
	{ 0x10DE0404,	NV_SUB_IDS,	"GeForce 8400 GS" },
	{ 0x10DE0405,	NV_SUB_IDS,	"GeForce 9500M GS" },
	{ 0x10DE0406,	NV_SUB_IDS,	"GeForce 8300 GS" },
	{ 0x10DE0407,	NV_SUB_IDS,	"GeForce 8600M GT" },
	{ 0x10DE0408,	NV_SUB_IDS,	"GeForce 9650M GS" },
	{ 0x10DE0409,	NV_SUB_IDS,	"GeForce 8700M GT" },
	{ 0x10DE040A,	NV_SUB_IDS,	"Quadro FX 370" },
	{ 0x10DE040B,	NV_SUB_IDS,	"Quadro NVS 320M" },
	{ 0x10DE040C,	NV_SUB_IDS,	"Quadro FX 570M" },
	{ 0x10DE040D,	NV_SUB_IDS,	"Quadro FX 1600M" },
	{ 0x10DE040E,	NV_SUB_IDS,	"Quadro FX 570" },
	{ 0x10DE040F,	NV_SUB_IDS,	"Quadro FX 1700" },
	// 0410 - 041F
	{ 0x10DE0410,	NV_SUB_IDS,	"GeForce GT 330" },
	// 0420 - 042F
	{ 0x10DE0420,	NV_SUB_IDS,	"GeForce 8400 SE" },
	{ 0x10DE0421,	NV_SUB_IDS,	"GeForce 8500 GT" },
	{ 0x10DE0422,	NV_SUB_IDS,	"GeForce 8400 GS" },
	{ 0x10DE0423,	NV_SUB_IDS,	"GeForce 8300 GS" },
	{ 0x10DE0424,	NV_SUB_IDS,	"GeForce 8400 GS" },
	{ 0x10DE0425,	NV_SUB_IDS,	"GeForce 8600M GS" },
	{ 0x10DE0426,	NV_SUB_IDS,	"GeForce 8400M GT" },
	{ 0x10DE0427,	NV_SUB_IDS,	"GeForce 8400M GS" },
	{ 0x10DE0428,	NV_SUB_IDS,	"GeForce 8400M G" },
	{ 0x10DE0429,	NV_SUB_IDS,	"Quadro NVS 140M" },
	{ 0x10DE042A,	NV_SUB_IDS,	"Quadro NVS 130M" },
	{ 0x10DE042B,	NV_SUB_IDS,	"Quadro NVS 135M" },
	{ 0x10DE042C,	NV_SUB_IDS,	"GeForce 9400 GT" },
	{ 0x10DE042D,	NV_SUB_IDS,	"Quadro FX 360M" },
	{ 0x10DE042E,	NV_SUB_IDS,	"GeForce 9300M G" },
	{ 0x10DE042F,	NV_SUB_IDS,	"Quadro NVS 290" },
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
	{ 0x10DE04C0,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04C1,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04C2,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04C3,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04C4,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04C5,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04C6,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04C7,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04C8,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04C9,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04CA,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04CB,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04CC,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04CD,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04CE,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04CF,	NV_SUB_IDS,	"NVIDIA G78" },
	// 04D0 - 04DF
	// 04E0 - 04EF
	// 04F0 - 04FF
	// 0500 - 050F
	// 0510 - 051F
	// 0520 - 052F
	// 0530 - 053F
	{ 0x10DE0530,	NV_SUB_IDS,	"GeForce 7190M / nForce 650M" },
	{ 0x10DE0531,	NV_SUB_IDS,	"GeForce 7150M / nForce 630M" },
	{ 0x10DE0533,	NV_SUB_IDS,	"GeForce 7000M / nForce 610M" },
	{ 0x10DE053A,	NV_SUB_IDS,	"GeForce 7050 PV / nForce 630a" },
	{ 0x10DE053B,	NV_SUB_IDS,	"GeForce 7050 PV / nForce 630a" },
	{ 0x10DE053E,	NV_SUB_IDS,	"GeForce 7025 / nForce 630a" },
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
	{ 0x10DE05E0,	NV_SUB_IDS,	"GeForce GTX 295" },
	{ 0x10DE05E1,	NV_SUB_IDS,	"GeForce GTX 280" },
	{ 0x10DE05E2,	NV_SUB_IDS,	"GeForce GTX 260" },
	{ 0x10DE05E3,	NV_SUB_IDS,	"GeForce GTX 285" },
	{ 0x10DE05E4,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05E5,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05E6,	NV_SUB_IDS,	"GeForce GTX 275" },
	{ 0x10DE05E7,	NV_SUB_IDS,	"nVidia Tesla C1060" },
	{ 0x10DE05E8,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05E9,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05EA,	NV_SUB_IDS,	"GeForce GTX 260" },
	{ 0x10DE05EB,	NV_SUB_IDS,	"GeForce GTX 295" },
	{ 0x10DE05EC,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05ED,	NV_SUB_IDS,	"Quadroplex 2200 D2" },
	{ 0x10DE05EE,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05EF,	NV_SUB_IDS,	"NVIDIA GT200" },
	// 05F0 - 05FF
	{ 0x10DE05F0,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05F1,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05F2,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05F3,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05F4,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05F5,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05F6,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05F7,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05F8,	NV_SUB_IDS,	"Quadroplex 2200 S4" },
	{ 0x10DE05F9,	NV_SUB_IDS,	"NVIDIA Quadro CX" },
	{ 0x10DE05FA,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05FB,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05FC,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05FD,	NV_SUB_IDS,	"Quadro FX 5800" },
	{ 0x10DE05FE,	NV_SUB_IDS,	"Quadro FX 4800" },
	{ 0x10DE05FF,	NV_SUB_IDS,	"Quadro FX 3800" },
	// 0600 - 060F
	{ 0x10DE0600,	NV_SUB_IDS,	"GeForce 8800 GTS 512" },
	{ 0x10DE0601,	NV_SUB_IDS,	"GeForce 9800 GT" },
	{ 0x10DE0602,	NV_SUB_IDS,	"GeForce 8800 GT" },
	{ 0x10DE0603,	NV_SUB_IDS,	"GeForce GT 230" },
	{ 0x10DE0604,	NV_SUB_IDS,	"GeForce 9800 GX2" },
	{ 0x10DE0605,	NV_SUB_IDS,	"GeForce 9800 GT" },
	{ 0x10DE0606,	NV_SUB_IDS,	"GeForce 8800 GS" },
	{ 0x10DE0607,	NV_SUB_IDS,	"GeForce GTS 240" },
	{ 0x10DE0608,	NV_SUB_IDS,	"GeForce 9800M GTX" },
	{ 0x10DE0609,	NV_SUB_IDS,	"GeForce 8800M GTS" },
	{ 0x10DE060A,	NV_SUB_IDS,	"GeForce GTX 280M" },
	{ 0x10DE060B,	NV_SUB_IDS,	"GeForce 9800M GT" },
	{ 0x10DE060C,	NV_SUB_IDS,	"GeForce 8800M GTX" },
	{ 0x10DE060D,	NV_SUB_IDS,	"GeForce 8800 GS" },
	{ 0x10DE060F,	NV_SUB_IDS,	"GeForce GTX 285M" },
	// 0610 - 061F
	{ 0x10DE0610,	NV_SUB_IDS,	"GeForce 9600 GSO" },
	{ 0x10DE0611,	NV_SUB_IDS,	"GeForce 8800 GT" },
	{ 0x10DE0612,	NV_SUB_IDS,	"GeForce 9800 GTX" },
	{ 0x10DE0613,	NV_SUB_IDS,	"GeForce 9800 GTX+" },
	{ 0x10DE0614,	NV_SUB_IDS,	"GeForce 9800 GT" },
	{ 0x10DE0615,	NV_SUB_IDS,	"GeForce GTS 250" },
	{ 0x10DE0617,	NV_SUB_IDS,	"GeForce 9800M GTX" },
	{ 0x10DE0618,	NV_SUB_IDS,	"GeForce GTX 170M" },
	{ 0x10DE0619,	NV_SUB_IDS,	"Quadro FX 4700 X2" },
	{ 0x10DE061A,	NV_SUB_IDS,	"Quadro FX 3700" },
	{ 0x10DE061B,	NV_SUB_IDS,	"Quadro VX 200" },
	{ 0x10DE061C,	NV_SUB_IDS,	"Quadro FX 3600M" },
	{ 0x10DE061D,	NV_SUB_IDS,	"Quadro FX 2800M" },
	{ 0x10DE061E,	NV_SUB_IDS,	"Quadro FX 3700M" },
	{ 0x10DE061F,	NV_SUB_IDS,	"Quadro FX 3800M" },
	// 0620 - 062F
	{ 0x10DE0620,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE0621,	NV_SUB_IDS,	"GeForce GT 230" },
	{ 0x10DE0622,	NV_SUB_IDS,	"GeForce 9600 GT" },
	{ 0x10DE0623,	NV_SUB_IDS,	"GeForce 9600 GS" },
	{ 0x10DE0624,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE0625,	NV_SUB_IDS,	"GeForce 9600 GSO 512"},
	{ 0x10DE0626,	NV_SUB_IDS,	"GeForce GT 130" },
	{ 0x10DE0627,	NV_SUB_IDS,	"GeForce GT 140" },
	{ 0x10DE0628,	NV_SUB_IDS,	"GeForce 9800M GTS" },
	{ 0x10DE0629,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE062A,	NV_SUB_IDS,	"GeForce 9700M GTS" },
	{ 0x10DE062B,	NV_SUB_IDS,	"GeForce 9800M GS" },
	{ 0x10DE062C,	NV_SUB_IDS,	"GeForce 9800M GTS" },
	{ 0x10DE062D,	NV_SUB_IDS,	"GeForce 9600 GT" },
	{ 0x10DE062E,	NV_SUB_IDS,	"GeForce 9600 GT" },
	{ 0x10DE062F,	NV_SUB_IDS,	"GeForce 9800 S" },
	// 0630 - 063F
	{ 0x10DE0630,	NV_SUB_IDS,	"GeForce 9700 S" },
	{ 0x10DE0631,	NV_SUB_IDS,	"GeForce GTS 160M" },
	{ 0x10DE0632,	NV_SUB_IDS,	"GeForce GTS 150M" },
	{ 0x10DE0633,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE0634,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE0635,	NV_SUB_IDS,	"GeForce 9600 GSO" },
	{ 0x10DE0636,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE0637,	NV_SUB_IDS,	"GeForce 9600 GT" },
	{ 0x10DE0638,	NV_SUB_IDS,	"Quadro FX 1800" },
	{ 0x10DE0639,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE063A,	NV_SUB_IDS,	"Quadro FX 2700M" },
	{ 0x10DE063B,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE063C,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE063D,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE063E,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE063F,	NV_SUB_IDS,	"NVIDIA G94" },
	// 0640 - 064F
	{ 0x10DE0640,	NV_SUB_IDS,	"GeForce 9500 GT" },
	{ 0x10DE0641,	NV_SUB_IDS,	"GeForce 9400 GT" },
	{ 0x10DE0642,	NV_SUB_IDS,	"GeForce 8400 GS" },
	{ 0x10DE0643,	NV_SUB_IDS,	"GeForce 9500 GT" },
	{ 0x10DE0644,	NV_SUB_IDS,	"GeForce 9500 GS" },
	{ 0x10DE0645,	NV_SUB_IDS,	"GeForce 9500 GS" },
	{ 0x10DE0646,	NV_SUB_IDS,	"GeForce GT 120" },
	{ 0x10DE0647,	NV_SUB_IDS,	"GeForce 9600M GT" },
	{ 0x10DE0648,	NV_SUB_IDS,	"GeForce 9600M GS" },
	{ 0x10DE0649,	NV_SUB_IDS,	"GeForce 9600M GT" },
	{ 0x10DE064A,	NV_SUB_IDS,	"GeForce 9700M GT" },
	{ 0x10DE064B,	NV_SUB_IDS,	"GeForce 9500M G" },
	{ 0x10DE064C,	NV_SUB_IDS,	"GeForce 9650M GT" },
	// 0650 - 065F
	{ 0x10DE0650,	NV_SUB_IDS,	"NVIDIA G96-825" },
	{ 0x10DE0651,	NV_SUB_IDS,	"GeForce G 110M" },
	{ 0x10DE0652,	NV_SUB_IDS,	"GeForce GT 130M" },
	{ 0x10DE0653,	NV_SUB_IDS,	"GeForce GT 120M" },
	{ 0x10DE0654,	NV_SUB_IDS,	"GeForce GT 220M" },
	{ 0x10DE0655,	NV_SUB_IDS,	"GeForce GT 120" },
	{ 0x10DE0656,	NV_SUB_IDS,	"GeForce 9650 S" },
	{ 0x10DE0657,	NV_SUB_IDS,	"NVIDIA G96" },
	{ 0x10DE0658,	NV_SUB_IDS,	"Quadro FX 380" },
	{ 0x10DE0659,	NV_SUB_IDS,	"Quadro FX 580" },
	{ 0x10DE065A,	NV_SUB_IDS,	"Quadro FX 1700M" },
	{ 0x10DE065B,	NV_SUB_IDS,	"GeForce 9400 GT" },
	{ 0x10DE065C,	NV_SUB_IDS,	"Quadro FX 770M" },
	{ 0x10DE065D,	NV_SUB_IDS,	"NVIDIA G96" },
	{ 0x10DE065E,	NV_SUB_IDS,	"NVIDIA G96" },
	{ 0x10DE065F,	NV_SUB_IDS,	"GeForce G210" },
	// 0660 - 066F
	// 0670 - 067F
	// 0680 - 068F
	// 0690 - 069F
	// 06A0 - 06AF
	{ 0x10DE06A0,	NV_SUB_IDS,	"NVIDIA GT214" },
	// 06B0 - 06BF
	{ 0x10DE06B0,	NV_SUB_IDS,	"NVIDIA GT214" },
	// 06C0 - 06CF
	{ 0x10DE06C0,	NV_SUB_IDS,	"GeForce GTX 480" },
	{ 0x10DE06C3,	NV_SUB_IDS,	"GeForce GTX D12U" },
	{ 0x10DE06C4,	NV_SUB_IDS,	"GeForce GTX 465" },
	{ 0x10DE06CA,	NV_SUB_IDS,	"GeForce GTX 480M" },
	{ 0x10DE06CD,	NV_SUB_IDS,	"GeForce GTX 470" },
	// 06D0 - 06DF
	{ 0x10DE06D1,	NV_SUB_IDS,	"Tesla C2050 / C2070" },
	{ 0x10DE06D2,	NV_SUB_IDS,	"Tesla M2070 / X2070" },
	{ 0x10DE06D8,	NV_SUB_IDS,	"Quadro 6000" },
	{ 0x10DE06D9,	NV_SUB_IDS,	"Quadro 5000" },
	{ 0x10DE06DA,	NV_SUB_IDS,	"Quadro 5000M" },
	{ 0x10DE06DC,	NV_SUB_IDS,	"Quadro 6000" },
	{ 0x10DE06DD,	NV_SUB_IDS,	"nVidia Quadro 4000" },
	{ 0x10DE06DE,	NV_SUB_IDS,	"nVidia Tesla S2050" },
	{ 0x10DE06DF,	NV_SUB_IDS,	"Tesla M2070Q" },
	// 06E0 - 06EF
	{ 0x10DE06E0,	NV_SUB_IDS,	"GeForce 9300 GE" },
	{ 0x10DE06E1,	NV_SUB_IDS,	"GeForce 9300 GS" },
	{ 0x10DE06E2,	NV_SUB_IDS,	"GeForce 8400" },
	{ 0x10DE06E3,	NV_SUB_IDS,	"GeForce 8400 SE" },
	{ 0x10DE06E4,	NV_SUB_IDS,	"GeForce 8400 GS" },
	{ 0x10DE06E5,	NV_SUB_IDS,	"GeForce 9300M GS" },
	{ 0x10DE06E6,	NV_SUB_IDS,	"GeForce G100" },
	{ 0x10DE06E7,	NV_SUB_IDS,	"GeForce 9300 SE" },
	{ 0x10DE06E8,	NV_SUB_IDS,	"GeForce 9200M GS" },
	{ 0x10DE06E9,	NV_SUB_IDS,	"GeForce 9300M GS" },
	{ 0x10DE06EA,	NV_SUB_IDS,	"Quadro NVS 150M" },
	{ 0x10DE06EB,	NV_SUB_IDS,	"Quadro NVS 160M" },
	{ 0x10DE06EC,	NV_SUB_IDS,	"GeForce G 105M" },
	{ 0x10DE06ED,	NV_SUB_IDS,	"NVIDIA G98" },
	{ 0x10DE06EF,	NV_SUB_IDS,	"GeForce G 103M" },
	// 06F0 - 06FF
	{ 0x10DE06F0,	NV_SUB_IDS,	"NVIDIA G98" },
	{ 0x10DE06F1,	NV_SUB_IDS,	"GeForce G105M" },
	{ 0x10DE06F2,	NV_SUB_IDS,	"NVIDIA G98" },
	{ 0x10DE06F3,	NV_SUB_IDS,	"NVIDIA G98" },
	{ 0x10DE06F4,	NV_SUB_IDS,	"NVIDIA G98" },
	{ 0x10DE06F5,	NV_SUB_IDS,	"NVIDIA G98" },
	{ 0x10DE06F6,	NV_SUB_IDS,	"NVIDIA G98" },
	{ 0x10DE06F7,	NV_SUB_IDS,	"NVIDIA G98" },
	{ 0x10DE06F8,	NV_SUB_IDS,	"Quadro NVS 420" },
	{ 0x10DE06F9,	NV_SUB_IDS,	"Quadro FX 370 LP" },
	{ 0x10DE06FA,	NV_SUB_IDS,	"Quadro NVS 450" },
	{ 0x10DE06FB,	NV_SUB_IDS,	"Quadro FX 370M" },
	{ 0x10DE06FC,	NV_SUB_IDS,	"NVIDIA G98" },
	{ 0x10DE06FD,	NV_SUB_IDS,	"Quadro NVS 295" },
	{ 0x10DE06FE,	NV_SUB_IDS,	"NVIDIA G98" },
	{ 0x10DE06FF,	NV_SUB_IDS,	"HICx16 + Graphics" },
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
	{ 0x10DE07E0,	NV_SUB_IDS,	"GeForce 7150 / nForce 630i" },
	{ 0x10DE07E1,	NV_SUB_IDS,	"GeForce 7100 / nForce 630i" },
	{ 0x10DE07E2,	NV_SUB_IDS,	"GeForce 7050 / nForce 630i" },
	{ 0x10DE07E3,	NV_SUB_IDS,	"GeForce 7050 / nForce 610i" },
	{ 0x10DE07E5,	NV_SUB_IDS,	"GeForce 7050 / nForce 620i" },
	// 07F0 - 07FF
	// 0800 - 080F
	// 0810 - 081F
	// 0820 - 082F
	// 0830 - 083F
	// 0840 - 084F
	{ 0x10DE0840,	NV_SUB_IDS,	"GeForce 8200M" },
	{ 0x10DE0844,	NV_SUB_IDS,	"GeForce 9100M G" },
	{ 0x10DE0845,	NV_SUB_IDS,	"GeForce 8200M G" },
	{ 0x10DE0846,	NV_SUB_IDS,	"GeForce 9200" },
	{ 0x10DE0847,	NV_SUB_IDS,	"GeForce 9100" },
	{ 0x10DE0848,	NV_SUB_IDS,	"GeForce 8300" },
	{ 0x10DE0849,	NV_SUB_IDS,	"GeForce 8200" },
	{ 0x10DE084A,	NV_SUB_IDS,	"nForce 730a" },
	{ 0x10DE084B,	NV_SUB_IDS,	"GeForce 9200" }, // nVidia GeForce 8200 ??
	{ 0x10DE084C,	NV_SUB_IDS,	"nForce 980a/780a SLI" },
	{ 0x10DE084D,	NV_SUB_IDS,	"nForce 750a SLI" },
	{ 0x10DE084F,	NV_SUB_IDS,	"GeForce 8100 / nForce 720a" },
	// 0850 - 085F
	// 0860 - 086F
	{ 0x10DE0860,	NV_SUB_IDS,	"GeForce 9300" },
	{ 0x10DE0861,	NV_SUB_IDS,	"GeForce 9400" },
	{ 0x10DE0862,	NV_SUB_IDS,	"GeForce 9400M G" },
	{ 0x10DE0863,	NV_SUB_IDS,	"GeForce 9400M" },
	{ 0x10DE0864,	NV_SUB_IDS,	"GeForce 9300" },
	{ 0x10DE0865,	NV_SUB_IDS,	"GeForce 9300" }, // ION ??
	{ 0x10DE0866,	NV_SUB_IDS,	"GeForce 9400M G" },
	{ 0x10DE0867,	NV_SUB_IDS,	"GeForce 9400" },
	{ 0x10DE0868,	NV_SUB_IDS,	"nForce 760i SLI" },
	{ 0x10DE0869,	NV_SUB_IDS,	"GeForce 9400" },
	{ 0x10DE086A,	NV_SUB_IDS,	"GeForce 9400" },
	{ 0x10DE086C,	NV_SUB_IDS,	"GeForce 9300 / nForce 730i" },
	{ 0x10DE086D,	NV_SUB_IDS,	"GeForce 9200" },
	{ 0x10DE086E,	NV_SUB_IDS,	"GeForce 9100M G" },
	{ 0x10DE086F,	NV_SUB_IDS,	"GeForce 8200M G" },
	// 0870 - 087F
	{ 0x10DE0870,	NV_SUB_IDS,	"GeForce 9400M" },
	{ 0x10DE0871,	NV_SUB_IDS,	"GeForce 9200" },
	{ 0x10DE0872,	NV_SUB_IDS,	"GeForce G102M" },
	{ 0x10DE0873,	NV_SUB_IDS,	"GeForce G205M" },
	{ 0x10DE0874,	NV_SUB_IDS,	"ION 9300M" },
	{ 0x10DE0876,	NV_SUB_IDS,	"ION 9400M" },
	{ 0x10DE087A,	NV_SUB_IDS,	"GeForce 9400" },
	{ 0x10DE087D,	NV_SUB_IDS,	"ION 9400M" },
	{ 0x10DE087E,	NV_SUB_IDS,	"ION LE" },
	{ 0x10DE087F,	NV_SUB_IDS,	"ION LE" },
	// 0880 - 088F
	// 0890 - 089F
	// 08A0 - 08AF
	{ 0x10DE08A0,	NV_SUB_IDS,	"GeForce 320M" },
	{ 0x10DE08A1,	NV_SUB_IDS,	"MCP89-MZT" },
	{ 0x10DE08A2,	NV_SUB_IDS,	"GeForce 320M" },
	{ 0x10DE08A3,	NV_SUB_IDS,	"GeForce 320M" },
	{ 0x10DE08A4,	NV_SUB_IDS,	"GeForce 320M" },
	{ 0x10DE08A5,	NV_SUB_IDS,	"GeForce 320M" },
	// 08B0 - 08BF
	{ 0x10DE08B0,	NV_SUB_IDS,	"MCP83 MMD" },
	{ 0x10DE08B1,	NV_SUB_IDS,	"GeForce 300M" },
	{ 0x10DE08B2,	NV_SUB_IDS,	"GeForce 300M" }, // MCP83-MJ
	{ 0x10DE08B3,	NV_SUB_IDS,	"MCP89 MM9" },
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
	// { 0x10DE0A00,	NV_SUB_IDS,	"NVIDIA GT212" },
	// 0A10 - 0A1F
	// { 0x10DE0A10,	NV_SUB_IDS,	"NVIDIA GT212" },
	// 0A20 - 0A2F
	{ 0x10DE0A20,	NV_SUB_IDS,	"GeForce GT 220" },
	{ 0x10DE0A21,	NV_SUB_IDS,	"D10M2-20" },
	{ 0x10DE0A22,	NV_SUB_IDS,	"GeForce 315" },
	{ 0x10DE0A23,	NV_SUB_IDS,	"GeForce 210" },
	{ 0x10DE0A26,	NV_SUB_IDS,	"GeForce 405" },
	{ 0x10DE0A27,	NV_SUB_IDS,	"GeForce 405" },
	{ 0x10DE0A28,	NV_SUB_IDS,	"GeForce GT 230M" },
	{ 0x10DE0A29,	NV_SUB_IDS,	"GeForce GT 330M" },
	{ 0x10DE0A2A,	NV_SUB_IDS,	"GeForce GT 230M" },
	{ 0x10DE0A2B,	NV_SUB_IDS,	"GeForce GT 330M" },
	{ 0x10DE0A2C,	NV_SUB_IDS,	"NVS 5100M" },
	{ 0x10DE0A2D,	NV_SUB_IDS,	"GeForce GT 320M" },	
	// 0A30 - 0A3F
	{ 0x10DE0A30,	NV_SUB_IDS,	"GeForce GT 330M" },
	{ 0x10DE0A32,	NV_SUB_IDS,	"GeForce GT 415" },
	{ 0x10DE0A34,	NV_SUB_IDS,	"GeForce GT 240M" },
	{ 0x10DE0A35,	NV_SUB_IDS,	"GeForce GT 325M" },
	{ 0x10DE0A38,	NV_SUB_IDS,	"Quadro 400" },
	{ 0x10DE0A3C,	NV_SUB_IDS,	"Quadro FX 880M" },
	{ 0x10DE0A3D,	NV_SUB_IDS,	"N10P-ES" },
	{ 0x10DE0A3F,	NV_SUB_IDS,	"GT216-INT" },
	// 0A40 - 0A4F
	// 0A50 - 0A5F
	// 0A60 - 0A6F
	{ 0x10DE0A60,	NV_SUB_IDS,	"GeForce G210" },
	{ 0x10DE0A61,	NV_SUB_IDS,	"NVS 2100" },
	{ 0x10DE0A62,	NV_SUB_IDS,	"GeForce 205" },
	{ 0x10DE0A63,	NV_SUB_IDS,	"GeForce 310" },
	{ 0x10DE0A64,	NV_SUB_IDS,	"ION" },
	{ 0x10DE0A65,	NV_SUB_IDS,	"GeForce 210" },
	{ 0x10DE0A66,	NV_SUB_IDS,	"GeForce 310" },
	{ 0x10DE0A67,	NV_SUB_IDS,	"GeForce 315" },
	{ 0x10DE0A68,	NV_SUB_IDS,	"GeForce G105M" },
	{ 0x10DE0A69,	NV_SUB_IDS,	"GeForce G105M" },
	{ 0x10DE0A6A,	NV_SUB_IDS,	"NVS 2100M" },
	{ 0x10DE0A6C,	NV_SUB_IDS,	"NVS 3100M" },
	{ 0x10DE0A6E,	NV_SUB_IDS,	"GeForce 305M" },
	{ 0x10DE0A6F,	NV_SUB_IDS,	"ION" },	
	// 0A70 - 0A7F
	{ 0x10DE0A70,	NV_SUB_IDS,	"GeForce 310M" },
	{ 0x10DE0A71,	NV_SUB_IDS,	"GeForce 305M" },
	{ 0x10DE0A72,	NV_SUB_IDS,	"GeForce 310M" },
	{ 0x10DE0A73,	NV_SUB_IDS,	"GeForce 305M" },
	{ 0x10DE0A74,	NV_SUB_IDS,	"GeForce G210M" },
	{ 0x10DE0A75,	NV_SUB_IDS,	"GeForce G310M" },
	{ 0x10DE0A76,	NV_SUB_IDS,	"ION" },
	{ 0x10DE0A78,	NV_SUB_IDS,	"Quadro FX 380 LP" },
	// { 0x10DE0A79,	NV_SUB_IDS,	"N12M-NS-S" },
	{ 0x10DE0A7A,	NV_SUB_IDS,	"GeForce 315M" },
	{ 0x10DE0A7B,	NV_SUB_IDS,	"GeForce 505" },
	{ 0x10DE0A7C,	NV_SUB_IDS,	"Quadro FX 380M" },
	{ 0x10DE0A7D,	NV_SUB_IDS,	"N11M-ES" }, //SUBIDS
	{ 0x10DE0A7E,	NV_SUB_IDS,	"GT218-INT-S" },
	{ 0x10DE0A7F,	NV_SUB_IDS,	"GT218-INT-B" },
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
	{ 0x10DE0CA0,	NV_SUB_IDS,	"GeForce GT 330 " },
	{ 0x10DE0CA2,	NV_SUB_IDS,	"GeForce GT 320" },
	{ 0x10DE0CA3,	NV_SUB_IDS,	"GeForce GT 240" },
	{ 0x10DE0CA4,	NV_SUB_IDS,	"GeForce GT 340" },
	{ 0x10DE0CA5,	NV_SUB_IDS,	"GeForce GT 220" },
	{ 0x10DE0CA7,	NV_SUB_IDS,	"GeForce GT 330" },
	{ 0x10DE0CA8,	NV_SUB_IDS,	"GeForce GTS 260M" },
	{ 0x10DE0CA9,	NV_SUB_IDS,	"GeForce GTS 250M" },
	{ 0x10DE0CAC,	NV_SUB_IDS,	"GeForce GT 220" },
	{ 0x10DE0CAD,	NV_SUB_IDS,	"N10E-ES" },
	{ 0x10DE0CAE,	NV_SUB_IDS,	"GT215-INT" },
	{ 0x10DE0CAF,	NV_SUB_IDS,	"GeForce GT 335M" },
	// 0CB0 - 0CBF	
	{ 0x10DE0CB0,	NV_SUB_IDS,	"GeForce GTS 350M" },
	{ 0x10DE0CB1,	NV_SUB_IDS,	"GeForce GTS 360M" },
	{ 0x10DE0CBC,	NV_SUB_IDS,	"Quadro FX 1800M" },
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
	{ 0x10DE0DC0,	NV_SUB_IDS,	"GeForce GT 440" },
	//  { 0x10DE0DC1,	NV_SUB_IDS,	"D12-P1-35" },
	//  { 0x10DE0DC2,	NV_SUB_IDS,	"D12-P1-35" },
	{ 0x10DE0DC4,	NV_SUB_IDS,	"GeForce GTS 450" },
	{ 0x10DE0DC5,	NV_SUB_IDS,	"GeForce GTS 450" },
	{ 0x10DE0DC6,	NV_SUB_IDS,	"GeForce GTS 450" },
	//  { 0x10DE0DCA,	NV_SUB_IDS,	"GF10x" },
	//  { 0x10DE0DCC,	NV_SUB_IDS,	"N12E-GS" },
	{ 0x10DE0DCD,	NV_SUB_IDS,	"GeForce GT 555M" },
	{ 0x10DE0DCE,	NV_SUB_IDS,	"GeForce GT 555M" },
	//  { 0x10DE0DCF,	NV_SUB_IDS,	"N12P-GT-B" },
	// 0DD0 - 0DDF	
	//  { 0x10DE0DD0,	NV_SUB_IDS,	"N11E-GT" },
	{ 0x10DE0DD1,	NV_SUB_IDS,	"GeForce GTX 460M" },
	{ 0x10DE0DD2,	NV_SUB_IDS,	"GeForce GT 445M" },
	{ 0x10DE0DD3,	NV_SUB_IDS,	"GeForce GT 435M" },
	{ 0x10DE0DD6,	NV_SUB_IDS,	"GeForce GT 550M" },
	{ 0x10DE0DD8,	NV_SUB_IDS,	"Quadro 2000" },
	{ 0x10DE0DDA,	NV_SUB_IDS,	"Quadro 2000M" },
	{ 0x10DE0DDE,	NV_SUB_IDS,	"GF106-ES" },
	//  { 0x10DE0DDF,	NV_SUB_IDS,	"GF106-INT" },
	// 0DE0 - 0DEF
	{ 0x10DE0DE0,	NV_SUB_IDS,	"GeForce GT 440" },
	{ 0x10DE0DE1,	NV_SUB_IDS,	"GeForce GT 430" },
	{ 0x10DE0DE2,	NV_SUB_IDS,	"GeForce GT 420" },
	{ 0x10DE0DE3,	NV_SUB_IDS,	"GeForce GT 635M" },
	{ 0x10DE0DE4,	NV_SUB_IDS,	"GeForce GT 520" },
	{ 0x10DE0DE5,	NV_SUB_IDS,	"GeForce GT 530" },
	{ 0x10DE0DE8,	NV_SUB_IDS,	"GeForce GT 620M" },
	{ 0x10DE0DE9,	NV_SUB_IDS,	"GeForce GT 630M" },
	{ 0x10DE0DEA,	NV_SUB_IDS,	"GeForce GT 610M" },
	{ 0x10DE0DEB,	NV_SUB_IDS,	"GeForce GT 555M" },
	{ 0x10DE0DEC,	NV_SUB_IDS,	"GeForce GT 525M" },
	{ 0x10DE0DED,	NV_SUB_IDS,	"GeForce GT 520M" },
	{ 0x10DE0DEE,	NV_SUB_IDS,	"GeForce GT 415M" },
	{ 0x10DE0DEF,	NV_SUB_IDS,	"N13P-NS1-A1" },
	// 0DF0 - 0DFF
	{ 0x10DE0DF0,	NV_SUB_IDS,	"GeForce GT 425M" },
	{ 0x10DE0DF1,	NV_SUB_IDS,	"GeForce GT 420M" },
	{ 0x10DE0DF2,	NV_SUB_IDS,	"GeForce GT 435M" },
	{ 0x10DE0DF3,	NV_SUB_IDS,	"GeForce GT 420M" },
	{ 0x10DE0DF4,	NV_SUB_IDS,	"GeForce GT 540M" },
	{ 0x10DE0DF5,	NV_SUB_IDS,	"GeForce GT 525M" },
	{ 0x10DE0DF6,	NV_SUB_IDS,	"GeForce GT 550M" },
	{ 0x10DE0DF7,	NV_SUB_IDS,	"GeForce GT 520M" },
	{ 0x10DE0DF8,	NV_SUB_IDS,	"Quadro 600" },
	{ 0x10DE0DF9,	NV_SUB_IDS,	"Quadro 500M" },
	{ 0x10DE0DFA,	NV_SUB_IDS,	"Quadro 1000M" },
	{ 0x10DE0DFC,	NV_SUB_IDS,	"NVS 5200M" },
	{ 0x10DE0DFE,	NV_SUB_IDS,	"GF108 ES" },
	//  { 0x10DE0DFF,	NV_SUB_IDS,	"GF108 INT" },
	// 0E00 - 0E0F
	// 0E10 - 0E1F
	// 0E20 - 0E2F
	{ 0x10DE0E21,	NV_SUB_IDS,	"D12U-25" },
	{ 0x10DE0E22,	NV_SUB_IDS,	"GeForce GTX 460" },
	{ 0x10DE0E23,	NV_SUB_IDS,	"GeForce GTX 460 SE" },
	{ 0x10DE0E24,	NV_SUB_IDS,	"GeForce GTX 460" },
	//  { 0x10DE0E25,	NV_SUB_IDS,	"D12U-50" },
	{ 0x10DE0E28,	NV_SUB_IDS,	"GeForce GTX 460" },
	// 0E30 - 0E3F
	{ 0x10DE0E30,	NV_SUB_IDS,	"GeForce GTX 470M" },
	{ 0x10DE0E31,	NV_SUB_IDS,	"GeForce GTX 485M" },
	//  { 0x10DE0E32,	NV_SUB_IDS,	"N12E-GT" },
	{ 0x10DE0E38,	NV_SUB_IDS,	"GF104GL" },
	{ 0x10DE0E3A,	NV_SUB_IDS,	"Quadro 3000M" },
	{ 0x10DE0E3B,	NV_SUB_IDS,	"Quadro 4000M" },
	//  { 0x10DE0E3E,	NV_SUB_IDS,	"GF104-ES" },
	//  { 0x10DE0E3F,	NV_SUB_IDS,	"GF104-INT" },
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
	{ 0x10DE0F00,	NV_SUB_IDS,	"GeForce GT 630" },
	{ 0x10DE0F01,	NV_SUB_IDS,	"GeForce GT 620" },
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
	{ 0x10DE0FC0,	NV_SUB_IDS,	"GeForce GT 640" },
	{ 0x10DE0FC1,	NV_SUB_IDS,	"GeForce GT 640" },
	{ 0x10DE0FC2,	NV_SUB_IDS,	"GeForce GT 630" },
	{ 0x10DE0FC6,	NV_SUB_IDS,	"GeForce GTX 650" },
	// 0FD0 - 0FDF
	{ 0x10DE0FD1,	NV_SUB_IDS,	"GeForce GT 650M" },
	{ 0x10DE0FD2,	NV_SUB_IDS,	"GeForce GT 640M" },
	{ 0x10DE0FD3,	NV_SUB_IDS,	"GeForce GT 640M LE" },
	{ 0x10DE0FD4,	NV_SUB_IDS,	"GeForce GTX 660M" },
	{ 0x10DE0FD5,	NV_SUB_IDS,	"GeForce GT 650M" },  //GK100
	{ 0x10DE0FDB,	NV_SUB_IDS,	"GK107-ESP-A1" },
	// 0FE0 - 0FEF
	{ 0x10DE0FE0,	NV_SUB_IDS,	"GeForce GTX 660M" },
	// 0FF0 - 0FFF
	{ 0x10DE0FFB,	NV_SUB_IDS,	"Quadro K2000M" },
	{ 0x10DE0FFC,	NV_SUB_IDS,	"Quadro K1000M" },
	{ 0x10DE0FFD,	NV_SUB_IDS,	"NVS 510" },
	{ 0x10DE0FFF,	NV_SUB_IDS,	"Quadro 410" },
	// 1000 - 100F
	// 1010 - 101F
	// 1020 - 102F
	// 1030 - 103F
	// 1040 - 104F
	{ 0x10DE1040,	NV_SUB_IDS,	"GeForce GT 520" },
	//  { 0x10DE1041,	NV_SUB_IDS,	"D13M1-45" },
	{ 0x10DE1042,	NV_SUB_IDS,	"GeForce 510" },
	{ 0x10DE1048,	NV_SUB_IDS,	"GeForce 605" },
	{ 0x10DE1049,	NV_SUB_IDS,	"GeForce GT 620" },
	{ 0x10DE104A,	NV_SUB_IDS,	"GeForce GT 610" },
	// 1050 - 105F
	{ 0x10DE1050,	NV_SUB_IDS,	"GeForce GT 520M" },
	{ 0x10DE1051,	NV_SUB_IDS,	"GeForce GT 520MX" },
	{ 0x10DE1052,	NV_SUB_IDS,	"GeForce GT 520M" },
	{ 0x10DE1054,	NV_SUB_IDS,	"GeForce GT 410M" },
	{ 0x10DE1055,	NV_SUB_IDS,	"GeForce 410M" },
	{ 0x10DE1056,	NV_SUB_IDS,	"Quadro NVS 4200M" },
	{ 0x10DE1057,	NV_SUB_IDS,	"Quadro NVS 4200M" },
	{ 0x10DE1058,	NV_SUB_IDS,	"GeForce 610M" },
	{ 0x10DE1059,	NV_SUB_IDS,	"GeForce 610M" },
	{ 0x10DE105A,	NV_SUB_IDS,	"GeForce 610M" },
	// 1060 - 106F
	// 1070 - 107F
	{ 0x10DE107D,	NV_SUB_IDS,	"NVS 310" },
	//  { 0x10DE107E,	NV_SUB_IDS,	"GF119-INT" },
	//  { 0x10DE107F,	NV_SUB_IDS,	"GF119-ES" },
	// 1080 - 108F
	{ 0x10DE1080,	NV_SUB_IDS,	"GeForce GTX 580" },
	{ 0x10DE1081,	NV_SUB_IDS,	"GeForce GTX 570" },
	{ 0x10DE1082,	NV_SUB_IDS,	"GeForce GTX 560 Ti" },
	{ 0x10DE1083,	NV_SUB_IDS,	"D13U" },
	{ 0x10DE1084,	NV_SUB_IDS,	"GeForce GTX 560" },
	{ 0x10DE1086,	NV_SUB_IDS,	"GeForce GTX 570" },
	{ 0x10DE1087,	NV_SUB_IDS,	"GeForce GTX 560 Ti-448" },
	{ 0x10DE1088,	NV_SUB_IDS,	"GeForce GTX 590" },
	{ 0x10DE1089,	NV_SUB_IDS,	"GeForce GTX 580" },
	{ 0x10DE108B,	NV_SUB_IDS,	"GeForce GTX 590" },
	//  { 0x10DE108C,	NV_SUB_IDS,	"D13U" },
	{ 0x10DE108E,	NV_SUB_IDS,	"Tesla C2090" },
	// 1090 - 109F
	{ 0x10DE1091,	NV_SUB_IDS,	"nVidia Tesla M2090" },
	{ 0x10DE1094,	NV_SUB_IDS,	"Tesla M2075 Dual-Slot Computing Processor Module" },
	{ 0x10DE1096,	NV_SUB_IDS,	"Tesla C2075" },
	{ 0x10DE1098,	NV_SUB_IDS,	"D13U" },
	{ 0x10DE109A,	NV_SUB_IDS,	"Quadro 5010M" },
	{ 0x10DE109B,	NV_SUB_IDS,	"Quadro 7000" },
	// 10A0 - 10AF
	// 10B0 - 10BF
	// 10C0 - 10CF
	{ 0x10DE10C0,	NV_SUB_IDS,	"GeForce 9300 GS" },
	{ 0x10DE10C3,	NV_SUB_IDS,	"GeForce 8400 GS" },
	{ 0x10DE10C4,	NV_SUB_IDS,	"NVIDIA ION" },
	{ 0x10DE10C5,	NV_SUB_IDS,	"GeForce 405" },
	// 10D0 - 10DF
	{ 0x10DE10D8,	NV_SUB_IDS,	"NVS 300" },
	// 10E0 - 10EF
	// 10F0 - 10FF
	// 1100 - 110F
	// 1110 - 111F
	// 1120 - 112F
	// 1130 - 113F
	// 1140 - 114F
	{ 0x10DE1140,	NV_SUB_IDS,	"GeForce GT 610M" },
	{ 0x10DE1141,	NV_SUB_IDS,	"GeForce 610M" },
	{ 0x10DE1142,	NV_SUB_IDS,	"GeForce 620M" },
	//  { 0x10DE1143,	NV_SUB_IDS,	"N13P-GV" },
	//  { 0x10DE1144,	NV_SUB_IDS,	"GF117" },
	//  { 0x10DE1145,	NV_SUB_IDS,	"GF117" },
	//  { 0x10DE1146,	NV_SUB_IDS,	"GF117" },
	//  { 0x10DE1147,	NV_SUB_IDS,	"GF117" },
	//  { 0x10DE1149,	NV_SUB_IDS,	"GF117-ES" },
	//  { 0x10DE114A,	NV_SUB_IDS,	"GF117-INT" },
	//  { 0x10DE114B,	NV_SUB_IDS,	"PCI-GEN3-B" },
	// 1150 - 115F
	// 1160 - 116F
	// 1170 - 117F
	// 1180 - 118F
	{ 0x10DE1180,	NV_SUB_IDS,	"GeForce GTX 680" },
	{ 0x10DE1183,	NV_SUB_IDS,	"GeForce GTX 660 Ti" },
  { 0x10DE1184,	NV_SUB_IDS,	"GeForce GTX 770" },
	{ 0x10DE1185,	NV_SUB_IDS,	"GeForce GTX 660" },
	{ 0x10DE1188,	NV_SUB_IDS,	"GeForce GTX 690" },
	{ 0x10DE1189,	NV_SUB_IDS,	"GeForce GTX 670" },
  // 118A - 118E
  { 0x10DE118F,	NV_SUB_IDS,	"Tesla K10" },
	// 1190 - 119F
	// 11A0 - 11AF
	{ 0x10DE11A0,	NV_SUB_IDS,	"GeForce GTX 680M" }, //optimus
	{ 0x10DE11A1,	NV_SUB_IDS,	"GeForce GTX 670MX" },
	{ 0x10DE11A7,	NV_SUB_IDS,	"GeForce GTX 675MX" },
	// 11B0 - 11BF
	{ 0x10DE11BA,	NV_SUB_IDS,	"Quadro K5000" },
	{ 0x10DE11BC,	NV_SUB_IDS,	"Quadro K5000M" },
	{ 0x10DE11BD,	NV_SUB_IDS,	"Quadro K4000M" },
	{ 0x10DE11BE,	NV_SUB_IDS,	"Quadro K3000M" },
	// 11C0 - 11CF
  { 0x10DE11C0,	NV_SUB_IDS,	"GeForce GTX 660" },
	{ 0x10DE11C6,	NV_SUB_IDS,	"GeForce GTX 650 TI" },
  { 0x10DE11D0, NV_SUB_IDS,	"GK106-INT353" },
	// 11D0 - 11DF
	// 11E0 - 11EF
	// 11F0 - 11FF
	{ 0x10DE11FA,	NV_SUB_IDS,	"Quadro K4000" },
	// 1200 - 120F
	{ 0x10DE1200,	NV_SUB_IDS,	"GeForce GTX 560 Ti" },
	{ 0x10DE1201,	NV_SUB_IDS,	"GeForce GTX 560" },
	{ 0x10DE1202,	NV_SUB_IDS,	"GeForce GTX 560 Ti" },
	{ 0x10DE1203,	NV_SUB_IDS,	"GeForce GTX 460 SE v2" },
	{ 0x10DE1205,	NV_SUB_IDS,	"GeForce GTX 460 v2" },
	{ 0x10DE1206,	NV_SUB_IDS,	"GeForce GTX 555" },
	{ 0x10DE1207,	NV_SUB_IDS,	"GeForce GT 645" },
	{ 0x10DE1208,	NV_SUB_IDS,	"GeForce GTX 560 SE" },
	{ 0x10DE1210,	NV_SUB_IDS,	"GeForce GTX 570M" },
	{ 0x10DE1211,	NV_SUB_IDS,	"GeForce GTX 580M" },
	{ 0x10DE1212,	NV_SUB_IDS,	"GeForce GTX 675M" },
	{ 0x10DE1213,	NV_SUB_IDS,	"GeForce GTX 670M" },
	{ 0x10DE1240,	NV_SUB_IDS,	"GeForce GT 620M" },
	{ 0x10DE1241,	NV_SUB_IDS,	"GeForce GT 545" },
	{ 0x10DE1243,	NV_SUB_IDS,	"GeForce GT 545" },
	{ 0x10DE1244,	NV_SUB_IDS,	"GeForce GTX 550 Ti" },
	{ 0x10DE1245,	NV_SUB_IDS,	"GeForce GTS 450" },
	{ 0x10DE1246,	NV_SUB_IDS,	"GeForce GTX 550M" },
	{ 0x10DE1247,	NV_SUB_IDS,	"GeForce GT 555M" },
	{ 0x10DE1248,	NV_SUB_IDS,	"GeForce GTX 555M" },
	{ 0x10DE1249,	NV_SUB_IDS,	"GeForce GTS 450" }, // 450M?
	{ 0x10DE124B,	NV_SUB_IDS,	"GeForce GT 640" },
	{ 0x10DE124D,	NV_SUB_IDS,	"GeForce GTX 555M" },
	//  { 0x10DE1250,	NV_SUB_IDS,	"GF116-INT" },
	{ 0x10DE1251,	NV_SUB_IDS,	"GeForce GTX 560M" },
	// 1260 - 126F
	// 1270 - 127F
	// 1280 - 128F
  { 0x10DE1280,	NV_SUB_IDS,	"GeForce GT 635" },
  { 0x10DE1282,	NV_SUB_IDS,	"GeForce GT 640" },
  { 0x10DE1284,	NV_SUB_IDS,	"GeForce GT 630" },
	// 1290 - 129F
	// 12A0 - 12AF
	// 12B0 - 12BF
	// 12C0 - 12CF
	// 12D0 - 12DF
	// 12E0 - 12EF
	// 12F0 - 12FF
};

/*static UINT16 swap16(UINT16 x)
{
	return (((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8));
}*/

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

EFI_STATUS read_nVidia_PRAMIN(pci_dt_t *nvda_dev, VOID* rom, UINT16 arch)
{
	EFI_STATUS Status;
	EFI_PCI_IO_PROTOCOL		*PciIo;
	PCI_TYPE00			Pci;
  
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
	EFI_STATUS Status;
	EFI_PCI_IO_PROTOCOL		*PciIo;
	PCI_TYPE00            Pci;
  UINT32                value;

  DBG("PROM\n");
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
  UINT8 num_outputs = 0, i = 0;
  UINT8 dcbtable_version;
  UINT8 headerlength	 = 0;
  UINT8 numentries		 = 0;
  UINT8 recordlength	 = 0;
  UINT8 channel1 = 0, channel2 = 0;
  UINT16 dcbptr;
  UINT8 *dcbtable;
  UINT8 *togroup;
  INT32 has_lvds = FALSE;
  struct dcbentry {
	 UINT8 type;
	 UINT8 index;
	 UINT8 *heads;
  }entries[MAX_NUM_DCB_ENTRIES];

	DBG("patch_nvidia_rom\n");
	if (!rom || (rom[0] != 0x55 && rom[1] != 0xaa)) {
		DBG("FALSE ROM signature: 0x%02x%02x\n", rom[0], rom[1]);
		return PATCH_ROM_FAILED;
	}

	dcbptr = SwapBytes16(read16(rom, 0x36));
	if(!dcbptr) {
		DBG("no dcb table found\n");
		return PATCH_ROM_FAILED;
	}
//	else
//		DBG("dcb table at offset 0x%04x\n", dcbptr);

	dcbtable		= &rom[dcbptr];
	dcbtable_version	= dcbtable[0];
	
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
	
	if (numentries >= MAX_NUM_DCB_ENTRIES){
		numentries = MAX_NUM_DCB_ENTRIES;
	}

	for (i = 0; i < numentries; i++)
	{
		UINT32 connection;
		connection = *(UINT32 *)&dcbtable[headerlength + recordlength * i];

		/* Should we allow discontinuous DCBs? Certainly DCB I2C tables can be discontinuous */
		if ((connection & 0x0000000f) == 0x0000000f) /* end of records */ 
		{
			continue;
		}
		if (connection == 0x00000000) /* seen on an NV11 with DCB v1.5 */ 
		{
			continue;
		}
		if ((connection & 0xf) == 0x6) /* we skip type 6 as it doesnt appear on macbook nvcaps */
		{
			continue;
		}

		entries[num_outputs].type = connection & 0xf;
		entries[num_outputs].index = num_outputs;
		entries[num_outputs++].heads = (UINT8*)&(dcbtable[(headerlength + recordlength * i) + 1]);
	}
	
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
			{
				continue;
			}

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
				{
					continue;
				}
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

							if (entries[i-1].type == 0x0)
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

							if (entries[i - 1].type == 0x0)
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
						default:
							break;

					}
					break;
				}
			}
		}
	}
	
	// if we have left ungrouped outputs merge them to the empty channel
	togroup = &channel2;// = (channel1 ? (channel2 ? NULL : &channel2) : &channel1);

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

CHAR8 *get_nvidia_model(UINT32 device_id, UINT32 subsys_id)
{
//	DBG("get_nvidia_model\n");
	INT32 i;
  
  // First check in the plist, (for e.g this can override any hardcoded devices)
	CARDLIST * nvcard = FindCardWithIds(device_id, subsys_id);
	if (nvcard) 
	{
		if (nvcard->Model) 
		{
			return nvcard->Model;
		}
	}
	
	// Then check the harcoded table

	for (i = 1; i < (sizeof(nvidia_cards) / sizeof(nvidia_cards[0])); i++)
	{
		if ((nvidia_cards[i].device == device_id) && (nvidia_cards[i].subdev == subsys_id))
		{
			return nvidia_cards[i].name_model;
			break;
		}
		else if ((nvidia_cards[i].device == device_id) && (nvidia_cards[i].subdev == 0x00000000))
		{
			return nvidia_cards[i].name_model;
			break;
		}
	}
	return nvidia_cards[0].name_model;
}

static INT32 devprop_add_nvidia_template(DevPropDevice *device)
{
	CHAR8 tmp[16];
	DBG("devprop_add_nvidia_template\n");

	if (!device)
	{
		return 0;
	}
	if (!DP_ADD_TEMP_VAL(device, nvidia_compatible_0))
	{
		return 0;
	}
	if (!DP_ADD_TEMP_VAL(device, nvidia_device_type_0))
	{
		return 0;
	}
	if (!DP_ADD_TEMP_VAL(device, nvidia_name_0))
	{
		return 0;
	}
	if (!DP_ADD_TEMP_VAL(device, nvidia_compatible_1))
	{
		return 0;
	}
	if (!DP_ADD_TEMP_VAL(device, nvidia_device_type_1))
	{
		return 0;
	}
	if (!DP_ADD_TEMP_VAL(device, nvidia_name_1))
	{
		return 0;
	}
	if (devices_number == 1)
	{
		if (!DP_ADD_TEMP_VAL(device, nvidia_device_type_parent))
		{
			return 0;
		}
	}
	else
	{
		if (!DP_ADD_TEMP_VAL(device, nvidia_device_type_child))
		{
			return 0;
		}
	}

	AsciiSPrint(tmp, 16, "Slot-%x",devices_number);
	devprop_add_value(device, "AAPL,slot-name", (UINT8 *) tmp, (UINT32)AsciiStrLen(tmp));
	devices_number++;

	return 1;
}

UINT8 hexstrtouint8 (CHAR8* buf)
{
	INT8 i;
	if (IS_DIGIT(buf[0]))
		i = buf[0]-'0';
	else if (IS_HEX(buf[0])) 
		i = buf[0]-'a' + 10; 
	else
		i = buf[0]-'A' + 10;
	if (AsciiStrLen(buf) == 1) {
		return i;
	}
	i <<= 4;
	if (IS_DIGIT(buf[1]))
		i += buf[1]-'0';
	else if (IS_HEX(buf[1])) 
		i += buf[1]-'a' + 10; 
	else
		i += buf[1]-'A'+ 10; //no error checking
	return i;
}

BOOLEAN IsHexDigit (CHAR8 c) {
	return (IS_DIGIT(c) || (c>='A'&&c<='F') || (c>='a'&&c<='f'))?TRUE:FALSE;
}


UINT32 hex2bin(IN CHAR8 *hex, OUT UINT8 *bin, UINT32 len) //assume len = number of UINT8 values
{
	CHAR8	*p;
	UINT32	i, outlen = 0;
	CHAR8	buf[3];

	if (hex == NULL || bin == NULL || len <= 0 || AsciiStrLen(hex) < len * 2) {
//		DBG("[ERROR] bin2hex input error\n"); //this is not error, this is empty value
		return FALSE;
	}

	buf[2] = '\0';
	p = (CHAR8 *) hex;

	for (i = 0; i < len; i++)
	{
		while ((*p == 0x20) || (*p == ',')) {
			p++; //skip spaces and commas
		}
		if (*p == 0) {
			break;
		}
		if (!IsHexDigit(p[0]) || !IsHexDigit(p[1])) {
			DBG("[ERROR] bin2hex '%a' syntax error\n", hex);
			return 0;
		}
		buf[0] = *p++;
		buf[1] = *p++;
		bin[i] = hexstrtouint8(buf);
		outlen++;
	}
	bin[outlen] = 0;
	return outlen;
}

UINT64 mem_detect(UINT16 nvCardType, pci_dt_t *nvda_dev)
{
	
	UINT64 vram_size = 0;
  
  // First check if any value exist in the plist
	CARDLIST * nvcard = FindCardWithIds(((nvda_dev->vendor_id << 16) | nvda_dev->device_id),((nvda_dev->subsys_id.subsys.vendor_id << 16) | nvda_dev->subsys_id.subsys.device_id));
	if (nvcard) 
	{
		if (nvcard->VideoRam > 0) 
		{
         // VideoRam * 1024 * 1024 == VideoRam << 20
			vram_size = LShiftU64(nvcard->VideoRam, 20);
			DBG("mem_detected %ld\n", vram_size);
			return vram_size;
		}
	}

	if (nvCardType < NV_ARCH_50)
	{
		vram_size  = (UINT64)(REG32(nvda_dev->regs, NV04_PFB_FIFO_DATA));
		vram_size &= NV10_PFB_FIFO_DATA_RAM_AMOUNT_MB_MASK;
	}
	else if (nvCardType < NV_ARCH_C0)
	{
		vram_size = (UINT64)(REG32(nvda_dev->regs, NV04_PFB_FIFO_DATA));
		vram_size |= LShiftU64(vram_size & 0xff, 32);
		vram_size &= 0xffffffff00ll;
	}
	else // >= NV_ARCH_C0
	{
		vram_size = LShiftU64(REG32(nvda_dev->regs, NVC0_MEM_CTRLR_RAM_AMOUNT), 20);
//		vram_size *= REG32(nvda_dev->regs, NVC0_MEM_CTRLR_COUNT);
    vram_size = MultU64x32(vram_size, REG32(nvda_dev->regs, NVC0_MEM_CTRLR_COUNT));
	}

	// Then, Workaround for 9600M GT, GT 210/420/430/440/525M/540M & GTX 560M
	switch (nvda_dev->device_id)
	{
		case 0x0647: // 9600M GT 0647
			vram_size = 512*1024*1024;
			break;
		case 0x0649:	// 9600M GT 0649
			// 10DE06491043202D 1GB VRAM
			if (((nvda_dev->subsys_id.subsys.vendor_id << 16) | nvda_dev->subsys_id.subsys.device_id) == 0x1043202D )
			{
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
	DBG("mem_detected %ld\n", vram_size);
	return vram_size;
}



BOOLEAN setup_nvidia_devprop(pci_dt_t *nvda_dev)
{
	const				INT32 MAX_BIOS_VERSION_LENGTH = 32;
	EFI_STATUS			Status = EFI_NOT_FOUND;
	DevPropDevice			*device = NULL;
	CHAR8				*devicepath = NULL;
	BOOLEAN				load_vbios = gSettings.LoadVBios;
  BOOLEAN				Injected;
	UINT8				*rom = NULL;
	UINT16				nvCardType = 0;
	UINT64				videoRam = 0;
	UINT32				bar[7];
	UINT32				boot_display = 0;
	INT32				nvPatch = 0;
	CHAR8				*model = NULL;
	CHAR16				FileName[64];
	UINT8				*buffer;
	UINTN				bufferLen;
	UINTN				j, n_ports = 0;
	INT32				i, version_start;
	INT32				crlf_count = 0;
	option_rom_pci_header_t		*rom_pci_header;
	CHAR8*				s;
	CHAR8*				s1;
	CHAR8*				version_str = (CHAR8*)AllocateZeroPool(MAX_BIOS_VERSION_LENGTH);
//	DBG("setup_nvidia_devprop\n");
//	DBG("%x:%x\n",nvda_dev->vendor_id, nvda_dev->device_id);
	devicepath = get_pci_dev_path(nvda_dev);
	bar[0] = pci_config_read32(nvda_dev, PCI_BASE_ADDRESS_0);
	nvda_dev->regs = (UINT8 *)(UINTN)(bar[0] & ~0x0f);
//	DBG("BAR: 0x%x\n", nvda_dev->regs);

//	gBS->Stall(50);

	// get card type
	nvCardType = (REG32(nvda_dev->regs, 0) >> 20) & 0x1ff;

	// Amount of VRAM in kilobytes (?) no, it is already in bytes!!!
	if (gSettings.VRAM != 0) {
		videoRam = gSettings.VRAM;
	} else {
		videoRam = mem_detect(nvCardType, nvda_dev);
    gSettings.VRAM = videoRam;
	}

/*	model = get_nvidia_model(((nvda_dev->vendor_id << 16) | nvda_dev->device_id),((nvda_dev->subsys_id.subsys.vendor_id << 16) | nvda_dev->subsys_id.subsys.device_id));
*/
	for (j = 0; j < NGFX; j++) {    
		if ((gGraphics[j].Vendor == Nvidia) && (gGraphics[j].DeviceID == nvda_dev->device_id)) {
			model = gGraphics[j].Model; //double?
			n_ports = gGraphics[j].Ports;
			load_vbios = gGraphics[j].LoadVBios;
			break;
		}
	}

	DBG("nVidia %a ", model);
	DBG(" %dMB NV%02x [%04x:%04x] :: ", (UINT32)(RShiftU64(videoRam, 20)),
      nvCardType, nvda_dev->vendor_id, nvda_dev->device_id);

	if (load_vbios){
		UnicodeSPrint(FileName, 128, L"ROM\\10de_%04x.rom", nvda_dev->device_id);
	if (FileExists(OEMDir, FileName)){
		Status = egLoadFile(OEMDir, FileName, &buffer, &bufferLen);
	}
	if (EFI_ERROR(Status)) {
		UnicodeSPrint(FileName, 128, L"\\EFI\\CLOVER\\ROM\\10de_%04x.rom", nvda_dev->device_id);
		if (FileExists(SelfRootDir, FileName)){
			Status = egLoadFile(SelfRootDir, FileName, &buffer, &bufferLen);
		}
	}
  }
  if (EFI_ERROR(Status)){
    rom = AllocateZeroPool(NVIDIA_ROM_SIZE+1);
		// PRAMIN first
    read_nVidia_PRAMIN(nvda_dev, rom, nvCardType);

    //DBG("got here\n");
		
    //DBG("%x%x\n", rom[0], rom[1]);
    rom_pci_header = NULL;

	if (rom[0] != 0x55 || rom[1] != 0xaa) {
		read_nVidia_PROM(nvda_dev, rom);
		if (rom[0] != 0x55 || rom[1] != 0xaa)
			DBG("ERROR: Unable to locate nVidia Video BIOS\n");
    }

	if (rom[0] == 0x55 && rom[1] == 0xaa) {
		if ((nvPatch = patch_nvidia_rom(rom)) == PATCH_ROM_FAILED) {
			DBG("ERROR: nVidia ROM Patching Failed!\n");
		}
	} else {
		DBG("using loaded ROM image\n");
	}


	rom_pci_header = (option_rom_pci_header_t*)(rom + *(UINT16 *)&rom[24]);

	// check for 'PCIR' sig
	if (rom_pci_header->signature == 0x52494350/*50434952*/) { //for some reason, the reverse byte order
		if (rom_pci_header->device_id != nvda_dev->device_id) {
			// Get Model from the OpROM
			model = get_nvidia_model(((rom_pci_header->vendor_id << 16) | rom_pci_header->device_id), NV_SUB_IDS);
//				DBG(model);
		}
	}
	else
	{
		DBG("nVidia incorrect PCI ROM signature: 0x%x\n", rom_pci_header->signature);
	}

	// get bios version

	//ZeroMem((VOID*)version_str, MAX_BIOS_VERSION_LENGTH);

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
						s = (CHAR8*)(rom + version_start);
            s1 = version_str;
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
	//device = devprop_add_device(string, devicepath);
	device = devprop_add_device_pci(string, nvda_dev);
	devprop_add_nvidia_template(device);

  for (i = 0; i < gSettings.NrAddProperties; i++) {
    if (gSettings.AddProperties[i].Device != DEV_NVIDIA) {
      continue;
    }
    Injected = TRUE;
    devprop_add_value(device,
                      gSettings.AddProperties[i].Key,
                      (UINT8*)gSettings.AddProperties[i].Value,
                      gSettings.AddProperties[i].ValueLen);
  }
  if (Injected) {
    DBG("custom NVIDIA properties injected, continue\n");
    //    return TRUE;
  }
  
  if (gSettings.FakeNVidia) {
    UINT32 FakeID = gSettings.FakeNVidia >> 16;
    devprop_add_value(device, "device-id", (UINT8*)&FakeID, 4);
    FakeID = gSettings.FakeNVidia & 0xFFFF;
    devprop_add_value(device, "vendor-id", (UINT8*)&FakeID, 4);
  }
    
  if (gSettings.NoDefaultProperties) {
    DBG("NVidia: no default properties\n");
    return TRUE;
  }
  
	/* FIXME: for primary graphics card only */
	boot_display = 1;
	devprop_add_value(device, "@0,AAPL,boot-display", (UINT8*)&boot_display, 4);
  devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 9);
	if (nvPatch == PATCH_ROM_SUCCESS_HAS_LVDS) {
		UINT8 built_in = 0x01;
		devprop_add_value(device, "@0,built-in", &built_in, 1);
	}

	//AsciiSPrint(biosVersion, 32, "%a", version_str);
	//AsciiSPrint(kNVCAP, 12, "NVCAP_%04x", nvda_dev->device_id);

	if ((gSettings.NVCAP[0] != 0)) {
		devprop_add_value(device, "NVCAP", &gSettings.NVCAP[0], NVCAP_LEN);
    DBG("default_NVCAP: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x\n",
        gSettings.NVCAP[0], gSettings.NVCAP[1], gSettings.NVCAP[2], gSettings.NVCAP[3],
        gSettings.NVCAP[4], gSettings.NVCAP[5], gSettings.NVCAP[6], gSettings.NVCAP[7],
        gSettings.NVCAP[8], gSettings.NVCAP[9], gSettings.NVCAP[10], gSettings.NVCAP[11],
        gSettings.NVCAP[12], gSettings.NVCAP[13], gSettings.NVCAP[14], gSettings.NVCAP[15],
        gSettings.NVCAP[16], gSettings.NVCAP[17], gSettings.NVCAP[18], gSettings.NVCAP[19]);    
	} else {
		devprop_add_value(device, "NVCAP", default_NVCAP, NVCAP_LEN);
    DBG("default_NVCAP: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x\n",
        default_NVCAP[0], default_NVCAP[1], default_NVCAP[2], default_NVCAP[3],
        default_NVCAP[4], default_NVCAP[5], default_NVCAP[6], default_NVCAP[7],
        default_NVCAP[8], default_NVCAP[9], default_NVCAP[10], default_NVCAP[11],
        default_NVCAP[12], default_NVCAP[13], default_NVCAP[14], default_NVCAP[15],
        default_NVCAP[16], default_NVCAP[17], default_NVCAP[18], default_NVCAP[19]);    
	}
	devprop_add_value(device, "NVPM", default_NVPM, NVPM_LEN);
	if ((gSettings.VRAM != 0)) {
		devprop_add_value(device, "VRAM,totalsize", (UINT8*)&gSettings.VRAM, 4);
	} else {
		devprop_add_value(device, "VRAM,totalsize", (UINT8*)&videoRam, 4);
	}
	if (gSettings.InjectEDID) {
		devprop_add_value(device, "AAPL00,override-no-connect", gSettings.CustomEDID, 128);
	}
	devprop_add_value(device, "model", (UINT8*)model, (UINT32)AsciiStrLen(model));
	devprop_add_value(device, "rom-revision", (UINT8*)version_str, (UINT32)AsciiStrLen(version_str));
	if ((gSettings.Dcfg[0] != 0) && (gSettings.Dcfg[1] != 0)) {
		devprop_add_value(device, "@0,display-cfg", &gSettings.Dcfg[0], DCFG0_LEN);
		devprop_add_value(device, "@1,display-cfg", &gSettings.Dcfg[4], DCFG1_LEN);
	} else {
		devprop_add_value(device, "@0,display-cfg", default_dcfg_0, DCFG0_LEN);
		devprop_add_value(device, "@1,display-cfg", default_dcfg_1, DCFG1_LEN);
	}
  devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 9);
  

	//add HDMI Audio back to nvidia
	//http://forge.voodooprojects.org/p/chameleon/issues/67/
//	UINT8 connector_type_1[]= {0x00, 0x08, 0x00, 0x00};
//	devprop_add_value(device, "@1,connector-type",connector_type_1, 4);
	//end Nvidia HDMI Audio


//	gDeviceProperties = (VOID*)devprop_generate_string(string);
//	gBS->Stall(2000000);
  FreePool(version_str);
	return TRUE;
}
