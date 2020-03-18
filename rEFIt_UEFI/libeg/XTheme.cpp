/*
 * a class to keep definitions for all theme settings
 */

extern "C" {
#include <Protocol/GraphicsOutput.h>
}

#include "libegint.h"
#include "../refit/screen.h"

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
  Name.takeValueFrom(IconsNames[Index]);
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
  Daylight = true;
}


XImage& XTheme::GetIcon(XStringW& Name)
{
  XImage* TheIcon = NULL;
  for (size_t i = 0; i < Icons.size(); i++)
  {
    if (Icons[i].Name == Name)
    {
      if (!Daylight) {
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

XImage& XTheme::GetIcon(INTN Id)
{
  XImage* TheIcon = NULL;
  for (size_t i = 0; i < Icons.size(); i++)
  {
    if (Icons[i].Id == Id)
    {
      if (!Daylight) {
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
    DEC_BUILTIN_ICON2(22, emb_logo, emb_dark_logo)
    DEC_BUILTIN_ICON2(23, emb_selection_small, emb_dark_selection_small)
    DEC_BUILTIN_ICON2(24, emb_selection_big, emb_dark_selection_big)

}

void XTheme::ClearScreen() //and restore background and banner
{
  if (BanHeight < 2) {
    BanHeight = ((UGAHeight - (int)(LayoutHeight * Scale)) >> 1);
  }
  if (!(HideUIFlags & HIDEUI_FLAG_BANNER)) {
    //Banner image prepared before
    if (!Banner.isEmpty()) {
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
          BannerPlace.XPos = (UGAWidth - Banner.GetWidth()) >> 1;
          BannerPlace.YPos = (BanHeight >= Banner.GetHeight()) ? (BanHeight - Banner.GetHeight()) : 0;
          //        DBG("banner position old style\n");
        }
      }
      
    }
  }
  
  //Then prepare Background from BigBack
  if (!Background.isEmpty() && (Background.GetWidth() != (UINTN)UGAWidth || Background.GetHeight() != (UINTN)UGAHeight)) { // should we type UGAWidth and UGAHeight as UINTN to avoid cast ?
    // Resolution changed
    Background.setEmpty();
  }
  
  if (Background.isEmpty()) {
    Background = XImage(UGAWidth, UGAHeight);
    Background.Fill((EFI_GRAPHICS_OUTPUT_BLT_PIXEL&)BlueBackgroundPixel);
  }
  if (!BigBack.isEmpty()) {
    switch (BackgroundScale) {
    case imScale:
      Background.CopyScaled(BigBack, Scale);
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
      //the function can be in XImage class
/*      egRawCopy(Background.GetPixelPtr(x1, y1),
                BigBack.GetPixelPtr(x2, y2),
                x, y, Background.GetWidth(), BigBack.GetWidth()); */
      Background.Compose(x, y, BigBack, true);
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
      break;
    }
    case imNone:
    default:
      // already scaled
      break;
    }
  }
  Background.Draw(0, 0, 1.f);
  //then draw banner
  if (!Banner.isEmpty()) {
    Banner.Draw(BannerPlace.XPos, BannerPlace.YPos, Scale);
  }
  
}

void XTheme::InitSelection()
{
  
  if (!AllowGraphicsMode)
    return;
  SelectionBackgroundPixel.r = (SelectionColor >> 24) & 0xFF;
  SelectionBackgroundPixel.g = (SelectionColor >> 16) & 0xFF;
  SelectionBackgroundPixel.b = (SelectionColor >> 8) & 0xFF;
  SelectionBackgroundPixel.a = (SelectionColor >> 0) & 0xFF;
  
  if (!SelectionImages[0].isEmpty()) {
    return;
  }
  // load small selection image
  if (SelectionSmallFileName.isEmpty()){
    SelectionImages[2].LoadImage(ThemeDir, SelectionSmallFileName);
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
  //TODO - to be continued
  // load big selection image
  if (!TypeSVG && !SelectionBigFileName.isEmpty()) {
    SelectionImages[0].LoadImage(ThemeDir, SelectionBigFileName);
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
    CopyMem(&BlueBackgroundPixel, &StdBackgroundPixel, sizeof(EG_PIXEL));
    if (SelectionImages[0].isEmpty()) {
      SelectionImages[2].setEmpty();
      return;
    }
//    if (SelectionOnTop) {
//      SelectionImages[0]->HasAlpha = TRUE; // TODO ?
//      SelectionImages[2]->HasAlpha = TRUE;
//    }
  }
  
  // BootCampStyle indicator image
  if (BootCampStyle) {
    // load indicator selection image
    if (!SelectionIndicatorName.isEmpty()) {
      SelectionImages[4].LoadImage(ThemeDir, SelectionIndicatorName);
    }
    if (!SelectionImages[4].isEmpty()) {
      SelectionImages[4].FromPNG(ACCESS_EMB_DATA(emb_selection_indicator), ACCESS_EMB_SIZE(emb_selection_indicator));     
    }
    INTN ScaledIndicatorSize = (INTN)(INDICATOR_SIZE * Scale);
//    SelectionImages[4].EnsureImageSize(ScaledIndicatorSize, ScaledIndicatorSize, &MenuBackgroundPixel);
    if (SelectionImages[4].isEmpty()) {
//      SelectionImages[4] = egCreateFilledImage(ScaledIndicatorSize, ScaledIndicatorSize,
//                                               TRUE, &StdBackgroundPixel);
      SelectionImages[4] = XImage(ScaledIndicatorSize, ScaledIndicatorSize);
      SelectionImages[4].Fill((EFI_GRAPHICS_OUTPUT_BLT_PIXEL&)StdBackgroundPixel);


    }
 //   SelectionImages[5] = egCreateFilledImage(ScaledIndicatorSize, ScaledIndicatorSize,
 //                                            TRUE, &MenuBackgroundPixel);
    SelectionImages[5] = XImage(ScaledIndicatorSize, ScaledIndicatorSize);
    SelectionImages[5].Fill((EFI_GRAPHICS_OUTPUT_BLT_PIXEL&)MenuBackgroundPixel);

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

  Button[0] = GetIcon(XStringWP("radio_button"));
  Button[1] = GetIcon(XStringWP("radio_button_selected"));
  Button[2] = GetIcon(XStringWP("checkbox"));
  Button[3] = GetIcon(XStringWP("checkbox_checked"));

  // non-selected background images

  EFI_GRAPHICS_OUTPUT_BLT_PIXEL& BackgroundPixel = { 0xbf, 0xbf, 0xbf, 0xff };
  if (!SelectionBigFileName.isEmpty()) {
    BackgroundPixel = { 0x00, 0x00, 0x00, 0x00 };
  } else if (DarkEmbedded || TypeSVG) {
    BackgroundPixel = { 0x33, 0x33, 0x33, 0xff };
  } else {
    BackgroundPixel = { 0xbf, 0xbf, 0xbf, 0xff };
  }
  SelectionImages[1] = XImage(row0TileSize, row0TileSize);
  SelectionImages[1].Fill((EFI_GRAPHICS_OUTPUT_BLT_PIXEL&)BackgroundPixel);
  SelectionImages[3] = XImage(row1TileSize, row1TileSize);
  SelectionImages[3].Fill((EFI_GRAPHICS_OUTPUT_BLT_PIXEL&)BackgroundPixel);

}
