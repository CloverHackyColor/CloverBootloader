#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "../cpp_foundation/XString.h"
#include "../cpp_foundation/XStringArray.h"
//#include "../cpp_foundation/unicode_conversions.h"
//#include "../Platform/plist/plist.h"

#include "../cpp_lib/XmlLiteSimpleTypes.h"
#include "../cpp_lib/XmlLiteCompositeTypes.h"
#include "../cpp_lib/XmlLiteParser.h"


static int breakpoint(int i)
{
  return i;
}


static XmlLiteParser gXmlLiteParser;





int move_tests()
{
  gXmlLiteParser.init("\n\n\n<a>\n  <b></b>\n</a>\n");
  while ( gXmlLiteParser.getLine() < 4 && gXmlLiteParser.moveForward() ) ;
  while ( gXmlLiteParser.moveBackward() );
  if ( gXmlLiteParser.getPosition().getLine() != 1 || gXmlLiteParser.getPosition().getCol() != 1 ) return 1;
  
  return 0;
}

int getNextTag_tests()
{
  const char* tag;
  size_t tagLength;
  bool isOpeningTag, isClosingTag;
  bool b;

  gXmlLiteParser.init("<key>");
  b = gXmlLiteParser.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( !b ) return breakpoint(1);
  if ( !isOpeningTag ) return breakpoint(2);
  if ( isClosingTag ) return breakpoint(3);
  if ( gXmlLiteParser.getchar() != 0 ) return breakpoint(4);

  
  gXmlLiteParser.init("</key>");
  b = gXmlLiteParser.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( !b ) return breakpoint(5);
  if ( isOpeningTag ) return breakpoint(6);
  if ( !isClosingTag ) return breakpoint(7);
  if ( gXmlLiteParser.getchar() != 0 ) return breakpoint(8);

  
  gXmlLiteParser.init("<key/>");
  b = gXmlLiteParser.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( !b ) return breakpoint(10);
  if ( !isOpeningTag ) return breakpoint(11);
  if ( isClosingTag ) return breakpoint(12);
  b = gXmlLiteParser.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( !b ) return breakpoint(13);
  if ( isOpeningTag ) return breakpoint(14);
  if ( !isClosingTag ) return breakpoint(15);

  //
  // Test xmlLiteParser.getErrorsAndWarnings()
  //
  gXmlLiteParser.init("foo1\n  foo2");
  gXmlLiteParser.moveForwardUntil(0);
  b = gXmlLiteParser.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( b ) return breakpoint(13);
  if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("line 2 col 7") ) return breakpoint(14);

  gXmlLiteParser.init("foo1\n  bar1");
  gXmlLiteParser.moveForwardUntil('b');
  b = gXmlLiteParser.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( b ) return breakpoint(13);
  if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("line 2 col 3") ) return breakpoint(14);
  
  gXmlLiteParser.init("foo1\n  </foo2/>");
  gXmlLiteParser.moveForwardUntil('<');
  b = gXmlLiteParser.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( b ) return breakpoint(13);
  if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("line 2 col 9") ) return breakpoint(14);
  
  gXmlLiteParser.init("foo1\n    </foo2");
  gXmlLiteParser.moveForwardUntil('<');
  b = gXmlLiteParser.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( b ) return breakpoint(13);
  if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("line 2 col 11") ) return breakpoint(14);
  
  gXmlLiteParser.init("foo1\n      </foo2 >");
  gXmlLiteParser.moveForwardUntil('<');
  b = gXmlLiteParser.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( b ) return breakpoint(13);
  if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("line 2 col 13") ) return breakpoint(14);

  gXmlLiteParser.init("foo1\n\n<foo2/");
  gXmlLiteParser.moveForwardUntil('/');
  gXmlLiteParser.moveForwardUntilSignificant();
  b = gXmlLiteParser.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( b ) return breakpoint(13);
  if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("line 3 col 7") ) return breakpoint(14);

  gXmlLiteParser.init("foo1\n\n  <foo2/a");
  gXmlLiteParser.moveForwardUntil('/');
  gXmlLiteParser.moveForwardUntilSignificant();
  b = gXmlLiteParser.getNextTag(&tag, &tagLength, &isOpeningTag, &isClosingTag, true);
  if ( b ) return breakpoint(13);
  if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("line 3 col 9") ) return breakpoint(14);


  return 0;
}

int getSimpleTag_tests()
{
  const char* tag;
  size_t tagLength;
  const char* value;
  size_t valueLength;
  bool b;
  
  gXmlLiteParser.init("<key>k</key><string>v</string>");
  b = gXmlLiteParser.getSimpleTag(&tag, &tagLength, &value, &valueLength, "key", true);
  if ( !b ) return breakpoint(1);
  if ( !strnIsEqual(tag, tagLength, "key") ) return breakpoint(2);
  if ( !strnIsEqual(value, valueLength, "k") ) return breakpoint(3);
  
  gXmlLiteParser.init("<key/>");
  b = gXmlLiteParser.getSimpleTag(&tag, &tagLength, &value, &valueLength, "key", true);
  if ( !b ) return breakpoint(1);
  if ( value != NULL ) return breakpoint(3);
  if ( valueLength != 0 ) return breakpoint(3);

  gXmlLiteParser.init("<key></key>");
  b = gXmlLiteParser.getSimpleTag(&tag, &tagLength, &value, &valueLength, "key", true);
  if ( !b ) return breakpoint(1);
  if ( value != NULL ) return breakpoint(3);
  if ( valueLength != 0 ) return breakpoint(3);

  gXmlLiteParser.init("<true/>");
  b = gXmlLiteParser.getSimpleTag(&tag, &tagLength, &value, &valueLength, "true", true);
  if ( !b ) return breakpoint(1);
  if ( !strnIsEqual(tag, tagLength, "true") ) return breakpoint(2);
  if ( value != NULL ) return breakpoint(3);
  if ( valueLength != 0 ) return breakpoint(3);

  gXmlLiteParser.init("<key>k<string>v</string>k</key>");
  b = gXmlLiteParser.getSimpleTag(&tag, &tagLength, &value, &valueLength, "key", true);
  if ( b ) return breakpoint(4);

  //
  // Test xmlLiteParser.getErrorsAndWarnings()
  //
  gXmlLiteParser.init("\n\n  </key>k</key>");
  gXmlLiteParser.moveForwardUntilSignificant();
  b = gXmlLiteParser.getSimpleTag(&tag, &tagLength, &value, &valueLength, "key", true);
  if ( b ) return breakpoint(4);
  if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("line 3 col 3") ) return breakpoint(14);
  
  gXmlLiteParser.init("\n\n    <foo>k</foo>");
  gXmlLiteParser.moveForwardUntilSignificant();
  b = gXmlLiteParser.getSimpleTag(&tag, &tagLength, &value, &valueLength, "key", true);
  if ( b ) return breakpoint(4);
  if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("line 3 col 5") ) return breakpoint(14);
  
  gXmlLiteParser.init("\n\n    <key></key>");
  gXmlLiteParser.moveForwardUntilSignificant();
  b = gXmlLiteParser.getSimpleTag(&tag, &tagLength, &value, &valueLength, "key", true);
  if ( !b ) return breakpoint(4);
  if ( value != NULL ) return breakpoint(3);
  if ( valueLength != 0 ) return breakpoint(3);

  gXmlLiteParser.init("\n\n    <key>v<key>");
  gXmlLiteParser.moveForwardUntilSignificant();
  b = gXmlLiteParser.getSimpleTag(&tag, &tagLength, &value, &valueLength, "key", true);
  if ( b ) return breakpoint(4);
  if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("line 3 col 11") ) return breakpoint(14);

  gXmlLiteParser.init("\n\n    <key>v</foo>");
  gXmlLiteParser.moveForwardUntilSignificant();
  b = gXmlLiteParser.getSimpleTag(&tag, &tagLength, &value, &valueLength, "key", true);
  if ( b ) return breakpoint(4);
  if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("line 3 col 11") ) return breakpoint(14);

  return 0;
}

int getKey_tests()
{
  const char* tag;
  size_t length;
  bool b;
  XmlParserPosition xmlParserPosition;

  gXmlLiteParser.init("<key></key><string>v</string>");
  b = gXmlLiteParser.getKeyTagValue(&tag, &length, &xmlParserPosition, true);
  if ( !b ) return breakpoint(3);
  if ( length != 0 ) return breakpoint(13);

  gXmlLiteParser.init("<key> </key><string>v</string>");
  b = gXmlLiteParser.getKeyTagValue(&tag, &length, &xmlParserPosition, true);
  if ( !b ) return breakpoint(3);
  if ( length != 0 ) return breakpoint(13);

  gXmlLiteParser.init("<key>a</key><string>v</string>");
  b = gXmlLiteParser.getKeyTagValue(&tag, &length, &xmlParserPosition, true);
  if ( !b ) return breakpoint(5);
  if ( !strnIsEqual(tag, length, "a") ) return breakpoint(6);

  gXmlLiteParser.init("<key> a </key><string>v</string>");
  b = gXmlLiteParser.getKeyTagValue(&tag, &length, &xmlParserPosition, true);
  if ( !b ) return breakpoint(6);
  if ( !strnIsEqual(tag, length, "a") ) return breakpoint(6);

  gXmlLiteParser.init("<key>  a  </key><string>v</string>");
  b = gXmlLiteParser.getKeyTagValue(&tag, &length, &xmlParserPosition, true);
  if ( !b ) return breakpoint(7);
  if ( !strnIsEqual(tag, length, "a") ) return breakpoint(6);

  gXmlLiteParser.init("<key>a  </key><string>v</string>");
  b = gXmlLiteParser.getKeyTagValue(&tag, &length, &xmlParserPosition, true);
  if ( !b ) return breakpoint(8);
  if ( !strnIsEqual(tag, length, "a") ) return breakpoint(6);

  gXmlLiteParser.init("<key>  a</key><string>v</string>");
  b = gXmlLiteParser.getKeyTagValue(&tag, &length, &xmlParserPosition, true);
  if ( !b ) return breakpoint(9);
  if ( !strnIsEqual(tag, length, "a") ) return breakpoint(6);

  return 0;
}

int skip_tests()
{
  bool b;

  gXmlLiteParser.init("k</key><string>v</string>");
  b = gXmlLiteParser.skipUntilClosingTag("key", strlen("key"), true);
  if ( !b ) return breakpoint(1);
  if ( !strnIsEqual(gXmlLiteParser.getcharPtr(), strlen(gXmlLiteParser.getcharPtr()), "<string>v</string>") ) return breakpoint(2);

  gXmlLiteParser.init("<string>v</string></key>foo"); // 'foo' is text after closing tag -> fail
  b = gXmlLiteParser.skipUntilClosingTag("key", strlen("key"), true);
  if ( b ) return breakpoint(1);
  if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("line 1 col 25") ) return breakpoint(14);

  gXmlLiteParser.init("<string>v</string></key><foo>"); // <foo> is the next tag -> success
  b = gXmlLiteParser.skipUntilClosingTag("key", strlen("key"), true);
  if ( !b ) return breakpoint(1);
  if ( !strnIsEqual(gXmlLiteParser.getcharPtr(), strlen(gXmlLiteParser.getcharPtr()), "<foo>") ) return breakpoint(2);
  
  gXmlLiteParser.init("<string>v</string></key>"); // end of file just after skipped tag -> success
  b = gXmlLiteParser.skipUntilClosingTag("key", strlen("key"), true);
  if ( !b ) return breakpoint(1);
  if ( gXmlLiteParser.getchar() != 0 ) return breakpoint(2);

  gXmlLiteParser.init("<string>v</string><foo1><foo11>bar11</foo11></foo1><foo2>bar2</foo2></key><meow>");
  b = gXmlLiteParser.skipUntilClosingTag("key", strlen("key"), true);
  if ( !b ) return breakpoint(1);
  if ( !strnIsEqual(gXmlLiteParser.getcharPtr(), strlen(gXmlLiteParser.getcharPtr()), "<meow>") ) return breakpoint(2);

  // Cannot have a tag containing chars AND subtag
  gXmlLiteParser.init("\n\n\nk<string>v</string></key>");
  gXmlLiteParser.moveForwardUntil('k');
  b = gXmlLiteParser.skipUntilClosingTag("key", strlen("key"), true);
  if ( b ) return breakpoint(2);
  if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("line 4 col 2") ) return breakpoint(14);

  // Cannot have a tag containing chars AND subtag
  gXmlLiteParser.init("<string>v</string>k</key>");
  b = gXmlLiteParser.skipUntilClosingTag("key", strlen("key"), true);
  if ( b ) return breakpoint(2);
  if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("line 1 col 19") ) return breakpoint(14);

  
  // Cannot have a tag containing chars AND subtag
  gXmlLiteParser.init("\n </string>v</string>k</key>");
  gXmlLiteParser.moveForwardUntil('<');
  b = gXmlLiteParser.skipNextTag(true);
  if ( b ) return breakpoint(2);
  if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(13);
  if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("line 2 col 2") ) return breakpoint(14);
  
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

    gXmlLiteParser.init(config_test);
    b = mainDict.parseFromXmlLite(&gXmlLiteParser, "/"_XS8, true);
//gXmlLiteParser.printfErrorsAndWarnings();
    if ( !b ) return breakpoint(1);
    if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(1);
    if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("Test1Bool tag") ) return breakpoint(14);

    if ( !b ) return breakpoint(1);
    gXmlLiteParser.init(config_test);
    mainDict.validate(&gXmlLiteParser, "/"_XS8, XmlParserPosition(), true);
//gXmlLiteParser.printfErrorsAndWarnings();
    if ( !b ) return breakpoint(1);
    if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(1);
    if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("ict1 tag") ) return breakpoint(14);
    
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

    gXmlLiteParser.init(config_test);
    b = mainDict.parseFromXmlLite(&gXmlLiteParser, "/"_XS8, true);
//gXmlLiteParser.printfErrorsAndWarnings();
    if ( !b ) return breakpoint(1);
    if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(1);
    if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("Expecting <true/> <false/>") ) return breakpoint(14);

    gXmlLiteParser.init(config_test);
    mainDict.validate(&gXmlLiteParser, "/"_XS8, XmlParserPosition(), true);
//gXmlLiteParser.printfErrorsAndWarnings();
    if ( !b ) return breakpoint(1);
    if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(1);
    if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("dict2 tag") ) return breakpoint(14);
    
    return 0;
}

int xml_lite_tests()
{

  int ret;
  
//  XmlLiteParser xmlLiteParser;

  
  
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
