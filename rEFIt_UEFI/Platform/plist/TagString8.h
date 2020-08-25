/*
 * TagString.h
 *
 *  Created on: 23 August 2020
 *      Author: jief
 */

#ifndef __TagString8_h__
#define __TagString8_h__

#include "plist.h"

class TagString : public TagStruct
{
  static XObjArray<TagString> tagsFree;
  XString8 _string;

public:

  TagString() : _string() {}
  TagString(const TagString& other) = delete; // Can be defined if needed
  const TagString& operator = (const TagString&); // Can be defined if needed
  virtual ~TagString() { }
  
  virtual bool operator == (const TagStruct& other) const;

  virtual TagString* getString() { return this; }
  virtual const TagString* getString() const { return this; }

  virtual bool isString() const { return true; }
  virtual const XString8 getTypeAsXString8() const { return "String8"_XS8; }
  static TagString* getEmptyTag();
  virtual void FreeTag();
  
  virtual void sprintf(unsigned int ident, XString8* s) const;

  /*
   *  getters and setters
   */
  const XString8& stringValue() const
  {
//    if ( !isString() ) panic("TagString::stringValue() : !isString() ");
    return _string;
  }
  XString8& stringValue()
  {
//    if ( !isString() ) panic("TagString::stringValue() : !isString() ");
    return _string;
  }
  void setStringValue(const XString8& xstring)
  {
    // empty string is allowed
    //if ( xstring.isEmpty() ) panic("TagStruct::setStringValue() : xstring.isEmpty() ");
    _string = xstring;
  }

};



#endif /* __TagString_h__ */
