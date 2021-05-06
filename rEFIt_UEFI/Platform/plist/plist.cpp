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
#include "../b64cdecode.h"
#include "plist.h"
#include "../../libeg/FloatLib.h"
#include "xml.h"

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
EFI_STATUS ParseTagBoolean(TagStruct* * tag, bool value, UINT32* lenPtr);

EFI_STATUS XMLParseNextTag (CHAR8  *buffer, TagStruct**tag, UINT32 *lenPtr);

EFI_STATUS FixDataMatchingTag( CHAR8* buffer, CONST CHAR8* tag,UINT32* lenPtr);


/****************************************  TagStruct  ****************************************/

#include "TagDict.h"
#include "TagKey.h"
#include "TagBool.h"
#include "TagData.h"
#include "TagDate.h"
#include "TagArray.h"
#include "TagFloat.h"
#include "TagInt64.h"
#include "TagString8.h"
//
////UINTN newtagcount = 0;
////UINTN tagcachehit = 0;
//TagStruct* TagStruct::getEmptyTag()
//{
//  TagStruct*  tag;
//
//  if ( gTagsFree.size() > 0 ) {
//    tag = &gTagsFree[0];
//    gTagsFree.RemoveWithoutFreeingAtIndex(0);
////tagcachehit++;
////DBG("tagcachehit=%lld\n", tagcachehit);
//    return tag;
//  }
//  tag = new TagStruct;
////newtagcount += 1;
////DBG("newtagcount=%lld\n", newtagcount);
//  return tag;
//}
//
//TagStruct* TagStruct::getEmptyDictTag()
//{
//  TagStruct* newDictTag = getEmptyTag();
//  newDictTag->type = kTagTypeDict;
//  return newDictTag;
//}
//
//TagStruct* TagStruct::getEmptyArrayTag()
//{
//  TagStruct* newArrayTag = getEmptyTag();
//  newArrayTag->type = kTagTypeArray;
//  return newArrayTag;
//}

//void TagStruct::FreeTag()
//{
//  // Clear and free the tag.
//  type = kTagTypeNone;
//
//  _string.setEmpty();
//  _intValue = 0;
//  _floatValue = 0;
//
//  if ( _data ) {
//      FreePool(_data);
//      _data = NULL;
//  }
//  _dataLen = 0;
//
//  //while ( tagIdx < _dictOrArrayContent.notEmpty() ) {
//  //  _dictOrArrayContent[0].FreeTag();
//  //  _dictOrArrayContent.RemoveWithoutFreeingAtIndex(0);
//  //}
//  // this loop is better because removing objects from the end don't do any memory copying.
//  for (size_t tagIdx = _dictOrArrayContent.size() ; tagIdx > 0  ; ) {
//    tagIdx--;
//    _dictOrArrayContent[tagIdx].FreeTag();
//    _dictOrArrayContent.RemoveWithoutFreeingAtIndex(tagIdx);
//  }
//
//  gTagsFree.AddReference(this, false);
//}


//TagStruct* GetNextProperty(TagStruct* dict)
//{
//  TagStruct* tagList, tag;
//
//  if (dict->isDict()) {
//    return NULL;
//  }
//
//  tag = NULL;
//  tagList = dict->tag;
//  while (tagList)
//  {
//    tag = tagList;
//    tagList = tag->tagNext;
//
//    if ( !tag->isKey() ||  tag->keyValue().isEmpty() ) {
//      continue;
//
//    }
//    return tag->tag;
//  }
//
//  return NULL;
//}
//

bool TagStruct::debugIsEqual(const TagStruct& other, const XString8& label) const
{
  if ( *this != other ) {
    MsgLog("Difference at %s\n", label.c_str());
    if ( *this != other ) {
    }
    return false;
  }
  return true;
}

void TagStruct::printf(unsigned int ident) const
{
  XString8 s;
  sprintf(ident, &s);
  MsgLog("%s", s.c_str());
}

// Convenience method
bool TagStruct::isTrue() const
{
  if ( isBool() ) return getBool()->boolValue();
  return false;
}
bool TagStruct::isFalse() const
{
  if ( isBool() ) return !getBool()->boolValue();
  return false;
}
bool TagStruct::isTrueOrYy() const
{
  if ( isBool() ) return getBool()->boolValue();
  if ( isString() && getString()->stringValue().notEmpty() && (getString()->stringValue()[0] == 'y' || getString()->stringValue()[0] == 'Y') ) return true;
  return false;
}
bool TagStruct::isTrueOrYes() const
{
  if ( isBool() ) return getBool()->boolValue();
  if ( isString() && getString()->stringValue().isEqual("Yes"_XS8) ) return true;
  return false;
}
bool TagStruct::isFalseOrNn() const
{
  if ( isBool() ) return !getBool()->boolValue();
  if ( isString() && getString()->stringValue().notEmpty() && (getString()->stringValue()[0] == 'n' || getString()->stringValue()[0] == 'N') ) return true;
  return false;
}


/****************************************  XML  ****************************************/

// Expects to see one dictionary in the XML file, the final pos will be returned
// If the pos is not equal to the strlen, then there are multiple dicts
// Puts the first dictionary it finds in the
// tag pointer and returns the end of the dic, or returns ?? if not found.
//

EFI_STATUS ParseXML(const CHAR8* buffer, TagDict** dict, size_t bufSize)
{
  EFI_STATUS  Status;
  UINT32    length = 0;
  UINT32    pos = 0;
  TagStruct*    tag = NULL;
  CHAR8*    configBuffer = NULL;
  size_t    bufferSize = 0;
  UINTN     i;

  if (bufSize) {
    bufferSize = bufSize;
  } else {
    bufferSize = (UINT32)strlen(buffer);
  }
  DBG("buffer size=%ld\n", bufferSize);
  if(dict == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  configBuffer = (__typeof__(configBuffer))malloc(bufferSize+1);
  memset(configBuffer, 0, bufferSize+1);
  if(configBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  memmove(configBuffer, buffer, bufferSize);
  for (i=0; i<bufferSize; i++) {
    if (configBuffer[i] == 0) {
      configBuffer[i] = 0x20;  //replace random zero bytes to spaces
    }
  }
  buffer_start = configBuffer;
  while (TRUE)
  {
    Status = XMLParseNextTag(configBuffer + pos, &tag, &length);
    DBG("pos=%u\n", pos);
    if (EFI_ERROR(Status)) {
      DBG("error parsing next tag\n");
      break;
    }

    pos += length;

    if (tag == NULL) {
      continue;
    }
    if (tag->isDict()||tag->isArray()) {
      break;
    }

	  tag->FreeTag();
    tag = NULL;
  }
//  FreePool(configBuffer);

  if (EFI_ERROR(Status)) {
    return Status;
  }
  if (tag->isDict()) *dict = tag->getDict();
  else *dict = NULL;
  return EFI_SUCCESS;
}

//
// xml
//

#define DOFREE 1


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
    DBG("NextTag error %s\n", efiStrError(Status));
    return Status;
  }

  pos = length;
  if (!strncmp(tagName, kXMLTagPList, 6)) {
    length=0;
    Status=EFI_SUCCESS;
  }
  /***** dict ****/
  else if (!strcmp(tagName, kXMLTagDict))
  {
    DBG("begin dict len=%d\n", length);
    Status = ParseTagDict(buffer + pos, tag, 0, &length);
  }
  else if (!strcmp(tagName, kXMLTagDict "/"))
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
  else if (!strcmp(tagName, kXMLTagKey))
  {
    DBG("parse key\n");
    Status = ParseTagKey(buffer + pos, tag, &length);
  }
  /***** string ****/
  else if (!strcmp(tagName, kXMLTagString))
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
  else if (!strcmp(tagName, kXMLTagInteger))
  {
    Status = ParseTagInteger(buffer + pos, tag, &length);
  }
  else if (!strncmp(tagName, kXMLTagInteger " ", 8))
  {
    Status = ParseTagInteger(buffer + pos, tag, &length);
  }
  /***** float ****/
  else if (!strcmp(tagName, kXMLTagFloat))
  {
    Status = ParseTagFloat(buffer + pos, tag, &length);
  }
  else if (!strncmp(tagName, kXMLTagFloat " ", 8))
  {
    Status = ParseTagFloat(buffer + pos, tag, &length);
  }
  /***** data ****/
  else if (!strcmp(tagName, kXMLTagData))
  {
    Status = ParseTagData(buffer + pos, tag, &length);
  }
  else if (!strncmp(tagName, kXMLTagData " ", 5))
  {
    Status = ParseTagData(buffer + pos, tag, &length);
  }
  /***** date ****/
  else if (!strcmp(tagName, kXMLTagDate))
  {
    Status = ParseTagDate(buffer + pos, tag, &length);
  }
  /***** FALSE ****/
  else if (!strcmp(tagName, kXMLTagFalse))
  {
    Status = ParseTagBoolean(tag, false, &length);
  }
  /***** TRUE ****/
  else if (!strcmp(tagName, kXMLTagTrue))
  {
    Status = ParseTagBoolean(tag, true, &length);
  }
  /***** array ****/
  else if (!strcmp(tagName, kXMLTagArray))
  {
    Status = ParseTagArray(buffer + pos, tag, 0, &length);
  }
  else if (!strncmp(tagName, kXMLTagArray " ", 6))
  {
    DBG("begin array len=%d\n", length);
    Status = ParseTagArray(buffer + pos, tag, 0, &length);
  }
  else if (!strcmp(tagName, kXMLTagArray "/"))
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
//  TagStruct*    tagTail;
  UINT32    length = 0;

  if (isArray) {
    DBG("parsing array len=%d\n", *lenPtr);
  } else {
    DBG("parsing dict len=%d\n", *lenPtr);
  }
//  tagTail = NULL;
  pos = 0;

  TagStruct* dictOrArrayTag;
  XObjArray<TagStruct>* tagListPtr;
  if (isArray) {
    dictOrArrayTag = TagArray::getEmptyTag();
    tagListPtr = &dictOrArrayTag->getArray()->arrayContent();
  } else {
    dictOrArrayTag = TagDict::getEmptyTag();
    tagListPtr = &dictOrArrayTag->getDict()->dictContent();
  }
  XObjArray<TagStruct>& tagList = *tagListPtr;

  if (!empty) {
    while (TRUE) {
      TagStruct* newDictOrArrayTag = NULL;
      Status = XMLParseNextTag(buffer + pos, &newDictOrArrayTag, &length);
      if (EFI_ERROR(Status)) {
        DBG("error XMLParseNextTag in array: %s\n", efiStrError(Status));
        break;
      }

      pos += length;

      if (newDictOrArrayTag == NULL) {
        break;
      }

      tagList.AddReference(newDictOrArrayTag, true);
    }

    if (EFI_ERROR(Status)) {
      dictOrArrayTag->FreeTag();
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
  TagKey*      tmpTag;
//  TagStruct*      subTag = NULL;

  Status = FixDataMatchingTag(buffer, kXMLTagKey, &length);
  DBG("fixing key len=%d status=%s\n", length, efiStrError(Status));
  if (EFI_ERROR(Status)){
    return Status;
  }

//  Status = XMLParseNextTag(buffer + length, &subTag, &length2);
//  if (EFI_ERROR(Status)) {
//    return Status;
//  }
  tmpTag = TagKey::getEmptyTag();
  tmpTag->setKeyValue(LString8(buffer, strlen(buffer)));

  *tag = tmpTag;
  *lenPtr = length + length2;
  DBG("parse key '%s' success len=%d\n", tmpTag->keyStringValue().c_str(), *lenPtr);
  return EFI_SUCCESS;
}

//==========================================================================
// ParseTagString

EFI_STATUS ParseTagString(CHAR8* buffer, TagStruct* * tag,UINT32* lenPtr)
{
  EFI_STATUS  Status;
  UINT32    length = 0;
  TagString*    tmpTag;

  Status = FixDataMatchingTag(buffer, kXMLTagString, &length);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  tmpTag = TagString::getEmptyTag();
  if (tmpTag == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  size_t outlen = XMLDecode(buffer, strlen(buffer), buffer, strlen(buffer));
  tmpTag->setStringValue(LString8(buffer, outlen));
  *tag = tmpTag;
  *lenPtr = length;
  DBG(" parse string %s\n", tmpTag->getString()->stringValue().c_str());
  return EFI_SUCCESS;
}

//==========================================================================
// ParseTagInteger

EFI_STATUS ParseTagInteger(CHAR8* buffer, TagStruct** tag, UINT32* lenPtr)
{
  EFI_STATUS  Status;
  UINT32    length = 0;
  INTN     integer;
  UINT32    size;
  BOOLEAN   negative = FALSE;
  CHAR8*    val = buffer;
  TagInt64* tmpTag;

  Status = FixDataMatchingTag(buffer, kXMLTagInteger, &length);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  tmpTag = TagInt64::getEmptyTag();
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
        integer = (integer * 16) + (*val++ - 'A' + 10);
      }
      else {
        MsgLog("ParseTagInteger hex error (0x%hhX) in buffer %s\n", *val, buffer);
        //        getchar();
        tmpTag->FreeTag();
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
          tmpTag->FreeTag();
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
  TagFloat*      tmpTag;
  
  Status = FixDataMatchingTag(buffer, kXMLTagFloat, &length);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  tmpTag = TagFloat::getEmptyTag();
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
  TagData*    tmpTag;

  Status = FixDataMatchingTag(buffer, kXMLTagData,&length);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  tmpTag = TagData::getEmptyTag();
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
  TagDate*    tmpTag;

  Status = FixDataMatchingTag(buffer, kXMLTagDate,&length);
  if (EFI_ERROR(Status)) {
    return Status;
  }


  tmpTag = TagDate::getEmptyTag();
  tmpTag->setDateValue(LString8(buffer, length));

  *tag = tmpTag;
  *lenPtr = length;

  return EFI_SUCCESS;
}

//==========================================================================
// ParseTagBoolean

EFI_STATUS ParseTagBoolean(TagStruct** tag, bool value, UINT32* lenPtr)
{
  TagBool* tmpTag;

  tmpTag = TagBool::getEmptyTag();
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

    if ((*endTag == '/') && !strcmp(endTag + 1, tag)) {
      break;
    }
    start += length;
  }
  DBG("fix buffer at pos=%d\n", start + stop);
  buffer[start + stop] = '\0';
  *lenPtr = start + length;

  return EFI_SUCCESS;
}

//==========================================================================


/*
 return TRUE if the property present && value = TRUE
 else return FALSE
 */
BOOLEAN
IsPropertyNotNullAndTrue(const TagStruct* Prop)
{
  return Prop != NULL && Prop->isTrueOrYy();
}

/*
 return TRUE if the property present && value = FALSE
 else return FALSE
 */
BOOLEAN
IsPropertyNotNullAndFalse(const TagStruct* Prop)
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
GetPropertyAsInteger(
                    const TagStruct* Prop,
                    INTN Default
                    )
{
  if (Prop == NULL) {
    return Default;
  }

  if (Prop->isInt64()) {
    return Prop->getInt64()->intValue();
  } else if ((Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
    if ( Prop->getString()->stringValue().length() > 1  &&  (Prop->getString()->stringValue()[1] == 'x' || Prop->getString()->stringValue()[1] == 'X') ) {
      return (INTN)AsciiStrHexToUintn(Prop->getString()->stringValue());
    }

    if (Prop->getString()->stringValue()[0] == '-') {
      return -(INTN)AsciiStrDecimalToUintn (Prop->getString()->stringValue().c_str() + 1);
    }

//    return (INTN)AsciiStrDecimalToUintn (Prop->getString()->stringValue());
    return (INTN)AsciiStrDecimalToUintn((Prop->getString()->stringValue()[0] == '+') ? (Prop->getString()->stringValue().c_str() + 1) : Prop->getString()->stringValue().c_str());
  } else if (Prop->isData()) {

    UINTN Size = Prop->getData()->dataLenValue();
    if (Size > 8) Size = 8;
    INTN Data = 0;
    memcpy(&Data, Prop->getData()->dataValue(), Size);
    return Data;
  }
  return Default;
}

float GetPropertyFloat (const TagStruct* Prop, float Default)
{
  if (Prop == NULL) {
    return Default;
  }
  if (Prop->isFloat()) {
    return Prop->getFloat()->floatValue(); //this is union char* or float
  } else if ((Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
    float fVar = 0.f;
    if(!AsciiStrToFloat(Prop->getString()->stringValue().c_str(), NULL, &fVar)) //if success then return 0
      return fVar;
  }
  
  return Default;
}
