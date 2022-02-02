/**
 platformdata.c
 **/

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "nvidia.h"
#include "smbios.h"
#include "cpu.h"
#include "Nvram.h"
#include "guid.h"

/* Machine Default Data */


#ifndef DEBUG_PLATFORMDATA
#ifndef DEBUG_ALL
#define DEBUG_PLATFORMDATA 1
#else
#define DEBUG_PLATFORMDATA DEBUG_ALL
#endif
#endif

#if DEBUG_PLATFORMDATA==0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_PLATFORMDATA, __VA_ARGS__)
#endif

// All SMBIOS data were updated by Sherlocks, PMheart.
// FredWst supported SmcExtract.

// Refactored to single data structure by RehabMan

//--------------------------
/* AppleGraphicsDevicePolicy.kext in 10.14.6 contains follow board-id to choose from graphics config
 none:
 Mac-00BE6ED71E35EB86 iMac13,1
 Mac-27ADBB7B4CEE8E61 iMac14,2  GTX775M devID=119d
 Mac-4B7AC7E43945597E MacBookPro9,1 HD4000+GT650M devID=fd5 display connected to nvidia
 Mac-77EB7D7DAF985301 iMac14,3
 Mac-C3EC7CD22292981F MacBookPro10,1 HD4000 + Kepler
 Mac-C9CF552659EA9913
 Mac-FC02E91DDD3FA6A4 iMac13,2  GTX675MX devID=11a2
 default others
 
 GFX1 only
 Mac-F60DEB81FF30ACF6 MacPro6,1
 
 GFX0 only
 Mac-031B6874CF7F642A iMac14,1  Intel Iris Pro devID=0d22
 Mac-42FD25EABCABB274 iMac15,1  Radeon R9 M290X == R9 270X devID=6810
 Mac-65CE76090165799A iMac17,1
 Mac-81E3E92DD6088272 iMac14,4
 Mac-B809C3757DA9BB8D iMac17,1
 Mac-DB15BD556843C820 iMac17,1  HD530(no FB) + Radeon HD7850 devID=6819
 Mac-FA842E06C61E91C5 iMac15,1
 
 GFX0+IGPU
 Mac-63001698E7A34814 iMac19,2 Vega
 Mac-77F17D7DA9285301 iMac18,2 Radeon Pro 555  devID=67ef
 Mac-AA95B1DDAB278B95 iMac19,1 Radeon Pro 570X devID=67df
 Mac-BE088AF8C5EB4FA2 iMac18,3 Radeon Pro 575 == RX480/580 devID=67df
 Mac-AF89B6D9451A490B (Monterey) iMac20,2 RX5700XT
 Mac-CFF7D910A743CAAF (Monterey) iMac20,1
 
 GFX0+IGPU+display
 Mac-7BA5B2D9E42DDD94 iMacPro1,1
 
 */
// for HWTarget recommended values for T2 models (by Gradou)
/*
MacBookPro 15,1 (J680AP) 15,2 (J132AP) 15,3 (J780AP) & 15,4 (J213AP)
MacBookPro16,1 (J152fAP) 16,3 (J223AP) & 16,4 (J215AP
MacBookPro16,2 (J214kAP)  //small k
MacBookAir8,1 (J140kAP) & 8,2 (J140aAP)
MacBookAir9,1 (J230kAP)
Macmini8,1 (J174AP)
iMac20,1 (J185AP) & 20,2 (J185fAP)
iMacPro1,1 (J137AP)
MacPro7,1 (J160AP)
*/
//--------------------------


constexpr PLATFORMDATA ApplePlatformDataArrayClass::m_ApplePlatformDataArrayClass[];

// this methods does nothing. It's a trick so static_assert can access private members
constexpr bool ApplePlatformDataArrayClass::asserts()
{
  // Check at compile time that all model in MacModel enum has a platformdata entry in m_ApplePlatformDataArrayClass
  #define DEFINE_ENUM(a, b) static_assert(ApplePlatformDataArrayClass::hasPlatformData(0, a), "Mac model '" b "' doesn't have platformdata");
  #include "PlatformdataModels.h"
  #undef DEFINE_ENUM
  return true; // we don't care about return value.
}

const PLATFORMDATA& ApplePlatformDataArrayClass::operator [] (MacModel m)
{
  if ( m >= MaxMacModel ) {
    log_technical_bug("ApplePlatformDataArrayClass : m >= MaxMacModel");
    return ApplePlatformDataArrayClass::m_ApplePlatformDataArrayClass[getDefaultModel()];
  }
  for ( size_t idx = 0 ; idx < sizeof(m_ApplePlatformDataArrayClass)/sizeof(m_ApplePlatformDataArrayClass[0]) ; idx++ ) {
    if ( m_ApplePlatformDataArrayClass[idx].model == m ) return m_ApplePlatformDataArrayClass[idx];
  }
  // The static asserts make sure at compile time that all MacModel has a platformdata entry.
  // Therefore, this cannot happen if static asserts were not removed.
  log_technical_bug("ApplePlatformDataArrayClass : Mac model '%s' doesn't have platformdata entry", MachineModelName[m].c_str());
  return m_ApplePlatformDataArrayClass[getDefaultModel()];
}

ApplePlatformDataArrayClass ApplePlatformDataArray;


//const PLATFORMDATA& ApplePlatformDataArrayClass::operator [] (size_t idx)
//{
//  if ( idx >= sizeof(m_ApplePlatformDataArrayClass)/sizeof(m_ApplePlatformDataArrayClass[0]) ) {
//    panic("(int)m >= sizeof(m_ApplePlatformDataArrayClass)/sizeof(m_ApplePlatformDataArrayClass[0])");
//  }
//  return m_ApplePlatformDataArrayClass[idx];
//}


// Firmware info for 10.13+
// by Sherlocks
uint32_t GetFwFeatures(MacModel Model)
{
  // Firmware info for 10.13+
  // by Sherlocks
  // FirmwareFeatures
  switch ( Model )
    {
    // Verified list from Firmware
    case MacBookPro91:
    case MacBookPro92:
      return 0xC00DE137;
      break;
    case MacBookAir41:
    case MacBookAir42:
    case MacMini51:
    case MacMini52:
    case MacMini53:
      return 0xD00DE137;
      break;
    case MacBookPro101:
    case MacBookPro102:
    case MacBookAir51:
    case MacBookAir52:
    case MacMini61:
    case MacMini62:
    case iMac131:
    case iMac132:
    case iMac133:
      return 0xE00DE137;
      break;
    case MacMini81:
      return 0xFD8FF467;
      break;
    case MacBookAir61:
    case MacBookAir62:
    case iMac141:
    case iMac142:
    case iMac143:
      return 0xE00FE137;
      break;
    case MacBookPro111:
    case MacBookPro112:
    case MacBookPro113:
    case MacBookPro114:
    case MacBookPro115:
      return 0xE80FE137;
      break;
    case iMac144:
      return 0xF00FE177;
      break;
    case iMac151:
      return 0xF80FE177;
      break;
    case MacBookPro131:
    case MacBookPro132:
    case MacBookPro141:
    case MacBookPro142:
    case iMac171:
    case iMac181:
    case iMac182:
    case iMac183:
      return 0xFC0FE177;
      break;
    case MacBook91:
    case MacBook101:
    case MacBookPro133:
    case MacBookPro143:
      return 0xFC0FE17F;
      break;
    case iMacPro11:
      return 0xFD8FF53F;
      break;
    case MacBookAir91:
      return 0xFD8FF42F;
      break;
    case iMac191:
    case iMac192:
    case iMac201:
    case iMac202:
      return 0xFD8FF577;
      break;
    case MacBookPro162:
    case MacBookPro163:
    case MacBookPro164:
      return 0xFDAFF067;
      break;
      // Verified list from Users
    case MacBookAir31:
    case MacBookAir32:
    case MacMini41:
      return 0xD00DE137;
      break;
    case MacBookAir71:
    case MacBookAir72:
      return 0xE00FE137;
      break;
    case iMac101:
    case iMac111:
    case iMac112:
    case iMac113:
    case iMac121:
    case iMac122:
    case MacMini71:
      return 0xE00DE137;
      break;
    case MacPro51:
      return 0xE80FE137;
      break;
    case MacPro61:
      return 0xE80FE177;
      break;
    case MacPro71:
      return 0xFD8FF53F;
      break;
    case MacBookPro61:
    case MacBookPro62:
    case MacBookPro71:
    case MacBookPro81:
    case MacBookPro82:
    case MacBookPro83:
      return 0xC00DE137;
      break;
    case MacBookPro121:
    case MacBookPro151:
    case MacBookPro152:
    case MacBookPro153:
    case MacBookPro154:
    case MacBookPro161:
    case MacBookAir81:
    case MacBookAir82:
    case iMac161:
    case iMac162:
      return 0xFC0FE137;
      break;
    case MacBook61:
    case MacBook71:
    case MacBook81:
      return 0xFC0FE13F;
      break;
    default:
      return 0xE907F537; //unknown - use oem SMBIOS value to be default
      break;
    }
}


uint64_t GetExtFwFeatures(MacModel Model)
{
  // (Extended)FirmwareFeatures for 12+
  switch ( Model )
  {
  	case MacBookPro114:
  	case MacBookPro115:
    case MacBookPro131:
    case MacBookPro132:
    case MacBookPro141:
    case MacBookPro142:
    case iMac171:
    case iMac181:
    case iMac182:
    case iMac183:
    case iMac191:
    case iMac192:
    case iMac201:
    case iMac202:
    case iMacPro11:
      return 0x8FC0FE177ull;
      break;
    case MacBook91:
    case MacBook101:
    case MacBookPro133:
    case MacBookPro143:
      return 0x8FC0FE17Eull;
      break;
    case MacBookPro121:
    case MacBookPro151:
    case MacBookPro152:
    case MacBookPro153:
    case MacBookPro154:
    case MacBookPro161:
    case MacBookAir81:
    case MacBookAir82:
    case MacBookAir91:
    case MacBookPro162:
    case MacBookPro163:
    case MacBookPro164:
    case iMac161:
    case iMac162:
    case MacMini71:
    case MacMini81:
      return 0x8FC0FE137ull;
      break;
    case MacBook81:
      return 0x8FC0FE13Full;
      break;
    case MacPro61:
      return 0x8E80FE177ull;
      break;
    case MacPro71:
      return 0x8FD8FF53Full;
      break;

    default:
      return (uint64_t)GetFwFeatures(Model); //unknown - use oem SMBIOS value to be default
      break;
  }
}

uint64_t GetExtFwFeaturesMask(MacModel Model)
{
  return (uint64_t)GetFwFeaturesMaskFromModel(Model) + 0xFF00000000ull;
}

XBool GetMobile(MacModel Model)
{
  // Mobile: the battery tab in Energy Saver
  switch ( Model )
    {
    case MacBook11:
    case MacBook21:
    case MacBook31:
    case MacBook41:
    case MacBook51:
    case MacBook52:
    case MacBook61:
    case MacBook71:
    case MacBook81:
    case MacBook91:
    case MacBook101:
    case MacBookPro11:
    case MacBookPro12:
    case MacBookPro21:
    case MacBookPro22:
    case MacBookPro31:
    case MacBookPro41:
    case MacBookPro51:
    case MacBookPro52:
    case MacBookPro53:
    case MacBookPro54:
    case MacBookPro55:
    case MacBookPro61:
    case MacBookPro62:
    case MacBookPro71:
    case MacBookPro81:
    case MacBookPro82:
    case MacBookPro83:
    case MacBookPro91:
    case MacBookPro92:
    case MacBookPro101:
    case MacBookPro102:
    case MacBookPro111:
    case MacBookPro112:
    case MacBookPro113:
    case MacBookPro114:
    case MacBookPro115:
    case MacBookPro121:
    case MacBookPro131:
    case MacBookPro132:
    case MacBookPro133:
    case MacBookPro141:
    case MacBookPro142:
    case MacBookPro143:
    case MacBookPro151:
    case MacBookPro152:
    case MacBookPro153:
    case MacBookPro154:
    case MacBookPro161:
    case MacBookPro162:
    case MacBookPro163:
    case MacBookPro164:
    case MacBookAir11:
    case MacBookAir21:
    case MacBookAir31:
    case MacBookAir32:
    case MacBookAir41:
    case MacBookAir42:
    case MacBookAir51:
    case MacBookAir52:
    case MacBookAir61:
    case MacBookAir62:
    case MacBookAir71:
    case MacBookAir72:
    case MacBookAir81:
    case MacBookAir82:
    case MacBookAir91:
      return true;
    case MacMini11:
    case MacMini21:
    case MacMini31:
    case MacMini41:
    case MacMini51:
    case MacMini52:
    case MacMini53:
    case MacMini61:
    case MacMini62:
    case MacMini71:
    case MacMini81:
      return false;
    case iMac41:
    case iMac42:
    case iMac51:
    case iMac52:
    case iMac61:
    case iMac71:
    case iMac81:
    case iMac91:
    case iMac101:
    case iMac111:
    case iMac112:
    case iMac113:
    case iMac121:
    case iMac122:
    case iMac131:
    case iMac132:
    case iMac133:
    case iMac141:
    case iMac142:
    case iMac143:
    case iMac144:
    case iMac151:
    case iMac161:
    case iMac162:
    case iMac171:
    case iMac181:
    case iMac182:
    case iMac183:
    case iMac191:
    case iMac192:
    case iMac201:
    case iMac202:
    case iMacPro11:
      return false;
    case MacPro11:
    case MacPro21:
    case MacPro31:
    case MacPro41:
    case MacPro51:
    case MacPro61:
    case MacPro71:
      return false;
    case Xserve11:
    case Xserve21:
    case Xserve31:
      return false;
    case MaxMacModel: // currently a copy of iMac132
      return false;
    default: // bug, unknown Apple model
      log_technical_bug("%s : cannot find model %d\n", __PRETTY_FUNCTION__, Model);
      return false;
    }
}

  // PlatformFeature
  // the memory tab in About This Mac
  // by TheRacerMaster
UINT64 GetPlatformFeature(MacModel Model)
{
  switch ( Model )
    {
    // Verified list from ioreg
    case iMac171:
    case iMac181:
    case iMac182:
    case iMac183:
    case MacPro71:
      return 0x00;
      break;
    case MacMini61:
    case MacMini62:
    case iMac131:
    case iMac132:
    case iMac133:
    case iMac141:
    case iMac142:
    case iMac143:
    case iMac144:
    case iMac151:
      return 0x01;
      break;
    case MacBookPro111:
    case MacBookPro112:
    case MacBookPro113:
    case MacBookPro114:
    case MacBookPro115:
    case MacBookPro121:
    case MacBookAir71:
    case MacBookAir72:
      return 0x02;
      break;
    case MacMini71:
    case iMac161:
    case iMac162:
      return 0x03;
      break;
    case MacPro61:
      return 0x04;
      break;
    case MacBook81:
    case MacBook91:
    case MacBook101:
    case MacBookPro131:
    case MacBookPro132:
    case MacBookPro133:
    case MacBookPro141:
    case MacBookPro142:
    case MacBookPro143:
      return 0x1A;
      break;
    case iMacPro11:
    case MacMini81:
    case iMac191:
    case iMac192:
    case iMac201:
    case iMac202:
      return 0x20;
      break;
    case MacBookPro151:
    case MacBookPro152:
    case MacBookPro153:
    case MacBookPro154:
    case MacBookPro161:
    case MacBookPro162: //there is also 0x3A
    case MacBookPro163:
    case MacBookPro164:
      return 0x32;
      break;
    case MacBookAir81:
    case MacBookAir82:
    case MacBookAir91:
      return 0x3A;
      break;
      // It is nonsense, ASCII code сharacter "2" = 0x32 != 0x02. Don't use ioreg, so that not to be confused. Use dmidecode dump.
      // Verified list from Users
      // case MacBookPro153:
      // case MacBookPro154:
      // case MacBookPro161:
      //   gSettings.Smbios.gPlatformFeature        = 0x02;
      //   break;
    default:
      return 0xFFFF; // disabled
      break;
    }
}

void getRBr(MacModel Model, UINT32 CPUModel, XBool isMobile, char RBr[8])
{
  memset(RBr, 0, 8);
  if (ApplePlatformDataArray[Model].smcBranch[0] != 'N') {
//    snprintf(RBr, 8, "%s", ApplePlatformData[Model].smcBranch.c_str());
    memcpy(RBr, ApplePlatformDataArray[Model].smcBranch.c_str(), MIN(8, ApplePlatformDataArray[Model].smcBranch.sizeInBytesIncludingTerminator()));
  } else {
    switch (CPUModel) {
    case CPU_MODEL_PENTIUM_M:
    case CPU_MODEL_CELERON:
      snprintf(RBr, 8, "%s", "m70");
      break;
      
    case CPU_MODEL_YONAH:
      snprintf(RBr, 8, "%s", "k22");
      break;
      
    case CPU_MODEL_MEROM: //TODO check for mobile
      snprintf(RBr, 8, "%s", "m75");
      break;
      
    case CPU_MODEL_PENRYN:
      if (isMobile) {
        snprintf(RBr, 8, "%s", "m82");
      } else {
        snprintf(RBr, 8, "%s", "k36");
      }
      break;
      
    case CPU_MODEL_SANDY_BRIDGE:
      if (isMobile) {
        snprintf(RBr, 8, "%s", "k90i");
      } else {
        snprintf(RBr, 8, "%s", "k60");
      }
      break;
      
    case CPU_MODEL_IVY_BRIDGE:
      snprintf(RBr, 8, "%s", "j30");
      break;
      
    case CPU_MODEL_IVY_BRIDGE_E5:
      snprintf(RBr, 8, "%s", "j90");
      break;
      
    case CPU_MODEL_HASWELL_ULT:
      snprintf(RBr, 8, "%s", "j44");
      break;
      
    case CPU_MODEL_HASWELL_U5: //Mobile - Broadwell
      snprintf(RBr, 8, "%s", "j52");
      break;
      
    case CPU_MODEL_SKYLAKE_D:
//      snprintf(RBr, 8, "%s", "j95j95am");
      memcpy(RBr, "j95j95am", 8);
      break;
      
    case CPU_MODEL_SKYLAKE_U:
      snprintf(RBr, 8, "%s", "2016mb");
      break;
      
    case CPU_MODEL_KABYLAKE1: //Mobile
      snprintf(RBr, 8, "%s", "2017mbp");
      break;
      
    case CPU_MODEL_KABYLAKE2: //Desktop
//      snprintf(RBr, 8, "%s", "j133_4_5");
      memcpy(RBr, "j133_4_5", 8);
      break;
      
    default:
      snprintf(RBr, 8, "%s", "t9");
      break;
    }
  }
}

void getRPlt(MacModel Model, UINT32 CPUModel, XBool isMobile, char RPlt[8])
{
  memset(RPlt, 0, 8);
  if (ApplePlatformDataArray[Model].smcPlatform[0] != 'N') {
    snprintf(RPlt, 8, "%s", ApplePlatformDataArray[Model].smcPlatform.c_str());
//    memcpy(RPlt, ApplePlatformData[Model].smcPlatform.c_str(), 8);
  } else {
    switch (CPUModel) {
    case CPU_MODEL_PENTIUM_M:
    case CPU_MODEL_CELERON:
      snprintf(RPlt, 8, "m70");
      break;
      
    case CPU_MODEL_YONAH:
      snprintf(RPlt, 8, "k22");
      break;
      
    case CPU_MODEL_MEROM: //TODO check for mobile
      snprintf(RPlt, 8, "m75");
      break;
      
    case CPU_MODEL_PENRYN:
      if (isMobile) {
        snprintf(RPlt, 8, "m82");
      } else {
        snprintf(RPlt, 8, "k36");
      }
      break;
      
    case CPU_MODEL_SANDY_BRIDGE:
      if (isMobile) {
        snprintf(RPlt, 8, "k90i");
      } else {
        snprintf(RPlt, 8, "k60");
      }
      break;
      
    case CPU_MODEL_IVY_BRIDGE:
      snprintf(RPlt, 8, "j30");
      break;
      
    case CPU_MODEL_IVY_BRIDGE_E5:
      snprintf(RPlt, 8, "j90");
      break;
      
    case CPU_MODEL_HASWELL_ULT:
      snprintf(RPlt, 8, "j44");
      break;
      
    case CPU_MODEL_HASWELL_U5: //Mobile - Broadwell
      snprintf(RPlt, 8, "j52");
      break;
      
    case CPU_MODEL_SKYLAKE_D:
      snprintf(RPlt, 8, "j95");
      break;
      
    case CPU_MODEL_SKYLAKE_U:
      snprintf(RPlt, 8, "j79");
      break;
      
    case CPU_MODEL_KABYLAKE1: //Mobile
      snprintf(RPlt, 8, "j130a");
      break;
      
    case CPU_MODEL_KABYLAKE2: //Desktop
      snprintf(RPlt, 8, "j135");
      break;
      
    default:
      snprintf(RPlt, 8, "t9");
      break;
    }
  }
}

XBool isReleaseDateWithYear20(MacModel Model)
{
  switch ( Model )
  {
    case MacBook11:
    case MacBook21:
    case MacBook31:
    case MacBook41:
    case MacBook51:
    case MacBook52:
    case MacBook61:
    case MacBook71:
    case MacBookPro11:
    case MacBookPro12:
    case MacBookPro21:
    case MacBookPro22:
    case MacBookPro31:
    case MacBookPro41:
    case MacBookPro51:
    case MacBookPro52:
    case MacBookPro53:
    case MacBookPro54:
    case MacBookPro55:
    case MacBookPro61:
    case MacBookPro62:
    case MacBookPro71:
    case MacBookPro81:
    case MacBookPro82:
    case MacBookPro83:
    case MacBookAir11:
    case MacBookAir21:
    case MacBookAir31:
    case MacBookAir32:
    case MacMini11:
    case MacMini21:
    case MacMini31:
    case MacMini41:
    case iMac41:
    case iMac42:
    case iMac51:
    case iMac52:
    case iMac61:
    case iMac71:
    case iMac81:
    case iMac91:
    case iMac101:
    case iMac111:
    case iMac112:
    case iMac113:
    case iMac121:
    case iMac122:
    case MacPro11:
    case MacPro21:
    case MacPro31:
    case MacPro41:
    case MacPro51:
    case Xserve11:
    case Xserve21:
    case Xserve31: {
      return false;
    }
    default: {
      return true;
    }
  }
}

// AppleReleaseDate
XString8 GetReleaseDate(MacModel Model)
{
  XString8 returnValue;

  const char* i = ApplePlatformDataArray[Model].firmwareVersion.c_str();
  i += strlen(i);
  while ( *i != '.' ) i--;
  if ( isReleaseDateWithYear20(Model) ) {
    returnValue.S8Printf("%c%c/%c%c/20%c%c", i[3], i[4], i[5], i[6], i[1], i[2]);
  }else{
    returnValue.S8Printf("%c%c/%c%c/%c%c", i[3], i[4], i[5], i[6], i[1], i[2]);
  }
  return returnValue;
}

void SetDMISettingsForModel(MacModel Model, SETTINGS_DATA* settingsData)
{
  settingsData->Smbios.BiosVersion = ApplePlatformDataArray[Model].firmwareVersion;
  settingsData->Smbios.BiosReleaseDate = GetReleaseDate(Model);
  settingsData->Smbios.EfiVersion = ApplePlatformDataArray[Model].efiversion;

  settingsData->Smbios.BiosVendor = AppleBiosVendor;
  settingsData->Smbios.ManufactureName = settingsData->Smbios.BiosVendor;
  settingsData->Smbios.ProductName = MachineModelName[Model];
  settingsData->Smbios.SystemVersion = ApplePlatformDataArray[Model].systemVersion;
  settingsData->Smbios.SerialNr = ApplePlatformDataArray[Model].serialNumber;
  settingsData->Smbios.FamilyName = ApplePlatformDataArray[Model].productFamily;
  settingsData->Smbios.BoardManufactureName = settingsData->Smbios.BiosVendor;
  settingsData->Smbios.BoardSerialNumber = AppleBoardSN;
  settingsData->Smbios.BoardNumber = ApplePlatformDataArray[Model].boardID;
  settingsData->Smbios.BoardVersion = MachineModelName[Model];
  settingsData->Smbios.LocationInChassis = AppleBoardLocation;
  settingsData->Smbios.ChassisManufacturer = settingsData->Smbios.BiosVendor;
  settingsData->Smbios.ChassisAssetTag = ApplePlatformDataArray[Model].chassisAsset;
  settingsData->Smbios.FirmwareFeatures = GetFwFeatures(Model);
  settingsData->Smbios.FirmwareFeaturesMask = GetFwFeaturesMaskFromModel(Model);
  settingsData->Smbios.ExtendedFirmwareFeatures = GetExtFwFeatures(Model);
  settingsData->Smbios.ExtendedFirmwareFeaturesMask = GetExtFwFeaturesMask(Model);
  settingsData->Smbios.gPlatformFeature = GetPlatformFeature(Model);
  if ((Model > MacPro31) && (Model < MacPro71)) {
    settingsData->Smbios.BoardType = BaseBoardTypeProcessorMemoryModule; //0xB;
  } else {
    settingsData->Smbios.BoardType = BaseBoardTypeMotherBoard; //0xA;
  }
  settingsData->Smbios.ChassisType = GetChassisTypeFromModel(Model);
  settingsData->Smbios.Mobile = GetMobile(Model); // Mobile: the battery tab in Energy Saver
}

MacModel GetModelFromString(const XString8& ProductName)
{
  MacModel i;

  for (i = (MacModel)(0); i < MaxMacModel; i = (MacModel)(i + 1)) {
    if ( ProductName == MachineModelName[i] ) {
      return i;
    }
  }
  // return ending enum as "not found"
  return MaxMacModel;
}

uint8_t GetChassisTypeFromModel(MacModel Model)
{

  // MiscChassisType
  // Mobile: the battery tab in Energy Saver
  switch (Model) {
    case MacBook11:
    case MacBook21:
    case MacBook31:
    case MacBook41:
    case MacBook51:
    case MacBook52:
    case MacBook61:
    case MacBook71:
    case MacBookAir11:
    case MacBookAir21:
    case MacBookAir31:
    case MacBookAir32:
    case MacBookAir41:
    case MacBookAir42:
    case MacBookAir51:
    case MacBookAir52:
    case MacBookAir61:
    case MacBookAir62:
    case MacBookPro81:
    case MacBookPro82:
    case MacBookPro83:
    case MacBookPro91:
    case MacBookPro92:
    case MacBookPro101:
    case MacBookPro102:
    case MacBookPro111:
    case MacBookPro112:
    case MacBookPro113:
    case MacBookPro114:
    case MacBookPro115:
    case MacMini71:
      return MiscChassisTypeNotebook; //0x0A;

    case MacBook81:
    case MacBook91:
    case MacBook101:
    case MacBookPro121:
    case MacBookPro131:
    case MacBookPro132:
    case MacBookPro133:
    case MacBookPro141:
    case MacBookPro142:
    case MacBookPro143:
    case MacBookPro151:
    case MacBookPro152:
    case MacBookPro153:
    case MacBookPro154:
    case MacBookPro161:
    case MacBookPro162:
    case MacBookPro163:
    case MacBookPro164:
    case MacBookAir71:
    case MacBookAir72:
    case MacBookAir81:
    case MacBookAir82:
    case MacBookAir91:
    case MacMini81:
    case iMac161:
    case iMac162:
    case iMac171:
    case iMac181:
    case iMac182:
    case iMac183:
    case iMac191:
    case iMac192:
    case iMac201:
    case iMac202:
    case iMacPro11:
      return MiscChassisTypeLapTop; //0x09;

    case MacBookPro11:
    case MacBookPro12:
    case MacBookPro21:
    case MacBookPro22:
    case MacBookPro31:
    case MacBookPro41:
    case MacBookPro51:
    case MacBookPro52:
    case MacBookPro53:
    case MacBookPro54:
    case MacBookPro55:
    case MacBookPro61:
    case MacBookPro62:
    case MacBookPro71:
      return MiscChassisTypePortable; //0x08;

    case iMac41:
    case iMac42:
    case iMac51:
    case iMac52:
    case iMac61:
    case iMac71:
    case iMac81:
    case iMac91:
    case iMac101:
    case iMac111:
    case iMac112:
    case iMac113:
    case iMac121:
    case iMac122:
    case iMac131:
    case iMac132:
    case iMac133:
    case iMac141:
    case iMac142:
    case iMac143:
    case iMac144:
    case iMac151:
      return MiscChassisTypeAllInOne; //0x0D;

    case MacMini11:
    case MacMini21:
      return MiscChassisTypeLowProfileDesktop; //0x04;

    case MacMini31:
    case MacMini41:
    case MacMini51:
    case MacMini52:
    case MacMini53:
    case MacMini61:
    case MacMini62:
      return MiscChassisTypeLunchBox; //0x10;
      break;

    case MacPro41:
    case MacPro51:
    case MacPro71:
      return MiscChassisTypeTower; //0x07;

    case MacPro11:
    case MacPro21:
    case MacPro31:
    case MacPro61:
      return MiscChassisTypeUnknown; //0x02; this is a joke but think different!
          
    case Xserve11:
    case Xserve21:
    case Xserve31:
      return MiscChassisTypeRackMountChassis; //0x17;

    default: //unknown - use oem SMBIOS value to be default
      /*if (gMobile) {
        return 10; //notebook
      } else {
        return MiscChassisTypeDeskTop; //0x03;
      }*/
      return 0;
      break;
  }
}




//gFwFeaturesMask


uint32_t GetFwFeaturesMaskFromModel(MacModel Model)
{

  // FirmwareFeaturesMask
  switch (Model) {
    // Verified list from Firmware
    case MacBookPro91:
    case MacBookPro92:
    case MacBookPro101:
    case MacBookPro102:
    case MacBookPro111:
    case MacBookPro112:
    case MacBookPro113:
    case MacBookPro114:
    case MacBookPro115:
    case MacBookAir41:
    case MacBookAir42:
    case MacBookAir51:
    case MacBookAir52:
    case MacBookAir61:
    case MacBookAir62:
    case MacMini51:
    case MacMini52:
    case MacMini53:
    case MacMini61:
    case MacMini62:
    case iMac131:
    case iMac132:
    case iMac133:
    case iMac141:
    case iMac142:
    case iMac143:
      return 0xFF1FFF3F;
      break;
          
    case MacBook91:
    case MacBook101:
    case MacBookPro131:
    case MacBookPro132:
    case MacBookPro133:
    case MacBookPro141:
    case MacBookPro142:
    case MacBookPro143:
    case iMac144:
    case iMac151:
    case iMac171:
    case iMac181:
    case iMac182:
    case iMac183:
    case MacPro61:
      return 0xFF1FFF7F;
      break;
    case iMacPro11:
    case MacBookAir91:
      return 0xFF9FFF3F;
      break;
    case iMac191:
    case iMac192:
    case iMac201:
    case iMac202:
    case MacMini81:
      return 0xFFDFFF7F;
      break;
    case MacBookPro162:
    case MacBookPro163:
    case MacBookPro164:
      return 0xFFFFFF7F;
      break;

    // Verified list from Users
    case MacBook61:
    case MacBook71:
    case MacBook81:
    case MacBookPro61:
    case MacBookPro62:
    case MacBookPro71:
    case MacBookPro81:
    case MacBookPro82:
    case MacBookPro83:
    case MacBookPro121:
    case MacBookPro151:
    case MacBookPro152:
    case MacBookPro153:
    case MacBookPro154:
    case MacBookPro161:
    case MacBookAir31:
    case MacBookAir32:
    case MacBookAir71:
    case MacBookAir72:
    case MacBookAir81:
    case MacBookAir82:
    case MacMini41:
    case MacMini71:
    case iMac101:
    case iMac111:
    case iMac112:
    case iMac113:
    case iMac121:
    case iMac122:
    case iMac161:
    case iMac162:
    case MacPro51:
      return 0xFF1FFF3F;
      break;

    case MacPro71:
      return 0xFF9FFF3F;
      break;

    default:
      return 0xFFFFFFFF; //unknown - use oem SMBIOS value to be default
      break;
  }
}

/*
 * parameters MUST contains at least a dot, followed by at lest 6 chars
 */
int compareBiosVersion(const XString8& version1, const XString8& version2)
{
  const CHAR8* v1p = version1.c_str();
  const CHAR8* v2p = version2.c_str();

  v1p += strlen(v1p);
  while (*v1p != '.') {
    v1p--;
  }

  v2p += strlen(v2p);
  while (*v2p != '.') {
    v2p--;
  }
  if ( strlen(v1p) < 7 ) {
    log_technical_bug("strlen(v1p) < 7");
    return false;
  }
  if ( strlen(v2p) < 7 ) {
    log_technical_bug("strlen(v2p) < 7");
    return false;
  }

  if (((v1p[1] > '0') && (v2p[1] == '0')) || ((v1p[1] >= v2p[1]) && (v1p[2] > v2p[2]))) {
    return 1;
  } else if ((v1p[1] == v2p[1]) && (v1p[2] == v2p[2])) {
    if (((v1p[3] > '0') && (v2p[3] == '0')) || ((v1p[3] >= v2p[3]) && (v1p[4] > v2p[4]))) {
      return 1;
    } else if ((v1p[3] == v2p[3]) && (v1p[4] == v2p[4])) {
      if (((v1p[5] > '0') && (v2p[5] == '0')) || ((v1p[5] > '1') && (v2p[5] == '1')) ||
          ((v1p[5] > '2') && (v2p[5] == '2')) || ((v1p[5] >= v2p[5]) && (v1p[6] > v2p[6]))) {
        return 1;
      } else if ((v1p[5] == v2p[5]) && (v1p[6] == v2p[6])) {
        // equal
        return 0;
      } else {
        return -1;
      }
    } else {
      return -1;
    }
  } else {
    return -1;
  }
}

XBool is2ndBiosVersionGreaterThan1st(const XString8& version1, const XString8& version2)
{
  return compareBiosVersion(version1, version2) <= 0;
}
XBool is2ndBiosVersionEqual(const XString8& version1, const XString8& version2)
{
  return compareBiosVersion(version1, version2) == 0;
}


int compareReleaseDate(const XString8& date1, const XString8& date2)
{
  const CHAR8* i = date1.c_str();
  const CHAR8* j = date2.c_str();

  if ( (strlen(i) == 8) && (strlen(j) == 8) )
  {
    if ( ((i[6] > '0') && (j[6] == '0')) || ((i[6] >= j[6]) && (i[7] > j[7])) )
    {
      return 1;
    } else if ( (i[6] == j[6]) && (i[7] == j[7]) )
    {
      if ( ((i[0] > '0') && (j[0] == '0')) || ((i[0] >= j[0]) && (i[1] > j[1])) )
      {
        return 1;
      } else if ( (i[0] == j[0]) && (i[1] == j[1]) )
      {
        if ( ((i[3] > '0') && (j[3] == '0')) || ((i[3] > '1') && (j[3] == '1')) || ((i[3] > '2') && (j[3] == '2')) || ((i[3] >= j[3]) && (i[4] > j[4])) )
        {
          return 1;
        } else if ( (i[3] == j[3]) && (i[4] == j[4]) )
        {
          return 0;
        } else
        {
          return -1;
        }
      } else
      {
        return -1;
      }
    } else
    {
      return -1;
    }
  } else if ( (strlen(i) == 8) && (strlen(j) == 10) )
  {
    if ( ((i[6] > '0') && (j[8] == '0')) || ((i[6] >= j[8]) && (i[7] > j[9])) )
    {
      return 1;
    } else if ( (i[6] == j[8]) && (i[7] == j[9]) )
    {
      if ( ((i[0] > '0') && (j[0] == '0')) || ((i[0] >= j[0]) && (i[1] > j[1])) )
      {
        return 1;
      } else if ( (i[0] == j[0]) && (i[1] == j[1]) )
      {
        if ( ((i[3] > '0') && (j[3] == '0')) || ((i[3] > '1') && (j[3] == '1')) || ((i[3] > '2') && (j[3] == '2')) || ((i[3] >= j[3]) && (i[4] > j[4])) )
        {
          return 1;
        } else if ( (i[3] == j[3]) && (i[4] == j[4]) )
        {
          return 0;
        } else
        {
          return -1;
        }
      } else
      {
        return -1;
      }
    } else
    {
      return -1;
    }
  } else if ( (strlen(i) == 10) && (strlen(j) == 10) )
  {
    if ( ((i[8] > '0') && (j[8] == '0')) || ((i[8] >= j[8]) && (i[9] > j[9])) )
    {
      return 1;
    } else if ( (i[8] == j[8]) && (i[9] == j[9]) )
    {
      if ( ((i[0] > '0') && (j[0] == '0')) || ((i[0] >= j[0]) && (i[1] > j[1])) )
      {
        return 1;
      } else if ( (i[0] == j[0]) && (i[1] == j[1]) )
      {
        if ( ((i[3] > '0') && (j[3] == '0')) || ((i[3] > '1') && (j[3] == '1')) || ((i[3] > '2') && (j[3] == '2')) || ((i[3] >= j[3]) && (i[4] > j[4])) )
        {
          return 1;
        } else if ( (i[3] == j[3]) && (i[4] == j[4]) )
        {
          return 0;
        } else
        {
          return -1;
        }
      } else
      {
        return -1;
      }
    } else
    {
      return -1;
    }
  } else if ( (strlen(i) == 10) && (strlen(j) == 8) )
  {
    if ( ((i[8] > '0') && (j[6] == '0')) || ((i[8] >= j[6]) && (i[9] > j[7])) )
    {
      return 1;
    } else if ( (i[8] == j[6]) && (i[9] == j[7]) )
    {
      if ( ((i[0] > '0') && (j[0] == '0')) || ((i[0] >= j[0]) && (i[1] > j[1])) )
      {
        return 1;
      } else if ( (i[0] == j[0]) && (i[1] == j[1]) )
      {
        if ( ((i[3] > '0') && (j[3] == '0')) || ((i[3] > '1') && (j[3] == '1')) || ((i[3] > '2') && (j[3] == '2')) || ((i[3] >= j[3]) && (i[4] > j[4])) )
        {
          return 1;
        } else if ( (i[3] == j[3]) && (i[4] == j[4]) )
        {
          return 0;
        } else
        {
          return -1;
        }
      } else
      {
        return -1;
      }
    } else
    {
      return -1;
    }
  } else
  {
    return -2;
  }
}


