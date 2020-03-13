/*
 * a class to keep definitions for all theme settings
 */
#include "XTheme.h"

Icon::Icon() {}
Icon::~Icon() {}

XTheme::XTheme()
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