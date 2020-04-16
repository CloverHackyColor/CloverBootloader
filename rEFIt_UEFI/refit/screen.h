#include "../libeg/libeg.h"

VOID InitScreen(IN BOOLEAN SetMaxResolution);
VOID SetupScreen(VOID);
VOID BeginTextScreen(IN CONST CHAR16 *Title);
VOID FinishTextScreen(IN BOOLEAN WaitAlways);
VOID BeginExternalScreen(IN BOOLEAN UseGraphicsMode/*, IN CONST CHAR16 *Title*/);
VOID FinishExternalScreen(VOID);
VOID TerminateScreen(VOID);
VOID SetNextScreenMode(INT32);

VOID SwitchToGraphicsAndClear(VOID);
VOID BltClearScreen();
VOID BltImage(IN EG_IMAGE *Image, IN INTN XPos, IN INTN YPos);
VOID BltImageAlpha(IN EG_IMAGE *Image, IN INTN XPos, IN INTN YPos, IN EG_PIXEL *BackgroundPixel, INTN Scale);
//VOID BltImageComposite(IN EG_IMAGE *BaseImage, IN EG_IMAGE *TopImage, IN INTN XPos, IN INTN YPos);
VOID BltImageCompositeBadge(IN EG_IMAGE *BaseImage, IN EG_IMAGE *TopImage, IN EG_IMAGE *BadgeImage, IN INTN XPos, IN INTN YPos, INTN Scale);
//VOID BltImageCompositeIndicator(IN EG_IMAGE *BaseImage, IN EG_IMAGE *TopImage, IN INTN XPos, IN INTN YPos, INTN Scale);


INTN HybridRepositioning(INTN Edge, INTN Value, INTN ImageDimension, INTN ScreenDimension, INTN DesignScreenDimension);
INTN CalculateNudgePosition(INTN Position, INTN NudgeValue, INTN ImageDimension, INTN ScreenDimension);


