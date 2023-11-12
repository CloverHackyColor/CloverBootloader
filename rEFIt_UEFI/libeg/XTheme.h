#if !defined(__XTHEME_H__)
#define __XTHEME_H__

#include "nanosvg.h"
#include "../cpp_foundation/XObjArray.h"
#include "../cpp_foundation/XString.h"
#include "../Settings/Self.h"
#include "libeg.h"
#include "XImage.h"
#include "XIcon.h"
#include "XCinema.h"


class TagDict;
class TagStruct;

#define INDICATOR_SIZE (52)
#define CONFIG_THEME_FILENAME L"theme.plist"
#define CONFIG_THEME_SVG L"theme.svg"
#define HEIGHT_2K 1100


EFI_STATUS InitTheme (const CHAR8* ChosenTheme);

extern textFaces nullTextFaces;

class XTheme
{
public:
  XObjArray<XIcon> Icons;
  XStringW     m_ThemePath = NullXStringW;
  EFI_FILE    *ThemeDir = 0;

//  UINTN       DisableFlags;
  UINTN       HideBadges;
  UINTN       HideUIFlags;
//  XBool    TextOnly;
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
  INTN        BackgroundSharp;
  XBool       BackgroundDark;
//  XBool    CustomIcons;
  XBool       SelectionOnTop;
  XBool       BootCampStyle;
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
  XBool       VerticalLayout;
  XBool       NonSelectedGrey;
  INTN        MainEntriesSize;
  INTN        TileXSpace;
  INTN        TileYSpace;
//  INTN        IconFormat;
  XBool       Proportional;
//  XBool    ShowOptimus;
  XBool       embedded;
  XBool       DarkEmbedded;
  XBool       TypeSVG;
//  INTN        Codepage;  //no! it is global settings
//  INTN        CodepageSize;
  float       Scale;
  float       CentreShift;
  INTN        row0TileSize;
  INTN        row1TileSize;
  INTN        BanHeight;
  INTN        LayoutHeight; //it was 376 before
  INTN        LayoutBannerOffset;
  INTN        LayoutButtonOffset;
  INTN        LayoutTextOffset;
  INTN        LayoutAnimMoveForMenuX;
  INTN        ScrollWidth;
  INTN        ScrollButtonsHeight;
  INTN        ScrollBarDecorationsHeight;
  INTN        ScrollScrollDecorationsHeight;


  INTN  FontWidth;
  INTN  FontHeight;
  INTN  TextHeight;

  XBool   Daylight;

  XImage  Background; //Background and Banner will not be in array as they live own life
  XImage  BigBack; //it size is not equal to screen size will be scaled or cropped
  XImage  Banner; //same as logo in the array, make a link?
  XImage  SelectionImages[6];
  XImage  Buttons[4];
  XImage  ScrollbarBackgroundImage;
  XImage  BarStartImage;
  XImage  BarEndImage;
  XImage  ScrollbarImage;
  XImage  ScrollStartImage;
  XImage  ScrollEndImage;
  XImage  UpButtonImage;
  XImage  DownButtonImage;

  XImage  FontImage;

  EG_RECT  BannerPlace;

  XCinema Cinema;

  UINTN NumFrames = 0;
  UINTN FrameTime = 0;


public:
  NSVGfontChain* fontsDB = 0;
  textFaces textFace[4]; //0-help 1-message 2-menu 3-test, far future it will be infinite list with id // in VectorGraphics, I use sizeof(textFace)/sizeof(textFace[0]. So if you change that to a pointer, it'll break.

  
  void Init();
  XTheme(); //default constructor
  XTheme(const XTheme&) = delete;
  XTheme& operator=(const XTheme&) = delete;

  ~XTheme() {
    if ( ThemeDir != NULL ) ThemeDir->Close(ThemeDir);
    if ( fontsDB ) {
      nsvg__deleteFontChain(fontsDB);
    }
    for (size_t i=0 ; i < Icons.length() ; ++i ) {
      Icons[i].setEmpty();
    }
  }

  
  const EFI_FILE& getThemeDir() const { return *ThemeDir; }
  XBool IsEmbeddedTheme(void)
  {
    if (embedded) {
      ThemeDir = NULL;
    }
    return ThemeDir == NULL;
  }


  //fill the theme
//  const XImage& GetIcon(const char* Name);
//  const XImage& GetIcon(const CHAR16* Name);
  const XIcon& GetIcon(const XString8& Name);  //get by name
        XIcon* GetIconP(const XString8& Name);
  const XIcon& GetIcon(INTN Id); //get by id
        XIcon& GetIconAlt(INTN Id, INTN Alt); //if id not found
  const XIcon& LoadOSIcon(const CHAR16* OSIconName); //TODO make XString provider
  const XIcon& LoadOSIcon(const XString8& Full);
  XBool CheckNative(INTN Id);
 
  //fonts
  void LoadFontImage(IN XBool UseEmbedded, IN INTN Rows, IN INTN Cols);
  void PrepareFont();
  INTN GetEmpty(const XImage& Buffer, const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& FirstPixel, INTN MaxWidth, INTN Start, INTN Step);
  INTN RenderText(IN const XStringW& Text, OUT XImage* CompImage_ptr,
                    IN INTN PosX, IN INTN PosY, IN UINTN Cursor, INTN textType, float textScale = 0.f);
  //overload for UTF8 text
  INTN RenderText(IN const XString8& Text, OUT XImage* CompImage_ptr,
                          IN INTN PosX, IN INTN PosY, IN UINTN Cursor, INTN textType, float textScale = 0.f);
  void MeasureText(IN const XStringW& Text, OUT INTN *Width, OUT INTN *Height);


//  void AddIcon(XIcon& NewIcon);  //return EFI_STATUS?
  void FillByEmbedded();
  void FillByDir();
  EFI_STATUS GetThemeTagSettings(const TagDict* DictPointer);
  void parseTheme(void* p, char** dict); //in nano project
  EFI_STATUS ParseSVGXTheme(UINT8* buffer, UINTN Size); // in VectorTheme
  EFI_STATUS ParseSVGXIcon(NSVGparser* SVGParser, INTN Id, const XString8& IconNameX, XImage* Image);
  TagDict* LoadTheme(const XStringW& TestTheme); //return TagStruct* why?
  EFI_STATUS LoadSvgFrame(NSVGparser* SVGParser, INTN i, OUT XImage* XFrame); // for animation

  const textFaces& getTextFace(size_t idx) {
    if (!TypeSVG ) return nullTextFaces;
    return textFace[idx];
  }

  //screen operations
  void ClearScreen();
  void FillRectAreaOfScreen(IN INTN XPos, IN INTN YPos, IN INTN Width, IN INTN Height);
//  void InitSelection();
  void InitBar();

protected:
  //internal layout variables instead of globals in menu.cpp

};

extern XTheme* ThemeX;


#endif
