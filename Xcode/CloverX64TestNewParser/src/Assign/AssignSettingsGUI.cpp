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

void AssignGUI_Mouse(const XString8& label, SETTINGS_DATA::GUIClass::MouseClass& oldS, const ConfigPlistClass::GUI_Class::GUI_Mouse_Class& newS)
{
  Assign(PointerSpeed);
  Assign(PointerEnabled);
  Assign(DoubleClickTime);
  Assign(PointerMirror);
}

void AssignGUI_Scan(const XString8& label, SETTINGS_DATA::GUIClass::ScanClass& oldS, const ConfigPlistClass::GUI_Class::GUI_Scan_Class& newS)
{
  Assign(DisableEntryScan);
  Assign(DisableToolScan);
  Assign(KernelScan);
  Assign(LinuxScan);
  Assign(LegacyFirst);
  Assign(NoLegacy);
}

void AssignCustomSubEntry(const XString8& label, CUSTOM_LOADER_SUBENTRY_SETTINGS& oldS, const ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_SubEntry_Class& newS)
{
  Assign(Disabled);
  Assign(_Arguments);
  Assign(_AddArguments);
  Assign(_FullTitle);
  Assign(_Title);
  Assign(_NoCaches);
}

void AssignCustomSubEntries(const XString8& label, XObjArray<CUSTOM_LOADER_SUBENTRY_SETTINGS>& oldS, const XmlArray<ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_SubEntry_Class>& newS)
{
  if ( sizeAssign(size()) ) {
    for ( size_t idx = 0; idx < newS.size (); ++idx )
    {
      oldS.AddReference(new (remove_ref(decltype(oldS))::type)(), true);
      AssignCustomSubEntry(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void AssignCustomEntry(const XString8& label, CUSTOM_LOADER_ENTRY_SETTINGS& oldS, const ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Entry_Class& newS)
{
  Assign(Disabled);
  Assign(ImageData);
  Assign(DriveImageData);
  Assign(Volume);
  Assign(Path);
  Assign(Arguments);
  Assign(AddArguments);
  Assign(FullTitle);
  Assign(Settings);
  Assign(Hotkey);
  Assign(CommonSettings);
  Assign(Hidden);
  Assign(AlwaysHidden);
  Assign(Type);
  Assign(VolumeType);
  Assign(KernelScan);
  Assign(CustomLogoAsXString8);
  Assign(CustomLogoAsData);
  Assign(BootBgColor);
  Assign(InjectKexts);
  Assign(NoCaches);
  AssignCustomSubEntries(S8Printf("%s.SubEntriesSettings", label.c_str()), oldS.SubEntriesSettings, newS.SubEntries);

  Assign(m_DriveImagePath);
  Assign(m_Title);
  Assign(CustomLogoTypeSettings);
  Assign(m_ImagePath);

  Assign(ForceTextMode);

}

void AssignCustomEntries(const XString8& label, XObjArray<CUSTOM_LOADER_ENTRY_SETTINGS>& oldS, const XmlArray<ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Entry_Class>& newS)
{
  if ( sizeAssign(size()) ) {
    for ( size_t idx = 0; idx < newS.size (); ++idx )
    {
      oldS.AddReference(new (remove_ref(decltype(oldS))::type)(), true);
      AssignCustomEntry(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void AssignLegacyEntry(const XString8& label, CUSTOM_LEGACY_ENTRY_SETTINGS& oldS, const ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Legacy_Class& newS)
{
  Assign(Disabled);
  Assign(ImagePath);
  Assign(ImageData);
  Assign(DriveImagePath);
  Assign(DriveImageData);
  Assign(Volume);
  Assign(FullTitle);
  Assign(Title);
  Assign(Hotkey);
  Assign(Hidden);
  Assign(AlwaysHidden);
  Assign(Type);
  Assign(VolumeType);
}

void AssignLegacyEntries(const XString8& label, XObjArray<CUSTOM_LEGACY_ENTRY_SETTINGS>& oldS, const XmlArray<ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Legacy_Class>& newS)
{
  if ( sizeAssign(size()) ) {
    for ( size_t idx = 0; idx < newS.size (); ++idx )
    {
      oldS.AddReference(new (remove_ref(decltype(oldS))::type)(), true);
      AssignLegacyEntry(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void AssignToolEntry(const XString8& label, CUSTOM_TOOL_ENTRY_SETTINGS& oldS, const ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Tool_Class& newS)
{
  Assign(Disabled);
  Assign(ImagePath);
  Assign(ImageData);
  Assign(Volume);
  Assign(Path);
  Assign(Arguments);
  Assign(Title);
  Assign(FullTitle);
  Assign(Hotkey);
  Assign(Hidden);
  Assign(AlwaysHidden);
  Assign(VolumeType);
}

void AssignToolEntries(const XString8& label, XObjArray<CUSTOM_TOOL_ENTRY_SETTINGS>& oldS, const XmlArray<ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Tool_Class>& newS)
{
  if ( sizeAssign(size()) ) {
    for ( size_t idx = 0; idx < newS.size (); ++idx )
    {
      oldS.AddReference(new (remove_ref(decltype(oldS))::type)(), true);
      AssignToolEntry(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void AssignGUI(const XString8& label, SETTINGS_DATA::GUIClass& oldS, const ConfigPlistClass::GUI_Class& newS)
{
  Assign(Timezone);
  Assign(Theme);
  Assign(EmbeddedThemeType);
  Assign(PlayAsync);
  Assign(CustomIcons);
  Assign(TextOnly);
  Assign(ShowOptimus);
  Assign(ScreenResolution);
  Assign(ProvideConsoleGop);
  Assign(ConsoleMode);
  Assign(languageCode);
  Assign(Language);
  Assign(KbdPrevLang);
  Assign(HVHideStrings);
  AssignGUI_Scan(S8Printf("%s.Scan", label.c_str()), oldS.Scan, newS.Scan);
  AssignGUI_Mouse(S8Printf("%s.Mouse", label.c_str()), oldS.Mouse, newS.Mouse);
  AssignCustomEntries(S8Printf("%s.CustomEntriesSettings", label.c_str()), oldS.CustomEntriesSettings, newS.Custom.Entries);
  AssignLegacyEntries(S8Printf("%s.CustomLegacySettings", label.c_str()), oldS.CustomLegacySettings, newS.Custom.Legacy);
  AssignToolEntries(S8Printf("%s.CustomToolSettings", label.c_str()), oldS.CustomToolSettings, newS.Custom.Tool);

}
