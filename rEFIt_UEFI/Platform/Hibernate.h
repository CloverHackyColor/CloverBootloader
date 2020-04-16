/*
 * Nvram.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_HIBERNATE_H_
#define PLATFORM_HIBERNATE_H_

/** Returns TRUE if given macOS on given volume is hibernated
 *  (/private/var/vm/sleepimage exists and it's modification time is close to volume modification time).
 */
BOOLEAN
IsOsxHibernated (
  IN LOADER_ENTRY    *Entry
  );


/** Prepares nvram vars needed for boot.efi to wake from hibernation. */
BOOLEAN
PrepareHibernation (
  IN REFIT_VOLUME *Volume
  );


#endif /* PLATFORM_HIBERNATE_H_ */
