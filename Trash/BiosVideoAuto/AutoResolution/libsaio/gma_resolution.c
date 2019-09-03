/*
 *  gma_resolution.c
 *  
 *
 *  Created by Le Bidou on 19/03/10.
 *  Copyright 2010 ---. All rights reserved.
 *
 */

#include "gma_resolution.h"

char * bios_type_names[] = {"UNKNOWN", "TYPE 1", "TYPE 2", "TYPE 3"};
int freqs[] = { 60, 75, 85 };


vbios_resolution_type1 * map_type1_resolution(vbios_map * map, UInt16 res) {
	vbios_resolution_type1 * ptr = ((vbios_resolution_type1*)(map->bios_ptr + res)); 
	return ptr;
}

vbios_resolution_type2 * map_type2_resolution(vbios_map * map, UInt16 res) {
	vbios_resolution_type2 * ptr = ((vbios_resolution_type2*)(map->bios_ptr + res)); 
	return ptr;
}

vbios_resolution_type3 * map_type3_resolution(vbios_map * map, UInt16 res) {
	vbios_resolution_type3 * ptr = ((vbios_resolution_type3*)(map->bios_ptr + res)); 
	return ptr;
}

char detect_bios_type(vbios_map * map, char modeline, int entry_size) {
	UInt32 i;
	UInt16 r1, r2;
	
	vbios_mode * mode_table = (vbios_mode *)map->mode_table;
	
	r1 = r2 = 32000;
	
	for (i=0; i < map->mode_table_size; i++) {
		if (mode_table[i].resolution <= r1) {
			r1 = mode_table[i].resolution;
		}
		else {
			if (mode_table[i].resolution <= r2) {
				r2 = mode_table[i].resolution;
			}
		}
		
		/*PRINT("r1 = %d  r2 = %d\n", r1, r2);*/
	}
	
	return (r2-r1-6) % entry_size == 0;
}

vbios_map * open_intel_vbios(vbios_map *map)
{
	/*
	 * Find the location of the Mode Table
	 */
	unsigned char* p = map->bios_ptr + 16;
	unsigned char* limit = map->bios_ptr + VBIOS_SIZE - (3 * sizeof(vbios_mode));
	
	while (p < limit && map->mode_table == 0) {
		vbios_mode* mode_ptr = (vbios_mode*) p;
		
		if (((mode_ptr[0].mode & 0xf0) == 0x30) && ((mode_ptr[1].mode & 0xf0) == 0x30) &&
			((mode_ptr[2].mode & 0xf0) == 0x30) && ((mode_ptr[3].mode & 0xf0) == 0x30)) {
			
			map->mode_table = (char *)mode_ptr;
		}
		
		p++;
	}
	
	if (map->mode_table == 0) {
		PRINT("Unable to locate the mode table.\n");
		PRINT("Please run the program 'dump_bios' as root and\n");
		PRINT("email the file 'vbios.dmp' to gaeloulacuisse@yahoo.fr.\n");
		
		close_vbios(map);
		return 0;
	}
	
	PRINT("Mode Table at offset : 0x%x\n", ((unsigned char *)map->mode_table) - map->bios_ptr);
	
	/*
	 * Determine size of mode table
	 */
	
	vbios_mode * mode_ptr = (vbios_mode *)map->mode_table;
	
	while (mode_ptr->mode != 0xff) {
		map->mode_table_size++;
		mode_ptr++;
	}
	
	map->modeline_num = map->mode_table_size;
	PRINT("Mode Table size : %d\n", map->modeline_num);
	
	/*
	 * Figure out what type of bios we have
	 *  order of detection is important
	 */
	
	if (detect_bios_type(map, TRUE, sizeof(vbios_modeline_type3))) {
		map->bios = BT_3;
		PRINT("Bios Type : BT_3\n");
	}
	else if (detect_bios_type(map, TRUE, sizeof(vbios_modeline_type2))) {
		map->bios = BT_2;
		PRINT("Bios Type : BT_2\n");
	}
	else if (detect_bios_type(map, FALSE, sizeof(vbios_resolution_type1))) {
		map->bios = BT_1;
		PRINT("Bios Type : BT_1\n");
	}
	else {
		PRINT("Unable to determine bios type.\n");
		PRINT("Please run the program 'dump_bios' as root and\n");
		PRINT("email the file 'vbios.dmp' to gaeloulacuisse@yahoo.fr.\n");
		
		return 0;
	}
	
	return map;
}
	


bool intel_set_mode_1(vbios_map* map, UInt8 idx, UInt32* x, UInt32* y)
{
	vbios_mode *mode_timing = (vbios_mode *) map->mode_table;
	vbios_resolution_type1 * res = map_type1_resolution(map, mode_timing[idx].resolution);
	
	UInt32 actual_x = ((res->x2 & 0xf0) << 4) | (res->x1 & 0xff);
	UInt32 actual_y = ((res->y2 & 0xf0) << 4) | (res->y1 & 0xff);
	
	if ((*x != 0) && (*y != 0) && ( actual_x >= 640 )) {
		
		PRINT("Mode %dx%d -> %dx%d \n", actual_x, actual_y, *x, *y);
		
		res->x2 = (res->x2 & 0x0f) | ((*x >> 4) & 0xf0);
		res->x1 = (*x & 0xff);
		
		res->y2 = ((*y >> 4) & 0xf0);
		res->y1 = (*y & 0xff);
	}
	
	res = map_type1_resolution(map, mode_timing[idx + 1].resolution);
	
	actual_x = ((res->x2 & 0xf0) << 4) | (res->x1 & 0xff);
	actual_y = ((res->y2 & 0xf0) << 4) | (res->y1 & 0xff);
	
	*x = actual_x;
	*y = actual_y;
	
	return TRUE;
}

bool intel_set_mode_2(vbios_map* map, UInt8 idx, UInt32* x, UInt32* y)
{
	UInt32 xprev, yprev, j = 0;
	
	vbios_mode *mode_timing = (vbios_mode *) map->mode_table;
	vbios_resolution_type2 * res = map_type2_resolution(map, mode_timing[idx].resolution);
	
	if ((*x != 0) && (*y != 0) && ((res->modelines[0].x1 + 1) >= 640 )) {
		
		PRINT("Mode %dx%d -> %dx%d \n", res->modelines[0].x1 + 1, res->modelines[0].y1 + 1, *x, *y);
		
		res->xchars = *x / 8;
		res->ychars = *y / 16 - 1;
		xprev = res->modelines[0].x1;
		yprev = res->modelines[0].y1;
		
		for(j = 0; j < 3; j++) {
			vbios_modeline_type2 * mode = &res->modelines[j];
			
			if (mode->x1 == xprev && mode->y1 == yprev) {
				mode->x1 = mode->x2 = *x-1;
				mode->y1 = mode->y2 = *y-1;
				
				gtf_timings(*x, *y, freqs[j], &mode->clock,
							&mode->hsyncstart, &mode->hsyncend,
							&mode->hblank, &mode->vsyncstart,
							&mode->vsyncend, &mode->vblank);
				
				mode->htotal = mode->hblank;
				mode->vtotal = mode->vblank;
			}
		}
	}
	
	res = map_type2_resolution(map, mode_timing[idx + 1].resolution);
	
	*x = res->modelines[0].x1 + 1;
	*y = res->modelines[0].y1 + 1;
	
	return TRUE;
}

bool intel_set_mode_3(vbios_map* map, UInt8 idx, UInt32* x, UInt32* y)
{
	UInt32 xprev, yprev, j = 0;
	
	vbios_mode *mode_timing = (vbios_mode *) map->mode_table;
	vbios_resolution_type3 * res = map_type3_resolution(map, mode_timing[idx].resolution);
	
	if ((*x != 0) && (*y != 0) && ((res->modelines[0].x1 + 1) >= 640 )) {
		
		PRINT("Mode %dx%d -> %dx%d  \n", res->modelines[0].x1 + 1, res->modelines[0].y1 + 1, *x, *y);
		
		xprev = res->modelines[0].x1;
		yprev = res->modelines[0].y1;
		
		for(j = 0; j < 3; j++) {
			vbios_modeline_type3 * mode = &res->modelines[j];
			
			if (mode->x1 == xprev && mode->y1 == yprev) {
				mode->x1 = mode->x2 = *x-1;
				mode->y1 = mode->y2 = *y-1;
				
				gtf_timings(*x, *y, freqs[j], &mode->clock,
							&mode->hsyncstart, &mode->hsyncend,
							&mode->hblank, &mode->vsyncstart,
							&mode->vsyncend, &mode->vblank);
				
				mode->htotal = mode->hblank;
				mode->vtotal = mode->vblank;
				
				mode->timing_h   = *y-1;
				mode->timing_v   = *x-1;
			}
		}
	}
	res = map_type3_resolution(map, mode_timing[idx + 1].resolution);
	
	*x = res->modelines[0].x1 + 1;
	*y = res->modelines[0].y1 + 1;
	
	return TRUE;
}