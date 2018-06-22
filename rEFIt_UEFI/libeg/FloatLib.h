//
//  FloatLib.h
//  
//
//  Created by Slice on 20.06.2018.
//

#ifndef FloatLib_h
#define FloatLib_h

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>

#define PI (3.1415926536f)
#define FLT_MAX (1.0e38f)
#define FLT_MIN (1.0e-37f)

float SqrtF(float X);
float PowF(float X, INTN N);
float SinF(float X);
float CosF(float X);
float TanF(float X);
float CeilF(float X);
float FloorF(float X);
float ModF(float X, float Y);
float AcosF(float X);
float Atan2F(float X, float Y);

RETURN_STATUS
EFIAPI
AsciiStrToFloat(IN  CONST CHAR8              *String,
                OUT       CHAR8              **EndPointer,  OPTIONAL
                OUT       float              *Data);

void QuickSort(void* Array, int Low, int High, INTN Size,
               int (*compare)(const void* a, const void* b));

#endif /* FloatLib_h */
