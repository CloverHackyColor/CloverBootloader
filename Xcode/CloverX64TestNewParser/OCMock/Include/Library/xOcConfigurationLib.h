
typedef struct OC_KERNEL_QUIRKS {
  bool AppleCpuPmCfgLock;
  bool AppleXcpmCfgLock;
  bool AppleXcpmExtraMsrs;
  bool AppleXcpmForceBoost;
  bool DisableIoMapper;
  bool DisableLinkeditJettison;
  bool DisableRtcChecksum;
  bool DummyPowerManagement;
  bool ExternalDiskIcons;
  bool IncreasePciBarSize;
  bool LapicKernelPanic;
  bool PanicNoKextDump;
  bool PowerTimeoutKernelPanic;
  bool ThirdPartyDrives;
  bool XhciPortLimit;
} OC_KERNEL_QUIRKS;


typedef struct OC_BOOTER_QUIRKS {
  bool AvoidRuntimeDefrag;
  bool DevirtualiseMmio;
  bool DisableSingleUser;
  bool DisableVariableWrite;
  bool DiscardHibernateMap;
  bool EnableSafeModeSlide;
  bool EnableWriteUnprotector;
  bool ForceExitBootServices;
  bool ProtectMemoryRegions;
  bool ProtectSecureBoot;
  bool ProtectUefiServices;
  bool ProvideCustomSlide;
  bool ProvideMaxSlide;
  bool RebuildAppleMemoryMap;
  bool SetupVirtualMap;
  bool SignalAppleOS;
  bool SyncRuntimePermissions;
} OC_BOOTER_QUIRKS;
