/*
 * TagBool.h
 *
 *  Created on: 23 August 2020
 *      Author: jief
 */

#ifndef __TagBool_h__
#define __TagBool_h__

#include "plist.h"

class TagBool : public TagStruct
{
  static XObjArray<TagBool> tagsFree;
  XBool value;

public:

  TagBool() : value(false) {}
  TagBool(const TagBool& other) = delete; // Can be defined if needed
  const TagBool& operator = (const TagBool&); // Can be defined if needed
  virtual ~TagBool() { }
  
  virtual XBool operator == (const TagStruct& other) const;

  virtual TagBool* getBool() { return this; }
  virtual const TagBool* getBool() const { return this; }

  virtual XBool isBool() const { return true; }
  virtual const XString8 getTypeAsXString8() const { return "Bool"_XS8; }
  static TagBool* getEmptyTag();
  virtual void FreeTag();
  
  virtual void sprintf(unsigned int ident, XString8* s) const;

  /*
   *  getters and setters
   */
  XBool boolValue() const
  {
//    if ( !isBool() ) panic("TagBool::boolValue() : !isBool() ");
    return value;
  }
  void setBoolValue(XBool b)
  {
//    if ( !isBool() ) panic("TagBool::setIntValue(XBool) : !isBool() ");
    value = b;
  }

};



#endif /* __TagBool_h__ */
