/*
 * Self.cpp
 *
 *  Created on: Sep 28, 2020
 *      Author: jief
 */

#include <Platform.h>
#include "SelfOem.h"
#include "Self.h"
#include "../refit/lib.h"

#ifndef DEBUG_ALL
#define DEBUG_SELF 1
#else
#define DEBUG_SELF DEBUG_ALL
#endif

#if DEBUG_SELF == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_SELF, __VA_ARGS__)
#endif

constexpr const LStringW KEXTS_DIRNAME(L"Kexts");


SelfOem selfOem;


EFI_STATUS SelfOem::_openDir(const XStringW& path, bool* b, EFI_FILE** efiDir)
{
  EFI_STATUS Status;
  Status = self.getCloverDir().Open(&self.getCloverDir(), efiDir, path.wc_str(), EFI_FILE_MODE_READ, 0);
  if ( EFI_ERROR(Status) ) {
    DBG("Error when opening dir '%ls\\%ls' : %s\n", self.getCloverDirPathAsXStringW().wc_str(), path.wc_str(), efiStrError(Status));
    *efiDir = NULL;
    *b = false;
  }else{
    *b = true;
  }
  return Status;
}


bool SelfOem::_checkOEMPath()
{
  EFI_STATUS Status;

//  if ( !selfOem.oemDirExists() ) return false;

  EFI_FILE* efiDir;
  Status = self.getCloverDir().Open(&self.getCloverDir(), &efiDir, m_OemPathRelToSelfDir.wc_str(), EFI_FILE_MODE_READ, 0);
  if ( Status == EFI_NOT_FOUND ) {
    DBG("_checkOEMPath Look for oem dir at path '%ls\\%ls'. Dir doesn't exist.\n", self.getCloverDirPathAsXStringW().wc_str(), m_OemPathRelToSelfDir.wc_str());
    return false;
  }
  if ( EFI_ERROR(Status) != EFI_SUCCESS ) {
    DBG("Cannot open dir at '%ls\\%ls' dir : %s\n", self.getCloverDirPathAsXStringW().wc_str(), m_OemPathRelToSelfDir.wc_str(), efiStrError(Status));
    return false;
  }
  BOOLEAN res2 = FileExists(efiDir, SWPrintf("%s.plist", m_ConfName.c_str()));
  if ( !res2 ) {
    DBG("_checkOEMPath looked for config file at '%ls\\%ls\\%s.plist'. File doesn't exist.\n", self.getCloverDirPathAsXStringW().wc_str(), m_OemPathRelToSelfDir.wc_str(), m_ConfName.c_str());
    return false;
  }
  DBG("_checkOEMPath: set OEMPath: '%ls\\%ls'\n", self.getCloverDirPathAsXStringW().wc_str(), m_OemPathRelToSelfDir.wc_str());
  return true;
}

bool SelfOem::_setOEMPath(bool isFirmwareClover, const XString8& OEMBoard, const XString8& OEMProduct, INT32 frequency, UINTN nLanCards, UINT8 gLanMac[4][6])
{

  if ( nLanCards > 0 ) {
    m_OemPathRelToSelfDir.SWPrintf("OEM\\%s--%02X-%02X-%02X-%02X-%02X-%02X", OEMProduct.c_str(), gLanMac[0][0], gLanMac[0][1], gLanMac[0][2], gLanMac[0][3], gLanMac[0][4], gLanMac[0][5]);
    if ( _checkOEMPath() ) return true;
  }
  if ( nLanCards > 1 ) {
    m_OemPathRelToSelfDir.SWPrintf("OEM\\%s--%02X-%02X-%02X-%02X-%02X-%02X", OEMProduct.c_str(), gLanMac[1][0], gLanMac[1][1], gLanMac[1][2], gLanMac[1][3], gLanMac[1][4], gLanMac[1][5]);
    if ( _checkOEMPath() ) return true;
  }
  if ( nLanCards > 2 ) {
    m_OemPathRelToSelfDir.SWPrintf("OEM\\%s--%02X-%02X-%02X-%02X-%02X-%02X", OEMProduct.c_str(), gLanMac[2][0], gLanMac[2][1], gLanMac[2][2], gLanMac[2][3], gLanMac[2][4], gLanMac[2][5]);
    if ( _checkOEMPath() ) return true;
  }
  if ( nLanCards > 3 ) {
    m_OemPathRelToSelfDir.SWPrintf("OEM\\%s--%02X-%02X-%02X-%02X-%02X-%02X", OEMProduct.c_str(), gLanMac[3][0], gLanMac[3][1], gLanMac[3][2], gLanMac[3][3], gLanMac[3][4], gLanMac[3][5]);
    if ( _checkOEMPath() ) return true;
  }
  if ( !isFirmwareClover ) {
    m_OemPathRelToSelfDir.SWPrintf("OEM\\%s\\UEFI", OEMBoard.c_str());
    if ( _checkOEMPath() ) return true;
  }
  m_OemPathRelToSelfDir.SWPrintf("OEM\\%s", OEMProduct.c_str());
  if ( _checkOEMPath() ) return true;
  m_OemPathRelToSelfDir.SWPrintf("OEM\\%s-%d", OEMProduct.c_str(), frequency);
  if ( _checkOEMPath() ) return true;
  m_OemPathRelToSelfDir.SWPrintf("OEM\\%s", OEMBoard.c_str());
  if ( _checkOEMPath() ) return true;
  m_OemPathRelToSelfDir.SWPrintf("OEM\\%s-%d", OEMBoard.c_str(), frequency);
  if ( _checkOEMPath() ) return true;

  m_OemPathRelToSelfDir.takeValueFrom(".");
  DBG("set OEMPath to \".\"\n");

  return false;
}


EFI_STATUS SelfOem::_initialize()
{
  EFI_STATUS Status;

  Status = self.getCloverDir().Open(&self.getCloverDir(), &m_OemDir, m_OemPathRelToSelfDir.wc_str(), EFI_FILE_MODE_READ, 0);
  if ( EFI_ERROR(Status) ) panic("Cannot open oem dir at '%ls\\%ls'", self.getCloverDirPathAsXStringW().wc_str(), m_OemPathRelToSelfDir.wc_str());
  DBG("Oem dir = %ls\n", (*this).getOemFullPath().wc_str());

  Status = m_OemDir->Open(m_OemDir, &m_KextsDir, KEXTS_DIRNAME.wc_str(), EFI_FILE_MODE_READ, 0);
  if ( EFI_ERROR(Status) ) {
    DBG("Cannot open %ls\\%ls : %s", getOemFullPath().wc_str(), KEXTS_DIRNAME.wc_str(), efiStrError(Status));
  }
//  if ( Status != EFI_SUCCESS  &&  Status != EFI_NOT_FOUND ) {
//    panic("Cannot open kexts dir %ls\\%ls : %s", getOemFullPath().wc_str(), KEXTS_DIRNAME.wc_str(), efiStrError(Status));
//  }
  if ( EFI_ERROR(Status) ) {
    Status = self.getCloverDir().Open(&self.getCloverDir(), &m_KextsDir, KEXTS_DIRNAME.wc_str(), EFI_FILE_MODE_READ, 0);
    if ( EFI_ERROR(Status) ) {
      DBG("Cannot open %ls\\%ls : %s", self.getCloverDirPathAsXStringW().wc_str(), KEXTS_DIRNAME.wc_str(), efiStrError(Status));
      //panic("Cannot open kexts dir at '%ls\\%ls'", self.getCloverDirPathAsXStringW().wc_str(), KEXTS_DIRNAME.wc_str());
      m_KextsDir = NULL;
      m_KextsPathRelToSelfDir.setEmpty();
      m_KextsFullPath.setEmpty();
    }else{
      m_KextsPathRelToSelfDir = KEXTS_DIRNAME;
      m_KextsFullPath.SWPrintf("%ls\\%ls", self.getCloverDirPathAsXStringW().wc_str(), KEXTS_DIRNAME.wc_str());
    }
  }else{
    m_KextsPathRelToSelfDir.SWPrintf("%ls\\%ls", getOemPathRelToSelfDir().wc_str(), KEXTS_DIRNAME.wc_str());
    m_KextsFullPath.SWPrintf("%ls\\%ls", getOemFullPath().wc_str(), KEXTS_DIRNAME.wc_str());
  }
  DBG("Kexts dir = '%ls'\n", m_KextsFullPath.wc_str()); // do not use 'getKextsFullPath()', it could panic

  return EFI_SUCCESS;
}

EFI_STATUS SelfOem::initialize(const XString8& confName, bool isFirmwareClover, const XString8& OEMBoard, const XString8& OEMProduct, INT32 frequency, UINTN nLanCards, UINT8 gLanMac[4][6])
{
  m_ConfName = confName;
  if ( _setOEMPath(isFirmwareClover, OEMBoard, OEMProduct, frequency, nLanCards, gLanMac) ) {
    m_OemFulPath = SWPrintf("%ls\\%ls", self.getCloverDirPathAsXStringW().wc_str(), m_OemPathRelToSelfDir.wc_str());
    m_OemDirExists = true;
  }else{
    m_OemFulPath = self.getCloverDirPathAsXStringW();
    m_OemDirExists = false;
  }
  return _initialize();
}

EFI_STATUS SelfOem::reInitialize()
{
  closeHandle();
  return _initialize();
}


void SelfOem::closeHandle(void)
{
  if (m_OemDir != NULL) {
    m_OemDir->Close(m_OemDir);
    m_OemDir = NULL;
  }
}

