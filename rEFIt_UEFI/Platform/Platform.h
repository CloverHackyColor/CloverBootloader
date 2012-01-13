/*
Headers collection for procedures
*/

#include <Uefi.h>

#include <Guid/Acpi.h>
#include <Guid/EventGroup.h>
#include <Guid/SmBios.h>
#include <Guid/Mps.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HiiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <IndustryStandard/Acpi10.h>
#include <IndustryStandard/Acpi20.h>

#include <Protocol/Cpu.h>
#include <Protocol/CpuIo.h>
#include <Protocol/DataHub.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/FrameworkHii.h>
#include <Protocol/Smbios.h>
#include <Protocol/VariableWrite.h>
#include <Protocol/Variable.h>


#include "boot.h"
#include "BiosVideo.h"
#include "Bmp.h"
#include "efiConsoleControl.h"

#define SAFE_LOG_SIZE	80

#define BOOTER_LOG_SIZE	(64 * 1024)
#define MsgLog(x...) {AsciiSPrint(msgCursor, BOOTER_LOG_SIZE, x); while(*msgCursor){msgCursor++;}}


extern CHAR8                    *msgbuf;
extern CHAR8                    *msgCursor;
extern SMBIOS_STRUCTURE_POINTER	SmbiosTable;
extern GFX_MANUFACTERER         gGraphicsCard;
extern CHAR8*                   cpuFrequencyMHz;
extern BOOLEAN                  gMobile;
extern UINT16                   gCpuSpeed;
extern CHAR8*                   BiosVendor;
extern UINT32                   mPropSize;
extern UINT8*                   mProperties;
extern CHAR8                    gSelectedUUID[];
extern CHAR8*                   AppleSystemVersion[];
extern UINT8                    gDefaultType;
extern EFI_SYSTEM_TABLE*        gST;
extern EFI_BOOT_SERVICES*       gBS; 
extern GUI_MENU_DATA            gSettings;
extern CPU_STRUCTURE            gCPUStructure;
extern EFI_GUID                 gUuid;
extern CHAR8                    gOEMProduct[];  //original name from SMBIOS


typedef struct {
  
  UINT32		type;
  CHAR8     *string;
  UINT32		offset;
  VOID      *tag;
  VOID      *tagNext;
  
}Tag, *TagPtr;


VOID InitBooterLog(VOID);
EFI_STATUS SetupBooterLog(VOID);

EFI_STATUS
EFIAPI
InitializeConsoleSim (
                      IN EFI_HANDLE           ImageHandle,
                      IN EFI_SYSTEM_TABLE     *SystemTable
                      );

UINT64 GetCPUProperties (VOID);

EFI_STATUS EFIAPI
LogDataHub(
           EFI_GUID					*TypeGuid,
           CHAR16                      *Name,
           VOID                        *Data,
           UINT32                       DataSize);

EFI_STATUS SetVariablesForOSX();
VOID SetupDataForOSX();
EFI_STATUS SetPrivateVarProto(VOID);

EFI_STATUS PatchACPI(VOID);

EFI_STATUS EventsInitialize (
                  IN EFI_HANDLE                             ImageHandle,
                  IN EFI_SYSTEM_TABLE                       *SystemTable
                  );

EFI_STATUS bootElTorito(REFIT_VOLUME*	volume);
EFI_STATUS bootMBR(REFIT_VOLUME* volume);
EFI_STATUS bootPBR(REFIT_VOLUME* volume);

CHAR8*      XMLDecode(const CHAR8* src);
EFI_STATUS  ParseXML(const CHAR8* buffer, TagPtr * dict);
TagPtr      GetProperty( TagPtr dict, const CHAR8* key );
EFI_STATUS  XMLParseNextTag(CHAR8* buffer, TagPtr * tag, UINT32* lenPtr);

VOID 		SaveSettings(VOID);

UINTN iStrLen(CHAR8* String, UINTN MaxLen);
EFI_STATUS PrepatchSmbios(VOID);
VOID PatchSmbios(VOID);
VOID FinalizeSmbios(VOID);

EFI_STATUS DisableUsbLegacySupport(VOID);
