/*
 *
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 */

// Only use angled for Platform, else, xcode project won't compile
#include "XmlLiteSimpleTypes.h"
#include "XmlLiteParser.h"

#include "../cpp_foundation/XString.h"
#include "../cpp_foundation/XStringArray.h"
#include "../cpp_foundation/unicode_conversions.h"


#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif


bool strnnIsEqual(const char* key, size_t keyLength, const char* value, size_t valueLength)
{
  if ( keyLength != valueLength ) return false;
  return strncmp(key, value, keyLength) == 0;
}

bool strnIsEqual(const char* key, size_t keyLength, const char* value)
{
  return strnnIsEqual(key, keyLength, value, strlen(value));
}


bool strnnIsEqualIC(const char* key, size_t keyLength, const char* value, size_t valueLength)
{
  if ( keyLength != valueLength ) return false;
  return strncasecmp(key, value, keyLength) == 0;
}

bool strnIsEqualIC(const char* key, size_t keyLength, const char* value)
{
  return strnnIsEqualIC(key, keyLength, value, strlen(value));
}


XmlParserPosition XmlLiteParser::getPosition()
{
  return currentPos;
}

void XmlLiteParser::restorePosition(XmlParserPosition& xml_position)
{
  currentPos = xml_position;
}

void XmlLiteParser::init(const char* buf, size_t size)
{
(void)size;
  p_start = (char*)malloc(size+1);
  memcpy(p_start, buf, size); // TODO remove that copy. I shouldnt need an ending 0 anymore, I think... Check.
  p_start[size] = 0;
  p_end = p_start + size;

  currentPos.p = p_start;
  currentPos.line = 1;
  currentPos.col = 1;
  
  errorsAndWarnings.setEmpty();

  for ( size_t i = 0; i < size ; ++i) {
    if ( p_start[i] == 0 ) {
      addWarning(true, S8Printf("Invalid NULL char at offset %zu. Replace by a space", i));
      p_start[i] = 0x20;  //replace random zero bytes to spaces
    }
  }

}

char XmlLiteParser::getchar()
{
  return *currentPos.p;
}

char XmlLiteParser::moveForward()
{
//  if ( getchar() == 0 ) {
  if ( currentPos.p >= p_end ) {
    if ( currentPos.p > p_end ) {
      panic("BUG in xmlLiteParser. Went past the end.");
    }
    return 0;
  }
  if ( getchar() == '\n' ) {
    currentPos.line++;
    currentPos.col = 1;
  }else{
    currentPos.col++;
  }
  return *( ++(currentPos.p) );
}

char XmlLiteParser::moveForward(int n)
{
  for ( int i = 0 ; i < n-1 ; i++ ) moveForward();
  return moveForward();
}

char XmlLiteParser::moveBackward()
{
  if ( currentPos.p == p_start ) return 0;
  
  currentPos.p--;
  if ( getchar() == '\n' ) {
    if ( currentPos.col != 1 ) panic("%s : currentPos.col != 1", __PRETTY_FUNCTION__);
    currentPos.line--;
    currentPos.col = 1;
    char* q = currentPos.p;
    while ( q > p_start && *(--q) != '\n' ) { currentPos.col++; };
  }else{
    currentPos.col--;
  }
  return getchar();
}

char XmlLiteParser::moveForwardUntilSignificant()
{
  char c = getchar();
  while ( c == ' '  ||  c == '\t' ||   c == '\r' ||  c == '\n' ) {
    c = moveForward();
  }
  return c;
}

char XmlLiteParser::moveBackwardUntilSignificant()
{
  char c = getchar();
  while ( c == ' '  ||  c == '\t' ||   c == '\r' ||  c == '\n' ) {
    c = moveBackward();
  }
  return c;
}

char XmlLiteParser::moveForwardUntil(char until)
{
  char c = getchar();
  while ( currentPos.p < p_end && c != until ) c = moveForward();
  #ifdef JIEF_DEBUG
    if ( currentPos.p > p_end ) panic("%s : currentPos.p > p_end", __PRETTY_FUNCTION__);
  #endif
  if ( currentPos.p == p_end ) return 0;
  return c;
}

char XmlLiteParser::moveBackwardUntil(char until)
{
  char c = moveBackward();
  while ( c && c != until ) c = moveBackward();
  return c;
}

char XmlLiteParser::moveForwardPastNext(char until)
{
  moveForwardUntil(until);
  return moveForward();
}

void XmlLiteParser::skipHeader()
{
  if ( strncmp(currentPos.p, "<?xml", 5) == 0 ) {
    moveForwardPastNext('>');
    moveForwardUntilSignificant();
    if ( strncmp(currentPos.p, "<!DOCTYPE", 9) == 0 ) {
      moveForwardPastNext('>');
      moveForwardUntilSignificant();
    }
    if ( strncmp(currentPos.p, "<plist", 6) == 0 ) {
      moveForwardPastNext('>');
      moveForwardUntilSignificant();
    }
  }
}

#define IS_TAGCHAR(x) (   ( x >= 'a' && x <='z' )  ||  ( x >= 'A' && x <='Z' )  ||  ( x >= '0' && x <= '9' )   )

bool XmlLiteParser::getNextTag(const char** tag, size_t* length, bool* isOpeningTag, bool* isClosingTag, bool generateErrors)
{
  if (tag == NULL) panic("tag == NULL");

//  // Find the start of the tag.
//  moveUntilNext('<');

  moveForwardUntilSignificant();

//  if ( getchar() == '\0' ) {
  if ( currentPos.p >= p_end ) {
    addXmlError(generateErrors, S8Printf("Unexpected end of file at line %d col %d", currentPos.line, currentPos.col));
    return false;
  }
  if ( getchar() == '<' ) {
    moveForward();
    if ( getchar() == '/') {
      *isOpeningTag = false;
      *isClosingTag = true;
      moveForward();
    }else{
      *isOpeningTag = true;
      *isClosingTag = false;
    }
    *tag = currentPos.p;
    
    // Find the end of the tag.
    char c = moveForward();
    while ( IS_TAGCHAR(c) ) c = moveForward();
    
    if ( c == '/' ) {
      if ( *isClosingTag ) {
        // tag like </true/> are illegal
        addXmlError(generateErrors, S8Printf("unexpected '/' at line %d col %d. Tag like </true/> are illegal.", currentPos.line, currentPos.col));
        return false;
      }
      *length = size_t(currentPos.p - *tag);
      return true;
    }
    if ( c != '>' ) {
      if ( c == '\0' ) {
        addXmlError(generateErrors, S8Printf("Unexpected end of file at line %d col %d while looking for closing char '>'", currentPos.line, currentPos.col));
        return false;
      }
      addXmlError(generateErrors, S8Printf("Unexpected char '%c' at line %d col %d while looking for closing char '>'", c, currentPos.line, currentPos.col));
      return false;
    }
    *length = size_t(currentPos.p - *tag);
    moveForward();
//    moveForwardUntilSignificant();
    return true;
  }else
  if ( getchar() == '/' ) {
    // special case where we left at an empty tag <true/>
    XmlParserPosition pos = getPosition();
    moveBackwardUntil('<');
    moveForward();
    *tag = currentPos.p;
    restorePosition(pos);
    *length = size_t(currentPos.p - *tag);
    char c = moveForward(); // skip '/'
    if ( c != '>' ) {
      if ( c == '\0' ) {
        addXmlError(generateErrors, S8Printf("Unexpected end of file at line %d col %d while looking for closing char '>'", currentPos.line, currentPos.col));
        return false;
      }
      addXmlError(generateErrors, S8Printf("Unexpected char '%c' at line %d col %d while looking for closing char '>'", c, currentPos.line, currentPos.col));
      return false;
    }
    moveForward();
//    moveForwardUntilSignificant();
    *isOpeningTag = false;
    *isClosingTag = true;
    return true;
  }else{
    addXmlError(generateErrors, S8Printf("Unexpected char '%c' at line %d col %d while looking for a tag that must start with '<'", getchar() , currentPos.line, currentPos.col));
    moveForward();
    return false;
  }
}

/*
 * Get the string from current position until the next '<', or end of buffer.
 * Spaces at begining or end are skipped.
 * If no '<', false is returned.
 * Position at the '<' when exit.
 */
void XmlLiteParser::getString(const char** string, size_t* length)
{
  if ( *currentPos.p == '/' && *(currentPos.p+1) == '>' ) {
    // Special case. We were left at '/>' to represent an empty tag
    *string = NULL;
    *length = 0;
    return;
  }
  char c;
//  c = moveForwardUntilSignificant();
    c = getchar();
//  if ( c == 0 || c == '<' /*|| c == '/'*/ ) {
    if ( currentPos.p >= p_end || c == '<' /*|| c == '/'*/ ) {
    *string = NULL;
    *length = 0;
    return;
  }
  *string = currentPos.p;
  c = moveForwardUntil('<');
  if ( c ) {
    moveBackward();
  }
//  moveBackwardUntilSignificant();
  *length = size_t(currentPos.p - *string + 1);
  if ( *length == 0 ) *string = NULL;
  moveForwardUntil('<');
}

#define RETURN_IF_FALSE(Expression) do { bool b = Expression; if ( !b ) return false; } while (0);

bool XmlLiteParser::getSimpleTag(const char** tag, size_t* tagLength, const char** value, size_t* valueLength, const char* expectedTag/*, bool valueCanBeEmpty*/, bool generateErrors)
{
  bool isOpeningTag, isClosingTag;
  
  XmlParserPosition pos = getPosition();
  RETURN_IF_FALSE( getNextTag(tag, tagLength, &isOpeningTag, &isClosingTag, generateErrors) );
  if ( isClosingTag ) {
    // opening tag expected
    addXmlError(generateErrors, S8Printf("Unexpected closing tag '%.*s' at line %d col %d. Was expecting <key>.", (int)*tagLength, *tag, pos.line, pos.col));
    return false;
  }
  if ( expectedTag  &&  !strnIsEqual(*tag, *tagLength, expectedTag) ) {
    addXmlError(generateErrors, S8Printf("Unexpected tag '%.*s' at line %d col %d. Was expecting <%s>.", (int)*tagLength, *tag, pos.line, pos.col, expectedTag));
    return false;
  }
  
  getString(value, valueLength);
//  if ( !valueCanBeEmpty  &&  *valueLength == 0 ) {
//    if ( generateErrors ) addError(S8Printf("Text of tag '%.*s' cannot be empty at line %d col %d\n", (int)*tagLength, *tag, currentPos.line, currentPos.col));
//    return false;
//  }

  const char* endTag;
  size_t endTagLength;
  pos = getPosition();
  RETURN_IF_FALSE( getNextTag(&endTag, &endTagLength, &isOpeningTag, &isClosingTag, generateErrors) );
  if ( isOpeningTag ) {
    // closing tag expected
    currentPos = pos;
    addXmlError(generateErrors, S8Printf("Expected closing tag '%.*s' at line %d col %d", (int)*tagLength, *tag, pos.line, pos.col));
    return false;
  }
  if ( !strnnIsEqual(endTag, endTagLength, *tag, *tagLength) ) {
    // closing tag name is different
    currentPos = pos;
    addXmlError(generateErrors, S8Printf("Expected closing tag '%.*s' at line %d col %d", (int)*tagLength, *tag, pos.line, pos.col));
    return false;
  }
//  if ( *valueLength == 0 ) {
//    addError(generateErrors, S8Printf("Key '%.*s' has an empty value at line %d col %d\n", (int)*tagLength, *tag, pos.line, pos.col));
//    return false;
//  }
  return true;
}

/*
 * The opening tag has been read. Skip until the closing tag
 */
bool XmlLiteParser::skipUntilClosingTag(const char* tagToSkip, size_t tagToSkipLength, bool generateErrors)
{
  const char* value;
  size_t valueLength;
  bool b;
  
  getString(&value, &valueLength);

//TODO
while ( valueLength > 0 && (*value == ' '  ||  *value == '\t' ||   *value == '\r' ||  *value == '\n') ) {
  ++value;
  --valueLength;
}

  const char* tag;
  size_t tagLength;
  bool isOpeningTag, isClosingTag;

  XmlParserPosition pos = getPosition();
  b = getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, generateErrors);
  if ( !b ) return false;
  if ( isOpeningTag && valueLength > 0 ) {
    // cannot have a tag containing text AND subtag
    addXmlError(generateErrors, S8Printf("Tag '%.*s' cannot contains text AND tag at line %d col %d", (int)tagToSkipLength, tagToSkip, pos.line, pos.col));
    return false;
  }
  while ( isOpeningTag )
  {
    b = skipUntilClosingTag(tag, tagLength, generateErrors);
    if ( !b ) return false;

    b = getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, generateErrors);
    if ( !b ) return false;
  }
  if ( !strnnIsEqual(tag, tagLength, tagToSkip, tagToSkipLength) ) {
    // closing tag is different
    addXmlError(generateErrors, S8Printf("Expected closing tag '%.*s' at line %d col %d", (int)tagToSkipLength, tagToSkip, pos.line, pos.col));
    return false;
  }
  char c = moveForwardUntilSignificant();
  if ( c != 0  &&  c != '<' ) {
    // After a closing tag, we can't have chars other than spaces
    addXmlError(generateErrors, S8Printf("Unexpected char '%c' at line %d col %d after the end of tag '%.*s'. The containing tag cannot have text abd tab", getchar() , currentPos.line, currentPos.col, (int)tagToSkipLength, tagToSkip));
    return false;
  }
  
  return true;
}

bool XmlLiteParser::skipNextTag(bool generateErrors)
{
  const char* tag;
  size_t tagLength;
  bool isOpeningTag, isClosingTag;
  bool b;

  XmlParserPosition pos = getPosition();
  b = getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, generateErrors);
  if ( !b ) return false;
  if ( isClosingTag ) {
    // next tag should be an opening one
    addXmlError(generateErrors, S8Printf("Expected an opening tag at line %d col %d", pos.line, pos.col));
    return false;
  }
  return skipUntilClosingTag(tag, tagLength, generateErrors);
}

bool XmlLiteParser::getSimpleTagValue(const char* expectedTag, size_t expectedTagLength, const char** value, size_t* valueLength, XmlParserPosition* xmlParserPosition, bool generateErrors)
{
  const char* tag;
  size_t tagLength;
  
  *xmlParserPosition = getPosition();
  bool b = getSimpleTag(&tag, &tagLength, value, valueLength, expectedTag, generateErrors);
  if ( !b ) return false;
  if ( !strnnIsEqualIC(tag, tagLength, expectedTag, expectedTagLength) ) {
    addXmlError(generateErrors, S8Printf("Expecting a <%s> at line %d col %d", expectedTag, (*xmlParserPosition).line, (*xmlParserPosition).col));
    return false;
  }
//  if ( *valueLength == 0 ) {
//    return false;
//    // todo Msg
//  }
//  if ( **value == '#' ) {
//    b = skipNextTag();
//    if ( !b ) return false;
//    return getSimpleTagValue(expectedTag, expectedTagLength, value, valueLength, xmlParserPosition, generateErrors);
//  }
  return true;
}


bool XmlLiteParser::getKeyTagValue(const char** value, size_t* valueLength, XmlParserPosition* xmlParserPosition, bool generateErrors)
{
  const char* tag;
  size_t tagLength;
#ifdef DEBUG_TRACE
#endif
  moveForwardUntilSignificant(); // to get the position more accurate
  *xmlParserPosition = getPosition();
  bool b = getSimpleTag(&tag, &tagLength, value, valueLength, "key", generateErrors);
  if ( !b ) {
    currentPos = *xmlParserPosition;
    return false;
  }
#ifdef DEBUG_TRACE
printf("XmlLiteParser::getKeyTagValue key=%.*s, line=%d, buffer=", (int)*valueLength, *value, (*xmlParserPosition).getLine());
for(size_t i=0 ; i<40 ; i++) printf("%c", (*xmlParserPosition).p[i] < 32 ? 0 : (*xmlParserPosition).p[i]);
printf("\n");
#endif
  // I think the following cannot happen anymore...
  if ( !strnIsEqualIC(tag, tagLength, "key") ) {
    addXmlError(generateErrors, S8Printf("Expecting a <key> at line %d col %d", (*xmlParserPosition).line, (*xmlParserPosition).col));
    currentPos = *xmlParserPosition;
    return false;
  }
//  if ( *valueLength == 0 ) {
//    if ( generateErrors ) addWarning(S8Printf("Expecting text for key tag at line %d col %d. SKipped\n", (*xmlParserPosition).line, (*xmlParserPosition).col));
//    b = skipNextTag();
//    if ( !b ) return false;
//    return getKeyTagValue(value, valueLength, xmlParserPosition, generateErrors);
//  }
//  if ( **value == '#' ) {
//    b = skipNextTag();
//    if ( !b ) return false;
//    return getKeyTagValue(value, valueLength, xmlParserPosition, generateErrors);
//  }
  return true;
}


bool XmlLiteParser::consumeOpeningTag(const char* expectedTag, bool generateErrors)
{
  const char* tag;
  size_t length;
  bool isOpeningTag, isClosingTag;

  auto pos = currentPos;

  RETURN_IF_FALSE ( getNextTag(&tag, &length, &isOpeningTag, &isClosingTag, generateErrors) );
  
  if ( !strnIsEqual(tag, length, expectedTag) ) {
    addXmlError(generateErrors, S8Printf("Expecting tag '%s' at line %d.", expectedTag, pos.getLine()));
    currentPos = pos;
    return false;
  }
  if ( isClosingTag ) {
    addXmlError(generateErrors, S8Printf("Expecting opening tag '%s' at line %d.", expectedTag, pos.getLine()));
    currentPos = pos;
    return false;
  }
  return true;
}

bool XmlLiteParser::consumeClosingTag(const char* expectedTag, bool generateErrors)
{
  const char* tag;
  size_t length;
  bool isOpeningTag, isClosingTag;

  auto pos = currentPos;

  RETURN_IF_FALSE ( getNextTag(&tag, &length, &isOpeningTag, &isClosingTag, generateErrors) );
  
  if ( !strnIsEqual(tag, length, expectedTag) ) {
    addXmlError(true, S8Printf("Expecting tag '%s' at line %d.", expectedTag, pos.getLine()));
    return false;
  }
  if ( isOpeningTag ) {
    addXmlError(true, S8Printf("Expecting closing tag '%s' at line %d.", expectedTag, pos.getLine()));
    return false;
  }
  return true;
}

bool XmlLiteParser::nextTagIsOpeningTag(const char* expectedTag)
{
  const char* tag;
  size_t length;
  bool isOpeningTag, isClosingTag;

  XmlParserPosition pos = getPosition();
  RETURN_IF_FALSE ( getNextTag(&tag, &length, &isOpeningTag, &isClosingTag, false) );
  restorePosition(pos);

  if ( !strnIsEqual(tag, length, expectedTag) ) return false;
  if ( isClosingTag ) return false;
  return true;
}

bool XmlLiteParser::nextTagIsClosingTag(const char* expectedTag)
{
  const char* tag;
  size_t length;
  bool isOpeningTag, isClosingTag;

  XmlParserPosition pos = getPosition();
  RETURN_IF_FALSE ( getNextTag(&tag, &length, &isOpeningTag, &isClosingTag, false) );
  restorePosition(pos);

  if ( !strnIsEqual(tag, length, expectedTag) ) return false;
  if ( isOpeningTag ) return false;
  return true;
}

