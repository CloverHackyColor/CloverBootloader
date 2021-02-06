/*
 * posix_additions.h
 *
 *  Created on: Feb 5, 2021
 *      Author: jief
 */

#ifndef PLATFORM_POSIX_POSIX_ADDITIONS_H_
#define PLATFORM_POSIX_POSIX_ADDITIONS_H_

const char* efiStrError(EFI_STATUS errnum);
const char* strguid(EFI_GUID* guid);

#endif /* PLATFORM_POSIX_POSIX_ADDITIONS_H_ */
