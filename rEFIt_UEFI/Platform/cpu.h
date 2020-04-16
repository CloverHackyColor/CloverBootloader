/*
 * cpu.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_CPU_H_
#define PLATFORM_CPU_H_

extern UINT64                         TurboMsr;
extern CPU_STRUCTURE                  gCPUStructure;


VOID
GetCPUProperties (VOID);

MACHINE_TYPES
GetDefaultModel (VOID);

UINT16
GetAdvancedCpuType (VOID);

VOID
SetCPUProperties (VOID);



#endif /* PLATFORM_CPU_H_ */
