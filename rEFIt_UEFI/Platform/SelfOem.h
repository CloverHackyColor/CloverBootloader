/*
 * Self.h
 *
 *  Created on: Sep 28, 2020
 *      Author: jief
 */

#ifndef PLATFORM_SELFOEM_H_
#define PLATFORM_SELFOEM_H_

#include <Platform.h>

class SelfOem
{
protected:
  XString8     m_ConfName;

  bool         m_OemDirExists;
  EFI_FILE*    m_OemDir;
  XStringW     m_OemPathRelToSelfDir;
  XStringW     m_OemFulPath;

  EFI_FILE*    m_KextsDir;
  XStringW     m_KextsPathRelToSelfDir;
  XStringW     m_KextsFullPath;

  EFI_STATUS _openDir(const XStringW& path, bool* b, EFI_FILE** efiDir);
  bool _checkOEMPath();
  bool _setOEMPath(bool isFirmwareClover, const XString8& OEMBoard, const XString8& OEMProduct, INT32 frequency, UINTN nLanCards, UINT8 gLanMac[4][6]);
  EFI_STATUS _initialize();

public:
  SelfOem () : m_ConfName(), m_OemDirExists(false), m_OemDir(NULL), m_OemPathRelToSelfDir(), m_OemFulPath(), m_KextsDir(NULL), m_KextsPathRelToSelfDir(), m_KextsFullPath() {};
  SelfOem(const SelfOem&) = delete;
  SelfOem& operator = (const SelfOem&) = delete;

  ~SelfOem () {};

  EFI_STATUS initialize(const XString8& confName, bool isFirmwareClover, const XString8& OEMBoard, const XString8& OEMProduct, INT32 frequency, UINTN nLanCards, UINT8 gLanMac[4][6]);
  EFI_STATUS reInitialize();
  void closeHandle();

  const XString8& getConfName() { return m_ConfName; }

  bool oemDirExists() { return m_OemDirExists; }
  const EFI_FILE& getOemDir() { return *m_OemDir; }
  const XStringW& getOemPathRelToSelfDir() { return m_OemPathRelToSelfDir; }
  const XStringW& getOemFullPath() { return m_OemFulPath; }

  bool  isKextsDirFound() { return m_KextsDir != NULL; }
  const EFI_FILE& getKextsDir() { if ( m_KextsDir == NULL) panic("Kexts dir wasn't found at initialization"); return *m_KextsDir; }
  const XStringW& getKextsPathRelToSelfDir() { if ( m_KextsDir == NULL) panic("Kexts dir wasn't found at initialization"); return m_KextsPathRelToSelfDir; }
  const XStringW& getKextsFullPath() { if ( m_KextsDir == NULL) panic("Kexts dir wasn't found at initialization"); return m_KextsFullPath; }

};

extern SelfOem selfOem;

#endif /* PLATFORM_SELF_H_ */
