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

//Screen.UpdateAnime(); called from Menu cycle wait for event
//Now = AsmReadTsc();
//if (TimeDiff(LastDraw, Now) < FrameTime) return;
//if (Film[CurrentFrame]) { Draw }
// else skip draw
//  CurrentFrame++;
//if (CurrentFrame >= Frames) {
//  AnimeRun = !Once;
//  CurrentFrame = 0;
//}
//LastDraw = Now;

// object XCinema::Cinema is a part of Theme
// object FILM::FilmX is a part or current Screen. Must be initialized from Cinema somewhere on Screen init
#if XCINEMA
VOID REFIT_MENU_SCREEN::UpdateFilm()
{
  // here we propose each screen has own link to a Film
  INT64      Now = AsmReadTsc();

  if (LastDraw == 0) {
    //save background into special place
    FilmPlaceImage.GetArea(FilmC->FilmPlace);
  }

  if (TimeDiff(LastDraw, Now) < (UINTN)FilmC->FrameTime) return;

  XImage Frame = FilmC->GetImage(); //take current image
  if (!Frame.isEmpty()) {
    Frame.DrawOnBack(FilmC->FilmPlace.XPos, FilmC->FilmPlace.YPos, FilmPlaceImage);
  }
  FilmC->Advance(); //next frame no matter if previous was not found
  if (FilmC->Finished()) { //first loop finished
    AnimeRun = !FilmC->RunOnce; //will stop anime if it set as RunOnce
  }
  LastDraw = Now;
}
#endif
FILM* XCinema::GetFilm(INTN Id)
{
  for (size_t i = 0; i < Cinema.size(); ++i) {
    if (Cinema[i].Id == Id) {
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
  for (size_t i = 0; i < Frames.size(); ++i) {
    if (Frames[i].Id == Index) {
      return Frames[i].getImage();
    }
  }
  return NullImage;
}

const XImage& FILM::GetImage() const
{
  for (size_t i = 0; i < Frames.size(); ++i) {
    if (Frames[i].Id == CurrentFrame) {
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
  if (Index > LastIndex) {
    LastIndex = Index;
  }
}

void FILM::GetFrames(XTheme& TheTheme, const XStringW& Path)
{
  EFI_FILE *ThemeDir = TheTheme.ThemeDir;
  EFI_STATUS Status;
  for (INTN Index = 0; Index < NumFrames; Index++) {
    XImage NewImage;
    Status = EFI_NOT_FOUND;
    if (TheTheme.TypeSVG) {
      Status = TheTheme.LoadSvgFrame(Index, &NewImage);
    } else {
      XStringW Name = SWPrintf("%ls\\%ls_%03lld.png", Path.wc_str(), Path.wc_str(), Index);
      if (FileExists(ThemeDir, Name.wc_str())) {
        Status = NewImage.LoadXImage(ThemeDir, Name);
      }
    }
    if (!EFI_ERROR(Status)) {
      AddFrame(&NewImage, Index);
    }
  }
}
