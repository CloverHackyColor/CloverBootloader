/*
 * card_vlist.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_CARD_VLIST_H_
#define PLATFORM_CARD_VLIST_H_

#include "../Platform/plist/plist.h"


class CARDLIST {
  public:
    UINT32            Signature = 0;
    XString8          Model = XString8();
    UINT32            Id = 0;
    UINT32            SubId = 0;
    UINT64            VideoRam = 0;
    UINTN             VideoPorts = 0;
    BOOLEAN           LoadVBios = 0;

  CARDLIST() {}
  CARDLIST(const CARDLIST& other) = delete; // Can be defined if needed
  const CARDLIST& operator = ( const CARDLIST & ) = delete; // Can be defined if needed
  ~CARDLIST() {}
};


#define CARDLIST_SIGNATURE SIGNATURE_32('C','A','R','D')


void
FillCardList (
  const TagDict* CfgDict
  );

const CARDLIST*
FindCardWithIds (
  UINT32 Id,
  UINT32 SubId
  );

void
AddCard (
  CONST CHAR8 *Model,
  UINT32      Id,
  UINT32      SubId,
  UINT64      VideoRam,
  UINTN       VideoPorts,
  BOOLEAN     LoadVBios
  );



#endif /* PLATFORM_CARD_VLIST_H_ */
