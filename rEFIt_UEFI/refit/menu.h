#ifndef __MENU_H__
#define __MENU_H__

#include "../cpp_foundation/XString.h"
#include "../gui/menu_items/menu_items.h"

//VOID AddMenuInfoLine(IN REFIT_MENU_SCREEN *Screen, IN CONST CHAR16 *InfoLine);
//VOID AddMenuInfo(IN REFIT_MENU_SCREEN  *SubScreen, IN CONST CHAR16 *Line);
//VOID AddMenuEntry(IN REFIT_MENU_SCREEN *Screen, IN REFIT_MENU_ENTRY *Entry, bool freeIt);
//VOID AddMenuCheck(REFIT_MENU_SCREEN *SubScreen, CONST CHAR8 *Text, UINTN Bit, INTN ItemNum);
//VOID FreeMenu(IN REFIT_MENU_SCREEN *Screen);
//UINTN RunMenu(IN REFIT_MENU_SCREEN *Screen, OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);
//UINTN RunMainMenu(IN REFIT_MENU_SCREEN *Screen, IN INTN DefaultSelection, OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);

//included into SCREEN
/*
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
 */
extern INTN LayoutAnimMoveForMenuX;
extern INTN LayoutMainMenuHeight;


VOID OptionsMenu(OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);
VOID FreeScrollBar(VOID);

VOID
FillInputs (
  BOOLEAN New
  );

VOID
ApplyInputs (VOID);




#endif

