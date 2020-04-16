/*
 * platformdata.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_PLATFORMDATA_H_
#define PLATFORM_PLATFORMDATA_H_

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
