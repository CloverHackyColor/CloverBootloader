//
//  XCinema.cpp
//  Clover
//
//  Created by Sergey Isakov on 09/04/2020.
//  Copyright © 2020 Slice. All rights reserved.
//


#include "libegint.h"
#include "XCinema.h"
#include "../gui/REFIT_MENU_SCREEN.h"

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


//Screen.UpdateAnime(); called from Menu cycle wait for event

// object XCinema Cinema is a part of Theme
// object FILM* FilmC is a part or current Screen. Must be initialized from Cinema somewhere on Screen init

VOID REFIT_MENU_SCREEN::UpdateFilm()
{
  if (FilmC == nullptr || !FilmC->AnimeRun) {
//    DBG("no anime -> run=%d\n", FilmC->AnimeRun?1:0);
    return;
  }
  // here we propose each screen has own link to a Film
  INT64      Now = AsmReadTsc();

  if (FilmC->LastDraw == 0) {
    DBG("=== Update Film ===\n");
    DBG("FilmX=%lld\n", FilmC->FilmX);
    DBG("ID=%lld\n", FilmC->GetIndex());
    DBG("RunOnce=%d\n", FilmC->RunOnce?1:0);
    DBG("NumFrames=%lld\n", FilmC->NumFrames);
    DBG("FrameTime=%lld\n", FilmC->FrameTime);
    DBG("Path=%ls\n", FilmC->Path.wc_str());
    DBG("LastFrame=%lld\n\n", FilmC->LastFrameID());

  }

  if (TimeDiff(FilmC->LastDraw, Now) < (UINTN)FilmC->FrameTime) return;

  XImage Frame = FilmC->GetImage(); //take current image
  if (!Frame.isEmpty()) {
    Frame.DrawOnBack(FilmC->FilmPlace.XPos, FilmC->FilmPlace.YPos, ThemeX.Background);
  }
  FilmC->Advance(); //next frame no matter if previous was not found
  if (FilmC->Finished()) { //first loop finished
    FilmC->AnimeRun = !FilmC->RunOnce; //will stop anime if it set as RunOnce
  }
  FilmC->LastDraw = Now;
}

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

const XImage& FILM::GetImage() const
{
  for (size_t i = 0; i < Frames.size(); ++i) {
    if (Frames[i].getIndex() == CurrentFrame) {
      return Frames[i].getImage();
    }
  }
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
  EFI_FILE *ThemeDir = TheTheme.ThemeDir;
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
//      DBG("  read status=%s\n", strerror(Status));
    }
    if (!EFI_ERROR(Status)) {
      AddFrame(&NewImage, Index);
    }
  }
}
