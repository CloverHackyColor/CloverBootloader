/*
 * TagKey.h
 *
 *  Created on: 23 August 2020
 *      Author: jief
 */

#ifndef __TagKey_h__
#define __TagKey_h__

#include "plist.h"

class TagKey : public TagStruct
{
  static XObjArray<TagKey> tagsFree;
  XString8 _string;

public:

  TagKey() : _string() {}
  TagKey(const TagKey& other) = delete; // Can be defined if needed
  const TagKey& operator = (const TagKey&); // Can be defined if needed
  virtual ~TagKey() { }
  
  virtual bool operator == (const TagStruct& other) const;

  virtual TagKey* getKey() { return this; }
  virtual const TagKey* getKey() const { return this; }

  virtual bool isKey() const { return true; }
  virtual const XString8 getTypeAsXString8() const { return "Key"_XS8; }
  static TagKey* getEmptyTag();
  virtual void FreeTag();
  
  virtual void sprintf(unsigned int ident, XString8* s) const;

  /*
   *  getters and setters
   */
  const XString8& keyStringValue() const
  {
//    if ( !isKey() ) panic("TagKey::keyStringValue() const : !isKey() ");
    return _string;
  }
  XString8& keyStringValue()
  {
//    if ( !isKey() ) panic("TagKey::keyStringValue() : !isKey() ");
    return _string;
  }
  void setKeyValue(const XString8& xstring)
  {
    if ( xstring.isEmpty() ) log_technical_bug("TagKey::setKeyValue() : xstring.isEmpty() ");
    _string = xstring;
  }

};



#endif /* __TagKey_h__ */
