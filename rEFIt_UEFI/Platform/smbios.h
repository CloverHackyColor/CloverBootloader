/*
 * smbios.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_SMBIOS_H_
#define PLATFORM_SMBIOS_H_

extern APPLE_SMBIOS_STRUCTURE_POINTER SmbiosTable;
extern BOOLEAN                        gMobile;



UINTN
iStrLen(
  CONST CHAR8* String,
  UINTN  MaxLen
  );

EFI_STATUS
PrepatchSmbios (VOID);

VOID
PatchSmbios (VOID);

VOID
FinalizeSmbios (VOID);



#endif /* PLATFORM_SMBIOS_H_ */
