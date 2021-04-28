#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "../cpp_foundation/XString.h"
#include "../cpp_foundation/XStringArray.h"
//#include "../cpp_foundation/unicode_conversions.h"
//#include "../Platform/plist/plist.h"

#include "../cpp_lib/XmlLiteSimpleTypes.h"
#include "../cpp_lib/XmlLiteCompositeTypes.h"
#include "../cpp_lib/XmlLiteDictTypes.h"
#include "../cpp_lib/XmlLiteParser.h"


static int breakpoint(int i)
{
  return i;
}


static XmlLiteParser gXmlLiteParser;


static int repeatingdict_test1()
{
  bool b;

  class DictClass : public XmlRepeatingDict<XmlAddKey<XmlKeyDisablable, XmlString8>>
  {
    public:
  } dict;


  const char* config_test =
   "<dict>\r\n\
      <key>key1.1</key>\r\n\
      <string>foo1.1</string>\r\n\
      <key>key1.2</key>\r\n\
      <string>foo1.2</string>\r\n\
      <key>key1.3</key>\r\n\
      <string>foo1.3</string>\r\n\
      <key>key1.4</key>\r\n\
      <string>foo1.4</string>\r\n\
    </dict>\r\n\
  </plist>";

    gXmlLiteParser.init(config_test);
    b = dict.parseFromXmlLite(&gXmlLiteParser, "/"_XS8, true);
//gXmlLiteParser.printfErrorsAndWarnings();
for ( size_t idx = 0 ; idx < dict.valueArray().size() ; idx++ ) {
  printf("%s %s\n", dict.valueArray()[idx].key().c_str(), dict.valueArray()[idx].value().c_str());
}
    if ( !b ) return breakpoint(1);
    if ( gXmlLiteParser.getErrorsAndWarnings().size() != 0 ) return breakpoint(1);
//    if ( mainDict.arrayValue()[0].key() != "key1.1"_XS8 ) return breakpoint(14);
//    if ( mainDict.arrayValue()[0].value() != "foo1.1"_XS8 ) return breakpoint(14);
//    if ( mainDict.arrayValue()[1].key() != "key1.2"_XS8 ) return breakpoint(14);
//    if ( mainDict.arrayValue()[1].value() != "foo1.2"_XS8 ) return breakpoint(14);
//    if ( mainDict.arrayValue()[2].key() != "key1.3"_XS8 ) return breakpoint(14);
//    if ( mainDict.arrayValue()[2].value() != "foo1.3"_XS8 ) return breakpoint(14);
//    if ( mainDict.arrayValue()[3].key() != "key1.4"_XS8 ) return breakpoint(14);
//    if ( mainDict.arrayValue()[3].value() != "foo1.4"_XS8 ) return breakpoint(14);
//
//    if ( !b ) return breakpoint(1);
//    gXmlLiteParser.init(config_test);
//    mainDict.validate(&gXmlLiteParser, "/"_XS8, XmlParserPosition(), true);
////gXmlLiteParser.printfErrorsAndWarnings();
//    if ( !b ) return breakpoint(1);
//    if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(1);
//    if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("ict1 tag") ) return breakpoint(14);

  return 0;
}


int repeatingdict_test2()
{
  bool b;

  class SubDictClass : public XmlAddKey<XmlKey, XmlDict>
  {
    public:
//      using ValueType = Main2Dict_Class;

      XmlBool xmlBool = XmlBool();
      XmlString8 xmlString = XmlString8();

      XmlDictField m_fields[2] = {
        {"keybool", xmlBool},
        {"keystring", xmlString},
      };

      virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
  };


  class DictClass : public XmlRepeatingDict<SubDictClass>
  {
    public:
  } dict;


  const char* config_test =
   "<dict>\r\n\
      <key>array2.1</key>\r\n\
      <dict>\r\n\
        <key>keybool</key>\r\n\
        <true/>\r\n\
        <key>keystring</key>\r\n\
        <string>foo2.1</string>\r\n\
      </dict>\r\n\
      <key>array2.2</key>\r\n\
      <dict>\r\n\
        <key>keybool</key>\r\n\
        <false/>\r\n\
        <key>keystring</key>\r\n\
        <string>foo2.2</string>\r\n\
      </dict>\r\n\
    </dict>\r\n\
  </plist>";

    gXmlLiteParser.init(config_test);
    b = dict.parseFromXmlLite(&gXmlLiteParser, "/"_XS8, true);
gXmlLiteParser.printfErrorsAndWarnings();
for ( size_t idx = 0 ; idx < dict.valueArray().size() ; idx++ ) {
  auto item = dict.valueArray()[idx];
  printf("%s %d %s\n", dict.valueArray()[idx].key().c_str(), dict.valueArray()[idx].xmlBool.value(), dict.valueArray()[idx].xmlString.value().c_str());
}
    if ( !b ) return breakpoint(1);
    if ( gXmlLiteParser.getErrorsAndWarnings().size() != 0 ) return breakpoint(1);
    if ( dict.valueArray()[0].xmlBool.value() != true ) return breakpoint(14);
    if ( dict.valueArray()[0].xmlString.value() != "foo2.1"_XS8 ) return breakpoint(14);
    if ( dict.valueArray()[1].xmlBool.value() != false ) return breakpoint(14);
    if ( dict.valueArray()[1].xmlString.value() != "foo2.2"_XS8 ) return breakpoint(14);

//    gXmlLiteParser.init(config_test);
//    mainDict.validate(&gXmlLiteParser, "/"_XS8, XmlParserPosition(), true);
////gXmlLiteParser.printfErrorsAndWarnings();
//    if ( !b ) return breakpoint(1);
//    if ( gXmlLiteParser.getErrorsAndWarnings().size() != 1 ) return breakpoint(1);
//    if ( !gXmlLiteParser.getErrorsAndWarnings()[0].msg.contains("dict2 tag") ) return breakpoint(14);

    return 0;
}

int xml_lite_reapeatingdict_tests()
{

  int ret;
  
  ret = repeatingdict_test1();
  if ( ret ) return ret;

  ret = repeatingdict_test2();
  if ( ret ) return ret;

//  ret = repeatingdict_test3();
//  if ( ret ) return ret;


  return 0;
}
