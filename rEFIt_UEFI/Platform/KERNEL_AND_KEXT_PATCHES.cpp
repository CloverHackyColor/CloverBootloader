/*
 * KERNEL_AND_KEXT_PATCHES.cpp
 *
 *  Created on: 4 Feb 2021
 *      Author: jief
 */

#include "KERNEL_AND_KEXT_PATCHES.h"
#include "MacOsVersion.h"

XBool ABSTRACT_PATCH::IsPatchEnabledByBuildNumber(const XString8& Build)
{
  XBool ret = false;

  if (MatchBuild.isEmpty() || Build.isEmpty()) {
    return true; //undefined matched corresponds to old behavior
  }

  XString8Array mos = Split<XString8Array>(MatchBuild, ","_XS8).trimEachString();

  if ( mos[0] == "All"_XS8) {
    return true;
  }

  for (size_t i = 0; i < mos.size(); ++i) {
    // dot represent MatchOS
    MacOsVersion mosv = mos[i];
    if ( mos[i].contains(Build) ) { // MatchBuild
      //DBG("\nthis patch will activated for OS %ls!\n", mos->array[i]);
      ret =  true;
      break;
    }
  }
  return ret;
}


XBool ABSTRACT_PATCH::IsPatchEnabled(const MacOsVersion& CurrOS)
{
  XBool ret = false;

  if (MatchOS.isEmpty() || CurrOS.isEmpty()) {
    return true; //undefined matched corresponds to old behavior
  }

  XString8Array mos = Split<XString8Array>(MatchOS, ","_XS8).trimEachString();

  if ( mos[0] == "All"_XS8) {
    return true;
  }

  for (size_t i = 0; i < mos.size(); ++i) {
    // dot represent MatchOS
    MacOsVersion mosv = mos[i];
    if ( CurrOS.match(mos[i]) ) {
      //DBG("\nthis patch will activated for OS %ls!\n", mos->array[i]);
      ret =  true;
      break;
    }
  }
  return ret;
}


//
//XBool KERNEL_AND_KEXT_PATCHES::IsPatchEnabledByBuildNumber(const XString8& Build)
//{
//  XBool ret = false;
//
//  if (MatchBuild.isEmpty() || Build.isEmpty()) {
//    return true; //undefined matched corresponds to old behavior
//  }
//
//  XString8Array mos = Split<XString8Array>(MatchBuild, ","_XS8).trimEachString();
//  
//  if ( mos[0] == "All"_XS8) {
//    return true;
//  }
//
//  for (size_t i = 0; i < mos.size(); ++i) {
//    // dot represent MatchOS
//    MacOsVersion mosv = mos[i];
//    if ( mos[i].contains(Build) ) { // MatchBuild
//      //DBG("\nthis patch will activated for OS %ls!\n", mos->array[i]);
//      ret =  true;
//      break;
//    }
//  }
//  return ret;
//}
