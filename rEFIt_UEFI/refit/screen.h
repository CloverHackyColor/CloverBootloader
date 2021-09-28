#include "../libeg/libeg.h"

void InitScreen(IN XBool SetMaxResolution);
void SetupScreen(void);
void BeginTextScreen(IN CONST CHAR16 *Title);
void FinishTextScreen(IN XBool WaitAlways);
void BeginExternalScreen(IN XBool UseGraphicsMode/*, IN CONST CHAR16 *Title*/);
void FinishExternalScreen(void);
void TerminateScreen(void);
void SetNextScreenMode(INT32);

void SwitchToGraphicsAndClear(void);
void BltClearScreen();

INTN HybridRepositioning(INTN Edge, INTN Value, INTN ImageDimension, INTN ScreenDimension, INTN DesignScreenDimension);
INTN CalculateNudgePosition(INTN Position, INTN NudgeValue, INTN ImageDimension, INTN ScreenDimension);

