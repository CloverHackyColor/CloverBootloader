
//VOID AddMenuInfoLine(IN REFIT_MENU_SCREEN *Screen, IN CONST CHAR16 *InfoLine);
//VOID AddMenuInfo(IN REFIT_MENU_SCREEN  *SubScreen, IN CONST CHAR16 *Line);
//VOID AddMenuEntry(IN REFIT_MENU_SCREEN *Screen, IN REFIT_MENU_ENTRY *Entry, bool freeIt);
//VOID AddMenuCheck(REFIT_MENU_SCREEN *SubScreen, CONST CHAR8 *Text, UINTN Bit, INTN ItemNum);
//VOID FreeMenu(IN REFIT_MENU_SCREEN *Screen);
//UINTN RunMenu(IN REFIT_MENU_SCREEN *Screen, OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);
//UINTN RunMainMenu(IN REFIT_MENU_SCREEN *Screen, IN INTN DefaultSelection, OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);

VOID OptionsMenu(OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);
VOID FreeScrollBar(VOID);
#if USE_XTHEME
INTN DrawTextXY(IN XStringW& Text, IN INTN XPos, IN INTN YPos, IN UINT8 XAlign);
VOID DrawMenuText(IN XStringW& Text, IN INTN SelectedWidth, IN INTN XPos, IN INTN YPos, IN INTN Cursor);
#else
INTN DrawTextXY(IN CONST CHAR16 *Text, IN INTN XPos, IN INTN YPos, IN UINT8 XAlign);
VOID DrawMenuText(IN CONST CHAR16 *Text, IN INTN SelectedWidth, IN INTN XPos, IN INTN YPos, IN INTN Cursor);
#endif
VOID DrawBCSText(IN CONST CHAR16 *Text, IN INTN XPos, IN INTN YPos, IN UINT8 XAlign);

UINT64 TimeDiff(UINT64 t0, UINT64 t1); //double in Platform.h

