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

//#include "libsaio.h"
//#include "edid.h"  //included
#include "915resolution.h"


void patchVideoBios()
{		
	UInt32 x = 0, y = 0, bp = 0;
	
	getResolution(&x, &y, &bp);
	verbose("getResolution: %dx%dx%d\n", (int)x, (int)y, (int)bp);
	
	if (x != 0 &&
		y != 0 && 
		bp != 0)
	{
		vbios_map * map;
		
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

int freqs[] = { 60, 75, 85 };

UInt32 get_chipset_id(void)
{
	outl(CONFIG_MECH_ONE_ADDR, 0x80000000);
	return inl(CONFIG_MECH_ONE_DATA);
}

chipset_type get_chipset(UInt32 id)
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
				//printf("Unknown chipset 0x%llX, please email id to meklort@gmail.com", id);
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

vbios_resolution_type1 * map_type1_resolution(vbios_map * map, UInt16 res)
{
	vbios_resolution_type1 * ptr = ((vbios_resolution_type1*)(map->bios_ptr + res)); 
	return ptr;
}

vbios_resolution_type2 * map_type2_resolution(vbios_map * map, UInt16 res)
{
	vbios_resolution_type2 * ptr = ((vbios_resolution_type2*)(map->bios_ptr + res)); 
	return ptr;
}

vbios_resolution_type3 * map_type3_resolution(vbios_map * map, UInt16 res)
{
	vbios_resolution_type3 * ptr = ((vbios_resolution_type3*)(map->bios_ptr + res)); 
	return ptr;
}

char detect_bios_type(vbios_map * map, char modeline, int entry_size)
{
	UInt32 i;
	UInt16 r1, r2;
	
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
		
		/*printf("r1 = %d  r2 = %d\n", r1, r2);*/
	}
	
	return (r2-r1-6) % entry_size == 0;
}

void close_vbios(vbios_map * map);

char detect_ati_bios_type(vbios_map * map)
{	
	return map->mode_table_size % sizeof(ATOM_MODE_TIMING) == 0;
}


vbios_map * open_vbios(chipset_type forced_chipset)
{
	UInt32 z;
	vbios_map * map = malloc(sizeof(vbios_map));
	for(z=0; z<sizeof(vbios_map); z++) ((char*)map)[z]=0;
	/*
	 * Determine chipset
	 */
	
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
		//verbose("Unknown chipset type.\n");
		//verbose("915resolution only works with Intel 800/900 series graphic chipsets.\n");
		//verbose("Chipset Id: %x\n", map->chipset_id);
		close_vbios(map);
		return 0;
	}
	
	
	/*
	 *  Map the video bios to memory
	 */
	map->bios_ptr=(char*)VBIOS_START;
	
	/*
	 * check if we have ATI Radeon
	 */
	map->ati_tables.base = map->bios_ptr;
	map->ati_tables.AtomRomHeader = (ATOM_ROM_HEADER *) (map->bios_ptr + *(unsigned short *) (map->bios_ptr + OFFSET_TO_POINTER_TO_ATOM_ROM_HEADER)); 
	if (strcmp ((char *) map->ati_tables.AtomRomHeader->uaFirmWareSignature, "ATOM") == 0)
	{
		// ATI Radeon Card
		map->bios = BT_ATI_1;
		
		map->ati_tables.MasterDataTables = (unsigned short *) &((ATOM_MASTER_DATA_TABLE *) (map->bios_ptr + map->ati_tables.AtomRomHeader->usMasterDataTableOffset))->ListOfDataTables;
		unsigned short std_vesa_offset = (unsigned short) ((ATOM_MASTER_LIST_OF_DATA_TABLES *)map->ati_tables.MasterDataTables)->StandardVESA_Timing;
		ATOM_STANDARD_VESA_TIMING * std_vesa = (ATOM_STANDARD_VESA_TIMING *) (map->bios_ptr + std_vesa_offset);
		
		map->ati_mode_table = (char *) &std_vesa->aModeTimings;
		if (map->ati_mode_table == 0)
		{
			printf("Unable to locate the mode table.\n");
			printf("Please run the program 'dump_bios' as root and\n");
			printf("email the file 'vbios.dmp' to stomljen@yahoo.com.\n");
			printf("Chipset: %d\n", map->chipset);
			close_vbios(map);
			return 0;
		}
		map->mode_table_size = std_vesa->sHeader.usStructureSize - sizeof(ATOM_COMMON_TABLE_HEADER);
		
		if (!detect_ati_bios_type(map)) map->bios = BT_ATI_2;
		
	}
	else {
		
		/*
		 * check if we have NVIDIA
		 */

		int i = 0;
		while (i < 512)
		{ // we don't need to look through the whole bios, just the first 512 bytes
			if ((	map->bios_ptr[i]   == 'N') 
				&& (map->bios_ptr[i+1] == 'V') 
				&& (map->bios_ptr[i+2] == 'I') 
				&& (map->bios_ptr[i+3] == 'D')) 
			{
				map->bios = BT_NVDA;
				unsigned short nv_data_table_offset = 0;
				unsigned short * nv_data_table;
				NV_VESA_TABLE * std_vesa;
				
				int i = 0;
				
				while (i < 0x300)
				{ //We don't need to look for the table in the whole bios, the 768 first bytes only
					if ((	map->bios_ptr[i] == 0x44) 
						&& (map->bios_ptr[i+1] == 0x01) 
						&& (map->bios_ptr[i+2] == 0x04) 
						&& (map->bios_ptr[i+3] == 0x00))
					{
						nv_data_table_offset = (unsigned short) (map->bios_ptr[i+4] | (map->bios_ptr[i+5] << 8));
						break;
					}
					i++;
				}
				
				nv_data_table = (unsigned short *) (map->bios_ptr + (nv_data_table_offset + OFFSET_TO_VESA_TABLE_INDEX));
				std_vesa = (NV_VESA_TABLE *) (map->bios_ptr + *nv_data_table);
				
				map->nv_mode_table = (char *) std_vesa->sModelines;
				if (map->nv_mode_table == 0)
				{
					printf("Unable to locate the mode table.\n");
					printf("Please run the program 'dump_bios' as root and\n");
					printf("email the file 'vbios.dmp' to stomljen@yahoo.com.\n");
					printf("Chipset: %s\n", map->chipset);
					close_vbios(map);
					return 0;
				}
				map->mode_table_size = std_vesa->sHeader.usTable_Size;
				
				break;
			}
			i++;
		}
	}
	
	
	/*
	 * check if we have Intel
	 */
	
	/*if (map->chipset == CT_UNKNOWN && memmem(map->bios_ptr, VBIOS_SIZE, INTEL_SIGNATURE, strlen(INTEL_SIGNATURE))) {
	 printf( "Intel chipset detected.  However, 915resolution was unable to determine the chipset type.\n");
	 
	 printf("Chipset Id: %x\n", map->chipset_id);
	 
	 printf("Please report this problem to stomljen@yahoo.com\n");
	 
	 close_vbios(map);
	 return 0;
	 }*/
	
	/*
	 * check for others
	 */
	

	
	/*
	 * Figure out where the mode table is 
	 */
	if ((map->bios != BT_ATI_1) && (map->bios != BT_NVDA)) 
	{
		char* p = map->bios_ptr + 16;
		char* limit = map->bios_ptr + VBIOS_SIZE - (3 * sizeof(vbios_mode));
		
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
			close_vbios(map);
			return 0;
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
		}
		else if (detect_bios_type(map, TRUE, sizeof(vbios_modeline_type2)))
		{
			map->bios = BT_2;
		}
		else if (detect_bios_type(map, FALSE, sizeof(vbios_resolution_type1)))
		{
			map->bios = BT_1;
		}
		else {
			return 0;
		}
	}
	
	return map;
}

void close_vbios(vbios_map * map)
{
	free(map);
}

void unlock_vbios(vbios_map * map)
{
	
	map->unlocked = TRUE;

	switch (map->chipset) {
		case CT_UNKNOWN:
			break;
		case CT_830:
		case CT_855GM:
			outl(CONFIG_MECH_ONE_ADDR, 0x8000005a);
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
		UInt32 t = inl(CONFIG_MECH_ONE_DATA);
		verbose("unlock PAM: (0x%08x)\n", t);
	}
#endif
}

void relock_vbios(vbios_map * map)
{
	
	map->unlocked = FALSE;
	
	switch (map->chipset)
	{
		case CT_UNKNOWN:
			break;
		case CT_830:
		case CT_855GM:
			outl(CONFIG_MECH_ONE_ADDR, 0x8000005a);
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
        UInt32 t = inl(CONFIG_MECH_ONE_DATA);
		verbose("relock PAM: (0x%08x)\n", t);
	}
#endif
}


int getMode(edid_mode *mode)
{
	char* edidInfo = readEDID();
			
	if(!edidInfo) return 1;
//Slice
	if(!fb_parse_edid((struct EDID *)edidInfo, mode)) 
	{
		free( edidInfo );
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
		
	free( edidInfo );
		
	if(!mode->h_active) return 1;
	
	return 0;
		
}


static void gtf_timings(UInt32 x, UInt32 y, UInt32 freq,
						unsigned long *clock,
						UInt16 *hsyncstart, UInt16 *hsyncend, UInt16 *hblank,
						UInt16 *vsyncstart, UInt16 *vsyncend, UInt16 *vblank)
{
	UInt32 hbl, vbl, vfreq;
	
	vbl = y + (y+1)/(20000.0/(11*freq) - 1) + 1.5;
	vfreq = vbl * freq;
	hbl = 16 * (int)(x * (30.0 - 300000.0 / vfreq) /
					 +            (70.0 + 300000.0 / vfreq) / 16.0 + 0.5);
	
	*vsyncstart = y;
	*vsyncend = y + 3;
	*vblank = vbl - 1;
	*hsyncstart = x + hbl / 2 - (x + hbl + 50) / 100 * 8 - 1;
	*hsyncend = x + hbl / 2 - 1;
	*hblank = x + hbl - 1;
	*clock = (x + hbl) * vfreq / 1000;
}

void set_mode(vbios_map * map, /*UInt32 mode,*/ UInt32 x, UInt32 y, UInt32 bp, UInt32 htotal, UInt32 vtotal) {
	UInt32 xprev, yprev;
	UInt32 i = 0, j;
	// patch first available mode
	
	//	for (i=0; i < map->mode_table_size; i++) {
	//		if (map->mode_table[0].mode == mode) {
	switch(map->bios) {
		case BT_INTEL:
			return;

		case BT_1:
		{
			vbios_resolution_type1 * res = map_type1_resolution(map, map->mode_table[i].resolution);
			
			if (bp) {
				map->mode_table[i].bits_per_pixel = bp;
			}
			
			res->x2 = (htotal?(((htotal-x) >> 8) & 0x0f) : (res->x2 & 0x0f)) | ((x >> 4) & 0xf0);
			res->x1 = (x & 0xff);
			
			res->y2 = (vtotal?(((vtotal-y) >> 8) & 0x0f) : (res->y2 & 0x0f)) | ((y >> 4) & 0xf0);
			res->y1 = (y & 0xff);
			if (htotal)
				res->x_total = ((htotal-x) & 0xff);
			
			if (vtotal)
				res->y_total = ((vtotal-y) & 0xff);
			
			break;
		}
		case BT_2:
		{
			vbios_resolution_type2 * res = map_type2_resolution(map, map->mode_table[i].resolution);
			
			res->xchars = x / 8;
			res->ychars = y / 16 - 1;
			xprev = res->modelines[0].x1;
			yprev = res->modelines[0].y1;
			
			for(j=0; j < 3; j++) {
				vbios_modeline_type2 * modeline = &res->modelines[j];
				
				if (modeline->x1 == xprev && modeline->y1 == yprev) {
					modeline->x1 = modeline->x2 = x-1;
					modeline->y1 = modeline->y2 = y-1;
					
					gtf_timings(x, y, freqs[j], &modeline->clock,
								&modeline->hsyncstart, &modeline->hsyncend,
								&modeline->hblank, &modeline->vsyncstart,
								&modeline->vsyncend, &modeline->vblank);
					
					if (htotal)
						modeline->htotal = htotal;
					else
						modeline->htotal = modeline->hblank;
					
					if (vtotal)
						modeline->vtotal = vtotal;
					else
						modeline->vtotal = modeline->vblank;
				}
			}
			break;
		}
		case BT_3:
		{
			vbios_resolution_type3 * res = map_type3_resolution(map, map->mode_table[i].resolution);
			
			xprev = res->modelines[0].x1;
			yprev = res->modelines[0].y1;
			
			for (j=0; j < 3; j++) {
				vbios_modeline_type3 * modeline = &res->modelines[j];
				
				if (modeline->x1 == xprev && modeline->y1 == yprev) {
					modeline->x1 = modeline->x2 = x-1;
					modeline->y1 = modeline->y2 = y-1;
					
					gtf_timings(x, y, freqs[j], &modeline->clock,
								&modeline->hsyncstart, &modeline->hsyncend,
								&modeline->hblank, &modeline->vsyncstart,
								&modeline->vsyncend, &modeline->vblank);
					if (htotal)
						modeline->htotal = htotal;
					else
						modeline->htotal = modeline->hblank;
					if (vtotal)
						modeline->vtotal = vtotal;
					else
						modeline->vtotal = modeline->vblank;
					
					modeline->timing_h   = y-1;
					modeline->timing_v   = x-1;
				}
			}
			break;
		}
		case BT_ATI_1:
		{
			edid_mode mode;
				
			ATOM_MODE_TIMING *mode_timing = (ATOM_MODE_TIMING *) map->ati_mode_table;

			//if (mode.pixel_clock && (mode.h_active == x) && (mode.v_active == y) && !force) {
			if (!getMode(&mode)) {
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
			
			/*if (mode.pixel_clock && (mode.h_active == x) && (mode.v_active == y) && !force) {*/
			if (!getMode(&mode)) {
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
									 +						mode_timing->usHActive = x;
									 +						mode_timing->usHSyncOffset = modeline.hsyncstart - x;
									 +						mode_timing->usHSyncWidth = modeline.hsyncend - modeline.hsyncstart;
									 +						
									 +						mode_timing->usVBlanking_Time = modeline.vblank;
									 +						mode_timing->usVActive = y;
									 +						mode_timing->usVSyncOffset = modeline.vsyncstart - y;
									 +						mode_timing->usVSyncWidth = modeline.hsyncend - modeline.hsyncstart;
									 +						
									 +						mode_timing->usPixClk = modeline.clock;
									 +					}*/
				
			
			break;
		}
		case BT_NVDA:
		{
			edid_mode mode;
			
			NV_MODELINE *mode_timing = (NV_MODELINE *) map->nv_mode_table;
			
			/*if (mode.pixel_clock && (mode.h_active == x) && (mode.v_active == y) && !force) {*/
			if (!getMode(&mode)) {
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
			break;
		}
	}
	//		}
	//	}
}

#endif // _RESOLUTION_H_