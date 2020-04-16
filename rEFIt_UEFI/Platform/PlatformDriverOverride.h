/*
 * PlatformDriverOverride.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_PLATFORMDRIVEROVERRIDE_H_
#define PLATFORM_PLATFORMDRIVEROVERRIDE_H_


/** Registers given PriorityDrivers (NULL terminated) to highest priority during connecting controllers.
 *  Does this by installing our EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL
 *  or by overriding existing EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL.GetDriver.
 */
VOID
RegisterDriversToHighestPriority (
  IN  EFI_HANDLE *PriorityDrivers
  );



#endif /* PLATFORM_PLATFORMDRIVEROVERRIDE_H_ */
