
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
#ifndef __915_RESOLUTION_H
#define __915_RESOLUTION_H

#include "edid.h"

#define NEW(a) ((a *)(malloc(sizeof(a))))
#define FREE(a) (free(a))

#ifdef AUTORES_DEBUG
#define PRINT(a, b...) printf(a, ##b);
#else 
#define PRINT(a, b...) verbose(a, ##b);
#endif

#define VBIOS_START         0xc0000
#define VBIOS_SIZE          0x10000

#define FALSE 0
#define TRUE 1

typedef struct {
	unsigned char width;
	unsigned char height;
} s_aspect;


typedef enum {
	CT_UNKWN, CT_830, CT_845G, CT_855GM, CT_865G, 
	CT_915G, CT_915GM, CT_945G, CT_945GM, CT_945GME, CT_946GZ, 
	CT_955X, CT_G965, CT_Q965, CT_965GM, CT_975X, 
	CT_P35, CT_X48, CT_B43, CT_Q45, CT_P45,
	CT_GM45, CT_G41, CT_G31, CT_G45, CT_500
} chipset_type;


typedef enum {
	BT_UNKWN, BT_1, BT_2, BT_3, BT_ATI_1, BT_ATI_2, BT_NVDA
} bios_type;

typedef struct {
		UInt32	clock;
		UInt16	x;
		UInt16	hsyncstart;
		UInt16	hsyncend;
		UInt16	htotal;
		UInt16	y;
		UInt16	vsyncstart;
		UInt16	vsyncend;
		UInt16	vtotal;
	} generic_modeline;


typedef struct {
	UInt32 chipset_id;
	chipset_type chipset;
	bios_type bios;
	
	UInt32 bios_fd;
	unsigned char* bios_backup_ptr;
	unsigned char* bios_ptr;
	
	char * mode_table;
	char * nv_mode_table_2;
	
	UInt32 mode_table_size;
	UInt32 nv_mode_table_2_size;
	
	UInt32 modeline_num;
	UInt32 nv_modeline_num_2;
	
	UInt8 b1, b2;
	
	s_aspect aspect_ratio;
	
	UInt8 unlocked;
} vbios_map;

vbios_map * open_vbios(chipset_type);
void close_vbios (vbios_map*);
void unlock_vbios(vbios_map*);
void relock_vbios(vbios_map*);
void save_vbios(vbios_map*);
void restore_vbios(vbios_map*);

void gtf_timings(UInt32 x, UInt32 y, UInt32 freq,
				 unsigned long *clock,
				 UInt16 *hsyncstart, UInt16 *hsyncend, UInt16 *hblank,
				 UInt16 *vsyncstart, UInt16 *vsyncend, UInt16 *vblank);

void cvt_timings(UInt32 x, UInt32 y, UInt32 freq,
				 unsigned long *clock,
				 UInt16 *hsyncstart, UInt16 *hsyncend, UInt16 *hblank,
				 UInt16 *vsyncstart, UInt16 *vsyncend, UInt16 *vblank, bool reduced);

void patch_vbios(vbios_map*, UInt32, UInt32, UInt32, UInt32, UInt32);

#endif
