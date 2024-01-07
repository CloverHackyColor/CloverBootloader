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
  XString8 _string;

public:
#ifdef TagStruct_USE_CACHE
  static XObjArray<TagKey> tagsFree;
#endif

  TagKey() : _string() {}
  TagKey(const TagKey& other) = delete; // Can be defined if needed
  const TagKey& operator = (const TagKey&); // Can be defined if needed
  virtual ~TagKey() { }
  
  virtual XBool operator == (const TagStruct& other) const;

  virtual TagKey* getKey() { return this; }
  virtual const TagKey* getKey() const { return this; }

  virtual XBool isKey() const { return true; }
  virtual const XString8 getTypeAsXString8() const { return "Key"_XS8; }
  static TagKey* getEmptyTag();
  virtual void ReleaseTag();
  
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
//    if ( xstring.isEmpty() ) log_technical_bug("TagKey::setKeyValue() : xstring.isEmpty() ");
    _string = xstring;
  }
  void setKeyValue(const char* value, size_t length)
  {
//    if ( value == NULL ) log_technical_bug("TagKey::setKeyValue() : value==NULL ");
//    if ( *value == 0 ) log_technical_bug("TagKey::setKeyValue() : *value==0 ");
    _string.strncpy(value, length);
  }

};



#endif /* __TagKey_h__ */
