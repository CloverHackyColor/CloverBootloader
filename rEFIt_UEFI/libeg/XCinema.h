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
protected:
  INTN Id;  //enumeration value but keep it to be int for extensibility
  INTN FrameTime; //usually 50, 100, 200 ms
  XString Path; //user defined name for folder and files Path/Path_002.png etc
  XArray<FRAME> Frames;
  INTN Count; // it is not Frames.size(), it is last index

public:
  FILM();
  FILM(INTN Id);
  ~FILM();

  const XImage& GetImage(INTN Index);
  void AddImage(const XImage& Image, INTN Index);
  size_t Size() { return Frames.size(); }
  INTN LastFrame() { return Count; }
  void GetFilm(const XStringW& Path); //read from Theme
  
};

//initially it was supposed to be one anime per one REFIT_SCREEN
// but this leads to large delays switching screens to initialize Film sequence again and again
// otherwise anime depends on theme and should be a member or XTheme
// then it should contain Screen->ID for each film
// but for next future we want to have other animated images except screen->titleimage
// so let it be frames arrays each with own purpose (Id)
// XTheme contains Cinema
// Each Screen contains a pointer to FILM. And moreover titleFilm, or BackgroundFilm or even entryFilm
// Next problem is a timeout between frames.
// A theme contains images with indexes 1,2,5,6 for one Id.
// This Id contains fixed timeout between frames
class XCinema
{
  protected:
  XArray<FILM> Cinema;


  public:
  XCinema();
  ~XCinema();

  FILM* GetFilm(INTN Id);
  void AddFilm(const FILM& NewFilm, INTN Id);


};

#endif /* XCinema_h */
