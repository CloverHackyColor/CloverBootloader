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

class TagStruct;
extern XObjArray<TagStruct> gTagsFree;

class TagStruct
{
  UINTN  type; // type is private. Use is...() functions.
  XString8 _string;
  INTN     _intValue;
  float    _floatValue;
  UINT8  *_data;
  UINTN  _dataLen;
  XObjArray<TagStruct> _dictOrArrayContent;

public:

  TagStruct() : type(kTagTypeNone), _string(), _intValue(0), _floatValue(0), _data(0), _dataLen(0), _dictOrArrayContent() {}
  TagStruct(const TagStruct& other) = delete; // Can be defined if needed
  const TagStruct& operator = ( const TagStruct & ) = delete; // Can be defined if needed
  ~TagStruct() { delete _data; }

  static TagStruct* getEmptyTag();
  static TagStruct* getEmptyDictTag();
  static TagStruct* getEmptyArrayTag();
  void FreeTag();

//  Property<XString8> string();
  bool isDict() const { return type == kTagTypeDict; }
  bool isKey() const { return type == kTagTypeKey; }
  bool isString() const { return type == kTagTypeString; }
  bool isInt() const { return type == kTagTypeInteger; }
  bool isFloat() const { return type == kTagTypeFloat; }
  bool isBool() const { return type == kTagTypeTrue  ||  type == kTagTypeFalse; }
  bool isData() const { return type == kTagTypeData; }
  bool isDate() const { return type == kTagTypeDate; }
  bool isArray() const { return type == kTagTypeArray; }

  const XString8 getTypeAsXString8() const {
    if ( isDict() ) return "Dict"_XS8;
    if ( isKey() ) return "Key"_XS8;
    if ( isString() ) return "String"_XS8;
    if ( isInt() ) return "Int"_XS8;
    if ( isFloat() ) return "Float"_XS8;
    if ( isBool() ) return "Bool"_XS8;
    if ( isData() ) return "Data"_XS8;
    if ( isDate() ) return "Date"_XS8;
    if ( isArray() ) return "Array"_XS8;
    panic("Unknown type %lld : this is bug", type);
  }

  /*
   *  getters and setters
   */

  /* data */
  const UINT8* dataValue() const
  {
    if ( !isData() ) panic("TagStruct::dataValue() : !isData() ");
    return _data;
  }
  UINT8* dataValue()
  {
    if ( !isData() ) panic("TagStruct::dataValue() : !isData() ");
    return _data;
  }
//  const XString8& dataStringValue()
//  {
//    if ( !isData() ) panic("TagStruct::dataStringValue() : !isData() ");
//    return _string;
//  }
  UINTN dataLenValue() const
  {
    if ( !isData() ) panic("TagStruct::dataLenValue() : !isData() ");
    return _dataLen;
  }
  void setDataValue(UINT8* data, UINTN dataLen)
  {
    if ( data == NULL ) panic("TagStruct::setDataValue() : _data == NULL ");
    _data = data;
    _dataLen = dataLen;
    type = kTagTypeData;
  }

  /* date */
  const XString8& dateValue()
  {
    if ( !isDict() ) panic("TagStruct::dictValue() : !isDict() ");
    return _string;
  }
  void setDateValue(const XString8& xstring)
  {
    if ( xstring.isEmpty() ) panic("TagStruct::setDateValue() : xstring.isEmpty() ");
    type = kTagTypeDate;
    _string = xstring;
  }

  /* dict */
  const XObjArray<TagStruct>& dictContent() const
  {
    if ( !isDict() ) panic("TagStruct::dictContent() : !isDict() ");
    return _dictOrArrayContent;
  }
  XObjArray<TagStruct>& dictContent()
  {
    if ( !isDict() ) panic("TagStruct::dictContent() : !isDict() ");
    return _dictOrArrayContent;
  }
  INTN dictKeyCount() const;
  EFI_STATUS dictKeyAndValueAtIndex(INTN id, const TagStruct** key, const TagStruct** value) const;
  const TagStruct* dictPropertyForKey(const CHAR8* key ) const;

  /* array */
  const XObjArray<TagStruct>& arrayContent() const
  {
    if ( isArray() ) return _dictOrArrayContent;
    panic("TagStruct::arrayContent() const : !isArray() ");
  }
  XObjArray<TagStruct>& arrayContent()
  {
    if ( isArray() ) return _dictOrArrayContent;
    panic("TagStruct::arrayContent() : !isDict() && !isArray() ");
  }

  /* key */
  const XString8& keyStringValue() const
  {
    if ( !isKey() ) panic("TagStruct::keyStringValue() const : !isKey() ");
    return _string;
  }
  XString8& keyStringValue()
  {
    if ( !isKey() ) panic("TagStruct::keyStringValue() : !isKey() ");
    return _string;
  }
  void setKeyValue(const XString8& xstring)
  {
    if ( xstring.isEmpty() ) panic("TagStruct::setKeyValue() : xstring.isEmpty() ");
    type = kTagTypeKey;
    _string = xstring;
  }

  /* string */
  const XString8& stringValue() const
  {
    if ( !isString() ) panic("TagStruct::stringValue() : !isString() ");
    return _string;
  }
  XString8& stringValue()
  {
    if ( !isString() ) panic("TagStruct::stringValue() : !isString() ");
    return _string;
  }
  void setStringValue(const XString8& xstring)
  {
    // empty string is allowed
    //if ( xstring.isEmpty() ) panic("TagStruct::setStringValue() : xstring.isEmpty() ");
    type = kTagTypeString;
    _string = xstring;
  }
  
  /* int */
  INTN intValue() const
  {
    if ( !isInt() ) panic("TagStruct::intValue() : !isInt() ");
    return _intValue;
  }
  void setIntValue(INTN i)
  {
    type = kTagTypeInteger;
    _intValue = i;
  }
  
  /* float */
  float floatValue() const
  {
    if ( !isFloat() ) panic("TagStruct::floatValue() : !isFloat() ");
    return _floatValue;
  }
  void setFloatValue(float f)
  {
    type = kTagTypeFloat;
    _floatValue = f;
  }

  /* bool */
  INTN boolValue() const
  {
    if ( !isBool() ) panic("TagStruct::boolValue() : !isBool() ");
    return type == kTagTypeTrue;
  }
  void setBoolValue(bool b)
  {
    if ( b ) type = kTagTypeTrue;
    else type = kTagTypeFalse;
  }
  
  // Convenience method
  bool isTrue() const
  {
    if ( isBool() ) return boolValue();
    return false;
  }
  bool isFalse() const
  {
    if ( isBool() ) return !boolValue();
    return false;
  }
  bool isTrueOrYy() const
  {
    if ( isBool() ) return boolValue();
    if ( isString() && stringValue().notEmpty() && (stringValue()[0] == 'y' || stringValue()[0] == 'Y') ) return true;
    return false;
  }
  bool isTrueOrYes() const
  {
    if ( isBool() ) return boolValue();
    if ( isString() && stringValue().equal("Yes"_XS8) ) return true;
    return false;
  }
  bool isFalseOrNn() const
  {
    if ( isBool() ) return !boolValue();
    if ( isString() && stringValue().notEmpty() && (stringValue()[0] == 'n' || stringValue()[0] == 'N') ) return true;
    return false;
  }
};


CHAR8*
XMLDecode (
  CHAR8 *src
  );

EFI_STATUS
ParseXML(
  CONST CHAR8  *buffer,
        TagStruct* *dict,
        UINT32 bufSize
  );


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
