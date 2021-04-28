#ifndef __OLD_SETTINGS_H__
#define __OLD_SETTINGS_H__

#include "../../../../rEFIt_UEFI/Platform/plist/TagDict.h"
#include "../../../../rEFIt_UEFI/Platform/Settings.h"

EFI_STATUS
GetEarlyUserSettings (
  const TagDict*   CfgDict,
  SETTINGS_DATA& gSettings
  );

EFI_STATUS
GetUserSettings(const TagDict* CfgDict, SETTINGS_DATA& gSettings);

#endif
