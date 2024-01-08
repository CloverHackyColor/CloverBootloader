#if !defined(__XICON_H__)
#define __XICON_H__

#include "../cpp_foundation/XObjArray.h"
#include "../cpp_foundation/XString.h"
#include "XImage.h"
#include "libeg.h"
#include "nanosvg.h"
#include "../Platform/BootLog.h"
#include "../Platform/Utils.h"

extern CONST LString8 IconsNames[];
extern const INTN IconsNamesSize;


class XIcon
{
public:
  INTN Id = 0;  //for example BUILTIN_ICON_POINTER
  XString8 Name = XString8(); //for example "os_moja", "vol_internal"
  XImage Image = XImage();
  XImage ImageNight = XImage();
  XBool Native = false;
protected:
  XBool Empty = true;
public:
  XIcon() {};
  XIcon(INTN Id, XBool Embedded = false);
  

  XBool isEmpty() const  { return Image.isEmpty()  &&  ImageNight.isEmpty(); }
  void setEmpty()  { Id = 0; Name.setEmpty(); Image.setEmpty(); ImageNight.setEmpty(); Native = false; }
  
  EFI_STATUS LoadXImage(const EFI_FILE *Dir, const XStringW& FileName); //for example LoadImage(ThemeDir, L"icons\\" + Name);
  EFI_STATUS LoadXImage(const EFI_FILE *Dir, const wchar_t* LIconName);
  EFI_STATUS LoadXImage(const EFI_FILE *Dir, const char* IconName);
  void GetEmbedded();

  // Default are not valid, as usual. We delete them. If needed, proper ones can be created
//  Icon(const Icon&) = delete;
  const XImage& GetBest(XBool night) const;
};


#endif
