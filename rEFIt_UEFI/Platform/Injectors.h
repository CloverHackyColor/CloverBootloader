/*
 * Injectors.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_INJECTORS_H_
#define PLATFORM_INJECTORS_H_

#include <Library/OcAfterBootCompatLib.h>

extern UINT32               mPropSize;
extern UINT8                *mProperties;
extern CHAR8                *gDeviceProperties;
extern UINT32               cPropSize;
extern UINT8                *cProperties;
extern CHAR8                *cDeviceProperties;
extern CHAR8                *BootOSName;
extern OC_ABC_SETTINGS      gQuirks;

EFI_STATUS
SetPrivateVarProto (VOID);



#endif /* PLATFORM_INJECTORS_H_ */
