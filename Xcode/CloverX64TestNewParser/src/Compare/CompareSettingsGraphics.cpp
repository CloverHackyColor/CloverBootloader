/*
 * CompareSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "CompareSettingsGraphics.h"
#include <Platform.h>
#include "CompareField.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

//void CompareDcfg(const XString8& label, const SETTINGS_DATA::GraphicsClass& oldS, const ConfigPlistClass::Graphics_Class& newS)
//{
//  XBuffer<uint8_t> xbuffer = newS.dgetDcfg();
//  compareField(oldS.Dcfg, sizeof(oldS.Dcfg), xbuffer.data(), xbuffer.size(), label);
//}

void CompareVBIOS_PATCH_BYTES(const XString8& label, const VBIOS_PATCH& oldS, const ConfigPlistClass::Graphics_Class::Graphics_PatchVBiosBytes_Class& newS)
{
  compare(Find);
  compare(Replace);
}

void CompareVBIOS_PATCH_BYTES_ARRAY(const XString8& label, const XObjArray<VBIOS_PATCH>& oldS, const XmlArray<ConfigPlistClass::Graphics_Class::Graphics_PatchVBiosBytes_Class>& newS)
{
  if ( fcompare(size()) ) {
    for ( size_t idx = 0; idx < oldS.size(); ++idx )
    {
      CompareVBIOS_PATCH_BYTES(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void CompareEDID(const XString8& label, const SETTINGS_DATA::GraphicsClass::EDIDClass& oldS, const ConfigPlistClass::Graphics_Class::Graphics_EDID_Class& newS)
{
  compare(InjectEDID);
  compare(CustomEDID);
  compare(VendorEDID);
  compare(ProductEDID);
  compare(EdidFixHorizontalSyncPulseWidth);
  compare(EdidFixVideoInputSignal);
}

void CompareInjectAsDict(const XString8& label, const SETTINGS_DATA::GraphicsClass::InjectAsDictClass& oldS, const ConfigPlistClass::Graphics_Class::XmlInjectUnion& newS)
{
  compare(GraphicsInjector);
  compare(InjectIntel);
  compare(InjectATI);
  compare(InjectNVidia);
}

void CompareGRAPHIC_CARD(const XString8& label, const SETTINGS_DATA::GraphicsClass::GRAPHIC_CARD& oldS, const ConfigPlistClass::Graphics_Class::Graphics_ATI_NVIDIA_Class& newS)
{
  compare(Signature);
  compare(Model);
  compare(Id);
  compare(SubId);
  compare(VideoRam);
  compare(VideoPorts);
  compare(LoadVBios);
}

void CompareGRAPHIC_CARD_ARRAY(const XString8& label, const XObjArray<SETTINGS_DATA::GraphicsClass::GRAPHIC_CARD>& oldS, const XmlArray<ConfigPlistClass::Graphics_Class::Graphics_ATI_NVIDIA_Class>& newS)
{
  if ( fcompare(size()) ) {
    for ( size_t idx = 0; idx < oldS.size(); ++idx )
    {
      CompareGRAPHIC_CARD(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void CompareGraphics(const XString8& label, const SETTINGS_DATA::GraphicsClass& oldS, const ConfigPlistClass::Graphics_Class& newS)
{
  compare(PatchVBios);
  CompareVBIOS_PATCH_BYTES_ARRAY(S8Printf("%s.PatchVBiosBytes", label.c_str()), oldS.PatchVBiosBytes, newS.PatchVBiosBytesArray);
//  compare(InjectAsBool);
  compare(RadeonDeInit);
  compare(LoadVBios);
  compare(VRAM);
  compare(RefCLK);
  compare(FBName);
  compare(VideoPorts);
  compare(NvidiaGeneric);
  compare(NvidiaNoEFI);
  compare(NvidiaSingle);
  compareField(oldS.Dcfg, oldS.Dcfg.size(), newS.dgetDcfg().data(), newS.dgetDcfg().size(), S8Printf("%s.Dcfg", label.c_str()));
  compareField(oldS.NVCAP, oldS.NVCAP.size(), newS.dgetNVCAP().data(), newS.dgetNVCAP().size(), S8Printf("%s.NVCAP", label.c_str()));
  compare(BootDisplay);
  compare(DualLink);
  compare(_IgPlatform);
  CompareEDID(S8Printf("%s.EDID", label.c_str()), oldS.EDID, newS.EDID);
  CompareInjectAsDict(S8Printf("%s.Dcfg", label.c_str()), oldS.InjectAsDict, newS.Inject);
  CompareGRAPHIC_CARD_ARRAY(S8Printf("%s.ATICardList", label.c_str()), oldS.ATICardList, newS.ATI);
  CompareGRAPHIC_CARD_ARRAY(S8Printf("%s.NVIDIACardList", label.c_str()), oldS.NVIDIACardList, newS.NVIDIA);
}
