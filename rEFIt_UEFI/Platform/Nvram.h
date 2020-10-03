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
#include "../gui/REFIT_MENU_SCREEN.h"


extern EFI_GUID                       *gEfiBootDeviceGuid;
extern EFI_DEVICE_PATH_PROTOCOL       *gEfiBootDeviceData;




INTN
FindStartupDiskVolume (
  REFIT_MENU_SCREEN *MainMenu
  );

void
*GetNvramVariable(
    IN      CONST CHAR16   *VariableName,
    IN      EFI_GUID       *VendorGuid,
    OUT  UINT32            *Attributes    OPTIONAL,
    OUT  UINTN             *DataSize      OPTIONAL
  );
XString8 GetNvramVariableAsXString8(
    IN      CONST CHAR16   *VariableName,
    IN      EFI_GUID       *VendorGuid,
    OUT     UINT32         *Attributes    OPTIONAL,
    OUT     UINTN          *DataSize      OPTIONAL
  );

EFI_STATUS
AddNvramVariable (
  IN  CONST CHAR16   *VariableName,
  IN  EFI_GUID *VendorGuid,
  IN  UINT32   Attributes,
  IN  UINTN    DataSize,
  IN  const void     *Data
  );
EFI_STATUS
AddNvramXString8 (
  IN  CONST CHAR16   *VariableName,
  IN  EFI_GUID *VendorGuid,
  IN  UINT32   Attributes,
  const XString8& s
  );

EFI_STATUS
SetNvramVariable (
  IN  CONST CHAR16      *VariableName,
  IN  EFI_GUID    *VendorGuid,
  IN  UINT32       Attributes,
  IN  UINTN        DataSize,
  IN  CONST void  *Data
  );
EFI_STATUS
SetNvramXString8 (
    IN  CONST CHAR16     *VariableName,
    IN  EFI_GUID   *VendorGuid,
    IN  UINT32      Attributes,
    const XString8& s
  );

EFI_STATUS
DeleteNvramVariable (
  IN  CONST CHAR16   *VariableName,
  IN  EFI_GUID *VendorGuid
  );

void
ResetNvram (void);

BOOLEAN
IsDeletableVariable (
  IN CHAR16    *Name,
  IN EFI_GUID  *Guid
  );

EFI_STATUS
ResetNativeNvram (void);
;

EFI_STATUS
GetEfiBootDeviceFromNvram (void);

EFI_GUID
*FindGPTPartitionGuidInDevicePath (
  const  EFI_DEVICE_PATH_PROTOCOL *DevicePath
  );

void
PutNvramPlistToRtVars (void);

void
GetSmcKeys(BOOLEAN WriteToSMC);
#if CHECK_SMC
void DumpSmcKeys();
#endif

EFI_STATUS
SetStartupDiskVolume (
  IN  REFIT_VOLUME *Volume,
  IN  CONST XStringW& LoaderPath
  );

void
RemoveStartupDiskVolume (void);

UINT64
GetEfiTimeInMs (IN EFI_TIME *T);

#endif /* PLATFORM_NVRAM_H_ */
