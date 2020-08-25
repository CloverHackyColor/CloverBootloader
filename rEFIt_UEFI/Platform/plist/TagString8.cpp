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
#include "TagString8.h"

#ifndef DEBUG_ALL
#define DEBUG_TagString 1
#else
#define DEBUG_TagString DEBUG_ALL
#endif

#if DEBUG_TagString == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_TagString, __VA_ARGS__)
#endif

#include "TagString8.h"

XObjArray<TagString> TagString::tagsFree;


//UINTN newtagcount = 0;
//UINTN tagcachehit = 0;
TagString* TagString::getEmptyTag()
{
  TagString* tag;

  if ( tagsFree.size() > 0 ) {
    tag = &tagsFree[0];
    tagsFree.RemoveWithoutFreeingAtIndex(0);
//tagcachehit++;
//DBG("tagcachehit=%lld\n", tagcachehit);
    return tag;
  }
  tag = new TagString();
//newtagcount += 1;
//DBG("newtagcount=%lld\n", newtagcount);
  return tag;
}

void TagString::FreeTag()
{
  _string.setEmpty();
  tagsFree.AddReference(this, true);
}

bool TagString::operator == (const TagStruct& other) const
{
  if ( !other.isString() ) return false;
  return _string == other.getString()->_string;
}

void TagString::sprintf(unsigned int ident, XString8* s) const
{
  for (size_t i = 0 ; i < (size_t)ident ; i++) *s += " ";
  *s += "<string>"_XS8;
  *s += _string;
  *s += "</string>\n"_XS8;
}
