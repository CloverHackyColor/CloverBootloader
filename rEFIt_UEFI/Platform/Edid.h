/*
 * Edid.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_EDID_H_
#define PLATFORM_EDID_H_



EFI_STATUS
InitializeEdidOverride (VOID);

UINT8*
getCurrentEdid (VOID);

EFI_STATUS
GetEdidDiscovered (VOID);



#endif /* PLATFORM_EDID_H_ */
