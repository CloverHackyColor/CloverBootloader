/*
 * Nvram.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_DATAHUBCPU_H_
#define PLATFORM_DATAHUBCPU_H_

#include "../gui/menu_items/menu_items.h"

EFI_STATUS
EFIAPI
SetVariablesForOSX (LOADER_ENTRY *Entry);


EFI_STATUS
EFIAPI
LogDataHub (
  EFI_GUID *TypeGuid,
  CONST CHAR16   *Name,
  VOID     *Data,
  UINT32   DataSize
  );

VOID
EFIAPI
SetupDataForOSX (BOOLEAN Hibernate);


#endif /* PLATFORM_DATAHUBCPU_H_ */
