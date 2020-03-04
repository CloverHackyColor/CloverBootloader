/*
 *
 * Copyright (c) 2020 Jief
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

#ifndef __menu_items_H__
#define __menu_items_H__


#include "libeg.h"
#include "../../refit/lib.h"
#ifdef __cplusplus
#include "../cpp_foundation/XObjArray.h"
#include "../cpp_foundation/XStringWArray.h"
#include "../cpp_foundation/XStringW.h"
#include "../../libeg/XPointer.h"
#endif

//
//#define REFIT_DEBUG (2)
//#define Print if ((!GlobalConfig.Quiet) || (GlobalConfig.TextOnly)) Print
////#include "GenericBdsLib.h"


//#define TAG_ABOUT_OLD              (1)
//#define TAG_RESET_OLD              (2)
//#define TAG_SHUTDOWN_OLD           (3)
//#define TAG_TOOL_OLD               (4)
////#define TAG_LOADER             (5)
////#define TAG_LEGACY             (6)
//#define TAG_INFO_OLD               (7)
//#define TAG_OPTIONS            (8)
//#define TAG_INPUT_OLD              (9)
//#define TAG_HELP_OLD               (10) // wasn't used ?
//#define TAG_SWITCH_OLD             (11)
//#define TAG_CHECKBIT_OLD           (12)
//#define TAG_SECURE_BOOT_OLD        (13)
//#define TAG_SECURE_BOOT_CONFIG_OLD (14)
//#define TAG_CLOVER_OLD             (100)
//#define TAG_EXIT_OLD               (101)
//#define TAG_RETURN_OLD             ((UINTN)(-1))

//typedef struct _refit_menu_screen REFIT_MENU_SCREEN;
class REFIT_MENU_SCREEN;
class REFIT_MENU_SWITCH;
class REFIT_MENU_CHECKBIT;
class REFIT_MENU_ENTRY_CLOVER;
class REFIT_MENU_ITEM_RETURN;
class REFIT_INPUT_DIALOG;
class REFIT_INFO_DIALOG;
class REFIT_MENU_ENTRY_LOADER_TOOL;
class REFIT_MENU_ITEM_SHUTDOWN;
class REFIT_MENU_ITEM_RESET;
class REFIT_MENU_ITEM_ABOUT;
class REFIT_MENU_ITEM_OPTIONS;
class REFIT_MENU_ITEM_ABSTRACT_ENTRY_LOADER;
class LOADER_ENTRY;
class LEGACY_ENTRY;
class REFIT_MENU_ENTRY_OTHER;
class REFIT_SIMPLE_MENU_ENTRY_TAG;
class REFIT_MENU_ITEM_IEM_ABSTRACT;
class XPointer;

class REFIT_ABSTRACT_MENU_ENTRY
{
  public:
  CONST CHAR16       *Title;
  UINTN              Row;
  CHAR16             ShortcutDigit;
  CHAR16             ShortcutLetter;
  EG_IMAGE          *Image;
  EG_RECT            Place;
  ACTION             AtClick;
  ACTION             AtDoubleClick;
  ACTION             AtRightClick;
  ACTION             AtMouseOver;
  REFIT_MENU_SCREEN *SubScreen;

  virtual EG_IMAGE* getDriveImage() const { return nullptr; };
  virtual EG_IMAGE* getBadgeImage() const { return nullptr; };


  virtual REFIT_SIMPLE_MENU_ENTRY_TAG* getREFIT_SIMPLE_MENU_ENTRY_TAG() { return nullptr; };
  virtual REFIT_MENU_SWITCH* getREFIT_MENU_SWITCH() { return nullptr; };
  virtual REFIT_MENU_CHECKBIT* getREFIT_MENU_CHECKBIT() { return nullptr; };
  virtual REFIT_MENU_ENTRY_CLOVER* getREFIT_MENU_ENTRY_CLOVER() { return nullptr; };
  virtual REFIT_MENU_ITEM_RETURN* getREFIT_MENU_ITEM_RETURN() { return nullptr; };
  virtual REFIT_INPUT_DIALOG* getREFIT_INPUT_DIALOG() { return nullptr; };
  virtual REFIT_INFO_DIALOG* getREFIT_INFO_DIALOG() { return nullptr; };
  virtual REFIT_MENU_ENTRY_LOADER_TOOL* getREFIT_MENU_ENTRY_LOADER_TOOL() { return nullptr; };
  virtual REFIT_MENU_ITEM_SHUTDOWN* getREFIT_MENU_ITEM_SHUTDOWN() { return nullptr; };
  virtual REFIT_MENU_ITEM_RESET* getREFIT_MENU_ITEM_RESET() { return nullptr; };
  virtual REFIT_MENU_ITEM_ABOUT* getREFIT_MENU_ITEM_ABOUT() { return nullptr; };
  virtual REFIT_MENU_ITEM_OPTIONS* getREFIT_MENU_ITEM_OPTIONS() { return nullptr; };
  virtual REFIT_MENU_ITEM_ABSTRACT_ENTRY_LOADER* getREFIT_MENU_ITEM_ABSTRACT_ENTRY_LOADER() { return nullptr; };
  virtual LOADER_ENTRY* getLOADER_ENTRY() { return nullptr; };
  virtual LEGACY_ENTRY* getLEGACY_ENTRY() { return nullptr; };
  virtual REFIT_MENU_ENTRY_OTHER* getREFIT_MENU_ENTRY_OTHER() { return nullptr; };
  virtual REFIT_MENU_ITEM_IEM_ABSTRACT* getREFIT_MENU_ITEM_IEM_ABSTRACT() { return nullptr; };

  REFIT_ABSTRACT_MENU_ENTRY(CONST CHAR16 *Title_) : Title(Title_), Row(0), ShortcutDigit(0), ShortcutLetter(0), Image(NULL), Place({0,0,0,0}), AtClick(ActionNone), AtDoubleClick(ActionNone), AtRightClick(ActionNone), AtMouseOver(ActionNone), SubScreen(NULL) {};
  REFIT_ABSTRACT_MENU_ENTRY(CONST CHAR16 *Title_, UINTN Row_,
                            CHAR16 ShortcutDigit_, CHAR16 ShortcutLetter_, EG_IMAGE* Image_,
                            EG_RECT Place_, ACTION AtClick_, ACTION AtDoubleClick_, ACTION AtRightClick_, ACTION AtMouseOver_,
                            REFIT_MENU_SCREEN *SubScreen_)
                          : Title(Title_), Row(Row_), ShortcutDigit(ShortcutDigit_), ShortcutLetter(ShortcutLetter_),
                          Image(Image_), Place(Place_),
                          AtClick(AtClick_), AtDoubleClick(AtDoubleClick_), AtRightClick(AtRightClick_), AtMouseOver(AtMouseOver_),
                          SubScreen(SubScreen_) {};
  virtual ~REFIT_ABSTRACT_MENU_ENTRY() {}; // virtual destructor : this is vital
};

class REFIT_SIMPLE_MENU_ENTRY_TAG : public REFIT_ABSTRACT_MENU_ENTRY
{
public:
  UINTN              Tag;
  ACTION             AtClick;

  REFIT_SIMPLE_MENU_ENTRY_TAG(CONST CHAR16 *Title_, UINTN Tag_, ACTION AtClick_)
             : REFIT_ABSTRACT_MENU_ENTRY(Title_), Tag(Tag_), AtClick(AtClick_)
             {};

  virtual REFIT_SIMPLE_MENU_ENTRY_TAG* getREFIT_SIMPLE_MENU_ENTRY_TAG() { return this; };
};

class REFIT_MENU_ENTRY : public REFIT_ABSTRACT_MENU_ENTRY
{
public:
//  CONST CHAR16       *Title;
//  UINTN              Tag;
//  UINTN              Row;
//  CHAR16             ShortcutDigit;
//  CHAR16             ShortcutLetter;
//  EG_IMAGE          *Image;
//  EG_IMAGE          *DriveImage;
//  EG_IMAGE          *BadgeImage;
//  EG_RECT            Place;
//  ACTION             AtClick;
//  ACTION             AtDoubleClick;
//  ACTION             AtRightClick;
//  ACTION             AtMouseOver;
//  REFIT_MENU_SCREEN *SubScreen;

  REFIT_MENU_ENTRY() : REFIT_ABSTRACT_MENU_ENTRY(NULL) {};
  REFIT_MENU_ENTRY(  CONST CHAR16 *Title_, UINTN Row_, CHAR16 ShortcutDigit_, CHAR16 ShortcutLetter_,
                     EG_IMAGE *Image_, EG_RECT Place_,
                     ACTION AtClick_, ACTION AtDoubleClick_, ACTION AtRightClick_,  ACTION AtMouseOver_,
                     REFIT_MENU_SCREEN *SubScreen_)
             : REFIT_ABSTRACT_MENU_ENTRY(Title_, Row_, ShortcutDigit_, ShortcutLetter_, Image_, Place_, AtClick_, AtDoubleClick_, AtRightClick_, AtMouseOver_, SubScreen_)
             {};
};

class REFIT_MENU_ENTRY_OTHER : public REFIT_MENU_ENTRY
{
public:
//  UINTN              Tag;

  REFIT_MENU_ENTRY_OTHER() : REFIT_MENU_ENTRY() {};
  REFIT_MENU_ENTRY_OTHER(CONST CHAR16 *Title_, UINTN Row_, CHAR16 ShortcutDigit_, CHAR16 ShortcutLetter_,
                     EG_IMAGE* Image_, EG_RECT Place_,
                     ACTION AtClick_, ACTION AtDoubleClick_, ACTION AtRightClick_, ACTION AtMouseOver_,
                     REFIT_MENU_SCREEN *SubScreen_)
             : REFIT_MENU_ENTRY(Title_, Row_, ShortcutDigit_, ShortcutLetter_, Image_, Place_, AtClick_, AtDoubleClick_, AtRightClick_, AtMouseOver_, SubScreen_)
             {};

  virtual REFIT_MENU_ENTRY_OTHER* getREFIT_MENU_ENTRY_OTHER() { return this; };
};

class REFIT_MENU_ITEM_RETURN : public REFIT_MENU_ENTRY_OTHER
{
public:
  REFIT_MENU_ITEM_RETURN(CONST CHAR16 *Title_, UINTN Row_, CHAR16 ShortcutDigit_, CHAR16 ShortcutLetter_, ACTION AtClick_)
             : REFIT_MENU_ENTRY_OTHER(Title_, Row_, ShortcutDigit_, ShortcutLetter_, NULL, {0, 0, 0, 0}, AtClick_, ActionEnter, ActionNone, ActionNone, NULL)
             {};
  REFIT_MENU_ITEM_RETURN(CONST CHAR16 *Title_, UINTN Row_, CHAR16 ShortcutDigit_, CHAR16 ShortcutLetter_,
                     EG_IMAGE* Image_, EG_RECT Place_,
                     ACTION AtClick_, ACTION AtDoubleClick_, ACTION AtRightClick_, ACTION AtMouseOver_,
                     REFIT_MENU_SCREEN *SubScreen_)
             : REFIT_MENU_ENTRY_OTHER(Title_, Row_, ShortcutDigit_, ShortcutLetter_, Image_, Place_, AtClick_, AtDoubleClick_, AtRightClick_, AtMouseOver_, SubScreen_)
             {};
  virtual REFIT_MENU_ITEM_RETURN* getREFIT_MENU_ITEM_RETURN() { return this; };
};

class REFIT_MENU_ITEM_SHUTDOWN : public REFIT_MENU_ENTRY_OTHER
{
public:
  REFIT_MENU_ITEM_SHUTDOWN(CONST CHAR16 *Title_, UINTN Row_, CHAR16 ShortcutDigit_, CHAR16 ShortcutLetter_, ACTION AtClick_)
             : REFIT_MENU_ENTRY_OTHER(Title_, Row_, ShortcutDigit_, ShortcutLetter_, NULL, {0, 0, 0, 0}, AtClick_, ActionEnter, ActionNone, ActionNone, NULL)
             {};
  REFIT_MENU_ITEM_SHUTDOWN(CONST CHAR16 *Title_, UINTN Row_, CHAR16 ShortcutDigit_, CHAR16 ShortcutLetter_,
                     EG_IMAGE* Image_, EG_RECT Place_,
                     ACTION AtClick_, ACTION AtDoubleClick_, ACTION AtRightClick_, ACTION AtMouseOver_,
                     REFIT_MENU_SCREEN *SubScreen_)
             : REFIT_MENU_ENTRY_OTHER(Title_, Row_, ShortcutDigit_, ShortcutLetter_, Image_, Place_, AtClick_, AtDoubleClick_, AtRightClick_, AtMouseOver_, SubScreen_)
             {};
  virtual REFIT_MENU_ITEM_SHUTDOWN* getREFIT_MENU_ITEM_SHUTDOWN() { return this; };
};

class REFIT_MENU_ITEM_RESET : public REFIT_MENU_ENTRY_OTHER {
public:
  REFIT_MENU_ITEM_RESET(CONST CHAR16 *Title_, UINTN Row_, CHAR16 ShortcutDigit_, CHAR16 ShortcutLetter_, ACTION AtClick_)
             : REFIT_MENU_ENTRY_OTHER(Title_, Row_, ShortcutDigit_, ShortcutLetter_, NULL, {0, 0, 0, 0}, AtClick_, ActionEnter, ActionNone, ActionNone, NULL)
             {};
  REFIT_MENU_ITEM_RESET(CONST CHAR16 *Title_, UINTN Row_, CHAR16 ShortcutDigit_, CHAR16 ShortcutLetter_,
                     EG_IMAGE* Image_, EG_RECT Place_,
                     ACTION AtClick_, ACTION AtDoubleClick_, ACTION AtRightClick_, ACTION AtMouseOver_,
                     REFIT_MENU_SCREEN *SubScreen_)
             : REFIT_MENU_ENTRY_OTHER(Title_, Row_, ShortcutDigit_, ShortcutLetter_, Image_, Place_, AtClick_, AtDoubleClick_, AtRightClick_, AtMouseOver_, SubScreen_)
             {};
  virtual REFIT_MENU_ITEM_RESET* getREFIT_MENU_ITEM_RESET() { return this; };
};

class REFIT_MENU_ITEM_ABOUT : public REFIT_MENU_ENTRY_OTHER
{
public:
  REFIT_MENU_ITEM_ABOUT(CONST CHAR16 *Title_, UINTN Row_, CHAR16 ShortcutDigit_, CHAR16 ShortcutLetter_, ACTION AtClick_)
             : REFIT_MENU_ENTRY_OTHER(Title_, Row_, ShortcutDigit_, ShortcutLetter_, NULL, {0, 0, 0, 0}, AtClick_, ActionEnter, ActionNone, ActionNone, NULL)
             {};
  REFIT_MENU_ITEM_ABOUT(CONST CHAR16 *Title_, UINTN Row_, CHAR16 ShortcutDigit_, CHAR16 ShortcutLetter_,
                     EG_IMAGE* Image_, EG_RECT Place_,
                     ACTION AtClick_, ACTION AtDoubleClick_, ACTION AtRightClick_, ACTION AtMouseOver_,
                     REFIT_MENU_SCREEN *SubScreen_)
             : REFIT_MENU_ENTRY_OTHER(Title_, Row_, ShortcutDigit_, ShortcutLetter_, Image_, Place_, AtClick_, AtDoubleClick_, AtRightClick_, AtMouseOver_, SubScreen_)
             {};
  virtual REFIT_MENU_ITEM_ABOUT* getREFIT_MENU_ITEM_ABOUT() { return this; };
};

class REFIT_MENU_ITEM_OPTIONS : public REFIT_MENU_ENTRY_OTHER {
public:
  REFIT_MENU_ITEM_OPTIONS() : REFIT_MENU_ENTRY_OTHER() {};
  REFIT_MENU_ITEM_OPTIONS(CONST CHAR16 *Title_, UINTN Row_, CHAR16 ShortcutDigit_, CHAR16 ShortcutLetter_, ACTION AtClick_)
             : REFIT_MENU_ENTRY_OTHER(Title_, Row_, ShortcutDigit_, ShortcutLetter_, NULL, {0, 0, 0, 0}, AtClick_, ActionEnter, ActionNone, ActionNone, NULL)
             {};
  REFIT_MENU_ITEM_OPTIONS(CONST CHAR16 *Title_, UINTN Row_, CHAR16 ShortcutDigit_, CHAR16 ShortcutLetter_,
                     EG_IMAGE* Image_, EG_RECT Place_,
                     ACTION AtClick_, ACTION AtDoubleClick_, ACTION AtRightClick_, ACTION AtMouseOver_,
                     REFIT_MENU_SCREEN *SubScreen_)
             : REFIT_MENU_ENTRY_OTHER(Title_, Row_, ShortcutDigit_, ShortcutLetter_, Image_, Place_, AtClick_, AtDoubleClick_, AtRightClick_, AtMouseOver_, SubScreen_)
             {};
  virtual REFIT_MENU_ITEM_OPTIONS* getREFIT_MENU_ITEM_OPTIONS() { return this; };
};

class REFIT_MENU_ITEM_IEM_ABSTRACT : public REFIT_MENU_ENTRY_OTHER {
public:
  INPUT_ITEM        *Item;
  virtual REFIT_MENU_ITEM_IEM_ABSTRACT* getREFIT_MENU_ITEM_IEM_ABSTRACT() { return this; };
};

class REFIT_INPUT_DIALOG : public REFIT_MENU_ITEM_IEM_ABSTRACT {
public:
  virtual REFIT_INPUT_DIALOG* getREFIT_INPUT_DIALOG() { return this; };
};

class REFIT_INFO_DIALOG : public REFIT_MENU_ENTRY_OTHER {
public:
  virtual REFIT_INFO_DIALOG* getREFIT_INFO_DIALOG() { return this; };
};

class REFIT_MENU_SWITCH : public REFIT_MENU_ITEM_IEM_ABSTRACT {
public:
  virtual REFIT_MENU_SWITCH* getREFIT_MENU_SWITCH() { return this; };
};

class REFIT_MENU_CHECKBIT : public REFIT_MENU_ITEM_IEM_ABSTRACT {
public:
  virtual REFIT_MENU_CHECKBIT* getREFIT_MENU_CHECKBIT() { return this; };
};

/*
 * SUper class of LOADER_ENTRY & LEGACY_ENTRY
 */
class REFIT_MENU_ITEM_ABSTRACT_ENTRY_LOADER : public REFIT_MENU_ENTRY
{
public:
  REFIT_VOLUME     *Volume;
  CONST CHAR16     *DevicePathString;
  CONST CHAR16     *LoadOptions; //moved here for compatibility with legacy
  UINTN             BootNum;
  CONST CHAR16     *LoaderPath;

  EG_IMAGE          *DriveImage;
  EG_IMAGE          *BadgeImage;

  REFIT_MENU_ITEM_ABSTRACT_ENTRY_LOADER()
                : Volume(0), DevicePathString(0), LoadOptions(0), BootNum(0), LoaderPath(0), DriveImage(0), BadgeImage(0)
                {}
  virtual EG_IMAGE* getDriveImage() const { return DriveImage; };
  virtual EG_IMAGE* getBadgeImage() const { return BadgeImage; };

  virtual REFIT_MENU_ITEM_ABSTRACT_ENTRY_LOADER* getREFIT_MENU_ITEM_ABSTRACT_ENTRY_LOADER() { return this; };
};

struct KERNEL_AND_KEXT_PATCHES;

class LOADER_ENTRY : public REFIT_MENU_ITEM_ABSTRACT_ENTRY_LOADER
{
public:
  CONST CHAR16     *VolName;
  EFI_DEVICE_PATH  *DevicePath;
  UINT16            Flags;
  UINT8             LoaderType;
  CHAR8            *OSVersion;
  CHAR8            *BuildVersion;
  EG_PIXEL         *BootBgColor;
  UINT8             CustomBoot;
  EG_IMAGE         *CustomLogo;
  KERNEL_AND_KEXT_PATCHES *KernelAndKextPatches;
  CONST CHAR16            *Settings;

  LOADER_ENTRY()
  			: REFIT_MENU_ITEM_ABSTRACT_ENTRY_LOADER(), VolName(0), DevicePath(0), Flags(0), LoaderType(0), OSVersion(0), BuildVersion(0), BootBgColor(0), CustomBoot(0), CustomLogo(0), KernelAndKextPatches(0), Settings(0)
  			{};

  virtual LOADER_ENTRY* getLOADER_ENTRY() { return this; };
} ;

class REFIT_MENU_ENTRY_LOADER_TOOL : public LOADER_ENTRY
{
public:
  virtual REFIT_MENU_ENTRY_LOADER_TOOL* getREFIT_MENU_ENTRY_LOADER_TOOL() { return this; };
};

class LEGACY_ENTRY : public REFIT_MENU_ITEM_ABSTRACT_ENTRY_LOADER
{
public:
//  REFIT_VOLUME     *Volume;
//  CONST CHAR16     *DevicePathString;
//  CONST CHAR16     *LoadOptions;
//  UINTN             BootNum;
//  CONST CHAR16     *LoaderPath; //will be set to NULL

  virtual LEGACY_ENTRY* getLEGACY_ENTRY() { return this; };
};

class REFIT_MENU_ENTRY_CLOVER : public LOADER_ENTRY
{
public:
  virtual REFIT_MENU_ENTRY_CLOVER* getREFIT_MENU_ENTRY_CLOVER() { return this; };
};


//some unreal values
#define FILM_CENTRE   40000
//#define FILM_LEFT     50000
//#define FILM_TOP      50000
//#define FILM_RIGHT    60000
//#define FILM_BOTTOM   60000
//#define FILM_PERCENT 100000
#define INITVALUE       40000

typedef VOID (REFIT_MENU_SCREEN::*MENU_STYLE_FUNC)(IN UINTN Function, IN CONST CHAR16 *ParamText);

class REFIT_MENU_SCREEN
{
public:
  UINTN             ID;
  CONST  CHAR16      *Title;  //Title is not const, but *Title is. It will be better to make it XStringW
  EG_IMAGE          *TitleImage;
//  INTN              InfoLineCount;
//  CONST CHAR16    **InfoLines;
  XStringWArray     InfoLines;
//  INTN              EntryCount;
//  REFIT_MENU_ENTRY  **Entries;
  XObjArray<REFIT_ABSTRACT_MENU_ENTRY> Entries;
  INTN              TimeoutSeconds;
  CONST CHAR16     *TimeoutText;
  CONST CHAR16     *Theme;
  BOOLEAN           AnimeRun;
  BOOLEAN           Once;
  UINT64            LastDraw;
  INTN              CurrentFrame;
  INTN              Frames;
  UINTN             FrameTime; //ms
  EG_RECT           FilmPlace;
  EG_IMAGE        **Film;
  ACTION      mAction;
  UINTN       mItemID;
  XPointer    *mPointer;
  SCROLL_STATE ScrollState;
//  MENU_STYLE_FUNC StyleFunc;


  REFIT_MENU_SCREEN()
						: ID(0), Title(0), TitleImage(0),
						  TimeoutSeconds(0), TimeoutText(0), Theme(0), AnimeRun(0),
						  Once(0), LastDraw(0), CurrentFrame(0),
						  Frames(0), FrameTime(0), FilmPlace({0,0,0,0}),
						  Film(0), mAction(ActionNone), mItemID(0), mPointer(NULL) //, StyleFunc(&REFIT_MENU_SCREEN::TextMenuStyle)
						{};

  REFIT_MENU_SCREEN(UINTN ID, CONST CHAR16* Title, CONST CHAR16* TimeoutText)
						: ID(ID), Title(Title), TitleImage(0),
						  TimeoutSeconds(0), TimeoutText(TimeoutText), Theme(0), AnimeRun(0),
						  Once(0), LastDraw(0), CurrentFrame(0),
						  Frames(0), FrameTime(0), FilmPlace({0,0,0,0}),
						  Film(0), mAction(ActionNone), mItemID(0), mPointer(NULL) //, StyleFunc(&REFIT_MENU_SCREEN::TextMenuStyle)
						{};
  REFIT_MENU_SCREEN(UINTN ID, CONST CHAR16* Title, CONST CHAR16* TimeoutText, REFIT_ABSTRACT_MENU_ENTRY* entry1, REFIT_ABSTRACT_MENU_ENTRY* entry2)
						: ID(ID), Title(Title), TitleImage(0),
						  TimeoutSeconds(0), TimeoutText(TimeoutText), Theme(0), AnimeRun(0),
						  Once(0), LastDraw(0), CurrentFrame(0),
						  Frames(0), FrameTime(0), FilmPlace({0,0,0,0}),
						  Film(0), mAction(ActionNone), mItemID(0), mPointer(NULL) //, StyleFunc(&REFIT_MENU_SCREEN::TextMenuStyle)
						{
	  	  	  	  	  	  	  Entries.AddReference(entry1, false);
	  	  	  	  	  	  	  Entries.AddReference(entry2, false);
						};

//  REFIT_MENU_SCREEN(  UINTN             ID_,
//											CONST  CHAR16      *Title_,
//											EG_IMAGE          *TitleImage_,
////											INTN              InfoLineCount_,
////											CONST CHAR16    **InfoLines_,
//											INTN              TimeoutSeconds_,
//											CONST CHAR16     *TimeoutText_,
//											CONST CHAR16     *Theme_,
//											BOOLEAN           AnimeRun_,
//											BOOLEAN           Once_,
//											UINT64            LastDraw_,
//											INTN              CurrentFrame_,
//											INTN              Frames_,
//											UINTN             FrameTime_,
//											EG_RECT           FilmPlace_,
//											EG_IMAGE        **Film_)
//						: ID(ID_), Title(Title_), TitleImage(TitleImage_),
//						  /*InfoLineCount(InfoLineCount_), InfoLines(InfoLines_),*/ TimeoutSeconds(TimeoutSeconds_),
//						  TimeoutText(TimeoutText_), Theme(Theme_), AnimeRun(AnimeRun_),
//						  Once(Once_), LastDraw(LastDraw_), CurrentFrame(CurrentFrame_),
//						  Frames(Frames_), FrameTime(FrameTime_), FilmPlace(FilmPlace_),
//						  Film(Film_), mAction(ActionNone), mItemID(0), mPointer(NULL)
//						{};
//
//  REFIT_MENU_SCREEN(  UINTN             ID_,
//											  CONST   CHAR16      *Title_,
//											EG_IMAGE          *TitleImage_,
////											INTN              InfoLineCount_,
////											CONST CHAR16    **InfoLines_,
//											REFIT_ABSTRACT_MENU_ENTRY* entry,
//											INTN              TimeoutSeconds_,
//											CONST CHAR16     *TimeoutText_,
//											CONST CHAR16     *Theme_,
//											BOOLEAN           AnimeRun_,
//											BOOLEAN           Once_,
//											UINT64            LastDraw_,
//											INTN              CurrentFrame_,
//											INTN              Frames_,
//											UINTN             FrameTime_,
//											EG_RECT           FilmPlace_,
//											EG_IMAGE        **Film_)
//						: ID(ID_), Title(Title_), TitleImage(TitleImage_),
//              /*InfoLineCount(InfoLineCount_), InfoLines(InfoLines_),*/ TimeoutSeconds(TimeoutSeconds_),
//						  TimeoutText(TimeoutText_), Theme(Theme_), AnimeRun(AnimeRun_),
//						  Once(Once_), LastDraw(LastDraw_), CurrentFrame(CurrentFrame_),
//						  Frames(Frames_), FrameTime(FrameTime_), FilmPlace(FilmPlace_),
//						  Film(Film_), mAction(ActionNone), mItemID(0), mPointer(NULL)
//						{
//							Entries.AddReference(entry, false);
//						};

//  REFIT_MENU_SCREEN(  UINTN             ID_,
//											  CONST   CHAR16      *Title_,
//											EG_IMAGE          *TitleImage_,
////											INTN              InfoLineCount_,
////											CONST CHAR16    **InfoLines_,
//											REFIT_ABSTRACT_MENU_ENTRY* entry1,
//											REFIT_ABSTRACT_MENU_ENTRY* entry2,
//											INTN              TimeoutSeconds_,
//											CONST CHAR16     *TimeoutText_,
//											CONST CHAR16     *Theme_,
//											BOOLEAN           AnimeRun_,
//											BOOLEAN           Once_,
//											UINT64            LastDraw_,
//											INTN              CurrentFrame_,
//											INTN              Frames_,
//											UINTN             FrameTime_,
//											EG_RECT           FilmPlace_,
//											EG_IMAGE        **Film_)
//						: ID(ID_), Title(Title_), TitleImage(TitleImage_),
//              /*InfoLineCount(InfoLineCount_), InfoLines(InfoLines_),*/ TimeoutSeconds(TimeoutSeconds_),
//						  TimeoutText(TimeoutText_), Theme(Theme_), AnimeRun(AnimeRun_),
//						  Once(Once_), LastDraw(LastDraw_), CurrentFrame(CurrentFrame_),
//						  Frames(Frames_), FrameTime(FrameTime_), FilmPlace(FilmPlace_),
//						  Film(Film_), mAction(ActionNone), mItemID(0), mPointer(NULL)
//						{
//							Entries.AddReference(entry1, false);
//							Entries.AddReference(entry2, false);
//						};
  //Scroll functions
  VOID InitScroll(IN INTN ItemCount, IN UINTN MaxCount,
                  IN UINTN VisibleSpace, IN INTN Selected);
  VOID UpdateScroll(IN UINTN Movement);
  VOID HidePointer();
  EFI_STATUS MouseBirth();
  VOID KillMouse();
  VOID AddMenuItem_(REFIT_MENU_ITEM_IEM_ABSTRACT* InputBootArgs, INTN Inx, CONST CHAR8 *Title, BOOLEAN Cursor);
  VOID AddMenuInfo(CONST CHAR16 *Line);
  VOID AddMenuInfoLine(IN CONST CHAR16 *InfoLine);
  VOID AddMenuEntry(IN REFIT_MENU_ENTRY *Entry, bool freeIt);
  VOID AddMenuItemSwitch(INTN Inx, CONST CHAR8 *Title, BOOLEAN Cursor);
  VOID AddMenuCheck(CONST CHAR8 *Text, UINTN Bit, INTN ItemNum);
  VOID AddMenuItemInput(INTN Inx, CONST CHAR8 *Title, BOOLEAN Cursor);
  VOID FreeMenu();
  INTN FindMenuShortcutEntry(IN CHAR16 Shortcut);
  UINTN RunGenericMenu(IN MENU_STYLE_FUNC StyleFunc, IN OUT INTN *DefaultEntryIndex, OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);
  UINTN RunMenu(OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);
  UINTN RunMainMenu(IN INTN DefaultSelection, OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);
  UINTN InputDialog(IN MENU_STYLE_FUNC StyleFunc);

  VOID DrawMainMenuLabel(IN CONST CHAR16 *Text, IN INTN XPos, IN INTN YPos);
  VOID CountItems();
  VOID InitAnime();
  BOOLEAN GetAnime();
  VOID UpdateAnime();


  //Style functions
  virtual VOID MainMenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText);
  virtual VOID MainMenuVerticalStyle(IN UINTN Function, IN CONST CHAR16 *ParamText);
  virtual VOID GraphicsMenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText);
  virtual VOID TextMenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText);

  ~REFIT_MENU_SCREEN() {};
};

#endif
/*
 
 EOF */
