/*
 * TagArray.h
 *
 *  Created on: 23 August 2020
 *      Author: jief
 */

#ifndef __TagArray_h__
#define __TagArray_h__

#include "plist.h"

class TagArray : public TagStruct
{
  static XObjArray<TagArray> tagsFree;
  XObjArray<TagStruct> _arrayContent;

public:

  TagArray() : _arrayContent() {}
  TagArray(const TagArray& other) = delete; // Can be defined if needed
  const TagArray& operator = (const TagArray&); // Can be defined if needed
  virtual ~TagArray() { }

  virtual bool operator == (const TagStruct& other) const;
  virtual bool debugIsEqual(const TagStruct& other, const XString8& label) const;

  virtual TagArray* getArray() { return this; }
  virtual const TagArray* getArray() const { return this; }

  virtual bool isArray() const { return true; }
  virtual const XString8 getTypeAsXString8() const { return "Array"_XS8; }
  static TagArray* getEmptyTag();
  virtual void FreeTag();
  
  virtual void sprintf(unsigned int ident, XString8* s) const;

  /*
   *  getters and setters
   */
  const XObjArray<TagStruct>& arrayContent() const
  {
//    if ( !isArray() ) panic("TagArray::arrayContent() const : !isArray() ");
    return _arrayContent;
  }
  XObjArray<TagStruct>& arrayContent()
  {
//    if ( !isArray() ) panic("TagArray::arrayContent() : !isArray() ");
    return _arrayContent;
  }
  const TagStruct* elementAt(size_t idx) const;
  const TagDict* dictElementAt(size_t idx) const;
  const TagDict* dictElementAt(size_t idx, const XString8& currentTag) const; // currentTag is just for error msg
  const TagArray* arrayElementAt(size_t idx) const;
  const TagArray* arrayElementAt(size_t idx, const XString8& currentTag) const; // currentTag is just for error msg
};



#endif /* __TagArray_h__ */
