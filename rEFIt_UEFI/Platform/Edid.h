/*
 * Edid.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_EDID_H_
#define PLATFORM_EDID_H_



EFI_STATUS
InitializeEdidOverride (void);

UINT8*
getCurrentEdid (void);

EFI_STATUS
GetEdidDiscovered (void);



#endif /* PLATFORM_EDID_H_ */
