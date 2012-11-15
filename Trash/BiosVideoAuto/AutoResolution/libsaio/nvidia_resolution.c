/*
 *  nvidia_resolution.c
 *  
 *
 *  Created by Le Bidou on 19/03/10.
 *  Copyright 2010 ---. All rights reserved.
 *
 */

#include "nvidia_resolution.h"

vbios_map * open_nvidia_vbios(vbios_map *map)
{
	UINT16 nv_data_table_offset = 0;
	UINT16 nv_modeline_2_offset = 0;
	UINT16 * nv_data_table = NULL;
	NV_VESA_TABLE * std_vesa;
	
	/*
	 * Locate the VESA Tables
	 */
	
	int i = 0;
	
	while (i < 0x300) { //We don't need to look for the table in the whole bios, the 768 first bytes only
		if ((map->bios_ptr[i] == 0x44) 
			&& (map->bios_ptr[i+1] == 0x01) 
			&& (map->bios_ptr[i+2] == 0x04) 
			&& (map->bios_ptr[i+3] == 0x00)) {
			nv_data_table_offset = (UINT16) (map->bios_ptr[i+4] | (map->bios_ptr[i+5] << 8));
			break;
		}
		i++;
	}
	//Second VESA Table on some nVidia 8xxx 9xxx and GT
	while (i < VBIOS_SIZE) { //We don't know how to locate it other way
		if ((map->bios_ptr[i] == 0x40) && (map->bios_ptr[i+1] == 0x01) //this is the first 320x200 modeline.
			&& (map->bios_ptr[i+2] == 0xC8) && (map->bios_ptr[i+3] == 0x00)
			&& (map->bios_ptr[i+4] == 0x28)
			&& (map->bios_ptr[i+5] == 0x18)
			&& (map->bios_ptr[i+6] == 0x08)
			&& (map->bios_ptr[i+7] == 0x08)) {
			nv_modeline_2_offset = (UINT16) i;
			break;
		}
		i++;
	}
	
	nv_data_table = (UINT16 *) (map->bios_ptr + (nv_data_table_offset + OFFSET_TO_VESA_TABLE_INDEX));
	std_vesa = (NV_VESA_TABLE *) (map->bios_ptr + *nv_data_table);
	map->mode_table = (CHAR8 *) std_vesa->sModelines;
	PRINT("First Standard VESA Table at offset 0x%x\n", *nv_data_table);
	
	if (nv_modeline_2_offset == (VBIOS_SIZE-1) || nv_modeline_2_offset == 0) {
		map->nv_mode_table_2 = NULL;
		PRINT("There is no Second Standard VESA Table to patch\n");
	} else {
		map->nv_mode_table_2 = (CHAR8*) map->bios_ptr + nv_modeline_2_offset;
		PRINT("Second Standard VESA Table at offset 0x%x\n", nv_modeline_2_offset);
	}
	
	if (map->mode_table == NULL) {
		PRINT("Unable to locate the mode table.\n");
		PRINT("Please run the program 'dump_bios' as root and\n");
		PRINT("email the file 'vbios.dmp' to gaeloulacuisse@yahoo.fr.\n");
		
		close_vbios(map);
		return 0;
	}
	
		/*
	 * Having trouble determining the number of table
	 * so it's hardcode to 16 and 31 for the First VESA table
	 * and the Second Vesa Table respectively
		 	 */
		
	map->modeline_num = 16;
	map->nv_modeline_num_2 = 31;
	
	/*NV_MODELINE_2 *	mode_2_ptr =	(NV_MODELINE_2 *)	map->nv_mode_table_2;
		map->modeline_num = map->nv_modeline_num_2 = 0;
		
		//First Table
	map->modeline_num = std_vesa->sHeader.usTableSize;
		
	PRINT("First VESA Table has %d modes\n",map->modeline_num);
		if (map->modeline_num == 0) {
		PRINT("%d is incorrect, make it a 16\n",map->modeline_num);
				map->modeline_num = 16;
			}
		map->mode_table_size = map->modeline_num * sizeof(NV_MODELINE);
		
		//Second Table
	map->nv_modeline_num_2 = *((UInt8 *)(mode_2_ptr - 2)) - 32;
		
	PRINT("Second VESA Table has %d modes\n",map->nv_modeline_num_2);
		if (map->nv_modeline_num_2 == 0) {
		PRINT("%d is incorrect, make it a 32\n",map->nv_modeline_num_2);
				map->nv_modeline_num_2 = 32;
			}
	map->nv_mode_table_2_size = map->nv_modeline_num_2 * sizeof(NV_MODELINE_2);*/
	
	return map;
}

bool nvidia_set_mode(vbios_map* map, UInt8 idx, UInt32* x, UInt32* y, CHAR8 Type)
{
	if (Type == MAIN_VESA_TABLE) {
		NV_MODELINE * mode_timing = (NV_MODELINE *) map->mode_table;
		
		if ((*x != 0) && (*y != 0) && ( mode_timing[idx].usH_Active >= 640 )) {
			
			PRINT("Mode %dx%d -> %dx%d ", mode_timing[idx].usH_Active, mode_timing[idx].usV_Active,
					*x, *y);
			
			generic_modeline modeline;
			
			cvt_timings(*x, *y, 60, &modeline.clock,
						&modeline.hsyncstart, &modeline.hsyncend,
						&modeline.htotal, &modeline.vsyncstart,
						&modeline.vsyncend, &modeline.vtotal, FALSE);
			
			mode_timing[idx].usH_Active = *x;
			mode_timing[idx].usH_Active_minus_One = *x - 1;
			mode_timing[idx].usH_Active_minus_One_ = *x - 1;
			
			mode_timing[idx].usV_Active = *y;
			mode_timing[idx].usV_Active_minus_One = *y - 1;
			mode_timing[idx].usV_Active_minus_One_ = *y - 1;
			
			mode_timing[idx].usH_Total = modeline.htotal;
			mode_timing[idx].usH_SyncStart = modeline.hsyncstart;
			mode_timing[idx].usH_SyncEnd = modeline.hsyncend;
			
			mode_timing[idx].usV_Total = modeline.vtotal;
			mode_timing[idx].usV_SyncStart = modeline.vsyncstart;
			mode_timing[idx].usV_SyncEnd = modeline.vsyncend;
			
			mode_timing[idx].usPixel_Clock = modeline.clock;
		}
		
		*x = mode_timing[idx + 1].usH_Active;
		*y = mode_timing[idx + 1].usV_Active;
	}
	
	if (Type == SECOND_VESA_TABLE) {
		NV_MODELINE_2 * mode_timing = (NV_MODELINE_2 *) map->nv_mode_table_2;
		
		if (mode_timing[idx].h_disp > 0x800) return FALSE;
		
		if ((*x != 0) && (*y != 0) && ( mode_timing[idx].h_disp >= 640 )) {
			
			PRINT("Mode %dx%d -> %dx%d ", mode_timing[idx].h_disp, mode_timing[idx].v_disp,
					*x, *y);
			
			generic_modeline modeline;
			
			cvt_timings(*x, *y, 60, &modeline.clock,
						&modeline.hsyncstart, &modeline.hsyncend,
						&modeline.htotal, &modeline.vsyncstart,
						&modeline.vsyncend, &modeline.vtotal, TRUE);
			
			mode_timing[idx].h_disp = *x;
			mode_timing[idx].v_disp = *y;
			mode_timing[idx].h_blank = modeline.htotal - *x;
			mode_timing[idx].h_syncoffset = modeline.hsyncstart - *x;
			mode_timing[idx].h_syncwidth = modeline.hsyncend - modeline.hsyncstart;
			mode_timing[idx].v_blank = modeline.vtotal - *y;
		}
		
		*x = mode_timing[idx + 1].h_disp;
		*y = mode_timing[idx + 1].v_disp;
	}
	return TRUE;
}
