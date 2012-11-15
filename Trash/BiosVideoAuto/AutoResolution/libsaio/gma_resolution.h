/*
 *  gma_resolution.h
 *  
 *
 *  Created by Le Bidou on 19/03/10.
 *  Copyright 2010 ---. All rights reserved.
 *
 */

#ifndef _GMA_RESOLUTION_H_
#define _GMA_RESOLTUION_H_

#include "libsaio.h"
#include "autoresolution.h"

#define MODE_TABLE_OFFSET_845G 617
#define INTEL_SIGNATURE "Intel Corp"

typedef struct {
	UInt8 mode;
	UInt8 bits_per_pixel;
	UInt16 resolution;
	UInt8 unknown;
} __attribute__((packed)) vbios_mode;

typedef struct {
	UInt8 unknow1[2];
	UInt8 x1;
	UInt8 x_total;
	UInt8 x2;
	UInt8 y1;
	UInt8 y_total;
	UInt8 y2;
} __attribute__((packed)) vbios_resolution_type1;

typedef struct {
	unsigned long clock;
	
	UInt16 x1;
	UInt16 htotal;
	UInt16 x2;
	UInt16 hblank;
	UInt16 hsyncstart;
	UInt16 hsyncend;
	UInt16 y1;
    UInt16 vtotal;
    UInt16 y2;
	UInt16 vblank;
	UInt16 vsyncstart;
	UInt16 vsyncend;
} __attribute__((packed)) vbios_modeline_type2;

typedef struct {
	UInt8 xchars;
	UInt8 ychars;
	UInt8 unknown[4];
	
	vbios_modeline_type2 modelines[];
} __attribute__((packed)) vbios_resolution_type2;

typedef struct {
	unsigned long clock;
	
	UInt16 x1;
	UInt16 htotal;
	UInt16 x2;
	UInt16 hblank;
	UInt16 hsyncstart;
	UInt16 hsyncend;
	
	UInt16 y1;
	UInt16 vtotal;
	UInt16 y2;
	UInt16 vblank;
	UInt16 vsyncstart;
	UInt16 vsyncend;
	
	UInt16 timing_h;
	UInt16 timing_v;
	
	UInt8 unknown[6];
} __attribute__((packed)) vbios_modeline_type3;

typedef struct {
	unsigned char unknown[6];
	
    vbios_modeline_type3 modelines[];
} __attribute__((packed)) vbios_resolution_type3;


vbios_resolution_type1 * map_type1_resolution(vbios_map * map, UInt16 res);
vbios_resolution_type2 * map_type2_resolution(vbios_map * map, UInt16 res);
vbios_resolution_type3 * map_type3_resolution(vbios_map * map, UInt16 res);

char detect_bios_type(vbios_map * map, char modeline, int entry_size);

vbios_map * open_intel_vbios(vbios_map *);

bool intel_set_mode_1(vbios_map* map, UInt8 idx, UInt32* x, UInt32* y);
bool intel_set_mode_2(vbios_map* map, UInt8 idx, UInt32* x, UInt32* y);
bool intel_set_mode_3(vbios_map* map, UInt8 idx, UInt32* x, UInt32* y);

#endif