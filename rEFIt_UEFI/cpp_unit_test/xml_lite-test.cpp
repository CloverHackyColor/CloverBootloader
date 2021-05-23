#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "../cpp_foundation/XString.h"
#include "../cpp_foundation/XStringArray.h"
//#include "../cpp_foundation/unicode_conversions.h"
//#include "../Platform/plist/plist.h"

#include "../cpp_lib/XmlLiteSimpleTypes.h"
#include "../cpp_lib/XmlLiteCompositeTypes.h"
#include "../cpp_lib/XmlLiteDictTypes.h"
#include "../cpp_lib/XmlLiteArrayTypes.h"
#include "../cpp_lib/XmlLiteParser.h"
#include "../Settings/ConfigPlist/ConfigPlistAbstract.h"

#include "xml_lite-reapeatingdict-test.h"

static int breakpoint(int i)
{
  return i;
}


static XmlLiteParser gXmlLiteParserTest;





int move_tests()
{
  gXmlLiteParserTest.init("\n\n\n<a>\n  <b></b>\n</a>\n");
  while ( gXmlLiteParserTest.getLine() < 4 && gXmlLiteParserTest.moveForward() ) ;
  while ( gXmlLiteParserTest.moveBackward() );
  if ( gXmlLiteParserTest.getPosition().getLine() != 1 || gXmlLiteParserTest.getPosition().getCol() != 1 ) return 1;
  
  return 0;
}

int getNextTag_tests()
{
  const char* tag;
  size_t tagLength;
  bool isOpeningTag, isClosingTag;
  bool b;

  gXmlLiteParserTest.init("<key>");
  b = gXmlLiteParserTest.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( !b ) return breakpoint(1);
  if ( !isOpeningTag ) return breakpoint(2);
  if ( isClosingTag ) return breakpoint(3);
  if ( gXmlLiteParserTest.getchar() != 0 ) return breakpoint(4);

  
  gXmlLiteParserTest.init("</key>");
  b = gXmlLiteParserTest.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( !b ) return breakpoint(5);
  if ( isOpeningTag ) return breakpoint(6);
  if ( !isClosingTag ) return breakpoint(7);
  if ( gXmlLiteParserTest.getchar() != 0 ) return breakpoint(8);

  
  gXmlLiteParserTest.init("<key/>");
  b = gXmlLiteParserTest.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( !b ) return breakpoint(10);
  if ( !isOpeningTag ) return breakpoint(11);
  if ( isClosingTag ) return breakpoint(12);
  b = gXmlLiteParserTest.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( !b ) return breakpoint(13);
  if ( isOpeningTag ) return breakpoint(14);
  if ( !isClosingTag ) return breakpoint(15);

  //
  // Test xmlLiteParser.getErrorsAndWarnings()
  //
  gXmlLiteParserTest.init("foo1\n  foo2");
  gXmlLiteParserTest.moveForwardUntil(0);
  b = gXmlLiteParserTest.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( b ) return breakpoint(13);
  if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParserTest.getErrorsAndWarnings()[0].msg.contains("line 2 col 7") ) return breakpoint(14);

  gXmlLiteParserTest.init("foo1\n  bar1");
  gXmlLiteParserTest.moveForwardUntil('b');
  b = gXmlLiteParserTest.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( b ) return breakpoint(13);
  if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParserTest.getErrorsAndWarnings()[0].msg.contains("line 2 col 3") ) return breakpoint(14);
  
  gXmlLiteParserTest.init("foo1\n  </foo2/>");
  gXmlLiteParserTest.moveForwardUntil('<');
  b = gXmlLiteParserTest.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( b ) return breakpoint(13);
  if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParserTest.getErrorsAndWarnings()[0].msg.contains("line 2 col 9") ) return breakpoint(14);
  
  gXmlLiteParserTest.init("foo1\n    </foo2");
  gXmlLiteParserTest.moveForwardUntil('<');
  b = gXmlLiteParserTest.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( b ) return breakpoint(13);
  if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParserTest.getErrorsAndWarnings()[0].msg.contains("line 2 col 11") ) return breakpoint(14);
  
  gXmlLiteParserTest.init("foo1\n      </foo2 >");
  gXmlLiteParserTest.moveForwardUntil('<');
  b = gXmlLiteParserTest.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( b ) return breakpoint(13);
  if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParserTest.getErrorsAndWarnings()[0].msg.contains("line 2 col 13") ) return breakpoint(14);

  gXmlLiteParserTest.init("foo1\n\n<foo2/");
  gXmlLiteParserTest.moveForwardUntil('/');
  gXmlLiteParserTest.moveForwardUntilSignificant();
  b = gXmlLiteParserTest.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( b ) return breakpoint(13);
  if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParserTest.getErrorsAndWarnings()[0].msg.contains("line 3 col 7") ) return breakpoint(14);

  gXmlLiteParserTest.init("foo1\n\n  <foo2/a");
  gXmlLiteParserTest.moveForwardUntil('/');
  gXmlLiteParserTest.moveForwardUntilSignificant();
  b = gXmlLiteParserTest.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( b ) return breakpoint(13);
  if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParserTest.getErrorsAndWarnings()[0].msg.contains("line 3 col 9") ) return breakpoint(14);


  return 0;
}

int getSimpleTag_tests()
{
  const char* tag;
  size_t tagLength;
  const char* value;
  size_t valueLength;
  bool b;
  
  gXmlLiteParserTest.init("<key>k</key><string>v</string>");
  b = gXmlLiteParserTest.getSimpleTag(&tag, &tagLength, &value, &valueLength, "key", true);
  if ( !b ) return breakpoint(1);
  if ( !strnIsEqual(tag, tagLength, "key") ) return breakpoint(2);
  if ( !strnIsEqual(value, valueLength, "k") ) return breakpoint(3);
  
  gXmlLiteParserTest.init("<key/>");
  b = gXmlLiteParserTest.getSimpleTag(&tag, &tagLength, &value, &valueLength, "key", true);
  if ( !b ) return breakpoint(1);
  if ( value != NULL ) return breakpoint(3);
  if ( valueLength != 0 ) return breakpoint(3);

  gXmlLiteParserTest.init("<key></key>");
  b = gXmlLiteParserTest.getSimpleTag(&tag, &tagLength, &value, &valueLength, "key", true);
  if ( !b ) return breakpoint(1);
  if ( value != NULL ) return breakpoint(3);
  if ( valueLength != 0 ) return breakpoint(3);

  gXmlLiteParserTest.init("<true/>");
  b = gXmlLiteParserTest.getSimpleTag(&tag, &tagLength, &value, &valueLength, "true", true);
  if ( !b ) return breakpoint(1);
  if ( !strnIsEqual(tag, tagLength, "true") ) return breakpoint(2);
  if ( value != NULL ) return breakpoint(3);
  if ( valueLength != 0 ) return breakpoint(3);

  gXmlLiteParserTest.init("<key>k<string>v</string>k</key>");
  b = gXmlLiteParserTest.getSimpleTag(&tag, &tagLength, &value, &valueLength, "key", true);
  if ( b ) return breakpoint(4);

  //
  // Test xmlLiteParser.getErrorsAndWarnings()
  //
  gXmlLiteParserTest.init("\n\n  </key>k</key>");
  gXmlLiteParserTest.moveForwardUntilSignificant();
  b = gXmlLiteParserTest.getSimpleTag(&tag, &tagLength, &value, &valueLength, "key", true);
  if ( b ) return breakpoint(4);
  if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParserTest.getErrorsAndWarnings()[0].msg.contains("line 3 col 3") ) return breakpoint(14);
  
  gXmlLiteParserTest.init("\n\n    <foo>k</foo>");
  gXmlLiteParserTest.moveForwardUntilSignificant();
  b = gXmlLiteParserTest.getSimpleTag(&tag, &tagLength, &value, &valueLength, "key", true);
  if ( b ) return breakpoint(4);
  if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParserTest.getErrorsAndWarnings()[0].msg.contains("line 3 col 5") ) return breakpoint(14);
  
  gXmlLiteParserTest.init("\n\n    <key></key>");
  gXmlLiteParserTest.moveForwardUntilSignificant();
  b = gXmlLiteParserTest.getSimpleTag(&tag, &tagLength, &value, &valueLength, "key", true);
  if ( !b ) return breakpoint(4);
  if ( value != NULL ) return breakpoint(3);
  if ( valueLength != 0 ) return breakpoint(3);

  gXmlLiteParserTest.init("\n\n    <key>v<key>");
  gXmlLiteParserTest.moveForwardUntilSignificant();
  b = gXmlLiteParserTest.getSimpleTag(&tag, &tagLength, &value, &valueLength, "key", true);
  if ( b ) return breakpoint(4);
  if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParserTest.getErrorsAndWarnings()[0].msg.contains("line 3 col 11") ) return breakpoint(14);

  gXmlLiteParserTest.init("\n\n    <key>v</foo>");
  gXmlLiteParserTest.moveForwardUntilSignificant();
  b = gXmlLiteParserTest.getSimpleTag(&tag, &tagLength, &value, &valueLength, "key", true);
  if ( b ) return breakpoint(4);
  if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParserTest.getErrorsAndWarnings()[0].msg.contains("line 3 col 11") ) return breakpoint(14);

  return 0;
}

int getKey_tests()
{
  const char* tag;
  size_t length;
  bool b;
  XmlParserPosition xmlParserPosition;

  gXmlLiteParserTest.init("<key></key><string>v</string>");
  b = gXmlLiteParserTest.getKeyTagValue(&tag, &length, &xmlParserPosition, true);
  if ( !b ) return breakpoint(3);
  if ( length != 0 ) return breakpoint(13);

  gXmlLiteParserTest.init("<key> </key><string>v</string>");
  b = gXmlLiteParserTest.getKeyTagValue(&tag, &length, &xmlParserPosition, true);
  if ( !b ) return breakpoint(3);
  if ( length != 1 ) return breakpoint(13);

  gXmlLiteParserTest.init("<key>a</key><string>v</string>");
  b = gXmlLiteParserTest.getKeyTagValue(&tag, &length, &xmlParserPosition, true);
  if ( !b ) return breakpoint(5);
  if ( !strnIsEqual(tag, length, "a") ) return breakpoint(6);

  gXmlLiteParserTest.init("<key> a </key><string>v</string>");
  b = gXmlLiteParserTest.getKeyTagValue(&tag, &length, &xmlParserPosition, true);
  if ( !b ) return breakpoint(6);
  if ( !strnIsEqual(tag, length, " a ") ) return breakpoint(6);

  gXmlLiteParserTest.init("<key>  a  </key><string>v</string>");
  b = gXmlLiteParserTest.getKeyTagValue(&tag, &length, &xmlParserPosition, true);
  if ( !b ) return breakpoint(7);
  if ( !strnIsEqual(tag, length, "  a  ") ) return breakpoint(6);

  gXmlLiteParserTest.init("<key>a  </key><string>v</string>");
  b = gXmlLiteParserTest.getKeyTagValue(&tag, &length, &xmlParserPosition, true);
  if ( !b ) return breakpoint(8);
  if ( !strnIsEqual(tag, length, "a  ") ) return breakpoint(6);

  gXmlLiteParserTest.init("<key>  a</key><string>v</string>");
  b = gXmlLiteParserTest.getKeyTagValue(&tag, &length, &xmlParserPosition, true);
  if ( !b ) return breakpoint(9);
  if ( !strnIsEqual(tag, length, "  a") ) return breakpoint(6);

  return 0;
}

int skip_tests()
{
  bool b;

  gXmlLiteParserTest.init("k</key><string>v</string>");
  b = gXmlLiteParserTest.skipUntilClosingTag("key", strlen("key"), true);
  if ( !b ) return breakpoint(1);
  if ( !strnIsEqual(gXmlLiteParserTest.getcharPtr(), strlen(gXmlLiteParserTest.getcharPtr()), "<string>v</string>") ) return breakpoint(2);

  gXmlLiteParserTest.init("<string>v</string></key>foo"); // 'foo' is text after closing tag -> fail
  b = gXmlLiteParserTest.skipUntilClosingTag("key", strlen("key"), true);
  if ( b ) return breakpoint(1);
  if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
auto msg = gXmlLiteParserTest.getErrorsAndWarnings()[0].msg;
  if ( !gXmlLiteParserTest.getErrorsAndWarnings()[0].msg.contains("line 1 col 25") ) return breakpoint(14);

  gXmlLiteParserTest.init("<string>v</string></key><foo>"); // <foo> is the next tag -> success
  b = gXmlLiteParserTest.skipUntilClosingTag("key", strlen("key"), true);
  if ( !b ) return breakpoint(1);
  if ( !strnIsEqual(gXmlLiteParserTest.getcharPtr(), strlen(gXmlLiteParserTest.getcharPtr()), "<foo>") ) return breakpoint(2);
  
  gXmlLiteParserTest.init("<string>v</string></key>"); // end of file just after skipped tag -> success
  b = gXmlLiteParserTest.skipUntilClosingTag("key", strlen("key"), true);
  if ( !b ) return breakpoint(1);
  if ( gXmlLiteParserTest.getchar() != 0 ) return breakpoint(2);

  gXmlLiteParserTest.init("<string>v</string><foo1><foo11>bar11</foo11></foo1><foo2>bar2</foo2></key><meow>");
  b = gXmlLiteParserTest.skipUntilClosingTag("key", strlen("key"), true);
  if ( !b ) return breakpoint(1);
  if ( !strnIsEqual(gXmlLiteParserTest.getcharPtr(), strlen(gXmlLiteParserTest.getcharPtr()), "<meow>") ) return breakpoint(2);

  // Cannot have a tag containing chars AND subtag
  gXmlLiteParserTest.init("\n\n\nk<string>v</string></key>");
  gXmlLiteParserTest.moveForwardUntil('k');
  b = gXmlLiteParserTest.skipUntilClosingTag("key", strlen("key"), true);
  if ( b ) return breakpoint(2);
  if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParserTest.getErrorsAndWarnings()[0].msg.contains("line 4 col 2") ) return breakpoint(14);

  // Cannot have a tag containing chars AND subtag
  gXmlLiteParserTest.init("<string>v</string>k</key>");
  b = gXmlLiteParserTest.skipUntilClosingTag("key", strlen("key"), true);
  if ( b ) return breakpoint(2);
  if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParserTest.getErrorsAndWarnings()[0].msg.contains("line 1 col 19") ) return breakpoint(14);

  
  // Cannot have a tag containing chars AND subtag
  gXmlLiteParserTest.init("\n </string>v</string>k</key>");
  gXmlLiteParserTest.moveForwardUntil('<');
  b = gXmlLiteParserTest.skipNextTag(true);
  if ( b ) return breakpoint(2);
  if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParserTest.getErrorsAndWarnings()[0].msg.contains("line 2 col 2") ) return breakpoint(14);
  
  return 0;
}

int xml_integer_tests()
{
//  XmlAbstractType xml_int8;
//  bool b;
//  UINTN result;
//  bool negative;
//  XString8 s;
//  
//  xmlLiteParser.init("<integer>10</integer>");
//  b = xml_int8.XmlInteger::parseFromXmlLite(&xmlLiteParser, "xmlpath"_XS8, &result, &negative, 0, 12);
//  if ( !b ) return breakpoint(1);
//  if ( result != 10 ) return breakpoint(2);
//  if ( negative ) return breakpoint(3);
//
//  xmlLiteParser.init("<integer>0</integer>");
//  b = xml_int8.XmlInteger::parseFromXmlLite(&xmlLiteParser, "xmlpath"_XS8, &result, &negative, -2, 12);
//  if ( !b ) return breakpoint(10);
//  if ( result != 0 ) return breakpoint(11);
//  if ( negative ) return breakpoint(12);
//
//  xmlLiteParser.init("<integer>-0</integer>");
//  b = xml_int8.XmlInteger::parseFromXmlLite(&xmlLiteParser, "xmlpath"_XS8, &result, &negative, 0, 0);
//  if ( !b ) return breakpoint(20);
//  if ( result != 0 ) return breakpoint(21);
//  if ( negative ) return breakpoint(22);
//
//  xmlLiteParser.init("<integer>10</integer>");
//  b = xml_int8.XmlInteger::parseFromXmlLite(&xmlLiteParser, "xmlpath"_XS8, &result, &negative, 0, UINT64_MAX);
//  if ( !b ) return breakpoint(30);
//  if ( result != 10 ) return breakpoint(31);
//  if ( negative ) return breakpoint(32);
//
//  xmlLiteParser.init("<integer>10</integer>");
//  b = xml_int8.XmlInteger::parseFromXmlLite(&xmlLiteParser, "xmlpath"_XS8, &result, &negative, 0, 12);
//  if ( !b ) return breakpoint(40);
//  if ( result != 10 ) return breakpoint(41);
//  if ( negative ) return breakpoint(42);
//
//  s = S8Printf("<integer>%llu</integer>", UINT64_MAX);
//  xmlLiteParser.init(s.c_str());
//  b = xml_int8.XmlInteger::parseFromXmlLite(&xmlLiteParser, "xmlpath"_XS8, &result, &negative, 0, UINT64_MAX);
//  if ( !b ) return breakpoint(50);
//  if ( result != UINT64_MAX ) return breakpoint(51);
//  if ( negative ) return breakpoint(52);
//  
//  s = S8Printf("<integer>%lld</integer>", INT64_MIN);
//  xmlLiteParser.init(s.c_str());
//  b = xml_int8.XmlInteger::parseFromXmlLite(&xmlLiteParser, "xmlpath"_XS8, &result, &negative, INT64_MIN, UINT64_MAX);
//  if ( !b ) return breakpoint(60);
//  if ( result != (UINT64)INT64_MIN ) return breakpoint(61);
//  if ( !negative ) return breakpoint(62);
//
//  
//  xmlLiteParser.init("<integer>-1000</integer>");
//  b = xml_int8.XmlInteger::parseFromXmlLite(&xmlLiteParser, "xmlpath"_XS8, &result, &negative, -1000, 0);
//  if ( !b ) return breakpoint(70);
//  if ( result != 1000 ) return breakpoint(71);
//  if ( !negative ) return breakpoint(72);
//
//  
//  xmlLiteParser.init("<integer>-1001</integer>");
//  xmlLiteParser.init(s.c_str());
//  b = xml_int8.XmlInteger::parseFromXmlLite(&xmlLiteParser, "xmlpath"_XS8, &result, &negative, -1000, 0);
//  if ( b ) return breakpoint(80);

  return 0;
}

int validate_dict_tests()
{
  bool b;

  class Dict1_Class : public XmlDict
  {
    public:
      class Test1Bool: public XmlBool
      {
      public:
        virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
          RETURN_IF_FALSE( XmlBool::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) );
          xmlLiteParser->addWarning(generateErrors, S8Printf("Test1Bool tag '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
          return false; // parsing can continue.
        }
      } test1Bool = Test1Bool();

      XmlDictField m_fields[1] = {
        {"test1Bool", test1Bool},
      };
      virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
  };

  class Main1Dict_Class : public XmlDict
  {
    public:
      Dict1_Class dict1 = Dict1_Class();

      virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
        RETURN_IF_FALSE( XmlDict::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) );
        xmlLiteParser->addWarning(generateErrors, S8Printf("dict1 tag '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
        return false; // parsing can continue.
      }

      XmlDictField m_fields[1] = {
        {"dict1", dict1},
      };

      Main1Dict_Class() {};
      virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
  } mainDict;


  const char* config_test =
   "<dict>\r\n\
      <key>dict1</key>\r\n\
      <dict>\r\n\
        <key>test1Bool</key>\r\n\
        <true/>\r\n\
      </dict>\r\n\
    </dict>\r\n\
  </plist>";

    gXmlLiteParserTest.init(config_test);
    b = mainDict.parseFromXmlLite(&gXmlLiteParserTest, "/"_XS8, true);
//gXmlLiteParser.printfErrorsAndWarnings();
    if ( !b ) return breakpoint(1);
    if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 1 ) return breakpoint(1);
    if ( !gXmlLiteParserTest.getErrorsAndWarnings()[0].msg.contains("Test1Bool tag") ) return breakpoint(14);

    if ( !b ) return breakpoint(1);
    gXmlLiteParserTest.init(config_test);
    mainDict.validate(&gXmlLiteParserTest, "/"_XS8, XmlParserPosition(), true);
//gXmlLiteParser.printfErrorsAndWarnings();
    if ( !b ) return breakpoint(1);
    if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 1 ) return breakpoint(1);
    if ( !gXmlLiteParserTest.getErrorsAndWarnings()[0].msg.contains("ict1 tag") ) return breakpoint(14);
    
  return 0;
}

int validate_array_tests()
{
  bool b;

  class Main2Dict_Class : public XmlDict
  {
    public:
      XmlArray<XmlBool> array = XmlArray<XmlBool>();

      virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
        RETURN_IF_FALSE( XmlDict::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) );
        xmlLiteParser->addWarning(generateErrors, S8Printf("dict2 tag '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
        return false; // parsing can continue.
      }

      XmlDictField m_fields[1] = {
        {"array1", array},
      };

      Main2Dict_Class() {};
      virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
  } mainDict;


  const char* config_test =
   "<dict>\r\n\
      <key>array1</key>\r\n\
      <array>\r\n\
        <true/>\r\n\
        <true/>\r\n\
        <string>a</string>\r\n\
      </array>\r\n\
    </dict>\r\n\
  </plist>";

    gXmlLiteParserTest.init(config_test);
    b = mainDict.parseFromXmlLite(&gXmlLiteParserTest, "/"_XS8, true);
//gXmlLiteParser.printfErrorsAndWarnings();
    if ( !b ) return breakpoint(1);
    if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 1 ) return breakpoint(1);
    if ( !gXmlLiteParserTest.getErrorsAndWarnings()[0].msg.contains("Expecting <true/> <false/>") ) return breakpoint(14);

    gXmlLiteParserTest.init(config_test);
    mainDict.validate(&gXmlLiteParserTest, "/"_XS8, XmlParserPosition(), true);
//gXmlLiteParser.printfErrorsAndWarnings();
    if ( !b ) return breakpoint(1);
    if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 1 ) return breakpoint(1);
    if ( !gXmlLiteParserTest.getErrorsAndWarnings()[0].msg.contains("dict2 tag") ) return breakpoint(14);
    
    return 0;
}


int documentation_test1()
{
  bool b;

  class MyDictClass : public XmlDict
  {
    public:
      XmlBool aBool {};
      XmlInt32 anInt32 {};

      XmlDictField m_fields[2] = {
          {"KeyNameForBool", aBool},
          {"KeyNameForInt32", anInt32},
      };
      virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
  } MyDict = MyDictClass();


  const char* config_test = R"V0G0N(
   <?xml version="1.0" encoding="UTF-8"?>
   <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
   <plist version="1.0">
   <dict>
       <key>KeyNameForBool</key>
       <true/>
       <key>KeyNameForInt32</key>
       <integer>13864</integer>
   </dict>
  )V0G0N";
    
    
    gXmlLiteParserTest.init(config_test);
    gXmlLiteParserTest.moveForwardUntilSignificant();
    gXmlLiteParserTest.skipHeader();
    b = MyDict.parseFromXmlLite(&gXmlLiteParserTest, "/"_XS8, true);
gXmlLiteParserTest.printfErrorsAndWarnings();
    if ( !b ) return breakpoint(1);
    if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 0 ) return breakpoint(1);
   
    return 0;
}

int documentation_test2()
{
  bool b;

  class MyInsideDictClass : public XmlDict
  {
    public:
      XmlBool aBool {};
      XmlInt32 anInt32 {};

      XmlDictField m_fields[2] = {
          {"KeyNameForBool", aBool},
          {"KeyNameForInt32", anInt32},
      };
      virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
  };

  class MyTopLevelDictClass : public ConfigPlistAbstractClass
  {
    public:
      MyInsideDictClass inside1 {};

      XmlDictField m_fields[1] = {
          {"KeyNameForInsideDict", inside1},
      };
      virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
  } MyDict = MyTopLevelDictClass();

  const char* config_test = R"V0G0N(
   <?xml version="1.0" encoding="UTF-8"?>
   <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
   <plist version="1.0">
   <dict>
       <key>KeyNameForInsideDict</key>
       <dict>
         <key>KeyNameForBool</key>
         <true/>
         <key>KeyNameForInt32</key>
         <integer>13864</integer>
     </dict>
   </dict>
  )V0G0N";
    
    
    b = MyDict.parse(config_test, strlen(config_test), "/"_XS8, &gXmlLiteParserTest);
gXmlLiteParserTest.printfErrorsAndWarnings();
    if ( !b ) return breakpoint(1);
    if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 0 ) return breakpoint(1);
   
    return 0;
}

int documentation_test3()
{
  bool b;

  class MyPlist : public ConfigPlistAbstractClass
  {
    public:
      XmlBool aBool {};
      XmlInt32 anInt32 {};
      class CountClass : public XmlInt64
      {
        using super = XmlInt64;
        virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
          if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
          if ( value() < -2 ) {
              xmlLiteParser->addWarning(generateErrors, S8Printf("Count cannot be negative. It must a number between -2 and 18 inclusive at '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
              return false;
          }
          if ( value() > 18 ) {
              xmlLiteParser->addWarning(generateErrors, S8Printf("Count cannot > 18. It must a number between -2 and 18 inclusive at '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
              return false;
          }
          return true;
        }
      } Count = CountClass();

      XmlDictField m_fields[1] = {
          {"Count", Count},
      };
      virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
  } MyDict = MyPlist();


  const char* config_test = R"V0G0N(
   <?xml version="1.0" encoding="UTF-8"?>
   <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
   <plist version="1.0">
   <dict>
       <key>Count</key>
       <integer>12</integer>
   </dict>
  )V0G0N";

    
    b = MyDict.parse(config_test, strlen(config_test), "/"_XS8, &gXmlLiteParserTest);
gXmlLiteParserTest.printfErrorsAndWarnings();
    if ( !b ) return breakpoint(1);
    if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 0 ) return breakpoint(1);
   
    return 0;
}

int documentation_test4()
{
  bool b;
  class MyXmlType : public XmlUInt8
  {
    using super = XmlUInt8;
    virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
      if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
      if ( value() < 1  || value() > 2 ) {
          xmlLiteParser->addWarning(generateErrors, S8Printf("Type must be 1 or 2 at '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
          return false;
      }
      return true;
    }
  };

  class MyXmlSubType : public XmlUInt8
  {
    using super = XmlUInt8;
    virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
      if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
      if ( value() < 11  || value() > 12 ) {
          xmlLiteParser->addWarning(generateErrors, S8Printf("SubType must be 11 or 22 at '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
          return false;
      }
      return true;
    }
  };

  class MyPlist : public ConfigPlistAbstractClass
  {
      using super = XmlDict;
    public:
      MyXmlType type {};       // this is a subclass of XmlUInt8 that check that type is 1 or 2
      MyXmlSubType subType {}; // this is a subclass of XmlUInt8 that check that subtype is 11 or 12
      XmlString8 name {};      // as many other field that there is in this dict

      XmlDictField m_fields[2] = {
          {"Type", type},
          {"SubType", subType},
      };
      virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
      
      virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
        if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
        if ( !type.isDefined() ) {
            xmlLiteParser->addWarning(generateErrors, S8Printf("Type must befined at '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
            return false;
        }
        if ( type.value() == 1 ) {
          if ( subType.isDefined() ) {
              xmlLiteParser->addWarning(generateErrors, S8Printf("Type 1 cannot have a subtype at '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
              return false;
          }
        }else if ( type.value() == 2 ) {
          // nothing to do because subtype is optional, and if it exists, weknow that the value is correct because of th validation in MyXmlSubType
        }else{
          panic("There is a bug in MyXmlType::validate() !");
        }
        return true;
      }
  } MyDict = MyPlist();


  const char* config_test = R"V0G0N(
   <?xml version="1.0" encoding="UTF-8"?>
   <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
   <plist version="1.0">
   <dict>
      <key>Type</key>
      <integer>2</integer>
      <key>SubType</key>
      <integer>11</integer>
   </dict>
  )V0G0N";

    
    b = MyDict.parse(config_test, strlen(config_test), "/"_XS8, &gXmlLiteParserTest);
gXmlLiteParserTest.printfErrorsAndWarnings();
    if ( !b ) return breakpoint(1);
    if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 0 ) return breakpoint(1);
   
    return 0;
}

int documentation_test5()
{
  bool b;
  class MyPlist : public ConfigPlistAbstractClass
  {
      using super = XmlDict;
    public:
      XmlUInt8 type {}; // no validation except that the value is an unsigned 8 bits int
      XmlUInt8 subType {}; // no validation except that the value is an unsigned 8 bits int
      XmlString8 name {};      // as many other field that there is in this dict

      XmlDictField m_fields[2] = {
          {"Type", type},
          {"SubType", subType},
      };
      virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
      
      virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
        if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
        if ( !type.isDefined() ) {
            xmlLiteParser->addWarning(generateErrors, S8Printf("Type must befined at '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
            return false;
        }
        if ( type.value() == 1 ) {
          if ( subType.isDefined() ) {
              xmlLiteParser->addWarning(generateErrors, S8Printf("Type 1 cannot have a subtype in dict '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
              return false;
          }
        }else if ( type.value() == 2 ) {
          if ( subType.isDefined() ) {
              if ( subType.value() != 11  &&  subType.value() != 12 ) {
                xmlLiteParser->addWarning(generateErrors, S8Printf("SubType must be 11 or 12 at '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
                return false;
              }
          }else{
            // subtype is optional, so it's ok.
          }
        }else{
          xmlLiteParser->addWarning(generateErrors, S8Printf("Type must be 1 or 2 at '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
          // Let's think that we want to ignore this value but we syill want to keep the dict as the other field still has meaning.
          type.reset(); // we only reset this field. We don't return false because that'll undefine the whole dict
          subType.reset(); // SubType means nothing without a Type.
        }
        return true;
      }
  } MyDict = MyPlist();


  const char* config_test = R"V0G0N(
   <?xml version="1.0" encoding="UTF-8"?>
   <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
   <plist version="1.0">
   <dict>
      <key>Type</key>
      <integer>2</integer>
      <key>SubType</key>
      <integer>11</integer>
   </dict>
  )V0G0N";

    
    b = MyDict.parse(config_test, strlen(config_test), "/"_XS8, &gXmlLiteParserTest);
gXmlLiteParserTest.printfErrorsAndWarnings();
    if ( !b ) return breakpoint(1);
    if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 0 ) return breakpoint(1);
   
    return 0;
}

int documentation_test6()
{
  bool b;

  class MyDictClass : public ConfigPlistAbstractClass
  {
    public:
      XmlArray<XmlBool> aBoolArray {};

      XmlDictField m_fields[1] = {
          {"KeyNameForBoolArray", aBoolArray},
      };
      virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
  } MyDict = MyDictClass();

  const char* config_test = R"V0G0N(
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>KeyNameForBoolArray</key>
    <array>
        <true/>
        <false/>
        <true/>
    </array>
</dict>
  )V0G0N";
    
    
    b = MyDict.parse(config_test, strlen(config_test), "/"_XS8, &gXmlLiteParserTest);
gXmlLiteParserTest.printfErrorsAndWarnings();
    if ( !b ) return breakpoint(1);
    if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 0 ) return breakpoint(1);
   
    return 0;
}

int documentation_test7()
{
  bool b;

  class MyDictClass : public ConfigPlistAbstractClass
  {
    public:
      XmlRepeatingDict<XmlAddKey<XmlKey, XmlInt32>> keyIntPairs {};

      XmlDictField m_fields[1] = {
          {"KeyNameForKeyIntPairs", keyIntPairs},
      };
      virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
  } MyDict = MyDictClass();

  const char* config_test = R"V0G0N(
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>KeyNameForKeyIntPairs</key>
    <dict>
        <key>a key</key>
        <integer>1</integer>
        <key>another key</key>
        <integer>2</integer>
        <key>third key</key>
        <integer>3</integer>
    </dict>
</dict>
  )V0G0N";
    
    
    b = MyDict.parse(config_test, strlen(config_test), "/"_XS8, &gXmlLiteParserTest);
gXmlLiteParserTest.printfErrorsAndWarnings();
    if ( !b ) return breakpoint(1);
    if ( gXmlLiteParserTest.getErrorsAndWarnings().size() != 0 ) return breakpoint(1);
    
    XObjArray<XmlAddKey<XmlKey, XmlInt32>> array = MyDict.keyIntPairs.valueArray();
    XString8 keyOne = array[1].key();
//    int32_t valueOne = array[1].value();

    return 0;
}

int xml_lite_tests()
{

  int ret;
  
//  XmlLiteParser xmlLiteParser;
  
  ret = documentation_test1();
  if ( ret ) return ret;
  
  ret = documentation_test2();
  if ( ret ) return ret;
  
  ret = documentation_test3();
  if ( ret ) return ret;
  
  ret = documentation_test4();
  if ( ret ) return ret;
  
  ret = documentation_test5();
  if ( ret ) return ret;
  
  ret = documentation_test6();
  if ( ret ) return ret;
  
  ret = documentation_test7();
  if ( ret ) return ret;
  

  ret = xml_lite_reapeatingdict_tests();
  if ( ret ) return ret;
  
  ret = validate_array_tests();
  if ( ret ) return ret;


  ret = validate_dict_tests();
  if ( ret ) return ret;

  
  ret = xml_integer_tests();
  if ( ret ) return ret;
  
  
  ret = move_tests();
  if ( ret ) return ret;
  
  ret = getNextTag_tests();
  if ( ret ) return ret;
  
  ret = getSimpleTag_tests();
  if ( ret ) return ret;
  
  ret = getKey_tests();
  if ( ret ) return ret;
  
  ret = skip_tests();
  if ( ret ) return ret;

  return 0;
}
