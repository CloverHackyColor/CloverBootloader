#include "../gui/menu_items/menu_items.h"

VOID InitScreen(IN BOOLEAN SetMaxResolution);
VOID SetupScreen(VOID);
VOID BeginTextScreen(IN CONST CHAR16 *Title);
VOID FinishTextScreen(IN BOOLEAN WaitAlways);
VOID BeginExternalScreen(IN BOOLEAN UseGraphicsMode, IN CONST CHAR16 *Title);
VOID FinishExternalScreen(VOID);
VOID TerminateScreen(VOID);
VOID SetNextScreenMode(INT32);


//BOOLEAN GetAnime(REFIT_MENU_SCREEN *Screen);
//VOID    InitAnime(REFIT_MENU_SCREEN *Screen);
//VOID    UpdateAnime(REFIT_MENU_SCREEN *Screen, const EG_RECT *Place);
VOID    FreeAnime(GUI_ANIME *Anime);


VOID SwitchToGraphicsAndClear(VOID);
VOID BltClearScreen(IN BOOLEAN ShowBanner);
VOID BltImage(IN EG_IMAGE *Image, IN INTN XPos, IN INTN YPos);
VOID BltImageAlpha(IN EG_IMAGE *Image, IN INTN XPos, IN INTN YPos, IN EG_PIXEL *BackgroundPixel, INTN Scale);
VOID BltImageComposite(IN EG_IMAGE *BaseImage, IN EG_IMAGE *TopImage, IN INTN XPos, IN INTN YPos);
VOID BltImageCompositeBadge(IN EG_IMAGE *BaseImage, IN EG_IMAGE *TopImage, IN EG_IMAGE *BadgeImage, IN INTN XPos, IN INTN YPos, INTN Scale);
//VOID BltImageCompositeIndicator(IN EG_IMAGE *BaseImage, IN EG_IMAGE *TopImage, IN INTN XPos, IN INTN YPos, INTN Scale);
