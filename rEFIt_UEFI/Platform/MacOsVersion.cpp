/*
  Some helper string functions
  JrCs 2014
*/

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "../include/OSTypes.h"
#include "../cpp_foundation/XString.h"
#include "MacOsVersion.h"

MacOsVersion nullMacOsVersion;


const XString8 getSuffixForMacOsVersion(int LoaderType)
{
  if (OSTYPE_IS_OSX_INSTALLER(LoaderType)) return "install"_XS8;
  if (OSTYPE_IS_OSX_RECOVERY(LoaderType)) return "recovery"_XS8;
  if (OSTYPE_IS_OSX(LoaderType)) return "normal"_XS8;
  return NullXString8;
}
