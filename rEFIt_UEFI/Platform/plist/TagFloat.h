/*
 * TagFloat.h
 *
 *  Created on: 23 August 2020
 *      Author: jief
 */

#ifndef __TagFloat_h__
#define __TagFloat_h__

#include "plist.h"

class TagFloat : public TagStruct
{
  static XObjArray<TagFloat> tagsFree;
  float value;

public:

  TagFloat() : value(0) {}
  TagFloat(const TagFloat& other) = delete; // Can be defined if needed
  const TagFloat& operator = (const TagFloat&); // Can be defined if needed
  virtual ~TagFloat() { }
  
  virtual bool operator == (const TagStruct& other) const;

  virtual TagFloat* getFloat() { return this; }
  virtual const TagFloat* getFloat() const { return this; }

  virtual bool isFloat() const { return true; }
  virtual const XString8 getTypeAsXString8() const { return "Float"_XS8; }
  static TagFloat* getEmptyTag();
  virtual void FreeTag();
  
  virtual void sprintf(unsigned int ident, XString8* s) const;

  /*
   *  getters and setters
   */
  float floatValue() const
  {
//    if ( !isFloat() ) panic("TagFloat::floatValue() : !isFloat() ");
    return value;
  }
  void setFloatValue(float f)
  {
    value = f;
  }

};



#endif /* __TagFloat_h__ */
