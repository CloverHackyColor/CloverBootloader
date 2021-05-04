/*
 * platformdata.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_PLATFORMDATA_H_
#define PLATFORM_PLATFORMDATA_H_


#include "../cpp_foundation/XString.h"
//#include "../Platform/Settings.h"

class SETTINGS_DATA;
class REFIT_CONFIG;

typedef enum {

  MacBook11,
  MacBook21,
  MacBook31,
  MacBook41,
  MacBook51,
  MacBook52,
  MacBook61,
  MacBook71,
  MacBook81,
  MacBook91,
  MacBook101,
  MacBookPro11,
  MacBookPro12,
  MacBookPro21,
  MacBookPro22,
  MacBookPro31,
  MacBookPro41,
  MacBookPro51,
  MacBookPro52,
  MacBookPro53,
  MacBookPro54,
  MacBookPro55,
  MacBookPro61,
  MacBookPro62,
  MacBookPro71,
  MacBookPro81,
  MacBookPro82,
  MacBookPro83,
  MacBookPro91,
  MacBookPro92,
  MacBookPro101,
  MacBookPro102,
  MacBookPro111,
  MacBookPro112,
  MacBookPro113,
  MacBookPro114,
  MacBookPro115,
  MacBookPro121,
  MacBookPro131,
  MacBookPro132,
  MacBookPro133,
  MacBookPro141,
  MacBookPro142,
  MacBookPro143,
  MacBookPro151,
  MacBookPro152,
  MacBookPro153,
  MacBookPro154,
  MacBookPro161,
  MacBookPro162,
  MacBookPro163,
  MacBookPro164,
  MacBookAir11,
  MacBookAir21,
  MacBookAir31,
  MacBookAir32,
  MacBookAir41,
  MacBookAir42,
  MacBookAir51,
  MacBookAir52,
  MacBookAir61,
  MacBookAir62,
  MacBookAir71,
  MacBookAir72,
  MacBookAir81,
  MacBookAir82,
  MacBookAir91,
  MacMini11,
  MacMini21,
  MacMini31,
  MacMini41,
  MacMini51,
  MacMini52,
  MacMini53,
  MacMini61,
  MacMini62,
  MacMini71,
  MacMini81,
  iMac41,
  iMac42,
  iMac51,
  iMac52,
  iMac61,
  iMac71,
  iMac81,
  iMac91,
  iMac101,
  iMac111,
  iMac112,
  iMac113,
  iMac121,
  iMac122,
  iMac131,
  iMac132,
  iMac133,
  iMac141,
  iMac142,
  iMac143,
  iMac144,
  iMac151,
  iMac161,
  iMac162,
  iMac171,
  iMac181,
  iMac182,
  iMac183,
  iMac191,
  iMac192,
  iMac201,
  iMac202,
  iMacPro11,
  MacPro11,
  MacPro21,
  MacPro31,
  MacPro41,
  MacPro51,
  MacPro61,
  MacPro71,
  Xserve11,
  Xserve21,
  Xserve31,

  MaxMachineType

} MACHINE_TYPES;


constexpr LString8 DefaultMemEntry        = "N/A"_XS8;
constexpr LString8 DefaultSerial          = "CT288GT9VT6"_XS8;
constexpr LString8 AppleBiosVendor        = "Apple Inc."_XS8;
constexpr LString8 AppleManufacturer      = "Apple Computer, Inc."_XS8; //Old name, before 2007
constexpr LString8 AppleBoardSN           = "C02140302D5DMT31M"_XS8;
constexpr LString8 AppleBoardLocation     = "Part Component"_XS8;


class PLATFORMDATA
{
public:
  const LString8 productName;
  const LString8 firmwareVersion;
  const LString8 efiversion;
  const LString8 boardID;
  const LString8 productFamily;
  const LString8 systemVersion;
  const XString8 serialNumber;
  const LString8 chassisAsset;
  UINT8 smcRevision[6];
  const LString8 smcBranch;
  const LString8 smcPlatform;
  UINT32 smcConfig;
  
  //PLATFORMDATA() : productName(), firmwareVersion(), efiversion(), boardID(), productFamily(), systemVersion(), serialNumber(), chassisAsset(), smcRevision{0,0,0,0,0,0}, smcBranch(), smcPlatform(), smcConfig() { }
  PLATFORMDATA(const LString8& _productName, const LString8& _firmwareVersion, const LString8& _efiversion, const LString8& _boardID, const LString8& _productFamily,
               const LString8& _systemVersion, const LString8& _serialNumber, const LString8& _chassisAsset,
               UINT8 _smcRevision0, UINT8 _smcRevision1, UINT8 _smcRevision2, UINT8 _smcRevision3, UINT8 _smcRevision4, UINT8 _smcRevision5,
               const LString8& _smcBranch, const LString8& _smcPlatform, UINT32 _smcConfig)
            :  productName(_productName), firmwareVersion(_firmwareVersion), efiversion(_efiversion), boardID(_boardID), productFamily(_productFamily),
               systemVersion(_systemVersion), serialNumber(_serialNumber), chassisAsset(_chassisAsset), smcRevision{0},
               smcBranch(_smcBranch), smcPlatform(_smcPlatform), smcConfig(_smcConfig)
            {
              smcRevision[0] = _smcRevision0;
              smcRevision[1] = _smcRevision1;
              smcRevision[2] = _smcRevision2;
              smcRevision[3] = _smcRevision3;
              smcRevision[4] = _smcRevision4;
              smcRevision[5] = _smcRevision5;
            }

  // Not sure if default are valid. Delete them. If needed, proper ones can be created
  PLATFORMDATA(const PLATFORMDATA&) = delete;
  PLATFORMDATA& operator=(const PLATFORMDATA&) = delete;
} ;


extern PLATFORMDATA ApplePlatformData[];

void SetDMISettingsForModel(MACHINE_TYPES Model, SETTINGS_DATA* settingsData, REFIT_CONFIG* liveConfig);
MACHINE_TYPES GetModelFromString (const XString8& ProductName);

bool isReleaseDateWithYear20(MACHINE_TYPES Model);
XString8 GetReleaseDate (MACHINE_TYPES Model);
uint8_t GetChassisTypeFromModel(MACHINE_TYPES Model);
uint32_t GetFwFeaturesMaskFromModel(MACHINE_TYPES Model);
uint32_t GetFwFeatures(MACHINE_TYPES Model);
bool GetMobile(MACHINE_TYPES Model);
UINT64 GetPlatformFeature(MACHINE_TYPES Model);
void getRBr(MACHINE_TYPES Model, UINT32 CPUModel, bool isMobile, char RBr[8]);
void getRPlt(MACHINE_TYPES Model, UINT32 CPUModel, bool isMobile, char RPlt[8]);

int compareBiosVersion(const XString8& version1, const XString8& version2);
bool is2ndBiosVersionGreaterThan1st(const XString8& version1, const XString8& version2);
bool isBiosVersionEquel(const XString8& version1, const XString8& version2);

int compareReleaseDate(const XString8& date1, const XString8& date2);

#endif /* PLATFORM_PLATFORMDATA_H_ */
