/*
 * CompareSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "CompareSettingsBoot.h"
#include <Platform.h>
#include "CompareField.h"
#include "../../../../rEFIt_UEFI/Settings/ConfigPlist/ConfigPlistClass.h"

void CompareDevicesAudio(const XString8& label, const SETTINGS_DATA::DevicesClass::AudioClass& oldS, const ConfigPlistClass::DevicesClass::Devices_Audio_Class& newS)
{
  compare(ResetHDA);
  compare(HDAInjection);
  compare(HDALayoutId);
  compare(AFGLowPowerState);
}


void CompareDevicesUSB(const XString8& label, const SETTINGS_DATA::DevicesClass::USBClass& oldS, const ConfigPlistClass::DevicesClass::Devices_USB_Class& newS)
{
  compare(USBInjection);
  compare(USBFixOwnership);
  compare(InjectClockID);
  compare(HighCurrent);
  compare(NameEH00);
//  compare(NameXH00);
}

void CompareDevicesFakeID(const XString8& label, const SETTINGS_DATA::DevicesClass::FakeIDClass& oldS, const ConfigPlistClass::DevicesClass::Devices_FakeID_Class& newS)
{
  compare(FakeATI);
  compare(FakeNVidia);
  compare(FakeIntel);
  compare(FakeLAN);
  compare(FakeWIFI);
  compare(FakeSATA);
  compare(FakeXHCI);
  compare(FakeIMEI);
}

//---------------------------------------------   SimpleProperty


void CompareSimplePropertyClass(const XString8& label, const SETTINGS_DATA::DevicesClass::SimplePropertyClass& oldS, const ConfigPlistClass::DevicesClass::SimplePropertyClass_Class& newS)
{
  compare(Key);
  compare(Value);
  compare(ValueType);
  compareField(oldS.MenuItem.BValue, (uint8_t)!newS.dgetDisabled(), S8Printf("%s.MenuItem.BValue", label.c_str()));
}

//---------------------------------------------   ArbProperties


void CompareArbitraryPropertyClassCustomPropertyArray(const XString8& label, const XObjArray<SETTINGS_DATA::DevicesClass::SimplePropertyClass>& oldS, const XmlArray<ConfigPlistClass::DevicesClass::SimplePropertyClass_Class>& newS)
{
//  CompareDEV_ADDPROPERTY(label, oldS, newS);
//  compare(Device);
//  compare(Key);
//  compare(Value);
    if ( fcompare(size()) ) {
      for ( size_t idx = 0; idx < oldS.size(); ++idx )
      {
        CompareSimplePropertyClass(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
      }
    }
}

void CompareArbitraryPropertyClass(const XString8& label, const SETTINGS_DATA::DevicesClass::ArbitraryPropertyClass& oldS, const ConfigPlistClass::DevicesClass::Devices_Arbitrary_Class& newS)
{
  compare(Device);
  compare(Label);
  CompareArbitraryPropertyClassCustomPropertyArray(S8Printf("%s.CustomPropertyArray", label.c_str()), oldS.CustomPropertyArray, newS.CustomProperties);
}

void CompareArbitraryPropertyClassArray(const XString8& label, const XObjArray<SETTINGS_DATA::DevicesClass::ArbitraryPropertyClass>& oldS, const XmlArray<ConfigPlistClass::DevicesClass::Devices_Arbitrary_Class>& newS)
{
  if ( fcompare(size()) ) {
    for ( size_t idx = 0; idx < oldS.size(); ++idx )
    {
//      CompareArbitraryPropertyClass(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx].dgetDevice(), newS[idx].dgetComment(), newS[idx].CustomProperties);
      CompareArbitraryPropertyClass(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}


//---------------------------------------------   AddProperties

//void CompareProperties(const XString8& label, const SETTINGS_DATA::DevicesClass::PropertiesClass& oldS, const ConfigPlistClass::DevicesClass::PropertiesUnion& newS)
//{
//  compare(cDeviceProperties);
//  ComparePropertiesAsDict(S8Printf("%s.newProperties", label.c_str()), oldS.PropertyArray, newS.PropertiesAsDict);
//}

void CompareAddPropertyDict(const XString8& label, const SETTINGS_DATA::DevicesClass::AddPropertyClass& oldS, const ConfigPlistClass::DevicesClass::Devices_AddProperties_Dict_Class& newS)
{
  compare(Device);
  compare(Key);
  compare(Value);
  compare(ValueType);
  compareField(oldS.MenuItem.BValue, (uint8_t)!newS.dgetDisabled(), S8Printf("%s.MenuItem.BValue", label.c_str()));
}

void CompareAddPropertiesClassAddPropertyAsArray(const XString8& label, const XObjArray<SETTINGS_DATA::DevicesClass::AddPropertyClass>& oldS, const XmlArray<ConfigPlistClass::DevicesClass::Devices_AddProperties_Dict_Class>& newS)
{
  if ( fcompare(size()) ) {
    for ( size_t idx = 0; idx < oldS.size(); ++idx )
    {
      CompareAddPropertyDict(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

//---------------------------------------------   Properties

void ComparePropertiesAsDictElementPropertiesArray(const XString8& label, const SETTINGS_DATA::DevicesClass::SimplePropertyClass& oldS, const ConfigPlistClass::DevicesClass::PropertiesUnion::Property& newS)
{
  compare(Key);
  compare(Value);
  compare(ValueType);
  compareField(oldS.MenuItem.BValue, newS.dgetBValue(), S8Printf("%s.enabled", label.c_str()));
}

void ComparePropertiesAsDictElement(const XString8& label, const SETTINGS_DATA::DevicesClass::PropertiesClass::PropertyClass& oldS, const ConfigPlistClass::DevicesClass::PropertiesUnion::Properties4DeviceClass& newS)
{
  compare(Enabled);
  compare(DevicePathAsString);
  if ( compareField(oldS.propertiesArray.size(), newS.valueArray().size(), S8Printf("%s size", label.c_str()) ) ) {
    for ( size_t idx = 0; idx < oldS.propertiesArray.size(); ++idx )
    {
      ComparePropertiesAsDictElementPropertiesArray(S8Printf("%s[%zu]", label.c_str(), idx), oldS.propertiesArray[idx], newS.valueArray()[idx]);
    }
  }
}

void ComparePropertiesClass(const XString8& label, const SETTINGS_DATA::DevicesClass::PropertiesClass& oldS, const ConfigPlistClass::DevicesClass::PropertiesUnion& newS)
{
  compare(propertiesAsString);
//for(size_t idx=0;idx<newS.PropertiesAsDict.valueArray().size();++idx){
//printf("%s\n", newS.PropertiesAsDict.valueArray()[idx].key().value().c_str());
//}
  if ( !newS.isDefined() || !newS.PropertiesAsDict.isDefined() ) {
    compareField(oldS.PropertyArray.size(), (size_t)0, S8Printf("%s size0", label.c_str()) );
  }else{
    if ( compareField(oldS.PropertyArray.size(), newS.PropertiesAsDict.valueArray().size(), S8Printf("%s size", label.c_str()) ) ) {
      for ( size_t idx = 0; idx < oldS.PropertyArray.size(); ++idx )
      {
        ComparePropertiesAsDictElement(S8Printf("%s[%zu]", label.c_str(), idx), oldS.PropertyArray[idx], newS.PropertiesAsDict.valueArray()[idx]);
      }
    }
  }
}

//---------------------------------------------

void CompareDevices(const XString8& label, const SETTINGS_DATA::DevicesClass& oldS, const ConfigPlistClass::DevicesClass& newS)
{
  compare(StringInjector);
  compare(IntelMaxBacklight);
  compare(IntelBacklight);
  compare(IntelMaxValue);
  compare(LANInjection);
  compare(HDMIInjection);
  compare(NoDefaultProperties);
  compare(UseIntelHDMI);
  compare(ForceHPET);
  compare(DisableFunctions);
  compare(AirportBridgeDeviceName);
  CompareDevicesAudio(S8Printf("%s.Audio", label.c_str()), oldS.Audio, newS.Audio);
  CompareDevicesUSB(S8Printf("%s.USB", label.c_str()), oldS.USB, newS.USB);
  CompareDevicesFakeID(S8Printf("%s.FakeID", label.c_str()), oldS.FakeID, newS.FakeID);

  CompareAddPropertiesClassAddPropertyAsArray(S8Printf("%s.AddPropertyArray", label.c_str()), oldS.AddPropertyArray, newS.AddProperties);
  CompareArbitraryPropertyClassArray(S8Printf("%s.ArbitraryArray", label.c_str()), oldS.ArbitraryArray, newS.Arbitrary);
  ComparePropertiesClass(S8Printf("%s.Properties", label.c_str()), oldS.Properties, newS.Properties);

}
