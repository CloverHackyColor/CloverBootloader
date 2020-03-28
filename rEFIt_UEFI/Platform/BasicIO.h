/*
 * BasicIO.h
 *
 *  Created on: 28 Mar 2020
 *      Author: jief
 */

#ifndef PLATFORM_BASICIO_H_
#define PLATFORM_BASICIO_H_


BOOLEAN ReadAllKeyStrokes(VOID);
VOID PauseForKey(CONST CHAR16* msg);

//VOID DebugPause(VOID);
VOID EndlessIdleLoop(VOID);



#endif /* PLATFORM_BASICIO_H_ */
