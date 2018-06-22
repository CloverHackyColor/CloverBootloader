/*
 *  Created by mcmatrix on 08.01.08.
 *  Copyright 2008 mcmatrix All rights reserved.
 *
 */
 
// Constants
#define MAX_FILENAME 255
#define DETECT_NUMBERS 1
//#define NULL (void*)0

typedef enum DATA_TYPES
{
	DATA_INT8	= 1,
	DATA_INT16	= 2,
	DATA_INT32	= 3,
	DATA_BINARY	= 4,
	DATA_STRING	= 5

} DATA_TYPES;

typedef enum FILE_TYPES
{
	FILE_BIN = 1,
	FILE_HEX = 2,
	FILE_XML = 3
	
} FILE_TYPES;

typedef struct SETTINGS
{
	char ifile[MAX_FILENAME];	// input filename
	FILE_TYPES ifile_type;		// input file type
	char ofile[MAX_FILENAME];	// output filename
	FILE_TYPES ofile_type;		// output file type
	int verbose;				// verbose mode
	int detect_strings;			// detect strings from binary data
	int detect_numbers;			// detect numberic data from binary
} SETTINGS;

// gfx main header
typedef struct GFX_HEADER
{
	unsigned int filesize;				// filesize
	unsigned int var1;					// unknown
	unsigned int countofblocks;			// count of datablocks
	struct GFX_BLOCKHEADER * blocks;	// pointer to datablock		
} GFX_HEADER;

// gfx block header
typedef struct GFX_BLOCKHEADER
{
	unsigned int blocksize;			// datablock size
	unsigned int records;			// records count
	struct _EFI_DEVICE_PATH_P_TAG *devpath;	// device address binary
	unsigned int devpath_len;		// device address binary len	
	struct GFX_ENTRY * entries;		// pointer to block entries
	struct GFX_BLOCKHEADER * next;	// pointer to next datablock	 
} GFX_BLOCKHEADER;

// gfx data entries
typedef struct GFX_ENTRY
{
	unsigned char *bkey;	// unicode key binary value
	unsigned int bkey_len;	// binary unicode key length
	char *key;				// ascii key value
	unsigned int key_len;	// ascii key length
	unsigned char *val;		// data binary value
	unsigned int val_len;	// binary data length
	DATA_TYPES val_type;	// binary data type
	struct GFX_ENTRY * next;	
} GFX_ENTRY;

GFX_HEADER *parse_binary(const unsigned char *bp);
//CFDictionaryRef CreateGFXDictionary(GFX_HEADER * gfx);
