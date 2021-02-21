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
  static XObjArray<TagDate> tagsFree;
  XString8 string;

public:

  TagDate() : string() {}
  TagDate(const TagDate& other) = delete; // Can be defined if needed
  const TagDate& operator = (const TagDate&); // Can be defined if needed
  virtual ~TagDate() { }
  
  virtual bool operator == (const TagStruct& other) const;

  virtual TagDate* getDate() { return this; }
  virtual const TagDate* getDate() const { return this; }

  virtual bool isDict() const { return true; }
  virtual const XString8 getTypeAsXString8() const { return "Dict"_XS8; }
  static TagDate* getEmptyTag();
  virtual void FreeTag();
  
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
//    if ( xstring.isEmpty() ) panic("TagDate::setDateValue() : xstring.isEmpty() ");
    if ( xstring.isEmpty() ) return; //do nothing rather then assign empty date
    string = xstring;
  }

};



#endif /* __TagDate_h__ */
