/*
 * libeg/libegint.h
 * EFI graphics library internal header
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
 */

#ifndef __LIBEG_LIBEGINT_H__
#define __LIBEG_LIBEGINT_H__


//#include <efi.h>
//#include <efilib.h>

#include "../Platform/Platform.h"


#if defined(_MSC_VER)
# define ALIGN_64 __declspec(align(8))
#else
# define ALIGN_64 __attribute__((__aligned__(8)))
#endif
#define DEFINE_EMB_DATA(ico) UINT8 ALIGN_64 const ico[] =
#define DEFINE_EMB_SIZE(ico) UINTN const ico##_size = sizeof(ico);
//REVIEW: const would be more useful if the cast was not needed, but that
// would require correct use of const in the related functions.
#define ACCESS_EMB_DATA(ico) ((UINT8*)ico)
#define ACCESS_EMB_SIZE(ico) ico##_size
#define DECLARE_EMB_EXTERN_WITH_SIZE(ico) extern UINT8 const ico[]; extern UINTN const ico##_size;

DECLARE_EMB_EXTERN_WITH_SIZE(emb_font_data)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_pointer)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_func_about)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_func_clover)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_func_options)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_func_reset)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_func_secureboot_config)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_func_secureboot)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_func_shell)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_func_exit)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_func_help)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_logo)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_selection_big)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_selection_small)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_selection_indicator)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_vol_internal)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_vol_internal_booter)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_vol_internal_hfs)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_vol_internal_apfs)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_vol_internal_ntfs)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_vol_internal_ext)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_vol_internal_recovery)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_vol_external)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_vol_optical)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_scroll_up_button)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_scroll_bar_start)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_scroll_scroll_start)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_scroll_scroll_fill)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_scroll_scroll_end)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_scroll_bar_fill)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_scroll_bar_end)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_scroll_down_button)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_radio_button_selected)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_radio_button)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_checkbox)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_checkbox_checked)

DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_font_data)
//DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_pointer)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_func_about)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_func_clover)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_func_options)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_func_reset)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_func_secureboot_config)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_func_secureboot)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_func_shell)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_func_exit)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_func_help)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_logo)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_selection_big)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_selection_small)
//DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_selection_indicator)

// same volumes for both light and dark mode
/*
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_vol_internal)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_vol_internal_booter)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_vol_internal_hfs)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_vol_internal_apfs)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_vol_internal_ntfs)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_vol_internal_ext)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_vol_internal_recovery)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_vol_external)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_vol_optical)
*/
#define emb_dark_vol_internal          emb_vol_internal
#define emb_dark_vol_internal_booter   emb_vol_internal_booter
#define emb_dark_vol_internal_hfs      emb_vol_internal_hfs
#define emb_dark_vol_internal_apfs     emb_vol_internal_apfs
#define emb_dark_vol_internal_ntfs     emb_vol_internal_ntfs
#define emb_dark_vol_internal_ext      emb_vol_internal_ext
#define emb_dark_vol_internal_recovery emb_vol_internal_recovery
#define emb_dark_vol_external          emb_vol_external
#define emb_dark_vol_optical           emb_vol_optical

/*
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_scroll_up_button)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_scroll_bar_start)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_scroll_scroll_start)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_scroll_scroll_fill)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_scroll_scroll_end)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_scroll_bar_fill)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_scroll_bar_end)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_scroll_down_button)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_radio_button_selected)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_radio_button)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_checkbox)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_checkbox_checked)
*/

/* types */

//typedef EG_IMAGE * (*EG_DECODE_FUNC)(IN UINT8 *FileData, IN UINTN FileDataLength, IN UINTN IconSize, IN BOOLEAN WantAlpha);

/* functions */

VOID egRestrictImageArea(IN EG_IMAGE *Image,
                         IN INTN AreaPosX, IN INTN AreaPosY,
                         IN OUT INTN *AreaWidth, IN OUT INTN *AreaHeight);
VOID egRawCopy(IN OUT EG_PIXEL *CompBasePtr, IN EG_PIXEL *TopBasePtr,
               IN INTN Width, IN INTN Height,
               IN INTN CompLineOffset, IN INTN TopLineOffset);
VOID egRawCompose(IN OUT EG_PIXEL *CompBasePtr, IN EG_PIXEL *TopBasePtr,
                  IN INTN Width, IN INTN Height,
                  IN INTN CompLineOffset, IN INTN TopLineOffset);
VOID egRawComposeOnFlat(IN OUT EG_PIXEL *CompBasePtr, IN EG_PIXEL *TopBasePtr,
                  IN INTN Width, IN INTN Height,
                  IN INTN CompLineOffset, IN INTN TopLineOffset);

//#define PLPTR(imagevar, colorname) ((UINT8 *) &((imagevar)->PixelData->colorname))

//VOID egDecompressIcnsRLE(IN OUT UINT8 **CompData, IN OUT UINTN *CompLen, IN UINT8 *DestPlanePtr, IN UINTN PixelCount);
//VOID egInsertPlane(IN UINT8 *SrcDataPtr, IN UINT8 *DestPlanePtr, IN UINTN PixelCount);
//VOID egSetPlane(IN UINT8 *DestPlanePtr, IN UINT8 Value, IN UINT64 PixelCount);
//VOID egCopyPlane(IN UINT8 *SrcPlanePtr, IN UINT8 *DestPlanePtr, IN UINTN PixelCount);

//EG_IMAGE * egDecodeBMP(IN UINT8 *FileData, IN UINTN FileDataLength, IN UINTN IconSize, IN BOOLEAN WantAlpha);
EG_IMAGE * egDecodeICNS(IN UINT8 *FileData, IN UINTN FileDataLength, IN UINTN IconSize, IN BOOLEAN WantAlpha);

EG_IMAGE * egDecodePNG(IN UINT8 *FileData, IN UINTN FileDataLength, IN BOOLEAN WantAlpha);


//VOID egEncodeBMP(IN EG_IMAGE *Image, OUT UINT8 **FileData, OUT UINTN *FileDataLength);



#endif /* __LIBEG_LIBEGINT_H__ */

/* EOF */
