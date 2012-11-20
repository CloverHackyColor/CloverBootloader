/*
 *  resolution.h
 *  
 *	NOTE: I don't beleive this code is production ready / should be in trunk
 * Atleast, not in it's current state. 
 *
 *  Created by Evan Lojewski on 3/4/10.
 *  Copyright 2009. All rights reserved.
 *
 */
#ifndef _RESOLUTION_H_
#define _RESOLUTION_H_

#include "VideoBiosPatchLibInternal.h"
//#include "libsaio.h"
//#include "edid.h"  //included
//#include "915resolution.h"


VOID patchVideoBios()
{
	UINT32 x = 0, y = 0, bp = 0;
	
	getResolution(&x, &y, &bp);
	DBG("getResolution: %dx%dx%d\n", x, y, bp);
	
	if (x != 0 &&
		y != 0 && 
		bp != 0)
	{
		vbios_map * map;
      DBG("Opening BIOS\n");
		
		map = open_vbios(CT_UNKNOWN);
		if(map)
		{
      
			unlock_vbios(map);
			
			set_mode(map, x, y, bp, 0, 0);
			
			relock_vbios(map);
			
			close_vbios(map);
		}
	}
}


static const UINT8 nvda_pattern[] = {
  0x44, 0x01, 0x04, 0x00
};

static const CHAR8 nvda_string[] = "NVID";

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

/**
 **
 **/

#define CONFIG_MECH_ONE_ADDR	0xCF8
#define CONFIG_MECH_ONE_DATA	0xCFC

INT32 freqs[] = { 60, 75, 85 };

UINT32 get_chipset_id(VOID)
{
  /*FIXME: assembler port I/O*/
  
	outl(CONFIG_MECH_ONE_ADDR, 0x80000000);
	return inl(CONFIG_MECH_ONE_DATA);
}

chipset_type get_chipset(UINT32 id)
{
	chipset_type type;
		
	switch (id) {
		case 0x35758086:
			type = CT_830;
			break;
			
		case 0x25608086:
			type = CT_845G;
			break;
			
		case 0x35808086:
			type = CT_855GM;
			break;
			
		case 0x25708086:
			type = CT_865G;
			break;
			
		case 0x25808086:
			type = CT_915G;
			break;
			
		case 0x25908086:
			type = CT_915GM;
			break;
			
		case 0x27708086:
			type = CT_945G;
			break;
			
		case 0x27a08086:
			type = CT_945GM;
			break;
			
		case 0x27ac8086:
			type = CT_945GME;
			break;
			
		case 0x29708086:
			type = CT_946GZ;
			break;
			
		case 0x27748086:
			type = CT_955X;
			break;
			
		case 0x277c8086:
			type = CT_975X;
			break;

		case 0x29a08086:
			type = CT_G965;
			break;
			
		case 0x29908086:
			type = CT_Q965;
			break;
			
		case 0x81008086:
			type = CT_500;
			break;
			
		case 0x2e108086:
		case 0X2e908086:
			type = CT_B43;
			break;

		case 0x2e208086:
			type = CT_P45;
			break;

		case 0x2e308086:
			type = CT_G41;
			break;
					
		case 0x29c08086:
			type = CT_G31;
			break;
			
		case 0x29208086:
			type = CT_G45;
			break;
			
		case 0xA0108086:	// mobile
		case 0xA0008086:	// desktop
			type = CT_3150;
			break;
			
		case 0x2a008086:
			type = CT_965GM;
			break;
			
		case 0x29e08086:
			type = CT_X48;
			break;			
				
		case 0x2a408086:
			type = CT_GM45;
			break;
			
			
		default:
			if((id & 0x0000FFFF) == 0x00008086) // Intel chipset
			{
				//DBG("Unknown chipset 0x%llX, please post id to projectosx.com or applelife.ru", id);
				//getc();
				type = CT_UNKNOWN_INTEL;
				//type = CT_UNKNOWN;

			}
			else
			{
				type = CT_UNKNOWN;
			}
			break;
	}
	return type;
}

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
		
		/*DBG("r1 = %d  r2 = %d\n", r1, r2);*/
	}
	
	return (r2-r1-6) % entry_size == 0;
}

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
	if (forced_chipset == CT_UNKNOWN)
	{
		map->chipset_id = get_chipset_id();
		map->chipset = get_chipset(map->chipset_id);
	}
	else if (forced_chipset != CT_UNKNOWN)
	{
		map->chipset = forced_chipset;
	}
	
	
	if (map->chipset == CT_UNKNOWN)
	{
		//DBG("Unknown chipset type.\n");
		//DBG("915resolution only works with Intel 800/900 series graphic chipsets.\n");
		//DBG("Chipset Id: %x\n", map->chipset_id);
		close_vbios(map);
		return 0;
	}
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
	if (AsciiStrCmp ((CHAR8 *) map->ati_tables.AtomRomHeader->uaFirmWareSignature, "ATOM") == 0)
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
			DBG(" Please run the program 'dump_bios' as root and\n");
			DBG(" email the file 'vbios.dmp' to stomljen@yahoo.com.\n");
			//DBG("Chipset: %d\n", map->chipset);
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
						// hm, orginal was:
						//nv_data_table_offset = (unsigned short) (map->bios_ptr[i+4] | (map->bios_ptr[i+5] << 8));

						break;
					}
				}
				
				nv_data_table = (UINT16 *) (map->bios_ptr + (nv_data_table_offset + OFFSET_TO_VESA_TABLE_INDEX));
				DBG(", nv_data_table: 0x%p", nv_data_table);
				std_vesa = (NV_VESA_TABLE *) (map->bios_ptr + *nv_data_table);
				DBG(", std_vesa: 0x%p", std_vesa);
				
				map->nv_mode_table = (CHAR8*)std_vesa+sizeof(NV_COMMON_TABLE_HEADER);
				DBG(", nv_mode_table: 0x%p", map->nv_mode_table);
				// hm, original was:
				//map->nv_mode_table = (char *) std_vesa->sModelines;

				
				if (map->nv_mode_table == 0)
				{
					DBG("\n Unable to locate the mode table.\n");
					DBG(" Please run the program 'dump_bios' as root and\n");
					DBG(" email the file 'vbios.dmp' to stomljen@yahoo.com.\n");
					//DBG("Chipset: %s\n", map->chipset);
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
	
	/*if (map->chipset == CT_UNKNOWN && memmem(map->bios_ptr, VBIOS_SIZE, INTEL_SIGNATURE, strlen(INTEL_SIGNATURE))) {
	 DBG( "Intel chipset detected.  However, 915resolution was unable to determine the chipset type.\n");
	 
	 DBG("Chipset Id: %x\n", map->chipset_id);
	 
	 DBG("Please report this problem to stomljen@yahoo.com\n");
	 
	 close_vbios(map);
	 return 0;
	 }*/
	
	/*
	 * check for others
	 */
	

	
	/*
	 * Figure out where the mode table is 
	 */
	if ((map->bios != BT_ATI_1) && (map->bios != BT_ATI_2) && (map->bios != BT_NVDA))
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
	
	
	/*
	 * Determine size of mode table
	 */
	if ((map->bios != BT_ATI_1) && (map->bios != BT_ATI_2) && (map->bios != BT_NVDA))
	{
		vbios_mode * mode_ptr = map->mode_table;
		
		while (mode_ptr->mode != 0xff)
		{
			map->mode_table_size++;
			mode_ptr++;
		}
		DBG(", mode_table_size: 0x%x", map->mode_table_size);
	}
	
	/*
	 * Figure out what type of bios we have
	 *  order of detection is important
	 */
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
	
	return map;
}

VOID close_vbios(vbios_map * map)
{
	FreePool(map);
}

VOID unlock_vbios(vbios_map * map)
{
	
	map->unlocked = TRUE;

	switch (map->chipset) {
		case CT_UNKNOWN:
			break;
		case CT_830:
		case CT_855GM:
			outl(CONFIG_MECH_ONE_ADDR, 0x8000005a); /*FIXME: assembler port I/O*/
			map->b1 = inb(CONFIG_MECH_ONE_DATA + 2);
			
			outl(CONFIG_MECH_ONE_ADDR, 0x8000005a);
			outb(CONFIG_MECH_ONE_DATA + 2, 0x33);
			break;
		case CT_845G:
		case CT_865G:
		case CT_915G:
		case CT_915GM:
		case CT_945G:
		case CT_945GM:
		case CT_945GME:
		case CT_946GZ:
		case CT_G965:
		case CT_Q965:
		case CT_965GM:
		case CT_975X:
		case CT_P35:
		case CT_955X:
		case CT_X48:
		case CT_B43:
		case CT_Q45:
		case CT_P45:
		case CT_GM45:
		case CT_G45:
		case CT_G41:
		case CT_G31:
		case CT_500:
		case CT_3150:
		case CT_UNKNOWN_INTEL:	// Assume newer intel chipset is the same as before
			outl(CONFIG_MECH_ONE_ADDR, 0x80000090);
			map->b1 = inb(CONFIG_MECH_ONE_DATA + 1);
			map->b2 = inb(CONFIG_MECH_ONE_DATA + 2);
			outl(CONFIG_MECH_ONE_ADDR, 0x80000090);
			outb(CONFIG_MECH_ONE_DATA + 1, 0x33);
			outb(CONFIG_MECH_ONE_DATA + 2, 0x33);
			break;
	}
	
#if DEBUG
	{
		UINT32 t = inl(CONFIG_MECH_ONE_DATA);
		DBG("unlock PAM: (0x%08x)\n", t);
	}
#endif
}

VOID relock_vbios(vbios_map * map)
{
	
	map->unlocked = FALSE;
	
	switch (map->chipset)
	{
		case CT_UNKNOWN:
			break;
		case CT_830:
		case CT_855GM:
			outl(CONFIG_MECH_ONE_ADDR, 0x8000005a); /*FIXME: assembler port I/O*/
			outb(CONFIG_MECH_ONE_DATA + 2, map->b1);
			break;
		case CT_845G:
		case CT_865G:
		case CT_915G:
		case CT_915GM:
		case CT_945G:
		case CT_945GM:
		case CT_945GME:
		case CT_946GZ:
		case CT_G965:
		case CT_955X:
		case CT_G45:
		case CT_Q965:
		case CT_965GM:
		case CT_975X:
		case CT_P35:
		case CT_X48:
		case CT_B43:
		case CT_Q45:
		case CT_P45:
		case CT_GM45:
		case CT_G41:
		case CT_G31:
		case CT_500:
		case CT_3150:
		case CT_UNKNOWN_INTEL:
			outl(CONFIG_MECH_ONE_ADDR, 0x80000090);
			outb(CONFIG_MECH_ONE_DATA + 1, map->b1);
			outb(CONFIG_MECH_ONE_DATA + 2, map->b2);
			break;
	}
	
#if DEBUG
	{
        UINT32 t = inl(CONFIG_MECH_ONE_DATA);
		DBG("relock PAM: (0x%08x)\n", t);
	}
#endif
}


INT32 getMode(edid_mode *mode)
{
	CHAR8* edidInfo = readEDID();
			
	if(!edidInfo) return 1;
//Slice
	if(!fb_parse_edid((struct EDID *)edidInfo, mode)) 
	{
		FreePool( edidInfo );
		return 1;
	}
/*	mode->pixel_clock = (edidInfo[55] << 8) | edidInfo[54];
	mode->h_active =  edidInfo[56] | ((edidInfo[58] & 0xF0) << 4);
	mode->h_blanking = ((edidInfo[58] & 0x0F) << 8) | edidInfo[57];
	mode->v_active = edidInfo[59] | ((edidInfo[61] & 0xF0) << 4);
	mode->v_blanking = ((edidInfo[61] & 0x0F) << 8) | edidInfo[60];
	mode->h_sync_offset = ((edidInfo[65] & 0xC0) >> 2) | edidInfo[62];
	mode->h_sync_width = (edidInfo[65] & 0x30) | edidInfo[63];
	mode->v_sync_offset = (edidInfo[65] & 0x0C) | ((edidInfo[64] & 0x0C) >> 2);
	mode->v_sync_width = ((edidInfo[65] & 0x3) << 2) | (edidInfo[64] & 0x03);
*/		
		
	FreePool( edidInfo );
		
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
			return;

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
			/*else
			{
				vbios_modeline_type2 modeline;

				cvt_timings(x, y, freqs[0], &modeline.clock,
							&modeline.hsyncstart, &modeline.hsyncend,
							&modeline.hblank, &modeline.vsyncstart,
							&modeline.vsyncend, &modeline.vblank, 0);

				mode_timing->usCRTC_H_Total = x + modeline.hblank;
				mode_timing->usCRTC_H_Disp = x;
				mode_timing->usCRTC_H_SyncStart = modeline.hsyncstart;
				mode_timing->usCRTC_H_SyncWidth = modeline.hsyncend - modeline.hsyncstart;

				mode_timing->usCRTC_V_Total = y + modeline.vblank;
				mode_timing->usCRTC_V_Disp = y;
				mode_timing->usCRTC_V_SyncStart = modeline.vsyncstart;
				mode_timing->usCRTC_V_SyncWidth = modeline.vsyncend - modeline.vsyncstart;
												
				mode_timing->usPixelClock = modeline.clock;
			 }*/
	
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
			/*else
			{
				vbios_modeline_type2 modeline;
			
				cvt_timings(x, y, freqs[0], &modeline.clock,
							&modeline.hsyncstart, &modeline.hsyncend,
							&modeline.hblank, &modeline.vsyncstart,
							&modeline.vsyncend, &modeline.vblank, 0);
											
				mode_timing->usHBlanking_Time = modeline.hblank;
											mode_timing->usHActive = x;
											mode_timing->usHSyncOffset = modeline.hsyncstart - x;
											mode_timing->usHSyncWidth = modeline.hsyncend - modeline.hsyncstart;
											
											mode_timing->usVBlanking_Time = modeline.vblank;
											mode_timing->usVActive = y;
											mode_timing->usVSyncOffset = modeline.vsyncstart - y;
											mode_timing->usVSyncWidth = modeline.hsyncend - modeline.hsyncstart;
											
											mode_timing->usPixClk = modeline.clock;
										}*/
				
			
			break;
		}
		case BT_NVDA:
		{
			edid_mode mode;
			
			NV_MODELINE *mode_timing = (NV_MODELINE *) map->nv_mode_table;
			DBG("BT_NVDA\n");
			
			/*if (mode.pixel_clock && (mode.h_active == x) && (mode.v_active == y) && !force) {*/
			if (!getMode(&mode)) {
				DBG(" mode %d (%dx%d) patched to %dx%d\n",
					i,
					mode_timing[i].usH_Active, mode_timing[i].usV_Active,
					mode.h_active, mode.v_active
					);
				mode_timing[i].usH_Total = mode.h_active + mode.h_blanking;
				mode_timing[i].usH_Active = mode.h_active;
				mode_timing[i].usH_SyncStart = mode.h_active + mode.h_sync_offset;
				mode_timing[i].usH_SyncEnd = mode.h_active + mode.h_sync_offset + mode.h_sync_width;
				
				mode_timing[i].usV_Total = mode.v_active + mode.v_blanking;
				mode_timing[i].usV_Active = mode.v_active;
				mode_timing[i].usV_SyncStart = mode.v_active + mode.v_sync_offset;
				mode_timing[i].usV_SyncEnd = mode.v_active + mode.v_sync_offset + mode.v_sync_width;
				
				mode_timing[i].usPixel_Clock = mode.pixel_clock;
			}
			/*else
			 {
			 vbios_modeline_type2 modeline;
			 
			 cvt_timings(x, y, freqs[0], &modeline.clock,
			 &modeline.hsyncstart, &modeline.hsyncend,
			 &modeline.hblank, &modeline.vsyncstart,
			 &modeline.vsyncend, &modeline.vblank, 0);
			 
			 mode_timing[i].usH_Total = x + modeline.hblank - 1;
			 mode_timing[i].usH_Active = x;
			 mode_timing[i].usH_SyncStart = modeline.hsyncstart - 1;
			 mode_timing[i].usH_SyncEnd = modeline.hsyncend - 1;
			 
			 mode_timing[i].usV_Total = y + modeline.vblank - 1;
			 mode_timing[i].usV_Active = y;
			 mode_timing[i].usV_SyncStart = modeline.vsyncstart - 1;
			 mode_timing[i].usV_SyncEnd = modeline.vsyncend - 1;
			 
			 mode_timing[i].usPixel_Clock = modeline.clock;
			 }*/
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