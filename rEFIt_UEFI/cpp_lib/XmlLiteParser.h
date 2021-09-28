/*
 *
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 */

#ifndef __XML_LITE_H__
#define __XML_LITE_H__

#include "../cpp_foundation/XBool.h"
#include "../cpp_foundation/XStringArray.h"
#include "../cpp_lib/XmlLiteSimpleTypes.h"
#include "../cpp_lib/XmlLiteParser.h"


#if defined(_MSC_VER) && !defined(__PRETTY_FUNCTION__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

XBool strnnIsEqual(const char* key, size_t keyLength, const char* value, size_t valueLength);
XBool strnIsEqual(const char* key, size_t keyLength, const char* value);
XBool strnnIsEqualIC(const char* key, size_t keyLength, const char* value, size_t valueLength);
XBool strnIsEqualIC(const char* key, size_t keyLength, const char* value);

class XmlParserMessage
{
  public:
    XBool isError = true;
    XString8 msg;
    XmlParserMessage(XBool _isError, const XString8& _msg) : isError(_isError), msg(_msg) {};
};

class XmlLiteParser;

class XmlParserPosition
{
  friend class XmlLiteParser;

public:
  char* p;
  int line;
protected:
  int col;
public:
  
  XmlParserPosition() : p(NULL), line(1), col(1) {};
  XmlParserPosition(char* _p) : p(_p), line(1), col(1) {};

  int getLine() const { return line; }
  int getCol() const { return col; }
  
  XBool operator == (const XmlParserPosition& other) const {
    return p == other.p;
  }
};

class XmlLiteParser
{
  friend class XmlParserPosition;
  
  char* p_start = NULL;
  char* p_end = NULL;
  XmlParserPosition currentPos = XmlParserPosition();
  XObjArray<XmlParserMessage> errorsAndWarnings = XObjArray<XmlParserMessage>();

  XBool AddErrorOrWarning(XmlParserMessage* msg) {
    if ( errorsAndWarnings.size() < 500 ) errorsAndWarnings.AddReference(msg, true);
    if ( errorsAndWarnings.size() == 500 ) errorsAndWarnings.AddReference(new XmlParserMessage(true, "Too many error. Stopping"_XS8), true);
    return false;
  }
public:
  XBool xmlParsingError = false;

  XmlLiteParser() {};
  XmlLiteParser(char* _p) : p_start(_p)
  {
    p_end = p_start + strlen(p_start);
  };
  XmlLiteParser(const XmlLiteParser&) = delete;
  XmlLiteParser& operator = (const XmlLiteParser&) = delete;

  void init(const char* buf, size_t size);
  void init(const char* buf) { init(buf, strlen(buf)); };
  
  int getLine() { return currentPos.line; }
  int getCol() { return currentPos.col; }
  XObjArray<XmlParserMessage>& getErrorsAndWarnings() { return errorsAndWarnings; }
  // Add warning, error and xml error always return false so you can return addWarning(...) from validate function
  XBool addWarning(XBool generateErrors, const XString8& warning) { if ( generateErrors ) AddErrorOrWarning(new XmlParserMessage(false, warning)); return false; }
  XBool addError(XBool generateErrors, const XString8& warning) { if ( generateErrors ) AddErrorOrWarning(new XmlParserMessage(true, warning)); return false; }
  // Xml stuctural error. Parsing should probably stop.
  XBool addXmlError(XBool generateErrors, const XString8& warning) {
    if ( generateErrors ) {xmlParsingError = true;  AddErrorOrWarning(new XmlParserMessage(true, warning));}
    return false;
  }
  void printfErrorsAndWarnings() {
    for ( size_t idx = 0 ; idx < getErrorsAndWarnings().size() ; idx++ ) {
      printf("%s: %s\n", getErrorsAndWarnings()[idx].isError ? "Error" : "Warning", getErrorsAndWarnings()[idx].msg.c_str());
    }
  }

  XmlParserPosition getPosition();
  void restorePosition(XmlParserPosition& xml_position);
  
  XBool isEof() const { return currentPos.p >= p_end; }
  char getchar();
  char* getcharPtr() { return currentPos.p; };

  char moveForward();
  char moveForward(int n);
  char moveBackward();
  char moveForwardUntil(char until);
  char moveBackwardUntil(char until);
  char moveForwardPastNext(char until);

  char moveForwardUntilSignificant();
  char moveBackwardUntilSignificant();
  
  void skipHeader();
  /*
   * The opening tag has been read. Skip until the closing tag
   */
  XBool skipUntilClosingTag(const char* tagToSkip, size_t tagToSkipLength, XBool generateErrors);
  XBool skipNextTag(XBool generateErrors);

  

  /*
   * Get the string from current position until the next '<', or end of buffer.
   * Spaces at begining or end are skipped.
   * If current char is '<', empty string is returned. (*string=NULL and *length=0).
   * If current char is '/', empty string is returned. This is because getNextTag leaves to pinter to the '/' for empty tag.
   * If no '<', false is returned.
   * Position at the '<' when exit.
   */
  void getString(const char** string, size_t* length);
  
  /*
   * Get the next tag, either opening or closing tag.
   * For empty tag (</true>), first call return that it's an opening tag. Second call return that it's a closing tag.
   */
  XBool getNextTag(const char** tag, size_t* length, XBool* isOpeningTag, XBool* isClosingTag, XBool generateErrors);
  XBool getSimpleTag(const char** tag, size_t* tagLength, const char** value, size_t* valueLength, const char* expectedTag/*, XBool valueCanBeEmpty*/, XBool generateErrors);

  XBool getSimpleTagValue(const char* tag, size_t tagLength, const char** value, size_t* valueLength, XmlParserPosition* xmlParserPosition, XBool generateErrors);
  
  /*
   * Get the next tag, check it's a key, get the value and consume the closing tag ("</key>").
   */
  XBool getKeyTagValue(const char** value, size_t* valueLength, XmlParserPosition* xmlParserPosition, XBool generateErrors);
  
  /*
   * Check and consume the tag
   * Returns true if it's the expected opening tag
   * Returns false, add error in error list and don't change position if it's not
   */
  XBool consumeOpeningTag(const char* expectedTag, XBool generateErrors = true);
  
  /*
   * Check and consume the tag
   * Returns true if it's the expected closing tag
   * Returns false, add error in error list and don't change position if it's not
   */
  XBool consumeClosingTag(const char* expectedTag, XBool generateErrors);
  
  /*
   * Check but do NOT consume the tag
   * Returns true if it's the expected opening tag
   * Returns false, DO NOT add error in error list and don't change position if it's not
   */
  XBool nextTagIsOpeningTag(const char* expectedTag);
  XBool nextTagIsClosingTag(const char* expectedTag);

//  XBool getTagBool(XmlBool* XmlBoolean);
  

};


#define RETURN_IF_FALSE(Expression) do { XBool b = Expression; if ( !b ) return false; } while (0);



#endif // __XML_LITE_H__

