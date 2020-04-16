/*
 * LegacyBoot.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_LEGACYBOOT_H_
#define PLATFORM_LEGACYBOOT_H_



EFI_STATUS
bootElTorito (
  IN REFIT_VOLUME *volume
  );


EFI_STATUS
bootMBR (
  IN REFIT_VOLUME *volume
  );

EFI_STATUS
bootPBR (
  IN REFIT_VOLUME *volume, BOOLEAN SataReset
  );

EFI_STATUS
bootPBRtest (
  IN REFIT_VOLUME *volume
  );

EFI_STATUS
bootLegacyBiosDefault (
  IN  UINT16 LegacyBiosDefaultEntry
  );

VOID
DumpBiosMemoryMap (VOID);


#endif /* PLATFORM_LEGACYBOOT_H_ */
