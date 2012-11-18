/*
 *  resolution.h
 *  
 *
 *  Created by Evan Lojewski on 3/4/10.
 *  Copyright 2009. All rights reserved.
 *
 */


#ifndef __RESOLUTION_H
#define __RESOLUTION_H

#include "shortatombios.h"
#include "edid.h"

//Slice - moved to edid.h
/*
typedef struct _edid_mode {
	unsigned short pixel_clock;
	unsigned short h_active;
	unsigned short h_blanking;
	unsigned short v_active;
	unsigned short v_blanking;
	unsigned short h_sync_offset;
	unsigned short h_sync_width;
	unsigned short v_sync_offset;
	unsigned short v_sync_width;
} edid_mode;
*/


void patchVideoBios();



/* Copied from 915 resolution created by steve tomljenovic
 *
 * This code is based on the techniques used in :
 *
 *   - 855patch.  Many thanks to Christian Zietz (czietz gmx net)
 *     for demonstrating how to shadow the VBIOS into system RAM
 *     and then modify it.
 *
 *   - 1280patch by Andrew Tipton (andrewtipton null li).
 *
 *   - 855resolution by Alain Poirier
 *
 * This source code is into the public domain.
 */

#define VBIOS_START         0xc0000
#define VBIOS_SIZE          0x10000

#define FALSE 0
#define TRUE 1

#define MODE_TABLE_OFFSET_845G 617


#define ATI_SIGNATURE1 "ATI MOBILITY RADEON"
#define ATI_SIGNATURE2 "ATI Technologies Inc"
#define NVIDIA_SIGNATURE "NVIDIA Corp"
#define INTEL_SIGNATURE "Intel Corp"



/*
 * NVidia Defines and structures
 */

#define OFFSET_TO_VESA_TABLE_INDEX 2

typedef struct {
	unsigned char	ucTable_Major;
	unsigned char	ucTable_Minor;
	unsigned char	ucTable_Rev;
	unsigned short	usTable_Size;
} NV_COMMON_TABLE_HEADER;

typedef struct {
	short reserved1;
	short reserved2;
	short reserved3;
} NV_RESERVED;

typedef struct {
	unsigned short	usPixel_Clock;
	unsigned short	usH_Active;
	NV_RESERVED		reserved1;
	unsigned short	usH_SyncStart;
	unsigned short	usH_SyncEnd;
	unsigned short	usH_Total;
	unsigned short	usV_Active;
	NV_RESERVED		reserved2;
	unsigned short	usV_SyncStart;
	unsigned short	usV_SyncEnd;
	unsigned short	usV_Total;
	unsigned short	reserved3;
} NV_MODELINE;

typedef struct {
	NV_COMMON_TABLE_HEADER	sHeader;
	NV_MODELINE	*			sModelines;
} NV_VESA_TABLE;

/*---*/


typedef enum {
	CT_UNKNOWN, CT_UNKNOWN_INTEL, CT_830, CT_845G, CT_855GM, CT_865G, 
	CT_915G, CT_915GM, CT_945G, CT_945GM, CT_945GME, CT_946GZ, 
	CT_955X, CT_G965, CT_Q965, CT_965GM, CT_975X, 
	CT_P35, CT_X48, CT_B43, CT_Q45, CT_P45,
	CT_GM45, CT_G41, CT_G31, CT_G45, CT_500, CT_3150
} chipset_type;



typedef enum {
	BT_UNKNOWN, BT_1, BT_2, BT_3, BT_ATI_1, BT_ATI_2, BT_NVDA, BT_INTEL
} bios_type;


typedef struct {
	char         *base;
	ATOM_ROM_HEADER  *AtomRomHeader;
	unsigned short         *MasterCommandTables;
	unsigned short         *MasterDataTables;
} bios_tables_t;

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

typedef struct {
	UInt32 chipset_id;
	chipset_type chipset;
	bios_type bios;
	
	bios_tables_t ati_tables;

	
	UInt32 bios_fd;
	char* bios_ptr;
	
	vbios_mode * mode_table;
	char * ati_mode_table;
	char * nv_mode_table;

	UInt32 mode_table_size;
	UInt8 b1, b2;
	
	UInt8 unlocked;
} vbios_map;



vbios_map * open_vbios(chipset_type);
void close_vbios (vbios_map*);
void unlock_vbios(vbios_map*);
void relock_vbios(vbios_map*);
void set_mode(vbios_map*, UInt32, UInt32, UInt32, UInt32, UInt32);

#endif //__RESOLUTION_H
