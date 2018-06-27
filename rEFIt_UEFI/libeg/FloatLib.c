//
//  FloatLib.c
//  
//
//  Created by Slice on 20.06.2018.
//

#include "FloatLib.h"

#define memcpy(dest,source,count) gBS->CopyMem(dest,(void*)source,(UINTN)(count))

float SqrtF(float X)
{
  struct FloatInt {
    union {
      INT32 i;
      float f;
    };
  };
  struct FloatInt Y;
  Y.f = X;
  Y.i = Y.i >> 1; // dirty hack - first iteration
  //do four iterations
  Y.f = Y.f * 0.5f + X / (Y.f * 2.0f);
  Y.f = Y.f * 0.5f + X / (Y.f * 2.0f);
  Y.f = Y.f * 0.5f + X / (Y.f * 2.0f);
  Y.f = Y.f * 0.5f + X / (Y.f * 2.0f);
  return Y.f;
}

float CosF(float X);
float SinF(float X)
{
  INTN Period = X / (2.0f * PI);
  float X2;
  X = X - Period * (2.0f * PI);
  if (X > PI) {
    X = PI - X;
  }
  if (X > PI * 0.5f) {
    X = PI - X;
  }
  if (X > PI * 0.25f) {
    return CosF(PI * 0.5f - X);
  }
  X2 = X * X;
  return (X - X2 * X / 6.0f + X2 * X2 * X / 120.0f);
}

float CosF(float X)
{
  INTN Period = X / (2.0f * PI);
  float Sign = 1.0f;
  float X2;
  X = X - Period * (2.0f * PI);
  if (X > PI) {
    X = PI - X;
    Sign = -1.0f;
  }
  if (X > PI * 0.5f) {
    X = PI - X;
    Sign *= -1.0f;
  }
  if (X > PI * 0.25f) {
    return SinF(PI * 0.5f - X);
  }
  X2 = X * X;
  return (Sign * (1.0f - X2 * 0.5f + X2 * X2 / 24.0f));
}

float TanF(float X)
{
  float Y = CosF(X);
  if (Y == 0.0f) {
    Y = 1.0e-37;
  }
  return SinF(X)/Y;
}

float PowF(float x, INTN n)
{
  float Data = x;
  if (n > 0) {
    while (n > 0) {
      Data *= 10.0f;
      n--;
    }
  } else {
    while (n < 0) {
      Data *= 0.1f;
      n++;
    }
  }
  return Data;
}

float CeilF(float X)
{
  INT32 I = X;
  return (float)(++I);
}

float FloorF(float X)
{
  INT32 I = X;
  return (float)I;
}

float ModF(float X, float Y)
{
  INT32 I = (INT32)(X / Y);
  return (X - I * Y);
}

float AcosF(float X)
{
  float X2 = X * X;
  return (PI * 0.5f - X * (1.0f + X2 / 6.0f + X2 * X2 * (3.0f / 40.0f)));
}

float AtanF(float X) //assume 0.0 < X < 1.0
{
  float Eps = 1.0e-4;
  int i = 1;
  float X2 = X * X;
  float D = - X;
  float Y = 0;
  float sign = 1.0f;
  while (D > (Eps * i)) {
    Y += D * sign / i;  //x -x3/3+x5/5-x7/7...
    D *= X2;
    sign = - sign;
    i += 2;
  }
  return Y;
}

float Atan2F(float X, float Y)
{
  float sign = (((X > 0.0f) && (Y < 0.0f)) ||
                ((X < 0.0f) && (Y > 0.0f)));
  X = (X > 0.0f)?X:(-X);
  Y = (Y > 0.0f)?Y:(-Y);
  if (X < Y) {
    return sign * AtanF(X / Y);
  } else if (Y == 0.0f) {
    return sign * (PI * 0.5f);
  } else {
    return sign * (PI * 0.5f - AtanF(Y / X));
  }
  return 0.0f;
}

/*
RETURN_STATUS
EFIAPI
AsciiStrDecimalToUintnS (
                         IN  CONST CHAR8              *String,
                         OUT       CHAR8              **EndPointer,  OPTIONAL
                         OUT       UINTN              *Data
                         );
*/
RETURN_STATUS
EFIAPI
AsciiStrToFloat(IN  CONST CHAR8              *String,
                OUT       CHAR8              **EndPointer,  OPTIONAL
                OUT       float              *Data)
{
  UINTN Temp = 0;
  INTN Sign = 1;
  float Mantissa, Ftemp;
  CHAR8* TmpStr = NULL;
  RETURN_STATUS Status = RETURN_SUCCESS;
  if (EndPointer != NULL) {
    *EndPointer = (CHAR8 *) String;
  }
  //
  // Ignore the pad spaces (space or tab)
  //
  while ((*String == ' ') || (*String == '\t')) {
    String++;
  }
  //
  // Ignore leading Zeros after the spaces
  //
/*  while (*String == '0') {
    String++;
  } */
  if (*String == '-') {
    Sign = -1;
    String++;
  } else if (*String == '+') {
    String++;
  }

  Status = AsciiStrDecimalToUintnS(String, &TmpStr, &Temp);
  Mantissa = Temp;
  String = TmpStr;
  if (*String == '.') {
    String++;
    Temp = 0;
    Status = AsciiStrDecimalToUintnS(String, &TmpStr, &Temp);
    Ftemp = Temp;
    while (String != TmpStr) {
      if (*String == '\0') {
        break;
      }
      Ftemp *= 0.1f;
      String++;
    }
    Mantissa += Ftemp;
  }
  
  if ((*String == 'E') || (*String == 'e')){
    INTN ExpSign = 1;
    String++;
    if (*String == '-') {
      ExpSign = -1;
      String++;
    } else if (*String == '+') {
      String++;
    }
    Temp = 0;
    Status = AsciiStrDecimalToUintnS(String, &TmpStr, &Temp);
    if (Status == RETURN_SUCCESS) {
      Ftemp = PowF(10.0f, ExpSign * Temp);
      Mantissa *= Ftemp;
    }
  }
  *Data = Mantissa * Sign;
  if (EndPointer != NULL) {
    *EndPointer = (CHAR8 *) TmpStr;
  }
  return RETURN_SUCCESS;
}

/*
 //Slice - this is my replacement for standard
 qsort(void* Array, int Num, size_t Size,
       int (*compare)(void* a, void* b))
 usage qsort(Array, Num, sizeof(*Array), compare);
 where for example
 int compare(void *a, void* b)
 {
   if (*(float*)a > *(float*)b) return 1;
   if (*(float*)a < *(float*)b) return -1;
   return -0;
 }
 */

void QuickSort(void* Array, int Low, int High, INTN Size, int (*compare)(const void* a, const void* b)) {
  int i = Low, j = High;
  void *Med, *Temp;
  Med = Array + ((Low + High) / 2) * Size; // Central element, just pointer
  Temp = AllocatePool(Size);
  // Sort around center
  while (i <= j)
  {
    while (compare((const void*)(Array+i*Size), (const void*)Med) == -1) i++;
    while (compare((const void*)(Array+j*Size), (const void*)Med) == 1) j--;
    // Change
    if (i <= j) {
      memcpy(Temp, Array+i*Size, Size);
      memcpy(Array+i*Size, Array+j*Size, Size);
      memcpy(Array+j*Size, Temp, Size);
      i++;
      j--;
    }
  }
  FreePool(Temp);
  // Recursion
  if (j > Low)    QuickSort(Array, Low, j, Size, compare);
  if (High > i)   QuickSort(Array, i, High, Size, compare);
}

