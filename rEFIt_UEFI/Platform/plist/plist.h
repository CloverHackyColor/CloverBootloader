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


typedef enum {
  kTagTypeNone,
  kTagTypeDict,   // 1
  kTagTypeKey,    // 2
  kTagTypeString, // 3
  kTagTypeInteger,// 4
  kTagTypeData,   // 5
  kTagTypeDate,   // 6
  kTagTypeFalse,  // 7
  kTagTypeTrue,   // 8
  kTagTypeArray,  // 9
  kTagTypeFloat   // 10
} TAG_TYPE;

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
  
  virtual bool operator == (const TagStruct& other) const = 0;
  virtual bool operator != (const TagStruct& other) const { return !(*this == other); };
  virtual bool debugIsEqual(const TagStruct& other, const XString8& label) const;

  virtual void sprintf(unsigned int ident, XString8* s) const = 0;
  void printf(unsigned int ident) const;
  virtual void printf() const { printf(0); }

  virtual const TagDict* getDict() const { return NULL; }
  virtual const TagKey* getKey() const { return NULL; }
  virtual const TagString* getString() const { return NULL; }
  virtual const TagInt64* getInt64() const { return NULL; }
  virtual const TagFloat* getFloat() const { return NULL; }
  virtual const TagBool* getBool() const { return NULL; }
  virtual const TagData* getData() const { return NULL; }
  virtual const TagDate* getDate() const { return NULL; }
  virtual const TagArray* getArray() const { return NULL; }

  virtual TagDict* getDict() { return NULL; }
  virtual TagKey* getKey() { return NULL; }
  virtual TagString* getString() { return NULL; }
  virtual TagInt64* getInt64() { return NULL; }
  virtual TagFloat* getFloat() { return NULL; }
  virtual TagBool* getBool() { return NULL; }
  virtual TagData* getData() { return NULL; }
  virtual TagDate* getDate() { return NULL; }
  virtual TagArray* getArray() { return NULL; }

  virtual bool isDict() const { return false; }
  virtual bool isKey() const { return false; }
  virtual bool isString() const { return false; }
  virtual bool isInt64() const { return false; }
  virtual bool isFloat() const { return false; }
  virtual bool isBool() const { return false; }
  virtual bool isData() const { return false; }
  virtual bool isDate() const { return false; }
  virtual bool isArray() const { return false; }

  virtual const XString8 getTypeAsXString8() const = 0;


  // Convenience method
  bool isTrue() const;
  bool isFalse() const;
  bool isTrueOrYy() const;
  bool isTrueOrYes() const;
  bool isFalseOrNn() const;
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

BOOLEAN
IsPropertyNotNullAndTrue(
  const TagStruct* Prop
  );

BOOLEAN
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
