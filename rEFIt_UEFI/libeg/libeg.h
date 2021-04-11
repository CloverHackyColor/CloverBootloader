/*
 * libeg/libeg.h
 * EFI graphics library header for users
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

#ifndef __LIBEG_LIBEG_H__
#define __LIBEG_LIBEG_H__

//#include <Efi.h>

#include "../cpp_foundation/XString.h"
#include "../cpp_foundation/XStringArray.h"
#include "../cpp_foundation/XBuffer.h"

extern "C" {
#include <Library/OcConfigurationLib.h>
#include <Guid/AppleApfsInfo.h>
}

/* types */

typedef enum {
  imNone,
  imScale,
  imCrop,
  imTile
} SCALING;

typedef enum {
  FONT_ALFA,
  FONT_GRAY,
  FONT_LOAD
} FONT_TYPE;

typedef enum {
  NoEvents,
  Move,
  LeftClick,
  RightClick,
  DoubleClick,
  ScrollClick,
  ScrollDown,
  ScrollUp,
  LeftMouseDown,
  RightMouseDown,
  MouseMove
} MOUSE_EVENT;

typedef enum {
  ActionNone = 0,
  ActionHelp,
  ActionSelect,
  ActionEnter,
  ActionDeselect,
  ActionDestroy,
  ActionOptions,
  ActionDetails,
  ActionFinish,
  ActionScrollDown,
  ActionScrollUp,
  ActionMoveScrollbar,
  ActionPageDown,
  ActionPageUp,
  ActionLight
} ACTION;

typedef struct {
  INTN    CurrentSelection, LastSelection;
  INTN    MaxScroll, MaxIndex;
  INTN    FirstVisible, LastVisible, MaxVisible, MaxFirstVisible;
  BOOLEAN IsScrolling, PaintAll, PaintSelection;
} SCROLL_STATE;

typedef enum {
  BoolValue,
  Decimal,
  Hex,
  ASString,
  UNIString,
  RadioSwitch,
  CheckBit,
} ITEM_TYPE;

class INPUT_ITEM {
public:
  ITEM_TYPE     ItemType = BoolValue; //string, value, boolean
  BOOLEAN       Valid = 0;
  UINT8         BValue = 0; // was BOOLEAN, but value 2 is sometimes assigned.
  //UINT8         Pad8;
  UINT32        IValue = 0;
  XStringW      SValue = XStringW();
  UINTN         LineShift = 0;
  
  bool operator == (const INPUT_ITEM& other) const
  {
    if ( !(ItemType == other.ItemType ) ) return false;
    if ( !(Valid == other.Valid ) ) return false;
    if ( !(BValue == other.BValue ) ) return false;
    if ( !(IValue == other.IValue ) ) return false;
    if ( !(SValue == other.SValue ) ) return false;
    if ( !(LineShift == other.LineShift ) ) return false;
    return true;
  }
  bool operator != (const INPUT_ITEM& other) const { return !(*this == other); }
};

typedef struct {
  EFI_STATUS          LastStatus;
  const EFI_FILE            *DirHandle;
  BOOLEAN             CloseDirHandle;
  EFI_FILE_INFO       *LastFileInfo;
} REFIT_DIR_ITER;

typedef struct {
  UINT8 Flags;
  UINT8 StartCHS[3];
  UINT8 Type;
  UINT8 EndCHS[3];
  UINT32 StartLBA;
  UINT32 Size;
} MBR_PARTITION_INFO;




typedef enum {
  AlignNo,
  AlignLeft,
  AlignRight,
  AlignCenter,
  AlignUp,
  AlignDown

} ALIGNMENT;



/* This should be compatible with EFI_UGA_PIXEL */
typedef struct {
    UINT8 b, g, r, a;
} EG_PIXEL;
/*
typedef struct {
  UINT8 Blue;
  UINT8 Green;
  UINT8 Red;
  UINT8 Reserved;
} EFI_GRAPHICS_OUTPUT_BLT_PIXEL;

typedef union {
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL Pixel;
  UINT32                        Raw;
} EFI_GRAPHICS_OUTPUT_BLT_PIXEL_UNION;
*/


#ifdef __cplusplus
class EG_RECT {
public:
  INTN     XPos;
  INTN     YPos;
  INTN     Width;
  INTN     Height;
	
  EG_RECT() : XPos(0), YPos(0), Width(0), Height(0) {};
  EG_RECT(INTN x, INTN y, INTN w, INTN h) : XPos(x), YPos(y), Width(w), Height(h) { }
  EG_RECT(const EG_RECT& other) : XPos(other.XPos), YPos(other.YPos), Width(other.Width), Height(other.Height) { }
  const EG_RECT& operator = (const EG_RECT& other) { XPos = other.XPos; YPos = other.YPos; Width = other.Width; Height = other.Height; return *this; }
  bool operator == (const EG_RECT& other) { return XPos == other.XPos  &&  YPos == other.YPos  &&  Width == other.Width  &&  Height == other.Height; }
  bool operator != (const EG_RECT& other) { return !(*this == other); }
};
#else
typedef struct EG_RECT {
  INTN     XPos;
  INTN     YPos;
  INTN     Width;
  INTN     Height;
} EG_RECT;
#endif

#define TEXT_YMARGIN (2)
#define TEXT_XMARGIN (8)

#define EG_EIPIXELMODE_GRAY         (0)
#define EG_EIPIXELMODE_GRAY_ALPHA   (1)
#define EG_EIPIXELMODE_COLOR        (2)
#define EG_EIPIXELMODE_COLOR_ALPHA  (3)
#define EG_EIPIXELMODE_ALPHA        (4)
#define EG_MAX_EIPIXELMODE          EG_EIPIXELMODE_ALPHA

#define EG_EICOMPMODE_NONE          (0)
#define EG_EICOMPMODE_RLE           (1)
#define EG_EICOMPMODE_EFICOMPRESS   (2)

/* functions */


void    egInitScreen(IN BOOLEAN SetMaxResolution);
void    egDumpGOPVideoModes(void);
//EFI_STATUS egSetScreenResolution(IN CHAR16 *WidthHeight); 
//EFI_STATUS egSetMaxResolution(void);
EFI_STATUS egSetMode(INT32 Next);

void    egGetScreenSize(OUT INTN *ScreenWidth, OUT INTN *ScreenHeight);
XString8 egScreenDescription(void);
BOOLEAN egHasGraphicsMode(void);
BOOLEAN egIsGraphicsModeEnabled(void);
void    egSetGraphicsModeEnabled(IN BOOLEAN Enable);
// NOTE: Even when egHasGraphicsMode() returns FALSE, you should
//  call egSetGraphicsModeEnabled(FALSE) to ensure the system
//  is running in text mode. egHasGraphicsMode() only determines
//  if libeg can draw to the screen in graphics mode.

EFI_STATUS egLoadFile(const EFI_FILE* BaseDir, IN CONST CHAR16 *FileName,
                      OUT UINT8 **FileData, OUT UINTN *FileDataLength);
EFI_STATUS egSaveFile(const EFI_FILE* BaseDir OPTIONAL, IN CONST CHAR16 *FileName,
                      IN CONST void *FileData, IN UINTN FileDataLength);
EFI_STATUS egMkDir(const EFI_FILE* BaseDir OPTIONAL, IN CONST CHAR16 *DirName);
EFI_STATUS egFindESP(OUT EFI_FILE** RootDir);

void egClearScreen(IN const void *Color);

EFI_STATUS egScreenShot(void);


#endif /* __LIBEG_LIBEG_H__ */

/* EOF */
