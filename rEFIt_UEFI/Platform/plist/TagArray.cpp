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
#include "TagArray.h"

#ifndef DEBUG_ALL
#define DEBUG_TagArray 1
#else
#define DEBUG_TagArray DEBUG_ALL
#endif

#if DEBUG_TagArray == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_TagArray, __VA_ARGS__)
#endif

#include "TagArray.h"

XObjArray<TagArray> TagArray::tagsFree;

bool TagArray::operator == (const TagStruct& other) const
{
  if ( !other.isArray() ) return false;
  if ( _arrayContent.size() != other.getArray()->arrayContent().size() ) {
    return false;
  }
  for (size_t tagIdx = 0 ; tagIdx < _arrayContent.size() ; tagIdx++ ) {
    if ( _arrayContent[tagIdx] != other.getArray()->arrayContent()[tagIdx] ) {
      return false;
    }
  }
  return true;
}

bool TagArray::debugIsEqual(const TagStruct& other, const XString8& label) const
{
  if ( !other.isArray() ) {
    MsgLog("counterpart of '%s' is not an array\n", label.c_str());
    return false;
  }
  if ( _arrayContent.size() != other.getArray()->arrayContent().size() ) {
    MsgLog("array '%s' is different size\n", label.c_str());
    return false;
  }
  for (size_t tagIdx = 0 ; tagIdx < _arrayContent.size() ; tagIdx++ ) {
    if ( !_arrayContent[tagIdx].debugIsEqual(other.getArray()->arrayContent()[tagIdx], label + "/array"_XS8) ) {
      return false;
    }
  }
  return true;
}


//UINTN newtagcount = 0;
//UINTN tagcachehit = 0;
TagArray* TagArray::getEmptyTag()
{
  TagArray* tag;

  if ( tagsFree.size() > 0 ) {
    tag = &tagsFree[0];
    tagsFree.RemoveWithoutFreeingAtIndex(0);
//tagcachehit++;
//DBG("tagcachehit=%lld\n", tagcachehit);
    return tag;
  }
  tag = new TagArray();
//newtagcount += 1;
//DBG("newtagcount=%lld\n", newtagcount);
  return tag;
}

void TagArray::FreeTag()
{
  //while ( tagIdx < _dictOrArrayContent.notEmpty() ) {
  //  _dictOrArrayContent[0].FreeTag();
  //  _dictOrArrayContent.RemoveWithoutFreeingAtIndex(0);
  //}
  // this loop is better because removing objects from the end don't do any memory copying.
  for (size_t tagIdx = _arrayContent.size() ; tagIdx > 0  ; ) {
    tagIdx--;
    _arrayContent[tagIdx].FreeTag();
    _arrayContent.RemoveWithoutFreeingAtIndex(tagIdx);
  }
  tagsFree.AddReference(this, true);
}

const TagStruct* TagArray::elementAt(size_t idx) const
{
  if ( idx >= _arrayContent.size() ) {
#ifdef DEBUG
    panic("TagArray::elementAt(%zu) -> trying to access element at %zu, but array has only %zu element(s)\n", idx, idx, _arrayContent.size());
#else
    return 0;
#endif
  }
  return &_arrayContent[idx];
}


const TagDict* TagArray::dictElementAt(size_t idx, const XString8& currentTag) const
{
  const TagStruct* tag = elementAt(idx);
  if ( !tag->isDict() ) {
#ifdef DEBUG
    panic("MALFORMED PLIST in '%s' : TagArray::dictElementAt(%zu) -> trying to get a dict element at %zu, but element is %s\n", currentTag.c_str(), idx, idx, tag->getTypeAsXString8().c_str());
#else
    return 0;
#endif
  }
  return _arrayContent[idx].getDict();
}

const TagArray* TagArray::arrayElementAt(size_t idx, const XString8& currentTag) const
{
  const TagStruct* tag = elementAt(idx);
  if ( !tag->isArray() ) {
#ifdef DEBUG
    panic("MALFORMED PLIST in '%s' : TagArray::dictElementAt(%zu) -> trying to get a array element at %zu, but element is %s\n", currentTag.c_str(), idx, idx, tag->getTypeAsXString8().c_str());
#else
    return 0;
#endif
  }
  return _arrayContent[idx].getArray();
}

const TagDict* TagArray::dictElementAt(size_t idx) const
{
  const TagStruct* tag = elementAt(idx);
  if ( !tag->isDict() ) {
#ifdef DEBUG
    panic("MALFORMED PLIST : TagArray::dictElementAt(%zu) -> trying to get a dict element at %zu, but element is %s\n", idx, idx, tag->getTypeAsXString8().c_str());
#else
    return 0;
#endif
  }
  return _arrayContent[idx].getDict();
}

const TagArray* TagArray::arrayElementAt(size_t idx) const
{
  const TagStruct* tag = elementAt(idx);
  if ( !tag->isArray() ) {
#ifdef DEBUG
    panic("MALFORMED PLIST : TagArray::dictElementAt(%zu) -> trying to get a array element at %zu, but element is %s\n", idx, idx, tag->getTypeAsXString8().c_str());
#else
    return 0;
#endif
  }
  return _arrayContent[idx].getArray();
}

void TagArray::sprintf(unsigned int ident, XString8* s) const
{
  for (size_t i = 0 ; i < (size_t)ident ; i++) *s += " ";
  *s += "<array>\n"_XS8;
  for (size_t i = 0 ; i < _arrayContent.size() ; i++) {
    _arrayContent[i].sprintf(ident+8, s);
  }
  for (size_t i = 0 ; i < (size_t)ident ; i++) *s += " ";
  *s += "</array>\n"_XS8;
}
