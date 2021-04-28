/*
 * AssignSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "AssignSettingsGraphics.h"
#include <Platform.h>
#include "AssignField.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

//void AssignDcfg(const XString8& label, SETTINGS_DATA::GraphicsClass& oldS, const ConfigPlistClass::Graphics_Class& newS)
//{
//  XBuffer<uint8_t> xbuffer = newS.dgetDcfg();
//  AssignField(oldS.Dcfg, sizeof(oldS.Dcfg), xbuffer.data(), xbuffer.size(), label);
//}

void AssignVBIOS_PATCH_BYTES(const XString8& label, VBIOS_PATCH& oldS, const ConfigPlistClass::Graphics_Class::Graphics_PatchVBiosBytes_Class& newS)
{
  Assign(Find);
  Assign(Replace);
}

void AssignVBIOS_PATCH_BYTES_ARRAY(const XString8& label, XObjArray<VBIOS_PATCH>& oldS, const XmlArray<ConfigPlistClass::Graphics_Class::Graphics_PatchVBiosBytes_Class>& newS)
{
  if ( sizeAssign(size()) ) {
    for ( size_t idx = 0; idx < newS.size(); ++idx )
    {
      oldS.AddReference(new (remove_ref(decltype(oldS))::type)(), true);
      AssignVBIOS_PATCH_BYTES(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void AssignEDID(const XString8& label, SETTINGS_DATA::GraphicsClass::EDIDClass& oldS, const ConfigPlistClass::Graphics_Class::Graphics_EDID_Class& newS)
{
  Assign(InjectEDID);
  Assign(CustomEDID);
  Assign(VendorEDID);
  Assign(ProductEDID);
  Assign(EdidFixHorizontalSyncPulseWidth);
  Assign(EdidFixVideoInputSignal);
}

void AssignInjectAsDict(const XString8& label, SETTINGS_DATA::GraphicsClass::InjectAsDictClass& oldS, const ConfigPlistClass::Graphics_Class::XmlInjectUnion& newS)
{
  Assign(GraphicsInjector);
  Assign(InjectIntel);
  Assign(InjectATI);
  Assign(InjectNVidia);
}

void AssignGRAPHIC_CARD(const XString8& label, SETTINGS_DATA::GraphicsClass::GRAPHIC_CARD& oldS, const ConfigPlistClass::Graphics_Class::Graphics_ATI_NVIDIA_Class& newS)
{
  Assign(Signature);
  Assign(Model);
  Assign(Id);
  Assign(SubId);
  Assign(VideoRam);
  Assign(VideoPorts);
  Assign(LoadVBios);
}

void AssignGRAPHIC_CARD_ARRAY(const XString8& label, XObjArray<SETTINGS_DATA::GraphicsClass::GRAPHIC_CARD>& oldS, const XmlArray<ConfigPlistClass::Graphics_Class::Graphics_ATI_NVIDIA_Class>& newS)
{
  if ( sizeAssign(size()) ) {
    for ( size_t idx = 0; idx < newS.size(); ++idx )
    {
      oldS.AddReference(new (remove_ref(decltype(oldS))::type)(), true);
      AssignGRAPHIC_CARD(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void AssignGraphics(const XString8& label, SETTINGS_DATA::GraphicsClass& oldS, const ConfigPlistClass::Graphics_Class& newS)
{
  Assign(PatchVBios);
  AssignVBIOS_PATCH_BYTES_ARRAY(S8Printf("%s.PatchVBiosBytes", label.c_str()), oldS.PatchVBiosBytes, newS.PatchVBiosBytesArray);
//  Assign(InjectAsBool);
  Assign(RadeonDeInit);
  Assign(LoadVBios);
  Assign(VRAM);
  Assign(RefCLK);
  Assign(FBName);
  Assign(VideoPorts);
  Assign(NvidiaGeneric);
  Assign(NvidiaNoEFI);
  Assign(NvidiaSingle);
  oldS.Dcfg.setSize(newS.dgetDcfg().size());
  memcpy(oldS.Dcfg.data(), newS.dgetDcfg().data(), oldS.Dcfg.size());
  oldS.NVCAP.setSize(newS.dgetNVCAP().size());
  memcpy(oldS.NVCAP.data(), newS.dgetNVCAP().data(), oldS.NVCAP.size());
  Assign(BootDisplay);
  Assign(DualLink);
  Assign(_IgPlatform);
  AssignEDID(S8Printf("%s.EDID", label.c_str()), oldS.EDID, newS.EDID);
  AssignInjectAsDict(S8Printf("%s.Dcfg", label.c_str()), oldS.InjectAsDict, newS.Inject);
  AssignGRAPHIC_CARD_ARRAY(S8Printf("%s.ATICardList", label.c_str()), oldS.ATICardList, newS.ATI);
  AssignGRAPHIC_CARD_ARRAY(S8Printf("%s.NVIDIACardList", label.c_str()), oldS.NVIDIACardList, newS.NVIDIA);
}
