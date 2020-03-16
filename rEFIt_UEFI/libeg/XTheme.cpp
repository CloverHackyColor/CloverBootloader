/*
 * a class to keep definitions for all theme settings
 */
#include "libegint.h"
#include "XTheme.h"

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
  "vol_internal_hfs" ,
  "vol_internal_apfs",
  "vol_internal_ntfs",
  "vol_internal_ext3" ,
  "vol_recovery",//21
// not used?
  "logo",
  "selection_small",
  "selection_big",
  //os icons
   "os_mac",  //0 + 25
   "os_tiger",
   "os_leo",
   "os_snow",
   "os_lion",
   "os_cougar",
   "os_mav",
   "os_yos",
   "os_cap",
   "os_sierra",
   "os_hsierra",
   "os_moja",  //11
   "os_cata",  //12
   "os_linux",
   "os_ubuntu",
   "os_suse",
   "os_freebsd", //16
   "os_freedos",
   "os_win",
   "os_vista",
   "radio_button",
   "radio_button_selected",
   "checkbox",
   "checkbox_checked",
   "scrollbar_background", //24
   "scrollbar_holder"
};

Icon::Icon(INTN Index) : Image(0), ImageNight(0)
{
  Id = Index;
  Name = XString(IconsNames[Index]);
}

Icon::~Icon() {}

XTheme::XTheme() {
  Init();
}

XTheme::~XTheme() {
  //nothing todo?
}

void XTheme::Init()
{
  DisableFlags = 0;             
  HideBadges = 0; 
  HideUIFlags = 0; 
  TextOnly = FALSE; 
  Font = FONT_GRAY;      // FONT_TYPE   
  CharWidth = 9;  
  SelectionColor = 0xFFFFFF80; 
  FontFileName.setEmpty();     
  Theme.takeValueFrom("embedded");  
  BannerFileName.setEmpty();    
  SelectionSmallFileName.setEmpty();  
  SelectionBigFileName.setEmpty();  
  SelectionIndicatorName.setEmpty(); 
  DefaultSelection.setEmpty();  
  BackgroundName.setEmpty();  
  BackgroundScale = imNone;     // SCALING 
  BackgroundSharp = 0;            
  BackgroundDark = FALSE;       //TODO should be set to true if Night theme
  CustomIcons = FALSE;          //TODO don't know how to handle with SVG theme
  SelectionOnTop = FALSE;         
  BootCampStyle = FALSE; 
  BadgeOffsetX = 0;
  BadgeOffsetY = 0;
  BadgeScale = 4;   // TODO now we have float scale = BadgeScale/16
  ThemeDesignWidth = 0xFFFF;
  ThemeDesignHeight = 0xFFFF;
  BannerPosX = 0xFFFF;
  BannerPosY = 0xFFFF;
  BannerEdgeHorizontal = 0;
  BannerEdgeVertical = 0;
  BannerNudgeX = 0;
  BannerNudgeY = 0;
  VerticalLayout = FALSE;
  NonSelectedGrey = FALSE;    //TODO what about SVG?
  MainEntriesSize = 128;
  TileXSpace = 8;
  TileYSpace = 24;
//  IconFormat = ICON_FORMAT_DEF;
  Proportional = FALSE;
  ShowOptimus = FALSE;
  DarkEmbedded = FALSE;  //looks like redundant, we always check Night or Daylight
  TypeSVG = FALSE;
  Codepage = 0xC0;           //this is for PNG theme
  CodepageSize = 0xC0;           // INTN        CodepageSize; //extended latin
  Scale = 1.0f;
  CentreShift = 0.0f;
}


XImage& XTheme::GetIcon(XStringW& Name, BOOLEAN Night)
{
  XImage* TheIcon = NULL;
  for (size_t i = 0; i < Icons.size(); i++)
  {
    if (Icons[i].Name == Name)
    {
      if (Night) {
        TheIcon = &Icons[i].ImageNight;
      }
      if (TheIcon == NULL || (*TheIcon).isEmpty()) { //if daylight or night icon absent
        TheIcon = &Icons[i].Image;
      }
      break;
    }
  }
  return *TheIcon;
}

XImage& XTheme::GetIcon(INTN Id, BOOLEAN Night)
{
  XImage* TheIcon = NULL;
  for (size_t i = 0; i < Icons.size(); i++)
  {
    if (Icons[i].Id == Id)
    {
      if (Night) {
        TheIcon = &Icons[i].ImageNight;
      }
      if (TheIcon == NULL || (*TheIcon).isEmpty()) { //if daylight or night icon absent
        TheIcon = &Icons[i].Image;
      }
      break;
    }
  }
  return *TheIcon;
}

void XTheme::AddIcon(Icon& NewIcon)
{
  Icons.AddCopy(NewIcon);
}

//if ImageNight is not set then Image should be used
#define DEC_BUILTIN_ICON(id, ico) { \
   Icon NewIcon(id); \
   NewIcon.Image.FromPNG(ACCESS_EMB_DATA(ico), ACCESS_EMB_SIZE(ico)); \
   Icons.AddCopy(NewIcon); \
}

#define DEC_BUILTIN_ICON2(id, ico, dark) { \
   Icon NewIcon(id); \
   NewIcon.Image.FromPNG(ACCESS_EMB_DATA(ico), ACCESS_EMB_SIZE(ico)); \
   NewIcon.ImageNight.FromPNG(ACCESS_EMB_DATA(dark), ACCESS_EMB_SIZE(dark)); \
   Icons.AddCopy(NewIcon); \
}


void XTheme::FillByEmbedded()
{
    DEC_BUILTIN_ICON2(0, emb_func_about, emb_dark_func_about)
    DEC_BUILTIN_ICON2(1, emb_func_options, emb_dark_func_options)
    DEC_BUILTIN_ICON2(2, emb_func_clover, emb_dark_func_clover)
    DEC_BUILTIN_ICON2(3, emb_func_secureboot, emb_dark_func_secureboot)
    DEC_BUILTIN_ICON2(4, emb_func_secureboot_config, emb_dark_func_secureboot_config)
    DEC_BUILTIN_ICON2(5, emb_func_reset, emb_dark_func_reset)
    DEC_BUILTIN_ICON2(6, emb_func_exit, emb_dark_func_exit)
    DEC_BUILTIN_ICON2(7, emb_func_help, emb_dark_func_help)
    DEC_BUILTIN_ICON2(8, emb_func_shell, emb_dark_func_shell)
    DEC_BUILTIN_ICON(11, emb_pointer)
    DEC_BUILTIN_ICON(12, emb_vol_internal)
    DEC_BUILTIN_ICON(13, emb_vol_external)
    DEC_BUILTIN_ICON(14, emb_vol_optical)
    DEC_BUILTIN_ICON(16, emb_vol_internal_booter)
    DEC_BUILTIN_ICON(17, emb_vol_internal_hfs)
    DEC_BUILTIN_ICON(18, emb_vol_internal_apfs)
    DEC_BUILTIN_ICON(19, emb_vol_internal_ntfs)
    DEC_BUILTIN_ICON(20, emb_vol_internal_ext)
    DEC_BUILTIN_ICON(21, emb_vol_internal_recovery)
    DEC_BUILTIN_ICON(22, emb_logo, emb_dark_logo)
    DEC_BUILTIN_ICON2(23, emb_selection_small, emb_dark_selection_small)
    DEC_BUILTIN_ICON2(24, emb_selection_big, emb_dark_selection_big)

}