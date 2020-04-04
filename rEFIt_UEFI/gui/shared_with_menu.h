/*
 * shared_with_menu.h
 *
 *  Created on: 4 Apr 2020
 *      Author: jief
 *
 *  THIS will most likely disappear soon !
 */

#ifndef GUI_SHARED_WITH_MENU_H_
#define GUI_SHARED_WITH_MENU_H_


// clovy - set row height based on text size
#define RowHeightFromTextHeight (1.35f)
extern INTN TextStyle; //why global? It will be class SCREEN member

extern CONST CHAR16 *VBIOS_BIN;

extern INPUT_ITEM *InputItems;

VOID DecodeOptions(REFIT_MENU_ITEM_BOOTNUM *Entry);
UINT32 EncodeOptions(CONST CHAR16 *Options);



#endif /* GUI_SHARED_WITH_MENU_H_ */
