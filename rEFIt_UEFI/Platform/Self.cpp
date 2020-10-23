/*
 * Self.cpp
 *
 *  Created on: Sep 28, 2020
 *      Author: jief
 */

#include "Self.h"
#include <Platform.h>
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


Self self;

constexpr const LStringW THEMES_DIRNAME(L"Themes");

EFI_STATUS Self::_openDir(const XStringW& path, bool* b, EFI_FILE** efiDir)
{
  EFI_STATUS Status;
  Status = self.getCloverDir().Open(&self.getCloverDir(), efiDir, path.wc_str(), EFI_FILE_MODE_READ, 0);
  if ( EFI_ERROR(Status) ) {
    DBG("Error when opening dir '%ls\\%ls' : %s\n", self.getCloverDirFullPath().wc_str(), path.wc_str(), efiStrError(Status));
    *efiDir = NULL;
    *b = false;
  }else{
    *b = true;
  }
  return Status;
}

EFI_STATUS Self::_initialize()
{
  EFI_STATUS Status;

  Status = gBS->HandleProtocol(self.getSelfImageHandle(), &gEfiLoadedImageProtocolGuid, (void **)&m_SelfLoadedImage);
  if ( EFI_ERROR(Status) ) panic("Cannot get SelfLoadedImage");
  if ( self.getSelfDeviceHandle() == NULL )  panic("self.getSelfDeviceHandle() == NULL");

  m_SelfDevicePath = DuplicateDevicePath(DevicePathFromHandle(self.getSelfDeviceHandle()));
  if ( m_SelfDevicePath == NULL )  panic("m_SelfDevicePath == NULL");
#ifdef JIEF_DEBUG
  DBG("self.getSelfDevicePath()=%ls @%llX\n", FileDevicePathToXStringW(&self.getSelfDevicePath()).wc_str(), (uintptr_t)self.getSelfDeviceHandle());
#endif
  Status = gBS->HandleProtocol(self.getSelfDeviceHandle(), &gEfiSimpleFileSystemProtocolGuid, (void**)&m_SelfSimpleVolume);
  if ( EFI_ERROR(Status) ) panic("Cannot get m_SelfSimpleVolume");
  Status = getSelfSimpleVolume().OpenVolume(&getSelfSimpleVolume(), &m_SelfRootDir);
  if ( EFI_ERROR(Status) ) panic("Cannot get m_SelfRootDir");


  // find the current directory
  m_CloverDirFullPath = FileDevicePathToXStringW(self.getSelfLoadedImage().FilePath);

  // Do this before the next check.
  if ( !m_CloverDirFullPath.startWith('\\') ) {
    //CHAR16* f = ConvertDevicePathToText(self.getSelfLoadedImage().FilePath, TRUE, TRUE);
    //panic("Bad format for m_CloverDirFullPath(%ls). It must start with a '\\'.\nConvertDevicePathToText=%ls", m_CloverDirFullPath.wc_str(), f);
    //
    // Somefirmware seems to not put a '\' at the begining. Do not panic anymore, just add it.
    m_CloverDirFullPath.insertAtPos('\\', 0);
  }

  // History : if this Clover was started as BootX64.efi, redirect to /EFI/CLOVER
  if ( m_CloverDirFullPath.equalIC("\\EFI\\Boot\\BootX64.efi") ) {
    m_CloverDirFullPath.takeValueFrom("\\EFI\\CLOVER\\CloverX64.efi");
  }
  if ( m_CloverDirFullPath.isEmpty() ) panic("m_CloverDirFullPath.isEmpty()");

  m_SelfDevicePath = FileDevicePath(self.getSelfDeviceHandle(), m_CloverDirFullPath);
  m_SelfDevicePathAsXStringW = FileDevicePathToXStringW(m_SelfDevicePath);

  if ( m_CloverDirFullPath.lastChar() == U'\\' ) panic("m_CloverDirFullPath.lastChar() == U'\\'");
//if ( m_CloverDirFullPath.endsWith('\\') ) panic("m_CloverDirFullPath.endsWith('\\')");

  size_t i = m_CloverDirFullPath.rindexOf(U'\\', SIZE_T_MAX-1);
  if ( i != SIZE_T_MAX && i > 0 ) m_CloverDirFullPath.deleteCharsAtPos(i, SIZE_T_MAX);
#ifdef JIEF_DEBUG
  DBG("SelfDirPath = %ls\n", m_CloverDirFullPath.wc_str());
#endif
  Status = self.getSelfVolumeRootDir().Open(&self.getSelfVolumeRootDir(), &m_CloverDir, m_CloverDirFullPath.wc_str(), EFI_FILE_MODE_READ, 0);
  if ( EFI_ERROR(Status) ) panic("Cannot open getSelfRootDir()");


  _openDir(THEMES_DIRNAME, &m_ThemesDirExists, &m_ThemesDir); // don't need to check returned status as it's ok to not have a themes dir.

  return EFI_SUCCESS;
}

EFI_STATUS Self::initialize(EFI_HANDLE ImageHandle)
{
  m_SelfImageHandle = ImageHandle;
  return _initialize();
}

EFI_STATUS Self::reInitialize()
{
  closeHandle();
  return _initialize();
}


void Self::closeHandle(void)
{
  // called before running external programs to close open file handles

  if (m_CloverDir != NULL) {
    m_CloverDir->Close(m_CloverDir);
    m_CloverDir = NULL;
  }

  if (m_SelfRootDir != NULL) {
    m_SelfRootDir->Close(m_SelfRootDir);
    m_SelfRootDir = NULL;
  }
}

