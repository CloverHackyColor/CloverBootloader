/*
 *  edid.c
 *  
 *
 *  Created by Evan Lojewski on 12/1/09.
 *  Copyright 2009. All rights reserved.
 *  
 *	Slice 2010, based on Joblo works
 */


//#include "libsaio.h"
#include "edid.h"
//#include "vbe.h"
//#include "graphics.h"
//#include "boot.h"
//----------------------------------------------------------------------------------

#define FBMON_FIX_HEADER 1
#define FBMON_FIX_INPUT  2
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define DEFAULT_SCREEN_WIDTH 1024
#define DEFAULT_SCREEN_HEIGHT 600

//#include <Protocol/EdidActive.h>
#include <Protocol/EdidDiscovered.h>

/*UINTN CompareMem (
                  IN VOID* Buffer1,
                  IN VOID* Buffer2,
                  IN UINTN Size
                  )
{
  UINT8* p1 = (UINT8*)Buffer1;
  UINT8* p2 = (UINT8*)Buffer2;
  for (UINTN i=0; i<Size; i++)
  {
    if (*p1++!=*p2++)
      return 1;
  }
  return 0;
}*/

//----------------------------------------------------------------------------------
/*
struct broken_edid {
	const CHAR8 manufacturer[4];
	UINT32 model;
	UINT32 fix;
};

//----------------------------------------------------------------------------------

broken_edid brokendb[] = {
	// DEC FR-PCXAV-YZ *
	{ "DEC", 0x073a, FBMON_FIX_HEADER,},
	// ViewSonic PF775a *
	{ "VSC", 0x5a44, FBMON_FIX_INPUT,	}
};
//----------------------------------------------------------------------------------
*/
const UINT8 edid_v1_header[] = { 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00	};

//----------------------------------------------------------------------------------
INT32 edid_compare(UINT8 *edid1, UINT8 *edid2)
{
	INT32 result = 0;
	UINT8 *block = edid1 + ID_MANUFACTURER_NAME, manufacturer1[4], manufacturer2[4];;
	manufacturer1[0] = ((block[0] & 0x7c) >> 2) + '@';
	manufacturer1[1] = ((block[0] & 0x03) << 3) + ((block[1] & 0xe0) >> 5) + '@';
	manufacturer1[2] = (block[1] & 0x1f) + '@';
	manufacturer1[3] = 0;
	
	block = edid2 + ID_MANUFACTURER_NAME;
	manufacturer2[0] = ((block[0] & 0x7c) >> 2) + '@';
	manufacturer2[1] = ((block[0] & 0x03) << 3) + ((block[1] & 0xe0) >> 5) + '@';
	manufacturer2[2] = (block[1] & 0x1f) + '@';
	manufacturer2[3] = 0;
	INT32 x;
	for(x = 0; x < 4; x++)
	{
		if(manufacturer1[x] == manufacturer2[x])
			result++;
	}
	
	return result;
}

INT32 check_edid(UINT8 *edid)
{
	UINT8 *block = edid + ID_MANUFACTURER_NAME, manufacturer[4];
	//UINT8 *b;
	UINT32 model;
	//INT32 i, fix = 0, ret = 0;
	
	manufacturer[0] = ((block[0] & 0x7c) >> 2) + '@';
	manufacturer[1] = ((block[0] & 0x03) << 3) +
	((block[1] & 0xe0) >> 5) + '@';
	manufacturer[2] = (block[1] & 0x1f) + '@';
	manufacturer[3] = 0;
	model = block[2] + (block[3] << 8);
/*	
	for (i = 0; i < (int)ARRAY_SIZE(brokendb); i++) {
		if (!strncmp((const CHAR8 *)manufacturer, brokendb[i].manufacturer, 4) &&
			brokendb[i].model == model) {
			DEBG("ATIFB: The EDID Block of "
				 "Manufacturer: %s Model: 0x%08lx is known to "
				 "be broken,\n",  manufacturer, model);
			fix = brokendb[i].fix;
			break;
		}
	}
	
	switch (fix) {
		case FBMON_FIX_HEADER:
			for (i = 0; i < 8; i++) {
				if (edid[i] != edid_v1_header[i])
					ret = fix;
			}
			break;
		case FBMON_FIX_INPUT:
			b = edid + EDID_STRUCT_DISPLAY;
			/// Only if display is GTF capable will
			 //the input type be reset to analog *
			if (b[4] & 0x01 && b[0] & 0x80)
				ret = fix;
			break;
	}
*/	
	return 0; //ret;
}

//----------------------------------------------------------------------------------

static VOID fix_edid(UINT8 *edid, INT32 fix)
{
	UINT8 *b;
	
	switch (fix) {
		case FBMON_FIX_HEADER:
			AsciiPrint("EDID: trying a header reconstruct\n");
			CopyMem(edid, edid_v1_header, 8);
			break;
		case FBMON_FIX_INPUT:
			AsciiPrint("EDID: trying to fix input type\n");
			b = edid + EDID_STRUCT_DISPLAY;
			b[0] &= ~0x80;
			edid[127] += 0x80;
	}
}

//----------------------------------------------------------------------------------

INT32 edid_checksum(UINT8 *edid)
{
	UINT8 i, csum = 0, all_null = 0;
	INT32 err = 0, fix = check_edid(edid);
	
	if (fix)
		fix_edid(edid, fix);
	
	for (i = 0; i < EDID_LENGTH; i++) {
		csum += edid[i];
		all_null |= edid[i];
	}
	
	if (csum == 0x00 && all_null) {
		/* checksum passed, everything's good */
		err = 1;
	}
	
	return err;
}

//----------------------------------------------------------------------------------

static INT32 edid_check_header(UINT8 *edid)
{
	INT32 i, err = 1, fix = check_edid(edid);
	
	if (fix)
		fix_edid(edid, fix);
	
	for (i = 0; i < 8; i++) {
		if (edid[i] != edid_v1_header[i])
			err = 0;
	}
	
	return err;
}
//------------------------------------------------------------------------
BOOLEAN verifyEDID(UINT8 *edid)
{
	if (edid == NULL || !edid_checksum(edid) ||	!edid_check_header(edid)) 
	{
		return FALSE;
	}
	return TRUE;
}

INT32 edid_is_timing_block(UINT8 *block)
{
	if ((block[0] != 0x00) || (block[1] != 0x00) ||
		(block[2] != 0x00) || (block[4] != 0x00))
		return 1;
	else
		return 0;
}
//----------------------------------------------------------------------------------

INT32 fb_parse_edid(struct EDID *edid, edid_mode* var)  //(struct EDID *edid, UINT32* x, UINT32* y)
{
	INT32 i;
	UINT8 *block;
	
	if(!verifyEDID((UINT8 *)edid)) return 1;
	
	block = (UINT8 *)edid + DETAILED_TIMING_DESCRIPTIONS_START; //54
	
	for (i = 0; i < 4; i++, block += DETAILED_TIMING_DESCRIPTION_SIZE) {
		if (edid_is_timing_block(block)) {
			var->h_active = H_ACTIVE;
			var->v_active = V_ACTIVE;
			var->h_sync_offset = H_SYNC_OFFSET;
			var->h_sync_width = H_SYNC_WIDTH;
			var->h_blanking = H_BLANKING;
			var->v_blanking = V_BLANKING;
			var->pixel_clock = PIXEL_CLOCK;
			var->h_sync_width = H_SYNC_WIDTH;
			var->v_sync_width = V_SYNC_WIDTH;
			/*
			var->xres = var->xres_virtual = H_ACTIVE;
			var->yres = var->yres_virtual = V_ACTIVE;
			var->height = var->width = -1;
			var->right_margin = H_SYNC_OFFSET;
			var->left_margin = (H_ACTIVE + H_BLANKING) -
			(H_ACTIVE + H_SYNC_OFFSET + H_SYNC_WIDTH);
			var->upper_margin = V_BLANKING - V_SYNC_OFFSET -
			V_SYNC_WIDTH;
			var->lower_margin = V_SYNC_OFFSET;
			var->hsync_len = H_SYNC_WIDTH;
			var->vsync_len = V_SYNC_WIDTH;
			var->pixclock = PIXEL_CLOCK;
			var->pixclock /= 1000;
			var->pixclock = KHZ2PICOS(var->pixclock);
			
			if (HSYNC_POSITIVE)
				var->sync |= FB_SYNC_HOR_HIGH_ACT;
			if (VSYNC_POSITIVE)
				var->sync |= FB_SYNC_VERT_HIGH_ACT;
			 */
			return 1;
		}
	}
	return 0;
}

VOID getResolution(UINT32* x, UINT32* y, UINT32* bp)
{
//	INT32 val;
  /**x = 1600;
  *y = 900;
  *bp = 32;
  return;*/
  
	static UINT32 xResolution, yResolution, bpResolution;
/*
	if(getIntForKey(kScreenWidth, &val, &bootInfo->chameleonConfig))
	{
		xResolution = val;
	}
	
	if(getIntForKey(kScreenHeight, &val, &bootInfo->chameleonConfig))
	{
		yResolution = val;
	}
*/
	bpResolution = 32;	// assume 32bits

	
			
		CHAR8* edidInfo = readEDID();
		
		if(!edidInfo) return;
		edid_mode mode;
		// TODO: check *all* resolutions reported and either use the highest, or the native resolution (if there is a flag for that)
		//xResolution =  edidInfo[56] | ((edidInfo[58] & 0xF0) << 4);  
		//yResolution = edidInfo[59] | ((edidInfo[61] & 0xF0) << 4); 
		//Slice - done here
		
		if(fb_parse_edid((struct EDID *)edidInfo, &mode) == 0)
		{
			xResolution = DEFAULT_SCREEN_WIDTH;
			yResolution = DEFAULT_SCREEN_HEIGHT;
		}
		else {
			xResolution = mode.h_active;
			yResolution = mode.v_active;
		}

		/*
		 0x00 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0x00 0x32 0x0C
		 0x00 0xDF 0x00 0x00 0x00 0x00 0xFF 0xFF 0xFF 0x00
		 0x0C 0xDF 0x00 0x00 0x12 0x03 0x21 0x78 0xE9 0x99 
		 0x53 0x28 0xFF 0xFF 0x32 0xDF 0x00 0x12 0x80 0x78 
		 0xD5 0x53 0x26 0x00 0x01 0x01 0x01 0x01 0xFF 0x00 
		 0xDF 0x00 0x03 0x78 0x99 0x28 0x00 0x01 0x01 0x01 
		 0x01 0x21 0x84 0x20 0xFF 0x0C 0x00 0x03 0x0A 0x53 
		 0x54 0x01 0x01 0x01 0xDE 0x84 0x56 0x00 0xA0 0x30
		 0xFF 0xDF 0x12 0x78 0x53 0x00 0x01 0x01 0x01 0x84 
		 0x00 0x18 0x84 0x00 0x00 0x57 0xFF 0x00 0x80 0x99
		 0x54 0x01 0x01 0x21 0x20 0x00 0x50 0x00 0x00 0x35
		 0x57 0xFE 0x00 0x00 0x78 0x28 0x01 0x01 0x21 0x20
		 0x18 0x30 0x00 0x57 0x34 0xFE 0xAA 0x9A 

		 */
		
		//AsciiPrint("H Active = %d ", edidInfo[56] | ((edidInfo[58] & 0xF0) << 4) );
		//AsciiPrint("V Active = %d \n", edidInfo[59] | ((edidInfo[61] & 0xF0) << 4) );
		
		FreePool( edidInfo );
		
		//if(!xResolution) xResolution = DEFAULT_SCREEN_WIDTH;
		//if(!yResolution) yResolution = DEFAULT_SCREEN_HEIGHT;

	

	*x  = xResolution;
	*y  = yResolution;
	*bp = bpResolution;
  
  AsciiPrint("Best mode: %dx%dx%d\n", *x, *y, *bp);
}

CHAR8* readEDID()
{
  EFI_STATUS    Status;
  EFI_EDID_DISCOVERED_PROTOCOL    *EdidDiscovered;
  Status = gBS->LocateProtocol (&gEfiEdidDiscoveredProtocolGuid, NULL, (VOID**)&EdidDiscovered);
  if (!EFI_ERROR(Status))
  {
    AsciiPrint("EdidDiscovered size=%d\n", EdidDiscovered->SizeOfEdid);
    if (EdidDiscovered->SizeOfEdid==0)
      return NULL;
    CHAR8* ret = AllocatePool(EdidDiscovered->SizeOfEdid);
    CopyMem(ret, EdidDiscovered->Edid, EdidDiscovered->SizeOfEdid);
    return ret;
  }
  return NULL;
}
