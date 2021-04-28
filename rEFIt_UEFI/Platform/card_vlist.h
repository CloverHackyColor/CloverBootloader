/*
 * card_vlist.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_CARD_VLIST_H_
#define PLATFORM_CARD_VLIST_H_

#include "../Platform/plist/plist.h"
#include "../Platform/Settings.h"

#define CARDLIST_SIGNATURE SIGNATURE_32('C','A','R','D')


const SETTINGS_DATA::GraphicsClass::GRAPHIC_CARD*
FindCardWithIds (
  UINT32 Id,
  UINT32 SubId
  );

//void
//AddCard (
//  CONST CHAR8 *Model,
//  UINT32      Id,
//  UINT32      SubId,
//  UINT64      VideoRam,
//  UINTN       VideoPorts,
//  BOOLEAN     LoadVBios
//  );



#endif /* PLATFORM_CARD_VLIST_H_ */
