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

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "../refit/lib.h"
#include "menu_items/menu_items.h"
#include "../Platform/Settings.h"

CONST XString8 ArgOptional[NUM_OPT] = {
  "arch=i386"_XS8,       //0
  "arch=x86_64"_XS8,     //1
  "-v"_XS8,             //2
  "-f"_XS8,             //3
  "-s"_XS8,             //4
  "-x"_XS8,             //5
  "nv_disable=1"_XS8,    //6
  "slide=0"_XS8,         //7
  "darkwake=0"_XS8,      //8
  "-xcpm"_XS8,           //9
  "-gux_no_idle"_XS8,    //10
  "-gux_nosleep"_XS8,    //11
  "-gux_nomsi"_XS8,      //12
  "-gux_defer_usb2"_XS8, //13
  "keepsyms=1"_XS8,      //14
  "debug=0x100"_XS8,     //15
  "kextlog=0xffff"_XS8,  //16
  "-alcoff"_XS8,         //17
  "-shikioff"_XS8,       //18
  "nvda_drv=1"_XS8       //19
};
CONST CHAR16 *VBIOS_BIN = L"misc\\c0000.bin";

INPUT_ITEM *InputItems = NULL;
INTN TextStyle; //why global? It will be class SCREEN member

UINT32 EncodeOptions(const XString8Array& Options)
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

void DecodeOptions(REFIT_MENU_ITEM_BOOTNUM *Entry)
{
  //set checked option
  INTN Index;
  if (!Entry) {
    return;
  }
  for (Index = 0; Index < INX_NVWEBON; Index++) { //not including INX_NVWEBON
    if (gSettings.OptionsBits & (1 << Index)) {
      Entry->LoadOptions.AddID(ArgOptional[Index]);
    }
  }
  //remove unchecked options
  for (Index = 0; Index < INX_NVWEBON; Index++) { //not including INX_NVWEBON
    if ((gSettings.OptionsBits & (1 << Index)) == 0) {
      Entry->LoadOptions.remove(ArgOptional[Index]);
    }
  }

  if (Entry->getLOADER_ENTRY()) {
    LOADER_ENTRY* loaderEntry = Entry->getLOADER_ENTRY();
    // Only for non-legacy entries, as LEGACY_ENTRY doesn't have OSVersion
    if (gSettings.OptionsBits & OPT_NVWEBON) {
      if ( loaderEntry->macOSVersion >= MacOsVersion("10.12"_XS8) ) {
        gSettings.NvidiaWeb = TRUE;
      } else {
        //Entry->LoadOptions = loaderEntry->LoadOptions;
//        Entry->LoadOptions = Split<XString8Array>(loaderEntry->LoadOptions.ConcatAll(" "_XS8).wc_str(), " ");
        Entry->LoadOptions.AddID(ArgOptional[INX_NVWEBON]);
      }
    }
    if ((gSettings.OptionsBits & OPT_NVWEBON) == 0) {
      if ( loaderEntry->macOSVersion >= MacOsVersion("10.12"_XS8)) {
        gSettings.NvidiaWeb = FALSE;
      } else {
        //Entry->LoadOptions = loaderEntry->LoadOptions;
//        Entry->LoadOptions = Split<XString8Array>(loaderEntry->LoadOptions.ConcatAll(" "_XS8).wc_str(), " ");
        Entry->LoadOptions.removeIC(ArgOptional[INX_NVWEBON]);
      }
    }
  }
}

