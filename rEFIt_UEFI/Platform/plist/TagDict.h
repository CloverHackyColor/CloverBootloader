/*
 * TagDict.h
 *
 *  Created on: 23 August 2020
 *      Author: jief
 */

#ifndef __TagDict_h__
#define __TagDict_h__

#include "plist.h"

class TagsDictFreeArray : public XObjArray<TagDict> {
public:
  ~TagsDictFreeArray() { (void)0; }
};

class TagDict : public TagStruct {
  XObjArray<TagStruct> _dictContent;

public:
#ifdef TagStruct_USE_CACHE
  static TagsDictFreeArray tagsFree;
#endif

  TagDict() : _dictContent() {}
  TagDict(const TagDict &other) = delete;    // Can be defined if needed
  const TagDict &operator=(const TagDict &); // Can be defined if needed
  virtual ~TagDict() {}

  virtual XBool operator==(const TagStruct &other) const;
  virtual XBool debugIsEqual(const TagStruct &other,
                             const XString8 &label) const;

  virtual TagDict *getDict() { return this; }
  virtual const TagDict *getDict() const { return this; }

  virtual XBool isDict() const { return true; }
  virtual const XString8 getTypeAsXString8() const { return "Dict"_XS8; }
  static TagDict *getEmptyTag();
  virtual void ReleaseTag();

  virtual void sprintf(unsigned int ident, XString8 *s) const;

  /*
   *  getters and setters
   */
  const XObjArray<TagStruct> &dictContent() const {
#ifdef JIEF_DEBUG
    if (!isDict())
      panic("TagDict::dictContent() : !isDict() ");
#endif
    return _dictContent;
  }
  XObjArray<TagStruct> &dictContent() {
#ifdef JIEF_DEBUG
    if (!isDict())
      panic("TagDict::dictContent() : !isDict() ");
#endif
    return _dictContent;
  }
  INTN dictKeyCount() const;
  EFI_STATUS getKeyAndValueAtIndex(INTN id, const TagKey **key,
                                   const TagStruct **value) const;
  const TagStruct *propertyForKey(const CHAR8 *key) const;
  const TagDict *dictPropertyForKey(const CHAR8 *key) const;
  const TagArray *arrayPropertyForKey(const CHAR8 *key) const;
};

EFI_STATUS
ParseXML(CONST UINT8 *buffer, TagDict **dict, size_t bufSize);

#endif /* __TagDict_h__ */
