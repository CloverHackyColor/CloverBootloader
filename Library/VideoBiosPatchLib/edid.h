/*
 *  edid.h
 *  
 *
 *  Created by Evan Lojewski on 12/1/09.
 *  Copyright 2009. All rights reserved.
 *
 * Slice 2010, based on Joblo works
 */
//#ifndef __EDID_H__
//#define __EDID_H__


//#include "libsaio.h"

#define EDID_BLOCK_SIZE	128
#define EDID_V1_BLOCKS_TO_GO_OFFSET 126
//Slice - some more info about EDID
#define EDID_LENGTH				0x80
#define EDID_HEADER				0x00
#define EDID_HEADER_END				0x07

#define ID_MANUFACTURER_NAME			0x08
#define ID_MANUFACTURER_NAME_END		0x09
#define ID_MODEL				0x0a

#define ID_SERIAL_NUMBER			0x0c

#define MANUFACTURE_WEEK			0x10
#define MANUFACTURE_YEAR			0x11

#define EDID_STRUCT_VERSION			0x12
#define EDID_STRUCT_REVISION			0x13

#define EDID_STRUCT_DISPLAY                     0x14

#define DPMS_FLAGS				0x18
#define ESTABLISHED_TIMING_1			0x23
#define ESTABLISHED_TIMING_2			0x24
#define MANUFACTURERS_TIMINGS			0x25

/* standard timings supported */
#define STD_TIMING                              8
#define STD_TIMING_DESCRIPTION_SIZE             2
#define STD_TIMING_DESCRIPTIONS_START           0x26

#define DETAILED_TIMING_DESCRIPTIONS_START	0x36
#define DETAILED_TIMING_DESCRIPTION_SIZE	18
#define NO_DETAILED_TIMING_DESCRIPTIONS		4

#define DETAILED_TIMING_DESCRIPTION_1		0x36
#define DETAILED_TIMING_DESCRIPTION_2		0x48
#define DETAILED_TIMING_DESCRIPTION_3		0x5a
#define DETAILED_TIMING_DESCRIPTION_4		0x6c

#define DESCRIPTOR_DATA				5

#define UPPER_NIBBLE( x ) \
(((128|64|32|16) & (x)) >> 4)

#define LOWER_NIBBLE( x ) \
((1|2|4|8) & (x))

#define COMBINE_HI_8LO( hi, lo ) \
( (((UINT32)hi) << 8) | (UINT32)lo )

#define COMBINE_HI_4LO( hi, lo ) \
( (((UINT32)hi) << 4) | (UINT32)lo )

#define PIXEL_CLOCK_LO     (UINT32)block[ 0 ]
#define PIXEL_CLOCK_HI     (UINT32)block[ 1 ]
//#define PIXEL_CLOCK	   (COMBINE_HI_8LO( PIXEL_CLOCK_HI,PIXEL_CLOCK_LO )*10)
#define PIXEL_CLOCK	   COMBINE_HI_8LO( PIXEL_CLOCK_HI,PIXEL_CLOCK_LO )
#define H_ACTIVE_LO        (UINT32)block[ 2 ]
#define H_BLANKING_LO      (UINT32)block[ 3 ]
#define H_ACTIVE_HI        UPPER_NIBBLE( (UINT32)block[ 4 ] )
#define H_ACTIVE           COMBINE_HI_8LO( H_ACTIVE_HI, H_ACTIVE_LO )
#define H_BLANKING_HI      LOWER_NIBBLE( (UINT32)block[ 4 ] )
#define H_BLANKING         COMBINE_HI_8LO( H_BLANKING_HI, H_BLANKING_LO )

#define V_ACTIVE_LO        (UINT32)block[ 5 ]
#define V_BLANKING_LO      (UINT32)block[ 6 ]
#define V_ACTIVE_HI        UPPER_NIBBLE( (UINT32)block[ 7 ] )
#define V_ACTIVE           COMBINE_HI_8LO( V_ACTIVE_HI, V_ACTIVE_LO )
#define V_BLANKING_HI      LOWER_NIBBLE( (UINT32)block[ 7 ] )
#define V_BLANKING         COMBINE_HI_8LO( V_BLANKING_HI, V_BLANKING_LO )

#define H_SYNC_OFFSET_LO   (UINT32)block[ 8 ]
#define H_SYNC_WIDTH_LO    (UINT32)block[ 9 ]

#define V_SYNC_OFFSET_LO   UPPER_NIBBLE( (UINT32)block[ 10 ] )
#define V_SYNC_WIDTH_LO    LOWER_NIBBLE( (UINT32)block[ 10 ] )

#define V_SYNC_WIDTH_HI    ((UINT32)block[ 11 ] & (1|2))
#define V_SYNC_OFFSET_HI   (((UINT32)block[ 11 ] & (4|8)) >> 2)

#define H_SYNC_WIDTH_HI    (((UINT32)block[ 11 ] & (16|32)) >> 4)
#define H_SYNC_OFFSET_HI   (((UINT32)block[ 11 ] & (64|128)) >> 6)

#define V_SYNC_WIDTH       COMBINE_HI_4LO( V_SYNC_WIDTH_HI, V_SYNC_WIDTH_LO )
#define V_SYNC_OFFSET      COMBINE_HI_4LO( V_SYNC_OFFSET_HI, V_SYNC_OFFSET_LO )

#define H_SYNC_WIDTH       COMBINE_HI_4LO( H_SYNC_WIDTH_HI, H_SYNC_WIDTH_LO )
#define H_SYNC_OFFSET      COMBINE_HI_4LO( H_SYNC_OFFSET_HI, H_SYNC_OFFSET_LO )

#define H_SIZE_LO          (UINT32)block[ 12 ]
#define V_SIZE_LO          (UINT32)block[ 13 ]

#define H_SIZE_HI          UPPER_NIBBLE( (UINT32)block[ 14 ] )
#define V_SIZE_HI          LOWER_NIBBLE( (UINT32)block[ 14 ] )

#define H_SIZE             COMBINE_HI_8LO( H_SIZE_HI, H_SIZE_LO )
#define V_SIZE             COMBINE_HI_8LO( V_SIZE_HI, V_SIZE_LO )

#define H_BORDER           (UINT32)block[ 15 ]
#define V_BORDER           (UINT32)block[ 16 ]

#define FLAGS              (UINT32)block[ 17 ]

#define INTERLACED         (FLAGS&128)
#define SYNC_TYPE          (FLAGS&3<<3)	/* bits 4,3 */
#define SYNC_SEPARATE      (3<<3)
#define HSYNC_POSITIVE     (FLAGS & 4)
#define VSYNC_POSITIVE     (FLAGS & 2)

#define V_MIN_RATE              block[ 5 ]
#define V_MAX_RATE              block[ 6 ]
#define H_MIN_RATE              block[ 7 ]
#define H_MAX_RATE              block[ 8 ]
#define MAX_PIXEL_CLOCK         (((INT)block[ 9 ]) * 10)
#define GTF_SUPPORT		block[10]

#define DPMS_ACTIVE_OFF		(1 << 5)
#define DPMS_SUSPEND		(1 << 6)
#define DPMS_STANDBY		(1 << 7)

struct EDID
{
    UINT8	header[8];			//0
    UINT8	vendorProduct[4];	//8
    UINT8	serialNumber[4];	//12
    UINT8	weekOfManufacture;	//16
    UINT8	yearOfManufacture;	//17
    UINT8	version;			//18
    UINT8	revision;			//19
    UINT8	displayParams[5];	//20
    UINT8	colorcharacteristics[10];	//25
    UINT8	establishedTimings[3];		//35
    UINT16	standardTimings[8];			//38
    UINT8	detailedTimings[72];		//54
    UINT8	extension;					//126
    UINT8	checksum;					//127
};


typedef struct _edid_mode {
	UINT16 pixel_clock;
	UINT16 h_active;
	UINT16 h_blanking;
	UINT16 v_active;
	UINT16 v_blanking;
	UINT16 h_sync_offset;
	UINT16 h_sync_width;
	UINT16 v_sync_offset;
	UINT16 v_sync_width;
} edid_mode;


CHAR8* readEDID();
VOID getResolution(UINT32* x, UINT32* y, UINT32* bp);
INT32 fb_parse_edid(struct EDID *edid, edid_mode* var);
INT32 getEDID( VOID * edidBlock, UINT8 block);
//#endif
