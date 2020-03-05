/*
 * refit/icns.c
 * Loader for .icns icon files
 *
 * Copyright (c) 2006-2007 Christoph Pfisterer
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
 */

#include "libegint.h"
//#include "../include/egemb_icons.h"

#ifndef DEBUG_ALL
#define DEBUG_ICNS 1
#else
#define DEBUG_ICNS DEBUG_ALL
#endif

#if DEBUG_ICNS == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_ICNS, __VA_ARGS__)
#endif

//
// well-known icons
//

BUILTIN_ICON BuiltinIconTable[] = {
  { NULL, L"icons\\func_about"             , L"png",  /*48*/32 },
  { NULL, L"icons\\func_options"           , L"png",  /*48*/32 },
  { NULL, L"icons\\func_clover"            , L"png",  /*48*/32 },
  { NULL, L"icons\\func_secureboot"        , L"png",  /*48*/32 },
  { NULL, L"icons\\func_secureboot_config" , L"png",  /*48*/32 },
  { NULL, L"icons\\func_reset"             , L"png",  /*48*/32 },
  { NULL, L"icons\\func_shutdown"          , L"png",  /*48*/32 },
  { NULL, L"icons\\func_help"              , L"png",  /*48*/32 },
  { NULL, L"icons\\tool_shell"             , L"png",  /*48*/32 },
  { NULL, L"icons\\tool_part"              , L"png",  /*48*/32 },
  { NULL, L"icons\\tool_rescue"            , L"png",  /*48*/32 },
  { NULL, L"icons\\pointer"                , L"png",  /*48*/32 },//11

  { NULL, L"icons\\vol_internal"           , L"icns", 128 },
  { NULL, L"icons\\vol_external"           , L"icns", 128 },
  { NULL, L"icons\\vol_optical"            , L"icns", 128 },
  { NULL, L"icons\\vol_firewire"           , L"icns", 128 },
  { NULL, L"icons\\vol_clover"             , L"icns", 128 },
  { NULL, L"icons\\vol_internal_hfs"       , L"icns", 128 },
  { NULL, L"icons\\vol_internal_apfs"      , L"icns", 128 },
  { NULL, L"icons\\vol_internal_ntfs"      , L"icns", 128 },
  { NULL, L"icons\\vol_internal_ext3"      , L"icns", 128 },
  { NULL, L"icons\\vol_recovery"           , L"icns", 128 },//21

  { NULL, L"logo"                          , L"png",  128 },
  { NULL, L"selection_small"               , L"png",  64  },
  { NULL, L"selection_big"                 , L"png",  144 },
  { NULL, NULL                 , NULL,  0 }
};
/*
typedef struct MISC_ICONS {
  EG_IMAGE *image;
  CHAR8    *name;
} MISC_ICONS;
*/
MISC_ICONS OSIconsTable[] = {
  {NULL, "os_mac"},  //0
  {NULL, "os_tiger"},
  {NULL, "os_leo"},
  {NULL, "os_snow"},
  {NULL, "os_lion"},
  {NULL, "os_cougar"},
  {NULL, "os_mav"},
  {NULL, "os_yos"},
  {NULL, "os_cap"},
  {NULL, "os_sierra"},
  {NULL, "os_hsierra"},
  {NULL, "os_moja"},  //11
  {NULL, "os_cata"},  //12
  {NULL, "os_linux"},
  {NULL, "os_ubuntu"},
  {NULL, "os_suse"},
  {NULL, "os_freebsd"}, //16
  {NULL, "os_freedos"},
  {NULL, "os_win"},
  {NULL, "os_vista"},
  {NULL, "radio_button"},
  {NULL, "radio_button_selected"},
  {NULL, "checkbox"},
  {NULL, "checkbox_checked"},
  {NULL, "scrollbar_background"}, //24
  {NULL, "scrollbar_holder"},
//  {NULL, "selection_indicator"},
  {NULL, NULL}
};

//#define DEC_BUILTIN_ICON(id, ico) BuiltinIconTable[id].Image = egDecodePNG(ico, sizeof(ico), BuiltinIconTable[id].PixelSize, TRUE)
//#define DEC_BUILTIN_ICON(id, ico) BuiltinIconTable[id].Image = egDecodePNG(&ico[0], SZ_##ico, TRUE)
#define DEC_BUILTIN_ICON(id, ico) BuiltinIconTable[id].Image = egDecodePNG(ACCESS_EMB_DATA(ico), ACCESS_EMB_SIZE(ico), TRUE)

CHAR16 * GetIconsExt(IN CONST CHAR16 *Icon, IN CONST CHAR16 *Def)
{
  return PoolPrint(L"%s.%s", Icon, ((GlobalConfig.IconFormat != ICON_FORMAT_DEF) && (IconFormat != NULL)) ? IconFormat : Def);
}

EG_IMAGE * BuiltinIcon(IN UINTN Id)
{
  INTN      Size;
  EG_IMAGE  *TextBuffer = NULL;
  CONST CHAR16    *p;
  CHAR16    *Text;
//  DBG("Take image for %d\n", Id);
  if (Id >= BUILTIN_ICON_COUNT) {
    return NULL;
  }

  if (BuiltinIconTable[Id].Image != NULL) {
//    DBG(" ... the image present\n");
    return BuiltinIconTable[Id].Image;
  }

  Size = BuiltinIconTable[Id].PixelSize;
//  DBG("Load Icon [id:%d]");

  if (ThemeDir && !GlobalConfig.TypeSVG) {
    CHAR16    *Path;
    Path = GetIconsExt(BuiltinIconTable[Id].Path, BuiltinIconTable[Id].Format);
    BuiltinIconTable[Id].Image = LoadIcnsFallback(ThemeDir, Path, Size);
    if (!BuiltinIconTable[Id].Image) {
      DebugLog(1, "        [!] Icon %d (%s) not found (path: %s)\n", Id, Path, ThemePath);
      if (Id >= BUILTIN_ICON_VOL_INTERNAL) {
        FreePool(Path);
        Path = GetIconsExt(BuiltinIconTable[BUILTIN_ICON_VOL_INTERNAL].Path, BuiltinIconTable[BUILTIN_ICON_VOL_INTERNAL].Format);
        BuiltinIconTable[Id].Image = LoadIcnsFallback(ThemeDir, Path, Size);
      }
    }
    FreePool(Path);
  }

  if (BuiltinIconTable[Id].Image) {

    return BuiltinIconTable[Id].Image;
  }

  if (GlobalConfig.DarkEmbedded) {
    switch (Id) {
      case BUILTIN_ICON_POINTER:
        DEC_BUILTIN_ICON(Id, emb_pointer); break;
      case BUILTIN_ICON_FUNC_ABOUT:
        DEC_BUILTIN_ICON(Id, emb_dark_func_about); break;
      case BUILTIN_ICON_FUNC_OPTIONS:
        DEC_BUILTIN_ICON(Id, emb_dark_func_options); break;
      case BUILTIN_ICON_FUNC_CLOVER:
        DEC_BUILTIN_ICON(Id, emb_dark_func_clover); break;
      case BUILTIN_ICON_FUNC_SECURE_BOOT:
        DEC_BUILTIN_ICON(Id, emb_dark_func_secureboot); break;
      case BUILTIN_ICON_FUNC_SECURE_BOOT_CONFIG:
        DEC_BUILTIN_ICON(Id, emb_dark_func_secureboot_config); break;
      case BUILTIN_ICON_FUNC_RESET:
        DEC_BUILTIN_ICON(Id, emb_dark_func_reset); break;
      case BUILTIN_ICON_FUNC_EXIT:
        DEC_BUILTIN_ICON(Id, emb_dark_func_exit); break;
      case BUILTIN_ICON_FUNC_HELP:
        DEC_BUILTIN_ICON(Id, emb_dark_func_help); break;
      case BUILTIN_ICON_TOOL_SHELL:
        DEC_BUILTIN_ICON(Id, emb_dark_func_shell); break;
      case BUILTIN_ICON_VOL_INTERNAL:
        DEC_BUILTIN_ICON(Id, emb_dark_vol_internal); break;
      case BUILTIN_ICON_VOL_EXTERNAL:
        DEC_BUILTIN_ICON(Id, emb_dark_vol_external); break;
      case BUILTIN_ICON_VOL_OPTICAL:
        DEC_BUILTIN_ICON(Id, emb_dark_vol_optical); break;
      case BUILTIN_ICON_VOL_BOOTER:
        DEC_BUILTIN_ICON(Id, emb_dark_vol_internal_booter); break;
      case BUILTIN_ICON_VOL_INTERNAL_HFS:
        DEC_BUILTIN_ICON(Id, emb_dark_vol_internal_hfs); break;
      case BUILTIN_ICON_VOL_INTERNAL_APFS:
        DEC_BUILTIN_ICON(Id, emb_dark_vol_internal_apfs); break;
      case BUILTIN_ICON_VOL_INTERNAL_NTFS:
        DEC_BUILTIN_ICON(Id, emb_dark_vol_internal_ntfs); break;
      case BUILTIN_ICON_VOL_INTERNAL_EXT3:
        DEC_BUILTIN_ICON(Id, emb_dark_vol_internal_ext); break;
      case BUILTIN_ICON_VOL_INTERNAL_REC:
        DEC_BUILTIN_ICON(Id, emb_dark_vol_internal_recovery); break;
      case BUILTIN_ICON_BANNER:
        DEC_BUILTIN_ICON(Id, emb_dark_logo); break;
      case BUILTIN_SELECTION_SMALL:
        DEC_BUILTIN_ICON(Id, emb_dark_selection_small); break;
      case BUILTIN_SELECTION_BIG:
        DEC_BUILTIN_ICON(Id, emb_dark_selection_big); break;
    }

  } else {
    switch (Id) {
      case BUILTIN_ICON_POINTER:
        DEC_BUILTIN_ICON(Id, emb_pointer); break;
      case BUILTIN_ICON_FUNC_ABOUT:
        DEC_BUILTIN_ICON(Id, emb_func_about); break;
      case BUILTIN_ICON_FUNC_OPTIONS:
        DEC_BUILTIN_ICON(Id, emb_func_options); break;
      case BUILTIN_ICON_FUNC_CLOVER:
        DEC_BUILTIN_ICON(Id, emb_func_clover); break;
      case BUILTIN_ICON_FUNC_SECURE_BOOT:
        DEC_BUILTIN_ICON(Id, emb_func_secureboot); break;
      case BUILTIN_ICON_FUNC_SECURE_BOOT_CONFIG:
        DEC_BUILTIN_ICON(Id, emb_func_secureboot_config); break;
      case BUILTIN_ICON_FUNC_RESET:
        DEC_BUILTIN_ICON(Id, emb_func_reset); break;
      case BUILTIN_ICON_FUNC_EXIT:
        DEC_BUILTIN_ICON(Id, emb_func_exit); break;
      case BUILTIN_ICON_FUNC_HELP:
        DEC_BUILTIN_ICON(Id, emb_func_help); break;
      case BUILTIN_ICON_TOOL_SHELL:
        DEC_BUILTIN_ICON(Id, emb_func_shell); break;
      case BUILTIN_ICON_VOL_INTERNAL:
        DEC_BUILTIN_ICON(Id, emb_vol_internal); break;
      case BUILTIN_ICON_VOL_EXTERNAL:
        DEC_BUILTIN_ICON(Id, emb_vol_external); break;
      case BUILTIN_ICON_VOL_OPTICAL:
        DEC_BUILTIN_ICON(Id, emb_vol_optical); break;
      case BUILTIN_ICON_VOL_BOOTER:
        DEC_BUILTIN_ICON(Id, emb_vol_internal_booter); break;
      case BUILTIN_ICON_VOL_INTERNAL_HFS:
        DEC_BUILTIN_ICON(Id, emb_vol_internal_hfs); break;
      case BUILTIN_ICON_VOL_INTERNAL_APFS:
        DEC_BUILTIN_ICON(Id, emb_vol_internal_apfs); break;
      case BUILTIN_ICON_VOL_INTERNAL_NTFS:
        DEC_BUILTIN_ICON(Id, emb_vol_internal_ntfs); break;
      case BUILTIN_ICON_VOL_INTERNAL_EXT3:
        DEC_BUILTIN_ICON(Id, emb_vol_internal_ext); break;
      case BUILTIN_ICON_VOL_INTERNAL_REC:
        DEC_BUILTIN_ICON(Id, emb_vol_internal_recovery); break;
      case BUILTIN_ICON_BANNER:
        DEC_BUILTIN_ICON(Id, emb_logo); break;
      case BUILTIN_SELECTION_SMALL:
        DEC_BUILTIN_ICON(Id, emb_selection_small); break;
      case BUILTIN_SELECTION_BIG:
        DEC_BUILTIN_ICON(Id, emb_selection_big); break;
    }
  }
//  DBG("Icon %d decoded, pointer %x\n", Id, (UINTN)(BuiltinIconTable[Id].Image));

  if (!BuiltinIconTable[Id].Image) {
    TextBuffer = egCreateFilledImage(Size, Size, TRUE, &MenuBackgroundPixel);  //new pointer
//    egFillImage(TextBuffer, &MenuBackgroundPixel);
    p = StrStr(BuiltinIconTable[Id].Path, L"_"); p++;
    Text = (CHAR16*)AllocateCopyPool(30, (VOID*)p);
// the 2 next lines seems to be useless, because there is no '.' in BuiltinIconTable
// TODO jief : double check
//    p = StrStr(Text, L".");
//    *p = L'\0';
    if (StrCmp(Text, L"shutdown") == 0) {
      // icon name is shutdown from historic reasons, but function is now exit
      UnicodeSPrint(Text, 30, L"exit");
    }
    egRenderText(Text, TextBuffer, 0, 0, 0xFFFF, 1);
    BuiltinIconTable[Id].Image = TextBuffer;
    DebugLog(1, "        [!] Icon %d: Text <%s> rendered\n", Id, Text);
    FreePool(Text);
  }

  return BuiltinIconTable[Id].Image;
}

//
// Load an icon for an operating system
//

EG_IMAGE * LoadOSIcon(IN CONST CHAR16 *OSIconName OPTIONAL, IN CONST CHAR16 *FallbackIconName, IN UINTN PixelSize, IN BOOLEAN BootLogo, IN BOOLEAN WantDummy)
{
  EG_IMAGE        *Image;
  CHAR16          CutoutName[16], FirstName[16];
  CHAR16          FileName[256];
  UINTN           StartIndex, Index, NextIndex;

  if (GlobalConfig.TextOnly)      // skip loading if it's not used anyway
    return NULL;
  Image = NULL;
  *FirstName = 0;

  // try the names from OSIconName
  for (StartIndex = 0; OSIconName != NULL && OSIconName[StartIndex]; StartIndex = NextIndex) {
    // find the next name in the list
    NextIndex = 0;
    for (Index = StartIndex; OSIconName[Index]; Index++) {
      if (OSIconName[Index] == ',') {
        NextIndex = Index + 1;
        break;
      }
    }
    if (OSIconName[Index] == 0)
      NextIndex = Index;

    // construct full path
    if (Index > StartIndex + 15)   // prevent buffer overflow
      continue;
    CopyMem(CutoutName, OSIconName + StartIndex, (Index - StartIndex) * sizeof(CHAR16));
    CutoutName[Index - StartIndex] = 0;
    UnicodeSPrint(FileName, 512, L"icons\\%s_%s.icns",
                  BootLogo ? L"boot" : L"os", CutoutName);

    // try to load it
    Image = egLoadIcon(ThemeDir, FileName, PixelSize);
    if (Image != NULL) {
      return Image;
    }

    if (*FirstName == '\0') {
      CopyMem(FirstName, CutoutName, StrSize(CutoutName));
      if ('a' <= FirstName[0] && FirstName[0] <= 'z') {
        FirstName[0] = (CHAR16) (FirstName[0] - 0x20);
      }
    }
  }

  // try the fallback name
  UnicodeSPrint(FileName, 512, L"icons\\%s_%s.icns",
                BootLogo ? L"boot" : L"os", FallbackIconName);
  Image = egLoadIcon(ThemeDir, FileName, PixelSize);
  if (Image != NULL) {
    return Image;
  }

  // try the fallback name with os_ instead of boot_
  if (BootLogo) {
    Image = LoadOSIcon(NULL, FallbackIconName, PixelSize, FALSE, WantDummy);
    if (Image != NULL)
      return Image;
  }

  if (!WantDummy) {
    return NULL;
  }

  if (IsEmbeddedTheme()) { // embedded theme - return rendered icon name
//    EG_IMAGE  *TextBuffer = egCreateFilledImage(PixelSize, PixelSize, TRUE, &MenuBackgroundPixel);
//    egFillImage(TextBuffer, &MenuBackgroundPixel);
//    egRenderText(FirstName, TextBuffer, PixelSize/4, PixelSize/3, 0xFFFF, 1);
//    DebugLog(1, "Text <%s> rendered\n", FirstName);
    return NULL; //TextBuffer;
  }

  return DummyImage(PixelSize);
}

//
// Load an image from a .icns file
//

EG_IMAGE * LoadIcns(IN EFI_FILE_HANDLE BaseDir, IN CONST CHAR16 *FileName, IN UINTN PixelSize)
{
  if (GlobalConfig.TextOnly)      // skip loading if it's not used anyway
    return NULL;
  if (BaseDir) {
    return egLoadIcon(BaseDir, FileName, PixelSize);
  }
  return DummyImage(PixelSize);
}


EG_IMAGE * DummyImage(IN UINTN PixelSize)
{
    EG_IMAGE        *Image;
    UINTN           x, y, LineOffset;
    CHAR8           *Ptr, *YPtr;

    Image = egCreateFilledImage(PixelSize, PixelSize, TRUE, &BlackPixel);

    LineOffset = PixelSize * 4;

    YPtr = (CHAR8 *)Image->PixelData + ((PixelSize - 32) >> 1) * (LineOffset + 4);
    for (y = 0; y < 32; y++) {
        Ptr = YPtr;
        for (x = 0; x < 32; x++) {
            if (((x + y) % 12) < 6) {
                *Ptr++ = 0;
                *Ptr++ = 0;
                *Ptr++ = 0;
            } else {
                *Ptr++ = 0;
                *Ptr++ = ~0; //yellow
                *Ptr++ = ~0;
            }
            *Ptr++ = ~111;
        }
        YPtr += LineOffset;
    }

    return Image;
}

EG_IMAGE * LoadIcnsFallback(IN EFI_FILE_HANDLE BaseDir, IN CONST CHAR16 *FileName, IN UINTN PixelSize)
{
    EG_IMAGE *Image;

    if (GlobalConfig.TextOnly)      // skip loading if it's not used anyway
        return NULL;

    Image = egLoadIcon(BaseDir, FileName, PixelSize);
//    if (Image == NULL)
//        Image = DummyImage(PixelSize);
    return Image;
}
