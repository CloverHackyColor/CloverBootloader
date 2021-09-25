/*
 * ConfigPlist.h
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#ifndef _CONFIGPLISTCLASS_QUIRKS_H_
#define _CONFIGPLISTCLASS_QUIRKS_H_


class Quirks_Class : public XmlDict
{
  using super = XmlDict;
public:


  class Quirks_MmioWhitelist_Class : public XmlDict
  {
    using super = XmlDict;
  public:
    XmlString8 Comment = XmlString8();
    XmlUInt64 Address = XmlUInt64();
    XmlBool Enabled = XmlBool();

    XmlDictField m_fields[3] = {
      {"Comment", Comment},
      {"Address", Address},
      {"Enabled", Enabled},
    };

    virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
    
    virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
      if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
      bool b = true;
      if ( !Address.isDefined() || Address.value() == 0) {
        if ( Enabled.isDefined() && Enabled.value() ) b = xmlLiteParser->addWarning(generateErrors, S8Printf("Enabled is ignored because Address is not defined or 0 in dict '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
      }
      return b;
    }

    static const decltype(Comment)::ValueType& defaultComment;
    const decltype(Comment)::ValueType& dgetcomment() const { return Comment.isDefined() ? Comment.value() : defaultComment; };
    const decltype(Address)::ValueType& dgetaddress() const { return Address.isDefined() ? Address.value() : Address.nullValue; };
    const decltype(Enabled)::ValueType& dgetenabled() const { return Enabled.isDefined() ? Enabled.value() : Enabled.nullValue; };

  };




  // This is to mimic what's in settings. This is NOT a plist dict section. It is just cosmetic. TODO: remove that OC coupling in SETTINGS_DATA.
  class OcKernelQuirks_Class {
    //const Quirks_Class& parent;
    public:
      XmlBool AppleXcpmExtraMsrs = XmlBool();
      XmlBool AppleXcpmForceBoost = XmlBool();
      XmlBool DisableIoMapper = XmlBool();
      XmlBool DisableLinkeditJettison = XmlBool();
      XmlBool DummyPowerManagement = XmlBool();
      XmlBool ExtendBTFeatureFlags = XmlBool();
      XmlBool ExternalDiskIcons = XmlBool();
      XmlBool IncreasePciBarSize = XmlBool();
      XmlBool PowerTimeoutKernelPanic = XmlBool();
      XmlBool ThirdPartyDrives = XmlBool();
      XmlBool XhciPortLimit = XmlBool();
      
      bool dgetAppleXcpmExtraMsrs() const { return AppleXcpmExtraMsrs.isDefined() ? AppleXcpmExtraMsrs.value() : AppleXcpmExtraMsrs.nullValue; };
      bool dgetAppleXcpmForceBoost() const { return AppleXcpmForceBoost.isDefined() ? AppleXcpmForceBoost.value() : AppleXcpmForceBoost.nullValue; };
      bool dgetDisableIoMapper() const { return DisableIoMapper.isDefined() ? DisableIoMapper.value() : DisableIoMapper.nullValue; };
      bool dgetDisableLinkeditJettison() const { return DisableLinkeditJettison.isDefined() ? DisableLinkeditJettison.value() : DisableLinkeditJettison.nullValue; };
      bool dgetDummyPowerManagement() const { return DummyPowerManagement.isDefined() ? DummyPowerManagement.value() : DummyPowerManagement.nullValue; };
      bool dgetExtendBTFeatureFlags() const { return ExtendBTFeatureFlags.isDefined() ? ExtendBTFeatureFlags.value() : ExtendBTFeatureFlags.nullValue; };
      bool dgetExternalDiskIcons() const { return ExternalDiskIcons.isDefined() ? ExternalDiskIcons.value() : ExternalDiskIcons.nullValue; };
      bool dgetIncreasePciBarSize() const { return IncreasePciBarSize.isDefined() ? IncreasePciBarSize.value() : IncreasePciBarSize.nullValue; };
      bool dgetPowerTimeoutKernelPanic() const { return PowerTimeoutKernelPanic.isDefined() ? PowerTimeoutKernelPanic.value() : PowerTimeoutKernelPanic.nullValue; };
      bool dgetThirdPartyDrives() const { return ThirdPartyDrives.isDefined() ? ThirdPartyDrives.value() : ThirdPartyDrives.nullValue; };
      bool dgetXhciPortLimit() const { return XhciPortLimit.isDefined() ? XhciPortLimit.value() : XhciPortLimit.nullValue; };
      
      OcKernelQuirks_Class(const Quirks_Class& _parent) /*: parent(_parent)*/ {}
  };

  // This is to mimic what's in settings. This is NOT a plist dict section. It is just cosmetic. TODO: remove that OC coupling in SETTINGS_DATA.
  class OcBooterQuirks_Class {
      const Quirks_Class& parent;
    public:
      XmlBool AvoidRuntimeDefrag = XmlBool();
      XmlBool DevirtualiseMmio = XmlBool();
      XmlBool DisableSingleUser = XmlBool();
      XmlBool DisableVariableWrite = XmlBool();
      XmlBool DiscardHibernateMap = XmlBool();
      XmlBool EnableSafeModeSlide = XmlBool();
      XmlBool EnableWriteUnprotector = XmlBool();
      XmlBool ForceExitBootServices = XmlBool();
      XmlBool ProtectMemoryRegions = XmlBool();
      XmlBool ProtectSecureBoot = XmlBool();
      XmlBool ProtectUefiServices = XmlBool();
      XmlBool ProvideCustomSlide = XmlBool();
      XmlUInt8 ProvideMaxSlide = XmlUInt8();
      XmlBool RebuildAppleMemoryMap = XmlBool();
      XmlBool SetupVirtualMap = XmlBool();
      XmlBool SignalAppleOS = XmlBool();
      XmlBool SyncRuntimePermissions = XmlBool();
      
      bool dgetAvoidRuntimeDefrag() const { return parent.isDefined() ? AvoidRuntimeDefrag.isDefined() ? AvoidRuntimeDefrag.value() : true : false; }; // TODO: different default value if section is not defined
      bool dgetDevirtualiseMmio() const { return DevirtualiseMmio.isDefined() ? DevirtualiseMmio.value() : DevirtualiseMmio.nullValue; };
      bool dgetDisableSingleUser() const { return DisableSingleUser.isDefined() ? DisableSingleUser.value() : DisableSingleUser.nullValue; };
      bool dgetDisableVariableWrite() const { return DisableVariableWrite.isDefined() ? DisableVariableWrite.value() : DisableVariableWrite.nullValue; };
      bool dgetDiscardHibernateMap() const { return DiscardHibernateMap.isDefined() ? DiscardHibernateMap.value() : DiscardHibernateMap.nullValue; };
      bool dgetEnableSafeModeSlide() const { return parent.isDefined() ? EnableSafeModeSlide.isDefined() ? EnableSafeModeSlide.value() : true : false; }; // TODO: different default value if section is not defined
      bool dgetEnableWriteUnprotector() const { return parent.isDefined() ? EnableWriteUnprotector.isDefined() ? EnableWriteUnprotector.value() : true : EnableWriteUnprotector.nullValue; }; // TODO: different default value if section is not defined
      bool dgetForceExitBootServices() const { return ForceExitBootServices.isDefined() ? ForceExitBootServices.value() : ForceExitBootServices.nullValue; };
      bool dgetProtectMemoryRegions() const { return ProtectMemoryRegions.isDefined() ? ProtectMemoryRegions.value() : ProtectMemoryRegions.nullValue; };
      bool dgetProtectSecureBoot() const { return ProtectSecureBoot.isDefined() ? ProtectSecureBoot.value() : ProtectSecureBoot.nullValue; };
      bool dgetProtectUefiServices() const { return ProtectUefiServices.isDefined() ? ProtectUefiServices.value() : ProtectUefiServices.nullValue; };
      bool dgetProvideCustomSlide() const { return ProvideCustomSlide.isDefined() ? ProvideCustomSlide.value() : ProvideCustomSlide.nullValue; };
      uint8_t dgetProvideMaxSlide() const { return ProvideMaxSlide.isDefined() ? ProvideMaxSlide.value() : ProvideMaxSlide.nullValue; };
      bool dgetRebuildAppleMemoryMap() const { return RebuildAppleMemoryMap.isDefined() ? RebuildAppleMemoryMap.value() : RebuildAppleMemoryMap.nullValue; };
      bool dgetSetupVirtualMap() const { return parent.isDefined() ? SetupVirtualMap.isDefined() ? SetupVirtualMap.value() : true : SetupVirtualMap.nullValue; }; // TODO: different default value if section is not defined
      bool dgetSignalAppleOS() const { return SignalAppleOS.isDefined() ? SignalAppleOS.value() : SignalAppleOS.nullValue; };
      bool dgetSyncRuntimePermissions() const { return parent.isDefined() ? SyncRuntimePermissions.isDefined() ? SyncRuntimePermissions.value() : true : false; }; // TODO: different default value if section is not defined

      OcBooterQuirks_Class(const Quirks_Class& _parent) : parent(_parent) {}
  };
  XmlArray<Quirks_MmioWhitelist_Class> MmioWhitelist = XmlArray<Quirks_MmioWhitelist_Class>();
protected:
  XmlBool FuzzyMatch = XmlBool();
  XmlString8AllowEmpty KernelCache = XmlString8AllowEmpty();
//  XmlBool ProvideConsoleGopEnable = XmlBool();
public:
  OcKernelQuirks_Class OcKernelQuirks;
  OcBooterQuirks_Class OcBooterQuirks;

  XmlDictField m_fields[31] = {
    {"AvoidRuntimeDefrag", OcBooterQuirks.AvoidRuntimeDefrag},
    {"DevirtualiseMmio", OcBooterQuirks.DevirtualiseMmio},
    {"DisableSingleUser", OcBooterQuirks.DisableSingleUser},
    {"DisableVariableWrite", OcBooterQuirks.DisableVariableWrite},
    {"DiscardHibernateMap", OcBooterQuirks.DiscardHibernateMap},
    {"EnableSafeModeSlide", OcBooterQuirks.EnableSafeModeSlide},
    {"EnableWriteUnprotector", OcBooterQuirks.EnableWriteUnprotector},
    {"ForceExitBootServices", OcBooterQuirks.ForceExitBootServices},
    {"ProtectMemoryRegions", OcBooterQuirks.ProtectMemoryRegions},
    {"ProtectSecureBoot", OcBooterQuirks.ProtectSecureBoot},
    {"ProtectUefiServices", OcBooterQuirks.ProtectUefiServices},
    {"ProvideCustomSlide", OcBooterQuirks.ProvideCustomSlide},
    {"ProvideMaxSlide", OcBooterQuirks.ProvideMaxSlide},
    {"RebuildAppleMemoryMap", OcBooterQuirks.RebuildAppleMemoryMap},
    {"SetupVirtualMap", OcBooterQuirks.SetupVirtualMap},
    {"SignalAppleOS", OcBooterQuirks.SignalAppleOS},
    {"SyncRuntimePermissions", OcBooterQuirks.SyncRuntimePermissions},
    {"MmioWhitelist", MmioWhitelist},
    {"FuzzyMatch", FuzzyMatch},
    {"KernelCache", KernelCache},
    {"AppleXcpmExtraMsrs", OcKernelQuirks.AppleXcpmExtraMsrs},
    {"AppleXcpmForceBoost", OcKernelQuirks.AppleXcpmForceBoost},
    {"DisableIoMapper", OcKernelQuirks.DisableIoMapper},
    {"DisableLinkeditJettison", OcKernelQuirks.DisableLinkeditJettison},
    {"DummyPowerManagement", OcKernelQuirks.DummyPowerManagement},
    {"ExtendBTFeatureFlags", OcKernelQuirks.ExtendBTFeatureFlags},
    {"ExternalDiskIcons", OcKernelQuirks.ExternalDiskIcons},
    {"IncreasePciBarSize", OcKernelQuirks.IncreasePciBarSize},
    {"PowerTimeoutKernelPanic", OcKernelQuirks.PowerTimeoutKernelPanic},
    {"ThirdPartyDrives", OcKernelQuirks.ThirdPartyDrives},
    {"XhciPortLimit", OcKernelQuirks.XhciPortLimit},
  };

  Quirks_Class() : OcKernelQuirks(*this), OcBooterQuirks(*this) {}
  
  virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
  
  virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
    if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
    bool b = true;
    return b;
  }
  const decltype(FuzzyMatch)::ValueType& dgetFuzzyMatch() const { return FuzzyMatch.isDefined() ? FuzzyMatch.value() : FuzzyMatch.nullValue; };
  const decltype(KernelCache)::ValueType& dgetOcKernelCache() const { return KernelCache.isDefined() ? KernelCache.value() : KernelCache.nullValue; };
//  bool dgetProvideConsoleGop() const { return ProvideConsoleGopEnable.isDefined() ? ProvideConsoleGopEnable.value() : ProvideConsoleGopEnable.nullValue; };
  UINT32 dgetQuirksMask() const {
    UINT32 mask = 0;
    mask  |= OcBooterQuirks.dgetAvoidRuntimeDefrag() ? QUIRK_DEFRAG:0;
    mask  |= OcBooterQuirks.dgetDevirtualiseMmio() ? QUIRK_MMIO:0;
    mask  |= OcBooterQuirks.dgetDisableSingleUser() ? QUIRK_SU:0;
    mask  |= OcBooterQuirks.dgetDisableVariableWrite() ? QUIRK_VAR:0;
    mask  |= OcBooterQuirks.dgetDiscardHibernateMap() ? QUIRK_HIBER:0;
    mask  |= OcBooterQuirks.dgetEnableSafeModeSlide() ? QUIRK_SAFE:0;
    mask  |= OcBooterQuirks.dgetEnableWriteUnprotector() ? QUIRK_UNPROT:0;
    mask  |= OcBooterQuirks.dgetForceExitBootServices() ? QUIRK_EXIT:0;
    mask  |= OcBooterQuirks.dgetProtectMemoryRegions() ? QUIRK_REGION:0;
    mask  |= OcBooterQuirks.dgetProtectSecureBoot() ? QUIRK_SECURE:0;
    mask  |= OcBooterQuirks.dgetProtectUefiServices() ? QUIRK_UEFI:0;
    mask  |= OcBooterQuirks.dgetProvideCustomSlide() ? QUIRK_CUSTOM:0;
    mask  |= OcBooterQuirks.dgetRebuildAppleMemoryMap() ? QUIRK_MAP:0;
    mask  |= OcBooterQuirks.dgetSetupVirtualMap() ? QUIRK_VIRT:0;
    mask  |= OcBooterQuirks.dgetSignalAppleOS() ? QUIRK_OS:0;
    mask  |= OcBooterQuirks.dgetSyncRuntimePermissions() ? QUIRK_PERM:0;
    return mask;
  };

};


#endif /* _CONFIGPLISTCLASS_QUIRKS_H_ */
