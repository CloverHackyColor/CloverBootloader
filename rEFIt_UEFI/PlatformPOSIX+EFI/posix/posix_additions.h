/*
 * posix_additions.h
 *
 *  Created on: Feb 5, 2021
 *      Author: jief
 */

#ifndef PLATFORM_POSIX_POSIX_ADDITIONS_H_
#define PLATFORM_POSIX_POSIX_ADDITIONS_H_

// This should probably no be here, in posix as it used EFI types
// This doesn't compile if compile under posix without all EFI sdk. It's the case for compiling the validator
// It's 2 EFI "utils" that probably shouldn't be here and 
const char* efiStrError(EFI_STATUS errnum);
const char* strguid(EFI_GUID* guid);

#endif /* PLATFORM_POSIX_POSIX_ADDITIONS_H_ */
