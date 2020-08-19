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
#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "b64cdecode.h"
#include "plist.h"
#include "../libeg/FloatLib.h"

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

XObjArray<TagStruct> gTagsFree;




void TagStruct::FreeTag()
{
  // Clear and free the tag.
  type = kTagTypeNone;

  _string.setEmpty();
  _intValue = 0;
  _floatValue = 0;

  if ( _data ) {
      FreePool(_data);
      _data = NULL;
  }
  _dataLen = 0;

  
  if ( _tag ) {
    _tag->FreeTag();
    _tag = NULL;
  }
//  while ( tagIdx < _dictOrArrayContent.notEmpty() ) {
//    _dictOrArrayContent[0].FreeTag();
//    _dictOrArrayContent.RemoveWithoutFreeingAtIndex(0);
//  }
  // this loop is better because removing objects from the end don't do any memory copying.
  for (size_t tagIdx = _dictOrArrayContent.size() ; tagIdx > 0  ; ) {
    tagIdx--;
    _dictOrArrayContent[tagIdx].FreeTag();
    _dictOrArrayContent.RemoveWithoutFreeingAtIndex(tagIdx);
  }

//  if ( _nextTag ) {
//    _nextTag->FreeTag();
//    _nextTag = NULL;
//  }

  gTagsFree.AddReference(this, false);
}









CHAR8* buffer_start = NULL;

// Forward declarations
EFI_STATUS ParseTagDict( CHAR8* buffer, TagStruct* * tag, UINT32 empty, UINT32* lenPtr);
EFI_STATUS ParseTagArray( CHAR8* buffer, TagStruct* * tag, UINT32 empty, UINT32* lenPtr);
EFI_STATUS ParseTagKey( char * buffer, TagStruct* * tag, UINT32* lenPtr);
EFI_STATUS ParseTagString(CHAR8* buffer, TagStruct* * tag, UINT32* lenPtr);
EFI_STATUS ParseTagInteger(CHAR8* buffer, TagStruct* * tag, UINT32* lenPtr);
EFI_STATUS ParseTagFloat(CHAR8* buffer, TagStruct* * tag, UINT32* lenPtr);
EFI_STATUS ParseTagData(CHAR8* buffer, TagStruct* * tag, UINT32* lenPtr);
EFI_STATUS ParseTagDate(CHAR8* buffer, TagStruct* * tag, UINT32* lenPtr);
EFI_STATUS ParseTagBoolean(CHAR8* buffer, TagStruct* * tag, bool value, UINT32* lenPtr);

EFI_STATUS XMLParseNextTag (CHAR8  *buffer, TagStruct**tag, UINT32 *lenPtr);

TagStruct*     NewTag( void );
EFI_STATUS FixDataMatchingTag( CHAR8* buffer, CONST CHAR8* tag,UINT32* lenPtr);

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

/* Replace XML entities by their value */
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
    if ( *s == '&' ) {
      BOOLEAN entFound = FALSE;
      UINTN i;
      s++;
      for (i = 0; i < sizeof(ents)/sizeof(ents[0]); i++) {
        if ( strncmp(s, ents[i].name, ents[i].nameLen) == 0 ) {
          entFound = TRUE;
          break;
        }
      }
      if ( entFound ) {
        *o++ = ents[i].value;
        s += ents[i].nameLen;
        continue;
      }
    }

    *o++ = *s++;
  }

  return out;
}

INTN GetTagCount(const TagStruct* dict )
{
  INTN count = 0;

  if ( !dict ) return 0;
  
  if ( dict->isArray() ) {
    return dict->dictOrArrayContent().size(); // If we are an array, any element is valid
  }else
  if ( dict->isDict() ) {
    const XObjArray<TagStruct>& tagList = dict->dictOrArrayContent();
    for (size_t tagIdx = 0 ; tagIdx < tagList.size() ; tagIdx++ ) {
      if ( tagList[tagIdx].isKey() ) count++;
    }
    return count;
  }else{
    return 0;
  }
}

EFI_STATUS GetElement(const TagStruct* dict, INTN id, const TagStruct** dict1)
{
  INTN element = 0;

  if ( !dict ) return EFI_UNSUPPORTED;
  if ( id < 0 ) return EFI_UNSUPPORTED;

  if ( dict->isArray() ) {
    if ( (size_t)id < dict->dictOrArrayContent().size() ) {
      *dict1 = &dict->dictOrArrayContent()[id];
      return EFI_SUCCESS;
    }
  }else
  if ( dict->isDict() ) {
    const XObjArray<TagStruct>& tagList = dict->dictOrArrayContent();
    size_t tagIdx;
    for (tagIdx = 0 ; tagIdx < tagList.size() ; tagIdx++ ) {
      if ( tagList[tagIdx].isKey() ) {
        if ( element == id ) {
          *dict1 = &tagList[tagIdx];
          return EFI_SUCCESS;
        }
        element++;
      }
    }
  }
  return EFI_UNSUPPORTED;
}

// Expects to see one dictionary in the XML file, the final pos will be returned
// If the pos is not equal to the strlen, then there are multiple dicts
// Puts the first dictionary it finds in the
// tag pointer and returns the end of the dic, or returns -1 if not found.
//

EFI_STATUS ParseXML(const CHAR8* buffer, TagStruct** dict, UINT32 bufSize)
{
  EFI_STATUS  Status;
  UINT32    length = 0;
  UINT32    pos = 0;
  TagStruct*    tag = NULL;
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
    if (tag->isDict()) {
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

const TagStruct* GetProperty(const TagStruct* dict, const CHAR8* key )
{
  if ( !dict->isDict() ) return NULL;
  
  const XObjArray<TagStruct>& tagList = dict->dictOrArrayContent();
  for (size_t tagIdx = 0 ; tagIdx < tagList.size() ; tagIdx++ )
  {
    if ( tagList[tagIdx].isKey()  &&  tagList[tagIdx].keyValue().equalIC(key) ) return tagList[tagIdx].keyTagValue();
  }

  return NULL;
}

//TagStruct* GetNextProperty(TagStruct* dict)
//{
//	TagStruct* tagList, tag;
//
//	if (dict->isDict()) {
//		return NULL;
//	}
//
//	tag = NULL;
//	tagList = dict->tag;
//	while (tagList)
//	{
//		tag = tagList;
//		tagList = tag->tagNext;
//
//		if ( !tag->isKey() ||  tag->keyValue().isEmpty() ) {
//			continue;
//
//		}
//		return tag->tag;
//	}
//
//	return NULL;
//}



//==========================================================================
// ParseNextTag

EFI_STATUS XMLParseNextTag(CHAR8* buffer, TagStruct** tag, UINT32* lenPtr)
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
  if (!strncmp(tagName, kXMLTagPList, 6)) {
    length=0;
    Status=EFI_SUCCESS;
  }
  /***** dict ****/
  else if (!AsciiStrCmp(tagName, kXMLTagDict))
  {
    DBG("begin dict len=%d\n", length);
    Status = ParseTagDict(buffer + pos, tag, 0, &length);
  }
  else if (!AsciiStrCmp(tagName, kXMLTagDict "/"))
  {
    DBG("end dict len=%d\n", length);
    Status = ParseTagDict(buffer + pos, tag, 1, &length);
  }
  else if (!strncmp(tagName, kXMLTagDict " ", 5))
  {
    DBG("space dict len=%d\n", length);
    Status = ParseTagDict(buffer + pos, tag, 0, &length);
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
  else if (!strncmp(tagName, kXMLTagString " ", 7))
  {
    DBG("parse String len=%d\n", length);
    Status = ParseTagString(buffer + pos, tag, &length);
  }
  /***** integer ****/
  else if (!AsciiStrCmp(tagName, kXMLTagInteger))
  {
    Status = ParseTagInteger(buffer + pos, tag, &length);
  }
  else if (!strncmp(tagName, kXMLTagInteger " ", 8))
  {
    Status = ParseTagInteger(buffer + pos, tag, &length);
  }
  /***** float ****/
  else if (!AsciiStrCmp(tagName, kXMLTagFloat))
  {
    Status = ParseTagFloat(buffer + pos, tag, &length);
  }
  else if (!strncmp(tagName, kXMLTagFloat " ", 8))
  {
    Status = ParseTagFloat(buffer + pos, tag, &length);
  }
  /***** data ****/
  else if (!AsciiStrCmp(tagName, kXMLTagData))
  {
    Status = ParseTagData(buffer + pos, tag, &length);
  }
  else if (!strncmp(tagName, kXMLTagData " ", 5))
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
    Status = ParseTagBoolean(buffer + pos, tag, false, &length);
  }
  /***** TRUE ****/
  else if (!AsciiStrCmp(tagName, kXMLTagTrue))
  {
    Status = ParseTagBoolean(buffer + pos, tag, true, &length);
  }
  /***** array ****/
  else if (!AsciiStrCmp(tagName, kXMLTagArray))
  {
    Status = ParseTagArray(buffer + pos, tag, 0, &length);
  }
  else if (!strncmp(tagName, kXMLTagArray " ", 6))
  {
    DBG("begin array len=%d\n", length);
    Status = ParseTagArray(buffer + pos, tag, 0, &length);
  }
  else if (!AsciiStrCmp(tagName, kXMLTagArray "/"))
  {
    DBG("end array len=%d\n", length);
    Status = ParseTagArray(buffer + pos, tag, 1, &length);
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

EFI_STATUS __ParseTagList(bool isArray, CHAR8* buffer, TagStruct** tag, UINT32 empty, UINT32* lenPtr)
{
  EFI_STATUS  Status = EFI_SUCCESS;
  UINT32    pos;
  TagStruct*    tagTail;
  UINT32    length = 0;

  if (isArray) {
    DBG("parsing array len=%d\n", *lenPtr);
  } else {
    DBG("parsing dict len=%d\n", *lenPtr);
  }
  tagTail = NULL;
  pos = 0;

  TagStruct* dictOrArrayTag = NewTag();
  if (isArray) {
    dictOrArrayTag->setArrayTagValue(NULL);
  } else {
    dictOrArrayTag->setDictTagValue(NULL);
  }
  XObjArray<TagStruct>& tagList = dictOrArrayTag->dictOrArrayContent();

  if (!empty) {
    while (TRUE) {
      TagStruct* newDictOrArrayTag = NULL;
      Status = XMLParseNextTag(buffer + pos, &newDictOrArrayTag, &length);
      if (EFI_ERROR(Status)) {
        DBG("error XMLParseNextTag in array: %s\n", strerror(Status));
        break;
      }

      pos += length;

      if (newDictOrArrayTag == NULL) {
        break;
      }

      tagList.AddReference(newDictOrArrayTag, true);
    }

    if (EFI_ERROR(Status)) {
      FreeTag(dictOrArrayTag);
      return Status;
    }
  }

  *tag = dictOrArrayTag;
  *lenPtr=pos;
  DBG(" return from ParseTagList with len=%d\n", *lenPtr);
  return Status;
}

EFI_STATUS ParseTagDict( CHAR8* buffer, TagStruct** tag, UINT32 empty, UINT32* lenPtr)
{
  return __ParseTagList(false, buffer, tag, empty, lenPtr);
}

EFI_STATUS ParseTagArray( CHAR8* buffer, TagStruct** tag, UINT32 empty, UINT32* lenPtr)
{
  return __ParseTagList(true, buffer, tag, empty, lenPtr);
}

//==========================================================================
// ParseTagKey

EFI_STATUS ParseTagKey( char * buffer, TagStruct** tag, UINT32* lenPtr)
{
  EFI_STATUS  Status;
  UINT32      length = 0;
  UINT32      length2 = 0;
  TagStruct*      tmpTag;
  TagStruct*      subTag = NULL;

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
  tmpTag->setKeyValue(LString8(buffer), subTag);

  *tag = tmpTag;
  *lenPtr = length + length2;
  DBG("parse key '%s' success len=%d\n", tmpString, *lenPtr);
  return EFI_SUCCESS;
}

//==========================================================================
// ParseTagString

EFI_STATUS ParseTagString(CHAR8* buffer, TagStruct* * tag,UINT32* lenPtr)
{
  EFI_STATUS  Status;
  UINT32    length = 0;
  TagStruct*    tmpTag;

  Status = FixDataMatchingTag(buffer, kXMLTagString, &length);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  tmpTag = NewTag();
  if (tmpTag == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  tmpTag->setStringValue(LString8(XMLDecode(buffer)));
  *tag = tmpTag;
  *lenPtr = length;
  DBG(" parse string %s\n", tmpString);
  return EFI_SUCCESS;
}

//==========================================================================
// ParseTagInteger

EFI_STATUS ParseTagInteger(CHAR8* buffer, TagStruct* * tag, UINT32* lenPtr)
{
  EFI_STATUS  Status;
  UINT32    length = 0;
  INTN     integer;
  UINT32    size;
  BOOLEAN   negative = FALSE;
  CHAR8*    val = buffer;
  TagStruct* tmpTag;

  Status = FixDataMatchingTag(buffer, kXMLTagInteger, &length);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  tmpTag = NewTag();
  tmpTag->setIntValue(0);

  size = length;
  integer = 0;
  
  if(buffer[0] == '<')
  {
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
        MsgLog("ParseTagInteger hex error (0x%hhX) in buffer %s\n", *val, buffer);
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
          MsgLog("ParseTagInteger decimal error (0x%hhX) in buffer %s\n", *val, buffer);
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


  tmpTag->setIntValue(integer);

  *tag = tmpTag;
  *lenPtr = length;
  return EFI_SUCCESS;
}

//==========================================================================
// ParseTagFloat

EFI_STATUS ParseTagFloat(CHAR8* buffer, TagStruct* * tag, UINT32* lenPtr)
{
  EFI_STATUS  Status;
  UINT32      length;
  TagStruct*      tmpTag;
  
  Status = FixDataMatchingTag(buffer, kXMLTagFloat, &length);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  tmpTag = NewTag();
  if (tmpTag == NULL)  {
    return EFI_OUT_OF_RESOURCES;
  }
//----
  float f;
  AsciiStrToFloat(buffer, NULL, &f);
//----
  tmpTag->setFloatValue(f);
  
  *tag = tmpTag;
  *lenPtr = length;
  return EFI_SUCCESS;
}

//==========================================================================
// ParseTagData

EFI_STATUS ParseTagData(CHAR8* buffer, TagStruct* * tag, UINT32* lenPtr)
{
  EFI_STATUS  Status;
  UINT32    length = 0;
  TagStruct*    tmpTag;

  Status = FixDataMatchingTag(buffer, kXMLTagData,&length);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  tmpTag = NewTag();
  if (tmpTag == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  //Slice - correction as Apple 2003
//  tmpTag->setStringValue(LString8(buffer));
  // dmazar: base64 decode data
  UINTN  len = 0;
  UINT8* data = (UINT8 *)Base64DecodeClover(buffer, &len);
  tmpTag->setDataValue(data, len);

  *tag = tmpTag;
  *lenPtr = length;

  return EFI_SUCCESS;
}

//==========================================================================
// ParseTagDate

EFI_STATUS ParseTagDate(CHAR8* buffer, TagStruct* * tag,UINT32* lenPtr)
{
  EFI_STATUS  Status;
  UINT32    length = 0;
  TagStruct*    tmpTag;

  Status = FixDataMatchingTag(buffer, kXMLTagDate,&length);
  if (EFI_ERROR(Status)) {
    return Status;
  }


  tmpTag = NewTag();
  if (tmpTag == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  tmpTag->setDateValue(LString8(buffer));

  *tag = tmpTag;
  *lenPtr = length;

  return EFI_SUCCESS;
}

//==========================================================================
// ParseTagBoolean

EFI_STATUS ParseTagBoolean(CHAR8* buffer, TagStruct* * tag, bool value, UINT32* lenPtr)
{
  TagStruct* tmpTag;

  tmpTag = NewTag();
  if (tmpTag == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  tmpTag->setBoolValue(value);

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

TagStruct* NewTag( void )
{
  TagStruct*  tag;

  if ( gTagsFree.size() > 0 ) {
    tag = &gTagsFree[0];
    gTagsFree.RemoveWithoutFreeingAtIndex(0);
    return tag;
  }
  tag = new TagStruct();
  return tag;
}

//==========================================================================
// XMLFreeTag

void FreeTag( TagStruct* tag )
{
  if (tag == NULL) {
    return;
  }
  tag->FreeTag();
}





/*
 return TRUE if the property present && value = TRUE
 else return FALSE
 */
BOOLEAN
IsPropertyTrue(const TagStruct* Prop)
{
  return Prop != NULL && Prop->isTrueOrYy();
}

/*
 return TRUE if the property present && value = FALSE
 else return FALSE
 */
BOOLEAN
IsPropertyFalse(const TagStruct* Prop)
{
  return Prop != NULL && Prop->isFalseOrNn();
}

/*
 Possible values
 <integer>1234</integer>
 <integer>+1234</integer>
 <integer>-1234</integer>
 <string>0x12abd</string>
 */
INTN
GetPropertyInteger(
                    const TagStruct* Prop,
                    INTN Default
                    )
{
  if (Prop == NULL) {
    return Default;
  }

  if (Prop->isInt()) {
    return Prop->intValue();
  } else if ((Prop->isString()) && Prop->stringValue().notEmpty()) {
    if ( Prop->stringValue().length() > 1  &&  (Prop->stringValue()[1] == 'x' || Prop->stringValue()[1] == 'X') ) {
      return (INTN)AsciiStrHexToUintn(Prop->stringValue());
    }

    if (Prop->stringValue()[0] == '-') {
      return -(INTN)AsciiStrDecimalToUintn (Prop->stringValue().c_str() + 1);
    }

//    return (INTN)AsciiStrDecimalToUintn (Prop->stringValue());
    return (INTN)AsciiStrDecimalToUintn((Prop->stringValue()[0] == '+') ? (Prop->stringValue().c_str() + 1) : Prop->stringValue().c_str());
  }
  return Default;
}

float GetPropertyFloat (const TagStruct* Prop, float Default)
{
  if (Prop == NULL) {
    return Default;
  }
  if (Prop->isFloat()) {
    return Prop->floatValue(); //this is union char* or float
  } else if ((Prop->isString()) && Prop->stringValue().notEmpty()) {
    float fVar = 0.f;
    if(!AsciiStrToFloat(Prop->stringValue().c_str(), NULL, &fVar)) //if success then return 0
      return fVar;
  }
  
  return Default;
}
