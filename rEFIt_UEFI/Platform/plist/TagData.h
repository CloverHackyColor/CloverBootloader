/*
 * TagData.h
 *
 *  Created on: 23 August 2020
 *      Author: jief
 */

#ifndef __TagData_h__
#define __TagData_h__

#include "plist.h"
#include "../../cpp_foundation/XBuffer.h"

class TagData : public TagStruct
{
  static XObjArray<TagData> tagsFree;
  XBuffer<UINT8> dataBuffer;

public:

  TagData() : dataBuffer() {}
  TagData(const TagData& other) = delete; // Can be defined if needed
  const TagData& operator = (const TagData&); // Can be defined if needed
  virtual ~TagData() { }
  
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
    return dataBuffer.data();
  }
  UINT8* dataValue()
  {
//    if ( !isData() ) panic("TagData::dataValue() : !isData() ");
    return dataBuffer.data();
  }
  UINTN dataLenValue() const
  {
//    if ( !isData() ) panic("TagData::dataLenValue() : !isData() ");
    return dataBuffer.size();
  }
  void setDataValue(UINT8* data, UINTN dataLen)
  {
    if ( data == NULL && dataLen != 0 ) panic("TagData::setDataValue() : data == NULL && dataLen != 0 ");
    if ( data != NULL && dataLen == 0 ) panic("TagData::setDataValue() : data != NULL && dataLen == 0 ");
    dataBuffer.stealValueFrom(data, dataLen);
  }

};



#endif /* __TagData_h__ */
