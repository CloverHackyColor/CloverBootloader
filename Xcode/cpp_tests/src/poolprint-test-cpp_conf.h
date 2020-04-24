
#include "../../../Include/Library/printf_lite.h"
#define F(x) x
#define LF(x) L##x
#define PRIF "%a"
#define PRILF "%s"

//#define loggf(...) printf__VA_ARGS__)
#define loggf(...) printf(__VA_ARGS__)

//#define DISPLAY_ONLY_FAILED
#define DISPLAY_START_INFO

