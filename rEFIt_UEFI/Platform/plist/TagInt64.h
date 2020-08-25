/*
 * TagInt64.h
 *
 *  Created on: 23 August 2020
 *      Author: jief
 */

#ifndef __TagInt64_h__
#define __TagInt64_h__

#include "plist.h"

class TagInt64 : public TagStruct
{
  static XObjArray<TagInt64> tagsFree;
  INT64 value;

public:

  TagInt64() : value(0) {}
  TagInt64(const TagInt64& other) = delete; // Can be defined if needed
  const TagInt64& operator = (const TagInt64&); // Can be defined if needed
  virtual ~TagInt64() { }
  
  virtual bool operator == (const TagStruct& other) const;

  virtual TagInt64* getInt64() { return this; }
  virtual const TagInt64* getInt64() const { return this; }

  virtual bool isInt64() const { return true; }
  virtual const XString8 getTypeAsXString8() const { return "Int64"_XS8; }
  static TagInt64* getEmptyTag();
  virtual void FreeTag();
  
  virtual void sprintf(unsigned int ident, XString8* s) const;

  /*
   *  getters and setters
   */
  INTN intValue() const
  {
//    if ( !isInt() ) panic("TagInt64::intValue() : !isInt() ");
    return value;
  }
  void setIntValue(INTN i)
  {
    value = i;
  }

};



#endif /* __TagInt64_h__ */
