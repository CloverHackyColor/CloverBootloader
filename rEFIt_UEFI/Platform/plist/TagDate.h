/*
 * TagDate.h
 *
 *  Created on: 23 August 2020
 *      Author: jief
 */

#ifndef __TagDate_h__
#define __TagDate_h__

#include "plist.h"

class TagDate : public TagStruct
{
  XString8 string;

public:
#ifdef TagStruct_USE_CACHE
  static XObjArray<TagDate> tagsFree;
#endif

  TagDate() : string() {}
  TagDate(const TagDate& other) = delete; // Can be defined if needed
  const TagDate& operator = (const TagDate&); // Can be defined if needed
  virtual ~TagDate() { }
  
  virtual XBool operator == (const TagStruct& other) const;

  virtual TagDate* getDate() { return this; }
  virtual const TagDate* getDate() const { return this; }

  virtual XBool isDict() const { return true; }
  virtual const XString8 getTypeAsXString8() const { return "Dict"_XS8; }
  static TagDate* getEmptyTag();
  virtual void ReleaseTag();
  
  virtual void sprintf(unsigned int ident, XString8* s) const;

  /*
   *  getters and setters
   */
  const XString8& dateValue()
  {
//    if ( !isDate() ) panic("TagDate::dateValue() : !isDate() ");
    return string;
  }
  void setDateValue(const XString8& xstring)
  {
    string = xstring;
  }
  void setDateValue(const char* value, size_t length)
  {
    string.strncpy(value, length); // strncpy can handle value==NULL, *value=0 and length=0
  }

};



#endif /* __TagDate_h__ */
