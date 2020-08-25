//
//  FloatLib.h
//  
//
//  Created by Slice on 20.06.2018.
//

#ifndef FloatLib_h
#define FloatLib_h

#define PI (3.1415926536f)
#define PI2 (6.283185307179586f)
#define PI5 (1.570796326794897f)
#define PI4 (0.78539816339745f)
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
float FabsF(float X);
float rndf(void);  //random number from 0 to 1.0f
int dither(float x, int level);
float nsvg__vmag(float x, float y); //sqrt(x*x+y*y)

inline float FabsF(float x) {
  if (x < 0.f) return -x;
  return x;
}

inline float SqrF(float x) { return x*x; }


RETURN_STATUS
AsciiStrToFloat(IN  CONST CHAR8              *String,
                OUT       CHAR8              **EndPointer,  OPTIONAL
                OUT       float              *Data);
#if 0
//VOID AsciiSPrintFloat(CHAR8* S, INTN N, CHAR8* F, float X);

VOID QuickSort(VOID* Array, INTN Low, INTN High, INTN Size,
               INTN (*compare)(CONST VOID* a, CONST VOID* b));
#endif


#endif /* FloatLib_h */
