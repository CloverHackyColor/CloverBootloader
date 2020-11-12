#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "../Platform/MacOsVersion.h"
#include "../cpp_foundation/XStringArray.h"

//
//static BOOLEAN IsOSValid_OLDOLD(const XString8& MatchOS, const XString8& CurrOS)
//{
//  /* example for valid matches are:
//   10.7, only 10.7 (10.7.1 will be skipped)
//   10.10.2 only 10.10.2 (10.10.1 or 10.10.5 will be skipped)
//   10.10.x (or 10.10.X), in this case is valid for all minor version of 10.10 (10.10.(0-9))
//   */
//
//  BOOLEAN ret = FALSE;
//
//  if (MatchOS.isEmpty() || CurrOS.isEmpty()) {
//    return TRUE; //undefined matched corresponds to old behavior
//  }
//
////  osToc = GetStrArraySeparatedByChar(MatchOS, '.');
//  XString8Array macthOsToc = Split<XString8Array>(MatchOS, "."_XS8).trimEachString();
//  XString8Array currOStoc = Split<XString8Array>(CurrOS, "."_XS8).trimEachString();
//
//  if (macthOsToc.size() == 2) {
//    if (currOStoc.size() == 2) {
//      if ( macthOsToc[0] == currOStoc[0] && macthOsToc[1] == currOStoc[1]) {
//        ret = TRUE;
//      }
//    }
//  } else if (macthOsToc.size() == 3) {
//    if (currOStoc.size() == 3) {
//      if ( macthOsToc[0] == currOStoc[0]
//          && macthOsToc[1] == currOStoc[1]
//          && macthOsToc[2] == currOStoc[2]) {
//        ret = TRUE;
//      } else if ( macthOsToc[0] == currOStoc[0]
//                 && macthOsToc[1] == currOStoc[1]
//                 && macthOsToc[2].equalIC("x") ) {
//        ret = TRUE;
//      }
//    } else if (currOStoc.size() == 2) {
//      if ( macthOsToc[0] == currOStoc[0]
//          && macthOsToc[1] ==  currOStoc[1] ) {
//        ret = TRUE;
//      } else if ( macthOsToc[0] == currOStoc[0]
//                 && macthOsToc[1] ==  currOStoc[1]
//                 && macthOsToc[2].equalIC("x") == 0 ) {
//        ret = TRUE;
//      }
//    }
//  }
//  return ret;
//}
//


static BOOLEAN IsOSValid_OLD(const XString8& MatchOS, const XString8& CurrOS)
{
  /* example for valid matches are:
   10.7, only 10.7 (10.7.1 will be skipped)
   10.10.2 only 10.10.2 (10.10.1 or 10.10.5 will be skipped)
   10.10.x (or 10.10.X), in this case is valid for all minor version of 10.10 (10.10.(0-9))
   */

  BOOLEAN ret = FALSE;

  if (MatchOS.isEmpty() || CurrOS.isEmpty()) {
    return TRUE; //undefined matched corresponds to old behavior
  }

//  osToc = GetStrArraySeparatedByChar(MatchOS, '.');
  XString8Array osToc = Split<XString8Array>(MatchOS, "."_XS8).trimEachString();
  XString8Array currOStoc = Split<XString8Array>(CurrOS, "."_XS8).trimEachString();

  if ( osToc.size() > 0 && currOStoc.size() > 0 && osToc[0] == "11"_XS8 && currOStoc[0] == "11"_XS8 ) {
    if (osToc.size() == 1 ) return true;
    if (osToc.size() == 2 ) {
      if ( osToc[1].equalIC("x") ) return true;
      if ( currOStoc.size() == 2 && osToc[1] == currOStoc[1] ) return true;
    }
  }
  if (osToc.size() == 2) {
    if (currOStoc.size() == 2) {
      if ( osToc[0] == currOStoc[0] && osToc[1] == currOStoc[1]) {
        ret = TRUE;
      }
    }
  } else if (osToc.size() == 3) {
    if (currOStoc.size() == 3) {
      if ( osToc[0] == currOStoc[0]
          && osToc[1] == currOStoc[1]
          && osToc[2] == currOStoc[2]) {
        ret = TRUE;
      } else if ( osToc[0] == currOStoc[0]
                 && osToc[1] == currOStoc[1]
                 && osToc[2].equalIC("x") ) {
        ret = TRUE;
      }
    } else if (currOStoc.size() == 2) {
      if ( osToc[0] == currOStoc[0]
          && osToc[1] ==  currOStoc[1] ) {
        ret = TRUE;
      } else if ( osToc[0] == currOStoc[0]
                 && osToc[1] ==  currOStoc[1]
                 && osToc[2].equalIC("x") == 0 ) {
        ret = TRUE;
      }
    }
  }
  return ret;
}

static int breakpoint(int v)
{
  return v;
}

int MacOsVersion_tests()
{
  {
    int vArray[5] = {9, 9, 9, -1, -1};
    MacOsVersion v(vArray);
    if ( v.elementAt(0) != 9 ) return breakpoint(1);
    if ( v.elementAt(1) != 9 ) return breakpoint(2);
    if ( v.elementAt(2) != 9 ) return breakpoint(3);
    if ( v.elementAt(3) != -1 ) return breakpoint(4);
  }
  {
    MacOsVersion v("10.10.10"_XS8);
    if ( v.elementAt(0) != 10 ) return breakpoint(1);
    if ( v.elementAt(1) != 10 ) return breakpoint(2);
    if ( v.elementAt(2) != 10 ) return breakpoint(3);
    if ( v.elementAt(3) != -1 ) return breakpoint(4);
  }

  if (  ! ( MacOsVersion("10.10.10"_XS8) == MacOsVersion("10.10.10"_XS8) )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("10.10.10"_XS8) != MacOsVersion("10.10.10.0"_XS8) )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("10.10.10"_XS8) != MacOsVersion("10.10.9"_XS8) )  ) return breakpoint(21);


  if (  ! ( MacOsVersion("10.10.10"_XS8) <= MacOsVersion("10.10.10"_XS8) )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("10.10.10"_XS8) >= MacOsVersion("10.10.10"_XS8) )  ) return breakpoint(20);

  if (  ! ( MacOsVersion("9.10.10"_XS8) < MacOsVersion("10.10.10"_XS8) )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("10.9.10"_XS8) < MacOsVersion("10.10.10"_XS8) )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("10.10.9"_XS8) < MacOsVersion("10.10.10"_XS8) )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("10.10.10"_XS8) < MacOsVersion("10.10.10.0"_XS8) )  ) return breakpoint(20);

  if (  ! ( MacOsVersion("9.10.10"_XS8) <= MacOsVersion("10.10.10"_XS8) )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("10.9.10"_XS8) <= MacOsVersion("10.10.10"_XS8) )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("10.10.9"_XS8) <= MacOsVersion("10.10.10"_XS8) )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("10.10.10"_XS8) <= MacOsVersion("10.10.10.0"_XS8) )  ) return breakpoint(20);

  if (  ! ( MacOsVersion("10.10.10"_XS8) > MacOsVersion("9.10.10"_XS8) )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("10.10.10"_XS8) > MacOsVersion("10.9.10"_XS8) )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("10.10.10"_XS8) > MacOsVersion("10.10.9"_XS8) )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("10.10.10.0"_XS8) > MacOsVersion("10.10.10"_XS8) )  ) return breakpoint(20);

  if (  ! ( MacOsVersion("10.10.10"_XS8) >= MacOsVersion("9.10.10"_XS8) )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("10.10.10"_XS8) >= MacOsVersion("10.9.10"_XS8) )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("10.10.10"_XS8) >= MacOsVersion("10.10.9"_XS8) )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("10.10.10.0"_XS8) >= MacOsVersion("10.10.10"_XS8) )  ) return breakpoint(20);

  if (  ! ( MacOsVersion() == MacOsVersion() ) == true  ) return breakpoint(20);
  if (  ! ( MacOsVersion() != MacOsVersion() ) == false  ) return breakpoint(20);
  if (  ! ( MacOsVersion() < MacOsVersion() ) == false  ) return breakpoint(20);
  if (  ! ( MacOsVersion() <= MacOsVersion() ) == true  ) return breakpoint(20);
  if (  ! ( MacOsVersion() > MacOsVersion() ) == false  ) return breakpoint(20);
  if (  ! ( MacOsVersion() >= MacOsVersion() ) == true  ) return breakpoint(20);
  if (  ! ( MacOsVersion() < MacOsVersion("10.10.10"_XS8) ) == true  ) return breakpoint(20);
  if (  ! ( MacOsVersion() <= MacOsVersion("10.10.10"_XS8) ) == true  ) return breakpoint(20);
  if (  ! ( MacOsVersion() > MacOsVersion("10.10.10"_XS8) ) == false  ) return breakpoint(20);
  if (  ! ( MacOsVersion() >= MacOsVersion("10.10.10"_XS8) ) == false  ) return breakpoint(20);


  if (  ! ( MacOsVersion().asString() == ""_XS8 )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("1"_XS8).asString() == "1"_XS8 )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("1.2"_XS8).asString() == "1.2"_XS8 )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("1.2.3"_XS8).asString() == "1.2.3"_XS8 )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("10.10.10.0"_XS8).asString() == "10.10.10.0"_XS8 )  ) return breakpoint(20);

  if (  ! ( MacOsVersion("10.10.10.0"_XS8).asString(1) == "10"_XS8 )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("10.10.10.0"_XS8).asString(2) == "10.10"_XS8 )  ) return breakpoint(20);
  if (  ! ( MacOsVersion("10.10.10.0"_XS8).asString(3) == "10.10.10"_XS8 )  ) return breakpoint(20);


  if ( ! ( IsOSValid_OLD("10"_XS8,     "10"_XS8    ) == false ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("10"_XS8,     "10.1"_XS8  ) == false ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("10"_XS8,     "10.1.2"_XS8) == false ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("10.1"_XS8,   "10"_XS8    ) == false ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("10.1"_XS8,   "10.1"_XS8  ) == true  ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("10.1"_XS8,   "10.1.2"_XS8) == false ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("10.x"_XS8,   "10"_XS8    ) == false ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("10.x"_XS8,   "10.1"_XS8  ) == false ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("10.x"_XS8,   "10.1.2"_XS8) == false ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("10.1.2"_XS8, "10"_XS8    ) == false ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("10.1.2"_XS8, "10.1"_XS8  ) == true  ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("10.1.2"_XS8, "10.1.2"_XS8) == true  ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("10.1.x"_XS8, "10"_XS8    ) == false ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("10.1.x"_XS8, "10.1"_XS8  ) == true  ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("10.1.x"_XS8, "10.1.2"_XS8) == true  ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("10.x.2"_XS8, "10"_XS8    ) == false ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("10.x.2"_XS8, "10.1"_XS8  ) == false ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("10.x.2"_XS8, "10.1.2"_XS8) == false ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("10.x.x"_XS8, "10"_XS8    ) == false ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("10.x.x"_XS8, "10.1"_XS8  ) == false ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("10.x.x"_XS8, "10.1.2"_XS8) == false ) ) return breakpoint(100);

  if ( ! ( IsOSValid_OLD("11"_XS8, "11.1"_XS8) == true ) ) return breakpoint(100); // jief : mistake?
  if ( ! ( IsOSValid_OLD("11.1"_XS8, "11.1"_XS8) == true ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("11.1"_XS8, "11.1.2"_XS8) == false ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("11.x"_XS8, "11.1.2"_XS8) == true ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("11.1.2"_XS8, "11.1"_XS8) == true ) ) return breakpoint(100); // I beleive this is a mistake.
  if ( ! ( IsOSValid_OLD("11.1.x"_XS8, "11.1"_XS8) == true ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("11.1.2"_XS8, "11.1.2"_XS8) == true ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("11.1.x"_XS8, "11.1.2"_XS8) == true ) ) return breakpoint(100);
  if ( ! ( IsOSValid_OLD("11"_XS8, "11.1.2"_XS8) == true ) ) return breakpoint(100); // jief : mistake?
  if ( ! ( IsOSValid_OLD("11.1"_XS8, "11.1.2"_XS8) == false ) ) return breakpoint(100);


  if ( ! ( MacOsVersion("10"_XS8    ).match(MacOsVersionPattern("10"_XS8)) == true  ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10.1"_XS8  ).match(MacOsVersionPattern("10"_XS8)) == false ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10.1.2"_XS8).match(MacOsVersionPattern("10"_XS8)) == false ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10"_XS8    ).match(MacOsVersionPattern("10.1"_XS8)) == false  ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10.1"_XS8  ).match(MacOsVersionPattern("10.1"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10.1.2"_XS8).match(MacOsVersionPattern("10.1"_XS8)) == false ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10"_XS8    ).match(MacOsVersionPattern("10.x"_XS8)) == true  ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10.1"_XS8  ).match(MacOsVersionPattern("10.x"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10.1.2"_XS8).match(MacOsVersionPattern("10.x"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10"_XS8    ).match(MacOsVersionPattern("10.1.2"_XS8)) == false  ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10.1"_XS8  ).match(MacOsVersionPattern("10.1.2"_XS8)) == false ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10.1.2"_XS8).match(MacOsVersionPattern("10.1.2"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10"_XS8    ).match(MacOsVersionPattern("10.1.x"_XS8)) == false  ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10.1"_XS8  ).match(MacOsVersionPattern("10.1.x"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10.1.2"_XS8).match(MacOsVersionPattern("10.1.x"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10"_XS8    ).match(MacOsVersionPattern("10.x.2"_XS8)) == false  ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10.1"_XS8  ).match(MacOsVersionPattern("10.x.2"_XS8)) == false ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10.1.2"_XS8).match(MacOsVersionPattern("10.x.2"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10"_XS8    ).match(MacOsVersionPattern("10.x.x"_XS8)) == false  ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10.1"_XS8  ).match(MacOsVersionPattern("10.x.x"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10.1.2"_XS8).match(MacOsVersionPattern("10.x.x"_XS8)) == true ) ) return breakpoint(100);

  if ( ! ( MacOsVersion("11"_XS8    ).match(MacOsVersionPattern("11"_XS8)) == true  ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("11.1"_XS8  ).match(MacOsVersionPattern("11"_XS8)) == false ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("11.1.2"_XS8).match(MacOsVersionPattern("11"_XS8)) == false ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("11"_XS8    ).match(MacOsVersionPattern("11.1"_XS8)) == false  ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("11.1"_XS8  ).match(MacOsVersionPattern("11.1"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("11.1.2"_XS8).match(MacOsVersionPattern("11.1"_XS8)) == false ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("11"_XS8    ).match(MacOsVersionPattern("11.x"_XS8)) == true  ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("11.1"_XS8  ).match(MacOsVersionPattern("11.x"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("11.1.2"_XS8).match(MacOsVersionPattern("11.x"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("11"_XS8    ).match(MacOsVersionPattern("11.1.2"_XS8)) == false  ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("11.1"_XS8  ).match(MacOsVersionPattern("11.1.2"_XS8)) == false ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("11.1.2"_XS8).match(MacOsVersionPattern("11.1.2"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("11"_XS8    ).match(MacOsVersionPattern("11.1.x"_XS8)) == false  ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("11.1"_XS8  ).match(MacOsVersionPattern("11.1.x"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("11.1.2"_XS8).match(MacOsVersionPattern("11.1.x"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("11"_XS8    ).match(MacOsVersionPattern("11.x.2"_XS8)) == false  ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("11.1"_XS8  ).match(MacOsVersionPattern("11.x.2"_XS8)) == false ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("11.1.2"_XS8).match(MacOsVersionPattern("11.x.2"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("11"_XS8    ).match(MacOsVersionPattern("11.x.x"_XS8)) == false  ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("11.1"_XS8  ).match(MacOsVersionPattern("11.x.x"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("11.1.2"_XS8).match(MacOsVersionPattern("11.x.x"_XS8)) == true ) ) return breakpoint(100);



  if ( ! ( MacOsVersion("10.1.2"_XS8).match(MacOsVersionPattern("x"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10.1.2"_XS8).match(MacOsVersionPattern("x.x"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10.1.2"_XS8).match(MacOsVersionPattern("x.1.2"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10.1.2"_XS8).match(MacOsVersionPattern("x.x.2"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10.1.2"_XS8).match(MacOsVersionPattern("xxX.XXx.2"_XS8)) == true ) ) return breakpoint(100);
  if ( ! ( MacOsVersion("10.1.2"_XS8).match(MacOsVersionPattern("xx.xx.XX.XX"_XS8)) == true ) ) return breakpoint(100);


	return 0;
}
