/*
 * ConfigPlist.h
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#ifndef _CONFIGPLISTCLASS_QUIRKS_H_
#define _CONFIGPLISTCLASS_QUIRKS_H_

class Quirks_Class : public XmlDict {
  using super = XmlDict;

public:
  class Quirks_MmioWhitelist_Class : public XmlDict {
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

    virtual void getFields(XmlDictField **fields, size_t *nb) override {
      *fields = m_fields;
      *nb = sizeof(m_fields) / sizeof(m_fields[0]);
    };

    virtual XBool validate(XmlLiteParser *xmlLiteParser,
                           const XString8 &xmlPath,
                           const XmlParserPosition &keyPos,
                           XBool generateErrors) override {
      bool b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
      if (!Address.isDefined() || Address.value() == 0) {
        if (Enabled.isDefined() && Enabled.value())
          b = xmlLiteParser->addWarning(
              generateErrors, S8Printf("Enabled is ignored because Address is "
                                       "not defined or 0 in dict '%s:%d'",
                                       xmlPath.c_str(), keyPos.getLine()));
      }
      return b;
    }

    static const decltype(Comment)::ValueType &defaultComment;
    const decltype(Comment)::ValueType &dgetcomment() const {
      return Comment.isDefined() ? Comment.value() : defaultComment;
    };
    const decltype(Address)::ValueType &dgetaddress() const {
      return Address.isDefined() ? Address.value() : Address.nullValue;
    };
    const decltype(Enabled)::ValueType &dgetenabled() const {
      return Enabled.isDefined() ? Enabled.value() : Enabled.nullValue;
    };
  };

  // This is to mimic what's in settings. This is NOT a plist dict section. It
  // is just cosmetic.
  class OcKernelQuirks_Class {
    // const Quirks_Class& parent;
  public:
    XmlBool AppleXcpmExtraMsrs = XmlBool();
    XmlBool AppleXcpmForceBoost = XmlBool();
    XmlBool DisableIoMapper = XmlBool();
    XmlBool DisableIoMapperMapping = XmlBool();
    XmlBool DisableLinkeditJettison = XmlBool();
    XmlBool DummyPowerManagement = XmlBool();
    XmlBool ExtendBTFeatureFlags = XmlBool();
    XmlBool ExternalDiskIcons = XmlBool();
    XmlBool IncreasePciBarSize = XmlBool();
    XmlBool ForceAquantiaEthernet = XmlBool();
    XmlBool PowerTimeoutKernelPanic = XmlBool();
    XmlBool ThirdPartyDrives = XmlBool();
    XmlBool XhciPortLimit = XmlBool();
    XmlBool ProvideCurrentCpuInfo = XmlBool();

    XBool dgetAppleXcpmExtraMsrs() const {
      return AppleXcpmExtraMsrs.isDefined() ? AppleXcpmExtraMsrs.value()
                                            : AppleXcpmExtraMsrs.nullValue;
    };
    XBool dgetAppleXcpmForceBoost() const {
      return AppleXcpmForceBoost.isDefined() ? AppleXcpmForceBoost.value()
                                             : AppleXcpmForceBoost.nullValue;
    };
    XBool dgetDisableIoMapper() const {
      return DisableIoMapper.isDefined() ? DisableIoMapper.value()
                                         : DisableIoMapper.nullValue;
    };
    XBool dgetDisableIoMapperMapping() const {
      return DisableIoMapperMapping.isDefined()
                 ? DisableIoMapperMapping.value()
                 : DisableIoMapperMapping.nullValue;
    };
    XBool dgetDisableLinkeditJettison() const {
      return DisableLinkeditJettison.isDefined()
                 ? DisableLinkeditJettison.value()
                 : DisableLinkeditJettison.nullValue;
    };
    XBool dgetDummyPowerManagement() const {
      return DummyPowerManagement.isDefined() ? DummyPowerManagement.value()
                                              : DummyPowerManagement.nullValue;
    };
    XBool dgetExtendBTFeatureFlags() const {
      return ExtendBTFeatureFlags.isDefined() ? ExtendBTFeatureFlags.value()
                                              : ExtendBTFeatureFlags.nullValue;
    };
    XBool dgetExternalDiskIcons() const {
      return ExternalDiskIcons.isDefined() ? ExternalDiskIcons.value()
                                           : ExternalDiskIcons.nullValue;
    };
    XBool dgetIncreasePciBarSize() const {
      return IncreasePciBarSize.isDefined() ? IncreasePciBarSize.value()
                                            : IncreasePciBarSize.nullValue;
    };
    XBool dgetForceAquantiaEthernet() const {
      return ForceAquantiaEthernet.isDefined()
                 ? ForceAquantiaEthernet.value()
                 : ForceAquantiaEthernet.nullValue;
    };
    XBool dgetPowerTimeoutKernelPanic() const {
      return PowerTimeoutKernelPanic.isDefined()
                 ? PowerTimeoutKernelPanic.value()
                 : PowerTimeoutKernelPanic.nullValue;
    };
    XBool dgetThirdPartyDrives() const {
      return ThirdPartyDrives.isDefined() ? ThirdPartyDrives.value()
                                          : ThirdPartyDrives.nullValue;
    };
    XBool dgetXhciPortLimit() const {
      return XhciPortLimit.isDefined() ? XhciPortLimit.value()
                                       : XhciPortLimit.nullValue;
    };
    XBool dgetProvideCurrentCpuInfo() const {
      return ProvideCurrentCpuInfo.isDefined()
                 ? ProvideCurrentCpuInfo.value()
                 : ProvideCurrentCpuInfo.nullValue;
    };

    OcKernelQuirks_Class(const Quirks_Class &_parent) /*: parent(_parent)*/ {}
  };

  // This is to mimic what's in settings. This is NOT a plist dict section. It
  // is just cosmetic.
  class OcBooterQuirks_Class {
    const Quirks_Class &parent;

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
    XmlInt8 ResizeAppleGpuBars = XmlInt8();
    XmlInt8 ResizeGpuBars = XmlInt8();
    XmlBool SetupVirtualMap = XmlBool();
    XmlBool SignalAppleOS = XmlBool();
    XmlBool SyncRuntimePermissions = XmlBool();
    XmlBool ForceOcWriteFlash = XmlBool();
    XmlUInt32 TscSyncTimeout = XmlUInt32();

    XBool dgetAvoidRuntimeDefrag() const {
      return parent.isDefined() ? AvoidRuntimeDefrag.isDefined()
                                      ? AvoidRuntimeDefrag.value()
                                      : XBool(true)
                                : XBool(false);
    }; // TODO: different default value if section is not defined
    XBool dgetDevirtualiseMmio() const {
      return DevirtualiseMmio.isDefined() ? DevirtualiseMmio.value()
                                          : DevirtualiseMmio.nullValue;
    };
    XBool dgetDisableSingleUser() const {
      return DisableSingleUser.isDefined() ? DisableSingleUser.value()
                                           : DisableSingleUser.nullValue;
    };
    XBool dgetDisableVariableWrite() const {
      return DisableVariableWrite.isDefined() ? DisableVariableWrite.value()
                                              : DisableVariableWrite.nullValue;
    };
    XBool dgetDiscardHibernateMap() const {
      return DiscardHibernateMap.isDefined() ? DiscardHibernateMap.value()
                                             : DiscardHibernateMap.nullValue;
    };
    XBool dgetEnableSafeModeSlide() const {
      return parent.isDefined() ? EnableSafeModeSlide.isDefined()
                                      ? EnableSafeModeSlide.value()
                                      : XBool(true)
                                : XBool(false);
    }; // TODO: different default value if section is not defined
    XBool dgetEnableWriteUnprotector() const {
      return parent.isDefined() ? EnableWriteUnprotector.isDefined()
                                      ? EnableWriteUnprotector.value()
                                      : XBool(true)
                                : EnableWriteUnprotector.nullValue;
    }; // TODO: different default value if section is not defined
    XBool dgetForceExitBootServices() const {
      return ForceExitBootServices.isDefined()
                 ? ForceExitBootServices.value()
                 : ForceExitBootServices.nullValue;
    };
    XBool dgetProtectMemoryRegions() const {
      return ProtectMemoryRegions.isDefined() ? ProtectMemoryRegions.value()
                                              : ProtectMemoryRegions.nullValue;
    };
    XBool dgetProtectSecureBoot() const {
      return ProtectSecureBoot.isDefined() ? ProtectSecureBoot.value()
                                           : ProtectSecureBoot.nullValue;
    };
    XBool dgetProtectUefiServices() const {
      return ProtectUefiServices.isDefined() ? ProtectUefiServices.value()
                                             : ProtectUefiServices.nullValue;
    };
    XBool dgetProvideCustomSlide() const {
      return ProvideCustomSlide.isDefined() ? ProvideCustomSlide.value()
                                            : ProvideCustomSlide.nullValue;
    };
    uint8_t dgetProvideMaxSlide() const {
      return ProvideMaxSlide.isDefined() ? ProvideMaxSlide.value()
                                         : ProvideMaxSlide.nullValue;
    };
    XBool dgetRebuildAppleMemoryMap() const {
      return RebuildAppleMemoryMap.isDefined()
                 ? RebuildAppleMemoryMap.value()
                 : RebuildAppleMemoryMap.nullValue;
    };
    XBool dgetSetupVirtualMap() const {
      return parent.isDefined() ? SetupVirtualMap.isDefined()
                                      ? SetupVirtualMap.value()
                                      : XBool(true)
                                : SetupVirtualMap.nullValue;
    }; // TODO: different default value if section is not defined
    XBool dgetSignalAppleOS() const {
      return SignalAppleOS.isDefined() ? SignalAppleOS.value()
                                       : SignalAppleOS.nullValue;
    };
    XBool dgetSyncRuntimePermissions() const {
      return parent.isDefined() ? SyncRuntimePermissions.isDefined()
                                      ? SyncRuntimePermissions.value()
                                      : XBool(true)
                                : XBool(false);
    }; // TODO: different default value if section is not defined
    int8_t dgetResizeAppleGpuBars() const {
      return parent.isDefined() && ResizeAppleGpuBars.isDefined()
                 ? ResizeAppleGpuBars.value()
                 : -1;
    };
    int8_t dgetResizeGpuBars() const {
      return parent.isDefined() && ResizeGpuBars.isDefined()
                 ? ResizeGpuBars.value()
                 : -1;
    };
    XBool dgetForceOcWriteFlash() const {
      return ForceOcWriteFlash.isDefined() ? ForceOcWriteFlash.value()
                                           : XBool(false);
    };
    uint32_t dgetTscSyncTimeout() const {
      return TscSyncTimeout.isDefined() ? TscSyncTimeout.value() : 0;
    };
    OcBooterQuirks_Class(const Quirks_Class &_parent) : parent(_parent) {}
  };
  XmlArray<Quirks_MmioWhitelist_Class> MmioWhitelist =
      XmlArray<Quirks_MmioWhitelist_Class>();

protected:
  XmlBool FuzzyMatch = XmlBool();
  XmlString8AllowEmpty KernelCache = XmlString8AllowEmpty();
  //  XmlBool ProvideConsoleGopEnable = XmlBool();
public:
  OcKernelQuirks_Class OcKernelQuirks;
  OcBooterQuirks_Class OcBooterQuirks;

  XmlDictField m_fields[38] = {
      {"AvoidRuntimeDefrag", OcBooterQuirks.AvoidRuntimeDefrag},
      {"DevirtualiseMmio", OcBooterQuirks.DevirtualiseMmio},
      {"DisableSingleUser", OcBooterQuirks.DisableSingleUser},
      {"DisableVariableWrite", OcBooterQuirks.DisableVariableWrite},
      {"DiscardHibernateMap", OcBooterQuirks.DiscardHibernateMap},
      {"EnableSafeModeSlide", OcBooterQuirks.EnableSafeModeSlide},
      {"EnableWriteUnprotector", OcBooterQuirks.EnableWriteUnprotector},
      {"ForceExitBootServices", OcBooterQuirks.ForceExitBootServices},
      {"ForceOcWriteFlash", OcBooterQuirks.ForceOcWriteFlash},
      {"ProtectMemoryRegions", OcBooterQuirks.ProtectMemoryRegions},
      {"ProtectSecureBoot", OcBooterQuirks.ProtectSecureBoot},
      {"ProtectUefiServices", OcBooterQuirks.ProtectUefiServices},
      {"ProvideCustomSlide", OcBooterQuirks.ProvideCustomSlide},
      {"ProvideMaxSlide", OcBooterQuirks.ProvideMaxSlide},
      {"RebuildAppleMemoryMap", OcBooterQuirks.RebuildAppleMemoryMap},
      {"ResizeAppleGpuBars", OcBooterQuirks.ResizeAppleGpuBars},
      {"ResizeGpuBars", OcBooterQuirks.ResizeGpuBars},
      {"SetupVirtualMap", OcBooterQuirks.SetupVirtualMap},
      {"SignalAppleOS", OcBooterQuirks.SignalAppleOS},
      {"SyncRuntimePermissions", OcBooterQuirks.SyncRuntimePermissions},
      {"TscSyncTimeout", OcBooterQuirks.TscSyncTimeout},
      {"MmioWhitelist", MmioWhitelist},
      {"FuzzyMatch", FuzzyMatch},
      {"KernelCache", KernelCache},
      {"AppleXcpmExtraMsrs", OcKernelQuirks.AppleXcpmExtraMsrs},
      {"AppleXcpmForceBoost", OcKernelQuirks.AppleXcpmForceBoost},
      {"DisableIoMapper", OcKernelQuirks.DisableIoMapper},
      {"DisableIoMapperMapping", OcKernelQuirks.DisableIoMapperMapping},
      {"DisableLinkeditJettison", OcKernelQuirks.DisableLinkeditJettison},
      {"DummyPowerManagement", OcKernelQuirks.DummyPowerManagement},
      {"ExtendBTFeatureFlags", OcKernelQuirks.ExtendBTFeatureFlags},
      {"ExternalDiskIcons", OcKernelQuirks.ExternalDiskIcons},
      {"IncreasePciBarSize", OcKernelQuirks.IncreasePciBarSize},
      {"ForceAquantiaEthernet", OcKernelQuirks.ForceAquantiaEthernet},
      {"PowerTimeoutKernelPanic", OcKernelQuirks.PowerTimeoutKernelPanic},
      {"ThirdPartyDrives", OcKernelQuirks.ThirdPartyDrives},
      {"XhciPortLimit", OcKernelQuirks.XhciPortLimit},
      {"ProvideCurrentCpuInfo", OcKernelQuirks.ProvideCurrentCpuInfo},
  };

  Quirks_Class() : OcKernelQuirks(*this), OcBooterQuirks(*this) {}

  virtual void getFields(XmlDictField **fields, size_t *nb) override {
    *fields = m_fields;
    *nb = sizeof(m_fields) / sizeof(m_fields[0]);
  };

  virtual XBool validate(XmlLiteParser *xmlLiteParser, const XString8 &xmlPath,
                         const XmlParserPosition &keyPos,
                         XBool generateErrors) override {
    bool b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
    return b;
  }
  const decltype(FuzzyMatch)::ValueType &dgetFuzzyMatch() const {
    return FuzzyMatch.isDefined() ? FuzzyMatch.value() : FuzzyMatch.nullValue;
  };
  const decltype(KernelCache)::ValueType &dgetOcKernelCache() const {
    return KernelCache.isDefined() ? KernelCache.value()
                                   : KernelCache.nullValue;
  };
  //  XBool dgetProvideConsoleGop() const { return
  //  ProvideConsoleGopEnable.isDefined() ? ProvideConsoleGopEnable.value() :
  //  ProvideConsoleGopEnable.nullValue; };
  UINT32 dgetQuirksMask() const {
    UINT32 mask = 0;
    mask |= OcBooterQuirks.dgetAvoidRuntimeDefrag() ? QUIRK_DEFRAG : 0;
    mask |= OcBooterQuirks.dgetDevirtualiseMmio() ? QUIRK_MMIO : 0;
    mask |= OcBooterQuirks.dgetDisableSingleUser() ? QUIRK_SU : 0;
    mask |= OcBooterQuirks.dgetDisableVariableWrite() ? QUIRK_VAR : 0;
    mask |= OcBooterQuirks.dgetDiscardHibernateMap() ? QUIRK_HIBER : 0;
    mask |= OcBooterQuirks.dgetEnableSafeModeSlide() ? QUIRK_SAFE : 0;
    mask |= OcBooterQuirks.dgetEnableWriteUnprotector() ? QUIRK_UNPROT : 0;
    mask |= OcBooterQuirks.dgetForceExitBootServices() ? QUIRK_EXIT : 0;
    mask |= OcBooterQuirks.dgetProtectMemoryRegions() ? QUIRK_REGION : 0;
    mask |= OcBooterQuirks.dgetProtectSecureBoot() ? QUIRK_SECURE : 0;
    mask |= OcBooterQuirks.dgetProtectUefiServices() ? QUIRK_UEFI : 0;
    mask |= OcBooterQuirks.dgetProvideCustomSlide() ? QUIRK_CUSTOM : 0;
    mask |= OcBooterQuirks.dgetRebuildAppleMemoryMap() ? QUIRK_MAP : 0;
    mask |= OcBooterQuirks.dgetSetupVirtualMap() ? QUIRK_VIRT : 0;
    mask |= OcBooterQuirks.dgetSignalAppleOS() ? QUIRK_OS : 0;
    mask |= OcBooterQuirks.dgetSyncRuntimePermissions() ? QUIRK_PERM : 0;
    return mask;
  };
};

#endif /* _CONFIGPLISTCLASS_QUIRKS_H_ */
