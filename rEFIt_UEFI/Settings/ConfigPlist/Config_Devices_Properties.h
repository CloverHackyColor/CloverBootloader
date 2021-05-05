/*
 * ConfigPlist.h
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#ifndef _CONFIGPLISTCLASS_DEVICES_PROPERTIES_H_
#define _CONFIGPLISTCLASS_DEVICES_PROPERTIES_H_



//Properties could be a string containing an EFIString to inject, or a dict of key-dict pair. The dict in the key-dict pair is a key-value pair
class PropertiesUnion: public XmlUnion
{
  public:
  
  
    class Properties4DeviceClass;
    class Property : public XmlAddKey<XmlKeyDisablable, XmlPropertyValue>
    {
        using super = XmlAddKey<XmlKeyDisablable, XmlPropertyValue>;
        const Properties4DeviceClass& parent;
      private:
      public:
//        XmlPropertyValue xmlValue = XmlPropertyValue();
      public:
//        XmlCompositeField m_fields[1] = { xmlValue };
//        virtual void getFields(XmlCompositeField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
        
        Property() : parent(*new Properties4DeviceClass) { panic("BUG. define getNewInstance()"); }
        Property(const Properties4DeviceClass& _parent) : parent(_parent) {}

        uint8_t dgetBValue() const {
          if ( !parent.dgetEnabled() ) return false;
          return xmlKey().isDefined() ? xmlKey().isEnabled() : false;
        }
        const keyType::ValueType& dgetKey() const { return xmlKey().isDefined() ? xmlKey().value() : xmlKey().nullValue; };
        XBuffer<uint8_t> dgetValue() const { return isDefined() ? value() : XBuffer<uint8_t>::NullXBuffer; };
        TAG_TYPE dgetValueType() const { return isDefined() ? valueType() : kTagTypeNone; };
    };

 
  //  class PropertiesClass;
  //  // Dict for Devices/Properties/each key.
  //  // A repeating key + dict
  //  // Key could be like "AAPL,GfxYTile" or "AAPL,ig-platform-id"
  class Properties4DeviceClass : public XmlAddKey<XmlKeyDisablable, XmlRepeatingDict<Property>>
  {
      using super = XmlAddKey<XmlKeyDisablable, XmlRepeatingDict<Property>>;
    public:

    virtual Property* getNewInstance() override { return new Property(*this); }

    public:
      const keyType::ValueType& dgetLabel() const { return super::xmlKey().isDefined() ? xmlKey().value() : xmlKey().nullValue; };
      const keyType::ValueType& dgetDevicePathAsString() const { return xmlKey().isDefined() ? xmlKey().value() : xmlKey().nullValue; };
      bool dgetEnabled() const { return xmlKey().isDefined() ? xmlKey().isEnabled() : false; };

    };

protected:
  XmlString8AllowEmpty PropertiesAsString = XmlString8AllowEmpty();
public:
  XmlRepeatingDict<Properties4DeviceClass> PropertiesAsDict = XmlRepeatingDict<Properties4DeviceClass>();
  
  XmlUnionField m_fields[2] = { PropertiesAsString, PropertiesAsDict };
  virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

public:
  const decltype(PropertiesAsString)::ValueType& dgetpropertiesAsString() const { return PropertiesAsString.isDefined() ? PropertiesAsString.value() : PropertiesAsString.nullValue; };

};


#endif /* _CONFIGPLISTCLASS_DEVICES_PROPERTIES_H_ */
