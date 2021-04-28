# How to use the xmlLite tools to read and validate plist

### Creation of a dict

Example 1 :

```
class MyDictClass : public XmlDict
{
    using super = XmlDict;
  public:
    XmlBool aBool;
    XmlInt32 anInt32;

    XmlDictField m_fields[2] = {
        {"KeyNameForBool", aBool},
        {"KeyNameForInt32", anInt32},
    };
    virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
} MyDict;
```

1. your class must heritate from XmlDict

2. define an array to create a link between the xml key in the dict and the field.

3. override method `getFields() `to return that array.

This can read this file

```
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>KeyNameForBool</key>
    <true/>
    <key>KeyNameForInt32</key>
    <integer>13864</integer>
</dict>
```

<?xml version="1.0" encoding="UTF-8"?>

<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">

<plist version="1.0">
<dict>

NOTE : the top level dict can heritate from ConfigPlistAbstractClass instead. That'll give you parse() methods to parse from a buffer

### Values

A dict is composed of key value pair. In the previous we used XmlBool and XmlInt32. In XmlLiteSimpleType.h there is few others :

- XmlBool

- XmlStrictBool

- XmlString8

- XmlStringW

- XmlString8AllowEmpty

- XmlString8Trimed

- XmlData

- XmlUInt{8, 16 32} and XmlInt {8, 16 32}

- Of course, you can declare an XmlDict as a value in a containing dict. Example 2 :

```
class MyInsideDictClass : public XmlDict
{
    using super = XmlDict;
  public:
    XmlBool aBool;
    XmlInt32 anInt32;

    XmlDictField m_fields[2] = {
        {"KeyNameForBool", aBool},
        {"KeyNameForInt32", anInt32},
    };
    virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
};

class MyTopLevelDictClass : public ConfigPlistAbstractClass
{
    using super = XmlDict;
  public:
    MyInsideDictClass inside1;

    XmlDictField m_fields[1] = {
        {"KeyNameForInsideDict", inside1},
    };
    virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
} MyDict;
```

  This can read this file

```
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
```

- Arrays

### Validation

##### Validation of a single value

To validate and reject bad input, override the method validate. Imagine we have an integer value that is a count between 0 and 15. Example 3 : 

```
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
```

Returning false will put back the value to an undefined state.

In your dict, declare a member CountClass Count. If the value is out of range, Count will be undefined.

##### Validation of interdependent values

To check for consistency of values in a dict, you override the same validate method for your dict.

Imagine you have a dict containing a type (1 or 2) and a subType, but type 1 cannot have subtype. Type 2 may have subtype, and if there is, it must be 11 or 12. Example 4 :

```
class MyDictClass : public XmlDict
{
    using super = XmlDict;
  public:
    MyXmlType type;       // this is a subclass of XmlUInt8 that check that type is 1 or 2
    MyXmlSubType subType; // this is a subclass of XmlUInt8 that check that subtype is 11 or 12
    XmlString8 name;      // as many other field that there is in this dict

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
};
```

Returning false will put back the dict to an undefined state. Which means, in that case, the whole dict to be undefined as it wasn't at all in the XML file.

NOTE : it's possible to do the single field validation at dict level. The previous example could written as example 5 :

```
class MyDictClass : public XmlDict
{
    using super = XmlDict;
  public:
    XmlUInt8 type; // no validation except that the value is an unsigned 8 bits int 
    XmlUInt8 subType; // no validation except that the value is an unsigned 8 bits int 
    XmlString8 name;      // as many other field that there is in this dict

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
        if ( subType.isDefined() ) {
            if ( type.value() != 11 and type.value() != 12 )
            xmlLiteParser->addWarning(generateErrors, S8Printf("SubType must be 11 or 12 at '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
            return false;
        }else{
          // subtype is optional, so it's ok.
        }
      }else{
        xmlLiteParser->addWarning(generateErrors, S8Printf("Type must be 1 or 2 at '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
        // Let's think that we want to ignore this value but we syill want to keep the dict as the other field still has meaning.
        type.reset(); // we only reset this field. We don't return false because that'll undefine the whole dict
        subtype.reset(); // SubType means nothing without a Type.
      }
      return true;
    }
};
```

### Arrays

To create an array, declare an XmlArray<[any class that inherite from XmlAbstractType]> member. Example 6 :

```
class MyDictClass : public XmlDict
{
    using super = XmlDict;
  public:
    XmlArray<XmlBool> aBoolArray;

    XmlDictField m_fields[2] = {
        {"KeyNameForBoolArray", aBoolArray},
    };
    virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
};
```

This can read this file :

```
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>KeyNameForBoolArray</key>
    <array>
      <true/>
      <false/>
      <true/>
      ... {as many as ther is}
  </dict>
</dict>
```

Of course, you can create an XmlArray of a dict you created.

### Repeating dict

Until now, examples shown a dict that has a predefined set of keys. But you can have a dict that contains repeating key/value pair. The key could be any string.

For that, you have a template class XmlRepeatingDict. To create a dict that can contains a sequence of key and int32. Example 7 :

```
class MyDictClass : public ConfigPlistAbstractClass
{
    using super = XmlDict;
  public:
    XmlRepeatingDict<XmlAddKey<XmlKey, XmlInt32>> keyIntPairs;

    XmlDictField m_fields[1] = {
        {"KeyNameForKeyIntPairs", keyIntPairs},
    };
    virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
} MyDict;
```

Because a dict has to be a sequence of key and value, please note the use of the class XmlAddKey<XmlKey, XmlInt32>> that "adds" a key to a XmlInt32. You can replace XmlInt32 by any type the inherite from XmlAbstractType. That includes your own dicts and arrays.

The example above will parse a file like :

```
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
```

After the dict is parsed, you can access values this way :

```
XObjArray<XmlAddKey<XmlKey, XmlInt32>> array = MyDict.keyIntPairs.valueArray();
XString8 keyOne = array[1].key(); // == "another key"
int32_t valueOne = array[1].value(); // == 2
```

### Union

Sometimes, a value can be, for example, a bool or a string. In that case it's easy to declare. Just subclass XmlUnion :

```
class XmlBoolOrString : public XmlUnion
{
  using super = XmlUnion;
public:
  XmlBool xmlBool = XmlBool();
  XmlString8 xmlString8 = XmlString8();
  virtual const char* getDescription() override { return "bool or string"; };
  XmlUnionField m_fields[2] = { xmlBool, xmlString8 };
  virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
};
```

In this example, the value will be tried to be parsed as a bool. If it doesn't work, the next possibility is tried. Here it's xmlString8. You can create union with any kind of field you create, including dict and arrays.
