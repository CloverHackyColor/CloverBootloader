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


VOID AddCard(CONST CHAR8* Model, UINT32 Id, UINT32 SubId, UINT64 VideoRam, UINTN VideoPorts, BOOLEAN LoadVBios)
{
	CARDLIST* new_card;		
	new_card = (__typeof__(new_card))BllocateZeroPool(sizeof(CARDLIST));
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

VOID FillCardList(TagPtr CfgDict)
{
  if (IsListEmpty(&gCardList) && (CfgDict != NULL)) {
    CONST CHAR8 *VEN[] = { "NVIDIA",  "ATI" };
    INTN Index, Count = sizeof(VEN) / sizeof(VEN[0]);
    TagPtr      prop;
    
    for (Index = 0; Index < Count; Index++) {
      CONST CHAR8 *key = VEN[Index];
      
      prop = GetProperty(CfgDict, key);
      if(prop && (prop->type == kTagTypeArray)) {
        INTN		i;
        INTN		 count;
        
        TagPtr		element		= 0;
        TagPtr		prop2		= 0;
        count = GetTagCount(prop);
        for (i = 0; i < count; i++) {
          CONST CHAR8     *model_name = NULL;
          UINT32		dev_id		= 0;
          UINT32		subdev_id	= 0;
          UINT64		VramSize	= 0;
          UINTN     VideoPorts  = 0;
          BOOLEAN   LoadVBios = FALSE;
          EFI_STATUS status = GetElement(prop, i, &element);
          
          if (status == EFI_SUCCESS) {
            if (element) {
              if ((prop2 = GetProperty(element, "Model")) != 0) {
                model_name = prop2->string;
              } else {
                model_name = "VideoCard";
              }
              
              prop2 = GetProperty(element, "IOPCIPrimaryMatch");
              dev_id = (UINT32)GetPropertyInteger(prop2, 0);
              
              prop2 = GetProperty(element, "IOPCISubDevId");
              subdev_id = (UINT32)GetPropertyInteger(prop2, 0);
              
              prop2 = GetProperty(element, "VRAM");
              VramSize = LShiftU64((UINTN)GetPropertyInteger(prop2, (INTN)VramSize), 20); //Mb -> bytes
              
              prop2 = GetProperty(element, "VideoPorts");
              VideoPorts = (UINT16)GetPropertyInteger(prop2, VideoPorts);
              
              prop2 = GetProperty(element, "LoadVBios");
              if (prop2 != NULL && IsPropertyTrue(prop2)) {
                LoadVBios = TRUE;
              }
              
              DBG("FillCardList :: %s : \"%s\" (%08X, %08X)\n", key, model_name, dev_id, subdev_id);
              
              AddCard(model_name, dev_id, subdev_id, VramSize, VideoPorts, LoadVBios);
            }
            
          }
        }
      }
    }
  }
}
