/*
 * Self.h
 *
 *  Created on: Sep 28, 2020
 *      Author: jief
 */

#ifndef PLATFORM_SELFOEM_H_
#define PLATFORM_SELFOEM_H_

#include <Platform.h>
#include "Self.h"

class SelfOem
{
protected:
  XString8     m_ConfName = NullXString8; // Initialisation required by -Weffc++. Couldn't use default ctor.

//  bool         m_OemDirExists;
  EFI_FILE*    m_OemDir = NULL;
  XStringW     m_OemPathRelToSelfDir = NullXStringW;
  XStringW     m_OemFulPath = NullXStringW;

//  const EFI_FILE*    m_userConfigDir = NULL;
  XStringW     m_configDirPathRelToSelfDir = NullXStringW; // could also be empty if config dir is clover dir.
  XStringW     m_configDirPathRelToSelfDirWithTrailingSlash = NullXStringW; // could also be empty if config dir is clover dir.

  EFI_FILE*    m_KextsDir = NULL;
  XStringW     m_KextsPathRelToSelfDir = NullXStringW;
  XStringW     m_KextsFullPath = NullXStringW;

//  EFI_STATUS _openDir(const XStringW& path, bool* b, EFI_FILE** efiDir);
  bool _checkOEMPath();
  bool _setOemPathRelToSelfDir(bool isFirmwareClover, const XString8& OEMBoard, const XString8& OEMProduct, INT32 frequency, UINTN nLanCards, UINT8 gLanMac[4][6]);
  EFI_STATUS _initialize();

public:
  SelfOem () {};
  SelfOem(const SelfOem&) = delete;
  SelfOem& operator = (const SelfOem&) = delete;

  ~SelfOem () {};

  EFI_STATUS initialize(const XString8& confName, bool isFirmwareClover, const XString8& OEMBoard, const XString8& OEMProduct, INT32 frequency, UINTN nLanCards, UINT8 gLanMac[4][6]);
  void unInitialize();
  EFI_STATUS reInitialize();
  void closeHandle();

  const XString8& getConfName() { return m_ConfName; }

  bool oemDirExists() { return m_OemPathRelToSelfDir.notEmpty(); }
  const EFI_FILE& getOemDir() { assert(m_OemDir != NULL); return *m_OemDir; }
  const XStringW& getOemPathRelToSelfDir() { assert(m_OemPathRelToSelfDir.notEmpty()); return m_OemPathRelToSelfDir; }
  const XStringW& getOemFullPath() { assert(m_OemFulPath.notEmpty()); return m_OemFulPath; }

  const EFI_FILE& getConfigDir() { if ( m_OemDir != NULL ) return *m_OemDir; return self.getCloverDir(); }
  const XStringW& getConfigDirPathRelToSelfDir() { return m_configDirPathRelToSelfDirWithTrailingSlash; }
  const XStringW& getConfigDirPathRelToSelfDirWithTrailingSlash() { return m_configDirPathRelToSelfDirWithTrailingSlash; }
  const XStringW& getConfigDirFullPath() { if ( m_OemDir != NULL ) return getOemFullPath(); return self.getCloverDirFullPath(); }


  bool  isKextsDirFound() { return m_KextsDir != NULL; }
  const EFI_FILE& getKextsDir() { assert(m_KextsDir != NULL); return *m_KextsDir; }
  const XStringW& getKextsDirPathRelToSelfDir() { assert(m_KextsPathRelToSelfDir.notEmpty()); return m_KextsPathRelToSelfDir; }
  const XStringW& getKextsFullPath() { assert(m_KextsFullPath.notEmpty()); return m_KextsFullPath; }

};

extern SelfOem selfOem;

#endif /* PLATFORM_SELF_H_ */
