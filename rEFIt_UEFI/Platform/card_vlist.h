/*
 * card_vlist.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_CARD_VLIST_H_
#define PLATFORM_CARD_VLIST_H_



VOID
FillCardList (
  TagPtr CfgDict
  );

CARDLIST
*FindCardWithIds (
  UINT32 Id,
  UINT32 SubId
  );

VOID
AddCard (
  CONST CHAR8 *Model,
  UINT32      Id,
  UINT32      SubId,
  UINT64      VideoRam,
  UINTN       VideoPorts,
  BOOLEAN     LoadVBios
  );



#endif /* PLATFORM_CARD_VLIST_H_ */
