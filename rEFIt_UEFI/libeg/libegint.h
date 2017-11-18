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

#include "Platform.h"

/* embedded arrays
extern UINT8 emb_font_data[];
extern UINT8 emb_pointer[];
extern UINT8 emb_func_about[];
extern UINT8 emb_func_clover[];
extern UINT8 emb_func_options[];
extern UINT8 emb_func_reset[];
extern UINT8 emb_func_secureboot_config[];
extern UINT8 emb_func_secureboot[];
extern UINT8 emb_func_shell[];
extern UINT8 emb_func_exit[];
extern UINT8 emb_func_help[];
extern UINT8 emb_logo[];
extern UINT8 emb_selection_big[];
extern UINT8 emb_selection_small[];
extern UINT8 emb_vol_internal[];
extern UINT8 emb_vol_internal_booter[];
extern UINT8 emb_vol_internal_hfs[];
extern UINT8 emb_vol_internal_ntfs[];
extern UINT8 emb_vol_internal_ext[];
extern UINT8 emb_vol_internal_recovery[];
extern UINT8 emb_scroll_up_button[];
extern UINT8 emb_scroll_bar_start[];
extern UINT8 emb_scroll_scroll_start[];
extern UINT8 emb_scroll_scroll_fill[];
extern UINT8 emb_scroll_scroll_end[];
extern UINT8 emb_scroll_bar_fill[];
extern UINT8 emb_scroll_bar_end[];
extern UINT8 emb_scroll_down_button[];
extern UINT8 emb_radio_button_selected[];
extern UINT8 emb_radio_button[];
extern UINT8 emb_selection_indicator[];
extern UINT8 emb_checkbox[];
extern UINT8 emb_checkbox_checked[];


#define SZ_emb_font_data 1549
#define SZ_emb_pointer 509
#define SZ_emb_func_about 199
#define SZ_emb_func_clover 209
#define SZ_emb_func_options 222
#define SZ_emb_func_reset 217
#define SZ_emb_func_secureboot_config 194
#define SZ_emb_func_secureboot 200
#define SZ_emb_func_shell 196
#define SZ_emb_func_exit 210
#define SZ_emb_func_help 199
#define SZ_emb_logo 3025
#define SZ_emb_selection_big 712
#define SZ_emb_selection_small 180
#define SZ_emb_vol_internal 918
#define SZ_emb_vol_internal_booter 1081
//#define SZ_emb_vol_internal_hfs 923
#define SZ_emb_vol_internal_hfs 857
#define SZ_emb_vol_internal_ntfs 935
#define SZ_emb_vol_internal_ext 947
#define SZ_emb_vol_internal_recovery 845
#define SZ_emb_scroll_up_button 162
#define SZ_emb_scroll_bar_start 151
#define SZ_emb_scroll_scroll_start 142
#define SZ_emb_scroll_scroll_fill 112
#define SZ_emb_scroll_scroll_end 142
#define SZ_emb_scroll_bar_fill 134
#define SZ_emb_scroll_bar_end 150
#define SZ_emb_scroll_down_button 174
#define SZ_emb_radio_button_selected 418
#define SZ_emb_radio_button 324
#define SZ_emb_selection_indicator 1637
#define SZ_emb_checkbox 173
#define SZ_emb_checkbox_checked 367
*/

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
DECLARE_EMB_EXTERN_WITH_SIZE(emb_vol_internal)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_vol_internal_booter)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_vol_internal_hfs)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_vol_internal_ntfs)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_vol_internal_ext)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_vol_internal_recovery)
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
DECLARE_EMB_EXTERN_WITH_SIZE(emb_selection_indicator)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_checkbox)
DECLARE_EMB_EXTERN_WITH_SIZE(emb_checkbox_checked)


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

#define PLPTR(imagevar, colorname) ((UINT8 *) &((imagevar)->PixelData->colorname))

VOID egDecompressIcnsRLE(IN OUT UINT8 **CompData, IN OUT UINTN *CompLen, IN UINT8 *DestPlanePtr, IN UINTN PixelCount);
VOID egInsertPlane(IN UINT8 *SrcDataPtr, IN UINT8 *DestPlanePtr, IN UINTN PixelCount);
VOID egSetPlane(IN UINT8 *DestPlanePtr, IN UINT8 Value, IN UINT64 PixelCount);
VOID egCopyPlane(IN UINT8 *SrcPlanePtr, IN UINT8 *DestPlanePtr, IN UINTN PixelCount);

EG_IMAGE * egDecodeBMP(IN UINT8 *FileData, IN UINTN FileDataLength, IN UINTN IconSize, IN BOOLEAN WantAlpha);
EG_IMAGE * egDecodeICNS(IN UINT8 *FileData, IN UINTN FileDataLength, IN UINTN IconSize, IN BOOLEAN WantAlpha);
#if defined(LODEPNG)
EG_IMAGE * egDecodePNG(IN UINT8 *FileData, IN UINTN FileDataLength, IN BOOLEAN WantAlpha);
#endif

//VOID egEncodeBMP(IN EG_IMAGE *Image, OUT UINT8 **FileData, OUT UINTN *FileDataLength);


#endif /* __LIBEG_LIBEGINT_H__ */

/* EOF */
