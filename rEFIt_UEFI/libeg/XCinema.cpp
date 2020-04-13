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
  UINT64      Now = AsmReadTsc();

  if (LastDraw == 0) {
    //save background into last frame
  }

  if (TimeDiff(LastDraw, Now) < FrameTime) return;
  XImage *Frame = nullptr; // a link to frame needed
//  EFI_STATUS Status = ThemeX.Cinema.GetFrame(CurrentFrame);
  EFI_STATUS Status = FilmX->GetFrame(CurrentFrame, Frame); //get a pointer to existing frame
  if (!EFI_ERROR(Status) && Frame != nullptr) {
    Frame->Draw(FilmPlace.XPos, FilmPlace.YPos);
  }
  FilmX->Advance(CurrentFrame); //next frame no matter if previous was not found
  if (CurrentFrame == 0) { //first loop finished
    AnimeRun = !Once; //will stop anime
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

EFI_STATUS FILM::GetFrame(IN INTN Index, OUT XImage *Image)
{
  for (size_t i = 0; i < Frames.size(); ++i) {
    if (Frames[i].Index == Index) {
      Image = &Frames[i].Image;
      return EFI_SUCCESS;
    }
  }
  return EFI_NOT_FOUND;
}
