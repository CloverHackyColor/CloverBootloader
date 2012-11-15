/*
 *  ati_resolution.c
 *  
 *
 *  Created by Le Bidou on 19/03/10.
 *  Copyright 2010 ---. All rights reserved.
 *
 */

#include "ati_resolution.h"

char detect_ati_bios_type(vbios_map * map) {	
	return map->mode_table_size % sizeof(ATOM_MODE_TIMING) == 0;
}

vbios_map * open_ati_vbios(vbios_map * map, bios_tables_t ati_tables)
{
	/*
	 * Locate the Standard VESA Table
	 */
	
	ati_tables.MasterDataTables = (unsigned short *) &((ATOM_MASTER_DATA_TABLE *) (map->bios_ptr + ati_tables.AtomRomHeader->usMasterDataTableOffset))->ListOfDataTables;
	unsigned short std_vesa_offset = (unsigned short) ((ATOM_MASTER_LIST_OF_DATA_TABLES *)ati_tables.MasterDataTables)->StandardVESA_Timing;
	ATOM_STANDARD_VESA_TIMING * std_vesa = (ATOM_STANDARD_VESA_TIMING *) (map->bios_ptr + std_vesa_offset);
	
	map->mode_table = (char *) &std_vesa->aModeTimings;
	PRINT("Standard VESA Table at offset * 0x%x\n", std_vesa_offset);
	if (map->mode_table == 0) {
		PRINT("Unable to locate the mode table.\n");
		PRINT("Please run the program 'dump_bios' as root and\n");
		PRINT("email the file 'vbios.dmp' to gaeloulacuisse@yahoo.fr.\n");
		
		close_vbios(map);
		return 0;
	}
	
	//Determine Size of the Table
	map->mode_table_size = std_vesa->sHeader.usStructureSize - sizeof(ATOM_COMMON_TABLE_HEADER);
	
	/*
	 * Find out type of table and how many entries it has
	 */
	
	if (!detect_ati_bios_type(map)) map->bios = BT_ATI_2;
	if (map->bios == BT_ATI_2) {
		map->modeline_num = map->mode_table_size / sizeof(ATOM_DTD_FORMAT);
		PRINT("Using DTD Format modelines\n");
	} else {
		map->modeline_num = map->mode_table_size / sizeof(ATOM_MODE_TIMING);
		PRINT("Using Atom Mode Timing modelines\n");
	}
	return map;
}

bool ati_set_mode_1(vbios_map* map, UInt8 idx, UInt32* x, UInt32* y)
{
	ATOM_MODE_TIMING *mode_timing = (ATOM_MODE_TIMING *) map->mode_table;
	
	if ((*x != 0) && (*y != 0) && ( mode_timing[idx].usCRTC_H_Disp >= 640 )) {
		
		PRINT("Mode %dx%d -> %dx%d\n",mode_timing[idx].usCRTC_H_Disp,mode_timing[idx].usCRTC_V_Disp,
			   *x, *y);
		
		mode_timing[idx].usCRTC_H_Disp = *x;
		mode_timing[idx].usCRTC_V_Disp = *y;
	}
	
	*x = mode_timing[idx + 1].usCRTC_H_Disp;
	*y = mode_timing[idx + 1].usCRTC_V_Disp;
	
	return TRUE;
}

bool ati_set_mode_2(vbios_map* map, UInt8 idx, UInt32* x, UInt32* y)
{
	ATOM_DTD_FORMAT *mode_timing = (ATOM_DTD_FORMAT *) map->mode_table;
	
	if ((*x != 0) && (*y != 0) && ( mode_timing[idx].usHActive >= 640 )) {
		
		PRINT("Mode %dx%d -> %dx%d\n", mode_timing[idx].usHActive, mode_timing[idx].usHActive,
			   *x, *y);
		
		mode_timing[idx].usHActive = *x;
		mode_timing[idx].usVActive = *y;
	}
	
	*x = mode_timing[idx + 1].usHActive;
	*y = mode_timing[idx + 1].usVActive;
	
	return TRUE;
}
