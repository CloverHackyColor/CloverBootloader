//
//  XCinema.cpp
//  Clover
//
//  Created by Sergey Isakov on 09/04/2020.
//  Copyright © 2020 Slice. All rights reserved.
//


#include "libegint.h"
#include "XCinema.h"
//#include "../gui/REFIT_MENU_SCREEN.h"
#include "../libeg/XTheme.h"
#include "../refit/lib.h"

#ifndef DEBUG_ALL
#define DEBUG_CINEMA 0
#else
#define DEBUG_CINEMA DEBUG_ALL
#endif

#if DEBUG_CINEMA == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_CINEMA, __VA_ARGS__)
#endif


FILM* XCinema::GetFilm(INTN Id)
{
//  DBG("ask film %lld from total of %lld\n", Id, Cinema.size());
  for (size_t i = 0; i < Cinema.size(); ++i) {
//    DBG("check film# %lld\n", Cinema[i].GetIndex());
    if (Cinema[i].GetIndex() == Id) {
//      DBG("   found ID\n");
      return &Cinema[i];
    }
  }
  return nullptr;
}

void XCinema::AddFilm(FILM* NewFilm)
{
  Cinema.AddReference(NewFilm, true);
}

static XImage NullImage;
const XImage& FILM::GetImage(INTN Index) const
{
  DBG("ask for frame #%lld from total of %zu\n", Index, Frames.size());
  for (size_t i = 0; i < Frames.size(); ++i) {
    if (Frames[i].getIndex() == Index) {
      DBG("...found\n");
      return Frames[i].getImage();
    }
  }
  DBG("...not found\n");
  return NullImage;
}

const XImage& FILM::GetImage(bool *free) const
{
  /*
   * for SVG anime we have to generate new XImage using CurrentFrame as an argument
    product(IconToAnime.ImageSVG, CurrentFrame, method); -- ImageSVG will be changed?
   or
    XImage *frame = IconToAnime.GetBest(!Daylight, free, CurrentFrame, method);
    
   return frame;
   *
   */
  for (size_t i = 0; i < Frames.size(); ++i) {
    if (Frames[i].getIndex() == CurrentFrame) {
      if (free) *free = false;
      return Frames[i].getImage();
    }
  }
  if (free) *free = false;
  return NullImage;
}

void FILM::AddFrame(XImage* Frame, INTN Index)
{
  IndexedImage* NewFrame = new IndexedImage(Index);
  NewFrame->setImage(*Frame);
  Frames.AddReference(NewFrame, true);
  DBG("index=%lld last=%lld\n", Index, LastIndex);
  if (Index > LastIndex) {
    LastIndex = Index;
  }
}


void FILM::GetFrames(XTheme& TheTheme /*, const XStringW& Path*/) // Path already exist as a member. Is it the same ?
{
  const EFI_FILE *ThemeDir = &TheTheme.getThemeDir();
  EFI_STATUS Status;
  LastIndex = 0;
  for (INTN Index = 0; Index < NumFrames; Index++) {
    XImage NewImage;
    Status = EFI_NOT_FOUND;
    if (TheTheme.TypeSVG) {
      Status = TheTheme.LoadSvgFrame(Index, &NewImage);
    } else {
      XStringW Name = SWPrintf("%ls\\%ls_%03lld.png", Path.wc_str(), Path.wc_str(), Index);
 //     DBG("try to load %ls\n", Name.wc_str()); //fine
      if (FileExists(ThemeDir, Name)) {
        Status = NewImage.LoadXImage(ThemeDir, Name);
      }
//      DBG("  read status=%s\n", efiStrError(Status));
    }
    if (!EFI_ERROR(Status)) {
      AddFrame(&NewImage, Index);
    }
  }
}


