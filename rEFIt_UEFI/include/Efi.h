/*
 * Efi.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef INCLUDE_EFI_H_
#define INCLUDE_EFI_H_



#ifdef __cplusplus
extern "C" {
#endif

//#include <Library/printf_lite.h>

#include <Uefi.h>
#include <Uefi/UefiSpec.h>

#include <Guid/Acpi.h>
#include <Guid/EventGroup.h>
#include <Guid/SmBios.h>
#include <Guid/Mps.h>
#include <Guid/FileInfo.h>
#include <Guid/FileSystemInfo.h>
#include <Guid/FileSystemVolumeLabelInfo.h>
#include <Guid/GlobalVariable.h>

#include <Pi/PiDxeCis.h>

#include <Protocol/DevicePath.h> // before #include <Library/DevicePathLib.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DebugPrintErrorLevelLib.h>
#include <Library/DevicePathLib.h>
#include <Uefi/UefiInternalFormRepresentation.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Library/GenericBdsLib.h>
//#include <Library/HiiLib.h>
#include <Library/HdaModels.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/PerformanceLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/UsbMass.h>
#include <Library/VideoBiosPatchLib.h>
#include <Library/MemLogLib.h>
#include <Library/WaveLib.h>
#include <Library/IoLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/HobLib.h>
#include <Library/SerialPortLib.h>
#include <Library/HdaVerbs.h>
//#include <Library/NetLib.h>

#include <Framework/FrameworkInternalFormRepresentation.h>

#include <IndustryStandard/Acpi10.h>
#include <IndustryStandard/Acpi20.h>
#include <IndustryStandard/Atapi.h>
#include <IndustryStandard/AppleHid.h>
#include <IndustryStandard/AppleSmBios.h>
#include <IndustryStandard/AppleFeatures.h>
#include <IndustryStandard/AppleBootArgs.h>
#include <IndustryStandard/Bmp.h>
#include <IndustryStandard/HdaCodec.h>
#include <IndustryStandard/Pci22.h>
#include <IndustryStandard/Pci23.h>

#include <Protocol/PciIo.h>
#include <Protocol/AudioIo.h>
#include <Protocol/Cpu.h>
#include <Protocol/CpuIo.h>
#include <Protocol/DataHub.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/EdidDiscovered.h>
#include <Protocol/EdidOverride.h>
#include <Protocol/FrameworkHii.h>
#include <Protocol/HdaIo.h>
#include <Protocol/SimplePointer.h>
#include <Protocol/Smbios.h>
#include <Protocol/VariableWrite.h>
#include <Protocol/Variable.h>
#include <Protocol/UgaDraw.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/UnicodeCollation.h>
#include <Protocol/ScsiIo.h>
#include <Protocol/LegacyBios.h>
#include <Protocol/EdidActive.h>
#include <Protocol/PlatformDriverOverride.h>
#include <Protocol/Legacy8259.h>
#include <Protocol/Timer.h>
#include <Protocol/OSInfo.h>
#include <Protocol/AppleGraphConfig.h>
#include <Protocol/KeyboardInfo.h>

#include <Protocol/FSInjectProtocol.h>
#include <Protocol/MsgLog.h>
//#include <Protocol/efiConsoleControl.h>
#include <Protocol/EmuVariableControl.h>
#include <Protocol/AppleSMC.h>
#include <Protocol/AppleImageCodecProtocol.h>
#include <Protocol/HdaCodecInfo.h>

#include "../../OpenCorePkg/Include/Acidanthera/Library/OcConsoleLib.h"
#ifdef __cplusplus
}
#endif


#define EFI_SYSTEM_TABLE_MAX_ADDRESS 0xFFFFFFFF



#endif /* INCLUDE_EFI_H_ */
