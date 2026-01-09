/*
 * Nvram.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_HIBERNATE_H_
#define PLATFORM_HIBERNATE_H_

/** Returns true if given macOS on given volume is hibernated
 *  (/private/var/vm/sleepimage exists and it's modification time is close to
 * volume modification time).
 */
class LOADER_ENTRY;
XBool IsOsxHibernated(IN LOADER_ENTRY *Entry);

/** Prepares nvram vars needed for boot.efi to wake from hibernation. */
XBool PrepareHibernation(IN REFIT_VOLUME *Volume);

#endif /* PLATFORM_HIBERNATE_H_ */
