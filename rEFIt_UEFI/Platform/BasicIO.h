/*
 * BasicIO.h
 *
 *  Created on: 28 Mar 2020
 *      Author: jief
 */

#ifndef PLATFORM_BASICIO_H_
#define PLATFORM_BASICIO_H_


BOOLEAN ReadAllKeyStrokes(void);
void PauseForKey(CONST CHAR16* msg);

//void DebugPause(void);
void EndlessIdleLoop(void);



#endif /* PLATFORM_BASICIO_H_ */
