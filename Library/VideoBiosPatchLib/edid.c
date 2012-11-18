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
#include "vbe.h"
#include "graphics.h"
#include "boot.h"
//----------------------------------------------------------------------------------

#define FBMON_FIX_HEADER 1
#define FBMON_FIX_INPUT  2
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

//----------------------------------------------------------------------------------
/*
struct broken_edid {
	const char manufacturer[4];
	UInt32 model;
	UInt32 fix;
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
const unsigned char edid_v1_header[] = { 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00	};

//----------------------------------------------------------------------------------
int edid_compare(unsigned char *edid1, unsigned char *edid2)
{
	int result = 0;
	unsigned char *block = edid1 + ID_MANUFACTURER_NAME, manufacturer1[4], manufacturer2[4];;
	manufacturer1[0] = ((block[0] & 0x7c) >> 2) + '@';
	manufacturer1[1] = ((block[0] & 0x03) << 3) + ((block[1] & 0xe0) >> 5) + '@';
	manufacturer1[2] = (block[1] & 0x1f) + '@';
	manufacturer1[3] = 0;
	
	block = edid2 + ID_MANUFACTURER_NAME;
	manufacturer2[0] = ((block[0] & 0x7c) >> 2) + '@';
	manufacturer2[1] = ((block[0] & 0x03) << 3) + ((block[1] & 0xe0) >> 5) + '@';
	manufacturer2[2] = (block[1] & 0x1f) + '@';
	manufacturer2[3] = 0;
	int x;
	for(x = 0; x < 4; x++)
	{
		if(manufacturer1[x] == manufacturer2[x])
			result++;
	}
	
	return result;
}

int check_edid(unsigned char *edid)
{
	unsigned char *block = edid + ID_MANUFACTURER_NAME, manufacturer[4];
	//unsigned char *b;
	UInt32 model;
	//int i, fix = 0, ret = 0;
	
	manufacturer[0] = ((block[0] & 0x7c) >> 2) + '@';
	manufacturer[1] = ((block[0] & 0x03) << 3) +
	((block[1] & 0xe0) >> 5) + '@';
	manufacturer[2] = (block[1] & 0x1f) + '@';
	manufacturer[3] = 0;
	model = block[2] + (block[3] << 8);
/*	
	for (i = 0; i < (int)ARRAY_SIZE(brokendb); i++) {
		if (!strncmp((const char *)manufacturer, brokendb[i].manufacturer, 4) &&
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

static void fix_edid(unsigned char *edid, int fix)
{
	unsigned char *b;
	
	switch (fix) {
		case FBMON_FIX_HEADER:
			msglog("EDID: trying a header reconstruct\n");
			memcpy(edid, edid_v1_header, 8);
			break;
		case FBMON_FIX_INPUT:
			msglog("EDID: trying to fix input type\n");
			b = edid + EDID_STRUCT_DISPLAY;
			b[0] &= ~0x80;
			edid[127] += 0x80;
	}
}

//----------------------------------------------------------------------------------

int edid_checksum(unsigned char *edid)
{
	unsigned char i, csum = 0, all_null = 0;
	int err = 0, fix = check_edid(edid);
	
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

static int edid_check_header(unsigned char *edid)
{
	int i, err = 1, fix = check_edid(edid);
	
	if (fix)
		fix_edid(edid, fix);
	
	for (i = 0; i < 8; i++) {
		if (edid[i] != edid_v1_header[i])
			err = 0;
	}
	
	return err;
}
//------------------------------------------------------------------------
bool verifyEDID(unsigned char *edid)
{
	if (edid == NULL || !edid_checksum(edid) ||	!edid_check_header(edid)) 
	{
		return false;
	}
	return true;
}

int edid_is_timing_block(unsigned char *block)
{
	if ((block[0] != 0x00) || (block[1] != 0x00) ||
		(block[2] != 0x00) || (block[4] != 0x00))
		return 1;
	else
		return 0;
}
//----------------------------------------------------------------------------------

int fb_parse_edid(struct EDID *edid, edid_mode* var)  //(struct EDID *edid, UInt32* x, UInt32* y)
{
	int i;
	unsigned char *block;
	
	if(!verifyEDID((unsigned char *)edid)) return 1;
	
	block = (unsigned char *)edid + DETAILED_TIMING_DESCRIPTIONS_START; //54
	
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

void getResolution(UInt32* x, UInt32* y, UInt32* bp)
{
//	int val;
	static UInt32 xResolution, yResolution, bpResolution;
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

	
	if(!xResolution || !yResolution || !bpResolution)
	{
		
		char* edidInfo = readEDID();
		
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
		
		//msglog("H Active = %d ", edidInfo[56] | ((edidInfo[58] & 0xF0) << 4) );
		//msglog("V Active = %d \n", edidInfo[59] | ((edidInfo[61] & 0xF0) << 4) );
		
		free( edidInfo );
		
		//if(!xResolution) xResolution = DEFAULT_SCREEN_WIDTH;
		//if(!yResolution) yResolution = DEFAULT_SCREEN_HEIGHT;

	}

	*x  = xResolution;
	*y  = yResolution;
	*bp = bpResolution;

}

char* readEDID()
{
	SInt16 last_reported = -1;
	UInt8 edidInfo[EDID_BLOCK_SIZE];

	UInt8 header1[] = {0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};
	UInt8 header2[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	
	SInt16 status;
	UInt16 blocks_left = 1;
//	msglog("readEDID\n");
	do
	{
		// TODO: This currently only retrieves the *last* block, make the block buffer expand as needed / calculated from the first block

		bzero( edidInfo, EDID_BLOCK_SIZE);

		status = getEDID(edidInfo, blocks_left);
		
		
		msglog("Buffer location: 0x%X status: %d\n", SEG(edidInfo) << 16 | OFF(edidInfo), status);
		
		int j, i;
		for (j = 0; j < 8; j++) {
			for(i = 0; i < 16; i++) msglog("0x%02X ", edidInfo[((i+1) * (j + 1)) - 1]);
			msglog("\n");
		}
		
		
		
		if(status == 0)
		{
			//if( edidInfo[0] == 0x00 || edidInfo[0] == 0xFF)
			if((memcmp(edidInfo, header1, sizeof(header1)) != 0) ||
			   (memcmp(edidInfo, header2, sizeof(header2)) != 0) )
			{
				blocks_left--;
				int reported = edidInfo[ EDID_V1_BLOCKS_TO_GO_OFFSET ];
				
				if ( reported > blocks_left )
				{
					
					msglog("EDID claims %d more blocks left\n", reported);
				}
				
				if ( (last_reported <= reported && last_reported != -1)
					|| reported == 0xff
					/* 0xff frequently comes up in corrupt edids */
					//|| reported == MAGIC
					)
				{
					msglog("Last reported %d\n", last_reported);
					msglog( "EDID blocks left is wrong.\n"
						   "Your EDID is probably invalid.\n");
					return 0;
				}
				else
				{
					//printf("Reading EDID block\n");
					//printf("H Active = %d", ebiosInfo[56] | ((ebiosInfo[58] & 0xF0) << 4) );
					//printf("V Active = %d", ebiosInfo[59] | ((ebiosInfo[61] & 0xF0) << 4) );

					last_reported = reported;
					blocks_left = reported;
				}
			} 
			else
			{
				msglog("Invalid block %d\n", blocks_left);
				msglog("Header1 = %d", memcmp(edidInfo, header1, sizeof(header1)) );
				msglog("Header2 = %d", memcmp(edidInfo, header2, sizeof(header2)) );
				return 0;
			}
		}
		blocks_left = 0;	
	} while(blocks_left);

	char* ret = malloc(sizeof(edidInfo));
	memcpy(ret, edidInfo, sizeof(edidInfo));
	return ret;
}


int getEDID( void * edidBlock, UInt8 block)
{
	biosBuf_t bb;
	
	bzero(&bb, sizeof(bb));
    bb.intno  = 0x10;
    bb.eax.rr = 0x4F15;
	bb.ebx.r.l= 0x01;
	bb.edx.rr = block;
	
    bb.es     = SEG( edidBlock );
    bb.edi.rr = OFF( edidBlock );
	
    bios( &bb );
    return(bb.eax.r.h);
}
