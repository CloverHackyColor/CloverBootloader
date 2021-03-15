
#ifndef __bootscreen_h__
#define __bootscreen_h__

//#include "../Platform/Settings.h"
#include "../gui/menu_items/menu_items.h"

EFI_STATUS
InitBootScreen (
  IN  LOADER_ENTRY *Entry
  );


CONST CHAR8 *CustomBootModeToStr(IN UINT8 Mode);

#endif //__bootscreen_h__

