/*
 * refit/menu.c
 * Menu functions
 *
 * Copyright (c) 2006 Christoph Pfisterer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Christoph Pfisterer nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *
 *
 *  THIS will most likely disappear soon !
 */

#include "../Platform/Platform.h"
#include "../refit/lib.h"
#include "menu_items/menu_items.h"

CONST CHAR16* ArgOptional[NUM_OPT] = {
  L"arch=i386",       //0
  L"arch=x86_64",     //1
  L"-v ",             //2
  L"-f ",             //3
  L"-s ",             //4
  L"-x ",             //5
  L"nv_disable=1",    //6
  L"slide=0",         //7
  L"darkwake=0",      //8
  L"-xcpm",           //9
  L"-gux_no_idle",    //10
  L"-gux_nosleep",    //11
  L"-gux_nomsi",      //12
  L"-gux_defer_usb2", //13
  L"keepsyms=1",      //14
  L"debug=0x100",     //15
  L"kextlog=0xffff",  //16
  L"-alcoff",         //17
  L"-shikioff",       //18
  L"nvda_drv=1"       //19
};
CONST CHAR16 *VBIOS_BIN = L"EFI\\CLOVER\\misc\\c0000.bin";

INPUT_ITEM *InputItems = NULL;
INTN TextStyle; //why global? It will be class SCREEN member

UINT32 EncodeOptions(CONST CHAR16 *Options)
{
  UINT32 OptionsBits = 0;
  INTN Index;
  if (!Options) {
    return 0;
  }
  for (Index = 0; Index < NUM_OPT; Index++) {
    if (StrStr(Options, ArgOptional[Index])) {
      OptionsBits |= (1 << Index);
      if (Index == 1) {
        OptionsBits &= ~1;
      }
    }
  }
  return OptionsBits;
}

VOID DecodeOptions(REFIT_MENU_ITEM_BOOTNUM *Entry)
{
  //set checked option
  INTN Index;
  if (!Entry) {
    return;
  }
  for (Index = 0; Index < INX_NVWEBON; Index++) { //not including INX_NVWEBON
    if (gSettings.OptionsBits & (1 << Index)) {
      Entry->LoadOptions = AddLoadOption(Entry->LoadOptions, ArgOptional[Index]);
    }
  }
  //remove unchecked options
  for (Index = 0; Index < INX_NVWEBON; Index++) { //not including INX_NVWEBON
    if ((gSettings.OptionsBits & (1 << Index)) == 0) {
      Entry->LoadOptions = RemoveLoadOption(Entry->LoadOptions, ArgOptional[Index]);
    }
  }

  if (Entry->getLOADER_ENTRY()) {
    LOADER_ENTRY* loaderEntry = Entry->getLOADER_ENTRY();
    // Only for non-legacy entries, as LEGACY_ENTRY doesn't have OSVersion
    if (gSettings.OptionsBits & OPT_NVWEBON) {
      if (AsciiOSVersionToUint64(loaderEntry->OSVersion) >= AsciiOSVersionToUint64("10.12")) {
        gSettings.NvidiaWeb = TRUE;
      } else {
        Entry->LoadOptions = AddLoadOption(loaderEntry->LoadOptions, ArgOptional[INX_NVWEBON]);
      }
    }
    if ((gSettings.OptionsBits & OPT_NVWEBON) == 0) {
      if (AsciiOSVersionToUint64(loaderEntry->OSVersion) >= AsciiOSVersionToUint64("10.12")) {
        gSettings.NvidiaWeb = FALSE;
      } else {
        Entry->LoadOptions = RemoveLoadOption(loaderEntry->LoadOptions, ArgOptional[INX_NVWEBON]);
      }
    }
  }
}

