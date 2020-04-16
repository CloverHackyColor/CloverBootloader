/*
 * cpu.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_CPU_H_
#define PLATFORM_CPU_H_

#include "platformdata.h"


typedef struct {
 //values from CPUID
  UINT32                  CPUID[CPUID_MAX][4];
  UINT32                  Vendor;
  UINT32                  Signature;
  UINT32                  Family;
  UINT32                  Model;
  UINT32                  Stepping;
  UINT32                  Type;
  UINT32                  Extmodel;
  UINT32                  Extfamily;
  UINT64                  Features;
  UINT64                  ExtFeatures;
  UINT32                  CoresPerPackage;
  UINT32                  LogicalPerPackage;
  CHAR8                   BrandString[48];

  //values from BIOS
  UINT64                  ExternalClock;
  UINT32                  MaxSpeed;       //MHz
  UINT32                  CurrentSpeed;   //MHz
//  UINT32                  Pad;

  //calculated from MSR
  UINT64                  MicroCode;
  UINT64                  ProcessorFlag;
  UINT32                  MaxRatio;
  UINT32                  SubDivider;
  UINT32                  MinRatio;
  UINT32                  DynFSB;
  UINT64                  ProcessorInterconnectSpeed; //MHz
  UINT64                  FSBFrequency; //Hz
  UINT64                  CPUFrequency;
  UINT64                  TSCFrequency;
  UINT8                   Cores;
  UINT8                   EnabledCores;
  UINT8                   Threads;
  UINT8                   Mobile;  //not for i3-i7
  BOOLEAN                 Turbo;
  UINT8                   Pad2[3];

  /* Core i7,5,3 */
  UINT16                  Turbo1; //1 Core
  UINT16                  Turbo2; //2 Core
  UINT16                  Turbo3; //3 Core
  UINT16                  Turbo4; //4 Core

  UINT64                  TSCCalibr;
  UINT64                  ARTFrequency;

} CPU_STRUCTURE;



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
