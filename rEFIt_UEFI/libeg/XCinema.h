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
#include "../cpp_foundation/XToolsCommon.h"
#include "../cpp_foundation/XArray.h"
#include "../cpp_foundation/XStringW.h"
#include "../libeg/libeg.h"
#include "XImage.h"

typedef struct FRAME {
  INTN Index;
  XImage Image;
} FRAME;

class FILM
{
public:
  bool RunOnce;
  INTN Id; //ScreenID, enumeration value but keep it to be int for extensibility
protected:

  INTN FrameTime; //usually 50, 100, 200 ms
  XString Path; //user defined name for folder and files Path/Path_002.png etc
  XObjArray<FRAME> Frames; //Frames can be not sorted
  INTN LastIndex; // it is not Frames.size(), it is last index inclusive, so frames 0,1,2,5,8 be LastIndex = 8
  EG_RECT FilmPlace;
//  INTN CurrentFrame; // like a static value will be increased between 0..LastIndex, no it's a part of Screen

public:
  FILM(): RunOnce(false), Id(0), FrameTime(0), Path(), Frames(), LastIndex(0), FilmPlace()
  {}
  FILM(INTN Id) : RunOnce(false), Id(Id), FrameTime(0), Path(), Frames(),
    LastIndex(0), FilmPlace() {}
  ~FILM() {}

  const XImage& GetImage(INTN Index);
  void AddFrame(const FRAME& Frame, INTN Index);
  size_t Size() { return Frames.size(); }
  INTN LastFrame() { return LastIndex; }
  void GetFilm(const XStringW& Path); //read from Theme
  void SetPlace(const EG_RECT& Rect);
  void Advance(INTN& Current) { ++Current %= (LastIndex + 1); }
  EFI_STATUS GetFrame(IN INTN Index, OUT XImage *Frame); //usually Index=CurrentFrame
  EFI_STATUS GetFrame(OUT XImage *Frame); 
  
};

//initially it was supposed to be one anime per one REFIT_SCREEN
// but this leads to large delays switching screens to initialize Film sequence again and again
// otherwise anime depends on theme and should be a member of XTheme
// then it should contain Screen->ID for each film
// but for next future we want to have other animated images in addition to screen->titleimage
// so let it be frames arrays each with own purpose (Id)
// XTheme contains Cinema which is an array of FILMs
// Each Screen contains a pointer to a FILM. And moreover titleFilm, or BackgroundFilm or even entryFilm
// Next problem is a timeout between frames.
// A theme contains images with indexes 1,2,5,6 for one Id.
// This Id contains fixed timeout between frames. Then next updateAnime Index will be compared with current tick
// if yes then update. Static index?
//
// in the far future I'll plan to make dynamic SVG: parse SVGIcon with a variable argument
// and then rasterize it. Real SVG contains constants only so it will be dynamicSVG.
// then Entry->Image should be reparsed each time it created or contains flag to update every frameTime

class XCinema
{
  protected:
  XObjArray<FILM> Cinema;


  public:
  XCinema() {}
  ~XCinema() {}

  FILM* GetFilm(INTN Id);
  void AddFilm(const FILM& NewFilm, INTN Id);


};

#endif /* XCinema_h */
