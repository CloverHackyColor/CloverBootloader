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
  UINTN  type; // type is private. Use is... functions.
  XString8 _string;
  INTN     _intValue;
  float    _floatValue;
  UINT8  *_data;
  UINTN  _dataLen;
  TagStruct *_tag;
  TagStruct *_nextTag;

public:

  TagStruct() : type(kTagTypeNone), _string(), _intValue(0), _floatValue(0), _data(0), _dataLen(0), /*offset(0), */_tag(NULL), _nextTag(NULL) {}
  TagStruct(const TagStruct& other) = delete; // Can be defined if needed
  const TagStruct& operator = ( const TagStruct & ) = delete; // Can be defined if needed
  ~TagStruct() {}

  void FreeTag();

//  Property<XString8> string();
  bool isDict() { return type == kTagTypeDict; }
  bool isKey() { return type == kTagTypeKey; }
  bool isString() { return type == kTagTypeString; }
  bool isInt() { return type == kTagTypeInteger; }
  bool isFloat() { return type == kTagTypeFloat; }
  bool isBool() { return type == kTagTypeTrue  ||  type == kTagTypeFalse; }
  bool isData() { return type == kTagTypeData; }
  bool isDate() { return type == kTagTypeDate; }
  bool isArray() { return type == kTagTypeArray; }

  TagStruct* nextTagValue()
  {
    return _nextTag;
  }
  void setNextTagValue(TagStruct* nextTag)
  {
    if ( nextTag == NULL ) panic("TagStruct::setDictNextTagValue() : nextTag == NULL ");
    if ( _nextTag != NULL ) panic("TagStruct::setDictNextTagValue() : _nextTag != NULL ");
    _nextTag = nextTag;
  }

  const XString8 getTypeAsXString8() {
    if ( isDict() ) return "Dict"_XS8;
    if ( isKey() ) return "Dict"_XS8;
    if ( isString() ) return "Dict"_XS8;
    if ( isInt() ) return "Dict"_XS8;
    if ( isFloat() ) return "Dict"_XS8;
    if ( isBool() ) return "Dict"_XS8;
    if ( isData() ) return "Dict"_XS8;
    if ( isDate() ) return "Dict"_XS8;
    if ( isArray() ) return "Dict"_XS8;
    panic("Unknown type %lld : this is bug", type);
  }

  // getter and setter
  UINT8* dataValue()
  {
    if ( !isData() ) panic("TagStruct::dataValue() : !isData() ");
    return _data;
  }
  const XString8& dataStringValue()
  {
    if ( !isData() ) panic("TagStruct::dataStringValue() : !isData() ");
    return _string;
  }
  UINTN dataLenValue()
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

  TagStruct* dictTagValue()
  {
    if ( !isDict() ) panic("TagStruct::dictValue() : !isDict() ");
    return _tag;
  }
  void setDictTagValue(TagStruct* tagList)
  {
    // empty dict is allowed
    //if ( tagList == NULL ) panic("TagStruct::setDictTagValue() : tagList == NULL ");
    if ( _tag != NULL ) panic("TagStruct::setDictTagValue() : _tag != NULL ");
    if ( _nextTag != NULL ) panic("TagStruct::setDictTagValue() : _nextTag != NULL ");
    _tag = tagList;
    _nextTag = NULL;
    type = kTagTypeDict;
  }

  TagStruct* arrayTagValue()
  {
    if ( !isArray() ) panic("TagStruct::arrayValue() : !isArray() ");
    return _tag;
  }
  void setArrayTagValue(TagStruct* tag)
  {
    // Array value with tagList = NULL is allowed
    //if ( tag == NULL ) panic("TagStruct::setArrayValue() : tag == NULL ");
    if ( _tag != NULL ) panic("TagStruct::setArrayValue() : _tag != NULL ");
    if ( _nextTag != NULL ) panic("TagStruct::setArrayTagValue() : _nextTag != NULL ");
    _tag = tag;
    type = kTagTypeArray;
  }

  XString8& keyValue()
  {
    if ( !isKey() ) panic("TagStruct::keyValue() : !isKey() ");
    return _string;
  }
  TagStruct* keyTagValue()
  {
    if ( !isKey() ) panic("TagStruct::keyTagValue() : !isKey() ");
    return _tag;
  }
  void setKeyValue(const XString8& xstring, TagStruct* subTag)
  {
    if ( xstring.isEmpty() ) panic("TagStruct::setKeyValue() : xstring.isEmpty() ");
    type = kTagTypeKey;
    _string = xstring;
    _tag = subTag;
  }

  const XString8& stringValue()
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
  
  INTN intValue()
  {
    if ( !isInt() ) panic("TagStruct::intValue() : !isInt() ");
    return _intValue;
  }
  void setIntValue(INTN i)
  {
    type = kTagTypeInteger;
    _intValue = i;
  }
  
  float floatValue()
  {
    if ( !isFloat() ) panic("TagStruct::floatValue() : !isFloat() ");
    return _floatValue;
  }
  void setFloatValue(float f)
  {
    type = kTagTypeFloat;
    _floatValue = f;
  }

  INTN boolValue()
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
  bool isTrue()
  {
    if ( isBool() ) return boolValue();
    return false;
  }
  bool isFalse()
  {
    if ( isBool() ) return !boolValue();
    return false;
  }
  bool isTrueOrYy()
  {
    if ( isBool() ) return boolValue();
    if ( isString() && stringValue().notEmpty() && (stringValue()[0] == 'y' || stringValue()[0] == 'Y') ) return true;
    return false;
  }
  bool isTrueOrYes()
  {
    if ( isBool() ) return boolValue();
    if ( isString() && stringValue().equal("Yes"_XS8) ) return true;
    return false;
  }
  bool isFalseOrNn()
  {
    if ( isBool() ) return !boolValue();
    if ( isString() && stringValue().notEmpty() && (stringValue()[0] == 'n' || stringValue()[0] == 'N') ) return true;
    return false;
  }
  TagStruct* dictOrArrayTagValue()
  {
    if ( isDict() ) return dictTagValue();
    if ( isArray() ) return arrayTagValue();
    panic("TagStruct::dictOrArrayTagValue() : !isDict() && isArray() ");
  }
};

typedef TagStruct* TagPtr;

typedef union {
  struct {
    float  fNum; //4 bytes
    UINT32 pad;  // else 4
  } B;
  CHAR8  *string;
} FlMix;


CHAR8*
XMLDecode (
  CHAR8 *src
  );

EFI_STATUS
ParseXML(
  CONST CHAR8  *buffer,
        TagPtr *dict,
        UINT32 bufSize
  );


//VOID RenderSVGfont(NSVGfont  *fontSVG);

TagPtr
GetProperty(
        TagPtr dict,
  CONST CHAR8* key
  );

EFI_STATUS
XMLParseNextTag (
  CHAR8  *buffer,
  TagPtr *tag,
  UINT32 *lenPtr
  );

VOID
FreeTag (
  TagPtr tag
  );

EFI_STATUS
GetNextTag (
  UINT8  *buffer,
  CHAR8  **tag,
  UINT32 *start,
  UINT32 *length
  );

INTN
GetTagCount (
  TagPtr dict
  );

EFI_STATUS
GetElement(
  TagPtr dict,
  INTN   id,
  TagPtr *dict1
);

BOOLEAN
IsPropertyTrue(
  TagPtr Prop
  );

BOOLEAN
IsPropertyFalse(
  TagPtr Prop
  );

INTN
GetPropertyInteger(
  TagPtr Prop,
  INTN Default
  );

float GetPropertyFloat (TagPtr Prop, float Default);


#endif /* PLATFORM_PLIST_H_ */
