//
//  SettingsUtils.h
//  CloverX64
//
//  Created by Jief on 24/04/2021.
//

#ifndef SettingsUtils_h
#define SettingsUtils_h

#include "../cpp_foundation/XObjArray.h"
#include "../cpp_lib/XmlLiteArrayTypes.h"
#include "../cpp_lib/XmlLiteDictTypes.h"

template <class SettingsClass, class ConfigPlistXmlArrayElementClass>
class XObjArrayWithTakeValueFromXmlArray: public XObjArray<SettingsClass>
{
  using super = XObjArray<SettingsClass>;
 public:
  void takeValueFrom(const XmlArray<ConfigPlistXmlArrayElementClass>& xmlArray)
  {
    if ( !xmlArray.isDefined() ) {
      super::setEmpty();
      return;
    }
    size_t idx;
    for ( idx = 0 ; idx < xmlArray.size() ; ++idx ) {
      if ( idx < super::size() ) super::ElementAt(idx).takeValueFrom(xmlArray[idx]);
      else {
        SettingsClass* s = new SettingsClass;
        s->takeValueFrom(xmlArray[idx]);
        super::AddReference(s, true);
      }
    }
    while ( idx < super::size() ) super::RemoveAtIndex(idx);
  }
};

template <class SettingsClass, class ConfigPlistXmlArrayElementClass>
class XObjArrayWithTakeValueFromXmlRepeatingDict: public XObjArray<SettingsClass>
{
  using super = XObjArray<SettingsClass>;
 public:
  void takeValueFrom(const XmlRepeatingDict<ConfigPlistXmlArrayElementClass>& xmlRepeatingDict)
  {
    if ( !xmlRepeatingDict.isDefined() ) {
      super::setEmpty();
      return;
    }
    size_t idx;
    for ( idx = 0 ; idx < xmlRepeatingDict.valueArray().size() ; ++idx ) {
      if ( idx < super::size() ) super::ElementAt(idx).takeValueFrom(xmlRepeatingDict.valueArray()[idx]);
      else {
        SettingsClass* s = new SettingsClass;
        s->takeValueFrom(xmlRepeatingDict.valueArray()[idx]);
        super::AddReference(s, true);
      }
    }
    while ( idx < super::size() ) super::RemoveAtIndex(idx);
  }
};

#endif /* SettingsUtils_h */
