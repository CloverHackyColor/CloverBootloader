/*
 * TagDict.h
 *
 *  Created on: 23 August 2020
 *      Author: jief
 */

#ifndef __TagDict_h__
#define __TagDict_h__

#include "plist.h"

class TagDict : public TagStruct
{
  static XObjArray<TagDict> tagsFree;
  XObjArray<TagStruct> _dictContent;

public:

  TagDict() : _dictContent() {}
  TagDict(const TagDict& other) = delete; // Can be defined if needed
  const TagDict& operator = (const TagDict&); // Can be defined if needed
  virtual ~TagDict() { }
  
  virtual bool operator == (const TagStruct& other) const;
  virtual bool debugIsEqual(const TagStruct& other, const XString8& label) const;

  virtual TagDict* getDict() { return this; }
  virtual const TagDict* getDict() const { return this; }

  virtual bool isDict() const { return true; }
  virtual const XString8 getTypeAsXString8() const { return "Dict"_XS8; }
  static TagDict* getEmptyTag();
  virtual void FreeTag();
  
  virtual void sprintf(unsigned int ident, XString8* s) const;

  /*
   *  getters and setters
   */
  const XObjArray<TagStruct>& dictContent() const
  {
    if ( !isDict() ) panic("TagDict::dictContent() : !isDict() ");
    return _dictContent;
  }
  XObjArray<TagStruct>& dictContent()
  {
    if ( !isDict() ) panic("TagDict::dictContent() : !isDict() ");
    return _dictContent;
  }
  INTN dictKeyCount() const;
  EFI_STATUS getKeyAndValueAtIndex(INTN id, const TagKey** key, const TagStruct** value) const;
  const TagStruct* propertyForKey(const CHAR8* key ) const;
  const TagDict* dictPropertyForKey(const CHAR8* key) const;
  const TagArray* arrayPropertyForKey(const CHAR8* key) const;

};


EFI_STATUS
ParseXML(
  CONST CHAR8  *buffer,
        TagDict** dict,
        size_t bufSize
  );



#endif /* __TagDict_h__ */
