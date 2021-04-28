/*
 * Self.cpp
 *
 *  Created on: Sep 28, 2020
 *      Author: jief
 */

#include <Platform.h>
#include "../Settings/SelfOem.h"
#include "../Settings/Self.h"
#include "../refit/lib.h"

#define DONT_DEFINE_GLOBALS

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

constexpr const LStringW KEXTS_DIRNAME = L"Kexts"_XSW;


SelfOem selfOem;


//EFI_STATUS SelfOem::_openDir(const XStringW& path, bool* b, EFI_FILE** efiDir)
//{
//  EFI_STATUS Status;
//  Status = self.getCloverDir().Open(&self.getCloverDir(), efiDir, path.wc_str(), EFI_FILE_MODE_READ, 0);
//  if ( EFI_ERROR(Status) ) {
//    DBG("Error when opening dir '%ls\\%ls' : %s\n", self.getCloverDirFullPath().wc_str(), path.wc_str(), efiStrError(Status));
//    *efiDir = NULL;
//    *b = false;
//  }else{
//    *b = true;
//  }
//  return Status;
//}


bool SelfOem::_checkOEMPath()
{
  EFI_STATUS Status;

//  if ( !selfOem.oemDirExists() ) return false;

//  EFI_FILE* efiDir;
  Status = self.getCloverDir().Open(&self.getCloverDir(), &m_OemDir, m_OemPathRelToSelfDir.wc_str(), EFI_FILE_MODE_READ, 0);
  if ( Status == EFI_NOT_FOUND ) {
    DBG("_checkOEMPath Look for oem dir at path '%ls\\%ls'. Dir doesn't exist.\n", self.getCloverDirFullPath().wc_str(), m_OemPathRelToSelfDir.wc_str());
    m_OemDir = NULL;
    return false;
  }
  if ( EFI_ERROR(Status) != EFI_SUCCESS ) {
    DBG("Cannot open dir at '%ls\\%ls' dir : %s\n", self.getCloverDirFullPath().wc_str(), m_OemPathRelToSelfDir.wc_str(), efiStrError(Status));
    m_OemDir = NULL;
    return false;
  }
  BOOLEAN res2 = FileExists(m_OemDir, SWPrintf("%s.plist", m_ConfName.c_str()));
  if ( !res2 ) {
    DBG("_checkOEMPath looked for config file at '%ls\\%ls\\%s.plist'. File doesn't exist.\n", self.getCloverDirFullPath().wc_str(), m_OemPathRelToSelfDir.wc_str(), m_ConfName.c_str());
    m_OemDir->Close(m_OemDir);
    m_OemDir = NULL;
    return false;
  }
  DBG("_checkOEMPath: set OEMPath: '%ls\\%ls'\n", self.getCloverDirFullPath().wc_str(), m_OemPathRelToSelfDir.wc_str());
  return true;
}

bool SelfOem::_setOemPathRelToSelfDir(bool isFirmwareClover, const XString8& OEMBoard, const XString8& OEMProduct, INT32 frequency, const XObjArray<LanCardClass>& lanCardArray)
{
  for ( size_t idx = 0 ; idx < lanCardArray.size() ; ++idx ) {
    m_OemPathRelToSelfDir.SWPrintf("OEM\\%s--%02X-%02X-%02X-%02X-%02X-%02X", OEMProduct.c_str(), lanCardArray[idx].MacAddress[0], lanCardArray[idx].MacAddress[1], lanCardArray[idx].MacAddress[2], lanCardArray[idx].MacAddress[3], lanCardArray[idx].MacAddress[4], lanCardArray[idx].MacAddress[5]);
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

//  m_OemPathRelToSelfDir.takeValueFrom(".");
//  DBG("set OEMPath to \".\"\n");
  m_OemPathRelToSelfDir.setEmpty();

  return false;
}


EFI_STATUS SelfOem::_initialize()
{
  EFI_STATUS Status;

  if ( oemDirExists() ) {
    m_OemFulPath = SWPrintf("%ls\\%ls", self.getCloverDirFullPath().wc_str(), m_OemPathRelToSelfDir.wc_str());
    m_configDirPathRelToSelfDir = getOemPathRelToSelfDir();
    m_configDirPathRelToSelfDirWithTrailingSlash.SWPrintf("%ls\\", m_configDirPathRelToSelfDir.wc_str());
  }else{
    m_OemFulPath.setEmpty();
    m_configDirPathRelToSelfDir.setEmpty();
    m_configDirPathRelToSelfDirWithTrailingSlash.setEmpty();
  }
  if ( m_OemDir == NULL ) {
    assert( m_OemPathRelToSelfDir.isEmpty() );
    assert( m_OemFulPath.isEmpty() );
  }else{
    assert( m_OemPathRelToSelfDir.notEmpty() );
    assert( m_OemFulPath.notEmpty() );
  }

  if ( m_KextsDir != NULL ) {
    log_technical_bug("%s : Kexts dir != NULL.", __PRETTY_FUNCTION__);
    m_KextsDir = NULL;
  }
  if ( oemDirExists() ) {
    Status = m_OemDir->Open(m_OemDir, &m_KextsDir, KEXTS_DIRNAME.wc_str(), EFI_FILE_MODE_READ, 0);
    if ( !EFI_ERROR(Status) ) {
      m_KextsPathRelToSelfDir.SWPrintf("%ls\\%ls", getOemPathRelToSelfDir().wc_str(), KEXTS_DIRNAME.wc_str());
      m_KextsFullPath.SWPrintf("%ls\\%ls", getOemFullPath().wc_str(), KEXTS_DIRNAME.wc_str());
    }else{
      DBG("Cannot open %ls\\%ls : %s", getOemFullPath().wc_str(), KEXTS_DIRNAME.wc_str(), efiStrError(Status));
      m_KextsDir = NULL;
    }
  }
//  if ( Status != EFI_SUCCESS  &&  Status != EFI_NOT_FOUND ) {
//    panic("Cannot open kexts dir %ls\\%ls : %s", getOemFullPath().wc_str(), KEXTS_DIRNAME.wc_str(), efiStrError(Status));
//  }
  if ( m_KextsDir == NULL ) {
    Status = self.getCloverDir().Open(&self.getCloverDir(), &m_KextsDir, KEXTS_DIRNAME.wc_str(), EFI_FILE_MODE_READ, 0);
    if ( EFI_ERROR(Status) ) {
      DBG("Cannot open %ls\\%ls : %s", self.getCloverDirFullPath().wc_str(), KEXTS_DIRNAME.wc_str(), efiStrError(Status));
      //panic("Cannot open kexts dir at '%ls\\%ls'", self.getCloverDirFullPath().wc_str(), KEXTS_DIRNAME.wc_str());
      m_KextsDir = NULL;
      m_KextsPathRelToSelfDir.setEmpty();
      m_KextsFullPath.setEmpty();
    }else{
      m_KextsPathRelToSelfDir = KEXTS_DIRNAME;
      m_KextsFullPath.SWPrintf("%ls\\%ls", self.getCloverDirFullPath().wc_str(), KEXTS_DIRNAME.wc_str());
    }
  }else{
  }
  if ( m_KextsDir == NULL ) {
    if ( m_KextsPathRelToSelfDir.notEmpty() ) log_technical_bug("%s : m_KextsPathRelToSelfDir.notEmpty()", __PRETTY_FUNCTION__);
    if ( m_KextsFullPath.notEmpty() ) log_technical_bug("%s : m_KextsFullPath.notEmpty()", __PRETTY_FUNCTION__);
  }else{
    if ( m_KextsPathRelToSelfDir.isEmpty() ) log_technical_bug("%s : m_KextsPathRelToSelfDir.isEmpty()", __PRETTY_FUNCTION__);
    if ( m_KextsFullPath.isEmpty() ) log_technical_bug("%s : m_KextsFullPath.isEmpty()", __PRETTY_FUNCTION__);
  }
#ifdef JIEF_DEBUG
  if ( isKextsDirFound() ) {
    DBG("Kexts dir = '%ls'\n", getKextsFullPath().wc_str());
  }else{
    DBG("Kexts dir = none\n");
  }
#endif
  return EFI_SUCCESS;
}

EFI_STATUS SelfOem::initialize(const XString8& confName, bool isFirmwareClover, const XString8& OEMBoard, const XString8& OEMProduct, INT32 frequency, const XObjArray<LanCardClass>& lanCardArray)
{
  if ( m_ConfName.notEmpty() ) {
    log_technical_bug("%s : cannot be called twice. Use reinitialize.", __PRETTY_FUNCTION__);
    return EFI_SUCCESS;
  }
  // confName must not be empty as it serves as a flag to know if it's initialized or not.
  // {confName}.plist will also be used to detect OEM dir.
  if ( confName.isEmpty() ) {
    log_technical_bug("%s : cannot be called with empty config name.", __PRETTY_FUNCTION__);
    return EFI_SUCCESS;
  }

  m_ConfName = confName;

  // Initialise m_OemPathRelToSelfDir and leave m_OemDir opened.
  _setOemPathRelToSelfDir(isFirmwareClover, OEMBoard, OEMProduct, frequency, lanCardArray);

  EFI_STATUS Status = _initialize();
  return Status;
}

void SelfOem::unInitialize()
{
  if ( m_ConfName.isEmpty() ) {
    log_technical_bug("%s : Already uninitialized.", __PRETTY_FUNCTION__);
    return;
  }

  closeHandle();
  m_ConfName.setEmpty();
}

EFI_STATUS SelfOem::reInitialize()
{
  if ( m_ConfName.isEmpty() ) {
    log_technical_bug("%s : initialize() must called once first", __PRETTY_FUNCTION__);
    return EFI_LOAD_ERROR;
  }

  closeHandle();

  // No need to call _setOemPathRelToSelfDir again, but need to open m_OemDir, if it exists
  if ( oemDirExists() ) {
    EFI_STATUS Status = self.getCloverDir().Open(&self.getCloverDir(), &m_OemDir, m_OemPathRelToSelfDir.wc_str(), EFI_FILE_MODE_READ, 0);
    if ( EFI_ERROR(Status) ) {
      log_technical_bug("Impossible to reopen dir '%ls\\%ls', although it was opened the first time : %s", self.getCloverDirFullPath().wc_str(), m_OemPathRelToSelfDir.wc_str(), efiStrError(Status));
      return Status;
    }
  }
  EFI_STATUS Status = _initialize();
  return Status;
}


void SelfOem::closeHandle()
{
//DBG("%s : enter.\n", __FUNCTION__);
  if (m_KextsDir != NULL) {
    m_KextsDir->Close(m_KextsDir);
    m_KextsDir = NULL;
    m_KextsPathRelToSelfDir.setEmpty();
    m_KextsFullPath.setEmpty();
  }
  if (m_OemDir != NULL) {
    m_OemDir->Close(m_OemDir);
    m_OemDir = NULL;
    // m_OemPathRelToSelfDir.setEmpty(); // do not empty m_OemPathRelToSelfDir, we need it in reInitialize()
    // m_OemFulPath.setEmpty(); // doesn't matter, it'll be re-initialised in _initialize()
  }
//DBG("%s : leave.\n", __FUNCTION__);
}

