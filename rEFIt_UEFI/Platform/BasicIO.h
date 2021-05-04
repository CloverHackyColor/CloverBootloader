/*
 * BasicIO.h
 *
 *  Created on: 28 Mar 2020
 *      Author: jief
 */

#ifndef PLATFORM_BASICIO_H_
#define PLATFORM_BASICIO_H_


BOOLEAN ReadAllKeyStrokes(void);
void PauseForKey(const XString8& msg);

//void DebugPause(void);
void EndlessIdleLoop(void);


BOOLEAN CheckFatalError(IN EFI_STATUS Status, IN CONST CHAR16 *where);
BOOLEAN CheckError(IN EFI_STATUS Status, IN CONST CHAR16 *where);


#endif /* PLATFORM_BASICIO_H_ */
