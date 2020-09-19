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
#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "TagDict.h"

#ifndef DEBUG_ALL
#define DEBUG_TagDict 1
#else
#define DEBUG_TagDict DEBUG_ALL
#endif

#if DEBUG_TagDict == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_TagDict, __VA_ARGS__)
#endif

#include "TagDict.h"

XObjArray<TagDict> TagDict::tagsFree;

//UINTN newtagcount = 0;
//UINTN tagcachehit = 0;
TagDict* TagDict::getEmptyTag()
{
  TagDict* tag;

  if ( tagsFree.size() > 0 ) {
    tag = &tagsFree[0];
    tagsFree.RemoveWithoutFreeingAtIndex(0);
//tagcachehit++;
//DBG("tagcachehit=%lld\n", tagcachehit);
    return tag;
  }
  tag = new TagDict();
//newtagcount += 1;
//DBG("newtagcount=%lld\n", newtagcount);
  return tag;
}

void TagDict::FreeTag()
{
  for (size_t tagIdx = _dictContent.size() ; tagIdx > 0  ; ) {
    tagIdx--;
    _dictContent[tagIdx].FreeTag();
    _dictContent.RemoveWithoutFreeingAtIndex(tagIdx);
  }
  tagsFree.AddReference(this, true);
}

bool TagDict::operator == (const TagStruct& other) const
{
  if ( !other.isDict() ) return false;
  if ( _dictContent.size() != other.getDict()->_dictContent.size() ) {
    return false;
  }
  for (size_t tagIdx = 0 ; tagIdx < _dictContent.size() ; tagIdx++ ) {
    if ( _dictContent[tagIdx] != other.getDict()->_dictContent[tagIdx] ) {
      return false;
    }
  }
  return true;
}

bool TagDict::debugIsEqual(const TagStruct& other, const XString8& label) const
{
  if ( !other.isDict()) {
    MsgLog("counterpart of '%s' is not a dict\n", label.c_str());
    return false;
  }
  if ( _dictContent.size() != other.getDict()->_dictContent.size() ) {
    MsgLog("array '%s' is different size\n", label.c_str());
    return false;
  }
  for (size_t tagIdx = 0 ; tagIdx < _dictContent.size() ; tagIdx++ ) {
    XString8 subLabel = label;
    if ( _dictContent[tagIdx].isKey() ) subLabel += "/dict key=" + _dictContent[tagIdx].getKey()->keyStringValue();
    else if ( tagIdx > 0  && _dictContent[tagIdx-1].isKey() ) subLabel += "/dict value of key=" + _dictContent[tagIdx-1].getKey()->keyStringValue();
    else subLabel += S8Printf("/dict value of key %zu", tagIdx);
    if ( !_dictContent[tagIdx].debugIsEqual(other.getDict()->_dictContent[tagIdx], subLabel ) ) {
      return false;
    }
  }
  return true;
}


INTN TagDict::dictKeyCount() const
{
  if ( !isDict() ) panic("TagStruct::dictKeyCount() : !isDict() ");
  INTN count = 0;
  for (size_t tagIdx = 0 ; tagIdx + 1 < _dictContent.size() ; tagIdx++ ) { // tagIdx + 1 because a key as a last element cannot have value and is ignored. Can't do size()-1, because it's unsigned.
    if ( _dictContent[tagIdx].isKey()  &&   !_dictContent[tagIdx+1].isKey() ) { // if this key is followed by another key, it'll be ignored
      count++;
    }
  }
  return count;
}

EFI_STATUS TagDict::getKeyAndValueAtIndex(INTN id, const TagKey** key, const TagStruct** value) const
{
  INTN element = 0;
  *key = NULL;
  *value = NULL;

  if ( id < 0 ) return EFI_UNSUPPORTED;

  const XObjArray<TagStruct>& tagList = _dictContent;
  size_t tagIdx;
  for (tagIdx = 0 ; tagIdx + 1 < tagList.size() ; tagIdx++ ) { // tagIdx + 1 because a key as a last element cannot have value and is ignored. Can't do size()-1, because it's unsigned.
    if ( tagList[tagIdx].isKey()  &&  !tagList[tagIdx+1].isKey() ) {
      if ( element == id ) {
        *key = tagList[tagIdx].getKey();
        *value = &tagList[tagIdx+1];
        return EFI_SUCCESS;
      }
      element++;
    }
  }
  return EFI_UNSUPPORTED;
}

const TagStruct* TagDict::propertyForKey(const CHAR8* key) const
{
  const XObjArray<TagStruct>& tagList = _dictContent;
  for (size_t tagIdx = 0 ; tagIdx < tagList.size() ; tagIdx++ )
  {
    if ( tagList[tagIdx].isKey()  &&  tagList[tagIdx].getKey()->keyStringValue().equalIC(key) ) {
      if ( tagIdx+1 >= tagList.size() ) return NULL;
      if ( tagList[tagIdx+1].isKey() ) return NULL;
      return &tagList[tagIdx+1];
    }
  }

  return NULL;
}

const TagDict* TagDict::dictPropertyForKey(const CHAR8* key) const
{
  const TagStruct* tag = propertyForKey(key);
  if ( tag == NULL ) return NULL;
  if ( !tag->isDict() ) {
    panic("MALFORMED PLIST : Property value for key %s must be a dict\n", key);
  }
  return tag->getDict();
}

const TagArray* TagDict::arrayPropertyForKey(const CHAR8* key) const
{
  const TagStruct* tag = propertyForKey(key);
  if ( tag == NULL ) return NULL;
  if ( !tag->isArray() ) {
    panic("MALFORMED PLIST : Property value for key %s must be an array\n", key);
  }
  return tag->getArray();
}


void TagDict::sprintf(unsigned int ident, XString8* s) const
{
  for (size_t i = 0 ; i < (size_t)ident ; i++) *s += " ";
  *s += "<dict>\n"_XS8;
  for (size_t i = 0 ; i < _dictContent.size() ; i++) {
    _dictContent[i].sprintf(ident+8, s);
  }
  for (size_t i = 0 ; i < (size_t)ident ; i++) *s += " ";
  *s += "</dict>\n"_XS8;
}
