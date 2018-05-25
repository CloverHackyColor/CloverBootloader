/*
 *  Created by mcmatrix on 07.01.08.
 *  Copyright 2008 mcmatrix. All rights reserved.
 *
 */
 
#include "gfxutil.h"

#define MAX_DEVICE_PATH_LEN 1000

int is_string(void * buffer, int size)
{
  int i;
  
  for(i=0;i < size; i++)
  {
    if(!IS_ALPHANUMMARK( ((unsigned char *)buffer)[i]) ) return 0;
  }
  return 1;
}


static int readbin(unsigned char **data, unsigned int *size, unsigned char **dat, unsigned int len)
{
  unsigned char *d = *data;
  unsigned int s = *size;
  
  if( s != 0 )
  {
    *dat = (unsigned char *)calloc(len, sizeof(unsigned char));
    if (!dat)
    {
      fprintf(stderr, "read_binary: out of memory\n");
      return 0;
    }
    
    if(((unsigned int)(len)) <= s)
    {
      memcpy((*dat),d,len);
      *data = d + len;
      *size = s - len;
      return 1;
    }
  }
  fprintf(stderr, "read_binary: invalid binary data\n");
  return 0;
}

/*
 * Returns zero if the data is badly formatted.
 */
static int uni2str(unsigned char *d, unsigned int length, char **str, unsigned int *len)
{
  unsigned unich;
  
  if(length != 0)
  {
    /* Allocate space for the converted string */
    // Two unicode characters make up 1 buffer byte. Round up
    if((*str = (char *)calloc( length*2 + 1, sizeof(char))) == 0)
    {
      fprintf(stderr, "unicode2str: out of memory\n");
      return 0;
    }
    
    /* Convert the string from Unicode into UTF-8 */
    *len = 0;
    while(length >= 2)
    {
      unich = READ_UINT16(d);
      d += 2;
      if(unich < 0x80)
      {
        (*str)[*len] = (char)unich;
        ++(*len);
      }
      else if(unich < (1 << 11))
      {
        (*str)[*len] = (char)(0xC0 | (unich >> 6));
        ++(*len);
        (*str)[*len] = (char)(0x80 | (unich & 0x3F));
        ++(*len);
      }
      else
      {
        (*str)[*len] = (char)(0xE0 | (unich >> 12));
        ++(*len);
        (*str)[*len] = (char)(0x80 | ((unich >> 6) & 0x3F));
        ++(*len);
        (*str)[*len] = (char)(0x80 | (unich & 0x3F));
        ++(*len);
      }
      length -= 2;
    }
    (*str)[*len] = '\0';
    return 1;
  }
  fprintf(stderr, "unicode2str: invalid binary unicode data\n");
  return 0;
}

unsigned char _nibbleValue(unsigned char hexchar)
{
  unsigned char val;
  
  if(hexchar >= '0' && hexchar <= '9')
    val = hexchar - '0';
  else if(hexchar >= 'A' && hexchar <= 'F')
    val = hexchar - 'A' + 10;
  else if(hexchar >= 'a' && hexchar <= 'f')
    val = hexchar - 'a' + 10;
  else
    val = 0xff;
  return(val);
  
}

// this reads gfx binary info and parses it
GFX_HEADER *parse_binary(CFTypeRef dataRef, SETTINGS settings)
{
	GFX_HEADER *gfx_header = (GFX_HEADER *) NULL;
	// head points to the first node in list, end points to the last node in list
	GFX_BLOCKHEADER *gfx_blockheader = (GFX_BLOCKHEADER *) NULL; 
	GFX_BLOCKHEADER *gfx_blockheader_head = (GFX_BLOCKHEADER *) NULL;
	GFX_BLOCKHEADER *gfx_blockheader_end = (GFX_BLOCKHEADER *) NULL;
	GFX_ENTRY *gfx_entry = (GFX_ENTRY *) NULL; 
	GFX_ENTRY *gfx_entry_head = (GFX_ENTRY *) NULL;
	GFX_ENTRY *gfx_entry_end  = (GFX_ENTRY *) NULL;
	unsigned char *data, *bin, *tmp, *dpathtmp;
	char * str;
	unsigned int str_len, data_len, size, length;	
	int i,j;
  
  unsigned char *bp = NULL;
  CFIndex    dp_length = 0;
  CFTypeID   typeID;
  //  int i;
  
  // Get the OF variable's type.
  typeID = CFGetTypeID(dataRef);
  
  if (typeID == CFDataGetTypeID()) {
    dp_length = CFDataGetLength(dataRef);
    if (dp_length == 0)
      return NULL;
    else
      bp = (unsigned char *)CFDataGetBytePtr(dataRef);
  } else {
    printf("<INVALID> settings\n");
    return NULL;
  }


	//read header data	
	gfx_header = (GFX_HEADER *)calloc(1, sizeof(GFX_HEADER));	
	if(!gfx_header)
	{
		fprintf(stderr, "parse_binary: out of memory\n");
		return NULL;	
	}

	gfx_header->filesize = READ_UINT32(bp);
	bp+=4;
	
	gfx_header->var1 = READ_UINT32(bp);
	bp+=4;
	
	gfx_header->countofblocks = READ_UINT32(bp);		
	bp+=4;

	//read blocks
	gfx_blockheader_head = NULL;
	gfx_blockheader_end = NULL;
	for(i=0;i<gfx_header->countofblocks;i++)
	{
		//create new block
		gfx_blockheader = (GFX_BLOCKHEADER *)calloc(1, sizeof(GFX_BLOCKHEADER));
		if(!gfx_blockheader)
		{
			fprintf(stderr, "parse_binary: out of memory\n");
			return NULL;	
		}
		//read block data
		gfx_blockheader->blocksize = READ_UINT32(bp);
		bp+=4;
	
		gfx_blockheader->records = READ_UINT32(bp);
		bp+=4;
		
		size = gfx_blockheader->blocksize;
		
		tmp = bp;
		
		unsigned int Count;
		// read device path data until devpath end node 0x0004FF7F
		for (Count = 0;;Count++) 
		{
			if(Count > MAX_DEVICE_PATH_LEN)
			{
				// BugBug: Code to catch bogus device path
				fprintf(stderr, "parse_binary: Cannot find device path end! Probably a bogus device path.\n");
				return NULL;
			}				
			if( READ_UINT32(tmp) == 0x0004ff7f || READ_UINT32(tmp) == 0x0004ffff )
			{
				tmp+=4;
				break;
			}		
			tmp++;
		}
		
		// read device path data
		gfx_blockheader->devpath_len = abs((int)tmp - (int)bp);
		readbin(&bp, &size, &dpathtmp,gfx_blockheader->devpath_len);
		gfx_blockheader->devpath = (EFI_DEVICE_PATH *)dpathtmp;		
		
		gfx_entry_head = NULL;
		gfx_entry_end = NULL;
		for(j=1;j <= gfx_blockheader->records;j++)
		{
			length = READ_UINT32(bp);
			length -= 4; bp += 4; size -=4;	
			if(readbin(&bp, &size, &bin, length))
			{
				if(!uni2str(bin, length, &str, &str_len))
				{
					return NULL;
				}
			}
			else
			{
				return NULL;
			}
			
			data_len = READ_UINT32(bp);
			data_len -= 4; bp += 4; size -=4;
			if(!readbin(&bp, &size, &data, data_len))
			{				
				return NULL;
			}	
			
			gfx_entry = (GFX_ENTRY *)calloc(1, sizeof(GFX_ENTRY));			
			if(!gfx_entry)
			{
				fprintf(stderr, "parse_binary: out of memory\n");
				return NULL;	
			}
			//read entries
			gfx_entry->bkey = bin;
			gfx_entry->bkey_len = length;				
			gfx_entry->key = str;
			gfx_entry->key_len = str_len;
			gfx_entry->val_type = DATA_BINARY; // set default data type
			gfx_entry->val = data;
			gfx_entry->val_len = data_len;
			
			if(settings.detect_numbers)	// detect numbers
			{			
				switch(data_len)
				{
					case sizeof(UINT8): // int8
						gfx_entry->val_type = DATA_INT8;
					break;
					case sizeof(UINT16): //int16
						gfx_entry->val_type = DATA_INT16;
					break;
					case sizeof(UINT32): //int32
						gfx_entry->val_type = DATA_INT32;
					break;
					default:
						gfx_entry->val_type = DATA_BINARY;
					break;
				}
			}
			
			// detect strings
			if(settings.detect_strings && is_string(data, data_len) && gfx_entry->val_type == DATA_BINARY)
			{
				gfx_entry->val_type = DATA_STRING;
			}						
			
			if(!gfx_entry_head)							// if there are no nodes in list then
				gfx_entry_head = gfx_entry;				// set head to this new node			
			if(gfx_entry_end)
				gfx_entry_end->next = gfx_entry;		// link in new node to the end of the list
			gfx_entry->next = NULL;						// set next field to signify the end of list
			gfx_entry_end = gfx_entry;					// adjust end to point to the last node
		}
				
		gfx_blockheader->entries = gfx_entry_head;
		
		if(!gfx_blockheader_head)						// if there are no nodes in list then
			gfx_blockheader_head = gfx_blockheader;		// set head to this new node		
		if(gfx_blockheader_end)
			gfx_blockheader_end->next = gfx_blockheader;// link in new node to the end of the list
		gfx_blockheader->next = NULL;					// set next field to signify the end of list
		gfx_blockheader_end = gfx_blockheader;			// adjust end to point to the last node
	}
	
	gfx_header->blocks = gfx_blockheader_head;

	return (gfx_header);
}

CFDictionaryRef CreateGFXDictionary(GFX_HEADER * gfx)
{
	CFMutableDictionaryRef dict, items;
	CFDataRef data = NULL;
	//CFNumberRef number = NULL;
	CFStringRef string = NULL;
	CFStringRef key = NULL; 
	GFX_BLOCKHEADER *gfx_blockheader_tmp;	
	GFX_ENTRY *gfx_entry_tmp;	
	uint64_t bigint;
	char hexstr[32];
	char *dpath;
	
	// Create dictionary that will hold gfx data
	dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0 ,&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	gfx_blockheader_tmp = gfx->blocks;
	while(gfx_blockheader_tmp)
	{
		items = CFDictionaryCreateMutable(kCFAllocatorDefault, 0 ,&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		gfx_entry_tmp = gfx_blockheader_tmp->entries;
		while(gfx_entry_tmp)
		{
			key = CFStringCreateWithCString(kCFAllocatorDefault, gfx_entry_tmp->key, kCFStringEncodingUTF8);
			switch(gfx_entry_tmp->val_type)
			{
				case DATA_STRING:
					string = CFStringCreateWithBytes(kCFAllocatorDefault,gfx_entry_tmp->val, gfx_entry_tmp->val_len, kCFStringEncodingASCII, false);
					CFDictionarySetValue(items, key, string);
					CFRelease(string);
					CFRelease(key);											
				break;
				case DATA_INT8:
					bigint = READ_UINT8(gfx_entry_tmp->val);
					sprintf(hexstr,"0x%02llx",bigint);
					string = CFStringCreateWithCString(kCFAllocatorDefault,hexstr, kCFStringEncodingASCII);
					CFDictionarySetValue(items, key, string);
					CFRelease(string);
					CFRelease(key);													
				break;
				case DATA_INT16:
					bigint = READ_UINT16(gfx_entry_tmp->val);
					sprintf(hexstr,"0x%04llx",bigint);
					string = CFStringCreateWithCString(kCFAllocatorDefault,hexstr, kCFStringEncodingASCII);
					CFDictionarySetValue(items, key, string);
					CFRelease(string);
					CFRelease(key);										
				break;
				case DATA_INT32:
					bigint = READ_UINT32(gfx_entry_tmp->val);
					sprintf(hexstr,"0x%08llx",bigint);
					string = CFStringCreateWithCString(kCFAllocatorDefault,hexstr, kCFStringEncodingASCII);
					CFDictionarySetValue(items, key, string);
					CFRelease(string);
					CFRelease(key);										
				break;
				default:				
				case DATA_BINARY:
					data = CFDataCreate(kCFAllocatorDefault,gfx_entry_tmp->val, gfx_entry_tmp->val_len);
					CFDictionarySetValue(items, key, data);
					CFRelease(data);
					CFRelease(key);					
				break;			
			}
			gfx_entry_tmp = gfx_entry_tmp->next;
		}

		dpath = ConvertDevicePathToText (gfx_blockheader_tmp->devpath, 1, 1);
		if(dpath != NULL)
		{
			key = CFStringCreateWithCString(kCFAllocatorDefault, dpath, kCFStringEncodingUTF8);
		}
		else
		{
			printf("CreateGFXDictionary: error converting device path to text shorthand notation\n");
			return NULL;			
		}
	
		CFDictionarySetValue(dict, key, items);
		
		free(dpath);
		CFRelease(key);
		CFRelease(items);							
		gfx_blockheader_tmp = gfx_blockheader_tmp->next;
	}

	return dict;
} 
