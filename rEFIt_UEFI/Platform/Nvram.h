/*
 * Nvram.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_NVRAM_H_
#define PLATFORM_NVRAM_H_

#define NON_APPLE_SMC_SIGNATURE SIGNATURE_64('S','M','C','H','E','L','P','E')

#include "../cpp_foundation/XString.h"

extern EFI_GUID                       *gEfiBootDeviceGuid;
extern EFI_DEVICE_PATH_PROTOCOL       *gEfiBootDeviceData;




INTN
FindStartupDiskVolume (
  REFIT_MENU_SCREEN *MainMenu
  );

VOID
*GetNvramVariable(
  IN      CONST CHAR16   *VariableName,
  IN      EFI_GUID *VendorGuid,
     OUT  UINT32   *Attributes    OPTIONAL,
     OUT  UINTN    *DataSize      OPTIONAL
     );

EFI_STATUS
AddNvramVariable (
  IN  CONST CHAR16   *VariableName,
  IN  EFI_GUID *VendorGuid,
  IN  UINT32   Attributes,
  IN  UINTN    DataSize,
  IN  VOID     *Data
  );

EFI_STATUS
SetNvramVariable (
  IN  CONST CHAR16      *VariableName,
  IN  EFI_GUID    *VendorGuid,
  IN  UINT32       Attributes,
  IN  UINTN        DataSize,
  IN  CONST VOID  *Data
  );

EFI_STATUS
DeleteNvramVariable (
  IN  CONST CHAR16   *VariableName,
  IN  EFI_GUID *VendorGuid
  );

VOID
ResetNvram (VOID);

BOOLEAN
IsDeletableVariable (
  IN CHAR16    *Name,
  IN EFI_GUID  *Guid
  );

EFI_STATUS
ResetNativeNvram (VOID);
;

EFI_STATUS
GetEfiBootDeviceFromNvram (VOID);

EFI_GUID
*FindGPTPartitionGuidInDevicePath (
  IN  EFI_DEVICE_PATH_PROTOCOL *DevicePath
  );

VOID
PutNvramPlistToRtVars (VOID);

VOID
GetSmcKeys(BOOLEAN WriteToSMC);


EFI_STATUS
SetStartupDiskVolume (
  IN  REFIT_VOLUME *Volume,
  IN  CONST XStringW& LoaderPath
  );

VOID
RemoveStartupDiskVolume (VOID);

UINT64
GetEfiTimeInMs (IN EFI_TIME *T);

#endif /* PLATFORM_NVRAM_H_ */
