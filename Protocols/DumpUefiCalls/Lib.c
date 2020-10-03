/** @file
 
 Various helper functions.
 
 By dmazar, 26/09/2012
 
 **/

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/DevicePathLib.h>
//#include <Library/DeviceTreeLib.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/BlockIo.h>
#include <Protocol/BlockIo2.h>
#include <Protocol/DiskIo.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/SimpleFileSystem.h>

#include <Include/Guid/FileInfo.h>
#include <Include/Guid/FileSystemInfo.h>
#include <Include/Guid/FileSystemVolumeLabelInfo.h>

#include <Include/Guid/Acpi.h>
#include <Include/Guid/DebugImageInfoTable.h>
#include <Include/Guid/DxeServices.h>
#include <Include/Guid/HobList.h>
#include <Include/Guid/Mps.h>
#include <Include/Guid/SmBios.h>

#include "Common.h"


/** Memory type to string conversion */
CHAR16 *EfiMemoryTypeDesc[EfiMaxMemoryType] = {
  L"reserved",
  L"LoaderCode",
  L"LoaderData",
  L"BS_code",
  L"BS_data",
  L"RT_code",
  L"RT_data",
  L"available",
  L"Unusable",
  L"ACPI_recl",
  L"ACPI_NVS",
  L"MemMapIO",
  L"MemPortIO",
  L"PAL_code"
};

/** Allocation type to string conversion */
CHAR16 *EfiAllocateTypeDesc[MaxAllocateType] = {
  L"AllocateAnyPages",
  L"AllocateMaxAddress",
  L"AllocateAddress"
};

/** Locate search type to string conversion */
CHAR16 *EfiLocateSearchType[] = {
  L"AllHandles",
  L"ByRegisterNotify",
  L"ByProtocol"
};

/** Reset type to string conversion */
CHAR16 *EfiResetType[] = {
  L"EfiResetCold",
  L"EfiResetWarm",
  L"EfiResetShutdown"
};

//
// Some known guids
//
EFI_GUID mEfiConsoleControlProtocolGuid           = {0xF42F7782, 0x012E, 0x4C12, {0x99, 0x56, 0x49, 0xF9, 0x43, 0x04, 0xF7, 0x21}};
EFI_GUID mAppleFirmwarePasswordProtocolGuid       = {0x8FFEEB3A, 0x4C98, 0x4630, {0x80, 0x3F, 0x74, 0x0F, 0x95, 0x67, 0x09, 0x1D}};
EFI_GUID mEfiGlobalVariableGuid                   = {0x8BE4DF61, 0x93CA, 0x11D2, {0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C}};
EFI_GUID mAppleDevicePropertyProtocolGuid         = {0x91BD12FE, 0xF6C3, 0x44FB, {0xA5, 0xB7, 0x51, 0x22, 0xAB, 0x30, 0x3A, 0xE0}};
EFI_GUID mEfiAppleBootGuid                        = {0x7C436110, 0xAB2A, 0x4BBB, {0xA8, 0x80, 0xFE, 0x41, 0x99, 0x5C, 0x9F, 0x82}};
EFI_GUID mEfiAppleNvramGuid                       = {0x4D1EDE05, 0x38C7, 0x4A6A, {0x9C, 0xC6, 0x4B, 0xCC, 0xA8, 0xB3, 0x8C, 0x14}};
EFI_GUID mAppleFramebufferInfoProtocolGuid        = {0xE316E100, 0x0751, 0x4C49, {0x90, 0x56, 0x48, 0x6C, 0x7E, 0x47, 0x29, 0x03}};
EFI_GUID mAppleKeyStateProtocolGuid               = {0x5B213447, 0x6E73, 0x4901, {0xA4, 0xF1, 0xB8, 0x64, 0xF3, 0xB7, 0xA1, 0x72}};
EFI_GUID mAppleNetBootProtocolGuid                = {0x78EE99FB, 0x6A5E, 0x4186, {0x97, 0xDE, 0xCD, 0x0A, 0xBA, 0x34, 0x5A, 0x74}};
EFI_GUID mAppleImageCodecProtocolGuid             = {0x0DFCE9F6, 0xC4E3, 0x45EE, {0xA0, 0x6A, 0xA8, 0x61, 0x3B, 0x98, 0xA5, 0x07}};
EFI_GUID mAppleEfiVendorGuid                      = {0xAC39C713, 0x7E50, 0x423D, {0x88, 0x9D, 0x27, 0x8F, 0xCC, 0x34, 0x22, 0xB6}};
EFI_GUID mAppleEFINVRAMTRBSecureGuid              = {0xF68DA75E, 0x1B55, 0x4E70, {0xB4, 0x1B, 0xA7, 0xB7, 0xA5, 0xB7, 0x58, 0xEA}};
EFI_GUID mDataHubOptionsGuid                      = {0x0021001C, 0x3CE3, 0x41F8, {0x99, 0xC6, 0xEC, 0xF5, 0xDA, 0x75, 0x47, 0x31}};
EFI_GUID mRestartDataProtocolGiud                 = {0x03B99B90, 0xECCF, 0x451D, {0x80, 0x9E, 0x83, 0x41, 0xFC, 0xB8, 0x30, 0xAC}};
EFI_GUID mAppleUserInterfaceThemeProtocolGuid     = {0xD5B0AC65, 0x9A2D, 0x4D2A, {0xBB, 0xD6, 0xE8, 0x71, 0xA9, 0x5E, 0x04, 0x35}};
EFI_GUID mApfsEfiBootRecordInfoProtocolGuid       = {0x03B8D751, 0xA02F, 0x4FF8, {0x9B, 0x1A, 0x55, 0x24, 0xAF, 0xA3, 0x94, 0x5F}};
EFI_GUID mApplePlatformInfoDatabaseProtocolGuid   = {0xAC5E4829, 0xA8FD, 0x440B, {0xAF, 0x33, 0x9F, 0xFE, 0x01, 0x3B, 0x12, 0xD8}};

EFI_GUID mNotifyMouseActivity                     = {0xF913C2C2, 0x5351, 0x4FDB, {0x93, 0x44, 0x70, 0xFF, 0xED, 0xB8, 0x42, 0x25}};
EFI_GUID mEfiDataHubProtocolGuid                  = {0xAE80D021, 0x618E, 0x11D4, {0xBC, 0xD7, 0x00, 0x80, 0xC7, 0x3C, 0x88, 0x81}};
EFI_GUID mEfiMiscSubClassGuid                     = {0x772484B2, 0x7482, 0x4B91, {0x9F, 0x9A, 0xAD, 0x43, 0xF8, 0x1C, 0x58, 0x81}};
EFI_GUID mEfiProcessorSubClassGuid                = {0x26FDEB7E, 0xB8AF, 0x4CCF, {0xAA, 0x97, 0x02, 0x63, 0x3C, 0xE4, 0x8C, 0xA7}};
EFI_GUID mEfiCacheSubClassGuid                    = {0x7f0013A7, 0xDC79, 0x4B22, {0x80, 0x99, 0x11, 0xF7, 0x5F, 0xDC, 0x82, 0x9D}};
EFI_GUID mEfiMemorySubClassGuid                   = {0x4E8F4EBB, 0x64B9, 0x4E05, {0x9B, 0x18, 0x4C, 0xFE, 0x49, 0x23, 0x50, 0x97}};
EFI_GUID mEfiDataHubStatusCodeRecordGuid          = {0xD083E94C, 0x6560, 0x42E4, {0xB6, 0xD4, 0x2D, 0xF7, 0x5A, 0xDF, 0x6A, 0x2A}};
EFI_GUID mEfiStatusCodeRuntimeProtocolGuid        = {0xD2B2B828, 0x0826, 0x48A7, {0xB3, 0xDF, 0x98, 0x3C, 0x00, 0x60, 0x24, 0xF0}};
EFI_GUID mEfiStatusCodeGuid                       = {0xD083E94C, 0x6560, 0x42E4, {0xB6, 0xD4, 0x2D, 0xF7, 0x5A, 0xDF, 0x6A, 0x2A}};

EFI_GUID mMsgLogProtocolGuid                      = {0x511CE018, 0x0018, 0x4002, {0x20, 0x12, 0x17, 0x38, 0x05, 0x01, 0x02, 0x03}};
EFI_GUID mEfiLegacy8259ProtocolGuid               = {0x38321DBA, 0x4FE0, 0x4E17, {0x8A, 0xEC, 0x41, 0x30, 0x55, 0xEA, 0xED, 0xC1}};
EFI_GUID mEfiMemoryTypeInformationGuid            = {0x4C19049F, 0x4137, 0x4DD3, {0x9C, 0x10, 0x8B, 0x97, 0xA8, 0x3F, 0xFD, 0xFA}};
EFI_GUID mEfiSimplePointerProtocolGuid            = {0x31878C87, 0x0B75, 0x11D5, {0x9A, 0x4F, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D}};
EFI_GUID mEfiTimeVariableGuid                     = {0x9D0DA369, 0x540B, 0x46F8, {0x85, 0xA0, 0x2B, 0x5F, 0x2C, 0x30, 0x1E, 0x15}};
EFI_GUID mEfiLegacyRegionProtocolGuid             = {0x0FC9013A, 0x0568, 0x4BA9, {0x9B, 0x7E, 0xC9, 0xC3, 0x90, 0xA6, 0x60, 0x9B}};
EFI_GUID mEfiCpuArchProtocolGuid                  = {0x26BACCB1, 0x6F42, 0x11D4, {0xBC, 0xE7, 0x00, 0x80, 0xC7, 0x3C, 0x88, 0x81}};
EFI_GUID mEfiMpServiceProtocolGuid                = {0xF33261E7, 0x23CB, 0x11D5, {0xBD, 0x5C, 0x00, 0x80, 0xC7, 0x3C, 0x88, 0x81}};
EFI_GUID mEfiApplePlatformInfoGuid                = {0x64517CC8, 0x6561, 0x4051, {0xB0, 0x3C, 0x59, 0x64, 0xB6, 0x0F, 0x4C, 0x7A}};
EFI_GUID mEfiOzmosisNvramGuid                     = {0x4D1FDA02, 0x38C7, 0x4A6A, {0x9C, 0xC6, 0x4B, 0xCC, 0xA8, 0xB3, 0x01, 0x02}};
EFI_GUID mEfiWdtPersistentDataGuid                = {0x78CE2354, 0xCFBC, 0x4643, {0xAE, 0xBA, 0x07, 0xA2, 0x7F, 0xA8, 0x92, 0xBF}};
EFI_GUID mAmiDefaultsVariableGuid                 = {0x4599D26F, 0x1A11, 0x49B8, {0xB9, 0x1F, 0x85, 0x87, 0x45, 0xCF, 0xF8, 0x24}};
EFI_GUID mEfiRomImageAddressGuid                  = {0xDDE1BC72, 0xD45E, 0x4209, {0xAB, 0x85, 0x14, 0x46, 0x2D, 0x2F, 0x50, 0x74}};
EFI_GUID mAmiSetupGuid                            = {0xEC87D643, 0xEBA4, 0x4BB5, {0xA1, 0xE5, 0x3F, 0x3E, 0x36, 0xB2, 0x0D, 0xA9}};
EFI_GUID mEfiPciIoProtocolGuid                    = {0x4CF5B200, 0x68B8, 0x4CA5, {0x9E, 0xEC, 0xB2, 0x3E, 0x3F, 0x50, 0x02, 0x9A}};
EFI_GUID mEfiFirmwareVolumeProtocolGuid           = {0x389F751F, 0x1838, 0x4388, {0x83, 0x90, 0xCD, 0x81, 0x54, 0xBD, 0x27, 0xF8}};
EFI_GUID mEfiTcp4ServiceBindingProtocolGuid       = {0x00720665, 0x67EB, 0x4a99, {0xBA, 0xF7, 0xD3, 0xC3, 0x3A, 0x1C, 0x7C, 0xC9}};
EFI_GUID mEfiIp4ServiceBindingProtocolGuid        = {0xC51711E7, 0xB4BF, 0x404A, {0xBF, 0xB8, 0x0A, 0x04, 0x8E, 0xF1, 0xFF, 0xE4}};
EFI_GUID mEfiUdp4ServiceBindingProtocolGuid       = {0x83F01464, 0x99BD, 0x45E5, {0xB3, 0x83, 0xAF, 0x63, 0x05, 0xD8, 0xE9, 0xE6}};
EFI_GUID mPchInitVariableGuid                     = {0xE6C2F70A, 0xB604, 0x4877, {0x85, 0xBA, 0xDE, 0xEC, 0x89, 0xE1, 0x17, 0xEB}};
EFI_GUID mEfiMemoryRestoreDataGuid                = {0x87F22DCB, 0x7304, 0x4105, {0xBB, 0x7C, 0x31, 0x71, 0x43, 0xCC, 0xC2, 0x3B}};
EFI_GUID mEfiSbAslBufferPtrGuid                   = {0x01F33C25, 0x764D, 0x43EA, {0xAE, 0xEA, 0x6B, 0x5A, 0x41, 0xF3, 0xF3, 0xE8}};
EFI_GUID mSaPegDataVariableGuid                   = {0xC4975200, 0x64F1, 0x4FB6, {0x97, 0x73, 0xF6, 0xA9, 0xF8, 0x9D, 0x98, 0x5E}};
EFI_GUID mAmiNvramSpdMapGuid                      = {0x717FC150, 0xABD9, 0x4614, {0x80, 0x15, 0x0B, 0x33, 0x23, 0xEA, 0xB9, 0x5C}};
EFI_GUID mShellVariableGuid                       = {0x158DEF5A, 0xF656, 0x419C, {0xB0, 0x27, 0x7A, 0x31, 0x92, 0xC0, 0x79, 0xD2}};
EFI_GUID mTerminalVarGuid                         = {0x560BF58A, 0x1E0D, 0x4D7E, {0x95, 0x3F, 0x29, 0x80, 0xA2, 0x61, 0xE0, 0x31}};

//AMI UEFI
EFI_GUID mAmiTseSetupGuid               = {0xC811FA38, 0x42C8, 0x4579, {0xA9, 0xBB, 0x60, 0xE9, 0x4E, 0xDD, 0xFB, 0x34}};
EFI_GUID mAmiNetworkStackGuid           = {0xD1405D16, 0x7AFC, 0x4695, {0xBB, 0x12, 0x41, 0x45, 0x9D, 0x36, 0x95, 0xA2}};
EFI_GUID mAmiRecoveryFormSetGuid        = {0x80E1202E, 0x2697, 0x4264, {0x9C, 0xC9, 0x80, 0x76, 0x2C, 0x3E, 0x58, 0x63}};
EFI_GUID mAmiIsaDmaIrqMaskVarGuid       = {0xFC8BE767, 0x89F1, 0x4D6E, {0x80, 0x99, 0x6F, 0x02, 0x1E, 0xBC, 0x87, 0xCC}};
EFI_GUID mAmiNvRamMailboxVariableGuid   = {0x54913A6D, 0xF4EE, 0x4CDB, {0x84, 0x75, 0x74, 0x06, 0x2B, 0xFC, 0xEC, 0xF5}};
EFI_GUID mAmiTableDataVarGuid           = {0x4BAFC2B4, 0x02DC, 0x4104, {0xB2, 0x36, 0xD6, 0xF1, 0xB9, 0x8D, 0x9E, 0x84}};
EFI_GUID mAmiSecurityFormSetGuid        = {0x981CEAEE, 0x931C, 0x4A17, {0xB9, 0xC8, 0x66, 0xC7, 0xBC, 0xFD, 0x77, 0xE1}};
EFI_GUID mAmiDriverHealthCountGuid      = {0x7459A7D4, 0x6533, 0x4480, {0xBB, 0xA7, 0x79, 0xE2, 0x5A, 0x44, 0x43, 0xC9}};
EFI_GUID mAmiDriverHlthEnableGuid       = {0x0885F288, 0x418C, 0x4bE1, {0xA6, 0xAF, 0x8B, 0xAD, 0x61, 0xDA, 0x08, 0xFE}};
EFI_GUID mAmiBootManagerGuid            = {0xB4909CF3, 0x7B93, 0x4751, {0x9B, 0xD8, 0x5B, 0xA8, 0x22, 0x0B, 0x9B, 0xB2}};
EFI_GUID mAmiBootNowCountGuid           = {0x052E6EB0, 0xF240, 0x42C5, {0x83, 0x09, 0x45, 0x87, 0x45, 0x45, 0xC6, 0xB4}};
EFI_GUID mAmiBootFlowVariableGuid       = {0xEF152FB4, 0x7B2F, 0x427D, {0xBD, 0xB4, 0x7E, 0x0A, 0x05, 0x82, 0x6E, 0x64}};
EFI_GUID mAmiSystemAccessGuid           = {0xE770BB69, 0xBCB4, 0x4D04, {0x9E, 0x97, 0x23, 0xFF, 0x94, 0x56, 0xFE, 0xAC}};
EFI_GUID mAmiFastBootVariableGuid       = {0xB540A530, 0x6978, 0x4DA7, {0x91, 0xCB, 0x72, 0x07, 0xD7, 0x64, 0xD2, 0x62}};
EFI_GUID mAmiSmbiosDynamicDataGuid      = {0xE380280C, 0x4C35, 0x4AA3, {0xB9, 0x61, 0x7A, 0xE4, 0x89, 0xA2, 0xB9, 0x26}};
EFI_GUID mMeSetupInfoGuid               = {0x78259433, 0x7B6D, 0x4DB3, {0x9A, 0xE8, 0x36, 0xC4, 0xC2, 0xC3, 0xA1, 0x7D}};
EFI_GUID mMeBiosExtensionGuid           = {0x1BAD711C, 0xD451, 0x4241, {0xB1, 0xF3, 0x85, 0x37, 0x81, 0x2E, 0x0C, 0x70}};
EFI_GUID mAmiKeycodeProtocolGuid        = {0x0ADFB62D, 0xFF74, 0x484C, {0x89, 0x44, 0xF8, 0x5C, 0x4B, 0xEA, 0x87, 0xA8}};

EFI_GUID mCpuWakeUpBufferVariableGuid           = {0xDF665292, 0x79D7, 0x40E2, {0xBA, 0x51, 0xF7, 0xD4, 0x94, 0x62, 0x81, 0x85}};
EFI_GUID mSmramCpuDataVariableGuid              = {0x429501D9, 0xE447, 0x40F4, {0x86, 0x7B, 0x75, 0xC9, 0x3A, 0x1D, 0xB5, 0x4E}};
EFI_GUID mSioDevStatusVarGuid                   = {0x5820DE98, 0xFC8E, 0x4B0B, {0xA4, 0xB9, 0x0A, 0x94, 0x0D, 0x16, 0x2A, 0x7E}};
EFI_GUID mEfiIp6ServiceBindingProtocolGuid      = {0xEC835DD3, 0xFE0F, 0x617B, {0xA6, 0x21, 0xB3, 0x50, 0xC3, 0xE1, 0x33, 0x88}};
EFI_GUID mEfiUdp6ServiceBindingProtocolGuid     = {0x66ED4721, 0x3C98, 0x4D3E, {0x81, 0xE3, 0xD0, 0x3D, 0xD3, 0x9A, 0x72, 0x54}};
EFI_GUID mEfiTcp6ServiceBindingProtocolGuid     = {0xEC20EB79, 0x6C1A, 0x4664, {0x9A, 0x0D, 0xD2, 0xE4, 0xCC, 0x16, 0xD6, 0x64}};
EFI_GUID mLegacyDeviceOrderGuid                 = {0xA56074DB, 0x65FE, 0x45F7, {0xBD, 0x21, 0x2D, 0x2B, 0xDD, 0x8E, 0x96, 0x52}};
EFI_GUID mEfiFirmwareVolume2ProtocolGuid        = {0x220E73B6, 0x6BDB, 0x4413, {0x84, 0x05, 0xB9, 0x74, 0xB1, 0x08, 0x61, 0x9A}};
EFI_GUID mEfiIp4ConfigProtocolGuid              = {0x3B95AA31, 0x3793, 0x434B, {0x86, 0x67, 0xC8, 0x07, 0x08, 0x92, 0xE0, 0x5E}};
EFI_GUID mEfiIp6ConfigProtocolGuid              = {0x937FE521, 0x95AE, 0x4D1A, {0x89, 0x29, 0x48, 0xBC, 0xD9, 0x0A, 0xD3, 0x1A}};
EFI_GUID mEfiDhcp4ServiceBindingProtocolGuid    = {0x9D9A39D8, 0xBD42, 0x4A73, {0xA4, 0xD5, 0x8E, 0xE9, 0x4B, 0xE1, 0x13, 0x80}};
EFI_GUID mEfiDhcp6ServiceBindingProtocolGuid    = {0x9FB9A8A1, 0x2F4A, 0x43A6, {0x88, 0x9C, 0xD0, 0xF7, 0xB6, 0xC4, 0x7A, 0xD5}};
EFI_GUID mEfiIpSec2ProtocolGuid                 = {0xA3979E64, 0xACE8, 0x4DDC, {0xBC, 0x07, 0x4D, 0x66, 0xB8, 0xFD, 0x09, 0x77}};
EFI_GUID mEfiPerformanceProtocolGuid            = {0xFFECFFFF, 0x923C, 0x14D2, {0x9E, 0x3F, 0x22, 0xA0, 0xC9, 0x69, 0x56, 0x3B}};
EFI_GUID mEfiPciRootBridgeIoProtocolGuid        = {0x2F707EBB, 0x4A1A, 0x11D4, {0x9A, 0x38, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D}};
EFI_GUID mMtcVendorGuid                         = {0xEB704011, 0x1402, 0x11D3, {0x8E, 0x77, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}};
EFI_GUID mEfiUsbIoProtocolGuid                  = {0x2B2F68D6, 0x0CD2, 0x44CF, {0x8E, 0x8B, 0xBB, 0xA2, 0x0B, 0x1B, 0x5B, 0x75}};
EFI_GUID mEfiUsb2HcProtocolGuid                 = {0x3E745226, 0x9818, 0x45B6, {0xA2, 0xAC, 0xD7, 0xCD, 0x0E, 0x8B, 0xA2, 0xBC}};
EFI_GUID mEfiUsbHcProtocolGuid                  = {0xF5089266, 0x1AA0, 0x4953, {0x97, 0xD8, 0x56, 0x2F, 0x8A, 0x73, 0xB5, 0x19}};
EFI_GUID mEfiIsaIoProtocolGuid                  = {0x7EE2BD44, 0x3DA0, 0x11D4, {0x9A, 0x38, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D}};
EFI_GUID mEfiUsbAtapiProtocolGuid               = {0x2B2F68DA, 0x0CD2, 0x44CF, {0x8E, 0x8B, 0xBB, 0xA2, 0x0B, 0x1B, 0x5B, 0x75}};
EFI_GUID mEfiSimpleTextOutputProtocolGuid       = {0x387477C2, 0x69C7, 0x11D2, {0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}};
EFI_GUID mEfiSimpleTextInputProtocolGuid        = {0x387477C1, 0x69C7, 0x11D2, {0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}};
EFI_GUID mEfiSimpleTextInputExProtocolGuid      = {0xDD9E7534, 0x7762, 0x4698, {0x8C, 0x14, 0xF5, 0x85, 0x17, 0xA6, 0x25, 0xAA}};
EFI_GUID mEfiConsoleInDeviceGuid                = {0xD3B36F2B, 0xD551, 0x11D4, {0x9A, 0x46, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D}};
EFI_GUID mEfiConsoleOutDeviceGuid               = {0xD3B36F2C, 0xD551, 0x11D4, {0x9A, 0x46, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D}};
EFI_GUID mEfiStandardErrorDeviceGuid            = {0xD3B36F2D, 0xd551, 0x11D4, {0x9A, 0x46, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D}};
EFI_GUID mEfiArpServiceBindingProtocolGuid      = {0xF44C00EE, 0x1F2C, 0x4A00, {0xAA, 0x09, 0x1C, 0x9F, 0x3E, 0x08, 0x00, 0xA3}};
EFI_GUID mEfiSerialIoProtocolGuid               = {0xBB25CF6F, 0xF1D4, 0x11D2, {0x9A, 0x0C, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0xFD}};
EFI_GUID mEfiIsaAcpiProtocolGuid                = {0x64A892DC, 0x5561, 0x4536, {0x92, 0xC7, 0x79, 0x9B, 0xFC, 0x18, 0x33, 0x55}};
EFI_GUID mEfiIdeControllerInitProtocolGuid      = {0xA1E37052, 0x80D9, 0x4E65, {0xA3, 0x17, 0x3E, 0x9A, 0x55, 0xC4, 0x3E, 0xC9}};
EFI_GUID mEfiDriverBindingProtGuid              = {0x18A031AB, 0xB443, 0x4D1A, {0xA5, 0xC0, 0x0C, 0x09, 0x26, 0x1E, 0x9F, 0x71}};
EFI_GUID mEfiPciPlatformProtocolGuid            = {0x07D75280, 0x27D4, 0x4D69, {0x90, 0xD0, 0x56, 0x43, 0xE2, 0x38, 0xB3, 0x41}};
EFI_GUID mEfiUnicodeCollationProtocolGuid       = {0x1D85CD7F, 0xF43D, 0x11D2, {0x9A, 0x0C, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D}};
EFI_GUID mEfiGenericMemTestProtocolGuid         = {0x309DE7F1, 0x7F5E, 0x4ACE, {0xB4, 0x9C, 0x53, 0x1B, 0xE5, 0xAA, 0x95, 0xEF}};
EFI_GUID mEfiHashProtocolGuid                   = {0xc5184932, 0xdba5, 0x46db, {0xa5, 0xba, 0xcc, 0x0b, 0xda, 0x9c, 0x14, 0x35}};
EFI_GUID mEfiHashServiceBindingProtocolGuid     = {0x42881c98, 0xa4f3, 0x44b0, {0xa3, 0x9d, 0xdf, 0xa1, 0x86, 0x67, 0xd8, 0xcd}};
EFI_GUID mEfiMtftp4ServiceBindingProtocolGuid   = {0x2FE800BE, 0x8F01, 0x4AA6, {0x94, 0x6B, 0xD7, 0x13, 0x88, 0xE1, 0x83, 0x3F}};
EFI_GUID mEfiSimpleNetworkProtocolGuid          = {0xA19832B9, 0xAC25, 0x11D3, {0x9A, 0x2D, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D}};
EFI_GUID mEfiManagedNetworkServiceBindingProtocolGuid   = {0xF36FF770, 0xA7E1, 0x42CF, {0x9E, 0xD2, 0x56, 0xF0, 0xF2, 0x71, 0xF4, 0x4C}};
EFI_GUID mEfiNetworkInterfaceIdentifierProtocolGuid_31  = {0x1ACED566, 0x76ED, 0x4218, {0xBC, 0x81, 0x76, 0x7F, 0x1F, 0x97, 0x7A, 0x89}};
EFI_GUID mEfiNetworkInterfaceIdentifierProtocolGuid     = {0xE18541CD, 0xF755, 0x4f73, {0x92, 0x8D, 0x64, 0x3C, 0x8A, 0x79, 0xB2, 0x29}};
EFI_GUID mEfiIncompatiblePciDeviceSupportProtocolGuid   = {0xEB23F55A, 0x7863, 0x4AC2, {0x8D, 0x3D, 0x95, 0x65, 0x35, 0xDE, 0x03, 0x75}};

EFI_GUID mEfiHdaControllerInfoProtocolGuid      = { 0xE5FC2CAF, 0x0291, 0x46F2, { 0x87, 0xF8, 0x10, 0xC7, 0x58, 0x72, 0x58, 0x04}};
EFI_GUID mEfiHdaIoProtocolGuid                  = { 0xA090D7F9, 0xB50A, 0x4EA1, { 0xBD, 0xE9, 0x1A, 0xA5, 0xE9, 0x81, 0x2F, 0x45}};
EFI_GUID mEfiHdaCodecInfoProtocolGuid           = { 0x6C9CDDE1, 0xE8A5, 0x43E5, { 0xBE, 0x88, 0xDA, 0x15, 0xBC, 0x1C, 0x02, 0x50}};
EFI_GUID mEfiAudioIoProtocolGuid                = { 0xF05B559C, 0x1971, 0x4AF5, { 0xB2, 0xAE, 0xD6, 0x08, 0x08, 0xF7, 0x4F, 0x70}};

// From MacOsxBootloader source:
EFI_GUID mAppleAcpiVariableGuid           = {0xAF9FFD67, 0xEC10, 0x488A, {0x9D, 0xFC, 0x6C, 0xBF, 0x5E, 0xE2, 0x2C, 0x2E}};
EFI_GUID mAppleFileVaultVariableGuid      = {0x8D63D4FE, 0xBD3C, 0x4AAD, {0x88, 0x1D, 0x86, 0xFD, 0x97, 0x4B, 0xC1, 0xDF}};
EFI_GUID mApplePasswordUIEfiFileNameGuid  = {0x9EBA2D25, 0xBBE3, 0x4AC2, {0xA2, 0xC6, 0xC8, 0x7F, 0x44, 0xA1, 0x27, 0x8C}};
EFI_GUID mAppleRamDmgDevicePathGuid       = {0x040B07E8, 0x0B9C, 0x427E, {0xB0, 0xD4, 0xA4, 0x66, 0xE6, 0xE5, 0x7A, 0x62}};
EFI_GUID mAppleSMCProtocolGuid            = {0x17407E5A, 0xAF6C, 0x4EE8, {0x98, 0xA8, 0x00, 0x21, 0x04, 0x53, 0xCD, 0xD9}};
EFI_GUID mAppleFireWireProtocolGuid       = {0x67708AA8, 0x2079, 0x4E4F, {0xB1, 0x58, 0xB1, 0x5B, 0x1F, 0x6A, 0x6C, 0x92}};
EFI_GUID mAppleDeviceControlProtocolGuid  = {0x8ECE08D8, 0xA6D4, 0x430B, {0xA7, 0xB0, 0x2D, 0xF3, 0x18, 0xE7, 0x88, 0x4A}};
EFI_GUID mAppleDiskIoProtocolGuid         = {0x5B27263B, 0x9083, 0x415E, {0x88, 0x9E, 0x64, 0x32, 0xCA, 0xA9, 0xB8, 0x13}};
EFI_GUID mEfiOSInfoProtocolGuid           = {0xC5C5DA95, 0x7D5C, 0x45E6, {0xB2, 0xF1, 0x3F, 0xD5, 0x2B, 0xB1, 0x00, 0x77}};
EFI_GUID mAppleGraphConfigProtocolGuid    = {0x03622D6D, 0x362A, 0x4E47, {0x97, 0x10, 0xC2, 0x38, 0xB2, 0x37, 0x55, 0xC1}};
EFI_GUID mAppleEventProtocolGuid          = {0x33BE0EF1, 0x89C9, 0x4A6D, {0xBB, 0x9F, 0x69, 0xDC, 0x8D, 0xD5, 0x16, 0xB9}};

EFI_GUID mAppleSMCStateProtocolGuid       = {0x5301FE59, 0x1770, 0x4166, {0xA1, 0x69, 0x00, 0xC4, 0xBD, 0xE4, 0xA1, 0x62}};
EFI_GUID mAppleKeyMapDatabaseProtocolGuid = {0x584B9EBE, 0x80C1, 0x4BD6, {0x98, 0xB0, 0xA7, 0x78, 0x6E, 0xC2, 0xF2, 0xE2}};

// Shell guids
EFI_GUID ShellInt           = {0x47C7B223, 0xC42A, 0x11D2, {0x8E, 0x57, 0x0, 0xA0, 0xC9, 0x69, 0x72, 0x3B}};
EFI_GUID SEnv               = {0x47C7B224, 0xC42A, 0x11D2, {0x8E, 0x57, 0x0, 0xA0, 0xC9, 0x69, 0x72, 0x3B}};
EFI_GUID ShellDevPathMap    = {0x47C7B225, 0xC42A, 0x11D2, {0x8E, 0x57, 0x0, 0xA0, 0xC9, 0x69, 0x72, 0x3B}};
EFI_GUID ShellProtId        = {0x47C7B226, 0xC42A, 0x11D2, {0x8E, 0x57, 0x0, 0xA0, 0xC9, 0x69, 0x72, 0x3B}};
EFI_GUID ShellAlias         = {0x47C7B227, 0xC42A, 0x11D2, {0x8E, 0x57, 0x0, 0xA0, 0xC9, 0x69, 0x72, 0x3B}};

/** Map of known guids and friendly names. Searchable with GuidStr(). */
MAP_EFI_GUID_STR EfiGuidStrMap[] = {
  {NULL, L"Tmp buffer AE074D26-6E9E-11E1-A5B8-9BFC4824019B"},
  {&gEfiFileInfoGuid, L"gEfiFileInfoGuid"},
  {&gEfiFileSystemInfoGuid, L"gEfiFileSystemInfoGuid"},
  {&gEfiFileSystemVolumeLabelInfoIdGuid, L"gEfiFileSystemVolumeLabelInfoIdGuid"},
  {&gEfiLoadedImageProtocolGuid, L"gEfiLoadedImageProtocolGuid"},
  {&gEfiDevicePathProtocolGuid, L"gEfiDevicePathProtocolGuid"},
  {&gEfiSimpleFileSystemProtocolGuid, L"gEfiSimpleFileSystemProtocolGuid"},
  {&gEfiBlockIoProtocolGuid, L"gEfiBlockIoProtocolGuid"},
  {&gEfiBlockIo2ProtocolGuid, L"gEfiBlockIo2ProtocolGuid"},
  {&gEfiDiskIoProtocolGuid, L"gEfiDiskIoProtocolGuid"},
  {&gEfiDiskIo2ProtocolGuid, L"gEfiDiskIo2ProtocolGuid"},
  {&gEfiGraphicsOutputProtocolGuid, L"gEfiGraphicsOutputProtocolGuid"},
  
  {&mEfiHdaControllerInfoProtocolGuid, L"gEfiHdaControllerInfoProtocolGuid"},
  {&mEfiHdaIoProtocolGuid, L"gEfiHdaIoProtocolGuid"},
  {&mEfiHdaCodecInfoProtocolGuid, L"gEfiHdaCodecInfoProtocolGuid"},
  {&mEfiAudioIoProtocolGuid, L"gEfiAudioIoProtocolGuid"},
  
  {&mEfiConsoleControlProtocolGuid, L"gEfiConsoleControlProtocolGuid"},
  {&mAppleFirmwarePasswordProtocolGuid, L"gAppleFirmwarePasswordProtocolGuid"},
  {&mEfiGlobalVariableGuid, L"gEfiGlobalVariableGuid"},
  {&mAppleDevicePropertyProtocolGuid, L"gAppleDevicePropertyProtocolGuid"},
  {&mEfiAppleBootGuid, L"gEfiAppleBootGuid"},
  {&mEfiAppleNvramGuid, L"gEfiAppleNvramGuid"},
  {&mAppleFramebufferInfoProtocolGuid, L"gAppleFramebufferInfoProtocolGuid"},
  {&mAppleKeyStateProtocolGuid, L"gAppleKeyStateProtocolGuid"},
  {&mAppleNetBootProtocolGuid, L"gAppleNetBootProtocolGuid"},
  {&mAppleImageCodecProtocolGuid, L"gAppleImageCodecProtocolGuid"},
  {&mAppleEfiVendorGuid, L"gAppleEfiVendorGuid"},
  {&mAppleEFINVRAMTRBSecureGuid, L"gAppleEFINVRAMTRBSecureGuid"},
  {&mDataHubOptionsGuid, L"gDataHubOptionsGuid"},
  {&mRestartDataProtocolGiud, L"gRestartDataProtocolGiud"},
  {&mAppleUserInterfaceThemeProtocolGuid, L"gAppleUserInterfaceThemeProtocolGuid"},
  {&mApfsEfiBootRecordInfoProtocolGuid, L"gApfsEfiBootRecordInfoProtocolGuid"},
  {&mApplePlatformInfoDatabaseProtocolGuid, L"gApplePlatformInfoDatabaseProtocolGuid"},
  {&mAppleSMCStateProtocolGuid, L"gAppleSMCStateProtocolGuid"},
  {&mAppleKeyMapDatabaseProtocolGuid, L"gAppleKeyMapDatabaseProtocolGuid"},
  
  {&mNotifyMouseActivity, L"gNotifyMouseActivity"},
  {&mEfiDataHubProtocolGuid, L"gEfiDataHubProtocolGuid"},
  {&mEfiMiscSubClassGuid, L"gEfiMiscSubClassGuid"},
  {&mEfiProcessorSubClassGuid, L"gEfiProcessorSubClassGuid"},
  {&mEfiMemorySubClassGuid, L"gEfiMemorySubClassGuid"},
  {&mEfiCacheSubClassGuid, L"gEfiCacheSubClassGuid"},
  {&mMsgLogProtocolGuid, L"gMsgLogProtocolGuid"},
  {&mEfiLegacy8259ProtocolGuid, L"gEfiLegacy8259ProtocolGuid"},
  {&mEfiDataHubStatusCodeRecordGuid, L"gEfiDataHubStatusCodeRecordGuid"},
  {&mEfiStatusCodeRuntimeProtocolGuid, L"gEfiStatusCodeRuntimeProtocolGuid"},
  {&mEfiStatusCodeGuid, L"gEfiStatusCodeGuid"},
  
  {&gEfiAcpi10TableGuid, L"gEfiAcpi10TableGuid"},
  {&gEfiAcpi20TableGuid, L"gEfiAcpi20TableGuid"},
  {&gEfiDebugImageInfoTableGuid, L"gEfiDebugImageInfoTableGuid"},
  {&gEfiDxeServicesTableGuid, L"gEfiDxeServicesTableGuid"},
  {&gEfiHobListGuid, L"gEfiHobListGuid"},
  {&gEfiMpsTableGuid, L"gEfiMpsTableGuid"},
  {&gEfiSmbiosTableGuid, L"gEfiSmbiosTableGuid"},
  {&gEfiSmbios3TableGuid, L"gEfiSmbios3TableGuid"},
  
  {&mEfiMemoryTypeInformationGuid, L"gEfiMemoryTypeInformationGuid"},
  {&mEfiSimplePointerProtocolGuid, L"gEfiSimplePointerProtocolGuid"},
  {&mEfiTimeVariableGuid, L"gEfiTimeVariableGuid"},
  {&mEfiLegacyRegionProtocolGuid, L"gEfiLegacyRegionProtocolGuid"},
  {&mEfiCpuArchProtocolGuid, L"gEfiCpuArchProtocolGuid"},
  {&mEfiMpServiceProtocolGuid, L"gEfiMpServiceProtocolGuid"},
  {&mEfiApplePlatformInfoGuid, L"gEfiApplePlatformInfoGuid"},
  {&mEfiOzmosisNvramGuid, L"gEfiOzmosisNvramGuid"},
  {&mEfiWdtPersistentDataGuid, L"gEfiWdtPersistentDataGuid"},
  
  {&mAmiDefaultsVariableGuid, L"gAmiDefaultsVariableGuid"},
  {&mEfiRomImageAddressGuid, L"gEfiRomImageAddressGuid"},
  {&mAmiSetupGuid, L"gAmiSetupGuid"},
  {&mEfiPciIoProtocolGuid, L"gEfiPciIoProtocolGuid"},
  {&mEfiFirmwareVolumeProtocolGuid, L"gEfiFirmwareVolumeProtocolGuid"},
  {&mEfiTcp4ServiceBindingProtocolGuid, L"gEfiTcp4ServiceBindingProtocolGuid"},
  {&mEfiIp4ServiceBindingProtocolGuid, L"gEfiIp4ServiceBindingProtocolGuid"},
  {&mEfiUdp4ServiceBindingProtocolGuid, L"gEfiUdp4ServiceBindingProtocolGuid"},
  {&mPchInitVariableGuid, L"gPchInitVariableGuid"},
  {&mEfiMemoryRestoreDataGuid, L"gEfiMemoryRestoreDataGuid"},
  {&mEfiSbAslBufferPtrGuid, L"gEfiSbAslBufferPtrGuid"},
  {&mSaPegDataVariableGuid, L"gSaPegDataVariableGuid"},
  {&mAmiNvramSpdMapGuid, L"gAmiNvramSpdMapGuid"},
  {&mShellVariableGuid, L"gShellVariableGuid"},
  {&mTerminalVarGuid, L"gTerminalVarGuid"},
  {&mAmiTseSetupGuid, L"gAmiTseSetupGuid"},
  {&mAmiNetworkStackGuid, L"gAmiNetworkStackGuid"},
  {&mAmiRecoveryFormSetGuid, L"gAmiRecoveryFormSetGuid"},
  {&mAmiIsaDmaIrqMaskVarGuid, L"gAmiIsaDmaIrqMaskVarGuid"},
  {&mAmiNvRamMailboxVariableGuid, L"gAmiNvRamMailboxVariableGuid"},
  {&mAmiTableDataVarGuid, L"gAmiTableDataVarGuid"},
  {&mCpuWakeUpBufferVariableGuid, L"gCpuWakeUpBufferVariableGuid"},
  {&mMeSetupInfoGuid, L"gMeSetupInfoGuid"},
  {&mMeBiosExtensionGuid, L"gMeBiosExtensionGuid"},
  {&mAmiSecurityFormSetGuid, L"gAmiSecurityFormSetGuid"},
  {&mAmiDriverHealthCountGuid, L"gAmiDriverHealthCountGuid"},
  {&mAmiDriverHlthEnableGuid, L"gAmiDriverHlthEnableGuid"},
  {&mSioDevStatusVarGuid, L"gSioDevStatusVarGuid"},
  {&mEfiIp6ServiceBindingProtocolGuid, L"gEfiIp6ServiceBindingProtocolGuid"},
  {&mEfiUdp6ServiceBindingProtocolGuid, L"gEfiUdp6ServiceBindingProtocolGuid"},
  {&mEfiTcp6ServiceBindingProtocolGuid, L"gEfiTcp6ServiceBindingProtocolGuid"},
  {&mSmramCpuDataVariableGuid, L"gSmramCpuDataVariableGuid"},
  {&mLegacyDeviceOrderGuid, L"gLegacyDeviceOrderGuid"},
  {&mAmiBootManagerGuid, L"gAmiBootManagerGuid"},
  {&mAmiBootNowCountGuid, L"gAmiBootNowCountGuid"},
  {&mAmiBootFlowVariableGuid, L"gAmiBootFlowVariableGuid"},
  {&mAmiSystemAccessGuid, L"gAmiSystemAccessGuid"},
  {&mAmiFastBootVariableGuid, L"gAmiFastBootVariableGuid"},
  {&mAmiSmbiosDynamicDataGuid, L"gAmiSmbiosDynamicDataGuid"},
  {&mAmiKeycodeProtocolGuid, L"gAmiKeycodeProtocolGuid"},
  
  {&mEfiFirmwareVolume2ProtocolGuid, L"gEfiFirmwareVolume2ProtocolGuid"},
  {&mEfiIp4ConfigProtocolGuid, L"gEfiIp4ConfigProtocolGuid"},
  {&mEfiIp6ConfigProtocolGuid, L"gEfiIp6ConfigProtocolGuid"},
  {&mEfiDhcp4ServiceBindingProtocolGuid, L"gEfiDhcp4ServiceBindingProtocolGuid"},
  {&mEfiDhcp6ServiceBindingProtocolGuid, L"gEfiDhcp6ServiceBindingProtocolGuid"},
  {&mEfiIpSec2ProtocolGuid, L"gEfiIpSec2ProtocolGuid"},
  {&mEfiPerformanceProtocolGuid, L"gEfiPerformanceProtocolGuid"},
  {&mEfiPciRootBridgeIoProtocolGuid, L"gEfiPciRootBridgeIoProtocolGuid"},
  {&mMtcVendorGuid, L"gMtcVendorGuid"},
  {&mEfiUsbIoProtocolGuid, L"gEfiUsbIoProtocolGuid"},
  {&mEfiUsb2HcProtocolGuid, L"gEfiUsb2HcProtocolGuid"},
  {&mEfiUsbHcProtocolGuid, L"gEfiUsbHcProtocolGuid"},
  {&mEfiIsaIoProtocolGuid, L"gEfiIsaIoProtocolGuid"},
  {&mEfiUsbAtapiProtocolGuid, L"gEfiUsbAtapiProtocolGuid"},
  {&mEfiSimpleTextOutputProtocolGuid, L"gEfiSimpleTextOutProtocolGuid"},
  {&mEfiSimpleTextInputProtocolGuid, L"gEfiSimpleTextInProtocolGuid"},
  {&mEfiSimpleTextInputExProtocolGuid, L"gEfiSimpleTextInExProtocolGuid"},
  {&mEfiConsoleInDeviceGuid, L"gEfiConsoleInDeviceGuid"},
  {&mEfiConsoleOutDeviceGuid, L"gEfiConsoleOutDeviceGuid"},
  {&mEfiStandardErrorDeviceGuid, L"gEfiStandardErrorDeviceGuid"},
  {&mEfiManagedNetworkServiceBindingProtocolGuid, L"gEfiManagedNetworkServiceBindingProtocolGuid"},
  {&mEfiSimpleNetworkProtocolGuid, L"gEfiSimpleNetworkProtocolGuid"},
  {&mEfiArpServiceBindingProtocolGuid, L"gEfiArpServiceBindingProtocolGuid"},
  {&mEfiNetworkInterfaceIdentifierProtocolGuid_31, L"gEfiNetworkInterfaceIdentifierProtocolGuid_31"},
  {&mEfiNetworkInterfaceIdentifierProtocolGuid, L"gEfiNetworkInterfaceIdentifierProtocolGuid"},
  {&mEfiSerialIoProtocolGuid, L"gEfiSerialIoProtocolGuid"},
  {&mEfiIsaAcpiProtocolGuid, L"gEfiIsaAcpiProtocolGuid"},
  {&mEfiIdeControllerInitProtocolGuid, L"gEfiIdeControllerInitProtocolGuid"},
  {&mEfiDriverBindingProtGuid, L"gEfiDriverBindingProtocolGuid"},
  {&mEfiIncompatiblePciDeviceSupportProtocolGuid, L"gEfiIncompatiblePciDeviceSupportProtocolGuid"},
  {&mEfiPciPlatformProtocolGuid, L"gEfiPciPlatformProtocolGuid"},
  {&mEfiMtftp4ServiceBindingProtocolGuid, L"gEfiMtftp4ServiceBindingProtocolGuid"},
  {&mEfiUnicodeCollationProtocolGuid, L"gEfiUnicodeCollationProtocolGuid"},
  {&mEfiGenericMemTestProtocolGuid, L"gEfiGenericMemTestProtocolGuid"},
  {&mEfiHashProtocolGuid, L"gEfiHashProtocolGuid"},
  {&mEfiHashServiceBindingProtocolGuid, L"gEfiHashServiceBindingProtocolGuid"},
  
  
  {&mAppleAcpiVariableGuid, L"gAppleAcpiVariableGuid"},
  {&mAppleFileVaultVariableGuid, L"gAppleFileVaultVariableGuid"},
  {&mApplePasswordUIEfiFileNameGuid, L"gApplePasswordUIEfiFileNameGuid"},
  {&mAppleRamDmgDevicePathGuid, L"gAppleRamDmgDevicePathGuid"},
  {&mAppleSMCProtocolGuid, L"gAppleSMCProtocolGuid"},
  {&mAppleFireWireProtocolGuid, L"gAppleFireWireProtocolGuid"},
  {&mAppleDeviceControlProtocolGuid, L"gAppleDeviceControlProtocolGuid"},
  {&mAppleDiskIoProtocolGuid, L"gAppleDiskIoProtocolGuid"},
  {&mEfiOSInfoProtocolGuid, L"gEfiOSInfoProtocolGuid"},
  {&mAppleGraphConfigProtocolGuid, L"gAppleGraphConfigProtocolGuid"},
  {&mAppleEventProtocolGuid, L"gAppleEventProtocolGuid"},
  
  {&ShellInt, L"ShellInt"},
  {&SEnv, L"SEnv"},
  {&ShellDevPathMap, L"ShellDevPathMap"},
  {&ShellProtId, L"ShellProtId"},
  {&ShellAlias, L"ShellAlias"},
  
  {NULL, NULL}
};


/** Print buffer for unknown GUID printing. Allocated as RT mem
 *  and gets converted in RuntimeServices.c VirtualAddressChangeEvent().
 */
CHAR16  *GuidPrintBuffer = NULL;
#define GUID_PRINT_BUFFER_SIZE    ((36+1) * sizeof(CHAR16))
//CHAR16  GuidPrintBuffer[40];

/** Buffer for RT variable names. Allocated as RT mem
 *  and gets converted in RuntimeServices.c VirtualAddressChangeEvent().
 */
CHAR16 *gVariableNameBuffer = NULL;
#define VARIABLE_NAME_BUFFER_SIZE  1024

/** Buffer for RT variable data. Allocated as RT mem
 *  and gets converted in RuntimeServices.c VirtualAddressChangeEvent().
 */
CHAR8 *gVariableDataBuffer = NULL;
#define VARIABLE_DATA_BUFFER_SIZE  (64 * 1024)


/** Returns GUID as string, with friendly name for known guids. */
CHAR16*
EFIAPI
GuidStr(IN EFI_GUID *Guid)
{
  UINTN  i;
  CHAR16  *Str = NULL;
  
  if (GuidPrintBuffer == NULL) {
    GuidPrintBuffer = AllocateRuntimePool(GUID_PRINT_BUFFER_SIZE);
  }
  
  for(i = 1; EfiGuidStrMap[i].Guid != NULL; i++) {
    if (CompareGuid(EfiGuidStrMap[i].Guid, Guid)) {
      Str = EfiGuidStrMap[i].Str;
      break;
    }
  }
  if (Str == NULL) {
    UnicodeSPrint(GuidPrintBuffer, GUID_PRINT_BUFFER_SIZE, L"%g", Guid);
    Str = GuidPrintBuffer;
  }
  return Str;
}


/** Prints Number of bytes in a row (hex and ascii). Row size is MaxNumber. */
VOID
EFIAPI
PrintBytesRow(IN CHAR8 *Bytes, IN UINTN Number, IN UINTN MaxNumber)
{
  UINTN  Index;
  
  // print hex vals
  for (Index = 0; Index < Number; Index++) {
    PRINT("%02x ", (UINT8)Bytes[Index]);
  }
  
  // pad to MaxNumber if needed
  for (; Index < MaxNumber; Index++) {
    PRINT("   ");
  }
  
  PRINT("| ");
  
  // print ASCII
  for (Index = 0; Index < Number; Index++) {
    if (Bytes[Index] >= 0x20 && Bytes[Index] <= 0x7e) {
      PRINT("%c", (CHAR16)Bytes[Index]);
    } else {
      PRINT("%c", L'.');
    }
  }
  
  PRINT("\n");
}

/** Prints series of bytes. */
VOID
EFIAPI
PrintBytes(IN CHAR8 *Bytes, IN UINTN Number)
{
  UINTN  Index;
  
  for (Index = 0; Index < Number; Index += 16) {
    PrintBytesRow(Bytes + Index, (Index + 16 < Number ? 16 : Number - Index), 16);
  }
}

/** Returns pointer to last Char in String or NULL. */
CHAR16*
EFIAPI
GetStrLastChar(IN CHAR16 *String)
{
  CHAR16  *Pos;
  
  if (String == NULL || *String == L'\0') {
    return NULL;
  }
  
  // go to end
  Pos = String;
  while (*Pos != L'\0') {
    Pos++;
  }
  Pos--;
  return Pos;
}

/** Returns pointer to last occurence of Char in String or NULL. */
CHAR16*
EFIAPI
GetStrLastCharOccurence(IN CHAR16 *String, IN CHAR16 Char)
{
  CHAR16  *Pos;
  
  if (String == NULL || *String == L'\0') {
    return NULL;
  }
  
  // go to end
  Pos = String;
  while (*Pos != L'\0') {
    Pos++;
  }
  // search for Char
  while (*Pos != Char && Pos != String) {
    Pos--;
  }
  return (*Pos == Char) ? Pos : NULL;
}

/** Returns upper case version of char - valid only for ASCII chars in unicode. */
CHAR16
EFIAPI
ToUpperChar(IN CHAR16 Chr)
{
  CHAR8  C;
  
  if (Chr > 0xFF) return Chr;
  C = (CHAR8)Chr;
  return ((C >= 'a' && C <= 'z') ? C - ('a' - 'A') : C);
}


/** Returns 0 if two strings are equal, !=0 otherwise. Compares just first 8 bits of chars (valid for ASCII), case insensitive.. */
UINTN
EFIAPI
StrCmpiBasic(IN CHAR16 *String1, IN CHAR16 *String2)
{
  CHAR16  Chr1;
  CHAR16  Chr2;
  
  if (String1 == NULL || String2 == NULL) {
    return 1;
  }
  if (*String1 == L'\0' && *String2 == L'\0') {
    return 0;
  }
  if (*String1 == L'\0' || *String2 == L'\0') {
    return 1;
  }
  
  Chr1 = ToUpperChar(*String1);
  Chr2 = ToUpperChar(*String2);
  while ((*String1 != L'\0') && (Chr1 == Chr2)) {
    String1++;
    String2++;
    Chr1 = ToUpperChar(*String1);
    Chr2 = ToUpperChar(*String2);
  }
  
  return Chr1 - Chr2;
}

/** Returns the first occurrence of a Null-terminated Unicode SearchString
 * in a Null-terminated Unicode String.
 * Compares just first 8 bits of chars (valid for ASCII), case insensitive.
 * Copied from MdePkg/Library/BaseLib/String.c and modified
 */
CHAR16*
EFIAPI
StrStriBasic(
             IN CONST CHAR16      *String,
             IN CONST CHAR16      *SearchString
             )
{
  CONST CHAR16      *FirstMatch;
  CONST CHAR16      *SearchStringTmp;
  
  
  if (*SearchString == L'\0') {
    return (CHAR16 *) String;
  }
  
  while (*String != L'\0') {
    SearchStringTmp = SearchString;
    FirstMatch = String;
    
    while ((ToUpperChar(*String) == ToUpperChar(*SearchStringTmp))
           && (*String != L'\0'))
    {
      String++;
      SearchStringTmp++;
    }
    
    if (*SearchStringTmp == L'\0') {
      return (CHAR16 *) FirstMatch;
    }
    
    if (*String == L'\0') {
      return NULL;
    }
    
    String = FirstMatch + 1;
  }
  
  return NULL;
}

/** Returns TRUE if String1 starts with String2, FALSE otherwise. Compares just first 8 bits of chars (valid for ASCII), case insensitive.. */
BOOLEAN
EFIAPI
StriStartsWithBasic(IN CHAR16 *String1, IN CHAR16 *String2)
{
  CHAR16  Chr1;
  CHAR16  Chr2;
  BOOLEAN Result;
  
  if (String1 == NULL || String2 == NULL) {
    return FALSE;
  }
  if (*String1 == L'\0' && *String2 == L'\0') {
    return TRUE;
  }
  if (*String1 == L'\0' || *String2 == L'\0') {
    return FALSE;
  }
  
  Chr1 = ToUpperChar(*String1);
  Chr2 = ToUpperChar(*String2);
  while ((Chr1 != L'\0') && (Chr2 != L'\0') && (Chr1 == Chr2)) {
    String1++;
    String2++;
    Chr1 = ToUpperChar(*String1);
    Chr2 = ToUpperChar(*String2);
  }
  
  Result = ((Chr1 == L'\0') && (Chr2 == L'\0'))
  || ((Chr1 != L'\0') && (Chr2 == L'\0'));
  
  return Result;
}

VOID EFIAPI
ShrinkMemMap(
             IN UINTN      *MemoryMapSize,
             IN EFI_MEMORY_DESCRIPTOR  *MemoryMap,
             IN UINTN      DescriptorSize,
             IN UINT32      DescriptorVersion
             )
{
  UINTN        SizeFromDescToEnd;
  UINT64        Bytes;
  EFI_MEMORY_DESCRIPTOR    *PrevDesc;
  EFI_MEMORY_DESCRIPTOR    *Desc;
  BOOLEAN        CanBeJoined;
  BOOLEAN        HasEntriesToRemove;
  
  PrevDesc = MemoryMap;
  Desc = NEXT_MEMORY_DESCRIPTOR(PrevDesc, DescriptorSize);
  SizeFromDescToEnd = *MemoryMapSize - DescriptorSize;
  *MemoryMapSize = DescriptorSize;
  HasEntriesToRemove = FALSE;
  while (SizeFromDescToEnd > 0) {
    Bytes = (((UINTN) PrevDesc->NumberOfPages) * EFI_PAGE_SIZE);
    CanBeJoined = FALSE;
    if ((Desc->Attribute == PrevDesc->Attribute) && (PrevDesc->PhysicalStart + Bytes == Desc->PhysicalStart)) {
      if (Desc->Type == EfiBootServicesCode
          || Desc->Type == EfiBootServicesData
          //|| Desc->Type == EfiConventionalMemory
          //|| Desc->Type == EfiLoaderCode
          //|| Desc->Type == EfiLoaderData
          )
      {
        CanBeJoined = PrevDesc->Type == EfiBootServicesCode
        || PrevDesc->Type == EfiBootServicesData
        //|| PrevDesc->Type == EfiConventionalMemory
        //|| PrevDesc->Type == EfiLoaderCode
        //|| PrevDesc->Type == EfiLoaderData
        ;
      }
    }
    
    if (CanBeJoined) {
      // two entries are the same/similar - join them
      PrevDesc->NumberOfPages += Desc->NumberOfPages;
      HasEntriesToRemove = TRUE;
    } else {
      // can not be joined - we need to move to next
      *MemoryMapSize += DescriptorSize;
      PrevDesc = NEXT_MEMORY_DESCRIPTOR(PrevDesc, DescriptorSize);
      if (HasEntriesToRemove) {
        // have entries between PrevDesc and Desc which are joined to PrevDesc
        // we need to copy [Desc, end of list] to PrevDesc + 1
        CopyMem(PrevDesc, Desc, SizeFromDescToEnd);
        Desc = PrevDesc;
      }
      HasEntriesToRemove = FALSE;
    }
    // move to next
    Desc = NEXT_MEMORY_DESCRIPTOR(Desc, DescriptorSize);
    SizeFromDescToEnd -= DescriptorSize;
  }
}

VOID EFIAPI
PrintMemMap(
            IN UINTN      MemoryMapSize,
            IN EFI_MEMORY_DESCRIPTOR  *MemoryMap,
            IN UINTN      DescriptorSize,
            IN UINT32      DescriptorVersion
            )
{
  UINTN        NumEntries;
  UINTN        Index;
  UINT64        Bytes;
  EFI_MEMORY_DESCRIPTOR    *Desc;
  
  Desc = MemoryMap;
  NumEntries = MemoryMapSize / DescriptorSize;
  PRINT("MEMMAP: Size=%d, Addr=%p, DescSize=%d, DescVersion: 0x%x\n", MemoryMapSize, MemoryMap, DescriptorSize, DescriptorVersion);
  PRINT("Type       Start            End       VStart               # Pages          Attributes\n");
  for (Index = 0; Index < NumEntries; Index++) {
    Bytes = (((UINTN) Desc->NumberOfPages) * EFI_PAGE_SIZE);
    PRINT("%-12s %lX-%lX %lX  %lX %lX\n",
          EfiMemoryTypeDesc[Desc->Type],
          Desc->PhysicalStart,
          Desc->PhysicalStart + Bytes - 1,
          Desc->VirtualStart,
          Desc->NumberOfPages,
          Desc->Attribute
          );
    Desc = NEXT_MEMORY_DESCRIPTOR(Desc, DescriptorSize);
    //if ((Index + 1) % 16 == 0) {
    //  WaitForKeyPress(L"press a key to continue\n");
    //}
  }
  //WaitForKeyPress(L"End: press a key to continue\n");
}

VOID EFIAPI
PrintSystemTable(IN EFI_SYSTEM_TABLE  *ST)
{
  UINTN      Index;
  
  PRINT("SysTable: %p\n", ST);
  PRINT("- FirmwareVendor: %p, %s\n", ST->FirmwareVendor, ST->FirmwareVendor);
  PRINT("- FirmwareRevision: %x\n", ST->FirmwareRevision);
  PRINT("- ConsoleInHandle: %p, ConIn: %p\n", ST->ConsoleInHandle, ST->ConIn);
  PRINT("- ConsoleOutHandle: %p, ConOut: %p\n", ST->ConsoleOutHandle, ST->ConOut);
  PRINT("- StandardErrorHandle: %p, StdErr: %p\n", ST->StandardErrorHandle, ST->StdErr);
  PRINT("- RuntimeServices: %p, BootServices: %p\n", ST->RuntimeServices, ST->BootServices);
  PRINT("- ConfigurationTable: %p\n", ST->ConfigurationTable);
  for(Index = 0; Index < ST->NumberOfTableEntries; Index++) {
    PRINT("  %p - %s\n", ST->ConfigurationTable[Index].VendorTable, GuidStr(&ST->ConfigurationTable[Index].VendorGuid));
  }
  
  // print RT services table
  PRINT("- RuntimeServices: %p\n", ST->RuntimeServices);
  PRINT("    GetTime: %p\n", ST->RuntimeServices->GetTime);
  PRINT("    SetTime: %p\n", ST->RuntimeServices->SetTime);
  PRINT("    GetWakeupTime: %p\n", ST->RuntimeServices->GetWakeupTime);
  PRINT("    SetWakeupTime: %p\n", ST->RuntimeServices->SetWakeupTime);
  PRINT("    SetVirtualAddressMap: %p\n", ST->RuntimeServices->SetVirtualAddressMap);
  PRINT("    ConvertPointer: %p\n", ST->RuntimeServices->ConvertPointer);
  PRINT("    GetVariable: %p\n", ST->RuntimeServices->GetVariable);
  PRINT("    GetNextVariableName: %p\n", ST->RuntimeServices->GetNextVariableName);
  PRINT("    SetVariable: %p\n", ST->RuntimeServices->SetVariable);
  PRINT("    GetNextHighMonotonicCount: %p\n", ST->RuntimeServices->GetNextHighMonotonicCount);
  PRINT("    ResetSystem: %p\n", ST->RuntimeServices->ResetSystem);
  PRINT("    UpdateCapsule: %p\n", ST->RuntimeServices->UpdateCapsule);
  PRINT("    QueryCapsuleCapabilities: %p\n", ST->RuntimeServices->QueryCapsuleCapabilities);
  PRINT("    QueryVariableInfo: %p\n", ST->RuntimeServices->QueryVariableInfo);
  
  PRINT("- RuntimeServices Oiginals:\n");
  PRINT("    GetTime: %p\n", gOrgRS.GetTime);
  PRINT("    SetTime: %p\n", gOrgRS.SetTime);
  PRINT("    GetWakeupTime: %p\n", gOrgRS.GetWakeupTime);
  PRINT("    SetWakeupTime: %p\n", gOrgRS.SetWakeupTime);
  PRINT("    SetVirtualAddressMap: %p\n", gOrgRS.SetVirtualAddressMap);
  PRINT("    ConvertPointer: %p\n", gOrgRS.ConvertPointer);
  PRINT("    GetVariable: %p\n", gOrgRS.GetVariable);
  PRINT("    GetNextVariableName: %p\n", gOrgRS.GetNextVariableName);
  PRINT("    SetVariable: %p\n", gOrgRS.SetVariable);
  PRINT("    GetNextHighMonotonicCount: %p\n", gOrgRS.GetNextHighMonotonicCount);
  PRINT("    ResetSystem: %p\n", gOrgRS.ResetSystem);
  PRINT("    UpdateCapsule: %p\n", gOrgRS.UpdateCapsule);
  PRINT("    QueryCapsuleCapabilities: %p\n", gOrgRS.QueryCapsuleCapabilities);
  PRINT("    QueryVariableInfo: %p\n", gOrgRS.QueryVariableInfo);
}

VOID
WaitForKeyPress(CHAR16 *Message)
{
  EFI_STATUS      Status;
  UINTN        index;
  EFI_INPUT_KEY      key;
  
  Print(Message);
  do {
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &key);
  } while(Status == EFI_SUCCESS);
  gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &index);
  do {
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &key);
  } while(Status == EFI_SUCCESS);
}

/** Returns file path from FilePathProto in allocated memory. Mem should be released by caler.*/
CHAR16 *
EFIAPI
FileDevicePathToText(EFI_DEVICE_PATH_PROTOCOL *FilePathProto)
{
  EFI_STATUS      Status;
  FILEPATH_DEVICE_PATH     *FilePath;
  CHAR16        FilePathText[256]; // possible problem: if filepath is bigger
  CHAR16        *OutFilePathText;
  INTN        Size;
  INTN        SizeAll;
  INTN        i;
  
  FilePathText[0] = L'\0';
  i = 4;
  SizeAll = 0;
  //PRINT("FilePathProto->Type: %d, SubType: %d, Length: %d\n", FilePathProto->Type, FilePathProto->SubType, DevicePathNodeLength(FilePathProto));
  while (FilePathProto != NULL && FilePathProto->Type != END_DEVICE_PATH_TYPE && i > 0) {
    if (FilePathProto->Type == MEDIA_DEVICE_PATH && FilePathProto->SubType == MEDIA_FILEPATH_DP) {
      FilePath = (FILEPATH_DEVICE_PATH *) FilePathProto;
      Size = (DevicePathNodeLength(FilePathProto) - 4) / 2;
      if (SizeAll + Size < 256) {
        if (SizeAll > 0 && FilePathText[SizeAll / 2 - 2] != L'\\') {
          StrCatS(FilePathText, 256, L"\\");
        }
        StrCatS(FilePathText, 256, FilePath->PathName);
        SizeAll = StrSize(FilePathText);
      }
    }
    FilePathProto = NextDevicePathNode(FilePathProto);
    //PRINT("FilePathProto->Type: %d, SubType: %d, Length: %d\n", FilePathProto->Type, FilePathProto->SubType, DevicePathNodeLength(FilePathProto));
    i--;
    //PRINT("FilePathText: %s\n", FilePathText);
  }
  
  OutFilePathText = NULL;
  Size = StrSize(FilePathText);
  if (Size > 2) {
    // we are allocating mem here - should be released by caller
    Status = gBS->AllocatePool(EfiBootServicesData, Size, (VOID*)&OutFilePathText);
    if (Status == EFI_SUCCESS) {
      StrCpyS(OutFilePathText, Size/sizeof(CHAR16), FilePathText);
    } else {
      OutFilePathText = NULL;
    }
  }
  
  return OutFilePathText;
}

/** Helper function that calls GetMemoryMap(), allocates space for mem map and returns it. */
EFI_STATUS
EFIAPI
GetMemoryMapAlloc (
                   IN EFI_GET_MEMORY_MAP    GetMemoryMapFunction,
                   OUT UINTN      *MemoryMapSize,
                   OUT EFI_MEMORY_DESCRIPTOR  **MemoryMap,
                   OUT UINTN      *MapKey,
                   OUT UINTN      *DescriptorSize,
                   OUT UINT32      *DescriptorVersion
                   )
{
  EFI_STATUS      Status;
  
  *MemoryMapSize = 0;
  *MemoryMap = NULL;
  Status = GetMemoryMapFunction(MemoryMapSize, *MemoryMap, MapKey, DescriptorSize, DescriptorVersion);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    // OK. Space needed for mem map is in MemoryMapSize
    // Important: next AllocatePool can increase mem map size - we must add some space for this
    *MemoryMapSize += 256;
    *MemoryMap = AllocatePool(*MemoryMapSize);
    Status = GetMemoryMapFunction(MemoryMapSize, *MemoryMap, MapKey, DescriptorSize, DescriptorVersion);
    if (EFI_ERROR(Status)) {
      FreePool(*MemoryMap);
    }
  }
  
  return Status;
}

/** Alloctes Pages from the top of mem, up to address specified in Memory. Returns allocated address in Memory. */
EFI_STATUS
EFIAPI
AllocatePagesFromTop(
                     IN EFI_MEMORY_TYPE    MemoryType,
                     IN UINTN      Pages,
                     IN OUT EFI_PHYSICAL_ADDRESS  *Memory
                     )
{
  EFI_STATUS      Status;
  UINTN        MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR    *MemoryMap;
  UINTN        MapKey;
  UINTN        DescriptorSize;
  UINT32        DescriptorVersion;
  EFI_MEMORY_DESCRIPTOR    *MemoryMapEnd;
  EFI_MEMORY_DESCRIPTOR    *Desc;
  
  
  Status = GetMemoryMapAlloc(gBS->GetMemoryMap, &MemoryMapSize, &MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
  
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  Status = EFI_NOT_FOUND;
  
  //PRINT("Search for Pages=%x, TopAddr=%lx\n", Pages, *Memory);
  //PRINT("MEMMAP: Size=%d, Addr=%p, DescSize=%d, DescVersion: 0x%x\n", MemoryMapSize, MemoryMap, DescriptorSize, DescriptorVersion);
  //PRINT("Type       Start            End       VStart               # Pages          Attributes\n");
  MemoryMapEnd = NEXT_MEMORY_DESCRIPTOR(MemoryMap, MemoryMapSize);
  Desc = PREV_MEMORY_DESCRIPTOR(MemoryMapEnd, DescriptorSize);
  for ( ; Desc >= MemoryMap; Desc = PREV_MEMORY_DESCRIPTOR(Desc, DescriptorSize)) {
    /*
     PRINT("%-12s %lX-%lX %lX  %lX %lX\n",
     EfiMemoryTypeDesc[Desc->Type],
     Desc->PhysicalStart,
     Desc->PhysicalStart + EFI_PAGES_TO_SIZE(Desc->NumberOfPages) - 1,
     Desc->VirtualStart,
     Desc->NumberOfPages,
     Desc->Attribute
     );
     */
    if (   (Desc->Type == EfiConventionalMemory)            // free mem
        && (Pages <= Desc->NumberOfPages)                // contains enough space
        && (Desc->PhysicalStart + EFI_PAGES_TO_SIZE(Pages) <= *Memory)  // contains space below specified Memory
        )
    {
      // free block found
      if (Desc->PhysicalStart + EFI_PAGES_TO_SIZE((UINTN)Desc->NumberOfPages) <= *Memory) {
        // the whole block is unded Memory - allocate from the top of the block
        *Memory = Desc->PhysicalStart + EFI_PAGES_TO_SIZE((UINTN)Desc->NumberOfPages - Pages);
        //PRINT("found the whole block under top mem, allocating at %lx\n", *Memory);
      } else {
        // the block contains enough pages under Memory, but spans above it - allocate below Memory.
        *Memory = *Memory - EFI_PAGES_TO_SIZE(Pages);
        //PRINT("found the whole block under top mem, allocating at %lx\n", *Memory);
      }
      Status = gBS->AllocatePages(AllocateAddress,
                                  MemoryType,
                                  Pages,
                                  Memory);
      //PRINT("Alloc Pages=%x, Addr=%lx, Status=%r\n", Pages, *Memory, Status);
      break;
    }
  }
  
  // release mem
  FreePool(MemoryMap);
  
  return Status;
}

/** Prints RT vars. */
VOID
EFIAPI
PrintRTVariables(
                 IN EFI_RUNTIME_SERVICES  *RT
                 )
{
  EFI_STATUS    Status;
  UINT32      Attributes;
  //UINT64      MaximumVariableStorageSize;
  //UINT64      RemainingVariableStorageSize;
  //UINT64      MaximumVariableSize;
  UINTN      VariableNameSize;
  UINTN      VariableNameBufferSize;
  UINTN      VariableDataSize;
  EFI_GUID    VendorGuid;
  BOOLEAN      IsDataPrintDisabled;
  
  //
  // Print storage info
  //
  /*
   PRINT("Vars storage:\n");
   PRINT("   Attrib: MaximumVariableStorageSize, RemainingVariableStorageSize, MaximumVariableSize\n");
   // NV+BS
   PRINT(" NV+BS   : ");
   Attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS;
   Status = RT->QueryVariableInfo(Attributes, &MaximumVariableStorageSize, &RemainingVariableStorageSize, &MaximumVariableSize);
   if (EFI_ERROR(Status)) {
   PRINT("%r\n", Status);
   } else {
   PRINT("%ld, %ld, %ld\n", MaximumVariableStorageSize, RemainingVariableStorageSize, MaximumVariableSize);
   }
   // NV+BS+RT
   PRINT(" NV+BS+RT: ");
   Attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
   Status = RT->QueryVariableInfo(Attributes, &MaximumVariableStorageSize, &RemainingVariableStorageSize, &MaximumVariableSize);
   if (EFI_ERROR(Status)) {
   PRINT("%r\n", Status);
   } else {
   PRINT("%ld, %ld, %ld\n", MaximumVariableStorageSize, RemainingVariableStorageSize, MaximumVariableSize);
   }
   */
  
  //
  // Print all vars
  //
  PRINT("Variables:\n");
  VariableNameBufferSize = VARIABLE_NAME_BUFFER_SIZE;
  if (gVariableNameBuffer == NULL) {
    // init var name buffer
    // note: this must be called during boot services,
    // so if vars are going to be printed during runtime
    // they must be first printed during boot services
    // to init this buffer.
    if (InBootServices) {
      gVariableNameBuffer = AllocateRuntimePool(VariableNameBufferSize);
    } else {
      // error: buffer not inited during boot services
      PRINT("ERROR: gVariableNameBuffer not inited\n");
      return;
    }
  }
  // first call to GetNextVariableName must be with empty string
  gVariableNameBuffer[0] = L'\0';
  
  while (TRUE) {
    VariableNameSize = VariableNameBufferSize;
    Status = RT->GetNextVariableName(&VariableNameSize, gVariableNameBuffer, &VendorGuid);
    
    if (Status == EFI_BUFFER_TOO_SMALL) {
      // we will not handle this to avoid problems during calling in runtime
      PRINT("ERROR: gVariableNameBuffer too small\n");
      return;
    }
    if (Status == EFI_NOT_FOUND) {
      // no more vars
      break;
    }
    if (EFI_ERROR(Status)) {
      // no more vars or error
      PRINT("ERROR: GetNextVariableName: %r\n", Status);
      return;
    }
    
    // prepare for var data if needed
    if (gVariableDataBuffer == NULL) {
      if (InBootServices) {
        gVariableDataBuffer = AllocateRuntimePool(VARIABLE_DATA_BUFFER_SIZE);
      } else {
        // error: buffer not inited during boot services
        PRINT("ERROR: gVariableDataBuffer not inited\n");
        return;
      }
    }
    
    IsDataPrintDisabled = FALSE;
    
#if PRINT_SHELL_VARS == 0
    {
      BOOLEAN      IsShellVar;
      
      IsShellVar = CompareGuid(&VendorGuid, &ShellInt)
      || CompareGuid(&VendorGuid, &SEnv)
      || CompareGuid(&VendorGuid, &ShellDevPathMap)
      || CompareGuid(&VendorGuid, &ShellProtId)
      || CompareGuid(&VendorGuid, &ShellAlias);
      
      IsDataPrintDisabled = IsShellVar;
    }
#endif
    
    // get and print this var
    VariableDataSize = VARIABLE_DATA_BUFFER_SIZE;
    Status = RT->GetVariable(gVariableNameBuffer, &VendorGuid, &Attributes, &VariableDataSize, gVariableDataBuffer);
    if (EFI_ERROR(Status)) {
      PRINT(" %s:%s = %r\n", GuidStr(&VendorGuid), gVariableNameBuffer, Status);
    } else {
      PRINT("%08x ", Attributes);
      PRINT("%a", (Attributes & EFI_VARIABLE_NON_VOLATILE) ? "NV+" : "   ");
      PRINT("%a", (Attributes & EFI_VARIABLE_BOOTSERVICE_ACCESS) ? "BS+" : "   ");
      PRINT("%a", (Attributes & EFI_VARIABLE_RUNTIME_ACCESS) ? "RT+" : "   ");
      PRINT("%a", (Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) ? "HW+" : "   ");
      PRINT(" %s:%s, DataSize = %x\n", GuidStr(&VendorGuid), gVariableNameBuffer, VariableDataSize);
      if (!IsDataPrintDisabled) {
        PrintBytes(gVariableDataBuffer, VariableDataSize);
      }
    }
  }
}


