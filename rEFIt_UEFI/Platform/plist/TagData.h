/*
 * TagData.h
 *
 *  Created on: 23 August 2020
 *      Author: jief
 */

#ifndef __TagData_h__
#define __TagData_h__

#include "plist.h"

class TagData : public TagStruct
{
  static XObjArray<TagData> tagsFree;
  UINT8  *_data;
  UINTN  _dataLen;

public:

  TagData() : _data(NULL), _dataLen(0) {}
  TagData(const TagData& other) = delete; // Can be defined if needed
  const TagData& operator = (const TagData&); // Can be defined if needed
  virtual ~TagData() { delete _data; }
  
  virtual bool operator == (const TagStruct& other) const;

  virtual TagData* getData() { return this; }
  virtual const TagData* getData() const { return this; }

  virtual bool isData() const { return true; }
  virtual const XString8 getTypeAsXString8() const { return "Data"_XS8; }
  static TagData* getEmptyTag();
  virtual void FreeTag();
  
  virtual void sprintf(unsigned int ident, XString8* s) const;

  /*
   *  getters and setters
   */
  const UINT8* dataValue() const
  {
//    if ( !isData() ) panic("TagData::dataValue() : !isData() ");
    return _data;
  }
  UINT8* dataValue()
  {
//    if ( !isData() ) panic("TagData::dataValue() : !isData() ");
    return _data;
  }
  UINTN dataLenValue() const
  {
//    if ( !isData() ) panic("TagData::dataLenValue() : !isData() ");
    return _dataLen;
  }
  void setDataValue(UINT8* data, UINTN dataLen)
  {
    if ( data == NULL ) panic("TagData::setDataValue() : _data == NULL ");
    _data = data;
    _dataLen = dataLen;
  }

};



#endif /* __TagData_h__ */
