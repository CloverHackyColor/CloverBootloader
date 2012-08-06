
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

#include "libsaio.h"
#include "autoresolution.h"
#include "nvidia_resolution.h"
#include "ati_resolution.h"
#include "gma_resolution.h"
#include "../boot2/graphics.h"

char * chipset_type_names[] = {
	"UNKNOWN", "830",  "845G", "855GM", "865G", "915G", "915GM", "945G", "945GM", "945GME",
	"946GZ",   "955X", "G965", "Q965", "965GM", "975X",
	"P35", "X48", "B43", "Q45", "P45", "GM45", "G41", "G31", "G45", "500"
};

UInt32 get_chipset_id(void) {
	outl(0xcf8, 0x80000000);
	return inl(0xcfc);
}

chipset_type get_chipset(UInt32 id) {
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
			
		case 0x27748086:
			type = CT_955X;
			break;
			
		case 0x277c8086:
			type = CT_975X;
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
			
		case 0x29a08086:
			type = CT_G965;
			break;
			
		case 0x29908086:
			type = CT_Q965;
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
			
		case 0x81008086:
			type = CT_500;
			break;
			
		default:
			type = CT_UNKWN;
			break;
	}
	return type;
}


void gtf_timings(UInt32 x, UInt32 y, UInt32 freq,
				 unsigned long *clock,
				 UInt16 *hsyncstart, UInt16 *hsyncend, UInt16 *hblank,
				 UInt16 *vsyncstart, UInt16 *vsyncend, UInt16 *vblank)
{
	UInt32 hbl, vbl, vfreq;
	
	vbl = y + (y+1)/(20000/(11*freq) - 1) + 1;
	
	vfreq = vbl * freq;
	hbl = 16 * (int)(x * (30 - 300000 / vfreq) /
					 +            (70 + 300000 / vfreq) / 16 + 0);
	
	*vsyncstart = y;
	*vsyncend = y + 3;
	*vblank = vbl;	
	*hsyncstart = x + hbl / 2 - (x + hbl + 50) / 100 * 8 ;	
	*hsyncend = x + hbl / 2;	
	*hblank = x + hbl;	
	*clock = (x + hbl) * vfreq / 1000;
}


void get_aspect_ratio(s_aspect* aspect, UInt32 x, UInt32 y)
{
	if ((y * 16 / 9) == x) {
		aspect->width  = 16;
		aspect->height = 9;
	} else if ((y * 16 / 10) == x) {
		aspect->width  = 16;
		aspect->height = 10;
	} else if ((y * 5 / 4) == x) {
		aspect->width  = 5;
		aspect->height = 4;
	} else if ((y * 15 / 9) == x) {
		aspect->width  = 15;
		aspect->height = 9;
	} else {
		aspect->width  = 4;
		aspect->height = 3;
	}
	PRINT("Aspect Ratio is %d/%d\n", aspect->width, aspect->height);
}

void cvt_timings(UInt32 x, UInt32 y, UInt32 freq,
				 unsigned long *clock,
				 UInt16 *hsyncstart, UInt16 *hsyncend, UInt16 *hblank,
				 UInt16 *vsyncstart, UInt16 *vsyncend, UInt16 *vblank, bool reduced)
{
	UInt32 hbl, hbp, vbl, vsync, hperiod;
	
	if (!(y % 3) && ((y * 4 / 3) == x))
        vsync = 4;
    else if (!(y % 9) && ((y * 16 / 9) == x))
        vsync = 5;
    else if (!(y % 10) && ((y * 16 / 10) == x))
        vsync = 6;
    else if (!(y % 4) && ((y * 5 / 4) == x))
        vsync = 7;
    else if (!(y % 9) && ((y * 15 / 9) == x))
        vsync = 7;
    else /* Custom */
        vsync = 10;
	
	if (!reduced) {
		hperiod = (1000000/freq - 550) / (y + 3);
		vbl = y + (550/hperiod) + 3;
		hbp = 30 - ((300*hperiod)/1000);
		hbl = (x * hbp) / (100 - hbp);
		
		*vsyncstart = y + 6;
		*vsyncend = *vsyncstart + vsync;
		*vblank = vbl - 1;	
		*hsyncstart = x + hbl / 2 - (x + hbl + 50) / 100 * 8 - 1;	
		*hsyncend = x + hbl / 2 - 1;	
		*hblank = x + hbl - 1;
		
	} else {
		hperiod = (1000000/freq - 460) / y;
		vbl = y + 460/hperiod + 1;
		hbl = 160;
		
		*vsyncstart = y + 3;
		*vsyncend = *vsyncstart + vsync;
		*vblank = vbl - 1;	
		*hsyncstart = x + hbl / 2 - 32;	
		*hsyncend = x + hbl / 2 - 1;	
		*hblank = x + hbl - 1;
		
	}
	*clock = (x + hbl) * 1000 / hperiod;
}



void close_vbios(vbios_map * map);

vbios_map * open_vbios(chipset_type forced_chipset) {
	UInt32 z;
	vbios_map * map = NEW(vbios_map);
	for(z=0; z<sizeof(vbios_map); z++) ((char*)map)[z]=0;
	
	/*
	 * Determine chipset
	 */
	
	if (forced_chipset == CT_UNKWN) {
		map->chipset_id = get_chipset_id();
		map->chipset = get_chipset(map->chipset_id);
		PRINT("Chipset is %s (pci id 0x%x)\n",chipset_type_names[map->chipset], map->chipset_id);
	}
	else if (forced_chipset != CT_UNKWN) {
		map->chipset = forced_chipset;
	}
	else {
		map->chipset = CT_915GM;
	}
	    
	/*
	 *  Map the video bios to memory
	 */
	
	map->bios_ptr=(unsigned char*)VBIOS_START;
	
	/*
	 * check if we have ATI Radeon and open atombios
	 */
	bios_tables_t ati_tables;
	
	ati_tables.base = map->bios_ptr;
	ati_tables.AtomRomHeader = (ATOM_ROM_HEADER *) (map->bios_ptr + *(unsigned short *) (map->bios_ptr + OFFSET_TO_POINTER_TO_ATOM_ROM_HEADER)); 
	if (strcmp ((char *) ati_tables.AtomRomHeader->uaFirmWareSignature, "ATOM") == 0) {
		map->bios = BT_ATI_1;
		PRINT("We have an AtomBios Card\n");
		return open_ati_vbios(map, ati_tables);
	}


	/*
	 * check if we have NVidia
	 */
	if (map->bios != BT_ATI_1) {
		int i = 0;
		while (i < 512) { // we don't need to look through the whole bios, just the firs 512 bytes
			if ((map->bios_ptr[i] == 'N') 
				&& (map->bios_ptr[i+1] == 'V') 
				&& (map->bios_ptr[i+2] == 'I') 
				&& (map->bios_ptr[i+3] == 'D')) 
			{
				map->bios = BT_NVDA;
				PRINT("We have an NVIDIA Card\n");
				return open_nvidia_vbios(map);
				break;
			}
			i++;
		}
	}
	
	/*
	 * check if we have Intel
	 */
	    
	if ((map->bios != BT_ATI_1) && (map->bios != BT_NVDA)) {
		int i = 0;
		while (i < VBIOS_SIZE) { // we don't need to look through the whole bios, just the firs 512 bytes
			if ((map->bios_ptr[i] == 'I') 
				&& (map->bios_ptr[i+1] == 'n') 
				&& (map->bios_ptr[i+2] == 't') 
				&& (map->bios_ptr[i+3] == 'e') 
				&& (map->bios_ptr[i+4] == 'l')) 
			{
				map->bios = BT_1;
				PRINT("We have an Intel Card\n");
				return open_intel_vbios(map);
				break;
			}
			i++;
		}
	}
	
	/*
	 * Unidentified Chipset
	 */
	
	if ( (map->chipset == CT_UNKWN) || ((map->bios != BT_ATI_1) && (map->bios != BT_NVDA) && (map->bios != BT_1)) )
			{
		PRINT("Unknown chipset type and unrecognized bios.\n");
					
		PRINT("autoresolution only works with Intel 800/900 series graphic chipsets.\n");
					
		PRINT("Chipset Id: %x\n", map->chipset_id);
		close_vbios(map);
		return 0;
	}

	/*
	 * Should never get there 
	 */
	return 0;
}

void close_vbios(vbios_map * map) {
	if (autoResolution == TRUE) autoResolution = FALSE;
	FREE(map);
}

void unlock_vbios(vbios_map * map) {

	map->unlocked = TRUE;
	    
	switch (map->chipset) {
		case CT_UNKWN:
			break;
		case CT_830:
		case CT_855GM:
			outl(0xcf8, 0x8000005a);
			map->b1 = inb(0xcfe);
				
			outl(0xcf8, 0x8000005a);
			outb(0xcfe, 0x33);
			break;
		case CT_845G:
		case CT_865G:
		case CT_915G:
		case CT_915GM:
		case CT_945G:
		case CT_945GM:
		case CT_945GME:
		case CT_946GZ:
		case CT_955X:
		case CT_G965:
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
		case CT_G45:
		case CT_500:

			outl(0xcf8, 0x80000090);
			map->b1 = inb(0xcfd);
			map->b2 = inb(0xcfe);
			outl(0xcf8, 0x80000090);
			outb(0xcfd, 0x33);
			outb(0xcfe, 0x33);
		break;
	}
	
	#if DEBUG
	{
		UInt32 t = inl(0xcfc);
		PRINT("unlock PAM: (0x%08x)\n", t);
	}
#endif
}

void relock_vbios(vbios_map * map) {

	map->unlocked = FALSE;
	
	switch (map->chipset) {
		case CT_UNKWN:
			break;
		case CT_830:
		case CT_855GM:
			outl(0xcf8, 0x8000005a);
			outb(0xcfe, map->b1);
			break;
		case CT_845G:
		case CT_865G:
		case CT_915G:
		case CT_915GM:
		case CT_945G:
		case CT_945GM:
		case CT_945GME:
		case CT_946GZ:
		case CT_955X:
		case CT_G965:
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
		case CT_G45:
		case CT_500:
			
			outl(0xcf8, 0x80000090);
			outb(0xcfd, map->b1);
			outb(0xcfe, map->b2);
			break;
	}
	
	#if DEBUG
	{
        UInt32 t = inl(0xcfc);
		PRINT("relock PAM: (0x%08x)\n", t);
	}
	#endif
}


void save_vbios(vbios_map * map)
{
	map->bios_backup_ptr = malloc(VBIOS_SIZE);
	bcopy((const unsigned char *)0xC0000, map->bios_backup_ptr, VBIOS_SIZE);
}

void restore_vbios(vbios_map * map)
{
	bcopy(map->bios_backup_ptr,(unsigned char *)0xC0000, VBIOS_SIZE);
}


void patch_vbios(vbios_map * map, UInt32 x, UInt32 y, UInt32 bp, UInt32 htotal, UInt32 vtotal) {
	UInt32 i = 0;
	
	/*
	 * Get the aspect ratio for the requested mode
	 */
	get_aspect_ratio(&map->aspect_ratio, x, y);
	
	i = x = y = 0;
	
	if (map->bios != BT_NVDA) {
		PRINT("%d modes to patch\n", map->modeline_num);
		switch (map->bios) {
			case BT_1:
		while (i < map->modeline_num) {
			if (x == 1400) x = 1440;
			if (x == 1600) x = 1680;
			
			y = x * map->aspect_ratio.height / map->aspect_ratio.width;
					intel_set_mode_1(map, i, &x, &y);
					i++;
				}
					break;
				case BT_2:
				while (i < map->modeline_num) {
					if (x == 1400) x = 1440;
					if (x == 1600) x = 1680;
					
					y = x * map->aspect_ratio.height / map->aspect_ratio.width;
					intel_set_mode_2(map, i, &x, &y);
					i++;
				}
					break;
				case BT_3:
				while (i < map->modeline_num) {
					if (x == 1400) x = 1440;
					if (x == 1600) x = 1680;
					
					y = x * map->aspect_ratio.height / map->aspect_ratio.width;
					intel_set_mode_3(map, i, &x, &y);
					i++;
				}
					break;
				case BT_ATI_1:
				while (i < map->modeline_num) {
					if (x == 1400) x = 1440;
					if (x == 1600) x = 1680;
					
					y = x * map->aspect_ratio.height / map->aspect_ratio.width;
					ati_set_mode_1(map, i, &x, &y);
					i++;
				}
					break;
				case BT_ATI_2:
				while (i < map->modeline_num) {
					if (x == 1400) x = 1440;
					if (x == 1600) x = 1680;
					
					y = x * map->aspect_ratio.height / map->aspect_ratio.width;
					ati_set_mode_2(map, i, &x, &y);
					i++;
				}
					break;
				default:
					break;
			}
		return;
	}
	
	if (map->bios == BT_NVDA) {
				
				i = x = y = 0;
				while (i < map->modeline_num) {
						if (x == 0)		x = 1024;
						if (x == 1400)	x = 1440;
						if (x == 1600)	x = 1680;
						
						y = x * map->aspect_ratio.height / map->aspect_ratio.width;
						nvidia_set_mode(map, i, &x, &y, MAIN_VESA_TABLE);
						i++;			
					}
				
				i = x = y = 0;
				while (i < map->nv_modeline_num_2) {
						if (x == 1400) x = 1440;
						if (x == 1600) x = 1680;
						
						y = x * map->aspect_ratio.height / map->aspect_ratio.width;
						nvidia_set_mode(map, i, &x, &y, SECOND_VESA_TABLE);
						i++;
					}
		return;
	}
} 