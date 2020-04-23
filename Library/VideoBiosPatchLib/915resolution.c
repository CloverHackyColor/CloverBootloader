/*
 *  resolution.h
 *
 *  Created by Evan Lojewski on 3/4/10.
 *  Copyright 2009. All rights reserved.
 *
 *  remake for Clover by dmazar and slice 2012
 */
#ifndef _RESOLUTION_H_
#define _RESOLUTION_H_

#include "VideoBiosPatchLibInternal.h"

static const UINT8 nvda_pattern[] = {
  0x44, 0x01, 0x04, 0x00
};

static const CHAR8 nvda_string[] = "NVID";


//more advanced tables from pene
#define RESOLUTIONS_NUMBER 11
static TABLE_0 nvda_res[RESOLUTIONS_NUMBER] = {
  {1280,  720},
  {1280,  800},
  {1360,  768},
  {1400, 1050},
  {1440,  900},
  {1600,  900},
  {1600, 1200},
  {1680, 1050},
  {1920, 1080},
  {1920, 1200},
  {2048, 1536}
};

static UINT8 Sample0[] = 
      {0x34, 0x2d, 0x27, 0x28, 0x90, 0x2b, 0xa0, 0xbf, 0x9c, 0x8f, 0x96, 0xb9, 0x8e, 0x1f, 0x00, 0x00, 0x00};

static TABLE_A nvda_key0[] = {
  {3, {0x16, 0xCB, 0x9F, 0x9F, 0x8F, 0xA7, 0x17, 0xEA, 0xD2, 0xCF, 0xCF, 0xEB, 0x47, 0xE0, 0xC0, 0x00, 0x01}},
  {0, {0x12, 0xCD, 0x9F, 0x9F, 0x91, 0xA9, 0x1A, 0x3A, 0x21, 0x1F, 0x1F, 0x3B, 0x44, 0xFE, 0xC0, 0x00, 0x01}},
  {3, {0x16, 0xB9, 0xA9, 0x9F, 0x8F, 0xB2, 0x16, 0x14, 0x01, 0xFF, 0xCF, 0xEB, 0x46, 0xEA, 0xC0, 0x00, 0x01}},
  {1, {0x12, 0xE6, 0xAE, 0xAE, 0x8A, 0xBB, 0x8E, 0x3D, 0x1B, 0x19, 0x19, 0x3E, 0x0E, 0x00, 0xC0, 0x24, 0x12}},
  {0, {0x12, 0xE9, 0xB3, 0xB3, 0x8D, 0xBF, 0x92, 0xA3, 0x85, 0x83, 0x83, 0xA4, 0x48, 0xFE, 0xC0, 0x00, 0x00}},
  {3, {0x1A, 0xD7, 0xC7, 0xC7, 0x9B, 0xCD, 0x11, 0x9C, 0x86, 0x83, 0x83, 0x9D, 0x4B, 0xFE, 0xC0, 0x00, 0x00}},
  {1, {0x12, 0x03, 0xC7, 0xC7, 0x87, 0xD1, 0x09, 0xE0, 0xB1, 0xAF, 0xAF, 0xE1, 0x04, 0x00, 0x01, 0x24, 0x13}},
  {0, {0x12, 0x15, 0xD1, 0xD1, 0x99, 0xE0, 0x17, 0x3D, 0x1B, 0x19, 0x19, 0x3E, 0x0E, 0x00, 0x01, 0x24, 0x13}},  
  {3, {0x16, 0x0E, 0xEF, 0x9F, 0x8F, 0xFD, 0x02, 0x63, 0x3B, 0x37, 0xCF, 0xEB, 0x40, 0x00, 0xC1, 0x24, 0x02}},
  {0, {0x12, 0x3F, 0xEF, 0xEF, 0x83, 0x01, 0x1B, 0xD8, 0xB1, 0xAF, 0xAF, 0xD9, 0x04, 0x00, 0x41, 0x25, 0x12}},
  {1, {0x12, 0x63, 0xFF, 0xFF, 0x9D, 0x12, 0x0E, 0x34, 0x01, 0x00, 0x00, 0x35, 0x44, 0xE0, 0x41, 0x25, 0x13}}
};

static UINT8 Sample1[] = 
      {0x28, 0x00, 0x19, 0x00, 0x28, 0x18, 0x08, 0x08, 0x05};

static TABLE_B nvda_key1[] = {
  {3, {0x00, 0x05, 0xD0, 0x02, 0xA0, 0x2C, 0x10, 0x07, 0x05}},
  {0, {0x00, 0x05, 0x20, 0x03, 0xA0, 0x32, 0x10, 0x23, 0x05}},
  {3, {0x50, 0x05, 0x00, 0x03, 0xAA, 0x2F, 0x10, 0x07, 0x05}},
  {1, {0x78, 0x05, 0x1A, 0x04, 0xAF, 0x4A, 0x0E, 0x21, 0x05}},
  {0, {0xA0, 0x05, 0x84, 0x03, 0xB4, 0x38, 0x10, 0x24, 0x05}},
  {3, {0x40, 0x06, 0x84, 0x03, 0xC8, 0x38, 0x10, 0x27, 0x05}},
  {1, {0x40, 0x06, 0xB0, 0x04, 0xC8, 0x4A, 0x10, 0x19, 0x05}},
  {0, {0x90, 0x06, 0x1A, 0x04, 0xD2, 0x41, 0x10, 0x25, 0x05}},
  {3, {0x80, 0x07, 0x38, 0x04, 0xF0, 0x42, 0x10, 0x07, 0x05}},
  {0, {0x80, 0x07, 0xB0, 0x04, 0xF0, 0x4B, 0x10, 0x26, 0x05}},
  {1, {0x00, 0x08, 0x00, 0x06, 0x00, 0x60, 0x10, 0x22, 0x05}}  
};

static UINT8 Sample2[] = 
      {0x82, 0x0f, 0x03, 0x01, 0x00, 0x00, 0x08, 0x04, 0x14, 0x00, 0x00, 0x08, 0x17};

static TABLE_D nvda_key2[] = {
  {3, {0x7B, 0x01, 0x03, 0x7B, 0x01, 0x08, 0x01, 0x20, 0x80, 0x02, 0xFF, 0xFF, 0x20}},
  {0, {0x61, 0x01, 0x03, 0x61, 0x01, 0x08, 0x01, 0x20, 0x80, 0x02, 0xFF, 0xFF, 0x20}},
  {3, {0x4D, 0x01, 0x03, 0x4D, 0x01, 0x08, 0x01, 0x20, 0xA8, 0x02, 0xFF, 0xFF, 0x20}},
  {1, {0x49, 0x01, 0x03, 0x49, 0x01, 0x08, 0x01, 0x20, 0xBC, 0x02, 0xFF, 0xFF, 0x20}},
  {0, {0x65, 0x01, 0x03, 0x65, 0x01, 0x08, 0x01, 0x20, 0xD0, 0x02, 0xFF, 0xFF, 0x20}},
  {3, {0x67, 0x01, 0x03, 0x67, 0x01, 0x08, 0x01, 0x20, 0x20, 0x03, 0xFF, 0xFF, 0x20}},
  {1, {0x4A, 0x01, 0x03, 0x4A, 0x01, 0x08, 0x01, 0x20, 0x20, 0x03, 0xFF, 0xFF, 0x20}},
  {0, {0x69, 0x01, 0x03, 0x69, 0x01, 0x08, 0x01, 0x20, 0x48, 0x03, 0xFF, 0xFF, 0x20}},
  {3, {0x4D, 0x01, 0x03, 0x4D, 0x01, 0x08, 0x01, 0x20, 0xC0, 0x03, 0xFF, 0xFF, 0x20}},
  {0, {0x7D, 0x01, 0x03, 0x7D, 0x01, 0x08, 0x01, 0x20, 0xC0, 0x03, 0xFF, 0xFF, 0x20}},
  {1, {0x7A, 0x01, 0x03, 0x52, 0x01, 0x08, 0x01, 0x20, 0x00, 0x04, 0xFF, 0xFF, 0x20}}    
};

static UINT8 Sample3[] = {0x40, 0x06, 0xba, 0xb0, 0x04};

static TABLE_LIMIT nvda_key3[] = {
  {3, {0x00, 0x05, 0xBA, 0xD0, 0x02}},
  {0, {0x00, 0x05, 0xBA, 0x20, 0x03}},
  {3, {0x50, 0x05, 0xBA, 0x00, 0x03}},
  {1, {0x78, 0x05, 0xBA, 0x1A, 0x04}},
  {0, {0xA0, 0x05, 0xBA, 0x84, 0x03}},
  {3, {0x40, 0x06, 0xBA, 0x84, 0x03}},
  {1, {0x40, 0x06, 0xBA, 0xB0, 0x04}},
  {0, {0x90, 0x06, 0xBA, 0x1A, 0x04}},
  {3, {0x80, 0x07, 0xBA, 0x38, 0x04}},
  {0, {0x80, 0x07, 0xBA, 0xB0, 0x04}},
  {1, {0x00, 0x08, 0xBA, 0x00, 0x06}}  
  
};

//DTD string to replace in Intel BIOSes
static UINT8 DTD_1024[] = {0x64, 0x19, 0x00, 0x40, 0x41, 0x00, 0x26, 0x30, 0x18, 
                           0x88, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18};

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


INT32 freqs[] = { 60, 75, 85 };

vbios_resolution_type1 * map_type1_resolution(vbios_map * map, UINT16 res)
{
	vbios_resolution_type1 * ptr = ((vbios_resolution_type1*)(map->bios_ptr + res)); 
	return ptr;
}

vbios_resolution_type2 * map_type2_resolution(vbios_map * map, UINT16 res)
{
	vbios_resolution_type2 * ptr = ((vbios_resolution_type2*)(map->bios_ptr + res)); 
	return ptr;
}

vbios_resolution_type3 * map_type3_resolution(vbios_map * map, UINT16 res)
{
	vbios_resolution_type3 * ptr = ((vbios_resolution_type3*)(map->bios_ptr + res)); 
	return ptr;
}
/*
CHAR8 detect_bios_type(vbios_map * map, CHAR8 modeline, INT32 entry_size);
CHAR8 detect_bios_type(vbios_map * map, CHAR8 modeline, INT32 entry_size)
{
	UINT32 i;
	UINT16 r1, r2;
	
	r1 = r2 = 32000;
	
	for (i=0; i < map->mode_table_size; i++)
	{
		if (map->mode_table[i].resolution <= r1)
		{
			r1 = map->mode_table[i].resolution;
		}
		else
		{
			if (map->mode_table[i].resolution <= r2)
			{
				r2 = map->mode_table[i].resolution;
			}
		}
	}
	
	return (r2-r1-6) % entry_size == 0;
}
*/
VOID close_vbios(vbios_map * map);

CHAR8 detect_ati_bios_type(vbios_map * map)
{	
	return map->mode_table_size % sizeof(ATOM_MODE_TIMING) == 0;
}


vbios_map * open_vbios(chipset_type forced_chipset)
{
	UINTN i;
	UINTN j;
	
	vbios_map * map = AllocateZeroPool(sizeof(vbios_map));
   DBG(" Bios:");
	/*
	 * Determine chipset
	 */
	
	/*
	 * dmazar: no need to mess with chipsets - we'll use LegacyRegion protocols to unlock vbios
	 *
	*/
	
	
	/*
	 *  Map the video bios to memory
	 */
	map->bios_ptr=(CHAR8*)(UINTN)VBIOS_START;
	
	/*
	 * check if we have ATI Radeon
	 */
	map->ati_tables.base = map->bios_ptr;
	map->ati_tables.AtomRomHeader = (ATOM_ROM_HEADER *) (map->bios_ptr + *(UINT16 *) (map->bios_ptr + OFFSET_TO_POINTER_TO_ATOM_ROM_HEADER)); 
	if (AsciiStrCmp((CHAR8 *) map->ati_tables.AtomRomHeader->uaFirmWareSignature, "ATOM") == 0)
	{
      UINT16 std_vesa_offset;
      ATOM_STANDARD_VESA_TIMING * std_vesa;

		// ATI Radeon Card
		DBG(" ATI");
		map->bios = BT_ATI_1;
		
		map->ati_tables.MasterDataTables = (UINT16 *) &((ATOM_MASTER_DATA_TABLE *) (map->bios_ptr + map->ati_tables.AtomRomHeader->usMasterDataTableOffset))->ListOfDataTables;
		DBG(", MasterDataTables: 0x%p", map->ati_tables.MasterDataTables);
		std_vesa_offset = (UINT16) ((ATOM_MASTER_LIST_OF_DATA_TABLES *)map->ati_tables.MasterDataTables)->StandardVESA_Timing;
		std_vesa = (ATOM_STANDARD_VESA_TIMING *) (map->bios_ptr + std_vesa_offset);
		DBG(", std_vesa: 0x%p", std_vesa);
		
		map->ati_mode_table = (CHAR8 *) &std_vesa->aModeTimings;
		DBG(", ati_mode_table: 0x%p", map->ati_mode_table);
		if (map->ati_mode_table == 0)
		{
			DBG("\n Unable to locate the mode table.\n");
			close_vbios(map);
			return 0;
		}
		map->mode_table_size = std_vesa->sHeader.usStructureSize - sizeof(ATOM_COMMON_TABLE_HEADER);
		DBG(", mode_table_size: 0x%x", map->mode_table_size);
		
		if (!detect_ati_bios_type(map)) map->bios = BT_ATI_2;
		DBG(" %a\n", map->bios == BT_ATI_1 ? "BT_ATI_1" : "BT_ATI_2");
	}
	else {
		
		/*
		 * check if we have NVIDIA
		 */

		for (i = 0; i < 512; i++)
		{ // we don't need to look through the whole bios, just the first 512 bytes
			if (CompareMem(map->bios_ptr+i, nvda_string, 4) == 0)
			{
				UINT16 nv_data_table_offset = 0;
				UINT16 * nv_data_table;
				NV_VESA_TABLE * std_vesa;
            DBG(" nVidia");
            map->bios = BT_NVDA;
				
				for (j = 0; j < 0x300; j++)
				{ //We don't need to look for the table in the whole bios, the 768 first bytes only
					if (CompareMem(map->bios_ptr+j, nvda_pattern, 4)==0)
					{
						nv_data_table_offset = *((UINT16*)(map->bios_ptr+j+4));
						DBG(", nv_data_table_offset: 0x%x", (UINTN)nv_data_table_offset);
						break;
					}
				}
				
				nv_data_table = (UINT16 *) (map->bios_ptr + (nv_data_table_offset + OFFSET_TO_VESA_TABLE_INDEX));
				DBG(", nv_data_table: 0x%p", nv_data_table);
				std_vesa = (NV_VESA_TABLE *) (map->bios_ptr + *nv_data_table);
				DBG(", std_vesa: 0x%p", std_vesa);
				
				map->nv_mode_table = (CHAR8*)std_vesa+sizeof(NV_COMMON_TABLE_HEADER);
				DBG(", nv_mode_table: 0x%p", map->nv_mode_table);
				
				if (map->nv_mode_table == 0)
				{
					DBG("\n Unable to locate the mode table.\n");
					close_vbios(map);
					return 0;
				}
				map->mode_table_size = std_vesa->sHeader.usTable_Size;
				DBG(", mode_table_size: 0x%x\n", map->mode_table_size);
				
				break;
			}
		}
	}
	
	/*
	 * check if we have Intel
	 */
  
	if (*(UINT16*)(map->bios_ptr + 0x44) == 0x8086) {
    map->bios = BT_INTEL;
  }
	
	/*
	 * Figure out where the mode table is 
	 */
/*	if ((map->bios != BT_ATI_1) && (map->bios != BT_ATI_2) && (map->bios != BT_NVDA))
	{
		CHAR8* p = map->bios_ptr + 16;
		CHAR8* limit = map->bios_ptr + VBIOS_SIZE - (3 * sizeof(vbios_mode));
		
		DBG(" Other");
		while (p < limit && map->mode_table == 0)
		{
			vbios_mode * mode_ptr = (vbios_mode *) p;
			
			if (((mode_ptr[0].mode & 0xf0) == 0x30) && ((mode_ptr[1].mode & 0xf0) == 0x30) &&
				((mode_ptr[2].mode & 0xf0) == 0x30) && ((mode_ptr[3].mode & 0xf0) == 0x30))
			{
				map->mode_table = mode_ptr;
			}
			
			p++;
		}
		
		if (map->mode_table == 0) 
		{
			DBG(", mode table not found");
			close_vbios(map);
			return 0;
		} else {
			DBG(", mode_table: 0x%p", map->mode_table);
		}
	}
*/	
	
	/*
	 * Determine size of mode table
	 */
/*	if ((map->bios != BT_ATI_1) && (map->bios != BT_ATI_2) && (map->bios != BT_NVDA))
	{
		vbios_mode * mode_ptr = map->mode_table;
		
		while (mode_ptr->mode != 0xff)
		{
			map->mode_table_size++;
			mode_ptr++;
		}
		DBG(", mode_table_size: 0x%x", map->mode_table_size);
	}
*/	
	/*
	 * Figure out what type of bios we have
	 *  order of detection is important
	 */
  /*
	if ((map->bios != BT_ATI_1) && (map->bios != BT_ATI_2) && (map->bios != BT_NVDA))
	{
		if (detect_bios_type(map, TRUE, sizeof(vbios_modeline_type3)))
		{
			map->bios = BT_3;
			DBG(", bios: BT_3\n");
		}
		else if (detect_bios_type(map, TRUE, sizeof(vbios_modeline_type2)))
		{
			map->bios = BT_2;
			DBG(", bios: BT_2\n");
		}
		else if (detect_bios_type(map, FALSE, sizeof(vbios_resolution_type1)))
		{
			map->bios = BT_1;
			DBG(", bios: BT_1\n");
		}
		else {
			DBG(", bios: unknown\n");
			return 0;
		}
	}
	*/
	return map;
}

VOID close_vbios(vbios_map * map)
{
	FreePool(map);
}


INT32 getMode(edid_mode *mode)
{
	CHAR8* edidInfo = readEDID();
			
	if(!edidInfo) return 1;
//Slice
	if(!fb_parse_edid((struct EDID *)edidInfo, mode)) 
	{
		return 1;
	}
		
	if(!mode->h_active) return 1;
	
	return 0;
		
}


VOID gtf_timings(UINT32 x, UINT32 y, UINT32 freq, /* 60, 75, 85 */
						UINT32 *clock,
						UINT16 *hsyncstart, UINT16 *hsyncend, UINT16 *hblank,
						UINT16 *vsyncstart, UINT16 *vsyncend, UINT16 *vblank)
{
	UINT32 hbl, vbl, vfreq;

	vbl = (UINT32)((2 * y) + ((2 * y)+2)/(20000/(11*freq) - 1) + 3) / 2;
	vfreq = vbl * freq;
	hbl = 16 * (INT32)((x * (30 - 300000 / vfreq) /
					 +            (70 + 300000 / vfreq) / 8 + 1) / 2);

	/*
   vbl = (UINT32)(y + (y+1)/(20000.0/(11*freq) - 1) + 1.5);
   vfreq = vbl * freq;
   hbl = 16 * (INT32)(x * (30.0 - 300000.0 / vfreq) /
      +            (70.0 + 300000.0 / vfreq) / 16.0 + 0.5);
   */

	*vsyncstart = (UINT16)y;
	*vsyncend = (UINT16)(y + 3);
	*vblank = (UINT16)(vbl - 1);
	*hsyncstart = (UINT16)(x + hbl / 2 - (x + hbl + 50) / 100 * 8 - 1);
	*hsyncend = (UINT16)(x + hbl / 2 - 1);
	*hblank = (UINT16)(x + hbl - 1);
	*clock = (UINT32)((x + hbl) * vfreq / 1000);
}

VOID set_mode(vbios_map * map, /*UINT32 mode,*/ UINT32 x, UINT32 y, UINT32 bp, UINT32 htotal, UINT32 vtotal) {
	UINT32 xprev, yprev;
	UINT32 i = 0, j;
	// patch first available mode
	
	//	for (i=0; i < map->mode_table_size; i++) {
	//		if (map->mode_table[0].mode == mode) {
	DBG(" Patching: ");
	switch(map->bios) {
		case BT_INTEL:
    {
      edid_mode mode;
      UINTN     NumReplaces;
      UINT8*    DTD_string = NULL; 
      CHAR8*    edidInfo = readEDID();
      
      DBG("Patch BT_INTEL: ");
			if (getMode(&mode)) {
        DBG("have no mode, check your EDID\n");
        break;
			}
      if ((edidInfo[54] != 0) || (edidInfo[55] != 0) ||
          (edidInfo[56] != 0) || (edidInfo[58] != 0)) {
        DTD_string = (UINT8*)&edidInfo[54];
      } else if ((edidInfo[72] != 0) || (edidInfo[73] != 0) ||
                 (edidInfo[74] != 0) || (edidInfo[76] != 0)) {
        DTD_string = (UINT8*)&edidInfo[72];
      } else {
        break;
      }

//      NumReplaces = 0;
      NumReplaces = VideoBiosPatchSearchAndReplace (
                                                    (UINT8*)(UINTN)VBIOS_START,
                                                    VBIOS_SIZE,
                                                    (UINT8*)&DTD_1024[0], 18,
                                                    DTD_string,
                                                    -1
                                                    );
      DBG(" patched %d time(s)\n", NumReplaces);
	  return;
    }

		case BT_1:
		{
			vbios_resolution_type1 * res = map_type1_resolution(map, map->mode_table[i].resolution);
			
			if (bp) {
				map->mode_table[i].bits_per_pixel = (UINT8)bp;
			}
			
			res->x2 = (htotal?(((htotal-x) >> 8) & 0x0f) : (res->x2 & 0x0f)) | ((x >> 4) & 0xf0);
			res->x1 = (x & 0xff);
			
			res->y2 = (vtotal?(((vtotal-y) >> 8) & 0x0f) : (res->y2 & 0x0f)) | ((y >> 4) & 0xf0);
			res->y1 = (y & 0xff);
			if (htotal)
				res->x_total = ((htotal-x) & 0xff);
			
			if (vtotal)
				res->y_total = ((vtotal-y) & 0xff);
			
			DBG("BT_1 patched\n");
			break;
		}
		case BT_2:
		{
			vbios_resolution_type2 * res = map_type2_resolution(map, map->mode_table[i].resolution);
			DBG("BT_2");
			
			res->xchars = (UINT8)(x / 8);
			res->ychars = (UINT8)(y / 16 - 1);
			xprev = res->modelines[0].x1;
			yprev = res->modelines[0].y1;
			
			for(j=0; j < 3; j++) {
				vbios_modeline_type2 * modeline = &res->modelines[j];
				
				if (modeline->x1 == xprev && modeline->y1 == yprev) {
					modeline->x1 = modeline->x2 = (UINT16)(x-1);
					modeline->y1 = modeline->y2 = (UINT16)(y-1);
					
					gtf_timings(x, y, freqs[j], &modeline->clock,
								&modeline->hsyncstart, &modeline->hsyncend,
								&modeline->hblank, &modeline->vsyncstart,
								&modeline->vsyncend, &modeline->vblank);
					
					if (htotal)
						modeline->htotal = (UINT16)htotal;
					else
						modeline->htotal = modeline->hblank;
					
					if (vtotal)
						modeline->vtotal = (UINT16)vtotal;
					else
						modeline->vtotal = modeline->vblank;
					DBG(", modeline %d patched", j);
				}
			}
			DBG("\n");
			break;
		}
		case BT_3:
		{
			vbios_resolution_type3 * res = map_type3_resolution(map, map->mode_table[i].resolution);
			DBG("BT_3");
			
			xprev = res->modelines[0].x1;
			yprev = res->modelines[0].y1;
			
			for (j=0; j < 3; j++) {
				vbios_modeline_type3 * modeline = &res->modelines[j];
				
				if (modeline->x1 == xprev && modeline->y1 == yprev) {
					modeline->x1 = modeline->x2 = (UINT16)(x-1);
					modeline->y1 = modeline->y2 = (UINT16)(y-1);
					
					gtf_timings(x, y, freqs[j], &modeline->clock,
								&modeline->hsyncstart, &modeline->hsyncend,
								&modeline->hblank, &modeline->vsyncstart,
								&modeline->vsyncend, &modeline->vblank);
					if (htotal)
						modeline->htotal = (UINT16)htotal;
					else
						modeline->htotal = modeline->hblank;
					if (vtotal)
						modeline->vtotal = (UINT16)vtotal;
					else
						modeline->vtotal = modeline->vblank;
					
					modeline->timing_h   = (UINT16)(y-1);
					modeline->timing_v   = (UINT16)(x-1);
					DBG(", modeline %d patched", j);
				}
			}
			DBG("\n");
			break;
		}
		case BT_ATI_1:
		{
			edid_mode mode;
				
			ATOM_MODE_TIMING *mode_timing = (ATOM_MODE_TIMING *) map->ati_mode_table;

			DBG("BT_ATI_1\n");
			//if (mode.pixel_clock && (mode.h_active == x) && (mode.v_active == y) && !force) {
			if (!getMode(&mode)) {
				DBG(" mode 0 (%dx%d) patched to %dx%d\n",
					mode_timing->usCRTC_H_Disp, mode_timing->usCRTC_V_Disp,
					mode.h_active, mode.v_active
					);
				mode_timing->usCRTC_H_Total = mode.h_active + mode.h_blanking;
				mode_timing->usCRTC_H_Disp = mode.h_active;
				mode_timing->usCRTC_H_SyncStart = mode.h_active + mode.h_sync_offset;
				mode_timing->usCRTC_H_SyncWidth = mode.h_sync_width;
					
				mode_timing->usCRTC_V_Total = mode.v_active + mode.v_blanking;
				mode_timing->usCRTC_V_Disp = mode.v_active;
				mode_timing->usCRTC_V_SyncStart = mode.v_active + mode.v_sync_offset;
				mode_timing->usCRTC_V_SyncWidth = mode.v_sync_width;

				mode_timing->usPixelClock = mode.pixel_clock;
			}
			break;
		}
		case BT_ATI_2:
		{
			edid_mode mode;
						
			ATOM_DTD_FORMAT *mode_timing = (ATOM_DTD_FORMAT *) map->ati_mode_table;
			
			DBG("BT_ATI_2\n");
			/*if (mode.pixel_clock && (mode.h_active == x) && (mode.v_active == y) && !force) {*/
			if (!getMode(&mode)) {
				DBG(" mode 0 (%dx%d) patched to %dx%d\n",
					mode_timing->usHActive, mode_timing->usVActive,
					mode.h_active, mode.v_active
					);
				mode_timing->usHBlanking_Time = mode.h_blanking;
				mode_timing->usHActive = mode.h_active;
				mode_timing->usHSyncOffset = mode.h_sync_offset;
				mode_timing->usHSyncWidth = mode.h_sync_width;
										
				mode_timing->usVBlanking_Time = mode.v_blanking;
				mode_timing->usVActive = mode.v_active;
				mode_timing->usVSyncOffset = mode.v_sync_offset;
				mode_timing->usVSyncWidth = mode.v_sync_width;
										
				mode_timing->usPixClk = mode.pixel_clock;
			}			
			break;
		}
		case BT_NVDA:
		{
			edid_mode mode;
      UINTN             NumReplaces;
//      UINTN             NumReplacesTotal;
      UINTN             Index = 0;
      
	//		NV_MODELINE *mode_timing = (NV_MODELINE *) map->nv_mode_table;
			DBG("BT_NVDA\n");
			// totally revised on work by pene
      // http://www.projectosx.com/forum/index.php?showtopic=2562&view=findpost&p=22683
			
			if (getMode(&mode)) {
        break;
			}
      //Search for desired mode in our matrix
      for (Index = 0; Index < RESOLUTIONS_NUMBER; Index++) {
        if ((nvda_res[Index].HRes == mode.h_active) && (nvda_res[Index].VRes == mode.v_active)) {
          break;
        }
      }
      if (Index == RESOLUTIONS_NUMBER) {
        DBG("the patch is not ready for the desired resolution\n");
        break; // not found
      }
      
//      NumReplaces = 0;
//      NumReplacesTotal = 0;
      NumReplaces = VideoBiosPatchSearchAndReplace (
                                                      (UINT8*)(UINTN)VBIOS_START,
                                                      VBIOS_SIZE,
                                                      (UINT8*)&Sample0[0], 17,
                                                      (UINT8*)&nvda_key0[Index].Matrix[0],
                                                      -1
                                                      );
//        NumReplacesTotal += NumReplaces;
        DBG(" patch 0: patched %d time(s)\n", NumReplaces);
      NumReplaces = VideoBiosPatchSearchAndReplace (
                                                    (UINT8*)(UINTN)VBIOS_START,
                                                    VBIOS_SIZE,
                                                    (UINT8*)&Sample1[0], 9,
                                                    (UINT8*)&nvda_key1[Index].Matrix[0],
                                                    -1
                                                    );
//      NumReplacesTotal += NumReplaces;
      DBG(" patch 1: patched %d time(s)\n", NumReplaces);
      NumReplaces = VideoBiosPatchSearchAndReplace (
                                                    (UINT8*)(UINTN)VBIOS_START,
                                                    VBIOS_SIZE,
                                                    (UINT8*)&Sample2[0], 13,
                                                    (UINT8*)&nvda_key2[Index].Matrix[0],
                                                    -1
                                                    );
//      NumReplacesTotal += NumReplaces;
      DBG(" patch 2: patched %d time(s)\n", NumReplaces);
      NumReplaces = VideoBiosPatchSearchAndReplace (
                                                    (UINT8*)(UINTN)VBIOS_START,
                                                    VBIOS_SIZE,
                                                    (UINT8*)&Sample3[0], 5,
                                                    (UINT8*)&nvda_key3[Index].Matrix[0],
                                                    -1
                                                    );
//      NumReplacesTotal += NumReplaces;
      DBG(" patch 3: patched %d time(s)\n", NumReplaces);
      
      if ((*((UINT8*)(UINTN)(VBIOS_START + 0x34)) & 0x8F) == 0x80 ) {
        *((UINT8*)(UINTN)(VBIOS_START + 0x34)) |= 0x01; 
      }
      
			break;
		}
		case BT_UNKNOWN:
		{
			DBG("unknown - not patching\n");
			break;
		}
	}
	//		}
	//	}
}

#endif // _RESOLUTION_H_
