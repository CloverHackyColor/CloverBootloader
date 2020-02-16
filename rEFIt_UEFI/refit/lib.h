/*
 * refit/lib.h
 * General header file
 *
 * Copyright (c) 2006-2009 Christoph Pfisterer
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

#ifndef __REFITLIB_STANDARD_H__
#define __REFITLIB_STANDARD_H__

// Experimental -->

/*
  - FKERNELPATCH: Sat Jul 30 19:13:21 2016

    Since we're in bruteforce mode, no need to check the existence of given patterns before patching (except for debugging purposes). Just patch or leave it.
    This will skip "SearchAndCount" to boost those operations. We hope this will be safe enough. The "SearchAndReplace" always do a CompareMem before CopyMem.
    And dataLen (for search & replace) already sanitised while parsing user config & should be matched.

  - ADVLOG: Thu Aug  4 18:14:19 2016

    Add log routine line separator.

  - LODEPNG: Thu Aug  4 18:14:19 2016

    Size matter, screenshot as PNG instead of BMP. Inspired by mr. Coderush "CrScreenshotDxe". Maybe useful for other PNG encoding purposes. Activate by "-D LODEPNG".

  - ANDX86: Mon Aug  8 04:07:13 2016

    Scan grubx64 (Remix, Phoenix, & Chrome OS). Tested with 64bit only & live USB. Build with "-D ANDX86".
    http://www.jide.com/remixos
    http://www.phoenixos.com
    https://www.chromium.org/chromium-os
*/

//#define FKERNELPATCH 1
#define ADVLOG 1

// Experimental <--

#include "libeg.h"

#define REFIT_DEBUG (2)
#define Print if ((!GlobalConfig.Quiet) || (GlobalConfig.TextOnly)) Print
//#include "GenericBdsLib.h"

extern EFI_HANDLE             gImageHandle;
extern EFI_SYSTEM_TABLE*			gST;
extern EFI_BOOT_SERVICES*			gBS;
extern EFI_RUNTIME_SERVICES*	gRT;

#define TAG_ABOUT              (1)
#define TAG_RESET              (2)
#define TAG_SHUTDOWN           (3)
#define TAG_TOOL               (4)
#define TAG_LOADER             (5)
#define TAG_LEGACY             (6)
#define TAG_INFO               (7)
#define TAG_OPTIONS            (8)
#define TAG_INPUT              (9)
#define TAG_HELP               (10)
#define TAG_SWITCH             (11)
#define TAG_CHECKBIT           (12)
#define TAG_SECURE_BOOT        (13)
#define TAG_SECURE_BOOT_CONFIG (14)
#define TAG_CLOVER             (100)
#define TAG_EXIT               (101)
#define TAG_RETURN             ((UINTN)(-1))

//
// lib module
//

typedef struct {
  EFI_STATUS          LastStatus;
  EFI_FILE            *DirHandle;
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

#define DISK_KIND_INTERNAL      (0)
#define DISK_KIND_EXTERNAL      (1)
#define DISK_KIND_OPTICAL       (2)
#define DISK_KIND_FIREWIRE      (3)
#define DISK_KIND_NODISK        (4)
#define DISK_KIND_BOOTER        (5)

#define BOOTING_BY_BOOTLOADER   (1)
#define BOOTING_BY_EFI          (2)
#define BOOTING_BY_BOOTEFI      (3)
#define BOOTING_BY_MBR          (4)
#define BOOTING_BY_PBR          (5)
#define BOOTING_BY_CD           (6)

#define OSTYPE_OSX              (1)
#define OSTYPE_WIN              (2)
#define OSTYPE_VAR              (3)
#define OSTYPE_LIN              (4)
#define OSTYPE_LINEFI           (5)
#define OSTYPE_EFI              (6)
#define OSTYPE_WINEFI           (7)
//#define OSTYPE_BOOT_OSX         (9)
#define OSTYPE_RECOVERY         (10)
#define OSTYPE_OSX_INSTALLER    (11)
/*#define OSTYPE_TIGER            (14)
 #define OSTYPE_LEO              (15)
 #define OSTYPE_SNOW             (16)
 #define OSTYPE_LION             (17)
 #define OSTYPE_ML               (18)
 #define OSTYPE_MAV              (19)*/
#define OSTYPE_OTHER            (99)
//#define OSTYPE_HIDE             (100)

#define OSTYPE_IS_OSX(type) ((type == OSTYPE_OSX) /*|| (type == OSTYPE_BOOT_OSX) || ((type >= OSTYPE_TIGER) && (type <= OSTYPE_MAV))*/ || (type == OSTYPE_VAR))
#define OSTYPE_IS_OSX_RECOVERY(type) ((type == OSTYPE_RECOVERY) /*|| ((type >= OSTYPE_TIGER) && (type <= OSTYPE_MAV))*/ || (type == OSTYPE_VAR))
#define OSTYPE_IS_OSX_INSTALLER(type) ((type == OSTYPE_OSX_INSTALLER) /*|| ((type >= OSTYPE_TIGER) && (type <= OSTYPE_MAV))*/ || (type == OSTYPE_VAR))
#define OSTYPE_IS_WINDOWS(type) ((type == OSTYPE_WIN) || (type == OSTYPE_WINEFI) || (type == OSTYPE_EFI) || (type == OSTYPE_VAR))
#define OSTYPE_IS_LINUX(type) ((type == OSTYPE_LIN) || (type == OSTYPE_EFI) || (type == OSTYPE_VAR))
#define OSTYPE_IS_OTHER(type) ((type == OSTYPE_OTHER) || (type == OSTYPE_EFI) || (type == OSTYPE_VAR))
#define OSTYPE_COMPARE_IMP(comparator, type1, type2) (comparator(type1) && comparator(type2))
#define OSTYPE_COMPARE(type1, type2) (OSTYPE_COMPARE_IMP(OSTYPE_IS_OSX, type1, type2) || OSTYPE_COMPARE_IMP(OSTYPE_IS_OSX_RECOVERY, type1, type2) || \
OSTYPE_COMPARE_IMP(OSTYPE_IS_OSX_INSTALLER, type1, type2) || OSTYPE_COMPARE_IMP(OSTYPE_IS_WINDOWS, type1, type2) || \
OSTYPE_COMPARE_IMP(OSTYPE_IS_LINUX, type1, type2) || OSTYPE_COMPARE_IMP(OSTYPE_IS_OTHER, type1, type2))

#define OSFLAG_ISSET(flags, flag) ((flags & flag) == flag)
#define OSFLAG_ISUNSET(flags, flag) ((flags & flag) != flag)
#define OSFLAG_SET(flags, flag) (flags | flag)
#define OSFLAG_UNSET(flags, flag) (flags & (~flag))
#define OSFLAG_TOGGLE(flags, flag) (flags ^ flag)
#define OSFLAG_USEGRAPHICS    (1 << 0)
#define OSFLAG_WITHKEXTS      (1 << 1)
#define OSFLAG_CHECKFAKESMC   (1 << 2)
#define OSFLAG_NOCACHES       (1 << 3)
#define OSFLAG_NODEFAULTARGS  (1 << 4)
#define OSFLAG_NODEFAULTMENU  (1 << 5)
#define OSFLAG_HIDDEN         (1 << 6)
#define OSFLAG_DISABLED       (1 << 7)
#define OSFLAG_HIBERNATED     (1 << 8)
#define OSFLAG_NOSIP          (1 << 9)

#define CUSTOM_BOOT_DISABLED       0
#define CUSTOM_BOOT_USER_DISABLED  1
#define CUSTOM_BOOT_NONE           2
#define CUSTOM_BOOT_APPLE          3
#define CUSTOM_BOOT_ALT_APPLE      4
#define CUSTOM_BOOT_THEME          5
#define CUSTOM_BOOT_USER           6

#define OPT_I386            (1 << 0)
#define OPT_X64             (1 << 1)
#define OPT_VERBOSE         (1 << 2)
#define OPT_NOCACHES        (1 << 3)
#define OPT_SINGLE_USER     (1 << 4)
#define OPT_SAFE            (1 << 5)
#define OPT_NVDISABLE       (1 << 6)
#define OPT_SLIDE           (1 << 7)
#define OPT_POWERNAPOFF     (1 << 8)
#define OPT_XCPM            (1 << 9)
#define OPT_GNOIDLE         (1 << 10)
#define OPT_GNOSLEEP        (1 << 11)
#define OPT_GNOMSI          (1 << 12)
#define OPT_EHCUSB          (1 << 13)
#define OPT_KEEPSYMS        (1 << 14)
#define OPT_DEBUG           (1 << 15)
#define OPT_KEXTLOG         (1 << 16)
#define OPT_APPLEALC        (1 << 17)
#define OPT_SHIKI           (1 << 18)
#define INX_NVWEBON         19
#define OPT_NVWEBON         (1 << INX_NVWEBON)
#define NUM_OPT             20
//extern CHAR16* ArgOptional[];

#define AsciiPageSize 0xC0

#define IS_EXTENDED_PART_TYPE(type) ((type) == 0x05 || (type) == 0x0f || (type) == 0x85)

typedef struct {
  UINT8               Type;
  CHAR16              *IconName;
  CHAR16              *Name;
} LEGACY_OS;

typedef struct {
  EFI_DEVICE_PATH     *DevicePath;
  EFI_HANDLE          DeviceHandle;
  EFI_FILE            *RootDir;
  CHAR16              *DevicePathString;
  CHAR16              *VolName;
  CHAR16              *VolLabel;
  UINT8               DiskKind;
  LEGACY_OS           *LegacyOS;
  BOOLEAN             Hidden;
  UINT8               BootType;
  BOOLEAN             IsAppleLegacy;
  BOOLEAN             HasBootCode;
  BOOLEAN             IsMbrPartition;
  UINTN               MbrPartitionIndex;
  EFI_BLOCK_IO        *BlockIO;
  UINT64              BlockIOOffset;
  EFI_BLOCK_IO        *WholeDiskBlockIO;
  EFI_DEVICE_PATH     *WholeDiskDevicePath;
  EFI_HANDLE          WholeDiskDeviceHandle;
  MBR_PARTITION_INFO  *MbrPartitionTable;
  UINT32              DriveCRC32;
  EFI_GUID            RootUUID; //for recovery it is UUID of parent partition
  UINT64              SleepImageOffset;
} REFIT_VOLUME;

typedef enum {
  AlignNo,
  AlignLeft,
  AlignRight,
  AlignCenter,
  AlignUp,
  AlignDown
  
} ALIGNMENT;

//mouse types
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

typedef struct _pointers {
  EFI_SIMPLE_POINTER_PROTOCOL *SimplePointerProtocol;
  EG_IMAGE *Pointer;
  EG_IMAGE *newImage;
  EG_IMAGE *oldImage;
  
  EG_RECT  newPlace;
  EG_RECT  oldPlace;
  
  UINT64	LastClickTime;  //not EFI_TIME
  EFI_SIMPLE_POINTER_STATE    State;
  MOUSE_EVENT MouseEvent;
} POINTERS;

//GUI types
typedef enum {
  BoolValue,
  Decimal,
  Hex,
  ASString,
  UNIString,
  RadioSwitch,
  CheckBit,
  
} ITEM_TYPE;

typedef struct {
  ITEM_TYPE ItemType; //string, value, boolean
  BOOLEAN Valid;
  BOOLEAN BValue;
  UINT8   Pad8;
  UINT32  IValue;
  //  UINT64  UValue;
  //  CHAR8*  AValue;
  CHAR16* SValue; // Max Size (see below) so the field can be edit by the GUI
  UINTN   LineShift;
} INPUT_ITEM;

// Allow for 255 unicode characters + 2 byte unicode null terminator.
#define SVALUE_MAX_SIZE 512

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

extern BOOLEAN ScrollEnabled;
extern EG_RECT UpButton;
extern EG_RECT DownButton;
extern EG_RECT ScrollbarBackground;
extern EG_RECT Scrollbar;
extern BOOLEAN IsDragging;
extern EG_RECT ScrollbarOldPointerPlace;
extern EG_RECT ScrollbarNewPointerPlace;
extern INTN ScrollbarYMovement;

#define SCREEN_UNKNOWN    0
#define SCREEN_MAIN       1
#define SCREEN_ABOUT      2
#define SCREEN_HELP       3
#define SCREEN_OPTIONS    4
#define SCREEN_GRAPHICS   5
#define SCREEN_CPU        6
#define SCREEN_BINARIES   7
#define SCREEN_DSDT       8
#define SCREEN_BOOT       9
#define SCREEN_SMBIOS     10
#define SCREEN_TABLES     11
#define SCREEN_RC_SCRIPTS 12
#define SCREEN_USB        13
#define SCREEN_THEME      14
#define SCREEN_SYSVARS    15
#define SCREEN_CSR        16
#define SCREEN_BLC        17
#define SCREEN_DSM        18
#define SCREEN_ACPI       19
#define SCREEN_GUI        20
#define SCREEN_SYSTEM     21
#define SCREEN_AUDIO      22
#define SCREEN_KEXTS      23
#define SCREEN_KERNELS    24
#define SCREEN_DSDT_PATCHES 25
#define SCREEN_DEVICES    26
#define SCREEN_BOOTER     27
#define SCREEN_KEXT_INJECT   28
#define SCREEN_KEXTS_MAN     29
#define SCREEN_AUDIOPORTS    30

#define MAX_ANIME  41

typedef struct _refit_menu_screen REFIT_MENU_SCREEN;

typedef struct _refit_menu_entry {
  CHAR16            *Title;
  UINTN              Tag;
  UINTN              Row;
  CHAR16             ShortcutDigit;
  CHAR16             ShortcutLetter;
  EG_IMAGE          *Image;
  EG_IMAGE          *DriveImage;
  EG_IMAGE          *BadgeImage;
  EG_RECT            Place;
  ACTION             AtClick;
  ACTION             AtDoubleClick;
  ACTION             AtRightClick;
  ACTION             AtMouseOver;
  REFIT_MENU_SCREEN *SubScreen;
} REFIT_MENU_ENTRY;

typedef struct _refit_input_dialog {
  REFIT_MENU_ENTRY  Entry;
  INPUT_ITEM        *Item;
} REFIT_INPUT_DIALOG;

//some unreal values
#define FILM_CENTRE   40000
//#define FILM_LEFT     50000
//#define FILM_TOP      50000
//#define FILM_RIGHT    60000
//#define FILM_BOTTOM   60000
//#define FILM_PERCENT 100000
#define INITVALUE       40000

struct _refit_menu_screen {
  UINTN             ID;
  CHAR16            *Title;
  EG_IMAGE          *TitleImage;
  INTN              InfoLineCount;
  CHAR16            **InfoLines;
  INTN              EntryCount;
  REFIT_MENU_ENTRY  **Entries;
  INTN              TimeoutSeconds;
  CHAR16            *TimeoutText;
  CHAR16            *Theme;
  BOOLEAN           AnimeRun;
  BOOLEAN           Once;
  UINT64            LastDraw;
  INTN              CurrentFrame;
  INTN              Frames;
  UINTN             FrameTime; //ms
  EG_RECT           FilmPlace;
  EG_IMAGE          **Film;
};

#define VOLTYPE_OPTICAL    (0x0001)
#define VOLTYPE_EXTERNAL   (0x0002)
#define VOLTYPE_INTERNAL   (0x0004)
#define VOLTYPE_FIREWIRE   (0x0008)

#define HIDEUI_FLAG_SHELL             (0x0010)
#define HIDEUI_FLAG_TOOLS             (0x0020)
#define HIDEUI_FLAG_SINGLEUSER        (0x0040)
#define HIDEUI_FLAG_HWTEST            (0x0080)
#define HIDEUI_FLAG_BANNER            (0x0100)
#define HIDEUI_FLAG_FUNCS             (0x0200)
#define HIDEUI_FLAG_LABEL             (0x0400)
#define HIDEUI_FLAG_REVISION          (0x0800)
#define HIDEUI_FLAG_MENU_TITLE        (0x1000)
#define HIDEUI_FLAG_MENU_TITLE_IMAGE  (0x2000)
#define HIDEUI_FLAG_HELP              (0x4000)
#define HIDEUI_FLAG_ROW1              (0x8000)
#define HIDEUI_ALL                    (0xffff & (~VOLTYPE_INTERNAL))

#define HDBADGES_SWAP   (1<<0)
#define HDBADGES_SHOW   (1<<1)
#define HDBADGES_INLINE (1<<2)

typedef enum {
  imNone,
  imScale,
  imCrop,
  imTile
  
} SCALING;

typedef struct {
  INTN        Timeout;
  UINTN       DisableFlags;
  UINTN       HideBadges;
  UINTN       HideUIFlags;
  BOOLEAN     TextOnly;
  BOOLEAN     Quiet;
  BOOLEAN     LegacyFirst;
  BOOLEAN     NoLegacy;
  BOOLEAN     DebugLog;
  BOOLEAN     FastBoot;
  BOOLEAN     NeverHibernate;
  BOOLEAN     StrictHibernate;
  BOOLEAN     RtcHibernateAware;
  FONT_TYPE   Font;
  INTN        CharWidth;
  UINTN       SelectionColor;
  CHAR16      *FontFileName;
  CHAR16      *Theme;
  CHAR16      *BannerFileName;
  CHAR16      *SelectionSmallFileName;
  CHAR16      *SelectionBigFileName;
  CHAR16      *SelectionIndicatorName;
  CHAR16      *DefaultSelection;
  CHAR16      *ScreenResolution;
  INTN        ConsoleMode;
  CHAR16      *BackgroundName;
  SCALING     BackgroundScale;
  UINTN       BackgroundSharp;
  BOOLEAN     BackgroundDark;
  BOOLEAN     CustomIcons;
  BOOLEAN     SelectionOnTop;
  BOOLEAN     BootCampStyle;
  INTN        BadgeOffsetX;
  INTN        BadgeOffsetY;
  INTN        BadgeScale;
  INTN        ThemeDesignWidth;
  INTN        ThemeDesignHeight;
  INTN        BannerPosX;
  INTN        BannerPosY;
  INTN        BannerEdgeHorizontal;
  INTN        BannerEdgeVertical;
  INTN        BannerNudgeX;
  INTN        BannerNudgeY;
  BOOLEAN     VerticalLayout;
  BOOLEAN     NonSelectedGrey;
  INTN        MainEntriesSize;
  INTN        TileXSpace;
  INTN        TileYSpace;
  INTN        IconFormat;
  BOOLEAN     Proportional;
  BOOLEAN     NoEarlyProgress;
  BOOLEAN     ShowOptimus;
  BOOLEAN     HibernationFixup;
  BOOLEAN     SignatureFixup;
  BOOLEAN     DarkEmbedded;
  BOOLEAN     TypeSVG;
  INT32       Timezone;
  INTN        Codepage;
  INTN        CodepageSize;
  float       Scale;
  float       CentreShift;
} REFIT_CONFIG;

// types
typedef struct KEXT_PATCH KEXT_PATCH;
struct KEXT_PATCH
{
  CHAR8       *Name;
  CHAR8       *Label;
  BOOLEAN     IsPlistPatch;
  CHAR8       align[7];
  INTN        DataLen;
#if defined(MDE_CPU_IA32)
  UINT32      align1;
#endif
  UINT8       *Data;
  UINT8       *Patch;
  UINT8       *MaskFind;
  UINT8       *MaskReplace;
  CHAR8       *MatchOS;
  CHAR8       *MatchBuild;
  INPUT_ITEM  MenuItem;
};

typedef struct {
  CHAR8       *Label;
  INTN        DataLen;
  UINT8       *Data;
  UINT8       *Patch;
  UINT8       *MaskFind;
  UINT8       *MaskReplace;
  INTN        Count;
  CHAR8       *MatchOS;
  CHAR8       *MatchBuild;
  INPUT_ITEM  MenuItem;
} KERNEL_PATCH;

typedef struct KERNEL_AND_KEXT_PATCHES
{
  BOOLEAN KPDebug;
  BOOLEAN KPKernelCpu;
  BOOLEAN KPKernelLapic;
  BOOLEAN KPKernelXCPM;
  BOOLEAN KPKernelPm; 
  BOOLEAN KPAppleIntelCPUPM;
  BOOLEAN KPAppleRTC;
  BOOLEAN KPDELLSMBIOS;  // Dell SMBIOS patch
  BOOLEAN KPPanicNoKextDump;
  UINT8   pad[3];
  UINT32  FakeCPUID;
//  UINT32  align0;
  CHAR16  *KPATIConnectorsController;
#if defined(MDE_CPU_IA32)
  UINT32  align1;
#endif
  
  UINT8   *KPATIConnectorsData;
#if defined(MDE_CPU_IA32)
  UINT32  align2;
#endif
  
  UINTN   KPATIConnectorsDataLen;
#if defined(MDE_CPU_IA32)
  UINT32  align3;
#endif
  UINT8   *KPATIConnectorsPatch;
#if defined(MDE_CPU_IA32)
  UINT32  align4;
#endif
  
  INT32   NrKexts;
  UINT32  align40;
  KEXT_PATCH *KextPatches;   //zzzz
#if defined(MDE_CPU_IA32)
  UINT32  align5;
#endif
  
  INT32    NrForceKexts;
  UINT32  align50;
  CHAR16 **ForceKexts;
#if defined(MDE_CPU_IA32)
  UINT32 align6;
#endif
  INT32   NrKernels;
  KERNEL_PATCH *KernelPatches;
  INT32   NrBoots;
  KERNEL_PATCH *BootPatches;
  
} KERNEL_AND_KEXT_PATCHES;

typedef struct {
  REFIT_MENU_ENTRY  me;
  REFIT_VOLUME     *Volume;
  CHAR16           *DevicePathString;
  CHAR16           *LoadOptions; //moved here for compatibility with legacy
  UINTN             BootNum;
  CHAR16           *LoaderPath;
  CHAR16           *VolName;
  EFI_DEVICE_PATH  *DevicePath;
  UINT16            Flags;
  UINT8             LoaderType;
  CHAR8            *OSVersion;
  CHAR8            *BuildVersion;
  EG_PIXEL         *BootBgColor;
  UINT8             CustomBoot;
  EG_IMAGE         *CustomLogo;
  KERNEL_AND_KEXT_PATCHES *KernelAndKextPatches;
  CHAR16            *Settings;
} LOADER_ENTRY;

typedef struct {
  REFIT_MENU_ENTRY  me;
  REFIT_VOLUME     *Volume;
  CHAR16           *DevicePathString;
  CONST CHAR16     *LoadOptions;
  UINTN             BootNum;
  CHAR16           *LoaderPath; //will be set to NULL
} LEGACY_ENTRY;

#define ANIME_INFINITE ((UINTN)-1)
//some unreal values
#define SCREEN_EDGE_LEFT    50000
#define SCREEN_EDGE_TOP     60000
#define SCREEN_EDGE_RIGHT   70000
#define SCREEN_EDGE_BOTTOM  80000

typedef struct GUI_ANIME GUI_ANIME;
struct GUI_ANIME {
  UINTN      ID;
  CHAR16    *Path;
  UINTN      Frames;
  UINTN      FrameTime;
  INTN       FilmX, FilmY;  //relative
  INTN       ScreenEdgeHorizontal;
  INTN       ScreenEdgeVertical;
  INTN       NudgeX, NudgeY;
  BOOLEAN    Once;
  GUI_ANIME *Next;
};

extern EFI_HANDLE       SelfImageHandle;
extern EFI_HANDLE       SelfDeviceHandle;
extern EFI_LOADED_IMAGE *SelfLoadedImage;
extern EFI_FILE         *SelfRootDir;
extern EFI_FILE         *SelfDir;
extern CHAR16           *SelfDirPath;
extern EFI_DEVICE_PATH  *SelfDevicePath;
extern EFI_DEVICE_PATH  *SelfFullDevicePath;
extern EFI_FILE         *ThemeDir;
extern CHAR16           *ThemePath;
extern EFI_FILE         *OEMDir;
extern CHAR16           *OEMPath;
extern EFI_FILE         *OemThemeDir;

extern BOOLEAN          MainAnime;
extern GUI_ANIME        *GuiAnime;

extern REFIT_VOLUME     *SelfVolume;
extern REFIT_VOLUME     **Volumes;
extern UINTN            VolumesCount;

extern EG_IMAGE         *Banner;
extern EG_IMAGE         *BigBack;
extern EG_IMAGE         *FontImage;
extern EG_IMAGE         *SelectionImages[];
extern EG_IMAGE         *Buttons[];
extern BOOLEAN          gThemeChanged;
//extern BOOLEAN          gBootArgsChanged;
extern BOOLEAN          gBootChanged;
extern BOOLEAN          gThemeOptionsChanged;
//extern POINTERS         gPointer;
extern REFIT_MENU_SCREEN OptionMenu;
//extern EFI_GUID gEfiAppleBootGuid;


EFI_STATUS  InitRefitLib(IN EFI_HANDLE ImageHandle);
EFI_STATUS  GetRootFromPath(IN EFI_DEVICE_PATH_PROTOCOL* DevicePath, OUT EFI_FILE **Root);
VOID        UninitRefitLib(VOID);
EFI_STATUS  ReinitRefitLib(VOID);
EFI_STATUS  ReinitSelfLib(VOID);
//extern EFI_STATUS FinishInitRefitLib(VOID); -- static
VOID        PauseForKey(IN CHAR16 *Msg);
BOOLEAN     IsEmbeddedTheme(VOID);
UINT8       GetOSTypeFromPath (IN  CHAR16 *Path);

VOID CreateList(OUT VOID ***ListPtr, OUT UINTN *ElementCount, IN UINTN InitialElementCount);
VOID AddListElement(IN OUT VOID ***ListPtr, IN OUT UINTN *ElementCount, IN VOID *NewElement);
//VOID FreeList(IN OUT VOID ***ListPtr, IN OUT UINTN *ElementCount /*, IN Callback*/);

VOID GetListOfThemes(VOID);
VOID GetListOfConfigs(VOID);
VOID GetListOfACPI(VOID);
VOID GetListOfDsdts(VOID);

// syscl - get list of inject kext(s)
VOID GetListOfInjectKext(CHAR16 *);

EFI_STATUS ExtractLegacyLoaderPaths(EFI_DEVICE_PATH **PathList, UINTN MaxPaths, EFI_DEVICE_PATH **HardcodedPathList);

VOID ScanVolumes(VOID);

REFIT_VOLUME *FindVolumeByName(IN CHAR16 *VolName);

BOOLEAN FileExists(IN EFI_FILE *BaseDir, IN CHAR16 *RelativePath);

BOOLEAN DeleteFile(IN EFI_FILE *Root, IN CHAR16 *RelativePath);

EFI_STATUS DirNextEntry(IN EFI_FILE *Directory, IN OUT EFI_FILE_INFO **DirEntry, IN UINTN FilterMode);

VOID    DirIterOpen(IN EFI_FILE *BaseDir, IN CHAR16 *RelativePath OPTIONAL, OUT REFIT_DIR_ITER *DirIter);
BOOLEAN DirIterNext(IN OUT REFIT_DIR_ITER *DirIter, IN UINTN FilterMode, IN CHAR16 *FilePattern OPTIONAL, OUT EFI_FILE_INFO **DirEntry);
EFI_STATUS DirIterClose(IN OUT REFIT_DIR_ITER *DirIter);

CHAR16 * Basename(IN CHAR16 *Path);
VOID   ReplaceExtension(IN OUT CHAR16 *Path, IN CHAR16 *Extension);
CHAR16 * egFindExtension(IN CHAR16 *FileName);

INTN FindMem(IN CONST VOID *Buffer, IN UINTN BufferLength, IN CONST VOID *SearchString, IN UINTN SearchStringLength);

CHAR16 *FileDevicePathToStr(IN EFI_DEVICE_PATH_PROTOCOL *DevPath);
CHAR16 *FileDevicePathFileToStr(IN EFI_DEVICE_PATH_PROTOCOL *DevPath);
//UINTN   FileDevicePathNameLen(IN CONST FILEPATH_DEVICE_PATH  *FilePath);

EFI_STATUS InitializeUnicodeCollationProtocol (VOID);

//
// screen module
//

#define ATTR_BASIC (EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK)
#define ATTR_ERROR (EFI_RED | EFI_BACKGROUND_BLACK)
#define ATTR_BANNER (EFI_WHITE | EFI_BACKGROUND_BLACK)
#define ATTR_CHOICE_BASIC ATTR_BASIC
#define ATTR_CHOICE_CURRENT (EFI_WHITE | EFI_BACKGROUND_LIGHTGRAY)
#define ATTR_SCROLLARROW (EFI_LIGHTGREEN | EFI_BACKGROUND_BLACK)

#define LAYOUT_TEXT_WIDTH (500)
#define LAYOUT_TOTAL_HEIGHT (376)
#define LAYOUT_BANNER_HEIGHT (32)
#define LAYOUT_BANNER_YOFFSET (LAYOUT_BANNER_HEIGHT + 32)
#define LAYOUT_Y_EDGE    (20)
#define LAYOUT_X_EDGE    (20)
#define BAR_WIDTH         (16)

extern INTN FontWidth;
extern INTN FontHeight;
extern INTN TextHeight;
extern INTN row0TileSize;
extern INTN row1TileSize;
extern INTN BCSMargin;

extern INTN LayoutBannerOffset;
extern INTN LayoutButtonOffset;
extern INTN LayoutTextOffset;
extern INTN LayoutAnimMoveForMenuX;
extern INTN LayoutMainMenuHeight;

extern UINTN ConWidth;
extern UINTN ConHeight;
extern CHAR16 *BlankLine;

extern INTN UGAWidth;
extern INTN UGAHeight;
extern BOOLEAN AllowGraphicsMode;

extern EG_PIXEL StdBackgroundPixel;
extern EG_PIXEL MenuBackgroundPixel;
extern EG_PIXEL InputBackgroundPixel;
extern EG_PIXEL BlueBackgroundPixel;
extern EG_PIXEL DarkBackgroundPixel;
extern EG_PIXEL SelectionBackgroundPixel;
extern EG_PIXEL DarkEmbeddedBackgroundPixel;
extern EG_PIXEL DarkSelectionPixel;
extern EG_PIXEL WhitePixel;
extern EG_PIXEL BlackPixel;

extern EG_RECT  BannerPlace;
extern EG_IMAGE *BackgroundImage;


VOID InitScreen(IN BOOLEAN SetMaxResolution);
VOID SetupScreen(VOID);
VOID BeginTextScreen(IN CHAR16 *Title);
VOID FinishTextScreen(IN BOOLEAN WaitAlways);
VOID BeginExternalScreen(IN BOOLEAN UseGraphicsMode, IN CHAR16 *Title);
VOID FinishExternalScreen(VOID);
VOID TerminateScreen(VOID);
VOID SetNextScreenMode(INT32);

#if REFIT_DEBUG > 0
VOID DebugPause(VOID);
#else
#define DebugPause()
#endif
VOID EndlessIdleLoop(VOID);

BOOLEAN CheckFatalError(IN EFI_STATUS Status, IN CHAR16 *where);
BOOLEAN CheckError(IN EFI_STATUS Status, IN CHAR16 *where);

VOID SwitchToGraphicsAndClear(VOID);
VOID BltClearScreen(IN BOOLEAN ShowBanner);
VOID BltImage(IN EG_IMAGE *Image, IN INTN XPos, IN INTN YPos);
VOID BltImageAlpha(IN EG_IMAGE *Image, IN INTN XPos, IN INTN YPos, IN EG_PIXEL *BackgroundPixel, INTN Scale);
VOID BltImageComposite(IN EG_IMAGE *BaseImage, IN EG_IMAGE *TopImage, IN INTN XPos, IN INTN YPos);
VOID BltImageCompositeBadge(IN EG_IMAGE *BaseImage, IN EG_IMAGE *TopImage, IN EG_IMAGE *BadgeImage, IN INTN XPos, IN INTN YPos, INTN Scale);
//VOID BltImageCompositeIndicator(IN EG_IMAGE *BaseImage, IN EG_IMAGE *TopImage, IN INTN XPos, IN INTN YPos, INTN Scale);

BOOLEAN GetAnime(REFIT_MENU_SCREEN *Screen);
VOID    InitAnime(REFIT_MENU_SCREEN *Screen);
VOID    UpdateAnime(REFIT_MENU_SCREEN *Screen, EG_RECT *Place);
VOID    FreeAnime(GUI_ANIME *Anime);

//
// icns loader module
//

EG_IMAGE * LoadOSIcon(IN CHAR16 *OSIconName OPTIONAL, IN CHAR16 *FallbackIconName, IN UINTN PixelSize, IN BOOLEAN BootLogo, IN BOOLEAN WantDummy);

EG_IMAGE * LoadIcns(IN EFI_FILE_HANDLE BaseDir, IN CHAR16 *FileName, IN UINTN PixelSize);
EG_IMAGE * LoadIcnsFallback(IN EFI_FILE_HANDLE BaseDir, IN CHAR16 *FileName, IN UINTN PixelSize);
EG_IMAGE * DummyImage(IN UINTN PixelSize);
EG_IMAGE * BuiltinIcon(IN UINTN Id);
CHAR16   * GetIconsExt(IN CHAR16 *Icon, IN CHAR16 *Def);
EG_IMAGE * GetSmallHover(IN UINTN Id);

#define BUILTIN_ICON_FUNC_ABOUT                (0)
#define BUILTIN_ICON_FUNC_OPTIONS              (1)
#define BUILTIN_ICON_FUNC_CLOVER               (2)
#define BUILTIN_ICON_FUNC_SECURE_BOOT          (3)
#define BUILTIN_ICON_FUNC_SECURE_BOOT_CONFIG   (4)
#define BUILTIN_ICON_FUNC_RESET                (5)
#define BUILTIN_ICON_FUNC_EXIT                 (6)
#define BUILTIN_ICON_FUNC_HELP                 (7)
#define BUILTIN_ICON_TOOL_SHELL                (8)
#define BUILTIN_ICON_TOOL_PART                 (9)
#define BUILTIN_ICON_TOOL_RESCUE               (10)
#define BUILTIN_ICON_POINTER                   (11)
#define BUILTIN_ICON_VOL_INTERNAL              (12)
#define BUILTIN_ICON_VOL_EXTERNAL              (13)
#define BUILTIN_ICON_VOL_OPTICAL               (14)
#define BUILTIN_ICON_VOL_FIREWIRE              (15)
#define BUILTIN_ICON_VOL_BOOTER                (16)
#define BUILTIN_ICON_VOL_INTERNAL_HFS          (17)
#define BUILTIN_ICON_VOL_INTERNAL_APFS         (18)
#define BUILTIN_ICON_VOL_INTERNAL_NTFS         (19)
#define BUILTIN_ICON_VOL_INTERNAL_EXT3         (20)
#define BUILTIN_ICON_VOL_INTERNAL_REC          (21)
#define BUILTIN_ICON_BANNER                    (22)
#define BUILTIN_SELECTION_SMALL                (23)
#define BUILTIN_SELECTION_BIG                  (24)
#define BUILTIN_ICON_COUNT                     (25)
#define BUILTIN_ICON_BACKGROUND                (100)
#define BUILTIN_ICON_SELECTION                 (101)
#define BUILTIN_ICON_ANIME                     (102)
//
// menu module
//

#define MENU_EXIT_ENTER       ((ACTION)(1))
#define MENU_EXIT_ESCAPE      (2)
#define MENU_EXIT_DETAILS     (3)
#define MENU_EXIT_TIMEOUT     (4)
#define MENU_EXIT_OPTIONS     (5)
#define MENU_EXIT_EJECT       (6)
#define MENU_EXIT_HELP        (7)
#define MENU_EXIT_HIDE_TOGGLE (8)

#define X_IS_LEFT    64
#define X_IS_RIGHT   0
#define X_IS_CENTER  1
#define BADGE_DIMENSION 64

// IconFormat
#define ICON_FORMAT_DEF       (0)
#define ICON_FORMAT_ICNS      (1)
#define ICON_FORMAT_PNG       (2)
#define ICON_FORMAT_BMP       (3)

VOID AddMenuInfoLine(IN REFIT_MENU_SCREEN *Screen, IN CHAR16 *InfoLine);
VOID AddMenuInfo(IN REFIT_MENU_SCREEN  *SubScreen, IN CHAR16 *Line);
VOID AddMenuEntry(IN REFIT_MENU_SCREEN *Screen, IN REFIT_MENU_ENTRY *Entry);
VOID AddMenuCheck(REFIT_MENU_SCREEN *SubScreen, CONST CHAR8 *Text, UINTN Bit, INTN ItemNum);
VOID FreeMenu(IN REFIT_MENU_SCREEN *Screen);
UINTN RunMenu(IN REFIT_MENU_SCREEN *Screen, OUT REFIT_MENU_ENTRY **ChosenEntry);
UINTN RunMainMenu(IN REFIT_MENU_SCREEN *Screen, IN INTN DefaultSelection, OUT REFIT_MENU_ENTRY **ChosenEntry);
VOID DrawMenuText(IN CHAR16 *Text, IN INTN SelectedWidth, IN INTN XPos, IN INTN YPos, IN INTN Cursor);
VOID ReinitVolumes(VOID);
BOOLEAN ReadAllKeyStrokes(VOID);
VOID OptionsMenu(OUT REFIT_MENU_ENTRY **ChosenEntry, IN CHAR8 *LastChosenOS);
VOID FreeScrollBar(VOID);
INTN DrawTextXY(IN CHAR16 *Text, IN INTN XPos, IN INTN YPos, IN UINT8 XAlign);
VOID DrawBCSText(IN CHAR16 *Text, IN INTN XPos, IN INTN YPos, IN UINT8 XAlign);

//
// config module
//

typedef struct MISC_ICONS {
  EG_IMAGE *image;
  CHAR8    *name;
} MISC_ICONS;

extern MISC_ICONS OSIconsTable[];
extern BUILTIN_ICON BuiltinIconTable[];
extern REFIT_CONFIG GlobalConfig;

VOID ReadConfig(INTN What);
//
// BmLib
//
extern EFI_STATUS
EfiLibLocateProtocol (
                      IN  EFI_GUID    *ProtocolGuid,
                      OUT VOID        **Interface
                      );


extern EFI_FILE_HANDLE
EfiLibOpenRoot (
                IN EFI_HANDLE                   DeviceHandle
                );

extern EFI_FILE_SYSTEM_VOLUME_LABEL *
EfiLibFileSystemVolumeLabelInfo (
                                 IN EFI_FILE_HANDLE      FHand
                                 );
extern CHAR16 *
EfiStrDuplicate (
                 IN CONST CHAR16   *Src
                 );

extern INTN StriCmp (
                     IN      CONST CHAR16              *FirstString,
                     IN      CONST CHAR16              *SecondString
                     );

extern INTN EFIAPI AsciiStriCmp (
                          IN      CONST CHAR8              *FirstString,
                          IN      CONST CHAR8              *SecondString
                          );

extern BOOLEAN AsciiStriNCmp (
                              IN      CONST CHAR8              *FirstString,
                              IN      CONST CHAR8              *SecondString,
                              IN      CONST UINTN               sSize
                              );

extern BOOLEAN AsciiStrStriN (
                              IN      CONST CHAR8              *WhatString,
                              IN      CONST UINTN               sWhatSize,
                              IN      CONST CHAR8              *WhereString,
                              IN      CONST UINTN               sWhereSize
                              );

extern EFI_FILE_INFO * EfiLibFileInfo (IN EFI_FILE_HANDLE      FHand);
extern EFI_FILE_SYSTEM_INFO * EfiLibFileSystemInfo (IN EFI_FILE_HANDLE   Root);

extern UINTN
EfiDevicePathInstanceCount (
                            IN EFI_DEVICE_PATH_PROTOCOL      *DevicePath
                            );

extern VOID *
EfiReallocatePool (
                   IN VOID                 *OldPool,
                   IN UINTN                OldSize,
                   IN UINTN                NewSize
                   );

extern BOOLEAN
TimeCompare (
             IN EFI_TIME               *FirstTime,
             IN EFI_TIME               *SecondTime
             );

extern BOOLEAN DumpVariable(CHAR16* Name, EFI_GUID* Guid, INTN DevicePathAt);
#ifdef DUMP_KERNEL_KEXT_PATCHES
// Utils functions
VOID DumpKernelAndKextPatches(KERNEL_AND_KEXT_PATCHES *Patches);
#endif
//VOID FilterKextPatches(IN LOADER_ENTRY *Entry);


UINT32 EncodeOptions(CHAR16 *Options);

CHAR8* GetUnicodeChar(CHAR8 *s, CHAR16* UnicodeChar);

#define KERNEL_MAX_SIZE 40000000
#if defined(FKERNELPATCH)
#define FSearchReplace(Source, Search, Replace) SearchAndReplace(Source, KERNEL_MAX_SIZE, Search, sizeof(Search), Replace, 1)
#endif //FKERNELPATCH

VOID DbgHeader(CHAR8 *str);

#endif
/*
 
 EOF */
