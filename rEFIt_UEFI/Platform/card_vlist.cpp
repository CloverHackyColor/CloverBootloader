/*
 * Copyright (c) 2012 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 *
 */

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "nvidia.h"


/*
 injection for NVIDIA card usage e.g (to be placed in the config.plist, under graphics tag): 
 <key>Graphics</key>
 <dict>
    <key>NVIDIA</key>
    <array>
      <dict>
        <key>Model</key>
        <string>Quadro FX 380</string>
        <key>IOPCIPrimaryMatch</key>
        <string>0x10DE0658</string>
        <key>VRAM</key>
        <integer>256</integer>
        <key>VideoPorts</key>
        <integer>2</integer>
        <key>LoadVBios</key>
        <true/>
      </dict>
    <dict>
      <key>Model</key>
      <string>YOUR_SECOND_CARD_NAME</string>
      <key>IOPCIPrimaryMatch</key>
      <string>YOUR_SECOND_CARD_ID</string>
      <key>IOPCISubDevId</key>
      <string>YOUR_SECOND_CARD_SUB_ID(if necessary)</string>
      <key>VRAM</key>
      <integer>YOUR_SECOND_CARD_VRAM_SIZE</integer>
      <key>VideoPorts</key>
      <integer>YOUR_SECOND_CARD_PORTS</integer>
      <key>LoadVBios</key>
      <true/><!--YOUR_SECOND_CARD_LOADVBIOS-->
    </dict>
  </array>
    <key>ATI</key>
    <array>
      <dict>
        <key>Model</key>
        <string>ATI Radeon HD6670</string>
        <key>IOPCIPrimaryMatch</key>
        <string>0x6758</string>
        <key>IOPCISubDevId</key>
        <string>0x1342</string>
        <key>VRAM</key>
        <integer>2048</integer>
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


void AddCard(CONST CHAR8* Model, UINT32 Id, UINT32 SubId, UINT64 VideoRam, UINTN VideoPorts, BOOLEAN LoadVBios)
{
	CARDLIST* new_card;		
	new_card = (__typeof__(new_card))AllocateZeroPool(sizeof(CARDLIST));
	if (new_card) {	
    new_card->Signature = CARDLIST_SIGNATURE;
	  new_card->Id = Id;
	  new_card->SubId = SubId;
	  new_card->VideoRam = VideoRam;
    new_card->VideoPorts = VideoPorts;
    new_card->LoadVBios = LoadVBios;
	  snprintf(new_card->Model, 64, "%s", Model);
    InsertTailList (&gCardList, (LIST_ENTRY *)(((UINT8 *)new_card) + OFFSET_OF(CARDLIST, Link)));
	}	
}

CARDLIST* FindCardWithIds(UINT32 Id, UINT32 SubId)
{
  LIST_ENTRY		*Link;
  CARDLIST      *entry;
//  FillCardList(); //moved to GetUserSettings
  
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

void FillCardList(const TagDict* CfgDict)
{
  if (IsListEmpty(&gCardList) && (CfgDict != NULL)) {
    CONST CHAR8 *VEN[] = { "NVIDIA",  "ATI" };
    size_t Count = sizeof(VEN) / sizeof(VEN[0]);
    
    for (size_t Index = 0; Index < Count; Index++) {
      CONST CHAR8 *key = VEN[Index];
      
      const TagArray* prop = CfgDict->arrayPropertyForKey(key);
      if( prop  &&  prop->isArray() ) {
        INTN		i;
        INTN		 count;
        
        const TagStruct*		prop2		= 0;
        const TagStruct*    element    = 0;
        count = prop->arrayContent().size();
        for (i = 0; i < count; i++) {
          CONST CHAR8     *model_name = NULL;
          UINT32		dev_id		= 0;
          UINT32		subdev_id	= 0;
          UINT64		VramSize	= 0;
          UINTN     VideoPorts  = 0;
          BOOLEAN   LoadVBios = FALSE;
          element = &prop->arrayContent()[i];
          if ( !element->isDict()) {
            MsgLog("MALFORMED PLIST in FillCardList() : element is not a dict");
            continue;
          }
          const TagDict* dictElement = element->getDict();
          
          prop2 = dictElement->propertyForKey("Model");
          if ( prop2->isString() && prop2->getString()->stringValue().notEmpty() ) {
            model_name = prop2->getString()->stringValue().c_str();
          } else {
            model_name = "VideoCard";
          }
          
          prop2 = dictElement->propertyForKey("IOPCIPrimaryMatch");
          dev_id = (UINT32)GetPropertyAsInteger(prop2, 0);
          
          prop2 = dictElement->propertyForKey("IOPCISubDevId");
          subdev_id = (UINT32)GetPropertyAsInteger(prop2, 0);
          
          prop2 = dictElement->propertyForKey("VRAM");
          VramSize = LShiftU64((UINTN)GetPropertyAsInteger(prop2, (INTN)VramSize), 20); //Mb -> bytes
          
          prop2 = dictElement->propertyForKey("VideoPorts");
          VideoPorts = (UINT16)GetPropertyAsInteger(prop2, VideoPorts);
          
          prop2 = dictElement->propertyForKey("LoadVBios");
          if (prop2 != NULL && IsPropertyNotNullAndTrue(prop2)) {
            LoadVBios = TRUE;
          }
          
          DBG("FillCardList :: %s : \"%s\" (%08X, %08X)\n", key, model_name, dev_id, subdev_id);
          
          AddCard(model_name, dev_id, subdev_id, VramSize, VideoPorts, LoadVBios);
        }
      }
    }
  }
}
