/*
 *  nviviaresolution.h
 *  
 *
 *  Created by Le Bidou on 19/03/10.
 *  Copyright 2010 ---. All rights reserved.
 *
 */

#ifndef _NVDA_RESOLUTION_HEADER_
#define _NVDA_RESOLUTION_HEADER_

#include "libsaio.h"
#include "autoresolution.h"

#define NVIDIA_SIGNATURE "NVIDIA Corp"

#define OFFSET_TO_VESA_TABLE_INDEX 2
#define MAIN_VESA_TABLE 0
#define SECOND_VESA_TABLE 1

typedef struct {
	unsigned char	ucTable_Major; //These names are probably wrong
	unsigned char	ucTable_Minor;
	unsigned char	ucTable_Rev;
	unsigned short	usTable_Size;
} NV_COMMON_TABLE_HEADER;

typedef struct {
	unsigned short	usPixel_Clock;
	unsigned short	usH_Active;
	unsigned short  usH_Active_minus_One;
	unsigned short	reserved1;
	unsigned short  usH_Active_minus_One_;
	unsigned short	usH_SyncStart;
	unsigned short	usH_SyncEnd;
	unsigned short	usH_Total;
	unsigned short	usV_Active;
	unsigned short  usV_Active_minus_One;
	unsigned short	reserved2;
	unsigned short  usV_Active_minus_One_;
	unsigned short	usV_SyncStart;
	unsigned short	usV_SyncEnd;
	unsigned short	usV_Total;
	unsigned short	reserved3;
} NV_MODELINE;

typedef struct {
	unsigned short h_disp;
	unsigned short v_disp;
	unsigned char  h_blank;
	unsigned char  h_syncoffset;
	unsigned char  h_syncwidth;
	unsigned char  v_blank;
	//unsigned char  v_syncwidth;
	unsigned char  flags; //looks like flags & 1 means "Graphics Mode", to oppose to "Console Mode"
	//on 7xxx the high four bits look like a mode id number.
	//on 8xxx only the low four bits are used, standard graphics mode are always 5.
	//		it can be 1 (1400x1050 and 2048x1536) (HSync High, VSync High ?)
	//				  3 (1440x900, 1680x1050 and 1920x1200) (Hsync High, VSync Low ?)
	//				  3 (Standard Timings) (Hsync Low, VSync High ?)
	//			   or 7 (1280x800 and 768x340) (Hync Low, VSync Low ?)
} NV_MODELINE_2;

typedef struct {
	NV_COMMON_TABLE_HEADER	sHeader;
	NV_MODELINE	*			sModelines;
} NV_VESA_TABLE;

vbios_map * open_nvidia_vbios(vbios_map *map);

bool nvidia_set_mode(vbios_map* map, UInt8 idx, UInt32* x, UInt32* y, char Type);

#endif