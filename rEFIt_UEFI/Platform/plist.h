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
  TagStruct *_tag;
  XObjArray<TagStruct> _dictOrArrayContent;

public:

  TagStruct() : type(kTagTypeNone), _string(), _intValue(0), _floatValue(0), _data(0), _dataLen(0), /*offset(0), */_tag(NULL), _dictOrArrayContent() {}
  TagStruct(const TagStruct& other) = delete; // Can be defined if needed
  const TagStruct& operator = ( const TagStruct & ) = delete; // Can be defined if needed
  ~TagStruct() { delete _data; delete _tag; }

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

  const XObjArray<TagStruct>& dictOrArrayContent() const
  {
    if ( isDict() ) return _dictOrArrayContent;
    if ( isArray() ) return _dictOrArrayContent;
    panic("TagStruct::dictOrArrayTagValue() : !isDict() && isArray() ");
  }
  XObjArray<TagStruct>& dictOrArrayContent()
  {
    if ( isDict() ) return _dictOrArrayContent;
    if ( isArray() ) return _dictOrArrayContent;
    panic("TagStruct::dictOrArrayTagValue() : !isDict() && isArray() ");
  }
//  void setNextTagValue(TagStruct* nextTag)
//  {
//    if ( nextTag == NULL ) panic("TagStruct::setDictNextTagValue() : nextTag == NULL ");
//    if ( _nextTag != NULL ) panic("TagStruct::setDictNextTagValue() : _nextTag != NULL ");
//    _nextTag = nextTag;
//  }

  const XString8 getTypeAsXString8() const {
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
  const XString8& dataStringValue()
  {
    if ( !isData() ) panic("TagStruct::dataStringValue() : !isData() ");
    return _string;
  }
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
    if ( _dictOrArrayContent.notEmpty() ) panic("TagStruct::setDictTagValue() : __dictOrArrayContent.notEmpty() ");
    _tag = tagList;
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
    if ( _dictOrArrayContent.notEmpty() ) panic("TagStruct::setArrayTagValue() : __dictOrArrayContent.notEmpty() ");
    _tag = tag;
    type = kTagTypeArray;
  }

  
  const XString8& keyValue() const
  {
    if ( !isKey() ) panic("TagStruct::keyValue() : !isKey() ");
    return _string;
  }
  XString8& keyValue()
  {
    if ( !isKey() ) panic("TagStruct::keyValue() : !isKey() ");
    return _string;
  }
  const TagStruct* keyTagValue() const
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
  TagStruct* dictOrArrayTagValue()
  {
    if ( isDict() ) return dictTagValue();
    if ( isArray() ) return arrayTagValue();
    panic("TagStruct::dictOrArrayTagValue() : !isDict() && isArray() ");
  }
};

//typedef union {
//  struct {
//    float  fNum; //4 bytes
//    UINT32 pad;  // else 4
//  } B;
//  CHAR8  *string;
//} FlMix;


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


//VOID RenderSVGfont(NSVGfont  *fontSVG);

const TagStruct*
GetProperty(
  const TagStruct* dict,
  CONST CHAR8* key
  );

TagStruct* NewTag( void );

VOID
FreeTag (
  TagStruct* tag
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
  const TagStruct* dict
  );

EFI_STATUS
GetElement(
  const TagStruct* dict,
  INTN   id,
  const TagStruct** dict1
);

BOOLEAN
IsPropertyTrue(
  const TagStruct* Prop
  );

BOOLEAN
IsPropertyFalse(
  const TagStruct* Prop
  );

INTN
GetPropertyInteger(
  const TagStruct* Prop,
  INTN Default
  );

float GetPropertyFloat (const TagStruct* Prop, float Default);


#endif /* PLATFORM_PLIST_H_ */
