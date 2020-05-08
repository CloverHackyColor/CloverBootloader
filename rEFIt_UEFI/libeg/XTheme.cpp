/*
 * a class to keep definitions for all theme settings
 */

extern "C" {
#include <Protocol/GraphicsOutput.h>
}

#include "libegint.h"
#include "../refit/screen.h"
#include "../refit/lib.h"

#include "XTheme.h"

#ifndef DEBUG_ALL
#define DEBUG_XTHEME 1
#else
#define DEBUG_XTHEME DEBUG_ALL
#endif

#if DEBUG_XTHEME == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_XTHEME, __VA_ARGS__)
#endif

CONST CHAR8* IconsNames[] = {
  "func_about",
  "func_options",
  "func_clover",
  "func_secureboot",
  "func_secureboot_config",
  "func_reset",
  "func_shutdown",
  "func_help",
  "tool_shell", //8
  "tool_part",
  "tool_rescue",
  "pointer",//11
  "vol_internal",
  "vol_external",
  "vol_optical",
  "vol_firewire",
  "vol_clover" ,
  "vol_internal_hfs" , //17
  "vol_internal_apfs",
  "vol_internal_ntfs",
  "vol_internal_ext3" ,
  "vol_recovery",//21
// not used? will be skipped while theme parsing
  "logo",
  "selection_small",
  "selection_big",  //BUILTIN_SELECTION_BIG=24 we keep this numeration
  //os icons
   "os_mac",  //0 + 25
   "os_tiger",
   "os_leo",
   "os_snow",
   "os_lion",
   "os_cougar",
   "os_mav",
   "os_yos",
   "os_cap", //33
   "os_sierra",
   "os_hsierra",
   "os_moja",  //36
   "os_cata",  //37  //there is no reserve for 10.16, next oses should be added to the end of the list
   "os_linux", //13 + 25 = 38
   "os_ubuntu",
   "os_suse",
   "os_freebsd", //16+25 = 41
   "os_freedos",
   "os_win",
   "os_vista",
   "radio_button", //20+25 = 45
   "radio_button_selected",
   "checkbox",  //22+25 = 47
   "checkbox_checked",
   "scrollbar_background", //24 - present here for SVG theme but should be done more common way
   "scrollbar_holder",
   "os_unknown", //51 == ICON_OTHER_OS
   "os_clover",  //52 == ICON_CLOVER
   //other oses will be added below
  ""
};
const INTN IconsNamesSize = sizeof(IconsNames) / sizeof(IconsNames[0]);

//icons class
//if ImageNight is not set then Image should be used
#define DEC_BUILTIN_ICON(id, ico) { \
Image.FromPNG(ACCESS_EMB_DATA(ico), ACCESS_EMB_SIZE(ico)); \
}

#define DEC_BUILTIN_ICON2(id, ico, dark) { \
Image.FromPNG(ACCESS_EMB_DATA(ico), ACCESS_EMB_SIZE(ico)); \
ImageNight.FromPNG(ACCESS_EMB_DATA(dark), ACCESS_EMB_SIZE(dark)); \
}

Icon::Icon(INTN Index, bool TakeEmbedded) : Image(), ImageNight()
{
  Id = Index;
  Name.setEmpty();
  Native = false;
  if (Index >= BUILTIN_ICON_FUNC_ABOUT && Index < IconsNamesSize) { //full table
    Name.takeValueFrom(IconsNames[Index]);
  }
  if (TakeEmbedded) {
    GetEmbedded();
  }
}

void Icon::GetEmbedded()
{
  switch (Id) {
    case BUILTIN_ICON_FUNC_ABOUT:
      DEC_BUILTIN_ICON2(BUILTIN_ICON_FUNC_ABOUT, emb_func_about, emb_dark_func_about)
      break;
    case BUILTIN_ICON_FUNC_OPTIONS:
      DEC_BUILTIN_ICON2(BUILTIN_ICON_FUNC_OPTIONS, emb_func_options, emb_dark_func_options)
      break;
    case BUILTIN_ICON_FUNC_CLOVER:
      DEC_BUILTIN_ICON2(BUILTIN_ICON_FUNC_CLOVER, emb_func_clover, emb_dark_func_clover)
      break;
    case BUILTIN_ICON_FUNC_SECURE_BOOT:
      DEC_BUILTIN_ICON2(BUILTIN_ICON_FUNC_SECURE_BOOT, emb_func_secureboot, emb_dark_func_secureboot)
      break;
    case BUILTIN_ICON_FUNC_SECURE_BOOT_CONFIG:
      DEC_BUILTIN_ICON2(BUILTIN_ICON_FUNC_SECURE_BOOT_CONFIG, emb_func_secureboot_config, emb_dark_func_secureboot_config)
      break;
    case BUILTIN_ICON_FUNC_RESET:
      DEC_BUILTIN_ICON2(BUILTIN_ICON_FUNC_RESET, emb_func_reset, emb_dark_func_reset)
      break;
    case BUILTIN_ICON_FUNC_EXIT:
      DEC_BUILTIN_ICON2(BUILTIN_ICON_FUNC_EXIT, emb_func_exit, emb_dark_func_exit)
      break;
    case BUILTIN_ICON_FUNC_HELP:
      DEC_BUILTIN_ICON2(BUILTIN_ICON_FUNC_HELP, emb_func_help, emb_dark_func_help)
      break;
    case BUILTIN_ICON_TOOL_SHELL:
      DEC_BUILTIN_ICON2(BUILTIN_ICON_TOOL_SHELL, emb_func_shell, emb_dark_func_shell)
      break;
    case BUILTIN_ICON_BANNER:
      DEC_BUILTIN_ICON2(BUILTIN_ICON_BANNER, emb_logo, emb_dark_logo)
      break;
    case BUILTIN_SELECTION_SMALL:
      DEC_BUILTIN_ICON2(BUILTIN_SELECTION_SMALL, emb_selection_small, emb_dark_selection_small)
      break;
    case BUILTIN_SELECTION_BIG:
      DEC_BUILTIN_ICON2(BUILTIN_SELECTION_BIG, emb_selection_big, emb_dark_selection_big)
      break;
      //next icons have no dark image
    case BUILTIN_ICON_POINTER:
      DEC_BUILTIN_ICON(BUILTIN_ICON_POINTER, emb_pointer)
      break;
    case BUILTIN_ICON_VOL_INTERNAL:
      DEC_BUILTIN_ICON(BUILTIN_ICON_VOL_INTERNAL, emb_vol_internal)
      break;
    case BUILTIN_ICON_VOL_EXTERNAL:
      DEC_BUILTIN_ICON(BUILTIN_ICON_VOL_EXTERNAL, emb_vol_external)
      break;
    case BUILTIN_ICON_VOL_OPTICAL:
      DEC_BUILTIN_ICON(BUILTIN_ICON_VOL_OPTICAL, emb_vol_optical)
      break;
    case BUILTIN_ICON_VOL_BOOTER:
      DEC_BUILTIN_ICON(BUILTIN_ICON_VOL_BOOTER, emb_vol_internal_booter)
      break;
    case BUILTIN_ICON_VOL_INTERNAL_HFS:
      DEC_BUILTIN_ICON(BUILTIN_ICON_VOL_INTERNAL_HFS, emb_vol_internal_hfs)
      break;
    case BUILTIN_ICON_VOL_INTERNAL_APFS:
      DEC_BUILTIN_ICON(BUILTIN_ICON_VOL_INTERNAL_APFS, emb_vol_internal_apfs)
      break;
    case BUILTIN_ICON_VOL_INTERNAL_NTFS:
      DEC_BUILTIN_ICON(BUILTIN_ICON_VOL_INTERNAL_NTFS, emb_vol_internal_ntfs)
      break;
    case BUILTIN_ICON_VOL_INTERNAL_EXT3:
      DEC_BUILTIN_ICON(BUILTIN_ICON_VOL_INTERNAL_EXT3, emb_vol_internal_ext)
      break;
    case BUILTIN_ICON_VOL_INTERNAL_REC:
      DEC_BUILTIN_ICON(BUILTIN_ICON_VOL_INTERNAL_REC, emb_vol_internal_recovery)
      break;
    case BUILTIN_RADIO_BUTTON:
      DEC_BUILTIN_ICON(BUILTIN_RADIO_BUTTON, emb_radio_button)
      break;
    case BUILTIN_RADIO_BUTTON_SELECTED:
      DEC_BUILTIN_ICON(BUILTIN_RADIO_BUTTON_SELECTED, emb_radio_button_selected)
      break;
    case BUILTIN_CHECKBOX:
      DEC_BUILTIN_ICON(BUILTIN_CHECKBOX, emb_checkbox)
      break;
    case BUILTIN_CHECKBOX_CHECKED:
      DEC_BUILTIN_ICON(BUILTIN_CHECKBOX_CHECKED, emb_checkbox_checked)
      break;
    case BUILTIN_ICON_SELECTION:
      Name.takeValueFrom("selection_indicator");
      DEC_BUILTIN_ICON(BUILTIN_ICON_SELECTION, emb_selection_indicator)
      break;
    default:
      //     Image.setEmpty(); //done by ctor?
      break;
  }
  //something to do else?
}

Icon::~Icon() {}

//xtheme class
XTheme::XTheme() {
  Init();
}

XTheme::~XTheme() {
  //nothing todo?
}

void XTheme::Init()
{
//  DisableFlags = 0;             
  HideBadges = 0; 
  HideUIFlags = 0; 
//  TextOnly = FALSE; 
  Font = FONT_GRAY;      // FONT_TYPE   
  CharWidth = 9;  
  SelectionColor = 0x80808080;
  SelectionBackgroundPixel = { 0xef, 0xef, 0xef, 0xff };
  FontFileName.setEmpty();     
//  Theme.takeValueFrom("embedded");
  embedded = false;
  BannerFileName.setEmpty();    
  SelectionSmallFileName.setEmpty();  
  SelectionBigFileName.setEmpty();  
  SelectionIndicatorName.setEmpty(); 
  DefaultSelection.setEmpty();  
  BackgroundName.setEmpty();  
  BackgroundScale = imNone;     // SCALING 
  BackgroundSharp = 0;            
  BackgroundDark = FALSE;       //TODO should be set to true if Night theme
//  CustomIcons = FALSE;          //TODO don't know how to handle with SVG theme
  SelectionOnTop = FALSE;         
  BootCampStyle = FALSE; 
  BadgeOffsetX = 0xFFFF;  //default offset
  BadgeOffsetY = 0xFFFF;
  BadgeScale = 4;   // TODO now we have float scale = BadgeScale/16
  ThemeDesignWidth = 0xFFFF;
  ThemeDesignHeight = 0xFFFF;
  BannerPosX = 0xFFFF; // the value out of range [0,1000] means default
  BannerPosY = 0xFFFF;
  BannerEdgeHorizontal = 0;
  BannerEdgeVertical = 0;
  BannerNudgeX = 0;
  BannerNudgeY = 0;
  BanHeight = 0;
  VerticalLayout = FALSE;
  NonSelectedGrey = FALSE;    
  MainEntriesSize = 128;
  TileXSpace = 8;
  TileYSpace = 24;
//  IconFormat = ICON_FORMAT_DEF;
  Proportional = FALSE;
//  ShowOptimus = FALSE;
//  DarkEmbedded = FALSE;  //looks like redundant, we always check Night or Daylight
  TypeSVG = FALSE;
//  Codepage = 0xC0;           //this is for PNG theme
//  CodepageSize = 0xC0;           // INTN        CodepageSize; //extended latin
  Scale = 1.0f;
  CentreShift = 0.0f;
//  Daylight = true;
  LayoutHeight = 376;
  LayoutBannerOffset                    = 64; //default value if not set
  LayoutButtonOffset                    = 0; //default value if not set
  LayoutTextOffset                      = 0; //default value if not set
  LayoutAnimMoveForMenuX                = 0; //default value if not set

  row0TileSize = 144;
  row1TileSize = 64;

  FontWidth = 9;
  FontHeight = 18;
  TextHeight = 19;

  Cinema.setEmpty();
}

/*
 * what if the icon is not found or name is wrong?
 * probably it whould return Empty image
 * Image.isEmpty() == true
 */
//const XImage& XTheme::GetIcon(const char* Name)
//{
//  return GetIcon(XString().takeValueFrom(Name));
//}
//
//const XImage& XTheme::GetIcon(const CHAR16* Name)
//{
//  return GetIcon(XString().takeValueFrom(Name));
//}

static XImage NullIcon;
static XImage DummyIcon;

const XImage& XTheme::GetIcon(const XString8& Name)
{
  for (size_t i = 0; i < Icons.size(); i++)
  {
    if (Icons[i].Name == Name) //night icon has same name as daylight icon
    {
      return GetIcon(Icons[i].Id);
    }
  }
  return NullIcon; //if name is not found
}

bool XTheme::CheckNative(INTN Id)
{
  for (size_t i = 0; i < Icons.size(); i++)
  {
    if (Icons[i].Id == Id)
    {
      return Icons[i].Native;
    }
  }
  return false;
}

const XImage& XTheme::GetIcon(INTN Id)
{
  return GetIconAlt(Id, -1);
}

/*
 * Get Icon with this ID=id, for example VOL_INTERNAL_HFS
 * if not found then search for ID=Alt with Native attribute set, for example VOL_INTERNAL
 * if not found then check embedded with ID=Id
 * if not found then check embedded with ID=Alt
 */
const XImage& XTheme::GetIconAlt(INTN Id, INTN Alt) //if not found then take embedded
{
  INTN IdFound = -1;
  INTN AltFound = -1;

  for (size_t i = 0; i < Icons.size() && (IdFound < 0 || (Alt >= 0 && AltFound < 0)); i++) {
    if (Icons[i].Id == Id) {
      IdFound = i;
    }
    if (Icons[i].Id == Alt) {
      AltFound = i;
    }
  }

  // if icon is empty, try to fill it with alternative
  if (IdFound >= 0 && Icons[IdFound].Image.isEmpty()) {
    // check for native ID=Alt, if Alt was specified
    if (Alt >= 0 && AltFound >= 0 && Icons[AltFound].Native && !Icons[AltFound].Image.isEmpty()) {
      // using Alt icon
      Icons[IdFound].Image = Icons[AltFound].Image;
      Icons[IdFound].ImageNight = Icons[AltFound].ImageNight;
    } else {
      // check for embedded with ID=Id
      Icon *NewIcon = new Icon(Id, true);
      if (NewIcon->Image.isEmpty()) {
        // check for embedded with ID=Alt
        NewIcon = new Icon(Alt, true);
      }
      if (!NewIcon->Image.isEmpty()) {
        // using Embedded icon
        Icons[IdFound].Image = NewIcon->Image;
        Icons[IdFound].ImageNight = NewIcon->ImageNight;
      }
    }
  }

  if (IdFound >= 0 && !Icons[IdFound].Image.isEmpty()) {
    // icon not empty, return it
    if (!Daylight && !Icons[IdFound].ImageNight.isEmpty()) {
      DBG("got night icon %lld name{%s}\n", Id, IconsNames[IdFound]);
      return Icons[IdFound].ImageNight;
    }
    //if daylight or night icon absent
    DBG("got day icon %lld name{%s}\n", Id, IconsNames[IdFound]);
    return Icons[IdFound].Image;
  }
  return NullIcon; //such Id is not found in the database
}

const XImage& XTheme::LoadOSIcon(const CHAR16* OSIconName)
{
  return LoadOSIcon(XString8().takeValueFrom(OSIconName));
}

const XImage& XTheme::LoadOSIcon(const XString8& Full)
{
  // input value can be L"win", L"ubuntu,linux", L"moja,mac" set by GetOSIconName (OSVersion)
  XString8 First;
  XString8 Second;
  XString8 Third;
  const XImage *ReturnImage;
  UINTN Comma = Full.indexOf(',');
  UINTN Size = Full.length();
  DBG("IconName=%s comma=%lld size=%lld\n", Full.c_str(), Comma, Size);
  if (Comma != MAX_XSIZE) {  //Comma
    First = "os_"_XS8 + Full.subString(0, Comma);
    ReturnImage = &GetIcon(First);
    DBG("  first=%s\n", First.c_str());
    if (!ReturnImage->isEmpty()) return *ReturnImage;
    //else search second name
    Second = "os_"_XS8 + Full.subString(Comma + 1, Size - Comma - 1);
    //moreover names can be triple L"chrome,grub,linux"
    UINTN SecondComma = Second.indexOf(',');
    if (Comma == MAX_XSIZE) {
      ReturnImage = &GetIcon(Second);
      if (!ReturnImage->isEmpty()) return *ReturnImage;
    } else {
      First = Second.subString(0, SecondComma);
      ReturnImage = &GetIcon(First);
      if (!ReturnImage->isEmpty()) return *ReturnImage;
      Third = "os_"_XS8 + Second.subString(SecondComma + 1, Size - SecondComma - 1);
      ReturnImage = &GetIcon(Third);
      if (!ReturnImage->isEmpty()) return *ReturnImage;
    }
    DBG("  Second=%s\n", Second.c_str());
    if (!ReturnImage->isEmpty()) return *ReturnImage;
  } else {
    ReturnImage = &GetIcon("os_"_XS8 + Full);
    DBG("  Full=%s\n", Full.c_str());
    if (!ReturnImage->isEmpty()) return *ReturnImage;
  }
  // else something
  if (DummyIcon.isEmpty()) //initialize once per session
    DummyIcon.DummyImage(MainEntriesSize);
  return DummyIcon;
}
//
//void XTheme::AddIcon(Icon& NewIcon)
//{
//  Icons.AddCopy(NewIcon);
//}


void XTheme::FillByEmbedded()
{
  embedded = true;
  Theme.takeValueFrom("embedded");
  SelectionColor = 0xA0A0A080;
  SelectionBackgroundPixel = { 0xa0, 0xa0, 0xa0, 0x80 };

  Icons.Empty();
  for (INTN i = 0; i < BUILTIN_ICON_COUNT; ++i) { //this is embedded icon count
    Icon* NewIcon = new Icon(i, true);
    Icons.AddReference(NewIcon, true);
  }

  BigBack.setEmpty();
  Background = XImage(UGAWidth, UGAHeight);

  if (Daylight) {
    Banner.FromPNG(ACCESS_EMB_DATA(emb_logo), emb_logo_size);
  } else {
    Banner.FromPNG(ACCESS_EMB_DATA(emb_dark_logo), emb_dark_logo_size);
  }
  
  //and buttons
  Buttons[0].FromPNG(ACCESS_EMB_DATA(emb_radio_button), ACCESS_EMB_SIZE(emb_radio_button));
  Buttons[1].FromPNG(ACCESS_EMB_DATA(emb_radio_button_selected), ACCESS_EMB_SIZE(emb_radio_button_selected));
  Buttons[2].FromPNG(ACCESS_EMB_DATA(emb_checkbox), ACCESS_EMB_SIZE(emb_checkbox));
  Buttons[3].FromPNG(ACCESS_EMB_DATA(emb_checkbox_checked), ACCESS_EMB_SIZE(emb_checkbox_checked));

  if (Daylight) {
    SelectionImages[0].FromPNG(ACCESS_EMB_DATA(emb_selection_big), ACCESS_EMB_SIZE(emb_selection_big));
    SelectionImages[2].FromPNG(ACCESS_EMB_DATA(emb_selection_small), ACCESS_EMB_SIZE(emb_selection_small));
  } else {
    SelectionImages[0].FromPNG(ACCESS_EMB_DATA(emb_dark_selection_big), ACCESS_EMB_SIZE(emb_dark_selection_big));
    SelectionImages[2].FromPNG(ACCESS_EMB_DATA(emb_dark_selection_small), ACCESS_EMB_SIZE(emb_dark_selection_small));
  }

  SelectionImages[4].FromPNG(ACCESS_EMB_DATA(emb_selection_indicator), ACCESS_EMB_SIZE(emb_selection_indicator));
}

void XTheme::ClearScreen() //and restore background and banner
{
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL FirstBannerPixel = MenuBackgroundPixel;
  if (BanHeight < 2) {
    BanHeight = ((UGAHeight - (int)(LayoutHeight * Scale)) >> 1);
  }
//  egClearScreen(&MenuBackgroundPixel); //not needed
  if (!(HideUIFlags & HIDEUI_FLAG_BANNER)) {
    //Banner image prepared before
    if (!Banner.isEmpty()) {
      FirstBannerPixel = Banner.GetPixel(0,0);

      BannerPlace.Width = Banner.GetWidth();
      BannerPlace.Height = (BanHeight >= Banner.GetHeight()) ? Banner.GetHeight() : BanHeight;
      BannerPlace.XPos = BannerPosX;
      BannerPlace.YPos = BannerPosY;
      if (!TypeSVG) {
        // Check if new style placement value was used for banner in theme.plist
        
        if ((BannerPosX >=0 && BannerPosX <=1000) && (BannerPosY >=0 && BannerPosY <=1000)) {
          // Check if screen size being used is different from theme origination size.
          // If yes, then recalculate the placement % value.
          // This is necessary because screen can be a different size, but banner is not scaled.
          BannerPlace.XPos = HybridRepositioning(BannerEdgeHorizontal, BannerPosX, BannerPlace.Width,  UGAWidth,  ThemeDesignWidth );
          BannerPlace.YPos = HybridRepositioning(BannerEdgeVertical, BannerPosY, BannerPlace.Height, UGAHeight, ThemeDesignHeight);
          // Check if banner is required to be nudged.
          BannerPlace.XPos = CalculateNudgePosition(BannerPlace.XPos, BannerNudgeX, Banner.GetWidth(),  UGAWidth);
          BannerPlace.YPos = CalculateNudgePosition(BannerPlace.YPos, BannerNudgeY, Banner.GetHeight(), UGAHeight);
          //         DBG("banner position new style\n");
        } else {
          // Use rEFIt default (no placement values speicifed)
          BannerPlace.XPos = (UGAWidth  >= Banner.GetWidth() ) ? (UGAWidth  - Banner.GetWidth() ) >> 1 : 0;
          BannerPlace.YPos = (BanHeight >= Banner.GetHeight()) ? (BanHeight - Banner.GetHeight())      : 0;
          //        DBG("banner position old style\n");
        }
      }
    }
  }
  DBG("BannerPlace at Clear Screen [%lld,%lld]\n",  BannerPlace.XPos, BannerPlace.YPos);
  //Then prepare Background from BigBack
  if (Background.GetWidth() != UGAWidth || Background.GetHeight() != UGAHeight) { // should we type UGAWidth and UGAHeight as UINTN to avoid cast ?
    // Resolution changed or empty background
    Background = XImage(UGAWidth, UGAHeight);
  }
// now we are sure Background has UGA sizes
  float BigScale;
  float BigScaleY;
  if (!BigBack.isEmpty()) {
    switch (BackgroundScale) {
    case imScale:
//    DBG("back copy scaled\n");
//    Background.setSizeInPixels(UGAWidth, UGAHeight); //anyway set
      BigScale = (float)UGAWidth/BigBack.GetWidth();
      BigScaleY = (float)UGAHeight/BigBack.GetHeight();
      Background.CopyScaled(BigBack, MAX(BigScale, BigScaleY));
      break;
    case imCrop:
    {
      INTN x = UGAWidth - BigBack.GetWidth();
      INTN x1, x2, y1, y2;
      if (x >= 0) {
        x1 = x >> 1;
        x2 = 0;
        x = BigBack.GetWidth();
      } else {
        x1 = 0;
        x2 = (-x) >> 1;
        x = UGAWidth;
      }
      INTN y = UGAHeight - BigBack.GetHeight();
      if (y >= 0) {
        y1 = y >> 1;
        y2 = 0;
        y = BigBack.GetHeight();
      } else {
        y1 = 0;
        y2 = (-y) >> 1;
        y = UGAHeight;
      }
      const EG_RECT BackRect = EG_RECT(x1, y1, x, y);
      const EG_RECT BigRect = EG_RECT(x2, y2, x, y);
      //the function can be in XImage class
//      egRawCopy((EG_PIXEL*)Background.GetPixelPtr(x1, y1),
//                (EG_PIXEL*)BigBack.GetPixelPtr(x2, y2),
//                x, y, Background.GetWidth(), BigBack.GetWidth());
//      DBG("crop to x,y: %lld, %lld\n", x, y);
      Background.CopyRect(BigBack, BackRect, BigRect);
//      DBG("back copy cropped\n");
      break;
    }
    case imTile:
    {
      INTN x = (BigBack.GetWidth() * ((UGAWidth - 1) / BigBack.GetWidth() + 1) - UGAWidth) >> 1;
      INTN y = (BigBack.GetHeight() * ((UGAHeight - 1) / BigBack.GetHeight() + 1) - UGAHeight) >> 1;
      EFI_GRAPHICS_OUTPUT_BLT_PIXEL* p1 = Background.GetPixelPtr(0, 0);
      for (INTN j = 0; j < UGAHeight; j++) {
        for (INTN i = 0; i < UGAWidth; i++) {
          *p1++ = BigBack.GetPixel((i + x) % BigBack.GetWidth(), (j + y) % BigBack.GetHeight());
        }
      }
//      DBG("back copy tiled\n");
      break;
    }
    case imNone:
    default:
      // already scaled
      Background = BigBack;
//        DBG("back copy equal\n");
      break;
    }
  } else {
    // no background loaded, fill by default
    if (!embedded) {
      BlueBackgroundPixel = FirstBannerPixel;
    } else if (Daylight) {
      // embedded light
      BlueBackgroundPixel = StdBackgroundPixel;
    } else {
      // embedded dark
      BlueBackgroundPixel = DarkEmbeddedBackgroundPixel;
    }
    Background.Fill(BlueBackgroundPixel); //blue opaque. May be better to set black opaque?
  }
  //join Banner and Background for menu drawing
  if (!Banner.isEmpty()) {
    Background.Compose(BannerPlace.XPos, BannerPlace.YPos, Banner, true);
  }
  Background.DrawWithoutCompose(0, 0, UGAWidth, UGAHeight);
}

#if 0
void XTheme::InitSelection() //for PNG theme
{
  EFI_STATUS Status;
  if (!AllowGraphicsMode)
    return;
  //used to fill TextBuffer if selected
  SelectionBackgroundPixel.Red      = (SelectionColor >> 24) & 0xFF;
  SelectionBackgroundPixel.Green    = (SelectionColor >> 16) & 0xFF;
  SelectionBackgroundPixel.Blue     = (SelectionColor >> 8) & 0xFF;
  SelectionBackgroundPixel.Reserved = (SelectionColor >> 0) & 0xFF;
  
  if (!SelectionImages[0].isEmpty()) { //already presents
    return;
  }

  if (TypeSVG) {
    SelectionImages[2] = GetIcon(BUILTIN_SELECTION_SMALL);
    SelectionImages[0] = GetIcon(BUILTIN_SELECTION_BIG);
    if (SelectionImages[0].isEmpty()) {
      SelectionImages[0] = SelectionImages[2]; //use same selection if OnTop for example
    }
  } else {
    // load small selection image
    if (SelectionImages[2].isEmpty()){
      SelectionImages[2].LoadXImage(ThemeDir, SelectionSmallFileName);
    }
    if (SelectionImages[2].isEmpty()){
      //    SelectionImages[2] = BuiltinIcon(BUILTIN_SELECTION_SMALL);
      //    SelectionImages[2]->HasAlpha = FALSE; // support transparensy for selection icons
      if (Daylight) {
        SelectionImages[2].FromPNG(ACCESS_EMB_DATA(emb_selection_small), ACCESS_EMB_SIZE(emb_selection_small));
      } else {
        SelectionImages[2].FromPNG(ACCESS_EMB_DATA(emb_dark_selection_small), ACCESS_EMB_SIZE(emb_dark_selection_small));
      }
      //    CopyMem(&BlueBackgroundPixel, &StdBackgroundPixel, sizeof(EG_PIXEL)); //why???
    }
    //cut or extend the image by Compose
    /*  SelectionImages[2] = egEnsureImageSize(SelectionImages[2],
     row1TileSize, row1TileSize, &MenuBackgroundPixel);
     if (SelectionImages[2] == NULL) {
     return;
     } */

    // load big selection image
    if (SelectionImages[0].isEmpty()) {
      SelectionImages[0].LoadXImage(ThemeDir, SelectionBigFileName);
      //   SelectionImages[0].EnsureImageSize(row0TileSize, row0TileSize, &MenuBackgroundPixel);
    }
    if (SelectionImages[0].isEmpty()) {
      // calculate big selection image from small one
      //    SelectionImages[0] = BuiltinIcon(BUILTIN_SELECTION_BIG);
      if (Daylight) {
        SelectionImages[0].FromPNG(ACCESS_EMB_DATA(emb_selection_big), ACCESS_EMB_SIZE(emb_selection_big));
      } else {
        SelectionImages[0].FromPNG(ACCESS_EMB_DATA(emb_dark_selection_big), ACCESS_EMB_SIZE(emb_dark_selection_big));
      }
      //    SelectionImages[0]->HasAlpha = FALSE; // support transparensy for selection icons
      //CopyMem(&BlueBackgroundPixel, &StdBackgroundPixel, sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
      BlueBackgroundPixel = StdBackgroundPixel;
      if (SelectionImages[0].isEmpty()) {
        SelectionImages[2].setEmpty();
        return;
      }
      //    if (SelectionOnTop) {
      //      SelectionImages[0]->HasAlpha = TRUE; // TODO ?
      //      SelectionImages[2]->HasAlpha = TRUE;
      //    }
    }
  }
  // BootCampStyle indicator image
  if (BootCampStyle) {
    // load indicator selection image
    if (!TypeSVG && SelectionImages[4].isEmpty()) {
      SelectionImages[4].LoadXImage(ThemeDir, SelectionIndicatorName);
    }
    if (SelectionImages[4].isEmpty()) {
      SelectionImages[4].FromPNG(ACCESS_EMB_DATA(emb_selection_indicator), ACCESS_EMB_SIZE(emb_selection_indicator));
    }
    INTN ScaledIndicatorSize = (INTN)(INDICATOR_SIZE * Scale);
    SelectionImages[4].EnsureImageSize(ScaledIndicatorSize, ScaledIndicatorSize, MenuBackgroundPixel);
    if (SelectionImages[4].isEmpty()) {
//      SelectionImages[4] = egCreateFilledImage(ScaledIndicatorSize, ScaledIndicatorSize,
//                                               TRUE, &StdBackgroundPixel);
      SelectionImages[4] = XImage(ScaledIndicatorSize, ScaledIndicatorSize);
      SelectionImages[4].Fill(StdBackgroundPixel);
    }
    SelectionImages[5] = XImage(ScaledIndicatorSize, ScaledIndicatorSize);
    SelectionImages[5].Fill(MenuBackgroundPixel);
  }
  
  /*
   Button & radio, or any other next icons with builtin icon as fallback should synced to:
   - BUILTIN_ICON_* in lib.h
   - BuiltinIconTable in icns.c
   - Data in egemb_icons.h / scroll_images.h
   */
  
  // Radio buttons
  //it was a nonsense egLoadImage is just inluded into egLoadIcon.
  // will be corrected with XTheme support
  //the procedure loadIcon should also check embedded icons
  //DECLARE_EMB_EXTERN_WITH_SIZE(emb_radio_button_selected)
  //DECLARE_EMB_EXTERN_WITH_SIZE(emb_radio_button)
  //DECLARE_EMB_EXTERN_WITH_SIZE(emb_checkbox)
  //DECLARE_EMB_EXTERN_WITH_SIZE(emb_checkbox_checked)
  //DECLARE_EMB_EXTERN_WITH_SIZE(emb_dark_font_data)

  if (!TypeSVG) { //SVG theme already parsed buttons
    Status = Buttons[0].LoadXImage(ThemeDir, "radio_button");
    if (EFI_ERROR(Status)) {
      Buttons[0].FromPNG(ACCESS_EMB_DATA(emb_radio_button), ACCESS_EMB_SIZE(emb_radio_button));
    }
    Status = Buttons[1].LoadXImage(ThemeDir, "radio_button_selected");
    if (EFI_ERROR(Status)) {
      Buttons[1].FromPNG(ACCESS_EMB_DATA(emb_radio_button_selected), ACCESS_EMB_SIZE(emb_radio_button_selected));
    }
    Status = Buttons[2].LoadXImage(ThemeDir, "checkbox");
    if (EFI_ERROR(Status)) {
      Buttons[2].FromPNG(ACCESS_EMB_DATA(emb_checkbox), ACCESS_EMB_SIZE(emb_checkbox));
    }
    Status = Buttons[3].LoadXImage(ThemeDir, "checkbox_checked");
    if (EFI_ERROR(Status)) {
      Buttons[3].FromPNG(ACCESS_EMB_DATA(emb_checkbox_checked), ACCESS_EMB_SIZE(emb_checkbox_checked));
    }
  } else {
    //SVG theme already parsed all icons
    Buttons[0] = GetIcon("radio_button"_XS8);
    Buttons[1] = GetIcon("radio_button_selected"_XS8);
    Buttons[2] = GetIcon("checkbox"_XS8);
    Buttons[3] = GetIcon("checkbox_checked"_XS8);
  }

  // non-selected background images

  EFI_GRAPHICS_OUTPUT_BLT_PIXEL BackgroundPixel = { 0xbf, 0xbf, 0xbf, 0xff };
  if (TypeSVG || !SelectionBigFileName.isEmpty()) {
    BackgroundPixel = { 0x00, 0x00, 0x00, 0x00 };
  } else if (DarkEmbedded) {
    BackgroundPixel = { 0x33, 0x33, 0x33, 0x00 }; //nonsense. Will be a sense if semi-transparent
  } else { //for example embedded daylight
    BackgroundPixel = { 0xbf, 0xbf, 0xbf, 0xff };
  }

  SelectionImages[1] = XImage(row0TileSize, row0TileSize);
  SelectionImages[1].Fill(BackgroundPixel);
  SelectionImages[3] = XImage(row1TileSize, row1TileSize);
  SelectionImages[3].Fill(BackgroundPixel);

}
#endif
//use this only for PNG theme
void XTheme::FillByDir() //assume ThemeDir is defined by InitTheme() procedure
{
  EFI_STATUS Status;
  Icons.Empty();
  for (INTN i = 0; i < IconsNamesSize; ++i) { //scan full table
    Status = EFI_NOT_FOUND;
    Icon* NewIcon = new Icon(i); //initialize without embedded
    switch (i) {
      case BUILTIN_SELECTION_SMALL:
        Status = NewIcon->Image.LoadXImage(ThemeDir, SelectionSmallFileName);
        break;
      case BUILTIN_SELECTION_BIG:
        Status = NewIcon->Image.LoadXImage(ThemeDir, SelectionBigFileName);
        break;
    }
    if (EFI_ERROR(Status)) {
      Status = NewIcon->Image.LoadXImage(ThemeDir, IconsNames[i]);
    }
    NewIcon->Native = !EFI_ERROR(Status);
    if (!EFI_ERROR(Status)) {
      NewIcon->ImageNight.LoadXImage(ThemeDir, SWPrintf("%s_night", IconsNames[i]));
    }
    Icons.AddReference(NewIcon, true);
    if (EFI_ERROR(Status)) {
      if (i >= BUILTIN_ICON_VOL_INTERNAL_HFS && i <= BUILTIN_ICON_VOL_INTERNAL_REC) {
        // call to GetIconAlt will get alternate/embedded into Icon if missing
        GetIconAlt(i, BUILTIN_ICON_VOL_INTERNAL);
      } else if (i == BUILTIN_SELECTION_BIG) {
        GetIconAlt(i, BUILTIN_SELECTION_SMALL);
      }
    }
  }
  if (BootCampStyle) {
    Icon *NewIcon = new Icon(BUILTIN_ICON_SELECTION);
    // load indicator selection image
    Status = NewIcon->Image.LoadXImage(ThemeDir, SelectionIndicatorName);
    if (EFI_ERROR(Status)) {
      Status = NewIcon->Image.LoadXImage(ThemeDir, "selection_indicator");
    }
    Icons.AddReference(NewIcon, true);
  }

  SelectionBackgroundPixel.Red      = (SelectionColor >> 24) & 0xFF;
  SelectionBackgroundPixel.Green    = (SelectionColor >> 16) & 0xFF;
  SelectionBackgroundPixel.Blue     = (SelectionColor >> 8) & 0xFF;
  SelectionBackgroundPixel.Reserved = (SelectionColor >> 0) & 0xFF;

  SelectionImages[2] = GetIcon(BUILTIN_SELECTION_SMALL);
  SelectionImages[0] = GetIcon(BUILTIN_SELECTION_BIG);
  if (BootCampStyle) {
    SelectionImages[4] = GetIcon(BUILTIN_ICON_SELECTION);
  }

  //and buttons
  Buttons[0] = GetIcon(BUILTIN_RADIO_BUTTON);
  Buttons[1] = GetIcon(BUILTIN_RADIO_BUTTON_SELECTED);
  Buttons[2] = GetIcon(BUILTIN_CHECKBOX);
  Buttons[3] = GetIcon(BUILTIN_CHECKBOX_CHECKED);

  //load banner and background
  Banner.LoadXImage(ThemeDir, BannerFileName); 
  Status = BigBack.LoadXImage(ThemeDir, BackgroundName);
  if (EFI_ERROR(Status) && !Banner.isEmpty()) {
    //take first pixel from banner
    const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& firstPixel = Banner.GetPixel(0,0);
    BigBack.setSizeInPixels(UGAWidth, UGAHeight);
    BigBack.Fill(firstPixel);
  }
}


void XTheme::InitBar()
{
  if (!TypeSVG) {
    ScrollbarBackgroundImage.LoadXImage(ThemeDir, "scrollbar\\bar_fill");
    BarStartImage.LoadXImage(ThemeDir, "scrollbar\\bar_start");
    BarEndImage.LoadXImage(ThemeDir, "scrollbar\\bar_end");
    ScrollbarImage.LoadXImage(ThemeDir, "scrollbar\\scroll_fill");
    ScrollStartImage.LoadXImage(ThemeDir, "scrollbar\\scroll_start");
    ScrollEndImage.LoadXImage(ThemeDir, "scrollbar\\scroll_end");
    UpButtonImage.LoadXImage(ThemeDir, "scrollbar\\up_button");
    DownButtonImage.LoadXImage(ThemeDir, "scrollbar\\down_button");
  } else {
    ScrollbarBackgroundImage = GetIcon("scrollbar_background"_XS8);
    BarStartImage.setEmpty();
    BarEndImage.setEmpty();
    ScrollbarImage = GetIcon("scrollbar_holder"_XS8); //"_night" is already accounting
    ScrollStartImage = GetIcon("scrollbar_start"_XS8);
    ScrollEndImage = GetIcon("scrollbar_end"_XS8);
    UpButtonImage = GetIcon("scrollbar_up_button"_XS8);
    DownButtonImage = GetIcon("scrollbar_down_button"_XS8);
  }

  //some help with embedded scroll

  if (!TypeSVG) {
    // fill these from embedded only for non-svg
    // Question: why we don't want these for svg? (upbutton, downbutton, scrollstart, scrollend - also have hardcoded 0 height in REFIT_MENU_SCREEN.cpp)
    if (BarStartImage.isEmpty()) {
      BarStartImage.FromPNG(ACCESS_EMB_DATA(emb_scroll_bar_start), ACCESS_EMB_SIZE(emb_scroll_bar_start));
    }
    if (BarEndImage.isEmpty()) {
      BarEndImage.FromPNG(ACCESS_EMB_DATA(emb_scroll_bar_end), ACCESS_EMB_SIZE(emb_scroll_bar_end));
    }
    if (ScrollStartImage.isEmpty()) {
      ScrollStartImage.FromPNG(ACCESS_EMB_DATA(emb_scroll_scroll_start), ACCESS_EMB_SIZE(emb_scroll_scroll_start));
    }
    if (ScrollEndImage.isEmpty()) {
      ScrollEndImage.FromPNG(ACCESS_EMB_DATA(emb_scroll_scroll_end), ACCESS_EMB_SIZE(emb_scroll_scroll_end));
    }
    if (UpButtonImage.isEmpty()) {
      UpButtonImage.FromPNG(ACCESS_EMB_DATA(emb_scroll_up_button), ACCESS_EMB_SIZE(emb_scroll_up_button));
    }
   if (DownButtonImage.isEmpty()) {
      DownButtonImage.FromPNG(ACCESS_EMB_DATA(emb_scroll_down_button), ACCESS_EMB_SIZE(emb_scroll_down_button));
    }
  }

  // fill these from embedded for both svg and non-svg
  if (ScrollbarBackgroundImage.isEmpty()) {
    ScrollbarBackgroundImage.FromPNG(ACCESS_EMB_DATA(emb_scroll_bar_fill), ACCESS_EMB_SIZE(emb_scroll_bar_fill));
  }
  if (ScrollbarImage.isEmpty()) {
    ScrollbarImage.FromPNG(ACCESS_EMB_DATA(emb_scroll_scroll_fill), ACCESS_EMB_SIZE(emb_scroll_scroll_fill));
  }

}

//the purpose of the procedure is restore Background in rect
//XAlign is always centre, Color is the Backgrounf fill
//TODO replace by some existing procedure
VOID XTheme::FillRectAreaOfScreen(IN INTN XPos, IN INTN YPos, IN INTN Width, IN INTN Height)
{
  //  TmpBuffer.CopyScaled(Background, 1.f);
  INTN X = XPos - (Width >> 1);  //X_IS_CENTRE
  if (X < 0) {
    X = 0;
  }
  if (X + Width > UGAWidth) {
    Width = (X > UGAWidth) ? 0 : (UGAWidth - X);
  }
  if (YPos + Height > UGAHeight) {
    Height = (YPos > UGAHeight) ? 0 : (UGAHeight - YPos);
  }

  XImage TmpBuffer(Width, Height);
  TmpBuffer.CopyRect(Background, X, YPos); //a part of BackGround image
  TmpBuffer.DrawWithoutCompose(X, YPos);
  //  TmpBuffer.Draw(X, YPos, 0, true);
}




