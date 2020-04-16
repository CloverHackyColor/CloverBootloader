/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 *
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 *  plist.c - plist parsing functions
 *
 *  Copyright (c) 2000-2005 Apple Computer, Inc.
 *
 *  DRI: Josh de Cesare
 *  code split out from drivers.c by Soren Spies, 2005
 */
//Slice - rewrite for UEFI with more functions like Copyright (c) 2003 Apple Computer
#include "Platform.h"
#include "b64cdecode.h"

#ifndef DEBUG_ALL
#define DEBUG_PLIST 0
#else
#define DEBUG_PLIST DEBUG_ALL
#endif

#if DEBUG_PLIST == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_PLIST, __VA_ARGS__)
#endif



/* XML Tags */
#define kXMLTagPList     "plist"
#define kXMLTagDict      "dict"
#define kXMLTagKey       "key"
#define kXMLTagString    "string"
#define kXMLTagInteger   "integer"
#define kXMLTagData      "data"
#define kXMLTagDate      "date"
#define kXMLTagFalse     "false/"
#define kXMLTagTrue      "true/"
#define kXMLTagArray     "array"
#define kXMLTagReference "reference"
#define kXMLTagID        "ID="
#define kXMLTagIDREF     "IDREF="


struct Symbol {
  UINTN         refCount;
  struct Symbol *next;
  CHAR8         string[1];
};

typedef struct Symbol Symbol, *SymbolPtr;



SymbolPtr gSymbolsHead = NULL;
TagPtr    gTagsFree = NULL;
CHAR8* buffer_start = NULL;

// Forward declarations
EFI_STATUS ParseTagList( CHAR8* buffer, TagPtr * tag, UINT32 type, UINT32 empty, UINT32* lenPtr);
EFI_STATUS ParseTagKey( char * buffer, TagPtr * tag,UINT32* lenPtr);
EFI_STATUS ParseTagString(CHAR8* buffer, TagPtr * tag,UINT32* lenPtr);
EFI_STATUS ParseTagInteger(CHAR8* buffer, TagPtr * tag,UINT32* lenPtr);
EFI_STATUS ParseTagData(CHAR8* buffer, TagPtr * tag,UINT32* lenPtr);
EFI_STATUS ParseTagDate(CHAR8* buffer, TagPtr * tag,UINT32* lenPtr);
EFI_STATUS ParseTagBoolean(CHAR8* buffer, TagPtr * tag, UINT32 type,UINT32* lenPtr);
//defined in Platform.h
//EFI_STATUS GetElement( TagPtr dict, INTN id,  TagPtr *dict1);
//INTN GetTagCount( TagPtr dict );

TagPtr     NewTag( void );
EFI_STATUS FixDataMatchingTag( CHAR8* buffer, CONST CHAR8* tag,UINT32* lenPtr);
CHAR8*      NewSymbol(CHAR8* string);
VOID        FreeSymbol(CHAR8* string);
SymbolPtr   FindSymbol( char * string, SymbolPtr * prevSymbol );

/* Function for basic XML character entities parsing */
typedef struct XMLEntity {
  const CHAR8* name;
  UINTN nameLen;
  CHAR8 value;
} XMLEntity;

/* This is ugly, but better than specifying the lengths by hand */
#define _e(str,c) {str,sizeof(str)-1,c}
CONST XMLEntity ents[] = {
  _e("quot;",'"'), _e("apos;",'\''),
  _e("lt;",  '<'), _e("gt;",  '>'),
  _e("amp;", '&')
};

CHAR8*
XMLDecode(CHAR8* src)
{
  UINTN len;
  CONST CHAR8 *s;
  CHAR8 *out, *o;

  if (!src) {
    return 0;
  }

  len = AsciiStrLen(src);

#if 0
  out = (__typeof__(out))AllocateZeroPool(len+1);
  if (!out)
    return 0;
#else // unsafe
  // out is always <= src, let's overwrite src
  out = src;
#endif


  o = out;
  s = src;
  while (s <= src+len) /* Make sure the terminator is also copied */
  {
    if ( *s == '&' )
    {
      BOOLEAN entFound = FALSE;
      UINTN i;

      s++;
      for ( i = 0; i < sizeof(ents)/sizeof(ents[0]); i++)
      {
        if ( AsciiStrnCmp(s, ents[i].name, ents[i].nameLen) == 0 )
        {
          entFound = TRUE;
          break;
        }
      }
      if ( entFound )
      {
        *o++ = ents[i].value;
        s += ents[i].nameLen;
        continue;
      }
    }

    *o++ = *s++;
  }

  return out;
}

INTN GetTagCount( TagPtr dict )
{
  INTN count = 0;
  TagPtr tagList, tag;

  if (!dict || (dict->type != kTagTypeDict && dict->type != kTagTypeArray)) {
    return 0;
  }
  tag = 0;
  tagList = dict->tag;
  while (tagList)
  {
    tag = tagList;
    tagList = tag->tagNext;

    if (((dict->type == kTagTypeDict) && (tag->type == kTagTypeKey)) ||
         (dict->type == kTagTypeArray)  // If we are an array, any element is valid
        )
    {
		  count++;
    }

    //if(tag->type == kTagTypeKey) printf("Located key %s\n", tag->string);
  }

  return count;
}

EFI_STATUS GetElement( TagPtr dict, INTN id,  TagPtr * dict1)
{
  INTN element = 0;
  TagPtr child;

  if(!dict || !dict1 || ((dict->type != kTagTypeArray) && (dict->type != kTagTypeDict))) {
    return EFI_UNSUPPORTED;
  }

  child = dict->tag;
  while (child != NULL)
  {
	  if (((dict->type == kTagTypeDict) && (child->type == kTagTypeKey)) ||  //in Dict count Keys
		  (dict->type == kTagTypeArray)  // If we are an array, any element is valid
		  )
	  {
		  if (element++ >= id) break;
	  }
    child = child->tagNext;
  }
  *dict1 = child;
  return EFI_SUCCESS;
}

// Expects to see one dictionary in the XML file, the final pos will be returned
// If the pos is not equal to the strlen, then there are multiple dicts
// Puts the first dictionary it finds in the
// tag pointer and returns the end of the dic, or returns -1 if not found.
//

EFI_STATUS ParseXML(const CHAR8* buffer, TagPtr * dict, UINT32 bufSize)
{
  EFI_STATUS  Status;
  UINT32    length = 0;
  UINT32    pos = 0;
  TagPtr    tag = NULL;
  CHAR8*    configBuffer = NULL;
  UINT32    bufferSize = 0;
  UINTN     i;

  if (bufSize) {
    bufferSize = bufSize;
  } else {
    bufferSize = (UINT32)AsciiStrLen(buffer);
  }

  if(dict == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  configBuffer = (__typeof__(configBuffer))AllocateZeroPool(bufferSize+1);
  if(configBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem(configBuffer, buffer, bufferSize);
  for (i=0; i<bufferSize; i++) {
    if (configBuffer[i] == 0) {
      configBuffer[i] = 0x20;  //replace random zero bytes to spaces
    }
  }

  buffer_start = configBuffer;
  while (TRUE)
  {
    Status = XMLParseNextTag(configBuffer + pos, &tag, &length);
    if (EFI_ERROR(Status)) {
      DBG("error parsing next tag\n");
      break;
    }

    pos += length;

    if (tag == NULL) {
      continue;
    }
    if (tag->type == kTagTypeDict) {
      break;
    }

	  FreeTag(tag); tag = NULL;
  }

//  FreePool(configBuffer);

  if (EFI_ERROR(Status)) {
    return Status;
  }

  *dict = tag;
  return EFI_SUCCESS;
}

//
// xml
//

#define DOFREE 1

//==========================================================================
// GetProperty

TagPtr GetProperty( TagPtr dict, const CHAR8* key )
{
  TagPtr tagList, tag;

  if (dict->type != kTagTypeDict) {
    return NULL;
  }

  tag = NULL;
  tagList = dict->tag;
  while (tagList)
  {
    tag = tagList;
    tagList = tag->tagNext;

    if ((tag->type != kTagTypeKey) || (tag->string == 0)) {
      continue;

    }

    if (!AsciiStriCmp(tag->string, key)) {
      return tag->tag;
    }
  }

  return NULL;
}

TagPtr GetNextProperty(TagPtr dict)
{
	TagPtr tagList, tag;

	if (dict->type != kTagTypeDict) {
		return NULL;
	}

	tag = NULL;
	tagList = dict->tag;
	while (tagList)
	{
		tag = tagList;
		tagList = tag->tagNext;

		if ((tag->type != kTagTypeKey) || (tag->string == 0)) {
			continue;

		}
		return tag->tag;
	}

	return NULL;
}



//==========================================================================
// ParseNextTag

EFI_STATUS XMLParseNextTag(CHAR8* buffer, TagPtr* tag, UINT32* lenPtr)
{
  EFI_STATUS  Status;
  UINT32      length = 0;
  UINT32      pos = 0;
  CHAR8*      tagName = NULL;

  *lenPtr=0;

  Status = GetNextTag((UINT8*)buffer, &tagName, 0, &length);
  if (EFI_ERROR(Status)) {
    DBG("NextTag error %s\n", strerror(Status));
    return Status;
  }

  pos = length;
  if (!AsciiStrnCmp(tagName, kXMLTagPList, 6)) {
    length=0;
    Status=EFI_SUCCESS;
  }
  /***** dict ****/
  else if (!AsciiStrCmp(tagName, kXMLTagDict))
  {
    DBG("begin dict len=%d\n", length);
    Status = ParseTagList(buffer + pos, tag, kTagTypeDict, 0, &length);
  }
  else if (!AsciiStrCmp(tagName, kXMLTagDict "/"))
  {
    DBG("end dict len=%d\n", length);
    Status = ParseTagList(buffer + pos, tag, kTagTypeDict, 1, &length);
  }
  else if (!AsciiStrnCmp(tagName, kXMLTagDict " ", 5))
  {
    DBG("space dict len=%d\n", length);
    Status = ParseTagList(buffer + pos, tag, kTagTypeDict, 0, &length);
  }
  /***** key ****/
  else if (!AsciiStrCmp(tagName, kXMLTagKey))
  {
    DBG("parse key\n");
    Status = ParseTagKey(buffer + pos, tag, &length);
  }
  /***** string ****/
  else if (!AsciiStrCmp(tagName, kXMLTagString))
  {
    DBG("parse String\n");
    Status = ParseTagString(buffer + pos, tag, &length);
  }
   /***** string ****/
  else if (!AsciiStrnCmp(tagName, kXMLTagString " ", 7))
  {
    DBG("parse String len=%d\n", length);
    Status = ParseTagString(buffer + pos, tag, &length);
  }
  /***** integer ****/
  else if (!AsciiStrCmp(tagName, kXMLTagInteger))
  {
    Status = ParseTagInteger(buffer + pos, tag, &length);
  }
  else if (!AsciiStrnCmp(tagName, kXMLTagInteger " ", 8))
  {
    Status = ParseTagInteger(buffer + pos, tag, &length);
  }

  /***** data ****/
  else if (!AsciiStrCmp(tagName, kXMLTagData))
  {
    Status = ParseTagData(buffer + pos, tag, &length);
  }
  else if (!AsciiStrnCmp(tagName, kXMLTagData " ", 5))
  {
    Status = ParseTagData(buffer + pos, tag, &length);
  }
  /***** date ****/
  else if (!AsciiStrCmp(tagName, kXMLTagDate))
  {
    Status = ParseTagDate(buffer + pos, tag, &length);
  }
  /***** FALSE ****/
  else if (!AsciiStrCmp(tagName, kXMLTagFalse))
  {
    Status = ParseTagBoolean(buffer + pos, tag, kTagTypeFalse, &length);
  }
  /***** TRUE ****/
  else if (!AsciiStrCmp(tagName, kXMLTagTrue))
  {
    Status = ParseTagBoolean(buffer + pos, tag, kTagTypeTrue, &length);
  }
  /***** array ****/
  else if (!AsciiStrCmp(tagName, kXMLTagArray))
  {
    Status = ParseTagList(buffer + pos, tag, kTagTypeArray, 0, &length);
  }
  else if (!AsciiStrnCmp(tagName, kXMLTagArray " ", 6))
  {
    DBG("begin array len=%d\n", length);
    Status = ParseTagList(buffer + pos, tag, kTagTypeArray, 0, &length);
  }
  else if (!AsciiStrCmp(tagName, kXMLTagArray "/"))
  {
    DBG("end array len=%d\n", length);
    Status = ParseTagList(buffer + pos, tag, kTagTypeArray, 1, &length);
  }
  /***** unknown ****/
  else
  {
    *tag = NULL;
    length = 0;
  }

  if (EFI_ERROR(Status)) {
    return Status;
  }

  // TODO jief : seems to me that length cannot be -1. Added the cast anyway to avoid regression. If confirmed, the next 3 lines must be removed.
  if (length == (UINT32)-1) {
    DBG("(length == -1)\n");
    return EFI_UNSUPPORTED;
  }

  *lenPtr = pos + length;
  DBG("  len after success parse next tag %d\n", *lenPtr);
  return EFI_SUCCESS;
}

//==========================================================================
// ParseTagList

EFI_STATUS ParseTagList( CHAR8* buffer, TagPtr* tag, UINT32 type, UINT32 empty, UINT32* lenPtr)
{
  EFI_STATUS  Status = EFI_SUCCESS;
  UINT32    pos;
  TagPtr    tagList;
  TagPtr    tagTail;
  TagPtr    tmpTag = NULL;
  UINT32    length = 0;

  if (type == kTagTypeArray) {
    DBG("parsing array len=%d\n", *lenPtr);
  } else if (type == kTagTypeDict) {
    DBG("parsing dict len=%d\n", *lenPtr);
  }
  tagList = NULL;
  tagTail = NULL;
  pos = 0;

  if (!empty) {
    while (TRUE) {
      Status = XMLParseNextTag(buffer + pos, &tmpTag, &length);
      if (EFI_ERROR(Status)) {
        DBG("error XMLParseNextTag in array: %s\n", strerror(Status));
        break;
      }

      pos += length;

      if (tmpTag == NULL) {
        break;
      }

      if (tagTail) {
        tagTail->tagNext = tmpTag;
      } else {
        tagList = tmpTag;
      }
      tagTail = tmpTag;
    }

    if (EFI_ERROR(Status)) {
      if (tagList) {
        FreeTag(tagList);
      }
      return Status;
    }
  }

  tmpTag = NewTag();
  if (tmpTag == NULL) {
    if (tagList) {
      FreeTag(tagList);
    }
    DBG("next tag is NULL\n");
    return EFI_OUT_OF_RESOURCES;
  }

  tmpTag->type = type;
  tmpTag->string = 0;
  tmpTag->offset = (UINT32)(buffer_start ? buffer - buffer_start : 0);
  tmpTag->tag = tagList;
  tmpTag->tagNext = 0;

  *tag = tmpTag;
  *lenPtr=pos;
  DBG(" return from ParseTagList with len=%d\n", *lenPtr);
  return Status;
}

//==========================================================================
// ParseTagKey

EFI_STATUS ParseTagKey( char * buffer, TagPtr* tag, UINT32* lenPtr)
{
  EFI_STATUS  Status;
  UINT32      length = 0;
  UINT32      length2 = 0;
  CHAR8*      tmpString;
  TagPtr      tmpTag;
  TagPtr      subTag = NULL;

  Status = FixDataMatchingTag(buffer, kXMLTagKey, &length);
  DBG("fixing key len=%d status=%s\n", length, strerror(Status));
  if (EFI_ERROR(Status)){
    return Status;
  }

  Status = XMLParseNextTag(buffer + length, &subTag, &length2);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  tmpTag = NewTag();
  if (tmpTag == NULL) {
    FreeTag(subTag);
    return EFI_OUT_OF_RESOURCES;
  }

  tmpString = NewSymbol(buffer);
  if (tmpString == NULL) {
    FreeTag(subTag);
    FreeTag(tmpTag);
    return EFI_OUT_OF_RESOURCES;
  }

  tmpTag->type = kTagTypeKey;
  tmpTag->string = tmpString;
  tmpTag->tag = subTag;
  tmpTag->offset = (UINT32)(buffer_start ? buffer - buffer_start: 0);
  tmpTag->tagNext = 0;

  *tag = tmpTag;
  *lenPtr = length + length2;
  DBG("parse key '%s' success len=%d\n", tmpString, *lenPtr);
  return EFI_SUCCESS;
}

//==========================================================================
// ParseTagString

EFI_STATUS ParseTagString(CHAR8* buffer, TagPtr * tag,UINT32* lenPtr)
{
  EFI_STATUS  Status;
  UINT32    length = 0;
  CHAR8*    tmpString;
  TagPtr    tmpTag;

  Status = FixDataMatchingTag(buffer, kXMLTagString, &length);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  tmpTag = NewTag();
  if (tmpTag == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  tmpString = XMLDecode(buffer);
  tmpString = NewSymbol(tmpString);
  if (tmpString == NULL)
  {
    FreeTag(tmpTag);
    return EFI_OUT_OF_RESOURCES;
  }

  tmpTag->type = kTagTypeString;
  tmpTag->string = tmpString;
  tmpTag->tag = NULL;
  tmpTag->tagNext = NULL;
  tmpTag->offset = (UINT32)(buffer_start ? buffer - buffer_start: 0);
  *tag = tmpTag;
  *lenPtr = length;
  DBG(" parse string %s\n", tmpString);
  return EFI_SUCCESS;
}

//==========================================================================
// ParseTagInteger

EFI_STATUS ParseTagInteger(CHAR8* buffer, TagPtr * tag,UINT32* lenPtr)
{
  EFI_STATUS  Status;
  UINT32    length = 0;
  INTN     integer;
  UINT32    size;
  BOOLEAN   negative = FALSE;
  CHAR8*    val = buffer;
  TagPtr tmpTag;

  Status = FixDataMatchingTag(buffer, kXMLTagInteger,&length);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  tmpTag = NewTag();
  if (tmpTag == NULL)  {
    return EFI_OUT_OF_RESOURCES;
  }
  size = length;

  integer = 0;
  if(buffer[0] == '<') {
    tmpTag->type = kTagTypeInteger;
    tmpTag->string = 0;
    tmpTag->tag = 0;
    tmpTag->offset =  0;
    tmpTag->tagNext = 0;

    *tag = tmpTag;
    length = 0;
    return EFI_SUCCESS;
  }
  if(size > 1 && (val[1] == 'x' || val[1] == 'X')) {  // Hex value
    val += 2;
    while(*val) {
      if ((*val >= '0' && *val <= '9')) { // 0 - 9
        integer = (integer * 16) + (*val++ - '0');
      }
      else if ((*val >= 'a' && *val <= 'f')) { // a - f
        integer = (integer * 16) + (*val++ - 'a' + 10);
      }
      else if ((*val >= 'A' && *val <= 'F')) {  // A - F
        integer = (integer * 16) + (*val++ - 'a' + 10);
      }
      else {
        MsgLog("ParseTagInteger hex error (0x%X) in buffer %s\n", *val, buffer);
        //        getchar();
        FreeTag(tmpTag);
        return EFI_UNSUPPORTED;
      }
    }
  }
  else if ( size ) {  // Decimal value
    if (*val == '-') {
      negative = TRUE;
      val++;
      size--;
    }

    for (integer = 0; size > 0; size--) {
      if(*val) { // UGLY HACK, fix me.
        if (*val < '0' || *val > '9') {
          MsgLog("ParseTagInteger decimal error (0x%X) in buffer %s\n", *val, buffer);
          //          getchar();
          FreeTag(tmpTag);
          return EFI_UNSUPPORTED;
        }
        integer = (integer * 10) + (*val++ - '0');
      }
    }

    if (negative) {
      integer = -integer;
    }
  }


  tmpTag->type = kTagTypeInteger;
  tmpTag->string = (CHAR8*)(UINTN)integer;
  tmpTag->tag = NULL;
  tmpTag->offset = (UINT32)(buffer_start ? buffer - buffer_start: 0);
  tmpTag->tagNext = NULL;

  *tag = tmpTag;
  *lenPtr = length;
  return EFI_SUCCESS;
}

//==========================================================================
// ParseTagData

EFI_STATUS ParseTagData(CHAR8* buffer, TagPtr * tag, UINT32* lenPtr)
{
  EFI_STATUS  Status;
  UINT32    length = 0;
  UINTN     len = 0;
  TagPtr    tmpTag;
  CHAR8*    tmpString;

  Status = FixDataMatchingTag(buffer, kXMLTagData,&length);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  tmpTag = NewTag();
  if (tmpTag == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  //Slice - correction as Apple 2003
  tmpString = NewSymbol(buffer);
  tmpTag->type = kTagTypeData;
  tmpTag->string = tmpString;
  // dmazar: base64 decode data
  tmpTag->data = (UINT8 *)Base64DecodeClover(tmpTag->string, &len);
  tmpTag->dataLen = len;
  tmpTag->tag = NULL;
  tmpTag->offset = (UINT32)(buffer_start ? buffer - buffer_start: 0);
  tmpTag->tagNext = NULL;

  *tag = tmpTag;
  *lenPtr = length;

  return EFI_SUCCESS;
}

//==========================================================================
// ParseTagDate

EFI_STATUS ParseTagDate(CHAR8* buffer, TagPtr * tag,UINT32* lenPtr)
{
  EFI_STATUS  Status;
  UINT32    length = 0;
  TagPtr    tmpTag;

  Status = FixDataMatchingTag(buffer, kXMLTagDate,&length);
  if (EFI_ERROR(Status)) {
    return Status;
  }


  tmpTag = NewTag();
  if (tmpTag == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  tmpTag->type = kTagTypeDate;
  tmpTag->string = NULL;
  tmpTag->tag = NULL;
  tmpTag->tagNext = NULL;
  tmpTag->offset = (UINT32)(buffer_start ? buffer - buffer_start: 0);

  *tag = tmpTag;
  *lenPtr = length;

  return EFI_SUCCESS;
}

//==========================================================================
// ParseTagBoolean

EFI_STATUS ParseTagBoolean(CHAR8* buffer, TagPtr * tag, UINT32 type,UINT32* lenPtr)
{
  TagPtr tmpTag;

  tmpTag = NewTag();
  if (tmpTag == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  tmpTag->type = type;
  tmpTag->string = NULL;
  tmpTag->tag = NULL;
  tmpTag->tagNext = NULL;
  tmpTag->offset = (UINT32)(buffer_start ? buffer - buffer_start: 0);

  *tag = tmpTag;
  *lenPtr = 0;
  return EFI_SUCCESS;
}

//==========================================================================
// GetNextTag

EFI_STATUS GetNextTag( UINT8* buffer, CHAR8** tag, UINT32* start, UINT32* length)
{
  UINT32 cnt, cnt2;

  if (tag == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Find the start of the tag.
  cnt = 0;
  while ((buffer[cnt] != '\0') && (buffer[cnt] != '<')) {
    cnt++;
  }

  if (buffer[cnt] == '\0') {
    DBG("empty buffer at cnt=%d\n", cnt);
    return EFI_UNSUPPORTED;
  }

  // Find the end of the tag.
  cnt2 = cnt + 1;
  while ((buffer[cnt2] != '\0') && (buffer[cnt2] != '>')) {
    cnt2++;
  }

  if (buffer[cnt2] == '\0') {
    DBG("empty buffer at cnt2=%d\n", cnt2);
    return EFI_UNSUPPORTED;
  }

  // Fix the tag data.
  *tag = (CHAR8*)(buffer + cnt + 1);
  buffer[cnt2] = '\0';
  if (start) {
    *start = cnt;
  }

  *length = cnt2 + 1;  //unreal to be -1. This is UINT32

  if (*length == (UINT32)-1) {
    DBG("GetNextTag with *length == -1\n");
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

//==========================================================================
// FixDataMatchingTag
// Modifies 'buffer' to add a '\0' at the end of the tag matching 'tag'.
// Returns the length of the data found, counting the end tag,
// or -1 if the end tag was not found.

EFI_STATUS FixDataMatchingTag( CHAR8* buffer, CONST CHAR8* tag, UINT32* lenPtr)
{
  EFI_STATUS  Status;
  UINT32    length;
  UINT32    start;
  UINT32    stop;
  CHAR8*    endTag;

  start = 0;
  while (1) {
    Status = GetNextTag(((UINT8 *)buffer) + start, &endTag, &stop, &length);
    if (EFI_ERROR(Status)) {
      return Status;
    }

    if ((*endTag == '/') && !AsciiStrCmp(endTag + 1, tag)) {
      break;
    }
    start += length;
  }
  DBG("fix buffer at pos=%d\n", start + stop);
  buffer[start + stop] = '\0';
  *lenPtr = start + length;

  if (*lenPtr == (__typeof_am__(*lenPtr))-1) { // Why is this test. -1 is UINTN_MAX.
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

//==========================================================================
// NewTag
#define TAGCACHESIZE 0x1000

TagPtr NewTag( void )
{
  UINT32  cnt;
  TagPtr  tag;

  if (gTagsFree == NULL) {
    tag = (TagPtr)AllocateZeroPool(TAGCACHESIZE * sizeof(TagStruct));
    if (tag == NULL) {
      return NULL;
    }

    // Initalize the new tags.
    for (cnt = 0; cnt < TAGCACHESIZE - 1; cnt++) {
      tag[cnt].type = kTagTypeNone;
      tag[cnt].tagNext = tag + cnt + 1;
    }
    tag[TAGCACHESIZE - 1].tagNext = 0;

    gTagsFree = tag;
  }

  tag = gTagsFree;
  gTagsFree = tag->tagNext;
  if (gTagsFree == NULL) {  //end of cache
    gTagsFree = NewTag();
    tag->tagNext = gTagsFree; //add new cache to old one
  }

  return tag;
}
#undef TAGCACHESIZE

//==========================================================================
// XMLFreeTag

void FreeTag( TagPtr tag )
{
  if (tag == NULL) {
    return;
  }

  if (tag->type != kTagTypeInteger && tag->string)  {
    FreeSymbol(tag->string);
  }
  if (tag->data) {
    FreePool(tag->data);
  }

  FreeTag(tag->tag);
  FreeTag(tag->tagNext);

  // Clear and free the tag.
  tag->type = kTagTypeNone;
  tag->string = NULL;
  tag->data = NULL;
  tag->dataLen = 0;
  tag->tag = NULL;
  tag->offset = 0;
  tag->tagNext = gTagsFree;
  gTagsFree = tag;
}


CHAR8* NewSymbol(CHAR8* tmpString)
{
#if 0
  SymbolPtr lastGuy = 0; // never used
#endif
  SymbolPtr symbol;
  UINTN      len;
  // Look for string in the list of symbols.
  symbol = FindSymbol(tmpString, 0);

  // Add the new symbol.
  if (symbol == NULL) {
    len = AsciiStrLen(tmpString);
    symbol = (SymbolPtr)AllocateZeroPool(sizeof(Symbol) + len + 1);
    if (symbol == NULL)  {
      return NULL;
    }

    // Set the symbol's data.
    symbol->refCount = 0;

    AsciiStrnCpyS(symbol->string, len+1, tmpString, len);

    // Add the symbol to the list.
    symbol->next = gSymbolsHead;
    gSymbolsHead = symbol;
  }

  // Update the refCount and return the string.
  symbol->refCount++;

#if 0
  if (lastGuy && lastGuy->next != 0) { // lastGuy is always 0, accessing to ((SymbolPtr)null)->next can be dangerous.
    return NULL;
  }
#endif

  return symbol->string;
}

//==========================================================================
// FreeSymbol

void FreeSymbol(CHAR8* tmpString)
{
  SymbolPtr symbol, prev;
  prev = NULL;

  // Look for string in the list of symbols.
  symbol = FindSymbol(tmpString, &prev);
  if (symbol == NULL) {
    return;
  }

  // Update the refCount.
  symbol->refCount--;

  if (symbol->refCount != 0) {
    return;
  }

  // Remove the symbol from the list.
  if (prev != NULL) {
    prev->next = symbol->next;
  }
  else {
    gSymbolsHead = symbol->next;
  }

  // Free the symbol's memory.
  FreePool(symbol);
}

//==========================================================================
// FindSymbol

SymbolPtr FindSymbol(CHAR8 *tmpString, SymbolPtr *prevSymbol )
{
  SymbolPtr symbol, prev;

  if (tmpString == NULL) {
    return NULL;
  }

  symbol = gSymbolsHead;
  prev = NULL;

  while (symbol != NULL) {
    if (!AsciiStrCmp(symbol->string, tmpString)) {
      break;
    }

    prev = symbol;
    symbol = symbol->next;
  }

  if ((symbol != NULL) && (prevSymbol != NULL)) {
    *prevSymbol = prev;
  }

  return symbol;
}



/*
 return TRUE if the property present && value = TRUE
 else return FALSE
 */
BOOLEAN
IsPropertyTrue (
                TagPtr Prop
                )
{
  return Prop != NULL &&
  ((Prop->type == kTagTypeTrue) ||
   ((Prop->type == kTagTypeString) && Prop->string &&
    ((Prop->string[0] == 'y') || (Prop->string[0] == 'Y'))));
}

/*
 return TRUE if the property present && value = FALSE
 else return FALSE
 */
BOOLEAN
IsPropertyFalse (
                 TagPtr Prop
                 )
{
  return Prop != NULL &&
  ((Prop->type == kTagTypeFalse) ||
   ((Prop->type == kTagTypeString) && Prop->string &&
    ((Prop->string[0] == 'N') || (Prop->string[0] == 'n'))));
}

/*
 Possible values
 <integer>1234</integer>
 <integer>+1234</integer>
 <integer>-1234</integer>
 <string>0x12abd</string>
 */
INTN
GetPropertyInteger (
                    TagPtr Prop,
                    INTN Default
                    )
{
  if (Prop == NULL) {
    return Default;
  }

  if (Prop->type == kTagTypeInteger) {
    return (INTN)Prop->string; //this is union char* or size_t
  } else if ((Prop->type == kTagTypeString) && Prop->string) {
    if ((Prop->string[1] == 'x') || (Prop->string[1] == 'X')) {
      return (INTN)AsciiStrHexToUintn (Prop->string);
    }

    if (Prop->string[0] == '-') {
      return -(INTN)AsciiStrDecimalToUintn (Prop->string + 1);
    }

//    return (INTN)AsciiStrDecimalToUintn (Prop->string);
    return (INTN)AsciiStrDecimalToUintn((Prop->string[0] == '+') ? (Prop->string + 1) : Prop->string);
  }
  return Default;
}
