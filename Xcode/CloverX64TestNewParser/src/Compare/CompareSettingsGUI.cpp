/*
 * CompareSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "CompareSettingsBoot.h"
#include <Platform.h>
#include "CompareField.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void CompareGUI_Mouse(const XString8& label, const SETTINGS_DATA::GUIClass::MouseClass& oldS, const ConfigPlistClass::GUI_Class::GUI_Mouse_Class& newS)
{
  compare(PointerSpeed);
  compare(PointerEnabled);
  compare(DoubleClickTime);
  compare(PointerMirror);
}

void CompareGUI_Scan(const XString8& label, const SETTINGS_DATA::GUIClass::ScanClass& oldS, const ConfigPlistClass::GUI_Class::GUI_Scan_Class& newS)
{
  compare(DisableEntryScan);
  compare(DisableToolScan);
  compare(KernelScan);
  compare(LinuxScan);
  compare(LegacyFirst);
  compare(NoLegacy);
}

void CompareCustomSubEntry(const XString8& label, const CUSTOM_LOADER_SUBENTRY_SETTINGS& oldS, const ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_SubEntry_Class& newS)
{
  compare(Disabled);
  compare(_Arguments);
  compare(_AddArguments);
  compare(_FullTitle);
  compare(_Title);
  compare(_NoCaches);
}

void CompareCustomSubEntries(const XString8& label, const XObjArray<CUSTOM_LOADER_SUBENTRY_SETTINGS>& oldS, const XmlArray<ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_SubEntry_Class>& newS)
{
  if ( fcompare(size()) ) {
    for ( size_t idx = 0; idx < oldS.size (); ++idx )
    {
      CompareCustomSubEntry(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void CompareCustomEntry(const XString8& label, const CUSTOM_LOADER_ENTRY_SETTINGS& oldS, const ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Entry_Class& newS)
{
  compare(Disabled);
  compare(ImageData);
  compare(DriveImageData);
  compare(Volume);
  compare(Path);
  compare(Arguments);
  compare(AddArguments);
  compare(FullTitle);
  compare(Settings);
  compare(Hotkey);
  compare(CommonSettings);
  compare(Hidden);
  compare(AlwaysHidden);
  compare(Type);
  compare(VolumeType);
  compare(KernelScan);
  compare(CustomLogoAsXString8);
  compare(CustomLogoAsData);
  compare(BootBgColor);
  compare(InjectKexts);
  compare(NoCaches);
  CompareCustomSubEntries(S8Printf("%s.SubEntriesSettings", label.c_str()), oldS.SubEntriesSettings, newS.SubEntries);

  compare(m_DriveImagePath);
  compare(m_Title);
  compare(CustomLogoTypeSettings);
  compare(m_ImagePath);

}

void CompareCustomEntries(const XString8& label, const XObjArray<CUSTOM_LOADER_ENTRY_SETTINGS>& oldS, const XmlArray<ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Entry_Class>& newS)
{
  if ( fcompare(size()) ) {
    for ( size_t idx = 0; idx < oldS.size (); ++idx )
    {
      CompareCustomEntry(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void CompareLegacyEntry(const XString8& label, const CUSTOM_LEGACY_ENTRY_SETTINGS& oldS, const ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Legacy_Class& newS)
{
  compare(Disabled);
  compare(ImagePath);
  compare(ImageData);
  compare(DriveImagePath);
  compare(DriveImageData);
  compare(Volume);
  compare(FullTitle);
  compare(Title);
  compare(Hotkey);
  compare(Hidden);
  compare(AlwaysHidden);
  compare(Type);
  compare(VolumeType);
}

void CompareLegacyEntries(const XString8& label, const XObjArray<CUSTOM_LEGACY_ENTRY_SETTINGS>& oldS, const XmlArray<ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Legacy_Class>& newS)
{
  if ( fcompare(size()) ) {
    for ( size_t idx = 0; idx < oldS.size (); ++idx )
    {
      CompareLegacyEntry(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void CompareToolEntry(const XString8& label, const CUSTOM_TOOL_ENTRY_SETTINGS& oldS, const ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Tool_Class& newS)
{
  compare(Disabled);
  compare(ImagePath);
  compare(ImageData);
  compare(Volume);
  compare(Path);
  compare(Arguments);
  compare(Title);
  compare(FullTitle);
  compare(Hotkey);
  compare(Hidden);
  compare(AlwaysHidden);
  compare(VolumeType);
}

void CompareToolEntries(const XString8& label, const XObjArray<CUSTOM_TOOL_ENTRY_SETTINGS>& oldS, const XmlArray<ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Tool_Class>& newS)
{
  if ( fcompare(size()) ) {
    for ( size_t idx = 0; idx < oldS.size (); ++idx )
    {
      CompareToolEntry(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void CompareGUI(const XString8& label, const SETTINGS_DATA::GUIClass& oldS, const ConfigPlistClass::GUI_Class& newS)
{
  compare(Timezone);
  compare(Theme);
  compare(EmbeddedThemeType);
  compare(PlayAsync);
  compare(CustomIcons);
  compare(TextOnly);
  compare(ShowOptimus);
  compare(ScreenResolution);
  compare(ProvideConsoleGop);
  compare(ConsoleMode);
  compare(languageCode);
  compare(Language);
  compare(KbdPrevLang);
  compare(HVHideStrings);
  CompareGUI_Scan(S8Printf("%s.Scan", label.c_str()), oldS.Scan, newS.Scan);
  CompareGUI_Mouse(S8Printf("%s.Mouse", label.c_str()), oldS.Mouse, newS.Mouse);
  CompareCustomEntries(S8Printf("%s.CustomEntriesSettings", label.c_str()), oldS.CustomEntriesSettings, newS.Custom.Entries);
  CompareLegacyEntries(S8Printf("%s.CustomLegacySettings", label.c_str()), oldS.CustomLegacySettings, newS.Custom.Legacy);
  CompareToolEntries(S8Printf("%s.CustomToolSettings", label.c_str()), oldS.CustomToolSettings, newS.Custom.Tool);
  
  fcompare(getDarkEmbedded(false));
  fcompare(getDarkEmbedded(true));

}
