#if !defined(__XTHEME_H__)
#define __XTHEME_H__

#include "../cpp_foundation/XToolsCommon.h"
#include "../cpp_foundation/XObjArray.h"
#include "../cpp_foundation/XStringW.h"
#include "libeg.h"
#include "XImage.h"

class Icon
{
public:
  INTN Id;  //for example BUILTIN_ICON_POINTER
  XStringW Name; //for example "os_moja", "vol_internal"
  XImage ImageNight;
  XImage Image;

  Icon();
  ~Icon();

};

class XTheme
{
public:
  XObjArray<Icon> Icons;

  UINTN       DisableFlags;
  UINTN       HideBadges;
  UINTN       HideUIFlags;
  BOOLEAN     TextOnly;
  FONT_TYPE   Font;
  INTN        CharWidth;
  UINTN       SelectionColor;
  XStringW    FontFileName;
  XStringW    Theme;
  XStringW    BannerFileName;
  XStringW    SelectionSmallFileName;
  XStringW    SelectionBigFileName;
  XStringW    SelectionIndicatorName;
  XStringW    DefaultSelection;
  XStringW    BackgroundName;
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
//  INTN        IconFormat;
  BOOLEAN     Proportional;
  BOOLEAN     ShowOptimus;
  BOOLEAN     DarkEmbedded;
  BOOLEAN     TypeSVG;
  INTN        Codepage;
  INTN        CodepageSize;
  float       Scale;
  float       CentreShift;

  XImage  Background; //Background and Banner will not be in array as they live own life
  XImage  Banner;

  XImage& GetIcon(XStringW& Name, BOOLEAN Night);  //get by name
  XImage& GetIcon(INTN Id, BOOLEAN Night); //get by id

  void AddIcon(Icon& NewIcon);  //return EFI_STATUS?

  XTheme(); //default constructor 
  ~XTheme();

protected:
  //internal layout variables instead of globals in menu.cpp

};
#endif