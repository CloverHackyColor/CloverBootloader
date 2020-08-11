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


//xtheme class
XTheme::XTheme() : Icons(), ThemeDir(0), HideBadges(0), HideUIFlags(0), Font(FONT_ALFA), CharWidth(0), SelectionColor(0), FontFileName(),
                   BannerFileName(), SelectionSmallFileName(), SelectionBigFileName(), SelectionIndicatorName(), DefaultSelection(),
                   BackgroundName(), BackgroundScale(imNone), BackgroundSharp(0), BackgroundDark(0), SelectionOnTop(0), BootCampStyle(0),
                   BadgeOffsetX(0), BadgeOffsetY(0), BadgeScale(0), ThemeDesignWidth(0), ThemeDesignHeight(0), BannerPosX(0), BannerPosY(0),
                   BannerEdgeHorizontal(0), BannerEdgeVertical(0), BannerNudgeX(0), BannerNudgeY(0), VerticalLayout(0), NonSelectedGrey(0),
                   MainEntriesSize(0), TileXSpace(0), TileYSpace(0), Proportional(0), embedded(0), DarkEmbedded(0), TypeSVG(0), Scale(0), CentreShift(0),
                   row0TileSize(0), row1TileSize(0), BanHeight(0), LayoutHeight(0), LayoutBannerOffset(0), LayoutButtonOffset(0), LayoutTextOffset(0),
                   LayoutAnimMoveForMenuX(0), ScrollWidth(0), ScrollButtonsHeight(0), ScrollBarDecorationsHeight(0), ScrollScrollDecorationsHeight(0),
                   FontWidth(0), FontHeight(0), TextHeight(0), Daylight(0), Background(), BigBack(), Banner(), SelectionImages(), Buttons(), ScrollbarBackgroundImage(), BarStartImage(), BarEndImage(),
                   ScrollbarImage(), ScrollStartImage(), ScrollEndImage(), UpButtonImage(), DownButtonImage(), FontImage(), BannerPlace(), Cinema(), SVGParser(0)
{
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

  Proportional = FALSE;
//  ShowOptimus = FALSE;
//  DarkEmbedded = FALSE;  //looks like redundant, we always check Night or Daylight
  TypeSVG = FALSE;
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

XIcon* XTheme::GetIconP(const XString8& Name)
{
  for (size_t i = 0; i < Icons.size(); i++)
  {
    if (Icons[i].Name == Name) //night icon has same name as daylight icon
    {
      return GetIconP(Icons[i].Id);
    }
  }
  return &NullIcon; //if name is not found
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

const XIcon& XTheme::GetIcon(INTN Id)
{
  return GetIconAlt(Id, -1);
}

XIcon* XTheme::GetIconP(INTN Id)
{
  return &GetIconAlt(Id, -1);
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
      Icons[IdFound].setFilled();
    } else {
      // check for embedded with ID=Id
      XIcon *NewIcon = new XIcon(Id, true);
      if (NewIcon->Image.isEmpty()) {
        // check for embedded with ID=Alt
        NewIcon = new XIcon(Alt, true);
      }
      if (!NewIcon->Image.isEmpty()) {
        // using Embedded icon
        Icons[IdFound].Image = NewIcon->Image;
        Icons[IdFound].ImageNight = NewIcon->ImageNight;
        Icons[IdFound].setFilled(); 
      }
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
  DBG("IconName=%s comma=%lld size=%lld\n", Full.c_str(), Comma, Size);
  if (Comma != MAX_XSIZE) {  //Comma
    First = "os_"_XS8 + Full.subString(0, Comma);
    ReturnIcon = &GetIcon(First);
    DBG("  first=%s\n", First.c_str());
    if (!ReturnIcon->isEmpty()) return *ReturnIcon;
    //else search second name
    Second = "os_"_XS8 + Full.subString(Comma + 1, Size - Comma - 1);
    //moreover names can be triple L"chrome,grub,linux"
    UINTN SecondComma = Second.indexOf(',');
    if (Comma == MAX_XSIZE) {
      ReturnIcon = &GetIcon(Second);
      if (!ReturnIcon->isEmpty()) return *ReturnIcon;
    } else {
      First = Second.subString(0, SecondComma);
      ReturnIcon = &GetIcon(First);
      if (!ReturnIcon->isEmpty()) return *ReturnIcon;
      Third = "os_"_XS8 + Second.subString(SecondComma + 1, Size - SecondComma - 1);
      ReturnIcon = &GetIcon(Third);
      if (!ReturnIcon->isEmpty()) return *ReturnIcon;
    }
    DBG("  Second=%s\n", Second.c_str());
    if (!ReturnIcon->isEmpty()) return *ReturnIcon;
  } else {
    ReturnIcon = &GetIcon("os_"_XS8 + Full);
    DBG("  Full=%s\n", Full.c_str());
    if (!ReturnIcon->isEmpty()) return *ReturnIcon;
  }
  // else something
  if (DummyIcon.isEmpty()) { //initialize once per session    
    DummyIcon.Image.DummyImage(MainEntriesSize);
    DummyIcon.setFilled();
  }
  return DummyIcon;
}


void XTheme::FillByEmbedded()
{
  embedded = true;
  Theme.takeValueFrom("embedded");
  SelectionColor = 0xA0A0A080;
  SelectionBackgroundPixel = { 0xa0, 0xa0, 0xa0, 0x80 };

  Icons.Empty();
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


//use this only for PNG theme
void XTheme::FillByDir() //assume ThemeDir is defined by InitTheme() procedure
{
  EFI_STATUS Status;
  Icons.Empty();
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
      NewIcon->setFilled();
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
  SelectionImages[2] = *GetIconP(BUILTIN_SELECTION_SMALL)->GetBest(!Daylight);
  SelectionImages[0] = *GetIconP(BUILTIN_SELECTION_BIG)->GetBest(!Daylight);
  if (BootCampStyle) {
    SelectionImages[4] = *GetIconP(BUILTIN_ICON_SELECTION)->GetBest(!Daylight);
  }

  //and buttons
  Buttons[0] = *GetIconP(BUILTIN_RADIO_BUTTON)->GetBest(!Daylight);
  Buttons[1] = *GetIconP(BUILTIN_RADIO_BUTTON_SELECTED)->GetBest(!Daylight);
  Buttons[2] = *GetIconP(BUILTIN_CHECKBOX)->GetBest(!Daylight);
  Buttons[3] = *GetIconP(BUILTIN_CHECKBOX_CHECKED)->GetBest(!Daylight);

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
    ScrollbarBackgroundImage = *GetIconP("scrollbar_background"_XS8)->GetBest(!Daylight);
    BarStartImage.setEmpty();
    BarEndImage.setEmpty();
    ScrollbarImage = *GetIconP("scrollbar_holder"_XS8)->GetBest(!Daylight); //"_night" is already accounting
    ScrollStartImage = *GetIconP("scrollbar_start"_XS8)->GetBest(!Daylight);
    ScrollEndImage = *GetIconP("scrollbar_end"_XS8)->GetBest(!Daylight);
    UpButtonImage = *GetIconP("scrollbar_up_button"_XS8)->GetBest(!Daylight);
    DownButtonImage = *GetIconP("scrollbar_down_button"_XS8)->GetBest(!Daylight);
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




