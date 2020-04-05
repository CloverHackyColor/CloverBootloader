/*
 * common.h
 *
 *  Created on: 5 Apr 2020
 *      Author: jief
 */

#ifndef ENTRY_SCAN_COMMON_H_
#define ENTRY_SCAN_COMMON_H_

#include "../cpp_foundation/XString.h"

XString AddLoadOption(IN const XString& LoadOptions, IN const XString& LoadOption);
XString RemoveLoadOption(IN const XString& LoadOptions, IN const XString& LoadOption);



#endif /* ENTRY_SCAN_COMMON_H_ */
