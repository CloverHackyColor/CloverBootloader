/*
 * smbios.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_SMBIOS_H_
#define PLATFORM_SMBIOS_H_

extern "C" {
#include <IndustryStandard/AppleSmBios.h>
}
//#include "../Settings/ConfigPlist/ConfigPlistClass.h"
#include "../Platform/cpu.h"
//#include "../Platform/Settings.h"

// The maximum number of RAM slots to detect
// even for 3-channels chipset X58 there are no more then 8 slots
#define MAX_RAM_SLOTS 24
static_assert(MAX_RAM_SLOTS < UINT8_MAX, "MAX_RAM_SLOTS < UINT8_MAX"); // Important

// The maximum sane frequency for a RAM module
#define MAX_RAM_FREQUENCY 5000

class RAM_SLOT_INFO {
public:
  UINT64  Slot = UINT64();
  UINT32  ModuleSize = UINT32();
  UINT32  Frequency = UINT32();
  XString8 Vendor = XString8();
  XString8 PartNo = XString8();
  XString8 SerialNo = XString8();
  UINT8   Type = UINT8();
  bool  InUse = bool();

  RAM_SLOT_INFO() {}
};
extern const RAM_SLOT_INFO nullRAM_SLOT_INFO;

class SmbiosMemoryConfigurationClass {
  public:
    UINT8         SlotCounts = UINT8();
    UINT8         UserChannels = UINT8();
    XObjArray<RAM_SLOT_INFO> _User = XObjArray<RAM_SLOT_INFO>();

    SmbiosMemoryConfigurationClass() {}

    void setEmpty() {
      SlotCounts = 0;
      UserChannels = 0;
      _User.setEmpty();
    }
  
    const RAM_SLOT_INFO& getSlotInfoForSlotID(UINT64 Slot) const {
      for ( size_t idx = 0 ; idx < _User.size() ; ++idx ) {
        if ( _User[idx].Slot == Slot ) return _User[idx];
      }
      return nullRAM_SLOT_INFO;
    }
};

class SLOT_DEVICE
{
public:
  uint8_t           Index = 0xFF;
  UINT16            SegmentGroupNum = UINT16();
  UINT8             BusNum = UINT8();
  UINT8             DevFuncNum = UINT8();
  UINT8             SlotID = UINT8();
  MISC_SLOT_TYPE    SlotType = MISC_SLOT_TYPE();
  XString8          SlotName = XString8();

  SLOT_DEVICE() {}
};
extern const SLOT_DEVICE nullSLOT_DEVICE;

/*
 * All settings from Smbios goes into this struct.
 * Goal : No globals set by getTablexxx functions
 */
class SmbiosDiscoveredSettings
{
  public:
    uint16_t SmbiosVersion            = 0;
    XString8 OEMBoardFromSmbios       = XString8();
    XString8 OEMProductFromSmbios     = XString8();
    XString8 OEMVendorFromSmbios      = XString8();
    uint8_t EnabledCores                 = 0;
    
    uint16_t RamSlotCount = 0; // this is maxed out to MAX_RAM_SLOTS

    // gCPUStructure
    UINT64                ExternalClock = 0;
    UINT32                CurrentSpeed = 0;
    UINT32                MaxSpeed = 0;

    SmbiosDiscoveredSettings() {}
};

/*
 * All settings that'll be injected goes into this struct.
 * Goal : No globals used by patchTablexxx functions
 * The method that initialises this is SmbiosFillPatchingValues()
 * Q: Why is this intersting ? Isn't it just copy and we should let smbios.cpp access globals, like gCPUStructure ?
 * A: Problems with globals, is that don't control where they are accessed from.
 *    Imagine you have a wrong information sent to Smbios.
 *    Putting a breakpoint or a log in SmbiosInjectedSettings::takeValueFrom, you immediatley know if the problem is
 *    on the Clover side (wrong info given by Clover) or on the Smbios patch side (Right info but wrong way of patching smbios table).
 *    This way, Smbios is a layer (or toolbox) independent of Clover.
 *    SmbiosInjectedSettings is a "touch point" (some say "bridge") between layer. Of course it has to have only 1 or 2, easily identifiable.
 *    SmbiosFillPatchingValues() is THE place to make some checks, gather values and be sure of what to send to the patching functions.
 *
 *    NOTE : I know it's tempting not to do it because it's a lot of copy/paste. But it's so much a time saver later to have better/simpler design...
 */
class SmbiosInjectedSettings
{
  public:
    // gCPUStructure
    UINT8                   Cores = 0;
    UINT32                  MaxSpeed = 0;
    UINT8                   Threads = 0;
    UINT64                  Features = 0;
    UINT64                  ExternalClock = 0;
    UINT32                  Model = 0;
    UINT8                   Mobile = 0;  //not for i3-i7
    UINT32                  Family = 0;
    UINT32                  Type = 0;
    XString8                BrandString = XString8();
    UINT32                  Extmodel = 0;
    UINT64                  ExtFeatures = 0;
    UINT64                  MicroCode = 0;
    UINT32                  Extfamily = 0;
    UINT32                  Stepping = 0;

    // gSettings
    XString8 BiosVendor              = XString8();
    XString8 BiosVersionUsed         = XString8();
    XString8 EfiVersionUsed          = XString8();
    XString8 ReleaseDateUsed         = XString8();
    XString8 ManufactureName         = XString8();
    XString8 ProductName             = XString8();
    XString8 SystemVersion           = XString8();
    XString8 SerialNr                = XString8();
    XString8 BoardNumber             = XString8();
    XString8 BoardManufactureName    = XString8();
    XString8 BoardVersion            = XString8();
    XString8 BoardSerialNumber       = XString8();
    XString8 LocationInChassis       = XString8();
    XString8 ChassisManufacturer     = XString8();
    XString8 ChassisAssetTag         = XString8();
    XString8 FamilyName              = XString8();
    XString8 SmUUID                  = XString8();
    bool NoRomInfo = 0;
    uint8_t EnabledCores = 0;
    bool TrustSMBIOS = 0;
    bool InjectMemoryTables = 0;
    uint8_t BoardType = 0;
    uint8_t ChassisType = 0;

    class SlotDevicesArrayClass : protected XObjArray<SLOT_DEVICE>
    {
        using super = XObjArray<SLOT_DEVICE>;
      public:
        void setEmpty() { super::setEmpty(); }
        void AddReference(SLOT_DEVICE* newElement, bool FreeIt) { super::AddReference(newElement, FreeIt); }

        const SLOT_DEVICE& getSlotForIndex(size_t Index) const {
          if ( Index >= MAX_RAM_SLOTS) {
            log_technical_bug("%s : Index >= MAX_RAM_SLOTS", __PRETTY_FUNCTION__);
          }
          for ( size_t idx = 0 ; idx < size() ; ++idx ) {
            if ( ElementAt(idx).Index == Index ) return ElementAt(idx);
          }
          return nullSLOT_DEVICE;
        }
        SLOT_DEVICE& getOrCreateSlotForIndex(size_t Index) {
          if ( Index >= MAX_RAM_SLOTS) {
            log_technical_bug("%s : Index >= MAX_RAM_SLOTS", __PRETTY_FUNCTION__);
          }
          for ( size_t idx = 0 ; idx < size() ; ++idx ) {
            if ( ElementAt(idx).Index == Index ) return ElementAt(idx);
          }
          SLOT_DEVICE* slotDevice = new SLOT_DEVICE;
          AddReference(slotDevice, true);
          return *slotDevice;
        }
        bool isSlotForIndexValid(uint8_t Index) const {
          if ( Index >= MAX_RAM_SLOTS) {
            log_technical_bug("%s : Index >= MAX_RAM_SLOTS", __PRETTY_FUNCTION__);
          }
          for ( size_t idx = 0 ; idx < size() ; ++idx ) {
            if ( ElementAt(idx).Index == Index ) return true;
          }
          return false;
        }
    } SlotDevices         = SlotDevicesArrayClass();
    
    SmbiosMemoryConfigurationClass Memory    = SmbiosMemoryConfigurationClass();

    uint64_t gPlatformFeature = 0;
    uint32_t FirmwareFeatures = 0;
    uint32_t FirmwareFeaturesMask = 0;
    uint64_t ExtendedFirmwareFeatures = 0;
    uint64_t ExtendedFirmwareFeaturesMask = 0;
    int8_t Attribute = 0;

    bool KPDELLSMBIOS = 0;

    // CPU
    uint16_t CpuType = 0;
    bool SetTable132 = 0;
    uint16_t QPI = 0;

    // from SmBios
    uint16_t RamSlotCount = 0;
    
    SmbiosInjectedSettings() {}
};

class MEM_STRUCTURE
{
public:
  UINT32        Frequency = UINT32();
  UINT32        Divider = UINT32();
  UINT8         TRC = UINT8();
  UINT8         TRP = UINT8();
  UINT8         RAS = UINT8();
  UINT8         Channels = UINT8();
  UINT8         Slots = UINT8();
  UINT8         Type = UINT8();
  UINT8         SPDInUse = UINT8();
  UINT8         SMBIOSInUse = UINT8();

  RAM_SLOT_INFO SPD[MAX_RAM_SLOTS * 4];
  RAM_SLOT_INFO SMBIOS[MAX_RAM_SLOTS * 4];

};


extern APPLE_SMBIOS_STRUCTURE_POINTER SmbiosTable;

// TODO stop using globals.
extern MEM_STRUCTURE            gRAM;
extern BOOLEAN                  gMobile;



UINTN
iStrLen(
  CONST CHAR8* String,
  UINTN  MaxLen
  );

EFI_STATUS PrepatchSmbios(SmbiosDiscoveredSettings* smbiosSettings);
void PatchSmbios(const SmbiosInjectedSettings& smbiosSettings);
void FinalizeSmbios(const SmbiosInjectedSettings& smbiosSettings);

bool getMobileFromSmbios();
XString8 getSmUUIDFromSmbios();


extern SmbiosDiscoveredSettings g_SmbiosDiscoveredSettings;
extern SmbiosInjectedSettings g_SmbiosInjectedSettings;

#endif /* PLATFORM_SMBIOS_H_ */
