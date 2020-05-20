/*
 * platformdata.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_PLATFORMDATA_H_
#define PLATFORM_PLATFORMDATA_H_




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





extern CONST CHAR8                          *BiosVendor;
extern CONST CHAR8                          *AppleManufacturer;
extern CONST CHAR8                          *AppleBoardSN;
extern CONST CHAR8                          *AppleBoardLocation;

extern UINT32                         gFwFeatures;
extern UINT32                         gFwFeaturesMask;
extern UINT64                         gPlatformFeature;



VOID
SetDMISettingsForModel (
  MACHINE_TYPES Model,
  BOOLEAN Redefine
  );

MACHINE_TYPES GetModelFromString (
  CHAR8 *ProductName
  );

VOID
GetDefaultSettings(VOID);


#endif /* PLATFORM_PLATFORMDATA_H_ */
