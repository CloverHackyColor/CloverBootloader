/*
 * Copyright (c) 2012 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 *
 */

#include "Platform.h"
#include "nvidia.h"


/*
 injection for NVIDIA card usage e.g (to be placed in the config.plist, under graphics tag): 
	<key>Graphics</key>
	<dict>
		<key>NVIDIA</key>
		<array>
			<dict>
				<key>Chipset Name</key>
				<string>Quadro FX 380</string>
				<key>IOPCIPrimaryMatch</key>
				<string>0x10DE0658</string>
				<key>VRam Size</key>
				<string>256</string>
			</dict>
			<dict>
				<key>Chipset Name</key>
				<string>ATI Radeon HD6670</string>
				<key>IOPCIPrimaryMatch</key>
				<string>0x6758</string>
				<key>IOPCISubDevId</key>
				<string>0x1342</string>
				<key>VRam Size</key>
				<integer>2048</integer>
			</dict>
			<dict>
				<key>Chipset Name</key>
				<string>YOUR_SECOND_CARD_NAME</string>
				<key>IOPCIPrimaryMatch</key>
				<string>YOUR_SECOND_CARD_ID</string>
				<key>IOPCISubDevId</key>
				<string>YOUR_SECOND_CARD_SUB_ID(if necessary)</string>
				<key>VRam Size</key>
				<string>YOUR_SECOND_CARD_VRAM_SIZE</string>
			</dict>
		</array>
	</dict>
*/

#define DEBUG_CARD_VLIST 1

#if DEBUG_CARD_VLIST == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_CARD_VLIST, __VA_ARGS__)
#endif

LIST_ENTRY gCardList = INITIALIZE_LIST_HEAD_VARIABLE (gCardList);


VOID AddCard(CONST CHAR8* Model, UINT32 Id, UINT32 SubId, UINT64 VideoRam)
{
	CARDLIST* new_card;		
	new_card = AllocateZeroPool(sizeof(CARDLIST));
	if (new_card) {	
      new_card->Signature = CARDLIST_SIGNATURE;		
		
	  new_card->Id = Id;
	  new_card->SubId = SubId;
	  new_card->VideoRam = VideoRam;
	  AsciiSPrint(new_card->Model, 64, "%a", Model);    
      InsertTailList (&gCardList, (LIST_ENTRY *)(((UINT8 *)new_card) + OFFSET_OF(CARDLIST, Link)));
	}	
}

CARDLIST* FindCardWithIds(UINT32 Id, UINT32 SubId)
{
  LIST_ENTRY		*Link;
  CARDLIST      *entry;
  FillCardList();
  
  if(!IsListEmpty(&gCardList)) {
	for (Link = gCardList.ForwardLink; Link != &gCardList; Link = Link->ForwardLink) {
	entry = CR(Link, CARDLIST, Link, CARDLIST_SIGNATURE);
      
      if(entry->Id == Id) {
        return entry;
      }	
    }
  }
  
  return NULL;
}

VOID FillCardList(VOID) 
{
  TagPtr      dict;
  TagPtr      prop;
  TagPtr      dictPointer;
  if (IsListEmpty(&gCardList)) {
	INTN cfgN = NUM_OF_CONFIGS-1;
    for (; cfgN > 0; cfgN--) {
      dict = gConfigDict[cfgN];
      if(dict != NULL) {
        dictPointer = GetProperty(dict, "Graphics");
        if (dictPointer) {
          prop = GetProperty(dictPointer, "NVIDIA");
          if(prop && (prop->type == kTagTypeArray)) {
            INTN		i;
            INTN		 count;

            TagPtr		element		= 0;
            TagPtr		prop2		= 0;
            count = GetTagCount(prop);

            for (i=0; i<count; i++) {
              CHAR8		*model_name;
              CHAR8		*match_id;
              CHAR8		*sub_id;
              CHAR8		*vram_size;
              UINT32		dev_id		= 0;
              UINT32		subdev_id	= 0;
              UINT64		VramSize	= 0;
              EFI_STATUS status = GetElement(prop, i, &element);

              if (status == EFI_SUCCESS) {
                if (element) {
                  model_name	= NULL;
                  match_id	= NULL;
                  sub_id		= NULL;
                  vram_size	= NULL;

                  if ((prop2 = GetProperty(element, "Chipset Name")) != 0) {
                    model_name = prop2->string;
                  }

                  if ((prop2 = GetProperty(element, "IOPCIPrimaryMatch")) != 0) {
                    match_id = prop2->string;
                  }

                  if ((prop2 = GetProperty(element, "IOPCISubDevId")) != 0) {
                    sub_id = prop2->string;
                  }

                  if ((prop2 = GetProperty(element, "VRam Size")) != 0) {
                    vram_size = prop2->string;
                  }

                  if (match_id) {
                    if ((match_id[0] == '0')  &&
                        (match_id[1] == 'x' || match_id[1] == 'X')) {
                      dev_id = (UINT32)AsciiStrHexToUintn(match_id);
                    } else {
                      dev_id = (UINT32)AsciiStrDecimalToUintn(match_id);
                    }
                  }

                  if (sub_id) {
                    if ((sub_id[0] == '0')  &&
                        (sub_id[1] == 'x' || sub_id[1] == 'X')) {
                      subdev_id = (UINT32)AsciiStrHexToUintn(sub_id);
                    } else {
                      subdev_id = (UINT32)AsciiStrDecimalToUintn(sub_id);
                    }
                  }
                  
                  if (vram_size) {
                    if ((vram_size[0] == '0')  && 
                        (vram_size[1] == 'x' || vram_size[1] == 'X')) {
                      VramSize = AsciiStrHexToUintn(vram_size);
                    } else {
                      VramSize = AsciiStrDecimalToUintn(vram_size);
                    }
                  }
                  AddCard(model_name, dev_id, subdev_id, VramSize);                 
                }
              }              
            }					
            return;
          } 
		} 
      }
	}
  }
}
