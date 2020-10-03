#include "../libeg/libeg.h"

void InitScreen(IN BOOLEAN SetMaxResolution);
void SetupScreen(void);
void BeginTextScreen(IN CONST CHAR16 *Title);
void FinishTextScreen(IN BOOLEAN WaitAlways);
void BeginExternalScreen(IN BOOLEAN UseGraphicsMode/*, IN CONST CHAR16 *Title*/);
void FinishExternalScreen(void);
void TerminateScreen(void);
void SetNextScreenMode(INT32);

void SwitchToGraphicsAndClear(void);
void BltClearScreen();

INTN HybridRepositioning(INTN Edge, INTN Value, INTN ImageDimension, INTN ScreenDimension, INTN DesignScreenDimension);
INTN CalculateNudgePosition(INTN Position, INTN NudgeValue, INTN ImageDimension, INTN ScreenDimension);

