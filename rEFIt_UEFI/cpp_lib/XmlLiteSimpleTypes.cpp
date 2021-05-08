/*
 *
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 */

#include "XmlLiteSimpleTypes.h"
#include "XmlLiteParser.h"
#include "../Platform/plist/xml.h"
#include "../Platform/Utils.h"

NullXmlType nullXmlType;

#define RETURN_IF_FALSE(Expression) do { bool b = Expression; if ( !b ) return false; } while (0);

const XmlStrictBool::ValueType XmlStrictBool::nullValue = XmlStrictBool::ValueType();

bool XmlStrictBool::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
{
  WARNING_IF_DEFINED;

  const char* tag;
  size_t tagLength;
  const char* value;
  size_t valueLength;

#ifdef JIEF_DEBUG
if ( xmlPath == "/Boot/XMPDetection"_XS8 ) {
  int i=0; (void)i;
}
#endif
  XmlParserPosition pos = xmlLiteParser->getPosition();
  if ( !xmlLiteParser->nextTagIsOpeningTag("string") && !xmlLiteParser->nextTagIsOpeningTag("true") && !xmlLiteParser->nextTagIsOpeningTag("false") ) {
    if ( xmlLiteParser->skipNextTag(generateErrors) ) {
      xmlLiteParser->addError(generateErrors, S8Printf("Expecting boolean value at line %d.", pos.getLine()));
    }else{
      xmlLiteParser->addXmlError(generateErrors, S8Printf("Expecting boolean value at line %d.", pos.getLine()));
    }
    return false;
  }
  RETURN_IF_FALSE( xmlLiteParser->getSimpleTag(&tag, &tagLength, &value, &valueLength, NULL, generateErrors) );
  if ( strnIsEqualIC(tag, tagLength, "true") ) {
    if ( valueLength == 0 ) return setBoolValue(true);
    xmlLiteParser->addError(generateErrors, S8Printf("Tag 'true' cannot contains text at line %d.", pos.getLine()));
  }else
  if ( strnIsEqualIC(tag, tagLength, "false") ) {
    if ( valueLength == 0 ) return setBoolValue(false);
    xmlLiteParser->addError(generateErrors, S8Printf("Tag 'false' cannot contains text at line %d.", pos.getLine()));
  }
  xmlLiteParser->addError(generateErrors, S8Printf("Expecting strict boolean value <true/> <false/> for tag '%s:%d'", xmlPath.c_str(), pos.getLine()));
  return false;
}

bool XmlBool::isTheNextTag(XmlLiteParser* xmlLiteParser)
{
  XmlParserPosition pos = xmlLiteParser->getPosition();
  // Because we currently accept string tag as boolean, we have to parse to see if it can be a bool. We can't just check the next tag.
  // Use a temporary variable. TODO improve by separating parsing and storage (ie Do not call setBoolValue in parseFromXmlLite().
  XmlBool xmlBool;
  bool b = xmlBool.parseFromXmlLite(xmlLiteParser, NullXString8, false);
  xmlLiteParser->restorePosition(pos);
  return b;
}

bool XmlBool::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
{
  WARNING_IF_DEFINED;

  const char* tag;
  size_t tagLength;
  const char* value;
  size_t valueLength;

#ifdef JIEF_DEBUG
if ( xmlPath.containsIC("/Debug"_XS8) ) {
  int i=0; (void)i;
}
#endif
  XmlParserPosition pos = xmlLiteParser->getPosition();
  if ( !xmlLiteParser->nextTagIsOpeningTag("string") && !xmlLiteParser->nextTagIsOpeningTag("true") && !xmlLiteParser->nextTagIsOpeningTag("false") ) {
    if ( xmlLiteParser->skipNextTag(generateErrors) ) {
      xmlLiteParser->addError(generateErrors, S8Printf("Expecting boolean value at line %d.", pos.getLine()));
    }else{
      xmlLiteParser->addXmlError(generateErrors, S8Printf("Expecting boolean value at line %d.", pos.getLine()));
    }
    return false;
  }
  RETURN_IF_FALSE( xmlLiteParser->getSimpleTag(&tag, &tagLength, &value, &valueLength, NULL, generateErrors) );
  if ( strnIsEqualIC(tag, tagLength, "true") ) {
//    if ( valueLength == 0 ) return setBoolValue(true, xmlLiteParser, xmlPath, pos, generateErrors);
    if ( valueLength == 0 ) return setBoolValue(true);
    xmlLiteParser->addError(generateErrors, S8Printf("Tag 'true' cannot contains text at line %d.", pos.getLine()));
  }else
  if ( strnIsEqualIC(tag, tagLength, "false") ) {
    if ( valueLength == 0 ) return setBoolValue(false);
    xmlLiteParser->addError(generateErrors, S8Printf("Tag 'false' cannot contains text at line %d.", pos.getLine()));
  }else
  if ( strnIsEqual(tag, tagLength, "string") ) {
    if ( strnIsEqualIC(value, valueLength, "true") ) {
      xmlLiteParser->addWarning(generateErrors, S8Printf("Boolean value contained in a string. Please use <true/> instead of <string>true</string> for tag '%s:%d'", xmlPath.c_str(), pos.getLine()));
      return setBoolValue(true);
    }else
    if ( strnIsEqualIC(value, valueLength, "yes") ) {
      xmlLiteParser->addWarning(generateErrors, S8Printf("Boolean value contained in a string. Please use <true/> instead of <string>yes</string> for tag '%s:%d'", xmlPath.c_str(), pos.getLine()));
      return setBoolValue(true);
    }else
    if ( strnIsEqualIC(value, valueLength, "false") ) {
      xmlLiteParser->addWarning(generateErrors, S8Printf("Boolean value contained in a string. Please use <false/> instead of <string>false</string> for tag '%s:%d'", xmlPath.c_str(), pos.getLine()));
      return setBoolValue(false);
    }else
    if ( strnIsEqualIC(value, valueLength, "no") ) {
      xmlLiteParser->addWarning(generateErrors, S8Printf("Boolean value contained in a string. Please use <false/> instead of <string>no</string> for tag '%s:%d'", xmlPath.c_str(), pos.getLine()));
      return setBoolValue(false);
    }
  }
  xmlLiteParser->addError(generateErrors, S8Printf("Expecting <true/> <false/> or <string> tag containing true, false, yes or no tag '%s:%d'", xmlPath.c_str(), pos.getLine()));
  return false;
}


bool XmlBoolYesNo::isTheNextTag(XmlLiteParser* xmlLiteParser)
{
  XmlParserPosition pos = xmlLiteParser->getPosition();
  // Because we currently accept string tag as boolean, we have to parse to see if it can be a bool. We can't just check the next tag.
  // Use a temporary variable. TODO improve by separating parsing and storage (ie Do not call setBoolValue in parseFromXmlLite().
  XmlBoolYesNo xmlBool;
  bool b = xmlBool.parseFromXmlLite(xmlLiteParser, NullXString8, false);
  xmlLiteParser->restorePosition(pos);
  return b;
}

bool XmlBoolYesNo::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
{
  WARNING_IF_DEFINED;

  const char* tag;
  size_t tagLength;
  const char* value;
  size_t valueLength;

#ifdef JIEF_DEBUG
if ( xmlPath.containsIC("/Debug"_XS8) ) {
  int i=0; (void)i;
}
#endif
  XmlParserPosition pos = xmlLiteParser->getPosition();
  if ( !xmlLiteParser->nextTagIsOpeningTag("string") && !xmlLiteParser->nextTagIsOpeningTag("true") && !xmlLiteParser->nextTagIsOpeningTag("false") ) {
    if ( xmlLiteParser->skipNextTag(generateErrors) ) {
      xmlLiteParser->addError(generateErrors, S8Printf("Expecting boolean value at line %d.", pos.getLine()));
    }else{
      xmlLiteParser->addXmlError(generateErrors, S8Printf("Expecting boolean value at line %d.", pos.getLine()));
    }
    return false;
  }
  RETURN_IF_FALSE( xmlLiteParser->getSimpleTag(&tag, &tagLength, &value, &valueLength, NULL, generateErrors) );
  if ( strnIsEqualIC(tag, tagLength, "true") ) {
//    if ( valueLength == 0 ) return setBoolValue(true, xmlLiteParser, xmlPath, pos, generateErrors);
    if ( valueLength == 0 ) return setBoolValue(true);
    xmlLiteParser->addError(generateErrors, S8Printf("Tag 'true' cannot contains text at line %d.", pos.getLine()));
  }else
  if ( strnIsEqualIC(tag, tagLength, "false") ) {
    if ( valueLength == 0 ) return setBoolValue(false);
    xmlLiteParser->addError(generateErrors, S8Printf("Tag 'false' cannot contains text at line %d.", pos.getLine()));
  }else
  if ( strnIsEqual(tag, tagLength, "string") ) {
    if ( strnIsEqualIC(value, valueLength, "true") ) {
      return setBoolValue(true);
    }else
    if ( strnIsEqualIC(value, valueLength, "yes") ) {
      return setBoolValue(true);
    }else
    if ( strnIsEqualIC(value, valueLength, "false") ) {
      return setBoolValue(false);
    }else
    if ( strnIsEqualIC(value, valueLength, "no") ) {
      return setBoolValue(false);
    }
  }
  xmlLiteParser->addError(generateErrors, S8Printf("Expecting <true/> <false/> or <string> tag containing true, false, yes or no tag '%s:%d'", xmlPath.c_str(), pos.getLine()));
  return false;
}



bool XmlString8::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
{
  WARNING_IF_DEFINED;

  const char* tag;
  size_t tagLength;
  const char* value;
  size_t valueLength;

#ifdef JIEF_DEBUG
if ( xmlPath.contains("Theme") ) {
  NOP;
}
if (xmlPath.containsIC("KextsToPatch[28]/Comment"_XS8) ) {
  NOP;
}
#endif
  XmlParserPosition pos = xmlLiteParser->getPosition();
  RETURN_IF_FALSE( xmlLiteParser->getSimpleTag(&tag, &tagLength, &value, &valueLength, NULL, generateErrors) );
  if ( strnIsEqual(tag, tagLength, "string") ) {
#ifdef JIEF_DEBUG
if ( value && strcmp(value, "/SMBIOs") == 0 ) {
  NOP;
}
#endif
    char* out = (char*)malloc(valueLength+1);
#ifdef JIEF_DEBUG
//printf("parse1   : %llx, out=%s\n", uintptr_t(out), out);
if ( out && strcmp(out, "/SMBIOs") == 0 ) {
  NOP;
}
//printf("parseFromXmlLite %s. XmlDecode(%s, %zu, %s, %lu)\n", xmlPath.c_str(), value, valueLength, out, valueLength+1);
#endif
    XMLDecode(value, valueLength, out, valueLength+1);
#ifdef JIEF_DEBUG
//printf("parse2   : %llx, out=%s\n", uintptr_t(out), out);
if ( out && strcmp(out, "/SMBIOs") == 0 ) {
  NOP;
}
#endif
    return stealStringValue(out, valueLength+1);
  }
  xmlLiteParser->addXmlError(generateErrors, S8Printf("Expecting a <string> tag in '%s' at line %d.", xmlPath.c_str(), pos.getLine()));
  return false;
}
const XmlString8::ValueType XmlString8::nullValue = XmlString8::ValueType();

bool XmlStringW::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
{
  WARNING_IF_DEFINED;

#ifdef JIEF_DEBUG
  if ( xmlPath.containsIC("/ACPI/DisabledAML") ) {
    printf("%s", "");
  }
#endif

  const char* tag;
  size_t tagLength;
  const char* value;
  size_t valueLength;

  XmlParserPosition pos = xmlLiteParser->getPosition();
  RETURN_IF_FALSE( xmlLiteParser->getSimpleTag(&tag, &tagLength, &value, &valueLength, NULL, generateErrors) );
  if ( strnIsEqual(tag, tagLength, "string") ) {
    char* out = (char*)malloc(valueLength+1); // Should we use a static buffer to speed up and minimize allocation/re-allocation ?
    size_t outlen = XMLDecode(value, valueLength, out, valueLength+1);
    setStringValue(out, outlen);
    free(out);
    return true;
  }
  xmlLiteParser->addXmlError(generateErrors, S8Printf("Expecting a <string> tag in '%s' at line %d.", xmlPath.c_str(), pos.getLine()));
  return false;
}
const XmlStringW::ValueType XmlStringW::nullValue = XmlString8::ValueType();

const XmlKey::ValueType XmlKey::nullValue = XmlKey::ValueType();

// TODO only difference with XmlString8::parseFromXmlLite is paramter "key" instead of "string"
bool XmlKey::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors, const char** keyValuePtr, size_t* keyValueLengthPtr)
{
  WARNING_IF_DEFINED;

#ifdef JIEF_DEBUG
  if ( xmlPath.startWithOrEqualToIC("/Devices/Properties/Properties_key1") ) {
    printf("%s", "");
  }
#endif

  const char* tag;
  size_t tagLength;
//  const char* _value;
//  size_t _valueLength;
//  if ( keyValuePtr == NULL ) keyValuePtr = &_value;
//  if ( keyValueLengthPtr == NULL ) keyValueLengthPtr = &_valueLength;

  XmlParserPosition pos = xmlLiteParser->getPosition();
  RETURN_IF_FALSE( xmlLiteParser->getSimpleTag(&tag, &tagLength, keyValuePtr, keyValueLengthPtr, NULL, generateErrors) );
  if ( strnIsEqual(tag, tagLength, "key") ) {
#ifdef JIEF_DEBUG
if ( LString8(*keyValuePtr).startWithOrEqualTo("NewWay_80000000"_XS8) ) {
 printf("%s", "NewWay_80000000");
}
#endif
#ifdef DEBUG_TRACE
printf("XmlKey::parseFromXmlLite key=%.*s, line=%d, buffer=", (int)*keyValueLengthPtr, *keyValuePtr, pos.getLine());
for(size_t i=0 ; i<40 ; i++) printf("%c", pos.p[i] < 32 ? 0 : pos.p[i]);
printf("\n");
#endif
    return setStringValue(*keyValuePtr, *keyValueLengthPtr);
  }
  xmlLiteParser->addXmlError(generateErrors, S8Printf("Expecting a <key> tag in '%s' at line %d.", xmlPath.c_str(), pos.getLine()));
  return false;
}

bool XmlKey::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
{
  const char* _value;
  size_t _valueLength;
  return parseFromXmlLite(xmlLiteParser, xmlPath, generateErrors, &_value, &_valueLength);
}

#include "../Platform/b64cdecode.h"

const XmlData::ValueType XmlData::nullValue = XmlData::ValueType();

bool XmlData::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
{
  WARNING_IF_DEFINED;

  const char* tag;
  size_t tagLength;
  const char* value;
  size_t valueLength;
  
#ifdef JIEF_DEBUG
  if ( xmlPath.startWithOrEqualToIC("/KernelAndKextPatches/KernelToPatch[0]/Find") ) {
    int i=0; (void)i;
  }
#endif

  XmlParserPosition pos = xmlLiteParser->getPosition();
  RETURN_IF_FALSE( xmlLiteParser->getSimpleTag(&tag, &tagLength, &value, &valueLength, NULL, generateErrors) );
  if ( strnIsEqual(tag, tagLength, "string") ) {
    size_t allocatedSize = valueLength/2; // number of hex digits
    uint8_t *Data = (uint8_t*)malloc(allocatedSize);
    size_t hexLen = hex2bin(value, valueLength, Data, allocatedSize);
    return stealDataValue(Data, hexLen, allocatedSize);
  }
  if ( strnIsEqual(tag, tagLength, "data") ) {
    UINTN decodedSize;
    UINT8* decoded = Base64DecodeClover(value, valueLength, &decodedSize);
    return stealDataValue(decoded, decodedSize);
  }
  xmlLiteParser->addError(generateErrors, S8Printf("Expecting a <data> tag following key '%s' at line %d.", xmlPath.c_str(), pos.getLine()));
  return false;
}


bool XmlIntegerAbstract::isTheNextTag(XmlLiteParser* xmlLiteParser)
{
  if ( xmlLiteParser->nextTagIsOpeningTag("integer") ) return true;

  XmlParserPosition pos = xmlLiteParser->getPosition();
  // Because we currently accept string tag containing an integer, we have to parse tosee if it can be an integer.
  // Use a temporary variable. TODO improve by separating parsing and storage (ie Do not call setBoolValue in parseFromXmlLite().

  UINTN result;
  bool negative;
  bool b = parseXmlInteger(xmlLiteParser, NullXString8, &result, &negative, INT64_MIN, UINT64_MAX, false);
  xmlLiteParser->restorePosition(pos);
  return b;
}

/*
 * Minimum is an absolute value. For a minium of -3, pass 3.
 * Minimum can be a positive integer. It's possible to pass minimum=10 and maximum=15.
 * Maximum can't be a negative integer.
 */
bool XmlIntegerAbstract::parseXmlInteger(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, UINTN* resultPtr, bool* negativePtr, INTN minimum, UINTN maximum, bool generateErrors)
{
  WARNING_IF_DEFINED;

  const char* tag;
  size_t tagLength;
  const char* value;
  size_t valueLength;

  XmlParserPosition pos = xmlLiteParser->getPosition();
  RETURN_IF_FALSE( xmlLiteParser->getSimpleTag(&tag, &tagLength, &value, &valueLength, NULL, generateErrors) );
  
  UINTN& result = *resultPtr;
  bool& negative = *negativePtr;
  bool atLeastOneDigit = false;

#ifdef JIEF_DEBUG
if ( xmlPath.contains("CsrActiveConfig") ) {
  int i=0; (void)i;
}
#endif

  if ( strnIsEqual(tag, tagLength, "integer") || strnIsEqual(tag, tagLength, "string") )
  {
    result = 0;
    negative = false;
    
    if( valueLength > 1  &&  (value[1] == 'x' || value[1] == 'X') ) {  // Hex value
      size_t idx = 2;
      int digit;
      while ( idx < valueLength ) {
        if ( value[idx] >= '0' && value[idx] <= '9' ) { // 0 - 9
          digit = value[idx] - '0';
          atLeastOneDigit = true;
        }
        else if ( value[idx] >= 'a' && value[idx] <= 'f' ) { // a - f
          digit = value[idx] - 'a' + 10;
          atLeastOneDigit = true;
        }
        else if ( value[idx] >= 'A' && value[idx] <= 'F' ) {  // A - F
          digit =value[idx] - 'A' + 10;
          atLeastOneDigit = true;
        }
        else {
          xmlLiteParser->addError(generateErrors, S8Printf("Expecting hex digits for tag '%s:%d'.", xmlPath.c_str(), pos.getLine()));
          return false;
        }
        if ( result > UINT64_MAX/16) {
          xmlLiteParser->addError(generateErrors, S8Printf("Expecting an integer between %lld and %llu for tag '%s:%d'.", minimum, maximum, xmlPath.c_str(), pos.getLine()));
          return false;
        }
        result *= 16;
        if ( result > UINT64_MAX - (unsigned int)digit) { // safe cast, digit is >= 0
          xmlLiteParser->addError(generateErrors, S8Printf("Expecting an integer between %lld and %llu for tag '%s:%d'.", minimum, maximum, xmlPath.c_str(), pos.getLine()));
          return false;
        }
        result += (unsigned int)digit; // safe cast, digit is >= 0
        idx++;
      }
      if ( !atLeastOneDigit ) {
        // Currently, Clover accepts 0x. // TODO: remove this in a near future ?
        return true;
      }
    }
    else
    if ( valueLength >= 1  &&  ( (value[0] >= '0' && value[0] <= '9') || value[0] == '-' )  ) {  // Decimal value
      size_t idx = 0;
      if (value[idx] == '-') {
        negative = TRUE;
        idx++;
      }
      for ( ; idx < valueLength; idx++)
      {
        int digit;
        if (value[idx] < '0' || value[idx] > '9') {
          xmlLiteParser->addError(generateErrors, S8Printf("Expecting decimal digits for tag '%s:%d'.", xmlPath.c_str(), pos.getLine()));
          return false;
        }
        atLeastOneDigit = true;
        digit = value[idx] - '0';
        if ( result > UINT64_MAX/10 ) {
          xmlLiteParser->addError(generateErrors, S8Printf("Expecting an integer between %lld and %llu for tag '%s:%d'.", minimum, maximum, xmlPath.c_str(), pos.getLine()));
          return false;
        }
        result *= 10;
        if ( result > UINT64_MAX - (unsigned int)digit ) { // safe cast, digit is >= 0
          xmlLiteParser->addError(generateErrors, S8Printf("Expecting an integer between %lld and %llu for tag '%s:%d'.", minimum, maximum, xmlPath.c_str(), pos.getLine()));
          return false;
        }
        result += (unsigned int)digit; // safe cast, digit is >= 0
      }
    }else{
      xmlLiteParser->addError(generateErrors, S8Printf("Expecting an integer starting with a decimal digit, a minus sign or '0x' for tag '%s:%d'.", xmlPath.c_str(), pos.getLine()));
      return false;
    }
    if ( !atLeastOneDigit ) {
      xmlLiteParser->addError(generateErrors, S8Printf("Expecting at least one digit for integer for tag '%s:%d'.", xmlPath.c_str(), pos.getLine()));
      return false;
    }
    if ( !negative ) {
      if ( (minimum > 0  &&  result < (UINT64)minimum)  ||  result > maximum ) { // safe cast, here, minimum is >= 0
        xmlLiteParser->addError(generateErrors, S8Printf("Expecting an integer between %lld and %llu for tag '%s:%d'.", minimum, maximum, xmlPath.c_str(), pos.getLine()));
        return false;
      }
    }else{
      // Number is negative and maximum is >= 0. So no need to test maximum.
      if ( result > UINT64(INT64_MAX)+1  ||  -((INT64)result) < minimum ) {
        xmlLiteParser->addError(generateErrors, S8Printf("Expecting an integer between %lld and %llu for tag '%s:%d'.", minimum, maximum, xmlPath.c_str(), pos.getLine()));
        return false;
      }
    }
    if ( negative && result == 0 ) {
      xmlLiteParser->addWarning(generateErrors, S8Printf("Integer is -0. It's mathematically correct but looks like a mistake for tag '%s:%d'. Corrected to 0.", xmlPath.c_str(), pos.getLine()));
      negative = false;
    }
  }else
  if ( strnIsEqual(tag, tagLength, "data") ) {
    xmlLiteParser->addWarning(generateErrors, S8Printf("Tag '%s:%d' should be an integer. It's currently a data. For now, I've made the conversion. Please update.", xmlPath.c_str(), pos.getLine()));
    UINTN decodedSize;
    UINT8* decoded = Base64DecodeClover(value, valueLength, &decodedSize);
    if ( decoded ) {
      if (decodedSize > sizeof(result) ) decodedSize = sizeof(result);
      result = 0;
      memcpy(&result, decoded, decodedSize); // decodedSize can be < 8 bytes, but that works because of litlle endian.
      free(decoded);
    }else{
      xmlLiteParser->addWarning(generateErrors, S8Printf("Tag '%s:%d' should be an integer. It's currently a data with conversion problem. Setting value to 0.", xmlPath.c_str(), pos.getLine()));
      result = 0;
    }
  }else{
    xmlLiteParser->addError(generateErrors, S8Printf("Expecting a <integer> tag following key '%s' for tag '%s:%d'.", xmlPath.c_str(), xmlPath.c_str(), pos.getLine()));
    return false;
  }
  if ( strnIsEqual(tag, tagLength, "string") ) {
//    xmlLiteParser->addWarning(generateErrors, S8Printf("Tag '%s:%d' should be an integer. It's currently a string. For now, I've made the conversion. Please update.", xmlPath.c_str(), pos.getLine()));
  }
  return true;
}


bool XmlUInt8::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
{
  UINTN value;
  bool sign;
  RETURN_IF_FALSE( parseXmlInteger(xmlLiteParser, xmlPath, &value, &sign, 0, UINT8_MAX, generateErrors) );
  return setUInt8Value((uint8_t)value, sign); // safe cast because parseXmlInteger minimum/maximum parameter
}

bool XmlUInt16::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
{
  UINTN value;
  bool sign;
  RETURN_IF_FALSE( parseXmlInteger(xmlLiteParser, xmlPath, &value, &sign, 0, UINT16_MAX, generateErrors) );
  return setUInt16Value((uint16_t)value, sign); // safe cast because parseXmlInteger minimum/maximum parameter
}

bool XmlUInt32::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
{
  UINTN value;
  bool sign;
  RETURN_IF_FALSE( parseXmlInteger(xmlLiteParser, xmlPath, &value, &sign, 0, UINT32_MAX, generateErrors) );
  return setUInt32Value((uint32_t)value, sign); // safe cast because parseXmlInteger minimum/maximum parameter
}

bool XmlUInt64::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
{
  UINTN value;
  bool sign;
  RETURN_IF_FALSE( parseXmlInteger(xmlLiteParser, xmlPath, &value, &sign, 0, UINT64_MAX, generateErrors) );
  return setUInt64Value((uint64_t)value, sign); // safe cast because parseXmlInteger minimum/maximum parameter
}

bool XmlInt8::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
{
  UINTN value;
  bool sign;
  RETURN_IF_FALSE( parseXmlInteger(xmlLiteParser, xmlPath, &value, &sign, INT8_MIN, INT8_MAX, generateErrors) );
  return setInt8Value((int8_t)value, sign); // safe cast because parseXmlInteger minimum/maximum parameter
}

bool XmlInt16::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
{
  UINTN value;
  bool sign;
  RETURN_IF_FALSE( parseXmlInteger(xmlLiteParser, xmlPath, &value, &sign, INT16_MIN, INT16_MAX, generateErrors) );
  return setInt16Value((int16_t)value, sign); // safe cast because parseXmlInteger minimum/maximum parameter
}

bool XmlInt32::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
{
  UINTN value;
  bool sign;
  RETURN_IF_FALSE( parseXmlInteger(xmlLiteParser, xmlPath, &value, &sign, INT32_MIN, INT32_MAX, generateErrors) );
  return setInt32Value((int32_t)value, sign); // safe cast because parseXmlInteger minimum/maximum parameter
}

bool XmlInt64::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
{
  UINTN value;
  bool sign;
  RETURN_IF_FALSE( parseXmlInteger(xmlLiteParser, xmlPath, &value, &sign, INT64_MIN, INT64_MAX, generateErrors) );
  return setInt64Value((int8_t)value, sign); // safe cast because parseXmlInteger minimum/maximum parameter
}
