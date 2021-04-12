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

#ifndef DEBUG_ALL
#define DEBUG_XML 1
#else
#define DEBUG_XML DEBUG_ALL
#endif

#if DEBUG_PLIST == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_XML, __VA_ARGS__)
#endif



/* Function for basic XML character entities parsing */
class XMLEntity
{
public:
  const XString8 name;
  size_t nameLen;
  CHAR8 value;

  XMLEntity() : name(), nameLen(0), value(0) { }
  XMLEntity(const XString8& _name, CHAR8 _value) : name(_name), nameLen(name.length()), value(_value) { }

  // Not sure if default are valid. Delete them. If needed, proper ones can be created
  XMLEntity(const XMLEntity&) = delete;
  XMLEntity& operator=(const XMLEntity&) = delete;

};

const XMLEntity ents[] = {
  { "quot;"_XS8, '"' },
  {"apos;"_XS8,'\''},
  {"lt;"_XS8,  '<'},
  {"gt;"_XS8,  '>'},
  {"amp;"_XS8, '&'}
};

/*
 * Replace XML entities by their value
 * out can be src
 */

size_t XMLDecode(const char* src, size_t srclen, char* out, size_t outlen)
{
  const char* s;
  char* o;

  if (!src) {
    if ( outlen > 0 ) *out = 0;
    return 0;
  }

  // out is always <= src, let's overwrite src
  s = src;
  o = out;
  while (s < src+srclen) /* Make sure the terminator is also copied */
  {
    if ( *s == '&' ) {
      s++;
      size_t i;
      for (i = 0; i < sizeof(ents)/sizeof(ents[0]); i++) {
        if ( ents[i].name.strncmp(s, ents[i].nameLen) == 0 ) {
          if ( uintptr_t(o)-uintptr_t(out) >= outlen ) return uintptr_t(o)-uintptr_t(out);
          *o++ = ents[i].value;
          s += ents[i].nameLen;
          break;
        }
      }
      if ( i < sizeof(ents)/sizeof(ents[0]) ) continue; // if entity is found, let's go up to the beginning of the loop and avoid inserting thenext char without checking if it's another entity
    }
    if ( uintptr_t(o)-uintptr_t(out) >= outlen ) return uintptr_t(o)-uintptr_t(out);
    *o++ = *s++;
  }
  if ( uintptr_t(o)-uintptr_t(out) < outlen ) *o = 0;
  return uintptr_t(o)-uintptr_t(out);
}

char* XMLDecode(char* src, size_t size)
{
  XMLDecode(src, size, src, size+1);
  return src;
}

char* XMLDecode(char* src)
{
  XMLDecode(src, strlen(src), src, strlen(src)+1);
  return src;
}
