/*
 *
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 */

#ifndef __XML_LITE_H__
#define __XML_LITE_H__

#include "../cpp_foundation/XStringArray.h"
#include "../cpp_lib/XmlLiteSimpleTypes.h"
#include "../cpp_lib/XmlLiteParser.h"


#if defined(_MSC_VER) && !defined(__PRETTY_FUNCTION__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

bool strnnIsEqual(const char* key, size_t keyLength, const char* value, size_t valueLength);
bool strnIsEqual(const char* key, size_t keyLength, const char* value);
bool strnnIsEqualIC(const char* key, size_t keyLength, const char* value, size_t valueLength);
bool strnIsEqualIC(const char* key, size_t keyLength, const char* value);

class XmlParserMessage
{
  public:
    bool isError = true;
    XString8 msg;
    XmlParserMessage(bool _isError, const XString8& _msg) : isError(_isError), msg(_msg) {};
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
  
  bool operator == (const XmlParserPosition& other) const {
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

  bool AddErrorOrWarning(XmlParserMessage* msg) {
    if ( errorsAndWarnings.size() < 500 ) errorsAndWarnings.AddReference(msg, true);
    if ( errorsAndWarnings.size() == 500 ) errorsAndWarnings.AddReference(new XmlParserMessage(true, "Too many error. Stopping"_XS8), true);
    return false;
  }
public:
  bool xmlParsingError = false;

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
  bool addWarning(bool generateErrors, const XString8& warning) { if ( generateErrors ) AddErrorOrWarning(new XmlParserMessage(false, warning)); return false; }
  bool addError(bool generateErrors, const XString8& warning) { if ( generateErrors ) AddErrorOrWarning(new XmlParserMessage(true, warning)); return false; }
  // Xml stuctural error. Parsing should probably stop.
  bool addXmlError(bool generateErrors, const XString8& warning) {
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
  
  bool isEof() const { return currentPos.p >= p_end; }
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
  bool skipUntilClosingTag(const char* tagToSkip, size_t tagToSkipLength, bool generateErrors);
  bool skipNextTag(bool generateErrors);

  

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
  bool getNextTag(const char** tag, size_t* length, bool* isOpeningTag, bool* isClosingTag, bool generateErrors);
  bool getSimpleTag(const char** tag, size_t* tagLength, const char** value, size_t* valueLength, const char* expectedTag/*, bool valueCanBeEmpty*/, bool generateErrors);

  bool getSimpleTagValue(const char* tag, size_t tagLength, const char** value, size_t* valueLength, XmlParserPosition* xmlParserPosition, bool generateErrors);
  
  /*
   * Get the next tag, check it's a key, get the value and consume the closing tag ("</key>").
   */
  bool getKeyTagValue(const char** value, size_t* valueLength, XmlParserPosition* xmlParserPosition, bool generateErrors);
  
  /*
   * Check and consume the tag
   * Returns true if it's the expected opening tag
   * Returns false, add error in error list and don't change position if it's not
   */
  bool consumeOpeningTag(const char* expectedTag, bool generateErrors = true);
  
  /*
   * Check and consume the tag
   * Returns true if it's the expected closing tag
   * Returns false, add error in error list and don't change position if it's not
   */
  bool consumeClosingTag(const char* expectedTag, bool generateErrors);
  
  /*
   * Check but do NOT consume the tag
   * Returns true if it's the expected opening tag
   * Returns false, DO NOT add error in error list and don't change position if it's not
   */
  bool nextTagIsOpeningTag(const char* expectedTag);
  bool nextTagIsClosingTag(const char* expectedTag);

//  bool getTagBool(XmlBool* XmlBoolean);
  

};


#define RETURN_IF_FALSE(Expression) do { bool b = Expression; if ( !b ) return false; } while (0);



#endif // __XML_LITE_H__

