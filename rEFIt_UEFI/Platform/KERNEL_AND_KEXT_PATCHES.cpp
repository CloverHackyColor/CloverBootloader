/*
 * KERNEL_AND_KEXT_PATCHES.cpp
 *
 *  Created on: 4 Feb 2021
 *      Author: jief
 */

#include "KERNEL_AND_KEXT_PATCHES.h"
#include "MacOsVersion.h"

bool ABSTRACT_PATCH::IsPatchEnabledByBuildNumber(const XString8& Build)
{
  BOOLEAN ret = FALSE;

  if (MatchBuild.isEmpty() || Build.isEmpty()) {
    return TRUE; //undefined matched corresponds to old behavior
  }

  XString8Array mos = Split<XString8Array>(MatchBuild, ","_XS8).trimEachString();

  if ( mos[0] == "All"_XS8) {
    return TRUE;
  }

  for (size_t i = 0; i < mos.size(); ++i) {
    // dot represent MatchOS
    MacOsVersion mosv = mos[i];
    if ( mos[i].contains(Build) ) { // MatchBuild
      //DBG("\nthis patch will activated for OS %ls!\n", mos->array[i]);
      ret =  TRUE;
      break;
    }
  }
  return ret;
}


bool ABSTRACT_PATCH::IsPatchEnabled(const MacOsVersion& CurrOS)
{
  BOOLEAN ret = FALSE;

  if (MatchOS.isEmpty() || CurrOS.isEmpty()) {
    return TRUE; //undefined matched corresponds to old behavior
  }

  XString8Array mos = Split<XString8Array>(MatchOS, ","_XS8).trimEachString();

  if ( mos[0] == "All"_XS8) {
    return TRUE;
  }

  for (size_t i = 0; i < mos.size(); ++i) {
    // dot represent MatchOS
    MacOsVersion mosv = mos[i];
    if ( CurrOS.match(mos[i]) ) {
      //DBG("\nthis patch will activated for OS %ls!\n", mos->array[i]);
      ret =  TRUE;
      break;
    }
  }
  return ret;
}


//
//bool KERNEL_AND_KEXT_PATCHES::IsPatchEnabledByBuildNumber(const XString8& Build)
//{
//  BOOLEAN ret = FALSE;
//
//  if (MatchBuild.isEmpty() || Build.isEmpty()) {
//    return TRUE; //undefined matched corresponds to old behavior
//  }
//
//  XString8Array mos = Split<XString8Array>(MatchBuild, ","_XS8).trimEachString();
//  
//  if ( mos[0] == "All"_XS8) {
//    return TRUE;
//  }
//
//  for (size_t i = 0; i < mos.size(); ++i) {
//    // dot represent MatchOS
//    MacOsVersion mosv = mos[i];
//    if ( mos[i].contains(Build) ) { // MatchBuild
//      //DBG("\nthis patch will activated for OS %ls!\n", mos->array[i]);
//      ret =  TRUE;
//      break;
//    }
//  }
//  return ret;
//}
