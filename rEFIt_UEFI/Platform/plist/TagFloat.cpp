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
#include "TagFloat.h"

#ifndef DEBUG_ALL
#define DEBUG_TagFloat 1
#else
#define DEBUG_TagFloat DEBUG_ALL
#endif

#if DEBUG_TagFloat == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_TagFloat, __VA_ARGS__)
#endif


#include "TagFloat.h"

#ifdef TagStruct_USE_CACHE
XObjArray<TagFloat> TagFloat::tagsFree;
#endif


TagFloat* TagFloat::getEmptyTag()
{
  TagFloat* tag;

#ifdef TagStruct_USE_CACHE
  if ( tagsFree.size() > 0 ) {
    tag = &tagsFree[0];
    tagsFree.RemoveWithoutFreeingAtIndex(0);
    #ifdef TagStruct_COUNT_CACHEHITMISS
      cachehit++;
     #endif
    return tag;
  }
#endif
  tag = new TagFloat;
  #ifdef TagStruct_COUNT_CACHEHITMISS
    cachemiss++;
   #endif
  return tag;
}

void TagFloat::ReleaseTag()
{
  value = 0;
#ifdef TagStruct_USE_CACHE
  tagsFree.AddReference(this, true);
#else
  delete this;
#endif
}

XBool TagFloat::operator == (const TagStruct& other) const
{
  if ( !other.isFloat() ) return false;
  return value == other.getFloat()->value;
}

void TagFloat::sprintf(unsigned int ident, XString8* s) const
{
  for (size_t i = 0 ; i < (size_t)ident ; i++) *s += " ";
  *s += S8Printf("%f", value);
}
