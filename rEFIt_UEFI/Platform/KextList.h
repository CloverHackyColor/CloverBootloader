/*
 * KextList.h
 *
 *  Created on: Feb 4, 2021
 *      Author: jief
 */

#ifndef PLATFORM_KEXTLIST_H_
#define PLATFORM_KEXTLIST_H_

#include "../cpp_foundation/XObjArray.h"
#include "Settings.h"

// Sideload/inject kext
extern XObjArray<SIDELOAD_KEXT>         InjectKextList;

void InitKextList(void);

#endif /* PLATFORM_KEXTLIST_H_ */
