/*
 *  edid.c
 *  
 *
 *  Created by Evan Lojewski on 12/1/09.
 *  Copyright 2009. All rights reserved.
 *
 */


#include "libsaio.h"
#include "edid.h"
#include "vbe.h"
#include "bootstruct.h"


//static biosBuf_t bb;

UInt32 xResolution = 0;
UInt32 yResolution = 0;
UInt32 bpResolution = 0;


void getResolution(UInt32* x, UInt32* y, UInt32* bp)
{
	unsigned char* edidInfo = readEDID();
	
	if(!edidInfo) {
		xResolution = 1024;
		yResolution = 768;
		bpResolution = 32;
		
		free( edidInfo );
	} else {
		// TODO: check *all* resolutions reported and eithe ruse the highest, or the native resolution (if there is a flag for that)
		xResolution =  edidInfo[56] | ((edidInfo[58] & 0xF0) << 4);
		yResolution = edidInfo[59] | ((edidInfo[61] & 0xF0) << 4);
		
		bpResolution = 32;	// assume 32bits
		
		free( edidInfo );
	}
	
	if (!xResolution) xResolution = 1024;
	if (!yResolution) yResolution = 768;
	
	*x  = xResolution;
	*y  = yResolution;
	*bp = bpResolution;

}


unsigned char* readEDID()
{
	SInt16 last_reported = -1;
	UInt8 edidInfo[EDID_BLOCK_SIZE];

	//UInt8 pointer;

	UInt8 header1[] = {0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};
	UInt8 header2[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	
	int status;
	unsigned int blocks_left = 1;
	
	do
	{
		// TODO: This currently only retrieves the *last* block, make the block buffer expand as needed / calculated from the first block

		bzero( edidInfo, EDID_BLOCK_SIZE);

		status = getEDID(edidInfo, blocks_left);
		
				//printf("Buffer location: 0x%X\n", SEG(buffer) << 16 | OFF(buffer));

		/*
		int j, i;
		for (j = 0; j < 8; j++) {
			for(i = 0; i < 16; i++) printf("0x%X ", ebiosInfo[((i+1) * (j + 1)) - 1]);

		}
		printf("\n");
		*/
		
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
					
					printf("EDID claims %d more blocks left\n", reported);
				}
				
				if ( (last_reported <= reported && last_reported != -1)
					|| reported == 0xff
					/* 0xff frequently comes up in corrupt edids */
					//|| reported == MAGIC
					)
				{
					printf("Last reported %d\n", last_reported);
					printf( "EDID blocks left is wrong.\n"
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
				printf("Invalid block %d\n", blocks_left);
				printf("Header1 = %d", memcmp(edidInfo, header1, sizeof(header1)) );
				printf("Header2 = %d", memcmp(edidInfo, header2, sizeof(header2)) );
				return 0;
			}
		} else {
			return 0;
		}

		
		blocks_left = 0;	
	} while(blocks_left);

	UInt8* ret = malloc(sizeof(edidInfo));
	memcpy(ret, edidInfo, sizeof(edidInfo));
	return (ret);
}
