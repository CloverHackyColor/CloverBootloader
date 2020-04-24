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

#include <Platform.h>
//#include "../refit/lib.h"
//#include "menu_items/menu_items.h"
//#include "../entry_scan/common.h"

CONST XString ArgOptional[NUM_OPT] = {
  "arch=i386"_XS,       //0
  "arch=x86_64"_XS,     //1
  "-v "_XS,             //2
  "-f "_XS,             //3
  "-s "_XS,             //4
  "-x "_XS,             //5
  "nv_disable=1"_XS,    //6
  "slide=0"_XS,         //7
  "darkwake=0"_XS,      //8
  "-xcpm"_XS,           //9
  "-gux_no_idle"_XS,    //10
  "-gux_nosleep"_XS,    //11
  "-gux_nomsi"_XS,      //12
  "-gux_defer_usb2"_XS, //13
  "keepsyms=1"_XS,      //14
  "debug=0x100"_XS,     //15
  "kextlog=0xffff"_XS,  //16
  "-alcoff"_XS,         //17
  "-shikioff"_XS,       //18
  "nvda_drv=1"_XS       //19
};
CONST CHAR16 *VBIOS_BIN = L"EFI\\CLOVER\\misc\\c0000.bin";

INPUT_ITEM *InputItems = NULL;
INTN TextStyle; //why global? It will be class SCREEN member

UINT32 EncodeOptions(const XString& Options)
{
  UINT32 OptionsBits = 0;
  INTN Index;
  if (Options.isEmpty()) {
    return 0;
  }
  for (Index = 0; Index < NUM_OPT; Index++) {
    if ( Options.contains(ArgOptional[Index]) ) {
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

