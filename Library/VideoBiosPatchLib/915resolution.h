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



//#include "shortatombios.h"
//#include <IndustryStandard/AtomBios.h>
//#include "edid.h"

//Slice - moved to edid.h
/*
typedef struct _edid_mode {
	UINT16 pixel_clock;
	UINT16 h_active;
	UINT16 h_blanking;
	UINT16 v_active;
	UINT16 v_blanking;
	UINT16 h_sync_offset;
	UINT16 h_sync_width;
	UINT16 v_sync_offset;
	UINT16 v_sync_width;
} edid_mode;
*/


VOID patchVideoBios();



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



#define MODE_TABLE_OFFSET_845G 617


#define ATI_SIGNATURE1 "ATI MOBILITY RADEON"
#define ATI_SIGNATURE2 "ATI Technologies Inc"
#define NVIDIA_SIGNATURE "NVIDIA Corp"
#define INTEL_SIGNATURE "Intel Corp"

#pragma pack(1)


/*
 * NVidia Defines and structures
 */

#define OFFSET_TO_VESA_TABLE_INDEX 2

typedef struct {
	UINT8	ucTable_Major;
	UINT8	ucTable_Minor;
	UINT8	ucTable_Rev;
	UINT16	usTable_Size;
} NV_COMMON_TABLE_HEADER;

typedef struct {
	INT16 reserved1;
	INT16 reserved2;
	INT16 reserved3;
} NV_RESERVED;

typedef struct {
	UINT16	usPixel_Clock;
	UINT16	usH_Active;
	NV_RESERVED		reserved1;
	UINT16	usH_SyncStart;
	UINT16	usH_SyncEnd;
	UINT16	usH_Total;
	UINT16	usV_Active;
	NV_RESERVED		reserved2;
	UINT16	usV_SyncStart;
	UINT16	usV_SyncEnd;
	UINT16	usV_Total;
	UINT16	reserved3;
} NV_MODELINE;

typedef struct {
	NV_COMMON_TABLE_HEADER	sHeader;
	NV_MODELINE	*			sModelines;
} NV_VESA_TABLE;

typedef struct {
  UINT16 HRes;
  UINT16 VRes;
} TABLE_0;

typedef struct {
  UINT8  Ratio; //0->16:10, 1->4:3, 2->5:4, 3->16:9
  UINT8  Matrix[17];
} TABLE_A;

typedef struct {
  UINT8  Ratio; //0->16:10, 1->4:3, 2->5:4, 3->16:9
  UINT8  Matrix[9];
} TABLE_B;

typedef struct {
  UINT8  Ratio; //0->16:10, 1->4:3, 2->5:4, 3->16:9
  UINT8  Matrix[13];
} TABLE_D;

typedef struct {
  UINT8  Ratio; //0->16:10, 1->4:3, 2->5:4, 3->16:9
  UINT8  Matrix[5];
} TABLE_LIMIT;

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
	CHAR8         *base;
	ATOM_ROM_HEADER  *AtomRomHeader;
	UINT16         *MasterCommandTables;
	UINT16         *MasterDataTables;
} bios_tables_t;

typedef struct {
	UINT8 mode;
	UINT8 bits_per_pixel;
	UINT16 resolution;
	UINT8 unknown;
} vbios_mode;

typedef struct {
	UINT8 unknow1[2];
	UINT8 x1;
	UINT8 x_total;
	UINT8 x2;
	UINT8 y1;
	UINT8 y_total;
	UINT8 y2;
} vbios_resolution_type1;

typedef struct {
	UINT32 clock;
	
	UINT16 x1;
	UINT16 htotal;
	UINT16 x2;
	UINT16 hblank;
	UINT16 hsyncstart;
	UINT16 hsyncend;
	UINT16 y1;
	UINT16 vtotal;
	UINT16 y2;
	UINT16 vblank;
	UINT16 vsyncstart;
	UINT16 vsyncend;
} vbios_modeline_type2;

typedef struct {
	UINT8 xchars;
	UINT8 ychars;
	UINT8 unknown[4];
	
	vbios_modeline_type2 modelines[1];
} vbios_resolution_type2;

typedef struct {
	UINT32 clock;
	
	UINT16 x1;
	UINT16 htotal;
	UINT16 x2;
	UINT16 hblank;
	UINT16 hsyncstart;
	UINT16 hsyncend;
	
	UINT16 y1;
	UINT16 vtotal;
	UINT16 y2;
	UINT16 vblank;
	UINT16 vsyncstart;
	UINT16 vsyncend;
	
	UINT16 timing_h;
	UINT16 timing_v;
	
	UINT8 unknown[6];
} vbios_modeline_type3;

typedef struct {
	UINT8 unknown[6];
	
	vbios_modeline_type3 modelines[1];
} vbios_resolution_type3;

#pragma pack()

typedef struct {
	UINT32 chipset_id;
	chipset_type chipset;
	bios_type bios;
	
	bios_tables_t ati_tables;

	
	UINT32 bios_fd;
	CHAR8* bios_ptr;
	
	vbios_mode * mode_table;
	CHAR8 * ati_mode_table;
	CHAR8 * nv_mode_table;

	UINT32 mode_table_size;
	UINT8 b1, b2;
	
	UINT8 unlocked;
} vbios_map;

vbios_map * open_vbios(chipset_type);
VOID close_vbios (vbios_map*);
VOID unlock_vbios(vbios_map*);
VOID relock_vbios(vbios_map*);
VOID set_mode(vbios_map*, UINT32, UINT32, UINT32, UINT32, UINT32);
UINTN VideoBiosPatchSearchAndReplace (
                                IN  UINT8       *Source,
                                IN  UINTN       SourceSize,
                                IN  UINT8       *Search,
                                IN  UINTN       SearchSize,
                                IN  UINT8       *Replace,
                                IN  INTN        MaxReplaces
                                );

#endif //__RESOLUTION_H
