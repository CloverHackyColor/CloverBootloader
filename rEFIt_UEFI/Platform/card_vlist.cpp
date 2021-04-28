/*
 * Copyright (c) 2012 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 *
 */

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "nvidia.h"
#include "../Platform/Settings.h"
#include "card_vlist.h"

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
