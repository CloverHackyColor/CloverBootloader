#if !defined(__XTHEME_H__)
#define __XTHEME_H__

#include "../cpp_foundation/XToolsCommon.h"
#include "../cpp_foundation/XObjArray.h"
#include "../cpp_foundation/XString.h"
#include "libeg.h"
#include "XImage.h"

#define INDICATOR_SIZE (52)

class Icon
{
public:
  INTN Id;  //for example BUILTIN_ICON_POINTER
  XStringW Name; //for example "os_moja", "vol_internal"
  XImage Image;
  XImage ImageNight;

  Icon(INTN Id);
  ~Icon();

};

class XTheme
{
public:
  XObjArray<Icon> Icons;
  EFI_FILE    *ThemeDir;

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
  INTN row0TileSize;
  INTN row1TileSize;
  UINTN BanHeight;
  INTN LayoutHeight; //it was 376 before
  BOOLEAN Daylight;

  void Init();
  XImage  Background; //Background and Banner will not be in array as they live own life
  XImage  BigBack; //it size is not equal to screen size will be scaled or cropped
  XImage  Banner; //same as logo in the array, make a link?
  XImage SelectionImages[6];
  XImage Button[4];

  //fill the theme
  XImage& GetIcon(XStringW& Name);  //get by name
  XImage& GetIcon(const char* Name);
  XImage& GetIcon(INTN Id); //get by id

  void AddIcon(Icon& NewIcon);  //return EFI_STATUS?
  void FillByEmbedded();

  //screen operations
  void ClearScreen();
  void FillRectAreaOfScreen(IN INTN XPos, IN INTN YPos, IN INTN Width, IN INTN Height);
  void InitSelection();

  XTheme(); //default constructor 
  ~XTheme();

protected:
  //internal layout variables instead of globals in menu.cpp

};
#endif
