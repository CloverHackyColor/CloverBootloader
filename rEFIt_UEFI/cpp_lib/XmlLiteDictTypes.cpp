/*
 *
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 */

#include "XmlLiteDictTypes.h"

void XmlDict::reset()
{
  super::reset();
  XmlDictField* fields;
  size_t nb;
  getFields(&fields, &nb);
  if ( nb == 0 ) {
    // XmlDict object with custom parseFromXmlLite method may not have defined getFields().
    // Therefore, it'll return 0. So : no panic.
//    panic("Dict has no field defined");

  }
  for ( size_t idx = 0 ; idx < nb ; idx++ ) {
    XmlDictField& xmlDictField = fields[idx];
    XmlAbstractType& xmlAbstractType = xmlDictField.xmlAbstractType;
    xmlAbstractType.reset();
  }
}

/*
 * Try to parse the tag following a key in a dict
 */
XmlAbstractType& XmlDict::parseValueFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors, const XmlParserPosition &keyPos, const char *keyValue, size_t keyValueLength, bool* keyFound)
{
  XmlDictField* fields;
  size_t nb;
  *keyFound = false;
  getFields(&fields, &nb);
  if ( nb == 0 ) {
    panic("Dict '%s' has no field defined", xmlPath.c_str());
  }
  for ( size_t idx = 0 ; !*keyFound && idx < nb ; idx++ )
  {
    XmlDictField& xmlDictField = fields[idx];
    const char* fieldName = xmlDictField.m_name;
    XmlAbstractType& xmlAbstractType = xmlDictField.xmlAbstractType;
    if ( strnIsEqualIC(keyValue, keyValueLength, fieldName) )
    {
#ifdef DEBUG
if ( xmlPath.containsIC("/ACPI/RenameDevices") ) {
  (void)1;
}
if ( strcmp(fieldName, "AutoMerge") == 0 ) {
  (void)1;
}
#endif
      *keyFound = true;
//#ifdef DEBUG
//  XmlParserPosition pos = xmlLiteParser->getPosition();
//#endif

//      if ( xmlAbstractType.isDefined() ) {
//        xmlLiteParser->addWarning(generateErrors, S8Printf("Tag '%s:%d' is previously defined. Previous value ignored.", xmlPath.c_str(), keyPos.getLine()));
//        xmlAbstractType.reset();
//      }
//      if ( !xmlAbstractType.parseFromXmlLite(xmlLiteParser, xmlPath, generateErrors) ) {
////#ifdef DEBUG
////        xmlAbstractType.reset();
////        xmlLiteParser->restorePosition(pos);
////        xmlAbstractType.parseFromXmlLite(xmlLiteParser, xmlPath, false);
////#endif
//      }
      if ( xmlAbstractType.isDefined() ) {
        xmlLiteParser->addWarning(generateErrors, S8Printf("Tag '%s:%d' is previously defined. New value ignored.", xmlPath.c_str(), keyPos.getLine()));
        xmlLiteParser->skipNextTag(false);
      }else{
        if ( !xmlAbstractType.parseFromXmlLite(xmlLiteParser, xmlPath, generateErrors) ) {
        }
      }
      return xmlAbstractType;
    }
  }
  if ( !keyFound ) {
    // This key doesn't exist in the dict. Try to skip the value.
    if ( !xmlLiteParser->nextTagIsOpeningTag("key") ) {
      xmlLiteParser->skipNextTag(false);
    }
  }
  return nullXmlType;
}

XmlAbstractType& XmlDict::parseKeyAndValueFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors, const char** keyValuePtr,
size_t* keyValueLengthPtr)
{
  const char*& keyValue = *keyValuePtr;
  size_t& keyValueLength = *keyValueLengthPtr;

  XmlParserPosition keyPos;
  if ( !xmlLiteParser->getKeyTagValue(&keyValue, &keyValueLength, &keyPos, true) ) {
     // This is an error. It may not be a parsing error.
     // ie : if key tag is not "key"
    xmlLiteParser->skipNextTag(false);
    return nullXmlType;
  }
  if ( keyValueLength == 0 ) {
    xmlLiteParser->addWarning(generateErrors, S8Printf("Key in '%s:%d' is empty. Skipped.", xmlPath.c_str(), keyPos.getLine()));
    xmlLiteParser->skipNextTag(generateErrors); // return value doesn't need to be tested, because skipNextTag() set xmlLiteParser::xmlParsingError to true.
    return nullXmlType;
  }
  XString8 xmlSubPath = xmlPath;
  if ( xmlPath.lastChar() == '/' ) xmlSubPath.S8Catf("%.*s", (int)keyValueLength, keyValue);
  else xmlSubPath.S8Catf("/%.*s", (int)keyValueLength, keyValue);

//#ifdef DEBUG
//XmlParserPosition valuePos = xmlLiteParser->getPosition();
//#endif

  bool keyFound;
  XmlAbstractType& xmlAbstractType = parseValueFromXmlLite(xmlLiteParser, xmlSubPath, generateErrors, keyPos, keyValue, keyValueLength, &keyFound);

  if ( !keyFound ) {
    if ( keyValueLength == 0  ||  keyValue[0] != '#' ) {
      XString8 xmlSubPath2 = xmlPath.lastChar() == '/' ? S8Printf("Unknown key '%s%.*s:%d'. Skipped.", xmlPath.c_str(), (int)keyValueLength, keyValue, keyPos.getLine()) : S8Printf("Unknown key '%s/%.*s:%d'. Skipped.", xmlPath.c_str(), (int)keyValueLength, keyValue, keyPos.getLine());
      xmlLiteParser->addWarning(generateErrors, xmlSubPath2);
    }
    xmlLiteParser->skipNextTag(generateErrors); // return value doesn't need to be tested, because skipNextTag() set xmlLiteParser::xmlParsingError to true.
  }

//#ifdef DEBUG
//  if ( xmlLiteParser->xmlParsingError ) {
//    xmlLiteParser->restorePosition(valuePos);
//    reset();
//    XmlAbstractType& xmlAbstractType2 = parseValueFromXmlLite(xmlLiteParser, xmlSubPath, false, keyPos, keyValue, keyValueLength, &keyFound);
//    (void)xmlAbstractType2;
//  }
//#endif

  return xmlAbstractType;
}

bool XmlDict::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
{
  WARNING_IF_DEFINED;

  RETURN_IF_FALSE ( xmlLiteParser->consumeOpeningTag("dict", generateErrors) );
  setDefined();

//  const char* keyValue;
//  size_t keyValueLength;
//  XString8 xmlSubPath;

  while ( !xmlLiteParser->isEof() && !xmlLiteParser->nextTagIsClosingTag("dict") )
  {
//    RETURN_IF_FALSE( xmlLiteParser->getKeyTagValue(&keyValue, &keyValueLength, &keyPos, true) );
//    xmlSubPath = xmlPath;
//    xmlSubPath.S8Catf("/%.*s", (int)keyValueLength, keyValue);
//
#ifdef DEBUG
XmlParserPosition valuePos = xmlLiteParser->getPosition();
(void)valuePos;
#endif
//    bool keyFound;
//    XmlAbstractType& xmlAbstractType = parseValueFromXmlLite(xmlLiteParser, xmlSubPath, generateErrors, keyPos, keyValue, keyValueLength, &keyFound);

    XmlParserPosition keyPos = xmlLiteParser->getPosition();

    const char* keyValue;
    size_t keyValueLength;
    XmlParserPosition beforePos = xmlLiteParser->getPosition();
    
    XmlAbstractType& xmlAbstractType = parseKeyAndValueFromXmlLite(xmlLiteParser, xmlPath, generateErrors, &keyValue, &keyValueLength);

//    if ( xmlLiteParser->xmlParsingError ) {
//      //return false;
//    }
//    else
//    {
      if ( xmlAbstractType.isDefined() )
      {
        XString8 xmlSubPath = xmlPath.lastChar() == '/' ? S8Printf("%s%.*s", xmlPath.c_str(), (int)keyValueLength, keyValue) : S8Printf("%s/%.*s", xmlPath.c_str(), (int)keyValueLength, keyValue);
        bool validated = xmlAbstractType.validate(xmlLiteParser, xmlSubPath, keyPos, generateErrors);
        if ( !validated ) xmlAbstractType.reset();
      }else{
        if ( xmlLiteParser->getPosition() == beforePos ) {
          // position is still the same. So we'll endlessly loop if we do not move forward
          if ( *xmlLiteParser->getPosition().p == '<' ) xmlLiteParser->skipNextTag(false);
          else xmlLiteParser->moveForwardUntil('<');
          if ( xmlLiteParser->getPosition() == beforePos ) {
            // still not moved ???
            return false;
          }
        }
        // if !xmlAbstractType.isDefined(), it's because a unknown key, or a comment.
        // Just try to continue.
      }
//    }
  }
  RETURN_IF_FALSE ( xmlLiteParser->consumeClosingTag("dict", generateErrors) );
  return true;
}

bool XmlDict::validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& pos, bool generateErrors)
{
  (void)xmlLiteParser;
  (void)xmlPath;
  (void)pos;
  (void)generateErrors;
//  XmlDictField* fields;
//  size_t nb;
//  getFields(&fields, &nb);
//  for ( size_t idx = 0 ; idx < nb ; idx++ ) {
//    XmlDictField& xmlDictField = fields[idx];
//    XmlAbstractType& xmlAbstractType = xmlDictField.xmlAbstractType;
//    XString8 subXmlPath = S8Printf("%s/%s", xmlPath.c_str(), xmlDictField.m_name);
//#ifdef DEBUG
////if ( subXmlPath == "/ACPI/DSDT/Name"_XS8 ) {
//if ( subXmlPath == "/Boot/Policy"_XS8 ) {
//  printf("");
//}
//#endif
//    if ( !xmlAbstractType.validate(xmlLiteParser, subXmlPath, pos, generateErrors) ) {
//      #ifdef DEBUG
////        xmlAbstractType.validate(xmlLiteParser, subXmlPath, pos, false);
//      #endif
//      return false;
//    }
//  }
  return true;
}


