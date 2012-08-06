/*
 *  edid.h
 *  
 *
 *  Created by Evan Lojewski on 12/1/09.
 *  Copyright 2009. All rights reserved.
 *
 */

#ifndef _EDID_H
#define _EDID_H

#define EDID_BLOCK_SIZE	128
#define EDID_V1_BLOCKS_TO_GO_OFFSET 126

#define SERVICE_REPORT_DDC	0
#define SERVICE_READ_EDID	1
#define SERVICE_LAST		1  // Read VDIF has been removed from the spec.

#define FUNC_GET_EDID		0x4F15

typedef struct _edid_mode {
	unsigned short pixel_clock;
	unsigned short h_active;
	unsigned short h_blanking;
	unsigned short v_active;
	unsigned short v_blanking;
	unsigned short h_sync_offset;
	unsigned short h_sync_width;
	unsigned short v_sync_offset;
	unsigned short v_sync_width;
}edid_mode;


unsigned char* readEDID();
void getResolution(UInt32* x, UInt32* y, UInt32* bp);

#endif