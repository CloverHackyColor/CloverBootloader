/*
 * BasicIO.h
 *
 *  Created on: 28 Mar 2020
 *      Author: jief
 */

#ifndef PLATFORM_BASICIO_H_
#define PLATFORM_BASICIO_H_


XBool ReadAllKeyStrokes(void);
void PauseForKey(const XString8& msg);

//void DebugPause(void);
void EndlessIdleLoop(void);


XBool CheckFatalError(IN EFI_STATUS Status, IN CONST CHAR16 *where);
XBool CheckError(IN EFI_STATUS Status, IN CONST CHAR16 *where);


#endif /* PLATFORM_BASICIO_H_ */
