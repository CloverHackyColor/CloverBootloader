//
//  XCinema.hpp
//  Clover
//
//  Created by Sergey Isakov on 09/04/2020.
//  Copyright © 2020 Slice. All rights reserved.
//

#ifndef XCinema_h
#define XCinema_h

extern "C" {
#include <Protocol/GraphicsOutput.h>
}
#include "../cpp_foundation/XArray.h"
#include "../cpp_foundation/XString.h"
#include "../libeg/libeg.h"
#include "XImage.h"
#include "XTheme.h"

class XTheme;

class FILM
{
protected:
  INTN      Id; //ScreenID, enumeration value but keep it to be int for extensibility
public:
  //I see no reason to make they protected
  BOOLEAN   RunOnce;
  INTN      NumFrames; //set by user in Theme.plist or in Theme.svg
  INTN      FrameTime; //usually 50, 100, 200 ms
  INTN      FilmX, FilmY;  //relative
  INTN      ScreenEdgeHorizontal;
  INTN      ScreenEdgeVertical;
  INTN      NudgeX, NudgeY;
  XStringW  Path; //user defined name for folder and files Path/Path_002.png etc
  BOOLEAN   AnimeRun;
  UINT64    LastDraw;

protected:
  XObjArray<IndexedImage> Frames; //Frames can be not sorted
  INTN      LastIndex; // it is not Frames.size(), it is last index inclusive, so frames 0,1,2,5,8 be LastIndex = 8
  INTN      CurrentFrame; // must be unique for each film

public:
  EG_RECT FilmPlace;  // Screen has several Films each in own place

public:
  FILM() : Id(0), RunOnce(0), NumFrames(0), FrameTime(0), FilmX(0), FilmY(0), ScreenEdgeHorizontal(0), ScreenEdgeVertical(0),
           NudgeX(0), NudgeY(0), Path(), AnimeRun(0), LastDraw(0), Frames(), LastIndex(0), CurrentFrame(0), FilmPlace()
         {}
  FILM(INTN Id) : Id(Id), RunOnce(0), NumFrames(0), FrameTime(0), FilmX(0), FilmY(0), ScreenEdgeHorizontal(0), ScreenEdgeVertical(0),
           NudgeX(0), NudgeY(0), Path(), AnimeRun(0), LastDraw(0), Frames(), LastIndex(0), CurrentFrame(0), FilmPlace()
         {}
  ~FILM() {}

  INTN GetIndex() { return  Id; }
  void SetIndex(INTN Index) { Id = Index; }

  const XImage& GetImage(INTN Index) const;
  const XImage& GetImage(bool *free = nullptr) const;
  void AddFrame(XImage* Frame, INTN Index); //IndexedImage will be created
  size_t Size() { return Frames.size(); }
  INTN LastFrameID() { return LastIndex; }
  bool Finished() { return CurrentFrame == 0; }
  void GetFrames(XTheme& TheTheme/*, const XStringW& Path*/); //read image sequence from Theme/Path/
  void SetPlace(const EG_RECT& Rect) { FilmPlace = Rect; }
  void Advance() { ++CurrentFrame %= (LastIndex + 1); }
  void Reset() { CurrentFrame = 0; }
//  EFI_STATUS GetFrame(IN INTN Index, OUT XImage *Frame); //usually Index=CurrentFrame
//  EFI_STATUS GetFrame(OUT XImage *Frame); 
  
};

//initially it was supposed to be one anime per one REFIT_SCREEN
// but this leads to large delays switching screens to initialize Film sequence again and again
// otherwise anime depends on theme and should be a member of XTheme
// then it should contain Screen->ID for each film
// but for next future we want to have other animated images in addition to screen->titleimage
// so let it be frames arrays each with own purpose (Id)
//
// XTheme contains Cinema which is an array of FILMs
// Each Screen contains a pointer to a FILM. And moreover titleFilm, or BackgroundFilm or even entryFilm
// Next problem is a timeout between frames.
// A theme contains images with indexes 1,2,5,6 for one Id.
// This Id contains fixed timeout between frames. Then next updateAnime Index will be compared with current tick
// if yes then update. Static index?
//
// in the far future I'll plan to make dynamic SVG: parse SVGIcon with a variable argument
// and then rasterize it. Real SVG contains constants only so it will be new dynamicSVG.
// then Entry->Image should be reparsed each time it created or contains flag to update every frameTime

class XCinema
{
  protected:
  XObjArray<FILM> Cinema;

  public:
  XCinema() : Cinema() {}
  ~XCinema() {}

  FILM* GetFilm(INTN Id);
  void AddFilm(FILM* NewFilm);
  void setEmpty() { Cinema.setEmpty(); }
};

#endif /* XCinema_h */
