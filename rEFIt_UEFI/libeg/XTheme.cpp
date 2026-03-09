/*
 * a class to keep definitions for all theme settings
 */

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile

extern "C" {
#include <Protocol/GraphicsOutput.h>
}

#include "libegint.h"
#include "../refit/screen.h"
#include "../refit/lib.h"
#include "../Platform/plist/plist.h"
#include "../Platform/Settings.h"
//#include "../Platform/Nvram.h"
#include "../Platform/StartupSound.h"

#include "XTheme.h"
#include "nanosvg.h"

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

XTheme* ThemeX = NULL;
textFaces nullTextFaces;


EFI_STATUS
InitTheme(const CHAR8* ChosenTheme)
{
EFI_STATUS Status       = EFI_NOT_FOUND;
  UINTN      i;
  TagDict*     ThemeDict    = NULL;
//  CHAR8      *ChosenTheme = NULL;
  UINTN      Rnd;
  EFI_TIME   Now;

  gRT->GetTime(&Now, NULL);
  DbgHeader("InitXTheme");

  if ( ThemeX != NULL ) delete ThemeX;
  ThemeX = new XTheme();
  ThemeX->Init();

  //initialize Daylight when we know timezone
  if (gSettings.GUI.Timezone != 0xFF) { // 0xFF:default=timezone not set
    INT32 NowHour = Now.Hour + gSettings.GUI.Timezone;
 //   DBG("now is %d, zone is %d\n", Now.Hour, gSettings.GUI.Timezone);
    if (NowHour <  0 ) NowHour += 24;
    if (NowHour >= 24 ) NowHour -= 24;
    ThemeX->Daylight = (NowHour > 8) && (NowHour < 20);
  } else {
    ThemeX->Daylight = true; // when timezone is not set
  }
  if (ThemeX->Daylight) {
    DBG("use Daylight theme\n");
  } else {
    DBG("use night theme\n");
  }

  ThemeX->FontImage.setEmpty();

  Rnd = (ThemeNameArray.size() != 0) ? Now.Second % ThemeNameArray.size() : 0;

  //  DBG("...done\n");
  ThemeX->GetThemeTagSettings(NULL);

  if (ThemeNameArray.size() > 0  &&
      (gSettings.GUI.Theme.isEmpty() || StriCmp(gSettings.GUI.Theme.wc_str(), L"embedded") != 0)) {
    // Try special theme first
      XStringW TestTheme;
 //   if (Time != NULL) {
      if ((Now.Month == 12) && ((Now.Day >= 25) && (Now.Day <= 31))) {
        TestTheme = L"christmas"_XSW;
      } else if ((Now.Month == 1) && ((Now.Day >= 1) && (Now.Day <= 3))) {
        TestTheme = L"newyear"_XSW;
      }

      if (TestTheme.notEmpty()) {
        ThemeDict = ThemeX->LoadTheme(TestTheme);
        if (ThemeDict != NULL) {
          DBG("special theme %ls found and %ls parsed\n", TestTheme.wc_str(), CONFIG_THEME_FILENAME);
//          ThemeX->Theme.takeValueFrom(TestTheme);
          gSettings.GUI.Theme = TestTheme;

        } else { // special theme not loaded
          DBG("special theme %ls not found, skipping\n", TestTheme.wc_str()/*, CONFIG_THEME_FILENAME*/);
        }
        TestTheme.setEmpty();
      }
//    }
    // Try theme from nvram
    if (ThemeDict == NULL && ChosenTheme) {
      if (AsciiStrCmp(ChosenTheme, "embedded") == 0) {
        goto finish;
      }
      if (AsciiStrCmp(ChosenTheme, "random") == 0) {
        ThemeDict = ThemeX->LoadTheme(XStringW(ThemeNameArray[Rnd]));
        goto finish;
      }

      TestTheme.takeValueFrom(ChosenTheme);
      if (TestTheme.notEmpty()) {
        ThemeDict = ThemeX->LoadTheme (TestTheme);
        if (ThemeDict != NULL) {
          DBG("theme %s defined in NVRAM found and %ls parsed\n", ChosenTheme, CONFIG_THEME_FILENAME);
//            ThemeX->Theme.takeValueFrom(TestTheme);
          gSettings.GUI.Theme = TestTheme;
        } else { // theme from nvram not loaded
          if (gSettings.GUI.Theme.notEmpty()) {
            DBG("theme %s chosen from nvram is absent, using theme defined in config: %ls\n", ChosenTheme, gSettings.GUI.Theme.wc_str());
          } else {
            DBG("theme %s chosen from nvram is absent, get first theme\n", ChosenTheme);
          }
        }
        TestTheme.setEmpty();
      }
      //FreePool(ChosenTheme); // ChosenTheme is an argument passed here, so the callee is "own" that memory.
      //ChosenTheme = NULL;    // Why is this bad pratice : it's assuming that ChosenTheme was dynamically allocated and freeable. What is this the content of an XString ?
    }
    // Try to get theme from settings
    if (ThemeDict == NULL) {
      if (gSettings.GUI.Theme.isEmpty()) {
        DBG("no default theme, get random theme %ls\n", ThemeNameArray[Rnd].wc_str());
        ThemeDict = ThemeX->LoadTheme(XStringW(ThemeNameArray[Rnd]));
      } else {
        if (StriCmp(gSettings.GUI.Theme.wc_str(), L"random") == 0) {
          ThemeDict = ThemeX->LoadTheme(XStringW(ThemeNameArray[Rnd]));
        } else {
          ThemeDict = ThemeX->LoadTheme(gSettings.GUI.Theme);
//          if (ThemeDict == NULL) {
//            DBG("GlobalConfig: %ls not found, get embedded theme\n", gSettings.GUI.Theme.wc_str());
//          } else {
//            DBG("chosen theme %ls\n", gSettings.GUI.Theme.wc_str());
//          }
        }
      }
    }
  } // ThemesNum>0

finish:
  if (!ThemeDict) {  // No theme could be loaded, use embedded
    DBG(" using embedded theme\n");
    if (ThemeX->DarkEmbedded) { // when using embedded, set Daylight according to darkembedded
      ThemeX->Daylight = false;
    } else {
      ThemeX->Daylight = true;
    }

    ThemeX->FillByEmbedded();
    OldChosenTheme = 0xFFFF;

    if (ThemeX->ThemeDir != NULL) {
      ThemeX->ThemeDir->Close(ThemeX->ThemeDir);
      ThemeX->ThemeDir = NULL;
    }

 //   ThemeX->GetThemeTagSettings(NULL); already done
    //fill some fields
    //ThemeX->Font = FONT_ALFA; //to be inverted later. At start we have FONT_GRAY
    ThemeX->embedded = true;
    Status = StartupSoundPlay(&ThemeX->getThemeDir(), NULL);
  } else { // theme loaded successfully
    ThemeX->embedded = false;
    ThemeX->Theme.takeValueFrom(gSettings.GUI.Theme); //XStringW from CHAR16*)
    // read theme settings
    if (!ThemeX->TypeSVG) {
      const TagDict* DictPointer = ThemeDict->dictPropertyForKey("Theme");
      if (DictPointer != NULL) {
        Status = ThemeX->GetThemeTagSettings(DictPointer);
        if (EFI_ERROR(Status)) {
          DBG("Config theme error: %s\n", efiStrError(Status));
        } else {
          ThemeX->FillByDir();
        }
      }
    }
    ThemeDict->ReleaseTag();

    if (!ThemeX->Daylight) {
      Status = StartupSoundPlay(&ThemeX->getThemeDir(), L"sound_night.wav");
      if (EFI_ERROR(Status)) {
        Status = StartupSoundPlay(&ThemeX->getThemeDir(), L"sound.wav");
      }
    } else {
      Status = StartupSoundPlay(&ThemeX->getThemeDir(), L"sound.wav");
    }

  }
  for (i = 0; i < ThemeNameArray.size(); i++) {
    if ( ThemeX->Theme.isEqualIC(ThemeNameArray[i]) ) {
      OldChosenTheme = i;
      break;
    }
  }
  if (ChosenTheme != NULL) {
      //FreePool(ChosenTheme); // ChosenTheme is an argument passed here, so the callee is "own" that memory.
                               // Why is this bad pratice : it's assuming that ChosenTheme was dynamically allocated and freeable. What is this the content of an XString ?
  }
  if (!ThemeX->TypeSVG) {
    ThemeX->PrepareFont();
  }
  //ThemeX->ClearScreen();
#ifdef JIEF_DEBUG
  displayFreeMemory("InitTheme end"_XS8);
#endif
  return Status;
}






//xtheme class
XTheme::XTheme() : Icons(), ThemeDir(0), HideBadges(0), HideUIFlags(0), Font(FONT_ALFA), CharWidth(0), SelectionColor(0), FontFileName(), Theme(),
                   BannerFileName(), SelectionSmallFileName(), SelectionBigFileName(), SelectionIndicatorName(), DefaultSelection(),
                   BackgroundName(), BackgroundScale(imNone), BackgroundSharp(0), BackgroundDark(false), SelectionOnTop(false), BootCampStyle(false),
                   BadgeOffsetX(0), BadgeOffsetY(0), BadgeScale(0), ThemeDesignWidth(0), ThemeDesignHeight(0), BannerPosX(0), BannerPosY(0),
                   BannerEdgeHorizontal(0), BannerEdgeVertical(0), BannerNudgeX(0), BannerNudgeY(0), VerticalLayout(false), NonSelectedGrey(false),
                   MainEntriesSize(0), TileXSpace(0), TileYSpace(0), Proportional(false), embedded(false), DarkEmbedded(false), TypeSVG(false), Scale(0), CentreShift(0),
                   row0TileSize(0), row1TileSize(0), BanHeight(0), LayoutHeight(0), LayoutBannerOffset(0), LayoutButtonOffset(0), LayoutTextOffset(0),
                   LayoutAnimMoveForMenuX(0), ScrollWidth(0), ScrollButtonsHeight(0), ScrollBarDecorationsHeight(0), ScrollScrollDecorationsHeight(0),
                   FontWidth(0), FontHeight(0), TextHeight(0), Daylight(false), Background(), BigBack(), Banner(), SelectionImages(), Buttons(), ScrollbarBackgroundImage(), BarStartImage(), BarEndImage(),
                   ScrollbarImage(), ScrollStartImage(), ScrollEndImage(), UpButtonImage(), DownButtonImage(), FontImage(), BannerPlace(), Cinema()
{
  Init();
}


void XTheme::Init()
{
//  DisableFlags = 0;             
  HideBadges = 0; 
  HideUIFlags = 0; 
//  TextOnly = false; 
  Font = FONT_GRAY;      // FONT_TYPE   
  CharWidth = 9;  
  SelectionColor = 0x80808080;
  SelectionBackgroundPixel = { 0xef, 0xef, 0xef, 0xff };
  FontFileName.setEmpty();

  Icons.setEmpty();
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
  BackgroundDark = false;       //TODO should be set to true if Night theme
//  CustomIcons = false;          //TODO don't know how to handle with SVG theme
  SelectionOnTop = false;         
  BootCampStyle = false; 
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
  VerticalLayout = false;
  NonSelectedGrey = false;    
  MainEntriesSize = 128;
  TileXSpace = 8;
  TileYSpace = 24;

  Proportional = false;
//  ShowOptimus = false;
//  DarkEmbedded = false;  //looks like redundant, we always check Night or Daylight
  TypeSVG = false;
//  Codepage = 0xC0;           //this is for PNG theme
//  CodepageSize = 0xC0;           // INTN        CodepageSize; //extended latin
  Scale = 1.0f;
  CentreShift = 0.0f;
  Daylight = true;
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

TagDict* XTheme::LoadTheme(const XStringW& TestTheme)
{
  EFI_STATUS Status    = EFI_UNSUPPORTED;
  TagDict*     ThemeDict = NULL;
  UINT8      *ThemePtr = NULL;
  UINTN      Size      = 0;

  if (TestTheme.isEmpty()) {
    return NULL;
  }
  if (UGAHeight > HEIGHT_2K) {
    m_ThemePath = SWPrintf("%ls@2x", TestTheme.wc_str());
  } else {
    m_ThemePath = SWPrintf("%ls", TestTheme.wc_str());
  }
  Status = self.getThemesDir().Open(&self.getThemesDir(), &ThemeDir, m_ThemePath.wc_str(), EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(Status)) {
    if (ThemeDir != NULL) {
      ThemeDir->Close (ThemeDir);
      ThemeDir = NULL;
    }
    m_ThemePath = SWPrintf("%ls", TestTheme.wc_str());
    Status = self.getThemesDir().Open(&self.getThemesDir(), &ThemeDir, m_ThemePath.wc_str(), EFI_FILE_MODE_READ, 0);
  }

  if (!EFI_ERROR(Status)) {
    Status = egLoadFile(ThemeDir, CONFIG_THEME_SVG, &ThemePtr, &Size);
    if (!EFI_ERROR(Status) && (ThemePtr != NULL) && (Size != 0)) {
      Status = ParseSVGXTheme(ThemePtr, Size);
      if (EFI_ERROR(Status)) {
        ThemeDict = NULL;
      } else {
        ThemeDict = TagDict::getEmptyTag();
      }
      if (ThemeDict == NULL) {
        DBG("svg file %ls not parsed\n", CONFIG_THEME_SVG);
      } else {
        DBG("Using vector theme '%ls' (%ls)\n", TestTheme.wc_str(), m_ThemePath.wc_str());
      }
    } else {
      Status = egLoadFile(ThemeDir, CONFIG_THEME_FILENAME, &ThemePtr, &Size);
      if (!EFI_ERROR(Status) && (ThemePtr != NULL) && (Size != 0)) {
        Status = ParseXML(ThemePtr, &ThemeDict, 0);
        if (EFI_ERROR(Status)) {
          ThemeDict = NULL;
        }
        if (ThemeDict == NULL) {
          DBG("xml file %ls not parsed\n", CONFIG_THEME_FILENAME);
        } else {
          DBG("Using theme '%ls' (%ls)\n", TestTheme.wc_str(), m_ThemePath.wc_str());
        }
      }
    }
  }
  if (ThemePtr != NULL) {
    FreePool(ThemePtr);
  }
  return ThemeDict;
}

EFI_STATUS
XTheme::GetThemeTagSettings(const TagDict* DictPointer)
{
  const TagDict* Dict;
  const TagDict* Dict3;
  const TagStruct* Prop;
  const TagStruct* Prop2;

  //fill default to have an ability change theme
  //assume Xtheme is already inited by embedded values
//theme variables
  ScrollWidth                           = 16;
  ScrollButtonsHeight                   = 20;
  ScrollBarDecorationsHeight            = 5;
  ScrollScrollDecorationsHeight         = 7;
  Font                     = FONT_LOAD; //not default

  // if NULL parameter, quit after setting default values, this is embedded theme
  if (DictPointer == NULL) {
    return EFI_SUCCESS;
  }

  Prop    = DictPointer->propertyForKey("BootCampStyle");
  BootCampStyle = IsPropertyNotNullAndTrue(Prop);

  Dict    = DictPointer->dictPropertyForKey("Background");
  if (Dict != NULL) {
    Prop = Dict->propertyForKey("Type");
    if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty() ) {
      if ((Prop->getString()->stringValue()[0] == 'S') || (Prop->getString()->stringValue()[0] == 's')) {
        BackgroundScale = imScale;
      } else if ((Prop->getString()->stringValue()[0] == 'T') || (Prop->getString()->stringValue()[0] == 't')) {
        BackgroundScale = imTile;
      }
    }

    Prop = Dict->propertyForKey("Path");
    if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty() ) {
      BackgroundName = Prop->getString()->stringValue();
    }

    Prop = Dict->propertyForKey("Sharp");
    BackgroundSharp  = GetPropertyAsInteger(Prop, BackgroundSharp);

    Prop = Dict->propertyForKey("Dark");
    BackgroundDark   = IsPropertyNotNullAndTrue(Prop);
  }

  Prop = DictPointer->propertyForKey("Banner");
  if (Prop != NULL) {
    // retain for legacy themes.
    if ( Prop->isString() && Prop->getString()->stringValue().notEmpty() ) {
      BannerFileName = Prop->getString()->stringValue();
    } else {
      // for new placement settings
      Dict = Prop->getDict();
      Prop2 = Dict->propertyForKey("Path");
      if (Prop2 != NULL) {
        if ( Prop2->isString() && Prop2->getString()->stringValue().notEmpty() ) {
          BannerFileName = Prop2->getString()->stringValue();
        }
      }

      Prop2 = Dict->propertyForKey("ScreenEdgeX");
      if (Prop2 != NULL && (Prop2->isString()) && Prop2->getString()->stringValue().notEmpty() ) {
        if (Prop2->getString()->stringValue().isEqual("left")) {
          BannerEdgeHorizontal = SCREEN_EDGE_LEFT;
        } else if (Prop2->getString()->stringValue().isEqual("right")) {
          BannerEdgeHorizontal = SCREEN_EDGE_RIGHT;
        }
      }

      Prop2 = Dict->propertyForKey("ScreenEdgeY");
      if (Prop2 != NULL && (Prop2->isString()) && Prop2->getString()->stringValue().notEmpty() ) {
        if (Prop2->getString()->stringValue().isEqual("top")) {
          BannerEdgeVertical = SCREEN_EDGE_TOP;
        } else if (Prop2->getString()->stringValue().isEqual("bottom")) {
          BannerEdgeVertical = SCREEN_EDGE_BOTTOM;
        }
      }

      Prop2 = Dict->propertyForKey("DistanceFromScreenEdgeX%");
      BannerPosX   = GetPropertyAsInteger(Prop2, 0);

      Prop2 = Dict->propertyForKey("DistanceFromScreenEdgeY%");
      BannerPosY   = GetPropertyAsInteger(Prop2, 0);

      Prop2 = Dict->propertyForKey("NudgeX");
      BannerNudgeX = GetPropertyAsInteger(Prop2, 0);

      Prop2 = Dict->propertyForKey("NudgeY");
      BannerNudgeY = GetPropertyAsInteger(Prop2, 0);
    }
  }

  Dict = DictPointer->dictPropertyForKey("Badges");
  if (Dict != NULL) {
    Prop = Dict->propertyForKey("Swap");
    if (Prop != NULL && Prop->isTrue()) {
      HideBadges |= HDBADGES_SWAP;
      DBG("OS main and drive as badge\n");
    }

    Prop = Dict->propertyForKey("Show");
    if (Prop != NULL && Prop->isTrue()) {
      HideBadges |= HDBADGES_SHOW;
    }

    Prop = Dict->propertyForKey("Inline");
    if (Prop != NULL && Prop->isTrue()) {
      HideBadges |= HDBADGES_INLINE;
    }

    // blackosx added X and Y position for badge offset.
    Prop = Dict->propertyForKey("OffsetX");
    BadgeOffsetX = GetPropertyAsInteger(Prop, BadgeOffsetX);

    Prop = Dict->propertyForKey("OffsetY");
    BadgeOffsetY = GetPropertyAsInteger(Prop, BadgeOffsetY);

    Prop = Dict->propertyForKey("Scale");
    BadgeScale = GetPropertyAsInteger(Prop, BadgeScale);
  }

  Dict = DictPointer->dictPropertyForKey("Origination");
  if (Dict != NULL) {
    Prop = Dict->propertyForKey("DesignWidth");
    ThemeDesignWidth = GetPropertyAsInteger(Prop, ThemeDesignWidth);

    Prop = Dict->propertyForKey("DesignHeight");
    ThemeDesignHeight = GetPropertyAsInteger(Prop, ThemeDesignHeight);
  }

  Dict = DictPointer->dictPropertyForKey("Layout");
  if (Dict != NULL) {
    Prop = Dict->propertyForKey("BannerOffset");
    LayoutBannerOffset = GetPropertyAsInteger(Prop, LayoutBannerOffset);

    Prop = Dict->propertyForKey("ButtonOffset");
    LayoutButtonOffset = GetPropertyAsInteger(Prop, LayoutButtonOffset);

    Prop = Dict->propertyForKey("TextOffset");
    LayoutTextOffset = GetPropertyAsInteger(Prop, LayoutTextOffset);

    Prop = Dict->propertyForKey("AnimAdjustForMenuX");
    LayoutAnimMoveForMenuX = GetPropertyAsInteger(Prop, LayoutAnimMoveForMenuX);

    Prop = Dict->propertyForKey("Vertical");
    VerticalLayout = IsPropertyNotNullAndTrue(Prop);

    // GlobalConfig.MainEntriesSize
    Prop = Dict->propertyForKey("MainEntriesSize");
    MainEntriesSize = GetPropertyAsInteger(Prop, MainEntriesSize);

    Prop = Dict->propertyForKey("TileXSpace");
    TileXSpace = GetPropertyAsInteger(Prop, TileXSpace);

    Prop = Dict->propertyForKey("TileYSpace");
    TileYSpace = GetPropertyAsInteger(Prop, TileYSpace);

    Prop = Dict->propertyForKey("SelectionBigWidth");
    row0TileSize = GetPropertyAsInteger(Prop, row0TileSize);

    Prop = Dict->propertyForKey("SelectionSmallWidth");
    row1TileSize = (INTN)GetPropertyAsInteger(Prop, row1TileSize);

  }

  Dict = DictPointer->dictPropertyForKey("Components");
  if (Dict != NULL) {
    Prop = Dict->propertyForKey("Banner");
    if (Prop && Prop->isFalse()) {
      HideUIFlags |= HIDEUI_FLAG_BANNER;
    }

    Prop = Dict->propertyForKey("Functions");
    if (Prop && Prop->isFalse()) {
      HideUIFlags |= HIDEUI_FLAG_FUNCS;
    }

    Prop = Dict->propertyForKey("Tools");
    if (Prop && Prop->isFalse()) {
      HideUIFlags |= HIDEUI_FLAG_TOOLS;
    }

    Prop = Dict->propertyForKey("Label");
    if (Prop && Prop->isFalse()) {
      HideUIFlags |= HIDEUI_FLAG_LABEL;
    }

    Prop = Dict->propertyForKey("Revision");
    if (Prop && Prop->isFalse()) {
      HideUIFlags |= HIDEUI_FLAG_REVISION;
    }

    Prop = Dict->propertyForKey("Help");
    if (Prop && Prop->isFalse()) {
      HideUIFlags |= HIDEUI_FLAG_HELP;
    }

    Prop = Dict->propertyForKey("MenuTitle");
    if (Prop && Prop->isFalse()) {
      HideUIFlags |= HIDEUI_FLAG_MENU_TITLE;
    }

    Prop = Dict->propertyForKey("MenuTitleImage");
    if (Prop && Prop->isFalse()) {
      HideUIFlags |= HIDEUI_FLAG_MENU_TITLE_IMAGE;
    }
  }

  Dict = DictPointer->dictPropertyForKey("Selection");
  if (Dict != NULL) {
    Prop = Dict->propertyForKey("Color");
    SelectionColor = (UINTN)GetPropertyAsInteger(Prop, SelectionColor);

    Prop = Dict->propertyForKey("Small");
    if ( Prop && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
      SelectionSmallFileName = Prop->getString()->stringValue();
    }

    Prop = Dict->propertyForKey("Big");
    if ( Prop && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
      SelectionBigFileName = Prop->getString()->stringValue();
    }

    Prop = Dict->propertyForKey("Indicator");
    if ( Prop && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
      SelectionIndicatorName = Prop->getString()->stringValue();
    }

    Prop = Dict->propertyForKey("OnTop");
    SelectionOnTop = IsPropertyNotNullAndTrue(Prop);

    Prop = Dict->propertyForKey("ChangeNonSelectedGrey");
    NonSelectedGrey = IsPropertyNotNullAndTrue(Prop);
  }

  Dict = DictPointer->dictPropertyForKey("Scroll");
  if (Dict != NULL) {
    Prop = Dict->propertyForKey("Width");
    ScrollWidth = (UINTN)GetPropertyAsInteger(Prop, ScrollWidth);

    Prop = Dict->propertyForKey("Height");
    ScrollButtonsHeight = (UINTN)GetPropertyAsInteger(Prop, ScrollButtonsHeight);

    Prop = Dict->propertyForKey("BarHeight");
    ScrollBarDecorationsHeight = (UINTN)GetPropertyAsInteger(Prop, ScrollBarDecorationsHeight);

    Prop = Dict->propertyForKey("ScrollHeight");
    ScrollScrollDecorationsHeight = (UINTN)GetPropertyAsInteger(Prop,ScrollScrollDecorationsHeight);
  }

  Dict = DictPointer->dictPropertyForKey("Font");
  if (Dict != NULL) {
    Prop = Dict->propertyForKey("Type");
    if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty() ) {
      if ((Prop->getString()->stringValue()[0] == 'A') || (Prop->getString()->stringValue()[0] == 'B')) {
        Font = FONT_ALFA;
      } else if ((Prop->getString()->stringValue()[0] == 'G') || (Prop->getString()->stringValue()[0] == 'W')) {
        Font = FONT_GRAY;
      } else if ((Prop->getString()->stringValue()[0] == 'L') || (Prop->getString()->stringValue()[0] == 'l')) {
        Font = FONT_LOAD;
      }
    }
    if (Font == FONT_LOAD) {
      Prop = Dict->propertyForKey("Path");
      if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
        FontFileName = Prop->getString()->stringValue();
      }
    }
    Prop = Dict->propertyForKey("CharWidth");
    CharWidth = (UINTN)GetPropertyAsInteger(Prop, CharWidth);
    if (CharWidth & 1) {
      MsgLog("Warning! Character width %lld should be even!\n", CharWidth);
    }

    Prop = Dict->propertyForKey("Proportional");
    Proportional = IsPropertyNotNullAndTrue(Prop);
  }

  const TagArray* AnimeArray = DictPointer->arrayPropertyForKey("Anime"); // array of dict
  if (AnimeArray != NULL) {
    INTN   Count = AnimeArray->arrayContent().size();
    for (INTN i = 0; i < Count; i++) {
      if ( !AnimeArray->elementAt(i)->isDict() ) {
        MsgLog("MALFORMED PLIST : Anime must be an array of dict\n");
        continue;
      }
      Dict3 = AnimeArray->dictElementAt(i, "Anime"_XS8);

      FILM *NewFilm = new FILM;

      Prop = Dict3->propertyForKey("ID");
      NewFilm->SetIndex((UINTN)GetPropertyAsInteger(Prop, 1)); //default=main screen

      Prop = Dict3->propertyForKey("Path");
      if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
        NewFilm->Path = Prop->getString()->stringValue();
      }

      Prop = Dict3->propertyForKey("Frames");
      NewFilm->NumFrames = (UINTN)GetPropertyAsInteger(Prop, 0);

      Prop = Dict3->propertyForKey("FrameTime");
      NewFilm->FrameTime = (UINTN)GetPropertyAsInteger(Prop, 50); //default will be 50ms

      Prop = Dict3->propertyForKey("ScreenEdgeX");
      if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty() ) {
        if (Prop->getString()->stringValue().isEqual("left")) {
          NewFilm->ScreenEdgeHorizontal = SCREEN_EDGE_LEFT;
        } else if (Prop->getString()->stringValue().isEqual("right")) {
          NewFilm->ScreenEdgeHorizontal = SCREEN_EDGE_RIGHT;
        }
      }

      Prop = Dict3->propertyForKey("ScreenEdgeY");
      if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty() ) {
        if (Prop->getString()->stringValue().isEqual("top")) {
          NewFilm->ScreenEdgeVertical = SCREEN_EDGE_TOP;
        } else if (Prop->getString()->stringValue().isEqual("bottom")) {
          NewFilm->ScreenEdgeVertical = SCREEN_EDGE_BOTTOM;
        }
      }

      //default values are centre

      Prop = Dict3->propertyForKey("DistanceFromScreenEdgeX%");
      NewFilm->FilmX = GetPropertyAsInteger(Prop, INITVALUE);

      Prop = Dict3->propertyForKey("DistanceFromScreenEdgeY%");
      NewFilm->FilmY = GetPropertyAsInteger(Prop, INITVALUE);

      Prop = Dict3->propertyForKey("NudgeX");
      NewFilm->NudgeX = GetPropertyAsInteger(Prop, INITVALUE);

      Prop = Dict3->propertyForKey("NudgeY");
      NewFilm->NudgeY = GetPropertyAsInteger(Prop, INITVALUE);

      Prop = Dict3->propertyForKey("Once");
      NewFilm->RunOnce = IsPropertyNotNullAndTrue(Prop);

      NewFilm->GetFrames(*ThemeX); //used properties: ID, Path, NumFrames
      Cinema.AddFilm(NewFilm);
 //     delete NewFilm; //looks like already deleted
    }
  }

//not sure if it needed
  if (BackgroundName.isEmpty()) {
    BackgroundName.takeValueFrom("background");
  }
  if (BannerFileName.isEmpty()) {
    BannerFileName.takeValueFrom("logo");
  }
  if (SelectionSmallFileName.isEmpty()) {
    SelectionSmallFileName.takeValueFrom("selection_small");
  }
  if (SelectionBigFileName.isEmpty()) {
    SelectionBigFileName.takeValueFrom("selection_big");
  }
  if (SelectionIndicatorName.isEmpty()) {
    SelectionIndicatorName.takeValueFrom("selection_indicator");
  }
  if (FontFileName.isEmpty()) {
    FontFileName.takeValueFrom("font");
  }

  return EFI_SUCCESS;
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

static XImage NullImage;
static XIcon DummyIcon;
static XIcon NullIcon;

const XIcon& XTheme::GetIcon(const XString8& Name)
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

XBool XTheme::CheckNative(INTN Id)
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

const XIcon& XTheme::GetIcon(INTN Id)
{
  return GetIconAlt(Id, -1);
}

/*
 * Get Icon with this ID=id, for example VOL_INTERNAL_HFS
 * if not found then search for ID=Alt with Native attribute set, for example VOL_INTERNAL
 * if not found then check embedded with ID=Id
 * if not found then check embedded with ID=Alt
 */
XIcon& XTheme::GetIconAlt(INTN Id, INTN Alt) //if not found then take embedded
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
      XIcon *NewIcon = new XIcon(Id, true);
      if (NewIcon->Image.isEmpty()) {
        // check for embedded with ID=Alt
        delete NewIcon;
        NewIcon = new XIcon(Alt, true);
      }
      if (!NewIcon->Image.isEmpty()) {
        // using Embedded icon
        Icons[IdFound].Image = NewIcon->Image;
        Icons[IdFound].ImageNight = NewIcon->ImageNight;
      }
      delete NewIcon; // there is probably a way better way to do this. By initializing directly Icons[IdFound].Image instead of using dynamically allocated copy.
    }
  }

  if (IdFound >= 0 && !Icons[IdFound].Image.isEmpty()) {
    // icon not empty, return it
//    if (!Daylight && !Icons[IdFound].ImageNight.isEmpty()) {
//      DBG("got night icon %lld name{%s}\n", Id, IconsNames[IdFound]);
//      return Icons[IdFound].ImageNight;
//    }
    //if daylight or night icon absent
//    DBG("got day icon %lld name{%s}\n", Id, IconsNames[IdFound]);
//    return Icons[IdFound].Image;
    return Icons[IdFound]; //check daylight at draw
  }
  return NullIcon; //such Id is not found in the database
}

const XIcon& XTheme::LoadOSIcon(const CHAR16* OSIconName)
{
  return LoadOSIcon(XString8().takeValueFrom(OSIconName));
}

const XIcon& XTheme::LoadOSIcon(const XString8& Full)
{
  // input value can be L"win", L"ubuntu,linux", L"moja,mac" set by GetOSIconName (OSVersion)
  XString8 First;
  XString8 Second;
  XString8 Third;
  const XIcon *ReturnIcon;
  UINTN Comma = Full.indexOf(',');
  UINTN Size = Full.length();
  DBG("      IconName=%s comma=%lld size=%lld\n", Full.c_str(), Comma, Size);
  if (Comma != MAX_XSIZE) {  //Comma
    First = "os_"_XS8 + Full.subString(0, Comma);
    ReturnIcon = &GetIcon(First);
    DBG("      first=%s\n", First.c_str());
    if (!ReturnIcon->isEmpty()) return *ReturnIcon;
    //else search second name
    Second = "os_"_XS8 + Full.subString(Comma + 1, MAX_XSIZE);
    //moreover names can be triple L"chrome,grub,linux"
    UINTN SecondComma = Second.indexOf(',');
    if (SecondComma == MAX_XSIZE) {
      ReturnIcon = &GetIcon(Second);
      if (!ReturnIcon->isEmpty()) return *ReturnIcon;
    } else {
      First = Second.subString(0, SecondComma);
      ReturnIcon = &GetIcon(First);
      if (!ReturnIcon->isEmpty()) return *ReturnIcon;
      Third = "os_"_XS8 + Second.subString(SecondComma + 1, MAX_XSIZE);
      ReturnIcon = &GetIcon(Third);
      if (!ReturnIcon->isEmpty()) return *ReturnIcon;
    }
    DBG("      Second=%s\n", Second.c_str());
    if (!ReturnIcon->isEmpty()) return *ReturnIcon;
  } else {
    ReturnIcon = &GetIcon("os_"_XS8 + Full);
    DBG("      Full=%s\n", Full.c_str());
    if (!ReturnIcon->isEmpty()) return *ReturnIcon;
  }
  if ( Full !="unknown"_XS8 ) {
    return LoadOSIcon("unknown"_XS8);
  }else{
    // else something
    if (DummyIcon.isEmpty()) { //initialize once per session
      DummyIcon.Image.DummyImage(MainEntriesSize);
    }
  }
  return DummyIcon;
}


void XTheme::FillByEmbedded()
{
  embedded = true;
  Theme.takeValueFrom("embedded");
  SelectionColor = 0xA0A0A080;
  SelectionBackgroundPixel = { 0xa0, 0xa0, 0xa0, 0x80 };

  Icons.setEmpty();
  for (INTN i = 0; i < BUILTIN_ICON_COUNT; ++i) { //this is embedded icon count
    XIcon* NewIcon = new XIcon(i, true);
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
  if ( UGAWidth == 0 || UGAHeight == 0 ) {
      // jief : I had the case where no graphic protocol were availbale. So UGAWidth == 0 && UGAHeight == 0 which panic because Background would be empty.
      //        Background.GetPixelPtr(0, 0) panic on an empty image because there is no pixel at x=0 y=0
      // test could be "if ( UGAWidth == 0 && UGAHeight == 0 )" (&& instead of ||), but I feel like if one of the dimension is 0, we have a graphic problem.
      // also, should we implement the case if we are in text mode ?
      return;
  }
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL FirstBannerPixel = MenuBackgroundPixel;
  if (BanHeight < 2) {
    BanHeight = ((UGAHeight - (int)(LayoutHeight * Scale)) >> 1);
  }
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
//      DBG("crop to x,y: %lld, %lld\n", x, y);
      Background.CopyRect(BigBack, BackRect, BigRect);
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
      //DBG("Assign Background = BigBack.  BigBack.Width=%lld  BigBack.Height=%lld\n", BigBack.GetWidth(), BigBack.GetHeight());
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


//use this only for PNG theme
void XTheme::FillByDir() //assume ThemeDir is defined by InitTheme() procedure
{
  EFI_STATUS Status;
  Icons.setEmpty();
  for (INTN i = 0; i < IconsNamesSize; ++i) { //scan full table
    Status = EFI_NOT_FOUND;
    XIcon* NewIcon = new XIcon(i); //initialize without embedded
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
      NewIcon->ImageNight.LoadXImage(ThemeDir, SWPrintf("%s_night", IconsNames[i].c_str()));
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
    XIcon *NewIcon = new XIcon(BUILTIN_ICON_SELECTION);
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
//TODO - make them XIcon
  SelectionImages[2] = GetIcon(BUILTIN_SELECTION_SMALL).GetBest(!Daylight);
  SelectionImages[0] = GetIcon(BUILTIN_SELECTION_BIG).GetBest(!Daylight);
  if (BootCampStyle) {
    SelectionImages[4] = GetIcon(BUILTIN_ICON_SELECTION).GetBest(!Daylight);
  }

  //and buttons
  Buttons[0] = GetIcon(BUILTIN_RADIO_BUTTON).GetBest(!Daylight);
  Buttons[1] = GetIcon(BUILTIN_RADIO_BUTTON_SELECTED).GetBest(!Daylight);
  Buttons[2] = GetIcon(BUILTIN_CHECKBOX).GetBest(!Daylight);
  Buttons[3] = GetIcon(BUILTIN_CHECKBOX_CHECKED).GetBest(!Daylight);

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
    ScrollbarBackgroundImage = GetIcon("scrollbar_background"_XS8).GetBest(!Daylight);
    BarStartImage.setEmpty();
    BarEndImage.setEmpty();
    ScrollbarImage = GetIcon("scrollbar_holder"_XS8).GetBest(!Daylight); //"_night" is already accounting
    ScrollStartImage = GetIcon("scrollbar_start"_XS8).GetBest(!Daylight);
    ScrollEndImage = GetIcon("scrollbar_end"_XS8).GetBest(!Daylight);
    UpButtonImage = GetIcon("scrollbar_up_button"_XS8).GetBest(!Daylight);
    DownButtonImage = GetIcon("scrollbar_down_button"_XS8).GetBest(!Daylight);
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
void XTheme::FillRectAreaOfScreen(IN INTN XPos, IN INTN YPos, IN INTN Width, IN INTN Height)
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


