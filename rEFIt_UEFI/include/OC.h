/*
 * OC.h
 *
 *  Created on: Oct 21, 2020
 *      Author: jief
 */

#ifndef INCLUDE_OC_H_
#define INCLUDE_OC_H_


#ifdef __cplusplus
extern "C" {
#endif

//#include "../../OpenCorePkg/Include/Acidanthera/OpenCore.h"

#include <Library/OcRtcLib.h>
#include <Library/OcOSInfoLib.h>
#include <Library/OcVirtualFsLib.h>
#include <Library/OcConfigurationLib.h>
#include <Library/OcDevicePathLib.h>
#include <Library/OcFileLib.h>
#include <Library/OcCpuLib.h> // OC_CPU_INFO
#include <Library/OcMainLib.h> // OcMiscEarlyInit
//#include <Protocol/OcBootstrap.h> // OC_BOOTSTRAP_PROTOCOL
#include <Library/OcBootManagementLib/BootManagementInternal.h>
#include <Library/OcAfterBootCompatLib/BootCompatInternal.h>

#include <Guid/AppleApfsInfo.h>

extern OC_GLOBAL_CONFIG mOpenCoreConfiguration;
extern OC_STORAGE_CONTEXT mOpenCoreStorage;
extern OC_CPU_INFO mOpenCoreCpuInfo;
//extern OC_BOOTSTRAP_PROTOCOL mOpenCoreBootStrap;
//extern OC_RSA_PUBLIC_KEY* mOpenCoreVaultKey;
//extern EFI_HANDLE mLoadHandle;


EFI_STATUS
EFIAPI
OcKernelFileOpen (
  IN  EFI_FILE_PROTOCOL       *This,
  OUT EFI_FILE_PROTOCOL       **NewHandle,
  IN  CHAR16                  *FileName,
  IN  UINT64                  OpenMode,
  IN  UINT64                  Attributes
  );

EFI_STATUS
EFIAPI
InternalEfiLoadImage (
  IN  BOOLEAN                      BootPolicy,
  IN  EFI_HANDLE                   ParentImageHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL     *DevicePath,
  IN  void                         *SourceBuffer OPTIONAL,
  IN  UINTN                        SourceSize,
  OUT EFI_HANDLE                   *ImageHandle
  );

EFI_STATUS
EFIAPI
OcStartImage (
  IN  EFI_HANDLE  ImageHandle,
  OUT UINTN       *ExitDataSize,
  OUT CHAR16      **ExitData  OPTIONAL
  );

void
OcLoadBooterUefiSupport (
  IN OC_GLOBAL_CONFIG  *Config
  );

UINT64
InternalCalculateARTFrequencyIntel (
  OUT UINT64   *CPUFrequency,
  OUT UINT64   *TscAdjustPtr OPTIONAL,
  IN  BOOLEAN  Recalculate
  );

EFI_STATUS
ClOcReadConfigurationFile(
  IN  OC_STORAGE_CONTEXT *Storage,
  IN  CONST CHAR16* configPath,
  OUT OC_GLOBAL_CONFIG   *Config
 );

EFI_STATUS
InternalGetApfsSpecialFileInfo (
  IN     EFI_FILE_PROTOCOL          *Root,
  IN OUT APPLE_APFS_VOLUME_INFO     **VolumeInfo OPTIONAL,
  IN OUT APPLE_APFS_CONTAINER_INFO  **ContainerInfo OPTIONAL
  );

void
OcMain (
  IN OC_STORAGE_CONTEXT        *Storage,
  IN EFI_DEVICE_PATH_PROTOCOL  *LoadPath OPTIONAL
  );


#ifdef __cplusplus
} // extern "C"
#endif

// This was a macro.
// But there is a catch : if parameter value depends on ocString.DynValue, DynValue is wiped before value is evaluated.
// Solution could have been an intermediary variable.
inline void OC_DATA_ASSIGN_N(OC_DATA& ocString, const unsigned char* value, size_t len) {
  if( len >= sizeof(ocString.Value) ) {
    memset(ocString.Value, 0, sizeof(ocString.Value));
    ocString.DynValue = (__typeof__(ocString.DynValue))malloc(len);
    memcpy(ocString.DynValue, value, len);
    ocString.MaxSize = (UINT32)len;
    ocString.Size = (UINT32)len;   /* unsafe cast */
  }else{
    ocString.DynValue = NULL;
    memcpy(ocString.Value, value, len);
    ocString.MaxSize = sizeof(ocString.Value);
    ocString.Size = (UINT32)len;   /* unsafe cast */
  }
}

// This was a macro.
// But there is a catch : if parameter value depends on ocString.DynValue, DynValue is wiped before value is evaluated.
// Solution could have been an intermediary variable.
inline void OC_STRING_ASSIGN_N(OC_STRING& ocString, const char* value, size_t len) {
  if( len >= sizeof(ocString.Value) ) {
    memset(ocString.Value, 0, sizeof(ocString.Value));
    ocString.DynValue = (__typeof__(ocString.DynValue))malloc(len);
    memcpy(ocString.DynValue, value, len);
    ocString.MaxSize = (UINT32)len;
    ocString.Size = (UINT32)len;   /* unsafe cast */
  }else{
    ocString.DynValue = NULL;
    memcpy(ocString.Value, value, len);
    ocString.MaxSize = sizeof(ocString.Value);
    ocString.Size = (UINT32)len;   /* unsafe cast */
  }
}

#define OC_STRING_ASSIGN(ocString, value) OC_STRING_ASSIGN_N(ocString, value, strlen(value)+1)



#endif /* INCLUDE_OC_H_ */
