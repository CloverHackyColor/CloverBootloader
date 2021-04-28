/*
 * AssignSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "AssignSettingsBoot.h"
#include <Platform.h>
#include "AssignField.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void AssignDevicesAudio(const XString8& label, SETTINGS_DATA::DevicesClass::AudioClass& oldS, const ConfigPlistClass::DevicesClass::Devices_Audio_Class& newS)
{
  Assign(ResetHDA);
  Assign(HDAInjection);
  Assign(HDALayoutId);
  Assign(AFGLowPowerState);
}



void AssignSimplePropertyClass(const XString8& label, SETTINGS_DATA::DevicesClass::SimplePropertyClass& oldS, const ConfigPlistClass::DevicesClass::SimplePropertyClass_Class& newS)
{
  Assign(Key);
  Assign(Value);
  Assign(ValueType);
  AssignField(oldS.MenuItem.BValue, !newS.dgetDisabled(), S8Printf("%s.enabled", label.c_str()));
}


void AssignDevicesUSB(const XString8& label, SETTINGS_DATA::DevicesClass::USBClass& oldS, const ConfigPlistClass::DevicesClass::Devices_USB_Class& newS)
{
  Assign(USBInjection);
  Assign(USBFixOwnership);
  Assign(InjectClockID);
  Assign(HighCurrent);
  Assign(NameEH00);
//  compare(NameXH00);
}

void AssignDevicesFakeID(const XString8& label, SETTINGS_DATA::DevicesClass::FakeIDClass& oldS, const ConfigPlistClass::DevicesClass::Devices_FakeID_Class& newS)
{
  Assign(FakeATI);
  Assign(FakeNVidia);
  Assign(FakeIntel);
  Assign(FakeLAN);
  Assign(FakeWIFI);
  Assign(FakeSATA);
  Assign(FakeXHCI);
  Assign(FakeIMEI);
}

//---------------------------------------------   ArbProperties


void AssignArbitraryPropertyClassCustomPropertyArray(const XString8& label, XObjArray<SETTINGS_DATA::DevicesClass::SimplePropertyClass>& oldS, const XmlArray<ConfigPlistClass::DevicesClass::SimplePropertyClass_Class>& newS)
{
//  AssignDEV_ADDPROPERTY(label, oldS, newS);
//  Assign(Device);
//  Assign(Key);
//  Assign(Value);
    if ( sizeAssign(size()) ) {
      for ( size_t idx = 0; idx < newS.size(); ++idx )
      {
        oldS.AddReference(new (remove_ref(decltype(oldS))::type)(), true);
        AssignSimplePropertyClass(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
      }
    }
}

void AssignArbitraryPropertyClass(const XString8& label, SETTINGS_DATA::DevicesClass::ArbitraryPropertyClass& oldS, const ConfigPlistClass::DevicesClass::Devices_Arbitrary_Class& newS)
{
  Assign(Device);
  Assign(Label);
  AssignArbitraryPropertyClassCustomPropertyArray(S8Printf("%s.CustomPropertyArray", label.c_str()), oldS.CustomPropertyArray, newS.CustomProperties);
}

void AssignArbitraryPropertyClassArray(const XString8& label, XObjArray<SETTINGS_DATA::DevicesClass::ArbitraryPropertyClass>& oldS, const XmlArray<ConfigPlistClass::DevicesClass::Devices_Arbitrary_Class>& newS)
{
  if ( sizeAssign(size()) ) {
    for ( size_t idx = 0; idx < newS.size(); ++idx )
    {
      oldS.AddReference(new (remove_ref(decltype(oldS))::type)(), true);
      AssignArbitraryPropertyClass(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}


//---------------------------------------------   AddProperties

//void AssignProperties(const XString8& label, SETTINGS_DATA::DevicesClass::PropertiesClass& oldS, const ConfigPlistClass::DevicesClass::PropertiesUnion& newS)
//{
//  Assign(cDeviceProperties);
//  AssignPropertiesAsDict(S8Printf("%s.newProperties", label.c_str()), oldS.PropertyArray, newS.PropertiesAsDict);
//}

void AssignAddPropertyDict(const XString8& label, SETTINGS_DATA::DevicesClass::AddPropertyClass& oldS, const ConfigPlistClass::DevicesClass::Devices_AddProperties_Dict_Class& newS)
{
  Assign(Device);
  Assign(Key);
  Assign(Value);
  Assign(ValueType);
  AssignField(oldS.MenuItem.BValue, !newS.dgetDisabled(), S8Printf("%s.enabled", label.c_str()));
}

void AssignAddPropertiesClassAddPropertyAsArray(const XString8& label, XObjArray<SETTINGS_DATA::DevicesClass::AddPropertyClass>& oldS, const XmlArray<ConfigPlistClass::DevicesClass::Devices_AddProperties_Dict_Class>& newS)
{
  if ( sizeAssign(size()) ) {
    for ( size_t idx = 0; idx < newS.size(); ++idx )
    {
      oldS.AddReference(new (remove_ref(decltype(oldS))::type)(), true);
      AssignAddPropertyDict(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

//---------------------------------------------   Properties

void AssignPropertiesAsDictElementPropertiesArray(const XString8& label, SETTINGS_DATA::DevicesClass::SimplePropertyClass& oldS, const ConfigPlistClass::DevicesClass::PropertiesUnion::Property& newS)
{
  Assign(Key);
  Assign(Value);
  Assign(ValueType);
  AssignField(oldS.MenuItem.BValue, newS.dgetBValue(), S8Printf("%s.enabled", label.c_str()));
}

void AssignPropertiesAsDictElement(const XString8& label, SETTINGS_DATA::DevicesClass::PropertiesClass::PropertyClass& oldS, const ConfigPlistClass::DevicesClass::PropertiesUnion::Properties4DeviceClass& newS)
{
  Assign(Enabled);
  Assign(DevicePathAsString);
//  if ( AssignField(oldS.propertiesArray.size(), newS.valueArray().size(), S8Printf("%s size", label.c_str()) ) ) {
    for ( size_t idx = 0; idx < newS.valueArray().size(); ++idx )
    {
      oldS.propertiesArray.AddReference(new (remove_ref(decltype(oldS.propertiesArray))::type)(), true);
      AssignPropertiesAsDictElementPropertiesArray(S8Printf("%s[%zu]", label.c_str(), idx), oldS.propertiesArray[idx], newS.valueArray()[idx]);
    }
//  }
}

void AssignPropertiesClass(const XString8& label, SETTINGS_DATA::DevicesClass::PropertiesClass& oldS, const ConfigPlistClass::DevicesClass::PropertiesUnion& newS)
{
  Assign(propertiesAsString);
  if ( newS.PropertiesAsDict.isDefined() ) {
    for ( size_t idx = 0; idx < newS.PropertiesAsDict.valueArray().size(); ++idx )
    {
      oldS.PropertyArray.AddReference(new (remove_ref(decltype(oldS.PropertyArray))::type)(), true);
      AssignPropertiesAsDictElement(S8Printf("%s[%zu]", label.c_str(), idx), oldS.PropertyArray[idx], newS.PropertiesAsDict.valueArray()[idx]);
    }
  }
}

//---------------------------------------------

void AssignDevices(const XString8& label, SETTINGS_DATA::DevicesClass& oldS, const ConfigPlistClass::DevicesClass& newS)
{
  Assign(StringInjector);
  Assign(IntelMaxBacklight);
  Assign(IntelBacklight);
  Assign(IntelMaxValue);
  Assign(LANInjection);
  Assign(HDMIInjection);
  Assign(NoDefaultProperties);
  Assign(UseIntelHDMI);
  Assign(ForceHPET);
  Assign(DisableFunctions);
  Assign(AirportBridgeDeviceName);
  AssignDevicesAudio(S8Printf("%s.Audio", label.c_str()), oldS.Audio, newS.Audio);
  AssignDevicesUSB(S8Printf("%s.USB", label.c_str()), oldS.USB, newS.USB);
  AssignDevicesFakeID(S8Printf("%s.FakeID", label.c_str()), oldS.FakeID, newS.FakeID);

  AssignAddPropertiesClassAddPropertyAsArray(S8Printf("%s.AddPropertyArray", label.c_str()), oldS.AddPropertyArray, newS.AddProperties);
  AssignArbitraryPropertyClassArray(S8Printf("%s.ArbitraryArray", label.c_str()), oldS.ArbitraryArray, newS.Arbitrary);
  AssignPropertiesClass(S8Printf("%s.Properties", label.c_str()), oldS.Properties, newS.Properties);

}
