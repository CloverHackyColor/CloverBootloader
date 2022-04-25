/*
 * plist.h
 *
 *  Created on: 31 Mar 2020
 *      Author: jief
 */

#ifndef PLATFORM_PLIST_H_
#define PLATFORM_PLIST_H_

/* XML Tags */
#define kXMLTagPList     "plist"
#define kXMLTagDict      "dict"
#define kXMLTagKey       "key"
#define kXMLTagString    "string"
#define kXMLTagInteger   "integer"
#define kXMLTagData      "data"
#define kXMLTagDate      "date"
#define kXMLTagFalse     "false/"
#define kXMLTagTrue      "true/"
#define kXMLTagArray     "array"
#define kXMLTagReference "reference"
#define kXMLTagID        "ID="
#define kXMLTagIDREF     "IDREF="
#define kXMLTagFloat     "real"

#include "../../include/TagTypes.h"

class TagDict;
class TagKey;
class TagString;
class TagInt64;
class TagFloat;
class TagBool;
class TagData;
class TagDate;
class TagArray;

class TagStruct
{
public:

  TagStruct() {}
  TagStruct(const TagStruct& other) = delete; // Can be defined if needed
  const TagStruct& operator = ( const TagStruct & ) = delete; // Can be defined if needed
  virtual ~TagStruct() { }

//  static TagStruct* getEmptyTag();
//  static TagStruct* getEmptyDictTag();
//  static TagStruct* getEmptyArrayTag();
  virtual void FreeTag() = 0;
  
  virtual XBool operator == (const TagStruct& other) const = 0;
  virtual XBool operator != (const TagStruct& other) const { return !(*this == other); };
  virtual XBool debugIsEqual(const TagStruct& other, const XString8& label) const;

  virtual void sprintf(unsigned int ident, XString8* s) const = 0;
  void printf(unsigned int ident) const;
  virtual void printf() const { printf(0); }

  virtual const TagDict* getDict() const { panic("getDict() called on a tag of type %s.", this->getTypeAsXString8().c_str()); return NULL; }
  virtual const TagKey* getKey() const { log_technical_bug("getKey() called on a tag of type %s.", this->getTypeAsXString8().c_str()); return NULL; }
  virtual const TagString* getString() const { log_technical_bug("getString() called on a tag of type %s.", this->getTypeAsXString8().c_str()); return NULL; }
  virtual const TagInt64* getInt64() const { log_technical_bug("getInt64() called on a tag of type %s.", this->getTypeAsXString8().c_str()); return NULL; }
  virtual const TagFloat* getFloat() const { log_technical_bug("getFloat() called on a tag of type %s.", this->getTypeAsXString8().c_str()); return NULL; }
  virtual const TagBool* getBool() const { log_technical_bug("getBool() called on a tag of type %s.", this->getTypeAsXString8().c_str()); return NULL; }
  virtual const TagData* getData() const { log_technical_bug("getData() called on a tag of type %s.", this->getTypeAsXString8().c_str()); return NULL; }
  virtual const TagDate* getDate() const { log_technical_bug("getDate() called on a tag of type %s.", this->getTypeAsXString8().c_str()); return NULL; }
  virtual const TagArray* getArray() const { log_technical_bug("getArray() called on a tag of type %s.", this->getTypeAsXString8().c_str()); return NULL; }

  virtual TagDict* getDict() { log_technical_bug("getDict() called on a tag of type %s.", this->getTypeAsXString8().c_str()); return NULL; }
  virtual TagKey* getKey() { log_technical_bug("getKey() called on a tag of type %s.", this->getTypeAsXString8().c_str()); return NULL; }
  virtual TagString* getString() { log_technical_bug("getString() called on a tag of type %s.", this->getTypeAsXString8().c_str()); return NULL; }
  virtual TagInt64* getInt64() { log_technical_bug("getInt64() called on a tag of type %s.", this->getTypeAsXString8().c_str()); return NULL; }
  virtual TagFloat* getFloat() { log_technical_bug("getFloat() called on a tag of type %s.", this->getTypeAsXString8().c_str()); return NULL; }
  virtual TagBool* getBool() { log_technical_bug("getBool() called on a tag of type %s.", this->getTypeAsXString8().c_str()); return NULL; }
  virtual TagData* getData() { log_technical_bug("getData() called on a tag of type %s.", this->getTypeAsXString8().c_str()); return NULL; }
  virtual TagDate* getDate() { log_technical_bug("getDate() called on a tag of type %s.", this->getTypeAsXString8().c_str()); return NULL; }
  virtual TagArray* getArray() { log_technical_bug("getArray() called on a tag of type %s.", this->getTypeAsXString8().c_str()); return NULL; }

  virtual XBool isDict() const { return false; }
  virtual XBool isKey() const { return false; }
  virtual XBool isString() const { return false; }
  virtual XBool isInt64() const { return false; }
  virtual XBool isFloat() const { return false; }
  virtual XBool isBool() const { return false; }
  virtual XBool isData() const { return false; }
  virtual XBool isDate() const { return false; }
  virtual XBool isArray() const { return false; }

  virtual const XString8 getTypeAsXString8() const = 0;


  // Convenience method
  XBool isTrue() const;
  XBool isFalse() const;
  XBool isTrueOrYy() const;
  XBool isTrueOrYes() const;
  XBool isFalseOrNn() const;
};

#include "TagDict.h"
#include "TagKey.h"
#include "TagBool.h"
#include "TagData.h"
#include "TagDate.h"
#include "TagArray.h"
#include "TagFloat.h"
#include "TagInt64.h"
#include "TagString8.h"

EFI_STATUS
GetNextTag (
  UINT8  *buffer,
  CHAR8  **tag,
  UINT32 *start,
  UINT32 *length
  );

XBool
IsPropertyNotNullAndTrue(
  const TagStruct* Prop
  );

XBool
IsPropertyNotNullAndFalse(
  const TagStruct* Prop
  );

INTN
GetPropertyAsInteger(
  const TagStruct* Prop,
  INTN Default
  );

float GetPropertyFloat (const TagStruct* Prop, float Default);


#endif /* PLATFORM_PLIST_H_ */
