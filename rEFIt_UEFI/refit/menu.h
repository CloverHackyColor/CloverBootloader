#ifndef __MENU_H__
#define __MENU_H__

#include "../cpp_foundation/XStringW.h"
#include "../gui/menu_items/menu_items.h"

//VOID AddMenuInfoLine(IN REFIT_MENU_SCREEN *Screen, IN CONST CHAR16 *InfoLine);
//VOID AddMenuInfo(IN REFIT_MENU_SCREEN  *SubScreen, IN CONST CHAR16 *Line);
//VOID AddMenuEntry(IN REFIT_MENU_SCREEN *Screen, IN REFIT_MENU_ENTRY *Entry, bool freeIt);
//VOID AddMenuCheck(REFIT_MENU_SCREEN *SubScreen, CONST CHAR8 *Text, UINTN Bit, INTN ItemNum);
//VOID FreeMenu(IN REFIT_MENU_SCREEN *Screen);
//UINTN RunMenu(IN REFIT_MENU_SCREEN *Screen, OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);
//UINTN RunMainMenu(IN REFIT_MENU_SCREEN *Screen, IN INTN DefaultSelection, OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);

extern EG_IMAGE* ScrollbarImage;
extern EG_IMAGE* UpButtonImage;
extern EG_IMAGE* DownButtonImage;
extern EG_IMAGE* ScrollbarBackgroundImage;
extern EG_IMAGE* BarStartImage;
extern EG_IMAGE* BarEndImage;
extern EG_IMAGE* ScrollStartImage;
extern EG_IMAGE* ScrollEndImage;

extern EG_RECT UpButton;
extern EG_RECT DownButton;
extern EG_RECT BarStart;
extern EG_RECT BarEnd;
extern EG_RECT ScrollbarBackground;
extern EG_RECT Scrollbar;
extern EG_RECT ScrollStart;
extern EG_RECT ScrollEnd;
extern EG_RECT ScrollTotal;
extern EG_RECT ScrollbarOldPointerPlace;
extern EG_RECT ScrollbarNewPointerPlace;

extern INTN LayoutAnimMoveForMenuX;
extern INTN LayoutMainMenuHeight;


extern INTN ScrollWidth;



#if !USE_XTHEME
VOID InitBar(VOID);
VOID FillRectAreaOfScreen(IN INTN XPos, IN INTN YPos, IN INTN Width, IN INTN Height, IN EG_PIXEL *Color, IN UINT8 XAlign);
VOID InitSelection(VOID);
#endif


VOID OptionsMenu(OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);
VOID FreeScrollBar(VOID);
#if USE_XTHEME
//it will be REFIT_SCREEN MEMBER, others as well?
#else
INTN DrawTextXY(IN CONST CHAR16 *Text, IN INTN XPos, IN INTN YPos, IN UINT8 XAlign);
VOID DrawMenuText(IN CONST CHAR16 *Text, IN INTN SelectedWidth, IN INTN XPos, IN INTN YPos, IN INTN Cursor);
VOID DrawMainMenuEntry(REFIT_ABSTRACT_MENU_ENTRY *Entry, BOOLEAN selected, INTN XPos, INTN YPos);
VOID DrawBCSText(IN CONST CHAR16 *Text, IN INTN XPos, IN INTN YPos, IN UINT8 XAlign);
VOID DrawTextCorner(UINTN TextC, UINT8 Align);
#endif

UINT64 TimeDiff(UINT64 t0, UINT64 t1); //double in Platform.h




#endif

