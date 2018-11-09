//
//  FloatLib.c
//  
//
//  Created by Slice on 20.06.2018.
//

#include "FloatLib.h"
#include "IO.h"


#define memcpy(dest,source,count) CopyMem(dest,(void*)source,(UINTN)(count))
//#define fabsf(x) ((x >= 0.0f)?x:(-x))
#define fabsf(x) FabsF(x)

float FabsF(float x)
{
  if (x < 0.f) return -x;
  return x;
}

//we will assume sqrt(abs(x))
float SqrtF(float X)
{
  struct FloatInt {
    union {
      INT32 i;
      float f;
    };
  };
  if (X == 0.0f) {
    return 0.0f;
  } else if (X < 0.0f) {
    X = -X;
  }
  struct FloatInt Y;
  Y.f = X * 0.3f;
//  Y.i = Y.i >> 1; // dirty hack - first iteration
  //do six iterations
  Y.f = Y.f * 0.5f + X / (Y.f * 2.0f);
  Y.f = Y.f * 0.5f + X / (Y.f * 2.0f);
  Y.f = Y.f * 0.5f + X / (Y.f * 2.0f);
  Y.f = Y.f * 0.5f + X / (Y.f * 2.0f);
  Y.f = Y.f * 0.5f + X / (Y.f * 2.0f);
  Y.f = Y.f * 0.5f + X / (Y.f * 2.0f);
  return Y.f;
}

float CosF(float X);

//we know sin is odd
float SinF(float X)
{
  INTN Period;
  float X2;
  float Sign = 1.0f;
  
  if (X < 0.0f) {
    X = -X;
    Sign = -1.0f;
  }
  Period = X / PI2;
  X = X - Period * PI2;
  if (X > PI) {
    X = X - PI;
    Sign *= -1.0f;
  }
  if (X > PI5) {
    X = PI - X;
  }
  if (X > PI * 0.25f) {
    return (Sign*CosF(PI5 - X));
  }
  X2 = X * X;
  return (Sign*(X - X2 * X / 6.0f + X2 * X2 * X / 120.0f));
}

//we know cos is even
float CosF(float X)
{
  INTN Period;
  float Sign = 1.0f;
  float X2;
  
  if (X < 0.0f) {
    X = -X;
  }
  Period = X / PI2;
  X = X - Period * PI2;
  if (X > PI) {
    X = PI - X;
    Sign = -1.0f;
  }
  if (X > PI5) {
    X = PI - X;
    Sign *= -1.0f;
  }
  if (X > PI * 0.25f) {
    return (Sign*SinF(PI5 - X));
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
  float res = 0.f, Y = 0.f;
  INTN Sign = 0;

  if (X2 < 0.3f) {
    Y = X * (1.0f + X2 / 6.0f + X2 * X2 * (3.0f / 40.0f));
    return (PI5 - Y);
  } else if (X2 >= 1.0f) {
    return 0.0f;
  } else {
    if (X < 0) {
      X = -X;
      Sign = 1;
    }
    Y = 1.0f - X; //for X ~ 1
    X2 = Y * (2.0 + Y * (1.0 / 3.0 + Y * (4.0f / 45.0 + Y  / 35.0f))); //Dwight, form.508
    res = SqrtF(X2);
    if (Sign) {
      res = PI - res;
    }
  }
  
  return res;
}

float AtanF(float X) //assume 0.0 < X < 1.0
{
  float Eps = 1.0e-8;
  int i = 1;
  float X2 = X * X;
  float D = X;
  float Y = 0;
  float sign = 1.0f;

  if (X > 0.5f) {
    //make here arctg(1-x)
    X = 1.0f - D;
    X2 = X * X;
    Y = PI4 - X * 0.5f - X2 * 0.25f - X * X2 * 0.25f * ( 1.f / 3.f - X2 * (0.1f + X / 12.f + X2 / 28.f));
  } else {
    //  Y = X * (1 - X2 * ( 1.0f / 3.0f - X2 * (1.0f / 5.0f - X2 * ( 1.0f / 7.0f))));
    for (i = 1; i < 50; i += 2) {
      Y += (D * sign / i);
      D *= X2;
      if (D < Eps) {
        break;
      }
      sign = - sign;
    }
  }
  return Y;
}

float Atan2F(float Y, float X)
{
  float sign = (((X >= 0.0f) && (Y < 0.0f)) ||
                ((X < 0.0f) && (Y >= 0.0f)))?-1.0f:1.0f;
  float PP = 0.f;
  float res = 0.f;
  //1,1 = pi4  1,-1=pi34   -1,-1=-pi34   -1,1=-pi4
  if (X < 0.f) {
    PP = PI;
  }
  X = (X >= 0.0f)?X:(-X);
  Y = (Y >= 0.0f)?Y:(-Y);
  if (Y < X) {
    res = AtanF(Y / X);
  } else if (X == 0.0f) {
    res = PI5;
  } else {
    res = (PI5 - AtanF(X / Y));
  }
  return sign * (res - PP);
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
#if 0
VOID QuickSort(VOID* Array, INTN Low, INTN High, INTN Size, INTN (*compare)(CONST VOID* a, CONST VOID* b)) {
  INTN i = Low, j = High;
  VOID *Med, *Temp;
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

//S must be allocated before use
VOID AsciiSPrintFloat(CHAR8* S, INTN N, CHAR8* F, float X)
{
  INTN I, Fract;
  float D;
  if (!S) {
    return;
  }
  
  I = (INTN)X;
  D = I;
  Fract = fabsf((X - D) * 1000000.0f);
  AsciiSPrint(S, N, "%D.%06D", I, (INTN)Fract);
}
#endif

CHAR16* PoolPrintFloat(float X)
{
  INTN I, Fract;
  CHAR8 S = ' ';
  float D;
  I = (INTN)X;
  D = (float)I;
  if (I == 0 && X < 0) {
    S = '-';
  }
  Fract = fabsf((X - D) * 1000000.0f);
  return PoolPrint(L"%c%d.%06d", S, I, (INTN)Fract);
}

static UINT32 seed = 12345;
float rndf() //expected 0..1
{
//  UINT16 Rand = 0;
//  AsmRdRand16(&Rand);  //it's a pity panic
//  return (float)Rand / 65536.f;
  seed = seed * 214013 + 2531011;
  float x = seed / 4294967296.0f;
  return x;
}

int dither(float x)
{
  int i = (int)x;
  float dx = x - (float)i;
  if (dx > rndf()) i++;
  return i;
}
//there is
#if 0
BOOLEAN
EFIAPI
AsmRdRand16 (
             OUT     UINT16                    *Rand
             );
#endif
