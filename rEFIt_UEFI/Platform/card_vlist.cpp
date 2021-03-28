/*
 * Copyright (c) 2012 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 *
 */

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "nvidia.h"
#include "../Platform/Settings.h"

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

//LIST_ENTRY gCardList = INITIALIZE_LIST_HEAD_VARIABLE (gCardList);


//void AddCard(CONST CHAR8* Model, UINT32 Id, UINT32 SubId, UINT64 VideoRam, UINTN VideoPorts, BOOLEAN LoadVBios)
//{
//	CARDLIST* new_card = new CARDLIST;
//  new_card->Signature = CARDLIST_SIGNATURE;
//  new_card->Id = Id;
//  new_card->SubId = SubId;
//  new_card->VideoRam = VideoRam;
//  new_card->VideoPorts = VideoPorts;
//  new_card->LoadVBios = LoadVBios;
//  new_card->Model.takeValueFrom(Model);
//  gCardList.AddReference(new_card, true);
//}

const SETTINGS_DATA::GraphicsClass::GRAPHIC_CARD* FindCardWithIds(UINT32 Id, UINT32 SubId)
{
  for ( size_t idx = 0; idx < gSettings.Graphics.ATICardList.size(); ++idx ) {
    const SETTINGS_DATA::GraphicsClass::GRAPHIC_CARD& entry = gSettings.Graphics.ATICardList[idx];
    if(entry.Id == Id) {
      return &entry;
    }
  }
  for ( size_t idx = 0; idx < gSettings.Graphics.NVIDIACardList.size(); ++idx ) {
    const SETTINGS_DATA::GraphicsClass::GRAPHIC_CARD& entry = gSettings.Graphics.NVIDIACardList[idx];
    if(entry.Id == Id) {
      return &entry;
    }
  }
  return NULL;
}

/*
* To ease copy/paste and text replacement from GetUserSettings, the parameter has the same name as the global
* and is passed by non-const reference.
* This temporary during the refactoring
*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
void FillCardList(const TagDict* CfgDict, SETTINGS_DATA& gSettings)
{
#pragma GCC diagnostic pop
  if (gSettings.Graphics.ATICardList.isEmpty() && gSettings.Graphics.NVIDIACardList.isEmpty() && (CfgDict != NULL)) {
    CONST CHAR8 *VEN[] = { "ATI", "NVIDIA" };
    XObjArray<SETTINGS_DATA::GraphicsClass::GRAPHIC_CARD>* cardlist[] = { &gSettings.Graphics.ATICardList, &gSettings.Graphics.NVIDIACardList };
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
        for (i = 0; i < count; i++)
        {
          SETTINGS_DATA::GraphicsClass::GRAPHIC_CARD* new_card = new SETTINGS_DATA::GraphicsClass::GRAPHIC_CARD;

//          CONST CHAR8     *model_name = NULL;
//          UINT32		dev_id		= 0;
//          UINT32		subdev_id	= 0;
//          UINT64		VramSize	= 0;
//          UINTN     VideoPorts  = 0;
//          BOOLEAN   LoadVBios = FALSE;
          element = &prop->arrayContent()[i];
          if ( !element->isDict()) {
            MsgLog("MALFORMED PLIST in FillCardList() : element[%lld] is not a dict\n", i);
            continue;
          }
          const TagDict* dictElement = element->getDict();
          
          prop2 = dictElement->propertyForKey("Model");
          if ( prop2 && prop2->isString() && prop2->getString()->stringValue().notEmpty() ) {
            new_card->Model = prop2->getString()->stringValue();
          } else {
            new_card->Model = "VideoCard"_XS8;
          }
          
          prop2 = dictElement->propertyForKey("IOPCIPrimaryMatch");
          new_card->Id = (UINT32)GetPropertyAsInteger(prop2, 0);
          
          prop2 = dictElement->propertyForKey("IOPCISubDevId");
          new_card->SubId = (UINT32)GetPropertyAsInteger(prop2, 0);
          
          prop2 = dictElement->propertyForKey("VRAM");
          new_card->VideoRam = LShiftU64((UINTN)GetPropertyAsInteger(prop2, 0), 20); //Mb -> bytes
          
          prop2 = dictElement->propertyForKey("VideoPorts");
          new_card->VideoPorts = (UINT16)GetPropertyAsInteger(prop2, 0);
          
          prop2 = dictElement->propertyForKey("LoadVBios");
          if (prop2 != NULL && IsPropertyNotNullAndTrue(prop2)) {
            new_card->LoadVBios = TRUE;
          }
          
          DBG("FillCardList :: %s : \"%s\" (%08X, %08X)\n", key, new_card->Model.c_str(), new_card->Id, new_card->SubId);
          
//          AddCard(model_name, dev_id, subdev_id, VramSize, VideoPorts, LoadVBios);
          new_card->Signature = CARDLIST_SIGNATURE;
//          new_card->Id = dev_id;
//          new_card->SubId = subdev_id;
//          new_card->VideoRam = VramSize;
//          new_card->VideoPorts = VideoPorts;
//          new_card->LoadVBios = LoadVBios;
//          new_card->Model.takeValueFrom(model_name);
          cardlist[Index]->AddReference(new_card, true);
        }
      }
    }
  }
}
